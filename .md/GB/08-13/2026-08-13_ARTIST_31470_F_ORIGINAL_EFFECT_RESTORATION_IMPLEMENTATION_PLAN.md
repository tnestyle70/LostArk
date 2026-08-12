# 2026-08-13 Artist 31470 F 원작 수치·실행 의미 복원 구현 계획

기준일: 2026-08-13 KST

현재 branch: `codex/artist-f-complete-visual-runtime`

현재 기준 HEAD: `8d76693e03f4c4c0c3939c7b71b9e3de8680441a`

Product: `false`

전체 occurrence 승인: `0/35`

## 문서 역할

이 문서는 2026-08-13 이후 Artist 31470 F 복원을 실제로 진행할 **현재 구현 순서와 admission gate의
정본**이다. 기술 사실과 장기 원칙은 [이펙트복원문서.md](../이펙트복원문서.md)를 정본으로 사용하고,
실제 완료 증거는 대응 RESULT와 canonical receipt에서만 판정한다.

| 역할 | 정본 |
|---|---|
| 기술 사실, 이전 실패, 이번 성공 원인, 장기 canary | `../이펙트복원문서.md` |
| 2026-08-13 이후 구현 순서와 종료 조건 | 이 문서 |
| main ShaderMap/DXBC/runtime 실제 결과 | `../08-10/2026-08-13_ARTIST_31470_F_TRACK_A_MAIN_SHADERMAP_DXBC_RUNTIME_RESULT.md` |
| occurrence 실행 장부 | `../이펙트최종추출.md` |
| 과거 전체 계획 | `../08-10/2026-08-10_ARTIST_31470_F_COMPLETE_RESTORATION_IMPLEMENTATION_PLAN.md` — LEGACY, 새 구현 상태를 판정하지 않음 |

기존 대형 계획서의 historical C/R/G 절은 당시 시행착오와 denominator를 보존하는 자료다. 앞으로
상충하는 순서나 상태가 있으면 이 문서와 최신 RESULT를 우선한다. 기존 dirty worktree를 reset하거나
다른 팀원의 변경을 stage하지 않는다.

---

## 0. 최종 운영 결정

결론은 다음과 같다.

1. 남은 Artist F는 하나의 원작 복원 캠페인으로 계속 진행한다.
2. RefShaderCache/ShaderMap/DXBC의 **증거 파이프라인**은 모든 renderer family에 재사용한다.
3. main `#9/#10/#11` HLSL이나 LocalVF 가정을 다른 family에 그대로 복사하지 않는다.
4. cache·MIC·VF/pass·transform·default/RNG의 read-only 증거 수집은 병렬로 진행할 수 있다.
5. `Effect_Playback`, `Effect_DocumentRenderer`, 공용 HLSL, Program→Catalog mutation은 한 writer가
   family별로 직렬 반영한다.
6. 축 문제는 다른 family의 증거 조사와 함께 진행하되 하나의 전역 회전/offset 보정을 전 family에
   적용하지 않는다. 공용 UE→Client basis 계약을 Mesh local, Sprite billboard, Decal projector,
   Ribbon tangent/facing adapter가 각각 소비한다.
7. `#32 ScreenPost`와 `#34 PointLight`는 뒤로 미룬다. `#33`은 distortion 성격이 있어도
   renderer denominator상 SpriteParticle이므로 core 범위에 포함한다.
8. 먼저 `MeshParticle 13 + SpriteParticle 16 + DecalParticle 3 + CascadeRibbon 1 = 33`을 닫고
   milestone을 정확히 `Artist F core renderers 33/35`라고 부른다.
9. Light/Post를 닫기 전에는 `35/35`, `완료`, Product publish를 주장하지 않는다.
10. Map Shader는 같은 증거 원칙을 쓰지만 별도 corpus와 global/deferred pass가 필요하므로 이 계획에
    합치지 않는다.

---

## 1. 현재 기준선

### 1.1 정확한 denominator

| renderer family | occurrence |
|---|---:|
| MeshParticle | 13 |
| SpriteParticle | 16 |
| DecalParticle | 3 |
| CascadeRibbon | 1 |
| PointLight | 1 |
| ScreenPost | 1 |
| 합계 | 35 |

Artist F에는 standalone Mesh occurrence가 **0개**다. 현재 mesh carrier 13개는 모두 MeshParticle이다.

- MeshParticle: `#4/#7/#8/#9/#10/#11/#12/#13/#14/#15/#17/#18/#26`
- SpriteParticle: `#0/#1/#2/#5/#6/#16/#19/#23/#24/#25/#27/#28/#29/#30/#31/#33`
- CascadeRibbon: `#3`
- DecalParticle: `#20/#21/#22`
- ScreenPost: `#32`
- PointLight: `#34`

### 1.2 이번 성공의 정확한 범위

현재 완료된 자동 증거는 official `45_975` same-distribution cohort의 RefShaderCache에서 active MIC
`FStaticParameterSet`를 `FMaterialShaderMap`에 exact structural join하고, main LocalVF BasePass
후보 PS의 원본 DXBC를 WARP에서 replay한 뒤 shipped mesh shader의 RT0 식과 parity를 닫은 것이다.

