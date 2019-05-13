
#include "ObjectHandler.hpp"

/*============================================================================
*						Public functions
*===========================================================================*/

ObjectHandler::ObjectHandler()
{}

ObjectHandler::~ObjectHandler()
{}

void ObjectHandler::InitializeObjects(ID3D11Device* Dev, ID3D11DeviceContext* DevCon)
{
	CreateHeightMap(Dev);
	LoadObjectModel(Dev, DevCon, L"wt_teapot.obj", DYNAMIC_OBJECT, false, false);
	LoadObjectModel(Dev, DevCon, L"cube.obj", DYNAMIC_OBJECT, false, false);

	for (int i = 0; i < 400; i++) 
	{
		LoadObjectModel(Dev, DevCon, L"cube.obj", STATIC_OBJECT, false, false);
	}

	MoveStaticObjects();

	CreatePerObjectConstantBuffers(Dev);
	CreateMaterialConstantBuffers(Dev);


	mQuadtree = Quadtree(DirectX::XMVectorZero(), DirectX::XMVectorSet(static_cast<float>(WORLD_WIDTH), 50.0f, static_cast<float>(WORLD_DEPTH), 0.0f), 1);
	InsertToQuadtree();

}

bool ObjectHandler::SetHeightMapBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	UINT32 squareVertexSize = sizeof(float) * 8;
	UINT32 offset = 0;

	DevCon->IASetIndexBuffer(gSquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DevCon->IASetVertexBuffers(0, 1, &gSquareVertBuffer, &squareVertexSize, &offset);
	
	if (passID == GEOMETRY_PASS)
	{
		DevCon->GSSetConstantBuffers(1, 1, &mHeightMapWorldBuffer);
		DevCon->PSSetConstantBuffers(0, 1, &mHeightMapMaterialBuffer);
	}

	if (passID == SHADOW_PASS)
	{
		DevCon->VSSetConstantBuffers(1, 1, &mHeightMapWorldBuffer);
	}

	return true;
}

bool ObjectHandler::SetObjectBufferWithIndex(ID3D11DeviceContext* DevCon, RenderPassID passID, 
	ObjectType objectType, int objectIndex, int materialIndex)
{
	using DirectX::operator*;

	UINT32 vertexSize = sizeof(Vertex);
	UINT32 offset = 0;

	std::vector<Object>* objectArray = nullptr; // pointer to one of the arrays of objects in ObjectHandler
	if (objectType == STATIC_OBJECT)  { objectArray = &mStaticObjects; }
	if (objectType == DYNAMIC_OBJECT) { objectArray = &mDynamicObjects; }

	// Set index and vertex buffers
	DevCon->IASetIndexBuffer((*objectArray)[objectIndex].meshIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DevCon->IASetVertexBuffers(0, 1, &(*objectArray)[objectIndex].meshVertexBuffer, &vertexSize, &offset);

	// Dynamic Objects will rotate at the same speed around the same axis but in different locations
	if (objectType == DYNAMIC_OBJECT)
	{
		static float rotation = 0.0f;
		rotation += 0.001f;
		float scale = 10.0f;


		DirectX::XMMATRIX scaleMatrix    = DirectX::XMMatrixScaling(scale, scale, scale);
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(90.0f)) * DirectX::XMMatrixRotationX(rotation);
		DirectX::XMMATRIX locationMatrix = DirectX::XMMatrixTranslation(100.0f + 20.f * (objectIndex + 1.0f), 30.0f * (objectIndex + 1.0f), 100.0f);

		DirectX::XMMATRIX finalMatrix = rotationMatrix * scaleMatrix * locationMatrix;

		XMStoreFloat4x4(&(*objectArray)[objectIndex].objectBufferData.World, DirectX::XMMatrixTranspose(finalMatrix));
	}

	if (objectType == STATIC_OBJECT)
	{
		XMStoreFloat4x4(&(*objectArray)[objectIndex].objectBufferData.World,(*objectArray)[objectIndex].worldMatrixPerObject);
	}

	D3D11_MAPPED_SUBRESOURCE worldMatrixPtr;
	DevCon->Map((*objectArray)[objectIndex].perObjectWorldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &worldMatrixPtr);
	// copy memory from CPU to GPU of the entire struct
	memcpy(worldMatrixPtr.pData, &(*objectArray)[objectIndex].objectBufferData, sizeof(cPerObjectBuffer));
	// Unmap constant buffer so that we can use it again in the GPU
	DevCon->Unmap((*objectArray)[objectIndex].perObjectWorldBuffer, 0);
	
	// The geometry pass computes the transformations in the Geometry shader and the shadow pass computes the transformations in the Vertex shader
	if (passID == GEOMETRY_PASS) { DevCon->GSSetConstantBuffers(1, 1, &(*objectArray)[objectIndex].perObjectWorldBuffer); }
	if (passID == SHADOW_PASS)   { DevCon->VSSetConstantBuffers(1, 1, &(*objectArray)[objectIndex].perObjectWorldBuffer); }

	if (passID == GEOMETRY_PASS)
	{
		int matInd = (*objectArray)[objectIndex].meshSubsetMaterialIndex[materialIndex];
		if (mMaterialArray[matInd].Data.HasTexture == 1)
		{
			int texIndex = mMaterialArray[matInd].Data.TexArrIndex;
			DevCon->PSSetShaderResources(1, 1, &mMeshTextureSRV[texIndex]);
		}

		D3D11_MAPPED_SUBRESOURCE materialPtr;
		DevCon->Map(mMaterialBufferArray[matInd], 0, D3D11_MAP_WRITE_DISCARD, 0, &materialPtr);
		memcpy(materialPtr.pData, &mMaterialArray[matInd].Data, sizeof(cMaterialBuffer));
		DevCon->Unmap(mMaterialBufferArray[matInd], 0);

		DevCon->PSSetConstantBuffers(0, 1, &mMaterialBufferArray[matInd]);
	}
	return true;
}

