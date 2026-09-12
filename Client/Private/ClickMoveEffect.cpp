#include "ClickMoveEffect.h"

#include "Character.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "Transform.h"

#include <cmath>

namespace
{
	constexpr const char* CLICK_EFFECT_ID = "effect.world.mouse_click";
	constexpr const char* DESTINATION_EFFECT_ID = "effect.world.move_destination";
	constexpr f32_t CLICK_DURATION_SECONDS = 1.2f;
}

bool_t Client::CClickMoveEffect::Uses_LevelMarkers(const LEVEL level)
{
	return LEVEL::CHARACTER_SELECT == level || LEVEL::BERN == level ||
		LEVEL::VALTAN_ARENA == level || LEVEL::KAKULSAYDON_ARENA == level ||
		LEVEL::DEVELOPMENT == level;
}

std::vector<std::string> Client::CClickMoveEffect::Queue_LevelResources(const LEVEL level)
{
	std::vector<std::string> accepted;
	if (!Uses_LevelMarkers(level)) return accepted;
	// Register each optional decoration independently: one missing asset must
	// neither hide the other nor fail the playable Level's admission.
	for (const char* id : { CLICK_EFFECT_ID, DESTINATION_EFFECT_ID })
	{
		if (id == DESTINATION_EFFECT_ID && LEVEL::KAKULSAYDON_ARENA != level)
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

void Client::CClickMoveEffect::Clear()
{
	CEffectPresentationService::Stop_WorldRoot(m_ClickHandle);
	m_ClickHandle = {};
	m_pCharacter.reset();
}

void Client::CClickMoveEffect::Late_Update(const f32_t fTimeDelta)
{
	if (!m_ClickHandle.Is_Valid()) return;
	const shared_ptr<CCharacter> character = m_pCharacter.lock();
	const auto& player = CCombatHUDViewModel::Get().Get_Player();
	if (!character || CGameInstance::Get().Get_CurrentLevelID() != m_iLevelIndex ||
		!player.isValid || 0u == player.iCurrentHp || player.isPatternBound)
	{
		Clear();
		return;
	}
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f) return;
	m_fClickSeconds += fTimeDelta;
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
