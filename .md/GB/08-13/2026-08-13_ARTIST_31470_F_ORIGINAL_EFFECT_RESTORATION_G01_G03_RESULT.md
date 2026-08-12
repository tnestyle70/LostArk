# 2026-08-13 Artist 31470 F Original Effect Restoration G01-G03 Result

## 판정

Artist F의 family renderer 작업에 들어가기 전 공통 기반인 G01 cache coverage, G02 transform,
G03 temporal canary를 구현하고 집중 자동 검증까지 완료했다. 현재 완료 상태는 family/Product
승인이 아니라 다음 세 경계의 자동 실행 계약이다.

- G01: 35 renderer occurrence와 27 material recipe의 cache 후보 coverage
- G02: main MeshParticle `#9/#10/#11`의 root snapshot, cue, 3축 basis, WModel pre-scale
- G03: 같은 세 occurrence의 source lookup과 60 Hz playback/seek/RNG/lifetime 계약

G01의 shader row는 여전히 candidate이며 actual selected VF/pass admission은 `false`다. G02/G03의
미확정 원본 provenance와 사용자 visual gate도 그대로 남아 있으므로 Product는 `false`다.

## G01 결과

`skill.31470.renderer-restoration-matrix.receipt.json`은 다음 분모와 disposition을 봉인한다.

| 항목 | 결과 |
|---|---:|
| renderer occurrence | 35 |
| rendered material occurrence | 34 |
| material recipe | 27 |
| static parameter-set recipe | 24/24 |
| official/current cache structural join | 48 |
| selected VF/pass admission | 0 |
| Product admission | 0 |

Direct/default material 3 recipe는 static identity 부재를 cache code missing으로 오판하지 않는다.
DXBC bundle과 unresolved code-position을 분리하고 descriptor-tail heuristic을 selection 증거로
승격하지 않는다. 이 결과는 commit `93d8f676`에 포함됐다.

## G02 결과

- extractor와 runtime의 Euler component swizzle을 basis-conjugated rotation으로 교정했다.
- 비대칭 3축 `(30, -45, 60)` canary가 naive swizzle을 거부한다.
- main `#9/#10/#11`은 source cue `[100,-100,0] cm`을 Client `[1,0,1] m`로 변환하고 cue scale 3을
  한 번 적용한다.
- non-follow element가 cue 순간의 nontrivial root를 snapshot하고 이후 이동한 root를 따라가지 않는
  것을 첫 fixed step world matrix로 검사한다.
- WModel `geometryPreScale=.01`은 CModel geometry 경계에서 한 번만 소비된다.

raw UPK에서 glTF까지의 pivot provenance, playable model import yaw와 effect root의 관계, late initial
seek의 historical actor root provider는 해결되지 않았다. 현재 root를 과거 root로 위조하지 않고
각 blocker를 유지한다.

## G03 결과

새 정본은 다음 세 파일이다.

- `Tools/LevelPlacementExtractor/build_artist_31470_main_temporal_oracle.py`
- `Tools/LevelPlacementExtractor/test_build_artist_31470_main_temporal_oracle.py`
- `Data/Effects/Imported/Artist/Temporal/skill.31470.main-temporal-oracle.receipt.json`

receipt schema는 `lostark.artist-31470-main-temporal-oracle-receipt`, format version 1이다. main 세
occurrence의 15개 source sample과 15/15 straight/seek offline projection을 봉인한다.

runtime에서는 deterministic distribution도 무조건 RNG를 소비하던 경로를 고쳤다. operation 2/3만
random draw를 소비하며 Playback과 reconstructed timing evaluator가 같은 정책을 사용한다. Playback의
spawn accumulator에는 source `Rate * RateScale`을 적용한다.

ClientFrontendHarness는 다음을 실제 staged document와 playback으로 검사한다.

- `#9/#10/#11`의 0/25/50/75/100% lifetime source lookup
- alpha, size, mesh rotation rate와 dynamic parameter
- 60 Hz straight play, independent direct seek, reseed의 같은 discrete-tick particle state
- `0.5 s`와 `0.8 s` lifetime의 first-owning-tick 제거 및 zero tail
- element-start-relative material local clock과 particle normalized age의 분리

spawn packet의 age 0과 end-of-step presented frame의 age `1/60`은 서로 다른 phase로 보존했다.
`#9/#10`의 25/75%가 7.5/22.5 tick이라는 사실도 requested source fraction과 fixed-step bracket으로
분리했으며 exact tick이라고 표기하지 않는다.

## 실행 검증

| 검증 | 결과 |
|---|---|
| G01 matrix deep deterministic `--check` | PASS, 35 occurrence / 27 recipe / static 24/24 / join 48 |
| G02 transform oracle mutation suite | PASS, 12 tests |
| G02 transform oracle validate/check | PASS |
| G02 ClientFrontendHarness executor/GPU focused modes | PASS, failures 0 |
| G03 temporal oracle mutation suite | PASS, 12 tests |
| G03 temporal oracle validate/check | PASS |
| Effect data project sync check | PASS, 1412 files / 185 filters |
| Effect data project registration harness | PASS, 1412 files |
| Client x64 Debug incremental build | PASS |
| ClientFrontendHarness x64 Debug focused build | PASS |
| `--effect-reconstructed-gpu-material` temporal canary | PASS, failures 0 |
| `git diff --check` | PASS |

전역 ProjectAudit은 사용자 결정에 따라 실행하거나 복구하지 않았다. Release 전체 빌드와 사용자의
Client visual 판정도 실행하지 않았으므로 PASS로 기록하지 않는다.

## 유지한 blocker

- source-era required delay/duration default provenance
- native UE3 `FMaterialUniformExpressionTime`의 clock origin
- raw UPK to glTF pivot provenance
- playable presentation yaw와 effect root 관계
- product late-join historical actor-root history
- plain product update의 60-step 초과 backlog clock convergence
- actual occurrence-selected VF/pass, shader-object register closure와 non-RT0 output state
- 사용자 fixed-time visual approval

G03 receipt의 runtime/Product admission은 이 미해결 source/native 경계를 숨기지 않기 위해 `false`를
유지한다. focused C++ harness PASS는 current reconstructed runtime의 재현성을 증명하지만 native source
exactness를 대신하지 않는다.

## 다음 재개점

G04에서 G01 receipt 자체의 candidate 의미를 바꾸지 않고 별도 selection-admission receipt를 만든다.
main `#9/#10/#11`의 실제 renderer draw를 occurrence에서 selected VF/pass/shader pair, register closure,
blend/depth/cull과 MRT output state까지 연결한다. 다른 세션의 cross-corpus ShaderMap 결과가 도착하면
그 source identity와 G01 recipe를 join하되 cache offset이나 heuristic code index를 semantic key로 쓰지
않는다. G04가 닫힌 뒤에만 remaining MeshParticle 10개로 진행한다.
