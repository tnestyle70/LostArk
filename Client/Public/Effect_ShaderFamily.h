#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Client
{
    enum class EFFECT_SHADER_CARRIER : uint8_t { MESH, PARTICLE };
    enum class EFFECT_SHADER_FAMILY : uint8_t
    {
        GENERIC, DIMENSIONMASTER_Q, DIMENSIONMASTER_V, DIMENSIONMASTER_ALTV,
        DIMENSIONMASTER_WR, DIMENSIONMASTER_SD, WARLORD, ARTIST, LANCE_MASTER
    };
    struct EFFECT_SHADER_PROGRAM_DESC final
    {
        EFFECT_SHADER_CARRIER eCarrier;
        EFFECT_SHADER_FAMILY eFamily;
        uint32_t iFirstProfile;
        uint32_t iLastProfile;
        std::string_view strShaderAssetId;
        const wchar_t* pLogicalShaderPath;
    };
    // Runtime and compiled-shader closure validation consume these exact rows.
    // This is an executable selection table; no authored IDs use array indices.
#define EFFECT_SHADER_WIDEN_INNER(Value) L##Value
#define EFFECT_SHADER_WIDEN(Value) EFFECT_SHADER_WIDEN_INNER(Value)
#define EFFECT_SHADER_PROGRAM_ROW(Carrier, Family, First, Last, File) \
    { EFFECT_SHADER_CARRIER::Carrier, EFFECT_SHADER_FAMILY::Family, First, Last, File, EFFECT_SHADER_WIDEN(File) }
    inline constexpr std::array<EFFECT_SHADER_PROGRAM_DESC, 58u> EFFECT_SHADER_PROGRAMS = {{
        EFFECT_SHADER_PROGRAM_ROW(MESH, GENERIC, 0u, 4294967295u, "Shader_VtxEffectMeshPreview.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, DIMENSIONMASTER_Q, 0u, 4294967295u, "Shader_VtxEffectMeshDimensionMasterQ.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, DIMENSIONMASTER_V, 0u, 4294967295u, "Shader_VtxEffectMeshDimensionMasterV.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, DIMENSIONMASTER_WR, 0u, 4294967295u, "Shader_VtxEffectMeshDimensionMasterWR.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, DIMENSIONMASTER_SD, 0u, 4294967295u, "Shader_VtxEffectMeshDimensionMasterSD.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, WARLORD, 0u, 959u, "Shader_VtxEffectMeshWarlord.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, DIMENSIONMASTER_ALTV, 64u, 127u, "Shader_VtxEffectMeshDimensionMasterALTV064.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, DIMENSIONMASTER_ALTV, 128u, 191u, "Shader_VtxEffectMeshDimensionMasterALTV128.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, DIMENSIONMASTER_ALTV, 192u, 255u, "Shader_VtxEffectMeshDimensionMasterALTV192.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 448u, 511u, "Shader_VtxEffectMeshArtist448.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 512u, 575u, "Shader_VtxEffectMeshArtist512.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 768u, 831u, "Shader_VtxEffectMeshArtist768.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 832u, 895u, "Shader_VtxEffectMeshArtist832.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 512u, 575u, "Shader_VtxEffectMeshLanceMaster512.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 576u, 639u, "Shader_VtxEffectMeshLanceMaster576.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 640u, 703u, "Shader_VtxEffectMeshLanceMaster640.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 704u, 767u, "Shader_VtxEffectMeshLanceMaster704.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 768u, 831u, "Shader_VtxEffectMeshLanceMaster768.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, GENERIC, 0u, 4294967295u, "Shader_VtxEffectParticle.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, DIMENSIONMASTER_Q, 0u, 4294967295u, "Shader_VtxEffectParticleDimensionMasterQ.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, DIMENSIONMASTER_V, 0u, 4294967295u, "Shader_VtxEffectParticleDimensionMasterV.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, DIMENSIONMASTER_WR, 0u, 4294967295u, "Shader_VtxEffectParticleDimensionMasterWR.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, DIMENSIONMASTER_SD, 0u, 4294967295u, "Shader_VtxEffectParticleDimensionMasterSD.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, WARLORD, 0u, 959u, "Shader_VtxEffectParticleWarlord.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, DIMENSIONMASTER_ALTV, 64u, 127u, "Shader_VtxEffectParticleDimensionMasterALTV064.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, DIMENSIONMASTER_ALTV, 128u, 191u, "Shader_VtxEffectParticleDimensionMasterALTV128.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, DIMENSIONMASTER_ALTV, 192u, 255u, "Shader_VtxEffectParticleDimensionMasterALTV192.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 448u, 511u, "Shader_VtxEffectParticleArtist448.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 512u, 575u, "Shader_VtxEffectParticleArtist512.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 768u, 831u, "Shader_VtxEffectParticleArtist768.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 832u, 895u, "Shader_VtxEffectParticleArtist832.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 512u, 575u, "Shader_VtxEffectParticleLanceMaster512.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 576u, 639u, "Shader_VtxEffectParticleLanceMaster576.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 640u, 703u, "Shader_VtxEffectParticleLanceMaster640.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 704u, 767u, "Shader_VtxEffectParticleLanceMaster704.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 768u, 831u, "Shader_VtxEffectParticleLanceMaster768.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, WARLORD, 960u, 1023u, "Shader_VtxEffectMeshWarlord960.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, WARLORD, 1024u, 1087u, "Shader_VtxEffectMeshWarlord1024.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, WARLORD, 1088u, 1151u, "Shader_VtxEffectMeshWarlord1088.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, WARLORD, 1152u, 1215u, "Shader_VtxEffectMeshWarlord1152.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, WARLORD, 1984u, 2047u, "Shader_VtxEffectMeshWarlord1984.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, WARLORD, 960u, 1023u, "Shader_VtxEffectParticleWarlord960.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, WARLORD, 1024u, 1087u, "Shader_VtxEffectParticleWarlord1024.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, WARLORD, 1088u, 1151u, "Shader_VtxEffectParticleWarlord1088.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, WARLORD, 1152u, 1215u, "Shader_VtxEffectParticleWarlord1152.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, WARLORD, 1984u, 2047u, "Shader_VtxEffectParticleWarlord1984.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 1600u, 1663u, "Shader_VtxEffectMeshArtist1600.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 1664u, 1727u, "Shader_VtxEffectMeshArtist1664.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 1600u, 1663u, "Shader_VtxEffectParticleArtist1600.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 1664u, 1727u, "Shader_VtxEffectParticleArtist1664.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 1152u, 1215u, "Shader_VtxEffectMeshLanceMaster1152.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 1216u, 1279u, "Shader_VtxEffectMeshLanceMaster1216.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 1280u, 1343u, "Shader_VtxEffectMeshLanceMaster1280.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, LANCE_MASTER, 1344u, 1407u, "Shader_VtxEffectMeshLanceMaster1344.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 1152u, 1215u, "Shader_VtxEffectParticleLanceMaster1152.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 1216u, 1279u, "Shader_VtxEffectParticleLanceMaster1216.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 1280u, 1343u, "Shader_VtxEffectParticleLanceMaster1280.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, LANCE_MASTER, 1344u, 1407u, "Shader_VtxEffectParticleLanceMaster1344.hlsl"),
    }};
#undef EFFECT_SHADER_PROGRAM_ROW
#undef EFFECT_SHADER_WIDEN
#undef EFFECT_SHADER_WIDEN_INNER

    constexpr uint32_t Find_EffectShaderProgramIndex(const EFFECT_SHADER_CARRIER eCarrier,
        const EFFECT_SHADER_FAMILY eFamily, const uint32_t iAdmittedProfile)
    {
        for (uint32_t i = 0u; i < EFFECT_SHADER_PROGRAMS.size(); ++i)
        {
            const auto& Program = EFFECT_SHADER_PROGRAMS[i];
            if (Program.eCarrier == eCarrier && Program.eFamily == eFamily &&
                iAdmittedProfile >= Program.iFirstProfile && iAdmittedProfile <= Program.iLastProfile)
                return i;
        }
        return UINT32_MAX;
    }

    constexpr const EFFECT_SHADER_PROGRAM_DESC* Get_EffectShaderProgram(const uint32_t iIndex)
    {
        return iIndex < EFFECT_SHADER_PROGRAMS.size() ? &EFFECT_SHADER_PROGRAMS[iIndex] : nullptr;
    }
    static_assert([] {
        for (size_t i = 0u; i < EFFECT_SHADER_PROGRAMS.size(); ++i)
        {
            const auto& A = EFFECT_SHADER_PROGRAMS[i];
            if (A.strShaderAssetId.empty() || A.iFirstProfile > A.iLastProfile) return false;
            for (size_t j = i + 1u; j < EFFECT_SHADER_PROGRAMS.size(); ++j)
            {
                const auto& B = EFFECT_SHADER_PROGRAMS[j];
                if (A.strShaderAssetId == B.strShaderAssetId) return false;
                if (A.eCarrier == B.eCarrier && A.eFamily == B.eFamily &&
                    A.iFirstProfile <= B.iLastProfile && B.iFirstProfile <= A.iLastProfile) return false;
            }
        }
        return true;
    }(), "Effect shader executable ranges or basenames overlap.");
}
