/* Base code from the book "Practical Rendering and Computation with Direct3D 11", page 504 */

// TODO: Rename buffers and variables
// TODO? Gamma correction

#define NR_OF_LIGHTS 2

//TODO? Texture2D<float4>

// Textures from G-Buffers
Texture2D NormalTexture			: register(t0); // x-y-z-unused
Texture2D PositionTexture		: register(t1); // x-y-z-unused
Texture2D DiffuseAlbedoTexture	: register(t2); // r-g-b-unused
Texture2D SpecularAlbedoTexture	: register(t3); // r-g-b-specularPower
Texture2D ShadowMap				: register(t4); // r

SamplerState textureSampler; // TODO: look up sampler settings

cbuffer shadowMapMatrix			: register(b0)
{
	float4x4 lightView;
	float4x4 lightProjection;
}

struct Light
{
	float4 light_positionWS;
	float3 color;
	int hasShadow;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	float ambient_coefficient;
};

cbuffer pointLightProperties	: register (b1)
{
	float4 camera_positionWS;
	float4 global_ambient;
	Light Lights[NR_OF_LIGHTS];
};



struct PS_IN
{
	float4 PositionCS	: SV_Position; //pixel location
	float2 TexCoord		: TEXCOORD1;
};


float4 PS_main ( PS_IN input ) : SV_Target
{
	float3 normal			= NormalTexture.Sample(textureSampler, input.TexCoord).xyz; 
	float3 positionWS		= PositionTexture.Sample(textureSampler, input.TexCoord).rgb;
	float3 diffuseColor		= DiffuseAlbedoTexture.Sample(textureSampler, input.TexCoord).rgb;
	float4 specularValues	= SpecularAlbedoTexture.Sample(textureSampler, input.TexCoord);

	float3 to_camera	= normalize(camera_positionWS.xyz - positionWS); // vectore from surface to camera
// LOOP OVER LIGHTS FROM HERE ---------------------------------------------------------------------------


	//shadow map
	float4x4 lightViewProjection = mul(lightView, lightProjection);


	float4 test = mul(float4(positionWS, 1.0), lightViewProjection);

	float bias = 0.001f;
	float2 shadowTexCoord;
	shadowTexCoord.x = test.x / test.w / 2.0 + 0.5;
	shadowTexCoord.y = test.y / test.w / 2.0 + 0.5;
	//Dividing by w gives positions in clip-space [-1, 1]
	//Dividing by 2.0 and adding 0.5 gives uv coordinates [0, 1]



	float3 final_color = global_ambient * diffuseColor;

	for (int i = 0; i < NR_OF_LIGHTS; i++)
	{
		
		//float3 normal = normalTex.xyz; // normal vector from surface
		float3 to_light = normalize(Lights[i].light_positionWS.xyz - positionWS); // vector from light source to surface
		float3 reflection = reflect(-to_light, normal); // vector of reflected light after light hits surface
		float distance1 = distance(Lights[i].light_positionWS, positionWS);

		// Diffuse part
		float diffuse_coefficient = max(dot(to_light, normal), 0.0);
		float3 diffuse_component = diffuse_coefficient * diffuseColor * Lights[i].color;

		// Specular part
		/* specular_coefficient = 0.0 if the light is behind the surface */
		float specular_coefficient = diffuse_coefficient > 0.0 ? pow(max(dot(reflection, to_camera), 0.0), specularValues.a) : 0.0;
		float3 specular_component = specular_coefficient * specularValues.rgb * Lights[i].color;

		// ambient part
		float3 ambient_component = Lights[i].ambient_coefficient * Lights[i].color * diffuseColor; // light intesities

		float attenuation = 1.0 / (Lights[i].constant_attenuation + distance1 * (Lights[i].linear_attenuation + distance1 * Lights[i].quadratic_attenuation));

		float3 light_contribution = saturate(ambient_component + attenuation * (diffuse_component + specular_component));
		final_color = saturate(final_color + light_contribution);
	}

// LOOP OVER LIGHTS TO HERE ---------------------------------------------------------------------------

	//final_color = saturate(final_color + global_ambient * diffuseColor );
	// In case you want to set it to one of the G-Buffers

	//float3 finalColor = normal;
	//float3 finalColor = positionWS;
	//float3 finalColor = diffuseColor;
	//float3 finalColor = specularValues.rgb;

	//float3 finalColor = float3(diffuse_coefficient, diffuse_coefficient, diffuse_coefficient);
	//float3 finalColor = diffuse_component;
	//float3 finalColor = float3(specular_coefficient, specular_coefficient, specular_coefficient);
	//float3 finalColor = float3(1.0, 0.0, 0.0);

	//float3 finalColor = final_color;

	float3 finalColor = float3(shadowTexCoord, 0.0);

	return float4(finalColor, 1.0);
}