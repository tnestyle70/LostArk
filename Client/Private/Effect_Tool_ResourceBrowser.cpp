#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "EffectAuthoringResourceTree.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "EffectAuthoringTransfer.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentRenderer.h"
#include "EffectResourceCatalog.h"
#include "Effect_Object.h"
#include "Effect_RuntimeAuthority.h"
#include "Effect_ThumbnailCache.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "MapEffectPresentationRuntime.h"
#include "Model.h"
#include "Profiler.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
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
#include "Transform.h"
#include "CharacterPreviewPanel.h"
#include "EffectAuthoringSequencer.h"

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
    if (Render_WorldObjectResourceGrid(bMeshAuthoringDraft)) return;
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
    if (!m_bResourceCatalogRefreshAttempted)
        Refresh_ResourceCatalog();
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
    ImGui::SameLine();
    if (ImGui::Button("Reload Selected Category"))
    {
        Refresh_ResourceCatalogDomain(
            m_strSelectedAuthoringDomainId, true);
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
        ImGui::TextWrapped("%s", m_strResourceStatus.empty() ?
            "The selected Resources/Effect category is unavailable." :
            m_strResourceStatus.c_str());
        if (ImGui::Button("Retry Resource Library"))
            Refresh_ResourceCatalog();
        return;
    }
    if (m_ResourceCatalogByDomain.end() ==
        m_ResourceCatalogByDomain.find(m_strSelectedAuthoringDomainId))
    {
        ImGui::TextWrapped("%s", m_strResourceStatus.empty() ?
            "The selected Resources/Effect category has not loaded." :
            m_strResourceStatus.c_str());
        if (ImGui::Button("Retry Selected Category"))
        {
            Refresh_ResourceCatalogDomain(
                m_strSelectedAuthoringDomainId, true);
        }
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
    if (m_pAuthoringSequencer)
    {
        m_pCharacterPreviewPanel->Render_Selector(false, {}, true);
        m_pAuthoringSequencer->Render_ModelView();
        if (ImGui::CollapsingHeader("Native Clip Preview"))
        {
            ImGui::BeginDisabled(m_pAuthoringSequencer->Owns_ModelClock());
            Render_AnimationControls(CAnimationTargetService::Resolve_Model());
            ImGui::EndDisabled();
        }
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

	const EFFECT_ELEMENT_DESC* pSelectedFollowElement = Find_SelectedElement();
	if (nullptr != pSelectedFollowElement &&
		Can_EditElementFollowAttachment(*pSelectedFollowElement))
	{
		ImGui::SeparatorText("Selected Element Follow");
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			pSelectedFollowElement->ActionCueAttachment;
		const bool_t bHasElementFollow =
			Attachment.bEnabled && Attachment.bFollow;
		if (bHasElementFollow)
		{
			ImGui::TextWrapped("Element-local follow: %s -> %s",
				Attachment.strRuntimeAnchorSlotId.c_str(),
				Attachment.strRuntimeBoneName.c_str());
		}
		else
		{
			ImGui::TextWrapped(
				"No element-local follow. Select a Socket / Bone above and attach this Element.");
		}
		ImGui::TextDisabled(
			"This edits only the selected Element. Detail > Element Follow Attachment changes orientation through Apply.");
		const bool_t bAttachmentEditable =
			EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
			EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource;
		const bool_t bCanSetElementFollow =
			bAttachmentEditable &&
			!Has_UnappliedDetailDraft() &&
			!m_strPreviewAnchorSlotId.empty();
		ImGui::BeginDisabled(!bCanSetElementFollow);
		if (ImGui::Button("Attach Selected Element to Bone"))
			Try_SetSelectedElementFollowAnchor(m_strPreviewAnchorSlotId);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!bAttachmentEditable ||
			Has_UnappliedDetailDraft() ||
			!bHasElementFollow);
		if (ImGui::Button("Clear Selected Element Follow"))
			Try_ClearSelectedElementFollowAnchor();
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
    if (m_pAuthoringSequencer && m_pAuthoringSequencer->Is_Active() && m_pAuthoringSequencer->Is_ElementPreview())
    {
        m_fPreviewTimeSeconds = static_cast<float>(m_pAuthoringSequencer->ClockMs()) * .001f;
        ImGui::Text("Element preview: %.3f s", m_fPreviewTimeSeconds);
        ImGui::TextWrapped("Use Play, Pause and Time in the Effect Sequencer to inspect this element. Stop returns to the saved sequence.");
    }
    else
    {
    if (ImGui::Button(m_bPreviewPlaying ? "Pause" : "Play"))
    {
        if (m_bPreviewPlaying)
		{
			m_bPreviewPlaying = false;
			Set_SynchronizedAnimationPaused(true);
		}
		else if (m_ValtanCombatObjectIndependentPreview.has_value())
		{
			if (m_fPreviewTimeSeconds >= m_fPreviewDurationSeconds)
				Start_WorldPreviewFromBeginning();
			else
			{
				m_bPreviewVisibleRequested = true;
				m_bPreviewPlaying = true;
			}
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
	ImGui::Text("World Preview: %s | Timeline %.3f / %.3f s",
		m_bPreviewPlaying ? "PLAYING" : "PAUSED",
		m_fPreviewTimeSeconds, m_fPreviewDurationSeconds);
	const f32_t fEffectTimelineStart = Resolve_EffectTimelineTime(0.f);
	ImGui::Text("Effect local: %.3f s | Effect 0 = Timeline %.3f s",
		Resolve_EffectSampleTime(m_fPreviewTimeSeconds),
		fEffectTimelineStart);
	if (m_fPreviewTimeSeconds + 0.0001f < fEffectTimelineStart)
		ImGui::TextDisabled("Waiting for the Effect cue start.");
	if (m_bReconstructedSourceRuntimeNaturalTailActive)
	{
		ImGui::TextDisabled(
			"Natural Stop tail: +%.3f s after animation end.",
			m_fReconstructedSourceRuntimeTailSeconds);
	}
    if (!m_SynchronizedAnimationClips.empty())
    {
        ImGui::TextDisabled(
            "Sample Time is the animation timeline; Start Delay uses Effect local time. Play rate is applied automatically.");
    }
	if (ImGui::SliderFloat("Sample Time", &m_fPreviewTimeSeconds,
		0.f, m_fPreviewDurationSeconds, "%.3f s"))
	{
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = true;
		Reset_ProductCueSnapshot();
		if (m_ValtanCombatObjectIndependentPreview.has_value())
		{
			if (!Sync_ValtanCombatObjectIndependentPreview(true))
			{
				const std::string Failure = m_strPreviewStatus;
				Release_WorldPreview(true);
				m_bPreviewVisibleRequested = false;
				m_strPreviewStatus = Failure;
			}
		}
		else if (m_bReconstructedSourceRuntimeActive)
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
					std::string TransformError;
					const bool_t bSampled =
						Seek_ValtanBossPatternTransformHistory(
							pObject,
							Resolve_EffectSampleTime(m_fPreviewTimeSeconds),
							TransformError);
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
						m_WorldPreviewDocument.has_value() ? *m_WorldPreviewDocument :
						(m_ProductPreview.has_value() &&
						m_SourcePreviewDocument.has_value() ?
							*m_SourcePreviewDocument : *m_ActiveDocument);
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
					else if (Has_RequiredSourceFollowAttachments(SourceAnchorDocument) &&
						!m_SynchronizedAnimationClips.empty())
					{
						m_strPreviewStatus = "Hand history sample failed; previous preview preserved: " + HistoryError;
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

void Client::CEffect_Tool::Render_UnifiedEffectTree(
	const UNIFIED_EFFECT_CACHE& Cache,
	const std::string& strFallbackDisplayName,
	const VALTAN_CLIP_OCCURRENCE_VIEW* pValtanClip,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pValtanCue)
{
	Engine::CProfilerScope Profile(
		CGameInstance::Get().Get_Profiler(), "EffectTool.UnifiedEffectTree");
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
		Observe_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	const std::string RootLabel = strFallbackDisplayName + "##mapped-effect";
	ImGui::PushID(Cache.Document.strEffectAssetId.c_str());
	const bool_t bOpen = ImGui::TreeNodeEx(RootLabel.c_str(),
		ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow |
		(bActive ? ImGuiTreeNodeFlags_Selected : 0));
	if (!bOpen)
	{
		ImGui::PopID();
		return;
	}

	ImGui::BeginDisabled(nullptr == pEditablePath || !bDrawable ||
		!bPreviewReady);
	if (ImGui::SmallButton("Play All") && nullptr != pEditablePath)
	{
		std::string strExactStatus;
		const std::filesystem::path* pExactPath =
			Resolve_DirectAuthoredEditablePath(
				Cache.Document.strEffectAssetId, strExactStatus);
		if (nullptr == pExactPath)
		{
			m_strPreviewStatus = std::move(strExactStatus);
		}
		else
		{
			bool_t bTargetReady = true;
			if (bValtanProductRow)
			{
				bTargetReady = bActive ?
					Play_ValtanProductCue(*pValtanClip, *pValtanCue) :
					Try_OpenValtanAuthoredEffect(*pExactPath,
						Cache.Document.strEffectAssetId,
						*pValtanClip, *pValtanCue, true);
			}
			if (bTargetReady)
				Try_PlayUnifiedEffect(Cache);
		}
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

	// Keep the view local to this draw: document edits never leave cached pointers.
	std::array<std::vector<const EFFECT_ELEMENT_DESC*>,
		static_cast<size_t>(EFFECT_AUTHORING_FAMILY::END)> ElementFamilies;
	std::array<size_t, static_cast<size_t>(EFFECT_AUTHORING_FAMILY::END)>
		PlayLockedCounts{};
	{
		Engine::CProfilerScope FamilyProfile(
			CGameInstance::Get().Get_Profiler(), "EffectTool.UnifiedFamilyRows");
		for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			const size_t iFamily = static_cast<size_t>(Resolve_AuthoringFamily(Element));
			if (iFamily >= ElementFamilies.size())
				continue;
			ElementFamilies[iFamily].push_back(&Element);
			if (!Is_ElementPreviewAdmitted(Element))
				++PlayLockedCounts[iFamily];
		}
	}
	for (int32_t iFamily = 0;
		iFamily < static_cast<int32_t>(EFFECT_AUTHORING_FAMILY::END);
		++iFamily)
	{
		const EFFECT_AUTHORING_FAMILY eFamily =
			static_cast<EFFECT_AUTHORING_FAMILY>(iFamily);
		const auto& FamilyElements = ElementFamilies[static_cast<size_t>(iFamily)];
		const size_t iCount = FamilyElements.size();
		if (0u == iCount)
			continue;
		const size_t iPlayLockedCount = PlayLockedCounts[static_cast<size_t>(iFamily)];
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
			ImGuiListClipper Clipper;
			Clipper.Begin(static_cast<int>(FamilyElements.size()), ImGui::GetTextLineHeightWithSpacing());
			while (Clipper.Step())
			{
				for (int iRow = Clipper.DisplayStart; iRow < Clipper.DisplayEnd; ++iRow)
				{
					const EFFECT_ELEMENT_DESC& Element = *FamilyElements[static_cast<size_t>(iRow)];
					const size_t iOrdinal = static_cast<size_t>(iRow) + 1u;
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
	/* Marks are keyed by stable Element ID, so drop any that a document
	   reload, rollback or delete removed instead of carrying a stale count. */
	if (m_bMarkedElementIdsNeedPrune)
	{
		Prune_MissingElementMarks(*m_ActiveDocument, m_MarkedElementIds);
		m_bMarkedElementIdsNeedPrune = false;
	}
	ImGui::BeginDisabled(!m_bActiveDocumentDrawable);
	if (ImGui::SmallButton("Play All##active-authored"))
		(void)Try_PlayActiveUnifiedEffect();
	ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_bActiveDocumentDrawable || !m_pAuthoringSequencer || m_MarkedElementIds.empty());
    if (ImGui::SmallButton("Play Group")) (void)Try_PlayMarkedElementGroup();
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Shift-click Element rows, then play the marked selection together in a loop.");
	ImGui::SameLine();
	ImGui::TextDisabled("%zu Elements", m_ActiveDocument->Elements.size());
	ImGui::SameLine();
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
		(m_MarkedElementIds.empty() ?
			(EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
				nullptr != pSelectedForDuplicate &&
				AuthoringFamily_CanCreate(Resolve_AuthoringFamily(*pSelectedForDuplicate))) :
			std::all_of(m_ActiveDocument->Elements.begin(),
				m_ActiveDocument->Elements.end(),
				[this](const EFFECT_ELEMENT_DESC& Element)
				{
					return !m_MarkedElementIds.contains(Element.strElementId) ||
						AuthoringFamily_CanCreate(Resolve_AuthoringFamily(Element));
				}));
	const std::string DuplicateLabel = m_MarkedElementIds.empty() ?
		std::string("Duplicate Selected") :
		"Duplicate " + std::to_string(m_MarkedElementIds.size()) + " Marked";
	ImGui::BeginDisabled(!bCanDuplicateSelected);
	if (ImGui::SmallButton(DuplicateLabel.c_str()))
		Try_DuplicateSelectedElement();
	ImGui::EndDisabled();
	const auto SelectedForOrder = std::find_if(
		m_ActiveDocument->Elements.begin(), m_ActiveDocument->Elements.end(),
		[this](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == m_strSelectedElementId;
		});
	bool_t bCanMoveSelectedUp = false;
	bool_t bCanMoveSelectedDown = false;
	if (m_MarkedElementIds.empty() && !Has_UnappliedDetailDraft() &&
		SelectedForOrder != m_ActiveDocument->Elements.end())
	{
		const EFFECT_AUTHORING_FAMILY eSelectedFamily =
			Resolve_AuthoringFamily(*SelectedForOrder);
		bCanMoveSelectedUp = std::any_of(
			m_ActiveDocument->Elements.begin(), SelectedForOrder,
			[eSelectedFamily](const EFFECT_ELEMENT_DESC& Element)
			{
				return Resolve_AuthoringFamily(Element) == eSelectedFamily;
			});
		bCanMoveSelectedDown = std::any_of(
			std::next(SelectedForOrder), m_ActiveDocument->Elements.end(),
			[eSelectedFamily](const EFFECT_ELEMENT_DESC& Element)
			{
				return Resolve_AuthoringFamily(Element) == eSelectedFamily;
			});
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanMoveSelectedUp);
	if (ImGui::SmallButton("Up"))
		Try_MoveSelectedElement(-1);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!bCanMoveSelectedDown);
	if (ImGui::SmallButton("Down"))
		Try_MoveSelectedElement(1);
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Move the selected stable Element within its displayed Family order. Apply or Revert Detail first; marked multi-selection is not reordered implicitly.");
	}
	if (!m_MarkedElementIds.empty())
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Clear Marks"))
			m_MarkedElementIds.clear();
	}
    Render_AuthoringCommands();
	ImGui::TextDisabled(
		"Ctrl or Shift click Element rows to mark several. Play Group loops the marked selection immediately; Delete and Duplicate use the same marks.");
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
	// Build the visible family rows once after all document-edit commands above.
	// The view is frame-local, so reload/delete/reorder cannot leave a stale cache.
	std::array<std::vector<const EFFECT_ELEMENT_DESC*>,
		static_cast<size_t>(EFFECT_AUTHORING_FAMILY::END)> ElementFamilies;
	std::array<size_t, static_cast<size_t>(EFFECT_AUTHORING_FAMILY::END)>
		PlayLockedCounts{};
	for (const EFFECT_ELEMENT_DESC& Element : m_ActiveDocument->Elements)
	{
		const size_t iFamily = static_cast<size_t>(Resolve_AuthoringFamily(Element));
		if (iFamily >= ElementFamilies.size())
			continue;
		ElementFamilies[iFamily].push_back(&Element);
		if (!Is_ElementPreviewAdmitted(Element))
			++PlayLockedCounts[iFamily];
	}
	for (int32_t iFamily = 0;
		iFamily < static_cast<int32_t>(EFFECT_AUTHORING_FAMILY::END);
		++iFamily)
	{
		const EFFECT_AUTHORING_FAMILY eFamily =
			static_cast<EFFECT_AUTHORING_FAMILY>(iFamily);
		const auto& FamilyElements = ElementFamilies[static_cast<size_t>(iFamily)];
		const size_t iCount = FamilyElements.size();
		if (0u == iCount)
			continue;
		const size_t iPlayLockedCount = PlayLockedCounts[static_cast<size_t>(iFamily)];
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
			ImGuiListClipper Clipper;
			Clipper.Begin(static_cast<int>(FamilyElements.size()), ImGui::GetTextLineHeightWithSpacing());
			while (Clipper.Step())
			{
				for (int iRow = Clipper.DisplayStart; iRow < Clipper.DisplayEnd; ++iRow)
				{
					const EFFECT_ELEMENT_DESC& Element = *FamilyElements[static_cast<size_t>(iRow)];
					const size_t iOrdinal = static_cast<size_t>(iRow) + 1u;
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
							/* Marking keeps the current Detail draft. The first modifier
                               click includes an already selected row in the group. */
                            if (m_MarkedElementIds.empty() &&
                                EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
                                !m_strSelectedElementId.empty() && m_strSelectedElementId != Element.strElementId)
                                m_MarkedElementIds.insert(m_strSelectedElementId);
							if (!m_MarkedElementIds.insert(
									Element.strElementId).second)
							{
								m_MarkedElementIds.erase(Element.strElementId);
							}
						}
						else
						{
                            if (Try_SelectElement(m_ActiveDocument->strEffectAssetId, Element.strElementId))
                                m_MarkedElementIds.clear();
						}
					}
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Stable Element: %s\n%s",
							Element.strElementId.c_str(),
							ElementPreviewAdmissionReason(Element));
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
						ImGui::SetTooltip("%s", ElementPreviewAdmissionReason(Element));
					}
					ImGui::PopID();
				}
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

void Client::CEffect_Tool::Render_SavedAuthoredEffectSection(
	const std::string& strSearch, const bool_t bWorld)
{
	const char_t* pOwnerLabel = bWorld ? "World" : "KoukuSaydon";
	const auto MatchesOwner = [bWorld](const std::string_view id)
	{ return bWorld ? Is_WorldEffectAssetId(id) : Is_KoukuEffectAssetId(id); };
    if (!m_bSavedEffectOrganizationLoaded)
    {
        std::vector<CEffectAuthoringResourceTree::RESOURCE> rows;
        std::string status;
        if (CEffectAuthoringResourceTree::Read_V1Organization(rows, status))
        {
            decltype(m_SavedEffectOrganization) staged;
            for (auto& row : rows) staged.emplace(row.strAssetId, std::make_pair(std::move(row.strDisplayName), std::move(row.CategoryPath)));
            m_SavedEffectOrganization = std::move(staged);
        }
        else m_strElementStatus = "Effect categories kept their previous state: " + status;
        m_bSavedEffectOrganizationLoaded = true;
    }
    const auto MatchesSearch = [&](const std::string& id)
    {
        if (strSearch.empty() || Contains_NoCase(id, strSearch)) return true;
        const auto found = m_SavedEffectOrganization.find(id);
        return found != m_SavedEffectOrganization.end() && (Contains_NoCase(found->second.first, strSearch) ||
            std::any_of(found->second.second.begin(), found->second.second.end(),
                [&](const auto& name) { return Contains_NoCase(name, strSearch); }));
    };
	std::vector<std::string> EffectIds;
	for (const std::string& strEffectAssetId : CEffectCatalog::Get_EffectAssetIds())
	{
		if (MatchesOwner(strEffectAssetId) &&
			CEffectCatalog::Is_DirectAuthoredDocument(strEffectAssetId) &&
			MatchesSearch(strEffectAssetId))
		{
			EffectIds.push_back(strEffectAssetId);
		}
	}
	std::ranges::sort(EffectIds);
	ImGui::SetNextItemOpen(true, strSearch.empty() ?
		ImGuiCond_FirstUseEver : ImGuiCond_Always);
	const std::string strLabel = std::string(pOwnerLabel) + " Saved Effects (" +
		std::to_string(EffectIds.size()) + ")";
	if (!ImGui::TreeNodeEx(strLabel.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		return;

	ImGui::TextWrapped("%s", bWorld ?
		"Play All animates the complete Effect at the scene player. Mouse Click plays once; Move Destination repeats. Open Editor exposes every Element." :
		"Play All starts the saved Effect at the scene player. Open Editor exposes each Element and its motion. Action Workbench chooses the anchor when it uses this Effect.");
	if (!m_strPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strPreviewStatus.c_str());
	if (m_pAuthoringSequencer && m_ActiveDocument &&
		MatchesOwner(m_ActiveDocument->strEffectAssetId))
	{
		ImGui::Text("Preview: %s | %.2f s", m_pAuthoringSequencer->Is_Active() ?
			(m_pAuthoringSequencer->Is_Paused() ? "Paused / finished" : "Playing") : "Stopped",
			m_pAuthoringSequencer->ClockMs() * .001);
		ImGui::TextWrapped("%s", m_pAuthoringSequencer->Status().c_str());
	}
	if (EffectIds.empty())
		ImGui::TextDisabled("No saved %s Effect matches the search.", pOwnerLabel);
	const auto RenderEffect = [&](const std::string& strEffectAssetId)
	{
		ImGui::PushID(strEffectAssetId.c_str());
		const auto organization = m_SavedEffectOrganization.find(strEffectAssetId);
        const std::string name = organization != m_SavedEffectOrganization.end() && !organization->second.first.empty() ?
            organization->second.first : strEffectAssetId;
        const bool_t open = ImGui::TreeNodeEx((name + "###SavedEffect").c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", strEffectAssetId.c_str());
		if (!open) { ImGui::PopID(); return; }
		const bool_t bActive = m_ActiveDocument.has_value() &&
			m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
			m_ActiveDocument->strEffectAssetId == strEffectAssetId;
		std::string strEditableStatus;
		const std::filesystem::path* pEditablePath =
			Observe_DirectAuthoredEditablePath(strEffectAssetId, strEditableStatus);
		ImGui::BeginDisabled(bActive || nullptr == pEditablePath);
		if (ImGui::SmallButton("Open Editor") && nullptr != pEditablePath)
		{
			std::string strExactStatus;
			const std::filesystem::path* pExactPath =
				Resolve_DirectAuthoredEditablePath(strEffectAssetId, strExactStatus);
			if (nullptr == pExactPath)
				m_strElementStatus = std::move(strExactStatus);
			else
				Try_LoadDocumentPath(*pExactPath, EFFECT_DOCUMENT_SOURCE::AUTHORED,
					strEffectAssetId, EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("%s", bActive ?
				"This Effect is already open in Current Effect." : strEditableStatus.c_str());
		ImGui::SameLine();
		ImGui::BeginDisabled(!bActive && nullptr == pEditablePath);
		if (ImGui::SmallButton("Play All"))
		{
			bool_t bLoaded = bActive;
			if (!bLoaded)
			{
				std::string strExactStatus;
				const std::filesystem::path* pExactPath =
					Resolve_DirectAuthoredEditablePath(strEffectAssetId, strExactStatus);
				if (nullptr == pExactPath)
					m_strElementStatus = std::move(strExactStatus);
				else
				{
					const std::filesystem::path ExactPath = *pExactPath;
					bLoaded = Try_LoadDocumentPath(ExactPath,
						EFFECT_DOCUMENT_SOURCE::AUTHORED, strEffectAssetId,
						EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT);
					if (!bLoaded)
					{
						if (m_PendingDocumentLoad.has_value() &&
							m_PendingDocumentLoad->Path == ExactPath &&
							m_PendingDocumentLoad->strSelectionId == strEffectAssetId &&
							m_PendingDocumentLoad->ePreviewIntent ==
								EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT)
						{
							m_PendingDocumentLoad->strElementSelectionId.clear();
							m_PendingDocumentLoad->strModelCueSelectionId.clear();
							m_PendingDocumentLoad->bPlayCompleteAfterLoad = true;
						}
						m_strElementStatus = m_strDocumentStatus;
					}
				}
			}
			if (bLoaded)
			{
				(void)Try_PlayActiveUnifiedEffect();
				m_strElementStatus = m_strPreviewStatus;
			}
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("%s", !bActive && nullptr == pEditablePath ?
				strEditableStatus.c_str() :
				"Replay every Element from zero at the current player position and facing.");
        ImGui::SameLine();
        ImGui::BeginDisabled(!m_pAuthoringSequencer || (!bActive && nullptr == pEditablePath));
        if (ImGui::SmallButton("Append Group"))
        {
            (void)m_pAuthoringSequencer->Append({EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, strEffectAssetId}, 0u);
            m_strElementStatus = m_pAuthoringSequencer->Status();
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Append an independent group at the benchmark cursor; tune its anchor, position and timing in Selected Group.");
        ImGui::TreePop();
        ImGui::PopID();
    };
    if (bWorld)
    {
        for (const auto& id : EffectIds) RenderEffect(id);
    }
    else
    {
        // The stable asset namespace supplies organization only. Opening a tree
        // never parses all particle documents or admits an unrelated boss graph.
        struct CATEGORY_NODE final
        {
            std::map<std::string, CATEGORY_NODE> children;
            std::vector<std::string> effects;
        };
        CATEGORY_NODE tree;
        for (const auto& id : EffectIds)
        {
            const auto organization = m_SavedEffectOrganization.find(id);
            std::vector<std::string> path;
            if (organization != m_SavedEffectOrganization.end()) path = organization->second.second;
            if (!path.empty() && path.front() == "KoukuSaydon") path.erase(path.begin());
            const auto start = id.find(".gate");
            std::string gate = "Common", category = "Other Effects";
            if (start != std::string::npos && start + 6u < id.size())
            {
                const auto end = id.find('.', start + 1u);
                gate = "Gate " + id.substr(start + 5u, end - start - 5u);
                if (end != std::string::npos)
                {
                    const auto next = id.find('.', end + 1u);
                    category = id.substr(end + 1u, next - end - 1u);
                }
            }
            if (category == "showtime") category = "Showtime";
            else if (category == "rainbow") category = "Rainbow";
            else if (category == "mario") category = "Mario";
            else if (category == "intro") category = "Intro";
            else if (category == "downstrike" || category == "slam" || category == "staff") category = "Staff Slam / Fire";
            else if (category == "spider") category = "Spider Counter";
            if (path.empty()) path = {gate, "Patterns", category};
            auto* node = &tree;
            for (const auto& segment : path) node = &node->children[segment];
            node->effects.push_back(id);
        }
        std::function<void(const CATEGORY_NODE&)> RenderCategory = [&](const CATEGORY_NODE& node)
        {
            for (const auto& [name, child] : node.children)
            {
                if (!strSearch.empty()) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
                { RenderCategory(child); ImGui::TreePop(); }
            }
            for (const auto& id : node.effects) RenderEffect(id);
        };
        RenderCategory(tree);
    }
    ImGui::TreePop();
}

void Client::CEffect_Tool::Render_AllEffectsWindow()
{
	ImGui::SetNextWindowPos(ImVec2(1110.f, 705.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(620.f, 560.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("All Effects"))
	{
		ImGui::End();
		return;
	}

	// Discover independent authored recoveries on the first visible open.
	// Only metadata is indexed here; Open/Play loads the selected document.
	if (!m_bAllEffectsRefreshAttempted)
		Refresh_AllEffects();
	if (!m_bDataFilesRefreshAttempted)
		Refresh_DataFiles();

	{
	if (m_bAllEffectsValtanBossSelected)
	{
		ImGui::TextWrapped(
			"Choose a Valtan Pattern and use Play Server for one Arena fixed-tick replay.");
		ImGui::TextDisabled(
			"Open Editor and local Play keep the Model View authoring timeline. Valtan Boss Tool owns Repeat and Revive.");
		if (Has_UnsavedWork())
		{
			ImGui::TextDisabled(
				"Complete Play uses the current Server Product; unsaved Effect edits are not included.");
		}
	}
	else
	{
		ImGui::TextDisabled(
			"Saved unified Effects come from EffectCatalog.json; files are parsed only when Open or Play is pressed.");
		if (!m_strUnifiedCandidateStatus.empty())
			ImGui::TextWrapped("%s", m_strUnifiedCandidateStatus.c_str());
	}
	const bool_t bValtanProductUnlinkPending =
		m_ValtanPatternProductUnlinkOperation.has_value();
	ImGui::BeginDisabled(bValtanProductUnlinkPending);
	const char_t* pAllEffectsOwnerLabel =
		m_bAllEffectsWorldSelected ? "World" :
		m_bAllEffectsKoukuBossSelected ? "KoukuSaydon" :
			m_bAllEffectsValtanBossSelected ?
				"Valtan" : Class_Label(m_eAllEffectsClass);
	if (ImGui::BeginCombo("Character / Boss / World", pAllEffectsOwnerLabel))
	{
		for (const EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTION& Owner :
			EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS)
		{
			const bool_t bValtanOwner =
				EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::VALTAN_BOSS == Owner.eKind;
			const bool_t bKoukuOwner =
				EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::KOUKU_BOSS == Owner.eKind;
			const bool_t bWorldOwner =
				EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::WORLD == Owner.eKind;
			if (bValtanOwner)
				ImGui::SeparatorText("Boss Patterns");
			const bool_t bSelected = bWorldOwner ? m_bAllEffectsWorldSelected : bKoukuOwner ?
				m_bAllEffectsKoukuBossSelected : bValtanOwner ?
				m_bAllEffectsValtanBossSelected :
				(!m_bAllEffectsValtanBossSelected &&
					!m_bAllEffectsKoukuBossSelected && !m_bAllEffectsWorldSelected &&
					Owner.eCharacterClass == m_eAllEffectsClass);
			if (ImGui::Selectable(Owner.strLabel.data(), bSelected))
			{
				m_bAllEffectsValtanBossSelected = bValtanOwner;
				m_bAllEffectsKoukuBossSelected = bKoukuOwner;
				m_bAllEffectsWorldSelected = bWorldOwner;
				if (bWorldOwner)
					Select_AuthoringDomain("World");
				else if (bKoukuOwner)
					Select_AuthoringDomain("KoukuSaydon");
				else if (!bValtanOwner)
				{
					m_eAllEffectsClass = Owner.eCharacterClass;
					Select_AuthoringDomainForClass(Owner.eCharacterClass);
				}
			}
		}
		ImGui::EndCombo();
	}
	ImGui::InputTextWithHint("##effect-search",
		(m_bAllEffectsValtanBossSelected || m_bAllEffectsKoukuBossSelected || m_bAllEffectsWorldSelected) ?
			"Search existing Effects, independent Effects or playable Patterns..." :
			"Search skill, Product cue, or saved Effect ID...",
		m_AllEffectsSearch.data(), m_AllEffectsSearch.size());
	ImGui::SameLine();
	if (ImGui::SmallButton("Refresh"))
	{
        m_bSavedEffectOrganizationLoaded = false;
		Refresh_AllEffects(true);
		Refresh_DataFiles();
        if (m_bAllEffectsValtanBossSelected)
        {
            Refresh_ValtanEffectResourceSnapshot();
            Refresh_ValtanPatternTree();
            Refresh_ValtanAreaStaticEffects();
        }
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Reload saved Effects and discover independent recovery copies under their Product source.");
	ImGui::SameLine();
	if (ImGui::SmallButton("Hide Preview"))
		Hide_WorldPreview();
	ImGui::EndDisabled();

	const std::string Search = m_AllEffectsSearch.data();
	const f32_t fStatusReserve = m_strElementStatus.empty() ? 1.f :
		ImGui::CalcTextSize(m_strElementStatus.c_str(), nullptr, false,
			ImGui::GetContentRegionAvail().x).y +
		ImGui::GetStyle().ItemSpacing.y;
	ImGui::BeginChild("ElementFirstEffectTree",
		ImVec2(0.f, -fStatusReserve), true);

	if (m_bAllEffectsWorldSelected || m_bAllEffectsKoukuBossSelected)
	{
		Render_SavedAuthoredEffectSection(Search, m_bAllEffectsWorldSelected);
	}
	else if (m_bAllEffectsValtanBossSelected)
	{
		Render_ValtanPatternTreeSection(Search);
	}
	else
	{
		const std::vector<PLAYER_SKILL_DEFINITION>& Skills =
			CPlayerSkillCatalog::Get_Skills();
		std::vector<const UNIFIED_EFFECT_CANDIDATE_BINDING*>
			UnjoinedEditorSources;
		for (const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding :
			m_UnifiedCandidateBindings)
		{
			if (Binding.eCharacterClass != m_eAllEffectsClass ||
				(!Search.empty() &&
				 !Contains_NoCase(Binding.strEffectAssetId, Search)))
			{
				continue;
			}
			const bool_t bHasSkillRow = std::any_of(
				Skills.begin(), Skills.end(),
				[&Binding](const PLAYER_SKILL_DEFINITION& Skill)
				{
					return Skill.eCharacterClass == Binding.eCharacterClass &&
						Skill.iSkillId == Binding.iSkillId;
				});
			if (!bHasSkillRow)
				UnjoinedEditorSources.push_back(&Binding);
		}
		if (!UnjoinedEditorSources.empty())
		{
			const std::string UnjoinedLabel =
				"INDEPENDENT / EDITOR-ONLY EFFECTS (" +
				std::to_string(UnjoinedEditorSources.size()) + ")";
			if (ImGui::TreeNodeEx(UnjoinedLabel.c_str(),
					ImGuiTreeNodeFlags_OpenOnArrow))
			{
				ImGui::TextDisabled(
					"PlayerSkills Product ownership is unavailable; exact authored JSON files remain editable and Play stays isolated.");
				for (const UNIFIED_EFFECT_CANDIDATE_BINDING* pBinding :
					UnjoinedEditorSources)
				{
					if (nullptr == pBinding)
						continue;
					ImGui::PushID(pBinding->strEffectAssetId.c_str());
					ImGui::TextWrapped("%s | skill %u | %s",
						pBinding->strEffectAssetId.c_str(),
						static_cast<uint32_t>(pBinding->iSkillId),
						pBinding->Path.generic_string().c_str());
					const bool_t bActive = m_ActiveDocument.has_value() &&
						m_eActiveDocumentSource ==
							EFFECT_DOCUMENT_SOURCE::AUTHORED &&
						m_ActiveDocument->strEffectAssetId ==
							pBinding->strEffectAssetId;
					std::string strEditableStatus;
					const std::filesystem::path* pEditablePath =
						Observe_DirectAuthoredEditablePath(
							pBinding->strEffectAssetId,
							strEditableStatus);
					ImGui::BeginDisabled(
						bActive || nullptr == pEditablePath);
					if (ImGui::SmallButton("Open Editor") &&
						nullptr != pEditablePath)
					{
						std::string strExactStatus;
						const std::filesystem::path* pExactPath =
							Resolve_DirectAuthoredEditablePath(
								pBinding->strEffectAssetId,
								strExactStatus);
						if (nullptr == pExactPath)
							m_strElementStatus =
								std::move(strExactStatus);
						else
							Try_LoadDocumentPath(*pExactPath,
								EFFECT_DOCUMENT_SOURCE::AUTHORED,
								pBinding->strEffectAssetId);
					}
					ImGui::EndDisabled();
					if (ImGui::IsItemHovered(
							ImGuiHoveredFlags_AllowWhenDisabled))
					{
						ImGui::SetTooltip("%s", bActive ?
							"This exact authored document is already the Current Effect." :
							strEditableStatus.c_str());
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
		}
		static const std::vector<EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE>
			EmptyProductCues;
		ImGui::SeparatorText("SAVED / PLAYABLE SKILL EFFECTS");
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
			if (ProductCues.empty() && SavedBindings.empty() &&
				!bArtistFToolAdapter)
			{
				continue;
			}
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
				if (Skill.eSkillKind ==
					LostArk::Shared::PLAYER_SKILL_KIND::COMBO)
				{
					ImGui::BeginDisabled(nullptr == pProductEntry ||
						ProductCues.empty());
					if (ImGui::Button("Play Buffered Combo Audition"))
						Try_PlayBufferedComboAudition(*pProductEntry);
					ImGui::EndDisabled();
					ImGui::SameLine();
					ImGui::TextDisabled(
						"Authoring only: non-final stages cut at Server comboAdvanceMs; the final stage uses actionDurationMs.");
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
									"Saved authored source for a matching Legacy reference; this lane is not direct-source Product runtime.");
							}
							else
							{
								ImGui::TextDisabled(
									"Saved authored source; no active Product cue mapping is inferred.");
							}
							ImGui::TextWrapped("Path: %s",
								pBinding->Path.generic_string().c_str());
							ImGui::BeginDisabled(bActive);
							if (ImGui::SmallButton("Open Editor"))
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
								ImGui::TextWrapped("Last document validation (not an Element Solo check): %s",
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
							const std::string strUnifiedCandidateId =
								Unified_CandidateAssetId(Cue.Cue.strEffectAssetId);
							const auto AuthoredBinding = std::find_if(
								SavedBindings.begin(), SavedBindings.end(),
								[&Cue, &strUnifiedCandidateId](
									const UNIFIED_EFFECT_CANDIDATE_BINDING* pBinding)
								{
									return nullptr != pBinding &&
										(pBinding->strEffectAssetId ==
											Cue.Cue.strEffectAssetId ||
										 pBinding->strEffectAssetId ==
											strUnifiedCandidateId);
								});
							std::string strEditorAssetId =
								AuthoredBinding == SavedBindings.end() ?
									Cue.Cue.strEffectAssetId :
									(*AuthoredBinding)->strEffectAssetId;
							std::string strEditableStatus;
							const std::filesystem::path* pEditablePath =
								Observe_DirectAuthoredEditablePath(
									strEditorAssetId, strEditableStatus);
							if (nullptr == pEditablePath &&
								strUnifiedCandidateId != strEditorAssetId)
							{
								strEditorAssetId = strUnifiedCandidateId;
								pEditablePath = Observe_DirectAuthoredEditablePath(
									strEditorAssetId, strEditableStatus);
							}
							const bool_t bEditorDocumentActive =
								m_ActiveDocument.has_value() &&
								m_eActiveDocumentSource ==
									EFFECT_DOCUMENT_SOURCE::AUTHORED &&
								m_ActiveDocument->strEffectAssetId == strEditorAssetId;
							/* Product cue ownership, decoded cache, drawable state and
							   Track A are Play/diagnostic contracts. The exact authored
							   source path alone owns this editor entry point. */
							ImGui::BeginDisabled(
								bEditorDocumentActive || nullptr == pEditablePath);
							if (ImGui::SmallButton("Open Editor") &&
								nullptr != pEditablePath)
							{
								std::string strExactStatus;
								const std::filesystem::path* pExactPath =
									Resolve_DirectAuthoredEditablePath(
										strEditorAssetId, strExactStatus);
								if (nullptr == pExactPath)
									m_strElementStatus =
										std::move(strExactStatus);
								else
									Try_LoadDocumentPath(*pExactPath,
										EFFECT_DOCUMENT_SOURCE::AUTHORED,
										strEditorAssetId);
							}
							ImGui::EndDisabled();
							if (ImGui::IsItemHovered(
									ImGuiHoveredFlags_AllowWhenDisabled))
							{
								ImGui::SetTooltip("%s", bEditorDocumentActive ?
									"This exact authored document is already the Current Effect." :
									strEditableStatus.c_str());
							}
							if (AuthoredBinding != SavedBindings.end())
							{
								const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding =
									**AuthoredBinding;
								auto Cache = m_UnifiedCandidateCaches.find(
									Binding.strEffectAssetId);
								if (Cache != m_UnifiedCandidateCaches.end() &&
									Cache->second.bObserved &&
									Cache->second.bValid)
								{
									Render_UnifiedEffectTree(Cache->second,
										"Editable Unified Effect | " +
											Binding.strEffectAssetId);
								}
								else
								{
									ImGui::TextDisabled(
										"Product preview tree unavailable: %s",
										Cache == m_UnifiedCandidateCaches.end() ?
											"the authored cache lost this Product ID." :
											(!Cache->second.bObserved ?
												"Open Editor or Play Saved Effect to load Details on demand." :
												Cache->second.strStatus.c_str()));
								}
							}
							else if (nullptr == pEditablePath)
							{
								ImGui::TextDisabled(
									"No exact direct-authored document is indexed for this Product cue.");
							}
							if (ImGui::Button("Play Full Effect"))
								Try_SelectProductCue(*pProductEntry, iCue);
							Render_VisualProgramAuthoring(*pProductEntry, iCue);
                            Render_RecoveryEffectForProduct(Cue.Cue.strEffectAssetId);
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
		"Saved Effect cues are usable presets; reference packages remain read-only.");
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
					"Core F 33 MeshParticle SpriteParticle LocalDecal CascadeRibbon",
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
				"Skill F | Core F (33)",
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
					"Active saved Effect cue missing; reference rows are read-only.";
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
					Observe_DirectAuthoredEditablePath(
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
					std::string strExactStatus;
					const std::filesystem::path* pExactPath =
						Resolve_DirectAuthoredEditablePath(
							strProductEffectAssetId, strExactStatus);
					if (nullptr == pExactPath)
						m_strElementStatus = std::move(strExactStatus);
					else
						Try_LoadDocumentPath(*pExactPath,
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
				"Reference Sources (" +
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
					"Legacy Assembly (read-only)"))
				{
					Render_AssemblyHierarchy(strProductEffectAssetId);
					ImGui::TreePop();
				}
				ImGui::TreePop();
				ImGui::PopID();
				continue;
			}
			if (bHasPublishedAssembly && bActiveProductDocument && ImGui::TreeNode(
				"Legacy Assembly (read-only)"))
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
					"Cascade Particle System | Source Systems " +
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
			"Current Effect / Reference",
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
			"This Document has no manual Hit groups; use All Effects for read-only references.");
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
    // Saved tree placement/Sequencer commands retain their typed owner. The
    // Data Files browser also exposes the existing per-Element copy workflow.
    if (m_pAuthoringResources) Render_AuthoringResourceTree();
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
    if (ImGui::CollapsingHeader("Advanced Document Commands"))
    {
        ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
            m_NewAssetId.size());
        ImGui::InputText("Display Name", m_NewDisplayName.data(),
            m_NewDisplayName.size());
		ImGui::TextDisabled(
			"New Effect -> Create Element -> choose Resources -> tune Details -> Save.");
		if (ImGui::Button("New Effect"))
            Try_CreateDocument();
        ImGui::SameLine();
		const bool_t bRegistryBoundAuditionActive =
			m_ActiveDocument.has_value() &&
			m_ActiveRegistryBoundAuditionProvenance.has_value() &&
			m_ActiveDocument->strEffectAssetId ==
				m_ActiveRegistryBoundAuditionProvenance->strEffectAssetId;
		ImGui::BeginDisabled(bRegistryBoundAuditionActive);
        if (ImGui::Button("Save As"))
            Try_SaveDocumentAs(m_NewAssetId.data());
		ImGui::EndDisabled();
		if (bRegistryBoundAuditionActive && ImGui::IsItemHovered(
				ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"This Effect keeps its current stable ID. Use Save in Effect Detail.");
		}
        ImGui::SameLine();
		if (ImGui::Button("Load Saved"))
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
        const std::string& CurrentName = m_ActiveDocument->strDisplayName.empty() ?
            m_ActiveDocument->strEffectAssetId : m_ActiveDocument->strDisplayName;
        ImGui::Text("Current: %s | %zu Elements%s", CurrentName.c_str(),
            m_ActiveDocument->Elements.size(), Has_UnsavedWork() ? " | DIRTY" : "");
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
		std::vector<EFFECT_DATA_FILE_ENTRY*>> SavedEffectsBySkill;
	SavedEffectsBySkill.reserve(m_AllEffects.size());
	std::vector<EFFECT_DATA_FILE_ENTRY*> UnassignedEffects;
	size_t iVisibleSavedEffects = 0u;
	for (EFFECT_DATA_FILE_ENTRY& DataFile : m_DataFiles)
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
	ImGui::TextDisabled("%zu saved Effects", iVisibleSavedEffects);
	const bool_t bReferenceFilesOpen = ImGui::GetStateStorage()->GetInt(
		ImGui::GetID("Reference Files (read-only)"), 0) != 0;
	const f32_t fFooterHeight = 3.f * ImGui::GetFrameHeightWithSpacing() +
		(m_strDocumentStatus.empty() ? 0.f : ImGui::GetTextLineHeightWithSpacing()) +
		(bReferenceFilesOpen ? 110.f + ImGui::GetFrameHeightWithSpacing() +
			ImGui::GetStyle().ItemSpacing.y : 0.f);
	const f32_t fSavedListHeight = (std::max)(180.f,
		ImGui::GetContentRegionAvail().y - fFooterHeight);
	ImGui::BeginChild("SavedEffectDataFileList", ImVec2(0.f, fSavedListHeight), true);
	const auto RenderSavedEffectRow = [this](
		EFFECT_DATA_FILE_ENTRY& DataFile,
		const EFFECT_SKILL_TREE_ENTRY* pSkill)
	{
		std::string RowLabel = "Saved Effect";
		bool_t bGameplayLinked = false;
		if (nullptr == pSkill)
		{
			RowLabel = nullptr != DataFile.pParsedDocument &&
				!DataFile.pParsedDocument->strDisplayName.empty() ?
				DataFile.pParsedDocument->strDisplayName + " | " +
					DataFile.strAssetId :
				DataFile.strAssetId;
		}
		else
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
				RowLabel = "Saved Skill Draft (not cataloged or gameplay-bound)";
			}
		}
		/* The physical document is decoded only after this row opens. Its
		   display name may therefore appear on the next frame; keep the ImGui
		   identity stable so that loading the Element tree does not collapse it. */
		RowLabel += "###saved-" + DataFile.strAssetId;
		const bool_t bEffectSelected =
			DataFile.strAssetId == m_strSelectedDataFileAssetId &&
			m_strSelectedDataFileElementId.empty();
		const bool_t bEffectOpen = ImGui::TreeNodeEx(RowLabel.c_str(),
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			(bEffectSelected ? ImGuiTreeNodeFlags_Selected : 0));
		if (ImGui::IsItemClicked())
		{
			m_strSelectedDataFileAssetId = DataFile.strAssetId;
			m_strSelectedDataFileElementId.clear();
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
			ImGui::SetTooltip("Effect Asset ID: %s\nGameplay mapping: %s\nTree parse: %s\n%s",
				DataFile.strAssetId.c_str(),
				bGameplayLinked ? "linked" : "not linked",
				DataFile.strDocumentParseStatus.empty() ? "not available" :
					DataFile.strDocumentParseStatus.c_str(),
				DataFile.Path.string().c_str());
		if (!bEffectOpen)
			return;

		(void)Ensure_DataFileDocumentParsed(DataFile);

		if (nullptr == DataFile.pParsedDocument)
		{
			ImGui::TextDisabled("%s",
				DataFile.strDocumentParseStatus.empty() ?
					"Saved Effect tree is unavailable." :
					DataFile.strDocumentParseStatus.c_str());
			ImGui::TreePop();
			return;
		}
		const EFFECT_DOCUMENT_DESC& Document = *DataFile.pParsedDocument;
		if (Document.Elements.empty())
			ImGui::TextDisabled("This saved Effect has no drawable Elements yet.");
		for (int32_t iFamily = 0;
			iFamily < static_cast<int32_t>(EFFECT_AUTHORING_FAMILY::END);
			++iFamily)
		{
			const EFFECT_AUTHORING_FAMILY eFamily =
				static_cast<EFFECT_AUTHORING_FAMILY>(iFamily);
			const size_t iCount = static_cast<size_t>(std::count_if(
				Document.Elements.begin(), Document.Elements.end(),
				[eFamily](const EFFECT_ELEMENT_DESC& Element)
				{
					return Resolve_AuthoringFamily(Element) == eFamily;
				}));
			if (0u == iCount)
				continue;
			ImGui::PushID(iFamily);
			const std::string FamilyLabel = std::string(
				AuthoringFamily_Label(eFamily)) + " (" +
				std::to_string(iCount) + ")";
			if (ImGui::TreeNodeEx(FamilyLabel.c_str(),
				ImGuiTreeNodeFlags_OpenOnArrow))
			{
				size_t iOrdinal = 0u;
				for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
				{
					if (Resolve_AuthoringFamily(Element) != eFamily)
						continue;
					++iOrdinal;
					ImGui::PushID(Element.strElementId.c_str());
					const std::string ElementLabel =
						FriendlyAuthoringElementLabel(
							eFamily, iOrdinal, Element);
					const bool_t bElementSelected =
						DataFile.strAssetId ==
							m_strSelectedDataFileAssetId &&
						Element.strElementId ==
							m_strSelectedDataFileElementId;
					if (ImGui::Selectable(
						ElementLabel.c_str(), bElementSelected))
					{
						m_strSelectedDataFileAssetId =
							DataFile.strAssetId;
						m_strSelectedDataFileElementId =
							Element.strElementId;
						Select_AuthoringDomain(DataFile.strDomainId);
						Copy_Buffer(m_NewAssetId.data(),
							m_NewAssetId.size(), DataFile.strAssetId);
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip(
							"Stable Element: %s\nFamily: %s\nSlots: %s",
							Element.strElementId.c_str(),
							AuthoringFamily_Label(eFamily),
							AuthoringElementResourceSlotSummary(
								Element).c_str());
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
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
			for (EFFECT_DATA_FILE_ENTRY* pDataFile : SkillEffects->second)
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
		for (EFFECT_DATA_FILE_ENTRY* pDataFile : UnassignedEffects)
			RenderSavedEffectRow(*pDataFile, nullptr);
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
					m_strSelectedDataFileElementId.clear();
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
	const EFFECT_ELEMENT_DESC* pSelectedSavedElement = nullptr;
	if (bSelectedSavedEffect &&
		nullptr != SelectedDataFile->pParsedDocument &&
		!m_strSelectedDataFileElementId.empty())
	{
		const auto SelectedElement = std::find_if(
			SelectedDataFile->pParsedDocument->Elements.begin(),
			SelectedDataFile->pParsedDocument->Elements.end(),
			[this](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId ==
					m_strSelectedDataFileElementId;
			});
		if (SelectedElement !=
			SelectedDataFile->pParsedDocument->Elements.end())
		{
			pSelectedSavedElement = &*SelectedElement;
		}
	}
	const bool_t bCanAppendSavedElement =
		nullptr != pSelectedSavedElement &&
		AuthoringFamily_CanCreate(
			Resolve_AuthoringFamily(*pSelectedSavedElement)) &&
		m_ActiveDocument.has_value() &&
		(EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource ||
		 EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource) &&
		!Has_UnappliedDetailDraft();
	const bool_t bSelectedMigrationReference =
		SelectedDataFile != m_DataFiles.end() &&
		IsLegacyMigrationReference(*SelectedDataFile);
	ImGui::BeginDisabled(!bCanAppendSavedElement);
	if (ImGui::Button("Add Element to Current Effect") &&
		SelectedDataFile != m_DataFiles.end())
	{
		Try_AppendSavedElementToActiveDocument(
			SelectedDataFile->Path,
			SelectedDataFile->strAssetId,
			m_strSelectedDataFileElementId);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			"Select one self-contained Mesh, Sprite, Particle, or Decal child row. Apply or Revert the open Detail draft before adding it to Current Effect. "
			"Elements with owner or transform-history dependencies require the complete Effect.");
	}
	ImGui::SameLine();
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
	if (ImGui::CollapsingHeader(
		"Reference Files (read-only)"))
    {
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
				m_strSelectedDataFileElementId.clear();
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
    if (!m_strDocumentStatus.empty())
    {
        const size_t iFirstLineEnd = m_strDocumentStatus.find_first_of("\r\n");
        const char* pStatus = m_strDocumentStatus.c_str();
        ImGui::TextUnformatted(pStatus, pStatus +
            (iFirstLineEnd == std::string::npos ? m_strDocumentStatus.size() : iFirstLineEnd));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", m_strDocumentStatus.c_str());
    }
    ImGui::End();
}
