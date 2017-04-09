/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: GBufferPixel.hlsl
*
* File summary: Pixel shader of the Geometry pass
*	Outputs information to the G-buffers and if the object is a heightmap then
*	it applies a simple height-based blend mapping on the textures.
*/

// Used for syntax highlighting
#if defined(__INTELLISENSE__)
#define IS_HEIGHT_MAP
#endif


#ifdef IS_HEIGHT_MAP
Texture2D LowerTexture	: register(t1);
Texture2D HigherTexture	: register(t2);
#else 
Texture2D Texture		: register(t1);
#endif

//TODO: rename and check settings
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

#ifdef IS_HEIGHT_MAP
	float height = saturate(input.PositionWS.y * 0.05); // = input.PositionWS.y / 20.0
	float3 diffuse_part = 
		(hasTexture == 1) 
			? ((1.0 - height) * LowerTexture.Sample(AnisoSampler, input.TexCoord).rgb
			+ height * HigherTexture.Sample(AnisoSampler, input.TexCoord).rgb)
			: DiffuseColor.rgb;
#else
	float3 diffuse_part = (hasTexture == 1) ? Texture.Sample(AnisoSampler, input.TexCoord).rgb : DiffuseColor.rgb;
#endif

	// Normalize the normal after interpolation
	float3 normalWS = normalize(input.NormalWS);

	// Ouput G-Buffer values
	output.Normal = float4(normalWS, 1.0);
	output.Position = float4(input.PositionWS, 1.0);
	output.DiffuseAlbedo = float4(diffuse_part, 1.0);
	output.SpecularAlbedo = float4(SpecularColor, SpecularPower);

	return output;
}
