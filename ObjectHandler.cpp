#include "ObjectHandler.hpp"


ObjectHandler::ObjectHandler()
{
	/*gPerObjectBuffer	= nullptr;
	ObjectBufferData	= cPerObjectBuffer();
	gMaterialBuffer		= nullptr;
	gMaterialBufferData = cMaterialBuffer();
	mTextureView		= nullptr;*/
}

ObjectHandler::~ObjectHandler()
{

}

//used in GraphicsHandler.InitializeGraphics
void ObjectHandler::InitializeObjects(ID3D11Device* Dev)
{
	
	CreateWorld(Dev);
	if (!LoadObjectModel(Dev, L"wt_teapot.obj", DYNAMIC_OBJECT, false, true))
	{
		exit(-1);
	}
	if (!LoadObjectModel(Dev, L"wt_teapot.obj", DYNAMIC_OBJECT, false, true))
	{
		exit(-1);
	}
	if (!LoadObjectModel(Dev, L"wt_teapot.obj", STATIC_OBJECT, false, true))
	{
		exit(-1);
	}
	CreatePerObjectConstantBuffers(Dev);
	CreateMaterialConstantBuffers(Dev);
}

//TODO: Make the SetConstantBuffers functions a seperate function that works with both the geometry and the shadow pass
//used in GraphicsHandler.RenderGeometryPass
bool ObjectHandler::SetHeightMapBuffer(ID3D11DeviceContext* DevCon, RenderPassID passID)
{
	UINT32 squareVertexSize = sizeof(float) * 8;
	UINT32 offset = 0;

	//set textures and constant buffers
	if (passID == GEOMETRY_PASS)
	{
		DevCon->PSSetShaderResources(0, 1, &mTextureView);
	}
	//DevCon->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	 //HEIGHT-MAP BEGIN ---------------------------------------------------------------------------

	DevCon->IASetIndexBuffer(gSquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DevCon->IASetVertexBuffers(0, 1, &gSquareVertBuffer, &squareVertexSize, &offset);

	 //HEIGHT-MAP END ---------------------------------------------------------------------------

	// update per-object buffer to spin cube
	static float rotation = 0.0f;
	//rotation += CUBE_ROTATION_SPEED;

	DirectX::XMStoreFloat4x4(&mHeightMapWorldBufferData.World, DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rotation)));

	D3D11_MAPPED_SUBRESOURCE worldMatrixPtr;
	DevCon->Map(mHeightMapWorldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &worldMatrixPtr);
	// copy memory from CPU to GPU of the entire struct
	memcpy(worldMatrixPtr.pData, &mHeightMapWorldBufferData, sizeof(cPerObjectBuffer));
	// Unmap constant buffer so that we can use it again in the GPU
	DevCon->Unmap(mHeightMapWorldBuffer, 0);
	// set resource to Geometry Shader
	if (passID == GEOMETRY_PASS)
	{
		DevCon->GSSetConstantBuffers(1, 1, &mHeightMapWorldBuffer);
	}
	if (passID == SHADOW_PASS)
	{
		DevCon->VSSetConstantBuffers(1, 1, &mHeightMapWorldBuffer);
	}

	// Map material properties buffer

	mHeightMapMaterialBufferData = Materials::Grass;
	mHeightMapMaterialBufferData.HasTexture = 1;

	D3D11_MAPPED_SUBRESOURCE materialPtr;
	DevCon->Map(mHeightMapMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &materialPtr);
	memcpy(materialPtr.pData, &mHeightMapMaterialBufferData, sizeof(cMaterialBuffer));
	DevCon->Unmap(mHeightMapMaterialBuffer, 0);
	if (passID == GEOMETRY_PASS)
	{
		DevCon->PSSetConstantBuffers(0, 1, &mHeightMapMaterialBuffer);
	}
	//new code ----------------------------------------------------------------------------------------------
	
	return true;
}

