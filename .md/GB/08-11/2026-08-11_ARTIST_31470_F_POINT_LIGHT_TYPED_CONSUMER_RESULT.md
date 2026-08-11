# Artist 31470 F PointLight Typed Consumer 결과

날짜: 2026-08-11

브랜치: `codex/artist-f-pointlight-consumer-v1`

기준 commit: `8352a3718bcf0a3f9386e913a8a90807e70f8bd0`

기준 tree: `77c75c6915d482e61b89c273f41f36d7407b4188`

상태: 독립 재감사 corrective 구현·D/R rebuild·audit 완료,
independent final review 대기, unstaged/uncommitted/unpushed

## 독립 재감사 corrective

초기 frozen verdict의 P1 2개와 P2 1개를 현재 source snapshot에서 교정했다.

- compiled fixture 입력을 Artist exact tuple로 올렸다. CPU executor가 미리 평가한 nontrivial world
  position `(1.25, -2.5, 3.75)`, source radius `200 * 0.01 = 2`, intensity `10`, exponent `2`,
  normalized white alpha `0`, zero ambient/specular를 사용한다.
- helper 직후와 `CPresentation_Manager` transient storage 뒤에 type, position XYZW, range,
  exponent, diffuse RGBA, ambient RGBA, specular RGBA를 모두 bit-exact 비교한다.
- compiled shader 호출은 Point descriptor인데도 directional-shadow 요청 인자를 `true`로 전달한다.
  production Point branch는 이를 무조건 `g_iApplyDirectionalShadow=0`으로 바인딩하며, focused
  source contract와 `0u -> 1u` mutation이 이 경계를 고정한다.
- `EFFECT_LIGHT_DETAIL_DESC`의 inherited/default exponent를 legacy `1.f`로 명시하고,
  enabled Light의 codec validation을 finite `> 0.f`로 강화했다. explicit `0` document는 direct
  validation과 parse 모두 실패하고, failed parse는 이전 output serialization을 보존한다.
- Artist 31930에서 확인된 enabled exponent `0` 두 행은 생성기 정본의 missing class-default
  처리 결과였다. 생성기를 `property absent -> 1.0`, `property present invalid -> ValueError`로 고친 뒤 같은 source
  receipt/normalized graph/module closure로 재생성했으며 JSON diff는 정확히 두 행의
  `0.0 -> 1.0`뿐이다. 전체 `Data/Effects` 재검사에서 enabled nonpositive Light는 `0`개다.

이 consumer는 CPU executor가 만든 pre-evaluated tuple부터 소비한다.
`EMITTER_TRANSFORM_PLUS_PARTICLE_LOCATION`의 emitter transform + particle location 계산과 source의
`castCompositeShadow=false`, `affectCompositeShadowDirection=false` 해석은 CPU executor의 선행
계약으로 남는다. 이 결과는 그 source 계산이나 Artist F 전체 실행을 증명하지 않는다.

### Explicit-property presence와 semantic-key follow-up

두 번째 독립 판정의 P1은 `dict.get()`과 `finite_number()` 조합이 explicit invalid 값을 missing
class default처럼 취급할 수 있다는 점이었다. 최종 생성기는 key membership를 먼저 확인한다.

- `falloffexponent` semantic base key가 없을 때만 inherited legacy `1.0`을 materialize한다.
- unindexed semantic key가 정확히 하나 있으면 raw value가 finite non-bool number여야 한다.
- present `False`, `True`, string, null, `NaN`, `+Inf`, `-Inf`, `0`, `-1`은 모두 `ValueError`다.
- explicit finite `2.0`은 output `2.0`으로 그대로 보존된다.

같은 unit test 안의 direct subtest matrix가 위 9개 invalid 값, missing `1.0`, explicit `2.0`을
검증한다. Artist31930을 같은 세 입력 정본으로 실제 재생성한 결과 current document
`8206722` bytes / `c6c7b3a2...107474`와 receipt `2047797` bytes /
`70f131e0...f4742`가 각각 raw byte-identical이었다. 전역 enabled nonpositive scan도 `0`이다.

