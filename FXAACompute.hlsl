//TODO: Rewrite using offsets instead of uv coordinate variables
//TODO: restructure the ifdef/else/endif mess 
//TODO: Rename a bunch of variables to improve the readability

Texture2D<float4>   inputTex  : register(t0);
RWTexture2D<float4> outputTex : register(u0);
SamplerState samp;


// TODO: Improve this mess of an if-else case
#ifdef TEXTURE_WIDTH
#ifdef TEXTURE_HEIGHT
groupshared float2 inverseScreenSize = float2((1.0 / TEXTURE_WIDTH), (1.0 / TEXTURE_HEIGHT));
#else
groupshared float2 inverseScreenSize = float2(-1.0, -1.0);
#endif
#else
groupshared float2 inverseScreenSize = float2(-1.0, -1.0);
#endif

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125

#define ITERATIONS 7U
groupshared static float QUALITY[7] = { 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };
#define SUBPIXEL_QUALITY 0.75


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
	uint	groupInDRx : SV_GroupIndex,
	uint3   groupID : SV_GroupID)
{
	float texWidth;
	float texHeight;
	float2 orig_uv = (dispaThreadID.xy + float2(0.5, 0.5)) * inverseScreenSize;

	float2 screen_pos = dispaThreadID.xy;

	GroupMemoryBarrierWithGroupSync(); //Needed?

	float3 colorCenter = inputTex.SampleLevel(samp, orig_uv, 0.0).rgb;//inputTex[screen_pos].rgb;
	float3 testCenter = inputTex[screen_pos].rgb;

	// Luma at the current fragment
	float lumaC = rgb2luma(colorCenter);

	// Luma at the four direct neighbours of the current fragment
	float lumaD	= rgb2luma(inputTex[screen_pos + float2( 0.0, 1.0)].rgb);
	float lumaU	= rgb2luma(inputTex[screen_pos + float2( 0.0,-1.0)].rgb);
	float lumaL	= rgb2luma(inputTex[screen_pos + float2(-1.0, 0.0)].rgb);
	float lumaR = rgb2luma(inputTex[screen_pos + float2( 1.0, 0.0)].rgb);

	// Find the max and min luma around the current fragment
	float lumaMin = min(lumaC, min(min(lumaD, lumaU), min(lumaL, lumaR)));
	float lumaMax = max(lumaC, max(max(lumaD, lumaU), max(lumaL, lumaR)));

	// Compare the difference
	float lumaRange = lumaMax - lumaMin;

	float4 result = float4(1.0, 1.0, 1.0, 1.0);

//#ifDRf TEXTURE_WIDTH
//	if (TEXTURE_WIDTH == 1280)
//	{
//		result = float4(1.0, 0.0, 0.0, 1.0);
//	}
//#endif

	// Early exit if no edge is DRtected in the area or if the area is really dark, 
	// or if the TEXTURE_WIDHT/HEIGHT macros have not gotten to the shader
	if (lumaRange < max(EDGE_THRESHOLD_MIN, (lumaMax * EDGE_THRESHOLD_MAX)) 
		|| inverseScreenSize.x == -1.0)						//TODO: get rid of this statement and handle teh macros better
	{
		result = float4(colorCenter, 1.0);
		/*if (testCenter.r == colorCenter.r && testCenter.g == colorCenter.g && testCenter.b == colorCenter.b)
		{
			result = float4(1.0, 0.0, 0.0, 1.0);
		}*/
	} else {
		// The four remaining corner lumaD
		float lumaDR = rgb2luma(inputTex[screen_pos + float2( 1.0, 1.0)].rgb);
		float lumaDL = rgb2luma(inputTex[screen_pos + float2(-1.0, 1.0)].rgb);
		float lumaUR = rgb2luma(inputTex[screen_pos + float2( 1.0,-1.0)].rgb);
		float lumaUL = rgb2luma(inputTex[screen_pos + float2(-1.0,-1.0)].rgb);

		// Combine the four edges' lumaD
		float lumaDU = lumaD + lumaU;
		float lumaLR = lumaL + lumaR;

		// The same is done for the corners
		float lumaRC = lumaUL + lumaDL; // Right Corner
		float lumaLC = lumaUR + lumaDR; // Left Corner
		float lumaDC = lumaDR + lumaDL; // Down Corner
		float lumaUC = lumaUR + lumaUL; // Up Corner

		// Compute an estimation of the gradient along the horizontal and vertical axiz
		float edgeHor = abs(-2.0 * lumaL + lumaLC) + abs(-2.0 * lumaC + lumaDU) * 2.0 + abs(-2.0 * lumaR + lumaRC);
		float edgeVer = abs(-2.0 * lumaU + lumaUC) + abs(-2.0 * lumaC + lumaLR) + 2.0 + abs(-2.0 * lumaD + lumaDC);

		bool isHorizontal = (edgeHor >= edgeVer);

		// Select the two neighbouring texels' lumaD in the opposite direction of the local edge
		float luma1 = isHorizontal ? lumaD : lumaL;
		float luma2 = isHorizontal ? lumaU : lumaR;
		// Compute gradients in this direction
		float grad1 = luma1 - lumaC;
		float grad2 = luma2 - lumaC;

		// Which directinon is the steepest?
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

		// Shift the texel coordinate by half a pixel. After this currentUV will be located between two pixels
		float2 currentUV = orig_uv;
		if (isHorizontal)
		{
			currentUV.y += stepLength * 0.5;
		} else {
			currentUV.x += stepLength * 0.5;
		}

		// ---------------------------- First iteration. One step in both directions --------------------------

		// Compute offset (for each iteration step) in the right directio
		float2 offset = isHorizontal ? float2(inverseScreenSize.x, 0.0) : float2(0.0, inverseScreenSize.y);
		// Compute UVs to explore each side of the edge. // NOTE: Something about QUALITY
		float2 uv1 = currentUV - offset;
		float2 uv2 = currentUV + offset;

		//inputTex.SampleLevel(samp, orig_uv, 0.0).rgb;
		// Read the lumas at both current extremities of the exploration segment, and compute the delta wrt(?) to the local average luma
		float lumaEnd1 = rgb2luma(inputTex.SampleLevel(samp, uv1, 0.0).rgb); //TODO: do this with offsets instead of uv1 and uv2
		float lumaEnd2 = rgb2luma(inputTex.SampleLevel(samp, uv2, 0.0).rgb);
		lumaEnd1 -= lumaLocalAverage;
		lumaEnd2 -= lumaLocalAverage;

		// If the luma deltas at the current extremities are larger than the local gradient, then the side of the edge has been reached
		bool reached1 = abs(lumaEnd1) >= gradScaled;
		bool reached2 = abs(lumaEnd2) >= gradScaled;

		// if the side is not reached then we continue to itearate in that direction
		if (!reached1)
		{
			uv1 -= offset;
		}
		if (!reached2)
		{
			uv2 += offset;
		}

		// If both sides have not been reached 
		if (!(reached1 && reached2))
		{
			for (uint i = 2; i < ITERATIONS; i++)
			{
				// If needed, read luma in 1st direction and compute delta.
				if (!reached1)
				{
					lumaEnd1 = rgb2luma(inputTex.SampleLevel(samp, uv1, 0.0).rgb);
					lumaEnd1 -= lumaLocalAverage;
				}

				// If needed, read luma in 2nd direction and compute delta.
				if (!reached1)
				{
					lumaEnd2 = rgb2luma(inputTex.SampleLevel(samp, uv2, 0.0).rgb);
					lumaEnd2 -= lumaLocalAverage;
				}

				// If the luma deltas at the current extremities are larger than the local gradient, 
				// then the side of the edge has been reached
				reached1 = abs(lumaEnd1) >= gradScaled;
				reached2 = abs(lumaEnd2) >= gradScaled;

				if (!reached1)
				{
					uv1 -= offset * QUALITY[i];
				}
				if (!reached2)
				{
					uv2 += offset * QUALITY[i];
				}
				// Stop when the edge has been found in both directions
				if (reached1 && reached2) { break; }
			} // end for ITERATIONS
		} // end if !reachedboth

		// Compute the distances to each extremity of the edge.
		float dist1 = isHorizontal ? (orig_uv.x - uv1.x) : (orig_uv.y - uv1.y);
		float dist2 = isHorizontal ? (uv2.x - orig_uv.x) : (uv2.y - orig_uv.y);

		// In which direction is the extremity of the edge closer?
		bool isDir1 = dist1 < dist2;
		float finalDist = min(dist1, dist2);

		float edgeLength = dist1 + dist2;

		// UV offset: read in the direction of the closest side of the edge
		float pixelOffset = -finalDist / edgeLength + 0.5;

		// is the luma at center smaller than the local average?
		bool isLumaCSmaller = lumaC < lumaLocalAverage;

		// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive
		bool correctVariation = (((isDir1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCSmaller);

		// Do not offset if the luma variation is incorrect
		float finalOffset = correctVariation ? pixelOffset : 0.0;


		// Sub-pixel shifting
		// Full weighted average of the luma over the 3x3 neighborhood
		float lumaAvg = (1.0 / 12.0) * (2.0 * (lumaDU + lumaLR) + lumaLC + lumaRC);

		//TODO: Rename
		// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighbourhood
		float subPixelOffset1 = saturate(abs(lumaAvg - lumaC) / lumaRange);
		float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
		// Compute a sub-pixel offset based on this delta
		float finalSubPixelOffset = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY; //TODO: optimize

		// Pixk the biggest of the two offsets
		finalOffset = max(finalOffset, finalSubPixelOffset);

		float2 finalUV = orig_uv;
		// Compute the final UV coordinates
		if (isHorizontal)
		{
			finalUV.y += finalOffset * stepLength;
		} else {
			finalUV.x += finalOffset * stepLength;
		}

		result = inputTex.SampleLevel(samp, finalUV, 0.0);
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