//Returns a pointer to either the std::vector of static objects or the std::vector of dynamic objects
std::vector<Object>* ObjectHandler::GetObjectArrayPtr(ObjectType objectType)
{
	return (objectType == STATIC_OBJECT) ? &mStaticObjects : &mDynamicObjects;
}

// Height Map -----------------------------------------------------------------

void ObjectHandler::CreateHeightMap(ID3D11Device* Dev)
{
	using DirectX::operator/;

	//creating what is needed for the heightmap
	HeightMapInfo hminfo;
	LoadHeightMap("heightmap.bmp", hminfo);
	unsigned int columns = hminfo.worldWidth;
	unsigned int rows = hminfo.worldHeight;

	NUMBER_OF_VERTICES = rows*columns;
	NUMBER_OF_FACES = (rows - 1)*(columns - 1) * 2;

	WORLD_WIDTH = rows;
	WORLD_DEPTH = columns;
	WORLD_HEIGHT = new float*[WORLD_WIDTH];

	std::vector<Vertex> mapVertex(NUMBER_OF_VERTICES);

	for (DWORD i = 0; i < rows; i++) {
		WORLD_HEIGHT[i] = new float[WORLD_DEPTH];
		for (DWORD j = 0; j < columns; j++) {
			mapVertex[i*columns + j].pos = hminfo.heightMap[i*columns + j]; //storing height and position in the struct
			mapVertex[i*columns + j].normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);//storing a default normal
			WORLD_HEIGHT[i][j] = hminfo.heightMap[i*columns + j].y;
		}
	}

	//assigns uv-coordinates as well as setting the order they will be drawn in 
	std::vector<DWORD> drawOrder(NUMBER_OF_FACES * 3);
	int k = 0, texUIndex = 0, texVIndex = 0;
	for (DWORD j = 0; j < rows - 1; j++) {
		for (DWORD i = 0; i < columns - 1; i++) {
			drawOrder[k] = i*columns + j;//bottom left
			mapVertex[i*columns + j].texCoord = DirectX::XMFLOAT2((texUIndex + 0.0f), (texVIndex + 1.0f));
			drawOrder[k + 1] = (1 + i)*columns + j;//top left
			mapVertex[(1 + i)*columns + j].texCoord = DirectX::XMFLOAT2((texUIndex + 0.0f), (texVIndex + 0.0f));
			drawOrder[k + 2] = i*columns + j + 1;//bottom right
			mapVertex[i*columns + j + 1].texCoord = DirectX::XMFLOAT2((texUIndex + 1.0f), (texVIndex + 1.0f));


			drawOrder[k + 3] = (1 + i)*columns + j + 1;//top right
			mapVertex[(1 + i)*columns + j + 1].texCoord = DirectX::XMFLOAT2((texUIndex + 1.0f), (texVIndex + 0.0f));
			drawOrder[k + 4] = i*columns + j + 1;//bottom right
			mapVertex[i*columns + j + 1].texCoord = DirectX::XMFLOAT2((texUIndex + 1.0f), (texVIndex + 1.0f));
			drawOrder[k + 5] = (1 + i)*columns + j;//top left
			mapVertex[(1 + i)*columns + j].texCoord = DirectX::XMFLOAT2((texUIndex + 0.0f), (texVIndex + 0.0f));

			k += 6;
			texVIndex++;
		}
		texVIndex = 0;
		texUIndex++;
	}

	// Change the scale of the textures.
	DirectX::XMVECTOR temp;
	for (int i = 0; i < NUMBER_OF_VERTICES; i++)
	{
		using DirectX::operator*;
		temp = DirectX::XMLoadFloat2(&mapVertex[i].texCoord);
		DirectX::XMStoreFloat2(&mapVertex[i].texCoord, temp * 0.05f);
	}

	std::vector<std::vector<int>> vertexFaces(NUMBER_OF_VERTICES); //So that each vertex knows which faces it's connected to

	//calculates the normal for each face
	std::vector<DirectX::XMFLOAT3> tempNormal;
	DirectX::XMFLOAT3 nonNormalized = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	float vecX, vecY, vecZ;
	DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < NUMBER_OF_FACES; i++)
	{
		vecX = mapVertex[drawOrder[(i * 3)]].pos.x - mapVertex[drawOrder[(i * 3) + 2]].pos.x;
		vecY = mapVertex[drawOrder[(i * 3)]].pos.y - mapVertex[drawOrder[(i * 3) + 2]].pos.y;
		vecZ = mapVertex[drawOrder[(i * 3)]].pos.z - mapVertex[drawOrder[(i * 3) + 2]].pos.z;
		edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

		vecX = mapVertex[drawOrder[(i * 3) + 2]].pos.x - mapVertex[drawOrder[(i * 3) + 1]].pos.x;
		vecY = mapVertex[drawOrder[(i * 3) + 2]].pos.y - mapVertex[drawOrder[(i * 3) + 1]].pos.y;
		vecZ = mapVertex[drawOrder[(i * 3) + 2]].pos.z - mapVertex[drawOrder[(i * 3) + 1]].pos.z;
		edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);


		DirectX::XMStoreFloat3(&nonNormalized, DirectX::XMVector3Cross(edge2, edge1));
		tempNormal.push_back(nonNormalized);

		vertexFaces[drawOrder[i * 3]].push_back(i);
		vertexFaces[drawOrder[(i * 3) + 1]].push_back(i);
		vertexFaces[drawOrder[(i * 3) + 2]].push_back(i);
	}

	//calculate the average normal in order to make the worldIdentity smooth
	DirectX::XMVECTOR averageNormal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 avgNorm = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NUMBER_OF_VERTICES; i++)
	{
		averageNormal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		size_t facesUsing = vertexFaces[i].size();

		for (size_t j = 0; j < facesUsing; j++)
		{
			int k = vertexFaces[i][j];
			avgNorm.x += tempNormal[k].x;
			avgNorm.y += tempNormal[k].y;
			avgNorm.z += tempNormal[k].z;
		} //end for vertexFaces[i]

		avgNorm.x /= facesUsing;
		avgNorm.y /= facesUsing;
		avgNorm.z /= facesUsing;

		averageNormal = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&avgNorm));

		DirectX::XMStoreFloat3(&mapVertex[i].normal, averageNormal);
	} //end for NUMBER_OF_VERTICIES

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD)*NUMBER_OF_FACES * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &drawOrder[0];
	Dev->CreateBuffer(&indexBufferDesc, &iinitData, &gSquareIndexBuffer);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex)*NUMBER_OF_VERTICES;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &mapVertex[0];
	Dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &gSquareVertBuffer);
}

