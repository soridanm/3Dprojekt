//--------------------------------- Geometry Shader -----------------------------------
// NOTE: Does not transpose normals correctly when non-uniform scaling is applied
//--------------------------------------------------------------------------------------

// WS - world space
// CS - clip-space

cbuffer PER_FRAME		: register(b0)
{
	//float4x4 viewProjection;
	float4x4 view;
	float4x4 projection;
};

cbuffer PER_OBJECT		: register(b1)
{
	float4x4 worldMatrix;
};


struct GS_IN
{
	float4 Position		: POSITION;
	float2 TexCoord		: TEXCOORD;
	float3 Normal		: NORMAL;
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

	// FLAT SHADING
	/*float3 vect1	= input[0].Position.xyz - input[1].Position.xyz;
	float3 vect2	= input[0].Position.xyz - input[2].Position.xyz;
	float3 normal	= (normalize(cross(vect1, vect2)));

	output.NormalWS = mul(input[0].Normal, (float3x3)worldMatrix);*/

	for (uint i = 0; i < 3; i++)
	{
		// Convert position to world space
		output.PositionWS = mul(input[i].Position, worldMatrix).xyz;

		// Calculate clip-space position
		output.PositionCS = mul(float4(output.PositionWS, 1.0), viewProjection);
		// Convert normals to world space
		output.NormalWS = mul(input[i].Normal, (float3x3)worldMatrix);


		// Pass along the texture coordinates
		output.TexCoord = input[i].TexCoord;

		outStream.Append(output);
	}
	outStream.RestartStrip();
}