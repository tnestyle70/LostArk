#include "ClientReplication.h"

#include "ActionPresentationTimeline.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CombatHUDViewModel.h"
#include "Effect_PresentationService.h"
#include "EstherActionSoundCueDocument.h"
#include "GameInstance.h"
#include "MapNavigationContract.h"
#include "Model.h"
#include "NetworkManager.h"
#include "MonsterPresentationAssetService.h"
#include "Npc.h"
#include "NpcPlacementPresentationService.h"
#include "NpcPresentationAssetService.h"
#include "PlayableCharacterAssetService.h"
#include "Transform.h"
#include "Valtan.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPatternFlowService.h"
#include "ValtanPresentationAssetService.h"
#include "DeployPropRuntime.h"

#include <algorithm>
#include <cmath>
#include <span>

namespace
{
	using Client::CValtan;

	bool_t Can_ReloadValtanPresentationWithoutResettingLiveSound(
		std::string& strOutStatus)
	{
		Client::CValtanPatternAuditionService& Audition =
			Client::CValtanPatternAuditionService::Get();
		Client::CValtanPatternFlowService& Flow =
			Client::CValtanPatternFlowService::Get();
		Audition.Update();
		Flow.Update();
		if (Audition.Has_PatternSoundMutationBarrier() ||
			Flow.Has_PatternSoundMutationBarrier())
		{
			strOutStatus =
				"Authoritative Valtan presentation reload is blocked while a Pattern/Restart/Next/Flow occurrence owns the pinned Pattern Sound receipt; cue attempt state was preserved.";
			return false;
		}
		strOutStatus.clear();
		return true;
	}

	Client::NET_PLAYER_RECORD Make_Record(
		const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned)
	{
		Client::NET_PLAYER_RECORD record{};

		record.iPlayerId = spawned.iPlayerId;

		record.iNetEntityId = spawned.iNetEntityId;

		record.eCharacterClass = spawned.eCharacterClass;

		record.strNickName = spawned.strNickName;

		record.fPositionX = spawned.fPositionX;
		record.fPositionY = spawned.fPositionY;
		record.fPositionZ = spawned.fPositionZ;

		record.fYawDegrees = spawned.fYawDegrees;

		return record;
	}
	// TCP ?ъ쟾?≪씠??以묐났 Event媛 媛숈? Spawn???ㅼ떆 留뚮뱾吏 ?딄쾶 ?섍퀬,
	// 媛숈? NetEntityId???ㅻⅨ ?댁슜???ㅻ㈃ Protocol 異⑸룎濡??먮떒?쒕떎.
	bool Is_Same_Record(
		const Client::NET_PLAYER_RECORD& left,
		const Client::NET_PLAYER_RECORD& right)
	{
		return
			left.iPlayerId == right.iPlayerId &&
			left.iNetEntityId == right.iNetEntityId &&
			left.eCharacterClass == right.eCharacterClass &&
			left.strNickName == right.strNickName &&
			left.fPositionX == right.fPositionX &&
			left.fPositionY == right.fPositionY &&
			left.fPositionZ == right.fPositionZ &&
			left.fYawDegrees == right.fYawDegrees;
	}

	constexpr const char* VALTAN_LEFT_HAND_BONE = "bip001-l-hand";

	bool Is_FiniteMatrix(const float4x4_t& matrix)
	{
		for (std::size_t row = 0u; row < 4u; ++row)
		{
			for (std::size_t column = 0u; column < 4u; ++column)
			{
				if (!std::isfinite(matrix.m[row][column]))
					return false;
			}
		}
		return true;
	}

	CValtan::PATTERN_TARGET_SNAPSHOT_POSE Resolve_ValtanPatternTargetSnapshotPose(
		const std::span<const LostArk::Shared::PLAYER_SNAPSHOT> players,
		const LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId)
	{
		CValtan::PATTERN_TARGET_SNAPSHOT_POSE pose;
		pose.iNetEntityId = iTargetNetEntityId;
		if (LostArk::Shared::INVALID_NET_ENTITY_ID == iTargetNetEntityId)
			return pose;

		bool_t bFound = false;
		for (const LostArk::Shared::PLAYER_SNAPSHOT& player : players)
		{
			if (player.iNetEntityId != iTargetNetEntityId)
				continue;
			/* Duplicate identity is not a pose authority.  Keep the target ID for
			   diagnostics but force the cue occurrence down its isolated path. */
			if (bFound)
			{
				pose.bHasFinitePose = false;
				return pose;
			}
			bFound = true;
			if (!std::isfinite(player.fPositionX) ||
				!std::isfinite(player.fPositionY) ||
				!std::isfinite(player.fPositionZ) ||
				!std::isfinite(player.fYawDegrees))
			{
				continue;
			}
			pose.vPosition = float3_t(
				player.fPositionX, player.fPositionY, player.fPositionZ);
			pose.fYawDegrees = player.fYawDegrees;
			pose.bHasFinitePose = true;
		}
		return pose;
	}
}

bool Client::CClientReplication::Initialize(const DESC& desc)
{
	//Layer ?뺣낫媛 ?좏븳?쒖? 寃?ы븯怨? ?ㅼ젙????ν븳??
	//?꾩옱 network ?곌껐 ?곹깭??湲곗뼲???먯뼱, ?댄썑 ?곌껐???딄꼈?붿? 媛먯??????덇쾶 ?쒕떎.
	if (nullptr == desc.pDevice ||
		nullptr == desc.pContext ||
		desc.strMapAreaId.empty() ||
		desc.strPlayerLayerTag.empty() ||
		desc.strWorldEntityLayerTag.empty() ||
		((nullptr == desc.pDeployPropRuntime) !=
			(nullptr == desc.pWorldDestructionProjection)) ||
		(nullptr != desc.pWorldDestructionProjection &&
			!desc.pWorldDestructionProjection->Is_Ready()) ||
		!CCombatHUDViewModel::Get().Initialize_Definitions())
		return false;

	MAP_NAVIGATION_CONTRACT navigationContract{};
	std::string navigationStatus;
	if (!CMapNavigationContract::Resolve_Area(
			desc.strMapAreaId, navigationContract, navigationStatus) ||
		!navigationContract.runtimeGridAvailable ||
		navigationContract.prototypeTag.empty())
	{
		OutputDebugStringA((
			"Client replication navigation unavailable: " +
			navigationStatus + "\n").c_str());
		return false;
	}

	m_Desc = desc;
	m_strLocalPlayerNavigationPrototypeTag =
		std::move(navigationContract.prototypeTag);
	m_isInitialized = true;
	m_wasConnected =
		CNetworkManager::Get().Is_Connected();
	m_hasPendingConnectionLoss = !m_wasConnected;
	m_hasFatalWorldDestructionFailure = false;
	m_WorldDestructionProjectionRuntime.Reset();
	Clear_DeferredLocalCharacterClassReplacement();
	m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;

	return true;
}

