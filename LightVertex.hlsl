
struct VS_OUT
{
	float4 Position	: SV_Position;
	float2 TexCoord	: TEXCOORD;
};

// Generates a full screen triangle using vertexID which goes from 0 to 2
VS_OUT VS_main( in uint vertexID : SV_VertexID ) 
{
	VS_OUT output = (VS_OUT)0;
	
	// clip space positions: (-1,-1,0), (-1,3,0), (3,-1,0)
	float x = (float)(vertexID / 2) * 4.0 - 1.0;
	float y = (float)(vertexID % 2) * 4.0 - 1.0;

	output.Position = float4(x, y, 0.0, 1.0);

	// texture coordinates: (0,0), (0,2), (2,0)
	output.TexCoord.x = (float)(vertexID / 2) * 2.0;
	output.TexCoord.y = 1.0 - (float)(vertexID % 2) * 2.0;

	return output;
}