//TODO: Implement this better for multiple objects
//TODO: Probably a memory leak here
bool ObjectHandler::SetObjectBufferWithIndex(ID3D11DeviceContext* DevCon, RenderPassID passID, ObjectType objectType, int objectIndex, int materialIndex)
{
	//TODO: make std::vector of world matrixes for each object
	UINT32 vertexSize = sizeof(Vertex);
	UINT32 offset = 0;

	std::vector<Object>* objectArray; // pointer to one of the arrays of objects in ObjectHandler
	if (objectType == STATIC_OBJECT)  { objectArray = &mStaticObjects; }
	if (objectType == DYNAMIC_OBJECT) { objectArray = &mDynamicObjects; }

	/*if (objectArray = nullptr)
	{
		OutputDebugString(L"\nObjectHandler::SetObjectBufferWithIndex() Invalid ObjectType\n\n");
		exit(-1);
	}*/

	D3D11_MAPPED_SUBRESOURCE worldMatrixPtr;


	// Set index and vertex buffers
	DevCon->IASetIndexBuffer((*objectArray)[objectIndex].meshIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DevCon->IASetVertexBuffers(0, 1, &(*objectArray)[objectIndex].meshVertexBuffer, &vertexSize, &offset);

	/***************************************************************************************************************************************
													World matrix
	***************************************************************************************************************************************/

	if (objectType == DYNAMIC_OBJECT)
	{
		static float rotation = 0.0f;
		rotation += 0.001f;
		float scale = 10.0f;
		//XMMATRIX rotMatrix = XMMatrixMultiply(XMMatrixRotationX(2), XMMatrixRotationY(rotation));

		using DirectX::operator*;

		DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale, scale, scale);
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(90.0f)) * DirectX::XMMatrixRotationX(rotation);
		DirectX::XMMATRIX locationMatrix = DirectX::XMMatrixTranslation(100.0f + 20.f * (objectIndex + 1.0f), 20.0f * (objectIndex + 1.0f), 100.0f);


		DirectX::XMMATRIX finalMatrix = rotationMatrix * scaleMatrix * locationMatrix;

		XMStoreFloat4x4(&(*objectArray)[objectIndex].objectBufferData.World, DirectX::XMMatrixTranspose(finalMatrix));
	}

	if (objectType == STATIC_OBJECT)
	{
		//TODO: This will be done at initialization so this part will be removed
		// A static object has already had its geometry multiplied by a world-matrix so their shader-side matrix is set to an identity-matrix
		XMStoreFloat4x4(&(*objectArray)[objectIndex].objectBufferData.World, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
	}

//	HRESULT hr;

	DevCon->Map((*objectArray)[objectIndex].perObjectWorldBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &worldMatrixPtr);
	// copy memory from CPU to GPU of the entire struct
	memcpy(worldMatrixPtr.pData, &(*objectArray)[objectIndex].objectBufferData, sizeof(cPerObjectBuffer));
	// Unmap constant buffer so that we can use it again in the GPU
	DevCon->Unmap((*objectArray)[objectIndex].perObjectWorldBuffer, 0);
	

	// The geometry pass computes the transformations in the Geometry shader and the shadow pass computes the transformations in the Vertex shader
	if (passID == GEOMETRY_PASS) { DevCon->GSSetConstantBuffers(1, 1, &(*objectArray)[objectIndex].perObjectWorldBuffer); }
	if (passID == SHADOW_PASS)   { DevCon->VSSetConstantBuffers(1, 1, &(*objectArray)[objectIndex].perObjectWorldBuffer); }



	/***************************************************************************************************************************************
															Texture
	***************************************************************************************************************************************/

	// REMOVE AFTER TEXTURES HAVE BEEN IMPLEMENTED ----------------------------------------------------
	materialVector[(*objectArray)[objectIndex].meshSubsetTexture[materialIndex]].Data.HasTexture = 0;
	// END REMOVE -------------------------------------------------------------------------------------
	
	(*objectArray)[objectIndex].materialBufferData.HasTexture = materialVector[(*objectArray)[objectIndex].meshSubsetTexture[materialIndex]].Data.HasTexture;

	if (materialVector[(*objectArray)[objectIndex].meshSubsetTexture[materialIndex]].Data.HasTexture == 1)
	{
		DevCon->PSSetShaderResources(0, 1, &(*objectArray)[objectIndex].meshTextureSRV[materialIndex]); // NOT IMPLEMENTED YET!!
	}

	/***************************************************************************************************************************************
															Material
	***************************************************************************************************************************************/

	if (passID == GEOMETRY_PASS)
	{
		//set material
		(*objectArray)[objectIndex].materialBufferData.SpecularColor = materialVector[(*objectArray)[objectIndex].meshSubsetTexture[materialIndex]].Data.SpecularColor;
		(*objectArray)[objectIndex].materialBufferData.SpecularPower = materialVector[(*objectArray)[objectIndex].meshSubsetTexture[materialIndex]].Data.SpecularPower;
		(*objectArray)[objectIndex].materialBufferData.DiffuseColor  = materialVector[(*objectArray)[objectIndex].meshSubsetTexture[materialIndex]].Data.DiffuseColor;
		
		// Map material properties buffer
		D3D11_MAPPED_SUBRESOURCE materialPtr;
		DevCon->Map((*objectArray)[objectIndex].materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &materialPtr);
		memcpy(materialPtr.pData, &(*objectArray)[objectIndex].materialBufferData, sizeof(cMaterialBuffer));
		DevCon->Unmap((*objectArray)[objectIndex].materialBuffer, 0);

		DevCon->PSSetConstantBuffers(0, 1, &(*objectArray)[objectIndex].materialBuffer);
	}

	return true;
}