bool Client::CClientReplication::Update()
{
	if (!m_isInitialized)
		return false;
	//network manager媛 留뚮뱾?대넃? replication event瑜??ㅼ젣 engine 蹂寃쎌쑝濡??곸슜?쒕떎.
	//baren main thread?먯꽌 留ㅽ봽?덉엫 ?몄텧?쒕떎.
	//珥덇린???щ? 寃??-> ?꾩옱 network ?곌껐 ?곹깭 ?뺤씤
	CNetworkManager& networkManager =
		CNetworkManager::Get();

	const bool isConnected =
		networkManager.Is_Connected();
	//?곌껐???딆뼱吏?寃쎌슦 : ?댁쟾 ?꾨젅?꾩뿉???곌껐???덉뿀?붽?? -> 洹몃젃?ㅻ㈃ reset world()
	//?⑥븘 ?덈뒗 event queue 鍮꾩슦湲?-> ?댁쟾 ?쒕쾭??spawn???ъ젒?????곸슜?섏? ?딄쾶 ??
	if (!isConnected)
	{
		if (m_wasConnected)
		{
			Reset_World();
			m_hasPendingConnectionLoss = true;
		}

		CLIENT_REPLICATION_EVENT ignored{};
		while (networkManager.Try_Consume_ReplicationEvent(ignored))
		{

		}
		m_wasConnected = false;
		return true;
	}

	m_wasConnected = true;
	bool allSucceeded = true;

	CLIENT_REPLICATION_EVENT event{};

	while (networkManager.Try_Consume_ReplicationEvent(event))
	{
		switch (event.eType)
		{
		case CLIENT_REPLICATION_EVENT_TYPE::PLAYER_SPAWNED:
			allSucceeded =
				Apply_Spawn(event.PlayerSpawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_SPAWNED:
			allSucceeded = Apply_WorldEntitySpawn(
				event.WorldEntitySpawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_SPAWNED:
			allSucceeded = Apply_CombatObjectSpawn(
				event.CombatObjectSpawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_PRESENTATION:
			allSucceeded = Apply_CombatObjectPresentationEvent(
				event.CombatObjectPresentation) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_DESPAWNED:
			allSucceeded =
				Apply_WorldEntityDespawn(event.WorldEntityDespawned) &&
				allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::COMBAT_OBJECT_DESPAWNED:
			allSucceeded = Apply_CombatObjectDespawn(
				event.CombatObjectDespawned) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PLAYER_DESPAWNED:
			allSucceeded =
				Apply_Despawn(event.PlayerDespawned) &&
				allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_SNAPSHOT:
			allSucceeded = Apply_WorldSnapshot(event.WorldSnapshot) &&
				allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_DESTRUCTION_FULL_SYNC:
			allSucceeded = Apply_WorldDestructionFullSync(
				event.WorldDestructionFullSync) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_DESTRUCTION_DELTA:
			allSucceeded = Apply_WorldDestructionDelta(
				event.WorldDestructionDelta) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::ENCOUNTER_PROP_SYNC:
			allSucceeded = Apply_EncounterPropSync(
				event.EncounterPropSync) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::INVENTORY_SNAPSHOT:
			allSucceeded = Apply_InventorySnapshot(
				event.InventorySnapshot) && allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PARTY_INVITE_RECEIVED:
			Apply_PartyInviteReceived(event.PartyInviteReceived);
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PARTY_ROSTER:
			Apply_PartyRoster(event.PartyRoster);
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::PARTY_TRANSFER_RESULT:
			m_PendingPartyTransferResult = event.PartyTransferResult;
			m_hasPendingPartyTransferResult = true;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::CHAT_RECEIVED:
			Apply_ChatReceived(event.ChatReceived);
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_SEQUENCE_PLAY:
			/* Queued rather than played here: the level owns the sequence
			   player and drains this on its own update. */
			m_PendingWorldSequencePlays.push_back(
				event.WorldSequencePlay.strSequenceInstanceId);
			break;
		}
	}

	Update_DeathPresentations();
	Update_PlayerAttachmentPresentations();
	return allSucceeded;
}

bool Client::CClientReplication::Apply_WorldDestructionFullSync(
	const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& fullSync)
{
	if (nullptr == m_Desc.pWorldDestructionProjection ||
		nullptr == m_Desc.pDeployPropRuntime)
	{
		m_strPendingPresentationFailure =
			"World destruction projection is not configured for this level.";
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	std::string status;
	if (!m_WorldDestructionProjectionRuntime.Apply_Full(
		*m_Desc.pWorldDestructionProjection, fullSync,
		*m_Desc.pDeployPropRuntime, status))
	{
		m_strPendingPresentationFailure = std::move(status);
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	m_WorldDestructionLiveEvents.clear();
	m_WorldDestructionDiagnostics = fullSync.Diagnostics;
	++m_iWorldDestructionPresentationGeneration;
	if (0u == m_iWorldDestructionPresentationGeneration)
		++m_iWorldDestructionPresentationGeneration;
	return true;
}

bool Client::CClientReplication::Apply_EncounterPropSync(
	const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& sync)
{
	/* Replace-in-full. A slot only ever holds its current state, so a late
	   joiner and a player who watched the whole cycle read the same thing, and
	   an out-of-order epoch cannot resurrect a retired pillar. */
	if (sync.iEncounterEpoch < m_EncounterPropState.iEncounterEpoch ||
		(sync.iEncounterEpoch == m_EncounterPropState.iEncounterEpoch &&
		 sync.iServerTick < m_EncounterPropState.iServerTick))
	{
		return true;
	}
	m_EncounterPropState = sync;
	return true;
}

bool Client::CClientReplication::Apply_InventorySnapshot(
	const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& snapshot)
{
	/* Replace-in-full: the Server always answers with the whole current
	   inventory, so the previous list is simply discarded. */
	m_InventoryState = snapshot;
	/* CCombatHUDViewModel is the level-agnostic singleton the F1 debug panel
	   already reads through (CMainApp owns no per-level CClientReplication of
	   its own), the same role it plays for Apply_LocalPlayer above. */
	CCombatHUDViewModel::Get().Apply_Inventory(snapshot);
	return true;
}

void Client::CClientReplication::Apply_PartyInviteReceived(
	const LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& received)
{
	// A newer invite silently replaces an unconsumed older one, mirroring
	// the Server's own one-pending-invite-per-target replacement rule.
	m_PendingPartyInvite = received;
	m_hasPendingPartyInvite = true;
}

bool Client::CClientReplication::Try_Consume_PartyInviteReceived(
	LostArk::Shared::S2C_PARTY_INVITE_RECEIVED& outInvite)
{
	if (!m_hasPendingPartyInvite)
		return false;
	outInvite = m_PendingPartyInvite;
	m_hasPendingPartyInvite = false;
	return true;
}

void Client::CClientReplication::Apply_PartyRoster(
	const LostArk::Shared::S2C_PARTY_ROSTER& roster)
{
	// Replace-in-full, same shape as Apply_InventorySnapshot/
	// Apply_EncounterPropSync -- the Server always sends the whole current
	// membership, never a delta.
	m_PartyRoster = roster;
}

bool Client::CClientReplication::Try_Consume_PartyTransferResult(
	LostArk::Shared::S2C_PARTY_TRANSFER_RESULT& outResult)
{
	if (!m_hasPendingPartyTransferResult)
		return false;
	outResult = m_PendingPartyTransferResult;
	m_hasPendingPartyTransferResult = false;
	return true;
}

void Client::CClientReplication::Apply_ChatReceived(
	const LostArk::Shared::S2C_CHAT& received)
{
	m_ChatBubblesByNetEntityId[received.iFromNetEntityId] = CHAT_BUBBLE_ENTRY{
		received.strText,
		std::chrono::steady_clock::now() + CHAT_BUBBLE_DURATION };
}

bool Client::CClientReplication::Try_Get_ActiveChatBubble(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	std::string& outText) const
{
	const auto found = m_ChatBubblesByNetEntityId.find(netEntityId);
	if (m_ChatBubblesByNetEntityId.end() == found ||
		std::chrono::steady_clock::now() >= found->second.ExpireAt)
	{
		return false;
	}
	outText = found->second.strText;
	return true;
}

bool Client::CClientReplication::Apply_WorldDestructionDelta(
	const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& delta)
{
	if (nullptr == m_Desc.pWorldDestructionProjection ||
		nullptr == m_Desc.pDeployPropRuntime)
	{
		m_strPendingPresentationFailure =
			"World destruction projection is not configured for this level.";
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	std::string status;
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents;
	if (!m_WorldDestructionProjectionRuntime.Apply_Delta(
		*m_Desc.pWorldDestructionProjection, delta,
		*m_Desc.pDeployPropRuntime, status, &liveEvents))
	{
		m_strPendingPresentationFailure = std::move(status);
		m_hasFatalWorldDestructionFailure = true;
		return false;
	}
	constexpr size_t MAX_PENDING_PRESENTATION_EVENTS =
		LostArk::Shared::MAX_WORLD_DESTRUCTION_EVENTS * 2u;
	for (LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& event : liveEvents)
	{
		if (m_WorldDestructionLiveEvents.size() >=
			MAX_PENDING_PRESENTATION_EVENTS)
		{
			m_strPendingPresentationFailure =
				"World destruction debris cue queue reached its presentation-only limit.";
			break;
		}
		m_WorldDestructionLiveEvents.push_back(std::move(event));
	}
	m_WorldDestructionDiagnostics = delta.Diagnostics;
	return true;
}

bool Client::CClientReplication::Has_PendingConnectionLoss() const
{
	return m_hasPendingConnectionLoss;
}

void Client::CClientReplication::Acknowledge_ConnectionLoss()
{
	m_hasPendingConnectionLoss = false;
}

void Client::CClientReplication::Reset()
{
	Reset_World();
	m_wasConnected = false;
	m_hasPendingConnectionLoss = false;
}

bool Client::CClientReplication::Has_WorldEntity(
	const std::string_view archetypeId) const
{
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		const bool_t hasLivePresentation =
			(LostArk::Shared::WORLD_ENTITY_KIND::NPC == presentation.eKind &&
				!presentation.pNpc.expired()) ||
			(LostArk::Shared::WORLD_ENTITY_KIND::MONSTER == presentation.eKind &&
				!presentation.pNpc.expired()) ||
			(LostArk::Shared::WORLD_ENTITY_KIND::BOSS == presentation.eKind &&
				!presentation.pValtan.expired());
		if (presentation.strArchetypeId == archetypeId &&
			hasLivePresentation)
		{
			return true;
		}
	}
	return false;
}

bool_t Client::CClientReplication::Resolve_PrimaryValtan(
	const std::string_view strArchetypeId,
	const LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId,
	std::shared_ptr<CValtan>& pOutValtan,
	std::string& strOutStatus) const
{
	pOutValtan.reset();
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (LostArk::Shared::WORLD_ENTITY_KIND::BOSS != presentation.eKind ||
			presentation.strArchetypeId != strArchetypeId ||
			presentation.iOwnerBossNetEntityId != iOwnerBossNetEntityId)
		{
			continue;
		}

		const std::shared_ptr<CValtan> Candidate = presentation.pValtan.lock();
		if (nullptr == Candidate)
		{
			strOutStatus =
				"The primary replicated Valtan registry entry has no live presentation consumer.";
			return false;
		}
		if (nullptr != pOutValtan)
		{
			pOutValtan.reset();
			strOutStatus =
				"Multiple primary replicated Valtan presentation consumers were found.";
			return false;
		}
		pOutValtan = Candidate;
	}

	strOutStatus = nullptr == pOutValtan ?
		"No active primary replicated Valtan; the next admitted spawn will load the saved presentation source." :
		"Resolved the authoritative primary replicated Valtan presentation consumer.";
	return true;
}

Client::CClientReplication::WORLD_ENTITY_PRESENTATION*
Client::CClientReplication::Find_ValtanPresentation(
	const std::shared_ptr<CValtan>& pValtan)
{
	if (nullptr == pValtan)
		return nullptr;
	for (auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (presentation.pValtan.lock().get() == pValtan.get())
			return &presentation;
	}
	return nullptr;
}

bool_t Client::CClientReplication::Ensure_ValtanPresentationRevision(
	WORLD_ENTITY_PRESENTATION& Presentation,
	const std::shared_ptr<CValtan>& pValtan,
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	const bool_t bIsPrimary,
	std::string& strOutStatus)
{
	using LostArk::Shared::Format_GameplayDataRevision;
	if (nullptr == pValtan || !ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Valtan snapshot has no live presentation or exact occurrence revision.";
		return false;
	}

	Presentation.PinnedDefinitionRevision = ExpectedRevision;
	if (!Presentation.bPresentationIsolated &&
		Presentation.AdmittedPresentationRevision == ExpectedRevision)
	{
		strOutStatus.clear();
		return true;
	}
	if (Presentation.bPresentationIsolated &&
		Presentation.RejectedPresentationRevision == ExpectedRevision)
	{
		strOutStatus =
			"Valtan presentation remains isolated for previously rejected occurrence revision " +
			Format_GameplayDataRevision(ExpectedRevision) + ".";
		return false;
	}

	VALTAN_PRESENTATION_GENERATION_RECEIPT Receipt;
	std::string ReceiptStatus;
	std::string ReloadStatus;
	const bool_t bHasReceipt =
		CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			ExpectedRevision, Receipt, ReceiptStatus);
	const bool_t bEntryReceiptRecoveryPending = !bHasReceipt &&
		CNetworkManager::Get().Get_GameplayRevisionState().
			hasPendingEntryPresentationBaselineRecovery;
	const bool_t bReloaded = bHasReceipt &&
		pValtan->Reload_PatternPresentationAuthoring(
			ExpectedRevision, Receipt, ReloadStatus);
	if (!bReloaded)
	{
		Presentation.AdmittedPresentationRevision = {};
		Presentation.RejectedPresentationRevision =
			bEntryReceiptRecoveryPending ?
			LostArk::Shared::GameplayDataRevision{} : ExpectedRevision;
		Presentation.bPresentationIsolated = true;
		CEffectPresentationService::Stop_BossOwner(pValtan);
		strOutStatus =
			"Valtan occurrence presentation revision " +
			Format_GameplayDataRevision(ExpectedRevision) +
			(bEntryReceiptRecoveryPending ?
				" is waiting for the saved entry presentation JSON after a canonical transaction: " :
				" was isolated after one exact reload attempt: ") +
			(bHasReceipt ? ReloadStatus : ReceiptStatus);
		m_strPendingPresentationFailure = strOutStatus;
		if (bIsPrimary && !bEntryReceiptRecoveryPending)
		{
			m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
			m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		}
		return false;
	}

	Presentation.AdmittedPresentationRevision = ExpectedRevision;
	Presentation.RejectedPresentationRevision = {};
	Presentation.bPresentationIsolated = false;
	strOutStatus = ReloadStatus;
	if (bIsPrimary)
	{
		m_PrimaryValtanJoinedPresentationFreshness.Admit(
			ExpectedRevision, ReloadStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Admit(
			ExpectedRevision, ReloadStatus);
	}
	return true;
}

bool_t Client::CClientReplication::Reload_PrimaryValtanPresentationAuthoring(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	static constexpr std::string_view PRIMARY_ARCHETYPE_ID = "BOSS_VALTAN";
	static constexpr LostArk::Shared::NET_ENTITY_ID PRIMARY_OWNER_ID =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	if (!Can_ReloadValtanPresentationWithoutResettingLiveSound(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Authoritative primary Valtan presentation reload requires one exact Server-active revision.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		return false;
	}
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT PresentationReceipt;
	std::string ReceiptStatus;
	if (!CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			ExpectedRevision, PresentationReceipt, ReceiptStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan has no exact presentation generation receipt: " +
			ReceiptStatus;
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	std::shared_ptr<CValtan> PrimaryValtan;
	std::string ResolveStatus;
	if (!Resolve_PrimaryValtan(
		PRIMARY_ARCHETYPE_ID, PRIMARY_OWNER_ID, PrimaryValtan, ResolveStatus))
	{
		m_PrimaryValtanJoinedPresentationFreshness.Reject(ResolveStatus);
		m_strPendingPresentationFailure = ResolveStatus;
		strOutStatus = ResolveStatus;
		return false;
	}
	if (nullptr == PrimaryValtan)
	{
		std::string FreshnessStatus;
		if (!m_PrimaryValtanJoinedPresentationFreshness.Can_Play(
				ExpectedRevision, FreshnessStatus))
		{
			strOutStatus = ResolveStatus + " " + FreshnessStatus +
				" The next primary spawn must admit a successful joined reload.";
			return false;
		}
		strOutStatus = ResolveStatus;
		return true;
	}
	WORLD_ENTITY_PRESENTATION* const Presentation =
		Find_ValtanPresentation(PrimaryValtan);
	if (nullptr == Presentation ||
		Presentation->PinnedDefinitionRevision != ExpectedRevision)
	{
		strOutStatus =
			"Authoritative primary Valtan reload revision does not match its Server-pinned occurrence.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}

	std::string ReloadStatus;
	if (!PrimaryValtan->Reload_PatternPresentationAuthoring(
			ExpectedRevision, PresentationReceipt, ReloadStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan joined presentation reload rejected: " +
			ReloadStatus;
		Presentation->AdmittedPresentationRevision = {};
		Presentation->RejectedPresentationRevision = ExpectedRevision;
		Presentation->bPresentationIsolated = true;
		CEffectPresentationService::Stop_BossOwner(PrimaryValtan);
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	Presentation->AdmittedPresentationRevision = ExpectedRevision;
	Presentation->RejectedPresentationRevision = {};
	Presentation->bPresentationIsolated = false;
	m_PrimaryValtanJoinedPresentationFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	/* Reload_PatternPresentationAuthoring stages animation, Effect, Pattern
	   Sound, combat-object Sound and shake as one joined generation. */
	m_PrimaryValtanCombatObjectSoundFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	strOutStatus =
		"Authoritative primary Valtan joined presentation reloaded. " +
		ReloadStatus;
	return true;
}

bool_t Client::CClientReplication::Reload_PrimaryValtanCombatObjectSoundCues(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	static constexpr std::string_view PRIMARY_ARCHETYPE_ID = "BOSS_VALTAN";
	static constexpr LostArk::Shared::NET_ENTITY_ID PRIMARY_OWNER_ID =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	if (!Can_ReloadValtanPresentationWithoutResettingLiveSound(strOutStatus))
		return false;
	if (!ExpectedRevision.Is_Valid())
	{
		strOutStatus =
			"Authoritative primary Valtan combat-object Sound reload requires one exact Server-active revision.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		return false;
	}
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT PresentationReceipt;
	std::string ReceiptStatus;
	if (!CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			ExpectedRevision, PresentationReceipt, ReceiptStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan has no exact presentation generation receipt: " +
			ReceiptStatus;
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	std::shared_ptr<CValtan> PrimaryValtan;
	std::string ResolveStatus;
	if (!Resolve_PrimaryValtan(
		PRIMARY_ARCHETYPE_ID, PRIMARY_OWNER_ID, PrimaryValtan, ResolveStatus))
	{
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(ResolveStatus);
		m_strPendingPresentationFailure = ResolveStatus;
		strOutStatus = ResolveStatus;
		return false;
	}
	if (nullptr == PrimaryValtan)
	{
		std::string FreshnessStatus;
		if (!m_PrimaryValtanCombatObjectSoundFreshness.Can_Play(
				ExpectedRevision, FreshnessStatus))
		{
			strOutStatus = ResolveStatus + " " + FreshnessStatus +
				" The next primary spawn must admit a successful combat-object Sound reload.";
			return false;
		}
		strOutStatus = ResolveStatus;
		return true;
	}
	WORLD_ENTITY_PRESENTATION* const Presentation =
		Find_ValtanPresentation(PrimaryValtan);
	if (nullptr == Presentation ||
		Presentation->PinnedDefinitionRevision != ExpectedRevision)
	{
		strOutStatus =
			"Authoritative primary Valtan combat-object Sound reload revision does not match its Server-pinned occurrence.";
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}

	std::string ReloadStatus;
	if (!PrimaryValtan->Reload_PatternPresentationAuthoring(
			ExpectedRevision, PresentationReceipt, ReloadStatus))
	{
		strOutStatus =
			"Authoritative primary Valtan combat-object Sound reload rejected: " +
			ReloadStatus;
		Presentation->AdmittedPresentationRevision = {};
		Presentation->RejectedPresentationRevision = ExpectedRevision;
		Presentation->bPresentationIsolated = true;
		CEffectPresentationService::Stop_BossOwner(PrimaryValtan);
		m_PrimaryValtanCombatObjectSoundFreshness.Reject(strOutStatus);
		m_PrimaryValtanJoinedPresentationFreshness.Reject(strOutStatus);
		m_strPendingPresentationFailure = strOutStatus;
		return false;
	}
	Presentation->AdmittedPresentationRevision = ExpectedRevision;
	Presentation->RejectedPresentationRevision = {};
	Presentation->bPresentationIsolated = false;
	/* CValtan's compatibility entry delegates to the same joined transaction. */
	m_PrimaryValtanJoinedPresentationFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	m_PrimaryValtanCombatObjectSoundFreshness.Admit(
		ExpectedRevision, ReloadStatus);
	strOutStatus =
		"Authoritative primary Valtan combat-object Sound reloaded. " +
		ReloadStatus;
	return true;
}

bool_t Client::CClientReplication::Can_Play_PrimaryValtanPresentation(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus) const
{
	if (!m_PrimaryValtanJoinedPresentationFreshness.Can_Play(
			ExpectedRevision, strOutStatus))
		return false;
	if (!m_PrimaryValtanCombatObjectSoundFreshness.Can_Play(
			ExpectedRevision, strOutStatus))
		return false;
	strOutStatus.clear();
	return true;
}

bool_t Client::CClientReplication::
	Get_PrimaryValtanPatternSoundSourceReceipt(
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt,
		std::string& strOutStatus) const
{
	static constexpr std::string_view PRIMARY_ARCHETYPE_ID = "BOSS_VALTAN";
	static constexpr LostArk::Shared::NET_ENTITY_ID PRIMARY_OWNER_ID =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::shared_ptr<CValtan> PrimaryValtan;
	if (!Resolve_PrimaryValtan(
			PRIMARY_ARCHETYPE_ID, PRIMARY_OWNER_ID,
			PrimaryValtan, strOutStatus) || nullptr == PrimaryValtan)
	{
		if (strOutStatus.empty())
		{
			strOutStatus =
				"Pattern playback requires one active primary replicated Valtan presentation consumer.";
		}
		return false;
	}
	const VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& Receipt =
		PrimaryValtan->Get_PatternSoundSourceReceipt();
	if (!Receipt.Is_Valid())
	{
		strOutStatus =
			"The primary replicated Valtan has no exact admitted Pattern Sound source receipt.";
		return false;
	}
	OutReceipt = Receipt;
	strOutStatus =
		"Resolved the primary replicated Valtan's exact Pattern Sound source receipt.";
	return true;
}

bool Client::CClientReplication::Try_Consume_PresentationFailure(
	std::string& outStatus)
{
	if (m_strPendingPresentationFailure.empty())
		return false;
	outStatus = std::move(m_strPendingPresentationFailure);
	m_strPendingPresentationFailure.clear();
	return true;
}

bool Client::CClientReplication::Try_Consume_WorldDestructionLiveEvent(
	LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE& outEvent)
{
	if (m_WorldDestructionLiveEvents.empty())
		return false;
	outEvent = std::move(m_WorldDestructionLiveEvents.front());
	m_WorldDestructionLiveEvents.pop_front();
	return true;
}

std::shared_ptr<CCharacter> Client::CClientReplication::Get_LocalCharacter() const
{
	return m_Registry.Resolve(m_LocalCharacterHandle);
}

bool_t Client::CClientReplication::Try_Get_DeferredLocalCharacterClassReplacement(
	DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_VIEW& OutView) const
{
	OutView = {};
	if (!m_DeferredLocalCharacterClassReplacement.isPending)
		return false;

	OutView.iGeneration =
		m_DeferredLocalCharacterClassReplacement.iGeneration;
	OutView.iNetEntityId =
		m_DeferredLocalCharacterClassReplacement.Snapshot.iNetEntityId;
	OutView.eCharacterClass =
		m_DeferredLocalCharacterClassReplacement.Snapshot.eCharacterClass;
	OutView.iServerTick =
		m_DeferredLocalCharacterClassReplacement.iServerTick;
	return true;
}

Client::DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT
Client::CClientReplication::Commit_DeferredLocalCharacterClassReplacement()
{
	using namespace LostArk::Shared;
	if (!m_DeferredLocalCharacterClassReplacement.isPending)
	{
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::NO_PENDING;
	}

	const DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT Pending =
		m_DeferredLocalCharacterClassReplacement;
	const NET_ENTITY_ID iLocalEntityId =
		CNetworkManager::Get().Get_LocalEntityId();
	const NET_PLAYER_RECORD* pCurrentRecord =
		m_Registry.Find_Record(Pending.Snapshot.iNetEntityId);
	if (INVALID_NET_ENTITY_ID == iLocalEntityId ||
		Pending.Snapshot.iNetEntityId != iLocalEntityId ||
		nullptr == pCurrentRecord)
	{
		Clear_DeferredLocalCharacterClassReplacement();
		m_strPendingPresentationFailure =
			"Deferred local class replacement lost its stable player identity.";
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::FATAL_FAILURE;
	}
	if (pCurrentRecord->eCharacterClass == Pending.Snapshot.eCharacterClass)
	{
		Clear_DeferredLocalCharacterClassReplacement();
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::COMMITTED;
	}

	const CHARACTER_REPLACE_RESULT ReplaceResult =
		Replace_CharacterClass(Pending.Snapshot);
	if (CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE == ReplaceResult)
	{
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::RECOVERED_FAILURE;
	}
	if (CHARACTER_REPLACE_RESULT::FATAL_FAILURE == ReplaceResult)
	{
		Clear_DeferredLocalCharacterClassReplacement();
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::FATAL_FAILURE;
	}

	const std::shared_ptr<CCharacter> pCharacter = Get_LocalCharacter();
	const float3_t Position(
		Pending.Snapshot.fPositionX,
		Pending.Snapshot.fPositionY,
		Pending.Snapshot.fPositionZ);
	const bool_t isMoving = Pending.Snapshot.eLocomotionState ==
		PLAYER_LOCOMOTION_STATE::MOVING;
	if (nullptr == pCharacter || !pCharacter->Apply_NetworkState(
			Position,
			Pending.Snapshot.fYawDegrees,
			isMoving,
			Pending.iServerTick) ||
		!pCharacter->Apply_NetworkAction(
			Pending.Snapshot.eAction,
			Pending.Snapshot.iSkillId,
			Pending.iServerTick,
			Pending.Snapshot.iActionStartTick,
			Pending.Snapshot.fYawDegrees,
			Pending.Snapshot.iComboStage,
			Pending.Snapshot.hasSkillTarget,
			float3_t(
				Pending.Snapshot.fSkillTargetX,
				Pending.Snapshot.fSkillTargetY,
				Pending.Snapshot.fSkillTargetZ)))
	{
		Clear_DeferredLocalCharacterClassReplacement();
		m_strPendingPresentationFailure =
			"Deferred local class replacement could not apply its latest snapshot.";
		return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::FATAL_FAILURE;
	}
	pCharacter->Apply_NetworkStance(Pending.Snapshot.eStance);
	CCombatHUDViewModel::Get().Apply_LocalPlayer(
		Pending.iServerTick,
		Pending.Snapshot.eCharacterClass,
		Pending.Snapshot);
	Clear_DeferredLocalCharacterClassReplacement();
	return DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT_RESULT::COMMITTED;
}

void Client::CClientReplication::Collect_PlayerViews(
	std::vector<REPLICATED_PLAYER_VIEW>& outPlayers) const
{
	const std::vector<LIVE_NET_PLAYER> livePlayers =
		m_Registry.Get_LivePlayers();
	const std::shared_ptr<CCharacter> localCharacter =
		m_Registry.Resolve(m_LocalCharacterHandle);

	outPlayers.resize(livePlayers.size());
	for (std::size_t index = 0u; index < livePlayers.size(); ++index)
	{
		const LIVE_NET_PLAYER& source = livePlayers[index];
		REPLICATED_PLAYER_VIEW& target = outPlayers[index];
		target.iPlayerId = source.Record.iPlayerId;
		target.iNetEntityId = source.Record.iNetEntityId;
		target.eCharacterClass = source.Record.eCharacterClass;
		target.strNickname = source.Record.strNickName;
		target.isLocal = nullptr != localCharacter &&
			source.pCharacter == localCharacter;
		target.pCharacter = source.pCharacter;
	}

	std::sort(
		outPlayers.begin(),
		outPlayers.end(),
		[](const REPLICATED_PLAYER_VIEW& left,
			const REPLICATED_PLAYER_VIEW& right)
		{
			return left.iNetEntityId < right.iNetEntityId;
		});
}

#ifdef _DEBUG
void Client::CClientReplication::Set_CombatColliderDebugVisible(
	const bool_t isVisible)
{
	m_isCombatColliderDebugVisible = isVisible;
	for (const std::shared_ptr<CCharacter>& character :
		m_Registry.Get_LiveObjects())
	{
		if (nullptr != character)
			character->Set_CombatColliderDebugVisible(isVisible);
	}
	for (auto& [netEntityId, presentation] : m_WorldEntities)
	{
		(void)netEntityId;
		if (std::shared_ptr<CNpc> npc = presentation.pNpc.lock())
			npc->Set_CombatColliderDebugVisible(isVisible);
		if (std::shared_ptr<CValtan> valtan = presentation.pValtan.lock())
			valtan->Set_CombatColliderDebugVisible(isVisible);
	}
}

void Client::CClientReplication::Set_SkillHitAreaDebugVisible(
	const bool_t isVisible)
{
	m_isSkillHitAreaDebugVisible = isVisible;
	for (const std::shared_ptr<CCharacter>& character :
		m_Registry.Get_LiveObjects())
	{
		if (nullptr != character)
			character->Set_SkillHitAreaDebugVisible(isVisible);
	}
	for (auto& [netEntityId, presentation] : m_WorldEntities)
	{
		(void)netEntityId;
		if (std::shared_ptr<CValtan> valtan = presentation.pValtan.lock())
			valtan->Set_PatternHitAreaDebugVisible(isVisible);
	}
}
#endif

bool Client::CClientReplication::Create_Character(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::string_view nickName,
	const float3_t& position,
	const f32_t yawDegrees,
	const bool_t isLocallyControlled,
	std::shared_ptr<CCharacter>& outCharacter)
{
	outCharacter.reset();
	if (FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
		m_Desc.pDevice,
		m_Desc.pContext,
		m_Desc.iPrototypeLevelIndex,
		characterClass)))
	{
		return false;
	}

	const CHARACTER_SPEC* spec =
		CCharacterCatalog::Find_Spec(characterClass);
	if (nullptr == spec)
		return false;

	CCharacter::CHARACTER_DESC desc{};
	desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
	desc.pSpec = spec;
	desc.pNavigationPrototypeTag =
		isLocallyControlled ?
			m_strLocalPlayerNavigationPrototypeTag.c_str() : nullptr;
	desc.fSpeedPerSec = 6.f;
	desc.fRotationPerSec = 180.f;
	desc.vPosition = position;
	desc.strNickName = nickName;
	desc.isLocallyControlled = isLocallyControlled;

	std::shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		m_Desc.iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Character"),
		m_Desc.iLayerLevelIndex,
		m_Desc.strPlayerLayerTag,
		&desc,
		&gameObject)))
	{
		return false;
	}

	const std::shared_ptr<CCharacter> character =
		std::dynamic_pointer_cast<CCharacter>(gameObject);
	if (nullptr == character || nullptr == character->Get_Transform())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			gameObject);
		return false;
	}

	character->Get_Transform()->Rotation(0.f, yawDegrees, 0.f);
#ifdef _DEBUG
	character->Set_CombatColliderDebugVisible(
		m_isCombatColliderDebugVisible);
	character->Set_SkillHitAreaDebugVisible(
		m_isSkillHitAreaDebugVisible);
#endif
	outCharacter = character;
	return true;
}

bool Client::CClientReplication::Apply_Spawn(
	const LostArk::Shared::S2C_PLAYER_SPAWNED& spawned)
{
	//?쒕쾭 player瑜?client character濡??앹꽦?쒕떎.
	//spawn message -> net player record 蹂??
	//?숈씪  entity id媛 ?대? ?덈뒗吏瑜?寃?ы븳??
	//character catalog?먯꽌 class spec 寃??->
	//character desc 援ъ꽦 -> local remote ?먯젙
	//add gameobject to layer -> character 罹먯뒪??諛?transform 寃??
	//yaw 諛섏쁺 -> registry ?깅줉 -> local?대㈃ localcharacterhandle ???

	const NET_PLAYER_RECORD stagedRecord =
		Make_Record(spawned);

	if (const NET_PLAYER_RECORD* existing =
		m_Registry.Find_Record(spawned.iNetEntityId))
	{
		//媛숈? event ?ъ쟾?≪? no-op, ?ㅻⅨ ?댁슜??媛숈? id?? protocol conflict?대떎.
		return Is_Same_Record(*existing, stagedRecord);
	}

	const bool_t isLocallyControlled =
		spawned.iPlayerId ==
		CNetworkManager::Get().Get_LocalPlayerId();
	std::shared_ptr<CCharacter> character;
	if (!Create_Character(
		spawned.eCharacterClass,
		spawned.strNickName,
		float3_t(
			spawned.fPositionX,
			spawned.fPositionY,
			spawned.fPositionZ),
		spawned.fYawDegrees,
		isLocallyControlled,
		character))
	{
		return false;
	}

	OBJECT_HANDLE handle{};

	if (!m_Registry.Register(
		stagedRecord,
		character,
		handle))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character);
		return false;
	}

	if (isLocallyControlled)
		m_LocalCharacterHandle = handle;

	return true;
}

bool Client::CClientReplication::Apply_Despawn(
	const LostArk::Shared::S2C_PLAYER_DESPAWNED& despawned)
{
	m_PlayerHealth.Erase(despawned.iNetEntityId);
	OBJECT_HANDLE handle{};

	if (!m_Registry.Find_Handle(
		despawned.iNetEntityId, handle))
	{
		//?대? ?쒓굅??以묐났 despawn? no-op?쇰줈 痍④툒
		return true;
	}

	const std::shared_ptr<CCharacter> character =
		m_Registry.Resolve(handle);
	//gameobject layer?먯꽌 ?쒓굅
	if (nullptr != character &&
		FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character)))
	{
		return false;
	}

	if (!m_Registry.Unregister(despawned.iNetEntityId))
		return false;
	m_PlayerAttachments.erase(despawned.iNetEntityId);

	if (m_LocalCharacterHandle.iSlotIndex == handle.iSlotIndex &&
		m_LocalCharacterHandle.iGeneration == handle.iGeneration)
	{
		m_LocalCharacterHandle = {};
		if (m_DeferredLocalCharacterClassReplacement.isPending &&
			m_DeferredLocalCharacterClassReplacement.Snapshot.iNetEntityId ==
				despawned.iNetEntityId)
		{
			Clear_DeferredLocalCharacterClassReplacement();
		}
	}

	return true;
}

