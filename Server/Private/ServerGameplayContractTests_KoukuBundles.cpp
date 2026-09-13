#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "KoukuSaydonBrain.h"
#include "ServerApp.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_KoukuBundles(TESTS& tests)
{

#ifdef _DEBUG
	{
		// A temporary admitted generation exercises two actors without changing the user's empty entrance drafts.
		namespace fs = std::filesystem;
		std::vector<wchar_t> buffer(32768u); fs::path dataRoot;
		const DWORD configured = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", buffer.data(), static_cast<DWORD>(buffer.size()));
		if (configured && configured < buffer.size()) dataRoot = buffer.data();
		else { GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size())); dataRoot = fs::path(buffer.data()).parent_path().parent_path() / L"DataFiles"; }
		std::ifstream input(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", std::ios::binary);
		std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
		if (!bytes.empty() && bytes.back() != '\n') bytes += '\n';
		const std::string encounter = "ENCOUNTER_KAKULSAYDON_G1";
		const auto appendPattern = [&](const std::string& id, const std::string& boss, const std::string& placement, const unsigned duration)
		{
			bytes += "PATTERN	" + encounter + "	" + id + "	" + id + ".action	AUDITION_ONLY	0	0	0	0	0	0	0	1	1	ANY	ANY	0\n";
			bytes += "PATTERNBOSS	" + encounter + "	" + id + "	" + boss + "\n";
			bytes += "PATTERNPOLICY	" + encounter + "	" + id + "	NORMAL	1	1	NONE	NONE\n";
			bytes += "PATTERNSOURCE	" + encounter + "	" + id + "	1	0	0	0	0	0	0\n";
			bytes += "PATTERNSTAGE	" + encounter + "	" + id + "	0	STAGE_1	" + id + ".stage.1	ACTIVE	" + std::to_string(duration) + "	NONE	0	0	0	0	0	0	0	0	-	0	0	0	0\n";
			bytes += "PATTERNSTAGEBRANCH	" + encounter + "	" + id + "	" + id + ".stage.1	TIMEOUT	-\n";
			bytes += "PATTERNTARGET	" + id + "	GATE2	" + placement + "\n";
			bytes += "PATTERNSPAWNRESET	" + encounter + "	" + id + "	1\n";
		};
		appendPattern("KAKULSAYDON_G1_BUNDLE_A", "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 200u);
		appendPattern("KAKULSAYDON_G1_BUNDLE_B", "BOSS_KAKULSAYDON_G2_BIG_SAYDON", "boss.kakulsaydon.g2.big-saydon", 1000u);
		appendPattern("KAKULSAYDON_G1_BUNDLE_A_FOLLOW", "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 200u);
		const std::string reentryId = "KAKULSAYDON_G1_REENTRY_CONTRACT";
		appendPattern(reentryId, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 10000u);
		bytes += "PATTERNLOGIC\t" + encounter + "\t" + reentryId + "\t0\treentry.hit\tENTER_AREA\t0\t10000\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\n";
		bytes += "PATTERNLOGICREARM\t" + encounter + "\t" + reentryId + "\treentry.hit\tON_REENTER\n";
		bytes += "PATTERNLOGICREGION\t" + encounter + "\t" + reentryId + "\treentry.hit\t0\treentry.body\tBOSS_CURRENT\tCIRCLE\t0\t0\t0\t0\t1\t1\t1\t4\t45\tNONE\tNONE\n";
		bytes += "PATTERNLOGICOUTCOME\t" + encounter + "\t" + reentryId + "\treentry.hit\tSUCCESS\t0\tMAX_HP_PERCENT_DAMAGE\t10\t0\t-\n";
		bytes += "PATTERNLOGICPUSH\t" + encounter + "\t" + reentryId + "\treentry.hit\tSUCCESS\t0\t2\t242\n";
		const std::string ellipseId = "KAKULSAYDON_G1_ELLIPSE_CONTRACT";
		appendPattern(ellipseId, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 10000u);
		bytes += "PATTERNLOGIC\t" + encounter + "\t" + ellipseId + "\t0\tellipse.hit\tENTER_AREA\t0\t10000\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\n";
		bytes += "PATTERNLOGICREGION\t" + encounter + "\t" + ellipseId + "\tellipse.hit\t0\tellipse.body\tBOSS_CURRENT\tREVERSE_SECTOR\t0\t0\t0\t725\t1\t1\t1\t0.6\t0\tNONE\tNONE\t0.6\t12\n";
		bytes += "PATTERNLOGICOUTCOME\t" + encounter + "\t" + ellipseId + "\tellipse.hit\tSUCCESS\t0\tMAX_HP_PERCENT_DAMAGE\t10\t0\t-\n";
		bytes += "PATTERNLOGICPUSH\t" + encounter + "\t" + ellipseId + "\tellipse.hit\tSUCCESS\t0\t2\t242\tBOSS_FORWARD\n";
		const std::string retargetId = "KAKULSAYDON_G1_RETARGET_CONTRACT";
		bytes += "PATTERN\t" + encounter + "\t" + retargetId + "\t" + retargetId + ".action\tAUDITION_ONLY\t0\t0\t0\t0\t0\t0\t0\t1\t3\tANY\tANY\t0\n";
		bytes += "PATTERNBOSS\t" + encounter + "\t" + retargetId + "\tBOSS_KAKULSAYDON_G2_BIG_SAYDON\n";
		bytes += "PATTERNPOLICY\t" + encounter + "\t" + retargetId + "\tNORMAL\t1\t1\tNONE\tNONE\n";
		bytes += "PATTERNSOURCE\t" + encounter + "\t" + retargetId + "\t1\t0\t0\t0\t0\t0\t0\n";
		bytes += "PATTERNTARGET\t" + retargetId + "\tGATE2\tboss.kakulsaydon.g2.big-saydon\n";
		for (unsigned index = 0u; index < 3u; ++index)
		{
			const auto actionId = retargetId + ".stage." + std::to_string(index + 1u);
			bytes += "PATTERNSTAGE\t" + encounter + "\t" + retargetId + "\t" + std::to_string(index) + "\tSTAGE_" + std::to_string(index + 1u) + "\t" + actionId + "\tACTIVE\t100\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n";
			bytes += "PATTERNSTAGEBRANCH\t" + encounter + "\t" + retargetId + "\t" + actionId + "\tTIMEOUT\t" + (index < 2u ? retargetId + ".stage." + std::to_string(index + 2u) : "-") + "\n";
			if (index != 1u)
				bytes += "PATTERNSTAGEACTION\t" + encounter + "\t" + retargetId + "\t" + actionId + "\t0\tENTER\tRETARGET_RANDOM_ALIVE\tboss.target.pattern\t1\t0\n";
		}
		const std::string motionId = "KAKULSAYDON_G1_MOTION_CONTRACT";
		for (const bool invalidNavigation : {false, true})
		{
			const auto id = motionId + (invalidNavigation ? "_OFFNAV" : "");
			appendPattern(id, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 7400u);
			const std::string reset = "PATTERNSPAWNRESET	" + encounter + "	" + id + "	1\n";
			bytes.erase(bytes.size() - reset.size());
			bytes += "PATTERNBOSSMOTION	" + encounter + "	" + id + "	1870	5780	2.04	10.56	316.95	" +
				(invalidNavigation ? "99999" : "11.79") + "	10.56	326.79	314.7368\n";
		}
		const std::string contactId = "KAKULSAYDON_G1_CONTACT_CONTRACT";
		const auto appendContactFixture = [&](const std::string& id, const bool miss, const bool lastTick)
		{
			appendPattern(id, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 1000u);
			const auto row = [&](const std::initializer_list<std::string> fields)
			{
				bool first = true;
				for (const auto& field : fields) { if (!first) bytes += '\t'; first = false; bytes += field; }
				bytes += '\n';
			};
			row({ "PATTERNWORLDSEQUENCE", encounter, id, "0", "world.contact.card", "1", "0", "0", "0", "NONE", "0", "0", "0", "1000", "contact.card" });
			row({ "PATTERNWORLDPLACEMENT", encounter, id, "contact.card", "0", "2", "0", "15", "0", "-10", "0.5", "2", "1.5" });
			row({ "PATTERNLOGIC", encounter, id, "0", "contact.deadline", "EXTERNAL_SIGNAL", "0", "1000", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", lastTick ? "1" : "0", "-", "0", "0" });
			row({ "PATTERNLOGICOUTCOME", encounter, id, "contact.deadline", "SUCCESS", "0", "FOLLOWUP_PATTERN", "0", "0", "KAKULSAYDON_G1_BUNDLE_A_FOLLOW" });
			row({ "PATTERNLOGICOUTCOME", encounter, id, "contact.deadline", "TIMEOUT", "0", "FOLLOWUP_PATTERN", "0", "0", "KAKULSAYDON_G1_BUNDLE_A" });
			row({ "PATTERNLOGIC", encounter, id, "1", "contact.hit", "OBJECT_CONTACT", lastTick ? "900" : "100", "100", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "-", "0", "0" });
			row({ "PATTERNLOGICCONTACTGROUP", encounter, id, "contact.hit", "-", "100" });
			row({ "PATTERNLOGICCONTACTTARGET", encounter, id, "contact.hit", "contact.card", "world.contact.card", "0", "0", "0.1" });
			row({ "PATTERNLOGICREGION", encounter, id, "contact.hit", "0", "contact.region", "WORLD", "CIRCLE", miss ? "100" : "0", "0", "0", "0", "1", "1", "1", "1", "45", "NONE", "NONE" });
			if (lastTick)
			{
				row({ "PATTERNLOGICREGIONWORLD", encounter, id, "contact.hit", "contact.region", "900", "0", "100", "1", "0", "0", "0", "0", "0", "1", "1", "1" });
				row({ "PATTERNLOGICREGIONWORLDKEY", encounter, id, "contact.hit", "contact.region", "0", "0", "0", "0", "0", "0", "1", "1", "1", "1", "0" });
				row({ "PATTERNLOGICREGIONWORLDKEY", encounter, id, "contact.hit", "contact.region", "1", "100", "0", "0", "0", "0", "1", "1", "1", "1", "1" });
			}
			row({ "PATTERNLOGICOUTCOME", encounter, id, "contact.hit", "SUCCESS", "0", "PLAY_CONTACT_WORLD_OBJECT_MOTION", "0", "0", "-" });
			row({ "PATTERNLOGICCONTACTMOTION", encounter, id, "contact.hit", "SUCCESS", "0", "contact.card", "world.contact.flip" });
			row({ "PATTERNLOGICOUTCOME", encounter, id, "contact.hit", "SUCCESS", "1", "COMPLETE_LOGIC_WINDOW", "0", "0", "-" });
			row({ "PATTERNLOGICSIGNAL", encounter, id, "contact.hit", "SUCCESS", "1", "contact.deadline", "contact.card" });
		};
		appendContactFixture(contactId, false, false);
		appendContactFixture(contactId + "_MISS", true, false);
		appendContactFixture(contactId + "_FINAL", false, true);
		for (const unsigned offset : {0u, 67u})
		{
			const std::string id = "kakulsaydon.bundle.contract." + std::to_string(offset);
			bytes += "PATTERNBUNDLE	" + id + "	" + encounter + "	GATE2\n";
			bytes += "PATTERNBUNDLEMEMBER	" + id + "	member.a	KAKULSAYDON_G1_BUNDLE_A	boss.kakulsaydon.g2.kouku	0\n";
			bytes += "PATTERNBUNDLEMEMBER	" + id + "	member.b	KAKULSAYDON_G1_BUNDLE_B	boss.kakulsaydon.g2.big-saydon	" + std::to_string(offset) + "\n";
		}
		const auto headerEnd = bytes.find('\n');
		const auto headerCount = bytes.rfind('\t', headerEnd);
		bytes.replace(headerCount + 1u, headerEnd - headerCount - 1u,
			std::to_string(std::count(bytes.begin(), bytes.end(), '\n') - 1u));
		const fs::path directory = fs::temp_directory_path() / (L"LostArkKoukuBundleContract-" + std::to_wstring(GetCurrentProcessId()));
		std::error_code error; fs::create_directories(directory, error); const fs::path path = directory / L"Gameplay.bootstrap";
		{ std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); }
		GameplayDataRevision revision; std::string status; auto generation = std::make_shared<CGameplayCatalog>();
		const bool loaded = !error && CServerApp::Hash_GameplayFileForAdmission(path, revision, status) && generation->Load_FromBootstrap(fs::canonical(path), revision, revision);
		if (!loaded) std::cout << "[STATUS] Bundle fixture: " << generation->Get_Status() << " / " << status << '\n';
		tests.Require(loaded, "Bundle loads typed target/member rows through normal catalog admission");
		if (loaded)
		{
			const auto* reentry = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, reentryId, status);
			tests.Require(reentry && reentry->LogicWindows.size() == 1u && reentry->LogicWindows.front().bRearmOnExit &&
				reentry->LogicWindows.front().OnSuccess.front().fPushRangeM == 2.f &&
				reentry->LogicWindows.front().OnSuccess.front().iPushMs == 242u,
				"Published supplemental rearm and push rows reach the admitted Kouku pattern");
			const auto* elliptic = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, ellipseId, status);
			tests.Require(elliptic && elliptic->LogicWindows.size() == 1u &&
				elliptic->LogicWindows.front().CardRegions.front().bReverseSector &&
				elliptic->LogicWindows.front().CardRegions.front().fRadiusXM == .6f &&
				elliptic->LogicWindows.front().CardRegions.front().fRadiusZM == 12.f &&
				elliptic->LogicWindows.front().CardRegions.front().fHalfAngleDegrees == 0.f &&
				elliptic->LogicWindows.front().OnSuccess.front().ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD,
				"Extended sector-axis and forward-push rows reach the real admitted Server catalog together");
			if (reentry)
			{
				auto invalid = *reentry; invalid.LogicWindows.front().eKind = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP;
				tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status), "Only ENTER_AREA may rearm on exit");
				invalid = *reentry; invalid.LogicWindows.front().OnSuccess.front().iPushMs = 0u;
				tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status), "Push range and duration must be paired");
				invalid = *reentry; invalid.LogicWindows.front().OnSuccess.front().fPushRangeM = 21.f;
				tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status), "Logic push cannot exceed the authored twenty-metre bound");
			}
		}
		if (loaded)
		{
			const auto makeRoom = [&]()
			{
				auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA, generation);
				for (const char* id : {"boss.kakulsaydon.g2.kouku", "boss.kakulsaydon.g2.big-saydon"})
				{
					const auto* placement = room->Find_Placement(id); SERVER_WORLD_ENTITY entity;
					if (placement && room->Build_WorldEntity(*placement, room->m_iNextNetEntityId, entity)) { ++room->m_iNextNetEntityId; room->m_WorldEntities.push_back(std::move(entity)); }
				}
				return room;
			};
			const auto getBoss = [](CGameRoom& room, const bool second) { return room.Find_KoukuSaydonArenaBoss(second ? "boss.kakulsaydon.g2.big-saydon" : "boss.kakulsaydon.g2.kouku", second ? "BOSS_KAKULSAYDON_G2_BIG_SAYDON" : "BOSS_KAKULSAYDON_G2_KOUKU"); };
			const auto requestFor = [&](CGameRoom& room, const unsigned offset)
			{
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request; request.iRequestSequence = 1u; request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_BUNDLE;
				request.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; request.Scope.strEncounterId = encounter; request.Scope.strGateId = "GATE2";
				request.Scope.ExpectedGameplayRevision = room.m_GameplayCatalog.Get_ActiveRevision(); request.Scope.iExpectedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(room.m_GameplayCatalog.Active());
				request.strBundleId = "kakulsaydon.bundle.contract." + std::to_string(offset); return request;
			};
			const auto tick = [](CGameRoom& room) { room.Update_WorldEntities(1.f / 30.f); ++room.m_iServerTick; };
			{
				auto contactRoom = makeRoom(); auto contactRequest = requestFor(*contactRoom, 0u);
				contactRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				contactRequest.strBundleId.clear(); contactRequest.strPatternId = contactId;
				contactRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku";
				contactRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT contactResult;
				const bool contactQueued = contactRoom->Evaluate_KoukuSaydonPatternAudition(908u, contactRequest, contactResult) ==
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
				for (unsigned i = 0u; i < 5u; ++i) tick(*contactRoom);
				const auto* contactBoss = getBoss(*contactRoom, false);
				const auto& contactRun = contactRoom->m_KoukuSaydonPatternAudition;
				tests.Require(contactQueued && contactBoss && contactBoss->strPatternId == contactId &&
					contactRun.Members.size() == 1u && contactRun.Members.front().PatternIds.size() == 2u &&
					contactRun.WorldPlays.size() == 2u && contactRun.WorldPlays.back().strTargetCueId == contactRun.WorldPlays.front().strCueId &&
					contactRun.WorldPlays.front().bHasPlacement && contactRun.WorldPlays.front().fWorldPositionY == 2.f &&
					contactRun.WorldPlays.front().fWorldRotationXDegrees == 15.f && contactRun.WorldPlays.front().fWorldRotationZDegrees == -10.f &&
					contactRun.WorldPlays.front().fWorldScaleX == .5f && contactRun.WorldPlays.front().fWorldScaleY == 2.f &&
					contactRun.WorldPlays.front().fWorldScaleZ == 1.5f && !contactRun.WorldPlays.back().bHasPlacement,
					"Placed Contact bootstrap passes Room and Brain admission, preserves full TRS and signals the exact owned card without replacing its placement");
			}
			for (const bool finalTickSuccess : { false, true })
			{
				auto deadlineRoom = makeRoom(); auto deadlineRequest = requestFor(*deadlineRoom, 0u);
				deadlineRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				deadlineRequest.strBundleId.clear(); deadlineRequest.strPatternId = contactId + (finalTickSuccess ? "_FINAL" : "_MISS");
				deadlineRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku";
				deadlineRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT deadlineResult;
				const bool queuedDeadline = deadlineRoom->Evaluate_KoukuSaydonPatternAudition(909u, deadlineRequest, deadlineResult) ==
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
				for (unsigned i = 0u; i < 30u; ++i) tick(*deadlineRoom);
				const auto* beforeBoss = getBoss(*deadlineRoom, false);
				const bool heldLastPose = beforeBoss && beforeBoss->strPatternId == deadlineRequest.strPatternId &&
					deadlineRoom->m_KoukuSaydonPatternAudition.Members.size() == 1u &&
					deadlineRoom->m_KoukuSaydonPatternAudition.Members.front().PatternIds.size() == 1u;
				tick(*deadlineRoom);
				const auto& deadlineRun = deadlineRoom->m_KoukuSaydonPatternAudition;
				tests.Require(queuedDeadline && heldLastPose && deadlineRun.Members.size() == 1u &&
					deadlineRun.Members.front().PatternIds.size() == 2u &&
					deadlineRun.Members.front().PatternIds.back() == (finalTickSuccess ? "KAKULSAYDON_G1_BUNDLE_A_FOLLOW" : "KAKULSAYDON_G1_BUNDLE_A") &&
					deadlineRun.WorldPlays.size() == (finalTickSuccess ? 2u : 1u), finalTickSuccess ?
					"Actual Room holds the final pose and resolves last-tick contact success before timeout and early completion" :
					"Actual Room preserves a missed strike until the external deadline and emits its timeout exactly once");
			}
			{
				auto motionRoom = makeRoom(); auto motionRequest = requestFor(*motionRoom, 0u);
				motionRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				motionRequest.strBundleId.clear(); motionRequest.strPatternId = motionId;
				motionRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku";
				motionRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT motionResult;
				const bool motionQueued = motionRoom->Evaluate_KoukuSaydonPatternAudition(920u, motionRequest, motionResult) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
				if (!motionQueued) std::cout << "[STATUS] Boss Motion admission: " << motionRoom->m_strStatus << '\n';
				tick(*motionRoom); auto* moving = getBoss(*motionRoom, false);
				const auto close = [](float a, float b) { return std::abs(a - b) < .0001f; };
				tests.Require(motionQueued && moving && close(moving->fPositionX, 2.04f) && close(moving->fPositionY, 10.56f) && close(moving->fPositionZ, 316.95f), "Boss Motion commits its authored start through normal Room admission");
				for (unsigned i = 0u; i < 56u; ++i) tick(*motionRoom);
				tests.Require(moving && close(moving->fPositionX, 2.04f), "Boss Motion holds its start through the last tick before 1870 ms");
				tick(*motionRoom);
				tests.Require(moving && moving->fPositionX > 2.04f && moving->fPositionX < 2.14f && close(moving->fPositionY, 10.56f), "Boss Motion starts within one 30 Hz tick and keeps base height separate from animation pose");
				for (unsigned i = 58u; i < 175u; ++i) tick(*motionRoom);
				tests.Require(moving && close(moving->fPositionX, 11.79f) && close(moving->fPositionZ, 326.79f) && close(moving->fYawDegrees, 314.7368f), "Boss Motion reaches the endpoint on the first tick at or after 5780 ms");
				for (unsigned i = 175u; i < 240u; ++i) tick(*motionRoom);
				tests.Require(moving && moving->strPatternId.empty() && close(moving->fPositionX, 11.79f) && close(moving->fPositionZ, 326.79f), "Natural completion holds the authored endpoint instead of replaying horizontal return");
				const auto* definition = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, motionId, status);
				if (definition && moving)
				{
					auto invalid = *definition; invalid.BossMotion->iEndMs = 7401u;
					auto untouched = *moving;
					tests.Require(!CKoukuSaydonBrain{}.Begin_Pattern(untouched, invalid, revision, 300u, status) && close(untouched.fPositionX, moving->fPositionX), "Invalid Boss Motion preserves the previous boss transform");
					auto wrapped = *moving; wrapped.strPatternId = motionId; wrapped.iPatternStartTick = (std::numeric_limits<std::uint32_t>::max)() - 99u;
					CKoukuSaydonBrain::Apply_BossMotion(wrapped, *definition, 100u);
					tests.Require(close(wrapped.fPositionX, 11.79f), "Boss Motion retains the endpoint across reserved-zero server tick wrap");
				}
				auto failedMotion = makeRoom(); motionRequest.Scope.ExpectedGameplayRevision = failedMotion->m_GameplayCatalog.Get_ActiveRevision(); motionRequest.strPatternId = motionId + "_OFFNAV";
				const float oldX = getBoss(*failedMotion, false)->fPositionX;
				tests.Require(failedMotion->Evaluate_KoukuSaydonPatternAudition(921u, motionRequest, motionResult) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_UNSUPPORTED_PATTERN &&
					getBoss(*failedMotion, false)->fPositionX == oldX && failedMotion->m_KoukuSaydonPatternAudition.Members.empty(), "Off-navigation Boss Motion fails before moving or reserving the actor");
			}
			{
				auto targetingRoom = makeRoom(); auto targetingRequest = requestFor(*targetingRoom, 0u);
				targetingRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				targetingRequest.strBundleId.clear(); targetingRequest.strPatternId = retargetId;
				targetingRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g2.big-saydon";
				targetingRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
				auto* targetingBoss = getBoss(*targetingRoom, true);
				SERVER_PLAYER target{}; target.iPlayerId = 940u; target.iNetEntityId = 940u;
				target.iCurrentHp = 100u; target.isCombatReady = true;
				target.fPositionX = targetingBoss->fPositionX + 5.f;
				target.fPositionY = targetingBoss->fPositionY; target.fPositionZ = targetingBoss->fPositionZ;
				targetingRoom->m_Players.emplace(target.iPlayerId, target);
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT targetingResult;
				const bool admitted = targetingRoom->Evaluate_KoukuSaydonPatternAudition(940u, targetingRequest, targetingResult) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
				tick(*targetingRoom);
				const auto approximatelyEqual = [](float a, float b) { return std::abs(a - b) < .0001f; };
				const auto facesTargetWithLocalX = [](const SERVER_WORLD_ENTITY& boss, const SERVER_PLAYER& player)
				{
					const float dx = player.fPositionX - boss.fPositionX, dz = player.fPositionZ - boss.fPositionZ;
					const float length = std::sqrt(dx * dx + dz * dz);
					const float radians = boss.fYawDegrees * 0.01745329251994329577f;
					return length > .000001f &&
						(std::cos(radians) * dx - std::sin(radians) * dz) / length > .999999f;
				};
				tests.Require(admitted && targetingBoss->iPatternTargetEntityId == target.iNetEntityId &&
					targetingBoss->bHasPatternTargetLastPosition && approximatelyEqual(targetingBoss->fYawDegrees, 0.f) &&
					approximatelyEqual(targetingBoss->fPatternTargetLastPositionX, target.fPositionX) &&
					facesTargetWithLocalX(*targetingBoss, target),
					"Kouku ENTER reuses Server alive target selection and stores one position/facing sample");
				auto& moved = targetingRoom->m_Players.at(target.iPlayerId);
				moved.fPositionX = targetingBoss->fPositionX; moved.fPositionZ = targetingBoss->fPositionZ + 5.f;
				for (unsigned i = 0u; i < 4u; ++i) tick(*targetingRoom);
				tests.Require(targetingBoss->iPatternStageIndex == 1u && approximatelyEqual(targetingBoss->fYawDegrees, 0.f) &&
					approximatelyEqual(targetingBoss->fPatternTargetLastPositionX, target.fPositionX),
					"Kouku holds sampled yaw and last target XYZ through movement and an unmarked stage");
				tick(*targetingRoom);
				tests.Require(targetingBoss->iPatternStageIndex == 2u && approximatelyEqual(targetingBoss->fYawDegrees, -90.f) &&
					approximatelyEqual(targetingBoss->fPatternTargetLastPositionZ, moved.fPositionZ) &&
					facesTargetWithLocalX(*targetingBoss, moved),
					"The next marked Stage ENTER takes the player's current position exactly once");
				moved.fPositionX -= 5.f; tick(*targetingRoom);
				tests.Require(approximatelyEqual(targetingBoss->fYawDegrees, -90.f), "Kouku retarget does not track the player every tick");
				const auto* definition = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, retargetId, status);
				if (definition)
				{
					auto invalid = *definition; invalid.Stages.front().Actions.front().iValue = 2u;
					tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status), "Kouku rejects malformed retarget actions");
					invalid = *definition; invalid.Stages.front().Actions.push_back(invalid.Stages.front().Actions.front());
					tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status), "Kouku rejects duplicate stage retarget writers");
					invalid = *definition; invalid.BossMotion.emplace();
					tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status), "Kouku rejects retarget with fixed-yaw Boss Motion");
				}
			}
			auto room = makeRoom(); auto request = requestFor(*room, 0u); S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
			const bool queued = room->Evaluate_KoukuSaydonPatternAudition(901u, request, result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
			tick(*room); auto* first = getBoss(*room, false); auto* second = getBoss(*room, true);
			tests.Require(queued && first && second && first->iPatternStartTick == 1u && second->iPatternStartTick == 1u, "Bundle actors start on the same exact first Server tick");
			S2C_KOUKUSAYDON_BUNDLE_STATE replicated;
			tests.Require(room->Build_KoukuBundleState(replicated) && replicated.Members.size() == 2u && replicated.iCommonStartTick == 1u, "Bundle room replication includes both actors and the shared clock");
			if (first)
			{
				KOUKUSAYDON_LOGIC_OUTPUT followup; followup.FollowupPatternIds = {"KAKULSAYDON_G1_BUNDLE_A_FOLLOW"}; room->Apply_KoukuLogicOutput(followup, *first, 1u);
				tests.Require(room->m_KoukuSaydonPatternAudition.Members[0].PatternIds.size() == 2u && room->m_KoukuSaydonPatternAudition.Members[1].PatternIds.size() == 1u, "Bundle follow-up remains in the requesting member chain");
			}
			for (unsigned i=0;i<18u;++i) tick(*room);
			tests.Require(room->Build_KoukuBundleState(replicated) && replicated.Members[0].eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::COMPLETED && replicated.Members[1].eState == KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE::ACTIVE, "Early member completion preserves the running sibling and parent");
			for (unsigned i=0;i<20u;++i) tick(*room);
			tests.Require(room->m_KoukuSaydonPatternAudition.ePhase == CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE, "Bundle finishes only when all member chains complete");
			auto isolated = makeRoom(); auto one = requestFor(*isolated,0u);
			one.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED; one.strBundleId.clear();
			one.strPatternId = "KAKULSAYDON_G1_BUNDLE_A"; one.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku"; one.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
			const bool singleQueued = isolated->Evaluate_KoukuSaydonPatternAudition(905u,one,result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED; tick(*isolated);
			tests.Require(singleQueued && getBoss(*isolated,false)->iPatternStartTick == 1u && getBoss(*isolated,true)->strPatternId.empty(), "Single child audition starts only its exact actor");
			auto ordered = makeRoom();
			const auto* orderedPlacement = ordered->Find_Placement("boss.kakulsaydon.g1.saydon"); SERVER_WORLD_ENTITY orderedBoss;
			const bool orderedReady = orderedPlacement && ordered->Build_WorldEntity(*orderedPlacement, ordered->m_iNextNetEntityId, orderedBoss);
			if (orderedReady) { ++ordered->m_iNextNetEntityId; ordered->m_WorldEntities.push_back(std::move(orderedBoss)); }
			auto all = requestFor(*ordered,0u); all.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_ALL; all.strBundleId.clear(); all.Scope.strGateId = "GATE1";
			all.Scope.strBossPlacementId = "boss.kakulsaydon.g1.saydon"; all.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
			const bool orderedQueued = ordered->Evaluate_KoukuSaydonPatternAudition(906u,all,result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
			tests.Require(orderedReady && orderedQueued && ordered->m_KoukuSaydonPatternAudition.Members.size() == 1u && ordered->m_KoukuSaydonPatternAudition.Members.front().PatternIds.size() > 1u, "Existing Play All remains one actor with a sequential Product queue");
			auto worldRoom = makeRoom(); auto worldRequest = requestFor(*worldRoom,0u); worldRoom->Evaluate_KoukuSaydonPatternAudition(907u,worldRequest,result); tick(*worldRoom);
			for (const bool b : {false,true})
			{
				KOUKUSAYDON_LOGIC_OUTPUT worldOutput; KOUKUSAYDON_LOGIC_WORLD_PLAY play; play.strInstanceId = b ? "world.second" : "world.first"; play.strOccurrenceId = b ? "occurrence.second" : "occurrence.first"; play.iStartTick = 1u; play.iDurationMs = 1000u;
				BOSS_PATTERN_WORLD_PLACEMENT placement; placement.fPositionX = b ? 20.f : 10.f;
				placement.fRotationYDegrees = 45.f; placement.fScaleY = 2.f; play.Placement = placement;
				worldOutput.WorldSequencePlays.push_back(play); worldRoom->Apply_KoukuLogicOutput(worldOutput,*getBoss(*worldRoom,b),1u);
			}
			KOUKUSAYDON_LOGIC_OUTPUT motionOutput; KOUKUSAYDON_LOGIC_WORLD_PLAY motion; motion.strInstanceId = "world.motion"; motion.strTargetSequenceInstanceId = "world.first"; motion.iStartTick = 2u; motionOutput.WorldSequencePlays.push_back(motion);
			worldRoom->Apply_KoukuLogicOutput(motionOutput,*getBoss(*worldRoom,false),2u);
			tests.Require(worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.size() == 3u && worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.back().strTargetCueId == worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.front().strCueId, "World motion targets the exact owned activation while preserving authored occurrence ID");
			// Two placements deliberately reuse the same saved instance; occurrence identity chooses the first card.
			const auto firstCueId = worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.front().strCueId;
			KOUKUSAYDON_LOGIC_OUTPUT duplicateIdle; KOUKUSAYDON_LOGIC_WORLD_PLAY duplicatePlay;
			duplicatePlay.strInstanceId = "world.first"; duplicatePlay.strOccurrenceId = "occurrence.first.again";
			duplicatePlay.iStartTick = 2u; duplicatePlay.iDurationMs = 1000u;
			BOSS_PATTERN_WORLD_PLACEMENT duplicatePlacement; duplicatePlacement.fPositionX = 99.f; duplicatePlacement.fScaleX = 3.f;
			duplicatePlay.Placement = duplicatePlacement; duplicateIdle.WorldSequencePlays.push_back(duplicatePlay);
			worldRoom->Apply_KoukuLogicOutput(duplicateIdle, *getBoss(*worldRoom, false), 2u);
			motionOutput.WorldSequencePlays.front().strTargetWorldOccurrenceId = "occurrence.first";
			worldRoom->Apply_KoukuLogicOutput(motionOutput, *getBoss(*worldRoom, false), 3u);
			tests.Require(worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.size() == 5u &&
				worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.back().strTargetCueId == firstCueId &&
				worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.front().fWorldPositionX == 10.f &&
				worldRoom->m_KoukuSaydonPatternAudition.WorldPlays[3u].fWorldPositionX == 99.f &&
				worldRoom->m_KoukuSaydonPatternAudition.WorldPlays[3u].fWorldScaleX == 3.f &&
				!worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.back().bHasPlacement,
				"Contact motion selects the exact card while both independent absolute placements remain available for replay");
			worldRoom->Stop_KoukuWorldOwner("member.a");
			tests.Require(worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.size() == 1u && worldRoom->m_KoukuSaydonPatternAudition.WorldPlays.front().strMemberId == "member.b" &&
				worldRoom->m_KoukuSaydonPatternAudition.Members.front().WorldCueByOccurrence.empty(), "Stopping one World owner preserves its sibling's activation and clears occurrence bindings");
			auto delayed = makeRoom(); auto delayRequest = requestFor(*delayed, 67u); delayed->Evaluate_KoukuSaydonPatternAudition(902u, delayRequest, result); tick(*delayed);
			tests.Require(getBoss(*delayed,false)->iPatternStartTick == 1u && getBoss(*delayed,true)->strPatternId.empty(), "Delayed member stays reserved before its offset");
			for(unsigned i=0;i<3u;++i) tick(*delayed);
			tests.Require(getBoss(*delayed,true)->iPatternStartTick == 4u, "Bundle rounds 67ms offset up to three fixed ticks");
			auto restart = delayRequest; restart.iRequestSequence = 2u; restart.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::RESTART_BUNDLE; restart.iExpectedRunEpoch = result.iRoomAuditionEpoch;
			const auto oldEpoch = result.iRoomAuditionEpoch;
			tests.Require(delayed->Evaluate_KoukuSaydonPatternAudition(902u,restart,result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED && result.iRoomAuditionEpoch != oldEpoch && result.iCommonStartTick == 5u, "Restart creates a new run epoch and common clock after cleanup");
			auto stop = restart; stop.iRequestSequence = 3u; stop.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::STOP; stop.iExpectedRunEpoch = oldEpoch;
			tests.Require(delayed->Evaluate_KoukuSaydonPatternAudition(902u,stop,result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_STALE_REQUEST, "Old run stop cannot terminate a restarted bundle");
			stop.iRequestSequence = 4u; stop.iExpectedRunEpoch = delayed->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
			tests.Require(delayed->Evaluate_KoukuSaydonPatternAudition(902u,stop,result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::STOPPED && delayed->m_KoukuSaydonPatternAudition.Members.empty() && getBoss(*delayed,false)->strPatternId.empty() && getBoss(*delayed,true)->strPatternId.empty(), "Exact Stop clears active and scheduled bundle participants");
			auto failed = makeRoom(); auto failRequest = requestFor(*failed,0u); auto* failFirst = getBoss(*failed,false); auto* failSecond = getBoss(*failed,true);
			failFirst->fPositionX += 3.f; const float before = failFirst->fPositionX; failSecond->iCurrentHp = 0u;
			tests.Require(failed->Evaluate_KoukuSaydonPatternAudition(903u,failRequest,result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_BOSS_DEAD && failFirst->fPositionX == before && failFirst->strPatternId.empty() && failed->m_KoukuSaydonPatternAudition.Members.empty(), "Second actor failure does not reset or start the first actor");
			auto lost = makeRoom(); auto lostRequest = requestFor(*lost,67u); lost->Evaluate_KoukuSaydonPatternAudition(904u,lostRequest,result); tick(*lost); getBoss(*lost,true)->iCurrentHp = 0u; tick(*lost);
			tests.Require(lost->m_KoukuSaydonPatternAudition.Members.empty() && getBoss(*lost,false)->strPatternId.empty(), "Delayed participant death aborts and cleans the entire bundle");
		}
		if (loaded)
		{
			// The production admission reads a newly published file while this room
			// retains its original process catalog. Never replace the user's Product.
			auto reloadRoom = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA, generation);
			for (const char* id : {"boss.kakulsaydon.g2.kouku", "boss.kakulsaydon.g2.big-saydon"})
			{
				const auto* placement = reloadRoom->Find_Placement(id); SERVER_WORLD_ENTITY entity;
				if (placement && reloadRoom->Build_WorldEntity(*placement, reloadRoom->m_iNextNetEntityId, entity))
				{ ++reloadRoom->m_iNextNetEntityId; reloadRoom->m_WorldEntities.push_back(std::move(entity)); }
			}
			const auto oldSource = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*generation);
			C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST play{};
			play.iRequestSequence = 1u; play.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_BUNDLE;
			play.Scope.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; play.Scope.strEncounterId = encounter;
			play.Scope.strGateId = "GATE2"; play.Scope.ExpectedGameplayRevision = revision;
			play.Scope.iExpectedSourceRevision = oldSource; play.strBundleId = "kakulsaydon.bundle.contract.0";
			S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, play, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED, "Kouku live reload begins from the original room generation");
			const auto initialPin = reloadRoom->m_KoukuSaydonPatternAudition.pProductGeneration;
			const auto initialEpoch = result.iRoomAuditionEpoch;
			const auto publishProject = directory / L"published";
			const auto publishRoot = publishProject / L"Server" / L"Bin" / L"DataFiles";
			fs::create_directories(publishRoot / L"Gameplay", error);
			const auto ownerScript = publishProject / L"Tools" / L"Build" / L"Invoke-BuildDomainOwner.ps1";
			const auto lockPath = publishProject / L"out" / L"BuildPipeline" / L"receipts" / L"locks" / L"runtime-owner.lock";
			fs::create_directories(ownerScript.parent_path(), error);
			fs::create_directories(lockPath.parent_path(), error);
			{ std::ofstream marker(ownerScript); marker << "# Temporary publisher lock fixture\n"; }
			const auto publishedPath = publishRoot / L"Gameplay" / L"Gameplay.bootstrap";
			const std::string sourcePrefix = "KOUKUSAYDONPRODUCTREVISION\t" + encounter + "\tBOSS_KAKULSAYDON_G1_KOUKU\t";
			const auto changeSource = [&](std::string value, unsigned source)
			{
				const auto start = value.find(sourcePrefix) + sourcePrefix.size();
				value.replace(start, value.find_first_of("\r\n", start) - start, std::to_string(source));
				return value;
			};
			std::string published = changeSource(bytes, oldSource + 1u);
			const std::string shortStage = "KAKULSAYDON_G1_BUNDLE_A.stage.1\tACTIVE\t200\t";
			const auto stageAt = published.find(shortStage);
			if (stageAt != std::string::npos) published.replace(stageAt, shortStage.size(),
				"KAKULSAYDON_G1_BUNDLE_A.stage.1\tACTIVE\t700\t");
			const auto publish = [&](const std::string& contents)
			{
				std::ofstream output(publishedPath, std::ios::binary | std::ios::trunc);
				output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
			};
			publish(published);
			std::vector<wchar_t> previousRoot(32768u);
			const auto previousLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", previousRoot.data(), static_cast<DWORD>(previousRoot.size()));
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", publishRoot.c_str());
			const HANDLE publishing = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
				0u, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			CGameplayCatalog blockedProduct;
			tests.Require(publishing != INVALID_HANDLE_VALUE && !blockedProduct.Load_PublishedKoukuProduct(*generation) &&
				blockedProduct.Get_Status().find("publish is in progress") != std::string::npos,
				"Kouku admission cannot read a publisher transaction before commit or rollback");
			auto restart = play; restart.iRequestSequence = 2u;
			restart.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::RESTART_BUNDLE;
			restart.iExpectedRunEpoch = initialEpoch;
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, restart, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED && result.iPinnedSourceRevision == oldSource &&
				reloadRoom->m_KoukuSaydonPatternAudition.pProductGeneration == initialPin,
				"Publish preserves the original immutable Product when restarting an existing run");
			if (publishing != INVALID_HANDLE_VALUE) CloseHandle(publishing);
			auto stop = restart; stop.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::STOP;
			stop.iRequestSequence = 3u; stop.iExpectedRunEpoch = result.iRoomAuditionEpoch;
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, stop, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::STOPPED, "Original source can stop its run after Publish");
			play.iRequestSequence = 4u; play.Scope.iExpectedSourceRevision = oldSource + 1u;
			const bool refreshed = reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, play, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
			if (!refreshed) std::cout << "[STATUS] Kouku live reload: " << result.strReason << '\n';
			const auto refreshedPin = reloadRoom->m_KoukuSaydonPatternAudition.pProductGeneration;
			tests.Require(refreshed && result.iPinnedSourceRevision == oldSource + 1u &&
				refreshedPin != initialPin && refreshedPin && refreshedPin->Get_ActiveRevision() != revision &&
				reloadRoom->Get_ActiveGameplayGeneration() == generation && result.PinnedGameplayRevision == revision,
				"Next Complete Play admits published Kouku source while preserving the real process gameplay hash");
			for (unsigned tick = 0u; tick < 10u && refreshed; ++tick)
			{ reloadRoom->Update_WorldEntities(1.f / 30.f); ++reloadRoom->m_iServerTick; }
			auto* liveBoss = reloadRoom->Find_KoukuSaydonArenaBoss("boss.kakulsaydon.g2.kouku", "BOSS_KAKULSAYDON_G2_KOUKU");
			tests.Require(refreshed && liveBoss && liveBoss->strPatternId == "KAKULSAYDON_G1_BUNDLE_A" &&
				liveBoss->PinnedDefinitionRevision == revision,
				"Server ticks consume the newly published 700ms stage instead of the old 200ms stage");
			const auto refreshedEpoch = reloadRoom->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
			stop = play; stop.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::STOP;
			stop.iRequestSequence = 5u; stop.iExpectedRunEpoch = refreshedEpoch;
			stop.Scope.iExpectedSourceRevision = oldSource;
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, stop, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SOURCE_REVISION_MISMATCH &&
				reloadRoom->m_KoukuSaydonPatternAudition.pProductGeneration == refreshedPin &&
				reloadRoom->m_KoukuSaydonPatternAudition.iRoomAuditionEpoch == refreshedEpoch,
				"Stale source Stop preserves the exact running generation and epoch");
			stop.iRequestSequence = 6u; stop.Scope.iExpectedSourceRevision = oldSource + 1u;
			(void)reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, stop, result);
			play.iRequestSequence = 7u; play.Scope.iExpectedSourceRevision = oldSource + 2u;
			publish("truncated published candidate\n");
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, play, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SOURCE_REVISION_MISMATCH &&
				reloadRoom->m_pKoukuPublishedProductGeneration == refreshedPin &&
				reloadRoom->m_KoukuSaydonPatternAudition.Members.empty(),
				"Corrupt published bootstrap cannot replace the last successful Product or start a partial run");
			publish(published);
			play.iRequestSequence = 8u;
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, play, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SOURCE_REVISION_MISMATCH &&
				reloadRoom->m_pKoukuPublishedProductGeneration == refreshedPin,
				"Unpublished requested source cannot borrow a different published revision");
			auto foreign = changeSource(published, oldSource + 2u);
			const auto damageAt = foreign.find("\nDAMAGE\t");
			if (damageAt != std::string::npos)
			{
				const auto damageEnd = foreign.find('\n', damageAt + 1u);
				const auto valueAt = foreign.rfind('\t', damageEnd) + 1u;
				foreign.replace(valueAt, damageEnd - valueAt, "99999");
			}
			publish(foreign); play.iRequestSequence = 9u;
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, play, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED &&
				result.iPinnedSourceRevision == oldSource + 2u &&
				reloadRoom->m_KoukuSaydonPatternAudition.pProductGeneration->Has_SameNonKoukuGameplay(*generation) &&
				reloadRoom->Get_ActiveGameplayGeneration() == generation,
				"Kouku reload admits only encounter edits and retains active damage despite unrelated disk changes");
			stop = play; stop.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::STOP;
			stop.iRequestSequence = 10u; stop.iExpectedRunEpoch = result.iRoomAuditionEpoch;
			(void)reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, stop, result);
			publish(published); play.iRequestSequence = 11u; play.Scope.iExpectedSourceRevision = oldSource;
			tests.Require(reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, play, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_SOURCE_REVISION_MISMATCH,
				"A new run cannot roll back to an older source after a newer Product was admitted");
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", previousLength && previousLength < previousRoot.size() ? previousRoot.data() : nullptr);
			fs::remove(publishedPath, error); fs::remove(publishRoot / L"Gameplay", error); fs::remove(publishRoot, error);
			fs::remove(publishRoot.parent_path(), error); fs::remove(publishProject / L"Server", error);
			fs::remove(ownerScript, error); fs::remove(ownerScript.parent_path(), error); fs::remove(publishProject / L"Tools", error);
			fs::remove(lockPath, error); fs::remove(lockPath.parent_path(), error);
			fs::remove(publishProject / L"out" / L"BuildPipeline" / L"receipts", error);
			fs::remove(publishProject / L"out" / L"BuildPipeline", error); fs::remove(publishProject / L"out", error); fs::remove(publishProject, error);
		}
		fs::remove(path,error); fs::remove(directory,error);
	}
#endif
}

