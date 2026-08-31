#include "Effect_Catalog.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternFlowDocument.h"
#include "ValtanPatternTree.h"

#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

/* ValtanPatternEffectCueDocument.cpp also owns a Product-prewarm entry point.
   The canonical graph loader exercised here calls Load_Source and never opens
   the runtime Effect catalog, but the standalone Debug linker still retains
   the unused function body. Keep this one harness-only symbol fail-closed; it
   is not an Effect catalog substitute and is never called by this test. */
bool_t Client::CEffectCatalog::Contains(const std::string&)
{
	throw std::runtime_error(
		"canonical graph harness unexpectedly entered Effect catalog prewarm");
}

namespace
{
	using namespace Client;
	constexpr size_t EXPECTED_PRODUCT_PATTERN_COUNT = 33u;
	constexpr size_t EXPECTED_ENCOUNTER_PATTERN_COUNT = 57u;
	constexpr const char* REQUESTED_PRODUCT_PATTERN_IDS[] = {
		"VALTAN_HIGH_JUMP",
		"VALTAN_SIX_PIZZA_106",
		"VALTAN_WARP",
		"VALTAN_TRASH",
		"VALTAN_CATCH_BREATH",
		"VALTAN_STRUGGLING",
		"VALTAN_DASH_CHARGE",
	};
	constexpr const char* REQUESTED_REFERENCE_ONLY_PATTERN_IDS[] = {
		"VALTAN_MAGIC_ORB_STAGGER_76",
		"VALTAN_TRIPLE_COUNTER",
		"VALTAN_FOUR_PILLARS_105",
	};

	void Require(const bool_t bCondition, const char* const pMessage)
	{
		if (!bCondition)
			throw std::runtime_error(pMessage);
	}

