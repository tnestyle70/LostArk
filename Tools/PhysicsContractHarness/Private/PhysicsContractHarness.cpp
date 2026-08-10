#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "Physics_Manager.h"
#include "DestructionSimulationDocument.h"
#include "WorldDestructionDocument.h"

namespace
{
	using namespace Client;

	DESTRUCTION_SIMULATION_PROFILE Make_Profile()
	{
		DESTRUCTION_SIMULATION_PROFILE profile;
		profile.profileId = "destroyable.group.contract.preview";
		profile.groupId = "destroyable.group.contract";
		profile.fDurationSeconds = 5.f;
		profile.isPreviewGroundEnabled = true;
		profile.fPreviewGroundHeight = 0.f;
		profile.vPreviewGroundHalfExtents = { 10.f, 10.f };

		for (const uint64_t placementId : { 101u, 102u })
		{
			DESTRUCTION_SIMULATION_ELEMENT element;
			element.elementId = "debris." + std::to_string(placementId);
			element.sourceRuntimePlacementId = placementId;
			element.vDirection = placementId == 101u ?
				float3_t(1.f, 0.f, 0.f) : float3_t(-1.f, 0.f, 0.f);
			element.fSpeedMetersPerSecond = 8.f;
			element.fGravityScale = 1.f;
			element.fLifetimeSeconds = 4.f;
			element.Trigger.eKind =
				DESTRUCTION_SIMULATION_TRIGGER_KIND::IMMEDIATE;
			profile.Elements.push_back(std::move(element));
		}
		profile.Elements.front().suppressionAliasPlacementIds.push_back(103u);
		return profile;
	}

	bool Write_Text(
		const std::filesystem::path& path,
		const std::string& text)
	{
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		output.write(text.data(), static_cast<std::streamsize>(text.size()));
		return output.good();
	}

	bool Read_Text(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return false;
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
			return false;
		outText = buffer.str();
		return true;
	}

	bool Replace_First(
		std::string& text,
		const std::string& from,
		const std::string& to)
	{
		const size_t offset = text.find(from);
		if (std::string::npos == offset)
			return false;
		text.replace(offset, from.size(), to);
		return true;
	}

