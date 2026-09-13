#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "GameplayCatalog.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_SkillStages(TESTS& tests, CGameplayCatalog& catalog)
{


	{
		/* A skill with no damage profile never resolves a hit, so the hit time and
		reach that only describe that hit must be zero. Accepting a reach here would
		let a movement skill silently keep a damage window. */
		namespace fs = std::filesystem;
		const fs::path noDamageRoot =
			fs::temp_directory_path() / L"LostArkNoDamageContractTest";
		const auto loadWithMovementSkill =
			[&noDamageRoot](
				const char* hitTimeMs,
				const char* maximumRange,
				const TEST_TIMELINE_VARIANT timelineVariant)
		{
			std::error_code prepareError;
			fs::remove_all(noDamageRoot, prepareError);
			fs::create_directories(noDamageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					noDamageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
					'\t' << (17u + Count_TestTimelineRows(timelineVariant)) << '\n' <<
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020"
					"\t8000\t900\t" << hitTimeMs << "\t242\t0\t6\t" << maximumRange <<
					"\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				Write_TestValtanTimelineRows(bootstrap, timelineVariant);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", noDamageRoot.c_str());
			CGameplayCatalog catalog;
			const bool loaded = catalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::VALID),
			"Accept a skill that carries no damage profile");
		tests.Require(!loadWithMovementSkill(
			"0", "3", TEST_TIMELINE_VARIANT::VALID),
			"Reject a damageless skill that still claims reach");
		tests.Require(!loadWithMovementSkill(
			"400", "0", TEST_TIMELINE_VARIANT::VALID),
			"Reject a damageless skill that still claims a hit time");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::MISSING_ROW),
			"Reject a timeline whose declared occurrence row is missing");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::MISSING_ACTION),
			"Reject a timeline occurrence whose pattern action is missing");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::OVERSIZED_ACTION_INDEX),
			"Reject an oversized timeline action index before staging storage");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::COMMAND_HASH_MISMATCH),
			"Reject a timeline command id that does not match its semantic row id");
		tests.Require(!loadWithMovementSkill(
			"0", "0", TEST_TIMELINE_VARIANT::COMMAND_HASH_COLLISION),
			"Reject distinct timeline row ids whose stable command ids collide");
		std::error_code noDamageCleanupError;
		fs::remove_all(noDamageRoot, noDamageCleanupError);
	}

	{
		/* Wall-contact admission is separate from player damage admission: a
		   malformed or duplicated allowlist row must fail before a room can use
		   the hit as an axe collider. */
		namespace fs = std::filesystem;
		const fs::path wallContactRoot =
			fs::temp_directory_path() / L"LostArkWallContactCatalogContractTest";
		const auto loadWithWallContactRows = [
			&wallContactRoot](
				const std::uint32_t version,
				const std::vector<std::string>& sourceRows,
				const std::vector<std::string>& wallRows)
		{
			std::error_code prepareError;
			fs::remove_all(wallContactRoot, prepareError);
			fs::create_directories(wallContactRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					wallContactRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap << "LOSTARK_GAMEPLAY_BOOTSTRAP\t" << version << "\t" <<
					(16u + VALID_VALTAN_TIMELINE_ROW_COUNT +
					 sourceRows.size() + wallRows.size()) << "\n"
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020\t8000\t900\t0\t242\t0\t6\t0\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				for (const std::string& row : sourceRows)
					bootstrap << row << '\n';
				for (const std::string& row : wallRows)
					bootstrap << row << '\n';
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", wallContactRoot.c_str());
			CGameplayCatalog wallCatalog;
			const bool loaded = wallCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		const std::string validWallRow =
			"PATTERNWALLCONTACT\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active";
		const std::string validSourceRow =
			"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180";
		tests.Require(
			loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION, { validSourceRow }, { validWallRow }),
			"Accept one exact ACTIVE axe wall-contact row");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION - 1u,
				{ validSourceRow }, { validWallRow }),
			"Reject the obsolete gameplay bootstrap version before wall-contact load");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow }, {
				"PATTERNWALLCONTACT\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.wrong" }),
			"Reject a wall-contact row whose action does not exactly join its stage");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow }, { validWallRow, validWallRow }),
			"Reject duplicate axe wall-contact ownership atomically");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION, {}, {}),
			"Reject a boss pattern with no compiled source timing row");
		tests.Require(
			!loadWithWallContactRows(
				GAMEPLAY_BOOTSTRAP_VERSION,
				{ validSourceRow, validSourceRow }, {}),
			"Reject duplicate source timing ownership atomically");
		tests.Require(
			!loadWithWallContactRows(GAMEPLAY_BOOTSTRAP_VERSION, {
				"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t149\t350\t300\t180" }, {}),
			"Reject a source cooldown whose 30 Hz tick conversion is inconsistent");

		const auto loadWithPatternStageRows = [&wallContactRoot](
			const std::string& stageRows,
			const std::vector<std::string>& hitOffsetRows)
		{
			std::error_code prepareError;
			fs::remove_all(wallContactRoot, prepareError);
			fs::create_directories(wallContactRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					wallContactRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap << "LOSTARK_GAMEPLAY_BOOTSTRAP\t" <<
					GAMEPLAY_BOOTSTRAP_VERSION << "\t" <<
					(17u + VALID_VALTAN_TIMELINE_ROW_COUNT +
						hitOffsetRows.size()) << "\n"
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34120\t361\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					<< stageRows;
				for (const std::string& hitOffsetRow : hitOffsetRows)
					bootstrap << hitOffsetRow << '\n';
				bootstrap <<
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34020\tLANCE_MASTER\tSPACE\tlancemaster.skill.34020\t8000\t900\t0\t242\t0\t6\t0\t\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34020\t0\t0\t0\n";
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", wallContactRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		const std::string acceptedContactDelayRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t600\tdamage.player.34120\t2\t242\t1\t2000\n"
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
			"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
			"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const bool acceptedContactDelay =
			loadWithPatternStageRows(acceptedContactDelayRows, {});
		tests.Require(
			acceptedContactDelay,
			"Accept a stage whose first hit lands at its authored contact delay");
		std::string invalidLegacySpawnCountRows = acceptedContactDelayRows;
		const std::string validLegacySpawnSuffix =
			"SPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const std::size_t legacySpawnSuffixAt =
			invalidLegacySpawnCountRows.find(validLegacySpawnSuffix);
		if (std::string::npos != legacySpawnSuffixAt)
		{
			invalidLegacySpawnCountRows.replace(
				legacySpawnSuffixAt, validLegacySpawnSuffix.size(),
				"SPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t2\t0\n");
		}
		tests.Require(
			std::string::npos != legacySpawnSuffixAt &&
			!loadWithPatternStageRows(invalidLegacySpawnCountRows, {}),
			"Reject legacy combat-object action counts above one");
		tests.Require(
			!loadWithPatternStageRows(
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t1000\tdamage.player.34120\t2\t242\t1\t2000\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n",
				{}),
			"Reject a hit delay at or beyond its stage duration");
		tests.Require(
			!loadWithPatternStageRows(
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tWINDUP\tvaltan.test.active\tWINDUP\t1000\tNONE\t0\t0\t0\t0\t0\t0\t0\t600\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n",
				{}),
			"Reject a hit delay on a stage without a hit shape");

		const std::string explicitHitStageRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t3\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
			"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
			"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
			"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
			"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n";
		const std::vector<std::string> validExplicitHitOffsets{
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t0\t100",
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t1\t450",
			"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t2\t900"
		};
		tests.Require(
			loadWithPatternStageRows(
				explicitHitStageRows, validExplicitHitOffsets),
			"Accept one complete ordered explicit boss hit schedule");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				validExplicitHitOffsets[0], validExplicitHitOffsets[1] }),
			"Reject an explicit boss hit schedule whose count is incomplete");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				validExplicitHitOffsets[0],
				"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\t1\t90",
				validExplicitHitOffsets[2] }),
			"Reject explicit boss hit offsets that are not strictly increasing");
		tests.Require(
			!loadWithPatternStageRows(explicitHitStageRows, {
				"PATTERNSTAGEHITOFFSET\tENCOUNTER_VALTAN\tVALTAN_MISSING\tvaltan.test.active\t0\t100",
				validExplicitHitOffsets[1], validExplicitHitOffsets[2] }),
			"Reject an explicit boss hit offset whose pattern owner is unknown");
		const std::string mixedScheduleStageRows =
			"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t3\t300\t0\tdamage.player.34120\t2\t242\t1\t2000\n" +
			explicitHitStageRows.substr(
				explicitHitStageRows.find("PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1"));
		tests.Require(
			!loadWithPatternStageRows(
				mixedScheduleStageRows, validExplicitHitOffsets),
			"Reject simultaneous legacy and explicit boss hit schedules");
		std::error_code cleanupError;
		fs::remove_all(wallContactRoot, cleanupError);
	}

	{
		/* A staged skill carries movement per stage, because a stage advance
		resets the action clock the curve is sampled on. */
		const PLAYER_SKILL_DEFINITION* basicAttack = catalog.Find_Skill(34010);
		bool everyStageCurveFits = nullptr != basicAttack &&
			!basicAttack->ComboStages.empty() &&
			basicAttack->RootMotion.empty();
		bool anyStageMoves = false;
		if (nullptr != basicAttack)
		{
			for (const PLAYER_COMBO_STAGE& stage : basicAttack->ComboStages)
			{
				if (stage.RootMotion.empty())
					continue;
				anyStageMoves = true;
				everyStageCurveFits = everyStageCurveFits &&
					stage.RootMotion.size() >= 2u &&
					stage.RootMotion.back().iTimeMs <= stage.iActionDurationMs;
			}
		}
		tests.Require(everyStageCurveFits && anyStageMoves,
			"Resolve per-stage root motion inside each combo stage duration");

		namespace fs = std::filesystem;
		const fs::path stageRoot =
			fs::temp_directory_path() / L"LostArkStageRootMotionContractTest";
		std::error_code stagePrepareError;
		const auto loadWithStageRow = [&](
			const char* stageIndex, const char* comboAdvanceMs)
		{
			fs::remove_all(stageRoot, stagePrepareError);
			fs::create_directories(stageRoot / L"Gameplay");
			{
				std::ofstream bootstrap(
					stageRoot / L"Gameplay" / L"Gameplay.bootstrap",
					std::ios::binary);
				bootstrap <<
					"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
					'\t' << (20u + VALID_VALTAN_TIMELINE_ROW_COUNT) << "\n"
					"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
					"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
					"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
					"DAMAGE\tdamage.player.34010\t100\n"
					"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
					"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
					"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34010\t0\t0\t0\t0\n"
					"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
					"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
					"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
					"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34010\t0\t0\t0\t0\n"
					"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
					"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
					"SKILL\t34010\tLANCE_MASTER\tLMB\tlancemaster.skill.34010"
					"\t0\t1633\t470\t0\t0\t0\t3\tdamage.player.34010\tCOMBO"
					"\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
					"SKILLCOMBATTRAITS\t34010\t0\t0\t0\n"
					"SKILLSTAGE\t34010\t0\t1633\t470\t" << comboAdvanceMs <<
					"\t329\t658\n"
					"SKILLSTAGE\t34010\t1\t1367\t356\t1367\t0\t0\n"
					"SKILLSTAGEROOTMOTION\t34010\t" << stageIndex <<
					"\t2\t0:0:0,1600:1.5:0\n";
				Write_ValidValtanTimelineRows(bootstrap);
			}
			wchar_t previous[32768]{};
			const DWORD previousLength = GetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", previous,
				static_cast<DWORD>(std::size(previous)));
			SetEnvironmentVariableW(
				L"LOSTARK_SERVER_DATA_ROOT", stageRoot.c_str());
			CGameplayCatalog stageCatalog;
			const bool loaded = stageCatalog.Load();
			SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
				0u == previousLength || previousLength >= std::size(previous) ?
					nullptr : previous);
			return loaded;
		};
		tests.Require(loadWithStageRow("0", "470"),
			"Accept a root motion row that names an existing combo stage");
		tests.Require(!loadWithStageRow("2", "470"),
			"Reject a root motion row past the last combo stage");
		tests.Require(!loadWithStageRow("0", "469"),
			"Reject a combo boundary before its damage time atomically");
		tests.Require(!loadWithStageRow("0", "1634"),
			"Reject a combo boundary after its stage duration atomically");
		std::error_code stageCleanupError;
		fs::remove_all(stageRoot, stageCleanupError);
	}

	{
		/* ?덈！??guards, and a hit taken inside that window is what buys the
		counter: no press advances it and the guard itself lands nothing. */
		SERVER_PLAYER counterPlayer{};
		counterPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		counterPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		counterPlayer.iCurrentHp = 1000;
		counterPlayer.iMaximumHp = 1000;
		counterPlayer.iCurrentResource = 1000;
		counterPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem counterSkills;

		C2S_USE_SKILL counterCommand{};
		counterCommand.iClientSequence = 1;
		counterCommand.iSkillId = 34580;
		counterCommand.fAimX = 1.f;
		counterCommand.fAimZ = 0.f;
		tests.Require(
			counterSkills.Try_Start(counterPlayer, counterCommand, catalog, 10) &&
			1u == counterPlayer.iComboStage,
			"Approve the counter guard stage");

		std::vector<SERVER_WORLD_ENTITY> counterEntities;
		std::vector<DAMAGE_EVENT> counterDamageEvents;
		counterSkills.Update(counterPlayer, counterEntities, catalog, nullptr,
			nullptr, 1.f / 30.f, 11, counterDamageEvents);
		tests.Require(
			1u == counterPlayer.iComboStage && counterDamageEvents.empty(),
			"Hold the guard stage and land no damage while it runs");

		const std::uint32_t hpBeforeCounter = counterPlayer.iCurrentHp;
		tests.Require(
			CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 12) &&
			2u == counterPlayer.iComboStage &&
			hpBeforeCounter == counterPlayer.iCurrentHp &&
			0.f == counterPlayer.fActionElapsedSeconds,
			"Absorb the hit inside the guard window and promote to the counter");
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(counterPlayer, catalog, 13),
			"Do not counter twice from one guard");

		SERVER_PLAYER lateCounter{};
		lateCounter.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		lateCounter.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		lateCounter.iCurrentHp = 1000;
		lateCounter.iMaximumHp = 1000;
		lateCounter.iCurrentResource = 1000;
		lateCounter.iMaximumResource = 1000;
		CPlayerSkillSystem lateSkills;
		C2S_USE_SKILL lateCommand = counterCommand;
		lateSkills.Try_Start(lateCounter, lateCommand, catalog, 10);
		lateCounter.fActionElapsedSeconds = 1.5f;
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(lateCounter, catalog, 20) &&
			1u == lateCounter.iComboStage,
			"Reject a hit that lands after the guard window closed");

		SERVER_PLAYER comboPlayer{};
		comboPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		comboPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		comboPlayer.iCurrentHp = 1000;
		comboPlayer.iMaximumHp = 1000;
		comboPlayer.iCurrentResource = 1000;
		comboPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem comboSkills;
		C2S_USE_SKILL basicAttack{};
		basicAttack.iClientSequence = 1;
		basicAttack.iSkillId = 34010;
		basicAttack.fAimX = 1.f;
		basicAttack.fAimZ = 0.f;
		comboSkills.Try_Start(comboPlayer, basicAttack, catalog, 10);
		tests.Require(
			!CPlayerSkillSystem::Try_Counter(comboPlayer, catalog, 11),
			"Never counter out of a skill that is not a COUNTER");
	}

	{
		SERVER_PLAYER stancePlayer{};
		stancePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		stancePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		stancePlayer.iCurrentHp = 1000;
		stancePlayer.iMaximumHp = 1000;
		stancePlayer.iCurrentResource = 1000;
		stancePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem stanceSkills;

		C2S_USE_SKILL shortOnlySkill{};
		shortOnlySkill.iClientSequence = 1;
		shortOnlySkill.iSkillId = 34540;
		shortOnlySkill.fAimX = 1.f;
		shortOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, shortOnlySkill, catalog, 10),
			"Reject a short spear skill while in the long spear stance");

		C2S_USE_SKILL switchToShort{};
		switchToShort.iClientSequence = 2;
		switchToShort.iSkillId = 34000;
		switchToShort.fAimX = 1.f;
		switchToShort.fAimZ = 0.f;
		tests.Require(stanceSkills.Try_Start(stancePlayer, switchToShort, catalog, 10),
			"Approve the long to short spear stance transition");
		std::vector<SERVER_WORLD_ENTITY> stanceEntities;
		std::vector<DAMAGE_EVENT> stanceDamageEvents;
		for (std::uint32_t tick = 11; tick < 40; ++tick)
		{
			stanceSkills.Update(stancePlayer, stanceEntities, catalog, nullptr,
				nullptr, 1.f / 30.f, tick, stanceDamageEvents);
		}
		tests.Require(
			PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == stancePlayer.eStance &&
			PLAYER_ACTION_STATE::NONE == stancePlayer.eAction,
			"Flip to the short spear stance once the transition action completes");

		C2S_USE_SKILL longOnlySkill{};
		longOnlySkill.iClientSequence = 3;
		longOnlySkill.iSkillId = 34120;
		longOnlySkill.fAimX = 1.f;
		longOnlySkill.fAimZ = 0.f;
		tests.Require(!stanceSkills.Try_Start(stancePlayer, longOnlySkill, catalog, 40),
			"Reject a long spear skill after switching to the short spear stance");

		C2S_USE_SKILL shortSkillNow = shortOnlySkill;
		shortSkillNow.iClientSequence = 4;
		tests.Require(stanceSkills.Try_Start(stancePlayer, shortSkillNow, catalog, 40),
			"Approve a short spear skill after switching to the short spear stance");
	}

	{
		SERVER_PLAYER holdPlayer{};
		holdPlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		holdPlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		holdPlayer.iCurrentHp = 1000;
		holdPlayer.iMaximumHp = 1000;
		holdPlayer.iCurrentResource = 1000;
		holdPlayer.iMaximumResource = 1000;
		CPlayerSkillSystem holdSkills;

		C2S_USE_SKILL chargeStart{};
		chargeStart.iClientSequence = 1;
		chargeStart.iSkillId = 34590;
		chargeStart.fAimX = 1.f;
		chargeStart.fAimZ = 0.f;
		tests.Require(holdSkills.Try_Start(holdPlayer, chargeStart, catalog, 10),
			"Approve the short spear hold skill");

		C2S_UPDATE_SKILL_AIM turnedAim{};
		turnedAim.iClientSequence = 2;
		turnedAim.iSkillId = 34590;
		turnedAim.fAimX = 0.f;
		turnedAim.fAimZ = -1.f;
		const float chargeYaw = holdPlayer.fYawDegrees;
		holdSkills.Update_Aim(holdPlayer, turnedAim, catalog);
		tests.Require(
			holdPlayer.fYawDegrees != chargeYaw &&
			holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Turn a charging hold skill toward a new aim");

		C2S_UPDATE_SKILL_AIM wrongSkillAim = turnedAim;
		wrongSkillAim.iClientSequence = 3;
		wrongSkillAim.iSkillId = 34540;
		wrongSkillAim.fAimX = 1.f;
		wrongSkillAim.fAimZ = 1.f;
		holdSkills.Update_Aim(holdPlayer, wrongSkillAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Ignore an aim update naming a skill that is not running");

		C2S_RELEASE_SKILL chargeRelease{};
		chargeRelease.iClientSequence = 2u;
		chargeRelease.iSkillId = 34590u;
		holdSkills.Release(holdPlayer, chargeRelease, catalog);
		tests.Require(
			holdPlayer.hasReleasedHold &&
			2u == holdPlayer.iLastSkillSequence,
			"Consume release for HOLD while COMBO release remains unsupported");
		C2S_UPDATE_SKILL_AIM releasedAim = turnedAim;
		releasedAim.iClientSequence = 4;
		releasedAim.fAimX = 1.f;
		releasedAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, releasedAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim once the hold key is released");

		holdPlayer.hasReleasedHold = false;
		holdPlayer.iComboStage = 3u;
		C2S_UPDATE_SKILL_AIM firingAim = turnedAim;
		firingAim.iClientSequence = 5;
		firingAim.fAimX = 1.f;
		firingAim.fAimZ = 0.f;
		holdSkills.Update_Aim(holdPlayer, firingAim, catalog);
		tests.Require(holdPlayer.fSkillAimDirectionZ < -0.99f,
			"Keep the last aim through the firing stage");

		SERVER_PLAYER activePlayer{};
		activePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		activePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		activePlayer.iCurrentHp = 1000;
		activePlayer.iMaximumHp = 1000;
		activePlayer.iCurrentResource = 1000;
		activePlayer.iMaximumResource = 1000;
		CPlayerSkillSystem activeSkills;
		C2S_USE_SKILL activeStart{};
		activeStart.iClientSequence = 1;
		activeStart.iSkillId = 34540;
		activeStart.fAimX = 1.f;
		activeStart.fAimZ = 0.f;
		tests.Require(activeSkills.Try_Start(activePlayer, activeStart, catalog, 10),
			"Approve a non-hold short spear skill for the aim guard");
		const float activeAimX = activePlayer.fSkillAimDirectionX;
		C2S_UPDATE_SKILL_AIM activeAim{};
		activeAim.iClientSequence = 2;
		activeAim.iSkillId = 34540;
		activeAim.fAimX = 0.f;
		activeAim.fAimZ = -1.f;
		activeSkills.Update_Aim(activePlayer, activeAim, catalog);
		tests.Require(activeAimX == activePlayer.fSkillAimDirectionX,
			"Ignore an aim update on a skill that is not a HOLD");
	}
}

