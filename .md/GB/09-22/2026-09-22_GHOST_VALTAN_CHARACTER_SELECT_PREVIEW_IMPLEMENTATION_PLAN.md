# 유령 발탄 Character Select 미리보기 재질 경로 복구

## G01. 실제 미리보기 consumer와 목표

첨부 화면에서 유령 발탄의 머리와 상체는 매우 옅고 도끼와 손·발 일부가 진하게 보인다. 기존 [Valtan 편집 결과](2026-09-22_VALTAN_EDITOR_VISIBILITY_RESULT.md)의 수치 검증은 전투 `CBody_Valtan`의 native84/pass10 경로다. Character Select의 `boss.valtan.ghost`는 `pBossArchetypeId=nullptr`이므로 `CharacterPreviewPanel::Select_Asset`의 generic `CPart_Body` 경로를 사용한다.

현재 `CPart_Body::Resolve_TranslucentSourcePass`는 native6/7/18/99만 등록하고 native84를 누락했다. 그래서 ghost 3개 mesh가 NONBLEND/pass0 deferred geometry에 들어간다. 전투는 동일 재질을 BLEND/pass10으로 제출한다. 기존 모델·재질의 색과 alpha를 보존하면서 두 consumer가 같은 원본 translucent pass를 선택하도록 연결한다.

## G02. 변경 파일과 함수

`Client/Private/Part_Body.cpp`의 기존 anonymous namespace에 one-sided pass 상수 10을 둔다. `Resolve_TranslucentSourcePass`는 SOURCE_CHARACTER/native84에 10을 반환하고 기존 두 면 hair/eye 프로그램의 9 선택을 유지한다. 모델 이름이나 현재 Level로 분기하지 않는다.

호출 흐름은 `Initialize`의 translucent mesh 인식 → `Late_Update`의 BLEND 등록 → 기본 `Render_Pass(0)`의 중복 제외 → `Render_Translucent`의 scene light/base material/light material/bone binding → pass10 mesh 제출이다. 생성 rollback, weapon socket, donor clip, 본 자세 및 데이터 정본을 바꾸지 않는다. 기존 H 계약과 project/filter 등록을 그대로 사용한다.

## G03. 검증

변경 TU를 Debug 설정으로 컴파일한다. 설치된 실제 ghost WModel와 cinematic donor 및 현재 `scene.character-select.warm-high-key.v1`의 조명으로 기존 pass0과 pass10을 비교한다. 수치 출력은 각 mesh의 covered/colored/RGB/alpha와 nonfinite를 구분하며 직접 consumer 선택의 누락을 별도로 확인한다. 기존 hair/eye의 pass9와 일반 body의 pass0 경로를 확인한다.

Client/UI는 실행하거나 캡처하지 않는다. headless 수치 검증은 사용자의 실제 카메라·현재 실행 메모리·최종 화면 판정을 대신하지 않는다. 전체 제품 빌드와 설치는 통합 작업에서 다루며 RESULT에 실행한 범위를 적는다.

## G04. 09-23 사용자 요청: 유령 몸체 opaque 정책

원본 native84는 diffuse alpha와 cloud/rim으로 opacity를 만든다. 기존 pass10은 opacity 1e-4 이하를 버리고 SRC_ALPHA로 합성하며 depth를 쓰지 않는다. 원본 base에는 별도의 source[23].x로 활성화하는 luminance/opacity discard도 있다. 현재 catalog 상수23은0이어서 이 원본 discard는 비활성이며, pass0의 ordered coverage와 pass10의 blend를 구분한다.

사용자가 요청한 opaque 정책은 원본 translucent와 별도로 기록한다. animated shader에 pass16을 추가하여 native84의 RGB 계산을 유지하고 opacity discard와 blend를 사용하지 않으며 alpha1과 depth write를 사용한다. pass10, Sea native88, 머리카락과 맵 masked material은 보존한다. native84 함수는 기본 false인 선택 인자로 원본 discard를 보존하며 pass16만 우회한다.

