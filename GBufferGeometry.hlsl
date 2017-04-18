/**
* Course: DV1542 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: GBufferGeometry.hlsl
*
* File summary: Geometry Shader of the Geometry pass
*	Does an eatly backface culling test.
*	Transforms all position coordinates to world space and clip space.
*	Transforms normals to world space. NOTE: Does not correctly transform 
*	normals correctly if non-uniform scaling has been applied on the geometry.
*
*	1. transform first vertex and backface cull
*	2. loop through second and third vertex
*
*	WS - world space
*	CS - clip space
*/


cbuffer PER_FRAME		: register(b0)
{
	float4x4 viewProjection;
	float4   cameraPosition;
};

cbuffer PER_OBJECT		: register(b1)
{
	float4x4 worldMatrix;
};

struct GS_IN
{
	float4 Position		: POSITION;
	float2 TexCoord		: TEXCOORD;
	float3 Normal       : NORMAL;
};

struct GS_OUT
{
	float4 PositionCS	: SV_Position;
	float2 TexCoord		: TEXCOORD;
	float3 NormalWS		: NORMALWS;
	float3 PositionWS	: POSITIONWS;
};

[maxvertexcount(6)]
void GS_main(triangle GS_IN input[3], inout TriangleStream <GS_OUT> outStream)
{
	GS_OUT output = (GS_OUT)0;

	/* Calculate un-normalized face normal in world space */
	float3 face_normal = mul(cross(
		input[0].Position.xyz - input[1].Position.xyz,
		input[0].Position.xyz - input[2].Position.xyz), (float3x3)worldMatrix);

	// Transform first vertex and backface cull -------------------------------

	// Convert the first vertex position to world space
	output.PositionWS = mul(input[0].Position, worldMatrix).xyz;

	// Cull if the face normal is pointing in the opposite direction of a 
	// vector from the camera to the first vertex.
	if (dot(cameraPosition.xyz - output.PositionWS, face_normal) < 0) { return; }

	// Calculate clip space position of the first vertex
	output.PositionCS = mul(float4(output.PositionWS, 1.0), viewProjection);

	// Normals are normalized in the pixel shader so no need to do that here
	output.NormalWS = mul(float4(input[0].Normal, 0.0), worldMatrix).xyz;

	// Pass along the texture coordinates
	output.TexCoord = input[0].TexCoord;

	outStream.Append(output);

	// Transform second and third vertices in a loop --------------------------
	for (uint i = 1; i < 3; i++)
	{
		// Convert position to world space
		output.PositionWS = mul(input[i].Position, worldMatrix).xyz;

		// Calculate clip-space position
		output.PositionCS = mul(float4(output.PositionWS, 1.0), viewProjection);

		// Normals are normalized in the pixel shader so no need to do that here
		output.NormalWS = mul(float4(input[i].Normal, 0.0), worldMatrix).xyz;

		// Pass along the texture coordinates
		output.TexCoord = input[i].TexCoord;

		outStream.Append(output);
	}
	outStream.RestartStrip();
}
