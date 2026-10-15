// 클릭·방 핑의 World 이펙트 handle과 표시 시간을 관리한다.
#include "ClickMoveEffect.h"

// 이동 표시의 대상 Character를 weak_ptr로 연결한다.
#include "Character.h"
// 복제 player의 유효·생존·패턴 구속 상태로 이동 표시를 중지한다.
#include "CombatHUDViewModel.h"
// 현재 Level을 조회해 다른 Level로 넘어간 표시를 정리한다.
#include "GameInstance.h"
#include "Transform.h"

// 위치·delta의 유한값을 검사한다. string_view는 aura asset ID를 분류할 때 쓴다.
#include <cmath>
#include <string_view>

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
		LEVEL::VALTAN_ARENA == level || LEVEL::KOUKUSAYDON_ARENA == level ||
		LEVEL::DEVELOPMENT == level || LEVEL::MAHARAKA == level ||
		LEVEL::COLOSSEUM == level;
}

std::vector<std::string> Client::CClickMoveEffect::Queue_LevelResources(const LEVEL level)
{
	std::vector<std::string> accepted;
	if (!Uses_LevelMarkers(level)) return accepted;
	// 선택적 장식을 각각 준비해 한 asset의 실패가 다른 장식의 준비를 중단하지 않게 한다.
	// 준비 실패는 로그로만 남기고 수락한 target 목록을 계속 모은다.
	for (const char* id : { CLICK_EFFECT_ID, DESTINATION_EFFECT_ID, PING_EFFECT_ID, PENDING_EFFECT_ID,
		"effect.world.entry_aura", "effect.world.entry_aura.active", "effect.world.respawn_aura" })
	{
		if (std::string_view(id).find("aura") != std::string_view::npos &&
			LEVEL::KOUKUSAYDON_ARENA != level && LEVEL::VALTAN_ARENA != level) continue;
		if (id == DESTINATION_EFFECT_ID &&
			LEVEL::KOUKUSAYDON_ARENA != level && LEVEL::VALTAN_ARENA != level &&
			LEVEL::MAHARAKA != level)
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

// 지원 Level만 저장한다. 여기서는 이펙트 자원을 만들지 않는다.
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

// 유효한 현재 Level과 플레이어일 때 이전 클릭을 중지하고 새 클릭 피드백을 생성한다.
void Client::CClickMoveEffect::Play(const float3_t& worldPosition,
	const shared_ptr<CCharacter>& character)
{
	if (!character || CGameInstance::Get().Get_CurrentLevelID() != m_iLevelIndex ||
		!std::isfinite(worldPosition.x) || !std::isfinite(worldPosition.y) ||
		!std::isfinite(worldPosition.z)) return;

	// Controller는 Request_MoveGoal 송신 성공 뒤 호출한다.
	// Server의 경로·도착점 승인은 이후 복제 상태로 확인한다.
	m_pCharacter = character;
	float4x4_t rootWorld{};
	// 피킹한 표면 위 Y 0.035에 루트를 두어 표면과 겹침을 줄인다.
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

// 유효 위치의 임시 이펙트 생성이 성공해야 기존 handle을 교체한다.
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

// PlayerController가 받은 방 핑을 발신자·순번별 배치로 만든다. 한도를 넘으면 가장 오래된 핑을 먼저 중지한다.
void Client::CClickMoveEffect::Play_Ping(const float3_t& worldPosition,
	const std::uint32_t senderId, const std::uint32_t sequence)
{
	if (CGameInstance::Get().Get_CurrentLevelID() != m_iLevelIndex) return;
	const float3_t position{ worldPosition.x, worldPosition.y + 0.035f, worldPosition.z };
	ROOM_PING_MARKER staged;
	const auto placement = "player.ping." + std::to_string(senderId) + "." + std::to_string(sequence);
	if (!Spawn_Marker(PING_EFFECT_ID, placement.c_str(), position, staged.Handle)) return;
	if (m_RoomPings.size() >= 64u)
	{
		CEffectPresentationService::Stop_WorldRoot(m_RoomPings.front().Handle);
		m_RoomPings.erase(m_RoomPings.begin());
	}
	m_RoomPings.push_back(std::move(staged));
}

// 클릭 이펙트를 중지하고 플레이어의 비소유 연결을 비운다.
void Client::CClickMoveEffect::Clear_Move()
{
	CEffectPresentationService::Stop_WorldRoot(m_ClickHandle);
	m_ClickHandle = {};
	m_pCharacter.reset();
}

// 소멸·Level 변경 시 클릭과 모든 방 핑을 중지한다.
void Client::CClickMoveEffect::Clear()
{
	Clear_Move();
	for (auto& ping : m_RoomPings) CEffectPresentationService::Stop_WorldRoot(ping.Handle);
	m_RoomPings.clear();
}

// Layer 갱신에서 클릭은 1.2초, 핑은 3초 뒤 중지한다. delta가 유효하지 않으면 시간은 진행하지 않는다.
void Client::CClickMoveEffect::Late_Update(const f32_t fTimeDelta)
{
	if (CGameInstance::Get().Get_CurrentLevelID() != m_iLevelIndex)
	{
		Clear();
		return;
	}
	const auto& player = CCombatHUDViewModel::Get().Get_Player();
	if (m_pCharacter.expired() || !player.isValid || !player.iCurrentHp || player.isPatternBound)
		Clear_Move();
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f) return;
	for (auto& ping : m_RoomPings)
	{
		ping.fSeconds += fTimeDelta;
		if (ping.fSeconds >= PING_DURATION_SECONDS)
		{
			CEffectPresentationService::Stop_WorldRoot(ping.Handle);
			ping.Handle = {};
		}
	}
	std::erase_if(m_RoomPings, [](const auto& ping) { return !ping.Handle.Is_Valid(); });
	m_fClickSeconds += fTimeDelta;
	if (m_ClickHandle.Is_Valid() && m_fClickSeconds >= CLICK_DURATION_SECONDS) Clear_Move();
}

// 이펙트 서비스의 실제 렌더 객체가 그리므로 여기서는 성공만 반환한다.
HRESULT Client::CClickMoveEffect::Render() { return S_OK; }

// 원형 초기화 실패는 임시 객체를 정리하고 nullptr로 반환한다.
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

// 기반 객체와 장치 참조를 복사하고 인스턴스를 초기화한다. 실패는 nullptr다.
shared_ptr<CPrototype> Client::CClickMoveEffect::Clone(void* pArg)
{
	auto instance = shared_ptr<CClickMoveEffect>(
		new CClickMoveEffect(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
