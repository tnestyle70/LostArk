// 룸이 소유하는 안내 actor·owner 상태·대사 큐와 카탈로그를 갱신한다.
#include "GameRoom.h"
// 실제 owner session을 차용하고 대사/상태 송신 실패 시 연결 종료를 요청한다.
#include "ClientSession.h"
// 위험 후보 위치와 게시 combat-object hit shape의 겹침을 계산한다.
#include "ServerCombatGeometry.h"
// 안내 상태·대사 메시지를 기존 프레임 송신 형식으로 직렬화한다.
#include "Network/PacketWriter.h"
// 현재 본문에서 WorldCollision 상수를 직접 참조하지는 않는다. 기존 include 구성을 보존한다.
#include "Gameplay/WorldCollisionContract.h"
// 인간 수·ID/별칭 조회·대사 우선순위 정렬·점수/입력 clamp를 사용한다.
#include <algorithm>
// world XZ 거리와 yaw 기반 추적점·주변 후보를 계산한다.
#include <cmath>
// 가이드 초기화/설치 skill 검증 실패를 Server 콘솔에 남긴다.
#include <iostream>
// HELP_COMMAND의 combo 우선순위 선택을 int 최솟값에서 시작한다.
#include <limits>

using namespace LostArk::Server;
using namespace LostArk::Shared;
namespace
{
	constexpr float PI = 3.14159265359f;

	// 안내 추적·시작 범위에 쓰는 world XZ 수평 거리(m)다. 층 높이 검사는 필요한 호출부에서 별도로 한다.
	float distance(const SERVER_PLAYER& a, const SERVER_PLAYER& b)
	{
		return std::hypot(a.fPositionX - b.fPositionX, a.fPositionZ - b.fPositionZ);
	}

	// 게시 cooldown ms를 초당 30 tick 기준으로 올림한다. catalog의 제한된 ms 범위를 전제로 곱셈을 수행한다.
	std::uint32_t ticks(std::uint32_t ms)
	{
		return (ms * 30u + 999u) / 1000u;
	}

	// 인간 전용 Mario/댄스/미로 상태에서는 가이드 이동을 따라붙이지 않도록 현재 player 상태를 판정한다.
	bool isMinigameAnchor(const SERVER_PLAYER& player)
	{
		return player.iMarioStage != 0 || player.eCardMazeRole != CARD_MAZE_ROLE::NONE ||
		       player.eKoukuHudMode == KOUKU_HUD_MODE::MARIO || player.eKoukuHudMode == KOUKU_HUD_MODE::DANCE ||
		       player.eKoukuHudMode == KOUKU_HUD_MODE::MAZE;
	}

	// 도움 명령의 앞뒤 ASCII 공백 네 종류만 제거한 복사본을 만든다. 대소문자 변경이나 부분 일치 검색은 하지 않는다.
	std::string trim(const std::string& value)
	{
		auto a = value.find_first_not_of(" \t\r\n");
		auto b = value.find_last_not_of(" \t\r\n");
		return a == std::string::npos ? std::string{} : value.substr(a, b - a + 1);
	}
}

// 레이드 인원·빈 방 정리·Esther 갱신이 실제 인간 제어 player 수만 조회한다. GUIDE_AI/WATERPANG_AI와 HP 상태는 별도다.
std::size_t CGameRoom::Count_HumanPlayers() const
{
	return std::count_if(m_Players.begin(), m_Players.end(), [](const auto& p) { return p.second.Is_Human(); });
}

// 초기 생성·부활·회피·로컬 도착이 후보 위치를 찾는다. guide/anchor는 호출 동안 차용하며 성공할 때만 out point를 덮어쓴다.
// anchor가 있으면 navigation이 필수이며 owner와의 연결을 검사한다. anchor가 없는 경로는 nav 미로드 시에도 충돌/인물 겹침 검사를 수행한다.
bool CGameRoom::Find_GuideLanding(const SERVER_PLAYER& guide, float x, float y, float z, SERVER_NAV_POINT& point,
                                  const SERVER_PLAYER* anchor) const
{
	if (anchor && !m_ServerNavigation.Is_Loaded())
		return false;
	// 중심과 반경 0.75/1.5/2.25/3m의 고정 12방향, 총 49개 후보를 같은 순서로 검사한다. 첫 통과 지점을 사용한다.
	for (unsigned sample = 0; sample < 49; ++sample)
	{
		const float radius = sample ? (.75f + static_cast<float>((sample - 1) / 12) * .75f) : 0.f;
		const float angle = static_cast<float>(sample % 12) * PI / 6.f;
		SERVER_NAV_POINT candidate{x + std::sin(angle) * radius, y, z + std::cos(angle) * radius};
		if (m_ServerNavigation.Is_Loaded())
		{
			// 높이 샘플만으로 동적 navigation blocker를 통과시키지 않도록 원위치 보행 가능 여부를 먼저 확인한다. 투영 높이는 요청 y에서 2m 이내여야 한다.
			if (!m_ServerNavigation.Is_PointWalkableExact(candidate.x, candidate.z, y) ||
			    !m_ServerNavigation.Sample_Position(candidate.x, candidate.z, candidate, y) ||
			    std::abs(candidate.y - y) > 2.f)
				continue;
			// owner가 있으면 그 높이 기준의 navigation 직선 연결을 확인한다. 단순히 XZ상 가까운 다른 섬/겹친 갑판을 추적 도착점으로 고르지 않는다.
			if (anchor && !m_ServerNavigation.Has_LineOfSight(anchor->fPositionX, anchor->fPositionZ, candidate.x,
			                                                  candidate.z, anchor->fPositionY))
				continue;
		}
		if (!m_ServerCollisionSystem.Is_PlayerPositionClear(candidate.x, candidate.y, candidate.z, guide.iNetEntityId))
			continue;

		// 정적/동적 충돌 통과 뒤 가까운 살아 있는 player와의 겹침을 검사한다. 현재 guide ID와 안내 actor ID는 이 인물 겹침 검사에서 제외한다.
		bool overlap = false;
		for (const auto& [id, other] : m_Players)
			if (id != guide.iPlayerId && id != m_iGuideReceptionId && other.iCurrentHp &&
			    std::abs(other.fPositionY - candidate.y) < 1.5f &&
			    std::hypot(other.fPositionX - candidate.x, other.fPositionZ - candidate.z) < .75f)
			{
				overlap = true;
				break;
			}
		if (!overlap)
		{
			point = candidate;
			return true;
		}
	}
	return false;
}

// 초기 안내 actor와 부활 후보를 지역 SERVER_PLAYER로 구성한다. 별도 transport session이나 party slot을 만들지 않는다.
// profile/ID/룸 수용량/착지 검사를 통과한 뒤 out에 이동 대입하므로 명시적 false는 기존 out을 유지한다.
bool CGameRoom::Build_GuidePlayer(PLAYER_ID playerId, NET_ENTITY_ID entityId, float x, float y, float z,
                                  SERVER_PLAYER& out) const
{
	const auto* profile = m_GameplayCatalog.Find_Player(CHARACTER_CLASS_ID::DIMENSIONMASTER);
	if (!m_GuideCatalog.Loaded || !profile || !playerId || !entityId ||
	    (!m_Players.contains(playerId) && m_Players.size() >= MAX_WORLD_SNAPSHOT_PLAYERS))
		return false;
	SERVER_PLAYER guide;
	guide.eControlKind = PLAYER_CONTROL_KIND::GUIDE_AI;
	guide.iPlayerId = playerId;
	guide.iNetEntityId = entityId;
	guide.eCharacterClass = CHARACTER_CLASS_ID::DIMENSIONMASTER;
	guide.strNickName = m_GuideCatalog.Name;
	guide.strSpawnPlacementId = m_GuideCatalog.PlacementId;
	guide.eStance = profile->eDefaultStance;
	guide.iCurrentHp = guide.iMaximumHp = profile->iMaximumHp;
	guide.iCurrentResource = guide.iMaximumResource = profile->iMaximumResource;
	guide.iMaximumIdentity = profile->iMaximumIdentity;
	guide.fMoveSpeed = profile->fMoveSpeed;
	guide.fYawDegrees = m_GuideCatalog.Yaw;
	CPlayerSkillSystem::Reset_Gauges(guide, m_GameplayCatalog);
	guide.isCombatReady = true;
	SERVER_NAV_POINT position;
	if (!Find_GuideLanding(guide, x, y, z, position))
		return false;
	guide.fPositionX = position.x;
	guide.fPositionY = position.y;
	guide.fPositionZ = position.z;
	out = std::move(guide);
	return true;
}

