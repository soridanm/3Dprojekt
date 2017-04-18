/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: GBufferVertex.hlsl
*
* File summary: Vertex shader of the Geometry pass
*	Turns the Position coordinates in to homogeneous coordinates.
*	Other than that only works as a pass-through
*/

struct VS_IN
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal   : NORMAL;
};

struct VS_OUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal   : NORMAL;
};

VS_OUT VS_main(in VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.Position = float4(input.Position, 1.0);
	output.TexCoord = input.TexCoord;
	output.Normal   = input.Normal;

	return output;
}