//used in GraphicsHandler.RenderGeometryPass
const int ObjectHandler::GetHeightMapNrOfFaces()
{
	return NUMBER_OF_FACES;
}

//used in ????
int ObjectHandler::GetHeightMapNrOfVerticies()
{
	return NUMBER_OF_VERTICES;
}

std::vector<Object>* ObjectHandler::GetObjectArrayPtr(ObjectType objectType)
{
	return &((objectType == STATIC_OBJECT) ? mStaticObjects : mDynamicObjects);
}

//int ObjectHandler::GetNrOfMeshSubsets()
//{
//	return meshSubsets;
//}

// private ---------------------------------------------------------------------------------------

//TODO: Split into multiple functions
bool ObjectHandler::LoadObjectModel(
	ID3D11Device* Dev,
	std::wstring filename,
	ObjectType objectType, 
	bool isRHCoordSys,
	bool computeNormals)
{
	std::wifstream fileIn(filename.c_str());    //Open file
	std::wstring meshMatLib;                    //String to hold our obj material library filename

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

	//Make sure we have a default if no tex coords or normals are defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	//Temp variables to store into vectors
	std::wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;        //The variable we will use to store one char from file at a time
	std::wstring face;        //Holds the string containing our face vertices
	int vIndex = 0;           //Keep track of our vertex index count
	int triangleCount = 0;    //Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	Object object;

	//TODO: improve with an error message or something
	if (!fileIn)	//Early exit if the file doesn't open
		exit(-1);

	bool debugBool = false; //TODO: REMOVE!

	// .obj file
	while (fileIn)
	{
		checkChar = fileIn.get();			//get the next char
		switch (checkChar)
		{
		case '#': //Comment line
		{
			while (checkChar != '\n')		//Keep reading chars until the line ends
				checkChar = fileIn.get();
		}
		break;
		case 'v': //Get vertex descriptions
		{
			checkChar = fileIn.get();
			if (checkChar == ' ')			//v - vertex position
			{
				float vx, vy, vz;
				fileIn >> vx >> vy >> vz;	//store the vertex positions
				vertPos.push_back(DirectX::XMFLOAT3(vx, vy, (isRHCoordSys) ? vz * -1.0f : vz));		//invert Z-axiz if the OBJ is RH
			}
			if (checkChar == 't')			//vt - vertex texture coordinates
			{
				float vu, vv;
				fileIn >> vu >> vv;			//store the uv-coordinates
				vertTexCoord.push_back(DirectX::XMFLOAT2(vu, (isRHCoordSys) ? 1.0f - vv : vv));		//Reverse the v-axis if the OBJ is RH
				hasTexCoord = true;
			}
			if (checkChar == 'n')			//vn - vertex normal
			{
				float vnx, vny, vnz;
				fileIn >> vnx >> vny >> vnz; //store the normal values
				vertNorm.push_back(DirectX::XMFLOAT3(vnx, vny, (isRHCoordSys) ? -1.0f * vnz : vnz)); //Invert the z-axiz if the OBJ is RH
				hasNorm = true;
			}
		}
		break;
		case 'g': //g - group
		{
			checkChar = fileIn.get();
			if (checkChar == ' ')
			{
				object.meshSubsetIndexStart.push_back(vIndex);
				object.nrOfMeshSubsets++;
				while (checkChar != '\n')		//Keep reading chars until the line ends
					checkChar = fileIn.get();
			}
		}
		break;
		case 'f': //f - faces. Vertex definition [vPos1/vTexCoord1/vNorm1 ... ]
		{									//Breaks faces with more than three sides into multiple triangles
			checkChar = fileIn.get();
			if (checkChar != ' ')	//early exit if the next character isn't a space
				break;

			face = L"";				//clear the temporary wide string which will be used to store the line
			std::wstring VertDef;	//holds one vertex definition at a time
			triangleCount = 0;

			checkChar = fileIn.get();
			while (checkChar != '\n')	//read through the whole line
			{
				face += checkChar;		//add every char to the face string
				checkChar = fileIn.get();
				if (checkChar == ' ')
					triangleCount++;	//increase triangle count if it's a space
			}

			//check for space at the end of our face string
			if (face[face.length() - 1] == ' ')
				triangleCount--;
			triangleCount -= 1; //every vertex in the face after the first two are new faces

			std::wstringstream ss(face);
			if (face.length() <= 0) // early exit if the line isn't just simply 'f '
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
					if (VertDef[j] != '/')		//if there's no divider add char to vertPart
						vertPart += VertDef[j];
					//If the current char is a divider or the last character in the string
					if (VertDef[j] == '/' || j == VertDef.length() - 1)
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
				//avoid duplicate vertices
				bool vertAlreadyExists = false;
				if (totalVerts >= 3) //make sure there's at least one triangle to check
				{
					//loop through all the verticies
					for (int iCheck = 0; iCheck < totalVerts; ++iCheck)
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
			meshTriangles++; //one triangle down

							 //convert the face to more than one triangle in case the face has more than three vertexes
			for (int l = 0; l < triangleCount - 1; ++l)
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

				//TODO: see if this duplicate code can be eliminated
				//parse this string (same procedure as in the for i-loop)
				for (unsigned int j = 0; j < VertDef.length(); j++)
				{
					if (VertDef[j] != '/')		//if there's no divider add char to vertPart
						vertPart += VertDef[j];
					//If the current char is a divider or the last character in the string
					if (VertDef[j] == '/' || j == VertDef.length() - 1)
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

				meshTriangles++; //new triangle defined
				vIndex++;
			} //end for l-loop
		}
		break;
		case 'm': //mtllib - material library filename
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
		case 'u': //usemtl - which material to use
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
		case 's': //s - smoothing group [NOT SUPPORTED]
			break;
		default:
			break;
		} //end switch .obj file
	} //end while .obj file

	object.meshSubsetIndexStart.push_back(vIndex); //set index start

										//make sure the first subset does not contain "0" indices. Can happen if "g" is defined at the very top of the file
	if (object.meshSubsetIndexStart[1] == 0)
	{
		object.meshSubsetIndexStart.erase(object.meshSubsetIndexStart.begin() + 1);
		object.nrOfMeshSubsets--;
	}

	//set default tex coords and norms if those are not specified in the file
	if (!hasNorm)
		vertNorm.push_back(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
	if (!hasTexCoord)
		vertTexCoord.push_back(DirectX::XMFLOAT2(0.0f, 0.0f));


	//----------------------------------------- Material -------------------------------------------------------------------
	// Does not take ambient and diffuse color into account since all objects will be textured.

	//close the obj file and open the mtl file (if it exists)
	fileIn.close();
	fileIn.open(meshMatLib.c_str());

	std::wstring lastStringRead; // ????? This is never used
	int matCount = materialVector.size(); //total materials

									//bool kdset = false; //if diffuse was not set, use ambient. if diffuse WAS set, no need to set diffuse to amb.

	if (!fileIn) //early exit if the material file doesn't open
		exit(-1);
	while (fileIn)
	{
		checkChar = fileIn.get();
		switch (checkChar)
		{
		case '#': //comment
		{
			while (checkChar != '\n')
				checkChar = fileIn.get();
		}
		break;
		case 'K': //Ks - specular color
		{
			checkChar = fileIn.get();
			if (checkChar == 's') // Ks - specular color
			{
				checkChar = fileIn.get(); //read over space

				fileIn >> materialVector[matCount - 1].Data.SpecularColor.x;
				fileIn >> materialVector[matCount - 1].Data.SpecularColor.y;
				fileIn >> materialVector[matCount - 1].Data.SpecularColor.z;
			}
			else if (checkChar == 'd')
			{
				checkChar = fileIn.get(); //read over space

				fileIn >> materialVector[matCount - 1].Data.DiffuseColor.x;
				fileIn >> materialVector[matCount - 1].Data.DiffuseColor.y;
				fileIn >> materialVector[matCount - 1].Data.DiffuseColor.z;
			}
		}
		break;
		case 'N': //Ns - specular power
		{
			checkChar = fileIn.get();
			if (checkChar == 's')
			{
				checkChar = fileIn.get(); //read over space

				fileIn >> materialVector[matCount - 1].Data.SpecularPower;
			}
		}
		break;
		case 'm': //map_Kd - texture file
		{
			std::wstring word_m = L"ap_Kd";
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

			std::wstring fileNamePath;
			bool texFilePathEnd = false;
			//read the filename
			while (!texFilePathEnd)
			{
				checkChar = fileIn.get();
				fileNamePath += checkChar;

				//stops reading after the filename extension (.png for example)
				if (checkChar == '.')
				{
					for (int i = 0; i < 3; ++i)
						fileNamePath += fileIn.get();

					texFilePathEnd = true;
				}
			}
			//check if this texture has already been loaded
			bool alreadyLoaded = false;
			for (unsigned int i = 0; i < object.textureNameArray.size(); ++i)
			{
				if (fileNamePath == object.textureNameArray[i])
				{
					alreadyLoaded = true;
					materialVector[matCount - 1].Data.TexArrIndex = i;
				}
			}
			//load the texture
			if (!alreadyLoaded)
			{
				// TEXTURE SUPPORT NOT IPMLEMENTED YET

				//ID3D11ShaderResourceView* tempMeshSRV;
				//// TODO: D3DX11CreateShaderResourceViewFromFile

				//if (SUCCEEDED(gHR))
				//{
				//	textureNameArray.push_back(fileNamePath.c_str());
				//	materialVector[matCount - 1].texArrayIndex = meshSRV.size();
				//	meshSRV.push_back(tempMeshSRV);
				//}
			}
		}
		break;
		case 'n': //newmtl - declare new material
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
				materialVector.push_back(tempMat);
				//fileIn >> materialVector[matCount].matName;
				materialVector[matCount].Data.TexArrIndex = 0;
				matCount++;
			}
		}
		break;
		default:
			break;
		} //end switch .mtl file
	} //end while .mtl file



	  //set the subset material to the index value of its material in the material array
	for (int i = 0; i < object.nrOfMeshSubsets; ++i)
	{
		bool hasMat = false;
		for (unsigned int j = 0; j < materialVector.size(); ++j)
		{
			if (meshMaterials[i] == materialVector[j].matName)
			{
				object.meshSubsetTexture.push_back(j);
				hasMat = true;
			}
		}
		if (!hasMat)
			object.meshSubsetTexture.push_back(0); //use first material in the array if the subset doesn't have a specified material
	}

	//create vertices
	std::vector<Vertex> verticies;
	Vertex tempVert;

	//store the verticies from the file in a vector
	for (int j = 0; j < totalVerts; ++j)
	{
		tempVert.pos = vertPos[vertPosIndex[j]];
		tempVert.texCoord = vertTexCoord[vertTCIndex[j]];
		tempVert.normal = vertNorm[vertNormIndex[j]];

		verticies.push_back(tempVert);
	}

	//Compute face normals
	if (computeNormals)
	{
		std::vector<DirectX::XMFLOAT3> tempNormal;

		//normalized and unnormalized normals
		DirectX::XMFLOAT3 unnormalized = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

		//used to get the vectors (sides) from the position of the verticies
		float vecX, vecY, vecZ;

		//two edges of a triangle
		DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		//compute face normals
		for (int i = 0; i < meshTriangles; ++i)
		{
			//get the vector describing one edge of the triangle (edge 0,2)
			vecX = verticies[indices[(i * 3)]].pos.x - verticies[indices[(i * 3) + 2]].pos.x;
			vecY = verticies[indices[(i * 3)]].pos.y - verticies[indices[(i * 3) + 2]].pos.y;
			vecZ = verticies[indices[(i * 3)]].pos.z - verticies[indices[(i * 3) + 2]].pos.z;
			edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f); //first edge

														 //get the vector describing one edge of the triangle (edge 2,1)
			vecX = verticies[indices[(i * 3) + 2]].pos.x - verticies[indices[(i * 3) + 1]].pos.x;
			vecY = verticies[indices[(i * 3) + 2]].pos.y - verticies[indices[(i * 3) + 1]].pos.y;
			vecZ = verticies[indices[(i * 3) + 2]].pos.z - verticies[indices[(i * 3) + 1]].pos.z;
			edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f); //second edge

														 //Cross multiply to get the un-normalized face normal
			DirectX::XMStoreFloat3(&unnormalized, DirectX::XMVector3Cross(edge2, edge1));
			tempNormal.push_back(unnormalized); //save unnormalized normal (for normal averaging)
		} //end face normal loop

		  //compute vertex normals (normal averaging)
		DirectX::XMVECTOR normalSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		int facesUsing = 0;
		float tX, tY, tZ;

		//loop through each vertex
		for (int i = 0; i < totalVerts; ++i)
		{
			//check which triangles are using this vertex
			for (int j = 0; j < meshTriangles; ++j)
			{
				if (indices[j * 3] == i || indices[(j * 3) + 1] == i || indices[(j * 3) + 2] == i)
				{
					tX = DirectX::XMVectorGetX(normalSum) + tempNormal[j].x;
					tY = DirectX::XMVectorGetY(normalSum) + tempNormal[j].y;
					tZ = DirectX::XMVectorGetZ(normalSum) + tempNormal[j].z;

					normalSum = DirectX::XMVectorSet(tX, tY, tZ, 0.0f); //add the unnormalized face normal to the normal sum
					facesUsing++;
				}
			} //end for j-loop through each triangle

			using DirectX::operator/;
			  //divide the unnormalized normal by the number of faces sharing the vertex and the normalize it to get the actual normal
			normalSum = DirectX::XMVector3Normalize(normalSum / static_cast<float>(facesUsing));

			//store it in the current vertex
			DirectX::XMStoreFloat3(&verticies[i].normal, normalSum);

			//clear normalSum and facesUsing
			normalSum = DirectX::XMVectorZero();
			facesUsing = 0;
		} //end for i-loop through each vertex
	} //end computeNormals

	  //create vertex and index buffers -------------------------------------------------------------------------


	  //index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
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

	vertexBufferData.pSysMem = &verticies[0];
	Dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &object.meshVertexBuffer);

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