// 룸 Initialize의 마지막 단계가 Guide 문서와 실제 설치된 차원술사 skill binding을 대조한다. 실패를 로그로 남기며 룸 초기화 반환값을 바꾸지 않는다. skill binding 불일치에서는 Loaded=false로 둔다.
// 지원 world에서 catalog를 읽되 안내 actor 삽입은 Bern에서만 한 번 수행하는 초기화 경로다.
void CGameRoom::Initialize_Guide()
{
	if (m_eWorldId != WORLD_ID::BERN && m_eWorldId != WORLD_ID::VALTAN_ARENA &&
	    m_eWorldId != WORLD_ID::KOUKUSAYDON_ARENA && m_eWorldId != WORLD_ID::COLOSSEUM)
		return;
	std::string status;
	if (!m_GuideCatalog.Load(status))
	{
		std::cout << "[Guide] " << status << '\n';
		return;
	}
	// publisher가 기록한 skill ID라도 현재 설치 세대의 class와 input slot이 같은지 다시 검사한다. 불일치하면 이후 안내 갱신을 막도록 Loaded=false로 둔다.
	for (const auto& combo : m_GuideCatalog.Combos)
	{
		if (combo.Skills.size() != combo.Slots.size())
		{
			m_GuideCatalog.Loaded = false;
			std::cout << "[Guide] unresolved combo " << combo.Id << '\n';
			return;
		}
		for (std::size_t i = 0; i < combo.Skills.size(); ++i)
		{
			const auto* skill = m_GameplayCatalog.Find_Skill(combo.Skills[i]);
			if (!skill || skill->eCharacterClass != CHARACTER_CLASS_ID::DIMENSIONMASTER ||
			    skill->strInputSlot != combo.Slots[i])
			{
				m_GuideCatalog.Loaded = false;
				std::cout << "[Guide] published skill binding mismatch " << combo.Id << '\n';
				return;
			}
		}
	}
	if (m_eWorldId != WORLD_ID::BERN)
		return;
	SERVER_PLAYER reception;
	if (!Build_GuidePlayer(m_iNextGuidePlayerId, m_iNextNetEntityId, m_GuideCatalog.Position[0],
	                       m_GuideCatalog.Position[1], m_GuideCatalog.Position[2], reception))
	{
		std::cout << "[Guide] reception position is unavailable\n";
		return;
	}

	// 생성된 actor를 룸 player/entity lookup에 등록한다. 시작 요청 전에는 전투 준비를 끄며 기존 player 송신 경로가 이 개체를 복제한다.
	reception.isCombatReady = false;
	m_iGuideReceptionId = reception.iPlayerId;
	m_PlayerIdByEntityId.emplace(reception.iNetEntityId, reception.iPlayerId);
	m_Players.emplace(reception.iPlayerId, std::move(reception));
	++m_iNextGuidePlayerId;
	++m_iNextNetEntityId;
}

// typed GUIDE_CONTROL의 START가 이미 배치된 guide를 최대 한 owner에게 연결한다. false는 대상이 guide가 아님을 뜻한다.
// true에는 조건 거절·기존 owner 유지도 포함되므로 새 안내 시작 성공 여부로 해석하지 않는다. owner의 실제 session을 weak_ptr로 저장한다.
bool CGameRoom::Start_Guide(const SERVER_PLAYER& inviter, const NET_ENTITY_ID target)
{
	const auto targetId = m_PlayerIdByEntityId.find(target);
	if (targetId == m_PlayerIdByEntityId.end())
		return false;
	const auto actor = m_Players.find(targetId->second);
	if (actor == m_Players.end() || !actor->second.Is_Guide())
		return false;
	if (!inviter.Is_Human() || !inviter.iCurrentHp || m_eWorldId != WORLD_ID::BERN || !m_GuideCatalog.Loaded)
		return true;
	const auto ownerSessionId = inviter.iSessionId;
	auto ownerSession = Find_Session(ownerSessionId);
	if (!ownerSession || ownerSession->Is_Closing() || !ownerSessionId)
		return true;
	if (!m_PersonalGuides.empty())
	{
		Broadcast_GuideOwnership();
		return true;
	}
	if (targetId->second != m_iGuideReceptionId || distance(inviter, actor->second) > 10.f)
		return true;
	auto& guide = actor->second;
	// 지상 owner와 다시 시작할 때 비행 중인 guide는 기존 착지 종료 처리를 먼저 요청한다. FALLING이면 아직 owner를 연결하지 않고 돌아간다.
	if (guide.eVehicleFlightPhase != VEHICLE_FLIGHT_PHASE::GROUNDED && inviter.iVehicleId != ANCIENT_SEA_VEHICLE_ID)
	{
		End_VehicleSkill(guide);
		if (guide.eAction == PLAYER_ACTION_STATE::FALLING)
			return true;
	}
	Reset_PlayerForDebugTeleport(guide);
	guide.isCombatReady = true;
	GUIDE_RUNTIME runtime;
	runtime.PlayerId = guide.iPlayerId;
	runtime.AnchorId = inviter.iPlayerId;
	runtime.OwnerSession = ownerSession;
	runtime.OwnerNetEntityId = inviter.iNetEntityId;
	runtime.Sequence =
	    (std::max)({guide.iLastMoveSequence, guide.iLastSkillSequence, guide.LastVehicleRidingResult.iRequestSequence});
	// 같은 안내 actor에 제어 상태만 연결한다. 새로운 player나 인간 party를 생성하지 않으며 시작 위치에 이미 겹친 공간 trigger도 기준 상태로 기록한다.
	m_PersonalGuides.emplace(ownerSessionId, std::move(runtime));
	Seed_GuideSpaceEntries(ownerSessionId, inviter);
	Broadcast_GuideOwnership();
	const auto category = m_GuideCatalog.Categories.find(WORLD_ID::BERN);
	for (const auto& trigger : m_GuideCatalog.Triggers)
		if (trigger.Enabled && category != m_GuideCatalog.Categories.end() && trigger.Category == category->second &&
		    (trigger.Type == "GUIDE_STARTED" || trigger.Type == "PARTY_JOINED"))
			Queue_GuidePrompt(ownerSessionId, trigger);
	return true;
}

// 룸 command drain이 Bern의 시작/중지 요청을 처리한다. session별 wrap 비교로 오래된 sequence를 거부하고 owner/actor identity를 확인한다.
// 새 sequence는 identity 검사보다 먼저 기록한다. STOP은 이 session이 현재 안내를 소유할 때만 상태를 제거한다.
void CGameRoom::Handle_GuideControl(SESSION_ID sessionId, const C2S_GUIDE_CONTROL& request)
{
	if (m_eWorldId != WORLD_ID::BERN || request.iRequestSequence == 0)
		return;
	auto& last = m_GuideControlSequences[sessionId];
	if (last && static_cast<std::int32_t>(request.iRequestSequence - last) <= 0)
		return;
	last = request.iRequestSequence;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	if (binding == m_PlayerIdBySessionId.end())
		return;
	const auto owner = m_Players.find(binding->second), actor = m_Players.find(m_iGuideReceptionId);
	if (owner == m_Players.end() || actor == m_Players.end() || !owner->second.Is_Human() ||
	    request.iGuideNetEntityId != actor->second.iNetEntityId)
		return;
	if (request.eAction == GUIDE_CONTROL_ACTION::START)
		(void)Start_Guide(owner->second, request.iGuideNetEntityId);
	else if (request.eAction == GUIDE_CONTROL_ACTION::STOP && m_PersonalGuides.contains(sessionId))
		Remove_Guide(sessionId);
}

// owner 표시와 판단 trace를 방 안 인간 session들에게 보낸다. 직렬화 실패는 송신 없이 반환하고 enqueue 실패는 해당 연결의 종료를 요청한다.
// 이미 바뀐 안내 상태를 되돌리거나 다른 수신자의 송신까지 하나의 transaction으로 묶지는 않는다.
void CGameRoom::Broadcast_GuideState(const S2C_GUIDE_STATE& message)
{
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	for (const auto& [id, human] : m_Players)
		if (human.Is_Human())
			if (auto session = Find_Session(human.iSessionId);
			    session && !session->Is_Closing() &&
			    !session->Send_Frame(PACKET_TYPE::S2C_GUIDE_STATE, writer.Get_Buffer()))
				session->Request_Close();
}

