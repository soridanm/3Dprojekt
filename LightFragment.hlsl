/* Base code from the book "Practical Rendering and Computation with Direct3D 11", page 504 */

/*
* Only outputs the Diffuse texture
* Diffuse and specular shading will be added later to complete the Phong shading
*
*
*/

// TODO: light constant buffer
// Gamma correction?

// Lightning fragment shader

#define MAX_LIGHTS 8

// Textures from G-Buffers
Texture2D NormalTexture			: register(t0);
Texture2D PositionTexture		: register(t1); 
Texture2D DiffuseAlbedoTexture	: register(t2);
//Texture2D SpecularAlbedoTexture	: register(t3);

SamplerState textureSampler; // TODO: set SamplerState settings

// TODO: light constant buffer here


struct materialStruct
{
	float3 specularAlbedo;
	float specularPower;
};

cbuffer materialProperties : register(b0)
{
	materialStruct Material;
};


struct Light
{
	float4 light_position; // the position of the light in world space
	float4 color; // the color of the light
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float ambient_coefficient; //might be moved/removed
};

cbuffer pointLightProperties		: register (b1)
{
	float4 camera_position;
	float4 global_ambient;
	Light Lights[MAX_LIGHTS];
};

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

float4 DiffusePart(Light light, float3 light_ray, float3 normal)
{
	return light.color;
}


float4 PS_main ( PS_IN input ) : SV_Target
{
	float3 camera_pos = float3(0.0, 0.0, 2.0);

	float shinyPower = 100.0;

	float3 light_pos = float3(0.0, -1.0, 4.0);
	float3 specular_color = float3(0.8, 0.8, 0.8);
	float3 ambient_power = float3(0.05, 0.05, 0.05);
	

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
	float4 DiffuseColor = DiffuseAlbedoTexture.Sample(textureSampler, input.TexCoord);
	float4 normalTex = NormalTexture.Sample(textureSampler, input.TexCoord);
	float3 position = PositionTexture.Sample(textureSampler, input.TexCoord).rgb;





	float3 normal = normalTex.xyz; // normal vector from surface
	float3 to_light = normalize(light_pos - position); // vector from light source to surface
	float3 to_camera = normalize(camera_pos - position); // vectore from surface to camera
	float3 reflection = reflect(-to_light, normal); // vector of reflected light after light hits surface

	// Diffuse part
	float3 diffuse_factor = specular_color * DiffuseColor.rgb * max(dot(to_light, normal), 0.0);

	// Specular part
	float3 specular_factor = specular_color * pow(max(dot(reflection, to_camera), 0.0), shinyPower);

	// ambient part
	float3 ambient_component = ambient_power*DiffuseColor.rgb;


	float3 final_color = saturate(ambient_component + diffuse_factor + specular_factor);



	float specularPower = normalTex.w;

	

	//diffuse lighting
	

	// vector from light to point
	//float3 light_ray = normalize(position - light_pos);
	//// diffuse lighting
	//float3 diffuse_light = DiffuseColor.rgb * DiffuseColor.a * saturate(dot(normal, -light_ray));

	//// specular lighting
	//float3 r = 2.0*dot(normal - light_ray)*normal - light_ray;
	//float3 specular_light = 2.0*float3(1.0,1.0,1.0)*dot(r, )

	float3 finalColor = final_color;

	return float4(finalColor, 1.0);
}