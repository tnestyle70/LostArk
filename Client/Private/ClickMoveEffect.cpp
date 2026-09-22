#include "ClickMoveEffect.h"

#include "Character.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "Transform.h"
#include "WorldPlayerNameplateView.h"

#include <cmath>

namespace
{
	constexpr const char* CLICK_EFFECT_ID = "effect.world.mouse_click";
	constexpr const char* DESTINATION_EFFECT_ID = "effect.world.move_destination";
	constexpr f32_t CLICK_DURATION_SECONDS = 1.2f;
	constexpr const char* PING_EFFECT_ID = "effect.world.ping";
	constexpr const char* PENDING_EFFECT_ID = "effect.world.target_reticle";
	constexpr f32_t PING_DURATION_SECONDS = 3.f;
}

bool_t Client::CClickMoveEffect::Uses_LevelMarkers(const LEVEL level)
{
	return LEVEL::CHARACTER_SELECT == level || LEVEL::BERN == level ||
		LEVEL::VALTAN_ARENA == level || LEVEL::KAKULSAYDON_ARENA == level ||
		LEVEL::DEVELOPMENT == level || LEVEL::MAHARAKA == level;
}

std::vector<std::string> Client::CClickMoveEffect::Queue_LevelResources(const LEVEL level)
{
	std::vector<std::string> accepted;
	if (!Uses_LevelMarkers(level)) return accepted;
	// Register each optional decoration independently: one missing asset must
	// neither hide the other nor fail the playable Level's admission.
	for (const char* id : { CLICK_EFFECT_ID, DESTINATION_EFFECT_ID, PING_EFFECT_ID, PENDING_EFFECT_ID })
	{
		if (id == DESTINATION_EFFECT_ID &&
			LEVEL::KAKULSAYDON_ARENA != level && LEVEL::VALTAN_ARENA != level)
			continue;
		std::vector<std::string> target;
		std::string status;
		if (CEffectPresentationService::Queue_ProductTargets_Priority({ id }, target, status))
			accepted.insert(accepted.end(), target.begin(), target.end());
		else
			OutputDebugStringA(("[ClickMoveEffect] Optional preparation isolated: " + status + "\n").c_str());
	}
	return accepted;
}

Client::CClickMoveEffect::CClickMoveEffect(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext)) {}

Client::CClickMoveEffect::CClickMoveEffect(const CClickMoveEffect& prototype)
	: CGameObject(prototype) {}

Client::CClickMoveEffect::~CClickMoveEffect() { Clear(); }

HRESULT Client::CClickMoveEffect::Initialize_Prototype() { return S_OK; }

HRESULT Client::CClickMoveEffect::Initialize(void* pArg)
{
	return __super::Initialize(pArg);
}

bool_t Client::CClickMoveEffect::Initialize_Effects(const uint32_t levelIndex)
{
	if (levelIndex >= ETOUI(LEVEL::END) || !Uses_LevelMarkers(static_cast<LEVEL>(levelIndex)))
		return false;
	m_iLevelIndex = levelIndex;
	return true;
}

void Client::CClickMoveEffect::Report_Failure(const std::string& status)
{
	if (status == m_strLastFailure) return;
	m_strLastFailure = status;
	OutputDebugStringA(("[ClickMoveEffect] Cosmetic feedback isolated: " + status + "\n").c_str());
}

void Client::CClickMoveEffect::Play(const float3_t& worldPosition,
	const shared_ptr<CCharacter>& character)
{
	if (!character || CGameInstance::Get().Get_CurrentLevelID() != m_iLevelIndex ||
		!std::isfinite(worldPosition.x) || !std::isfinite(worldPosition.y) ||
		!std::isfinite(worldPosition.z)) return;

	// The Controller calls this only after Request_MoveGoal succeeds. That is a
	// queued command receipt, not a Server path/goal-acceptance acknowledgement.
	m_pCharacter = character;
	float4x4_t rootWorld{};
	XMStoreFloat4x4(&rootWorld, XMMatrixTranslation(worldPosition.x,
		worldPosition.y + 0.035f, worldPosition.z));
	m_fClickSeconds = 0.f;
	CEffectPresentationService::Stop_WorldRoot(m_ClickHandle);
	m_ClickHandle = {};
	EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
	desc.iLevelIndex = m_iLevelIndex;
	desc.strPlacementId = "player.move.click";
	desc.strEffectAssetId = CLICK_EFFECT_ID;
	desc.RootWorld = rootWorld;
	std::string status;
	if (!CEffectPresentationService::Spawn_LevelPlacement(desc, m_ClickHandle, status))
		Report_Failure(status);
}

