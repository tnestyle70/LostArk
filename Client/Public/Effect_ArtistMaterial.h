#pragma once
// Explicit selected Artist source material branches. Engine prefix/fog/depth adapters
// are in Shader_EffectArtistNative.hlsli, not unnamed JSON fallbacks.
#include "Effect_AuthoringDocument.h"
#include <array>
#include <span>
#include <string_view>
#include <algorithm>
#include <cmath>

NS_BEGIN(Client)
struct ARTIST_PARAMETER_DESC final
{
    std::string_view strName;
    uint32_t iRow;
    uint32_t iLane;
    bool bVector;
};
struct ARTIST_SWITCH_DESC final
{
    std::string_view strName;
    bool bValue;
};
struct ARTIST_PROGRAM_DESC final
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
    std::span<const ARTIST_PARAMETER_DESC> Parameters;
    std::span<const ARTIST_SWITCH_DESC> StaticSwitches;
    bool bSourceTransformMesh = false;
};

// Native material tables are compiled once; generators use native_material_tables.py.
// native-material-tables: ../Private/Effect_ArtistMaterial_Tables.inl
extern const std::span<const ARTIST_PROGRAM_DESC> ARTIST_PROGRAMS;
// native-material-tables-end

inline const ARTIST_PROGRAM_DESC* Find_ArtistProgram(
    const std::string_view runtimeId)
{
    for (const auto& Program : ARTIST_PROGRAMS)
        if (Program.strRuntimeProfileId == runtimeId) return &Program;
    return nullptr;
}