//used in CreateWorld
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

			hminfo.heightMap[index] = DirectX::XMFLOAT3(static_cast<float>(i), static_cast<float>(height) / smoothingValue, static_cast<float>(j));
			offset += 3;
		}
	}
	delete[] bitmapImage;
	bitmapImage = 0;
	return true;
}

//used in InitializeObjects
void ObjectHandler::CreateWorld(ID3D11Device* Dev)
{
	using DirectX::operator/;

	//creating what is needed for the heightmap
	HeightMapInfo hminfo;
	LoadHeightMap("heightmap.bmp", hminfo);
	unsigned int columns = hminfo.worldWidth;
	unsigned int rows = hminfo.worldHeight;

	NUMBER_OF_VERTICES = rows*columns;
	NUMBER_OF_FACES = (rows - 1)*(columns - 1) * 2;

	std::vector<Vertex> mapVertex(NUMBER_OF_VERTICES);

	for (DWORD i = 0; i < rows; i++) {
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
			texUIndex++;
		}
		texUIndex = 0;
		texVIndex++;
	}

	// Change the scale of the texture.
	// Definitely not the best (nor the most elegant) way to do it but at least it works
	DirectX::XMVECTOR temp;
	for (unsigned int i = 0; i < NUMBER_OF_VERTICES; i++)
	{
		using DirectX::operator*;
		temp = DirectX::XMLoadFloat2(&mapVertex[i].texCoord);
		DirectX::XMStoreFloat2(&mapVertex[i].texCoord, temp * 0.05f);
	}

	//calculates the normal for each face
	std::vector<DirectX::XMFLOAT3> tempNormal;
	DirectX::XMFLOAT3 nonNormalized = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	float vecX, vecY, vecZ;
	DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	for (unsigned int i = 0; i < NUMBER_OF_FACES; i++) {
		vecX = mapVertex[drawOrder[(i * 3)]].pos.x - mapVertex[drawOrder[(i * 3) + 2]].pos.x;
		vecY = mapVertex[drawOrder[(i * 3)]].pos.y - mapVertex[drawOrder[(i * 3) + 2]].pos.y;
		vecZ = mapVertex[drawOrder[(i * 3)]].pos.z - mapVertex[drawOrder[(i * 3) + 2]].pos.z;
		edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

		vecX = mapVertex[drawOrder[(i * 3) + 2]].pos.x - mapVertex[drawOrder[(i * 3) + 1]].pos.x;
		vecY = mapVertex[drawOrder[(i * 3) + 2]].pos.y - mapVertex[drawOrder[(i * 3) + 1]].pos.y;
		vecZ = mapVertex[drawOrder[(i * 3) + 2]].pos.z - mapVertex[drawOrder[(i * 3) + 1]].pos.z;
		edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

		XMStoreFloat3(&nonNormalized, DirectX::XMVector3Cross(edge2, edge1));
		tempNormal.push_back(nonNormalized);
	}

	//this part is still really slow. NOTE: Obly in Debug configurations
	//calculates the average normal in order to make the world smooth
	DirectX::XMVECTOR averageNormal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 avgNorm = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	unsigned int facesUsing = 0;

	for (unsigned int i = 0; i < NUMBER_OF_VERTICES; i++) {
		for (unsigned int j = 0; j < NUMBER_OF_FACES; j++) {
			if (drawOrder[j * 3] == i || drawOrder[(j * 3) + 1] == i || drawOrder[(j * 3) + 2] == i) {
				avgNorm.x += tempNormal[j].x;
				avgNorm.y += tempNormal[j].y;
				avgNorm.z += tempNormal[j].z;

				facesUsing++;
			}
		}
		avgNorm.x /= facesUsing;
		avgNorm.y /= facesUsing;
		avgNorm.z /= facesUsing;

		averageNormal = DirectX::XMLoadFloat3(&avgNorm);
		averageNormal = DirectX::XMVector3Normalize(averageNormal);

		DirectX::XMStoreFloat3(&mapVertex[i].normal, averageNormal);

		averageNormal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		facesUsing = 0;
	}

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

