// 입장 후보·플레이어/session lookup과 퇴장할 룸 실행 상태를 선언한다. 아래 함수는 룸 실행 스레드가 직렬로 호출한다.
#include "GameRoom.h"

// 실제 연결의 수명을 붙잡고 player binding·종료 요청·초기 reliable FIFO 예약을 수행한다.
#include "ClientSession.h"
// 이 파일 본문에서는 타격 runtime API를 직접 호출하지 않는다. 기존 include 의존은 유지한다.
#include "ServerCombatHitRuntime.h"

// 입장 요청 검증과 승인·인벤토리·월드 초기 상태 메시지의 직렬화 계약을 사용한다.
#include "Network/PacketMessages.h"
// 각 초기 메시지를 지역 payload로 인코딩한다. writer 버퍼는 Frames가 복사해 소유한다.
#include "Network/PacketWriter.h"
// 공유 몸체 규격 헤더지만 이 본문에서 해당 상수를 직접 쓰지는 않는다. 기존 include를 유지한다.
#include "Gameplay/WorldCollisionContract.h"

// spawn/owner 검색, 배치 예약 검사, 퇴장 항목 제거와 상한 선택에 쓴다.
#include <algorithm>
// array·chrono·cctype는 이 파일 본문에서 직접 사용하지 않는 기존 include다.
#include <array>
#include <chrono>
#include <cctype>
// NPC 접근 위치의 yaw(도)를 라디안으로 바꾸어 sin/cos를 구하고 반대 방향을 fmod로 계산한다.
#include <cmath>
// packet·entity·시계와 콜로세움 순번의 고정 폭 정수 형식을 쓴다.
#include <cstdint>
// 이 본문은 cstdlib 함수를 직접 호출하지 않는다. 기존 include를 유지한다.
#include <cstdlib>
// 입장/퇴장과 실패한 빈 방 초기화의 진단을 Server 로그에 남긴다.
#include <iostream>
// 동시 입장 후보의 ID offset이 정수 최댓값을 넘지 않는지 검사한다.
#include <limits>
// new·set은 이 본문에서 직접 사용하지 않는 기존 include다. 동적 컨테이너의 할당 예외를 여기서 잡지는 않는다.
#include <new>
#include <set>
// 종료 진단 이름과 잘림 표식을 호출 중 차용한다.
#include <string_view>
// 검증한 후보·payload를 출력 컨테이너로 이동한다.
#include <utility>

// SquareHole ID·각도 상수·Unix ms·파괴 상태 wire 변환을 룸의 다른 구현과 공유한다.
#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

// Tick의 REGISTER_SESSION이 연결을 약한 참조로 등록한다. 큐의 shared_ptr는 호출 동안 연결 수명을 붙잡는다.
// 유효하지 않은 입력은 무시하며 이 단계는 player를 만들거나 session의 player ID를 바꾸지 않는다.
void LostArk::Server::CGameRoom::Handle_Register(
	const std::shared_ptr<CClientSession>& session)
{
	if (nullptr == session || session->Get_SessionId() == INVALID_SESSION_ID)
		return;

	m_Sessions.insert_or_assign(session->Get_SessionId(), session);
}

