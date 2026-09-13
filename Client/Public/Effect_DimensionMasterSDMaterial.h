#pragma once
// Explicit selected S_D unlit material branches. Engine prefix/fog/depth adapters
// are in Shader_EffectDimensionMasterSDNative.hlsli, not unnamed JSON fallbacks.
#include "Effect_AuthoringDocument.h"
#include <array>
#include <span>
#include <string_view>
#include <algorithm>
#include <cmath>

NS_BEGIN(Client)
struct DIMENSIONMASTER_SD_PARAMETER_DESC final
{
    std::string_view strName;
    uint32_t iRow;
    uint32_t iLane;
    bool bVector;
};
struct DIMENSIONMASTER_SD_SWITCH_DESC final
{
    std::string_view strName;
    bool bValue;
};
struct DIMENSIONMASTER_SD_PROGRAM_DESC final
{
    uint32_t iProfileIndex;
    std::string_view strRuntimeProfileId;
    std::string_view strSourceMaterialPath;
    std::string_view strParentMaterialPath;
    std::string_view strProfileId;
    bool bMesh;
    std::string_view strRendererShape;
    bool bNeedsSceneColor;
    bool bNeedsDepthSample;
    bool bNeedsTangentView;
    EFFECT_RENDER_PROFILE eRenderProfile;
    std::span<const std::string_view> TextureNames;
    std::span<const DIMENSIONMASTER_SD_PARAMETER_DESC> Parameters;
    std::span<const DIMENSIONMASTER_SD_SWITCH_DESC> StaticSwitches;
    bool bUsesOneLayerDistortion = false;
};

// Native material tables are compiled once; generators use native_material_tables.py.
// native-material-tables: ../Private/Effect_DimensionMasterSDMaterial_Tables.inl
extern const std::span<const DIMENSIONMASTER_SD_PROGRAM_DESC> DIMENSIONMASTER_SD_PROGRAMS;
// native-material-tables-end

inline const DIMENSIONMASTER_SD_PROGRAM_DESC* Find_DimensionMasterSDProgram(
    const std::string_view runtimeId)
{
    for (const auto& Program : DIMENSIONMASTER_SD_PROGRAMS)
        if (Program.strRuntimeProfileId == runtimeId) return &Program;
    return nullptr;
}

inline bool Build_DimensionMasterSDParameters(const EFFECT_SOURCE_MATERIAL_DESC& Source,
    std::array<float4_t,32>& Output)
{
    const auto* Program=Find_DimensionMasterSDProgram(Source.strRuntimeShaderProfileId);
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

inline bool Has_DimensionMasterSDMaterialContract(const EFFECT_ELEMENT_DESC& Element)
{
    const auto& Source=Element.Material.SourceMaterial;
    const auto* Program=Find_DimensionMasterSDProgram(Source.strRuntimeShaderProfileId);
    if (!Program || Element.Material.Execution.bEnabled ||
        Element.Material.eRenderProfile!=Program->eRenderProfile ||
        Element.eKind!=EFFECT_ELEMENT_KIND::PARTICLE || !Element.SourceRecipe.bEnabled ||
        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||
        Element.SourceRecipe.strRendererShape!=Program->strRendererShape) return false;
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
    return Build_DimensionMasterSDParameters(Source,Parameters);
}
NS_END