bool Client::CClientReplication::Apply_WorldEntitySpawn(
	const LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED ownerIdentity{};
	const S2C_WORLD_ENTITY_SPAWNED* owner = nullptr;
	if (INVALID_NET_ENTITY_ID != spawned.iOwnerBossNetEntityId)
	{
		const auto foundOwner = m_WorldEntities.find(spawned.iOwnerBossNetEntityId);
		if (foundOwner == m_WorldEntities.end())
		{
			m_strPendingPresentationFailure = "Dependent boss arrived before its owner.";
			return false;
		}
		const std::shared_ptr<CValtan> owningBoss = foundOwner->second.pValtan.lock();
		if (nullptr == owningBoss || CValtan::DEAD == owningBoss->Get_State() ||
			foundOwner->second.bPresentationIsolated)
		{
			m_strPendingPresentationFailure =
				"Dependent boss has no admitted live owner presentation.";
			return false;
		}
		ownerIdentity.iNetEntityId = foundOwner->first;
		ownerIdentity.iOwnerBossNetEntityId = foundOwner->second.iOwnerBossNetEntityId;
		ownerIdentity.eKind = foundOwner->second.eKind;
		ownerIdentity.strEncounterId = foundOwner->second.strEncounterId;
		ownerIdentity.PinnedDefinitionRevision =
			foundOwner->second.PinnedDefinitionRevision;
		owner = &ownerIdentity;
	}
	if (!Is_Valid_WorldEntitySpawnOwner(spawned, owner) ||
		(("BOSS_VALTAN_GHOST" == spawned.strArchetypeId) !=
		 (INVALID_NET_ENTITY_ID != spawned.iOwnerBossNetEntityId)))
	{
		m_strPendingPresentationFailure = "Invalid dependent boss ownership graph.";
		return false;
	}
	const auto existing = m_WorldEntities.find(spawned.iNetEntityId);
	if (existing != m_WorldEntities.end())
	{
			const bool_t hasLivePresentation =
				(WORLD_ENTITY_KIND::NPC == existing->second.eKind &&
					!existing->second.pNpc.expired()) ||
				(WORLD_ENTITY_KIND::MONSTER == existing->second.eKind &&
					!existing->second.pNpc.expired()) ||
				(WORLD_ENTITY_KIND::BOSS == existing->second.eKind &&
				!existing->second.pValtan.expired());
		return hasLivePresentation &&
			existing->second.eKind == spawned.eKind &&
			existing->second.strArchetypeId == spawned.strArchetypeId &&
			existing->second.strEncounterId == spawned.strEncounterId &&
			existing->second.strPlacementId == spawned.strPlacementId &&
			existing->second.iOwnerBossNetEntityId == spawned.iOwnerBossNetEntityId &&
			existing->second.fCollisionRadius == spawned.fCollisionRadius &&
			existing->second.PinnedDefinitionRevision ==
				spawned.PinnedDefinitionRevision;
	}

	if (WORLD_ENTITY_KIND::NPC == spawned.eKind)
	{
		const NPC_ACTOR_ENTRY* actor =
			CActorCatalog::Find_Npc(spawned.strArchetypeId);
		const wstring_t modelTag =
			CNpcPresentationAssetService::Get_ModelPrototypeTag(
				spawned.strArchetypeId);
		if (nullptr == actor || modelTag.empty() ||
			FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
				m_Desc.pDevice,
				m_Desc.pContext,
				m_Desc.iPrototypeLevelIndex,
				spawned.strArchetypeId)))
		{
			return false;
		}

		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
		desc.strModelTag = modelTag;
		desc.strShaderTag = actor->shaderProfile == "esther" ?
			TEXT("Prototype_Component_Shader_VtxEstherNpc") :
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		NPC_PLACEMENT_PRESENTATION_ENTRY placementPresentation;
		const bool_t hasPlacementPresentation =
			CNpcPlacementPresentationService::Try_Get_Presentation(
				m_Desc.iPrototypeLevelIndex,
				spawned.strPlacementId,
				placementPresentation);
		const std::string& resolvedIdle =
			hasPlacementPresentation &&
			!placementPresentation.strIdleClip.empty() ?
				placementPresentation.strIdleClip : actor->idleClip;
		desc.pIdleClip = resolvedIdle.c_str();
		/* Only authored placement behavior suppresses root motion. Esther
		summons have no placement document and retain their action-chain travel. */
		desc.bSuppressRootMotion = hasPlacementPresentation;
		desc.bInterpolateNetworkTransform = hasPlacementPresentation;
		desc.vPosition = float3_t(
			spawned.fPositionX,
			spawned.fPositionY,
			spawned.fPositionZ);
		desc.fYawDegree = spawned.fYawDegrees;
		desc.fCollisionRadius = spawned.fCollisionRadius;
		if (actor->shaderProfile == "esther")
			desc.fOutlineWidth = CNpc::ESTHER_OUTLINE_WIDTH;

		std::shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_Desc.iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Npc"),
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			&desc,
			&gameObject)))
		{
			return false;
		}
		const std::shared_ptr<CNpc> npc =
			std::dynamic_pointer_cast<CNpc>(gameObject);
		if (nullptr == npc || !npc->Apply_NetworkState(
			desc.vPosition, spawned.fYawDegrees))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				gameObject);
			return false;
		}

		WORLD_ENTITY_PRESENTATION presentation{};
		presentation.eKind = spawned.eKind;
		presentation.strPlacementId = spawned.strPlacementId;
		presentation.strArchetypeId = spawned.strArchetypeId;
		presentation.strEncounterId = spawned.strEncounterId;
		presentation.strCurrentClip = resolvedIdle;
		presentation.strResolvedIdleClip = resolvedIdle;
		presentation.NpcPresentation = std::move(placementPresentation);
		presentation.fCollisionRadius = spawned.fCollisionRadius;
		presentation.PinnedDefinitionRevision =
			spawned.PinnedDefinitionRevision;
		presentation.pNpc = npc;
		/* An entity that spawns mid-action (a raid Esther summon) must show
		its action clip from the very first rendered frame; waiting for the
		next snapshot leaves it one interval in the idle pose at the caster's
		feet before the clip teleports it into its authored entrance. */
		const auto spawnActionClip =
			actor->actionClips.find(spawned.strActionId);
		if (spawnActionClip != actor->actionClips.end() &&
			npc->Set_Animation(
				spawnActionClip->second.front().c_str(), false))
		{
			presentation.strActiveActionId = spawned.strActionId;
			presentation.iActionClipIndex = 0u;
			presentation.strCurrentClip = spawnActionClip->second.front();
			if (const std::shared_ptr<Engine::CModel> model =
				npc->Get_Model())
			{
				model->Play_Animation(0.f);
			}
			CCombatHUDViewModel::Get().Apply_EstherCutinAction(
				spawned.strArchetypeId, spawnActionClip->second);
		}
		else if (hasPlacementPresentation)
		{
			const NPC_ACTION_PLAYBACK_REQUEST request =
				CNpcActionPresentationRuntime::Resolve_Playback(
					spawned.strActionId,
					presentation.strResolvedIdleClip,
					presentation.NpcPresentation,
					nullptr);
			CNpcActionPresentationRuntime::Apply_Playback(
				request,
				presentation.strResolvedIdleClip,
				[&npc](const NPC_ACTION_PLAYBACK_REQUEST& playback)
				{
					return npc->Play_NetworkAction(
						playback.strClipName.c_str(),
						playback.isLoop,
						playback.fPlaybackRate,
						playback.fBlendSeconds);
				},
				[&npc](const f32_t blendSeconds)
				{
					return npc->Play_DefaultIdle(blendSeconds);
				},
				presentation.strCurrentClip);
		}
