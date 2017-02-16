/* Base code from the book "Practical Rendering and Computation with Direct3D 11", page 504 */

// TODO: loop over multiple lights
// TODO? Gamma correction

#define MAX_LIGHTS 8

// Textures from G-Buffers
Texture2D NormalTexture			: register(t0); // x-y-z-unused
Texture2D PositionTexture		: register(t1); // x-y-z-unused
Texture2D DiffuseAlbedoTexture	: register(t2); // r-g-b-unused
Texture2D SpecularAlbedoTexture	: register(t3); // r-g-b-specularPower

SamplerState textureSampler; // TODO: look up sampler settings

struct Light
{
	float4 light_positionWS;
	float4 color;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float ambient_coefficient;
};

cbuffer pointLightProperties	: register (b0)
{
	float4 camera_positionWS;
	float4 global_ambient;
	Light Lights[MAX_LIGHTS];
};

struct PS_IN
{
	float4 PositionCS	: SV_Position; //pixel location
	float2 TexCoord		: TEXCOORD;
};


float4 PS_main ( PS_IN input ) : SV_Target
{
	float3 normal			= NormalTexture.Sample(textureSampler, input.TexCoord).xyz; 
	float3 positionWS		= PositionTexture.Sample(textureSampler, input.TexCoord).rgb;
	float3 diffuseColor		= DiffuseAlbedoTexture.Sample(textureSampler, input.TexCoord).rgb;
	float4 specularValues	= SpecularAlbedoTexture.Sample(textureSampler, input.TexCoord);

// LOOP OVER LIGHTS FROM HERE ---------------------------------------------------------------------------

	//float3 normal = normalTex.xyz; // normal vector from surface
	float3 to_light		= normalize(Lights[0].light_positionWS.xyz - positionWS); // vector from light source to surface
	float3 to_camera	= normalize(camera_positionWS.xyz - positionWS); // vectore from surface to camera
	float3 reflection	= reflect(-to_light, normal); // vector of reflected light after light hits surface
	float distance1		= distance(Lights[0].light_positionWS, positionWS);

	// Diffuse part
	float diffuse_coefficient	= max(dot(to_light, normal), 0.0);
	float3 diffuse_component	= diffuse_coefficient * diffuseColor * Lights[0].color;

	// Specular part
	/* specular_coefficient = 0.0 if the light is behind the surface */
	float specular_coefficient	= diffuse_coefficient > 0.0 ? pow(max(dot(reflection, to_camera), 0.0), specularValues.a) : 0.0;
	float3 specular_component	= specular_coefficient * specularValues.rgb * Lights[0].color;

	// ambient part
	float3 ambient_component = Lights[0].ambient_coefficient * Lights[0].color * diffuseColor; // light intesities

	float attenuation = 1.0 / (Lights[0].constant_attenuation + distance1 * (Lights[0].linear_attenuation + distance1 * Lights[0].quadratic_attenuation));

	float3 final_color = saturate(global_ambient + ambient_component + attenuation * (diffuse_component + specular_component));

// LOOP OVER LIGHTS TO HERE ---------------------------------------------------------------------------

	// In case you want to set it to one of the G-Buffers

	//float3 finalColor = normal;
	//float3 finalColor = positionWS;
	//float3 finalColor = diffuseColor;
	//float3 finalColor = specularValues.rgb;

	//float3 finalColor = diffuse_component;
	//float3 finalColor = float3(specular_coefficient, specular_coefficient, specular_coefficient);

	float3 finalColor = final_color;

	return float4(finalColor, 1.0);
}