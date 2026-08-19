#include "WorldDestructionBootstrapContractTests.h"

#include "WorldDestructionBootstrap.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>

namespace
{
	const char* VALID_BOOTSTRAP =
		"LOSTARK_WORLD_DESTRUCTION_BOOTSTRAP\t2\tENCOUNTER_VALTAN\t"
		"LV_LUT_HEARTRB_ED\t1d571860636803667901cfdd4b26c2aa8656be7cab129156bed62eb777149aab\t30\t1\t1\t1\n"
		"G\tdestroyable.group.valtan.wall.3705102\tINTACT\t5\t"
		"9335938568718910930\t9681544306002658031\t12037145985028191659\t"
		"17280669848983777578\t18177041425620847396\n"
		"M\tmutation.valtan.wall.3705102.break\t"
		"destroyable.group.valtan.wall.3705102\tFRACTURED\t8\t-\t-\t0\n"
		"B\tbinding.valtan.wall.3705102.preview\t"
		"mutation.valtan.wall.3705102.break\tSTAGE\tVALTAN_ARENA_BREAK_109\t"
		"IMPACT\tvaltan.mechanic.arena-break-109.impact\t2\t-\n";

	void Write_File(
		const std::filesystem::path& path,
		const std::string& contents)
	{
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		output.write(contents.data(),
			static_cast<std::streamsize>(contents.size()));
	}

	void Replace_First(
		std::string& value,
		const std::string& from,
		const std::string& to)
	{
		const std::size_t position = value.find(from);
		if (std::string::npos != position)
			value.replace(position, from.size(), to);
	}
}