float** ObjectHandler::getWorldHeight() const
{
	return WORLD_HEIGHT;
}

int ObjectHandler::getWorldDepth() const 
{
	return WORLD_DEPTH;
}

int ObjectHandler::getWorldWidth() const
{
	return WORLD_WIDTH;
}

int ObjectHandler::GetHeightMapNrOfFaces() const
{
	return NUMBER_OF_FACES;
}

// Quadtree -------------------------------------------------------------------

void ObjectHandler::MoveStaticObjects() 
{
	using DirectX::operator*;

	float scalingFactor = 2.0f;
	for (size_t i = 0; i < mStaticObjects.size(); i++) 
	{
		DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scalingFactor, scalingFactor, scalingFactor);
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(0.1f * i, 0.2f * i, 0.3f * i);
		DirectX::XMMATRIX locationMatrix = DirectX::XMMatrixTranslation(
			10.0f * (i / 20 + 1.0f),
			WORLD_HEIGHT[10 * (i % 20 + 1)][10 * (i / 20 + 1)] + 4.0f,
			10.0f * (i % 20 + 1.0f));

		DirectX::XMMATRIX finalMatrix = rotationMatrix * scaleMatrix * locationMatrix;

		mStaticObjects[i].worldMatrixPerObject = DirectX::XMMatrixTranspose(finalMatrix);
	}
}

void ObjectHandler::InsertToQuadtree() 
{
	for (UINT i = 0; i < mStaticObjects.size(); i++) 
	{
		DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(mStaticObjects[i].worldMatrixPerObject);
		for (UINT j = 0; j < mStaticObjects[i].meshVertexData.size(); j++) 
		{
			DirectX::XMVECTOR modelVertexPos = DirectX::XMLoadFloat3(&mStaticObjects[i].meshVertexData[j].pos);
			DirectX::XMVECTOR worldVertexPos = DirectX::XMVector3Transform(modelVertexPos, world);
			mQuadtree.storeObjects(i, worldVertexPos, mQuadtree.root);
		}
	}
}

/*=============================================================================
*						Private functions
*===========================================================================*/

