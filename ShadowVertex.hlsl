/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: ShadowVertex.hlsl
*
* File summary: Vertex shader of the Shadow pass
*	Transforms the vertex positions to clip-space
*/

//TODO: remove normal and texcoord from input as well as the camera position

cbuffer PER_FRAME	:register(b0)
{
	float4x4 viewProjection;
	float4 cameraPosition; //NOT USED IN THIS SHADER
}

cbuffer PER_OBJECT		: register(b1)
{
	float4x4 worldMatrix;
};

//TODO: change input layout since only the position is relevant
struct VS_IN
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal   : NORMAL;
};

struct VS_OUT
{
	float4 PositionCS : SV_Position;
};

VS_OUT VS_main(in VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.PositionCS = mul(mul(float4(input.Position, 1.0), worldMatrix), viewProjection);

	return output;
}