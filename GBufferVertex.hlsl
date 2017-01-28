// NEW
// each constant buffer uses a register number (0)
// which has to match the number used in the CPU when
// setting the buffer for the shader to be able to use it.
//cbuffer EXAMPLE_BUFFER : register(b0)
//{
//	float value1;
//	float value2;
//	float value3;
//	float value4;
//};
//cbuffer WORLDMATRIX:register(b1) {
//	float4x4 worldMatrix;
//}
//cbuffer VIEWMATRIX : register (b2) {
//	float4x4 viewMatrix;
//}
//cbuffer PROJECTIONMATRIX : register (b3) {
//	float4x4 projectionMatrix;
//}
//struct VS_IN
//{
//	float3 Pos : POSITION;
//	float2 Texcoord : TEXCOORD;
//	//float3 Color:COLOR;
//};
//
//struct VS_OUT
//{
//	float4 Pos : POSITION;
//	float2 Texcoord : TEXCOORD;
//	//float3 Color:COLOR;
//};
////-----------------------------------------------------------------------------------------
//// VertexShader: VSScene
////-----------------------------------------------------------------------------------------
//VS_OUT VS_main(VS_IN input)
//{
//	VS_OUT output = (VS_OUT)0;
//	output.Pos=float4(input.Pos,1.0f);
//	//output.Pos = mul(float4(input.Pos, 1.0f),mul(worldMatrix,mul(viewMatrix,projectionMatrix)));
//	output.Texcoord = input.Texcoord;
////	output.Color=input.Color;
//
//	return output;
//}  



//-----------------------------------------------------------------------------------------
//			DEFERRED VERSION
// Turns the Position into homogeneous coordinates.
// Otherwise only works as a passthrough
//-----------------------------------------------------------------------------------------

struct VS_IN
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD; // <- look up number
};

struct VS_OUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
};

VS_OUT VS_main(in VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.Position = float4(input.Position, 1.0f);
	output.TexCoord = input.TexCoord;

	return output;
}