#ifdef _DEBUG
		npc->Set_CombatColliderDebugVisible(
			m_isCombatColliderDebugVisible);
#endif
		const auto [iter, inserted] = m_WorldEntities.emplace(
			spawned.iNetEntityId, std::move(presentation));
		(void)iter;
		if (!inserted)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				npc);
		}
		return inserted;
	}

	if (WORLD_ENTITY_KIND::MONSTER == spawned.eKind)
	{
		const MONSTER_ACTOR_ENTRY* actor =
			CActorCatalog::Find_Monster(spawned.strArchetypeId);
		if (nullptr == actor || actor->runtimeStatus != "supported" ||
			FAILED(CMonsterPresentationAssetService::Ensure_Prototypes(
				m_Desc.pDevice,
				m_Desc.pContext,
				m_Desc.iPrototypeLevelIndex,
				spawned.strArchetypeId)))
		{
			return false;
		}

		const std::wstring modelTag =
			CMonsterPresentationAssetService::Get_ModelPrototypeTag(
				spawned.strArchetypeId);
		CNpc::NPC_DESC desc{};
		desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
		desc.strModelTag = modelTag;
		desc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		desc.pIdleClip = actor->presentationClips.idle.c_str();
		desc.bSuppressRootMotion = true;
		desc.bInterpolateNetworkTransform = true;
		desc.vPosition = float3_t(
			spawned.fPositionX,
			spawned.fPositionY,
			spawned.fPositionZ);
		desc.fYawDegree = spawned.fYawDegrees;
		desc.fCollisionRadius = spawned.fCollisionRadius;

		std::shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			m_Desc.iPrototypeLevelIndex,
			CMonsterPresentationAssetService::Get_GameObjectPrototypeTag(),
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			&desc,
			&gameObject)))
		{
			return false;
		}
		const std::shared_ptr<CNpc> monster =
			std::dynamic_pointer_cast<CNpc>(gameObject);
		if (nullptr == monster || !monster->Apply_NetworkState(
			desc.vPosition, spawned.fYawDegrees))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				gameObject);
			return false;
		}

		WORLD_ENTITY_PRESENTATION presentation{};
		presentation.eKind = spawned.eKind;
		presentation.strPlacementId = spawned.strPlacementId;
		presentation.strArchetypeId = spawned.strArchetypeId;
		presentation.strEncounterId = spawned.strEncounterId;
		presentation.strCurrentClip = actor->presentationClips.idle;
		presentation.fCollisionRadius = spawned.fCollisionRadius;
		presentation.PinnedDefinitionRevision =
			spawned.PinnedDefinitionRevision;
		presentation.pNpc = monster;
