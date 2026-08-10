# 2026-08-10 Artist 31470 F Material Source Value Acquisition Result

## 결론

Artist F Material은 아직 R2로 진입할 수 없다. 다만 기존 `0/94 static` 판정은 너무 거칠었고,
기존 `3+1 SOURCE_EXACT_SAMPLER` 판정은 잘못됐다.

| gate | strict denominator | source value acquired | execution ready |
|---|---:|---:|---:|
| render-state default | 89 | 0 | 0 |
| static permutation | 94 | 23 | 0 |
| sampler full descriptor | 72 | 0 | 0 |
| 합계 | 255 | 23 | 0 |

Unit A 시점에는 acquisition receipt 자체만 evidence integrity PASS였고 입력이 된 frozen Material
`627ddc76ef58e45f35821363c93197157da4cf89`의 evidence-integrity PASS를 철회해야 했다. Unit B는
exact 4를 BLOCK으로 재분류하고 static 94의 GUID/value/`bOverride` lineage 및 strict 72/total 255를
typed contract와 runtime receipt에 coordinated reseal했다. 현재 Material evidence integrity는 다시
PASS지만 execution readiness는 0/255다. Product는 false이고 R2는 `NO-GO`다.

이 결과는 최신 계획 정본 `7ffb8a3bf123703ea451cbe53a178f449f102fbe`의
`Material 0/89 + 0/94 + 0/68` 뒤에 발견된 post-plan corrective delta다.

## static 94 재감사

94개 parent static-switch expression을 source UPK에서 다시 읽어 `ParameterName`, `DefaultValue`,
`ExpressionGUID` raw property를 고정했다. 같은 recipe의 MIC serial/native tail도 다시 읽고
`FStaticParameterSet` entry의 value, `bOverride`, GUID, byte offset를 조인했다.

- name+GUID exact join: `66/94`
- `bOverride=true`: `23`
  - source MIC가 선택값을 명시한 행이므로 `SOURCE_EXACT_INSTANCE_OVERRIDE_VALUE_ACQUIRED`
  - parent와 같은 값 14, 다른 값 9
- `bOverride=false`: `43`
  - raw entry와 exact parent default는 일치하지만 inheritance semantics의 독립 output proof가 없다.
  - 값 후보는 보존하되 execution-ready로 승격하지 않았다.
- exact GUID entry 없음: `28`
  - parent default만 존재한다. selection 값으로 세탁하지 않았다.

기존의 `NO_INSTANCE_STATIC_SELECTION_RECORD` blanket reason은 exact native-tail entry가 있는 66행에
대해 사실이 아니다. 후속 corrective는 세 decision을 행별로 보존해야 한다.

## sampler strict denominator 72

기존 blocked 68과 이전 exact 4를 전부 같은 기준으로 다시 검사했다.

- strict row: `72 = instance override 71 + parent default 1`
- unique Texture2D: 48
  - source archive exact Texture2D: 45
  - current-only `WP_MN_LRCN_01`: 3
- source explicit value-only evidence
  - `AddressX=Clamp`, `AddressY=Clamp`: 9행
  - `sRGB=false`: 3행
  - LODGroup record는 존재하지만 source-revision TextureGroup filter configuration은 없다.
  - explicit `Filter`: 0행
- full source-exact descriptor: `0/72`

이전 exact 4의 raw field 재감사 결과는 다음과 같다.

- 3행: AddressX/AddressY/sRGB 모두 omitted
- 1행: AddressX/AddressY만 serialized clamp, sRGB omitted
- 4행 모두 Filter와 source-revision class/config default provider가 없다.

따라서 `SOURCE_EXACT_SAMPLER 4/4`는 `BLOCKED 4/4`로 바뀌었고, typed contract의 direct sampler는
exact 0/unproven 71, strict sampler denominator는 `68 -> 72`, 전체 feasibility denominator는
`251 -> 255`로 coordinated reseal됐다.

추가로 `fx_tex_04.fx_h_wave_01`은 Artist F 31470 matrix에서 실제 소비하지만
`Artist.resource-source-manifest.json`에는 skillIds `[31920]`만 있어 기존 skill selector에서 빠진다.
이번 corrective는 current fallback을 exact로 세탁하지 않고 이 selector 경계를 receipt에 그대로
보존했다. 별도 source manifest 수정 없이는 해당 행을 source-exact로 승격할 수 없다.

## render-state 89와 CDO

89행은 다음 5 provenance family로 줄었다.

| field | row |
|---|---:|
| bDisableDepthTest | 26 |
| bUseOneLayerDistortion | 26 |
| OpacityMaskClipValue | 26 |
| TwoSided | 9 |
| LightingModel | 2 |