| gate | 현재 결과 |
|---|---:|
| active MIC → ShaderMap join | 2/2 |
| candidate PS / related VS | PS 2 / VS 4 |
| native texture binding | 7 |
| original PS DXBC WARP | 21/21 |
| runtime constant/spatial parity | 19/19, 15/15 |
| focused runtime recipe/occurrence/CPU opcode | 9/9, 10/10, 20/20 |
| production-path Debug/Release first draw | `failures:0` |
| 사용자 판정 | `#9/#10/#11 RT0 material composition USER MATERIAL PASS` |

이 성공은 `SOURCE_DXBC_CANDIDATE_REPLAY`다. occurrence 전체 `SOURCE_EXACT`가 아니다.
`localVertexFactorySelectionAdmission=false`이므로 source TypeDataMesh가 실제로 같은 VF/pass를
선택했다는 native admission, 복구 VS의 실제 runtime 사용, transform, lifetime/RNG, pass 전체는 열려 있다.

### 1.3 원시 가시화 단계에 대한 판정

main 3의 material 계산식은 보라 billboard나 임의 파란 판을 띄우던 단계에서 벗어났다. 그러나 전체
35가 그 단계에 도달한 것은 아니다. 새 Sprite의 거대 quad, Decal의 파란 판, Ribbon의 흰 strip은
다시 나타날 수 있다. 앞으로는 이것을 main material 성공의 취소가 아니라 다음처럼 family별 failure
signature로 분류한다.

- Sprite 큰 판: texture/default alpha, SubUV crop, billboard basis, size 단위
- Decal 파란 판: projector volume, depth/near/far, decal pass/MRT
- Ribbon 흰 strip: point history, topology, tangent/facing, UV와 material binding

---

## 2. family별 복원 성격, 위험, 실패 양상

비교축은 material 식, 형상 생성, 공간/축, 시간/RNG, render pass다.

| family | 복원의 중심 | 현재 위험 | 실패하면 보이는 형태 |
|---|---|---:|---|
| main MeshParticle `#9/#10/#11` | RT0 material은 크게 de-risk. actual MeshParticle→VF/pass, root/basis/pivot, lifetime/RNG, 외부 opacity와 non-RT0가 남음 | 중간 | 색·dissolve는 맞지만 원호가 반대/아래/과대·과소, one-sided cull 소실, 정지 프레임만 일치, offline PASS인데 actual draw 0 |
| remaining MeshParticle 10 | CModel/LocalVF carrier 재사용성이 가장 높지만 recipe마다 ShaderMap/DXBC, TypeDataMesh, material slot, dynamic lane, signed scale/cull을 다시 닫음 | 중간 | 검정/흰 wedge, fan layer 누락, scale `1/100` 또는 과대, winding/cull 반전, dissolve tail 잔류 |
| standalone Mesh | Artist F 밖의 후속 corpus. particle spawn/lifetime/RNG가 없지만 element transform owner, model material slot과 pass 계약이 별도 | 중간 이하~중간 | 잘못된 transform/material slot의 한 장만 표시, 중복 static object, particle dynamic을 기다려 neutral/zero로 고정 |
| SpriteParticle | Particle/SubUV VF, camera/velocity/axis billboard, signed size/flip, atlas cadence/RNG와 sort/blend/depth가 material만큼 중요 | 중간~높음 | 보라/흰 거대 quad, atlas 전체 노출, frame jump, edge-on/반대-facing, mirror 손실, size/rotation 폭주, additive 과노출 |
| DecalParticle | textured quad가 아니라 projector volume, decal pass, scene depth, near/far/depth, blend/MRT를 함께 복구 | 높음 | 파란/흰 바닥판, footprint/depth 이중 적용, camera-facing/세로/공중 decal, 뒤 surface bleed, z-fighting, depth clip으로 완전 소실 |
| CascadeRibbon | material보다 BeamTrail VF, historical point, strip topology, tangent/facing, width/tessellation, age/distance UV가 큼 | 높음 | zero draw, degenerate/끊긴 strip, spike/NaN, 꼬임/camera flip, 폭·길이 오류, UV stretch/slide, tail pop, capacity 실패 |

현재의 일반 위험 순서는
`remaining MeshParticle < SpriteParticle < DecalParticle ~= CascadeRibbon`이다. 다만 target ShaderMap이나
embedded shader code가 official cache에 없으면 exact track이 즉시 BLOCK되므로 G01 coverage 결과에 따라
순서는 바뀔 수 있다.

### 2.1 standalone Mesh를 이번 denominator에 넣지 않는 이유

MeshParticle과 standalone Mesh는 mesh geometry와 LocalVF를 공유할 수 있지만 실행 owner가 다르다.
MeshParticle은 emitter spawn, lifetime, localSpace, StartSize, MeshRotation과 particle dynamic을 공급한다.
standalone Mesh는 element transform과 model material slot이 직접 owner다. 현재 Artist F에는 비교할
standalone source occurrence가 없으므로 Track B의 authored approximation을 exact canary로 승격하지 않는다.
Artist F core 33을 닫은 뒤 실제 standalone source corpus가 있는 다음 스킬에서 별도 vertical slice를 연다.