#ifdef _DEBUG
		monster->Set_CombatColliderDebugVisible(
			m_isCombatColliderDebugVisible);
#endif
		const auto [iter, inserted] = m_WorldEntities.emplace(
			spawned.iNetEntityId, std::move(presentation));
		(void)iter;
		if (!inserted)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				monster);
		}
		return inserted;
	}

	const BOSS_ACTOR_ENTRY* pBoss =
		CActorCatalog::Find_Boss(spawned.strArchetypeId);
	if (spawned.eKind != WORLD_ENTITY_KIND::BOSS ||
		nullptr == pBoss ||
		pBoss->clientPresentationId != "boss.valtan.client.v1")
	{
		return false;
	}
	if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
		m_Desc.pDevice,
		m_Desc.pContext,
		m_Desc.iPrototypeLevelIndex, spawned.strArchetypeId)))
	{
		m_strPendingPresentationFailure =
			"Boss model admission failed: " + spawned.strArchetypeId;
		return false;
	}

	CValtan::VALTAN_DESC desc{};
	desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
	desc.vPosition = float3_t(
		spawned.fPositionX,
		spawned.fPositionY,
		spawned.fPositionZ);
	desc.fScale = pBoss->presentationScale;
	desc.isServerAuthoritative = true;
	desc.strArchetypeId = spawned.strArchetypeId;
	desc.iOwnerBossNetEntityId = spawned.iOwnerBossNetEntityId;
	desc.fCollisionRadius = spawned.fCollisionRadius;
	std::shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		m_Desc.iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Valtan"),
		m_Desc.iLayerLevelIndex,
		m_Desc.strWorldEntityLayerTag,
		&desc,
		&gameObject)))
	{
		return false;
	}
	const std::shared_ptr<CValtan> valtan =
		std::dynamic_pointer_cast<CValtan>(gameObject);
	if (nullptr == valtan || !valtan->Apply_NetworkState(
		desc.vPosition,
		spawned.fYawDegrees,
		WORLD_ENTITY_ACTION::IDLE,
		{}, {}, 0u, 0u, 0u, 0u, {}))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			gameObject);
		return false;
	}

	WORLD_ENTITY_PRESENTATION presentation{};
	presentation.eKind = spawned.eKind;
	presentation.strPlacementId = spawned.strPlacementId;
	presentation.strArchetypeId = spawned.strArchetypeId;
	presentation.strEncounterId = spawned.strEncounterId;
	presentation.fCollisionRadius = spawned.fCollisionRadius;
	presentation.PinnedDefinitionRevision = spawned.PinnedDefinitionRevision;
	presentation.pValtan = valtan;
	presentation.iOwnerBossNetEntityId = spawned.iOwnerBossNetEntityId;
	const bool_t isPrimaryValtan = "BOSS_VALTAN" == spawned.strArchetypeId &&
		LostArk::Shared::INVALID_NET_ENTITY_ID ==
			spawned.iOwnerBossNetEntityId;
	std::string JoinedReloadStatus;
	std::string CombatObjectSoundReloadStatus;
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT PresentationReceipt;
	std::string ReceiptStatus;
	const bool_t hasExactReceipt =
		CNetworkManager::Get().Try_Get_ValtanPresentationGenerationReceipt(
			spawned.PinnedDefinitionRevision,
			PresentationReceipt, ReceiptStatus);
	const bool_t entryReceiptRecoveryPending = !hasExactReceipt &&
		CNetworkManager::Get().Get_GameplayRevisionState().
			hasPendingEntryPresentationBaselineRecovery;
	const bool_t joinedReloaded = hasExactReceipt &&
		valtan->Reload_PatternPresentationAuthoring(
			spawned.PinnedDefinitionRevision,
			PresentationReceipt, JoinedReloadStatus);
	const bool_t combatObjectSoundReloaded = joinedReloaded;
	CombatObjectSoundReloadStatus = joinedReloaded ? JoinedReloadStatus :
		(hasExactReceipt ? JoinedReloadStatus : ReceiptStatus);
	if (joinedReloaded)
	{
		presentation.AdmittedPresentationRevision =
			spawned.PinnedDefinitionRevision;
	}
	else
	{
		if (!entryReceiptRecoveryPending)
		{
			presentation.RejectedPresentationRevision =
				spawned.PinnedDefinitionRevision;
		}
		presentation.bPresentationIsolated = true;
	}
#ifdef _DEBUG
	valtan->Set_CombatColliderDebugVisible(
		m_isCombatColliderDebugVisible);
	valtan->Set_PatternHitAreaDebugVisible(
		m_isSkillHitAreaDebugVisible);
#endif
	if (!isPrimaryValtan && !joinedReloaded)
	{
		m_strPendingPresentationFailure =
			"Replicated Valtan presentation rejected its exact generation receipt: " +
			(hasExactReceipt ? JoinedReloadStatus : ReceiptStatus);
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			valtan);
		return false;
	}
	const auto [iter, inserted] = m_WorldEntities.emplace(
		spawned.iNetEntityId,
		std::move(presentation));
	(void)iter;
	if (!inserted)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			valtan);
	}
	else if (isPrimaryValtan)
	{
		if (joinedReloaded)
		{
			m_PrimaryValtanJoinedPresentationFreshness.Admit(
				spawned.PinnedDefinitionRevision, JoinedReloadStatus);
		}
		else
		{
			const std::string Diagnostic =
				(entryReceiptRecoveryPending ?
					"Primary replicated Valtan spawn is waiting to recover the saved joined presentation cache: " :
					"Primary replicated Valtan spawn could not admit the joined presentation cache: ") +
				(hasExactReceipt ? JoinedReloadStatus : ReceiptStatus);
			if (!entryReceiptRecoveryPending)
				m_PrimaryValtanJoinedPresentationFreshness.Reject(Diagnostic);
			m_strPendingPresentationFailure = Diagnostic;
		}
		if (combatObjectSoundReloaded)
		{
			m_PrimaryValtanCombatObjectSoundFreshness.Admit(
				spawned.PinnedDefinitionRevision,
				CombatObjectSoundReloadStatus);
		}
		else
		{
			const std::string Diagnostic =
				(entryReceiptRecoveryPending ?
					"Primary replicated Valtan spawn is waiting to recover the saved combat-object Sound cache: " :
					"Primary replicated Valtan spawn could not admit the combat-object Sound cache: ") +
				CombatObjectSoundReloadStatus;
			if (!entryReceiptRecoveryPending)
				m_PrimaryValtanCombatObjectSoundFreshness.Reject(Diagnostic);
			if (!m_strPendingPresentationFailure.empty())
				m_strPendingPresentationFailure += " ";
			m_strPendingPresentationFailure += Diagnostic;
		}
	}
	return inserted;
}

void Client::CClientReplication::Remove_DependentBossPresentations(
	const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId)
{
	std::vector<LostArk::Shared::NET_ENTITY_ID> children;
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		if (presentation.iOwnerBossNetEntityId == ownerBossNetEntityId)
			children.push_back(entityId);
	}
	// Admission forbids nested ownership. Collect IDs before erasing map entries.
	for (const LostArk::Shared::NET_ENTITY_ID child : children)
	{
		LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED despawned{};
		despawned.iNetEntityId = child;
		Apply_WorldEntityDespawn(despawned);
	}
}

bool Client::CClientReplication::Apply_WorldEntityDespawn(
	const LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	using LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == despawned.iNetEntityId ||
		(despawned.eReason != WORLD_ENTITY_DESPAWN_REASON::REMOVED &&
		 despawned.eReason != WORLD_ENTITY_DESPAWN_REASON::DEAD))
	{
		return false;
	}
	const auto iter = m_WorldEntities.find(despawned.iNetEntityId);
	if (m_WorldEntities.end() == iter)
		return true;
	/* A DEAD despawn is the reliable terminal edge for a boss. The Server removes
	   the entity before it builds that tick's world snapshot, so a final snapshot
	   with eAction == DEAD is explicitly not guaranteed. Latch the primary boss
	   death before removing its presentation; dependent bosses must not complete
	   the raid. */
	if (WORLD_ENTITY_DESPAWN_REASON::DEAD == despawned.eReason &&
		LostArk::Shared::WORLD_ENTITY_KIND::BOSS == iter->second.eKind &&
		LostArk::Shared::INVALID_NET_ENTITY_ID ==
			iter->second.iOwnerBossNetEntityId)
	{
		CCombatHUDViewModel::Get().Set_BossDeadRaw(true);
	}
	Remove_DependentBossPresentations(despawned.iNetEntityId);
	COMBAT_OBJECT_PRESENTATION_SINK combatObjectSink{ *this };
	const size_t removedCombatObjects =
		m_CombatObjectProjectionRuntime.Remove_Source(
			despawned.iNetEntityId, combatObjectSink);
	if (0u != removedCombatObjects)
	{
		m_strPendingPresentationFailure =
			"Combat-object owner despawned before reliable object cleanup.";
	}

	if (const std::shared_ptr<CNpc> npc = iter->second.pNpc.lock())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			npc);
	}
	if (const std::shared_ptr<CValtan> valtan = iter->second.pValtan.lock())
	{
		CEffectPresentationService::Stop_BossOwner(valtan);
		if (!iter->second.bPresentationIsolated &&
			WORLD_ENTITY_DESPAWN_REASON::DEAD == despawned.eReason &&
			valtan->Begin_NetworkDeathPresentation())
		{
			// A reliable death starts even if its last DEAD snapshot never arrived.
			// Repeated despawns find no registry entry and cannot restart the clip.
			m_DeathPresentations.emplace_back(valtan);
		}
		else
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				valtan);
		}
	}
	if (m_ValtanPresentationState.isValid &&
		m_ValtanPresentationState.iNetEntityId == despawned.iNetEntityId)
	{
		m_ValtanPresentationState = {};
		CCombatHUDViewModel::Get().Clear_Boss();
	}
	if (m_Desc.onWorldEntityDespawned &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId)
	{
		m_Desc.onWorldEntityDespawned(
			iter->second.strPlacementId,
			iter->second.strArchetypeId);
	}
	m_WorldEntities.erase(iter);
	/* A despawn must not erase a reload rejection. Complete Play remains blocked
	   until Reset_World establishes a new world lifetime or a later primary spawn
	   successfully reloads the authoritative caches. */
	return true;
}