이 follow-up은 Python generator, generator unit, focused PowerShell audit만 바꿨다. C++/project/data
byte는 바뀌지 않았으므로 C++ rebuild는 반복하지 않았고, 기존 최종 Debug/Release PointLight WARP
binaries를 다시 실행해 양쪽 모두 exact tuple PASS를 확인했다.

다음 독립 판정에서 JSON property spelling의 case alias가 membership 검사를 우회할 수 있음을 추가로
확인했다. 최종 생성기는 `base_property_name(str(key)) == "falloffexponent"`인 모든 semantic key를
먼저 수집한다.

- semantic matching key `0`개만 missing으로 보아 default `1.0`을 쓴다.
- indexed key가 없고 matching key가 정확히 `1`개면 그 실제 key의 값을 strict finite non-bool number와 `> 0`으로
  검증하며, source parameter path에도 실제 key spelling을 보존한다.
- indexed key가 없고 matching key가 `2`개 이상이면 canonical/case-alias duplicate로 생성 전체를 거부한다.
- unit direct cases는 `FalloffExponent`, `falloffExponent`, `FALLOFFEXPONENT` 각각에서 valid `2.0`
  보존과 invalid `0` 거부를 확인한다. canonical+alias duplicate와 JSON `\u0045`/`\u0065` escape로
  만들어진 upper/lower alias duplicate도 거부한다.

세 번째 독립 판정은 scalar property의 indexed spelling이 semantic key 검사를 우회하는 문제였다.
`base_property_name`은 첫 `[` 앞부분을 casefold하므로 `falloffexponent[0]`, `[1]`, 대소문자 variant도
같은 semantic key로 수집된다. 그 집합에 bracket key가 하나라도 있으면 scalar indexed alias로 즉시
`ValueError`를 낸다. standalone `[0]`, `[1]`, `FalloffExponent[0]`, `FALLOFFEXPONENT[1]`과 valid
canonical `2.0` + indexed alias `0.0` 조합을 실제 `build_document` direct case로 거부했다. 기존
canonical/case-alias duplicate와 escaped JSON alias 거부도 그대로 유지한다.

이들 semantic-key/scalar-indexed follow-up 뒤 Artist31930을 다시 실제 재생성했다. document와 receipt는 위와 같은
raw byte/SHA로 다시 byte-identical이었고, 1,394개 `Data/Effects` JSON 전역 scan에서 enabled Light는
2개, invalid/nonpositive exponent는 `0`개였다. 변경 파일은 같은 Python generator/unit/focused audit
세 개뿐이므로 C++ rebuild 없이 기존 WARP D/R binaries를 다시 실행해 양쪽 exact tuple PASS를
확인했다.

## Corrective raw source freeze

아래 16개 구현·data·project·harness·audit 파일을 ordinal path 순서로 raw byte count와 SHA-256에
고정했다. 이 표를 포함하는 PLAN/RESULT 문서는 순환 hash를 피하려고 manifest에서 제외했다.

ordered manifest SHA-256:
`952ba1b8098d10a85b498fb1b590e351e96abcc452f350acdd32a34b7f508abc`

