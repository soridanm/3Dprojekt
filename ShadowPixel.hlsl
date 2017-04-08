/**
* Course: DV142 - 3D-Programming
* Authors: Viktor Enfeldt, Peter Meunier
*
* File: ShadowPixel.hlsl
*
* File summary: Pixel shader of the Shadow pass
*	Fills in the depth texture that's used in the light pass
*/

struct PS_IN
{
	float4 PositionCS : SV_POSITION;
};

void PS_main(in PS_IN input) 
{}
