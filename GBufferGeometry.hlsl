//--------------------------------- Geometry Shader -----------------------------------
// NOTE: Does not transpose normals correctly when non-uniform scaling is applied
//--------------------------------------------------------------------------------------

// WS - world space
// CS - clip-space

//TODO: Put cameraPositino in its own buffer

cbuffer PER_FRAME		: register(b0)
{
	float4x4 viewProjection;
	float4 cameraPosition;
};

cbuffer PER_OBJECT		: register(b1)
{
	float4x4 worldMatrix;
};


struct GS_IN
{
	float4 Position		: POSITION;
	float2 TexCoord		: TEXCOORD;
	float3 Normal       : NORMAL;
};

struct GS_OUT
{
	float4 PositionCS	: SV_Position;
	float2 TexCoord		: TEXCOORD;
	float3 NormalWS		: NORMALWS;
	float3 PositionWS	: POSITIONWS;
};

[maxvertexcount(6)]
void GS_main (triangle GS_IN input[3], inout TriangleStream <GS_OUT> outStream) 
{
	GS_OUT output = (GS_OUT)0;


	//float4x4 viewProjection = mul(view, projection);

	//calculate and transpose face normals
	/*float3 vect1	= input[0].Position.xyz - input[1].Position.xyz;
	float3 vect2	= input[0].Position.xyz - input[2].Position.xyz;
	float3 face_normal = cross(vect1, vect2);
	float3 face_normalWS = mul(face_normal, (float3x3)worldMatrix);*/
	

	float3 face_normal = normalize(mul(cross(input[0].Position.xyz - input[1].Position.xyz, input[0].Position.xyz - input[2].Position.xyz), (float3x3)worldMatrix));

	//int test = 0;
	//new

	// Convert position to world space
	output.PositionWS = mul(input[0].Position, worldMatrix).xyz;

	if (dot(normalize(cameraPosition.xyz - output.PositionWS), face_normal) < 0)
	{
		return;
	}

	/* Back-face culling */
	/*int d = sign(dot(cameraPosition.xyz - output.PositionWS, face_normalWS));
	if (d == -1)
	{
		return;
	}*/
	
	// Calculate clip-space position
	output.PositionCS = mul(float4(output.PositionWS, 1.0), viewProjection);

	/*if (output.PositionCS.z < 0.0)
	{
		test++;
	}*/

	//output.NormalWS = normalize(mul(float4(input[0].Normal, 0.0), worldMatrix)).xyz;
	output.NormalWS = normalize((input[0].Normal.x*worldMatrix[0] + (input[0].Normal.y*worldMatrix[1] + input[0].Normal.z*worldMatrix[2])).xyz);

	//output.NormalWS = face_normalWS;

	// Pass along the texture coordinates
	output.TexCoord = input[0].TexCoord;

	outStream.Append(output);

	for (uint i = 1; i < 3; i++)
	{
		// Convert position to world space
		output.PositionWS = mul(input[i].Position, worldMatrix).xyz;

		// Calculate clip-space position
		//output.PositionCS = mul(float4(output.PositionWS, 1.0), viewProjection);
		output.PositionCS = output.PositionWS.x*viewProjection[0] + (output.PositionWS.y*viewProjection[1] + (output.PositionWS.z*viewProjection[2] + viewProjection[3]));
		/*if (output.PositionCS.z < 0.0)
		{
			test++;
			if (test == 3) {
				return;
			}
		}*/



		//output.NormalWS = normalize(mul(float4(input[i].Normal, 0.0), worldMatrix)).xyz;
		output.NormalWS = normalize((input[i].Normal.x*worldMatrix[0] + (input[i].Normal.y*worldMatrix[1] + input[i].Normal.z*worldMatrix[2])).xyz);
		//output.NormalWS = face_normalWS;

		// Pass along the texture coordinates
		output.TexCoord = input[i].TexCoord;

		outStream.Append(output);
	}
	outStream.RestartStrip();


	//old
	//for (uint i = 0; i < 3; i++)
	//{
	//	// Convert position to world space
	//	output.PositionWS = mul(input[i].Position, worldMatrix).xyz;

	//	if (i == 0) /* Back-face culling */
	//	{
	//		int d = sign(dot(cameraPosition.xyz - output.PositionWS, face_normalWS));
	//		if (d == -1) 
	//		{
	//			return;
	//		}
	//	}

	//	// Calculate clip-space position
	//	output.PositionCS = mul(float4(output.PositionWS, 1.0), viewProjection);

	//	output.NormalWS = normalize(mul(float4(input[i].Normal, 0.0), worldMatrix)).xyz;
	//	//output.NormalWS = face_normalWS;

	//	// Pass along the texture coordinates
	//	output.TexCoord = input[i].TexCoord;

	//	outStream.Append(output);
	//}
	//outStream.RestartStrip();
}