// 시작/해제 및 owner 부재 대기 tick이 단일 안내 actor의 소유 상태를 알린다. 예약이 있으면 저장된 owner entity ID를 사용한다.
void CGameRoom::Broadcast_GuideOwnership()
{
	const auto actor = m_Players.find(m_iGuideReceptionId);
	if (actor == m_Players.end())
		return;
	S2C_GUIDE_STATE state;
	state.iGuideNetEntityId = actor->second.iNetEntityId;
	state.iRevision = m_GuideCatalog.Revision;
	state.iServerTick = m_iServerTick ? m_iServerTick : 1;
	if (!m_PersonalGuides.empty())
	{
		const auto& runtime = m_PersonalGuides.begin()->second;
		state.iOwnerNetEntityId = runtime.OwnerNetEntityId;
		state.iAction = runtime.WaitingForOwner || runtime.WaitingForShip ? 8 : 1;
		state.strReason = "Guidance is reserved for its current owner";
	}
	else
		state.strReason = "Ready to start guidance";
	Broadcast_GuideState(state);
}

// STOP·disconnect·actor 소실이 owner 제어/대사 예약을 해제한다. actor 자체는 룸에 남기며 공통 reset으로 이동·스킬을 정리하고 비행 입력/속도를 0으로 둔다.
// publish=false는 호출부의 다른 transaction 중 즉시 outbound 접근을 피하기 위한 선택이다.
void CGameRoom::Remove_Guide(SESSION_ID ownerSessionId, bool publish)
{
	const auto found = m_PersonalGuides.find(ownerSessionId);
	if (found == m_PersonalGuides.end())
		return;
	auto actor = m_Players.find(found->second.PlayerId);
	if (actor != m_Players.end())
	{
		Reset_PlayerForDebugTeleport(actor->second);
		actor->second.isCombatReady = false;
		actor->second.fVehicleFlightInputX = actor->second.fVehicleFlightInputY = actor->second.fVehicleFlightInputZ =
		    0.f;
		actor->second.fVehicleFlightVelocityX = actor->second.fVehicleFlightVelocityY =
		    actor->second.fVehicleFlightVelocityZ = 0.f;
	}
	// owner runtime을 지우면 그 안의 pending combo·대사·쿨다운 기록도 함께 소멸한다. 배치된 안내 actor는 제거하지 않는다.
	m_PersonalGuides.erase(found);
	if (publish)
		Broadcast_GuideOwnership();
}

// Leave의 LEVEL_CHANGED가 Bern guide를 원래 방에 대기시킨다. owner weak_ptr와 actor identity는 유지하되 실행/대사 예약을 비운다.
// 이미 준비된 입장 송신 transaction 안에서 호출될 수 있으므로 여기서는 안내 packet을 직접 보내지 않는다.
void CGameRoom::Suspend_PersonalGuide(SESSION_ID ownerSessionId)
{
	if (m_eWorldId != WORLD_ID::BERN)
		return;
	const auto found = m_PersonalGuides.find(ownerSessionId);
	if (found == m_PersonalGuides.end())
		return;
	auto& state = found->second;
	auto actor = m_Players.find(state.PlayerId);
	if (actor != m_Players.end())
	{
		Reset_PlayerForDebugTeleport(actor->second);
		actor->second.isCombatReady = false;
		actor->second.fVehicleFlightInputX = actor->second.fVehicleFlightInputY = actor->second.fVehicleFlightInputZ =
		    0.f;
		actor->second.fVehicleFlightVelocityX = actor->second.fVehicleFlightVelocityY =
		    actor->second.fVehicleFlightVelocityZ = 0.f;
	}
	// 월드 이동 commit 안에서도 호출하므로 outbound queue에 packet이나 callback을 추가하지 않고 대기 상태만 기록한다.
	state.WaitingForOwner = true;
	state.ReturningOnFoot = false;
	state.ComboId.clear();
	state.PendingComboId.clear();
	state.Reason.clear();
	state.PromptQueue.clear();
	state.PromptRemaining = 0.f;
	state.InsideBoxes.clear();
	state.FarElapsed = 0.f;
}

// 단독/파티 입장 commit 이후 Bern 복귀를 알린다. 기존 weak owner와 현재 session/binding이 같은 경우만 다시 연결한다.
// 새 player binding이 m_Players에도 존재한다는 성공한 입장 계약을 따른다. 대사는 예약만 하며 실제 송신은 다음 Update_Guides가 수행한다.
void CGameRoom::Resume_PersonalGuide(SESSION_ID ownerSessionId, WORLD_ID sourceWorld)
{
	if (m_eWorldId != WORLD_ID::BERN)
		return;
	const auto found = m_PersonalGuides.find(ownerSessionId);
	if (found == m_PersonalGuides.end() || !found->second.WaitingForOwner)
		return;
	auto& state = found->second;
	const auto owner = state.OwnerSession.lock();
	const auto binding = m_PlayerIdBySessionId.find(ownerSessionId);
	if (!owner || owner->Is_Closing() || owner != Find_Session(ownerSessionId) ||
	    binding == m_PlayerIdBySessionId.end())
		return;
	const auto actor = m_Players.find(state.PlayerId);
	if (actor == m_Players.end())
		return;
	state.AnchorId = binding->second;
	state.OwnerNetEntityId = m_Players.at(binding->second).iNetEntityId;
	state.WaitingForOwner = false;
	state.ReturningOnFoot = true;
	state.FarElapsed = 0.f;
	actor->second.isCombatReady = actor->second.iCurrentHp != 0;
	state.Reason = "Owner returned; approaching on the existing navigation path";
	Seed_GuideSpaceEntries(ownerSessionId, m_Players.at(binding->second));
	const char* type = nullptr;
	const char* world = nullptr;
	switch (sourceWorld)
	{
	case WORLD_ID::VALTAN_ARENA:
		type = "RAID_RETURNED";
		world = "VALTAN_ARENA";
		break;
	case WORLD_ID::KOUKUSAYDON_ARENA:
		type = "RAID_RETURNED";
		world = "KOUKUSAYDON_ARENA";
		break;
	case WORLD_ID::MAHARAKA:
		type = "WORLD_RETURNED";
		world = "MAHARAKA";
		break;
	case WORLD_ID::COLOSSEUM:
		type = "WORLD_RETURNED";
		world = "COLOSSEUM";
		break;
	default:
		break;
	}
	// 성공한 입장 commit 이후의 source world에 맞는 복귀 대사를 예약한다. 알려지지 않은 source도 추적은 재개하지만 복귀 trigger는 고르지 않는다.
	const auto category = m_GuideCatalog.Categories.find(WORLD_ID::BERN);
	if (type)
		for (const auto& trigger : m_GuideCatalog.Triggers)
			if (trigger.Enabled && category != m_GuideCatalog.Categories.end() &&
			    trigger.Category == category->second && trigger.Type == type && trigger.PatternId == world)
				Queue_GuidePrompt(ownerSessionId, trigger);
}

// 시작/월드 복귀가 owner의 현재 공간 접촉을 기준값으로 기록한다. 새 진입 대사를 만들거나 trigger cooldown을 소비하지 않는다.
void CGameRoom::Seed_GuideSpaceEntries(SESSION_ID ownerSessionId, const SERVER_PLAYER& anchor)
{
	const auto guide = m_PersonalGuides.find(ownerSessionId);
	if (guide == m_PersonalGuides.end())
		return;
	auto& inside = guide->second.InsideBoxes;
	inside.clear();
	const auto category = m_GuideCatalog.Categories.find(m_eWorldId);
	if (category == m_GuideCatalog.Categories.end())
		return;
	// 시작/복귀 시 이미 상자 안에 있다면 바깥→안 진입으로 세지 않는다. 이후 실제로 나갔다가 들어온 edge만 별도 갱신이 발견한다.
	for (const auto& trigger : m_GuideCatalog.Triggers)
		if (trigger.Enabled && trigger.Category == category->second && trigger.Type == "SPACE_ENTER" &&
		    CServerTriggerSystem::Contains_Placement(trigger.Box, anchor))
			inside.insert(trigger.Id);
}