| Path | Raw bytes | SHA-256 |
|---|---:|---|
| `Client/Default/Client.vcxproj` | 223621 | `c581581dc7b7ca97ca602c2e2028f0e32366ae1c391e05f4e7133e0405d27c8b` |
| `Client/Default/Client.vcxproj.filters` | 400698 | `f1332b0120bfa7f3137789c1bf4e94c22cadda06ce5933bbd6833da0f04b5492` |
| `Client/Private/Effect_DocumentCodec.cpp` | 199317 | `bc1307b518967942a15b62e1179b11efc9e7eb3bf8889a8c12f393cc7f6e37db` |
| `Client/Private/Effect_LightPresentation.cpp` | 1692 | `412c78cf803716bf201aeef1a6ded85a0a1af98a3b808fe5eb77b95a72545369` |
| `Client/Private/Effect_Object.cpp` | 9367 | `7f3acfec91f1fe9fd2f8b66819b77a5ae927b2567c2fc64a9611d94ffb6ff3df` |
| `Client/Public/Effect_AuthoringDocument.h` | 23825 | `1843c1df9b47d8935daacb0ef8d249e82ff52ba32717bca10ef3b711ffef2108` |
| `Client/Public/Effect_LightPresentation.h` | 344 | `b6fc7a3e416f50e78c0af8128499f547700b2ef68ef08f72ea958e07dbc6ff5e` |
| `Data/Effects/Imported/Artist/CurrentCombat/Converted/effect.artist.skill.31930.imported.effect.json` | 8206722 | `c6c7b3a24f6c5cbe0602d35150cfc30dd6ff2a4434de95e410ac907dec107474` |
| `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp` | 331038 | `460ecaf33e04e4fb6d2960f173e48bed4f6a4a2c5d4a40b0aeb885ee72953843` |
| `Tools/LevelPlacementExtractor/build_imported_effect_documents.py` | 101889 | `6722178244776d45149aedf10014c2eae73b1781d21272e9b258a96d71468ab0` |
| `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py` | 47801 | `14d279ad4f3df37f3ae07054bf1c98ce89a0b03bb59414a34e03de49f8c0b2f1` |
| `Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj` | 5460 | `b729aad86cccc616255662b679ca32d9c7b3d8117d90ce05e0a6e12feb1f2eee` |
| `Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj.filters` | 889 | `e4ee87ae4ff7bca89427fbade47afb416b9698fe5c89424422b84e29d86610f6` |
| `Tools/PointLightFalloffContractHarness/Private/PointLightFalloffContractHarness.cpp` | 12544 | `b130d89b5028cf9f3e5c176570b8abd74eaeb482a1fc792620b07b739dd9884a` |
| `Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1` | 1252 | `153a624b0c27f19c1265d066aedc09f7483f5192b95199d4185c7a1f782854a7` |
| `Tools/ProjectAudit/Test-PointLightFalloff.ps1` | 37203 | `c15a74dcf475bee6ca2aa24db3756951149b2b60d70634c616364b08dd49dd8f` |

## 완료한 계약

- 기존 `CEffectPlayback`이 만든 `EFFECT_EVALUATED_LIGHT::fFalloffExponent`를
  `LIGHT_DESC::fFalloffExponent`에 빠짐없이 복사하는 단일 production 변환 경계를 추가했다.
- 변환 경계 `Try_BuildEffectPointLightDesc`는 position, range, intensity, color, ambient,
  falloff exponent와 intensity 곱셈 결과를 모두 검증한다.
- range와 exponent는 finite 양수, intensity는 finite 비음수여야 한다. 검증에 실패하면 caller의
  기존 `LIGHT_DESC`를 한 byte도 commit하지 않는다.
- `CEffectObject::Submit_Presentation`은 수동 descriptor 조립 대신 이 변환을 호출하고, 성공한
  descriptor만 기존 `CPresentation_Manager::Add_TransientLight`에 제출한다.
- 기존 경로는 하나뿐이다.

```text
CEffectPlayback evaluated frame
-> CEffectObject::Submit_Presentation
-> Try_BuildEffectPointLightDesc
-> CPresentation_Manager::Add_TransientLight
-> CLight::Render_Desc
-> g_fLightFalloffExponent
-> Resolve_PointLightAttenuation
```

- evaluated `2.0f`는 `LIGHT_DESC`, transient vector, 실제 `CLight::Render_Desc`까지 bit-exact
  `2.0f`로 유지된다.
- explicit/default `1.0f`는 같은 경로에서 bit-exact로 유지되고 기존 shader 선형 감쇠 분기를 쓴다.
- 새 Light manager, renderer, test-only production 분기, Artist ID 하드코딩은 추가하지 않았다.

## 변경 파일

- `Client/Public/Effect_LightPresentation.h`
- `Client/Private/Effect_LightPresentation.cpp`
- `Client/Private/Effect_Object.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `Client/Public/Effect_AuthoringDocument.h`
- `Client/Private/Effect_DocumentCodec.cpp`
- `Data/Effects/Imported/Artist/CurrentCombat/Converted/effect.artist.skill.31930.imported.effect.json`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py`
- `Tools/PointLightFalloffContractHarness/Private/PointLightFalloffContractHarness.cpp`
- `Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj`
- `Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj.filters`
- `Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1`
- `Tools/ProjectAudit/Test-PointLightFalloff.ps1`
- `.md/GB/08-11/2026-08-11_ARTIST_31470_F_POINT_LIGHT_TYPED_CONSUMER_PLAN.md`
- `.md/GB/08-11/2026-08-11_ARTIST_31470_F_POINT_LIGHT_TYPED_CONSUMER_RESULT.md`

