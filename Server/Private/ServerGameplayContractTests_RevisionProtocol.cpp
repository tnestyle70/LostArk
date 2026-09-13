#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "ServerApp.h"
#include "ValtanBrain.h"
#include "WorldBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

#include "ServerGameplayContractTests_PinnedGenerationFixture.h"

void LostArk::Server::CServerGameplayContractRunner::Run_RevisionProtocol(TESTS& tests)
{

	{
		auto revisionRoomStorage =
			std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
		CGameRoom& revisionRoom = *revisionRoomStorage;
		const GameplayDataRevision activeRevision =
			revisionRoom.m_GameplayCatalog.Get_ActiveRevision();
		GameplayDataRevision candidateRevision = activeRevision;
		candidateRevision.Bytes[0] ^= 0x80u;
		if (!candidateRevision.Is_Valid() ||
			candidateRevision == activeRevision)
		{
			candidateRevision.Bytes[0] = 1u;
		}

		constexpr SESSION_ID ACCEPTED_SESSION = 71001u;
		auto acceptedSession = std::make_shared<CClientSession>(
			ACCEPTED_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		acceptedSession->m_isSendRunning.store(true);
		SERVER_PLAYER acceptedPlayer{};
		acceptedPlayer.iPlayerId = 71001u;
		acceptedPlayer.iNetEntityId = 71001u;
		SERVER_WORLD_ENTITY requiredPinEntity{};
		requiredPinEntity.PinnedDefinitionRevision = candidateRevision;
		revisionRoom.m_WorldEntities.push_back(requiredPinEntity);
		const bool sentAccepted = revisionRoom.Send_Accepted(
			acceptedSession, acceptedPlayer);
		S2C_ENTER_ACCEPTED decodedAccepted{};
		bool acceptedPins = false;
		if (sentAccepted && 1u == acceptedSession->m_OutboundFrames.size())
		{
			const auto& bytes =
				acceptedSession->m_OutboundFrames.front().Bytes;
			PACKET_HEADER header{};
			if (Read_Packet_Header(bytes, header) &&
				PACKET_TYPE::S2C_ENTER_ACCEPTED == header.ePacketType)
			{
				CPacketReader reader{ std::span<const std::uint8_t>(
					bytes.data() + PACKET_HEADER_BYTES,
					bytes.size() - PACKET_HEADER_BYTES) };
				acceptedPins = Read_Message(reader, decodedAccepted) &&
					0u == reader.Get_RemainingSize() &&
					activeRevision == decodedAccepted.ActiveGameplayRevision &&
					1u == decodedAccepted.RequiredPinnedGameplayRevisions.size() &&
					candidateRevision ==
						decodedAccepted.RequiredPinnedGameplayRevisions.front();
			}
		}
		revisionRoom.m_WorldEntities.pop_back();
		acceptedSession->Request_Close();
		tests.Require(acceptedPins,
			"Publish active and required gameplay revision identities on enter accepted");

		namespace fs = std::filesystem;
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		else
		{
			const DWORD moduleLength = GetModuleFileNameW(nullptr,
				pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
			if (0u != moduleLength && moduleLength < pathBuffer.size())
				dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() /
					L"DataFiles";
		}
		std::error_code pathError;
		const fs::path bootstrapPath = fs::canonical(
			dataRoot / L"Gameplay" / L"Gameplay.bootstrap", pathError);
		const fs::path domainHashRoot = fs::temp_directory_path() /
			(L"LostArkValtanDomainHashContract-" +
				std::to_wstring(GetCurrentProcessId()));
		std::error_code domainHashError;
		fs::remove_all(domainHashRoot, domainHashError);
		fs::create_directories(domainHashRoot, domainHashError);
		std::string bootstrapText;
		if (!pathError && !domainHashError)
		{
			std::ifstream source(bootstrapPath, std::ios::binary);
			bootstrapText.assign(std::istreambuf_iterator<char>(source),
				std::istreambuf_iterator<char>());
		}
		std::string valtanOnlyText = bootstrapText;
		std::string nonValtanText = bootstrapText;
		const std::string valtanNeedle =
			"DAMAGE\tdamage.valtan.swing\t220";
		const std::string nonValtanNeedle =
			"DAMAGE\tdamage.player.17000\t100";
		const std::size_t valtanNeedleAt = valtanOnlyText.find(valtanNeedle);
		const std::size_t nonValtanNeedleAt =
			nonValtanText.find(nonValtanNeedle);
		if (std::string::npos != valtanNeedleAt)
			valtanOnlyText.replace(
				valtanNeedleAt, valtanNeedle.size(),
				"DAMAGE\tdamage.valtan.swing\t221");
		if (std::string::npos != nonValtanNeedleAt)
			nonValtanText.replace(
				nonValtanNeedleAt, nonValtanNeedle.size(),
				"DAMAGE\tdamage.player.17000\t101");
		const fs::path valtanOnlyBootstrap =
			domainHashRoot / L"ValtanOnly.bootstrap";
		const fs::path nonValtanBootstrap =
			domainHashRoot / L"NonValtan.bootstrap";
		if (!bootstrapText.empty())
		{
			std::ofstream valtanOnly(
				valtanOnlyBootstrap, std::ios::binary | std::ios::trunc);
			valtanOnly.write(
				valtanOnlyText.data(),
				static_cast<std::streamsize>(valtanOnlyText.size()));
			std::ofstream nonValtan(
				nonValtanBootstrap, std::ios::binary | std::ios::trunc);
			nonValtan.write(
				nonValtanText.data(),
				static_cast<std::streamsize>(nonValtanText.size()));
		}
		GameplayDataRevision baseNonValtanRevision{};
		GameplayDataRevision valtanOnlyNonValtanRevision{};
		GameplayDataRevision changedNonValtanRevision{};
		std::string domainHashStatus;
		const bool domainHashContract =
			std::string::npos != valtanNeedleAt &&
			std::string::npos != nonValtanNeedleAt &&
			CServerApp::Build_NonValtanGameplayRevisionForAdmission(
				bootstrapPath, baseNonValtanRevision, domainHashStatus) &&
			CServerApp::Build_NonValtanGameplayRevisionForAdmission(
				valtanOnlyBootstrap, valtanOnlyNonValtanRevision,
				domainHashStatus) &&
			CServerApp::Build_NonValtanGameplayRevisionForAdmission(
				nonValtanBootstrap, changedNonValtanRevision,
				domainHashStatus) &&
			baseNonValtanRevision == valtanOnlyNonValtanRevision &&
			baseNonValtanRevision != changedNonValtanRevision;
		tests.Require(domainHashContract,
			"Allow repeated Valtan-only bootstrap changes while rejecting stale non-Valtan gameplay drift");
		fs::remove_all(domainHashRoot, domainHashError);
		auto baseGeneration = revisionRoom.Get_ActiveGameplayGeneration();
		auto candidateGeneration = std::make_shared<CGameplayCatalog>();
		const bool candidateLoaded = nullptr != baseGeneration && !pathError &&
			candidateGeneration->Load_FromBootstrap(
				bootstrapPath, activeRevision, candidateRevision);

		auto valtanRoom = std::make_shared<CGameRoom>(
			WORLD_ID::VALTAN_ARENA, baseGeneration);
		auto privateRoom = std::make_shared<CGameRoom>(
			WORLD_ID::CHARACTER_SELECT_ARENA, baseGeneration);
		auto pinFixture =
			std::make_unique<PINNED_MECHANIC_GENERATION_FIXTURE>();
		pinFixture->Boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		pinFixture->Boss.strArchetypeId = "BOSS_VALTAN";
		pinFixture->Boss.strEncounterId = "ENCOUNTER_VALTAN";
		pinFixture->Boss.iCurrentHp = 49125u;
		pinFixture->Boss.iMaximumHp = 60000u;
		pinFixture->Boss.iMaximumHealthBars = 160u;
		pinFixture->Boss.iLastEvaluatedHealthBar = 131u;
		pinFixture->Boss.iPhase = 1u;
		pinFixture->Boss.fPositionX = 151.f;
		pinFixture->Boss.fPositionY = 22.97f;
		pinFixture->Boss.fPositionZ = -122.f;
		pinFixture->Boss.fEngageDistance = 35.f;
		pinFixture->Boss.fMoveSpeed = 3.f;
		pinFixture->Boss.bIntroPatternConsumed = true;
		pinFixture->Boss.bAutomaticPatternSequenceAuditionOverride = true;
		pinFixture->Boss.PinnedDefinitionRevision = activeRevision;
		pinFixture->Target.iPlayerId = 71004u;
		pinFixture->Target.iNetEntityId = 71004u;
		pinFixture->Target.iCurrentHp = 10000u;
		pinFixture->Target.iMaximumHp = 10000u;
		pinFixture->Target.fPositionX = 151.f;
		pinFixture->Target.fPositionY = 22.97f;
		pinFixture->Target.fPositionZ = -128.f;
		pinFixture->Target.isCombatReady = true;
		pinFixture->Players.emplace(
			pinFixture->Target.iPlayerId, pinFixture->Target);
		pinFixture->Brain.Update(
			pinFixture->Boss, pinFixture->Players, *baseGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 10u, {},
			pinFixture->DamageEvents);
		pinFixture->Boss.iCurrentHp = 48750u;
		pinFixture->Brain.Update(
			pinFixture->Boss, pinFixture->Players, *baseGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 11u, {},
			pinFixture->DamageEvents);
		const auto queuedUnderActive = std::find_if(
			pinFixture->Boss.MechanicOccurrences.begin(),
			pinFixture->Boss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
			});
		const bool capturedQueuedOccurrenceGeneration =
			pinFixture->Boss.MechanicOccurrences.end() != queuedUnderActive &&
			activeRevision == queuedUnderActive->PinnedDefinitionRevision &&
			SERVER_BOSS_MECHANIC_STATE::QUEUED == queuedUnderActive->eState &&
			!pinFixture->Boss.strPatternId.empty();
		valtanRoom->m_WorldEntities.push_back(pinFixture->Boss);

		const fs::path runtimePersistenceRoot =
			fs::temp_directory_path() /
			(L"lostark-server-runtime-active-contract-" +
			 std::to_wstring(::GetCurrentProcessId()));
		std::error_code runtimePersistenceError;
		fs::remove_all(runtimePersistenceRoot, runtimePersistenceError);
		auto appStorage = std::make_unique<CServerApp>();
		CServerApp& app = *appStorage;
		app.m_RuntimeActiveGameplayRootOverride = runtimePersistenceRoot;
		app.m_isRuntimeActivePersistenceEnabled = true;
		app.m_pActiveGameplayGeneration = baseGeneration;
		app.m_ActiveGameplayBootstrapContentRevision = activeRevision;
		app.m_ActiveNonValtanGameplayRevision = activeRevision;
		app.m_SharedGameRooms.emplace(WORLD_ID::VALTAN_ARENA, valtanRoom);
		constexpr SESSION_ID REQUESTER_SESSION = 71002u;
		constexpr SESSION_ID PARTICIPANT_SESSION = 71003u;
		auto requester = std::make_shared<CClientSession>(
			REQUESTER_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto participant = std::make_shared<CClientSession>(
			PARTICIPANT_SESSION, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		requester->m_isSendRunning.store(true);
		participant->m_isSendRunning.store(true);
		app.m_Sessions.emplace(REQUESTER_SESSION, requester);
		app.m_Sessions.emplace(PARTICIPANT_SESSION, participant);
		CServerApp::SESSION_GAMEPLAY_BINDING requesterBinding{};
		requesterBinding.eWorldId = WORLD_ID::VALTAN_ARENA;
		requesterBinding.pSimulation = valtanRoom;
		CServerApp::SESSION_GAMEPLAY_BINDING participantBinding{};
		participantBinding.eWorldId = WORLD_ID::CHARACTER_SELECT_ARENA;
		participantBinding.iPrivateArenaOwnerSessionId = PARTICIPANT_SESSION;
		participantBinding.pSimulation = privateRoom;
		app.m_GameplayBindingBySessionId.emplace(
			REQUESTER_SESSION, requesterBinding);
		app.m_GameplayBindingBySessionId.emplace(
			PARTICIPANT_SESSION, participantBinding);
		app.m_CharacterSelectArenas.emplace(PARTICIPANT_SESSION, privateRoom);

		C2S_DATA_REVISION_PREPARE_REQUEST request{};
		request.iTransactionSequence = 71u;
		request.BaseRevision = activeRevision;
		request.CandidateRevision = candidateRevision;
		request.iRequiredPresentationLaneMask =
			GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
		CServerApp::SERVER_CONTROL_EVENT begin{};
		begin.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		begin.iSessionId = REQUESTER_SESSION;
		begin.RevisionRequest = request;
		begin.pCandidateGeneration = candidateGeneration;
		begin.BaseBootstrapContentRevision = activeRevision;
		begin.CandidateBootstrapContentRevision = activeRevision;
		begin.BaseNonValtanGameplayRevision = activeRevision;
		begin.CandidateNonValtanGameplayRevision = activeRevision;
		const bool queuedBegin = app.Queue_ServerControlEvent(std::move(begin));
		app.Advance_ServerControlTransactions();
		const bool preparedEveryClient = queuedBegin && candidateLoaded &&
			app.m_DataRevisionTransaction.Is_Active() &&
			1u == requester->m_OutboundFrames.size() &&
			1u == participant->m_OutboundFrames.size();

		const auto queueReady = [&app, &request](const SESSION_ID sessionId)
		{
			CServerApp::SERVER_CONTROL_EVENT event{};
			event.eKind =
				CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
			event.iSessionId = sessionId;
			event.RevisionResponse.iTransactionSequence =
				request.iTransactionSequence;
			event.RevisionResponse.CandidateRevision =
				request.CandidateRevision;
			event.RevisionResponse.eStatus =
				DATA_REVISION_PREPARE_STATUS::READY;
			event.RevisionResponse.iRequiredPresentationLaneMask =
				request.iRequiredPresentationLaneMask;
			event.RevisionResponse.iPreparedPresentationLaneMask =
				request.iRequiredPresentationLaneMask;
			return app.Queue_ServerControlEvent(std::move(event));
		};
		const bool queuedReady = queueReady(REQUESTER_SESSION) &&
			queueReady(PARTICIPANT_SESSION);
		app.Advance_ServerControlTransactions();
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION packagedRuntime{};
		packagedRuntime.eSource =
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
		packagedRuntime.Revision = activeRevision;
		packagedRuntime.BootstrapContentRevision = activeRevision;
		packagedRuntime.NonValtanGameplayRevision = activeRevision;
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION interruptedCandidateRuntime{};
		interruptedCandidateRuntime.eSource =
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE;
		interruptedCandidateRuntime.Revision = candidateRevision;
		interruptedCandidateRuntime.BootstrapContentRevision = activeRevision;
		interruptedCandidateRuntime.NonValtanGameplayRevision = activeRevision;
		const fs::path interruptedRuntimeRoot = fs::temp_directory_path() /
			(L"lostark-server-runtime-interrupted-contract-" +
			 std::to_wstring(::GetCurrentProcessId()));
		std::error_code interruptedRuntimeError;
		fs::remove_all(interruptedRuntimeRoot, interruptedRuntimeError);
		std::string interruptedRuntimeStatus;
		const bool persistedBeforeJournalCleanup =
			CServerApp::Persist_RuntimeGameplayActivation(
				interruptedRuntimeRoot, 7001u, packagedRuntime,
				interruptedCandidateRuntime, interruptedRuntimeStatus);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredInterruptedRuntime{};
		bool recoveredInterruptedPointer = false;
		const bool recoveredPromotedPointerWithJournal =
			persistedBeforeJournalCleanup &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				interruptedRuntimeRoot, packagedRuntime,
				recoveredInterruptedRuntime, recoveredInterruptedPointer,
				interruptedRuntimeStatus) &&
			recoveredInterruptedPointer &&
			candidateRevision == recoveredInterruptedRuntime.Revision;
		CServerApp::Complete_RuntimeGameplayActivation(interruptedRuntimeRoot);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredAfterJournalCleanup{};
		bool recoveredAfterJournalCleanupPointer = false;
		const bool recoveredCandidateAfterRestartCleanup =
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				interruptedRuntimeRoot, packagedRuntime,
				recoveredAfterJournalCleanup,
				recoveredAfterJournalCleanupPointer,
				interruptedRuntimeStatus) &&
			recoveredAfterJournalCleanupPointer &&
			candidateRevision == recoveredAfterJournalCleanup.Revision;
		fs::remove_all(interruptedRuntimeRoot, interruptedRuntimeError);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredRuntime{};
		bool recoveredPointer = false;
		std::string runtimeRecoveryStatus;
		const bool recoveredCommittedCandidate =
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				runtimePersistenceRoot, packagedRuntime, recoveredRuntime,
				recoveredPointer, runtimeRecoveryStatus) && recoveredPointer &&
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE ==
				recoveredRuntime.eSource &&
			candidateRevision == recoveredRuntime.Revision &&
			activeRevision == recoveredRuntime.BootstrapContentRevision &&
			activeRevision == recoveredRuntime.NonValtanGameplayRevision;
		const bool committedEveryRoom = queuedReady &&
			!app.m_DataRevisionTransaction.Is_Active() &&
			candidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			candidateRevision == valtanRoom->Get_ActiveGameplayGeneration()->
				Get_ActiveRevision() &&
			candidateRevision == privateRoom->Get_ActiveGameplayGeneration()->
				Get_ActiveRevision() &&
			nullptr != valtanRoom->Resolve_GameplayGeneration(activeRevision) &&
			2u == requester->m_OutboundFrames.size() &&
			2u == participant->m_OutboundFrames.size();
		tests.Require(
			capturedQueuedOccurrenceGeneration && preparedEveryClient &&
			committedEveryRoom && recoveredCommittedCandidate &&
			recoveredPromotedPointerWithJournal &&
			recoveredCandidateAfterRestartCleanup,
			"Commit one candidate across every room and durable restart pointer only after every bound client READY");

		/* Model an A occurrence that held the boss at 158 bars while the process
		committed B. The first idle B evaluation must reconcile the untriggered
		159 mechanic even though there can no longer be a 160 -> 159 crossing.
		No target deliberately delays consumption for one tick, proving the queued
		occurrence itself, rather than the idle boss pin, owns B. */
		SERVER_WORLD_ENTITY& reconcileBoss = pinFixture->ReconcileBoss;
		reconcileBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		reconcileBoss.strArchetypeId = "BOSS_VALTAN";
		reconcileBoss.strEncounterId = "ENCOUNTER_VALTAN";
		reconcileBoss.iCurrentHp = 59250u;
		reconcileBoss.iMaximumHp = 60000u;
		reconcileBoss.iMaximumHealthBars = 160u;
		reconcileBoss.iLastEvaluatedHealthBar = 158u;
		reconcileBoss.iLastHealthMechanicGenerationEpoch = 1u;
		reconcileBoss.iPhase = 1u;
		reconcileBoss.fPositionX = 151.f;
		reconcileBoss.fPositionY = 22.97f;
		reconcileBoss.fPositionZ = -122.f;
		reconcileBoss.fEngageDistance = 35.f;
		reconcileBoss.fMoveSpeed = 3.f;
		reconcileBoss.bIntroPatternConsumed = true;
		reconcileBoss.bAutomaticPatternSequenceAuditionOverride = true;
		reconcileBoss.PinnedDefinitionRevision = activeRevision;
		pinFixture->Brain.Update(
			reconcileBoss, pinFixture->ReconcilePlayers, *candidateGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 20u, {},
			pinFixture->DamageEvents, candidateGeneration.get(), 2u);
		const auto reconciledOccurrence = std::find_if(
			reconcileBoss.MechanicOccurrences.begin(),
			reconcileBoss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_ARMOR_BREAK_OPENING" == occurrence.strPatternId;
			});
		const bool reconciledBelowThresholdWithoutTarget =
			reconcileBoss.MechanicOccurrences.end() != reconciledOccurrence &&
			SERVER_BOSS_MECHANIC_STATE::QUEUED ==
				reconciledOccurrence->eState &&
			candidateRevision ==
				reconciledOccurrence->PinnedDefinitionRevision &&
			reconcileBoss.strPatternId.empty();
		pinFixture->ReconcilePlayers.emplace(
			pinFixture->Target.iPlayerId, pinFixture->Target);
		pinFixture->Brain.Update(
			reconcileBoss, pinFixture->ReconcilePlayers, *candidateGeneration,
			valtanRoom->m_ServerNavigation, 1.f / 30.f, 21u, {},
			pinFixture->DamageEvents, candidateGeneration.get(), 2u);
		const bool delayedReconciledOccurrenceUsesCandidate =
			reconcileBoss.MechanicOccurrences.end() != reconciledOccurrence &&
			SERVER_BOSS_MECHANIC_STATE::ACTIVE ==
				reconciledOccurrence->eState &&
			"VALTAN_ARMOR_BREAK_OPENING" == reconcileBoss.strPatternId &&
			candidateRevision == reconcileBoss.PinnedDefinitionRevision;
		tests.Require(
			reconciledBelowThresholdWithoutTarget &&
			delayedReconciledOccurrenceUsesCandidate,
			"Reconcile an untriggered threshold below current HP on generation change and pin delayed consumption to the evaluating catalog");

		/* Keep occurrence execution on A while changing the non-transition armour
		break from 159 to 158 in B. Falling to 159 must not be interpreted by A's
		threshold table; the exact 158 crossing must queue one B occurrence and
		pin B. The 109 phase boundary is independently topology-locked below. */
		const fs::path thresholdRoot = fs::temp_directory_path() /
			(L"LostArkValtanThresholdGenerationContract-" +
			 std::to_wstring(GetCurrentProcessId()));
		std::error_code thresholdError;
		fs::remove_all(thresholdRoot, thresholdError);
		fs::create_directories(thresholdRoot, thresholdError);
		std::string thresholdBootstrapText = bootstrapText;
		const std::string thresholdA =
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARMOR_BREAK_OPENING\tvaltan.mechanic.armor-break-opening\tHEALTH_BAR\t0\t0\t159\t1\t0\t0\t0\t100\t4\tANY\tANY\t0";
		const std::string thresholdB =
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARMOR_BREAK_OPENING\tvaltan.mechanic.armor-break-opening\tHEALTH_BAR\t0\t0\t158\t1\t0\t0\t0\t100\t4\tANY\tANY\t0";
		const std::size_t thresholdRowAt =
			thresholdBootstrapText.find(thresholdA);
		if (std::string::npos != thresholdRowAt)
			thresholdBootstrapText.replace(
				thresholdRowAt, thresholdA.size(), thresholdB);
		const fs::path thresholdBootstrap =
			thresholdRoot / L"Gameplay.bootstrap";
		if (!thresholdError && std::string::npos != thresholdRowAt)
		{
			std::ofstream stream(
				thresholdBootstrap, std::ios::binary | std::ios::trunc);
			stream.write(thresholdBootstrapText.data(),
				static_cast<std::streamsize>(thresholdBootstrapText.size()));
		}
		GameplayDataRevision thresholdBootstrapRevision{};
		GameplayDataRevision thresholdDefinitionRevision = candidateRevision;
		thresholdDefinitionRevision.Bytes[8] ^= 0x40u;
		if (!thresholdDefinitionRevision.Is_Valid() ||
			thresholdDefinitionRevision == activeRevision ||
			thresholdDefinitionRevision == candidateRevision)
		{
			thresholdDefinitionRevision.Bytes[9] ^= 1u;
		}
		std::string thresholdStatus;
		auto thresholdGeneration = std::make_shared<CGameplayCatalog>();
		const bool thresholdAdmissionInputsReady = !thresholdError &&
			std::string::npos != thresholdRowAt;
		bool thresholdGenerationLoaded = false;
		if (thresholdAdmissionInputsReady)
		{
			// Hashing and catalog validation have sizeable but bounded call trees. Keep
			// this synthetic second generation off the already-large contract-test
			// driver frame so the executable remains valid on the default 1 MiB
			// Windows thread stack used by production Server.exe.
			struct THRESHOLD_GENERATION_LOAD_CONTEXT final
			{
				std::shared_ptr<CGameplayCatalog>* pGeneration = nullptr;
				const fs::path* pBootstrap = nullptr;
				GameplayDataRevision* pBootstrapRevision = nullptr;
				const GameplayDataRevision* pDefinitionRevision = nullptr;
				std::string* pStatus = nullptr;
				bool isLoaded = false;
			};
			THRESHOLD_GENERATION_LOAD_CONTEXT loadContext{
				&thresholdGeneration, &thresholdBootstrap,
				&thresholdBootstrapRevision, &thresholdDefinitionRevision,
				&thresholdStatus, false };
			const auto loadThresholdGeneration = [](void* opaque)
			{
				THRESHOLD_GENERATION_LOAD_CONTEXT& context =
					*static_cast<THRESHOLD_GENERATION_LOAD_CONTEXT*>(opaque);
				context.isLoaded =
					CServerApp::Hash_GameplayFileForAdmission(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pStatus) &&
					(*context.pGeneration)->Load_FromBootstrap(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pDefinitionRevision);
			};
			thresholdGenerationLoaded = Run_WithContractWorkerStack(
				loadThresholdGeneration, &loadContext) && loadContext.isLoaded;
		}
		const auto findHealthThreshold = [](const CGameplayCatalog& source,
			const std::string_view patternId)
		{
			const auto* definitions =
				source.Find_BossPatterns("ENCOUNTER_VALTAN");
			if (nullptr == definitions) return std::uint32_t{ 0u };
			const auto found = std::find_if(
				definitions->begin(), definitions->end(),
				[patternId](const BOSS_PATTERN_DEFINITION& definition)
				{
					return definition.strPatternId == patternId;
				});
			return definitions->end() == found ?
				std::uint32_t{ 0u } : found->iTriggerHealthBar;
		};
		tests.Require(
			thresholdGenerationLoaded &&
			159u == findHealthThreshold(*baseGeneration,
				"VALTAN_ARMOR_BREAK_OPENING") &&
			158u == findHealthThreshold(*thresholdGeneration,
				"VALTAN_ARMOR_BREAK_OPENING"),
			"Stage exact A159 and B158 non-transition threshold generations for routing");
		SERVER_WORLD_ENTITY& thresholdBoss = pinFixture->ThresholdBoss;
		thresholdBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		thresholdBoss.strArchetypeId = "BOSS_VALTAN";
		thresholdBoss.strEncounterId = "ENCOUNTER_VALTAN";
		thresholdBoss.iCurrentHp = 59625u;
		thresholdBoss.iMaximumHp = 60000u;
		thresholdBoss.iMaximumHealthBars = 160u;
		thresholdBoss.iLastEvaluatedHealthBar = 160u;
		thresholdBoss.iLastHealthMechanicGenerationEpoch = 1u;
		thresholdBoss.iPhase = 1u;
		thresholdBoss.fPositionX = 151.f;
		thresholdBoss.fPositionY = 22.97f;
		thresholdBoss.fPositionZ = -122.f;
		thresholdBoss.fEngageDistance = 35.f;
		thresholdBoss.fMoveSpeed = 3.f;
		thresholdBoss.bIntroPatternConsumed = true;
		thresholdBoss.bAutomaticPatternSequenceAuditionOverride = true;
		thresholdBoss.PinnedDefinitionRevision = activeRevision;
		pinFixture->ReconcilePlayers.clear();
		if (thresholdGenerationLoaded)
		{
			const auto* definitions = thresholdGeneration->Find_BossPatterns(
				"ENCOUNTER_VALTAN");
			if (nullptr != definitions)
			{
				for (const BOSS_PATTERN_DEFINITION& definition : *definitions)
				{
					if (BOSS_PATTERN_SELECTION::HEALTH_BAR !=
						definition.eSelection ||
						"VALTAN_ARMOR_BREAK_OPENING" == definition.strPatternId)
						continue;
					SERVER_BOSS_MECHANIC_OCCURRENCE completed{};
					completed.strPatternId = definition.strPatternId;
					completed.PinnedDefinitionRevision =
						thresholdDefinitionRevision;
					completed.eState =
						SERVER_BOSS_MECHANIC_STATE::COMPLETED;
					completed.iTriggerHealthBar =
						definition.iTriggerHealthBar;
					thresholdBoss.MechanicOccurrences.push_back(
						std::move(completed));
				}
			}
			pinFixture->Brain.Update(
				thresholdBoss, pinFixture->ReconcilePlayers, *baseGeneration,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 30u, {},
				pinFixture->DamageEvents, thresholdGeneration.get(), 2u);
		}
		const auto countThresholdOccurrence = [&thresholdBoss]()
		{
			return static_cast<std::size_t>(std::count_if(
				thresholdBoss.MechanicOccurrences.begin(),
				thresholdBoss.MechanicOccurrences.end(),
				[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
				{
					return "VALTAN_ARMOR_BREAK_OPENING" ==
						occurrence.strPatternId;
				}));
		};
		const bool oldOnlyCrossingDidNotQueue =
			thresholdGenerationLoaded && 0u == countThresholdOccurrence() &&
			159u == thresholdBoss.iLastEvaluatedHealthBar &&
			2u == thresholdBoss.iLastHealthMechanicGenerationEpoch;
		tests.Require(oldOnlyCrossingDidNotQueue,
			"Ignore the old A159-only crossing while active B owns threshold evaluation");
		thresholdBoss.iCurrentHp = 59250u;
		if (thresholdGenerationLoaded)
		{
			pinFixture->Brain.Update(
				thresholdBoss, pinFixture->ReconcilePlayers, *baseGeneration,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 31u, {},
				pinFixture->DamageEvents, thresholdGeneration.get(), 2u);
		}
		const auto thresholdOccurrence = std::find_if(
			thresholdBoss.MechanicOccurrences.begin(),
			thresholdBoss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_ARMOR_BREAK_OPENING" == occurrence.strPatternId;
			});
		const bool activeCrossingQueuedExactCandidate =
			1u == countThresholdOccurrence() &&
			thresholdBoss.MechanicOccurrences.end() != thresholdOccurrence &&
			158u == thresholdOccurrence->iTriggerHealthBar &&
			thresholdDefinitionRevision ==
				thresholdOccurrence->PinnedDefinitionRevision;
		tests.Require(
			activeCrossingQueuedExactCandidate,
			"Evaluate new threshold crossings from active B while an old A occurrence catalog advances independently");
		fs::remove_all(thresholdRoot, thresholdError);

		const auto replaceBootstrapRow = [](
			std::string& text, const std::string_view before,
			const std::string_view after)
		{
			const std::size_t at = text.find(before);
			if (std::string::npos == at || std::string::npos !=
				text.find(before, at + before.size()))
			{
				return false;
			}
			text.replace(at, before.size(), after);
			return true;
		};
		const auto removeBootstrapRow = [](
			std::string& text, const std::string_view row)
		{
			const std::size_t at = text.find(row);
			if (std::string::npos == at ||
				(0u != at && '\n' != text[at - 1u]) ||
				std::string::npos != text.find(row, at + row.size()))
			{
				return false;
			}
			const std::size_t lineEnd = text.find('\n', at);
			text.erase(at, std::string::npos == lineEnd ?
				text.size() - at : lineEnd - at + 1u);

			const std::size_t headerEnd = text.find('\n');
			const std::size_t countTab = text.rfind('\t', headerEnd);
			std::size_t countEnd = headerEnd;
			if (std::string::npos == headerEnd ||
				std::string::npos == countTab || countTab + 1u >= countEnd)
			{
				return false;
			}
			if (0u != countEnd && '\r' == text[countEnd - 1u])
				--countEnd;
			const unsigned long count = std::stoul(text.substr(
				countTab + 1u, countEnd - countTab - 1u));
			if (0u == count) return false;
			text.replace(
				countTab + 1u, countEnd - countTab - 1u,
				std::to_string(count - 1u));
			return true;
		};
		const auto loadBootstrapVariant = [&candidateRevision](
			const std::wstring_view suffix,
			const std::string& bytes,
			std::shared_ptr<CGameplayCatalog> generation = nullptr,
			GameplayDataRevision definitionRevision = {})
			-> std::shared_ptr<CGameplayCatalog>
		{
			const fs::path root = fs::temp_directory_path() /
				(L"LostArkGameplayV19Variant-" +
				 std::to_wstring(GetCurrentProcessId()) + L"-" +
				 std::wstring(suffix));
			std::error_code error;
			fs::remove_all(root, error);
			error.clear();
			fs::create_directories(root, error);
			const fs::path bootstrap = root / L"Gameplay.bootstrap";
			if (!error)
			{
				std::ofstream stream(
					bootstrap, std::ios::binary | std::ios::trunc);
				stream.write(bytes.data(),
					static_cast<std::streamsize>(bytes.size()));
				if (!stream.good()) error = std::make_error_code(
					std::errc::io_error);
			}
			const fs::path canonical = error ? fs::path{} :
				fs::canonical(bootstrap, error);
			if (nullptr == generation)
				generation = std::make_shared<CGameplayCatalog>();
			GameplayDataRevision bootstrapRevision{};
			std::string status;
			struct VARIANT_LOAD_CONTEXT final
			{
				std::shared_ptr<CGameplayCatalog>* pGeneration = nullptr;
				const fs::path* pBootstrap = nullptr;
				GameplayDataRevision* pBootstrapRevision = nullptr;
				const GameplayDataRevision* pDefinitionRevision = nullptr;
				std::string* pStatus = nullptr;
				bool isLoaded = false;
			};
			if (!definitionRevision.Is_Valid()) definitionRevision = candidateRevision;
			VARIANT_LOAD_CONTEXT context{
				&generation, &canonical, &bootstrapRevision,
				&definitionRevision, &status, false };
			const auto load = [](void* opaque)
			{
				VARIANT_LOAD_CONTEXT& context =
					*static_cast<VARIANT_LOAD_CONTEXT*>(opaque);
				context.isLoaded =
					CServerApp::Hash_GameplayFileForAdmission(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pStatus) &&
					(*context.pGeneration)->Load_FromBootstrap(
						*context.pBootstrap, *context.pBootstrapRevision,
						*context.pDefinitionRevision);
			};
			const bool loaded = !error && nullptr != generation &&
				Run_WithContractWorkerStack(load, &context) && context.isLoaded;
			fs::remove_all(root, error);
			return loaded ? generation : nullptr;
		};
		{
			const auto zeroSourceAction = [](std::string& text, const std::string& prefix)
			{
				const std::size_t row = text.find(prefix);
				if (std::string::npos == row) return false;
				const std::size_t action = row + prefix.size();
				const std::size_t actionEnd = text.find('\t', action);
				if (std::string::npos == actionEnd) return false;
				text.replace(action, actionEnd - action, "0");
				return true;
			};
			const std::string koukuSourcePrefix =
				"PATTERNSOURCE\tENCOUNTER_KAKULSAYDON_G1\tKAKULSAYDON_G1_PATTERN_1\t";
			std::string zeroSourceText = bootstrapText;
			const bool zeroSourceReady = zeroSourceAction(zeroSourceText, koukuSourcePrefix);
			const auto zeroSourceCatalog = zeroSourceReady ?
				loadBootstrapVariant(L"kouku-source-action-zero", zeroSourceText) : nullptr;
			const auto* zeroSourcePatterns = nullptr == zeroSourceCatalog ? nullptr :
				zeroSourceCatalog->Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1");
			const bool preservedZero = nullptr != zeroSourcePatterns &&
				std::any_of(zeroSourcePatterns->begin(), zeroSourcePatterns->end(),
					[](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == "KAKULSAYDON_G1_PATTERN_1" &&
							0u == pattern.iSourcePrimaryActionId;
					});
			std::string duplicateZeroText = zeroSourceText;
			const std::size_t sourceAt = duplicateZeroText.find(koukuSourcePrefix);
			const std::size_t sourceEnd = std::string::npos == sourceAt ? std::string::npos :
				duplicateZeroText.find('\n', sourceAt);
			if (std::string::npos != sourceEnd)
				duplicateZeroText.insert(sourceEnd + 1u,
					duplicateZeroText.substr(sourceAt, sourceEnd - sourceAt + 1u));
			auto duplicateZeroCatalog = std::make_shared<CGameplayCatalog>();
			const bool duplicateRejected = std::string::npos != sourceEnd &&
				nullptr == loadBootstrapVariant(L"kouku-source-action-zero-duplicate",
					duplicateZeroText, duplicateZeroCatalog) &&
				duplicateZeroCatalog->Get_Status() ==
					"Boss pattern source timing has no owner or is duplicated";
			tests.Require(preservedZero && duplicateRejected,
				"Load referenced Kouku action zero and reject duplicate zero-valued source rows");
		}
		std::string truncatedMotionBootstrap = bootstrapText;
		const std::string motionRowPrefix =
			"PATTERNMOTION\tENCOUNTER_VALTAN\tVALTAN_HIGH_JUMP\t";
		const std::size_t motionRowAt =
			truncatedMotionBootstrap.find(motionRowPrefix);
		const std::size_t motionRowEnd = std::string::npos == motionRowAt ?
			std::string::npos :
			truncatedMotionBootstrap.find('\n', motionRowAt);
		std::size_t tenthMotionTab = motionRowAt;
		for (std::uint32_t tabIndex = 0u;
			tabIndex < 10u && std::string::npos != tenthMotionTab;
			++tabIndex)
		{
			tenthMotionTab = truncatedMotionBootstrap.find(
				'\t', 0u == tabIndex ? tenthMotionTab : tenthMotionTab + 1u);
		}
		const bool madeTruncatedMotionRow =
			std::string::npos != motionRowAt &&
			std::string::npos != motionRowEnd &&
			std::string::npos != tenthMotionTab &&
			tenthMotionTab < motionRowEnd;
		if (madeTruncatedMotionRow)
		{
			truncatedMotionBootstrap.erase(
				tenthMotionTab, motionRowEnd - tenthMotionTab);
		}
		auto motionRollbackCatalog = std::make_shared<CGameplayCatalog>();
		const bool motionRollbackBaselineLoaded =
			motionRollbackCatalog->Load();
		const GameplayDataRevision motionRollbackRevision =
			motionRollbackCatalog->Get_ActiveRevision();
		const bool rejectedTruncatedMotionRow =
			madeTruncatedMotionRow && motionRollbackBaselineLoaded &&
			nullptr == loadBootstrapVariant(
				L"truncated-pattern-motion", truncatedMotionBootstrap,
				motionRollbackCatalog) &&
			motionRollbackRevision ==
				motionRollbackCatalog->Get_ActiveRevision() &&
			std::string::npos != motionRollbackCatalog->Get_Status().find(
				"Boss pattern motion row is invalid");
		tests.Require(
			rejectedTruncatedMotionRow,
			"Reject a truncated PATTERNMOTION row and preserve the active gameplay generation");

		// The packaged Product order is user authored. Build small admitted variants
		// from its definitions instead of assuming the old fixed 28-step program.
		std::vector<std::string> publishedSequenceRows;
		std::istringstream publishedRows(bootstrapText);
		for (std::string row; std::getline(publishedRows, row);)
		{
			if (!row.empty() && '\r' == row.back()) row.pop_back();
			if (row.starts_with("PATTERNSEQUENCE\tENCOUNTER_VALTAN\t") ||
				row.starts_with("PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\t"))
				publishedSequenceRows.push_back(std::move(row));
		}
		const auto makeProductSequenceBootstrap = [&bootstrapText, &publishedSequenceRows,
			&removeBootstrapRow](const std::string& sequenceId,
				const std::vector<std::string>& patternIds)
		{
			std::string text = bootstrapText;
			if (publishedSequenceRows.empty()) return std::string{};
			for (const auto& row : publishedSequenceRows)
				if (!removeBootstrapRow(text, row)) return std::string{};
			const auto headerEnd = text.find('\n');
			const auto countTab = text.rfind('\t', headerEnd);
			if (std::string::npos == headerEnd || std::string::npos == countTab)
				return std::string{};
			auto countEnd = headerEnd;
			if (0u != countEnd && '\r' == text[countEnd - 1u]) --countEnd;
			const auto oldCount = std::stoul(text.substr(countTab + 1u, countEnd - countTab - 1u));
			text.replace(countTab + 1u, countEnd - countTab - 1u,
				std::to_string(oldCount + 1u + patternIds.size()));
			if (!text.empty() && '\n' != text.back()) text += '\n';
			text += "PATTERNSEQUENCE\tENCOUNTER_VALTAN\t" + sequenceId +
				"\tORDERED_ONCE_THEN_IDLE\t100\t" + std::to_string(patternIds.size()) + "\n";
			for (std::size_t index = 0u; index < patternIds.size(); ++index)
				text += "PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\t" + sequenceId + "\t" +
					std::to_string(index) + "\t" + patternIds[index] + "\n";
			return text;
		};
		const std::vector<std::string> maximumSequenceOrder(
			MAX_VALTAN_PATTERN_FLOW_SLOTS, "VALTAN_WHIRLWIND");
		const std::string maximumSequenceBootstrap = makeProductSequenceBootstrap(
			"sequence.valtan.contract.maximum", maximumSequenceOrder);
		const auto maximumSequenceCatalog = loadBootstrapVariant(
			L"sequence-maximum", maximumSequenceBootstrap);
		const BOSS_PATTERN_SEQUENCE_DEFINITION* maximumLoadedSequence =
			nullptr == maximumSequenceCatalog ? nullptr :
			maximumSequenceCatalog->Find_BossPatternSequence("ENCOUNTER_VALTAN");
		tests.Require(
			nullptr != maximumLoadedSequence &&
			maximumLoadedSequence->PatternIds == maximumSequenceOrder &&
			maximumLoadedSequence->iExpectedStepCount ==
				MAX_VALTAN_PATTERN_FLOW_SLOTS,
			"Load the maximum 255-step saved Product sequence");
		const std::vector<std::string> overflowSequenceOrder(
			MAX_VALTAN_PATTERN_FLOW_SLOTS + 1u, "VALTAN_WHIRLWIND");
		const std::string overflowSequenceBootstrap = makeProductSequenceBootstrap(
			"sequence.valtan.contract.overflow", overflowSequenceOrder);
		const std::vector<std::string> repeatOrder{
			"VALTAN_WHIRLWIND", "VALTAN_FIST_IN_OUT", "VALTAN_WHIRLWIND" };
		const std::string sequenceBootstrap = makeProductSequenceBootstrap(
			"sequence.valtan.contract.repeat", repeatOrder);
		const std::string sequenceHeader =
			"PATTERNSEQUENCE\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t"
			"ORDERED_ONCE_THEN_IDLE\t100\t3";
		const std::vector<std::string> sequenceSteps{
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t0\tVALTAN_WHIRLWIND",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t1\tVALTAN_FIST_IN_OUT",
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t2\tVALTAN_WHIRLWIND" };
		std::string nonUniformSequenceBootstrap = sequenceBootstrap;
		bool madeNonUniformSequence = replaceBootstrapRow(
			nonUniformSequenceBootstrap, sequenceSteps[0u], sequenceSteps[0u] + "\t100");
		madeNonUniformSequence = replaceBootstrapRow(
			nonUniformSequenceBootstrap, sequenceSteps[1u], sequenceSteps[1u] + "\t900") &&
			madeNonUniformSequence;
		madeNonUniformSequence = replaceBootstrapRow(
			nonUniformSequenceBootstrap, sequenceSteps[2u], sequenceSteps[2u] + "\t0") &&
			madeNonUniformSequence;
		const auto nonUniformSequenceCatalog = loadBootstrapVariant(
			L"sequence-non-uniform-pursuit", nonUniformSequenceBootstrap);
		const BOSS_PATTERN_SEQUENCE_DEFINITION* nonUniformSequence =
			nullptr == nonUniformSequenceCatalog ? nullptr :
				nonUniformSequenceCatalog->Find_BossPatternSequence("ENCOUNTER_VALTAN");
		tests.Require(
			madeNonUniformSequence && nullptr != nonUniformSequence &&
			std::vector<std::uint32_t>{ 100u, 900u } ==
				nonUniformSequence->TransitionPursuitMs &&
			std::vector<std::uint32_t>{ 3u, 27u } ==
				nonUniformSequence->TransitionPursuitTicks,
			"Load distinct pursuit timing for each saved Product sequence transition");
		std::string missingSequenceBootstrap = sequenceBootstrap;
		bool removedSequence = removeBootstrapRow(missingSequenceBootstrap, sequenceHeader);
		for (const std::string& row : sequenceSteps)
			removedSequence = removeBootstrapRow(missingSequenceBootstrap, row) && removedSequence;
		std::string missingSequenceStepBootstrap = sequenceBootstrap;
		const bool removedFinalSequenceStep = removeBootstrapRow(
			missingSequenceStepBootstrap, sequenceSteps.back());
		std::string invalidSequencePursuitBootstrap = sequenceBootstrap;
		const bool madeInvalidSequencePursuit = replaceBootstrapRow(
			invalidSequencePursuitBootstrap, sequenceHeader,
			"PATTERNSEQUENCE\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t"
			"ORDERED_ONCE_THEN_IDLE\t0\t3");
		std::string duplicateSequenceStepBootstrap = sequenceBootstrap;
		const bool madeDuplicateSequenceStep = replaceBootstrapRow(
			duplicateSequenceStepBootstrap, sequenceSteps[1u],
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t0\tVALTAN_FIST_IN_OUT");
		std::string nonContiguousSequenceBootstrap = sequenceBootstrap;
		const bool madeNonContiguousSequence = replaceBootstrapRow(
			nonContiguousSequenceBootstrap, sequenceSteps[1u],
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t2\tVALTAN_FIST_IN_OUT");
		std::string outOfBoundsSequenceBootstrap = sequenceBootstrap;
		const bool madeOutOfBoundsSequence = replaceBootstrapRow(
			outOfBoundsSequenceBootstrap, sequenceSteps[2u],
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t3\tVALTAN_WHIRLWIND");
		std::string unknownSequenceStepBootstrap = sequenceBootstrap;
		const bool madeUnknownSequenceStep = replaceBootstrapRow(
			unknownSequenceStepBootstrap, sequenceSteps[1u],
			"PATTERNSEQUENCESTEP\tENCOUNTER_VALTAN\tsequence.valtan.contract.repeat\t1\tVALTAN_UNKNOWN_SEQUENCE_STEP");
		const std::string dashTimeoutRecoveryRow =
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_DASH_CHARGE\t"
			"valtan.attack.dash-charge.active\tTIMEOUT\t"
			"valtan.attack.dash-charge.recovery";
		std::string divergentDashRecoveryBootstrap = bootstrapText;
		const bool madeDivergentDashRecovery = replaceBootstrapRow(
			divergentDashRecoveryBootstrap, dashTimeoutRecoveryRow,
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_DASH_CHARGE\t"
			"valtan.attack.dash-charge.active\tTIMEOUT\tvaltan.unknown.recovery");
		const std::string counterProxyRow =
			"PATTERNSTAGECOUNTERPROXY\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.step-07\t"
			"BOSS_LOCAL_CIRCLE\t1\t0\t2.25\t0";
		std::string invalidPartDamageBootstrap = bootstrapText;
		const bool madeInvalidPartDamage = replaceBootstrapRow(
			invalidPartDamageBootstrap, counterProxyRow,
			"PATTERNSTAGEPARTDAMAGE\tENCOUNTER_VALTAN\tVALTAN_DASH_CHARGE\t"
			"valtan.attack.dash-charge.recovery\tUNKNOWN");
		std::string invalidCounterProxyBootstrap = bootstrapText;
		const bool madeInvalidCounterProxy = replaceBootstrapRow(
			invalidCounterProxyBootstrap, counterProxyRow,
			"PATTERNSTAGECOUNTERPROXY\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.step-07\t"
			"BOSS_LOCAL_CIRCLE\t1\t0\t21\t0");
		const std::string forwardArcRow =
			"PATTERNSTAGECOUNTERPROXY\tENCOUNTER_VALTAN\tVALTAN_TRIPLE_COUNTER\t"
			"valtan.reactive.triple-counter.first\t"
			"BOSS_FORWARD_ARC\t0\t0\t0\t180";
		std::string invalidForwardArcBootstrap = bootstrapText;
		const bool madeInvalidForwardArc = replaceBootstrapRow(
			invalidForwardArcBootstrap, forwardArcRow,
			"PATTERNSTAGECOUNTERPROXY\tENCOUNTER_VALTAN\tVALTAN_TRIPLE_COUNTER\t"
			"valtan.reactive.triple-counter.first\t"
			"BOSS_FORWARD_ARC\t0\t0\t1\t180");
		const std::string magicResponseRow =
			"PATTERNSTAGERESPONSE\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t"
			"ACCUMULATED_HEALTH_DAMAGE\t1000";
		std::string invalidMagicResponseBootstrap = bootstrapText;
		const bool madeInvalidMagicResponse = replaceBootstrapRow(
			invalidMagicResponseBootstrap, magicResponseRow,
			"PATTERNSTAGERESPONSE\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\tUNKNOWN\t1000");
		std::string zeroMagicResponseBootstrap = bootstrapText;
		const bool madeZeroMagicResponse = replaceBootstrapRow(
			zeroMagicResponseBootstrap, magicResponseRow,
			"PATTERNSTAGERESPONSE\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t"
			"ACCUMULATED_HEALTH_DAMAGE\t0");
		const std::string magicVerticalOffsetRow =
			"PATTERNSTAGEVERTICALOFFSET\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t0.5";
		std::string zeroMagicVerticalOffsetBootstrap = bootstrapText;
		const bool madeZeroMagicVerticalOffset = replaceBootstrapRow(
			zeroMagicVerticalOffsetBootstrap, magicVerticalOffsetRow,
			"PATTERNSTAGEVERTICALOFFSET\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t0");
		const std::string magicFollowupRow =
			"PATTERNSTAGEFOLLOWUP\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t"
			"HEALTH_DAMAGE_THRESHOLD_REACHED\tVALTAN_GROGGY_FOLLOWUP";
		std::string unknownMagicFollowupBootstrap = bootstrapText;
		const bool madeUnknownMagicFollowup = replaceBootstrapRow(
			unknownMagicFollowupBootstrap, magicFollowupRow,
			"PATTERNSTAGEFOLLOWUP\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t"
			"HEALTH_DAMAGE_THRESHOLD_REACHED\tVALTAN_UNKNOWN_FOLLOWUP");
		std::string selfMagicFollowupBootstrap = bootstrapText;
		const bool madeSelfMagicFollowup = replaceBootstrapRow(
			selfMagicFollowupBootstrap, magicFollowupRow,
			"PATTERNSTAGEFOLLOWUP\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t"
			"HEALTH_DAMAGE_THRESHOLD_REACHED\tVALTAN_STAGGER_SLOT");
		const std::string magicTimeoutRow =
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\tTIMEOUT\t"
			"valtan.authoring.stagger-slot.final-attack";
		std::string bothMagicBranchKindsBootstrap = bootstrapText;
		const bool madeBothMagicBranchKinds = replaceBootstrapRow(
			bothMagicBranchKindsBootstrap, magicTimeoutRow,
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_STAGGER_SLOT\t"
			"valtan.authoring.stagger-slot.channel\t"
			"HEALTH_DAMAGE_THRESHOLD_REACHED\t"
			"valtan.authoring.stagger-slot.final-attack");
		const std::string groggyTerminalRow =
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_GROGGY_FOLLOWUP\t"
			"valtan.followup.groggy.active\tTIMEOUT\t-";
		std::string cyclicMagicFollowupBootstrap = bootstrapText;
		const bool madeCyclicMagicFollowup = replaceBootstrapRow(
			cyclicMagicFollowupBootstrap, groggyTerminalRow,
			"PATTERNSTAGEFOLLOWUP\tENCOUNTER_VALTAN\tVALTAN_GROGGY_FOLLOWUP\t"
			"valtan.followup.groggy.active\tTIMEOUT\tVALTAN_STAGGER_SLOT");
		const std::string rearTargetPolicyRow =
			"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_CATCH_BREATH\t"
			"NORMAL\t1\t3\tLOCK_RANDOM_ALIVE_BEHIND_ON_START\t"
			"LOCK_FACING_ON_START";
		std::string invalidRearTargetPolicyBootstrap = bootstrapText;
		const bool madeInvalidRearTargetPolicy = replaceBootstrapRow(
			invalidRearTargetPolicyBootstrap, rearTargetPolicyRow,
			"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_CATCH_BREATH\t"
			"NORMAL\t1\t3\tLOCK_RANDOM_ALIVE_BEHIND_ON_START_BROKEN\t"
			"LOCK_FACING_ON_START");
		const std::string trashCounterStagePrefix =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TRASH\t6\tSTEP_07\t"
			"valtan.sequence.center-trash-rush-if.step-07\tWINDUP\t1000";
		std::string invalidTrashCounterKindBootstrap = bootstrapText;
		const bool madeInvalidTrashCounterKind = replaceBootstrapRow(
			invalidTrashCounterKindBootstrap, trashCounterStagePrefix,
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TRASH\t6\tSTEP_07\t"
			"valtan.sequence.center-trash-rush-if.step-07\tACTIVE\t1000");
		const std::string trashGroggyExitRow =
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.groggy\t2\tEXIT\t"
			"SET_BOSS_FLAG\tboss.flag.groggy\t0\t0";
		std::string unpairedTrashGroggyBootstrap = bootstrapText;
		const bool removedTrashGroggyExit = removeBootstrapRow(
			unpairedTrashGroggyBootstrap, trashGroggyExitRow);
		const std::string trashCounterBranchRow =
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.step-07\tCOUNTER_HIT\t"
			"valtan.sequence.center-trash-rush-if.groggy";
		std::string crossPatternTrashCounterBootstrap = bootstrapText;
		const bool madeCrossPatternTrashCounter = replaceBootstrapRow(
			crossPatternTrashCounterBootstrap, trashCounterBranchRow,
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.step-07\tCOUNTER_HIT\t"
			"valtan.sequence.rush-if.groggy");
		const std::string trashCounterTimeoutRow =
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TRASH\t"
			"valtan.sequence.center-trash-rush-if.step-07\tTIMEOUT\t"
			"valtan.sequence.center-trash-rush-if.step-08";
		std::string duplicateTrashCounterBranchBootstrap = bootstrapText;
		const bool madeDuplicateTrashCounterBranch = replaceBootstrapRow(
			duplicateTrashCounterBranchBootstrap, trashCounterTimeoutRow,
			trashCounterBranchRow);
		const std::string portalVolleyRow =
			"PATTERNSTAGEVOLLEY\tENCOUNTER_VALTAN\t"
			"VALTAN_GHOST_PORTAL_ONCE\tvaltan.ghost.portal-once.active\t"
			"0\tENTER\tcombatobject.valtan.ghost.portal-charge\t"
			"BOSS_RELATIVE\t3\tRADIAL\t9\t30\t120\t0\t3\t1\t0\t"
			"0\t0\t0\t0\tNONE";
		std::string wrappingPortalVolleyBootstrap = bootstrapText;
		const bool madeWrappingPortalVolley = replaceBootstrapRow(
			wrappingPortalVolleyBootstrap, portalVolleyRow,
			"PATTERNSTAGEVOLLEY\tENCOUNTER_VALTAN\t"
			"VALTAN_GHOST_PORTAL_ONCE\tvaltan.ghost.portal-once.active\t"
			"0\tENTER\tcombatobject.valtan.ghost.portal-charge\t"
			"BOSS_RELATIVE\t3\tRADIAL\t9\t30\t121\t0\t3\t1\t0\t"
			"0\t0\t0\t0\tNONE");
		std::string nonEquilateralPortalVolleyBootstrap = bootstrapText;
		const bool madeNonEquilateralPortalVolley = replaceBootstrapRow(
			nonEquilateralPortalVolleyBootstrap, portalVolleyRow,
			"PATTERNSTAGEVOLLEY\tENCOUNTER_VALTAN\t"
			"VALTAN_GHOST_PORTAL_ONCE\tvaltan.ghost.portal-once.active\t"
			"0\tENTER\tcombatobject.valtan.ghost.portal-charge\t"
			"BOSS_RELATIVE\t3\tRADIAL\t9\t30\t119\t0\t3\t1\t0\t"
			"0\t0\t0\t0\tNONE");
		const std::string highJumpVolleyRow =
			"PATTERNSTAGEVOLLEY\tENCOUNTER_VALTAN\tVALTAN_HIGH_JUMP\t"
			"valtan.attack.high-jump.airborne\t0\tENTER\t"
			"combatobject.valtan.high-jump.target-axe\tPER_ALIVE_PLAYER\t"
			"1\tSINGLE\t0\t0\t0\t0\t36\t3\t0\t1333\t4\t14\t1\tBOSS_SPAWN_POSITION";
		std::string excessiveHighJumpScheduleBootstrap = bootstrapText;
		const bool madeExcessiveHighJumpSchedule = replaceBootstrapRow(
			excessiveHighJumpScheduleBootstrap, highJumpVolleyRow,
			"PATTERNSTAGEVOLLEY\tENCOUNTER_VALTAN\tVALTAN_HIGH_JUMP\t"
			"valtan.attack.high-jump.airborne\t0\tENTER\t"
			"combatobject.valtan.high-jump.target-axe\tPER_ALIVE_PLAYER\t"
			"1\tSINGLE\t0\t0\t0\t0\t36\t9\t500\t1333\t4\t14\t1\tBOSS_SPAWN_POSITION");
		const auto rejectsSequenceVariant = [
			&loadBootstrapVariant](const std::wstring_view suffix,
				const std::string& bytes, const std::string_view statusNeedle)
		{
			auto rollbackCatalog = std::make_shared<CGameplayCatalog>();
			if (!rollbackCatalog->Load()) return false;
			const GameplayDataRevision before =
				rollbackCatalog->Get_ActiveRevision();
			const auto beforeSequence =
				rollbackCatalog->Find_BossPatternSequence("ENCOUNTER_VALTAN")->PatternIds;
			return nullptr == loadBootstrapVariant(
					suffix, bytes, rollbackCatalog) &&
				before == rollbackCatalog->Get_ActiveRevision() &&
				beforeSequence == rollbackCatalog->Find_BossPatternSequence("ENCOUNTER_VALTAN")->PatternIds &&
				std::string::npos != rollbackCatalog->Get_Status().find(
					statusNeedle);
		};
		tests.Require(
			madeWrappingPortalVolley &&
			rejectsSequenceVariant(
				L"volley-radial-wrap", wrappingPortalVolleyBootstrap,
				"Boss pattern stage volley layout is invalid"),
			"Reject a radial volley whose count and angle step wrap beyond 360 degrees");
		tests.Require(
			madeNonEquilateralPortalVolley &&
			rejectsSequenceVariant(
				L"volley-ghost-portal-non-equilateral",
				nonEquilateralPortalVolleyBootstrap,
				"exact simultaneous triangle volley"),
			"Reject a ghost portal volley that no longer owns the exact equilateral triangle");
		tests.Require(
			madeExcessiveHighJumpSchedule &&
			rejectsSequenceVariant(
				L"volley-spawn-count", excessiveHighJumpScheduleBootstrap,
				"Boss pattern stage volley row is invalid"),
			"Reject a volley schedule whose spawnCount exceeds eight");
		tests.Require(
			!overflowSequenceBootstrap.empty() &&
			rejectsSequenceVariant(
				L"sequence-overflow", overflowSequenceBootstrap,
				"sequence row is invalid"),
			"Reject a 256-step saved Product sequence and preserve the active gameplay generation");
		tests.Require(
			removedSequence && removedFinalSequenceStep &&
			madeInvalidSequencePursuit &&
			madeDuplicateSequenceStep && madeNonContiguousSequence &&
			madeOutOfBoundsSequence && madeUnknownSequenceStep &&
			rejectsSequenceVariant(
				L"sequence-missing", missingSequenceBootstrap,
				"exclusive pattern sequence") &&
			rejectsSequenceVariant(
				L"sequence-final-step-missing", missingSequenceStepBootstrap,
				"tagged shape is incomplete") &&
			rejectsSequenceVariant(
				L"sequence-invalid-pursuit", invalidSequencePursuitBootstrap,
				"sequence row is invalid") &&
			rejectsSequenceVariant(
				L"sequence-duplicate", duplicateSequenceStepBootstrap,
				"ordered contract") &&
			rejectsSequenceVariant(
				L"sequence-non-contiguous", nonContiguousSequenceBootstrap,
				"ordered contract") &&
			rejectsSequenceVariant(
				L"sequence-out-of-bounds", outOfBoundsSequenceBootstrap,
				"ordered contract") &&
			rejectsSequenceVariant(
				L"sequence-unknown-step", unknownSequenceStepBootstrap,
				"ordered contract"),
			"Reject missing, invalid-pursuit, incomplete, duplicate-ordinal, non-contiguous, out-of-bounds or unknown sequence rows without replacing the active generation");
		tests.Require(
			madeDivergentDashRecovery && rejectsSequenceVariant(
				L"dash-charge-divergent-recovery",
				divergentDashRecoveryBootstrap,
				"branch or action contract"),
			"Reject a Dash outcome whose target action is absent from its pinned pattern graph");
		tests.Require(
			madeInvalidPartDamage && madeInvalidCounterProxy &&
			madeInvalidRearTargetPolicy &&
			rejectsSequenceVariant(
				L"stage-part-damage-invalid", invalidPartDamageBootstrap,
				"part-damage row is invalid") &&
			rejectsSequenceVariant(
				L"stage-counter-proxy-invalid", invalidCounterProxyBootstrap,
				"counter-proxy geometry is invalid") &&
			rejectsSequenceVariant(
				L"rear-target-policy-invalid", invalidRearTargetPolicyBootstrap,
				"pattern policy row is invalid"),
			"Reject invalid Dash part-damage, trash counter-proxy, and rear-target policy refinements transactionally");
		tests.Require(
			madeInvalidForwardArc && madeInvalidMagicResponse &&
			madeZeroMagicResponse && madeZeroMagicVerticalOffset &&
			rejectsSequenceVariant(
				L"stage-forward-arc-invalid", invalidForwardArcBootstrap,
				"counter-proxy geometry is invalid") &&
			rejectsSequenceVariant(
				L"stage-response-unknown", invalidMagicResponseBootstrap,
				"stage response row is invalid") &&
			rejectsSequenceVariant(
				L"stage-response-zero", zeroMagicResponseBootstrap,
				"stage response row is invalid") &&
			rejectsSequenceVariant(
				L"pattern-stage-vertical-offset-zero",
				zeroMagicVerticalOffsetBootstrap,
				"vertical-offset row is invalid"),
			"Reject malformed forward-arc, accumulated-health response and Stage vertical-offset tagged rows transactionally");
		tests.Require(
			madeUnknownMagicFollowup && madeSelfMagicFollowup &&
			madeBothMagicBranchKinds && madeCyclicMagicFollowup &&
			rejectsSequenceVariant(
				L"magic-followup-unknown", unknownMagicFollowupBootstrap,
				"branch or action contract") &&
			rejectsSequenceVariant(
				L"magic-followup-self", selfMagicFollowupBootstrap,
				"branch or action contract") &&
			rejectsSequenceVariant(
				L"magic-followup-both", bothMagicBranchKindsBootstrap,
				"follow-up has no stage owner or is duplicated") &&
			rejectsSequenceVariant(
				L"magic-followup-cycle", cyclicMagicFollowupBootstrap,
				"follow-up graph is cyclic or dangling"),
			"Reject unknown, self, local-plus-cross and cyclic magic-orb follow-up graphs without replacing the active generation");
		tests.Require(
			madeInvalidTrashCounterKind && rejectsSequenceVariant(
				L"trash-counter-stage-kind", invalidTrashCounterKindBootstrap,
				"branch or action contract"),
			"Reject a non-WINDUP managed Counter stage without replacing the active generation");
		tests.Require(
			removedTrashGroggyExit && rejectsSequenceVariant(
				L"trash-counter-groggy-unpaired", unpairedTrashGroggyBootstrap,
				"branch or action contract"),
			"Reject an unpaired managed Groggy flag without replacing the active generation");
		tests.Require(
			madeCrossPatternTrashCounter && rejectsSequenceVariant(
				L"trash-counter-cross-pattern", crossPatternTrashCounterBootstrap,
				""),
			"Reject a cross-pattern managed Counter target without replacing the active generation");
		tests.Require(
			madeDuplicateTrashCounterBranch && rejectsSequenceVariant(
				L"trash-counter-duplicate-branch", duplicateTrashCounterBranchBootstrap,
				"branch"),
			"Reject a duplicate managed Counter branch without replacing the active generation");

		const auto rejectsRuntimeRefinement = [&bootstrapText, &replaceBootstrapRow,
			&rejectsSequenceVariant](const wchar_t* suffix, const std::string& before,
				const std::string& after, const std::string_view reason)
		{
			std::string changed = bootstrapText;
			return replaceBootstrapRow(changed, before, after) &&
				rejectsSequenceVariant(suffix, changed, reason);
		};
		const std::string finalePrefix =
			"PATTERNFINALE\tENCOUNTER_VALTAN\tVALTAN_GHOST_FINALE\tGHOST_PORTAL_LOOP\tBOSS_VALTAN_GHOST\t";
		const std::string finaleChildren =
			"\tVALTAN_WHIRLWIND\tVALTAN_FOUR_SLASH\tVALTAN_SEQUENCE_FOUR"
			"\tVALTAN_CROSS\tVALTAN_CHARGE\tVALTAN_CHARGE_2";
		const std::string finaleRow = finalePrefix + "10\t10\t1" + finaleChildren;
		const std::string dynamicFinaleRow =
			finalePrefix + "10\t10\t2" + finaleChildren;
		std::string dynamicFinaleBootstrap = bootstrapText;
		const bool madeDynamicFinale = replaceBootstrapRow(
			dynamicFinaleBootstrap, finaleRow, dynamicFinaleRow);
		const auto dynamicFinaleCatalog = madeDynamicFinale ?
			loadBootstrapVariant(L"finale-data-driven", dynamicFinaleBootstrap) : nullptr;
		const auto* dynamicFinalePatterns = nullptr == dynamicFinaleCatalog ? nullptr :
			dynamicFinaleCatalog->Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto dynamicFinale = nullptr == dynamicFinalePatterns ?
			std::vector<BOSS_PATTERN_DEFINITION>::const_iterator{} :
			std::find_if(dynamicFinalePatterns->begin(), dynamicFinalePatterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == "VALTAN_GHOST_FINALE"; });
		tests.Require(nullptr != dynamicFinalePatterns &&
			dynamicFinale != dynamicFinalePatterns->end() &&
			2u == dynamicFinale->Finale.iMaximumActiveGhosts &&
			dynamicFinale->Finale.GhostPatternIds == std::vector<std::string>{
				"VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH",
				"VALTAN_SEQUENCE_FOUR", "VALTAN_CROSS",
				"VALTAN_CHARGE", "VALTAN_CHARGE_2" },
			"Load the canonical six-child finale pool and retain its authored capacity");
		tests.Require(
			rejectsRuntimeRefinement(L"finale-half-extent", finaleRow,
				finalePrefix + "0.5\t10\t1" + finaleChildren,
				"finale row is invalid") &&
			rejectsRuntimeRefinement(L"finale-maximum-active-zero", finaleRow,
				finalePrefix + "10\t10\t0" + finaleChildren,
				"finale row is invalid") &&
			rejectsRuntimeRefinement(L"finale-maximum-active-overflow", finaleRow,
				finalePrefix + "10\t10\t65" + finaleChildren,
				"finale row is invalid") &&
			rejectsRuntimeRefinement(L"finale-empty-children", finaleRow,
				finalePrefix + "10\t10\t1", "finale row is invalid") &&
			rejectsRuntimeRefinement(L"finale-duplicate-child", finaleRow,
				finalePrefix + "10\t10\t1\tVALTAN_WHIRLWIND\tVALTAN_FOUR_SLASH"
					"\tVALTAN_SEQUENCE_FOUR\tVALTAN_CROSS\tVALTAN_CHARGE"
					"\tVALTAN_CHARGE",
				"child pattern identity") &&
			rejectsRuntimeRefinement(L"finale-missing-child", finaleRow,
				finalePrefix + "10\t10\t1\tVALTAN_WHIRLWIND\tVALTAN_FOUR_SLASH"
					"\tVALTAN_SEQUENCE_FOUR\tVALTAN_CROSS\tVALTAN_CHARGE"
					"\tVALTAN_UNKNOWN_GHOST_ATTACK",
				"primary-loop order is invalid"),
			"Finale rejects invalid ranges, empty/duplicate children and unresolved references transactionally");
		const std::string targetPortalRow =
			"PATTERNSTAGEMOTION\tENCOUNTER_VALTAN\tVALTAN_WARP\tvaltan.sequence.warp.step-02\tPORTAL_TARGET_RUSH\t300\t12.3076925\t16";
		const std::string cornerPortalPrefix =
			"PATTERNSTAGEMOTION\tENCOUNTER_VALTAN\tVALTAN_GHOST_FINALE\tvaltan.sequence.ghost-finale.step-02\tPORTAL_CROSS_ARENA\t";
		tests.Require(rejectsRuntimeRefinement(L"portal-target-speed", targetPortalRow,
			"PATTERNSTAGEMOTION\tENCOUNTER_VALTAN\tVALTAN_WARP\tvaltan.sequence.warp.step-02\tPORTAL_TARGET_RUSH\t300\t0\t16",
			"stage motion policy is invalid") &&
			rejectsRuntimeRefinement(L"portal-corner", cornerPortalPrefix + "0\t22\t22",
				cornerPortalPrefix + "4\t22\t22", "stage motion policy is invalid") &&
			rejectsRuntimeRefinement(L"portal-extent", cornerPortalPrefix + "0\t22\t22",
				cornerPortalPrefix + "0\t0.5\t22", "stage motion policy is invalid"),
			"Portal target and corner policies reject malformed rows without replacing the active generation");
		const std::string trashMissBranch =
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TRASH\tvaltan.sequence.center-trash-rush-if.rush-miss\tTIMEOUT\t";
		tests.Require(rejectsRuntimeRefinement(L"trash-cycle",
			trashMissBranch + "valtan.sequence.center-trash-rush-if.recharge-wait-02",
			trashMissBranch + "valtan.sequence.center-trash-rush-if.step-07", "cyclic or dangling"),
			"Server rejects reintroduced trash retry cycles while preserving the previous immutable catalog");
		GameplayDataRevision repeatRevision = candidateRevision;
		repeatRevision.Bytes[0] ^= 0x25u;
		GameplayDataRevision reorderedRevision = candidateRevision;
		reorderedRevision.Bytes[0] ^= 0x52u;
		const std::vector<std::string> reorderedOrder{
			"VALTAN_FOUR_SLASH", "VALTAN_WHIRLWIND", "VALTAN_FIST_IN_OUT" };
		const auto repeatGeneration = loadBootstrapVariant(
			L"sequence-repeated-occurrences", sequenceBootstrap, nullptr, repeatRevision);
		const auto reorderedGeneration = loadBootstrapVariant(
			L"sequence-reordered-generation", makeProductSequenceBootstrap(
				"sequence.valtan.contract.reordered", reorderedOrder), nullptr, reorderedRevision);
		const bool repeatedStepsAdmitted = nullptr != repeatGeneration && nullptr != reorderedGeneration &&
			repeatGeneration->Find_BossPatternSequence("ENCOUNTER_VALTAN")->PatternIds == repeatOrder &&
			reorderedGeneration->Find_BossPatternSequence("ENCOUNTER_VALTAN")->PatternIds == reorderedOrder;
		tests.Require(repeatedStepsAdmitted,
			"Admit A/B/A as three distinct ordered Product occurrences without deduplicating pattern IDs");
		if (repeatedStepsAdmitted)
		{
			auto productRoomStorage = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA, repeatGeneration);
			CGameRoom& productRoom = *productRoomStorage;
			const bool activated = productRoom.Is_Ready() && productRoom.Activate_Encounter("boss.valtan.center");
			SERVER_WORLD_ENTITY* productBoss = productRoom.Find_AuditionBoss();
			SERVER_PLAYER productPlayer{};
			productPlayer.iPlayerId = 73601u;
			productPlayer.iNetEntityId = 73602u;
			productPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			productPlayer.iCurrentHp = productPlayer.iMaximumHp = 1000000000u;
			productPlayer.isCombatReady = true;
			if (nullptr != productBoss)
			{
				productPlayer.fPositionX = productBoss->fPositionX + 2.f;
				productPlayer.fPositionY = productBoss->fPositionY;
				productPlayer.fPositionZ = productBoss->fPositionZ;
			}
			productRoom.m_Players.emplace(productPlayer.iPlayerId, productPlayer);
			productRoom.m_PlayerIdByEntityId.emplace(productPlayer.iNetEntityId, productPlayer.iPlayerId);
			productRoom.Tick(1.f / 30.f);
			const bool startedFirst = activated && nullptr != productBoss &&
				productBoss->strPatternId == repeatOrder.front() &&
				productBoss->ProductSequencePinnedDefinitionRevision == repeatRevision;
			std::string sequenceStatus;
			const bool committedDuringFirst = startedFirst &&
				productRoom.Stage_GameplayGeneration(73603u, repeatRevision, reorderedGeneration, sequenceStatus) &&
				productRoom.Commit_GameplayGeneration(73603u);
			std::vector<std::string> observed;
			std::uint32_t lastSequence = 0u;
			bool everyOldOccurrencePinned = committedDuringFirst;
			if (committedDuringFirst)
			{
				for (std::uint32_t tick = 0u; tick < 1500u && productRoom.Is_Ready(); ++tick)
				{
					if (!productBoss->strPatternId.empty() && productBoss->iPatternSequence != lastSequence)
					{
						observed.push_back(productBoss->strPatternId);
						lastSequence = productBoss->iPatternSequence;
					}
					everyOldOccurrencePinned = everyOldOccurrencePinned &&
						productBoss->PinnedDefinitionRevision == repeatRevision &&
						productBoss->ProductSequencePinnedDefinitionRevision == repeatRevision;
					if (productBoss->strPatternId.empty() && productBoss->iRotationStepIndex == repeatOrder.size())
						break;
					productRoom.Tick(1.f / 30.f);
				}
			}
			std::vector<GameplayDataRevision> sequencePins;
			const bool pinsBuilt = productRoom.Build_RequiredPinnedGameplayRevisions(sequencePins);
			productRoom.m_GameplayCatalog.Collect_Garbage(sequencePins);
			const auto terminalSequence = nullptr == productBoss ? 0u : productBoss->iPatternSequence;
			for (std::uint32_t tick = 0u; tick < 90u && productRoom.Is_Ready(); ++tick)
				productRoom.Tick(1.f / 30.f);
			const bool heldOldTerminal = pinsBuilt && nullptr != productBoss &&
				sequencePins.end() != std::find(sequencePins.begin(), sequencePins.end(), repeatRevision) &&
				productRoom.Resolve_ValtanGameplayCatalog(*productBoss) == repeatGeneration.get() &&
				productBoss->strPatternId.empty() && terminalSequence == productBoss->iPatternSequence &&
				productBoss->iRotationStepIndex == repeatOrder.size() &&
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED == productBoss->PatternTerminalReceipt.eResult;
			tests.Require(committedDuringFirst && everyOldOccurrencePinned && observed == repeatOrder && heldOldTerminal,
				"A running Product sequence keeps A/B/A and its catalog through hot reload, pursuit gaps, duplicate occurrences and terminal idle");
			const bool reset = nullptr != productBoss && productRoom.Reset_ValtanAuditionState(
				*productBoss, productRoom.m_iServerTick, sequenceStatus);
			const bool releasedAtReset = reset && !productBoss->ProductSequencePinnedDefinitionRevision.Is_Valid();
			observed.clear();
			lastSequence = nullptr == productBoss ? 0u : productBoss->iPatternSequence;
			bool everyNewOccurrencePinned = releasedAtReset;
			for (std::uint32_t tick = 0u; releasedAtReset && tick < 1500u && productRoom.Is_Ready(); ++tick)
			{
				productRoom.Tick(1.f / 30.f);
				if (!productBoss->strPatternId.empty() && productBoss->iPatternSequence != lastSequence)
				{
					observed.push_back(productBoss->strPatternId);
					lastSequence = productBoss->iPatternSequence;
				}
				everyNewOccurrencePinned = everyNewOccurrencePinned &&
					productBoss->PinnedDefinitionRevision == reorderedRevision &&
					productBoss->ProductSequencePinnedDefinitionRevision == reorderedRevision;
				if (productBoss->strPatternId.empty() && productBoss->iRotationStepIndex == reorderedOrder.size())
					break;
			}
			auto freshRoom = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA, reorderedGeneration);
			const bool freshActivated = freshRoom->Is_Ready() && freshRoom->Activate_Encounter("boss.valtan.center");
			freshRoom->m_Players.emplace(productPlayer.iPlayerId, productPlayer);
			freshRoom->m_PlayerIdByEntityId.emplace(productPlayer.iNetEntityId, productPlayer.iPlayerId);
			freshRoom->Tick(1.f / 30.f);
			const auto* freshBoss = freshRoom->Find_AuditionBoss();
			tests.Require(everyNewOccurrencePinned && observed == reorderedOrder && freshActivated &&
				nullptr != freshBoss && freshBoss->strPatternId == reorderedOrder.front() &&
				freshBoss->ProductSequencePinnedDefinitionRevision == reorderedRevision,
				"Explicit reset releases the old Product sequence and a fresh encounter or reset starts the new C/A/B order at its first occurrence");
		}

		constexpr std::string_view AUDITION_PATTERN_ID =
			"VALTAN_TEST_ANIMATION_AUDITION";
		constexpr std::string_view AUDITION_STAGE_ACTION_ID =
			"valtan.test.animation-audition.step-01";
		const auto appendAuditionPattern = [](
			std::string text,
			const std::string_view patternRow,
			const std::string_view category)
		{
			const std::size_t headerEnd = text.find('\n');
			if (std::string::npos == headerEnd)
				return std::string{};
			const std::size_t rowCountTab = text.rfind('\t', headerEnd);
			std::size_t rowCountEnd = headerEnd;
			if (0u != rowCountEnd && '\r' == text[rowCountEnd - 1u])
				--rowCountEnd;
			if (std::string::npos == rowCountTab ||
				rowCountTab + 1u >= rowCountEnd)
			{
				return std::string{};
			}
			const unsigned long rowCount = std::stoul(text.substr(
				rowCountTab + 1u, rowCountEnd - rowCountTab - 1u));
			text.replace(
				rowCountTab + 1u, rowCountEnd - rowCountTab - 1u,
				std::to_string(rowCount + 5u));
			if (!text.empty() && '\n' != text.back()) text.push_back('\n');
			text.append(patternRow);
			text.append(
				"\nPATTERNPOLICY\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t");
			text.append(category);
			text.append(
				"\t1\t3\tNONE\tNONE"
				"\nPATTERNSOURCE\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t420638\t0\t0\t0\t0\t0\t0"
				"\nPATTERNSTAGE\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t0\tSTEP_01\t"
				"valtan.test.animation-audition.step-01\tACTIVE\t100\tNONE\t"
				"0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0"
				"\nPATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\t"
				"VALTAN_TEST_ANIMATION_AUDITION\t"
				"valtan.test.animation-audition.step-01\tTIMEOUT\t-\n");
			return text;
		};
		const std::string validAuditionRow =
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"0\t0\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0";
		const std::string validAuditionBootstrap = appendAuditionPattern(
			bootstrapText, validAuditionRow, "NORMAL");
		const std::shared_ptr<CGameplayCatalog> auditionCatalog =
			loadBootstrapVariant(L"audition-only", validAuditionBootstrap);
		const std::string invalidAuditionHealthBootstrap = appendAuditionPattern(
			bootstrapText,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"1\t160\t0\t0\t0\t0\t0\t1\t1\tANY\tANY\t0",
			"NORMAL");
		const std::string invalidAuditionTriggerBootstrap = appendAuditionPattern(
			bootstrapText,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"0\t0\t100\t1\t0\t0\t0\t1\t1\tANY\tANY\t0",
			"NORMAL");
		const std::string invalidAuditionWeightBootstrap = appendAuditionPattern(
			bootstrapText,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST_ANIMATION_AUDITION\t"
			"valtan.test.animation-audition\tAUDITION_ONLY\t"
			"0\t0\t0\t0\t1\t1\t0\t1\t1\tANY\tANY\t0",
			"NORMAL");
		const std::string invalidAuditionMechanicBootstrap =
			appendAuditionPattern(
				bootstrapText, validAuditionRow, "MECHANIC");
		std::string invalidAuditionIntroBootstrap = validAuditionBootstrap;
		const bool madeAuditionIntro = replaceBootstrapRow(
			invalidAuditionIntroBootstrap,
			"ENCOUNTERINTRO\tENCOUNTER_VALTAN\tVALTAN_ENTRANCE_WHIRLWIND",
			"ENCOUNTERINTRO\tENCOUNTER_VALTAN\t"
			"VALTAN_TEST_ANIMATION_AUDITION");
		const std::shared_ptr<CGameplayCatalog> invalidAuditionHealthCatalog =
			loadBootstrapVariant(
				L"audition-health-fields", invalidAuditionHealthBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionTriggerCatalog =
			loadBootstrapVariant(
				L"audition-trigger-fields", invalidAuditionTriggerBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionWeightCatalog =
			loadBootstrapVariant(
				L"audition-weight-fields", invalidAuditionWeightBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionMechanicCatalog =
			loadBootstrapVariant(
				L"audition-mechanic", invalidAuditionMechanicBootstrap);
		const std::shared_ptr<CGameplayCatalog> invalidAuditionIntroCatalog =
			madeAuditionIntro ? loadBootstrapVariant(
				L"audition-intro", invalidAuditionIntroBootstrap) : nullptr;
		const std::vector<BOSS_PATTERN_DEFINITION>* auditionPatterns =
			nullptr == auditionCatalog ? nullptr :
				auditionCatalog->Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto auditionPattern = nullptr == auditionPatterns ?
			std::vector<BOSS_PATTERN_DEFINITION>::const_iterator{} :
			std::find_if(
				auditionPatterns->begin(), auditionPatterns->end(),
				[](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return "VALTAN_TEST_ANIMATION_AUDITION" ==
						pattern.strPatternId;
				});
		const bool loadedAuditionOnly = nullptr != auditionPatterns &&
			auditionPatterns->end() != auditionPattern &&
			BOSS_PATTERN_SELECTION::AUDITION_ONLY ==
				auditionPattern->eSelection &&
			0u == auditionPattern->iMinimumHealthBar &&
			0u == auditionPattern->iMaximumHealthBar &&
			0u == auditionPattern->iTriggerHealthBar &&
			0u == auditionPattern->iTriggerOrder &&
			0u == auditionPattern->iSelectionWeight &&
			0u == auditionPattern->iMaximumConsecutiveUses;
		tests.Require(
			loadedAuditionOnly && madeAuditionIntro &&
			nullptr == invalidAuditionHealthCatalog &&
			nullptr == invalidAuditionTriggerCatalog &&
			nullptr == invalidAuditionWeightCatalog &&
			nullptr == invalidAuditionMechanicCatalog &&
			nullptr == invalidAuditionIntroCatalog,
			"Admit zero-gated audition-only patterns and reject health, trigger, weight, mechanic, or intro promotion fields");

		if (loadedAuditionOnly)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> auditionPlayers;
			SERVER_PLAYER auditionTarget{};
			auditionTarget.iPlayerId = 74002u;
			auditionTarget.iNetEntityId = 74002u;
			auditionTarget.iCurrentHp = 10000u;
			auditionTarget.iMaximumHp = 10000u;
			auditionTarget.fPositionX = 151.f;
			auditionTarget.fPositionY = 22.97f;
			auditionTarget.fPositionZ = -128.f;
			auditionTarget.isCombatReady = true;
			auditionPlayers.emplace(
				auditionTarget.iPlayerId, auditionTarget);
			const auto makeAuditionBoss = [&auditionCatalog]()
			{
				SERVER_WORLD_ENTITY boss{};
				boss.iNetEntityId = 74003u;
				boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
				boss.eAction = SERVER_ENTITY_ACTION::IDLE;
				boss.strArchetypeId = "BOSS_VALTAN";
				boss.strEncounterId = "ENCOUNTER_VALTAN";
				boss.iCurrentHp = 60000u;
				boss.iMaximumHp = 60000u;
				boss.iMaximumHealthBars = 160u;
				boss.iLastEvaluatedHealthBar = 160u;
				boss.iLastHealthMechanicGenerationEpoch = 1u;
				boss.iPhase = 1u;
				boss.fPositionX = 151.f;
				boss.fPositionY = 22.97f;
				boss.fPositionZ = -122.f;
				boss.fEngageDistance = 35.f;
				boss.fMoveSpeed = 3.f;
				boss.bIntroPatternConsumed = true;
				boss.bScriptedPatternPlayback = true;
				boss.PinnedDefinitionRevision =
					auditionCatalog->Get_ActiveRevision();
				return boss;
			};
			SERVER_WORLD_ENTITY automaticBoss = makeAuditionBoss();
			CValtanBrain automaticBrain;
			std::vector<DAMAGE_EVENT> auditionDamageEvents;
			automaticBrain.Update(
				automaticBoss, auditionPlayers, *auditionCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 22000u, {},
				auditionDamageEvents);
			const VALTAN_DECISION_TRACE* automaticTrace =
				automaticBrain.Get_LatestDecisionTrace();
			const auto automaticCandidate = nullptr == automaticTrace ?
				std::vector<VALTAN_DECISION_CANDIDATE_TRACE>::const_iterator{} :
				std::find_if(
					automaticTrace->Candidates.begin(),
					automaticTrace->Candidates.end(),
					[](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
					{
						return "VALTAN_TEST_ANIMATION_AUDITION" ==
							candidate.strPatternId;
					});
			const bool excludedFromAutomaticSelection =
				nullptr != automaticTrace &&
				(automaticTrace->Candidates.end() == automaticCandidate ||
				 (0u != (automaticCandidate->iExclusionMask &
					 VALTAN_EXCLUDE_WRONG_SELECTION_KIND) &&
				  0u == automaticCandidate->iEffectiveWeight &&
				  !automaticCandidate->bSelected)) &&
				AUDITION_PATTERN_ID != automaticBoss.strPatternId &&
				AUDITION_PATTERN_ID != automaticTrace->strSelectedPatternId;

			SERVER_WORLD_ENTITY forcedBoss = makeAuditionBoss();
			forcedBoss.iNetEntityId = 74004u;
			forcedBoss.PendingPatternIds.emplace_back(AUDITION_PATTERN_ID);
			CValtanBrain forcedBrain;
			auditionDamageEvents.clear();
			const std::uint32_t targetHpBefore =
				auditionPlayers.begin()->second.iCurrentHp;
			forcedBrain.Update(
				forcedBoss, auditionPlayers, *auditionCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 22001u, {},
				auditionDamageEvents);
			const VALTAN_DECISION_TRACE* forcedTrace =
				forcedBrain.Get_LatestDecisionTrace();
			const bool forcedStableIdExecuted =
				AUDITION_PATTERN_ID == forcedBoss.strPatternId &&
				AUDITION_STAGE_ACTION_ID == forcedBoss.strActionId &&
				SERVER_ENTITY_ACTION::PATTERN_ACTIVE == forcedBoss.eAction &&
				1u == forcedBoss.iPatternSequence &&
				forcedBoss.PendingPatternIds.empty() &&
				forcedBoss.TriggeredPatternIds.empty() &&
				forcedBoss.MechanicOccurrences.empty() &&
				nullptr != forcedTrace &&
				VALTAN_DECISION_SOURCE::FORCED_AUDITION == forcedTrace->eSource &&
				VALTAN_DECISION_RESULT::SELECTED == forcedTrace->eResult &&
				AUDITION_PATTERN_ID == forcedTrace->strPendingPatternId &&
				AUDITION_PATTERN_ID == forcedTrace->strSelectedPatternId &&
				auditionDamageEvents.empty() &&
				targetHpBefore == auditionPlayers.begin()->second.iCurrentHp;
			tests.Require(
				excludedFromAutomaticSelection && forcedStableIdExecuted,
				"Exclude audition-only from every automatic weight roll and execute it only through its pending stable ID");
		}

		std::string mixedTaggedRows = bootstrapText;
		const bool madeMixedTaggedRows = replaceBootstrapRow(
			mixedTaggedRows,
			"PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\trotation.valtan.28.14\t0\tVALTAN_WHIRLWIND",
			"PATTERNROTATIONSTEP\tENCOUNTER_VALTAN\trotation.valtan.160.130\t0\tVALTAN_WHIRLWIND");
		std::string duplicateManagedCandidate = bootstrapText;
		const bool madeDuplicateManagedCandidate = replaceBootstrapRow(
			duplicateManagedCandidate,
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t1\tVALTAN_DASH_CHARGE\t30\t0",
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t1\tVALTAN_WHIRLWIND\t30\t0");
		std::string missingManagedOrdinal = bootstrapText;
		const bool madeMissingManagedOrdinal = replaceBootstrapRow(
			missingManagedOrdinal,
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t4\tVALTAN_HIGH_JUMP\t14\t1",
			"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\trotation.valtan.160.130\t5\tVALTAN_HIGH_JUMP\t14\t1");
		std::string mismatchedManagedWindow = bootstrapText;
		const bool madeMismatchedManagedWindow = replaceBootstrapRow(
			mismatchedManagedWindow,
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\tselectionset.valtan.160.130\t160\t130\t6",
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\tselectionset.valtan.160.130\t159\t130\t6");
		std::string invalidManagedPhase = bootstrapText;
		const bool madeInvalidManagedPhase = replaceBootstrapRow(
			invalidManagedPhase,
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t1\tselectionset.valtan.160.130\t160\t130\t6",
			"PATTERNROTATIONWINDOW\tENCOUNTER_VALTAN\trotation.valtan.160.130\twindow.valtan.phase1.160.130\t4\tselectionset.valtan.160.130\t160\t130\t6");
		std::string collidingHealthMechanic = bootstrapText;
		const bool madeCollidingHealthMechanic = replaceBootstrapRow(
			collidingHealthMechanic,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FOUR_PILLARS_105\tvaltan.mechanic.four-pillars-105\tHEALTH_BAR\t0\t0\t100\t1\t0\t0\t0\t100\t4\tANY\tANY\t0",
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FOUR_PILLARS_105\tvaltan.mechanic.four-pillars-105\tHEALTH_BAR\t0\t0\t84\t1\t0\t0\t0\t100\t4\tANY\tANY\t0");
		std::string divergentPhaseTopology = bootstrapText;
		const bool madeDivergentPhaseTopology = replaceBootstrapRow(
			divergentPhaseTopology,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARENA_BREAK_109\tvaltan.mechanic.arena-break-109\tHEALTH_BAR\t0\t0\t109\t1\t0\t0\t0\t100\t6\tANY\tANY\t0",
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_ARENA_BREAK_109\tvaltan.mechanic.arena-break-109\tHEALTH_BAR\t0\t0\t100\t2\t0\t0\t0\t100\t6\tANY\tANY\t0");
		tests.Require(
			madeMixedTaggedRows && madeDuplicateManagedCandidate &&
			madeMissingManagedOrdinal && madeMismatchedManagedWindow &&
			madeInvalidManagedPhase && madeCollidingHealthMechanic &&
			madeDivergentPhaseTopology &&
			nullptr == loadBootstrapVariant(L"mixed", mixedTaggedRows) &&
			nullptr == loadBootstrapVariant(
				L"duplicate", duplicateManagedCandidate) &&
			nullptr == loadBootstrapVariant(
				L"missing", missingManagedOrdinal) &&
			nullptr == loadBootstrapVariant(
				L"window-mismatch", mismatchedManagedWindow) &&
			nullptr == loadBootstrapVariant(
				L"phase-four", invalidManagedPhase) &&
			nullptr == loadBootstrapVariant(
				L"health-order-collision", collidingHealthMechanic) &&
			nullptr == loadBootstrapVariant(
				L"phase-topology", divergentPhaseTopology),
			"Reject v20 mixed tags, malformed managed windows, duplicate health mechanic order, and divergent phase-transition topology");

		std::string managedWeightVariant = bootstrapText;
		bool managedWeightRowsReady = replaceBootstrapRow(
			managedWeightVariant,
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FIST_IN_OUT\tvaltan.attack.fist-in-out\tNORMAL\t1\t130\t0\t0\t14\t1\t0\t16\t1\tANY\tANY\t0",
			"PATTERN\tENCOUNTER_VALTAN\tVALTAN_FIST_IN_OUT\tvaltan.attack.fist-in-out\tNORMAL\t1\t160\t0\t0\t14\t1\t0\t16\t1\tANY\tANY\t0");
		const auto tuneManagedCandidate = [&managedWeightVariant,
			&replaceBootstrapRow, &managedWeightRowsReady](
			const std::string_view rotationId, const std::uint32_t ordinal,
			const std::string_view patternId, const std::uint32_t oldWeight,
			const std::uint32_t newWeight, const bool enabled)
		{
			const std::string before =
				"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t" +
				std::string(rotationId) + "\t" + std::to_string(ordinal) + "\t" +
				std::string(patternId) + "\t" + std::to_string(oldWeight) +
				("VALTAN_DASH_CHARGE" == patternId ? "\t0" : "\t1");
			const std::string after =
				"PATTERNROTATIONCANDIDATE\tENCOUNTER_VALTAN\t" +
				std::string(rotationId) + "\t" + std::to_string(ordinal) + "\t" +
				std::string(patternId) + "\t" + std::to_string(newWeight) +
				"\t" + (enabled ? "1" : "0");
			managedWeightRowsReady = managedWeightRowsReady &&
				replaceBootstrapRow(managedWeightVariant, before, after);
		};
		tuneManagedCandidate(
			"rotation.valtan.160.130", 0u, "VALTAN_WHIRLWIND", 20u, 100u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 1u, "VALTAN_DASH_CHARGE", 30u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 2u, "VALTAN_FOUR_SLASH", 12u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 3u, "VALTAN_FIST_IN_OUT", 14u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.160.130", 4u, "VALTAN_HIGH_JUMP", 14u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 0u, "VALTAN_WHIRLWIND", 20u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 1u, "VALTAN_DASH_CHARGE", 30u, 100u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 2u, "VALTAN_FOUR_SLASH", 12u, 1u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 3u, "VALTAN_FIST_IN_OUT", 14u, 2u, true);
		tuneManagedCandidate(
			"rotation.valtan.130.109", 4u, "VALTAN_HIGH_JUMP", 14u, 1u, false);
		const std::shared_ptr<CGameplayCatalog> managedWeightCatalog =
			managedWeightRowsReady ?
				loadBootstrapVariant(L"managed-window-weights",
					managedWeightVariant) : nullptr;

		struct MANAGED_WEIGHT_TRACE_FIXTURE final
		{
			VALTAN_DECISION_TRACE TicketZero;
			VALTAN_DECISION_TRACE TicketBoundary;
			VALTAN_DECISION_TRACE OpeningDifference;
			VALTAN_DECISION_TRACE SecondDifference;
			bool hasTicketZero = false;
			bool hasTicketBoundary = false;
			bool hasWindowDifference = false;
		};
		auto managedTraceFixture =
			std::make_unique<MANAGED_WEIGHT_TRACE_FIXTURE>();
		const auto evaluateManagedWindow = [&managedWeightCatalog,
			&valtanRoom, &pinFixture](
			const std::uint32_t healthBar, const std::uint32_t serverTick,
			VALTAN_DECISION_TRACE& outTrace)
		{
			if (nullptr == managedWeightCatalog) return false;
			auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
			boss->iNetEntityId = 74001u;
			boss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
			boss->eAction = SERVER_ENTITY_ACTION::IDLE;
			boss->strArchetypeId = "BOSS_VALTAN";
			boss->strEncounterId = "ENCOUNTER_VALTAN";
			boss->iMaximumHp = 60000u;
			boss->iMaximumHealthBars = 160u;
			boss->iCurrentHp = CValtanBrain::Resolve_HealthBarHp(
				*boss, healthBar);
			boss->iLastEvaluatedHealthBar = healthBar;
			boss->iLastHealthMechanicGenerationEpoch = 1u;
			boss->iPhase = 1u;
			boss->fPositionX = 151.f;
			boss->fPositionY = 22.97f;
			boss->fPositionZ = -122.f;
			boss->fCollisionRadius = 2.6f;
			boss->fEngageDistance = 35.f;
			boss->fMoveSpeed = 3.f;
			boss->bIntroPatternConsumed = true;
			boss->bAutomaticPatternSequenceAuditionOverride = true;
			boss->PinnedDefinitionRevision =
				managedWeightCatalog->Get_ActiveRevision();
			boss->ArmorPlates.push_back({ 0u, 4000u, 50u });
			boss->ArmorPlates.push_back({ 1u, 4000u, 50u });
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			SERVER_PLAYER player = pinFixture->Target;
			player.iCurrentHp = (std::max)(player.iCurrentHp, 1u);
			player.isCombatReady = true;
			player.fPositionX = boss->fPositionX + 6.f;
			player.fPositionY = boss->fPositionY;
			player.fPositionZ = boss->fPositionZ;
			players.emplace(player.iPlayerId, std::move(player));
			auto brain = std::make_unique<CValtanBrain>();
			std::vector<DAMAGE_EVENT> damageEvents;
			brain->Update(
				*boss, players, *managedWeightCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, serverTick, {},
				damageEvents, managedWeightCatalog.get(), 1u);
			const VALTAN_DECISION_TRACE* trace =
				brain->Get_LatestDecisionTrace();
			if (nullptr == trace ||
				VALTAN_DECISION_SOURCE::WEIGHTED != trace->eSource ||
				VALTAN_DECISION_RESULT::SELECTED != trace->eResult)
			{
				return false;
			}
			outTrace = *trace;
			return true;
		};
		if (nullptr != managedWeightCatalog)
		{
			for (std::uint32_t tick = 1u; tick <= 20000u &&
				(!managedTraceFixture->hasTicketZero ||
				 !managedTraceFixture->hasTicketBoundary ||
				 !managedTraceFixture->hasWindowDifference); ++tick)
			{
				VALTAN_DECISION_TRACE opening{};
				if (!evaluateManagedWindow(159u, tick, opening) ||
					104u != opening.iTotalWeight)
				{
					continue;
				}
				if (!managedTraceFixture->hasTicketZero &&
					0u == opening.iRandomTicket)
				{
					managedTraceFixture->TicketZero = opening;
					managedTraceFixture->hasTicketZero = true;
				}
				if (!managedTraceFixture->hasTicketBoundary &&
					100u == opening.iRandomTicket)
				{
					managedTraceFixture->TicketBoundary = opening;
					managedTraceFixture->hasTicketBoundary = true;
				}
				if (!managedTraceFixture->hasWindowDifference &&
					opening.iRandomTicket >= 1u &&
					opening.iRandomTicket < 100u)
				{
					VALTAN_DECISION_TRACE second{};
					if (evaluateManagedWindow(129u, tick, second) &&
						104u == second.iTotalWeight &&
						opening.iRandomTicket == second.iRandomTicket)
					{
						managedTraceFixture->OpeningDifference = opening;
						managedTraceFixture->SecondDifference = second;
						managedTraceFixture->hasWindowDifference = true;
					}
				}
			}
		}
		const auto findTraceCandidate = [](
			const VALTAN_DECISION_TRACE& trace,
			const std::string_view patternId)
			-> const VALTAN_DECISION_CANDIDATE_TRACE*
		{
			const auto found = std::find_if(
				trace.Candidates.begin(), trace.Candidates.end(),
				[patternId](const VALTAN_DECISION_CANDIDATE_TRACE& candidate)
				{ return candidate.strPatternId == patternId; });
			return trace.Candidates.end() == found ? nullptr : &*found;
		};
		const VALTAN_DECISION_CANDIDATE_TRACE* zeroWhirlwind =
			findTraceCandidate(
				managedTraceFixture->TicketZero, "VALTAN_WHIRLWIND");
		const VALTAN_DECISION_CANDIDATE_TRACE* boundaryDash =
			findTraceCandidate(
				managedTraceFixture->TicketBoundary, "VALTAN_DASH_CHARGE");
		const VALTAN_DECISION_CANDIDATE_TRACE* openingWhirlwind =
			findTraceCandidate(
				managedTraceFixture->OpeningDifference, "VALTAN_WHIRLWIND");
		const VALTAN_DECISION_CANDIDATE_TRACE* openingDash =
			findTraceCandidate(
				managedTraceFixture->OpeningDifference, "VALTAN_DASH_CHARGE");
		const VALTAN_DECISION_CANDIDATE_TRACE* secondWhirlwind =
			findTraceCandidate(
				managedTraceFixture->SecondDifference, "VALTAN_WHIRLWIND");
		const VALTAN_DECISION_CANDIDATE_TRACE* secondDash =
			findTraceCandidate(
				managedTraceFixture->SecondDifference, "VALTAN_DASH_CHARGE");
		const VALTAN_DECISION_CANDIDATE_TRACE* secondHighJump =
			findTraceCandidate(
				managedTraceFixture->SecondDifference, "VALTAN_HIGH_JUMP");
		const auto* managedPatterns = nullptr == managedWeightCatalog ? nullptr :
			managedWeightCatalog->Find_BossPatterns("ENCOUNTER_VALTAN");
		const auto findCompatibilityWeight = [managedPatterns](
			const std::string_view patternId)
		{
			if (nullptr == managedPatterns) return std::uint32_t{ 0u };
			const auto found = std::find_if(
				managedPatterns->begin(), managedPatterns->end(),
				[patternId](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == patternId; });
			return managedPatterns->end() == found ?
				std::uint32_t{ 0u } : found->iSelectionWeight;
		};
		const std::array<std::string_view, 5u> expectedCandidateOrder{
			"VALTAN_WHIRLWIND", "VALTAN_DASH_CHARGE", "VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT", "VALTAN_HIGH_JUMP" };
		bool exactManagedCandidateOrder =
			managedTraceFixture->OpeningDifference.Candidates.size() >=
				expectedCandidateOrder.size();
		for (std::size_t index = 0u;
			exactManagedCandidateOrder && index < expectedCandidateOrder.size();
			++index)
		{
			exactManagedCandidateOrder = expectedCandidateOrder[index] ==
				managedTraceFixture->OpeningDifference.Candidates[index].strPatternId;
		}
		tests.Require(
			managedWeightRowsReady && nullptr != managedWeightCatalog &&
			managedTraceFixture->hasTicketZero &&
			managedTraceFixture->hasTicketBoundary &&
			managedTraceFixture->hasWindowDifference &&
			exactManagedCandidateOrder && nullptr != zeroWhirlwind &&
			nullptr != boundaryDash && nullptr != openingWhirlwind &&
			nullptr != openingDash && nullptr != secondWhirlwind &&
			nullptr != secondDash && nullptr != secondHighJump &&
			"VALTAN_WHIRLWIND" ==
				managedTraceFixture->TicketZero.strSelectedPatternId &&
			zeroWhirlwind->bSelected && 100u == zeroWhirlwind->iAuthoredWeight &&
			0u == zeroWhirlwind->iWeightBeginInclusive &&
			100u == zeroWhirlwind->iWeightEndExclusive &&
			"VALTAN_DASH_CHARGE" ==
				managedTraceFixture->TicketBoundary.strSelectedPatternId &&
			boundaryDash->bSelected &&
			100u == boundaryDash->iWeightBeginInclusive &&
			101u == boundaryDash->iWeightEndExclusive &&
			"VALTAN_WHIRLWIND" ==
				managedTraceFixture->OpeningDifference.strSelectedPatternId &&
			"VALTAN_DASH_CHARGE" ==
				managedTraceFixture->SecondDifference.strSelectedPatternId &&
			100u == openingWhirlwind->iAuthoredWeight &&
			100u == openingWhirlwind->iEffectiveWeight &&
			1u == openingDash->iAuthoredWeight &&
			1u == secondWhirlwind->iAuthoredWeight &&
			100u == secondDash->iAuthoredWeight &&
			0u == secondWhirlwind->iWeightBeginInclusive &&
			1u == secondWhirlwind->iWeightEndExclusive &&
			1u == secondDash->iWeightBeginInclusive &&
			101u == secondDash->iWeightEndExclusive &&
			0u != (secondHighJump->iExclusionMask &
				VALTAN_EXCLUDE_DISABLED) &&
			1u == secondHighJump->iAuthoredWeight &&
			0u == secondHighJump->iEffectiveWeight &&
			20u == findCompatibilityWeight("VALTAN_WHIRLWIND") &&
			30u == findCompatibilityWeight("VALTAN_DASH_CHARGE"),
			"Consume strict v22 managed candidate ordinal, per-window weight/enabled overrides, exact ticket boundaries, and truthful trace intervals without mutating compatibility weights");

		SERVER_WORLD_ENTITY& forcedOccurrenceBoss =
			valtanRoom->m_WorldEntities.back();
		const bool runningOldPatternStayedPinned =
			activeRevision == forcedOccurrenceBoss.PinnedDefinitionRevision;
		/* Model the ordinary end-of-tick retirement of the pattern that crossed
		   130 bars. The room may publish B on the idle entity, but the queued
		   occurrence itself must keep A live and select from A next tick. */
		forcedOccurrenceBoss.strPatternId.clear();
		forcedOccurrenceBoss.strPatternStageId.clear();
		forcedOccurrenceBoss.strActionId.clear();
		forcedOccurrenceBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		forcedOccurrenceBoss.PinnedDefinitionRevision = candidateRevision;
		const CGameplayCatalog* forcedOccurrenceCatalog =
			valtanRoom->Resolve_ValtanGameplayCatalog(forcedOccurrenceBoss);
		const bool queuedOldGenerationStayedLive =
			valtanRoom->Build_RequiredPinnedGameplayRevisions(
				pinFixture->RequiredPins) &&
			pinFixture->RequiredPins.end() != std::find(
				pinFixture->RequiredPins.begin(),
				pinFixture->RequiredPins.end(), activeRevision) &&
			nullptr != forcedOccurrenceCatalog &&
			activeRevision == forcedOccurrenceCatalog->Get_ActiveRevision();
		if (nullptr != forcedOccurrenceCatalog)
		{
			pinFixture->Brain.Update(
				forcedOccurrenceBoss, pinFixture->Players,
				*forcedOccurrenceCatalog,
				valtanRoom->m_ServerNavigation, 1.f / 30.f, 12u, {},
				pinFixture->DamageEvents);
		}
		const auto forcedOccurrence = std::find_if(
			forcedOccurrenceBoss.MechanicOccurrences.begin(),
			forcedOccurrenceBoss.MechanicOccurrences.end(),
			[](const SERVER_BOSS_MECHANIC_OCCURRENCE& occurrence)
			{
				return "VALTAN_FLOOR_WIPE_130" == occurrence.strPatternId;
			});
		const bool forcedMechanicStartedFromOldGeneration =
			forcedOccurrenceBoss.MechanicOccurrences.end() != forcedOccurrence &&
			SERVER_BOSS_MECHANIC_STATE::ACTIVE == forcedOccurrence->eState &&
			"VALTAN_FLOOR_WIPE_130" == forcedOccurrenceBoss.strPatternId &&
			activeRevision == forcedOccurrenceBoss.PinnedDefinitionRevision;
		if (forcedOccurrenceBoss.MechanicOccurrences.end() != forcedOccurrence)
			forcedOccurrence->eState = SERVER_BOSS_MECHANIC_STATE::COMPLETED;
		forcedOccurrenceBoss.strPatternId.clear();
		forcedOccurrenceBoss.strPatternStageId.clear();
		forcedOccurrenceBoss.strActionId.clear();
		forcedOccurrenceBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		forcedOccurrenceBoss.PinnedDefinitionRevision = candidateRevision;
		const CGameplayCatalog* nextNormalCatalog =
			valtanRoom->Resolve_ValtanGameplayCatalog(forcedOccurrenceBoss);
		const bool laterNormalUsesNewGeneration =
			nullptr != nextNormalCatalog && candidateRevision ==
				nextNormalCatalog->Get_ActiveRevision();
		tests.Require(
			runningOldPatternStayedPinned && queuedOldGenerationStayedLive &&
			forcedMechanicStartedFromOldGeneration &&
			laterNormalUsesNewGeneration,
			"Run a threshold mechanic from its queued old generation after commit, then return normal selection to the new generation");

		GameplayDataRevision secondCandidateRevision = candidateRevision;
		secondCandidateRevision.Bytes[2] ^= 0x20u;
		if (!secondCandidateRevision.Is_Valid() ||
			secondCandidateRevision == candidateRevision ||
			secondCandidateRevision == activeRevision)
			secondCandidateRevision.Bytes[3] ^= 1u;
		auto secondCandidate = std::make_shared<CGameplayCatalog>();
		const bool secondCandidateLoaded = secondCandidate->Load_FromBootstrap(
			bootstrapPath, activeRevision, secondCandidateRevision);
		C2S_DATA_REVISION_PREPARE_REQUEST abortRequest{};
		abortRequest.iTransactionSequence = 72u;
		abortRequest.BaseRevision = candidateRevision;
		abortRequest.CandidateRevision = secondCandidateRevision;
		abortRequest.iRequiredPresentationLaneMask =
			GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
		CServerApp::SERVER_CONTROL_EVENT abortBegin{};
		abortBegin.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		abortBegin.iSessionId = REQUESTER_SESSION;
		abortBegin.RevisionRequest = abortRequest;
		abortBegin.pCandidateGeneration = secondCandidate;
		abortBegin.BaseBootstrapContentRevision = activeRevision;
		abortBegin.CandidateBootstrapContentRevision = activeRevision;
		abortBegin.BaseNonValtanGameplayRevision = activeRevision;
		abortBegin.CandidateNonValtanGameplayRevision = activeRevision;
		(void)app.Queue_ServerControlEvent(std::move(abortBegin));
		app.Advance_ServerControlTransactions();
		CServerApp::SERVER_CONTROL_EVENT nack{};
		nack.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
		nack.iSessionId = REQUESTER_SESSION;
		nack.RevisionResponse.iTransactionSequence =
			abortRequest.iTransactionSequence;
		nack.RevisionResponse.CandidateRevision = secondCandidateRevision;
		nack.RevisionResponse.eStatus = DATA_REVISION_PREPARE_STATUS::NACK;
		nack.RevisionResponse.iRequiredPresentationLaneMask =
			abortRequest.iRequiredPresentationLaneMask;
		nack.RevisionResponse.strReason = "contract NACK";
		CServerApp::SERVER_CONTROL_EVENT innocentReady{};
		innocentReady.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
		innocentReady.iSessionId = PARTICIPANT_SESSION;
		innocentReady.RevisionResponse.iTransactionSequence =
			abortRequest.iTransactionSequence;
		innocentReady.RevisionResponse.CandidateRevision =
			secondCandidateRevision;
		innocentReady.RevisionResponse.eStatus =
			DATA_REVISION_PREPARE_STATUS::READY;
		innocentReady.RevisionResponse.iRequiredPresentationLaneMask =
			abortRequest.iRequiredPresentationLaneMask;
		innocentReady.RevisionResponse.iPreparedPresentationLaneMask =
			abortRequest.iRequiredPresentationLaneMask;
		(void)app.Queue_ServerControlEvent(std::move(nack));
		app.Advance_ServerControlTransactions();
		const bool abortedBeforeLateReady =
			!app.m_DataRevisionTransaction.Is_Active();
		(void)app.Queue_ServerControlEvent(std::move(innocentReady));
		app.Advance_ServerControlTransactions();
		const bool rolledBackWithoutClosingInnocent = secondCandidateLoaded &&
			abortedBeforeLateReady &&
			!app.m_DataRevisionTransaction.Is_Active() &&
			candidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			candidateRevision == valtanRoom->Get_ActiveGameplayGeneration()->
				Get_ActiveRevision() && participant->m_isSendRunning.load();
		tests.Require(
			rolledBackWithoutClosingInnocent,
			"Abort every room on NACK and ignore an innocent next-drain READY for the exact aborted transaction");

		C2S_DATA_REVISION_PREPARE_REQUEST staleBootstrapRequest =
			abortRequest;
		staleBootstrapRequest.iTransactionSequence = 73u;
		GameplayDataRevision staleBootstrapBaseline = activeRevision;
		staleBootstrapBaseline.Bytes[6] ^= 0x40u;
		if (!staleBootstrapBaseline.Is_Valid() ||
			staleBootstrapBaseline == activeRevision)
		{
			staleBootstrapBaseline.Bytes[7] ^= 1u;
		}
		CServerApp::SERVER_CONTROL_EVENT staleBootstrap{};
		staleBootstrap.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		staleBootstrap.iSessionId = REQUESTER_SESSION;
		staleBootstrap.RevisionRequest = staleBootstrapRequest;
		staleBootstrap.pCandidateGeneration = secondCandidate;
		staleBootstrap.BaseBootstrapContentRevision = activeRevision;
		staleBootstrap.CandidateBootstrapContentRevision = activeRevision;
		staleBootstrap.BaseNonValtanGameplayRevision = activeRevision;
		staleBootstrap.CandidateNonValtanGameplayRevision =
			staleBootstrapBaseline;
		const std::size_t beforeStaleBootstrapResult =
			requester->m_OutboundFrames.size();
		(void)app.Queue_ServerControlEvent(std::move(staleBootstrap));
		app.Advance_ServerControlTransactions();
		tests.Require(
			!app.m_DataRevisionTransaction.Is_Active() &&
			requester->m_isSendRunning.load() &&
			requester->m_OutboundFrames.size() ==
				beforeStaleBootstrapResult + 1u &&
			candidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			activeRevision ==
				app.m_ActiveGameplayBootstrapContentRevision &&
			activeRevision == app.m_ActiveNonValtanGameplayRevision,
			"Reject a stale full-bootstrap candidate baseline before PREPARE can roll back non-Valtan rows");

		const auto makeFrame = [](const C2S_DATA_REVISION_PREPARE_REQUEST& value)
		{
			CPacketWriter writer;
			PACKET_FRAME frame{};
			frame.ePacketType =
				PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST;
			if (Write_Message(writer, value))
			{
				const auto payload = writer.Get_Buffer();
				frame.Payload.assign(payload.begin(), payload.end());
			}
			return frame;
		};
		const std::size_t beforeIdempotent =
			requester->m_OutboundFrames.size();
		app.On_SessionFrame(REQUESTER_SESSION, makeFrame(request));
		S2C_DATA_REVISION_RESULT idempotentResult{};
		bool decodedIdempotentResult = false;
		if (requester->m_OutboundFrames.size() == beforeIdempotent + 1u)
		{
			const auto& bytes = requester->m_OutboundFrames.back().Bytes;
			PACKET_HEADER header{};
			if (Read_Packet_Header(bytes, header) &&
				PACKET_TYPE::S2C_DATA_REVISION_RESULT == header.ePacketType)
			{
				CPacketReader resultReader{ std::span<const std::uint8_t>(
					bytes.data() + PACKET_HEADER_BYTES,
					bytes.size() - PACKET_HEADER_BYTES) };
				decodedIdempotentResult =
					Read_Message(resultReader, idempotentResult) &&
					0u == resultReader.Get_RemainingSize();
			}
		}
		const bool idempotentVerdict = decodedIdempotentResult &&
			DATA_REVISION_RESULT::COMMITTED == idempotentResult.eResult &&
			candidateRevision == idempotentResult.ActiveRevision;
		const char* idempotentContract =
			"Answer an already-active retry with the only truthful typed COMMITTED terminal state";
		tests.Require(
			requester->m_isSendRunning.load() && idempotentVerdict,
			idempotentContract);

		C2S_DATA_REVISION_PREPARE_REQUEST repeatedRequest = abortRequest;
		repeatedRequest.iTransactionSequence = 74u;
		CServerApp::SERVER_CONTROL_EVENT repeatedBegin{};
		repeatedBegin.eKind =
			CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		repeatedBegin.iSessionId = REQUESTER_SESSION;
		repeatedBegin.RevisionRequest = repeatedRequest;
		repeatedBegin.pCandidateGeneration = secondCandidate;
		repeatedBegin.BaseBootstrapContentRevision = activeRevision;
		repeatedBegin.CandidateBootstrapContentRevision = activeRevision;
		repeatedBegin.BaseNonValtanGameplayRevision = activeRevision;
		repeatedBegin.CandidateNonValtanGameplayRevision = activeRevision;
		(void)app.Queue_ServerControlEvent(std::move(repeatedBegin));
		app.Advance_ServerControlTransactions();
		const bool repeatedPrepared =
			app.m_DataRevisionTransaction.Is_Active();
		const auto queueRepeatedReady =
			[&app, &repeatedRequest](const SESSION_ID sessionId)
		{
			CServerApp::SERVER_CONTROL_EVENT event{};
			event.eKind =
				CServerApp::SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
			event.iSessionId = sessionId;
			event.RevisionResponse.iTransactionSequence =
				repeatedRequest.iTransactionSequence;
			event.RevisionResponse.CandidateRevision =
				repeatedRequest.CandidateRevision;
			event.RevisionResponse.eStatus =
				DATA_REVISION_PREPARE_STATUS::READY;
			event.RevisionResponse.iRequiredPresentationLaneMask =
				repeatedRequest.iRequiredPresentationLaneMask;
			event.RevisionResponse.iPreparedPresentationLaneMask =
				repeatedRequest.iRequiredPresentationLaneMask;
			return app.Queue_ServerControlEvent(std::move(event));
		};
		const bool repeatedReady =
			queueRepeatedReady(REQUESTER_SESSION) &&
			queueRepeatedReady(PARTICIPANT_SESSION);
		app.Advance_ServerControlTransactions();
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredSecondRuntime{};
		bool recoveredSecondPointer = false;
		runtimeRecoveryStatus.clear();
		const bool recoveredSecondCommit =
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				runtimePersistenceRoot, packagedRuntime,
				recoveredSecondRuntime, recoveredSecondPointer,
				runtimeRecoveryStatus) && recoveredSecondPointer &&
			secondCandidateRevision == recoveredSecondRuntime.Revision;
		tests.Require(
			repeatedPrepared && repeatedReady &&
			!app.m_DataRevisionTransaction.Is_Active() &&
			secondCandidateRevision ==
				app.m_pActiveGameplayGeneration->Get_ActiveRevision() &&
			secondCandidateRevision ==
				valtanRoom->Get_ActiveGameplayGeneration()->
					Get_ActiveRevision() &&
			secondCandidateRevision ==
				privateRoom->Get_ActiveGameplayGeneration()->
					Get_ActiveRevision() &&
			activeRevision == app.m_ActiveNonValtanGameplayRevision &&
			recoveredSecondCommit,
			"Commit a second Valtan-only candidate in the same Server process without requiring the packaged bootstrap baseline");

		std::string resetStatus;
		const bool resetEscapesMissingOrRetiredDurableCandidate =
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				runtimePersistenceRoot, packagedRuntime, resetStatus) &&
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				runtimePersistenceRoot, packagedRuntime, resetStatus);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION packagedAfterCandidateReset{};
		bool packagedPointerAfterCandidateReset = false;
		const bool candidateResetSelectedPackaged =
			resetEscapesMissingOrRetiredDurableCandidate &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				runtimePersistenceRoot, packagedRuntime,
				packagedAfterCandidateReset,
				packagedPointerAfterCandidateReset, resetStatus) &&
			packagedPointerAfterCandidateReset &&
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
				packagedAfterCandidateReset.eSource &&
			packagedRuntime.Revision == packagedAfterCandidateReset.Revision &&
			packagedRuntime.BootstrapContentRevision ==
				packagedAfterCandidateReset.BootstrapContentRevision &&
			packagedRuntime.NonValtanGameplayRevision ==
				packagedAfterCandidateReset.NonValtanGameplayRevision;

		const fs::path packagedResetRoot = fs::temp_directory_path() /
			(L"lostark-server-runtime-packaged-reset-contract-" +
			 std::to_wstring(::GetCurrentProcessId()));
		std::error_code packagedResetError;
		fs::remove_all(packagedResetRoot, packagedResetError);
		fs::create_directories(packagedResetRoot, packagedResetError);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION oldPackaged = packagedRuntime;
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION newPackaged = packagedRuntime;
		newPackaged.Revision.Bytes[16] ^= 0x20u;
		newPackaged.BootstrapContentRevision.Bytes[17] ^= 0x40u;
		newPackaged.NonValtanGameplayRevision.Bytes[18] ^= 0x80u;
		const bool wroteOldPackagedPointer = !packagedResetError &&
			CServerApp::Rollback_RuntimeGameplayActivation(
				packagedResetRoot, oldPackaged, resetStatus);
		const bool resetAcrossPackagedDrift = wroteOldPackagedPointer &&
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus) &&
			CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION recoveredNewPackaged{};
		bool recoveredNewPackagedPointer = false;
		const bool recoveredResetPackage = resetAcrossPackagedDrift &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				packagedResetRoot, newPackaged, recoveredNewPackaged,
				recoveredNewPackagedPointer, resetStatus) &&
			recoveredNewPackagedPointer &&
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
				recoveredNewPackaged.eSource &&
			newPackaged.Revision == recoveredNewPackaged.Revision &&
			newPackaged.BootstrapContentRevision ==
				recoveredNewPackaged.BootstrapContentRevision &&
			newPackaged.NonValtanGameplayRevision ==
				recoveredNewPackaged.NonValtanGameplayRevision;

		const bool restoredOldPointerForCorruptJournal =
			CServerApp::Rollback_RuntimeGameplayActivation(
				packagedResetRoot, oldPackaged, resetStatus);
		bool wroteCorruptJournal = false;
		if (restoredOldPointerForCorruptJournal)
		{
			std::ofstream journal(
				packagedResetRoot / L"active-generation.journal.json",
				std::ios::binary | std::ios::trunc);
			constexpr std::string_view corruptJournal =
				"{\"schema\":\"corrupt-runtime-journal\"}";
			journal.write(corruptJournal.data(),
				static_cast<std::streamsize>(corruptJournal.size()));
			wroteCorruptJournal = journal.good();
		}
		const bool corruptJournalResetRejected = wroteCorruptJournal &&
			!CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus);
		std::error_code corruptJournalRemoveError;
		fs::remove(packagedResetRoot / L"active-generation.journal.json",
			corruptJournalRemoveError);
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION pointerAfterCorruptJournal{};
		bool pointerAfterCorruptJournalPresent = false;
		const bool corruptJournalPreservedOldPointer =
			corruptJournalResetRejected && !corruptJournalRemoveError &&
			CServerApp::Recover_RuntimeActiveGameplayPointer(
				packagedResetRoot, oldPackaged, pointerAfterCorruptJournal,
				pointerAfterCorruptJournalPresent, resetStatus) &&
			pointerAfterCorruptJournalPresent &&
			oldPackaged.Revision == pointerAfterCorruptJournal.Revision;

		bool wroteCorruptPointer = false;
		{
			std::ofstream pointer(
				packagedResetRoot / L"active-generation.json",
				std::ios::binary | std::ios::trunc);
			constexpr std::string_view corruptPointer =
				"{\"schema\":\"corrupt-runtime-pointer\"}";
			pointer.write(corruptPointer.data(),
				static_cast<std::streamsize>(corruptPointer.size()));
			wroteCorruptPointer = pointer.good();
		}
		const bool corruptPointerResetRejected = wroteCorruptPointer &&
			!CServerApp::Reset_RuntimeGameplayActivationToPackaged(
				packagedResetRoot, newPackaged, resetStatus);
		fs::remove_all(packagedResetRoot, packagedResetError);
		tests.Require(
			candidateResetSelectedPackaged && recoveredResetPackage &&
			corruptJournalPreservedOldPointer && corruptPointerResetRejected,
			"Reset a missing or retired candidate to packaged idempotently and reject corrupt durable state without mutation");

		// Never contend with a user's running Server. Only the name is test-
		// owned; creation, refusal and release use the production Win32 path.
		const std::wstring mutexName =
			L"Local\\LostArk.Server.ValtanRuntimeActivation.Contract." +
			std::to_wstring(GetCurrentProcessId());
		void* ownerMutex = nullptr;
		std::string mutexStatus;
		const bool acquiredOwnerMutex =
			CServerApp::Acquire_NamedRuntimeGameplayProcessMutex(
				mutexName.c_str(), ownerMutex, mutexStatus) &&
			nullptr != ownerMutex && mutexStatus.empty();
		std::atomic_bool secondThreadWasRefused{ false };
		std::thread competingResetThread([&mutexName, &secondThreadWasRefused]()
		{
			void* competingMutex = nullptr;
			std::string competingStatus;
			secondThreadWasRefused.store(
				!CServerApp::Acquire_NamedRuntimeGameplayProcessMutex(
					mutexName.c_str(), competingMutex, competingStatus) &&
				nullptr == competingMutex &&
				"Another Server process owns the runtime gameplay activation lock" == competingStatus);
			CServerApp::Release_RuntimeGameplayProcessMutex(competingMutex);
		});
		competingResetThread.join();
		CServerApp::Release_RuntimeGameplayProcessMutex(ownerMutex);
		const bool ownerReleased = nullptr == ownerMutex;
		const bool acquiredAfterRelease =
			CServerApp::Acquire_NamedRuntimeGameplayProcessMutex(
				mutexName.c_str(), ownerMutex, mutexStatus) &&
			nullptr != ownerMutex && mutexStatus.empty();
		CServerApp::Release_RuntimeGameplayProcessMutex(ownerMutex);
		tests.Require(
			acquiredOwnerMutex && secondThreadWasRefused.load() &&
			ownerReleased && acquiredAfterRelease && nullptr == ownerMutex,
			"Acquire, refuse a concurrent owner, release and reacquire through the production mutex path in an isolated namespace");
		const bool rejectedNullMutexName =
			!CServerApp::Acquire_NamedRuntimeGameplayProcessMutex(nullptr, ownerMutex, mutexStatus) &&
			nullptr == ownerMutex && !mutexStatus.empty();
		const bool rejectedEmptyMutexName =
			!CServerApp::Acquire_NamedRuntimeGameplayProcessMutex(L"", ownerMutex, mutexStatus) &&
			nullptr == ownerMutex && !mutexStatus.empty();
		tests.Require(rejectedNullMutexName && rejectedEmptyMutexName,
			"Reject a missing mutex namespace instead of creating an unprotected unnamed lock");
		fs::remove_all(runtimePersistenceRoot, runtimePersistenceError);

		valtanRoom->m_WorldEntities.pop_back();
		valtanRoom->m_GameplayCatalog.Collect_Garbage({ candidateRevision });
		tests.Require(
			nullptr == valtanRoom->Resolve_GameplayGeneration(activeRevision),
			"Collect an old immutable gameplay generation after its final occurrence pin is released");
		auto capacityRoomStorage = std::make_unique<CGameRoom>(
			WORLD_ID::TRAINING_GROUND, baseGeneration);
		CGameRoom& capacityRoom = *capacityRoomStorage;
		SERVER_WORLD_ENTITY capacityBasePin{};
		capacityBasePin.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		capacityBasePin.strPatternId = "VALTAN_GENERATION_CAPACITY_PIN";
		capacityBasePin.PinnedDefinitionRevision = activeRevision;
		capacityRoom.m_WorldEntities.push_back(capacityBasePin);
		bool filledGenerationCapacity = capacityRoom.Is_Ready();
		GameplayDataRevision capacityActive = activeRevision;
		for (std::uint32_t ordinal = 1u;
			filledGenerationCapacity && ordinal <
				CGameplayCatalogGenerations::MAX_GENERATION_COUNT;
			++ordinal)
		{
			GameplayDataRevision generationRevision = activeRevision;
			generationRevision.Bytes[28] ^= 0x5au;
			generationRevision.Bytes[29] =
				static_cast<std::uint8_t>(ordinal);
			auto generation = std::make_shared<CGameplayCatalog>();
			std::string capacityStatus;
			filledGenerationCapacity = generation->Load_FromBootstrap(
				bootstrapPath, activeRevision, generationRevision) &&
				capacityRoom.Stage_GameplayGeneration(
					100u + ordinal, capacityActive, generation, capacityStatus) &&
				capacityRoom.Commit_GameplayGeneration(100u + ordinal);
			if (!filledGenerationCapacity) break;
			SERVER_WORLD_ENTITY pin = capacityBasePin;
			pin.PinnedDefinitionRevision = generationRevision;
			capacityRoom.m_WorldEntities.push_back(std::move(pin));
			capacityActive = generationRevision;
		}
		GameplayDataRevision overflowRevision = activeRevision;
		overflowRevision.Bytes[28] ^= 0xa5u;
		overflowRevision.Bytes[29] ^= 0xffu;
		auto overflowGeneration = std::make_shared<CGameplayCatalog>();
		std::string overflowStatus;
		const bool rejectedGenerationOverflow = filledGenerationCapacity &&
			CGameplayCatalogGenerations::MAX_GENERATION_COUNT ==
				capacityRoom.m_GameplayCatalog.Get_GenerationCount() &&
			overflowGeneration->Load_FromBootstrap(
				bootstrapPath, activeRevision, overflowRevision) &&
			!capacityRoom.Stage_GameplayGeneration(
				999u, capacityActive, overflowGeneration, overflowStatus) &&
			overflowStatus.find("capacity") != std::string::npos;
		if (!rejectedGenerationOverflow)
		{
			std::cout << "[STATUS] generation capacity count=" <<
				capacityRoom.m_GameplayCatalog.Get_GenerationCount() <<
				", filled=" << filledGenerationCapacity <<
				", status=" << overflowStatus << '\n';
		}
		tests.Require(rejectedGenerationOverflow,
			"Fail-close the seventeenth immutable gameplay generation while sixteen occurrence revisions remain pinned");

		const fs::path pathContractRoot = fs::temp_directory_path() /
			(L"LostArkCandidatePathContract-" +
				std::to_wstring(GetCurrentProcessId()));
		const fs::path siblingRoot = fs::path(
			pathContractRoot.native() + L"Evil");
		std::error_code pathContractError;
		fs::remove_all(pathContractRoot, pathContractError);
		pathContractError.clear();
		fs::remove_all(siblingRoot, pathContractError);
		fs::create_directories(pathContractRoot / L"nested", pathContractError);
		fs::create_directories(siblingRoot, pathContractError);
		{
			std::ofstream validFile(pathContractRoot / L"nested" / L"payload.bin",
				std::ios::binary | std::ios::trunc);
			validFile << "candidate-path-contract";
			std::ofstream siblingFile(siblingRoot / L"payload.bin",
				std::ios::binary | std::ios::trunc);
			siblingFile << "sibling-prefix-contract";
		}
		const fs::path canonicalContractRoot =
			fs::canonical(pathContractRoot, pathContractError);
		fs::path resolvedArtifact;
		std::string pathContractStatus;
		const bool admittedNested = !pathContractError &&
			CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, "nested/payload.bin",
				resolvedArtifact, pathContractStatus);
		const bool rejectedDriveQualified =
			!CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, "C:/Windows/System32/kernel32.dll",
				resolvedArtifact, pathContractStatus);
		const bool rejectedParent =
			!CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, "../LostArkCandidatePathContract-" +
					std::to_string(GetCurrentProcessId()) + "Evil/payload.bin",
				resolvedArtifact, pathContractStatus);
		const bool rejectedSiblingAbsolute =
			!CServerApp::Resolve_CandidateArtifactForAdmission(
				canonicalContractRoot, siblingRoot.string() + "/payload.bin",
				resolvedArtifact, pathContractStatus);
		bool rejectedParentLink = true;
		pathContractError.clear();
		fs::create_directory_symlink(
			siblingRoot, pathContractRoot / L"parent-link", pathContractError);
		if (!pathContractError)
		{
			rejectedParentLink =
				!CServerApp::Resolve_CandidateArtifactForAdmission(
					canonicalContractRoot, "parent-link/payload.bin",
					resolvedArtifact, pathContractStatus);
		}
		tests.Require(
			admittedNested && rejectedDriveQualified && rejectedParent &&
			rejectedSiblingAbsolute && rejectedParentLink,
			"Admit only canonical descendant candidate files and reject drive, sibling-prefix, parent, and parent-link escapes");
		pathContractError.clear();
		fs::remove_all(pathContractRoot, pathContractError);
		pathContractError.clear();
		fs::remove_all(siblingRoot, pathContractError);
		requester->Request_Close();
		participant->Request_Close();
		app.m_Sessions.clear();
		app.m_GameplayBindingBySessionId.clear();
		app.m_CharacterSelectArenas.clear();
		app.m_SharedGameRooms.clear();
		app.m_pActiveGameplayGeneration.reset();
	}
}