// The width and height of the .bmp file must be multiples of 4
bool ObjectHandler::LoadObjectModel(ID3D11Device* Dev, ID3D11DeviceContext* DevCon, std::wstring filename, ObjectType objectType, bool isRHCoordSys, bool computeNormals)
{
	std::wifstream fileIn(filename.c_str());    //Open file
	std::wstring meshMatLib;                    //String to hold the obj material library filename

	//Arrays to store the model's information
	std::vector<DWORD> indices;
	std::vector<DirectX::XMFLOAT3> vertPos;
	std::vector<DirectX::XMFLOAT3> vertNorm;
	std::vector<DirectX::XMFLOAT2> vertTexCoord;
	std::vector<std::wstring> meshMaterials;

	//Vertex definition indices
	std::vector<int> vertPosIndex;
	std::vector<int> vertNormIndex;
	std::vector<int> vertTCIndex;

	// bools to check if the .obj file has texture coordinates and normals defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	//Temp variables to store into vectors
	std::wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;				//Used to read through the file charater by character
	std::wstring face;				//Used to hold the string containing the verticies
	int vIndex = 0;					//Keep track of the vertex index count
	int perFaceTriangleCount = 0;	//Number of triangles each face consists of
	int totalVerts = 0;				//Total number of verticies in the entire model
	int totalTriangles = 0;			//Total number of triangles in the entire model

	Object object; //Object which will eventually store all the information


	if (!fileIn) //Early exit if the file doesn't open
	{
		OutputDebugString(L"\nObjectHandler::LoadObjectModel() Failed to open .obj file\n\n");
		exit(-1);
	}

	// .obj file loop
	while (fileIn)
	{
		checkChar = fileIn.get();			//get the next char
		switch (checkChar)
		{
		case L'#': //Comment line
		{
			while (checkChar != L'\n')		//Keep reading chars until the line ends
				checkChar = fileIn.get();
		}
		break;
		case L'v': //Get vertex descriptions
		{
			checkChar = fileIn.get();
			if (checkChar == L' ')			//v - vertex position
			{
				float vx, vy, vz;
				fileIn >> vx >> vy >> vz;	//store the vertex positions
				vertPos.push_back(DirectX::XMFLOAT3(vx, vy, (isRHCoordSys) ? vz * -1.0f : vz));		//invert Z-axiz if the OBJ is RH
			}
			if (checkChar == L't')			//vt - vertex texture coordinates
			{
				float vu, vv;
				fileIn >> vu >> vv;			//store the uv-coordinates
				vertTexCoord.push_back(DirectX::XMFLOAT2(vu, (isRHCoordSys) ? 1.0f - vv : vv));		//Reverse the v-axis if the OBJ is RH
				hasTexCoord = true;
			}
			if (checkChar == L'n')			//vn - vertex normal
			{
				float vnx, vny, vnz;
				fileIn >> vnx >> vny >> vnz; //store the normal values
				vertNorm.push_back(DirectX::XMFLOAT3(vnx, vny, ((isRHCoordSys) ? (-1.0f * vnz) : vnz))); //Invert the z-axiz if the OBJ is RH
				hasNorm = true;
			}
		}
		break;
		case L'g': //g - group
		{
			checkChar = fileIn.get();
			if (checkChar == L' ')
			{
				object.meshSubsetIndexStart.push_back(vIndex);
				object.nrOfMeshSubsets++;
				while (checkChar != L'\n')		//Keep reading chars until the line ends
					checkChar = fileIn.get();
			}
		}
		break;
		case L'f': //f - faces. Vertex definition [vPos1/vTexCoord1/vNorm1 ... ]
		{
			checkChar = fileIn.get();
			if (checkChar != L' ') //early exit if the next character isn't a space
			{
				break;
			}

			face = L"";				//clear the temporary wide string which will be used to store the line
			std::wstring VertDef;	//holds one vertex definition at a time
			perFaceTriangleCount = 0;

			checkChar = fileIn.get();
			//read through the whole line (faces are at the bottom of .obj files so we need to check for end of file as well as new line)
			while (checkChar != L'\n' && !fileIn.eof())
			{
				face += checkChar;		//add every char to the face string
				checkChar = fileIn.get();
				if (checkChar == L' ')
					perFaceTriangleCount++;	//increase triangle count if it's a space
			}

			//check for space at the end of the face string
			if (face[face.length() - 1] == L' ')
				perFaceTriangleCount--;
			perFaceTriangleCount -= 1; //every vertex in the face after the first two are new faces

			std::wstringstream ss(face);
			if (face.length() <= 0) // early exit if the entire line is 'f '
				break;

			int firstVIndex, lastVIndex; // first and last vertice's index

			for (int i = 0; i < 3; i++) //First three vertices
			{
				ss >> VertDef; //get one vertex definition [vPos/vTexCoord/vNorm]
				std::wstring vertPart;
				int whichPart = 0; //vPos = 0, vTexCoord = 1, vNorm = 2
									//Parse the string
				for (unsigned int j = 0; j < VertDef.length(); j++)
				{
					if (VertDef[j] != L'/')		//if there's no divider add char to vertPart
						vertPart += VertDef[j];
					//If the current char is a divider or the last character in the string
					if (VertDef[j] == L'/' || j == VertDef.length() - 1)
					{
						std::wistringstream wstringToInt(vertPart); //used to convert wstring to int

						if (whichPart == 0) //if vPos
						{
							wstringToInt >> vertPosIndexTemp;
							vertPosIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1

													//check if the vert pos was the only thing specified
							if (j == VertDef.length() - 1)
							{
								vertNormIndexTemp = 0;
								vertTCIndexTemp = 0;
							}
						} //end if vPos
						else if (whichPart == 1) //if vTexCoord
						{
							if (vertPart != L"") //check if there is a tex coord
							{
								wstringToInt >> vertTCIndexTemp;
								vertTCIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
							}
							else //if there is no tex coord 0 will be default index
								vertTCIndexTemp = 0;

							//if the current char is the second to last in the string then there must be no normal so a default normal is set to index 0
							if (j == VertDef.length() - 1)
								vertNormIndexTemp = 0;

						} //end if vTexCoord
						else if (whichPart == 2) //if vNorm
						{
							//std::wistringstream wstringToInt(vertPart);
							wstringToInt >> vertNormIndexTemp;
							vertNormIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
						} //end if vNorm
						vertPart = L"";
						whichPart++;
					} //end if current char is a divider or the last char in the string
				} //end for j-loop

					//check to make sure there's at least one subset
				if (object.nrOfMeshSubsets == 0)
				{
					object.meshSubsetIndexStart.push_back(vIndex); //start index for this subset
					object.nrOfMeshSubsets++;
				}

				vertPosIndex.push_back(vertPosIndexTemp);
				vertTCIndex.push_back(vertTCIndexTemp);
				vertNormIndex.push_back(vertNormIndexTemp);
				totalVerts++;						//new vertex created
				indices.push_back(totalVerts - 1);	//set index for this vertex

				//make sure the rest of the triangles use the first vertex
				if (i == 0)
				{
					firstVIndex = indices[vIndex];		//the first vertex index of this face
				}

				//if this was the last vertex in the first triangle the next triangle will use it as well
				if (i == 2)
				{
					lastVIndex = indices[vIndex];		//the last vertex index of this triangle
				}
				vIndex++; //increment index count
			} //end for i-loop
			totalTriangles++; //one triangle down

			//Break the face into more than one triangle in case the face has more than three vertexes
			for (int l = 0; l < perFaceTriangleCount - 1; ++l)
			{
				//first vertex of this triangle (the very first vertex of the face too)
				indices.push_back(firstVIndex); //set index for this vertex
				vIndex++;

				//second vertex of this triangle (the last vertex used in the tri before this one)
				indices.push_back(lastVIndex); //set index for this vertex
				vIndex++;

				//get the third vertex for this triangle
				ss >> VertDef;

				std::wstring vertPart;
				int whichPart = 0;

				//parse this string (same procedure as in the for i-loop)
				for (unsigned int j = 0; j < VertDef.length(); j++)
				{
					if (VertDef[j] != L'/')		//if there's no divider add char to vertPart
						vertPart += VertDef[j];
					//If the current char is a divider or the last character in the string
					if (VertDef[j] == L'/' || j == VertDef.length() - 1)
					{
						std::wistringstream wstringToInt(vertPart); //used to convert wstring to int

						if (whichPart == 0) //if vPos
						{
							wstringToInt >> vertPosIndexTemp;
							vertPosIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1

							//check if the vert pos was the only thing specified
							if (j == VertDef.length() - 1)
							{
								vertNormIndexTemp = 0;
								vertTCIndexTemp = 0;
							}
						} //end if vPos
						else if (whichPart == 1) //if vTexCoord
						{
							if (vertPart != L"") //check if there is a tex coord
							{
								wstringToInt >> vertTCIndexTemp;
								vertTCIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
							}
							else //if there is no tex coord 0 will be default index
								vertTCIndexTemp = 0;

							//if the current char is the second to last in the string then there must be no normal so a default normal is set to index 0
							if (j == VertDef.length() - 1)
								vertNormIndexTemp = 0;

						} //end if vTexCoord
						else if (whichPart == 2) //if vNorm
						{
							//std::wistringstream wstringToInt(vertPart);
							wstringToInt >> vertNormIndexTemp;
							vertNormIndexTemp -= 1; //subtract one since c++ starts arrays at 0 and .obj starts indexing at 1
						} //end if vNorm

						vertPart = L"";
						whichPart++;
					} //end if current char is a divider or the last char in the string
				} //end for j-loop

					//check for duplicate vertices
				bool vertAlreadyExists = false;
				if (totalVerts >= 3) //make sure there's at least one triangle to check
				{
					//loop through all the verticies
					for (int iCheck = 0; iCheck < totalVerts; iCheck)
					{
						//if the vertex position and tex coord in memory are the same as the ones currently being read then 
						// this faces vertex index is set to the vertex's value in memory. 
						if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
						{
							if (vertTCIndexTemp == vertTCIndex[iCheck])
							{
								indices.push_back(iCheck);	//set the index for this vertex
								vertAlreadyExists = true;
							}
						}
					} //end loop through verticies
				} //end duplicate test

				if (!vertAlreadyExists)
				{
					vertPosIndex.push_back(vertPosIndexTemp);
					vertTCIndex.push_back(vertTCIndexTemp);
					vertNormIndex.push_back(vertNormIndexTemp);
					totalVerts++;						//new vertex created
					indices.push_back(totalVerts - 1);	//set index for this vertex
				}

				//set the second vertex for the next triangle to the last vertex
				lastVIndex = indices[vIndex]; //the last vertex index of this triangle

				totalTriangles++; //new triangle defined
				vIndex++;
			} //end for l-loop
		}
		break;
		case L'm': //mtllib - material library filename
		{
			std::wstring word_m = L"tllib ";
			bool test_m = true;
			for (int i = 0; i < 6; i++) {	//loop over 'tllib '
				checkChar = fileIn.get();
				if (checkChar != word_m[i])
				{
					test_m = false;
					break;
				}
			}
			if (!test_m)
				break;
			fileIn >> meshMatLib;			//store the material library file name 
		}
		break;
		case L'u': //usemtl - which material to use
		{
			std::wstring word_u = L"semtl ";
			bool test_u = true;
			for (int i = 0; i < 6; i++) {	//loop over 'semtl '
				checkChar = fileIn.get();
				if (checkChar != word_u[i])
				{
					test_u = false;
					break;
				}
			}
			if (!test_u)
				break;
			meshMaterialsTemp = L"";					//Clear the temporary wide string
			fileIn >> meshMaterialsTemp;				//store the material name as a wide string
			meshMaterials.push_back(meshMaterialsTemp); //store it in a vectore of wide strings
		}
		break;
		default:
			break;
		} //end switch .obj file
	} //end while .obj file

	object.meshSubsetIndexStart.push_back(vIndex); //set index start

	//set default tex coords and norms if those are not specified in the file
	if (!hasNorm)
		vertNorm.push_back(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	if (!hasTexCoord)
		vertTexCoord.push_back(DirectX::XMFLOAT2(0.0f, 0.0f));

	//----------------------------------------- Material -------------------------------------------------------------------

	bool mtlFileAlreadyLoaded = false;
	for (size_t i = 0; i < mMaterialFileNameArray.size(); i++)
	{
		if (meshMatLib.compare(mMaterialFileNameArray[i]) == 0)
		{
			mtlFileAlreadyLoaded = true;
		}
	}

	// Only parses the file if it hasn't already been loaded
	if (!mtlFileAlreadyLoaded)
	{
		mMaterialFileNameArray.push_back(meshMatLib);
		//close the obj file and open the mtl file (if it exists)
		fileIn.close();
		fileIn.open(meshMatLib.c_str());

		int matCount = mMaterialArray.size(); //total number of materials


		if (!fileIn) //early exit if the material file doesn't open
		{
			OutputDebugString(L"\nObjectHandler::LoadObjectModel() Failed to open .mtl file\n\n");
			exit(-1);
		}
		while (fileIn)
		{
			checkChar = fileIn.get();
			switch (checkChar)
			{
			case L'#': //comment line
			{
				while (checkChar != L'\n')
					checkChar = fileIn.get();
			}
			break;
			case L'K': //Ks - specular color
			{
				checkChar = fileIn.get();
				if (checkChar == L's')		// Ks - Specular color
				{
					checkChar = fileIn.get(); //read over space

					fileIn >> mMaterialArray[matCount - 1].Data.SpecularColor.x;
					fileIn >> mMaterialArray[matCount - 1].Data.SpecularColor.y;
					fileIn >> mMaterialArray[matCount - 1].Data.SpecularColor.z;
				}
				else if (checkChar == L'd') // Kd - Diffuse color
				{
					checkChar = fileIn.get(); //read over space

					fileIn >> mMaterialArray[matCount - 1].Data.DiffuseColor.x;
					fileIn >> mMaterialArray[matCount - 1].Data.DiffuseColor.y;
					fileIn >> mMaterialArray[matCount - 1].Data.DiffuseColor.z;
				}
			}
			break;
			case L'N': //Ns - specular power
			{
				checkChar = fileIn.get();
				if (checkChar == L's')
				{
					checkChar = fileIn.get(); //read over space

					fileIn >> mMaterialArray[matCount - 1].Data.SpecularPower;
				}
			}
			break;
			case L'm': //map_Kd - texture filename
			{
				std::wstring word_m = L"ap_Kd ";
				bool test_m = true;
				//loop over "ap_Kd "
				for (int i = 0; i < 6; i++)
				{
					checkChar = fileIn.get();
					if (checkChar != word_m[i])
					{
						test_m = false;
						break;
					}
				}
				if (!test_m) //early exit if the letter following 'm' aren't "ap_Kd "
					break;

				mMaterialArray[matCount - 1].Data.HasTexture = 1;

				std::wstring fileNamePath;
				bool texFilePathEnd = false;
				//read the filename
				while (!texFilePathEnd)
				{
					checkChar = fileIn.get();
					fileNamePath += checkChar;
					//stops reading after the filename extension
					if (checkChar == L'.')
					{
						for (int i = 0; i < 3; ++i)
							fileNamePath += fileIn.get();

						texFilePathEnd = true;
					}
				}
				//check if this texture has already been loaded
				bool alreadyLoaded = false;
				for (unsigned int i = 0; i < mTextureNameArray.size(); ++i)
				{
					if (fileNamePath == mTextureNameArray[i])
					{
						alreadyLoaded = true;
						mMaterialArray[matCount - 1].Data.TexArrIndex = i;
					}
				}
				//load the texture
				if (!alreadyLoaded)
				{
					ID3D11Resource* tempTexture;
					ID3D11ShaderResourceView* tempSRV;
					HRESULT hr = DirectX::CreateDDSTextureFromFile(Dev, DevCon, const_cast<wchar_t*>(fileNamePath.c_str()), &tempTexture, &tempSRV);

					if (FAILED(hr))
					{
						OutputDebugString(L"\nObjectHandler::LoadObjectModel() Failed to create DDS texture \n\n");
						exit(-1);
					}
					else
					{
						mMaterialArray[matCount - 1].Data.TexArrIndex = mMeshTextureSRV.size();
						mTextureNameArray.push_back(fileNamePath);
						mMeshTextureSRV.push_back(tempSRV);
					}
				}
			}
			break;
			case L'n': //newmtl - declare new material
			{
				std::wstring word_n = L"ewmtl ";
				bool test_n = true;
				//loop over "ewmtl "
				for (int i = 0; i < 6; i++)
				{
					checkChar = fileIn.get();
					if (checkChar != word_n[i])
					{
						test_n = false;
						break;
					}
				}
				if (test_n)
				{
					materialStruct tempMat;
					mMaterialArray.push_back(tempMat);
					fileIn >> mMaterialArray[matCount].matName;
					mMaterialArray[matCount].Data.TexArrIndex = 0;
					matCount++;
				}
			}
			break;
			default:
				break;
			} //end switch .mtl file
		} //end while .mtl file
	} //end if material file not already loaded


	//set the subset material to the index value of its material in the material array
	for (int i = 0; i < object.nrOfMeshSubsets; i++)
	{
		bool hasMat = false;
		for (size_t j = 0; j < mMaterialArray.size(); j++)
		{
			int int1 = i;
			int int2 = j;
			std::wstring temp1 = meshMaterials[i];
			std::wstring temp2 = mMaterialArray[j].matName;
			if (meshMaterials[i].compare(mMaterialArray[j].matName) == 0)
			{
				object.meshSubsetMaterialIndex.push_back(j);

				hasMat = true;
				break;
			}
		}
		if (!hasMat)
			object.meshSubsetMaterialIndex.push_back(0); //use first material in the array if the subset doesn't have a specified material
	}

	//create vertices
	Vertex tempVert;

	//store the verticies from the file in a vector
	for (int j = 0; j < totalVerts; ++j)
	{
		tempVert.pos = vertPos[vertPosIndex[j]];
		tempVert.texCoord = vertTexCoord[vertTCIndex[j]];
		tempVert.normal = vertNorm[vertNormIndex[j]];

		object.meshVertexData.push_back(tempVert);
	}
	
	//Compute normals (flat shading)
	if (computeNormals)
	{
		DirectX::XMFLOAT3 tempNormal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

		//used to get the vectors (sides) from the position of the verticies
		float vecX, vecY, vecZ;

		//two edges of each triangle
		DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		//compute and set face normals
		for (int i = 0; i < totalTriangles; ++i)
		{
			//get the vector describing one edge of the triangle (edge 0,2)
			vecX = object.meshVertexData[indices[(i * 3)]].pos.x - object.meshVertexData[indices[(i * 3) + 2]].pos.x;
			vecY = object.meshVertexData[indices[(i * 3)]].pos.y - object.meshVertexData[indices[(i * 3) + 2]].pos.y;
			vecZ = object.meshVertexData[indices[(i * 3)]].pos.z - object.meshVertexData[indices[(i * 3) + 2]].pos.z;
			edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f); //first edge

			//get the vector describing one edge of the triangle (edge 2,1)
			vecX = object.meshVertexData[indices[(i * 3) + 2]].pos.x - object.meshVertexData[indices[(i * 3) + 1]].pos.x;
			vecY = object.meshVertexData[indices[(i * 3) + 2]].pos.y - object.meshVertexData[indices[(i * 3) + 1]].pos.y;
			vecZ = object.meshVertexData[indices[(i * 3) + 2]].pos.z - object.meshVertexData[indices[(i * 3) + 1]].pos.z;
			edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f); //second edge

			//Cross multiply to get the un-normalized face normal
			DirectX::XMStoreFloat3(&tempNormal, DirectX::XMVector3Normalize(DirectX::XMVector3Cross(edge2, edge1)));
			//Set the normals of the verticies in the triangle to the face normal
			object.meshVertexData[3 * i].normal = tempNormal;
			object.meshVertexData[3 * i + 1].normal = tempNormal;
			object.meshVertexData[3 * i + 2].normal = tempNormal;

		} //end face normal loop
	} //end computeNormals

	//create vertex and index buffers -------------------------------------------------------------------------

	//index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * totalTriangles * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indicesData;

	indicesData.pSysMem = &indices[0];
	Dev->CreateBuffer(&indexBufferDesc, &indicesData, &object.meshIndexBuffer);

	//vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * totalVerts;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	vertexBufferData.pSysMem = &object.meshVertexData[0];
	Dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &object.meshVertexBuffer);

	//Add object to array of objects
	if (objectType == STATIC_OBJECT)
	{
		mStaticObjects.push_back(object);
	}
	if (objectType == DYNAMIC_OBJECT)
	{
		mDynamicObjects.push_back(object);
	}

	return true;
}

