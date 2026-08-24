#include "imgui.h"

#include "Effect_Tool.h"

#include "ActionPresentationTimeline.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "DataJson.h"
#include "EffectAuthoringTransfer.h"
#include "Effect_Artist31470ShaderRegistry.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DocumentRenderer.h"
#include "Effect_DirectAuthoredSourceIndex.h"
#include "ValtanPatternEffectCueDocument.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "Effect_ReconstructedExecution.h"
#include "Effect_RuntimeAuthority.h"
#include "Effect_ThumbnailCache.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_Artist.h"
#include "Logic_DimensionMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_LanceMaster.h"
#include "Logic_Slayer.h"
#include "Logic_Warlord.h"
#include "Model.h"
#include "NetworkManager.h"
#include "Profiler.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace
{
    constexpr const wchar_t* PREVIEW_LAYER = L"Layer_EffectPreview";
	constexpr uint32_t ARTIST_F_CORE_SKILL_ID = 31470u;
	constexpr const char_t* ARTIST_F_VISUAL_PROGRAM_ASSET_ID =
		"effect.artist.skill.31470";
	constexpr const char_t* ARTIST_F_UNIFIED_EFFECT_ASSET_ID =
		"effect.artist.skill.31470.unified";
	constexpr const char_t* ARTIST_F_SOURCE_AUTHORING_OVERLAY_PATH =
		"Effects/AuthoredCorrections/Artist/"
		"effect.artist.skill.31470.source-authoring-overlay.json";
	constexpr uint32_t DIMENSION_MASTER_T_SKILL_ID = 2050500u;
	constexpr const char_t* DIMENSION_MASTER_T_BASELINE_EFFECT_ASSET_ID =
		"effect.dimensionmaster.skill.2050500.authored-baseline";
	constexpr const char_t* DIMENSION_MASTER_T_SOURCE_EFFECT_ASSET_ID =
		"effect.dimensionmaster.skill.2050500";
	constexpr const char_t* DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID =
		"effect.dimensionmaster.skill.2050500.unified";
	constexpr const char_t* VALTAN_EXACT_HISTORY_BINDING_ID =
		"valtan.whirlwind.420633.active";
	constexpr const char_t* VALTAN_EXACT_HISTORY_EFFECT_ASSET_ID =
		"effect.valtan.pattern.420633.active";
	constexpr const char_t* VALTAN_CHARACTER_SELECT_BOSS_PLACEMENT_ID =
		"boss.valtan.character-select.lazy";
	constexpr const char_t* VALTAN_ARENA_BOSS_PLACEMENT_ID =
		"boss.valtan.center";
	constexpr std::array<std::string_view, 6u>
		VALTAN_CHARACTER_SELECT_ENVIRONMENT_PATTERN_IDS =
	{
		"VALTAN_ARMOR_BREAK_OPENING",
		"VALTAN_ENTRANCE_WHIRLWIND",
		"VALTAN_ARENA_BREAK_109",
		"VALTAN_ARENA_BREAK_84",
		"VALTAN_ARENA_BREAK_33",
		"VALTAN_FOUR_PILLARS_105"
	};

	const char_t* Resolve_ValtanServerPatternBossPlacement(
		const uint32_t iLevel)
	{
		if (ETOUI(Client::LEVEL::CHARACTER_SELECT) == iLevel)
			return VALTAN_CHARACTER_SELECT_BOSS_PLACEMENT_ID;
		if (ETOUI(Client::LEVEL::VALTAN_ARENA) == iLevel)
			return VALTAN_ARENA_BOSS_PLACEMENT_ID;
		return nullptr;
	}

	bool_t Is_ValtanCharacterSelectEnvironmentPattern(
		const std::string_view strPatternId)
	{
		return std::find(
			VALTAN_CHARACTER_SELECT_ENVIRONMENT_PATTERN_IDS.begin(),
			VALTAN_CHARACTER_SELECT_ENVIRONMENT_PATTERN_IDS.end(),
			strPatternId) !=
			VALTAN_CHARACTER_SELECT_ENVIRONMENT_PATTERN_IDS.end();
	}

	const char_t* Tool_PlayerStanceLabel(
		const LostArk::Shared::PLAYER_STANCE_ID eStance)
	{
		using LostArk::Shared::PLAYER_STANCE_ID;
		switch (eStance)
		{
		case PLAYER_STANCE_ID::NONE: return "NONE";
		case PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR:
			return "LANCE_MASTER_LONG_SPEAR";
		case PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR:
			return "LANCE_MASTER_SHORT_SPEAR";
		case PLAYER_STANCE_ID::WARLORD_NORMAL: return "WARLORD_NORMAL";
		case PLAYER_STANCE_ID::WARLORD_DEFENSE: return "WARLORD_DEFENSE";
		case PLAYER_STANCE_ID::END:
		default: return "INVALID";
		}
	}

	std::string Tool_SkillIdentitySuffix(
		const Client::PLAYER_SKILL_DEFINITION& Skill)
	{
		return " | #" + std::to_string(Skill.iSkillId) + " | Stance " +
			Tool_PlayerStanceLabel(Skill.eRequiredStance);
	}

	struct TOOL_SOURCE_ANCHOR_REQUEST final
	{
		std::string strRuntimeAnchorSlotId;
		std::string strRuntimeBoneName;
		Client::EFFECT_TRANSFORM_DESC SocketLocalTransform{};
	};

	std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Collect_ToolSourceAnchorRequests(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Requests;
		const auto AddRequest = [&Requests](TOOL_SOURCE_ANCHOR_REQUEST Request)
		{
			if (Request.strRuntimeAnchorSlotId.empty() ||
				Request.strRuntimeBoneName.empty())
			{
				return;
			}
			const auto Existing = std::find_if(Requests.begin(), Requests.end(),
				[&Request](const TOOL_SOURCE_ANCHOR_REQUEST& Value)
				{
					return Value.strRuntimeAnchorSlotId ==
						Request.strRuntimeAnchorSlotId;
				});
			if (Existing == Requests.end())
				Requests.push_back(std::move(Request));
		};

		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (Element.bVisible &&
				Element.ActionCueAttachment.bEnabled &&
				Element.ActionCueAttachment.bFollow)
			{
				AddRequest({
					Element.ActionCueAttachment.strRuntimeAnchorSlotId,
					Element.ActionCueAttachment.strRuntimeBoneName,
					Element.ActionCueAttachment.SocketLocalTransform });
			}
		}
		std::sort(Requests.begin(), Requests.end(),
			[](const TOOL_SOURCE_ANCHOR_REQUEST& Left,
				const TOOL_SOURCE_ANCHOR_REQUEST& Right)
			{
				return Left.strRuntimeAnchorSlotId < Right.strRuntimeAnchorSlotId;
			});
		return Requests;
	}

	bool Resolve_ToolSourceAnchorWorlds(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::unordered_map<std::string, float4x4_t>& OutWorlds,
		std::string& strOutError)
	{
		OutWorlds.clear();
		const std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Requests =
			Collect_ToolSourceAnchorRequests(Document);
		OutWorlds.reserve(Requests.size());
		bool_t bAllResolved = true;
		for (const TOOL_SOURCE_ANCHOR_REQUEST& Request : Requests)
		{
			float4x4_t BoneAnchorWorld{};
			if (!Client::CAnimationTargetService::Resolve_AnchorTransform(
					Request.strRuntimeBoneName.c_str(), &BoneAnchorWorld))
			{
				if (strOutError.empty())
				{
					strOutError = "preview model cannot resolve source slot '" +
						Request.strRuntimeAnchorSlotId + "' from bone '" +
						Request.strRuntimeBoneName + "'.";
				}
				bAllResolved = false;
				continue;
			}

			const Client::EFFECT_TRANSFORM_DESC& Local =
				Request.SocketLocalTransform;
			const matrix_t SocketLocal = XMMatrixScaling(
				Local.vScale.x, Local.vScale.y, Local.vScale.z) *
				XMMatrixRotationRollPitchYaw(
					XMConvertToRadians(Local.vRotationDegrees.x),
					XMConvertToRadians(Local.vRotationDegrees.y),
					XMConvertToRadians(Local.vRotationDegrees.z)) *
				XMMatrixTranslation(
					Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
			float4x4_t World{};
			XMStoreFloat4x4(&World,
				SocketLocal * XMLoadFloat4x4(&BoneAnchorWorld));
			OutWorlds.emplace(Request.strRuntimeAnchorSlotId, World);
		}
		if (bAllResolved)
			strOutError.clear();
		return bAllResolved;
	}

	bool Is_CompilerOwnedPortableRecipe(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Element.SourceRecipe.bEnabled && !Document.bSourceContract &&
			Element.SourceRecipe.strSourceContractProfileId.empty() &&
			Element.SourceRecipe.strSourceContractSha256.empty() &&
			Element.SourceRecipe.strSourceGraphSha256.empty();
	}

	bool Try_ParseMaterialExecutionLaneSlotId(
		const std::string_view strSlotId,
		std::string_view& strOutLaneId)
	{
		return Client::Try_ParseEffectMaterialExecutionLaneStableSlotId(
			strSlotId, strOutLaneId);
	}

	std::string MaterialExecutionLaneSlotId(const std::string& strLaneId)
	{
		return Client::Build_EffectMaterialExecutionLaneStableSlotId(strLaneId);
	}

	Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC* Find_MaterialExecutionLane(
		Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strLaneId;
		if (!Try_ParseMaterialExecutionLaneSlotId(strSlotId, strLaneId))
			return nullptr;
		const auto Iterator = std::find_if(
			Element.Material.Execution.TextureLanes.begin(),
			Element.Material.Execution.TextureLanes.end(),
			[strLaneId](
				const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane)
			{ return Lane.strLaneId == strLaneId; });
		return Iterator == Element.Material.Execution.TextureLanes.end() ?
			nullptr : &*Iterator;
	}

	Client::EFFECT_NAMED_TEXTURE_DESC* Find_SourceMaterialTexture(
		Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strName;
		if (!Client::Try_ParseEffectSourceMaterialTextureStableSlotId(
				strSlotId, strName))
		{
			return nullptr;
		}
		const auto Iterator = std::find_if(
			Element.Material.SourceMaterial.Textures.begin(),
			Element.Material.SourceMaterial.Textures.end(),
			[strName](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{ return Texture.strName == strName; });
		return Iterator == Element.Material.SourceMaterial.Textures.end() ?
			nullptr : &*Iterator;
	}

	const Client::EFFECT_NAMED_TEXTURE_DESC* Find_SourceMaterialTexture(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strName;
		if (!Client::Try_ParseEffectSourceMaterialTextureStableSlotId(
				strSlotId, strName))
		{
			return nullptr;
		}
		const auto Iterator = std::find_if(
			Element.Material.SourceMaterial.Textures.begin(),
			Element.Material.SourceMaterial.Textures.end(),
			[strName](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{ return Texture.strName == strName; });
		return Iterator == Element.Material.SourceMaterial.Textures.end() ?
			nullptr : &*Iterator;
	}

	bool Reset_AllAuthoringOverrides(
		Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError)
	{
		Client::EFFECT_ELEMENT_DESC Staged = Element;
		std::vector<std::string> ResourceSlots;
		std::vector<std::string> Scalars;
		std::vector<std::string> Colors;
		ResourceSlots.reserve(Staged.AuthoringOverrides.ResourceBindings.size());
		Scalars.reserve(Staged.AuthoringOverrides.Scalars.size());
		Colors.reserve(Staged.AuthoringOverrides.Colors.size());
		for (const Client::EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override :
			Staged.AuthoringOverrides.ResourceBindings)
		{
			ResourceSlots.push_back(Override.strSlotId);
		}
		for (const Client::EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Override :
			Staged.AuthoringOverrides.Scalars)
		{
			Scalars.push_back(Override.strName);
		}
		for (const Client::EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Override :
			Staged.AuthoringOverrides.Colors)
		{
			Colors.push_back(Override.strName);
		}
		for (const std::string& strSlotId : ResourceSlots)
		{
			if (!Client::CEffectDocumentCodec::Reset_AuthoringResourceOverride(
					Staged, strSlotId, strOutError))
			{
				return false;
			}
		}
		for (const std::string& strName : Scalars)
		{
			if (!Client::CEffectDocumentCodec::Reset_AuthoringScalarOverride(
					Staged, strName, strOutError))
			{
				return false;
			}
		}
		for (const std::string& strName : Colors)
		{
			if (!Client::CEffectDocumentCodec::Reset_AuthoringColorOverride(
					Staged, strName, strOutError))
			{
				return false;
			}
		}
		Element = std::move(Staged);
		strOutError.clear();
		return true;
	}

	const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC*
	Find_MaterialExecutionLane(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		std::string_view strLaneId;
		if (!Try_ParseMaterialExecutionLaneSlotId(strSlotId, strLaneId))
			return nullptr;
		const auto Iterator = std::find_if(
			Element.Material.Execution.TextureLanes.begin(),
			Element.Material.Execution.TextureLanes.end(),
			[strLaneId](
				const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane)
			{ return Lane.strLaneId == strLaneId; });
		return Iterator == Element.Material.Execution.TextureLanes.end() ?
			nullptr : &*Iterator;
	}

	const char* MaterialExecutionBackendLabel(
		const Client::EFFECT_MATERIAL_EXECUTION_BACKEND eBackend)
	{
		switch (eBackend)
		{
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC:
			return "Generic";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2:
			return "RuntimeMaterialV2";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4:
			return "ArtistVisualV4";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL:
			return "LocalDecal";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1:
			return "StandardColorV1";
		case Client::EFFECT_MATERIAL_EXECUTION_BACKEND::END:
		default:
			return "Invalid";
		}
	}

	const char* MaterialTextureColorSpaceLabel(
		const Client::EFFECT_TEXTURE_COLOR_SPACE eColorSpace)
	{
		switch (eColorSpace)
		{
		case Client::EFFECT_TEXTURE_COLOR_SPACE::LINEAR:
			return "Linear";
		case Client::EFFECT_TEXTURE_COLOR_SPACE::SRGB:
			return "sRGB";
		case Client::EFFECT_TEXTURE_COLOR_SPACE::END:
		default:
			return "Invalid";
		}
	}

	const char* MaterialTextureFilterLabel(
		const Client::EFFECT_MATERIAL_TEXTURE_FILTER eFilter)
	{
		switch (eFilter)
		{
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::POINT:
			return "Point";
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::LINEAR:
			return "Linear";
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::ANISOTROPIC:
			return "Anisotropic";
		case Client::EFFECT_MATERIAL_TEXTURE_FILTER::END:
		default:
			return "Invalid";
		}
	}

	bool Try_ResolveArtistCoreFamily(
		const Client::EFFECT_RUNTIME_RENDERER_KIND eRenderer,
		Client::EFFECT_GPU_RENDER_FAMILY& eOutFamily)
	{
		switch (eRenderer)
		{
		case Client::EFFECT_RUNTIME_RENDERER_KIND::MESH_PARTICLE:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::MESH;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::SPRITE_PARTICLE:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::SPRITE;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::DECAL_PARTICLE:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::DECAL;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::CASCADE_RIBBON:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::RIBBON;
			return true;
		case Client::EFFECT_RUNTIME_RENDERER_KIND::SCREEN_POST:
		case Client::EFFECT_RUNTIME_RENDERER_KIND::LIGHT_PARTICLE:
		default:
			eOutFamily = Client::EFFECT_GPU_RENDER_FAMILY::END;
			return false;
		}
	}

	bool Try_NarrowRuntimeFloat3(
		const std::array<double, 3u>& Source,
		float3_t& OutValue)
	{
		constexpr double MAX_FLOAT =
			static_cast<double>((std::numeric_limits<f32_t>::max)());
		for (const double Value : Source)
		{
			if (!std::isfinite(Value) || std::abs(Value) > MAX_FLOAT)
				return false;
		}
		OutValue = {
			static_cast<f32_t>(Source[0]),
			static_cast<f32_t>(Source[1]),
			static_cast<f32_t>(Source[2])
		};
		return true;
	}

	const char* ArtistCoreFamilyLabel(
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_GPU_RENDER_FAMILY::MESH:
			return "MeshParticle";
		case Client::EFFECT_GPU_RENDER_FAMILY::SPRITE:
			return "SpriteParticle";
		case Client::EFFECT_GPU_RENDER_FAMILY::DECAL:
			return "LocalDecal";
		case Client::EFFECT_GPU_RENDER_FAMILY::RIBBON:
			return "CascadeRibbon";
		case Client::EFFECT_GPU_RENDER_FAMILY::END:
		default:
			return "Invalid";
		}
	}

	const char* VisualProgramFamilyLabel(
		const Client::EFFECT_VISUAL_PROGRAM_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE:
			return "MeshParticle";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE:
			return "SpriteParticle";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE:
			return "LocalDecal";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON:
			return "CascadeRibbon";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL:
			return "AnimationTrail";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE:
			return "LightParticle";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::SCREEN_POST:
			return "ScreenPost";
		case Client::EFFECT_VISUAL_PROGRAM_FAMILY::END:
		default:
			return "Invalid";
		}
	}

	std::string StableIdentityLeaf(const std::string_view strIdentity)
	{
		const size_t iEvent = strIdentity.find(".event_source-");
		const std::string_view Primary = std::string_view::npos == iEvent ?
			strIdentity : strIdentity.substr(0u, iEvent);
		const size_t iSeparator = Primary.find_last_of("./:\\");
		std::string Label = std::string_view::npos == iSeparator ?
			std::string(Primary) : std::string(Primary.substr(iSeparator + 1u));
		if (std::string_view::npos != iEvent)
		{
			Label += " @ ";
			Label += strIdentity.substr(iEvent + std::string_view(
				".event_source-").size());
		}
		return Label.empty() ? std::string("unnamed") : Label;
	}

	std::string ResourceAssetLeaf(const std::string_view strAssetId)
	{
		if (strAssetId.empty())
			return {};
		return std::filesystem::path(strAssetId).filename().string();
	}

	std::string PrimaryVisualResourceLeaf(
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources)
	{
		static constexpr std::array<std::string_view, 9u> SLOT_PRIORITY{
			"meshModel", "base", "emissive", "mask", "noise", "dissolve",
			"base2", "mask2", "noise2" };
		for (const std::string_view strSlot : SLOT_PRIORITY)
		{
			const auto Found = std::find_if(Resources.begin(), Resources.end(),
				[strSlot](const auto& Resource)
				{ return Resource.strSlotId == strSlot && !Resource.strAssetId.empty(); });
			if (Found != Resources.end())
				return ResourceAssetLeaf(Found->strAssetId);
		}
		const auto Found = std::find_if(Resources.begin(), Resources.end(),
			[](const auto& Resource) { return !Resource.strAssetId.empty(); });
		return Found == Resources.end() ? std::string("unbound") :
			ResourceAssetLeaf(Found->strAssetId);
	}

	std::string PrimaryAuthoringResourceLeaf(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		static constexpr std::array<std::string_view, 9u> SLOT_PRIORITY{
			"meshModel", "base", "emissive", "mask", "noise", "dissolve",
			"base2", "mask2", "noise2" };
		for (const std::string_view strSlot : SLOT_PRIORITY)
		{
			const auto Found = std::find_if(Element.ResourceBindings.begin(),
				Element.ResourceBindings.end(),
				[strSlot](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
				{ return Binding.strSlotId == strSlot && !Binding.strAssetId.empty(); });
			if (Found != Element.ResourceBindings.end())
				return ResourceAssetLeaf(Found->strAssetId);
		}
		const auto Found = std::find_if(Element.Material.SourceMaterial.Textures.begin(),
			Element.Material.SourceMaterial.Textures.end(),
			[](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{ return !Texture.strAssetId.empty(); });
		return Found == Element.Material.SourceMaterial.Textures.end() ?
			std::string("unbound") : ResourceAssetLeaf(Found->strAssetId);
	}

	std::string FriendlyModelCueLabel(
		const size_t iOrdinal,
		const Client::EFFECT_MODEL_CUE_DESC& Cue)
	{
		std::ostringstream Label;
		Label << "Summon ";
		if (iOrdinal < 10u)
			Label << '0';
		Label << iOrdinal << " | ";
		const std::string ResourceLeaf = ResourceAssetLeaf(Cue.strModelAssetId);
		Label << (ResourceLeaf.empty() ? std::string("unbound") : ResourceLeaf);
		return Label.str();
	}

	std::string FriendlyDocumentLabel(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string_view strFallback)
	{
		if (!strFallback.empty() &&
			(Document.strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID ||
			 Document.strEffectAssetId ==
				DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID))
		{
			return std::string(strFallback);
		}
		if (!Document.strDisplayName.empty())
			return Document.strDisplayName;
		return strFallback.empty() ? std::string("Effect") :
			std::string(strFallback);
	}

	std::string StableUnifiedElementId(
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily,
		const std::string_view strSourceIdentity)
	{
		const std::string Digest =
			Client::CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				std::string(ArtistCoreFamilyLabel(eFamily)) + "\n" +
				std::string(strSourceIdentity));
		std::string Prefix;
		switch (eFamily)
		{
		case Client::EFFECT_GPU_RENDER_FAMILY::MESH: Prefix = "mesh"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::SPRITE: Prefix = "sprite"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::DECAL: Prefix = "decal"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::RIBBON: Prefix = "ribbon"; break;
		case Client::EFFECT_GPU_RENDER_FAMILY::END:
		default: return {};
		}
		return Prefix + "." + Digest.substr(0u, 16u);
	}

	std::string VisualProgramResourceSlotSummary(
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources)
	{
		std::string Summary;
		for (const Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW& Resource :
			Resources)
		{
			if (Resource.strSlotId.empty())
				continue;
			if (!Summary.empty())
				Summary += ", ";
			Summary += Resource.strSlotId;
			if (!Resource.strAssetId.empty())
			{
				Summary += "=";
				Summary += std::filesystem::path(
					Resource.strAssetId).filename().string();
			}
		}
		return Summary.empty() ? std::string("no bound slots") : Summary;
	}

	std::string AuthoringElementResourceSlotSummary(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		std::string Summary;
		std::set<std::string, std::less<>> SeenSlots;
		const auto Append = [&Summary, &SeenSlots](
			const std::string& strSlotId,
			const std::string& strAssetId)
		{
			if (strSlotId.empty() || strAssetId.empty() ||
				!SeenSlots.insert(strSlotId).second)
			{
				return;
			}
			if (!Summary.empty())
				Summary += ", ";
			Summary += strSlotId + "=" +
				std::filesystem::path(strAssetId).filename().string();
		};
		for (const Client::EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			Append(Binding.strSlotId, Binding.strAssetId);
		}
		for (const Client::EFFECT_NAMED_TEXTURE_DESC& Texture :
			Element.Material.SourceMaterial.Textures)
		{
			Append(Texture.strName, Texture.strAssetId);
		}
		for (const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane :
			Element.Material.Execution.TextureLanes)
		{
			Append(Lane.strRole.empty() ? Lane.strLaneId : Lane.strRole,
				Lane.strAssetId);
		}
		return Summary.empty() ? std::string("no bound slots") : Summary;
	}

	std::string VisualProgramElementRowLabel(
		const Client::EFFECT_VISUAL_PROGRAM_FAMILY eFamily,
		const size_t iElementOrdinal,
		const std::string& strSourceRecordId,
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources)
	{
		(void)strSourceRecordId;
		return std::string(VisualProgramFamilyLabel(eFamily)) + " " +
			(iElementOrdinal < 10u ? "0" : "") +
			std::to_string(iElementOrdinal) + " | " +
			PrimaryVisualResourceLeaf(Resources);
	}

	void Upsert_OccurrenceTuningEntry(
		Client::EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const std::string& strOccurrenceId,
		const std::string& strSourceRowSha256,
		const Client::EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Transform)
	{
		auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_OCCURRENCE_TUNING_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		Client::EFFECT_OCCURRENCE_TUNING_ENTRY Staged;
		Staged.strOccurrenceId = strOccurrenceId;
		Staged.strSourceOccurrenceRowSha256 = strSourceRowSha256;
		Staged.strProvenance = "PROJECT_TUNED";
		Staged.EffectiveLocalTransform = Transform;
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			*Found = std::move(Staged);
		}
		else
		{
			Document.Entries.insert(Found, std::move(Staged));
		}
	}

	void Remove_OccurrenceTuningEntry(
		Client::EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const std::string& strOccurrenceId)
	{
		const auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_OCCURRENCE_TUNING_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			Document.Entries.erase(Found);
		}
	}

	void Upsert_SourceAuthoringOverlayEntry(
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const std::string& strOccurrenceId,
		const std::string& strSourceRowSha256,
		const std::string& strSourceElementId,
		const Client::EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Transform)
	{
		auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY Staged;
		Staged.strOccurrenceId = strOccurrenceId;
		Staged.strSourceOccurrenceRowSha256 = strSourceRowSha256;
		Staged.strSourceElementId = strSourceElementId;
		Staged.strProvenance = "PROJECT_TUNED";
		Staged.EffectiveLocalTransform = Transform;
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			*Found = std::move(Staged);
		}
		else
		{
			Document.Entries.insert(Found, std::move(Staged));
		}
	}

	void Remove_SourceAuthoringOverlayEntry(
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const std::string& strOccurrenceId)
	{
		const auto Found = std::lower_bound(
			Document.Entries.begin(), Document.Entries.end(), strOccurrenceId,
			[](const Client::EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY& Entry,
				const std::string& Id)
			{
				return Entry.strOccurrenceId < Id;
			});
		if (Found != Document.Entries.end() &&
			Found->strOccurrenceId == strOccurrenceId)
		{
			Document.Entries.erase(Found);
		}
	}

    bool Run_OwnedToolProcess(
        std::wstring Command,
        const std::filesystem::path& WorkingDirectory,
        const DWORD iTimeoutMilliseconds,
        const std::string_view strLabel,
        std::string& strOutStatus)
    {
        if (Command.empty() || WorkingDirectory.empty())
        {
            strOutStatus = std::string(strLabel) + " command is invalid.";
            return false;
        }
        std::vector<wchar_t> MutableCommand(Command.begin(), Command.end());
        MutableCommand.push_back(L'\0');
        STARTUPINFOW Startup{};
        Startup.cb = sizeof(Startup);
        PROCESS_INFORMATION Process{};
        if (!CreateProcessW(
            nullptr, MutableCommand.data(), nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW, nullptr, WorkingDirectory.c_str(),
            &Startup, &Process))
        {
            strOutStatus = "Could not start " + std::string(strLabel) + ".";
            return false;
        }
        CloseHandle(Process.hThread);
        const DWORD iWait = WaitForSingleObject(
            Process.hProcess, iTimeoutMilliseconds);
        if (WAIT_TIMEOUT == iWait)
        {
            TerminateProcess(Process.hProcess, 124u);
            WaitForSingleObject(Process.hProcess, 5000u);
            CloseHandle(Process.hProcess);
            strOutStatus = std::string(strLabel) +
                " timed out; its owned process was terminated.";
            return false;
        }
        DWORD iExitCode = 1u;
        const bool bSucceeded = WAIT_OBJECT_0 == iWait &&
            GetExitCodeProcess(Process.hProcess, &iExitCode) &&
            0u == iExitCode;
        CloseHandle(Process.hProcess);
        strOutStatus = bSucceeded ?
            std::string(strLabel) + " succeeded." :
            std::string(strLabel) + " failed with exit code " +
                std::to_string(iExitCode) + ".";
        return bSucceeded;
    }

    const char* Kind_Label(const Client::EFFECT_ELEMENT_KIND eKind)
    {
        switch (eKind)
        {
        case Client::EFFECT_ELEMENT_KIND::MESH: return "Standalone Mesh";
        case Client::EFFECT_ELEMENT_KIND::SPRITE: return "Standalone Sprite";
        case Client::EFFECT_ELEMENT_KIND::PARTICLE: return "Cascade Particle";
        case Client::EFFECT_ELEMENT_KIND::DECAL: return "Decal";
        case Client::EFFECT_ELEMENT_KIND::TRAIL: return "Trail";
        case Client::EFFECT_ELEMENT_KIND::LIGHT: return "Light";
        case Client::EFFECT_ELEMENT_KIND::SCREEN_POST: return "Screen Post";
        case Client::EFFECT_ELEMENT_KIND::END:
        default: return "Invalid";
        }
    }

	const char* AuthoringFamily_Label(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		return Client::Get_EffectToolAuthoringFamilyLabel(eFamily);
	}

	Client::EFFECT_ELEMENT_KIND AuthoringFamily_Kind(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_AUTHORING_FAMILY::MESH:
			return Client::EFFECT_ELEMENT_KIND::MESH;
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE:
			return Client::EFFECT_ELEMENT_KIND::SPRITE;
		case Client::EFFECT_AUTHORING_FAMILY::MESH_PARTICLE:
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE:
			return Client::EFFECT_ELEMENT_KIND::PARTICLE;
		case Client::EFFECT_AUTHORING_FAMILY::LOCAL_DECAL:
			return Client::EFFECT_ELEMENT_KIND::DECAL;
		case Client::EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON:
			return Client::EFFECT_ELEMENT_KIND::TRAIL;
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_LIGHT:
			return Client::EFFECT_ELEMENT_KIND::LIGHT;
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_SCREEN_POST:
			return Client::EFFECT_ELEMENT_KIND::SCREEN_POST;
		case Client::EFFECT_AUTHORING_FAMILY::END:
		default: return Client::EFFECT_ELEMENT_KIND::END;
		}
	}

	bool_t AuthoringFamily_CanCreate(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		return eFamily >= Client::EFFECT_AUTHORING_FAMILY::MESH &&
			eFamily <= Client::EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON;
	}

	bool_t AuthoringFamily_RequiresMesh(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		return eFamily == Client::EFFECT_AUTHORING_FAMILY::MESH ||
			eFamily == Client::EFFECT_AUTHORING_FAMILY::MESH_PARTICLE;
	}

	const char* AuthoringFamily_ElementPrefix(
		const Client::EFFECT_AUTHORING_FAMILY eFamily)
	{
		switch (eFamily)
		{
		case Client::EFFECT_AUTHORING_FAMILY::MESH: return "mesh";
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE: return "sprite";
		case Client::EFFECT_AUTHORING_FAMILY::MESH_PARTICLE:
			return "mesh_particle";
		case Client::EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE:
			return "sprite_particle";
		case Client::EFFECT_AUTHORING_FAMILY::LOCAL_DECAL:
			return "local_decal";
		case Client::EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON:
			return "trail_ribbon";
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_LIGHT:
			return "presentation_light";
		case Client::EFFECT_AUTHORING_FAMILY::PRESENTATION_SCREEN_POST:
			return "presentation_screen_post";
		case Client::EFFECT_AUTHORING_FAMILY::END:
		default: return "element";
		}
	}

	Client::EFFECT_AUTHORING_FAMILY Resolve_AuthoringFamily(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::Resolve_EffectToolAuthoringFamily(Element);
	}

	const char_t* ScreenPostProfile_Label(
		const Client::EFFECT_SCREEN_POST_PROFILE eProfile)
	{
		switch (eProfile)
		{
		case Client::EFFECT_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED_V1:
			return "RGB Noise (Reconstructed v1)";
		case Client::EFFECT_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED_V1:
			return "Zoom Blur (Reconstructed v1)";
		case Client::EFFECT_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED_V1:
			return "Film Noise (Reconstructed v1)";
		case Client::EFFECT_SCREEN_POST_PROFILE::END:
		default:
			return "Unresolved";
		}
	}

	bool_t HasAuthoringApproximate(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		return std::any_of(Document.Elements.begin(), Document.Elements.end(),
			[](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Client::Get_EffectAuthoringFidelity(
					Element.Material.Execution) ==
					Client::EFFECT_AUTHORING_FIDELITY::APPROXIMATE;
			});
	}

	std::string FriendlyAuthoringElementLabel(
		const Client::EFFECT_AUTHORING_FAMILY eFamily,
		const size_t iOrdinal,
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		std::ostringstream Label;
		Label << AuthoringFamily_Label(eFamily) << ' ';
		if (iOrdinal < 10u)
			Label << '0';
		Label << iOrdinal << " | " << PrimaryAuthoringResourceLeaf(Element) <<
			" | t+" << std::fixed << std::setprecision(3) <<
			Element.Detail.Timing.fStartDelaySeconds << 's';
		if (Element.strSourceNode.starts_with("valtan.source."))
			Label << " [SOURCE]";
		else if (Element.strSourceNode.starts_with("project-authored:"))
			Label << " [PROJECT]";
		const Client::EFFECT_AUTHORING_FIDELITY eFidelity =
			Client::Get_EffectAuthoringFidelity(Element.Material.Execution);
		if (eFidelity == Client::EFFECT_AUTHORING_FIDELITY::APPROXIMATE)
			Label << " [APPROXIMATE]";
		else if (eFidelity ==
			Client::EFFECT_AUTHORING_FIDELITY::PROJECT_TUNED_APPROX)
			Label << " [PROJECT_TUNED_APPROX]";
		return Label.str();
	}

	// CAnimationTargetService reports the preview target by the pAssetName of
	// its ANIMATION_PREVIEW_ASSETS descriptor, which is "Valtan" for the boss
	// entry. Keeping the comparison in one place stops the boss authoring
	// paths from silently falling through to the playable-class branch when
	// that descriptor name changes.
	constexpr const char* VALTAN_ANIMATION_ASSET_NAME = "Valtan";

	bool_t AuthoringFamily_AllowsSlot(
		const Client::EFFECT_AUTHORING_FAMILY eFamily,
		const std::string_view strSlotId)
	{
		return strSlotId != Client::EFFECT_MESH_SHAPE_SLOT_ID ||
			AuthoringFamily_RequiresMesh(eFamily);
	}

    const char* Slot_Label(const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
        switch (eSlot)
        {
        case Client::EFFECT_RESOURCE_SLOT::MESH_MODEL: return "Mesh";
        case Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE: return "Base";
        case Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE: return "Noise";
        case Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE: return "Mask";
        case Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE: return "Emissive";
        case Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE: return "Dissolve";
        case Client::EFFECT_RESOURCE_SLOT::BASE2_TEXTURE: return "Base 2";
        case Client::EFFECT_RESOURCE_SLOT::MASK2_TEXTURE: return "Mask 2";
        case Client::EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE: return "Noise 2";
        case Client::EFFECT_RESOURCE_SLOT::END:
        default: return "Invalid";
        }
    }

    const char* SourceMaterialStatus_Label(
        const Client::EFFECT_SOURCE_MATERIAL_STATUS eStatus)
    {
        using Client::EFFECT_SOURCE_MATERIAL_STATUS;
        switch (eStatus)
        {
        case EFFECT_SOURCE_MATERIAL_STATUS::SOURCE_EXACT:
            return "SOURCE_EXACT";
        case EFFECT_SOURCE_MATERIAL_STATUS::RUNTIME_EXACT:
            return "RUNTIME_EXACT";
        case EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE:
            return "RECONSTRUCTED_PROFILE";
        case EFFECT_SOURCE_MATERIAL_STATUS::UNSUPPORTED:
            return "UNSUPPORTED";
        case EFFECT_SOURCE_MATERIAL_STATUS::MISSING_RESOURCE:
            return "MISSING_RESOURCE";
        case EFFECT_SOURCE_MATERIAL_STATUS::END:
        default:
            return "INVALID";
        }
    }

    const char* Class_Label(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "Lance Master";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "Dimension Master";
        case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
        case CHARACTER_CLASS_ID::END:
        default: return "Invalid";
        }
    }

    const char* Resource_DomainId(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
        case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
        case CHARACTER_CLASS_ID::END:
        default: return nullptr;
        }
    }

    std::string EffectAsset_DomainId(const std::string& strEffectAssetId)
    {
        constexpr std::pair<std::string_view, std::string_view> Domains[] =
        {
            { "effect.lancemaster.", "LanceMaster" },
            { "effect.gunslinger.", "Gunslinger" },
            { "effect.slayer.", "Slayer" },
            { "effect.artist.", "Artist" },
            { "effect.dimensionmaster.", "DimensionMaster" },
            { "effect.warlord.", "Warlord" },
            { "effect.valtan.", "Valtan" }
        };
        for (const auto& [Prefix, DomainId] : Domains)
        {
            if (strEffectAssetId.starts_with(Prefix))
                return std::string(DomainId);
        }
        return "Uncategorized";
    }

    std::string First_PathComponent(const std::filesystem::path& Relative)
    {
        const auto Iterator = Relative.begin();
        if (Iterator == Relative.end())
            return {};
        return Iterator->generic_string();
    }

    bool Try_DeriveEffectAssetIdFromFilename(
        const std::filesystem::path& Path,
        const Client::EFFECT_DOCUMENT_SOURCE eSource,
        std::string& OutAssetId)
    {
        constexpr std::string_view EffectSuffix = ".effect.json";
        constexpr std::string_view ImportedSuffix = ".imported";
        const std::string Name = Path.filename().string();
        if (!Name.ends_with(EffectSuffix))
            return false;

        std::string AssetId = Name.substr(
            0u, Name.size() - EffectSuffix.size());
        if (Client::EFFECT_DOCUMENT_SOURCE::IMPORTED == eSource &&
            AssetId.ends_with(ImportedSuffix))
        {
            AssetId.resize(AssetId.size() - ImportedSuffix.size());
        }
        if (AssetId.empty() || AssetId.size() > 128u ||
            !std::all_of(AssetId.begin(), AssetId.end(),
                [](const char Character)
                {
                    const unsigned char Value =
                        static_cast<unsigned char>(Character);
                    return 0 != std::isalnum(Value) || Character == '_' ||
                        Character == '-' || Character == '.';
                }))
        {
            return false;
        }
        OutAssetId = std::move(AssetId);
        return true;
    }

    bool_t Ensure_PlayerSkillCatalog(std::string& OutStatus)
    {
        if (!Client::CPlayerSkillCatalog::Get_Skills().empty())
        {
            OutStatus = "Using the loaded player skill catalog.";
            return true;
        }
        return Client::CPlayerSkillCatalog::Load(OutStatus);
    }

    const char* Animation_AssetName(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (eClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
        case CHARACTER_CLASS_ID::GUNSLINGER: return "GunSlinger";
        case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
        case CHARACTER_CLASS_ID::ARTIST: return "Artist";
        case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
        case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
        case CHARACTER_CLASS_ID::END:
        default: return nullptr;
        }
    }

    std::vector<std::string> Collect_AnimationClipNames(
        const std::shared_ptr<Engine::CModel>& pModel)
    {
        std::vector<std::string> Clips;
        if (nullptr == pModel)
            return Clips;
        Clips.reserve(pModel->Get_NumAnimations());
        for (uint32_t iAnimation = 0u;
            iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
        {
            const char* pName = pModel->Get_AnimationName(iAnimation);
            if (nullptr != pName)
                Clips.emplace_back(pName);
        }
        return Clips;
    }

    bool Read_TextFile(
        const std::filesystem::path& Path,
        std::string& OutText,
        std::string& OutStatus)
    {
        OutText.clear();
        std::ifstream Input(Path, std::ios::binary);
        if (Path.empty() || !Input)
        {
            OutStatus = "Could not open presentation document: " +
                Path.string();
            return false;
        }
        std::ostringstream Buffer;
        Buffer << Input.rdbuf();
        if (!Input.good() && !Input.eof())
        {
            OutStatus = "Could not read presentation document: " +
                Path.string();
            return false;
        }
        OutText = Buffer.str();
        return true;
    }

    std::vector<Client::ANIMATION_SKILL_CLIP> Flatten_BindingClips(
        const Client::ANIMATION_SKILL_BINDING& Binding)
    {
        std::vector<Client::ANIMATION_SKILL_CLIP> Clips;
        for (const Client::ANIMATION_SKILL_STAGE& Stage : Binding.Stages)
        {
            Clips.insert(Clips.end(), Stage.Clips.begin(), Stage.Clips.end());
        }
        return Clips;
    }

    bool Try_ParseEffectDiagnosticRow(
        const std::string_view Line,
        std::string& OutClip,
        bool_t& OutImported,
        bool_t& OutEmptyPayload)
    {
        OutClip.clear();
        OutImported = false;
        OutEmptyPayload = false;
        if (Line.empty() || Line.front() != '"')
            return false;
        const size_t iClipEnd = Line.find('"', 1u);
        if (std::string_view::npos == iClipEnd ||
            std::string_view::npos == Line.find(" EFFECT ", iClipEnd) ||
            std::string_view::npos != Line.find(" effectref=asset "))
        {
            return false;
        }
        const size_t iPayload = Line.find(" payload=\"", iClipEnd);
        if (std::string_view::npos == iPayload)
            return false;
        const size_t iValueBegin = iPayload + 10u;
        const size_t iValueEnd = Line.find('"', iValueBegin);
        if (std::string_view::npos == iValueEnd)
            return false;
        OutClip.assign(Line.substr(1u, iClipEnd - 1u));
        OutImported = std::string_view::npos != Line.find(" src=orig");
        OutEmptyPayload = iValueBegin == iValueEnd;
        return true;
    }

    float4x4_t Compose_EffectLocal(
        const Client::EFFECT_TRANSFORM_DESC& Local,
        const float4x4_t& Anchor)
    {
        float4x4_t Result{};
        XMStoreFloat4x4(&Result,
            XMMatrixScaling(Local.vScale.x, Local.vScale.y, Local.vScale.z) *
            XMMatrixRotationRollPitchYaw(
                XMConvertToRadians(Local.vRotationDegrees.x),
                XMConvertToRadians(Local.vRotationDegrees.y),
                XMConvertToRadians(Local.vRotationDegrees.z)) *
            XMMatrixTranslation(
                Local.vPosition.x,
                Local.vPosition.y,
                Local.vPosition.z) *
            XMLoadFloat4x4(&Anchor));
        return Result;
    }

	bool_t Try_ExtractPlanarYawDegrees(
		const float4x4_t& Root,
		f32_t& fOutYawDegrees)
	{
		const f32_t fForwardLength = std::sqrt(
			Root._31 * Root._31 + Root._33 * Root._33);
		if (!std::isfinite(Root._31) || !std::isfinite(Root._33) ||
			!std::isfinite(fForwardLength) || fForwardLength <= 1.0e-6f)
		{
			return false;
		}
		const f32_t fYawDegrees = XMConvertToDegrees(
			std::atan2(Root._31, Root._33));
		if (!std::isfinite(fYawDegrees))
			return false;
		fOutYawDegrees = fYawDegrees;
		return true;
	}

	const char* Source_Label(const Client::EFFECT_DOCUMENT_SOURCE eSource)
	{
        switch (eSource)
        {
        case Client::EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT: return "New";
        case Client::EFFECT_DOCUMENT_SOURCE::AUTHORED: return "Authored";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED: return "Imported";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE:
            return "Imported Draft";
		case Client::EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE:
			return "Migration Reference";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY:
			return "Assembly";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT:
			return "WFX Component";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM:
			return "Visual Program Copy";
        case Client::EFFECT_DOCUMENT_SOURCE::END:
        default: return "Invalid";
		}
	}

	std::string Unified_CandidateAssetId(const std::string_view AssetId)
	{
		constexpr std::string_view LegacyToken = ".authored-baseline";
		constexpr std::string_view UnifiedSuffix = ".unified";
		std::string Candidate(AssetId);
		if (const size_t iLegacyToken = Candidate.find(LegacyToken);
			iLegacyToken != std::string::npos)
		{
			Candidate.erase(iLegacyToken, LegacyToken.size());
		}
		if (!Candidate.ends_with(UnifiedSuffix))
			Candidate += UnifiedSuffix;
		return Candidate;
	}

	const char* Profile_Label(const Client::EFFECT_RENDER_PROFILE eProfile)
    {
        switch (eProfile)
        {
        case Client::EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
            return "Opaque / Back / Depth Write";
        case Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
            return "Alpha / Two Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
            return "Additive / Two Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ:
            return "Alpha / One Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ:
            return "Additive / One Sided / Depth Read";
        case Client::EFFECT_RENDER_PROFILE::END:
        default: return "Invalid";
        }
    }

    bool Contains_NoCase(
        const std::string& Value,
        const std::string_view Filter)
    {
        if (Filter.empty())
            return true;
        return Value.end() != std::search(
            Value.begin(), Value.end(), Filter.begin(), Filter.end(),
            [](const char Left, const char Right)
            {
                return std::tolower(static_cast<unsigned char>(Left)) ==
                    std::tolower(static_cast<unsigned char>(Right));
            });
    }

    std::string Build_ValtanV0EffectAssetId(const std::string_view ClipName)
    {
        std::string Suffix;
        Suffix.reserve((std::min)(ClipName.size(), size_t{ 80u }));
        bool_t bPreviousSeparator = false;
        for (const char_t Character : ClipName)
        {
            const unsigned char Value =
                static_cast<unsigned char>(Character);
            if (0 != std::isalnum(Value))
            {
                if (Suffix.size() >= 80u)
                    break;
                Suffix.push_back(static_cast<char_t>(std::tolower(Value)));
                bPreviousSeparator = false;
            }
            else if (!Suffix.empty() && !bPreviousSeparator)
            {
                Suffix.push_back('-');
                bPreviousSeparator = true;
            }
        }
        while (!Suffix.empty() && '-' == Suffix.back())
            Suffix.pop_back();
        if (Suffix.empty())
            Suffix = "cue";
        return "effect.valtan.user." + Suffix + ".v001";
    }

    bool Matches_MeshShapeCategory(
        const std::string& strAssetId,
        const std::string_view strCategory)
    {
        if (strCategory.empty() || "All" == strCategory)
            return true;
        const auto HasAny = [&strAssetId](
            const std::initializer_list<std::string_view> Tokens)
        {
            return std::any_of(Tokens.begin(), Tokens.end(),
                [&strAssetId](const std::string_view Token)
                {
                    return Contains_NoCase(strAssetId, Token);
                });
        };
        if ("Ring / Torus / Circle" == strCategory)
            return HasAny({ "ring", "torus", "circle" });
        if ("Slash / Trail / Plane" == strCategory)
            return HasAny({ "slash", "swing", "sword", "trail", "plane" });
        if ("Crack / Broken" == strCategory)
            return HasAny({ "crack", "broken" });
        if ("Sphere / Hemisphere" == strCategory)
            return HasAny({ "sphere", "hemisphere" });
        if ("Cylinder / Cone" == strCategory)
            return HasAny({ "cylinder", "cone" });
        if ("Helix" == strCategory)
            return HasAny({ "helix" });
        if ("Box / Cube / Square" == strCategory)
            return HasAny({ "box", "cube", "square" });
        if ("Wave / Aurora / Electric" == strCategory)
            return HasAny({ "wave", "aurora", "electric" });
        if ("Other" == strCategory)
        {
            return !HasAny({ "ring", "torus", "circle", "slash", "swing",
                "sword", "trail", "plane", "crack", "broken", "sphere",
                "hemisphere", "cylinder", "cone", "helix", "box", "cube",
                "square", "wave", "aurora", "electric" });
        }
        return true;
    }

    // Effect DDS filenames follow fx_<bucket>_<kind>_<index>[_variant]. The
    // bucket letter only mirrors the source package folder, so the kind token
    // is the only part that says what the texture is for. Tokens that also
    // appear inside unrelated words are anchored with the separator.
    bool Matches_TextureKindCategory(
        const std::string& strAssetId,
        const std::string_view strCategory)
    {
        if (strCategory.empty() || "All" == strCategory)
            return true;
        const auto HasAny = [&strAssetId](
            const std::initializer_list<std::string_view> Tokens)
        {
            return std::any_of(Tokens.begin(), Tokens.end(),
                [&strAssetId](const std::string_view Token)
                {
                    return Contains_NoCase(strAssetId, Token);
                });
        };
        if ("Base / Sprite" == strCategory)
            return HasAny({ "atypical", "glow", "shine", "star", "_hit",
                "spatter", "fragment", "stoneparts", "aura" });
        if ("Noise / Distortion" == strCategory)
            return HasAny({ "noise", "flow", "turbulence" });
        if ("Normal / Bump" == strCategory)
            return HasAny({ "normal", "_n.", "_n_" });
        if ("Decal / Ground" == strCategory)
            return HasAny({ "decal", "grid", "symbol", "sector" });
        if ("Ring / Shockwave" == strCategory)
            return HasAny({ "ring", "wave" });
        if ("Trail / Beam" == strCategory)
            return HasAny({ "trail", "_line", "auraline", "thunder",
                "electric", "electile" });
        if ("Cloud / Smoke / Fire" == strCategory)
            return HasAny({ "cloud", "smoke", "fire", "fogsheet" });
        if ("Fluid / Water" == strCategory)
            return HasAny({ "fluid", "liquid", "water", "softriver", "_ice" });
        if ("Other" == strCategory)
        {
            return !HasAny({ "atypical", "glow", "shine", "star", "_hit",
                "spatter", "fragment", "stoneparts", "aura", "noise", "flow",
                "turbulence", "normal", "_n.", "_n_", "decal", "grid",
                "symbol", "sector", "ring", "wave", "trail", "_line",
                "auraline", "thunder", "electric", "electile", "cloud",
                "smoke", "fire", "fogsheet", "fluid", "liquid", "water",
                "softriver", "_ice" });
        }
        return true;
    }

    enum class CASCADE_RENDERER_KIND : uint8_t
    {
        MESH,
        SPRITE,
        UNRESOLVED
    };

    CASCADE_RENDERER_KIND Resolve_CascadeRendererKind(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
        const bool_t bHasMeshModel = std::any_of(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId ==
                    Client::EFFECT_MESH_SHAPE_SLOT_ID;
            });
        if (!Element.SourceRecipe.bEnabled)
            return bHasMeshModel ? CASCADE_RENDERER_KIND::MESH :
                CASCADE_RENDERER_KIND::SPRITE;
        if (Element.SourceRecipe.strRendererShape == "mesh")
            return bHasMeshModel ? CASCADE_RENDERER_KIND::MESH :
                CASCADE_RENDERER_KIND::UNRESOLVED;
        if (Element.SourceRecipe.strRendererShape == "sprite")
            return bHasMeshModel ? CASCADE_RENDERER_KIND::UNRESOLVED :
                CASCADE_RENDERER_KIND::SPRITE;
        return CASCADE_RENDERER_KIND::UNRESOLVED;
    }

    const char* CascadeRenderer_Label(const CASCADE_RENDERER_KIND eKind)
    {
        switch (eKind)
        {
        case CASCADE_RENDERER_KIND::MESH: return "Mesh Renderer";
        case CASCADE_RENDERER_KIND::SPRITE: return "Sprite Renderer";
        case CASCADE_RENDERER_KIND::UNRESOLVED:
        default: return "Unresolved Renderer";
        }
    }

    const char* Element_RendererLabel(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
        return Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ?
            CascadeRenderer_Label(Resolve_CascadeRendererKind(Element)) :
            Kind_Label(Element.eKind);
    }

	const char* PreviewPivot_Label(
		const Client::EFFECT_PREVIEW_PIVOT_KIND eKind)
	{
		switch (eKind)
		{
		case Client::EFFECT_PREVIEW_PIVOT_KIND::WORLD: return "World";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT: return "Player Root";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET: return "Weapon Socket";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE: return "Model Bone";
		case Client::EFFECT_PREVIEW_PIVOT_KIND::END:
		default: return "Invalid";
		}
	}

	std::string Lower_Ascii(const std::string_view Value)
	{
		std::string Result(Value);
		std::transform(Result.begin(), Result.end(), Result.begin(),
			[](const char Value)
			{
				return static_cast<char>(std::tolower(
					static_cast<unsigned char>(Value)));
			});
		return Result;
	}

	struct SOURCE_MODULE_UI_DESC final
	{
		const char* pRole = "Source Module";
		const char* pDescription =
			"Lossless source values are shown with their original property paths.";
	};

	SOURCE_MODULE_UI_DESC Describe_SourceModule(
		const std::string_view strClassName)
	{
		const std::string ClassName = Lower_Ascii(strClassName);
		if (ClassName.find("velocityoverlife") != std::string::npos)
			return { "Velocity Over Life",
				"Scales or replaces particle velocity over normalized lifetime." };
		if (ClassName.find("meshrotationrate") != std::string::npos ||
			ClassName.find("rotationrate") != std::string::npos)
			return { "Rotation Speed",
				"Controls sprite or mesh angular velocity over particle life." };
		if (ClassName.find("meshrotation") != std::string::npos ||
			ClassName.find("rotation") != std::string::npos)
			return { "Initial Rotation",
				"Defines the initial sprite or mesh orientation distribution." };
		if (ClassName.find("initiallocation") != std::string::npos ||
			ClassName.find("locationdirect") != std::string::npos)
			return { "Initial Location",
				"Defines the emitter-relative starting position or direct source location." };
		if (ClassName.find("bone") != std::string::npos ||
			ClassName.find("socket") != std::string::npos)
			return { "Bone / Socket Location",
				"Spawns from the named source bones or sockets and their selection policy." };
		if (ClassName.find("locationsphere") != std::string::npos ||
			ClassName.find("locationprimitivesphere") != std::string::npos)
			return { "Sphere Surface",
				"Spawns particles from the source sphere volume or surface contract." };
		if (ClassName.find("locationcylinder") != std::string::npos ||
			ClassName.find("locationprimitivecylinder") != std::string::npos)
			return { "Cylinder Surface",
				"Spawns particles from the source cylinder volume or surface contract." };
		if (ClassName.find("locationcircle") != std::string::npos ||
			ClassName.find("locationprimitivecircle") != std::string::npos)
			return { "Circle Surface",
				"Spawns particles from the source circle radius or surface contract." };
		if (ClassName.find("vectorfield") != std::string::npos)
			return { "Vector Field",
				"Applies the referenced local vector-field force and its source parameters." };
		if (ClassName.find("cameraoffset") != std::string::npos)
			return { "Camera Offset",
				"Offsets particles along the active camera direction." };
		if (ClassName.find("subuv") != std::string::npos)
			return { "SubUV Animation",
				"Selects and blends source texture-atlas frames over particle life." };
		if (ClassName.find("dynamicparameter") != std::string::npos ||
			ClassName.find("parameterdynamic") != std::string::npos)
			return { "Dynamic Material Parameters",
				"Evaluates the four source particle-to-material parameter channels." };
		if (ClassName.find("meshmaterial") != std::string::npos)
			return { "Mesh Material Override",
				"Binds the source material overrides for mesh renderer sections." };
		if (ClassName.find("typedatamesh") != std::string::npos)
			return { "Mesh Renderer",
				"Defines the source mesh renderer asset, alignment, and material policy." };
		if (ClassName.find("axislock") != std::string::npos)
			return { "Renderer Axis Lock",
				"Constrains the renderer-facing or rotation axis using the source policy." };
		if (ClassName.find("orbit") != std::string::npos)
			return { "Orbit",
				"Applies source orbit offset, rotation, and rotation-rate distributions." };
		if (ClassName.find("vortex") != std::string::npos)
			return { "Vortex",
				"Applies the source vortex axis, strength, and radial motion." };
		if (ClassName.find("event") != std::string::npos)
			return { "Particle Event",
				"Defines source event generation or receiver behavior." };
		if (ClassName.find("acceleration") != std::string::npos)
			return { "Acceleration",
				"Adds the source acceleration distribution during particle life." };
		if (ClassName.find("velocity") != std::string::npos)
			return { "Initial Velocity",
				"Defines particle launch direction and speed distributions." };
		if (ClassName.find("lifetime") != std::string::npos)
			return { "Lifetime",
				"Defines the minimum, maximum, or curved particle lifetime." };
		if (ClassName.find("spawn") != std::string::npos)
			return { "Spawn",
				"Defines continuous spawn rate, rate scaling, or per-unit spawning." };
		if (ClassName.find("size") != std::string::npos)
			return { "Size",
				"Defines initial size or size scaling over particle life." };
		if (ClassName.find("color") != std::string::npos ||
			ClassName.find("alpha") != std::string::npos)
			return { "Color / Alpha",
				"Defines source color, alpha, and their lifetime curves." };
		if (ClassName.find("location") != std::string::npos)
			return { "Location",
				"Defines emitter-relative position offsets or source-emitter locations." };
		if (ClassName.find("required") != std::string::npos)
			return { "Emitter Contract",
				"Defines renderer alignment, local space, duration, delay, and loop policy." };
		return {};
	}

	std::string Friendly_SourcePropertyLabel(
		const std::string_view strPropertyPath)
	{
		std::string Label(strPropertyPath);
		constexpr std::string_view DistributionSuffix = ".distribution";
		if (Label.ends_with(DistributionSuffix))
			Label.erase(Label.size() - DistributionSuffix.size());
		const size_t iLastDot = Label.rfind('.');
		if (iLastDot != std::string::npos)
			Label.erase(0u, iLastDot + 1u);
		std::string Friendly;
		Friendly.reserve(Label.size() + 8u);
		for (size_t iCharacter = 0u; iCharacter < Label.size(); ++iCharacter)
		{
			const char Character = Label[iCharacter];
			if (Character == '_' || Character == '-')
			{
				if (!Friendly.empty() && Friendly.back() != ' ')
					Friendly.push_back(' ');
				continue;
			}
			if (!Friendly.empty() && iCharacter > 0u && std::isupper(
				static_cast<unsigned char>(Character)) &&
				!std::isupper(static_cast<unsigned char>(Label[iCharacter - 1u])) &&
				Friendly.back() != ' ')
			{
				Friendly.push_back(' ');
			}
			Friendly.push_back(Character);
		}
		return Friendly.empty() ? std::string(strPropertyPath) : Friendly;
	}

	Client::EFFECT_RESOURCE_FILE_KIND Resource_FileKind(
		const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
	{
		const std::string Extension = Lower_Ascii(
			std::filesystem::path(Binding.strAssetId).extension().string());
		return Extension == ".wmodel" ?
			Client::EFFECT_RESOURCE_FILE_KIND::MODEL :
			Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	}

	void Render_SourceMaterialParameterGroups(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial,
		Client::CEffectThumbnailCache* pThumbnailCache)
	{
		const size_t iParameterCount = SourceMaterial.Textures.size() +
			SourceMaterial.Scalars.size() +
			SourceMaterial.Vectors.size() + SourceMaterial.StaticSwitches.size();
		if (!ImGui::CollapsingHeader(("Material Instance Parameters (" +
			std::to_string(iParameterCount) + ")").c_str()))
		{
			return;
		}
		ImGui::TextDisabled(
			"Read-only source names, groups, and resolved MI values.");
		ImGui::TextWrapped("Parent: %s",
			SourceMaterial.strParentMaterialPath.empty() ? "(none)" :
				SourceMaterial.strParentMaterialPath.c_str());
		ImGui::TextWrapped("Profile: %s | Runtime: %s",
			SourceMaterial.strProfileId.empty() ? "(none)" :
				SourceMaterial.strProfileId.c_str(),
			SourceMaterial.strRuntimeShaderProfileId.empty() ? "(none)" :
				SourceMaterial.strRuntimeShaderProfileId.c_str());

		std::set<std::string> Groups;
		const auto AddGroup = [&Groups](const std::string& strGroup)
		{
			Groups.insert(strGroup.empty() ? "(Ungrouped)" : strGroup);
		};
		for (const Client::EFFECT_NAMED_TEXTURE_DESC& Texture :
			SourceMaterial.Textures)
			AddGroup(Texture.strGroup);
		for (const Client::EFFECT_NAMED_FLOAT_DESC& Scalar :
			SourceMaterial.Scalars)
			AddGroup(Scalar.strGroup);
		for (const Client::EFFECT_NAMED_FLOAT4_DESC& Vector :
			SourceMaterial.Vectors)
			AddGroup(Vector.strGroup);
		for (const Client::EFFECT_NAMED_BOOL_DESC& StaticSwitch :
			SourceMaterial.StaticSwitches)
			AddGroup(StaticSwitch.strGroup);
		for (const std::string& Group : Groups)
		{
			const auto MatchesGroup = [&Group](const std::string& strGroup)
			{
				return (strGroup.empty() ? "(Ungrouped)" : strGroup) == Group;
			};
			size_t iGroupCount = 0u;
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.Textures.begin(), SourceMaterial.Textures.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_TEXTURE_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.Scalars.begin(), SourceMaterial.Scalars.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_FLOAT_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.Vectors.begin(), SourceMaterial.Vectors.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_FLOAT4_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			iGroupCount += static_cast<size_t>(std::count_if(
				SourceMaterial.StaticSwitches.begin(),
				SourceMaterial.StaticSwitches.end(),
				[&MatchesGroup](const Client::EFFECT_NAMED_BOOL_DESC& Value)
				{ return MatchesGroup(Value.strGroup); }));
			const std::string GroupLabel = Group + " (" +
				std::to_string(iGroupCount) + ")";
			if (!ImGui::TreeNode(GroupLabel.c_str()))
				continue;
			for (size_t iTexture = 0u;
				iTexture < SourceMaterial.Textures.size(); ++iTexture)
			{
				const Client::EFFECT_NAMED_TEXTURE_DESC& Texture =
					SourceMaterial.Textures[iTexture];
				if (!MatchesGroup(Texture.strGroup))
					continue;
				const auto AddressLabel = [](
					const Client::EFFECT_TEXTURE_ADDRESS_MODE eMode)
				{
					return eMode == Client::EFFECT_TEXTURE_ADDRESS_MODE::CLAMP ?
						"Clamp" : "Wrap";
				};
				const char* pColorSpace =
					Texture.eColorSpace ==
						Client::EFFECT_TEXTURE_COLOR_SPACE::SRGB ?
					"sRGB" : "Linear";
				ImGui::PushID(static_cast<int>(iTexture));
				ImGui::BeginGroup();
				if (nullptr != pThumbnailCache && !Texture.strAssetId.empty())
				{
					const Client::CEffectThumbnailCache::RESULT Thumbnail =
						pThumbnailCache->Request(Texture.strAssetId,
							Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE);
					if (nullptr != Thumbnail.pTextureView)
						ImGui::Image(Thumbnail.pTextureView, ImVec2(48.f, 48.f));
					else
					{
						ImGui::Button("DDS", ImVec2(48.f, 48.f));
						if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
							ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
					}
				}
				else
					ImGui::Button("DDS", ImVec2(48.f, 48.f));
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Text("Texture | %s", Texture.strName.c_str());
				ImGui::TextWrapped("DDS: %s",
					Texture.strAssetId.empty() ? "(unresolved)" :
						Texture.strAssetId.c_str());
				if (ImGui::IsItemHovered() &&
					!Texture.strSourceObjectPath.empty())
				{
					ImGui::SetTooltip("Source: %s",
						Texture.strSourceObjectPath.c_str());
				}
				ImGui::TextDisabled("Address U/V: %s / %s | %s",
					AddressLabel(Texture.eAddressU),
					AddressLabel(Texture.eAddressV), pColorSpace);
				ImGui::TextDisabled("Sampling: %s",
					Texture.strSamplingEvidence.c_str());
				ImGui::EndGroup();
				ImGui::EndGroup();
				ImGui::PopID();
			}
			for (const Client::EFFECT_NAMED_FLOAT_DESC& Scalar :
				SourceMaterial.Scalars)
			{
				if (MatchesGroup(Scalar.strGroup))
					ImGui::BulletText("Scalar | %s = %.9g",
						Scalar.strName.c_str(), Scalar.fValue);
			}
			for (const Client::EFFECT_NAMED_FLOAT4_DESC& Vector :
				SourceMaterial.Vectors)
			{
				if (MatchesGroup(Vector.strGroup))
					ImGui::BulletText("Vector | %s = [%.6g, %.6g, %.6g, %.6g]",
						Vector.strName.c_str(), Vector.vValue.x, Vector.vValue.y,
						Vector.vValue.z, Vector.vValue.w);
			}
			for (const Client::EFFECT_NAMED_BOOL_DESC& StaticSwitch :
				SourceMaterial.StaticSwitches)
			{
				if (MatchesGroup(StaticSwitch.strGroup))
					ImGui::BulletText("Static Switch | %s = %s",
						StaticSwitch.strName.c_str(),
						StaticSwitch.bValue ? "true" : "false");
			}
			ImGui::TreePop();
		}
		if (0u == iParameterCount)
			ImGui::TextDisabled("(no named MI parameters or texture lanes captured)");
		ImGui::TextDisabled("Dynamic semantics: X=%s | Y=%s | Z=%s | W=%s",
			SourceMaterial.DynamicParameterSemantics[0].c_str(),
			SourceMaterial.DynamicParameterSemantics[1].c_str(),
			SourceMaterial.DynamicParameterSemantics[2].c_str(),
			SourceMaterial.DynamicParameterSemantics[3].c_str());
		ImGui::TextDisabled("SubUV mode: %s",
			SourceMaterial.strSubUVMode.c_str());
	}

    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_KIND eKind,
        const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
		if (Client::EFFECT_ELEMENT_KIND::LIGHT == eKind ||
			Client::EFFECT_ELEMENT_KIND::SCREEN_POST == eKind)
		{
			return false;
		}
        if (Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
            return Client::EFFECT_ELEMENT_KIND::MESH == eKind ||
                Client::EFFECT_ELEMENT_KIND::PARTICLE == eKind;
        return eKind < Client::EFFECT_ELEMENT_KIND::END &&
            eSlot >= Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE &&
            eSlot <= Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE;
    }

    Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_RESOURCE_SLOT eSlot)
    {
        return Client::EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot ?
            Client::EFFECT_RESOURCE_FILE_KIND::MODEL :
            Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    }

    std::string Default_SlotId(const Client::EFFECT_ELEMENT_KIND eKind)
    {
        return Client::EFFECT_ELEMENT_KIND::MESH == eKind ?
            std::string(Client::EFFECT_MESH_SHAPE_SLOT_ID) :
            std::string(Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
    }

    const Client::EFFECT_RESOURCE_BINDING_DESC* Find_Binding(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        const auto Iterator = std::find_if(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [strSlotId](const Client::EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == strSlotId;
            });
        return Iterator == Element.ResourceBindings.end() ?
            nullptr : &*Iterator;
    }

	bool Is_SourceDecalBaseAdmissionCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::EFFECT_ELEMENT_KIND::DECAL == Element.eKind &&
			Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "decal" &&
			Element.Material.strTemplateId ==
				Client::EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			!Element.Material.SourceMaterial.bEnabled &&
			!Element.Material.Execution.bEnabled;
	}

	bool Has_BaseTextureBinding(const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const Client::EFFECT_RESOURCE_BINDING_DESC* pBase = Find_Binding(
			Element, Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
		return nullptr != pBase && !pBase->strAssetId.empty();
	}

	bool Is_MissingBaseSourceDecal(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Is_SourceDecalBaseAdmissionCarrier(Element) &&
			Element.Material.Execution.bFailClosed &&
			!Has_BaseTextureBinding(Element);
	}

	bool Is_BaseTextureSlot(const std::string_view strSlotId)
	{
		return strSlotId ==
			Client::EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId;
	}

	bool Is_ElementPreviewAdmitted(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		if (Element.eKind == Client::EFFECT_ELEMENT_KIND::LIGHT ||
			Element.eKind == Client::EFFECT_ELEMENT_KIND::SCREEN_POST)
		{
			return Client::Is_EffectToolPresentationPreviewAdmitted(Element);
		}
		// An authoring-approximate carrier owns its exact source resources and
		// only lacks proven material semantics.  It previews so the artist can
		// tune it; product admission is refused elsewhere and is unaffected.
		return Element.bVisible &&
			Client::Is_EffectAuthoringExecutionTarget(
				Element.Material.Execution);
	}

    struct PARTICLE_LAYER_SUMMARY final
    {
        size_t iStandaloneMeshCount = 0u;
        size_t iStandaloneSpriteCount = 0u;
        size_t iSourceSystemCount = 0u;
        size_t iSourceEmitterCount = 0u;
        size_t iLayerCount = 0u;
        size_t iMeshRendererCount = 0u;
        size_t iSpriteRendererCount = 0u;
        size_t iUnresolvedRendererCount = 0u;
        uint64_t iParticleBudget = 0u;
    };

    PARTICLE_LAYER_SUMMARY Summarize_ParticleLayers(
        const Client::EFFECT_DOCUMENT_DESC& Document)
    {
        PARTICLE_LAYER_SUMMARY Summary;
        struct SOURCE_EMITTER_BUDGET final
        {
            uint32_t iMaxParticles = 0u;
            bool_t bHasBaseLayer = false;
        };
        std::map<std::string, SOURCE_EMITTER_BUDGET> SourceEmitterBudgets;
        std::set<std::string> SourceSystems;
        for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
        {
            if (Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind)
            {
                ++Summary.iStandaloneMeshCount;
                continue;
            }
            if (Client::EFFECT_ELEMENT_KIND::SPRITE == Element.eKind)
            {
                ++Summary.iStandaloneSpriteCount;
                continue;
            }
            if (Client::EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind)
                continue;
            ++Summary.iLayerCount;
            if (!Element.strGroupId.empty())
                SourceSystems.insert(Element.strGroupId);
            Summary.iParticleBudget += Element.Detail.Particle.iMaxParticles;
            switch (Resolve_CascadeRendererKind(Element))
            {
            case CASCADE_RENDERER_KIND::MESH:
                ++Summary.iMeshRendererCount;
                break;
            case CASCADE_RENDERER_KIND::SPRITE:
                ++Summary.iSpriteRendererCount;
                break;
            case CASCADE_RENDERER_KIND::UNRESOLVED:
            default:
                ++Summary.iUnresolvedRendererCount;
                break;
            }

            if (!Element.strSourceNode.empty())
            {
                std::string strSourceEmitter = Element.strSourceNode;
                const size_t iBurstMarker = strSourceEmitter.rfind("|burst:");
                const bool_t bBurstLayer = std::string::npos != iBurstMarker ||
                    std::string::npos != Element.strDisplayName.rfind(" Burst ");
                if (std::string::npos != iBurstMarker)
                    strSourceEmitter.erase(iBurstMarker);
                SOURCE_EMITTER_BUDGET& EmitterBudget =
                    SourceEmitterBudgets[strSourceEmitter];
                EmitterBudget.iMaxParticles = (std::max)(
                    EmitterBudget.iMaxParticles,
                    Element.Detail.Particle.iMaxParticles);
                EmitterBudget.bHasBaseLayer =
                    EmitterBudget.bHasBaseLayer || !bBurstLayer;
            }
        }
        Summary.iSourceSystemCount = SourceSystems.size();
        Summary.iSourceEmitterCount = SourceEmitterBudgets.size();
        for (const auto& Entry : SourceEmitterBudgets)
        {
            const SOURCE_EMITTER_BUDGET& EmitterBudget = Entry.second;
            if (!EmitterBudget.bHasBaseLayer)
                Summary.iParticleBudget += EmitterBudget.iMaxParticles;
        }
        return Summary;
    }

	bool_t Is_SourceParticleCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		if (!Element.SourceRecipe.bEnabled)
			return false;
		const std::string_view Shape = Element.SourceRecipe.strRendererShape;
		return Shape == "mesh" || Shape == "sprite" || Shape == "decal";
	}

	bool_t Is_PreviewParticleSimulationElement(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
			Is_SourceParticleCarrier(Element);
	}

	f32_t Element_PreviewEndSeconds(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const Client::EFFECT_TIMING_DESC& Timing = Element.Detail.Timing;
		f32_t fTail = 0.f;
		if (Is_PreviewParticleSimulationElement(Element))
		{
			fTail = Element.Detail.Particle.vLifeTimeSeconds.y *
				(Element.SourceRecipe.bEnabled ?
					Element.Detail.Particle.SourceScale.fLifeTime : 1.f);
		}
        else if (Client::EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
            fTail = Element.Detail.Trail.fPointLifeTimeSeconds;
		f32_t fEmissionDuration = Timing.fLifeTimeSeconds;
		if (Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.fEmitterDurationSeconds > 0.f &&
			0u != Element.SourceRecipe.iEmitterLoopCount)
		{
			fEmissionDuration = Element.SourceRecipe.fEmitterDurationSeconds *
				static_cast<f32_t>(Element.SourceRecipe.iEmitterLoopCount);
		}
		return Timing.fStartDelaySeconds +
			(Element.SourceRecipe.bEnabled ?
				Element.SourceRecipe.fEmitterDelaySeconds : 0.f) +
			fEmissionDuration + Timing.fAfterImageSeconds + fTail;
	}

    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
		if (Client::EFFECT_ELEMENT_KIND::LIGHT == Element.eKind ||
			Client::EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind)
		{
			return false;
		}
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind ||
                Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
		if (nullptr != Find_Binding(Element, strSlotId))
			return true;
        return nullptr != Client::Find_EffectMaterialInput(
            Element.Material.strTemplateId, strSlotId);
    }

	bool Is_DirectHandAuthoredElement(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const bool_t bDirectHandAuthored = Element.strSourceNode.empty() ||
			Element.strSourceNode.starts_with("authored-copy:");
		return bDirectHandAuthored && !Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			!Element.Material.SourceMaterial.bEnabled &&
			Element.Material.strSourceMaterialPath.empty() &&
			!Element.Material.Execution.bEnabled;
	}

	bool Is_OptionalHandAuthoredResourceSlot(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId)
	{
		if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID ||
			strSlotId == "base" || !Is_DirectHandAuthoredElement(Element))
		{
			return false;
		}
		return nullptr != Client::Find_EffectMaterialInput(
			Element.Material.strTemplateId, strSlotId);
	}

	Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
		if (const Client::EFFECT_RESOURCE_BINDING_DESC* pBinding =
			Find_Binding(Element, strSlotId))
		{
			return Resource_FileKind(*pBinding);
		}
        const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
            Client::Find_EffectMaterialInput(
                Element.Material.strTemplateId, strSlotId);
        return nullptr == pInput ? Client::EFFECT_RESOURCE_FILE_KIND::END :
            pInput->eAllowedResourceKind;
    }

    std::string Slot_Label(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return "Mesh Shape";
        const Client::EFFECT_MATERIAL_INPUT_SLOT_DESC* pInput =
            Client::Find_EffectMaterialInput(
                Element.Material.strTemplateId, strSlotId);
        return nullptr == pInput ? std::string(strSlotId) :
            std::string(pInput->strDisplayName);
    }

    bool InputFloat2(const char* Label, float2_t& Value)
    {
        return ImGui::InputFloat2(Label, &Value.x, "%.3f");
    }

    bool InputFloat3(const char* Label, float3_t& Value)
    {
        return ImGui::InputFloat3(Label, &Value.x, "%.3f");
    }

    bool InputFloat4(const char* Label, float4_t& Value)
    {
        return ImGui::InputFloat4(Label, &Value.x, "%.3f");
    }

    bool DragFloat2(
        const char* Label,
        float2_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f")
    {
        return ImGui::DragFloat2(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    bool DragFloat3(
        const char* Label,
        float3_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f")
    {
        return ImGui::DragFloat3(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    bool DragFloat4(
        const char* Label,
        float4_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f")
    {
        return ImGui::DragFloat4(
            Label, &Value.x, Speed, Minimum, Maximum, Format,
            ImGuiSliderFlags_AlwaysClamp);
    }

    void Copy_Buffer(char* pDestination, const size_t iCapacity,
        const std::string& Source)
    {
        if (nullptr == pDestination || 0u == iCapacity)
            return;
        const size_t Count = (std::min)(iCapacity - 1u, Source.size());
        std::memcpy(pDestination, Source.data(), Count);
        pDestination[Count] = '\0';
    }

    bool_t Is_ManualElementGroupMember(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
		return Element.strGroupId.starts_with("manual.");
    }

    std::string ManualGroup_Label(const std::string& strGroupId)
    {
        const size_t iSeparator = strGroupId.find_last_of('.');
        const std::string strLeaf = std::string::npos == iSeparator ?
            strGroupId : strGroupId.substr(iSeparator + 1u);
        if (strLeaf.starts_with("hit") && strLeaf.size() > 3u)
            return "Hit " + strLeaf.substr(3u) + " | " + strGroupId;
        return strGroupId;
    }

    std::string ManualElement_Label(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
        const size_t iSeparator = Element.strElementId.find_last_of('.');
        const std::string strLeaf = std::string::npos == iSeparator ?
            Element.strElementId : Element.strElementId.substr(iSeparator + 1u);
        return std::string(Element.bVisible ? "[ON] " : "[OFF] ") +
            strLeaf + "##" + Element.strElementId;
    }

    float4x4_t Identity_Matrix()
    {
        float4x4_t Result{};
        XMStoreFloat4x4(&Result, XMMatrixIdentity());
        return Result;
    }

    const Client::CHARACTER_SPEC* Resolve_CurrentTargetSpec()
    {
        const std::string assetName =
            Client::CAnimationTargetService::Resolve_AssetName();
        const Client::CHARACTER_SPEC* specs[] =
        {
            &Client::Spec_LanceMaster,
            &Client::Spec_GunSlinger,
            &Client::Spec_Slayer,
            &Client::Spec_Artist,
            &Client::Spec_DimensionMaster,
            &Client::Spec_Warlord
        };
        for (const Client::CHARACTER_SPEC* pSpec : specs)
        {
            if (nullptr != pSpec && nullptr != pSpec->pAssetName &&
                assetName == pSpec->pAssetName)
                return pSpec;
        }
        return nullptr;
    }
}

Client::CEffect_Tool::CEffect_Tool(
    ComPtr<ID3D11Device> pDevice,
    ComPtr<ID3D11DeviceContext> pContext,
    shared_ptr<CCharacterPreviewPanel> pCharacterPreviewPanel)
    : m_pDevice(std::move(pDevice)),
      m_pContext(std::move(pContext)),
      m_pThumbnailCache(std::make_unique<CEffectThumbnailCache>(
          m_pDevice, m_pContext)),
      m_pCharacterPreviewPanel(std::move(pCharacterPreviewPanel)),
      m_PreviewWorldRoot(Identity_Matrix())
{
    Copy_Buffer(m_PreviewAnchorBuffer.data(),
        m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
    Reset_MeshAuthoringDraft();
}

Client::CEffect_Tool::~CEffect_Tool()
{
    m_pCharacterPreviewPanel->Set_SessionLock(
        CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL, false, {});
    Release_WorldPreview(true);
}

void Client::CEffect_Tool::Update(const f32_t fTimeDelta)
{
    ++m_iFrameNumber;
	Update_ValtanServerPatternAudition();
    m_pThumbnailCache->Begin_Frame(m_iFrameNumber);
    m_pCharacterPreviewPanel->Set_SessionLock(
        CHARACTER_PREVIEW_LOCK_OWNER::EFFECT_TOOL,
        Has_UnsavedWork(),
        "Apply or discard Effect changes before changing target.");
    m_pCharacterPreviewPanel->Refresh_Level();
    const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
    if (m_iWorldPreviewLevel != UINT32_MAX &&
        m_iWorldPreviewLevel != iCurrentLevel)
    {
        Release_WorldPreview(false);
    }
	if (m_bReconstructedDiagnosticActive)
	{
		Update_ReconstructedDiagnosticRoot();
		return;
	}
	if (m_bReconstructedSourceRuntimeActive)
	{
		Update_SynchronizedAnimationSequence();
		Update_ReconstructedSourceRuntimeTimeline(fTimeDelta);
		return;
	}
    Update_SynchronizedAnimationSequence();
    if (!m_ActiveDocument.has_value() &&
        !(m_ProductPreview.has_value() &&
          m_SourcePreviewDocument.has_value()))
        return;
    f32_t fSequentialAdvance = 0.f;
    bool_t bSeekAfterLoop = false;
    if (m_bPreviewPlaying)
    {
        const f32_t fPreviousTime = m_fPreviewTimeSeconds;
        const f32_t fPreviousEffectTime =
            Resolve_EffectSampleTime(fPreviousTime);
        f32_t fSynchronizedAnimationTime = 0.f;
        const bool_t bAnimationOwnsTime =
            Try_ResolveSynchronizedAnimationTime(fSynchronizedAnimationTime);
        if (bAnimationOwnsTime)
        {
            m_fPreviewTimeSeconds =
                (std::max)(0.f, fSynchronizedAnimationTime);
            bSeekAfterLoop =
                m_fPreviewTimeSeconds + 0.0001f < fPreviousTime;
        }
        else
        {
            m_fPreviewTimeSeconds += (std::max)(0.f, fTimeDelta);
        }
        if (m_fPreviewTimeSeconds > m_fPreviewDurationSeconds)
        {
            if (m_bPreviewLoop)
            {
                m_fPreviewTimeSeconds = std::fmod(
                    m_fPreviewTimeSeconds, m_fPreviewDurationSeconds);
                bSeekAfterLoop = true;
            }
            else
            {
                m_fPreviewTimeSeconds = m_fPreviewDurationSeconds;
                m_bPreviewPlaying = false;
                bSeekAfterLoop = true;
            }
        }
        if (!bSeekAfterLoop)
            fSequentialAdvance = (std::max)(
                0.f, Resolve_EffectSampleTime(m_fPreviewTimeSeconds) -
                    fPreviousEffectTime);
    }
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject)
        return;
    if (pObject->Is_RenderFailureIsolated())
    {
        pObject->Set_Playing(false);
        pObject->Set_Visible(false);
        m_bPreviewPlaying = false;
        m_bPreviewVisibleRequested = false;
        Set_SynchronizedAnimationPaused(true);
        m_strPreviewStatus = pObject->Get_Status();
        return;
    }
	if (m_bValtanBossPatternTransformHistoryRequired)
	{
		if (!m_bValtanBossPatternTransformHistoryActive ||
			!m_ActiveDocument.has_value() ||
			m_ActiveDocument->strEffectAssetId !=
				m_strValtanBossPatternPreviewEffectAssetId)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			Set_SynchronizedAnimationPaused(true);
			m_strPreviewStatus =
				"World preview hidden: Valtan 420633 exact b_effectroot history is unavailable.";
			return;
		}
		if (bSeekAfterLoop)
		{
			Reset_ProductCueSnapshot();
			Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
			[this](const f32_t fSampleTimeSeconds,
				EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
				std::string& strOutError)
			{
				return Build_ValtanBossPatternTransformSample(
					fSampleTimeSeconds, OutSample, strOutError);
			};
		const f32_t fEffectSampleTime =
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
		std::string TransformError;
		bool_t bHistoryAdvanced = true;
		if (bSeekAfterLoop ||
			std::abs(pObject->Get_PreviewFixedStepClockSeconds() -
				static_cast<f64_t>(fEffectSampleTime)) > 1.0e-5 &&
			fSequentialAdvance <= 0.f)
		{
			bHistoryAdvanced = pObject->Set_SampleTimeWithTransformHistory(
				fEffectSampleTime, TransformProvider, TransformError);
		}
		else if (fSequentialAdvance > 0.f)
		{
			bHistoryAdvanced = pObject->Advance_PreviewWithTransformHistory(
				fSequentialAdvance, TransformProvider, TransformError);
		}
		if (!bHistoryAdvanced)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			Set_SynchronizedAnimationPaused(true);
			m_bValtanBossPatternTransformHistoryActive = false;
			m_strPreviewStatus =
				"World preview hidden: Valtan 420633 anchor history failed: " +
				TransformError;
			return;
		}
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		if (m_bPreviewVisibleRequested)
		{
			m_strPreviewStatus =
				"Valtan 420633 preview follows exact B_EffectRoot / b_effectroot history.";
		}
		return;
	}
    if (bSeekAfterLoop)
    {
        Reset_ProductCueSnapshot();
        Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
    }
	/* Resolve follow anchors after any loop seek so the world submitted for this
	   playback tick reflects the newly selected animation pose, not the final
	   pose from the previous loop. */
	const EFFECT_DOCUMENT_DESC& SourceAnchorDocument =
		m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value() ?
			*m_SourcePreviewDocument : *m_ActiveDocument;
	std::unordered_map<std::string, float4x4_t> SourceAnchorWorlds;
	std::string SourceAnchorError;
	const bool_t bSourceAnchorsResolved = Resolve_ToolSourceAnchorWorlds(
		SourceAnchorDocument, SourceAnchorWorlds, SourceAnchorError);
	pObject->Set_SourceAnchorWorlds(std::move(SourceAnchorWorlds));
	if (!bSourceAnchorsResolved)
	{
		m_strPreviewStatus =
			"World preview source anchor unavailable: " + SourceAnchorError;
	}
	else if (0u == m_strPreviewStatus.find(
		"World preview source anchor unavailable:"))
	{
		m_strPreviewStatus = "World preview source anchors resolved.";
	}
    float4x4_t Root{};
    const bool_t bRootResolved = Resolve_PreviewRoot(Root);
    const bool_t bCueVisible =
        Is_ProductCueVisible(m_fPreviewTimeSeconds);
    pObject->Set_Visible(
        m_bPreviewVisibleRequested && bRootResolved && bCueVisible);
    if (m_bPreviewVisibleRequested && bRootResolved && bCueVisible)
    {
        if (0u == m_strPreviewStatus.find("World preview hidden:"))
            m_strPreviewStatus = "World preview anchor resolved.";
    }
    else if (m_bPreviewVisibleRequested && bRootResolved &&
		!bCueVisible && Has_ProductCuePreview())
    {
        m_strPreviewStatus = "Product cue is outside its admitted start/end window.";
    }
    else if (m_bPreviewVisibleRequested)
    {
        if (Has_ProductCuePreview())
        {
			const std::string& strCueAnchor = m_ProductPreview.has_value() ?
				m_ProductPreview->ProductCue.Cue.strAnchorSlotId :
				m_ValtanProductPreview->Cue.strAnchorSlotId;
            m_strPreviewStatus = "World preview hidden: Product cue cannot resolve " +
                ("root" == strCueAnchor ? std::string("its root anchor.") :
                    std::string("anchor '") + strCueAnchor + "'.");
        }
        else
        {
            m_strPreviewStatus = "World preview hidden: current target cannot resolve " +
                (EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                    std::string("its root pivot.") :
                    std::string("anchor '") + m_strPreviewAnchorSlotId + "'.");
        }
    }
    if (!m_bPreviewVisibleRequested)
		return;
    if (bSeekAfterLoop)
    {
		const f32_t fEffectSampleSeconds =
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
		std::string HistoryError;
		if (!Seek_WorldPreviewWithSourceAnchorHistory(
				pObject, SourceAnchorDocument,
				fEffectSampleSeconds, HistoryError))
		{
			if (bRootResolved)
				pObject->Set_RootWorld(Root);
			pObject->Set_SampleTime(fEffectSampleSeconds);
			m_strPreviewStatus =
				"Loop seek used current-pose fallback: " + HistoryError;
		}
    }

	else if (bRootResolved && fSequentialAdvance > 0.f)
		pObject->Advance_Preview(fSequentialAdvance, Root);
	else if (bRootResolved)
		pObject->Set_RootWorld(Root);
}

void Client::CEffect_Tool::Render()
{
    Engine::CProfilerScope Profile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.Render");
    {
        Engine::CProfilerScope InitialIndexProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.InitialIndexStep");
        if (!m_bResourceCatalogRefreshAttempted)
            Refresh_ResourceCatalog();
        else if (!m_bAllEffectsRefreshAttempted)
            Refresh_AllEffects();
        else if (!m_bDataFilesRefreshAttempted)
            Refresh_DataFiles();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.AuthoringWindow");
        Render_EffectToolWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.ModelViewWindow");
        Render_ModelViewWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DetailWindow");
        Render_EffectDetailWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.AllEffectsWindow");
        Render_AllEffectsWindow();
    }
    {
        Engine::CProfilerScope WindowProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DataFilesWindow");
        Render_DataFilesWindow();
    }
    {
        Engine::CProfilerScope TrimProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.ThumbnailTrim");
        m_pThumbnailCache->Trim();
    }
}

void Client::CEffect_Tool::Render_EffectToolWindow()
{
    ImGui::SetNextWindowPos(ImVec2(10.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620.f, 760.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(560.f, 680.f), ImVec2(2200.f, 2200.f));
    const bool_t bWindowVisible = ImGui::Begin("Effect Tool");
    Render_PendingDocumentLoadModal();
    if (!bWindowVisible)
    {
        ImGui::End();
        return;
    }
    ImGui::TextUnformatted(
        "Build individual Elements, combine them into one Effect, then tune each Element in Effect Detail.");
    const ImGuiIO& IO = ImGui::GetIO();
    ImGui::TextDisabled("FPS %.1f | Frame %.2f ms",
        IO.Framerate,
        IO.DeltaTime > 0.f ? IO.DeltaTime * 1000.f : 0.f);
	Render_ActiveAuthoredEffectTree();
    Render_MeshAuthoringWorkbench();
	if (ImGui::CollapsingHeader("Selected Element Resources",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		const bool_t bAdapterPacketInspection =
			EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
				m_eActiveDocumentSource &&
			nullptr != m_pSelectedVisualSourceProjection &&
			m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
		ImGui::TextDisabled(
			bAdapterPacketInspection ?
			"Exact adapter resource bindings are read-only here; ordinary authored Save As cannot preserve their projector/VF packet." :
			"Select one Element under Current Effect, then bind or replace its WModel/DDS slots here.");
		ImGui::BeginDisabled(bAdapterPacketInspection);
        Render_ResourceSlots(false);
        Render_ResourceGrid(false);
		ImGui::EndDisabled();
    }
    if (!m_strResourceStatus.empty())
        ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
    ImGui::End();
}

void Client::CEffect_Tool::Render_MeshAuthoringWorkbench()
{
    if (!m_bMeshAuthoringDraftInitialized)
        Reset_MeshAuthoringDraft();
    ImGui::SeparatorText("Element Authoring");
	static constexpr std::array<EFFECT_AUTHORING_FAMILY, 6u> FAMILIES{
		EFFECT_AUTHORING_FAMILY::MESH,
		EFFECT_AUTHORING_FAMILY::SPRITE,
		EFFECT_AUTHORING_FAMILY::MESH_PARTICLE,
		EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE,
		EFFECT_AUTHORING_FAMILY::LOCAL_DECAL,
		EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON };
	ImGui::TextUnformatted("Element Type");
	for (size_t iFamily = 0u; iFamily < FAMILIES.size(); ++iFamily)
	{
		if (0u != iFamily % 3u)
			ImGui::SameLine();
		const EFFECT_AUTHORING_FAMILY eFamily = FAMILIES[iFamily];
		if (ImGui::RadioButton(AuthoringFamily_Label(eFamily),
			m_eSelectedAuthoringFamily == eFamily))
		{
			const std::string strLayerId = m_NewElementId.data();
			m_eSelectedAuthoringFamily = eFamily;
			m_eSelectedEffectType = AuthoringFamily_Kind(eFamily);
			Reset_MeshAuthoringDraft();
			Copy_Buffer(m_NewElementId.data(), m_NewElementId.size(), strLayerId);
		}
	}
	if (m_SourceElementPresetSelection.has_value())
	{
		const SOURCE_ELEMENT_PRESET_SELECTION& Loaded =
			*m_SourceElementPresetSelection;
		ImGui::SeparatorText("Imported Element Draft");
		ImGui::TextDisabled(
			"Editable seed copy. Current Effect changes only after Create Element; Save Changes is the only Data File write.");
		ImGui::Text("Editable Type: %s | Source Family: %s",
			AuthoringFamily_Label(m_eSelectedAuthoringFamily),
			Loaded.strSourceFamily.c_str());
		static constexpr std::array<std::string_view, 9u> SUMMARY_SLOTS{
			EFFECT_MESH_SHAPE_SLOT_ID, "base", "noise", "mask",
			"emissive", "dissolve", "base2", "mask2", "noise2" };
		static constexpr std::array<const char*, 9u> SUMMARY_LABELS{
			"WModel", "Base", "Noise", "Mask", "Emissive", "Dissolve",
			"Base 2", "Mask 2", "Noise 2" };
		for (size_t iSlot = 0u; iSlot < SUMMARY_SLOTS.size(); ++iSlot)
		{
			const EFFECT_RESOURCE_BINDING_DESC* pBinding = Find_Binding(
				m_MeshAuthoringDraft, SUMMARY_SLOTS[iSlot]);
			ImGui::TextWrapped("%s: %s", SUMMARY_LABELS[iSlot],
				nullptr == pBinding ? "(not bound)" :
					pBinding->strAssetId.c_str());
		}
		if (ImGui::TreeNodeEx("Source Identity",
			ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::TextWrapped("%s", Loaded.strSourceRecordId.c_str());
			ImGui::TreePop();
		}
	}
	ImGui::SeparatorText("New Effect");
    ImGui::InputText("Effect Name", m_NewAssetId.data(),
        m_NewAssetId.size());
    ImGui::InputText("Display Name (optional)", m_NewDisplayName.data(),
        m_NewDisplayName.size());
	const bool_t bHasEffectName = '\0' != m_NewAssetId[0u];
	ImGui::BeginDisabled(!bHasEffectName || Has_UnsavedWork());
	if (ImGui::Button("New Effect"))
		Try_CreateDocument();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Creates an unsaved Current Effect. No file is written until Save Changes.");

	ImGui::SeparatorText("Create Element Draft");
    ImGui::InputText("Layer Name (optional)", m_NewElementId.data(),
        m_NewElementId.size());
    ImGui::InputText("Resource Filter", m_ResourceFilter.data(),
        m_ResourceFilter.size());

	const EFFECT_RESOURCE_BINDING_DESC* pMesh = Find_Binding(
		m_MeshAuthoringDraft, EFFECT_MESH_SHAPE_SLOT_ID);
	const bool_t bArtistFSeedTargetsDifferentParent =
		m_SourceElementPresetSelection.has_value() &&
		m_SourceElementPresetSelection->strSourceEffectAssetId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		(!m_ActiveDocument.has_value() ||
		 m_ActiveDocument->strEffectAssetId !=
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID);
	const bool_t bMeshParticleNeedsCarrier =
		EFFECT_AUTHORING_FAMILY::MESH_PARTICLE ==
			m_eSelectedAuthoringFamily && nullptr == pMesh;
	const bool_t bCanCreateElement =
		m_ActiveDocument.has_value() &&
		!Has_UnappliedDetailDraft() && !m_bOccurrenceTuningDirty &&
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
		!bMeshParticleNeedsCarrier && !bArtistFSeedTargetsDifferentParent;

	if (ImGui::Button("Reset Element Draft"))
		Reset_MeshAuthoringDraft();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanCreateElement);
	if (ImGui::Button("Create Element"))
		Try_CreateElementDraft();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Refresh Resources"))
	{
		Refresh_ResourceCatalog();
	}

	if (!m_ActiveDocument.has_value() ||
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		 EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource))
	{
		ImGui::TextDisabled(
			"Create or open one editable Current Effect before adding an Element.");
	}
	else if (bMeshParticleNeedsCarrier)
	{
		ImGui::TextDisabled(
			"Choose a WModel seed for Mesh Particle so its Family remains stable. After creation, bind DDS in Selected Element Resources above.");
	}
	else if (bArtistFSeedTargetsDifferentParent)
	{
		ImGui::TextDisabled(
			"Open Artist F > Editable Skill Effect before creating this Track A Seed.");
	}
	else
		ImGui::TextDisabled(
			"Create adds an unsaved Element to Current Effect. Select it, bind WModel/DDS slots, tune Details/Visible, then use Save Changes once.");

	if (m_SourceElementPresetSelection.has_value() ||
		EFFECT_AUTHORING_FAMILY::MESH_PARTICLE == m_eSelectedAuthoringFamily)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::CollapsingHeader("Optional Element Seed Resources"))
		{
			Render_ResourceSlots(true);
			Render_ResourceGrid(true);
		}
	}
}

void Client::CEffect_Tool::Render_PendingDocumentLoadModal()
{
    if (m_bPendingDocumentLoadModalRequested &&
        m_PendingDocumentLoad.has_value())
    {
        ImGui::OpenPopup("Unsaved Effect Changes");
        m_bPendingDocumentLoadModalRequested = false;
    }
    if (!ImGui::BeginPopupModal(
        "Unsaved Effect Changes", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }

    const char* pCurrentAsset = m_ActiveDocument.has_value() ?
        m_ActiveDocument->strEffectAssetId.c_str() : "(none)";
    const char* pTargetAsset = m_PendingDocumentLoad.has_value() ?
        m_PendingDocumentLoad->strSelectionId.c_str() : "(none)";
    ImGui::Text("Current: %s", pCurrentAsset);
    ImGui::Text("Load: %s", pTargetAsset);
    ImGui::Separator();
    ImGui::TextWrapped(
        "The active Effect has unsaved changes. Choose how to continue.");
    if (Has_UnappliedDetailDraft())
    {
        ImGui::TextDisabled(
            "Save & Load requires Apply or Revert for the open Detail draft first.");
    }

    ImGui::BeginDisabled(Has_UnappliedDetailDraft());
    if (ImGui::Button("Save & Load"))
    {
        if (Execute_PendingDocumentLoad(true))
            ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Discard & Load"))
    {
        if (Execute_PendingDocumentLoad(false))
            ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        m_PendingDocumentLoad.reset();
        m_strDocumentStatus =
            "Cancelled the pending Effect document load.";
        ImGui::CloseCurrentPopup();
    }
    if (!m_strDocumentStatus.empty())
        ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
    ImGui::EndPopup();
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
    ImGui::TextUnformatted("Effect Type");
    for (int32_t iKind = 0;
        iKind < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++iKind)
    {
        if (0 != iKind)
            ImGui::SameLine();
        const EFFECT_ELEMENT_KIND eKind =
            static_cast<EFFECT_ELEMENT_KIND>(iKind);
        if (ImGui::RadioButton(Kind_Label(eKind),
            m_eSelectedEffectType == eKind))
        {
            m_eSelectedEffectType = eKind;
            m_strSelectedResourceSlotId = Default_SlotId(eKind);
            m_eResourceLibraryFileKind = EFFECT_ELEMENT_KIND::MESH == eKind ?
                EFFECT_RESOURCE_FILE_KIND::MODEL :
                EFFECT_RESOURCE_FILE_KIND::TEXTURE;
        }
    }
}

void Client::CEffect_Tool::Render_ResourceSlots(
    const bool_t bMeshAuthoringDraft)
{
    const EFFECT_ELEMENT_DESC* pElement = bMeshAuthoringDraft ?
        &m_MeshAuthoringDraft : Find_SelectedElement();
	ImGui::SeparatorText(bMeshAuthoringDraft ?
		"Element Resource Slots" : "Selected Element Resource Set");
    if (nullptr == pElement)
    {
		if (!m_ActiveDocument.has_value())
		{
			ImGui::TextDisabled("Select a Skill, Component, or Emitter in All Effects.");
			return;
		}
		ImGui::TextWrapped("Skill: %s",
			m_ActiveDocument->strEffectAssetId.c_str());
		if (EFFECT_DETAIL_SELECTION::SKILL == m_eDetailSelection)
		{
			const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
				CEffectCatalog::Find_Assembly(
					m_ActiveDocument->strEffectAssetId);
			ImGui::TextDisabled("Skill selected | Components %zu | Elements %zu",
				nullptr == Assembly ? 0u : Assembly->ComponentCues.size(),
				m_ActiveDocument->Elements.size());
			ImGui::TextWrapped(
				"A Skill has no single Resource Set. Open its first Emitter or select "
				"a specific Component/Emitter in All Effects.");
			if (ImGui::Button("Open First Emitter##resource.skill"))
				Try_SelectFirstEmitter(
					m_ActiveDocument->strEffectAssetId, {});
		}
		else if (EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM == m_eDetailSelection)
		{
			const PARTICLE_LAYER_SUMMARY Summary =
				Summarize_ParticleLayers(*m_ActiveDocument);
			ImGui::TextDisabled(
				"Cascade System selected | Emitters %zu | Mesh Particles %zu | Sprite Particles %zu | Unresolved %zu",
				Summary.iSourceEmitterCount, Summary.iMeshRendererCount,
				Summary.iSpriteRendererCount, Summary.iUnresolvedRendererCount);
			ImGui::TextWrapped(
				"The Cascade System owns many Resource Sets. Open an Emitter to see "
				"its mesh, every texture binding, and Material Instance parameters.");
			if (ImGui::Button("Open First Emitter##resource.system"))
				Try_SelectFirstEmitter(
					m_ActiveDocument->strEffectAssetId, {});
		}
		else if (EFFECT_DETAIL_SELECTION::COMPONENT == m_eDetailSelection)
		{
			const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
				CEffectCatalog::Find_Component(m_strSelectedComponentId);
			ImGui::TextDisabled("Component selected | Emitters %zu",
				nullptr == Component ? 0u : Component->Emitters.size());
			ImGui::TextWrapped(
				"A Component owns one or more Emitter Resource Sets. Open an Emitter "
				"to inspect or bind its resources.");
			if (nullptr != Component &&
				ImGui::Button("Open First Emitter##resource.component"))
			{
				Try_SelectFirstEmitter(
					m_ActiveDocument->strEffectAssetId,
					Component->strComponentAssetId);
			}
		}
		else
		{
			ImGui::TextDisabled(
				"Select an Emitter to inspect its dynamic Resource Set.");
		}
        return;
    }

    if (EFFECT_ELEMENT_KIND::PARTICLE == pElement->eKind)
        ImGui::Text("%s | %s | %s", pElement->strDisplayName.c_str(),
            Kind_Label(pElement->eKind), Element_RendererLabel(*pElement));
    else
        ImGui::Text("%s | %s", pElement->strDisplayName.c_str(),
            Kind_Label(pElement->eKind));
	ImGui::TextDisabled("Emitter Element ID: %s",
		pElement->strElementId.c_str());
	std::string strResetSlot;
    const auto RenderSlotCard = [this, pElement, &strResetSlot,
		bMeshAuthoringDraft](
		const EFFECT_RESOURCE_BINDING_DESC* pBinding,
		const std::string& strSlotId,
		const std::string& strLabel,
		const EFFECT_RESOURCE_FILE_KIND eFileKind,
		const bool_t bModified)
    {
		ImGui::PushID(strSlotId.c_str());
        ImGui::BeginGroup();
        bool_t bClicked = false;
        if (nullptr != pBinding)
        {
            const CEffectThumbnailCache::RESULT Thumbnail =
                m_pThumbnailCache->Request(pBinding->strAssetId, eFileKind);
            if (nullptr != Thumbnail.pTextureView)
            {
                ImGui::Image(Thumbnail.pTextureView, ImVec2(64.f, 58.f));
                bClicked = ImGui::IsItemClicked();
            }
            else
            {
                bClicked = ImGui::Button(
                    EFFECT_RESOURCE_FILE_KIND::MODEL == eFileKind ?
                        "Mesh" : "DDS", ImVec2(64.f, 58.f));
                if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
                    ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
            }
        }
        else
        {
            bClicked = ImGui::Button("Empty", ImVec2(64.f, 58.f));
        }
        if (m_strSelectedResourceSlotId == strSlotId)
        {
            ImGui::GetWindowDrawList()->AddRect(
                ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 2.f);
        }
        if (bClicked)
        {
            m_strSelectedResourceSlotId = strSlotId;
            m_strSelectedResourceAssetId.clear();
            m_eResourceLibraryFileKind = eFileKind;
        }
		std::string ShortLabel = strLabel;
		if (ShortLabel.size() > 11u)
			ShortLabel = ShortLabel.substr(0u, 9u) + "..";
        ImGui::TextUnformatted(ShortLabel.c_str());
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Slot: %s", strSlotId.c_str());
        if (nullptr != pBinding)
        {
            std::string Name = std::filesystem::path(
                pBinding->strAssetId).filename().string();
            if (Name.size() > 9u)
                Name = Name.substr(0u, 7u) + "..";
            ImGui::TextDisabled("%s", Name.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", pBinding->strAssetId.c_str());
        }
		if (bModified)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f), "Modified");
			if (ImGui::SmallButton("Reset to Source"))
				strResetSlot = strSlotId;
		}
		else if (!bMeshAuthoringDraft)
		{
			ImGui::TextDisabled("Source");
		}
        ImGui::EndGroup();
        ImGui::PopID();
    };

    if (bMeshAuthoringDraft)
    {
		const bool_t bRequiresMesh =
			AuthoringFamily_RequiresMesh(m_eSelectedAuthoringFamily);
        struct AUTHORING_SLOT_CARD final
        {
            const char* pSlotId;
            const char* pLabel;
            EFFECT_RESOURCE_FILE_KIND eKind;
        };
        constexpr AUTHORING_SLOT_CARD Slots[] = {
            { "meshModel", "Mesh", EFFECT_RESOURCE_FILE_KIND::MODEL },
            { "base", "Base", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
            { "noise", "Noise", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
            { "mask", "Mask", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
            { "emissive", "Emissive", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
            { "dissolve", "Dissolve", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
            { "base2", "Base 2", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
            { "mask2", "Mask 2", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
            { "noise2", "Noise 2", EFFECT_RESOURCE_FILE_KIND::TEXTURE }
        };
        const float fCardWidth = 78.f;
        const size_t iColumns = static_cast<size_t>((std::max)(1,
            static_cast<int32_t>(
                ImGui::GetContentRegionAvail().x / fCardWidth)));
		const size_t iFirstSlot = bRequiresMesh ? 0u : 1u;
		for (size_t iSlot = iFirstSlot;
            iSlot < sizeof(Slots) / sizeof(Slots[0]); ++iSlot)
        {
			if (0u != (iSlot - iFirstSlot) % iColumns)
                ImGui::SameLine();
            const AUTHORING_SLOT_CARD& Slot = Slots[iSlot];
            RenderSlotCard(Find_Binding(*pElement, Slot.pSlotId),
                Slot.pSlotId, Slot.pLabel, Slot.eKind, false);
        }
        if (m_strSelectedResourceSlotId == "meshModel")
			ImGui::TextDisabled(
				"Mesh: one WModel carrier shape (required for Mesh and Mesh Particle).");
        else if (m_strSelectedResourceSlotId == "base")
            ImGui::TextDisabled("Base: RGB color and A opacity (required).");
        else if (m_strSelectedResourceSlotId == "noise")
            ImGui::TextDisabled("Noise: RG surface distortion; R also modulates dissolve.");
        else if (m_strSelectedResourceSlotId == "mask")
            ImGui::TextDisabled("Mask: R channel multiplies opacity.");
		else if (m_strSelectedResourceSlotId == "emissive")
			ImGui::TextDisabled(
				"Emissive: RGB adds local HDR color using Emissive Intensity. Scene Bloom is configured separately in F1 Rendering Workbench.");
        else if (m_strSelectedResourceSlotId == "dissolve")
            ImGui::TextDisabled("Dissolve: R channel is the lifetime threshold.");
        else if (m_strSelectedResourceSlotId == "base2")
            ImGui::TextDisabled("Base 2: second diffuse layer, multiplied over Base.");
        else if (m_strSelectedResourceSlotId == "mask2")
            ImGui::TextDisabled("Mask 2: second R-channel opacity, multiplied into Mask.");
        else if (m_strSelectedResourceSlotId == "noise2")
            ImGui::TextDisabled("Noise 2: second RG distortion, averaged with Noise.");
        return;
    }

	ImGui::SeparatorText("Declared Resources");
	const float fCardWidth = 78.f;
	const size_t iColumns = static_cast<size_t>((std::max)(1,
		static_cast<int32_t>(ImGui::GetContentRegionAvail().x / fCardWidth)));
	const auto IsModified = [pElement](const std::string_view strSlotId)
	{
		return pElement->AuthoringOverrides.ResourceBindings.end() !=
			std::find_if(
				pElement->AuthoringOverrides.ResourceBindings.begin(),
				pElement->AuthoringOverrides.ResourceBindings.end(),
				[strSlotId](
					const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
				{ return Override.strSlotId == strSlotId; });
	};
	size_t iRendered = 0u;
	/* An editable Element must expose every slot its kind and material
	   template allow, not only the ones that already carry an assetId.
	   Otherwise a freshly created Element that was seeded with nothing has no
	   card to select and can never be bound after creation. */
	const bool_t bEditableElement =
		EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource;
	std::vector<std::string_view> AuthoringSlots;
	if (bEditableElement)
	{
		struct AUTHORING_SLOT_CARD final
		{
			const char* pSlotId;
			const char* pLabel;
			EFFECT_RESOURCE_FILE_KIND eKind;
		};
		constexpr AUTHORING_SLOT_CARD Slots[] = {
			{ "meshModel", "Mesh", EFFECT_RESOURCE_FILE_KIND::MODEL },
			{ "base", "Base", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
			{ "noise", "Noise", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
			{ "mask", "Mask", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
			{ "emissive", "Emissive", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
			{ "dissolve", "Dissolve", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
			{ "base2", "Base 2", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
			{ "mask2", "Mask 2", EFFECT_RESOURCE_FILE_KIND::TEXTURE },
			{ "noise2", "Noise 2", EFFECT_RESOURCE_FILE_KIND::TEXTURE }
		};
		for (const AUTHORING_SLOT_CARD& Slot : Slots)
		{
			if (!Slot_Allowed(*pElement, Slot.pSlotId))
				continue;
			if (0u != iRendered % iColumns)
				ImGui::SameLine();
			RenderSlotCard(Find_Binding(*pElement, Slot.pSlotId),
				Slot.pSlotId, Slot.pLabel, Slot.eKind,
				IsModified(Slot.pSlotId));
			AuthoringSlots.push_back(Slot.pSlotId);
			++iRendered;
		}
	}
	const auto Is_AuthoringSlot = [&AuthoringSlots](
		const std::string_view strSlotId)
	{
		return AuthoringSlots.end() != std::find(
			AuthoringSlots.begin(), AuthoringSlots.end(), strSlotId);
	};
	for (size_t iBinding = 0u;
		iBinding < pElement->ResourceBindings.size(); ++iBinding)
	{
		const EFFECT_RESOURCE_BINDING_DESC& Binding =
			pElement->ResourceBindings[iBinding];
		if (Binding.strAssetId.empty())
			continue;
		if (Is_AuthoringSlot(Binding.strSlotId))
			continue;
		if (0u != iRendered % iColumns)
			ImGui::SameLine();
		RenderSlotCard(&Binding, Binding.strSlotId,
			Slot_Label(*pElement, Binding.strSlotId),
			Resource_FileKind(Binding), IsModified(Binding.strSlotId));
		++iRendered;
	}
	for (const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane :
		pElement->Material.Execution.TextureLanes)
	{
		if (Lane.strLaneId.empty() || Lane.strAssetId.empty())
			continue;
		if (0u != iRendered % iColumns)
			ImGui::SameLine();
		const std::string strSlotId =
			Build_EffectMaterialExecutionLaneStableSlotId(Lane.strLaneId);
		const EFFECT_RESOURCE_BINDING_DESC DisplayBinding{
			strSlotId, Lane.strAssetId };
		RenderSlotCard(&DisplayBinding, strSlotId,
			Lane.strRole.empty() ? Lane.strLaneId : Lane.strRole,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE, IsModified(strSlotId));
		++iRendered;
	}
	for (const EFFECT_NAMED_TEXTURE_DESC& Texture :
		pElement->Material.SourceMaterial.Textures)
	{
		if (Texture.strName.empty() || Texture.strAssetId.empty())
			continue;
		if (0u != iRendered % iColumns)
			ImGui::SameLine();
		const std::string strSlotId =
			Build_EffectSourceMaterialTextureStableSlotId(Texture.strName);
		const EFFECT_RESOURCE_BINDING_DESC DisplayBinding{
			strSlotId, Texture.strAssetId };
		RenderSlotCard(&DisplayBinding, strSlotId, Texture.strName,
			EFFECT_RESOURCE_FILE_KIND::TEXTURE, IsModified(strSlotId));
		++iRendered;
	}
	if (!Is_AuthoringSlot("base") && Is_MissingBaseSourceDecal(*pElement))
	{
		if (0u != iRendered % iColumns)
			ImGui::SameLine();
		RenderSlotCard(nullptr, "base", "Base (Decal)",
			EFFECT_RESOURCE_FILE_KIND::TEXTURE, false);
		++iRendered;
	}
	if (0u == iRendered)
		ImGui::TextDisabled("(no resources bound)");

	ImGui::TextDisabled(bEditableElement ?
		"Empty slots are bindable: pick one card, choose a DDS or WModel in Resource Library, then Bind Selected and Save Changes." :
		"Only compiler-declared DDS/WModel lanes are shown. Add lanes while creating a new Element draft.");
	if (!strResetSlot.empty())
		Try_ResetAuthoringResourceOverride(strResetSlot);
}

void Client::CEffect_Tool::Render_ResourceGrid(
    const bool_t bMeshAuthoringDraft)
{
    Engine::CProfilerScope Profile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.ResourceGrid");
    const EFFECT_ELEMENT_DESC* pElement = bMeshAuthoringDraft ?
        &m_MeshAuthoringDraft : Find_SelectedElement();
	const EFFECT_MATERIAL_TEXTURE_LANE_DESC* pMaterialLane =
		nullptr == pElement ? nullptr : Find_MaterialExecutionLane(
			*pElement, m_strSelectedResourceSlotId);
	const EFFECT_NAMED_TEXTURE_DESC* pSourceTexture =
		nullptr == pElement ? nullptr : Find_SourceMaterialTexture(
			*pElement, m_strSelectedResourceSlotId);
	const bool_t bMaterialLaneSelected =
		nullptr != pMaterialLane || nullptr != pSourceTexture;
	const EFFECT_RESOURCE_BINDING_DESC* pSelectedBinding =
		nullptr == pElement ? nullptr :
			Find_Binding(*pElement, m_strSelectedResourceSlotId);
    const bool_t bSlotSelected = nullptr != pElement &&
		(bMaterialLaneSelected ||
		 (Slot_Allowed(*pElement, m_strSelectedResourceSlotId) &&
		  (!bMeshAuthoringDraft || AuthoringFamily_AllowsSlot(
			  m_eSelectedAuthoringFamily, m_strSelectedResourceSlotId))));
	const bool_t bSelectedSlotModified = nullptr != pElement &&
		pElement->AuthoringOverrides.ResourceBindings.end() != std::find_if(
			pElement->AuthoringOverrides.ResourceBindings.begin(),
			pElement->AuthoringOverrides.ResourceBindings.end(),
			[this](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
			{ return Override.strSlotId == m_strSelectedResourceSlotId; });
	const bool_t bSelectedOptionalAuthoredBinding =
		!bMeshAuthoringDraft && nullptr != pElement &&
		nullptr != pSelectedBinding && !bMaterialLaneSelected &&
		!bSelectedSlotModified &&
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
		Is_OptionalHandAuthoredResourceSlot(
			*pElement, m_strSelectedResourceSlotId);
	const bool_t bSelectedDecalBaseException = nullptr != pElement &&
		Is_BaseTextureSlot(m_strSelectedResourceSlotId) &&
		Is_SourceDecalBaseAdmissionCarrier(*pElement);
	const bool_t bAdapterPacketInspection = !bMeshAuthoringDraft &&
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	EFFECT_RESOURCE_FILE_KIND eWanted = bMaterialLaneSelected ?
		EFFECT_RESOURCE_FILE_KIND::TEXTURE : bSlotSelected ?
		Slot_FileKind(*pElement, m_strSelectedResourceSlotId) :
        m_eResourceLibraryFileKind;
    if (eWanted >= EFFECT_RESOURCE_FILE_KIND::END)
        eWanted = EFFECT_RESOURCE_FILE_KIND::MODEL;
    const std::string Filter = m_ResourceFilter.data();
    std::string BoundAssetId;
	const EFFECT_RESOURCE_FILE_KIND eSlotFileKind = bMaterialLaneSelected ?
		EFFECT_RESOURCE_FILE_KIND::TEXTURE : bSlotSelected ?
		Slot_FileKind(*pElement, m_strSelectedResourceSlotId) :
        EFFECT_RESOURCE_FILE_KIND::END;
    bool_t bCompatibleSlot =
        bSlotSelected && eSlotFileKind == eWanted;
    if (bSlotSelected)
    {
		if (bMaterialLaneSelected)
			BoundAssetId = nullptr != pMaterialLane ?
				pMaterialLane->strAssetId : pSourceTexture->strAssetId;
		else if (nullptr != pSelectedBinding)
			BoundAssetId = pSelectedBinding->strAssetId;
    }

    ImGui::SeparatorText("Resource Library");
    if (ImGui::BeginCombo("Authoring Category##ResourceDomain",
        m_strSelectedAuthoringDomainId.c_str()))
    {
        for (const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain : m_ResourceDomains)
        {
            if (ImGui::Selectable(Domain.strDomainId.c_str(),
                Domain.strDomainId == m_strSelectedAuthoringDomainId))
            {
                Select_AuthoringDomain(Domain.strDomainId);
            }
        }
        ImGui::EndCombo();
    }
    if (!bMeshAuthoringDraft)
    {
        if (ImGui::RadioButton("Meshes",
            EFFECT_RESOURCE_FILE_KIND::MODEL == eWanted))
        {
            eWanted = EFFECT_RESOURCE_FILE_KIND::MODEL;
            m_eResourceLibraryFileKind = eWanted;
            m_strSelectedResourceAssetId.clear();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Textures",
            EFFECT_RESOURCE_FILE_KIND::TEXTURE == eWanted))
        {
            eWanted = EFFECT_RESOURCE_FILE_KIND::TEXTURE;
            m_eResourceLibraryFileKind = eWanted;
            m_strSelectedResourceAssetId.clear();
        }
    }
    if (bMeshAuthoringDraft &&
        EFFECT_RESOURCE_FILE_KIND::MODEL == eWanted)
    {
        constexpr const char* ShapeCategories[] = {
            "All",
            "Ring / Torus / Circle",
            "Slash / Trail / Plane",
            "Crack / Broken",
            "Sphere / Hemisphere",
            "Cylinder / Cone",
            "Helix",
            "Box / Cube / Square",
            "Wave / Aurora / Electric",
            "Other"
        };
        if (ImGui::BeginCombo(
            "Mesh Shape Category", m_strMeshShapeCategory.c_str()))
        {
            for (const char* pCategory : ShapeCategories)
            {
                if (ImGui::Selectable(pCategory,
                    m_strMeshShapeCategory == pCategory))
                {
                    m_strMeshShapeCategory = pCategory;
                    m_iResourceViewRevision = UINT64_MAX;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::TextDisabled(
            "Shape categories are filename search hints; assetId remains authoritative.");
    }
    if (EFFECT_RESOURCE_FILE_KIND::TEXTURE == eWanted)
    {
        constexpr const char* TextureKindCategories[] = {
            "All",
            "Base / Sprite",
            "Noise / Distortion",
            "Normal / Bump",
            "Decal / Ground",
            "Ring / Shockwave",
            "Trail / Beam",
            "Cloud / Smoke / Fire",
            "Fluid / Water",
            "Other"
        };
        if (ImGui::BeginCombo(
            "Texture Kind", m_strTextureKindCategory.c_str()))
        {
            for (const char* pCategory : TextureKindCategories)
            {
                if (ImGui::Selectable(pCategory,
                    m_strTextureKindCategory == pCategory))
                {
                    m_strTextureKindCategory = pCategory;
                    m_iResourceViewRevision = UINT64_MAX;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::TextDisabled(
            "Kind categories are filename search hints; assetId remains authoritative.");
    }
    bCompatibleSlot = bSlotSelected && eSlotFileKind == eWanted;
    const auto DomainIterator = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [this](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
        {
            return Domain.strDomainId == m_strSelectedAuthoringDomainId;
        });
    if (DomainIterator == m_ResourceDomains.end())
    {
        ImGui::TextDisabled(
            "The selected authoring category has no Resources/Effect folder.");
        return;
    }

    const size_t iFileKind = static_cast<size_t>(eWanted);
    if (iFileKind >= DomainIterator->Categories.size())
        return;
    const vector<string>& Categories = DomainIterator->Categories[iFileKind];
    std::string Category = m_ResourceCategory.data();
    if (Category.empty() ||
        std::find(Categories.begin(), Categories.end(), Category) ==
            Categories.end())
    {
        Category = "All";
        Copy_Buffer(m_ResourceCategory.data(),
            m_ResourceCategory.size(), Category);
    }
    if (ImGui::BeginCombo("Resource Folder", Category.c_str()))
    {
        for (const std::string& Candidate : Categories)
        {
            if (ImGui::Selectable(Candidate.c_str(), Candidate == Category))
            {
                Category = Candidate;
                Copy_Buffer(m_ResourceCategory.data(),
                    m_ResourceCategory.size(), Category);
            }
        }
        ImGui::EndCombo();
    }
	if (bSlotSelected)
    {
		const std::string strSelectedSlotLabel = nullptr != pMaterialLane ?
			(pMaterialLane->strRole.empty() ? pMaterialLane->strLaneId :
			 pMaterialLane->strRole) : nullptr != pSourceTexture ?
			pSourceTexture->strName :
			Slot_Label(*pElement, m_strSelectedResourceSlotId);
        ImGui::TextDisabled("Selected slot: %s / %s%s",
			strSelectedSlotLabel.c_str(),
            EFFECT_RESOURCE_FILE_KIND::MODEL == eSlotFileKind ?
                "WModel" : "DDS",
            bCompatibleSlot ? "" : " | switch Library kind to bind");
    }
    ImGui::TextDisabled("%s: %zu candidates",
        m_strSelectedAuthoringDomainId.c_str(),
        DomainIterator->ResourceCounts[iFileKind]);
	ImGui::BeginDisabled(bAdapterPacketInspection || !bCompatibleSlot ||
        m_strSelectedResourceAssetId.empty());
    if (ImGui::Button(bMeshAuthoringDraft ?
        "Use Selected" : "Bind Selected"))
    {
        if (bMeshAuthoringDraft)
            Try_BindMeshAuthoringResource(m_strSelectedResourceAssetId);
        else
            Try_BindResource(m_strSelectedResourceAssetId);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
	ImGui::BeginDisabled(bAdapterPacketInspection || !bSlotSelected ||
		(!bMeshAuthoringDraft && !bSelectedSlotModified &&
			!bSelectedDecalBaseException &&
			!bSelectedOptionalAuthoredBinding));
	if (ImGui::Button(bMeshAuthoringDraft ?
		"Clear Slot" : bSelectedOptionalAuthoredBinding ?
			"Delete Selected Slot" : "Reset Selected to Source"))
    {
        if (bMeshAuthoringDraft)
            Try_ClearMeshAuthoringSlot();
        else
            Try_ClearSelectedSlot();
    }
    ImGui::EndDisabled();
	if (bAdapterPacketInspection)
	{
		ImGui::TextDisabled(
			"Exact adapter resources are inspection-only. Create the generic Authored starting copy before binding or clearing slots.");
	}

    const float CardWidth = 92.f;
    const int32_t Columns = (std::max)(1,
        static_cast<int32_t>(ImGui::GetContentRegionAvail().x / CardWidth));
    Rebuild_ResourceBrowserView(eWanted, Filter,
        m_strSelectedAuthoringDomainId, Category,
        EFFECT_RESOURCE_FILE_KIND::TEXTURE == eWanted ?
            m_strTextureKindCategory :
            (bMeshAuthoringDraft ? m_strMeshShapeCategory : "All"));
    const int32_t Rows = static_cast<int32_t>(
        (m_VisibleResourceIndices.size() + Columns - 1u) / Columns);
    ImGuiListClipper Clipper;
    Clipper.Begin(Rows, 112.f);
    while (Clipper.Step())
    {
        for (int32_t iRow = Clipper.DisplayStart;
            iRow < Clipper.DisplayEnd; ++iRow)
        {
            for (int32_t iColumn = 0; iColumn < Columns; ++iColumn)
            {
                const size_t iEntry = static_cast<size_t>(
                    iRow * Columns + iColumn);
                if (iEntry >= m_VisibleResourceIndices.size())
                    break;
                const EFFECT_RESOURCE_CATALOG_ENTRY& Entry =
                    m_ResourceCatalog[m_VisibleResourceIndices[iEntry]];
                ImGui::PushID(Entry.strAssetId.c_str());
                if (0 != iColumn)
                    ImGui::SameLine();
                ImGui::BeginGroup();
                bool_t bClicked = false;
                const CEffectThumbnailCache::RESULT Thumbnail =
                    m_pThumbnailCache->Request(
                        Entry.strAssetId, Entry.eFileKind);
                if (nullptr != Thumbnail.pTextureView)
                {
                    ImGui::Image(Thumbnail.pTextureView, ImVec2(80.f, 80.f));
                    bClicked = ImGui::IsItemClicked();
                }
                else
                {
                    bClicked = ImGui::Button(
                        EFFECT_RESOURCE_FILE_KIND::MODEL == Entry.eFileKind ?
                            "Mesh" : "DDS", ImVec2(80.f, 80.f));
                    if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
                        ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
                }
                if (Entry.strAssetId == BoundAssetId)
                {
                    ImGui::GetWindowDrawList()->AddRect(
                        ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                        ImGui::GetColorU32(ImGuiCol_HeaderActive),
                        2.f, 0, 3.f);
                }
                else if (Entry.strAssetId == m_strSelectedResourceAssetId)
                {
                    ImGui::GetWindowDrawList()->AddRect(
                        ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                        ImGui::GetColorU32(ImGuiCol_NavHighlight),
                        2.f, 0, 2.f);
                }
                std::string Name = std::filesystem::path(
                    Entry.strAssetId).filename().string();
                if (Name.size() > 13u)
                    Name = Name.substr(0u, 10u) + "...";
                ImGui::TextUnformatted(Name.c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", Entry.strAssetId.c_str());
                ImGui::EndGroup();
                if (bClicked)
                {
                    m_strSelectedResourceAssetId = Entry.strAssetId;
                    if (bMeshAuthoringDraft)
                        Try_BindMeshAuthoringResource(Entry.strAssetId);
                }
                ImGui::PopID();
            }
        }
    }
}

void Client::CEffect_Tool::Rebuild_ResourceBrowserView(
    const EFFECT_RESOURCE_FILE_KIND eFileKind,
    const std::string& strFilter,
    const std::string& strDomainId,
    const std::string& strCategory,
    const std::string& strKindCategory)
{
    if (m_iResourceViewRevision == m_iResourceCatalogRevision &&
        m_eResourceViewFileKind == eFileKind &&
        m_strResourceViewFilter == strFilter &&
        m_strResourceViewDomainId == strDomainId &&
        m_strResourceViewCategory == strCategory &&
        m_strResourceViewKindCategory == strKindCategory)
    {
        return;
    }

    vector<size_t> Staged;
    Staged.reserve(m_ResourceCatalog.size());
    for (size_t iEntry = 0u; iEntry < m_ResourceCatalog.size(); ++iEntry)
    {
        const EFFECT_RESOURCE_CATALOG_ENTRY& Entry =
            m_ResourceCatalog[iEntry];
        if (Entry.eFileKind != eFileKind ||
            Entry.strDomainId != strDomainId ||
            !Contains_NoCase(Entry.strAssetId, strFilter) ||
            (EFFECT_RESOURCE_FILE_KIND::MODEL == eFileKind &&
                !Matches_MeshShapeCategory(
                    Entry.strAssetId, strKindCategory)) ||
            (EFFECT_RESOURCE_FILE_KIND::TEXTURE == eFileKind &&
                !Matches_TextureKindCategory(
                    Entry.strAssetId, strKindCategory)) ||
            (strCategory != "All" && Entry.strCategory != strCategory))
        {
            continue;
        }
        Staged.push_back(iEntry);
    }

    m_VisibleResourceIndices = std::move(Staged);
    m_iResourceViewRevision = m_iResourceCatalogRevision;
    m_eResourceViewFileKind = eFileKind;
    m_strResourceViewFilter = strFilter;
    m_strResourceViewDomainId = strDomainId;
    m_strResourceViewCategory = strCategory;
    m_strResourceViewKindCategory = strKindCategory;
}

void Client::CEffect_Tool::Render_ModelViewWindow()
{
    ImGui::SetNextWindowPos(ImVec2(450.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(650.f, 660.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.f);
    if (!ImGui::Begin("Model View"))
    {
        ImGui::End();
        return;
    }
    /* Boss and monster bodies use the same CModel preview contract as playable
       classes. Keeping them visible here lets an authored boss Effect be
       inspected without introducing a second preview renderer. */
    m_pCharacterPreviewPanel->Render_Selector(false, {}, true);

    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    Render_AnimationControls(pModel);

    const bool_t bValtanTarget =
        CAnimationTargetService::Resolve_AssetName() == VALTAN_ANIMATION_ASSET_NAME;
    if (bValtanTarget)
    {
        ImGui::SeparatorText("Valtan V0 Quick Start");
        ImGui::TextDisabled(
            "Body + armor: MN_RPBF_01.wmodel | Axe: ValtanWeapon.wmodel | Socket: b_wp_r_01");

        if (ImGui::SmallButton("Actor Root##ValtanPivot"))
        {
            m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
            m_strPreviewAnchorSlotId = "root";
            Copy_Buffer(m_PreviewAnchorBuffer.data(),
                m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
            m_strPreviewStatus = "Valtan V0 pivot: actor root.";
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Effect Root##ValtanPivot"))
        {
            constexpr const char_t* EFFECT_ROOT = "b_effectroot";
            float4x4_t Test{};
            if (CAnimationTargetService::Resolve_AnchorTransform(
                    EFFECT_ROOT, &Test))
            {
                m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE;
                m_strPreviewAnchorSlotId = EFFECT_ROOT;
                Copy_Buffer(m_PreviewAnchorBuffer.data(),
                    m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
                m_strPreviewStatus =
                    "Valtan V0 pivot: b_effectroot.";
            }
            else
                m_strPreviewStatus = "Valtan b_effectroot was not found.";
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Axe b_wp_r_01##ValtanPivot"))
        {
            constexpr const char_t* AXE_SOCKET = "b_wp_r_01";
            float4x4_t Test{};
            if (CAnimationTargetService::Resolve_AnchorTransform(
                    AXE_SOCKET, &Test))
            {
                m_ePreviewPivotKind =
                    EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET;
                m_strPreviewAnchorSlotId = AXE_SOCKET;
                Copy_Buffer(m_PreviewAnchorBuffer.data(),
                    m_PreviewAnchorBuffer.size(), m_strPreviewAnchorSlotId);
                m_strPreviewStatus =
                    "Valtan V0 pivot: axe socket b_wp_r_01.";
            }
            else
                m_strPreviewStatus =
                    "Valtan axe socket b_wp_r_01 was not found.";
        }

        if (nullptr != pModel && pModel->Get_NumAnimations() > 0u)
        {
            const char_t* pClipName = pModel->Get_AnimationName(
                pModel->Get_CurrentAnimIndex());
            if (ImGui::SmallButton("Use Current Clip Name##ValtanV0"))
            {
                const std::string ClipName = nullptr == pClipName ?
                    "cue" : pClipName;
                Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
                    Build_ValtanV0EffectAssetId(ClipName));
                Copy_Buffer(m_NewDisplayName.data(),
                    m_NewDisplayName.size(), "Valtan V0 " + ClipName);
                Select_AuthoringDomain("Valtan");
                m_strDocumentStatus =
                    "Prepared a Valtan V0 Effect ID from the selected clip.";
            }
            ImGui::SameLine();
            ImGui::TextDisabled("Clip: %s",
                nullptr == pClipName ? "Invalid" : pClipName);
        }
        ImGui::InputText("V0 Effect ID", m_NewAssetId.data(),
            m_NewAssetId.size());
        const bool_t bCanCreateValtanV0 =
            '\0' != m_NewAssetId[0u] && !Has_UnsavedWork();
        ImGui::BeginDisabled(!bCanCreateValtanV0);
        if (ImGui::Button("Create Valtan V0 Effect"))
        {
            Select_AuthoringDomain("Valtan");
            Try_CreateDocument();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled(
            "Then choose Element Type, DDS/WModel, and Create Element.");
    }

    ImGui::SeparatorText("Effect Pivot");
    if (m_ProductPreview.has_value())
    {
        const ANIMATION_EFFECT_CUE& Cue =
            m_ProductPreview->ProductCue.Cue;
        ImGui::TextWrapped(
            "Product cue placement: %s | %s | %s | %u ms",
            Cue.strAnchorSlotId.c_str(),
            EFFECT_FOLLOW_POLICY::FOLLOW == Cue.eFollowPolicy ?
                "follow" : "snapshot",
			EFFECT_ORIENTATION_POLICY::ANCHOR == Cue.eOrientationPolicy ?
				"anchor orientation" : "action facing",
            Cue.iStartMs);
        ImGui::TextDisabled(
            "Product Play locks pivot and local transform to the admitted animation cue.");
		if (!m_PlayerPreviewCueCandidates.empty() &&
			m_iPlayerPreviewCueCandidateIndex <
				m_PlayerPreviewCueCandidates.size())
		{
			const ANIMATION_EFFECT_PREVIEW_CANDIDATE& Selected =
				m_PlayerPreviewCueCandidates[
					m_iPlayerPreviewCueCandidateIndex];
			const std::string SelectedLabel = "Stage " +
				std::to_string(Selected.iStageIndex + 1u) + " | " +
				Selected.Clip.strClipName;
			optional<size_t> PendingSelection;
			if (ImGui::BeginCombo(
					"Product Cue Stage", SelectedLabel.c_str()))
			{
				for (size_t iCandidate = 0u;
					iCandidate < m_PlayerPreviewCueCandidates.size();
					++iCandidate)
				{
					const ANIMATION_EFFECT_PREVIEW_CANDIDATE& Candidate =
						m_PlayerPreviewCueCandidates[iCandidate];
					const std::string Label = "Stage " +
						std::to_string(Candidate.iStageIndex + 1u) + " | " +
						Candidate.Clip.strClipName;
					if (ImGui::Selectable(Label.c_str(),
						iCandidate == m_iPlayerPreviewCueCandidateIndex))
					{
						PendingSelection = iCandidate;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::TextDisabled(
				"Cue startMs: %u (read-only; authored by Animation events)",
				Selected.Cue.iStartMs);
			if (PendingSelection.has_value())
				Select_PlayerPreviewCueCandidate(*PendingSelection);
		}
    }
	else if (m_ValtanProductPreview.has_value())
	{
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue =
			m_ValtanProductPreview->Cue;
		ImGui::TextWrapped(
			"Valtan Product cue placement: %s | %s | source %u ms",
			Cue.strAnchorSlotId.c_str(), Cue.strFollowPolicy.c_str(),
			Cue.iSourceStartMs);
		ImGui::TextDisabled(
			"Valtan Product Play locks pivot, transform, and source-local timing to this occurrence.");
	}
    if (const CHARACTER_SPEC* pSpec = Resolve_CurrentTargetSpec())
    {
        if (nullptr != pSpec && nullptr != pSpec->pWeapons)
        {
            for (uint32_t iWeapon = 0u;
                iWeapon < pSpec->iNumWeapons; ++iWeapon)
            {
                const char* pSocket = pSpec->pWeapons[iWeapon].pSocketBone;
                if (nullptr == pSocket)
                    continue;
                if (ImGui::Selectable(pSocket,
                    m_strPreviewAnchorSlotId == pSocket))
                {
                    m_strPreviewAnchorSlotId = pSocket;
                    Copy_Buffer(m_PreviewAnchorBuffer.data(),
                        m_PreviewAnchorBuffer.size(),
                        m_strPreviewAnchorSlotId);
                }
            }
        }
    }
    if (ImGui::InputText("Socket / Bone", m_PreviewAnchorBuffer.data(),
        m_PreviewAnchorBuffer.size()))
    {
        m_strPreviewAnchorSlotId = m_PreviewAnchorBuffer.data();
    }
    ImGui::BeginDisabled(Has_ProductCuePreview());
    if (ImGui::Button("Set Effect Pivot Player"))
    {
        m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
        m_strPreviewStatus = "Effect follows the selected Character root.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Effect Pivot Weapon"))
    {
        float4x4_t Test{};
        if (CAnimationTargetService::Resolve_AnchorTransform(
            m_strPreviewAnchorSlotId.c_str(), &Test))
        {
            m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET;
            m_strPreviewStatus = "Effect follows weapon socket: " +
                m_strPreviewAnchorSlotId;
        }
        else
            m_strPreviewStatus = "Selected weapon socket does not exist.";
    }
    if (ImGui::Button("Set Effect Pivot Bone"))
    {
        float4x4_t Test{};
        if (CAnimationTargetService::Resolve_AnchorTransform(
            m_strPreviewAnchorSlotId.c_str(), &Test))
        {
            m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE;
            m_strPreviewStatus = "Effect follows model bone: " +
                m_strPreviewAnchorSlotId;
        }
        else
            m_strPreviewStatus = "Selected model bone does not exist.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Effect Pivot"))
    {
        m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
        m_strPreviewStatus = "Effect uses the fixed world pivot.";
    }
    if (ImGui::Button("Pick World Pivot"))
    {
        m_bPendingWorldPivotPick = true;
        m_strPreviewStatus =
            "Click empty space in Model View to pick the world surface.";
    }
    ImGui::EndDisabled();

	const EFFECT_ELEMENT_DESC* pSelectedTrail = Find_SelectedElement();
	if (nullptr != pSelectedTrail &&
		EFFECT_ELEMENT_KIND::TRAIL == pSelectedTrail->eKind)
	{
		ImGui::SeparatorText("Selected Trail Follow");
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			pSelectedTrail->ActionCueAttachment;
		const bool_t bHasTrailFollow =
			Attachment.bEnabled && Attachment.bFollow;
		if (bHasTrailFollow)
		{
			ImGui::TextWrapped("Element-local follow: %s -> %s",
				Attachment.strRuntimeAnchorSlotId.c_str(),
				Attachment.strRuntimeBoneName.c_str());
		}
		else
		{
			ImGui::TextWrapped(
				"No element-local follow. A stationary root produces only one Trail point and no ribbon draw.");
		}
		ImGui::TextDisabled(
			"This edits only the selected Trail; Product cue placement and the other Elements stay unchanged.");
		const bool_t bTrailAttachmentEditable =
			EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
			EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource;
		const bool_t bCanSetTrailFollow =
			bTrailAttachmentEditable &&
			!Has_UnappliedDetailDraft() &&
			!m_strPreviewAnchorSlotId.empty();
		ImGui::BeginDisabled(!bCanSetTrailFollow);
		if (ImGui::Button("Attach Selected Trail to Bone"))
			Try_SetSelectedTrailFollowAnchor(m_strPreviewAnchorSlotId);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!bTrailAttachmentEditable ||
			Has_UnappliedDetailDraft() ||
			!bHasTrailFollow);
		if (ImGui::Button("Clear Selected Trail Follow"))
			Try_ClearSelectedTrailFollowAnchor();
		ImGui::EndDisabled();
	}

    ImGui::SeparatorText("Animation Cue Transfer");
    InputFloat3("Cue Local Position", m_CueTransferLocalTransform.vPosition);
    InputFloat3("Cue Local Rotation", m_CueTransferLocalTransform.vRotationDegrees);
    InputFloat3("Cue Local Scale", m_CueTransferLocalTransform.vScale);
    if (ImGui::RadioButton("Cue Follow",
        EFFECT_FOLLOW_POLICY::FOLLOW == m_eCueTransferFollowPolicy))
        m_eCueTransferFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
    ImGui::SameLine();
    if (ImGui::RadioButton("Cue Snapshot",
        EFFECT_FOLLOW_POLICY::SNAPSHOT == m_eCueTransferFollowPolicy))
        m_eCueTransferFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
	if (ImGui::RadioButton("Cue Anchor Orientation",
		EFFECT_ORIENTATION_POLICY::ANCHOR ==
			m_eCueTransferOrientationPolicy))
	{
		m_eCueTransferOrientationPolicy =
			EFFECT_ORIENTATION_POLICY::ANCHOR;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Cue Action Facing",
		EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
			m_eCueTransferOrientationPolicy))
	{
		m_eCueTransferOrientationPolicy =
			EFFECT_ORIENTATION_POLICY::ACTION_FACING;
	}
    if (ImGui::RadioButton("Natural Stop",
        EFFECT_STOP_POLICY::NATURAL == m_eCueTransferStopPolicy))
        m_eCueTransferStopPolicy = EFFECT_STOP_POLICY::NATURAL;
    ImGui::SameLine();
    if (ImGui::RadioButton("Cue End Stop",
        EFFECT_STOP_POLICY::CUE_END == m_eCueTransferStopPolicy))
        m_eCueTransferStopPolicy = EFFECT_STOP_POLICY::CUE_END;
    if (EFFECT_STOP_POLICY::CUE_END == m_eCueTransferStopPolicy)
        ImGui::InputScalar("Cue Duration (ms)", ImGuiDataType_U32,
            &m_iCueTransferDurationMs);

    const bool_t bCueScaleValid =
        m_CueTransferLocalTransform.vScale.x > 0.f &&
        m_CueTransferLocalTransform.vScale.y > 0.f &&
        m_CueTransferLocalTransform.vScale.z > 0.f;
	const bool_t bHasAuthoringApproximate =
		m_ActiveDocument.has_value() &&
		HasAuthoringApproximate(*m_ActiveDocument);
    const bool_t bCanTransfer = m_ActiveDocument.has_value() &&
		!bHasAuthoringApproximate &&
		!Has_ProductCuePreview() &&
        !Has_UnsavedWork() &&
        m_bActiveDocumentMatchesRuntime &&
        EFFECT_PREVIEW_PIVOT_KIND::WORLD != m_ePreviewPivotKind &&
		(EFFECT_ORIENTATION_POLICY::ACTION_FACING !=
			m_eCueTransferOrientationPolicy ||
		 EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind) &&
        nullptr != pModel && bCueScaleValid &&
        (EFFECT_STOP_POLICY::NATURAL == m_eCueTransferStopPolicy ||
            m_iCueTransferDurationMs > 0u);
    ImGui::BeginDisabled(!bCanTransfer);
    if (ImGui::Button("Use Selected Effect in Animation Tool"))
    {
        const uint32_t iAnimation = pModel->Get_CurrentAnimIndex();
        const char_t* pClipName = pModel->Get_AnimationName(iAnimation);
        f32_t fPosition = 0.f;
        f32_t fDuration = 0.f;
        const f32_t fTicksPerSecond =
            pModel->Get_AnimationTickPerSecond(iAnimation);
        if (nullptr == pClipName ||
            !pModel->Get_AnimationProgress(iAnimation, fPosition, fDuration) ||
            !std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
        {
            m_strPreviewStatus = "Current animation time cannot be transferred.";
        }
        else
        {
            EFFECT_AUTHORING_CUE_TRANSFER Transfer;
            Transfer.iTargetGeneration =
                CAnimationTargetService::Resolve_TargetGeneration();
            Transfer.strAnimationAssetId =
                CAnimationTargetService::Resolve_AssetName();
            Transfer.strClipName = pClipName;
            Transfer.iTimeMs = static_cast<uint32_t>((std::max)(
                0.f, fPosition / fTicksPerSecond * 1000.f));
            Transfer.iDurationMs =
                EFFECT_STOP_POLICY::CUE_END == m_eCueTransferStopPolicy ?
                m_iCueTransferDurationMs : 0u;
            Transfer.strEffectAssetId =
                m_ActiveDocument->strEffectAssetId;
            Transfer.strAnchorSlotId =
                EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                "root" : m_strPreviewAnchorSlotId;
            Transfer.ePivotKind =
                EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                EFFECT_CUE_PIVOT_KIND::PLAYER_ROOT :
                (EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET == m_ePreviewPivotKind ?
                    EFFECT_CUE_PIVOT_KIND::WEAPON_SOCKET :
                    EFFECT_CUE_PIVOT_KIND::MODEL_BONE);
            Transfer.LocalTransform = m_CueTransferLocalTransform;
            Transfer.eFollowPolicy = m_eCueTransferFollowPolicy;
			Transfer.eOrientationPolicy = m_eCueTransferOrientationPolicy;
            Transfer.eStopPolicy = m_eCueTransferStopPolicy;
            CEffectAuthoringTransfer::Publish(std::move(Transfer));
            m_strPreviewStatus =
                "Queued admitted Effect cue for Animation Tool: " +
                m_ActiveDocument->strEffectAssetId;
        }
    }
    ImGui::EndDisabled();
    if (!bCanTransfer)
    {
        ImGui::TextDisabled("%s",
			bHasAuthoringApproximate ?
				"Animation Cue Transfer refuses a document containing APPROXIMATE authoring carriers; Open/Edit/Save/Audition/Solo/Family preview remain available." :
			Has_ProductCuePreview() ?
                "Product Play consumes the existing admitted cue; use manual Document preview to author a new transfer." :
                "Save the exact runtime-admitted Effect, then choose Player/Weapon/Bone pivot.");
    }

    ImGui::SeparatorText("Effect Preview");
    const auto SelectPreviewFilter = [this](
        const char* pLabel,
        const EFFECT_PREVIEW_FILTER eFilter)
    {
        if (ImGui::RadioButton(pLabel, m_ePreviewFilter == eFilter))
            Try_SetPreviewFilter(eFilter);
    };
    SelectPreviewFilter("Complete Effect", EFFECT_PREVIEW_FILTER::COMPLETE);
    ImGui::SameLine();
    SelectPreviewFilter(
        "All Particles", EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM);
    ImGui::SameLine();
	SelectPreviewFilter(
		"Standalone Mesh", EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES);
	ImGui::SameLine();
	SelectPreviewFilter(
		"Mesh Particles", EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS);
	ImGui::SameLine();
	SelectPreviewFilter(
		"Standalone Sprite", EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_SPRITES);
	ImGui::SameLine();
	SelectPreviewFilter(
		"Sprite Particles", EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS);
	ImGui::SameLine();
    SelectPreviewFilter("Solo Element", EFFECT_PREVIEW_FILTER::SOLO_SELECTED);
    ImGui::SameLine();
    SelectPreviewFilter("Mute Element", EFFECT_PREVIEW_FILTER::MUTE_SELECTED);
    SelectPreviewFilter(
        "Solo Group", EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP);
    ImGui::SameLine();
    SelectPreviewFilter(
        "Mute Group", EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP);
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED == m_ePreviewFilter) &&
		m_strPreviewIsolationElementId.empty())
		ImGui::TextDisabled("Use an Element Solo button to set the preview target.");
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter) &&
		m_strPreviewIsolationGroupId.empty())
    {
        ImGui::TextDisabled(
			"Use a Play Group button to set the group preview target.");
    }
	const bool_t bPreviousScreenPost = m_bPreviewScreenPostEnabled;
	if (ImGui::Checkbox("Screen Post", &m_bPreviewScreenPostEnabled) &&
		m_ActiveDocument.has_value())
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if (m_bParticleSystemDraftDirty)
			Apply_ParticleSystemDraft(Staged);
		if (m_bDetailDraftDirty)
			Apply_DetailDraft(Staged);
		if (m_bModelCueDraftDirty)
			Apply_ModelCueDraft(Staged);
		const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
		Recalculate_PreviewDuration(Build_PreviewDocument(Staged));
		if (!Stage_WorldPreview(Staged))
		{
			m_bPreviewScreenPostEnabled = bPreviousScreenPost;
			m_fPreviewDurationSeconds = fPreviousDuration;
		}
	}
	ImGui::SameLine();
	ImGui::TextDisabled("A/B isolate without editing the Authored document");

    ImGui::SeparatorText("Timeline");
    if (ImGui::Button(m_bPreviewPlaying ? "Pause" : "Play"))
    {
        if (m_bPreviewPlaying)
        {
            m_bPreviewPlaying = false;
            Set_SynchronizedAnimationPaused(true);
		}
		else if (nullptr != m_pWorldPreviewObject.lock())
		{
			if (m_bReconstructedSourceRuntimeActive &&
				m_bReconstructedSourceRuntimeNaturalTailActive)
			{
				m_bPreviewVisibleRequested = true;
				m_bPreviewPlaying = true;
				Set_SynchronizedAnimationPaused(true);
			}
			else if (m_fPreviewTimeSeconds >= m_fPreviewDurationSeconds)
				Start_WorldPreviewFromBeginning();
			else
			{
				m_bPreviewVisibleRequested = true;
				m_bPreviewPlaying = true;
				Set_SynchronizedAnimationPaused(
					m_bReconstructedSourceRuntimeStartPending);
			}
		}
		else if (m_ActiveDocument.has_value() &&
			m_bActiveDocumentDrawable)
		{
			Start_WorldPreviewFromBeginning();
		}
    }
    ImGui::SameLine();
	if (ImGui::Checkbox("Loop", &m_bPreviewLoop))
	{
		if (m_bReconstructedSourceRuntimeActive &&
			m_bReconstructedSourceRuntimeNaturalTailActive &&
			m_bPreviewLoop)
		{
			Start_WorldPreviewFromBeginning();
		}
		else
		{
			Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
			if (m_bReconstructedSourceRuntimeActive)
			{
				m_fReconstructedSourceRuntimeClockSeconds =
					m_fPreviewTimeSeconds;
			}
			Set_SynchronizedAnimationPaused(
				!m_bPreviewPlaying ||
				m_bReconstructedSourceRuntimeStartPending ||
				m_bReconstructedSourceRuntimeNaturalTailActive);
		}
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart + Play"))
        Start_WorldPreviewFromBeginning();
	ImGui::Text("World Preview: %s | %.3f / %.3f s",
		m_bPreviewPlaying ? "PLAYING" : "PAUSED",
		m_fPreviewTimeSeconds, m_fPreviewDurationSeconds);
	if (m_bReconstructedSourceRuntimeNaturalTailActive)
	{
		ImGui::TextDisabled(
			"Natural Stop tail: +%.3f s after animation end.",
			m_fReconstructedSourceRuntimeTailSeconds);
	}
    if (!m_SynchronizedAnimationClips.empty())
    {
        ImGui::TextDisabled(
            "Animation source time owns this Effect timeline; playRate is applied automatically.");
    }
	if (ImGui::SliderFloat("Sample Time", &m_fPreviewTimeSeconds,
		0.f, m_fPreviewDurationSeconds, "%.3f s"))
	{
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = true;
		Reset_ProductCueSnapshot();
		if (m_bReconstructedSourceRuntimeActive)
		{
			Seek_ReconstructedSourceRuntimeTimeline(
				m_fPreviewTimeSeconds);
		}
		else
		{
			Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
			if (const shared_ptr<CEffectObject> pObject =
				m_pWorldPreviewObject.lock())
			{
				if (m_bValtanBossPatternTransformHistoryRequired)
				{
					const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
						[this](const f32_t fSampleTimeSeconds,
							EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
							std::string& strOutError)
						{
							return Build_ValtanBossPatternTransformSample(
								fSampleTimeSeconds, OutSample, strOutError);
						};
					std::string TransformError;
					const bool_t bSampled =
						m_bValtanBossPatternTransformHistoryActive &&
						pObject->Set_SampleTimeWithTransformHistory(
							Resolve_EffectSampleTime(m_fPreviewTimeSeconds),
							TransformProvider, TransformError);
					pObject->Set_Visible(bSampled);
					if (!bSampled)
					{
						m_bValtanBossPatternTransformHistoryActive = false;
						m_strPreviewStatus =
							"Valtan 420633 sample-time anchor history failed: " +
							TransformError;
					}
				}
				else
				{
					const EFFECT_DOCUMENT_DESC& SourceAnchorDocument =
						m_ProductPreview.has_value() &&
						m_SourcePreviewDocument.has_value() ?
							*m_SourcePreviewDocument : *m_ActiveDocument;
					const f32_t fEffectSampleSeconds =
						Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
					std::string HistoryError;
					const bool_t bHistorySampled =
						Seek_WorldPreviewWithSourceAnchorHistory(
							pObject, SourceAnchorDocument,
							fEffectSampleSeconds, HistoryError);
					if (bHistorySampled)
					{
						pObject->Set_Visible(
							Is_ProductCueVisible(m_fPreviewTimeSeconds));
						m_strPreviewStatus =
							"Sample Time rebuilt moving source-anchor history.";
					}
					else
					{
						float4x4_t Root{};
						const bool_t bRootResolved = Resolve_PreviewRoot(Root);
						if (bRootResolved)
							pObject->Set_RootWorld(Root);
						pObject->Set_SampleTime(fEffectSampleSeconds);
						pObject->Set_Visible(bRootResolved &&
							Is_ProductCueVisible(m_fPreviewTimeSeconds));
						m_strPreviewStatus =
							"Sample Time used current-pose fallback: " +
							HistoryError;
					}
				}
			}
			Set_SynchronizedAnimationPaused(true);
		}
	}
	if (m_ActiveDocument.has_value())
	{
		const std::string& SelectedEmitter =
			m_strSelectedEmitterId.empty() ? m_strSelectedElementId :
				m_strSelectedEmitterId;
		ImGui::SeparatorText("Reference A/B Capture");
		ImGui::TextWrapped(
			"Active Effect: %s | Sample Time: %.3f s | Selected Emitter: %s | Screen Post: %s",
			m_ActiveDocument->strEffectAssetId.c_str(),
			m_fPreviewTimeSeconds,
			SelectedEmitter.empty() ? "(complete effect)" :
				SelectedEmitter.c_str(),
			m_bPreviewScreenPostEnabled ? "ON" : "OFF");
		if (ImGui::Button("Copy A/B Metadata"))
		{
			std::ostringstream Metadata;
			Metadata << "effect=" << m_ActiveDocument->strEffectAssetId
				<< " sample=" << m_fPreviewTimeSeconds
				<< " selected_emitter="
				<< (SelectedEmitter.empty() ? "complete" : SelectedEmitter)
				<< " screen_post="
				<< (m_bPreviewScreenPostEnabled ? "on" : "off")
				<< " class=" << Class_Label(m_eAllEffectsClass)
				<< " pivot=" << PreviewPivot_Label(m_ePreviewPivotKind);
			ImGui::SetClipboardText(Metadata.str().c_str());
		}
		ImGui::TextDisabled(
			"Keep the same camera transform, FOV, resolution, class, and pivot for both captures.");
	}

    const ImVec2 Mouse = ImGui::GetMousePos();
    const ImVec2 WindowPosition = ImGui::GetWindowPos();
    m_vMouseViewportPosition = {
        Mouse.x - WindowPosition.x,
        Mouse.y - WindowPosition.y };
    ImGui::Text("Mouse Viewport Position: %.0f, %.0f",
        m_vMouseViewportPosition.x, m_vMouseViewportPosition.y);
    ImGui::Text("Picked World Position: %.3f, %.3f, %.3f",
        m_vPickedWorldPosition.x,
        m_vPickedWorldPosition.y,
        m_vPickedWorldPosition.z);
    if (!m_strPreviewStatus.empty())
        ImGui::TextWrapped("%s", m_strPreviewStatus.c_str());
    if (!m_strPreviewAnimationStatus.empty())
        ImGui::TextWrapped("%s", m_strPreviewAnimationStatus.c_str());
    if (!m_pCharacterPreviewPanel->Get_Status().empty())
        ImGui::TextWrapped("%s",
            m_pCharacterPreviewPanel->Get_Status().c_str());
    Update_Picking();
    ImGui::End();
}

void Client::CEffect_Tool::Render_AnimationControls(
    const shared_ptr<Engine::CModel>& pModel)
{
    ImGui::SeparatorText("Animation");
    if (nullptr == pModel || 0u == pModel->Get_NumAnimations())
    {
        ImGui::TextDisabled("Select a Character model with animations.");
        return;
    }
    Refresh_AnimationClipLabels(pModel, false);
    ImGui::InputTextWithHint("##AnimationClipFilter",
        "Search clip or Valtan action...",
        m_AnimationClipFilter.data(), m_AnimationClipFilter.size());
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear##AnimationClipFilter"))
        m_AnimationClipFilter[0u] = '\0';
    uint32_t iCurrent = pModel->Get_CurrentAnimIndex();
    const char* pCurrentName = pModel->Get_AnimationName(iCurrent);
    const char* pCurrentLabel =
        iCurrent < m_AnimationClipDisplayLabels.size() ?
            m_AnimationClipDisplayLabels[iCurrent].c_str() : pCurrentName;
    if (ImGui::BeginCombo("Animation Clip",
        nullptr != pCurrentLabel ? pCurrentLabel : "Invalid"))
    {
        size_t iVisibleClipCount = 0u;
        for (uint32_t iAnimation = 0u;
            iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
        {
            const char* pName = pModel->Get_AnimationName(iAnimation);
            const char* pLabel =
                iAnimation < m_AnimationClipDisplayLabels.size() ?
                    m_AnimationClipDisplayLabels[iAnimation].c_str() : pName;
            const std::string_view Filter = m_AnimationClipFilter.data();
            const bool_t bMatches = nullptr != pName && nullptr != pLabel &&
                (Contains_NoCase(pLabel, Filter) ||
                 Contains_NoCase(pName, Filter) ||
                 (iAnimation < m_AnimationClipSearchTokens.size() &&
                  Contains_NoCase(
                    m_AnimationClipSearchTokens[iAnimation], Filter)));
            if (!bMatches)
                continue;
            ++iVisibleClipCount;
            if (nullptr != pName && nullptr != pLabel && ImGui::Selectable(
                pLabel, iAnimation == iCurrent))
            {
                Reset_SynchronizedAnimationSequence();
                pModel->Start_Animation(iAnimation, true);
                pModel->Set_AnimPaused(false);
                iCurrent = iAnimation;
            }
        }
        if (0u == iVisibleClipCount)
            ImGui::TextDisabled("No animation matches this search.");
        ImGui::EndCombo();
    }
    if (ImGui::Button("Reload Labels"))
        Refresh_AnimationClipLabels(pModel, true);
    ImGui::SameLine();
    ImGui::TextDisabled(
        CAnimationTargetService::Resolve_AssetName() == VALTAN_ANIMATION_ASSET_NAME ?
            "[Valtan] Pattern Action | Model Clip" :
            "[Input] Korean Skill Name | Model Clip");
    if (!m_strAnimationClipLabelStatus.empty())
        ImGui::TextDisabled("%s", m_strAnimationClipLabelStatus.c_str());
    const bool_t bSkillTimelineOwnsAnimation =
        !m_SynchronizedAnimationClips.empty() &&
        m_iSynchronizedAnimationTargetGeneration ==
            CAnimationTargetService::Resolve_TargetGeneration();
    if (bSkillTimelineOwnsAnimation)
    {
        ImGui::TextDisabled(
            "Bound Effect Timeline owns animation Play/Pause/Restart/Sample Time.");
    }
    ImGui::BeginDisabled(bSkillTimelineOwnsAnimation);
    if (ImGui::Button(pModel->Is_AnimPaused() ?
        "Play Animation" : "Pause Animation"))
    {
        pModel->Set_AnimPaused(!pModel->Is_AnimPaused());
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart Animation"))
        pModel->Set_AnimTrackPosition(iCurrent, 0.f);
    f32_t fPosition = 0.f;
    f32_t fDuration = 0.f;
    if (pModel->Get_AnimationProgress(iCurrent, fPosition, fDuration) &&
        fDuration > 0.f && ImGui::SliderFloat(
            "Animation Frame", &fPosition, 0.f, fDuration, "%.2f"))
    {
        pModel->Set_AnimPaused(true);
        pModel->Set_AnimTrackPosition(iCurrent, fPosition);
    }
    ImGui::EndDisabled();
}

void Client::CEffect_Tool::Update_Picking()
{
    if (!m_bPendingWorldPivotPick ||
        !ImGui::IsWindowHovered() ||
        ImGui::IsAnyItemHovered() ||
        !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;
    float4_t Picked{};
    if (!CGameInstance::Get().Picking(Picked))
    {
        m_strPreviewStatus = "No world surface was hit.";
        return;
    }
    m_vPickedWorldPosition = { Picked.x, Picked.y, Picked.z };
    XMStoreFloat4x4(&m_PreviewWorldRoot, XMMatrixTranslation(
        Picked.x, Picked.y, Picked.z));
    m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
    m_bPendingWorldPivotPick = false;
    m_strPreviewStatus = "World pivot committed from CGameInstance::Picking.";
}

void Client::CEffect_Tool::Render_SelectionPath() const
{
	if (!m_ActiveDocument.has_value())
		return;
	ImGui::SeparatorText("Selection");
	const char* pSelectionLevel = "None";
	switch (m_eDetailSelection)
	{
	case EFFECT_DETAIL_SELECTION::SKILL: pSelectionLevel = "Skill"; break;
	case EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM:
		pSelectionLevel = "Particle System"; break;
	case EFFECT_DETAIL_SELECTION::COMPONENT: pSelectionLevel = "Component"; break;
	case EFFECT_DETAIL_SELECTION::EMITTER: pSelectionLevel = "Emitter"; break;
	case EFFECT_DETAIL_SELECTION::SOURCE_MODULE: pSelectionLevel = "Module"; break;
	case EFFECT_DETAIL_SELECTION::ELEMENT: pSelectionLevel = "Element"; break;
	case EFFECT_DETAIL_SELECTION::MODEL_CUE: pSelectionLevel = "Model / Summon"; break;
	case EFFECT_DETAIL_SELECTION::NONE:
	case EFFECT_DETAIL_SELECTION::END:
	default: break;
	}
	ImGui::Text("Level: %s", pSelectionLevel);
	ImGui::TextWrapped("Skill / Document: %s",
		m_ActiveDocument->strEffectAssetId.c_str());
	if (m_ProductPreview.has_value())
	{
		const PLAYER_SKILL_DEFINITION* pSkill =
			CPlayerSkillCatalog::Find_ById(m_ProductPreview->iSkillId);
		if (nullptr != pSkill &&
			pSkill->eCharacterClass == m_ProductPreview->eCharacterClass)
		{
			const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
				m_ProductPreview->ProductCue;
			ImGui::TextWrapped(
				"Product Skill: #%u | Input %s | Required Stance %s",
				static_cast<uint32_t>(pSkill->iSkillId),
				pSkill->strInputSlot.c_str(),
				Tool_PlayerStanceLabel(pSkill->eRequiredStance));
			ImGui::TextWrapped(
				"Product Cue: Stage %zu / Clip %zu | %s @ %u ms | %s",
				ProductCue.iStageIndex + 1u,
				ProductCue.iStageClipIndex + 1u,
				ProductCue.Cue.strClipName.c_str(),
				ProductCue.Cue.iStartMs,
				ProductCue.Cue.strEffectAssetId.c_str());
			ImGui::TextDisabled(
				"Apply / Save replaces only this Product target at the current "
				"catalog revision. An active occurrence stays immutable; Restart "
				"or the next cast consumes it.");
		}
	}
	if (!m_strSelectedComponentId.empty())
		ImGui::TextWrapped("Component: %s", m_strSelectedComponentId.c_str());
	if (!m_strSelectedEmitterId.empty())
		ImGui::TextWrapped("Emitter: %s", m_strSelectedEmitterId.c_str());
	if (!m_strSelectedSourceModuleId.empty())
		ImGui::TextWrapped("Module: %s", m_strSelectedSourceModuleId.c_str());
	if (EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
		!m_strSelectedElementId.empty())
	{
		if (!m_strSelectedElementGroupId.empty())
			ImGui::TextWrapped("Group: %s",
				m_strSelectedElementGroupId.c_str());
		ImGui::TextWrapped("Element: %s", m_strSelectedElementId.c_str());
	}
	if (EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection &&
		!m_strSelectedModelCueId.empty())
	{
		ImGui::TextWrapped("Model Cue: %s", m_strSelectedModelCueId.c_str());
	}
}

void Client::CEffect_Tool::Render_ModelCueDetail()
{
	const EFFECT_MODEL_CUE_DESC* pCurrent = Find_SelectedModelCue();
	if (nullptr == pCurrent)
	{
		ImGui::TextDisabled("Select one row under Current Effect > Model / Summon.");
		return;
	}
	if (!m_ModelCueDraft.has_value() ||
		m_ModelCueDraft->strCueId != pCurrent->strCueId)
	{
		m_ModelCueDraft = *pCurrent;
		Copy_Buffer(m_ModelCueAssetIdDraft.data(),
			m_ModelCueAssetIdDraft.size(), pCurrent->strModelAssetId);
		Copy_Buffer(m_ModelCueClipNameDraft.data(),
			m_ModelCueClipNameDraft.size(), pCurrent->strClipName);
		m_bModelCueDraftDirty = false;
		m_strDetailStatus.clear();
	}

	EFFECT_MODEL_CUE_DESC& Draft = *m_ModelCueDraft;
	bool_t bChanged = ImGui::Checkbox("Visible", &Draft.bVisible);
	if (ImGui::InputText("WModel", m_ModelCueAssetIdDraft.data(),
		m_ModelCueAssetIdDraft.size()))
	{
		Draft.strModelAssetId = m_ModelCueAssetIdDraft.data();
		bChanged = true;
	}
	if (ImGui::InputText("Animation Clip", m_ModelCueClipNameDraft.data(),
		m_ModelCueClipNameDraft.size()))
	{
		Draft.strClipName = m_ModelCueClipNameDraft.data();
		bChanged = true;
	}
	bChanged |= ImGui::DragFloat("Start Delay (Seconds)",
		&Draft.fStartDelaySeconds, 0.01f, 0.f, 30.f, "%.3f");
	bChanged |= ImGui::DragFloat("Duration (Seconds)",
		&Draft.fDurationSeconds, 0.01f, 0.001f, 30.f, "%.3f");
	bChanged |= ImGui::DragFloat3("Local Position",
		&Draft.LocalTransform.vPosition.x, 0.01f);
	bChanged |= ImGui::DragFloat3("Local Rotation (Degrees)",
		&Draft.LocalTransform.vRotationDegrees.x, 0.25f);
	bChanged |= ImGui::DragFloat3("Local Scale",
		&Draft.LocalTransform.vScale.x, 0.01f, 0.0001f, 100.f);
	if (ImGui::CollapsingHeader("Asset Pre-Transform"))
	{
		bChanged |= ImGui::DragFloat3("Asset Pre-Scale",
			&Draft.vAssetPreScale.x, 0.0001f, 0.000001f, 100.f, "%.6f");
		bChanged |= ImGui::DragFloat3("Asset Pre-Rotation",
			&Draft.vAssetPreRotationDegrees.x, 0.25f);
	}
	if (bChanged)
	{
		m_bModelCueDraftDirty = true;
		m_strDetailStatus =
			"Live Model / Summon preview; Apply commits this draft to memory.";
		Stage_ModelCueDraftPreview();
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!m_bModelCueDraftDirty);
	if (ImGui::Button("Apply Model Cue"))
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if (!Apply_ModelCueDraft(Staged))
		{
			m_strDetailStatus =
				"Apply failed; the selected Model Cue is no longer present.";
		}
		else if (Try_CommitDocument(std::move(Staged)))
		{
			m_bModelCueDraftDirty = false;
			if (const EFFECT_MODEL_CUE_DESC* pCommitted = Find_SelectedModelCue())
				m_ModelCueDraft = *pCommitted;
			m_strDetailStatus =
				"Applied Model / Summon to active Document memory; Save required.";
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert Model Cue"))
	{
		m_ModelCueDraft = *pCurrent;
		Copy_Buffer(m_ModelCueAssetIdDraft.data(),
			m_ModelCueAssetIdDraft.size(), pCurrent->strModelAssetId);
		Copy_Buffer(m_ModelCueClipNameDraft.data(),
			m_ModelCueClipNameDraft.size(), pCurrent->strClipName);
		m_bModelCueDraftDirty = false;
		Stage_WorldPreview(*m_ActiveDocument);
		m_strDetailStatus = "Reverted the Model Cue draft.";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Audition Summon"))
		Try_SoloModelCue(m_ActiveDocument->strEffectAssetId, Draft.strCueId);
	if (!m_strDetailStatus.empty())
		ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
}

void Client::CEffect_Tool::Render_SkillSelectionDetail()
{
	if (!m_ActiveDocument.has_value())
		return;
	const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
		CEffectCatalog::Find_Assembly(m_ActiveDocument->strEffectAssetId);
	const PARTICLE_LAYER_SUMMARY Summary =
		Summarize_ParticleLayers(*m_ActiveDocument);
	size_t iEmitterCount = 0u;
	size_t iModuleCount = 0u;
	size_t iResourceCount = 0u;
	if (nullptr != Assembly)
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
		{
			const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
				CEffectCatalog::Find_Component(Cue.strComponentAssetId);
			if (nullptr == Component)
				continue;
			iEmitterCount += Component->Emitters.size();
			for (const EFFECT_ELEMENT_DESC& Element : Component->Document.Elements)
			{
				iModuleCount += Element.SourceRecipe.Modules.size();
				iResourceCount += Element.ResourceBindings.size();
			}
		}
	}
	ImGui::Text("Skill Effect: %s",
		m_ActiveDocument->strDisplayName.c_str());
	ImGui::TextDisabled("Stable ID: %s",
		m_ActiveDocument->strEffectAssetId.c_str());
	ImGui::TextDisabled(
		"Timeline Cues %zu | Components %zu | Emitters %zu | Elements %zu",
		nullptr == Assembly ? 0u : Assembly->ComponentCues.size(),
		nullptr == Assembly ? 0u : Assembly->ComponentCues.size(),
		iEmitterCount, m_ActiveDocument->Elements.size());
	ImGui::TextDisabled(
		"Particle Source Systems %zu | Layers %zu | Resources %zu | Modules %zu",
		Summary.iSourceSystemCount, Summary.iLayerCount,
		iResourceCount, iModuleCount);
	ImGui::TextWrapped(
		"This is the complete Skill Effect. Select Particle System for aggregate "
		"multipliers, a Component for one source cue, an Emitter for renderer and "
		"particle controls, or a Module for the original Cascade values.");
	if (Summary.iLayerCount > 0u &&
		ImGui::Button("Select Particle System"))
	{
		Try_SelectParticleSystem(m_ActiveDocument->strEffectAssetId);
		return;
	}
	if (Summary.iLayerCount > 0u)
		ImGui::SameLine();
	ImGui::BeginDisabled(0u == iEmitterCount);
	if (ImGui::Button("Open First Emitter"))
		Try_SelectFirstEmitter(m_ActiveDocument->strEffectAssetId, {});
	ImGui::EndDisabled();
}

void Client::CEffect_Tool::Render_ComponentSelectionDetail()
{
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(m_strSelectedComponentId);
	if (nullptr == Component)
	{
		ImGui::TextDisabled("Selected Component is no longer admitted.");
		return;
	}
	size_t iResourceCount = 0u;
	size_t iModuleCount = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Component->Document.Elements)
	{
		iResourceCount += Element.ResourceBindings.size();
		iModuleCount += Element.SourceRecipe.Modules.size();
	}
	ImGui::Text("Component: %s", Component->strDisplayName.c_str());
	ImGui::TextDisabled("Stable ID: %s",
		Component->strComponentAssetId.c_str());
	ImGui::TextDisabled("Type: %s | Emitters %zu | Resources %zu | Modules %zu",
		Component->strComponentType.c_str(), Component->Emitters.size(),
		iResourceCount, iModuleCount);
	ImGui::TextDisabled("Source Group: %s",
		Component->strSourceGroupId.c_str());
	if (ImGui::TreeNode("Source Nodes"))
	{
		for (const std::string& Node : Component->SourceNodes)
			ImGui::BulletText("%s", Node.c_str());
		ImGui::TreePop();
	}
	ImGui::TextWrapped(
		"A Component owns the original Emitters in source order. "
		"Select an Emitter to edit its Renderer, Resource Set, and Module Stack.");
	ImGui::BeginDisabled(Component->Emitters.empty());
	if (ImGui::Button("Open First Emitter"))
	{
		Try_SelectFirstEmitter(m_ActiveDocument->strEffectAssetId,
			Component->strComponentAssetId);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Audition Component"))
	{
		EFFECT_DOCUMENT_DESC Preview = Component->Document;
		f32_t fCueOffset = 0.f;
		if (m_ActiveDocument.has_value())
		{
			const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
				CEffectCatalog::Find_Assembly(
					m_ActiveDocument->strEffectAssetId);
			if (nullptr != Assembly)
			{
				const auto Cue = std::find_if(Assembly->ComponentCues.begin(),
					Assembly->ComponentCues.end(),
					[this](const EFFECT_COMPONENT_CUE_DESC& Value)
					{
						return Value.strComponentAssetId ==
							m_strSelectedComponentId;
					});
				if (Assembly->ComponentCues.end() != Cue)
					fCueOffset = Cue->fStartDelaySeconds;
			}
		}
		for (EFFECT_ELEMENT_DESC& Element : Preview.Elements)
			Element.Detail.Timing.fStartDelaySeconds += fCueOffset;
		Recalculate_PreviewDuration(Preview);
		if (Stage_WorldPreview(Preview))
		{
			m_fPreviewTimeSeconds = fCueOffset;
			m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
			m_strPreviewStatus = "Selected Component audition staged at its Assembly cue.";
		}
	}
}

void Client::CEffect_Tool::Render_EmitterSelectionDetail()
{
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(m_strSelectedComponentId);
	if (nullptr == Component)
		return;
	const auto Emitter = std::find_if(Component->Emitters.begin(),
		Component->Emitters.end(), [this](const EFFECT_COMPONENT_EMITTER_DESC& Value)
		{
			return Value.strEmitterId == m_strSelectedEmitterId;
		});
	if (Component->Emitters.end() == Emitter)
		return;
	ImGui::Text("Emitter: %s", Emitter->strEmitterId.c_str());
	ImGui::TextDisabled("Component: %s | Renderer: %s",
		Component->strComponentAssetId.c_str(),
		Emitter->strRendererType.c_str());
	ImGui::TextDisabled("Source order %u | Resources %u | Modules %u",
		Emitter->iSourceElementIndex, Emitter->iResourceBindingCount,
		Emitter->iModuleCount);
	if (const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement())
	{
		ImGui::TextWrapped("Source Material: %s",
			pElement->Material.strSourceMaterialPath.empty() ? "(none)" :
				pElement->Material.strSourceMaterialPath.c_str());
		ImGui::TextDisabled("Template %s | Render %s",
			pElement->Material.strTemplateId.c_str(),
			Profile_Label(pElement->Material.eRenderProfile));
	}
	ImGui::TextWrapped(
		"Emitter controls below edit the selected renderer instance. Select one "
		"Module in All Effects to focus its original source-class values.");
	ImGui::SeparatorText("Typed Emitter Detail");
}

void Client::CEffect_Tool::Render_SourceModuleSelectionDetail()
{
	const EFFECT_ELEMENT_DESC* pCurrent = Find_SelectedElement();
	if (nullptr == pCurrent)
	{
		ImGui::TextDisabled("Selected Module has no active Emitter Element.");
		return;
	}
	if (!m_DetailDraft.has_value() ||
		m_strDetailDraftElementId != pCurrent->strElementId)
	{
		m_DetailDraft = *pCurrent;
		m_strDetailDraftElementId = pCurrent->strElementId;
		Refresh_DetailDraftAdmission(*pCurrent);
		m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
	}
	const auto Module = std::find_if(
		m_DetailDraft->SourceRecipe.Modules.begin(),
		m_DetailDraft->SourceRecipe.Modules.end(),
		[this](const EFFECT_SOURCE_MODULE_DESC& Value)
		{
			return Value.strStableId == m_strSelectedSourceModuleId;
		});
	if (m_DetailDraft->SourceRecipe.Modules.end() == Module)
	{
		ImGui::TextDisabled("Selected source Module is no longer present.");
		return;
	}
	ImGui::Text("Module: %s", Module->strClassName.c_str());
	ImGui::TextDisabled("Stable ID: %s", Module->strStableId.c_str());
	ImGui::TextDisabled("Emitter: %s", m_strSelectedEmitterId.c_str());
	if (m_bDetailDraftPortableRecipeReadOnly)
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"Compiler-owned portable SourceRecipe. Module values are read-only in the Tool; refresh them through the source compiler/reimport path.");
	}
	bool_t bChanged = false;
	ImGui::BeginDisabled(m_bDetailDraftPortableRecipeReadOnly);
	Render_SourceModuleDetail(*Module, bChanged, true);
	ImGui::EndDisabled();
	if (bChanged)
	{
		m_bDetailDraftDirty = true;
		m_strDetailStatus =
			"Live source Module preview; Apply commits it to active Document memory.";
		m_bDetailDraftPreviewPending = true;
		m_fDetailDraftPreviewDueSeconds = ImGui::GetTime() + 0.060;
	}
	if (m_bDetailDraftPreviewPending &&
		(!ImGui::IsAnyItemActive() ||
		 ImGui::GetTime() >= m_fDetailDraftPreviewDueSeconds))
	{
		m_bDetailDraftPreviewPending = false;
		Stage_DetailDraftPreview();
	}
	ImGui::Separator();
	ImGui::BeginDisabled(
		m_bDetailDraftPortableRecipeReadOnly || !m_bDetailDraftDirty);
	if (ImGui::Button("Apply Module"))
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if (Apply_DetailDraft(Staged) && Try_CommitDocument(std::move(Staged)))
		{
			m_bDetailDraftDirty = false;
			m_bDetailDraftPreviewPending = false;
			if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
			{
				m_DetailDraft = *pCommitted;
				Refresh_DetailDraftAdmission(*pCommitted);
			}
			m_strDetailStatus =
				"Applied source Module to active Document memory; Save required.";
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert Module"))
	{
		m_DetailDraft = *pCurrent;
		Refresh_DetailDraftAdmission(*pCurrent);
		m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
		Stage_WorldPreview();
		m_strDetailStatus = "Reverted source Module draft.";
	}
	ImGui::EndDisabled();
	if (!m_strDetailStatus.empty())
		ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
}

void Client::CEffect_Tool::Render_AuthoringSessionBar()
{
	if (!m_ActiveDocument.has_value())
		return;
	const bool_t bEditableSource =
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource;
	const bool_t bMigrationReference =
		EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE ==
			m_eActiveDocumentSource;
	const bool_t bVisualProgramCopy =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource;
	const bool_t bAdapterPacketVisualCopy = bVisualProgramCopy &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	const bool_t bDrawable = m_bActiveDocumentDrawable;
	const bool_t bLivePreview = bDrawable &&
		m_bPreviewVisibleRequested &&
		nullptr != m_pWorldPreviewObject.lock();
	ImGui::SeparatorText("Editing Session");
	ImGui::Text("Source: %s | Draft: %s | Document: %s | Preview: %s | Runtime: %s",
		Source_Label(m_eActiveDocumentSource),
		Has_UnappliedDetailDraft() ? "UNAPPLIED" : "committed",
		m_bDocumentDirty ? "UNSAVED" : "saved",
		bLivePreview ? "LIVE" : "HIDDEN",
		Runtime_SyncLabel());
	const bool_t bCleanHotReloadRetry = !Has_UnsavedWork() &&
		!m_bActiveDocumentMatchesRuntime &&
		Can_HotReloadSavedProduct();
	ImGui::BeginDisabled(!bEditableSource ||
		(!Has_UnsavedWork() && !bCleanHotReloadRetry));
	if (ImGui::Button(bCleanHotReloadRetry ?
		"Retry Product Hot Reload" : "Save Changes"))
		Try_ApplyDraftAndSave();
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool_t bCanReloadSaved =
		(EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ||
		 bMigrationReference) &&
		!m_ActiveDocumentPath.empty();
	ImGui::BeginDisabled(!bCanReloadSaved);
	if (ImGui::Button(Has_UnsavedWork() ?
		"Reload Saved..." : "Reload Saved"))
	{
		if (Has_UnsavedWork())
		{
			Try_LoadDocumentPath(m_ActiveDocumentPath,
				m_eActiveDocumentSource,
				m_ActiveDocument->strEffectAssetId);
		}
		else
			Try_ReloadActiveDocument();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Restart Preview"))
		Start_WorldPreviewFromBeginning();
	if (!Has_UnsavedWork() && !m_strSaveHotReloadStatus.empty())
		ImGui::TextWrapped("Save Hot Reload: %s",
			m_strSaveHotReloadStatus.c_str());
	if (!bEditableSource)
	{
		if (bMigrationReference)
		{
			ImGui::TextWrapped(
				"This Legacy/Rollback document is an immutable migration reference. In-memory inspection and preview are allowed, but Save Changes cannot overwrite the original file.");
			ImGui::InputText("Migrated Authored Copy ID", m_NewAssetId.data(),
				m_NewAssetId.size());
			ImGui::BeginDisabled(Has_UnappliedDetailDraft());
			if (ImGui::Button("Save As New Authored Effect"))
				Try_SaveDocumentAs(m_NewAssetId.data());
			ImGui::EndDisabled();
		}
		else if (bVisualProgramCopy)
		{
			if (bAdapterPacketVisualCopy)
			{
				ImGui::TextWrapped(
					"This exact adapter projection is immutable. Full Details are available for inspection and temporary preview only; ordinary Save As would discard its projector/VF/resource packet and is blocked.");
				ImGui::TextDisabled(
					"Persistent position / rotation / scale: use Stable occurrences and the PROJECT_TUNED Transform Save / Reload controls.");
				ImGui::BeginDisabled(true);
				ImGui::Button("Direct Save As (adapter packet unsupported)");
				ImGui::EndDisabled();
				ImGui::InputText("Generic Starting Copy ID", m_NewAssetId.data(),
					m_NewAssetId.size());
				ImGui::BeginDisabled(Has_UnappliedDetailDraft());
				if (ImGui::Button(
					"Save Selected As Generic Authored Starting Copy"))
				{
					Try_SaveSelectedAdapterElementAsGenericAuthoredCopy(
						m_NewAssetId.data());
				}
				ImGui::EndDisabled();
				ImGui::TextDisabled(
					"The generic copy keeps this Element's Detail, Material, and resource bindings, but intentionally does not keep the exact adapter packet.");
			}
			else
			{
				ImGui::TextWrapped(
					"The extracted Visual Program is immutable. This in-memory copy exposes full Element Details and Resource Library editing; persist it only as a new Authored Effect.");
				ImGui::InputText("Authored Copy ID", m_NewAssetId.data(),
					m_NewAssetId.size());
				ImGui::BeginDisabled(Has_UnappliedDetailDraft());
				if (ImGui::Button("Save As Authored Copy"))
					Try_SaveDocumentAs(m_NewAssetId.data());
				ImGui::EndDisabled();
			}
		}
		else
		{
			ImGui::TextDisabled(
				"Imported/runtime views use Promote or Save As before authoring save.");
		}
	}
	else if (!bDrawable)
	{
		ImGui::TextWrapped(
			"Structurally valid partial draft. Preview hidden and Product hot reload blocked: %s",
			m_strActiveDocumentDrawableError.c_str());
	}
	else if (m_bActiveDocumentMatchesRuntime)
	{
		ImGui::TextDisabled(
			"Saved Authored matches the Runtime Catalog snapshot loaded in this process.");
	}
	else if (Can_HotReloadSavedProduct())
	{
		ImGui::TextDisabled(
			"Saved Authored differs from Product. Use Retry Product Hot Reload; a failed retry preserves the existing prepared Product target.");
	}
	else
	{
		ImGui::TextDisabled(
			"World preview uses active Authored; no exact direct-authored Product cue is selected for hot reload.");
	}
	ImGui::Separator();
}

void Client::CEffect_Tool::Render_EffectDetailWindow()
{
    ImGui::SetNextWindowPos(ImVec2(1110.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 660.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Effect Detail"))
    {
        ImGui::End();
        return;
    }
	if (EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
	{
		Render_RuntimeOccurrenceDetail();
		ImGui::End();
		return;
	}
    if (!m_ActiveDocument.has_value())
    {
        Reset_ParticleSystemDraft();
        Reset_DetailDraft();
		Reset_ModelCueDraft();
        m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
        ImGui::TextDisabled(
			"Load an authored Effect, then select one Element under Current Effect.");
        ImGui::End();
        return;
    }
	Render_AuthoringSessionBar();
	Render_SelectionPath();
	if (EFFECT_DETAIL_SELECTION::SKILL == m_eDetailSelection)
	{
		Render_SkillSelectionDetail();
		ImGui::End();
		return;
	}
    if (EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM == m_eDetailSelection)
    {
        Render_ParticleSystemDetail();
        ImGui::End();
        return;
    }
	if (EFFECT_DETAIL_SELECTION::COMPONENT == m_eDetailSelection)
	{
		Render_ComponentSelectionDetail();
		ImGui::End();
		return;
	}
	if (EFFECT_DETAIL_SELECTION::SOURCE_MODULE == m_eDetailSelection)
	{
		Render_SourceModuleSelectionDetail();
		ImGui::End();
		return;
	}
	if (EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection)
	{
		Render_ModelCueDetail();
		ImGui::End();
		return;
	}
    const EFFECT_ELEMENT_DESC* pCurrent = Find_SelectedElement();
	const bool_t bElementOrEmitter =
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ||
		EFFECT_DETAIL_SELECTION::EMITTER == m_eDetailSelection;
    if (!bElementOrEmitter ||
        nullptr == pCurrent)
    {
        ImGui::TextDisabled(
			"Select one Element under Effect Tool > Current Effect.");
		ImGui::End();
		return;
	}
	const bool_t bAdapterPacketInspection =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (EFFECT_DETAIL_SELECTION::EMITTER == m_eDetailSelection)
		Render_EmitterSelectionDetail();
    if (!m_DetailDraft.has_value() ||
        m_strDetailDraftElementId != pCurrent->strElementId)
    {
        m_DetailDraft = *pCurrent;
        m_strDetailDraftElementId = pCurrent->strElementId;
		Refresh_DetailDraftAdmission(*pCurrent);
        m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
        m_strDetailStatus.clear();
    }
    const f32_t fElementStart =
        m_DetailDraft->Detail.Timing.fStartDelaySeconds;
    ImGui::TextDisabled("Visible timeline: %.3f - %.3f s",
        fElementStart, Element_PreviewEndSeconds(*m_DetailDraft));
    bool_t bChanged = false;
	ImGui::TextDisabled(bAdapterPacketInspection ?
		"Exact adapter packet inspection. Persistent transforms use Stable occurrence Save / Reload; create a generic Authored starting copy before material/resource editing." :
		"Drag numeric values for live preview; Apply updates Current Effect memory, and Save Changes writes the whole Effect.");
	ImGui::BeginDisabled(bAdapterPacketInspection);
	const bool_t bPresentationElement =
		m_DetailDraft->eKind == EFFECT_ELEMENT_KIND::LIGHT ||
		m_DetailDraft->eKind == EFFECT_ELEMENT_KIND::SCREEN_POST;
	if (bPresentationElement)
	{
		ImGui::TextDisabled(
			"Presentation carriers have no WModel/DDS material slots. Their typed payload is edited below.");
	}
	else
	{
		Render_ResourceSlots(false);
	}
	Render_Detail(*m_DetailDraft, bChanged);
    if (bChanged)
    {
        m_bDetailDraftDirty = true;
        m_strDetailStatus =
            "Live preview only; Apply Detail commits this draft to memory.";
		m_bDetailDraftPreviewPending = true;
		m_fDetailDraftPreviewDueSeconds = ImGui::GetTime() + 0.060;
    }
	if (m_bDetailDraftPreviewPending &&
		(!ImGui::IsAnyItemActive() ||
		 ImGui::GetTime() >= m_fDetailDraftPreviewDueSeconds))
	{
		m_bDetailDraftPreviewPending = false;
		Stage_DetailDraftPreview();
	}
    ImGui::Separator();
    ImGui::BeginDisabled(!m_bDetailDraftDirty);
	if (ImGui::Button("Apply to Current Effect (Unsaved)"))
    {
        EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
        if (!Apply_DetailDraft(Staged))
        {
            m_strDetailStatus =
                "Apply failed; the selected Element is no longer present.";
        }
        else if (Try_CommitDocument(std::move(Staged)))
        {
            m_bDetailDraftDirty = false;
			m_bDetailDraftPreviewPending = false;
			if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
			{
				m_DetailDraft = *pCommitted;
				Refresh_DetailDraftAdmission(*pCommitted);
			}
            m_strDetailStatus =
                "Applied to active Document memory; Save required to persist.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert Detail"))
    {
        m_DetailDraft = *pCurrent;
		Refresh_DetailDraftAdmission(*pCurrent);
        m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
		Recalculate_PreviewDuration(*m_ActiveDocument);
		if (Stage_WorldPreview(*m_ActiveDocument))
            m_strPreviewStatus =
                "Detail draft reverted to the active Document preview.";
        m_strDetailStatus =
            "Reverted the Detail draft to the active Document.";
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
	const bool_t bDetailExecutionTarget =
		Is_EffectAuthoringExecutionTarget(
			m_DetailDraft->Material.Execution);
	const bool_t bDetailPreviewLocked =
		m_bDetailDraftCapabilityDeferred || !bDetailExecutionTarget ||
		!m_DetailDraft->bVisible;
	ImGui::BeginDisabled(bDetailPreviewLocked);
    if (ImGui::Button("Audition Selected"))
        Try_AuditionSelectedElement();
	ImGui::EndDisabled();
	if (bDetailPreviewLocked && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		if (m_bDetailDraftCapabilityDeferred)
		{
			ImGui::SetTooltip(
				"Play is locked because this SourceRecipe is outside the supported authoring-preview capability. Editing and Save remain available.");
		}
		else if (!bDetailExecutionTarget)
		{
			ImGui::SetTooltip(
				"Play is locked by hard material/runtime fail-closed admission. Editing and Save remain available.");
		}
		else
		{
			ImGui::SetTooltip(
				"Enable Visible to audition this Element. Runtime availability remains fail-closed; editing and Save remain available.");
		}
	}
    if (m_bDetailDraftDirty)
        ImGui::TextDisabled("Detail draft is local until Apply Detail.");
	if (!m_strDetailStatus.empty())
		ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
	ImGui::EndDisabled();
	ImGui::End();
}

void Client::CEffect_Tool::Render_SelectedVisualProgramEvidence() const
{
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM !=
			m_eActiveDocumentSource ||
		nullptr == m_pSelectedVisualSourceProjection ||
		m_strSelectedRuntimeOccurrenceEffectId.empty() ||
		m_strSelectedRuntimeOccurrenceId.empty() ||
		m_strSelectedRuntimeOccurrenceElementId.empty())
	{
		return;
	}

	const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& Projection =
		*m_pSelectedVisualSourceProjection;
	const EFFECT_VISUAL_PROGRAM_ROW* pVisualRow =
		Projection.Find_RowByOccurrenceId(
			m_strSelectedRuntimeOccurrenceId);
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pSupplemental =
		Projection.Find_SupplementalElementByOccurrenceId(
			m_strSelectedRuntimeOccurrenceId);
	const bool_t bSelectedTargetMatches =
		m_strSelectedElementId == m_strSelectedRuntimeOccurrenceElementId;
	const bool_t bVisualMatches = bSelectedTargetMatches &&
		nullptr != pVisualRow &&
		Projection.Get_EffectAssetId() ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pVisualRow->Selector.strEffectAssetId ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pVisualRow->Selector.strOccurrenceId ==
			m_strSelectedRuntimeOccurrenceId &&
		pVisualRow->strRowSha256 ==
			m_strSelectedRuntimeOccurrenceRowSha256 &&
		pVisualRow->TargetIdentity.has_value() &&
		pVisualRow->TargetIdentity->strTargetElementId ==
			m_strSelectedRuntimeOccurrenceElementId &&
		pVisualRow->SourceIdentity.strSourceRecordId ==
			m_strSelectedRuntimeOccurrenceEmitterPath;
	const bool_t bSupplementalMatches = bSelectedTargetMatches &&
		nullptr != pSupplemental &&
		Projection.Get_EffectAssetId() ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pSupplemental->Selector.strEffectAssetId ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pSupplemental->Selector.strOccurrenceId ==
			m_strSelectedRuntimeOccurrenceId &&
		pSupplemental->strRowSha256 ==
			m_strSelectedRuntimeOccurrenceRowSha256 &&
		pSupplemental->TargetIdentity.strTargetElementId ==
			m_strSelectedRuntimeOccurrenceElementId &&
		pSupplemental->strSourceRecordId ==
			m_strSelectedRuntimeOccurrenceEmitterPath;

	ImGui::SeparatorText("Read-only Exact Source Evidence");
	ImGui::TextDisabled(
		"Immutable Visual Program packet data. Standard authored Mesh/Base/Noise/Mask/Emissive/Dissolve slots are edited separately below.");
	if (bVisualMatches == bSupplementalMatches)
	{
		ImGui::TextWrapped(
			"Evidence unavailable: the selected occurrence/target stable identity no longer resolves to exactly one admitted packet row.");
		return;
	}

	using EVIDENCE_FIELDS =
		std::vector<std::pair<std::string, std::string>>;
	const auto AddText = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const std::string& strValue)
	{
		if (!strValue.empty())
			Fields.emplace_back(std::string(strLabel), strValue);
	};
	const auto AddUnsigned = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const uint64_t iValue)
	{
		if (0u != iValue)
			Fields.emplace_back(
				std::string(strLabel), std::to_string(iValue));
	};
	const auto AddDouble = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const double fValue)
	{
		if (0.0 == fValue)
			return;
		std::ostringstream Text;
		Text << fValue;
		Fields.emplace_back(std::string(strLabel), Text.str());
	};
	const auto AddBool = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const bool_t bValue)
	{
		Fields.emplace_back(
			std::string(strLabel), bValue ? "true" : "false");
	};
	const auto RenderFields = [](const char_t* pTableId,
		const EVIDENCE_FIELDS& Fields)
	{
		if (Fields.empty() || !ImGui::BeginTable(pTableId, 2,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp))
		{
			return;
		}
		ImGui::TableSetupColumn("Typed field",
			ImGuiTableColumnFlags_WidthFixed, 124.f);
		ImGui::TableSetupColumn("Exact value",
			ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();
		for (const auto& [strLabel, strValue] : Fields)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(strLabel.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("%s", strValue.c_str());
		}
		ImGui::EndTable();
	};
	const auto RenderResources = [&](const char_t* pSectionId,
		const std::vector<EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>& Resources)
	{
		if (Resources.empty())
			return;
		const std::string Header = std::string(pSectionId) + " (" +
			std::to_string(Resources.size()) + ")";
		if (!ImGui::TreeNodeEx(Header.c_str(),
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}
		for (size_t iResource = 0u; iResource < Resources.size(); ++iResource)
		{
			const EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW& Resource =
				Resources[iResource];
			EVIDENCE_FIELDS Fields;
			AddText(Fields, "Role", Resource.strRole);
			AddText(Fields, "Slot ID", Resource.strSlotId);
			AddText(Fields, "Asset ID", Resource.strAssetId);
			AddText(Fields, "Resolution", Resource.strResolutionStatus);
			AddText(Fields, "Raw SHA-256", Resource.strRawSha256);
			AddUnsigned(Fields, "Byte count", Resource.iByteCount);
			AddText(Fields, "Shader register", Resource.strShaderRegister);
			AddText(Fields, "Source channel", Resource.strSourceChannel);
			if (Fields.empty())
				continue;
			ImGui::PushID(static_cast<int>(iResource));
			const std::string Label = "Resource " +
				std::to_string(iResource + 1u) +
				(Resource.strRole.empty() ? std::string{} :
					": " + Resource.strRole);
			if (ImGui::TreeNodeEx(Label.c_str(),
				ImGuiTreeNodeFlags_DefaultOpen))
			{
				RenderFields("resource_evidence", Fields);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	};
	const auto RenderLimitations = [&](
		const std::vector<std::string>& Limitations)
	{
		for (const std::string& strLimitation : Limitations)
		{
			if (!strLimitation.empty())
				ImGui::BulletText("%s", strLimitation.c_str());
		}
	};

	EVIDENCE_FIELDS Identity;
	AddText(Identity, "Effect asset ID",
		m_strSelectedRuntimeOccurrenceEffectId);
	AddText(Identity, "Occurrence ID",
		m_strSelectedRuntimeOccurrenceId);
	AddText(Identity, "Target element ID",
		m_strSelectedRuntimeOccurrenceElementId);
	AddText(Identity, "Row SHA-256",
		m_strSelectedRuntimeOccurrenceRowSha256);
	AddText(Identity, "Source record ID",
		m_strSelectedRuntimeOccurrenceEmitterPath);
	if (bVisualMatches)
	{
		AddText(Identity, "Selector SHA-256",
			pVisualRow->Selector.strSelectorSha256);
		AddText(Identity, "Source row ID",
			pVisualRow->SourceIdentity.strSourceRowId);
		AddText(Identity, "Source row SHA-256",
			pVisualRow->SourceIdentity.strSourceRowSha256);
		AddText(Identity, "Source record SHA-256",
			pVisualRow->SourceIdentity.strSourceRecordSha256);
		AddText(Identity, "Source recipe SHA-256",
			pVisualRow->SourceIdentity.strSourceRecipeSha256);
		AddText(Identity, "Module closure SHA-256",
			pVisualRow->SourceIdentity.strModuleClosureSha256);
		AddUnsigned(Identity, "Module count",
			pVisualRow->SourceIdentity.iModuleCount);
		AddText(Identity, "Target record SHA-256",
			pVisualRow->TargetIdentity->strTargetRecordSha256);
		AddText(Identity, "Target payload raw SHA-256",
			pVisualRow->TargetIdentity->strTargetPayloadRawSha256);
	}
	else
	{
		AddText(Identity, "Selector SHA-256",
			pSupplemental->Selector.strSelectorSha256);
		AddText(Identity, "Source record SHA-256",
			pSupplemental->strSourceRecordSha256);
		AddText(Identity, "Source payload raw SHA-256",
			pSupplemental->strSourcePayloadRawSha256);
		AddText(Identity, "Target record SHA-256",
			pSupplemental->TargetIdentity.strTargetRecordSha256);
		AddText(Identity, "Target payload raw SHA-256",
			pSupplemental->TargetIdentity.strTargetPayloadRawSha256);
	}
	RenderFields("selected_visual_identity", Identity);

	if (bVisualMatches)
	{
		RenderResources("Row.Resources", pVisualRow->Resources);
		if (!pVisualRow->LocalDecalPacket.has_value())
			return;
		const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET& Packet =
			*pVisualRow->LocalDecalPacket;
		EVIDENCE_FIELDS PacketFields;
		AddUnsigned(PacketFields, "Packet version", Packet.iPacketVersion);
		AddText(PacketFields, "Adapter ID", Packet.strAdapterId);
		AddBool(PacketFields, "Bounded semantic replay",
			Packet.bBoundedSemanticReplay);
		AddBool(PacketFields, "Native execution", Packet.bNativeExecution);
		AddBool(PacketFields, "Native vertex factory admitted",
			Packet.bNativeVertexFactoryAdmitted);
		AddBool(PacketFields, "Native MRT admitted",
			Packet.bNativeMrtAdmitted);
		AddText(PacketFields, "Runtime carrier", Packet.strRuntimeCarrier);
		AddText(PacketFields, "Native VF candidate",
			Packet.strNativeVertexFactoryCandidate);
		AddText(PacketFields, "Native VS SHA-256",
			Packet.strNativeVertexShaderSha256);
		AddText(PacketFields, "Native PS SHA-256",
			Packet.strNativePixelShaderSha256);
		AddText(PacketFields, "Render profile", Packet.strRenderProfile);
		AddText(PacketFields, "Rasterizer state", Packet.strRasterizerState);
		AddText(PacketFields, "Depth stencil state",
			Packet.strDepthStencilState);
		AddText(PacketFields, "Blend state", Packet.strBlendState);
		AddUnsigned(PacketFields, "Texture lane count",
			Packet.iTextureLaneCount);
		AddUnsigned(PacketFields, "Texture mask", Packet.iTextureMask);
		AddText(PacketFields, "Packet SHA-256", Packet.strPacketSha256);
		if (ImGui::TreeNodeEx("LocalDecal packet",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderFields("local_decal_packet", PacketFields);
			for (size_t iSrv = 0u; iSrv < Packet.Srvs.size(); ++iSrv)
			{
				const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV& Srv =
					Packet.Srvs[iSrv];
				EVIDENCE_FIELDS SrvFields;
				AddText(SrvFields, "Role", Srv.strRole);
				AddText(SrvFields, "Asset ID", Srv.strAssetId);
				AddText(SrvFields, "Raw SHA-256", Srv.strRawSha256);
				AddUnsigned(SrvFields, "Byte count", Srv.iByteCount);
				AddText(SrvFields, "Shader register", Srv.strShaderRegister);
				AddText(SrvFields, "Source channel", Srv.strSourceChannel);
				AddText(SrvFields, "Runtime sampler",
					Srv.strRuntimeSamplerRegister);
				AddText(SrvFields, "Source sampler evidence",
					Srv.strSourceSamplerEvidence);
				AddText(SrvFields, "Sampler policy", Srv.strSamplerPolicy);
				AddText(SrvFields, "Linear format", Srv.strLinearFormat);
				if (!SrvFields.empty())
					AddBool(SrvFields, "sRGB", Srv.bSrgb);
				AddUnsigned(SrvFields, "Width", Srv.iWidth);
				AddUnsigned(SrvFields, "Height", Srv.iHeight);
				AddUnsigned(SrvFields, "Mip count", Srv.iMipCount);
				AddUnsigned(SrvFields, "Array size", Srv.iArraySize);
				if (SrvFields.empty())
					continue;
				ImGui::PushID(static_cast<int>(iSrv));
				const std::string Label = "SRV " +
					std::to_string(iSrv) +
					(Srv.strRole.empty() ? std::string{} :
						": " + Srv.strRole);
				if (ImGui::TreeNodeEx(Label.c_str(),
					ImGuiTreeNodeFlags_DefaultOpen))
				{
					RenderFields("local_decal_srv", SrvFields);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			RenderLimitations(Packet.PreservedLimitations);
			ImGui::TreePop();
		}
		return;
	}

	RenderResources("Supplemental Resources", pSupplemental->Resources);
	EVIDENCE_FIELDS SupplementalFields;
	AddText(SupplementalFields, "Adapter ID", pSupplemental->strAdapterId);
	AddText(SupplementalFields, "Packet layout", pSupplemental->strPacketLayout);
	AddText(SupplementalFields, "Fidelity", pSupplemental->strFidelity);
	AddText(SupplementalFields, "Stage ID", pSupplemental->strStageId);
	AddText(SupplementalFields, "Source event ID",
		pSupplemental->strSourceEventId);
	AddDouble(SupplementalFields, "Source timeline seconds",
		pSupplemental->fSourceTimelineSeconds);
	AddDouble(SupplementalFields, "Local time seconds",
		pSupplemental->fLocalTimeSeconds);
	AddDouble(SupplementalFields, "Duration seconds",
		pSupplemental->fDurationSeconds);
	RenderFields("supplemental_row_packet", SupplementalFields);
	if (pSupplemental->CascadeRibbonPacket.has_value())
	{
		const EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET& Packet =
			*pSupplemental->CascadeRibbonPacket;
		EVIDENCE_FIELDS Fields;
		AddUnsigned(Fields, "Packet version", Packet.iPacketVersion);
		AddText(Fields, "Adapter ID", Packet.strAdapterId);
		AddBool(Fields, "Bounded semantic replay",
			Packet.bBoundedSemanticReplay);
		AddBool(Fields, "Native execution", Packet.bNativeExecution);
		AddText(Fields, "Runtime carrier", Packet.strRuntimeCarrier);
		AddText(Fields, "TypeData stable ID", Packet.strTypeDataStableId);
		AddText(Fields, "TypeData class", Packet.strTypeDataClassName);
		AddText(Fields, "TypeData object path", Packet.strTypeDataObjectPath);
		AddText(Fields, "TypeData module SHA-256",
			Packet.strTypeDataModuleSha256);
		AddText(Fields, "Resolved renderer", Packet.strResolvedRendererShape);
		AddDouble(Fields, "Tiling distance", Packet.fTilingDistance);
		AddDouble(Fields, "Distance tessellation step",
			Packet.fDistanceTessellationStepSize);
		AddDouble(Fields, "Tangent tessellation scalar",
			Packet.fTangentTessellationScalar);
		AddUnsigned(Fields, "Operational max points",
			Packet.iOperationalMaxPoints);
		AddText(Fields, "Source recipe SHA-256",
			Packet.strSourceRecipeSha256);
		AddText(Fields, "Module closure SHA-256",
			Packet.strModuleClosureSha256);
		AddUnsigned(Fields, "Module count", Packet.iModuleCount);
		AddText(Fields, "Packet SHA-256", Packet.strPacketSha256);
		if (ImGui::TreeNodeEx("CascadeRibbon packet",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderFields("cascade_ribbon_packet", Fields);
			RenderLimitations(Packet.PreservedLimitations);
			ImGui::TreePop();
		}
	}
	if (pSupplemental->AnimationTrailPacket.has_value())
	{
		const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET& Packet =
			*pSupplemental->AnimationTrailPacket;
		EVIDENCE_FIELDS Fields;
		AddUnsigned(Fields, "Packet version", Packet.iPacketVersion);
		AddText(Fields, "Adapter ID", Packet.strAdapterId);
		AddBool(Fields, "Bounded semantic replay",
			Packet.bBoundedSemanticReplay);
		AddBool(Fields, "Native execution", Packet.bNativeExecution);
		AddText(Fields, "Runtime carrier", Packet.strRuntimeCarrier);
		AddText(Fields, "Source notify type", Packet.strSourceNotifyType);
		AddText(Fields, "Source event ID", Packet.strSourceEventId);
		AddText(Fields, "Source event record SHA-256",
			Packet.strSourceEventRecordSha256);
		AddText(Fields, "Source asset", Packet.strSourceAsset);
		AddText(Fields, "Clip", Packet.strClip);
		AddDouble(Fields, "Local time seconds", Packet.fLocalTimeSeconds);
		AddDouble(Fields, "Global time seconds", Packet.fGlobalTimeSeconds);
		AddDouble(Fields, "Duration seconds", Packet.fDurationSeconds);
		AddText(Fields, "Target element ID", Packet.strTargetElementId);
		AddText(Fields, "Packet SHA-256", Packet.strPacketSha256);
		if (ImGui::TreeNodeEx("AnimationTrail packet",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderFields("animation_trail_packet", Fields);
			RenderLimitations(Packet.PreservedLimitations);
			ImGui::TreePop();
		}
	}
}

void Client::CEffect_Tool::Render_ParticleSystemDetail()
{
    if (!m_ActiveDocument.has_value())
        return;
    if (!m_ParticleSystemDraft.has_value())
    {
        m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
        m_bParticleSystemDraftDirty = false;
        m_strDetailStatus.clear();
    }

    const PARTICLE_LAYER_SUMMARY Summary =
        Summarize_ParticleLayers(*m_ActiveDocument);
    ImGui::Text("Cascade System: %s",
        m_ActiveDocument->strDisplayName.c_str());
    ImGui::TextDisabled(
        "Source Systems %zu | Emitters %zu | Layers %zu",
        Summary.iSourceSystemCount,
        Summary.iSourceEmitterCount,
        Summary.iLayerCount);
    ImGui::TextDisabled(
        "Mesh Particles %zu | Sprite Particles %zu | Unresolved %zu | Budget %llu",
        Summary.iMeshRendererCount, Summary.iSpriteRendererCount,
        Summary.iUnresolvedRendererCount,
        static_cast<unsigned long long>(Summary.iParticleBudget));
    ImGui::TextWrapped(
        "These controls preserve every source emitter, renderer, material, burst, "
        "and lifetime. They apply only to Cascade emitters; Model Cues and Decals "
        "are unchanged.");
	if (ImGui::Button("Open First Emitter"))
	{
		Try_SelectFirstEmitter(m_ActiveDocument->strEffectAssetId, {});
		return;
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Emitter selection reveals Renderer, Resources, and Modules.");

    bool_t bChanged = false;
    bChanged |= ImGui::DragFloat("Uniform Scale Multiplier",
        &m_ParticleSystemDraft->fUniformScaleMultiplier,
        0.01f, 0.001f, 100.f, "%.3f");
    bChanged |= ImGui::DragFloat("System Yaw Offset (Degrees)",
        &m_ParticleSystemDraft->fYawOffsetDegrees,
        0.25f, -360.f, 360.f, "%.3f");
    bChanged |= ImGui::DragFloat("Emission Direction Yaw (Degrees)",
        &m_ParticleSystemDraft->fDirectionYawDegrees,
        0.25f, -360.f, 360.f, "%.3f");
    bChanged |= ImGui::DragFloat("Initial Speed Multiplier",
        &m_ParticleSystemDraft->fInitialSpeedMultiplier,
        0.01f, 0.f, 100.f, "%.3f");
    ImGui::TextDisabled(
        "System Yaw rotates the whole layout; Direction Yaw rotates initial emission only.");
    if (bChanged)
    {
        m_bParticleSystemDraftDirty = true;
        m_strDetailStatus =
            "Live preview only; Apply Particle System commits this draft to memory.";
        Stage_ParticleSystemDraftPreview();
    }

    ImGui::Separator();
    ImGui::BeginDisabled(!m_bParticleSystemDraftDirty);
    if (ImGui::Button("Apply Particle System"))
    {
        EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
        if (!Apply_ParticleSystemDraft(Staged))
        {
            m_strDetailStatus = "Apply failed; Particle System draft is missing.";
        }
        else if (Try_CommitDocument(std::move(Staged)))
        {
            m_bParticleSystemDraftDirty = false;
            m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
            m_strDetailStatus =
                "Applied Particle System to active Document memory; Save required to persist.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert Particle System"))
    {
        m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
        m_bParticleSystemDraftDirty = false;
        Recalculate_PreviewDuration();
        if (Stage_WorldPreview())
            m_strPreviewStatus =
                "Particle System draft reverted to the active Document preview.";
        m_strDetailStatus =
            "Reverted the Particle System draft to the active Document.";
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Audition Particle System"))
        Try_AuditionParticleSystem();
	ImGui::SameLine();
	if (ImGui::Button("Play Mesh Particles") &&
		Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS))
	{
		Start_WorldPreviewFromBeginning();
	}
    if (m_bParticleSystemDraftDirty)
        ImGui::TextDisabled(
            "Particle System draft is local until Apply Particle System.");
    if (!m_strDetailStatus.empty())
        ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
}

void Client::CEffect_Tool::Render_Detail(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
	const EFFECT_AUTHORING_FAMILY eSurface = Resolve_AuthoringFamily(Element);
	const char* pSurface = EFFECT_AUTHORING_FAMILY::END == eSurface ?
		Kind_Label(Element.eKind) : AuthoringFamily_Label(eSurface);
	const EFFECT_AUTHORING_FIDELITY eFidelity =
		Get_EffectAuthoringFidelity(Element.Material.Execution);
	bool_t bModified = !Element.AuthoringOverrides.Is_Empty();
	ImGui::Text("%s %s %s%s", pSurface, "\xC2\xB7",
		Get_EffectAuthoringFidelityLabel(eFidelity),
		bModified ? " \xC2\xB7 Modified" : "");
	const bool_t bTypedPresentationElement =
		Element.eKind == EFFECT_ELEMENT_KIND::LIGHT ||
		Element.eKind == EFFECT_ELEMENT_KIND::SCREEN_POST;
	/* Drawable source carriers keep their compiler module ownership. Typed
	   presentation carriers are different: source modules still supply their
	   lifetime/curves, while Detail.Light/ScreenPost owns the submitted output. */
	if (Element.SourceRecipe.bEnabled && !bTypedPresentationElement)
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"SourceRecipe drives native spawn, lifetime, motion, size, and dynamic values.");
		ImGui::TextDisabled(
			"%zu source modules. Only the explicitly labelled authored overlays below remain live.",
			Element.SourceRecipe.Modules.size());
	}
	else if (bTypedPresentationElement)
	{
		ImGui::TextDisabled(
			"Typed Presentation Detail owns output; source modules retain lifetime and curve evaluation only.");
	}
	else
	{
		ImGui::TextDisabled("Authored Detail owns playback.");
	}
	if (bModified)
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset All to Source"))
		{
			std::string strError;
			if (Reset_AllAuthoringOverrides(Element, strError))
			{
				bChanged = true;
				bModified = false;
				m_strDetailStatus =
					"All authoring overrides reset to source values.";
			}
			else
			{
				m_strDetailStatus = "Reset to Source failed: " + strError;
			}
		}
	}
	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID &&
		nullptr != m_pArtistFSourcePreparation &&
		nullptr != m_pArtistFSourcePreparation->Get_Program())
	{
		const auto TrackASource = std::find_if(
			m_pArtistFSourcePreparation->Get_Program()->Emitters.begin(),
			m_pArtistFSourcePreparation->Get_Program()->Emitters.end(),
			[&Element](const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter)
			{
				EFFECT_GPU_RENDER_FAMILY eFamily =
					EFFECT_GPU_RENDER_FAMILY::END;
				return Emitter.bVisible &&
					Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily) &&
					StableUnifiedElementId(eFamily, Emitter.Row.strId) ==
						Element.strElementId;
			});
		if (TrackASource !=
			m_pArtistFSourcePreparation->Get_Program()->Emitters.end())
		{
			if (TrackASource->ActionCueAttachment.bEnabled &&
				TrackASource->ActionCueAttachment.bFollow)
			{
				ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
					"Track A Live Follow not migrated: this Element keeps its emit-start baked Transform approximation.");
			}
			const auto Snapshot = m_ArtistFMaterialExecutionSnapshots.find(
				TrackASource->strSourceElementId);
			if (Snapshot != m_ArtistFMaterialExecutionSnapshots.end() &&
				!Snapshot->second.bEnabled)
			{
				ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
					"Track A Material unresolved: no typed recipe was baked for this row; its existing authored material was preserved.");
			}
		}
	}
	const bool_t bMissingBaseSourceDecal =
		Is_MissingBaseSourceDecal(Element);
	const bool_t bAuthoringExecutionTarget =
		Is_EffectAuthoringExecutionTarget(Element.Material.Execution);
	if (m_bDetailDraftCapabilityDeferred)
	{
		bool_t bLockedVisible = false;
		ImGui::BeginDisabled();
		ImGui::Checkbox("Visible", &bLockedVisible);
		ImGui::EndDisabled();
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
			"Visibility locked OFF: this SourceRecipe carrier is capability-deferred and cannot be safely previewed or approved.");
		ImGui::TextWrapped("Reason: %s",
			m_strDetailDraftCapabilityReason.c_str());
	}
	else if (bMissingBaseSourceDecal)
	{
		bool_t bLockedVisible = false;
		ImGui::BeginDisabled();
		ImGui::Checkbox("Visible", &bLockedVisible);
		ImGui::EndDisabled();
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"Visibility locked OFF: this imported Decal has no Base DDS.");
		ImGui::TextWrapped(
			"In Resources, select the empty Base input, choose one DDS, then click Bind Selected. A valid Base bind clears this exact material fail-closed marker and enables preview; Transform, rotation, scale, and color remain editable while locked.");
	}
	else if (eFidelity == EFFECT_AUTHORING_FIDELITY::APPROXIMATE)
	{
		bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
		ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.55f, 1.f),
			"PROJECT_TUNED APPROXIMATE | Preview enabled | editable and tunable");
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"The typed runtime packet and Adapter are validated, but original Material arithmetic is not SOURCE_EXACT. Overrides never change this fidelity label.");
		}
	}
	else if (eFidelity == EFFECT_AUTHORING_FIDELITY::PROJECT_TUNED_APPROX)
	{
		bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
		ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.55f, 1.f),
			"PROJECT_TUNED_APPROX | Runtime enabled | editable and tunable");
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"This admitted Program/Layout/Descriptor/Adapter packet is project-tuned, not a source-exact material replay.");
		}
	}
	else if (!bAuthoringExecutionTarget)
	{
		bool_t bLockedVisible = false;
		ImGui::BeginDisabled();
		ImGui::Checkbox("Visible", &bLockedVisible);
		ImGui::EndDisabled();
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
			"Visibility locked OFF: this material/runtime carrier is fail-closed.");
		ImGui::TextWrapped(
			"Element editing and Save remain available, but Play/Solo stay disabled until a supported admission path explicitly clears this marker.");
	}
	else
	{
		bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
	}
	Render_TransformDetail(Element.Detail, bChanged);
	Render_TimingDetail(Element, bChanged);
	Render_SizeDetail(Element, bChanged);
	Render_ColorDetail(Element.Detail, bChanged,
		nullptr != Find_Binding(Element, "emissive"));
	Render_AuthoringMaterialParameters(Element, bChanged);
	if (ImGui::CollapsingHeader("Advanced Authoring"))
	{
		Render_UVDetail(Element.Detail, bChanged);
		Render_UVKeyframes(Element, bChanged);
		Render_KindDetail(Element, bChanged);
		Render_LerpDetail(Element.Detail, bChanged);
	}
	if (!ImGui::CollapsingHeader("Advanced Diagnostics"))
		return;
	Render_SelectedVisualProgramEvidence();
	ImGui::TextDisabled("Element ID: %s", Element.strElementId.c_str());
	ImGui::Text("Display Name: %s", Element.strDisplayName.c_str());
	ImGui::TextDisabled("Group: %s",
		Element.strGroupId.empty() ? "(none)" : Element.strGroupId.c_str());
	ImGui::TextDisabled("Source Node: %s",
		Element.strSourceNode.empty() ? "(authored)" :
			Element.strSourceNode.c_str());
	const EFFECT_GENERIC_AUTHORING_FAMILY eGenericFamily =
		Resolve_EffectGenericAuthoringFamily(Element);
	ImGui::TextDisabled("Automatic authoring family: %s",
		Get_EffectGenericAuthoringFamilyLabel(eGenericFamily));
	if (bModified && ImGui::TreeNode("Raw Authoring Overrides"))
	{
		ImGui::TextDisabled(
			"Read-only diagnostics. Use the field-level Reset to Source controls above.");
		for (const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override :
			Element.AuthoringOverrides.ResourceBindings)
		{
			ImGui::BulletText("%s: %s (source %s)",
				Override.strSlotId.c_str(), Override.strAssetId.c_str(),
				Override.strCompilerAssetId.empty() ? "(none)" :
					Override.strCompilerAssetId.c_str());
		}
		for (const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Scalar :
			Element.AuthoringOverrides.Scalars)
		{
			ImGui::BulletText("%s: %.4f (source %.4f)",
				Scalar.strName.c_str(), Scalar.fValue,
				Scalar.fCompilerValue);
		}
		for (const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Color :
			Element.AuthoringOverrides.Colors)
		{
			ImGui::BulletText("%s: %.3f %.3f %.3f %.3f",
				Color.strName.c_str(), Color.vValue.x, Color.vValue.y,
				Color.vValue.z, Color.vValue.w);
		}
		ImGui::TreePop();
	}
    ImGui::TextDisabled("Material Template: %s",
        Element.Material.strTemplateId.c_str());
	const EFFECT_MATERIAL_EXECUTION_DESC& MaterialExecution =
		Element.Material.Execution;
	if (!Is_EffectAuthoringExecutionTarget(MaterialExecution) &&
		!bMissingBaseSourceDecal)
	{
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
			"Fail-closed source pass: generic/white fallback is disabled. This Element will not draw until a supported authored recipe replaces it.");
	}
	if (MaterialExecution.bEnabled &&
		ImGui::CollapsingHeader("Track A Material Slots"))
	{
		ImGui::Text("Recipe: %s",
			MaterialExecutionBackendLabel(MaterialExecution.eBackend));
		ImGui::TextDisabled(
			"Each card preserves the source semantic role, t#/s# register, channel, color space, and sampler. Select a card, then bind one DDS in the Resource Library.");
		const float fCardWidth = 108.f;
		const size_t iColumns = static_cast<size_t>((std::max)(1,
			static_cast<int32_t>(
				ImGui::GetContentRegionAvail().x / fCardWidth)));
		for (size_t iLane = 0u;
			iLane < MaterialExecution.TextureLanes.size(); ++iLane)
		{
			const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				MaterialExecution.TextureLanes[iLane];
			if (0u != iLane % iColumns)
				ImGui::SameLine();
			ImGui::PushID(Lane.strLaneId.c_str());
			ImGui::BeginGroup();
			const CEffectThumbnailCache::RESULT Thumbnail =
				m_pThumbnailCache->Request(
					Lane.strAssetId, EFFECT_RESOURCE_FILE_KIND::TEXTURE);
			bool_t bClicked = false;
			if (nullptr != Thumbnail.pTextureView)
			{
				ImGui::Image(Thumbnail.pTextureView, ImVec2(82.f, 58.f));
				bClicked = ImGui::IsItemClicked();
			}
			else
			{
				bClicked = ImGui::Button(
					Lane.strAssetId.empty() ? "Empty DDS" : "DDS",
					ImVec2(82.f, 58.f));
				if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
					ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
			}
			const std::string strSlotId =
				MaterialExecutionLaneSlotId(Lane.strLaneId);
			if (m_strSelectedResourceSlotId == strSlotId)
			{
				ImGui::GetWindowDrawList()->AddRect(
					ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
					ImGui::GetColorU32(ImGuiCol_HeaderActive),
					2.f, 0, 2.f);
			}
			if (bClicked)
			{
				m_strSelectedResourceSlotId = strSlotId;
				m_strSelectedResourceAssetId.clear();
				m_eResourceLibraryFileKind =
					EFFECT_RESOURCE_FILE_KIND::TEXTURE;
			}
			std::string strRole = Lane.strRole.empty() ?
				Lane.strLaneId : Lane.strRole;
			if (strRole.size() > 14u)
				strRole = strRole.substr(0u, 12u) + "..";
			ImGui::TextUnformatted(strRole.c_str());
			ImGui::TextDisabled("t%u / s%u / %s",
				Lane.iTextureRegister, Lane.iSamplerRegister,
				Lane.strSourceChannel.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Role: %s\nLane: %s\nDDS: %s\nColor: %s\nFilter: %s",
					Lane.strRole.c_str(), Lane.strLaneId.c_str(),
					Lane.strAssetId.c_str(),
					MaterialTextureColorSpaceLabel(Lane.eColorSpace),
					MaterialTextureFilterLabel(Lane.Sampler.eFilter));
			}
			ImGui::EndGroup();
			ImGui::PopID();
		}
		if (ImGui::TreeNode("Advanced Material Packet"))
		{
			ImGui::TextDisabled(
				"Opcode %u | Pass %u | Texture mask 0x%08X",
				MaterialExecution.iOpcode,
				MaterialExecution.iPassIndex,
				MaterialExecution.iTextureMask);
			ImGui::TextWrapped("Rasterizer: %s",
				MaterialExecution.strRasterizerState.c_str());
			ImGui::TextWrapped("Depth/Stencil: %s",
				MaterialExecution.strDepthStencilState.c_str());
			ImGui::TextWrapped("Blend: %s",
				MaterialExecution.strBlendState.c_str());
			ImGui::TextDisabled(
				"Scalars %zu | Vectors %zu | Artist Params %zu | Colors %zu",
				MaterialExecution.Scalars.size(),
				MaterialExecution.Vectors.size(),
				MaterialExecution.ArtistParameters.size(),
				MaterialExecution.Colors.size());
			ImGui::TreePop();
		}
	}
	const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
		Element.Material.SourceMaterial;
	if (SourceMaterial.bEnabled &&
		ImGui::CollapsingHeader("Source Material Profile"))
	{
		ImGui::TextDisabled("Read-only compiler output.");
		ImGui::Text("Status: %s",
			SourceMaterialStatus_Label(SourceMaterial.eStatus));
		ImGui::TextWrapped("Source: %s",
			Element.Material.strSourceMaterialPath.c_str());
		ImGui::TextWrapped("Parent: %s",
			SourceMaterial.strParentMaterialPath.c_str());
		ImGui::TextWrapped("Profile: %s",
			SourceMaterial.strProfileId.c_str());
		ImGui::TextWrapped("Runtime Shader: %s",
			SourceMaterial.strRuntimeShaderProfileId.c_str());
		ImGui::Text("SubUV: %s", SourceMaterial.strSubUVMode.c_str());
		ImGui::TextDisabled("Dynamic: X=%s Y=%s Z=%s W=%s",
			SourceMaterial.DynamicParameterSemantics[0].c_str(),
			SourceMaterial.DynamicParameterSemantics[1].c_str(),
			SourceMaterial.DynamicParameterSemantics[2].c_str(),
			SourceMaterial.DynamicParameterSemantics[3].c_str());
		Render_SourceMaterialParameterGroups(
			SourceMaterial, m_pThumbnailCache.get());
	}
    if (ImGui::CollapsingHeader("Runtime Sample"))
    {
        const bool_t bHasBaseBinding = std::any_of(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [](const EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == "base";
            });
        const bool_t bFallbackBlocked =
            SourceMaterial.strRuntimeShaderProfileId ==
                "effect.ue3.fallback-blocked.v1";
        const bool_t bGenericReconstructed =
            SourceMaterial.strRuntimeShaderProfileId ==
                "effect.ue3.reconstructed-standard.v1";
        const bool_t bUnsafeBaseBinding = std::any_of(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [](const EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == "base" &&
                    Is_UnsafeEffectBaseTextureAssetId(Binding.strAssetId);
            });
        const bool_t bRuntimeFallbackBlocked = bFallbackBlocked ||
            (bGenericReconstructed &&
                (!bHasBaseBinding || bUnsafeBaseBinding));
        uint32_t iMeshMaterialOverrideCount = 0u;
        for (const EFFECT_SOURCE_MODULE_DESC& Module :
            Element.SourceRecipe.Modules)
        {
            if (Module.strClassName.find("meshmaterial") == std::string::npos)
                continue;
            iMeshMaterialOverrideCount += static_cast<uint32_t>(std::count_if(
                Module.Literals.begin(), Module.Literals.end(),
                [](const EFFECT_SOURCE_LITERAL_DESC& Literal)
                {
                    return Literal.strPropertyPath.starts_with("meshmaterials[") &&
                        Literal.strPropertyPath.ends_with("].objectpath");
                }));
        }
        ImGui::TextWrapped("Source Material: %s",
            Element.Material.strSourceMaterialPath.c_str());
        ImGui::TextWrapped("Parent: %s",
            SourceMaterial.strParentMaterialPath.c_str());
        ImGui::TextWrapped("Profile: %s | Runtime Shader: %s",
            SourceMaterial.strProfileId.c_str(),
            SourceMaterial.strRuntimeShaderProfileId.c_str());
        ImGui::Text("Semantic: %s | Render: %s",
            SourceMaterialStatus_Label(SourceMaterial.eStatus),
            Profile_Label(Element.Material.eRenderProfile));
        ImGui::Text("Mesh model material: %s | Per-section overrides: %u",
            Element.Detail.Mesh.bUseModelMaterial ? "use" : "override",
            iMeshMaterialOverrideCount);
        if (ImGui::TreeNode("Resolved Runtime Resources"))
        {
            for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
                Element.ResourceBindings)
            {
                ImGui::BulletText("%s = %s [staged-loaded]",
                    Binding.strSlotId.c_str(), Binding.strAssetId.c_str());
            }
            if (Element.ResourceBindings.empty())
                ImGui::TextDisabled("(none)");
            ImGui::TreePop();
        }
        ImGui::Text("Fallback: %s",
            bRuntimeFallbackBlocked ? "FAIL-CLOSED / emitter skipped" :
            !bHasBaseBinding ? "missing Base (white fallback risk)" :
            "no missing-Base fallback");
        EFFECT_PARTICLE_RUNTIME_PROBE Probe;
        const shared_ptr<CEffectObject> pPreview =
            m_pWorldPreviewObject.lock();
        const bool_t bFound = nullptr != pPreview &&
            pPreview->Query_ParticleRuntimeProbe(
                Element.strElementId, Probe);
        if (!bFound)
        {
            ImGui::TextDisabled(
                "No staged runtime element for the current selection.");
        }
        else
        {
            ImGui::Text("Sample: %.6f s | Active: %u | Renderer: %s",
                Probe.fSampleTimeSeconds, Probe.iActiveParticleCount,
                Probe.bMeshRenderer ? "Mesh Particle" : "Sprite Particle");
            if (0u == Probe.iActiveParticleCount)
            {
                ImGui::TextDisabled(
                    "The selected emitter has no live particles at this sample.");
            }
            else
            {
                ImGui::Text("CPU pre-material alpha first/min/max: %.6g / %.6g / %.6g",
                    Probe.fFirstAlpha, Probe.fMinAlpha, Probe.fMaxAlpha);
                ImGui::TextDisabled(
                    "Final GPU Material opacity is not sampled by this probe.");
                ImGui::Text(
                    "Dynamic first: [%.6g, %.6g, %.6g, %.6g]",
                    Probe.vFirstDynamicParameter.x,
                    Probe.vFirstDynamicParameter.y,
                    Probe.vFirstDynamicParameter.z,
                    Probe.vFirstDynamicParameter.w);
                ImGui::Text(
                    "Dynamic min: [%.6g, %.6g, %.6g, %.6g]",
                    Probe.vMinDynamicParameter.x,
                    Probe.vMinDynamicParameter.y,
                    Probe.vMinDynamicParameter.z,
                    Probe.vMinDynamicParameter.w);
                ImGui::Text(
                    "Dynamic max: [%.6g, %.6g, %.6g, %.6g]",
                    Probe.vMaxDynamicParameter.x,
                    Probe.vMaxDynamicParameter.y,
                    Probe.vMaxDynamicParameter.z,
                    Probe.vMaxDynamicParameter.w);
                ImGui::Text("Life: %.6g | Raw SubImage: %.6g",
                    Probe.fFirstNormalizedLife,
                    Probe.fFirstSubImageIndex);
                ImGui::Text(
                    "Resolved SubUV current=[%.6g, %.6g, %.6g, %.6g] "
                    "next=[%.6g, %.6g, %.6g, %.6g] blend=%.6g",
                    Probe.FirstSubUV.Current.x, Probe.FirstSubUV.Current.y,
                    Probe.FirstSubUV.Current.z, Probe.FirstSubUV.Current.w,
                    Probe.FirstSubUV.Next.x, Probe.FirstSubUV.Next.y,
                    Probe.FirstSubUV.Next.z, Probe.FirstSubUV.Next.w,
                    Probe.FirstSubUV.fBlend);
            }
            if (ImGui::Button("Copy Runtime Probe"))
            {
                std::ostringstream Text;
                Text << "effect="
                    << (m_ActiveDocument.has_value() ?
                        m_ActiveDocument->strEffectAssetId : "(none)")
                    << " element=" << Element.strElementId
                    << " sample=" << Probe.fSampleTimeSeconds
                    << " active=" << Probe.iActiveParticleCount
                    << " renderer="
                    << (Probe.bMeshRenderer ? "mesh" : "sprite")
                    << " source_material="
                    << Element.Material.strSourceMaterialPath
                    << " parent=" << SourceMaterial.strParentMaterialPath
                    << " profile=" << SourceMaterial.strProfileId
                    << " runtime_shader="
                    << SourceMaterial.strRuntimeShaderProfileId
                    << " semantic="
                    << SourceMaterialStatus_Label(SourceMaterial.eStatus)
                    << " render_profile="
                    << Profile_Label(Element.Material.eRenderProfile)
                    << " mesh_use_model_material="
                    << (Element.Detail.Mesh.bUseModelMaterial ? 1 : 0)
                    << " mesh_section_overrides="
                    << iMeshMaterialOverrideCount
                    << " fallback="
                    << (bRuntimeFallbackBlocked ? "fail_closed" :
                        !bHasBaseBinding ? "missing_base_white_risk" : "none")
                    << " cpu_pre_material_alpha=" << Probe.fFirstAlpha << '/'
                    << Probe.fMinAlpha << '/' << Probe.fMaxAlpha
                    << " dynamic=[" << Probe.vFirstDynamicParameter.x << ','
                    << Probe.vFirstDynamicParameter.y << ','
                    << Probe.vFirstDynamicParameter.z << ','
                    << Probe.vFirstDynamicParameter.w << ']'
                    << " life=" << Probe.fFirstNormalizedLife
                    << " subimage=" << Probe.fFirstSubImageIndex
                    << " subuv_current=[" << Probe.FirstSubUV.Current.x << ','
                    << Probe.FirstSubUV.Current.y << ','
                    << Probe.FirstSubUV.Current.z << ','
                    << Probe.FirstSubUV.Current.w << ']'
                    << " subuv_next=[" << Probe.FirstSubUV.Next.x << ','
                    << Probe.FirstSubUV.Next.y << ','
                    << Probe.FirstSubUV.Next.z << ','
                    << Probe.FirstSubUV.Next.w << ']'
                    << " subuv_blend=" << Probe.FirstSubUV.fBlend;
                for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
                    Element.ResourceBindings)
                {
                    Text << " resource[" << Binding.strSlotId << "]="
                        << Binding.strAssetId << ":staged_loaded";
                }
                ImGui::SetClipboardText(Text.str().c_str());
            }
        }
        ImGui::TextDisabled(
            "Read-only evaluated playback data; no authoring values are changed.");
    }
	if (Element.SourceRecipe.bEnabled &&
		ImGui::TreeNode("Original Emitter / Module Stack"))
	{
		ImGui::TextDisabled("Read-only compiler output.");
		ImGui::Text("Renderer: %s",
			Element.SourceRecipe.strRendererShape.empty() ? "(unspecified)" :
				Element.SourceRecipe.strRendererShape.c_str());
		ImGui::TextDisabled("Delay %.6f | Duration %.6f | Loops %u",
			Element.SourceRecipe.fEmitterDelaySeconds,
			Element.SourceRecipe.fEmitterDurationSeconds,
			Element.SourceRecipe.iEmitterLoopCount);
		ImGui::TextDisabled("Bursts %zu | Modules %zu",
			Element.SourceRecipe.Bursts.size(),
			Element.SourceRecipe.Modules.size());
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			ImGui::BulletText("%s | %s",
				Module.strStableId.c_str(), Module.strClassName.c_str());
			if (ImGui::IsItemHovered() && !Module.strObjectPath.empty())
				ImGui::SetTooltip("%s", Module.strObjectPath.c_str());
		}
		ImGui::TreePop();
	}
	/* The compiler derives the blend from the source material, but an Element
	   authored by hand has no source material to derive it from and simply
	   keeps the struct default of alpha two-sided. That default draws a glow
	   texture as an opaque quad, so leaving it read-only on authored Elements
	   locked the one control that fixes it. */
	const bool_t bCompilerOwnsRenderProfile =
		!Element.Material.strSourceMaterialPath.empty() ||
		Element.Material.SourceMaterial.bEnabled ||
		Element.Material.Execution.bEnabled;
	ImGui::SeparatorText(bCompilerOwnsRenderProfile ?
		"Compiler-owned Render Profile" : "Render Profile");
	if (bCompilerOwnsRenderProfile)
	{
		ImGui::TextDisabled("%s", Profile_Label(Element.Material.eRenderProfile));
		return;
	}
	static const char* const s_RenderProfileLabels[] =
	{
		"Opaque (back faces, depth write)",
		"Alpha (two sided, depth read)",
		"Additive (two sided, depth read)",
		"Alpha (one sided, depth read)",
		"Additive (one sided, depth read)"
	};
	int32_t iRenderProfile = static_cast<int32_t>(
		Element.Material.eRenderProfile);
	if (ImGui::Combo("Blend", &iRenderProfile, s_RenderProfileLabels,
		IM_ARRAYSIZE(s_RenderProfileLabels)))
	{
		Element.Material.eRenderProfile =
			static_cast<EFFECT_RENDER_PROFILE>(iRenderProfile);
		bChanged = true;
	}
	ImGui::TextDisabled(
		"Additive treats black as transparent, which is how glow and flare textures are drawn. Alpha needs the texture to carry its own alpha channel.");
}

void Client::CEffect_Tool::Render_TransformDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    ImGui::TextDisabled(
        "Start values update the live preview immediately; Lerp checkboxes only enable Start-to-End interpolation.");
    bChanged |= DragFloat3(
        "Position", Detail.Transform.vPosition, 0.01f, -1000.f, 1000.f);
    bChanged |= DragFloat3("Rotation (Degrees)",
        Detail.Transform.vRotationDegrees, 0.25f, -360.f, 360.f);
    bChanged |= DragFloat3("Revolution (Degrees/Second)",
        Detail.Transform.vRevolutionDegreesPerSecond,
        0.5f, -3600.f, 3600.f);
    bChanged |= DragFloat3(
        "Scaling", Detail.Transform.vScale, 0.01f, 0.001f, 100.f);
    bChanged |= DragFloat3("Velocity",
        Detail.Transform.vVelocityPerSecond, 0.01f, -1000.f, 1000.f);
}

void Client::CEffect_Tool::Render_ColorDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged,
    const bool_t bHasEmissiveTexture)
{
    if (!ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= DragFloat4(
        "Color Offset", Detail.Color.vColorOffset, 0.01f, -10.f, 10.f);
    bChanged |= DragFloat4(
        "Color Multiply", Detail.Color.vColorMultiply, 0.01f, 0.f, 10.f);
    bChanged |= ImGui::SliderFloat("Color Clip (Alpha Threshold)",
        &Detail.Color.fColorClip, 0.f, 1.f);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(
            "Current v12 runtime clips pixels below this final alpha threshold.");
	ImGui::BeginDisabled(!bHasEmissiveTexture);
	bChanged |= ImGui::DragFloat("Emissive Intensity (HDR)",
		&Detail.Color.fEmissiveIntensity, 0.05f, 0.f, 100.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	if (!bHasEmissiveTexture)
		ImGui::TextDisabled(
			"Emissive Intensity is inactive until an Emissive texture is bound.");
	else
		ImGui::TextDisabled(
			"This scales the Emissive texture's own RGB. It is not the scene Bloom multiplier; grayscale Emissive can produce a white core at high values.");
    bChanged |= ImGui::DragFloat("Distortion Intensity",
        &Detail.Color.fDistortionIntensity, 0.01f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::Checkbox("Distortion On Base Material",
        &Detail.Color.bDistortionOnBaseMaterial);
    bChanged |= ImGui::DragFloat("Radial Time",
        &Detail.Color.fRadialTime, 0.01f, -100.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("Radial Intensity",
        &Detail.Color.fRadialIntensity, 0.01f, -100.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
}

void Client::CEffect_Tool::Render_UVDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("UV", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= DragFloat2(
        "UV Start", Detail.UV.vStart, 0.001f, -100.f, 100.f);
    bChanged |= DragFloat2(
        "UV Speed", Detail.UV.vSpeed, 0.001f, -100.f, 100.f);
    bChanged |= ImGui::Checkbox("UV Wave", &Detail.UV.bWave);
    bChanged |= DragFloat2("Wave Amplitude",
        Detail.UV.vWaveAmplitude, 0.001f, -100.f, 100.f);
    bChanged |= ImGui::DragFloat("Wave Frequency",
        &Detail.UV.fWaveFrequency, 0.01f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::Checkbox("UV Sequence", &Detail.UV.bSequence);
    bChanged |= ImGui::Checkbox("UV Loop", &Detail.UV.bLoop);
    bChanged |= ImGui::DragFloat("Sequence Term",
        &Detail.UV.fSequenceTerm, 0.001f, 0.001f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bool_t bTileChanged = ImGui::DragInt(
        "UV Tile Columns", &Detail.UV.iTileColumns, 0.1f, 1, 64, "%d",
        ImGuiSliderFlags_AlwaysClamp);
    bTileChanged |= ImGui::DragInt(
        "UV Tile Rows", &Detail.UV.iTileRows, 0.1f, 1, 64, "%d",
        ImGuiSliderFlags_AlwaysClamp);
    const int32_t iMaximumTile = (std::max)(
        0, Detail.UV.iTileColumns * Detail.UV.iTileRows - 1);
    bTileChanged |= ImGui::DragInt(
        "UV Tile Index", &Detail.UV.iTileIndex, 0.1f, 0, iMaximumTile,
        "%d", ImGuiSliderFlags_AlwaysClamp);
    Detail.UV.iTileIndex = std::clamp(
        Detail.UV.iTileIndex, 0, iMaximumTile);
    bChanged |= bTileChanged;
}

void Client::CEffect_Tool::Render_UVKeyframes(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
    EFFECT_UV_DESC& UV = Element.Detail.UV;
    if (!UV.bSequence || UV.iTileColumns <= 0 || UV.iTileRows <= 0)
        return;
    const auto Base = std::find_if(
        Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
        [](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.strSlotId == "base";
        });
    if (Base == Element.ResourceBindings.end())
    {
        ImGui::TextDisabled("Keyframes require a Base texture.");
        return;
    }
    const CEffectThumbnailCache::RESULT Thumbnail =
        m_pThumbnailCache->Request(
            Base->strAssetId, EFFECT_RESOURCE_FILE_KIND::TEXTURE);
    if (nullptr == Thumbnail.pTextureView)
    {
        ImGui::TextDisabled("Keyframe thumbnail is loading.");
        return;
    }
    ImGui::SeparatorText("Keyframes");
    const int64_t iTotal = static_cast<int64_t>(UV.iTileColumns) * UV.iTileRows;
    const int32_t iVisible = static_cast<int32_t>((std::min<int64_t>)(iTotal, 64));
    for (int32_t iTile = 0; iTile < iVisible; ++iTile)
    {
        if (0 != iTile % 8)
            ImGui::SameLine();
        ImGui::PushID(iTile);
        const int32_t iColumn = iTile % UV.iTileColumns;
        const int32_t iRow = iTile / UV.iTileColumns;
        const ImVec2 UV0(
            static_cast<float>(iColumn) / UV.iTileColumns,
            static_cast<float>(iRow) / UV.iTileRows);
        const ImVec2 UV1(
            static_cast<float>(iColumn + 1) / UV.iTileColumns,
            static_cast<float>(iRow + 1) / UV.iTileRows);
        const ImVec4 Border = iTile == UV.iTileIndex ?
            ImVec4(0.2f, 0.75f, 1.f, 1.f) : ImVec4(0.f, 0.f, 0.f, 0.f);
        ImGui::Image(Thumbnail.pTextureView, ImVec2(42.f, 42.f),
            UV0, UV1, ImVec4(1.f, 1.f, 1.f, 1.f), Border);
        if (ImGui::IsItemClicked())
        {
            UV.iTileIndex = iTile;
            bool_t bCommitted = false;
            if (m_ActiveDocument.has_value())
            {
                EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
                for (EFFECT_ELEMENT_DESC& Active : Staged.Elements)
                {
                    if (Active.strElementId != Element.strElementId)
                        continue;
                    Active.Detail.UV.iTileIndex = iTile;
                    bCommitted = Try_CommitDocument(std::move(Staged));
                    break;
                }
            }
            if (bCommitted)
            {
                m_fPreviewTimeSeconds = 0.f;
                if (const shared_ptr<CEffectObject> pObject =
                    m_pWorldPreviewObject.lock())
                    pObject->Set_SampleTime(
                        Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
            }
            else
                bChanged = true;
        }
        ImGui::PopID();
    }
    if (iTotal > iVisible)
        ImGui::TextDisabled("Showing the first 64 sequence tiles.");
}

void Client::CEffect_Tool::Render_TimingDetail(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	if (!ImGui::CollapsingHeader("Timing", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	EFFECT_DETAIL_DESC& Detail = Element.Detail;
	bChanged |= ImGui::DragFloat("Life Time",
		&Detail.Timing.fLifeTimeSeconds, 0.01f, 0.001f, 60.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	bChanged |= ImGui::DragFloat("Start Delay Timer",
		&Detail.Timing.fStartDelaySeconds, 0.01f, 0.f, 60.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
		Element.eKind == EFFECT_ELEMENT_KIND::SPRITE)
	{
		bChanged |= ImGui::DragFloat("After Image Timer",
			&Detail.Timing.fAfterImageSeconds, 0.01f, 0.f, 60.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
	}
	if (Is_SourceParticleCarrier(Element))
	{
		ImGui::TextDisabled(
			"Start Delay positions this Element in the clip. The SourceRecipe owns its native emitter schedule; Life Time remains the authored emission-window fallback.");
	}
	bChanged |= ImGui::SliderFloat("Dissolve Start",
		&Detail.Timing.fDissolveStartNormalized, 0.f, 1.f);
}

void Client::CEffect_Tool::Render_SizeDetail(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	const bool_t bSourceParticleCarrier =
		Is_SourceParticleCarrier(Element);
	const bool_t bHasSizeSurface =
		Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
		Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE ||
		Element.eKind == EFFECT_ELEMENT_KIND::DECAL ||
		Element.eKind == EFFECT_ELEMENT_KIND::TRAIL ||
		bSourceParticleCarrier;
	if (!bHasSizeSurface ||
		!ImGui::CollapsingHeader(bSourceParticleCarrier ?
			"Source Playback Tuning###SizeDetail" : "Size###SizeDetail",
			ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}
	if (bSourceParticleCarrier)
	{
		if (Element.SourceRecipe.strRendererShape == "mesh")
		{
			ImGui::SeparatorText("Mesh Carrier Geometry");
			bChanged |= ImGui::DragFloat("Model Import Scale",
				&Element.Detail.Mesh.fModelPreScale, 0.001f, 0.0001f, 100.f,
				"%.4f", ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Applied once to the WModel carrier. It is independent from the source particle Size multiplier below.");
			}
		}
		ImGui::SeparatorText("Evaluated Source Multipliers");
		EFFECT_PARTICLE_SOURCE_SCALE_DESC& Tuning =
			Element.Detail.Particle.SourceScale;
		bChanged |= ImGui::DragFloat("Count x", &Tuning.fCount,
			0.01f, 0.01f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Scales source rate, fixed bursts, and the particle ceiling. SpawnPerUnit and event-receiver counts remain module-owned.");
		}
		bChanged |= ImGui::DragFloat("Size x", &Tuning.fSize,
			0.01f, 0.01f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Life x", &Tuning.fLifeTime,
			0.01f, 0.01f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Speed x", &Tuning.fSpeed,
			0.01f, -16.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Scales source spawn/base velocity. A later absolute VelocityOverLife or vector-field module may still own the final velocity.");
		}
		bChanged |= ImGui::DragFloat("Rotation x", &Tuning.fRotation,
			0.01f, -16.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Alpha x", &Tuning.fAlpha,
			0.01f, 0.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Scales source base alpha. A later absolute ColorOverLife module may still own the final alpha.");
		}
		ImGui::TextDisabled(
			"These multipliers tune supported source carrier axes without replacing its modules. Use Timing > Start Delay to place the Element inside the clip; Start/End Size and raw spawn fields belong to a manual Element.");
		if (Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
		{
			ImGui::SeparatorText("Decal Projection");
			bChanged |= ImGui::DragFloat("Projection Depth",
				&Element.Detail.Decal.fDepth, 0.01f, 0.001f, 1000.f, "%.3f",
				ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Projection depth is a working Decal overlay. Source Size x owns the projected width and height.");
			}
		}
		if (!Tuning.Is_Default() &&
			ImGui::Button("Reset Source Playback Tuning"))
		{
			Tuning = EFFECT_PARTICLE_SOURCE_SCALE_DESC{};
			bChanged = true;
		}
		return;
	}
	if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
		(Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			Resolve_AuthoringFamily(Element) ==
				EFFECT_AUTHORING_FAMILY::MESH_PARTICLE))
	{
		bChanged |= ImGui::DragFloat("Model Import Scale",
			&Element.Detail.Mesh.fModelPreScale, 0.001f, 0.0001f, 100.f,
			"%.4f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Applied once when the WModel is loaded. Valtan Effect WModels commonly use 0.01; particle size remains a separate authored or source-tuning axis.");
		}
	}
	if (Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE)
	{
		bChanged |= DragFloat2("Start Size",
			Element.Detail.Particle.vStartSize, 0.01f, 0.001f, 100.f);
		bChanged |= DragFloat2("End Size",
			Element.Detail.Particle.vEndSize, 0.01f, 0.f, 100.f);
	}
	else if (Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
	{
		bChanged |= DragFloat2("Decal Size", Element.Detail.Decal.vSize,
			0.01f, 0.001f, 1000.f);
		bChanged |= ImGui::DragFloat("Projection Depth",
			&Element.Detail.Decal.fDepth, 0.01f, 0.001f, 1000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
	}
	else if (Element.eKind == EFFECT_ELEMENT_KIND::TRAIL)
	{
		bChanged |= ImGui::DragFloat("Start Width",
			&Element.Detail.Trail.fStartWidth, 0.01f, 0.001f, 100.f,
			"%.3f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("End Width",
			&Element.Detail.Trail.fEndWidth, 0.01f, 0.f, 100.f,
			"%.3f", ImGuiSliderFlags_AlwaysClamp);
	}
}

void Client::CEffect_Tool::Render_AuthoringMaterialParameters(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	struct SCALAR_CONTROL final
	{
		std::string strName;
		f32_t fValue = 0.f;
		bool_t bConsistent = true;
	};
	struct VECTOR_CONTROL final
	{
		std::string strName;
		float4_t vValue{};
		bool_t bConsistent = true;
	};
	std::vector<SCALAR_CONTROL> ScalarControls;
	std::vector<VECTOR_CONTROL> VectorControls;
	std::unordered_map<std::string, size_t> ScalarCounts;
	std::unordered_map<std::string, size_t> VectorCounts;
	const auto AddScalar = [&ScalarControls, &ScalarCounts](
		const std::string& strName, const f32_t fValue)
	{
		if (strName.empty())
			return;
		++ScalarCounts[strName];
		const auto Existing = std::find_if(ScalarControls.begin(),
			ScalarControls.end(), [&strName](const SCALAR_CONTROL& Control)
			{
				return Control.strName == strName;
			});
		if (Existing == ScalarControls.end())
			ScalarControls.push_back({ strName, fValue, true });
		else if (Existing->fValue != fValue)
			Existing->bConsistent = false;
	};
	const auto AddVector = [&VectorControls, &VectorCounts](
		const std::string& strName, const float4_t& vValue)
	{
		if (strName.empty())
			return;
		++VectorCounts[strName];
		const auto Existing = std::find_if(VectorControls.begin(),
			VectorControls.end(), [&strName](const VECTOR_CONTROL& Control)
			{
				return Control.strName == strName;
			});
		if (Existing == VectorControls.end())
		{
			VectorControls.push_back({ strName, vValue, true });
		}
		else if (Existing->vValue.x != vValue.x ||
			Existing->vValue.y != vValue.y ||
			Existing->vValue.z != vValue.z ||
			Existing->vValue.w != vValue.w)
		{
			Existing->bConsistent = false;
		}
	};
	for (const EFFECT_NAMED_FLOAT_DESC& Scalar :
		Element.Material.SourceMaterial.Scalars)
	{
		AddScalar(Scalar.strName, Scalar.fValue);
	}
	for (const EFFECT_MATERIAL_SCALAR_PARAMETER_DESC& Scalar :
		Element.Material.Execution.Scalars)
	{
		AddScalar(Scalar.strName, Scalar.fValue);
	}
	for (const EFFECT_NAMED_FLOAT4_DESC& Vector :
		Element.Material.SourceMaterial.Vectors)
	{
		AddVector(Vector.strName, Vector.vValue);
	}
	const auto AddExecutionVectors = [&AddVector](
		const std::vector<EFFECT_MATERIAL_VECTOR_PARAMETER_DESC>& Values)
	{
		for (const EFFECT_MATERIAL_VECTOR_PARAMETER_DESC& Value : Values)
			AddVector(Value.strName, Value.vValue);
	};
	AddExecutionVectors(Element.Material.Execution.Vectors);
	AddExecutionVectors(Element.Material.Execution.ArtistParameters);
	AddExecutionVectors(Element.Material.Execution.Colors);
	std::erase_if(ScalarControls,
		[&VectorCounts](const SCALAR_CONTROL& Control)
		{
			return !Control.bConsistent ||
				0u != VectorCounts.count(Control.strName);
		});
	std::erase_if(VectorControls,
		[&ScalarCounts](const VECTOR_CONTROL& Control)
		{
			return !Control.bConsistent ||
				0u != ScalarCounts.count(Control.strName);
		});
	if (ScalarControls.empty() && VectorControls.empty())
		return;
	if (!ImGui::CollapsingHeader(
			"Material Parameters", ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}
	ImGui::TextDisabled(
		"Only compiler-declared parameters are editable. Overrides never change admission.");
	for (const SCALAR_CONTROL& Control : ScalarControls)
	{
		ImGui::PushID("scalar");
		ImGui::PushID(Control.strName.c_str());
		f32_t fValue = Control.fValue;
		if (ImGui::DragFloat(Control.strName.c_str(), &fValue, 0.001f,
			-100000.f, 100000.f, "%.6g"))
		{
			std::string strError;
			if (CEffectDocumentCodec::Set_AuthoringScalarOverride(
					Element, Control.strName, fValue, strError))
			{
				bChanged = true;
			}
			else
			{
				m_strDetailStatus = "Scalar override rejected: " + strError;
			}
		}
		const auto Override = std::find_if(
			Element.AuthoringOverrides.Scalars.begin(),
			Element.AuthoringOverrides.Scalars.end(),
			[&Control](const EFFECT_AUTHORING_SCALAR_OVERRIDE_DESC& Candidate)
			{ return Candidate.strName == Control.strName; });
		if (Override != Element.AuthoringOverrides.Scalars.end())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f), "Modified");
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset to Source"))
			{
				std::string strError;
				if (CEffectDocumentCodec::Reset_AuthoringScalarOverride(
						Element, Control.strName, strError))
				{
					bChanged = true;
				}
				else
				{
					m_strDetailStatus = "Scalar reset rejected: " + strError;
				}
			}
		}
		ImGui::PopID();
		ImGui::PopID();
	}
	for (const VECTOR_CONTROL& Control : VectorControls)
	{
		ImGui::PushID("vector");
		ImGui::PushID(Control.strName.c_str());
		float4_t vValue = Control.vValue;
		if (DragFloat4(Control.strName.c_str(), vValue, 0.001f,
			-100000.f, 100000.f, "%.6g"))
		{
			std::string strError;
			if (CEffectDocumentCodec::Set_AuthoringColorOverride(
					Element, Control.strName, vValue, strError))
			{
				bChanged = true;
			}
			else
			{
				m_strDetailStatus = "Vector override rejected: " + strError;
			}
		}
		const auto Override = std::find_if(
			Element.AuthoringOverrides.Colors.begin(),
			Element.AuthoringOverrides.Colors.end(),
			[&Control](const EFFECT_AUTHORING_COLOR_OVERRIDE_DESC& Candidate)
			{ return Candidate.strName == Control.strName; });
		if (Override != Element.AuthoringOverrides.Colors.end())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f), "Modified");
			ImGui::SameLine();
			if (ImGui::SmallButton("Reset to Source"))
			{
				std::string strError;
				if (CEffectDocumentCodec::Reset_AuthoringColorOverride(
						Element, Control.strName, strError))
				{
					bChanged = true;
				}
				else
				{
					m_strDetailStatus = "Vector reset rejected: " + strError;
				}
			}
		}
		ImGui::PopID();
		ImGui::PopID();
	}
}

void Client::CEffect_Tool::Render_KindDetail(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Type Detail",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    EFFECT_DETAIL_DESC& Detail = Element.Detail;
    switch (Element.eKind)
    {
	case EFFECT_ELEMENT_KIND::MESH:
		bChanged |= ImGui::Checkbox("Use Model Material",
			&Detail.Mesh.bUseModelMaterial);
		ImGui::TextDisabled("Model Import Scale is edited once in Size above.");
        break;
    case EFFECT_ELEMENT_KIND::SPRITE:
        bChanged |= ImGui::Checkbox("Billboard",
            &Detail.Sprite.bBillboard);
		bChanged |= ImGui::DragFloat("Billboard Roll Degrees",
			&Detail.Sprite.fBillboardRollDegrees, 1.f, -3600.f, 3600.f,
			"%.1f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Billboard Roll Degrees Per Second",
			&Detail.Sprite.fBillboardRollDegreesPerSecond, 1.f, -3600.f,
			3600.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        break;
	case EFFECT_ELEMENT_KIND::DECAL:
		ImGui::TextDisabled(Is_SourceParticleCarrier(Element) ?
			"Source Size and Projection Depth are edited once in Source Playback Tuning above." :
			"Decal Size and Projection Depth are edited once in Size above.");
		break;
	case EFFECT_ELEMENT_KIND::PARTICLE:
	{
		const bool_t bSourcePlayback = Element.SourceRecipe.bEnabled;
		const bool_t bCompilerOwnedSourceParticle =
			m_bDetailDraftPortableRecipeReadOnly &&
			bSourcePlayback;
		const bool_t bMeshParticle = Resolve_AuthoringFamily(Element) ==
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE;
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
				"Runtime SourceRecipe owns spawn rate/bursts, particle lifetime, initial position/velocity/acceleration, size, and DynamicParameter values. Those Detail controls are read-only because changing them would not affect playback.");
			ImGui::TextDisabled(
				"Working overlays: Max Particles, Random Seed, Local Space, Target Attractor, sprite-family Billboard/Roll, Element Transform, Start Delay, Color/material, and supported resources.");
		}
		if (bMeshParticle)
		{
			ImGui::SeparatorText("Mesh Carrier");
			bChanged |= ImGui::Checkbox("Use Model Material",
				&Detail.Mesh.bUseModelMaterial);
			ImGui::TextDisabled(
				"Model Import Scale is edited once in Size or Source Playback Tuning above.");
		}
		ImGui::SeparatorText("Particle Runtime Overlays");
		if (ImGui::InputScalar("Max Particles", ImGuiDataType_U32,
			&Detail.Particle.iMaxParticles))
		{
			const uint32_t iMinimum = (std::min)(2048u,
				(std::max)(1u, Detail.Particle.iBurstCount));
			Detail.Particle.iMaxParticles = std::clamp(
				Detail.Particle.iMaxParticles, iMinimum, 2048u);
			bChanged = true;
		}
		if (ImGui::InputScalar("Random Seed", ImGuiDataType_U32,
			&Detail.Particle.iRandomSeed))
		{
			Detail.Particle.iRandomSeed = (std::max)(
				1u, Detail.Particle.iRandomSeed);
			bChanged = true;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Seeds the emitter stream. Source modules with their own explicit seed keep that module-local seed.");
		}
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextDisabled(
				"The compiler-owned SourceRecipe supplies spawn, lifetime, motion, size, and Dynamic Parameters. Tune its evaluated result in Source Playback Tuning above; Max Particles and Random Seed remain working overlays.");
		}
		else
		{
			ImGui::SeparatorText("Particle Spawn");
			bChanged |= ImGui::DragFloat("Spawn Rate / Second",
				&Detail.Particle.fSpawnRatePerSecond, 1.f, 0.f, 2048.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::InputScalar("Fixed Burst at Element Start",
				ImGuiDataType_U32, &Detail.Particle.iBurstCount);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Emits this many particles once at the Element-local start.");
			}
			ImGui::TextDisabled(
				"Start Delay controls the skill time; Spawn Rate and this fixed burst can be used together.");

			ImGui::SeparatorText("Particle Lifetime and Shape");
			if (DragFloat2("Particle Life Min/Max",
			Detail.Particle.vLifeTimeSeconds, 0.01f, 0.001f, 30.f))
		{
            Detail.Particle.vLifeTimeSeconds.y = (std::max)(
                Detail.Particle.vLifeTimeSeconds.x,
                Detail.Particle.vLifeTimeSeconds.y);
			bChanged = true;
		}
		const bool_t bPositionMinChanged = DragFloat3(
			"Initial Position Min", Detail.Particle.vInitialPositionMin,
			0.01f, -1000.f, 1000.f);
		const bool_t bPositionMaxChanged = DragFloat3(
			"Initial Position Max", Detail.Particle.vInitialPositionMax,
			0.01f, -1000.f, 1000.f);
		if (bPositionMinChanged || bPositionMaxChanged)
		{
			Detail.Particle.vInitialPositionMax.x = (std::max)(
				Detail.Particle.vInitialPositionMin.x,
				Detail.Particle.vInitialPositionMax.x);
			Detail.Particle.vInitialPositionMax.y = (std::max)(
				Detail.Particle.vInitialPositionMin.y,
				Detail.Particle.vInitialPositionMax.y);
			Detail.Particle.vInitialPositionMax.z = (std::max)(
				Detail.Particle.vInitialPositionMin.z,
				Detail.Particle.vInitialPositionMax.z);
			bChanged = true;
		}
		const bool_t bVelocityMinChanged = DragFloat3("Initial Velocity Min",
            Detail.Particle.vInitialVelocityMin, 0.01f, -1000.f, 1000.f);
        const bool_t bVelocityMaxChanged = DragFloat3("Initial Velocity Max",
            Detail.Particle.vInitialVelocityMax, 0.01f, -1000.f, 1000.f);
        if (bVelocityMinChanged || bVelocityMaxChanged)
        {
            Detail.Particle.vInitialVelocityMax.x = (std::max)(
                Detail.Particle.vInitialVelocityMin.x,
                Detail.Particle.vInitialVelocityMax.x);
            Detail.Particle.vInitialVelocityMax.y = (std::max)(
                Detail.Particle.vInitialVelocityMin.y,
                Detail.Particle.vInitialVelocityMax.y);
            Detail.Particle.vInitialVelocityMax.z = (std::max)(
                Detail.Particle.vInitialVelocityMin.z,
                Detail.Particle.vInitialVelocityMax.z);
            bChanged = true;
        }
		bChanged |= DragFloat3("Acceleration",
			Detail.Particle.vAcceleration, 0.01f, -1000.f, 1000.f);

		/* Spawn volume and emission direction: the two axes the authored Detail
		   could not express at all, so a ring that collapses inward or a mesh
		   that flies along an arc had to stay owned by the source modules. */
		ImGui::SeparatorText("Particle Spawn Volume");
		EFFECT_PARTICLE_SPAWN_SHAPE_DESC& Shape = Detail.Particle.SpawnShape;
		static const char* const s_SpawnShapeLabels[] =
		{
			"Point (Initial Position box)", "Sphere", "Ring (XZ)", "Box"
		};
		int32_t iSpawnShape = static_cast<int32_t>(Shape.eKind);
		if (ImGui::Combo("Spawn Shape", &iSpawnShape, s_SpawnShapeLabels,
			IM_ARRAYSIZE(s_SpawnShapeLabels)))
		{
			Shape.eKind = static_cast<EFFECT_PARTICLE_SPAWN_SHAPE>(iSpawnShape);
			if (EFFECT_PARTICLE_SPAWN_SHAPE::SPHERE == Shape.eKind ||
				EFFECT_PARTICLE_SPAWN_SHAPE::RING == Shape.eKind)
			{
				Shape.fRadius = (std::max)(0.001f, Shape.fRadius);
			}
			if (EFFECT_PARTICLE_SPAWN_SHAPE::BOX == Shape.eKind &&
				Shape.vExtents.x <= 0.f && Shape.vExtents.y <= 0.f &&
				Shape.vExtents.z <= 0.f)
			{
				Shape.vExtents = { 0.5f, 0.5f, 0.5f };
			}
			bChanged = true;
		}
		if (EFFECT_PARTICLE_SPAWN_SHAPE::POINT != Shape.eKind)
		{
			if (EFFECT_PARTICLE_SPAWN_SHAPE::BOX == Shape.eKind)
			{
				bChanged |= DragFloat3("Spawn Box Half Extents",
					Shape.vExtents, 0.01f, 0.f, 1000.f);
			}
			else
			{
				const bool_t bRadiusChanged = ImGui::DragFloat("Spawn Radius",
					&Shape.fRadius, 0.01f, 0.001f, 1000.f, "%.3f",
					ImGuiSliderFlags_AlwaysClamp);
				const bool_t bInnerChanged = ImGui::DragFloat(
					"Spawn Inner Radius", &Shape.fInnerRadius, 0.01f, 0.f,
					1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				if (bRadiusChanged || bInnerChanged)
				{
					Shape.fInnerRadius = (std::min)(
						Shape.fInnerRadius, Shape.fRadius);
					bChanged = true;
				}
				bChanged |= ImGui::DragFloat("Spawn Arc Degrees",
					&Shape.fArcDegrees, 1.f, 0.001f, 360.f, "%.1f",
					ImGuiSliderFlags_AlwaysClamp);
			}
			ImGui::TextDisabled(
				"The shape offset is added on top of the Initial Position box.");
		}

		ImGui::SeparatorText("Particle Emission Direction");
		EFFECT_PARTICLE_INITIAL_VELOCITY_DESC& Emission =
			Detail.Particle.InitialVelocity;
		static const char* const s_VelocityModeLabels[] =
		{
			"Fixed (Initial Velocity box)", "Outward", "Inward", "Cone (+Y)"
		};
		int32_t iVelocityMode = static_cast<int32_t>(Emission.eMode);
		if (ImGui::Combo("Emission Mode", &iVelocityMode, s_VelocityModeLabels,
			IM_ARRAYSIZE(s_VelocityModeLabels)))
		{
			Emission.eMode =
				static_cast<EFFECT_PARTICLE_VELOCITY_MODE>(iVelocityMode);
			bChanged = true;
		}
		if (EFFECT_PARTICLE_VELOCITY_MODE::FIXED != Emission.eMode)
		{
			if (DragFloat2("Emission Speed Min/Max", Emission.vSpeedRange,
				0.01f, -1000.f, 1000.f))
			{
				Emission.vSpeedRange.y = (std::max)(
					Emission.vSpeedRange.x, Emission.vSpeedRange.y);
				bChanged = true;
			}
			if (EFFECT_PARTICLE_VELOCITY_MODE::CONE == Emission.eMode)
			{
				bChanged |= ImGui::DragFloat("Cone Half Angle Degrees",
					&Emission.fConeAngleDegrees, 1.f, 0.f, 180.f, "%.1f",
					ImGuiSliderFlags_AlwaysClamp);
			}
			ImGui::TextDisabled(
				"Outward and Inward are radial about the Element origin and replace the Initial Velocity box.");
		}
		}
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextDisabled(
				"Source-owned raw fields are hidden because editing them would not change this portable runtime recipe.");
		}

		ImGui::SeparatorText("Particle Target Attractor");
		EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC& Attractor =
			Detail.Particle.TargetAttractor;
		bool_t bAttractorEnabled = Attractor.bEnabled;
		if (ImGui::Checkbox("Enable Target Attractor", &bAttractorEnabled))
		{
			if (bAttractorEnabled)
			{
				Attractor.bEnabled = true;
				if (Attractor.fRadialAcceleration == 0.f)
					Attractor.fRadialAcceleration = 8.f;
			}
			else
			{
				Attractor = EFFECT_PARTICLE_TARGET_ATTRACTOR_DESC{};
			}
			bChanged = true;
		}
		if (Attractor.bEnabled)
		{
			static const char* const s_AttractorTargetSpaceLabels[] =
			{
				"Effect Root Local", "Element Local"
			};
			int32_t iTargetSpace = static_cast<int32_t>(
				Attractor.eTargetSpace);
			if (ImGui::Combo("Target Space", &iTargetSpace,
				s_AttractorTargetSpaceLabels,
				IM_ARRAYSIZE(s_AttractorTargetSpaceLabels)))
			{
				Attractor.eTargetSpace =
					static_cast<EFFECT_PARTICLE_ATTRACTOR_TARGET_SPACE>(
						iTargetSpace);
				bChanged = true;
			}
			bChanged |= DragFloat3("Target Offset",
				Attractor.vTargetOffset, 0.01f, -1000.f, 1000.f);
			if (DragFloat2("Active Normalized Min/Max",
				Attractor.vActiveNormalized, 0.01f, 0.f, 1.f))
			{
				Attractor.vActiveNormalized.x = std::clamp(
					Attractor.vActiveNormalized.x, 0.f, 0.999f);
				Attractor.vActiveNormalized.y = std::clamp(
					Attractor.vActiveNormalized.y,
					Attractor.vActiveNormalized.x + 0.001f, 1.f);
				bChanged = true;
			}
			bChanged |= ImGui::DragFloat("Radial Acceleration",
				&Attractor.fRadialAcceleration, 0.1f, 0.f, 10000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Tangential Acceleration",
				&Attractor.fTangentialAcceleration, 0.1f, -10000.f,
				10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Maximum Speed",
				&Attractor.fMaximumSpeed, 0.1f, 0.001f, 1000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Convergence Radius",
				&Attractor.fConvergenceRadius, 0.01f, 0.001f, 1000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			bChanged |= ImGui::DragFloat("Arrival Damping",
				&Attractor.fArrivalDamping, 0.1f, 0.f, 1000.f,
				"%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextDisabled(
				"Authored PROJECT_TUNED motion layer. SourceRecipe modules run first; this layer then steers the effective velocity toward the selected centre.");
		}
        bChanged |= ImGui::Checkbox("Particle Local Space",
            &Detail.Particle.bLocalSpace);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Local Space keeps spawned particles relative to this Element; "
				"disabled particles continue in world/root space.");
		}
		if (!bMeshParticle)
		{
			bChanged |= ImGui::Checkbox("Particle Billboard",
				&Detail.Particle.bBillboard);
			if (Detail.Particle.bBillboard)
			{
				/* The renderer rebuilds a billboarded quad from the camera every
				   frame, so the Transform rotation above never reaches it. */
				bChanged |= ImGui::DragFloat("Billboard Roll Degrees##particle",
					&Detail.Sprite.fBillboardRollDegrees, 1.f, -3600.f, 3600.f,
					"%.1f", ImGuiSliderFlags_AlwaysClamp);
				bChanged |= ImGui::DragFloat(
					"Billboard Roll Degrees Per Second##particle",
					&Detail.Sprite.fBillboardRollDegreesPerSecond, 1.f, -3600.f,
					3600.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextDisabled(
					"Billboard faces the camera, so Transform rotation does not apply. Source Playback Tuning > Rotation x scales source rotation modules on top of this roll.");
			}
		}
		else
		{
			ImGui::TextDisabled(
				"Mesh Particles use their model orientation; Billboard and sprite Roll are not consumed.");
		}
		if (bCompilerOwnedSourceParticle)
		{
			ImGui::TextDisabled(
				"SourceRecipe ParameterDynamic modules own runtime material parameters; ignored Detail fallbacks are hidden.");
		}
		else
		{
			ImGui::SeparatorText("Particle Dynamic Material Parameters");
			ImGui::TextDisabled(
				"Enabled components interpolate Start -> End over particle life.");
			bChanged |= ImGui::CheckboxFlags(
				"X##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 0u);
			ImGui::SameLine();
			bChanged |= ImGui::CheckboxFlags(
				"Y##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 1u);
			ImGui::SameLine();
			bChanged |= ImGui::CheckboxFlags(
				"Z##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 2u);
			ImGui::SameLine();
			bChanged |= ImGui::CheckboxFlags(
				"W##particle.dynamic", &Detail.Particle.iDynamicParameterComponentMask,
				1u << 3u);
			bChanged |= DragFloat4("Dynamic Start",
				Detail.Particle.vDynamicParameterStart,
				0.01f, -1000.f, 1000.f);
			bChanged |= DragFloat4("Dynamic End",
				Detail.Particle.vDynamicParameterEnd,
				0.01f, -1000.f, 1000.f);
			const uint32_t iTrackAConsumedMask =
				Element.Material.Execution.iDynamicConsumedMask & 0x0fu;
			if (Element.Material.Execution.bEnabled &&
				0u != iTrackAConsumedMask)
			{
				ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
					"Track A consumed mask 0x%X: Dynamic Start/End is the editable bounded source carrier.",
					iTrackAConsumedMask);
			}
		}
        break;
    }
	case EFFECT_ELEMENT_KIND::TRAIL:
	{
        bChanged |= ImGui::InputScalar("Trail Max Points",
            ImGuiDataType_U32, &Detail.Trail.iMaxPoints);
        bChanged |= ImGui::DragFloat("Trail Point Life",
            &Detail.Trail.fPointLifeTimeSeconds,
            0.01f, 0.001f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail Sample Interval",
            &Detail.Trail.fSampleIntervalSeconds,
            0.001f, 0.001f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail Minimum Distance",
            &Detail.Trail.fMinimumDistance,
            0.001f, 0.f, 100.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
		const bool_t bSourceOwnedTrailGeometry =
			Element.SourceRecipe.bEnabled || Element.Material.Execution.bEnabled;
		ImGui::BeginDisabled(bSourceOwnedTrailGeometry);
		bChanged |= ImGui::DragFloat("Trail UV Repeat Distance",
			&Detail.Trail.fTilingDistanceWorldUnits,
			0.01f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Trail Curve Step",
			&Detail.Trail.fDistanceTessellationStepWorldUnits,
			0.001f, 0.f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::EndDisabled();
		if (bSourceOwnedTrailGeometry)
		{
			ImGui::TextDisabled(
				"Typed source ribbons own fixed UV/tessellation values; manual Trail controls are read-only.");
		}
		ImGui::TextDisabled(
			"UV Repeat Distance > 0 maps U by traveled distance; 0 keeps legacy one-texture-per-segment UVs.");
		ImGui::TextDisabled(
			"Curve Step > 0 subdivides long segments when UV Repeat Distance is also > 0.");
		ImGui::TextDisabled("Trail Start/End Width are edited once in Size above.");
        bChanged |= ImGui::Checkbox("Trail Faces Camera",
            &Detail.Trail.bFaceCamera);
        break;
	}
    case EFFECT_ELEMENT_KIND::LIGHT:
	{
		EFFECT_LIGHT_DETAIL_DESC& Light = Detail.Light;
		ImGui::SeparatorText("Presentation Light");
		ImGui::TextDisabled("Profile: Point Light (Reconstructed v1)");
		if (!Light.bEnabled)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
				"This source row has no admitted typed Light payload. Delete or hide it here; enabling it requires source-backed materialization.");
		}
		bool_t bPresentationChanged = false;
		ImGui::BeginDisabled(!Light.bEnabled);
		bPresentationChanged |= ImGui::DragFloat("Light Range",
			&Light.fRange, 0.01f, 0.001f, 100000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::DragFloat("Light Intensity",
			&Light.fIntensity, 0.01f, 0.f, 100000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::ColorEdit4("Light Color",
			&Light.vColor.x, ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_HDR);
		bPresentationChanged |= ImGui::ColorEdit4("Ambient Color",
			&Light.vAmbient.x, ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_HDR);
		bPresentationChanged |= ImGui::DragFloat("Falloff Exponent",
			&Light.fFalloffExponent, 0.01f, 0.001f, 128.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		ImGui::EndDisabled();
		if (bPresentationChanged)
		{
			Light.eProfile = EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1;
			Light.eStatus =
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
			bChanged = true;
		}
		ImGui::TextDisabled(
			"Visible, Transform, Timing, Color Multiply, Solo and Delete use the same active Element transaction.");
        break;
	}
    case EFFECT_ELEMENT_KIND::SCREEN_POST:
	{
		EFFECT_SCREEN_POST_DETAIL_DESC& Post = Detail.ScreenPost;
		ImGui::SeparatorText("Presentation Screen Post");
		if (!Post.bEnabled)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
				"This source row has no admitted typed Screen Post payload. Delete or hide it here; enabling it requires source-backed materialization.");
		}
		bool_t bPresentationChanged = false;
		ImGui::BeginDisabled(!Post.bEnabled);
		if (ImGui::BeginCombo("Screen Post Profile",
			ScreenPostProfile_Label(Post.eProfile)))
		{
			for (uint8_t iProfile = 0u;
				iProfile < static_cast<uint8_t>(EFFECT_SCREEN_POST_PROFILE::END);
				++iProfile)
			{
				const EFFECT_SCREEN_POST_PROFILE eCandidate =
					static_cast<EFFECT_SCREEN_POST_PROFILE>(iProfile);
				const bool_t bSelected = eCandidate == Post.eProfile;
				if (ImGui::Selectable(ScreenPostProfile_Label(eCandidate),
					bSelected))
				{
					Post.eProfile = eCandidate;
					bPresentationChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		bPresentationChanged |= ImGui::DragFloat("Post Intensity",
			&Post.fIntensity, 0.001f, 0.f, 100.f, "%.4f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::DragFloat("Post Secondary Intensity",
			&Post.fSecondaryIntensity, 0.001f, 0.f, 100.f, "%.4f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::DragFloat("Post Frequency",
			&Post.fFrequency, 0.01f, 0.f, 1000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		bPresentationChanged |= ImGui::ColorEdit4("Post Tint",
			&Post.vTint.x, ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_HDR);
		if (ImGui::InputScalar("Post Random Seed", ImGuiDataType_U32,
			&Post.iRandomSeed))
		{
			Post.iRandomSeed = (std::max)(1u, Post.iRandomSeed);
			bPresentationChanged = true;
		}
		ImGui::EndDisabled();
		if (bPresentationChanged)
		{
			Post.eStatus =
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
			bChanged = true;
		}
		ImGui::TextDisabled(
			"The global Preview ScreenPost switch remains a non-persistent A/B gate; these fields persist only after Apply and Save.");
        break;
	}
    case EFFECT_ELEMENT_KIND::END:
    default:
        break;
    }
    if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
        Element.eKind == EFFECT_ELEMENT_KIND::SPRITE)
    {
        ImGui::SeparatorText("After Image");
        bChanged |= ImGui::DragFloat("AfterImage Sample Interval",
            &Detail.AfterImage.fSampleIntervalSeconds,
            0.001f, 0.001f, 30.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::InputScalar("AfterImage Max Copies",
            ImGuiDataType_U32, &Detail.AfterImage.iMaxCopies);
        bChanged |= ImGui::DragFloat("AfterImage Alpha Exponent",
            &Detail.AfterImage.fAlphaExponent,
            0.01f, 0.001f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    }
}

void Client::CEffect_Tool::Render_SourceRecipeDetail(
    EFFECT_CASCADE_RECIPE_DESC& Recipe,
    bool_t& bChanged,
	const bool_t bPortableReadOnly)
{
    if (!ImGui::CollapsingHeader("Original Emitter / Module Stack",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

	ImGui::TextDisabled(bPortableReadOnly ?
		"Compiler-owned portable recipe. Runtime-owned Particle Detail controls and this module stack are read-only; authored overlays and supported resource overrides remain editable outside this section." :
        "Imported UE3 values. Editing changes only the Authored document; the Imported baseline remains unchanged.");
    ImGui::BeginDisabled(bPortableReadOnly);
    bChanged |= ImGui::Checkbox("Execute Source Recipe", &Recipe.bEnabled);
    ImGui::Text("Renderer: %s",
        Recipe.strRendererShape.empty() ? "(unspecified)" :
            Recipe.strRendererShape.c_str());
    bChanged |= ImGui::DragFloat("Emitter Delay",
        &Recipe.fEmitterDelaySeconds, 0.001f, 0.f, 3600.f, "%.6f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("Emitter Duration",
        &Recipe.fEmitterDurationSeconds, 0.001f, 0.f, 3600.f, "%.6f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::InputScalar("Emitter Loop Count (0 = infinite)",
        ImGuiDataType_U32, &Recipe.iEmitterLoopCount);

    if (ImGui::TreeNodeEx("Bursts", ImGuiTreeNodeFlags_DefaultOpen,
        "Bursts (%zu)", Recipe.Bursts.size()))
    {
        for (size_t iBurst = 0u; iBurst < Recipe.Bursts.size(); ++iBurst)
        {
            EFFECT_PARTICLE_BURST_DESC& Burst = Recipe.Bursts[iBurst];
            ImGui::PushID(static_cast<int>(iBurst));
            ImGui::SeparatorText(("Burst " + std::to_string(iBurst)).c_str());
            bChanged |= ImGui::DragFloat("Time",
                &Burst.fTimeSeconds, 0.001f, 0.f, 3600.f, "%.6f",
                ImGuiSliderFlags_AlwaysClamp);
            bChanged |= ImGui::InputScalar(
                "Count Minimum", ImGuiDataType_U32, &Burst.iCountMinimum);
            bChanged |= ImGui::InputScalar(
                "Count Maximum", ImGuiDataType_U32, &Burst.iCountMaximum);
            Burst.iCountMaximum = (std::max)(
                Burst.iCountMinimum, Burst.iCountMaximum);
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    ImGui::SeparatorText("Dynamic Module Editors");
    ImGui::TextDisabled(
        "%zu modules; duplicate classes execute in source order.",
        Recipe.Modules.size());
    for (EFFECT_SOURCE_MODULE_DESC& Module : Recipe.Modules)
        Render_SourceModuleDetail(Module, bChanged);
    ImGui::EndDisabled();
}

void Client::CEffect_Tool::Render_SourceModuleDetail(
    EFFECT_SOURCE_MODULE_DESC& Module,
	bool_t& bChanged,
	const bool_t bDefaultOpen)
{
    ImGui::PushID(Module.strStableId.c_str());
	const SOURCE_MODULE_UI_DESC ModuleUI =
		Describe_SourceModule(Module.strClassName);
    const std::string strLabel = std::string(ModuleUI.pRole) + "##module";
	const ImGuiTreeNodeFlags Flags = bDefaultOpen ?
		ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None;
	if (ImGui::TreeNodeEx(strLabel.c_str(), Flags,
		"%s | %s | %zu values | %zu distributions",
		ModuleUI.pRole, Module.strClassName.c_str(), Module.Literals.size(),
        Module.Distributions.size()))
    {
		ImGui::TextWrapped("%s", ModuleUI.pDescription);
        ImGui::TextDisabled("Stable ID: %s", Module.strStableId.c_str());
		ImGui::TextDisabled("Source class: %s", Module.strClassName.c_str());
        ImGui::TextWrapped("Source: %s", Module.strObjectPath.c_str());

        if (!Module.Literals.empty() &&
            ImGui::TreeNodeEx("Literal Values", ImGuiTreeNodeFlags_DefaultOpen,
                "Literal Values (%zu)", Module.Literals.size()))
        {
            for (EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
            {
                ImGui::PushID(Literal.strPropertyPath.c_str());
				const std::string FriendlyLabel =
					Friendly_SourcePropertyLabel(Literal.strPropertyPath);
                switch (Literal.eKind)
                {
                case EFFECT_SOURCE_LITERAL_KIND::BOOLEAN:
                    bChanged |= ImGui::Checkbox(
						FriendlyLabel.c_str(), &Literal.bBoolean);
                    break;
                case EFFECT_SOURCE_LITERAL_KIND::NUMBER:
                {
                    const double fStep = 0.001;
                    bChanged |= ImGui::DragScalar(
						FriendlyLabel.c_str(), ImGuiDataType_Double,
                        &Literal.fNumber, 0.01f, nullptr, nullptr, "%.9g");
                    (void)fStep;
                    break;
                }
                case EFFECT_SOURCE_LITERAL_KIND::STRING:
                    ImGui::TextWrapped("%s = %s",
						FriendlyLabel.c_str(),
                        Literal.strString.c_str());
                    break;
                case EFFECT_SOURCE_LITERAL_KIND::END:
                default:
                    ImGui::TextDisabled("%s = (invalid)",
                        Literal.strPropertyPath.c_str());
                    break;
                }
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Source property: %s",
						Literal.strPropertyPath.c_str());
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

		for (EFFECT_DISTRIBUTION_DESC& Distribution : Module.Distributions)
			Render_SourceDistributionDetail(
				Distribution, Module.strClassName, bChanged);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Client::CEffect_Tool::Render_SourceDistributionDetail(
    EFFECT_DISTRIBUTION_DESC& Distribution,
	const std::string_view strModuleClassName,
    bool_t& bChanged)
{
    ImGui::PushID(Distribution.strPropertyPath.c_str());
	const SOURCE_MODULE_UI_DESC ModuleUI =
		Describe_SourceModule(strModuleClassName);
	const std::string FriendlyLabel =
		Friendly_SourcePropertyLabel(Distribution.strPropertyPath);
	if (ImGui::TreeNodeEx("##distribution", ImGuiTreeNodeFlags_None,
		"%s / %s | %uD | keys %zu | table %zu",
		ModuleUI.pRole, FriendlyLabel.c_str(),
        Distribution.iComponentCount, Distribution.Keys.size(),
        Distribution.LookupTable.size()))
    {
		ImGui::TextDisabled("Source property: %s",
			Distribution.strPropertyPath.c_str());
        ImGui::TextDisabled("Distribution: %s",
            Distribution.strSourceClass.empty() ? "inline cooked table" :
                Distribution.strSourceClass.c_str());
        if (!Distribution.strSourceObjectPath.empty())
            ImGui::TextWrapped("Source: %s",
                Distribution.strSourceObjectPath.c_str());
        bChanged |= ImGui::InputScalar(
            "Operation", ImGuiDataType_U32, &Distribution.iOperation);
        Distribution.iOperation = (std::min)(3u, Distribution.iOperation);
        bChanged |= ImGui::DragFloat("Lookup Time Scale",
            &Distribution.fLookupTableTimeScale, 0.001f,
            -100000.f, 100000.f, "%.9g");
        bChanged |= ImGui::DragFloat("Lookup Start Time",
            &Distribution.fLookupTableStartTime, 0.001f,
            -100000.f, 100000.f, "%.9g");
        bChanged |= DragFloat4("Default Minimum",
            Distribution.vDefaultMinimum, 0.001f, -100000.f, 100000.f,
            "%.6f");
        bChanged |= DragFloat4("Default Maximum",
            Distribution.vDefaultMaximum, 0.001f, -100000.f, 100000.f,
            "%.6f");

        if (!Distribution.Keys.empty() &&
            ImGui::TreeNodeEx("Curve Keys", ImGuiTreeNodeFlags_None,
                "Curve Keys (%zu)", Distribution.Keys.size()))
        {
            for (size_t iKey = 0u; iKey < Distribution.Keys.size(); ++iKey)
            {
                EFFECT_DISTRIBUTION_KEY_DESC& Key = Distribution.Keys[iKey];
                ImGui::PushID(static_cast<int>(iKey));
                ImGui::SeparatorText(("Key " + std::to_string(iKey)).c_str());
                bChanged |= ImGui::DragFloat("Time", &Key.fTime,
                    0.001f, -100000.f, 100000.f, "%.9g");
                bChanged |= DragFloat4("Minimum", Key.vMinimum,
                    0.001f, -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Maximum", Key.vMaximum,
                    0.001f, -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Arrive Tangent Minimum",
                    Key.vArriveTangentMinimum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Leave Tangent Minimum",
                    Key.vLeaveTangentMinimum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Arrive Tangent Maximum",
                    Key.vArriveTangentMaximum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                bChanged |= DragFloat4("Leave Tangent Maximum",
                    Key.vLeaveTangentMaximum, 0.001f,
                    -100000.f, 100000.f, "%.6f");
                int32_t iInterpolation = static_cast<int32_t>(
                    Key.eInterpolation);
                constexpr const char* INTERPOLATION_LABELS[] =
                {
                    "Constant", "Linear", "Cubic"
                };
                if (ImGui::Combo("Interpolation", &iInterpolation,
                    INTERPOLATION_LABELS,
                    static_cast<int>(std::size(INTERPOLATION_LABELS))))
                {
                    Key.eInterpolation =
                        static_cast<EFFECT_DISTRIBUTION_INTERPOLATION>(
                            iInterpolation);
                    bChanged = true;
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (!Distribution.LookupTable.empty() &&
            ImGui::TreeNodeEx("Cooked Lookup Table", ImGuiTreeNodeFlags_None,
                "Cooked Lookup Table (%zu)", Distribution.LookupTable.size()))
        {
            ImGui::TextDisabled(
                "The runtime evaluates this table before source curve keys.");
            ImGuiListClipper Clipper;
            Clipper.Begin(static_cast<int>(Distribution.LookupTable.size()));
            while (Clipper.Step())
            {
                for (int32_t iValue = Clipper.DisplayStart;
                    iValue < Clipper.DisplayEnd; ++iValue)
                {
                    ImGui::PushID(iValue);
                    bChanged |= ImGui::DragFloat(
                        std::to_string(iValue).c_str(),
                        &Distribution.LookupTable[
                            static_cast<size_t>(iValue)],
                        0.001f, -100000.f, 100000.f, "%.9g");
                    ImGui::PopID();
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void Client::CEffect_Tool::Render_LerpDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Linear Lerp",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    EFFECT_LINEAR_LERP_DESC& Lerp = Detail.LinearLerp;
    ImGui::TextDisabled(
        "Enable a Lerp checkbox to interpolate its Start value to End over Lifetime. Enabling restarts live preview.");
    const auto RenderLerpToggle = [this, &bChanged](
        const char* pLabel,
        bool_t& bEnabled)
    {
        const bool_t bWasEnabled = bEnabled;
        const bool_t bToggleChanged = ImGui::Checkbox(pLabel, &bEnabled);
        bChanged |= bToggleChanged;
        if (bToggleChanged && !bWasEnabled && bEnabled)
        {
            m_fPreviewTimeSeconds = 0.f;
            m_bPreviewPlaying = true;
        }
    };
    RenderLerpToggle("Lerp Position", Lerp.bPosition);
    if (Lerp.bPosition)
        bChanged |= DragFloat3(
            "Position End", Lerp.vEndPosition, 0.01f, -1000.f, 1000.f);
    RenderLerpToggle("Lerp Rotation", Lerp.bRotation);
    if (Lerp.bRotation)
        bChanged |= DragFloat3(
            "Rotation End", Lerp.vEndRotationDegrees, 0.25f, -360.f, 360.f);
    RenderLerpToggle("Lerp Revolution", Lerp.bRevolution);
    if (Lerp.bRevolution)
        bChanged |= DragFloat3("Revolution End",
            Lerp.vEndRevolutionDegreesPerSecond, 0.5f, -3600.f, 3600.f);
    RenderLerpToggle("Lerp Scaling", Lerp.bScale);
    if (Lerp.bScale)
        bChanged |= DragFloat3(
            "Scaling End", Lerp.vEndScale, 0.01f, 0.001f, 100.f);
    RenderLerpToggle("Lerp Velocity", Lerp.bVelocity);
    if (Lerp.bVelocity)
        bChanged |= DragFloat3("Velocity End",
            Lerp.vEndVelocityPerSecond, 0.01f, -1000.f, 1000.f);
    RenderLerpToggle("Lerp ColorOffset", Lerp.bColorOffset);
    if (Lerp.bColorOffset)
        bChanged |= DragFloat4(
            "ColorOffset End", Lerp.vEndColorOffset, 0.01f, -10.f, 10.f);
    RenderLerpToggle("Lerp Color Multiply", Lerp.bColorMultiply);
    if (Lerp.bColorMultiply)
        bChanged |= DragFloat4("Color Multiply End",
            Lerp.vEndColorMultiply, 0.01f, 0.f, 10.f);
	RenderLerpToggle("Lerp Emissive Intensity (HDR)",
		Lerp.bEmissiveIntensity);
	if (Lerp.bEmissiveIntensity)
		bChanged |= ImGui::DragFloat("Emissive Intensity End##lerp",
			&Lerp.fEndEmissiveIntensity, 0.05f, 0.f, 100.f, "%.3f",
            ImGuiSliderFlags_AlwaysClamp);
}

void Client::CEffect_Tool::Render_AssemblyHierarchy(
	const std::string& strEffectAssetId)
{
	const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
		CEffectCatalog::Find_Assembly(strEffectAssetId);
	if (nullptr == Assembly)
	{
		ImGui::TextDisabled("Runtime Assembly is not admitted.");
		return;
	}
	const std::shared_ptr<const EFFECT_DOCUMENT_DESC> RuntimeDocument =
		CEffectCatalog::Find(strEffectAssetId);
	const PARTICLE_LAYER_SUMMARY ParticleSummary = nullptr != RuntimeDocument ?
		Summarize_ParticleLayers(*RuntimeDocument) : PARTICLE_LAYER_SUMMARY{};
	const bool_t bParticleSystemSelected =
		m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == strEffectAssetId &&
		EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM == m_eDetailSelection;
	const std::string ParticleSystemLabel = "Cascade System | Emitters " +
		std::to_string(ParticleSummary.iSourceEmitterCount) +
		" | Mesh Particles " +
		std::to_string(ParticleSummary.iMeshRendererCount) +
		" | Sprite Particles " +
		std::to_string(ParticleSummary.iSpriteRendererCount) + " | Unresolved " +
		std::to_string(ParticleSummary.iUnresolvedRendererCount);
	const bool_t bParticleSystemOpen = ImGui::TreeNodeEx(
		(ParticleSystemLabel + "##particle-system." + strEffectAssetId).c_str(),
		ImGuiTreeNodeFlags_OpenOnArrow |
		(bParticleSystemSelected ? ImGuiTreeNodeFlags_Selected : 0));
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		Try_SelectParticleSystem(strEffectAssetId);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"Aggregate controls preserve all source Emitters. Open a Component or "
			"Emitter below for source-level editing.");
	if (bParticleSystemOpen)
	{
		ImGui::TextDisabled("Source Systems %zu | Budget %llu",
			ParticleSummary.iSourceSystemCount,
			static_cast<unsigned long long>(ParticleSummary.iParticleBudget));
		if (ParticleSummary.iSourceEmitterCount > 0u &&
			ImGui::SmallButton("Open First Emitter"))
		{
			Try_SelectFirstEmitter(strEffectAssetId, {});
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode(("Timeline (" +
		std::to_string(Assembly->ComponentCues.size()) + " Component Cues)##" +
		strEffectAssetId).c_str()))
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
			ImGui::BulletText("%.3f s | %s", Cue.fStartDelaySeconds,
				Cue.strComponentAssetId.c_str());
		if (!Assembly->ModelCues.empty() && ImGui::TreeNode(
			("Animated Model Cues (" +
				std::to_string(Assembly->ModelCues.size()) + ")").c_str()))
		{
			for (const EFFECT_MODEL_CUE_DESC& Cue : Assembly->ModelCues)
				ImGui::BulletText("%.3f s | %s | %s",
					Cue.fStartDelaySeconds, Cue.strClipName.c_str(),
					Cue.strModelAssetId.c_str());
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (!ImGui::TreeNode(("Components (" +
		std::to_string(Assembly->ComponentCues.size()) + ")##components." +
		strEffectAssetId).c_str()))
	{
		return;
	}
	for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(Cue.strComponentAssetId);
		if (nullptr == Component)
		{
			ImGui::BulletText("MISSING %s", Cue.strComponentAssetId.c_str());
			continue;
		}
		ImGui::PushID(Component->strComponentAssetId.c_str());
		const bool_t bSelected =
			m_strSelectedComponentId == Component->strComponentAssetId &&
			m_eDetailSelection >= EFFECT_DETAIL_SELECTION::COMPONENT;
		const std::string Label = Component->strDisplayName + " | " +
			Component->strComponentType + " | " +
			std::to_string(Component->Emitters.size()) + " Emitters";
		const bool_t bOpen = ImGui::TreeNodeEx(Label.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow |
			(bSelected ? ImGuiTreeNodeFlags_Selected : 0));
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			Try_SelectComponent(strEffectAssetId,
				Component->strComponentAssetId);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stable ID: %s\nSource Group: %s\nCue: %.3f s",
				Component->strComponentAssetId.c_str(),
				Component->strSourceGroupId.c_str(), Cue.fStartDelaySeconds);
		if (bOpen)
		{
			for (const EFFECT_COMPONENT_EMITTER_DESC& Emitter :
				Component->Emitters)
			{
				const auto ElementIterator = std::find_if(
					Component->Document.Elements.begin(),
					Component->Document.Elements.end(),
					[&Emitter](const EFFECT_ELEMENT_DESC& Element)
					{
						return Element.strElementId == Emitter.strElementId;
					});
				if (Component->Document.Elements.end() == ElementIterator)
					continue;
				const EFFECT_ELEMENT_DESC& Element = *ElementIterator;
				ImGui::PushID(Emitter.strEmitterId.c_str());
				const bool_t bEmitterSelected = bSelected &&
					m_strSelectedEmitterId == Emitter.strEmitterId &&
					m_eDetailSelection >= EFFECT_DETAIL_SELECTION::EMITTER;
				const std::string EmitterLabel = Element.strDisplayName + " | " +
					Element_RendererLabel(Element) + " | Runtime " +
					Emitter.strRendererType + " | " +
					std::to_string(Emitter.iModuleCount) + " Modules";
				const bool_t bEmitterOpen = ImGui::TreeNodeEx(
					EmitterLabel.c_str(), ImGuiTreeNodeFlags_OpenOnArrow |
					(bEmitterSelected ? ImGuiTreeNodeFlags_Selected : 0));
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
					Try_SelectEmitter(strEffectAssetId,
						Component->strComponentAssetId, Emitter.strEmitterId);
				if (bEmitterOpen)
				{
					ImGui::BulletText("Renderer: %s",
						Emitter.strRendererType.c_str());
					if (ImGui::TreeNode(("Resources (" +
						std::to_string(Element.ResourceBindings.size()) + ")").c_str()))
					{
						for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
							Element.ResourceBindings)
						{
							ImGui::BulletText("%s: %s", Binding.strSlotId.c_str(),
								Binding.strAssetId.c_str());
						}
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(("Modules (" +
						std::to_string(Element.SourceRecipe.Modules.size()) + ")").c_str()))
					{
						for (const EFFECT_SOURCE_MODULE_DESC& Module :
							Element.SourceRecipe.Modules)
						{
							const bool_t bModuleSelected = bEmitterSelected &&
								m_eDetailSelection ==
									EFFECT_DETAIL_SELECTION::SOURCE_MODULE &&
								m_strSelectedSourceModuleId == Module.strStableId;
							const std::string ModuleLabel = Module.strClassName + "##" +
								Module.strStableId;
							if (ImGui::Selectable(ModuleLabel.c_str(), bModuleSelected))
								Try_SelectSourceModule(strEffectAssetId,
									Component->strComponentAssetId,
									Emitter.strEmitterId, Module.strStableId);
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("%s\n%s", Module.strStableId.c_str(),
									Module.strObjectPath.c_str());
						}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TreePop();
}

void Client::CEffect_Tool::Render_VisualProgramAuthoring(
	const EFFECT_SKILL_TREE_ENTRY& Entry,
	const size_t iCueIndex)
{
	if (iCueIndex >= Entry.ProductCues.size())
		return;
	const std::string& strEffectAssetId =
		Entry.ProductCues[iCueIndex].Cue.strEffectAssetId;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == Program)
		return;
	const bool_t bArtistFAdapter =
		strEffectAssetId == ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		Program->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		AuthoringProjection = bArtistFAdapter ?
			(ARTIST_F_PREPARATION_STATE::READY ==
					m_eArtistFSourcePreparationState &&
			 m_iArtistFSourceSnapshotRevision ==
				CEffectCatalog::Get_RuntimeRevision() ?
				m_pArtistFSourceProjection : nullptr) :
			CEffectCatalog::Find_VisualProjection_Loaded(strEffectAssetId);
	const bool_t bAdapterPacketProgram = Program->eProjectionKind ==
		EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketProgram && nullptr == AuthoringProjection)
		ImGui::TextDisabled(
			"This source recipe is unavailable for migration.");
	const auto IsAuthorableVisualRow =
		[&AuthoringProjection](const EFFECT_VISUAL_PROGRAM_ROW& Row)
		{
			if (nullptr == AuthoringProjection ||
				Row.eDisposition !=
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				!Row.TargetIdentity.has_value())
			{
				return false;
			}
			const EFFECT_VISUAL_PROGRAM_ROW* pProjectedRow =
				AuthoringProjection->Find_RowByOccurrenceId(
					Row.Selector.strOccurrenceId);
			return nullptr != pProjectedRow &&
				pProjectedRow->eDisposition ==
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
				pProjectedRow->strRowSha256 == Row.strRowSha256 &&
				pProjectedRow->TargetIdentity.has_value() &&
				pProjectedRow->TargetIdentity->strTargetElementId ==
					Row.TargetIdentity->strTargetElementId &&
				pProjectedRow->SourceIdentity.strSourceRecordId ==
					Row.SourceIdentity.strSourceRecordId;
		};
	const auto IsAuthorableSupplementalRow =
		[&AuthoringProjection](
			const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row)
		{
			if (nullptr == AuthoringProjection ||
				Row.eDisposition !=
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED ||
				Row.TargetIdentity.strTargetElementId.empty())
			{
				return false;
			}
			const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pProjectedRow =
				AuthoringProjection->Find_SupplementalElementByOccurrenceId(
					Row.Selector.strOccurrenceId);
			return nullptr != pProjectedRow &&
				pProjectedRow->eDisposition ==
					EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
				pProjectedRow->strRowSha256 == Row.strRowSha256 &&
				pProjectedRow->TargetIdentity.strTargetElementId ==
					Row.TargetIdentity.strTargetElementId &&
				pProjectedRow->strSourceRecordId == Row.strSourceRecordId;
		};

	static constexpr std::array<EFFECT_VISUAL_PROGRAM_FAMILY, 7u> FAMILIES{
		EFFECT_VISUAL_PROGRAM_FAMILY::MESH_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::SPRITE_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::DECAL_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::CASCADE_RIBBON,
		EFFECT_VISUAL_PROGRAM_FAMILY::ANIMATION_TRAIL,
		EFFECT_VISUAL_PROGRAM_FAMILY::LIGHT_PARTICLE,
		EFFECT_VISUAL_PROGRAM_FAMILY::SCREEN_POST };
	if (!ImGui::TreeNodeEx("Track A Element Seeds",
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		return;
	}
	ImGui::TextDisabled(
		"Read-only source data. Load Seed copies WModel/DDS and safe starter values into Element Authoring; it never changes Current Effect by itself.");
	for (const EFFECT_VISUAL_PROGRAM_FAMILY eFamily : FAMILIES)
	{
		const size_t iRowCount = static_cast<size_t>(std::count_if(
			Program->VisualRows.begin(), Program->VisualRows.end(),
			[eFamily, &IsAuthorableVisualRow](
				const EFFECT_VISUAL_PROGRAM_ROW& Row)
			{ return Row.eFamily == eFamily && IsAuthorableVisualRow(Row); })) +
			static_cast<size_t>(std::count_if(
				Program->SupplementalElements.begin(),
				Program->SupplementalElements.end(),
				[eFamily, &IsAuthorableSupplementalRow](
					const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row)
				{ return Row.eFamily == eFamily &&
					IsAuthorableSupplementalRow(Row); }));
		if (0u == iRowCount)
			continue;
		ImGui::PushID(static_cast<int>(eFamily));
		const std::string FamilyLabel =
			std::string(VisualProgramFamilyLabel(eFamily)) + " (" +
			std::to_string(iRowCount) + ")";
		if (ImGui::TreeNode(FamilyLabel.c_str()))
		{
			size_t iElementOrdinal = 0u;
			for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program->VisualRows)
			{
				if (Row.eFamily != eFamily)
					continue;
				ImGui::PushID(Row.Selector.strOccurrenceId.c_str());
				if (!IsAuthorableVisualRow(Row))
				{
					ImGui::PopID();
					continue;
				}
				++iElementOrdinal;
				const bool_t bSelected =
					m_SourceElementPresetSelection.has_value() &&
					m_SourceElementPresetSelection->strSourceEffectAssetId ==
						strEffectAssetId &&
					m_SourceElementPresetSelection->strOccurrenceId ==
						Row.Selector.strOccurrenceId &&
					m_SourceElementPresetSelection->strRowSha256 == Row.strRowSha256;
				const std::string Label = VisualProgramElementRowLabel(
					eFamily, iElementOrdinal,
					Row.SourceIdentity.strSourceRecordId,
					Row.Resources);
				if (bSelected)
					ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
						"%s", Label.c_str());
				else
					ImGui::TextWrapped("%s", Label.c_str());
				const bool_t bOccurrenceHovered = ImGui::IsItemHovered();
				ImGui::SameLine();
				if (ImGui::SmallButton("Load Seed"))
					Try_OpenVisualProgramElementForAuthoring(strEffectAssetId,
						Row.Selector.strOccurrenceId, Row.strRowSha256,
						Row.TargetIdentity->strTargetElementId,
						Row.SourceIdentity.strSourceRecordId);
				if (bOccurrenceHovered)
				{
					ImGui::SetTooltip(
						"Source: %s\nSlots: %s\nLoad Seed copies this immutable source into Element Authoring. Use Create Element, then Save Changes.",
						Row.SourceIdentity.strSourceRecordId.c_str(),
						VisualProgramResourceSlotSummary(Row.Resources).c_str());
				}
				ImGui::PopID();
			}
			for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row :
				Program->SupplementalElements)
			{
				if (Row.eFamily != eFamily)
					continue;
				ImGui::PushID(Row.Selector.strOccurrenceId.c_str());
				if (!IsAuthorableSupplementalRow(Row))
				{
					ImGui::PopID();
					continue;
				}
				++iElementOrdinal;
				const bool_t bSelected =
					m_SourceElementPresetSelection.has_value() &&
					m_SourceElementPresetSelection->strSourceEffectAssetId ==
						strEffectAssetId &&
					m_SourceElementPresetSelection->strOccurrenceId ==
						Row.Selector.strOccurrenceId &&
					m_SourceElementPresetSelection->strRowSha256 == Row.strRowSha256;
				std::string Label = VisualProgramElementRowLabel(
					eFamily, iElementOrdinal, Row.strSourceRecordId,
					Row.Resources);
				if (bSelected)
					ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
						"%s", Label.c_str());
				else
					ImGui::TextWrapped("%s", Label.c_str());
				const bool_t bOccurrenceHovered = ImGui::IsItemHovered();
				ImGui::SameLine();
				if (ImGui::SmallButton("Load Seed"))
					Try_OpenVisualProgramElementForAuthoring(strEffectAssetId,
						Row.Selector.strOccurrenceId, Row.strRowSha256,
						Row.TargetIdentity.strTargetElementId,
						Row.strSourceRecordId);
				if (bOccurrenceHovered)
				{
					ImGui::SetTooltip(
						"Source: %s\nSlots: %s\nLoad Seed copies this immutable source into Element Authoring. Use Create Element, then Save Changes.",
						Row.strSourceRecordId.c_str(),
						VisualProgramResourceSlotSummary(Row.Resources).c_str());
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TreePop();
}

void Client::CEffect_Tool::Render_ArtistFCoreAuthoring()
{
	/* Rendering the tree must stay metadata-only.  Source projection and typed
	   material preparation synchronously prewarm WModels/DDS resources, so they
	   are initiated only by Load Seed/Upgrade below. */
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	const uint64_t iRuntimeRevision = CEffectCatalog::Get_RuntimeRevision();
	const bool_t bSourcePreparedForRevision =
		ARTIST_F_PREPARATION_STATE::READY ==
			m_eArtistFSourcePreparationState &&
		m_iArtistFSourceSnapshotRevision == iRuntimeRevision;
	const bool_t bMaterialPreparedForRevision =
		ARTIST_F_PREPARATION_STATE::READY ==
			m_eArtistFMaterialPreparationState &&
		m_iArtistFMaterialExecutionSnapshotRevision == iRuntimeRevision;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = bSourcePreparedForRevision ?
			m_pArtistFSourceProjection : nullptr;
	const bool_t bEditableSkillEffectActive =
		m_ActiveDocument.has_value() &&
		(EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource) &&
		m_ActiveDocument->strEffectAssetId ==
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
	if (nullptr == pProgram ||
		pProgram->strRuntimeCatalogAssetId != ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		ImGui::TextDisabled("Artist F source recipe is unavailable.");
		if (!m_strArtistFSourceSnapshotStatus.empty())
			ImGui::TextWrapped("%s", m_strArtistFSourceSnapshotStatus.c_str());
		return;
	}
	if ((ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFSourcePreparationState &&
		 m_iArtistFSourcePreparationAttemptRevision == iRuntimeRevision) ||
		(ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFMaterialPreparationState &&
		 m_iArtistFMaterialPreparationAttemptRevision == iRuntimeRevision))
	{
		ImGui::TextColored(ImVec4(1.f, 0.55f, 0.25f, 1.f),
			"Track A preparation failed. Refresh after source/catalog changes before retrying.");
		if (!m_strArtistFSourceSnapshotStatus.empty())
			ImGui::TextWrapped("%s", m_strArtistFSourceSnapshotStatus.c_str());
	}
	else if (!bMaterialPreparedForRevision)
	{
		ImGui::TextDisabled(
			"Track A seed resources are prepared only when Load Seed is pressed.");
	}

	const bool_t bRootOpen = ImGui::TreeNodeEx(
		"Track A Element Seeds (33)##artist-f-seeds",
		ImGuiTreeNodeFlags_OpenOnArrow);
	if (!bRootOpen)
		return;
	ImGui::TextDisabled(
		"Read-only source library. Open Editable Skill Effect above, then Load Seed -> Create Element -> tune -> Save Changes.");
	if (!bEditableSkillEffectActive)
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"Open the Artist F Editable Skill Effect first. Seeds are ingredients, not a playable Effect.");
	}

	static constexpr std::array<EFFECT_GPU_RENDER_FAMILY, 4u> FAMILIES{
		EFFECT_GPU_RENDER_FAMILY::MESH,
		EFFECT_GPU_RENDER_FAMILY::SPRITE,
		EFFECT_GPU_RENDER_FAMILY::DECAL,
		EFFECT_GPU_RENDER_FAMILY::RIBBON };
	static constexpr std::array<size_t, 4u> EXPECTED_COUNTS{
		13u, 16u, 3u, 1u };
	const auto LoadSeed = [this](
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter) -> bool_t
	{
		if (!Ensure_ArtistFMaterialExecutionSnapshots())
		{
			m_strElementStatus = m_strArtistFSourceSnapshotStatus.empty() ?
				"Artist F Track A material snapshots are unavailable." :
				m_strArtistFSourceSnapshotStatus;
			return false;
		}
		const std::shared_ptr<const
			EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection =
			m_pArtistFSourceProjection;
		if (nullptr == pProjection || !pProjection->Is_Valid())
		{
			m_strElementStatus =
				"Artist F Track A source projection is unavailable after preparation.";
			return false;
		}
		const std::string strVisualOccurrenceId =
			Emitter.strMaterialOccurrenceId.value_or(Emitter.Row.strId);
		const EFFECT_VISUAL_PROGRAM_ROW* pVisualRow =
			pProjection->Find_RowByOccurrenceId(strVisualOccurrenceId);
		const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pSupplemental =
			pProjection->Find_SupplementalElementByOccurrenceId(
				strVisualOccurrenceId);
		if (nullptr != pVisualRow && nullptr != pSupplemental)
		{
			m_strElementStatus =
				"The Track A seed matched both a visual row and a supplemental Element.";
			return false;
		}
		const bool_t bRequiresStrictPreset =
			strVisualOccurrenceId == "source-active-003" ||
			strVisualOccurrenceId == "source-active-020" ||
			strVisualOccurrenceId == "source-active-021";
		if (bRequiresStrictPreset && nullptr == pVisualRow &&
			nullptr == pSupplemental)
		{
			m_strElementStatus =
				"The typed Artist F Decal/Ribbon seed lost its admitted Track A identity.";
			return false;
		}
		bool_t bLoaded = false;
		if (nullptr != pVisualRow)
		{
			if (!pVisualRow->TargetIdentity.has_value() ||
				pVisualRow->TargetIdentity->strTargetElementId !=
					Emitter.strSourceElementId)
			{
				m_strElementStatus =
					"The admitted Track A seed target no longer matches the selected source Element.";
				return false;
			}
			bLoaded = Try_OpenVisualProgramElementForAuthoring(
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID,
				pVisualRow->Selector.strOccurrenceId,
				pVisualRow->strRowSha256,
				pVisualRow->TargetIdentity->strTargetElementId,
				pVisualRow->SourceIdentity.strSourceRecordId);
		}
		else if (nullptr != pSupplemental)
		{
			if (pSupplemental->TargetIdentity.strTargetElementId !=
				Emitter.strSourceElementId)
			{
				m_strElementStatus =
					"The admitted Track A supplemental target no longer matches the selected source Element.";
				return false;
			}
			bLoaded = Try_OpenVisualProgramElementForAuthoring(
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID,
				pSupplemental->Selector.strOccurrenceId,
				pSupplemental->strRowSha256,
				pSupplemental->TargetIdentity.strTargetElementId,
				pSupplemental->strSourceRecordId);
		}
		else
		{
			bLoaded = Try_OpenArtistFReconstructedElementForAuthoring(Emitter);
		}
		if (!bLoaded)
			return false;

		const auto Source = std::find_if(
			pProjection->Get_Document().Elements.begin(),
			pProjection->Get_Document().Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Emitter.strSourceElementId; });
		std::string Error;
		if (Source == pProjection->Get_Document().Elements.end() ||
			!Try_ApplyArtistFTrackASeedData(
				Emitter, *Source, m_MeshAuthoringDraft, Error))
		{
			m_strElementStatus = Error.empty() ?
				"Artist F Track A seed data could not be normalized for authoring." :
				Error;
			return false;
		}
		const auto Registry = Emitter.strMaterialOccurrenceId.has_value() ?
			Find_Artist31470ShaderRegistry(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId) :
			std::nullopt;
		if (!Registry.has_value())
		{
			m_strElementStatus =
				"Artist F Track A seed lost its shader-registry identity.";
			return false;
		}
		if (m_MeshAuthoringDraft.Material.Execution.bEnabled)
		{
			m_strElementStatus =
				"Loaded Artist F Track A seed with its typed Material slots, fixed burst, local-space, WModel import scale, and constant-one DynamicParameter fallback where consumed. Attachment basis remains emit-start baked. Ribbon history restoration is outside this pass. Use Create Element, then Save Changes.";
		}
		else if (Registry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON)
		{
			m_strElementStatus =
				"Loaded Artist F #17 FiniteCommon seed with its bounded standard authored Material. Use Create Element, tune, then Save Changes.";
		}
		else
		{
			m_strElementStatus =
				"Loaded an Artist F fail-closed seed (#1/#16/#26/#33). It is invisible by default because its missing SceneColor/depth/fog/MRT contract must not be replaced by a generic white/distortion fallback.";
		}
		return true;
	};
	for (size_t iFamilyIndex = 0u; iFamilyIndex < FAMILIES.size();
		++iFamilyIndex)
	{
		const EFFECT_GPU_RENDER_FAMILY eFamily = FAMILIES[iFamilyIndex];
		std::vector<const EFFECT_RUNTIME_PROGRAM_EMITTER*> Emitters;
		for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : pProgram->Emitters)
		{
			EFFECT_GPU_RENDER_FAMILY eEmitterFamily =
				EFFECT_GPU_RENDER_FAMILY::END;
			if (!Emitter.bVisible ||
				!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eEmitterFamily) ||
				eEmitterFamily != eFamily)
			{
				continue;
			}
			if (nullptr == pProjection)
			{
				Emitters.push_back(&Emitter);
				continue;
			}
			const auto Element = std::find_if(
				pProjection->Get_Document().Elements.begin(),
				pProjection->Get_Document().Elements.end(),
				[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId == Emitter.strSourceElementId;
				});
			if (Element != pProjection->Get_Document().Elements.end() &&
				Element->bVisible)
			{
				Emitters.push_back(&Emitter);
			}
		}

		ImGui::PushID(static_cast<int>(eFamily));
		const std::string FamilyLabel =
			std::string(ArtistCoreFamilyLabel(eFamily)) + " (" +
			std::to_string(Emitters.size()) + ")";
		if (ImGui::TreeNodeEx(FamilyLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow))
		{
			if (Emitters.size() != EXPECTED_COUNTS[iFamilyIndex])
			{
				ImGui::TextColored(ImVec4(1.f, 0.55f, 0.25f, 1.f),
					"Fail closed: expected %zu visible Elements, found %zu.",
					EXPECTED_COUNTS[iFamilyIndex], Emitters.size());
			}
			for (size_t iElement = 0u; iElement < Emitters.size(); ++iElement)
			{
				const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter = *Emitters[iElement];
				ImGui::PushID(Emitter.Row.strId.c_str());
				const EFFECT_ELEMENT_DESC* pElement = nullptr;
				if (nullptr != pProjection)
				{
					const auto ProjectedElement = std::find_if(
						pProjection->Get_Document().Elements.begin(),
						pProjection->Get_Document().Elements.end(),
						[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
						{
							return Candidate.strElementId ==
								Emitter.strSourceElementId;
						});
					if (ProjectedElement !=
						pProjection->Get_Document().Elements.end())
					{
						pElement = &*ProjectedElement;
					}
				}
				const std::string strVisualOccurrenceId =
					Emitter.strMaterialOccurrenceId.value_or(Emitter.Row.strId);
				const bool_t bUsesAdmittedVisualSeed =
					nullptr != pProjection &&
					(nullptr != pProjection->Find_RowByOccurrenceId(
						strVisualOccurrenceId) ||
					 nullptr != pProjection->Find_SupplementalElementByOccurrenceId(
						strVisualOccurrenceId));
				const std::string& strSelectionOccurrenceId =
					bUsesAdmittedVisualSeed ? strVisualOccurrenceId :
						Emitter.Row.strId;
				const bool_t bSelected =
					m_SourceElementPresetSelection.has_value() &&
					m_SourceElementPresetSelection->strSourceEffectAssetId ==
						ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
					m_SourceElementPresetSelection->strOccurrenceId ==
						strSelectionOccurrenceId;
				const std::string Label =
					std::string(ArtistCoreFamilyLabel(eFamily)) + " " +
					(iElement + 1u < 10u ? "0" : "") +
					std::to_string(iElement + 1u) + " | " +
					(nullptr == pElement ?
						StableIdentityLeaf(Emitter.strSourceEmitterPath) :
						PrimaryAuthoringResourceLeaf(*pElement));
				if (bSelected)
				{
					ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
						"%s", Label.c_str());
				}
				else
				{
					ImGui::TextWrapped("%s", Label.c_str());
				}
				const bool_t bLabelHovered = ImGui::IsItemHovered();
				ImGui::SameLine();
				ImGui::BeginDisabled(!bEditableSkillEffectActive);
				if (ImGui::SmallButton("Load Seed"))
					LoadSeed(Emitter);
				ImGui::EndDisabled();
				if (bLabelHovered)
				{
					ImGui::SetTooltip(
						"Track A source Element\nSource: %s\nSlots: %s\nLoad Seed -> Create Element -> Save Changes",
						Emitter.strSourceEmitterPath.c_str(),
						nullptr == pElement ? "deferred until Load Seed" :
							AuthoringElementResourceSlotSummary(*pElement).c_str());
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::TreePop();
}

bool_t Client::CEffect_Tool::Refresh_UnifiedEffectCache(
	UNIFIED_EFFECT_CACHE& Cache,
	const std::filesystem::path& Path,
	const std::string& strExpectedEffectAssetId)
{
	std::error_code FileError;
	const bool_t bExists = !Path.empty() &&
		std::filesystem::exists(Path, FileError);
	if (FileError)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus =
			"Unified Effect path could not be inspected: " + FileError.message();
		return false;
	}
	if (!bExists)
	{
		if (!Cache.bObserved || Cache.Path != Path || Cache.bExists)
		{
			Cache = {};
			Cache.Path = Path;
			Cache.bObserved = true;
			Cache.strStatus = "Unified Effect has not been created yet.";
		}
		return false;
	}

	const bool_t bRegular = std::filesystem::is_regular_file(Path, FileError);
	if (FileError || !bRegular)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus = FileError ?
			("Unified Effect path could not be inspected: " + FileError.message()) :
			"Unified Effect path exists but is not a regular file; overwrite is blocked.";
		return false;
	}

	const std::filesystem::file_time_type LastWriteTime =
		std::filesystem::last_write_time(Path, FileError);
	if (FileError)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus =
			"Unified Effect timestamp could not be read: " + FileError.message();
		return false;
	}
	const uint64_t iFileSize = static_cast<uint64_t>(
		std::filesystem::file_size(Path, FileError));
	if (FileError)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus =
			"Unified Effect size could not be read: " + FileError.message();
		return false;
	}
	if (Cache.bObserved && Cache.bExists && Cache.Path == Path &&
		Cache.LastWriteTime == LastWriteTime && Cache.iFileSize == iFileSize)
	{
		return Cache.bValid;
	}

	UNIFIED_EFFECT_CACHE Staged;
	Staged.Path = Path;
	Staged.LastWriteTime = LastWriteTime;
	Staged.iFileSize = iFileSize;
	Staged.bObserved = true;
	Staged.bExists = true;
	std::string Error;
	if (!CEffectDocumentCodec::Load(Path, Staged.Document, Error))
	{
		Staged.strStatus = "Unified Effect could not be parsed: " + Error;
		Cache = std::move(Staged);
		return false;
	}
	if (Staged.Document.strEffectAssetId != strExpectedEffectAssetId)
	{
		Staged.strStatus = "Unified Effect ID mismatch; expected '" +
			strExpectedEffectAssetId + "', found '" +
			Staged.Document.strEffectAssetId + "'. Overwrite is blocked.";
		Staged.Document = {};
		Staged.bDrawable = false;
		Staged.strDrawableError.clear();
		Cache = std::move(Staged);
		return false;
	}
	if (!CEffectDocumentCodec::Validate_Drawable(Staged.Document, Error))
	{
		Staged.bValid = true;
		Staged.bDrawable = false;
		Staged.bPreviewReady = false;
		Staged.strDrawableError = Error;
		Staged.strPreviewReadinessError = Error;
		Staged.strStatus =
			"Saved authoring parent is structurally valid but not drawable yet: " +
			Error;
		Cache = std::move(Staged);
		return true;
	}
	Staged.bValid = true;
	Staged.bDrawable = true;
	Staged.strDrawableError.clear();
	Staged.bPreviewReady = Validate_UnifiedEffectPreviewReadiness(
		Staged.Document, Staged.strPreviewReadinessError);
	Staged.strStatus = Staged.bPreviewReady ?
		"Editable Skill Effect is ready." :
		("Editable Skill Effect is not ready for preview: " +
		 Staged.strPreviewReadinessError);
	Cache = std::move(Staged);
	return true;
}

bool_t Client::CEffect_Tool::Refresh_DirectAuthoredEditableIndex(
	const std::vector<EFFECT_DATA_FILE_ENTRY>& DataFiles)
{
	const auto PreservePrevious = [this](std::string Status)
	{
		m_strDirectAuthoredEditableStatus = std::move(Status);
		m_strUnifiedCandidateStatus =
			"Saved authored catalog refresh preserved the previous index: " +
			m_strDirectAuthoredEditableStatus;
		return false;
	};
	const std::filesystem::path CatalogPath = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"EffectCatalog.json");
	const std::filesystem::path AuthoredRoot = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored");
	std::string SkillCatalogStatus;
	if (!Ensure_PlayerSkillCatalog(SkillCatalogStatus))
	{
		return PreservePrevious(
			"Direct authored edit index could not validate PlayerSkills owners: " +
			SkillCatalogStatus);
	}
	EFFECT_DIRECT_AUTHORED_OWNER_SET PlayerSkillOwners;
	for (const PLAYER_SKILL_DEFINITION& Skill :
		CPlayerSkillCatalog::Get_Skills())
	{
		PlayerSkillOwners.emplace(Skill.eCharacterClass, Skill.iSkillId);
	}
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT BossCueDocument;
	std::string BossCueStatus;
	if (!CValtanPatternEffectCueDocument::Load_Source(
			BossCueDocument, BossCueStatus))
	{
		return PreservePrevious(
			"Direct authored edit index could not validate Valtan pattern owners: " +
			BossCueStatus);
	}
	EFFECT_DIRECT_AUTHORED_BOSS_OWNER_MAP BossPatternOwners;
	std::unordered_map<std::string, size_t> StagedBossProductCueMappingCounts;
	for (const VALTAN_PATTERN_EFFECT_CUE& Cue : BossCueDocument.Cues)
	{
		const EFFECT_DIRECT_AUTHORED_BOSS_OWNER Owner{
			BossCueDocument.strOwnerArchetypeId, Cue.strPatternId,
				Cue.strStageId, Cue.strActionId };
		BossPatternOwners.emplace(Cue.strEffectAssetId, Owner);
		++StagedBossProductCueMappingCounts[Cue.strEffectAssetId];
		if (!Cue.strV1EffectAssetId.empty())
		{
			BossPatternOwners.emplace(Cue.strV1EffectAssetId, Owner);
			++StagedBossProductCueMappingCounts[Cue.strV1EffectAssetId];
		}
	}
	const BOSS_ACTOR_ENTRY* pBossActor = CActorCatalog::Find_Boss(
		BossCueDocument.strOwnerArchetypeId);
	if (nullptr == pBossActor)
	{
		return PreservePrevious(
			"Direct authored edit index could not validate Valtan combat-object owners: " +
			CActorCatalog::Get_Status());
	}
	EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER_MAP
		BossCombatObjectOwners;
	for (const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& Visual :
		pBossActor->combatObjectVisuals)
	{
		BossCombatObjectOwners.emplace(Visual.effectAssetId,
			EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER{
				pBossActor->archetypeId, Visual.combatObjectArchetypeId,
				Visual.clientVisualId });
	}

	std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE> ScannedFiles;
	ScannedFiles.reserve(DataFiles.size());
	for (const EFFECT_DATA_FILE_ENTRY& DataFile : DataFiles)
	{
		if (EFFECT_DOCUMENT_SOURCE::AUTHORED == DataFile.eSource)
		{
			ScannedFiles.push_back(
				{ DataFile.strAssetId, DataFile.Path });
		}
	}
	EFFECT_DIRECT_AUTHORED_SOURCE_INDEX SourceIndex;
	std::string SourceIndexStatus;
	if (!CEffectDirectAuthoredSourceIndex::Build(
			CatalogPath, AuthoredRoot, ScannedFiles, PlayerSkillOwners,
			BossPatternOwners, BossCombatObjectOwners,
			SourceIndex, SourceIndexStatus))
	{
		return PreservePrevious(SourceIndexStatus);
	}

	std::unordered_map<std::string, DIRECT_AUTHORED_EDITABLE_ENTRY>
		StagedEntries;
	StagedEntries.reserve(SourceIndex.Entries.size());
	std::vector<UNIFIED_EFFECT_CANDIDATE_BINDING> StagedBindings;
	StagedBindings.reserve(SourceIndex.Entries.size());
	for (const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Source :
		SourceIndex.Entries)
	{
		DIRECT_AUTHORED_EDITABLE_ENTRY Staged;
		Staged.Path = Source.Path;
		Staged.LastWriteTime = Source.LastWriteTime;
		Staged.iFileSize = Source.iFileSize;
		const auto Existing = m_DirectAuthoredEditableEntries.find(
			Source.strEffectAssetId);
		if (Existing != m_DirectAuthoredEditableEntries.end() &&
			Existing->second.Path.lexically_normal() ==
				Staged.Path.lexically_normal() &&
			Existing->second.LastWriteTime == Staged.LastWriteTime &&
			Existing->second.iFileSize == Staged.iFileSize)
		{
			Staged = Existing->second;
		}
		StagedEntries.emplace(Source.strEffectAssetId, std::move(Staged));
		if (EFFECT_DIRECT_AUTHORED_OWNER_KIND::PLAYER_SKILL ==
			Source.eOwnerKind)
		{
			UNIFIED_EFFECT_CANDIDATE_BINDING Binding;
			Binding.eCharacterClass = Source.eCharacterClass;
			Binding.iSkillId = Source.iSkillId;
			Binding.strEffectAssetId = Source.strEffectAssetId;
			Binding.Path = Source.Path;
			StagedBindings.push_back(std::move(Binding));
		}
	}
	std::unordered_map<std::string, UNIFIED_EFFECT_CACHE> StagedCaches;
	StagedCaches.reserve(StagedBindings.size());
	for (const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding : StagedBindings)
	{
		const auto Existing = m_UnifiedCandidateCaches.find(
			Binding.strEffectAssetId);
		if (Existing != m_UnifiedCandidateCaches.end() &&
			Existing->second.Path.lexically_normal() ==
				Binding.Path.lexically_normal())
		{
			StagedCaches.emplace(
				Binding.strEffectAssetId, std::move(Existing->second));
		}
		else
		{
			UNIFIED_EFFECT_CACHE Cache;
			Cache.Path = Binding.Path;
			StagedCaches.emplace(
				Binding.strEffectAssetId, std::move(Cache));
		}
	}
	m_DirectAuthoredEditableEntries = std::move(StagedEntries);
	m_UnifiedCandidateBindings = std::move(StagedBindings);
	m_UnifiedCandidateCaches = std::move(StagedCaches);
	m_BossProductCueMappingCounts =
		std::move(StagedBossProductCueMappingCounts);
	m_strDirectAuthoredEditableStatus = SourceIndexStatus;
	m_strUnifiedCandidateStatus =
		"Saved authored catalog indexed " +
		std::to_string(m_UnifiedCandidateBindings.size()) +
		" unified Effects from EffectCatalog.json; document decode is deferred until Open or Play.";
	if (0u != SourceIndex.iUnavailableCount)
	{
		const std::string IsolationStatus = " Isolated " +
			std::to_string(SourceIndex.iUnavailableCount) +
			" unavailable rows; first: " +
			SourceIndex.strFirstUnavailable;
		m_strUnifiedCandidateStatus += IsolationStatus;
	}
	return true;
}

const std::filesystem::path*
Client::CEffect_Tool::Resolve_DirectAuthoredEditablePath(
	const std::string& strEffectAssetId,
	std::string& strOutStatus)
{
	const auto Iterator =
		m_DirectAuthoredEditableEntries.find(strEffectAssetId);
	if (Iterator == m_DirectAuthoredEditableEntries.end())
	{
		strOutStatus =
			"Open is unavailable: this Effect is not an exact writable "
			"DIRECT_AUTHORED_DOCUMENT_V13 source path.";
		return nullptr;
	}
	DIRECT_AUTHORED_EDITABLE_ENTRY& Entry = Iterator->second;
	std::error_code FileError;
	const std::filesystem::file_time_type LastWriteTime =
		std::filesystem::last_write_time(Entry.Path, FileError);
	uint64_t iFileSize = 0u;
	if (!FileError)
	{
		iFileSize = static_cast<uint64_t>(
			std::filesystem::file_size(Entry.Path, FileError));
	}
	if (FileError)
	{
		Entry.bIdentityObserved = true;
		Entry.bIdentityValid = false;
		Entry.strStatus =
			"Open is unavailable: the direct authored file cannot be inspected: " +
			FileError.message();
		strOutStatus = Entry.strStatus;
		return nullptr;
	}
	if (Entry.LastWriteTime != LastWriteTime || Entry.iFileSize != iFileSize)
	{
		Entry.LastWriteTime = LastWriteTime;
		Entry.iFileSize = iFileSize;
		Entry.bIdentityObserved = false;
		Entry.bIdentityValid = false;
		Entry.strStatus.clear();
	}
	if (!Entry.bIdentityObserved)
	{
		EFFECT_DOCUMENT_DESC Document;
		std::string Error;
		Entry.bIdentityValid = CEffectDocumentCodec::Load(
			Entry.Path, Document, Error) &&
			Document.iLoadedFormatVersion == EFFECT_AUTHORING_FORMAT_VERSION &&
			Document.strEffectAssetId == strEffectAssetId;
		Entry.bIdentityObserved = true;
		if (Entry.bIdentityValid)
		{
			Entry.strStatus =
				"Open the writable Data/Effects/Authored version 13 document. "
				"Save updates Authored and, while the exact ID remains mapped by a Product cue, hot reloads only that Product target. A failed runtime replacement preserves the existing prepared target.";
		}
		else if (!Error.empty())
		{
			Entry.strStatus =
				"Open is unavailable: direct authored validation failed: " +
				Error;
		}
		else
		{
			Entry.strStatus =
				"Open is unavailable: the direct authored version/embedded Effect ID disagrees with EffectCatalog.json.";
		}
	}
	strOutStatus = Entry.strStatus;
	return Entry.bIdentityValid ? &Entry.Path : nullptr;
}

bool_t Client::CEffect_Tool::Is_UnifiedEffectActive(
	const UNIFIED_EFFECT_CACHE& Cache) const
{
	return Cache.bValid && m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		m_ActiveDocument->strEffectAssetId == Cache.Document.strEffectAssetId &&
		!m_ActiveDocumentPath.empty() &&
		m_ActiveDocumentPath.lexically_normal() == Cache.Path.lexically_normal();
}

bool_t Client::CEffect_Tool::Validate_UnifiedEffectPreviewReadiness(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError) const
{
	if (Document.strEffectAssetId != ARTIST_F_UNIFIED_EFFECT_ASSET_ID)
	{
		const bool_t bHasVisibleCarrier = std::any_of(
			Document.Elements.begin(), Document.Elements.end(),
			[](const EFFECT_ELEMENT_DESC& Element)
			{ return Is_ElementPreviewAdmitted(Element); }) ||
			std::any_of(Document.ModelCues.begin(), Document.ModelCues.end(),
				[](const EFFECT_MODEL_CUE_DESC& Cue)
				{ return Cue.bVisible; });
		if (!bHasVisibleCarrier)
		{
			strOutError =
				"Candidate has no visible Element or Model / Summon cue to stage.";
			return false;
		}
		strOutError.clear();
		return true;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		CEffectCatalog::Find_ReconstructedRuntimeProgram(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	if (nullptr == pProgram)
	{
		strOutError = "Artist F Track A Program metadata is unavailable.";
		return false;
	}
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS Stats;
	return CEffectDocumentCodec::
		Validate_Artist31470UnifiedAuthoredReadiness(
			*pProgram, Document, Stats, strOutError);
}

bool_t Client::CEffect_Tool::Try_LoadUnifiedElement(
	const UNIFIED_EFFECT_CACHE& Cache,
	const std::string& strElementId)
{
	if (!Cache.bValid || strElementId.empty())
		return false;
	std::string strEditableStatus;
	const std::filesystem::path* pEditablePath =
		Resolve_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	if (nullptr == pEditablePath)
	{
		m_strDocumentStatus = strEditableStatus;
		return false;
	}
	if (!Is_UnifiedEffectActive(Cache))
	{
		if (!Try_LoadDocumentPath(*pEditablePath,
				EFFECT_DOCUMENT_SOURCE::AUTHORED,
				Cache.Document.strEffectAssetId))
		{
			if (m_PendingDocumentLoad.has_value() &&
				m_PendingDocumentLoad->Path == *pEditablePath)
			{
				m_PendingDocumentLoad->strElementSelectionId = strElementId;
				m_PendingDocumentLoad->strModelCueSelectionId.clear();
			}
			return false;
		}
	}
	return Try_SelectElement(Cache.Document.strEffectAssetId, strElementId);
}

bool_t Client::CEffect_Tool::Try_LoadUnifiedModelCue(
	const UNIFIED_EFFECT_CACHE& Cache,
	const std::string& strCueId)
{
	if (!Cache.bValid || strCueId.empty())
		return false;
	std::string strEditableStatus;
	const std::filesystem::path* pEditablePath =
		Resolve_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	if (nullptr == pEditablePath)
	{
		m_strDocumentStatus = strEditableStatus;
		return false;
	}
	if (!Is_UnifiedEffectActive(Cache))
	{
		if (!Try_LoadDocumentPath(*pEditablePath,
				EFFECT_DOCUMENT_SOURCE::AUTHORED,
				Cache.Document.strEffectAssetId))
		{
			if (m_PendingDocumentLoad.has_value() &&
				m_PendingDocumentLoad->Path == *pEditablePath)
			{
				m_PendingDocumentLoad->strElementSelectionId.clear();
				m_PendingDocumentLoad->strModelCueSelectionId = strCueId;
			}
			return false;
		}
	}
	return Try_SelectModelCue(Cache.Document.strEffectAssetId, strCueId);
}

bool_t Client::CEffect_Tool::Try_PlayUnifiedAuthoringFamily(
	const std::string& strEffectAssetId,
	const EFFECT_AUTHORING_FAMILY eFamily)
{
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
		EFFECT_AUTHORING_FAMILY::END == eFamily)
	{
		m_strPreviewStatus =
			"Load this Effect before playing one Family.";
		return false;
	}
	if (std::none_of(m_ActiveDocument->Elements.begin(),
		m_ActiveDocument->Elements.end(),
		[eFamily](const EFFECT_ELEMENT_DESC& Element)
		{
			return Resolve_AuthoringFamily(Element) == eFamily &&
				Is_ElementPreviewAdmitted(Element);
		}))
	{
		m_strPreviewStatus =
			"The selected Family has no visible authoring-admitted Element to play. Hard-locked Elements remain editable; APPROXIMATE Elements are admitted for authoring preview.";
		return false;
	}
	const EFFECT_AUTHORING_FAMILY ePreviousFamily =
		m_ePreviewIsolationAuthoringFamily;
	m_ePreviewIsolationAuthoringFamily = eFamily;
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY))
	{
		m_ePreviewIsolationAuthoringFamily = ePreviousFamily;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayUnifiedEffect(
	const UNIFIED_EFFECT_CACHE& Cache)
{
	if (!Cache.bValid)
	{
		m_strPreviewStatus = "The saved Skill Effect is unavailable.";
		return false;
	}
	std::string strEditableStatus;
	const std::filesystem::path* pEditablePath =
		Resolve_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	if (nullptr == pEditablePath)
	{
		m_strPreviewStatus = strEditableStatus;
		return false;
	}
	const bool_t bActive = Is_UnifiedEffectActive(Cache);
	std::string ActiveReadinessError;
	const bool_t bPreviewReady = bActive ?
		Validate_UnifiedEffectPreviewReadiness(
			*m_ActiveDocument, ActiveReadinessError) : Cache.bPreviewReady;
	if (!bPreviewReady)
	{
		const std::string& ReadinessError = bActive ?
			ActiveReadinessError : Cache.strPreviewReadinessError;
		m_strPreviewStatus = ReadinessError.empty() ?
			"The saved Skill Effect is not ready for preview." :
			ReadinessError;
		return false;
	}
	if (!bActive)
	{
		if (!Try_LoadDocumentPath(*pEditablePath,
				EFFECT_DOCUMENT_SOURCE::AUTHORED,
				Cache.Document.strEffectAssetId))
		{
			if (m_PendingDocumentLoad.has_value() &&
				m_PendingDocumentLoad->Path == *pEditablePath)
			{
				m_PendingDocumentLoad->strElementSelectionId.clear();
				m_PendingDocumentLoad->strModelCueSelectionId.clear();
				m_PendingDocumentLoad->bPlayCompleteAfterLoad = true;
			}
			return false;
		}
	}
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
		return false;
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayActiveUnifiedEffect()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strPreviewStatus = "No saved Effect is loaded for preview.";
		return false;
	}
	if (!m_bActiveDocumentDrawable)
	{
		m_strPreviewStatus = m_strActiveDocumentDrawableError.empty() ?
			"The saved Effect is not drawable." :
			m_strActiveDocumentDrawableError;
		return false;
	}
	std::string ReadinessError;
	if (!Validate_UnifiedEffectPreviewReadiness(
			*m_ActiveDocument, ReadinessError))
	{
		m_strPreviewStatus = ReadinessError.empty() ?
			"The saved Effect is not ready for preview." : ReadinessError;
		return false;
	}
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
		return false;
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_PlaySavedUnifiedEffect(
	const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding)
{
	const auto Cache = m_UnifiedCandidateCaches.find(
		Binding.strEffectAssetId);
	if (Cache == m_UnifiedCandidateCaches.end())
	{
		m_strPreviewStatus =
			"The saved authored Effect is no longer in the source catalog index.";
		return false;
	}
	if (!Refresh_UnifiedEffectCache(Cache->second, Binding.Path,
			Binding.strEffectAssetId))
	{
		m_strPreviewStatus = Cache->second.strStatus;
		return false;
	}
	return Try_PlayUnifiedEffect(Cache->second);
}

bool_t Client::CEffect_Tool::Try_PlayUnifiedModelCues(
	const std::string& strEffectAssetId)
{
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
		m_ActiveDocument->ModelCues.empty())
	{
		m_strPreviewStatus =
			"Load this Effect before playing Model / Summon.";
		return false;
	}
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES))
		return false;
	Start_WorldPreviewFromBeginning();
	return true;
}

void Client::CEffect_Tool::Render_UnifiedEffectTree(
	const UNIFIED_EFFECT_CACHE& Cache,
	const std::string& strFallbackDisplayName,
	const VALTAN_CLIP_OCCURRENCE_VIEW* pValtanClip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pValtanCue)
{
	const bool_t bValtanProductRow = nullptr != pValtanClip &&
		nullptr != pValtanCue;
	if (!Cache.bValid)
		return;
	const bool_t bActive = Is_UnifiedEffectActive(Cache);
	const EFFECT_DOCUMENT_DESC& Document = bActive ?
		*m_ActiveDocument : Cache.Document;
	const bool_t bDrawable = bActive ?
		m_bActiveDocumentDrawable : Cache.bDrawable;
	std::string strActivePreviewReadinessError;
	const bool_t bPreviewReady = bActive ?
		Validate_UnifiedEffectPreviewReadiness(
			Document, strActivePreviewReadinessError) : Cache.bPreviewReady;
	const std::string& PreviewReadinessError = bActive ?
		strActivePreviewReadinessError : Cache.strPreviewReadinessError;
	const std::string& DrawableError = bActive ?
		m_strActiveDocumentDrawableError : Cache.strDrawableError;
	std::string strEditableStatus;
	const std::filesystem::path* pEditablePath =
		Resolve_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	const std::string RootLabel = strFallbackDisplayName + "##mapped-effect";
	ImGui::PushID(Cache.Document.strEffectAssetId.c_str());
	const bool_t bOpen = ImGui::TreeNodeEx(RootLabel.c_str(),
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow |
		(bActive ? ImGuiTreeNodeFlags_Selected : 0));
	ImGui::SameLine();
	ImGui::BeginDisabled(bActive || nullptr == pEditablePath);
	if (ImGui::SmallButton("Open Editor") && nullptr != pEditablePath)
	{
		if (!bValtanProductRow)
		{
			Try_LoadDocumentPath(*pEditablePath,
				EFFECT_DOCUMENT_SOURCE::AUTHORED,
				Cache.Document.strEffectAssetId);
		}
		else
		{
			Try_OpenValtanAuthoredEffect(*pEditablePath,
				Cache.Document.strEffectAssetId,
				*pValtanClip, *pValtanCue);
		}
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("%s", bActive ?
			"This saved authored Effect is already the Current Effect." :
			strEditableStatus.c_str());
	}
	if (!bOpen)
	{
		ImGui::PopID();
		return;
	}

	ImGui::BeginDisabled(nullptr == pEditablePath || !bDrawable ||
		!bPreviewReady);
	if (ImGui::SmallButton("Play All") && nullptr != pEditablePath)
	{
		bool_t bTargetReady = true;
		if (bValtanProductRow)
		{
			bTargetReady = bActive ?
				Play_ValtanProductCue(*pValtanClip, *pValtanCue) :
				Try_OpenValtanAuthoredEffect(*pEditablePath,
					Cache.Document.strEffectAssetId,
					*pValtanClip, *pValtanCue, true);
		}
		if (bTargetReady)
			Try_PlayUnifiedEffect(Cache);
	}
	ImGui::EndDisabled();
	if (nullptr == pEditablePath && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("%s", strEditableStatus.c_str());
	}
	else if (!bDrawable && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			("Finish required WModel/DDS bindings before preview: " +
				DrawableError).c_str());
	}
	else if (!bPreviewReady && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("%s", PreviewReadinessError.c_str());
	}
	else if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Load this saved Skill Effect if needed, then play it from the beginning.");
	if (!bDrawable)
		ImGui::TextDisabled("Saved partial Effect: %s", DrawableError.c_str());

	for (int32_t iFamily = 0;
		iFamily < static_cast<int32_t>(EFFECT_AUTHORING_FAMILY::END);
		++iFamily)
	{
		const EFFECT_AUTHORING_FAMILY eFamily =
			static_cast<EFFECT_AUTHORING_FAMILY>(iFamily);
		const size_t iCount = static_cast<size_t>(std::count_if(
			Document.Elements.begin(), Document.Elements.end(),
			[eFamily](const EFFECT_ELEMENT_DESC& Element)
			{ return Resolve_AuthoringFamily(Element) == eFamily; }));
		if (0u == iCount)
			continue;
		const size_t iPlayLockedCount = static_cast<size_t>(std::count_if(
			Document.Elements.begin(), Document.Elements.end(),
			[eFamily](const EFFECT_ELEMENT_DESC& Element)
			{
				return Resolve_AuthoringFamily(Element) == eFamily &&
					!Is_ElementPreviewAdmitted(Element);
			}));
		const bool_t bFamilyPreviewAdmitted = iPlayLockedCount < iCount;
		ImGui::PushID(iFamily);
		const std::string FamilyLabel = std::string(
			AuthoringFamily_Label(eFamily)) + " (" + std::to_string(iCount) +
			(iPlayLockedCount > 0u ?
				", play-locked " + std::to_string(iPlayLockedCount) :
				std::string()) + ")";
		const bool_t bFamilyOpen = ImGui::TreeNodeEx(FamilyLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow);
		ImGui::SameLine();
		ImGui::BeginDisabled(!bActive || !bDrawable || !bPreviewReady ||
			!bFamilyPreviewAdmitted);
		if (ImGui::SmallButton("Play Family"))
			Try_PlayUnifiedAuthoringFamily(Document.strEffectAssetId, eFamily);
		ImGui::EndDisabled();
		if (!bActive && ImGui::IsItemHovered(
				ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Load this Effect before playing one Family.");
		}
		else if (!bFamilyPreviewAdmitted && ImGui::IsItemHovered(
				ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"Every Element in this Family is hidden or hard-locked by material/runtime admission. APPROXIMATE Elements remain playable for authoring only; Load/edit/Save remain available.");
		}
		if (bFamilyOpen)
		{
			size_t iOrdinal = 0u;
			for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
			{
				if (Resolve_AuthoringFamily(Element) != eFamily)
					continue;
				++iOrdinal;
				ImGui::PushID(Element.strElementId.c_str());
				const std::string RowLabel = FriendlyAuthoringElementLabel(
					eFamily, iOrdinal, Element);
				ImGui::TextUnformatted(RowLabel.c_str());
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(
						"Stable Element: %s\nFamily Group: %s\n"
						"Start: %.3f s\nSlots: %s",
						Element.strElementId.c_str(),
						Element.strGroupId.empty() ? "(ungrouped)" :
							Element.strGroupId.c_str(),
						Element.Detail.Timing.fStartDelaySeconds,
						AuthoringElementResourceSlotSummary(Element).c_str());
				}
				ImGui::SameLine();
				ImGui::BeginDisabled(nullptr == pEditablePath);
				if (ImGui::SmallButton("Load"))
					Try_LoadUnifiedElement(Cache, Element.strElementId);
				ImGui::EndDisabled();
				if (nullptr == pEditablePath && ImGui::IsItemHovered(
						ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::SetTooltip("%s", strEditableStatus.c_str());
				}
				ImGui::SameLine();
				const bool_t bElementPreviewAdmitted =
					Is_ElementPreviewAdmitted(Element);
				ImGui::BeginDisabled(!bActive || !bPreviewReady ||
					!bElementPreviewAdmitted);
				if (ImGui::SmallButton("Solo"))
				{
					Try_SoloElement(Document.strEffectAssetId,
						Element.strElementId);
				}
				ImGui::EndDisabled();
				if (!bElementPreviewAdmitted && ImGui::IsItemHovered(
						ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::SetTooltip(
						"Solo is play-locked because this Element is hidden or hard-locked by material/runtime admission. APPROXIMATE Elements remain playable for authoring only; Load/edit/Save remain available.");
				}
				if (!bActive && ImGui::IsItemHovered(
						ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::SetTooltip(
						"Load this Effect first; Solo never changes Current Effect.");
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	if (!Document.ModelCues.empty())
	{
		const std::string ModelFamilyLabel = "Model / Summon (" +
			std::to_string(Document.ModelCues.size()) + ")";
		const bool_t bModelOpen = ImGui::TreeNodeEx(ModelFamilyLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow);
		ImGui::SameLine();
		ImGui::BeginDisabled(!bActive || !bDrawable || !bPreviewReady);
		if (ImGui::SmallButton("Play Family##model-cues"))
			Try_PlayUnifiedModelCues(Document.strEffectAssetId);
		ImGui::EndDisabled();
		if (bModelOpen)
		{
			for (size_t iCue = 0u; iCue < Document.ModelCues.size(); ++iCue)
			{
				const EFFECT_MODEL_CUE_DESC& Cue = Document.ModelCues[iCue];
				ImGui::PushID(Cue.strCueId.c_str());
				const std::string RowLabel = FriendlyModelCueLabel(iCue + 1u, Cue);
				ImGui::TextUnformatted(RowLabel.c_str());
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s\n%s", Cue.strModelAssetId.c_str(),
						Cue.strClipName.c_str());
				ImGui::SameLine();
				ImGui::BeginDisabled(nullptr == pEditablePath);
				if (ImGui::SmallButton("Load"))
					Try_LoadUnifiedModelCue(Cache, Cue.strCueId);
				ImGui::EndDisabled();
				if (nullptr == pEditablePath && ImGui::IsItemHovered(
						ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::SetTooltip("%s", strEditableStatus.c_str());
				}
				ImGui::SameLine();
				ImGui::BeginDisabled(!bActive || !bPreviewReady);
				if (ImGui::SmallButton("Solo"))
					Try_SoloModelCue(Document.strEffectAssetId, Cue.strCueId);
				ImGui::EndDisabled();
				if (!bActive && ImGui::IsItemHovered(
						ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::SetTooltip(
						"Load this Effect first; Solo never changes Current Effect.");
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}
	ImGui::TreePop();
	ImGui::PopID();
}

bool_t Client::CEffect_Tool::Render_ManualElementGroups(
    const EFFECT_DOCUMENT_DESC& Document,
    const std::string& strEffectAssetId,
    const bool_t bDefaultOpen)
{
    std::map<std::string, std::vector<const EFFECT_ELEMENT_DESC*>> Groups;
    for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
    {
        if (Is_ManualElementGroupMember(Element))
            Groups[Element.strGroupId].push_back(&Element);
    }
    if (Groups.empty())
        return false;

    ImGui::PushID((strEffectAssetId + ".manual-groups").c_str());
    const std::string RootLabel = "Element Groups (" +
        std::to_string(Groups.size()) + ")";
    const ImGuiTreeNodeFlags RootFlags = ImGuiTreeNodeFlags_OpenOnArrow |
        (bDefaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0);
    if (ImGui::TreeNodeEx(RootLabel.c_str(), RootFlags))
    {
        for (const auto& [strGroupId, Elements] : Groups)
        {
            ImGui::PushID(strGroupId.c_str());
            const bool_t bGroupSelected =
                strGroupId == m_strSelectedElementGroupId;
            const std::string GroupLabel = ManualGroup_Label(strGroupId) +
                " (" + std::to_string(Elements.size()) + ")";
            const ImGuiTreeNodeFlags GroupFlags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_DefaultOpen |
                (bGroupSelected ? ImGuiTreeNodeFlags_Selected : 0);
            const bool_t bGroupOpen =
                ImGui::TreeNodeEx(GroupLabel.c_str(), GroupFlags);
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                m_strSelectedElementGroupId = strGroupId;
            ImGui::SameLine();
            if (ImGui::SmallButton("Play Group"))
            {
				Try_SoloElementGroup(strEffectAssetId, strGroupId);
            }
            if (bGroupOpen)
            {
                for (const EFFECT_ELEMENT_DESC* pElement : Elements)
                {
                    const bool_t bElementSelected =
                        m_ActiveDocument.has_value() &&
                        m_ActiveDocument->strEffectAssetId == strEffectAssetId &&
                        EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
                        pElement->strElementId == m_strSelectedElementId;
                    const std::string ElementLabel =
                        ManualElement_Label(*pElement);
                    const float fElementWidth = (std::max)(1.f,
                        ImGui::GetContentRegionAvail().x - 58.f);
                    if (ImGui::Selectable(
                        ElementLabel.c_str(), bElementSelected, 0,
                        ImVec2(fElementWidth, 0.f)))
                    {
                        Try_SelectElement(
                            strEffectAssetId, pElement->strElementId);
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip(
                            "%s\nClick to edit this Element. Use Solo Group to preview the combined hit.",
                            pElement->strElementId.c_str());
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton(
						("Solo##" + pElement->strElementId).c_str()))
                    {
						Try_SoloElement(
							strEffectAssetId, pElement->strElementId);
                    }
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
    return true;
}

void Client::CEffect_Tool::Render_ActiveAuthoredEffectTree()
{
	ImGui::SeparatorText("Current Effect");
	if (!m_ActiveDocument.has_value() ||
		(EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource &&
		 EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource))
	{
		if (m_ActiveDocument.has_value() &&
			(EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
				m_eActiveDocumentSource ||
			 EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY ==
				m_eActiveDocumentSource ||
			 EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT ==
				m_eActiveDocumentSource ||
			 EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE ==
				m_eActiveDocumentSource))
		{
			ImGui::TextWrapped(
				"Advanced read-only document: %s. It is not Current Effect and cannot overwrite a saved Effect.",
				m_ActiveDocument->strEffectAssetId.c_str());
		}
		else if (m_SourceElementPresetSelection.has_value())
		{
			ImGui::TextWrapped(
				"A Source Element seed is loaded. Create or open Current Effect, then use Create Element and Save Changes.");
		}
		else
		{
			ImGui::TextDisabled(
				"No saved Effect is open. Use Data Files > Load Saved Effect for Editing, or load a Source Element from All Effects.");
		}
		return;
	}
	const std::string CurrentDisplayName = FriendlyDocumentLabel(
		*m_ActiveDocument, "Current Effect");
	ImGui::TextWrapped("Editing and saving: %s", CurrentDisplayName.c_str());
	ImGui::TextDisabled(
		(m_bDocumentDirty || Has_UnappliedDetailDraft()) ?
			"Status: unsaved or unapplied changes. Element row edits; Solo only changes preview." :
			"Status: saved document. Element row edits; Solo only changes preview.");
	ImGui::BeginDisabled(!m_bActiveDocumentDrawable);
	if (ImGui::SmallButton("Play All##active-authored") &&
		Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
	{
		Start_WorldPreviewFromBeginning();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("%zu Elements", m_ActiveDocument->Elements.size());
	ImGui::SameLine();
	/* Marks are keyed by stable Element ID, so drop any that a document
	   reload, rollback or delete removed instead of carrying a stale count. */
	std::erase_if(m_MarkedElementIds,
		[this](const std::string& strElementId)
		{
			return std::none_of(m_ActiveDocument->Elements.begin(),
				m_ActiveDocument->Elements.end(),
				[&strElementId](const EFFECT_ELEMENT_DESC& Element)
				{
					return Element.strElementId == strElementId;
				});
		});
	const bool_t bCanDeleteSelected = !Has_UnappliedDetailDraft() &&
		(!m_MarkedElementIds.empty() ||
			(EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
				!m_strSelectedElementId.empty()));
	const std::string DeleteLabel = m_MarkedElementIds.empty() ?
		std::string("Delete Selected") :
		"Delete " + std::to_string(m_MarkedElementIds.size()) + " Marked";
	ImGui::BeginDisabled(!bCanDeleteSelected);
	if (ImGui::SmallButton(DeleteLabel.c_str()))
		Try_DeleteSelectedElement();
	ImGui::EndDisabled();
	ImGui::SameLine();
	const EFFECT_ELEMENT_DESC* pSelectedForDuplicate = Find_SelectedElement();
	const bool_t bCanDuplicateSelected = !Has_UnappliedDetailDraft() &&
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
		nullptr != pSelectedForDuplicate &&
		AuthoringFamily_CanCreate(
			Resolve_AuthoringFamily(*pSelectedForDuplicate));
	ImGui::BeginDisabled(!bCanDuplicateSelected);
	if (ImGui::SmallButton("Duplicate Selected"))
		Try_DuplicateSelectedElement();
	ImGui::EndDisabled();
	if (!m_MarkedElementIds.empty())
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Clear Marks"))
			m_MarkedElementIds.clear();
	}
	ImGui::TextDisabled(
		"Ctrl or Shift click Element rows to mark several, then Delete; Duplicate copies the open row as one independent occurrence.");
	ImGui::SameLine();
	const EFFECT_ELEMENT_DESC* pSelectedForSeed = Find_SelectedElement();
	const bool_t bCanSeedSelected =
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
		nullptr != pSelectedForSeed && !Has_UnappliedDetailDraft() &&
		AuthoringFamily_CanCreate(
			Resolve_AuthoringFamily(*pSelectedForSeed));
	ImGui::BeginDisabled(!bCanSeedSelected);
	if (ImGui::SmallButton("Use Selected as New Layer Seed"))
		Try_UseSelectedElementAsAuthoringPreset();
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Effect Detail edits one selected Element at a time; all Families and Elements below remain in this one saved Effect.");
	const std::string CurrentRootLabel = CurrentDisplayName +
		(m_bDocumentDirty ? " [UNSAVED]" : "") + "##" +
		m_ActiveDocument->strEffectAssetId;
	if (!ImGui::TreeNodeEx(CurrentRootLabel.c_str(),
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		return;
	}
	for (int32_t iFamily = 0;
		iFamily < static_cast<int32_t>(EFFECT_AUTHORING_FAMILY::END);
		++iFamily)
	{
		const EFFECT_AUTHORING_FAMILY eFamily =
			static_cast<EFFECT_AUTHORING_FAMILY>(iFamily);
		const size_t iCount = static_cast<size_t>(std::count_if(
			m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
			[eFamily](const EFFECT_ELEMENT_DESC& Element)
			{ return Resolve_AuthoringFamily(Element) == eFamily; }));
		if (0u == iCount)
			continue;
		const size_t iPlayLockedCount = static_cast<size_t>(std::count_if(
			m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
			[eFamily](const EFFECT_ELEMENT_DESC& Element)
			{
				return Resolve_AuthoringFamily(Element) == eFamily &&
					!Is_ElementPreviewAdmitted(Element);
			}));
		const bool_t bFamilyPreviewAdmitted = iPlayLockedCount < iCount;
		ImGui::PushID(iFamily);
		const std::string FamilyLabel = std::string(
			AuthoringFamily_Label(eFamily)) + " (" + std::to_string(iCount) +
			(iPlayLockedCount > 0u ?
				", play-locked " + std::to_string(iPlayLockedCount) :
				std::string()) + ")";
		const bool_t bFamilyOpen = ImGui::TreeNodeEx(FamilyLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow);
		ImGui::SameLine();
		ImGui::BeginDisabled(!m_bActiveDocumentDrawable ||
			!bFamilyPreviewAdmitted);
		if (ImGui::SmallButton("Play Family"))
		{
			Try_PlayUnifiedAuthoringFamily(
				m_ActiveDocument->strEffectAssetId, eFamily);
		}
		ImGui::EndDisabled();
		if (!bFamilyPreviewAdmitted && ImGui::IsItemHovered(
				ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"Every Element in this Family is hidden or hard-locked by material/runtime admission. APPROXIMATE Elements remain playable for authoring only; editing and Save remain available.");
		}
		if (bFamilyOpen)
		{
			size_t iOrdinal = 0u;
			for (const EFFECT_ELEMENT_DESC& Element : m_ActiveDocument->Elements)
			{
				if (Resolve_AuthoringFamily(Element) != eFamily)
					continue;
				++iOrdinal;
				ImGui::PushID(Element.strElementId.c_str());
				const bool_t bSelected =
					EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
					m_strSelectedElementId == Element.strElementId;
				const float fRowWidth = (std::max)(1.f,
					ImGui::GetContentRegionAvail().x - 54.f);
				const bool_t bMarked = m_MarkedElementIds.contains(
					Element.strElementId);
				/* The row is where Elements get judged for deletion, so it has
				   to say when what is on screen is the source playing rather
				   than anything the authored values could change. */
				const std::string RowLabel =
					std::string(bMarked ? "[x] " : "") +
					(Element.SourceRecipe.bEnabled ? "(src) " : "") +
					FriendlyAuthoringElementLabel(
						eFamily, iOrdinal, Element);
				if (ImGui::Selectable(RowLabel.c_str(), bSelected || bMarked,
					0, ImVec2(fRowWidth, 0.f)))
				{
					const ImGuiIO& Io = ImGui::GetIO();
					if (Io.KeyCtrl || Io.KeyShift)
					{
						/* Marking never changes the Detail selection, so the
						   open Element keeps its draft while rows are marked. */
						if (!m_MarkedElementIds.insert(
								Element.strElementId).second)
						{
							m_MarkedElementIds.erase(Element.strElementId);
						}
					}
					else
					{
						m_MarkedElementIds.clear();
						Try_SelectElement(m_ActiveDocument->strEffectAssetId,
							Element.strElementId);
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(
						"Stable Element: %s%s",
						Element.strElementId.c_str(),
						Is_ElementPreviewAdmitted(Element) ? "" :
							"\nPlay locked: hidden or hard material/runtime admission; editing and Save remain available.");
				ImGui::SameLine();
				const bool_t bElementPreviewAdmitted =
					Is_ElementPreviewAdmitted(Element);
				ImGui::BeginDisabled(!bElementPreviewAdmitted);
				if (ImGui::SmallButton("Solo"))
				{
					Try_SoloElement(m_ActiveDocument->strEffectAssetId,
						Element.strElementId);
				}
				ImGui::EndDisabled();
				if (!bElementPreviewAdmitted && ImGui::IsItemHovered(
						ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::SetTooltip(
						"Solo is play-locked; select the row to edit and Save this Element.");
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	if (!m_ActiveDocument->ModelCues.empty())
	{
		const bool_t bModelOpen = ImGui::TreeNodeEx(("Model / Summon (" +
			std::to_string(m_ActiveDocument->ModelCues.size()) + ")").c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow);
		ImGui::SameLine();
		ImGui::BeginDisabled(!m_bActiveDocumentDrawable);
		if (ImGui::SmallButton("Play Family##active-model-cues"))
			Try_PlayUnifiedModelCues(m_ActiveDocument->strEffectAssetId);
		ImGui::EndDisabled();
		if (bModelOpen)
		{
			for (size_t iCue = 0u; iCue < m_ActiveDocument->ModelCues.size();
				++iCue)
			{
				const EFFECT_MODEL_CUE_DESC& Cue =
					m_ActiveDocument->ModelCues[iCue];
				ImGui::PushID(Cue.strCueId.c_str());
				const bool_t bSelected =
					EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection &&
					m_strSelectedModelCueId == Cue.strCueId;
				const float fRowWidth = (std::max)(1.f,
					ImGui::GetContentRegionAvail().x - 54.f);
				const std::string RowLabel = FriendlyModelCueLabel(iCue + 1u, Cue);
				if (ImGui::Selectable(RowLabel.c_str(), bSelected, 0,
					ImVec2(fRowWidth, 0.f)))
				{
					Try_SelectModelCue(m_ActiveDocument->strEffectAssetId,
						Cue.strCueId);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s\n%s", Cue.strModelAssetId.c_str(),
						Cue.strClipName.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("Solo"))
					Try_SoloModelCue(m_ActiveDocument->strEffectAssetId,
						Cue.strCueId);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}
	ImGui::TreePop();
}

bool_t Client::CEffect_Tool::Play_ValtanClipOccurrence(
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
{
	return Play_ValtanStageSequence({ Clip });
}

bool_t Client::CEffect_Tool::Play_ValtanProductCue(
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
{
	if (Clip.strClipOccurrenceId.empty() ||
		Cue.strClipOccurrenceId != Clip.strClipOccurrenceId ||
		Cue.strEffectAssetId.empty() || 0u == Cue.iStageDurationMs)
	{
		m_strPreviewAnimationStatus =
			"Valtan Product cue rejected a stale clip/stage occurrence join.";
		return false;
	}
	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds =
		static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = Clip.fPlayRate;
	Timing.fCueSourceStartSeconds =
		static_cast<f32_t>(Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds =
		static_cast<f32_t>(Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd = Cue.bHasSourceEnd;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE InitialSample;
	if (!CActionPresentationTimeline::Resolve_CuePreviewSample(
			Timing, 0.f, InitialSample))
	{
		m_strPreviewAnimationStatus =
			"Valtan Product cue rejected an invalid source-local preview window.";
		return false;
	}
	if (!Play_ValtanClipOccurrence(Clip))
		return false;

	Clear_ProductCuePreview();
	VALTAN_PRODUCT_PREVIEW Preview;
	Preview.Clip = Clip;
	Preview.Cue = Cue;
	m_ValtanProductPreview = std::move(Preview);
	Reset_ProductCueSnapshot();
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_fPreviewTimeSeconds = 0.f;
	Recalculate_PreviewDuration();
	m_strPreviewAnimationStatus =
		"Valtan Product cue animation/effect clocks synced: " +
		Clip.strClipName + " | cue " + Cue.strOccurrenceId + " @ source " +
		std::to_string(Cue.iSourceStartMs) + " ms | stage " +
		std::to_string(Cue.iStageDurationMs) + " ms";
	return true;
}

bool_t Client::CEffect_Tool::Play_ValtanStageSequence(
	const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips)
{
	if (Clips.empty() || std::any_of(Clips.begin(), Clips.end(),
			[](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId.empty() ||
					Clip.strClipName.empty() ||
					!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f;
			}))
	{
		return false;
	}
	if (nullptr == m_pCharacterPreviewPanel ||
		(CAnimationTargetService::Resolve_AssetName() !=
			VALTAN_ANIMATION_ASSET_NAME &&
		 !m_pCharacterPreviewPanel->Select_TargetAsset(
			VALTAN_ANIMATION_ASSET_NAME)))
	{
		m_strPreviewAnimationStatus =
			"Valtan model could not be staged for the ordered clip sequence.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		m_strPreviewAnimationStatus =
			"Valtan model is not staged; the ordered clip sequence was not started.";
		return false;
	}

	std::vector<SYNCHRONIZED_ANIMATION_CLIP> Staged;
	Staged.reserve(Clips.size());
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Source : Clips)
	{
		SYNCHRONIZED_ANIMATION_CLIP Clip;
		Clip.strClipName = Source.strClipName;
		Clip.iPlayMs = Source.iPlayMs;
		Clip.fPlayRate = Source.fPlayRate;
		Clip.iSourceStartMs = Source.iSourceStartMs;
		Clip.bLoop = Source.bLoop;
		Clip.bHasExplicitLoopPolicy = true;
		Staged.push_back(std::move(Clip));
	}
	m_SynchronizedAnimationClips = std::move(Staged);
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	m_iSynchronizedAnimationTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	if (!Start_SynchronizedAnimationClip(0u, false))
	{
		const std::string FailedClip = Clips.front().strClipName;
		Reset_SynchronizedAnimationSequence();
		m_strPreviewAnimationStatus =
			"Valtan clip occurrence could not be started: " + FailedClip;
		return false;
	}
	m_strPreviewAnimationStatus = "Valtan ordered clip sequence synced to the Effect clock: " +
		Clips.front().strClipName + " (1/" +
		std::to_string(Clips.size()) + ")";
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayValtanSavedUnifiedEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
{
	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
		!Cache.bValid || !Cache.bDrawable)
	{
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}
	const bool_t bTargetReady = Is_UnifiedEffectActive(Cache) ?
		Play_ValtanProductCue(Clip, Cue) :
		Try_OpenValtanAuthoredEffect(
			Path, strEffectAssetId, Clip, Cue, true);
	return bTargetReady && Try_PlayUnifiedEffect(Cache);
}

bool_t Client::CEffect_Tool::Try_OpenValtanSavedReferenceEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips,
	const bool_t bQueuePlayCompleteAfterLoad)
{
	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
		!Cache.bValid || !Cache.bDrawable)
	{
		/* A retired evidence shell must not replace the Current Effect merely
		   because its historical authored file still exists. */
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}
	const bool_t bAlreadyActive = m_ActiveDocument.has_value() &&
		m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
		m_ActiveDocument->strEffectAssetId == strEffectAssetId;
	if (!bAlreadyActive && !Try_LoadDocumentPath(
			Path, EFFECT_DOCUMENT_SOURCE::AUTHORED, strEffectAssetId))
	{
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == Path)
		{
			m_PendingDocumentLoad->ValtanReferenceClips = Clips;
			m_PendingDocumentLoad->ValtanClip.reset();
			m_PendingDocumentLoad->ValtanCue.reset();
			m_PendingDocumentLoad->bPlayCompleteAfterLoad =
				bQueuePlayCompleteAfterLoad;
		}
		return false;
	}
	if (!Clips.empty())
	{
		if (!Play_ValtanStageSequence(Clips))
			return false;
	}
	else if (nullptr == m_pCharacterPreviewPanel ||
		(CAnimationTargetService::Resolve_AssetName() !=
			VALTAN_ANIMATION_ASSET_NAME &&
		 !m_pCharacterPreviewPanel->Select_TargetAsset(
			VALTAN_ANIMATION_ASSET_NAME)))
	{
		m_strPreviewAnimationStatus =
			"Valtan model could not be staged for this saved Effect.";
		return false;
	}
	if (bQueuePlayCompleteAfterLoad && !Try_PlayActiveUnifiedEffect())
		return false;
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenValtanAuthoredEffect(
	const std::filesystem::path& Path,
	const std::string& strEffectAssetId,
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
	const bool_t bQueuePlayCompleteAfterLoad)
{
	UNIFIED_EFFECT_CACHE& Cache =
		m_ValtanUnifiedEffectCaches[strEffectAssetId];
	if (!Refresh_UnifiedEffectCache(Cache, Path, strEffectAssetId) ||
		!Cache.bValid)
	{
		m_strPreviewStatus = Cache.strStatus;
		return false;
	}
	if (!Try_LoadDocumentPath(
			Path, EFFECT_DOCUMENT_SOURCE::AUTHORED, strEffectAssetId))
	{
		if (m_PendingDocumentLoad.has_value() &&
			m_PendingDocumentLoad->Path == Path)
		{
			m_PendingDocumentLoad->ValtanClip = Clip;
			m_PendingDocumentLoad->ValtanCue = Cue;
			m_PendingDocumentLoad->bPlayCompleteAfterLoad =
				bQueuePlayCompleteAfterLoad;
		}
		return false;
	}
	/* Do not switch the Model View target before the unsaved-document guard
	   decides whether this load will commit.  Play_ValtanProductCue owns the
	   target stage after a successful load, so Cancel preserves the previous
	   Character Product preview exactly. */
	const bool_t bAnimationReady = Play_ValtanProductCue(Clip, Cue);
	if (bAnimationReady && !Cache.bDrawable)
	{
		m_strPreviewStatus =
			"Opened an empty Product Effect for authoring; create its first Element before Play.";
	}
	return bAnimationReady;
}

bool_t Client::CEffect_Tool::Refresh_ValtanPatternTree()
{
	/* parse -> validate -> stage -> commit. A failed reload keeps whatever the
	   window is already showing so a transient read error never empties it. */
	VALTAN_PATTERN_TREE_VIEW Staged;
	std::string Status;
	if (!CValtanPatternTree::Load(Staged, Status))
	{
		m_strValtanPatternTreeStatus = m_bValtanPatternTreeLoaded ?
			("Valtan tree reload preserved the previous tree: " + Status) :
			Status;
		return false;
	}
	m_ValtanPatternTree = std::move(Staged);
	m_bValtanPatternTreeLoaded = true;
	m_strValtanPatternTreeStatus = Status;
	/* Explicit Refresh is also the retry boundary for a repaired authored
	   document that was previously observed as invalid or non-drawable. */
	m_ValtanUnifiedEffectCaches.clear();
	return true;
}

bool_t Client::CEffect_Tool::Matches_ValtanPatternSearch(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::string& strSearch) const
{
	if (strSearch.empty())
		return true;
	if (Contains_NoCase(Pattern.strPatternId, strSearch) ||
		Contains_NoCase(Pattern.strDisplayName, strSearch) ||
		Contains_NoCase(Pattern.strActionId, strSearch))
	{
		return true;
	}
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		if (Contains_NoCase(Stage.strStageId, strSearch) ||
			Contains_NoCase(Stage.strActionId, strSearch) ||
			Contains_NoCase(Stage.strHitShape, strSearch))
		{
			return true;
		}
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
			Stage.ClipOccurrences)
		{
			if (Contains_NoCase(Clip.strClipOccurrenceId, strSearch) ||
				Contains_NoCase(Clip.strClipName, strSearch) ||
				Contains_NoCase(Clip.strMappingBasis, strSearch))
			{
				return true;
			}
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				Clip.ProductCues)
			{
				if (Contains_NoCase(Cue.strBindingId, strSearch) ||
					Contains_NoCase(Cue.strOccurrenceId, strSearch) ||
					Contains_NoCase(Cue.strEffectAssetId, strSearch) ||
					Contains_NoCase(Cue.strV1EffectAssetId, strSearch))
				{
					return true;
				}
			}
		}
		for (const VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
		{
			if (Contains_NoCase(Effect.strEffectAssetId, strSearch))
				return true;
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect :
			Stage.CombatObjectEffects)
		{
			if (Contains_NoCase(Effect.strCombatObjectArchetypeId, strSearch) ||
				Contains_NoCase(Effect.strClientVisualId, strSearch) ||
				Contains_NoCase(Effect.strEffectAssetId, strSearch))
			{
				return true;
			}
		}
	}
	return false;
}

void Client::CEffect_Tool::Render_ValtanStageRow(
	const VALTAN_STAGE_VIEW& Stage)
{
	/* One line states what the Server does in this window, because that is the
	   window the Effect has to fill. The numbers are read, never written. */
	std::string Shape = Stage.strHitShape.empty() ? "NONE" : Stage.strHitShape;
	if (Stage.Has_HitShape())
	{
		char_t Detail[128]{};
		if ("CIRCLE" == Stage.strHitShape)
			sprintf_s(Detail, " r=%.1f", Stage.fHitOuterRadius);
		else if ("RING" == Stage.strHitShape)
			sprintf_s(Detail, " in=%.1f out=%.1f",
				Stage.fHitInnerRadius, Stage.fHitOuterRadius);
		else if ("CONE" == Stage.strHitShape)
			sprintf_s(Detail, " %.0fdeg L=%.1f",
				Stage.fHitAngleDegrees, Stage.fHitLength);
		else if ("BOX" == Stage.strHitShape || "CROSS" == Stage.strHitShape ||
			"SIX_DIRECTIONS" == Stage.strHitShape)
			sprintf_s(Detail, " L=%.1f W=%.1f",
				Stage.fHitLength, Stage.fHitHalfWidth);
		Shape += Detail;
		if (Stage.iHitCount > 1u)
		{
			Shape += " x" + std::to_string(Stage.iHitCount) + " @";
			if (!Stage.HitOffsetsMs.empty())
			{
				Shape += "[";
				for (size_t iOffset = 0u;
					iOffset < Stage.HitOffsetsMs.size(); ++iOffset)
				{
					if (iOffset > 0u)
						Shape += ",";
					Shape += std::to_string(Stage.HitOffsetsMs[iOffset]);
				}
				Shape += "]ms";
			}
			else
			{
				Shape += std::to_string(Stage.iHitDelayMs) + "+k*" +
					std::to_string(Stage.iHitIntervalMs) + "ms";
			}
		}
	}

	ImGui::PushID(Stage.strActionId.c_str());
	const std::string StageLabel = Stage.strStageId + " | " +
		Stage.strStageKind + " | " + std::to_string(Stage.iDurationMs) +
		" ms | " + Shape + " | " +
		std::to_string(Stage.ClipOccurrences.size()) + " clips | " +
		std::to_string(Stage.ProductCues.size()) + " cues | " +
		std::to_string(Stage.CombatObjectEffects.size()) + " moving fx";
	const bool_t bStageOpen = ImGui::TreeNodeEx(StageLabel.c_str(),
		ImGuiTreeNodeFlags_OpenOnArrow);
	if (bStageOpen)
	{
		ImGui::TextDisabled("action %s", Stage.strActionId.c_str());
		if (!Stage.ClipOccurrences.empty())
		{
			ImGui::SameLine();
			if (ImGui::SmallButton("Replay Sequence"))
				Play_ValtanStageSequence(Stage.ClipOccurrences);
		}
		if (!Stage.strServerDamageProfileId.empty())
		{
			ImGui::TextDisabled("Server damage %s",
				Stage.strServerDamageProfileId.c_str());
		}

		for (size_t iClip = 0u; iClip < Stage.ClipOccurrences.size(); ++iClip)
		{
			Render_ValtanClipOccurrence(
				Stage, Stage.ClipOccurrences[iClip], iClip + 1u);
		}
		if (Stage.ClipOccurrences.empty())
			ImGui::TextDisabled("(no ordered animation clip occurrence)");

		if (!Stage.CombatObjectEffects.empty())
		{
			ImGui::SeparatorText("World-root Effect Occurrences");
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect :
				Stage.CombatObjectEffects)
			{
				ImGui::PushID(Effect.strCombatObjectArchetypeId.c_str());
				ImGui::TextDisabled("%s | %s | %s x%u | %s",
					Effect.strCombatObjectArchetypeId.c_str(),
					Effect.strClientVisualId.c_str(),
					Effect.strTrigger.c_str(), Effect.iSpawnValue,
					Effect.strEffectAssetId.c_str());
				ImGui::PopID();
			}
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Client::CEffect_Tool::Render_ValtanClipOccurrence(
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const size_t iClipOrdinal)
{
	ImGui::PushID(Clip.strClipOccurrenceId.c_str());
	std::string Label = "Clip " + std::to_string(iClipOrdinal) + " | " +
		Clip.strClipName + " | " + Clip.strClipOccurrenceId + " | " +
		std::to_string(Clip.ProductCues.size()) + " cues";
	if (Clip.iSourceStartMs > 0u || Clip.iPlayMs > 0u ||
		std::abs(Clip.fPlayRate - 1.f) > 0.0001f || Clip.bLoop)
	{
		Label += " | src+" + std::to_string(Clip.iSourceStartMs) +
			" play=" + (0u == Clip.iPlayMs ? std::string("natural") :
				std::to_string(Clip.iPlayMs) + "ms") +
			" rate=" + std::to_string(Clip.fPlayRate) +
			(Clip.bLoop ? " loop" : "");
	}
	const bool_t bOpen = ImGui::TreeNodeEx(
		Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
	ImGui::SameLine();
	if (ImGui::SmallButton("Replay"))
		Play_ValtanClipOccurrence(Clip);
	if (bOpen)
	{
		ImGui::TextDisabled("mapping %s | stage wall %u ms",
			Clip.strMappingBasis.empty() ? "(unspecified)" :
				Clip.strMappingBasis.c_str(),
			Stage.iDurationMs);
		for (size_t iCue = 0u; iCue < Clip.ProductCues.size(); ++iCue)
		{
			Render_ValtanProductCue(
				Clip, Clip.ProductCues[iCue], iCue + 1u);
		}
		if (Clip.ProductCues.empty())
			ImGui::TextDisabled("(no Product cue occurrence)");
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Client::CEffect_Tool::Render_ValtanProductCue(
	const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
	const size_t iCueOrdinal)
{
	(void)Clip;
	ImGui::PushID(Cue.strOccurrenceId.c_str());
	const std::string SourceWindow = Cue.bHasSourceEnd ?
		(std::to_string(Cue.iSourceStartMs) + "-" +
		 std::to_string(Cue.iSourceEndMs) + " ms") :
		(std::to_string(Cue.iSourceStartMs) + "-natural");
	const std::string Label = "Cue " + std::to_string(iCueOrdinal) + " | " +
		Cue.strOccurrenceId + " | src " + SourceWindow + " | " +
		Cue.strEffectAssetId;
	const bool_t bOpen = ImGui::TreeNodeEx(
		Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
	if (!bOpen)
	{
		ImGui::PopID();
		return;
	}

	ImGui::TextDisabled("binding %s | anchor %s | %s / %s | repeat %s | stage %u ms | mapped %zu times",
		Cue.strBindingId.c_str(), Cue.strAnchorSlotId.c_str(),
		Cue.strFollowPolicy.c_str(), Cue.strStopPolicy.c_str(),
		Cue.strRepeatPolicy.c_str(), Cue.iStageDurationMs,
		Count_ProductCueMappings(Cue.strEffectAssetId));
	ImGui::TextDisabled(
		"Open/Play controls are listed once in Saved Unified Effects above.");
	ImGui::TreePop();
	ImGui::PopID();
}

bool_t Client::CEffect_Tool::Can_PlayValtanServerPattern(
	const VALTAN_PATTERN_VIEW& Pattern,
	std::string& strOutReason) const
{
	strOutReason.clear();
	const uint32_t iCurrentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (nullptr == Resolve_ValtanServerPatternBossPlacement(iCurrentLevel))
	{
		strOutReason =
			"Server Pattern Play is available only in Character Select or Valtan Arena.";
		return false;
	}
	if (!CNetworkManager::Get().Is_Connected())
	{
		strOutReason = "Start and connect the Debug Server first.";
		return false;
	}
	if (m_PendingValtanServerPatternRequest.Is_Active())
	{
		strOutReason = "Waiting for the Server verdict for " +
			m_PendingValtanServerPatternRequest.strPatternId + ".";
		return false;
	}
	if (ETOUI(LEVEL::CHARACTER_SELECT) == iCurrentLevel &&
		Is_ValtanCharacterSelectEnvironmentPattern(Pattern.strPatternId))
	{
		strOutReason =
			"This pattern owns Valtan Arena walls, floor, pillars, or opening state; play it in Valtan Arena.";
		return false;
	}
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayValtanServerPattern(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	std::string strReason;
	if (!Can_PlayValtanServerPattern(Pattern, strReason))
	{
		m_strValtanServerPatternStatusPatternId = Pattern.strPatternId;
		m_strValtanServerPatternStatus = std::move(strReason);
		return false;
	}

	const char_t* pBossPlacementId = Resolve_ValtanServerPatternBossPlacement(
		CGameInstance::Get().Get_CurrentLevelID());
	if (nullptr == pBossPlacementId)
		return false;
	const uint32_t iSequence =
		0u == m_iNextValtanServerPatternRequestSequence ?
			1u : m_iNextValtanServerPatternRequestSequence;
	if (!CNetworkManager::Get().Send_ValtanPatternAuditionById(
			iSequence, pBossPlacementId, Pattern.strPatternId))
	{
		m_strValtanServerPatternStatusPatternId = Pattern.strPatternId;
		m_strValtanServerPatternStatus =
			"Could not send Server Pattern Play; check the Server connection.";
		return false;
	}

	m_iNextValtanServerPatternRequestSequence =
		(std::numeric_limits<uint32_t>::max)() == iSequence ?
			1u : iSequence + 1u;
	m_PendingValtanServerPatternRequest.iSequence = iSequence;
	m_PendingValtanServerPatternRequest.iWorldInboundGeneration =
		CNetworkManager::Get().Get_WorldInboundGeneration();
	m_PendingValtanServerPatternRequest.strBossPlacementId = pBossPlacementId;
	m_PendingValtanServerPatternRequest.strPatternId = Pattern.strPatternId;
	m_strValtanServerPatternStatusPatternId = Pattern.strPatternId;
	m_strValtanServerPatternStatus =
		"Waiting for the Server to reset the replicated Valtan and queue this pattern...";
	return true;
}

void Client::CEffect_Tool::Update_ValtanServerPatternAudition()
{
	using namespace LostArk::Shared;
	/* A world reset clears the NetworkManager verdict queue.  Reject the old
	   transaction before draining so a late verdict that arrives in the new
	   generation cannot be mistaken for the old world's exact request. */
	if (m_PendingValtanServerPatternRequest.Is_Active() &&
		m_PendingValtanServerPatternRequest.iWorldInboundGeneration !=
			CNetworkManager::Get().Get_WorldInboundGeneration())
	{
		m_strValtanServerPatternStatusPatternId =
			m_PendingValtanServerPatternRequest.strPatternId;
		m_strValtanServerPatternStatus =
			"Pattern Play was cancelled because the Server world session changed.";
		m_PendingValtanServerPatternRequest = {};
	}

	S2C_VALTAN_AUDITION_RESULT Result{};
	while (CNetworkManager::Get().Try_Consume_ValtanPatternAuditionByIdResult(
			Result))
	{
		if (!m_PendingValtanServerPatternRequest.Is_Active())
			continue;
		if (VALTAN_AUDITION_OPERATION::PLAY_PATTERN_ID != Result.eOperation ||
			Result.iRequestSequence !=
				m_PendingValtanServerPatternRequest.iSequence ||
			Result.strBossPlacementId !=
				m_PendingValtanServerPatternRequest.strBossPlacementId ||
			Result.strPatternId !=
				m_PendingValtanServerPatternRequest.strPatternId)
		{
			m_strValtanServerPatternStatusPatternId =
				m_PendingValtanServerPatternRequest.strPatternId;
			m_strValtanServerPatternStatus =
				"Ignored a mismatched Server verdict; waiting for the exact request identity.";
			continue;
		}

		const VALTAN_SERVER_PATTERN_REQUEST Completed =
			m_PendingValtanServerPatternRequest;
		m_PendingValtanServerPatternRequest = {};
		m_strValtanServerPatternStatusPatternId = Completed.strPatternId;
		switch (Result.eResult)
		{
		case VALTAN_AUDITION_RESULT::QUEUED:
			m_strValtanServerPatternStatus =
				"Server queued the full fixed-tick pattern. Watch the replicated Valtan animation, motion, cues, and Effects.";
			break;
		case VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED:
			m_strValtanServerPatternStatus =
				"The Server already handled this request sequence; no duplicate pattern was queued.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS:
			m_strValtanServerPatternStatus =
				"No replicated Valtan exists at " + Completed.strBossPlacementId +
				". Spawn Valtan first, then press Play Server Pattern again.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED:
			m_strValtanServerPatternStatus =
				"The Server needs a living combat-ready player near Valtan; move into engage range and retry.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD:
			m_strValtanServerPatternStatus =
				"The replicated Valtan is dead; respawn or re-enter before replaying a pattern.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE:
			m_strValtanServerPatternStatus =
				"The Server rejected this pattern for the current world or boss state.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR:
			m_strValtanServerPatternStatus =
				"The Server encounter does not own this stable pattern ID.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD:
			m_strValtanServerPatternStatus =
				"The active Server room does not match this Effect Tool pattern request.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD:
			m_strValtanServerPatternStatus =
				"Server Pattern Play is Debug-only; start the Debug Server.";
			break;
		case VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED:
			m_strValtanServerPatternStatus =
				"The Server returned an unexpected health-bar audition state for this stable-ID request.";
			break;
		case VALTAN_AUDITION_RESULT::ARMED:
			m_strValtanServerPatternStatus =
				"The Server armed the request but did not queue the selected pattern.";
			break;
		case VALTAN_AUDITION_RESULT::END:
		default:
			m_strValtanServerPatternStatus =
				"The Server returned an unknown Pattern Play verdict.";
			break;
		}
	}

	if (m_PendingValtanServerPatternRequest.Is_Active() &&
		!CNetworkManager::Get().Is_Connected())
	{
		m_strValtanServerPatternStatusPatternId =
			m_PendingValtanServerPatternRequest.strPatternId;
		m_strValtanServerPatternStatus =
			"The Server disconnected before answering Pattern Play.";
		m_PendingValtanServerPatternRequest = {};
	}
}

void Client::CEffect_Tool::Render_ValtanPatternNode(
	const VALTAN_PATTERN_VIEW& Pattern,
	const char_t* pGroupLabel,
	const std::string& strSearch)
{
	if (!Matches_ValtanPatternSearch(Pattern, strSearch))
		return;

	struct SAVED_VALTAN_PRODUCT_SOURCE final
	{
		const VALTAN_STAGE_VIEW* pStage = nullptr;
		const VALTAN_CLIP_OCCURRENCE_VIEW* pClip = nullptr;
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pCue = nullptr;
	};
	struct SAVED_VALTAN_EFFECT_ROW final
	{
		std::string strEffectAssetId;
		std::filesystem::path Path;
		std::vector<SAVED_VALTAN_PRODUCT_SOURCE> ProductSources;
		std::vector<const VALTAN_STAGE_VIEW*> ReferenceStages;
		std::vector<const VALTAN_STAGE_VIEW*> CombatObjectStages;
		bool_t bV1Alias = false;
	};

	std::vector<SAVED_VALTAN_EFFECT_ROW> SavedRows;
	std::unordered_map<std::string, size_t> SavedRowIndices;
	const auto ResolveRow = [&SavedRows, &SavedRowIndices](
		const std::string& strEffectAssetId) -> SAVED_VALTAN_EFFECT_ROW&
	{
		const auto [Found, bInserted] = SavedRowIndices.try_emplace(
			strEffectAssetId, SavedRows.size());
		if (bInserted)
		{
			SAVED_VALTAN_EFFECT_ROW Row;
			Row.strEffectAssetId = strEffectAssetId;
			SavedRows.push_back(std::move(Row));
		}
		return SavedRows[Found->second];
	};
	const auto AppendStageOnce = [](auto& Stages,
		const VALTAN_STAGE_VIEW& Stage)
	{
		if (std::find(Stages.begin(), Stages.end(), &Stage) == Stages.end())
			Stages.push_back(&Stage);
	};
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
			Stage.ClipOccurrences)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				Clip.ProductCues)
			{
				SAVED_VALTAN_EFFECT_ROW& Row = ResolveRow(Cue.strEffectAssetId);
				Row.ProductSources.push_back({ &Stage, &Clip, &Cue });
				if (!Cue.strV1EffectAssetId.empty())
				{
					SAVED_VALTAN_EFFECT_ROW& V1Row =
						ResolveRow(Cue.strV1EffectAssetId);
					V1Row.ProductSources.push_back({ &Stage, &Clip, &Cue });
					V1Row.bV1Alias = true;
				}
			}
		}
		for (const VALTAN_STAGE_EFFECT_VIEW& Effect : Stage.Effects)
		{
			SAVED_VALTAN_EFFECT_ROW& Row = ResolveRow(Effect.strEffectAssetId);
			if (Row.Path.empty() && !Effect.DocumentPath.empty())
				Row.Path = Effect.DocumentPath;
			AppendStageOnce(Row.ReferenceStages, Stage);
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect :
			Stage.CombatObjectEffects)
		{
			SAVED_VALTAN_EFFECT_ROW& Row = ResolveRow(Effect.strEffectAssetId);
			AppendStageOnce(Row.CombatObjectStages, Stage);
		}
	}
	for (SAVED_VALTAN_EFFECT_ROW& Row : SavedRows)
	{
		const auto Editable = m_DirectAuthoredEditableEntries.find(
			Row.strEffectAssetId);
		if (Editable != m_DirectAuthoredEditableEntries.end())
		{
			/* The catalog path is authoritative. A legacy patterneffects row may
			   carry a Data-prefixed path that is only a reference fallback. */
			Row.Path = Editable->second.Path;
		}
	}
	/* Saved Unified Effects is the active authoring surface, not a list of
	   retired naming-rule shells. Product cues and Server-owned world visuals
	   stay here; reference-only documents remain available through Data Files. */
	std::erase_if(SavedRows,
		[](const SAVED_VALTAN_EFFECT_ROW& Row)
		{
			return Row.ProductSources.empty() &&
				Row.CombatObjectStages.empty();
		});

	std::string Label = "Pattern | " + std::string(pGroupLabel) + " | " +
		Pattern.strPatternId;
	if (!Pattern.strDisplayName.empty())
		Label += " | " + Pattern.strDisplayName;
	Label += " | Saved " + std::to_string(SavedRows.size());
	ImGui::PushID(Pattern.strPatternId.c_str());
	/* A search that matched deeper than the pattern row should not make the
	   person expand every node by hand. */
	if (!strSearch.empty())
		ImGui::SetNextItemOpen(true, ImGuiCond_Always);
	const bool_t bPatternOpen =
		ImGui::TreeNodeEx(Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
	ImGui::SameLine();
	std::string strServerPlayReason;
	const bool_t bCanPlayServerPattern =
		Can_PlayValtanServerPattern(Pattern, strServerPlayReason);
	ImGui::BeginDisabled(!bCanPlayServerPattern);
	if (ImGui::SmallButton("Play Server Pattern"))
		Try_PlayValtanServerPattern(Pattern);
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("%s", bCanPlayServerPattern ?
			"Reset and run this complete pattern on the replicated Valtan through the Server fixed-tick path." :
			strServerPlayReason.c_str());
	}
	if (m_strValtanServerPatternStatusPatternId == Pattern.strPatternId &&
		!m_strValtanServerPatternStatus.empty())
	{
		ImGui::Indent();
		ImGui::TextWrapped("Server Pattern: %s",
			m_strValtanServerPatternStatus.c_str());
		ImGui::Unindent();
	}
	if (bPatternOpen)
	{
		ImGui::SeparatorText("Saved Unified Effects");
		if (SavedRows.empty())
		{
			ImGui::TextDisabled(
				"No saved unified Effect is mapped to this pattern.");
		}
		for (const SAVED_VALTAN_EFFECT_ROW& Row : SavedRows)
		{
			std::vector<const VALTAN_STAGE_VIEW*> NonProductStages =
				Row.ReferenceStages;
			for (const VALTAN_STAGE_VIEW* pStage : Row.CombatObjectStages)
			{
				if (std::find(NonProductStages.begin(), NonProductStages.end(),
						pStage) == NonProductStages.end())
				{
					NonProductStages.push_back(pStage);
				}
			}
			const SAVED_VALTAN_PRODUCT_SOURCE* pProductSource =
				1u == Row.ProductSources.size() ?
					&Row.ProductSources.front() : nullptr;
			const VALTAN_STAGE_VIEW* pNonProductStage =
				Row.ProductSources.empty() && 1u == NonProductStages.size() ?
					NonProductStages.front() : nullptr;
			const bool_t bAmbiguousOccurrence =
				1u < Row.ProductSources.size() ||
				(Row.ProductSources.empty() && 1u < NonProductStages.size());
			const bool_t bHasExactPlaybackOwner =
				nullptr != pProductSource || nullptr != pNonProductStage;
			const bool_t bActive = m_ActiveDocument.has_value() &&
				m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
				m_ActiveDocument->strEffectAssetId == Row.strEffectAssetId;
			const auto ObservedCache = m_ValtanUnifiedEffectCaches.find(
				Row.strEffectAssetId);
			const bool_t bKnownInvalid =
				ObservedCache != m_ValtanUnifiedEffectCaches.end() &&
				ObservedCache->second.bObserved &&
				!ObservedCache->second.bValid;
			const bool_t bKnownNonDrawable =
				ObservedCache != m_ValtanUnifiedEffectCaches.end() &&
				ObservedCache->second.bObserved &&
				ObservedCache->second.bValid &&
				!ObservedCache->second.bDrawable;
			const std::string SavedLabel =
				(Row.bV1Alias ? "[V1] " :
					(Row.ProductSources.empty() ? "[WORLD] " : "[PRODUCT] ")) +
				Row.strEffectAssetId;
			ImGui::PushID(Row.strEffectAssetId.c_str());
			const bool_t bSavedOpen = ImGui::TreeNodeEx(
				SavedLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow |
					(bActive ? ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", Row.Path.empty() ?
					"Saved Effect source path is unavailable." :
					Row.Path.generic_string().c_str());
			}
			if (bSavedOpen)
			{
				if (Row.bV1Alias)
				{
					ImGui::TextColored(ImVec4(0.67f, 0.86f, 1.f, 1.f),
						"Optional V1 alias; Product clip, timing, transform, and attachment are shared with V0.");
				}
				if (!Row.ProductSources.empty())
				{
					if (1u == Row.ProductSources.size())
					{
						ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
							"Active Product cue uses this saved unified Effect.");
					}
					else
					{
						ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
							"Product cue provenance: %zu occurrences.",
							Row.ProductSources.size());
					}
					for (const SAVED_VALTAN_PRODUCT_SOURCE& Source :
						Row.ProductSources)
					{
						ImGui::TextDisabled("Stage %s | Clip %s | Cue %s",
							Source.pStage->strStageId.c_str(),
							Source.pClip->strClipName.c_str(),
							Source.pCue->strOccurrenceId.c_str());
					}
				}
				if (!Row.ReferenceStages.empty())
				{
					ImGui::TextDisabled(
						"Stage reference provenance: %zu owner stage(s).",
						Row.ReferenceStages.size());
					for (const VALTAN_STAGE_VIEW* pStage : Row.ReferenceStages)
					{
						ImGui::TextDisabled("Stage %s | ordered clips %zu",
							pStage->strStageId.c_str(),
							pStage->ClipOccurrences.size());
					}
				}
				if (!Row.CombatObjectStages.empty())
				{
					ImGui::TextDisabled(
						"World-root visual provenance: %zu owner stage(s); Tool preview still replays the owner animation.",
						Row.CombatObjectStages.size());
				}
				if (bAmbiguousOccurrence)
				{
					ImGui::TextDisabled(
						"Multiple exact owners exist; Open/Play is disabled until an occurrence selector is authored.");
				}
				ImGui::TextWrapped("Path: %s", Row.Path.empty() ?
					"(unavailable)" : Row.Path.generic_string().c_str());

				/* A structurally valid empty Product document must still open so
				   its first Element can be authored. Play remains fail-closed
				   below until the document becomes drawable. */
				ImGui::BeginDisabled(bActive || Row.Path.empty() ||
					!bHasExactPlaybackOwner || bAmbiguousOccurrence ||
					bKnownInvalid);
				if (ImGui::SmallButton("Open Saved Effect"))
				{
					if (nullptr != pProductSource)
					{
						VALTAN_PRODUCT_EFFECT_CUE_VIEW PlaybackCue =
							*pProductSource->pCue;
						PlaybackCue.strEffectAssetId = Row.strEffectAssetId;
						Try_OpenValtanAuthoredEffect(Row.Path,
							Row.strEffectAssetId, *pProductSource->pClip,
							PlaybackCue);
					}
					else
					{
						Try_OpenValtanSavedReferenceEffect(Row.Path,
							Row.strEffectAssetId,
							pNonProductStage->ClipOccurrences);
					}
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::BeginDisabled(Row.Path.empty() ||
					!bHasExactPlaybackOwner || bAmbiguousOccurrence ||
					bKnownInvalid || bKnownNonDrawable);
				if (ImGui::SmallButton("Play Saved Effect"))
				{
					if (nullptr != pProductSource)
					{
						VALTAN_PRODUCT_EFFECT_CUE_VIEW PlaybackCue =
							*pProductSource->pCue;
						PlaybackCue.strEffectAssetId = Row.strEffectAssetId;
						Try_PlayValtanSavedUnifiedEffect(Row.Path,
							Row.strEffectAssetId, *pProductSource->pClip,
							PlaybackCue);
					}
					else
					{
						Try_OpenValtanSavedReferenceEffect(Row.Path,
							Row.strEffectAssetId,
							pNonProductStage->ClipOccurrences, true);
					}
				}
				ImGui::EndDisabled();
				/* Open/Play may insert and rehash the unordered cache, so never
				   retain its iterator across either button action. */
				const auto RefreshedCache = m_ValtanUnifiedEffectCaches.find(
					Row.strEffectAssetId);
				if (RefreshedCache != m_ValtanUnifiedEffectCaches.end() &&
					RefreshedCache->second.bObserved &&
					!RefreshedCache->second.strStatus.empty())
				{
					ImGui::TextWrapped("Last explicit validation: %s",
						RefreshedCache->second.strStatus.c_str());
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		ImGui::SeparatorText("Animations / Semantic Stages");
		for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			Render_ValtanStageRow(Stage);
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Client::CEffect_Tool::Render_ValtanPatternTreeSection(
	const std::string& strSearch)
{
	if (!m_bValtanPatternTreeLoaded)
		Refresh_ValtanPatternTree();
	if (!m_strValtanPatternTreeStatus.empty())
		ImGui::TextDisabled("%s", m_strValtanPatternTreeStatus.c_str());
	if (m_ValtanPatternTree.Gimmicks.empty() &&
		m_ValtanPatternTree.Rotation.empty())
	{
		ImGui::TextDisabled(
			"No Valtan pattern was staged; press Refresh for the reason.");
		return;
	}

	if (VALTAN_PHASE_VIEW::INVALID_INDEX !=
		m_ValtanPatternTree.iIntroRotationIndex)
	{
		Render_ValtanPatternNode(
			m_ValtanPatternTree.Rotation[
				m_ValtanPatternTree.iIntroRotationIndex],
			"Intro", strSearch);
	}
	for (const VALTAN_PATTERN_VIEW& Pattern : m_ValtanPatternTree.Gimmicks)
		Render_ValtanPatternNode(Pattern, "Gimmick", strSearch);
	for (size_t iRotation = 0u;
		iRotation < m_ValtanPatternTree.Rotation.size(); ++iRotation)
	{
		if (iRotation == m_ValtanPatternTree.iIntroRotationIndex)
			continue;
		Render_ValtanPatternNode(
			m_ValtanPatternTree.Rotation[iRotation], "Rotation", strSearch);
	}
}

void Client::CEffect_Tool::Render_AllEffectsWindow()
{
	ImGui::SetNextWindowPos(ImVec2(1110.f, 705.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(620.f, 560.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(
		ImVec2(540.f, 500.f), ImVec2(2200.f, 2200.f));
	if (!ImGui::Begin("All Effects"))
	{
		ImGui::End();
		return;
	}

	{
	if (m_bAllEffectsValtanBossSelected)
	{
		ImGui::TextWrapped(
			"Top Pattern: Play Server Pattern resets and runs the complete pattern on the replicated Valtan through the Server fixed-tick path.");
		ImGui::TextDisabled(
			"Saved Effect, Stage Replay Sequence, and Product Play buttons below remain local Model View authoring previews.");
	}
	else
	{
		ImGui::TextDisabled(
			"Saved unified Effects come from EffectCatalog.json; files are parsed only when Open or Play is pressed.");
		if (!m_strUnifiedCandidateStatus.empty())
			ImGui::TextWrapped("%s", m_strUnifiedCandidateStatus.c_str());
	}
	const char_t* pAllEffectsOwnerLabel =
		m_bAllEffectsValtanBossSelected ?
			"Valtan" : Class_Label(m_eAllEffectsClass);
	if (ImGui::BeginCombo("Character / Boss", pAllEffectsOwnerLabel))
	{
		for (const EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTION& Owner :
			EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS)
		{
			const bool_t bBossOwner =
				EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::VALTAN_BOSS ==
					Owner.eKind;
			if (bBossOwner)
				ImGui::SeparatorText("Boss Patterns");
			const bool_t bSelected = bBossOwner ?
				m_bAllEffectsValtanBossSelected :
				(!m_bAllEffectsValtanBossSelected &&
					Owner.eCharacterClass == m_eAllEffectsClass);
			if (ImGui::Selectable(Owner.strLabel.data(), bSelected))
			{
				m_bAllEffectsValtanBossSelected = bBossOwner;
				if (!bBossOwner)
				{
					m_eAllEffectsClass = Owner.eCharacterClass;
					Select_AuthoringDomainForClass(Owner.eCharacterClass);
				}
			}
		}
		ImGui::EndCombo();
	}
	ImGui::InputTextWithHint("##effect-search",
		"Search skill, pattern, Product cue, or saved Effect ID...",
		m_AllEffectsSearch.data(), m_AllEffectsSearch.size());
	ImGui::SameLine();
	if (ImGui::SmallButton("Refresh"))
	{
		Refresh_AllEffects(true);
		Refresh_DataFiles();
		Refresh_ValtanPatternTree();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Hide Preview"))
		Hide_WorldPreview();

	const std::string Search = m_AllEffectsSearch.data();
	const f32_t fStatusReserve = m_strElementStatus.empty() ? 1.f :
		ImGui::CalcTextSize(m_strElementStatus.c_str(), nullptr, false,
			ImGui::GetContentRegionAvail().x).y +
		ImGui::GetStyle().ItemSpacing.y;
	ImGui::BeginChild("ElementFirstEffectTree",
		ImVec2(0.f, -fStatusReserve), true);

	if (m_bAllEffectsValtanBossSelected)
	{
		Render_ValtanPatternTreeSection(Search);
	}
	else
	{
		const std::vector<PLAYER_SKILL_DEFINITION>& Skills =
			CPlayerSkillCatalog::Get_Skills();
		static const std::vector<EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE>
			EmptyProductCues;
		for (const PLAYER_SKILL_DEFINITION& Skill : Skills)
		{
			if (Skill.eCharacterClass != m_eAllEffectsClass)
				continue;
			const auto ProductEntry = std::find_if(
				m_AllEffects.begin(), m_AllEffects.end(),
				[&Skill](const EFFECT_SKILL_TREE_ENTRY& Entry)
				{
					return Entry.Skill.eCharacterClass ==
							Skill.eCharacterClass &&
						Entry.Skill.iSkillId == Skill.iSkillId;
				});
			const EFFECT_SKILL_TREE_ENTRY* pProductEntry =
				ProductEntry == m_AllEffects.end() ? nullptr : &*ProductEntry;
			const std::vector<EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE>& ProductCues =
				nullptr == pProductEntry ?
					EmptyProductCues : pProductEntry->ProductCues;
			std::vector<const UNIFIED_EFFECT_CANDIDATE_BINDING*> SavedBindings;
			for (const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding :
				m_UnifiedCandidateBindings)
			{
				if (Binding.eCharacterClass == Skill.eCharacterClass &&
					Binding.iSkillId == Skill.iSkillId)
				{
					SavedBindings.push_back(&Binding);
				}
			}
			const bool_t bArtistFSkill =
				Skill.eCharacterClass ==
					LostArk::Shared::CHARACTER_CLASS_ID::ARTIST &&
				Skill.iSkillId == ARTIST_F_CORE_SKILL_ID;
			const std::shared_ptr<const EFFECT_VISUAL_PROGRAM>
				pArtistFToolProgram = bArtistFSkill ?
					CEffectCatalog::Find_VisualProgram(
						ARTIST_F_VISUAL_PROGRAM_ASSET_ID) : nullptr;
			const bool_t bArtistFToolAdapter =
				nullptr != pArtistFToolProgram &&
				pArtistFToolProgram->eProjectionKind ==
					EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
			const bool_t bSavedMatchesSearch = std::any_of(
				SavedBindings.begin(), SavedBindings.end(),
				[&Search](const UNIFIED_EFFECT_CANDIDATE_BINDING* pBinding)
				{
					return nullptr != pBinding &&
						Contains_NoCase(pBinding->strEffectAssetId, Search);
				});
			const bool_t bCueMatchesSearch = std::any_of(
				ProductCues.begin(), ProductCues.end(),
				[&Search](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Cue)
				{
					if (Contains_NoCase(Cue.Cue.strClipName, Search) ||
						Contains_NoCase(Cue.Cue.strEffectAssetId, Search))
					{
						return true;
					}
					const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
						CEffectCatalog::Find_VisualProgram(
							Cue.Cue.strEffectAssetId);
					if (nullptr == Program)
						return false;
					const auto ResourcesMatch = [&Search](const auto& Row)
					{
						return std::any_of(Row.Resources.begin(),
							Row.Resources.end(), [&Search](const auto& Resource)
							{
								return Contains_NoCase(
									Resource.strAssetId, Search) ||
									Contains_NoCase(
										Resource.strSlotId, Search);
							});
					};
					return std::any_of(Program->VisualRows.begin(),
						Program->VisualRows.end(), ResourcesMatch) ||
						std::any_of(Program->SupplementalElements.begin(),
							Program->SupplementalElements.end(), ResourcesMatch);
				}) || (bArtistFToolAdapter &&
				(Contains_NoCase(ARTIST_F_VISUAL_PROGRAM_ASSET_ID, Search) ||
				 std::any_of(pArtistFToolProgram->VisualRows.begin(),
					pArtistFToolProgram->VisualRows.end(),
					[&Search](const EFFECT_VISUAL_PROGRAM_ROW& Row)
					{
						return std::any_of(Row.Resources.begin(),
							Row.Resources.end(),
							[&Search](const auto& Resource)
							{
								return Contains_NoCase(
									Resource.strAssetId, Search) ||
									Contains_NoCase(
										Resource.strSlotId, Search);
							});
					})));
			if (!Contains_NoCase(Skill.strInputSlot, Search) &&
				!Contains_NoCase(Skill.strDisplayName, Search) &&
				!Contains_NoCase(Skill.strEffectId, Search) &&
				!bCueMatchesSearch && !bSavedMatchesSearch)
			{
				continue;
			}

			ImGui::PushID(static_cast<int>(Skill.iSkillId));
			const std::string SkillLabel = "Skill | Input " +
				Skill.strInputSlot + " | " + Skill.strDisplayName +
				Tool_SkillIdentitySuffix(Skill) + " | Saved " +
				std::to_string(SavedBindings.size());
			if (ImGui::TreeNodeEx(SkillLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow))
			{
				const bool_t bHoldPhaseFamily =
					Skill.eSkillKind == LostArk::Shared::PLAYER_SKILL_KIND::HOLD &&
					1u < Skill.iComboStageCount &&
					ProductCues.size() == Skill.iComboStageCount &&
					[&ProductCues, &Skill]()
					{
						for (size_t iStage = 0u;
							iStage < Skill.iComboStageCount; ++iStage)
						{
							if (1u != std::ranges::count_if(ProductCues,
								[iStage](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Cue)
								{
									return Cue.iStageIndex == iStage;
								}))
							{
								return false;
							}
						}
						return true;
					}();
				const auto BuildPhaseLabel = [&Skill](
					const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Cue,
					const bool_t bIncludeClip)
				{
					const EFFECT_TOOL_SKILL_PHASE_ROLE eRole =
						Resolve_EffectToolSkillPhaseRole(Skill.eSkillKind,
							Cue.iStageIndex, Skill.iComboStageCount);
					std::string Label(EffectToolSkillPhaseRoleLabel(eRole));
					if (EFFECT_TOOL_SKILL_PHASE_ROLE::BASIC_ATTACK == eRole ||
						EFFECT_TOOL_SKILL_PHASE_ROLE::STAGE == eRole)
					{
						Label += " " + std::to_string(Cue.iStageIndex + 1u);
					}
					if (bIncludeClip && !Cue.Cue.strClipName.empty())
						Label += " | " + Cue.Cue.strClipName;
					return Label;
				};
				if (bHoldPhaseFamily)
				{
					ImGui::TextDisabled(
						"One HOLD family: Start / Charge / Release. Product documents remain phase-local because the Server owns release timing.");
				}
				if (!SavedBindings.empty())
				{
					ImGui::SeparatorText(bHoldPhaseFamily ?
						"Saved HOLD Phase Documents" : "Saved Unified Effects");
					for (const UNIFIED_EFFECT_CANDIDATE_BINDING* pBinding :
						SavedBindings)
					{
						if (nullptr == pBinding)
							continue;
						const auto ExactProduct = std::find_if(
							ProductCues.begin(), ProductCues.end(),
							[pBinding](
								const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Cue)
							{
								return Cue.Cue.strEffectAssetId ==
									pBinding->strEffectAssetId;
							});
						const bool_t bExactProduct =
							ExactProduct != ProductCues.end();
						const auto LegacyProduct = bExactProduct ?
							ProductCues.end() :
							std::find_if(ProductCues.begin(), ProductCues.end(),
								[pBinding](
									const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Cue)
								{
									return Unified_CandidateAssetId(
										Cue.Cue.strEffectAssetId) ==
										pBinding->strEffectAssetId;
								});
						const bool_t bLegacyProduct =
							LegacyProduct != ProductCues.end();
						const auto MatchedProduct = bExactProduct ?
							ExactProduct : LegacyProduct;
						const bool_t bActive =
							m_ActiveDocument.has_value() &&
							m_eActiveDocumentSource ==
								EFFECT_DOCUMENT_SOURCE::AUTHORED &&
							m_ActiveDocument->strEffectAssetId ==
								pBinding->strEffectAssetId;
						ImGui::PushID(pBinding->strEffectAssetId.c_str());
						const std::string SavedTreeLabel =
							bHoldPhaseFamily && MatchedProduct != ProductCues.end() ?
								BuildPhaseLabel(*MatchedProduct, false) +
									"##" + pBinding->strEffectAssetId :
								pBinding->strEffectAssetId;
						const bool_t bSavedOpen = ImGui::TreeNodeEx(
							SavedTreeLabel.c_str(),
							ImGuiTreeNodeFlags_OpenOnArrow |
								(bActive ? ImGuiTreeNodeFlags_Selected : 0));
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("%s",
								pBinding->Path.generic_string().c_str());
						}
						if (bSavedOpen)
						{
							if (bHoldPhaseFamily)
							{
								ImGui::TextWrapped("Stable Product ID: %s",
									pBinding->strEffectAssetId.c_str());
							}
							if (bExactProduct)
							{
								ImGui::TextColored(
									ImVec4(0.36f, 0.72f, 1.f, 1.f),
									"Active Product cue uses this saved unified Effect.");
							}
							else if (bLegacyProduct)
							{
								ImGui::TextDisabled(
									"Saved authored source for a matching Legacy Product cue; publish remains separate.");
							}
							else
							{
								ImGui::TextDisabled(
									"Saved authored source; no active Product cue mapping is inferred.");
							}
							ImGui::TextWrapped("Path: %s",
								pBinding->Path.generic_string().c_str());
							ImGui::BeginDisabled(bActive);
							if (ImGui::SmallButton("Open Saved Effect"))
							{
								std::string strEditableStatus;
								const std::filesystem::path* pEditablePath =
									Resolve_DirectAuthoredEditablePath(
										pBinding->strEffectAssetId,
										strEditableStatus);
								if (nullptr == pEditablePath)
									m_strElementStatus = strEditableStatus;
								else
									Try_LoadDocumentPath(*pEditablePath,
										EFFECT_DOCUMENT_SOURCE::AUTHORED,
										pBinding->strEffectAssetId);
							}
							ImGui::EndDisabled();
							ImGui::SameLine();
							if (ImGui::SmallButton("Play Saved Effect"))
								Try_PlaySavedUnifiedEffect(*pBinding);
							const auto Cache = m_UnifiedCandidateCaches.find(
								pBinding->strEffectAssetId);
							if (Cache != m_UnifiedCandidateCaches.end() &&
								Cache->second.bObserved &&
								!Cache->second.strStatus.empty())
							{
								ImGui::TextWrapped("Last explicit validation: %s",
									Cache->second.strStatus.c_str());
							}
							ImGui::TreePop();
						}
						ImGui::PopID();
					}
				}
				if (bArtistFSkill && bArtistFToolAdapter)
					Render_ArtistFCoreAuthoring();
				if (ProductCues.empty() && !bArtistFToolAdapter &&
					SavedBindings.empty())
				{
					ImGui::TextDisabled(
						"No saved authored or playable Product Effect is mapped to this skill.");
				}
				if (!bArtistFSkill && nullptr != pProductEntry)
				{
					const auto RenderProductCue = [this, pProductEntry,
						&ProductCues, &SavedBindings,
						&BuildPhaseLabel](const size_t iCue)
					{
						const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Cue =
							ProductCues[iCue];
						const bool_t bMultipleStageClips = std::any_of(
							ProductCues.begin(), ProductCues.end(),
							[&Cue](
								const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Other)
							{
								return Other.iStageIndex == Cue.iStageIndex &&
									Other.iStageClipIndex != Cue.iStageClipIndex;
							});
						std::string StageLabel = BuildPhaseLabel(Cue, true);
						if (bMultipleStageClips)
						{
							StageLabel += " / Clip " +
								std::to_string(Cue.iStageClipIndex + 1u);
						}
						StageLabel += "##" + std::to_string(iCue);
						ImGui::PushID(static_cast<int>(iCue));
						if (ImGui::TreeNodeEx(StageLabel.c_str(),
							ImGuiTreeNodeFlags_OpenOnArrow))
						{
							/* A Product cue is the primary authoring entry point.  Keep
							   its exact direct-authored document and family tree beside
							   Play Full Effect so Q/W/E/R/T and BA phases never require
							   discovering a second, detached Saved list. */
							const auto AuthoredBinding = std::find_if(
								SavedBindings.begin(), SavedBindings.end(),
								[&Cue](
									const UNIFIED_EFFECT_CANDIDATE_BINDING* pBinding)
								{
									return nullptr != pBinding &&
										pBinding->strEffectAssetId ==
											Cue.Cue.strEffectAssetId;
								});
							if (AuthoredBinding != SavedBindings.end())
							{
								const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding =
									**AuthoredBinding;
								auto Cache = m_UnifiedCandidateCaches.find(
									Binding.strEffectAssetId);
								if (Cache != m_UnifiedCandidateCaches.end() &&
									Refresh_UnifiedEffectCache(Cache->second,
										Binding.Path, Binding.strEffectAssetId) &&
									Cache->second.bValid)
								{
									Render_UnifiedEffectTree(Cache->second,
										"Editable Unified Effect | " +
											Binding.strEffectAssetId);
								}
								else
								{
									ImGui::TextDisabled(
										"Open Editor is unavailable: %s",
										Cache == m_UnifiedCandidateCaches.end() ?
											"the authored cache lost this Product ID." :
											Cache->second.strStatus.c_str());
								}
							}
							else
							{
								ImGui::TextDisabled(
									"Open Editor is unavailable: this Product cue has no exact direct-authored document.");
							}
							if (ImGui::Button("Play Full Effect"))
								Try_SelectProductCue(*pProductEntry, iCue);
							Render_VisualProgramAuthoring(*pProductEntry, iCue);
							if (nullptr == CEffectCatalog::Find_VisualProgram(
									Cue.Cue.strEffectAssetId))
							{
								ImGui::TextDisabled(
									"No Track A Family Elements are available for this Effect.");
							}
							ImGui::TreePop();
						}
						ImGui::PopID();
					};
					if (bHoldPhaseFamily)
					{
						ImGui::SeparatorText("HOLD Product Family");
						const std::string FamilyLabel = "Start / Charge / Release (" +
							std::to_string(ProductCues.size()) +
							" phase cues)##hold-product-family";
						if (ImGui::TreeNodeEx(FamilyLabel.c_str(),
							ImGuiTreeNodeFlags_DefaultOpen |
								ImGuiTreeNodeFlags_OpenOnArrow))
						{
							for (size_t iCue = 0u; iCue < ProductCues.size(); ++iCue)
								RenderProductCue(iCue);
							ImGui::TreePop();
						}
					}
					else
					{
						for (size_t iCue = 0u; iCue < ProductCues.size(); ++iCue)
							RenderProductCue(iCue);
					}
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	if (!m_strElementStatus.empty())
		ImGui::TextWrapped("%s", m_strElementStatus.c_str());
	if (!m_bAllEffectsValtanBossSelected &&
		ImGui::CollapsingHeader("Advanced Diagnostics"))
	{
		ImGui::TextDisabled(
			"Product cue diagnostics are optional annotations; the saved source list does not depend on them.");
		for (const EFFECT_SKILL_TREE_ENTRY& Entry : m_AllEffects)
		{
			if (Entry.Skill.eCharacterClass != m_eAllEffectsClass)
				continue;
			ImGui::PushID(static_cast<int>(Entry.Skill.iSkillId));
			const std::string DiagnosticLabel = Entry.Skill.strInputSlot +
				" | " + Entry.Skill.strDisplayName + " | source refs " +
				std::to_string(Entry.iSourceReferenceCount);
			if (ImGui::TreeNode(DiagnosticLabel.c_str()))
			{
				ImGui::TextWrapped("Skill Effect ID: %s",
					Entry.Skill.strEffectId.c_str());
				for (const auto& Cue : Entry.ProductCues)
				{
					ImGui::BulletText("%s | %s | %u ms | anchor=%s",
						Cue.Cue.strEffectAssetId.c_str(),
						Cue.Cue.strClipName.c_str(), Cue.Cue.iStartMs,
						Cue.Cue.strAnchorSlotId.c_str());
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
	return;
	}
	ImGui::BeginDisabled(!m_ActiveDocument.has_value());
    if (ImGui::Button("Play Active Document") &&
        Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
    {
        Start_WorldPreviewFromBeginning();
    }
    ImGui::SameLine();
	if (ImGui::Button("Play Mesh Particles") &&
		Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS))
	{
		Start_WorldPreviewFromBeginning();
	}
	ImGui::SameLine();
    if (ImGui::Button("Hide Preview"))
        Hide_WorldPreview();
    ImGui::EndDisabled();
    if (ImGui::CollapsingHeader("Editing Commands (Advanced)"))
    {
        if (ImGui::Button("Delete Selected Element"))
            Try_DeleteSelectedElement();
        ImGui::SameLine();
        if (ImGui::Button("Clear Active Document"))
            ImGui::OpenPopup("Confirm Clear All Elements");
    }
    if (ImGui::BeginPopupModal(
        "Confirm Clear All Elements", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        const size_t elementCount = m_ActiveDocument.has_value() ?
            m_ActiveDocument->Elements.size() : 0u;
        ImGui::Text("Delete all %zu Elements from the active draft?", elementCount);
        if (ImGui::Button("Clear All Elements"))
        {
            if (Try_ClearElements())
                ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
	if (!ImGui::CollapsingHeader("2. Source Presets"))
	{
		ImGui::TextDisabled(
			"Open Source Presets to choose a Product Cue family Element, then click its row for Effect Details or Solo for preview only.");
		if (!m_strElementStatus.empty())
			ImGui::TextWrapped("%s", m_strElementStatus.c_str());
		ImGui::End();
		return;
	}
	ImGui::TextDisabled(
		"Choose Class / Skill / Product Cue, open a family, then click an Element row. Use Selected Element as Preset creates the editable starting copy.");
	if (ImGui::BeginCombo("Class", Class_Label(m_eAllEffectsClass)))
	{
		for (const EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTION& Owner :
			EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS)
		{
			if (Owner.eKind !=
				EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS)
			{
				continue;
			}
			const auto eClass = Owner.eCharacterClass;
			if (ImGui::Selectable(Class_Label(eClass),
				eClass == m_eAllEffectsClass))
			{
				m_eAllEffectsClass = eClass;
				Select_AuthoringDomainForClass(eClass);
			}
		}
		ImGui::EndCombo();
	}
	ImGui::InputText("Search Source Presets", m_AllEffectsSearch.data(),
		m_AllEffectsSearch.size());
	if (ImGui::SmallButton("Refresh Source Presets"))
	{
		Refresh_AllEffects(true);
	}
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Product cues are usable presets; Source/Imported package rows remain diagnostics.");
    const f32_t fStatusReserve = m_strElementStatus.empty() ? 1.f :
        ImGui::CalcTextSize(
            m_strElementStatus.c_str(), nullptr, false,
            ImGui::GetContentRegionAvail().x).y +
            ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild(
        "AllEffectsTree", ImVec2(0.f, -fStatusReserve), true);
    const std::string Search = m_AllEffectsSearch.data();
    bool_t bActiveAppearsInTree = false;
    for (const EFFECT_SKILL_TREE_ENTRY& Entry : m_AllEffects)
    {
		const bool_t bProductMatchesSearch = std::any_of(
            Entry.ProductCues.begin(), Entry.ProductCues.end(),
            [&Search](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue)
            {
                return Contains_NoCase(
                    ProductCue.Cue.strEffectAssetId, Search) ||
                    Contains_NoCase(ProductCue.Cue.strClipName, Search) ||
                    Contains_NoCase(ProductCue.Cue.strAnchorSlotId, Search);
            });
		const bool_t bArtistFRestore =
			Entry.Skill.eCharacterClass ==
				LostArk::Shared::CHARACTER_CLASS_ID::ARTIST &&
			Entry.Skill.iSkillId == ARTIST_F_CORE_SKILL_ID;
		const bool_t bRestoreMatchesSearch = bArtistFRestore &&
			Contains_NoCase(
					"Core F 33 MeshParticle SpriteParticle LocalDecal CascadeRibbon Diagnostics",
				Search);
		if (Entry.Skill.eCharacterClass != m_eAllEffectsClass ||
			(!Contains_NoCase(Entry.Skill.strInputSlot, Search) &&
			 !Contains_NoCase(Entry.Skill.strDisplayName, Search) &&
			 !Contains_NoCase(Entry.Skill.strEffectId, Search) &&
			 !bProductMatchesSearch && !bRestoreMatchesSearch))
			continue;

		if (bArtistFRestore)
		{
			ImGui::PushID("artist-f-original-restore");
			const ImGuiTreeNodeFlags RestoreFlags =
				(m_bReconstructedSourceRuntimeActive ?
					ImGuiTreeNodeFlags_Selected : 0);
			const bool_t bRestoreOpen = ImGui::TreeNodeEx(
				"Advanced Diagnostics | Skill F | Core F (33)",
				RestoreFlags);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Plays all 33 core renderer occurrences through the shared runtime.\n"
					"PointLight/ScreenPost stay deferred; final visual approval remains manual.");
			}
			if (bRestoreOpen)
			{
				ImGui::TextWrapped(
					"Core scope: MeshParticle 13 | SpriteParticle 16 | LocalDecal 3 | CascadeRibbon 1.");
				ImGui::TextDisabled(
					"NonProduct preview: PointLight #34 and ScreenPost #32 are not included.");
				if (ImGui::Button("Play Core F (33)##artist-f-core"))
				{
					Try_StartArtist31470FullPreview();
				}
				ImGui::SameLine();
				if (ImGui::Button("All 33##artist-f-isolation"))
					Try_ResetArtist31470PreviewIsolation();
				if (ImGui::Button("MeshParticle 13##artist-f-isolation"))
				{
					Try_SetArtist31470PreviewFamilyIsolation(
						EFFECT_GPU_RENDER_FAMILY::MESH);
				}
				ImGui::SameLine();
				if (ImGui::Button("SpriteParticle 16##artist-f-isolation"))
				{
					Try_SetArtist31470PreviewFamilyIsolation(
						EFFECT_GPU_RENDER_FAMILY::SPRITE);
				}
				if (ImGui::Button("LocalDecal 3##artist-f-isolation"))
				{
					Try_SetArtist31470PreviewFamilyIsolation(
						EFFECT_GPU_RENDER_FAMILY::DECAL);
				}
				ImGui::SameLine();
				if (ImGui::Button("CascadeRibbon 1##artist-f-isolation"))
				{
					Try_SetArtist31470PreviewFamilyIsolation(
						EFFECT_GPU_RENDER_FAMILY::RIBBON);
				}
				ImGui::TextWrapped("Core F (33) preview: %s",
					m_strPreviewStatus.empty() ?
						"not staged" : m_strPreviewStatus.c_str());
				if (const shared_ptr<CEffectObject> pDiagnostic =
					m_pWorldPreviewObject.lock();
					m_bReconstructedSourceRuntimeActive && nullptr != pDiagnostic)
				{
					ImGui::TextDisabled(
						"Runtime: %s", pDiagnostic->Get_Status().c_str());
					const shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
						pProgram = pDiagnostic->Get_ReconstructedRuntimeProgram();
					if (m_bReconstructedSourceRuntimeActive && nullptr != pProgram &&
						ImGui::TreeNode("Stable occurrences (grouped by runtime family)"))
					{
						static constexpr std::array<EFFECT_GPU_RENDER_FAMILY, 4u>
							CORE_FAMILIES = {
								EFFECT_GPU_RENDER_FAMILY::MESH,
								EFFECT_GPU_RENDER_FAMILY::SPRITE,
								EFFECT_GPU_RENDER_FAMILY::DECAL,
								EFFECT_GPU_RENDER_FAMILY::RIBBON
							};
						for (const EFFECT_GPU_RENDER_FAMILY eFamily : CORE_FAMILIES)
						{
							ImGui::PushID(static_cast<int>(eFamily));
							if (ImGui::TreeNode(ArtistCoreFamilyLabel(eFamily)))
							{
								for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter :
									pProgram->Emitters)
								{
									EFFECT_GPU_RENDER_FAMILY eEmitterFamily =
										EFFECT_GPU_RENDER_FAMILY::END;
									if (!Emitter.bVisible ||
										!Try_ResolveArtistCoreFamily(
											Emitter.eRenderer, eEmitterFamily) ||
										eEmitterFamily != eFamily)
									{
										continue;
									}
									ImGui::PushID(Emitter.Row.strId.c_str());
									const bool_t bSelected =
										EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE ==
											m_eDetailSelection &&
										m_strSelectedRuntimeOccurrenceEffectId ==
											pProgram->strRuntimeCatalogAssetId &&
										m_strSelectedRuntimeOccurrenceId == Emitter.Row.strId;
									const std::string Label = "#" +
										std::to_string(Emitter.Row.iOrder) + " | " +
										Emitter.strSourceElementId;
									if (ImGui::Selectable(Label.c_str(), bSelected))
									{
										Try_SelectRuntimeOccurrence(
											pProgram->strRuntimeCatalogAssetId, Emitter);
									}
									const bool_t bOccurrenceHovered =
										ImGui::IsItemHovered();
									ImGui::SameLine();
									if (ImGui::SmallButton("Solo"))
									{
										Try_SetVisualPreviewOccurrenceIsolation(
											Emitter.strSourceElementId);
									}
									if (bOccurrenceHovered)
									{
										ImGui::SetTooltip(
											"Program row: %s\nMaterial occurrence: %s\nSource emitter: %s",
											Emitter.Row.strId.c_str(),
											Emitter.strMaterialOccurrenceId.has_value() ?
												Emitter.strMaterialOccurrenceId->c_str() : "none",
											Emitter.strSourceEmitterPath.c_str());
									}
									ImGui::PopID();
								}
								ImGui::TreePop();
							}
							ImGui::PopID();
						}
						ImGui::TreePop();
					}
				}
				if (!Entry.ProductCues.empty())
					Render_VisualProgramAuthoring(Entry, 0u);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		const bool_t bSelectedProductSkill = m_ProductPreview.has_value() &&
            m_ProductPreview->eCharacterClass ==
                Entry.Skill.eCharacterClass &&
            m_ProductPreview->iSkillId == Entry.Skill.iSkillId;
        size_t iProductCueIndex = 0u;
        if (bSelectedProductSkill)
        {
            const auto SelectedCue = std::find_if(
                Entry.ProductCues.begin(), Entry.ProductCues.end(),
                [this](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Candidate)
                {
                    const ANIMATION_EFFECT_CUE& Left = Candidate.Cue;
                    const ANIMATION_EFFECT_CUE& Right =
                        m_ProductPreview->ProductCue.Cue;
                    return Left.strEffectAssetId == Right.strEffectAssetId &&
                        Left.strClipName == Right.strClipName &&
                        Left.iStartMs == Right.iStartMs &&
                        Left.strAnchorSlotId == Right.strAnchorSlotId;
                });
            if (SelectedCue != Entry.ProductCues.end())
            {
                iProductCueIndex = static_cast<size_t>(
                    std::distance(Entry.ProductCues.begin(), SelectedCue));
            }
        }
        const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE* pProductCue =
            Entry.ProductCues.empty() ? nullptr :
                &Entry.ProductCues[iProductCueIndex];
        const std::string strProductEffectAssetId =
            nullptr == pProductCue ? std::string{} :
                pProductCue->Cue.strEffectAssetId;
        const shared_ptr<const EFFECT_DOCUMENT_DESC> pIndexedRuntime =
            strProductEffectAssetId.empty() ? nullptr :
                CEffectCatalog::Find_Loaded(strProductEffectAssetId);
        const bool_t bActiveProductDocument =
            bSelectedProductSkill && m_ActiveDocument.has_value() &&
            m_ActiveDocument->strEffectAssetId == strProductEffectAssetId;
        if (m_ActiveDocument.has_value() && std::any_of(
            Entry.ProductCues.begin(), Entry.ProductCues.end(),
            [this](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue)
            {
                return ProductCue.Cue.strEffectAssetId ==
                    m_ActiveDocument->strEffectAssetId;
            }))
        {
            bActiveAppearsInTree = true;
        }
        const EFFECT_DOCUMENT_DESC* pTreeDocument =
            bActiveProductDocument ? &*m_ActiveDocument :
                pIndexedRuntime.get();
        const std::string strTreeEffectAssetId =
            nullptr == pTreeDocument ? strProductEffectAssetId :
                pTreeDocument->strEffectAssetId;
        const PARTICLE_LAYER_SUMMARY ParticleSummary =
            nullptr == pTreeDocument ? PARTICLE_LAYER_SUMMARY{} :
                Summarize_ParticleLayers(*pTreeDocument);
        ImGui::PushID(static_cast<int32_t>(Entry.Skill.iSkillId));
		const bool_t bSkillSelected = bActiveProductDocument &&
			EFFECT_DETAIL_SELECTION::SKILL == m_eDetailSelection;
        const std::string SkillLabel = "Skill | " + Entry.Skill.strInputSlot +
			" | " + Entry.Skill.strDisplayName +
			Tool_SkillIdentitySuffix(Entry.Skill) +
            (Entry.ProductCues.empty() ?
                " | [Active Product Cue missing]" :
                (Entry.ProductCues.size() == 1u ?
                    " | Product: " + strProductEffectAssetId :
                    " | Product Cues: " +
                        std::to_string(Entry.ProductCues.size()))) +
			(bActiveProductDocument ? " [loaded]" : "");
        const ImGuiTreeNodeFlags SkillFlags =
            ImGuiTreeNodeFlags_OpenOnArrow |
			(bSelectedProductSkill ? ImGuiTreeNodeFlags_DefaultOpen : 0) |
			(bSkillSelected ? ImGuiTreeNodeFlags_Selected : 0);
        const bool_t bSkillOpen = ImGui::TreeNodeEx(
            SkillLabel.c_str(), SkillFlags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            if (Entry.ProductCues.empty())
            {
                m_strElementStatus =
                    "Active Product Cue missing; Source/Imported rows are diagnostic only.";
            }
            else
                Try_SelectProductCue(Entry, iProductCueIndex);
        }
        if (ImGui::IsItemHovered())
        {
            if (nullptr != pTreeDocument)
            {
                ImGui::SetTooltip(
                    "Product cue target: %s\n"
                    "Clip %s @ %u ms | Anchor %s | %s | %s\n"
                    "%zu Elements in the indexed Authored Product.\n"
                    "Standalone Mesh %zu | Mesh Particle %zu\n"
                    "Standalone Sprite %zu | Sprite Particle %zu\n"
                    "Cascade System: Source Systems %zu | Emitters %zu | Layers %zu\n"
                    "Unresolved Particle %zu | Budget %llu\n"
                    "Click the skill label to Product Play the admitted cue.",
                    strProductEffectAssetId.c_str(),
                    pProductCue->Cue.strClipName.c_str(),
                    pProductCue->Cue.iStartMs,
                    pProductCue->Cue.strAnchorSlotId.c_str(),
                    EFFECT_FOLLOW_POLICY::FOLLOW ==
                        pProductCue->Cue.eFollowPolicy ? "follow" : "snapshot",
					EFFECT_ORIENTATION_POLICY::ANCHOR ==
						pProductCue->Cue.eOrientationPolicy ?
						"anchor orientation" : "action facing",
                    pTreeDocument->Elements.size(),
                    ParticleSummary.iStandaloneMeshCount,
                    ParticleSummary.iMeshRendererCount,
                    ParticleSummary.iStandaloneSpriteCount,
                    ParticleSummary.iSpriteRendererCount,
                    ParticleSummary.iSourceSystemCount,
                    ParticleSummary.iSourceEmitterCount,
                    ParticleSummary.iLayerCount,
                    ParticleSummary.iUnresolvedRendererCount,
                    static_cast<unsigned long long>(
                        ParticleSummary.iParticleBudget));
            }
            else
            {
                ImGui::SetTooltip(
                    Entry.ProductCues.empty() ?
                    "Active Product Cue missing. Source/Imported EFFECT rows are reference-only." :
                    "The admitted Product cue target has no loaded Runtime snapshot or Authored document.");
            }
        }
        if (bSkillOpen)
        {
			if (m_DirectAuthoredEditableEntries.contains(
					strProductEffectAssetId))
			{
				std::string strEditableStatus;
				const std::filesystem::path* pEditablePath =
					Resolve_DirectAuthoredEditablePath(
						strProductEffectAssetId, strEditableStatus);
				const bool_t bEditableAuthoredActive =
					m_ActiveDocument.has_value() &&
					m_eActiveDocumentSource ==
						EFFECT_DOCUMENT_SOURCE::AUTHORED &&
					m_ActiveDocument->strEffectAssetId ==
						strProductEffectAssetId;
				ImGui::BeginDisabled(
					bEditableAuthoredActive || nullptr == pEditablePath);
				if (ImGui::Button("Open Saved Authored for Editing") &&
					nullptr != pEditablePath)
				{
					Try_LoadDocumentPath(*pEditablePath,
						EFFECT_DOCUMENT_SOURCE::AUTHORED,
						strProductEffectAssetId);
				}
				ImGui::EndDisabled();
				if (ImGui::IsItemHovered(
						ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::SetTooltip("%s", bEditableAuthoredActive ?
						"This direct authored document is already the Current Effect." :
						strEditableStatus.c_str());
				}
				ImGui::SameLine();
			}
			ImGui::BeginDisabled(Entry.ProductCues.empty());
			if (ImGui::Button(bActiveProductDocument ?
				"Replay Active Product Cue" : "Product Play"))
				Try_SelectProductCue(Entry, iProductCueIndex);
			ImGui::EndDisabled();
            if (Entry.ProductCues.empty())
                ImGui::TextDisabled("Active Product Cue missing (fail-closed).");

            if (!Entry.ProductCues.empty() && ImGui::TreeNode((
                "Product Cues (" +
                std::to_string(Entry.ProductCues.size()) + ")").c_str()))
            {
                for (size_t iCue = 0u; iCue < Entry.ProductCues.size(); ++iCue)
                {
                    const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
                        Entry.ProductCues[iCue];
                    const bool_t bCueSelected = bSelectedProductSkill &&
                        iCue == iProductCueIndex;
                    const std::string CueLabel =
                        ProductCue.Cue.strClipName + " @ " +
                        std::to_string(ProductCue.Cue.iStartMs) + " ms | " +
                        ProductCue.Cue.strEffectAssetId + "##product-cue-" +
                        std::to_string(iCue);
                    if (ImGui::Selectable(CueLabel.c_str(), bCueSelected))
                        Try_SelectProductCue(Entry, iCue);
                    ImGui::TextDisabled(
                        "Anchor %s | %s | %s | %s",
                        ProductCue.Cue.strAnchorSlotId.c_str(),
                        EFFECT_FOLLOW_POLICY::FOLLOW ==
                            ProductCue.Cue.eFollowPolicy ? "follow" : "snapshot",
						EFFECT_ORIENTATION_POLICY::ANCHOR ==
							ProductCue.Cue.eOrientationPolicy ?
							"anchor orientation" : "action facing",
                        EFFECT_STOP_POLICY::NATURAL ==
                            ProductCue.Cue.eStopPolicy ? "natural" : "cue_end");
                }
                ImGui::TreePop();
            }
			if (!Entry.ProductCues.empty())
				Render_VisualProgramAuthoring(Entry, iProductCueIndex);
            if (0u != Entry.iSourceReferenceCount && ImGui::TreeNode((
				"Advanced Diagnostics | Source / Imported (" +
                std::to_string(Entry.iSourceReferenceCount) + ")").c_str()))
            {
                ImGui::TextDisabled(
                    "Reference-only: %zu imported rows | %zu empty payloads.",
                    Entry.iImportedReferenceCount,
                    Entry.iEmptySourceReferenceCount);
                ImGui::TextWrapped(
                    "These source package references never feed Product Play; promote an exact Authored product and save an effectref=asset cue first.");
                ImGui::TreePop();
            }

			const bool_t bHasPublishedAssembly = nullptr !=
				CEffectCatalog::Find_Assembly(strProductEffectAssetId);
			if (bHasPublishedAssembly && !bActiveProductDocument)
			{
				if (ImGui::TreeNode(
					"Advanced Diagnostics | Published Runtime Hierarchy"))
				{
					Render_AssemblyHierarchy(strProductEffectAssetId);
					ImGui::TreePop();
				}
				ImGui::TreePop();
				ImGui::PopID();
				continue;
			}
			if (bHasPublishedAssembly && bActiveProductDocument && ImGui::TreeNode(
				"Advanced Diagnostics | Published Runtime Hierarchy"))
			{
				Render_AssemblyHierarchy(strProductEffectAssetId);
				ImGui::TreePop();
			}
            if (nullptr == pTreeDocument)
            {
                ImGui::TextDisabled(
                    Entry.ProductCues.empty() ?
                    "No Product layers: an admitted asset cue is required." :
                    "The Product target cannot be inspected until its Authored document is admitted.");
                ImGui::TreePop();
                ImGui::PopID();
                continue;
            }
            const EFFECT_DOCUMENT_DESC& TreeDocument = *pTreeDocument;
            if (!TreeDocument.ModelCues.empty() &&
                ImGui::TreeNode(("Model Cues (" +
                    std::to_string(TreeDocument.ModelCues.size()) + ")").c_str()))
            {
                for (const EFFECT_MODEL_CUE_DESC& Cue : TreeDocument.ModelCues)
                {
                    ImGui::BulletText("%s | %s | %.3f s",
                        Cue.strCueId.c_str(), Cue.strClipName.c_str(),
                        Cue.fDurationSeconds);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", Cue.strModelAssetId.c_str());
                }
                ImGui::TreePop();
            }
            Render_ManualElementGroups(
                TreeDocument, strTreeEffectAssetId,
                false);
            for (int32_t iKind = 0;
                iKind < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END);
                ++iKind)
            {
                const EFFECT_ELEMENT_KIND eKind =
                    static_cast<EFFECT_ELEMENT_KIND>(iKind);
                const size_t iKindCount = static_cast<size_t>(std::count_if(
                    TreeDocument.Elements.begin(), TreeDocument.Elements.end(),
                    [eKind](const EFFECT_ELEMENT_DESC& Element)
                    {
                        return Element.eKind == eKind &&
                            !Is_ManualElementGroupMember(Element);
                    }));
                const std::string KindLabel =
                    EFFECT_ELEMENT_KIND::PARTICLE == eKind ?
                    "Imported Cascade Diagnostics | Source Systems " +
                        std::to_string(ParticleSummary.iSourceSystemCount) +
                        " | Emitters " +
                        std::to_string(ParticleSummary.iSourceEmitterCount) +
                        " | Mesh Particles " +
                        std::to_string(ParticleSummary.iMeshRendererCount) +
                        " | Sprite Particles " +
                        std::to_string(ParticleSummary.iSpriteRendererCount) +
                        " | Unresolved " +
                        std::to_string(ParticleSummary.iUnresolvedRendererCount) +
                        " | Budget " +
                        std::to_string(ParticleSummary.iParticleBudget) :
                    std::string(Kind_Label(eKind)) + " (" +
                        std::to_string(iKindCount) + ")";
                if (0u == iKindCount)
                    continue;
                bool_t bKindOpen = false;
                if (EFFECT_ELEMENT_KIND::PARTICLE == eKind)
                {
                    const bool_t bSystemSelected = bActiveProductDocument &&
                        EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM ==
                            m_eDetailSelection;
                    bKindOpen = ImGui::TreeNodeEx(
                        KindLabel.c_str(),
                        ImGuiTreeNodeFlags_OpenOnArrow |
                        (bSystemSelected ? ImGuiTreeNodeFlags_Selected : 0));
                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                        Try_SelectParticleSystem(strTreeEffectAssetId);
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip(
                            "Select this parent to tune all Cascade emitters together.\n"
                            "Mesh and Sprite are renderer types, not separate simulations.");
                    }
                }
                else
                    bKindOpen = ImGui::TreeNode(KindLabel.c_str());
                if (!bKindOpen)
                    continue;
                const auto RenderElementRows = [this, &TreeDocument,
                    &strTreeEffectAssetId, eKind](
                        const CASCADE_RENDERER_KIND* pRendererKind)
                {
                    for (const EFFECT_ELEMENT_DESC& Element :
                        TreeDocument.Elements)
                    {
                        if (Element.eKind != eKind ||
                            Is_ManualElementGroupMember(Element) ||
                            (nullptr != pRendererKind &&
                                Resolve_CascadeRendererKind(Element) !=
                                    *pRendererKind))
                        {
                            continue;
                        }
                        const bool_t bSelected = m_ActiveDocument.has_value() &&
                            m_ActiveDocument->strEffectAssetId ==
                                strTreeEffectAssetId &&
                            EFFECT_DETAIL_SELECTION::ELEMENT ==
                                m_eDetailSelection &&
                            Element.strElementId == m_strSelectedElementId;
                        std::string Label = Element.strDisplayName + "##" +
                            Element.strElementId;
                        if (!Element.strGroupId.empty())
                            Label = "[" + Element.strGroupId + "] " + Label;
						ImGui::PushID(Element.strElementId.c_str());
						const float fRowWidth = (std::max)(1.f,
							ImGui::GetContentRegionAvail().x - 54.f);
						if (ImGui::Selectable(Label.c_str(), bSelected, 0,
							ImVec2(fRowWidth, 0.f)))
                            Try_SelectElement(strTreeEffectAssetId,
                                Element.strElementId);
						ImGui::SameLine();
						if (ImGui::SmallButton("Solo"))
							Try_SoloElement(strTreeEffectAssetId,
								Element.strElementId);
						ImGui::PopID();
                    }
                };
                if (EFFECT_ELEMENT_KIND::PARTICLE == eKind)
                {
                    constexpr CASCADE_RENDERER_KIND RendererKinds[] = {
                        CASCADE_RENDERER_KIND::MESH,
                        CASCADE_RENDERER_KIND::SPRITE,
                        CASCADE_RENDERER_KIND::UNRESOLVED };
                    for (const CASCADE_RENDERER_KIND eRendererKind :
                        RendererKinds)
                    {
                        const size_t iRendererCount =
                            CASCADE_RENDERER_KIND::MESH == eRendererKind ?
                                ParticleSummary.iMeshRendererCount :
                            CASCADE_RENDERER_KIND::SPRITE == eRendererKind ?
                                ParticleSummary.iSpriteRendererCount :
                                ParticleSummary.iUnresolvedRendererCount;
                        if (0u == iRendererCount)
                            continue;
                        const std::string RendererLabel =
                            (CASCADE_RENDERER_KIND::MESH == eRendererKind ?
                                std::string("Mesh Particles (") :
                            CASCADE_RENDERER_KIND::SPRITE == eRendererKind ?
                                std::string("Sprite Particles (") :
                                std::string("Unresolved Particles (")) +
                            std::to_string(iRendererCount) + ")";
                        if (ImGui::TreeNode(RendererLabel.c_str()))
                        {
                            RenderElementRows(&eRendererKind);
                            ImGui::TreePop();
                        }
                    }
                }
                else
                    RenderElementRows(nullptr);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    if (m_ActiveDocument.has_value() && !bActiveAppearsInTree &&
        ImGui::TreeNodeEx(
			"Advanced Diagnostics | Active Effect Document / Imported",
			ImGuiTreeNodeFlags_OpenOnArrow))
    {
        const PARTICLE_LAYER_SUMMARY ParticleSummary =
            Summarize_ParticleLayers(*m_ActiveDocument);
        Render_ManualElementGroups(*m_ActiveDocument,
			m_ActiveDocument->strEffectAssetId, false);
        for (int32_t iKind = 0;
            iKind < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++iKind)
        {
            const EFFECT_ELEMENT_KIND eKind =
                static_cast<EFFECT_ELEMENT_KIND>(iKind);
            const size_t iKindCount = static_cast<size_t>(std::count_if(
                m_ActiveDocument->Elements.begin(),
                m_ActiveDocument->Elements.end(),
                [eKind](const EFFECT_ELEMENT_DESC& Element)
                {
                    return Element.eKind == eKind &&
                        !Is_ManualElementGroupMember(Element);
                }));
            const std::string KindLabel =
                EFFECT_ELEMENT_KIND::PARTICLE == eKind ?
                "Cascade System | Source Systems " +
                    std::to_string(ParticleSummary.iSourceSystemCount) +
                    " | Emitters " +
                    std::to_string(ParticleSummary.iSourceEmitterCount) +
                    " | Mesh Particles " +
                    std::to_string(ParticleSummary.iMeshRendererCount) +
                    " | Sprite Particles " +
                    std::to_string(ParticleSummary.iSpriteRendererCount) +
                    " | Unresolved " +
                    std::to_string(ParticleSummary.iUnresolvedRendererCount) +
                    " | Budget " +
                    std::to_string(ParticleSummary.iParticleBudget) :
                std::string(Kind_Label(eKind)) + " (" +
                    std::to_string(iKindCount) + ")";
            if (0u == iKindCount)
                continue;
            bool_t bKindOpen = false;
            if (EFFECT_ELEMENT_KIND::PARTICLE == eKind)
            {
                bKindOpen = ImGui::TreeNodeEx(
                    KindLabel.c_str(),
                    ImGuiTreeNodeFlags_OpenOnArrow |
                    (EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM ==
                        m_eDetailSelection ? ImGuiTreeNodeFlags_Selected : 0));
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {
                    Try_SelectParticleSystem(
                        m_ActiveDocument->strEffectAssetId);
                }
            }
            else
                bKindOpen = ImGui::TreeNode(KindLabel.c_str());
            if (!bKindOpen)
                continue;
            bool_t bLayerListOpen = true;
            if (EFFECT_ELEMENT_KIND::PARTICLE == eKind)
            {
                const std::string LayerLabel = "Layers (" +
                    std::to_string(iKindCount) + ")";
                bLayerListOpen = ImGui::TreeNode(LayerLabel.c_str());
            }
            if (bLayerListOpen)
            {
            for (const EFFECT_ELEMENT_DESC& Element : m_ActiveDocument->Elements)
            {
                if (Element.eKind != eKind ||
                    Is_ManualElementGroupMember(Element))
                    continue;
				ImGui::PushID(Element.strElementId.c_str());
				const float fRowWidth = (std::max)(1.f,
					ImGui::GetContentRegionAvail().x - 54.f);
				if (ImGui::Selectable(Element.strDisplayName.c_str(),
                    EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
					Element.strElementId == m_strSelectedElementId, 0,
					ImVec2(fRowWidth, 0.f)))
                    Try_SelectElement(
                        m_ActiveDocument->strEffectAssetId,
                        Element.strElementId);
				ImGui::SameLine();
				if (ImGui::SmallButton("Solo"))
					Try_SoloElement(m_ActiveDocument->strEffectAssetId,
						Element.strElementId);
				ImGui::PopID();
            }
                if (EFFECT_ELEMENT_KIND::PARTICLE == eKind)
                    ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::EndChild();
    if (!m_strElementStatus.empty())
        ImGui::TextWrapped("%s", m_strElementStatus.c_str());
    ImGui::End();
}

void Client::CEffect_Tool::Render_LoadedEffectContents()
{
    if (!m_ActiveDocument.has_value())
    {
        ImGui::TextDisabled("No Effect Document is loaded.");
        return;
    }

    ImGui::SeparatorText("Loaded Effect Contents");
    ImGui::TextWrapped("%s", m_ActiveDocument->strEffectAssetId.c_str());
    if (ImGui::Button("Play Complete Effect") &&
        Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
    {
        Start_WorldPreviewFromBeginning();
    }
    ImGui::SameLine();
    if (ImGui::Button("Hide Preview"))
        Hide_WorldPreview();
    ImGui::TextDisabled(
        "Element row = edit | Solo = one Element | Play Group = one complete hit");

    if (!Render_ManualElementGroups(*m_ActiveDocument,
        m_ActiveDocument->strEffectAssetId, true))
    {
        ImGui::TextDisabled(
            "This Document has no manual Hit groups; use All Effects for imported diagnostics.");
    }
}

void Client::CEffect_Tool::Refresh_AnimationClipLabels(
    const shared_ptr<Engine::CModel>& pModel,
    const bool_t bForce)
{
    const uint64_t iTargetGeneration =
        CAnimationTargetService::Resolve_TargetGeneration();
    const uint32_t iAnimationCount = nullptr == pModel ?
        0u : pModel->Get_NumAnimations();
    if (!bForce &&
        m_iAnimationClipLabelTargetGeneration == iTargetGeneration &&
        m_AnimationClipDisplayLabels.size() == iAnimationCount &&
        m_AnimationClipSearchTokens.size() == iAnimationCount)
    {
        return;
    }

    m_iAnimationClipLabelTargetGeneration = iTargetGeneration;
    m_AnimationClipDisplayLabels.clear();
    m_AnimationClipDisplayLabels.reserve(iAnimationCount);
    m_AnimationClipSearchTokens.clear();
    m_AnimationClipSearchTokens.reserve(iAnimationCount);
    for (uint32_t iAnimation = 0u;
        iAnimation < iAnimationCount; ++iAnimation)
    {
        const char* pName = pModel->Get_AnimationName(iAnimation);
        m_AnimationClipDisplayLabels.emplace_back(
            nullptr == pName ? "Invalid" : pName);
        m_AnimationClipSearchTokens.emplace_back(
            nullptr == pName ? "Invalid" : pName);
    }

    if (CAnimationTargetService::Resolve_AssetName() == VALTAN_ANIMATION_ASSET_NAME)
    {
        BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT Bindings;
        std::string BindingStatus;
        if (!CValtanPatternAnimationBindingDocument::Load(
                "Valtan", "BOSS_VALTAN", Collect_AnimationClipNames(pModel),
                Bindings, BindingStatus))
        {
            m_strAnimationClipLabelStatus =
                "Valtan labels preserved raw clip names: " + BindingStatus;
            return;
        }

        std::unordered_map<std::string, std::vector<std::string>>
            ActionsByClip;
        for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
            Bindings.Bindings)
        {
            for (const std::string& ClipName : Binding.Clips)
                ActionsByClip[ClipName].push_back(Binding.strActionId);
        }
        size_t iLabeledClipCount = 0u;
        for (uint32_t iAnimation = 0u;
            iAnimation < iAnimationCount; ++iAnimation)
        {
            const char_t* pName = pModel->Get_AnimationName(iAnimation);
            if (nullptr == pName)
                continue;
            const auto Actions = ActionsByClip.find(pName);
            if (Actions == ActionsByClip.end() || Actions->second.empty())
                continue;
            const std::vector<std::string>& ClipActions = Actions->second;
            std::string Label = "[Valtan] " + ClipActions.front();
            if (ClipActions.size() > 1u)
                Label += " (+" + std::to_string(ClipActions.size() - 1u) + ")";
            Label += " | ";
            Label += pName;
            m_AnimationClipDisplayLabels[iAnimation] = std::move(Label);
            std::string SearchTokens = pName;
            for (const std::string& Action : ClipActions)
                SearchTokens += " " + Action;
            m_AnimationClipSearchTokens[iAnimation] = std::move(SearchTokens);
            ++iLabeledClipCount;
        }
        m_strAnimationClipLabelStatus = "Valtan: " +
            std::to_string(iLabeledClipCount) +
            " clips labeled from pattern action bindings.";
        return;
    }

    const CHARACTER_SPEC* pSpec = Resolve_CurrentTargetSpec();
    if (nullptr == pSpec || nullptr == pSpec->pAssetName)
    {
        m_strAnimationClipLabelStatus =
            "No playable-class skill binding owns the selected model.";
        return;
    }

    std::string CatalogStatus;
    if (!Ensure_PlayerSkillCatalog(CatalogStatus))
    {
        m_strAnimationClipLabelStatus =
            "PlayerSkills label load failed: " + CatalogStatus;
        return;
    }
    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
    std::string BindingStatus;
    if (!CAnimationSkillBindingDocument::Load(
        pSpec->pAssetName,
        pSpec->eCharacterClass,
        Skills,
        Collect_AnimationClipNames(pModel),
        Bindings,
        BindingStatus))
    {
        m_strAnimationClipLabelStatus =
            "Skill binding labels preserved raw clip names: " +
            BindingStatus;
        return;
    }

    size_t iLabeledClipCount = 0u;
    for (const ANIMATION_SKILL_BINDING& Binding : Bindings.Bindings)
    {
        const auto Skill = std::find_if(
            Skills.begin(), Skills.end(),
            [&Binding, pSpec](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return Candidate.eCharacterClass == pSpec->eCharacterClass &&
                    Candidate.iSkillId == Binding.iSkillId;
            });
        if (Skill == Skills.end())
            continue;
        std::vector<ANIMATION_SKILL_CLIP> BoundClips;
        for (const ANIMATION_SKILL_STAGE& Stage : Binding.Stages)
        {
            BoundClips.insert(
                BoundClips.end(), Stage.Clips.begin(), Stage.Clips.end());
        }
        for (size_t iClip = 0u;
            iClip < BoundClips.size(); ++iClip)
        {
            const ANIMATION_SKILL_CLIP& Clip = BoundClips[iClip];
            for (uint32_t iAnimation = 0u;
                iAnimation < iAnimationCount; ++iAnimation)
            {
                const char* pName = pModel->Get_AnimationName(iAnimation);
                if (nullptr == pName || Clip.strClipName != pName)
                    continue;
                std::string Label = "[" + Skill->strInputSlot + "] " +
                    Skill->strDisplayName;
                if (BoundClips.size() > 1u)
                {
                    Label += " " + std::to_string(iClip + 1u) + "/" +
                        std::to_string(BoundClips.size());
                }
                Label += " | " + Clip.strClipName;
                m_AnimationClipDisplayLabels[iAnimation] = std::move(Label);
                m_AnimationClipSearchTokens[iAnimation] =
                    m_AnimationClipDisplayLabels[iAnimation];
                ++iLabeledClipCount;
                break;
            }
        }
    }
    m_strAnimationClipLabelStatus =
        std::string(pSpec->pAssetName) + ": " +
        std::to_string(iLabeledClipCount) +
        " skill-bound clips labeled from Authored bindings.";
}

void Client::CEffect_Tool::Render_DataFilesWindow()
{
    ImGui::SetNextWindowPos(ImVec2(10.f, 705.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(760.f, 560.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Data Files"))
    {
        ImGui::End();
        return;
    }
    if (ImGui::BeginCombo("Authoring Category##DataFilesDomain",
        m_strSelectedAuthoringDomainId.c_str()))
    {
        for (const std::string& DomainId : m_DataFileDomains)
        {
            if (ImGui::Selectable(DomainId.c_str(),
                DomainId == m_strSelectedAuthoringDomainId))
            {
                Select_AuthoringDomain(DomainId);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::TextDisabled(
        "Category changes filter the cached index; Refresh Index rescans disk.");
    if (ImGui::CollapsingHeader("Advanced Document Commands"))
    {
        ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
            m_NewAssetId.size());
        ImGui::InputText("Display Name", m_NewDisplayName.data(),
            m_NewDisplayName.size());
        ImGui::TextDisabled(
			"Normal workflow: New Effect -> Create Element -> slots/Details -> Save Changes.");
		if (ImGui::Button("New Effect"))
            Try_CreateDocument();
        ImGui::SameLine();
		const bool_t bRuntimeView =
			EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == m_eActiveDocumentSource ||
			EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource ||
			EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
				m_eActiveDocumentSource ||
			EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE ==
				m_eActiveDocumentSource;
		ImGui::BeginDisabled(bRuntimeView);
		if (ImGui::Button("Save Changes"))
			Try_ApplyDraftAndSave();
		ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Save As"))
            Try_SaveDocumentAs(m_NewAssetId.data());
        ImGui::SameLine();
        if (ImGui::Button("Reload Saved"))
            Try_ReloadActiveDocument();
        ImGui::BeginDisabled(
            EFFECT_DOCUMENT_SOURCE::IMPORTED != m_eActiveDocumentSource);
        if (ImGui::Button("Promote Imported to Authored Skill"))
            m_bPromoteConfirmationRequested = true;
        ImGui::EndDisabled();
    }
    if (m_bDiscardConfirmationRequested)
    {
        ImGui::OpenPopup("Unload Effect with unsaved changes?");
        m_bDiscardConfirmationRequested = false;
    }
    if (ImGui::BeginPopupModal(
        "Unload Effect with unsaved changes?", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(
            "The in-memory changes will be lost. The saved Data File will not be deleted.");
        if (ImGui::Button("Unload and Discard Changes"))
        {
            Discard_ActiveDocument();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (m_bPromoteConfirmationRequested)
    {
        ImGui::OpenPopup("Promote Imported Effect?");
        m_bPromoteConfirmationRequested = false;
    }
    if (ImGui::BeginPopupModal(
        "Promote Imported Effect?", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(
            "Replace the matching skill Authored Document atomically?");
        if (ImGui::Button("Promote and Replace"))
        {
            if (Try_PromoteImportedDocument())
                ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (m_ActiveDocument.has_value())
    {
        const PARTICLE_LAYER_SUMMARY ParticleSummary =
            Summarize_ParticleLayers(*m_ActiveDocument);
        ImGui::Text("Active: %s | %s | %s | Elements %zu | Model Cues %zu%s",
            m_ActiveDocument->strEffectAssetId.c_str(),
            m_ActiveDocument->strDisplayName.c_str(),
            Source_Label(m_eActiveDocumentSource),
            m_ActiveDocument->Elements.size(),
            m_ActiveDocument->ModelCues.size(),
            Has_UnsavedWork() ? " | DIRTY" : "");
        ImGui::TextDisabled(
            "Subtypes: Standalone Mesh %zu | Mesh Particle %zu | Standalone Sprite %zu | Sprite Particle %zu",
            ParticleSummary.iStandaloneMeshCount,
            ParticleSummary.iMeshRendererCount,
            ParticleSummary.iStandaloneSpriteCount,
            ParticleSummary.iSpriteRendererCount);
        ImGui::TextDisabled(
            "Cascade System: Source Systems %zu | Emitters %zu | Layers %zu | Unresolved %zu | Budget %llu",
            ParticleSummary.iSourceSystemCount,
            ParticleSummary.iSourceEmitterCount,
            ParticleSummary.iLayerCount,
            ParticleSummary.iUnresolvedRendererCount,
            static_cast<unsigned long long>(
                ParticleSummary.iParticleBudget));
    }
    ImGui::InputTextWithHint("##DataFilesSearch",
		"Search skill name or Effect ID", m_DataFilesSearch.data(),
		m_DataFilesSearch.size());
    const std::string strDataFileSearch = m_DataFilesSearch.data();
	std::unordered_set<std::string> AuthoredAssetIds;
	AuthoredAssetIds.reserve(m_DataFiles.size());
	for (const EFFECT_DATA_FILE_ENTRY& DataFile : m_DataFiles)
	{
		if (DataFile.eSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
			DataFile.strDomainId == m_strSelectedAuthoringDomainId)
		{
			AuthoredAssetIds.insert(DataFile.strAssetId);
		}
	}
	const auto IsLegacyMigrationReference = [&AuthoredAssetIds](
		const EFFECT_DATA_FILE_ENTRY& DataFile)
	{
		if (DataFile.eSource != EFFECT_DOCUMENT_SOURCE::AUTHORED ||
			DataFile.strAssetId.ends_with(".unified"))
		{
			return false;
		}
		return AuthoredAssetIds.contains(
			Unified_CandidateAssetId(DataFile.strAssetId));
	};
	std::unordered_map<std::string, const EFFECT_SKILL_TREE_ENTRY*>
		SavedEffectSkillByAssetId;
	SavedEffectSkillByAssetId.reserve(m_AllEffects.size() * 4u);
	for (const EFFECT_SKILL_TREE_ENTRY& Entry : m_AllEffects)
	{
		const char* pDomainId = Resource_DomainId(
			Entry.Skill.eCharacterClass);
		if (nullptr == pDomainId ||
			m_strSelectedAuthoringDomainId != pDomainId)
		{
			continue;
		}
		const auto RegisterAsset = [&SavedEffectSkillByAssetId, &Entry](
			const std::string_view AssetId)
		{
			if (AssetId.empty())
				return;
			SavedEffectSkillByAssetId.try_emplace(std::string(AssetId), &Entry);
			if (!AssetId.ends_with(".unified"))
			{
				SavedEffectSkillByAssetId.try_emplace(
					Unified_CandidateAssetId(AssetId), &Entry);
			}
		};
		RegisterAsset(Entry.Skill.strEffectId);
		for (const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Cue :
			Entry.ProductCues)
		{
			RegisterAsset(Cue.Cue.strEffectAssetId);
		}
		if (Entry.Skill.eCharacterClass ==
				LostArk::Shared::CHARACTER_CLASS_ID::ARTIST &&
			Entry.Skill.iSkillId == ARTIST_F_CORE_SKILL_ID)
		{
			RegisterAsset(ARTIST_F_UNIFIED_EFFECT_ASSET_ID);
		}
		if (Entry.Skill.eCharacterClass ==
				LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER &&
			Entry.Skill.iSkillId == DIMENSION_MASTER_T_SKILL_ID)
		{
			RegisterAsset(DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID);
		}
	}
	const auto FindSavedEffectSkill = [&SavedEffectSkillByAssetId](
		const EFFECT_DATA_FILE_ENTRY& DataFile)
		-> const EFFECT_SKILL_TREE_ENTRY*
	{
		const auto Found = SavedEffectSkillByAssetId.find(DataFile.strAssetId);
		return Found == SavedEffectSkillByAssetId.end() ? nullptr : Found->second;
	};
	const auto MatchesSavedEffectSearchContent = [this, &strDataFileSearch,
		&FindSavedEffectSkill](const EFFECT_DATA_FILE_ENTRY& DataFile)
	{
		if (DataFile.eSource != EFFECT_DOCUMENT_SOURCE::AUTHORED ||
			DataFile.strDomainId != m_strSelectedAuthoringDomainId)
		{
			return false;
		}
		if (Contains_NoCase(DataFile.strAssetId, strDataFileSearch))
			return true;
		const EFFECT_SKILL_TREE_ENTRY* pSkill =
			FindSavedEffectSkill(DataFile);
		return nullptr != pSkill &&
			(Contains_NoCase(pSkill->Skill.strInputSlot, strDataFileSearch) ||
			 Contains_NoCase(pSkill->Skill.strDisplayName, strDataFileSearch));
	};
	const auto MatchesSavedEffectSearch = [&MatchesSavedEffectSearchContent,
		&IsLegacyMigrationReference](const EFFECT_DATA_FILE_ENTRY& DataFile)
	{
		return MatchesSavedEffectSearchContent(DataFile) &&
			!IsLegacyMigrationReference(DataFile);
	};
	std::vector<const EFFECT_DATA_FILE_ENTRY*> MigrationReferences;
	std::unordered_map<const EFFECT_SKILL_TREE_ENTRY*,
		std::vector<const EFFECT_DATA_FILE_ENTRY*>> SavedEffectsBySkill;
	SavedEffectsBySkill.reserve(m_AllEffects.size());
	std::vector<const EFFECT_DATA_FILE_ENTRY*> UnassignedEffects;
	size_t iVisibleSavedEffects = 0u;
	for (const EFFECT_DATA_FILE_ENTRY& DataFile : m_DataFiles)
	{
		if (MatchesSavedEffectSearchContent(DataFile) &&
			IsLegacyMigrationReference(DataFile))
		{
			MigrationReferences.push_back(&DataFile);
			continue;
		}
		if (!MatchesSavedEffectSearch(DataFile))
			continue;
		++iVisibleSavedEffects;
		if (const EFFECT_SKILL_TREE_ENTRY* pSkill =
			FindSavedEffectSkill(DataFile))
		{
			SavedEffectsBySkill[pSkill].push_back(&DataFile);
		}
		else
		{
			UnassignedEffects.push_back(&DataFile);
		}
	}
	ImGui::SeparatorText("Saved Skill Effects");
	ImGui::TextDisabled("%s: %zu saved Effects, including Unassigned/Test. Each row opens one complete multi-Element Effect.",
		m_strSelectedAuthoringDomainId.c_str(), iVisibleSavedEffects);
	ImGui::TextDisabled(
		"Character is selected above; skills are grouped below. Internal asset IDs are available in tooltips.");
	ImGui::BeginChild("SavedEffectDataFileList", ImVec2(0.f, 190.f), true);
	const auto RenderSavedEffectRow = [this](
		const EFFECT_DATA_FILE_ENTRY& DataFile,
		const EFFECT_SKILL_TREE_ENTRY* pSkill)
	{
		std::string RowLabel = "Saved Effect";
		bool_t bGameplayLinked = false;
		if (nullptr != pSkill)
		{
			const auto Cue = std::find_if(pSkill->ProductCues.begin(),
				pSkill->ProductCues.end(), [&DataFile](const auto& Candidate)
				{
					return Candidate.Cue.strEffectAssetId == DataFile.strAssetId;
				});
			if (Cue != pSkill->ProductCues.end())
			{
				bGameplayLinked = true;
				if (pSkill->Skill.eSkillKind ==
					LostArk::Shared::PLAYER_SKILL_KIND::COMBO)
				{
					RowLabel = "BA " +
						std::to_string(Cue->iBoundClipOrdinal + 1u);
				}
				else if (pSkill->ProductCues.size() > 1u)
				{
					RowLabel = "Stage " +
						std::to_string(Cue->iBoundClipOrdinal + 1u);
				}
				else
				{
					RowLabel = "Gameplay Effect";
				}
			}
			else if (DataFile.strAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID ||
				DataFile.strAssetId == DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID)
			{
				RowLabel = "Editable Draft (not linked to gameplay)";
			}
			else
			{
				RowLabel = "Saved Skill Draft (not published)";
			}
		}
		RowLabel += "##saved-" + DataFile.strAssetId;
		if (ImGui::Selectable(RowLabel.c_str(),
			DataFile.strAssetId == m_strSelectedDataFileAssetId))
		{
			m_strSelectedDataFileAssetId = DataFile.strAssetId;
			Select_AuthoringDomain(DataFile.strDomainId);
			Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
				DataFile.strAssetId);
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				Try_LoadDocumentPath(
					DataFile.Path, DataFile.eSource, DataFile.strAssetId);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Effect Asset ID: %s\nGameplay mapping: %s\n%s",
				DataFile.strAssetId.c_str(),
				bGameplayLinked ? "linked" : "not linked",
				DataFile.Path.string().c_str());
	};
	for (const EFFECT_SKILL_TREE_ENTRY& SkillEntry : m_AllEffects)
	{
		const char* pDomainId = Resource_DomainId(
			SkillEntry.Skill.eCharacterClass);
		if (nullptr == pDomainId || m_strSelectedAuthoringDomainId != pDomainId)
			continue;
		const auto SkillEffects = SavedEffectsBySkill.find(&SkillEntry);
		if (SkillEffects == SavedEffectsBySkill.end() ||
			SkillEffects->second.empty())
			continue;
		ImGui::PushID(static_cast<int>(SkillEntry.Skill.iSkillId));
		const std::string SkillLabel = "[" + SkillEntry.Skill.strInputSlot +
			"] " + SkillEntry.Skill.strDisplayName + " (" +
			std::to_string(SkillEffects->second.size()) + ")";
		if (ImGui::TreeNodeEx(SkillLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow))
		{
			for (const EFFECT_DATA_FILE_ENTRY* pDataFile : SkillEffects->second)
				RenderSavedEffectRow(*pDataFile, &SkillEntry);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	if (!UnassignedEffects.empty() && ImGui::TreeNodeEx(
		("Unassigned / Test Effects (" +
		 std::to_string(UnassignedEffects.size()) + ")").c_str(),
		ImGuiTreeNodeFlags_OpenOnArrow))
	{
		for (const EFFECT_DATA_FILE_ENTRY* pDataFile : UnassignedEffects)
		{
			ImGui::PushID(pDataFile->strAssetId.c_str());
			if (ImGui::Selectable(pDataFile->strAssetId.c_str(),
				pDataFile->strAssetId == m_strSelectedDataFileAssetId))
			{
				m_strSelectedDataFileAssetId = pDataFile->strAssetId;
				Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
					pDataFile->strAssetId);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Unmapped authoring/test Effect\n%s",
					pDataFile->Path.string().c_str());
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	if (!MigrationReferences.empty())
	{
		ImGui::Separator();
		const std::string MigrationLabel =
			"Advanced Migration Reference (" +
			std::to_string(MigrationReferences.size()) + ")";
		if (ImGui::TreeNodeEx(MigrationLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow))
		{
			ImGui::TextDisabled(
				"Authoring reference only. Gameplay may keep this baseline until its .unified candidate is visually approved and mapped.");
			for (const EFFECT_DATA_FILE_ENTRY* pDataFile : MigrationReferences)
			{
				const EFFECT_SKILL_TREE_ENTRY* pSkill =
					FindSavedEffectSkill(*pDataFile);
				std::string Label = "Legacy / Rollback | ";
				if (nullptr != pSkill)
				{
					Label += "[" + pSkill->Skill.strInputSlot + "] " +
						pSkill->Skill.strDisplayName + " | ";
				}
				Label += pDataFile->strAssetId + "##migration-" +
					pDataFile->strAssetId;
				if (ImGui::Selectable(Label.c_str(),
					pDataFile->strAssetId == m_strSelectedDataFileAssetId))
				{
					m_strSelectedDataFileAssetId = pDataFile->strAssetId;
					Select_AuthoringDomain(pDataFile->strDomainId);
					Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
						pDataFile->strAssetId + ".migrated");
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						Try_LoadDocumentPath(pDataFile->Path,
							EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE,
							pDataFile->strAssetId);
					}
				}
				if (ImGui::IsItemHovered())
				{
					const bool_t bCurrentGameplayBaseline = nullptr != pSkill &&
						std::any_of(pSkill->ProductCues.begin(),
							pSkill->ProductCues.end(),
							[pDataFile](const auto& Cue)
							{
								return Cue.Cue.strEffectAssetId ==
									pDataFile->strAssetId;
							});
					ImGui::SetTooltip(
						"Legacy/Rollback migration reference\nCurrent gameplay baseline: %s\nEffect Asset ID: %s\n%s",
						bCurrentGameplayBaseline ? "yes" : "no",
						pDataFile->strAssetId.c_str(),
						pDataFile->Path.string().c_str());
				}
			}
			ImGui::TreePop();
		}
	}
    ImGui::EndChild();
    const auto SelectedDataFile = std::find_if(
        m_DataFiles.begin(), m_DataFiles.end(),
        [this](const EFFECT_DATA_FILE_ENTRY& Entry)
        {
            return Entry.strAssetId == m_strSelectedDataFileAssetId;
        });
    const bool_t bSelectedSavedEffect =
        SelectedDataFile != m_DataFiles.end() &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED == SelectedDataFile->eSource &&
		!IsLegacyMigrationReference(*SelectedDataFile);
	const bool_t bSelectedMigrationReference =
		SelectedDataFile != m_DataFiles.end() &&
		IsLegacyMigrationReference(*SelectedDataFile);
    ImGui::BeginDisabled(!bSelectedSavedEffect);
    if (ImGui::Button("Load Saved Effect for Editing") &&
        SelectedDataFile != m_DataFiles.end())
    {
        Try_LoadDocumentPath(
            SelectedDataFile->Path,
            SelectedDataFile->eSource,
            SelectedDataFile->strAssetId);
    }
    ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bSelectedMigrationReference);
	if (ImGui::Button("Load Migration Reference") &&
		SelectedDataFile != m_DataFiles.end())
	{
		Try_LoadDocumentPath(SelectedDataFile->Path,
			EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE,
			SelectedDataFile->strAssetId);
	}
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_ActiveDocument.has_value());
    if (ImGui::Button("Unload Document"))
    {
        if (Has_UnsavedWork())
            m_bDiscardConfirmationRequested = true;
        else
            Discard_ActiveDocument();
    }
    ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Refresh Index"))
	{
		Refresh_AllEffects(true);
		Refresh_DataFiles();
	}
    ImGui::TextDisabled(
        "Load opens the existing name as Current Effect and selects its first Element. Edit in Effect Details, then Save Changes.");
    ImGui::TextDisabled(
        "Unload = remove it from the screen, never delete the file");
	if (ImGui::CollapsingHeader(
		"Advanced Diagnostics (Imported / WFX / Runtime)"))
    {
        ImGui::TextDisabled(
            "Imported, runtime, and extraction rows are diagnostic or read-only sources; they are not saved Current Effects.");
        ImGui::BeginChild("AdvancedEffectDataFileList",
            ImVec2(0.f, 110.f), true);
        for (const EFFECT_DATA_FILE_ENTRY& Entry : m_DataFiles)
        {
            if (Entry.eSource == EFFECT_DOCUMENT_SOURCE::AUTHORED ||
                Entry.strDomainId != m_strSelectedAuthoringDomainId ||
                !Contains_NoCase(Entry.strAssetId, strDataFileSearch))
            {
                continue;
            }
            const std::string Label = std::string("[") +
                Source_Label(Entry.eSource) + "] " + Entry.strAssetId;
            if (ImGui::Selectable(Label.c_str(),
                Entry.strAssetId == m_strSelectedDataFileAssetId))
            {
                m_strSelectedDataFileAssetId = Entry.strAssetId;
                Select_AuthoringDomain(Entry.strDomainId);
                if (EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE == Entry.eSource)
                {
                    m_strDocumentStatus =
                        "Selected extraction draft is reference-only; use All Effects to load one admitted Source Element.";
                }
                else
                {
                    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
                        Entry.strAssetId);
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        Try_LoadDocumentPath(
                            Entry.Path, Entry.eSource, Entry.strAssetId);
                    }
                }
            }
        }
        ImGui::EndChild();
        const auto SelectedAdvancedDocument = std::find_if(
            m_DataFiles.begin(), m_DataFiles.end(),
            [this](const EFFECT_DATA_FILE_ENTRY& Entry)
            {
                return Entry.strAssetId == m_strSelectedDataFileAssetId;
            });
        const bool_t bSelectedAdvancedDocumentLoadable =
            SelectedAdvancedDocument != m_DataFiles.end() &&
            SelectedAdvancedDocument->eSource !=
                EFFECT_DOCUMENT_SOURCE::AUTHORED &&
            SelectedAdvancedDocument->eSource !=
                EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE;
        ImGui::BeginDisabled(!bSelectedAdvancedDocumentLoadable);
        if (ImGui::Button("Load Advanced Document") &&
            SelectedAdvancedDocument != m_DataFiles.end())
        {
            Try_LoadDocumentPath(
                SelectedAdvancedDocument->Path,
                SelectedAdvancedDocument->eSource,
                SelectedAdvancedDocument->strAssetId);
        }
        ImGui::EndDisabled();
    }
	ImGui::TextDisabled(
		"After opening a saved row, use Effect Tool > Current Effect for Family/Element selection, Play All, Family, and Solo.");
    if (!m_strDocumentStatus.empty())
        ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
    ImGui::End();
}

bool_t Client::CEffect_Tool::Try_CreateDocument()
{
    if (Has_UnsavedWork())
    {
        m_strDocumentStatus =
            "Save or explicitly discard the active Effect changes first.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Document;
    Document.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
    Document.strEffectAssetId = m_NewAssetId.data();
    Document.strDisplayName = m_NewDisplayName.data();
	if (Document.strDisplayName.empty())
		Document.strDisplayName = Document.strEffectAssetId;
    std::string Error;
    if (!CEffectDocumentCodec::Validate(Document, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    std::string DrawableError;
    const bool_t bDrawable =
        CEffectDocumentCodec::Validate_Drawable(Document, DrawableError);
    const std::filesystem::path ExistingPath = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(Document.strEffectAssetId).wstring() +
            L".effect.json"));
	if (ExistingPath.empty() || std::filesystem::is_regular_file(ExistingPath))
	{
        m_strDocumentStatus = ExistingPath.empty() ?
            "New Effect path escaped Data/Effects/Authored." :
            "New refuses an existing Effect ID; load that file or choose another ID.";
		return false;
	}
	Release_WorldPreview(true);
    Clear_ProductCuePreview();
    m_ActiveDocument = std::move(Document);
    Set_ActiveDocumentDrawableStatus(bDrawable, std::move(DrawableError));
    m_ActiveDocumentPath.clear();
	m_strActiveDocumentBaselineCanonical.clear();
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
	Reset_ModelCueDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
    m_strSelectedElementId.clear();
	m_strSelectedElementGroupId.clear();
	m_strSelectedModelCueId.clear();
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewIsolationModelCueId.clear();
	m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_bDocumentDirty = true;
    m_bActiveDocumentMatchesRuntime = false;
	m_strSaveHotReloadStatus.clear();
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration();
    m_strDocumentStatus =
		"Created a new Current Effect in memory. Create an Element, bind WModel/DDS slots, tune Details, then use Save Changes.";
    return true;
}

bool_t Client::CEffect_Tool::Try_CreateMeshEffect(
	const bool_t bAddToCurrentEffect)
{
    if (!m_bResourceCatalogRefreshAttempted && !Refresh_ResourceCatalog())
        return false;

	if (bAddToCurrentEffect &&
		(!m_ActiveDocument.has_value() ||
		 (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		  EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource)))
	{
		m_strElementStatus =
			"Add to Current Effect requires one open authored Effect.";
		return false;
	}
	const std::string strTargetEffectId = bAddToCurrentEffect ?
		m_ActiveDocument->strEffectAssetId : std::string(m_NewAssetId.data());
    if (strTargetEffectId.empty())
    {
        m_strElementStatus =
			"Enter an Effect Name before creating the authored Effect.";
        return false;
    }
	const EFFECT_AUTHORING_FAMILY eAuthoringFamily =
		m_eSelectedAuthoringFamily;
	const EFFECT_ELEMENT_KIND eElementKind =
		AuthoringFamily_Kind(eAuthoringFamily);
	if (!AuthoringFamily_CanCreate(eAuthoringFamily) ||
		EFFECT_ELEMENT_KIND::END == eElementKind)
	{
		m_strElementStatus =
			"Select one drawable authoring family. Presentation Light and Screen Post creation requires source-backed materialization.";
		return false;
	}

	const bool_t bUsesActiveDocument = bAddToCurrentEffect;
    if (!bUsesActiveDocument && Has_UnsavedWork())
    {
        m_strElementStatus =
            "Save or discard the different active Effect before creating this named Data File.";
        return false;
    }
    if (m_ActiveDocument.has_value() &&
        m_ActiveDocument->strEffectAssetId == strTargetEffectId &&
        !bUsesActiveDocument)
    {
        m_strElementStatus =
            "Imported/runtime Effects are read-only; enter a unique Effect Name for the authored Data File.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged;
    if (bUsesActiveDocument)
    {
        Staged = *m_ActiveDocument;
        if (m_bParticleSystemDraftDirty)
            Apply_ParticleSystemDraft(Staged);
        if (m_bDetailDraftDirty && !Apply_DetailDraft(Staged))
        {
            m_strElementStatus =
				"The open Detail draft no longer matches its Element; the authoring transaction preserved all data.";
            return false;
        }
    }
    else
    {
        Staged.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
        Staged.strEffectAssetId = strTargetEffectId;
    }
    const std::string strRequestedDisplayName = m_NewDisplayName.data();
	if (!bUsesActiveDocument && !strRequestedDisplayName.empty())
        Staged.strDisplayName = strRequestedDisplayName;
    else if (Staged.strDisplayName.empty())
        Staged.strDisplayName = strTargetEffectId;

    EFFECT_ELEMENT_DESC Element = m_MeshAuthoringDraft;
	Element.eKind = eElementKind;
	Element.Renderer = {};
	if (Element.Material.strTemplateId.empty())
	{
		Element.Material.strTemplateId =
			std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
	}
	Element.strGroupId = "manual.hit1";
	Element.strSourceNode.clear();
    Element.SourceRecipe = {};
    Element.SourcePresentation = {};
    Element.ActionCueAttachment = {};
	Element.TransformInheritance = {};
	Element.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
    Element.strElementId = m_NewElementId.data();
    if (Element.strElementId.empty())
    {
		const std::string Prefix = std::string(
			AuthoringFamily_ElementPrefix(eAuthoringFamily)) + "_";
        size_t iCandidate = Staged.Elements.size() + 1u;
        do
        {
            Element.strElementId = Prefix + std::to_string(iCandidate++);
        }
        while (std::any_of(Staged.Elements.begin(),
            Staged.Elements.end(),
            [&Element](const EFFECT_ELEMENT_DESC& Existing)
            {
                return Existing.strElementId == Element.strElementId;
            }));
    }
    Element.strDisplayName = Element.strElementId;

    if (std::any_of(Staged.Elements.begin(),
        Staged.Elements.end(),
        [&Element](const EFFECT_ELEMENT_DESC& Existing)
        {
            return Existing.strElementId == Element.strElementId;
        }))
    {
        m_strElementStatus = "Create Effect rejected a duplicate Element ID.";
        return false;
    }

    const EFFECT_RESOURCE_BINDING_DESC* pMesh = Find_Binding(
        Element, EFFECT_MESH_SHAPE_SLOT_ID);
    const EFFECT_RESOURCE_BINDING_DESC* pBase = Find_Binding(
        Element, EFFECT_STANDARD_MATERIAL_INPUTS[0u].strSlotId);
    const bool_t bUnsafeBase = nullptr != pBase &&
        Is_UnsafeEffectBaseTextureAssetId(pBase->strAssetId);
	const bool_t bRequiresMesh = AuthoringFamily_RequiresMesh(
		eAuthoringFamily);
	if (nullptr == pBase || bUnsafeBase || (bRequiresMesh && nullptr == pMesh))
    {
        m_strElementStatus =
			bRequiresMesh ?
			"Mesh and Mesh Particle require one WModel Mesh and one safe 2D Base texture." :
			"Sprite, Sprite Particle, Local Decal, and Trail / Ribbon require one safe 2D Base texture.";
        return false;
    }

    std::set<std::string> Slots;
    for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
        Element.ResourceBindings)
    {
		if (!Slots.insert(Binding.strSlotId).second ||
			!Slot_Allowed(Element, Binding.strSlotId) ||
			!AuthoringFamily_AllowsSlot(eAuthoringFamily, Binding.strSlotId))
        {
            m_strElementStatus =
                "Create Effect rejected a duplicate or unsupported resource slot.";
            return false;
        }
        const EFFECT_RESOURCE_FILE_KIND eExpected =
            Slot_FileKind(Element, Binding.strSlotId);
        const auto CatalogEntry = std::find_if(
            m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
            [this, &Binding, eExpected](
                const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
            {
                return Entry.strAssetId == Binding.strAssetId &&
                    Entry.strDomainId == m_strSelectedAuthoringDomainId &&
                    Entry.eFileKind == eExpected;
            });
        if (CatalogEntry == m_ResourceCatalog.end())
        {
            m_strElementStatus =
                "Create Effect preserved the draft: a resource left the active domain or file kind.";
            return false;
        }
    }

    Staged.Elements.push_back(Element);

    std::string Error;
    if (!CEffectDocumentCodec::Validate(Staged, Error))
    {
        m_strElementStatus = Error;
        return false;
    }
    std::string DrawableError;
    if (!CEffectDocumentCodec::Validate_Drawable(Staged, DrawableError))
    {
        m_strElementStatus =
			"The authoring transaction rejected a non-drawable Element: " +
            DrawableError;
        return false;
    }

    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(strTargetEffectId).wstring() +
            L".effect.json"));
    if (Path.empty())
    {
        m_strElementStatus =
            "Effect Name escaped Data/Effects/Authored.";
        return false;
    }
    if (!bUsesActiveDocument && std::filesystem::is_regular_file(Path))
    {
        m_strElementStatus =
            "That Effect Name already exists in Data Files; load it before adding another layer.";
        return false;
    }
    if (bUsesActiveDocument &&
        EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource &&
        std::filesystem::is_regular_file(Path))
    {
        m_strElementStatus =
            "The new Effect Name appeared on disk; Create Effect preserved it and refused overwrite.";
        return false;
    }

	const optional<EFFECT_PRODUCT_PREVIEW> PreviousProductPreview =
		m_ProductPreview;
	const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanProductPreview =
		m_ValtanProductPreview;
	const optional<EFFECT_DOCUMENT_DESC> PreviousSourcePreviewDocument =
		m_SourcePreviewDocument;
	const bool_t bPreviousArtistAdapterPreviewActive =
		m_bReconstructedSourceRuntimeActive;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
    const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter = m_ePreviewFilter;
	const f32_t fPreviousPreviewTimeSeconds = m_fPreviewTimeSeconds;
	const f32_t fPreviousPreviewDurationSeconds = m_fPreviewDurationSeconds;
	const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
	const bool_t bPreviousPreviewVisibleRequested =
		m_bPreviewVisibleRequested;
	const float4x4_t PreviousProductCueSnapshotRoot =
		m_ProductCueSnapshotRoot;
	const bool_t bPreviousProductCueSnapshotCaptured =
		m_bProductCueSnapshotCaptured;
	const f32_t fPreviousProductCueActionFacingYawDegrees =
		m_fProductCueActionFacingYawDegrees;
	const bool_t bPreviousProductCueActionFacingCaptured =
		m_bProductCueActionFacingCaptured;
	std::string ValtanRestoreError;
	const auto RestorePreviousSourcePreviewState = [this,
		&PreviousProductPreview, &PreviousValtanProductPreview,
		&PreviousSourcePreviewDocument,
		&strPreviousIsolationElement, &strPreviousIsolationGroup,
		bPreviousArtistAdapterPreviewActive,
		ePreviousPreviewFilter, fPreviousPreviewTimeSeconds,
		fPreviousPreviewDurationSeconds, bPreviousPreviewPlaying,
		bPreviousPreviewVisibleRequested, &PreviousProductCueSnapshotRoot,
		bPreviousProductCueSnapshotCaptured,
		fPreviousProductCueActionFacingYawDegrees,
		bPreviousProductCueActionFacingCaptured, &ValtanRestoreError]()
	{
		m_ProductPreview = PreviousProductPreview;
		m_ValtanProductPreview = PreviousValtanProductPreview;
		m_SourcePreviewDocument = PreviousSourcePreviewDocument;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
		m_ePreviewFilter = ePreviousPreviewFilter;
		m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
		m_fPreviewDurationSeconds = fPreviousPreviewDurationSeconds;
		Reset_ProductCueSnapshot();
		if (!bPreviousArtistAdapterPreviewActive)
		{
			if (m_ValtanProductPreview.has_value())
				return Restore_ValtanProductPreviewPlayback(
					m_ValtanProductPreview,
					fPreviousPreviewTimeSeconds,
					fPreviousPreviewDurationSeconds,
					bPreviousPreviewPlaying,
					bPreviousPreviewVisibleRequested,
					PreviousProductCueSnapshotRoot,
					bPreviousProductCueSnapshotCaptured,
					ValtanRestoreError);
			else
			{
				m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
				m_bProductCueSnapshotCaptured =
					bPreviousProductCueSnapshotCaptured;
				m_fProductCueActionFacingYawDegrees =
					fPreviousProductCueActionFacingYawDegrees;
				m_bProductCueActionFacingCaptured =
					bPreviousProductCueActionFacingCaptured;
				Synchronize_LoadedSkillPreview();
			}
		}
		return true;
	};
	const auto RestorePreviousSourceIsolation = [this,
		&strPreviousIsolationElement, &strPreviousIsolationGroup]()
	{
		if (!strPreviousIsolationElement.empty())
		{
			Try_SetVisualPreviewOccurrenceIsolation(
				strPreviousIsolationElement);
			return;
		}
		if (strPreviousIsolationGroup.empty())
			return;
		const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection = nullptr == pObject ? nullptr :
				pObject->Get_SourceVisualProgramProjection();
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
			nullptr == pProjection ? nullptr :
				CEffectCatalog::Find_VisualProgram(
					pProjection->Get_EffectAssetId());
		if (nullptr == Program)
			return;
		for (int32_t iFamily = 0;
			iFamily < static_cast<int32_t>(EFFECT_VISUAL_PROGRAM_FAMILY::END);
			++iFamily)
		{
			const EFFECT_VISUAL_PROGRAM_FAMILY eFamily =
				static_cast<EFFECT_VISUAL_PROGRAM_FAMILY>(iFamily);
			if (strPreviousIsolationGroup ==
				VisualProgramFamilyLabel(eFamily))
			{
				Try_SetVisualPreviewFamilyIsolation(*Program, eFamily);
				return;
			}
		}
	};
	const auto RestorePreviousPlaybackState = [this,
		bPreviousArtistAdapterPreviewActive, fPreviousPreviewTimeSeconds,
		bPreviousPreviewPlaying, bPreviousPreviewVisibleRequested]()
	{
		const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
		if (nullptr == pObject)
			return;
		if (bPreviousArtistAdapterPreviewActive)
		{
			Seek_ReconstructedSourceRuntimeTimeline(
				fPreviousPreviewTimeSeconds);
		}
		else
		{
			m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
			pObject->Set_SampleTime(
				Resolve_EffectSampleTime(fPreviousPreviewTimeSeconds));
		}
		m_bPreviewPlaying = bPreviousPreviewPlaying;
		m_bPreviewVisibleRequested = bPreviousPreviewVisibleRequested;
		pObject->Set_Playing(bPreviousPreviewPlaying);
		pObject->Set_Visible(bPreviousPreviewVisibleRequested &&
			Is_ProductCueVisible(fPreviousPreviewTimeSeconds));
		Set_SynchronizedAnimationPaused(!bPreviousPreviewPlaying);
	};
	Clear_ProductCuePreview();
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    if (!Stage_WorldPreview(Staged))
    {
		const std::string StageError = m_strPreviewStatus;
		const bool_t bRestored = RestorePreviousSourcePreviewState();
        m_strElementStatus =
			"Element preview failed; the active Document, Data File, and builder were preserved: " +
			StageError + (bRestored ? std::string{} :
				" Previous exact Valtan preview rollback failed: " +
				ValtanRestoreError);
        return false;
    }
    const std::string_view strExpectedCanonical =
        bUsesActiveDocument &&
        EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ?
            std::string_view(m_strActiveDocumentBaselineCanonical) :
            std::string_view{};
    if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
        Path, Staged, strExpectedCanonical, Error))
    {
		const bool_t bRestored = RestorePreviousSourcePreviewState();
		if (bRestored && PreviousValtanProductPreview.has_value())
		{
			/* The exact helper already re-staged the Valtan document, sample,
			   root, visibility, and held animation pose. */
		}
		else if (bRestored && bPreviousArtistAdapterPreviewActive)
		{
			Try_StartArtist31470FullPreview();
			RestorePreviousSourceIsolation();
			RestorePreviousPlaybackState();
		}
		else if (bRestored && m_ProductPreview.has_value() &&
			m_SourcePreviewDocument.has_value())
		{
			Stage_WorldPreview(*m_SourcePreviewDocument, true);
			RestorePreviousSourceIsolation();
			RestorePreviousPlaybackState();
		}
		else if (bRestored && m_ActiveDocument.has_value())
        {
            if (m_bActiveDocumentDrawable)
			{
				Stage_WorldPreview(*m_ActiveDocument);
				RestorePreviousPlaybackState();
			}
            else
                Release_WorldPreview(true);
        }
		else if (bRestored)
            Release_WorldPreview(true);
        m_strElementStatus =
			bRestored ?
				"Element save failed; previous Document and preview were restored: " +
					Error :
				"Element save failed and the previous exact Valtan preview could not be restored: " +
					Error + " Rollback failed: " + ValtanRestoreError;
        return false;
    }

    m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(true, {});
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
    m_strActiveDocumentBaselineCanonical =
        CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
    m_bActiveDocumentMatchesRuntime = false;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
	Reset_ModelCueDraft();
    const std::string strCreatedElementId = Element.strElementId;
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = strCreatedElementId;
    m_strSelectedElementGroupId = Element.strGroupId;
    m_strSelectedComponentId.clear();
    m_strSelectedEmitterId.clear();
    m_strSelectedSourceModuleId.clear();
	m_eSelectedEffectType = Element.eKind;
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    m_strSelectedResourceAssetId.clear();
    m_strSelectedDataFileAssetId = strTargetEffectId;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    m_NewElementId[0u] = '\0';
    Recalculate_PreviewDuration();
	Synchronize_LoadedSkillPreview();
    Start_WorldPreviewFromBeginning();
    Refresh_RuntimeEquivalence();
    Refresh_DataFiles();
    Refresh_AllEffects();
    m_strElementStatus =
		std::string(bUsesActiveDocument ? "Added one " : "Created one ") +
		AuthoringFamily_Label(eAuthoringFamily) +
		" Element and saved the Effect atomically; builder selections were preserved.";
    m_strDocumentStatus = "Saved Authored atomically: " + Path.string() +
        " Live preview is active; Assembly/WFX/Runtime Catalog publish remains separate.";
    return true;
}

bool_t Client::CEffect_Tool::Try_UseSelectedElementAsAuthoringPreset()
{
	const EFFECT_ELEMENT_DESC* pSelected = Find_SelectedElement();
	if (nullptr == pSelected || !m_ActiveDocument.has_value())
	{
		m_strElementStatus = "Select one drawable Element before using a preset.";
		return false;
	}
	SOURCE_ELEMENT_PRESET_SELECTION Selection;
	Selection.strSourceEffectAssetId = m_ActiveDocument->strEffectAssetId;
	Selection.strTargetElementId = pSelected->strElementId;
	Selection.strSourceRecordId = pSelected->strSourceNode.empty() ?
		pSelected->strElementId : pSelected->strSourceNode;
	Selection.strSourceFamily =
		AuthoringFamily_Label(Resolve_AuthoringFamily(*pSelected));
	return Try_StageElementAsAuthoringPreset(
		*m_ActiveDocument, pSelected->strElementId, std::move(Selection));
}

bool_t Client::CEffect_Tool::Try_StageElementAsAuthoringPreset(
	const EFFECT_DOCUMENT_DESC& SourceDocument,
	const std::string& strElementId,
	SOURCE_ELEMENT_PRESET_SELECTION Selection)
{
	EFFECT_DOCUMENT_DESC GenericCopy;
	std::string Error;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
			SourceDocument, strElementId, "effect.authoring.preset",
			GenericCopy, Error) ||
		GenericCopy.Elements.size() != 1u)
	{
		m_strElementStatus = Error.empty() ?
			"The selected Element could not be loaded into the builder." : Error;
		return false;
	}
	EFFECT_ELEMENT_DESC Preset = GenericCopy.Elements.front();
	const EFFECT_AUTHORING_FAMILY eFamily = Resolve_AuthoringFamily(Preset);
	if (!AuthoringFamily_CanCreate(eFamily))
	{
		m_strElementStatus =
			"Presentation Light and Screen Post are edited or deleted in the active Effect; creating them from a drawable Element preset is not admitted.";
		return false;
	}
	Preset.strGroupId = "manual.hit1";
	Preset.strElementId.clear();
	Preset.strDisplayName = AuthoringFamily_Label(eFamily);
	Selection.GenericElement = Preset;
	if (Selection.strSourceFamily.empty())
		Selection.strSourceFamily = AuthoringFamily_Label(eFamily);

	/* Commit only after the complete immutable source selection and generic
	   builder copy have validated.  The current Effect document, Detail draft,
	   save baseline, and preview are intentionally not part of this transaction. */
	m_MeshAuthoringDraft = std::move(Preset);
	m_SourceElementPresetSelection = std::move(Selection);
	m_eSelectedAuthoringFamily = eFamily;
	m_eSelectedEffectType = AuthoringFamily_Kind(eFamily);
	m_bMeshAuthoringDraftInitialized = true;
	const bool_t bRequiresMesh = AuthoringFamily_RequiresMesh(eFamily);
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	m_strSelectedResourceAssetId.clear();
	m_iResourceViewRevision = UINT64_MAX;
	m_strElementStatus = std::string("Loaded one ") +
		AuthoringFamily_Label(eFamily) +
		" Source Element seed into Element Authoring. Current Effect was not changed; use Create Element, then Save Changes.";
	return true;
}

bool_t Client::CEffect_Tool::Try_BindMeshAuthoringResource(
    const std::string& strAssetId)
{
    if (!m_bMeshAuthoringDraftInitialized)
        Reset_MeshAuthoringDraft();
    if (!Slot_Allowed(m_MeshAuthoringDraft,
		m_strSelectedResourceSlotId) ||
		!AuthoringFamily_AllowsSlot(
			m_eSelectedAuthoringFamily, m_strSelectedResourceSlotId))
    {
        m_strResourceStatus =
            "Select Mesh, Base, Noise, Mask, Emissive, or Dissolve first.";
        return false;
    }
    const EFFECT_RESOURCE_FILE_KIND eExpectedKind = Slot_FileKind(
        m_MeshAuthoringDraft, m_strSelectedResourceSlotId);
    const auto CatalogEntry = std::find_if(
        m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
        [this, &strAssetId, eExpectedKind](
            const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
        {
            return Entry.strAssetId == strAssetId &&
                Entry.strDomainId == m_strSelectedAuthoringDomainId &&
                Entry.eFileKind == eExpectedKind;
        });
    if (CatalogEntry == m_ResourceCatalog.end())
    {
        m_strResourceStatus =
            "The selected resource is outside the active domain or file kind.";
        return false;
    }
    const bool_t bBaseSlot = m_strSelectedResourceSlotId ==
        EFFECT_STANDARD_MATERIAL_INPUTS[0u].strSlotId;
    const bool_t bNoiseSlot = m_strSelectedResourceSlotId ==
        EFFECT_STANDARD_MATERIAL_INPUTS[1u].strSlotId;
    if ((bBaseSlot || bNoiseSlot) &&
        Is_UnsafeEffectBaseTextureAssetId(strAssetId))
    {
        m_strResourceStatus = bBaseSlot ?
            "Base rejects blank/normal/bump textures." :
            "Noise rejects normal data without source Material distortion evidence.";
        return false;
    }

    auto Binding = std::find_if(
        m_MeshAuthoringDraft.ResourceBindings.begin(),
        m_MeshAuthoringDraft.ResourceBindings.end(),
        [this](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
        {
            return Candidate.strSlotId == m_strSelectedResourceSlotId;
        });
    if (Binding == m_MeshAuthoringDraft.ResourceBindings.end())
    {
        m_MeshAuthoringDraft.ResourceBindings.push_back(
            { m_strSelectedResourceSlotId, strAssetId });
    }
    else
    {
        Binding->strAssetId = strAssetId;
    }
    m_strResourceStatus = "Selected " + strAssetId + " for " +
        Slot_Label(m_MeshAuthoringDraft,
            m_strSelectedResourceSlotId) + ".";
    return true;
}

bool_t Client::CEffect_Tool::Try_ClearMeshAuthoringSlot()
{
    if (!m_bMeshAuthoringDraftInitialized ||
        !Slot_Allowed(m_MeshAuthoringDraft,
			m_strSelectedResourceSlotId) ||
		!AuthoringFamily_AllowsSlot(
			m_eSelectedAuthoringFamily, m_strSelectedResourceSlotId))
    {
        return false;
    }
    std::erase_if(m_MeshAuthoringDraft.ResourceBindings,
        [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.strSlotId == m_strSelectedResourceSlotId;
        });
    m_strSelectedResourceAssetId.clear();
	m_strResourceStatus = "Cleared the selected Element Authoring slot.";
    return true;
}

bool_t Client::CEffect_Tool::Try_CreateElementDraft()
{
    if (Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before creating another Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value() ||
        (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
         EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource))
    {
        m_strElementStatus =
            "Create or open one editable Current Effect before creating an Element.";
        return false;
    }
    if (m_bOccurrenceTuningDirty)
    {
        m_strElementStatus =
            "Save the occurrence tuning artifact before editing Current Effect.";
        return false;
    }

    const EFFECT_AUTHORING_FAMILY eFamily = m_eSelectedAuthoringFamily;
	const bool_t bImportedSeed = m_SourceElementPresetSelection.has_value();
	if (bImportedSeed &&
		m_SourceElementPresetSelection->strSourceEffectAssetId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		m_ActiveDocument->strEffectAssetId !=
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID)
	{
		m_strElementStatus =
			"Artist F Track A Seed belongs to the Artist F Editable Skill Effect. Open that parent before Create Element.";
		return false;
	}
    const EFFECT_ELEMENT_KIND eKind = AuthoringFamily_Kind(eFamily);
    if (!AuthoringFamily_CanCreate(eFamily) ||
        EFFECT_ELEMENT_KIND::END == eKind)
    {
		m_strElementStatus =
			"Select one drawable Element Type. Presentation Light and Screen Post creation requires source-backed materialization.";
        return false;
    }
    if (EFFECT_AUTHORING_FAMILY::MESH_PARTICLE == eFamily &&
        nullptr == Find_Binding(
            m_MeshAuthoringDraft, EFFECT_MESH_SHAPE_SLOT_ID))
    {
        m_strElementStatus =
            "Choose a WModel seed before creating Mesh Particle so its Family remains stable.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    EFFECT_ELEMENT_DESC Element = m_MeshAuthoringDraft;
    Element.eKind = eKind;
    Element.Renderer = {};
    Element.strGroupId = "manual.hit1";
    Element.strSourceNode.clear();
    Element.SourceRecipe = {};
    Element.SourcePresentation = {};
    if (!bImportedSeed)
    {
        Element.ActionCueAttachment = {};
        Element.TransformInheritance = {};
        Element.Detail.Mesh.vSourceTypeDataRotationDegrees = {};
        Element.Material.strTemplateId =
            std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
		Element.Material.eRenderProfile =
			EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
        Element.Material.strSourceMaterialPath.clear();
        Element.Material.SourceMaterial = {};
		Element.Material.Execution = {};
        Element.Detail.Mesh.bUseModelMaterial = false;
    }

    if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind &&
		!bImportedSeed &&
        Element.Detail.Particle.fSpawnRatePerSecond <= 0.f &&
        0u == Element.Detail.Particle.iBurstCount)
    {
        Element.Detail.Particle.iMaxParticles = 1u;
        Element.Detail.Particle.iBurstCount = 1u;
        Element.Detail.Particle.vLifeTimeSeconds = { 1.f, 1.f };
        if (Element.Detail.Particle.vStartSize.x <= 0.f ||
            Element.Detail.Particle.vStartSize.y <= 0.f)
        {
            Element.Detail.Particle.vStartSize = { 1.f, 1.f };
        }
    }
	float4x4_t TrailAnchorWorld{};
	const bool_t bCreateTrailFollow =
		EFFECT_ELEMENT_KIND::TRAIL == Element.eKind && !bImportedSeed &&
		!m_strPreviewAnchorSlotId.empty() &&
		(EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET == m_ePreviewPivotKind ||
		 EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE == m_ePreviewPivotKind) &&
		CAnimationTargetService::Resolve_AnchorTransform(
			m_strPreviewAnchorSlotId.c_str(), &TrailAnchorWorld);
	if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind && !bImportedSeed &&
		!bCreateTrailFollow &&
		std::abs(Element.Detail.Transform.vVelocityPerSecond.x) <= 1e-6f &&
		std::abs(Element.Detail.Transform.vVelocityPerSecond.y) <= 1e-6f &&
		std::abs(Element.Detail.Transform.vVelocityPerSecond.z) <= 1e-6f)
	{
		Element.Detail.Transform.vVelocityPerSecond = { 1.f, 0.f, 0.f };
	}

    Element.strElementId = m_NewElementId.data();
    if (Element.strElementId.empty())
    {
        const std::string Prefix = std::string(
            AuthoringFamily_ElementPrefix(eFamily)) + "_";
        size_t iCandidate = Staged.Elements.size() + 1u;
        do
        {
            Element.strElementId = Prefix + std::to_string(iCandidate++);
        }
        while (std::any_of(Staged.Elements.begin(), Staged.Elements.end(),
            [&Element](const EFFECT_ELEMENT_DESC& Existing)
            {
                return Existing.strElementId == Element.strElementId;
            }));
    }
    Element.strDisplayName = Element.strElementId;
	if (bCreateTrailFollow)
	{
		Element.ActionCueAttachment.bEnabled = true;
		Element.ActionCueAttachment.bFollow = true;
		Element.ActionCueAttachment.strSourceAnchorSlotId =
			m_strPreviewAnchorSlotId;
		Element.ActionCueAttachment.strRuntimeAnchorSlotId =
			Element.strElementId;
		Element.ActionCueAttachment.strRuntimeBoneName =
			m_strPreviewAnchorSlotId;
	}
    if (std::any_of(Staged.Elements.begin(), Staged.Elements.end(),
        [&Element](const EFFECT_ELEMENT_DESC& Existing)
        {
            return Existing.strElementId == Element.strElementId;
        }))
    {
        m_strElementStatus =
            "Create Element rejected a duplicate Element ID.";
        return false;
    }

    std::set<std::string> Slots;
    for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
        Element.ResourceBindings)
    {
        if (!Slots.insert(Binding.strSlotId).second ||
            !Slot_Allowed(Element, Binding.strSlotId) ||
            !AuthoringFamily_AllowsSlot(eFamily, Binding.strSlotId))
        {
            m_strElementStatus =
                "Create Element rejected a duplicate or unsupported resource slot.";
            return false;
        }
    }

    Staged.Elements.push_back(Element);
    if (!Try_CommitDocument(std::move(Staged)))
        return false;

    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    Reset_MeshAuthoringDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = Element.strElementId;
    m_strSelectedElementGroupId = Element.strGroupId;
    m_strSelectedComponentId.clear();
    m_strSelectedEmitterId.clear();
    m_strSelectedSourceModuleId.clear();
    const bool_t bHasMesh = nullptr != Find_Binding(
        Element, EFFECT_MESH_SHAPE_SLOT_ID);
    m_strSelectedResourceSlotId =
        AuthoringFamily_RequiresMesh(eFamily) && !bHasMesh ?
            std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
            std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
    m_eResourceLibraryFileKind = Slot_FileKind(
        Element, m_strSelectedResourceSlotId);
    m_strSelectedResourceAssetId.clear();
    m_NewElementId[0u] = '\0';
    if (m_bActiveDocumentDrawable)
        Start_WorldPreviewFromBeginning();
    m_strElementStatus = "Created one unsaved " +
        std::string(AuthoringFamily_Label(eFamily)) +
        " Element in Current Effect. Bind WModel/DDS slots and tune Details, then use Save Changes.";
	if (bImportedSeed && !m_bActiveDocumentDrawable)
	{
		m_strElementStatus +=
			" The Track A data was normalized to the standard renderer and needs a generic slot binding before preview: " +
			m_strActiveDocumentDrawableError;
	}
    m_strDocumentStatus =
        "Current Effect has unsaved Element changes; no Data File was written.";
    return true;
}

bool_t Client::CEffect_Tool::Try_DuplicateSelectedElement()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strElementStatus =
			"Apply or Revert the open Detail draft before duplicating an Element.";
		return false;
	}
	if (!m_ActiveDocument.has_value() ||
		(EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource &&
		 EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource) ||
		m_strSelectedElementId.empty())
	{
		m_strElementStatus =
			"Open an authored Effect and select one Element to duplicate.";
		return false;
	}

	const auto Selected = std::find_if(
		m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == m_strSelectedElementId;
		});
	if (Selected == m_ActiveDocument->Elements.end())
	{
		m_strElementStatus =
			"The selected Element no longer exists; nothing was duplicated.";
		return false;
	}
	if (!AuthoringFamily_CanCreate(Resolve_AuthoringFamily(*Selected)))
	{
		m_strElementStatus =
			"Presentation Light and Screen Post duplication is not admitted; use source-backed materialization or edit/delete the existing occurrence.";
		return false;
	}
	if (Selected->TransformInheritance.bEnabled)
	{
		const std::string& strMasterId =
			Selected->TransformInheritance.strMasterElementId;
		if (strMasterId == Selected->strElementId ||
			std::none_of(m_ActiveDocument->Elements.begin(),
				m_ActiveDocument->Elements.end(),
				[&strMasterId](const EFFECT_ELEMENT_DESC& Element)
				{
					return Element.strElementId == strMasterId;
				}))
		{
			m_strElementStatus =
				"Duplicate rejected an invalid or self-referential transform inheritance master.";
			return false;
		}
	}
	const std::string SourceElementId = Selected->strElementId;

	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	const std::string Prefix = "authored.copy.";
	std::string DuplicateId;
	for (size_t iCopy = 1u; iCopy <= Staged.Elements.size() + 1u; ++iCopy)
	{
		const std::string Suffix = "." + std::to_string(iCopy);
		const size_t iMaximumSourceLength = 128u - Prefix.size() - Suffix.size();
		DuplicateId = Prefix +
			SourceElementId.substr(0u, iMaximumSourceLength) + Suffix;
		if (std::none_of(Staged.Elements.begin(), Staged.Elements.end(),
			[&DuplicateId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == DuplicateId;
			}))
		{
			break;
		}
		DuplicateId.clear();
	}
	if (DuplicateId.empty())
	{
		m_strElementStatus =
			"Duplicate could not allocate a unique authored Element ID.";
		return false;
	}

	EFFECT_ELEMENT_DESC Duplicate = *Selected;
	Duplicate.strElementId = DuplicateId;
	Duplicate.strSourceNode = "authored-copy:" + SourceElementId;
	Duplicate.SourcePresentation = {};
	const size_t iSelectedIndex = static_cast<size_t>(
		std::distance(m_ActiveDocument->Elements.begin(), Selected));
	Staged.Elements.insert(
		Staged.Elements.begin() + static_cast<ptrdiff_t>(iSelectedIndex + 1u),
		Duplicate);

	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::MUTE_SELECTED == m_ePreviewFilter)
	{
		m_strPreviewIsolationElementId = DuplicateId;
	}
	if (!Try_CommitDocument(std::move(Staged)))
	{
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		return false;
	}

	Reset_DetailDraft();
	m_MarkedElementIds.clear();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
	m_strSelectedElementId = DuplicateId;
	m_strSelectedElementGroupId = Duplicate.strGroupId;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_eSelectedEffectType = Duplicate.eKind;
	m_strSelectedResourceSlotId = Default_SlotId(Duplicate.eKind);
	m_eResourceLibraryFileKind = Slot_FileKind(
		Duplicate, m_strSelectedResourceSlotId);
	m_strSelectedResourceAssetId.clear();
	if (m_bActiveDocumentDrawable && m_bPreviewVisibleRequested)
		Start_WorldPreviewFromBeginning();
	m_strElementStatus = "Duplicated '" + SourceElementId +
		"' as '" + DuplicateId +
		"'; timing, material, resources, attachment, and source recipe were preserved.";
	m_strDocumentStatus =
		"Current Effect has one unsaved duplicated Element; no Data File was written.";
	return true;
}

bool_t Client::CEffect_Tool::Try_DeleteSelectedElement()
{
    if (Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before deleting an Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value() ||
        (m_MarkedElementIds.empty() && m_strSelectedElementId.empty()))
    {
        m_strElementStatus = "Select one Element to delete.";
        return false;
    }
    /* Marked rows are the delete set when there are any; otherwise the single
       open Element stays the target so every existing caller behaves the same. */
    const std::set<std::string, std::less<>> Targets =
        m_MarkedElementIds.empty() ?
            std::set<std::string, std::less<>>{ m_strSelectedElementId } :
            m_MarkedElementIds;
    if (Targets.size() >= m_ActiveDocument->Elements.size())
    {
        m_strElementStatus =
            "Deleting every Element would leave no drawable Effect; keep at least one.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    const auto NewEnd = std::remove_if(
        Staged.Elements.begin(), Staged.Elements.end(),
        [&Targets](const EFFECT_ELEMENT_DESC& Element)
        {
            return Targets.contains(Element.strElementId);
        });
    if (NewEnd == Staged.Elements.end())
        return false;
    const size_t iRemovedCount = static_cast<size_t>(
        std::distance(NewEnd, Staged.Elements.end()));
    Staged.Elements.erase(NewEnd, Staged.Elements.end());
    if (Targets.contains(m_strSelectedElementId))
        m_strSelectedElementId.clear();
    m_MarkedElementIds.clear();
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	if (!m_strPreviewIsolationElementId.empty() &&
		std::none_of(Staged.Elements.begin(), Staged.Elements.end(),
			[this](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId ==
					m_strPreviewIsolationElementId;
			}))
	{
		m_strPreviewIsolationElementId.clear();
		if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ||
			EFFECT_PREVIEW_FILTER::MUTE_SELECTED == m_ePreviewFilter)
		{
			m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		}
	}
	if (!m_strPreviewIsolationGroupId.empty() &&
		std::none_of(Staged.Elements.begin(), Staged.Elements.end(),
			[this](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strGroupId ==
					m_strPreviewIsolationGroupId;
			}))
	{
		m_strPreviewIsolationGroupId.clear();
		if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
			EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter)
		{
			m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		}
	}
    if (!Try_CommitDocument(std::move(Staged)))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
        return false;
	}
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strElementStatus = 1u == iRemovedCount ?
        "Deleted the selected Element." :
        "Deleted " + std::to_string(iRemovedCount) + " marked Elements.";
    return true;
}

bool_t Client::CEffect_Tool::Try_ClearElements()
{
    if (Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before clearing Elements.";
        return false;
    }
    if (!m_ActiveDocument.has_value())
        return false;
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.Elements.clear();
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::MUTE_SELECTED == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter)
	{
		m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	}
    if (!Try_CommitDocument(std::move(Staged)))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
        return false;
	}
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
    m_strElementStatus = "Cleared all visual layers.";
    return true;
}

size_t Client::CEffect_Tool::Count_ProductCueMappings(
	const std::string& strEffectAssetId) const
{
	if (strEffectAssetId.empty())
		return 0u;

	size_t iMappingCount = 0u;
	for (const EFFECT_SKILL_TREE_ENTRY& Entry : m_AllEffects)
	{
		if (Entry.Skill.eCharacterClass ==
			LostArk::Shared::CHARACTER_CLASS_ID::END)
		{
			continue;
		}
		iMappingCount += static_cast<size_t>(std::count_if(
			Entry.ProductCues.begin(), Entry.ProductCues.end(),
			[&strEffectAssetId](
				const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue)
			{
				return ProductCue.Cue.strEffectAssetId == strEffectAssetId;
			}));
	}
	const auto BossMappings =
		m_BossProductCueMappingCounts.find(strEffectAssetId);
	if (BossMappings != m_BossProductCueMappingCounts.end())
		iMappingCount += BossMappings->second;
	return iMappingCount;
}

bool_t Client::CEffect_Tool::Can_HotReloadSavedProduct() const
{
	return m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		!m_ActiveDocumentPath.empty() &&
		m_bActiveDocumentDrawable &&
		CEffectCatalog::Is_DirectAuthoredDocument(
			m_ActiveDocument->strEffectAssetId) &&
		0u != Count_ProductCueMappings(
			m_ActiveDocument->strEffectAssetId);
}

bool_t Client::CEffect_Tool::Try_HotReloadSavedProduct()
{
	m_strSaveHotReloadStatus.clear();
	if (!m_ActiveDocument.has_value() ||
		EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource ||
		m_ActiveDocumentPath.empty())
	{
		m_strSaveHotReloadStatus =
			"Authored save only: there is no saved Authored document to reload.";
		return false;
	}

	const std::string& strEffectAssetId =
		m_ActiveDocument->strEffectAssetId;
	if (!m_bActiveDocumentDrawable)
	{
		m_strSaveHotReloadStatus =
			"Authored save only: the structurally valid partial draft is not drawable, so the existing Product runtime was preserved.";
		return false;
	}
	const size_t iMappingCount =
		Count_ProductCueMappings(strEffectAssetId);
	if (0u == iMappingCount)
	{
		m_strSaveHotReloadStatus =
			"Authored save only: the exact Effect ID is not used by any Product cue.";
		return false;
	}
	if (!CEffectCatalog::Is_DirectAuthoredDocument(strEffectAssetId))
	{
		m_strSaveHotReloadStatus =
			"Authored save only: the exact Product Effect is not cataloged as direct-authored.";
		return false;
	}

	std::string ReloadStatus;
	if (!CEffectPresentationService::Reload_SelectedProductEffect(
			m_pDevice, m_pContext, strEffectAssetId,
			m_ActiveDocumentPath, ReloadStatus))
	{
		m_bActiveDocumentMatchesRuntime = false;
		m_strSaveHotReloadStatus =
			"Saved Authored; Product hot reload failed for '" +
			strEffectAssetId + "': " + ReloadStatus +
			" The existing prepared Product target and active occurrences were preserved.";
		return false;
	}

	Refresh_RuntimeEquivalence();
	bool_t bPreviewStaged = false;
	if (m_ProductPreview.has_value() &&
		m_ProductPreview->ProductCue.Cue.strEffectAssetId == strEffectAssetId)
	{
		const std::shared_ptr<const
			EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> pProjection =
			CEffectCatalog::Find_VisualProjection_Loaded(strEffectAssetId);
		const std::shared_ptr<const EFFECT_DOCUMENT_DESC> pRuntimeDocument =
			CEffectCatalog::Find_Loaded(strEffectAssetId);
		const EFFECT_DOCUMENT_DESC* pPreviewDocument =
			nullptr != pProjection ? &pProjection->Get_Document() :
				pRuntimeDocument.get();
		if (nullptr != pPreviewDocument)
		{
			m_SourcePreviewDocument = *pPreviewDocument;
			Recalculate_PreviewDuration(*m_SourcePreviewDocument);
			Synchronize_LoadedSkillPreview();
			bPreviewStaged = Stage_WorldPreview(
				*m_SourcePreviewDocument, true);
		}
	}
	else
	{
		Recalculate_PreviewDuration(*m_ActiveDocument);
		bPreviewStaged = Stage_WorldPreview(*m_ActiveDocument);
	}

	if (!bPreviewStaged)
	{
		m_strSaveHotReloadStatus =
			"Saved Authored and hot reloaded Product '" +
			strEffectAssetId + "' for " + std::to_string(iMappingCount) +
			" mapped cue(s), but the Tool preview could not be restaged: " +
			m_strPreviewStatus +
			" Subsequent Product spawns still use the saved revision.";
		return false;
	}

	Start_WorldPreviewFromBeginning();
	if (!m_bPreviewPlaying || nullptr == m_pWorldPreviewObject.lock())
	{
		m_strSaveHotReloadStatus =
			"Saved Authored and hot reloaded Product '" +
			strEffectAssetId + "' for " + std::to_string(iMappingCount) +
			" mapped cue(s), but the Tool preview restart failed. "
			"Subsequent Product spawns still use the saved revision.";
		return false;
	}

	m_strSaveHotReloadStatus =
		"Saved Authored and hot reloaded Product '" + strEffectAssetId +
		"' for all " + std::to_string(iMappingCount) +
		" mapped cue(s). The Tool preview restarted; active gameplay "
		"occurrences are unchanged and subsequent spawns use the saved revision.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyDraftAndSave()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strDocumentStatus = "There is no active Document to save.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource &&
		EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource)
	{
		m_strDocumentStatus =
			"Promote or Save As this view before applying an Authored save.";
		return false;
	}

	const bool_t bParticleDraft = m_bParticleSystemDraftDirty;
	const bool_t bDetailDraft = m_bDetailDraftDirty;
	const bool_t bModelCueDraft = m_bModelCueDraftDirty;
	if (bParticleDraft || bDetailDraft || bModelCueDraft)
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if ((bParticleDraft && !Apply_ParticleSystemDraft(Staged)) ||
			(bDetailDraft && !Apply_DetailDraft(Staged)) ||
			(bModelCueDraft && !Apply_ModelCueDraft(Staged)) ||
			!Try_CommitDocument(std::move(Staged)))
		{
			m_strDocumentStatus =
				"Apply + Save rejected; active file and preview were preserved.";
			return false;
		}
		if (bParticleDraft)
		{
			m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
			m_bParticleSystemDraftDirty = false;
		}
		if (bDetailDraft)
		{
			if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
			{
				m_DetailDraft = *pCommitted;
				Refresh_DetailDraftAdmission(*pCommitted);
			}
			m_bDetailDraftDirty = false;
		}
		if (bModelCueDraft)
		{
			if (const EFFECT_MODEL_CUE_DESC* pCommitted = Find_SelectedModelCue())
				m_ModelCueDraft = *pCommitted;
			m_bModelCueDraftDirty = false;
		}
	}
	if (!m_bDocumentDirty)
	{
		m_strDocumentStatus = "The active Authored Document is already saved.";
		if (m_bActiveDocumentMatchesRuntime &&
			Can_HotReloadSavedProduct())
		{
			m_strSaveHotReloadStatus =
				"Saved Authored already matches the loaded Product revision; no hot reload was needed.";
		}
		else
		{
			(void)Try_HotReloadSavedProduct();
		}
		return true;
	}
	const bool_t bSaved = Try_SaveDocument();
	if (bSaved)
	{
		if (m_bActiveDocumentDrawable && m_bPreviewVisibleRequested &&
			nullptr != m_pWorldPreviewObject.lock())
		{
			m_strDetailStatus =
				"Applied and saved Authored; live world preview is active.";
		}
		else
		{
			m_strDetailStatus =
				"Applied and saved a structurally valid partial draft; world preview is hidden and Product hot reload is blocked: " +
				m_strActiveDocumentDrawableError;
		}
	}
	return bSaved;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
	if (EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
		return Try_SaveRuntimeOccurrenceTuning();
    if (!m_ActiveDocument.has_value())
    {
        m_strDocumentStatus = "There is no active Document to save.";
        return false;
    }
    if (Has_UnappliedDetailDraft())
    {
        m_strDocumentStatus =
            "Apply or Revert the open Detail draft before saving the Document.";
        return false;
    }
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(
            m_ActiveDocument->strEffectAssetId).wstring() +
            L".effect.json"));
    if (EFFECT_DOCUMENT_SOURCE::IMPORTED == m_eActiveDocumentSource)
    {
        m_strDocumentStatus =
            "Imported Effect must use Save As to create a unique Authored ID.";
        return false;
    }
	if (EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE ==
		m_eActiveDocumentSource)
	{
		m_strDocumentStatus =
			"Legacy/Rollback migration references are immutable; use Save As for a new Authored Effect.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource)
	{
		m_strDocumentStatus =
			"Runtime Assembly/WFX/Visual Program views are immutable copies; use Save As for a new Authored Effect.";
		return false;
	}
    if (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource &&
        std::filesystem::is_regular_file(Path))
    {
        m_strDocumentStatus =
            "Save refuses to replace an existing Authored file from New; use Save As.";
        return false;
    }
    std::string Error;
    if (Path.empty() || !CEffectDocumentCodec::Save_AtomicIfUnchanged(
        Path, *m_ActiveDocument,
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ?
			std::string_view(m_strActiveDocumentBaselineCanonical) :
			std::string_view{},
		Error))
    {
        m_strDocumentStatus = Path.empty() ?
            "Effect authoring path escaped Data/Effects/Authored." : Error;
        return false;
    }
    m_bDocumentDirty = false;
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
	m_bActiveDocumentMatchesRuntime = false;
    m_strSelectedDataFileAssetId =
        m_ActiveDocument->strEffectAssetId;
    m_strDocumentStatus = "Saved Authored atomically: " + Path.string();
	if (!m_bActiveDocumentDrawable)
	{
		m_strDocumentStatus +=
			" Structurally valid partial draft saved; world preview hidden and Product hot reload blocked: " +
			m_strActiveDocumentDrawableError;
	}
    else
    {
        m_strDocumentStatus += " Authored preview is drawable.";
    }
	/* All Effects may have cached this Product while it was still an empty,
	   non-drawable authoring shell. Revalidate the exact saved path now so its
	   Play gate reflects the first authored Element without requiring a manual
	   pattern-tree Refresh. */
	const auto ValtanCache = m_ValtanUnifiedEffectCaches.find(
		m_ActiveDocument->strEffectAssetId);
	if (ValtanCache != m_ValtanUnifiedEffectCaches.end())
	{
		ValtanCache->second = {};
		(void)Refresh_UnifiedEffectCache(ValtanCache->second, Path,
			m_ActiveDocument->strEffectAssetId);
	}
	(void)Try_HotReloadSavedProduct();
    return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocumentAs(
    const std::string& strAssetId)
{
    if (!m_ActiveDocument.has_value())
    {
        m_strDocumentStatus = "There is no active Document to save.";
        return false;
    }
    if (Has_UnappliedDetailDraft())
    {
        m_strDocumentStatus =
            "Apply or Revert the open Detail draft before Save As.";
        return false;
    }
	const bool_t bWasVisualProgramCopy =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource;
	const bool_t bAdapterPacketVisualCopy = bWasVisualProgramCopy &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (bAdapterPacketVisualCopy)
	{
		m_strDocumentStatus =
			"Adapter-packet Visual Programs cannot be saved through the ordinary Authored Effect codec because that would discard the exact projector/VF/resource packet. Use stable occurrence Transform Save/Reload, or create a separately validated generic authored starting copy.";
		return false;
	}
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.strEffectAssetId = strAssetId;
    std::string Error;
    if (!CEffectDocumentCodec::Validate(Staged, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(strAssetId).wstring() + L".effect.json"));
    if (Path.empty())
    {
        m_strDocumentStatus =
            "Effect Save As path escaped Data/Effects/Authored.";
        return false;
    }
    if (std::filesystem::is_regular_file(Path))
    {
        m_strDocumentStatus =
            "Save As refuses to overwrite an existing Effect ID.";
        return false;
    }
    if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
		Path, Staged, std::string_view{}, Error))
    {
        m_strDocumentStatus = Error;
        return false;
    }
    const bool_t bWasDrawable = m_bActiveDocumentDrawable;
    std::string PreviousDrawableError = m_strActiveDocumentDrawableError;
    Clear_ProductCuePreview();
    m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(
        bWasDrawable, std::move(PreviousDrawableError));
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	if (bWasVisualProgramCopy)
		m_pSelectedVisualSourceProjection.reset();
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
	m_strSaveHotReloadStatus.clear();
    Refresh_RuntimeEquivalence();
    m_strSelectedDataFileAssetId = strAssetId;
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), strAssetId);
    Refresh_DataFiles();
    Refresh_AllEffects();
    m_strDocumentStatus = "Saved new Authored Effect atomically: " +
        Path.string() +
		" World preview updated; Assembly/WFX/Runtime Catalog publish pending.";
	if (bWasVisualProgramCopy)
	{
		m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		if (m_bActiveDocumentDrawable && Stage_WorldPreview(*m_ActiveDocument))
		{
			m_strDocumentStatus +=
				" The preview now uses the generic Authored renderer path.";
		}
		else if (m_bActiveDocumentDrawable)
		{
			m_strDocumentStatus +=
				" The file was saved, but generic Authored preview staging failed; the previous preview was preserved.";
		}
	}
    return true;
}

bool_t Client::CEffect_Tool::
	Try_SaveSelectedAdapterElementAsGenericAuthoredCopy(
		const std::string& strAssetId)
{
	if (!m_ActiveDocument.has_value() ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM !=
			m_eActiveDocumentSource ||
		nullptr == m_pSelectedVisualSourceProjection ||
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		m_strSelectedElementId.empty())
	{
		m_strDocumentStatus =
			"Select one admitted adapter Decal or Trail Element before creating a generic Authored starting copy.";
		return false;
	}
	if (Has_UnappliedDetailDraft())
	{
		m_strDocumentStatus =
			"Close or revert the open inspection draft before creating a generic Authored starting copy.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged;
	std::string Error;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
			m_pSelectedVisualSourceProjection->Get_Document(),
			m_strSelectedElementId, strAssetId, Staged, Error))
	{
		m_strDocumentStatus =
			"Generic Authored starting copy rejected: " + Error;
		return false;
	}
	const std::filesystem::path Path = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored" /
		(std::filesystem::path(strAssetId).wstring() + L".effect.json"));
	if (Path.empty())
	{
		m_strDocumentStatus =
			"Generic Authored starting copy path escaped Data/Effects/Authored.";
		return false;
	}
	if (std::filesystem::is_regular_file(Path))
	{
		m_strDocumentStatus =
			"Generic Authored starting copy refuses to overwrite an existing Effect ID.";
		return false;
	}
	CEffectDocumentRenderer StagingRenderer(m_pDevice, m_pContext);
	if (FAILED(StagingRenderer.Initialize()) ||
		!StagingRenderer.Stage_Document(Staged, Error))
	{
		m_strDocumentStatus =
			"Generic Authored starting copy preview preflight failed; the active Effect and Data File were preserved: " +
			Error;
		return false;
	}
	if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
		Path, Staged, std::string_view{}, Error))
	{
		m_strDocumentStatus = Error;
		return false;
	}

	Clear_ProductCuePreview();
	m_ActiveDocument = std::move(Staged);
	Set_ActiveDocumentDrawableStatus(true, {});
	m_ActiveDocumentPath = Path;
	m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_pSelectedVisualSourceProjection.reset();
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
	m_bDocumentDirty = false;
	m_bActiveDocumentMatchesRuntime = false;
	m_strSelectedDataFileAssetId = strAssetId;
	Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), strAssetId);
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	Recalculate_PreviewDuration();
	Refresh_RuntimeEquivalence();
	Refresh_DataFiles();
	Refresh_AllEffects();
	m_strDocumentStatus =
		"Saved one selected Decal/Trail as a generic Authored starting copy: " +
		Path.string() +
		". Detail, Material, and resource bindings were preserved through the ordinary codec; the exact adapter projector/VF/resource packet was intentionally not copied. The preview now uses the generic Authored renderer and can be edited/saved/reloaded normally.";
	return true;
}

bool_t Client::CEffect_Tool::Try_PromoteImportedDocument()
{
    if (!m_ActiveDocument.has_value() ||
        EFFECT_DOCUMENT_SOURCE::IMPORTED != m_eActiveDocumentSource)
    {
        m_strDocumentStatus =
            "Load an executable Imported Effect before promotion.";
        return false;
    }
    if (Has_UnappliedDetailDraft())
    {
        m_strDocumentStatus =
            "Apply or Revert the open Detail draft before promotion.";
        return false;
    }
    constexpr std::string_view Suffix = ".imported";
    const std::string& ImportedId = m_ActiveDocument->strEffectAssetId;
    if (ImportedId.size() <= Suffix.size() ||
        0 != ImportedId.compare(
            ImportedId.size() - Suffix.size(), Suffix.size(), Suffix))
    {
        m_strDocumentStatus =
            "Imported Effect ID does not have the canonical .imported suffix.";
        return false;
    }
    const std::string TargetId = ImportedId.substr(
        0u, ImportedId.size() - Suffix.size());
    std::string CatalogStatus;
    if (!Ensure_PlayerSkillCatalog(CatalogStatus))
    {
        m_strDocumentStatus = "PlayerSkills load failed: " + CatalogStatus;
        return false;
    }
    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    const auto Skill = std::find_if(
        Skills.begin(), Skills.end(),
        [&TargetId](const PLAYER_SKILL_DEFINITION& Candidate)
        {
            return Candidate.strEffectId == TargetId;
        });
    if (Skill == Skills.end())
    {
        m_strDocumentStatus =
            "Promotion target is not owned by PlayerSkills: " + TargetId;
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    Staged.strEffectAssetId = TargetId;
    Staged.strDisplayName = Skill->strDisplayName;
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(TargetId).wstring() + L".effect.json"));
    std::string Error;
    std::string ExpectedCanonicalDocument;
	if (!Path.empty() && std::filesystem::is_regular_file(Path))
    {
        EFFECT_DOCUMENT_DESC Existing;
        if (!CEffectDocumentCodec::Load(Path, Existing, Error) ||
            Existing.strEffectAssetId != TargetId)
        {
            m_strDocumentStatus =
                "Existing Authored Effect could not preserve its Model Cues: " +
                Error;
            return false;
        }
		ExpectedCanonicalDocument = CEffectDocumentCodec::Serialize(Existing);
		if (Staged.ModelCues.empty())
			Staged.ModelCues = std::move(Existing.ModelCues);
    }
    if (!CEffectDocumentCodec::Validate_Drawable(Staged, Error))
    {
        m_strDocumentStatus = "Promotion validation failed: " + Error;
        return false;
    }
    if (!Stage_WorldPreview(Staged))
    {
        m_strDocumentStatus =
            "Promotion preview stage failed; existing Authored file preserved: " +
            m_strPreviewStatus;
        return false;
    }
    if (Path.empty() || !CEffectDocumentCodec::Save_AtomicIfUnchanged(
		Path, Staged, ExpectedCanonicalDocument, Error))
    {
        Stage_WorldPreview(*m_ActiveDocument);
        m_strDocumentStatus = Path.empty() ?
            "Promotion path escaped Data/Effects/Authored." :
            "Promotion failed; existing Authored file restored: " + Error;
        return false;
    }
    Clear_ProductCuePreview();
    m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(true, {});
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
	m_strSaveHotReloadStatus.clear();
    Refresh_RuntimeEquivalence();
    m_strSelectedDataFileAssetId = TargetId;
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), TargetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(),
        m_ActiveDocument->strDisplayName);
    Refresh_DataFiles();
    Refresh_AllEffects();
    Start_WorldPreviewFromBeginning();
    m_strDocumentStatus =
        "Promoted Imported Effect to Authored skill atomically: " +
        Path.string();
    return true;
}

bool_t Client::CEffect_Tool::Try_ReloadActiveDocument()
{
	const bool_t bRuntimeView =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource;
	if (!m_ActiveDocument.has_value() ||
		(m_ActiveDocumentPath.empty() && !bRuntimeView))
    {
        m_strDocumentStatus = "The active Effect has no saved source to reload.";
        return false;
    }
    if (Has_UnsavedWork())
    {
        m_strDocumentStatus =
            "Save or Discard changes before Reload.";
        return false;
    }
	const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanPreview =
		m_ValtanProductPreview;
	const f32_t fPreviousPreviewTimeSeconds = m_fPreviewTimeSeconds;
	const f32_t fPreviousPreviewDurationSeconds =
		m_fPreviewDurationSeconds;
	const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
	const bool_t bPreviousPreviewVisibleRequested =
		m_bPreviewVisibleRequested;
	const float4x4_t PreviousProductCueSnapshotRoot =
		m_ProductCueSnapshotRoot;
	const bool_t bPreviousProductCueSnapshotCaptured =
		m_bProductCueSnapshotCaptured;
	std::string ValtanRestoreError;
	/* Parse/identity/drawability checks in Try_LoadDocumentPath all precede its
	   Release_WorldPreview commit boundary. A failed load therefore leaves the
	   current exact Valtan object and pose untouched; only a successful document
	   commit needs the full restore transaction below. */
	if (!Try_LoadDocumentPath(
		m_ActiveDocumentPath, m_eActiveDocumentSource,
			bRuntimeView ? m_strSelectedDataFileAssetId :
				m_ActiveDocument->strEffectAssetId))
	{
		return false;
	}
	if (!PreviousValtanPreview.has_value())
		return true;

	/* The saved document commit succeeded.  Rebuild its exact Valtan cue
	   duration, then restore the previous wall clock and held pose. */
	m_ValtanProductPreview = PreviousValtanPreview;
	Recalculate_PreviewDuration();
	const f32_t fReloadedPreviewDurationSeconds =
		m_fPreviewDurationSeconds;
	if (Restore_ValtanProductPreviewPlayback(
			PreviousValtanPreview,
			fPreviousPreviewTimeSeconds,
			fReloadedPreviewDurationSeconds,
			bPreviousPreviewPlaying,
			bPreviousPreviewVisibleRequested,
			PreviousProductCueSnapshotRoot,
			bPreviousProductCueSnapshotCaptured,
			ValtanRestoreError))
	{
		return true;
	}

	m_ValtanProductPreview.reset();
	m_strDocumentStatus +=
		" The document reload committed, but its Valtan cue preview could "
		"not be replayed; the saved document remains loaded. Preview restore "
		"failed: " + ValtanRestoreError;
	return true;
}

bool_t Client::CEffect_Tool::Try_LoadDocument(
    const std::string& strAssetId)
{
    const std::filesystem::path Path = CProjectDataRoot::Resolve(
        std::filesystem::path(L"Effects") / L"Authored" /
        (std::filesystem::path(strAssetId).wstring() + L".effect.json"));
    return Try_LoadDocumentPath(
        Path, EFFECT_DOCUMENT_SOURCE::AUTHORED, strAssetId);
}

bool_t Client::CEffect_Tool::Try_LoadDocumentPath(
    const std::filesystem::path& Path,
    const EFFECT_DOCUMENT_SOURCE eSource,
    const std::string& strSelectionId)
{
    return Try_LoadDocumentPathStaged(
        Path, eSource, strSelectionId, false);
}

bool_t Client::CEffect_Tool::Try_LoadDocumentPathStaged(
    const std::filesystem::path& Path,
    const EFFECT_DOCUMENT_SOURCE eSource,
    const std::string& strSelectionId,
    const bool_t bBypassUnsavedGuard)
{
    Engine::CProfilerScope LoadProfile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.DocumentLoad");
    if (EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE == eSource)
    {
        m_strDocumentStatus =
            "Extraction drafts are reference-only and cannot be played until "
            "they are converted to a validated Effect Document.";
        return false;
    }
    if (!bBypassUnsavedGuard && Has_UnsavedWork())
    {
        m_PendingDocumentLoad = PENDING_DOCUMENT_LOAD{
            Path, strSelectionId, eSource };
        m_bPendingDocumentLoadModalRequested = true;
        m_strDocumentStatus =
            "Pending Effect load requires Save, Discard, or Cancel.";
        return false;
    }
	EFFECT_DOCUMENT_DESC Staged;
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pStagedVisualProjection;
	std::string Error;
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == eSource)
	{
		constexpr std::string_view Suffix = "::assembly";
		if (!strSelectionId.ends_with(Suffix))
		{
			m_strDocumentStatus = "Runtime Assembly selection ID is invalid.";
			return false;
		}
		const std::string EffectId = strSelectionId.substr(
			0u, strSelectionId.size() - Suffix.size());
		const std::shared_ptr<const EFFECT_DOCUMENT_DESC> Runtime =
			CEffectCatalog::Find(EffectId);
		if (nullptr == Runtime)
		{
			m_strDocumentStatus =
				"Runtime Assembly is no longer admitted by EffectCatalog: " + EffectId;
			return false;
		}
		Staged = *Runtime;
	}
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == eSource)
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(strSelectionId);
		if (nullptr == Component)
		{
			m_strDocumentStatus =
				"WFX Component is no longer admitted by EffectCatalog: " +
				strSelectionId;
			return false;
		}
		Staged = Component->Document;
	}
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM == eSource)
	{
		pStagedVisualProjection =
			CEffectCatalog::Find_VisualProjection(strSelectionId);
		if (nullptr == pStagedVisualProjection)
		{
			m_strDocumentStatus =
				"Visual Program is no longer admitted by EffectCatalog: " +
				strSelectionId;
			return false;
		}
		Staged = pStagedVisualProjection->Get_Document();
	}
    else
    {
        if (Path.empty())
        {
            m_strDocumentStatus =
                "Effect load path escaped Data/Effects/Authored.";
            return false;
        }
        Engine::CProfilerScope ParseProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DocumentLoad.Parse");
        if (!CEffectDocumentCodec::Load(Path, Staged, Error))
        {
            m_strDocumentStatus = Error;
            return false;
        }
		if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource &&
			Staged.strEffectAssetId != strSelectionId)
		{
			m_strDocumentStatus =
				"Authored Effect identity mismatch; selected '" + strSelectionId +
				"', file contains '" + Staged.strEffectAssetId +
				"'. The previous Current Effect was preserved.";
			return false;
		}
    }
    std::string PreviewStatus;
    bool_t bDrawable = false;
    {
        Engine::CProfilerScope ValidationProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DocumentLoad.ValidateDrawable");
        bDrawable =
            CEffectDocumentCodec::Validate_Drawable(Staged, PreviewStatus);
    }
	Reset_RuntimeOccurrenceTuningSession();
	m_pSelectedVisualSourceProjection = std::move(pStagedVisualProjection);
    Release_WorldPreview(true);
    std::string CanonicalBaseline;
	if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource)
    {
        Engine::CProfilerScope CanonicalProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DocumentLoad.CanonicalBaseline");
        CanonicalBaseline = CEffectDocumentCodec::Serialize(Staged);
    }
	optional<EFFECT_PRODUCT_PREVIEW> RetainedProductPreview;
	if (m_ProductPreview.has_value() &&
		m_ProductPreview->ProductCue.Cue.strEffectAssetId ==
			Staged.strEffectAssetId)
	{
		RetainedProductPreview = m_ProductPreview;
	}
	Clear_ProductCuePreview();
	if (RetainedProductPreview.has_value())
		m_ProductPreview = std::move(RetainedProductPreview);
	m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(bDrawable, PreviewStatus);
    m_ActiveDocumentPath = Path;
	m_strActiveDocumentBaselineCanonical = CanonicalBaseline;
    m_eActiveDocumentSource = eSource;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
	Reset_ModelCueDraft();
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == eSource)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::SKILL;
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == eSource)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::COMPONENT;
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM == eSource)
		m_eDetailSelection = m_ActiveDocument->Elements.empty() ?
			EFFECT_DETAIL_SELECTION::NONE : EFFECT_DETAIL_SELECTION::ELEMENT;
	else
	{
		m_eDetailSelection = !m_ActiveDocument->Elements.empty() ?
			EFFECT_DETAIL_SELECTION::ELEMENT :
			(!m_ActiveDocument->ModelCues.empty() ?
				EFFECT_DETAIL_SELECTION::MODEL_CUE :
				EFFECT_DETAIL_SELECTION::NONE);
	}
    m_strSelectedElementId =
        EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
            m_ActiveDocument->Elements.front().strElementId : std::string{};
	m_strSelectedElementGroupId =
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
			m_ActiveDocument->Elements.front().strGroupId : std::string{};
	m_strSelectedModelCueId =
		EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection ?
			m_ActiveDocument->ModelCues.front().strCueId : std::string{};
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewIsolationModelCueId.clear();
	m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
	m_strSelectedComponentId =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == eSource ?
			strSelectionId : std::string{};
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    if (const EFFECT_ELEMENT_DESC* pSelected = Find_SelectedElement())
    {
        m_eSelectedEffectType = pSelected->eKind;
        m_strSelectedResourceSlotId = Default_SlotId(pSelected->eKind);
        m_eResourceLibraryFileKind = Slot_FileKind(
            *pSelected, m_strSelectedResourceSlotId);
    }
    m_strSelectedDataFileAssetId = strSelectionId;
	std::string SuggestedAssetId = m_ActiveDocument->strEffectAssetId;
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM == eSource &&
		SuggestedAssetId.size() + std::string_view(".authored-copy").size() <
			m_NewAssetId.size())
	{
		SuggestedAssetId += ".authored-copy";
	}
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), SuggestedAssetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(),
        m_ActiveDocument->strDisplayName);
    m_bDocumentDirty = false;
	m_strSaveHotReloadStatus.clear();
    Refresh_RuntimeEquivalence();
    m_PendingDocumentLoad.reset();
    m_fPreviewTimeSeconds = 0.f;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Recalculate_PreviewDuration();
    Synchronize_LoadedSkillPreview();
    m_bPreviewVisibleRequested = false;
    m_bPreviewPlaying = false;
    if (bDrawable)
        Set_SynchronizedAnimationPaused(true);

    m_strPreviewStatus = bDrawable ?
        "Document loaded; GPU resources are deferred until an explicit preview scope is played." :
        "Preview is unavailable until required resources bind: " + PreviewStatus;
    if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource)
    {
        m_strDocumentStatus = "Loaded saved Effect '" +
            m_ActiveDocument->strEffectAssetId +
            "' with its existing name; " +
			(EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
				std::string("selected its first Element for Effect Details.") :
				(EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection ?
					std::string("selected its first Model / Summon cue for Effect Details.") :
					std::string("it has no Element or Model Cue to select.")));
    }
    else
    {
        m_strDocumentStatus = bDrawable ?
            "Loaded advanced document for inspection without GPU staging; choose Complete, Group, or Solo Play: " +
				(Path.empty() ? strSelectionId : Path.string()) :
            "Loaded advanced draft; preview is hidden until required resources bind: " +
                PreviewStatus;
    }
    return true;
}

bool_t Client::CEffect_Tool::Execute_PendingDocumentLoad(
    const bool_t bSaveFirst)
{
    if (!m_PendingDocumentLoad.has_value())
    {
        m_strDocumentStatus = "No pending Effect document load exists.";
        return false;
    }
    if (bSaveFirst)
    {
        if (Has_UnappliedDetailDraft())
        {
            m_strDocumentStatus =
                "Apply or Revert the open Detail draft before Save & Load.";
            return false;
        }
        if (!Try_SaveDocument())
            return false;
    }

	const PENDING_DOCUMENT_LOAD Pending = *m_PendingDocumentLoad;
	if (!Try_LoadDocumentPathStaged(
		Pending.Path,
		Pending.eSource,
		Pending.strSelectionId,
		true))
	{
		return false;
	}
	const auto CompleteValtanPreviewPartial =
		[this, &Pending](std::string Reason)
		{
			/* The document load already committed and cleared the pending target.
			   Close the modal instead of leaving a stale Save/Discard operation. */
			m_PendingDocumentLoad.reset();
			m_bPendingDocumentLoadModalRequested = false;
			m_strDocumentStatus =
				"Loaded saved Effect '" + Pending.strSelectionId +
				"', but its Valtan animation/effect preview could not be staged. "
				"The document remains loaded: " + std::move(Reason);
			return true;
		};
	if (Pending.ValtanClip.has_value() && Pending.ValtanCue.has_value())
	{
		if (!Play_ValtanProductCue(
				*Pending.ValtanClip, *Pending.ValtanCue))
		{
			return CompleteValtanPreviewPartial(
				m_strPreviewAnimationStatus.empty() ?
					std::string("animation target unavailable") :
					m_strPreviewAnimationStatus);
		}
		if (Pending.bPlayCompleteAfterLoad)
		{
			if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
			{
				return CompleteValtanPreviewPartial(
					m_strPreviewStatus.empty() ?
						std::string("Effect world preview unavailable") :
						m_strPreviewStatus);
			}
			Start_WorldPreviewFromBeginning();
		}
		return true;
	}
	if (Pending.ValtanReferenceClips.has_value())
	{
		const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips =
			*Pending.ValtanReferenceClips;
		const bool_t bTargetReady = Clips.empty() ?
			(nullptr != m_pCharacterPreviewPanel &&
			 (CAnimationTargetService::Resolve_AssetName() ==
				VALTAN_ANIMATION_ASSET_NAME ||
			  m_pCharacterPreviewPanel->Select_TargetAsset(
				VALTAN_ANIMATION_ASSET_NAME))) :
			Play_ValtanStageSequence(Clips);
		if (!bTargetReady)
		{
			return CompleteValtanPreviewPartial(
				m_strPreviewAnimationStatus.empty() ?
					std::string("Valtan model or ordered animation unavailable") :
					m_strPreviewAnimationStatus);
		}
		if (Pending.bPlayCompleteAfterLoad)
		{
			if (!Try_PlayActiveUnifiedEffect())
			{
				return CompleteValtanPreviewPartial(
					m_strPreviewStatus.empty() ?
						std::string("Effect world preview unavailable") :
						m_strPreviewStatus);
			}
		}
		return true;
	}
	if (Pending.bPlayCompleteAfterLoad)
	{
		if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
			return false;
		Start_WorldPreviewFromBeginning();
		return true;
	}
	if (!Pending.strElementSelectionId.empty())
	{
		return Try_SelectElement(Pending.strSelectionId,
			Pending.strElementSelectionId);
	}
	if (!Pending.strModelCueSelectionId.empty())
	{
		return Try_SelectModelCue(Pending.strSelectionId,
			Pending.strModelCueSelectionId);
	}
	return true;
}

bool_t Client::CEffect_Tool::Refresh_AllEffects(
    const bool_t bReloadSkillCatalog)
{
    m_bAllEffectsRefreshAttempted = true;
	if (bReloadSkillCatalog)
		Reset_ArtistFPreparationFailureLatch();
    std::string CatalogStatus;
    if ((bReloadSkillCatalog || CPlayerSkillCatalog::Get_Skills().empty()) &&
        !CPlayerSkillCatalog::Load(CatalogStatus))
    {
        m_strElementStatus =
            "All Effects refresh preserved the previous tree: " +
            CatalogStatus;
        return false;
    }

    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    vector<EFFECT_SKILL_TREE_ENTRY> Staged;
    Staged.reserve(Skills.size());
    std::map<LostArk::Shared::SKILL_ID, size_t> EntryIndices;
    for (const PLAYER_SKILL_DEFINITION& Skill : Skills)
    {
        EntryIndices.emplace(Skill.iSkillId, Staged.size());
        EFFECT_SKILL_TREE_ENTRY Entry;
        Entry.Skill = Skill;
        Staged.push_back(std::move(Entry));
    }
	const bool_t bHadPreviousAllEffectsTree = !m_AllEffects.empty();
	if (!bHadPreviousAllEffectsTree)
	{
		/* PlayerSkills owns the navigation tree. Product presentation is optional
		   child data and must never hide Q/W/E/R/T/A/S/D/F on a cold entry. */
		m_AllEffects = Staged;
	}
    const auto FailRefresh = [this, bHadPreviousAllEffectsTree](
		const std::string& strReason)
    {
        m_strElementStatus =
			(bHadPreviousAllEffectsTree ?
				"All Effects Product enrichment preserved the previous tree: " :
				"All Effects Product enrichment is unavailable; base PlayerSkills rows remain visible: ") +
			strReason;
        return false;
    };
    constexpr LostArk::Shared::CHARACTER_CLASS_ID Classes[] = {
        LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
        LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
        LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
        LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
        LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
        LostArk::Shared::CHARACTER_CLASS_ID::WARLORD };
    size_t iProductCueCount = 0u;
    size_t iProductSkillCount = 0u;
    size_t iSourceReferenceCount = 0u;
    std::set<std::string> MissingAuthoredTargets;
    for (const LostArk::Shared::CHARACTER_CLASS_ID eClass : Classes)
    {
        const char* pAnimationAsset = Animation_AssetName(eClass);
        if (nullptr == pAnimationAsset)
            return FailRefresh("a playable class has no animation asset ID.");

        std::string BindingText;
        std::string PresentationStatus;
        const std::filesystem::path BindingPath =
            CAnimationSkillBindingDocument::Resolve_Path(pAnimationAsset);
        if (!Read_TextFile(BindingPath, BindingText, PresentationStatus))
            return FailRefresh(PresentationStatus);

        ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
        if (!CAnimationSkillBindingDocument::Parse_Text(
            BindingText, Bindings, PresentationStatus))
        {
            return FailRefresh(PresentationStatus);
        }
        vector<string> BoundClipNames;
		struct BOUND_CLIP_OWNER final
		{
			LostArk::Shared::SKILL_ID iSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			ANIMATION_SKILL_CLIP Clip;
			size_t iBoundClipOrdinal = 0u;
			size_t iStageIndex = 0u;
			size_t iStageClipIndex = 0u;
		};
        std::map<string, std::vector<BOUND_CLIP_OWNER>> ClipOwners;
        for (const ANIMATION_SKILL_BINDING& Binding : Bindings.Bindings)
        {
            const auto EntryIndex = EntryIndices.find(Binding.iSkillId);
            if (EntryIndex == EntryIndices.end() ||
                Staged[EntryIndex->second].Skill.eCharacterClass != eClass)
            {
                return FailRefresh(
                    "a skill binding is not owned by its PlayerSkills class.");
            }
            const vector<ANIMATION_SKILL_CLIP> BindingClips =
                Flatten_BindingClips(Binding);
            if (BindingClips.empty())
                return FailRefresh("a skill binding has no animation clips.");
			size_t iBoundClipOrdinal = 0u;
			for (size_t iStage = 0u; iStage < Binding.Stages.size(); ++iStage)
            {
				const ANIMATION_SKILL_STAGE& Stage = Binding.Stages[iStage];
				for (size_t iStageClip = 0u;
					iStageClip < Stage.Clips.size();
					++iStageClip, ++iBoundClipOrdinal)
				{
					const string& strClipName =
						Stage.Clips[iStageClip].strClipName;
					BoundClipNames.push_back(strClipName);
					auto& Owners = ClipOwners[strClipName];
					if (std::any_of(Owners.begin(), Owners.end(),
						[&Binding](const BOUND_CLIP_OWNER& Owner)
						{ return Owner.iSkillId != Binding.iSkillId; }))
					{
						return FailRefresh(
							"one animation clip is claimed by multiple skills: " +
							strClipName);
					}
					Owners.push_back(BOUND_CLIP_OWNER{
						Binding.iSkillId, Stage.Clips[iStageClip], iBoundClipOrdinal,
						iStage, iStageClip });
				}
            }
        }
        std::sort(BoundClipNames.begin(), BoundClipNames.end());
        BoundClipNames.erase(std::unique(
            BoundClipNames.begin(), BoundClipNames.end()),
            BoundClipNames.end());
        if (!CAnimationSkillBindingDocument::Validate(
            Bindings, pAnimationAsset, eClass, Skills,
            BoundClipNames, PresentationStatus))
        {
            return FailRefresh(PresentationStatus);
        }

        const std::filesystem::path EventPath = CProjectDataRoot::Resolve(
            std::filesystem::path(L"Animation") / L"Authored" /
            std::filesystem::path(pAnimationAsset) /
            (std::filesystem::path(pAnimationAsset).wstring() +
                L".animevents"));
        std::string EventText;
        if (!Read_TextFile(EventPath, EventText, PresentationStatus))
            return FailRefresh(PresentationStatus);

        ANIMATION_EFFECT_CUE_DOCUMENT CueDocument;
        if (!CAnimationEffectCueDocument::Load_FromText(
            pAnimationAsset, EventText, BoundClipNames,
			CueDocument, PresentationStatus, true))
        {
            return FailRefresh(PresentationStatus);
        }
        for (const ANIMATION_EFFECT_CUE& Cue : CueDocument.Cues)
        {
            const auto Owner = ClipOwners.find(Cue.strClipName);
            if (Owner == ClipOwners.end())
				continue;
			for (const BOUND_CLIP_OWNER& ClipOwner : Owner->second)
			{
				if (!CAnimationEffectCueDocument::Is_CueStartInClipWindow(
						ClipOwner.Clip, Cue.iStartMs))
				{
					continue;
				}
				const auto EntryIndex = EntryIndices.find(ClipOwner.iSkillId);
				if (EntryIndex == EntryIndices.end())
					return FailRefresh(
						"an admitted Effect cue has no skill row.");
				EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE ProductCue;
				ProductCue.Cue = Cue;
				ProductCue.Clip = ClipOwner.Clip;
				ProductCue.iBoundClipOrdinal =
					ClipOwner.iBoundClipOrdinal;
				ProductCue.iStageIndex = ClipOwner.iStageIndex;
				ProductCue.iStageClipIndex = ClipOwner.iStageClipIndex;
				Staged[EntryIndex->second].ProductCues.push_back(
					std::move(ProductCue));
			}
        }

        for (const PLAYER_SKILL_DEFINITION& Skill : Skills)
        {
            if (Skill.eCharacterClass != eClass)
                continue;
            const auto EntryIndex = EntryIndices.find(Skill.iSkillId);
            if (EntryIndex == EntryIndices.end())
            {
                return FailRefresh(
                    "a PlayerSkills row has no complete animation binding.");
            }
            EFFECT_SKILL_TREE_ENTRY& Entry = Staged[EntryIndex->second];
            std::sort(Entry.ProductCues.begin(), Entry.ProductCues.end(),
                [](const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Left,
                    const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Right)
                {
                    return std::tie(
                        Left.iBoundClipOrdinal,
                        Left.Cue.iStartMs,
                        Left.Cue.strEffectAssetId,
                        Left.Cue.strAnchorSlotId) <
                        std::tie(
                            Right.iBoundClipOrdinal,
                            Right.Cue.iStartMs,
                            Right.Cue.strEffectAssetId,
                            Right.Cue.strAnchorSlotId);
                });
            if (!Entry.ProductCues.empty())
                ++iProductSkillCount;
            iProductCueCount += Entry.ProductCues.size();
            for (const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue :
                Entry.ProductCues)
            {
                const std::filesystem::path AuthoredPath =
                    CProjectDataRoot::Resolve(
                        std::filesystem::path(L"Effects") / L"Authored" /
                        (std::filesystem::path(
                            ProductCue.Cue.strEffectAssetId).wstring() +
                            L".effect.json"));
                if (AuthoredPath.empty() ||
                    !std::filesystem::is_regular_file(AuthoredPath))
                {
                    MissingAuthoredTargets.insert(
                        ProductCue.Cue.strEffectAssetId);
                }
            }
        }

        std::istringstream EventRows(EventText);
        std::string EventLine;
        std::getline(EventRows, EventLine);
        while (std::getline(EventRows, EventLine))
        {
            std::string strClipName;
            bool_t bImported = false;
            bool_t bEmptyPayload = false;
            if (!Try_ParseEffectDiagnosticRow(
                EventLine, strClipName, bImported, bEmptyPayload))
            {
                continue;
            }
            const auto Owner = ClipOwners.find(strClipName);
            if (Owner == ClipOwners.end())
                continue;
            const auto EntryIndex = EntryIndices.find(
				Owner->second.front().iSkillId);
            if (EntryIndex == EntryIndices.end())
                continue;
            EFFECT_SKILL_TREE_ENTRY& Entry = Staged[EntryIndex->second];
            ++Entry.iSourceReferenceCount;
            ++iSourceReferenceCount;
            if (bImported)
                ++Entry.iImportedReferenceCount;
            if (bEmptyPayload)
                ++Entry.iEmptySourceReferenceCount;
        }
    }
    std::sort(Staged.begin(), Staged.end(),
        [](const EFFECT_SKILL_TREE_ENTRY& Left,
            const EFFECT_SKILL_TREE_ENTRY& Right)
        {
            if (Left.Skill.eCharacterClass != Right.Skill.eCharacterClass)
                return Left.Skill.eCharacterClass < Right.Skill.eCharacterClass;
            if (Left.Skill.strInputSlot != Right.Skill.strInputSlot)
                return Left.Skill.strInputSlot < Right.Skill.strInputSlot;
            return Left.Skill.iSkillId < Right.Skill.iSkillId;
        });
    m_AllEffects = std::move(Staged);
    m_strElementStatus =
        "All Effects indexed Product presentation from PlayerSkills + "
        "skillbindings + animevents: " +
        std::to_string(m_AllEffects.size()) + " skills, " +
        std::to_string(iProductSkillCount) + " Product skills, " +
        std::to_string(iProductCueCount) + " executable cues, " +
        std::to_string(iSourceReferenceCount) +
        " Source/Imported diagnostic rows";
    if (!MissingAuthoredTargets.empty())
        m_strElementStatus += ", " +
            std::to_string(MissingAuthoredTargets.size()) +
            " Product targets have no Authored document";
    m_strElementStatus += ".";
    return true;
}

bool_t Client::CEffect_Tool::Refresh_DataFiles()
{
    m_bDataFilesRefreshAttempted = true;
    vector<EFFECT_DATA_FILE_ENTRY> Staged;
    std::set<std::string> AssetIds;
    std::set<std::string> StagedDomainIds;
    size_t iRejectedDocumentCount = 0u;
    std::string strFirstRejectedDocument;
    const auto RecordRejectedDocument =
        [&iRejectedDocumentCount, &strFirstRejectedDocument](
            const std::string& strReason)
    {
        ++iRejectedDocumentCount;
        if (strFirstRejectedDocument.empty())
            strFirstRejectedDocument = strReason;
    };
    for (const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain : m_ResourceDomains)
        StagedDomainIds.insert(Domain.strDomainId);

    std::map<std::string, std::string> SkillDomains;
    std::string SkillCatalogStatus;
    if (Ensure_PlayerSkillCatalog(SkillCatalogStatus))
    {
        for (const PLAYER_SKILL_DEFINITION& Skill :
            CPlayerSkillCatalog::Get_Skills())
        {
            const char* pDomainId = Resource_DomainId(Skill.eCharacterClass);
            if (!Skill.strEffectId.empty() && nullptr != pDomainId)
                SkillDomains.emplace(Skill.strEffectId, pDomainId);
        }
    }
    const auto ResolveDocumentDomain = [&SkillDomains](
        const std::string& strAssetId,
        const std::filesystem::path& RelativePath,
        const EFFECT_DOCUMENT_SOURCE eSource)
    {
        if (EFFECT_DOCUMENT_SOURCE::IMPORTED == eSource)
        {
            const std::string PathDomain = First_PathComponent(RelativePath);
            if (!PathDomain.empty())
                return PathDomain;
        }
        const auto SkillDomain = SkillDomains.find(strAssetId);
        if (SkillDomain != SkillDomains.end())
            return SkillDomain->second;
        return EffectAsset_DomainId(strAssetId);
    };
    const auto Scan = [this, &Staged, &AssetIds, &StagedDomainIds,
        &ResolveDocumentDomain, &RecordRejectedDocument](
        const std::filesystem::path& RelativeRoot,
        const EFFECT_DOCUMENT_SOURCE eSource) -> bool_t
    {
        const std::filesystem::path Root =
            CProjectDataRoot::Resolve(RelativeRoot);
        std::error_code Error;
        if (Root.empty() || !std::filesystem::exists(Root, Error))
        {
            RecordRejectedDocument(
                "missing Data Files root: " + Root.string());
            return true;
        }
        if (Error || !std::filesystem::is_directory(Root, Error))
        {
            m_strDocumentStatus = "Data Files root is invalid: " +
                Root.string();
            return false;
        }
        for (std::filesystem::recursive_directory_iterator Iterator(
            Root, std::filesystem::directory_options::skip_permission_denied,
            Error), End; Iterator != End; Iterator.increment(Error))
        {
            if (Error)
            {
                RecordRejectedDocument(
                    "Data Files directory iteration failed: " +
                    Root.string() + ": " + Error.message());
                Error.clear();
                break;
            }
            if (!Iterator->is_regular_file())
                continue;
            const std::string Name = Iterator->path().filename().string();
            if (!Name.ends_with(".effect.json"))
                continue;
            std::string EffectAssetId;
            if (!Try_DeriveEffectAssetIdFromFilename(
                Iterator->path(), eSource, EffectAssetId))
            {
                RecordRejectedDocument(
                    "noncanonical Effect filename: " +
                    Iterator->path().string());
                continue;
            }
            if (!AssetIds.insert(EffectAssetId).second)
            {
                RecordRejectedDocument(
                    "duplicate Effect ID: " + EffectAssetId);
                continue;
            }
            const std::filesystem::path RelativePath =
                Iterator->path().lexically_relative(Root);
            const std::string DomainId = ResolveDocumentDomain(
                EffectAssetId, RelativePath, eSource);
            StagedDomainIds.insert(DomainId);
            Staged.push_back({
                EffectAssetId, DomainId,
                Iterator->path(), eSource });
        }
        return true;
    };
    if (!Scan(L"Effects/Authored", EFFECT_DOCUMENT_SOURCE::AUTHORED) ||
        !Scan(L"Effects/Imported", EFFECT_DOCUMENT_SOURCE::IMPORTED))
        return false;

	size_t iRuntimeAssemblyCount = 0u;
	size_t iRuntimeComponentCount = 0u;
	for (const std::string& EffectId : CEffectCatalog::Get_EffectAssetIds())
	{
		if (nullptr == CEffectCatalog::Find_Assembly(EffectId))
			continue;
		const std::string SelectionId = EffectId + "::assembly";
		if (!AssetIds.insert(SelectionId).second)
		{
			m_strDocumentStatus =
				"Data Files refresh rejected duplicate Assembly ID: " + EffectId;
			return false;
		}
		const std::string DomainId = EffectAsset_DomainId(EffectId);
		StagedDomainIds.insert(DomainId);
		Staged.push_back({ SelectionId, DomainId, {},
			EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY });
		++iRuntimeAssemblyCount;
	}
	for (const std::string& ComponentId :
		CEffectCatalog::Get_ComponentAssetIds())
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(ComponentId);
		if (nullptr == Component)
			continue;
		if (!AssetIds.insert(ComponentId).second)
		{
			m_strDocumentStatus =
				"Data Files refresh rejected duplicate WFX Component ID: " +
				ComponentId;
			return false;
		}
		const std::string DomainId = EffectAsset_DomainId(
			Component->strSourceEffectAssetId);
		StagedDomainIds.insert(DomainId);
		Staged.push_back({ ComponentId, DomainId, {},
			EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT });
		++iRuntimeComponentCount;
	}

    size_t iImportedReferenceCount = 0u;
    const std::filesystem::path ImportedRoot =
        CProjectDataRoot::Resolve(L"Effects/Imported");
    std::error_code ReferenceError;
    if (!ImportedRoot.empty() &&
        std::filesystem::is_directory(ImportedRoot, ReferenceError))
    {
        for (std::filesystem::recursive_directory_iterator Iterator(
            ImportedRoot,
            std::filesystem::directory_options::skip_permission_denied,
            ReferenceError), End;
            Iterator != End; Iterator.increment(ReferenceError))
        {
            if (ReferenceError)
            {
                m_strDocumentStatus =
                    "Imported draft scan failed; previous list preserved.";
                return false;
            }
            if (!Iterator->is_regular_file())
                continue;
            const std::string Name = Iterator->path().filename().string();
            if (!Name.ends_with(".imported-effect-draft.json") &&
                !Name.ends_with(".unbound-effect-draft-index.json"))
                continue;
            const std::filesystem::path Relative =
                Iterator->path().lexically_relative(ImportedRoot);
            if (Relative.empty())
                continue;
            std::string DomainId = First_PathComponent(Relative);
            if (DomainId.empty())
                DomainId = "Uncategorized";
            StagedDomainIds.insert(DomainId);
            Staged.push_back({
                Relative.generic_string(), DomainId, Iterator->path(),
                EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE });
            ++iImportedReferenceCount;
        }
    }
    std::sort(Staged.begin(), Staged.end(),
        [](const EFFECT_DATA_FILE_ENTRY& Left,
            const EFFECT_DATA_FILE_ENTRY& Right)
        {
            if (Left.strDomainId != Right.strDomainId)
                return Left.strDomainId < Right.strDomainId;
            if (Left.eSource != Right.eSource)
                return Left.eSource < Right.eSource;
            return Left.strAssetId < Right.strAssetId;
        });
	const bool_t bDirectAuthoredEditableIndexReady =
		Refresh_DirectAuthoredEditableIndex(Staged);
    m_DataFiles = std::move(Staged);
    m_DataFileDomains.assign(
        StagedDomainIds.begin(), StagedDomainIds.end());
    if (m_DataFileDomains.end() == std::find(
        m_DataFileDomains.begin(), m_DataFileDomains.end(),
        m_strSelectedAuthoringDomainId) && !m_DataFileDomains.empty())
    {
        Select_AuthoringDomain(m_DataFileDomains.front());
    }
    m_strDocumentStatus = "Data Files refreshed: " +
		std::to_string(m_DataFiles.size() - iImportedReferenceCount -
			iRuntimeAssemblyCount - iRuntimeComponentCount) +
		" authored/imported documents, " +
		std::to_string(iRuntimeAssemblyCount) + " runtime Assemblies, " +
		std::to_string(iRuntimeComponentCount) + " WFX Components, " +
        std::to_string(iImportedReferenceCount) +
        " reference-only extraction drafts.";
    if (0u != iRejectedDocumentCount)
    {
        m_strDocumentStatus += " Isolated " +
            std::to_string(iRejectedDocumentCount) +
            " invalid/duplicate entries; first: " +
            strFirstRejectedDocument;
    }
	m_strDocumentStatus += " " + m_strDirectAuthoredEditableStatus;
	if (!bDirectAuthoredEditableIndexReady)
	{
		m_strDocumentStatus +=
			" The previous direct-authored and saved unified index was preserved; Open remains unavailable only when no valid index was admitted yet.";
	}
	m_strDocumentStatus += " " + m_strUnifiedCandidateStatus;
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectProductCue(
    const EFFECT_SKILL_TREE_ENTRY& Entry,
    const size_t iCueIndex)
{
    if (iCueIndex >= Entry.ProductCues.size())
    {
        m_strElementStatus = "The selected BA stage has no playable Effect.";
        return false;
    }
    const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
        Entry.ProductCues[iCueIndex];
    if (ProductCue.Cue.strEffectAssetId.empty() ||
        !CEffectCatalog::Contains(ProductCue.Cue.strEffectAssetId))
    {
        m_strElementStatus =
            "Play Full Effect rejected an Effect that is no longer available.";
        return false;
    }
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProductVisualProjection = CEffectCatalog::Find_VisualProjection(
			ProductCue.Cue.strEffectAssetId);
	const shared_ptr<const EFFECT_DOCUMENT_DESC> pRuntimeDocument =
		nullptr == pProductVisualProjection ?
			CEffectCatalog::Find(ProductCue.Cue.strEffectAssetId) : nullptr;
	const EFFECT_DOCUMENT_DESC* pSourceDocument =
		nullptr != pProductVisualProjection ?
			&pProductVisualProjection->Get_Document() : pRuntimeDocument.get();
	if (nullptr == pSourceDocument)
	{
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
			CEffectCatalog::Find_VisualProgram(
				ProductCue.Cue.strEffectAssetId);
		if (ProductCue.Cue.strEffectAssetId ==
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
			nullptr != Program && Program->eProjectionKind ==
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
		{
			const optional<EFFECT_PRODUCT_PREVIEW> PreviousProductPreview =
				m_ProductPreview;
			const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanProductPreview =
				m_ValtanProductPreview;
			const optional<EFFECT_DOCUMENT_DESC> PreviousSourcePreviewDocument =
				m_SourcePreviewDocument;
			const std::string strPreviousIsolationElement =
				m_strPreviewIsolationElementId;
			const std::string strPreviousIsolationGroup =
				m_strPreviewIsolationGroupId;
			const f32_t fPreviousPreviewTimeSeconds =
				m_fPreviewTimeSeconds;
			const f32_t fPreviousPreviewDurationSeconds =
				m_fPreviewDurationSeconds;
			const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
			const bool_t bPreviousPreviewVisibleRequested =
				m_bPreviewVisibleRequested;
			const float4x4_t PreviousProductCueSnapshotRoot =
				m_ProductCueSnapshotRoot;
			const bool_t bPreviousProductCueSnapshotCaptured =
				m_bProductCueSnapshotCaptured;
			const f32_t fPreviousProductCueActionFacingYawDegrees =
				m_fProductCueActionFacingYawDegrees;
			const bool_t bPreviousProductCueActionFacingCaptured =
				m_bProductCueActionFacingCaptured;
			Clear_ProductCuePreview();
			if (!Try_StartArtist31470FullPreview())
			{
				const std::string ArtistStartError = m_strPreviewStatus;
				m_ProductPreview = PreviousProductPreview;
				m_ValtanProductPreview = PreviousValtanProductPreview;
				m_SourcePreviewDocument = PreviousSourcePreviewDocument;
				m_strPreviewIsolationElementId =
					strPreviousIsolationElement;
				m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
				Reset_ProductCueSnapshot();
				std::string ValtanRestoreError;
				bool_t bRestored = true;
				if (m_ValtanProductPreview.has_value())
					bRestored = Restore_ValtanProductPreviewPlayback(
						m_ValtanProductPreview,
						fPreviousPreviewTimeSeconds,
						fPreviousPreviewDurationSeconds,
						bPreviousPreviewPlaying,
						bPreviousPreviewVisibleRequested,
						PreviousProductCueSnapshotRoot,
						bPreviousProductCueSnapshotCaptured,
						ValtanRestoreError);
				else
				{
					m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
					m_bProductCueSnapshotCaptured =
						bPreviousProductCueSnapshotCaptured;
					m_fProductCueActionFacingYawDegrees =
						fPreviousProductCueActionFacingYawDegrees;
					m_bProductCueActionFacingCaptured =
						bPreviousProductCueActionFacingCaptured;
					Synchronize_LoadedSkillPreview();
				}
				m_strElementStatus = bRestored ?
					"Artist F full preview failed; the previous preview was restored: " +
						ArtistStartError :
					"Artist F full preview failed and the previous exact Valtan preview rollback failed: " +
						ArtistStartError + " Rollback failed: " +
						ValtanRestoreError;
				return false;
			}
			m_eAllEffectsClass = Entry.Skill.eCharacterClass;
			m_strElementStatus = "Playing full Effect | " +
				Entry.Skill.strDisplayName + " | " +
				ProductCue.Cue.strClipName;
			return true;
		}
		m_strElementStatus =
			"This Effect needs its Track A adapter prepared before playback.";
		return false;
	}
	const optional<EFFECT_PRODUCT_PREVIEW> PreviousProductPreview =
		m_ProductPreview;
	const optional<VALTAN_PRODUCT_PREVIEW> PreviousValtanProductPreview =
		m_ValtanProductPreview;
	const optional<EFFECT_DOCUMENT_DESC> PreviousSourcePreviewDocument =
		m_SourcePreviewDocument;
	const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter = m_ePreviewFilter;
	const std::string strPreviousIsolationElement =
		m_strPreviewIsolationElementId;
	const std::string strPreviousIsolationGroup =
		m_strPreviewIsolationGroupId;
	const f32_t fPreviousPreviewTimeSeconds = m_fPreviewTimeSeconds;
	const f32_t fPreviousPreviewDurationSeconds = m_fPreviewDurationSeconds;
	const bool_t bPreviousScreenPostEnabled = m_bPreviewScreenPostEnabled;
	const bool_t bPreviousPreviewPlaying = m_bPreviewPlaying;
	const bool_t bPreviousPreviewVisibleRequested =
		m_bPreviewVisibleRequested;
	const float4x4_t PreviousProductCueSnapshotRoot =
		m_ProductCueSnapshotRoot;
	const bool_t bPreviousProductCueSnapshotCaptured =
		m_bProductCueSnapshotCaptured;
	const f32_t fPreviousProductCueActionFacingYawDegrees =
		m_fProductCueActionFacingYawDegrees;
	const bool_t bPreviousProductCueActionFacingCaptured =
		m_bProductCueActionFacingCaptured;
    EFFECT_PRODUCT_PREVIEW Preview;
    Preview.eCharacterClass = Entry.Skill.eCharacterClass;
    Preview.iSkillId = Entry.Skill.iSkillId;
    Preview.ProductCue = ProductCue;
	m_ValtanProductPreview.reset();
    m_ProductPreview = std::move(Preview);
	m_SourcePreviewDocument = *pSourceDocument;
    Reset_ProductCueSnapshot();
    m_eAllEffectsClass = Entry.Skill.eCharacterClass;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_bPreviewScreenPostEnabled = true;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
    m_fPreviewTimeSeconds = 0.f;
	Recalculate_PreviewDuration(*m_SourcePreviewDocument);
    Synchronize_LoadedSkillPreview();
	if (!Stage_WorldPreview(*m_SourcePreviewDocument, true))
	{
		const std::string StageError = m_strPreviewStatus;
		m_ProductPreview = PreviousProductPreview;
		m_ValtanProductPreview = PreviousValtanProductPreview;
		m_SourcePreviewDocument = PreviousSourcePreviewDocument;
		m_ePreviewFilter = ePreviousPreviewFilter;
		m_strPreviewIsolationElementId = strPreviousIsolationElement;
		m_strPreviewIsolationGroupId = strPreviousIsolationGroup;
		m_fPreviewTimeSeconds = fPreviousPreviewTimeSeconds;
		m_fPreviewDurationSeconds = fPreviousPreviewDurationSeconds;
		m_bPreviewScreenPostEnabled = bPreviousScreenPostEnabled;
		Reset_ProductCueSnapshot();
		std::string ValtanRestoreError;
		bool_t bRestored = true;
		if (m_ValtanProductPreview.has_value())
			bRestored = Restore_ValtanProductPreviewPlayback(
				m_ValtanProductPreview,
				fPreviousPreviewTimeSeconds,
				fPreviousPreviewDurationSeconds,
				bPreviousPreviewPlaying,
				bPreviousPreviewVisibleRequested,
				PreviousProductCueSnapshotRoot,
				bPreviousProductCueSnapshotCaptured,
				ValtanRestoreError);
		else
		{
			m_ProductCueSnapshotRoot = PreviousProductCueSnapshotRoot;
			m_bProductCueSnapshotCaptured =
				bPreviousProductCueSnapshotCaptured;
			m_fProductCueActionFacingYawDegrees =
				fPreviousProductCueActionFacingYawDegrees;
			m_bProductCueActionFacingCaptured =
				bPreviousProductCueActionFacingCaptured;
			Synchronize_LoadedSkillPreview();
		}
        m_strElementStatus =
			"Play Full Effect could not stage this source preview: " +
			StageError + (bRestored ? std::string{} :
				" Previous exact Valtan preview rollback failed: " +
				ValtanRestoreError);
        return false;
    }
    Start_WorldPreviewFromBeginning();
	m_strElementStatus = "Playing full Effect | " +
		Entry.Skill.strDisplayName + " | " + ProductCue.Cue.strClipName;
	if (nullptr == pProductVisualProjection)
	{
		m_strElementStatus += Describe_ProductPlaybackAuthoredDivergence(
			ProductCue.Cue.strEffectAssetId);
	}
    return true;
}

bool_t Client::CEffect_Tool::Try_PlayVisualProgramFamily(
	const EFFECT_SKILL_TREE_ENTRY& Entry,
	const size_t iCueIndex,
	const EFFECT_VISUAL_PROGRAM_FAMILY eFamily)
{
	if (iCueIndex >= Entry.ProductCues.size())
		return false;
	const std::string& strEffectAssetId =
		Entry.ProductCues[iCueIndex].Cue.strEffectAssetId;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == Program)
	{
		m_strPreviewStatus = "This Effect has no Track A Family program.";
		return false;
	}
	if (!Try_SelectProductCue(Entry, iCueIndex))
	{
		return false;
	}
	return Try_SetVisualPreviewFamilyIsolation(*Program, eFamily);
}

bool_t Client::CEffect_Tool::Try_PlayVisualProgramElement(
	const EFFECT_SKILL_TREE_ENTRY& Entry,
	const size_t iCueIndex,
	const std::string& strTargetElementId)
{
	if (iCueIndex >= Entry.ProductCues.size())
		return false;
	const std::string& strEffectAssetId =
		Entry.ProductCues[iCueIndex].Cue.strEffectAssetId;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> Program =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == Program)
		return false;
	if (!Try_SelectProductCue(Entry, iCueIndex))
	{
		return false;
	}
	return Try_SetVisualPreviewOccurrenceIsolation(strTargetElementId);
}

bool_t Client::CEffect_Tool::Try_SelectParticleSystem(
    const std::string& strEffectAssetId)
{
    const bool_t bChangesSelection =
        !m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
        EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM != m_eDetailSelection;
    if (bChangesSelection &&
		(Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty))
    {
        m_strElementStatus =
            "Save, Apply, or Revert the open Detail/tuning work before selecting the Particle System.";
        return false;
    }
    if (!m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId)
    {
        if (!Try_LoadDocument(strEffectAssetId))
            return false;
    }
    const bool_t bHasParticles = std::any_of(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [](const EFFECT_ELEMENT_DESC& Element)
        {
            return EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
        });
    if (!bHasParticles)
    {
        m_strElementStatus =
            "The selected Effect has no Particle layers to group.";
        return false;
    }

    Reset_DetailDraft();
    Reset_ParticleSystemDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM;
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strSelectedResourceAssetId.clear();
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Recalculate_PreviewDuration();
    if (!Stage_WorldPreview())
        return false;
    m_strElementStatus =
        "Selected the complete Particle System; child layers remain available below.";
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectElement(
    const std::string& strEffectAssetId,
    const std::string& strElementId)
{
    const bool_t bChangesSelection =
        !m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
        EFFECT_DETAIL_SELECTION::ELEMENT != m_eDetailSelection ||
        (!strElementId.empty() &&
            m_strSelectedElementId != strElementId);
    if (bChangesSelection &&
		(Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty))
    {
        m_strElementStatus =
            "Save, Apply, or Revert the open Detail/tuning work before selecting another Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId)
    {
        if (!Try_LoadDocument(strEffectAssetId))
            return false;
    }
    if (strElementId.empty())
        return true;
    const auto Iterator = std::find_if(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [&strElementId](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == strElementId;
        });
    if (Iterator == m_ActiveDocument->Elements.end())
    {
        m_strElementStatus = "Selected Element ID is no longer present.";
        return false;
    }
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = strElementId;
    m_strSelectedElementGroupId = Iterator->strGroupId;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_eSelectedEffectType = Iterator->eKind;
    m_strSelectedResourceSlotId = Default_SlotId(Iterator->eKind);
    m_eResourceLibraryFileKind = Slot_FileKind(
        *Iterator, m_strSelectedResourceSlotId);
    m_strSelectedResourceAssetId.clear();
    return true;
}

bool_t Client::CEffect_Tool::Try_SoloElement(
	const std::string& strEffectAssetId,
	const std::string& strElementId)
{
	if (strEffectAssetId.empty() || strElementId.empty())
		return false;
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const auto Element = std::find_if(
		m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
		[&strElementId](const EFFECT_ELEMENT_DESC& Element)
		{ return Element.strElementId == strElementId; });
	if (Element == m_ActiveDocument->Elements.end())
	{
		m_strPreviewStatus =
			"Element Solo rejected a missing stable Element ID.";
		return false;
	}
	if (!Is_ElementPreviewAdmitted(*Element))
	{
		m_strPreviewStatus =
			!Is_EffectAuthoringExecutionTarget(
				Element->Material.Execution) ?
			"Element Solo is locked by material/runtime fail-closed admission; editing and Save remain available." :
			"Element Solo is unavailable while the authored Element is hidden.";
		return false;
	}
	const std::string strPreviousElement = m_strPreviewIsolationElementId;
	const std::string strPreviousGroup = m_strPreviewIsolationGroupId;
	m_strPreviewIsolationElementId = strElementId;
	m_strPreviewIsolationGroupId.clear();
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_SELECTED))
	{
		m_strPreviewIsolationElementId = strPreviousElement;
		m_strPreviewIsolationGroupId = strPreviousGroup;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectModelCue(
	const std::string& strEffectAssetId,
	const std::string& strCueId)
{
	const bool_t bChangesSelection = !m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
		EFFECT_DETAIL_SELECTION::MODEL_CUE != m_eDetailSelection ||
		m_strSelectedModelCueId != strCueId;
	if (!bChangesSelection)
		return true;
	if (Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save, Apply, or Revert the open Detail before selecting another Model Cue.";
		return false;
	}
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const auto Iterator = std::find_if(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(),
		[&strCueId](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == strCueId; });
	if (Iterator == m_ActiveDocument->ModelCues.end())
	{
		m_strElementStatus = "Selected Model Cue ID is no longer present.";
		return false;
	}
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	Reset_ModelCueDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::MODEL_CUE;
	m_strSelectedModelCueId = strCueId;
	m_strSelectedElementId.clear();
	m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_strSelectedResourceAssetId.clear();
	return true;
}

bool_t Client::CEffect_Tool::Try_SoloModelCue(
	const std::string& strEffectAssetId,
	const std::string& strCueId)
{
	if (strEffectAssetId.empty() || strCueId.empty())
		return false;
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	if (std::none_of(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(),
		[&strCueId](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == strCueId; }))
	{
		m_strPreviewStatus =
			"Model Cue Solo rejected a missing stable Cue ID.";
		return false;
	}
	const std::string strPreviousCue = m_strPreviewIsolationModelCueId;
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	m_strPreviewIsolationModelCueId = strCueId;
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE))
	{
		m_strPreviewIsolationModelCueId = strPreviousCue;
		m_ePreviewFilter = ePreviousFilter;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_SoloElementGroup(
	const std::string& strEffectAssetId,
	const std::string& strGroupId)
{
	if (strEffectAssetId.empty() || strGroupId.empty())
		return false;
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const bool_t bGroupExists = std::any_of(
		m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
		[&strGroupId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strGroupId == strGroupId &&
				Is_ElementPreviewAdmitted(Element);
		});
	if (!bGroupExists)
	{
		m_strPreviewStatus =
			"Group Solo has no visible authoring-admitted Element to play; hard-locked Elements remain editable and APPROXIMATE Elements remain authoring-preview targets.";
		return false;
	}
	const std::string strPreviousElement = m_strPreviewIsolationElementId;
	const std::string strPreviousGroup = m_strPreviewIsolationGroupId;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId = strGroupId;
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP))
	{
		m_strPreviewIsolationElementId = strPreviousElement;
		m_strPreviewIsolationGroupId = strPreviousGroup;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectComponent(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId)
{
	if (Has_UnappliedDetailDraft() || m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save, Apply, or Revert the open Detail/tuning work before selecting a Component.";
		return false;
	}
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId)
	{
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
	}
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(strComponentAssetId);
	if (nullptr == Component ||
		(Component->strSourceEffectAssetId != strEffectAssetId &&
			Component->strComponentAssetId != strEffectAssetId))
	{
		m_strElementStatus = "Selected Component is not admitted by this Assembly.";
		return false;
	}
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::COMPONENT;
	m_strSelectedComponentId = strComponentAssetId;
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_strSelectedElementId.clear();
	m_strSelectedElementGroupId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus = "Selected stable Effect Component.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectEmitter(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId,
	const std::string& strEmitterId)
{
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(strComponentAssetId);
	if (nullptr == Component ||
		(Component->strSourceEffectAssetId != strEffectAssetId &&
			Component->strComponentAssetId != strEffectAssetId))
	{
		m_strElementStatus = "Selected Emitter has no admitted Component.";
		return false;
	}
	const auto Emitter = std::find_if(Component->Emitters.begin(),
		Component->Emitters.end(),
		[&strEmitterId](const EFFECT_COMPONENT_EMITTER_DESC& Value)
		{
			return Value.strEmitterId == strEmitterId;
		});
	const std::string strActiveDocumentId = m_ActiveDocument.has_value() ?
		m_ActiveDocument->strEffectAssetId : strEffectAssetId;
	if (Component->Emitters.end() == Emitter ||
		!Try_SelectElement(strActiveDocumentId, Emitter->strElementId))
	{
		m_strElementStatus = "Selected Emitter identity is no longer present.";
		return false;
	}
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::EMITTER;
	m_strSelectedComponentId = strComponentAssetId;
	m_strSelectedEmitterId = strEmitterId;
	m_strSelectedSourceModuleId.clear();
	if (const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement();
		nullptr != pElement && !pElement->ResourceBindings.empty())
	{
		m_strSelectedResourceSlotId =
			pElement->ResourceBindings.front().strSlotId;
		m_eResourceLibraryFileKind =
			Resource_FileKind(pElement->ResourceBindings.front());
	}
	m_strElementStatus = "Selected source Emitter; Renderer, Resources, and Module Stack are active.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectSourceModule(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId,
	const std::string& strEmitterId,
	const std::string& strModuleStableId)
{
	if (!Try_SelectEmitter(strEffectAssetId, strComponentAssetId, strEmitterId))
		return false;
	const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement();
	if (nullptr == pElement || std::none_of(
		pElement->SourceRecipe.Modules.begin(),
		pElement->SourceRecipe.Modules.end(),
		[&strModuleStableId](const EFFECT_SOURCE_MODULE_DESC& Module)
		{
			return Module.strStableId == strModuleStableId;
		}))
	{
		m_strElementStatus = "Selected source Module identity is no longer present.";
		return false;
	}
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::SOURCE_MODULE;
	m_strSelectedSourceModuleId = strModuleStableId;
	m_strElementStatus = "Selected source Module; Effect Detail follows its source class.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectFirstEmitter(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId)
{
	if (!strComponentAssetId.empty())
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(strComponentAssetId);
		if (nullptr != Component && !Component->Emitters.empty())
		{
			return Try_SelectEmitter(strEffectAssetId,
				Component->strComponentAssetId,
				Component->Emitters.front().strEmitterId);
		}
		m_strElementStatus =
			"The selected Component has no admitted Emitter.";
		return false;
	}

	const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
		CEffectCatalog::Find_Assembly(strEffectAssetId);
	if (nullptr != Assembly)
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
		{
			const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
				CEffectCatalog::Find_Component(Cue.strComponentAssetId);
			if (nullptr == Component || Component->Emitters.empty())
				continue;
			return Try_SelectEmitter(strEffectAssetId,
				Component->strComponentAssetId,
				Component->Emitters.front().strEmitterId);
		}
	}
	m_strElementStatus = "The selected Skill has no admitted Emitter.";
	return false;
}

bool_t Client::CEffect_Tool::Try_AuditionParticleSystem()
{
    if (!m_ActiveDocument.has_value())
    {
        m_strPreviewStatus =
            "Load an Effect before starting a Particle System audition.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (m_bParticleSystemDraftDirty &&
        !Apply_ParticleSystemDraft(Staged))
    {
        m_strPreviewStatus =
            "Particle System audition rejected: the draft is missing.";
        return false;
    }
    const bool_t bHasParticles = std::any_of(
        Staged.Elements.begin(), Staged.Elements.end(),
        [](const EFFECT_ELEMENT_DESC& Element)
        {
            return EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
        });
    if (!bHasParticles)
    {
        m_strPreviewStatus =
            "Particle System audition rejected: no Particle layers exist.";
        return false;
    }

    const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const bool_t bPreviousPlaying = m_bPreviewPlaying;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM;
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration(Build_PreviewDocument(Staged));
    if (!Stage_WorldPreview(Staged))
    {
        m_ePreviewFilter = ePreviousFilter;
        m_fPreviewTimeSeconds = fPreviousTime;
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_bPreviewPlaying = bPreviousPlaying;
        return false;
    }

    if (const shared_ptr<CEffectObject> pObject =
        m_pWorldPreviewObject.lock())
    {
        Reset_ProductCueSnapshot();
        pObject->Reset();
        pObject->Set_SampleTime(
            Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
		m_bPreviewVisibleRequested = true;
        m_bPreviewPlaying = true;
    }
    else
        m_bPreviewPlaying = false;
    m_strPreviewStatus =
        "Auditioning all Particle layers without Model Cues, Decals, or other kinds.";
    m_strDetailStatus = m_bParticleSystemDraftDirty ?
        "Audition uses the live Particle System draft; Apply then Save to persist." :
        "Auditioning the committed Particle System.";
    return true;
}

bool_t Client::CEffect_Tool::Try_AuditionSelectedElement()
{
    if (!m_ActiveDocument.has_value() || m_strSelectedElementId.empty())
    {
        m_strPreviewStatus =
            "Select an Element before starting an audition preview.";
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (m_bDetailDraftDirty && !Apply_DetailDraft(Staged))
    {
        m_strPreviewStatus =
            "Element audition rejected: the Detail draft target is missing.";
        return false;
    }
    const auto Selected = std::find_if(
        Staged.Elements.begin(), Staged.Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    if (Selected == Staged.Elements.end())
    {
        m_strPreviewStatus =
            "Element audition rejected: the selected Element is missing.";
        return false;
    }
	if (!Is_ElementPreviewAdmitted(*Selected))
	{
		m_strPreviewStatus =
			!Is_EffectAuthoringExecutionTarget(
				Selected->Material.Execution) ?
			"Element audition is locked by material/runtime fail-closed admission; editing and Save remain available." :
			"Element audition is unavailable while the authored Element is hidden.";
		return false;
	}

    const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const bool_t bPreviousPlaying = m_bPreviewPlaying;
	EFFECT_DOCUMENT_DESC AuditionDocument = Staged;
	std::erase_if(AuditionDocument.Elements,
		[this](const EFFECT_ELEMENT_DESC& Element)
		{ return Element.strElementId != m_strSelectedElementId; });
	AuditionDocument.ModelCues.clear();
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_fPreviewTimeSeconds = Resolve_EffectTimelineTime(
		Selected->Detail.Timing.fStartDelaySeconds);
	Recalculate_PreviewDuration(AuditionDocument);
	if (!Stage_WorldPreview(AuditionDocument))
    {
        m_ePreviewFilter = ePreviousFilter;
        m_fPreviewTimeSeconds = fPreviousTime;
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_bPreviewPlaying = bPreviousPlaying;
        return false;
    }
	m_ePreviewFilter = ePreviousFilter;

    if (const shared_ptr<CEffectObject> pObject =
        m_pWorldPreviewObject.lock())
    {
        Reset_ProductCueSnapshot();
        Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
        float4x4_t Root{};
        const bool_t bRootResolved = Resolve_PreviewRoot(Root);
        if (bRootResolved)
            pObject->Set_RootWorld(Root);
        pObject->Reset();
        pObject->Set_SampleTime(
            Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
		pObject->Set_Visible(
			bRootResolved &&
			Is_ProductCueVisible(m_fPreviewTimeSeconds));
		m_bPreviewVisibleRequested = true;
        m_bPreviewPlaying = true;
    }
    else
    {
        m_bPreviewPlaying = false;
    }
    m_strPreviewStatus = "Auditioning selected Element from " +
        std::to_string(m_fPreviewTimeSeconds) + " s.";
    m_strDetailStatus = m_bDetailDraftDirty ?
        "Audition uses the live Detail draft; Apply Detail then Save to persist." :
        "Auditioning the committed Detail; Save is only required after Apply.";
    return true;
}

bool_t Client::CEffect_Tool::Refresh_ResourceCatalog()
{
    m_bResourceCatalogRefreshAttempted = true;
    const std::filesystem::path Root = CRuntimeAssetRoot::Get();
    const std::filesystem::path EffectRoot = Root / L"Effect";
    std::error_code Error;
    if (!std::filesystem::is_directory(EffectRoot, Error) || Error)
    {
        m_strResourceStatus = "Resources/Effect is missing.";
        return false;
    }
    vector<EFFECT_RESOURCE_CATALOG_ENTRY> Staged;
    for (std::filesystem::recursive_directory_iterator Iterator(
        EffectRoot,
        std::filesystem::directory_options::skip_permission_denied,
        Error), End; Iterator != End; Iterator.increment(Error))
    {
        if (Error)
        {
            Error.clear();
            continue;
        }
        if (!Iterator->is_regular_file())
            continue;
        std::string Extension = Iterator->path().extension().string();
        std::transform(Extension.begin(), Extension.end(), Extension.begin(),
            [](const char Character)
            {
                return static_cast<char>(std::tolower(
                    static_cast<unsigned char>(Character)));
            });
        EFFECT_RESOURCE_FILE_KIND eKind = EFFECT_RESOURCE_FILE_KIND::END;
        if (".dds" == Extension)
            eKind = EFFECT_RESOURCE_FILE_KIND::TEXTURE;
        else if (".wmodel" == Extension)
            eKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
        else
            continue;
        const std::filesystem::path Relative =
            Iterator->path().lexically_relative(Root);
        const std::filesystem::path EffectRelative =
            Iterator->path().lexically_relative(EffectRoot);
        if (Relative.empty() || EffectRelative.empty() ||
            EffectRelative.parent_path().empty())
            continue;
        const std::string DomainId = First_PathComponent(EffectRelative);
        if (DomainId.empty())
            continue;
        const std::filesystem::path DomainRelative =
            EffectRelative.lexically_relative(std::filesystem::path(DomainId));
        const std::filesystem::path CategoryPath = DomainRelative.parent_path();
        const string Category = CategoryPath.empty() ?
            "Root" : CategoryPath.generic_string();
        Staged.push_back({
            Relative.generic_string(), DomainId, Category, eKind });
    }
    std::sort(Staged.begin(), Staged.end(),
        [](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
            const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
        {
            return Left.strAssetId < Right.strAssetId;
        });
    Staged.erase(std::unique(Staged.begin(), Staged.end(),
        [](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
            const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
        {
            return Left.strAssetId == Right.strAssetId;
        }), Staged.end());

    std::map<std::string, array<std::set<string>,
        static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)>>
        StagedCategorySets;
    std::map<std::string, array<size_t,
        static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)>>
        StagedResourceCounts;
    for (const EFFECT_RESOURCE_CATALOG_ENTRY& Entry : Staged)
    {
        const size_t iKind = static_cast<size_t>(Entry.eFileKind);
        StagedCategorySets[Entry.strDomainId][iKind].insert("All");
        StagedCategorySets[Entry.strDomainId][iKind].insert(Entry.strCategory);
        ++StagedResourceCounts[Entry.strDomainId][iKind];
    }

    vector<EFFECT_RESOURCE_DOMAIN_CATALOG> StagedDomains;
    StagedDomains.reserve(StagedCategorySets.size());
    for (const auto& [DomainId, CategorySets] : StagedCategorySets)
    {
        EFFECT_RESOURCE_DOMAIN_CATALOG Domain;
        Domain.strDomainId = DomainId;
        Domain.ResourceCounts = StagedResourceCounts[DomainId];
        for (size_t iKind = 0u; iKind < CategorySets.size(); ++iKind)
        {
            Domain.Categories[iKind].assign(
                CategorySets[iKind].begin(), CategorySets[iKind].end());
        }
        StagedDomains.push_back(std::move(Domain));
    }

    m_ResourceCatalog = std::move(Staged);
    m_ResourceDomains = std::move(StagedDomains);
    ++m_iResourceCatalogRevision;
    if (0u == m_iResourceCatalogRevision)
        m_iResourceCatalogRevision = 1u;
    m_iResourceViewRevision = UINT64_MAX;
    m_VisibleResourceIndices.clear();
    m_pThumbnailCache->Invalidate(m_iResourceCatalogRevision);
    const auto SelectedDomain = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [this](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
        {
            return Domain.strDomainId == m_strSelectedAuthoringDomainId;
        });
    if (SelectedDomain == m_ResourceDomains.end() && !m_ResourceDomains.empty())
    {
        const char* pPreferredDomain = Resource_DomainId(m_eAllEffectsClass);
        const auto Preferred = nullptr == pPreferredDomain ?
            m_ResourceDomains.end() : std::find_if(
                m_ResourceDomains.begin(), m_ResourceDomains.end(),
                [pPreferredDomain](const EFFECT_RESOURCE_DOMAIN_CATALOG& Domain)
                {
                    return Domain.strDomainId == pPreferredDomain;
                });
        Select_AuthoringDomain(Preferred == m_ResourceDomains.end() ?
            m_ResourceDomains.front().strDomainId : Preferred->strDomainId);
    }
    m_strResourceStatus = "Catalog refreshed: " +
        std::to_string(m_ResourceCatalog.size()) +
        " supported Resources/Effect files across " +
        std::to_string(m_ResourceDomains.size()) +
        " authoring categories.";
    return true;
}

void Client::CEffect_Tool::Select_AuthoringDomain(
    const std::string& strDomainId)
{
    if (strDomainId.empty() ||
        m_strSelectedAuthoringDomainId == strDomainId)
        return;
    m_strSelectedAuthoringDomainId = strDomainId;
    Copy_Buffer(m_ResourceCategory.data(),
        m_ResourceCategory.size(), "All");
    m_strSelectedResourceAssetId.clear();
    m_iResourceViewRevision = UINT64_MAX;
    m_strResourceStatus = "Resource browser category selected: " +
		strDomainId + ". The Element draft and its bound slots were preserved.";
}

bool_t Client::CEffect_Tool::Select_AuthoringDomainForClass(
    const LostArk::Shared::CHARACTER_CLASS_ID eClass)
{
    const char* pDomainId = Resource_DomainId(eClass);
    if (nullptr == pDomainId)
        return false;
    const auto Domain = std::find_if(
        m_ResourceDomains.begin(), m_ResourceDomains.end(),
        [pDomainId](const EFFECT_RESOURCE_DOMAIN_CATALOG& Candidate)
        {
            return Candidate.strDomainId == pDomainId;
        });
    if (Domain == m_ResourceDomains.end())
    {
        m_strResourceStatus = std::string("Resources/Effect/") + pDomainId +
            " is not available; the previous authoring category was preserved.";
        return false;
    }
    Select_AuthoringDomain(Domain->strDomainId);
    return true;
}

bool_t Client::CEffect_Tool::Try_ResetAuthoringResourceOverride(
	const std::string& strSlotId)
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before resetting resources.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
		return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pElement = nullptr;
	for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
	{
		if (Element.strElementId == m_strSelectedElementId)
		{
			pElement = &Element;
			break;
		}
	}
	if (nullptr == pElement)
		return false;
	std::string strError;
	if (!CEffectDocumentCodec::Reset_AuthoringResourceOverride(
			*pElement, strSlotId, strError))
	{
		m_strResourceStatus = "Reset to Source rejected: " + strError;
		return false;
	}
	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	m_strResourceStatus = "Reset " + strSlotId + " to the source value.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearAuthoringOverrides()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before resetting overrides.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
		return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pElement = nullptr;
	for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
	{
		if (Element.strElementId == m_strSelectedElementId)
		{
			pElement = &Element;
			break;
		}
	}
	if (nullptr == pElement || pElement->AuthoringOverrides.Is_Empty())
		return false;
	std::string strError;
	if (!Reset_AllAuthoringOverrides(*pElement, strError))
	{
		m_strResourceStatus = "Reset all to Source rejected: " + strError;
		return false;
	}
	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	m_strResourceStatus = "All authoring overrides reset to source values.";
	return true;
}

bool_t Client::CEffect_Tool::Try_BindResource(
    const std::string& strAssetId)
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before changing resources.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		m_strResourceStatus =
			"The exact adapter packet is inspection-only. Save the selected Decal/Trail as a generic Authored starting copy before binding resources.";
		return false;
	}
    const EFFECT_ELEMENT_DESC* pSelectedElement = Find_SelectedElement();
    if (!m_ActiveDocument.has_value() || nullptr == pSelectedElement)
    {
        m_strResourceStatus = "Select an Element before choosing a resource.";
        return false;
    }
	if (pSelectedElement->eKind == EFFECT_ELEMENT_KIND::LIGHT ||
		pSelectedElement->eKind == EFFECT_ELEMENT_KIND::SCREEN_POST)
	{
		m_strResourceStatus =
			"Presentation Light and Screen Post have no material resource lanes; edit their typed fields or delete the occurrence.";
		return false;
	}
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    EFFECT_ELEMENT_DESC* pElement = nullptr;
    for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
    {
        if (Element.strElementId == m_strSelectedElementId)
        {
            pElement = &Element;
            break;
        }
    }
	EFFECT_MATERIAL_TEXTURE_LANE_DESC* pMaterialLane =
		nullptr == pElement ? nullptr : Find_MaterialExecutionLane(
			*pElement, m_strSelectedResourceSlotId);
	EFFECT_NAMED_TEXTURE_DESC* pSourceTexture =
		nullptr == pElement ? nullptr : Find_SourceMaterialTexture(
			*pElement, m_strSelectedResourceSlotId);
	EFFECT_RESOURCE_BINDING_DESC* pBinding = nullptr;
	if (nullptr != pElement)
	{
		const auto Binding = std::find_if(
			pElement->ResourceBindings.begin(), pElement->ResourceBindings.end(),
			[this](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
			{ return Candidate.strSlotId == m_strSelectedResourceSlotId; });
		if (Binding != pElement->ResourceBindings.end())
			pBinding = &*Binding;
	}
    if (nullptr == pElement ||
		(nullptr == pMaterialLane && nullptr == pSourceTexture &&
		 !Slot_Allowed(*pElement, m_strSelectedResourceSlotId)))
    {
        m_strResourceStatus = "That resource slot is not allowed for this Element.";
        return false;
    }
	const EFFECT_RESOURCE_FILE_KIND eExpectedKind =
		(nullptr != pMaterialLane || nullptr != pSourceTexture) ?
		EFFECT_RESOURCE_FILE_KIND::TEXTURE :
		Slot_FileKind(*pElement, m_strSelectedResourceSlotId);
    const auto CatalogEntry = std::find_if(
        m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
        [this, &strAssetId, eExpectedKind](
            const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
        {
            return Entry.strAssetId == strAssetId &&
                Entry.strDomainId == m_strSelectedAuthoringDomainId &&
                Entry.eFileKind == eExpectedKind;
        });
    if (CatalogEntry == m_ResourceCatalog.end())
    {
        m_strResourceStatus =
            "Selected resource is outside the active authoring category or file kind.";
        return false;
    }
	std::string strSlotLabel;
	bool_t bUnlockedMissingBaseSourceDecal = false;
	const bool_t bUnlockMissingBaseSourceDecal =
		Is_BaseTextureSlot(m_strSelectedResourceSlotId) &&
		Is_MissingBaseSourceDecal(*pElement);
	/* A hand-authored Element has no compiler lane set to override. Its
	   material template is the declaration, so the first bind of a template
	   slot creates the binding instead of being rejected as undeclared.
	   Imported and runtime documents keep the compiler lane set authoritative. */
	const bool_t bAuthoredTemplateSlot =
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
		Is_DirectHandAuthoredElement(*pElement) &&
		nullptr == pMaterialLane && nullptr == pSourceTexture &&
		nullptr == pBinding &&
		(m_strSelectedResourceSlotId == EFFECT_MESH_SHAPE_SLOT_ID ?
			(EFFECT_ELEMENT_KIND::MESH == pElement->eKind ||
			 EFFECT_ELEMENT_KIND::PARTICLE == pElement->eKind) :
			nullptr != Find_EffectMaterialInput(
				pElement->Material.strTemplateId,
				m_strSelectedResourceSlotId));
	if (bUnlockMissingBaseSourceDecal && nullptr == pBinding)
	{
		// Decal Base is the sole contract that may create a previously absent
		// resource target and change fail-closed preview admission.
		pElement->ResourceBindings.push_back(
			{ m_strSelectedResourceSlotId, strAssetId });
		pElement->Material.Execution.bFailClosed = false;
		pElement->bVisible = true;
		bUnlockedMissingBaseSourceDecal = true;
		strSlotLabel = Slot_Label(*pElement, m_strSelectedResourceSlotId);
	}
	else if (bAuthoredTemplateSlot)
	{
		pElement->ResourceBindings.push_back(
			{ m_strSelectedResourceSlotId, strAssetId });
		strSlotLabel = Slot_Label(*pElement, m_strSelectedResourceSlotId);
	}
	else
	{
		if (nullptr == pMaterialLane && nullptr == pSourceTexture &&
			nullptr == pBinding)
		{
			m_strResourceStatus =
				"Bind rejected: the compiler did not declare that resource lane.";
			return false;
		}
		std::string strError;
		if (!CEffectDocumentCodec::Set_AuthoringResourceOverride(
				*pElement, m_strSelectedResourceSlotId, strAssetId, strError))
		{
			m_strResourceStatus = "Resource override rejected: " + strError;
			return false;
		}
		if (nullptr != pMaterialLane)
		{
			strSlotLabel = pMaterialLane->strRole.empty() ?
				pMaterialLane->strLaneId : pMaterialLane->strRole;
		}
		else if (nullptr != pSourceTexture)
			strSlotLabel = pSourceTexture->strName;
		else
			strSlotLabel = Slot_Label(*pElement, m_strSelectedResourceSlotId);
	}
	const bool_t bWasDrawable = m_bActiveDocumentDrawable;
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
    m_strResourceStatus = "Bound " + strAssetId + " to " +
        strSlotLabel + ".";
	if (bUnlockedMissingBaseSourceDecal)
	{
		m_strResourceStatus +=
			" The imported Decal is now visible and admitted for preview; Save persists this authored Base binding.";
	}
	if (!bWasDrawable && m_bActiveDocumentDrawable)
	{
		m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
		m_strPreviewIsolationElementId.clear();
		m_strPreviewIsolationGroupId.clear();
		m_strPreviewIsolationModelCueId.clear();
		m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
		Start_WorldPreviewFromBeginning();
		m_strResourceStatus +=
			" The Effect became drawable and its Complete preview started.";
	}
    return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedSlot()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strResourceStatus =
			"Apply or Revert the open Detail draft before clearing resources.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		m_strResourceStatus =
			"The exact adapter packet is inspection-only. Save the selected Decal/Trail as a generic Authored starting copy before clearing resources.";
		return false;
	}
    if (!m_ActiveDocument.has_value() || nullptr == Find_SelectedElement())
        return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	bool_t bReset = false;
	bool_t bDeletedOptionalAuthoredBinding = false;
	bool_t bRelockedMissingBaseSourceDecal = false;
    for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
    {
		if (Element.strElementId != m_strSelectedElementId)
			continue;
		const auto Binding = std::find_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[this](const EFFECT_RESOURCE_BINDING_DESC& Candidate)
			{
				return Candidate.strSlotId == m_strSelectedResourceSlotId;
			});
		const bool_t bHasAuthoringOverride =
			Element.AuthoringOverrides.ResourceBindings.end() != std::find_if(
				Element.AuthoringOverrides.ResourceBindings.begin(),
				Element.AuthoringOverrides.ResourceBindings.end(),
				[this](const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
				{
					return Override.strSlotId ==
						m_strSelectedResourceSlotId;
				});
		const bool_t bDeleteOptionalAuthoredBinding =
			(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
			 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
			Binding != Element.ResourceBindings.end() &&
			!bHasAuthoringOverride &&
			nullptr == Find_MaterialExecutionLane(
				Element, m_strSelectedResourceSlotId) &&
			nullptr == Find_SourceMaterialTexture(
				Element, m_strSelectedResourceSlotId) &&
			Is_OptionalHandAuthoredResourceSlot(
				Element, m_strSelectedResourceSlotId);
		if (bDeleteOptionalAuthoredBinding)
		{
			Element.ResourceBindings.erase(Binding);
			bReset = true;
			bDeletedOptionalAuthoredBinding = true;
		}
		else if (Is_BaseTextureSlot(m_strSelectedResourceSlotId) &&
			Is_SourceDecalBaseAdmissionCarrier(Element))
		{
			const size_t iPreviousCount = Element.ResourceBindings.size();
			std::erase_if(Element.ResourceBindings,
				[this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
				{ return Binding.strSlotId == m_strSelectedResourceSlotId; });
			std::erase_if(Element.AuthoringOverrides.ResourceBindings,
				[this](
					const EFFECT_AUTHORING_RESOURCE_OVERRIDE_DESC& Override)
				{ return Override.strSlotId == m_strSelectedResourceSlotId; });
			bReset = Element.ResourceBindings.size() != iPreviousCount;
			if (bReset)
			{
				Element.Material.Execution.bFailClosed = true;
				Element.bVisible = false;
				bRelockedMissingBaseSourceDecal = true;
			}
		}
		else
		{
			std::string strError;
			bReset = CEffectDocumentCodec::Reset_AuthoringResourceOverride(
				Element, m_strSelectedResourceSlotId, strError);
			if (!bReset)
			{
				m_strResourceStatus =
					"Compiler-owned lanes cannot be deleted. " + strError;
				return false;
			}
		}
        break;
    }
	if (!bReset)
	{
		m_strResourceStatus =
			"The selected resource lane no longer exists on this Element.";
		return false;
	}
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
	if (bDeletedOptionalAuthoredBinding)
		m_strSelectedResourceAssetId.clear();
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	m_strResourceStatus = bRelockedMissingBaseSourceDecal ?
		"Cleared the Decal Base exception." :
		bDeletedOptionalAuthoredBinding ?
			"Deleted the selected optional authored resource slot. Save Changes to persist it." :
			"Reset the selected resource lane to Source.";
	if (bRelockedMissingBaseSourceDecal)
	{
		m_strResourceStatus +=
			" The imported Decal was hidden and returned to fail-closed until a Base DDS is bound.";
	}
    return true;
}

bool_t Client::CEffect_Tool::Try_SetSelectedTrailFollowAnchor(
	const std::string& strBoneName)
{
	if (Has_UnappliedDetailDraft())
	{
		m_strPreviewStatus =
			"Apply or Revert the open Detail draft before changing Trail follow.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
	{
		m_strPreviewStatus =
			"Load an authored Effect and select one Trail before setting follow.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource)
	{
		m_strPreviewStatus =
			"Trail follow is editable only on a New or Authored Effect document.";
		return false;
	}
	if (strBoneName.empty())
	{
		m_strPreviewStatus =
			"Enter a Socket / Bone before setting Trail follow.";
		return false;
	}
	float4x4_t AnchorWorld{};
	if (!CAnimationTargetService::Resolve_AnchorTransform(
			strBoneName.c_str(), &AnchorWorld))
	{
		m_strPreviewStatus = "Trail follow rejected: model bone '" +
			strBoneName + "' does not exist on the current target.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	auto Selected = std::find_if(Staged.Elements.begin(), Staged.Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == m_strSelectedElementId;
		});
	if (Selected == Staged.Elements.end() ||
		EFFECT_ELEMENT_KIND::TRAIL != Selected->eKind)
	{
		m_strPreviewStatus =
			"Select one Trail Element before setting follow.";
		return false;
	}

	EFFECT_ACTION_CUE_ATTACHMENT_DESC Attachment =
		Selected->ActionCueAttachment;
	Attachment.bEnabled = true;
	Attachment.bFollow = true;
	Attachment.strSourceAnchorSlotId = strBoneName;
	/* Element IDs are stable and unique inside a Document, which makes them a
	   safe per-playback anchor-map key without inventing a vector-index ID. */
	Attachment.strRuntimeAnchorSlotId = Selected->strElementId;
	Attachment.strRuntimeBoneName = strBoneName;
	Attachment.fSnapshotRootSourceBasisYawDegrees = 0.f;
	Selected->ActionCueAttachment = std::move(Attachment);

	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	Start_WorldPreviewFromBeginning();
	m_strPreviewStatus = "Selected Trail now follows model bone '" +
		strBoneName + "'. Save Changes to persist the attachment.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedTrailFollowAnchor()
{
	if (Has_UnappliedDetailDraft())
	{
		m_strPreviewStatus =
			"Apply or Revert the open Detail draft before clearing Trail follow.";
		return false;
	}
	if (!m_ActiveDocument.has_value())
		return false;
	if (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT != m_eActiveDocumentSource &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource)
	{
		m_strPreviewStatus =
			"Trail follow is editable only on a New or Authored Effect document.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	auto Selected = std::find_if(Staged.Elements.begin(), Staged.Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == m_strSelectedElementId;
		});
	if (Selected == Staged.Elements.end() ||
		EFFECT_ELEMENT_KIND::TRAIL != Selected->eKind ||
		!Selected->ActionCueAttachment.bEnabled)
	{
		m_strPreviewStatus =
			"The selected Trail has no element-local follow to clear.";
		return false;
	}
	Selected->ActionCueAttachment = {};

	if (!Try_CommitDocument(std::move(Staged)))
		return false;
	if (!m_bDetailDraftDirty && m_DetailDraft.has_value() &&
		m_strDetailDraftElementId == m_strSelectedElementId)
	{
		if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
		{
			m_DetailDraft = *pCommitted;
			Refresh_DetailDraftAdmission(*pCommitted);
		}
	}
	Start_WorldPreviewFromBeginning();
	m_strPreviewStatus =
		"Cleared the selected Trail follow. A stationary root will not form a ribbon unless Transform velocity moves it.";
	return true;
}

bool_t Client::CEffect_Tool::Try_CommitDocument(
    EFFECT_DOCUMENT_DESC&& Staged)
{
	if (m_bOccurrenceTuningDirty ||
		EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
	{
		m_strElementStatus =
			"Save the occurrence tuning artifact, then select an authored Effect before editing its Document.";
		return false;
	}
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
	{
		m_strElementStatus =
			"The exact adapter packet is read-only in full Details. Persist position/rotation/scale through Stable occurrence tuning; material/resource Save As requires a paired adapter authoring contract.";
		return false;
	}
    std::string Error;
    if (!CEffectDocumentCodec::Validate(Staged, Error))
    {
        m_strElementStatus = Error;
        return false;
    }
    std::string DrawableError;
    if (!CEffectDocumentCodec::Validate_Drawable(Staged, DrawableError))
    {
        m_ActiveDocument = std::move(Staged);
        Set_ActiveDocumentDrawableStatus(false, DrawableError);
        m_bDocumentDirty = true;
        m_bActiveDocumentMatchesRuntime = false;
		m_strSaveHotReloadStatus.clear();
        Recalculate_PreviewDuration();
        Release_WorldPreview(true);
        m_strPreviewStatus =
            "Document draft committed; preview hidden until required resources bind: " +
            DrawableError;
        return true;
    }
    if (!Stage_WorldPreview(Staged))
    {
        m_strElementStatus =
            "Change rejected; active Document and preview were preserved: " +
            m_strPreviewStatus;
        return false;
    }
    m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(true, {});
    m_bDocumentDirty = true;
    m_bActiveDocumentMatchesRuntime = false;
	m_strSaveHotReloadStatus.clear();
    Recalculate_PreviewDuration();
    return true;
}

bool_t Client::CEffect_Tool::Try_SetPreviewFilter(
    const EFFECT_PREVIEW_FILTER eFilter)
{
    if (EFFECT_PREVIEW_FILTER::END == eFilter)
        return false;
    if (!m_ActiveDocument.has_value())
    {
        m_strPreviewStatus =
            "Load one Data File before choosing a preview scope.";
        return false;
    }
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED == eFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED == eFilter) &&
		m_strPreviewIsolationElementId.empty())
    {
        m_strPreviewStatus =
			"Use an Element Solo button before choosing Element Solo/Mute.";
        return false;
    }
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == eFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == eFilter) &&
		m_strPreviewIsolationGroupId.empty())
    {
        m_strPreviewStatus =
			"Use a Play Group button before choosing Group Solo/Mute.";
        return false;
    }
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE == eFilter &&
		m_strPreviewIsolationModelCueId.empty())
	{
		m_strPreviewStatus =
			"Use a Model / Summon Solo button before choosing Model Cue Solo.";
		return false;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES == eFilter &&
		m_ActiveDocument->ModelCues.empty())
	{
		m_strPreviewStatus = "The active Effect has no Model / Summon cue.";
		return false;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY == eFilter &&
		EFFECT_AUTHORING_FAMILY::END == m_ePreviewIsolationAuthoringFamily)
	{
		m_strPreviewStatus =
			"Use a Play Family button before choosing Family preview.";
		return false;
	}

    const EFFECT_PREVIEW_FILTER ePrevious = m_ePreviewFilter;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    m_ePreviewFilter = eFilter;
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (m_bParticleSystemDraftDirty)
        Apply_ParticleSystemDraft(Staged);
    if (m_bDetailDraftDirty)
        Apply_DetailDraft(Staged);
	if (m_bModelCueDraftDirty)
		Apply_ModelCueDraft(Staged);
    Recalculate_PreviewDuration(Build_PreviewDocument(Staged));
    if (!Stage_WorldPreview(Staged))
    {
        m_ePreviewFilter = ePrevious;
        m_fPreviewTimeSeconds = fPreviousTime;
        m_fPreviewDurationSeconds = fPreviousDuration;
        return false;
    }
    return true;
}

bool_t Client::CEffect_Tool::Ensure_WorldPreviewObject()
{
    if (nullptr != m_pWorldPreviewObject.lock())
        return true;
    const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
    CEffectObject::EFFECT_OBJECT_DESC Desc{};
    Desc.pDocument = nullptr;
    Desc.RootWorld = m_PreviewWorldRoot;
    Desc.bAutoPlay = false;
    shared_ptr<CGameObject> pGameObject;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectObject",
        iLevel, PREVIEW_LAYER, &Desc, &pGameObject)))
    {
        m_strPreviewStatus =
            "EffectObject prototype is not registered for world preview.";
        return false;
    }
    const shared_ptr<CEffectObject> pEffect =
        dynamic_pointer_cast<CEffectObject>(pGameObject);
    if (nullptr == pEffect)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            iLevel, PREVIEW_LAYER, pGameObject);
        m_strPreviewStatus = "World preview clone returned the wrong type.";
        return false;
    }
    m_pWorldPreviewObject = pEffect;
    m_iWorldPreviewLevel = iLevel;
    return true;
}

bool_t Client::CEffect_Tool::Try_StartArtist31470FullPreview()
{
	if (!Ensure_WorldPreviewObject())
	{
		Reset_ReconstructedSourceRuntimeTimeline();
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject)
	{
		Reset_ReconstructedSourceRuntimeTimeline();
		m_strPreviewStatus =
			"Artist Core F (33) EffectObject is unavailable.";
		return false;
	}
	const auto FailStart = [this, &pObject](std::string Reason)
	{
		pObject->Set_Playing(false);
		pObject->Set_Visible(false);
		Set_SynchronizedAnimationPaused(true);
		Reset_SynchronizedAnimationSequence();
		Reset_ReconstructedSourceRuntimeTimeline();
		m_bReconstructedDiagnosticActive = false;
		m_bReconstructedSourceRuntimeActive = false;
		m_pVisualPreviewProjection.reset();
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = std::move(Reason);
		return false;
	};

	/* Resource preparation is synchronous and may consume several seconds.
	   Neither the previous Object nor its animation may absorb that wall time. */
	pObject->Set_Playing(false);
	Set_SynchronizedAnimationPaused(true);
	std::string Error;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION> Preparation;
	if (!CEffectPresentationService::Acquire_ReconstructedArtist31470(
		m_pDevice, m_pContext, Preparation, Error))
	{
		return FailStart(
			"Artist Core F (33) cache failed: " + Error);
	}
	if (!Ensure_ArtistFSourceAuthoringOverlaySession())
	{
		return FailStart(m_strElementStatus.empty() ?
			"Artist F source-backed edit session could not be opened." :
			m_strElementStatus);
	}
	const bool_t bReusePreparedPreview =
		m_bReconstructedSourceRuntimeActive &&
		pObject->Is_ReconstructedSourceRuntimeActive() &&
		pObject->Get_ReconstructedRuntimePreparation().get() ==
			Preparation.get();
	/* A successful Tool start always obtains a fresh consumption receipt.
	   Reuse skips resource preparation, not the shared cache attach boundary. */
	if (!Synchronize_Artist31470FullPreview(Preparation))
	{
		return FailStart(m_strPreviewAnimationStatus);
	}
	if (!CEffectPresentationService::Stage_ReconstructedArtist31470Preview(
			pObject, Preparation, Error))
	{
		return FailStart(
			"Artist Core F (33) cache stage failed: " + Error);
	}
	EFFECT_ARTIST_31470_CACHE_PROBE CacheProbe;
	if (!CEffectPresentationService::Get_ReconstructedArtist31470CacheProbe(
		CacheProbe, Error))
	{
		return FailStart(
			"Artist Core F (33) cache receipt probe failed: " + Error);
	}
	const bool_t bToolReceiptMatches =
		CacheProbe.Current.Is_ExactCoreScope() &&
		CacheProbe.iToolPreviewConsumeCount > 0u &&
		CacheProbe.Current.Matches(
			CacheProbe.LastToolPreviewConsumption);
	const bool_t bGameplayReceiptExists =
		CacheProbe.iGameplayConsumeCount > 0u;
	if (!bToolReceiptMatches ||
		(bGameplayReceiptExists &&
		 !CacheProbe.Current.Matches(
			 CacheProbe.LastGameplayConsumption)))
	{
		return FailStart(
			"Artist Core F (33) Tool/gameplay cache identity diverged.");
	}
	m_bReconstructedDiagnosticActive = false;
	m_bReconstructedSourceRuntimeActive = true;
	m_pVisualPreviewProjection.reset();
	m_bPreviewPlaying = true;
	m_bPreviewVisibleRequested = true;
	m_fPreviewTimeSeconds = 0.f;
	Reset_ReconstructedSourceRuntimeTimeline();
	if (!Prepare_ReconstructedSourceRuntimeTransformHistory())
	{
		return FailStart(
			"Artist F historical anchor preparation failed: " +
			m_strPreviewAnimationStatus);
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		Preparation->Get_Program();
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		Preparation->Get_CatalogEntry();
	const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT> pLegacyTuning =
		nullptr == pEntry ? nullptr : pEntry->Get_OccurrenceTuning();
	/* An empty source overlay is a true no-op only while the shared cache also
	   carries no legacy occurrence overrides.  If legacy overrides exist, an
	   intentionally empty overlay means reset-to-source and must restage. */
	const bool_t bRequiresSourceOverlayRestage =
		m_SourceAuthoringOverlayDocument.has_value() &&
		(!m_SourceAuthoringOverlayDocument->Entries.empty() ||
		 (nullptr != pLegacyTuning && !pLegacyTuning->Entries.empty()));
	if (nullptr != pProgram && bRequiresSourceOverlayRestage)
	{
		EFFECT_DOCUMENT_DESC SourceTransformDocument;
		std::string SourceTransformError;
		if (!CEffectReconstructedSourceRuntimeFactory::Build_Document(
				Preparation, SourceTransformDocument, SourceTransformError,
				EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS) ||
			!CEffectSourceAuthoringOverlayCodec::Apply_ToProjectedDocument(
				SourceTransformDocument, *pProgram,
				*m_SourceAuthoringOverlayDocument, SourceTransformError))
		{
			return FailStart(
				"Artist F source cue-local overlay validation failed: " +
				SourceTransformError);
		}
	}
	if (bRequiresSourceOverlayRestage && nullptr != pProgram &&
		m_SourceAuthoringOverlayDocument->strEffectAssetId ==
			pProgram->strRuntimeCatalogAssetId &&
		!CEffectPresentationService::
			Stage_ReconstructedSourceAuthoringOverlayPreview(
				m_pDevice, m_pContext, pObject, Preparation,
				*m_SourceAuthoringOverlayDocument, Error))
	{
		return FailStart(
			"Source-backed edit preview restage failed: " + Error);
	}
	else if (!m_SourceAuthoringOverlayDocument.has_value() &&
		m_OccurrenceTuningDocument.has_value() && nullptr != pProgram &&
		m_OccurrenceTuningDocument->strEffectAssetId ==
			pProgram->strRuntimeCatalogAssetId &&
		!CEffectPresentationService::Stage_ReconstructedOccurrenceTuningPreview(
			m_pDevice, m_pContext, pObject, Preparation,
			*m_OccurrenceTuningDocument, Error))
	{
		return FailStart(
			"Occurrence tuning preview restage failed: " + Error);
	}
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this](const f32_t fSampleTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strOutError)
		{
			return Build_ReconstructedSourceRuntimeTransformSample(
				fSampleTimeSeconds, OutSample, strOutError);
		};
	if (!pObject->Set_SampleTimeWithTransformHistory(
			0.f, TransformProvider, Error))
	{
		return FailStart(
			"Artist F historical zero-frame stage failed: " + Error);
	}
	m_bReconstructedSourceRuntimeStartPending = true;
	pObject->Set_Playing(false);
	/* Do not expose a zero-time frame whose bone palette predates the deferred
	   animation update.  The next Tool update refreshes the exact zero pose,
	   its normalized source anchors, and only then publishes visibility. */
	pObject->Set_Visible(false);
	m_strPreviewStatus =
		bReusePreparedPreview ?
		"Artist Core F (35 document / 33 visible) reused the shared CORE_RENDERERS cache" +
			std::string(bGameplayReceiptExists ?
				" and matches the latest gameplay F receipt; " :
				"; no gameplay F receipt exists yet; ") +
			"playback starts on the next update." :
		"Artist Core F (35 document / 33 visible) consumed the shared CORE_RENDERERS cache" +
			std::string(bGameplayReceiptExists ?
				" and matches the latest gameplay F receipt; " :
				"; no gameplay F receipt exists yet; ") +
			"playback starts on the next update.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ResetArtist31470PreviewIsolation()
{
	if (!m_bReconstructedSourceRuntimeActive &&
		!Try_StartArtist31470FullPreview())
	{
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive())
	{
		m_strPreviewStatus =
			"Artist Core F isolation requires the shared Core33 preview.";
		return false;
	}
	pObject->Reset_PreviewSubmissionIsolation();
	m_strPreviewStatus =
		"Artist Core F submission isolation: ALL 33 stable occurrences.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SetArtist31470PreviewFamilyIsolation(
	const EFFECT_GPU_RENDER_FAMILY eFamily)
{
	if (eFamily != EFFECT_GPU_RENDER_FAMILY::MESH &&
		eFamily != EFFECT_GPU_RENDER_FAMILY::SPRITE &&
		eFamily != EFFECT_GPU_RENDER_FAMILY::DECAL &&
		eFamily != EFFECT_GPU_RENDER_FAMILY::RIBBON)
	{
		m_strPreviewStatus = "Artist Core F isolation family is invalid.";
		return false;
	}
	if (!m_bReconstructedSourceRuntimeActive &&
		!Try_StartArtist31470FullPreview())
	{
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive())
	{
		m_strPreviewStatus =
			"Artist Core F isolation requires the shared Core33 preview.";
		return false;
	}
	EFFECT_PREVIEW_SUBMISSION_ISOLATION Isolation;
	Isolation.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::FAMILY;
	Isolation.eFamily = eFamily;
	std::string Error;
	if (!pObject->Set_PreviewSubmissionIsolation(Isolation, Error))
	{
		m_strPreviewStatus = "Artist Core F family isolation failed: " + Error;
		return false;
	}
	m_strPreviewStatus = std::string("Artist Core F submission isolation: ") +
		ArtistCoreFamilyLabel(eFamily) + ".";
	return true;
}

void Client::CEffect_Tool::Reset_ArtistFPreparationFailureLatch()
{
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
		m_eArtistFSourcePreparationState)
	{
		m_eArtistFSourcePreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFSourcePreparationAttemptRevision = UINT64_MAX;
	}
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
		m_eArtistFMaterialPreparationState)
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
	}
}

bool_t Client::CEffect_Tool::Ensure_ArtistFSourceSnapshotForAuthoring()
{
	const uint64_t iRuntimeRevision = CEffectCatalog::Get_RuntimeRevision();
	if (m_iArtistFSourceSnapshotRevision != iRuntimeRevision &&
		m_iArtistFSourcePreparationAttemptRevision != iRuntimeRevision)
	{
		m_eArtistFSourcePreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFSourcePreparationAttemptRevision = UINT64_MAX;
		m_pArtistFSourcePreparation.reset();
		m_pArtistFSourceProjection.reset();
		m_iArtistFSourceSnapshotRevision = UINT64_MAX;
		m_ArtistFMaterialExecutionSnapshots.clear();
		m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
	}
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> pVisualProgram =
		CEffectCatalog::Find_VisualProgram(ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	if (iRuntimeRevision == m_iArtistFSourceSnapshotRevision &&
		nullptr != m_pArtistFSourcePreparation &&
		nullptr != m_pArtistFSourceProjection &&
		nullptr != pEntry && nullptr != pProgram && nullptr != pVisualProgram &&
		m_pArtistFSourcePreparation->Get_CatalogEntry().get() == pEntry.get() &&
		m_pArtistFSourcePreparation->Get_Program().get() == pProgram.get() &&
		m_pArtistFSourceProjection->Is_Valid() &&
		m_pArtistFSourceProjection->Get_EffectAssetId() ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		m_pArtistFSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 &&
		m_pArtistFSourceProjection->Get_ProgramSha256() ==
			pVisualProgram->strProgramSha256)
	{
		m_eArtistFSourcePreparationState =
			ARTIST_F_PREPARATION_STATE::READY;
		m_iArtistFSourcePreparationAttemptRevision = iRuntimeRevision;
		return true;
	}
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFSourcePreparationState &&
		m_iArtistFSourcePreparationAttemptRevision == iRuntimeRevision)
	{
		return false;
	}
	m_iArtistFSourcePreparationAttemptRevision = iRuntimeRevision;
	m_eArtistFSourcePreparationState = ARTIST_F_PREPARATION_STATE::FAILED;
	m_pArtistFSourcePreparation.reset();
	m_pArtistFSourceProjection.reset();
	m_iArtistFSourceSnapshotRevision = UINT64_MAX;
	m_ArtistFMaterialExecutionSnapshots.clear();
	m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
	m_eArtistFMaterialPreparationState =
		ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
	m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
	Engine::CProfilerScope PreparationProfile(
		CGameInstance::Get().Get_Profiler(),
		"EffectTool.ArtistF.SourcePreparation");

	std::string Error;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		StagedPreparation;
	if (!CEffectCatalog::Prepare_ReconstructedRuntimeProgram(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID, StagedPreparation, Error) ||
		nullptr == StagedPreparation ||
		nullptr == StagedPreparation->Get_CatalogEntry() ||
		nullptr == StagedPreparation->Get_Program())
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F source recipe is unavailable." : Error;
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	if (!CEffectReconstructedSourceRuntimeFactory::Build_Document(
			StagedPreparation, StagedDocument, Error,
			EFFECT_RECONSTRUCTED_VISUAL_SCOPE::CORE_RENDERERS))
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F source recipe could not be opened." : Error;
		return false;
	}
	const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT> pTuning =
		StagedPreparation->Get_CatalogEntry()->Get_OccurrenceTuning();
	if (nullptr != pTuning &&
		!CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
			StagedDocument, *StagedPreparation->Get_Program(), *pTuning, Error))
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F saved source adjustments are invalid." : Error;
		return false;
	}

	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_CORPUS> pCorpus =
		CEffectCatalog::Find_VisualProgramCorpus();
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		StagedProjection;
	if (nullptr == pCorpus ||
		!CEffectVisualProgramCorpusCodec::Create_DocumentProjection(
			*pCorpus, StagedDocument, StagedProjection, Error) ||
		nullptr == StagedProjection || !StagedProjection->Is_Valid() ||
		StagedProjection->Get_EffectAssetId() !=
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID ||
		StagedProjection->Get_ProjectionKind() !=
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 ||
		StagedProjection->Get_ProgramSha256() !=
			pVisualProgram->strProgramSha256)
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F source recipe identity changed while opening." : Error;
		return false;
	}

	static constexpr std::array<size_t, 4u> EXPECTED_COUNTS{
		13u, 16u, 3u, 1u };
	std::array<size_t, 4u> Counts{};
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter :
		StagedPreparation->Get_Program()->Emitters)
	{
		EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const size_t iFamily = eFamily == EFFECT_GPU_RENDER_FAMILY::MESH ? 0u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ? 1u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ? 2u : 3u;
		const auto Element = std::find_if(
			StagedProjection->Get_Document().Elements.begin(),
			StagedProjection->Get_Document().Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Emitter.strSourceElementId; });
		if (Element == StagedProjection->Get_Document().Elements.end() ||
			!Element->bVisible)
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F source recipe is missing a visible family Element.";
			return false;
		}
		++Counts[iFamily];
	}
	if (Counts != EXPECTED_COUNTS)
	{
		m_strArtistFSourceSnapshotStatus =
			"Artist F source recipe family counts changed; import was not opened.";
		return false;
	}

	m_pArtistFSourcePreparation = std::move(StagedPreparation);
	m_pArtistFSourceProjection = std::move(StagedProjection);
	m_iArtistFSourceSnapshotRevision = iRuntimeRevision;
	m_iArtistFSourcePreparationAttemptRevision = iRuntimeRevision;
	m_eArtistFSourcePreparationState = ARTIST_F_PREPARATION_STATE::READY;
	m_ArtistFMaterialExecutionSnapshots.clear();
	m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
	m_strArtistFSourceSnapshotStatus =
		"Artist F source recipe ready: Mesh 13, Sprite 16, Decal 3, Ribbon 1.";
	return true;
}

bool_t Client::CEffect_Tool::Ensure_ArtistFMaterialExecutionSnapshots()
{
	const uint64_t iRuntimeRevision = CEffectCatalog::Get_RuntimeRevision();
	if (m_iArtistFMaterialExecutionSnapshotRevision != iRuntimeRevision &&
		m_iArtistFMaterialPreparationAttemptRevision != iRuntimeRevision)
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
		m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
		m_ArtistFMaterialExecutionSnapshots.clear();
		m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
	}
	if (ARTIST_F_PREPARATION_STATE::FAILED ==
			m_eArtistFMaterialPreparationState &&
		m_iArtistFMaterialPreparationAttemptRevision == iRuntimeRevision)
	{
		return false;
	}
	if (!Ensure_ArtistFSourceSnapshotForAuthoring() ||
		nullptr == m_pArtistFSourcePreparation ||
		nullptr == m_pArtistFSourceProjection ||
		nullptr == m_pArtistFSourcePreparation->Get_Program())
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::FAILED;
		m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
		return false;
	}
	if (m_iArtistFMaterialExecutionSnapshotRevision ==
			m_iArtistFSourceSnapshotRevision &&
		!m_ArtistFMaterialExecutionSnapshots.empty())
	{
		m_eArtistFMaterialPreparationState =
			ARTIST_F_PREPARATION_STATE::READY;
		m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
		return true;
	}
	m_eArtistFMaterialPreparationState = ARTIST_F_PREPARATION_STATE::FAILED;
	m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
	Engine::CProfilerScope PreparationProfile(
		CGameInstance::Get().Get_Profiler(),
		"EffectTool.ArtistF.MaterialPreparation");

	std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC> Staged;
	std::string Error;
	if (!CEffectDocumentRenderer::
		Bake_ReconstructedMaterialExecutionSnapshots(
			m_pDevice, m_pContext,
			m_pArtistFSourceProjection->Get_Document(),
			m_pArtistFSourcePreparation, Staged, Error))
	{
		m_strArtistFSourceSnapshotStatus = Error.empty() ?
			"Artist F Track A material execution snapshots could not be baked." :
			Error;
		return false;
	}

	size_t iCoreCount = 0u;
	size_t iEnabledCount = 0u;
	size_t iRuntimeMaterialCount = 0u;
	size_t iArtistVisualCount = 0u;
	size_t iLocalDecalCount = 0u;
	size_t iFiniteCommonCount = 0u;
	size_t iFailClosedCount = 0u;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter :
		m_pArtistFSourcePreparation->Get_Program()->Emitters)
	{
		EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const auto Snapshot = Staged.find(Emitter.strSourceElementId);
		if (Snapshot == Staged.end())
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A material bake lost a Core33 Element identity.";
			return false;
		}
		++iCoreCount;
		if (!Emitter.strMaterialOccurrenceId.has_value())
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A material row lost its stable occurrence ID.";
			return false;
		}
		const auto Registry = Find_Artist31470ShaderRegistry(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
		if (!Registry.has_value() ||
			!Validate_Artist31470ShaderRegistryEmitterIdentity(
				Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
				Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
		{
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A shader-registry identity changed.";
			return false;
		}
		if (!Snapshot->second.bEnabled)
		{
			if (Registry->eBackend ==
					EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
				Registry->eFidelity ==
					EFFECT_ARTIST31470_SHADER_FIDELITY::BOUNDED_EXPLICIT &&
				Registry->bDrawAdmitted && Emitter.Row.iOrder == 17u)
			{
				++iFiniteCommonCount;
			}
			else if (Registry->eBackend ==
					EFFECT_ARTIST31470_SHADER_BACKEND::NONE &&
				Registry->eFidelity ==
					EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
				!Registry->bDrawAdmitted)
			{
				++iFailClosedCount;
			}
			else
			{
				m_strArtistFSourceSnapshotStatus =
					"Artist F disabled material row is neither FiniteCommon nor fail-closed.";
				return false;
			}
			continue;
		}
		++iEnabledCount;
		switch (Snapshot->second.eBackend)
		{
		case EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2:
			++iRuntimeMaterialCount;
			break;
		case EFFECT_MATERIAL_EXECUTION_BACKEND::ARTIST_VISUAL_V4:
			++iArtistVisualCount;
			break;
		case EFFECT_MATERIAL_EXECUTION_BACKEND::LOCAL_DECAL:
			++iLocalDecalCount;
			break;
		case EFFECT_MATERIAL_EXECUTION_BACKEND::STANDARD_COLOR_V1:
		case EFFECT_MATERIAL_EXECUTION_BACKEND::GENERIC:
		case EFFECT_MATERIAL_EXECUTION_BACKEND::END:
		default:
			m_strArtistFSourceSnapshotStatus =
				"Artist F Track A material bake enabled an unsupported backend.";
			return false;
		}
	}
	if (iCoreCount != 33u || iEnabledCount != 28u ||
		iRuntimeMaterialCount + iLocalDecalCount != 18u ||
		iArtistVisualCount != 10u || iLocalDecalCount != 2u ||
		iFiniteCommonCount != 1u || iFailClosedCount != 4u)
	{
		m_strArtistFSourceSnapshotStatus =
			"Artist F Track A material denominator changed; expected typed 28 (RuntimeMaterialV2 family 18 including LocalDecal 2, ArtistVisualV4 10), FiniteCommon 1, fail-closed 4.";
		return false;
	}

	m_ArtistFMaterialExecutionSnapshots = std::move(Staged);
	m_iArtistFMaterialExecutionSnapshotRevision =
		m_iArtistFSourceSnapshotRevision;
	m_iArtistFMaterialPreparationAttemptRevision = iRuntimeRevision;
	m_eArtistFMaterialPreparationState = ARTIST_F_PREPARATION_STATE::READY;
	m_strArtistFSourceSnapshotStatus =
		"Artist F Track A material snapshots ready: typed 28, FiniteCommon bounded 1, fail-closed 4.";
	return true;
}

bool_t Client::CEffect_Tool::Ensure_ArtistFSourceAuthoringOverlaySession()
{
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	if (nullptr == pEntry || nullptr == pProgram ||
		pProgram->strRuntimeCatalogAssetId != ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		m_strElementStatus =
			"Artist F Track A source Program is unavailable.";
		return false;
	}
	if (m_SourceAuthoringOverlayDocument.has_value() &&
		m_SourceAuthoringOverlayDocument->strEffectAssetId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		std::string Error;
		if (!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				*m_SourceAuthoringOverlayDocument, *pProgram, Error))
		{
			m_strElementStatus = Error;
			return false;
		}
		if (m_SourceAuthoringOverlayPath.empty())
		{
			m_strElementStatus =
				"Artist F source-backed edit path is unavailable.";
			return false;
		}
		std::error_code FileError;
		const bool_t bExists = std::filesystem::is_regular_file(
			m_SourceAuthoringOverlayPath, FileError);
		if (FileError)
		{
			m_strElementStatus =
				"Artist F saved overlay path could not be inspected: " +
				FileError.message();
			return false;
		}
		if (!m_bOccurrenceTuningDirty &&
			!m_bOccurrenceTransformDraftDirty)
		{
			if (!bExists &&
				!m_strSourceAuthoringOverlayBaselineCanonical.empty())
			{
				m_strElementStatus =
					"Artist F saved overlay disappeared; use Reload Saved before editing.";
				return false;
			}
			if (bExists &&
				m_strSourceAuthoringOverlayBaselineCanonical.empty() &&
				m_bSourceAuthoringOverlayNeedsInitialSave)
			{
				m_strElementStatus =
					"Artist F saved overlay appeared; use Reload Saved before editing.";
				return false;
			}
			if (bExists)
			{
				EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Disk;
				if (!CEffectSourceAuthoringOverlayCodec::Load(
						m_SourceAuthoringOverlayPath, Disk, Error) ||
					!CEffectSourceAuthoringOverlayCodec::
						Validate_AgainstProgram(Disk, *pProgram, Error))
				{
					m_strElementStatus = Error.empty() ?
						"Artist F saved overlay could not be reloaded." : Error;
					return false;
				}
				const std::string DiskCanonical =
					CEffectSourceAuthoringOverlayCodec::Serialize(Disk);
				if (DiskCanonical !=
					m_strSourceAuthoringOverlayBaselineCanonical)
				{
					m_strElementStatus =
						"Artist F saved overlay changed on disk; use Reload Saved before editing.";
					return false;
				}
				m_bSourceAuthoringOverlayNeedsInitialSave = false;
			}
		}
		return true;
	}
	if (m_bOccurrenceTuningDirty || m_bOccurrenceTransformDraftDirty)
	{
		m_strElementStatus =
			"Save or Reload the current Effect Detail changes before loading Artist F.";
		return false;
	}

	const std::filesystem::path Path = CProjectDataRoot::Resolve(
		std::filesystem::path(ARTIST_F_SOURCE_AUTHORING_OVERLAY_PATH));
	if (Path.empty())
	{
		m_strElementStatus =
			"Artist F source-backed edit path escaped the project Data root.";
		return false;
	}

	EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged;
	Staged.strEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	Staged.strSourceProgramSha256 = pProgram->Identity.strProgramSha256;
	Staged.SupplementalDocument =
		CEffectSourceAuthoringOverlayCodec::Create_EmptySupplementalDocument(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	std::string Baseline;
	std::string Error;
	std::error_code FileError;
	const bool_t bExists = std::filesystem::is_regular_file(Path, FileError);
	if (FileError)
	{
		m_strElementStatus =
			"Artist F saved overlay path could not be inspected: " +
			FileError.message();
		return false;
	}
	if (bExists)
	{
		if (!CEffectSourceAuthoringOverlayCodec::Load(Path, Staged, Error) ||
			!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error))
		{
			m_strElementStatus = Error.empty() ?
				"Artist F saved overlay could not be loaded." : Error;
			return false;
		}
		Baseline = CEffectSourceAuthoringOverlayCodec::Serialize(Staged);
	}
	else
	{
		const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT>
			pLegacyTuning = pEntry->Get_OccurrenceTuning();
		if (nullptr != pLegacyTuning)
		{
			if (!CEffectSourceAuthoringOverlayCodec::
					Migrate_FromOccurrenceTuning(
						*pProgram, *pLegacyTuning, Staged, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Artist F legacy tuning migration failed." : Error;
				return false;
			}
		}
		else if (!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error))
		{
			m_strElementStatus = Error;
			return false;
		}
	}

	m_OccurrenceTuningDocument.reset();
	m_OccurrenceTuningPath.clear();
	m_strOccurrenceTuningBaselineCanonical.clear();
	m_SourceAuthoringOverlayDocument = std::move(Staged);
	m_SourceAuthoringOverlayPath = Path;
	m_strSourceAuthoringOverlayBaselineCanonical = std::move(Baseline);
	m_bSourceAuthoringOverlayNeedsInitialSave = !bExists;
	m_bOccurrenceTuningDirty = !bExists &&
		!m_SourceAuthoringOverlayDocument->Entries.empty();
	m_bOccurrenceTransformDraftDirty = false;
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyArtistFTrackASeedData(
	const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
	const EFFECT_ELEMENT_DESC& SourceElement,
	EFFECT_ELEMENT_DESC& InOutElement,
	std::string& strOutError) const
{
	EFFECT_GPU_RENDER_FAMILY eSourceFamily = EFFECT_GPU_RENDER_FAMILY::END;
	if (!Emitter.bVisible ||
		!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eSourceFamily))
	{
		strOutError = "Artist F Track A seed is outside the visible Core33 set.";
		return false;
	}
	const EFFECT_AUTHORING_FAMILY eExpectedFamily =
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::MESH ?
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE :
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ?
			EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE :
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ?
			EFFECT_AUTHORING_FAMILY::LOCAL_DECAL :
		eSourceFamily == EFFECT_GPU_RENDER_FAMILY::RIBBON ?
			EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON :
			EFFECT_AUTHORING_FAMILY::END;
	if (Resolve_AuthoringFamily(InOutElement) != eExpectedFamily ||
		SourceElement.strElementId != Emitter.strSourceElementId)
	{
		strOutError =
			"Artist F Track A seed no longer matches its authored Family/Element identity.";
		return false;
	}
	if (!Emitter.strMaterialOccurrenceId.has_value())
	{
		strOutError =
			"Artist F Track A seed has no stable material occurrence ID.";
		return false;
	}
	const auto Registry = Find_Artist31470ShaderRegistry(
		Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId);
	if (!Registry.has_value() ||
		!Validate_Artist31470ShaderRegistryEmitterIdentity(
			Emitter.Row.iOrder, *Emitter.strMaterialOccurrenceId,
			Emitter.strSourceElementId, Emitter.strSourceEmitterPath))
	{
		strOutError =
			"Artist F Track A seed no longer matches the shader registry.";
		return false;
	}

	if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
	{
		uint64_t iFixedBurstCount = 0u;
		for (const EFFECT_RUNTIME_PROGRAM_BURST& Burst : Emitter.Timing.Bursts)
		{
			if (!std::isfinite(Burst.fTimeSeconds) ||
				std::abs(Burst.fTimeSeconds) > 1.0e-9 ||
				Burst.iCountMinimum != Burst.iCountMaximum)
			{
				strOutError =
					"Artist F Track A burst is not a fixed t=0 burst representable by the authored Particle Detail.";
				return false;
			}
			iFixedBurstCount += Burst.iCountMaximum;
		}
		if (iFixedBurstCount > (std::numeric_limits<uint32_t>::max)())
		{
			strOutError = "Artist F Track A fixed burst count overflowed uint32.";
			return false;
		}
		InOutElement.Detail.Particle.iBurstCount =
			static_cast<uint32_t>(iFixedBurstCount);
		InOutElement.Detail.Particle.bLocalSpace = Emitter.bLocalSpace;
		InOutElement.Detail.Particle.iRandomSeed =
			Emitter.Random.iEmitterRandomSeed;
		InOutElement.Detail.Particle.iMaxParticles = (std::max)(
			InOutElement.Detail.Particle.iMaxParticles,
			Emitter.iOperationalMaxParticles);
	}
	if (EFFECT_AUTHORING_FAMILY::MESH_PARTICLE == eExpectedFamily)
	{
		const EFFECT_SOURCE_GEOMETRY_BINDING_DESC& Geometry =
			SourceElement.SourceRecipe.GeometryBinding;
		const auto ModelBinding = std::find_if(
			InOutElement.ResourceBindings.begin(),
			InOutElement.ResourceBindings.end(),
			[](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.strSlotId == EFFECT_MESH_SHAPE_SLOT_ID;
			});
		constexpr f32_t ARTIST_F_MODEL_PRE_SCALE = 0.01f;
		if (!Geometry.bEnabled ||
			ModelBinding == InOutElement.ResourceBindings.end() ||
			ModelBinding->strAssetId != Geometry.strAssetId ||
			!std::isfinite(Geometry.fCarrierGeometryPreScale) ||
			std::abs(Geometry.fCarrierGeometryPreScale -
				ARTIST_F_MODEL_PRE_SCALE) > 1.0e-7f)
		{
			strOutError =
				"Artist F MeshParticle lost its WModel geometry pre-scale 0.01 contract.";
			return false;
		}
		InOutElement.Detail.Mesh.fModelPreScale =
			Geometry.fCarrierGeometryPreScale;
	}

	/* The current unified baseline already flattened root snapshot yaw and the
	   five follow parents into Detail.Transform. Re-enabling either attachment
	   here would apply the Artist import basis/weapon basis a second time. The
	   P0 authored result therefore keeps exactly one emit-start baked basis. */
	InOutElement.ActionCueAttachment = {};
	InOutElement.TransformInheritance = {};
	if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
	{
		uint32_t iRequiredMask = 0u;
		float4_t vSourceStart{};
		float4_t vSourceEnd{};
		switch (Emitter.Row.iOrder)
		{
		case 13u:
		case 14u:
			iRequiredMask = 0x0fu;
			vSourceStart = { 1.f, 0.f, 1.f, 1.f };
			vSourceEnd = { 1.f, 2.f, 1.f, 1.f };
			break;
		case 25u:
			iRequiredMask = 0x06u;
			vSourceStart = { 1.f, 1.f, 0.f, 0.f };
			vSourceEnd = { 1.f, 0.5f, 0.f, 0.f };
			break;
		case 29u:
			iRequiredMask = 0x06u;
			vSourceStart = { 1.f, 1.f, 1.f, 0.f };
			vSourceEnd = { 1.f, 0.5f, 1.f, 0.f };
			break;
		default:
			break;
		}
		f32_t* pStart =
			&InOutElement.Detail.Particle.vDynamicParameterStart.x;
		f32_t* pEnd = &InOutElement.Detail.Particle.vDynamicParameterEnd.x;
		const f32_t* pSourceStart = &vSourceStart.x;
		const f32_t* pSourceEnd = &vSourceEnd.x;
		for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
		{
			const uint32_t iBit = 1u << iComponent;
			if (0u == (iRequiredMask & iBit) ||
				0u != (InOutElement.Detail.Particle.
					iDynamicParameterComponentMask & iBit))
			{
				continue;
			}
			pStart[iComponent] = pSourceStart[iComponent];
			pEnd[iComponent] = pSourceEnd[iComponent];
		}
		InOutElement.Detail.Particle.iDynamicParameterComponentMask |=
			iRequiredMask;
	}

	const auto MaterialSnapshot = m_ArtistFMaterialExecutionSnapshots.find(
		SourceElement.strElementId);
	if (MaterialSnapshot == m_ArtistFMaterialExecutionSnapshots.end())
	{
		strOutError =
			"Artist F Track A material snapshot no longer matches its source Element identity.";
		return false;
	}
	if (MaterialSnapshot->second.bEnabled)
	{
		if ((Registry->eBackend !=
				EFFECT_ARTIST31470_SHADER_BACKEND::RUNTIME_V2 &&
			 Registry->eBackend !=
				EFFECT_ARTIST31470_SHADER_BACKEND::ARTIST_V4) ||
			!Registry->bDrawAdmitted)
		{
			strOutError =
				"Artist F typed Material snapshot disagrees with the shader registry.";
			return false;
		}
		EFFECT_MATERIAL_EXECUTION_DESC StagedExecution =
			MaterialSnapshot->second;
		/* Re-running Upgrade refreshes the Track A recipe, but an artist's DDS
		   override remains authoritative when its semantic lane identity still
		   exists.  Generic ResourceBindings are independent and stay untouched. */
		if (InOutElement.Material.Execution.bEnabled)
		{
			for (EFFECT_MATERIAL_TEXTURE_LANE_DESC& StagedLane :
				StagedExecution.TextureLanes)
			{
				const auto ExistingLane = std::find_if(
					InOutElement.Material.Execution.TextureLanes.begin(),
					InOutElement.Material.Execution.TextureLanes.end(),
					[&StagedLane](
						const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Candidate)
					{ return Candidate.strLaneId == StagedLane.strLaneId; });
				if (ExistingLane !=
						InOutElement.Material.Execution.TextureLanes.end() &&
					!ExistingLane->strAssetId.empty())
				{
					StagedLane.strAssetId = ExistingLane->strAssetId;
				}
			}
		}
		InOutElement.Material.Execution = std::move(StagedExecution);
		/* The typed snapshot owns t#/s#/channel execution.  Keep the source path
		   as provenance, but never execute the older SourceMaterial profile in
		   parallel. */
		InOutElement.Material.SourceMaterial = {};
		if (EFFECT_ELEMENT_KIND::PARTICLE == InOutElement.eKind)
		{
			const uint32_t iConsumedMask =
				InOutElement.Material.Execution.iDynamicConsumedMask & 0x0fu;
			f32_t* pStart =
				&InOutElement.Detail.Particle.vDynamicParameterStart.x;
			f32_t* pEnd =
				&InOutElement.Detail.Particle.vDynamicParameterEnd.x;
			for (uint32_t iComponent = 0u; iComponent < 4u; ++iComponent)
			{
				const uint32_t iBit = 1u << iComponent;
				if (0u == (iConsumedMask & iBit) ||
					0u != (InOutElement.Detail.Particle.
						iDynamicParameterComponentMask & iBit))
				{
					continue;
				}
				/* Track A has no action-cue DynamicParameter sample for these
				   rows. UE's missing-payload carrier is constant one; seed it once
				   and preserve any later artist-authored component values. */
				pStart[iComponent] = 1.f;
				pEnd[iComponent] = 1.f;
			}
			InOutElement.Detail.Particle.iDynamicParameterComponentMask |=
				iConsumedMask;
		}
	}
	else if (Registry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::FINITE_COMMON &&
		Registry->eFidelity ==
			EFFECT_ARTIST31470_SHADER_FIDELITY::BOUNDED_EXPLICIT &&
		Registry->bDrawAdmitted && Emitter.Row.iOrder == 17u)
	{
		/* #17 is the one valid FiniteCommon row. Unlike the 28 typed snapshots,
		   its bounded missile-trail evaluator is the ordinary SourceMaterial
		   profile 13. Copy that self-contained authored recipe instead of
		   degrading it to generic profile 0, while preserving same-name DDS
		   overrides already made in the editor. */
		const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
			SourceElement.Material.SourceMaterial;
		if (!SourceMaterial.bEnabled ||
			SourceMaterial.strRuntimeShaderProfileId !=
				"effect.ue3.missiletrail-01.v1")
		{
			strOutError =
				"Artist F #17 lost its bounded FiniteCommon missile-trail profile.";
			return false;
		}
		EFFECT_MATERIAL_DESC StagedMaterial = SourceElement.Material;
		StagedMaterial.Execution = {};
		for (EFFECT_NAMED_TEXTURE_DESC& StagedTexture :
			StagedMaterial.SourceMaterial.Textures)
		{
			const auto ExistingTexture = std::find_if(
				InOutElement.Material.SourceMaterial.Textures.begin(),
				InOutElement.Material.SourceMaterial.Textures.end(),
				[&StagedTexture](const EFFECT_NAMED_TEXTURE_DESC& Candidate)
				{
					return Candidate.strName == StagedTexture.strName;
				});
			if (ExistingTexture !=
					InOutElement.Material.SourceMaterial.Textures.end() &&
				!ExistingTexture->strAssetId.empty())
			{
				StagedTexture.strAssetId = ExistingTexture->strAssetId;
			}
		}
		InOutElement.Material = std::move(StagedMaterial);
	}
	else if (Registry->eBackend ==
			EFFECT_ARTIST31470_SHADER_BACKEND::NONE &&
		Registry->eFidelity ==
			EFFECT_ARTIST31470_SHADER_FIDELITY::UNRESOLVED_FAIL_CLOSED &&
		!Registry->bDrawAdmitted &&
		(Emitter.Row.iOrder == 1u || Emitter.Row.iOrder == 16u ||
		 Emitter.Row.iOrder == 26u || Emitter.Row.iOrder == 33u))
	{
		/* SceneColor/depth/fog/aux-MRT owners must not silently become a generic
		   white or distortion draw. Keep their seed resources for later manual
		   reconstruction, but make the automatic Track A import fail closed. */
		InOutElement.Material.Execution = {};
		InOutElement.Material.Execution.bFailClosed = true;
		InOutElement.Material.SourceMaterial = {};
		InOutElement.bVisible = false;
	}
	else
	{
		strOutError =
			"Artist F disabled Material snapshot has no admitted FiniteCommon/fail-closed policy.";
		return false;
	}
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Try_CreateArtistFUnifiedDraft()
{
	if (Has_UnsavedWork())
	{
		m_strElementStatus =
			"Save or discard the current Effect changes before creating the Artist F Unified draft.";
		return false;
	}
	const std::filesystem::path Path = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored" /
		(std::filesystem::path(ARTIST_F_UNIFIED_EFFECT_ASSET_ID).wstring() +
		 L".effect.json"));
	if (Path.empty())
	{
		m_strElementStatus =
			"Artist F Unified draft path escaped Data/Effects/Authored.";
		return false;
	}
	if (std::filesystem::is_regular_file(Path))
	{
		m_strElementStatus =
			"Artist F Unified Effect already exists. Use Load Effect; migration never overwrites it.";
		return false;
	}
	if (!Ensure_ArtistFSourceSnapshotForAuthoring() ||
		!Ensure_ArtistFMaterialExecutionSnapshots() ||
		nullptr == m_pArtistFSourcePreparation ||
		nullptr == m_pArtistFSourceProjection ||
		nullptr == m_pArtistFSourcePreparation->Get_Program())
	{
		m_strElementStatus = m_strArtistFSourceSnapshotStatus.empty() ?
			"Artist F source recipe is unavailable." :
			m_strArtistFSourceSnapshotStatus;
		return false;
	}

	const EFFECT_DOCUMENT_DESC& SourceDocument =
		m_pArtistFSourceProjection->Get_Document();
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		m_pArtistFSourcePreparation->Get_Program();
	struct PENDING_ARTIST_F_UNIFIED_ELEMENT final
	{
		EFFECT_ELEMENT_DESC LoweredElement;
		EFFECT_GENERIC_AUTHORED_STARTING_BAKE_REQUEST BakeRequest;
		std::string strSourceElementId;
		std::string strFollowAnchorSlotId;
	};
	std::array<size_t, 4u> FamilyOrdinals{};
	std::set<std::string, std::less<>> TargetIds;
	std::vector<PENDING_ARTIST_F_UNIFIED_ELEMENT> PendingElements;
	PendingElements.reserve(33u);
	size_t iFollowElementCount = 0u;
	size_t iStrictPresetElementCount = 0u;
	std::string strFirstSourceElementId;
	std::string Error;
	for (const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter : pProgram->Emitters)
	{
		EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
		if (!Emitter.bVisible ||
			!Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily))
		{
			continue;
		}
		const auto Source = std::find_if(
			SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
			[&Emitter](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Emitter.strSourceElementId; });
		if (Source == SourceDocument.Elements.end() || !Source->bVisible)
		{
			m_strElementStatus =
				"Artist F source Element disappeared before starting-state capture: " +
				Emitter.strSourceElementId;
			return false;
		}

		PENDING_ARTIST_F_UNIFIED_ELEMENT Pending;
		Pending.strSourceElementId = Emitter.strSourceElementId;
		if (!Try_NarrowRuntimeFloat3(
				Emitter.CueLocalTransform.vPosition,
				Pending.BakeRequest.CueLocalTransform.vPosition) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.CueLocalTransform.vRotationDegrees,
				Pending.BakeRequest.CueLocalTransform.vRotationDegrees) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.CueLocalTransform.vScale,
				Pending.BakeRequest.CueLocalTransform.vScale) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.DetailTransform.vPosition,
				Pending.BakeRequest.EmitterLocalTransform.vPosition) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.DetailTransform.vRotationDegrees,
				Pending.BakeRequest.EmitterLocalTransform.vRotationDegrees) ||
			!Try_NarrowRuntimeFloat3(
				Emitter.DetailTransform.vScale,
				Pending.BakeRequest.EmitterLocalTransform.vScale) ||
			(Emitter.RendererRuntimeConfig.Mesh.has_value() &&
			 !Try_NarrowRuntimeFloat3(
				 Emitter.RendererRuntimeConfig.Mesh->
					 vSourceTypeDataRotationDegrees,
				 Pending.BakeRequest.vSourceTypeDataRotationDegrees)))
		{
			m_strElementStatus =
				"Artist F runtime emitter transform is not representable: " +
				Emitter.strSourceElementId;
			return false;
		}
		Pending.BakeRequest.fScheduleStartDelaySeconds =
			Source->Detail.Timing.fStartDelaySeconds;
		Pending.BakeRequest.fScheduleLifeTimeSeconds =
			Source->Detail.Timing.fLifeTimeSeconds;
		Pending.BakeRequest.fEmitterDelaySeconds =
			Source->SourceRecipe.fEmitterDelaySeconds;
		Pending.BakeRequest.fEmitterDurationSeconds =
			Source->SourceRecipe.fEmitterDurationSeconds;
		Pending.BakeRequest.iEmitterLoopCount =
			Source->SourceRecipe.iEmitterLoopCount;
		Pending.BakeRequest.bAttachmentEnabled =
			Source->ActionCueAttachment.bEnabled;
		Pending.BakeRequest.bFollowAttachment =
			Source->ActionCueAttachment.bFollow;
		Pending.BakeRequest.fSnapshotRootSourceBasisYawDegrees =
			Source->ActionCueAttachment.fSnapshotRootSourceBasisYawDegrees;
		Pending.BakeRequest.bTransformInheritanceEnabled =
			Source->TransformInheritance.bEnabled;
		if (Pending.BakeRequest.bFollowAttachment)
		{
			const auto Binding = std::find_if(
				m_pArtistFSourcePreparation->Get_AnchorRequests().begin(),
				m_pArtistFSourcePreparation->Get_AnchorRequests().end(),
				[&Emitter](const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Candidate)
				{ return Candidate.strOwnerEmitterId == Emitter.Row.strId; });
			if (!Pending.BakeRequest.bAttachmentEnabled ||
				Source->ActionCueAttachment.strRuntimeAnchorSlotId.empty() ||
				Binding ==
					m_pArtistFSourcePreparation->Get_AnchorRequests().end() ||
				!Binding->Request.bFollow ||
				Binding->Request.strRuntimeAnchorSlotId !=
					Source->ActionCueAttachment.strRuntimeAnchorSlotId ||
				Binding->Request.strRuntimeBoneName !=
					Source->ActionCueAttachment.strRuntimeBoneName)
			{
				m_strElementStatus =
					"Artist F follow Element has no exact typed anchor binding: " +
					Emitter.strSourceElementId;
				return false;
			}
			Pending.strFollowAnchorSlotId =
				Binding->Request.strRuntimeAnchorSlotId;
			++iFollowElementCount;
		}

		EFFECT_DOCUMENT_DESC OneElement;
		const std::string strStrictOccurrenceId =
			Emitter.strMaterialOccurrenceId.value_or(std::string{});
		const EFFECT_VISUAL_PROGRAM_ROW* pStrictVisualRow =
			strStrictOccurrenceId.empty() ? nullptr :
			m_pArtistFSourceProjection->Find_RowByOccurrenceId(
				strStrictOccurrenceId);
		const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pStrictSupplemental =
			strStrictOccurrenceId.empty() ? nullptr :
			m_pArtistFSourceProjection->
				Find_SupplementalElementByOccurrenceId(strStrictOccurrenceId);
		const bool_t bRequiresStrictPreset =
			strStrictOccurrenceId == "source-active-003" ||
			strStrictOccurrenceId == "source-active-020" ||
			strStrictOccurrenceId == "source-active-021";
		if (bRequiresStrictPreset && nullptr == pStrictVisualRow &&
			nullptr == pStrictSupplemental)
		{
			m_strElementStatus =
				"Artist F typed Decal/Ribbon source row lost its admitted preset identity.";
			return false;
		}
		if (nullptr != pStrictVisualRow && nullptr != pStrictSupplemental)
		{
			m_strElementStatus =
				"Artist F source Element matched more than one admitted row.";
			return false;
		}
		if (nullptr == pStrictVisualRow && nullptr == pStrictSupplemental)
		{
			if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
					SourceDocument, Emitter.strSourceElementId,
					ARTIST_F_UNIFIED_EFFECT_ASSET_ID, OneElement, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Artist F source Element could not be lowered." : Error;
				return false;
			}
		}
		else if (m_bActiveDocumentDrawable)
		{
			m_strDetailStatus =
				"Applied and saved Authored; the preview is ready but hidden. Use Play All or Restart Preview.";
		}
		else
		{
			if (nullptr != pStrictVisualRow &&
				!pStrictVisualRow->TargetIdentity.has_value())
			{
				m_strElementStatus =
					"Artist F admitted source Element has no target identity.";
				return false;
			}
			EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST Request;
			Request.strEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
			Request.strOccurrenceId = strStrictOccurrenceId;
			Request.strRowSha256 = nullptr != pStrictVisualRow ?
				pStrictVisualRow->strRowSha256 : pStrictSupplemental->strRowSha256;
			Request.strTargetElementId = nullptr != pStrictVisualRow ?
				pStrictVisualRow->TargetIdentity->strTargetElementId :
				pStrictSupplemental->TargetIdentity.strTargetElementId;
			Request.strSourceRecordId = nullptr != pStrictVisualRow ?
				pStrictVisualRow->SourceIdentity.strSourceRecordId :
				pStrictSupplemental->strSourceRecordId;
			if (Request.strTargetElementId != Emitter.strSourceElementId)
			{
				m_strElementStatus =
					"Artist F admitted source row target no longer matches its Core Element.";
				return false;
			}
			EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE Stage;
			if (!CEffectVisualProgramCorpusCodec::Build_ElementAuthoringPresetStage(
					m_pArtistFSourceProjection, Request, Stage, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Artist F admitted source Element could not be staged." : Error;
				return false;
			}
			OneElement.strEffectAssetId = ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
			OneElement.strDisplayName = "Artist F Unified starting Element";
			OneElement.Elements.push_back(std::move(Stage.Element));
			++iStrictPresetElementCount;
		}
		if (OneElement.Elements.size() != 1u)
		{
			m_strElementStatus = Error.empty() ?
				"Artist F source Element could not be lowered." : Error;
			return false;
		}
		Pending.LoweredElement = std::move(OneElement.Elements.front());
		const std::string StableId = StableUnifiedElementId(
			eFamily, Emitter.Row.strId);
		if (StableId.empty() || !TargetIds.insert(StableId).second)
		{
			m_strElementStatus =
				"Artist F Unified Element stable-ID collision was rejected.";
			return false;
		}
		const size_t iFamily = eFamily == EFFECT_GPU_RENDER_FAMILY::MESH ? 0u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ? 1u :
			eFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ? 2u : 3u;
		const size_t iOrdinal = ++FamilyOrdinals[iFamily];
		Pending.LoweredElement.strElementId = StableId;
		Pending.LoweredElement.strGroupId = std::string("family.") +
			(eFamily == EFFECT_GPU_RENDER_FAMILY::MESH ? "mesh" :
			 eFamily == EFFECT_GPU_RENDER_FAMILY::SPRITE ? "sprite" :
			 eFamily == EFFECT_GPU_RENDER_FAMILY::DECAL ? "decal" : "ribbon");
		Pending.LoweredElement.strDisplayName =
			std::string(ArtistCoreFamilyLabel(eFamily)) +
			" " + (iOrdinal < 10u ? "0" : "") +
			std::to_string(iOrdinal) + " | " +
			PrimaryAuthoringResourceLeaf(Pending.LoweredElement);
		if (PendingElements.empty())
			strFirstSourceElementId = Emitter.strSourceElementId;
		PendingElements.push_back(std::move(Pending));
	}
	if (PendingElements.size() != 33u || iFollowElementCount != 5u ||
		iStrictPresetElementCount != 3u ||
		FamilyOrdinals != std::array<size_t, 4u>{ 13u, 16u, 3u, 1u })
	{
		m_strElementStatus =
			"Artist F Unified draft requires Mesh 13, Sprite 16, Decal 3, Ribbon 1, including five exact follow starts.";
		return false;
	}
	if (!Synchronize_Artist31470FullPreview(m_pArtistFSourcePreparation))
	{
		m_strElementStatus =
			"Artist F Unified starting-state animation preparation failed: " +
			m_strPreviewAnimationStatus;
		return false;
	}
	CAnimationHistoricalPoseBinding PoseBinding;
	f32_t fAnimationDurationSeconds = 0.f;
	if (!Prepare_Artist31470HistoricalPoseBinding(
			m_pArtistFSourcePreparation, PoseBinding,
			fAnimationDurationSeconds, Error))
	{
		m_strElementStatus =
			"Artist F Unified historical pose binding failed: " + Error;
		return false;
	}

	std::vector<EFFECT_ELEMENT_DESC> BakedElements;
	BakedElements.reserve(PendingElements.size());
	for (PENDING_ARTIST_F_UNIFIED_ELEMENT& Pending : PendingElements)
	{
		if (Pending.BakeRequest.bFollowAttachment)
		{
			const f32_t fEmitStartSeconds =
				Pending.BakeRequest.fScheduleStartDelaySeconds +
				Pending.BakeRequest.fEmitterDelaySeconds;
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Sample;
			if (!Build_Artist31470HistoricalTransformSample(
					m_pArtistFSourcePreparation, PoseBinding,
					fAnimationDurationSeconds, fEmitStartSeconds,
					Sample, Error))
			{
				m_strElementStatus =
					"Artist F Unified follow start sampling failed for " +
					Pending.strSourceElementId + ": " + Error;
				return false;
			}
			const auto Anchor = Sample.SourceAnchorWorlds.find(
				Pending.strFollowAnchorSlotId);
			if (Anchor == Sample.SourceAnchorWorlds.end())
			{
				m_strElementStatus =
					"Artist F Unified follow start lost its exact anchor: " +
					Pending.strSourceElementId;
				return false;
			}
			const matrix_t RootWorld = XMLoadFloat4x4(&Sample.RootWorld);
			vector_t vRootDeterminant = XMMatrixDeterminant(RootWorld);
			const f32_t fRootDeterminant = XMVectorGetX(vRootDeterminant);
			if (!std::isfinite(fRootDeterminant) ||
				std::abs(fRootDeterminant) <= 1.0e-8f)
			{
				m_strElementStatus =
					"Artist F Unified follow start has a singular root: " +
					Pending.strSourceElementId;
				return false;
			}
			const matrix_t ParentLocal =
				XMLoadFloat4x4(&Anchor->second) *
				XMMatrixInverse(&vRootDeterminant, RootWorld);
			XMStoreFloat4x4(
				&Pending.BakeRequest.FollowParentLocalTransform, ParentLocal);
			Pending.BakeRequest.bHasFollowParentLocalTransform = true;
		}

		EFFECT_ELEMENT_DESC BakedElement;
		if (!CEffectDocumentCodec::Bake_GenericAuthoredElementStartingState(
				Pending.LoweredElement, Pending.BakeRequest,
				BakedElement, Error))
		{
			m_strElementStatus =
				"Artist F Unified starting-state bake failed for " +
				Pending.strSourceElementId + ": " + Error;
			return false;
		}
		const auto ProgramEmitter = std::find_if(
			pProgram->Emitters.begin(), pProgram->Emitters.end(),
			[&Pending](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
			{ return Candidate.strSourceElementId == Pending.strSourceElementId; });
		const auto SourceElement = std::find_if(
			SourceDocument.Elements.begin(), SourceDocument.Elements.end(),
			[&Pending](const EFFECT_ELEMENT_DESC& Candidate)
			{ return Candidate.strElementId == Pending.strSourceElementId; });
		if (ProgramEmitter == pProgram->Emitters.end() ||
			SourceElement == SourceDocument.Elements.end() ||
			!Try_ApplyArtistFTrackASeedData(
				*ProgramEmitter, *SourceElement, BakedElement, Error))
		{
			m_strElementStatus = Error.empty() ?
				"Artist F Unified Track A particle/attachment seed could not be applied." :
				Error;
			return false;
		}
		BakedElements.push_back(std::move(BakedElement));
	}

	EFFECT_DOCUMENT_DESC Candidate;
	if (!CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy(
			SourceDocument, strFirstSourceElementId,
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID, Candidate, Error))
	{
		m_strElementStatus = Error;
		return false;
	}
	Candidate.strEffectAssetId = ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
	Candidate.strDisplayName = "Artist F Unified Effect";
	Candidate.ModelCues.clear();
	Candidate.Elements.assign(1u, BakedElements.front());
	std::vector<EFFECT_ELEMENT_DESC> Remaining(
		std::next(BakedElements.begin()), BakedElements.end());
	EFFECT_DOCUMENT_DESC Merged;
	if (!CEffectDocumentCodec::Merge_GenericAuthoredElements(
			Candidate, Remaining, Merged, Error) ||
		Merged.Elements.size() != 33u ||
		!CEffectDocumentCodec::Validate_Drawable(Merged, Error))
	{
		m_strElementStatus = Error.empty() ?
			"Artist F Unified draft merge failed." : Error;
		return false;
	}

	const bool_t bPreviousPreviewIsSource =
		m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value();
	const optional<EFFECT_DOCUMENT_DESC> PreviousPreview =
		bPreviousPreviewIsSource ?
			m_SourcePreviewDocument : m_ActiveDocument;
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_fPreviewTimeSeconds = 0.f;
	if (!Stage_WorldPreview(Merged))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		m_strElementStatus =
			"Artist F Unified draft preview preflight failed: " +
			m_strPreviewStatus;
		return false;
	}
	if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
			Path, Merged, std::string_view{}, Error))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		if (PreviousPreview.has_value())
			Stage_WorldPreview(*PreviousPreview, bPreviousPreviewIsSource);
		else
			Hide_WorldPreview();
		m_strElementStatus = Error.empty() ?
			"Artist F Unified draft could not be saved." : Error;
		return false;
	}
	if (!Try_LoadDocumentPathStaged(Path, EFFECT_DOCUMENT_SOURCE::AUTHORED,
			ARTIST_F_UNIFIED_EFFECT_ASSET_ID, true))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		if (PreviousPreview.has_value())
			Stage_WorldPreview(*PreviousPreview, bPreviousPreviewIsSource);
		else
			Hide_WorldPreview();
		m_strElementStatus =
			"Artist F Unified draft was saved, but could not be opened. Use Load Effect to retry.";
		return false;
	}
	m_SourceElementPresetSelection.reset();
	m_strElementStatus =
		"Created Artist F Unified draft with 33 editable Elements, 28 typed Track A Material recipes, fixed particle bursts/local-space, editable constant-one DynamicParameter fallbacks, root snapshots, and five exact emit-start follow poses. Five unsupported Material rows remain explicit bounded generic starters; product mapping was unchanged.";
	return true;
}

bool_t Client::CEffect_Tool::Try_CreateDimensionMasterTUnifiedDraft()
{
	if (Has_UnsavedWork())
	{
		m_strElementStatus =
			"Save or discard the current Effect changes before creating the DimensionMaster T Unified draft.";
		return false;
	}
	const std::filesystem::path AuthoredRoot = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored");
	const std::filesystem::path BaselinePath = AuthoredRoot /
		(std::filesystem::path(DIMENSION_MASTER_T_BASELINE_EFFECT_ASSET_ID).wstring() +
		 L".effect.json");
	const std::filesystem::path SourcePath = AuthoredRoot /
		(std::filesystem::path(DIMENSION_MASTER_T_SOURCE_EFFECT_ASSET_ID).wstring() +
		 L".effect.json");
	const std::filesystem::path Path = AuthoredRoot /
		(std::filesystem::path(DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID).wstring() +
		 L".effect.json");
	if (AuthoredRoot.empty() || std::filesystem::is_regular_file(Path))
	{
		m_strElementStatus = AuthoredRoot.empty() ?
			"DimensionMaster T Unified path escaped Data/Effects/Authored." :
			"DimensionMaster T Unified draft already exists. Use Load Unified Effect.";
		return false;
	}

	EFFECT_DOCUMENT_DESC Baseline;
	EFFECT_DOCUMENT_DESC Source;
	std::string Error;
	if (!CEffectDocumentCodec::Load(BaselinePath, Baseline, Error) ||
		!CEffectDocumentCodec::Validate_Drawable(Baseline, Error) ||
		Baseline.strEffectAssetId !=
			DIMENSION_MASTER_T_BASELINE_EFFECT_ASSET_ID ||
		Baseline.Elements.size() != 35u || !Baseline.ModelCues.empty())
	{
		m_strElementStatus = Error.empty() ?
			"DimensionMaster T baseline must contain exactly 35 drawable Elements and no Model Cue." : Error;
		return false;
	}
	if (!CEffectDocumentCodec::Load(SourcePath, Source, Error) ||
		Source.strEffectAssetId != DIMENSION_MASTER_T_SOURCE_EFFECT_ASSET_ID ||
		Source.ModelCues.size() != 1u ||
		Source.ModelCues.front().strCueId != "dimension_summon" ||
		Source.ModelCues.front().strModelAssetId !=
			"Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel" ||
		Source.ModelCues.front().strClipName !=
			"sk_swp_dms_00_sk_sk_dimensionprison")
	{
		m_strElementStatus = Error.empty() ?
			"DimensionMaster T source must resolve the one exact dimension_summon Model Cue." : Error;
		return false;
	}

	EFFECT_DOCUMENT_DESC Candidate = Baseline;
	Candidate.strEffectAssetId = DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID;
	Candidate.strDisplayName = "DimensionMaster T Unified Effect";
	Candidate.ModelCues = Source.ModelCues;
	const std::string Canonical = CEffectDocumentCodec::Serialize(Candidate);
	EFFECT_DOCUMENT_DESC RoundTripped;
	if (!CEffectDocumentCodec::Parse(Canonical, RoundTripped, Error) ||
		!CEffectDocumentCodec::Validate_Drawable(RoundTripped, Error) ||
		RoundTripped.Elements.size() != 35u ||
		RoundTripped.ModelCues.size() != 1u ||
		CEffectDocumentCodec::Serialize(RoundTripped) != Canonical)
	{
		m_strElementStatus = Error.empty() ?
			"DimensionMaster T Unified draft failed canonical validation." : Error;
		return false;
	}

	const bool_t bPreviousPreviewIsSource =
		m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value();
	const optional<EFFECT_DOCUMENT_DESC> PreviousPreview =
		bPreviousPreviewIsSource ?
			m_SourcePreviewDocument : m_ActiveDocument;
	const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_fPreviewTimeSeconds = 0.f;
	if (!Stage_WorldPreview(RoundTripped))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		m_strElementStatus =
			"DimensionMaster T Unified preview preflight failed: " +
			m_strPreviewStatus;
		return false;
	}
	if (!CEffectDocumentCodec::Save_AtomicIfUnchanged(
		Path, RoundTripped, std::string_view{}, Error))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		if (PreviousPreview.has_value())
			Stage_WorldPreview(*PreviousPreview, bPreviousPreviewIsSource);
		else
			Hide_WorldPreview();
		m_strElementStatus = Error.empty() ?
			"DimensionMaster T Unified draft could not be saved." : Error;
		return false;
	}
	if (!Try_LoadDocumentPathStaged(Path, EFFECT_DOCUMENT_SOURCE::AUTHORED,
		DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID, true))
	{
		m_ePreviewFilter = ePreviousFilter;
		m_fPreviewTimeSeconds = fPreviousTime;
		if (PreviousPreview.has_value())
			Stage_WorldPreview(*PreviousPreview, bPreviousPreviewIsSource);
		else
			Hide_WorldPreview();
		m_strElementStatus =
			"DimensionMaster T Unified draft was saved, but could not be opened. Use Load Unified Effect to retry.";
		return false;
	}
	m_strElementStatus =
		"Created DimensionMaster T Unified draft with 35 editable Elements and one editable Dimension Summon. Product mapping was unchanged.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectRuntimeOccurrence(
	const std::string& strEffectAssetId,
	const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter)
{
	const bool_t bChangesSelection =
		EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE != m_eDetailSelection ||
		m_strSelectedRuntimeOccurrenceEffectId != strEffectAssetId ||
		m_strSelectedRuntimeOccurrenceId != Emitter.Row.strId;
	if (!bChangesSelection)
		return true;
	if (m_bDocumentDirty || m_bParticleSystemDraftDirty ||
		m_bDetailDraftDirty || m_bModelCueDraftDirty)
	{
		m_strElementStatus =
			"Save or discard the active authored Effect work before opening runtime occurrence tuning.";
		return false;
	}
	if (m_bOccurrenceTransformDraftDirty)
	{
		m_strElementStatus =
			"Apply or Revert the occurrence Transform draft before selecting another occurrence.";
		return false;
	}
	if ((m_OccurrenceTuningDocument.has_value() ||
		 m_SourceAuthoringOverlayDocument.has_value()) &&
		m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save or Reload the current Effect Detail changes before selecting another Effect.";
		return false;
	}

	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(strEffectAssetId);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == pEntry ? nullptr : pEntry->Get_Program();
	if (nullptr == pEntry || nullptr == pProgram)
	{
		m_strElementStatus =
			"The selected occurrence has no admitted reconstructed Program entry.";
		return false;
	}
	const auto ProgramEmitter = std::find_if(
		pProgram->Emitters.begin(), pProgram->Emitters.end(),
		[&Emitter](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
		{
			return Candidate.Row.strId == Emitter.Row.strId;
		});
	if (ProgramEmitter == pProgram->Emitters.end() ||
		ProgramEmitter->Row.strRowSha256 != Emitter.Row.strRowSha256 ||
		ProgramEmitter->strSourceElementId != Emitter.strSourceElementId ||
		ProgramEmitter->strSourceEmitterPath != Emitter.strSourceEmitterPath)
	{
		m_strElementStatus =
			"The selected occurrence identity changed after the All Effects tree was built.";
		return false;
	}
	if (!Ensure_ArtistFSourceSnapshotForAuthoring() ||
		!Ensure_ArtistFSourceAuthoringOverlaySession())
		return false;
	if (nullptr == m_pArtistFSourceProjection)
	{
		m_strElementStatus =
			"Artist F source projection is unavailable while opening Effect Detail.";
		return false;
	}

	const auto ProjectedElement = std::find_if(
		m_pArtistFSourceProjection->Get_Document().Elements.begin(),
		m_pArtistFSourceProjection->Get_Document().Elements.end(),
		[&ProgramEmitter](const EFFECT_ELEMENT_DESC& Candidate)
		{
			return Candidate.strElementId == ProgramEmitter->strSourceElementId;
		});
	if (ProjectedElement ==
			m_pArtistFSourceProjection->Get_Document().Elements.end() ||
		!ProjectedElement->bVisible)
	{
		m_strElementStatus =
			"Artist F projected source Element is unavailable while opening Effect Detail.";
		return false;
	}

	EFFECT_OCCURRENCE_LOCAL_TRANSFORM CueLocalSourceTransform{};
	if (!Try_NarrowRuntimeFloat3(
			ProgramEmitter->CueLocalTransform.vPosition,
			CueLocalSourceTransform.vPosition) ||
		!Try_NarrowRuntimeFloat3(
			ProgramEmitter->CueLocalTransform.vRotationDegrees,
			CueLocalSourceTransform.vRotationDegrees) ||
		!Try_NarrowRuntimeFloat3(
			ProgramEmitter->CueLocalTransform.vScale,
			CueLocalSourceTransform.vScale))
	{
		m_strElementStatus =
			"Artist F source-local Transform is non-finite while opening Effect Detail.";
		return false;
	}

	if (!m_SourceAuthoringOverlayDocument.has_value() ||
		m_SourceAuthoringOverlayDocument->strEffectAssetId != strEffectAssetId)
	{
		m_strElementStatus =
			"Artist F source-backed edit session changed while opening the Element.";
		return false;
	}
	const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY* pTuned =
		CEffectSourceAuthoringOverlayCodec::Find_Entry(
			*m_SourceAuthoringOverlayDocument, Emitter.Row.strId);
	const EFFECT_OCCURRENCE_LOCAL_TRANSFORM StagedDraft = nullptr == pTuned ?
		CueLocalSourceTransform : pTuned->EffectiveLocalTransform;
	m_OccurrenceTransformDraft = StagedDraft;
	m_SelectedOccurrenceSourceTransform = CueLocalSourceTransform;
	m_pSelectedVisualSourceProjection.reset();
	m_strSelectedRuntimeOccurrenceEffectId = strEffectAssetId;
	m_strSelectedRuntimeOccurrenceId = Emitter.Row.strId;
	m_strSelectedRuntimeOccurrenceRowSha256 = Emitter.Row.strRowSha256;
	m_strSelectedRuntimeOccurrenceElementId = Emitter.strSourceElementId;
	m_strSelectedRuntimeOccurrenceEmitterPath = Emitter.strSourceEmitterPath;
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectSourceAuthoringOverlayCodec::Serialize(
			*m_SourceAuthoringOverlayDocument) !=
		m_strSourceAuthoringOverlayBaselineCanonical;
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE;
	m_strElementStatus =
		"Loaded one Artist F Element into Effect Detail.";
	m_strDetailStatus =
		"The Track A source renderer, material, DDS roles, attachment, and distributions remain active; Position, Rotation, and Scale are editable here.";
	return true;
}

std::shared_ptr<const Client::EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
Client::CEffect_Tool::Resolve_VisualProgramProjectionForAuthoring(
	const std::string& strEffectAssetId)
{
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM> pProgram =
		CEffectCatalog::Find_VisualProgram(strEffectAssetId);
	if (nullptr == pProgram)
		return nullptr;

	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = CEffectCatalog::Find_VisualProjection(strEffectAssetId);
	if (strEffectAssetId == ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
		pProgram->eProjectionKind ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1 &&
		Ensure_ArtistFSourceSnapshotForAuthoring())
	{
		pProjection = m_pArtistFSourceProjection;
	}
	if (nullptr == pProjection)
	{
		if (pProgram->eProjectionKind !=
				EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1)
		{
			return nullptr;
		}
		if (strEffectAssetId == ARTIST_F_VISUAL_PROGRAM_ASSET_ID &&
			Ensure_ArtistFSourceSnapshotForAuthoring())
		{
			pProjection = m_pArtistFSourceProjection;
		}
		if (nullptr != pProjection)
		{
			// CPU-only immutable source snapshot; Play/Solo still owns GPU staging.
		}
		else if (!m_bReconstructedSourceRuntimeActive)
		{
			return nullptr;
		}
		else
		{
		const shared_ptr<CEffectObject> pObject =
			m_pWorldPreviewObject.lock();
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pRuntimeProgram = nullptr == pObject ? nullptr :
				pObject->Get_ReconstructedRuntimeProgram();
		if (nullptr == pObject ||
			!pObject->Is_ReconstructedSourceRuntimeActive() ||
			!pObject->Is_SourceVisualProgramActive() ||
			nullptr == pRuntimeProgram ||
			pRuntimeProgram->strRuntimeCatalogAssetId != strEffectAssetId)
		{
			return nullptr;
		}
		pProjection = pObject->Get_SourceVisualProgramProjection();
		}
	}
	if (nullptr == pProjection || !pProjection->Is_Valid() ||
		pProjection->Get_EffectAssetId() != strEffectAssetId ||
		pProjection->Get_ProjectionKind() != pProgram->eProjectionKind ||
		pProjection->Get_ProgramSha256() != pProgram->strProgramSha256)
	{
		return nullptr;
	}
	return pProjection;
}

bool_t Client::CEffect_Tool::Try_OpenVisualProgramElementForAuthoring(
	const std::string& strEffectAssetId,
	const std::string& strOccurrenceId,
	const std::string& strRowSha256,
	const std::string& strTargetElementId,
	const std::string& strSourceRecordId)
{
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection =
			Resolve_VisualProgramProjectionForAuthoring(strEffectAssetId);
	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_REQUEST Request;
	Request.strEffectAssetId = strEffectAssetId;
	Request.strOccurrenceId = strOccurrenceId;
	Request.strRowSha256 = strRowSha256;
	Request.strTargetElementId = strTargetElementId;
	Request.strSourceRecordId = strSourceRecordId;
	EFFECT_VISUAL_PROGRAM_ELEMENT_PRESET_STAGE Stage;
	std::string Error;
	if (!CEffectVisualProgramCorpusCodec::Build_ElementAuthoringPresetStage(
			pProjection, Request, Stage, Error))
	{
		m_strElementStatus = Error.empty() ?
			"The selected Element is no longer available." : Error;
		return false;
	}

	SOURCE_ELEMENT_PRESET_SELECTION Selection;
	Selection.pProjection = Stage.pProjection;
	Selection.GenericElement = std::move(Stage.Element);
	Selection.strSourceEffectAssetId = Stage.Identity.strEffectAssetId;
	Selection.strOccurrenceId = Stage.Identity.strOccurrenceId;
	Selection.strRowSha256 = Stage.Identity.strRowSha256;
	Selection.strTargetElementId = Stage.Identity.strTargetElementId;
	Selection.strSourceRecordId = Stage.Identity.strSourceRecordId;
	Selection.strSourceFamily = VisualProgramFamilyLabel(Stage.eSourceFamily);

	EFFECT_ELEMENT_DESC Preset = Selection.GenericElement;
	const EFFECT_AUTHORING_FAMILY eFamily = Resolve_AuthoringFamily(Preset);
	if (!AuthoringFamily_CanCreate(eFamily))
	{
		m_strElementStatus =
			"Presentation Light and Screen Post are edited or deleted in the active Effect; creating them from a source preset is not admitted.";
		return false;
	}
	Preset.strGroupId = "manual.hit1";
	Preset.strElementId.clear();
	Preset.strDisplayName = AuthoringFamily_Label(eFamily);
	Selection.GenericElement = Preset;
	m_eSelectedAuthoringFamily = eFamily;
	m_eSelectedEffectType = Preset.eKind;
	m_MeshAuthoringDraft = std::move(Preset);
	m_bMeshAuthoringDraftInitialized = true;
	m_SourceElementPresetSelection = std::move(Selection);
	m_NewElementId[0u] = '\0';
	const bool_t bRequiresMesh = AuthoringFamily_RequiresMesh(eFamily);
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
	m_strSelectedResourceAssetId.clear();
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	m_iResourceViewRevision = UINT64_MAX;
	m_strElementStatus = std::string("Loaded one ") +
		AuthoringFamily_Label(eFamily) +
		" Track A data as an Element seed. Current Effect and Effect Details were preserved; use Create Element, then Save Changes.";
	return true;
}

bool_t Client::CEffect_Tool::Try_OpenArtistFReconstructedElementForAuthoring(
	const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter)
{
	if (!Ensure_ArtistFSourceSnapshotForAuthoring())
	{
		m_strElementStatus = m_strArtistFSourceSnapshotStatus.empty() ?
			"Artist F source recipe is unavailable." :
			m_strArtistFSourceSnapshotStatus;
		return false;
	}
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = Resolve_VisualProgramProjectionForAuthoring(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		nullptr == m_pArtistFSourcePreparation ? nullptr :
			m_pArtistFSourcePreparation->Get_Program();
	if (nullptr == pProjection || nullptr == pProgram ||
		pProgram->strRuntimeCatalogAssetId != ARTIST_F_VISUAL_PROGRAM_ASSET_ID)
	{
		m_strElementStatus =
			"Artist F source recipe identity changed before import.";
		return false;
	}
	const std::string strVisualOccurrenceId =
		Emitter.strMaterialOccurrenceId.value_or(Emitter.Row.strId);
	if (nullptr != pProjection->Find_RowByOccurrenceId(strVisualOccurrenceId) ||
		nullptr != pProjection->Find_SupplementalElementByOccurrenceId(
			strVisualOccurrenceId))
	{
		m_strElementStatus =
			"This admitted Track A row must use its exact preset-stage identity.";
		return false;
	}

	const auto RuntimeEmitter = std::find_if(
		pProgram->Emitters.begin(), pProgram->Emitters.end(),
		[&Emitter](const EFFECT_RUNTIME_PROGRAM_EMITTER& Candidate)
		{
			return Candidate.Row.strId == Emitter.Row.strId;
		});
	EFFECT_GPU_RENDER_FAMILY eFamily = EFFECT_GPU_RENDER_FAMILY::END;
	if (RuntimeEmitter == pProgram->Emitters.end() ||
		RuntimeEmitter->Row.strRowSha256 != Emitter.Row.strRowSha256 ||
		RuntimeEmitter->strSourceElementId != Emitter.strSourceElementId ||
		RuntimeEmitter->strSourceEmitterPath != Emitter.strSourceEmitterPath ||
		!RuntimeEmitter->bVisible ||
		!Try_ResolveArtistCoreFamily(RuntimeEmitter->eRenderer, eFamily))
	{
		m_strElementStatus =
			"The selected Core33 occurrence identity changed after the tree was built.";
		return false;
	}
	const auto ProjectedElement = std::find_if(
		pProjection->Get_Document().Elements.begin(),
		pProjection->Get_Document().Elements.end(),
		[&Emitter](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == Emitter.strSourceElementId;
		});
	if (ProjectedElement == pProjection->Get_Document().Elements.end() ||
		!ProjectedElement->bVisible)
	{
		m_strElementStatus =
			"The immutable Core33 projection no longer contains this visible Element.";
		return false;
	}

	SOURCE_ELEMENT_PRESET_SELECTION Selection;
	Selection.pProjection = pProjection;
	Selection.strSourceEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	Selection.strOccurrenceId = Emitter.Row.strId;
	Selection.strRowSha256 = Emitter.Row.strRowSha256;
	Selection.strTargetElementId = Emitter.strSourceElementId;
	Selection.strSourceRecordId = Emitter.strSourceEmitterPath;
	Selection.strSourceFamily = ArtistCoreFamilyLabel(eFamily);
	if (!Try_StageElementAsAuthoringPreset(
			pProjection->Get_Document(), Emitter.strSourceElementId,
			std::move(Selection)))
	{
		return false;
	}
	m_strElementStatus = std::string("Loaded one ") +
		ArtistCoreFamilyLabel(eFamily) +
		" Element seed. Safe Detail values and DDS/WModel slots were copied; native source execution was not copied.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SelectVisualOccurrence(
	const std::string& strEffectAssetId,
	const std::string& strOccurrenceId,
	const std::string& strRowSha256,
	const std::string& strTargetElementId,
	const std::string& strSourceRecordId)
{
	const bool_t bChangesSelection =
		EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE != m_eDetailSelection ||
		m_strSelectedRuntimeOccurrenceEffectId != strEffectAssetId ||
		m_strSelectedRuntimeOccurrenceId != strOccurrenceId ||
		m_strSelectedRuntimeOccurrenceRowSha256 != strRowSha256 ||
		m_strSelectedRuntimeOccurrenceElementId != strTargetElementId ||
		m_strSelectedRuntimeOccurrenceEmitterPath != strSourceRecordId;
	if (!bChangesSelection)
		return true;
	if (m_bDocumentDirty || m_bParticleSystemDraftDirty ||
		m_bDetailDraftDirty || m_bModelCueDraftDirty)
	{
		m_strElementStatus =
			"Save or discard the active authored Effect work before opening visual occurrence tuning.";
		return false;
	}
	if (m_bOccurrenceTransformDraftDirty)
	{
		m_strElementStatus =
			"Apply or Revert the occurrence Transform draft before selecting another occurrence.";
		return false;
	}
	if (m_OccurrenceTuningDocument.has_value() &&
		m_OccurrenceTuningDocument->strEffectAssetId != strEffectAssetId &&
		m_bOccurrenceTuningDirty)
	{
		m_strElementStatus =
			"Save or Reload the current occurrence tuning artifact before selecting another Effect.";
		return false;
	}

	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pProjection = CEffectCatalog::Find_VisualProjection(strEffectAssetId);
	const EFFECT_VISUAL_PROGRAM_ROW* pCatalogRow = nullptr == pProjection ?
		nullptr : pProjection->Find_RowByOccurrenceId(
			strOccurrenceId);
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pSupplemental =
		nullptr == pProjection ? nullptr :
			pProjection->Find_SupplementalElementByOccurrenceId(strOccurrenceId);
	const bool_t bVisualRowMatches = nullptr != pCatalogRow &&
		pCatalogRow->strRowSha256 == strRowSha256 &&
		pCatalogRow->bTuningEligibleTransform &&
		pCatalogRow->TargetIdentity.has_value() &&
		pCatalogRow->TargetIdentity->strTargetElementId == strTargetElementId;
	const bool_t bSupplementalMatches = nullptr != pSupplemental &&
		pSupplemental->strRowSha256 == strRowSha256 &&
		pSupplemental->bTuningEligibleTransform &&
		pSupplemental->TargetIdentity.strTargetElementId == strTargetElementId;
	if (nullptr == pProjection ||
		bVisualRowMatches == bSupplementalMatches)
	{
		m_strElementStatus =
			"The selected visual occurrence is not admitted for Transform tuning.";
		return false;
	}
	const shared_ptr<CEffectObject> pPreviewObject =
		m_pWorldPreviewObject.lock();
	if (nullptr == pPreviewObject ||
		!pPreviewObject->Is_SourceVisualProgramActive() ||
		nullptr == m_pVisualPreviewProjection ||
		m_pVisualPreviewProjection->Get_EffectAssetId() != strEffectAssetId)
	{
		m_strElementStatus =
			"Play this Effect cue before selecting one of its visual occurrences.";
		return false;
	}
	const auto SourceElement = std::find_if(
		pProjection->Get_Document().Elements.begin(),
		pProjection->Get_Document().Elements.end(),
		[&strTargetElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strTargetElementId;
		});
	if (SourceElement == pProjection->Get_Document().Elements.end())
	{
		m_strElementStatus =
			"The selected visual occurrence target Element is missing.";
		return false;
	}

	EFFECT_OCCURRENCE_TUNING_DOCUMENT StagedTuning;
	std::filesystem::path StagedPath;
	std::string StagedBaseline;
	std::string Error;
	const bool_t bReuseSession = m_OccurrenceTuningDocument.has_value() &&
		m_OccurrenceTuningDocument->strEffectAssetId == strEffectAssetId;
	if (bReuseSession)
	{
		StagedTuning = *m_OccurrenceTuningDocument;
		StagedPath = m_OccurrenceTuningPath;
		StagedBaseline = m_strOccurrenceTuningBaselineCanonical;
	}
	else
	{
		StagedPath = CProjectDataRoot::Resolve(
			std::filesystem::path("Effects/AuthoredCorrections/VisualPrograms") /
				(strEffectAssetId + ".occurrence-tuning.json"));
		if (StagedPath.empty())
		{
			m_strElementStatus =
				"The visual occurrence tuning authoring path is invalid.";
			return false;
		}
		if (std::filesystem::is_regular_file(StagedPath))
		{
			if (!CEffectOccurrenceTuningCodec::Load(
					StagedPath, StagedTuning, Error) ||
				!CEffectOccurrenceTuningCodec::Validate_AgainstProjection(
					StagedTuning, *pProjection, Error))
			{
				m_strElementStatus = Error.empty() ?
					"Visual occurrence tuning authoring file is unavailable." :
					Error;
				return false;
			}
			StagedBaseline =
				CEffectOccurrenceTuningCodec::Serialize(StagedTuning);
		}
		else
		{
			StagedTuning.iFormatVersion =
				EFFECT_OCCURRENCE_TUNING_FORMAT_VERSION;
			StagedTuning.strEffectAssetId = strEffectAssetId;
			if (!CEffectOccurrenceTuningCodec::Validate_AgainstProjection(
					StagedTuning, *pProjection, Error))
			{
				m_strElementStatus = Error;
				return false;
			}
		}
	}
	if (!Stage_RuntimeOccurrenceTuningPreview(StagedTuning))
		return false;

	EFFECT_OCCURRENCE_LOCAL_TRANSFORM SourceTransform;
	SourceTransform.vPosition = SourceElement->Detail.Transform.vPosition;
	SourceTransform.vRotationDegrees =
		SourceElement->Detail.Transform.vRotationDegrees;
	SourceTransform.vScale = SourceElement->Detail.Transform.vScale;
	const EFFECT_OCCURRENCE_TUNING_ENTRY* pTuned =
		CEffectOccurrenceTuningCodec::Find_Entry(
			StagedTuning, strOccurrenceId);
	const EFFECT_OCCURRENCE_LOCAL_TRANSFORM StagedDraft = nullptr == pTuned ?
		SourceTransform : pTuned->EffectiveLocalTransform;
	const bool_t bStartedEmptyArtifact = StagedBaseline.empty();
	m_OccurrenceTuningDocument = std::move(StagedTuning);
	m_OccurrenceTuningPath = std::move(StagedPath);
	m_strOccurrenceTuningBaselineCanonical = std::move(StagedBaseline);
	m_OccurrenceTransformDraft = StagedDraft;
	m_SelectedOccurrenceSourceTransform = SourceTransform;
	m_pSelectedVisualSourceProjection = pProjection;
	m_strSelectedRuntimeOccurrenceEffectId = strEffectAssetId;
	m_strSelectedRuntimeOccurrenceId = strOccurrenceId;
	m_strSelectedRuntimeOccurrenceRowSha256 = strRowSha256;
	m_strSelectedRuntimeOccurrenceElementId = strTargetElementId;
	m_strSelectedRuntimeOccurrenceEmitterPath = strSourceRecordId;
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument) !=
		m_strOccurrenceTuningBaselineCanonical;
	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE;
	m_strElementStatus =
		"Selected stable visual occurrence; playback scope was preserved.";
	m_strDetailStatus = bReuseSession ?
		"Visual occurrence tuning session preserved." :
		(bStartedEmptyArtifact ?
			"Started an empty PROJECT_TUNED override set from immutable Source." :
			"Loaded and staged the saved visual occurrence tuning artifact.");
	return true;
}

bool_t Client::CEffect_Tool::Try_SetVisualPreviewOccurrenceIsolation(
	const std::string& strTargetElementId)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_SourceVisualProgramActive() ||
		strTargetElementId.empty())
	{
		m_strPreviewStatus =
			"Visual element Solo requires an active admitted visual-program preview.";
		return false;
	}
	EFFECT_PREVIEW_SUBMISSION_ISOLATION Isolation;
	Isolation.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::OCCURRENCE;
	Isolation.strElementId = strTargetElementId;
	std::string Error;
	if (!pObject->Set_PreviewSubmissionIsolation(Isolation, Error))
	{
		m_strPreviewStatus = "Visual element Solo failed: " + Error;
		return false;
	}
	m_strPreviewIsolationElementId = strTargetElementId;
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewStatus = "Visual element Solo: " + strTargetElementId +
		". Click selection remains independent; Return to All restores playback.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SetVisualPreviewFamilyIsolation(
	const EFFECT_VISUAL_PROGRAM& Program,
	const EFFECT_VISUAL_PROGRAM_FAMILY eFamily)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_SourceVisualProgramActive() ||
		eFamily >= EFFECT_VISUAL_PROGRAM_FAMILY::END)
	{
		m_strPreviewStatus =
			"Visual Family playback requires an active admitted visual-program preview.";
		return false;
	}
	std::set<std::string, std::less<>> TargetIds;
	for (const EFFECT_VISUAL_PROGRAM_ROW& Row : Program.VisualRows)
	{
		if (Row.eFamily == eFamily && Row.eDisposition ==
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
			Row.TargetIdentity.has_value())
		{
			TargetIds.insert(Row.TargetIdentity->strTargetElementId);
		}
	}
	for (const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT& Row :
		Program.SupplementalElements)
	{
		if (Row.eFamily == eFamily && Row.eDisposition ==
				EFFECT_VISUAL_PROGRAM_DISPOSITION::ADMITTED_BOUNDED &&
			!Row.TargetIdentity.strTargetElementId.empty())
		{
			TargetIds.insert(Row.TargetIdentity.strTargetElementId);
		}
	}
	if (TargetIds.empty())
	{
		m_strPreviewStatus =
			"Visual Family has no admitted runtime target Elements.";
		return false;
	}
	EFFECT_PREVIEW_SUBMISSION_ISOLATION Isolation;
	Isolation.eKind = EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ELEMENT_SET;
	Isolation.ElementIds.assign(TargetIds.begin(), TargetIds.end());
	std::string Error;
	if (!pObject->Set_PreviewSubmissionIsolation(Isolation, Error))
	{
		m_strPreviewStatus = "Visual Family playback failed: " + Error;
		return false;
	}
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId = VisualProgramFamilyLabel(eFamily);
	m_strPreviewStatus = std::string("Visual-program playback Family: ") +
		VisualProgramFamilyLabel(eFamily) + " (" +
		std::to_string(TargetIds.size()) + " stable targets).";
	return true;
}

bool_t Client::CEffect_Tool::Try_ResetVisualPreviewIsolation()
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_SourceVisualProgramActive())
	{
		m_strPreviewStatus =
			"Return to All requires an active admitted visual-program preview.";
		return false;
	}
	pObject->Reset_PreviewSubmissionIsolation();
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewStatus = "Visual-program playback scope: All.";
	return true;
}

void Client::CEffect_Tool::Render_RuntimeOccurrenceDetail()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if ((!bSourceBacked && !m_OccurrenceTuningDocument.has_value()) ||
		!m_OccurrenceTransformDraft.has_value() ||
		m_strSelectedRuntimeOccurrenceId.empty())
	{
		ImGui::TextDisabled(
			"The selected Element edit session is unavailable.");
		return;
	}
	ImGui::TextWrapped("Artist F | %s",
		m_strSelectedRuntimeOccurrenceElementId.c_str());
	if (bSourceBacked)
	{
		ImGui::TextDisabled(
			"Track A source-backed Element. Renderer, DDS roles, attachment, timing distributions, and material evaluator remain connected.");
		if (m_pArtistFSourceProjection)
		{
			const auto Element = std::find_if(
				m_pArtistFSourceProjection->Get_Document().Elements.begin(),
				m_pArtistFSourceProjection->Get_Document().Elements.end(),
				[this](const EFFECT_ELEMENT_DESC& Candidate)
				{
					return Candidate.strElementId ==
						m_strSelectedRuntimeOccurrenceElementId;
				});
			if (Element !=
				m_pArtistFSourceProjection->Get_Document().Elements.end())
			{
				ImGui::TextWrapped("Connected slots: %s",
					AuthoringElementResourceSlotSummary(*Element).c_str());
			}
		}
	}
	if (ImGui::TreeNode("Advanced Source Identity"))
	{
		ImGui::TextWrapped("Effect: %s",
			m_strSelectedRuntimeOccurrenceEffectId.c_str());
		ImGui::TextWrapped("Occurrence: %s",
			m_strSelectedRuntimeOccurrenceId.c_str());
		ImGui::TextWrapped("Source Emitter: %s",
			m_strSelectedRuntimeOccurrenceEmitterPath.c_str());
		ImGui::TextDisabled("Source row SHA-256: %s",
			m_strSelectedRuntimeOccurrenceRowSha256.c_str());
		ImGui::TreePop();
	}
	ImGui::Separator();
	ImGui::TextDisabled("Source local Transform (read-only)");
	ImGui::TextDisabled("Position: %.4f, %.4f, %.4f",
		m_SelectedOccurrenceSourceTransform.vPosition.x,
		m_SelectedOccurrenceSourceTransform.vPosition.y,
		m_SelectedOccurrenceSourceTransform.vPosition.z);
	ImGui::TextDisabled("Rotation: %.4f, %.4f, %.4f degrees",
		m_SelectedOccurrenceSourceTransform.vRotationDegrees.x,
		m_SelectedOccurrenceSourceTransform.vRotationDegrees.y,
		m_SelectedOccurrenceSourceTransform.vRotationDegrees.z);
	ImGui::TextDisabled("Scale: %.4f, %.4f, %.4f",
		m_SelectedOccurrenceSourceTransform.vScale.x,
		m_SelectedOccurrenceSourceTransform.vScale.y,
		m_SelectedOccurrenceSourceTransform.vScale.z);
	ImGui::Separator();
	const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY* pSourceOverride =
		bSourceBacked ? CEffectSourceAuthoringOverlayCodec::Find_Entry(
			*m_SourceAuthoringOverlayDocument,
			m_strSelectedRuntimeOccurrenceId) : nullptr;
	const EFFECT_OCCURRENCE_TUNING_ENTRY* pLegacyOverride =
		bSourceBacked ? nullptr : CEffectOccurrenceTuningCodec::Find_Entry(
			*m_OccurrenceTuningDocument,
			m_strSelectedRuntimeOccurrenceId);
	const bool_t bHasCommittedOverride =
		nullptr != pSourceOverride || nullptr != pLegacyOverride;
	ImGui::TextDisabled(bHasCommittedOverride ?
		"Saved in-memory values are active for this Element." :
		"This Element currently uses the Track A source Transform.");
	bool_t bChanged = false;
	bChanged |= DragFloat3("Position",
		m_OccurrenceTransformDraft->vPosition, 0.01f, -1000.f, 1000.f);
	bChanged |= DragFloat3("Rotation (Degrees)",
		m_OccurrenceTransformDraft->vRotationDegrees,
		0.25f, -360.f, 360.f);
	bChanged |= DragFloat3("Scale",
		m_OccurrenceTransformDraft->vScale, 0.01f, 0.001f, 100.f);
	if (bChanged)
	{
		m_bOccurrenceTransformDraftDirty = true;
		bool_t bPreviewStaged = false;
		if (bSourceBacked)
		{
			EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Preview =
				*m_SourceAuthoringOverlayDocument;
			Upsert_SourceAuthoringOverlayEntry(Preview,
				m_strSelectedRuntimeOccurrenceId,
				m_strSelectedRuntimeOccurrenceRowSha256,
				m_strSelectedRuntimeOccurrenceElementId,
				*m_OccurrenceTransformDraft);
			bPreviewStaged = Stage_SourceAuthoringOverlayPreview(Preview);
		}
		else
		{
			EFFECT_OCCURRENCE_TUNING_DOCUMENT Preview =
				*m_OccurrenceTuningDocument;
			Upsert_OccurrenceTuningEntry(Preview,
				m_strSelectedRuntimeOccurrenceId,
				m_strSelectedRuntimeOccurrenceRowSha256,
				*m_OccurrenceTransformDraft);
			bPreviewStaged = Stage_RuntimeOccurrenceTuningPreview(Preview);
		}
		if (bPreviewStaged)
		{
			m_strDetailStatus =
				"Live preview updated. Save Changes persists this Element.";
		}
	}
	ImGui::Separator();
	ImGui::TextDisabled("Effective local Transform (read-only preview)");
	const EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Effective =
		*m_OccurrenceTransformDraft;
	ImGui::TextDisabled("Position: %.4f, %.4f, %.4f",
		Effective.vPosition.x, Effective.vPosition.y, Effective.vPosition.z);
	ImGui::TextDisabled("Rotation: %.4f, %.4f, %.4f degrees",
		Effective.vRotationDegrees.x, Effective.vRotationDegrees.y,
		Effective.vRotationDegrees.z);
	ImGui::TextDisabled("Scale: %.4f, %.4f, %.4f",
		Effective.vScale.x, Effective.vScale.y, Effective.vScale.z);

	ImGui::Separator();
	ImGui::BeginDisabled(!m_bOccurrenceTransformDraftDirty);
	if (ImGui::Button("Apply Tuning"))
		Try_ApplyRuntimeOccurrenceDraft();
	ImGui::SameLine();
	if (ImGui::Button("Revert Draft"))
	{
		const EFFECT_OCCURRENCE_LOCAL_TRANSFORM Reverted =
			nullptr != pSourceOverride ?
				pSourceOverride->EffectiveLocalTransform :
			(nullptr != pLegacyOverride ?
				pLegacyOverride->EffectiveLocalTransform :
				m_SelectedOccurrenceSourceTransform);
		const bool_t bRestaged = bSourceBacked ?
			Stage_SourceAuthoringOverlayPreview(
				*m_SourceAuthoringOverlayDocument) :
			Stage_RuntimeOccurrenceTuningPreview(
				*m_OccurrenceTuningDocument);
		if (bRestaged)
		{
			m_OccurrenceTransformDraft = Reverted;
			m_bOccurrenceTransformDraftDirty = false;
			m_strDetailStatus =
				"Reverted the draft to the last applied Element values.";
		}
	}
	ImGui::EndDisabled();

	ImGui::BeginDisabled(!bHasCommittedOverride);
	if (ImGui::Button("Reset to Source"))
		Try_ResetRuntimeOccurrenceToSource();
	ImGui::EndDisabled();

	ImGui::Separator();
	ImGui::BeginDisabled(
		!m_bOccurrenceTuningDirty && !m_bOccurrenceTransformDraftDirty &&
		!m_bSourceAuthoringOverlayNeedsInitialSave);
	if (ImGui::Button("Save Changes"))
	{
		const bool_t bApplied = !m_bOccurrenceTransformDraftDirty ||
			Try_ApplyRuntimeOccurrenceDraft();
		if (bApplied)
			Try_SaveRuntimeOccurrenceTuning();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bOccurrenceTransformDraftDirty);
	if (ImGui::Button("Reload Saved"))
		Try_ReloadRuntimeOccurrenceTuning();
	ImGui::EndDisabled();

	ImGui::TextWrapped("%s", m_strDetailStatus.empty() ?
		"Edit Position, Rotation, or Scale, then Save Changes." :
		m_strDetailStatus.c_str());
	if (bSourceBacked)
	{
		ImGui::TextDisabled(
			"Save writes only the Artist F edit overlay. Track A source data and Product mapping are unchanged.");
	}
}

bool_t Client::CEffect_Tool::Stage_RuntimeOccurrenceTuningPreview(
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pSourceProjection = CEffectCatalog::Find_VisualProjection(
			Tuning.strEffectAssetId);
	if (nullptr != pSourceProjection)
	{
		if (nullptr == pObject)
		{
			m_strDetailStatus =
				"Visual occurrence tuning preview requires an active world preview.";
			return false;
		}
		EFFECT_DOCUMENT_DESC TunedDocument = pSourceProjection->Get_Document();
		std::string Error;
		if (!CEffectOccurrenceTuningCodec::Apply_ToProjectedDocument(
				TunedDocument, *pSourceProjection, Tuning, Error))
		{
			m_strDetailStatus = Error;
			return false;
		}
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pTunedProjection;
		if (!CEffectVisualProgramCorpusCodec::Derive_TransformTunedProjection(
				*pSourceProjection, TunedDocument, pTunedProjection, Error) ||
			nullptr == pTunedProjection)
		{
			m_strDetailStatus = Error.empty() ?
				"Visual Transform tuning projection could not be resealed." : Error;
			return false;
		}
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pTunedPrepared;
		if (!CEffectDocumentRenderer::Prepare_VisualProgramDocument(
				m_pDevice, m_pContext, pTunedProjection, pTunedPrepared, Error) ||
			nullptr == pTunedPrepared)
		{
			m_strDetailStatus = Error.empty() ?
				"Visual Transform tuning prewarm failed." : Error;
			return false;
		}
		const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pRollbackProjection = nullptr != m_pVisualPreviewProjection ?
				m_pVisualPreviewProjection : pSourceProjection;
		std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
			pRollbackPrepared;
		if (!CEffectDocumentRenderer::Prepare_VisualProgramDocument(
				m_pDevice, m_pContext, pRollbackProjection,
				pRollbackPrepared, Error) || nullptr == pRollbackPrepared)
		{
			m_strDetailStatus = Error.empty() ?
				"Visual Transform tuning rollback prewarm failed." : Error;
			return false;
		}
		const EFFECT_PREVIEW_SUBMISSION_ISOLATION PreviousIsolation =
			pObject->Get_PreviewSubmissionIsolation();
		const auto ApplyIsolation = [&pObject](
			const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
			std::string& strOutError)
		{
			if (Isolation.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL)
			{
				pObject->Reset_PreviewSubmissionIsolation();
				strOutError.clear();
				return true;
			}
			return pObject->Set_PreviewSubmissionIsolation(
				Isolation, strOutError);
		};
		const auto RestorePrior = [&]()
		{
			std::string RollbackError;
			const bool_t bRestaged =
				pObject->Stage_PrevalidatedVisualProgramDocument(
					pRollbackProjection, pRollbackPrepared, RollbackError);
			const bool_t bIsolation = bRestaged &&
				ApplyIsolation(PreviousIsolation, RollbackError);
			if (bRestaged && bIsolation)
			{
				pObject->Set_SampleTime(
					Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
			}
			return std::pair<bool_t, std::string>{
				bRestaged && bIsolation, std::move(RollbackError) };
		};
		if (!pObject->Stage_PrevalidatedVisualProgramDocument(
				pTunedProjection, pTunedPrepared, Error) ||
			!ApplyIsolation(PreviousIsolation, Error))
		{
			const auto [bRestored, RollbackError] = RestorePrior();
			m_strDetailStatus = "Visual Transform preview failed: " + Error +
				(bRestored ? " Previous preview/scope restored." :
					" Rollback failed: " + RollbackError);
			return false;
		}
		pObject->Set_SampleTime(
			Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
		m_pVisualPreviewProjection = std::move(pTunedProjection);
		m_strDetailStatus =
			"Visual Transform override staged; playback scope and sample time preserved.";
		return true;
	}
	const auto pPreparation = nullptr == pObject ? nullptr :
		pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == pObject || nullptr == pPreparation ||
		!m_bReconstructedSourceRuntimeActive)
	{
		m_strDetailStatus =
			"Occurrence tuning preview requires an active reconstructed runtime preview.";
		return false;
	}
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION PreviousIsolation =
		pObject->Get_PreviewSubmissionIsolation();
	const std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pEntry =
		CEffectCatalog::Find_RuntimeProgramEntry(Tuning.strEffectAssetId);
	const std::shared_ptr<const EFFECT_OCCURRENCE_TUNING_DOCUMENT> pPublished =
		nullptr == pEntry ? nullptr : pEntry->Get_OccurrenceTuning();
	const EFFECT_OCCURRENCE_TUNING_DOCUMENT* pRollback =
		m_OccurrenceTuningDocument.has_value() &&
		m_OccurrenceTuningDocument->strEffectAssetId == Tuning.strEffectAssetId ?
			&*m_OccurrenceTuningDocument : pPublished.get();
	const auto StageNext =
		[this, &pObject, &pPreparation, &Tuning](std::string& strOutError)
		{
			return CEffectPresentationService::
				Stage_ReconstructedOccurrenceTuningPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					Tuning, strOutError);
		};
	const auto StageRollback =
		[this, &pObject, &pPreparation, pRollback](std::string& strOutError)
		{
			if (nullptr == pRollback)
			{
				strOutError =
					"The prior occurrence tuning artifact is unavailable.";
				return false;
			}
			return CEffectPresentationService::
				Stage_ReconstructedOccurrenceTuningPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					*pRollback, strOutError);
		};
	const auto ApplyIsolation =
		[&pObject](
			const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
			std::string& strOutError)
		{
			if (Isolation.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL)
			{
				pObject->Reset_PreviewSubmissionIsolation();
				strOutError.clear();
				return true;
			}
			return pObject->Set_PreviewSubmissionIsolation(
				Isolation, strOutError);
		};
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	std::string Error;
	if (!CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
			PreviousIsolation, StageNext, StageRollback, ApplyIsolation, Error))
	{
		m_strDetailStatus = "Occurrence tuning preview failed: " + Error;
		return false;
	}
	if (!Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime))
	{
		std::string RollbackError;
		const bool_t bRollbackStaged =
			CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
				PreviousIsolation, StageRollback, StageRollback,
				ApplyIsolation, RollbackError);
		const bool_t bRollbackSeeked = bRollbackStaged &&
			Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime);
		m_strDetailStatus = bRollbackSeeked ?
			"Occurrence tuning preview could not commit; the previous tuning, All/Family scope, and synchronized sample time were restored." :
			"Occurrence tuning preview failed and rollback could not restore its prior tuning, All/Family scope, and synchronized sample time: " +
				(RollbackError.empty() ? std::string("seek failed.") : RollbackError);
		return false;
	}
	m_strDetailStatus =
		"Occurrence tuning staged object-locally at the current synchronized sample time.";
	return true;
}

bool_t Client::CEffect_Tool::Stage_SourceAuthoringOverlayPreview(
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Overlay)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	const auto pPreparation = nullptr == pObject ? nullptr :
		pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == pObject || nullptr == pPreparation ||
		!m_bReconstructedSourceRuntimeActive)
	{
		m_strDetailStatus =
			"Artist F editing requires an active Track A source preview.";
		return false;
	}
	const EFFECT_PREVIEW_SUBMISSION_ISOLATION PreviousIsolation =
		pObject->Get_PreviewSubmissionIsolation();
	const bool_t bPreviousPlaying = m_bPreviewPlaying;
	const bool_t bPreviousVisible = m_bPreviewVisibleRequested;
	const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT* pRollback =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_SourceAuthoringOverlayDocument->strEffectAssetId ==
			Overlay.strEffectAssetId ?
			&*m_SourceAuthoringOverlayDocument : nullptr;
	const auto StageNext =
		[this, &pObject, &pPreparation, &Overlay](std::string& strOutError)
		{
			return CEffectPresentationService::
				Stage_ReconstructedSourceAuthoringOverlayPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					Overlay, strOutError);
		};
	const auto StageRollback =
		[this, &pObject, &pPreparation, pRollback](std::string& strOutError)
		{
			if (nullptr == pRollback)
			{
				strOutError =
					"The prior Artist F edit overlay is unavailable.";
				return false;
			}
			return CEffectPresentationService::
				Stage_ReconstructedSourceAuthoringOverlayPreview(
					m_pDevice, m_pContext, pObject, pPreparation,
					*pRollback, strOutError);
		};
	const auto ApplyIsolation =
		[&pObject](const EFFECT_PREVIEW_SUBMISSION_ISOLATION& Isolation,
			std::string& strOutError)
		{
			if (Isolation.eKind ==
				EFFECT_PREVIEW_SUBMISSION_ISOLATION_KIND::ALL)
			{
				pObject->Reset_PreviewSubmissionIsolation();
				strOutError.clear();
				return true;
			}
			return pObject->Set_PreviewSubmissionIsolation(
				Isolation, strOutError);
		};
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	std::string Error;
	if (!CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
			PreviousIsolation, StageNext, StageRollback, ApplyIsolation, Error))
	{
		const bool_t bRollbackFailed =
			std::string::npos != Error.find("Rollback failed:");
		m_bPreviewPlaying = !bRollbackFailed && bPreviousPlaying;
		m_bPreviewVisibleRequested = !bRollbackFailed && bPreviousVisible;
		pObject->Set_Playing(false);
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
		m_strDetailStatus = "Artist F edit preview failed: " + Error;
		return false;
	}
	if (!Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime))
	{
		std::string RollbackError;
		const bool_t bRollbackStaged =
			CEffectPresentationService::Restage_ObjectLocalOccurrencePreview(
				PreviousIsolation, StageRollback, StageRollback,
				ApplyIsolation, RollbackError);
		const bool_t bRollbackSeeked = bRollbackStaged &&
			Seek_ReconstructedSourceRuntimeTimeline(fPreviousTime);
		m_bPreviewPlaying = bRollbackSeeked && bPreviousPlaying;
		m_bPreviewVisibleRequested = bRollbackSeeked && bPreviousVisible;
		pObject->Set_Playing(false);
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
		m_strDetailStatus = bRollbackSeeked ?
			"Artist F edit preview could not commit; the previous values, playback scope, and sample time were restored." :
			"Artist F edit preview failed and rollback could not restore the prior session: " +
				(RollbackError.empty() ? std::string("seek failed.") :
					RollbackError);
		return false;
	}
	m_bPreviewPlaying = bPreviousPlaying;
	m_bPreviewVisibleRequested = bPreviousVisible;
	pObject->Set_Playing(false);
	pObject->Set_Visible(bPreviousVisible);
	Set_SynchronizedAnimationPaused(!bPreviousPlaying);
	m_strDetailStatus =
		"Artist F source-backed preview updated at the current sample time.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyRuntimeOccurrenceDraft()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if ((!bSourceBacked && !m_OccurrenceTuningDocument.has_value()) ||
		!m_OccurrenceTransformDraft.has_value() ||
		!m_bOccurrenceTransformDraftDirty)
		return false;
	if (bSourceBacked)
	{
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged =
			*m_SourceAuthoringOverlayDocument;
		Upsert_SourceAuthoringOverlayEntry(Staged,
			m_strSelectedRuntimeOccurrenceId,
			m_strSelectedRuntimeOccurrenceRowSha256,
			m_strSelectedRuntimeOccurrenceElementId,
			*m_OccurrenceTransformDraft);
		if (!Stage_SourceAuthoringOverlayPreview(Staged))
			return false;
		m_SourceAuthoringOverlayDocument = std::move(Staged);
		m_bOccurrenceTransformDraftDirty = false;
		m_bOccurrenceTuningDirty =
			CEffectSourceAuthoringOverlayCodec::Serialize(
				*m_SourceAuthoringOverlayDocument) !=
			m_strSourceAuthoringOverlayBaselineCanonical;
		m_strDetailStatus =
			"Applied this Element to the in-memory Artist F edit overlay; Save Changes is required to persist.";
		return true;
	}
	EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged = *m_OccurrenceTuningDocument;
	Upsert_OccurrenceTuningEntry(Staged,
		m_strSelectedRuntimeOccurrenceId,
		m_strSelectedRuntimeOccurrenceRowSha256,
		*m_OccurrenceTransformDraft);
	if (!Stage_RuntimeOccurrenceTuningPreview(Staged))
		return false;
	m_OccurrenceTuningDocument = std::move(Staged);
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument) !=
		m_strOccurrenceTuningBaselineCanonical;
	m_strDetailStatus =
		"Applied to the in-memory occurrence tuning artifact; Save is required to persist.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ResetRuntimeOccurrenceToSource()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if (!bSourceBacked && !m_OccurrenceTuningDocument.has_value())
		return false;
	if (bSourceBacked)
	{
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged =
			*m_SourceAuthoringOverlayDocument;
		Remove_SourceAuthoringOverlayEntry(
			Staged, m_strSelectedRuntimeOccurrenceId);
		if (!Stage_SourceAuthoringOverlayPreview(Staged))
			return false;
		m_SourceAuthoringOverlayDocument = std::move(Staged);
		m_OccurrenceTransformDraft = m_SelectedOccurrenceSourceTransform;
		m_bOccurrenceTransformDraftDirty = false;
		m_bOccurrenceTuningDirty =
			CEffectSourceAuthoringOverlayCodec::Serialize(
				*m_SourceAuthoringOverlayDocument) !=
			m_strSourceAuthoringOverlayBaselineCanonical;
		m_strDetailStatus =
			"Restored the immutable Track A source Transform; Save Changes is required to persist.";
		return true;
	}
	EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged = *m_OccurrenceTuningDocument;
	Remove_OccurrenceTuningEntry(Staged, m_strSelectedRuntimeOccurrenceId);
	if (!Stage_RuntimeOccurrenceTuningPreview(Staged))
		return false;
	m_OccurrenceTuningDocument = std::move(Staged);
	m_OccurrenceTransformDraft = m_SelectedOccurrenceSourceTransform;
	m_bOccurrenceTransformDraftDirty = false;
	m_bOccurrenceTuningDirty =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument) !=
		m_strOccurrenceTuningBaselineCanonical;
	m_strDetailStatus =
		"Removed this PROJECT_TUNED entry and restored the immutable source Transform; Save is required to persist.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SaveRuntimeOccurrenceTuning()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if (bSourceBacked)
	{
		if (m_SourceAuthoringOverlayPath.empty())
		{
			m_strDetailStatus =
				"There is no Artist F edit overlay path to save.";
			return false;
		}
		if (m_bOccurrenceTransformDraftDirty)
		{
			m_strDetailStatus =
				"Apply or Revert the Element Transform draft before saving.";
			return false;
		}
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pProgram = CEffectCatalog::Find_ReconstructedRuntimeProgram(
				m_SourceAuthoringOverlayDocument->strEffectAssetId);
		std::string Error;
		if (nullptr == pProgram ||
			!CEffectSourceAuthoringOverlayCodec::Save_AtomicIfUnchanged(
				m_SourceAuthoringOverlayPath,
				*m_SourceAuthoringOverlayDocument, *pProgram,
				m_strSourceAuthoringOverlayBaselineCanonical, Error))
		{
			m_strDetailStatus = nullptr == pProgram ?
				"Artist F Track A source Program is no longer admitted." : Error;
			return false;
		}
		m_strSourceAuthoringOverlayBaselineCanonical =
			CEffectSourceAuthoringOverlayCodec::Serialize(
				*m_SourceAuthoringOverlayDocument);
		m_bOccurrenceTuningDirty = false;
		m_bSourceAuthoringOverlayNeedsInitialSave = false;
		m_strDetailStatus =
			"Saved Artist F Element changes atomically. Track A source data and Product mapping were not modified.";
		return true;
	}
	if (!m_OccurrenceTuningDocument.has_value() ||
		m_OccurrenceTuningPath.empty())
	{
		m_strDetailStatus = "There is no occurrence tuning artifact to save.";
		return false;
	}
	if (m_bOccurrenceTransformDraftDirty)
	{
		m_strDetailStatus =
			"Apply or Revert the occurrence Transform draft before saving.";
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		CEffectCatalog::Find_ReconstructedRuntimeProgram(
			m_OccurrenceTuningDocument->strEffectAssetId);
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection = CEffectCatalog::Find_VisualProjection(
			m_OccurrenceTuningDocument->strEffectAssetId);
	std::string Error;
	const bool_t bSaved = nullptr != pVisualProjection ?
		CEffectOccurrenceTuningCodec::Save_AtomicIfUnchanged(
			m_OccurrenceTuningPath, *m_OccurrenceTuningDocument,
			*pVisualProjection, m_strOccurrenceTuningBaselineCanonical, Error) :
		(nullptr != pProgram &&
		 CEffectOccurrenceTuningCodec::Save_AtomicIfUnchanged(
			m_OccurrenceTuningPath, *m_OccurrenceTuningDocument, *pProgram,
			m_strOccurrenceTuningBaselineCanonical, Error));
	if (!bSaved)
	{
		m_strDetailStatus = nullptr == pProgram && nullptr == pVisualProjection ?
			"Occurrence tuning Program is no longer admitted by the catalog." : Error;
		return false;
	}
	m_strOccurrenceTuningBaselineCanonical =
		CEffectOccurrenceTuningCodec::Serialize(*m_OccurrenceTuningDocument);
	m_bOccurrenceTuningDirty = false;
	m_strDetailStatus =
		"Saved the source occurrence tuning artifact atomically; Publish Effects and runtime catalog reload are still required.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ReloadRuntimeOccurrenceTuning()
{
	const bool_t bSourceBacked =
		m_SourceAuthoringOverlayDocument.has_value() &&
		m_strSelectedRuntimeOccurrenceEffectId ==
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
	if (bSourceBacked)
	{
		if (m_SourceAuthoringOverlayPath.empty() ||
			m_bOccurrenceTransformDraftDirty)
		{
			m_strDetailStatus =
				"Apply or Revert the current Element draft before Reload Saved.";
			return false;
		}
		const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
			pProgram = CEffectCatalog::Find_ReconstructedRuntimeProgram(
				m_SourceAuthoringOverlayDocument->strEffectAssetId);
		if (nullptr == pProgram)
		{
			m_strDetailStatus =
				"Artist F Track A source Program is no longer admitted.";
			return false;
		}
		EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT Staged;
		Staged.strEffectAssetId = ARTIST_F_VISUAL_PROGRAM_ASSET_ID;
		Staged.strSourceProgramSha256 = pProgram->Identity.strProgramSha256;
		Staged.SupplementalDocument =
			CEffectSourceAuthoringOverlayCodec::Create_EmptySupplementalDocument(
				ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
		std::string Error;
		std::error_code FileError;
		const bool_t bExists = std::filesystem::is_regular_file(
			m_SourceAuthoringOverlayPath, FileError);
		if (FileError ||
			(bExists && !CEffectSourceAuthoringOverlayCodec::Load(
				m_SourceAuthoringOverlayPath, Staged, Error)) ||
			!CEffectSourceAuthoringOverlayCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error) ||
			!Stage_SourceAuthoringOverlayPreview(Staged))
		{
			if (!Error.empty())
				m_strDetailStatus = Error;
			else if (FileError)
				m_strDetailStatus = FileError.message();
			return false;
		}
		const EFFECT_SOURCE_AUTHORING_OVERLAY_ENTRY* pEntry =
			CEffectSourceAuthoringOverlayCodec::Find_Entry(
				Staged, m_strSelectedRuntimeOccurrenceId);
		m_OccurrenceTransformDraft = nullptr == pEntry ?
			m_SelectedOccurrenceSourceTransform :
			pEntry->EffectiveLocalTransform;
		m_strSourceAuthoringOverlayBaselineCanonical = bExists ?
			CEffectSourceAuthoringOverlayCodec::Serialize(Staged) :
			std::string{};
		m_SourceAuthoringOverlayDocument = std::move(Staged);
		m_bOccurrenceTuningDirty = false;
		m_bOccurrenceTransformDraftDirty = false;
		m_bSourceAuthoringOverlayNeedsInitialSave = !bExists;
		m_strDetailStatus =
			"Reloaded and staged the saved Artist F Element changes transactionally.";
		return true;
	}
	if (!m_OccurrenceTuningDocument.has_value() ||
		m_OccurrenceTuningPath.empty() || m_bOccurrenceTuningDirty ||
		m_bOccurrenceTransformDraftDirty)
	{
		m_strDetailStatus =
			"Reload requires a clean occurrence tuning session.";
		return false;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		CEffectCatalog::Find_ReconstructedRuntimeProgram(
			m_OccurrenceTuningDocument->strEffectAssetId);
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pVisualProjection = CEffectCatalog::Find_VisualProjection(
			m_OccurrenceTuningDocument->strEffectAssetId);
	EFFECT_OCCURRENCE_TUNING_DOCUMENT Staged;
	std::string Error;
	const bool_t bLoaded =
		CEffectOccurrenceTuningCodec::Load(
			m_OccurrenceTuningPath, Staged, Error);
	const bool_t bValidated = bLoaded &&
		(nullptr != pVisualProjection ?
			CEffectOccurrenceTuningCodec::Validate_AgainstProjection(
				Staged, *pVisualProjection, Error) :
			(nullptr != pProgram &&
			 CEffectOccurrenceTuningCodec::Validate_AgainstProgram(
				Staged, *pProgram, Error)));
	if (!bValidated ||
		!Stage_RuntimeOccurrenceTuningPreview(Staged))
	{
		if (!Error.empty())
			m_strDetailStatus = Error;
		return false;
	}
	const EFFECT_OCCURRENCE_TUNING_ENTRY* pEntry =
		CEffectOccurrenceTuningCodec::Find_Entry(
			Staged, m_strSelectedRuntimeOccurrenceId);
	m_OccurrenceTransformDraft = nullptr == pEntry ?
		m_SelectedOccurrenceSourceTransform : pEntry->EffectiveLocalTransform;
	m_strOccurrenceTuningBaselineCanonical =
		CEffectOccurrenceTuningCodec::Serialize(Staged);
	m_OccurrenceTuningDocument = std::move(Staged);
	m_bOccurrenceTuningDirty = false;
	m_bOccurrenceTransformDraftDirty = false;
	m_bSourceAuthoringOverlayNeedsInitialSave = false;
	m_strDetailStatus =
		"Reloaded and staged the saved source occurrence tuning artifact transactionally.";
	return true;
}

void Client::CEffect_Tool::Reset_RuntimeOccurrenceTuningSession()
{
	m_OccurrenceTuningDocument.reset();
	m_SourceAuthoringOverlayDocument.reset();
	m_OccurrenceTransformDraft.reset();
	m_OccurrenceTuningPath.clear();
	m_SourceAuthoringOverlayPath.clear();
	m_strOccurrenceTuningBaselineCanonical.clear();
	m_strSourceAuthoringOverlayBaselineCanonical.clear();
	m_strSelectedRuntimeOccurrenceEffectId.clear();
	m_strSelectedRuntimeOccurrenceId.clear();
	m_strSelectedRuntimeOccurrenceRowSha256.clear();
	m_strSelectedRuntimeOccurrenceElementId.clear();
	m_strSelectedRuntimeOccurrenceEmitterPath.clear();
	m_SelectedOccurrenceSourceTransform = {};
	m_pSelectedVisualSourceProjection.reset();
	m_bOccurrenceTuningDirty = false;
	m_bOccurrenceTransformDraftDirty = false;
	m_bSourceAuthoringOverlayNeedsInitialSave = false;
	if (EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
}

bool_t Client::CEffect_Tool::Synchronize_Artist31470FullPreview(
	const std::shared_ptr<const
		EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation)
{
	m_strPreviewAnimationStatus.clear();
	if (nullptr == pPreparation ||
		5u != pPreparation->Get_AnchorRequests().size())
	{
		m_strPreviewAnimationStatus =
			"Artist F exact five-anchor preparation is unavailable.";
		return false;
	}
	std::string CatalogStatus;
	if (!Ensure_PlayerSkillCatalog(CatalogStatus))
	{
		m_strPreviewAnimationStatus =
			"Artist F animation catalog unavailable: " + CatalogStatus;
		return false;
	}
	const vector<PLAYER_SKILL_DEFINITION>& Skills =
		CPlayerSkillCatalog::Get_Skills();
	const auto Skill = std::find_if(Skills.begin(), Skills.end(),
		[](const PLAYER_SKILL_DEFINITION& Candidate)
		{
			return Candidate.eCharacterClass ==
				LostArk::Shared::CHARACTER_CLASS_ID::ARTIST &&
				Candidate.iSkillId == 31470u;
		});
	if (Skill == Skills.end())
	{
		m_strPreviewAnimationStatus =
			"Artist F animation catalog row 31470 is unavailable.";
		return false;
	}
	const char* pAnimationAsset = Animation_AssetName(
		LostArk::Shared::CHARACTER_CLASS_ID::ARTIST);
	if (nullptr == pAnimationAsset ||
		(CAnimationTargetService::Resolve_AssetName() != pAnimationAsset &&
		 !m_pCharacterPreviewPanel->Select_TargetAsset(pAnimationAsset)))
	{
		m_strPreviewAnimationStatus =
			"Artist preview character could not be staged.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		m_strPreviewAnimationStatus =
			"Artist preview animation model is unavailable.";
		return false;
	}
	float4x4_t PreviewRoot{};
	if (!CAnimationTargetService::Resolve_RootTransform(&PreviewRoot))
	{
		m_strPreviewAnimationStatus =
			"Artist preview root transform is unavailable.";
		return false;
	}
	for (const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding :
		pPreparation->Get_AnchorRequests())
	{
		const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request = Binding.Request;
		if (!Request.bFollow || Request.strRuntimeAnchorSlotId.empty() ||
			Request.strRuntimeBoneName.empty() ||
			!pModel->Has_Bone(Request.strRuntimeBoneName.c_str()))
		{
			m_strPreviewAnimationStatus =
				"Artist F required animation anchor is unavailable: " +
				Request.strRuntimeBoneName;
			return false;
		}
		EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
		XMStoreFloat4x4(&AnchorBuild.RawBone,
			pModel->Get_BoneMatrix(Request.strRuntimeBoneName.c_str()));
		AnchorBuild.OwnerWorld = PreviewRoot;
		float4x4_t Anchor{};
		if (!CEffectPresentationService::Build_SourceBoneAnchorWorld(
			AnchorBuild, Anchor))
		{
			m_strPreviewAnimationStatus =
				"Artist F required animation anchor import transform is invalid: " +
				Request.strRuntimeBoneName;
			return false;
		}
	}
	ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
	std::string BindingStatus;
	if (!CAnimationSkillBindingDocument::Load(pAnimationAsset,
		LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, Skills,
		Collect_AnimationClipNames(pModel),
		Bindings, BindingStatus))
	{
		m_strPreviewAnimationStatus =
			"Artist F animation binding unavailable: " + BindingStatus;
		return false;
	}
	const auto Binding = std::find_if(Bindings.Bindings.begin(),
		Bindings.Bindings.end(), [](const ANIMATION_SKILL_BINDING& Candidate)
		{
			return Candidate.iSkillId == 31470u;
		});
	if (Binding == Bindings.Bindings.end())
	{
		m_strPreviewAnimationStatus =
			"Artist F animation binding row 31470 is unavailable.";
		return false;
	}
	vector<ANIMATION_SKILL_CLIP> StagedClips;
	for (const ANIMATION_SKILL_STAGE& Stage : Binding->Stages)
	{
		StagedClips.insert(
			StagedClips.end(),
			Stage.Clips.begin(), Stage.Clips.end());
	}
	if (StagedClips.empty())
	{
		m_strPreviewAnimationStatus =
			"Artist F animation binding contains no clips.";
		return false;
	}
	const ANIMATION_SKILL_CLIP FirstClip =
		StagedClips.front();
	const bool_t bSingleClip = 1u == StagedClips.size();
	if (!pModel->Start_Animation(
		FirstClip.strClipName.c_str(), bSingleClip && m_bPreviewLoop))
	{
		m_strPreviewAnimationStatus =
			"Artist F first animation clip is unavailable: " +
			FirstClip.strClipName;
		return false;
	}
	m_SynchronizedAnimationClips.assign(
		StagedClips.begin(), StagedClips.end());
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	m_iSynchronizedAnimationTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	pModel->Set_AnimationSpeed(FirstClip.fPlayRate);
	pModel->Set_AnimPaused(true);
	m_strPreviewAnimationStatus = "Artist F animation prepared at zero: " +
		FirstClip.strClipName + " (skill 31470).";
	return true;
}

void Client::CEffect_Tool::Update_ReconstructedDiagnosticRoot()
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedDiagnosticActive())
	{
		m_bReconstructedDiagnosticActive = false;
		return;
	}
	matrix_t CameraWorld = XMLoadFloat4x4(
		CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
	float4x4_t DiagnosticRoot{};
	XMStoreFloat4x4(&DiagnosticRoot,
		XMMatrixTranslation(-2.25f, -0.4f, 4.5f) * CameraWorld);
	pObject->Set_RootWorld(DiagnosticRoot);
	pObject->Set_Visible(true);
}

bool_t Client::CEffect_Tool::Prepare_Artist31470HistoricalPoseBinding(
	const std::shared_ptr<const
		EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation,
	CAnimationHistoricalPoseBinding& OutPoseBinding,
	f32_t& fOutDurationSeconds,
	std::string& strOutError) const
{
	if (nullptr == pPreparation ||
		5u != pPreparation->Get_AnchorRequests().size() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		strOutError =
			"Artist F historical anchor target identity is invalid.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		strOutError = "Artist F historical anchor model is unavailable.";
		return false;
	}
	const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
	if (nullptr == pCurrentClip ||
		m_SynchronizedAnimationClips.front().strClipName != pCurrentClip)
	{
		strOutError = "Artist F historical anchor clip identity changed.";
		return false;
	}

	std::vector<std::string> BoneNames;
	BoneNames.reserve(pPreparation->Get_AnchorRequests().size());
	for (const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding :
		pPreparation->Get_AnchorRequests())
	{
		const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request = Binding.Request;
		if (!Request.bFollow || Request.strRuntimeAnchorSlotId.empty() ||
			Request.strRuntimeBoneName.empty())
		{
			strOutError =
				"Artist F historical anchor request is incomplete: " +
				Binding.strOwnerEmitterId;
			return false;
		}
		BoneNames.push_back(Request.strRuntimeBoneName);
	}

	CAnimationHistoricalPoseBinding StagedBinding;
	if (!CAnimationTargetService::Prepare_HistoricalPoseBinding(
			m_iSynchronizedAnimationTargetGeneration,
			iAnimationIndex, BoneNames, StagedBinding) ||
		StagedBinding.Get_BoneCount() != BoneNames.size())
	{
		strOutError =
			"Artist F historical bone binding could not be prepared.";
		return false;
	}
	f32_t fDurationSeconds = StagedBinding.Get_DurationSeconds();
	const uint32_t iPlayMs = m_SynchronizedAnimationClips.front().iPlayMs;
	if (0u != iPlayMs)
	{
		fDurationSeconds = (std::min)(fDurationSeconds,
			static_cast<f32_t>(iPlayMs) * 0.001f);
	}
	if (!std::isfinite(fDurationSeconds) || fDurationSeconds <= 0.f)
	{
		strOutError = "Artist F historical animation duration is invalid.";
		return false;
	}

	OutPoseBinding = std::move(StagedBinding);
	fOutDurationSeconds = fDurationSeconds;
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Build_Artist31470HistoricalTransformSample(
	const std::shared_ptr<const
		EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation,
	const CAnimationHistoricalPoseBinding& PoseBinding,
	const f32_t fAnimationDurationSeconds,
	const f32_t fEffectSampleTimeSeconds,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
	std::string& strOutError) const
{
	if (nullptr == pPreparation ||
		5u != pPreparation->Get_AnchorRequests().size() ||
		!std::isfinite(fAnimationDurationSeconds) ||
		fAnimationDurationSeconds <= 0.f ||
		!std::isfinite(fEffectSampleTimeSeconds) ||
		fEffectSampleTimeSeconds < 0.f || !PoseBinding.Is_Valid() ||
		PoseBinding.Get_BoneCount() !=
			pPreparation->Get_AnchorRequests().size() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		strOutError =
			"Artist F historical transform sample identity is invalid.";
		return false;
	}

	const f32_t fAnimationEndSeconds = (std::min)(
		fAnimationDurationSeconds, PoseBinding.Get_DurationSeconds());
	const f32_t fAnimationSampleTimeSeconds = std::clamp(
		fEffectSampleTimeSeconds, 0.f, fAnimationEndSeconds);
	ANIMATION_HISTORICAL_POSE_SAMPLE PoseSample;
	if (!CAnimationTargetService::Sample_HistoricalPose(
			PoseBinding, fAnimationSampleTimeSeconds, PoseSample) ||
		PoseSample.BoneCombinedMatrices.size() !=
			pPreparation->Get_AnchorRequests().size())
	{
		strOutError =
			"Artist F historical animation pose sampling failed.";
		return false;
	}

	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
	Staged.RootWorld = PoseSample.RootWorld;
	for (size_t iAnchor = 0u;
		iAnchor < pPreparation->Get_AnchorRequests().size(); ++iAnchor)
	{
		const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding =
			pPreparation->Get_AnchorRequests()[iAnchor];
		const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request = Binding.Request;
		if (!Request.bFollow || Request.strRuntimeAnchorSlotId.empty() ||
			Request.strRuntimeBoneName.empty())
		{
			strOutError =
				"Artist F historical anchor request became invalid: " +
				Binding.strOwnerEmitterId;
			return false;
		}
		EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
		AnchorBuild.RawBone = PoseSample.BoneCombinedMatrices[iAnchor];
		AnchorBuild.OwnerWorld = PoseSample.RootWorld;
		float4x4_t BoneWorld{};
		if (!CEffectPresentationService::Build_SourceBoneAnchorWorld(
				AnchorBuild, BoneWorld))
		{
			strOutError =
				"Artist F historical anchor import transform is invalid: " +
				Request.strRuntimeBoneName;
			return false;
		}

		const auto& Local = Request.SocketLocalTransform;
		const matrix_t SocketLocal = XMMatrixScaling(
			static_cast<f32_t>(Local.vScale[0]),
			static_cast<f32_t>(Local.vScale[1]),
			static_cast<f32_t>(Local.vScale[2])) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(static_cast<f32_t>(Local.vRotationDegrees[0])),
				XMConvertToRadians(static_cast<f32_t>(Local.vRotationDegrees[1])),
				XMConvertToRadians(static_cast<f32_t>(Local.vRotationDegrees[2]))) *
			XMMatrixTranslation(
				static_cast<f32_t>(Local.vPosition[0]),
				static_cast<f32_t>(Local.vPosition[1]),
				static_cast<f32_t>(Local.vPosition[2]));
		float4x4_t AnchorWorld{};
		XMStoreFloat4x4(&AnchorWorld,
			SocketLocal * XMLoadFloat4x4(&BoneWorld));
		const auto [It, bInserted] = Staged.SourceAnchorWorlds.emplace(
			Request.strRuntimeAnchorSlotId, AnchorWorld);
		if (!bInserted && 0 != std::memcmp(
				&It->second, &AnchorWorld, sizeof(float4x4_t)))
		{
			strOutError =
				"Artist F duplicate historical anchor slot disagrees: " +
				Request.strRuntimeAnchorSlotId;
			return false;
		}
	}
	if (Staged.SourceAnchorWorlds.empty())
	{
		strOutError = "Artist F historical anchor map is empty.";
		return false;
	}

	OutSample = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Prepare_ReconstructedSourceRuntimeTransformHistory()
{
	m_strPreviewAnimationStatus.clear();
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		m_strPreviewAnimationStatus =
			"Artist F historical anchor target identity is invalid.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		m_strPreviewAnimationStatus =
			"Artist F historical anchor model is unavailable.";
		return false;
	}
	const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
	if (nullptr == pCurrentClip ||
		m_SynchronizedAnimationClips.front().strClipName != pCurrentClip)
	{
		m_strPreviewAnimationStatus =
			"Artist F historical anchor clip identity changed.";
		return false;
	}

	const auto Preparation = pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == Preparation || 5u != Preparation->Get_AnchorRequests().size())
	{
		m_strPreviewAnimationStatus =
			"Artist F exact five-anchor preparation is unavailable.";
		return false;
	}
	std::string Error;
	CAnimationHistoricalPoseBinding StagedBinding;
	f32_t fDurationSeconds = 0.f;
	if (!Prepare_Artist31470HistoricalPoseBinding(
			Preparation, StagedBinding, fDurationSeconds, Error))
	{
		m_strPreviewAnimationStatus = std::move(Error);
		return false;
	}

	m_ReconstructedSourceRuntimePoseBinding = std::move(StagedBinding);
	m_fPreviewDurationSeconds = fDurationSeconds;
	m_strPreviewAnimationStatus =
		"Artist F historical root plus five ordered anchor samples prepared.";
	return true;
}

bool_t Client::CEffect_Tool::Build_ReconstructedSourceRuntimeTransformSample(
	const f32_t fEffectSampleTimeSeconds,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
	std::string& strOutError) const
{
	if (!m_bReconstructedSourceRuntimeActive ||
		!std::isfinite(fEffectSampleTimeSeconds) ||
		fEffectSampleTimeSeconds < 0.f ||
		!m_ReconstructedSourceRuntimePoseBinding.Is_Valid() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		strOutError =
			"Artist F historical transform sample identity is invalid.";
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	const auto Preparation = nullptr == pObject ? nullptr :
		pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive() ||
		nullptr == Preparation || 5u != Preparation->Get_AnchorRequests().size() ||
		m_ReconstructedSourceRuntimePoseBinding.Get_BoneCount() !=
			Preparation->Get_AnchorRequests().size())
	{
		strOutError =
			"Artist F historical transform preparation was lost.";
		return false;
	}
	return Build_Artist31470HistoricalTransformSample(
		Preparation, m_ReconstructedSourceRuntimePoseBinding,
		m_fPreviewDurationSeconds, fEffectSampleTimeSeconds,
		OutSample, strOutError);
}

bool_t Client::CEffect_Tool::Prepare_ValtanBossPatternTransformHistory(
	const BOSS_PATTERN_EFFECT_BINDING& Binding,
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	m_bValtanBossPatternTransformHistoryRequired = true;
	m_bValtanBossPatternTransformHistoryActive = false;
	m_strValtanBossPatternPreviewEffectAssetId = Document.strEffectAssetId;
	m_strValtanBossPatternAnchorSlotId.clear();
	m_strValtanBossPatternBoneName.clear();
	m_fValtanBossPatternAnimationDurationSeconds = 0.f;
	m_ValtanBossPatternPoseBinding = {};

	if (Binding.strEffectAssetId != Document.strEffectAssetId ||
		Binding.strRuntimeClipName.empty() ||
		Binding.strRuntimeBoneName.empty() ||
		CAnimationTargetService::Resolve_AssetName() != VALTAN_ANIMATION_ASSET_NAME ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		m_SynchronizedAnimationClips.front().strClipName !=
			Binding.strRuntimeClipName ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration())
	{
		strOutError =
			"Valtan 420633 transform history target identity is invalid.";
		return false;
	}

	size_t iVisibleExecutionCount = 0u;
	size_t iFollowCarrierCount = 0u;
	EFFECT_TRANSFORM_DESC SocketLocalTransform{};
	bool_t bHasSocketLocalTransform = false;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.bVisible ||
			!Is_EffectAuthoringExecutionTarget(Element.Material.Execution))
		{
			continue;
		}
		++iVisibleExecutionCount;
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			Element.ActionCueAttachment;
		if (!Attachment.bEnabled || !Attachment.bFollow ||
			Attachment.strRuntimeAnchorSlotId.empty() ||
			Attachment.strRuntimeBoneName != Binding.strRuntimeBoneName)
		{
			strOutError =
				"Valtan 420633 visible carrier lost its exact follow attachment.";
			return false;
		}
		if (m_strValtanBossPatternAnchorSlotId.empty())
		{
			m_strValtanBossPatternAnchorSlotId =
				Attachment.strRuntimeAnchorSlotId;
			m_strValtanBossPatternBoneName = Attachment.strRuntimeBoneName;
			SocketLocalTransform = Attachment.SocketLocalTransform;
			bHasSocketLocalTransform = true;
		}
		else if (m_strValtanBossPatternAnchorSlotId !=
				Attachment.strRuntimeAnchorSlotId ||
			m_strValtanBossPatternBoneName != Attachment.strRuntimeBoneName ||
			0 != std::memcmp(&SocketLocalTransform,
				&Attachment.SocketLocalTransform,
				sizeof(EFFECT_TRANSFORM_DESC)))
		{
			strOutError =
				"Valtan 420633 visible follow carriers disagree on their anchor contract.";
			return false;
		}
		++iFollowCarrierCount;
	}
	if (3u != iVisibleExecutionCount || 3u != iFollowCarrierCount ||
		!bHasSocketLocalTransform ||
		m_strValtanBossPatternAnchorSlotId != "B_EffectRoot" ||
		m_strValtanBossPatternBoneName != "b_effectroot")
	{
		strOutError =
			"Valtan 420633 preview requires exactly three B_EffectRoot follow carriers.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		strOutError = "Valtan 420633 preview model is unavailable.";
		return false;
	}
	const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
	if (nullptr == pCurrentClip || Binding.strRuntimeClipName != pCurrentClip)
	{
		strOutError = "Valtan 420633 preview clip identity changed.";
		return false;
	}

	const std::array<std::string, 1u> BoneNames = {
		m_strValtanBossPatternBoneName };
	CAnimationHistoricalPoseBinding StagedBinding;
	if (!CAnimationTargetService::Prepare_HistoricalPoseBinding(
			m_iSynchronizedAnimationTargetGeneration, iAnimationIndex,
			BoneNames, StagedBinding) || 1u != StagedBinding.Get_BoneCount())
	{
		strOutError =
			"Valtan 420633 b_effectroot historical pose binding failed.";
		return false;
	}
	const f32_t fDurationSeconds = StagedBinding.Get_DurationSeconds();
	if (!std::isfinite(fDurationSeconds) || fDurationSeconds <= 0.f)
	{
		strOutError = "Valtan 420633 animation duration is invalid.";
		return false;
	}

	m_ValtanBossPatternPoseBinding = std::move(StagedBinding);
	m_ValtanBossPatternSocketLocalTransform = SocketLocalTransform;
	m_fValtanBossPatternAnimationDurationSeconds = fDurationSeconds;
	m_bValtanBossPatternTransformHistoryActive = true;
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Build_ValtanBossPatternTransformSample(
	const f32_t fEffectSampleTimeSeconds,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
	std::string& strOutError) const
{
	if (!m_bValtanBossPatternTransformHistoryRequired ||
		!m_bValtanBossPatternTransformHistoryActive ||
		!m_ValtanBossPatternPoseBinding.Is_Valid() ||
		1u != m_ValtanBossPatternPoseBinding.Get_BoneCount() ||
		m_strValtanBossPatternPreviewEffectAssetId.empty() ||
		m_strValtanBossPatternAnchorSlotId != "B_EffectRoot" ||
		m_strValtanBossPatternBoneName != "b_effectroot" ||
		!std::isfinite(fEffectSampleTimeSeconds) ||
		fEffectSampleTimeSeconds < 0.f ||
		!std::isfinite(m_fValtanBossPatternAnimationDurationSeconds) ||
		m_fValtanBossPatternAnimationDurationSeconds <= 0.f ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() != VALTAN_ANIMATION_ASSET_NAME)
	{
		strOutError =
			"Valtan 420633 historical transform sample identity is invalid.";
		return false;
	}

	ANIMATION_HISTORICAL_POSE_SAMPLE PoseSample;
	const f32_t fAnimationSampleTimeSeconds = std::clamp(
		fEffectSampleTimeSeconds, 0.f,
		(std::min)(m_fValtanBossPatternAnimationDurationSeconds,
			m_ValtanBossPatternPoseBinding.Get_DurationSeconds()));
	if (!CAnimationTargetService::Sample_HistoricalPose(
			m_ValtanBossPatternPoseBinding, fAnimationSampleTimeSeconds,
			PoseSample) || 1u != PoseSample.BoneCombinedMatrices.size())
	{
		strOutError =
			"Valtan 420633 b_effectroot historical sampling failed.";
		return false;
	}

	EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
	AnchorBuild.RawBone = PoseSample.BoneCombinedMatrices.front();
	AnchorBuild.OwnerWorld = PoseSample.RootWorld;
	float4x4_t BoneWorld{};
	if (!CEffectPresentationService::Build_SourceBoneAnchorWorld(
			AnchorBuild, BoneWorld))
	{
		strOutError =
			"Valtan 420633 b_effectroot import transform is invalid.";
		return false;
	}

	const EFFECT_TRANSFORM_DESC& Local =
		m_ValtanBossPatternSocketLocalTransform;
	const matrix_t SocketLocal = XMMatrixScaling(
		Local.vScale.x, Local.vScale.y, Local.vScale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Local.vRotationDegrees.x),
			XMConvertToRadians(Local.vRotationDegrees.y),
			XMConvertToRadians(Local.vRotationDegrees.z)) *
		XMMatrixTranslation(
			Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
	float4x4_t AnchorWorld{};
	XMStoreFloat4x4(&AnchorWorld,
		SocketLocal * XMLoadFloat4x4(&BoneWorld));

	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
	Staged.RootWorld = PoseSample.RootWorld;
	Staged.SourceAnchorWorlds.emplace(
		m_strValtanBossPatternAnchorSlotId, AnchorWorld);
	OutSample = std::move(Staged);
	strOutError.clear();
	return true;
}

void Client::CEffect_Tool::Reset_ValtanBossPatternTransformHistory()
{
	m_ValtanBossPatternPoseBinding = {};
	m_ValtanBossPatternSocketLocalTransform = {};
	m_strValtanBossPatternPreviewEffectAssetId.clear();
	m_strValtanBossPatternAnchorSlotId.clear();
	m_strValtanBossPatternBoneName.clear();
	m_fValtanBossPatternAnimationDurationSeconds = 0.f;
	m_bValtanBossPatternTransformHistoryRequired = false;
	m_bValtanBossPatternTransformHistoryActive = false;
}

bool_t Client::CEffect_Tool::Update_ReconstructedSourceRuntimeTimeline(
	const f32_t fTimeDelta)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !m_bReconstructedSourceRuntimeActive)
	{
		m_bReconstructedSourceRuntimeActive = false;
		Reset_ReconstructedSourceRuntimeTimeline();
		return false;
	}
	const auto FailPreview = [this, &pObject](const std::string& Reason)
	{
		pObject->Set_Playing(false);
		pObject->Set_Visible(false);
		Set_SynchronizedAnimationPaused(true);
		Reset_SynchronizedAnimationSequence();
		Reset_ReconstructedSourceRuntimeTimeline();
		m_bReconstructedSourceRuntimeActive = false;
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = Reason;
		return false;
	};
	if (pObject->Is_RenderFailureIsolated())
		return FailPreview(pObject->Get_Status());
	if (!pObject->Is_ReconstructedSourceRuntimeActive())
	{
		m_bReconstructedSourceRuntimeActive = false;
		Reset_ReconstructedSourceRuntimeTimeline();
		return false;
	}
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this](const f32_t fSampleTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strOutError)
		{
			return Build_ReconstructedSourceRuntimeTransformSample(
				fSampleTimeSeconds, OutSample, strOutError);
		};

	/* Tool previews are clocked only from the bound animation.  Keeping the
	   Object's autonomous update disabled prevents the same wall-clock delta
	   from being consumed by both the Level and the Effect Tool. */
	pObject->Set_Playing(false);
	if (m_bReconstructedSourceRuntimeStartPending)
	{
		/* Force the model's zero pose to refresh without exposing an unpaused
		   Engine update to the cache-build delta. */
		Seek_SynchronizedAnimationSequence(0.f);
		Set_SynchronizedAnimationPaused(true);
		f32_t fVisibleAnimationTimeSeconds = 0.f;
		if (!Try_ResolveSynchronizedAnimationTime(
				fVisibleAnimationTimeSeconds) ||
			std::abs(fVisibleAnimationTimeSeconds) > 1.0e-5f)
		{
			return FailPreview(
				"Artist F visible animation could not commit synchronized time zero.");
		}
		/* The expensive prepared-cache frame has already passed through Engine
		   update while both participants were paused.  Publish an exact zero frame
		   before allowing the animation clock to advance. */
		std::string Error;
		if (!pObject->Set_SampleTimeWithTransformHistory(
				0.f, TransformProvider, Error))
		{
			return FailPreview(
				"Artist F zero-frame history sample failed: " + Error);
		}
		if (std::abs(pObject->Get_PreviewFixedStepClockSeconds()) > 1.0e-6)
		{
			return FailPreview(
				"Artist F zero-frame effect clock diverged from animation time zero.");
		}
		m_fPreviewTimeSeconds = 0.f;
		m_fReconstructedSourceRuntimeClockSeconds = 0.f;
		m_bReconstructedSourceRuntimeStartPending = false;
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
	m_strPreviewStatus = m_bPreviewPlaying ?
			"Artist Core F (33) is running from synchronized time zero; Product remains OFF." :
			"Artist Core F (33) is paused at synchronized time zero; Product remains OFF.";
		return true;
	}
	if (m_bReconstructedSourceRuntimeNaturalTailActive)
	{
		const f32_t fTailDelta = std::isfinite(fTimeDelta) ?
			std::clamp(fTimeDelta, 0.f, 0.1f) : 0.f;
		if (m_bPreviewPlaying && fTailDelta > 0.f)
		{
			std::string Error;
			if (!pObject->Advance_PreviewWithTransformHistory(
					fTailDelta, TransformProvider, Error))
			{
				return FailPreview(
					"Artist F Natural Stop anchor history failed: " + Error);
			}
			m_fReconstructedSourceRuntimeTailSeconds += fTailDelta;
		}
		if (pObject->Is_Finished())
		{
			m_bReconstructedSourceRuntimeNaturalTailActive = false;
			m_bPreviewPlaying = false;
			m_strPreviewStatus =
				"Artist Core F (33) Natural Stop tail completed; Product remains OFF.";
		}
		return true;
	}

	f32_t fAnimationTimeSeconds = 0.f;
	if (!Try_ResolveSynchronizedAnimationTime(fAnimationTimeSeconds))
	{
		return FailPreview(
			"Artist F preview stopped: synchronized animation time is unavailable.");
	}
	fAnimationTimeSeconds = std::clamp(
		fAnimationTimeSeconds, 0.f, m_fPreviewDurationSeconds);
	const f32_t fPreviousTimeSeconds =
		m_fReconstructedSourceRuntimeClockSeconds;
	const bool_t bTimelineWrapped =
		fAnimationTimeSeconds + 0.0001f < fPreviousTimeSeconds;

	std::string Error;
	if (bTimelineWrapped)
	{
		if (!pObject->Set_SampleTimeWithTransformHistory(
				fAnimationTimeSeconds, TransformProvider, Error))
		{
			return FailPreview(
				"Artist F loop history replay failed: " + Error);
		}
	}
	else if (m_bPreviewPlaying &&
		fAnimationTimeSeconds > fPreviousTimeSeconds)
	{
		if (!pObject->Advance_PreviewWithTransformHistory(
				fAnimationTimeSeconds - fPreviousTimeSeconds,
				TransformProvider, Error))
		{
			return FailPreview(
				"Artist F fixed-step anchor history failed: " + Error);
		}
	}
	const f64_t fEffectClockSeconds =
		pObject->Get_PreviewFixedStepClockSeconds();
	if (!std::isfinite(fEffectClockSeconds) ||
		std::abs(fEffectClockSeconds -
			static_cast<f64_t>(fAnimationTimeSeconds)) > 1.0e-5)
	{
		return FailPreview(
			"Artist F effect clock diverged from its animation clock.");
	}
	m_fPreviewTimeSeconds = fAnimationTimeSeconds;
	m_fReconstructedSourceRuntimeClockSeconds = fAnimationTimeSeconds;

	if (m_bPreviewPlaying && !m_bPreviewLoop &&
		fAnimationTimeSeconds + 0.0001f >= m_fPreviewDurationSeconds)
	{
		Set_SynchronizedAnimationPaused(true);
		m_bReconstructedSourceRuntimeNaturalTailActive =
			!pObject->Is_Finished();
		m_fReconstructedSourceRuntimeTailSeconds = 0.f;
		if (m_bReconstructedSourceRuntimeNaturalTailActive)
		{
			m_strPreviewStatus =
				"Artist F animation reached its synchronized end; Natural Stop particle tail is running.";
		}
		else
		{
			m_bPreviewPlaying = false;
			m_strPreviewStatus =
				"Artist Core F (33) reached its synchronized end and completed; Product remains OFF.";
		}
	}
	return true;
}

bool_t Client::CEffect_Tool::Seek_ReconstructedSourceRuntimeTimeline(
	const f32_t fSampleTimeSeconds)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !m_bReconstructedSourceRuntimeActive ||
		!std::isfinite(fSampleTimeSeconds))
	{
		return false;
	}
	const f32_t fClampedTime = std::clamp(
		fSampleTimeSeconds, 0.f, m_fPreviewDurationSeconds);
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this](const f32_t fHistoryTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strOutError)
		{
			return Build_ReconstructedSourceRuntimeTransformSample(
				fHistoryTimeSeconds, OutSample, strOutError);
		};
	std::string Error;
	/* History replay is validated and committed before the visible model seek.
	   Internal fixed steps never seek or mutate the live animation cursor. */
	if (!pObject->Set_SampleTimeWithTransformHistory(
			fClampedTime, TransformProvider, Error))
	{
		m_strPreviewStatus =
			"Artist F synchronized history seek failed: " + Error;
		return false;
	}
	if (std::abs(pObject->Get_PreviewFixedStepClockSeconds() -
			static_cast<f64_t>(fClampedTime)) > 1.0e-5)
	{
		m_strPreviewStatus =
			"Artist F synchronized history seek produced a divergent effect clock.";
		return false;
	}
	m_bPreviewPlaying = false;
	m_bReconstructedSourceRuntimeStartPending = false;
	m_bReconstructedSourceRuntimeNaturalTailActive = false;
	m_fReconstructedSourceRuntimeTailSeconds = 0.f;
	m_fPreviewTimeSeconds = fClampedTime;
	m_fReconstructedSourceRuntimeClockSeconds = fClampedTime;
	pObject->Set_Playing(false);
	Seek_SynchronizedAnimationSequence(fClampedTime);
	Set_SynchronizedAnimationPaused(true);
	f32_t fVisibleAnimationTimeSeconds = 0.f;
	if (!Try_ResolveSynchronizedAnimationTime(
			fVisibleAnimationTimeSeconds) ||
		std::abs(fVisibleAnimationTimeSeconds - fClampedTime) > 1.0e-4f)
	{
		pObject->Set_Visible(false);
		Reset_SynchronizedAnimationSequence();
		Reset_ReconstructedSourceRuntimeTimeline();
		m_bReconstructedSourceRuntimeActive = false;
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus =
			"Artist F visible animation seek diverged from the effect clock.";
		return false;
	}
	pObject->Set_Visible(m_bPreviewVisibleRequested);
	m_strPreviewStatus =
		"Artist Core F (33) sampled from the synchronized animation clock; Product remains OFF.";
	return true;
}

void Client::CEffect_Tool::Reset_ReconstructedSourceRuntimeTimeline()
{
	m_bReconstructedSourceRuntimeStartPending = false;
	m_bReconstructedSourceRuntimeNaturalTailActive = false;
	m_fReconstructedSourceRuntimeClockSeconds = 0.f;
	m_fReconstructedSourceRuntimeTailSeconds = 0.f;
}

bool_t Client::CEffect_Tool::Stage_WorldPreview()
{
	if (m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value())
		return Stage_WorldPreview(*m_SourcePreviewDocument, true);
    return m_ActiveDocument.has_value() ?
		Stage_WorldPreview(*m_ActiveDocument) : false;
}

bool_t Client::CEffect_Tool::Stage_WorldPreview(
    const EFFECT_DOCUMENT_DESC& Document)
{
	return Stage_WorldPreview(Document, false);
}

bool_t Client::CEffect_Tool::Stage_WorldPreview(
	const EFFECT_DOCUMENT_DESC& Document,
	const bool_t bAllowReadOnlySourceProjection)
{
	const EFFECT_DOCUMENT_DESC PreviewDocument =
		Build_PreviewDocument(Document);
    if (!Ensure_WorldPreviewObject())
        return false;
    shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    std::string Error;
	const std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		pCatalogProjection = CEffectCatalog::Find_VisualProjection(
			PreviewDocument.strEffectAssetId);
	const bool_t bReadOnlySourcePreview = bAllowReadOnlySourceProjection &&
		m_ProductPreview.has_value() &&
		m_SourcePreviewDocument.has_value() &&
		Document.strEffectAssetId ==
			m_SourcePreviewDocument->strEffectAssetId &&
		CEffectDocumentCodec::Serialize(Document) ==
			CEffectDocumentCodec::Serialize(*m_SourcePreviewDocument);
	if (bAllowReadOnlySourceProjection && !bReadOnlySourcePreview)
	{
		m_strPreviewStatus =
			"Read-only source preview identity changed before staging.";
		return false;
	}
	const bool_t bExactVisualProjection = nullptr != pCatalogProjection &&
		bReadOnlySourcePreview &&
		m_ePreviewFilter == EFFECT_PREVIEW_FILTER::COMPLETE &&
		CEffectDocumentCodec::Serialize(PreviewDocument) ==
			CEffectDocumentCodec::Serialize(pCatalogProjection->Get_Document());
	std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT>
		pVisualPrepared;
	const bool_t bVisualPrepared = !bExactVisualProjection ||
		CEffectDocumentRenderer::Prepare_VisualProgramDocument(
			m_pDevice, m_pContext, pCatalogProjection,
			pVisualPrepared, Error);
	bool_t bStaged = nullptr != pObject && bVisualPrepared &&
		(bExactVisualProjection ?
			pObject->Stage_PrevalidatedVisualProgramDocument(
				pCatalogProjection, pVisualPrepared, Error) :
			pObject->Stage_Document(PreviewDocument, Error));
	if (!bStaged)
    {
        m_strPreviewStatus = "Document is editable but not drawable yet: " + Error;
        return false;
    }
	m_pVisualPreviewProjection = bExactVisualProjection ?
		pCatalogProjection : nullptr;
	m_bReconstructedDiagnosticActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	Reset_ReconstructedSourceRuntimeTimeline();
	pObject->Set_Playing(false);
	const f32_t fEffectSampleSeconds =
		Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
	std::string HistoryError;
	const bool_t bHistorySampled =
		!m_bValtanBossPatternTransformHistoryRequired &&
		Seek_WorldPreviewWithSourceAnchorHistory(
			pObject, PreviewDocument, fEffectSampleSeconds, HistoryError);
	if (!bHistorySampled)
		pObject->Set_SampleTime(fEffectSampleSeconds);
    switch (m_ePreviewFilter)
    {
    case EFFECT_PREVIEW_FILTER::COMPLETE:
        m_strPreviewStatus =
            "Complete Effect preview committed from the active Document.";
        break;
    case EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM:
        m_strPreviewStatus =
            "All Particle subtypes preview committed.";
        break;
	case EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES:
		m_strPreviewStatus =
			"Standalone Mesh-only preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS:
		m_strPreviewStatus =
			"Mesh Particle-only preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_SPRITES:
		m_strPreviewStatus =
			"Standalone Sprite-only preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS:
		m_strPreviewStatus =
			"Sprite Particle-only preview committed.";
		break;
    case EFFECT_PREVIEW_FILTER::SOLO_SELECTED:
        m_strPreviewStatus = "Selected Element Solo preview committed.";
        break;
	case EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE:
		m_strPreviewStatus = "Selected Model / Summon Solo preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES:
		m_strPreviewStatus = "Model / Summon Family preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY:
		m_strPreviewStatus = "Selected authoring Family preview committed.";
		break;
    case EFFECT_PREVIEW_FILTER::MUTE_SELECTED:
        m_strPreviewStatus = "Selected Element Mute preview committed.";
        break;
    case EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP:
        m_strPreviewStatus = "Selected Element Group Solo preview committed.";
        break;
    case EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP:
        m_strPreviewStatus = "Selected Element Group Mute preview committed.";
        break;
    case EFFECT_PREVIEW_FILTER::END:
	default:
		m_strPreviewStatus = "Effect preview committed.";
		break;
	}
	if (!HistoryError.empty())
	{
		m_strPreviewStatus +=
			" Source-anchor history used current-pose fallback: " + HistoryError;
	}
	return true;
}

Client::EFFECT_DOCUMENT_DESC
Client::CEffect_Tool::Build_PreviewDocument(
	const EFFECT_DOCUMENT_DESC& Document) const
{
    EFFECT_DOCUMENT_DESC Preview = Document;
	if (!m_bPreviewScreenPostEnabled)
	{
		std::erase_if(Preview.Elements,
			[](const EFFECT_ELEMENT_DESC& Element)
			{
				return EFFECT_ELEMENT_KIND::SCREEN_POST == Element.eKind;
			});
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE == m_ePreviewFilter)
	{
		for (EFFECT_ELEMENT_DESC& Element : Preview.Elements)
			Element.bVisible = false;
		for (EFFECT_MODEL_CUE_DESC& Cue : Preview.ModelCues)
		{
			Cue.bVisible = Cue.bVisible &&
				Cue.strCueId == m_strPreviewIsolationModelCueId;
		}
		return Preview;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES == m_ePreviewFilter)
	{
		for (EFFECT_ELEMENT_DESC& Element : Preview.Elements)
			Element.bVisible = false;
		return Preview;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY == m_ePreviewFilter)
	{
		std::erase_if(Preview.Elements,
			[this](const EFFECT_ELEMENT_DESC& Element)
			{
				return Resolve_AuthoringFamily(Element) !=
					m_ePreviewIsolationAuthoringFamily;
			});
		for (EFFECT_MODEL_CUE_DESC& Cue : Preview.ModelCues)
			Cue.bVisible = false;
		return Preview;
	}
    if (EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM == m_ePreviewFilter)
    {
        std::erase_if(Preview.Elements,
            [](const EFFECT_ELEMENT_DESC& Element)
            {
                return EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind;
            });
        Preview.ModelCues.clear();
        return Preview;
    }
	if (EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_SPRITES == m_ePreviewFilter)
	{
		const EFFECT_ELEMENT_KIND eRequired =
			EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES == m_ePreviewFilter ?
				EFFECT_ELEMENT_KIND::MESH : EFFECT_ELEMENT_KIND::SPRITE;
		std::erase_if(Preview.Elements,
			[eRequired](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.eKind != eRequired;
			});
		Preview.ModelCues.clear();
		return Preview;
	}
	if (EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS == m_ePreviewFilter)
	{
		const CASCADE_RENDERER_KIND eRequired =
			EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS == m_ePreviewFilter ?
				CASCADE_RENDERER_KIND::MESH :
				CASCADE_RENDERER_KIND::SPRITE;
		std::erase_if(Preview.Elements,
			[eRequired](const EFFECT_ELEMENT_DESC& Element)
			{
				return EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind ||
					Resolve_CascadeRendererKind(Element) != eRequired;
			});
		Preview.ModelCues.clear();
		return Preview;
	}
    if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter)
    {
		if (m_strPreviewIsolationGroupId.empty())
            return Preview;
        const bool_t bGroupExists = std::any_of(
            Preview.Elements.begin(), Preview.Elements.end(),
			[this](const EFFECT_ELEMENT_DESC& Element)
            {
				return Element.strGroupId == m_strPreviewIsolationGroupId;
            });
        if (!bGroupExists)
            return Preview;
        std::erase_if(Preview.Elements,
            [this](const EFFECT_ELEMENT_DESC& Element)
            {
				const bool_t bSelectedGroup =
					Element.strGroupId == m_strPreviewIsolationGroupId;
                return EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP ==
                    m_ePreviewFilter ? !bSelectedGroup : bSelectedGroup;
            });
        if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter)
            Preview.ModelCues.clear();
        return Preview;
    }
    if (EFFECT_PREVIEW_FILTER::COMPLETE == m_ePreviewFilter ||
		m_strPreviewIsolationElementId.empty())
        return Preview;
    const bool_t bSelectionExists = std::any_of(
        Preview.Elements.begin(), Preview.Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
        {
			return Element.strElementId == m_strPreviewIsolationElementId;
        });
    if (!bSelectionExists)
        return Preview;
    std::erase_if(Preview.Elements,
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
			const bool_t bSelected =
				Element.strElementId == m_strPreviewIsolationElementId;
            return EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter ?
                !bSelected : bSelected;
        });
    if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED == m_ePreviewFilter)
        Preview.ModelCues.clear();
    return Preview;
}

bool_t Client::CEffect_Tool::Stage_ParticleSystemDraftPreview()
{
    if (!m_ActiveDocument.has_value() ||
        !m_ParticleSystemDraft.has_value())
    {
        return false;
    }

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (!Apply_ParticleSystemDraft(Staged))
    {
        m_strPreviewStatus =
            "Live Particle System preview rejected: draft is missing.";
        return false;
    }
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    Recalculate_PreviewDuration(Staged);
    if (!Stage_WorldPreview(Staged))
    {
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_fPreviewTimeSeconds = fPreviousTime;
        return false;
    }
    m_strPreviewStatus =
        "Live Particle System draft staged; Apply commits it to the active Document.";
    return true;
}

bool_t Client::CEffect_Tool::Stage_DetailDraftPreview()
{
    if (!m_ActiveDocument.has_value() || !m_DetailDraft.has_value())
        return false;

    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    if (!Apply_DetailDraft(Staged))
    {
        m_strPreviewStatus =
            "Live Detail preview rejected: selected Element is missing.";
        return false;
    }

    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    Recalculate_PreviewDuration(Staged);
    if (!Stage_WorldPreview(Staged))
    {
        m_fPreviewDurationSeconds = fPreviousDuration;
        m_fPreviewTimeSeconds = fPreviousTime;
        return false;
    }
    m_strPreviewStatus =
        "Live Detail draft staged; Apply Detail commits it to the active Document.";
    return true;
}

bool_t Client::CEffect_Tool::Stage_ModelCueDraftPreview()
{
	if (!m_ActiveDocument.has_value() || !m_ModelCueDraft.has_value())
		return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	if (!Apply_ModelCueDraft(Staged))
	{
		m_strPreviewStatus =
			"Live Model Cue preview rejected: selected Cue is missing.";
		return false;
	}
	const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
	const f32_t fPreviousTime = m_fPreviewTimeSeconds;
	Recalculate_PreviewDuration(Staged);
	if (!Stage_WorldPreview(Staged))
	{
		m_fPreviewDurationSeconds = fPreviousDuration;
		m_fPreviewTimeSeconds = fPreviousTime;
		return false;
	}
	m_strPreviewStatus =
		"Live Model Cue draft staged; Apply commits it to the active Document.";
	return true;
}

bool_t Client::CEffect_Tool::Apply_ParticleSystemDraft(
    EFFECT_DOCUMENT_DESC& Document) const
{
    if (!m_ParticleSystemDraft.has_value())
        return false;
    Document.ParticleSystem = *m_ParticleSystemDraft;
    return true;
}

bool_t Client::CEffect_Tool::Apply_DetailDraft(
    EFFECT_DOCUMENT_DESC& Document) const
{
    if (!m_DetailDraft.has_value())
        return false;
    for (EFFECT_ELEMENT_DESC& Element : Document.Elements)
    {
        if (Element.strElementId != m_strDetailDraftElementId)
            continue;
        Apply_EffectElementDetailDraft(Element, *m_DetailDraft);
		if (m_bDetailDraftCapabilityDeferred ||
			!Is_EffectAuthoringExecutionTarget(
				Element.Material.Execution))
			Element.bVisible = false;
        return true;
    }
    return false;
}

bool_t Client::CEffect_Tool::Apply_ModelCueDraft(
	EFFECT_DOCUMENT_DESC& Document) const
{
	if (!m_ModelCueDraft.has_value())
		return false;
	for (EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
	{
		if (Cue.strCueId != m_ModelCueDraft->strCueId)
			continue;
		Cue = *m_ModelCueDraft;
		return true;
	}
	return false;
}

bool_t Client::CEffect_Tool::Resolve_PreviewRoot(float4x4_t& OutRoot)
{
	const bool_t bPlayerSnapshot = m_ProductPreview.has_value() &&
		EFFECT_FOLLOW_POLICY::SNAPSHOT ==
			m_ProductPreview->ProductCue.Cue.eFollowPolicy;
	const bool_t bValtanSnapshot = m_ValtanProductPreview.has_value() &&
		EFFECT_FOLLOW_POLICY::SNAPSHOT ==
			m_ValtanProductPreview->Cue.eFollowPolicy;
	const bool_t bPlayerActionFacing = m_ProductPreview.has_value() &&
		EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
			m_ProductPreview->ProductCue.Cue.eOrientationPolicy;
    if ((bPlayerSnapshot || bValtanSnapshot) &&
		m_bProductCueSnapshotCaptured)
    {
        OutRoot = m_ProductCueSnapshotRoot;
        return true;
    }

    float4x4_t Anchor{};
    bool_t bResolved = false;
    if (m_ProductPreview.has_value())
    {
        const std::string& strCueAnchor =
            m_ProductPreview->ProductCue.Cue.strAnchorSlotId;
        bResolved = "root" == strCueAnchor ?
            CAnimationTargetService::Resolve_RootTransform(&Anchor) :
            CAnimationTargetService::Resolve_AnchorTransform(
                strCueAnchor.c_str(), &Anchor);
    }
	else if (m_ValtanProductPreview.has_value())
	{
		const std::string& strCueAnchor =
			m_ValtanProductPreview->Cue.strAnchorSlotId;
		bResolved = "root" == strCueAnchor ?
			CAnimationTargetService::Resolve_RootTransform(&Anchor) :
			CAnimationTargetService::Resolve_AnchorTransform(
				strCueAnchor.c_str(), &Anchor);
	}
    else
    {
        switch (m_ePreviewPivotKind)
        {
        case EFFECT_PREVIEW_PIVOT_KIND::WORLD:
            Anchor = m_PreviewWorldRoot;
            bResolved = true;
            break;
        case EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT:
            bResolved = CAnimationTargetService::Resolve_RootTransform(&Anchor);
            break;
        case EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET:
        case EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE:
            bResolved = CAnimationTargetService::Resolve_AnchorTransform(
                m_strPreviewAnchorSlotId.c_str(), &Anchor);
            break;
        case EFFECT_PREVIEW_PIVOT_KIND::END:
        default:
            return false;
        }
    }
    if (!bResolved)
        return false;
	if (!Has_ProductCuePreview())
    {
        OutRoot = Anchor;
        return true;
    }

	if (m_ProductPreview.has_value())
	{
		const ANIMATION_EFFECT_CUE& Cue =
			m_ProductPreview->ProductCue.Cue;
		if (bPlayerActionFacing && "root" != Cue.strAnchorSlotId)
			return false;
		if (bPlayerActionFacing && !m_bProductCueActionFacingCaptured)
		{
			if (!Try_ExtractPlanarYawDegrees(
				Anchor, m_fProductCueActionFacingYawDegrees))
			{
				return false;
			}
			m_bProductCueActionFacingCaptured = true;
		}
		if (!CAnimationEffectCueDocument::Try_ComposeRootTransform(
			Cue.LocalTransform, Anchor, Cue.eOrientationPolicy,
			m_fProductCueActionFacingYawDegrees, OutRoot))
		{
			return false;
		}
	}
	else
	{
		OutRoot = Compose_EffectLocal(
			m_ValtanProductPreview->Cue.LocalTransform, Anchor);
	}
	if ((bPlayerSnapshot || bValtanSnapshot) &&
		Is_ProductCueVisible(m_fPreviewTimeSeconds))
    {
        m_ProductCueSnapshotRoot = OutRoot;
        m_bProductCueSnapshotCaptured = true;
    }
    return true;
}

bool_t Client::CEffect_Tool::Has_ProductCuePreview() const
{
	return m_ProductPreview.has_value() ||
		m_ValtanProductPreview.has_value();
}

f32_t Client::CEffect_Tool::Resolve_EffectSampleTime(
    const f32_t fTimelineSeconds) const
{
	if (m_ProductPreview.has_value())
	{
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		return CActionPresentationTimeline::Resolve_CuePreviewSample(
			Timing, (std::max)(0.f, fTimelineSeconds), Sample) ?
			Sample.fEffectSampleSeconds : 0.f;
	}
	if (!m_ValtanProductPreview.has_value())
        return (std::max)(0.f, fTimelineSeconds);

	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
	Timing.fCueSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd =
		m_ValtanProductPreview->Cue.bHasSourceEnd;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
	return CActionPresentationTimeline::Resolve_CuePreviewSample(
		Timing, (std::max)(0.f, fTimelineSeconds), Sample) ?
		Sample.fEffectSampleSeconds : 0.f;
}

f32_t Client::CEffect_Tool::Resolve_EffectTimelineTime(
	const f32_t fEffectSampleSeconds) const
{
	const f32_t fClampedEffectSample =
		(std::max)(0.f, fEffectSampleSeconds);
	if (m_ProductPreview.has_value())
	{
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		if (!CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.f, Sample))
		{
			return fClampedEffectSample;
		}
		return Sample.fCueWallStartSeconds +
			fClampedEffectSample / Timing.fPlayRate;
	}
	if (!m_ValtanProductPreview.has_value())
		return fClampedEffectSample;

	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
	Timing.fCueSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd =
		m_ValtanProductPreview->Cue.bHasSourceEnd;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
	if (!CActionPresentationTimeline::Resolve_CuePreviewSample(
			Timing, 0.f, Sample))
	{
		return fClampedEffectSample;
	}
	return Sample.fCueWallStartSeconds +
		fClampedEffectSample / Timing.fPlayRate;
}

bool_t Client::CEffect_Tool::Seek_WorldPreviewWithSourceAnchorHistory(
	const std::shared_ptr<CEffectObject>& pObject,
	const EFFECT_DOCUMENT_DESC& Document,
	const f32_t fEffectSampleSeconds,
	std::string& strOutError)
{
	strOutError.clear();
	if (nullptr == pObject || !std::isfinite(fEffectSampleSeconds) ||
		fEffectSampleSeconds < 0.f)
	{
		strOutError = "Effect history seek received an invalid sample request.";
		return false;
	}
	const bool_t bStableRootPreview = m_ProductPreview.has_value() ?
		"root" == m_ProductPreview->ProductCue.Cue.strAnchorSlotId :
		m_ValtanProductPreview.has_value() ?
			"root" == m_ValtanProductPreview->Cue.strAnchorSlotId :
			(EFFECT_PREVIEW_PIVOT_KIND::WORLD == m_ePreviewPivotKind ||
			 EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind);
	if (!bStableRootPreview)
	{
		strOutError =
			"Source-anchor history currently requires a stable root Effect pivot.";
		return false;
	}

	float4x4_t EffectRoot{};
	if (!Resolve_PreviewRoot(EffectRoot))
	{
		strOutError = "Effect history seek could not resolve its preview root.";
		return false;
	}
	const std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Requests =
		Collect_ToolSourceAnchorRequests(Document);
	if (Requests.empty())
	{
		pObject->Set_SourceAnchorWorlds({});
		pObject->Set_RootWorld(EffectRoot);
		pObject->Set_SampleTime(fEffectSampleSeconds);
		return true;
	}

	/* HistoricalPoseBinding samples one current clip without mutating the live
	   animation cursor. Multi-clip reconstruction needs a separate sequence
	   binding and deliberately remains outside this narrow Tool seek path. */
	if (1u != m_SynchronizedAnimationClips.size() ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration())
	{
		strOutError =
			"Source-anchor history requires one synchronized animation clip.";
		return false;
	}
	const SYNCHRONIZED_ANIMATION_CLIP Clip =
		m_SynchronizedAnimationClips.front();
	if (!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
	{
		strOutError = "Source-anchor history has an invalid animation play rate.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		strOutError = "Source-anchor history has no animation model.";
		return false;
	}
	const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
	if (nullptr == pCurrentClip || Clip.strClipName != pCurrentClip)
	{
		strOutError = "Source-anchor history clip identity changed.";
		return false;
	}

	std::vector<std::string> BoneNames;
	BoneNames.reserve(Requests.size());
	for (const TOOL_SOURCE_ANCHOR_REQUEST& Request : Requests)
		BoneNames.push_back(Request.strRuntimeBoneName);
	CAnimationHistoricalPoseBinding PoseBinding;
	if (!CAnimationTargetService::Prepare_HistoricalPoseBinding(
			m_iSynchronizedAnimationTargetGeneration, iAnimationIndex,
			BoneNames, PoseBinding) ||
		PoseBinding.Get_BoneCount() != Requests.size())
	{
		strOutError =
			"Source-anchor historical pose binding could not be prepared.";
		return false;
	}

	const f32_t fSourceStartSeconds =
		static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
	f32_t fSourceDurationSeconds =
		PoseBinding.Get_DurationSeconds() - fSourceStartSeconds;
	if (0u != Clip.iPlayMs)
	{
		fSourceDurationSeconds = (std::min)(fSourceDurationSeconds,
			static_cast<f32_t>(Clip.iPlayMs) * 0.001f);
	}
	if (!std::isfinite(fSourceStartSeconds) || fSourceStartSeconds < 0.f ||
		!std::isfinite(fSourceDurationSeconds) ||
		fSourceDurationSeconds <= 0.f)
	{
		strOutError = "Source-anchor history has an invalid source segment.";
		return false;
	}
	const f32_t fWallDurationSeconds =
		fSourceDurationSeconds / Clip.fPlayRate;
	if (!std::isfinite(fWallDurationSeconds) ||
		fWallDurationSeconds <= 0.f)
	{
		strOutError = "Source-anchor history has an invalid wall duration.";
		return false;
	}

	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this, &PoseBinding, &Requests, EffectRoot, Clip,
		 fSourceStartSeconds, fSourceDurationSeconds,
		 fWallDurationSeconds](const f32_t fHistoryEffectSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strProviderError)
		{
			f32_t fWallSeconds = (std::max)(
				0.f, Resolve_EffectTimelineTime(fHistoryEffectSeconds));
			if (Clip.bHasExplicitLoopPolicy && Clip.bLoop)
				fWallSeconds = std::fmod(fWallSeconds, fWallDurationSeconds);
			else
				fWallSeconds = (std::min)(fWallSeconds, fWallDurationSeconds);
			const f32_t fAnimationSeconds = fSourceStartSeconds +
				(std::min)(fSourceDurationSeconds,
					fWallSeconds * Clip.fPlayRate);

			ANIMATION_HISTORICAL_POSE_SAMPLE PoseSample;
			if (!CAnimationTargetService::Sample_HistoricalPose(
					PoseBinding, fAnimationSeconds, PoseSample) ||
				PoseSample.BoneCombinedMatrices.size() != Requests.size())
			{
				strProviderError =
					"Source-anchor historical bone sampling failed.";
				return false;
			}

			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
			Staged.RootWorld = EffectRoot;
			Staged.SourceAnchorWorlds.reserve(Requests.size());
			for (size_t iRequest = 0u; iRequest < Requests.size(); ++iRequest)
			{
				const TOOL_SOURCE_ANCHOR_REQUEST& Request = Requests[iRequest];
				float4x4_t BoneWorld{};
				XMStoreFloat4x4(&BoneWorld,
					XMLoadFloat4x4(&PoseSample.BoneCombinedMatrices[iRequest]) *
					XMLoadFloat4x4(&PoseSample.RootWorld));
				const EFFECT_TRANSFORM_DESC& Local =
					Request.SocketLocalTransform;
				const matrix_t SocketLocal = XMMatrixScaling(
					Local.vScale.x, Local.vScale.y, Local.vScale.z) *
					XMMatrixRotationRollPitchYaw(
						XMConvertToRadians(Local.vRotationDegrees.x),
						XMConvertToRadians(Local.vRotationDegrees.y),
						XMConvertToRadians(Local.vRotationDegrees.z)) *
					XMMatrixTranslation(
						Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
				float4x4_t AnchorWorld{};
				XMStoreFloat4x4(&AnchorWorld,
					SocketLocal * XMLoadFloat4x4(&BoneWorld));
				Staged.SourceAnchorWorlds.emplace(
					Request.strRuntimeAnchorSlotId, AnchorWorld);
			}
			if (Staged.SourceAnchorWorlds.size() != Requests.size())
			{
				strProviderError =
					"Source-anchor history produced duplicate runtime slots.";
				return false;
			}
			OutSample = std::move(Staged);
			strProviderError.clear();
			return true;
		};
	return pObject->Set_SampleTimeWithTransformHistory(
		fEffectSampleSeconds, TransformProvider, strOutError);
}

bool_t Client::CEffect_Tool::Is_ProductCueVisible(
    const f32_t fTimelineSeconds) const
{
	if (m_ProductPreview.has_value())
	{
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		return CActionPresentationTimeline::Resolve_CuePreviewSample(
			Timing, (std::max)(0.f, fTimelineSeconds), Sample) &&
			Sample.bVisible;
	}
	if (!m_ValtanProductPreview.has_value())
        return true;
	if (0u == m_ValtanProductPreview->Cue.iStageDurationMs ||
		fTimelineSeconds * 1000.f + 0.5f >= static_cast<f32_t>(
			m_ValtanProductPreview->Cue.iStageDurationMs))
	{
		return false;
	}

	ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
	Timing.fClipSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
	Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
	Timing.fCueSourceStartSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
	Timing.fCueSourceEndSeconds = static_cast<f32_t>(
		m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
	Timing.bHasCueSourceEnd =
		m_ValtanProductPreview->Cue.bHasSourceEnd;
	ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
	return CActionPresentationTimeline::Resolve_CuePreviewSample(
		Timing, (std::max)(0.f, fTimelineSeconds), Sample) &&
		Sample.bVisible;
}

bool_t Client::CEffect_Tool::Restore_ValtanProductPreviewPlayback(
	const optional<VALTAN_PRODUCT_PREVIEW>& Preview,
	const f32_t fTimelineSeconds,
	const f32_t fDurationSeconds,
	const bool_t bPlaying,
	const bool_t bVisibleRequested,
	const float4x4_t& SnapshotRoot,
	const bool_t bSnapshotCaptured,
	std::string& strOutError)
{
	strOutError.clear();
	if (!Preview.has_value())
		return true;
	const EFFECT_DOCUMENT_DESC* pRestoreDocument = nullptr;
	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == Preview->Cue.strEffectAssetId)
	{
		pRestoreDocument = &*m_ActiveDocument;
	}
	else if (m_SourcePreviewDocument.has_value() &&
		m_SourcePreviewDocument->strEffectAssetId ==
			Preview->Cue.strEffectAssetId)
	{
		pRestoreDocument = &*m_SourcePreviewDocument;
	}
	if (nullptr == pRestoreDocument)
	{
		strOutError =
			"the exact Valtan Effect document is no longer active";
		return false;
	}

	const auto FailRestore = [this, &strOutError](std::string Reason)
	{
		const shared_ptr<CEffectObject> pObject =
			m_pWorldPreviewObject.lock();
		if (nullptr != pObject)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
		}
		Set_SynchronizedAnimationPaused(true);
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		strOutError = std::move(Reason);
		m_strPreviewStatus =
			"Exact Valtan Product preview restore failed: " + strOutError;
		return false;
	};

	m_ProductPreview.reset();
	m_ValtanProductPreview = Preview;
	m_fPreviewDurationSeconds = (std::max)(0.f, fDurationSeconds);
	m_fPreviewTimeSeconds = (std::clamp)(
		fTimelineSeconds, 0.f, m_fPreviewDurationSeconds);
	m_bPreviewPlaying = bPlaying;
	m_bPreviewVisibleRequested = bVisibleRequested;
	m_ProductCueSnapshotRoot = SnapshotRoot;
	m_bProductCueSnapshotCaptured = bSnapshotCaptured;
	m_fProductCueActionFacingYawDegrees = 0.f;
	m_bProductCueActionFacingCaptured = false;
	/* Rebuild document-owned boss state first. This restores the exact 420633
	   transform-history preparation when the selected document owns it. The v2
	   occurrence then replaces the legacy clip with its exact source segment. */
	Synchronize_LoadedSkillPreview();
	if (!Play_ValtanClipOccurrence(Preview->Clip))
	{
		return FailRestore(
			"the exact animation clip occurrence could not be staged");
	}
	Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
	if (m_bPreviewPlaying)
		Update_SynchronizedAnimationSequence();
	Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);

	if (!Stage_WorldPreview(*pRestoreDocument))
	{
		return FailRestore(
			"the exact Effect document could not be staged: " +
			m_strPreviewStatus);
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject)
		return FailRestore("the restored EffectObject is unavailable");

	const f32_t fEffectSampleSeconds =
		Resolve_EffectSampleTime(m_fPreviewTimeSeconds);
	if (m_bValtanBossPatternTransformHistoryRequired)
	{
		if (!m_bValtanBossPatternTransformHistoryActive)
		{
			return FailRestore(
				"the exact Valtan transform history is unavailable");
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
			[this](const f32_t fSampleTimeSeconds,
				EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
				std::string& strError)
			{
				return Build_ValtanBossPatternTransformSample(
					fSampleTimeSeconds, OutSample, strError);
			};
		std::string TransformError;
		if (!pObject->Set_SampleTimeWithTransformHistory(
				fEffectSampleSeconds, TransformProvider, TransformError))
		{
			return FailRestore(
				"the exact Valtan transform-history sample failed: " +
				TransformError);
		}
	}
	else
	{
		std::string HistoryError;
		if (!Seek_WorldPreviewWithSourceAnchorHistory(
				pObject, *pRestoreDocument,
				fEffectSampleSeconds, HistoryError))
		{
			return FailRestore(
				"the source-anchor history could not be restored: " +
				HistoryError);
		}
	}

	/* Effect Tool owns this wall clock, so the EffectObject remains autonomous
	   playback-off while Tool and animation play/pause state are restored. */
	pObject->Set_Playing(false);
	pObject->Set_Visible(m_bPreviewVisibleRequested &&
		Is_ProductCueVisible(m_fPreviewTimeSeconds));
	Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
	return true;
}

void Client::CEffect_Tool::Clear_ProductCuePreview()
{
    m_ProductPreview.reset();
	m_ValtanProductPreview.reset();
	m_SourcePreviewDocument.reset();
	m_PlayerPreviewCueCandidates.clear();
	m_iPlayerPreviewCueCandidateIndex = 0u;
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
    Reset_ProductCueSnapshot();
}

void Client::CEffect_Tool::Select_PlayerPreviewCueCandidate(
	const size_t iCandidateIndex)
{
	if (!m_ProductPreview.has_value() || !m_ActiveDocument.has_value() ||
		iCandidateIndex >= m_PlayerPreviewCueCandidates.size())
	{
		return;
	}

	const ANIMATION_EFFECT_PREVIEW_CANDIDATE Candidate =
		m_PlayerPreviewCueCandidates[iCandidateIndex];
	EFFECT_PRODUCT_PREVIEW Preview = *m_ProductPreview;
	Preview.ProductCue.Cue = Candidate.Cue;
	Preview.ProductCue.Clip = Candidate.Clip;
	Preview.ProductCue.iBoundClipOrdinal = Candidate.iBoundClipOrdinal;
	Preview.ProductCue.iStageIndex = Candidate.iStageIndex;
	Preview.ProductCue.iStageClipIndex = Candidate.iStageClipIndex;
	m_ProductPreview = std::move(Preview);
	m_SourcePreviewDocument.reset();
	m_iPlayerPreviewCueCandidateIndex = iCandidateIndex;
	m_fPreviewTimeSeconds = 0.f;
	Reset_ProductCueSnapshot();
	Recalculate_PreviewDuration(*m_ActiveDocument);
	Synchronize_LoadedSkillPreview();
}

void Client::CEffect_Tool::Reset_ProductCueSnapshot()
{
    m_ProductCueSnapshotRoot = Identity_Matrix();
    m_bProductCueSnapshotCaptured = false;
	m_fProductCueActionFacingYawDegrees = 0.f;
	m_bProductCueActionFacingCaptured = false;
}

void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()
{
    m_fPreviewTimeSeconds = 0.f;
    Reset_ProductCueSnapshot();
    Restart_SynchronizedAnimationSequence();
    shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject &&
		(m_SourcePreviewDocument.has_value() || m_ActiveDocument.has_value()) &&
		Stage_WorldPreview())
    {
        pObject = m_pWorldPreviewObject.lock();
    }
    if (nullptr == pObject)
    {
		if (m_bReconstructedSourceRuntimeActive)
		{
			Set_SynchronizedAnimationPaused(true);
			Reset_SynchronizedAnimationSequence();
			m_bReconstructedSourceRuntimeActive = false;
			Reset_ReconstructedSourceRuntimeTimeline();
		}
        m_bPreviewPlaying = false;
        return;
    }
	if (m_bReconstructedSourceRuntimeActive)
	{
		Set_SynchronizedAnimationPaused(true);
		Reset_ReconstructedSourceRuntimeTimeline();
		if (!Prepare_ReconstructedSourceRuntimeTransformHistory())
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Reset_SynchronizedAnimationSequence();
			m_bReconstructedSourceRuntimeActive = false;
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Artist F restart historical anchor preparation failed: " +
				m_strPreviewAnimationStatus;
			return;
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
			[this](const f32_t fSampleTimeSeconds,
				EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
				std::string& strOutError)
			{
				return Build_ReconstructedSourceRuntimeTransformSample(
					fSampleTimeSeconds, OutSample, strOutError);
			};
		std::string Error;
		if (!pObject->Set_SampleTimeWithTransformHistory(
				0.f, TransformProvider, Error))
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			Reset_SynchronizedAnimationSequence();
			m_bReconstructedSourceRuntimeActive = false;
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Artist F restart historical zero-frame failed: " + Error;
			return;
		}
		m_bReconstructedSourceRuntimeStartPending = true;
		m_bPreviewVisibleRequested = true;
		m_bPreviewPlaying = true;
		pObject->Set_Playing(false);
		pObject->Set_Visible(false);
		m_strPreviewStatus =
			"Artist Core F (33) restart prepared at synchronized time zero; playback starts on the next update.";
		return;
	}
	if (m_bValtanBossPatternTransformHistoryRequired)
	{
		if (!m_bValtanBossPatternTransformHistoryActive)
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Valtan 420633 preview refused a missing exact b_effectroot history binding.";
			return;
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
			[this](const f32_t fSampleTimeSeconds,
				EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
				std::string& strOutError)
			{
				return Build_ValtanBossPatternTransformSample(
					fSampleTimeSeconds, OutSample, strOutError);
			};
		std::string TransformError;
		pObject->Reset();
		if (!pObject->Set_SampleTimeWithTransformHistory(
				0.f, TransformProvider, TransformError))
		{
			pObject->Set_Playing(false);
			pObject->Set_Visible(false);
			Set_SynchronizedAnimationPaused(true);
			m_bValtanBossPatternTransformHistoryActive = false;
			m_bPreviewPlaying = false;
			m_bPreviewVisibleRequested = false;
			m_strPreviewStatus =
				"Valtan 420633 zero-frame anchor history failed: " +
				TransformError;
			return;
		}
		m_bPreviewVisibleRequested = true;
		m_bPreviewPlaying = true;
		pObject->Set_Visible(true);
		m_strPreviewStatus =
			"Valtan 420633 restart prepared with exact B_EffectRoot / b_effectroot history.";
		return;
	}
    float4x4_t TargetRoot{};
    const bool_t bRootResolved = Resolve_PreviewRoot(TargetRoot);
    if (bRootResolved)
        pObject->Set_RootWorld(TargetRoot);
    pObject->Set_Visible(
        bRootResolved && Is_ProductCueVisible(m_fPreviewTimeSeconds));
    pObject->Reset();
    pObject->Set_SampleTime(
        Resolve_EffectSampleTime(m_fPreviewTimeSeconds));
	m_bPreviewVisibleRequested = true;
    m_bPreviewPlaying = true;
}

void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()
{
    Reset_SynchronizedAnimationSequence();
	Reset_ValtanBossPatternTransformHistory();
	m_PlayerPreviewCueCandidates.clear();
	m_iPlayerPreviewCueCandidateIndex = 0u;
    m_strPreviewAnimationStatus.clear();
	const EFFECT_DOCUMENT_DESC* pPreviewDocument =
		m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value() ?
			&*m_SourcePreviewDocument :
			(m_ActiveDocument.has_value() ? &*m_ActiveDocument : nullptr);
	if (nullptr == pPreviewDocument)
        return;

	/* Boss pattern Effects are action-owned rather than PlayerSkills-owned.
	   Resolve them before the playable catalog join so Model View can stage the
	   real Valtan model and exact authored action clip without a hard-coded
	   effect-to-clip switch. */
	BOSS_PATTERN_EFFECT_BINDING_DOCUMENT BossEffectBindings;
	std::string BossEffectStatus;
	const std::filesystem::path BossEffectPath =
		CValtanPatternEffectBindingDocument::Resolve_Path("Valtan");
	std::ifstream BossEffectInput(BossEffectPath, std::ios::binary);
	if (BossEffectInput)
	{
		const std::string BossEffectText{
			std::istreambuf_iterator<char>(BossEffectInput),
			std::istreambuf_iterator<char>() };
		if (CValtanPatternEffectBindingDocument::Parse_Text(
				BossEffectText, BossEffectBindings, BossEffectStatus))
		{
			const auto BossBinding = std::find_if(
				BossEffectBindings.Bindings.begin(),
				BossEffectBindings.Bindings.end(),
				[pPreviewDocument](
					const BOSS_PATTERN_EFFECT_BINDING& Candidate)
				{
					return Candidate.strEffectAssetId ==
						pPreviewDocument->strEffectAssetId;
				});
			if (BossBinding != BossEffectBindings.Bindings.end())
			{
				const bool_t bRequiresExactTransformHistory =
					BossBinding->strBindingId == VALTAN_EXACT_HISTORY_BINDING_ID &&
					BossBinding->strEffectAssetId ==
						VALTAN_EXACT_HISTORY_EFFECT_ASSET_ID;
				if (bRequiresExactTransformHistory)
				{
					m_bValtanBossPatternTransformHistoryRequired = true;
					m_strValtanBossPatternPreviewEffectAssetId =
						pPreviewDocument->strEffectAssetId;
				}
				constexpr const char_t* BOSS_PREVIEW_ASSET = VALTAN_ANIMATION_ASSET_NAME;
				if (CAnimationTargetService::Resolve_AssetName() !=
						BOSS_PREVIEW_ASSET &&
					!m_pCharacterPreviewPanel->Select_TargetAsset(
						BOSS_PREVIEW_ASSET))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect is loaded, but the boss model could not be staged.";
					return;
				}
				const shared_ptr<Engine::CModel> pBossModel =
					CAnimationTargetService::Resolve_Model();
				if (nullptr == pBossModel ||
					!CValtanPatternEffectBindingDocument::Validate(
						BossEffectBindings, "BOSS_VALTAN",
						Collect_AnimationClipNames(pBossModel), BossEffectStatus))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect binding was not applied: " + BossEffectStatus;
					return;
				}
				BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
				std::string AnimationStatus;
				if (!CValtanPatternAnimationBindingDocument::Load(
						"Valtan", "BOSS_VALTAN",
						Collect_AnimationClipNames(pBossModel), AnimationBindings,
						AnimationStatus))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect animation was not applied: " +
						AnimationStatus;
					return;
				}
				const auto AnimationBinding = std::find_if(
					AnimationBindings.Bindings.begin(),
					AnimationBindings.Bindings.end(),
					[&BossBinding](
						const BOSS_PATTERN_ANIMATION_BINDING& Candidate)
					{
						return Candidate.strActionId ==
							BossBinding->strActionId;
					});
				if (AnimationBinding == AnimationBindings.Bindings.end() ||
					AnimationBinding->Clips.end() == std::find(
						AnimationBinding->Clips.begin(),
						AnimationBinding->Clips.end(),
						BossBinding->strRuntimeClipName))
				{
					m_strPreviewAnimationStatus =
						"Valtan Effect action/clip binding drifted; preview failed closed.";
					return;
				}
				m_SynchronizedAnimationClips = {
					{ BossBinding->strRuntimeClipName, 0u, 1.f } };
				m_iSynchronizedAnimationClipIndex = 0u;
				m_iSynchronizedAnimationLoopEpoch = 0u;
				m_iSynchronizedAnimationTargetGeneration =
					CAnimationTargetService::Resolve_TargetGeneration();
				if (!pBossModel->Start_Animation(
						BossBinding->strRuntimeClipName.c_str(), m_bPreviewLoop))
				{
					Reset_SynchronizedAnimationSequence();
					m_strPreviewAnimationStatus =
						"Valtan Effect clip could not be started: " +
						BossBinding->strRuntimeClipName;
					return;
				}
				pBossModel->Set_AnimationSpeed(1.f);
				pBossModel->Set_AnimPaused(false);
				if (bRequiresExactTransformHistory)
				{
					std::string TransformHistoryError;
					if (!Prepare_ValtanBossPatternTransformHistory(
							*BossBinding, *pPreviewDocument,
							TransformHistoryError))
					{
						pBossModel->Set_AnimPaused(true);
						m_strPreviewAnimationStatus =
							"Valtan Effect exact follow anchor was not staged: " +
							TransformHistoryError;
						return;
					}
				}
				m_strPreviewAnimationStatus =
					"Boss pattern animation synced: " +
					BossBinding->strPatternId + " / " +
					BossBinding->strActionId + " -> " +
					BossBinding->strRuntimeClipName + " | " +
					BossBinding->strProductAdmissionStatus;
				return;
			}
		}
	}

    std::string CatalogStatus;
    const bool_t bCatalogAvailable =
        Ensure_PlayerSkillCatalog(CatalogStatus);
    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    auto Skill = m_ProductPreview.has_value() ?
        std::find_if(
            Skills.begin(), Skills.end(),
            [this](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return Candidate.eCharacterClass ==
                        m_ProductPreview->eCharacterClass &&
                    Candidate.iSkillId == m_ProductPreview->iSkillId;
            }) :
        std::find_if(
            Skills.begin(), Skills.end(),
            [pPreviewDocument](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return Candidate.strEffectId ==
                    pPreviewDocument->strEffectAssetId;
            });
    if (!m_ProductPreview.has_value() && Skill == Skills.end() &&
        (pPreviewDocument->strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID ||
         pPreviewDocument->strEffectAssetId ==
            DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID))
    {
        const bool_t bArtistFUnified =
            pPreviewDocument->strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID;
        const LostArk::Shared::CHARACTER_CLASS_ID eUnifiedClass = bArtistFUnified ?
            LostArk::Shared::CHARACTER_CLASS_ID::ARTIST :
            LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER;
        const uint32_t iUnifiedSkillId = bArtistFUnified ?
            ARTIST_F_CORE_SKILL_ID : DIMENSION_MASTER_T_SKILL_ID;
        Skill = std::find_if(
            Skills.begin(), Skills.end(),
            [eUnifiedClass, iUnifiedSkillId](
                const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return Candidate.eCharacterClass == eUnifiedClass &&
                    Candidate.iSkillId == iUnifiedSkillId;
            });
    }
	if (!m_ProductPreview.has_value() && Skill == Skills.end())
	{
		const std::string& DocumentAssetId =
			pPreviewDocument->strEffectAssetId;
		const auto OwnsDocument = [&DocumentAssetId](
			const std::string_view ProductAssetId)
		{
			return !ProductAssetId.empty() &&
				(ProductAssetId == DocumentAssetId ||
				 Unified_CandidateAssetId(ProductAssetId) == DocumentAssetId);
		};
		const auto Owner = std::find_if(m_AllEffects.begin(), m_AllEffects.end(),
			[&OwnsDocument](const EFFECT_SKILL_TREE_ENTRY& Entry)
			{
				if (OwnsDocument(Entry.Skill.strEffectId))
					return true;
				return std::any_of(Entry.ProductCues.begin(),
					Entry.ProductCues.end(), [&OwnsDocument](const auto& Cue)
					{
						return OwnsDocument(Cue.Cue.strEffectAssetId);
					});
			});
		if (Owner != m_AllEffects.end())
		{
			Skill = std::find_if(Skills.begin(), Skills.end(),
				[&Owner](const PLAYER_SKILL_DEFINITION& Candidate)
				{
					return Candidate.eCharacterClass ==
							Owner->Skill.eCharacterClass &&
						Candidate.iSkillId == Owner->Skill.iSkillId;
			});
		}
	}
	if (!m_ProductPreview.has_value() && Skill == Skills.end())
	{
		const std::string& DocumentAssetId =
			pPreviewDocument->strEffectAssetId;
		const auto CandidateOwner = std::find_if(
			m_UnifiedCandidateBindings.begin(),
			m_UnifiedCandidateBindings.end(),
			[&DocumentAssetId](
				const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding)
			{
				return Binding.strEffectAssetId == DocumentAssetId;
			});
		if (CandidateOwner != m_UnifiedCandidateBindings.end())
		{
			Skill = std::find_if(Skills.begin(), Skills.end(),
				[&CandidateOwner](const PLAYER_SKILL_DEFINITION& Candidate)
				{
					return Candidate.eCharacterClass ==
							CandidateOwner->eCharacterClass &&
						Candidate.iSkillId == CandidateOwner->iSkillId;
				});
		}
	}
    if (!m_ProductPreview.has_value() && Skill == Skills.end() &&
        std::string::npos != pPreviewDocument->strEffectAssetId.find(
            "restoration-candidate"))
    {
        Skill = std::find_if(
            Skills.begin(), Skills.end(),
            [pPreviewDocument](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return !Candidate.strEffectId.empty() &&
                    pPreviewDocument->strEffectAssetId.starts_with(
                        Candidate.strEffectId + ".");
            });
    }
    if (Skill == Skills.end())
    {
        m_strPreviewAnimationStatus = bCatalogAvailable ?
            "No PlayerSkills row owns this Effect; animation was left unchanged." :
            "PlayerSkills refresh failed; animation was left unchanged: " +
                CatalogStatus;
        return;
    }
    if (m_ProductPreview.has_value() &&
		pPreviewDocument->strEffectAssetId !=
            m_ProductPreview->ProductCue.Cue.strEffectAssetId)
    {
        m_strPreviewAnimationStatus =
            "Product animation sync rejected a stale cue/document pairing.";
        return;
    }

    m_eAllEffectsClass = Skill->eCharacterClass;
    Select_AuthoringDomainForClass(Skill->eCharacterClass);
    const char* pAnimationAsset = Animation_AssetName(Skill->eCharacterClass);
    if (nullptr == pAnimationAsset)
    {
        m_strPreviewAnimationStatus =
            "The loaded Effect has no admitted playable class target.";
        return;
    }

    const std::string CurrentAsset =
        CAnimationTargetService::Resolve_AssetName();
    if (CurrentAsset != pAnimationAsset &&
        !m_pCharacterPreviewPanel->Select_TargetAsset(pAnimationAsset))
    {
        m_strPreviewAnimationStatus =
            "Effect is playing on the current target; the matching class model "
            "could not be staged.";
        return;
    }

    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel)
    {
        m_strPreviewAnimationStatus =
            "Effect is loaded, but no animation model target is available.";
        return;
    }

    ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
    std::string BindingStatus;
	const std::vector<std::string> AvailableClips =
		Collect_AnimationClipNames(pModel);
    if (!CAnimationSkillBindingDocument::Load(
        pAnimationAsset,
        Skill->eCharacterClass,
        Skills,
        AvailableClips,
        Bindings,
        BindingStatus))
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; skill animation binding was not applied: " +
            BindingStatus;
        return;
    }

    const auto Binding = std::find_if(
        Bindings.Bindings.begin(), Bindings.Bindings.end(),
        [&Skill](const ANIMATION_SKILL_BINDING& Candidate)
        {
            return Candidate.iSkillId == Skill->iSkillId;
        });
    if (Binding == Bindings.Bindings.end() || Binding->Stages.empty())
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }

	const bool_t bSavedPlayerDirectAuthored =
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		m_ActiveDocument.has_value() &&
		pPreviewDocument == &*m_ActiveDocument;
	if (bSavedPlayerDirectAuthored)
	{
		const std::filesystem::path EventPath = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Animation") / L"Authored" /
			std::filesystem::path(pAnimationAsset) /
			(std::filesystem::path(pAnimationAsset).wstring() +
				L".animevents"));
		std::string EventText;
		std::string CandidateStatus;
		ANIMATION_EFFECT_CUE_DOCUMENT CueDocument;
		if (!Read_TextFile(EventPath, EventText, CandidateStatus) ||
			!CAnimationEffectCueDocument::Load_FromText(
				pAnimationAsset, EventText, AvailableClips, CueDocument,
				CandidateStatus, true) ||
			!CAnimationEffectCueDocument::Resolve_PreviewCandidates(
				*Binding, CueDocument.Cues,
				pPreviewDocument->strEffectAssetId,
				m_PlayerPreviewCueCandidates, CandidateStatus))
		{
			m_ProductPreview.reset();
			m_SourcePreviewDocument.reset();
			Recalculate_PreviewDuration(*m_ActiveDocument);
			m_strPreviewAnimationStatus =
				"Saved Player Effect has no exact Product cue mapping; "
				"full skill-chain preview was refused: " + CandidateStatus;
			return;
		}

		size_t iSelectedCandidate = 0u;
		if (m_ProductPreview.has_value())
		{
			const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Selected =
				m_ProductPreview->ProductCue;
			const auto Match = std::find_if(
				m_PlayerPreviewCueCandidates.begin(),
				m_PlayerPreviewCueCandidates.end(),
				[&Selected](
					const ANIMATION_EFFECT_PREVIEW_CANDIDATE& Candidate)
				{
					return Candidate.iStageIndex == Selected.iStageIndex &&
						Candidate.iStageClipIndex == Selected.iStageClipIndex &&
						Candidate.Cue.strClipName == Selected.Cue.strClipName &&
						Candidate.Cue.iStartMs == Selected.Cue.iStartMs &&
						Candidate.Cue.strEffectAssetId ==
							Selected.Cue.strEffectAssetId;
				});
			if (Match != m_PlayerPreviewCueCandidates.end())
			{
				iSelectedCandidate = static_cast<size_t>(
					std::distance(m_PlayerPreviewCueCandidates.begin(), Match));
			}
		}
		m_iPlayerPreviewCueCandidateIndex = iSelectedCandidate;
		const ANIMATION_EFFECT_PREVIEW_CANDIDATE& Candidate =
			m_PlayerPreviewCueCandidates[iSelectedCandidate];
		EFFECT_PRODUCT_PREVIEW Preview;
		Preview.eCharacterClass = Skill->eCharacterClass;
		Preview.iSkillId = Skill->iSkillId;
		Preview.ProductCue.Cue = Candidate.Cue;
		Preview.ProductCue.Clip = Candidate.Clip;
		Preview.ProductCue.iBoundClipOrdinal = Candidate.iBoundClipOrdinal;
		Preview.ProductCue.iStageIndex = Candidate.iStageIndex;
		Preview.ProductCue.iStageClipIndex = Candidate.iStageClipIndex;
		m_ProductPreview = std::move(Preview);
		m_SourcePreviewDocument.reset();
		Recalculate_PreviewDuration(*m_ActiveDocument);
	}
    m_SynchronizedAnimationClips.clear();
    if (m_ProductPreview.has_value())
    {
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& Selected =
			m_ProductPreview->ProductCue;
		if (Selected.iStageIndex < Binding->Stages.size())
		{
			const ANIMATION_SKILL_STAGE& Stage =
				Binding->Stages[Selected.iStageIndex];
			if (Selected.iStageClipIndex < Stage.Clips.size())
			{
				const ANIMATION_SKILL_CLIP& Clip =
					Stage.Clips[Selected.iStageClipIndex];
				if (Clip == Selected.Clip &&
					Clip.strClipName == Selected.Cue.strClipName &&
					CAnimationEffectCueDocument::Is_CueStartInClipWindow(
						Clip, Selected.Cue.iStartMs))
				{
					m_SynchronizedAnimationClips.push_back(Clip);
				}
			}
		}
    }
    else
    {
        /* Generic Data File preview retains the full authored chain. Product
        Play above intentionally owns only the exact clip named by its cue. */
        for (const ANIMATION_SKILL_STAGE& Stage : Binding->Stages)
        {
            m_SynchronizedAnimationClips.insert(
                m_SynchronizedAnimationClips.end(),
                Stage.Clips.begin(), Stage.Clips.end());
        }
    }
    if (m_SynchronizedAnimationClips.empty())
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }
    m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
    m_iSynchronizedAnimationTargetGeneration =
        CAnimationTargetService::Resolve_TargetGeneration();
	const SYNCHRONIZED_ANIMATION_CLIP& FirstClip =
        m_SynchronizedAnimationClips.front();
	if (!Start_SynchronizedAnimationClip(0u, false))
    {
        Reset_SynchronizedAnimationSequence();
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }
    m_strPreviewAnimationStatus = m_ProductPreview.has_value() ?
        "Product cue animation synced: " : "Skill animation synced: ";
    m_strPreviewAnimationStatus +=
        Skill->strInputSlot + " | " + Skill->strDisplayName + " -> " +
        FirstClip.strClipName;
    if (m_ProductPreview.has_value())
    {
        m_strPreviewAnimationStatus += " @ " + std::to_string(
            m_ProductPreview->ProductCue.Cue.iStartMs) + " ms | anchor=" +
            m_ProductPreview->ProductCue.Cue.strAnchorSlotId;
    }
	if (1u != m_SynchronizedAnimationClips.size())
    {
        m_strPreviewAnimationStatus += " (sequence 1/" +
            std::to_string(m_SynchronizedAnimationClips.size()) + ")";
    }
	/* Product authoring must retain the complete selected animation window even
	   when this occurrence's Effect tail is shorter.  Recalculate only after
	   the exact class model and clip window have been staged. */
	if (m_ProductPreview.has_value())
		Recalculate_PreviewDuration(*pPreviewDocument);
}

bool_t Client::CEffect_Tool::Start_SynchronizedAnimationClip(
	const size_t iClipIndex,
	const bool_t bPaused)
{
	if (iClipIndex >= m_SynchronizedAnimationClips.size())
		return false;
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
		return false;
	const SYNCHRONIZED_ANIMATION_CLIP& Clip =
		m_SynchronizedAnimationClips[iClipIndex];
	const bool_t bHasSourceWindow =
		0u != Clip.iSourceStartMs || 0u != Clip.iPlayMs;
	const bool_t bEngineLoop = !Clip.bHasExplicitLoopPolicy &&
		!bHasSourceWindow &&
		1u == m_SynchronizedAnimationClips.size() && m_bPreviewLoop;
	if (!pModel->Start_Animation(Clip.strClipName.c_str(), bEngineLoop))
		return false;

	pModel->Set_AnimationSpeed(Clip.fPlayRate);
	if (0u != Clip.iSourceStartMs)
	{
		const uint32_t iAnimation = pModel->Get_CurrentAnimIndex();
		f32_t fPosition = 0.f;
		f32_t fDuration = 0.f;
		const f32_t fTicksPerSecond =
			pModel->Get_AnimationTickPerSecond(iAnimation);
		const f32_t fSourceStartTicks =
			static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f *
			fTicksPerSecond;
		if (!pModel->Get_AnimationProgress(
				iAnimation, fPosition, fDuration) ||
			!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
			!std::isfinite(fSourceStartTicks) ||
			fSourceStartTicks < 0.f || fSourceStartTicks >= fDuration)
		{
			return false;
		}
		pModel->Set_AnimTrackPosition(iAnimation, fSourceStartTicks);
		pModel->Play_Animation(0.f);
	}
	pModel->Set_AnimPaused(bPaused);
	return true;
}

void Client::CEffect_Tool::Restart_SynchronizedAnimationSequence()
{
    if (m_SynchronizedAnimationClips.empty() ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return;
    }
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	if (!Start_SynchronizedAnimationClip(0u, false))
	{
		m_strPreviewAnimationStatus =
			"Skill animation restart failed: " +
			m_SynchronizedAnimationClips.front().strClipName;
		return;
	}
}

void Client::CEffect_Tool::Seek_SynchronizedAnimationSequence(
    const f32_t fTimeSeconds)
{
    if (m_SynchronizedAnimationClips.empty() ||
        !std::isfinite(fTimeSeconds) ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return;
    }
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel)
        return;

    f32_t fRemainingSeconds = (std::max)(0.f, fTimeSeconds);
    for (size_t iClip = 0u; iClip < m_SynchronizedAnimationClips.size();
        ++iClip)
    {
		const SYNCHRONIZED_ANIMATION_CLIP& Clip =
            m_SynchronizedAnimationClips[iClip];
        uint32_t iAnimation = UINT32_MAX;
        for (uint32_t iCandidate = 0u;
            iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
        {
            const char_t* pName = pModel->Get_AnimationName(iCandidate);
            if (nullptr != pName && Clip.strClipName == pName)
            {
                iAnimation = iCandidate;
                break;
            }
        }
        f32_t fPosition = 0.f;
        f32_t fDuration = 0.f;
        const f32_t fTicksPerSecond = UINT32_MAX == iAnimation ? 0.f :
            pModel->Get_AnimationTickPerSecond(iAnimation);
        if (UINT32_MAX == iAnimation ||
            !pModel->Get_AnimationProgress(
                iAnimation, fPosition, fDuration) ||
            !std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
            !std::isfinite(fDuration) || fDuration <= 0.f)
        {
            m_strPreviewAnimationStatus =
                "Skill animation seek failed: " + Clip.strClipName;
            return;
        }

		const f32_t fSourceStartSeconds =
			static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
		f32_t fSourceDurationSeconds =
			fDuration / fTicksPerSecond - fSourceStartSeconds;
		if (!std::isfinite(fSourceDurationSeconds) ||
			fSourceDurationSeconds <= 0.f ||
			!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
		{
			m_strPreviewAnimationStatus =
				"Skill animation seek has an invalid source segment: " +
				Clip.strClipName;
			return;
		}
		if (0u != Clip.iPlayMs)
        {
            fSourceDurationSeconds = (std::min)(
                fSourceDurationSeconds,
                static_cast<f32_t>(Clip.iPlayMs) * 0.001f);
        }
		const f32_t fWallDurationSeconds =
			fSourceDurationSeconds / Clip.fPlayRate;
		const bool_t bLastClip =
			iClip + 1u == m_SynchronizedAnimationClips.size();
		if (!bLastClip && fRemainingSeconds >= fWallDurationSeconds)
		{
			fRemainingSeconds -= fWallDurationSeconds;
			continue;
		}

		f32_t fLocalWallSeconds = (std::min)(
			fRemainingSeconds, fWallDurationSeconds);
		m_iSynchronizedAnimationLoopEpoch = 0u;
		if (Clip.bHasExplicitLoopPolicy && Clip.bLoop)
		{
			const f32_t fEpoch = std::floor(
				fRemainingSeconds / fWallDurationSeconds);
			if (!std::isfinite(fEpoch) || fEpoch < 0.f)
				return;
			m_iSynchronizedAnimationLoopEpoch =
				static_cast<uint64_t>(fEpoch);
			fLocalWallSeconds = std::fmod(
				fRemainingSeconds, fWallDurationSeconds);
		}
		m_iSynchronizedAnimationClipIndex = iClip;
		if (!Start_SynchronizedAnimationClip(iClip, !m_bPreviewPlaying))
		{
			m_strPreviewAnimationStatus =
				"Skill animation seek failed: " + Clip.strClipName;
			return;
		}
		const f32_t fTrackPosition =
			(fSourceStartSeconds + (std::min)(
				fLocalWallSeconds * Clip.fPlayRate,
				fSourceDurationSeconds)) * fTicksPerSecond;
		pModel->Set_AnimTrackPosition(iAnimation, fTrackPosition);
		pModel->Play_Animation(0.f);
		pModel->Set_AnimPaused(!m_bPreviewPlaying);
		return;
    }
}

void Client::CEffect_Tool::Set_SynchronizedAnimationPaused(
    const bool_t bPaused)
{
    if (m_SynchronizedAnimationClips.empty() ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return;
    }
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr != pModel)
        pModel->Set_AnimPaused(bPaused);
}

bool_t Client::CEffect_Tool::Try_ResolveSynchronizedAnimationTime(
    f32_t& fOutTimeSeconds) const
{
    fOutTimeSeconds = 0.f;
    if (m_SynchronizedAnimationClips.empty() ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return false;
    }
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel ||
        m_iSynchronizedAnimationClipIndex >=
            m_SynchronizedAnimationClips.size())
    {
        return false;
    }

    for (size_t iClip = 0u;
        iClip <= m_iSynchronizedAnimationClipIndex; ++iClip)
    {
		const SYNCHRONIZED_ANIMATION_CLIP& Clip =
            m_SynchronizedAnimationClips[iClip];
        uint32_t iAnimation = UINT32_MAX;
        for (uint32_t iCandidate = 0u;
            iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
        {
            const char_t* pName = pModel->Get_AnimationName(iCandidate);
            if (nullptr != pName && Clip.strClipName == pName)
            {
                iAnimation = iCandidate;
                break;
            }
        }
        f32_t fPosition = 0.f;
        f32_t fDuration = 0.f;
        const f32_t fTicksPerSecond = UINT32_MAX == iAnimation ? 0.f :
            pModel->Get_AnimationTickPerSecond(iAnimation);
        if (UINT32_MAX == iAnimation ||
            !pModel->Get_AnimationProgress(
                iAnimation, fPosition, fDuration) ||
            !std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
            !std::isfinite(fDuration) || fDuration <= 0.f)
        {
            return false;
        }

		const f32_t fSourceStartSeconds =
			static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
		f32_t fSourceDurationSeconds =
			fDuration / fTicksPerSecond - fSourceStartSeconds;
		if (!std::isfinite(fSourceDurationSeconds) ||
			fSourceDurationSeconds <= 0.f ||
			!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
		{
			return false;
		}
		if (0u != Clip.iPlayMs)
        {
            fSourceDurationSeconds = (std::min)(
                fSourceDurationSeconds,
                static_cast<f32_t>(Clip.iPlayMs) * 0.001f);
        }
		if (iClip < m_iSynchronizedAnimationClipIndex)
		{
			fOutTimeSeconds +=
				fSourceDurationSeconds / Clip.fPlayRate;
			continue;
        }

        const char_t* pCurrentName = pModel->Get_AnimationName(
            pModel->Get_CurrentAnimIndex());
        if (nullptr == pCurrentName || Clip.strClipName != pCurrentName)
            return false;
		const f32_t fWallDurationSeconds =
			fSourceDurationSeconds / Clip.fPlayRate;
		if (Clip.bHasExplicitLoopPolicy && Clip.bLoop)
		{
			fOutTimeSeconds += static_cast<f32_t>(
				m_iSynchronizedAnimationLoopEpoch) * fWallDurationSeconds;
		}
		const f32_t fCurrentSourceSeconds = (std::min)(
			(std::max)(0.f,
				fPosition / fTicksPerSecond - fSourceStartSeconds),
			fSourceDurationSeconds);
		const bool_t bHasSourceWindow =
			0u != Clip.iSourceStartMs || 0u != Clip.iPlayMs;
		if (CActionPresentationTimeline::
			Should_ReleaseCompletedAnimationClock(
				Clip.bHasExplicitLoopPolicy || bHasSourceWindow,
				Clip.bHasExplicitLoopPolicy ? Clip.bLoop : m_bPreviewLoop,
				iClip + 1u == m_SynchronizedAnimationClips.size(),
				pModel->Is_AnimPaused(), fCurrentSourceSeconds,
				fSourceDurationSeconds))
		{
			/* Keep the model paused on its final pose, but let the Effect Tool
			   wall clock finish any natural tail beyond the animation segment. */
			return false;
		}
		fOutTimeSeconds += fCurrentSourceSeconds / Clip.fPlayRate;
    }
    return std::isfinite(fOutTimeSeconds);
}

void Client::CEffect_Tool::Update_SynchronizedAnimationSequence()
{
    if (m_SynchronizedAnimationClips.empty())
        return;
    if (m_iSynchronizedAnimationTargetGeneration !=
        CAnimationTargetService::Resolve_TargetGeneration())
    {
        Reset_SynchronizedAnimationSequence();
        return;
    }
	if (m_SynchronizedAnimationClips.size() <= 1u &&
		!m_SynchronizedAnimationClips.front().bHasExplicitLoopPolicy &&
		0u == m_SynchronizedAnimationClips.front().iSourceStartMs &&
		0u == m_SynchronizedAnimationClips.front().iPlayMs)
		return;
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel ||
        m_iSynchronizedAnimationClipIndex >=
            m_SynchronizedAnimationClips.size())
    {
        Reset_SynchronizedAnimationSequence();
        return;
    }
    const uint32_t iAnimation = pModel->Get_CurrentAnimIndex();
    const char_t* pCurrentName = pModel->Get_AnimationName(iAnimation);
	const SYNCHRONIZED_ANIMATION_CLIP& CurrentClip =
        m_SynchronizedAnimationClips[m_iSynchronizedAnimationClipIndex];
    if (nullptr == pCurrentName || CurrentClip.strClipName != pCurrentName)
    {
        Reset_SynchronizedAnimationSequence();
        return;
    }
    if (pModel->Is_AnimPaused())
        return;

    f32_t fPosition = 0.f;
    f32_t fDuration = 0.f;
    if (!pModel->Get_AnimationProgress(
        iAnimation, fPosition, fDuration) || fDuration <= 0.f)
    {
        return;
    }
	const f32_t fTicksPerSecond =
		pModel->Get_AnimationTickPerSecond(iAnimation);
	if (!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
		return;
	const f32_t fSourceStart =
		static_cast<f32_t>(CurrentClip.iSourceStartMs) * 0.001f *
		fTicksPerSecond;
	if (!std::isfinite(fSourceStart) || fSourceStart < 0.f ||
		fSourceStart >= fDuration)
	{
		Reset_SynchronizedAnimationSequence();
		return;
	}
	f32_t fLimit = fDuration;
	if (0u != CurrentClip.iPlayMs && std::isfinite(fTicksPerSecond) &&
		fTicksPerSecond > 0.f)
	{
		fLimit = (std::min)(
			fDuration,
			fSourceStart +
				static_cast<f32_t>(CurrentClip.iPlayMs) * 0.001f *
				fTicksPerSecond);
    }
    if (fPosition + 0.0001f < fLimit)
    {
        return;
    }

	const bool_t bLastClip =
		m_iSynchronizedAnimationClipIndex + 1u ==
			m_SynchronizedAnimationClips.size();
	if (CurrentClip.bHasExplicitLoopPolicy && CurrentClip.bLoop)
	{
		++m_iSynchronizedAnimationLoopEpoch;
		if (!Start_SynchronizedAnimationClip(
				m_iSynchronizedAnimationClipIndex, false))
		{
			Reset_SynchronizedAnimationSequence();
		}
		return;
	}
	if (bLastClip &&
		(CurrentClip.bHasExplicitLoopPolicy || !m_bPreviewLoop))
	{
		pModel->Set_AnimPaused(true);
		return;
	}
	m_iSynchronizedAnimationClipIndex = bLastClip ?
		0u : m_iSynchronizedAnimationClipIndex + 1u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	const SYNCHRONIZED_ANIMATION_CLIP& NextClip =
		m_SynchronizedAnimationClips[m_iSynchronizedAnimationClipIndex];
	if (!Start_SynchronizedAnimationClip(
			m_iSynchronizedAnimationClipIndex, false))
	{
        m_strPreviewAnimationStatus =
            "Skill animation sequence stopped; next clip is unavailable: " +
            NextClip.strClipName;
        Reset_SynchronizedAnimationSequence();
        return;
    }
	m_strPreviewAnimationStatus = "Skill animation sequence: " +
        NextClip.strClipName +
        " (" + std::to_string(m_iSynchronizedAnimationClipIndex + 1u) +
        "/" + std::to_string(m_SynchronizedAnimationClips.size()) + ")";
}

void Client::CEffect_Tool::Reset_SynchronizedAnimationSequence()
{
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr != pModel)
        pModel->Set_AnimationSpeed(1.f);
	m_SynchronizedAnimationClips.clear();
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
    m_iSynchronizedAnimationTargetGeneration = 0u;
	m_ReconstructedSourceRuntimePoseBinding = {};
}

void Client::CEffect_Tool::Hide_WorldPreview()
{
    m_bPreviewPlaying = false;
	m_bPreviewVisibleRequested = false;
    Release_WorldPreview(true);
    m_strPreviewStatus =
        "World preview hidden; the loaded Document and Effect Detail values were preserved.";
}

void Client::CEffect_Tool::Release_WorldPreview(
    const bool_t bRemoveFromLayer)
{
	const bool_t bWasReconstructedSourceRuntimeActive =
		m_bReconstructedSourceRuntimeActive;
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (bRemoveFromLayer && nullptr != pObject &&
        m_iWorldPreviewLevel == CGameInstance::Get().Get_CurrentLevelID())
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            m_iWorldPreviewLevel, PREVIEW_LAYER, pObject);
    }
    m_pWorldPreviewObject.reset();
	m_pVisualPreviewProjection.reset();
	m_iWorldPreviewLevel = UINT32_MAX;
	m_bReconstructedDiagnosticActive = false;
	m_bReconstructedSourceRuntimeActive = false;
	Reset_ReconstructedSourceRuntimeTimeline();
	Reset_ValtanBossPatternTransformHistory();
	if (bWasReconstructedSourceRuntimeActive)
	{
		Set_SynchronizedAnimationPaused(true);
		Reset_SynchronizedAnimationSequence();
	}
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	Release_WorldPreview(true);
    Clear_ProductCuePreview();
	Reset_RuntimeOccurrenceTuningSession();
    m_ActiveDocument.reset();
    Clear_ActiveDocumentDrawableStatus();
    m_ActiveDocumentPath.clear();
	m_strActiveDocumentBaselineCanonical.clear();
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedModelCueId.clear();
	m_strPreviewIsolationElementId.clear();
	m_strPreviewIsolationGroupId.clear();
	m_strPreviewIsolationModelCueId.clear();
	m_ePreviewIsolationAuthoringFamily = EFFECT_AUTHORING_FAMILY::END;
	m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strSelectedResourceAssetId.clear();
    m_bDocumentDirty = false;
    m_bActiveDocumentMatchesRuntime = false;
	m_strSaveHotReloadStatus.clear();
    m_bPreviewPlaying = false;
    m_fPreviewTimeSeconds = 0.f;
    Reset_SynchronizedAnimationSequence();
    Release_WorldPreview(true);
    m_strDocumentStatus =
        "Unloaded the in-memory Effect Document and hid its preview; the saved Data File was preserved.";
}

void Client::CEffect_Tool::Reset_MeshAuthoringDraft()
{
	m_SourceElementPresetSelection.reset();
    m_MeshAuthoringDraft = {};
	m_MeshAuthoringDraft.eKind =
		AuthoringFamily_Kind(m_eSelectedAuthoringFamily);
	m_MeshAuthoringDraft.Renderer = {};
	m_MeshAuthoringDraft.strGroupId = "manual.hit1";
    m_MeshAuthoringDraft.Material.strTemplateId =
        std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
    m_MeshAuthoringDraft.Material.eRenderProfile =
        EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
    m_MeshAuthoringDraft.Detail.Mesh.bUseModelMaterial = false;
	m_MeshAuthoringDraft.Detail.Timing.fLifeTimeSeconds = 5.f;
	if (AuthoringFamily_RequiresMesh(m_eSelectedAuthoringFamily))
	{
		m_MeshAuthoringDraft.Detail.Transform.vScale = {
			EFFECT_MANUAL_MESH_DEFAULT_SCALE,
			EFFECT_MANUAL_MESH_DEFAULT_SCALE,
			EFFECT_MANUAL_MESH_DEFAULT_SCALE };
	}
	if (EFFECT_AUTHORING_FAMILY::MESH_PARTICLE ==
			m_eSelectedAuthoringFamily ||
		EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE ==
			m_eSelectedAuthoringFamily)
	{
		EFFECT_PARTICLE_DESC& Particle =
			m_MeshAuthoringDraft.Detail.Particle;
		Particle.iMaxParticles = 1u;
		Particle.fSpawnRatePerSecond = 0.f;
		Particle.iBurstCount = 1u;
		Particle.iRandomSeed = 1u;
		Particle.vLifeTimeSeconds = { 2.f, 2.f };
		Particle.vInitialPositionMin = {};
		Particle.vInitialPositionMax = {};
		Particle.vInitialVelocityMin = {};
		Particle.vInitialVelocityMax = {};
		Particle.vAcceleration = {};
		const f32_t fSize =
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE ==
				m_eSelectedAuthoringFamily ? 1.f : 0.75f;
		Particle.vStartSize = { fSize, fSize };
		Particle.vEndSize = { fSize, fSize };
		Particle.bLocalSpace = true;
		Particle.bBillboard =
			EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE ==
				m_eSelectedAuthoringFamily;
	}
	if (EFFECT_AUTHORING_FAMILY::LOCAL_DECAL ==
		m_eSelectedAuthoringFamily)
	{
		m_MeshAuthoringDraft.Detail.Decal.vSize = { 3.5f, 3.5f };
		m_MeshAuthoringDraft.Detail.Decal.fDepth = 1.f;
	}
	if (EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON ==
		m_eSelectedAuthoringFamily)
	{
		m_MeshAuthoringDraft.Detail.Transform.vVelocityPerSecond =
			{ 1.f, 0.f, 0.f };
		m_MeshAuthoringDraft.Detail.Trail.fPointLifeTimeSeconds = 0.75f;
		m_MeshAuthoringDraft.Detail.Trail.fStartWidth = 0.25f;
		m_MeshAuthoringDraft.Detail.Trail.fEndWidth = 0.1f;
	}
	m_MeshAuthoringDraft.SourceRecipe = {};
	m_MeshAuthoringDraft.SourcePresentation = {};
	m_MeshAuthoringDraft.ActionCueAttachment = {};
	m_MeshAuthoringDraft.TransformInheritance = {};
    m_bMeshAuthoringDraftInitialized = true;
    m_NewElementId[0u] = '\0';
	const bool_t bRequiresMesh =
		AuthoringFamily_RequiresMesh(m_eSelectedAuthoringFamily);
	m_strSelectedResourceSlotId = bRequiresMesh ?
		std::string(EFFECT_MESH_SHAPE_SLOT_ID) :
		std::string(EFFECT_STANDARD_MATERIAL_INPUTS.front().strSlotId);
    m_strSelectedResourceAssetId.clear();
	m_eResourceLibraryFileKind = bRequiresMesh ?
		EFFECT_RESOURCE_FILE_KIND::MODEL : EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    m_iResourceViewRevision = UINT64_MAX;
}

void Client::CEffect_Tool::Reset_ParticleSystemDraft()
{
    m_ParticleSystemDraft.reset();
    m_bParticleSystemDraftDirty = false;
    m_strDetailStatus.clear();
}

void Client::CEffect_Tool::Refresh_DetailDraftAdmission(
	const EFFECT_ELEMENT_DESC& Element)
{
	m_bDetailDraftPortableRecipeReadOnly = false;
	m_bDetailDraftCapabilityDeferred = false;
	m_strDetailDraftCapabilityReason.clear();
	if (!m_ActiveDocument.has_value() ||
		!Is_CompilerOwnedPortableRecipe(*m_ActiveDocument, Element))
	{
		return;
	}
	m_bDetailDraftPortableRecipeReadOnly = true;
	if (EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind)
		return;

	EFFECT_ELEMENT_DESC PortableProbe = Element;
	std::string Error;
	if (CEffectDocumentCodec::Apply_PortableAuthoredParticleRuntimeCarrier(
			Element, PortableProbe, Error))
	{
		return;
	}
	m_bDetailDraftCapabilityDeferred = true;
	m_strDetailDraftCapabilityReason = Error.empty() ?
		"The current SourceRecipe is outside the ordinary portable Particle capability." :
		std::move(Error);
}

void Client::CEffect_Tool::Reset_DetailDraft()
{
    m_DetailDraft.reset();
    m_strDetailDraftElementId.clear();
	m_strDetailDraftCapabilityReason.clear();
    m_bDetailDraftDirty = false;
	m_bDetailDraftPortableRecipeReadOnly = false;
	m_bDetailDraftCapabilityDeferred = false;
	m_bDetailDraftPreviewPending = false;
	m_fDetailDraftPreviewDueSeconds = 0.0;
    m_strDetailStatus.clear();
}

void Client::CEffect_Tool::Recalculate_PreviewDuration()
{
	if (m_ProductPreview.has_value() && m_SourcePreviewDocument.has_value())
	{
		Recalculate_PreviewDuration(*m_SourcePreviewDocument);
		return;
	}
    if (!m_ActiveDocument.has_value())
    {
        m_fPreviewDurationSeconds = 1.f;
        m_fPreviewTimeSeconds = std::clamp(
            m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
        return;
    }
    Recalculate_PreviewDuration(*m_ActiveDocument);
}

void Client::CEffect_Tool::Recalculate_PreviewDuration(
    const EFFECT_DOCUMENT_DESC& Document)
{
    f32_t fEffectDurationSeconds = 1.f;
    for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
    {
        if (!Element.bVisible)
            continue;
        fEffectDurationSeconds = (std::max)(fEffectDurationSeconds,
			Element_PreviewEndSeconds(Element));
    }
    for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
    {
        if (Cue.bVisible)
        {
            fEffectDurationSeconds = (std::max)(
                fEffectDurationSeconds,
                Cue.fStartDelaySeconds + Cue.fDurationSeconds);
        }
    }
    m_fPreviewDurationSeconds = fEffectDurationSeconds;
    if (m_ProductPreview.has_value())
    {
		const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
			m_ProductPreview->ProductCue;
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = ProductCue.Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			ProductCue.Cue.iStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			ProductCue.Cue.iEndMs) * 0.001f;
		Timing.bHasCueSourceEnd = EFFECT_STOP_POLICY::CUE_END ==
			ProductCue.Cue.eStopPolicy;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		if (CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.f, Sample))
		{
			m_fPreviewDurationSeconds = Sample.fCueWallStartSeconds +
				fEffectDurationSeconds / Timing.fPlayRate;
			if (Timing.bHasCueSourceEnd)
			{
				m_fPreviewDurationSeconds = (std::max)(
					m_fPreviewDurationSeconds,
					Sample.fCueWallEndSeconds);
			}
		}
		f32_t fClipWallDurationSeconds = 0.f;
		if (Try_ResolvePlayerProductClipWallDuration(
				fClipWallDurationSeconds))
		{
			m_fPreviewDurationSeconds = (std::max)(
				m_fPreviewDurationSeconds, fClipWallDurationSeconds);
		}
    }
	else if (m_ValtanProductPreview.has_value())
	{
		ACTION_PRESENTATION_CUE_PREVIEW_TIMING Timing;
		Timing.fClipSourceStartSeconds = static_cast<f32_t>(
			m_ValtanProductPreview->Clip.iSourceStartMs) * 0.001f;
		Timing.fPlayRate = m_ValtanProductPreview->Clip.fPlayRate;
		Timing.fCueSourceStartSeconds = static_cast<f32_t>(
			m_ValtanProductPreview->Cue.iSourceStartMs) * 0.001f;
		Timing.fCueSourceEndSeconds = static_cast<f32_t>(
			m_ValtanProductPreview->Cue.iSourceEndMs) * 0.001f;
		Timing.bHasCueSourceEnd =
			m_ValtanProductPreview->Cue.bHasSourceEnd;
		ACTION_PRESENTATION_CUE_PREVIEW_SAMPLE Sample;
		if (CActionPresentationTimeline::Resolve_CuePreviewSample(
				Timing, 0.f, Sample))
		{
			m_fPreviewDurationSeconds = Sample.fCueWallStartSeconds +
				fEffectDurationSeconds / Timing.fPlayRate;
			if (Timing.bHasCueSourceEnd)
			{
				m_fPreviewDurationSeconds = (std::max)(
					m_fPreviewDurationSeconds,
					Sample.fCueWallEndSeconds);
			}
		}
		if (0u != m_ValtanProductPreview->Cue.iStageDurationMs)
		{
			/* Valtan Product Play owns one semantic stage occurrence. Source
			   clips may loop inside it, but neither the Tool timeline nor a
			   natural Effect tail may redefine the Server-owned wall window. */
			m_fPreviewDurationSeconds = static_cast<f32_t>(
				m_ValtanProductPreview->Cue.iStageDurationMs) * 0.001f;
		}
	}
    m_fPreviewTimeSeconds = std::clamp(
        m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
}

bool_t Client::CEffect_Tool::Try_ResolvePlayerProductClipWallDuration(
	f32_t& fOutWallDurationSeconds) const
{
	fOutWallDurationSeconds = 0.f;
	if (!m_ProductPreview.has_value() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration())
	{
		return false;
	}

	const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
		m_ProductPreview->ProductCue;
	const ANIMATION_SKILL_CLIP& Clip = ProductCue.Clip;
	const SYNCHRONIZED_ANIMATION_CLIP& Synchronized =
		m_SynchronizedAnimationClips.front();
	if (Synchronized.strClipName != Clip.strClipName ||
		Synchronized.iSourceStartMs != Clip.iSourceStartMs ||
		Synchronized.iPlayMs != Clip.iPlayMs ||
		Synchronized.fPlayRate != Clip.fPlayRate)
	{
		return false;
	}

	const char_t* pExpectedAsset = Animation_AssetName(
		m_ProductPreview->eCharacterClass);
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pExpectedAsset || nullptr == pModel ||
		CAnimationTargetService::Resolve_AssetName() != pExpectedAsset)
	{
		return false;
	}

	uint32_t iAnimation = UINT32_MAX;
	for (uint32_t iCandidate = 0u;
		iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
	{
		const char_t* pName = pModel->Get_AnimationName(iCandidate);
		if (nullptr == pName || Clip.strClipName != pName)
			continue;
		/* Duplicate model clip names make the duration source ambiguous. */
		if (UINT32_MAX != iAnimation)
			return false;
		iAnimation = iCandidate;
	}
	if (UINT32_MAX == iAnimation)
		return false;

	f32_t fPositionTicks = 0.f;
	f32_t fDurationTicks = 0.f;
	const f32_t fTicksPerSecond =
		pModel->Get_AnimationTickPerSecond(iAnimation);
	if (!pModel->Get_AnimationProgress(
			iAnimation, fPositionTicks, fDurationTicks) ||
		!std::isfinite(fDurationTicks) || fDurationTicks <= 0.f ||
		!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
	{
		return false;
	}

	ACTION_PRESENTATION_CLIP_TIMING Timing;
	Timing.fModelSourceDurationSeconds =
		fDurationTicks / fTicksPerSecond;
	Timing.iPlayMs = Clip.iPlayMs;
	Timing.fPlayRate = Clip.fPlayRate;
	Timing.fSourceStartSeconds =
		static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f;
	f32_t fSourceDurationSeconds = 0.f;
	return CActionPresentationTimeline::Resolve_ClipDuration(
		Timing, fSourceDurationSeconds, fOutWallDurationSeconds);
}

bool_t Client::CEffect_Tool::Has_UnsavedWork() const
{
    return m_bDocumentDirty || m_bOccurrenceTuningDirty ||
		Has_UnappliedDetailDraft();
}

void Client::CEffect_Tool::Reset_ModelCueDraft()
{
	m_ModelCueDraft.reset();
	m_bModelCueDraftDirty = false;
	m_ModelCueAssetIdDraft[0u] = '\0';
	m_ModelCueClipNameDraft[0u] = '\0';
	m_strDetailStatus.clear();
}

bool_t Client::CEffect_Tool::Has_UnappliedDetailDraft() const
{
    return m_bParticleSystemDraftDirty || m_bDetailDraftDirty ||
		m_bModelCueDraftDirty || m_bOccurrenceTransformDraftDirty;
}

void Client::CEffect_Tool::Set_ActiveDocumentDrawableStatus(
    const bool_t bDrawable,
    std::string strError)
{
    m_bActiveDocumentDrawable = bDrawable;
    m_strActiveDocumentDrawableError = bDrawable ?
        std::string{} : std::move(strError);
}

void Client::CEffect_Tool::Clear_ActiveDocumentDrawableStatus()
{
    m_bActiveDocumentDrawable = false;
    m_strActiveDocumentDrawableError.clear();
}

void Client::CEffect_Tool::Refresh_RuntimeEquivalence()
{
    m_bActiveDocumentMatchesRuntime = false;
    if (!m_ActiveDocument.has_value() || Has_UnsavedWork())
        return;

    const shared_ptr<const EFFECT_DOCUMENT_DESC> pRuntimeDocument =
        CEffectCatalog::Find(m_ActiveDocument->strEffectAssetId);
    if (nullptr == pRuntimeDocument)
        return;
    if (m_pRuntimeEquivalenceDocument != pRuntimeDocument)
    {
        m_pRuntimeEquivalenceDocument = pRuntimeDocument;
        m_strRuntimeEquivalenceCanonical =
            CEffectDocumentCodec::Serialize(*pRuntimeDocument);
    }

    std::string ActiveCanonicalStorage;
    std::string_view ActiveCanonical = m_strActiveDocumentBaselineCanonical;
    if (ActiveCanonical.empty())
    {
        ActiveCanonicalStorage =
            CEffectDocumentCodec::Serialize(*m_ActiveDocument);
        ActiveCanonical = ActiveCanonicalStorage;
    }
    m_bActiveDocumentMatchesRuntime =
        m_strRuntimeEquivalenceCanonical == ActiveCanonical;
}

// Product Play stages the immutable Runtime Catalog revision, never an unsaved
// Authored draft. Save can replace one exact direct-authored Product target;
// failures leave the previously prepared target available.
std::string Client::CEffect_Tool::Describe_ProductPlaybackAuthoredDivergence(
    const std::string& strProductEffectAssetId)
{
    if (!m_ActiveDocument.has_value() ||
        EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource ||
        m_ActiveDocument->strEffectAssetId != strProductEffectAssetId)
    {
        return {};
    }
    if (Has_UnsavedWork())
    {
        return " | STALE PRODUCT: this plays the loaded Product revision; the "
            "open Authored document has unsaved edits. Save Changes to attempt "
            "an exact selected-target hot reload.";
    }
    Refresh_RuntimeEquivalence();
    if (m_bActiveDocumentMatchesRuntime)
        return {};
    return Can_HotReloadSavedProduct() ?
        " | STALE PRODUCT: this loaded Product revision differs from saved "
            "Authored. Use Retry Product Hot Reload; failure preserves the "
            "existing prepared Product target." :
        " | STALE PRODUCT: this Effect is not an exact direct-authored "
            "Product mapping, so Authored Save does not replace it.";
}

// The session bar states whether the file is saved; this states whether the
// product path would actually play it. Those are separate facts and only the
// second one decides what appears in combat.
// This runs per frame, so it only reads the equivalence flag that the save,
// load, create and edit paths already maintain. Recomputing it here would
// compare two multi-megabyte canonical documents every frame.
const char_t* Client::CEffect_Tool::Runtime_SyncLabel() const
{
    if (!m_ActiveDocument.has_value())
        return "none";
    if (Has_UnsavedWork())
        return "unsaved";
    if (!CEffectCatalog::Contains(m_ActiveDocument->strEffectAssetId))
        return "unpublished";
    return m_bActiveDocumentMatchesRuntime ? "synced" : "STALE";
}

Client::EFFECT_ELEMENT_DESC* Client::CEffect_Tool::Find_SelectedElement()
{
    if (!m_ActiveDocument.has_value())
        return nullptr;
    const auto Iterator = std::find_if(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    return Iterator == m_ActiveDocument->Elements.end() ?
        nullptr : &*Iterator;
}

Client::EFFECT_MODEL_CUE_DESC* Client::CEffect_Tool::Find_SelectedModelCue()
{
	if (!m_ActiveDocument.has_value())
		return nullptr;
	const auto Iterator = std::find_if(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(), [this](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == m_strSelectedModelCueId; });
	return Iterator == m_ActiveDocument->ModelCues.end() ? nullptr : &*Iterator;
}

const Client::EFFECT_ELEMENT_DESC*
Client::CEffect_Tool::Find_SelectedElement() const
{
    if (!m_ActiveDocument.has_value())
        return nullptr;
    const auto Iterator = std::find_if(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    return Iterator == m_ActiveDocument->Elements.end() ?
        nullptr : &*Iterator;
}

const Client::EFFECT_MODEL_CUE_DESC*
Client::CEffect_Tool::Find_SelectedModelCue() const
{
	if (!m_ActiveDocument.has_value())
		return nullptr;
	const auto Iterator = std::find_if(m_ActiveDocument->ModelCues.begin(),
		m_ActiveDocument->ModelCues.end(), [this](const EFFECT_MODEL_CUE_DESC& Cue)
		{ return Cue.strCueId == m_strSelectedModelCueId; });
	return Iterator == m_ActiveDocument->ModelCues.end() ? nullptr : &*Iterator;
}
