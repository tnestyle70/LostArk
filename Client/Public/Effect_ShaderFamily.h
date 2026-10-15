#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Client
{
    // 이 표가 고르는 mesh/particle shader 입력 경로다. Effect 전체의 decal/trail/light 같은 모든 carrier 종류를 열거하는 enum은 아니다.
    enum class EFFECT_SHADER_CARRIER : uint8_t
    {
        MESH,
        PARTICLE
    };
    // ResourceStaging이 원본 profile registry 조회 결과를 이 실행 분류로 바꾼다.
    // 이름이 class별로 남아 있어도 class 한정 소유권은 아니다. 예를 들어 ARTIST 행에는 Kouku shader 구간도 들어 있다.
    enum class EFFECT_SHADER_FAMILY : uint8_t
    {
        GENERIC, DIMENSIONMASTER_Q, DIMENSIONMASTER_V, DIMENSIONMASTER_ALTV,
        DIMENSIONMASTER_WR, DIMENSIONMASTER_SD, WARLORD, ARTIST, LANCE_MASTER
    };
    // carrier, family, 승인된 숫자 profile의 닫힌 구간으로 실제 shader 파일을 선택한다.
    // 원본 material의 texture/CB 값이나 개별 opcode 실행 packet은 다른 descriptor가 소유한다.
    struct EFFECT_SHADER_PROGRAM_DESC final
    {
        EFFECT_SHADER_CARRIER eCarrier;
        EFFECT_SHADER_FAMILY eFamily;
        uint32_t iFirstProfile;
        uint32_t iLastProfile;
        // 두 이름은 같은 문자열 literal을 좁은/넓은 문자열로 보관한다. 테이블의 정적 수명 동안 유효하다.
        std::string_view strShaderAssetId;
        const wchar_t* pLogicalShaderPath;
    };
    // runtime 선택과 컴파일된 shader 연결 검사가 아래의 같은 행을 소비한다.
    // 실행 shader 선택 표이며 배열 index를 저작 stable ID로 저장하지 않는다.
#define EFFECT_SHADER_WIDEN_INNER(Value) L##Value
#define EFFECT_SHADER_WIDEN(Value) EFFECT_SHADER_WIDEN_INNER(Value)
#define EFFECT_SHADER_PROGRAM_ROW(Carrier, Family, First, Last, File) \
    { EFFECT_SHADER_CARRIER::Carrier, EFFECT_SHADER_FAMILY::Family, First, Last, File, EFFECT_SHADER_WIDEN(File) }
    // 아래 행의 순서와 범위는 실행/검증 계약이다. installer는 일부 Kouku 행을 이 표에 추가하므로 행 형식도 유지한다.
    // 기본 GENERIC은 해당 family의 모든 uint32 profile을 받지만, 다른 family의 누락을 자동으로 이 행에 fallback하지 않는다.
    inline constexpr std::array<EFFECT_SHADER_PROGRAM_DESC, 148u> EFFECT_SHADER_PROGRAMS = {{
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
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2304u, 2367u, "Shader_VtxEffectMeshKouku2304.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2368u, 2431u, "Shader_VtxEffectMeshKouku2368.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2432u, 2495u, "Shader_VtxEffectMeshKouku2432.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2496u, 2559u, "Shader_VtxEffectMeshKouku2496.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2560u, 2623u, "Shader_VtxEffectMeshKouku2560.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2624u, 2687u, "Shader_VtxEffectMeshKouku2624.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2688u, 2751u, "Shader_VtxEffectMeshKouku2688.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2752u, 2815u, "Shader_VtxEffectMeshKouku2752.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2816u, 2879u, "Shader_VtxEffectMeshKouku2816.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2880u, 2943u, "Shader_VtxEffectMeshKouku2880.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 2944u, 3007u, "Shader_VtxEffectMeshKouku2944.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3008u, 3071u, "Shader_VtxEffectMeshKouku3008.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3072u, 3135u, "Shader_VtxEffectMeshKouku3072.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3136u, 3199u, "Shader_VtxEffectMeshKouku3136.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3200u, 3263u, "Shader_VtxEffectMeshKouku3200.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3264u, 3327u, "Shader_VtxEffectMeshKouku3264.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3328u, 3391u, "Shader_VtxEffectMeshKouku3328.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3584u, 3647u, "Shader_VtxEffectMeshKouku3584.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3648u, 3711u, "Shader_VtxEffectMeshKouku3648.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3712u, 3775u, "Shader_VtxEffectMeshKouku3712.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3776u, 3839u, "Shader_VtxEffectMeshKouku3776.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3840u, 3903u, "Shader_VtxEffectMeshKouku3840.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3904u, 3967u, "Shader_VtxEffectMeshKouku3904.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 3968u, 4031u, "Shader_VtxEffectMeshKouku3968.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4032u, 4095u, "Shader_VtxEffectMeshKouku4032.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4096u, 4159u, "Shader_VtxEffectMeshKouku4096.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4160u, 4223u, "Shader_VtxEffectMeshKouku4160.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4224u, 4287u, "Shader_VtxEffectMeshKouku4224.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4288u, 4351u, "Shader_VtxEffectMeshKouku4288.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4352u, 4415u, "Shader_VtxEffectMeshKouku4352.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4416u, 4479u, "Shader_VtxEffectMeshKouku4416.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4480u, 4543u, "Shader_VtxEffectMeshKouku4480.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4544u, 4607u, "Shader_VtxEffectMeshKouku4544.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4608u, 4671u, "Shader_VtxEffectMeshKouku4608.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4672u, 4735u, "Shader_VtxEffectMeshKouku4672.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4736u, 4799u, "Shader_VtxEffectMeshKouku4736.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4800u, 4863u, "Shader_VtxEffectMeshKouku4800.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4864u, 4927u, "Shader_VtxEffectMeshKouku4864.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4928u, 4991u, "Shader_VtxEffectMeshKouku4928.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 4992u, 5055u, "Shader_VtxEffectMeshKouku4992.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 5056u, 5119u, "Shader_VtxEffectMeshKouku5056.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 5120u, 5183u, "Shader_VtxEffectMeshKouku5120.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 5184u, 5247u, "Shader_VtxEffectMeshKouku5184.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 5248u, 5311u, "Shader_VtxEffectMeshKouku5248.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(MESH, ARTIST, 5312u, 5375u, "Shader_VtxEffectMeshKouku5312.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2304u, 2367u, "Shader_VtxEffectParticleKouku2304.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2368u, 2431u, "Shader_VtxEffectParticleKouku2368.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2432u, 2495u, "Shader_VtxEffectParticleKouku2432.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2496u, 2559u, "Shader_VtxEffectParticleKouku2496.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2560u, 2623u, "Shader_VtxEffectParticleKouku2560.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2624u, 2687u, "Shader_VtxEffectParticleKouku2624.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2688u, 2751u, "Shader_VtxEffectParticleKouku2688.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2752u, 2815u, "Shader_VtxEffectParticleKouku2752.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2816u, 2879u, "Shader_VtxEffectParticleKouku2816.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2880u, 2943u, "Shader_VtxEffectParticleKouku2880.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 2944u, 3007u, "Shader_VtxEffectParticleKouku2944.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3008u, 3071u, "Shader_VtxEffectParticleKouku3008.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3072u, 3135u, "Shader_VtxEffectParticleKouku3072.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3136u, 3199u, "Shader_VtxEffectParticleKouku3136.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3200u, 3263u, "Shader_VtxEffectParticleKouku3200.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3264u, 3327u, "Shader_VtxEffectParticleKouku3264.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3328u, 3391u, "Shader_VtxEffectParticleKouku3328.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3584u, 3647u, "Shader_VtxEffectParticleKouku3584.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3648u, 3711u, "Shader_VtxEffectParticleKouku3648.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3712u, 3775u, "Shader_VtxEffectParticleKouku3712.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3776u, 3839u, "Shader_VtxEffectParticleKouku3776.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3840u, 3903u, "Shader_VtxEffectParticleKouku3840.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3904u, 3967u, "Shader_VtxEffectParticleKouku3904.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 3968u, 4031u, "Shader_VtxEffectParticleKouku3968.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4032u, 4095u, "Shader_VtxEffectParticleKouku4032.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4096u, 4159u, "Shader_VtxEffectParticleKouku4096.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4160u, 4223u, "Shader_VtxEffectParticleKouku4160.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4224u, 4287u, "Shader_VtxEffectParticleKouku4224.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4288u, 4351u, "Shader_VtxEffectParticleKouku4288.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4352u, 4415u, "Shader_VtxEffectParticleKouku4352.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4416u, 4479u, "Shader_VtxEffectParticleKouku4416.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4480u, 4543u, "Shader_VtxEffectParticleKouku4480.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4544u, 4607u, "Shader_VtxEffectParticleKouku4544.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4608u, 4671u, "Shader_VtxEffectParticleKouku4608.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4672u, 4735u, "Shader_VtxEffectParticleKouku4672.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4736u, 4799u, "Shader_VtxEffectParticleKouku4736.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4800u, 4863u, "Shader_VtxEffectParticleKouku4800.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4864u, 4927u, "Shader_VtxEffectParticleKouku4864.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4928u, 4991u, "Shader_VtxEffectParticleKouku4928.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 4992u, 5055u, "Shader_VtxEffectParticleKouku4992.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 5056u, 5119u, "Shader_VtxEffectParticleKouku5056.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 5120u, 5183u, "Shader_VtxEffectParticleKouku5120.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 5184u, 5247u, "Shader_VtxEffectParticleKouku5184.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 5248u, 5311u, "Shader_VtxEffectParticleKouku5248.hlsl"),
        EFFECT_SHADER_PROGRAM_ROW(PARTICLE, ARTIST, 5312u, 5375u, "Shader_VtxEffectParticleKouku5312.hlsl"),
    }};
#undef EFFECT_SHADER_PROGRAM_ROW
#undef EFFECT_SHADER_WIDEN
#undef EFFECT_SHADER_WIDEN_INNER

    // ResourceStaging이 사용하는 선형 검색이다. 양 끝을 포함한 구간과 carrier/family가 모두 맞는 첫 행의 index를 반환한다.
    // 실패는 UINT32_MAX이며 호출자가 준비를 거절한다. 반환 index는 이 실행 테이블 내부용이다.
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

    // Renderer가 index를 정적 descriptor 포인터로 바꾼다. 범위 밖이면 nullptr이고 별도의 GPU 자원을 생성하지 않는다.
    constexpr const EFFECT_SHADER_PROGRAM_DESC* Get_EffectShaderProgram(const uint32_t iIndex)
    {
        return iIndex < EFFECT_SHADER_PROGRAMS.size() ? &EFFECT_SHADER_PROGRAMS[iIndex] : nullptr;
    }
    // 빌드 시 빈 파일명/역전 구간/파일명 중복과 같은 carrier-family의 구간 겹침을 거부한다.
    // 파일 존재, 실제 shader 컴파일, texture 바인딩과 최종 화면은 이 static_assert의 검증 범위가 아니다.
    static_assert([] {
        for (size_t i = 0u; i < EFFECT_SHADER_PROGRAMS.size(); ++i)
        {
            const auto& A = EFFECT_SHADER_PROGRAMS[i];
            if (A.strShaderAssetId.empty() || A.iFirstProfile > A.iLastProfile)
                return false;
            for (size_t j = i + 1u; j < EFFECT_SHADER_PROGRAMS.size(); ++j)
            {
                const auto& B = EFFECT_SHADER_PROGRAMS[j];
                if (A.strShaderAssetId == B.strShaderAssetId)
                    return false;
                if (A.eCarrier == B.eCarrier && A.eFamily == B.eFamily &&
                    A.iFirstProfile <= B.iLastProfile && B.iFirstProfile <= A.iLastProfile)
                    return false;
            }
        }
        return true;
    }(), "Effect shader executable ranges or basenames overlap.");
}
