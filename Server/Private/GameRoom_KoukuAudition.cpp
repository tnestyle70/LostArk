#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_KoukuSaydonArenaBoss(
	const std::string& placementId,
	const std::string& archetypeId)
{
	const auto found = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[this, &placementId, &archetypeId](const SERVER_WORLD_ENTITY& entity)
		{
			return CKoukuSaydonBrain::Is_ArenaBoss(m_eWorldId, entity) &&
				entity.strPlacementId == placementId &&
				entity.strArchetypeId == archetypeId;
		});
	return m_WorldEntities.end() == found ? nullptr : &*found;
}

LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
LostArk::Server::CGameRoom::Evaluate_KoukuSaydonPatternAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
	LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult)
{
	using namespace LostArk::Shared;
	outResult = {};
	outResult.iRequestSequence = request.iRequestSequence;
	outResult.eOperation = request.eOperation;
	outResult.Scope = request.Scope;
	outResult.strRequestedPatternId = request.strPatternId;
	outResult.strBundleId = request.strBundleId;
	outResult.iExpectedRunEpoch = request.iExpectedRunEpoch;
	outResult.PinnedGameplayRevision = m_GameplayCatalog.Get_ActiveRevision();
	outResult.iPinnedSourceRevision =
		CKoukuSaydonBrain::Resolve_ProductSourceRevision(m_GameplayCatalog.Active());
#ifdef _DEBUG
	if (m_pKoukuPublishedProductGeneration &&
		m_pKoukuPublishedProductGeneration->Has_SameNonKoukuGameplay(m_GameplayCatalog.Active()))
		outResult.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(
			*m_pKoukuPublishedProductGeneration);
#endif

	const auto reject = [&outResult](
		const KOUKUSAYDON_PATTERN_AUDITION_RESULT result,
		std::string reason)
	{
		outResult.eResult = result;
		outResult.iRoomAuditionEpoch = 0u;
		outResult.iBossNetEntityId = INVALID_NET_ENTITY_ID;
		outResult.strResolvedPatternId.clear();
		outResult.iPatternSequence = 0u;
		outResult.iStageIndex = 0u;
		outResult.strReason = std::move(reason);
		return result;
	};

#ifndef _DEBUG
	(void)sessionId;
	return reject(KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_RELEASE_BUILD,
		"KoukuSaydon pattern audition is available only in Debug builds");
#else
	const auto sameRequest = [](const auto& left, const auto& right)
	{
		return left.iRequestSequence == right.iRequestSequence &&
			left.eOperation == right.eOperation &&
			left.Scope.eWorldId == right.Scope.eWorldId &&
			left.Scope.strEncounterId == right.Scope.strEncounterId &&
			left.Scope.strBossPlacementId == right.Scope.strBossPlacementId &&
			left.Scope.strBossArchetypeId == right.Scope.strBossArchetypeId &&
			left.Scope.ExpectedGameplayRevision ==
				right.Scope.ExpectedGameplayRevision &&
			left.Scope.iExpectedSourceRevision ==
				right.Scope.iExpectedSourceRevision &&
			left.strPatternId == right.strPatternId && left.strBundleId == right.strBundleId &&
			left.iExpectedRunEpoch == right.iExpectedRunEpoch && left.Scope.strGateId == right.Scope.strGateId;
	};
	const auto previous =
		m_KoukuSaydonPatternAuditionReceiptBySessionId.find(sessionId);
	if (m_KoukuSaydonPatternAuditionReceiptBySessionId.end() != previous &&
		request.iRequestSequence <= previous->second.Request.iRequestSequence)
	{
		if (sameRequest(request, previous->second.Request))
		{
			outResult = previous->second.Result;
			if (KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED ==
				previous->second.Result.eResult)
			{
				outResult.eResult =
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::DUPLICATE_IGNORED;
				if (previous->second.LastLifecycle)
				{
					const auto& lifecycle = *previous->second.LastLifecycle;
					outResult.strResolvedPatternId = lifecycle.strPatternId;
					outResult.iPatternSequence = lifecycle.iPatternSequence;
					outResult.iStageIndex = lifecycle.iStageIndex;
				}
			}
			return outResult.eResult;
		}
		return reject(
			KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST,
			"KoukuSaydon audition request sequence is stale or reused");
	}


	using OP = KOUKUSAYDON_PATTERN_AUDITION_OPERATION;
	using RESULT = KOUKUSAYDON_PATTERN_AUDITION_RESULT;
	using PHASE = KOUKUSAYDON_PATTERN_AUDITION_PHASE;
	const bool bundleRequest = request.eOperation == OP::PLAY_BUNDLE || request.eOperation == OP::RESTART_BUNDLE;
	const bool restart = request.eOperation == OP::RESTART_BUNDLE;
	const bool running = m_KoukuSaydonPatternAudition.ePhase != PHASE::INACTIVE;
	CPacketWriter shape;
	if (!Write_Message(shape, request) || m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA || request.Scope.eWorldId != m_eWorldId ||
		request.Scope.strEncounterId != KOUKUSAYDON_G1_ENCOUNTER_ID)
		return reject(RESULT::REJECTED_SCOPE_MISMATCH, "KoukuSaydon audition world/session/selection scope is invalid");
	const bool controlsRun = request.eOperation == OP::STOP || restart;
	const auto expectedGameplay = controlsRun && running ?
		m_KoukuSaydonPatternAudition.PinnedGameplayRevision : m_GameplayCatalog.Get_ActiveRevision();
	if (request.Scope.ExpectedGameplayRevision != expectedGameplay)
		return reject(RESULT::REJECTED_REVISION_MISMATCH, "KoukuSaydon audition expected gameplay revision is not active");
	if (controlsRun)
	{
		if (!running || m_KoukuSaydonPatternAudition.iOwnerSessionId != sessionId ||
			request.iExpectedRunEpoch != m_KoukuSaydonPatternAudition.iRoomAuditionEpoch ||
			request.strBundleId != m_KoukuSaydonPatternAudition.Request.strBundleId ||
			request.Scope.strGateId != m_KoukuSaydonPatternAudition.Request.Scope.strGateId)
			return reject(RESULT::REJECTED_STALE_REQUEST, "Stop/restart does not own the exact active run epoch");
		outResult.PinnedGameplayRevision = m_KoukuSaydonPatternAudition.PinnedGameplayRevision;
		outResult.iPinnedSourceRevision = m_KoukuSaydonPatternAudition.iPinnedSourceRevision;
		if (request.Scope.iExpectedSourceRevision != outResult.iPinnedSourceRevision)
			return reject(RESULT::REJECTED_SOURCE_REVISION_MISMATCH,
				"Stop/restart must name the original run Product revision; stop that run before playing a newly published Product");
		if (request.eOperation == OP::STOP)
		{
			const auto& member = m_KoukuSaydonPatternAudition.Members.front();
			outResult.eResult = RESULT::STOPPED;
			outResult.iRoomAuditionEpoch = m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
			outResult.iCommonStartTick = m_KoukuSaydonPatternAudition.iCommonStartTick;
			outResult.iBossNetEntityId = member.iBossEntityId;
			outResult.strResolvedPatternId = member.PatternIds[member.iPatternIndex];
			outResult.iPatternSequence = member.iPatternSequence;
			Clear_KoukuSaydonPatternAudition();
			m_KoukuSaydonPatternAuditionReceiptBySessionId[sessionId] = {request, outResult, std::nullopt};
			return outResult.eResult;
		}
	}
	else if (running) return reject(RESULT::REJECTED_BUSY, "KoukuSaydon room already owns an audition run");

	std::shared_ptr<const CGameplayCatalog> productGeneration = restart ?
		m_KoukuSaydonPatternAudition.pProductGeneration : m_GameplayCatalog.Get_ActiveGeneration();
	if (!restart && m_pKoukuPublishedProductGeneration &&
		m_pKoukuPublishedProductGeneration->Has_SameNonKoukuGameplay(m_GameplayCatalog.Active()))
		productGeneration = m_pKoukuPublishedProductGeneration;
	if (!productGeneration) productGeneration = m_GameplayCatalog.Get_ActiveGeneration();
	if (!restart && request.Scope.iExpectedSourceRevision !=
		CKoukuSaydonBrain::Resolve_ProductSourceRevision(*productGeneration))
	{
		auto candidate = std::make_shared<CGameplayCatalog>();
		if (!candidate->Load_PublishedKoukuProduct(m_GameplayCatalog.Active()))
			return reject(RESULT::REJECTED_SOURCE_REVISION_MISMATCH,
				"KoukuSaydon published Product was not admitted: " + candidate->Get_Status());
		const auto publishedSource = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*candidate);
		if (publishedSource != request.Scope.iExpectedSourceRevision ||
			publishedSource < outResult.iPinnedSourceRevision)
			return reject(RESULT::REJECTED_SOURCE_REVISION_MISMATCH,
				"KoukuSaydon Product revision mismatch: requested " +
				std::to_string(request.Scope.iExpectedSourceRevision) + ", published " +
				std::to_string(publishedSource) + ". Publish the saved PRODUCT and reload the inventory.");
		if (!candidate->Has_SameNonKoukuGameplay(m_GameplayCatalog.Active()))
			return reject(RESULT::REJECTED_REVISION_MISMATCH,
				"KoukuSaydon reload cannot change player/boss balance or other encounters; restart Server to apply those separately published changes");
		productGeneration = std::move(candidate);
	}
	if (nullptr == productGeneration || request.Scope.iExpectedSourceRevision !=
		CKoukuSaydonBrain::Resolve_ProductSourceRevision(*productGeneration))
		return reject(RESULT::REJECTED_SOURCE_REVISION_MISMATCH, "KoukuSaydon exact run Product revision is unavailable");
	const auto& catalog = *productGeneration;
	const auto* definitions = catalog.Find_BossPatterns(request.Scope.strEncounterId);
	if (!definitions) return reject(RESULT::REJECTED_UNKNOWN_PATTERN, "KoukuSaydon Product patterns are unavailable");
	const auto findPattern = [definitions](const std::string& id) -> const BOSS_PATTERN_DEFINITION*
	{
		const auto found = std::find_if(definitions->begin(), definitions->end(), [&](const auto& item) { return item.strPatternId == id; });
		return found == definitions->end() ? nullptr : &*found;
	};
	const auto gateOwns = [](const std::string& gate, const std::string& placement)
	{
		return (gate == "GATE1" && (placement == "boss.kakulsaydon.g1.kouku" || placement == "boss.kakulsaydon.g1.saydon")) ||
			(gate == "GATE2" && (placement == "boss.kakulsaydon.g2.kouku" || placement == "boss.kakulsaydon.g2.big-saydon")) ||
			(gate == "GATE3" && placement == "boss.kakulsaydon.g3.saydon") || (gate == "BINGO" && placement == "boss.kakulsaydon.bingo.saydon");
	};
	KOUKUSAYDON_PATTERN_AUDITION_STATE staged;
	staged.ePhase = PHASE::PENDING; staged.iOwnerSessionId = sessionId; staged.Request = request;
	staged.iRoomAuditionEpoch = m_iNextKoukuSaydonPatternAuditionEpoch;
	staged.PinnedGameplayRevision = request.Scope.ExpectedGameplayRevision; staged.iPinnedSourceRevision = request.Scope.iExpectedSourceRevision;
	staged.pProductGeneration = std::move(productGeneration);
	staged.iCommonStartTick = m_iServerTick == 0u ? 1u : Add_ServerTicksSkippingReservedZero(m_iServerTick, 1u);
	std::vector<BOSS_PATTERN_BUNDLE_MEMBER> requestedMembers;
	if (bundleRequest)
	{
		const auto* bundle = catalog.Find_BossPatternBundle(request.strBundleId);
		if (!bundle || bundle->strEncounterId != request.Scope.strEncounterId || bundle->strGateId != request.Scope.strGateId || bundle->Members.empty())
			return reject(RESULT::REJECTED_UNKNOWN_PATTERN, "KoukuSaydon bundle is not admitted in the requested Gate");
		requestedMembers = bundle->Members;
	}
	else requestedMembers.push_back({"single", request.strPatternId, request.Scope.strBossPlacementId, 0u});
	std::set<NET_ENTITY_ID> reservedBosses;
	std::set<std::string> ownedWorldInstances;
	std::size_t playerModeOwners = 0u;
	for (const auto& sourceMember : requestedMembers)
	{
		const auto* placement = Find_Placement(sourceMember.strTargetBossPlacementId);
		if (!placement || placement->eKind != WORLD_BOOTSTRAP_KIND::BOSS || placement->strEncounterId != request.Scope.strEncounterId ||
			(bundleRequest && !gateOwns(request.Scope.strGateId, placement->strPlacementId)) ||
			(!bundleRequest && placement->strArchetypeId != request.Scope.strBossArchetypeId))
			return reject(RESULT::REJECTED_SCOPE_MISMATCH, "KoukuSaydon target placement/Gate/archetype does not match Product");
		auto* boss = Find_KoukuSaydonArenaBoss(placement->strPlacementId, placement->strArchetypeId);
		if (!boss) return reject(RESULT::REJECTED_NO_BOSS, "KoukuSaydon target is not spawned: " + placement->strPlacementId);
		if (!boss->iCurrentHp || boss->eAction == SERVER_ENTITY_ACTION::DEAD)
			return reject(RESULT::REJECTED_BOSS_DEAD, "KoukuSaydon target is dead: " + placement->strPlacementId);
		if (!reservedBosses.insert(boss->iNetEntityId).second || (!boss->strPatternId.empty() && !(restart && Find_KoukuAuditionMember(boss->iNetEntityId))))
			return reject(RESULT::REJECTED_BUSY, "KoukuSaydon target is duplicated or already owns an occurrence");
		KOUKUSAYDON_PATTERN_AUDITION_MEMBER member;
		member.strMemberId = sourceMember.strMemberId; member.iBossEntityId = boss->iNetEntityId;
		member.iScheduledStartTick = Add_ServerTicksSkippingReservedZero(staged.iCommonStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(sourceMember.iStartOffsetMs));
		member.iNextStartTick = member.iScheduledStartTick;
		std::string status;
		if (request.eOperation == OP::PLAY_ALL)
		{
			const auto* sequence = catalog.Find_BossPatternSequence(request.Scope.strEncounterId);
			if (!sequence || !CKoukuSaydonBrain::Select_AnimationOnlySequence(*definitions, *sequence,
				boss->strArchetypeId, member.PatternIds, member.TransitionTicks, status,
				request.Scope.strGateId, placement->strPlacementId))
				return reject(RESULT::REJECTED_NO_PRODUCT_SEQUENCE, status.empty() ? "KoukuSaydon sequential Product is unavailable" : status);
		}
		else member.PatternIds.push_back(sourceMember.strPatternId);
		std::vector<std::string> toCheck = member.PatternIds;
		std::set<std::string> visited, memberWorlds;
		for (std::size_t index = 0; index < toCheck.size(); ++index)
		{
			if (!visited.insert(toCheck[index]).second) continue;
			const auto* pattern = findPattern(toCheck[index]);
			if (!pattern) return reject(RESULT::REJECTED_UNKNOWN_PATTERN, "KoukuSaydon member/follow-up pattern is missing: " + toCheck[index]);
			if (!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(*pattern, status)) return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, status);
			const bool admitsBody = pattern->AuditionBossArchetypeIds.empty() ? boss->strArchetypeId == KOUKUSAYDON_G1_BOSS_ARCHETYPE_ID :
				std::find(pattern->AuditionBossArchetypeIds.begin(), pattern->AuditionBossArchetypeIds.end(), boss->strArchetypeId) != pattern->AuditionBossArchetypeIds.end();
			if (!admitsBody || (bundleRequest && (pattern->strGateId != request.Scope.strGateId || pattern->strTargetBossPlacementId != placement->strPlacementId)) ||
				(!pattern->strTargetBossPlacementId.empty() && pattern->strTargetBossPlacementId != placement->strPlacementId) ||
				(!request.Scope.strGateId.empty() && !pattern->strGateId.empty() && request.Scope.strGateId != pattern->strGateId))
				return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, "KoukuSaydon member/follow-up body or Gate target differs from Product");
			if (!Is_KoukuBossMotionNavigable(*pattern, m_ServerNavigation))
				return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, "KoukuSaydon Boss Motion leaves active navigation");
			if (pattern->bResetBossToSpawn && (!m_ServerNavigation.Is_PointWalkableExact(boss->fSpawnPositionX, boss->fSpawnPositionZ) || !std::isfinite(boss->fSpawnPositionY)))
				return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, "KoukuSaydon spawn reset is not on active navigation");
			for (const auto& trigger : pattern->MechanicTriggers)
				member.bOwnsPlayerMode = member.bOwnsPlayerMode || trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::HUD_ENTER ||
					trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_HIDE_NEXT || trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_ENTER;
			for (const auto& window : pattern->LogicWindows)
			{
				member.bOwnsPlayerMode = member.bOwnsPlayerMode || window.eKind == BOSS_PATTERN_LOGIC_KIND::POSE_INPUT || window.eKind == BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH;
				for (const auto* outcomes : {&window.OnSuccess, &window.OnFail, &window.OnTimeout}) for (const auto& result : *outcomes)
				{
					member.bOwnsPlayerMode = member.bOwnsPlayerMode || result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CLOWN_TRANSFORM;
					if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN) toCheck.push_back(result.strPatternId);
				}
			}
			for (const auto& world : pattern->WorldSequences) memberWorlds.insert(world.strInstanceId);
		}
		if (member.bOwnsPlayerMode && ++playerModeOwners > 1u)
			return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, "Bundle members conflict over shared player/HUD mode");
		for (const auto& world : memberWorlds) if (!ownedWorldInstances.insert(world).second)
			return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, "Bundle members target the same World sequence instance");
		// Preflight the actual Begin contract on a copy; no reset or broadcast occurs before all members pass.
		auto candidate = *boss;
		if (restart && !candidate.strPatternId.empty()) m_KoukuSaydonBrain.Abort_Pattern(candidate, m_iServerTick);
		if (!m_KoukuSaydonBrain.Begin_Pattern(candidate, *findPattern(member.PatternIds.front()), staged.PinnedGameplayRevision, member.iNextStartTick, status))
			return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, status);
		staged.Members.push_back(std::move(member));
	}
	// Child Scene windows share one global output. The publisher additionally checks Camera/common-lane ownership.
	for (std::size_t i = 0; i < requestedMembers.size(); ++i) for (std::size_t j = 0; j < i; ++j)
	{
		const auto* left = findPattern(staged.Members[i].PatternIds.front());
		const auto* right = findPattern(staged.Members[j].PatternIds.front());
		const double leftOffset = CKoukuSaydonLogicRuntime::Ticks_FromMs(requestedMembers[i].iStartOffsetMs) * (1000.0 / 30.0);
		const double rightOffset = CKoukuSaydonLogicRuntime::Ticks_FromMs(requestedMembers[j].iStartOffsetMs) * (1000.0 / 30.0);
		for (const auto& a : left->SceneProfiles) for (const auto& b : right->SceneProfiles)
			if ((std::max)(leftOffset + a.iStartMs, rightOffset + b.iStartMs) < (std::min)(leftOffset + a.iStartMs + a.iDurationMs, rightOffset + b.iStartMs + b.iDurationMs))
				return reject(RESULT::REJECTED_UNSUPPORTED_PATTERN, "Bundle members have overlapping Scene Profile ownership");
	}
	if (restart) Clear_KoukuSaydonPatternAudition();
	m_KoukuSaydonPatternAudition = std::move(staged);
	m_pKoukuPublishedProductGeneration = m_KoukuSaydonPatternAudition.pProductGeneration;
	m_iNextKoukuSaydonPatternAuditionEpoch = Add_ServerTicksSkippingReservedZero(m_iNextKoukuSaydonPatternAuditionEpoch, 1u);
	const auto& first = m_KoukuSaydonPatternAudition.Members.front();
	outResult.eResult = RESULT::QUEUED; outResult.iRoomAuditionEpoch = m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
	outResult.PinnedGameplayRevision = m_KoukuSaydonPatternAudition.PinnedGameplayRevision;
	outResult.iPinnedSourceRevision = m_KoukuSaydonPatternAudition.iPinnedSourceRevision;
	outResult.iCommonStartTick = m_KoukuSaydonPatternAudition.iCommonStartTick;
	outResult.iBossNetEntityId = first.iBossEntityId; outResult.strResolvedPatternId = first.PatternIds.front();
	m_KoukuSaydonPatternAuditionReceiptBySessionId[sessionId] = {request, outResult, std::nullopt};
	for (const auto& member : m_KoukuSaydonPatternAudition.Members)
		Queue_KoukuSaydonPatternAuditionLifecycle(member.PatternIds.front(), 0u, 0u, KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING, {}, member.iBossEntityId);
	Broadcast_KoukuBundleState(KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING);
	return outResult.eResult;
