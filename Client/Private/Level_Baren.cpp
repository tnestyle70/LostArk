#include "Level_Baren.h"

#include "Camera_Free.h"
#include "Character.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "NetworkManager.h"
#include "Transform.h"

#include <algorithm>

CLevel_Baren::CLevel_Baren(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{}

CLevel_Baren::~CLevel_Baren()
{}

HRESULT CLevel_Baren::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	/*
	 * 기존 베른성 맵 로딩 경로를 그대로 유지한다.
	 */
	if (!m_MapRuntime.Load_Area(
		ETOUI(LEVEL::BAREN),
		"LV_BER_BERNCASTLE"))
	{
		OutputDebugStringA(
			("[Level_Baren] " +
				m_MapRuntime.Get_Status() +
				"\n").c_str());

		return E_FAIL;
	}

	if (FAILED(CMapPlacementRuntime::Ensure_DefaultLight()) ||
		FAILED(Ready_Layer_Camera(
			TEXT("Layer_Camera"))))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	/*
	 * Server에서 받은 Player Spawn을 어느 Level과 Layer에
	 * CCharacter로 생성할 것인지 설정한다.
	 */
	CClientReplication::DESC replicationDesc{};

	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::BAREN);

	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::BAREN);

	replicationDesc.strPlayerLayerTag =
		TEXT("Layer_Player");

	if (!m_Replication.Initialize(replicationDesc))
	{
		m_MapRuntime.Clear();
		return E_FAIL;
	}

	return S_OK;
}

void CLevel_Baren::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);

	/*
	 * MainApp에서 NetworkManager::Update()가 먼저 호출된다.
	 *
	 * NetworkManager
	 * → 수신 Frame 해석
	 * → Replication Event Queue
	 *
	 * 여기서는 Queue를 실제 Engine GameObject 변경으로 적용한다.
	 */
	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_Baren] Failed to apply replication event.\n");
	}

	/*
	 * Local Player가 처음 Spawn된 프레임에 카메라를 연결한다.
	 * Remote Player는 카메라 대상이 되지 않는다.
	 */
	if (!Bind_CameraToLocalCharacter())
	{
		OutputDebugStringA(
			"[Level_Baren] Failed to bind local character camera.\n");
	}

	if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
		(GetKeyState(VK_F2) & 0x8000))
	{
		/*
		 * 베른성을 떠나기 전에 Server 연결을 닫아야 한다.
		 * Server는 연결 종료를 감지하여 다른 Client에
		 * S2C_PLAYER_DESPAWNED를 Broadcast한다.
		 */
		CNetworkManager::Get().Close_ServerConnection();

		if (FAILED(CGameInstance::Get().Change_Level(
			ETOUI(LEVEL::LOADING),
			CLevel_Loading::Create(
				m_pDevice,
				m_pContext,
				LEVEL::ASSET_TEST))))
		{
			MSG_BOX("Failed to Change Level");
		}

		return;
	}
}

HRESULT CLevel_Baren::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(
		g_hWnd,
		TEXT("Bern Castle Network Player Test"));
#endif

	return S_OK;
}

HRESULT CLevel_Baren::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	float3_t minimum{};
	float3_t maximum{};
	float3_t focus(0.f, 0.f, 0.f);

	f32_t span = 80.f;

	if (m_MapRuntime.Try_Get_PlacementBounds(
		minimum,
		maximum))
	{
		focus = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);

		span = (std::max)(
			maximum.x - minimum.x,
			maximum.z - minimum.z);

		span = (std::clamp)(
			span,
			40.f,
			5000.f);
	}

	const f32_t distance =
		(std::max)(40.f, span * 0.7f);

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};

	/*
	 * Player가 Spawn되기 전까지는 기존 베른성 전체 맵 카메라다.
	 */
	cameraDesc.vEye = float3_t(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);

	cameraDesc.vAt = focus;

	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar =
		(std::max)(2000.f, span * 8.f);

	cameraDesc.fSpeedPerSec =
		(std::max)(20.f, span * 0.08f);

	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	/*
	 * Player Spawn 이후 사용할 Follow Camera 설정이다.
	 * Initialize 시점에는 Player가 없으므로 비활성화한다.
	 */
	cameraDesc.pFollowTarget = nullptr;

	cameraDesc.vPositionOffset =
		float3_t(0.4f, 7.5f, 4.5f);

	cameraDesc.vLookOffset =
		float3_t(0.f, 1.2f, 0.f);

	cameraDesc.fFollowResponse = 18.f;
	cameraDesc.isFollowEnabled = false;

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

	m_pCamera =
		dynamic_pointer_cast<CCamera_Free>(
			gameObject);

	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::BAREN),
			strLayerTag,
			gameObject);

		return E_FAIL;
	}

	return S_OK;
}

bool_t CLevel_Baren::Bind_CameraToLocalCharacter()
{
	if (nullptr == m_pCamera)
		return false;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();

	/*
	 * 아직 Local Spawn Event가 도착하지 않았거나
	 * Local Character가 Despawn된 상태다.
	 */
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();

		m_pCamera->Set_FollowTarget(nullptr);
		m_pCamera->Set_FollowEnabled(false);

		return true;
	}

	/*
	 * 이미 같은 Character에 연결되어 있으면 매 프레임
	 * Camera Target을 다시 설정하지 않는다.
	 */
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();

	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;

	m_pCamera->Set_PositionOffset(
		float3_t(0.4f, 7.5f, 4.5f));

	m_pCamera->Set_FollowTarget(transform);
	m_pCamera->Set_FollowEnabled(true);

	return true;
}

unique_ptr<CLevel_Baren> CLevel_Baren::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance =
		unique_ptr<CLevel_Baren>(
			new CLevel_Baren(
				pDevice,
				pContext));

	if (FAILED(instance->Initialize()))
		return nullptr;

	return instance;
}