// 단독 Join과 파티/콜로세움 전환이 목적 룸의 player 값을 준비한다. precedingEntries는 같은 배치의 ID와 spawn 예약이다.
// 입력 참조는 호출 중 차용하고 후보는 실제 session을 공유 소유한다. 명시적 false는 staged와 룸 membership을 보존하되 진단은 바꾼다.
// 성공도 룸 삽입·송신은 하지 않는다. 컨테이너 할당 예외를 bool 실패로 변환하거나 rollback하는 함수는 아니다.
bool LostArk::Server::CGameRoom::Stage_PlayerEntry(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
	STAGED_PLAYER_ENTRY& staged,
	LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory,
	const LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId,
	const std::string& raidReturnNpcPlacementId,
	const SERVER_PURSE& carriedPurse, const bool hasCarriedCharacterState)
{
	using namespace LostArk::Shared;

	// 거절 이유를 먼저 초기화한다. 성공 시 outReason은 성공 코드로 바꾸지 않으므로 호출자는 bool을 먼저 판정한다.
	outReason = SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED;
	status.clear();
	const auto reject = [&outReason, &status](
		const SESSION_DIAGNOSTIC_REASON reason, const char* detail)
	{
		outReason = reason;
		status = detail;
		return false;
	};

	// 기존 player와 앞선 후보를 포함한 수용량·ID 범위를 검사한다. ID는 이 단계에서 예약값만 계산하고 증가시키지 않는다.
	const std::size_t offset = precedingEntries.size();
	if (!raidReturnNpcPlacementId.empty() &&
		((WORLD_ID::VALTAN_ARENA != m_eWorldId && WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId) ||
		 (raidReturnNpcPlacementId != "npc.bern.beda.guide" && raidReturnNpcPlacementId != "npc.bern.aylara")))
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
			"invalid raid return guide or destination world");
	if (!m_isReady || nullptr == session || session->Is_Closing() ||
		INVALID_SESSION_ID == session->Get_SessionId() ||
		!Is_Valid_EnterWorld(enterWorld) || enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(session->Get_SessionId()) ||
		m_Players.size() + offset >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		INVALID_PLAYER_ID == m_iNextPlayerId ||
		INVALID_NET_ENTITY_ID == m_iNextNetEntityId ||
		offset > (std::numeric_limits<PLAYER_ID>::max)() - m_iNextPlayerId ||
		offset > (std::numeric_limits<NET_ENTITY_ID>::max)() - m_iNextNetEntityId)
	{
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
			"player entry room/session/identity validation failed");
	}

	// 콜로세움의 승인된 좌석, 명시한 override, 일반 빈 PLAYER_SPAWN 순으로 착지 원본을 선택한다. 포인터는 룸 bootstrap을 차용한다.
	const WORLD_BOOTSTRAP_PLACEMENT* spawn = nullptr;
	if (m_iColosseumMatchId)
	{
		const auto seat = std::find(m_ColosseumSessions.begin(), m_ColosseumSessions.end(), session->Get_SessionId());
		if (seat == m_ColosseumSessions.end())
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				"session does not belong to this Colosseum match");
		const std::size_t index = static_cast<std::size_t>(seat - m_ColosseumSessions.begin());
		const std::string id = std::string("player.spawn.colosseum.team") +
			(index % 2u ? "b.0" : "a.0") + std::to_string(index / 2u + 1u);
		spawn = Find_Placement(id);
		if (!spawn)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED, "Colosseum team spawn missing");
	}
	else if (!spawnPlacementOverrideId.empty())
	{
		// override는 NPC 등을 포함한 특정 placement ID를 직접 지정한다. PLAYER_SPAWN 종류나
		// 일반 spawn의 단독 점유 검사를 적용하지 않아 여러 복귀자가 같은 placement를 사용할 수 있다.
		spawn = Find_Placement(spawnPlacementOverrideId);

		// 비활성 TRIGGER_BOX도 허용한다. Maharaka에서 Bern 바다로 복귀하는 착지처럼
		// trigger 동작 없이 저작한 위치만 쓰는 배치를 지원한다.
		if (nullptr == spawn ||
			(!spawn->isEnabled && WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != spawn->eKind))
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				"spawn placement override id does not exist in this world's bootstrap");
	}
	else
	{
		// 일반 spawn은 기존 player와 같은 배치의 앞선 후보가 점유한 ID를 건너뛴다. 첫 빈 항목을 사용한다.
		for (const auto& candidate : m_WorldBootstrap.Get_Placements())
		{
			if (!candidate.isEnabled || WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != candidate.eKind)
				continue;
			const bool occupied = std::any_of(m_Players.begin(), m_Players.end(),
				[&candidate](const auto& value)
				{ return value.second.strSpawnPlacementId == candidate.strPlacementId; });
			const bool reserved = std::any_of(precedingEntries.begin(), precedingEntries.end(),
				[&candidate](const auto& value)
				{ return value.Player.strSpawnPlacementId == candidate.strPlacementId; });
			if (!occupied && !reserved)
			{
				spawn = &candidate;
				break;
			}
		}
		if (nullptr == spawn)
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				"target room has fewer free player spawns than the transfer batch");
	}

	// 입력 identity/외형을 지역 player로 복사한다. 기존 룸 및 session binding은 아직 바꾸지 않는다.
	STAGED_PLAYER_ENTRY candidate{};
	candidate.pSession = session;
	SERVER_PLAYER& player = candidate.Player;
	player.iSessionId = session->Get_SessionId();
	player.iPlayerId = m_iNextPlayerId + static_cast<PLAYER_ID>(offset);
	player.iNetEntityId = m_iNextNetEntityId + static_cast<NET_ENTITY_ID>(offset);
	player.eCharacterClass = enterWorld.eCharacterClass;
	player.strNickName = enterWorld.strNickName;
	player.iVoiceType = enterWorld.iVoiceType;
	player.strAppearanceJson = enterWorld.strAppearanceJson;

	// 출발 때의 칭호라도 목적 룸 catalog에 있어야 유지한다. 알 수 없는 ID는 칭호 없음으로 준비한다.
	player.iHonorTitleId = m_HonorTitleCatalog.Has_Title(carriedHonorTitleId) ?
		carriedHonorTitleId : INVALID_HONOR_TITLE_ID;
	player.strSpawnPlacementId = spawn->strPlacementId;
	if (m_iColosseumMatchId)
	{
		player.iColosseumMatchId = m_iColosseumMatchId;
		player.iColosseumArrivalIndex = static_cast<std::uint8_t>(
			std::find(m_ColosseumSessions.begin(), m_ColosseumSessions.end(), player.iSessionId) -
			m_ColosseumSessions.begin());
		player.iColosseumTeam = player.iColosseumArrivalIndex % 2u;
		player.bColosseumParticipant = true;
	}
	player.strRaidReturnNpcPlacementId = raidReturnNpcPlacementId;
	player.fPositionY = spawn->fPositionY;
	const std::uint16_t bernSquareHoleId = WORLD_ID::BERN == m_eWorldId ?
		Resolve_BernSquareHoleId(spawnPlacementOverrideId) : 0u;
	if (!spawnPlacementOverrideId.empty() && 0u == bernSquareHoleId)
	{
		// SquareHole 이외의 override는 NPC 접근 배치 방식으로 처리한다. 종류가 NPC인지 따로 검사하지 않는다.
		// 벽에 붙은 원점 대신 world XZ 전방(sin(yaw), cos(yaw))으로 2.5m 옮기고 NPC 쪽으로 180도 돌린다.
		// 그 결과를 아래 navigation 투영의 입력으로 사용한다.
		constexpr float NPC_APPROACH_OFFSET_M = 2.5f;
		const float yawRadians = spawn->fYawDegrees * DEGREES_TO_RADIANS;
		player.fPositionX = spawn->fPositionX + std::sin(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fPositionZ = spawn->fPositionZ + std::cos(yawRadians) * NPC_APPROACH_OFFSET_M;
		player.fYawDegrees = std::fmod(spawn->fYawDegrees + 180.f, 360.f);
	}
	else
	{
		player.fPositionX = spawn->fPositionX;
		player.fPositionZ = spawn->fPositionZ;
		player.fYawDegrees = spawn->fYawDegrees;
	}

	// 목적 룸 class 정의로 HP·자원·자세·게이지를 새로 초기화한다. 이 전환은 전투 중 상태를 그대로 복사하는 경로가 아니다.
	const PLAYER_RUNTIME_PROFILE* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
	if (nullptr == profile)
		return reject(SESSION_DIAGNOSTIC_REASON::SERVER_PROFILE_MISSING,
			"selected character class has no runtime profile");
	player.eStance = profile->eDefaultStance;
	player.iCurrentHp = player.iMaximumHp = profile->iMaximumHp;
	player.iCurrentResource = player.iMaximumResource = profile->iMaximumResource;
	player.fMoveSpeed = profile->fMoveSpeed;
	player.iMaximumIdentity = profile->iMaximumIdentity;
	CPlayerSkillSystem::Reset_Gauges(player, m_GameplayCatalog);
	player.iCurrentMadness = 0u;
	player.dMadnessRemainder = 0.;
	player.iMaximumMadness = SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM;
	player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
	player.Clear_KoukuInteractionState();
	player.isCombatReady = WORLD_ID::VALTAN_ARENA != m_eWorldId;

	// SquareHole은 전용 착지 검사를 사용한다. 일반 navigation 투영은 로드되어 있을 때만 수행하며
	// 일반 spawn은 높이 미지정, override는 저작 Y를 힌트로 사용한다.
	if (0u != bernSquareHoleId)
	{
		SERVER_NAV_POINT landing{};
		if (!Resolve_SquareHoleDestination(player, bernSquareHoleId, landing))
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_NAVIGATION_FAILED,
				"Bern square hole failed authored landing admission");
		player.fPositionX = landing.x;
		player.fPositionY = landing.y;
		player.fPositionZ = landing.z;
	}
	else if (m_ServerNavigation.Is_Loaded())
	{
		SERVER_NAV_POINT projected{};
		if (!m_ServerNavigation.Project_Point(player.fPositionX, player.fPositionZ, projected,
			spawnPlacementOverrideId.empty() ? NAVIGATION_HEIGHT_UNKNOWN : player.fPositionY))
			return reject(SESSION_DIAGNOSTIC_REASON::SERVER_NAVIGATION_FAILED,
				"player spawn could not project onto Server navigation");
		player.fPositionX = projected.x;
		player.fPositionY = projected.y;
		player.fPositionZ = projected.z;
	}

	// Bern의 SquareHole 외 override에 선박 복귀 기록이 있으면 유효한 탈것 ID와 출항 부두를 복원한다.
	// Maharaka 복귀에 쓰는 기록이며 여기서는 source world를 검사하지 않는다. 기록/정의가 없으면 도보 상태다.
	if (WORLD_ID::BERN == m_eWorldId && !spawnPlacementOverrideId.empty() && 0u == bernSquareHoleId)
	{
		const auto shipIter = m_MaharakaShipReturnBySession.find(session->Get_SessionId());
		if (shipIter != m_MaharakaShipReturnBySession.end() &&
			nullptr != m_VehicleCatalog.Find_Vehicle(shipIter->second.iVehicleId))
		{
			player.iVehicleId = shipIter->second.iVehicleId;
			player.bShipDockValid = true;
			player.fShipDockX = shipIter->second.fDockX;
			player.fShipDockY = shipIter->second.fDockY;
			player.fShipDockZ = shipIter->second.fDockZ;
			player.fShipDockYawDegrees = shipIter->second.fDockYawDegrees;
		}
	}
	if (hasCarriedCharacterState || !carriedInventory.empty() || 0u != bernSquareHoleId)
	{
		// 출발 player의 보상·소지품·통화를 그대로 가져와 새 입장 기본 지급을 대체한다.
		// 명시적 carried-state 또는 SquareHole은 빈 inventory도 유효하므로 비어 있다는 이유로 재지급하지 않는다.
		player.Inventory = carriedInventory;
		player.Purse = carriedPurse;
	}
	else
	{
		// 이월 상태가 없는 입장에는 정의가 있는 HP 물약 세 종류를 최대 500개씩 지급하고 시작 통화/장착을 준비한다.
		for (const char* potionId : { "POTION_HP_SMALL", "POTION_HP_MEDIUM", "POTION_HP_LARGE" })
		{
			const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(potionId);
			if (nullptr == definition)
				continue;
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = potionId;
			item.iQuantity = (std::min)(500u, definition->iMaxStack);
			player.Inventory.push_back(std::move(item));
		}
		player.Purse = m_ItemCatalog.Get_StartingPurse();
		player.bRestoreAvailable = true;

		// 새 캐릭터의 시작 장비는 catalog가 지정한 슬롯에 장착하고 강화 가능한 항목만 초기 단계를 준다.
		for (const auto& [itemId, slot] : m_ItemCatalog.Get_StartingEquipment())
		{
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = itemId;
			item.iQuantity = 1u;
			item.eEquippedSlot = slot;
			if (const auto* definition = m_ItemCatalog.Find_Item(itemId); definition &&
				Is_Upgradeable_Equipment(itemId, definition->strEquipSlot))
				item.iUpgradeLevel = INITIAL_EQUIPMENT_UPGRADE_LEVEL;
			player.Inventory.push_back(std::move(item));
		}
	}

	// 모든 명시적 검사를 통과한 후보만 출력으로 이동한다. 다음 단계가 초기 packet과 FIFO 수용량을 준비한다.
	staged = std::move(candidate);
	return true;
}