bool_t Client::CClickMoveEffect::Spawn_Marker(const char* effectId,
	const char* placementId, const float3_t& position,
	EFFECT_WORLD_ROOT_HANDLE& handle, const bool_t sustained)
{
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
		return false;
	EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
	desc.iLevelIndex = m_iLevelIndex;
	desc.strPlacementId = placementId;
	desc.strEffectAssetId = effectId;
	desc.bOwnerSustainedSourceLoops = sustained;
	XMStoreFloat4x4(&desc.RootWorld, XMMatrixTranslation(position.x, position.y, position.z));
	EFFECT_WORLD_ROOT_HANDLE staged;
	std::string status;
	if (!CEffectPresentationService::Spawn_LevelPlacement(desc, staged, status))
	{
		Report_Failure(status);
		return false;
	}
	CEffectPresentationService::Stop_WorldRoot(handle);
	handle = staged;
	return true;
}

void Client::CClickMoveEffect::Play_Ping(const float3_t& worldPosition,
	const shared_ptr<CCharacter>& character)
{
	if (!character || CGameInstance::Get().Get_CurrentLevelID() != m_iLevelIndex) return;
	const float3_t position{ worldPosition.x, worldPosition.y + 0.035f, worldPosition.z };
	if (Spawn_Marker(PING_EFFECT_ID, "player.ping", position, m_PingHandle))
	{
		m_pCharacter = character;
		m_fPingSeconds = 0.f;
	}
}

void Client::CClickMoveEffect::Set_PingPending(const bool_t pending,
	const shared_ptr<CCharacter>& character)
{
	m_bPingPending = pending && character != nullptr;
	if (m_bPingPending) m_pCharacter = character;
	else
	{
		CEffectPresentationService::Stop_WorldRoot(m_PendingHandle);
		m_PendingHandle = {};
	}
}

void Client::CClickMoveEffect::Clear()
{
	CEffectPresentationService::Stop_WorldRoot(m_ClickHandle);
	m_ClickHandle = {};
	CEffectPresentationService::Stop_WorldRoot(m_PingHandle);
	CEffectPresentationService::Stop_WorldRoot(m_PendingHandle);
	m_PingHandle = {};
	m_PendingHandle = {};
	m_bPingPending = false;
	m_pCharacter.reset();
}

void Client::CClickMoveEffect::Late_Update(const f32_t fTimeDelta)
{
	if (!m_ClickHandle.Is_Valid() && !m_PingHandle.Is_Valid() && !m_bPingPending) return;
	const shared_ptr<CCharacter> character = m_pCharacter.lock();
	const auto& player = CCombatHUDViewModel::Get().Get_Player();
	if (!character || CGameInstance::Get().Get_CurrentLevelID() != m_iLevelIndex ||
		!player.isValid || 0u == player.iCurrentHp || player.isPatternBound)
	{
		Clear();
		return;
	}
	if (m_bPingPending)
	{
		float3_t head{};
		if (CWorldPlayerNameplateView::Try_GetHeadAnchor(*character, head))
		{
			head.y += 0.55f;
			if (!m_PendingHandle.Is_Valid())
				(void)Spawn_Marker(PENDING_EFFECT_ID, "player.ping.pending", head, m_PendingHandle, true);
			else
			{
				float4x4_t world{};
				XMStoreFloat4x4(&world, XMMatrixTranslation(head.x, head.y, head.z));
				if (!CEffectPresentationService::Update_WorldRoot(m_PendingHandle, world))
				{
					CEffectPresentationService::Stop_WorldRoot(m_PendingHandle);
					m_PendingHandle = {};
				}
			}
		}
	}
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f) return;
	m_fClickSeconds += fTimeDelta;
	m_fPingSeconds += fTimeDelta;
	if (m_PingHandle.Is_Valid() && m_fPingSeconds >= PING_DURATION_SECONDS)
	{
		CEffectPresentationService::Stop_WorldRoot(m_PingHandle);
		m_PingHandle = {};
	}
	if (m_ClickHandle.Is_Valid() && m_fClickSeconds >= CLICK_DURATION_SECONDS)
	{
		CEffectPresentationService::Stop_WorldRoot(m_ClickHandle);
		m_ClickHandle = {};
	}
}

HRESULT Client::CClickMoveEffect::Render() { return S_OK; }

unique_ptr<Client::CClickMoveEffect> Client::CClickMoveEffect::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CClickMoveEffect>(
		new CClickMoveEffect(std::move(pDevice), std::move(pContext)));
	if (FAILED(instance->Initialize_Prototype()))
		return nullptr;
	return instance;
}

shared_ptr<CPrototype> Client::CClickMoveEffect::Clone(void* pArg)
{
	auto instance = shared_ptr<CClickMoveEffect>(
		new CClickMoveEffect(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