	bool Run_DestructionDocumentContract()
	{
		const std::filesystem::path root =
			std::filesystem::temp_directory_path() /
			("lostark-destruction-contract-" +
				std::to_string(GetCurrentProcessId()));
		std::error_code cleanupError;
		std::filesystem::remove_all(root, cleanupError);
		std::filesystem::create_directories(root, cleanupError);
		if (cleanupError)
			return false;

		const std::filesystem::path validPath = root / "valid.json";
		const std::filesystem::path badVersionPath = root / "bad-version.json";
		const std::filesystem::path badSchemaPath = root / "bad-schema.json";
		const std::filesystem::path malformedPath = root / "malformed.json";
		std::string status;

		CDestructionSimulationDocument authored;
		authored.Reset_Empty();
		const DESTRUCTION_SIMULATION_PROFILE profile = Make_Profile();
		const bool added = authored.Add_Profile(profile, status);
		const bool saved = added && authored.Save(
			validPath, "TEST_AREA", status);

		CDestructionSimulationDocument loaded;
		const bool roundTrip = saved && loaded.Load(
			validPath, "TEST_AREA", status) &&
			loaded.Get_Profiles().size() == 1u &&
			loaded.Get_Profiles().front().Elements.size() == 2u &&
			loaded.Get_Profiles().front().Elements.front().suppressionAliasPlacementIds.size() == 1u &&
			loaded.Get_Profiles().front().Elements.front().suppressionAliasPlacementIds.front() == 103u;
		std::string validText;
		const bool readValid = Read_Text(validPath, validText);

		std::string badVersion = validText;
		const bool wroteBadVersion = readValid &&
			Replace_First(badVersion, "\"formatVersion\": 2",
				"\"formatVersion\": 1") &&
			Write_Text(badVersionPath, badVersion);
		const bool rejectedVersion = wroteBadVersion &&
			!loaded.Load(badVersionPath, "TEST_AREA", status) &&
			loaded.Get_Profiles().size() == 1u;

		std::string badSchema = validText;
		const bool wroteBadSchema = readValid &&
			Replace_First(badSchema, "lostark.destruction-simulation",
				"lostark.unknown") &&
			Write_Text(badSchemaPath, badSchema);
		const bool rejectedSchema = wroteBadSchema &&
			!loaded.Load(badSchemaPath, "TEST_AREA", status) &&
			loaded.Get_Profiles().size() == 1u;
		const bool rejectedArea =
			!loaded.Load(validPath, "OTHER_AREA", status) &&
			loaded.Get_Profiles().size() == 1u;
		const bool rejectedMalformed =
			Write_Text(malformedPath, "{\"schema\":") &&
			!loaded.Load(malformedPath, "TEST_AREA", status) &&
			loaded.Get_Profiles().size() == 1u;

		const bool rejectedDuplicateProfile =
			!loaded.Add_Profile(profile, status);
		DESTRUCTION_SIMULATION_PROFILE duplicateElement = profile;
		duplicateElement.profileId =
			"destroyable.group.contract.duplicate-element";
		duplicateElement.Elements[1].elementId =
			duplicateElement.Elements[0].elementId;
		const bool rejectedDuplicateElement =
			!loaded.Add_Profile(duplicateElement, status);
		DESTRUCTION_SIMULATION_PROFILE duplicatePlacement = profile;
		duplicatePlacement.profileId =
			"destroyable.group.contract.duplicate-placement";
		duplicatePlacement.Elements[1].sourceRuntimePlacementId =
			duplicatePlacement.Elements[0].sourceRuntimePlacementId;
		const bool rejectedDuplicatePlacement =
			!loaded.Add_Profile(duplicatePlacement, status);
		DESTRUCTION_SIMULATION_PROFILE duplicateAlias = profile;
		duplicateAlias.profileId =
			"destroyable.group.contract.duplicate-alias";
		duplicateAlias.Elements.front().suppressionAliasPlacementIds.front() =
			duplicateAlias.Elements.back().sourceRuntimePlacementId;
		const bool rejectedDuplicateAlias =
			!loaded.Add_Profile(duplicateAlias, status);
		DESTRUCTION_SIMULATION_PROFILE badId = profile;
		badId.profileId = "bad/profile/id";
		const bool rejectedBadId = !loaded.Add_Profile(badId, status);
		const bool rejectedEmptySavePath =
			!loaded.Save({}, "TEST_AREA", status);

		CWorldDestructionDocument world;
		world.Reset_Empty();
		const bool builtWorld =
			world.Add_Group(profile.groupId, status) &&
			world.Add_Member(profile.groupId, 101u, status) &&
			world.Add_Member(profile.groupId, 102u, status) &&
			world.Add_Member(profile.groupId, 103u, status);
		const bool acceptedCrossReferences = builtWorld &&
			loaded.Validate_GroupReferences(world, status);
		const bool rejectedCrossMismatch =
			world.Add_Member(profile.groupId, 104u, status) &&
			!loaded.Validate_GroupReferences(world, status);

		CDestructionSimulationDocument changed = loaded;
		DESTRUCTION_SIMULATION_ELEMENT changedElement =
			*changed.Find_Element(profile.profileId, "debris.101");
		changedElement.fSpeedMetersPerSecond = 12.f;
		const bool changedDocument = changed.Update_Element(
			profile.profileId, changedElement, status);
		std::string beforeLockedSave;
		const bool readBeforeLockedSave = Read_Text(
			validPath, beforeLockedSave);
		HANDLE lockedDestination = CreateFileW(
			validPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		const bool lockOpened = INVALID_HANDLE_VALUE != lockedDestination;
		const bool rejectedLockedReplace = lockOpened && changedDocument &&
			!changed.Save(validPath, "TEST_AREA", status);
		if (lockOpened)
			CloseHandle(lockedDestination);
		std::string afterLockedSave;
		const bool preservedDestination = readBeforeLockedSave &&
			Read_Text(validPath, afterLockedSave) &&
			beforeLockedSave == afterLockedSave &&
			!std::filesystem::exists(validPath.wstring() + L".tmp");

		const std::filesystem::path worldPath = root / "world-events.json";
		const bool savedWorld = world.Save(
			worldPath, "TEST_AREA", "ENCOUNTER_TEST", status);
		std::string worldBeforeLockedSave;
		const bool readWorldBeforeLockedSave = savedWorld &&
			Read_Text(worldPath, worldBeforeLockedSave);
		world.Set_NavPolarity(profile.groupId,
			DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED);
		HANDLE lockedWorld = CreateFileW(
			worldPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		const bool worldLockOpened = INVALID_HANDLE_VALUE != lockedWorld;
		const bool rejectedLockedWorldReplace = worldLockOpened &&
			!world.Save(worldPath, "TEST_AREA", "ENCOUNTER_TEST", status);
		if (worldLockOpened)
			CloseHandle(lockedWorld);
		std::string worldAfterLockedSave;
		const bool preservedWorldDestination = readWorldBeforeLockedSave &&
			Read_Text(worldPath, worldAfterLockedSave) &&
			worldBeforeLockedSave == worldAfterLockedSave &&
			!std::filesystem::exists(worldPath.wstring() + L".tmp");

		const std::filesystem::path pairWorldPath =
			root / "pair-world-events.json";
		const std::filesystem::path pairSimulationPath =
			root / "pair-simulation.json";
		CWorldDestructionDocument pairWorld;
		pairWorld.Reset_Empty();
		const bool builtPairWorld =
			pairWorld.Add_Group(profile.groupId, status) &&
			pairWorld.Add_Member(profile.groupId, 101u, status) &&
			pairWorld.Add_Member(profile.groupId, 102u, status) &&
			pairWorld.Add_Member(profile.groupId, 103u, status);
		CDestructionSimulationDocument pairSimulation;
		pairSimulation.Reset_Empty();
		const bool builtPairSimulation =
			pairSimulation.Add_Profile(profile, status);
		const bool pairCommitted = builtPairWorld && builtPairSimulation &&
			CDestructionSimulationDocument::Save_AuthoringPair(
				pairWorld, pairWorldPath,
				pairSimulation, pairSimulationPath,
				"TEST_AREA", "ENCOUNTER_TEST", status);
		CWorldDestructionDocument reloadedPairWorld;
		CDestructionSimulationDocument reloadedPairSimulation;
		const bool pairRoundTrip = pairCommitted &&
			reloadedPairWorld.Load(pairWorldPath, "TEST_AREA",
				"ENCOUNTER_TEST", status) &&
			reloadedPairSimulation.Load(
				pairSimulationPath, "TEST_AREA", status) &&
			reloadedPairSimulation.Validate_GroupReferences(
				reloadedPairWorld, status);

		std::string pairWorldBeforeFailure;
		std::string pairSimulationBeforeFailure;
		const bool readPairBeforeFailure = pairRoundTrip &&
			Read_Text(pairWorldPath, pairWorldBeforeFailure) &&
			Read_Text(pairSimulationPath, pairSimulationBeforeFailure);
		CWorldDestructionDocument changedPairWorld = pairWorld;
		const bool changedPairWorldDocument = changedPairWorld.Set_NavPolarity(
			profile.groupId,
			DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_FRACTURED);
		CDestructionSimulationDocument changedPairSimulation = pairSimulation;
		const DESTRUCTION_SIMULATION_ELEMENT* pairElement =
			changedPairSimulation.Find_Element(profile.profileId, "debris.101");
		DESTRUCTION_SIMULATION_ELEMENT changedPairElement =
			nullptr != pairElement ? *pairElement :
			DESTRUCTION_SIMULATION_ELEMENT{};
		changedPairElement.fSpeedMetersPerSecond = 17.f;
		const bool changedPairSimulationDocument = nullptr != pairElement &&
			changedPairSimulation.Update_Element(
				profile.profileId, changedPairElement, status);
		HANDLE lockedPairSimulation = CreateFileW(
			pairSimulationPath.c_str(), GENERIC_READ, 0u,
			nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		const bool pairLockOpened =
			INVALID_HANDLE_VALUE != lockedPairSimulation;
		const bool rejectedPairSecondCommit = pairLockOpened &&
			readPairBeforeFailure && changedPairWorldDocument &&
			changedPairSimulationDocument &&
			!CDestructionSimulationDocument::Save_AuthoringPair(
				changedPairWorld, pairWorldPath,
				changedPairSimulation, pairSimulationPath,
				"TEST_AREA", "ENCOUNTER_TEST", status);
		const bool reportedPairRollback = rejectedPairSecondCommit &&
			status == "Destruction simulation pair commit failed; "
				"original documents restored";
		if (pairLockOpened)
			CloseHandle(lockedPairSimulation);
		std::string pairWorldAfterFailure;
		std::string pairSimulationAfterFailure;
		const bool preservedPairDestinations = rejectedPairSecondCommit &&
			Read_Text(pairWorldPath, pairWorldAfterFailure) &&
			Read_Text(pairSimulationPath, pairSimulationAfterFailure) &&
			pairWorldBeforeFailure == pairWorldAfterFailure &&
			pairSimulationBeforeFailure == pairSimulationAfterFailure;
		std::error_code sidecarError;
		bool pairSidecarsClean = true;
		std::filesystem::directory_iterator sidecarIterator(root, sidecarError);
		const std::filesystem::directory_iterator sidecarEnd;
		for (; !sidecarError && sidecarIterator != sidecarEnd;
			sidecarIterator.increment(sidecarError))
		{
			if (std::string::npos != sidecarIterator->path().filename().string().find(
				".destruction-pair."))
			{
				pairSidecarsClean = false;
			}
		}
		pairSidecarsClean = pairSidecarsClean && !sidecarError;
		const bool pairRollback = reportedPairRollback &&
			preservedPairDestinations &&
			pairSidecarsClean;

		const bool passed = roundTrip && rejectedVersion && rejectedSchema &&
			rejectedArea && rejectedMalformed && rejectedDuplicateProfile &&
			rejectedDuplicateElement && rejectedDuplicatePlacement &&
			rejectedDuplicateAlias &&
			rejectedBadId && rejectedEmptySavePath &&
			acceptedCrossReferences && rejectedCrossMismatch &&
			rejectedLockedReplace && preservedDestination &&
			rejectedLockedWorldReplace && preservedWorldDestination &&
			pairRoundTrip && pairRollback;

		std::cout
			<< "documentRoundTrip=" << (roundTrip ? "true" : "false") << '\n'
			<< "documentNegativeInputs="
			<< ((rejectedVersion && rejectedSchema && rejectedArea &&
				rejectedMalformed && rejectedDuplicateProfile &&
				rejectedDuplicateElement && rejectedDuplicatePlacement &&
				rejectedDuplicateAlias &&
				rejectedBadId && rejectedEmptySavePath) ? "true" : "false")
			<< '\n'
			<< "documentCrossReferences="
			<< ((acceptedCrossReferences && rejectedCrossMismatch) ?
				"true" : "false") << '\n'
			<< "documentCommitRollback="
			<< ((rejectedLockedReplace && preservedDestination &&
				rejectedLockedWorldReplace && preservedWorldDestination) ?
				"true" : "false") << '\n'
			<< "documentPairCommit="
			<< (pairRoundTrip ? "true" : "false") << '\n'
			<< "documentPairRollback="
			<< (pairRollback ? "true" : "false") << '\n';

		std::filesystem::remove_all(root, cleanupError);
		return passed && !cleanupError;
	}
}

int main()
{
	Engine::PHYSICS_CONTRACT_RESULT result;
	const HRESULT physicsResult =
		Engine::CPhysics_Manager::Run_ContractTest(result);
	const bool documentResult = Run_DestructionDocumentContract();

	std::cout << std::fixed << std::setprecision(3)
		<< "initialHeight=" << result.fInitialHeight << '\n'
		<< "finalHeight=" << result.fFinalHeight << '\n'
		<< "steps=" << result.iStepsExecuted << '\n'
		<< "rejectUnpausedDebugSteps="
		<< (result.didRejectUnpausedDebugSteps ? "true" : "false") << '\n'
		<< "preserveDebugAccumulator="
		<< (result.didPreserveDebugAccumulator ? "true" : "false") << '\n'
		<< "rejectInvalidDebugStepCount="
		<< (result.didRejectInvalidDebugStepCount ? "true" : "false") << '\n'
		<< "shapeLocalPose="
		<< (result.didRespectShapeLocalPose ? "true" : "false") << '\n'
		<< "settled=" << (result.didSettleOnSupport ? "true" : "false")
		<< '\n'
		<< "clearInvalidated="
		<< (result.didInvalidateOnClear ? "true" : "false") << '\n';

	if (FAILED(physicsResult) || !documentResult)
	{
		std::cerr << "[FAIL] physics-and-destruction.contract\n";
		return 1;
	}

	std::cout << "[PASS] physics-and-destruction.contract\n";
	return 0;
}
