#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "GameRoom.h"
#include "KoukuSaydonBrain.h"
#include "ServerApp.h"
#include "ClientSession.h"
#include "WorldDestructionBootstrapContractTests.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
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
		S2C_WORLD_SNAPSHOT snapshot{}; snapshot.iServerTick = 10u; snapshot.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
		snapshot.ActiveGameplayRevision.Bytes.front() = 1u;
		PLAYER_SNAPSHOT player{}; player.iNetEntityId = 1u; player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		snapshot.Players.push_back(player);
		WORLD_ENTITY_SNAPSHOT boss{}; boss.iNetEntityId = 2u; boss.eAction = WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		boss.strPatternId = "parent.pattern"; boss.strActionId = "parent.action";
		boss.iPatternSequence = 3u; boss.iPatternStartTick = 2u; boss.iActionStartTick = 2u;
		boss.strPresentationPatternId = "child.pattern"; boss.strPresentationActionId = "child.breath";
		boss.iPresentationPatternStartTick = 4u; boss.iPresentationActionStartTick = 8u; boss.iPresentationPatternStageIndex = 1u;
		boss.hasBossCombatState = true; boss.BossCombat.iStateRevision = 1u; boss.PinnedDefinitionRevision = snapshot.ActiveGameplayRevision;
		snapshot.Entities.push_back(boss); CPacketWriter writer;
		const bool written = Write_Message(writer, snapshot); CPacketReader reader{writer.Get_Buffer()}; S2C_WORLD_SNAPSHOT decoded{};
		tests.Require(written && Read_Message(reader, decoded) && decoded.Entities.size() == 1u &&
			decoded.Entities[0].strPatternId == "parent.pattern" && decoded.Entities[0].strPresentationPatternId == "child.pattern" &&
			decoded.Entities[0].strPresentationActionId == "child.breath" && decoded.Entities[0].iPresentationPatternStartTick == 4u &&
			decoded.Entities[0].iPresentationActionStartTick == 8u && decoded.Entities[0].iPresentationPatternStageIndex == 1u,
			"Cross direction snapshot preserves parent and independent child clocks");
		snapshot.Entities[0].iPresentationPatternStartTick = 0u; CPacketWriter invalid;
		tests.Require(!Write_Message(invalid, snapshot) && invalid.Get_Buffer().empty(), "Cross direction snapshot rejects incomplete child before writing");
	}
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
		const std::string encoreId = "KAKULSAYDON_G3_ENCORE_REUSE_CONTRACT";
		for (const bool layout : {false, true})
		{
			const std::string id = encoreId + (layout ? "_LAYOUT" : "");
			const auto begin = bytes.size();
			appendPattern(id, "BOSS_KAKULSAYDON_G3_SAYDON", "boss.kakulsaydon.g3.saydon", 1000u);
			const auto gate = bytes.find("GATE2", begin); bytes.replace(gate, 5u, "GATE3");
			if (layout) bytes += "PATTERNWORLDSEQUENCE\t" + encounter + "\t" + id +
				"\t0\tworld.encore.layout\t1\t0\t0\t0\tNONE\t0\t0\t0\t1000\tencore.layout\n";
		}
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
		const std::string randomTriggerId = "KAKULSAYDON_G1_RANDOM_TARGET_TRIGGER_CONTRACT";
        const std::string randomPresentationId = randomTriggerId + "_PRESENTATION";
        appendPattern(randomPresentationId, "BOSS_KAKULSAYDON_G2_BIG_SAYDON", "boss.kakulsaydon.g2.big-saydon", 1000u);
        for (unsigned index = 0u; index < 3u; ++index)
            bytes += "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + randomPresentationId + "\trandom.presentation." +
                std::to_string(index) + "\tBOSS_RANDOM_TARGET_PRESENTATION\t" + std::to_string(index * 200u) +
                "\t150\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0\n";
		appendPattern(randomTriggerId, "BOSS_KAKULSAYDON_G2_BIG_SAYDON", "boss.kakulsaydon.g2.big-saydon", 1000u);
		for (unsigned index = 0u; index < 3u; ++index)
			bytes += "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + randomTriggerId + "\trandom.target." +
				std::to_string(index) + "\tBOSS_RANDOM_TARGET\t" + std::to_string(index * 200u) +
				"\t34\t0\t0\t0\t0\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0\n";
		const std::string rootId = "KAKULSAYDON_G1_ROOT_MOTION_CONTRACT";
		for (const bool legacy : { false, true })
		{
			const auto id = rootId + (legacy ? "_LEGACY" : "");
			appendPattern(id, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 3000u);
			bytes += "PATTERNSTAGEROOTMOTION\t" + encounter + "\t" + id + "\t0\t4\t" +
				(legacy ? "0:0:0,1000:0:0,2000:0.6:0.3,3000:0:0\n" :
				 "0:0:0:0,1000:0:0:0,2000:0.6:0.3:1.25,3000:0:0:0\n");
		}
		const std::string rootChainId = rootId + "_CHAIN";
		appendPattern(rootChainId, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 1500u);
		const auto chainHeader = bytes.find("PATTERN\t" + encounter + "\t" + rootChainId + "\t");
		const auto chainStageCount = bytes.find("\t1\t1\tANY\tANY\t0\n", chainHeader);
		bytes.replace(chainStageCount, std::string("\t1\t1\tANY\tANY\t0\n").size(), "\t1\t2\tANY\tANY\t0\n");
		const auto chainBranch = bytes.find(rootChainId + ".stage.1\tTIMEOUT\t-\n", chainHeader);
		bytes.replace(chainBranch, (rootChainId + ".stage.1\tTIMEOUT\t-\n").size(),
			rootChainId + ".stage.1\tTIMEOUT\t" + rootChainId + ".stage.2\n");
		bytes += "PATTERNSTAGE\t" + encounter + "\t" + rootChainId + "\t1\tSTAGE_2\t" + rootChainId + ".stage.2\tACTIVE\t1500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n";
		bytes += "PATTERNSTAGEBRANCH\t" + encounter + "\t" + rootChainId + "\t" + rootChainId + ".stage.2\tTIMEOUT\t-\n";
		bytes += "PATTERNSTAGEROOTMOTION\t" + encounter + "\t" + rootChainId + "\t0\t2\t0:0:0:0,1500:0.2:0:0.3\n";
		bytes += "PATTERNSTAGEROOTMOTION\t" + encounter + "\t" + rootChainId + "\t1\t3\t0:0:0:0,750:0:0.05:0.5,1500:0:0.1:0\n";
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
		const std::string crossParentId = "KAKULSAYDON_G1_CROSS_PARENT";
		appendPattern(crossParentId, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 1000u);
        const auto resetRow = "PATTERNSPAWNRESET\t" + encounter + "\t" + crossParentId + "\t1\n";
        bytes.erase(bytes.find(resetRow), resetRow.size());
		bytes += "PATTERNFIXEDTIMELINE\t" + encounter + "\t" + crossParentId + "\n";
		std::array<std::string, 4u> crossIds{};
		for (unsigned direction = 0u; direction < 4u; ++direction)
		{
			const auto id = crossIds[direction] = "KAKULSAYDON_G1_CROSS_CHILD_" + std::to_string(direction);
			const auto begin = bytes.size();
			appendPattern(id, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", 100u);
			const auto reset = bytes.find("PATTERNSPAWNRESET\t" + encounter + "\t" + id + "\t1\n", begin);
			bytes.erase(reset);
			const auto count = bytes.find("\t1\t1\tANY\tANY\t0\n", begin);
			bytes.replace(count, std::string("\t1\t1\tANY\tANY\t0\n").size(), "\t1\t2\tANY\tANY\t0\n");
			const auto policy = bytes.find("\tNORMAL\t1\t1\tNONE\tNONE\n", begin);
			bytes.replace(policy, std::string("\tNORMAL\t1\t1\tNONE\tNONE\n").size(), "\tMECHANIC\t1\t1\tNONE\tNONE\n");
			const auto branch = bytes.find(id + ".stage.1\tTIMEOUT\t-\n", begin);
			bytes.replace(branch, (id + ".stage.1\tTIMEOUT\t-\n").size(), id + ".stage.1\tTIMEOUT\t" + id + ".stage.2\n");
			bytes += "PATTERNSTAGE\t" + encounter + "\t" + id + "\t1\tSTAGE_2\t" + id + ".stage.2\tACTIVE\t100\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n";
			bytes += "PATTERNSTAGEBRANCH\t" + encounter + "\t" + id + "\t" + id + ".stage.2\tTIMEOUT\t-\n";
			bytes += "PATTERNFIXEDTIMELINE\t" + encounter + "\t" + id + "\n";
			const std::array<std::string, 4u> endpoints{ "0:1", "0:-1", "1:0", "-1:0" };
			bytes += "PATTERNSTAGEROOTMOTION\t" + encounter + "\t" + id + "\t0\t2\t0:0:0:0,100:" + endpoints[direction] + ":0\n";
		}
        bytes += "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + crossIds[0] +
            "\tfront.nearest\tBOSS_TRACK_TARGET\t34\t34\t0\t0\t0\t0\t-\t0\t0\t0\t1\n";
		bytes += "PATTERNCROSSDIRECTION\t" + encounter + "\t" + crossParentId + "\tcross.logic\t0\t200\t" +
			crossIds[0] + "\t" + crossIds[1] + "\t" + crossIds[2] + "\t" + crossIds[3] + "\tSTAGE_1\n";
		bytes += "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + crossParentId + "\tparent.later.hud\tHUD_ENTER\t300\t300\t3\t0\t0\t0\t-\t0\t0\t0\t1\n";
		const auto appendContactFixture = [&](const std::string& id, const bool miss, const bool lastTick)
		{
			appendPattern(id, "BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", miss || lastTick ? 500u : 1000u);
			if (miss || lastTick) bytes += "PATTERNTIMELINE\t" + encounter + "\t" + id + "\t1000\n";
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
			const auto* crossParent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, crossParentId, status);
			tests.Require(crossParent && crossParent->MechanicTriggers.size() == 2u, "Cross direction catalog admits typed candidates");
			if (crossParent && crossParent->MechanicTriggers.size() == 2u)
			{
				const auto& trigger = crossParent->MechanicTriggers.front();
				for (unsigned expected = 0u; expected < 4u; ++expected)
				{
					SERVER_WORLD_ENTITY owner{};
					owner.fPositionX = expected == 0u ? -10.f : expected == 1u ? 10.f : 0.f;
					owner.fPositionZ = expected == 2u ? -10.f : expected == 3u ? 10.f : 0.f;
					std::size_t selected = 99u; std::array<std::uint32_t, 4u> durations{};
					tests.Require(CKoukuSaydonBrain::Select_CrossDirection(owner, *crossParent, trigger, *generation, selected, durations, status) &&
						selected == expected && std::all_of(durations.begin(), durations.end(), [](auto value) { return value == 100u; }),
						"Cross direction chooses actual root endpoint nearest center and cuts clones at Stage 1");
				}
				SERVER_WORLD_ENTITY rotated{}; rotated.fYawDegrees = 90.f; rotated.fPositionZ = 10.f;
				std::size_t selected = 99u; std::array<std::uint32_t, 4u> durations{};
				tests.Require(CKoukuSaydonBrain::Select_CrossDirection(rotated, *crossParent, trigger, *generation, selected, durations, status) && selected == 0u,
					"Cross direction rotates root endpoints with the current boss yaw");
				rotated.fPositionZ = 0.f;
				tests.Require(CKoukuSaydonBrain::Select_CrossDirection(rotated, *crossParent, trigger, *generation, selected, durations, status) && selected == 0u,
					"Cross direction center tie retains authored direction order");
                auto fixed = trigger; fixed.strRealPatternId = crossIds[2];
                for (float x : {-10.f, 0.f, 10.f})
                {
                    rotated.fPositionX = x;
                    tests.Require(CKoukuSaydonBrain::Select_CrossDirection(rotated, *crossParent, fixed, *generation, selected, durations, status) && selected == 2u,
                        "Authored real Pattern keeps the left body independent of the boss position and center tie");
                }
                fixed.strRealPatternId = "not.a.direction"; selected = 77u;
                tests.Require(!CKoukuSaydonBrain::Select_CrossDirection(rotated, *crossParent, fixed, *generation, selected, durations, status) && selected == 77u,
                    "An invalid fixed real Pattern fails without committing a selection");
                for (const auto& real : {crossIds[2], std::string("not.a.direction")})
                {
                    auto fixedBytes = bytes;
                    const std::string end = crossIds[3] + "\tSTAGE_1\n";
                    fixedBytes.replace(fixedBytes.find(end), end.size(), crossIds[3] + "\tSTAGE_1\t" + real + "\n");
                    { std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(fixedBytes.data(), static_cast<std::streamsize>(fixedBytes.size())); }
                    GameplayDataRevision fixedRevision; CGameplayCatalog fixedCatalog;
                    const bool admitted = CServerApp::Hash_GameplayFileForAdmission(path, fixedRevision, status) && fixedCatalog.Load_FromBootstrap(fs::canonical(path), fixedRevision, fixedRevision);
                    tests.Require(admitted == (real == crossIds[2]), "Bootstrap admits only fixed real Patterns named among the four candidates");
                    if (admitted)
                    {
                        const auto* fixedParent = CKoukuSaydonBrain::Find_AnimationOnlyPattern(fixedCatalog, crossParentId, status);
                        tests.Require(fixedParent && fixedParent->MechanicTriggers.front().strRealPatternId == real,
                            "Bootstrap preserves the authored real Pattern through catalog admission");
                    }
                }
                { std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); }
				auto invalid = trigger; invalid.strCloneEndStageId = "STAGE_2"; selected = 77u;
				tests.Require(!CKoukuSaydonBrain::Select_CrossDirection(rotated, *crossParent, invalid, *generation, selected, durations, status) && selected == 77u,
					"Cross direction rejects terminal clone cutoff without changing selection");
			}
			for (const std::string malformedSample : { "2000:0.6:0.3:", "2000:0.6:0.3:NaN", "2000:0.6:0.3:1.25:9" })
			{
				auto malformed = bytes;
				const std::string validSample = "2000:0.6:0.3:1.25";
				malformed.replace(malformed.find(validSample), validSample.size(), malformedSample);
				{ std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(malformed.data(), static_cast<std::streamsize>(malformed.size())); }
				GameplayDataRevision invalidRevision; CGameplayCatalog invalid;
				tests.Require(CServerApp::Hash_GameplayFileForAdmission(path, invalidRevision, status) &&
					!invalid.Load_FromBootstrap(fs::canonical(path), invalidRevision, invalidRevision),
					"Root bootstrap rejects missing nonfinite and extra fourth-component data before admission");
			}
			{ std::ofstream output(path, std::ios::binary | std::ios::trunc); output.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); }
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
			for (const bool layout : {false, true})
			{
				auto room = makeRoom();
				const auto* placement = room->Find_Placement("boss.kakulsaydon.bingo.saydon");
				SERVER_WORLD_ENTITY entity;
				const bool spawned = placement && room->Build_WorldEntity(*placement, room->m_iNextNetEntityId, entity);
				if (spawned) { ++room->m_iNextNetEntityId; room->m_WorldEntities.push_back(std::move(entity)); }
				auto request = requestFor(*room, 0u);
				request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				request.strBundleId.clear(); request.strPatternId = encoreId + (layout ? "_LAYOUT" : "");
				request.Scope.strGateId = "BINGO";
				request.Scope.strBossPlacementId = "boss.kakulsaydon.bingo.saydon";
				request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_BINGO_SAYDON";
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
				const auto verdict = room->Evaluate_KoukuSaydonPatternAudition(931u, request, result);
				tests.Require(spawned && verdict == (layout ? KOUKUSAYDON_PATTERN_AUDITION_RESULT::REJECTED_UNSUPPORTED_PATTERN :
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED),
					"Encore reuses a Gate 3 attack but rejects Gate-specific world geometry");
				if (!layout && verdict == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED)
				{
					tick(*room);
					const auto* boss = room->Find_KoukuSaydonArenaBoss("boss.kakulsaydon.bingo.saydon", "BOSS_KAKULSAYDON_BINGO_SAYDON");
					tests.Require(boss && boss->strPatternId == encoreId && boss->iPatternSequence != 0u,
						"Encore Server snapshot retains the exact original Gate 3 pattern identity");
				}
			}

			{
				auto room = makeRoom(); auto request = requestFor(*room, 0u);
				request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				request.strBundleId.clear(); request.strPatternId = crossParentId;
				request.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku";
				request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
				const bool queued = room->Evaluate_KoukuSaydonPatternAudition(917u, request, result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
				tick(*room); auto* boss = getBoss(*room, false);
				const auto sequence = boss ? boss->iPatternSequence : 0u;
				const auto start = boss ? boss->iPatternStartTick : 0u;
				const auto clones = [&]() { return std::count_if(room->m_WorldEntities.begin(), room->m_WorldEntities.end(), [](const auto& entity) { return entity.bKoukuSummonClone; }); };
				tests.Require(queued && boss && boss->KoukuDirectionPlayback && clones() == 3 && boss->strPatternId == crossParentId,
					"Cross direction commits three clones and one child on the original parent identity");
				const std::array<float, 3u> crossOrigin = boss ? std::array<float, 3u>{boss->fPositionX, boss->fPositionY, boss->fPositionZ} : std::array<float, 3u>{};
				const float crossYaw = boss ? boss->fYawDegrees * .01745329251994329577f : 0.f;
				for (unsigned index = 0u; index < 2u; ++index) tick(*room);
				boss = getBoss(*room, false);
				unsigned verifiedPoses = 0u, verifiedEndpoints = 0u;
				for (const auto& entity : room->m_WorldEntities)
				{
					const auto* actor = entity.bKoukuSummonClone ? &entity : entity.KoukuDirectionPlayback.get();
					if (!actor) continue;
					const auto* childPattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, actor->strPatternId, status);
					if (!childPattern) continue;
					const auto root = CKoukuSaydonBrain::Sample_StageRootMotion(childPattern->Stages.front().Motion.RootMotion,
						CKoukuSaydonBrain::Pattern_ElapsedMs(*actor, room->m_iServerTick));
					const auto at = [&](const ROOT_MOTION_SAMPLE& value) { return std::array<float, 2u>{
						crossOrigin[0] + value.fLateral * std::cos(crossYaw) + value.fForward * std::sin(crossYaw),
						crossOrigin[2] - value.fLateral * std::sin(crossYaw) + value.fForward * std::cos(crossYaw)}; };
					const auto expected = at(root);
					if (std::abs(actor->fPositionX - expected[0]) < .002f && std::abs(actor->fPositionZ - expected[1]) < .002f) ++verifiedPoses;
					auto terminal = *actor;
					const auto endpoint = at(childPattern->Stages.front().Motion.RootMotion.back());
					if (CKoukuSaydonBrain::Apply_StageRootMotion(terminal, *childPattern, actor->iPatternStartTick + 3u,
						room->m_ServerNavigation, room->m_ServerCollisionSystem, status) &&
						std::abs(terminal.fPositionX - endpoint[0]) < .002f && std::abs(terminal.fPositionZ - endpoint[1]) < .002f) ++verifiedEndpoints;
				}
				tests.Require(verifiedPoses == 4u && verifiedEndpoints == 4u,
					"All four actual bodies move on their root curves and can reach exact directional endpoints without same-origin collision suppression");
				for (unsigned index = 0u; index < 2u; ++index) tick(*room);
				boss = getBoss(*room, false);
				tests.Require(boss && boss->KoukuDirectionPlayback && clones() == 0 &&
					boss->KoukuDirectionPlayback->iPatternStageIndex == 1u && boss->iPatternSequence == sequence && boss->iPatternStartTick == start,
					"At Stage 1 cutoff all fake bodies disappear while only the original boss plays breath");
				tests.Require(boss && std::abs(std::hypot(boss->fPositionX - crossOrigin[0], boss->fPositionZ - crossOrigin[2]) - 1.f) < .002f,
					"Original boss reaches the full selected directional endpoint before breath");
				for (unsigned index = 0u; index < 7u; ++index) tick(*room);
				boss = getBoss(*room, false); const auto* member = boss ? room->Find_KoukuAuditionMember(boss->iNetEntityId) : nullptr;
				tests.Require(boss && !boss->KoukuDirectionPlayback && boss->strPatternId == crossParentId && member &&
					member->LogicLedger.eHudMode == KOUKU_HUD_MODE::DANCE && boss->iPatternSequence == sequence && boss->iPatternStartTick == start,
					"Parent continues its original clock and later Logic after the selected child ends");
				for (unsigned index = 0u; index < 22u; ++index) tick(*room);
				boss = getBoss(*room, false);
				tests.Require(boss && boss->strPatternId.empty() && !boss->KoukuDirectionPlayback && clones() == 0,
					"Parent completion releases child and clone lifetime state");
			}
            for (const bool frontIsReal : {true, false})
            {
                auto room = makeRoom(); auto* owner = getBoss(*room, false);
                if (!owner) continue;
                owner->fPositionX = owner->fSpawnPositionX + (frontIsReal ? -10.f : 10.f);
                owner->fPositionZ = owner->fSpawnPositionZ; owner->fYawDegrees = 0.f;
                auto request = requestFor(*room, 0u);
                request.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
                request.strBundleId.clear(); request.strPatternId = crossParentId;
                request.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku";
                request.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
                S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
                const bool queued = room->Evaluate_KoukuSaydonPatternAudition(917u, request, result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
                tick(*room); tick(*room);
                const auto front = [&]() -> const SERVER_WORLD_ENTITY* {
                    for (auto& body : room->m_WorldEntities)
                    {
                        auto* actor = body.bKoukuSummonClone ? &body : body.KoukuDirectionPlayback.get();
                        if (actor && actor->strPatternId == crossIds[0]) return actor;
                    }
                    return nullptr;
                };
                auto* actor = front(); owner = getBoss(*room, false);
                tests.Require(queued && actor && owner && (actor == owner->KoukuDirectionPlayback.get()) == frontIsReal && actor->fYawDegrees == 0.f,
                    "Front-only facing fixture retains either real or clone actor and original yaw before its attack tick");
                if (!actor) continue;
                const auto actorId = actor->iNetEntityId;
                for (unsigned id = 1u; id <= 5u; ++id)
                {
                    auto& player = room->m_Players[id]; player.iPlayerId = id; player.iSessionId = 700u + id;
                    player.iNetEntityId = 200u - id; player.isCombatReady = true;
                    player.iCurrentHp = player.iMaximumHp = 100u; player.iCurrentResource = player.iMaximumResource = 100u;
                    player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
                    player.fPositionX = actor->fPositionX + (id <= 2u ? 4.f : 0.f);
                    player.fPositionY = actor->fPositionY; player.fPositionZ = actor->fPositionZ + (id <= 2u ? 8.f : 0.f);
                }
                room->m_Players.at(3u).iCurrentHp = 0u;
                room->m_Players.at(4u).eAction = PLAYER_ACTION_STATE::GRABBED;
                room->m_Players.at(5u).isCombatReady = false;
                tick(*room); actor = front();
                const auto& target = room->m_Players.at(2u);
                const float expectedYaw = actor ? std::atan2(target.fPositionX - actor->fPositionX,
                    target.fPositionZ - actor->fPositionZ) * 57.295779513f : 0.f;
                tests.Require(actor && actor->iPatternTargetEntityId == target.iNetEntityId && std::abs(actor->fYawDegrees - expectedYaw) < .001f &&
                    actor->KoukuContactLedger && actor->KoukuContactLedger->PlayerTargetWindows.front().bClosed,
                    "Actual cross body targets nearest living ready player once at attack start with stable entity-ID tie breaking");
                unsigned unchanged = 0u;
                for (const auto& body : room->m_WorldEntities)
                {
                    const auto* sibling = body.bKoukuSummonClone ? &body : body.KoukuDirectionPlayback.get();
                    if (sibling && sibling->strPatternId != crossIds[0] && sibling->fYawDegrees == 0.f) ++unchanged;
                }
                tests.Require(unchanged == 3u, "Front actor facing leaves all three other directional actor yaws unchanged");
                if (!actor) continue;
                const auto* child = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, actor->strPatternId, status);
                const float heldYaw = actor->fYawDegrees;
                room->m_Players.at(2u).fPositionZ -= 30.f;
                auto repeatedActor = *actor;
                room->Update_KoukuActorContacts(repeatedActor, *child, *generation, room->m_iServerTick + 1u);
                tests.Require(repeatedActor.fYawDegrees == heldYaw, "Moving a target after the instantaneous child Trigger never retargets that attack");
                auto session = std::make_shared<CClientSession>(701u, INVALID_SOCKET, CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
                session->m_isSendRunning.store(true); room->m_Sessions.emplace(701u, session); room->m_PlayerIdBySessionId[701u] = 1u;
                auto savedPlayers = room->m_Players;
                room->m_Players.erase(3u); room->m_Players.erase(4u); room->m_Players.erase(5u);
                room->Broadcast_WorldSnapshot(); room->m_Players = std::move(savedPlayers); bool delivered = false;
                for (const auto& frame : session->m_OutboundFrames)
                {
                    if (frame.ePacketType != PACKET_TYPE::S2C_WORLD_SNAPSHOT) continue;
                    CPacketReader reader(std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES));
                    S2C_WORLD_SNAPSHOT snapshot;
                    if (!Read_Message(reader, snapshot)) continue;
                    const auto body = std::find_if(snapshot.Entities.begin(), snapshot.Entities.end(), [&](const auto& value) { return value.iNetEntityId == actorId; });
                    delivered = body != snapshot.Entities.end() && body->fYawDegrees == heldYaw &&
                        (frontIsReal ? body->strPresentationPatternId == crossIds[0] : body->strPatternId == crossIds[0]);
                }
                tests.Require(delivered, "Real snapshot serialization sends the exact front real-or-clone yaw on its existing entity and child clock");
                // A fresh actor samples an empty eligible raid roster once and preserves its pose.
                auto emptyActor = *actor; emptyActor.KoukuContactLedger.reset(); emptyActor.fYawDegrees = 17.f;
                room->m_KoukuRaid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT; room->m_KoukuRaid.PlayerIds = {3u};
                room->Update_KoukuActorContacts(emptyActor, *child, *generation, room->m_iServerTick);
                tests.Require(emptyActor.fYawDegrees == 17.f && emptyActor.iPatternTargetEntityId == INVALID_NET_ENTITY_ID &&
                    emptyActor.KoukuContactLedger->PlayerTargetWindows.front().bClosed,
                    "Instant actor facing excludes dead and nonparticipating raid players without random fallback or delayed retry");
                room->m_KoukuRaid.State.ePhase = KOUKUSAYDON_RAID_PHASE::INACTIVE;
                tick(*room);
                tests.Require(frontIsReal || !front(), "A clone already at its authored cutoff is removed without deferred facing or resurrection");
            }
			{
				const auto* root = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, rootId, status);
				const auto* legacy = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, rootId + "_LEGACY", status);
				const auto close = [](const float a, const float b) { return std::abs(a - b) < .001f; };
				tests.Require(root && legacy && root->Stages.front().Motion.RootMotion[2u].fUp == 1.25f &&
					legacy->Stages.front().Motion.RootMotion[2u].fUp == 0.f,
					"Stage root motion admits full XYZ and preserves legacy three-component samples");
				if (root)
				{
					const auto& curve = root->Stages.front().Motion.RootMotion;
					const auto delay = CKoukuSaydonBrain::Sample_StageRootMotion(curve, 999.0);
					const auto middle = CKoukuSaydonBrain::Sample_StageRootMotion(curve, 1500.0);
					const auto terminal = CKoukuSaydonBrain::Sample_StageRootMotion(curve, 4000.0);
					tests.Require(delay.fForward == 0.f && close(middle.fForward, .3f) &&
						close(middle.fLateral, .15f) && close(middle.fUp, .625f) &&
						terminal.fForward == 0.f && terminal.fLateral == 0.f && terminal.fUp == 0.f,
						"Cumulative root sampling retains delayed XYZ travel and a moving curve whose endpoint returns to zero");
					for (unsigned invalidKind = 0u; invalidKind < 5u; ++invalidKind)
					{
						auto invalid = *root;
						if (invalidKind == 0u) invalid.Stages.front().Motion.RootMotion.front().fUp = 1.f;
						if (invalidKind == 1u) invalid.Stages.front().Motion.RootMotion.back().iTimeMs = 2999u;
						if (invalidKind == 2u) invalid.Stages.front().Motion.RootMotion[2u].fUp = (std::numeric_limits<float>::infinity)();
						if (invalidKind == 3u) invalid.BossMotion.emplace();
						if (invalidKind == 4u) { invalid.LogicWindows.emplace_back(); invalid.LogicWindows.back().fBossChargeDistanceM = 1.f; }
						tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status),
							"Root admission rejects invalid curves and a second authored movement authority");
					}
					auto rootRoom = makeRoom(); auto rootRequest = requestFor(*rootRoom, 0u);
					rootRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
					rootRequest.strBundleId.clear(); rootRequest.strPatternId = rootId;
					rootRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku";
					rootRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
					auto* moving = getBoss(*rootRoom, false);
					moving->fYawDegrees = 90.f;
					const std::array<float, 3u> origin{moving->fSpawnPositionX, moving->fSpawnPositionY, moving->fSpawnPositionZ};
					// The pre-admission pose differs: the curve must capture the committed spawn reset.
					moving->fPositionX += 1.f;
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT rootResult;
					const bool queued = rootRoom->Evaluate_KoukuSaydonPatternAudition(919u, rootRequest, rootResult) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
					for (unsigned index = 0u; index < 30u; ++index) tick(*rootRoom);
					tests.Require(queued && moving->bPatternStageRootOriginCaptured &&
						close(moving->fPositionX, origin[0]) && close(moving->fPositionY, origin[1]) && close(moving->fPositionZ, origin[2]),
						"Room root motion captures after spawn reset and holds through the authored delay");
					for (unsigned index = 30u; index < 45u; ++index) tick(*rootRoom);
					tests.Require(close(moving->fPositionX, origin[0] + .3f) && close(moving->fPositionY, origin[1] + .625f) &&
						close(moving->fPositionZ, origin[2] - .15f),
						"Room applies lateral forward and up once in the captured yaw basis before Logic");
					const auto beforeRepeat = *moving;
					const bool repeated = CKoukuSaydonBrain::Apply_StageRootMotion(*moving, *root, moving->iPatternStageRootLastTick,
						rootRoom->m_ServerNavigation, rootRoom->m_ServerCollisionSystem, status);
					tests.Require(repeated && moving->fPositionX == beforeRepeat.fPositionX && moving->fPositionY == beforeRepeat.fPositionY &&
						moving->fPositionZ == beforeRepeat.fPositionZ, "Repeated root evaluation on one fixed tick cannot move the actor twice");
					auto blocked = *moving;
					blocked.PatternStageRootMotion.back().fForward = 100000.f;
					blocked.iPatternStageRootLastTick = 0u;
					const bool held = CKoukuSaydonBrain::Apply_StageRootMotion(blocked, *root, 120u,
						rootRoom->m_ServerNavigation, rootRoom->m_ServerCollisionSystem, status);
					// Yaw 90 sends the terminal 100 km forward sample along +X. The boss must stop
					// flush at the last navigable point of that segment (within 1 mm of the first
					// height step) with Y from the sample plus the ground delta, not hold one tick short.
					SERVER_NAV_POINT originGround{}, clampGround{}, beyondGround{};
					const float clampRatio = (blocked.fPositionX - moving->fPositionX) / (origin[0] + 100000.f - moving->fPositionX);
					tests.Require(held && blocked.fPositionX > moving->fPositionX + .01f &&
						std::abs(blocked.fPositionZ - (moving->fPositionZ + (origin[2] - moving->fPositionZ) * clampRatio)) < .002f &&
						rootRoom->m_ServerNavigation.Is_PointWalkableExact(blocked.fPositionX, blocked.fPositionZ) &&
						!rootRoom->m_ServerNavigation.Resolve_TraversalStep(blocked.fPositionX, blocked.fPositionZ, blocked.fPositionX + .002f, blocked.fPositionZ, beyondGround) &&
						rootRoom->m_ServerNavigation.Sample_Position(origin[0], origin[2], originGround) &&
						rootRoom->m_ServerNavigation.Sample_Position(blocked.fPositionX, blocked.fPositionZ, clampGround) &&
						close(blocked.fPositionY, origin[1] + clampGround.y - originGround.y) && blocked.iPatternStageRootLastTick == 120u,
						"Navigation clamps an off-grid root destination to the last navigable point on the segment");
					const auto clampedPose = blocked;
					const bool heldAgain = CKoukuSaydonBrain::Apply_StageRootMotion(blocked, *root, 121u,
						rootRoom->m_ServerNavigation, rootRoom->m_ServerCollisionSystem, status);
					tests.Require(heldAgain && blocked.fPositionX == clampedPose.fPositionX && blocked.fPositionY == clampedPose.fPositionY &&
						blocked.fPositionZ == clampedPose.fPositionZ && blocked.iPatternStageRootLastTick == 121u,
						"A root segment blocked within 1 mm of the pose preserves the exact previous XYZ");
					auto failed = *moving; failed.fCollisionRadius = 0.f; failed.iPatternStageRootLastTick = 0u;
					const bool rejected = !CKoukuSaydonBrain::Apply_StageRootMotion(failed, *root, 46u,
						rootRoom->m_ServerNavigation, rootRoom->m_ServerCollisionSystem, status);
					tests.Require(rejected && failed.fPositionX == moving->fPositionX && failed.fPositionY == moving->fPositionY &&
						failed.fPositionZ == moving->fPositionZ, "A root collision failure preserves the complete previous transform");
					CKoukuSaydonBrain{}.Abort_Pattern(failed, 47u);
					tests.Require(failed.PatternStageRootMotion.empty() && !failed.bPatternStageRootOriginCaptured &&
						failed.iPatternStageRootLastTick == 0u, "Abort clears every active stage root origin and sampling state");
					for (unsigned index = 45u; index < 100u; ++index) tick(*rootRoom);
					tests.Require(moving->strPatternId.empty() && moving->PatternStageRootMotion.empty() &&
						!moving->bPatternStageRootOriginCaptured && close(moving->fPositionX, origin[0]) &&
						close(moving->fPositionY, origin[1]) && close(moving->fPositionZ, origin[2]),
						"Natural completion commits the return endpoint and releases stage root state without cumulative drift");
				}
			}
			{
				auto chainRoom = makeRoom(); auto chainRequest = requestFor(*chainRoom, 0u);
				chainRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				chainRequest.strBundleId.clear(); chainRequest.strPatternId = rootChainId;
				chainRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g2.kouku";
				chainRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_KOUKU";
				auto* moving = getBoss(*chainRoom, false); moving->fYawDegrees = 0.f;
				const std::array<float, 3u> origin{moving->fSpawnPositionX, moving->fSpawnPositionY, moving->fSpawnPositionZ};
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT result;
				const bool queued = chainRoom->Evaluate_KoukuSaydonPatternAudition(918u, chainRequest, result) == KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
				for (unsigned index = 0u; index < 45u; ++index) tick(*chainRoom);
				const auto close = [](float a, float b) { return std::abs(a - b) < .001f; };
				tests.Require(queued && moving->iPatternStageIndex == 1u && !moving->bPatternStageRootOriginCaptured &&
					close(moving->fPositionZ, origin[2] + .2f) && close(moving->fPositionY, origin[1] + .3f),
					"Stage transition commits the first root endpoint before resetting its curve origin");
				tick(*chainRoom);
				auto dead = *moving; dead.iCurrentHp = 0u;
				tests.Require(CKoukuSaydonBrain{}.Update(dead, *generation, chainRoom->m_iServerTick, status) ==
					KOUKUSAYDON_BRAIN_UPDATE_RESULT::ABORTED_BOSS_DEAD && dead.PatternStageRootMotion.empty() &&
					!dead.bPatternStageRootOriginCaptured && dead.iPatternStageRootLastTick == 0u,
					"Death releases a sampled stage root curve and its origin through the existing brain termination");
				for (unsigned index = 46u; index < 95u; ++index) tick(*chainRoom);
				tests.Require(moving->strPatternId.empty() && close(moving->fPositionX, origin[0] + .1f) &&
					close(moving->fPositionZ, origin[2] + .2f) && close(moving->fPositionY, origin[1] + .3f),
					"Following root stages accumulate from committed XYZ instead of reusing the previous stage origin");
			}
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
				for (unsigned i = 0u; i < 20u; ++i) tick(*deadlineRoom);
				const auto* beforeBoss = getBoss(*deadlineRoom, false);
				const bool releasedStage = beforeBoss && beforeBoss->strPatternId.empty() &&
					deadlineRoom->m_KoukuSaydonPatternAudition.Tails.size() == 1u &&
					deadlineRoom->m_KoukuSaydonPatternAudition.Members.size() == 1u &&
					deadlineRoom->m_KoukuSaydonPatternAudition.Members.front().PatternIds.size() == 1u;
				for (unsigned i = 20u; i < 31u; ++i) tick(*deadlineRoom);
				const auto& deadlineRun = deadlineRoom->m_KoukuSaydonPatternAudition;
				const auto expected = finalTickSuccess ? "KAKULSAYDON_G1_BUNDLE_A_FOLLOW" : "KAKULSAYDON_G1_BUNDLE_A";
				const bool resolvedOnce = deadlineRun.Members.size() == 1u &&
					deadlineRun.Members.front().PatternIds.size() == 2u &&
					deadlineRun.Members.front().PatternIds.back() == expected &&
					deadlineRun.WorldPlays.size() == (finalTickSuccess ? 1u : 0u);
				for (unsigned i = 0u; i < 3u; ++i) tick(*deadlineRoom);
				tests.Require(queuedDeadline && releasedStage && resolvedOnce &&
					deadlineRun.Members.front().PatternIds.size() == 2u &&
					beforeBoss && beforeBoss->strPatternId == expected, finalTickSuccess ?
					"The 500ms Stage releases before its 1000ms tail; last-tick contact starts its successor once without timeout" :
					"The 500ms Stage releases before its 1000ms tail; a missed strike starts its timeout successor exactly once");
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
			for (const bool presentationOnly : {false, true})
			{
				auto randomRoom = makeRoom(); auto randomRequest = requestFor(*randomRoom, 0u);
				randomRequest.eOperation = KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
				randomRequest.strBundleId.clear(); randomRequest.strPatternId = presentationOnly ? randomPresentationId : randomTriggerId;
				randomRequest.Scope.strBossPlacementId = "boss.kakulsaydon.g2.big-saydon";
				randomRequest.Scope.strBossArchetypeId = "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
				auto* boss = getBoss(*randomRoom, true);
				for (unsigned index = 0u; index < 4u; ++index)
				{
					SERVER_PLAYER player; player.iPlayerId = player.iNetEntityId = 970u + index;
					player.iCurrentHp = index == 2u ? 0u : 100u; player.isCombatReady = true;
					player.eAction = index == 3u ? PLAYER_ACTION_STATE::FALLING : PLAYER_ACTION_STATE::NONE;
					player.fPositionX = boss->fPositionX + (index ? -5.f : 5.f);
					player.fPositionY = boss->fPositionY; player.fPositionZ = boss->fPositionZ;
					randomRoom->m_Players.emplace(player.iPlayerId, player);
				}
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT randomResult;
				const bool admitted = randomRoom->Evaluate_KoukuSaydonPatternAudition(970u, randomRequest, randomResult) ==
					KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
				std::vector<std::uint32_t> changeTicks;
				NET_ENTITY_ID previous = INVALID_NET_ENTITY_ID;
				bool aliveTargets = true, capturedPose = true, durationYawPreserved = true;
                bool endPosePinned = true, fixedAimHeld = true;
                float initialYaw = 0.f, pinnedYaw = 0.f, pinnedX = 0.f, pinnedY = 0.f, pinnedZ = 0.f;
				for (unsigned step = 0u; step < 29u; ++step)
				{
                    if (presentationOnly && (step == 4u || step == 8u || step == 18u))
                        for (auto& [id, player] : randomRoom->m_Players) { player.fPositionX += 2.f; player.fPositionZ += 5.f; }
					tick(*randomRoom);
                    if (!step) initialYaw = boss->fYawDegrees;
					if (boss->iPatternTargetEntityId != previous)
					{ changeTicks.push_back(randomRoom->m_iServerTick); previous = boss->iPatternTargetEntityId; }
					aliveTargets = aliveTargets && (previous == 970u || previous == 971u);
					const auto target = randomRoom->m_Players.find(previous);
                    if (presentationOnly)
                    {
                        const bool inDuration = step < 5u || (step >= 6u && step < 11u) || (step >= 12u && step < 17u);
                        if (inDuration) durationYawPreserved &= std::abs(boss->fYawDegrees - initialYaw) < .0001f;
                        if (step == 5u || step == 11u || step == 17u)
                        {
                            endPosePinned &= target != randomRoom->m_Players.end();
                            if (target != randomRoom->m_Players.end())
                            {
                                pinnedX = target->second.fPositionX; pinnedY = target->second.fPositionY; pinnedZ = target->second.fPositionZ;
                                pinnedYaw = std::atan2(pinnedX - boss->fPositionX, pinnedZ - boss->fPositionZ) * 57.29577951308232f - 90.f;
                                endPosePinned &= std::abs(boss->fPatternTargetLastPositionX - pinnedX) < .0001f &&
                                    std::abs(boss->fPatternTargetLastPositionY - pinnedY) < .0001f &&
                                    std::abs(boss->fPatternTargetLastPositionZ - pinnedZ) < .0001f &&
                                    std::abs(boss->fYawDegrees - pinnedYaw) < .0001f;
                            }
                        }
                        if (!inDuration) fixedAimHeld &= std::abs(boss->fYawDegrees - pinnedYaw) < .0001f &&
                            std::abs(boss->fPatternTargetLastPositionX - pinnedX) < .0001f &&
                            std::abs(boss->fPatternTargetLastPositionY - pinnedY) < .0001f &&
                            std::abs(boss->fPatternTargetLastPositionZ - pinnedZ) < .0001f;
                    }
					capturedPose = capturedPose && target != randomRoom->m_Players.end() && boss->bHasPatternTargetLastPosition &&
						boss->iTargetEntityId == previous && (presentationOnly || boss->fPatternTargetLastPositionX == target->second.fPositionX);
				}
				tests.Require(admitted && aliveTargets && capturedPose && changeTicks == std::vector<std::uint32_t>{1u, 7u, 13u},
					"Three random target Triggers select at start and twice later, hold between ticks, and exclude dead/falling/previous targets");
                if (presentationOnly)
                {
                    tests.Require(durationYawPreserved, "Random target Duration keeps original body yaw while the eye follows its selected player");
                    tests.Require(endPosePinned, "Random target Duration captures the current player position and fixes boss aim on each authored END tick");
                    tests.Require(fixedAimHeld, "Random target hammer aim keeps the pinned END position after the selected player moves");
                    for (unsigned step = 29u; step < 32u; ++step) tick(*randomRoom);
                    tests.Require(boss->strPatternId.empty() && std::abs(boss->fYawDegrees - initialYaw) < .0001f,
                        "Random target Duration completion restores original body yaw before the next pattern");
                }
				for (auto& [id, player] : randomRoom->m_Players) if (id != previous) player.iCurrentHp = 0u;
				const auto* sole = randomRoom->Select_BossRandomAliveTarget(*boss, "solo", "boss.target.random.next", 30u);
				tests.Require(sole && sole->iNetEntityId == previous,
					"A random retarget keeps the sole living candidate instead of dropping the target");
				for (auto& [id, player] : randomRoom->m_Players) player.iCurrentHp = 0u;
				tests.Require(!randomRoom->Select_BossRandomAliveTarget(*boss, "empty", "boss.target.random.next", 31u),
					"A random target Trigger cannot select a dead room member");
				const auto* definition = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*generation, presentationOnly ? randomPresentationId : randomTriggerId, status);
				if (definition)
				{
					auto invalid = *definition; invalid.MechanicTriggers.front().fFollowSpeedScale = 1.f;
					tests.Require(!CKoukuSaydonBrain::Validate_AnimationOnlyPattern(invalid, status),
						"One-shot random target Triggers reject continuous-follow values");
				}
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
			std::string damageId;
			const CGameplayCatalog::DAMAGE_PROFILE* activeDamage = nullptr;
			std::uint32_t foreignRate = 0u;
			if (damageAt != std::string::npos)
			{
				const auto idAt = damageAt + std::string_view("\nDAMAGE\t").size();
				const auto idEnd = foreign.find('\t', idAt);
				const auto valueAt = idEnd + 1u;
				const auto valueEnd = foreign.find_first_of("\t\r\n", valueAt);
				if (idEnd != std::string::npos && valueEnd != std::string::npos)
				{
					damageId = foreign.substr(idAt, idEnd - idAt);
					activeDamage = generation->Find_DamageProfile(damageId);
					if (activeDamage)
					{
						// The rate is field 2; Retail appends coefficient, addend and spread.
						foreignRate = activeDamage->iRatePercent == 99999u ? 99998u : 99999u;
						foreign.replace(valueAt, valueEnd - valueAt, std::to_string(foreignRate));
					}
				}
			}
			publish(foreign); play.iRequestSequence = 9u;
			CGameplayCatalog foreignCatalog;
			const bool foreignLoaded = foreignCatalog.Load();
			const auto* diskDamage = foreignLoaded ? foreignCatalog.Find_DamageProfile(damageId) : nullptr;
			tests.Require(activeDamage && diskDamage && diskDamage->iRatePercent == foreignRate &&
				diskDamage->iRatePercent != activeDamage->iRatePercent &&
				diskDamage->iAttackCoefficientBp == activeDamage->iAttackCoefficientBp &&
				diskDamage->iDamageAddend == activeDamage->iDamageAddend &&
				diskDamage->iDamageSpreadPercent == activeDamage->iDamageSpreadPercent,
				"Unrelated disk damage edit changes only the valid rate field while preserving Retail formula and spread");
			const bool foreignAdmitted = reloadRoom->Evaluate_KoukuSaydonPatternAudition(950u, play, result) ==
				KOUKUSAYDON_PATTERN_AUDITION_RESULT::QUEUED;
			const auto foreignPin = reloadRoom->m_KoukuSaydonPatternAudition.pProductGeneration;
			const auto* pinnedDamage = foreignPin ? foreignPin->Find_DamageProfile(damageId) : nullptr;
			if (!foreignAdmitted) std::cout << "[STATUS] Kouku unrelated damage reload: " << result.strReason << '\n';
			tests.Require(foreignAdmitted && activeDamage && pinnedDamage &&
				pinnedDamage->iRatePercent == activeDamage->iRatePercent &&
				pinnedDamage->iAttackCoefficientBp == activeDamage->iAttackCoefficientBp &&
				pinnedDamage->iDamageAddend == activeDamage->iDamageAddend &&
				pinnedDamage->iDamageSpreadPercent == activeDamage->iDamageSpreadPercent &&
				result.iPinnedSourceRevision == oldSource + 2u &&
				foreignPin->Has_SameNonKoukuGameplay(*generation) &&
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

