struct GeoOut {
	float4 Pos:SV_POSITION;
	float2 Texcoord:TEXCOORD;
	float3 Color:COLOR;
	float4 Normal:NORMAL;
	float4 worldPos:POSITION;
};

struct VertexIn {
	float4 Pos:SV_POSITION;
	float2 Texcoord:TEXCOORD;
	float4 Normal : NORMAL;
	//float3 Color:COLOR;
};
cbuffer WORLDMATRIX:register(b1) {
	float4x4 worldMatrix;
};
cbuffer VIEWMATRIX : register (b2) {
	float4x4 viewMatrix;
};
cbuffer PROJECTIONMATRIX : register (b3) {
	float4x4 projectionMatrix;
};
cbuffer CAMPOS:register(b4) {
	float3 campos;
}



[maxvertexcount(6)]
void GS_main (triangle VertexIn thingOut[3], inout TriangleStream<GeoOut> triStream) {
	GeoOut geOut = (GeoOut)0;
	
	//float4 norm[3];
	//for (int h = 0; h < 3; h++) {
	//	geOut.Pos = mul(thingOut[h].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));
	//	norm[h] = geOut.Pos;
	//}
	//geOut.Normal = mul(float4(cross(norm[1].xyz - norm[0].xyz, norm[2].xyz - norm[0].xyz),0.0f), projectionMatrix);
	for (int i = 0; i < 3; i++) {
		geOut.Pos = mul(thingOut[i].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));
		geOut.Texcoord = thingOut[i].Texcoord;
		geOut.Normal = thingOut[i].Normal;
		geOut.worldPos = mul(thingOut[i].Pos, mul(worldMatrix, mul(viewMatrix, projectionMatrix)));
		//geOut.Color=thingOut[i].Color;
		geOut.Color = campos;
		triStream.Append(geOut);
	}
	
	//geOut.Normal = cross(thingOut[0].Pos- thingOut[1].Pos, thingOut[0].Pos - thingOut[2].Pos);
	triStream.RestartStrip();
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