bool ObjectHandler::LoadHeightMap(char* filename, HeightMapInfo &hminfo)
{
	FILE *fileptr;
	BITMAPFILEHEADER bitmapFileH;
	BITMAPINFOHEADER bitmapInfoh;
	int imageSize, index;
	unsigned char height;

	//open and load file
	fopen_s(&fileptr, filename, "rb");
	if (fileptr == NULL) {
		return false;
	}
	fread(&bitmapFileH, sizeof(BITMAPFILEHEADER), 1, fileptr);
	fread(&bitmapInfoh, sizeof(BITMAPINFOHEADER), 1, fileptr);

	//size of the image
	hminfo.worldWidth = bitmapInfoh.biWidth;
	hminfo.worldHeight = bitmapInfoh.biHeight;
	imageSize = hminfo.worldWidth*hminfo.worldHeight * 3;

	//read values from file into array for generation
	unsigned char* bitmapImage = new unsigned char[imageSize];
	fseek(fileptr, bitmapFileH.bfOffBits, SEEK_SET);
	fread(bitmapImage, 1, imageSize, fileptr);
	fclose(fileptr);

	//create array for storing heightvalues, since greyscale only first value relevant, then skip 2
	hminfo.heightMap = new DirectX::XMFLOAT3[hminfo.worldWidth*hminfo.worldHeight];
	int offset = 0;
	float smoothingValue = 10.0f;

	//stores the height values and their respective position
	for (int j = 0; j < hminfo.worldHeight; j++) {
		for (int i = 0; i < hminfo.worldWidth; i++) {
			height = bitmapImage[offset];
			index = (hminfo.worldHeight*j) + i;

			hminfo.heightMap[index] = 
				DirectX::XMFLOAT3(
					static_cast<float>(i), 
					static_cast<float>(height) / smoothingValue, 
					static_cast<float>(j));
			offset += 3;
		}
	}
	delete[] bitmapImage;
	bitmapImage = 0;
	return true;
}

