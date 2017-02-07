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
struct VS_IN
{
	float3 Pos : POSITION;
	float2 Texcoord : TEXCOORD;
	//float3 Color:COLOR;
	float3 Normal:NORMAL;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Texcoord : TEXCOORD;
	float4 Normal:NORMAL;
	//float3 Color:COLOR;
};
//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
	output.Pos=float4(input.Pos,1.0f);
	//output.Pos = mul(float4(input.Pos, 1.0f),mul(worldMatrix,mul(viewMatrix,projectionMatrix)));
	output.Texcoord = input.Texcoord;
	output.Normal = float4(input.Normal,0.0f);
//	output.Color=input.Color;

	return output;
}