#endif
}

void LostArk::Server::CGameRoom::Handle_KoukuSaydonPatternAudition(
	const SESSION_ID sessionId,
	const LostArk::Shared::
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result{};
	(void)Evaluate_KoukuSaydonPatternAudition(sessionId, request, result);
	if (!Send_KoukuSaydonPatternAuditionResult(session, result))
		session->Request_Close();
}

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Queue_KoukuSaydonPatternAuditionLifecycle(
	const std::string& patternId,
	const std::uint32_t patternSequence,
	const std::uint32_t stageIndex,
	const LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
	std::string reason, const LostArk::Shared::NET_ENTITY_ID bossId)
{
	using namespace LostArk::Shared;
	if (INVALID_SESSION_ID == m_KoukuSaydonPatternAudition.iOwnerSessionId)
		return;
	S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE message{};
	message.iRequestSequence =
		m_KoukuSaydonPatternAudition.Request.iRequestSequence;
	message.eOperation = m_KoukuSaydonPatternAudition.Request.eOperation;
	message.Scope = m_KoukuSaydonPatternAudition.Request.Scope;
	message.iRoomAuditionEpoch =
		m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
	const auto* member = Find_KoukuAuditionMember(bossId);
	message.iBossNetEntityId = member ? member->iBossEntityId : m_KoukuSaydonPatternAudition.Members.front().iBossEntityId;
	message.strMemberId = member ? member->strMemberId : std::string{};
	message.strBundleId = m_KoukuSaydonPatternAudition.Request.strBundleId;
	message.iCommonStartTick = m_KoukuSaydonPatternAudition.iCommonStartTick;
	message.strPatternId = patternId;
	message.iPatternSequence = patternSequence;
	message.iStageIndex = stageIndex;
	message.eState = state;
	message.PinnedGameplayRevision =
		m_KoukuSaydonPatternAudition.PinnedGameplayRevision;
	message.iPinnedSourceRevision =
		m_KoukuSaydonPatternAudition.iPinnedSourceRevision;
	message.strReason = std::move(reason);
	m_PendingKoukuSaydonPatternAuditionLifecycle.push_back(
		{ m_KoukuSaydonPatternAudition.iOwnerSessionId, message });
	const auto receipt = m_KoukuSaydonPatternAuditionReceiptBySessionId.find(
		m_KoukuSaydonPatternAudition.iOwnerSessionId);
	if (m_KoukuSaydonPatternAuditionReceiptBySessionId.end() != receipt &&
		receipt->second.Request.iRequestSequence == message.iRequestSequence)
	{
		receipt->second.LastLifecycle = message;
	}
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Flush_KoukuSaydonPatternAuditionLifecycle()
{
	using namespace LostArk::Shared;
	auto pending = std::move(m_PendingKoukuSaydonPatternAuditionLifecycle);
	m_PendingKoukuSaydonPatternAuditionLifecycle.clear();
	for (const TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& targeted : pending)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(targeted.iSessionId);
		if (nullptr == session)
			continue;
		CPacketWriter writer;
		if (!Write_Message(writer, targeted.Message))
			return false;
		if (!session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE,
			writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}
#endif

#ifdef _DEBUG
LostArk::Server::CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_MEMBER*
LostArk::Server::CGameRoom::Find_KoukuAuditionMember(const LostArk::Shared::NET_ENTITY_ID bossId)
{
	for (auto& member : m_KoukuSaydonPatternAudition.Members) if (member.iBossEntityId == bossId) return &member;
	return nullptr;
}
#endif

#ifdef _DEBUG
LostArk::Server::KOUKUSAYDON_LOGIC_LEDGER* LostArk::Server::CGameRoom::Active_KoukuPlayerLedger()
{
	for (auto& member : m_KoukuSaydonPatternAudition.Members)
		if (member.LogicLedger.Is_Active() && (member.bOwnsPlayerMode || m_KoukuSaydonPatternAudition.Members.size() == 1u)) return &member.LogicLedger;
	return nullptr;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Build_KoukuBundleState(LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& message) const
{
	using namespace LostArk::Shared;
	const auto& run = m_KoukuSaydonPatternAudition;
	if (run.ePhase == KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE || run.Members.empty()) return false;
	message = {};
	message.eWorldId = m_eWorldId; message.strEncounterId = run.Request.Scope.strEncounterId; message.strBundleId = run.Request.strBundleId;
	message.iRunEpoch = run.iRoomAuditionEpoch; message.iCommonStartTick = run.iCommonStartTick; message.iServerTick = m_iServerTick;
	message.PinnedGameplayRevision = run.PinnedGameplayRevision; message.iPinnedSourceRevision = run.iPinnedSourceRevision;
	message.eState = run.ePhase == KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING ? KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING : KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE;
	for (const auto& member : run.Members)
	{
		KOUKUSAYDON_BUNDLE_MEMBER_STATE state;
		state.strMemberId = member.strMemberId; state.iBossNetEntityId = member.iBossEntityId;
		state.strPatternId = member.PatternIds[member.iPatternIndex]; state.iPatternSequence = member.iPatternSequence; state.iStartTick = member.iScheduledStartTick;
		state.eState = member.bCompleted ? KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED :
			(member.ePhase == KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING ? KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING : KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE);
		message.Members.push_back(std::move(state));
	}
	return true;
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Broadcast_KoukuBundleState(const LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state)
{
	using namespace LostArk::Shared;
	S2C_KOUKUSAYDON_BUNDLE_STATE message; if (!Build_KoukuBundleState(message)) return;
	message.eState = state;
	CPacketWriter writer; if (!Write_Message(writer, message)) { m_strStatus = "KoukuSaydon run state serialization failed"; return; }
	for (const auto& [id, player] : m_Players)
		if (const auto session = Find_Session(player.iSessionId); session && !session->Send_Frame(PACKET_TYPE::S2C_KOUKUSAYDON_BUNDLE_STATE, writer.Get_Buffer())) session->Request_Close();
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Broadcast_OwnedWorldSequence(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& message)
{
	using namespace LostArk::Shared;
	CPacketWriter writer; if (!Write_Message(writer, message)) { m_strStatus = "KoukuSaydon owned World cue serialization failed"; return; }
	for (const auto& [id, player] : m_Players)
		if (const auto session = Find_Session(player.iSessionId); session && !session->Send_Frame(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer())) session->Request_Close();
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Stop_KoukuWorldOwner(const std::string& memberId, const bool finished)
{
	using namespace LostArk::Shared;
	if (!m_KoukuSaydonPatternAudition.iRoomAuditionEpoch) return;
	S2C_WORLD_SEQUENCE_PLAY stop;
	stop.eOperation = finished ? WORLD_SEQUENCE_OPERATION::FINISH_OWNER : WORLD_SEQUENCE_OPERATION::STOP_OWNER;
	stop.iRunEpoch = m_KoukuSaydonPatternAudition.iRoomAuditionEpoch; stop.strMemberId = memberId; stop.iServerTick = m_iServerTick;
	Broadcast_OwnedWorldSequence(stop);
	std::erase_if(m_KoukuSaydonPatternAudition.WorldPlays, [&](const auto& play) { return memberId.empty() || play.strMemberId == memberId; });
	std::erase_if(m_KoukuSaydonPatternAudition.SupportSchedule, [&](const auto& support) { return memberId.empty() || support.strMemberId == memberId; });
	for (auto& member : m_KoukuSaydonPatternAudition.Members) if (memberId.empty() || member.strMemberId == memberId)
	{ member.WorldCueByInstance.clear(); member.WorldCueByOccurrence.clear(); }
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Clear_KoukuSaydonPatternAudition(const bool completed)
{
	using namespace LostArk::Shared;
	auto& run = m_KoukuSaydonPatternAudition;
	if (run.Members.empty()) { run = {}; return; }
	const auto& first = run.Members.front();
	Queue_KoukuSaydonPatternAuditionLifecycle(first.PatternIds[first.iPatternIndex], first.iPatternSequence, 0u,
		completed ? KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED : KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED,
		completed ? std::string{} : "KoukuSaydon run stopped or lost an admitted participant");
	Broadcast_KoukuBundleState(completed ? KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED : KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ABORTED);
	Stop_KoukuWorldOwner({}, completed);
	for (auto& member : run.Members)
	{
		SERVER_WORLD_ENTITY* boss = nullptr;
		for (auto& entity : m_WorldEntities) if (entity.iNetEntityId == member.iBossEntityId) { boss = &entity; break; }
		if (boss && !boss->strPatternId.empty()) m_KoukuSaydonBrain.Abort_Pattern(*boss, m_iServerTick);
		(void)Release_PlayerAttachments(member.iBossEntityId, 0.f, 0u, false, 0u, m_iServerTick ? m_iServerTick : 1u);
		CKoukuSaydonLogicRuntime::Discard(member.LogicLedger, m_Players, boss);
		// Explicit stop/restart cancels the owned volley; natural completion retains its tail.
		if (!completed) m_CombatObjectRuntime.Cancel_Source(member.iBossEntityId);
		std::erase_if(m_PendingKoukuMechanicTriggers, [&](const auto& trigger) { return trigger.iBossEntityId == member.iBossEntityId; });
	}
	run = {};
	(void)Refresh_KoukuSupportSurfaces(m_iServerTick);
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Refresh_KoukuSupportSurfaces(const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA) return true;
	auto& schedule = m_KoukuSaydonPatternAudition.SupportSchedule;
	std::erase_if(schedule, [&](const auto& support) { return Has_ReachedServerTick(serverTick, support.iEndTick); });
	std::vector<SERVER_NAVIGATION_SUPPORT_SURFACE> surfaces;
	for (const auto& support : schedule)
		if (Has_ReachedServerTick(serverTick, support.iStartTick)) surfaces.push_back(support.Surface);
	const auto previousRevision = m_ServerNavigation.Get_Revision();
	const auto previousSurfaces = m_ServerNavigation.Get_RuntimeSupportSurfaces();
	std::string status;
	if (!m_ServerNavigation.Set_RuntimeSupportSurfaces(surfaces, status))
	{
		Clear_KoukuSaydonPatternAudition();
		std::string cleanupStatus;
		surfaces.clear();
		if (!m_ServerNavigation.Set_RuntimeSupportSurfaces(surfaces, cleanupStatus))
		{ Mark_RuntimeFailure("kouku.support-surface-cleanup"); return false; }
		m_strStatus = "KoukuSaydon support surface rejected: " + status;
	}
	const bool changed = previousRevision != m_ServerNavigation.Get_Revision();
	if (changed) Invalidate_DynamicNavigationPaths();
	if (!changed && surfaces.empty()) return true;
	// No movement command is needed when a floor appears under a stationary player.
	// Attachment, scripted motion and falling keep their existing vertical authority.
	for (auto& [id, player] : m_Players)
	{
		if (!player.iCurrentHp || player.eAction == PLAYER_ACTION_STATE::DEAD ||
			player.eAction == PLAYER_ACTION_STATE::FALLING || player.eAction == PLAYER_ACTION_STATE::GRABBED ||
			player.bPatternBound || player.bArenaEjectionActive || player.TriggerMove.isActive ||
			player.fKnockbackRemainingSeconds > 0.f) continue;
		SERVER_NAV_POINT ground;
		if (m_ServerNavigation.Is_PointWalkableExact(player.fPositionX, player.fPositionZ) &&
			m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ, ground))
			player.fPositionY = ground.y;
	}
	// Only a boss on an added/removed support may change height. Authored airborne motion keeps its Y.
	for (auto& boss : m_WorldEntities)
	{
		if (!CKoukuSaydonBrain::Is_ArenaBoss(m_eWorldId, boss) || !boss.iCurrentHp ||
			boss.eAction == SERVER_ENTITY_ACTION::DEAD || !boss.PatternStageRootMotion.empty() ||
			boss.fPatternForcedMotionSpeed != 0.f) continue;
		const auto containsBoss = [&](const auto& surface)
		{
			const double dx = boss.fPositionX - surface.fCenterX, dz = boss.fPositionZ - surface.fCenterZ;
			return dx * dx + dz * dz <= static_cast<double>(surface.fRadiusM) * surface.fRadiusM;
		};
		if (!std::any_of(previousSurfaces.begin(), previousSurfaces.end(), containsBoss) &&
			!std::any_of(surfaces.begin(), surfaces.end(), containsBoss)) continue;
		if (!boss.strPatternId.empty())
		{
			const auto* catalog = Resolve_KoukuProductCatalog();
			std::string patternStatus;
			const auto* pattern = catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, boss.strPatternId, patternStatus) : nullptr;
			if (!pattern || pattern->BossMotion) continue;
		}
		SERVER_NAV_POINT ground;
		if (m_ServerNavigation.Is_PointWalkableExact(boss.fPositionX, boss.fPositionZ) &&
			m_ServerNavigation.Sample_Position(boss.fPositionX, boss.fPositionZ, ground))
			boss.fPositionY = ground.y;
	}
	return true;
}
#endif

#ifdef _DEBUG
const LostArk::Server::CGameplayCatalog*
LostArk::Server::CGameRoom::Resolve_KoukuProductCatalog() const noexcept
{
	const auto& run = m_KoukuSaydonPatternAudition;
	if (run.ePhase == KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) return nullptr;
	const auto* product = run.pProductGeneration ? run.pProductGeneration.get() :
		m_GameplayCatalog.Resolve(run.PinnedGameplayRevision);
	return product && CKoukuSaydonBrain::Resolve_ProductSourceRevision(*product) == run.iPinnedSourceRevision ?
		product : nullptr;
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Prepare_KoukuAuditionTick(const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	auto& run = m_KoukuSaydonPatternAudition;
	if (run.ePhase == KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE) return;
	const auto* catalog = Resolve_KoukuProductCatalog();
	if (!catalog || CKoukuSaydonBrain::Resolve_ProductSourceRevision(*catalog) != run.iPinnedSourceRevision)
	{ m_strStatus = "KoukuSaydon pinned run generation is unavailable"; Clear_KoukuSaydonPatternAudition(); return; }
	struct START final { SERVER_WORLD_ENTITY* live; SERVER_WORLD_ENTITY staged; KOUKUSAYDON_PATTERN_AUDITION_MEMBER* member; KOUKUSAYDON_LOGIC_LEDGER ledger; };
	std::vector<START> starts;
	auto supportSchedule = run.SupportSchedule;
	std::erase_if(supportSchedule, [&](const auto& support) { return Has_ReachedServerTick(serverTick, support.iEndTick); });
	for (auto& member : run.Members)
	{
		SERVER_WORLD_ENTITY* boss = nullptr;
		for (auto& entity : m_WorldEntities) if (entity.iNetEntityId == member.iBossEntityId) { boss = &entity; break; }
		if (!boss || !boss->iCurrentHp || boss->eAction == SERVER_ENTITY_ACTION::DEAD || !CKoukuSaydonBrain::Is_ArenaBoss(m_eWorldId, *boss))
		{ m_strStatus = "KoukuSaydon admitted participant died or disappeared"; Clear_KoukuSaydonPatternAudition(); return; }
		if (member.bCompleted || member.ePhase != KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING || !Has_ReachedServerTick(serverTick, member.iNextStartTick)) continue;
		std::string status;
		const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, member.PatternIds[member.iPatternIndex], status);
		START start{boss, *boss, &member, {}};
		if (!pattern || !Is_KoukuBossMotionNavigable(*pattern, m_ServerNavigation) || (pattern->bResetBossToSpawn && (!m_ServerNavigation.Is_PointWalkableExact(boss->fSpawnPositionX, boss->fSpawnPositionZ) || !std::isfinite(boss->fSpawnPositionY))) ||
			!m_KoukuSaydonBrain.Begin_Pattern(start.staged, *pattern, run.PinnedGameplayRevision, member.iNextStartTick, status))
		{ m_strStatus = status.empty() ? "KoukuSaydon scheduled member could not begin" : status; Clear_KoukuSaydonPatternAudition(); return; }
		if (pattern->bResetBossToSpawn)
		{
			start.staged.fPositionX = boss->fSpawnPositionX; start.staged.fPositionY = boss->fSpawnPositionY; start.staged.fPositionZ = boss->fSpawnPositionZ;
			if (pattern->ResetBossYawDegrees) start.staged.fYawDegrees = *pattern->ResetBossYawDegrees;
		}
		if (!Commit_BossPatternPlayerStageActions(start.staged, *catalog, pattern->strPatternId,
			pattern->Stages.front().strActionId, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, member.iNextStartTick, 0u))
		{ m_strStatus = "KoukuSaydon initial target selection failed"; Clear_KoukuSaydonPatternAudition(); return; }
		CKoukuSaydonLogicRuntime::Build(*pattern, start.staged, member.iNextStartTick, start.ledger);
		for (const auto& cue : pattern->WorldSequences)
		{
			const auto cueStart = Add_ServerTicksSkippingReservedZero(member.iNextStartTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(cue.iStartMs));
			for (const auto& window : cue.SupportWindows)
			{
				KOUKU_SCHEDULED_SUPPORT_SURFACE support;
				support.strMemberId = member.strMemberId;
				support.iStartTick = Add_ServerTicksSkippingReservedZero(cueStart, window.iStartOffsetTicks);
				support.iEndTick = Add_ServerTicksSkippingReservedZero(cueStart, window.iEndOffsetTicks);
				support.Surface.strOwnerKey = std::to_string(run.iRoomAuditionEpoch) + "/" + member.strMemberId + "/" +
					std::to_string(start.staged.iPatternSequence) + "/" + cue.strOccurrenceId;
				support.Surface.fCenterX = window.fCenterX + cue.fPositionOffsetX +
					(cue.bAnchorBossSpawn ? boss->fSpawnPositionX - cue.fAnchorPositionX : 0.f);
				support.Surface.fCenterZ = window.fCenterZ + cue.fPositionOffsetZ +
					(cue.bAnchorBossSpawn ? boss->fSpawnPositionZ - cue.fAnchorPositionZ : 0.f);
				support.Surface.fHeightY = window.fHeightY + cue.fPositionOffsetY +
					(cue.bAnchorBossSpawn ? boss->fSpawnPositionY - cue.fAnchorPositionY : 0.f);
				support.Surface.fRadiusM = window.fRadiusM;
				supportSchedule.push_back(std::move(support));
			}
		}
		if (supportSchedule.size() > 2048u)
		{ m_strStatus = "KoukuSaydon support schedule capacity exceeded"; Clear_KoukuSaydonPatternAudition(); return; }

		starts.push_back(std::move(start));
	}
	run.SupportSchedule = std::move(supportSchedule);
	// Commit every due actor before any member may judge, broadcast, or advance its first stage.
	for (auto& start : starts)
	{
		*start.live = std::move(start.staged); start.member->LogicLedger = std::move(start.ledger);
		start.member->iPatternSequence = start.live->iPatternSequence; start.member->ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
		run.ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
	}
	for (const auto& start : starts)
		Queue_KoukuSaydonPatternAuditionLifecycle(start.live->strPatternId, start.live->iPatternSequence, start.live->iPatternStageIndex,
			KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE, {}, start.live->iNetEntityId);
	if (!starts.empty()) Broadcast_KoukuBundleState(KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE);
}
#endif

bool LostArk::Server::CGameRoom::Update_KoukuSaydonBoss(SERVER_WORLD_ENTITY& boss, const std::uint32_t serverTick)
{
#ifndef _DEBUG
	if (!boss.strPatternId.empty()) m_KoukuSaydonBrain.Abort_Pattern(boss, serverTick);
	boss.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision(); return true;
#else
	using namespace LostArk::Shared;
	auto* member = Find_KoukuAuditionMember(boss.iNetEntityId);
	if (!member)
	{
		if (!boss.strPatternId.empty()) { m_KoukuSaydonBrain.Abort_Pattern(boss, serverTick); m_strStatus = "KoukuSaydon boss had an unowned pattern occurrence"; return false; }
		boss.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision(); return true;
	}
	if (member->bCompleted || member->ePhase == KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING) return true;
	const auto* catalog = Resolve_KoukuProductCatalog();
	std::string status;
	const auto* pattern = catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, member->PatternIds[member->iPatternIndex], status) : nullptr;
	if (!pattern || boss.strPatternId != pattern->strPatternId)
	{ m_strStatus = "KoukuSaydon member lost its exact running pattern"; Clear_KoukuSaydonPatternAudition(); return true; }
	CKoukuSaydonBrain::Apply_BossMotion(boss, *pattern, serverTick);
	const auto completedPatternId = boss.strPatternId;
	const auto sequence = boss.iPatternSequence, stage = boss.iPatternStageIndex;
	KOUKUSAYDON_LOGIC_OUTPUT output;
	const auto* baseCatalog = m_GameplayCatalog.Resolve(m_KoukuSaydonPatternAudition.PinnedGameplayRevision);
	if (!baseCatalog) { m_strStatus = "KoukuSaydon base gameplay pin is unavailable"; Clear_KoukuSaydonPatternAudition(); return true; }
	CKoukuSaydonLogicRuntime::Update(boss, *pattern, member->LogicLedger, m_Players, *baseCatalog,
		catalog->Find_KoukuMadnessPolicy(boss.strEncounterId), serverTick, m_TickDamageEvents, output,
        &m_ServerNavigation, &m_ServerCollisionSystem);
	const bool early = Apply_KoukuLogicOutput(output, boss, serverTick);
	const auto update = early ? KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED : m_KoukuSaydonBrain.Update(boss, *catalog, serverTick, status);
	if (update == KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED)
	{
		if (!Commit_BossPatternPlayerStageActions(boss, *catalog, completedPatternId,
			pattern->Stages[boss.iPatternStageIndex].strActionId, BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick, 0u))
		{ m_strStatus = "KoukuSaydon stage target selection failed"; Clear_KoukuSaydonPatternAudition(); return true; }
		Queue_KoukuSaydonPatternAuditionLifecycle(completedPatternId, boss.iPatternSequence, boss.iPatternStageIndex, KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE, {}, boss.iNetEntityId);
	}
	if (update == KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_INVALID_DEFINITION || update == KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD)
	{ m_strStatus = status; Clear_KoukuSaydonPatternAudition(); return true; }
	if (update != KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED) return true;
	(void)Release_PlayerAttachments(boss.iNetEntityId, 0.f, 0u, false, 0u, serverTick);
	CKoukuSaydonLogicRuntime::Discard(member->LogicLedger, m_Players, &boss);
	Queue_KoukuSaydonPatternAuditionLifecycle(completedPatternId, sequence, stage, KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PATTERN_COMPLETED, {}, boss.iNetEntityId);
	const auto completedIndex = member->iPatternIndex;
	if (completedIndex + 1u < member->PatternIds.size())
	{
		++member->iPatternIndex; member->ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
		member->iNextStartTick = Add_ServerTicksSkippingReservedZero(serverTick, (std::max)(1u, member->TransitionTicks[completedIndex]));
		Queue_KoukuSaydonPatternAuditionLifecycle(member->PatternIds[member->iPatternIndex], 0u, 0u, KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::PENDING, {}, boss.iNetEntityId);
		if (m_KoukuSaydonPatternAudition.Members.size() == 1u) m_KoukuSaydonPatternAudition.ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
	}
	else
	{
		member->bCompleted = true; Stop_KoukuWorldOwner(member->strMemberId, true);
		if (std::all_of(m_KoukuSaydonPatternAudition.Members.begin(), m_KoukuSaydonPatternAudition.Members.end(), [](const auto& value) { return value.bCompleted; }))
		{ Clear_KoukuSaydonPatternAudition(true); boss.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision(); return true; }
	}
	Broadcast_KoukuBundleState(KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE);
	return true;
#endif
}

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_AuditionBoss()
{
	return Find_AuditionBoss("boss.valtan.center");
}

LostArk::Server::SERVER_WORLD_ENTITY*
LostArk::Server::CGameRoom::Find_AuditionBoss(
	const std::string& placementId)
{
	if (placementId.empty())
		return nullptr;
	const auto found = std::find_if(
		m_WorldEntities.begin(),
		m_WorldEntities.end(),
		[&placementId](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				placementId == entity.strPlacementId &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"ENCOUNTER_VALTAN" == entity.strEncounterId;
		});
	return m_WorldEntities.end() == found ? nullptr : &*found;
}

bool LostArk::Server::CGameRoom::Has_EngagedAuditionPlayer(
	const SERVER_WORLD_ENTITY& boss) const
{
	/* CValtanBrain::Update drops its target and calls FinishPattern when no
	combat-ready living player is inside the engage distance, so a pattern
	queued while that is true would be discarded before it ever started. The
	brain widens the radius to the longest authored pattern range; this checks
	only the boss's own engage distance, which is the narrower of the two, so an
	accepted audition is always one the brain will also act on. */
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (0u == player.iCurrentHp || !player.isCombatReady ||
			LostArk::Shared::PLAYER_ACTION_STATE::DEAD == player.eAction ||
			LostArk::Shared::PLAYER_ACTION_STATE::FALLING == player.eAction)
		{
			continue;
		}
		const float deltaX = player.fPositionX - boss.fPositionX;
		const float deltaZ = player.fPositionZ - boss.fPositionZ;
		if (deltaX * deltaX + deltaZ * deltaZ <=
			boss.fEngageDistance * boss.fEngageDistance)
		{
			return true;
		}
	}
	return false;
}

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Queue_ValtanAuditionLifecycle(
	const SESSION_ID ownerSessionId,
	const std::uint32_t requestSequence,
	const std::uint32_t roomEpoch,
	const std::uint32_t patternSequence,
	const std::string& patternId,
	const LostArk::Shared::GameplayDataRevision& pinnedRevision,
	const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	if (INVALID_SESSION_ID == ownerSessionId)
		return;
	LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE message{};
	message.iRequestSequence = requestSequence;
	message.iRoomAuditionEpoch = roomEpoch;
	message.iPatternSequence = patternSequence;
	message.strPatternId = patternId;
	message.eState = state;
	message.PinnedDefinitionRevision = pinnedRevision;
	message.strReason = std::move(reason);
	m_PendingValtanAuditionLifecycle.push_back({ ownerSessionId, std::move(message) });
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Queue_ValtanPatternIdAuditionLifecycle(
	const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	using namespace LostArk::Shared;
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE ==
			m_ValtanPatternIdAudition.ePhase ||
		INVALID_SESSION_ID == m_ValtanPatternIdAudition.iOwnerSessionId ||
		m_ValtanPatternIdAudition.bAdoptedLivePredecessor)
	{
		return;
	}
	Queue_ValtanAuditionLifecycle(
		m_ValtanPatternIdAudition.iOwnerSessionId,
		m_ValtanPatternIdAudition.iRequestSequence,
		m_ValtanPatternIdAudition.iRoomAuditionEpoch,
		m_ValtanPatternIdAudition.iExpectedPatternSequence,
		m_ValtanPatternIdAudition.strPatternId,
		m_ValtanPatternIdAudition.PinnedDefinitionRevision, state, std::move(reason));
	const auto receipt = m_ValtanPatternIdAuditionSequenceBySessionId.find(
		m_ValtanPatternIdAudition.iOwnerSessionId);
	if (m_ValtanPatternIdAuditionSequenceBySessionId.end() != receipt &&
		receipt->second.Request.iRequestSequence ==
			m_ValtanPatternIdAudition.iRequestSequence &&
		!m_PendingValtanAuditionLifecycle.empty())
	{
		receipt->second.LastLifecycle =
			m_PendingValtanAuditionLifecycle.back().Message;
	}
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Queue_ValtanNextPatternLifecycle(
	const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
	const LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
	std::string reason)
{
	Queue_ValtanAuditionLifecycle(
		reservation.iOwnerSessionId, reservation.iRequestSequence,
		reservation.iRoomAuditionEpoch, reservation.iExpectedPatternSequence,
		reservation.strPatternId, reservation.PinnedDefinitionRevision,
		state, std::move(reason));
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Is_ValtanPatternIdAuditionRunning() const noexcept
{
	return VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == m_ValtanPatternIdAudition.ePhase ||
		m_ValtanNextPattern.has_value();
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Cancel_ValtanNextPatternReservation(std::string reason)
{
	if (!m_ValtanNextPattern)
		return;
	Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
		LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED, std::move(reason));
	m_ValtanNextPattern.reset();
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Cancel_ValtanPatternIdAudition(std::string reason)
{
	Cancel_ValtanNextPatternReservation(reason);
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == m_ValtanPatternIdAudition.ePhase)
	{
		// Completed holds still own a Next CAS identity; invalidate it on reset.
		Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ABORTED, std::move(reason));
	}
	m_ValtanPatternIdAudition = {};
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Flush_ValtanPatternIdAuditionLifecycle()
{
	using namespace LostArk::Shared;
	auto pending = std::move(m_PendingValtanAuditionLifecycle);
	m_PendingValtanAuditionLifecycle.clear();
	for (const TARGETED_VALTAN_AUDITION_LIFECYCLE& targeted : pending)
	{
		const std::shared_ptr<CClientSession> session =
			Find_Session(targeted.iSessionId);
		if (nullptr == session)
			continue;
		CPacketWriter writer;
		if (!Write_Message(writer, targeted.Message))
			return false;
		if (!session->Send_Frame(
			PACKET_TYPE::S2C_VALTAN_AUDITION_LIFECYCLE,
			writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}
#endif

#ifdef _DEBUG
LostArk::Shared::VALTAN_AUDITION_RESULT
LostArk::Server::CGameRoom::Evaluate_ValtanNextPatternControl(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	std::uint32_t& outCurrentHealthBar)
{
	using namespace LostArk::Shared;
	outCurrentHealthBar = 0u;
	if ((WORLD_ID::VALTAN_ARENA != m_eWorldId &&
		 WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId) ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD;
	}
	const auto previous = m_ValtanNextPatternReceiptBySessionId.find(sessionId);
	if (m_ValtanNextPatternReceiptBySessionId.end() != previous)
	{
		const C2S_VALTAN_AUDITION_REQUEST& prior = previous->second.Request;
		if (request.iRequestSequence == prior.iRequestSequence &&
			request.eOperation == prior.eOperation &&
			request.iTargetHealthBar == prior.iTargetHealthBar &&
			request.strBossPlacementId == prior.strBossPlacementId &&
			request.strPatternId == prior.strPatternId &&
			request.iPredecessorRoomAuditionEpoch == prior.iPredecessorRoomAuditionEpoch &&
			request.iPredecessorPatternSequence == prior.iPredecessorPatternSequence &&
			request.iExpectedNextRequestSequence == prior.iExpectedNextRequestSequence &&
			request.ExpectedDefinitionRevision == prior.ExpectedDefinitionRevision)
		{
			outCurrentHealthBar = previous->second.iCurrentHealthBar;
			// LIVE's result echoes epoch zero. Replay its still-owned lifecycle so
			// an exact retry can recover the authoritative epoch without restarting.
			if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == prior.eOperation &&
				VALTAN_AUDITION_RESULT::QUEUED == previous->second.Result)
			{
				if (m_ValtanNextPattern && m_ValtanNextPattern->iOwnerSessionId == sessionId &&
					m_ValtanNextPattern->iRequestSequence == prior.iRequestSequence)
				{
					Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
						m_ValtanNextPattern->bReportedWaitingForPlayer ?
						VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER :
						VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
				}
				else if (!m_ValtanPatternIdAudition.bAdoptedLivePredecessor &&
					m_ValtanPatternIdAudition.iOwnerSessionId == sessionId &&
					m_ValtanPatternIdAudition.iRequestSequence == prior.iRequestSequence)
				{
					const auto phase = m_ValtanPatternIdAudition.ePhase;
					if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == phase)
						Queue_ValtanPatternIdAuditionLifecycle(VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
					else if (VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == phase)
						Queue_ValtanPatternIdAuditionLifecycle(VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
					else if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == phase)
						Queue_ValtanPatternIdAuditionLifecycle(
							m_ValtanPatternIdAudition.bReportedWaitingForPlayer ?
							VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER :
							VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
				}
			}
			return previous->second.Result;
		}
		// Next command streams never wrap. A stale/different replay cannot replace
		// the receipt needed to answer an unconfirmed newer command exactly.
		if (request.iRequestSequence <= prior.iRequestSequence)
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
	}
	if (0u == request.iRequestSequence)
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;

	const auto evaluate = [&]() -> VALTAN_AUDITION_RESULT
	{
		CPacketWriter validation;
		if (!Write_Message(validation, request))
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		SERVER_WORLD_ENTITY* boss = Find_AuditionBoss(request.strBossPlacementId);
		if (nullptr == boss)
			return VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS;
		outCurrentHealthBar = CValtanBrain::Calculate_HealthBar(*boss);
		if (0u == boss->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss->eAction)
			return VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD;
		if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_LIVE_PATTERN_ID == request.eOperation)
			return Adopt_ValtanLiveNextPattern(sessionId, request, *boss);
		const auto& current = m_ValtanPatternIdAudition;
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE == current.ePhase ||
			request.iPredecessorRoomAuditionEpoch != current.iRoomAuditionEpoch ||
			request.iPredecessorPatternSequence != current.iExpectedPatternSequence ||
			request.strBossPlacementId != current.strBossPlacementId ||
			boss->iNetEntityId != current.iBossEntityId)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
		}
		if (sessionId != current.iOwnerSessionId)
			return VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER;
		/* Queue/replace/clear is an exact CAS on the predecessor's immutable
		   definition, not on whichever generation happens to be active now. */
		if (request.ExpectedDefinitionRevision !=
			current.PinnedDefinitionRevision)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
		}
		const bool liveOccurrence =
			VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == current.ePhase &&
			((boss->iPatternSequence == current.iExpectedPatternSequence &&
			  boss->strPatternId == current.strPatternId &&
			  boss->PinnedDefinitionRevision == current.PinnedDefinitionRevision) ||
			 Is_ValtanOutcomeFollowupInFlight(
				 *boss, current.iExpectedPatternSequence,
				 current.PinnedDefinitionRevision));
		const bool pendingOccurrence =
			VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == current.ePhase &&
			boss->iPatternSequence == current.iExpectedPatternSequence - 1u &&
			boss->PendingPatternIds.end() != std::find(
				boss->PendingPatternIds.begin(), boss->PendingPatternIds.end(), current.strPatternId);
		const bool completedOccurrence =
			VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == current.ePhase &&
			Has_ValtanOutcomeGroupCompleted(
				*boss, current.iExpectedPatternSequence,
				current.PinnedDefinitionRevision);
		if ((!liveOccurrence && !pendingOccurrence && !completedOccurrence) ||
			boss->bMechanicLedgerRequiresReset)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
		}
		if (Is_ValtanPatternFlowRunning() || m_ValtanFightPageStart.Is_Active() ||
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		const std::uint32_t currentNextToken = m_ValtanNextPattern ?
			m_ValtanNextPattern->iRequestSequence : 0u;
		if (request.iExpectedNextRequestSequence != currentNextToken)
			return VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED;
		if (VALTAN_AUDITION_OPERATION::CLEAR_NEXT_PATTERN_ID == request.eOperation)
		{
			if (!m_ValtanNextPattern ||
				request.strPatternId != m_ValtanNextPattern->strPatternId)
			{
				return VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED;
			}
			Cancel_ValtanNextPatternReservation("cleared");
			return VALTAN_AUDITION_RESULT::CLEARED;
		}
		if (VALTAN_AUDITION_OPERATION::QUEUE_NEXT_PATTERN_ID != request.eOperation ||
			(std::numeric_limits<std::uint32_t>::max)() == current.iExpectedPatternSequence)
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}
		const CGameplayCatalog* pinned =
			m_GameplayCatalog.Resolve(current.PinnedDefinitionRevision);
		const auto* patterns = nullptr == pinned ? nullptr :
			pinned->Find_BossPatterns(boss->strEncounterId);
		if (nullptr == patterns || patterns->end() == std::find_if(
			patterns->begin(), patterns->end(),
			[&request](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return pattern.strPatternId == request.strPatternId &&
					pattern.bAuthoringMasterManaged;
			}) ||
			(WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId &&
			 Is_CharacterSelectEnvironmentDependentPattern(request.strPatternId)))
		{
			return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
		}

		VALTAN_NEXT_PATTERN_RESERVATION reservation{};
		reservation.iOwnerSessionId = sessionId;
		reservation.iBossEntityId = boss->iNetEntityId;
		reservation.iRequestSequence = request.iRequestSequence;
		reservation.iRoomAuditionEpoch = current.iRoomAuditionEpoch;
		reservation.iPredecessorPatternSequence = current.iExpectedPatternSequence;
		reservation.iExpectedPatternSequence = current.iExpectedPatternSequence + 1u;
		reservation.strBossPlacementId = current.strBossPlacementId;
		reservation.strPatternId = request.strPatternId;
		reservation.PinnedDefinitionRevision = current.PinnedDefinitionRevision;
		// Validation is complete. Only the reservation and its two lifecycle edges
		// change; no boss, player, prop, combat object or navigation reset occurs.
		Cancel_ValtanNextPatternReservation("replaced");
		m_ValtanNextPattern = std::move(reservation);
		Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
			VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
		return VALTAN_AUDITION_RESULT::QUEUED;
	};
	const VALTAN_AUDITION_RESULT verdict = evaluate();
	m_ValtanNextPatternReceiptBySessionId.insert_or_assign(sessionId,
		VALTAN_NEXT_PATTERN_COMMAND_RECEIPT{ request, verdict, outCurrentHealthBar });
	return verdict;
}
#endif

#ifdef _DEBUG
LostArk::Shared::VALTAN_AUDITION_RESULT
LostArk::Server::CGameRoom::Adopt_ValtanLiveNextPattern(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	SERVER_WORLD_ENTITY& boss)
{
	using namespace LostArk::Shared;
	if (request.iPredecessorPatternSequence != boss.iPatternSequence ||
		(0u == boss.iPatternSequence && !boss.strPatternId.empty()))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
	}
	const bool hasFlow = Is_ValtanPatternFlowRunning();
	const auto& current = m_ValtanPatternIdAudition;
	if ((hasFlow && m_ValtanPatternFlowAudition.iOwnerSessionId != sessionId) ||
		(VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE != current.ePhase &&
		 INVALID_SESSION_ID != current.iOwnerSessionId && current.iOwnerSessionId != sessionId) ||
		(m_ValtanNextPattern && m_ValtanNextPattern->iOwnerSessionId != sessionId))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_NOT_OWNER;
	}
	if (m_ValtanNextPattern)
		return VALTAN_AUDITION_RESULT::REJECTED_NEXT_CHANGED;
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == current.ePhase)
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	if (boss.PendingPatternFollowup.Is_Pending() ||
		boss.iPatternFollowupDepth > 0u)
	{
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	}
	if (VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase ||
		m_ValtanFightPageStart.Is_Active() || 0u != m_iValtanAuditionArmedHealthBar ||
		boss.bMechanicLedgerRequiresReset || 0u == m_iNextValtanAuditionEpoch ||
		(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence)
	{
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	}
	const bool active = !boss.strPatternId.empty();
	if (hasFlow && (boss.iNetEntityId != m_ValtanPatternFlowAudition.iBossEntityId ||
		boss.strPlacementId != m_ValtanPatternFlowAudition.strBossPlacementId ||
		(active && (!boss.bAutomaticPatternSequenceStepRunning ||
		 boss.strRotationId != m_ValtanPatternFlowAudition.Sequence.strSequenceId ||
		 boss.iRotationStepIndex >= m_ValtanPatternFlowAudition.Sequence.PatternIds.size() ||
		 boss.strPatternId != m_ValtanPatternFlowAudition.Sequence.PatternIds[boss.iRotationStepIndex]))))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
	}
	const GameplayDataRevision pinnedRevision = hasFlow ?
		m_ValtanPatternFlowAudition.PinnedDefinitionRevision :
		(active ? boss.PinnedDefinitionRevision : m_GameplayCatalog.Get_ActiveRevision());
	if (request.ExpectedDefinitionRevision != pinnedRevision)
		return VALTAN_AUDITION_RESULT::REJECTED_STALE_AUDITION;
	const CGameplayCatalog* pinned = m_GameplayCatalog.Resolve(pinnedRevision);
	const auto* patterns = nullptr == pinned ? nullptr :
		pinned->Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns || (active && boss.PinnedDefinitionRevision != pinnedRevision) ||
		patterns->end() == std::find_if(patterns->begin(), patterns->end(),
			[&request](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return pattern.strPatternId == request.strPatternId && pattern.bAuthoringMasterManaged;
			}) ||
		(active && patterns->end() == std::find_if(patterns->begin(), patterns->end(),
			[&boss](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return pattern.strPatternId == boss.strPatternId;
			})) ||
		(WORLD_ID::CHARACTER_SELECT_ARENA == m_eWorldId &&
		 Is_CharacterSelectEnvironmentDependentPattern(request.strPatternId)))
	{
		return VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE;
	}

	VALTAN_PATTERN_ID_AUDITION_STATE adopted{};
	adopted.ePhase = active ? VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE :
		VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD;
	adopted.iOwnerSessionId = sessionId;
	adopted.iBossEntityId = boss.iNetEntityId;
	adopted.iExpectedPatternSequence = boss.iPatternSequence;
	adopted.iRequestSequence = request.iRequestSequence;
	adopted.iRoomAuditionEpoch = m_iNextValtanAuditionEpoch;
	adopted.strBossPlacementId = boss.strPlacementId;
	adopted.strPatternId = boss.strPatternId;
	adopted.PinnedDefinitionRevision = pinnedRevision;
	adopted.bAdoptedLivePredecessor = true;
	if (hasFlow && active)
		adopted.AdoptedFlowSequence = m_ValtanPatternFlowAudition.Sequence;
	VALTAN_NEXT_PATTERN_RESERVATION next{};
	next.iOwnerSessionId = sessionId;
	next.iBossEntityId = boss.iNetEntityId;
	next.iRequestSequence = request.iRequestSequence;
	next.iRoomAuditionEpoch = adopted.iRoomAuditionEpoch;
	next.iPredecessorPatternSequence = boss.iPatternSequence;
	next.iExpectedPatternSequence = boss.iPatternSequence + 1u;
	next.strBossPlacementId = boss.strPlacementId;
	next.strPatternId = request.strPatternId;
	next.PinnedDefinitionRevision = pinnedRevision;

	// Admission is complete. Transfer only playback ownership: the active
	// occurrence, player, boss pose/HP and all arena runtimes remain untouched.
	if (hasFlow)
	{
		Queue_ValtanPatternFlowLifecycle(VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
			&boss, "Flow remainder replaced by a live Next Pattern reservation");
		m_ValtanPatternFlowAudition = {};
	}
	m_ValtanPatternIdAudition = std::move(adopted);
	m_ValtanNextPattern = std::move(next);
	m_iNextValtanAuditionEpoch =
		(std::numeric_limits<std::uint32_t>::max)() == m_iNextValtanAuditionEpoch ?
		0u : m_iNextValtanAuditionEpoch + 1u;
	boss.bIntroPatternConsumed = true;
	boss.PendingPatternIds.clear();
	if (!active)
	{
		// Freeze the idle decision until this tick's world/prop commits finish.
		// IDLE_HOLD is admission evidence, never a fabricated COMPLETED receipt.
		boss.PinnedDefinitionRevision = pinnedRevision;
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		boss.bAutomaticPatternSequenceStepRunning = false;
		boss.bAutomaticPatternSequencePausedForRevive = false;
		boss.iAutomaticPatternSequencePauseLastTick = 0u;
		boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
		boss.MovePath.clear();
	}
	Queue_ValtanNextPatternLifecycle(*m_ValtanNextPattern,
		VALTAN_AUDITION_LIFECYCLE_STATE::NEXT_RESERVED);
	return VALTAN_AUDITION_RESULT::QUEUED;
}
#endif