새 H/CPP는 Client project와 기존 `03. Tools\02. Effect` filter에 등록했다. harness project는
같은 production CPP를 직접 컴파일하며 Client include path와 `d3d11.lib`를 명시한다.

## Compiled focused harness — corrective final snapshot

최종 raw source freeze 뒤 Debug와 Release를 각각 빌드하고 WARP에서 실행했다. full Client rebuild가
Engine DLL을 재배포한 뒤에도 같은 D/R executable을 다시 실행해 최종 runtime binary 조합을 확인했다.

```text
PointLightFalloffContractHarness PASS: artist-exact-tuple-desc-transient-
shader-point-shadow0; legacy1; invalid-evaluated-rollback; abi/default;
final-boundary; scene-rollback; transient-no-push
```

harness는 WARP D3D11 device, 실제 `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`, `CShader`,
`CVIBuffer_Rect`를 만들고 production 변환이 만든 exponent `2.0f`와 legacy `1.0f` descriptor로
`CLight::Render_Desc`를 실제 호출했다.

fail-closed 입력 행렬은 다음과 같다.

- exponent: `0`, `-1`, `NaN`, `+Inf`, `-Inf`
- range: `0`, `-1`, `NaN`, `+Inf`, `-Inf`
- position: nonfinite
- intensity: 음수, nonfinite
- color/ambient: nonfinite
- intensity 곱셈: finite 입력끼리 overflow하여 nonfinite 결과

각 변환 실패에서 기존 output descriptor가 보존됐다. 기존 Engine final boundary의 invalid reject,
scene batch rollback, transient no-push 회귀도 함께 PASS했다.

## Focused audit

`Tools/ProjectAudit/Test-PointLightFalloff.ps1` 결과:

```text
PointLight falloff exponent audit PASS; legacyDefaultSamples=6;
exponents=0.5/1.0/2.0; invalidExponents=5; invalidRanges=5;
mutations=49; engineClientShaderEqual=true
```

corrective source snapshot의 현재 focused 결과는 `mutations=49`이다. 추가 mutation은 exact tuple의
position/range/color/ambient/specular 매핑, Point shadow `0`, authored default `1`, codec `> 0`,
generator missing default/explicit-zero reject, generator unit assertion, Artist 31930 enabled row,
failed parse rollback assertion을 각각 깨뜨린다. 두 번째 follow-up은 property-presence predicate,
present-invalid finite rejection, invalid-value matrix, explicit-two preservation을 깨뜨리는 mutation
4개를 더한다. semantic-key follow-up은 casefold key match, zero-key predicate, alias duplicate rejection,
세 casing matrix, duplicate direct case, escaped upper/lower JSON key를 각각 깨뜨리는 mutation까지
추가한다. scalar-indexed follow-up은 semantic base helper/call, bracket-key 수집, indexed scalar reject,
indexed casing matrix, canonical+indexed direct case를 각각 깨뜨리며 모두 focused audit에서 거부됐다.

## Corrective 최종 빌드와 실행 검증

source manifest를 먼저 고정한 뒤 build lease에서 `/m:1`, `/nr:false`로 직렬 실행했다.

| 단계 | 결과 |
|---|---|
| Engine x64 Debug | clean rebuild exit 0, compile/link 성공 |
| Engine x64 Release | clean rebuild exit 0, compile/link 성공 |
| `UpdateLib.bat Debug` | exit 0 |
| `UpdateLib.bat Release` | exit 0 |
| PointLightFalloffContractHarness Debug build/run | exit 0 / exit 0, Artist exact tuple PASS |
| PointLightFalloffContractHarness Release build/run | exit 0 / exit 0, Artist exact tuple PASS |
| ClientFrontendHarness Debug clean rebuild/full run | build exit 0, corrective tests PASS, known baseline 8개만 실패 |
| ClientFrontendHarness Release clean rebuild/full run | build exit 0, corrective tests PASS, Debug와 동일한 known baseline 8개만 실패 |
| Client x64 Debug | clean rebuild exit 0, 새 helper/codec/`Effect_Object` compile/link 성공 |
| Client x64 Release | clean rebuild exit 0, 새 helper/codec/`Effect_Object` compile/link 성공 |