---

## 3. 공통으로 재사용할 것과 family별로 다시 증명할 것

### 3.1 공통 evidence chain

모든 rendered occurrence는 다음 단계를 같은 receipt 형식으로 통과한다.

```text
source package / MIC / Material identity
-> active FStaticParameterSet
-> official cohort RefShaderCache의 FMaterialShaderMap join
-> occurrence-selected VF / pass / shader object admission
-> DXBC와 uniform/register/texture/channel closure
-> raw DXBC WARP replay
-> runtime HLSL 또는 native shader binding parity
-> actual first draw와 state/rollback
-> 사용자 fixed-time occurrence 판정
```

재사용 대상은 parser, identity/equality, receipt schema, WARP replay, runtime parity와 transaction gate다.
main Watertrail/Spritewave의 HLSL 식이나 LocalVF 선택을 Sprite/Decal/Ribbon에 복사하지 않는다.

### 3.2 반드시 family별로 닫을 것

| family | 별도 admission |
|---|---|
| MeshParticle | TypeDataMesh → selected mesh VF/pass, local basis, mesh slot, particle dynamic |
| SpriteParticle | sprite/SubUV VF, screen alignment/axis lock, signed flip, atlas frame policy |
| DecalParticle | projector basis/volume, scene depth, decal pass, MRT/depth state |
| CascadeRibbon | trail VF, point→vertex/index topology, tangent/facing, width, UV policy |
| PointLight | source light field → CPU LightManager consumer |
| ScreenPost | scene texture/global shader, viewport, render order와 post state |

MaterialShaderMap 안에 후보 permutation이 존재한다는 사실만으로 occurrence가 그것을 선택했다고
판정하지 않는다. target map/code가 없으면 비슷한 shader를 빌리지 않고 해당 행을
`CACHE_MISSING_BLOCK` 또는 명시적 `RECONSTRUCTED`로 남긴다.

---

## 4. 실제 코드와 데이터의 책임 경계

| 계층 | 현재 파일 | 이 계획에서의 책임 |
|---|---|---|
| source/evidence | `Tools/LevelPlacementExtractor/*.py`, `Data/Effects/Imported/Artist/**` | raw identity, cache join, DXBC/resource/transform/temporal receipt 생성 |
| Program→Document | `Client/Private/Effect_ReconstructedExecution.cpp` | 검증된 Program row를 typed element/material profile로 projection |
| 시간·spawn·world | `Client/Private/Effect_Playback.cpp`, `Client/Public/Effect_Playback.h` | fixed-step, root snapshot, local/world 합성, particle age/RNG, family geometry 입력 |
| resource stage | `Client/Private/Effect_DocumentRenderer.cpp`, `Client/Public/Effect_DocumentRenderer.h` | material/mesh/texture/state를 전부 검증한 prepared resource 생성과 rollback |
| family shader | `Shader_VtxEffectMeshPreview.hlsl`, `Shader_VtxEffectParticle.hlsl`, `Shader_VtxEffectDecal.hlsl`, `Shader_VtxEffectTrail.hlsl`, `Shader_EffectCommon.hlsli` | renderer별 VS input과 PS output, source register/texture/state 소비 |
| geometry carrier | `Engine/Private/VIBuffer_ParticleRect.cpp`, `Engine/Private/VIBuffer_DynamicTrail.cpp`, CModel 경로 | billboard quad, trail buffer, mesh geometry 제출 |
| actual consumer | `Client/Private/Effect_Object.cpp`, ObjectManager/Renderer | prepared effect의 attempted/submitted/failed/committed first draw |
| automatic gate | `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`, `Tools/ProjectAudit` | numeric oracle, mutation reject, WARP draw, Debug/Release 회귀 |
| manual gate | 사용자 Effect Tool/Client 화면 | 고정 sample time의 visual fidelity와 occurrence 승인 |

호출 흐름은 다음 하나를 유지한다.

```text
validated Program/Catalog
-> CEffectReconstructedSourceRuntimeFactory::Build_Document
-> CEffectPlayback::Stage_ReconstructedSourceRuntime
-> fixed-step Seek/update와 evaluated frame
-> CEffectDocumentRenderer의 prepared resource
-> EffectObject/ObjectManager/Renderer
-> family shader/geometry draw
```

새 family를 위해 두 번째 Effect runtime을 만들지 않는다. 모든 stage는
`parse -> validate -> stage -> commit`을 따르고 identity, register, resource, draw-state 중 하나라도
실패하면 이전 document/composite를 보존한다. `S_FALSE`, zero draw, suppressed-only를 성공으로 세지 않는다.

---

## 5. 병렬 조사와 직렬 구현 규칙

다음 read-only evidence는 서로 다른 receipt를 쓰므로 동시에 진행할 수 있다.

- G01 전체 cache/VF/pass coverage matrix
- G02 main root/basis/pivot oracle의 raw 입력 조사
- G03 delay/duration/default/RNG/clock source 조사
- 각 family의 raw TypeData, material identity와 geometry input inventory

다음 mutation은 공유 파일을 사용하므로 반드시 직렬이다.