CBody_Valtan과 generic CPart_Body가 ghost를 기존 NONLIGHT 그룹의 forward pass16으로 보낸다. 이 그룹은 deferred 합성 뒤, scene snapshot과 BLEND 효과 전에 SceneHDR에 그리므로 다른 불투명 표면의 깊이를 읽고 ghost 자신의 깊이를 쓰며 이후 반투명 효과의 가림 기준이 된다. normal body와 그 shadow는 기존 경로이며 pose afterimage는 계속 BLEND다. 유령 shadow만 같은 opaque silhouette를 쓰는 depth-only pass17로 보낸다. CPart_Body의 hair 등은 기존 pass9를 유지한다. 새 파일·프로젝트 등록은 없으며 기존 header와 CPP, Shader_VtxAnimMeshBinary.hlsl, Shader_SourceCharacterForward.hlsli, Engine/Client BaseGroup084 mirror 및 같은 선택 인자를 재생성하는 기존 build_vehicle_source_material.py의 native84/base 분기를 변경한다.

검증은 두 CPP compile, FX compile, 실제 설치 WModel와 catalog 상수·DDS를 쓰는 WARP draw/readback으로 진행한다. pass10/16의 RGB·alpha·coverage, 낮은 alpha와 source discard 조건, blend/depth state, depth 가림을 비교한다. Client 실행·화면 성공은 포함하지 않는다.

## G05. 유령 발탄 표시를 유지하는 컴파일 범위 축소

### G05-01. 목표와 종료 조건

사용자가 확인한 유령 발탄의 pass16 불투명 표시와 pass17 그림자 silhouette를 유지한다. 현재 ghost entry는 runtime g_SourceCharacterProgram 검사 뒤 shared evaluator를 호출하므로 fxc가 다른 native program을 제거하지 못했다. 대표 cohort001에는 불필요한 Base/Light 16case, cohort084에는 Light 29case가 남는다. 이번 변경은 ghost가 사용하는 프로그램을 컴파일 시점에84로 고정하고 animated source 범위에 코드를 둔다.

종료 조건은 대표 non84/84 FX 컴파일, 실제 배포 Engine DLL과 CSO를 기준으로 한 동일3mesh WARP pass16/17 비교, 기존 pass10 회귀 확인이다. RGBA 수치와 coverage/depth를 구분하고 Product O1과 후보 Od의 부동소수점 차이를 기록한다. 전체 Product 빌드와 Client 실행은 수행하지 않는다.

### G05-02. 파일과 HLSL 계약

Client/Bin/ShaderFiles/Shader_SourceCharacterForward.hlsli의 EvaluateSourceCharacterForward helper와 ghost entry를 제거한다. PS_MAIN_SOURCE_CHARACTER_TRANSLUCENT는 opaqueGhost가 추가되기 전 함수로 복원한다. 이 파일은 static/animated가 공유하므로 기존 translucent 입력·native dispatcher·opacity discard를 소유하고, animated ghost 정책은 소유하지 않는다. 복원 후보가 첫 ghost 변경 전 백업과 동일한지 확인하여 이전의 다른 변경을 보존한다.

Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl에서 shared Forward include 바로 다음에 ghost entry를 둔다. SOURCE_CHARACTER_PROGRAM_GROUP==84일 때만 실제 함수를 컴파일하고 base0 및 다른 cohort는 discard만 수행한다. base CShader는 기존 variant 계약으로 program84/pass16을84cohort에 전달한다. pass16/17의 이름·index·state, pass17의 BINARY_ANIMATED_NATIVE_PASS_POLICY(base1/shard2)는 변경하지 않는다. 새 H/CPP, enum, 멤버, include, JSON, project/filter 항목은 없다. 기존 UTF-8 no BOM/CRLF를 유지한다.

### G05-03. 함수의 변경 책임과 흐름

