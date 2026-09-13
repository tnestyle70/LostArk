#pragma once
// Explicit selected LanceMasterVA source material branches. Engine prefix/fog/depth adapters
// are in Shader_EffectLanceMasterVANative.hlsli, not unnamed JSON fallbacks.
#include "Effect_AuthoringDocument.h"
#include <array>
#include <span>
#include <string_view>
#include <algorithm>
#include <cmath>

NS_BEGIN(Client)
struct LANCEMASTER_VA_PARAMETER_DESC final
{
    std::string_view strName;
    uint32_t iRow;
    uint32_t iLane;
    bool bVector;
};
struct LANCEMASTER_VA_SWITCH_DESC final
{
    std::string_view strName;
    bool bValue;
};
struct LANCEMASTER_VA_PROGRAM_DESC final
{
    uint32_t iProfileIndex;
    std::string_view strRuntimeProfileId;
    std::string_view strSourceMaterialPath;
    std::string_view strParentMaterialPath;
    std::string_view strProfileId;
    bool bMesh;
    bool bModelCue;
    std::string_view strRendererShape;
    bool bNeedsSceneColor;
    bool bNeedsDepthSample;
    bool bNeedsTangentView;
    bool bDynamicVertexFactory;
    EFFECT_RENDER_PROFILE eRenderProfile;
    std::span<const std::string_view> TextureNames;
    std::span<const LANCEMASTER_VA_PARAMETER_DESC> Parameters;
    std::span<const LANCEMASTER_VA_SWITCH_DESC> StaticSwitches;
};

// Native material tables are compiled once; generators use native_material_tables.py.
// native-material-tables: ../Private/Effect_LanceMasterVAMaterial_Tables.inl
extern const std::span<const LANCEMASTER_VA_PROGRAM_DESC> LANCEMASTER_VA_PROGRAMS;
// native-material-tables-end

inline bool Has_LanceVAStaticSourceAsset(uint32_t Program, const std::string& AssetId)
{
    switch(Program)
    {
    case 781u: return AssetId=="Effect/LanceMaster/Meshes/Native/LV_MATTE/sky_mirror_sm.section0.wmodel";
    case 782u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01a_sm.section0.wmodel";
    case 783u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01a_sm.section1.wmodel";
    case 784u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section9.wmodel" || AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01a_sm.section2.wmodel";
    case 785u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01a_sm.section3.wmodel";
    case 786u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01b_sm.section0.wmodel";
    case 787u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01b_sm.section1.wmodel";
    case 788u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01b_sm.section2.wmodel";
    case 789u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01c_sm.section0.wmodel";
    case 790u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section0.wmodel";
    case 791u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section1.wmodel";
    case 792u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section2.wmodel";
    case 793u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section3.wmodel";
    case 794u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section4.wmodel";
    case 795u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section5.wmodel";
    case 796u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section6.wmodel";
    case 797u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section7.wmodel";
    case 798u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section8.wmodel";
    case 799u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section10.wmodel";
    case 800u: return AssetId=="Effect/LanceMaster/Meshes/Native/BG_GDOGODS_C/bg_gdogods_skillfloor01_sm.section11.wmodel";
    default: return false;
    }
}

inline const LANCEMASTER_VA_PROGRAM_DESC* Find_LanceMasterVAProgram(
    const std::string_view runtimeId)
{
    for (const auto& Program : LANCEMASTER_VA_PROGRAMS)
        if (Program.strRuntimeProfileId == runtimeId) return &Program;
    return nullptr;
}

inline bool Build_LanceMasterVAParameters(const EFFECT_SOURCE_MATERIAL_DESC& Source,
    std::array<float4_t,32>& Output)
{
    const auto* Program=Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId);
    if (!Program || !Source.bEnabled || Source.strProfileId!=Program->strProfileId ||
        Source.strParentMaterialPath!=Program->strParentMaterialPath) return false;
    std::array<float4_t,32> Candidate{};
    const auto vectorCount=std::count_if(Program->Parameters.begin(),Program->Parameters.end(),
        [](const auto& P){return P.bVector;});
    if (Source.Vectors.size()!=static_cast<std::size_t>(vectorCount) ||
        Source.Scalars.size()!=Program->Parameters.size()-static_cast<std::size_t>(vectorCount)) return false;
    for (const auto& P : Program->Parameters)
    {
        if (P.bVector)
        {
            const auto count=std::count_if(Source.Vectors.begin(),Source.Vectors.end(),
                [&](const auto& V){return V.strName==P.strName;});
            if (count!=1) return false;
            const auto it=std::find_if(Source.Vectors.begin(),Source.Vectors.end(),
                [&](const auto& V){return V.strName==P.strName;});
            const auto& V=it->vValue;
            if (!std::isfinite(V.x)||!std::isfinite(V.y)||!std::isfinite(V.z)||!std::isfinite(V.w)) return false;
            Candidate[P.iRow]=V;
        }
        else
        {
            const auto count=std::count_if(Source.Scalars.begin(),Source.Scalars.end(),
                [&](const auto& V){return V.strName==P.strName;});
            if (count!=1) return false;
            const auto it=std::find_if(Source.Scalars.begin(),Source.Scalars.end(),
                [&](const auto& V){return V.strName==P.strName;});
            if (!std::isfinite(it->fValue)) return false;
            auto& V=Candidate[P.iRow];
            if (P.iLane==0u) V.x=it->fValue;
            else if (P.iLane==1u) V.y=it->fValue;
            else if (P.iLane==2u) V.z=it->fValue;
            else V.w=it->fValue;
        }
    }
    Output=Candidate;
    return true;
}