```text
main transform
-> main temporal
-> actual selected VF/pass와 full output state
-> remaining MeshParticle
-> SpriteParticle
-> DecalParticle
-> CascadeRibbon
-> core 33/35 freeze
```

main `#9/#10/#11`의 합격한 RT0 material HLSL과 canonical receipt를 먼저 golden fixture로 동결한다.
후속 family 구현이 main 21/19/15 replay와 Debug/Release first draw를 깨면 해당 family commit을 거부한다.

---

## G00. 2026-08-13 checkpoint와 denominator 동결

### 목표

Track A 결과와 사용자 `USER MATERIAL PASS`를 새 계획의 시작점으로 고정하고, 서로 다른 완료 분모를
섞지 않는다.

### 구현 범위

- technical SSOT, 현재 PLAN, RESULT, occurrence ledger의 역할을 위 표대로 연결한다.
- 35 renderer row와 34 rendered material occurrence를 분리한다.
- main 3 material sub-check, occurrence `0/35`, Product `false`를 별도 필드로 유지한다.
- 과거 pre-cache `join=0` receipt와 현재 RefShaderCache `join=2/2` receipt를 시간 순서로 보존한다.
- old COMPLETE plan은 LEGACY로만 참조한다.

### 종료 조건

- 문서와 receipt가 `Mesh 13 / Sprite 16 / Decal 3 / Ribbon 1 / Light 1 / Post 1`에 합의한다.
- `#26`이 remaining MeshParticle 10에서 빠지지 않는다.
- `#33`은 core Sprite, `#32/#34`만 deferred로 분류한다.
- 완료되지 않은 occurrence나 Product를 PASS로 바꾸지 않는다.

---

## G01. 전체 occurrence cache coverage와 renderer-selection matrix

### 목표

고점을 더 올릴 수 있는지 가장 빨리 판정하기 위해 27 material recipe/34 rendered occurrence의 exact
cache coverage, candidate VF/pass와 embedded code 유무를 전수 조사한다.

### 추가·변경 대상

- 새 tool: `Tools/LevelPlacementExtractor/build_artist_31470_renderer_restoration_matrix.py`
- 새 test: `Tools/LevelPlacementExtractor/test_build_artist_31470_renderer_restoration_matrix.py`
- 새 receipt: `Data/Effects/Imported/Artist/Materials/skill.31470.renderer-restoration-matrix.receipt.json`
- focused 검증은 generator의 `--validate-only`와 독립 Python unit test로 실행한다.
- 기존 RefShaderCache/ShaderMap parser와 main golden fixture는 재사용한다.

새 Python 파일은 기존 project에 등록하지 않는다. 다만 새 Git 관리 `Data` receipt는
`Sync-EffectDataProject.ps1`로 Client `.vcxproj/.filters`의 `96.DataFiles` 아래에 등록한다.
전역 ProjectAudit aggregate/report에는 이 deep cache scan을 연결하지 않는다.

### 행별 필수 필드

- stable occurrence ID와 renderer family
- source package/MIC/Material SHA와 BaseMaterialId
- active `FStaticParameterSet` semantic identity
- exact ShaderMap count와 cache cohort identity
- candidate VF/pass/shader type/shader object ID/DXBC SHA
- embedded code 존재 여부
- uniform, constant, texture/sampler register closure
- source TypeData/native selection evidence
- `JOINED_CANDIDATE`, `SELECTED_ADMITTED`, `CACHE_MISSING_BLOCK`,
  `CODE_MISSING_BLOCK`, `RECONSTRUCTED_ONLY` 중 하나의 disposition

### 검증

- 35 occurrence, rendered material 34, recipe 27의 누락·중복이 0이어야 한다.
- wrong BaseMaterialId, static switch/mask, cache SHA, VF/pass, shader ID mutation을 각각 거부한다.
- 한 ShaderMap의 여러 VF/pass 후보를 자동으로 selected로 승격하지 않는다.
- output JSON은 canonical serialize/self-hash를 통과하고 재실행 결과가 byte-identical이어야 한다.

### 종료 조건

모든 rendered occurrence에 exact candidate 또는 명시적 hard blocker가 존재한다. 이 matrix가 없는 상태에서
family HLSL 구현을 먼저 시작하지 않는다. cache/code가 없는 행은 즉시 BLOCK로 표시해 사용자 시간이
근사 shader 추적에 소모되지 않게 한다.

---

## G02. main root projection·3축 rotation·mesh pivot canary

### 목표

`#9/#10/#11` material을 바꾸지 않고 원호 위치·방향·크기의 owner를 source 수치로 봉인한다.

### source 입력

- notify `1.380396962 s`
- `SNAPSHOT_ROOT`, bone/socket 없음, `follow=false`
- source cue `[100,-100,0] cm` → Client `[1,0,1] m`
- cue scale `3`, rotation `0`, `localSpace=true`
- source StartSize/MeshRotation
- raw UPK, glTF와 WModel bounds/origin/pivot
- playable model presentation yaw와 captured root

### 추가·변경 대상

- oracle tool/test/receipt:
  `build_artist_31470_main_transform_oracle.py`,
  `test_build_artist_31470_main_transform_oracle.py`,
  `skill.31470.main-transform-oracle.receipt.json`