bool Client::CClientReplication::Apply_CombatObjectSpawn(
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
	const auto source = m_WorldEntities.find(spawned.iSourceNetEntityId);
	if (source == m_WorldEntities.end() ||
		WORLD_ENTITY_KIND::BOSS != source->second.eKind ||
		source->second.pValtan.expired())
	{
		return false;
	}
	if (!source->second.bPresentationIsolated &&
		nullptr == CActorCatalog::Find_BossCombatObjectVisual(
			source->second.strArchetypeId,
			spawned.strCombatObjectArchetypeId,
			spawned.strClientVisualId))
	{
		return false;
	}
	if (source->second.PinnedDefinitionRevision !=
		spawned.PinnedDefinitionRevision)
	{
		m_strPendingPresentationFailure =
			"Combat-object spawn revision does not match its boss occurrence.";
		return false;
	}

	COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
	std::string status;
	if (!m_CombatObjectProjectionRuntime.Apply_Spawn(
		spawned, sink, status))
	{
		m_strPendingPresentationFailure = std::move(status);
		return false;
	}
	const COMBAT_OBJECT_PROJECTION_RECORD* record =
		m_CombatObjectProjectionRuntime.Find(spawned.iCombatObjectId);
	if (nullptr != record && 0u == record->iPresentationHandle)
		m_strPendingPresentationFailure = std::move(status);
	return true;
}

bool Client::CClientReplication::Apply_CombatObjectPresentationEvent(
	const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event)
{
	const auto source = m_WorldEntities.find(event.iSourceNetEntityId);
	if (source == m_WorldEntities.end() ||
		LostArk::Shared::WORLD_ENTITY_KIND::BOSS != source->second.eKind)
	{
		m_strPendingPresentationFailure =
			"Combat-object presentation event has no live boss owner.";
		return false;
	}
	if (source->second.bPresentationIsolated)
		return true;
	if (source->second.PinnedDefinitionRevision !=
		event.PinnedDefinitionRevision)
	{
		m_strPendingPresentationFailure =
			"Combat-object presentation event revision does not match its boss occurrence.";
		return false;
	}
	const std::shared_ptr<CValtan> boss = source->second.pValtan.lock();
	if (nullptr == boss)
	{
		m_strPendingPresentationFailure =
			"Combat-object presentation event boss projection expired.";
		return false;
	}
	std::string status;
	const bool_t applied =
		boss->Apply_CombatObjectPresentationEvent(event, status);
	if (!applied)
		m_strPendingPresentationFailure = std::move(status);
	return applied;
}

bool Client::CClientReplication::Apply_CombatObjectDespawn(
	const LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED& despawned)
{
	COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
	std::string status;
	const bool_t applied = m_CombatObjectProjectionRuntime.Apply_Despawn(
		despawned, sink, status);
	if (!applied)
		m_strPendingPresentationFailure = std::move(status);
	return applied;
}

bool Client::CClientReplication::Spawn_CombatObjectPresentation(
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned,
	uint64_t& outHandle,
	std::string& outStatus)
{
	outHandle = 0u;
	const auto source = m_WorldEntities.find(spawned.iSourceNetEntityId);
	if (source == m_WorldEntities.end() ||
		source->second.eKind != LostArk::Shared::WORLD_ENTITY_KIND::BOSS)
	{
		outStatus = "Combat-object visual has no live boss owner.";
		return false;
	}
	if (source->second.bPresentationIsolated)
	{
		outStatus =
			"Combat-object visual is isolated with its rejected boss presentation revision.";
		return true;
	}
	if (source->second.PinnedDefinitionRevision !=
		spawned.PinnedDefinitionRevision)
	{
		outStatus =
			"Combat-object visual revision does not match its boss occurrence.";
		return false;
	}
	const std::shared_ptr<CValtan> boss = source->second.pValtan.lock();
	const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* visual =
		CActorCatalog::Find_BossCombatObjectVisual(
			source->second.strArchetypeId,
			spawned.strCombatObjectArchetypeId,
			spawned.strClientVisualId);
	if (nullptr == boss || nullptr == visual)
	{
		outStatus = "Combat-object visual join is missing.";
		return false;
	}

	float actionAgeSeconds = 0.f;
	if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
		spawned.iServerTick,
		spawned.iSpawnTick,
		30.f,
		actionAgeSeconds))
	{
		outStatus = "Combat-object spawn tick is not seekable.";
		return false;
	}

	EFFECT_WORLD_ROOT_SPAWN_DESC desc;
	desc.strEffectAssetId = visual->effectAssetId;
	desc.pBossBudgetAndLifetimeOwner = boss;
	desc.RootWorld = visual->Make_WorldRoot(
		float3_t(spawned.fPositionX, spawned.fPositionY, spawned.fPositionZ),
		spawned.fYawDegrees);
	desc.strOccurrenceId = "combatobject.instance." +
		std::to_string(spawned.iCombatObjectId);
	desc.iSpawnTick = spawned.iSpawnTick;
	desc.fInitialSampleTimeSeconds = actionAgeSeconds;
	EFFECT_WORLD_ROOT_HANDLE handle;
	if (!CEffectPresentationService::Spawn_WorldRoot(
		desc, handle, outStatus))
	{
		return false;
	}
	outHandle = handle.iValue;
	return true;
}

bool Client::CClientReplication::Update_CombatObjectPresentation(
	const uint64_t handle,
	const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot)
{
	const COMBAT_OBJECT_PROJECTION_RECORD* record =
		m_CombatObjectProjectionRuntime.Find(snapshot.iCombatObjectId);
	const auto source = m_WorldEntities.find(snapshot.iSourceNetEntityId);
	if (nullptr == record || source == m_WorldEntities.end() ||
		record->iSourceNetEntityId != snapshot.iSourceNetEntityId ||
		record->iPresentationHandle != handle)
	{
		return false;
	}
	if (source->second.bPresentationIsolated)
		return true;
	if (source->second.PinnedDefinitionRevision !=
		snapshot.PinnedDefinitionRevision)
	{
		return false;
	}
	const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* visual =
		CActorCatalog::Find_BossCombatObjectVisual(source->second.strArchetypeId,
			record->strCombatObjectArchetypeId, record->strClientVisualId);
	if (nullptr == visual)
		return false;
	EFFECT_WORLD_ROOT_HANDLE effectHandle;
	effectHandle.iValue = handle;
	return CEffectPresentationService::Update_WorldRoot(effectHandle,
		visual->Make_WorldRoot(
			float3_t(snapshot.fPositionX, snapshot.fPositionY, snapshot.fPositionZ),
			snapshot.fYawDegrees));
}

void Client::CClientReplication::Stop_CombatObjectPresentation(
	const uint64_t handle)
{
	EFFECT_WORLD_ROOT_HANDLE effectHandle;
	effectHandle.iValue = handle;
	CEffectPresentationService::Stop_WorldRoot(effectHandle);
}

void Client::CClientReplication::Stage_PlayerAttachmentPresentation(
	const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::GRABBED != snapshot.eAction)
	{
		m_PlayerAttachments.erase(snapshot.iNetEntityId);
		return;
	}

	PLAYER_ATTACHMENT_PRESENTATION& presentation =
		m_PlayerAttachments[snapshot.iNetEntityId];
	if (presentation.iOwnerNetEntityId !=
			snapshot.iAttachmentOwnerNetEntityId ||
		presentation.eSlot != snapshot.eAttachmentSlot)
	{
		presentation = {};
		presentation.iOwnerNetEntityId =
			snapshot.iAttachmentOwnerNetEntityId;
		presentation.eSlot = snapshot.eAttachmentSlot;
		/* The replicated offset is expressed in the boss gameplay-root frame and
		cannot be composed with a presentation bone. Capture the hand-local matrix
		once both actual presentation transforms are available below. */
		presentation.bHasLocalOffset = false;
	}
}

void Client::CClientReplication::Update_PlayerAttachmentPresentations()
{
	using namespace LostArk::Shared;
	for (auto attachment = m_PlayerAttachments.begin();
		attachment != m_PlayerAttachments.end();)
	{
		OBJECT_HANDLE playerHandle{};
		if (!m_Registry.Find_Handle(attachment->first, playerHandle))
		{
			attachment = m_PlayerAttachments.erase(attachment);
			continue;
		}
		const std::shared_ptr<CCharacter> character =
			m_Registry.Resolve(playerHandle);
		const auto owner =
			m_WorldEntities.find(attachment->second.iOwnerNetEntityId);
		if (nullptr == character || nullptr == character->Get_Transform() ||
			m_WorldEntities.end() == owner ||
			WORLD_ENTITY_KIND::BOSS != owner->second.eKind ||
			owner->second.bPresentationIsolated ||
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND !=
				attachment->second.eSlot)
		{
			/* Apply_NetworkState already staged the Server fallback transform.
			Keep the identity so a presentation that spawns later can attach, but
			never apply a stale matrix while its owner is unavailable. */
			++attachment;
			continue;
		}

		const std::shared_ptr<CValtan> valtan = owner->second.pValtan.lock();
		const std::shared_ptr<Engine::CModel> body =
			nullptr == valtan ? nullptr : valtan->Get_BodyModel();
		float4x4_t presentationRoot{};
		if (nullptr == valtan || nullptr == body ||
			!body->Has_Bone(VALTAN_LEFT_HAND_BONE) ||
			!valtan->Try_Get_PresentationRootMatrix(&presentationRoot) ||
			!Is_FiniteMatrix(presentationRoot))
		{
			++attachment;
			continue;
		}

		const matrix_t handWorld = body->Get_BoneMatrix(
			VALTAN_LEFT_HAND_BONE) * XMLoadFloat4x4(&presentationRoot);
		float4x4_t handWorldStored{};
		XMStoreFloat4x4(&handWorldStored, handWorld);
		if (!Is_FiniteMatrix(handWorldStored))
		{
			++attachment;
			continue;
		}
		if (!attachment->second.bHasLocalOffset)
		{
			float4x4_t localOffset{};
			if (!CPlayerHandGripTransform::Build_LocalOffset(
					*character->Get_Transform()->Get_WorldMatrixPtr(),
					handWorldStored, localOffset))
			{
				++attachment;
				continue;
			}
			attachment->second.LocalOffset = localOffset;
			attachment->second.bHasLocalOffset = true;
		}

		/* The grip origin is fixed for every captured player. Only the captured
		   orientation/scale follows the animated hand, never the hit distance. */
		float4x4_t attachedStored{};
		if (!CPlayerHandGripTransform::Compose_World(
				attachment->second.LocalOffset, handWorldStored, attachedStored))
		{
			++attachment;
			continue;
		}
		const matrix_t attachedWorld = XMLoadFloat4x4(&attachedStored);

		const std::shared_ptr<Engine::CTransform> transform =
			character->Get_Transform();
		transform->Set_State(STATE::RIGHT, attachedWorld.r[0]);
		transform->Set_State(STATE::UP, attachedWorld.r[1]);
		transform->Set_State(STATE::LOOK, attachedWorld.r[2]);
		transform->Set_State(
			STATE::POSITION, XMVectorSetW(attachedWorld.r[3], 1.f));
		++attachment;
	}
}

