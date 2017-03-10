Texture2D<float4>   inputTex  : register(t0);
RWTexture2D<float4> outputTex : register(u0);
SamplerState samp;

groupshared float4 image_square[40][20];

// TODO: Improve this mess of an if-else case
#ifdef TEXTURE_WIDTH
#ifdef TEXTURE_HEIGHT
groupshared float2 inverseScreenSize = float2((1.0 / TEXTURE_WIDTH), (1.0 / TEXTURE_HEIGHT));
#else
groupshared float2 inverseScreenSize = float2(1.0, 1.0);
#endif
#else
groupshared float2 inverseScreenSize = float2(1.0, 1.0);
#endif

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125

//static float3 kernel1D = float3(0.27901f, 0.44198f, 0.27901f);

//TODO: See if this can be optimized
float rgb2luma(float3 rgb)
{
	return sqrt(dot(rgb, float3(0.299, 0.587, 0.114)));
}


[numthreads(40, 20, 1)]
void FXAA_main(
	uint3	dispaThreadID	: SV_DispatchThreadID,	// Global position
	uint3	groupThreadID : SV_GroupThreadID,		// Group position
	uint	groupIndex : SV_GroupIndex,
	uint3   groupID : SV_GroupID)
{

	float texWidth;
	float texHeight;
	float2 UV = (dispaThreadID.xy + float2(0.5, 0.5)) * inverseScreenSize;

	float2 screen_pos = dispaThreadID.xy;
	image_square[groupThreadID.x][groupThreadID.y] = inputTex[screen_pos];

	GroupMemoryBarrierWithGroupSync();

	float3 colorCenter = inputTex.SampleLevel(samp, UV, 0.0).rgb;//inputTex[screen_pos].rgb;
	float3 testCenter = inputTex[screen_pos].rgb;

	// Luma at the current fragment
	float lumaC = rgb2luma(colorCenter);

	// Luma at the four direct neighbours of the current fragment
	float lumaS	= rgb2luma(inputTex[screen_pos + float2( 0.0, 1.0)].rgb);
	float lumaN	= rgb2luma(inputTex[screen_pos + float2( 0.0,-1.0)].rgb);
	float lumaW	= rgb2luma(inputTex[screen_pos + float2(-1.0, 0.0)].rgb);
	float lumaE = rgb2luma(inputTex[screen_pos + float2( 1.0, 0.0)].rgb);

	// Find the max and min luma around the current fragment
	float lumaMin = min(lumaC, min(min(lumaS, lumaN), min(lumaW, lumaE)));
	float lumaMax = max(lumaC, max(max(lumaS, lumaN), max(lumaW, lumaE)));

	// Compare the difference
	float lumaRange = lumaMax - lumaMin;

	float4 result = float4(1.0, 1.0, 1.0, 1.0);

//#ifdef TEXTURE_WIDTH
//	if (TEXTURE_WIDTH == 1280)
//	{
//		result = float4(1.0, 0.0, 0.0, 1.0);
//	}
//#endif
	// Early exit if no edge is detected in the area or if the area is really dark
	if (lumaRange < max(EDGE_THRESHOLD_MIN, (lumaMax * EDGE_THRESHOLD_MAX)))
	{
		result = float4(colorCenter, 1.0);
		/*if (testCenter.r == colorCenter.r && testCenter.g == colorCenter.g && testCenter.b == colorCenter.b)
		{
			result = float4(1.0, 0.0, 0.0, 1.0);
		}*/
	}
	else
	{
		// The four remaining corner lumas
		float lumaSE = rgb2luma(inputTex[screen_pos + float2( 1.0, 1.0)].rgb);
		float lumaSW = rgb2luma(inputTex[screen_pos + float2(-1.0, 1.0)].rgb);
		float lumaNE = rgb2luma(inputTex[screen_pos + float2( 1.0,-1.0)].rgb);
		float lumaNW = rgb2luma(inputTex[screen_pos + float2(-1.0,-1.0)].rgb);

		// Combine the four edges' lumas
		float lumaSN = lumaS + lumaN;
		float lumaWE = lumaW + lumaE;

		// The same is done for the corners
		float lumaCW = lumaNW + lumaSW; // Corner West
		float lumaCE = lumaNE + lumaSE; // Corner East
		float lumaCS = lumaSE + lumaSW; // Corner South
		float lumaCN = lumaNE + lumaNW; // Corner North

		// Compute an estimation of the gradient along the horizontal and vertical axiz
		float edgeHor = abs(-2.0 * lumaW + lumaCW) + abs(-2.0 * lumaC + lumaSN) * 2.0 + abs(-2.0 * lumaE + lumaCE);
		float edgeVer = abs(-2.0 * lumaN + lumaCN) + abs(-2.0 * lumaC + lumaWE) + 2.0 + abs(-2.0 * lumaS + lumaCS);

		bool isHorizontal = (edgeHor >= edgeVer);

		// Select the two neighbouring texels' lumas in teh opposite direction of the local edge
		float luma1 = isHorizontal ? lumaS : lumaW;
		float luma2 = isHorizontal ? lumaN : lumaE;
		// Compute gradients in this direction
		float grad1 = luma1 - lumaC;
		float grad2 = luma2 - lumaC;

		// Which directinon is teh steepest?
		bool is1Steepest = abs(grad1) >= abs(grad2);

		// Normalized gradient in the corresponding directino
		float gradScaled = 0.25 * max(abs(grad1), abs(grad2));

		// Choose the step size (one pixel) according to the edge direction
		float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

		// average loma in the correct direction
		float lumaLocalAverage = 0.0;

		if (is1Steepest)
		{
			// switch the direction
			stepLength = -stepLength;
			lumaLocalAverage = 0.5 * (luma1 + lumaC);

		} else {
			lumaLocalAverage = 0.5 * (luma2 + lumaC);
		}

		// Shift the texel coordinate by half a pixel
	//	float2 currentUV = 

	}





	//Flat color
	//float4 result = float4(1.0f, 0.0f, 0.0f, 1.0f);

	//Copy image
	//float4 result = inputTex[screen_pos];
	//float4 result = image_square[groupThreadID.x][groupThreadID.y];

	//GroupID
	//float4 result = float4((groupID % 5) /4.0, 1.0);

	//Thread ID
	//float4 result = float4((screen_pos.x / 1920.0f), (screen_pos.y / 1080.0f), 0.0f, 1.0f);

	//Edge filter
	//float4 result = abs(image_row[min(pos.x+1,640)] - image_row[max(pos.x-1,0)]); 

	//Horizontal blur 1x3
	//float4 result = image_row[max(pos.x - 1, 0)] * kernel1D[0]
	//	+ image_row[pos.x] * kernel1D[1]
	//	+ image_row[min(pos.x + 1, 1280)] * kernel1D[2];


	outputTex[screen_pos] = result;

}