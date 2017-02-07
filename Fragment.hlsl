Texture2D txDiffuse:register(t0);
SamplerState sampAni;
struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Texcoord:TEXCOORD;
	float4 Normal:NORMAL;
	float4 worldPos:POSITION;
	float3 Color : COLOR;
};


float4 PS_main(VS_OUT input) : SV_Target
{
	float3 lightPos={0.0f,200.0f,-2.1f};
	//float3 lightPos = input.Color;
	float3 s = txDiffuse.Sample(sampAni,input.Texcoord).xyz;
	float3 normal = normalize(input.Normal.xyz);
	float3 lightVector = normalize(lightPos-input.worldPos.xyz);
	float angle = clamp(dot(normal, lightVector),0,1);
	float4 finalLight = float4(s*angle, 0.0f);
	//finalLight = float4(input.worldPos.y, input.worldPos.y , input.worldPos.y , 0.0f);
	//return float4(s, 1.0f);
	//return finalLight;
	return input.Normal;
	//return float4(input.Normal.xyz,1.0f);
};