// G-Buffer fragment shader

#if HEIGHT_MAP
Texture2D DiffuseMap	: register(t1);
Texture2D DiffuseMap2	: register(t2);
#else 
Texture2D Texture		: register(t1);
#endif

SamplerState AnisoSampler	: register(s0);

cbuffer MaterialBuffer		: register(b0)
{
	float3 SpecularColor;
	float SpecularPower;
	float3 DiffuseColor;
	int TexArrayIndex;
	int hasTexture;

	float3 padding;
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
#if HEIGHT_MAP
	float height = input.PositionWS.y * 0.05; // input.PositionWS.y / 20.0
	float3 diffuse_part = (hasTexture == 1) ? (((1.0 - height) * DiffuseMap.Sample(AnisoSampler, input.TexCoord).rgb) + (height * DiffuseMap2.Sample(AnisoSampler, input.TexCoord).rgb)) : DiffuseColor.rgb;
#else
	float3 diffuse_part = (hasTexture == 1) ? Texture.Sample(AnisoSampler, input.TexCoord).rgb : DiffuseColor.rgb;
	//diffuse_part = (hasTexture == 1) ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
#endif
	// Normalize the normal after interpolation
	float3 normalWS	= normalize(input.NormalWS);

	// Ouput G-Buffer values
	output.Normal			= float4(normalWS, 1.0);
	output.Position			= float4(input.PositionWS, 1.0);
	output.DiffuseAlbedo	= float4(diffuse_part, 1.0);
	output.SpecularAlbedo	= float4(SpecularColor, SpecularPower);

	return output;
};