- runtime delta: `Client/Private/Effect_Playback.cpp`
- 필요한 경우 typed diagnostic만 `Client/Public/Effect_Playback.h`에 추가
- numeric harness: `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- focused 검증은 oracle의 `--validate-only`/`--check`, 독립 Python unit test와
  `ClientFrontendHarness` numeric canary로 실행한다. 전역 ProjectAudit wrapper는 만들지 않는다.

실제 H/CPP 선언은 구현 직전 G02 DETAIL PLAN에서 현재 전체 코드를 기준으로 확정한다. public header를
변경하면 Engine/Client consumer와 Debug/Release 전체를 함께 검증한다.

### canary

각 `#9/#10/#11`에 다음 값을 같은 fixed fixture로 기록한다.

```text
source local point/basis
-> UE basis-conjugated Client local
-> captured root
-> cue local/world
-> playable presentation yaw
-> final world matrix/bounds
-> fixed view-projection clip/NDC point
```

UPK→glTF→WModel의 min/max/center와 pivot delta, geometry pre-scale, StartSize, cue scale가 각각 정확히 한 번
소비되었는지 검사한다.

### reject fixture

- X/Y/Z 중 하나를 잘못 swizzle
- Euler component만 치환하고 basis conjugation 누락
- cm→m 또는 geometry pre-scale 이중 적용
- cue scale과 StartSize 이중 적용
- `SNAPSHOT_ROOT`를 live-follow로 mutation
- 임의 hand/weapon attachment, Y offset 또는 scale 보정
- raw pivot/bounds와 불일치

### 종료 조건

- source→Client 각 matrix/point/bounds가 finite tolerance 안에서 일치한다.
- `#9/#10/#11`의 transform owner가 하나씩만 존재한다.
- material 21/19/15 golden replay가 그대로 PASS한다.
- 자동 gate 뒤 사용자가 동일 sample time에서 anchor/orientation을 판정한다.
- source edge가 없을 때만 occurrence-local correction을 `MANUAL_TUNED`로 별도 제안한다.

---

## G03. main lifetime·clock·default·RNG temporal canary

### 목표

고정 프레임 하나가 아니라 `#9/#10/#11`의 전체 lifetime에서 source lookup, CurrentTime, spawn/default와
RNG 소비를 결정론적으로 재생한다.

### 추가·변경 대상

- oracle tool/test/receipt:
  `build_artist_31470_main_temporal_oracle.py`,
  `test_build_artist_31470_main_temporal_oracle.py`,
  `skill.31470.main-temporal-oracle.receipt.json`
- runtime delta: `Client/Private/Effect_Playback.cpp`,
  필요 시 `Client/Private/Effect_ReconstructedExecution.cpp`
- numeric harness: `ClientFrontendHarness.cpp`
- focused 검증은 oracle의 `--validate-only`/`--check`, 독립 Python unit test와
  `ClientFrontendHarness` temporal canary로 실행한다. 전역 ProjectAudit wrapper는 만들지 않는다.

### canary

- 60 Hz fixed step의 `0/25/50/75/100%` lifetime sample
- `#9/#10` lifetime `0.5 s`, `#11` lifetime `0.8 s`
- alpha, size, rotation-rate, dynamic parameter, material CurrentTime 입력
- emitter delay/duration CDO/current default의 provenance
- straight play와 direct seek의 같은 tick 결과
- 같은 seed replay의 byte-identical 결과
- late join/seek 후 clock origin 일치
- SubUV가 없는 main에서도 RNG 소비 순서가 후속 emitter seed를 바꾸지 않는지 검사

모든 값이 0→1 선형이라는 가정을 만들지 않는다. normalized age는 lookup 입력축이며 출력은
constant/linear/Hermite와 cooked sample 의미를 보존한다.

### reject fixture

- source/current default 무표시 혼합
- local clock origin을 material CurrentTime과 동일하다고 가정
- frame-rate dependent update
- seek와 straight play의 RNG 소비 차이
- seed 또는 emitter 순서 mutation을 조용히 허용
- lifetime 끝에서 alpha/size/dissolve tail 잔류

### 종료 조건

five-sample numeric oracle, seek/replay determinism과 fixed-step가 모두 PASS한다. source-era default를
찾지 못한 필드는 `CURRENT_REVISION_EVIDENCE` 또는 `RECONSTRUCTED`로 분리하며 exact 통계에 넣지 않는다.
자동 gate 뒤 사용자가 같은 다섯 시점에서 main temporal을 판정한다.

---

## G04. actual MeshParticle VF/pass와 RT0 밖의 output/state admission

### 목표

ShaderMap membership을 실제 occurrence-selected renderer 실행으로 연결하고, main material 성공 범위를
필요한 VS/input, opacity, sampler, pass와 MRT까지 확장한다.

### 추가·변경 대상

- G01 matrix와 기존 RefShaderCache extraction/replay tool
- TypeDataMesh/native selection receipt 또는 actual draw shader-selection instrumentation
- `Effect_DocumentRenderer.cpp/.h`
- `Shader_VtxEffectMeshPreview.hlsl`과 source material HLSL include
- 필요 시 Engine renderer의 read-only shader/state diagnostic
- original DXBC WARP fixture와 production first-draw harness

