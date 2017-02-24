/*
* TODO: Should probably move MAX_LIGHTS to this file and rename it
* The cLightBuffer might have to be public
*/


#ifndef LIGHTHANDLER_HPP
#define LIGHTHANDLER_HPP

#include "GlobalResources.hpp"

const int NR_OF_LIGHTS = 1;

class LightHandler
{
public:
	LightHandler(DirectX::XMVECTOR CAM_POS);
	~LightHandler();

	bool InitializeLights(ID3D11Device* Dev, DirectX::XMVECTOR CAM_POS);
	bool BindLightBuffer(ID3D11DeviceContext* DevCon, DirectX::XMVECTOR &CAM_POS);
	bool CreateLightBuffer(ID3D11Device* Dev);
private:
	struct Light
	{
		Light(DirectX::XMFLOAT4 pos = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
			DirectX::XMFLOAT4 col = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
			float c_att = 1.0f,
			float l_att = 0.0f,
			float q_att = 0.0f,
			float amb = 0.0f)
			: PositionWS(pos),
			Color(col),
			constantAttenuation(c_att),
			linearAttenuation(l_att),
			quadraticAttenuation(q_att),
			ambientCoefficient(amb)
		{}

		DirectX::XMFLOAT4 PositionWS;
		DirectX::XMFLOAT4 Color;
		float constantAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;
		float ambientCoefficient;
	};

	struct cLightBuffer
	{
		cLightBuffer() 
		{
			//DirectX::XMStoreFloat4(&cameraPositionWS, CAM_POS);
			globalAmbient = DirectX::XMFLOAT4(0.05f, 0.05f, 0.05f, 0.05f);

			for (int i = 0; i < NR_OF_LIGHTS; i++)
			{
				LightArray[i] = Light();
			}
		}

		Light LightArray[NR_OF_LIGHTS];
		DirectX::XMFLOAT4 cameraPositionWS;
		DirectX::XMFLOAT4 globalAmbient;
	};
	static_assert((sizeof(cLightBuffer) % 16) == 0, "Constant Light Buffer size must be 16-byte aligned");

	cLightBuffer mLightBufferData;
	ID3D11Buffer* mLightBuffer;
};


#endif // !LIGHTHANDLER_HPP
