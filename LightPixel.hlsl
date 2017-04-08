/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: Handler.hpp
*
* File summary:
*
*
*
*
*
*/

// TODO: make the seperate components work
// TODO: Rename buffers and variables
// TODO: look up if doubles are more suitable for the shadow calculations
// TODO? Gamma correction

#define NR_OF_LIGHTS 2

#ifdef SHADOW_MAP_SIZE
static const float inverseShadowMapSize = rcp(SHADOW_MAP_SIZE);
#else 
static const float inverseShadowMapSize = 1.0;
#endif


// Textures from G-Buffers
Texture2D<float4> NormalTexture			: register(t0); // x-y-z-unused
Texture2D<float4> PositionTexture		: register(t1); // x-y-z-unused
Texture2D<float4> DiffuseAlbedoTexture	: register(t2); // r-g-b-unused
Texture2D<float4> SpecularTexture		: register(t3); // r-g-b-specularPower
Texture2D<float>  ShadowMap				: register(t4); // r

SamplerComparisonState compSampler : register(s0);
SamplerState textureSampler; // TODO: look up sampler settings

cbuffer shadowMapMatrix			: register(b0)
{
	float4x4 lightViewProjection;
	float4 cameraPositionUnused;
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
};

float2 texOffset(int u, int v)
{
	return float2(u, v) * inverseShadowMapSize;
}