### 구현 범위

- source TypeDataMesh → actual selected VF/pass/shader object를 증명한다.
- current runtime VS input semantic과 복구 VS의 경계를 기록한다.
- original external opacity `c0.x`의 owner를 찾고 임의 unity로 고정하지 않는다.
- source-exact sampler가 확인될 때만 POINT_CLAMP 실험 조건을 제품 정책으로 교체한다.
- blend/depth/cull, fog/pass carrier를 original object/state에 연결한다.
- Water의 `o2` normal과 필요한 `o2..o5` MRT를 1-RTV/6-RTV fixture로 분리한다.
- attempted/submitted/suppressed/failed/committed와 실제 bound shader ID를 같은 draw receipt에 기록한다.

### 종료 조건

- `localVertexFactorySelectionAdmission=true` 또는 같은 강도의 native/actual draw evidence가 존재한다.
- wrong VF/pass/shader/state와 missing MRT attachment를 harness가 거부한다.
- zero draw와 `S_FALSE`가 PASS가 아니다.
- main 3의 material, transform, temporal, actual selection 자동 gate가 모두 닫힌다.
- 사용자 승인 후에만 세 occurrence를 `OCCURRENCE APPROVED`로 올린다.

---

## G05. remaining MeshParticle 10 복원

### 범위

`#4/#7/#8/#12/#13/#14/#15/#17/#18/#26`

### 구현 순서

1. G01 matrix에서 distinct material recipe/ShaderMap/VF/pass group을 확정한다.
2. `#4`, `#7/#8`, `#12~#15`, `#17/#18`, `#26` group마다 대표 canary를 먼저 닫는다.
3. raw DXBC replay와 nonuniform texture fixture를 통과한 recipe만 runtime profile을 추가한다.
4. TypeDataMesh, model material slot, localSpace, StartSize, MeshRotation, signed scale와 dynamic lane을
   occurrence별로 projection한다.
5. actual first draw가 성공한 뒤 같은 group의 나머지 occurrence로 sweep한다.

### 변경 대상

- G01 matrix와 family별 mesh replay receipt
- `Effect_ReconstructedExecution.cpp`
- `Effect_Playback.cpp`
- `Effect_DocumentRenderer.cpp/.h`
- `Shader_VtxEffectMeshPreview.hlsl` 및 recipe include
- `ClientFrontendHarness.cpp`와 새 focused ProjectAudit wrapper

### 자동 gate

- geometry/material slot identity와 WModel bounds
- signed scale에 따른 determinant/winding/cull
- dynamic/particle color 소비 mask
- material texture/register closure
- raw DXBC ↔ runtime constant/nonuniform parity
- Debug/Release actual first draw와 rollback
- main golden fixture 무회귀

### 종료 조건

MeshParticle 13/13이 각 occurrence의 material, transform, temporal, actual VF/pass 자동 gate를 통과하고,
사용자가 fixed-time으로 승인한다. 검정/흰 wedge, fan layer 누락, scale 1/100, cull 반전, dissolve tail이
남으면 해당 occurrence만 RETUNE/BLOCK하고 group 전체를 완료 처리하지 않는다.

---

## G06. SpriteParticle 16 복원

### 범위

`#0/#1/#2/#5/#6/#16/#19/#23/#24/#25/#27/#28/#29/#30/#31/#33`

### 대표 canary

- basic square/rect camera-facing billboard
- velocity/screen alignment와 axis lock
- random 2x2 SubUV
- linear/blended 6x6 SubUV
- signed X/Y size flip과 pivot
- additive/translucent sort/depth가 다른 대표 row
- `#33` distortion Sprite의 distortion render target, scene-color composite와 실행 순서

각 canary는 source occurrence가 실제 가진 policy만 검증한다. 모든 Sprite에 한 billboard mode를 강제하지
않는다.

### 변경 대상

- Sprite material/ShaderMap/VF/pass replay receipt
- `Effect_ReconstructedExecution.cpp`
- `Effect_Playback.cpp`의 SubUV/RNG/size/roll projection
- `Effect_DocumentRenderer.cpp/.h`
- `Engine/Private/VIBuffer_ParticleRect.cpp`
- `Shader_VtxEffectParticle.hlsl`, `Shader_EffectCommon.hlsli`
- family harness와 audit wrapper

### 자동 gate

- quad corner와 camera basis/velocity basis의 numeric oracle
- cm→m size가 정확히 한 번 적용
- signed flip과 pivot 보존
- current/next SubUV transform, blend와 random cadence
- atlas frame 범위와 texture alpha/channel
- sort/blend/depth/cull와 actual particle VF/pass
- `#33` distortion vector/channel, distortion target/pass/state, scene-color composite order의 raw/runtime parity
- 같은 seed의 frame sequence와 seek determinism
- giant quad, full atlas, opacity zero와 submitted 0을 실패로 분류

### 종료 조건

대표 canary가 각각 raw replay/runtime geometry/first draw를 통과한 뒤 16 occurrence를 sweep한다.
사용자가 고정 카메라와 sample time에서 각 occurrence를 승인하기 전에는 Sprite 16/16을 선언하지 않는다.