// 대사 예약과 매 tick이 아직 시작하지 않은 공간 대사의 실제 발생 source를 정리한다. 첫 segment가 이미 나간 대사는 접촉 이탈로 취소하지 않는다.
// 현재 catalog/owner 상태를 차용해 queue만 바꾸며 새 source를 추측해 추가하지 않는다.
void CGameRoom::Prune_GuideSpacePrompts(SESSION_ID ownerSessionId)
{
	const auto guide = m_PersonalGuides.find(ownerSessionId);
	if (guide == m_PersonalGuides.end())
		return;
	const auto anchor = m_Players.find(guide->second.AnchorId);
	const auto category = m_GuideCatalog.Categories.find(m_eWorldId);
	// 기록된 source 중 여전히 활성·같은 category/prompt·현재 접촉인 것만 남긴다. 그중 가장 높은 priority를 예약의 대표값으로 다시 고른다.
	auto& queue = guide->second.PromptQueue;
	for (auto occurrence = queue.begin(); occurrence != queue.end();)
	{
		auto& queued = *occurrence;
		if (!queued.IsSpaceEnter || queued.NextSegment > 0)
		{
			++occurrence;
			continue;
		}
		const GUIDE_TRIGGER* best = nullptr;
		std::erase_if(
		    queued.SpaceTriggerIds,
		    [&](const auto& id)
		    {
			    const auto source = std::find_if(m_GuideCatalog.Triggers.begin(), m_GuideCatalog.Triggers.end(),
			                                     [&](const auto& row) { return row.Id == id; });
			    if (anchor == m_Players.end() || source == m_GuideCatalog.Triggers.end() || !source->Enabled ||
			        source->Type != "SPACE_ENTER" || source->PromptId != queued.PromptId ||
			        category == m_GuideCatalog.Categories.end() || source->Category != category->second ||
			        !CServerTriggerSystem::Contains_Placement(source->Box, anchor->second))
				    return true;
			    if (!best || source->Priority > best->Priority)
				    best = &*source;
			    return false;
		    });
		if (!best)
		{
			occurrence = queue.erase(occurrence);
			continue;
		}
		queued.TriggerId = best->Id;
		queued.Priority = best->Priority;
		++occurrence;
	}

	// 재생을 시작한 맨 앞 예약은 고정하고 나머지만 높은 priority 순으로 안정 정렬한다. 같은 priority의 기존 순서는 보존한다.
	auto first = queue.begin();
	if (first != queue.end() && first->NextSegment > 0)
		++first;
	std::stable_sort(first, queue.end(), [](const auto& a, const auto& b) { return a.Priority > b.Priority; });
}

// 시작/복귀/도움/공간/보스 사건이 prompt ID를 owner 큐에 예약한다. 같은 prompt는 시작 여부·source 종류·priority에 따라 합치거나 유지/교체한다.
// 큐 한도 16에 도달하면 새 예약을 버리며 여기서는 cooldown을 소비하거나 packet을 보내지 않는다.
void CGameRoom::Queue_GuidePrompt(SESSION_ID ownerSessionId, const GUIDE_TRIGGER& trigger)
{
	auto guide = m_PersonalGuides.find(ownerSessionId);
	const auto* prompt = m_GuideCatalog.Find_Prompt(trigger.PromptId);
	if (guide == m_PersonalGuides.end() || !prompt)
		return;
	// 같은 prompt의 새 공간 발생이 이미 이탈한 source를 이어받지 않도록 먼저 미시작 예약을 정리한다.
	Prune_GuideSpacePrompts(ownerSessionId);
	auto& queue = guide->second.PromptQueue;
	const auto duplicate = std::find_if(queue.begin(), queue.end(),
	                                    [&](const auto& queued) { return queued.PromptId == trigger.PromptId; });
	if (duplicate != queue.end())
	{
		if (duplicate->NextSegment > 0)
			return;
		if (duplicate->IsSpaceEnter && trigger.Type == "SPACE_ENTER")
		{
			if (std::find(duplicate->SpaceTriggerIds.begin(), duplicate->SpaceTriggerIds.end(), trigger.Id) ==
			    duplicate->SpaceTriggerIds.end())
				duplicate->SpaceTriggerIds.push_back(trigger.Id);
			Prune_GuideSpacePrompts(ownerSessionId);
			return;
		}
		if (duplicate->Priority >= trigger.Priority)
			return;
		queue.erase(duplicate);
	}
	if (queue.size() >= 16)
		return;
	auto place = queue.begin();
	if (place != queue.end() && place->NextSegment > 0)
		++place;
	// 현재 실제로 발생한 trigger의 priority로 삽입한다. 시작한 대사는 끊지 않고 동순위 기존 예약 뒤에 둔다.
	while (place != queue.end() && place->Priority >= trigger.Priority)
		++place;
	queue.insert(place,
	             {trigger.PromptId, trigger.Id, 0, trigger.Priority, trigger.Type == "SPACE_ENTER",
	              trigger.Type == "SPACE_ENTER" ? std::vector<std::string>{trigger.Id} : std::vector<std::string>{}});
}

// Handle_Chat이 인간 owner의 원문을 전달한다. 앞뒤 공백을 제거한 정확 별칭만 허용하고 월드/선박 대기 중에는 처리하지 않는다.
// 명령은 안내 actor의 combo 예약을 바꾸며 채팅 자체의 방 송신은 호출자가 계속 수행한다.
void CGameRoom::Guide_ChatCommand(const SERVER_PLAYER& sender, const std::string& line)
{
	if (!sender.Is_Human())
		return;
	auto guide = m_PersonalGuides.find(sender.iSessionId);
	if (guide == m_PersonalGuides.end() || guide->second.WaitingForOwner || guide->second.WaitingForShip)
		return;
	const auto input = trim(line);
	auto& state = guide->second;
	for (const auto& command : m_GuideCatalog.Commands)
	{
		if (!command.Enabled ||
		    std::find(command.Aliases.begin(), command.Aliases.end(), input) == command.Aliases.end())
			continue;

		// Stop은 일반 명령 cooldown을 우회하되 처리 tick은 기록한다. HELP trigger의 가장 높은 priority combo가 명령 기본 combo를 덮어쓸 수 있다.
		const auto commandTick = m_iServerTick ? m_iServerTick : 1;
		const auto lastCommand = state.CommandTicks.find(command.Id);
		if (!command.Stop && lastCommand != state.CommandTicks.end() &&
		    commandTick - lastCommand->second < ticks(command.CooldownMs))
			return;
		state.CommandTicks[command.Id] = commandTick;
		std::string comboId = command.ComboId;
		int comboPriority = (std::numeric_limits<int>::min)();
		const auto category = m_GuideCatalog.Categories.find(m_eWorldId);
		for (const auto& trigger : m_GuideCatalog.Triggers)
			if (trigger.Enabled && trigger.Type == "HELP_COMMAND" && trigger.PatternId == command.Id &&
			    category != m_GuideCatalog.Categories.end() && trigger.Category == category->second &&
			    !trigger.ComboId.empty() && trigger.Priority > comboPriority)
			{
				comboId = trigger.ComboId;
				comboPriority = trigger.Priority;
			}

		// Stop은 실행/대기 combo ID를 비우지만 이미 진행 중인 player skill 자체를 강제로 끝내지는 않는다. 새 combo는 skill 종료를 기다리거나 즉시 커서를 초기화한다.
		if (command.Stop)
		{
			state.ComboId.clear();
			state.PendingComboId.clear();
			state.ComboStep = 0;
			state.Reason = "Assistance stopped by owner command";
		}
		else if (state.ComboId != comboId)
		{
			auto actor = m_Players.find(state.PlayerId);
			if (actor != m_Players.end() && actor->second.eAction == PLAYER_ACTION_STATE::SKILL)
				state.PendingComboId = comboId;
			else
			{
				state.ComboId = comboId;
				state.ComboStep = 0;
				state.ComboElapsed = state.StepElapsed = 0;
			}
		}
		for (const auto& trigger : m_GuideCatalog.Triggers)
			if (trigger.Enabled && trigger.Type == "HELP_COMMAND" && trigger.PatternId == command.Id &&
			    category != m_GuideCatalog.Categories.end() && trigger.Category == category->second)
				Queue_GuidePrompt(sender.iSessionId, trigger);
		return;
	}
}

