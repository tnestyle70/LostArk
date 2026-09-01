#include "Effect_Catalog.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternFlowDocument.h"
#include "ValtanPatternTree.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
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
	constexpr const char* REQUESTED_PRODUCT_PATTERN_IDS[] = {
		"VALTAN_HIGH_JUMP",
		"VALTAN_SIX_PIZZA_106",
		"VALTAN_WARP",
		"VALTAN_TRASH",
		"VALTAN_CATCH_BREATH",
		"VALTAN_STRUGGLING",
		"VALTAN_DASH_CHARGE",
		"VALTAN_GROUND_ROAR",
		"VALTAN_TRIPLE_COUNTER",
		"VALTAN_STAGGER_SLOT",
		"VALTAN_BIND_SLOT",
		"VALTAN_SILENCE_SLOT",
	};
	constexpr const char* REQUESTED_REFERENCE_ONLY_PATTERN_IDS[] = {
		"VALTAN_MAGIC_ORB_STAGGER_76",
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

	struct SIX_PIZZA_ELEMENT_EVIDENCE final
	{
		f32_t fStartDelaySeconds = 0.f;
		EFFECT_TRANSFORM_DESC LocalTransform{};
	};

	const DATA_JSON_VALUE& RequireJsonField(
		const DATA_JSON_VALUE& Parent,
		const char* const pKey,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* const pValue = Parent.Find(pKey);
		Require(nullptr != pValue && pValue->Get_Type() == eType,
			(std::string("six-pizza Effect source field is invalid: ") +
				pKey).c_str());
		return *pValue;
	}

	float3_t ReadJsonFloat3(
		const DATA_JSON_VALUE& Parent,
		const char* const pKey)
	{
		const DATA_JSON_VALUE& Value = RequireJsonField(
			Parent, pKey, DATA_JSON_TYPE::ARRAY);
		Require(3u == Value.Get_Array().size(),
			"six-pizza Effect source vector does not contain three values");
		float3_t Out{};
		f32_t* const pComponents[3]{ &Out.x, &Out.y, &Out.z };
		for (size_t i = 0u; i < 3u; ++i)
		{
			const DATA_JSON_VALUE& Component = Value.Get_Array()[i];
			Require(Component.Is_Number() &&
				std::isfinite(Component.Get_Number()),
				"six-pizza Effect source vector contains a non-finite value");
			*pComponents[i] = static_cast<f32_t>(Component.Get_Number());
		}
		return Out;
	}

	SIX_PIZZA_ELEMENT_EVIDENCE ReadSixPizzaElement(
		const DATA_JSON_VALUE& Root,
		const std::string_view ElementId)
	{
		const DATA_JSON_VALUE& Elements = RequireJsonField(
			Root, "elements", DATA_JSON_TYPE::ARRAY);
		const auto Found = std::find_if(
			Elements.Get_Array().begin(), Elements.Get_Array().end(),
			[ElementId](const DATA_JSON_VALUE& Element)
			{
				const DATA_JSON_VALUE* const pId = Element.Find("id");
				return nullptr != pId && pId->Is_String() &&
					pId->Get_String() == ElementId;
			});
		Require(Elements.Get_Array().end() != Found,
			(std::string("six-pizza composite lost element: ") +
				std::string(ElementId)).c_str());

		const DATA_JSON_VALUE& Attachment = RequireJsonField(
			*Found, "actionCueAttachment", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE& AttachmentEnabled = RequireJsonField(
			Attachment, "enabled", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE& Inheritance = RequireJsonField(
			*Found, "transformInheritance", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE& InheritanceEnabled = RequireJsonField(
			Inheritance, "enabled", DATA_JSON_TYPE::BOOLEAN);
		Require(!AttachmentEnabled.Get_Boolean() &&
			!InheritanceEnabled.Get_Boolean(),
			"six-pizza composite element escaped its shared cue root");

		const DATA_JSON_VALUE& Detail = RequireJsonField(
			*Found, "detail", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE& Timing = RequireJsonField(
			Detail, "timing", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE& Delay = RequireJsonField(
			Timing, "startDelaySeconds", DATA_JSON_TYPE::NUMBER);
		Require(std::isfinite(Delay.Get_Number()) && Delay.Get_Number() >= 0.0,
			"six-pizza composite element delay is invalid");
		const DATA_JSON_VALUE& Transform = RequireJsonField(
			Detail, "transform", DATA_JSON_TYPE::OBJECT);

		SIX_PIZZA_ELEMENT_EVIDENCE Out;
		Out.fStartDelaySeconds = static_cast<f32_t>(Delay.Get_Number());
		Out.LocalTransform.vPosition = ReadJsonFloat3(Transform, "position");
		Out.LocalTransform.vRotationDegrees = ReadJsonFloat3(
			Transform, "rotationDegrees");
		Out.LocalTransform.vScale = ReadJsonFloat3(Transform, "scale");
		return Out;
	}

	bool_t ReadSixPizzaParticleLocalSpace(
		const DATA_JSON_VALUE& Root,
		const std::string_view ElementId)
	{
		const DATA_JSON_VALUE& Elements = RequireJsonField(
			Root, "elements", DATA_JSON_TYPE::ARRAY);
		const auto Found = std::find_if(
			Elements.Get_Array().begin(), Elements.Get_Array().end(),
			[ElementId](const DATA_JSON_VALUE& Element)
			{
				const DATA_JSON_VALUE* const pId = Element.Find("id");
				return nullptr != pId && pId->Is_String() &&
					pId->Get_String() == ElementId;
			});
		Require(Elements.Get_Array().end() != Found,
			(std::string("six-pizza composite lost static sector: ") +
				std::string(ElementId)).c_str());
		const DATA_JSON_VALUE& Detail = RequireJsonField(
			*Found, "detail", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE& Particle = RequireJsonField(
			Detail, "particle", DATA_JSON_TYPE::OBJECT);
		return RequireJsonField(
			Particle, "localSpace", DATA_JSON_TYPE::BOOLEAN).Get_Boolean();
	}

	bool_t MatrixNearlyEquals(
		const float4x4_t& Left,
		const float4x4_t& Right,
		const f32_t fTolerance = 0.0001f)
	{
		const f32_t* const pLeft = &Left._11;
		const f32_t* const pRight = &Right._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			if (!std::isfinite(pLeft[i]) || !std::isfinite(pRight[i]) ||
				std::abs(pLeft[i] - pRight[i]) > fTolerance)
			{
				return false;
			}
		}
		return true;
	}

	float4x4_t ComposeElementWorld(
		const EFFECT_TRANSFORM_DESC& Local,
		const float4x4_t& CueRoot)
	{
		float4x4_t Out{};
		DirectX::XMStoreFloat4x4(&Out,
			DirectX::XMMatrixScaling(
				Local.vScale.x, Local.vScale.y, Local.vScale.z) *
			DirectX::XMMatrixRotationRollPitchYaw(
				DirectX::XMConvertToRadians(Local.vRotationDegrees.x),
				DirectX::XMConvertToRadians(Local.vRotationDegrees.y),
				DirectX::XMConvertToRadians(Local.vRotationDegrees.z)) *
			DirectX::XMMatrixTranslation(
				Local.vPosition.x, Local.vPosition.y, Local.vPosition.z) *
			DirectX::XMLoadFloat4x4(&CueRoot));
		return Out;
	}

	void VerifySixPizzaArenaTargetFollowRoot()
	{
		VALTAN_PATTERN_TREE_VIEW View;
		std::string Status;
		if (!CValtanPatternTree::Load(View, Status))
			throw std::runtime_error(Status);
		const auto FindPattern = [&View]() -> const VALTAN_PATTERN_VIEW*
		{
			for (const auto* const pRows : { &View.Gimmicks, &View.Rotation })
			{
				const auto Found = std::find_if(
					pRows->begin(), pRows->end(),
					[](const VALTAN_PATTERN_VIEW& Pattern)
					{
						return Pattern.strPatternId == "VALTAN_SIX_PIZZA_106";
					});
				if (pRows->end() != Found)
					return &*Found;
			}
			return nullptr;
		};
		const VALTAN_PATTERN_VIEW* const pPizza = FindPattern();
		Require(nullptr != pPizza && pPizza->ServerMotion.has_value() &&
			pPizza->strTargetPolicy == "LOCK_RANDOM_ALIVE_ON_START" &&
			pPizza->strAimPolicy == "TRACK_TARGET_EACH_TICK" &&
			pPizza->ServerMotion->strKind == "LEAP_TO_ANCHOR" &&
			pPizza->ServerMotion->bMoveToAnchorBeforeTakeoff,
			"six-pizza canonical pattern lost its locked-target tick-facing authority");

		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pCompositeCue = nullptr;
		for (const VALTAN_STAGE_VIEW& Stage : pPizza->Stages)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
			{
				if (Cue.strEffectAssetId !=
					"effect.valtan.project-tuned.sequence.six-pizza-106")
				{
					continue;
				}
				Require(nullptr == pCompositeCue,
					"six-pizza composite Effect is invoked more than once");
				pCompositeCue = &Cue;
			}
		}
		Require(nullptr != pCompositeCue &&
			pCompositeCue->strAnchorSlotId == "arena.center.target-follow" &&
			pCompositeCue->eFollowPolicy == EFFECT_FOLLOW_POLICY::FOLLOW &&
			pCompositeCue->eScalePolicy ==
				VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT &&
			pCompositeCue->vWorldScale.x == 1.5f &&
			pCompositeCue->vWorldScale.y == 1.5f &&
			pCompositeCue->vWorldScale.z == 1.5f &&
			pCompositeCue->LocalTransform.vPosition.x == 0.f &&
			pCompositeCue->LocalTransform.vPosition.y == 0.f &&
			pCompositeCue->LocalTransform.vPosition.z == 0.f &&
			pCompositeCue->LocalTransform.vRotationDegrees.x == 0.f &&
			pCompositeCue->LocalTransform.vRotationDegrees.y == 0.f &&
			pCompositeCue->LocalTransform.vRotationDegrees.z == 0.f &&
			pCompositeCue->LocalTransform.vScale.x == 1.f &&
			pCompositeCue->LocalTransform.vScale.y == 1.f &&
			pCompositeCue->LocalTransform.vScale.z == 1.f,
			"six-pizza composite cue did not retain its center target-follow root");

		const float3_t ArenaCenter{
			pPizza->ServerMotion->LandingPosition[0],
			pPizza->ServerMotion->LandingPosition[1],
			pPizza->ServerMotion->LandingPosition[2] };
		const float3_t FirstTickTarget{
			ArenaCenter.x + 12.f, ArenaCenter.y, ArenaCenter.z - 7.f };
		const float3_t SecondTickTarget{
			ArenaCenter.x - 5.f, ArenaCenter.y, ArenaCenter.z + 16.f };
		const auto TargetYaw = [&ArenaCenter](const float3_t& Target)
		{
			return std::atan2(
				Target.x - ArenaCenter.x,
				Target.z - ArenaCenter.z) *
				(180.f / 3.14159265358979323846f);
		};
		const f32_t fFirstTickYawDegrees = TargetYaw(FirstTickTarget);
		const f32_t fSecondTickYawDegrees = TargetYaw(SecondTickTarget);
		float4x4_t FirstTickAnchor{};
		Require(CValtanPatternEffectCueDocument::Try_BuildArenaCenterAnchor(
			pCompositeCue->strAnchorSlotId, ArenaCenter,
			fFirstTickYawDegrees, FirstTickAnchor),
			"runtime rejected the first admitted six-pizza target-follow yaw");
		float4x4_t SecondTickAnchor{};
		Require(CValtanPatternEffectCueDocument::Try_BuildArenaCenterAnchor(
			pCompositeCue->strAnchorSlotId, ArenaCenter,
			fSecondTickYawDegrees, SecondTickAnchor),
			"runtime rejected the second admitted six-pizza target-follow yaw");
		const f32_t fSecondRootYawDegrees = std::atan2(
			SecondTickAnchor._31, SecondTickAnchor._33) *
			(180.f / 3.14159265358979323846f);
		Require(std::abs(std::remainder(
			fSecondRootYawDegrees - fSecondTickYawDegrees, 360.f)) < 0.001f &&
			std::abs(SecondTickAnchor._41 - ArenaCenter.x) < 0.0001f &&
			std::abs(SecondTickAnchor._42 - ArenaCenter.y) < 0.0001f &&
			std::abs(SecondTickAnchor._43 - ArenaCenter.z) < 0.0001f &&
			!MatrixNearlyEquals(FirstTickAnchor, SecondTickAnchor),
			"arena.center.target-follow did not rotate at the fixed center on tick two");

		float4x4_t Preserved = SecondTickAnchor;
		const float4x4_t Baseline = Preserved;
		Require(!CValtanPatternEffectCueDocument::Try_BuildArenaCenterAnchor(
			"arena.center.unknown", ArenaCenter, fSecondTickYawDegrees, Preserved) &&
			MatrixNearlyEquals(Preserved, Baseline, 0.f) &&
			!CValtanPatternEffectCueDocument::Try_BuildArenaCenterAnchor(
				"arena.center.target-follow", ArenaCenter,
				(std::numeric_limits<f32_t>::quiet_NaN)(), Preserved) &&
			MatrixNearlyEquals(Preserved, Baseline, 0.f),
			"invalid target-follow yaw replaced the previously admitted root matrix");

		const std::filesystem::path EffectPath = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"Authored" /
			L"effect.valtan.project-tuned.sequence.six-pizza-106.effect.json");
		std::ifstream EffectInput(EffectPath, std::ios::binary);
		Require(static_cast<bool_t>(EffectInput),
			"six-pizza composite authored Effect source is missing");
		const std::string EffectText{
			std::istreambuf_iterator<char>(EffectInput),
			std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE EffectRoot;
		Require(CDataJson::Parse(EffectText, EffectRoot, Status),
			"six-pizza composite authored Effect source is malformed");
		const SIX_PIZZA_ELEMENT_EVIDENCE Landing = ReadSixPizzaElement(
			EffectRoot,
			"authored.copy.requested.20260827.terrain-3.landing.01.1");
		const SIX_PIZZA_ELEMENT_EVIDENCE Sector = ReadSixPizzaElement(
			EffectRoot, "requested.20260827.six-pizza.sector.yellow-05");
		const SIX_PIZZA_ELEMENT_EVIDENCE MirroredSector = ReadSixPizzaElement(
			EffectRoot,
			"authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1");
		const SIX_PIZZA_ELEMENT_EVIDENCE LateSector = ReadSixPizzaElement(
			EffectRoot,
			"authored.copy.authored.copy.authored.copy.authored.copy."
			"sprite_particle_8.1.1.1.1");
		const SIX_PIZZA_ELEMENT_EVIDENCE Finale = ReadSixPizzaElement(
			EffectRoot,
			"authored.copy.authored.copy.whirlwind.mesh.10.cyan.phase000.2.1");
		Require(Landing.fStartDelaySeconds > 5.f &&
			Sector.fStartDelaySeconds > Landing.fStartDelaySeconds &&
			MirroredSector.fStartDelaySeconds == Sector.fStartDelaySeconds &&
			LateSector.fStartDelaySeconds > Sector.fStartDelaySeconds &&
			Finale.fStartDelaySeconds > LateSector.fStartDelaySeconds,
			"six-pizza composite no longer contains ordered late-root elements");
		Require(ReadSixPizzaParticleLocalSpace(
				EffectRoot, "requested.20260827.six-pizza.sector.yellow-05") &&
			ReadSixPizzaParticleLocalSpace(EffectRoot,
				"authored.copy.authored.copy.authored.copy.sprite_particle_8.1.1.1") &&
			ReadSixPizzaParticleLocalSpace(EffectRoot,
				"authored.copy.authored.copy.authored.copy.authored.copy."
				"sprite_particle_8.1.1.1.1"),
			"all three static six-pizza sectors must inherit the mutable root");

		struct ROOT_TICK_EVIDENCE final
		{
			uint64_t iWorldRootHandle = 0u;
			float4x4_t CueRoot{};
		};
		const auto BuildCueRoot = [pCompositeCue](const float4x4_t& Anchor)
		{
			float4x4_t Root{};
			DirectX::XMStoreFloat4x4(&Root,
				DirectX::XMMatrixScaling(
					pCompositeCue->vWorldScale.x,
					pCompositeCue->vWorldScale.y,
					pCompositeCue->vWorldScale.z) *
				DirectX::XMLoadFloat4x4(&Anchor));
			return Root;
		};
		constexpr uint64_t SHARED_WORLD_ROOT_HANDLE = 77u;
		const ROOT_TICK_EVIDENCE FirstTickRoot{
			SHARED_WORLD_ROOT_HANDLE, BuildCueRoot(FirstTickAnchor) };
		const ROOT_TICK_EVIDENCE SecondTickRoot{
			SHARED_WORLD_ROOT_HANDLE, BuildCueRoot(SecondTickAnchor) };
		Require(FirstTickRoot.iWorldRootHandle ==
				SecondTickRoot.iWorldRootHandle &&
			!MatrixNearlyEquals(FirstTickRoot.CueRoot, SecondTickRoot.CueRoot),
			"two Server yaw ticks did not update one shared world-root handle");
		const float4x4_t LandingWorld = ComposeElementWorld(
			Landing.LocalTransform, FirstTickRoot.CueRoot);
		Require(MatrixNearlyEquals(LandingWorld, FirstTickRoot.CueRoot),
			"initial landing element did not use the shared composite root");
		const float4x4_t SectorWorld = ComposeElementWorld(
			Sector.LocalTransform, SecondTickRoot.CueRoot);
		const float4x4_t MirroredSectorWorld = ComposeElementWorld(
			MirroredSector.LocalTransform, SecondTickRoot.CueRoot);
		const float4x4_t LateSectorWorld = ComposeElementWorld(
			LateSector.LocalTransform, SecondTickRoot.CueRoot);
		const float4x4_t FinaleWorld = ComposeElementWorld(
			Finale.LocalTransform, SecondTickRoot.CueRoot);
		Require(std::isfinite(SectorWorld._41) &&
			std::isfinite(SectorWorld._43) &&
			std::isfinite(MirroredSectorWorld._41) &&
			std::isfinite(MirroredSectorWorld._43) &&
			std::isfinite(LateSectorWorld._41) &&
			std::isfinite(LateSectorWorld._43) &&
			std::isfinite(FinaleWorld._41) &&
			std::isfinite(FinaleWorld._43),
			"late six-pizza elements did not compose under the updated shared root");
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
		std::size_t iCrossPatternBranchCount = 0u;
		for (const auto& [PatternId, pPattern] : Patterns)
		{
			for (const VALTAN_STAGE_VIEW& Stage : pPattern->Stages)
			{
				for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
				{
					if (!Branch.strNextPatternId.has_value())
						continue;
					++iCrossPatternBranchCount;
					Require(!Branch.strNextActionId.has_value() &&
						Patterns.contains(*Branch.strNextPatternId),
						"canonical cross-Pattern branch is ambiguous or dangling");
				}
			}
		}
		Require(iCrossPatternBranchCount >= 3u,
			"canonical graph did not preserve the dash/groggy/part-break Pattern boundaries");
		const auto RequireCrossTarget = [&Patterns](
			const char* const pPatternId, const char* const pStageId,
			const char* const pOutcome, const char* const pTargetPatternId)
		{
			const VALTAN_PATTERN_VIEW& Pattern = *Patterns.at(pPatternId);
			const auto Stage = std::find_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[pStageId](const VALTAN_STAGE_VIEW& Candidate)
				{ return Candidate.strStageId == pStageId; });
			Require(Stage != Pattern.Stages.end(),
				"canonical cross-Pattern source Stage is missing");
			const auto Branch = std::find_if(
				Stage->Branches.begin(), Stage->Branches.end(),
				[pOutcome](const VALTAN_STAGE_BRANCH_VIEW& Candidate)
				{ return Candidate.strOutcome == pOutcome; });
			Require(Branch != Stage->Branches.end() &&
				!Branch->strNextActionId.has_value() &&
				Branch->strNextPatternId.has_value() &&
				*Branch->strNextPatternId == pTargetPatternId,
				"canonical cross-Pattern branch target drifted");
		};
		RequireCrossTarget(
			"VALTAN_DASH_CHARGE", "CHARGE", "WALL_CONTACT",
			"VALTAN_DASH_CHARGE_GROGGY");
		RequireCrossTarget(
			"VALTAN_DASH_CHARGE", "CHARGE", "TIMEOUT",
			"VALTAN_DASH_CHARGE_GROGGY");
		RequireCrossTarget(
			"VALTAN_DASH_CHARGE_GROGGY", "GROGGY", "PART_DESTROYED",
			"VALTAN_PART_BREAK");
		std::vector<const VALTAN_STAGE_VIEW*> DashWallBoundaryPath;
		if (!CValtanPatternTree::Build_PreviewStagePath(
				*Patterns.at("VALTAN_DASH_CHARGE"),
				VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
				DashWallBoundaryPath, Status))
		{
			throw std::runtime_error(Status);
		}
		Require(!DashWallBoundaryPath.empty() &&
			"CHARGE" == DashWallBoundaryPath.back()->strStageId,
			"local dash preview did not stop cleanly at its cross-Pattern boundary");
		std::set<std::string, std::less<>> ManagedPatternIds;
		std::set<std::string, std::less<>> ReferencePatternIds;
		for (const auto& [PatternId, pPattern] : Patterns)
			(pPattern->bAuthoringMasterManaged ? ManagedPatternIds :
				ReferencePatternIds).insert(PatternId);
		Require(!ManagedPatternIds.empty() &&
			!ReferencePatternIds.empty() &&
			ManagedPatternIds.size() + ReferencePatternIds.size() == Patterns.size(),
			"Encounter closure no longer partitions into managed and reference patterns");
		const VALTAN_PATTERN_VIEW& GroundRoar =
			*Patterns.at("VALTAN_GROUND_ROAR");
		Require(GroundRoar.bAuthoringMasterManaged &&
			GroundRoar.bManualServerAudition &&
			1u == GroundRoar.iAuthoringPhase &&
			"AUDITION_ONLY" == GroundRoar.strSelectionMode &&
			"sequence.400440.0" == GroundRoar.strSourceAnimationChainId &&
			"MANUAL_SERVER_AUDITION" == GroundRoar.strAdmissionState,
			"VALTAN_GROUND_ROAR lost its promoted manual authoring identity");

		VALTAN_TOOL_AUDITION_INVENTORY Inventory;
		if (!CValtanPatternTree::Build_PlayablePatternInventory(
			View, Inventory, Status))
			throw std::runtime_error(Status);
		Require(ManagedPatternIds.size() == Inventory.Get_PatternCount(),
			"Complete Play does not match the managed split Product patterns");
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
		for (const char* const pPatternId : {
			"VALTAN_STAGGER_SLOT", "VALTAN_BIND_SLOT", "VALTAN_SILENCE_SLOT" })
		{
			const VALTAN_PATTERN_VIEW& Skeleton = *Patterns.at(pPatternId);
			const std::size_t iExpectedStageCount =
				"VALTAN_STAGGER_SLOT" == std::string_view(pPatternId) ? 3u : 1u;
			Require(Skeleton.bAuthoringMasterManaged &&
				Skeleton.bManualServerAudition &&
				"DERIVED_SERVER_PATTERN" == Skeleton.strAdmissionState &&
				iExpectedStageCount == Skeleton.Stages.size(),
				"status-pattern authoring row lost its derived Product contract");
		}
		const auto ClipNames = [](const VALTAN_STAGE_VIEW& Stage)
		{
			std::vector<std::string> Names;
			Names.reserve(Stage.ClipOccurrences.size());
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence : Stage.ClipOccurrences)
				Names.push_back(Occurrence.strClipName);
			return Names;
		};
		const VALTAN_PATTERN_VIEW& Stagger = *Patterns.at("VALTAN_STAGGER_SLOT");
		const VALTAN_PATTERN_VIEW& Bind = *Patterns.at("VALTAN_BIND_SLOT");
		const VALTAN_PATTERN_VIEW& Silence = *Patterns.at("VALTAN_SILENCE_SLOT");
		Require(Stagger.Stages[0].bSuppressAnimation &&
			!Stagger.Stages[1].bSuppressAnimation &&
			Stagger.Stages[2].bSuppressAnimation &&
			ClipNames(Stagger.Stages[1]) == std::vector<std::string>{
				"mesh_abn_groggy_1_start", "mesh_abn_groggy_1_loop",
				"mesh_abn_groggy_1_loop", "mesh_abn_groggy_1_loop",
				"mesh_abn_groggy_1_end" } &&
			!Bind.Stages[0].bSuppressAnimation &&
			ClipNames(Bind.Stages[0]) == std::vector<std::string>{
				"mesh_att_battle_5_01_start", "mesh_att_battle_5_01_loop",
				"mesh_att_battle_5_01_loop", "mesh_att_battle_5_01_loop",
				"mesh_att_battle_5_01_end" } &&
			!Silence.Stages[0].bSuppressAnimation &&
			ClipNames(Silence.Stages[0]) == std::vector<std::string>{
				"mesh_evt1_att_battle_5_01_end" },
			"status-pattern NONE and saved animation occurrence contract drifted");
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
		const std::set<std::string, std::less<>> InventoryPatternIds(
			AdmittedPatternIds.begin(), AdmittedPatternIds.end());
		Require(InventoryPatternIds == ManagedPatternIds,
			"Complete Play IDs are not the exact managed canonical pattern closure");
		Require(Inventory.AnimatorPatternIds.end() != std::find(
				Inventory.AnimatorPatternIds.begin(),
				Inventory.AnimatorPatternIds.end(), "VALTAN_GROUND_ROAR") &&
			Inventory.CorePatternIds.end() == std::find(
				Inventory.CorePatternIds.begin(),
				Inventory.CorePatternIds.end(), "VALTAN_GROUND_ROAR") &&
			Inventory.DerivedPatternIds.end() == std::find(
				Inventory.DerivedPatternIds.begin(),
				Inventory.DerivedPatternIds.end(), "VALTAN_GROUND_ROAR"),
			"VALTAN_GROUND_ROAR is not exclusively in the Animator inventory bucket");

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
		if (!FlowDocument.Load_CanonicalSequence(
				View.strScriptedSequenceId,
				View.strScriptedSequenceMode,
				View.iScriptedSequenceInterStepPursuitMs,
				View.ScriptedSequencePatternIds,
				AdmittedPatternIds, Status))
			throw std::runtime_error(Status);
		Require(FlowDocument.Is_Ready(),
			"canonical sequence adapter did not enter the ready state");
		const VALTAN_PATTERN_FLOW_DEFINITION* const pFlow =
			FlowDocument.Get_DefaultFlow();
		Require(nullptr != pFlow && !pFlow->Slots.empty(),
			"canonical sequence adapter has no selectable slots");
		Require(pFlow->Slots.size() ==
				View.ScriptedSequencePatternIds.size() &&
			std::equal(
				pFlow->Slots.begin(), pFlow->Slots.end(),
				View.ScriptedSequencePatternIds.begin(),
				[](const VALTAN_PATTERN_FLOW_SLOT& Slot,
					const std::string& PatternId)
				{ return Slot.strPatternId == PatternId; }),
			"canonical sequence adapter drifted from gameplay order");

		std::set<std::string, std::less<>> SlotIds;
		std::cout << "Canonical sequence adapter: " << pFlow->strFlowId << " slots=" <<
			pFlow->Slots.size() << " interStepPursuitMs=" <<
			pFlow->iInterStepPursuitMs << '\n';
		for (const VALTAN_PATTERN_FLOW_SLOT& Slot : pFlow->Slots)
		{
			Require(!Slot.strSlotId.empty() &&
				SlotIds.insert(Slot.strSlotId).second,
				"canonical sequence contains an empty or duplicate projected slot ID");
			Require(Inventory.Contains(Slot.strPatternId),
				"canonical sequence Pattern is unavailable in Complete Play inventory");
			std::cout << "  " << Slot.strSlotId << " -> " <<
				Slot.strPatternId << " [available]\n";
		}
	}

	void VerifyFlowDocumentV2AuthoringContract()
	{
		std::string Status;
		const std::vector<std::string> SequencePatternIds{
			"VALTAN_WHIRLWIND", "VALTAN_DASH_CHARGE",
			"VALTAN_DASH_CHARGE", "VALTAN_FOUR_SLASH",
			"VALTAN_HIGH_JUMP", "VALTAN_COUNTER",
			"VALTAN_WARP", "VALTAN_TRASH" };
		std::vector<std::string> AdmittedPatternIds = SequencePatternIds;
		std::sort(AdmittedPatternIds.begin(), AdmittedPatternIds.end());
		AdmittedPatternIds.erase(std::unique(
			AdmittedPatternIds.begin(), AdmittedPatternIds.end()),
			AdmittedPatternIds.end());
		AdmittedPatternIds.push_back("VALTAN_ENTRANCE_CINEMATIC");

		CValtanPatternFlowDocument Document;
		Require(Document.Load_CanonicalSequence(
				"sequence.valtan.server-authored.v1",
				"ORDERED_ONCE_THEN_IDLE", 1000u,
				SequencePatternIds, AdmittedPatternIds, Status),
			"canonical sequence adapter did not load");
		const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT Canonical =
			Document.Get_Draft();
		Require(2u == Canonical.iFormatVersion && 1u == Canonical.Flows.size(),
			"canonical sequence adapter header was not admitted");
		const VALTAN_PATTERN_FLOW_DEFINITION& Flow = Canonical.Flows.front();
		Require(Flow.Nodes.size() == SequencePatternIds.size() &&
			Flow.Edges.size() + 1u == Flow.Nodes.size() &&
			255u == Flow.iMaxTransitionsPerRun &&
			Flow.strEntryNodeId == Flow.Nodes.front().strNodeId &&
			std::equal(
				Flow.Nodes.begin(), Flow.Nodes.end(),
				SequencePatternIds.begin(),
				[](const VALTAN_PATTERN_FLOW_NODE& Node,
					const std::string& PatternId)
				{ return Node.strPatternId == PatternId; }),
			"canonical sequence adapter changed order or topology");
		Require(CValtanPatternFlowDocument::Has_LegacyLinearProjection(Flow) &&
			Flow.Slots.size() == Flow.Nodes.size(),
			"canonical sequence adapter did not expose its ordered projection");
		for (const VALTAN_PATTERN_FLOW_NODE& Node : Flow.Nodes)
			Require(0u == Node.iWatchdogMs,
				"migrated v2 node unexpectedly enabled a watchdog");

		const std::string Serialized =
			CValtanPatternFlowDocument::Serialize_Text(Canonical);
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT RoundTrip;
		Require(CValtanPatternFlowDocument::Parse_Text(
			Serialized, RoundTrip, Status) &&
			CValtanPatternFlowDocument::Validate(
				RoundTrip, AdmittedPatternIds, Status) &&
			RoundTrip == Canonical,
			"v2 Flow serialize/reparse roundtrip drifted");

		const auto Reject = [&](
			const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& Candidate,
			const char* const pMessage)
		{
			Require(!CValtanPatternFlowDocument::Validate(
				Candidate, AdmittedPatternIds, Status), pMessage);
		};

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT Dangling = Canonical;
		Dangling.Flows.front().Edges.front().strToNodeId =
			"flow.valtan.boss-tool.default.node.999999";
		Reject(Dangling, "v2 Flow admitted a dangling edge");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT NonDeterministic = Canonical;
		VALTAN_PATTERN_FLOW_DEFINITION& NonDeterministicFlow =
			NonDeterministic.Flows.front();
		NonDeterministicFlow.Edges.push_back({
			"flow.valtan.boss-tool.default.edge.000029",
			NonDeterministicFlow.Edges.front().strFromNodeId,
			VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED,
			NonDeterministicFlow.Nodes.back().strNodeId, 1000u, std::nullopt });
		NonDeterministicFlow.iNextEdgeOrdinal = 30u;
		Reject(NonDeterministic,
			"v2 Flow admitted two COMPLETED edges from one node");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT Unreachable = Canonical;
		Unreachable.Flows.front().Edges.erase(
			Unreachable.Flows.front().Edges.begin() + 5u);
		Reject(Unreachable, "v2 Flow admitted an unreachable suffix");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT UnboundedCycle = Canonical;
		VALTAN_PATTERN_FLOW_DEFINITION& UnboundedFlow =
			UnboundedCycle.Flows.front();
		UnboundedFlow.Edges.push_back({
			"flow.valtan.boss-tool.default.edge.000029",
			UnboundedFlow.Nodes.back().strNodeId,
			VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED,
			UnboundedFlow.strEntryNodeId, 1000u, std::nullopt });
		UnboundedFlow.iNextEdgeOrdinal = 30u;
		Reject(UnboundedCycle, "v2 Flow admitted an unbounded cycle");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT BoundedCycle = UnboundedCycle;
		BoundedCycle.Flows.front().Edges.back().iMaxTraversals = 2u;
		Require(CValtanPatternFlowDocument::Validate(
			BoundedCycle, AdmittedPatternIds, Status),
			"v2 Flow rejected a finite cycle-closing back-edge");
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT PrematureWatchdog = BoundedCycle;
		PrematureWatchdog.Flows.front().iMaxTransitionsPerRun = 5u;
		Reject(PrematureWatchdog,
			"v2 Flow admitted a transition watchdog before its authored terminal");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT ForwardCap = Canonical;
		ForwardCap.Flows.front().Edges.front().iMaxTraversals = 2u;
		Reject(ForwardCap,
			"v2 Flow admitted maxTraversals on a non-back-edge");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT BadEntrance = Canonical;
		BadEntrance.Flows.front().Nodes[1u].strPatternId =
			"VALTAN_ENTRANCE_CINEMATIC";
		Reject(BadEntrance,
			"v2 Flow admitted an entrance cinematic outside entryNodeId");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT BadOrdinal = Canonical;
		BadOrdinal.Flows.front().iNextNodeOrdinal = 8u;
		Reject(BadOrdinal, "v2 Flow admitted a reused node ordinal");
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT DuplicateOrdinal = Canonical;
		DuplicateOrdinal.Flows.front().Nodes[1u].strNodeId =
			"flow.valtan.boss-tool.default.node.000001";
		DuplicateOrdinal.Flows.front().Edges.front().strToNodeId =
			DuplicateOrdinal.Flows.front().Nodes[1u].strNodeId;
		DuplicateOrdinal.Flows.front().Edges[1u].strFromNodeId =
			DuplicateOrdinal.Flows.front().Nodes[1u].strNodeId;
		Reject(DuplicateOrdinal,
			"v2 Flow admitted the same ordinal in legacy and new node namespaces");

		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT BadWatchdog = Canonical;
		BadWatchdog.Flows.front().Nodes.front().iWatchdogMs = 999u;
		Reject(BadWatchdog, "v2 Flow admitted a sub-minimum node watchdog");
		BadWatchdog.Flows.front().Nodes.front().iWatchdogMs = 1000u;
		Require(CValtanPatternFlowDocument::Validate(
			BadWatchdog, AdmittedPatternIds, Status),
			"v2 Flow rejected the minimum enabled node watchdog");
		Require(Document.Set_NodeWatchdogMs(
			Flow.Nodes.front().strNodeId, 1000u, AdmittedPatternIds, Status),
			"v2 Flow document rejected a valid node watchdog mutation");
		Require(nullptr != Document.Get_DefaultFlow() &&
			!CValtanPatternFlowDocument::Has_LegacyLinearProjection(
				*Document.Get_DefaultFlow()),
			"watchdog graph remained exposed as a legacy-playable slot projection");

		CValtanPatternFlowDocument GraphEditor;
		Require(GraphEditor.Load_CanonicalSequence(
				"sequence.valtan.server-authored.v1",
				"ORDERED_ONCE_THEN_IDLE", 1000u,
				SequencePatternIds, AdmittedPatternIds, Status),
			"v2 graph editor contract could not stage the canonical sequence");
		const VALTAN_PATTERN_FLOW_DEFINITION* EditorFlow =
			GraphEditor.Get_DefaultFlow();
		Require(nullptr != EditorFlow && !EditorFlow->Nodes.empty(),
			"v2 graph editor contract loaded no nodes");
		const std::size_t OriginalNodeCount = EditorFlow->Nodes.size();
		const std::string OriginalEntryId = EditorFlow->strEntryNodeId;
		const std::string InsertAfterId = EditorFlow->Nodes.back().strNodeId;
		const std::string RepeatTargetId = EditorFlow->Nodes[1u].strNodeId;
		const std::string InsertPatternId =
			EditorFlow->Nodes[1u].strPatternId;
		std::string InsertedNodeId;
		Require(GraphEditor.Insert_Node_After(
			InsertAfterId, InsertPatternId, AdmittedPatternIds,
			InsertedNodeId, Status),
			"v2 graph editor rejected a valid terminal node insertion");
		EditorFlow = GraphEditor.Get_DefaultFlow();
		Require(nullptr != EditorFlow &&
			OriginalNodeCount + 1u == EditorFlow->Nodes.size() &&
			CValtanPatternFlowDocument::Has_LegacyLinearProjection(*EditorFlow),
			"v2 graph editor insertion did not preserve its linear projection");

		std::string RepeatEdgeId;
		Require(GraphEditor.Connect_CompletedEdge(
			InsertedNodeId, RepeatTargetId,
			EditorFlow->iDefaultPursuitMs, 2u,
			AdmittedPatternIds, RepeatEdgeId, Status),
			"v2 graph editor rejected a finite terminal back-edge");
		EditorFlow = GraphEditor.Get_DefaultFlow();
		Require(nullptr != EditorFlow &&
			!CValtanPatternFlowDocument::Has_LegacyLinearProjection(*EditorFlow),
			"finite graph editor back-edge remained legacy-playable");
		Require(GraphEditor.Set_EdgePursuitMs(
			RepeatEdgeId, 1200u, AdmittedPatternIds, Status) &&
			GraphEditor.Set_EdgeMaxTraversals(
				RepeatEdgeId, 3u, AdmittedPatternIds, Status) &&
			GraphEditor.Set_NodeWatchdogMs(
				InsertedNodeId, 1000u, AdmittedPatternIds, Status),
			"v2 graph editor rejected typed edge or watchdog tuning");
		const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT BeforeInvalidDelete =
			GraphEditor.Get_Draft();
		Require(!GraphEditor.Remove_Edge(
			BeforeInvalidDelete.Flows.front().Edges.front().strEdgeId,
			AdmittedPatternIds, Status) &&
			GraphEditor.Get_Draft() == BeforeInvalidDelete,
			"v2 graph editor partially committed an orphaning edge deletion");
		Require(GraphEditor.Remove_Edge(
			RepeatEdgeId, AdmittedPatternIds, Status) &&
			GraphEditor.Set_NodeWatchdogMs(
				InsertedNodeId, 0u, AdmittedPatternIds, Status),
			"v2 graph editor could not remove its repeat edge and watchdog");
		EditorFlow = GraphEditor.Get_DefaultFlow();
		Require(nullptr != EditorFlow &&
			CValtanPatternFlowDocument::Has_LegacyLinearProjection(*EditorFlow),
			"v2 graph editor did not restore the linear projection");
		Require(GraphEditor.Set_EntryNode(
			RepeatTargetId, AdmittedPatternIds, Status),
			"v2 graph editor could not rotate its first acyclic start");
		Require(GraphEditor.Remove_Node(
			OriginalEntryId, AdmittedPatternIds, Status),
			"v2 graph editor could not remove and rejoin a non-terminal node");
		EditorFlow = GraphEditor.Get_DefaultFlow();
		Require(nullptr != EditorFlow && EditorFlow->Nodes.size() >= 5u,
			"v2 graph editor lost its deterministic path after entry removal");
		const std::string RotatedEntryId = EditorFlow->Nodes[4u].strNodeId;
		Require(GraphEditor.Set_EntryNode(
			RotatedEntryId, AdmittedPatternIds, Status),
			"v2 graph editor could not rotate an acyclic Flow start");
		EditorFlow = GraphEditor.Get_DefaultFlow();
		Require(nullptr != EditorFlow &&
			EditorFlow->strEntryNodeId == RotatedEntryId &&
			CValtanPatternFlowDocument::Validate(
				GraphEditor.Get_Draft(), AdmittedPatternIds, Status),
			"v2 graph editor start rotation did not remain strictly admitted");

		std::string WrongVersion = Serialized;
		const std::string VersionToken = "\"formatVersion\": 2";
		WrongVersion.replace(
			WrongVersion.find(VersionToken), VersionToken.size(),
			"\"formatVersion\": 1");
		VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT Preserved = RoundTrip;
		Require(!CValtanPatternFlowDocument::Parse_Text(
			WrongVersion, RoundTrip, Status) && RoundTrip == Preserved,
			"v2 parse failure replaced the caller's admitted document");
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
		VerifyFlowDocumentV2AuthoringContract();
		std::cout << "ValtanPatternFlowDocumentContractTests: PASS\n";
		VerifyCanonicalProductReadAdmissionExcludesWriter();
		VerifySixPizzaArenaTargetFollowRoot();
		VerifyCanonicalGraphInventoryAndFlow();
		std::cout << "ValtanCanonicalGraphContractTests: 4/4 passed\n";
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "FAIL canonical Valtan graph/inventory/Flow: " <<
			Error.what() << '\n';
		return 1;
	}
}
