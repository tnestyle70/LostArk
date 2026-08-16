#include "ClientReplication.h"

#include "ActionPresentationTimeline.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "NetworkManager.h"
#include "MonsterPresentationAssetService.h"
#include "Npc.h"
#include "NpcPlacementPresentationService.h"
#include "NpcPresentationAssetService.h"
#include "PlayableCharacterAssetService.h"
#include "Transform.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"
#include "DeployPropRuntime.h"

#include <algorithm>

namespace
{
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
}

bool Client::CClientReplication::Initialize(const DESC& desc)
{
	//Layer ?뺣낫媛 ?좏븳?쒖? 寃?ы븯怨? ?ㅼ젙????ν븳??
	//?꾩옱 network ?곌껐 ?곹깭??湲곗뼲???먯뼱, ?댄썑 ?곌껐???딄꼈?붿? 媛먯??????덇쾶 ?쒕떎.
	if (nullptr == desc.pDevice ||
		nullptr == desc.pContext ||
		desc.strPlayerLayerTag.empty() ||
		desc.strWorldEntityLayerTag.empty() ||
		((nullptr == desc.pDeployPropRuntime) !=
			(nullptr == desc.pWorldDestructionProjection)) ||
		(nullptr != desc.pWorldDestructionProjection &&
			!desc.pWorldDestructionProjection->Is_Ready()) ||
		!CCombatHUDViewModel::Get().Initialize_Definitions())
		return false;

	m_Desc = desc;
	m_isInitialized = true;
	m_hasPendingConnectionLoss = false;
	m_hasFatalWorldDestructionFailure = false;
	m_WorldDestructionProjectionRuntime.Reset();
	m_wasConnected =
		CNetworkManager::Get().Is_Connected();

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
			allSucceeded =
				Apply_WorldEntitySpawn(event.WorldEntitySpawned) &&
				allSucceeded;
			break;

		case CLIENT_REPLICATION_EVENT_TYPE::WORLD_ENTITY_DESPAWNED:
			allSucceeded =
				Apply_WorldEntityDespawn(event.WorldEntityDespawned) &&
				allSucceeded;
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
		}
	}

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
	desc.pNavigationPrototypeTag = nullptr;
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

	if (m_LocalCharacterHandle.iSlotIndex == handle.iSlotIndex &&
		m_LocalCharacterHandle.iGeneration == handle.iGeneration)
	{
		m_LocalCharacterHandle = {};
	}

	return true;
}

bool Client::CClientReplication::Apply_WorldEntitySpawn(
	const LostArk::Shared::S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
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
			existing->second.fCollisionRadius == spawned.fCollisionRadius;
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
		desc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		const std::string* pIdleClipOverride =
			CNpcPlacementPresentationService::Find_IdleClip(
				m_Desc.iPrototypeLevelIndex, spawned.strPlacementId);
		desc.pIdleClip = nullptr != pIdleClipOverride ?
			pIdleClipOverride->c_str() : actor->idleClip.c_str();
		desc.vPosition = float3_t(
			spawned.fPositionX,
			spawned.fPositionY,
			spawned.fPositionZ);
		desc.fYawDegree = spawned.fYawDegrees;
		desc.fCollisionRadius = spawned.fCollisionRadius;

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
		presentation.strArchetypeId = spawned.strArchetypeId;
		presentation.strEncounterId = spawned.strEncounterId;
		presentation.fCollisionRadius = spawned.fCollisionRadius;
		presentation.pNpc = npc;
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
		presentation.strArchetypeId = spawned.strArchetypeId;
		presentation.strEncounterId = spawned.strEncounterId;
		presentation.strCurrentClip = actor->presentationClips.idle;
		presentation.fCollisionRadius = spawned.fCollisionRadius;
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
		m_Desc.iPrototypeLevelIndex)))
	{
		return false;
	}

	CValtan::VALTAN_DESC desc{};
	desc.iPrototypeLevelIndex = m_Desc.iPrototypeLevelIndex;
	desc.vPosition = float3_t(
		spawned.fPositionX,
		spawned.fPositionY,
		spawned.fPositionZ);
	desc.isServerAuthoritative = true;
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
		{}, {}, 0u, 0u, 0u, 0u))
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			gameObject);
		return false;
	}

	WORLD_ENTITY_PRESENTATION presentation{};
	presentation.eKind = spawned.eKind;
	presentation.strArchetypeId = spawned.strArchetypeId;
	presentation.strEncounterId = spawned.strEncounterId;
	presentation.fCollisionRadius = spawned.fCollisionRadius;
	presentation.pValtan = valtan;
