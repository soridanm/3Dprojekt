//--------------------------------- Geometry Shader -----------------------------------
// NOTE: Does not transpose normals correctly when non-uniform scaling is applied
//--------------------------------------------------------------------------------------

// WS - world space
// CS - clip-space

cbuffer WORLDMATRIX		: register(b1) 
{
	float4x4 worldMatrix;
};
cbuffer VIEWMATRIX		: register (b2) 
{
	float4x4 viewMatrix;
};
cbuffer PROJECTIONMATRIX	: register (b3) 
{
	float4x4 projectionMatrix;
};


struct GS_IN
{
	float4 Position		: POSITION;
	float2 TexCoord		: TEXCOORD0;
};

struct GS_OUT
{
	float4 PositionCS		: SV_Position;
	float2 TexCoord		: TEXCOORD;
	float3 NormalWS		: NORMALWS;
	float3 PositionWS		: POSITIONWS;
};

//struct GeoOut {
//	float4 Pos		:SV_POSITION;
//	float2 Texcoord	:TEXCOORD;
//	//float3 Color:COLOR;
//	float4 Normal	:NORMAL;
//	float4 worldPos	:POSITION;
//};
//struct VertexIn {
//	float4 Pos:SV_POSITION;
//	float2 Texcoord:TEXCOORD;
//	//float3 Color:COLOR;
//};



[maxvertexcount(6)]
void GS_main (triangle GS_IN input[3], inout TriangleStream <GS_OUT> outStream) 
{
	GS_OUT output = (GS_OUT)0;

	//calculate ViewProjection matrix
	matrix VP = mul(viewMatrix, projectionMatrix);

	//calculate and transpose normal
	float3 vect1 = input[0].Position.xyz - input[1].Position.xyz;
	float3 vect2 = input[0].Position.xyz - input[2].Position.xyz;
	float3 normal = (normalize(cross(vect1, vect2)));

	// Convert normals to world space
	output.NormalWS = normalize(mul(normal, (float3x3)worldMatrix));

	for (uint i = 0; i < 3; i++)
	{
		// Convert position to world space
		output.PositionWS = mul(input[i].Position, worldMatrix).xyz;

		// Calculate clip-space position
		output.PositionCS = mul(float4(output.PositionWS, 1.0f), VP);

		// Pass along the texture coordinates
		output.TexCoord = input[i].TexCoord;

		outStream.Append(output);
	}
	outStream.RestartStrip();

	//	---

	//GeoOut geOut = (GeoOut)0;
	//
	//float4 norm[3];
	//for (int h = 0; h < 3; h++) {
	//	geOut.Pos = mul(thingOut[h].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));
	//	norm[h] = geOut.Pos;
	//}
	//geOut.Normal = mul(float4(cross(norm[1].xyz - norm[0].xyz, norm[2].xyz - norm[0].xyz),0.0f), projectionMatrix);
	//for (int i = 0; i < 3; i++) {
	//	geOut.Pos = mul(thingOut[i].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));
	//	geOut.Texcoord = thingOut[i].Texcoord;
	//	geOut.worldPos = mul(thingOut[i].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));
	//	//geOut.Color=thingOut[i].Color;
	//	triStream.Append(geOut);
	//}
	//
	////geOut.Normal = cross(thingOut[0].Pos- thingOut[1].Pos, thingOut[0].Pos - thingOut[2].Pos);
	//triStream.RestartStrip();
	//for (int j = 0; j< 3; j++) {

	//	geOut.Pos =geOut.Normal + mul(thingOut[j].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));

	////	geOut.Pos =mul(float4(geOut.Normal, 0.0f), mul(worldMatrix, mul(viewMatrix, projectionMatrix))) + mul(thingOut[j].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));

	//geOut.Texcoord = thingOut[j].Texcoord;
	//geOut.worldPos = mul(thingOut[j].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));

	////	geOut.Color=thingOut[j].Color;
	//	triStream.Append(geOut);
	//}
	//triStream.RestartStrip();
}