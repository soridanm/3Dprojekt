/* Code from the book "Practical Rendering and Computation with Direct3D 11", page 499 */

// TODO: make one single cbuffer instead of 3
cbuffer WORLDMATRIX:register(b1) {
	float4x4 worldMatrix;
};
cbuffer VIEWMATRIX : register (b2) {
	float4x4 viewMatrix;
};
cbuffer PROJECTIONMATRIX : register (b3) {
	float4x4 projectionMatrix;
};

Texture2D DiffuseMap		: register(t0);
SamplerState AnisoSampler	: register(s0);

struct VSInput 
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORDS0; //look up TEXCOORDS0
};

struct VSOutput
{
	float4 PositionCS	: SV_Position;
	float2 TexCoord	: TEXCOORD;
	float3 NormalWS	: NORMALWS;
	float3 PositionWS : POSITIONWS;
};

struct PSInput
{
	float4 PositionSS : SV_Position;
	float2 TexCoord : TEXCOORD;
	float3 NormalWS : NORMALWS;
	float3 PositionWS : POSITIONWS;
};

/* Output to G-buffer */
struct PSOutput
{
	float4 Normal : SV_Target0;
	float4 DiffuseAlbedo : SV_Target1;
	//float4 SpecularAlbedo : SV_Target2;
	float4 Position : SV_Target3;
};

/* G-Buffer vertex shader */
VSOutput VSMain(in VSInput input)
{

	float4x4 worldViewProjMatrix = mul(worldMatrix, mul(viewPatrix, projectionMatrix));



	VSOutput output;
	// Convert position and normals to world space
	output.PositionWS = mul(input.Position, worldMatrix).xyz;
	output.NormalWS = normalize(mul(input.Normal, (float3x3)worldMatrix));

	// Calculate the clip-space position
	output.PositionCS = mul(input.Position, worldViewProjMatrix);

	// Pass along the texture coordinate
	output.TexCoord = input.TexCoord;

	return output;

}

/* G-Buffer pixel shader */
PSOutput PSMain(in PSInput input)
{
	PSOutput output;

	//Sample the diffuse map
	float3 diffuseAlbedo = DiffuseMap.Sample(AnisoSampler, input.TexCoord).rgb;

	//Normalize the normal after interpolation
	float3 normalWS = normalize(input.NormalWS);

	//Output our G-Buffer values
	output.Normal = float4(normalWS, 1.0f);
	output.DiffuseAlbedo - float4(diffuseAlbedo, 1.0f);
	//output.SpecularAlbedo = float4(SpecularAlbedo, SpecularPower);
	output.Position = float4(input.PositionWS, 1.0f);

	return output;
}



