//--------------------------------- Geometry Shader -----------------------------------
// NOTE: Does not transpose normals correctly when non-uniform scaling is applied
//--------------------------------------------------------------------------------------

// WS - world space
// CS - clip-space

//TODO: Put cameraPositino in its own buffer

cbuffer PER_FRAME		: register(b0)
{
	//float4x4 viewProjection;
	float4x4 view;
	float4x4 projection;
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

	float4x4 viewProjection = mul(view, projection);

	//calculate and transpose face normals
	float3 vect1	= input[0].Position.xyz - input[1].Position.xyz;
	float3 vect2	= input[0].Position.xyz - input[2].Position.xyz;
	float3 face_normal = cross(vect1, vect2);
	float3 face_normalWS = mul(face_normal, (float3x3)worldMatrix);
	
	for (uint i = 0; i < 3; i++)
	{
		// Convert position to world space
		output.PositionWS = mul(input[i].Position, worldMatrix).xyz;

		if (i == 0) /* Back-face culling */
		{
			int d = sign(dot(cameraPosition.xyz - output.PositionWS, face_normalWS));
			if (d == -1) 
			{
				//return;
			}
		}

		// Calculate clip-space position
		output.PositionCS = mul(float4(output.PositionWS, 1.0), viewProjection);

		output.NormalWS = normalize(mul(input[i].Normal, (float3x3)worldMatrix));
		
		// Pass along the texture coordinates
		output.TexCoord = input[i].TexCoord;

		outStream.Append(output);
	}
	outStream.RestartStrip();
}