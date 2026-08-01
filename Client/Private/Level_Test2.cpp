#include "Level_Test2.h"

#include "Camera_Free.h"
#include "GameInstance.h"
#include "Character.h"

#include "Logic_LanceMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_Artist.h"
#include "Logic_Slayer.h"

#include "Npc.h"

CLevel_Test2::CLevel_Test2(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{}

CLevel_Test2::~CLevel_Test2()
{}

HRESULT CLevel_Test2::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	const wchar_t* pCommandLine = GetCommandLineW();
	const bool_t isHDRReadbackRequested =
		nullptr != pCommandLine &&
		nullptr != wcsstr(pCommandLine, L"--hdr-readback");
	if (!isHDRReadbackRequested &&
		FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
	{
		return E_FAIL;
	}
	if (FAILED(Ready_Layer_Npc(TEXT("Layer_Npc"))))
		return E_FAIL;
	return S_OK;
}

void CLevel_Test2::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CLevel_Test2::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Test2::Ready_Lights()
{
	LIGHT_DESC light{};
	light.eType = LIGHT::DIRECTIONAL;
	light.vDirection = float4_t(0.5f, -1.f, 0.5f, 0.f);
	light.vDiffuse = float4_t(0.8f, 0.8f, 0.8f, 1.f);
	light.vAmbient = float4_t(0.35f, 0.35f, 0.35f, 1.f);
	light.vSpecular = float4_t(0.5f, 0.5f, 0.5f, 1.f);
	return CGameInstance::Get().Add_Light(light);
}

HRESULT CLevel_Test2::Ready_Layer_Camera(const wstring_t& strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	CameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	CameraDesc.fFovy = 60.f;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.fSpeedPerSec = 20.f;
	CameraDesc.fRotationPerSec = 90.f;
	CameraDesc.fMouseSensor = 0.1f;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::TEST_LEVEL2), strLayerTag, &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Test2::Ready_Layer_Player(const wstring_t& strLayerTag)
{
	/* One character at a time: a full class costs ~100 MB of decoded model and
	animation, so the loader registers only the class spawned here. Swap both this
	spec and the matching Ready_*_Prototypes call in CLoader together. */
	CCharacter::CHARACTER_DESC		CharacterDesc{};
	CharacterDesc.iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
	CharacterDesc.pSpec = &Spec_Slayer;
	CharacterDesc.vPosition = float3_t(0.f, 0.f, 0.f);

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::TEST_LEVEL2),
		TEXT("Prototype_GameObject_Character"),
		ETOUI(LEVEL::TEST_LEVEL2), strLayerTag, &CharacterDesc)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CLevel_Test2> CLevel_Test2::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_Test2>(new CLevel_Test2(pDevice, pContext));
	if (FAILED(pInstance->Initialize()))
		return nullptr;
	return pInstance;
}

HRESULT CLevel_Test2::Ready_Layer_Npc(const wstring_t& strLayerTag)
{
	/* The pilot four, stood in a row so the cook can be eyeballed: a standard
	female, a standard male, one carrying grafted bones (towel and fingers the
	archetype rig lacks) and one self-rigged NPC that owns its own animation. */
	static const struct
	{
		const tchar_t* pModelTag;
		const char_t* pClip;
		float3_t vPosition;
	} Npcs[] =
	{
		{ TEXT("Prototype_Component_Model_Npc_Aylara"),  "npc_idle_normal_1", { -3.f, 0.f, 3.f } },
		{ TEXT("Prototype_Component_Model_Npc_Forman"),  "npc_idle_normal_1", { -1.f, 0.f, 3.f } },
		{ TEXT("Prototype_Component_Model_Npc_Schmidt"), "npc_idle_normal_1", {  1.f, 0.f, 3.f } },
		{ TEXT("Prototype_Component_Model_Npc_Beda"),    "npc_idle_normal_1", {  3.f, 0.f, 3.f } },
	};

	for (const auto& npc : Npcs)
	{
		CNpc::NPC_DESC NpcDesc{};
		NpcDesc.iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
		NpcDesc.strModelTag = npc.pModelTag;
		NpcDesc.strShaderTag = TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		NpcDesc.pIdleClip = npc.pClip;
		NpcDesc.vPosition = npc.vPosition;

		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			ETOUI(LEVEL::TEST_LEVEL2),
			TEXT("Prototype_GameObject_Npc"),
			ETOUI(LEVEL::TEST_LEVEL2), strLayerTag, &NpcDesc)))
			return E_FAIL;
	}

	return S_OK;
}