// Join/파티/콜로세움 전환이 목적 룸의 현재 상태와 같은 입장 batch를 초기 reliable 메시지 목록으로 만든다.
// 지역 payload를 끝까지 준비한 뒤 entry.Frames만 교체한다. false는 기존 Frames를 보존하며 socket 송신이나 membership 변경은 없다.
bool LostArk::Server::CGameRoom::Build_PlayerEntryFrames(
	STAGED_PLAYER_ENTRY& entry, const std::span<const STAGED_PLAYER_ENTRY> batch,
	std::string& status)
{
	using namespace LostArk::Shared;
	std::vector<PACKET_FRAME> frames;

	// Write_Message의 검증 실패는 packet 종류를 진단에 남긴다. 성공한 writer의 버퍼는 지역 Frames로 복사한다.
	const auto append = [&frames, &status](const PACKET_TYPE type, const auto& message)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
		{
			status = "initial entry payload failed validation, packet=" +
				std::to_string(static_cast<std::uint16_t>(type));
			return false;
		}
		frames.push_back({ type, writer.Get_Buffer() });
		return true;
	};

	// 승인과 active/pinned gameplay revision을 가장 먼저 넣고 해당 player의 인벤토리를 잇는다. Client가 이후 entity 정의를 해석할 기준이다.
	S2C_ENTER_ACCEPTED accepted{};
	accepted.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	accepted.eWorldId = m_eWorldId;
	accepted.iPlayerId = entry.Player.iPlayerId;
	accepted.iNetEntityId = entry.Player.iNetEntityId;
	accepted.ActiveGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(accepted.RequiredPinnedGameplayRevisions))
	{
		status = "initial entry pinned gameplay revisions failed validation";
		return false;
	}
	if (!append(PACKET_TYPE::S2C_ENTER_ACCEPTED, accepted))
		return false;
	S2C_INVENTORY_SNAPSHOT inventory{};
	inventory.Items = entry.Player.Inventory;
	inventory.iSilver = entry.Player.Purse.iSilver;
	inventory.iGold = entry.Player.Purse.iGold;
	if (!append(PACKET_TYPE::S2C_INVENTORY_SNAPSHOT, inventory))
		return false;

	// Valtan 파괴 상태는 초기화가 필수이며 전체 동기화로 보낸다. encounter prop은 초기화된 경우에만 뒤에 추가한다.
	if (WORLD_ID::VALTAN_ARENA == m_eWorldId)
	{
		if (!m_WorldDestructionRuntime.Is_Initialized())
		{
			status = "initial world destruction runtime is not initialized";
			return false;
		}
		S2C_WORLD_DESTRUCTION_FULL_SYNC fullSync{};
		fullSync.strCombatRuntimeRevision = m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
		fullSync.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		fullSync.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
		for (const auto& state : m_WorldDestructionRuntime.Get_GroupStates())
			fullSync.GroupStates.push_back(To_NetworkDestructionState(state));
		fullSync.Diagnostics = Build_WorldDestructionDiagnostics();
		if (!append(PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, fullSync))
			return false;
	}
	if (m_EncounterPropRuntime.Is_Initialized())
	{
		S2C_ENCOUNTER_PROP_SYNC props{};
		props.strPropSetId = m_EncounterPropRuntime.Get_PropSetId();
		props.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		props.iEncounterEpoch = m_EncounterPropRuntime.Get_EncounterEpoch();
		for (const auto& slot : m_EncounterPropRuntime.Get_SlotStates())
		{
			ENCOUNTER_PROP_SLOT_WIRE wire{};
			wire.strSlotId = slot.strSlotId;
			wire.eState = slot.eState;
			wire.iStateVersion = slot.iStateVersion;
			wire.iStateStartTick = slot.iStateStartTick;
			wire.iOccurrenceSequence = slot.iOccurrenceSequence;
			props.Slots.push_back(std::move(wire));
		}
		if (!append(PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC, props))
			return false;
	}

	// primary entity를 먼저 넣은 뒤 종속 boss를 넣는다. WORLD_OBJECT는 이 목록에서 제외하고,
	// 종속 entity의 owner·encounter·고정 revision과 ID 중복을 검사하여 앞선 owner 없이 초기화되는 일을 막는다.
	std::unordered_set<NET_ENTITY_ID> admittedWorldEntityIds;
	for (const bool dependentPass : { false, true })
	{
		for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
		{
			if (entity.eKind == WORLD_BOOTSTRAP_KIND::WORLD_OBJECT)
				continue;
			const bool isDependent = INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId;
			if (isDependent != dependentPass)
				continue;
			if (!admittedWorldEntityIds.insert(entity.iNetEntityId).second)
			{
				status = "Initial world entity ID is duplicated";
				return false;
			}
			if (isDependent)
			{
				const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
					[&entity](const SERVER_WORLD_ENTITY& candidate)
					{ return candidate.iNetEntityId == entity.iOwnerBossNetEntityId; });
				if (!admittedWorldEntityIds.contains(entity.iOwnerBossNetEntityId) ||
					owner == m_WorldEntities.end() || WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
					INVALID_NET_ENTITY_ID != owner->iOwnerBossNetEntityId ||
					owner->strEncounterId != entity.strEncounterId ||
					owner->PinnedDefinitionRevision !=
						entity.PinnedDefinitionRevision)
				{
					status = "Initial dependent boss has no preceding primary owner";
					return false;
				}
			}
			std::vector<std::uint8_t> payload;
			if (!Build_WorldEntitySpawnedPayload(entity, payload))
			{
				status = "World entity spawn payload preflight failed: " + entity.strPlacementId;
				return false;
			}
			frames.push_back({ PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, std::move(payload) });
		}
	}

	// 진행 중인 섬/레이드/묶음의 입장 시점 상태를 더한다. 선택적 Build_*가 false인 상태는 생략하며 append 실패만 입장 실패로 전파한다.
	S2C_KOUKUSAYDON_RAID_STATE raidState;
	if (m_MaharakaWaterpangIntro)
	{
		auto intro = *m_MaharakaWaterpangIntro;
		intro.iServerTick = m_iServerTick;
		if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, intro))
			return false;
	}

	// 늦게 들어온 player도 아직 살아 있는 Debug 강제 이벤트를 같은 Server 시계로 받는다.
	if (m_MaharakaWaterpangDebugEvent && Is_MaharakaWaterpangDebugEventLive(m_iServerTick))
	{
		auto forced = *m_MaharakaWaterpangDebugEvent;
		forced.iServerTick = m_iServerTick;
		if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, forced))
			return false;
	}
	if (Build_KoukuRaidState(raidState) && !append(PACKET_TYPE::S2C_KOUKUSAYDON_RAID_STATE, raidState))
		return false;
	S2C_GATE_PROGRESS_STATE gateState;
	if (Build_GateProgressState(gateState, false, GATE_PROGRESS_VOTE_RESULT::NONE) &&
		!append(PACKET_TYPE::S2C_GATE_PROGRESS_STATE, gateState))
		return false;
	S2C_KOUKUSAYDON_BUNDLE_STATE bundleState;
	if (Build_KoukuBundleState(bundleState) && !append(PACKET_TYPE::S2C_KOUKUSAYDON_BUNDLE_STATE, bundleState))
		return false;
	{
		// stage/run 자연 종료 뒤에도 남은 독립 World 재생 행을 늦은 입장에 포함한다.
		for (auto play : m_KoukuSaydonPatternAudition.WorldPlays)
		{
			if (play.bUntilDestroyed || play.iCombatBodyNetEntityId)
				continue; // 살아 있는 전투 몸체의 재생은 아래 cue 경로가 맡는다.
			if (play.iDurationMs && Has_ReachedServerTick(m_iServerTick,
				Add_ServerTicksSkippingReservedZero(play.iStartTick,
					CKoukuSaydonLogicRuntime::Ticks_FromMs(play.iDurationMs))))
				continue;
			play.iServerTick = m_iServerTick;
			if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, play))
				return false;
		}
	}

	// 취소되지 않았고 살아 있는 본체 또는 생성 대기 본체가 있는 cue만 재생한다. 오래된 cue 전체를 재전송하지 않는다.
	for (const auto& cue : m_KoukuDamageableWorldCues)
	{
		if (cue.bCancelled)
			continue;
		const auto live = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
			[&](const auto& row)
			{ return row.iNetEntityId == cue.iBodyId && row.iCurrentHp; });
		const auto pending = std::find_if(m_PendingKoukuWorldBodies.begin(), m_PendingKoukuWorldBodies.end(),
			[&](const auto& row)
			{ return row.iNetEntityId == cue.iBodyId && row.iCurrentHp; });
		if (live == m_WorldEntities.end() && pending == m_PendingKoukuWorldBodies.end())
			continue;
		auto play = cue.Play;
		play.iServerTick = m_iServerTick;
		if (!append(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, play))
			return false;
	}

	// 현재 전투 object의 spawn과 기존/신규 player들을 마지막에 붙인다. 새 batch에는 자기 player도 포함될 수 있다.
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> combatObjects;
	m_CombatObjectRuntime.Build_LiveSpawnMessages(0u == m_iServerTick ? 1u : m_iServerTick, combatObjects);
	for (const auto& object : combatObjects)
		if (!append(PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, object))
			return false;
	const auto appendPlayer = [&append](const SERVER_PLAYER& player)
	{
		S2C_PLAYER_SPAWNED message{};
		message.iPlayerId = player.iPlayerId;
		message.iNetEntityId = player.iNetEntityId;
		message.eCharacterClass = player.eCharacterClass;
		message.eControlKind = player.eControlKind;
		message.strWaterpangNpcArchetypeId = player.strWaterpangNpcArchetypeId;
		message.strNickName = player.strNickName;
		message.iVoiceType = player.iVoiceType;
		message.strAppearanceJson = player.strAppearanceJson;
		message.fPositionX = player.fPositionX;
		message.fPositionY = player.fPositionY;
		message.fPositionZ = player.fPositionZ;
		message.fYawDegrees = player.fYawDegrees;
		return append(PACKET_TYPE::S2C_PLAYER_SPAWNED, message);
	};
	for (const auto& [id, player] : m_Players)
	{
		(void)id;
		if (!appendPlayer(player))
			return false;
	}
	for (const auto& staged : batch)
		if (!appendPlayer(staged.Player))
			return false;

	// 완성된 순서 그대로 호출자의 입장 후보에 넘긴다. 실제 FIFO 예약/commit은 호출자의 transaction이 담당한다.
	entry.Frames = std::move(frames);
	return true;
}