// 확정된 이동 완료와 먼 거리 복구가 owner 도착을 알린다. Bern 로컬 이동 요청이고 실제 승선 상태가 아니면 연결된 착지점을 먼저 찾는다.
// 착지 거절은 actor와 실행 상태를 유지하고 Reason만 바꾼다. 그 외 경로는 현재 위치에서 도보 재접근하도록 공통 reset을 수행한다.
void CGameRoom::Guide_AnchorArrived(const SERVER_PLAYER& anchor, const bool localMapTravel)
{
	if (!anchor.Is_Human() || isMinigameAnchor(anchor))
		return;
	const auto found = m_PersonalGuides.find(anchor.iSessionId);
	if (found == m_PersonalGuides.end() || found->second.WaitingForOwner)
		return;
	auto& state = found->second;
	if (state.AnchorId != anchor.iPlayerId)
		return;
	auto actor = m_Players.find(state.PlayerId);
	if (actor == m_Players.end())
		return;
	const bool relocate = localMapTravel && m_eWorldId == WORLD_ID::BERN && !anchor.bShipDockValid;
	SERVER_NAV_POINT landing;
	if (relocate)
	{
		const float yaw = anchor.fYawDegrees * PI / 180.f;
		// owner 뒤의 추적 offset 주변을 먼저 찾고 실패하면 owner 중심 주변을 찾는다. 둘 다 owner 높이의 navigation 연결·충돌 조건을 통과해야 한다.
		if (!Find_GuideLanding(actor->second, anchor.fPositionX - std::sin(yaw) * m_GuideCatalog.DesiredDistance,
		                       anchor.fPositionY, anchor.fPositionZ - std::cos(yaw) * m_GuideCatalog.DesiredDistance,
		                       landing, &anchor) &&
		    !Find_GuideLanding(actor->second, anchor.fPositionX, anchor.fPositionY, anchor.fPositionZ, landing,
		                       &anchor))
		{
			state.Reason = "Owner arrived; no connected guide landing is available";
			return;
		}
	}
	// 로컬 순간 재배치가 필요할 때 착지를 승인받은 뒤에만 actor의 이동/스킬 상태를 정리한다.
	Reset_PlayerForDebugTeleport(actor->second);
	state.ComboId.clear();
	state.PendingComboId.clear();
	state.FarElapsed = 0.f;
	// InsideBoxes를 초기화하지 않는다. 실제 로컬 이동으로 새 상자에 들어갔다면 다음 갱신이 그 진입 edge를 볼 수 있다.
	state.ReturningOnFoot = true;
	state.Reason = "Following committed owner arrival on foot";
	// Bern SquareHole·출항 준비·저작 건물 이동은 승인한 착지로 actor 위치를 옮긴다. 실제 승선과 월드 복귀는 별도 대기/도보 재접근 정책을 따른다.
	if (relocate)
	{
		actor->second.fPositionX = landing.x;
		actor->second.fPositionY = landing.y;
		actor->second.fPositionZ = landing.z;
		actor->second.fYawDegrees = anchor.fYawDegrees;
		actor->second.isCombatReady = actor->second.iCurrentHp != 0u;
		state.ReturningOnFoot = false;
		state.WaitingForShip = false;
		state.Reason = "Following committed owner arrival on connected ground";
	}
}

