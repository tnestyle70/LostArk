#include "Level_Baren.h"

#include "Camera_Free.h"
#include "GameInstance.h"
#include "Level_Loading.h"

#include <algorithm>

CLevel_Baren::CLevel_Baren(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{pDevice, pContext}
{}

CLevel_Baren::~CLevel_Baren()
{}

HRESULT CLevel_Baren::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (!m_MapRuntime.Load_Area(
		ETOUI(LEVEL::BAREN), "LV_BER_BERNCASTLE"))
	{
		OutputDebugStringA(("[Level_Baren] " +
			m_MapRuntime.Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	if (FAILED(CMapPlacementRuntime::Ensure_DefaultLight()) ||
		FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	return S_OK;
}

void CLevel_Baren::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
		(GetKeyState(VK_F2) & 0x8000))
	{
		if (FAILED(CGameInstance::Get().Change_Level(
			ETOUI(LEVEL::LOADING),
			CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::ASSET_TEST))))
		{
			MSG_BOX("Failed to Change Level");
			return;
		}

		return;
	}

}

HRESULT CLevel_Baren::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("Bern Castle Map"));
#endif

	return S_OK;
}

HRESULT CLevel_Baren::Ready_Layer_Camera(const wstring_t& strLayerTag)
{
	float3_t minimum{};
	float3_t maximum{};
	float3_t focus(0.f, 0.f, 0.f);
	f32_t span = 80.f;
	if (m_MapRuntime.Try_Get_PlacementBounds(minimum, maximum))
	{
		focus = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		span = (std::max)(
			maximum.x - minimum.x,
			maximum.z - minimum.z);
		span = (std::clamp)(span, 40.f, 5000.f);
	}

	const f32_t distance = (std::max)(40.f, span * 0.7f);
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);
	cameraDesc.vAt = focus;
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = (std::max)(2000.f, span * 8.f);
	cameraDesc.fSpeedPerSec = (std::max)(20.f, span * 0.08f);
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::BAREN),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::BAREN),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	m_pCamera = dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::BAREN), strLayerTag, gameObject);
		return E_FAIL;
	}

	return S_OK;
}

unique_ptr<CLevel_Baren> CLevel_Baren::Create(ComPtr<ID3D11Device> pDevice, 
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_Baren>(new CLevel_Baren(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
		return nullptr;
	
	return pInstance;
}
