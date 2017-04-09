/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: ShadowVertex.hlsl
*
* File summary: Vertex shader of the Shadow pass
*	Transforms the vertex positions to clip-space
*/


cbuffer PER_FRAME	:register(b0)
{
	float4x4 viewProjection;
	float4 cameraPosition; //not used in this shader
}

cbuffer PER_OBJECT		: register(b1)
{
	float4x4 worldMatrix;
};

struct VS_IN
{
	float3 Position : POSITION;
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