// 룸 고정 tick이 안내를 진행한다. 현재 입구 조건은 Loaded인 Bern뿐이며 worker가 player·owner map·대사 큐를 직렬로 접근한다.
// 초 단위 dt는 룸 호출 계약을 따르고 여기서 유한성/양수를 재검사하지 않는다. state 송신과 gameplay executor 호출은 이미 변경한 상태의 rollback을 제공하지 않는다.
void CGameRoom::Update_Guides(float seconds)
{
	if (!m_GuideCatalog.Loaded || m_eWorldId != WORLD_ID::BERN)
		return;
	m_fGuideOwnershipElapsed += seconds;
	if (m_fGuideOwnershipElapsed >= m_GuideCatalog.ThinkSeconds &&
	    (m_PersonalGuides.empty() || m_PersonalGuides.begin()->second.WaitingForOwner))
	{
		m_fGuideOwnershipElapsed = 0.f;
		Broadcast_GuideOwnership();
	}

	// owner weak_ptr를 이번 반복 동안 잠가 차용하고 actor/binding을 다시 확인한다. 제거 전 iterator를 다음으로 옮겨 erase 뒤 참조를 재사용하지 않는다.
	for (auto it = m_PersonalGuides.begin(); it != m_PersonalGuides.end();)
	{
		const auto ownerSessionId = it->first;
		auto& state = it->second;
		const auto ownerSession = state.OwnerSession.lock();
		if (!ownerSession || ownerSession->Is_Closing())
		{
			++it;
			Remove_Guide(ownerSessionId);
			continue;
		}
		auto guideIt = m_Players.find(state.PlayerId);
		if (guideIt == m_Players.end())
		{
			++it;
			Remove_Guide(ownerSessionId);
			continue;
		}
		if (state.WaitingForOwner)
		{
			++it;
			continue;
		}
		const auto binding = m_PlayerIdBySessionId.find(ownerSessionId);
		if (binding == m_PlayerIdBySessionId.end())
		{
			++it;
			continue;
		}
		state.AnchorId = binding->second;
		auto anchorIt = m_Players.find(state.AnchorId);
		if (anchorIt == m_Players.end())
		{
			++it;
			continue;
		}
		const bool anchorSuppressed = isMinigameAnchor(anchorIt->second);
		auto& guide = guideIt->second;
		const auto& anchor = anchorIt->second;

		// 대사는 현재 owner 한 session에게만 보낸다. 직렬화/송신 실패는 false이며 enqueue 실패에서는 연결 종료도 요청한다.
		auto send = [&](PACKET_TYPE kind, const auto& message)
		{
			CPacketWriter writer;
			if (!Write_Message(writer, message))
				return false;
			if (!ownerSession->Send_Frame(kind, writer.Get_Buffer()))
			{
				ownerSession->Request_Close();
				return false;
			}
			return true;
		};
		float evaluatedThreat = 0.f;
		bool survivalOverride = false;

		// 현재 판단 상태와 owner identity를 방 전체 인간에게 알린다. 참조는 이 반복 안에서만 캡처하며 callback을 외부에 저장하지 않는다.
		auto publishTrace = [&]()
		{
			S2C_GUIDE_STATE trace;
			trace.iGuideNetEntityId = guide.iNetEntityId;
			trace.iOwnerNetEntityId = anchor.iNetEntityId;
			trace.iRevision = m_GuideCatalog.Revision;
			trace.iServerTick = m_iServerTick ? m_iServerTick : 1;
			trace.iContext = state.ComboId.empty() ? 0 : 1;
			trace.iAction = state.Action;
			trace.fFollowScore = state.FollowScore;
			trace.fEvadeScore = state.EvadeScore;
			trace.fCombatScore = state.CombatScore;
			trace.fThreat = evaluatedThreat;
			trace.fAnchorDistance = distance(guide, anchor);
			trace.fHpRatio =
			    guide.iMaximumHp ? std::clamp(static_cast<float>(guide.iCurrentHp) / guide.iMaximumHp, 0.f, 1.f) : 0.f;
			trace.bSurvivalOverride = survivalOverride;
			trace.strReason = state.Reason;
			trace.strComboId = state.ComboId;
			trace.iComboStep = static_cast<std::uint32_t>(state.ComboStep);
			Broadcast_GuideState(trace);
		};
		// 실제 선박에 탄 owner는 부두에서 기다린다. 이 분기는 대사/판단 타이머를 진행하지 않고 다음 owner로 넘어가며 용 탑승은 아래 공통 비행 경로를 사용한다.
		if (anchor.bShipDockValid)
		{
			if (!state.WaitingForShip)
			{
				Reset_PlayerForDebugTeleport(guide);
				guide.isCombatReady = false;
				state.WaitingForShip = true;
				state.ComboId.clear();
				state.PendingComboId.clear();
			}
			state.Action = 8;
			state.FarElapsed = 0.f;
			state.Reason = "Waiting at the pier while the owner sails";
			publishTrace();
			++it;
			continue;
		}
		if (state.WaitingForShip)
		{
			state.WaitingForShip = false;
			state.ReturningOnFoot = true;
			guide.isCombatReady = true;
		}
		// 다른 대사를 재생 중이어도 이탈한 미시작 공간 예약을 정리해 새로운 재진입을 다시 예약할 수 있게 한다.
		Prune_GuideSpacePrompts(ownerSessionId);
		const bool waitingToSpeak = state.ReturningOnFoot && distance(guide, anchor) > m_GuideCatalog.MaximumDistance;
		if (!waitingToSpeak)
			state.PromptRemaining -= seconds;
		// 새 segment를 보내기 직전에 owner 상태를 먼저 송신한다. 도보 복귀 중 아직 멀면 대사 남은 시간도 진행하지 않는다.
		if (!waitingToSpeak && state.PromptRemaining <= 0 && !state.PromptQueue.empty())
			publishTrace();
		if (!waitingToSpeak && state.PromptRemaining <= 0 && !state.PromptQueue.empty())
		{
			auto& queued = state.PromptQueue.front();
			const auto* prompt = m_GuideCatalog.Find_Prompt(queued.PromptId);
			if (prompt && queued.NextSegment < prompt->Segments.size())
			{
				const auto& segment = prompt->Segments[queued.NextSegment];
				S2C_GUIDE_PROMPT message;
				message.iGuideNetEntityId = guide.iNetEntityId;
				message.iEventSequence = ++m_iGuideEventSequence;
				if (!message.iEventSequence)
					message.iEventSequence = ++m_iGuideEventSequence;
				message.iRevision = m_GuideCatalog.Revision;
				message.strPromptId = prompt->Id;
				message.strText = segment.Text;
				message.iDurationMs = segment.DurationMs;
				if (send(PACKET_TYPE::S2C_GUIDE_PROMPT, message))
				{
					// 첫 segment의 송신 성공 때만 실제 발생 source들의 cooldown을 기록한다. 취소/한도 초과 예약과 송신 실패는 cooldown·커서를 진행시키지 않는다.
					if (queued.NextSegment == 0)
					{
						if (queued.IsSpaceEnter)
							for (const auto& id : queued.SpaceTriggerIds)
								state.TriggerTicks[id] = m_iServerTick;
						else if (!queued.TriggerId.empty())
							state.TriggerTicks[queued.TriggerId] = m_iServerTick;
					}
					++queued.NextSegment;
					state.PromptRemaining = segment.DurationMs / 1000.f;
				}
			}
			if (!prompt || queued.NextSegment >= prompt->Segments.size())
				state.PromptQueue.pop_front();
		}

		// 대사 송신과 별개로 판단/유지/combo 경과를 누적한다. ThinkSeconds에 도달한 tick에서만 아래 이동·스킬 판단을 수행한다.
		state.ThinkElapsed += seconds;
		state.HoldElapsed += seconds;
		if (!state.ComboId.empty())
			state.ComboElapsed += seconds;
		if (state.ThinkElapsed < m_GuideCatalog.ThinkSeconds)
		{
			++it;
			continue;
		}
		const float elapsed = state.ThinkElapsed;
		state.ThinkElapsed = 0;
		state.FollowScore = state.EvadeScore = state.CombatScore = 0;

		// 인간 전용 기믹 또는 사망 중 owner는 추적/공격을 중단한다. 이 검사보다 앞의 대사 큐 처리는 이미 실행될 수 있다.
		if (anchorSuppressed)
		{
			guide.hasMoveGoal = false;
			guide.MovePath.clear();
			state.Action = 8;
			state.ComboId.clear();
			state.PendingComboId.clear();
			state.Reason = "Waiting outside the human-only minigame";
			publishTrace();
			++it;
			continue;
		}
		if (!anchor.iCurrentHp || anchor.eAction == PLAYER_ACTION_STATE::DEAD)
		{
			state.Action = 7;
			guide.hasMoveGoal = false;
			guide.MovePath.clear();
			state.ComboId.clear();
			state.PendingComboId.clear();
			state.Reason = "Waiting for the owner to revive";
			publishTrace();
			++it;
			continue;
		}

		// 안내 actor가 죽으면 같은 ID의 지역 player를 재구성해 성공할 때만 교체한다. 현재 함수의 Bern 진입 제한 때문에 아래 레이드별 위치 분기는 일반 호출에서 실행되지 않는다.
		if (!guide.iCurrentHp)
		{
			state.Action = 4;
			SERVER_NAV_POINT center{anchor.fPositionX, anchor.fPositionY, anchor.fPositionZ};
			float yaw = 0;
			if (m_eWorldId == WORLD_ID::VALTAN_ARENA)
			{
				if (const auto* p = Find_Placement("boss.valtan.center"))
					center = {p->fPositionX, p->fPositionY, p->fPositionZ};
			}
			else if (m_eWorldId == WORLD_ID::KOUKUSAYDON_ARENA)
				(void)Resolve_KoukuRevivePosition(guide, center, yaw);
			SERVER_PLAYER revived;
			if (Build_GuidePlayer(guide.iPlayerId, guide.iNetEntityId, center.x, center.y, center.z, revived))
			{
				m_CombatObjectRuntime.Cancel_Source(guide.iNetEntityId);
				guide = std::move(revived);
				state.ComboId.clear();
				state.PendingComboId.clear();
				state.Reason = "Guide revived at an admitted arena position";
			}
			publishTrace();
			++it;
			continue;
		}

		// 공간은 owner의 바깥→안 edge, 보스는 직전 관측 sequence와 다른 pattern으로 사건을 만든다.
		// 같은 판단에서 모든 trigger를 검사한 뒤 보스 sequence를 갱신하므로 먼저 본 trigger가 다른 trigger의 관측을 지우지 않는다.
		const auto category = m_GuideCatalog.Categories.find(m_eWorldId);
		for (const auto& trigger : m_GuideCatalog.Triggers)
		{
			if (!trigger.Enabled || category == m_GuideCatalog.Categories.end() || trigger.Category != category->second)
				continue;
			bool fire = false;
			if (trigger.Type == "SPACE_ENTER")
			{
				const bool inside = CServerTriggerSystem::Contains_Placement(trigger.Box, anchor);
				const bool was = state.InsideBoxes.contains(trigger.Id);
				if (inside)
					state.InsideBoxes.insert(trigger.Id);
				else
					state.InsideBoxes.erase(trigger.Id);
				fire = inside && !was;
			}
			else if (trigger.Type == "BOSS_PATTERN_STARTED")
				for (const auto& boss : m_WorldEntities)
					if (boss.iCurrentHp && boss.strPatternId == trigger.PatternId && boss.iPatternSequence &&
					    state.PatternSequences[boss.iNetEntityId] != boss.iPatternSequence)
					{
						fire = true;
						break;
					}
			auto last = state.TriggerTicks.find(trigger.Id);
			if (fire && (last == state.TriggerTicks.end() || m_iServerTick - last->second >= ticks(trigger.CooldownMs)))
			{
				Queue_GuidePrompt(ownerSessionId, trigger);
			}
		}
		for (const auto& boss : m_WorldEntities)
			if (!boss.strPatternId.empty())
				state.PatternSequences[boss.iNetEntityId] = boss.iPatternSequence;

		// 살아 있는 boss/monster 중 Esther 소환을 제외한 가장 가까운 XZ 대상을 차용한다. 이 반복에서 world entity 저장소를 교체하지 않는 룸 실행 순서를 따른다.
		SERVER_WORLD_ENTITY* enemy = nullptr;
		float enemyDistance = 100000;
		for (auto& candidate : m_WorldEntities)
			if ((candidate.eKind == WORLD_BOOTSTRAP_KIND::BOSS || candidate.eKind == WORLD_BOOTSTRAP_KIND::MONSTER) &&
			    candidate.iCurrentHp && !candidate.isEstherSummon)
			{
				const float d =
				    std::hypot(candidate.fPositionX - guide.fPositionX, candidate.fPositionZ - guide.fPositionZ);
				if (d < enemyDistance)
				{
					enemy = &candidate;
					enemyDistance = d;
				}
			}

		// 35m 안의 행동 중 적 또는 활성 combo를 전투 상황으로 본다. 전투·도보 복귀가 아닌 과도한 이격 시간만 복구 지연에 누적한다.
		const bool battle =
		    enemy && enemyDistance < 35.f && (enemy->eAction != SERVER_ENTITY_ACTION::IDLE || !state.ComboId.empty());
		const float anchorDistance = distance(guide, anchor);
		if (state.ReturningOnFoot && anchorDistance <= m_GuideCatalog.MaximumDistance)
			state.ReturningOnFoot = false;
		state.FarElapsed = !state.ReturningOnFoot && anchorDistance > m_GuideCatalog.RecoverDistance && !battle
		                       ? state.FarElapsed + elapsed
		                       : 0.f;
		if (state.FarElapsed >= m_GuideCatalog.RecoverDelay)
		{
			state.Action = 4;
			Guide_AnchorArrived(anchor);
			publishTrace();
			++it;
			continue;
		}

		// 붙잡힘·낙하·피격 이동·저작 이동이 현재 이동을 소유하면 AI가 새 이동을 제출하지 않는다.
		if (guide.bPatternBound || guide.TriggerMove.isActive || guide.eAction == PLAYER_ACTION_STATE::GRABBED ||
		    guide.eAction == PLAYER_ACTION_STATE::FALLING || guide.fKnockbackRemainingSeconds > 0)
		{
			state.Action = 5;
			state.Reason = "Contact reaction owns the guide movement";
			publishTrace();
			++it;
			continue;
		}

		// owner의 탑승 상태를 공통 적용 함수로 요청한다. 실패 반환을 별도 상태로 처리하지 않으며 실제 guide 값은 공통 검증 결과를 따른다.
		if (anchor.iVehicleId != guide.iVehicleId && guide.eAction == PLAYER_ACTION_STATE::NONE)
		{
			C2S_SET_VEHICLE_RIDING command;
			command.eWorldId = m_eWorldId;
			command.iRequestSequence = ++state.Sequence;
			command.iVehicleId = anchor.iVehicleId;
			(void)Apply_SetVehicleRiding(guide, command);
		}

		// 용은 E 슬롯의 기존 이착륙 skill을 사용한다. 비행 중에는 owner 뒤 목표점 방향과 높이 차를 입력으로 만들어 공통 이동 executor에 제출한다.
		if (guide.iVehicleId == ANCIENT_SEA_VEHICLE_ID)
		{
			if (((anchor.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::FLYING ||
			      anchor.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::TAKEOFF) &&
			     guide.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::GROUNDED) ||
			    ((anchor.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::GROUNDED ||
			      anchor.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::LANDING) &&
			     guide.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::FLYING))
			{
				const auto* vehicle = m_VehicleCatalog.Find_Vehicle(guide.iVehicleId);
				if (vehicle)
				{
					for (const auto& skill : vehicle->Skills)
						if (skill.eSlot == VEHICLE_SKILL_SLOT::E)
						{
							C2S_USE_SKILL command;
							command.iClientSequence = ++state.Sequence;
							command.iSkillId = skill.iSkillId;
							command.fAimX = anchor.fPositionX;
							command.fAimZ = anchor.fPositionZ;
							(void)Try_StartVehicleSkill(guide, command);
							break;
						}
				}
			}
			if (guide.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::FLYING)
			{
				const float yaw = anchor.fYawDegrees * PI / 180.f;
				float dx = anchor.fPositionX - std::sin(yaw) * m_GuideCatalog.DesiredDistance - guide.fPositionX,
				      dz = anchor.fPositionZ - std::cos(yaw) * m_GuideCatalog.DesiredDistance - guide.fPositionZ;
				float length = std::hypot(dx, dz);
				C2S_MOVE move;
				move.iClientSequence = ++state.Sequence;
				move.eIntent = PLAYER_MOVE_INTENT::VEHICLE_FLIGHT;

				// 비행 입력은 0.5m 이내에서 0, 그 밖에서는 max(1m,거리)로 나눠 크기 1 이하의 XZ 방향을 만든다. 높이 차도 -1~1로 제한한다.
				move.fGoalX = length > .5f ? dx / (std::max)(1.f, length) : 0;
				move.fGoalZ = length > .5f ? dz / (std::max)(1.f, length) : 0;
				move.fVerticalInput = std::clamp(anchor.fPositionY - guide.fPositionY, -1.f, 1.f);
				Execute_PlayerMove(guide, move);
				state.Action = 6;
				state.Reason = "Following replicated dragon flight";
				publishTrace();
				++it;
				continue;
			}
		}

		// 현재 HOLD skill은 1초 이후 공통 release로 종료를 요청한다. 대기 combo는 실제 action이 NONE일 때만 새 커서로 넘긴다.
		if (guide.eAction == PLAYER_ACTION_STATE::SKILL)
			if (const auto* running = m_GameplayCatalog.Find_Skill(guide.iCurrentSkillId);
			    running && running->eSkillKind == PLAYER_SKILL_KIND::HOLD && guide.fActionElapsedSeconds >= 1.f)
			{
				C2S_RELEASE_SKILL release;
				release.iClientSequence = ++state.Sequence;
				release.iSkillId = guide.iCurrentSkillId;
				m_PlayerSkillSystem.Release(guide, release, m_GameplayCatalog);
			}
		if (!state.PendingComboId.empty() && guide.eAction == PLAYER_ACTION_STATE::NONE)
		{
			state.ComboId = std::move(state.PendingComboId);
			state.PendingComboId.clear();
			state.ComboStep = 0;
			state.ComboElapsed = state.StepElapsed = 0;
		}
		auto combo = m_GuideCatalog.Find_Combo(state.ComboId);
		if (combo && state.ComboElapsed * 1000 >= combo->TimeoutMs)
		{
			state.ComboId.clear();
			combo = nullptr;
			state.Reason = "Combo total deadline reached";
		}
		// 접촉 ledger의 위험에 현재 살아 있는 비플레이어 combat-object hit 위험을 더한다.
		// 현재/예측 중간/끝의 세 위치를 현재 방향·속도로 선형 외삽하므로 미래 simulation의 정확한 전체 재생은 아니다.
		auto danger = [&](float x, float z)
		{
			float risk = Predict_GuideContactRisk(guide, x, z);
			for (const auto& object : m_CombatObjectRuntime.Get_LiveObjects())
			{
				if (object.eSourceKind == SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER)
					continue;
				const auto& pose = object.LiveState.CurrentPose;
				if (std::abs(pose.fPositionY - guide.fPositionY) > 3.f)
					continue;

				// 종료한 hit와 예측 구간 뒤에 시작하는 hit를 제외한다. 후보 반경 0.5m와 한 sample이라도 겹치면 hit당 일반1·즉사5를 더한다.
				for (const auto& hit : object.Hits)
				{
					if (hit.iEndMs && object.fElapsedMilliseconds > hit.iEndMs)
						continue;
					if (hit.iAtMs > object.fElapsedMilliseconds + m_GuideCatalog.HorizonSeconds * 1000)
						continue;
					for (int sample = 0; sample < 3; ++sample)
					{
						const float t = m_GuideCatalog.HorizonSeconds * sample * .5f;
						float ox = pose.fPositionX + pose.fDirectionX * object.fSpeedMps * t,
						      oz = pose.fPositionZ + pose.fDirectionZ * object.fSpeedMps * t;
						if (CServerCombatGeometry::Overlaps_Pose(hit.Shape, ox, oz, pose.fDirectionX, pose.fDirectionZ,
						                                         {x, z, .5f}))
						{
							risk += (hit.bInstantDeath ? 5.f : 1.f);
							break;
						}
					}
				}
			}
			return risk;
		};

		// 회피 목적지까지 navigation 경로를 구하고 XZ 길이 최대 0.5m 간격으로 위험을 샘플한다. 총128개 초과나 현재 위험+0.001 초과 지점이 있으면 거절한다.
		// 위험 감소 경로의 근사 판정이며 이동 실행 성공 또는 모든 미래 충돌의 부재를 보증하지 않는다.
		auto pathSafe = [&](const SERVER_NAV_POINT& goal, float currentRisk)
		{
			std::vector<SERVER_NAV_POINT> path;
			if (m_ServerNavigation.Is_Loaded())
			{
				if (!m_ServerNavigation.Find_Path(guide.fPositionX, guide.fPositionZ, goal.x, goal.z, path,
				                                  guide.fPositionY))
					return false;
				m_ServerNavigation.Smooth_Path(guide.fPositionX, guide.fPositionZ, goal.x, goal.z, path,
				                               guide.fPositionY);
			}
			else
				path.push_back(goal);
			SERVER_NAV_POINT from{guide.fPositionX, guide.fPositionY, guide.fPositionZ};
			unsigned samples = 0;
			for (const auto& to : path)
			{
				const float length = std::hypot(to.x - from.x, to.z - from.z);
				const unsigned count = (std::max)(1u, static_cast<unsigned>(std::ceil(length / .5f)));
				if (samples + count > 128)
					return false;
				for (unsigned i = 1; i <= count; ++i)
				{
					float t = float(i) / count;
					if (danger(from.x + (to.x - from.x) * t, from.z + (to.z - from.z) * t) > currentRisk + .001f)
						return false;
				}
				samples += count;
				from = to;
			}
			return true;
		};

		// 정규화된 가중치에 추적 거리 오차·위험·combo/적 존재를 곱해 점수를 만든다. 위험은 피해량이 아니라 접촉 위험 점수다.
		const float risk = danger(guide.fPositionX, guide.fPositionZ);
		const auto& weight = combo ? m_GuideCatalog.AssistWeights : m_GuideCatalog.FollowWeights;
		const bool needsFollow = (state.ReturningOnFoot && anchorDistance > m_GuideCatalog.MaximumDistance) ||
		                         anchorDistance > m_GuideCatalog.ResumeDistance ||
		                         (guide.hasMoveGoal && anchorDistance > m_GuideCatalog.MaximumDistance) ||
		                         anchorDistance < m_GuideCatalog.MinimumDistance;
		const float distanceError = anchorDistance < m_GuideCatalog.MinimumDistance
		                                ? m_GuideCatalog.MinimumDistance - anchorDistance
		                                : anchorDistance - m_GuideCatalog.MaximumDistance;
		const float followScore = (needsFollow ? weight.Follow : 0.f) *
		                          std::clamp(distanceError / (std::max)(1.f, m_GuideCatalog.ResumeDistance), 0.f, 1.f);
		const float evadeScore = weight.Avoid * std::clamp(risk, 0.f, 1.f);
		const float combatScore = combo && enemy ? weight.Attack : 0;

		// 즉사급 합계5 이상 또는 위험이 있으면서 HP 비율이 낮으면 생존 회피를 우선한다. 그 밖에는 점수 우열과 최소 유지 시간/전환 여유를 적용한다.
		const bool lethal =
		    risk >= 5 || (risk > 0 && guide.iCurrentHp < guide.iMaximumHp * m_GuideCatalog.LethalHpFraction);
		evaluatedThreat = risk;
		survivalOverride = lethal;
		std::uint8_t action = lethal || evadeScore > (std::max)(followScore, combatScore) ? 2
		                      : combatScore > followScore                                 ? 3
		                                                                                  : 1;
		const float scores[] = {0, followScore, evadeScore, combatScore};
		if (!lethal && action != state.Action && state.Action >= 1 && state.Action <= 3 &&
		    state.HoldElapsed < m_GuideCatalog.MinimumHoldSeconds &&
		    scores[action] < scores[state.Action] + m_GuideCatalog.SwitchMargin)
			action = state.Action;
		if (action != state.Action)
		{
			state.Action = action;
			state.HoldElapsed = 0;
		}

		// 반경3m의 16방향을 착지 함수로 보정하고 현재보다 낮은 위험·안전한 샘플 경로를 갖춘 최저 위험 후보를 찾는다. 없으면 이유만 남긴다.
		if (action == 2)
		{
			float bestRisk = risk;
			SERVER_NAV_POINT best{};
			bool found = false;
			for (unsigned i = 0; i < 16; ++i)
			{
				const float angle = i * PI / 8.f;
				SERVER_NAV_POINT p;
				if (!Find_GuideLanding(guide, guide.fPositionX + std::sin(angle) * 3.f, guide.fPositionY,
				                       guide.fPositionZ + std::cos(angle) * 3.f, p))
					continue;
				const float candidateRisk = danger(p.x, p.z);
				if (candidateRisk < bestRisk && pathSafe(p, risk))
				{
					bestRisk = candidateRisk;
					best = p;
					found = true;
				}
			}
			if (found)
			{
				C2S_MOVE move;
				move.iClientSequence = ++state.Sequence;
				move.fGoalX = best.x;
				move.fGoalZ = best.z;
				Execute_PlayerMove(guide, move);
				state.Reason =
				    lethal ? "Lethal contact predicted; survival override" : "Evade has the highest weighted score";
			}
			else
				state.Reason = "Danger detected; no lower-risk navigation candidate";
		}

		// 공격은 게시된 combo 순서와 기존 Execute_PlayerSkill을 사용한다. 반복은 step만 되돌리고 전체 combo 시간은 계속 누적한다.
		else if (action == 3 && combo && enemy)
		{
			if (state.ComboStep >= combo->Skills.size())
			{
				if (combo->Repeat)
				{
					state.ComboStep = 0;
					state.StepElapsed = 0;
				}
				else
				{
					state.ComboId.clear();
					state.Reason = "Combo completed";
				}
			}
			else if (guide.eAction == PLAYER_ACTION_STATE::NONE)
			{
				const auto* skill = m_GameplayCatalog.Find_Skill(combo->Skills[state.ComboStep]);
				const float range = skill ? (std::max)(2.f, skill->fMaximumRange) : 2.f;

				// 사거리 밖이면 적에서 guide 방향으로 max(1m,사거리×0.75) 떨어진 지점으로 이동 요청만 한다. 이 접근 시간은 StepElapsed 거절 대기에 더하지 않는다.
				if (enemyDistance > range)
				{
					C2S_MOVE move;
					move.iClientSequence = ++state.Sequence;
					const float stop = (std::max)(1.f, range * .75f);
					move.fGoalX = enemy->fPositionX + (guide.fPositionX - enemy->fPositionX) * stop / enemyDistance;
					move.fGoalZ = enemy->fPositionZ + (guide.fPositionZ - enemy->fPositionZ) * stop / enemyDistance;
					Execute_PlayerMove(guide, move);
					state.Reason = "Approaching the next combo skill range";
				}
				else
				{
					C2S_USE_SKILL command;
					command.iClientSequence = ++state.Sequence;
					command.iSkillId = combo->Skills[state.ComboStep];
					command.eTargetIntent = skill ? skill->eTargetIntent : SKILL_TARGET_INTENT_KIND::AIM_POINT;
					command.fAimX = enemy->fPositionX;
					command.fAimZ = enemy->fPositionZ;

					// 공통 skill 실행이 승인된 경우만 combo step을 올린다. 실행 거절의 판단 간격만 누적해 StepWaitMs를 넘으면 combo 예약을 종료한다.
					if (Execute_PlayerSkill(guide, command))
					{
						++state.ComboStep;
						state.StepElapsed = 0;
						state.Reason = "Skill admitted by the shared player executor";
					}
					else
					{
						state.StepElapsed += elapsed;
						state.Reason = "Skill waiting: cooldown, resource or current status";
						if (state.StepElapsed * 1000 >= combo->StepWaitMs)
						{
							state.ComboId.clear();
							state.Reason = "Unavailable skill exceeded its wait deadline";
						}
					}
				}
			}
			else
				state.Reason = "Current skill is still executing";
		}

		// 일반 추적 목표는 owner yaw 기준 뒤쪽 DesiredDistance(m)다. 실제 경로/상태 검증은 공통 이동 executor가 맡는다.
		else if (needsFollow)
		{
			const float yaw = anchor.fYawDegrees * PI / 180.f;
			C2S_MOVE move;
			move.iClientSequence = ++state.Sequence;
			move.fGoalX = anchor.fPositionX - std::sin(yaw) * m_GuideCatalog.DesiredDistance;
			move.fGoalZ = anchor.fPositionZ - std::cos(yaw) * m_GuideCatalog.DesiredDistance;
			Execute_PlayerMove(guide, move);
			state.Reason = "Maintaining the configured anchor distance";
		}
		else
		{
			if (guide.eAction == PLAYER_ACTION_STATE::NONE)
			{
				guide.hasMoveGoal = false;
				guide.MovePath.clear();
			}
			state.Reason = "Within the configured following band";
		}
		state.FollowScore = followScore;
		state.EvadeScore = evadeScore;
		state.CombatScore = combatScore;
		publishTrace();
		++it;
	}
}