Engine project reference의 기존 post-step은 PointLight harness build 끝에 `pwsh.exe not found` 문구를
출력했지만 Debug/Release compile/link와 MSBuild exit는 0이었고 runner를 통한 exe 직접 실행도 각각
exit 0이었다. 기존 FXC/encoding/PDB warnings 외 새 Light slice warning/error는 없었다.

build 종료 뒤 첫 corrective의 16-file source manifest mismatch는 `0`이었다. 이후 Python/test/audit
3개만 바꾼 explicit-property, semantic-key, scalar-indexed follow-up의 최종 ordered manifest SHA는
`952ba1b8098d10a85b498fb1b590e351e96abcc452f350acdd32a34b7f508abc`다. 나머지 C++/project/data
13개 raw hash는 build snapshot과 동일하다.

### Final binary provenance

timestamp는 UTC다.

| Binary | Raw bytes | Last write UTC | SHA-256 |
|---|---:|---|---|
| `Engine/Bin/Debug/Engine.dll` | 8046080 | `2026-08-11T02:10:50.1647707Z` | `31562560eb7c053c5d77fdf464da24dd2704558c5888b2b68dd929811e2c6be1` |
| `Engine/Bin/Release/Engine.dll` | 1878016 | `2026-08-11T02:16:19.6810788Z` | `c176445d190e32ef42bf247ce09d8627653e17b56d0c3ee38382bd037586d63e` |
| `Tools/PointLightFalloffContractHarness/Bin/Debug/PointLightFalloffContractHarness.exe` | 223232 | `2026-08-11T02:02:06.6315858Z` | `ef7b430a64f01ae044b94589312040b018b7b59375c1842669c9126e5ebbaa43` |
| `Tools/PointLightFalloffContractHarness/Bin/Release/PointLightFalloffContractHarness.exe` | 40960 | `2026-08-11T02:02:26.1954573Z` | `a5f64d872cc94c96ec579ad428ea6e7dd73d38fbdd5ba12c88e51c2d0159e277` |
| `Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe` | 7386112 | `2026-08-11T02:03:34.3620270Z` | `57105874970788edf075a174f3d049ff6cc21a0dbaf325c5ebed8bc15bb3afc2` |
| `Tools/ClientFrontendHarness/Bin/Release/ClientFrontendHarness.exe` | 1726464 | `2026-08-11T02:08:10.9957138Z` | `15d2b38f7562e0e1b9726b01b5e5da591615cf961f8fa36405651aa533cacccb` |
| `Client/Bin/Debug/Client.exe` | 13987840 | `2026-08-11T02:14:22.0083695Z` | `d542dbe66d51864aee07ff92c7b724af614291199b1414bbe060d3d47789d40a` |
| `Client/Bin/Release/Client.exe` | 2254848 | `2026-08-11T02:19:46.2289687Z` | `60f9ea175753d7f20f61e5651997a0c46d1f65e53a37ee26badcf296e2f8f742` |

`Client/Bin/Debug/Engine.dll`과 `Client/Bin/Release/Engine.dll`은 각 configuration의 위 Engine
binary와 raw SHA-256이 동일하다.

### ClientFrontendHarness 외부 baseline

전용 clean worktree에는 Git 비관리 `Client/Bin/Resources`가 없었다. 첫 Debug 실행은 이 환경 누락으로
19개 resource 관련 실패를 냈다. Resources를 복사하거나 검증을 완화하지 않았다.