//used in InitializeObjects
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

	DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());

	// Height Map
	DirectX::XMStoreFloat4x4(&mHeightMapWorldBufferData.World, world);
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
		DirectX::XMStoreFloat4x4(&mStaticObjects[i].objectBufferData.World, world);
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
		DirectX::XMStoreFloat4x4(&mDynamicObjects[i].objectBufferData.World, world);
		InitData.pSysMem = &mDynamicObjects[i].objectBufferData;
		hr = Dev->CreateBuffer(&WBufferDesc, nullptr, &mDynamicObjects[i].perObjectWorldBuffer);
		if (FAILED(hr)) 
		{
			OutputDebugString(L"\nObjectHandler::CreatePerObjectConstantBuffers() Failed to create buffer for dynamic object\n\n");
			exit(-1);
		}
	}
}

//used in InitializeObjects
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

	// Height Map
	hr = Dev->CreateBuffer(&materialBufferDesc, nullptr, &mHeightMapMaterialBuffer);
	if (FAILED(hr))
	{
		OutputDebugString(L"\nObjectHandler::CreateMaterialConstantBuffers() Failed to create buffer for height map\n\n");
		exit(-1);
	}

	// Static Objects
	for (size_t i = 0; i < mStaticObjects.size(); i++)
	{
		hr = Dev->CreateBuffer(&materialBufferDesc, nullptr, &mStaticObjects[i].materialBuffer);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nObjectHandler::CreateMaterialConstantBuffers() Failed to create buffer for static object\n\n");
			exit(-1);
		}
	}

	// Dynamic Objects
	for (size_t i = 0; i < mDynamicObjects.size(); i++)
	{
		hr = Dev->CreateBuffer(&materialBufferDesc, nullptr, &mDynamicObjects[i].materialBuffer);
		if (FAILED(hr))
		{
			OutputDebugString(L"\nObjectHandler::CreateMaterialConstantBuffers() Failed to create buffer for dynamic object\n\n");
			exit(-1);
		}
	}

}