bool Client::CClientReplication::Apply_WorldSnapshot(
	const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot)
{
	//server tick ?좏슚??寃??-> 留덉?留?tick蹂대떎 ??snapshot?몄? 寃??
	//->snapshot??player 諛곗뿴 ?쒗쉶 -> netentityid濡?objecthandle 寃??
	//handle濡??댁븘 ?덈뒗 character resolve
	//?꾩튂 float3 ?앹꽦 -> locomotion??bool ismoving?쇰줈 蹂??
	//character apply networkstate -> 留덉?留?servertick 媛깆떊

	//snapshot ?덉뿉 ?덈뒗 packet??낆씠??session ?뺣낫瑜?character濡??섍린吏 ?딅뒗??
	//character媛 諛쏅뒗 媛믪쓣 ?쒖닔 ?쒗쁽 媛믪씠??

	using namespace LostArk::Shared;

	if (0 == snapshot.iServerTick)
		return false;

	// ?꾩옱 TCP 寃쎈줈???쒖꽌媛 蹂댁옣?쒕떎. 以묐났 ?먮뒗 ??쟾 Tick? ?ㅼ떆 ?곸슜?섏? ?딅뒗??
	if (!CActionPresentationTimeline::Is_ForwardTick(
		snapshot.iServerTick, m_iLastServerTick))
		return true;
	if (!m_PlayerHealth.Apply_Snapshot(snapshot))
		return false;

	bool allSucceeded = true;

	for (const PLAYER_SNAPSHOT& player : snapshot.Players)
	{
		const NET_PLAYER_RECORD* record =
			m_Registry.Find_Record(player.iNetEntityId);
		const bool_t isLocallyControlled = player.iNetEntityId ==
			CNetworkManager::Get().Get_LocalEntityId();
		if (nullptr != record &&
			record->eCharacterClass != player.eCharacterClass)
		{
			if (isLocallyControlled &&
				m_Desc.bDeferLocalCharacterClassReplacement)
			{
				Stage_LocalCharacterClassReplacement(
					player, snapshot.iServerTick);
				continue;
			}
			const CHARACTER_REPLACE_RESULT replaceResult =
				Replace_CharacterClass(player);
			if (CHARACTER_REPLACE_RESULT::FATAL_FAILURE == replaceResult)
			{
				allSucceeded = false;
				continue;
			}
			if (CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE == replaceResult)
				continue;
		}
		else if (isLocallyControlled &&
			m_DeferredLocalCharacterClassReplacement.isPending)
		{
			/* A newer authoritative snapshot returned to the currently committed
			   class before presentation commit.  Drop the superseded generation. */
			Clear_DeferredLocalCharacterClassReplacement();
		}
		OBJECT_HANDLE handle{};

		if (!m_Registry.Find_Handle(
			player.iNetEntityId,
			handle))
		{
			allSucceeded = false;
			continue;
		}

		const std::shared_ptr<CCharacter> character =
			m_Registry.Resolve(handle);

		if (nullptr == character)
		{
			allSucceeded = false;
			continue;
		}

		const float3_t position(
			player.fPositionX,
			player.fPositionY,
			player.fPositionZ);

		const bool_t isMoving =
			player.eLocomotionState ==
			PLAYER_LOCOMOTION_STATE::MOVING;

		if (!character->Apply_NetworkState(
			position,
			player.fYawDegrees,
			isMoving,
			snapshot.iServerTick) ||
			!character->Apply_NetworkAction(
				player.eAction,
				player.iSkillId,
				snapshot.iServerTick,
				player.iActionStartTick,
				player.fYawDegrees,
				player.iComboStage,
				player.hasSkillTarget,
				float3_t(
					player.fSkillTargetX,
					player.fSkillTargetY,
					player.fSkillTargetZ)))
		{
			allSucceeded = false;
		}
		Stage_PlayerAttachmentPresentation(player);
		character->Apply_NetworkStance(player.eStance);
		if (isLocallyControlled)
		{
			const NET_PLAYER_RECORD* localRecord =
				m_Registry.Find_Record(player.iNetEntityId);
			if (nullptr == localRecord)
			{
				allSucceeded = false;
			}
			else
			{
				CCombatHUDViewModel::Get().Apply_LocalPlayer(
					snapshot.iServerTick,
					localRecord->eCharacterClass,
					player);
			}
		}
	}
	std::vector<NET_ENTITY_ID> deadBossOwners;
	for (const WORLD_ENTITY_SNAPSHOT& entity : snapshot.Entities)
	{
		const auto iter = m_WorldEntities.find(entity.iNetEntityId);
		if (iter == m_WorldEntities.end())
		{
			allSucceeded = false;
			continue;
		}
		const bool_t expectsBossCombatState =
			WORLD_ENTITY_KIND::BOSS == iter->second.eKind;
		if (expectsBossCombatState != entity.hasBossCombatState)
		{
			allSucceeded = false;
			continue;
		}
		const float3_t position(
			entity.fPositionX,
			entity.fPositionY,
			entity.fPositionZ);
		if (WORLD_ENTITY_KIND::BOSS != iter->second.eKind)
		{
			iter->second.PinnedDefinitionRevision =
				entity.PinnedDefinitionRevision;
		}
		if (WORLD_ENTITY_KIND::NPC == iter->second.eKind)
		{
			const std::shared_ptr<CNpc> npc = iter->second.pNpc.lock();
			if (nullptr == npc ||
				!npc->Apply_NetworkState(
					position, entity.fYawDegrees, snapshot.iServerTick))
			{
				allSucceeded = false;
				continue;
			}
			/* Raid Esther actions retain their catalog-authored multi-clip chains.
			Town placement actions are resolved separately from the published
			placement presentation binding below. */
			const NPC_ACTOR_ENTRY* actor =
				CActorCatalog::Find_Npc(iter->second.strArchetypeId);
			if (nullptr == actor)
			{
				allSucceeded = false;
				continue;
			}
			const auto actionClip =
				actor->actionClips.find(entity.strActionId);
			if (actionClip != actor->actionClips.end())
			{
				std::string soundStatus;
				/* Reliable spawn carries the action id but not its Server start tick.
				   The first snapshot supplies the authoritative occurrence clock;
				   subsequent snapshots advance/deduplicate the same cue cursor. */
				(void)CEstherActionSoundCueDocument::Play_Due(
					ESTHER_ACTION_SOUND_OWNER_KIND::NPC_ACTION,
					iter->second.strArchetypeId, entity.strActionId,
					snapshot.iServerTick, entity.iActionStartTick,
					iter->second.EstherActionSoundState, soundStatus);
				const std::vector<std::string>& chain = actionClip->second;
				if (iter->second.strActiveActionId != entity.strActionId)
				{
					if (!npc->Set_Animation(chain.front().c_str(), false))
					{
						allSucceeded = false;
						continue;
					}
					iter->second.strActiveActionId = entity.strActionId;
					iter->second.iActionClipIndex = 0u;
					iter->second.strCurrentClip = chain.front();
					CCombatHUDViewModel::Get().Apply_EstherCutinAction(
						iter->second.strArchetypeId, chain);
				}
				else if (iter->second.iActionClipIndex + 1u < chain.size())
				{
					const std::shared_ptr<Engine::CModel> model =
						npc->Get_Model();
					f32_t position = 0.f;
					f32_t duration = 0.f;
					if (nullptr != model &&
						model->Get_AnimationProgress(
							model->Get_CurrentAnimIndex(),
							position, duration) &&
						duration > 0.f && position >= duration)
					{
						const std::string& next =
							chain[iter->second.iActionClipIndex + 1u];
						if (!npc->Set_Animation(next.c_str(), false))
						{
							allSucceeded = false;
						}
						else
						{
							++iter->second.iActionClipIndex;
							iter->second.strCurrentClip = next;
						}
					}
				}
				continue;
			}

			/* Placement-authored behavior uses one validated semantic-action
			binding. It is deliberately separate from the Esther chain above:
			flattening an Esther chain here would lose its cutin and root motion. */
			iter->second.strActiveActionId.clear();
			iter->second.iActionClipIndex = 0u;
			if (!CNpcActionPresentationRuntime::Is_NewActionEdge(
					iter->second.NpcActionEdge,
					entity.strActionId,
					entity.iActionStartTick))
			{
				continue;
			}
			const NPC_ACTION_PLAYBACK_REQUEST request =
				CNpcActionPresentationRuntime::Resolve_Playback(
					entity.strActionId,
					iter->second.strResolvedIdleClip,
					iter->second.NpcPresentation,
					nullptr);
			CNpcActionPresentationRuntime::Apply_Playback(
				request,
				iter->second.strResolvedIdleClip,
				[&npc](const NPC_ACTION_PLAYBACK_REQUEST& playback)
				{
					return npc->Play_NetworkAction(
						playback.strClipName.c_str(),
						playback.isLoop,
						playback.fPlaybackRate,
						playback.fBlendSeconds);
				},
				[&npc](const f32_t blendSeconds)
				{
					return npc->Play_DefaultIdle(blendSeconds);
				},
				iter->second.strCurrentClip);
			CNpcActionPresentationRuntime::Commit_ActionEdge(
				iter->second.NpcActionEdge,
				entity.strActionId,
				entity.iActionStartTick);
		}
		else if (WORLD_ENTITY_KIND::MONSTER == iter->second.eKind)
		{
			const std::shared_ptr<CNpc> monster = iter->second.pNpc.lock();
			const MONSTER_ACTOR_ENTRY* actor =
				CActorCatalog::Find_Monster(iter->second.strArchetypeId);
			if (nullptr == monster || nullptr == actor ||
				!monster->Apply_NetworkState(
					position, entity.fYawDegrees, snapshot.iServerTick))
			{
				allSucceeded = false;
				continue;
			}

			const MONSTER_PRESENTATION_ACTION_FRAME frame =
				CMonsterPresentationContract::Project(
					entity.eAction,
					entity.iActionStartTick,
					iter->second.MonsterActionState);
			if (!frame.shouldRestartClip)
				continue;

			const std::string* clip = &actor->presentationClips.idle;
			f32_t playbackRate = 1.f;
			switch (frame.eKind)
			{
			case MONSTER_PRESENTATION_ACTION_KIND::CHASE:
				clip = &actor->presentationClips.chase;
				break;
			case MONSTER_PRESENTATION_ACTION_KIND::ATTACK:
				if (!actor->attackPresentations.empty())
				{
					const std::size_t attackIndex =
						CMonsterPresentationContract::Select_AttackPresentation(
							entity.iNetEntityId,
							frame.iOccurrenceStartTick,
							actor->attackPresentations.size());
					clip = &actor->attackPresentations[attackIndex].clip;
					playbackRate =
						actor->attackPresentations[attackIndex].playbackRate;
				}
				break;
			case MONSTER_PRESENTATION_ACTION_KIND::DEAD:
				clip = &actor->presentationClips.dead;
				break;
			default:
				break;
			}
			if (monster->Play_NetworkAction(
					clip->c_str(), frame.isLoop, playbackRate, 0.12f))
			{
				iter->second.strCurrentClip = *clip;
			}
			else
			{
				/* A missing optional action clip isolates only this presentation.
				The Server entity and the rest of the snapshot remain valid. */
				monster->Play_DefaultIdle(0.12f);
				iter->second.strCurrentClip = actor->presentationClips.idle;
			}
		}
		else if (WORLD_ENTITY_KIND::BOSS == iter->second.eKind)
		{
			/* Raw, un-gated read of this tick's own eAction -- independent of
			whether Apply_NetworkState/Apply_BossCombatState/Apply_BrokenArmorMask
			below succeed. Apply_BossCombatState rejects a snapshot whose
			iStateRevision matches the last-applied one but whose content differs
			(Is_SameBossCombatState mismatch); if the Server's own BossCombat
			sub-state does not bump iStateRevision on the kill tick, that check can
			reject every post-death snapshot forever, leaving CCombatHUDViewModel's
			own Apply_Boss()-gated eAction stuck non-DEAD for the rest of the
			encounter (RaidClear/Return button never trigger, no matter how long
			you wait) even though the boss really is dead. RaidClear only needs a
			reliable death edge, not the rest of the gated boss state, so it reads
			this flag instead of Get_Boss().eAction. */
			if (INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId &&
				WORLD_ENTITY_ACTION::DEAD == entity.eAction)
			{
				CCombatHUDViewModel::Get().Set_BossDeadRaw(true);
			}

			VALTAN_PRESENTATION_STATE latest{};
			latest.isValid = true;
			latest.iNetEntityId = entity.iNetEntityId;
			latest.iServerTick = snapshot.iServerTick;
			latest.eAction = entity.eAction;
			latest.strArchetypeId = iter->second.strArchetypeId;
			latest.strPatternId = entity.strPatternId;
			latest.strActionId = entity.strActionId;
			latest.iPatternSequence = entity.iPatternSequence;
			latest.iPatternStageIndex = entity.iPatternStageIndex;
			latest.iPatternTargetNetEntityId =
				entity.iPatternTargetNetEntityId;
			latest.iActionStartTick = entity.iActionStartTick;
			latest.BossCombat = entity.BossCombat;
			latest.vPosition = position;
			latest.fYawDegrees = entity.fYawDegrees;

			const std::shared_ptr<CValtan> valtan =
				iter->second.pValtan.lock();
			const bool_t bIsPrimary =
				INVALID_NET_ENTITY_ID == iter->second.iOwnerBossNetEntityId;
			if (nullptr == valtan)
			{
				allSucceeded = false;
				continue;
			}
			std::string RevisionStatus;
			if (!Ensure_ValtanPresentationRevision(
					iter->second, valtan,
					entity.PinnedDefinitionRevision,
					bIsPrimary, RevisionStatus))
			{
				/* Gameplay/HUD truth continues to advance while animation, Effect,
				   Sound and combat-object presentation stay isolated. */
				if (bIsPrimary)
				{
					if (WORLD_ENTITY_ACTION::DEAD == entity.eAction)
						deadBossOwners.push_back(entity.iNetEntityId);
					m_ValtanPresentationState = std::move(latest);
					CCombatHUDViewModel::Get().Apply_Boss(
						snapshot.iServerTick,
						iter->second.strArchetypeId,
						entity);
				}
				continue;
			}
			const CValtan::PATTERN_TARGET_SNAPSHOT_POSE PatternTargetPose =
				Resolve_ValtanPatternTargetSnapshotPose(
					std::span<const PLAYER_SNAPSHOT>(snapshot.Players.data(),
						snapshot.Players.size()),
					entity.iPatternTargetNetEntityId);
			if (!valtan->Apply_NetworkState(
				position,
				entity.fYawDegrees,
				entity.eAction,
				entity.strPatternId,
				entity.strActionId,
				snapshot.iServerTick,
				entity.iActionStartTick,
				entity.iPatternSequence,
				entity.iPatternStageIndex,
				PatternTargetPose) ||
				!valtan->Apply_BossCombatState(entity.BossCombat) ||
				!valtan->Apply_BrokenArmorMask(entity.iBrokenArmorMask))
			{
				allSucceeded = false;
			}
			else if (bIsPrimary)
			{
				if (WORLD_ENTITY_ACTION::DEAD == entity.eAction)
					deadBossOwners.push_back(entity.iNetEntityId);
				m_ValtanPresentationState = std::move(latest);
				CCombatHUDViewModel::Get().Apply_Boss(
					snapshot.iServerTick,
					iter->second.strArchetypeId,
					entity);
			}
		}
		else
		{
			allSucceeded = false;
		}
	}
	for (const BOSS_COMBAT_EVENT& event : snapshot.BossCombatEvents)
	{
		const auto boss = m_WorldEntities.find(event.iBossNetEntityId);
		if (boss == m_WorldEntities.end() ||
			WORLD_ENTITY_KIND::BOSS != boss->second.eKind)
		{
			allSucceeded = false;
			continue;
		}
		if (boss->second.bPresentationIsolated)
			continue;
		const std::shared_ptr<CValtan> valtan = boss->second.pValtan.lock();
		if (nullptr == valtan || !valtan->Apply_BossCombatEvent(event))
			allSucceeded = false;
	}
	{
		COMBAT_OBJECT_PRESENTATION_SINK sink{ *this };
		std::string status;
		if (!m_CombatObjectProjectionRuntime.Apply_Snapshot(
			snapshot.iServerTick, snapshot.CombatObjects, sink, status))
		{
			m_strPendingPresentationFailure = std::move(status);
			allSucceeded = false;
		}
		else if (status.find("failed") != std::string::npos)
		{
			m_strPendingPresentationFailure = std::move(status);
		}
	}
	for (const NET_ENTITY_ID owner : deadBossOwners)
		Remove_DependentBossPresentations(owner);
	for (const DAMAGE_EVENT& damageEvent : snapshot.DamageEvents)
	{
		if (!damageEvent.isOutgoing)
			continue;
		const auto hitEntity =
			m_WorldEntities.find(damageEvent.iTargetNetEntityId);
		if (hitEntity == m_WorldEntities.end())
			continue;
		if (WORLD_ENTITY_KIND::MONSTER == hitEntity->second.eKind)
		{
			if (const std::shared_ptr<CNpc> monster =
				hitEntity->second.pNpc.lock())
			{
				monster->Trigger_HitFlash();
				const MONSTER_ACTOR_ENTRY* actor = CActorCatalog::Find_Monster(
					hitEntity->second.strArchetypeId);
				const MONSTER_PRESENTATION_ACTION_KIND actionKind =
					hitEntity->second.MonsterActionState.eLastKind;
				if (nullptr != actor &&
					(MONSTER_PRESENTATION_ACTION_KIND::IDLE == actionKind ||
					 MONSTER_PRESENTATION_ACTION_KIND::CHASE == actionKind))
				{
					const std::string& returnClip =
						MONSTER_PRESENTATION_ACTION_KIND::CHASE == actionKind ?
						actor->presentationClips.chase :
						actor->presentationClips.idle;
					(void)monster->Play_TransientNetworkAction(
						actor->presentationClips.hit.c_str(),
						1.f,
						actor->hitDurationSeconds,
						returnClip.c_str(),
						true);
				}
			}
		}
		else if (WORLD_ENTITY_KIND::BOSS == hitEntity->second.eKind)
		{
			if (hitEntity->second.bPresentationIsolated)
				continue;
			if (const std::shared_ptr<CValtan> boss =
				hitEntity->second.pValtan.lock())
			{
				boss->Trigger_HitFlash();
			}
		}
	}
	CCombatHUDViewModel::Get().Apply_DamageEvents(
		snapshot.iServerTick,
		snapshot.DamageEvents);
	CCombatHUDViewModel::Get().Apply_EstherGauge(
		snapshot.iEstherGauge,
		snapshot.iEstherGaugeMaximum);

	m_iLastServerTick = snapshot.iServerTick;
	return allSucceeded;
}