float4 PS_main ( PS_IN input ) : SV_Target
{
	// Values from the G-Buffers
	float3 normal			= NormalTexture.Load(int3(input.PositionCS.xy, 0)).rgb;
	float3 positionWS		= PositionTexture.Load(int3(input.PositionCS.xy, 0)).rgb;
	float3 diffuseColor		= DiffuseAlbedoTexture.Load(int3(input.PositionCS.xy, 0)).rgb;
	float4 specularValues	= SpecularTexture.Load(int3(input.PositionCS.xy, 0));

	// Phong lighting variables
	float attenuation, diffuse_coefficient, specular_coefficient;
	float3 ambient_component, diffuse_component, specular_component, light_contribution;
	float3 final_color = global_ambient.rgb * diffuseColor.rgb; // All positions are hit by the global ambient light

	// Used for the diffuse and specular calculations
	float3 to_camera = normalize(camera_positionWS.xyz - positionWS); // vector from surface to camera
	float3 to_light;			// vector from light source to surface
	float3 reflection;			// vector of reflected light after light hits surface
	float distance_to_light;	// distance between the light and the surface point

	// Used for the shadow map calculations
	//float4x4 lightViewProjection = mul(lightView, lightProjection); //TODO: change the cbuffer to a single matrix
	float4 light_view_pos = mul(float4(positionWS, 1.0), lightViewProjection); // The clip space position of the point from the light's point of view
	float bias = 0.00002;
	
	float2 shadow_tex_coord;
	//shadow_tex_coord.x = (( light_view_pos.x / light_view_pos.w) / 2.0) + 0.5;  // Dividing by w gives positions as normalized device coordinates [-1, 1]
	//shadow_tex_coord.y = ((-light_view_pos.y / light_view_pos.w) / 2.0) + 0.5;  // Dividing by 2.0 and adding 0.5 gives position as uv coordinates [0, 1]
	shadow_tex_coord.x = (light_view_pos.x * rcp(light_view_pos.w) * 0.5) + 0.5;  // Dividing by w gives positions as normalized device coordinates [-1, 1]
	shadow_tex_coord.y = (-light_view_pos.y * rcp(light_view_pos.w) * 0.5) + 0.5;  // Dividing by 2.0 and adding 0.5 gives position as uv coordinates [0, 1]

	// calculate the distance from the light to the texel (the z value in NDC)
	//float light_depth_value = light_view_pos.z / light_view_pos.w;
	float light_depth_value = light_view_pos.z * rcp(light_view_pos.w);


	//subtract the bias to avoid self-shadowing
	light_depth_value -= bias;

	// Test if the point is outside the light's view frustum
	bool outside_shadow_map = (saturate(shadow_tex_coord.y) != shadow_tex_coord.y && saturate(shadow_tex_coord.x) != shadow_tex_coord.x) ? true : false;
	// 16-tap (4x4 texel area) Percentage-Closer Filtering using a comparison sampler
	float shadow_sum = 0.0;
	float x, y;
	if (!outside_shadow_map)
	{
		for (y = -1.5; y <= 1.5; y += 1.0)
		{
			for (x = -1.5; x <= 1.5; x += 1.0)
			{
				shadow_sum += ShadowMap.SampleCmpLevelZero(compSampler, shadow_tex_coord + texOffset(x, y), light_depth_value);
			}
		}
		shadow_sum *= 0.0625; // Division by 16
	}
	
	float shadow_coefficient = (outside_shadow_map) ? 1.0 : shadow_sum;

	// FOR DEBUGGING: Overwrites the Percentage-Closer Filtering
	//shadow_coefficient = (!outside_shadow_map) ? ShadowMap.SampleCmpLevelZero(compSampler, shadow_tex_coord - texOffset(0.5, 0.5), light_depth_value) : 1.0;
	//shadow_coefficient = (light_depth_value < ShadowMap.Sample(textureSampler, shadow_tex_coord).r) ? 1.0 : 0.0;
	// !FOR DEBUGGING

	for (int i = 0; i < NR_OF_LIGHTS; i++)
	{
		// The light contributes with its ambient part even if the pixel is in shadow
		//ambient_component = Lights[i].ambient_coefficient * (Lights[i].color * diffuseColor); // light intesities
		final_color += Lights[i].ambient_coefficient * (Lights[i].color * diffuseColor); // light intesities

		// If the pixel is in full shadow then the diffuse and specular parts won't contribute to the final color so we won't calculate them
		if (!(Lights[i].hasShadow == 1 && shadow_coefficient == 0.0))
		{
			to_light = normalize(Lights[i].light_positionWS.xyz - positionWS);
			reflection = reflect(-to_light, normal);
			distance_to_light = distance(Lights[i].light_positionWS.xyz, positionWS);

			// Diffuse part
			diffuse_coefficient = max(dot(to_light, normal), 0.0);
			diffuse_component = diffuse_coefficient * diffuseColor * Lights[i].color;

			// Specular part
			/* specular_coefficient equals 0.0 if the light is behind the surface */
			//specular_coefficient = (diffuse_coefficient > 0.0) ? pow(max(dot(reflection, to_camera), 0.0), specularValues.a) : 0.0;
			specular_coefficient = (diffuse_coefficient > 0.0) ? pow(saturate(dot(reflection, to_camera)), specularValues.a) : 0.0;
			specular_component = specular_coefficient * (specularValues.rgb * Lights[i].color);

			// rcp(x) = (1.0 / x)
			attenuation = rcp(Lights[i].constant_attenuation + distance_to_light * (Lights[i].linear_attenuation + distance_to_light * Lights[i].quadratic_attenuation));

			final_color += attenuation * (diffuse_component + specular_component) * ((Lights[i].hasShadow == 1) ? shadow_coefficient : 1.0);
		}
	}

	//float3 finalColor = normal;
	//float3 finalColor = positionWS/255.0;
	//float3 finalColor = diffuseColor;
	//float3 finalColor = specularValues.rgb;

	//float3 finalColor = float3(diffuse_coefficient, diffuse_coefficient, diffuse_coefficient);
	//float3 finalColor = diffuse_component;
	//float3 finalColor = float3(specular_coefficient, specular_coefficient, specular_coefficient);
	//float3 finalColor = float3(1.0, 0.0, 0.0);

	//float3 finalColor = float3(shadow_tex_coord, 0.0);
	//float3 finalColor = float3(shadow_coefficient, 0.0, 0.0);

	float3 finalColor = saturate(final_color);

	return float4(finalColor, 1.0);
}