---

## G07. DecalParticle 3 복원

### 범위

`#20/#21/#22`

### 핵심

현재 size 단일 소유권과 Client Y-yaw canary는 출발점일 뿐이다. original projector volume과
decal VF/pass, scene depth, near/far/depth, blend/MRT를 함께 닫는다.

### 변경 대상

- Decal ShaderMap/VF/pass와 projector/depth receipt
- `Effect_Playback.cpp`의 evaluated projection volume
- `Effect_DocumentRenderer.cpp/.h`
- `Shader_VtxEffectDecal.hlsl`
- 필요한 deferred target/state 경계
- family harness와 audit wrapper

### 자동 gate

- source footprint X/Z와 depth의 단일 owner
- UE→Client projector basis
- projection volume corner/plane과 near/far
- scene-depth compare와 뒤 surface reject
- original blend/depth/cull 및 MRT output
- 카메라 각도/거리 fixture에서 finite projection
- zero draw, whole-surface bleed, z-fighting/depth clip signature 분류

### 종료 조건

3개 모두 material replay, projector numeric oracle, actual decal pass first draw를 통과한다. 파란/흰 판을
단순 texture 문제로 숨기지 않고 projection/depth/pass 중 실패 gate를 receipt에 남긴다. 사용자 승인 전에는
Decal 3/3을 선언하지 않는다.

---

## G08. CascadeRibbon `#3` 복원

### 목표

source point history에서 실제 strip topology와 UV를 생성하고 material replay까지 한 vertical slice로 닫는다.

### 변경 대상

- Ribbon material/BeamTrail VF/pass receipt
- source point-generation/width/lifetime/UV receipt
- `Effect_Playback.cpp`
- `Engine/Private/VIBuffer_DynamicTrail.cpp`
- `Shader_VtxEffectTrail.hlsl`
- `Effect_DocumentRenderer.cpp/.h`
- trail geometry harness와 audit wrapper

### numeric oracle

- source lifetime `2 s`, width `0.15 m` 등 source-owned field
- fixed tick별 point count와 historical position
- segment order, tangent/facing, left/right vertex
- vertex/index count와 degenerate reject
- width interpolation과 cap
- age/distance UV 및 tiling
- source control-point 최대 500을 authoring/runtime cap 512가 수용하는지
- segment tessellation 뒤 render point가 별도 ceiling 16,384 안에 있는지와 overflow를 조용히
  truncate하지 않는지
- seek/straight play의 point history 동치

500-point buffer 수용은 stage blocker만 닫은 상태다. 그것을 topology/width/UV 완료로 승격하지 않는다.

### 종료 조건

zero draw, NaN/spike, twist/camera flip, tail pop, UV slide/stretch와 capacity failure를 각각 자동 fixture가
탐지한다. raw material replay, generated vertex/index/UV oracle와 actual first draw를 모두 통과한 뒤
사용자가 `#3`을 승인한다.

---

## G09. core renderer 33/35 동결

### 범위

`MeshParticle 13 + SpriteParticle 16 + DecalParticle 3 + CascadeRibbon 1`

### 자동 종료 gate

- 33 occurrence의 source identity와 renderer family가 중복 없이 존재
- material/geometry/transform/temporal/renderer-selection disposition이 각 행에 존재
- exact가 아닌 값은 `CURRENT_REVISION_EVIDENCE`, `RECONSTRUCTED`, `MANUAL_TUNED`로 분리
- family별 raw replay/runtime parity/actual first draw PASS
- fixed-step replay와 seek determinism
- corrupt identity/path/register/resource/state와 mid-commit failure rollback
- Debug/Release Engine→UpdateLib→Harness→Client build
- focused core audit PASS, full ProjectAudit 실행과 unrelated/deferred failure 분리, `git diff --check` 0

### 사용자 gate

에이전트는 Client/Effect Tool을 실행하거나 visual PASS를 대신 판정하지 않는다. 자동 gate가 닫히면
사용자에게 complete→family→occurrence solo 순서, 고정 sample time과 확인 항목을 제공한다. 사용자의
서면 승인만 occurrence ledger에 기록한다.

### milestone

33개가 승인되면 상태는 정확히 `Artist F core renderers 33/35`다. `#32/#34`는 deferred이고
Product는 계속 `false`다.

---

## G10. deferred PointLight·ScreenPost와 최종 Product

이 G는 사용자가 우선순위를 다시 올릴 때만 시작한다.

### PointLight `#34`

MaterialShaderMap보다 source PointLight field, lifetime/color/intensity/radius와 CPU LightManager consumer,
실제 scene light submission이 우선이다. flare material이 있으면 별도 rendered occurrence로 replay한다.

### ScreenPost `#32`

scene texture input, global/post-process shader cache, viewport, depth와 render order가 필요하다.
MaterialShaderMap만으로 닫지 않고 post/global shader track으로 분리한다.

### 최종 종료 조건

