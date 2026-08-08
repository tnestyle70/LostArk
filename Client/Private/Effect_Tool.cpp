#include "imgui.h"

#include "Effect_Tool.h"

#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "EffectAuthoringTransfer.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_ThumbnailCache.h"
#include "GameInstance.h"
#include "Logic_Artist.h"
#include "Logic_DimensionMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_LanceMaster.h"
#include "Logic_Slayer.h"
#include "Logic_Warlord.h"
#include "Model.h"
#include "Profiler.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <utility>

namespace
{
    constexpr const wchar_t* PREVIEW_LAYER = L"Layer_EffectPreview";

    const char* Kind_Label(const Client::EFFECT_ELEMENT_KIND eKind)
    {
        switch (eKind)
        {
        case Client::EFFECT_ELEMENT_KIND::MESH: return "Mesh";
        case Client::EFFECT_ELEMENT_KIND::SPRITE: return "Texture";
        case Client::EFFECT_ELEMENT_KIND::PARTICLE: return "Cascade Emitter";
        case Client::EFFECT_ELEMENT_KIND::DECAL: return "Decal";
        case Client::EFFECT_ELEMENT_KIND::TRAIL: return "Trail";
        case Client::EFFECT_ELEMENT_KIND::LIGHT: return "Light";
        case Client::EFFECT_ELEMENT_KIND::SCREEN_POST: return "Screen Post";
        case Client::EFFECT_ELEMENT_KIND::END:
        default: return "Invalid";
        }
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

    const char* Source_Label(const Client::EFFECT_DOCUMENT_SOURCE eSource)
    {
        switch (eSource)
        {
        case Client::EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT: return "New";
        case Client::EFFECT_DOCUMENT_SOURCE::AUTHORED: return "Authored";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED: return "Imported";
        case Client::EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE:
            return "Imported Draft";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY:
			return "Assembly";
		case Client::EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT:
			return "WFX Component";
        case Client::EFFECT_DOCUMENT_SOURCE::END:
        default: return "Invalid";
        }
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
		const Client::EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial)
	{
		const size_t iParameterCount = SourceMaterial.Scalars.size() +
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
			ImGui::TextDisabled("(no named MI parameters captured)");
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

    struct PARTICLE_LAYER_SUMMARY final
    {
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

    f32_t Element_PreviewEndSeconds(
        const Client::EFFECT_ELEMENT_DESC& Element)
    {
        const Client::EFFECT_TIMING_DESC& Timing = Element.Detail.Timing;
        f32_t fTail = 0.f;
        if (Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
            fTail = Element.Detail.Particle.vLifeTimeSeconds.y;
        else if (Client::EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
            fTail = Element.Detail.Trail.fPointLifeTimeSeconds;
        return Timing.fStartDelaySeconds + Timing.fLifeTimeSeconds +
            Timing.fAfterImageSeconds + fTail;
    }

    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId)
    {
        if (strSlotId == Client::EFFECT_MESH_SHAPE_SLOT_ID)
            return Client::EFFECT_ELEMENT_KIND::MESH == Element.eKind ||
                Client::EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
		if (nullptr != Find_Binding(Element, strSlotId))
			return true;
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
        return Client::EFFECT_ELEMENT_KIND::PARTICLE != Element.eKind &&
            Element.strGroupId.starts_with("manual.");
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
    Update_SynchronizedAnimationSequence();
    if (!m_ActiveDocument.has_value())
        return;
    f32_t fSequentialAdvance = 0.f;
    bool_t bSeekAfterLoop = false;
    if (m_bPreviewPlaying)
    {
        const f32_t fPreviousTime = m_fPreviewTimeSeconds;
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
                0.f, m_fPreviewTimeSeconds - fPreviousTime);
    }
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject)
        return;
    float4x4_t Root{};
    const bool_t bRootResolved = Resolve_PreviewRoot(Root);
    pObject->Set_Visible(m_bPreviewVisibleRequested && bRootResolved);
    if (m_bPreviewVisibleRequested && bRootResolved)
    {
        if (0u == m_strPreviewStatus.find("World preview hidden:"))
            m_strPreviewStatus = "World preview anchor resolved.";
    }
    else if (m_bPreviewVisibleRequested)
    {
        m_strPreviewStatus = "World preview hidden: current target cannot resolve " +
            (EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT == m_ePreviewPivotKind ?
                std::string("its root pivot.") :
                std::string("anchor '") + m_strPreviewAnchorSlotId + "'.");
    }
    if (!m_bPreviewVisibleRequested)
		return;
    if (bSeekAfterLoop)
    {
		if (bRootResolved)
			pObject->Set_RootWorld(Root);
        pObject->Set_SampleTime(m_fPreviewTimeSeconds);
        Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
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
        "Build one carrier Mesh layer, then tune it in Effect Detail.");
    const ImGuiIO& IO = ImGui::GetIO();
    ImGui::TextDisabled("FPS %.1f | Frame %.2f ms",
        IO.Framerate,
        IO.DeltaTime > 0.f ? IO.DeltaTime * 1000.f : 0.f);
    Render_MeshAuthoringWorkbench();
    if (ImGui::CollapsingHeader("Edit Selected Layer Resources (Advanced)"))
    {
        ImGui::TextDisabled(
            "Imported Cascade data remains available here as diagnostic input.");
        Render_ResourceSlots(false);
        Render_ResourceGrid(false);
    }
    if (!m_strResourceStatus.empty())
        ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
    ImGui::End();
}

void Client::CEffect_Tool::Render_MeshAuthoringWorkbench()
{
    if (!m_bMeshAuthoringDraftInitialized)
        Reset_MeshAuthoringDraft();
    ImGui::SeparatorText("Mesh Effect Authoring");
    ImGui::Text("Effect Type: Mesh");
    ImGui::SameLine();
    ImGui::TextDisabled(
        "Manual Particle authoring is excluded from this workbench.");
    ImGui::InputText("Effect Name", m_NewAssetId.data(),
        m_NewAssetId.size());
    ImGui::InputText("Display Name (optional)", m_NewDisplayName.data(),
        m_NewDisplayName.size());
    ImGui::InputText("Layer Name (optional)", m_NewElementId.data(),
        m_NewElementId.size());
    ImGui::InputText("Resource Filter", m_ResourceFilter.data(),
        m_ResourceFilter.size());

    const EFFECT_RESOURCE_BINDING_DESC* pMesh = Find_Binding(
        m_MeshAuthoringDraft, EFFECT_MESH_SHAPE_SLOT_ID);
    const EFFECT_RESOURCE_BINDING_DESC* pBase = Find_Binding(
        m_MeshAuthoringDraft,
        EFFECT_STANDARD_MATERIAL_INPUTS[0u].strSlotId);
    const bool_t bUnsafeBase = nullptr != pBase &&
        Is_UnsafeEffectBaseTextureAssetId(pBase->strAssetId);
    const bool_t bHasEffectName = '\0' != m_NewAssetId[0u];
    const bool_t bCanCreate = Is_EffectManualMeshCreateReady(
        m_NewAssetId.data(), nullptr != pMesh, nullptr != pBase,
        bUnsafeBase);

    if (ImGui::Button("Reset"))
        Reset_MeshAuthoringDraft();
    ImGui::SameLine();
    ImGui::BeginDisabled(!bCanCreate);
    if (ImGui::Button("Create Effect"))
        Try_CreateMeshEffect();
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Refresh Resources"))
        Refresh_ResourceCatalog();

    if (!bHasEffectName)
        ImGui::TextDisabled("Enter an Effect Name; Create Effect creates and saves the Data File directly.");
    else if (nullptr == pMesh)
        ImGui::TextDisabled("Create Effect requires one WModel Mesh.");
    else if (nullptr == pBase)
        ImGui::TextDisabled("Create Effect requires one safe 2D Base texture.");
    else if (bUnsafeBase)
        ImGui::TextDisabled("The selected Base looks like blank/normal data and is blocked.");

    Render_ResourceSlots(true);
    Render_ResourceGrid(true);
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
        "Mesh + Material Slots" : "Selected Element Resource Set");
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
				"Cascade System selected | Emitters %zu | Mesh %zu | Sprite %zu | Unresolved %zu",
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
    const auto RenderSlotCard = [this, pElement](
		const EFFECT_RESOURCE_BINDING_DESC* pBinding,
		const std::string& strSlotId,
		const std::string& strLabel,
		const EFFECT_RESOURCE_FILE_KIND eFileKind)
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
        ImGui::EndGroup();
        ImGui::PopID();
    };

    if (bMeshAuthoringDraft)
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
            { "dissolve", "Dissolve", EFFECT_RESOURCE_FILE_KIND::TEXTURE }
        };
        const float fCardWidth = 78.f;
        const size_t iColumns = static_cast<size_t>((std::max)(1,
            static_cast<int32_t>(
                ImGui::GetContentRegionAvail().x / fCardWidth)));
        for (size_t iSlot = 0u;
            iSlot < sizeof(Slots) / sizeof(Slots[0]); ++iSlot)
        {
            if (0u != iSlot % iColumns)
                ImGui::SameLine();
            const AUTHORING_SLOT_CARD& Slot = Slots[iSlot];
            RenderSlotCard(Find_Binding(*pElement, Slot.pSlotId),
                Slot.pSlotId, Slot.pLabel, Slot.eKind);
        }
        if (m_strSelectedResourceSlotId == "meshModel")
            ImGui::TextDisabled("Mesh: one WModel carrier shape (required).");
        else if (m_strSelectedResourceSlotId == "base")
            ImGui::TextDisabled("Base: RGB color and A opacity (required).");
        else if (m_strSelectedResourceSlotId == "noise")
            ImGui::TextDisabled("Noise: RG surface distortion; R also modulates dissolve.");
        else if (m_strSelectedResourceSlotId == "mask")
            ImGui::TextDisabled("Mask: R channel multiplies opacity.");
        else if (m_strSelectedResourceSlotId == "emissive")
            ImGui::TextDisabled("Emissive: RGB adds HDR color using Bloom Intensity.");
        else if (m_strSelectedResourceSlotId == "dissolve")
            ImGui::TextDisabled("Dissolve: R channel is the lifetime threshold.");
        return;
    }

	ImGui::SeparatorText(("Bound Resources (" +
		std::to_string(pElement->ResourceBindings.size()) + ")").c_str());
	const float fCardWidth = 78.f;
	const size_t iColumns = static_cast<size_t>((std::max)(1,
		static_cast<int32_t>(ImGui::GetContentRegionAvail().x / fCardWidth)));
	for (size_t iBinding = 0u;
		iBinding < pElement->ResourceBindings.size(); ++iBinding)
	{
		const EFFECT_RESOURCE_BINDING_DESC& Binding =
			pElement->ResourceBindings[iBinding];
		if (0u != iBinding % iColumns)
			ImGui::SameLine();
		RenderSlotCard(&Binding, Binding.strSlotId,
			Slot_Label(*pElement, Binding.strSlotId),
			Resource_FileKind(Binding));
	}
	if (pElement->ResourceBindings.empty())
		ImGui::TextDisabled("(no resources bound)");

	ImGui::SeparatorText("Available Empty Inputs");
    ImGui::TextDisabled("Template: %s",
        pElement->Material.strTemplateId.c_str());
    const EFFECT_MATERIAL_TEMPLATE_DESC* pTemplate =
        Find_EffectMaterialTemplate(pElement->Material.strTemplateId);
	bool_t bRenderedEmptyInput = false;
	size_t iEmptyInput = 0u;
	const auto RenderEmptyInput = [&](const std::string& strSlotId,
		const std::string& strLabel,
		const EFFECT_RESOURCE_FILE_KIND eFileKind)
    {
		if (nullptr != Find_Binding(*pElement, strSlotId))
			return;
		if (0u != iEmptyInput % iColumns)
			ImGui::SameLine();
		RenderSlotCard(nullptr, strSlotId, strLabel, eFileKind);
		++iEmptyInput;
		bRenderedEmptyInput = true;
	};
	if (EFFECT_ELEMENT_KIND::MESH == pElement->eKind ||
		EFFECT_ELEMENT_KIND::PARTICLE == pElement->eKind)
    {
		RenderEmptyInput(std::string(EFFECT_MESH_SHAPE_SLOT_ID), "Mesh Shape",
			EFFECT_RESOURCE_FILE_KIND::MODEL);
    }
	if (nullptr != pTemplate)
	{
		for (size_t iInput = 0u; iInput < pTemplate->iInputCount; ++iInput)
		{
			RenderEmptyInput(
				std::string(pTemplate->pInputs[iInput].strSlotId),
				std::string(pTemplate->pInputs[iInput].strDisplayName),
				pTemplate->pInputs[iInput].eAllowedResourceKind);
		}
	}
	else
		ImGui::TextDisabled("Unknown Material Template; existing bindings remain visible.");
	if (!bRenderedEmptyInput && nullptr != pTemplate)
		ImGui::TextDisabled("(all declared inputs are bound)");

	Render_SourceMaterialParameterGroups(pElement->Material.SourceMaterial);
}