	void AppendPatterns(
		const std::vector<VALTAN_PATTERN_VIEW>& Source,
		std::map<std::string, const VALTAN_PATTERN_VIEW*, std::less<>>& Out)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Source)
		{
			Require(!Pattern.strPatternId.empty(),
				"canonical graph contains an empty pattern ID");
			Require(Out.emplace(Pattern.strPatternId, &Pattern).second,
				"canonical graph contains a duplicate pattern ID");
		}
	}

	void AppendInventoryGroup(
		const char* const pLabel,
		const std::vector<std::string>& PatternIds,
		const std::map<std::string,
			const VALTAN_PATTERN_VIEW*, std::less<>>& Patterns,
		std::vector<std::string>& OutAdmitted)
	{
		std::cout << "  " << pLabel << "(" << PatternIds.size() << "):";
		for (const std::string& PatternId : PatternIds)
		{
			Require(Patterns.contains(PatternId),
				"Complete Play inventory references an unknown pattern");
			Require(std::find(
				OutAdmitted.begin(), OutAdmitted.end(), PatternId) ==
				OutAdmitted.end(),
				"Complete Play inventory contains a duplicate pattern");
			OutAdmitted.push_back(PatternId);
			std::cout << ' ' << PatternId;
		}
		std::cout << '\n';
	}

	void VerifyCanonicalGraphInventoryAndFlow()
	{
		VALTAN_PATTERN_TREE_VIEW View;
		std::string Status;
		if (!CValtanPatternTree::Load(View, Status))
			throw std::runtime_error(Status);

		std::map<std::string,
			const VALTAN_PATTERN_VIEW*, std::less<>> Patterns;
		AppendPatterns(View.Gimmicks, Patterns);
		AppendPatterns(View.Rotation, Patterns);
		Require(!Patterns.empty() && Patterns.size() == View.Get_PatternCount(),
			"canonical graph pattern count does not match its stable-ID closure");
		Require(EXPECTED_ENCOUNTER_PATTERN_COUNT == Patterns.size(),
			"Encounter/reference pattern count is no longer the admitted 57-row closure");

		VALTAN_TOOL_AUDITION_INVENTORY Inventory;
		if (!CValtanPatternTree::Build_PlayablePatternInventory(
			View, Inventory, Status))
			throw std::runtime_error(Status);
		Require(EXPECTED_PRODUCT_PATTERN_COUNT == Inventory.Get_PatternCount(),
			"Complete Play did not admit exactly the 33 split Product patterns");
		for (const char* const pPatternId : REQUESTED_PRODUCT_PATTERN_IDS)
		{
			const auto Found = Patterns.find(pPatternId);
			Require(Patterns.end() != Found &&
				Found->second->bAuthoringMasterManaged &&
				Inventory.Contains(pPatternId),
				"requested Product pattern is not admitted to Complete Play");
		}
		for (const char* const pPatternId : REQUESTED_REFERENCE_ONLY_PATTERN_IDS)
		{
			const auto Found = Patterns.find(pPatternId);
			Require(Patterns.end() != Found &&
				!Found->second->bAuthoringMasterManaged &&
				!Inventory.Contains(pPatternId),
				"Encounter/reference-only row leaked into Complete Play");
		}

		/* The Workbench outcome selector is not a label-only branch. Exercise
		   the production graph resolver against the admitted Product graph and
		   require the counter occurrence to terminate in the authored groggy
		   stage. A pattern with no COUNTER_HIT edge must fail without replacing
		   the caller's previously staged path. */
		const VALTAN_PATTERN_VIEW& Trash = *Patterns.at("VALTAN_TRASH");
		std::vector<const VALTAN_STAGE_VIEW*> CounterPath;
		if (!CValtanPatternTree::Build_PreviewStagePath(
				Trash, VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY,
				CounterPath, Status))
		{
			throw std::runtime_error(Status);
		}
		Require(CounterPath.size() >= 2u &&
			"STEP_07" == CounterPath[CounterPath.size() - 2u]->strStageId &&
			"GROGGY" == CounterPath.back()->strStageId,
			"COUNTER_GROGGY did not select the admitted STEP_07 -> GROGGY edge");
		const std::vector<const VALTAN_STAGE_VIEW*> PreservedCounterPath =
			CounterPath;
		Require(!CValtanPatternTree::Build_PreviewStagePath(
				*Patterns.at("VALTAN_DASH_CHARGE"),
				VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY,
				CounterPath, Status) &&
			CounterPath == PreservedCounterPath,
			"COUNTER_GROGGY accepted a graph without COUNTER_HIT or replaced the previous path on failure");
		for (const auto& [PatternId, _] : Patterns)
			Require(std::string::npos == PatternId.find("SILENCE") &&
				std::string::npos == PatternId.find("STONE"),
				"silence/stone request must remain explicitly unimplemented until it owns a stable pattern");
		std::vector<std::string> AdmittedPatternIds;
		AdmittedPatternIds.reserve(Inventory.Get_PatternCount());

		std::cout << "Canonical Valtan graph: patterns=" <<
			View.Get_PatternCount() << " stages=" << View.Get_StageCount() << '\n';
		std::cout << "  patternIds(" << Patterns.size() << "):";
		for (const auto& [PatternId, _] : Patterns)
			std::cout << ' ' << PatternId;
		std::cout << "\nComplete Play inventory: total=" <<
			Inventory.Get_PatternCount() << '\n';
		AppendInventoryGroup(
			"core", Inventory.CorePatternIds, Patterns, AdmittedPatternIds);
		AppendInventoryGroup(
			"animator", Inventory.AnimatorPatternIds,
			Patterns, AdmittedPatternIds);
		AppendInventoryGroup(
			"derived", Inventory.DerivedPatternIds,
			Patterns, AdmittedPatternIds);

		std::vector<std::string> NextPatternIds;
		if (!CValtanPatternTree::Build_NextPatternInventory(
			View, NextPatternIds, Status))
			throw std::runtime_error(Status);
		std::vector<std::string> SortedAdmitted = AdmittedPatternIds;
		std::vector<std::string> SortedNext = NextPatternIds;
		std::sort(SortedAdmitted.begin(), SortedAdmitted.end());
		std::sort(SortedNext.begin(), SortedNext.end());
		Require(SortedAdmitted == SortedNext,
			"Next inventory does not close over the Complete Play inventory");

		CValtanPatternFlowDocument FlowDocument;
		if (!FlowDocument.Load(AdmittedPatternIds, Status))
			throw std::runtime_error(Status);
		Require(FlowDocument.Is_Ready(),
			"saved Flow did not enter the ready state");
		const VALTAN_PATTERN_FLOW_DEFINITION* const pFlow =
			FlowDocument.Get_DefaultFlow();
		Require(nullptr != pFlow && !pFlow->Slots.empty(),
			"default saved Flow has no selectable slots");
		Require(View.strSavedFlowSourceRevision ==
			FlowDocument.Get_SourceRevision(),
			"canonical graph and saved Flow revisions do not exact-join");

		std::set<std::string, std::less<>> SlotIds;
		std::cout << "Saved Flow: " << pFlow->strFlowId << " slots=" <<
			pFlow->Slots.size() << " interStepPursuitMs=" <<
			pFlow->iInterStepPursuitMs << '\n';
		for (const VALTAN_PATTERN_FLOW_SLOT& Slot : pFlow->Slots)
		{
			Require(!Slot.strSlotId.empty() &&
				SlotIds.insert(Slot.strSlotId).second,
				"saved Flow contains an empty or duplicate slot ID");
			Require(Inventory.Contains(Slot.strPatternId),
				"saved Flow slot is unavailable in Complete Play inventory");
			std::cout << "  " << Slot.strSlotId << " -> " <<
				Slot.strPatternId << " [available]\n";
		}
	}

	std::string ReadExactBytes(const std::filesystem::path& Path)
	{
		std::ifstream Input(Path, std::ios::binary);
		Require(static_cast<bool_t>(Input),
			"canonical Product interleaving oracle could not open a target");
		return std::string(
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
	}

	void VerifyCanonicalProductReadAdmissionExcludesWriter()
	{
		const std::filesystem::path LockPath =
			CProjectDataRoot::Get().parent_path() /
			L"out\\ValtanPatternTransactions\\create-pattern.lock";
		HANDLE hWriter = CreateFileW(
			LockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		Require(INVALID_HANDLE_VALUE != hWriter,
			"canonical Product interleaving oracle could not open writer lock");
		OVERLAPPED WriterOverlap{};
		const std::filesystem::path BindingsPath = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
			L"Valtan.patternbindings.json");
		const std::filesystem::path EffectsPath = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
			L"Valtan.patterneffectcues.json");
		try
		{
			{
				CValtanCanonicalProductReadAdmission Admission;
				std::string Status;
				if (!Admission.Acquire(Status))
					throw std::runtime_error(Status);
				const std::string OldBindings = ReadExactBytes(BindingsPath);
				const std::string OldEffects = ReadExactBytes(EffectsPath);
				Require(!OldBindings.empty() && !OldEffects.empty(),
					"canonical Product interleaving oracle read an empty target");

				SetLastError(ERROR_SUCCESS);
				const BOOL bWriterInterposed = LockFileEx(
					hWriter,
					LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
					0u, 1u, 0u, &WriterOverlap);
				Require(FALSE == bWriterInterposed &&
					ERROR_LOCK_VIOLATION == GetLastError(),
					"exclusive writer interposed between canonical component reads");
				Require(OldBindings == ReadExactBytes(BindingsPath) &&
					OldEffects == ReadExactBytes(EffectsPath),
					"canonical Product reader observed a mixed generation");
				if (!Admission.Validate_StillCurrent(Status))
					throw std::runtime_error(Status);
			}

			Require(FALSE != LockFileEx(
				hWriter,
				LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
				0u, 1u, 0u, &WriterOverlap),
				"exclusive writer did not acquire after canonical reader release");
			Require(FALSE != UnlockFileEx(
				hWriter, 0u, 1u, 0u, &WriterOverlap),
				"canonical Product interleaving oracle could not release writer lock");
			CloseHandle(hWriter);
		}
		catch (...)
		{
			CloseHandle(hWriter);
			throw;
		}
	}
}

int Run_ValtanCanonicalGraphContractTests()
{
	try
	{
		VerifyCanonicalProductReadAdmissionExcludesWriter();
		VerifyCanonicalGraphInventoryAndFlow();
		std::cout << "ValtanCanonicalGraphContractTests: 3/3 passed\n";
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "FAIL canonical Valtan graph/inventory/Flow: " <<
			Error.what() << '\n';
		return 1;
	}
}
