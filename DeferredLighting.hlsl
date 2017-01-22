/* Code from the book "Practical Rendering and Computation with Direct3D 11", page 504 */


// Textures from G-Buffers
Texture2D NormalTexture			: register(t0);
Texture2D DiffuseAlbedoTexture	: register(t1);
//Texture2D SpecularAlbedoTexture	: register(t2);
Texture2D PositionTexture		: register(t3);


// Helper function for extracting G-Buffer attributes
void GetGBufferAttributes(in float2 screenPos, out float3 normal,
	out float3 position,
	out float3 diffuseAlbedo /*,out float3 specularAlbedo,
	out float specularPower*/)
{
	// Determine our indices for sampling the texture based on the current 
	// screen position
	int3 sampleIndices = int3(screenPos.xy, 0);

	normal = NormalTexture.Load(sampleIndices).xyz;
	position = PositionTexture.Load(sampleIndices).xyz;
	diffuseAlbedo = DiffuseAlbedoTexture.Load(sampleIndices).xyz;
	//float4 spec = SpecularAlbedoTexture.Load(sampleIndices);

	//specularAlbedo = spec.xyz;
	//specularPower = spec.w;
}



float3 PSMain ( in float4 screenPos : SV_Position ) : SV_Target0
{
	float3 normal;
	float3 position;
	float3 diffuseAlbedo;
	//float3 specularAlbedo;
	//float specularPower;
	
	// Sample the G-Buffer properties from the textures
	GetGBufferAttributes(screenPos.xy, normal, position, diffuseAlbedo/*,
	specularAlbedo, specularPower*/);

	//float3 lighting = CalcLighting(normal, position, diffuseAlbedo, specularAlbedo, specularPower);
	float3 lighting = diffuseAlbedo;

	return float4(lighting, 1.0f);
}