void Client::CEffect_Tool::Render_ResourceGrid(
    const bool_t bMeshAuthoringDraft)
{
    Engine::CProfilerScope Profile(
        CGameInstance::Get().Get_Profiler(), "EffectTool.ResourceGrid");
    const EFFECT_ELEMENT_DESC* pElement = bMeshAuthoringDraft ?
        &m_MeshAuthoringDraft : Find_SelectedElement();
    const bool_t bSlotSelected = nullptr != pElement &&
        Slot_Allowed(*pElement, m_strSelectedResourceSlotId);
    EFFECT_RESOURCE_FILE_KIND eWanted = bSlotSelected ?
        Slot_FileKind(*pElement, m_strSelectedResourceSlotId) :
        m_eResourceLibraryFileKind;
    if (eWanted >= EFFECT_RESOURCE_FILE_KIND::END)
        eWanted = EFFECT_RESOURCE_FILE_KIND::MODEL;
    const std::string Filter = m_ResourceFilter.data();
    std::string BoundAssetId;
    const EFFECT_RESOURCE_FILE_KIND eSlotFileKind = bSlotSelected ?
        Slot_FileKind(*pElement, m_strSelectedResourceSlotId) :
        EFFECT_RESOURCE_FILE_KIND::END;
    bool_t bCompatibleSlot =
        bSlotSelected && eSlotFileKind == eWanted;
    if (bSlotSelected)
    {
        if (const EFFECT_RESOURCE_BINDING_DESC* pBinding = Find_Binding(
            *pElement, m_strSelectedResourceSlotId))
            BoundAssetId = pBinding->strAssetId;
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
        ImGui::TextDisabled("Selected slot: %s / %s%s",
            Slot_Label(*pElement, m_strSelectedResourceSlotId).c_str(),
            EFFECT_RESOURCE_FILE_KIND::MODEL == eSlotFileKind ?
                "WModel" : "DDS",
            bCompatibleSlot ? "" : " | switch Library kind to bind");
    }
    ImGui::TextDisabled("%s: %zu candidates",
        m_strSelectedAuthoringDomainId.c_str(),
        DomainIterator->ResourceCounts[iFileKind]);
    ImGui::BeginDisabled(!bCompatibleSlot ||
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
    ImGui::BeginDisabled(!bSlotSelected);
    if (ImGui::Button("Clear Slot"))
    {
        if (bMeshAuthoringDraft)
            Try_ClearMeshAuthoringSlot();
        else
            Try_ClearSelectedSlot();
    }
    ImGui::EndDisabled();

    const float CardWidth = 92.f;
    const int32_t Columns = (std::max)(1,
        static_cast<int32_t>(ImGui::GetContentRegionAvail().x / CardWidth));
    Rebuild_ResourceBrowserView(eWanted, Filter,
        m_strSelectedAuthoringDomainId, Category,
        bMeshAuthoringDraft ? m_strMeshShapeCategory : "All");
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
    const std::string& strShapeCategory)
{
    if (m_iResourceViewRevision == m_iResourceCatalogRevision &&
        m_eResourceViewFileKind == eFileKind &&
        m_strResourceViewFilter == strFilter &&
        m_strResourceViewDomainId == strDomainId &&
        m_strResourceViewCategory == strCategory &&
        m_strResourceViewShapeCategory == strShapeCategory)
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
                    Entry.strAssetId, strShapeCategory)) ||
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
    m_strResourceViewShapeCategory = strShapeCategory;
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
    m_pCharacterPreviewPanel->Render_Selector(false, {}, false);
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    Render_AnimationControls(pModel);

    ImGui::SeparatorText("Effect Pivot");
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
    const bool_t bCanTransfer = m_ActiveDocument.has_value() &&
        !Has_UnsavedWork() &&
        m_bActiveDocumentMatchesRuntime &&
        EFFECT_PREVIEW_PIVOT_KIND::WORLD != m_ePreviewPivotKind &&
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
        ImGui::TextDisabled(
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
        "Particles Only", EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM);
    ImGui::SameLine();
	SelectPreviewFilter(
		"Mesh Emitters", EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS);
	ImGui::SameLine();
	SelectPreviewFilter(
		"Sprite Emitters", EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS);
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
        m_strSelectedElementId.empty())
        ImGui::TextDisabled("Select an Element for Solo/Mute preview.");
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == m_ePreviewFilter) &&
        m_strSelectedElementGroupId.empty())
    {
        ImGui::TextDisabled(
            "Select one grouped Element before using Group Solo/Mute.");
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
            if (m_fPreviewTimeSeconds >= m_fPreviewDurationSeconds)
                Start_WorldPreviewFromBeginning();
            else
            {
				m_bPreviewVisibleRequested = true;
				m_bPreviewPlaying = true;
                Set_SynchronizedAnimationPaused(false);
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Loop", &m_bPreviewLoop))
    {
        Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
        Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart + Play"))
        Start_WorldPreviewFromBeginning();
    ImGui::Text("World Preview: %s | %.3f / %.3f s",
        m_bPreviewPlaying ? "PLAYING" : "PAUSED",
        m_fPreviewTimeSeconds, m_fPreviewDurationSeconds);
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
        if (const shared_ptr<CEffectObject> pObject =
            m_pWorldPreviewObject.lock())
            pObject->Set_SampleTime(m_fPreviewTimeSeconds);
        Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds);
        Set_SynchronizedAnimationPaused(true);
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
    uint32_t iCurrent = pModel->Get_CurrentAnimIndex();
    const char* pCurrentName = pModel->Get_AnimationName(iCurrent);
    const char* pCurrentLabel =
        iCurrent < m_AnimationClipDisplayLabels.size() ?
            m_AnimationClipDisplayLabels[iCurrent].c_str() : pCurrentName;
    if (ImGui::BeginCombo("Animation Clip",
        nullptr != pCurrentLabel ? pCurrentLabel : "Invalid"))
    {
        for (uint32_t iAnimation = 0u;
            iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
        {
            const char* pName = pModel->Get_AnimationName(iAnimation);
            const char* pLabel =
                iAnimation < m_AnimationClipDisplayLabels.size() ?
                    m_AnimationClipDisplayLabels[iAnimation].c_str() : pName;
            if (nullptr != pName && nullptr != pLabel && ImGui::Selectable(
                pLabel, iAnimation == iCurrent))
            {
                Reset_SynchronizedAnimationSequence();
                pModel->Start_Animation(iAnimation, true);
                pModel->Set_AnimPaused(false);
                iCurrent = iAnimation;
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button("Reload Skill Labels"))
        Refresh_AnimationClipLabels(pModel, true);
    ImGui::SameLine();
    ImGui::TextDisabled("[Input] Korean Skill Name | Model Clip");
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
	case EFFECT_DETAIL_SELECTION::NONE:
	case EFFECT_DETAIL_SELECTION::END:
	default: break;
	}
	ImGui::Text("Level: %s", pSelectionLevel);
	ImGui::TextWrapped("Skill / Document: %s",
		m_ActiveDocument->strEffectAssetId.c_str());
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
		m_bDetailDraftDirty = false;
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
	bool_t bChanged = false;
	Render_SourceModuleDetail(*Module, bChanged, true);
	if (bChanged)
	{
		m_bDetailDraftDirty = true;
		m_strDetailStatus =
			"Live source Module preview; Apply commits it to active Document memory.";
		Stage_DetailDraftPreview();
	}
	ImGui::Separator();
	ImGui::BeginDisabled(!m_bDetailDraftDirty);
	if (ImGui::Button("Apply Module"))
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if (Apply_DetailDraft(Staged) && Try_CommitDocument(std::move(Staged)))
		{
			m_bDetailDraftDirty = false;
			if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
				m_DetailDraft = *pCommitted;
			m_strDetailStatus =
				"Applied source Module to active Document memory; Save required.";
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert Module"))
	{
		m_DetailDraft = *pCurrent;
		m_bDetailDraftDirty = false;
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
	const bool_t bDrawable = m_bActiveDocumentDrawable;
	const bool_t bLivePreview = bDrawable &&
		m_bPreviewVisibleRequested &&
		nullptr != m_pWorldPreviewObject.lock();
	ImGui::SeparatorText("Restoration Session");
	ImGui::Text("Source: %s | Draft: %s | Document: %s | Preview: %s",
		Source_Label(m_eActiveDocumentSource),
		Has_UnappliedDetailDraft() ? "UNAPPLIED" : "committed",
		m_bDocumentDirty ? "UNSAVED" : "saved",
		bLivePreview ? "LIVE" : "HIDDEN");
	ImGui::BeginDisabled(!bEditableSource || !Has_UnsavedWork());
	if (ImGui::Button("Apply + Save Authored"))
		Try_ApplyDraftAndSave();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bEditableSource ||
		Has_UnappliedDetailDraft() || !m_bDocumentDirty);
	if (ImGui::Button("Save Authored"))
		Try_SaveDocument();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(Has_UnsavedWork());
	if (ImGui::Button("Reload Saved"))
		Try_ReloadActiveDocument();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Restart Preview"))
		Start_WorldPreviewFromBeginning();
	if (!bEditableSource)
	{
		ImGui::TextDisabled(
			"Imported/runtime views use Promote or Save As before authoring save.");
	}
	else if (!bDrawable)
	{
		ImGui::TextWrapped(
			"Structurally valid partial draft. Preview hidden and publish blocked: %s",
			m_strActiveDocumentDrawableError.c_str());
	}
	else if (m_bActiveDocumentMatchesRuntime)
	{
		ImGui::TextDisabled(
			"Saved Authored matches the Runtime Catalog snapshot loaded in this process.");
	}
	else
	{
		ImGui::TextDisabled(
			"World preview uses active Authored; Assembly/WFX publish and Runtime Catalog reload are pending.");
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
    if (!m_ActiveDocument.has_value())
    {
        Reset_ParticleSystemDraft();
        Reset_DetailDraft();
        m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
        ImGui::TextDisabled(
            "Select a Particle System or one Element in All Effects.");
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
    const EFFECT_ELEMENT_DESC* pCurrent = Find_SelectedElement();
	const bool_t bElementOrEmitter =
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ||
		EFFECT_DETAIL_SELECTION::EMITTER == m_eDetailSelection;
    if (!bElementOrEmitter ||
        nullptr == pCurrent)
    {
        ImGui::TextDisabled(
			"Select Skill, Particle System, Component, Emitter, or Module in All Effects.");
        ImGui::End();
        return;
    }
	if (EFFECT_DETAIL_SELECTION::EMITTER == m_eDetailSelection)
		Render_EmitterSelectionDetail();
    if (!m_DetailDraft.has_value() ||
        m_strDetailDraftElementId != pCurrent->strElementId)
    {
        m_DetailDraft = *pCurrent;
        m_strDetailDraftElementId = pCurrent->strElementId;
        m_bDetailDraftDirty = false;
        m_strDetailStatus.clear();
    }
    const f32_t fElementStart =
        m_DetailDraft->Detail.Timing.fStartDelaySeconds;
    ImGui::TextDisabled("Visible timeline: %.3f - %.3f s",
        fElementStart, Element_PreviewEndSeconds(*m_DetailDraft));
    bool_t bChanged = false;
    ImGui::TextDisabled(
        "Drag numeric values for live world preview; Apply commits the draft.");
    Render_Detail(*m_DetailDraft, bChanged);
    if (bChanged)
    {
        m_bDetailDraftDirty = true;
        m_strDetailStatus =
            "Live preview only; Apply Detail commits this draft to memory.";
        Stage_DetailDraftPreview();
    }
    ImGui::Separator();
    ImGui::BeginDisabled(!m_bDetailDraftDirty);
    if (ImGui::Button("Apply Detail"))
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
            if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
                m_DetailDraft = *pCommitted;
            m_strDetailStatus =
                "Applied to active Document memory; Save required to persist.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert Detail"))
    {
        m_DetailDraft = *pCurrent;
        m_bDetailDraftDirty = false;
        Recalculate_PreviewDuration();
        if (Stage_WorldPreview())
            m_strPreviewStatus =
                "Detail draft reverted to the active Document preview.";
        m_strDetailStatus =
            "Reverted the Detail draft to the active Document.";
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Audition Selected"))
        Try_AuditionSelectedElement();
    if (m_bDetailDraftDirty)
        ImGui::TextDisabled("Detail draft is local until Apply Detail.");
    if (!m_strDetailStatus.empty())
        ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
    ImGui::End();
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
        "Mesh Renderers %zu | Sprite Renderers %zu | Unresolved %zu | Budget %llu",
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
	if (ImGui::Button("Play Mesh Emitters") &&
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
    if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
        ImGui::Text("Element: %s (%s / %s)", Element.strElementId.c_str(),
            Kind_Label(Element.eKind), Element_RendererLabel(Element));
    else
        ImGui::Text("Element: %s (%s)", Element.strElementId.c_str(),
            Kind_Label(Element.eKind));
    ImGui::Text("Display Name: %s", Element.strDisplayName.c_str());
    ImGui::TextDisabled("Group: %s",
        Element.strGroupId.empty() ? "(none)" : Element.strGroupId.c_str());
    ImGui::TextDisabled("Source Node: %s",
        Element.strSourceNode.empty() ? "(authored)" :
            Element.strSourceNode.c_str());
    bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
    ImGui::TextDisabled("Material Template: %s",
        Element.Material.strTemplateId.c_str());
    EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
        Element.Material.SourceMaterial;
    if (SourceMaterial.bEnabled &&
        ImGui::CollapsingHeader(
            "Source Material Profile", ImGuiTreeNodeFlags_DefaultOpen))
    {
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
        ImGui::TextDisabled(
            "Dynamic: X=%s Y=%s Z=%s W=%s",
            SourceMaterial.DynamicParameterSemantics[0].c_str(),
            SourceMaterial.DynamicParameterSemantics[1].c_str(),
            SourceMaterial.DynamicParameterSemantics[2].c_str(),
            SourceMaterial.DynamicParameterSemantics[3].c_str());
        ImGui::TextDisabled(
            "Parent/source identity and parameter names are provenance. Values below are the Authored effective copy initialized from Imported.");
        bool_t bMaterialChanged = false;
        if (ImGui::BeginCombo("Finite Runtime Shader",
            SourceMaterial.strRuntimeShaderProfileId.c_str()))
        {
            for (const std::string_view strProfileId :
                EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS)
            {
                const bool_t bSelected =
                    SourceMaterial.strRuntimeShaderProfileId == strProfileId;
                if (ImGui::Selectable(strProfileId.data(), bSelected))
                {
                    SourceMaterial.strRuntimeShaderProfileId = strProfileId;
                    bMaterialChanged = true;
                }
                if (bSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::TreeNodeEx("Authored Named Parameters",
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (size_t iScalar = 0u;
                iScalar < SourceMaterial.Scalars.size(); ++iScalar)
            {
                EFFECT_NAMED_FLOAT_DESC& Scalar = SourceMaterial.Scalars[iScalar];
                ImGui::PushID(static_cast<int>(iScalar));
                bMaterialChanged |= ImGui::DragFloat(
                    Scalar.strName.c_str(), &Scalar.fValue, 0.001f,
                    -100000.f, 100000.f, "%.6g");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Source group: %s",
                        Scalar.strGroup.empty() ? "(none)" :
                            Scalar.strGroup.c_str());
                ImGui::PopID();
            }
            for (size_t iVector = 0u;
                iVector < SourceMaterial.Vectors.size(); ++iVector)
            {
                EFFECT_NAMED_FLOAT4_DESC& Vector = SourceMaterial.Vectors[iVector];
                ImGui::PushID(static_cast<int>(SourceMaterial.Scalars.size() +
                    iVector));
                bMaterialChanged |= DragFloat4(
                    Vector.strName.c_str(), Vector.vValue, 0.001f,
                    -100000.f, 100000.f, "%.6g");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Source group: %s",
                        Vector.strGroup.empty() ? "(none)" :
                            Vector.strGroup.c_str());
                ImGui::PopID();
            }
            for (size_t iSwitch = 0u;
                iSwitch < SourceMaterial.StaticSwitches.size(); ++iSwitch)
            {
                EFFECT_NAMED_BOOL_DESC& Switch =
                    SourceMaterial.StaticSwitches[iSwitch];
                ImGui::PushID(static_cast<int>(SourceMaterial.Scalars.size() +
                    SourceMaterial.Vectors.size() + iSwitch));
                bMaterialChanged |= ImGui::Checkbox(
                    Switch.strName.c_str(), &Switch.bValue);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Source group: %s",
                        Switch.strGroup.empty() ? "(none)" :
                            Switch.strGroup.c_str());
                ImGui::PopID();
            }
            if (SourceMaterial.Scalars.empty() &&
                SourceMaterial.Vectors.empty() &&
                SourceMaterial.StaticSwitches.empty())
            {
                ImGui::TextDisabled("No extracted named values for this Material.");
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Dynamic / SubUV Binding"))
        {
            constexpr const char* Axes[] = { "X", "Y", "Z", "W" };
            for (size_t iSemantic = 0u;
                iSemantic < SourceMaterial.DynamicParameterSemantics.size();
                ++iSemantic)
            {
                ImGui::PushID(static_cast<int>(iSemantic));
                std::string& Semantic =
                    SourceMaterial.DynamicParameterSemantics[iSemantic];
                if (ImGui::BeginCombo(Axes[iSemantic], Semantic.c_str()))
                {
                    for (const std::string_view strSemantic :
                        EFFECT_SOURCE_DYNAMIC_PARAMETER_SEMANTICS)
                    {
                        const bool_t bSelected = Semantic == strSemantic;
                        if (ImGui::Selectable(strSemantic.data(), bSelected))
                        {
                            Semantic = strSemantic;
                            bMaterialChanged = true;
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();
            }
            if (ImGui::BeginCombo("SubUV Mode",
                SourceMaterial.strSubUVMode.c_str()))
            {
                for (const std::string_view strMode : EFFECT_SOURCE_SUBUV_MODES)
                {
                    const bool_t bSelected =
                        SourceMaterial.strSubUVMode == strMode;
                    if (ImGui::Selectable(strMode.data(), bSelected))
                    {
                        SourceMaterial.strSubUVMode = strMode;
                        bMaterialChanged = true;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TreePop();
        }
        if (bMaterialChanged)
        {
            SourceMaterial.eStatus =
                EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE;
            bChanged = true;
        }
        ImGui::TextDisabled(
            "Mesh/Base/Noise/Mask/Emissive/Dissolve remain typed Resource Library bindings. The Imported file is never modified by this editor.");
    }
    if (ImGui::CollapsingHeader(
        "Runtime Sample", ImGuiTreeNodeFlags_DefaultOpen))
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
    Render_TransformDetail(Element.Detail, bChanged);
    Render_ColorDetail(Element.Detail, bChanged,
        nullptr != Find_Binding(Element, "emissive"));
    Render_UVDetail(Element.Detail, bChanged);
    Render_UVKeyframes(Element, bChanged);
    Render_TimingDetail(Element.Detail, bChanged);
    Render_KindDetail(Element, bChanged);
    Render_SourceRecipeDetail(Element.SourceRecipe, bChanged);
    Render_LerpDetail(Element.Detail, bChanged);

    ImGui::SeparatorText("Pass Name");
    if (ImGui::BeginCombo("Render Profile",
        Profile_Label(Element.Material.eRenderProfile)))
    {
        for (int32_t iProfile = 0;
            iProfile < static_cast<int32_t>(EFFECT_RENDER_PROFILE::END);
            ++iProfile)
        {
            const EFFECT_RENDER_PROFILE eProfile =
                static_cast<EFFECT_RENDER_PROFILE>(iProfile);
            if (ImGui::Selectable(Profile_Label(eProfile),
                eProfile == Element.Material.eRenderProfile))
            {
                Element.Material.eRenderProfile = eProfile;
                bChanged = true;
            }
        }
        ImGui::EndCombo();
    }
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
    bChanged |= ImGui::DragFloat("Bloom Intensity",
        &Detail.Color.fEmissiveIntensity, 0.05f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndDisabled();
    if (!bHasEmissiveTexture)
        ImGui::TextDisabled("Bloom Intensity is inactive until an Emissive texture is bound.");
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
                    pObject->Set_SampleTime(0.f);
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
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Timing", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= ImGui::DragFloat("Life Time",
        &Detail.Timing.fLifeTimeSeconds, 0.01f, 0.001f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("Start Delay Timer",
        &Detail.Timing.fStartDelaySeconds, 0.01f, 0.f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::DragFloat("After Image Timer",
        &Detail.Timing.fAfterImageSeconds, 0.01f, 0.f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::SliderFloat("Dissolve Start",
        &Detail.Timing.fDissolveStartNormalized, 0.f, 1.f);
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
        break;
    case EFFECT_ELEMENT_KIND::SPRITE:
        bChanged |= ImGui::Checkbox("Billboard",
            &Detail.Sprite.bBillboard);
        break;
    case EFFECT_ELEMENT_KIND::DECAL:
        bChanged |= DragFloat2(
            "Decal Size", Detail.Decal.vSize, 0.01f, 0.001f, 1000.f);
        bChanged |= ImGui::DragFloat("Decal Projection Depth",
            &Detail.Decal.fDepth, 0.01f, 0.001f, 1000.f, "%.3f",
            ImGuiSliderFlags_AlwaysClamp);
        break;
    case EFFECT_ELEMENT_KIND::PARTICLE:
    {
        bChanged |= ImGui::InputScalar("Max Particles",
            ImGuiDataType_U32, &Detail.Particle.iMaxParticles);
        bChanged |= ImGui::DragFloat("Spawn Rate / Second",
            &Detail.Particle.fSpawnRatePerSecond, 1.f, 0.f, 2048.f, "%.3f",
            ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::InputScalar("Burst Count",
            ImGuiDataType_U32, &Detail.Particle.iBurstCount);
        bChanged |= ImGui::InputScalar("Random Seed",
            ImGuiDataType_U32, &Detail.Particle.iRandomSeed);
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
        bChanged |= DragFloat2(
            "Start Size", Detail.Particle.vStartSize, 0.01f, 0.001f, 100.f);
        bChanged |= DragFloat2(
            "End Size", Detail.Particle.vEndSize, 0.01f, 0.f, 100.f);
        bChanged |= ImGui::Checkbox("Particle Local Space",
            &Detail.Particle.bLocalSpace);
        bChanged |= ImGui::Checkbox("Particle Billboard",
            &Detail.Particle.bBillboard);
        break;
    }
    case EFFECT_ELEMENT_KIND::TRAIL:
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
        bChanged |= ImGui::DragFloat("Trail Start Width",
            &Detail.Trail.fStartWidth,
            0.01f, 0.001f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::DragFloat("Trail End Width",
            &Detail.Trail.fEndWidth,
            0.01f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bChanged |= ImGui::Checkbox("Trail Faces Camera",
            &Detail.Trail.bFaceCamera);
        break;
    case EFFECT_ELEMENT_KIND::LIGHT:
        ImGui::TextWrapped(
            "Light execution is described by the original typed module stack below. "
            "Transform, color, timing, and every source distribution remain editable.");
        break;
    case EFFECT_ELEMENT_KIND::SCREEN_POST:
        ImGui::TextWrapped(
            "Screen Post execution is described by the original typed module stack below. "
            "It is a presentation channel and is not rendered as a Particle sprite.");
        break;
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
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Original Emitter / Module Stack",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    ImGui::TextDisabled(
        "Imported UE3 values. Editing changes only the Authored document; "
        "the Imported baseline remains unchanged.");
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
    RenderLerpToggle("Lerp Bloom Intensity", Lerp.bEmissiveIntensity);
    if (Lerp.bEmissiveIntensity)
        bChanged |= ImGui::DragFloat("Bloom Intensity End",
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
		std::to_string(ParticleSummary.iSourceEmitterCount) + " | Mesh " +
		std::to_string(ParticleSummary.iMeshRendererCount) + " | Sprite " +
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
                bool_t bCanPreview = m_ActiveDocument.has_value() &&
                    m_ActiveDocument->strEffectAssetId == strEffectAssetId;
                if (!bCanPreview && !Elements.empty())
                {
                    bCanPreview = Try_SelectElement(
                        strEffectAssetId, Elements.front()->strElementId);
                }
                if (bCanPreview)
                {
                    m_strSelectedElementGroupId = strGroupId;
                    if (Try_SetPreviewFilter(
                        EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP))
                    {
                        Start_WorldPreviewFromBeginning();
                    }
                }
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
                        ("Solo##" + pElement->strElementId).c_str()) &&
                        Try_SelectElement(
                            strEffectAssetId, pElement->strElementId) &&
                        Try_SetPreviewFilter(
                            EFFECT_PREVIEW_FILTER::SOLO_SELECTED))
                    {
                        Start_WorldPreviewFromBeginning();
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
    constexpr LostArk::Shared::CHARACTER_CLASS_ID Classes[] = {
        LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
        LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
        LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
        LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
        LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
        LostArk::Shared::CHARACTER_CLASS_ID::WARLORD };
    if (ImGui::BeginCombo("Class", Class_Label(m_eAllEffectsClass)))
    {
        for (const auto eClass : Classes)
        {
            if (ImGui::Selectable(Class_Label(eClass),
                eClass == m_eAllEffectsClass))
            {
                m_eAllEffectsClass = eClass;
                Select_AuthoringDomainForClass(eClass);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::InputText("Search", m_AllEffectsSearch.data(),
        m_AllEffectsSearch.size());
    ImGui::TextDisabled(
        "Authored Mesh layers are the primary outliner; imported Cascade remains diagnostic.");
    ImGui::BeginDisabled(!m_ActiveDocument.has_value());
    if (ImGui::Button("Play Complete Effect") &&
        Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
    {
        Start_WorldPreviewFromBeginning();
    }
    ImGui::SameLine();
	if (ImGui::Button("Play Mesh Emitters") &&
		Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS))
	{
		Start_WorldPreviewFromBeginning();
	}
	ImGui::SameLine();
    if (ImGui::Button("Hide Preview"))
        Hide_WorldPreview();
    ImGui::EndDisabled();
    if (ImGui::CollapsingHeader("Catalog / Destructive Edit Commands"))
    {
        if (ImGui::Button("Refresh Skill Catalog"))
            Refresh_AllEffects(true);
        ImGui::SameLine();
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
        const EFFECT_DOCUMENT_DESC* pIndexedDocument =
            nullptr != Entry.pRuntimeDocument ?
                Entry.pRuntimeDocument.get() : nullptr;
        if (Entry.Skill.eCharacterClass != m_eAllEffectsClass ||
            (!Contains_NoCase(Entry.Skill.strInputSlot, Search) &&
             !Contains_NoCase(Entry.Skill.strDisplayName, Search) &&
             !Contains_NoCase(Entry.Skill.strEffectId, Search) &&
             (nullptr == pIndexedDocument || !Contains_NoCase(
                 pIndexedDocument->strDisplayName, Search))))
            continue;
        if (m_ActiveDocument.has_value() &&
            m_ActiveDocument->strEffectAssetId == Entry.Skill.strEffectId)
            bActiveAppearsInTree = true;
        const EFFECT_DOCUMENT_DESC* pTreeDocument =
            m_ActiveDocument.has_value() &&
                m_ActiveDocument->strEffectAssetId == Entry.Skill.strEffectId ?
            &*m_ActiveDocument : pIndexedDocument;
        const PARTICLE_LAYER_SUMMARY ParticleSummary =
            nullptr == pTreeDocument ? PARTICLE_LAYER_SUMMARY{} :
                Summarize_ParticleLayers(*pTreeDocument);
        ImGui::PushID(static_cast<int32_t>(Entry.Skill.iSkillId));
        const bool_t bActiveSkill = m_ActiveDocument.has_value() &&
            m_ActiveDocument->strEffectAssetId == Entry.Skill.strEffectId;
		const bool_t bSkillSelected = bActiveSkill &&
			EFFECT_DETAIL_SELECTION::SKILL == m_eDetailSelection;
        const std::string SkillLabel = "Skill | " + Entry.Skill.strInputSlot +
			" | " + Entry.Skill.strDisplayName + " (" +
			Entry.Skill.strEffectId + ")" +
			(bActiveSkill ? " [loaded]" : "");
        const ImGuiTreeNodeFlags SkillFlags =
            ImGuiTreeNodeFlags_OpenOnArrow |
			(bActiveSkill ? ImGuiTreeNodeFlags_DefaultOpen : 0) |
			(bSkillSelected ? ImGuiTreeNodeFlags_Selected : 0);
        const bool_t bSkillOpen = ImGui::TreeNodeEx(
            SkillLabel.c_str(), SkillFlags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            Try_SelectSkill(Entry.Skill.strEffectId);
        if (ImGui::IsItemHovered())
        {
            if (nullptr != pTreeDocument)
            {
                ImGui::SetTooltip(
                    "%zu Elements in the indexed Effect Document.\n"
                    "Cascade System: Source Systems %zu | Emitters %zu | Layers %zu\n"
                    "Mesh %zu | Sprite %zu | Unresolved %zu | Budget %llu\n"
                    "Click the skill label to load the Authored document.",
                    pTreeDocument->Elements.size(),
                    ParticleSummary.iSourceSystemCount,
                    ParticleSummary.iSourceEmitterCount,
                    ParticleSummary.iLayerCount,
                    ParticleSummary.iMeshRendererCount,
                    ParticleSummary.iSpriteRendererCount,
                    ParticleSummary.iUnresolvedRendererCount,
                    static_cast<unsigned long long>(
                        ParticleSummary.iParticleBudget));
            }
            else
            {
                ImGui::SetTooltip(
                    "Authored data is indexed but has no published Runtime snapshot.\n"
                    "Load it to validate and inspect its Elements.");
            }
        }
        if (bSkillOpen)
        {
			if (ImGui::Button(bActiveSkill ?
				"Play Loaded Complete Skill" : "Load / Play Complete Skill"))
			{
				if (Try_SelectSkill(Entry.Skill.strEffectId) &&
					Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
				{
					Start_WorldPreviewFromBeginning();
				}
			}
			const bool_t bHasPublishedAssembly = nullptr !=
				CEffectCatalog::Find_Assembly(Entry.Skill.strEffectId);
			if (bHasPublishedAssembly && !bActiveSkill)
			{
				Render_AssemblyHierarchy(Entry.Skill.strEffectId);
				ImGui::TreePop();
				ImGui::PopID();
				continue;
			}
			if (bHasPublishedAssembly && bActiveSkill && ImGui::TreeNode(
				"Published Runtime Hierarchy (diagnostic)"))
			{
				Render_AssemblyHierarchy(Entry.Skill.strEffectId);
				ImGui::TreePop();
			}
            if (nullptr == pTreeDocument)
            {
                ImGui::TextDisabled(
                    "Load the Authored document to inspect unpublished layers.");
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
                TreeDocument, Entry.Skill.strEffectId, bActiveSkill);
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
                        " | Mesh " +
                        std::to_string(ParticleSummary.iMeshRendererCount) +
                        " | Sprite " +
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
                    const bool_t bSystemSelected = bActiveSkill &&
                        EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM ==
                            m_eDetailSelection;
                    bKindOpen = ImGui::TreeNodeEx(
                        KindLabel.c_str(),
                        ImGuiTreeNodeFlags_OpenOnArrow |
                        (bSystemSelected ? ImGuiTreeNodeFlags_Selected : 0));
                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                        Try_SelectParticleSystem(Entry.Skill.strEffectId);
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
                    &Entry, eKind](const CASCADE_RENDERER_KIND* pRendererKind)
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
                                Entry.Skill.strEffectId &&
                            EFFECT_DETAIL_SELECTION::ELEMENT ==
                                m_eDetailSelection &&
                            Element.strElementId == m_strSelectedElementId;
                        std::string Label = Element.strDisplayName + "##" +
                            Element.strElementId;
                        if (!Element.strGroupId.empty())
                            Label = "[" + Element.strGroupId + "] " + Label;
                        if (ImGui::Selectable(Label.c_str(), bSelected))
                            Try_SelectElement(Entry.Skill.strEffectId,
                                Element.strElementId);
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
                        const std::string RendererLabel = std::string(
                            CascadeRenderer_Label(eRendererKind)) + "s (" +
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
            "Active Effect Document / Imported Diagnostics",
            ImGuiTreeNodeFlags_DefaultOpen |
            ImGuiTreeNodeFlags_OpenOnArrow))
    {
        const PARTICLE_LAYER_SUMMARY ParticleSummary =
            Summarize_ParticleLayers(*m_ActiveDocument);
        Render_ManualElementGroups(*m_ActiveDocument,
            m_ActiveDocument->strEffectAssetId, true);
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
                    " | Mesh " +
                    std::to_string(ParticleSummary.iMeshRendererCount) +
                    " | Sprite " +
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
                if (ImGui::Selectable(Element.strDisplayName.c_str(),
                    EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
                    Element.strElementId == m_strSelectedElementId))
                    Try_SelectElement(
                        m_ActiveDocument->strEffectAssetId,
                        Element.strElementId);
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
        m_AnimationClipDisplayLabels.size() == iAnimationCount)
    {
        return;
    }

    m_iAnimationClipLabelTargetGeneration = iTargetGeneration;
    m_AnimationClipDisplayLabels.clear();
    m_AnimationClipDisplayLabels.reserve(iAnimationCount);
    for (uint32_t iAnimation = 0u;
        iAnimation < iAnimationCount; ++iAnimation)
    {
        const char* pName = pModel->Get_AnimationName(iAnimation);
        m_AnimationClipDisplayLabels.emplace_back(
            nullptr == pName ? "Invalid" : pName);
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
            "Mesh Create Effect saves directly; New Empty is for exceptional authoring only.");
        if (ImGui::Button("New Empty"))
            Try_CreateDocument();
        ImGui::SameLine();
		const bool_t bRuntimeView =
			EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == m_eActiveDocumentSource ||
			EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource;
		ImGui::BeginDisabled(bRuntimeView);
        if (ImGui::Button("Save Document"))
            Try_SaveDocument();
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
            "Cascade System: Source Systems %zu | Emitters %zu | Layers %zu | Mesh %zu | Sprite %zu | Unresolved %zu | Budget %llu",
            ParticleSummary.iSourceSystemCount,
            ParticleSummary.iSourceEmitterCount,
            ParticleSummary.iLayerCount,
            ParticleSummary.iMeshRendererCount,
            ParticleSummary.iSpriteRendererCount,
            ParticleSummary.iUnresolvedRendererCount,
            static_cast<unsigned long long>(
                ParticleSummary.iParticleBudget));
    }
    ImGui::InputTextWithHint("##DataFilesSearch",
        "Filter Effect Asset ID", m_DataFilesSearch.data(),
        m_DataFilesSearch.size());
    const std::string strDataFileSearch = m_DataFilesSearch.data();
    const size_t iVisibleDataFiles = static_cast<size_t>(std::count_if(
        m_DataFiles.begin(), m_DataFiles.end(),
        [this, &strDataFileSearch](const EFFECT_DATA_FILE_ENTRY& Entry)
        {
            return Entry.strDomainId == m_strSelectedAuthoringDomainId &&
                Contains_NoCase(Entry.strAssetId, strDataFileSearch);
        }));
    ImGui::TextDisabled("%s: %zu matching Data Files",
        m_strSelectedAuthoringDomainId.c_str(), iVisibleDataFiles);
    ImGui::BeginChild("EffectDataFileList", ImVec2(0.f, 130.f), true);
    for (const EFFECT_DATA_FILE_ENTRY& Entry : m_DataFiles)
    {
        if (Entry.strDomainId != m_strSelectedAuthoringDomainId ||
            !Contains_NoCase(Entry.strAssetId, strDataFileSearch))
            continue;
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
                    "Selected extraction draft is not a runtime Effect "
                    "Document. Use the matching Authored row for playback.";
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
    const auto SelectedDataFile = std::find_if(
        m_DataFiles.begin(), m_DataFiles.end(),
        [this](const EFFECT_DATA_FILE_ENTRY& Entry)
        {
            return Entry.strAssetId == m_strSelectedDataFileAssetId;
        });
    const bool_t bSelectedFileLoadable =
        SelectedDataFile != m_DataFiles.end() &&
        EFFECT_DOCUMENT_SOURCE::IMPORTED_REFERENCE !=
            SelectedDataFile->eSource;
    ImGui::BeginDisabled(!bSelectedFileLoadable);
    if (ImGui::Button("Load Document") &&
        SelectedDataFile != m_DataFiles.end())
    {
        Try_LoadDocumentPath(
            SelectedDataFile->Path,
            SelectedDataFile->eSource,
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
        Refresh_DataFiles();
    ImGui::TextDisabled(
        "Load = inspect saved data without autoplay | Play Complete/Group/Solo starts preview");
    ImGui::TextDisabled(
        "Unload = remove it from the screen, never delete the file");
    ImGui::TextDisabled(
        "Imported Draft rows are extraction reference only; load the matching Authored row.");
    Render_LoadedEffectContents();
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
    m_ActiveDocument = std::move(Document);
    Set_ActiveDocumentDrawableStatus(bDrawable, std::move(DrawableError));
    m_ActiveDocumentPath.clear();
	m_strActiveDocumentBaselineCanonical.clear();
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_bDocumentDirty = true;
    m_bActiveDocumentMatchesRuntime = false;
    m_fPreviewTimeSeconds = 0.f;
    Recalculate_PreviewDuration();
    Stage_WorldPreview();
    m_strDocumentStatus =
        "Created an empty v12 Effect Document in memory; Mesh builder selections were preserved.";
    return true;
}

bool_t Client::CEffect_Tool::Try_CreateMeshEffect()
{
    if (!m_bResourceCatalogRefreshAttempted && !Refresh_ResourceCatalog())
        return false;

    const std::string strTargetEffectId = m_NewAssetId.data();
    if (strTargetEffectId.empty())
    {
        m_strElementStatus =
            "Enter an Effect Name before creating the Mesh Effect.";
        return false;
    }

    const bool_t bUsesActiveDocument = m_ActiveDocument.has_value() &&
        m_ActiveDocument->strEffectAssetId == strTargetEffectId &&
        (EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
            EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource);
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
                "The open Detail draft no longer matches its Element; Create Effect preserved all data.";
            return false;
        }
    }
    else
    {
        Staged.iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
        Staged.strEffectAssetId = strTargetEffectId;
    }
    const std::string strRequestedDisplayName = m_NewDisplayName.data();
    if (!strRequestedDisplayName.empty())
        Staged.strDisplayName = strRequestedDisplayName;
    else if (Staged.strDisplayName.empty())
        Staged.strDisplayName = strTargetEffectId;

    EFFECT_ELEMENT_DESC Element = m_MeshAuthoringDraft;
    Element.eKind = EFFECT_ELEMENT_KIND::MESH;
    Element.Material.strTemplateId =
        std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
    Element.Material.SourceMaterial = {};
    Element.Detail.Mesh.bUseModelMaterial = false;
    Element.SourceRecipe = {};
    Element.SourcePresentation = {};
    Element.ActionCueAttachment = {};
    Element.strElementId = m_NewElementId.data();
    if (Element.strElementId.empty())
    {
        const std::string Prefix = "mesh_layer_";
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
    if (!Is_EffectManualMeshAuthoringContractSatisfied(
        nullptr != pMesh, nullptr != pBase, bUnsafeBase))
    {
        m_strElementStatus =
            "Create Effect requires one WModel Mesh and one safe 2D Base texture.";
        return false;
    }

    std::set<std::string> Slots;
    for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
        Element.ResourceBindings)
    {
        if (!Slots.insert(Binding.strSlotId).second ||
            !Slot_Allowed(Element, Binding.strSlotId))
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
            "Create Effect rejected a non-drawable Mesh document: " +
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

    const EFFECT_PREVIEW_FILTER ePreviousPreviewFilter = m_ePreviewFilter;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    if (!Stage_WorldPreview(Staged))
    {
        m_ePreviewFilter = ePreviousPreviewFilter;
        m_strElementStatus =
            "Create Effect preview failed; the active Document, Data File, and builder were preserved: " +
            m_strPreviewStatus;
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
        m_ePreviewFilter = ePreviousPreviewFilter;
        if (m_ActiveDocument.has_value())
        {
            if (m_bActiveDocumentDrawable)
                Stage_WorldPreview(*m_ActiveDocument);
            else
                Release_WorldPreview(true);
        }
        else
            Release_WorldPreview(true);
        m_strElementStatus =
            "Create Effect save failed; previous Document and preview were restored: " +
            Error;
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
    const std::string strCreatedElementId = Element.strElementId;
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = strCreatedElementId;
    m_strSelectedElementGroupId = Element.strGroupId;
    m_strSelectedComponentId.clear();
    m_strSelectedEmitterId.clear();
    m_strSelectedSourceModuleId.clear();
    m_eSelectedEffectType = EFFECT_ELEMENT_KIND::MESH;
    m_strSelectedResourceSlotId = std::string(EFFECT_MESH_SHAPE_SLOT_ID);
    m_eResourceLibraryFileKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
    m_strSelectedResourceAssetId.clear();
    m_strSelectedDataFileAssetId = strTargetEffectId;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    m_NewElementId[0u] = '\0';
    Recalculate_PreviewDuration();
    Start_WorldPreviewFromBeginning();
    Refresh_RuntimeEquivalence();
    Refresh_DataFiles();
    Refresh_AllEffects();
    m_strElementStatus =
        "Created one Mesh Effect layer and saved it atomically to Data Files; builder selections were preserved.";
    m_strDocumentStatus = "Saved Authored atomically: " + Path.string() +
        " Live preview is active; Assembly/WFX/Runtime Catalog publish remains separate.";
    return true;
}

bool_t Client::CEffect_Tool::Try_BindMeshAuthoringResource(
    const std::string& strAssetId)
{
    if (!m_bMeshAuthoringDraftInitialized)
        Reset_MeshAuthoringDraft();
    if (!Slot_Allowed(m_MeshAuthoringDraft,
        m_strSelectedResourceSlotId))
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
            m_strSelectedResourceSlotId))
    {
        return false;
    }
    std::erase_if(m_MeshAuthoringDraft.ResourceBindings,
        [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.strSlotId == m_strSelectedResourceSlotId;
        });
    m_strSelectedResourceAssetId.clear();
    m_strResourceStatus = "Cleared the selected Mesh Authoring slot.";
    return true;
}

bool_t Client::CEffect_Tool::Try_AddElement()
{
    if (Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before adding an Element.";
        return false;
    }
    if (!m_ActiveDocument.has_value())
    {
        m_strElementStatus = "Create or load a Document first.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    EFFECT_ELEMENT_DESC Element;
    Element.strElementId = m_NewElementId.data();
    Element.strDisplayName = Element.strElementId;
    Element.eKind = m_eSelectedEffectType;
    Element.Material.eRenderProfile =
        EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
    if (Element.strElementId.empty())
    {
        Element.strElementId = "element_" +
            std::to_string(Staged.Elements.size() + 1u);
        Element.strDisplayName = Element.strElementId;
        Copy_Buffer(m_NewElementId.data(), m_NewElementId.size(),
            Element.strElementId);
    }
    Staged.Elements.push_back(Element);
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
    m_eDetailSelection = EFFECT_DETAIL_SELECTION::ELEMENT;
    m_strSelectedElementId = Element.strElementId;
    m_strSelectedElementGroupId = Element.strGroupId;
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strSelectedResourceSlotId = Default_SlotId(Element.eKind);
    m_eResourceLibraryFileKind = EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
        EFFECT_RESOURCE_FILE_KIND::MODEL :
        EFFECT_RESOURCE_FILE_KIND::TEXTURE;
    m_strElementStatus = "Added one typed visual layer.";
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
    if (!m_ActiveDocument.has_value() || m_strSelectedElementId.empty())
    {
        m_strElementStatus = "Select one Element to delete.";
        return false;
    }
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    const auto NewEnd = std::remove_if(
        Staged.Elements.begin(), Staged.Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    if (NewEnd == Staged.Elements.end())
        return false;
    Staged.Elements.erase(NewEnd, Staged.Elements.end());
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    Reset_DetailDraft();
    m_strSelectedElementId.clear();
    m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strElementStatus = "Deleted the selected Element.";
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
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
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
	if (bParticleDraft || bDetailDraft)
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if ((bParticleDraft && !Apply_ParticleSystemDraft(Staged)) ||
			(bDetailDraft && !Apply_DetailDraft(Staged)) ||
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
				m_DetailDraft = *pCommitted;
			m_bDetailDraftDirty = false;
		}
	}
	if (!m_bDocumentDirty)
	{
		m_strDocumentStatus = "The active Authored Document is already saved.";
		return true;
	}
	const bool_t bSaved = Try_SaveDocument();
	if (bSaved)
	{
		if (m_bActiveDocumentDrawable &&
			nullptr != m_pWorldPreviewObject.lock())
		{
			m_strDetailStatus =
				"Applied and saved Authored; live world preview is active. Runtime publish remains a separate step.";
		}
		else
		{
			m_strDetailStatus =
				"Applied and saved a structurally valid partial draft; world preview is hidden and publish is blocked: " +
				m_strActiveDocumentDrawableError;
		}
	}
	return bSaved;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
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
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource)
	{
		m_strDocumentStatus =
			"Runtime Assembly/WFX views are audition copies; use Save As for an Authored override.";
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
    Refresh_RuntimeEquivalence();
    m_strSelectedDataFileAssetId =
        m_ActiveDocument->strEffectAssetId;
    m_strDocumentStatus = "Saved Authored atomically: " + Path.string();
	if (!m_bActiveDocumentDrawable)
	{
		m_strDocumentStatus +=
			" Structurally valid partial draft saved; world preview hidden and publish blocked: " +
			m_strActiveDocumentDrawableError;
	}
	else
	{
		m_strDocumentStatus += m_bActiveDocumentMatchesRuntime ?
			" Loaded Runtime Catalog snapshot is equivalent." :
			" Live world preview updated; Assembly/WFX/Runtime Catalog publish pending.";
	}
    Refresh_DataFiles();
    Refresh_AllEffects();
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
    m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(
        bWasDrawable, std::move(PreviousDrawableError));
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
    Refresh_RuntimeEquivalence();
    m_strSelectedDataFileAssetId = strAssetId;
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(), strAssetId);
    Refresh_DataFiles();
    Refresh_AllEffects();
    m_strDocumentStatus = "Saved new Authored Effect atomically: " +
        Path.string() +
		" World preview updated; Assembly/WFX/Runtime Catalog publish pending.";
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
    m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(true, {});
    m_ActiveDocumentPath = Path;
    m_eActiveDocumentSource = EFFECT_DOCUMENT_SOURCE::AUTHORED;
	m_strActiveDocumentBaselineCanonical =
		CEffectDocumentCodec::Serialize(*m_ActiveDocument);
    m_bDocumentDirty = false;
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
		EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == m_eActiveDocumentSource;
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
    return Try_LoadDocumentPath(
        m_ActiveDocumentPath, m_eActiveDocumentSource,
		bRuntimeView ? m_strSelectedDataFileAssetId :
			m_ActiveDocument->strEffectAssetId);
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
    Release_WorldPreview(true);
    std::string CanonicalBaseline;
    if (EFFECT_DOCUMENT_SOURCE::AUTHORED == eSource)
    {
        Engine::CProfilerScope CanonicalProfile(
            CGameInstance::Get().Get_Profiler(),
            "EffectTool.DocumentLoad.CanonicalBaseline");
        CanonicalBaseline = CEffectDocumentCodec::Serialize(Staged);
    }
	m_ActiveDocument = std::move(Staged);
    Set_ActiveDocumentDrawableStatus(bDrawable, PreviewStatus);
    m_ActiveDocumentPath = Path;
	m_strActiveDocumentBaselineCanonical = CanonicalBaseline;
    m_eActiveDocumentSource = eSource;
    Reset_ParticleSystemDraft();
    Reset_DetailDraft();
	const bool_t bHasParticleSystem = std::any_of(
        m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
        [](const EFFECT_ELEMENT_DESC& Element)
        {
            return EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind;
        });
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY == eSource)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::SKILL;
	else if (EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT == eSource)
		m_eDetailSelection = EFFECT_DETAIL_SELECTION::COMPONENT;
	else
	{
		m_eDetailSelection = bHasParticleSystem ?
			EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM :
			(m_ActiveDocument->Elements.empty() ?
				EFFECT_DETAIL_SELECTION::NONE :
				EFFECT_DETAIL_SELECTION::ELEMENT);
	}
    m_strSelectedElementId =
        EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
            m_ActiveDocument->Elements.front().strElementId : std::string{};
	m_strSelectedElementGroupId =
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ?
			m_ActiveDocument->Elements.front().strGroupId : std::string{};
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
    Copy_Buffer(m_NewAssetId.data(), m_NewAssetId.size(),
        m_ActiveDocument->strEffectAssetId);
    Copy_Buffer(m_NewDisplayName.data(), m_NewDisplayName.size(),
        m_ActiveDocument->strDisplayName);
    m_bDocumentDirty = false;
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
    m_strDocumentStatus = bDrawable ?
        "Loaded existing Effect for inspection without GPU staging; choose Complete, Group, or Solo Play: " +
			(Path.empty() ? strSelectionId : Path.string()) :
        "Loaded editable draft; preview is hidden until required resources bind: " +
            PreviewStatus;
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
    return Try_LoadDocumentPathStaged(
        Pending.Path,
        Pending.eSource,
        Pending.strSelectionId,
        true);
}

bool_t Client::CEffect_Tool::Refresh_AllEffects(
    const bool_t bReloadSkillCatalog)
{
    m_bAllEffectsRefreshAttempted = true;
    std::string CatalogStatus;
    if ((bReloadSkillCatalog || CPlayerSkillCatalog::Get_Skills().empty()) &&
        !CPlayerSkillCatalog::Load(CatalogStatus))
    {
        m_strElementStatus =
            "All Effects refresh preserved the previous tree: " +
            CatalogStatus;
        return false;
    }

    vector<EFFECT_SKILL_TREE_ENTRY> Staged;
    size_t iMissingAuthored = 0u;
    for (const PLAYER_SKILL_DEFINITION& Skill :
        CPlayerSkillCatalog::Get_Skills())
    {
        if (Skill.strEffectId.empty())
            continue;
        const std::filesystem::path Path = CProjectDataRoot::Resolve(
            std::filesystem::path(L"Effects") / L"Authored" /
            (std::filesystem::path(Skill.strEffectId).wstring() +
                L".effect.json"));
        if (Path.empty() || !std::filesystem::is_regular_file(Path))
        {
            ++iMissingAuthored;
            continue;
        }
        Staged.push_back({ Skill, CEffectCatalog::Find(Skill.strEffectId) });
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
    m_strElementStatus = "All Effects indexed from PlayerSkills + Authored: " +
        std::to_string(m_AllEffects.size()) + " complete skills";
    if (0u != iMissingAuthored)
        m_strElementStatus += ", " + std::to_string(iMissingAuthored) +
            " mappings have no Authored document";
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
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectSkill(
    const std::string& strEffectAssetId)
{
	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == strEffectAssetId &&
		EFFECT_DETAIL_SELECTION::SKILL != m_eDetailSelection &&
		Has_UnappliedDetailDraft())
	{
		m_strElementStatus =
			"Apply or Revert the open Detail draft before selecting the Skill.";
		return false;
	}
    if (!m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId)
    {
		if (!Try_LoadDocument(strEffectAssetId))
			return false;
    }

	Reset_ParticleSystemDraft();
	Reset_DetailDraft();
	m_eDetailSelection = EFFECT_DETAIL_SELECTION::SKILL;
	m_strSelectedElementId.clear();
	m_strSelectedElementGroupId.clear();
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
	m_strSelectedResourceAssetId.clear();
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    Recalculate_PreviewDuration();
    Synchronize_LoadedSkillPreview();
    if (!Stage_WorldPreview())
    {
        m_strElementStatus =
            "Complete Effect preview could not be restarted: " +
            m_strPreviewStatus;
        return false;
    }
    Start_WorldPreviewFromBeginning();
	m_strElementStatus =
		"Selected the complete Skill Effect; choose Particle System, Component, "
		"Emitter, or Module for narrower controls.";
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectParticleSystem(
    const std::string& strEffectAssetId)
{
    const bool_t bChangesSelection =
        !m_ActiveDocument.has_value() ||
        m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
        EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM != m_eDetailSelection;
    if (bChangesSelection && Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before selecting the Particle System.";
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
    if (bChangesSelection && Has_UnappliedDetailDraft())
    {
        m_strElementStatus =
            "Apply or Revert the open Detail draft before selecting another Element.";
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
    if (EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS == m_ePreviewFilter ||
		EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS == m_ePreviewFilter)
        m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    if (EFFECT_PREVIEW_FILTER::COMPLETE != m_ePreviewFilter)
        Stage_WorldPreview();
    return true;
}

bool_t Client::CEffect_Tool::Try_SelectComponent(
	const std::string& strEffectAssetId,
	const std::string& strComponentAssetId)
{
	if (Has_UnappliedDetailDraft())
	{
		m_strElementStatus =
			"Apply or Revert the open Detail draft before selecting a Component.";
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
        pObject->Reset();
        pObject->Set_SampleTime(0.f);
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

    const EFFECT_PREVIEW_FILTER ePreviousFilter = m_ePreviewFilter;
    const f32_t fPreviousTime = m_fPreviewTimeSeconds;
    const f32_t fPreviousDuration = m_fPreviewDurationSeconds;
    const bool_t bPreviousPlaying = m_bPreviewPlaying;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::SOLO_SELECTED;
    m_fPreviewTimeSeconds =
        Selected->Detail.Timing.fStartDelaySeconds;
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
        pObject->Reset();
        pObject->Set_SampleTime(m_fPreviewTimeSeconds);
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
    if (m_bMeshAuthoringDraftInitialized)
    {
        m_MeshAuthoringDraft.ResourceBindings.clear();
        m_strSelectedResourceSlotId =
            std::string(EFFECT_MESH_SHAPE_SLOT_ID);
        m_eResourceLibraryFileKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
    }
    Copy_Buffer(m_ResourceCategory.data(),
        m_ResourceCategory.size(), "All");
    m_strSelectedResourceAssetId.clear();
    m_iResourceViewRevision = UINT64_MAX;
    m_strResourceStatus = "Authoring category selected: " + strDomainId + ".";
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

bool_t Client::CEffect_Tool::Try_BindResource(
    const std::string& strAssetId)
{
    if (Has_UnappliedDetailDraft())
    {
        m_strResourceStatus =
            "Apply or Revert the open Detail draft before changing resources.";
        return false;
    }
    if (!m_ActiveDocument.has_value() || nullptr == Find_SelectedElement())
    {
        m_strResourceStatus = "Select an Element before choosing a resource.";
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
    if (nullptr == pElement ||
        !Slot_Allowed(*pElement, m_strSelectedResourceSlotId))
    {
        m_strResourceStatus = "That resource slot is not allowed for this Element.";
        return false;
    }
    const EFFECT_RESOURCE_FILE_KIND eExpectedKind =
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
    auto Iterator = std::find_if(
        pElement->ResourceBindings.begin(), pElement->ResourceBindings.end(),
        [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.strSlotId == m_strSelectedResourceSlotId;
        });
    if (Iterator == pElement->ResourceBindings.end())
        pElement->ResourceBindings.push_back(
            { m_strSelectedResourceSlotId, strAssetId });
    else
        Iterator->strAssetId = strAssetId;
    const std::string strSlotLabel =
        Slot_Label(*pElement, m_strSelectedResourceSlotId);
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    m_strResourceStatus = "Bound " + strAssetId + " to " +
        strSlotLabel + ".";
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
    if (!m_ActiveDocument.has_value() || nullptr == Find_SelectedElement())
        return false;
    EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
    for (EFFECT_ELEMENT_DESC& Element : Staged.Elements)
    {
        if (Element.strElementId != m_strSelectedElementId)
            continue;
        std::erase_if(Element.ResourceBindings,
            [this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == m_strSelectedResourceSlotId;
            });
        break;
    }
    if (!Try_CommitDocument(std::move(Staged)))
        return false;
    m_strResourceStatus = "Cleared the selected resource slot.";
    return true;
}

bool_t Client::CEffect_Tool::Try_CommitDocument(
    EFFECT_DOCUMENT_DESC&& Staged)
{
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
        m_strSelectedElementId.empty())
    {
        m_strPreviewStatus =
            "Select one Element before choosing Element Solo/Mute.";
        return false;
    }
    if ((EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == eFilter ||
        EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP == eFilter) &&
        m_strSelectedElementGroupId.empty())
    {
        m_strPreviewStatus =
            "Select one grouped Element before choosing Group Solo/Mute.";
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

bool_t Client::CEffect_Tool::Stage_WorldPreview()
{
    return m_ActiveDocument.has_value() ?
        Stage_WorldPreview(*m_ActiveDocument) : false;
}

bool_t Client::CEffect_Tool::Stage_WorldPreview(
    const EFFECT_DOCUMENT_DESC& Document)
{
    const EFFECT_DOCUMENT_DESC PreviewDocument =
        Build_PreviewDocument(Document);
    if (!Ensure_WorldPreviewObject())
        return false;
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    std::string Error;
    if (nullptr == pObject || !pObject->Stage_Document(
        PreviewDocument, Error))
    {
        m_strPreviewStatus = "Document is editable but not drawable yet: " + Error;
        return false;
    }
    pObject->Set_Playing(false);
    pObject->Set_SampleTime(m_fPreviewTimeSeconds);
    switch (m_ePreviewFilter)
    {
    case EFFECT_PREVIEW_FILTER::COMPLETE:
        m_strPreviewStatus =
            "Complete Effect preview committed from the active Document.";
        break;
    case EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM:
        m_strPreviewStatus =
            "Particle System-only preview committed.";
        break;
	case EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS:
		m_strPreviewStatus =
			"Mesh-backed Cascade emitter preview committed.";
		break;
	case EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS:
		m_strPreviewStatus =
			"Sprite Cascade emitter preview committed.";
		break;
    case EFFECT_PREVIEW_FILTER::SOLO_SELECTED:
        m_strPreviewStatus = "Selected Element Solo preview committed.";
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
        if (m_strSelectedElementGroupId.empty())
            return Preview;
        const bool_t bGroupExists = std::any_of(
            Preview.Elements.begin(), Preview.Elements.end(),
            [this](const EFFECT_ELEMENT_DESC& Element)
            {
                return Element.strGroupId == m_strSelectedElementGroupId;
            });
        if (!bGroupExists)
            return Preview;
        std::erase_if(Preview.Elements,
            [this](const EFFECT_ELEMENT_DESC& Element)
            {
                const bool_t bSelectedGroup =
                    Element.strGroupId == m_strSelectedElementGroupId;
                return EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP ==
                    m_ePreviewFilter ? !bSelectedGroup : bSelectedGroup;
            });
        if (EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP == m_ePreviewFilter)
            Preview.ModelCues.clear();
        return Preview;
    }
    if (EFFECT_PREVIEW_FILTER::COMPLETE == m_ePreviewFilter ||
        m_strSelectedElementId.empty())
        return Preview;
    const bool_t bSelectionExists = std::any_of(
        Preview.Elements.begin(), Preview.Elements.end(),
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            return Element.strElementId == m_strSelectedElementId;
        });
    if (!bSelectionExists)
        return Preview;
    std::erase_if(Preview.Elements,
        [this](const EFFECT_ELEMENT_DESC& Element)
        {
            const bool_t bSelected =
                Element.strElementId == m_strSelectedElementId;
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
        return true;
    }
    return false;
}

bool_t Client::CEffect_Tool::Resolve_PreviewRoot(float4x4_t& OutRoot)
{
    switch (m_ePreviewPivotKind)
    {
    case EFFECT_PREVIEW_PIVOT_KIND::WORLD:
        OutRoot = m_PreviewWorldRoot;
        return true;
    case EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT:
        return CAnimationTargetService::Resolve_RootTransform(&OutRoot);
    case EFFECT_PREVIEW_PIVOT_KIND::WEAPON_SOCKET:
    case EFFECT_PREVIEW_PIVOT_KIND::MODEL_BONE:
        return CAnimationTargetService::Resolve_AnchorTransform(
            m_strPreviewAnchorSlotId.c_str(), &OutRoot);
    case EFFECT_PREVIEW_PIVOT_KIND::END:
    default:
        return false;
    }
}

void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()
{
    m_fPreviewTimeSeconds = 0.f;
    Restart_SynchronizedAnimationSequence();
    shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (nullptr == pObject && m_ActiveDocument.has_value() &&
        Stage_WorldPreview())
    {
        pObject = m_pWorldPreviewObject.lock();
    }
    if (nullptr == pObject)
    {
        m_bPreviewPlaying = false;
        return;
    }
    float4x4_t TargetRoot{};
    if (CAnimationTargetService::Resolve_RootTransform(&TargetRoot))
    {
        m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
        pObject->Set_RootWorld(TargetRoot);
    }
    pObject->Set_Visible(true);
    pObject->Reset();
    pObject->Set_SampleTime(0.f);
	m_bPreviewVisibleRequested = true;
    m_bPreviewPlaying = true;
}

void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()
{
    Reset_SynchronizedAnimationSequence();
    m_strPreviewAnimationStatus.clear();
    if (!m_ActiveDocument.has_value())
        return;

    std::string CatalogStatus;
    const bool_t bCatalogAvailable =
        Ensure_PlayerSkillCatalog(CatalogStatus);
    const vector<PLAYER_SKILL_DEFINITION>& Skills =
        CPlayerSkillCatalog::Get_Skills();
    auto Skill = std::find_if(
        Skills.begin(), Skills.end(),
        [this](const PLAYER_SKILL_DEFINITION& Candidate)
        {
            return Candidate.strEffectId ==
                m_ActiveDocument->strEffectAssetId;
        });
    if (Skill == Skills.end() &&
        std::string::npos != m_ActiveDocument->strEffectAssetId.find(
            "restoration-candidate"))
    {
        Skill = std::find_if(
            Skills.begin(), Skills.end(),
            [this](const PLAYER_SKILL_DEFINITION& Candidate)
            {
                return !Candidate.strEffectId.empty() &&
                    m_ActiveDocument->strEffectAssetId.starts_with(
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
    if (!CAnimationSkillBindingDocument::Load(
        pAnimationAsset,
        Skill->eCharacterClass,
        Skills,
        Collect_AnimationClipNames(pModel),
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
    /* The preview plays the authored clips end to end: it is showing the effect,
    not waiting on the Server stage the combo would need. */
    m_SynchronizedAnimationClips.clear();
    for (const ANIMATION_SKILL_STAGE& Stage : Binding->Stages)
    {
        m_SynchronizedAnimationClips.insert(
            m_SynchronizedAnimationClips.end(),
            Stage.Clips.begin(), Stage.Clips.end());
    }
    if (m_SynchronizedAnimationClips.empty())
    {
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }
    m_iSynchronizedAnimationClipIndex = 0u;
    m_iSynchronizedAnimationTargetGeneration =
        CAnimationTargetService::Resolve_TargetGeneration();
    const bool_t bSingleClip = 1u == m_SynchronizedAnimationClips.size();
    const ANIMATION_SKILL_CLIP& FirstClip =
        m_SynchronizedAnimationClips.front();
    if (!pModel->Start_Animation(
        FirstClip.strClipName.c_str(), bSingleClip && m_bPreviewLoop))
    {
        Reset_SynchronizedAnimationSequence();
        m_strPreviewAnimationStatus =
            "Effect is playing; its first bound animation clip is unavailable.";
        return;
    }
    pModel->Set_AnimationSpeed(FirstClip.fPlayRate);
    pModel->Set_AnimPaused(false);
    m_strPreviewAnimationStatus = "Skill animation synced: " +
        Skill->strInputSlot + " | " + Skill->strDisplayName + " -> " +
        FirstClip.strClipName;
    if (!bSingleClip)
    {
        m_strPreviewAnimationStatus += " (sequence 1/" +
            std::to_string(m_SynchronizedAnimationClips.size()) + ")";
    }
}

void Client::CEffect_Tool::Restart_SynchronizedAnimationSequence()
{
    if (m_SynchronizedAnimationClips.empty() ||
        m_iSynchronizedAnimationTargetGeneration !=
            CAnimationTargetService::Resolve_TargetGeneration())
    {
        return;
    }
    const shared_ptr<Engine::CModel> pModel =
        CAnimationTargetService::Resolve_Model();
    if (nullptr == pModel)
        return;
    m_iSynchronizedAnimationClipIndex = 0u;
    const ANIMATION_SKILL_CLIP& FirstClip =
        m_SynchronizedAnimationClips.front();
    const bool_t bSingleClip = 1u == m_SynchronizedAnimationClips.size();
    if (!pModel->Start_Animation(
        FirstClip.strClipName.c_str(), bSingleClip && m_bPreviewLoop))
    {
        m_strPreviewAnimationStatus =
            "Skill animation restart failed: " + FirstClip.strClipName;
        return;
    }
    pModel->Set_AnimationSpeed(FirstClip.fPlayRate);
    pModel->Set_AnimPaused(false);
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
        const ANIMATION_SKILL_CLIP& Clip =
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

        f32_t fSourceDurationSeconds = fDuration / fTicksPerSecond;
        if (0u != Clip.iPlayMs)
        {
            fSourceDurationSeconds = (std::min)(
                fSourceDurationSeconds,
                static_cast<f32_t>(Clip.iPlayMs) * 0.001f);
        }
        const bool_t bLastClip =
            iClip + 1u == m_SynchronizedAnimationClips.size();
        if (!bLastClip && fRemainingSeconds > fSourceDurationSeconds)
        {
            fRemainingSeconds -= fSourceDurationSeconds;
            continue;
        }

        const bool_t bLoop =
            1u == m_SynchronizedAnimationClips.size() && m_bPreviewLoop;
        if (!pModel->Start_Animation(iAnimation, bLoop))
        {
            m_strPreviewAnimationStatus =
                "Skill animation seek failed: " + Clip.strClipName;
            return;
        }
        pModel->Set_AnimationSpeed(Clip.fPlayRate);
        const f32_t fTrackPosition = (std::min)(
            fRemainingSeconds, fSourceDurationSeconds) * fTicksPerSecond;
        pModel->Set_AnimTrackPosition(iAnimation, fTrackPosition);
        pModel->Play_Animation(0.f);
        pModel->Set_AnimPaused(!m_bPreviewPlaying);
        m_iSynchronizedAnimationClipIndex = iClip;
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
        const ANIMATION_SKILL_CLIP& Clip =
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

        f32_t fSourceDurationSeconds = fDuration / fTicksPerSecond;
        if (0u != Clip.iPlayMs)
        {
            fSourceDurationSeconds = (std::min)(
                fSourceDurationSeconds,
                static_cast<f32_t>(Clip.iPlayMs) * 0.001f);
        }
        if (iClip < m_iSynchronizedAnimationClipIndex)
        {
            fOutTimeSeconds += fSourceDurationSeconds;
            continue;
        }

        const char_t* pCurrentName = pModel->Get_AnimationName(
            pModel->Get_CurrentAnimIndex());
        if (nullptr == pCurrentName || Clip.strClipName != pCurrentName)
            return false;
        fOutTimeSeconds += (std::min)(
            fPosition / fTicksPerSecond, fSourceDurationSeconds);
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
    if (m_SynchronizedAnimationClips.size() <= 1u)
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
    const ANIMATION_SKILL_CLIP& CurrentClip =
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
    f32_t fLimit = fDuration;
    const f32_t fTicksPerSecond =
        pModel->Get_AnimationTickPerSecond(iAnimation);
    if (0u != CurrentClip.iPlayMs && std::isfinite(fTicksPerSecond) &&
        fTicksPerSecond > 0.f)
    {
        fLimit = (std::min)(
            fDuration,
            static_cast<f32_t>(CurrentClip.iPlayMs) * 0.001f *
                fTicksPerSecond);
    }
    if (fPosition + 0.0001f < fLimit)
    {
        return;
    }

    m_iSynchronizedAnimationClipIndex =
        (m_iSynchronizedAnimationClipIndex + 1u) %
        m_SynchronizedAnimationClips.size();
    const ANIMATION_SKILL_CLIP& NextClip =
        m_SynchronizedAnimationClips[m_iSynchronizedAnimationClipIndex];
    if (!pModel->Start_Animation(NextClip.strClipName.c_str(), false))
    {
        m_strPreviewAnimationStatus =
            "Skill animation sequence stopped; next clip is unavailable: " +
            NextClip.strClipName;
        Reset_SynchronizedAnimationSequence();
        return;
    }
    pModel->Set_AnimationSpeed(NextClip.fPlayRate);
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
    m_iSynchronizedAnimationTargetGeneration = 0u;
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
    const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
    if (bRemoveFromLayer && nullptr != pObject &&
        m_iWorldPreviewLevel == CGameInstance::Get().Get_CurrentLevelID())
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            m_iWorldPreviewLevel, PREVIEW_LAYER, pObject);
    }
    m_pWorldPreviewObject.reset();
    m_iWorldPreviewLevel = UINT32_MAX;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
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
	m_strSelectedComponentId.clear();
	m_strSelectedEmitterId.clear();
	m_strSelectedSourceModuleId.clear();
    m_strSelectedResourceAssetId.clear();
    m_bDocumentDirty = false;
    m_bActiveDocumentMatchesRuntime = false;
    m_bPreviewPlaying = false;
    m_fPreviewTimeSeconds = 0.f;
    Reset_SynchronizedAnimationSequence();
    Release_WorldPreview(true);
    m_strDocumentStatus =
        "Unloaded the in-memory Effect Document and hid its preview; the saved Data File was preserved.";
}

void Client::CEffect_Tool::Reset_MeshAuthoringDraft()
{
    m_MeshAuthoringDraft = {};
    m_MeshAuthoringDraft.eKind = EFFECT_ELEMENT_KIND::MESH;
    m_MeshAuthoringDraft.Material.strTemplateId =
        std::string(EFFECT_STANDARD_MATERIAL_TEMPLATE_ID);
    m_MeshAuthoringDraft.Material.eRenderProfile =
        EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
    m_MeshAuthoringDraft.Detail.Mesh.bUseModelMaterial = false;
    m_MeshAuthoringDraft.Detail.Transform.vScale = {
        EFFECT_MANUAL_MESH_DEFAULT_SCALE,
        EFFECT_MANUAL_MESH_DEFAULT_SCALE,
        EFFECT_MANUAL_MESH_DEFAULT_SCALE };
    m_MeshAuthoringDraft.SourceRecipe.bEnabled = false;
    m_bMeshAuthoringDraftInitialized = true;
    m_NewElementId[0u] = '\0';
    m_strSelectedResourceSlotId =
        std::string(EFFECT_MESH_SHAPE_SLOT_ID);
    m_strSelectedResourceAssetId.clear();
    m_eResourceLibraryFileKind = EFFECT_RESOURCE_FILE_KIND::MODEL;
    m_iResourceViewRevision = UINT64_MAX;
}

void Client::CEffect_Tool::Reset_ParticleSystemDraft()
{
    m_ParticleSystemDraft.reset();
    m_bParticleSystemDraftDirty = false;
    m_strDetailStatus.clear();
}

void Client::CEffect_Tool::Reset_DetailDraft()
{
    m_DetailDraft.reset();
    m_strDetailDraftElementId.clear();
    m_bDetailDraftDirty = false;
    m_strDetailStatus.clear();
}

void Client::CEffect_Tool::Recalculate_PreviewDuration()
{
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
    m_fPreviewDurationSeconds = 1.f;
    for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
    {
        if (!Element.bVisible)
            continue;
        const EFFECT_TIMING_DESC& Timing = Element.Detail.Timing;
        f32_t fElementTail = 0.f;
        if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
            fElementTail = Element.Detail.Particle.vLifeTimeSeconds.y;
        else if (EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
            fElementTail = Element.Detail.Trail.fPointLifeTimeSeconds;
        m_fPreviewDurationSeconds = (std::max)(m_fPreviewDurationSeconds,
            Timing.fStartDelaySeconds + Timing.fLifeTimeSeconds +
            Timing.fAfterImageSeconds + fElementTail);
    }
    for (const EFFECT_MODEL_CUE_DESC& Cue : Document.ModelCues)
    {
        if (Cue.bVisible)
        {
            m_fPreviewDurationSeconds = (std::max)(
                m_fPreviewDurationSeconds,
                Cue.fStartDelaySeconds + Cue.fDurationSeconds);
        }
    }
    m_fPreviewTimeSeconds = std::clamp(
        m_fPreviewTimeSeconds, 0.f, m_fPreviewDurationSeconds);
}

bool_t Client::CEffect_Tool::Has_UnsavedWork() const
{
    return m_bDocumentDirty || Has_UnappliedDetailDraft();
}

bool_t Client::CEffect_Tool::Has_UnappliedDetailDraft() const
{
    return m_bParticleSystemDraftDirty || m_bDetailDraftDirty;
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
