/* Base code from the book "Practical Rendering and Computation with Direct3D 11", page 504 */

/*
* Only outputs the Diffuse texture
* Diffuse and specular shading will be added later to complete the Phong shading
*
*
*/

// TODO: light constant buffer


// Lightning fragment shader

// Textures from G-Buffers
Texture2D NormalTexture			: register(t0);
Texture2D PositionTexture		: register(t1); 
Texture2D DiffuseAlbedoTexture	: register(t2);
//Texture2D SpecularAlbedoTexture	: register(t3);

SamplerState textureSampler; // TODO: set SamplerState settings

// TODO: light constant buffer here


struct PS_IN
{
	float4 Position	: SV_Position; //pixel location
	float2 TexCoord	: TEXCOORD;
};

// Helper function for extracting G-Buffer attributes
void GetGBufferAttributes( in float2 screenPos, 
	out float3 normal,
	out float3 position,
	out float3 diffuseAlbedo/*, 
	out float3 specularAlbedo,
	out float specularPower*/)
{
	// Determine our indices for sampling the texture based on the current screen position
	int3 sampleIndices = int3(screenPos.xy, 0);

	normal = NormalTexture.Load(sampleIndices).xyz;
	position = PositionTexture.Load(sampleIndices).xyz;
	diffuseAlbedo = DiffuseAlbedoTexture.Load(sampleIndices).xyz;
	//float4 spec = SpecularAlbedoTexture.Load(sampleIndices);

	//specularAlbedo = spec.xyz;
	//specularPower = spec.w;
}

// Lighting fragment shader

float4 PS_main ( PS_IN input ) : SV_Target
{
	//float3 normal;
	//float3 position;
	//float3 diffuseAlbedo;
	//float3 specularAlbedo;
	//float specularPower;
	
	// Sample the G-Buffer properties from the textures
	//GetGBufferAttributes(screenPos.xy, normal, position, diffuseAlbedo, specularAlbedo, specularPower);

	//float3 lighting = CalcLighting(normal, position, diffuseAlbedo, specularAlbedo, specularPower);
	
	// Only renders the diffuse color G-Buffer
	// Change to normal, or position for respective G-Buffers.
	float3 DiffuseColor = DiffuseAlbedoTexture.Sample(textureSampler, input.TexCoord).rgb;
	float3 normal = NormalTexture.Sample(textureSampler, input.TexCoord).rgb;
	float3 position = PositionTexture.Sample(textureSampler, input.TexCoord).rgb;

	float3 finalColor = position;

	return float4(finalColor, 1.0);
}