현재 설치 `Default__Material`에는 `OpacityMaskClipValue=0.33329999446868896`가 serialized되어 있고,
`Default__Texture`에는 `sRGB=true`, `Filter=TF_Linear`가 serialized되어 있다. 그러나 source package
revision과 현재 Engine/Core/native binary의 build identity를 묶는 manifest가 없으므로 모두
`CURRENT_REVISION_CROSS_REVISION_CANDIDATE_ONLY`다. depth/two-sided WARP pilot도 descriptor consumer
semantics만 증명하며 source 값을 증명하지 않는다. render source value는 `0/89`다.

## provider exhaustion

- local source archive
  - raw UPK `1,813`, `1,932,762,844` bytes
  - SHA-256 unique `624`, duplicate copy `1,189`
  - v3 manifest exact `621`, extra unique package `3`
  - extra 3 package에서 Artist nativeStateKey raw/logical hit `0/23`
  - source-era Engine/Core script package, ShaderCache, native provider 없음
- current installed ShaderCache/driver cache
  - source Material/native key/DXBC exact join 0
  - D3DSCache 72/72 readable, NVIDIA DXCache 548/561 readable, GLCache 74/74 readable
  - nativeStateKey `0/23`, shaderId `0/271`, DXBC digest `0/240`
  - NVIDIA 2023 cache의 `lostark.exe` 문자열은 basename-only이고 PE/package/material identity가 없다.
- Git/remote
  - reachable UE3 UPK/script package 0
  - unreachable 대형 UPK 1개는 map-only이며 Material/MIC/ShaderCache 0
  - LFS store UE3 UPK 0, game native provider 0
  - GitHub Actions artifacts/cache 0, Releases 0, user container source package 0
- runtime capture
  - source-revision runtime bundle/debug API가 없다.
  - process injection과 anti-cheat 우회는 시도하지 않았고 허용하지 않는다.

위 driver/Git/remote 수치는 이 generator가 재실행하는 검증 manifest가 아니라 2026-08-10 외부
read-only audit의 corroboration-only snapshot이다. 접근 가능한 경로와 readable bytes만을 범위로 하며
NVIDIA share-locked 13개 파일은 제외됐다. VSS는 `PERMISSION_UNCHECKED`, global exhaustion claim은 false다.

최소 missing artifact는 source package revision과 인증된 `Engine.u/Core.u`, version-coupled native
EFEngine/LOSTARK binary, SystemSettings TextureGroup config, ShaderCache/material map, vendor-authorized
offline capture path를 하나의 build identity로 묶은 bundle이다.

## 산출물

- `Tools/LevelPlacementExtractor/build_artist_31470_material_source_value_acquisition.py`
- `Tools/LevelPlacementExtractor/test_build_artist_31470_material_source_value_acquisition.py`
- `Data/Effects/Imported/Artist/Materials/skill.31470.material-source-value-acquisition.receipt.json`
- 대응 PLAN/RESULT

receipt의 row-set digest는 다음과 같다.

- render 89: `f0f20d487a76778fc8daae91e6b19b3f1621d64b1a6b6a73eaf3ced1da109cd3`
- static 94: `4952bd40f2dab08bb5e88122baf6d154fc71c5b9462f8c3a90a69cbfaa5d74a9`
- sampler 72: `420ebef1ebd4bee9c405f2f7cd735b18b837a8315b7440c1bc61bd0d342f6de7`
- invalidated exact 4: `5c640f8e90a5563a7660419d7a9120842787b2f274424c5be33e85262306e37b`

## 검증

```powershell
python -m unittest Tools.LevelPlacementExtractor.test_build_artist_31470_material_source_value_acquisition -v
```

결과: 7 tests PASS. raw source full rebuild, GUID/value/`bOverride` coordinated reseal, omitted
default의 SOURCE_EXACT 승격, exact-4/72 denominator, external snapshot/VSS qualification,
Product/R2 closure를 검사했다.

Unit A에서 전체 `Invoke-ProjectAudit.ps1`도 실행했지만 이 branch 밖의 기존 baseline 12개 항목 때문에 FAIL했다.
대표 원인은 source-contract publisher version 문자열 불일치, 미빌드 WModel harness, 누락 runtime
character asset, Artist/31210 rollout stage mismatch다. 이번 5개 신규 파일은 audit 실행 전후 동일했고,
Unit B는 전체 audit를 재실행했다고 기록하지 않고 Material evidence/runtime focused audit와 raw
generated receipt checks를 다시 PASS했다. Unit A의 FAIL을 Material
source-value PASS로 기록하지도, 이번 범위에서 무관한 baseline을 수정하지도 않았다.

Unit B는 이 receipt를 입력으로 exact 4, static 94 reason, sampler/total denominator, dependent hash와
테스트를 coordinated corrective했다. 이 복구는 evidence-integrity만 다시 열었으며 0/255 execution
readiness, Product false, R2 NO-GO는 그대로다.