- Light/Post typed consumer와 actual scene submission
- 35/35 사용자 occurrence 승인
- full ProjectAudit exit 0
- `skillbindings`가 선택한 clip-local `effectref=asset` animevent가 가리키는 Authored Effect,
  Catalog와 admission receipt의 atomic publish/prewarm
- `PlayerSkills.effectId`나 Imported source 이름을 Product target으로 추측하지 않음
- publish 중 실패 시 이전 Product composite 유지

이 조건 전에는 `Artist F 35/35 완료`나 Product 복원을 선언하지 않는다.

---

## 6. 고점을 계속 뚫기 위해 반드시 남은 공통 과제

새 RefShaderCache 발견으로 material 계산식의 가장 큰 벽은 넘었지만 다음은 여전히 별도 복원 대상이다.

1. 전 occurrence cache/code coverage와 hard blocker matrix
2. source TypeData/renderer에서 actual selected VF/pass로 이어지는 admission
3. root/cue projection, 3축 basis, playable yaw, raw pivot의 transform oracle
4. delay/duration/default, source CurrentTime, lifetime와 native RNG의 temporal oracle
5. Sprite billboard/SubUV, Decal projector/depth, Ribbon topology/UV의 geometry oracle
6. VS/input semantic, external opacity, sampler, blend/depth/cull, fog/pass와 non-RT0 MRT
7. actual first draw의 attempted/submitted/failed/committed와 rollback
8. family와 occurrence별 사용자 fixed-time 승인
9. golden fixture를 유지하는 manifest-driven 일반화

일반화 대상은 evidence schema와 harness다. renderer geometry와 output sink를 하나의 범용 shader나 전역
좌표 보정으로 합치지 않는다.

---

## 7. 검증 명령과 증거 기록

### 매 G에서 유지할 golden gate

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470MainShaderMapIdentity.ps1
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470MainOriginalDxbcReplay.ps1
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470MainRuntimeSourceReplay.ps1 -Deep
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-Artist31470RuntimeMaterialV2.ps1
```

### runtime first draw

```powershell
MSBuild Engine/Default/Engine.vcxproj /p:Configuration=<Debug|Release> /p:Platform=x64
UpdateLib.bat Debug
UpdateLib.bat Release
MSBuild Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj /p:Configuration=<Debug|Release> /p:Platform=x64
MSBuild Client/Default/Client.vcxproj /p:Configuration=<Debug|Release> /p:Platform=x64
Tools/ClientFrontendHarness/Bin/<Configuration>/ClientFrontendHarness.exe --effect-reconstructed-gpu-material Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json
```

제품 Client의 기본 driver는 HARDWARE를 유지하고 noninteractive actual-draw fixture만 WARP를 사용한다.

### final regression

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

실행하지 않은 검증은 RESULT에 PASS로 쓰지 않는다. full ProjectAudit의 unrelated/stale failure와 focused
Artist gate 결과를 분리하고, 최종 Product 직전에는 full exit 0을 요구한다.

---

## 8. 상태와 승인 용어

| 상태 | 의미 |
|---|---|
| `SOURCE_EXACT` | source-era identity, selected execution path와 값/평가 의미까지 봉인 |
| `SOURCE_DXBC_CANDIDATE_REPLAY` | exact ShaderMap/DXBC 후보와 fixed-input replay가 닫힘 |
| `CURRENT_REVISION_EVIDENCE` | current revision default/native 값이며 source-era exact가 아님 |
| `RECONSTRUCTED` | bounded evidence로 결정론적 재구성 |
| `MANUAL_TUNED` | source edge가 끝내 없어 사용자가 occurrence-local로 승인한 보정 |
| `AUTO PASS` | parser/replay/build/first-draw 자동 gate 통과 |
| `USER MATERIAL PASS` | material sub-check만 사용자 승인 |
| `OCCURRENCE APPROVED` | material, geometry, transform, temporal, renderer 전체 사용자 승인 |
| `CORE 33/35` | Mesh/Sprite/Decal/Ribbon 승인, Light/Post deferred, Product false |
| `PRODUCT 35/35` | Light/Post 포함, full audit와 atomic publish까지 완료 |

비슷하게 보인다는 이유로 상태를 올리지 않는다. 자동 PASS와 사용자 visual approval를 합산해 하나의
완료 수치로 만들지 않는다.

---

## 9. 바로 다음 세션의 범위

다음 구현 세션은 **G01 coverage matrix와 G02 main transform canary**로 연다.

- G01은 read-only로 전체 family의 cache/code hard blocker를 먼저 드러낸다.
- G02는 공유 runtime의 첫 mutation이며 main material HLSL을 고정한 채 root/basis/pivot만 닫는다.
- G03 temporal source 조사는 G01/G02와 병렬로 수집할 수 있지만 runtime 반영은 G02 뒤에 한다.
- G02/G03/G04와 사용자 main occurrence 판정이 끝나기 전 remaining family runtime sweep으로 넘어가지 않는다.

다음 세션 종료 산출물은 구현 diff 자체가 아니라 다음 네 가지가 함께 있어야 한다.

1. canonical coverage 또는 transform receipt
2. mutation/negative fixture가 있는 focused harness
3. Debug/Release와 main golden fixture 무회귀
4. 실제 실행 증거만 기록한 새 RESULT