inline bool Build_ArtistParameters(const EFFECT_SOURCE_MATERIAL_DESC& Source,
    std::array<float4_t,32>& Output)
{
    const auto* Program=Find_ArtistProgram(Source.strRuntimeShaderProfileId);
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

// The native program owns the named parameter packing. Curves reuse those exact lanes.
inline bool Build_ArtistMaterialTrackBindings(const EFFECT_ELEMENT_DESC& Element,
    std::vector<ARTIST_PARAMETER_DESC>& Bindings, std::string& Error)
{
    if (!Element.SourceTransformTrack || Element.SourceTransformTrack->MaterialParameterTracks.empty())
    { Bindings.clear(); return true; }
    const auto& Tracks = Element.SourceTransformTrack->MaterialParameterTracks;
    const auto* Program = Find_ArtistProgram(Element.Material.SourceMaterial.strRuntimeShaderProfileId);
    std::array<float4_t, 32u> Parameters;
    const bool bSourceMesh = Program && Element.eKind == EFFECT_ELEMENT_KIND::MESH &&
        Program->bMesh && Program->bSourceTransformMesh;
    // LocalDecal already binds the same named native parameter packet at the
    // document source clock. Keep that carrier's existing SourceRecipe contract.
    const bool bSourceDecal = Program && Element.eKind == EFFECT_ELEMENT_KIND::DECAL &&
        !Program->bMesh && !Program->bModelCue && Program->strRuntimeProfileId.starts_with("effect.ue3.kouku-") &&
        Program->strRendererShape == "decal" && Element.SourceRecipe.bEnabled &&
        Element.SourceRecipe.strRendererShape == "decal";
    if ((!bSourceMesh && !bSourceDecal) ||
        Tracks.size() > 64u || !std::isfinite(Element.SourceTransformTrack->fSourceTimeOriginSeconds) ||
        !Build_ArtistParameters(Element.Material.SourceMaterial, Parameters))
    { Error = "Source material parameter tracks require an admitted native Mesh or LocalDecal material."; return false; }
    std::vector<ARTIST_PARAMETER_DESC> Candidate;
    for (const auto& Track : Tracks)
    {
        const auto Parameter = std::find_if(Program->Parameters.begin(), Program->Parameters.end(),
            [&](const auto& Value) { return Value.strName == Track.strName && Value.bVector == Track.bVector; });
        if (Parameter == Program->Parameters.end() || Parameter->iRow >= 32u || Parameter->iLane >= 4u ||
            (Parameter->bVector && Parameter->iLane != 0u) ||
            std::any_of(Candidate.begin(), Candidate.end(), [&](const auto& Value) { return Value.strName == Track.strName; }) ||
            Track.Values.Keys.empty() || Track.Values.iOperation != 1u ||
            Track.Values.iComponentCount != (Track.bVector ? 3u : 1u) || !CEffectDistribution::Validate(Track.Values, Error))
        { if (Error.empty()) Error = "Missing, duplicate or invalid native material parameter track: " + Track.strName; return false; }
        Candidate.push_back(*Parameter);
    }
    Bindings = std::move(Candidate);
    return true;
}

inline bool Apply_ArtistMaterialTrackSamples(const EFFECT_SOURCE_TRANSFORM_TRACK& Track,
    std::span<const ARTIST_PARAMETER_DESC> Bindings, const f32_t SampleTimeSeconds,
    std::array<float4_t, 32u>& Parameters)
{
    if (Bindings.size() != Track.MaterialParameterTracks.size()) return false;
    if (Bindings.empty()) return true;
    const f32_t SourceTime = SampleTimeSeconds + Track.fSourceTimeOriginSeconds;
    if (!std::isfinite(SourceTime)) return false;
    auto Candidate = Parameters;
    for (size_t i = 0u; i < Bindings.size(); ++i)
    {
        const auto& Binding = Bindings[i];
        const auto& Curve = Track.MaterialParameterTracks[i];
        if (Binding.iRow >= Candidate.size() || Binding.iLane >= 4u ||
            Binding.strName != Curve.strName || Binding.bVector != Curve.bVector) return false;
        const auto Value = CEffectDistribution::Evaluate(Curve.Values, SourceTime, 0.f);
        if (!std::isfinite(Value.x) || (Binding.bVector && (!std::isfinite(Value.y) || !std::isfinite(Value.z)))) return false;
        auto& Row = Candidate[Binding.iRow];
        if (Binding.bVector) { Row.x = Value.x; Row.y = Value.y; Row.z = Value.z; }
        else if (Binding.iLane == 0u) Row.x = Value.x;
        else if (Binding.iLane == 1u) Row.y = Value.x;
        else if (Binding.iLane == 2u) Row.z = Value.x;
        else Row.w = Value.x;
    }
    Parameters = Candidate;
    return true;
}

inline bool Has_ArtistMaterialContract(const EFFECT_ELEMENT_DESC& Element)
{
    const auto& Source=Element.Material.SourceMaterial;
    const auto* Program=Find_ArtistProgram(Source.strRuntimeShaderProfileId);
    if (!Program || Program->bModelCue || Element.Material.Execution.bEnabled || Element.Material.Execution.bFailClosed ||
        Element.Material.eRenderProfile!=Program->eRenderProfile ||
        Element.Material.strSourceMaterialPath!=Program->strSourceMaterialPath) return false;
    const bool bStaticAction = Program->strRendererShape == "staticMesh";
    if (Program->bSourceTransformMesh)
    {
        if (!Program->bMesh || Program->strRendererShape != "mesh" || Element.eKind != EFFECT_ELEMENT_KIND::MESH ||
            Element.SourceRecipe.bEnabled || !Element.SourceTransformTrack || Element.SourceTransformTrack->Nodes.empty() ||
            Element.ActionCueAttachment.bEnabled || Element.Detail.Mesh.bUseModelMaterial) return false;
    }
    else if (bStaticAction)
    {
        const auto& Attachment = Element.ActionCueAttachment;
        if (Element.eKind != EFFECT_ELEMENT_KIND::MESH || Element.SourceRecipe.bEnabled ||
            !Attachment.bEnabled || Attachment.bFollow ||
            Attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::BONE ||
            Attachment.strSourceAnchorSlotId != "EffectRoot" || Attachment.strRuntimeAnchorSlotId != "root" ||
            !Attachment.strRuntimeBoneName.empty() || Attachment.fSnapshotRootSourceBasisYawDegrees != -90.f ||
            Element.Detail.Mesh.bUseModelMaterial || Element.Detail.Mesh.fModelPreScale != .01f) return false;
        if (std::count_if(Element.ResourceBindings.begin(),Element.ResourceBindings.end(),[](const auto& B)
            { return B.strSlotId=="meshModel" && B.strAssetId=="Effect/Artist/Meshes/Native/LV_MATTE/sky_mirror_sm.wmodel"; }) != 1) return false;
    }
    else if (Program->strRuntimeProfileId.starts_with("effect.ue3.kouku-") &&
        Program->strRendererShape == "animationTrail")
    {
        if (Element.eKind != EFFECT_ELEMENT_KIND::TRAIL || Element.SourceRecipe.bEnabled ||
            Element.SourceRecipe.strRendererShape != "animationTrail" ||
            Element.RuntimeCarrier.eKind != EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::ANIMATION_TRAIL_BAKED_EDGE_V1)
            return false;
    }
    else if (Program->strRuntimeProfileId.starts_with("effect.ue3.kouku-") &&
        (Program->strRendererShape == "ribbon" || Program->strRendererShape == "beam"))
    {
        const auto eCarrier = Program->strRendererShape == "beam" ?
            EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_BEAM_V1 :
            EFFECT_AUTHORED_RUNTIME_CARRIER_KIND::CASCADE_RIBBON_V1;
        if (Element.eKind != EFFECT_ELEMENT_KIND::TRAIL || !Element.SourceRecipe.bEnabled ||
            Element.SourceRecipe.strRendererShape != Program->strRendererShape ||
            Element.RuntimeCarrier.eKind != eCarrier ||
            Element.RuntimeCarrier.eAdmission != EFFECT_AUTHORED_RUNTIME_CARRIER_ADMISSION::BOUNDED)
            return false;
    }
    else if (Program->strRuntimeProfileId.starts_with("effect.ue3.kouku-") &&
        Program->strRendererShape == "decal")
    {
        if (Element.eKind != EFFECT_ELEMENT_KIND::DECAL || !Element.SourceRecipe.bEnabled ||
            Element.SourceRecipe.strRendererShape != "decal") return false;
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
    return Build_ArtistParameters(Source,Parameters);
}
inline bool Has_ArtistModelCueMaterialContract(const EFFECT_MODEL_CUE_DESC& Cue)
{
    if (!Cue.Material) return false;
    const auto& Material=*Cue.Material;
    const auto& Source=Material.SourceMaterial;
    const auto* Program=Find_ArtistProgram(Source.strRuntimeShaderProfileId);
    if (!Program || !Program->bModelCue || Material.Execution.bEnabled || Material.Execution.bFailClosed ||
        Material.eRenderProfile!=Program->eRenderProfile || Material.strSourceMaterialPath!=Program->strSourceMaterialPath ||
        Source.Textures.size()!=Program->TextureNames.size() || Source.StaticSwitches.size()!=Program->StaticSwitches.size()) return false;
    const std::string_view expected=Program->iProfileIndex==460u ?
        "Effect/Artist/Models/SK_SDM_TIG_00/sk_sdm_tig_00_sk.wmodel" :
        "Effect/Artist/Models/SK_SDM_DRA_00/sk_sdm_dra_00_sk.wmodel";
    if (Cue.strModelAssetId!=expected) return false;
    for (const auto name: Program->TextureNames)
        if(std::count_if(Source.Textures.begin(),Source.Textures.end(),[&](const auto& T){return T.strName==name&&!T.strAssetId.empty()&&!T.strSourceObjectPath.empty();})!=1) return false;
    for (const auto& S:Program->StaticSwitches)
        if(std::count_if(Source.StaticSwitches.begin(),Source.StaticSwitches.end(),[&](const auto& V){return V.strName==S.strName&&V.bValue==S.bValue;})!=1) return false;
    std::array<float4_t,32> Parameters{};
    return Build_ArtistParameters(Source,Parameters);
}
NS_END