#ifdef _DEBUG
	valtan->Set_CombatColliderDebugVisible(
		m_isCombatColliderDebugVisible);
#endif
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
	return inserted;
}

bool Client::CClientReplication::Apply_WorldEntityDespawn(
	const LostArk::Shared::S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == despawned.iNetEntityId)
		return false;
	const auto iter = m_WorldEntities.find(despawned.iNetEntityId);
	if (m_WorldEntities.end() == iter)
		return true;

	if (const std::shared_ptr<CNpc> npc = iter->second.pNpc.lock())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			npc);
	}
	if (const std::shared_ptr<CValtan> valtan = iter->second.pValtan.lock())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_Desc.iLayerLevelIndex,
			m_Desc.strWorldEntityLayerTag,
			valtan);
	}
	if (m_ValtanPresentationState.isValid &&
		m_ValtanPresentationState.iNetEntityId == despawned.iNetEntityId)
	{
		m_ValtanPresentationState = {};
	}
	m_WorldEntities.erase(iter);
	return true;
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

	bool allSucceeded = true;

	for (const PLAYER_SNAPSHOT& player : snapshot.Players)
	{
		const NET_PLAYER_RECORD* record =
			m_Registry.Find_Record(player.iNetEntityId);
		if (nullptr != record &&
			record->eCharacterClass != player.eCharacterClass)
		{
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
				player.iComboStage))
		{
			allSucceeded = false;
		}
		character->Apply_NetworkStance(player.eStance);
		if (player.iNetEntityId ==
			CNetworkManager::Get().Get_LocalEntityId())
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
	for (const WORLD_ENTITY_SNAPSHOT& entity : snapshot.Entities)
	{
		const auto iter = m_WorldEntities.find(entity.iNetEntityId);
		if (iter == m_WorldEntities.end())
		{
			allSucceeded = false;
			continue;
		}
		const float3_t position(
			entity.fPositionX,
			entity.fPositionY,
			entity.fPositionZ);
		if (WORLD_ENTITY_KIND::NPC == iter->second.eKind)
		{
			const std::shared_ptr<CNpc> npc = iter->second.pNpc.lock();
			if (nullptr == npc ||
				!npc->Apply_NetworkState(position, entity.fYawDegrees))
			{
				allSucceeded = false;
				continue;
			}
			/* Server-driven NPC actions (raid Esther summons). An action the
			catalog maps plays once; anything else stands in the idle clip, so
			a plain placement NPC without actionClips never switches at all. */
			const NPC_ACTOR_ENTRY* actor =
				CActorCatalog::Find_Npc(iter->second.strArchetypeId);
			if (nullptr == actor || actor->actionClips.empty())
				continue;
			const std::string* clip = &actor->idleClip;
			bool_t loop = true;
			const auto actionClip =
				actor->actionClips.find(entity.strActionId);
			if (actionClip != actor->actionClips.end())
			{
				clip = &actionClip->second;
				loop = false;
			}
			if (iter->second.strCurrentClip != *clip)
			{
				if (!npc->Set_Animation(clip->c_str(), loop))
					allSucceeded = false;
				else
					iter->second.strCurrentClip = *clip;
			}
		}
		else if (WORLD_ENTITY_KIND::MONSTER == iter->second.eKind)
		{
			const std::shared_ptr<CNpc> monster = iter->second.pNpc.lock();
			const MONSTER_ACTOR_ENTRY* actor =
				CActorCatalog::Find_Monster(iter->second.strArchetypeId);
			if (nullptr == monster || nullptr == actor ||
				!monster->Apply_NetworkState(position, entity.fYawDegrees))
			{
				allSucceeded = false;
				continue;
			}

			const std::string* clip = &actor->presentationClips.idle;
			bool_t loop = true;
			if (WORLD_ENTITY_ACTION::CHASE == entity.eAction)
			{
				clip = &actor->presentationClips.chase;
			}
			else if (WORLD_ENTITY_ACTION::PATTERN_WINDUP == entity.eAction ||
				WORLD_ENTITY_ACTION::PATTERN_ACTIVE == entity.eAction ||
				WORLD_ENTITY_ACTION::PATTERN_RECOVERY == entity.eAction)
			{
				clip = &actor->presentationClips.attack;
				loop = false;
			}
			else if (WORLD_ENTITY_ACTION::DEAD == entity.eAction)
			{
				clip = &actor->presentationClips.dead;
				loop = false;
			}
			if (iter->second.strCurrentClip != *clip)
			{
				if (!monster->Set_Animation(clip->c_str(), loop))
					allSucceeded = false;
				else
					iter->second.strCurrentClip = *clip;
			}
		}
		else if (WORLD_ENTITY_KIND::BOSS == iter->second.eKind)
		{
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
			latest.iActionStartTick = entity.iActionStartTick;
			latest.vPosition = position;
			latest.fYawDegrees = entity.fYawDegrees;
			m_ValtanPresentationState = std::move(latest);

			const std::shared_ptr<CValtan> valtan =
				iter->second.pValtan.lock();
			if (nullptr == valtan || !valtan->Apply_NetworkState(
				position,
				entity.fYawDegrees,
				entity.eAction,
				entity.strPatternId,
				entity.strActionId,
				snapshot.iServerTick,
				entity.iActionStartTick,
				entity.iPatternSequence,
				entity.iPatternStageIndex))
			{
				allSucceeded = false;
			}
		}
		else
		{
			allSucceeded = false;
		}
		if (iter->second.eKind == WORLD_ENTITY_KIND::BOSS)
		{
			CCombatHUDViewModel::Get().Apply_Boss(
				iter->second.strArchetypeId,
				entity);
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

void Client::CClientReplication::Reset_World()
{
	//?묒냽???딄꼈?????꾩옱 registry???댁븘?덈뒗 character瑜?紐⑤몢 layer?먯꽌 ?쒓굅?섍퀬,
	//registry? local handle??珥덇린?뷀븳??
	//?뚭눼?먯뿉???몄텧?섏? ?딅뒗 ?댁쑀??留욌떎. ?꾩옱 engine? ?덈꺼 ?꾪솚 ??layer瑜?
	//癒쇱? ?쒓굅?섎?濡? level ?뚭눼?먯뿉???ㅼ떆 layer ?쒓굅瑜??쒕룄?섎㈃, ?대?
	//?щ씪吏?layer瑜?嫄대뱶由????덈떎.
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
	m_Registry.Reset();
	m_LocalCharacterHandle = {};
	m_iLastServerTick = 0;
	m_ValtanPresentationState = {};
	m_WorldDestructionProjectionRuntime.Reset();
	m_WorldDestructionLiveEvents.clear();
	m_EncounterPropState = {};
	++m_iWorldDestructionPresentationGeneration;
	if (0u == m_iWorldDestructionPresentationGeneration)
		++m_iWorldDestructionPresentationGeneration;
	m_hasFatalWorldDestructionFailure = false;
	m_strPendingPresentationFailure.clear();
	CCombatHUDViewModel::Get().Reset_RuntimeState();
}
