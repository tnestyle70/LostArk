#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>

NS_BEGIN(Client)

#ifdef _DEBUG
struct ESTHER_CUTIN_TUNING
{
	f32_t fRectX;
	f32_t fRectY;
	f32_t fRectWidth;
	f32_t fRectHeight;
	f32_t fModelYawDegrees;
	f32_t fEyeXPerHeight;
	f32_t fEyeYPerHeight;
	f32_t fDistancePerHeight;
	f32_t fAtYPerHeight;
	f32_t fFovDegrees;
};
#endif

class CEstherCutinPresentationService final
{
public:
	CEstherCutinPresentationService() = delete;

	static HRESULT Render(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

#ifdef _DEBUG
	static ESTHER_CUTIN_TUNING& Debug_Tuning();
	static void Debug_ResetTuning();
	static bool_t Debug_Preview(const std::string& archetypeId);
#endif
};

NS_END