PS_MAIN_SOURCE_CHARACTER_TRANSLUCENT는 원본 투명 재질의 Base/Light dispatcher와 alpha discard만 수행하도록 돌아간다. 별도의 opaque 인자를 받던 helper를 호출하지 않는다. 아래는 복원할 함수 전체다.

```hlsl
SCENE_COLOR_BLOOM_OUT PS_MAIN_SOURCE_CHARACTER_TRANSLUCENT(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
    if (6u != g_SourceCharacterProgram && 7u != g_SourceCharacterProgram &&
        18u != g_SourceCharacterProgram && 84u != g_SourceCharacterProgram &&
        88u != g_SourceCharacterProgram && 99u != g_SourceCharacterProgram &&
        !(g_SourceCharacterProgram == 160u || g_SourceCharacterProgram == 166u || g_SourceCharacterProgram == 168u || g_SourceCharacterProgram == 169u || g_SourceCharacterProgram == 170u || g_SourceCharacterProgram == 171u || g_SourceCharacterProgram == 172u || g_SourceCharacterProgram == 174u || g_SourceCharacterProgram == 182u || g_SourceCharacterProgram == 187u || g_SourceCharacterProgram == 190u || g_SourceCharacterProgram == 192u || g_SourceCharacterProgram == 195u || g_SourceCharacterProgram == 212u || g_SourceCharacterProgram == 213u)) discard;
    const float3 camera = -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
    float3 ambient = 0.f;
    [loop] for (uint ambientIndex = 0u; ambientIndex < g_SourceMapForwardLightCount; ++ambientIndex)
    {
        float3 unusedDirection;
        const float attenuation = SourceCharacterForwardLightAttenuation(ambientIndex,
            input.vWorldPos.xyz, input.vNormal.xyz, unusedDirection);
        if (attenuation > 0.f)
            ambient += g_SourceMapForwardLightColorExponent[ambientIndex].rgb *
                g_SourceMapForwardLightAmbient[ambientIndex].rgb * attenuation;
    }
    // Base programs never read lightColor; program 88 takes the scene ambient
    // through it as its unbound engine sky-light rows.
    const SOURCE_CHARACTER_NATIVE_INPUT baseInput = MakeSourceCharacterInput(input.vTexcoord,
        input.vSourceExtraUV, input.vWorldPos.xyz, input.vTangent.xyz, input.vBinormal.xyz,
        input.vNormal.xyz, camera, input.vProjPos, mul(g_ViewMatrix, g_ProjMatrix),
        float3(0.f, 1.f, 0.f), ambient, 1.f, frontFace);
    const SOURCE_CHARACTER_NATIVE_OUTPUT base = EvaluateSourceCharacterBase(baseInput);
    const float opacity = saturate(base.targets[0].a);
    if (base.discarded || opacity <= 1e-4f) discard;

    float3 direct = 0.f;
    [loop] for (uint index = 0u; index < g_SourceMapForwardLightCount; ++index)
    {
        const float4 colorExponent = g_SourceMapForwardLightColorExponent[index];
        float3 direction;
        const float attenuation = SourceCharacterForwardLightAttenuation(index,
            input.vWorldPos.xyz, input.vNormal.xyz, direction);
        if (attenuation <= 0.f) continue;
        const SOURCE_CHARACTER_NATIVE_INPUT lightInput =
            MakeSourceCharacterForwardLightInput(input, camera, direction, colorExponent.rgb, frontFace);
        const SOURCE_CHARACTER_NATIVE_OUTPUT lit = EvaluateSourceCharacterLight(lightInput);
        if (!lit.discarded) direct += lit.targets[0].rgb * attenuation;
    }

    float3 color = base.targets[3].rgb * ambient + direct;
    const float4 fog = EvaluateSceneFog(input.vWorldPos.xyz, camera);
    color = color * fog.w + fog.rgb + base.targets[0].rgb;
    return Write_SceneColorAndBloom(float4(color, opacity));
}

```