Client::CClientReplication::CHARACTER_REPLACE_RESULT
Client::CClientReplication::Replace_CharacterClass(
	const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
	const NET_PLAYER_RECORD* currentRecord =
		m_Registry.Find_Record(snapshot.iNetEntityId);
	if (nullptr == currentRecord)
		return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;
	const NET_PLAYER_RECORD oldRecord = *currentRecord;
	OBJECT_HANDLE oldHandle{};
	if (!m_Registry.Find_Handle(snapshot.iNetEntityId, oldHandle))
		return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;
	const std::shared_ptr<CCharacter> oldCharacter =
		m_Registry.Resolve(oldHandle);
	if (nullptr == oldCharacter)
		return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;

	const bool_t isLocallyControlled = snapshot.iNetEntityId ==
		CNetworkManager::Get().Get_LocalEntityId();
	std::shared_ptr<CCharacter> stagedCharacter;
	if (!Create_Character(
		snapshot.eCharacterClass,
		oldRecord.strNickName,
		float3_t(snapshot.fPositionX, snapshot.fPositionY, snapshot.fPositionZ),
		snapshot.fYawDegrees,
		isLocallyControlled,
		stagedCharacter))
	{
		m_strPendingPresentationFailure =
			"Class change asset admission failed; the previous character remains active.";
		return CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE;
	}

	NET_PLAYER_RECORD newRecord = oldRecord;
	newRecord.eCharacterClass = snapshot.eCharacterClass;
	newRecord.fPositionX = snapshot.fPositionX;
	newRecord.fPositionY = snapshot.fPositionY;
	newRecord.fPositionZ = snapshot.fPositionZ;
	newRecord.fYawDegrees = snapshot.fYawDegrees;
	OBJECT_HANDLE newHandle{};
	if (!m_Registry.Replace(
		snapshot.iNetEntityId, newRecord, stagedCharacter, newHandle))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex, m_Desc.strPlayerLayerTag, stagedCharacter);
		m_strPendingPresentationFailure =
			"Class change registry commit failed; the previous character was kept.";
		return CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE;
	}

	if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
		m_Desc.iLayerLevelIndex, m_Desc.strPlayerLayerTag, oldCharacter)))
	{
		OBJECT_HANDLE restoredHandle{};
		const bool removedStaged = SUCCEEDED(
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strPlayerLayerTag, stagedCharacter));
		const bool restored = m_Registry.Replace(
			snapshot.iNetEntityId, oldRecord, oldCharacter, restoredHandle);
		if (!removedStaged || !restored)
			return CHARACTER_REPLACE_RESULT::FATAL_FAILURE;
		if (isLocallyControlled)
			m_LocalCharacterHandle = restoredHandle;
		m_strPendingPresentationFailure =
			"Class change layer commit failed; the previous character was restored.";
		return CHARACTER_REPLACE_RESULT::RECOVERED_FAILURE;
	}

	if (isLocallyControlled)
		m_LocalCharacterHandle = newHandle;
	return CHARACTER_REPLACE_RESULT::REPLACED;
}

void Client::CClientReplication::Stage_LocalCharacterClassReplacement(
	const LostArk::Shared::PLAYER_SNAPSHOT& Snapshot,
	const std::uint32_t iServerTick)
{
	DEFERRED_LOCAL_CHARACTER_CLASS_REPLACEMENT& Pending =
		m_DeferredLocalCharacterClassReplacement;
	if (!Pending.isPending ||
		Pending.Snapshot.iNetEntityId != Snapshot.iNetEntityId ||
		Pending.Snapshot.eCharacterClass != Snapshot.eCharacterClass)
	{
		Pending.iGeneration =
			m_iNextDeferredLocalCharacterClassReplacementGeneration++;
		if (0u == m_iNextDeferredLocalCharacterClassReplacementGeneration)
			m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
	}
	Pending.isPending = true;
	Pending.iServerTick = iServerTick;
	Pending.Snapshot = Snapshot;
}

void Client::CClientReplication::Clear_DeferredLocalCharacterClassReplacement()
{
	m_DeferredLocalCharacterClassReplacement = {};
}

void Client::CClientReplication::Update_DeathPresentations()
{
	for (auto iter = m_DeathPresentations.begin(); iter != m_DeathPresentations.end();)
	{
		const std::shared_ptr<CValtan> valtan = iter->lock();
		if (nullptr != valtan && !valtan->Is_NetworkDeathPresentationComplete())
		{
			++iter;
			continue;
		}
		if (nullptr != valtan)
		{
			CEffectPresentationService::Stop_BossOwner(valtan);
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strWorldEntityLayerTag, valtan);
		}
		iter = m_DeathPresentations.erase(iter);
	}
}

void Client::CClientReplication::Reset_World()
{
	//?묒냽???딄꼈?????꾩옱 registry???댁븘?덈뒗 character瑜?紐⑤몢 layer?먯꽌 ?쒓굅?섍퀬,
	//registry? local handle??珥덇린?뷀븳??
	//?뚭눼?먯뿉???몄텧?섏? ?딅뒗 ?댁쑀??留욌떎. ?꾩옱 engine? ?덈꺼 ?꾪솚 ??layer瑜?
	//癒쇱? ?쒓굅?섎?濡? level ?뚭눼?먯뿉???ㅼ떆 layer ?쒓굅瑜??쒕룄?섎㈃, ?대?
	//?щ씪吏?layer瑜?嫄대뱶由????덈떎.
	COMBAT_OBJECT_PRESENTATION_SINK combatObjectSink{ *this };
	m_CombatObjectProjectionRuntime.Reset(combatObjectSink);
	const std::vector<std::shared_ptr<CCharacter>> characters =
		m_Registry.Get_LiveObjects();

	for (const auto& character : characters)
	{
		if (nullptr == character)
			continue;

		//Layer?먯꽌 GameObject ?쒓굅?섍린
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strPlayerLayerTag,
			character);
	}
	for (const auto& [entityId, presentation] : m_WorldEntities)
	{
		(void)entityId;
		if (const std::shared_ptr<CValtan> valtan = presentation.pValtan.lock())
		{
			CEffectPresentationService::Stop_BossOwner(valtan);
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				valtan);
		}
		if (const std::shared_ptr<CNpc> npc = presentation.pNpc.lock())
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex,
				m_Desc.strWorldEntityLayerTag,
				npc);
		}
	}
	m_WorldEntities.clear();
	m_PrimaryValtanJoinedPresentationFreshness.Reject(
		"Replicated world reset; the next primary Valtan spawn must reload authoring sources for its exact revision.");
	m_PrimaryValtanCombatObjectSoundFreshness.Reject(
		"Replicated world reset; the next primary Valtan spawn must reload authoring sources for its exact revision.");
	for (const std::weak_ptr<CValtan>& deathPresentation : m_DeathPresentations)
	{
		if (const std::shared_ptr<CValtan> valtan = deathPresentation.lock())
		{
			CEffectPresentationService::Stop_BossOwner(valtan);
			CGameInstance::Get().Remove_GameObject_from_Layer(
				m_Desc.iLayerLevelIndex, m_Desc.strWorldEntityLayerTag, valtan);
		}
	}
	m_DeathPresentations.clear();
	m_Registry.Reset();
	m_PlayerAttachments.clear();
	m_LocalCharacterHandle = {};
	Clear_DeferredLocalCharacterClassReplacement();
	m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
	m_iLastServerTick = 0;
	m_ValtanPresentationState = {};
	m_WorldDestructionProjectionRuntime.Reset();
	m_WorldDestructionLiveEvents.clear();
	m_EncounterPropState = {};
	m_InventoryState = {};
	m_PlayerHealth.Reset();
	m_PartyRoster = {};
	m_hasPendingPartyInvite = false;
	m_PendingPartyInvite = {};
	m_hasPendingPartyTransferResult = false;
	m_PendingPartyTransferResult = {};
	m_ChatBubblesByNetEntityId.clear();
	++m_iWorldDestructionPresentationGeneration;
	if (0u == m_iWorldDestructionPresentationGeneration)
		++m_iWorldDestructionPresentationGeneration;
	m_hasFatalWorldDestructionFailure = false;
	m_strPendingPresentationFailure.clear();
	CCombatHUDViewModel::Get().Reset_RuntimeState();
}