#ifdef _DEBUG
void LostArk::Server::CGameRoom::Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss)
{
	using namespace LostArk::Shared;
	const bool idleAnchor = VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD ==
		m_ValtanPatternIdAudition.ePhase;
	if (!m_ValtanNextPattern ||
		(!idleAnchor && VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD !=
			m_ValtanPatternIdAudition.ePhase))
	{
		return;
	}
	auto& next = *m_ValtanNextPattern;
	const auto& current = m_ValtanPatternIdAudition;
	const bool completedOutcomeGroup = idleAnchor ||
		Has_ValtanOutcomeGroupCompleted(
			boss, current.iExpectedPatternSequence,
			current.PinnedDefinitionRevision);
	if (!m_PlayerIdBySessionId.contains(next.iOwnerSessionId) ||
		next.iOwnerSessionId != current.iOwnerSessionId ||
		next.iBossEntityId != boss.iNetEntityId ||
		next.strBossPlacementId != boss.strPlacementId ||
		next.iRoomAuditionEpoch != current.iRoomAuditionEpoch ||
		next.iPredecessorPatternSequence != current.iExpectedPatternSequence ||
		next.iExpectedPatternSequence <= next.iPredecessorPatternSequence ||
		next.iExpectedPatternSequence != next.iPredecessorPatternSequence + 1u ||
		next.PinnedDefinitionRevision != current.PinnedDefinitionRevision ||
		next.PinnedDefinitionRevision != boss.PinnedDefinitionRevision ||
		nullptr == m_GameplayCatalog.Resolve(next.PinnedDefinitionRevision) ||
		!boss.strPatternId.empty() || !boss.PendingPatternIds.empty() ||
		boss.bMechanicLedgerRequiresReset || !completedOutcomeGroup)
	{
		Cancel_ValtanNextPatternReservation("predecessor no longer owns the completed occurrence");
		return;
	}
	// An idle admission has no cancellable predecessor. Consume it at this
	// post-commit seam; the existing pending-Next guard waits for a player.
	if (!idleAnchor && !Has_EngagedAuditionPlayer(boss))
	{
		if (!next.bReportedWaitingForPlayer)
		{
			Queue_ValtanNextPatternLifecycle(next,
				VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER);
			next.bReportedWaitingForPlayer = true;
		}
		return;
	}
	/* A branch-owned child consumes its own PatternSequence values. Keep the
	   reservation's root CAS immutable while it is replaceable/waiting, then
	   rebase the consumed successor exactly once to the completed leaf. */
	next.iPredecessorPatternSequence = boss.iPatternSequence;
	next.iExpectedPatternSequence =
		(std::numeric_limits<std::uint32_t>::max)() == boss.iPatternSequence ?
			1u : boss.iPatternSequence + 1u;
	VALTAN_PATTERN_ID_AUDITION_STATE promoted{};
	promoted.ePhase = VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING;
	promoted.iOwnerSessionId = next.iOwnerSessionId;
	promoted.iBossEntityId = next.iBossEntityId;
	promoted.iExpectedPatternSequence = next.iExpectedPatternSequence;
	promoted.iRequestSequence = next.iRequestSequence;
	promoted.iRoomAuditionEpoch = next.iRoomAuditionEpoch;
	promoted.strBossPlacementId = next.strBossPlacementId;
	promoted.strPatternId = next.strPatternId;
	promoted.PinnedDefinitionRevision = next.PinnedDefinitionRevision;
	promoted.bResetlessContinuation = true;
	boss.PendingPatternIds.push_back(next.strPatternId);
	boss.bAutomaticPatternSequenceAuditionOverride = true;
	boss.bAutomaticPatternSequenceAuditionHold = false;
	m_ValtanPatternIdAudition = std::move(promoted);
	m_ValtanNextPattern.reset();
	Queue_ValtanPatternIdAuditionLifecycle(VALTAN_AUDITION_LIFECYCLE_STATE::PENDING);
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Prepare_ValtanPatternIdAuditionBeforeBrain(
	SERVER_WORLD_ENTITY& boss)
{
	if (boss.iNetEntityId != m_ValtanPatternIdAudition.iBossEntityId ||
		boss.strPlacementId != m_ValtanPatternIdAudition.strBossPlacementId ||
		0u == boss.iCurrentHp || SERVER_ENTITY_ACTION::DEAD == boss.eAction)
	{
		return true;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD == m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD == m_ValtanPatternIdAudition.ePhase)
		return false;
	const bool exactActiveOccurrence =
		boss.iPatternSequence ==
			m_ValtanPatternIdAudition.iExpectedPatternSequence &&
		boss.strPatternId == m_ValtanPatternIdAudition.strPatternId &&
		boss.PinnedDefinitionRevision ==
			m_ValtanPatternIdAudition.PinnedDefinitionRevision;
	const bool outcomeFollowupInFlight = Is_ValtanOutcomeFollowupInFlight(
		boss, m_ValtanPatternIdAudition.iExpectedPatternSequence,
		m_ValtanPatternIdAudition.PinnedDefinitionRevision);
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
			m_ValtanPatternIdAudition.ePhase &&
		!exactActiveOccurrence && !outcomeFollowupInFlight)
	{
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		Cancel_ValtanPatternIdAudition("active audition identity was discarded before the tick");
		return false;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING != m_ValtanPatternIdAudition.ePhase ||
		!m_ValtanPatternIdAudition.bResetlessContinuation)
	{
		return true;
	}
	if (boss.iPatternSequence != m_ValtanPatternIdAudition.iExpectedPatternSequence - 1u ||
		!boss.strPatternId.empty() || boss.PendingPatternIds.end() == std::find(
			boss.PendingPatternIds.begin(), boss.PendingPatternIds.end(), m_ValtanPatternIdAudition.strPatternId))
	{
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		Cancel_ValtanPatternIdAudition("pending Next identity was discarded before the tick");
		return false;
	}
	// The last target may disappear after promotion but before BeginPattern.
	// Retain the exact pending occurrence without invoking the no-target abort
	// path or allowing the ordinary Product selector to take this slot.
	if (!Has_EngagedAuditionPlayer(boss))
	{
		boss.bAutomaticPatternSequenceAuditionOverride = true;
		boss.bAutomaticPatternSequenceAuditionHold = true;
		if (!m_ValtanPatternIdAudition.bReportedWaitingForPlayer)
		{
			Queue_ValtanPatternIdAuditionLifecycle(
				LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::WAITING_FOR_PLAYER);
			m_ValtanPatternIdAudition.bReportedWaitingForPlayer = true;
		}
		return false;
	}
	boss.bAutomaticPatternSequenceAuditionOverride = true;
	boss.bAutomaticPatternSequenceAuditionHold = false;
	m_ValtanPatternIdAudition.bReportedWaitingForPlayer = false;
	return true;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Refresh_ValtanPatternIdAuditionState()
{
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE ==
		m_ValtanPatternIdAudition.ePhase)
	{
		return false;
	}

	const auto boss = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[this](const SERVER_WORLD_ENTITY& entity)
		{
			return entity.iNetEntityId ==
					m_ValtanPatternIdAudition.iBossEntityId &&
				entity.strPlacementId ==
					m_ValtanPatternIdAudition.strBossPlacementId &&
				WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"ENCOUNTER_VALTAN" == entity.strEncounterId;
		});
	if (m_WorldEntities.end() == boss || 0u == boss->iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss->eAction)
	{
		Cancel_ValtanPatternIdAudition("Valtan audition boss became unavailable");
		return false;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD ==
			m_ValtanPatternIdAudition.ePhase ||
		VALTAN_PATTERN_ID_AUDITION_PHASE::IDLE_HOLD ==
			m_ValtanPatternIdAudition.ePhase)
	{
		Try_PromoteValtanNextPattern(*boss);
		return Is_ValtanPatternIdAuditionRunning();
	}
	if (boss->bMechanicLedgerRequiresReset)
	{
		boss->bAutomaticPatternSequenceAuditionOverride = true;
		boss->bAutomaticPatternSequenceAuditionHold = true;
		Cancel_ValtanPatternIdAudition("Valtan occurrence requires an authoritative reset");
		return false;
	}

	if (boss->iPatternSequence ==
			m_ValtanPatternIdAudition.iExpectedPatternSequence &&
		boss->strPatternId == m_ValtanPatternIdAudition.strPatternId)
	{
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE !=
			m_ValtanPatternIdAudition.ePhase)
		{
			if (!boss->PinnedDefinitionRevision.Is_Valid() ||
				boss->PinnedDefinitionRevision !=
					m_ValtanPatternIdAudition.PinnedDefinitionRevision)
			{
				Cancel_ValtanPatternIdAudition(
					"Valtan audition definition revision changed before start");
				return false;
			}
			m_ValtanPatternIdAudition.ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE;
			Queue_ValtanPatternIdAuditionLifecycle(
				LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::ACTIVE);
		}
		return true;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
			m_ValtanPatternIdAudition.ePhase &&
		Is_ValtanOutcomeFollowupInFlight(
			*boss, m_ValtanPatternIdAudition.iExpectedPatternSequence,
			m_ValtanPatternIdAudition.PinnedDefinitionRevision))
	{
		return true;
	}
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING == m_ValtanPatternIdAudition.ePhase &&
		boss->iPatternSequence == m_ValtanPatternIdAudition.iExpectedPatternSequence - 1u &&
		boss->PendingPatternIds.end() != std::find(
			boss->PendingPatternIds.begin(), boss->PendingPatternIds.end(),
			m_ValtanPatternIdAudition.strPatternId))
	{
		m_ValtanPatternIdAudition.ePhase =
			VALTAN_PATTERN_ID_AUDITION_PHASE::PENDING;
		return true;
	}

	// An empty pattern ID is not completion evidence: no-target, death and
	// authoritative discard also retire it. Only FinishPattern's exact receipt
	// may advance the chain after this tick's final world/prop/damage commits.
	if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE ==
			m_ValtanPatternIdAudition.ePhase &&
		Has_ValtanOutcomeGroupCompleted(
			*boss, m_ValtanPatternIdAudition.iExpectedPatternSequence,
			m_ValtanPatternIdAudition.PinnedDefinitionRevision) &&
		!boss->bMechanicLedgerRequiresReset)
	{
		boss->iPatternFollowupDepth = 0u;
		boss->iPatternFollowupRootSequence = 0u;
		boss->bAutomaticPatternSequenceAuditionOverride = true;
		boss->bAutomaticPatternSequenceAuditionHold = true;
		Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE::COMPLETED);
		m_ValtanPatternIdAudition.ePhase = VALTAN_PATTERN_ID_AUDITION_PHASE::COMPLETED_HOLD;
		Try_PromoteValtanNextPattern(*boss);
		return Is_ValtanPatternIdAuditionRunning();
	}
	boss->bAutomaticPatternSequenceAuditionOverride = true;
	boss->bAutomaticPatternSequenceAuditionHold = true;
	Cancel_ValtanPatternIdAudition("Valtan audition occurrence aborted, discarded or replaced");
	return false;
}
#endif

#ifdef _DEBUG
bool LostArk::Server::CGameRoom::Is_ValtanPatternFlowRunning() const noexcept
{
	return VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE !=
		m_ValtanPatternFlowAudition.ePhase;
}
#endif

#ifdef _DEBUG
const LostArk::Server::BOSS_PATTERN_SEQUENCE_DEFINITION*
LostArk::Server::CGameRoom::Resolve_ValtanPatternFlowSequence(
	const SERVER_WORLD_ENTITY& boss) const noexcept
{
	if (!Is_ValtanPatternFlowRunning() ||
		boss.iNetEntityId != m_ValtanPatternFlowAudition.iBossEntityId)
	{
		const auto& current = m_ValtanPatternIdAudition;
		if (VALTAN_PATTERN_ID_AUDITION_PHASE::ACTIVE == current.ePhase &&
			current.bAdoptedLivePredecessor && current.AdoptedFlowSequence &&
			boss.iNetEntityId == current.iBossEntityId)
		{
			return &*current.AdoptedFlowSequence;
		}
		return nullptr;
	}
	return &m_ValtanPatternFlowAudition.Sequence;
}
#endif
