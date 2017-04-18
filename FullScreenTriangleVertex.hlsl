/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: FullScreenTriangleVertex.hlsl
*
* File summary: Vertex shader used in both the Light pass and the Screen pass
*	Generates a single triangle that covers the entire screen.
*/

struct VS_OUT
{
	float4 PositionCS	: SV_Position;
};

// Generates a full screen triangle using vertexID (= 0, 1, 2)
VS_OUT VS_main(in uint vertexID : SV_VertexID)
{
	VS_OUT output = (VS_OUT)0;

	// clip space positions: (-1,-1,0), (-1,3,0), (3,-1,0)
	float x = (float)(vertexID / 2) * 4.0 - 1.0;
	float y = (float)(vertexID % 2) * 4.0 - 1.0;

	output.PositionCS = float4(x, y, 0.0, 1.0);

	return output;
}