// 단독 Join이 초기 FIFO 예약 성공 뒤 player와 역방향 lookup을 룸에 삽입하고 session binding을 확정한다.
// Stage 이후 ID/룸 상태가 바뀌지 않는 직렬 호출 전제다. 할당 예외를 잡지 않아 중간 삽입 rollback은 보장하지 않는다.
// 파티/콜로세움 일괄 전환은 별도 map 복사본을 swap하므로 이 함수를 직접 사용하지 않는다.
void LostArk::Server::CGameRoom::Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry)
{
	const SERVER_PLAYER& player = entry.Player;
	m_Sessions.insert_or_assign(player.iSessionId, entry.pSession);
	m_Players.emplace(player.iPlayerId, player);
	if (m_iColosseumMatchId)
	{
		auto& committed = m_Players.at(player.iPlayerId);
		committed.fColosseumSpawnX = committed.fPositionX;
		committed.fColosseumSpawnY = committed.fPositionY;
		committed.fColosseumSpawnZ = committed.fPositionZ;
		committed.fColosseumSpawnYaw = committed.fYawDegrees;
	}

	// 두 lookup과 다음 ID를 갱신한 뒤 연결이 새 player ID를 가리키게 한다. 아직 FIFO 잠금은 호출자가 보유한다.
	m_PlayerIdBySessionId.emplace(player.iSessionId, player.iPlayerId);
	m_PlayerIdByEntityId.emplace(player.iNetEntityId, player.iPlayerId);
	++m_iNextPlayerId;
	++m_iNextNetEntityId;
	entry.pSession->Bind_PlayerId(player.iPlayerId);
}