팀 정본 `C:/Users/user/Desktop/LostArk/Client/Bin/Resources`가 정확히
`Fonts, Character, Deploy, Effect, Map, UI` 여섯 폴더이고 대표 DDS 두 파일이 존재함을 확인한 뒤,
이 경로를 process-local `LOSTARK_RESOURCE_ROOT`로만 지정해 다시 실행했다. Debug/Release는 모두
exit 1, 아래 기존 baseline 8개로 정확히 동일했다.

1. Every Current Imported Baseline Round Trips And Preserves Pending Materialization State
2. Canonical DimensionMaster T Round Trips Summon Model Cue
3. All Effects Joins Every PlayerSkills EffectId To Valid Authored Document
4. Manual Mesh Create Contract Round Trips One Percent Scale Carrier Layer
5. Effect Local Crack Admits Legacy Carrier But Rejects Partial Named Contract
6. Effect Fail-Closed Mesh Without Base Stages For Hidden Rendering
7. DimensionMaster Q Stable Emitter Material Tuning Saves And Reloads Losslessly
8. Effect Runtime Invalid Catalog Preserves Committed Assembly State

이 8개 이름과 개수는 D/R에서 동일하고, 이번 Light helper/Object 경로와 연결된 실패는 0개다.
같은 실행에서 Four-Class runtime contract, 모든 product document stage, current WFX component stage는
PASS했다.

## 전체 ProjectAudit

canonical Resources를 명시한 `Invoke-ProjectAudit.ps1`은 첫 corrective snapshot에서 108 checks를 실행했다. 새
`rendering.point-light-falloff-contract`는 PASS했고 당시 detail에 34-mutation focused evidence를
기록했다. 전체 exit는 이 slice 외 기존 baseline 11개 때문에 1이다.

explicit-property, semantic-key, scalar-indexed follow-up은 전체 audit orchestration을 다시 돌리지 않고,
그 check가 직접 호출하는 focused audit를 최종 source에서 별도 실행해 `mutations=49` PASS를
확인했다. 변경된 세 파일은 generator/test/focused audit뿐이고 Artist31930 data는 byte-identical이다.

```text
projects.data-source-visibility
effect.g09-authoring-world-runtime-boundary
effect.g09-cross-document-contract
effect.artist-31470-reconstructed-source-capability
effect.artist-31470-material-texture-runtime-binding
effect.artist-31470-exact-dds-runtime-deployment
effect.artist-31470-wmodel-geometry-contract
effect.artist-31470-reconstructed-runtime-program
effect.wfx-component-assembly
effect.representative-authored-readiness
effect.four-class-authored-clip-product-exact101
```

대표 baseline detail은 `Artist/31210` manifest/binding stage count mismatch와 clean worktree에 미빌드된
`WModelGeometryContractHarness.exe`다. PointLight focused check에는 실패가 없다.

## 유지한 경계와 미검증

- reconstructed `runtimeExecution`와 `Product` admission은 계속 false다.
- `Admit_Execution`, `Admit_Submit`, `Admit_Render`를 열지 않았다.
- `Data/Balance/PlayerSkills.json`, animevents, Product/runtime data는 변경하지 않았다.
- 이 slice는 이미 존재하는 evaluated Light의 truthful Presentation 소비자만 닫았다.
- Artist 31470 F reconstructed program 전체 실행, Playback 실행 admission, 실제 F 입력과 화면 확인은
  아직 완료하지 않았다.
- 따라서 “Artist F Light가 인게임에서 연결 완료” 또는 “눈으로 확인 완료”라고 선언하지 않는다.

## Frozen 상태

- XML parse: PASS
- PowerShell parser: PASS
- imported-effect generator unit: 16/16 PASS
- Artist 31930 enabled Light: 2개, exponent `1.0/1.0`; 전체 `Data/Effects` enabled nonpositive: 0개
- `git diff --check`: PASS
- final raw source manifest: 16 files, ordered SHA `952ba1b8...508abc`
- 이 worktree 소유 MSBuild/cl/link/Client/harness 잔류 process: 0
- build lease: release 완료
- commit/push: 수행하지 않음
- manual runtime eye validation: 수행하지 않음

변경은 dedicated clean worktree에 unstaged/uncommitted로 동결했다. independent review가 승인하기 전에는
commit이나 push를 수행하지 않는다.