int LostArk::Server::Run_WorldDestructionBootstrapContractTests()
{
	int failures = 0;
	const auto require = [&failures](const bool condition, const char* name)
	{
		std::cout << (condition ? "[PASS] " : "[FAILURE] ") << name << '\n';
		if (!condition) ++failures;
	};

	const std::filesystem::path root =
		std::filesystem::temp_directory_path() /
		L"LostArkWorldDestructionBootstrapContract";
	std::error_code error;
	std::filesystem::remove_all(root, error);
	std::filesystem::create_directories(root, error);
	const std::filesystem::path path = root / L"fixture.bootstrap";
	Write_File(path, VALID_BOOTSTRAP);

	CWorldDestructionBootstrap publishedBootstrap;
	std::size_t publishedMemberCount = 0u;
	require(
		publishedBootstrap.Load_ValtanArena(),
		"Load the canonical published Valtan destruction product");
	for (const WORLD_DESTRUCTION_GROUP_DESCRIPTOR& group :
		publishedBootstrap.Get_DescriptorGraph().Groups)
	{
		publishedMemberCount += group.MemberPlacementIds.size();
	}
	/* The enabled product graph is thirty independent 109 outer-ring walls and
	sixty-nine ordinary contact walls. The outer ring owns only its 109 stage
	binding; attack/body contact must never remove it. */
	std::size_t publishedOuterGroupCount = 0u;
	std::size_t publishedOuterMemberCount = 0u;
	for (const WORLD_DESTRUCTION_GROUP_DESCRIPTOR& group :
		publishedBootstrap.Get_DescriptorGraph().Groups)
	{
		if (0u != group.strGroupId.rfind(
			"destroyable.group.valtan.outerwall109.", 0u))
		{
			continue;
		}
		++publishedOuterGroupCount;
		publishedOuterMemberCount += group.MemberPlacementIds.size();
	}
	/* Ninety-nine independent walls plus the six arena floor sectors: two outer
	rail halves that drop at 84 bars and four brick sectors that drop at 30. The
	floor sectors add one member each and never join the 109 outer ring. */
	std::size_t publishedFloorGroupCount = 0u;
	std::size_t publishedFloorMemberCount = 0u;
	for (const WORLD_DESTRUCTION_GROUP_DESCRIPTOR& group :
		publishedBootstrap.Get_DescriptorGraph().Groups)
	{
		if (0u != group.strGroupId.rfind("destroyable.group.valtan.floor", 0u))
		{
			continue;
		}
		++publishedFloorGroupCount;
		publishedFloorMemberCount += group.MemberPlacementIds.size();
	}
	/* Only a floor sector takes ground away. A wall mutation clears an
	obstacle beside the player, so claiming otherwise would drop somebody
	inside standing geometry. Every ground-removing mutation must also own the
	navigation condition whose cells are the hole. */
	std::size_t publishedRemovedGroundCount = 0u;
	bool publishedRemovedGroundIsFloorOnly = true;
	for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
		publishedBootstrap.Get_DescriptorGraph().Mutations)
	{
		if (!mutation.bRemovesGround)
			continue;
		++publishedRemovedGroundCount;
		if (0u != mutation.strGroupId.rfind(
				"destroyable.group.valtan.floor", 0u) ||
			mutation.strNavigationStateId.empty())
		{
			publishedRemovedGroundIsFloorOnly = false;
		}
	}
	require(
		6u == publishedRemovedGroundCount &&
		publishedRemovedGroundIsFloorOnly,
		"Declare removed ground on the six floor mutations and nowhere else");

	require(
		105u == publishedBootstrap.Get_DescriptorGraph().Groups.size() &&
		105u == publishedBootstrap.Get_DescriptorGraph().Mutations.size() &&
		117u == publishedBootstrap.Get_DescriptorGraph().Bindings.size() &&
		113u == publishedMemberCount &&
		30u == publishedOuterGroupCount &&
		30u == publishedOuterMemberCount &&
		6u == publishedFloorGroupCount &&
		6u == publishedFloorMemberCount &&
		publishedBootstrap.Get_CombatRuntimeRevision().size() == 64u,
		"Load ninety-nine independent walls and six Valtan floor collapse sectors");
	CWorldDestructionRuntime publishedRuntime;
	std::string publishedRuntimeStatus;
	WORLD_DESTRUCTION_TRANSACTION publishedTransaction{};
	const WORLD_DESTRUCTION_ACTION_TUPLE arenaBreakAction{
		"VALTAN_ARENA_BREAK_109",
		"IMPACT",
		"valtan.mechanic.arena-break-109.impact",
		2u };
	require(
		publishedRuntime.Initialize(
			publishedBootstrap.Get_DescriptorGraph(), publishedRuntimeStatus) &&
		WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
			publishedRuntime.Prepare_StageTrigger(
				arenaBreakAction, 7001u, 80u, 450u,
				publishedTransaction, publishedRuntimeStatus) &&
		30u == publishedTransaction.Transitions.size() &&
		30u == publishedTransaction.BindingApplications.size() &&
		std::all_of(
			publishedTransaction.Transitions.begin(),
			publishedTransaction.Transitions.end(),
			[](const WORLD_DESTRUCTION_STATE_TRANSITION& transition)
			{
				/* The 109 batch is the outer ring alone. An interior group
				reaching this transaction is the exact regression that made the
				whole arena collapse at once. */
				return !transition.strCollisionStateId.empty() &&
					0u == transition.strGroupId.rfind(
						"destroyable.group.valtan.outerwall109.", 0u);
			}) &&
		30u == std::accumulate(
			publishedTransaction.Transitions.begin(),
			publishedTransaction.Transitions.end(),
			std::size_t{ 0u },
			[](const std::size_t total,
				const WORLD_DESTRUCTION_STATE_TRANSITION& transition)
			{
				return total + transition.MemberPlacementIds.size();
			}) &&
		publishedRuntime.Commit(
			publishedTransaction, publishedRuntimeStatus),
		"Prepare and commit thirty independent 109-bar outer ring walls in one batch");
	require(
		WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST ==
			publishedRuntime.Prepare_StageTrigger(
				arenaBreakAction, 7001u, 80u, 451u,
				publishedTransaction, publishedRuntimeStatus) &&
		publishedTransaction.Transitions.empty(),
		"Treat the repeated 109-bar impact edge as one idempotent no-op");

	/* The 109 collapse opens the outer ring only. Every floor sector has to
	survive it, otherwise the arena would lose its footing one whole health-bar
	chain too early. */
	{
		std::size_t survivingFloorSectors = 0u;
		for (const WORLD_DESTRUCTION_GROUP_STATE& state :
			publishedRuntime.Get_GroupStates())
		{
			if (0u != state.strGroupId.rfind(
				"destroyable.group.valtan.floor", 0u))
			{
				continue;
			}
			if (WORLD_DESTRUCTION_STATE::INTACT == state.eState)
				++survivingFloorSectors;
		}
		require(
			6u == survivingFloorSectors,
			"Leave every floor sector INTACT when the 109 outer ring collapses");
	}

	/* Stage A is the two outer rail halves at 84 bars and stage B is the four
	brick sectors at the 30-bar landing. Each stage has to reach its own sectors
	and nothing else, and a floor sector owns no collision channel at all. */
	{
		const WORLD_DESTRUCTION_ACTION_TUPLE floorStageAAction{
			"VALTAN_ARENA_BREAK_84",
			"IMPACT",
			"valtan.mechanic.arena-floor-84.impact",
			1u };
		const WORLD_DESTRUCTION_ACTION_TUPLE floorStageBAction{
			"VALTAN_ARENA_BREAK_33",
			"LANDING",
			"valtan.mechanic.arena-break-33.landing",
			1u };
		const auto reachesOnly = [](
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const char* prefix) -> bool
			{
				return std::all_of(
					transaction.Transitions.begin(),
					transaction.Transitions.end(),
					[prefix](const WORLD_DESTRUCTION_STATE_TRANSITION& transition)
					{
						return transition.strCollisionStateId.empty() &&
							!transition.strNavigationStateId.empty() &&
							WORLD_DESTRUCTION_STATE::DESPAWNED ==
								transition.eFinalState &&
							0u == transition.strGroupId.rfind(prefix, 0u);
					});
			};

		CWorldDestructionRuntime floorRuntime;
		WORLD_DESTRUCTION_TRANSACTION floorTransaction{};
		require(
			floorRuntime.Initialize(
				publishedBootstrap.Get_DescriptorGraph(),
				publishedRuntimeStatus) &&
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				floorRuntime.Prepare_StageTrigger(
					floorStageAAction, 7001u, 90u, 600u,
					floorTransaction, publishedRuntimeStatus) &&
			2u == floorTransaction.Transitions.size() &&
			reachesOnly(
				floorTransaction, "destroyable.group.valtan.floor84.rail.") &&
			floorRuntime.Commit(floorTransaction, publishedRuntimeStatus),
			"Collapse only the two outer rail sectors at the 84-bar impact");

		require(
			WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
				floorRuntime.Prepare_StageTrigger(
					floorStageBAction, 7001u, 91u, 900u,
					floorTransaction, publishedRuntimeStatus) &&
			4u == floorTransaction.Transitions.size() &&
			reachesOnly(
				floorTransaction, "destroyable.group.valtan.floor30.brick.") &&
			floorRuntime.Commit(floorTransaction, publishedRuntimeStatus),
			"Collapse only the four brick sectors at the 30-bar landing");

		require(
			WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST ==
				floorRuntime.Prepare_StageTrigger(
					floorStageAAction, 7001u, 90u, 1200u,
					floorTransaction, publishedRuntimeStatus) &&
			floorTransaction.Transitions.empty(),
			"Treat a repeated floor collapse edge as one idempotent no-op");
	}

	CWorldDestructionRuntime contactRuntime;
	require(
		contactRuntime.Initialize(
			publishedBootstrap.Get_DescriptorGraph(), publishedRuntimeStatus) &&
		WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
			contactRuntime.Prepare_ContactTrigger(
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634",
				7001u, 1u, 499u,
				publishedTransaction, publishedRuntimeStatus) &&
		1u == publishedTransaction.Transitions.size() &&
		1u == publishedTransaction.BindingApplications.size() &&
		publishedTransaction.Transitions.front().strGroupId ==
			"destroyable.group.valtan.wall159.15719065619666776634" &&
		contactRuntime.Commit(
			publishedTransaction, publishedRuntimeStatus),
		"Prepare one exact wall from its independent direct collider contact");

	CWorldDestructionRuntime protectedOuterRuntime;
	require(
		protectedOuterRuntime.Initialize(
			publishedBootstrap.Get_DescriptorGraph(), publishedRuntimeStatus) &&
		WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH ==
			protectedOuterRuntime.Prepare_ContactTrigger(
				"collision.valtan.wallgroup.sector00.1090000000000001",
				7001u, 2u, 499u,
				publishedTransaction, publishedRuntimeStatus) &&
		publishedTransaction.Transitions.empty(),
		"Protect every 109 outer wall from ordinary collider contact");

	const WORLD_DESTRUCTION_ACTION_TUPLE openingImpactAction{
		"VALTAN_ARMOR_BREAK_OPENING",
		"WALL_CHARGE",
		"valtan.mechanic.armor-break-opening.charge",
		0u };
	CWorldDestructionRuntime impactRuntime;
	require(
		impactRuntime.Initialize(
			publishedBootstrap.Get_DescriptorGraph(), publishedRuntimeStatus) &&
		WORLD_DESTRUCTION_PREPARE_RESULT::READY ==
			impactRuntime.Prepare_ImpactTrigger(
				openingImpactAction,
				"collision.valtan.wallgroup.11047903315509031966.15719065619666776634.receiver",
				7001u, 159u, 500u,
				publishedTransaction, publishedRuntimeStatus) &&
		1u == publishedTransaction.Transitions.size() &&
		1u == publishedTransaction.BindingApplications.size() &&
		publishedTransaction.Transitions.front().strCollisionStateId ==
			"collision.valtan.wallgroup.11047903315509031966.15719065619666776634" &&
		publishedTransaction.Transitions.front().strNavigationStateId ==
			"condition.valtan.wall159.15719065619666776634.destroyed" &&
		impactRuntime.Commit(
			publishedTransaction, publishedRuntimeStatus),
		"Prepare the exact opening charge receiver with collision and navigation channels");

	CWorldDestructionBootstrap bootstrap;
	require(
		bootstrap.Load_FromFile(path) &&
		bootstrap.Get_AreaId() == "LV_LUT_HEARTRB_ED" &&
		bootstrap.Get_EncounterId() == "ENCOUNTER_VALTAN" &&
		bootstrap.Get_FixedTickHz() == 30u &&
		bootstrap.Get_CombatRuntimeRevision() ==
			"1d571860636803667901cfdd4b26c2aa8656be7cab129156bed62eb777149aab" &&
		1u == bootstrap.Get_DescriptorGraph().Groups.size() &&
		1u == bootstrap.Get_DescriptorGraph().Mutations.size() &&
		1u == bootstrap.Get_DescriptorGraph().Bindings.size(),
		"Load the strict first-slice world destruction bootstrap");

	CWorldDestructionRuntime runtime;
	std::string runtimeStatus;
	require(
		runtime.Initialize(bootstrap.Get_DescriptorGraph(), runtimeStatus),
		"Initialize world destruction runtime from the loaded descriptor graph");

	const std::string preservedRevision =
		bootstrap.Get_CombatRuntimeRevision();
	const std::size_t preservedGroupCount =
		bootstrap.Get_DescriptorGraph().Groups.size();
	std::string invalidVersion = VALID_BOOTSTRAP;
	Replace_First(invalidVersion,
		"WORLD_DESTRUCTION_BOOTSTRAP\t2\t",
		"WORLD_DESTRUCTION_BOOTSTRAP\t3\t");
	Write_File(path, invalidVersion);
	require(
		!bootstrap.Load_FromFile(path) &&
		bootstrap.Get_CombatRuntimeRevision() == preservedRevision &&
		bootstrap.Get_DescriptorGraph().Groups.size() == preservedGroupCount,
		"Reject an unknown version without replacing the last valid graph");

	std::string badRevision = VALID_BOOTSTRAP;
	Replace_First(badRevision,
		"1d571860636803667901cfdd4b26c2aa8656be7cab129156bed62eb777149aab",
		"cabc172d2f7a2a73b44aaa76e22ddea60570f3e9362845addb5fad1adcb70511");
	Write_File(path, badRevision);
	require(
		!bootstrap.Load_FromFile(path) &&
		bootstrap.Get_CombatRuntimeRevision() == preservedRevision,
		"Reject content whose deterministic revision does not match");

	std::string duplicateGroup = VALID_BOOTSTRAP;
	Replace_First(duplicateGroup, "\t30\t1\t1\t1\n", "\t30\t2\t1\t1\n");
	const std::size_t mutationRow = duplicateGroup.find("M\t");
	const std::size_t groupRow = duplicateGroup.find("G\t");
	const std::string groupLine = duplicateGroup.substr(
		groupRow, duplicateGroup.find('\n', groupRow) - groupRow + 1u);
	duplicateGroup.insert(mutationRow, groupLine);
	Write_File(path, duplicateGroup);
	require(
		!bootstrap.Load_FromFile(path) &&
		bootstrap.Get_DescriptorGraph().Groups.size() == preservedGroupCount,
		"Reject duplicate group ownership atomically");

	std::string danglingMutation = VALID_BOOTSTRAP;
	Replace_First(danglingMutation,
		"M\tmutation.valtan.wall.3705102.break\t"
		"destroyable.group.valtan.wall.3705102",
		"M\tmutation.valtan.wall.3705102.break\t"
		"destroyable.group.valtan.wall.missing");
	Write_File(path, danglingMutation);
	require(
		!bootstrap.Load_FromFile(path) &&
		bootstrap.Get_DescriptorGraph().Groups.size() == preservedGroupCount,
		"Reject a dangling mutation group reference atomically");

	/* A v1 document predates the removed-ground column. Guessing the flag
	would silently decide which sectors a player can fall through, so the whole
	document is refused and the last valid graph stays. */
	std::string legacyVersion = VALID_BOOTSTRAP;
	Replace_First(legacyVersion,
		"WORLD_DESTRUCTION_BOOTSTRAP\t2\t",
		"WORLD_DESTRUCTION_BOOTSTRAP\t1\t");
	Replace_First(legacyVersion,
		"FRACTURED\t8\t-\t-\t0\n",
		"FRACTURED\t8\t-\t-\n");
	Write_File(path, legacyVersion);
	require(
		!bootstrap.Load_FromFile(path) &&
		bootstrap.Get_CombatRuntimeRevision() == preservedRevision &&
		bootstrap.Get_DescriptorGraph().Groups.size() == preservedGroupCount,
		"Reject a v1 bootstrap instead of guessing the removed-ground flag");

	/* The hole is the cells of a navigation condition. A mutation that claims
	to take ground away without owning one has nothing to open. */
	std::string groundWithoutCondition = VALID_BOOTSTRAP;
	Replace_First(groundWithoutCondition,
		"FRACTURED\t8\t-\t-\t0\n",
		"FRACTURED\t8\t-\t-\t1\n");
	Write_File(path, groundWithoutCondition);
	require(
		!bootstrap.Load_FromFile(path) &&
		bootstrap.Get_DescriptorGraph().Groups.size() == preservedGroupCount,
		"Reject removed ground on a mutation without a navigation condition");

	std::filesystem::remove_all(root, error);
	return failures;
}