// Tick의 ENTER_WORLD가 등록된 실제 session을 목적 룸 player로 바꾼다. 초기 메시지/FIFO 준비 실패는 입장 전 상태에서 연결 종료를 요청한다.
// 룸 삽입과 FIFO commit 뒤 안내 복귀·다른 player 통지를 진행한다. true는 이 로컬 commit을 뜻하며 상대 Client 수신까지 보장하지 않는다.
bool LostArk::Server::CGameRoom::Join(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
	const std::string& spawnPlacementOverrideId,
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& carriedInventory,
	const LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId,
	const std::string& raidReturnNpcPlacementId,
	const SERVER_PURSE& carriedPurse,
	const LostArk::Shared::WORLD_ID sourceWorld, const bool hasCarriedCharacterState)
{
	using namespace LostArk::Shared;

	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session || !Is_Valid_EnterWorld(enterWorld) ||
		enterWorld.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.contains(sessionId) ||
		m_Players.size() >= MAX_WORLD_SNAPSHOT_PLAYERS ||
		m_iNextPlayerId == INVALID_PLAYER_ID ||
		m_iNextNetEntityId == INVALID_NET_ENTITY_ID)
	{
		if (nullptr != session)
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				WSAEINVAL,
				"ENTER_WORLD failed room/session/id validation");
		}
		return false;
	}

	// 수용량 거절은 typed ROOM_FULL을 먼저 예약한다. 성공하면 FIFO flush 뒤 종료를 요청하고 실패하면 즉시 종료를 요청한다.
	if (Is_PlayerAdmissionFull())
	{
		const std::size_t enabledPlayerSpawnCount =
			static_cast<std::size_t>(std::count_if(
				m_WorldBootstrap.Get_Placements().begin(),
				m_WorldBootstrap.Get_Placements().end(),
				[](const WORLD_BOOTSTRAP_PLACEMENT& placement)
				{
					return placement.isEnabled &&
						WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind;
				}));
		const std::string roomFullCounts =
			"activePlayers=" + std::to_string(m_Players.size()) +
			", registeredSessionsIncludingCandidate=" +
			std::to_string(m_Sessions.size()) +
			", candidateSessionId=" + std::to_string(sessionId) +
			", candidateRegistered=" +
			(m_Sessions.contains(sessionId) ? "true" : "false") +
			", enabledPlayerSpawns=" +
			std::to_string(enabledPlayerSpawnCount);

		// 종료 동작의 접두 문자열 공간을 남기려고 roster 진단을 960byte 안으로 제한한다.
		// 마지막 수신 시각이 없거나 관측 시각보다 미래이면 경과 ms는 0으로 기록한다.
		constexpr std::size_t MAX_ROOM_FULL_CONTEXT_BYTES = 960u;
		const std::uint64_t observedUnixMilliseconds =
			Current_UnixMilliseconds();
		std::string roomFullContext = roomFullCounts + ", activeRoster=[";
		bool isFirstRosterEntry = true;
		for (const auto& [playerId, player] : m_Players)
		{
			const std::shared_ptr<CClientSession> incumbent =
				Find_Session(player.iSessionId);
			std::string peer = "unavailable";
			std::uint64_t lastInboundUnixMilliseconds = 0u;
			if (nullptr != incumbent)
			{
				const CLIENT_SESSION_PEER_ENDPOINT& endpoint =
					incumbent->Get_PeerEndpoint();
				peer = endpoint.strAddress + ':' +
					std::to_string(endpoint.iPort);
				lastInboundUnixMilliseconds =
					incumbent->Get_LastInboundUnixMilliseconds();
			}
			const std::uint64_t lastInboundAgeMilliseconds =
				0u != lastInboundUnixMilliseconds &&
				observedUnixMilliseconds >= lastInboundUnixMilliseconds ?
				observedUnixMilliseconds - lastInboundUnixMilliseconds : 0u;
			const std::string rosterEntry =
				(isFirstRosterEntry ? "" : ", ") +
				std::string{ "{sessionId=" } +
				std::to_string(player.iSessionId) +
				", playerId=" + std::to_string(playerId) +
				", spawn=" + player.strSpawnPlacementId +
				", peer=" + peer +
				", lastInboundUnixMs=" +
				std::to_string(lastInboundUnixMilliseconds) +
				", lastInboundAgeMs=" +
				std::to_string(lastInboundAgeMilliseconds) + '}';
			if (roomFullContext.size() + rosterEntry.size() + 1u >
				MAX_ROOM_FULL_CONTEXT_BYTES)
			{
				constexpr std::string_view TRUNCATED =
					", {truncated=true}]";
				roomFullContext.resize((std::min)(
					roomFullContext.size(),
					MAX_ROOM_FULL_CONTEXT_BYTES - TRUNCATED.size()));
				roomFullContext.append(TRUNCATED);
				break;
			}
			roomFullContext += rosterEntry;
			isFirstRosterEntry = false;
		}
		if (roomFullContext.empty() || ']' != roomFullContext.back())
			roomFullContext += ']';
		if (Send_EnterRejected(
				session, ENTER_WORLD_REJECTION_REASON::ROOM_FULL))
		{
			session->Request_Close_After_Flush(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"typed ROOM_FULL rejection flushed before close; " +
					roomFullContext);
		}
		else
		{
			session->Request_Close(
				SESSION_DIAGNOSTIC_REASON::SERVER_EXPECTED_ROOM_FULL,
				0,
				"ROOM_FULL rejection could not be queued; " +
					roomFullContext);
		}
		return false;
	}

	// 지역 player와 자기 자신을 포함한 초기 Frames를 준비한다. 준비 실패마다 진단을 기록하고 연결을 닫되 membership은 만들지 않는다.
	STAGED_PLAYER_ENTRY entry{};
	SESSION_DIAGNOSTIC_REASON reason{};
	std::string status;
	if (!Stage_PlayerEntry(session, enterWorld, {}, entry, reason, status,
			spawnPlacementOverrideId, carriedInventory, carriedHonorTitleId,
			raidReturnNpcPlacementId, carriedPurse, hasCarriedCharacterState))
	{
		session->Request_Close(reason, WSAEINVAL, status);
		return false;
	}
	if (!Build_PlayerEntryFrames(entry, std::span<const STAGED_PLAYER_ENTRY>{ &entry, 1u }, status))
	{
		m_strStatus = status;
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, 0, status);
		return false;
	}

	// 연결의 진단/송신 mutex 아래 FIFO 복사본을 예약한다. Prepare가 false면 기존 큐를 보존하고 잠금을 해제한다.
	CClientSession::RELIABLE_BATCH_TRANSACTION outbound;
	if (!outbound.Prepare({ { session, entry.Frames } }, status))
	{
		session->Request_Close(SESSION_DIAGNOSTIC_REASON::SERVER_INITIAL_SYNC_ENQUEUE_FAILED, 0, status);
		return false;
	}

	// 여기부터 룸 상태가 바뀐다. membership 삽입은 할당할 수 있어 예외에 대한 전체 rollback 보장은 없다.
	// FIFO commit이 잠금을 푼 뒤에 안내 복귀와 다른 수신자 대상 spawn 통지를 수행한다.
	Commit_PlayerEntry(entry);
	outbound.Commit();
	Resume_PersonalGuide(sessionId, sourceWorld);
	Broadcast_Spawned(entry.Player, sessionId);
	std::cout << "Player joined. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId << ", PlayerId=" << entry.Player.iPlayerId
		<< ", Spawn=" << entry.Player.strSpawnPlacementId
		<< ", RoomPlayers=" << m_Players.size() << '\n';
	return true;
}

