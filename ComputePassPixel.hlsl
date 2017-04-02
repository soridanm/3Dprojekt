/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: Handler.hpp
*
* File summary:
*
*
*
*
*
*/

Texture2D InputTexture : register(t0);

struct PS_IN
{
	float4 PositionCS	: SV_Position;
};

float4 PS_main( PS_IN input ) : SV_TARGET
{
	return float4(InputTexture.Load(int3(input.PositionCS.xy, 0)));
}