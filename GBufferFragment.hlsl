// G-Buffer fragment shader

Texture2D DiffuseMap		: register(t0);
SamplerState AnisoSampler	: register(s0);

struct materialStruct
{
	float3 specularAlbedo;
	float specularPower;
};

cbuffer materialProperties	: register(b0)
{
	materialStruct Material;
};

struct PS_IN
{
	float4 PositionCS		: SV_Position;
	float2 TexCoord		: TEXCOORD;
	float3 NormalWS		: NORMALWS;
	float3 PositionWS		: POSITIONWS;
};

struct PS_OUT
{
	float4 Normal		: SV_Target0; // x-y-z-unused
	float4 Position		: SV_Target1; // x-y-z-unused
	float4 DiffuseAlbedo	: SV_Target2; // r-g-b-unused
	float4 SpecularAlbedo	: SV_Target3; // r-g-b-specularPower
};


PS_OUT PS_main(in PS_IN input) //: SV_Target
{
	PS_OUT output = (PS_OUT)0;

	// Sample the diffuse map
	float3 diffuseAlbedo = DiffuseMap.Sample(AnisoSampler, input.TexCoord).rgb;

	// Normalize the normal after interpolation
	float3 normalWS = normalize(input.NormalWS);

	// Ouput G-Buffer values
	output.Normal = float4(normalWS, 1.0);
	output.Position = float4(input.PositionWS, 1.0);
	output.DiffuseAlbedo = float4(diffuseAlbedo, 1.0);
	output.SpecularAlbedo = float4(Material.specularAlbedo, Material.specularPower);

	return output;

	//float3 lightPos={0.0f,0.0f,-2.1f};
	//float3 s = txDiffuse.Sample(sampAni,input.Texcoord).xyz;
	/*float3 normal = normalize(input.Normal.xyz);
	float3 lightVector = normalize(lightPos-input.worldPos.xyz);
	float angle = clamp(dot(normal, lightVector),0,1);
	float4 finalLight = float4(s*angle, 1.0f);*/

	//return float4(s, 1.0f);

//	return input.Normal;
	//return float4(input.Normal.xyz,1.0f);
};