PS_MAIN_SOURCE_CHARACTER_GHOST_OPAQUE는84cohort에 대해서만 현재 opaque 계산을 컴파일한다. runtime program이84가 아니면 discard한다. 같은 카메라·광원 배열로 ambient를 합산하고 기존 input adapter를 거쳐 SourceCharacterBase84(baseInput,true)를 호출한다. 반환된 native RGB는 유지하고 기존 source opacity만 우회한다. 각 forward light에 SourceCharacterLight84를 직접 호출한 뒤 기존 fog와 bloom 순서로 alpha1을 출력한다. EvaluateSourceCharacterLight의 case84는 이 함수의 직접 return이므로 dispatcher를 생략해도 해당 case의 부가 연산은 누락되지 않는다. 다른 cohort는 색과 깊이를 제출하지 않는 discard stub이다. 아래는 animated include 다음에 둘 함수 전체다.

```hlsl
SCENE_COLOR_BLOOM_OUT PS_MAIN_SOURCE_CHARACTER_GHOST_OPAQUE(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
#if SOURCE_CHARACTER_PROGRAM_GROUP == 84
    if (g_SourceCharacterProgram != 84u) discard;
    const float3 camera = -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
    float3 ambient = 0.f;
    [loop] for (uint ambientIndex = 0u; ambientIndex < g_SourceMapForwardLightCount; ++ambientIndex)
    {
        float3 unusedDirection;
        const float attenuation = SourceCharacterForwardLightAttenuation(ambientIndex,
            input.vWorldPos.xyz, input.vNormal.xyz, unusedDirection);
        if (attenuation > 0.f)
            ambient += g_SourceMapForwardLightColorExponent[ambientIndex].rgb *
                g_SourceMapForwardLightAmbient[ambientIndex].rgb * attenuation;
    }
    // Preserve the shipped ghost base input and ambient-light accumulation.
    const SOURCE_CHARACTER_NATIVE_INPUT baseInput = MakeSourceCharacterInput(input.vTexcoord,
        input.vSourceExtraUV, input.vWorldPos.xyz, input.vTangent.xyz, input.vBinormal.xyz,
        input.vNormal.xyz, camera, input.vProjPos, mul(g_ViewMatrix, g_ProjMatrix),
        float3(0.f, 1.f, 0.f), ambient, 1.f, frontFace);
    const SOURCE_CHARACTER_NATIVE_OUTPUT base = SourceCharacterBase84(baseInput, true);
    if (base.discarded) discard;

    float3 direct = 0.f;
    [loop] for (uint index = 0u; index < g_SourceMapForwardLightCount; ++index)
    {
        const float4 colorExponent = g_SourceMapForwardLightColorExponent[index];
        float3 direction;
        const float attenuation = SourceCharacterForwardLightAttenuation(index,
            input.vWorldPos.xyz, input.vNormal.xyz, direction);
        if (attenuation <= 0.f) continue;
        const SOURCE_CHARACTER_NATIVE_INPUT lightInput =
            MakeSourceCharacterForwardLightInput(input, camera, direction, colorExponent.rgb, frontFace);
        const SOURCE_CHARACTER_NATIVE_OUTPUT lit = SourceCharacterLight84(lightInput);
        if (!lit.discarded) direct += lit.targets[0].rgb * attenuation;
    }

    float3 color = base.targets[3].rgb * ambient + direct;
    const float4 fog = EvaluateSceneFog(input.vWorldPos.xyz, camera);
    color = color * fog.w + fog.rgb + base.targets[0].rgb;
    return Write_SceneColorAndBloom(float4(color, 1.f));
#else
    // Base FX dispatches program84 to its cohort. Other cohorts cannot draw it.
    discard;
    return (SCENE_COLOR_BLOOM_OUT)0;
#endif
}

```

### G05-04. 적용과 검증 순서

out/GhostCompileOptimize20260923의 후보와 검증 기록을 준비한 뒤 두 tracked 파일의 SHA256을 tracked-apply-guard.json과 다시 비교한다. 일치할 때만 같은 후보 bytes를 적용하고, 파일 drift가 있으면 기존 변경 위에 좁게 병합한다. shared Forward의 복원은 다음 실제 빌드에서 static 의존성을 한 번 무효화하지만, 이후 ghost 함수 편집이 static 공유 include를 다시 변경하는 경로를 제거한다.