inline bool Has_LanceMasterVAMaterialContract(const EFFECT_ELEMENT_DESC& Element)
{
    const auto& Source=Element.Material.SourceMaterial;
    const auto* Program=Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId);
    if (!Program || Program->bModelCue || Element.Material.Execution.bEnabled || Element.Material.Execution.bFailClosed ||
        Element.Material.eRenderProfile!=Program->eRenderProfile ||
        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath) return false;
    const bool bStaticAction = Program->strRendererShape == "staticMesh";
    if (bStaticAction)
    {
        const auto& Attachment = Element.ActionCueAttachment;
        if (Element.eKind != EFFECT_ELEMENT_KIND::MESH || Element.SourceRecipe.bEnabled ||
            !Attachment.bEnabled || Attachment.bFollow ||
            Attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::BONE ||
            Attachment.strSourceAnchorSlotId != "EffectRoot" || Attachment.strRuntimeAnchorSlotId != "root" ||
            !Attachment.strRuntimeBoneName.empty() || Attachment.fSnapshotRootSourceBasisYawDegrees != -90.f ||
            Element.Detail.Mesh.bUseModelMaterial || Element.Detail.Mesh.fModelPreScale != .01f) return false;
        if (std::count_if(Element.ResourceBindings.begin(),Element.ResourceBindings.end(),[&](const auto& B)
            { return B.strSlotId=="meshModel" && Has_LanceVAStaticSourceAsset(Program->iProfileIndex,B.strAssetId); }) != 1) return false;
    }
    else if (Element.eKind!=(Program->strRendererShape=="screenPost" ? EFFECT_ELEMENT_KIND::SCREEN_POST : EFFECT_ELEMENT_KIND::PARTICLE) ||
        !Element.SourceRecipe.bEnabled || Element.SourceRecipe.strRendererShape!=Program->strRendererShape) return false;
    const auto meshCount=std::count_if(Element.ResourceBindings.begin(),Element.ResourceBindings.end(),
        [](const auto& B){return B.strSlotId=="meshModel"&&!B.strAssetId.empty();});
    if (meshCount!=(Program->bMesh ? 1 : 0)) return false;
    if (Source.Textures.size()!=Program->TextureNames.size() ||
        Source.StaticSwitches.size()!=Program->StaticSwitches.size()) return false;
    for (const auto name : Program->TextureNames)
    {
        if (std::count_if(Source.Textures.begin(),Source.Textures.end(),[&](const auto& T){
            return T.strName==name&&!T.strAssetId.empty()&&!T.strSourceObjectPath.empty();})!=1) return false;
    }
    for (const auto& S : Program->StaticSwitches)
    {
        if (std::count_if(Source.StaticSwitches.begin(),Source.StaticSwitches.end(),
            [&](const auto& V){return V.strName==S.strName&&V.bValue==S.bValue;})!=1) return false;
    }
    std::array<float4_t,32> Parameters{};
    return Build_LanceMasterVAParameters(Source,Parameters);
}
inline bool Has_LanceMasterVAModelCueMaterialContract(const EFFECT_MODEL_CUE_DESC& Cue)
{
    if (!Cue.Material) return false;
    const auto& Material=*Cue.Material;
    const auto& Source=Material.SourceMaterial;
    const auto* Program=Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId);
    if (!Program || !Program->bModelCue || Material.Execution.bEnabled || Material.Execution.bFailClosed ||
        Material.eRenderProfile!=Program->eRenderProfile || Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||
        Source.Textures.size()!=Program->TextureNames.size() || Source.StaticSwitches.size()!=Program->StaticSwitches.size()) return false;
    const std::string_view expected=Program->strSourceMaterialPath=="sk_flm_hor_01.mat.sk_flm_hor_01_01_mi_dead" ?
        "Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section0.wmodel" :
        Program->strSourceMaterialPath=="sk_flm_hor_01.mat.sk_flm_hor_01_02_mi_dead" ?
        "Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section1.wmodel" :
        Program->strSourceMaterialPath=="sk_flm_hor_01.mat.sk_flm_hor_01_03_dead" ?
        "Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section2.wmodel" :
        "Effect/LanceMaster/Models/SK_FLM_HOR_00/sk_flm_hor_00_sk.section3.wmodel";
    const bool bTDragon = Program->iProfileIndex == 1360u &&
        Program->strSourceMaterialPath == "fx_m_mi_t_00.fx_mi.fx_t_me_master_02_01_sk_dt_tr";
    if (bTDragon)
    {
        if (Cue.strModelAssetId != "Effect/LanceMaster/Models/SK_FLM_GDR_01/sk_flm_pmshb_00_sk.section0.wmodel" &&
            Cue.strModelAssetId != "Effect/LanceMaster/Models/SK_FLM_GDR_01/sk_flm_pmshb_00_sk.section1.wmodel") return false;
        if (Cue.strClipName != "sk_dragoncleave_03") return false;
    }
    else if (Cue.strModelAssetId != expected) return false;
    for (const auto name: Program->TextureNames)
        if(std::count_if(Source.Textures.begin(),Source.Textures.end(),[&](const auto& T){return T.strName==name&&!T.strAssetId.empty()&&!T.strSourceObjectPath.empty();})!=1) return false;
    for (const auto& S:Program->StaticSwitches)
        if(std::count_if(Source.StaticSwitches.begin(),Source.StaticSwitches.end(),[&](const auto& V){return V.strName==S.strName&&V.bValue==S.bValue;})!=1) return false;
    std::array<float4_t,32> Parameters{};
    return Build_LanceMasterVAParameters(Source,Parameters);
}
NS_END
