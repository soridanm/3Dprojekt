#include "ObjectHandler.hpp"


ObjectHandler::ObjectHandler()
{
	gPerObjectBuffer	= nullptr;
	ObjectBufferData	= cPerObjectBuffer();
	gMaterialBuffer		= nullptr;
	gMaterialBufferData = cMaterialBuffer();
	mTextureView		= nullptr;
}

ObjectHandler::~ObjectHandler()
{

}

bool ObjectHandler::SetGeometryPassObjectBuffers()
{
	UINT32 vertexSize = sizeof(float) * 5;
	UINT32 offset = 0;
	UINT32 squareVertexSize = sizeof(float) * 8;

	// set textures and constant buffers
	gDeviceContext->PSSetShaderResources(0, 1, &mTextureView);
	//gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	// HEIGHT-MAP BEGIN ---------------------------------------------------------------------------

	//gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetIndexBuffer(gSquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	gDeviceContext->IASetVertexBuffers(0, 1, &gSquareVertBuffer, &squareVertexSize, &offset);

	// HEIGHT-MAP END ---------------------------------------------------------------------------

	// update per-object buffer to spin cube
	static float rotation = 0.0f;
	//rotation += CUBE_ROTATION_SPEED;

	DirectX::XMStoreFloat4x4(&ObjectBufferData.World, DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rotation)));

	D3D11_MAPPED_SUBRESOURCE worldMatrixPtr;
	gDeviceContext->Map(gPerObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &worldMatrixPtr);
	// copy memory from CPU to GPU of the entire struct
	memcpy(worldMatrixPtr.pData, &ObjectBufferData, sizeof(cPerObjectBuffer));
	// Unmap constant buffer so that we can use it again in the GPU
	gDeviceContext->Unmap(gPerObjectBuffer, 0);
	// set resource to Geometry Shader
	gDeviceContext->GSSetConstantBuffers(1, 1, &gPerObjectBuffer);

	// Map material properties buffer

	//SetMaterial(Materials::Black_plastic);
	gMaterialBufferData.material = Materials::Black_plastic;

	D3D11_MAPPED_SUBRESOURCE materialPtr;
	gDeviceContext->Map(gMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &materialPtr);
	memcpy(materialPtr.pData, &gMaterialBufferData, sizeof(cMaterialBuffer));
	//gDeviceContext->Unmap(mPerFrameBuffer, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	return true;
}

// private ---------------------------------------------------------------------------------------

bool ObjectHandler::LoadHeightMap(char* filename, HeightMapInfo &hminfo)
{
	FILE *fileptr;
	BITMAPFILEHEADER bitmapFileH;
	BITMAPINFOHEADER bitmapInfoh;
	int imageSize, index;
	unsigned char height;

	//open and load file
	fileptr = fopen(filename, "rb");
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

			hminfo.heightMap[index] = DirectX::XMFLOAT3(i, static_cast<float>(height) / smoothingValue, j);
			offset += 3;
		}
	}
	delete[] bitmapImage;
	bitmapImage = 0;
	return true;
}

void ObjectHandler::CreateWorld()
{
	using DirectX::operator/;

	//creating what is needed for the heightmap
	HeightMapInfo hminfo;
	LoadHeightMap("heightmap.bmp", hminfo);
	int columns = hminfo.worldWidth;
	int rows = hminfo.worldHeight;

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
			mapVertex[i*columns + j].texCoord = DirectX::XMFLOAT2((texUIndex + 0.0f) / rows, (texVIndex + 1.0f) / rows);
			drawOrder[k + 1] = (1 + i)*columns + j;//top left
			mapVertex[(1 + i)*columns + j].texCoord = DirectX::XMFLOAT2((texUIndex + 0.0f) / rows, (texVIndex + 0.0f) / rows);
			drawOrder[k + 2] = i*columns + j + 1;//bottom right
			mapVertex[i*columns + j + 1].texCoord = DirectX::XMFLOAT2((texUIndex + 1.0f) / rows, (texVIndex + 1.0f) / rows);


			drawOrder[k + 3] = (1 + i)*columns + j + 1;//top right
			mapVertex[(1 + i)*columns + j + 1].texCoord = DirectX::XMFLOAT2((texUIndex + 1.0f) / rows, (texVIndex + 0.0f) / rows);
			drawOrder[k + 4] = i*columns + j + 1;//bottom right
			mapVertex[i*columns + j + 1].texCoord = DirectX::XMFLOAT2((texUIndex + 1.0f) / rows, (texVIndex + 1.0f) / rows);
			drawOrder[k + 5] = (1 + i)*columns + j;//top left
			mapVertex[(1 + i)*columns + j].texCoord = DirectX::XMFLOAT2((texUIndex + 0.0f) / rows, (texVIndex + 0.0f) / rows);

			k += 6;
			texUIndex++;
		}
		texUIndex = 0;
		texVIndex++;
	}

	//calculates the normal for each face
	std::vector<DirectX::XMFLOAT3> tempNormal;
	DirectX::XMFLOAT3 nonNormalized = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	float vecX, vecY, vecZ;
	DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	for (int i = 0; i < NUMBER_OF_FACES; i++) {
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

	//calculates the average normal in order to make the world smooth
	DirectX::XMVECTOR averageNormal = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	int facesUsing = 0;
	float tx, ty, tz;
	for (int i = 0; i < NUMBER_OF_VERTICES; i++) {
		for (int j = 0; j < NUMBER_OF_FACES; j++) {
			if (drawOrder[j * 3] == i || drawOrder[(j * 3) + 1] == i || drawOrder[(j * 3) + 2] == i) {
				tx = DirectX::XMVectorGetX(averageNormal) + tempNormal[j].x;
				ty = DirectX::XMVectorGetY(averageNormal) + tempNormal[j].y;
				tz = DirectX::XMVectorGetZ(averageNormal) + tempNormal[j].z;

				averageNormal = DirectX::XMVectorSet(tx, ty, tz, 0.0f);
				facesUsing++;
			}
		}
		averageNormal = averageNormal / facesUsing;
		averageNormal = DirectX::XMVector3Normalize(averageNormal);
		mapVertex[i].normal.x = DirectX::XMVectorGetX(averageNormal);
		mapVertex[i].normal.y = DirectX::XMVectorGetY(averageNormal);
		mapVertex[i].normal.z = DirectX::XMVectorGetZ(averageNormal);
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
	gDevice->CreateBuffer(&indexBufferDesc, &iinitData, &gSquareIndexBuffer);

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
	gDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &gSquareVertBuffer);
}


void ObjectHandler::CreatePerObjectConstantBuffer()
{
	DirectX::XMMATRIX world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(0.0f));

	DirectX::XMStoreFloat4x4(&ObjectBufferData.World, world);

	D3D11_BUFFER_DESC WBufferDesc;
	WBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	WBufferDesc.ByteWidth = sizeof(cPerObjectBuffer);
	WBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	WBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	WBufferDesc.MiscFlags = 0;
	WBufferDesc.StructureByteStride = 0;

	gHR = gDevice->CreateBuffer(&WBufferDesc, nullptr, &gPerObjectBuffer);
	if (FAILED(gHR)) {
		exit(-1);
	}
}

void ObjectHandler::CreateMaterialConstantBuffer()
{
	D3D11_BUFFER_DESC materialBufferDesc;
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(cMaterialBuffer);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;

	// check if the creation failed for any reason
	HRESULT hr = 0;
	hr = gDevice->CreateBuffer(&materialBufferDesc, nullptr, &gMaterialBuffer);
	if (FAILED(hr))
	{
		// handle the error, could be fatal or a warning...
		exit(-1);
	}
}