대표 FX는 fxc /T fx_5_0 /Od로 cohort001과084를 각각 한 번 컴파일한다. ghost entry-only /T ps_5_0 /E PS_MAIN_SOURCE_CHARACTER_GHOST_OPAQUE /O1 결과는 실제 Product FX에 포함된 ghost PS와 token/switch 수를 비교한다. code size 감소를 전체 빌드 시간이나 프레임 성능 감소율로 해석하지 않는다.

기존 actual ghost probe를 out에서만 확장해 full RGBA32F/depth readback을 저장하고, 실제 Product base+14cohort admission을 통과한 뒤 pass16, opacity0의 base-owned pass17, 원본 pass10을 비교한다. 후보 runtime은 대표001/084만 새 FX로 교체하고 나머지는 Product 복사본을 사용한다. 새로운 base FX나 다른12개 cohort를 컴파일하지 않은 경계는 RESULT에 적는다. 적용 후 candidate SHA 일치, CRLF, 두 shader와 PLAN/RESULT의 git diff --check를 확인한다.

## G06. 09-26 최후 컷신도 부활 유령의 표시 경로 사용

### G06-01. 실제 consumer와 목표

`VALTAN_GHOST_RESPAWN_AUDITION`은 `mesh_respawn_1`과 phase3을 선택하며 CBody_Valtan이 native84를 NONLIGHT/pass16으로 그린다. 일반 preview의 CPart_Body도 같은 경로다. 반면 `world.sequence.instance.valtan.source-preview.finale`은 같은 `BOSS_VALTAN_GHOST` prototype에서 만든 CWorldSequenceObject를 사용하고, 이 객체의 native84 resolver만 BLEND/pass10에 남아 있다. 부활의 opaque 표시를 최후 컷신에 연결하는 변경이며 원본 translucent 식이나 색을 다시 조정하지 않는다.

### G06-02. H/CPP 변경과 수명

기존 WorldSequenceObject.h/cpp만 변경한다. `Resolve_TranslucentSourcePass`를 `Resolve_ForwardSourcePass`로 바꾸고 native84의 반환 pass를16으로 맞춘다. `Initialize`에서 opaque ghost와 나머지 translucent mesh를 따로 인식한다. `Late_Update`는 ghost를 기존 NONLIGHT 그룹에 등록하고 다른 forward mesh의 BLEND 등록을 유지한다. `Render_Group`에서 두 그룹을 같은 `Render_ForwardSource(bool opaqueGhost)`로 전달하며, 각 호출은 자신의 mesh만 material/light/bone 바인딩 후 제출한다. 기본 GBuffer draw는 모든 forward mesh를 계속 제외한다.

NONLIGHT의 성공이 다른 BLEND 실패를 지우지 않도록 두 render status를 구분한다. 컷신의 기존 visible/sampled world/clip/pool 수명과 material copy-on-write, 장비·소켓, 반사 transform 처리는 그대로 소비한다. body shader/pass는 부활과 동일하고 다른 프로그램의 Sea/hair/eye/static movie pass를 바꾸지 않는다. 새 파일·프로젝트 등록·shader 재컴파일 입력·authoring/publish 데이터 변경은 없다.

### G06-03. 종료 증거

현재 제품 build에서 WorldSequenceObject 헤더를 사용하는 TU를 증분 컴파일한다. native84의 program/pass/group 연결과 native88/18/equipment/static movie의 기존 분류, GBuffer와 BLEND 중복 배제, opaque/translucent 실패 상태 보존을 현재 코드로 확인한다. 기존 shader pass16의 blend/depth/native84 계약을 사용한다. Client/UI를 실행하지 않으며 사용자가 최후 컷신과 부활의 몸체 표시를 직접 비교한다.