// Tick 정리 큐와 단독/파티/콜로세움 전환이 session의 예약·player·lookup을 순서대로 해제한다. 본문은 룸 mutex를 직접 잡지 않는다.
// publishDeparture=false는 안내/대기열/명시적 despawn 통지를 억제하지만 모든 하위 호출의 송신을 차단하는 전역 스위치는 아니다.
// 정리된 앞부분을 rollback하지 않으며 빈 방 초기화 실패를 로그로 남긴다. 하위 초기화 실패는 룸을 실패 상태로 내릴 수 있다.
void LostArk::Server::CGameRoom::Leave(
	const SESSION_ID sessionId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason, const bool publishDeparture)
{
	using namespace LostArk::Shared;

	// 새 Client Level이 새 안내 제어 순번을 사용하도록 지운다. 월드 이동이면 actor를 유지한 대기, 다른 퇴장이면 안내 해제다.
	m_GuideControlSequences.erase(sessionId);
	if (reason == PLAYER_DESPAWN_REASON::LEVEL_CHANGED)
		Suspend_PersonalGuide(sessionId);
	else
		Remove_Guide(sessionId, publishDeparture);

	// 퇴장 session만 콜로세움 기대 인원/대기열에서 제거하고 나머지는 대기한다. 빈 큐의 마감 시각은 해제한다.
	Remove_ColosseumExpectedSession(sessionId);
	const auto queuedRemoved = std::erase_if(m_ColosseumQueue,
		[sessionId](const COLOSSEUM_QUEUE_ENTRY& entry) { return entry.iSessionId == sessionId; });
	if (m_ColosseumQueue.empty())
		m_iColosseumQueueDeadline = 0u;
	if (queuedRemoved && publishDeparture && !m_bColosseumTransferPending)
		Broadcast_ColosseumQueueState();

	// 참가자가 나가면 진행 중 쿠크 raid를 중단한다. Valtan 예약은 owner와 실행 phase에 따라 취소 범위를 나눈다.
	if (Is_KoukuRaidRunning())
	{
		const auto departing = m_PlayerIdBySessionId.find(sessionId);
		if (departing != m_PlayerIdBySessionId.end() &&
			std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), departing->second) !=
				m_KoukuRaid.PlayerIds.end())
			Stop_KoukuRaid("A raid participant left the room");
	}
	m_KoukuRaidReceipts.erase(sessionId);
	if (sessionId == m_ValtanPatternIdAudition.iOwnerSessionId)
	{
		Cancel_ValtanNextPatternReservation("owner left the room");
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase)
		{
			if (SERVER_WORLD_ENTITY* boss =
				Find_AuditionBoss(m_ValtanPatternIdAudition.strBossPlacementId))
			{
				std::erase(boss->PendingPatternIds, m_ValtanPatternIdAudition.strPatternId);
				boss->bAutomaticPatternSequenceAuditionOverride = true;
				boss->bAutomaticPatternSequenceAuditionHold = true;
			}
			Cancel_ValtanPatternIdAudition("owner left before the occurrence started");
		}
		else
		{
			// 이미 진행 중인 occurrence는 다른 player와 정상 종료할 수 있게 남긴다.
			// 나간 owner가 그 실행의 종료 지점에 다음 패턴을 덧붙이지 못하도록 소유만 해제한다.
			m_ValtanPatternIdAudition.iOwnerSessionId = INVALID_SESSION_ID;
		}
	}

	// 세션 대상 응답을 버리고 owner의 Flow/Timeline 실행을 종료한다. 여기서는 각 하위 종료 함수의 상태 변경을 되돌리지 않는다.
	m_ValtanNextPatternReceiptBySessionId.erase(sessionId);
	if (sessionId == m_ValtanPatternFlowAudition.iOwnerSessionId &&
		Is_ValtanPatternFlowRunning())
	{
		Abort_ValtanPatternFlowForOwner(
			sessionId, "Valtan pattern-flow owner left the room");
		(void)Flush_ValtanPatternFlowLifecycle();
	}
	if (sessionId == m_ValtanTimelineAudition.iOwnerSessionId &&
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
			m_ValtanTimelineAudition.ePhase)
	{
		Stop_ValtanTimelineRow();
	}

	// 이 owner의 쿠크 world body를 취소한다. solo Mario의 필수 귀환 전 이탈이면 원인을 남기고 실패로 정리한다.
	Cancel_KoukuWorldBodies({}, sessionId);
	const bool koukuOwnerLeft = sessionId == m_KoukuSaydonPatternAudition.iOwnerSessionId;
	const auto soloMarioDeparture = std::find_if(
		m_KoukuSaydonPatternAudition.Members.begin(), m_KoukuSaydonPatternAudition.Members.end(),
		[sessionId, koukuOwnerLeft](const auto& member) {
			return member.bMarioSoloReturnRequired && !member.bCompletionChainSuccessQueued &&
				(koukuOwnerLeft || member.iMarioEntrantSessionId == sessionId);
		});
	if (soloMarioDeparture != m_KoukuSaydonPatternAudition.Members.end())
	{
		m_strStatus = std::string("Mario solo ") +
			(soloMarioDeparture->iMarioEntrantSessionId == sessionId ? "entrant " : "run owner ") +
			(reason == PLAYER_DESPAWN_REASON::DISCONNECTED ? "disconnected" : "left the room") + " before phase 2";

		// solo Mario의 필수 귀환 전에 나간 참가자 또는 실행 owner의 중단 원인을 Server 로그에 남긴다.
		std::cout << "[MarioSoloAbort] session=" << sessionId << " entrant=" << soloMarioDeparture->iMarioEntrantPlayerId
			<< " reason=" << m_strStatus << '\n';
		Clear_KoukuSaydonPatternAudition(false, m_strStatus);
	}
	else if (koukuOwnerLeft)
		Clear_KoukuSaydonPatternAudition();

	// 퇴장 session에 연결된 저작 응답·업로드·입력 순번·도착 실행을 지운다. player binding이 없어도 이 정리는 먼저 수행한다.
	std::erase_if(m_PendingKoukuSaydonPatternAuditionLifecycle,
		[sessionId](const TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& edge)
		{
			return edge.iSessionId == sessionId;
		});
	m_KoukuSaydonPatternAuditionReceiptBySessionId.erase(sessionId);
	m_KoukuDraftUploads.erase(sessionId);
	m_ValtanAuditionSequenceBySessionId.erase(sessionId);
	m_KillGateBossesRequestSequences.erase(sessionId);
	m_CooldownModeRequestSequences.erase(sessionId);
	m_WorldPlaybackRequestSequences.erase(sessionId);
	m_RoomPlayerArrivalRuns.erase(sessionId);
	m_ValtanPatternIdAuditionSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowStartSequenceBySessionId.erase(sessionId);
	m_ValtanPatternFlowControlSequenceBySessionId.erase(sessionId);
	m_PendingPartyTransferResults.erase(sessionId);

	// 등록만 된 session은 약한 참조를 지우고 끝낸다. lookup만 남은 경우도 그 mapping과 등록을 정리하고 반환한다.
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
	{
		m_Sessions.erase(sessionId);
		return;
	}
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter == m_Players.end())
	{
		m_PlayerIdBySessionId.erase(sessionPlayerIter);
		m_Sessions.erase(sessionId);
		return;
	}

	// 실제 player가 있으면 전투 object·미로·trigger 상태와 연결의 player ID를 해제한 뒤 entity/session lookup을 지운다.
	const NET_ENTITY_ID netEntityId = playerIter->second.iNetEntityId;
	m_CombatObjectRuntime.Cancel_Source(netEntityId);
	if (m_KoukuCardMaze.Remove_Player(playerId))
		Reset_CardMaze();
	m_ServerTriggerSystem.Remove_Player(playerId);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
	m_PlayerIdByEntityId.erase(netEntityId);
	m_PlayerIdBySessionId.erase(sessionPlayerIter);

	// 받은 초대와 보낸 초대, 레이드 투표·파티 소속을 정리한 다음 player/session을 제거한다.
	m_PendingPartyInviteByTargetPlayerId.erase(playerId);
	std::erase_if(m_PendingPartyInviteByTargetPlayerId,
		[playerId](const auto& invite) { return invite.second == playerId; });
	Cancel_RaidEntryProposalsInvolving(playerId);
	Remove_FromParty(playerId);
	m_Players.erase(playerIter);
	m_Sessions.erase(sessionId);
	if (publishDeparture)
		Broadcast_Despawned(netEntityId, reason);

	std::cout << "Player left. World=" << static_cast<unsigned>(m_eWorldId)
		<< ", SessionId=" << sessionId
		<< ", RoomPlayers=" << m_Players.size() << '\n';

	// 실제 player 제거를 마친 뒤 빈 아레나의 재실행 상태를 초기화한다. Leave는 실패를 로그로 남기며 퇴장을 되돌리지 않는다.
	// 하위 초기화 실패는 Mark_RuntimeFailure를 통해 룸을 실패 상태로 내릴 수 있다.
	if (!Reset_ReplayableArenaWhenEmpty())
	{
		std::cerr << "Replayable arena reset failed. World="
			<< static_cast<unsigned>(m_eWorldId) << ", Status="
			<< m_strStatus << '\n';
	}
	if (!Reset_ValtanArenaWhenEmpty())
	{
		std::cerr << "Valtan arena reset failed: "
			<< m_strStatus << '\n';
	}
}

// 이동/skill 등 명령 handler가 session→player 연결 불일치를 발견하면 공통 종료 이유와 packet/검증 이름을 남긴다.
// 약한 참조를 lock할 수 없는 session은 무시한다. 조회/종료 요청만 하며 player lookup 정리는 후속 Leave가 맡는다.
void LostArk::Server::CGameRoom::Close_SessionForBindingFailure(
	const SESSION_ID sessionId,
	const std::string_view packetName,
	const std::string_view validation)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	session->Request_Close(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
		WSAENOTCONN,
		"packet=" + std::string{ packetName } + " validation=" +
			std::string{ validation });
}
