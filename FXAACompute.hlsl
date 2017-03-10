Texture2D<float4> inputTex : register(t0);
RWTexture2D<float4> output : register(u0);
SamplerState samp;

//groupshared float4 image_row[1];

static float3 kernel1D = float3(0.27901f, 0.44198f, 0.27901f);

[numthreads(20, 20, 1)]
void FXAA_main(
	uint3	dispaThreadID	: SV_DispatchThreadID,	// Global position
	uint3	groupThreadID	: SV_GroupThreadID,		// Group position
	uint	groupIndex		: SV_GroupIndex)
{
	float2 square_id = dispaThreadID.xy;
	float2 screen_pos = dispaThreadID.xy;// +groupThreadID.xy;
	//image_row[pos.x] = inputTex[pos];

	GroupMemoryBarrierWithGroupSync();

	//Flat color
	//float4 result = float4(1.0f, 0.0f, 0.0f, 1.0f);

	//Copy image
	float4 result = inputTex[screen_pos];

	//Thread ID
	//float4 result = float4((pos.x / 640.0f), (pos.y / 480.0f), 0.0f, 1.0f);

	//Edge filter
	//float4 result = abs(image_row[min(pos.x+1,640)] - image_row[max(pos.x-1,0)]); 

	//Horizontal blur 1x3
	//float4 result = image_row[max(pos.x - 1, 0)] * kernel1D[0]
	//	+ image_row[pos.x] * kernel1D[1]
	//	+ image_row[min(pos.x + 1, 1280)] * kernel1D[2];

	output[screen_pos] = result;
}