//-----------------------------------------------------------------------------------------
//			SHADOW PASS VERSION
// Ony outputs the clip-space position since that's the only information
// needed to fill in the depth buffer
//-----------------------------------------------------------------------------------------

//TODO: remove normal and texcoord from input as well as the camera position

cbuffer PER_FRAME	:register(b0)
{
	float4x4 viewProjection;
	//float4x4 view;
	//float4x4 projection;
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
	float2 TexCoord : TEXCOORD; // <- look up number
	float3 Normal   : NORMAL;
};

struct VS_OUT
{
	float4 PositionCS : SV_Position;
};

VS_OUT VS_main(in VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	//float4x4 viewProjection = mul(view, projection);
	//float4x4 worldViewProjection = mul(worldMatrix, viewProjection);

	float4 temp = mul(float4(input.Position, 1.0), worldMatrix);

	output.PositionCS = mul(temp, viewProjection);

	return output;
}