void ObjectHandler::CreatePerObjectConstantBuffers(ID3D11Device* Dev)
{
	HRESULT hr;

	D3D11_BUFFER_DESC WBufferDesc;
	WBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	WBufferDesc.ByteWidth = sizeof(cPerObjectBuffer);
	WBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	WBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	WBufferDesc.MiscFlags = 0;
	WBufferDesc.StructureByteStride = 0;


	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = 0;
	InitData.SysMemPitch = 0;

	// All world matrices are set to identity on creation
	DirectX::XMMATRIX worldIdentity = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());

	// Height Map
	DirectX::XMStoreFloat4x4(&mHeightMapWorldBufferData.World, worldIdentity);
	InitData.pSysMem = &mHeightMapWorldBufferData;
	hr = Dev->CreateBuffer(&WBufferDesc, &InitData, &mHeightMapWorldBuffer);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nObjectHandler::CreatePerObjectConstantBuffers() Failed to create buffer for height map\n\n");
		exit(-1);
	}

	// Static Objects
	for (size_t i = 0; i < mStaticObjects.size(); i++)
	{
		DirectX::XMStoreFloat4x4(&mStaticObjects[i].objectBufferData.World, worldIdentity);

		InitData.pSysMem = &mStaticObjects[i].objectBufferData;
		hr = Dev->CreateBuffer(&WBufferDesc, &InitData, &mStaticObjects[i].perObjectWorldBuffer);
		if (FAILED(hr)) 
		{
			OutputDebugString(L"\nObjectHandler::CreatePerObjectConstantBuffers() Failed to create buffer for static object\n\n");
			exit(-1);
		}
	}

	// Dynamic Objects
	for (size_t i = 0; i < mDynamicObjects.size(); i++)
	{
		DirectX::XMStoreFloat4x4(&mDynamicObjects[i].objectBufferData.World, worldIdentity);
		InitData.pSysMem = &mDynamicObjects[i].objectBufferData;
		hr = Dev->CreateBuffer(&WBufferDesc, nullptr, &mDynamicObjects[i].perObjectWorldBuffer);
		if (FAILED(hr)) 
		{
			OutputDebugString(L"\nObjectHandler::CreatePerObjectConstantBuffers() Failed to create buffer for dynamic object\n\n");
			exit(-1);
		}
	}
}

void ObjectHandler::CreateMaterialConstantBuffers(ID3D11Device* Dev)
{
	HRESULT hr;

	D3D11_BUFFER_DESC materialBufferDesc;
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(cMaterialBuffer);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;

	// Set height map material properties
	mHeightMapMaterialBufferData = Materials::Grass;
	mHeightMapMaterialBufferData.HasTexture = 1;

	D3D11_SUBRESOURCE_DATA InitHMData{};
	InitHMData.pSysMem = &mHeightMapMaterialBufferData;

	hr = Dev->CreateBuffer(&materialBufferDesc, &InitHMData, &mHeightMapMaterialBuffer);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nObjectHandler::CreateMaterialConstantBuffers() Failed to create buffer for height map\n\n");
		exit(-1);
	}

	for (size_t i = 0; i < mMaterialArray.size(); i++)
	{
		mMaterialBufferArray.push_back(nullptr);
		hr = Dev->CreateBuffer(&materialBufferDesc, nullptr, &mMaterialBufferArray[i]);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nObjectHandler::CreateMaterialConstantBuffers() Failed to create material buffers \n\n");
			exit(-1);
		}
	}

}
