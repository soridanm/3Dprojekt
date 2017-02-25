// G-Buffer fragment shader

Texture2D DiffuseMap		: register(t1);
SamplerState AnisoSampler	: register(s0);

cbuffer MaterialBuffer		: register(b0)
{
	float3 SpecularAlbedo;
	float SpecularPower;
	bool hasTexture;
	int TexArrayIndex;

	int padding;
	int padding2;
};

struct PS_IN
{
	float4 PositionCS	: SV_Position;
	float2 TexCoord		: TEXCOORD;
	float3 NormalWS		: NORMALWS;
	float3 PositionWS	: POSITIONWS;
};

struct PS_OUT
{
	float4 Normal			: SV_Target0; // x-y-z-unused
	float4 Position			: SV_Target1; // x-y-z-unused
	float4 DiffuseAlbedo	: SV_Target2; // r-g-b-unused
	float4 SpecularAlbedo	: SV_Target3; // r-g-b-specularPower
};


PS_OUT PS_main(in PS_IN input) //: SV_Target
{
	PS_OUT output = (PS_OUT)0;

	// Sample the diffuse map
	float3 diffuseAlbedo = DiffuseMap.Sample(AnisoSampler, input.TexCoord).rgb;

	// Normalize the normal after interpolation
	float3 normalWS	= normalize(input.NormalWS);

	// Ouput G-Buffer values
	output.Normal			= float4(normalWS, 1.0);
	output.Position			= float4(input.PositionWS, 1.0);
	output.DiffuseAlbedo	= float4(diffuseAlbedo, 1.0);
	output.SpecularAlbedo	= float4(SpecularAlbedo, SpecularPower);

	return output;
};