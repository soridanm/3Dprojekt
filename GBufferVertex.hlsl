//-----------------------------------------------------------------------------------------
//			DEFERRED VERSION
// Turns the Position into homogeneous coordinates.
// Otherwise only works as a passthrough
//-----------------------------------------------------------------------------------------

struct VS_IN
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD; // <- look up number
	float3 Normal	: NORMAL;
};

struct VS_OUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
};

VS_OUT VS_main(in VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.Position = float4(input.Position, 1.0);
	output.TexCoord = input.TexCoord;
	output.Normal	= input.Normal;

	return output;
}