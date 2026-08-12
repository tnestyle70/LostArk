# 2026-08-11 Artist 31470 F High-Ceiling Restoration Final Implementation Plan

기준일: 2026-08-11

참고 materializer checkpoint: `4ffe1102ed9cf3e21f669da292fac1f143e18d8f` — BLOCK, 재사용하지 않고 final Source/Material schema 위에서 다시 작성

계획 갱신 branch: `codex/artist-f-high-ceiling-final-plan-v2`

최종 통합 branch: `codex/artist-f-reconstructed-integration-v1`

최종 통합 시작 commit: `38ebe7cf7dceb5054bde93812907173cc0f98c67`

현재 구현 기준 commit: `18d2b48920b2a327ac59b572960325d352e77a6f`

2026-08-10 사용자 승인: source-era 동일성을 주장하지 않는 `RECONSTRUCTED_APPROVED_V1` 실행 경로를
진행한다. 이 승인은 Source 29행과 Material 255행의 historical evidence blocker를 삭제하거나
`SOURCE_EXACT`로 승격하는 허가가 아니다. 각 행을 stable policy/capability ID, explicit typed value,
fixed-input/output oracle, finite tolerance와 implementation identity에 결합한 뒤 execution blocker만
별도로 해소하는 허가다. R2 immutable candidate/parser/catalog transport는 완료됐고, 선택 Mesh/Sprite
production evaluator checkpoint `18d2b489`는 독립 review 중이다. GPU sink, actual Artist F, Product admission은
아직 열리지 않았으며 `Execute=false, Submit=false, Render=false, Product=false`를 유지한다.

문서 역할: 이 파일은 전체 범위와 G 순서, worktree 소유권, admission predicate를 소유하는 최종
구현 계획서다. 각 mutation lane은 기존 Source/Geometry/Material/Compiler PLAN을 갱신하거나 해당 G의
전용 구현 PLAN을 먼저 만들고, public H/CPP의 세부 선언·전체 코드는 그 lane 문서에서만 소유한다.
같은 코드를 이 master 문서에 복제하지 않는다.

## 2026-08-12 실제 완결 구현 정본

이 절은 2026-08-12 사용자의 실제 Full F 화면 확인과 공유 worktree 인수인계 뒤 다시 고정한
구현 정본이다. 아래의 2026-08-11 M0~M3, historical R/G 절은 denominator와 과거 증거를 보존하는
참고 계약이며, 구현 순서·현재 상태·완료 판정이 이 절과 다르면 이 절을 우선한다.

이번 구현은 계획서만 동결하고 멈추는 작업이 아니다. 현재 dirty tree의 이미 구현된 수직 슬라이스를
보존한 채, 아래 C0~C8을 한 명의 writer가 직렬로 구현하고 실제 Engine/Client build까지 닫는다.
별도 에이전트는 dirty 파일을 수정하지 않고 읽기 전용 P0/P1 검토만 수행한다.

### 사용자가 고정한 최종 목표와 fidelity 경계

최종 목표는 도화가 31470 F의 35 occurrence를 원본에서 확보한 값으로 최대한 동일하게 재생하고,
cooked에서 실제로 소실된 값만 명시적인 reconstructed policy와 사용자 눈 검증으로 조정하는 것이다.
화면에 무언가 보인다는 이유로 exact를 주장하지 않으며, 다음 네 경계를 섞지 않는다.

| 등급 | 의미 | 구현·승인 정책 |
|---|---|---|
| A. 직접 복구 | source export, package decode, WModel/DDS, action cue에서 직접 얻은 값 | 값을 바꾸지 않고 parser -> Program -> CPU/GPU까지 동일 값 소비를 증명한다 |
| B. 동 revision 증거 | 현재 설치 revision의 parent/default/CDO에서 복구한 값 | `CURRENT_REVISION_EVIDENCE`를 보존하고 우선 화면에 적용하되 source-era exact라 부르지 않는다 |
| C. cooked 소실 재구성 | Material graph, static permutation, 일부 sampler/default처럼 원본 수식이 소실된 값 | versioned recipe/family adapter로만 구현하고 수치 oracle와 사용자 occurrence 판정을 함께 기록한다 |
| D. 안전하지 않은 미결 | provider 없음, NaN, 전체 화면/거대 사각형을 만드는 무경계 neutral, 정체 불명 fallback | Full 35에서 조용히 그리지 않는다. occurrence Solo에서 원인을 표시하고 해결 또는 사용자 승인 전까지 BLOCK한다 |

따라서 앞으로의 tuning은 parser·단위·anchor·RNG·Shader 소비 버그를 가리는 수단이 아니다. A/B 값을
먼저 코드가 정확히 소비한 뒤에도 남는 C 영역만 stable occurrence ID의 versioned overlay로 조정한다.

### 2026-08-12 인수 기준선

기준 branch는 `codex/artist-f-complete-visual-runtime`, 기준 HEAD는
`8d76693e03f4c4c0c3939c7b71b9e3de8680441a`다. 이 HEAD 위의 Effect/Animation/Engine/Shader/Tool dirty
변경은 앞선 Artist F 구현 세션의 인계분이며 무작정 reset, checkout, stage, commit하지 않는다.
`Data/Animation/Authored/Warlord/Warlord.animevents`는 이번 Artist F 범위와 무관한 사용자 변경이므로
읽기만 하고 수정하거나 포함하지 않는다.

인수 시점의 실제 상태는 다음과 같다.

| 영역 | 현재 실제 상태 | 판정 |
|---|---|---|
| Full F 화면 | 35행이 CPU/GPU 경로에 도달해 사용자 화면에 검정/흰 호, Sprite, Decal 계층이 보이기 시작함 | first-visible 기반은 존재하나 visual complete 아님 |
| Mesh 단위 | WModel `geometryPreScale=0.01` 한 번, Mesh StartSize dimensionless XZY, Sprite/Decal만 cm->m | 코드 교정됨 |
| `b_wp_1` combined anchor scale | prototype admission `0.0001`과 rig `sdm` root `100`이 합쳐진 실제 combined basis `0.01`을 검증하고 3x3에만 `x100`; 회전·이동 보존 | 실제 Artist bind/`sdm_sk_onestroke` Debug·Release 회귀 CLOSED |
| Tool bone history | side-effect-free arbitrary-tick Model sampler와 Playback 60 Hz transform history가 Tool/seek에 연결됨 | 정적 Tool 경로 교정, 움직이는 gameplay RootWorld history는 남음 |
| Decal | reconstructed-only projection size 단일 소유권, Client Y-yaw, shader unity projection | 구조 교정, material/near/far fidelity 남음 |
| SubUV | random 2x2 3행, linear 6x6 2행 atlas crop/mode | crop 교정, random cadence/RNG 남음 |
| Trail staging | source max point 500을 Engine 512 buffer가 수용 | stage blocker만 닫힘 |
| Light/Post | Light 1, Post 1 typed packet과 제출 통계 구조가 연결됨 | focused denominator/commit 검증 남음 |
| Material runtime | runtime-v2 occurrence 10(`#3/#4/#9/#10/#11/#16/#22/#23/#30/#31`), reconstructed evaluator 17, one-layer 1, neutral fallback 0 | first-draw code/build CLOSED, cooked-lost recipe 수식과 사용자 visual fidelity는 OPEN |
| Product | Restore Full35 nonProduct Debug path | actual Product effectId/effectref/Release publication은 미완료 |
| 사용자 승인 | 화면에 보인다는 관찰은 확보 | formal occurrence `APPROVED`는 0/35 |

### `b_wp_1` combined transform 재발 방지 계약

`Artist.wmodel`의 prototype admission transform과 `CModel::Get_BoneMatrix("b_wp_1")`가 반환하는
combined bone transform은 같은 계층의 값이 아니다. 실제 Artist 경로는 다음처럼 합성된다.

```text
playable prototype admission scale 0.0001
* Artist rig sdm root scale 100
= b_wp_1 combined basis scale 0.01
```

과거 helper는 첫 번째 값 `0.0001`을 combined bone의 기대값으로 잘못 사용했다. 그래서 정상적인
`b_wp_1` basis `0.01`을 invalid transform으로 거부했고, 이를 통과시키려고 `x10000`을 적용하면 반대로
combined basis가 100배 커지는 이중 교정이 된다. 현재 Artist 전용 계약은 다음과 같다.

1. bind pose와 실제 `sdm_sk_onestroke` pose 모두 combined basis 길이 `0.01`을 fail-close로 검사한다.
2. 통과한 matrix의 3x3 basis에만 reciprocal `100`을 적용한다.
3. bone translation, animation rotation, asset orientation과 owner world는 바꾸지 않는다.
4. 관찰한 임의 scale을 자동 정규화하거나 `0.0001 또는 0.01` 두 값을 동시에 허용하지 않는다.
5. `ClientFrontendHarness --effect-reconstructed-gpu-material`은 실제 `Artist.wmodel`, playable
   pretransform, `b_wp_1` bind pose와 `sdm_sk_onestroke` pose를 함께 검사한다.

이 회귀는 Material/Shader packet 검사와 별개다. Full35 준비가 anchor transform에서 실패하면 shader를
조정하지 않고 `prototype admission -> rig root -> combined socket`의 세 계층을 먼저 실측한다.

### 인수 당시 화면이 틀린 직접 원인

현재 가장 큰 시각 결함은 데이터 부족보다 Material runtime의 잘못된 소비다.

1. Program에는 Material family 23, recipe 27, occurrence 34, input 729가 있으나 현재 Full F의 common
   evaluator는 12행, 9 family, 24 texture-lane use만 소비한다.
2. #7~#16/#18의 legacy common 경로는 `Shader_Artist31470Diagnostic.hlsli`의 numeric oracle용 입력 샘플을 production Shader로 재사용한다.
   transformed UV를 계산해도 실제 texture는 원래 `sourceUV`에서 sample하고, distortion bit가 MRT
   `Distortion`이 아니라 color의 `result.xy`를 바꾸며, opacity mask clip과 particle dynamic parameter를
   소비하지 않는다.
3. `Build_PreparedDocument`는 recipe의 `numericBindingSamples.front()`를 실제 recipe 상수처럼 사용한다.
   이 네 sample은 evaluator 검증용 공통 test vector이지 원본 Material instance의 runtime operand가 아니다.
4. #4는 별도 `Shader_Artist31470RuntimeMaterial.hlsli`와 4-lane packet으로 분리해 diffuse/normal/
   specular/dissolve, alpha clip `0.333`, particle color/alpha, inverse-transpose TBN과 실제 sampler를 소비한다.
   `alpha` dynamic channel만 현재 reconstructed mask/dissolve 정책으로 소비하며, cooked graph 수식이 없는
   `bloodline`, `finger_up`, `distortion[0-x]`는 조용히 버리지 않고 suppressed mask `0x0e`로 남긴다.
5. #9~#18의 검정 core, 흰 외피, flow/dissolve Mesh가 서로 다른 recipe인데도 동일한 10-bit feature
   evaluator로 평탄화되어 UV flow와 dynamic dissolve가 원본처럼 전개되지 않는다.
6. Sprite/Decal/Ribbon Shader는 common evaluator packet을 동일하게 소비하지 않으므로 같은 recipe라도
   renderer family에 따라 계산 결과가 달라진다.
7. Renderer와 Presentation의 일부 실패·S_FALSE·zero draw가 상위 frame success로 접힐 수 있어
   `running` 또는 `frame rendered`가 실제 6-family draw 성공을 뜻하지 않는다.

이 항목들은 눈으로 값만 조정하지 않고 아래 수직 슬라이스에서 코드로 닫는다.

### 최종 denominator와 완료 정의

```text
Source execution       schedule 7 / emitter 35 / module 399 / distribution 629
Attachment             follow b_wp_1 5 / snapshot root 30
Geometry               WModel carrier 7 / Mesh use 13
Material               family 23 / recipe 27 / occurrence 34 / input 729
Material input kinds   scalar 578 / vector 30 / texture 121 / static 94
Resolved GPU texture   unique DDS 52 / runtime texture binding 77 / renderer slot 57
Visual families        Mesh 13 / Sprite 16 / Decal 3 / Ribbon 1
Presentation families  Light 1 / Post 1
Visual material rows   33 / Post material adapter 1 / Light builtin 1
User decision          occurrence APPROVED 35 / 35
```

`완료`는 위 분모가 존재한다는 뜻이 아니라 각 행이 `active -> evaluated -> resource prepared -> submitted ->
drawn/presented` 중 어디에 갔는지 정확히 disposition되고, silent fallback·unknown·false-success가 0이며,
사용자가 고정 시간 화면에서 35/35를 승인한 상태다.

### C0. 단일 writer 기준선과 변경 소유권

1. 현재 다른 구현 task는 모든 편집·빌드를 중단하고 이 task에 인계한다.
2. 현재 dirty diff를 파일·함수·denominator별로 읽고 이미 닫힌 수정은 보존한다.
3. reviewer는 frozen commit이 없어도 읽기 전용으로 현재 physical tree의 P0/P1을 보고할 수 있으나,
   구현 파일을 수정하지 않는다.
4. 코드 변경은 현재 task 하나만 수행하며 build도 한 번에 하나만 직렬 실행한다.
5. 사용자 첨부 이미지는 반드시 분석하되 에이전트가 Client/UI를 자율 실행·조작·캡처하지 않는다.

C0 종료 조건은 다른 writer/build 0, 최신 diff ownership 확정, 이 절의 계획서 반영이다.

### C1. 실패 은폐 제거와 exact runtime denominator

첫 구현은 화면 모양보다 성공/실패의 진실성을 닫는다.

1. `Engine/Private/Renderer.cpp`의 Effect `Render_NonLight`와 `Render_Blend`는 각 객체의 첫 FAILED
   HRESULT를 보존하되 전체 queue cleanup은 끝까지 수행하고 최종 failure를 반환한다.
2. `CEffectDocumentRenderer`는 frame마다 family별
   `configured, active, evaluated, candidate, submitted, drawn, suppressed, failed`를 기록한다.
3. frame 종료에서 Element/Particle/Trail cursor가 source 배열 끝과 정확히 일치하는지 검사한다.
   material BLOCK, zero particle, degenerate trail, disabled family는 각각 별도 disposition이며 S_FALSE를
   성공 draw로 세지 않는다.
4. `CPresentation_Manager`는 provider commit 전에 Light/Post 각각
   `expected == attempted`와 `accepted + suppressed == attempted`, `failed == 0`을 확인한다.
5. 뒤 provider failure로 outer frame이 rollback되면 앞 provider의 local `committed=true`도 final manager
   transaction 결과와 함께 false로 표시한다.

C1 종료 denominator는 visual33과 Light1/Post1의 합계가 35와 일치하고, draw 0 또는 submit 0인 행을
`Effect frame rendered`로 보고하는 경로가 0인 것이다.

2026-08-12 구현 checkpoint: Engine renderer가 첫 FAILED HRESULT를 보존하면서 모든 queue를 정리하고,
DocumentRenderer는 visible GPU occurrence의 exact set과 Mesh13/Sprite16/Decal3/Ribbon1 disposition을
검증한다. Presentation Manager는 Light/Post transaction을 전체 provider에 finalize한다. Client
Debug/Release build가 통과했으며, 사용자 수동 visual 판정과 별도 harness는 실행하지 않았다.

2026-08-12 실제 첫 draw 교정 checkpoint: prepared-cache만 검사하던 기존 gate 뒤에 제품과 같은
`CEffectObject -> ObjectManager layer -> fixed-step Playback -> DocumentRenderer Render` 경계를 추가했다.
native-v14에서 format-13으로 낮추는 과정이 typed `Renderer`를 지워 Artist occurrence를 family 없음으로
만들던 문제를 교정하고, 35/35 element를 Program emitter의 renderer/source-space와 exact join한다. 동시에
legacy `Renderer==END` 문서는 kind/geometry binding으로 family를 결정한다. `SourceRecipe.bEnabled`를
legacy fallback 차단 조건으로 쓰면 authored 18문서의 particle/decal 1,300 occurrence가 사라지므로 이를
명시적으로 금지하고 executor fixture와 corpus audit에 고정한다.

최종 자동 first-draw gate는 Artist `t=1.5s`와 Lance BA1 `t=0.5207s`, `+1/60s`를 모두 실제로 draw한다.
Debug 기준 Artist는 `active 28 / candidate 26 / attempted 28 / submitted 25 / suppressed 3 / failed 0`,
Lance Mesh는 두 sample 모두 `1 / 1 / 1 / 1 / 0 / 0`, transaction `committed=1`이다. 이 수치는
process 생존과 실행 계약 증거이며 visual fidelity 승인이나 historical ObjectManager AV 해결 증거가 아니다.

### C2. SNAPSHOT_ROOT와 immutable artifact transaction

현재 builder의 임시 결과는 follow 5 / snapshot 30이지만 배포된 15 MB Program과 Catalog는 follow 5 /
snapshot 0 / disabled 30이다. 코드만 고친 상태를 완료로 세지 않는다.

1. `build_artist_31470_reconstructed_runtime_program.py`는 exact attachment mode를
   `FOLLOW_NAMED_ANCHORS` 5, `SNAPSHOT_ROOT` 30으로 materialize한다.
2. RuntimeAuthority/parser validator는 `enabled=true`를 곧바로 follow-only로 해석하지 않고
   다음 두 tuple만 허용한다.

```text
FOLLOW:   enabled=true, follow=true,  exact named anchor request 1
SNAPSHOT: enabled=true, follow=false, runtime slot=root, anchor request 0
```

3. source-semantics receipt -> custom handler/source oracle/capability -> Program -> material approval ->
   render-resource sidecar -> runtime Catalog -> C++ literal pin을 한 transaction으로 재생성한다.
4. candidate/runtime Catalog는 exact LF checkout byte identity를 유지한다.
5. 실패하면 이전 Program/Catalog/C++ pin 전체를 보존하고 일부 파일만 publish하지 않는다.

C2 종료 조건은 배포 Catalog에서 7/35/399/629와 follow5/snapshot30을 C++ parser가 같은 immutable
Program pointer로 읽고, old legacy 101/non-target entry projection delta가 0인 것이다.

### C3. production CPU sole-authority와 animation history

`m_pReconstructedExecutionPlan`을 저장만 하고 `SourceRecipe` 문자열을 다시 해석하는 현재 구조를
최종 runtime으로 인정하지 않는다.

1. Stage에서 source element/module/distribution을 exact typed Plan row에 1:1 결합하고, Step은 이 결합된
   row의 handler/opcode/property/seed policy를 직접 소비한다. raw class-name alias는 stage validation용으로만
   남기고 production Step의 dispatch authority가 되지 않는다.
2. missing `Spawn.RateScale`은 source 0으로 위조하지 않는다. 현재 Engine CDO identity 1을
   `CURRENT_REVISION_EVIDENCE` reconstructed default로 명시한 뒤 Program에 값과 provenance를 함께 봉인한다.
   현재 정확한 default-dependent 분모는 137이며 RateScale 35만 이 정책으로 READY다. 나머지 102
   (`Spawn.Rate` 1 + non-Spawn 101)는 BLOCKED로 유지하고 property별 근거 없이 0이나 CDO 값으로 일반화하지 않는다.
3. continuous spawn은 `Rate * RateScale * dt`를 정확히 두 distribution draw/order로 계산한다.
4. particle occurrence xorshift state와 seeded module state를 분리한다. ordinary lifetime은 occurrence draw를
   재사용하고 이후 op2/op3은 typed module/property/component order로 particle-local xorshift를 소비한다.
5. Ground 2행은 `skiplocation`을 먼저 평가하고 skip이면 no-op, 아니면 typed XYZ adjust를 더한다.
6. SpawnPerUnit 2행은 이동거리, unitScalar, fractional accumulator, cap을 typed row로 실행한다.
7. random SubUV 3행은 RandomImageTime과 seed policy로 frame 재선택 cadence를 재현한다.
8. Light/Post 1-burst 행도 particle lifetime interval을 사용해 각각 0.2 s / 0.6 s active 구간을 만든다.
9. Tool은 이미 연결된 arbitrary-tick bone sampler를 유지한다. gameplay/late join/seek도 각 1/60 step의
   action time에 RootWorld와 `b_wp_1` pose를 공급하며 현재 frame pose 하나를 과거 전체에 재사용하지 않는다.
10. snapshot30은 schedule edge의 root를 한 번 저장하고 이후 live actor root를 따라가지 않는다.

C3 종료 조건은 Step의 raw SourceRecipe/module dispatch 0, typed module 399/399, distribution 629/629,
follow history5/5, snapshot30/30, unknown/silent ignore 0이다.

### C4. Material/Shader runtime v2 — 최우선 시각 슬라이스

Material은 추출된 729 입력과 reconstructed arithmetic을 분리해 하나의 immutable packet으로 compile한다.

#### C4-A. immutable recipe packet

1. Program의 family/recipe/occurrence/input/static/texture/policy row를 exact ID와 row SHA로 join한다.
2. 현재 27 recipe의 최대 분모는 scalar 50, vector 3, resolved texture lane 5다. production packet은
   sampler/SRV lane 5를 고정 ABI로 사용하고, scalar는 이름/role에 따라 fixed float4 block, vector는 fixed vector block, static switch는
   versioned bit/decision block으로 compile한다.
3. 공통 oracle의 `numericBindingSamples` 네 행은 test input/output 검증에만 사용한다. 첫 sample을
   production constant로 복사하는 코드를 제거한다.
4. 실제 runtime operand는 729 Material input과 recipe의 named binding/role에서 compile한다.
   resolved texture 72는 exact asset/SRV/sampler identity로 stage하고, 미해결 texture input 49는 각 recipe에서
   `unused`, `current default`, `explicit neutral`, `BLOCK` 중 하나로 disposition한다.
5. 23 family의 feature mask와 evaluator ID는 같은 10-op arithmetic core를 사용하더라도 identity와
   operand mapping을 보존한다. 같은 feature mask라는 이유로 서로 다른 recipe 상수를 공유하지 않는다.

#### C4-B. production HLSL evaluator

1. UV scale/rotation/pan을 실제 texture sample 좌표에 적용한다.
2. dynamic parameter 네 채널을 해당 recipe semantic에 연결한다.
3. normal/specular/mask/emissive/dissolve/fresnel/alpha clip을 packet에 존재하는 범위에서 실제로 소비한다.
4. distortion은 SceneColor RGB를 변형하지 않고 `EFFECT_PS_OUT.Distortion` MRT에 기록한다.
5. opacity mask는 recipe clip threshold를 사용하고, alpha/additive/opaque pass와 depth/cull을 occurrence
   composite 정책으로 선택한다.
6. Mesh, Sprite, Decal, Ribbon Shader가 같은 packet/evaluator를 소비한다. family마다 별도 두 번째
   renderer를 만들지 않는다.
7. sampler72는 실제 두 descriptor class를 device resource로 stage해 scoped Set/Apply/readback/Undo로
   적용한다. shader의 기존 이름 기반 static sampler가 exact policy를 대신하지 않는다.

#### C4-C. 우선 occurrence

1. #4 `a4ee...`: diffuse, normal, specular, dissolve, mask clip 0.333을 모두 소비한다.
2. #9/#10 black core, #11 bright outer, #12~#15 flow/dissolve, #17/#18 surrounding Mesh를 recipe별로
   복원해 1.432 s main silhouette를 닫는다.
3. #16/#30/#31은 SubUV와 alpha/mask/dynamic을 결합해 2.8 s의 atlas 사각형 노출을 제거한다.
4. #23은 의도된 horizontal Sprite 크기·방향은 유지하고 procedural center glow/mask를 복원해
   무경계 연보라 사각형만 제거한다.
5. neutral provider는 evaluator 수학상 명시적으로 선택된 경우에만 사용한다. 실제 binding load 실패를
   neutral로 대체하지 않는다.

C4 종료 조건은 Material family23/23, recipe27/27, occurrence34/34, input729/729의 disposition,
visual occurrence33/33 packet 소비, Post adapter1/1, fallback-blocked/unknown 0이다. 이 조건은 source-exact
graph23을 뜻하지 않으며 reconstructed family adapter와 user decision을 별도로 유지한다.

2026-08-12 #4 첫 수직 checkpoint:

- Program/sidecar의 `source-active-004`, recipe `a4ee2b242b08bb39`, family `2c00ce5593538d7c`를 exact
  row identity로 join한다.
- diffuse/normal/specular/dissolve 4 DDS의 byte/SHA/SRV/color-space와 4 sampler descriptor를 검증·stage한다.
- Effects pass `Begin` 뒤 PS sampler slot 5~8에 실제 sampler를 적용하고 draw 뒤 원상복구한다.
- nonuniform particle scale에서는 inverse-transpose normal matrix와 재직교화 TBN을 사용한다.
- `alpha` dynamic은 mask에 소비하고 나머지 3채널은 근거 없는 수식을 만들지 않고 명시적으로 suppressed한다.
- Client Debug/Release full build와 link는 PASS다. 사용자 화면 판정은 PENDING이며 #7~#18로의 일반화는 OPEN이다.

2026-08-12 #23 두 번째 수직 checkpoint:

- `source-active-023`의 `600×400 cm`, cue scale `1.3`, `EPAL_Z` 수평 carrier는 원본값으로 유지한다.
- texture input 0개인 recipe를 white-neutral 2-lane으로 보내던 경로를 제거하고 zero-texture opcode2로
  stage한다. neutral denominator는 5에서 4로 줄었다.
- radius `0.5`, sphere power `4`, sphere strength `1`, fresnel power `1`과 named dynamic
  `uv_sphery`만 bounded radial policy가 소비한다. y-pan/UV distortion/twirl/depth-alpha bias는 원본 식이
  cooked에서 소실돼 명시적 staged-unconsumed/suppressed다.
- 이는 original graph exact 복원이 아니라 `RECONSTRUCTED_VISUAL_POLICY`다. 사용자 판정 전 geometry
  size를 줄이지 않고 radial center/rim만 RETUNE 대상으로 둔다.
- Client Debug/Release, Particle/Mesh Effects11 compile, Release reflection, Program identity와 radial UV
  numeric oracle 5/5는 PASS다. 실제 D3D pixel oracle와 사용자 화면 판정은 PENDING이다.

2026-08-12 #9/#10 세 번째 원자적 수직 checkpoint:

- 범위는 `source-active-009/010` 두 occurrence, recipe `material-recipe-03cc03b86c1a4c8f`, family
  `material-family-89af5c77d8e35f99` 하나로 고정한다. 두 occurrence는 같은 recipe/family/geometry를 쓰되
  occurrence row와 dynamic module/distribution identity는 각각 따로 검증한다.
- recipe 고유 분모는 input 32, static 14, render 6, texture 2다. 두 occurrence packet 분모는 input 64,
  static 28, render 12, texture lane 4이며, dynamic은 module 2 / named lane 8 / literal 8 / distribution 8이다.
- 기존 common diagnostic 식은 `disslove_hardness=2`를 alpha threshold로 잘못 포장했다. normalized texture
  alpha가 1을 넘을 수 없어 #9/#10의 alpha가 항상 0이 되는 것이 직접 원인이었다. opcode3은 synthetic
  `NumericBindingSamples.front()`를 production 값으로 쓰지 않고 named scalar 29, vector 1, exact texture
  lane 2와 dynamic 4채널을 immutable packet으로 stage한다.
- opcode3의 input disposition은 consumed `0xc3ffffe8`, suppressed `0x3c000017`, static은 selected
  `0x33ff`, consumed `0x3f17`, suppressed `0x00e8`, render는 consumed `0x2f`, suppressed `0x10`,
  dynamic은 consumed `0x0f`, suppressed `0`이다. cooked에서 식을 회복하지 못한 camera Fresnel과 세
  out-falloff switch는 소비했다고 세탁하지 않는다.
- 검은 RGB는 source-exact가 아니다. 현재 Program의 current-revision/default-zero StartColor를 유지한
  **bounded reconstructed black-core policy**다. 원본 UE3 graph, source shader cache와 Product evaluator
  authority가 복구되기 전에는 이 식을 원본 수식으로 승격하지 않는다.
- primary authority는 Catalog embedded Program `618d5684...`/raw `72e41774...`/15,072,141 bytes와
  embedded sidecar `bc5cd1ac...`/746,788 bytes다. working candidate와는 emitter snapshot 행만 다르고
  모든 material section은 동일해야 한다.
- 현재 v2 전체 checkpoint는 occurrence 4(`#4/#9/#10/#23`), recipe 3, family 3, unique recipe
  input/static/render/texture `48/20/18/6`, occurrence packet `80/34/24/8`, dynamic module4/lane16이다.
  HLSLI normalized SHA는 `df459b59fbf23e70542fd26afb205643b3050821ee2506ef85faddaa25dbeaff`로
  focused test에 고정했다. Mesh/Particle 부모 shader의 include/branch/call, Catalog ordered field/native JSON
  type, occurrence별 packet branch ABI, 전체 raw bind/texture/dynamic 운반과 sampler slot5 복원도 함께 검사한다.
  scalar/vector/provider lane 순서와 뒤쪽 conflicting 재할당을 놓치지 않도록 세 packet branch, 공용
  texture-stage, bind/transport/sampler source slice의 normalized golden projection SHA도 고정한다.
  occurrence selector/join, stage 성공 flag, caller/count, 최종 `13/4` denominator와 header ABI default도
  별도 wiring projection으로 고정한다.
- focused contract, standalone Effects11 Debug/Release 4/4, Client Debug/Release build/link와 독립 review
  P0=0은 PASS다. GPU pixel/WARP, 사용자 육안, opcode3 전용 Program/receipt/Product authority는 OPEN이다.
- 이 checkpoint 보고 전에는 #16/#30/#31/Decal/Ribbon recipe 확장을 시작하지 않는다. 현재 checkpoint를
  닫은 뒤에도 사용자의 다음 지시 전까지 그 확장은 PAUSE다.

### C5. 여섯 family sink 완결

| family | 구현 계약 | 종료 분모 |
|---|---|---:|
| Mesh | geometryPreScale 한 번, dimensionless signed StartSize, recipe packet, dynamic/normal/dissolve | 13/13 |
| Sprite | cm->m size/location, signed flip, billboard/axis lock, SubUV cadence, recipe packet | 16/16 |
| Decal | projection extent 단일 소유권, Client Y-yaw, near/far/depth, recipe packet | 3/3 |
| Ribbon | cap500, typed lifetime2.0, width0.15 m, color/dynamic, tiling600, tessellation5, historical b_wp_1 points, material | 1/1 |
| Light | typed world transform, color/alpha/intensity/radius/falloff, active 0.2 s, atomic submit | 1/1 |
| Post | zoom profile, intensity/alpha curve, active 0.6 s, atomic submit | 1/1 |

각 sink는 active row가 없으면 0을 정상 기록하고, active row가 있는데 draw/submit이 0이면 frame failure다.

### C6. prepared cache, action-time I/O와 성능

1. Tool과 actual F는 PresentationService의 동일 immutable
   `{CatalogEntry, Program, Document, PreparedDocument, Device}` composite를 사용한다.
2. Play 클릭마다 15 MB Program compile, 2.96 MB source JSON parse/SHA, DDS48/WModel7 prewarm을 반복하지 않는다.
3. action edge와 per-frame에 file I/O는 0이다. first Character Select load에서 CPU prepare와 render-thread GPU
   commit을 분리할 수 있으나, 별도 Tool-private cache를 만들지 않는다.
4. cache miss/identity change는 이전 composite를 보존한 채 새 composite를 stage하고 마지막에 한 번 swap한다.
5. first-use 준비 시간이 존재해도 그 wall time을 animation/effect delta로 넣지 않는다.

### C7. Tool, actual F와 Product publication

1. `All Effects -> Artist -> Skill F Restore -> Play Full F (35)`는 effect-only 복사본이 아니라 실제 Artist
   `sdm_sk_onestroke` animation, root, `b_wp_1`, 같은 prepared composite와 같은 2.833 s clock을 사용한다.
2. complete/family/occurrence Solo와 fixed seek는 production packet/sink를 그대로 사용하며 별도 diagnostic
   Shader로 결과를 위조하지 않는다.
3. actual Server command -> snapshot -> Artist action 경로도 같은 prepared pointer와 CPU/GPU path를 사용한다.
4. Restore와 기존 legacy Product cue는 공존한다. 다른 101 legacy cue와 non-target Catalog row는 delta 0이다.
5. 35 occurrence의 userDecision이 전부 `APPROVED`가 되기 전에는 Product `effectId`, animevent
   `effectref=asset`, Release admission을 publish하지 않는다.
6. Product publication은 cue/effectId/animevent/Catalog/receipt를 한 transaction으로 commit하고 실패하면
   기존 legacy selection과 Catalog revision을 보존한다.

### C8. 검증, 문서와 사용자 수동 판정

사용자가 별도로 허용하기 전에는 ClientFrontendHarness와 full ProjectAudit를 실행하지 않는다. 구현 중
자동 검증은 `git diff --check`, JSON parse, shader compile, changed translation unit compile, Engine/UpdateLib/
Client Debug·Release build와 코드상 exact denominator 검사로 제한한다. 에이전트는 Client/UI를 실행하거나
화면을 캡처하지 않는다.

사용자 수동 검증 시간은 다음으로 고정한다.

| 시간 | 반드시 확인할 계층 |
|---:|---|
| 0.000 s | 초기 Sprite 3 + Ribbon 1, 무기 anchor 시작 |
| 1.340 s | #4 weapon Mesh 출현과 b_wp_1 궤적 |
| 1.432 s | #9~#18 main black core, white outer, flow/dissolve |
| 1.656 s | hit Sprite, Decal3, Post, Light 합성 |
| 2.800 s | #16/#30/#31 잔류 Sprite와 dissolve, 사각형/atlas 노출 여부 |

각 observation은 `.md/GB/이펙트최종추출.md`에
`occurrence / 원본·증거 tier / parser 값 / CPU 값 / GPU packet / 실제 관찰 / 수정 / userDecision` 순서로
추가한다. 코드 완료와 사용자 visual 승인, Product publication을 별도 상태로 기록한다.

### 확장 전 gate

Artist F C0~C8이 끝나기 전 다른 4-class/Valtan에 Artist 전용 ID, count, switch를 복제하지 않는다.
확장 시 재사용 가능한 계약은 다음이어야 한다.

- effect별 manifest가 35/23/27 같은 분모와 함께 `prototypeAdmissionScale`, `rigRootScale`,
  `combinedAnchorScale`, `normalizationReciprocal`을 서로 다른 typed transform 계층으로 제공한다.
- raw package/export/serial SHA와 provenance tier가 각 field에 남는다.
- unit·basis·anchor·RNG·material operand·sampler·state·sink disposition이 generic packet으로 표현된다.
- 새 semantic이 나오면 fallback으로 접지 않고 shared capability를 먼저 추가한다.
- runtime C++에 class명·skillId·Valtan action별 새 public API를 추가하지 않는다.

## 2026-08-11 고점 복원 최종 결정 정본

이 절은 현재 구현 상태, Effect Tool 정책, 세션 운영과 M0~M3 순서의 최상위 정본이다. 아래 historical
R0~G11 기록과 기존 M0 문구가 이 절과 충돌하면 이 절을 우선한다. 기존 evidence와 frozen SHA는 삭제하지
않되, 과거 상태 문구인 `R2 IN_PROGRESS`, `exact10 only`, `production evaluator NOT IMPLEMENTED`,
`R3-R8 GATED`, 상시 3세션 병렬 구현은 현재 판정으로 사용하지 않는다.

### 바뀌지 않는 최종 목표

- 수동 근사치로 적당히 보이게 만드는 데서 멈추지 않고, 동일 generic pipeline으로 도화가 F 35/35를
  실제 Client에서 실행하고 occurrence별로 눈 검증·수치 환류한 뒤 4개 class와 Valtan까지 확장한다.
- historical source-era 동일성은 주장하지 않는다. 결과는 끝까지 versioned reconstructed restoration이며
  `sourceExactAdmission=false`를 보존한다.
- 사람이 보는 결과는 필수 종료 증거지만 유일한 admission 근거는 아니다. 자동 numeric/structural gate를
  먼저 통과하고, 수동 관찰을 stable occurrence fixture와 tuning overlay로 환류한 뒤 다시 자동 gate를
  통과해야 한다.
- 첫 픽셀, 35/35 nonProduct, 실제 F Debug, Product D/R을 서로 다른 milestone로 판정한다. 앞 단계 화면이
  보였다는 이유로 뒤 단계 admission을 열지 않는다.

### 현재 exact 기준선

| 영역 | exact 상태 | 현재 판정 |
|---|---|---|
| integration | `18d2b48920b2a327ac59b572960325d352e77a6f`, clean/pushed | 이 계획과 fresh 구현 세션의 기준 |
| LF exact13 producer/parser/catalog | `ab559784a7fa0c5d19ccedf600d08f8a9ee50a25` | PASS. fresh-LF exact13 `27,065,827 / ea3afd4e...`, old10 `26,255,930 / b5086d14...` |
| immutable runtime program | 7 schedule / 35 emitter / 399 module / 629 distribution | candidate/parser/catalog/CPU plan compile 완료, 모든 admission false |
| selected evaluator | `18d2b489`, Mesh 1 + Sprite 1 / module-handler consumption24 / unique route15 | 코드와 public API는 존재하나 독립 review 미종료, production caller 0이므로 admission 0 |
| offline resource evidence | WModel 7/7, DDS 48/48, binding 72/72 | bytes와 offline authority만 PASS. immutable GPU composite와 renderer readiness는 0 |
| renderer sinks | Mesh/Sprite/Decal/Ribbon/Light/Post | production typed sink 0/6, first pixel 0/1 |
| Effect Tool typed path | exact CatalogEntry -> evaluator -> GPU composite | 미연결. 기존 legacy Product mode만 존재 |
| actual Artist F | `PlayerSkills 31470 effectId=""`, original animevent refs | Debug diagnostic와 Product 모두 미연결, 0/35 |
| admissions | Execute / Submit / Render / Product | 모두 false. 현재 상태에서 하나라도 일반 경로를 열면 empty/no-pixel success가 될 수 있으므로 금지 |
| expansion raw inventory | `9b7ac2dc571cc456fa6095f8f69b2df0ea54bd53` | extraction/inspection 완료, Artist F GPU schema 동결 전 runtime binding으로 승격 금지 |

`18d2b489`의 최대 인정 범위는 evaluator review가 PASS한 뒤에도 Mesh/Sprite packet 2/35와
module-handler consumption 24/399, unique route 15다.
GPU draw, Tool caller, actual F, Product 완료를 뜻하지 않는다. 현재 main worktree의 Manual Restoration
Workbench dirty WIP도 별도 보존 대상이며 이 integration 기준선이나 runtime authority가 아니다.

### Effect Tool과 기존 effect 정책

Effect Tool은 새 도구를 만들지 않고 기존 `CEffect_Tool` 하나를 두 mode로 확장한다.

| mode | 입력 권위 | 허용 작업 | 금지 |
|---|---|---|---|
| Legacy Product | 기존 v13 Authored/Assembly와 legacy prepared document | 기존 effect의 inspect/play/edit 흐름 유지 | typed Artist F로 자동 변환, 기존 cue 일괄 교체 |
| Typed Restoration | 같은 immutable CatalogEntry pointer의 compiled program/resource authority | complete/family/occurrence solo, seek, read-only Source/IR inspection, tuning overlay 편집, validation 실행 | Source/IR Save·Apply, disk reread, 두 번째 renderer/runtime, legacy fallback |

기존 effect는 Artist F 개발 때문에 바꾸거나 재publish하지 않는다. 101은 4-class corpus의 기존 Product
cue 분모이며 전체 Catalog effect 수와 같은 뜻이 아니다. typed migration은 Artist F가 M3를 통과한
뒤 `Artist 9-skill canary -> DimensionMaster -> Warlord -> LanceMaster` 순서로 cue 하나씩 atomic하게 수행한다.
한 cue의 typed stage/prewarm/validation이 실패하면 그 cue의 legacy Product를 그대로 보존한다. 101 cue를
한 번에 변환하지 않는다.

fresh 구현 세션의 step 0에서 어떤 M0 코드도 수정하기 전에 plan parent `18d2b489`의 exact runtime
Catalog로 `legacy-product-cue-projection-v1`을 만들고 독립 review로 동결한다. 이후 새 tree에서 projection을
다시 만들어 baseline을 바꾸지 않는다. 4-class 기존 Product cue
101개 각각의 cue ID, effect asset ID, payload kind, canonical Catalog entry SHA, Authored/Assembly identity,
prepared mode를 기록하고 전체 non-target Catalog entry set도 canonical projection으로 고정한다. Artist F
publication 전후 old101은 ID set과 row SHA가 101/101 exact-equal이어야 하고, 그 밖의 non-target entry도
set/row projection delta 0이어야 한다. M0~M3의 각 frozen checkpoint에서 이 baseline과 delta를 검사한다.
Debug/Release에서 Legacy mode의 101/101 load/prepare와 대표 actual
playback, Legacy↔Typed mode 전환 실패 rollback을 검증한다. 새 Artist F entry의 stage 또는 mode switch가
실패하면 기존 Catalog snapshot, legacy prepared pointer와 Tool selection을 모두 보존한다.

Source Contract와 immutable IR은 read-only다. 사람이 조정할 수 있는 것은 stable occurrence ID를 key로 한
별도 versioned tuning overlay뿐이다. overlay는 숫자/curve/visibility timing 같은 승인된 조정 field만
소유하고 asset ID, resource SHA, renderer family, handler/opcode, owner, source identity를 바꾸지 못한다.
main worktree의 미커밋 Workbench는 삭제하지 않되 M0 first pixel 뒤 UI/queue 구현으로만 재감사하며,
runtime authority나 Artist F 구현 branch에 그대로 cherry-pick하지 않는다.

overlay authoring path는
`Data/Effects/AuthoredCorrections/Artist/effect.artist.skill.31470.tuning-overlay.json` 하나다. 저장 계약은
`lostark.effect-restoration-tuning-overlay` formatVersion 1로 동결한다. root는 정확히
`schema, formatVersion, effectAssetId, baseProgramId, baseProgramVersion, baseProgramSha256, entries,
canonicalProjectionSha256, overlaySha256`를 가지며, entry는 exact stable occurrence ID, approved parameter ID,
typed finite value/curve, rationale와 user decision만 가진다. unknown key, bool-as-number, nonfinite, duplicate
occurrence/parameter, base Program mismatch를 거부한다.

Tool의 Save는 active runtime을 직접 바꾸지 않는다. temp overlay를 `parse -> validate -> canonical seal ->
offline compile -> new Program/receipt -> Catalog stage -> resource prewarm -> equivalence -> atomic commit`한 뒤에만
새 revision을 선택한다. 실패하면 기존 overlay file, compiled Program, Catalog revision, prepared pointer와
GPU composite를 모두 보존한다. overlay SHA는 compiler input receipt와 compiled Program canonical projection에
들어가며 exact13이 pin한 Program SHA를 통해 publisher/runtime identity에 결합한다. action-time/per-frame
overlay file read, runtime patch, Program SHA를 유지한 in-place tuning은 0이어야 한다.

### 사람 눈 검증과 occurrence 튜닝 계약

사용자가 유일한 화면 조작자이자 최종 visual reviewer다. 에이전트는 Client나 Effect Tool UI를
자율적으로 실행·조작하지 않고 화면을 직접 캡처하거나 스크린샷을 만들지 않으며 visual fidelity를
대신 판정하지 않는다. 사용자가 대화에 첨부한 스크린샷이나 이미지를 분석해 달라고 요청하면
에이전트는 반드시 열람·분석하고, 관찰된 결함과 가능한 occurrence 진단을 보고한다.
에이전트는 빌드, 구조화된 로그와 수치 진단, 실행 준비까지만 수행하고 Server CMD/Client 상태와
사용자가 누를 정확한 경로를 전달한 뒤 멈춘다. 사용자의 서면 판정 전에는 `manual first pixel`,
`eye smoke`, `visual PASS`, occurrence 승인을 완료로 기록하지 않는다. 완성·복원·시각 검증을
요청한 일반 문장은 Client/UI 자율 실행·조작이나 화면 캡처 권한이 아니다. 요청받아 분석한 사용자
첨부 이미지는 진단·리뷰 입력이지만 최종 visual PASS나 단독 admission 증거가 아니다.

외부 reference root는
`C:/Users/user/Desktop/로스트아크이펙트이미지`이며 차원술사 기준 PNG는
`C:/Users/user/Desktop/로스트아크이펙트이미지/차원술사`에 있다. 현재 이 폴더의 PNG는 68개이며,
`차원술사_복원`의 기존 비교 자료와 함께 사람이 형태·타이밍·색·밀도·궤적을 판단하는 참고 자료로만
사용한다. Git 정본, source-exact 증거, 자동 screenshot oracle로 승격하지 않는다.

수동 관찰 한 건은 최소 다음 key로 기록한다.

```text
catalogRevision / compiledProgramSha256
skillId / cueId / occurrenceId / rendererFamily
fixed sample time 또는 seek time / camera·attachment 조건
referenceImagePath 또는 사용자의 known-form 설명
defect category: transform, scale, timing, spawn, color, alpha, UV, material, trail,
                 decal, light, post, missing, extra 중 하나
expected observation / actual observation / approved tuning field
numeric or structural fixture added / Debug recheck / Release recheck
userDecision: APPROVED, RETUNE, BLOCKED
```

35 occurrence는 각각 사용자가 `APPROVED`해야 한다. 전역 scale·brightness·speed로 여러 occurrence를
한꺼번에 덮지 않고 complete -> family -> occurrence solo와 seek로 원인을 좁힌다. 조정 뒤에는 해당
occurrence fixture와 전체 35/399/629 회귀를 모두 다시 실행한다.

### fresh 세션 운영 결정

Artist F M0부터 M3까지 shared C++ 구현은 **fresh 구현 세션 하나**가 직렬 소유한다. Catalog, evaluator,
Object, Renderer, Shader, Effect Tool, Presentation을 여러 구현 세션에 나누지 않는다. 별도 reviewer는
구현 중 live dirty diff를 따라가지 않고 commit/push된 frozen SHA가 생겼을 때만 read-only로 P0/P1을
판정한다. raw corpus specialist는 Artist F interface가 M3에서 동결된 뒤 R8에서만 다시 병렬화한다.

세션 간 협업은 main-local ignored 규칙
`C:/Users/user/Desktop/LostArk/.md/GB/gotchas.local.md`를 따른다. 이 파일은 새 worktree에 자동 checkout되지
않으므로 fresh task prompt에도 동일 규칙을 전문이 아닌 실행 가능한 요약으로 반드시 넣는다. 다른 task에 메시지를 보내기 전에
대상 task의 최신 exact SHA, dirty ownership, 현재 단계와 blocker를 먼저 읽고 `ADD`, `CORRECT`, `STOP`,
`REVIEW_EXACT_SHA` 중 하나로 분류한다. 상대 진행을 읽지 않은 일반 조언, live WIP 재구현, reviewer의
직접 수정은 금지한다.

fresh 구현 세션의 규칙:

1. 이 plan-only commit을 parent로 clean worktree를 만들고, 코드 수정 전에 `18d2b489` old101/non-target
   Catalog projection을 생성·독립 동결한다.
2. 한 번에 milestone의 검증 가능한 한 checkpoint만 구현하고 commit/push한다.
3. 보고는 `base SHA / exact commit / owned files / actual tests / blocker delta / next entry`만 보낸다.
4. reviewer PASS 전 다음 admission을 열지 않는다. P1은 같은 branch의 corrective commit으로 닫고 새 SHA를 재감사한다.
5. acceptance와 직접 연결되지 않은 새 receipt, wrapper, audit framework를 추가하지 않는다.
6. 모든 fresh checkout에서 candidate/catalog LF byte identity를 재검증한다. 오래된 CRLF-smudged worktree는
   증거로 사용하지 않는다.
7. dirty main worktree의 source/data/build output은 읽지 않는다. 유일한 예외는 AGENTS 계약상 팀장이 직접
   관리하는 physical runtime input `C:/Users/user/Desktop/LostArk/Client/Bin/Resources`다. 이 root는 M0~M3에서
   read-only로만 사용하고 selected asset의 Resources-relative path/size/SHA를 compiled authority와 대조한다.
   배포·교체·정리는 별도 explicit transaction 없이는 금지한다.

### 최종 milestone과 순서

| milestone | 구현·검증 범위 | 종료 상태 |
|---|---|---|
| M0-A | `18d2b489` evaluator 독립 판정과 필요 corrective, harness-only request 482줄을 Program/sidecar-derived generic production diagnostic factory로 factor | same CatalogEntry, Mesh/Sprite packet 2/2, module-handler consumption24/unique route15, empty/failure rollback PASS; 일반 admission false |
| M0-B | 1 WModel/preScale, 4 DDS same-byte load, 2 material packet, exact sampler Set/Apply/readback/Undo, Mesh/Sprite typed sink | actual GPU draw 2/2, pipeline stats nonzero, D3D ERROR/CORRUPTION 0, sink 2/6; 일반 admission false |
| M0-C | 기존 Effect Tool의 Typed Restoration diagnostic/solo에서 actual Debug Client first pixel | 사용자가 실제 HWND를 확인하고 occurrence 026/027 결과를 기록; Product false |
| M1 | production evaluator를 7/35/399/629로 일반화, Geometry 7/7, Material 23 family/27 recipe/34 occurrence, DDS48/sampler72/state-input46 exhaustive disposition, output visual-composite33, six sink, provider-local Light+Post atomic submit | full 35 nonProduct standalone, sink 6/6, state46/46·visual33/33, unknown/silent fallback 0; Execute/Submit/Render seam은 각 proof만 별도 개방, Product false |
| M2 | actual Server command -> snapshot -> Artist 31470 animation action의 Debug-only nonProduct typed route, 같은 CatalogEntry pointer, Tool complete/family/occurrence solo·seek, 관찰→overlay/fixture→재compile 반복 | 실제 F Debug에서 35 occurrence 모두 사용자 `APPROVED`; `RETUNE/BLOCKED`가 하나라도 있으면 M2 안에서 반복. Product Spawn과 Release Product route는 false |
| M3 | M2 approved overlay/program을 변경 없이 Debug/Release full regression하고 Product cue/effectId/animevent/format3 publication과 rollback을 한 transaction으로 commit | Source 35/399/629, Geometry 7/7, Material 27/34, renderer 13/16/3/1/1/1, old101/non-target delta0, Product 35/35, 실제 F D/R eye smoke. 새 visual defect는 M2로 되돌림 |
| R8 | frozen generic interface에 4-class와 Valtan corpus 적용 | class/count production 분기 0, 새 의미가 나오면 근사하지 않고 shared capability gate로 복귀 |

진행률은 코드 줄 수가 아니라 아래 exact denominator로만 계산한다.

| 시점 | occurrence/module | material/geometry/resource | renderer/visual |
|---|---|---|---|
| 현재 `18d2` | admitted 0; review 후보 packet2/35·module-handler consumption24/399·unique route15 | recipe2/27·material occurrence2/34·carrier1/7·DDS4/48 후보 | sink0/6, first pixel0 |
| M0 PASS 뒤 잔여 | occurrence33·module375. distribution/property는 slice 차감 근거가 없어 629/1,434/1,572 전체 gate 유지 | recipe25·material occurrence32·carrier6·DDS44·sampler68 | Mesh12·Sprite15·Decal3·Ribbon1·Light1·Post1, sink4 |
| M1 PASS | 35/399/629와 property/leaf 전체 consumed 또는 verified-irrelevant | Material27/34·Geometry7·DDS48·sampler72·state input46/46 exhaustive disposition | sink6/6·visual occurrence composite33/33 |
| M3 PASS | actual cue7/7·occurrence35/35 | 같은 frozen tree에서 resource/overlay/publish identity 일치 | renderer13/16/3/1/1/1·Product35/35 |

현재 evaluator 뒤 M0~M3는 “몇 함수만 연결”하는 규모가 아니다. 중복을 제외해도 약 36개의
production/data/pipeline path, harness/audit/project/result를 포함하면 현실적으로 40~46 tracked file이
영향받을 수 있다. 이 수치는 scope를 늘리는 목표가 아니라 작은 검증 단위로 나눌 현실적 하한이며,
milestone denominator가 닫히지 않으면 파일 수나 경과 시간으로 완료를 주장하지 않는다.

M2의 actual F는 Debug-only nonProduct diagnostic route다. 현재 Product `Spawn`을 거짓으로 열지 않고 실제
Server/animation timing을 사용하되 Release에는 포함하지 않는다. `PlayerSkills.effectId`와 generated Product
`effectref=asset`의 최종 commit은 M3 atomic publication에서만 수행한다.

M2는 한 번 재생하고 종료하는 단계가 아니다. `RETUNE`이면 승인된 overlay field만 수정해 새 Program SHA를
만들고 자동 gate와 실제 F Debug를 다시 실행한다. 35개가 모두 `APPROVED`가 될 때까지 M2를 벗어나지
않는다. M3는 approved Program의 Release/Product 재현 단계이며 M3에서 값이나 overlay를 즉석 수정하지
않는다. 새 차이가 나오면 publication을 rollback하고 M2로 되돌아간다.

Light/Post atomicity는 한 Artist provider가 만든 mixed batch 내부에서만 보장한다. 전체
`Presentation_Manager::Submit_FrameProviders`의 다른 provider까지 rollback하는 global Option-B 변경은 이
작업 범위가 아니다. provider failure 시 whole-frame을 비울지 다른 provider를 보존할지는 별도 Engine
계약으로 남기며 Artist F completion claim에 섞지 않는다.

sampler/state는 이름 기반으로 기존 FX11 backing을 신뢰하지 않는다. offline policy의
`W=WRAP, MaxAnisotropy=0, MinLOD=0`과 현재 Effects11 backing의
`W=CLAMP, MaxAnisotropy=16, MinLOD=-FLT_MAX` 불일치를 보존한다. M0/M1은 approved D3D sampler를
`SetSampler -> Apply -> effect/PS readback -> draw -> UndoSetSampler`로 scope하고 모든 실패 경로에서
원상복구한다. `single mip`을 full descriptor equality로 사용하지 않는다. render state는
`family + blend + two-sided + disable-depth + pass`의 visual occurrence composite로 결정하며 ScreenPost를
material D3D state denominator에 섞지 않는다.

### R8 raw corpus 재사용 계약

raw extraction은 다시 하지 않는다. latest frozen inventory `9b7ac2dc571cc456fa6095f8f69b2df0ea54bd53`의
4-class `51 skill / 74 stage / 113 clip / 5,232 occurrence`, Valtan typed target `8,753`, combined unique raw
request `1,212`를 재사용한다. inspected `1,208`, TGA blocker 4와 provenance/corpus blocker 18의 합계 22를
R8 진입 전에 독립 review하고 닫는다. Artist F의 sampler/state/material/geometry schema가 M3에서 동결된 뒤
한 번만 binding/materializer를 재생성한다. R8 순서는 `Artist 9-skill -> DimensionMaster -> Warlord ->
LanceMaster`, 이어서 `Valtan Particle -> Decal -> Trail -> Material -> Camera`다. 공통 handler가 없으면
값을 근사하거나 class switch를 넣지 않고 해당 capability를 generic pipeline에 추가한 뒤 다시 진행한다.

## 2026-08-11 Integration/M0 Captain 재개 계약

이번 재개는 최종 통합 branch `codex/artist-f-reconstructed-integration-v1`의 exact frozen HEAD
`18d2b48920b2a327ac59b572960325d352e77a6f`에서 시작한다. 아래 M0는 기존 R2~R8 전체 복원
계획을 대체하거나 Product admission을 앞당기는 단계가 아니다. 같은 immutable CatalogEntry에서
선택한 Mesh 1행과 Sprite 1행만 production evaluator와 새 typed Mesh/Sprite sink에 연결해
실제 Debug 창의 첫 픽셀을 확인하는 좁은 nonProduct checkpoint다.

### 재개 기준선

| 항목 | frozen 상태 |
|---|---|
| R2 fresh-LF candidate/parser | PASS. candidate 7/35/399/629와 LF identity 재사용 |
| Catalog | strict historical old10/current exact13, immutable CatalogEntry Program/sidecar pointer/revision PASS |
| CPU | typed execution Plan compile/semantic projection PASS |
| production evaluator | `18d2b489` 선택 Mesh/Sprite 구현, 독립 review 중, production caller 0 |
| GPU sink | 0/6 |
| actual Artist F action | NOT REACHABLE |
| 일반 admission | Execute=false, Submit=false, Render=false, Product=false |

### M0/M1/M2/M3

| milestone | 범위 | 종료 조건 | fresh captain sequence |
|---|---|---|---|
| M0 | exact CatalogEntry의 선택 Mesh/Sprite 2행, production packet, production GPU sink, Debug diagnostic/solo first pixel | 아래 M0 acceptance와 자동 gate 및 사람의 실제 창 확인 | **CURRENT** |
| M1 | Mesh/Sprite 외 Decal/Ribbon/Light/ScreenPost를 포함한 production evaluator와 GPU sink 6/6, sampler72/state46/visual33 closure | 여섯 family packet/sink와 전체 sampler/state rollback | NEXT, same captain |
| M2 | 7 cue/35 occurrence actual Artist F Debug-only nonProduct route, provider-local Light/Post atomic batch와 사용자 튜닝 반복 | actual F Debug 35/35, 같은 CatalogEntry, occurrence별 user APPROVED | NEXT, same captain |
| M3 | approved Program의 Debug/Release eye smoke, full regression, Product Catalog/admission | 기존 최종 `35/35, atomic cue publish`, old101/non-target delta0 | FINAL, same captain |

M0에서 일반 Execute/Submit/Render/Product admission은 계속 false다. M0의 완료 문구는 다음 한 줄만
사용하며 이보다 넓은 복원, renderer, actual F 또는 Product 완료를 주장하지 않는다.

```text
Artist 31470 nonProduct Debug M0 selected Mesh/Sprite 2/2 production packet and manual first-pixel PASS
```

### M0 acceptance 동결

1. publisher source exact4와 runtime exact13 생산자, C++ Catalog old10/new13 strict consumer는
   `ab559784`에서 PASS했다. fresh 구현 세션은 이 identity를 재생성하지 않고 fresh-LF checkout에서
   raw byte/hash와 Debug/Release focused harness를 재검증한다.
2. new13 CatalogEntry는 기존 Program과 새 render-resource sidecar를 같은 immutable entry가 강하게
   소유한다. Preparation, Plan, Object, Renderer가 이 entry pointer 하나를 공유하며 두 번째 catalog,
   loader, runtime을 만들지 않는다.
3. `18d2b489` production evaluator는 `LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS`와
   `CEffectReconstructedCpuInspector` 밖에 구현됐다. 독립 review를 먼저 끝내고, harness에만 있는 exact
   selected request factory를 Program/sidecar-derived generic production diagnostic authority로 factor한다.
   production C++에 Artist/31470/emitter hash/row SHA/count를 복제하지 않고 stable selector는 typed
   diagnostic input으로만 받는다. synthetic legacy
   Document/packet과 action-time candidate/sidecar reread는 계속 금지한다.
4. Debug F1 Effect Tool의 기존 world-preview Object가 diagnostic/solo orchestrator가 된다. 이 좁은
   M0 경로만 production evaluator와 production Mesh/Sprite sink를 명시적으로 호출한다. Object,
   Playback, Renderer의 일반 admission 함수와 일반 action 경로는 계속 false다.
5. 자동 판정은 nonempty packet 2/2, 실제 draw/pipeline statistics, D3D11 debug
   ERROR/CORRUPTION 0, 지정 rollback만 사용한다. 이미지 비교, 자동 이미지 oracle, screenshot oracle은
   사용하지 않는다. 마지막 판정은 사람이 x64 Debug Client의 실제 HWND를 눈으로 확인해 기록한다.
6. M0 rollback은 Catalog load, evaluator empty/failure, GPU composite stage, revision/device mismatch,
   sampler Undo에만 추가한다. 전역 Presentation transaction, 타 family rollback은 M1/M2로 남긴다.

### exact10/new13 Catalog 계약

source Artist 행은 기존 세 필드 뒤에
`reconstructedRenderResourceAuthorityPath`를 추가한 exact4다. runtime consumer는 outer 전체 key 순서가
exact10 또는 exact13일 때만 분기하며 optional-field 방식으로 받지 않는다.

```text
old outer exact10
  payloadKind, effectAssetId, artifactRevision, compilerRevision,
  sourceExact, runtimeExecutionAdmission, productAdmission,
  publishReceiptSha256, publishReceipt, reconstructedRuntimeProgram

new outer exact13
  old exact10 +
  renderResourcePublishReceiptSha256,
  renderResourcePublishReceipt,
  reconstructedRenderResourceAuthority

new link exact21
  schema, formatVersion, encoding, effectAssetId, programId, programVersion,
  programSha256, sidecarSchema, sidecarFormatVersion, sidecarAuthorityId,
  sidecarDecisionProjectionSha256, sidecarReceiptSha256, sidecarRawSha256,
  sidecarByteCount, sourceExact, runtimeExecutionAdmission, executeAdmission,
  submitAdmission, renderAdmission, productAdmission, sidecarUtf8Json

new receipt exact26
  schema, formatVersion, receiptRole, payloadKind, effectAssetId,
  artifactRevision, compilerRevision, sourceExact, runtimeExecutionAdmission,
  executeAdmission, submitAdmission, renderAdmission, productAdmission,
  programId, programVersion, programSha256, baseRuntimeEntryProjectionSha256,
  reconstructedRuntimeProgramSha256, basePublishReceiptSha256,
  renderResourceAuthorityLinkSha256, sidecarRawSha256, sidecarReceiptSha256,
  sidecarDecisionProjectionSha256, toolDependencies,
  receiptSha256Domain, receiptSha256

tool row exact4
  role, path, hashDomain, sha256
```

old10 첫 열 필드는 그대로 canonical projection해 기존 identity를 다시 검증한다.

```text
old outer10 canonical  e9694f000a50a426386afd6ff8f65b4a2a5fcafe9883860efff9103e1fff82d2
old link16             74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2
old receipt self       5c91709f2f0ec855c54c94e6dad5bcd7ed048c6133ca9a9af7d4873f20da1bd3
old receipt full       92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94
```

embedded sidecar는 `sidecarUtf8Json` bytes를 한 번만 parse한다. exact root19, owner
`ARTIST/31470/F`, program/publisher tuple, 모든 false admission, `actionTimeIoAllowed=false`, 전체 self
projection과 decision projection을 검증한다.

```text
sidecar authority      ARTIST_31470_RECONSTRUCTED_RENDER_RESOURCE_AUTHORITY_V1
sidecar bytes          746788
sidecar raw SHA        bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff
sidecar self SHA       bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a
sidecar decision SHA   4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412
program SHA            618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b
candidate raw SHA      72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849
```

old10 entry는 sidecar pointer가 null인 inspection-only entry로 계속 load된다. new13만 non-null typed
sidecar를 가진다. 실제 tracked exact13 publish는 producer source/test와 C++ Debug/Release harness가
temporary exact13 output을 함께 통과한 뒤 마지막에 atomic replace한다. 중간 실패는 기존 snapshot의
revision/status/all maps/entry/Program/sidecar pointer를 모두 보존한다.

### M0 immutable composite identity

M0 GPU composite는 다음을 하나의 staged object에 결합하고 전부 맞을 때만 atomic commit한다.

```text
shared_ptr<const CatalogEntry> exact pointer
Catalog revision
Program ID/version/SHA and candidate raw SHA
sidecar authority/raw/self/decision SHA
selected schedule/emitter/occurrence/recipe/texture/geometry/state row ID + row SHA
shader tracked identity and pass ID
exact ID3D11Device COM identity and diagnostic adapter LUID
production evaluated packet and all staged GPU resources
```

draw 직전 현재 Catalog lookup의 entry pointer/revision/hash와 composite의 값, renderer의 D3D device를
다시 비교한다. 하나라도 다르면 draw하지 않고 기존 composite를 보존한다. device context pointer는
저장 identity로 사용하지 않는다.

### M0 stable selection과 sample 동결

선택은 같은 action schedule과 같은 CatalogEntry를 사용한다.

```text
schedule ID       action-schedule-daa8fc7a3723b850ca9579f2
schedule row SHA  9d716d70a77a810f5c72e05084e3b6b9bfa0e0f5a823463ecc8af9a46352ea9f
source cue        skill-31470/clip-000/notify-022
global time       1.4506419897079468
fixed clock       60 Hz
sample step       88
sample time       1.4666666666666666
local time        0.016024676958719786
spawn serial      0
occurrence suffix ::occurrence:0000000000000000
```

Mesh는 emitter order 26이다.

```text
emitter ID        fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_17
emitter row SHA   23a6519e750f5fcdd22bd5e3f8ebd5ea63427f47440c3f2f74d60213d9977ddb
occurrence ID     source-active-026
occurrence SHA    abbd647d1068f2b17a321214c4608fe9ceb56c78aa26170299cdf13ac90190cc
recipe ID         material-recipe-4b4c59364690a66d
recipe row SHA    ef66bad94bd48c14d421ea5ae32e0fbb0dfe8cf84a4efe9aca8eeb28ac670fbf
family/evaluator  material-family-5fc89efe09353236 /
                  reconstructed-evaluator-c3ac12f104b50f06 v1
geometry use      <emitter ID>::geometry-use
geometry use SHA  ecdec710c4ca2ded8253b771936a8c1b123f694613316ad4ae705a81dddd5b71
carrier ID        geometry-carrier-d98b591aa386ac0dd11f
carrier row SHA   5b800463ed5278e9afca3b4867a661281630a131ead6987a0dd0c0baafad8744
model asset       Effect/Artist/Meshes/fm_a_stone_001.wmodel
model raw SHA     eb08b11e4631938f93b896d9ebf9e7f25d22492094dcf69de443080d5c111c54
geometryPreScale 0.01, vertex와 local bounds에 exactly once
```

Mesh texture/state 선택은 다음과 같다.

| 역할 | stable 선택 | asset / raw SHA |
|---|---|---|
| recipe texture decision | `recipe-texture-binding-06` | row `b2604680e40023ff1ef5efcbaad9e2e6a193fed6b5a70c133c31eaa87f960393` |
| texture0/base | `renderer-material-input-binding-048`, `render-binding-14`, sampler `material-reconstructed-policy-515a17c3340198bdcf21` | `Effect/Artist/Textures/fx_a_environ_002.dds` / `cff398ace89a994c044fcce3736beaa3215cb54b99b1acc105c0c2304ce55962` |
| texture1/normal | recipe가 명시 선택한 `render-binding-15`, sampler `material-reconstructed-policy-ae06d5776b669f2578ec` | `Effect/Artist/Textures/fx_a_environ_002_n.dds` / `62f18a7c49165a62a04525f5954b9c5f494a48ae68b6ce0b9ecc57803ebe63c6` |
| blend | `render-state-descriptor-12` | `BS_EffectOpaque` |
| raster | `material-reconstructed-policy-4ce70dabfda6cdc8633f` | `RS_Default` |
| depth | `material-reconstructed-policy-5c910249033a5d8fcd03` | `DSS_Default` |
| shader/pass | `Shader_VtxEffectMeshPreview.hlsl` | pass 0 `OpaqueBackDepthWrite` |

Sprite는 emitter order 27이다.

```text
emitter ID        fx_pc_sdm_07.par_v_sdm_onestroke_hit_01::action-31470/stage-000/notify-022::FX_PC_SDM_07.par_v_sdm_onestroke_hit_01.particlespriteemitter_0
emitter row SHA   57228087ab6d3ffb84fe634eab1b5406666536bd140400963be873ba56e8249b
occurrence ID     source-active-027
occurrence SHA    6eac180a4d907b9bd4510161d4e200f2c9bc81280ca618f086321fcaa461fe92
recipe ID         material-recipe-2073fb45e643d1d5
recipe row SHA    f210da08033a522e3ab3e581a5535df5629a15411a6c46f5052b6c73d03202a1
family/evaluator  material-family-ee42f716afdf6145 /
                  reconstructed-evaluator-b64318cb50070e35 v1
size policy       UE3_LENGTH_XZY_0P01, world-size에 exactly once
```

Sprite texture/state 선택은 다음과 같다.

| 역할 | stable 선택 | asset / raw SHA |
|---|---|---|
| recipe texture decision | `recipe-texture-binding-01` | row `238f88b150d88389e897d27c946c63b38f512f7c91a24a81ea77c7786e72e1a3` |
| texture0/base | `renderer-material-input-binding-050`, `render-binding-03`, sampler `material-reconstructed-policy-478283291730ec2c2599` | `Effect/Artist/Textures/fx_a_decal_013.dds` / `c37194e45c9dea1b1f897c150ae0fae113431c90cd8161494f716bc705ac368e` |
| texture1/mask | `renderer-material-input-binding-049`, `render-binding-02`, sampler `material-reconstructed-policy-300f64f5c91f35575f26` | `Effect/Artist/Textures/fx_e_fluid_021.dds` / `1cf86038645760963d6a6db584283795f09fc5078b94ffe8e204ca44ed8bdc75` |
| blend | `render-state-descriptor-02` | `BS_EffectAlpha` |
| raster | `render-state-descriptor-03` | `RS_Cull_None` |
| shader/pass | `Shader_VtxEffectParticle.hlsl` | pass 1 `AlphaTwoSidedDepthRead`, 실제 `DSS_ReadOnly` context descriptor 검증 |

Sprite의 standalone `bdisabledepthtest=false` policy oracle를 `DSS_ReadOnly`와 같다고 주장하지 않는다.
translucent production pass 1을 Apply한 뒤 `OMGetDepthStencilState/GetDesc`로 실제 read-only state를
검증한다. 세 candidateCount=2 renderer decision
`renderer-material-input-binding-025/044/051`은 M0에서 전부 거부한다.

선택한 네 texture의 approved policy descriptor는 모두
`MIN_MAG_MIP_LINEAR(21), WRAP U/V/W(1), bias 0, anisotropy 0, NEVER, minLOD 0,
maxLOD FLT_MAX`다. 그러나 현재 Effects11 `LinearSampler` backing은 실제 reflection에서
`W=CLAMP, MaxAnisotropy=16, MinLOD=-FLT_MAX`이므로 이름이 같다는 이유로 안전하다고 보지 않는다.
approved state를 별도 생성해 exact sampler variable에 scope한 뒤 `GetSampler/GetDesc`와
`PSGetSamplers`를 모두 확인하고 draw 뒤 `UndoSetSampler`한다. SRGB/LINEAR은 sampler 추측이 아니라
각 DDS SRV descriptor로 별도 검증한다. clamp/wrap이 섞인 행은 M0에 넣지 않는다.

authority의 non-anisotropic sampler는 raw `MaxAnisotropy=0`을 그대로 보존한다. D3D11 API는 실행
descriptor에 1 이상을 요구하므로 non-anisotropic/raw0인 경우에만 실행 descriptor를 1로 materialize한다.
raw authority와 실행 materialization을 구분하고 anisotropic/raw0이나 그 밖의 범위 오류는 fail-closed한다.

### production evaluator와 sink 구현 경계

`Effect_ReconstructedExecution.h/.cpp`의 macro 밖에 generic selected evaluator와 immutable evaluated
frame/packet을 둔다. 입력은 Plan과 stable-ID selection이며 Artist ID는 evaluator 내부에 하드코딩하지
않는다. 기존 production `Evaluate_Distribution`과 refactor한 occurrence RNG/fixed-step core를 사용한다.
packet은 raw `EFFECT_ELEMENT_DESC*`를 갖지 않고 Plan/Preparation/CatalogEntry를 강하게 보존한다.

Mesh의 ordered module 000~014와 Sprite의 000~008은 모두 stable module/handler ID로 소비한다. step 88에서
age가 0이라 update delta가 0인 module도 consumed-handler 집합에서 빠지면 실패한다. 알 수 없는 class를
정상값으로 처리하거나 rendererRuntimeConfig를 presentation fallback으로 쓰지 않는다. 결과 frame은
정확히 Mesh 1개와 Sprite 1개가 nonempty이고 같은 entry pointer를 가질 때만 commit한다.

Mesh module 009의 선택 계약도 구현 전에 다음처럼 고정한다.

```text
module ID          <Mesh emitter ID>::module:009
module row SHA     cd94729dd24af5799a9f97a48a1b27908df6e7f720dda2ea728554ed7c53b856
handler registry   handler-a90067d9043e62f6e79a5cc4
handler row SHA    4691180c80cda74a93ea57d68fe2d54d00842562c364a41192b4aeb0fe9f0a40
implementation     source.module.exact.particlemodulelocationprimitivecylinder.v1
implementation SHA dbbdcb1c7b4b28726062cc3846d89b38073fd15dd2cf2cf21869c109c757d5d7
height axis        property/literal/default row가 없는 이 선택 행은 exact handler의 Z-axis default
```

`heightaxis`가 없는 행을 매번 class 문자열로 추측하지 않는다. production dispatch table이 위 handler
registry/implementation identity와 module row를 모두 확인한 뒤 **absent exactly Z**를 적용한다. 다른
handler, 같은 module의 새 `heightaxis`, 또는 X/Y/unknown 값은 이 M0 선택 계약이 아니므로 fail closed 한다.
이는 기존 production cylinder handler의 empty-value Z 동작을 stable handler identity에 결합한 것이며,
새 implicit-default 데이터 행을 발명하지 않는다.

selected evaluator의 distribution RNG는 `artist-f.selected-occurrence-xorshift32.v1`로 고정한다. timing
core의 emitter stream과 lifetime stream은 그대로 두고, packet evaluator는 선택 occurrence의
`iOccurrenceRandomValue`를 별도 local state seed로 복사한다. 각 draw 직전에 `x ^= x << 13`,
`x ^= x >> 17`, `x ^= x << 5`, zero이면 1로 치환하고 `x / UINT32_MAX`를 random unit으로 사용한다.
ordered module 순서와 module의 `distributionIds` 순서를 따른다. operation 2는 component 0..N-1마다
정확히 한 unit을 소비해 component별 lerp를 수행하고, operation 3은 `RandomUnits[0]`용 unit 하나만 소비해
전체 minimum 또는 maximum vector를 선택한다. operation 1과 FLOAT_PARAMETER는 unit을 소비하지 않는다.
operation 2의 random-lock axis는 필요한 component draw를 모두 소비한 뒤 적용한다. cylinder handler는
radius, height, angle, surfaceOnly=false일 때만 radial,
height offset, startLocation, velocityScale 순서다. 선택 Mesh는 surfaceOnly=true이므로 radial draw가 없다.
required/lifetime/spawn module과 그 timing distribution은 fixed-step timing core가 이미 occurrence 생성,
`iLifetimeRandomValue`, lifetime, spawn step에 반영한 authority다. selected packet evaluator는 이 세 handler를
consumed 집합에 포함하고 timing packet과 identity를 검증하지만 별도 local xorshift stream에서 다시 평가하거나
unit을 소비하지 않는다. 따라서 local stream의 첫 draw는 ordered module 순서의 첫 non-timing visual distribution이다.
required module의 fixed literal도 draw를 추가하지 않는다. 선택 행의 exact local draw sequence는 다음과 같다.

```text
Mesh026 seed 2215704123
01 3224154448 0.75068195554210848 M002 startsize op2 c0
02 2530594867 0.58920003184797243 M002 startsize op2 c1
03 3064425754 0.71349222089943765 M002 startsize op2 c2
04  998511512 0.23248407808888799 M003 startvelocity op2 c0
05 2266931971 0.52781123005035591 M003 startvelocity op2 c1
06 1017363772 0.23687346192004008 M003 startvelocity op2 c2
07  730271614 0.17002960996935834 M006 acceleration op2 c0
08 2273174282 0.52926463133871193 M006 acceleration op2 c1
09 1485532676 0.34587752920246628 M006 acceleration op2 c2
10 2620743809 0.61018946804343477 M007 startrotationrate op2 c0
11  881740531 0.20529621541623402 M009 cylinder angle
12 1205524347 0.28068300971777249 M009 cylinder height offset
13 3145893313 0.73246036510273360 M009 velocityscale op2 c0
14 2355157980 0.54835294851762073 M010 startlocation op3 whole-vector selector
final state 2355157980; surfaceOnly=true이므로 cylinder radial draw 없음

Sprite027 seed 244989949
01 1708315311 0.39774815351649845 M002 startsize op3 whole-vector selector
02 3479339244 0.81009679585930350 M004 startvelocity op2 c0
03 1888920883 0.43979866510252436 M004 startvelocity op2 c1
04  572682827 0.13333811125097286 M004 startvelocity op2 c2
05 3775900606 0.87914536867270832 M007 startlocation op2 c0
06 2608091586 0.60724364281800658 M007 startlocation op2 c1
07  953103844 0.22191178151916521 M007 startlocation op2 c2
final state 953103844; procedural draw 없음
```

이 sequence에서 Mesh pre-cue packet의 size XZY는
`[1.6260229333131626,1.5702383313491564,1.3838000477719588]`, acceleration은
`[0,-11.308244941595067,0]`, rotation rate는 `79.33641699127304 deg/s`, local position은
`[1.0635928511667767,0.6978068300971776,-0.28824338214256146]`, velocity는
`[0.18857087353033217,2.231900362239327,-0.653603293505097]`, handler RGBA는 implicit alpha
identity를 적용한 `[0,0,0,1]`이다. Sprite packet의 signed world-size XZY는 `[-0.5,0,0.4]`,
velocity는 `[0.8100967958593035,0.28666618887490275,0]`, local position은
`[3.5165814746908333,0.1,-0.10724364281800661]`, color는
`[0.009999999776482582,0.10000000149011612,0.20000000298023224,1.5]`, dynamic parameter는
`[1,0.5,1,1]`이다.

이 순서와 RNG version은 packet projection에 들어가며 legacy UE LCG나 timing emitter stream을 재사용하지
않는다. step 88 serial 0의 timing identity는 Mesh occurrence/lifetime RNG
`2215704123/2215704123`, lifetime `2.257941908612368`; Sprite는
`244989949/244989949`, lifetime `0.6228164894929623`로 고정한다.

size basis는 source `[x,y,z]`를 client XZY `[x,z,y]`로 한 번만 바꾼다. Mesh packet은 이 값을
dimensionless scale로 보존하고 `.01`을 곱하지 않는다. Sprite packet은 XZY에 `.01`을 한 번 곱한 signed
world-size를 보존하며 production quad의 width/height는 packet X/Z, 즉 source X/Y를 사용한다. 음수 X는
승인된 image flip을 보존하고 절댓값 fallback으로 지우지 않는다. sink는 이 값을 다시 reorder하거나
centimeter 변환하지 않는다.

두 material evaluator의 production common-shader binding도 다음처럼 고정한다.

| 선택 | evaluator/family identity | feature mask | texture lane / shader pass |
|---|---|---:|---|
| Mesh 026 | `reconstructed-evaluator-c3ac12f104b50f06`, family row `b95ca3e38af0bab700b1941c9f34e7c1819fd11eafb8e4ae22bcd0dd374ab43b` | 41 | decision 06의 texture0/1 -> `g_SourceTexture0/1`, Mesh pass 0 |
| Sprite 027 | `reconstructed-evaluator-b64318cb50070e35`, family row `78577bca3d6ff10f53428196606c79548b3b1d52cc0594e469f701a5ced8c568` | 811 | decision 01의 texture0/1 -> `g_SourceTexture0/1`, Particle pass 1 |

packet은 `g_ReconstructedMaterialEvaluatorEnabled`, exact feature mask, `g_ReconstructedUVScale`,
`g_ReconstructedPanRotationAux`, `g_ReconstructedColor`, `g_ReconstructedParams0/1`에 해당하는 typed block을
소유한다. HLSL은 이미 WARP oracle이 검증한
`SECOND_TEXTURE_MULTIPLY -> UV_TRANSFORM_PHASE -> PANNER_PHASE -> COLOR_MULTIPLY -> DESATURATION ->
SIGNED_POWER -> FRESNEL_GAIN -> DISTORTION_OFFSET -> DISSOLVE_ALPHA -> ALPHA_MULTIPLY` 순서만 production
common path에 옮긴다. legacy `g_SourceMaterialProfile`이나 base/noise/mask fallback으로 두 family를 추측하지
않고, evaluator/family/recipe/decision/lane identity가 하나라도 다르면 stage를 rollback한다.

GPU stage는 기존 `CEffectDocumentRenderer`가 소유한다. model은 `CModel` 통합 경로로만 만들고 carrier
preScale을 create/pretransform에서 한 번만 적용한다. Sprite particle은 production particle instance
buffer와 shader를 사용한다. diagnostic placement root만 실제 camera 앞의 visible 위치를 제공하며 packet의
source local transform을 대신 만들지 않는다.

각 DDS는 안전한 Resources-relative path에서 immutable byte vector로 한 번 읽는다. **그 같은 vector**를
SHA-256 검사와 `CreateDDSTextureFromMemoryEx`에 넘기고 SRV format/dimension/mips를 검사한다. stage 뒤에는
DDS, WModel, candidate, sidecar를 reread하지 않는다. M0 smoke는
`LOSTARK_RESOURCE_ROOT=C:\Users\user\Desktop\LostArk\Client\Bin\Resources`를 사용한다. 이 경로는
AGENTS가 팀장 관리 physical runtime input으로 지정한 유일한 dirty-main read-only 예외다. source/code/Data는
이 root에서 읽지 않으며 selected 1 WModel과 4 DDS 이외를 M0 authority로 세지 않는다. M0는 이 root를
수정하지 않는다.

Engine `CShader`에는 exact sampler variable `LinearSampler`의 최소 Set/Undo wrapper만 추가한다.

```text
SetSampler(0, approved state)
-> Begin(exact pass) / FX Apply
-> approved state GetDesc + PSGetSamplers actual COM/descriptor
-> OMGetBlendState + RSGetState + OMGetDepthStencilState actual descriptor
-> production draw
-> UndoSetSampler(0)
-> prior PS sampler restore 또는 original pass re-Apply
```

Set/Apply 뒤, draw 전, draw 실패를 포함한 모든 경로에서 Undo와 이전 context sampler 복원을 수행한다.
Undo 실패면 M0 draw 성공으로 기록하지 않고 composite를 deactivate하며 이전 composite를 보존한다.

### 검증과 checkpoint 순서

1. 이 final plan-only commit을 push하고 reviewer에게 exact SHA와 `PLAN only` scope를 보낸다.
2. exact13 producer/consumer는 `ab559784` PASS를 fresh-LF worktree에서 재검증한다. schema/hash를 바꾸는
   corrective가 필요하지 않으면 새 구현 checkpoint로 다시 만들지 않는다.
3. `18d2b489` selected evaluator를 독립 판정하고, PASS 뒤 harness-only request builder를 하나의 production
   Program/sidecar-derived generic diagnostic factory로 factor한다. M0 selector data는 Tool input일 수 있지만
   production factory의 class/count/hash authority가 될 수 없으며 M1 전에 Artist-specific C++ fixture가 0인지
   audit한다. macro 없는 actual-catalog harness에서 step 88 Mesh/Sprite 2/2,
   module-handler consumption24/unique route15, determinism, empty/failure preservation을 통과시키고 frozen SHA를
   재감사한다.
4. existing renderer/Object/Effect Tool에 selected composite, production Mesh/Sprite sink, sampler scope,
   draw statistics와 D3D debug gate를 연결한다. catalog revision/device mismatch와 GPU stage/Undo failure에서
   이전 composite 보존을 검증한다. commit/push 뒤 exact SHA/scope만 보낸다.
5. `git diff --check`, JSON/XML parse, focused Python/PowerShell audit, ClientFrontendHarness Debug/Release,
   Engine Debug/Release -> UpdateLib Debug/Release -> Client x64 Debug build, ProjectAudit를 실행한다.
6. 사용자가 `Client/Default` cwd와 위 Resources root에서 x64 Debug Client를 실행하고 F1 Effect Tool의
   nonProduct M0 diagnostic/solo를 사용한다. pipeline query는 두 production draw의 IA vertex/primitive,
   VS/PS invocation이 모두 nonzero인지 기록하며 에이전트는 공유된 구조화 로그에서 값과
   ID3D11InfoQueue의 새 ERROR/CORRUPTION 0을 확인한다. 자동 gate가 모두 PASS한 뒤 사용자가 실제
   Client 창을 눈으로 확인해 수동 결과를 기록한다.

M0에 직접 필요하지 않은 새 provenance/audit framework는 만들지 않고 M1~M3 backlog에만 기록한다.
Presentation provider-local mixed batch, Light/Post, Decal/Ribbon, sampler72 전체, visual33 전체,
PlayerSkills/animevent actual F, Product, R6/R7, 타 class/Valtan은 M0 구현 파일과 결과에 포함하지 않는다.

## 이번 재개판의 핵심 판단

Artist F M0~M3는 fresh 구현 세션 하나가 shared runtime을 끝까지 직렬 소유한다. 사용자가 여러 작업의
진행률을 해석하거나 서로 다른 implementation branch의 결합 순서를 관리하지 않게 한다. 별도 agent/task는
frozen SHA의 독립 review만 수행하고 코드를 수정하지 않는다.

Source/Material evidence 때 유효했던 상시 specialist 병렬 모델은 여기서 종료한다. Catalog, evaluator,
Object, Renderer, Shader, Effect Tool, Presentation은 같은 immutable pointer와 admission graph를 공유하므로
둘 이상의 writer가 나누면 interface drift와 반복 build가 더 커진다. M3 동결 뒤 R8 raw corpus 적용만
data-only lane으로 병렬화할 수 있으며, generic C++ 변경이 필요하면 즉시 단일 writer로 되돌아간다.

이 결정은 independent review를 줄이는 것이 아니다. reviewer는 각 frozen checkpoint에서 과거처럼
coordinated reseal, identity laundering, rollback, nonfinite/bool, empty-frame/no-pixel success를 공격하되,
live WIP를 계속 따라가거나 구현자의 파일을 고치지 않는다. 구현자는 reviewer의 재현을 같은 branch의
corrective commit으로 닫고 새 exact SHA를 다시 제출한다.

## 목표와 종료 증거

도화가 `31470 / 필법 : 한획긋기 / F`의 7 cue와 35 active occurrence를 하나의
Source Contract에서 immutable typed Cascade IR, geometry와 material binding, 여섯 renderer,
Effect Tool의 동일 prepared revision, Product Catalog까지 연결한다.

이 작업의 종료는 문서 생성이나 scaffold build가 아니다. 다음 조건을 모두 만족해야 한다.

```text
Source denominator        7 cue / 35 occurrence / 399 ordered module reference
Source property           1,434 top-level tagged property / 1,572 primitive leaf / 629 distribution
Renderer denominator      Mesh 13 / Sprite 16 / Decal 3 / Ribbon 1 / Light 1 / ScreenPost 1
Local reference           15 definition / 17 occurrence / PointLight 1 occurrence
Geometry                  7 WModel carrier / 13 Mesh occurrence
Material                  27 unique recipe / 34 rendered material occurrence / builtin Light 1 separate
Runtime authority         immutable compiled IR only
Silent fallback           0
Unknown evaluator run     0
Missing material fallback 0
Double geometry scale     0
Stale generated hash      0
Failed transaction leak   0
Product admission         35/35, atomic cue publish
```

이미지 기반 자동 판정은 종료 증거로 사용하지 않는다. 대신 자동 numeric/structural gate 뒤 사용자의
실제 Client 육안 확인과 occurrence별 `APPROVED`는 필수 종료 증거다. fixed seed, fixed timestep,
명시적인 ActionCue 입력과 sample time, numeric tolerance를 가진 oracle와 컴파일·링크·실행 오류 검증이
수동 판단보다 먼저 PASS해야 하며, 육안에서 발견된 차이는 overlay와 fixture로 환류해 재검증한다.

## 현재 실제 기준점

R0/R1 evidence acquisition 뒤 사용자는 reconstructed high-ceiling 경로를 승인했고, R2 immutable
candidate/parser/catalog와 CPU plan compile은 완료됐다. source-era actual-output/state provider는 여전히
없으므로 historical blocker와 `sourceExactAdmission=false`를 보존한다. `4ffe1102`는 폐기했으며 현재
runtime program은 그 checkpoint를 재사용한 것이 아니다. 선택 production evaluator는 구현됐지만 독립
review 중이고 production caller가 없으므로 GPU/Tool/actual F/Product admission은 모두 0이다.

| 영역 | frozen checkpoint | 독립 판정과 현재 사용 정책 |
|---|---|---|
| Geometry evidence/resource | `0aca792819fdda3f541bb7cec7451c5ed93c6467` | PASS. 7 WModel v1.1 physical deploy와 7/7 decode를 재사용하되 runtime preScale consumer는 아직 0/7 |
| Material evidence/runtime corrective | `cde8f3bddea2f9415f682b387d2705fd25794075` | `627ddc76`, `ab76b7ec`, `d39097c3`의 admission/semantic validation gap을 supersede. evidence integrity PASS, source value는 render 0/89·static 23/94·sampler 0/72, execution readiness 0/255 BLOCK. exact sampler 0, rejected legacy 4, Product false |
| Generic compiler | `c4b00f14b32d27604ac677e9a9ea81b01ecaa551` | non-executable inspection core 범위 PASS, Product false |
| Runtime candidate/parser/catalog | `ab559784a7fa0c5d19ccedf600d08f8a9ee50a25` | fresh-LF exact13 PASS. immutable Program/sidecar pointer와 rollback을 재사용하며 admission false |
| CPU typed plan | `7d3e957f4d93bfd1416fa6a05d5d7fa8f46c12a2` ancestry | 7/35/399/629 compile와 exact semantic projection PASS. production evaluation proof는 아님 |
| selected production evaluator | `18d2b48920b2a327ac59b572960325d352e77a6f` | Mesh/Sprite 2행, module-handler consumption24/unique route15 구현, 독립 review 중, production caller/GPU sink 없음 |
| offline render-resource authority | integration ancestry의 audited sidecar | DDS48/binding72와 recipe/state evidence만 재사용. current FX11 sampler 불일치 때문에 render authority로 자동 승격 금지 |
| Source execution corrective | `c927e397811d4e5718efd27b187eb59775023685` | evidence integrity PASS, module 370 ready/29 blocked. current-only evidence를 source exact로 승격하지 않음 |
| Source provider acquisition | `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3` | accessible-scope evidence PASS, execution readiness 0/29 BLOCK. 29 occurrence→15 class→7 native family, provider/pilot 0, VSS `PERMISSION_UNCHECKED`, Product false |
| Source runtime materializer | `4ffe1102ed9cf3e21f669da292fac1f143e18d8f` | 폐기한 BLOCK checkpoint. current candidate의 parent나 runtime authority로 사용하지 않음 |

현재 frozen R1 검증의 사실은 다음과 같다.

```text
Source acquisition mutation                21/21 PASS
Source actual-output readiness             0/29 BLOCK
Source ownerless row                       0
Geometry physical resource                 7/7 deployed and decoded
Material source-value acquisition          12/12 PASS
Material runtime mutation                  23/23 PASS
Material WARP oracle plumbing              numeric 200 / state pilot 4 PASS
Material source value                      render 0/89, static 23/94, sampler 0/72
Material execution readiness               0/255 BLOCK
Selected evaluator admitted                0 until frozen review PASS
Production evaluator caller                0
GPU typed sink                             0/6
Actual Artist F                            0/35
Runtime execution admission                false
Product admission                          0/35
```

위 WARP PASS는 evaluator/state consumer plumbing의 재현성만 증명하며 source-era 값을 제공하지 않는다.
Source full audit의 lane 외 14건과 Material acquisition 이전 full audit의 lane 외 12건도 전체 green으로
세탁하지 않는다. 현재 병목은 더 이상 artifact 조사만이 아니라 production caller, immutable GPU composite,
typed sink 6개와 actual action 연결이다. source-era provider 부재를 current default/fallback으로 덮는 것도,
구현되지 않은 seam을 receipt boolean만으로 여는 것도 금지한다.

## 고정 fidelity와 admission 계약

모든 definition, occurrence, resource binding은 다음 네 축과 네 blocker set을 독립적으로 가진다.

```text
sourceFidelity
  SOURCE_EXACT, CURRENT_REVISION_EVIDENCE, RECONSTRUCTED_NUMERICALLY_VERIFIED,
  RECONSTRUCTED_GRAPH, UNRESOLVED를 구분한다.

artifactBindingIntegrity
  compiled expected identity와 실제 payload bytes, package/export/record, model/material hash가
  일치하는지를 판정한다. self-signed container hash만으로 true가 되지 않는다.

executionAdmission
  payload를 읽기 전에 blocker가 0이고 해당 opcode/evaluator/renderer handler와 numeric oracle이
  존재할 때만 true다.

productAdmission
  execution, geometry, material, renderer, Tool/Catalog transaction과 최종 regression이 모두
  통과한 occurrence만 true다. 35개 중 하나라도 false면 cue 전체를 publish하지 않는다.

evidenceBlockers
  historical source fidelity가 닫히지 않은 이유다. verified reconstruction이 성공해도 삭제하지 않는다.

artifactBindingBlockers
  expected package/export/record/model/material identity와 실제 bytes가 다를 때만 유지한다.

executionBlockers
  opcode/evaluator/default/resource/renderer의 실행 의미가 닫히지 않았을 때 유지한다.
  numeric reconstruction proof는 이 집합의 명시된 blocker만 해소할 수 있다.

productBlockers
  execution, prepared transaction, runtime resource, regression이 제품 조건을 충족하지 못할 때 유지한다.
  evidenceBlockers가 남아 있다는 이유만으로 자동 생성하지 않고 G00의 허용 fidelity matrix로 계산한다.
```

`SOURCE_EXACT`는 완화하지 않는다. historical source가 없는 target을 current Constant나 class
이름만으로 exact 또는 executable로 바꾸지 않는다. `RECONSTRUCTED_NUMERICALLY_VERIFIED`는 별도의
evaluator ID/version, oracle provenance, 입력 영역, expected sample, tolerance, 독립 구현 비교를
모두 가진 경우에만 허용한다. `evidenceBlockers`는 이 승격 뒤에도 보존하되 독립적으로 검증된
reconstruction은 대응 `executionBlockers`만 해소한다.

maximum-reconstruction admission branch는 사용자의 별도 명시 승인으로
`RECONSTRUCTED_APPROVED_V1_POLICY_CONSTRUCTION` 상태다. Material의 기존 23 arithmetic evaluator와
WARP replay만으로 source value나 execution row를 자동 승격하지 않는다. Source 29행과 Material 255행은
각각 explicit policy row와 실제 consumer capability가 생긴 경우에만 reconstructed execution candidate에
포함한다. policy/capability가 없는 current default, class 이름, 암묵적 texture/state fallback은 계속
실행 또는 Product admission에 사용할 수 없다.

Product predicate는 마지막에 수동으로 정하지 않는다. 네 축, 네 blocker set, 허용 fidelity matrix로
G00에서 고정하고 G11은 같은 predicate를 평가만 한다. 서로 다른 blocker set을 하나의 union으로
합쳐 source exact가 아닌 행을 영구 Product false로 만드는 방식은 금지한다.

## 세션과 worktree 구조

M0~M3의 active implementation writer는 하나다. 이 plan-only SHA에서 fresh worktree를 만들고
`18d2b489` evaluator 판정/교정 -> production request factory -> immutable GPU composite -> M0 first pixel ->
full 35 evaluator/resource/sink -> actual F Debug -> Product D/R 순서로 같은 branch를 전진시킨다.

reviewer는 frozen commit이 push된 뒤에만 exact tree를 새 worktree에서 읽는다. 구현 세션은 reviewer가
끝날 때까지 다음 admission을 열지 않지만, unrelated 문서/추가 framework를 만들며 시간을 보내지 않는다.
reviewer는 PASS/BLOCK과 재현 입력만 보내고 파일을 수정하지 않는다.

R8에서는 4-class raw/data 적용과 Valtan raw/data 적용을 서로 다른 worktree에서 병렬화할 수 있다.
단 shared compiler/evaluator/renderer/header 수정은 implementation captain 한 명에게 반환한다.

세션 간 메시지는 target task의 최신 상태를 먼저 읽은 뒤 다음 네 종류만 사용한다.

1. `ADD`: 현재 plan/checkpoint에 새로 필요한 입력이나 증거
2. `CORRECT`: exact SHA에서 재현한 P0/P1과 최소 교정 범위
3. `STOP`: admission 또는 destructive action을 즉시 막아야 하는 조건
4. `REVIEW_EXACT_SHA`: parent/SHA/owned files/tests/declared scope가 완결된 frozen review 요청

Checkpoint 보고는 다음 필드를 모두 가진다.

```text
base SHA
commit SHA
owned files
public contract changes
tests actually executed
Product admission
remaining blockers
integration order
```

## 파일과 단일 소유자

아래 표의 과거 absolute worktree prefix는 기록용이며 실행 경로가 아니다. fresh 구현 세션은 자기 clean
repository root를 `$ARTIST_F_WORKTREE`로 두고 표의 repository-relative suffix만 사용한다. 다른 task의
worktree나 main dirty worktree에서 파일을 읽어 build/PASS를 만들지 않는다. 단 앞에서 명시한 team-managed
physical `Client/Bin/Resources` read-only 예외는 허용한다.

| 구분 | 절대 경로 | 단일 소유자와 역할 |
|---|---|---|
| 통합 | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/.md/GB/08-10/2026-08-10_ARTIST_31470_F_COMPLETE_RESTORATION_IMPLEMENTATION_PLAN.md` | Integration Captain의 전체 G와 gate 정본 |
| Source schema | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Public/Effect_AuthoringDocument.h` | G00 Integration만 public transport를 변경하고 이후 additive request를 중재 |
| Source Codec | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_DocumentCodec.cpp` | G00 Integration과 Source reviewed bridge만 수정 |
| Source generator | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/LevelPlacementExtractor/build_artist_31470_source_contract.py` | Source Closure 단일 소유 |
| Source outputs | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Imported/Artist/Candidates` | Source Closure가 candidate/receipt를 단 한 번 재생성 |
| Source registry | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Contracts/ue3-cascade-source-v1.registry.json` | Source Closure 단일 소유 |
| Geometry evidence | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/ModelAssetConverter` | Geometry Evidence lane이 decoder/cooker/golden만 소유 |
| Geometry Engine | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Private/BinaryAsset/Winters` | Geometry Evidence lane이 WModel decode contract를 소유 |
| Material evidence | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py` | Material Evidence lane 단일 소유 |
| Material outputs | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Imported/Artist/Materials` | Material Evidence lane receipt/recipe 단일 소유 |
| Compiler | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Public/Effect_CascadeCompiler.h` | G04가 inspection core를 만들고 G06이 execution IR shape/admission을 최종 동결 |
| Compiler implementation | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_CascadeCompiler.cpp` | G04 inspection adapter 뒤 G06만 execution adapter/opcode/receipt를 최종 수정 |
| Executor | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_Playback.cpp` | G07만 finalized execution IR 소비와 Mesh hidden scale 제거를 소유 |
| Prepared render | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_DocumentRenderer.cpp` | Renderer Foundation이 common dispatch/cache/upload를 소유 |
| Presentation | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_PresentationService.cpp` | Prepared/Catalog transaction lane이 exact revision attach를 소유 |
| Effect Tool | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_Tool.cpp` | Tool lane이 read-only Source mode와 Draft/Product Play 경계를 소유 |
| Common shader | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli` | Renderer Foundation 단일 소유; family lane이 수정하지 않음 |
| Light public payload | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Public/Engine_Struct.h` | G06이 radius/falloff/color/brightness typed payload를 소유 |
| Deferred Light shader | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl` | G06이 typed falloff 소비와 attenuation contract를 소유 |
| Shared registrations | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Default/Client.vcxproj` | 필요한 lane commit에 additive 등록하고 Integration이 충돌 해결 |
| Audit registration | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | Integration이 gate별 focused audit를 최종 등록 |

Source candidate는 실제로 직접 pin하는 Source/geometry/active-material-closure 입력만 재생성한다.
새 typed MaterialRecipe 전체를 Source candidate에 다시 복제하지 않는다. 최종 compiled receipt가 Source,
Geometry, Material contract hash와 compiler revision을 함께 결합해 불필요한 hash regeneration cycle을
막는다.

## 현재부터 완성까지의 실행 순서

아래 순서는 기존 G 전체를 다시 시작하는 계획이 아니라 R0 evidence PASS 뒤 execution-readiness를 열어
PASS된 Runtime foundation `38ebe7cf` 위에 최종 pipeline을 결합하는 critical path다.

### R0. 35/35 feasibility와 blocker owner 동결 — matrix PASS, execution BLOCK

Playback과 renderer 구현 전에 Source 29개 blocked module과 Material 255개 execution row를 행 단위로
지도화했다. 이 gate는 문서상 owner 이름만 채운 것이 아니라 각 행의 acquisition/oracle 경로와 실패
판정을 frozen receipt로 고정했다.

Source feasibility matrix의 각 행은 다음을 가진다.

```text
moduleOccurrenceId
exactSourceClass
family
requiredRuntimeOutputs
sourceEraPackageOrBinaryIdentity
currentRevisionEvidenceIdentity
nativeEntryOrDispatchIdentity
numericOracleInputDomain
numericOracleExpectedOutput
independentOracleImplementation
oracleProvider
pilotFixtureIds
pilotExpectedMutatedOutputs
numericTolerance
pilotDecision
fidelityDecision
executionDecision
owner
remainingBlockers
```

Source safe denominator는 29 blocked module이다. standard seeded 11, EF custom 15, EF multiply
distribution owner 3을 임의로 합치거나 denominator에서 제거하지 않는다. input digest parity는 output
oracle가 아니다. source-era identity 또는 실제 native particle output 비교가 없으면 READY로 승격하지
않고 `CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`와 blocker를 유지한다. 각 family는 최소 하나의
실제 pilot fixture로 particle/component output 변화와 tolerance를 증명해야 하며 `FEASIBLE`,
`VERIFIED_IRRELEVANT`, `BLOCKED` 중 하나로 판정한다.

Material feasibility matrix의 각 행은 다음을 가진다.

```text
materialRecipeId
materialOccurrenceIds
fieldId
fieldKind
bindingOriginAndOwner
instanceRecordIdentity
parentOrCdoIdentity
staticOrShaderMapIdentity
rendererConsumption
acquisitionPath
oracleProvider
pilotFixtureIds
numericOracleInputDomain
numericOracleExpectedOutput
numericTolerance
pilotDecision
fidelityDecision
executionDecision
owner
remainingBlockers
```

Material safe denominator는 27 recipe/34 occurrence, 162 render-state field 중 explicit 73/unresolved 89,
static input 94, strict sampler 72다. 이전 exact sampler 3 instance+1 parent는 texture binding/DDS identity만
exact하며 source-era Texture2D CDO와 TextureGroup filter configuration이 없는 full descriptor는 증명하지
못하므로 strict sampler에 포함한다. render-state 89, static 94, sampler 72를 별도 matrix로 유지하며
validator correction과 혼동하지 않는다.
omitted field는 instance -> Parent Material -> nested default -> class CDO 순서로만 해석한다. D3D
blend/depth/cull state, static permutation, sampler address/filter/sRGB를 offscreen WARP 또는 독립 state
oracle의 작은 pilot로 실제 관측하지 못한 행은 BLOCK으로 유지한다. source-revision ShaderCache 또는
controlled runtime capture가 필요한 행은 acquisition provider가 실제로 동작하기 전까지 FEASIBLE로도
승격하지 않는다.

Frozen 결과는 source value와 execution readiness를 분리한다. Material static 23행은 source MIC의 exact
`bOverride=true` 값을 확보했지만 actual state-output pilot과 final consumer가 없으므로 READY가 아니다.
WARP 200 numeric sample과 state pilot 4건도 source 값 provenance가 아니라 oracle plumbing 검증이다.

```text
Source matrix integrity                    PASS, ownerless 0
Source execution readiness                 0/29 BLOCK
Material matrix integrity                  PASS, ownerless/unknown 0
Material source value                      23/255
Material execution readiness               0/255 BLOCK
VSS                                        PERMISSION_UNCHECKED
NVIDIA DXCache                             548/561 readable, 13 share-locked
global exhaustion claim                    false
```

R0 evidence-integrity 합격 조건:

- Source blocked 29/29, Material render-state 89/89, static 94/94, sampler 72/72가 matrix에 존재한다.
- owner 없는 행 0, silent fallback 0, denominator shrink 0이다.
- 각 행/family는 필요한 expected output과 tolerance 계약을 기록하며 provider, pilot 또는 tolerance가
  없으면 `BLOCKED`로 보존한다.
- READY 후보는 source-era evidence 또는 실제 expected mutated output과 tolerance를 고정한 독립
  actual-output/state oracle pilot을 가진 경우에만 존재할 수 있다.
- current-only/cross-revision 행을 `SOURCE_EXACT`로 승격한 행 0이다.
- acquisition이 불가능한 행은 즉시 명시되어 Artist F 35/35의 hard blocker로 보고된다.
- R0 matrix와 resolver/validator mutation test가 frozen review PASS하기 전 R2 이후 shared runtime 구현을 시작하지 않는다.

위 evidence-integrity 조건은 `7da937ae`와 `cde8f3bd`에서 충족했다. 이는 원본 의미 복원 완료가 아니다.
execution readiness는 Source 0/29, Material 0/255이므로 별도 BLOCK이며 R2 이후를 열지 않는다.

### R1. Source/Material provider acquisition — evidence PASS, execution BLOCK

Source `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3`은 접근 가능한 filesystem/backup/current install/
Git remote·LFS·unreachable object 범위를 조사하고 29 occurrence를 15 exact class와 7 native family로
축약했다. source-era actual-output provider와 standalone pilot은 각각 0이고 blocker delta는 `29 -> 29`다.
current wrapper/dataflow와 fixed input digest는 `CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`로만 보존한다.

Material `cde8f3bddea2f9415f682b387d2705fd25794075`는 `627ddc76`의 잘못 승인된 exact sampler 4건,
`ab76b7ec`의 row/evaluator validator gap, `d39097c3`의 acquisition/runtime top-level schema gap을
교정하고 typed contract v4와 runtime receipt v3를 재봉인했다. render-state source value는 0/89,
static은 exact override value 23/94, sampler full descriptor는 0/72다. static nonoverride semantics 미확정
43행과 GUID join 없음 28행을 분리하며 execution readiness는 전체 0/255다.

두 lane 모두 VSS는 비관리자 권한으로 `PERMISSION_UNCHECKED`이고 Material GPU cache는 NVIDIA 561개 중
13개가 share-locked다. 접근 가능한 범위는 소진했지만 `globalExhaustionClaim=false`다. safe source-era
standalone runtime/UCC/commandlet/debug capture 경로가 없으며 injection/hooking/anti-cheat 우회는 시도하거나
허용하지 않는다. current capture는 가능해져도 `CURRENT_REVISION_OBSERVED`일 뿐 source-era fidelity를 닫지 못한다.

아래 표는 reconstructed approval 직후의 historical R1 판정이다. 현재 구현 상태는 앞의
`2026-08-11 고점 복원 최종 결정 정본` 표를 사용한다.

```text
Source acquisition evidence                PASS_ACCESSIBLE_SCOPE_EXHAUSTED
Source actual-output execution readiness   0/29 BLOCK
Material evidence integrity                PASS
Material source value                      23/255
Material execution readiness               0/255 BLOCK
ownerless/unknown row                       0
typed materializer                         historical BLOCK at R1
reconstructed branch                       RECONSTRUCTED_APPROVED_V1 / POLICY_CONSTRUCTION
Product admission                          false / 0/35
R2                                          historical IN_PROGRESS, now transport PASS
R3-R8                                       current milestones M0-M3/R8로 superseded
```

source-era exact 경로의 R2 진입 predicate는 Source 29/29, Material render 89/89·static 94/94·sampler
72/72, unresolved execution row 0으로 유지한다. 사용자가 승인한 reconstructed 경로의 R2 진입
predicate는 frozen evidence-integrity PASS, `sourceExactAdmission=false`, 29+255 denominator 보존,
explicit per-row policy/capability/oracle owner, silent fallback 0이다. Corrected materializer는 이 정책을
입력으로 R2에서 새로 작성하고 R2 종료 시 독립 PASS해야 한다. R3 진입은 materializer의 field coverage
100%, unknown/ownerless 0, 29+255 reconstructed capability receipt와 Debug/Release parser mutation PASS를
추가로 요구한다.

### R1 재개 입력 계약

`SOURCE_EXACT` 재개에는 같은 revision의 `EFEngine.dll`, `LOSTARK.exe`, `Engine.u`, `Core.u`, `EFGame.u`,
target UPK, ShaderCache/material map, SystemSettings TextureGroup config와 single-revision identity manifest가
필요하다. 대안은 동일 build identity와 fixed seed/time/world/parameter input, pre/post full numeric state,
expected output과 tolerance를 가진 인증된 source-era capture다.

최대 reconstruction 분기는 사용자 승인으로 시작한다. 행별 explicit project policy와 independent
same-input output/state oracle을 통과한 값만 versioned `RECONSTRUCTED_APPROVED_*` 또는
`RECONSTRUCTED_NUMERICALLY_VERIFIED`로 표시하며, evidence blocker, `sourceExactAdmission=false`,
Product false를 보존한다. 이미지 비교는 단독 numeric admission 입력이 아니지만 사람 눈 검증은 필수다.
수동 smoke에서 발견한 차이는 occurrence별 tuning overlay와 numeric/structural fixture로 환류한다.

이하 historical R2~R8의 denominator와 failure predicate는 유지하되 실제 진행 순서는 앞의 M0~M3/R8을
정본으로 사용한다. R2 transport는 완료됐고 M0 selected evaluator가 독립 review 중이다.

### R2. 승인 checkpoint 통합과 final typed schema 동결 — transport PASS

아래 1~7은 historical construction 기록이다. 현재 fresh-LF candidate/parser/exact13 Catalog transport는
PASS했으며 current implementation parent는 `18d2b489`다. 이 절을 다시 cherry-pick하는 실행 목록으로
사용하지 않는다.

1. clean final-integration worktree를 PASS된 Runtime foundation `38ebe7cf`에서 만든다.
2. Geometry는 조상에 이미 포함된 evidence commit을 재적용하지 않고 `git cherry-pick -x 669acf07`,
   이어서 `git cherry-pick -x 0aca7928`을 각각 실행한다. `669acf07..0aca7928` range는 첫 commit을
   누락하므로 사용하지 않는다.
   Compiler `c4b00f14`와 Publisher `c90c462`는 patch-equivalent 조상에 이미 포함되므로 재적용하지 않는다.
3. Source는 `ae425aaf -> 9b046d61 -> c927e397 -> 7da937ae (evidence PASS, readiness 0/29)
   -> <future 29/29 readiness>` 전체 검증 단위를, Material은 `532d1f52 -> b6757a21 -> e54a5a2a
   -> 627ddc76 (superseded) -> ab76b7ec (validator BLOCK) -> d39097c3 (top-level schema BLOCK)
   -> cde8f3bd (evidence PASS, readiness 0/255)
   -> <future 255/255 readiness>`
   전체 검증 단위를 적용한다. 잘못된 중간 READY 상태에서는 build, publish, admission을 수행하지 않는다.
4. BLOCK materializer `4ffe1102`는 cherry-pick하지 않는다. final Source/Material schema를 받은 combined
   head에서 corrected materializer를 새로 작성한다.
5. Source 35/399/629, Geometry 7, Material 27/34를 결합한 six-hash compiler input을 한 번 생성한다.
6. exact class, opcode, distribution evaluator, renderer, GeometryBinding, MaterialBinding capability ID와
   version을 동결한다.
7. blocked payload는 읽기 전에 거부하고 runtime fallback은 0으로 유지한다.

`native-v14.source-contract-candidate.effect.json`, 그 receipt, `4ffe1102` runtime-program candidate는 현재
R1 readiness를 반영하지 않은 stale 산출물이므로 재사용하지 않는다. 재생성은 Source/Material readiness
receipt -> Source candidate/receipt -> corrected runtime program -> six-hash compiler input -> derived
Assembly/artifact -> Catalog/prewarm 순서로 정확히 한 번 수행한다.

R2 종료 조건은 corrected materializer independent PASS, field loss 0, stale output 0, unknown capability 0,
ownerless blocker 0이며 Product false다.

### R3. Actual typed executor와 resource consumer — M0/M1 remaining

이 구간은 shared C++ dependency가 촘촘하므로 fresh implementation captain 한 명이 다음 순서로 직렬 구현한다.

1. Playback이 immutable compiled program pointer만 소비하게 하고 raw SourceRecipe/module 문자열 scan을 0으로 만든다.
2. standard distribution, fixed seed/time, ActionCue, spawn/lifetime/location/velocity/size/color/SubUV와
   TypeData handler consumption receipt를 연결한다.
3. GeometryBinding expected tuple을 `CModel -> CMesh -> CMaterial`에 연결한다.
   `geometryPreScale=0.01`은 vertex와 bounds에 정확히 한 번 적용하고 Mesh StartSize hidden `x0.01`을 제거한다.
4. MaterialBinding 27 recipe/34 occurrence를 typed evaluator와 HLSL에 연결하고 explicit render state만 소비한다.
5. post-compile raw semantic I/O와 재compile은 0이며 실패 시 이전 prepared pointer/cache를 보존한다.

R3 종료 조건은 399 opcode/629 distribution consumption, Geometry 7/7, Material 34/34, deterministic
fixed seed/time, fallback 0, Debug/Release PASS다. Product는 아직 false다.

### R4. 여섯 renderer family — typed sink 0/6 remaining

공통 render packet, vertex layout, MaterialBinding slot, family dispatch interface를 fresh captain이 먼저
동결하고 같은 captain이 여섯 family를 직렬 연결한다. Artist F M0~M3에서는 family implementation을
병렬화하지 않는다.

1. Mesh + Sprite: 13 + 16
2. Decal + Ribbon: 3 + 1
3. Light + ScreenPost: 1 + 1

각 family는 같은 compiled IR pointer를 소비하고 size/rotation/local-space/timing/material/render-state
numeric packet을 검증한다. raw `eKind`, raw SourceRecipe, legacy heuristic dispatch는 Product path에서 0이다.

### R5. Effect Tool dual mode, actual Artist F와 35/35

1. 기존 `CEffect_Tool` 하나에 Legacy Product와 Typed Restoration 두 mode를 둔다.
2. v14 Source Contract와 compiled IR은 read-only이고 조정은 occurrence-keyed overlay에만 저장한다.
3. Typed Play는 Authored disk reload가 아니라 exact catalog prepared revision을 attach한다.
4. publish -> catalog load -> prewarm -> equivalence -> target commit을 하나의 transaction으로 묶는다.
5. 35 occurrence 각각에 source/compiled identity, opcode, distribution, geometry, material, renderer,
   fixed sample expected/actual/tolerance를 기록한다.
6. `35/35`, Geometry `7/7`, Material `34/34`, fallback/blocker `0`일 때만 cue 전체 Product를 atomic publish한다.

### R6. Runtime 실행과 눈으로 확인

M2에서는 사용자가 Client를 `Client/Default` working directory에서 Debug로 실행해 actual Server/animation F action을
Debug-only nonProduct typed route로 확인한다. M3 자동 admission과 atomic Product publication 뒤 사용자가 같은 tree의
Debug와 Release에서 exact published revision을 재생한다. complete/family/occurrence filter가 같은 IR
pointer를 사용하는지, 이펙트가 실제 world/camera에서 누락·폭주·잘못된 scale 없이 재생되는지 수동 smoke한다.

정식 build는 저장소 root에서 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`와 Release를
직렬 실행한다. 실제 Client/UI 실행과 조작은 사용자가 `Framework.slnLaunch`의 `Server + Client` profile로 수행한다. Lobby에서
Server 승인을 받아 Character Select로 진입한 뒤 Artist thumbnail을 선택하고, Debug는 F1 Effect Tool의 Active Product Cue와
실제 F 입력을 모두 확인한다. Release는 F1 도구가 없으므로 Server command -> snapshot -> animation
event -> exact catalog prepared spawn의 실제 F 경로만 확인한다. `Lobby -> Test`는 캐릭터/네트워크 없는
Map Editor이므로 이 smoke에 사용하지 않는다.

M3 전에 `effect.artist.skill.31470`의 v13 identity carrier, Assembly, compiled artifact/receipt, format-3
Catalog entry, `PlayerSkills.json.effectId`, `sdm_sk_onestroke`의 generated `effectref=asset`이 같은 prepared
identity를 가져야 한다. M2 Debug-only route는 이 Product field를 거짓으로 열지 않는다. 현재 Product chain은
존재하지 않으므로 M3가 atomic하게 생성한다. Geometry WModel 7개와
compiled MaterialBinding에서 재도출한 DDS 전부를 hash-bind하며, exact recovery 4개
`fx_a_noise_011`, `fx_e_ring_001_cl`, `fx_a_decal_014`, `fx_c_atypical_016`도 canonical Resources에
publish됐는지 확인한다.

수동 timeline 기준은 0 ms Ink 4, 1338 ms Weapon 1, 1380 ms Swing 15, 1451 ms Hit 12와
ZoomBlur 1, 1452 ms Distortion 1, 1453 ms Light 1이다. 총 action 2.833초 안에서 종료되어야 하며 Mesh
scale/pivot/basis, Sprite billboard/SubUV, Decal projection/depth, Ribbon continuity/tail, Light radius/lifetime,
Post lifetime, Material fallback/depth/cull/sampler, attachment/world-space를 family와 occurrence filter로 확인한다.

이 단계는 numeric/structural admission을 대체하지 않는다. 외부 reference PNG는 사용자의 눈 검증에
사용하되 이미지 기반 자동 oracle로 만들지 않는다. 사용자가 대화에 첨부한 스크린샷이나 이미지의
분석을 요청하면 에이전트는 반드시 열람·분석해 관찰 결과와 가능한 occurrence 진단을 보고한다.
수동 관찰에서 이상이 보이면 해당 occurrence ID,
compiled revision, sample/seek time, reference path와 defect category를 기록하고 tuning overlay와 R3~R5의
numeric/structural fixture에 재현을 추가한 뒤 다시 admission한다. 35 occurrence 각각의 사용자
`APPROVED`가 없으면 M3 Product를 열지 않는다.

### R7. Artist F 최종 동결과 회귀

M2 수동 smoke에서 발견된 모든 이상을 occurrence ID와 compiled revision으로 overlay와 numeric/structural
fixture에 환류하고 35개 user APPROVED를 먼저 닫는다. M3/R7에서 새 이상이 나오면 Product publication을
rollback하고 M2로 되돌아가 새 Program SHA를 만든다. approved Program을 변경하지 않은 채 Debug/Release,
focused/deep ProjectAudit, publish/prewarm rollback, no-I/O prepared attach를 다시 통과하고 Source
`35/399/629`, Geometry `7/7`, Material `27/34`, renderer `13/16/3/1/1/1`, Product
`35/35`가 동일 frozen tree에서 유지될 때 Artist F compiler/runtime/renderer interface를 동결한다.
수동으로만 맞춘 값이나 Product path 전용 class/count 분기는 0이어야 한다.

### R8. 4-class와 Valtan 확장

Artist F runtime/compiler/renderer를 M3에서 동결한 뒤 raw/data-only worktree는 4-class corpus와 Valtan
Particle/Decal/Trail/Material/Camera corpus를 병렬 적용할 수 있다. shared C++ writer는 계속 한 명이다.
production code에 class/count switch를 추가하지 않고 fixture denominator만 확장한다. 공통 handler가
부족하면 corpus 값을 근사하지 않고 확장을 멈춘 뒤 shared pipeline gate로 되돌아간다.

latest raw inventory는 `9b7ac2dc571cc456fa6095f8f69b2df0ea54bd53`다. 4-class unique request 835와
Valtan unique request 377, 합계 1,212 중 1,208은 structural inspection을 끝냈다. TGA 4와
provenance/corpus 18, 총 blocker 22를 독립 review한 뒤 사용한다. extraction은 반복하지 않고 M3 GPU
schema 위에서 binding/materializer를 한 번만 재생성한다.

4-class source corpus는 Artist F 35개를 포함하지 않는 별도 분모다.

| class | skill/stage/clip | source occurrence | 기존 Product cue |
|---|---:|---:|---:|
| DimensionMaster | 12/15/22 | 872 | 19 |
| LanceMaster | 17/27/45 | 2,199 | 41 |
| Artist (`31470` 제외) | 9/15/16 | 841 | 14 |
| Warlord | 13/17/30 | 1,320 | 27 |
| 합계 | 51/74/113 | 5,232 | 101 |

portable inventory revision을 먼저 고정하고 Artist 9-skill canary -> DimensionMaster -> Warlord ->
LanceMaster 순으로 cue/document atomic admission한다. source 51문서와 기존 Product 101 cue는 서로 다른
분모이므로 합치지 않는다.

Valtan의 정본 분모는 action 170, stage 2,464, clip 2,378, notify 21,931이며 typed target은 Particle
6,159(`PlayParticleEffect` 6,026 + quarantined `DefaultParticle` 133), Decal 536, Trail 430, Material
606, Camera 1,022다. generic `Effect` 3,787은 typed target으로 승격하지 않고 격리한다. 세 Action LOA를
raw SHA로 pin한 뒤 Particle -> Decal -> Trail -> Material -> Camera 순으로 적용한다.

R1 historical exact provider는 없지만 사용자가 reconstructed high-ceiling 정책을 승인해 R2 transport와
M0 구현이 진행됐다. evidence blocker는 보존하며 explicit policy/capability와 실제 consumer proof로만
execution blocker를 해소한다. typed consumer, renderer, actual Client smoke와 final regression 전에는
화면에 보이는 legacy effect나 harness packet을 복원 완료로 판정하지 않는다.

## 기존 G 상세 계약

아래 G00~G11은 원래 계획의 상세 acceptance contract다. 현재 실행 순서는 앞의 R1~R8을 정본으로
사용하며, 이미 PASS한 G를 재구현하지 않는다.

## G00. Known-red base와 public handshake 동결 (historical)

Integration Captain이 `068fcc2` 위에 이 계획과 최소 public handshake를 고정한다.

고정 필드는 다음과 같다.

- `(sourceOccurrenceId, sourceSystemId)` composite emitter identity
- selected LOD node/path/provenance
- ordered module reference index, role, stable ID, exact class/alias lineage
- property `(storage, canonicalPropertyPath, referenceId)`
- distribution definition/occurrence/reference ID, typed payload variant, fidelity, blocker, admission
- PointLight typed component binding과 instance/current inherited default 분리
- GeometryBinding expected tuple
- MaterialRecipe/MaterialOccurrence ID와 contract hash
- renderer family와 source space
- canonical source document hash, compiled IR hash, catalog/compiler revision

G00 종료 조건:

- 모든 Wave 1 branch가 같은 G00 commit에서 시작한다.
- Product 0/35와 known-red baseline 세 항목을 기록한다.
- Source/Compiler가 공통 schema/Codec를 각각 수정하지 않는다.
- 부족한 field는 Integration에 최소 typed request로 제출하며 승인 전 다른 field에 smuggle하지 않는다.

## Wave 1A. Evidence와 generic compiler core 병렬

### G01. Source Evidence Final

Source Closure는 local distribution만이 아니라 selected-LOD 전체 semantic evidence와 reconstruction
oracle을 소유한다.

- 15 distribution target / 17 occurrence와 PointLight 1 occurrence
- 399 ordered module reference, 1,434 top-level property, 1,572 primitive leaf, 629 distribution
- `INSTANCE_EXPLICIT -> NESTED_ARCHETYPE_TEMPLATE -> CLASS_CDO -> PARENT_CDO_HIERARCHY -> EVALUATOR_DEFAULT`
  순서의 field provenance
- target000/001/007/009/014와 custom evaluator의 pre-payload rejection
- raw decoded evidence와 executable typed payload의 분리
- external native-tail 248, seeded module 14, Required local-space default 8, Decal default 3,
  Ribbon default 1, ScreenPost default 1과 selected-LOD/class default의 closure 또는 실행 무관 증명
- standard UE3 module은 tagged instance/archetype/CDO/script/native contract를 결합하고 custom EF class는
  별도 native/differential numeric oracle 없이는 standard handler로 분류하지 않음
- 각 property를 `EXECUTION_CONSUMED`, `VERIFIED_IRRELEVANT`, `UNRESOLVED` 중 하나로 분류하고 근거 receipt 고정
- evidence/artifact/execution/Product blocker를 분리해 definition/property/module/element/receipt/registry/compiler로 전파
- code-only checkpoint와 마지막 bounded source-era artifact 조사

Source-era artifact가 없으면 source fidelity는 `UNRESOLVED` 또는 `CURRENT_REVISION_EVIDENCE`로
동결한다. current package constant를 old child payload로 사용하지 않는다. 실행 의미는 별도의
`RECONSTRUCTED_NUMERICALLY_VERIFIED` receipt가 있는 경우에만 열 수 있으며, standard/native evaluator와
default/seed가 numeric output에 미치는 영향을 독립 sample로 고정한다. Geometry receipt가 확정되기 전
candidate/header를 최종 재생성하지 않는다.

합격:

- 15 definition/17 occurrence와 PointLight active reference all-consumed
- 399 module, 1,434 top-level property, 1,572 leaf, 629 distribution이 consumed 또는 verified-irrelevant이며
  unknown/unconsumed/silent-ignored 행 0
- payload decoded/unresolved, semantic ready/blocked, compiled/admitted 분모를 각각 보고
- target000/001/007/009/014, seed/native-tail/default/Light archetype-CDO mutation이 production resolver에서 실패
- code-only commit은 pure resolver/binder, temp-output generation, mutation test만 PASS로 주장
- checked-in `--check`, deep audit, Codec round-trip은 Gate 1 regeneration commit에서 실행
- Product false

### G02. Geometry Evidence Final — PASS `0aca7928`

Geometry lane은 WModel evidence와 Engine decode contract만 닫는다. `Effect_Playback`과
`Effect_DocumentRenderer`는 수정하지 않는다.

필수 작업:

- strict JSON integer version과 canonical EOL/raw hash domain
- Python/C++ 동일 tangent-W 허용 계약
- writer-independent immutable golden과 actual 7-carrier C++ channel/index/bounds/hash oracle
- actual Resources legacy v1.0 static/skinned/hasBounds/multi-submesh corpus C++ sweep
- full transactional rollback
- self-consistent metadata와 externally bound expected identity 분리
- pivot와 exact UPK→glTF가 없으면 source blocker 유지

합격:

- frozen commit 독립 PASS
- actual 7 carrier C++ Debug/Release 7/7
- legacy corpus 2,586 decode regression
- Engine -> UpdateLib -> Client Debug/Release
- Resources와 Effect runtime diff 0
- Product false

### G03. Material Evidence Final — `cde8f3bd` evidence PASS, execution 0/255 BLOCK

Material lane은 raw evidence와 typed recipe만 닫고 HLSL/runtime을 수정하지 않는다.

필수 작업:

- source-pack manifest를 통한 logical package와 raw export identity 결합
- 342 scalar / 19 vector / 71 texture의 raw array order/name/value/reference/owner 결합
- MIC Parent edge와 selected base graph exact identity
- Texture2D export/serial/tagged sampler와 DDS binding
- parent/default/render/static field owner lineage
- recipe composition digest와 34 occurrence identity 연결
- coordinated closure/receipt reseal, label/parentGraph/inputs/defaults/sampler owner swap 거부
- checked receipt/contract 재생성으로 현재 hash mismatch 제거
- MIC native `FStaticParameterSet`, Engine/EFGame Material CDO, Texture sampler default, installed shader cache와
  surviving expression/native material metadata를 조사해 static/full-render/full-cull/default/sampler/graph별
  reconstruction oracle receipt 생성
- render-state 89, static 94, strict sampler 72, graph 23/502 edge를 각각 행 단위로 보존하며 값이 없는
  행을 공통 fallback으로 채우지 않음

합격:

- recipe row 27/27, occurrence 34/34, unused/unexpected recipe 0
- sampler exact 0, rejected legacy 3 instance + 1 parent, direct unproven 71, strict sampler 72
- graph family 23, null 1,803, unresolved edge 502, Source-exact graph 0
- shallow/deep generator, raw UPK/DDS/manifest audit와 mutation tests
- 23 family의 independent expected sample 출처와 아직 구현되지 않은 evaluator를 별도 기록
- evidence commit에서는 evaluator implemented 0과 Product false를 정직하게 유지하되,
  Gate 1 실행-readiness matrix에서 owner 없는 material blocker 0

### G04. Generic Compiler Core — non-executable checkpoint PASS `c4b00f14`

Compiler lane은 Source 재생성 전 generic typed core만 구현한다.

필수 작업:

- System -> Emitter -> selected LOD -> ordered module -> property/distribution immutable IR
- opcode별 allowed/required schema와 실제 handler consumption receipt
- canonical document identity와 full-field deterministic IR hash
- admission-before-payload, unknown/duplicate/nonfinite/alias/LOD mutation rejection
- generic production invariant와 Artist fixture denominator 분리
- Source generated header read-only 소비

G04에서는 current generated candidate의 최종 adapter PASS를 주장하지 않는다. `7/35/399/629`와
`13/16/3/1/1/1`은 harness fixture에만 둔다. production에 Artist/31470/count hardcode를 넣지 않는다.

합격:

- raw module payload materialization 0
- generic inspection/compiler Debug/Release harness와 static audit
- Product false

## Gate 1. Frozen audit, 통합, Source 단일 재생성 — historical, R2가 supersede

각 G01-G04 lane은 stable commit을 push하고 mutation을 멈춘다. Independent Review는 commit SHA 하나만
검사하고 read-only PASS/BLOCK을 보낸다. 구현 helper와 같은 parser만 사용한 자기확인은 PASS가 아니다.

이 절의 옛 cherry-pick 순서는 사용하지 않는다. 실제 재개 순서는 R2를 정본으로 하며 base
`38ebe7cf`, Geometry `669acf07`과 `0aca7928`의 개별 cherry-pick, compiler/publisher 재-pick 금지,
`4ffe1102` 폐기와 combined head 재작성을 따른다. 이 historical 문구의 통합 금지는
`RECONSTRUCTED_APPROVED_V1` 승인 전 상태를 설명한다. 승인 뒤 실제 통합은 R2 절의 exact 14-commit
순서와 새 policy/materializer gate를 따른다.

Gate 1 종료 조건:

- stale generated output 0
- Source 7/35/399/629와 renderer 분모 보존
- Material 27/34와 Geometry 7 hash를 compiled input receipt가 결합
- actual regenerated candidate의 Codec round-trip
- occurrence별 remaining execution blocker matrix와 owner가 존재
- owner 없는 blocker가 하나라도 있으면 Renderer 단계로 진입하지 않음
- Product false, integration worktree clean

## Gate 1B. Execution-readiness와 runtime resource provisioning

Renderer나 runtime authority를 수정하기 전에 네 병렬 closure job으로 실제 실행 입력을 준비한다.

### G05-S. Source Semantic Reconstruction Readiness — `c927e397 -> 7da937ae`, evidence PASS/readiness 0/29 BLOCK

G01에서 남은 standard/native/default/seed/custom evaluator 행을 actual regenerated candidate 기준으로
다시 계산한다. 각 399 module과 1,434 top-level property는 실행 handler가 소비하거나 독립적으로
실행 무관임을 증명해야 한다. target007/014 custom EF evaluator, external native-tail, seed와 local-space,
Decal/Ribbon/Post/Light default에 evaluator ID와 numeric oracle을 부여한다. current-only evidence는
source exact로 승격하지 않는다.

접근 가능한 provider acquisition은 끝났고 source-era provider/pilot은 0이다. 아래 semantic 작업은 동일
revision artifact 또는 승인된 authenticated capture가 들어와 R1을 다시 열 때만 재개한다.

### G05-G. Geometry Candidate Cook and Resource Provisioning — PASS `0aca7928`

G02의 7개 WModel 1.1 candidate를 deterministic cook하고 expected tuple을 생성한다. candidate는 먼저
임시 staging directory에서 검증한 뒤 팀장 관리 물리 root인
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/Artist/Meshes`의 정확한 7개 target만
backup -> atomic replace한다. 실패하면 기존 7개를 복원한다. Git worktree의 Resources 유무를 제품
payload 존재 증거로 사용하지 않고 test process의 `LOSTARK_RESOURCE_ROOT`를 위 물리 root로 고정한다.

### G05-M. Material Reconstruction Oracle and Texture Provisioning — `cde8f3bd` evidence PASS/readiness 0/255 BLOCK

current Engine/EFGame CDO와 installed cache는 cross-revision 후보일 뿐 source value provider가 아니다.
`cde8f3bd`는 source MIC static override value 23건만 보존했고 render/sampler 및 execution readiness는
열지 않았다. 동일 revision provider 또는 승인된 authenticated capture가 들어올 때만 27 recipe의
static/render/cull/default/sampler와 23 arithmetic family expected numeric sample을 닫는다.
모든 34 occurrence가 참조하는 texture dependency를 열거하고 기존 Resources DDS 또는 새로 검증한 DDS를
명시적인 asset ID/hash로 stage한다. white/black/legacy-default texture를 missing input 대체값으로 쓰지 않는다.
물리 texture 교체가 필요하면 G05-G와 같은 backup -> validate -> atomic replace -> rollback을 사용한다.

### G05-P1. Derived Artifact and Publisher Schema Code

v14 Source Contract 자체는 계속 read-only/non-drawable이다. 이 lane은 v14 Source, Geometry,
Material contract와 semantic reconstruction receipt를 결합할 `lostark.effect-authoring` v13
derived authoring/Assembly identity carrier와 compiled artifact의 schema만 구현한다. v13 carrier는
runtime semantic authority가 아니며 생성물은 다음 identity를 저장한다.

```text
sourceContractHash
sourceSemanticClosureHash
geometryContractHash
materialContractHash
resourceBindingHash
compilerInputHash
```

이 병렬 lane은 generator, serializer, publisher validator, runtime catalog format 3 schema와 synthetic
zero-blocker fixture만 구현한다. G05-S/G/M의 최종 hash를 요구하거나 actual Artist F v13
document/Assembly/compiled artifact를 생성하지 않는다.
`Publish-Effects.ps1`은 verified v13 derived document와 compiled artifact를 함께 검증하고 v14를 직접
publish하지 않는다. runtime catalog format 3은 위 identity와 compiled receipt/revision을 보존한다.
actual generator는 execution blocker가 하나라도 있으면 v13 document/Assembly를 만들지 않고 이전
Product를 유지하며, G10 이후 모든 final hash가 동결된 Gate 10B에서 한 번 실행한다.

Gate 1B 종료 조건:

- unknown opcode/property, unconsumed property, silent ignored native/default field가 각각 0
- Source semantic, Geometry payload, Material recipe/resource, derived Product 입력에 owner 없는 blocker 0
- runtime Resources의 7 WModel과 모든 required DDS가 compiled expected identity와 일치
- G05-P1 synthetic zero-blocker fixture만 v13/schema/publisher round-trip을 통과
- actual Artist v13/compiled artifact는 아직 생성하지 않고 Product bit/publish는 false
- Source exact가 아닌 reconstruction 행의 evidence blocker는 보존

## G06. Runtime Authority Foundation — PASS `38ebe7cf`

G05-S/G/M/P1 frozen PASS 뒤 단일 owner가 중앙 runtime authority와 extension point를 먼저 고정한다.
이 G 뒤에는 family/geometry/material lane이 중앙 파일을 다시 수정하지 않는다.

소유 파일:

- `Effect_CascadeCompiler.h/.cpp`
- `Effect_Playback.h/.cpp`의 prepared input boundary
- `Effect_DocumentRenderer.h/.cpp`의 common cache, upload와 family dispatch boundary
- `Effect_PresentationService.cpp`
- `Effect_Catalog.cpp`
- `Effect_Object.h/.cpp`
- common shader binding layout
- `Publish-Effects.ps1`과 runtime catalog format 3 parser/stager
- `Engine/Public/Engine_Struct.h`
- `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`

필수 계약:

- immutable prepared identity `{catalogRevision, compilerRevision, assetId, sourceHash, compiledHash,
  geometryHash, materialHash, resourceBindingHash}`
- v14 Source/Geometry/Material에서 immutable execution IR을 만드는 최종 adapter, opcode shape와
  execution admission compile을 G06이 소유하고 이후 G07은 compiler 파일을 수정하지 않음
- 각 opcode에 required handler ID/version을 고정하고 G07-G10이 fixed extension point로 제출하는
  capability/consumption receipt가 없으면 execution admission을 열지 않음
- Source/Geometry/Material을 한 번 compile하고 prepared pointer만 consumer에 전달
- Catalog -> Presentation -> Object -> Playback -> Renderer가 같은 compiled document identity를 확인
- central family interface, geometry cache key, material evaluator handle와 shader slot 선할당
- typed Light descriptor에 radius/falloff/color/brightness를 명시하고 `CEffectObject`와 deferred shader가
  고정 linear attenuation으로 세탁하지 않을 public binding 고정
- publish/load/prewarm/equivalence 전체 stage -> validate -> commit과 실패 rollback
- combat Spawn의 compile/model/DDS/vector-field I/O 0
- compile A 뒤 raw B attach, revision/hash/pointer/Product mismatch 거부
- `v14 Source Contract -> immutable compiled execution IR`만 runtime semantics의 단일 권위이며,
  v13 derived document/Assembly는 Catalog identity와 authoring carrier일 뿐 Playback/Renderer가 raw
  semantics를 읽지 않음

G06은 아직 family output을 Product로 승인하지 않는다. 중앙 Renderer의 v14 dispatch는 compiled family
enum만 사용하고 unsupported family는 fail-close한다. Light public payload와 deferred shader 변경은 같은
SHA/worktree에서 `Engine -> UpdateLib -> Client` Debug/Release build lease를 사용한다.

합격:

- actual `Prepare_Catalog -> Find_Prepared -> PresentationService::Spawn -> EffectObject` harness
- compile 1, attach 1, runtime I/O 0
- Catalog/Object/Renderer/Presentation rollback에서 이전 catalog/cache/targets 보존
- 중앙 public API와 common shader 동결
- Product false

## G06B. Source Runtime Program Materializer — `4ffe1102` 폐기, R1 readiness 뒤 R2에서 새로 작성

Source execution receipt의 35 emitter/399 module/629 distribution을 immutable program으로 옮긴다.
`4ffe1102`를 cherry-pick하거나 mutation하지 않는다. R1 readiness가 모두 PASS한 combined head에서
denominator와 fail-closed 요구만 새 구현의 acceptance contract로 재적용하고 generic JSON bag은 runtime
authority로 승격하지 않는다.

추가 합격 조건:

- ParticleParameter 17 occurrence의 name/mode/four-range/fallback/oracle/provenance typed coverage 100%
- PointLight typed component field와 handler receipt coverage 100%
- seed/default closed typed variant coverage 14/14
- READY distribution 629/629 closed evaluator registry join
- exact class -> opcode -> handler -> allowed/required property schema와 consumption receipt
- source record/value, handler SHA, lookup shape, emitter/opcode order, nested key mutation transactional reject
- tracked LF/CRLF에서 동일 canonical program SHA
- Debug/Release actual candidate mutation harness
- runtime execution admission false, Product false

## Wave 2A. Typed executor, Geometry와 Material consumer 병렬

### G07. Typed Cascade Executor — remaining

G06이 최종 compile한 immutable execution IR을 strict consumer로 받아 `CEffectPlayback`의 v14
production simulation이 typed opcode만 소비하도록 바꾼다. G07은 `Effect_CascadeCompiler`를 수정하거나
별도의 IR 승격 경로를 만들지 않는다. G06이 동결한 handler extension point에 Playback capability와
consumption receipt만 등록한다.

필수 opcode/handler 범위:

- Constant, Uniform, ConstantCurve
- standard Float/Vector ParticleParameter의 Direct/Normal/Abs와 네 range
- oracle이 있는 custom EF evaluator의 별도 opcode
- spawn, burst, lifetime, delay, loop, size, rotation, location, velocity, acceleration, color
- local/world/source space와 axis lock
- SubUV, spawn-per-unit, location-on-ground
- TypeData Mesh/Decal/Ribbon/Light
- ActionCue parameter binding과 none binding
- fixed seed, fixed timestep, deterministic sample order
- `Effect_Playback.cpp`의 Mesh StartSize hidden `x0.01`을 제거하고 signed dimensionless axis reorder만 유지

합격:

- 399 ordered reference, 1,434 property, 1,572 leaf, 629 distribution handler receipt
- Playback raw `SourceRecipe.Modules`, 문자열 class/property scan 0
- unresolved/custom payload read 0
- fixed seed/time repeated run deterministic
- failed compile/stage에서 이전 playback/prepared revision 보존
- Debug/Release actual candidate harness
- Product false

### G08. GeometryBinding Runtime Consumer — remaining

G02/G05-G 결과를 기존 `CModel -> CMesh -> CMaterial` 경로와 G06 extension point에 연결한다.

필수 작업:

- expected tuple `{assetId, payloadHash, metadataIdentity, geometryPreScale=0.01,
  DIMENSIONLESS_AXIS_REORDER_ONLY}`와 decoded bytes 대조
- vertex와 embedded/derived bounds에 preScale 정확히 한 번 적용
- prepared model cache identity에 tuple 전체 포함
- G07에서 hidden `x0.01`이 제거됐음을 검증하고 signed dimensionless axis reorder와 carrier preScale이
  정확히 한 번씩만 적용되는지 확인
- Sprite/Decal cm-to-m 경계와 분리
- tangent-W/COLOR sidecar를 Mesh family shader가 소비할 immutable vertex binding까지 전달

합격:

- 7 carrier / 13 Mesh occurrence combined magnitude numeric oracle
- cache collision, wrong hash, double/zero/nonfinite scale, final bounds mutation 거부
- failed load/prewarm에서 prepared cache residue 0
- source fidelity와 external authentication 상태 보존
- Product false

### G09. Material Recipe Compiler and Evaluator — remaining

G03/G05-M의 27 recipe를 G06 extension point가 소비할 immutable MaterialBinding으로 compile한다.

필수 작업:

- exact scalar/vector/texture input과 sampler/render-state payload
- source-exact input과 reconstructed arithmetic family의 분리
- 23 family별 stable evaluator ID/version과 implemented state
- Add/Multiply/Panner를 포함한 실제 관측 연산을 typed evaluator/HLSL로 구현
- static permutation, blend/depth/cull/default/sampler는 source 또는 G05-M oracle이 있는 값만 compile
- strict sampler 72(instance 71 + parent 1)와 omitted default를 공통 fallback으로 채우지 않음

합격:

- 27 recipe / 34 occurrence handler consumption
- per-family independent numeric sample과 mutation test
- unknown expression/input/sampler/render state 실행 0
- static/render/cull/default/sampler execution blocker 0
- reconstructed graph를 `SOURCE_EXACT`로 승격한 행 0
- Product false

## G10. Renderer family와 Effect Tool — historical detail, current single captain

G07-G09 통합·독립 감사 뒤 G06의 중앙 Runtime Authority/Foundation API를 다시 동결한다. 아래 옛
family lane 분할은 파일 경계 참고용이며 현재 M0~M3에서는 fresh captain 한 명이 순서대로 구현한다.

G06 Foundation이 이미 소유하고 family가 변경하지 않는 계약:

- compiled render packet
- common particle attributes와 CPU/GPU upload layout
- immutable geometry/material binding handles
- renderer family dispatch interface
- central dispatcher와 common shader binding slot
- family 구현의 별도 파일/project/filter 경계

중앙 header, dispatcher, common shader는 동결한다. family 구현은 지정된 파일과 shader 경계를 지킨다.

family 검증 순서:

| lane | 분모 | numeric/structural 종료 증거 |
|---|---:|---|
| Mesh + Sprite | 13 + 16 | size/rotation/local-space/lifetime, geometry/material binding, tangent/COLOR input, double-scale mutation |
| Decal + Ribbon | 3 + 1 | projection, signed source space, segment ordering, spawn-per-unit, trail lifetime, invalid topology rollback |
| Light + ScreenPost | 1 + 1 | exact child Brightness/flags, current inherited default 분리, `LIGHT_DESC` radius/falloff/color/brightness 전달, deferred attenuation numeric oracle, post weight/lifetime, unsupported recipe rejection |
| Effect Tool | 35 inspection rows | Source Evidence/IR read-only tree와 Draft/Product Play 경계 |

Effect Tool 계약:

- v14 `SOURCE_CONTRACT`는 read-only이며 Save, Apply, resource replace, execute를 비활성화한다.
- Source tree는 7 System -> 35 Emitter -> 35 selected LOD -> 399 ordered module -> 629 distribution 순서다.
- Legacy mode는 기존 Authored Draft Preview/Save를 유지하고 Typed Restoration mode는 versioned tuning
  overlay만 Save한다.
- Product Play는 Authored 파일을 disk에서 다시 load하지 않고 exact catalog prepared revision을 attach한다.
- Complete/family/occurrence solo는 같은 IR pointer의 transient mask이며 compile/model/texture I/O가 0이다.
- Tool 전용 두 번째 renderer나 Catalog 우회를 만들지 않는다.

fresh captain은 family별 frozen checkpoint를 같은 branch에 순서대로 commit/push하고 독립 감사를 받은 뒤
다음 family로 전진한다. 별도 family branch를 cherry-pick하지 않는다. six-family 완료 뒤 Tool의 exact
prepared attach/no-I/O와 Legacy↔Typed rollback을 작은 후속 checkpoint로 재검증한다.

G10 통합 종료 조건에는 Renderer와 Presentation의 raw `eKind`, raw SourceRecipe/module/default scan 0을
포함한다. compiled family enum과 typed render packet 이외의 dispatch는 v14 Product path에서 거부한다.

## Gate 10B. Actual Artist Derived Artifact 단일 생성

G05-P1은 schema/publisher code만 만들었으므로 actual Artist artifact 생성은 Source semantic,
Geometry/Material contract와 resource binding, G06 execution compiler, G07-G10 handler/renderer의 final
hash가 모두 동결된 뒤 Integration이 한 번만 수행한다.

생성 순서:

1. final v14 Source Contract와 Source semantic reconstruction receipt를 읽는다.
2. final Geometry/Material contract와 실제 Resources binding hash를 결합한다.
3. G06 offline compiler가 v14-derived immutable execution IR과 compiled receipt를 생성한다.
4. G05-P1 generator가 같은 six-hash identity를 가진 v13 derived authoring/Assembly carrier를 생성한다.
5. publisher가 v13 carrier와 compiled artifact의 identity/revision 일치를 검증해 catalog format 3을
   임시 stage하되 Product bit와 기존 catalog publish는 계속 false/미변경으로 둔다.

실행 의미의 단일 권위는 3번의 compiled IR이다. v13 carrier의 raw Assembly/SourceRecipe는 Catalog ID,
Effect Tool Draft/inspection과 provenance 연결에만 사용하며 Playback/Renderer가 실행하지 않는다.
execution blocker, unconsumed handler, resource hash mismatch 또는 six-hash 불일치가 하나라도 있으면
두 artifact와 catalog stage를 전부 rollback하고 이전 Product를 유지한다. 일부 hash만 바꿔 반복
재생성하지 않고 최종 입력 묶음 전체를 한 transaction으로 생성한다.

Gate 10B 종료 조건:

- actual Artist v13 carrier와 compiled IR/receipt의 six-hash identity가 동일
- 399/1,434/1,572/629 consumption receipt와 renderer 13/16/3/1/1/1이 compiled artifact에 고정
- actual Resources 7 WModel과 required DDS expected identity 일치
- raw v13 runtime semantic consumer 0
- Product false, 기존 published catalog/cache/targets 미변경

## G11. Artist F Admission Oracle와 Product publish — remaining

Integration이 새로운 feature를 만들지 않고 전체 oracle만 실행한다. expected 값은 compiler/renderer가
자기 출력으로 다시 만든 값이 아니라 raw evidence, 독립 decoder, hand-authored immutable golden 또는
native numeric oracle에서 온다.

각 35 occurrence row는 다음을 기록한다.

- source fidelity와 원본 blocker
- artifact binding integrity
- execution admission과 evaluator ID/version
- source/compiled document identity
- tuning overlay SHA와 해당 occurrence user decision
- geometry/material binding과 hash
- selected LOD와 ordered opcode coverage
- renderer family
- fixed seed/timestep/ActionCue input/sample time
- expected/actual/tolerance
- Product blocker와 최종 decision

전체 consumption gate는 분모를 축소하지 않는다.

```text
ordered module reference 399/399
top-level property       1,434/1,434 consumed or verified-irrelevant
primitive leaf           1,572/1,572 consumed or verified-irrelevant
distribution             629/629 compiled or verified-irrelevant
unknown opcode           0
unknown property         0
unconsumed property      0
silent ignored native/default field 0
```

최종 자동 gate:

```text
Source contract shallow/deep and all mutation tests
Geometry actual 7-carrier Debug/Release C++ oracle and legacy corpus
Material 27/34 shallow/deep raw UPK/DDS/manifest oracle
Compiler/Executor Debug/Release actual candidate oracle
Six renderer family 35-occurrence numeric packet/state oracle
Effect Tool Legacy/Typed mode, read-only IR, overlay seal/recompile, revision and no-I/O transaction oracle
legacy Product cue projection 101/101 and all non-target Catalog entry delta 0
Legacy <-> Typed mode switch failure rollback preserves prior selection/prepared pointer
Engine Debug/Release
UpdateLib Debug/Release
ClientFrontendHarness Debug/Release
Client Debug/Release
focused Artist ProjectAudit checks all PASS
full Invoke-ProjectAudit executed with the canonical physical Resources root
all Artist/Effect-related full-audit failures fixed
unrelated baseline failure ID/detail delta 0
JSON/XML parse
git diff --check
conflict marker and unmerged path 0
residual Client/Server process and listener check
```

Product predicate가 35/35 true일 때만 Artist F cue를 Catalog에 atomic publish한다. partial Product나
missing occurrence 축소는 허용하지 않는다. publish 뒤 exact revision의 load/prewarm/Spawn을 다시 실행하고
실패하면 이전 catalog/cache/targets를 유지한다.

publish 대상은 Gate 10B의 verified v13 derived authoring/Assembly identity carrier와 runtime catalog
format 3 compiled entry다. v14 Source Contract를 drawable로 바꾸거나 Catalog가 v14를 직접 읽게 하지
않으며 v13 raw semantics도 실행하지 않는다. publisher, catalog stage, prepared compiler와 runtime
Spawn은 같은 six-hash identity와 compiled IR pointer를 확인해야 한다.

G00에서 canonical Resources root를 지정해 full ProjectAudit baseline report를 생성하고 failure ID/detail을
고정한다. Artist/Effect와 직접 관련된 baseline failure는 G11까지 전부 닫는다. Map/Character 등 이 기능과
무관한 baseline failure가 남으면 full audit 전체를 PASS라고 쓰지 않고, exact same failure 집합이며 신규
failure가 0이라는 비교 증거를 남긴다.

## Build lease와 commit 규칙

Integration Captain이 전역 full-build lease 하나를 관리한다.

- Engine, UpdateLib, Client full build는 한 번에 한 worktree만 실행한다.
- Debug와 Release도 직렬화한다.
- Engine public 변경은 같은 SHA/worktree에서 `Engine -> UpdateLib -> Client`를 실행한다.
- 다른 worktree의 `EngineSDK`, `.lib`, generated header를 복사해 PASS를 만들지 않는다.
- Python/unit은 output path가 겹치지 않을 때만 병렬 실행한다.
- `__pycache__`, temp JSON, build/intermediate, ignored EngineSDK를 stage하지 않는다.
- lane commit은 하나의 검증 단위이며 frozen audit PASS 뒤 rebase하지 않는다.
- correction은 같은 lane의 후속 commit으로 만들고 새 SHA를 다시 감사한다.
- lane branch는 checkpoint push 용도이며 main에는 최종 Integration PR 하나만 연다.

## 도화가 F 이후 확장

G11의 Product 35/35와 동일 prepared runtime path가 닫히기 전에는 다른 class와 Valtan occurrence를
Product로 확장하지 않는다. 완료 뒤에는 production code에 Artist-specific count/switch를 추가하지 않고
같은 generic compiler, material evaluator, geometry binding, six-family renderer와 Tool contract를 사용한다.

확장 순서:

```text
도화가 F golden fixture
-> Artist 9-skill canary (31470 제외)
-> 차원술사
-> 워로드
-> 창술사
-> 발탄 Particle/Decal/Trail/Material/Camera occurrence
-> 전체 corpus regression과 공통 renderer 최적화
```

Artist의 `7/35/399/629`, renderer `13/16/3/1/1/1`, material `27/34`는 fixture harness에만 남긴다.
production compiler/renderer/catalog에 이 숫자를 hardcode하지 않는다.

## 2026-08-12 V3 최종 복원 시도: occurrence admission 전환

이 절은 기존의 `Full35를 한 화면에 표시`하는 검토 방식을 폐기하고 V3의 화면·판정 계약을
재정의한다. 원본 35행과 진단 기능은 보존하지만, 행이 존재하거나 versioned shader를 가진다는 이유로
복원 성공 또는 Complete 출력에 포함하지 않는다. Product가 false라는 기존 경계도 유지한다.

### V3 목표와 현재 정직한 분모

- 원본 Program과 policy의 현재 값은 `sourceExact=false`, `runtimeExecution=false`,
  `product=false`, `MANUAL_HUMAN_EYE_VALIDATION_0_OF_35`다.
- 따라서 `A / ADMITTED_EXACT`는 현재 **0/35**다.
- source carrier와 versioned/typed 경로가 있어 개별 검토 가치만 있는
  `B / CONDITIONAL_REVIEW`는 **13/35**다.
- graph/channel/state 또는 실행 의미가 닫히지 않은 `C / DIAGNOSTIC_ONLY`는 **22/35**다.
- V3 합성 화면은 B 전체도 아닌 본체 검토용 `#9/#10/#11` 세 행만 명시적으로 노출한다.
  이 세 행도 복원 성공이 아니라 사용자 육안 검토 대상으로 표시한다.
- `Complete` 기본 scope는 A만 소비하므로 현재 빈 화면이어야 한다. Full35는 inventory/원인 진단에서만
  사용한다.

등급 승격 조건은 다음과 같다.

| 등급 | 의미 | 필수 조건 | 합성 기본값 |
|---|---|---|---|
| A `ADMITTED_EXACT` | 복원된 원본 행 | source exact identity, geometry/material/render state/sampler/transform/timing 전부 닫힘, neutral fallback 0, 자동 golden PASS, 사용자 occurrence 육안 승인 | 표시 |
| B `CONDITIONAL_REVIEW` | bounded/typed 검토 후보 | stable occurrence ID, versioned 구현, 입력 소비·억제 목록, 유한 bounds, 자동 packet PASS. source graph 일부 유실과 사용자 승인 미완료를 명시 | 명시 Review에서만 표시 |
| M `MANUAL_TUNED` | 복원 중단 뒤 수동 완성 행 | 사람이 정한 mesh/material/envelope/timing revision과 사용자 승인. 복원 통계와 분리 | 승인된 수동 합성에서만 표시 |
| C `DIAGNOSTIC_ONLY` | 미복원/차단 | generic numeric oracle, neutral visible fallback, 필수 texture/channel/state/transform 의미 누락, 과대 carrier 또는 미승인 | 숨김, Solo 진단만 |

### 첨부 화면 네 시점의 owner와 원인

| 시점 | 관찰 결함 | 가능한 원본 행 | 확인된 원인 | V3 처리 |
|---:|---|---|---|---|
| 1.059s | 검은 판/쐐기와 보라 노이즈 | 이 시점에는 #0~#3만 가능. fixed-step particle은 #2가 9개이며 #3은 ribbon | #2는 renderer BASE와 occurrence shader가 없고, #3도 dynamic 4 lane과 원본 graph를 복원하지 못한 bounded ribbon | #0~#3 전부 합성 차단. #3만 별도 Solo 후보 |
| 1.670s | 흰색·형광 녹색 면 폭발 | #4, #7~#18 main group과 hit/decal/sprite가 동시 중첩 | generic seven(#7,#8,#12~#15,#18)은 실제 graph가 아닌 고정 UV numeric oracle. bounded 행도 alpha/mask/ParticleColor edge가 누락됨 | main review #9/#10/#11만 표시 |
| 2.177s | 거대한 파란 곡면과 페인트/바닥 판 | main #7~#18, blob #16/#29/#30, decal #20~#22 | 큰 부채는 한 mesh가 아니라 여러 carrier 합성. #16은 50개 장수 sprite, #29/#30은 15개씩, decal은 큰 projection과 긴 lifetime | #11만 이 시점에 남고 나머지 차단 |
| 2.610s | 잔류 흰 쐐기·페인트·바닥 | #7/#8/#18, #16/#29/#30, #20/#21/#22 | #7/#8 scale 11.4와 #18 scale 9의 긴 carrier, #16 life 3~4, #20/#21 life 2.8, #22 life 4라 데이터상 잔류가 예정됨 | V3 main review는 전부 종료되어 무출력 |

`#9~#15` Program에는 선택된 Position/Velocity가 없다. 이 본체의 위에서 아래로 내려오는 인상은
arc WModel 형상, mesh rotation, UV/dissolve reveal의 결합일 가능성이 높다. 따라서 기준 영상이나 원본
transform edge 없이 downward/ballistic translation을 추가하지 않는다. 실제 source velocity/gravity가 있는
것은 파편 #26이며 main swing과 역할이 다르다.

### 35행 exact admission 표

`G`는 geometry/transform, `M`은 material/render 의미다. `B`는 성공이 아니라 검토 가능이라는 뜻이다.

| # | family / carrier | 등급 | G 상태 | M 상태와 핵심 reason | V3 |
|---:|---|:---:|---|---|---|
| 0 | Sprite skull | C | 초기 anchor 의존 | renderer base 없음, graph 미복원 | 숨김 |
| 1 | Sprite master | C | billboard schedule 존재 | 4-texture graph/channel 미복원 | 숨김 |
| 2 | Sprite wind | C | 1.059s 9개 활성 | base/opcode 없음, 초기 검은 판 1순위 | 숨김 |
| 3 | Ribbon | B | trail point 경로 존재 | dynamic 4 lane 억제, oracle UV, incomplete graph | Solo만 |
| 4 | Mesh weapon transition | B | 본체가 아닌 부착/전환 carrier | StartAlpha 미해결, masked/alpha state 불일치 가능 | Solo만 |
| 5 | Sprite smoke square | C | 1.67~2.17 활성 | generic reconstructed graph | 숨김 |
| 6 | Sprite smoke square | C | 1.67~2.17 활성 | generic reconstructed graph | 숨김 |
| 7 | Mesh `fm_m_trail_002` | C | scale 11.4, 2.610s 잔류 | generic numeric-oracle material | 숨김 |
| 8 | Mesh `fm_m_trail_002` | C | scale 11.4, 2.610s 잔류 | generic numeric-oracle material | 숨김 |
| 9 | Mesh `fm_h_swing_03` core | B | 0.5s 중앙 core carrier | texture luminance를 alpha로 재구성, exact edge 아님 | V3 main |
| 10 | Mesh `fm_h_swing_03` core | B | 0.5s 중앙 core carrier | #9와 동일 bounded graph | V3 main |
| 11 | Mesh `fm_h_swing_05` outer | B | 0.8s 외곽 carrier | 3 texture/Particle RGB 억제, alpha 25 | V3 main |
| 12 | Mesh `fm_h_swing_01` accent | C | 0.35s accent | generic numeric oracle | 숨김 |
| 13 | Mesh `fm_o_swing_02` fill | C | 평가 scale Y=0 | generic numeric oracle | 숨김 |
| 14 | Mesh `fm_o_swing_02` fill | C | 평가 scale Y=0 | generic numeric oracle | 숨김 |
| 15 | Mesh `fm_o_swing_02` fill | C | fill carrier | generic numeric oracle | 숨김 |
| 16 | Sprite BFX master | B | 50개, life 3~4 | Particle RGB/texture 일부 억제, full-quad blob 위험 | Solo만 |
| 17 | Mesh finite missile | B | finite carrier profile | typed finite template도 source-exact 아님 | Solo만 |
| 18 | Mesh `fm_m_trail_002` | C | scale 9, life 2, 잔류 쐐기 1순위 | generic numeric oracle | 숨김 |
| 19 | Sprite wind | C | 20개 hit cloud | base/graph 미복원, paint overlap | 숨김 |
| 20 | Decal water | C | 3.5x5, life 2.8 | generic material, 4/8 texture만 연결 | 숨김 |
| 21 | Decal water | C | 2x5, life 2.8 | generic material, 4/8 texture만 연결 | 숨김 |
| 22 | Decal unlit | B | 3.5x3.5, life 4 | 6 texture 중 emissive 1개만 소비, mask/dissolve 누락 | Solo만 |
| 23 | Sprite procedural glow | B | 3개 | sphere/twirl 식 유실 뒤 radial carrier 재구성 | Solo만 |
| 24 | Sprite attack | C | source row inert/불명 | graph/channel 미복원 | 숨김 |
| 25 | Sprite splash | C | 10개 단기 hit | generic graph | 숨김 |
| 26 | Mesh stone debris | C | source ballistic, 본체 아님 | masked source와 alpha depth-read state 불일치 | 숨김, 수동 파편 후보 |
| 27 | Sprite fire | C | 4개 | generic graph | 숨김 |
| 28 | Sprite flowmask | C | 4개 | generic graph | 숨김 |
| 29 | Sprite splash paint | C | 15개 장수 blob | generic graph | 숨김 |
| 30 | Sprite gibs master | B | 15개 장수 blob | bounded two-provider graph, 높은 alpha | Solo만 |
| 31 | Sprite wind | B | 9개, raw sheet 위험 | HDR 20/20/60을 peak와 50으로 추측 정규화 | Solo만 |
| 32 | ScreenPost zoom blur | B | typed screen-space/lifetime | material 성공이 아닌 특수 family | 독립 검토만 |
| 33 | Sprite one-layer distortion | C | 3개 | source emissive/distortion과 runtime slot correspondence 단절 | 숨김 |
| 34 | Point Light | B | typed non-material family | geometry artifact owner 아님, material 성공으로 계산 금지 | 독립 검토만 |

전용 RuntimeMaterialV2는 #3/#4/#9/#10/#11/#16/#22/#23/#30/#31의 10개다. generic numeric-oracle
경로는 #7/#8/#12/#13/#14/#15/#18의 7개다. `evaluator=17`은 이 둘을 합친 분모이며
`generic=17`로 기록하면 안 된다.

### V3 코드 scope와 고정 시점 계약

`CEffectReconstructedSourceRuntimeFactory::Build_Document`는 다음 네 scope를 가진다.

```text
ADMITTED_ONLY       -> 0개 (Complete 기본, fail closed)
V3_MAIN_REVIEW      -> #9, #10, #11
CONDITIONAL_REVIEW  -> #3,#4,#9,#10,#11,#16,#17,#22,#23,#30,#31,#32,#34
ALL_DIAGNOSTIC      -> 원본 inventory 35개
```

Artist F의 nonProduct Character Select/Effect Tool 검토 경로만 `V3_MAIN_REVIEW`를 명시한다. UI에는
`0 admitted | 13 conditional | 22 diagnostic-only`와 `Play V3 Main Review (3)`을 표시하고 Full35 또는
Restore 성공 문구를 사용하지 않는다.

고정 60Hz packet 회귀는 첨부 화면과 같은 네 시점을 검사한다.

```text
1.059s -> visible occurrence {}
1.670s -> visible occurrence {9,10,11}
2.177s -> visible occurrence {11}
2.610s -> visible occurrence {}
```

### decal 구조 버그와 분리 원칙

재구성 #20~#22는 source size/depth를 이미 World에 소유한다. 기존
`Resolve_DecalShaderProjection`은 이 branch에서 value-initialized `{0,0,0}`을 검증해 draw를 거부했다.
world-owned branch의 shader projection만 exact identity `{1,1}, depth=1`로 설정한다. legacy authored
decal은 기존 size/depth를 그대로 사용하고, signed mirror는 허용하되 singular/nonfinite World는 거부한다.

이 수정은 decal이 구조적으로 draw 가능한 조건을 닫을 뿐 #20~#22 재질을 복원하지 않는다. 그러므로
#20/#21은 C, #22는 B를 유지하고 V3 main 합성에서는 숨긴다.

### 다른 캐릭터와 Valtan에 적용할 공통 admission row

Artist 숫자나 order switch를 범용 renderer에 복사하지 않는다. publisher가 stable occurrence ID별로 다음
필드를 생성하고 runtime은 sealed revision만 소비한다.

```text
occurrenceId, rendererKind,
scheduleStartSeconds, emitterDurationSeconds, lifetimeMinMax,
geometryAssetId, geometryPreScaleConsumed,
materialRecipeId, shaderRoute,
requiredTextureAssetIds, missingTextureCount, samplerResolved, renderStateResolved,
transformCoordinatePolicy, cueTransform, maxWorldBounds, maxScreenCoverage,
sourceExact, consumedInputMask, suppressedInputMask,
automatedProbeStatus, manualEyeReviewStatus,
admission {ADMITTED_EXACT|CONDITIONAL_REVIEW|MANUAL_TUNED|DIAGNOSTIC_ONLY},
reasonCodes, approvedArtifactRevision
```

화면에 보이는 것을 근거로 gain/alpha/threshold/tint/scale을 맞추거나, unresolved default를 0/1로 바꾸거나,
흰/검정 neutral texture로 보이게 만드는 방식은 금지한다. 원본 edge가 끝내 복원되지 않으면 해당 행을
숨기고 필요한 역할만 새 `MANUAL_TUNED` 행으로 저작한다.

### V3 종료와 수동 제작 전환 기준

1. 자동 검증은 admission 분모, 네 fixed-time packet, actual DDS/GPU stage, ObjectManager layer Add,
   decal projection, Debug Client link를 확인한다.
2. 사용자가 Effect Tool의 `Artist -> F -> V3 Admission -> Play V3 Main Review (3)`을 직접 재생한다.
3. #9/#10/#11에서 원본으로 인정할 수 있는 core/outer가 보이면 승인된 행만 남기고 다음 Solo B를 한 행씩
   검토한다.
4. main 세 행도 흰/파란 raw carrier 또는 위치/형태 오류가 계속되면 V3 복원은 종료한다. screenshot에 맞춘
   보정을 더하지 않고 #9/#10/#11까지 숨긴 뒤, 원본 역할을 참고한 별도 `MANUAL_TUNED` 본체를 저작한다.
5. 수동 본체는 `위에서 아래로`라는 사용자 시각 목표를 사용할 수 있지만 `RESTORED`가 아닌
   `MANUAL_TUNED` revision으로 기록하고 다른 캐릭터/Valtan admission 통계와 분리한다.

## 2026-08-12 V4: Artist F 단독 material-composition canary

V4는 다른 캐릭터나 Valtan으로 확장하는 단계가 아니다. `effect.artist.skill.31470`의 비제품
preview 안에서 원본 프레임과 현재 V3 결과의 차이를 닫고, 사용자가 직접 승인할 수 있는 한 revision을
만드는 마지막 Artist F 복원 단계다. Product admission은 계속 false이며 기존 4직업 Product cue와
도화가 R, LanceMaster BA 데이터의 delta는 0이어야 한다.

### 원본 프레임을 반영한 계층 분리

원본 첫 프레임의 거대한 붓과 반원을 하나의 Effect mesh로 취급하지 않는다.

- 캐릭터가 들고 내려찍는 붓은 캐릭터 weapon/animation presentation과 notify-014의 `#4`
  weapon-transition carrier를 먼저 분리해서 본다.
- 캐릭터 주위를 휘어 올라가는 짙은 반원은 notify-018의 여러 MeshParticle 합성이다. V4 첫 canary는
  source DDS와 전용 bounded shader를 이미 가진 `#9/#10` watertrail core와 `#11` spritewave outer다.
  이 세 행의 성공을 전체 `#4/#7~#18` main 복원 성공으로 기록하지 않는다.
- 내려찍는 다음 프레임의 바닥 잉크는 notify-022의 `#20/#21/#22` Decal 계층이며, 뒤쪽 비산물은
  같은 hit cue의 `#16/#19/#23/#25/#27~#31` Particle 계층이다. main canary가 승인된 뒤 별도
  impact canary에서 한 family씩 검토한다.

### V3 화면 결함의 직접 원인

`#9/#10/#11` DDS는 이미 각자의 mesh draw에 연결돼 있다. 검은 DDS를 흰 mesh에 decal처럼 추가로
붙이는 누락이 아니다. `#9/#10`과 `#11`은 서로 다른 mesh, material family, texture 집합과 shader
formula를 가진 세 번의 translucent draw다.

- `#9/#10`: `fm_h_swing_03.wmodel`, `fx_h_me_watertrail_01_2_tr`, `fx_h_wave_01` +
  `fx_a_noise_011`. 현재 reconstructed opcode는 원본 opacity/dissolve/Fresnel edge가 소실된 상태에서
  main RGB luminance를 color와 coverage로 직접 사용해 밝은 raw carrier가 앞 레이어를 덮는다.
- `#11`: `fm_h_swing_05.wmodel`, `fx_m_pa_spritewave_01_19_tr`. main DDS
  `fx_m_trail_004_cl`의 alpha는 전 texel 255인데 현재 식은 여기에 `maintex_alpha_strength=2000`을
  곱한다. 따라서 검은 RGB 영역도 불투명한 검은 판으로 남는다. alpha channel을 coverage로 쓰지 않고
  main RGB mask, sphere, dissolve, particle alpha로 bounded coverage를 다시 만든다.
- 두 family의 공식을 합치거나 `#11` texture를 `#9/#10`에 재지정하지 않는다. cooked graph가 끝내
  닫히지 않는 식은 `EXACT_RESTORED`가 아니라 revisioned `RECONSTRUCTED` 또는 `MANUAL_TUNED`다.

### transform 판정 순서

`fm_h_swing_03/05` carrier는 source/runtime 모두 XZ가 넓고 Y가 얇은 평면형이며 geometry pre-scale
0.01은 정확히 한 번 적용된다. 실제 V3 preview가 쓰는 `CEffectPlayback`은 source MeshRotation의
3축을 `(X,Z,-Y)` client basis로 변환하고 `#9/#10`의 source Z 회전을 client Y yaw로 소비한다.
따라서 현재 지면 방향을 전역 90도 회전이나 downward translation으로 고치지 않는다.

1. 먼저 raw white/black coverage와 dissolve/reveal을 닫는다.
2. 같은 sample time과 camera에서 원본 반원 실루엣과 다시 비교한다.
3. 그래도 방향/anchor가 다르면 TypeDataMesh axis, inheritance, pivot evidence를 다시 조사한다.
4. source edge가 없을 때만 occurrence-local rotation/anchor를 `MANUAL_TUNED`로 추가한다.

### 구현 순서와 admission

1. V3의 35행 inventory와 `ADMITTED_ONLY=0`을 보존한다.
2. `#9/#10` watertrail과 `#11` spritewave를 서로 다른 versioned shader revision으로 수정한다.
3. white/black neutral fallback 없이 필수 DDS/SRV/sampler, finite packet, exact occurrence/recipe identity를
   만족할 때만 세 행을 V4 review에 노출한다.
4. `1.059s {}`, `1.670s {9,10,11}`, `2.177s {11}`, `2.610s {}` 시간 집합과 source
   position/velocity 미주입, geometry pre-scale exact-once를 자동 회귀한다.
5. 사용자가 원본 프레임과 비교해 main arc의 색, coverage, reveal, 방향을 승인한 뒤에만 `#4/#7~#18`
   보조 main 행과 impact Decal/Particle 행을 각각 Solo로 연다.
6. Product는 승인 뒤에도 자동 true로 바꾸지 않는다. 승인 revision을 고정한 별도 atomic publish 단계가
   필요하다.

### V4 수동 화면 검증

에이전트는 Client를 실행하거나 화면 PASS를 대신 판정하지 않는다. 자동 검증 뒤 사용자가 Effect Tool에서
Artist F를 재생하고 최소 다음 시점을 직접 비교한다.

- 약 1.59~1.67초: 흰 crescent와 불투명 검은 판이 없어지고, 캐릭터 주위를 휘감는 짙은 남청/먹색
  core/outer가 원본과 같은 방향으로 reveal되는지
- 약 2.17초: `#11` outer가 raw carrier 사각형 없이 자연스럽게 소멸하는지
- 이후 impact canary: 바닥 잉크 Decal과 뒤쪽 비산 Particle이 서로 다른 계층으로 보이는지

사용자 판정은 `PENDING`, `APPROVED`, `REJECTED`와 sample time, 관찰 이유, artifact revision을 RESULT에
기록한다. 첨부 프레임은 진단 입력이며 그 자체로 자동 승인하지 않는다.

### Artist F 승인 뒤의 후속 순서

후속 계획은 Artist F의 사용자 `APPROVED` revision이 고정된 뒤에만 시작한다.

```text
Artist F main + impact 승인
-> Artist R (Decal 중심) 단일 canary
-> LanceMaster BA1 (Trail 중심) 단일 canary
-> LanceMaster BA2
-> LanceMaster BA3
-> 각 canary에서 검증된 family만 별도 공용 계약 후보로 승격
```

이 순서는 향후 범위 기록일 뿐 현재 V4 구현 권한을 확장하지 않는다.

## 2026-08-12 V4 Track A 재정렬: source-revision shader evidence 우선

사용자는 Artist F를 수동 미감으로 먼저 맞추는 분기보다 원본 근거를 최대한 복구하는 `Track A`를
우선하기로 결정했다. 따라서 앞 절의 `#9/#10/#11` 수동 material-composition 수식은 작업 초안으로만
보존하고, source-revision 근거가 닫히기 전에는 빌드·화면 검증·승격 입력으로 사용하지 않는다.
초안을 되돌리거나 다른 작업자의 변경을 정리하지도 않는다.

Track A의 순서는 다음과 같이 고정한다.

```text
matching source-revision runtime identity
-> ShaderCache / FMaterialShaderMap 또는 native material bytecode 획득
-> Material/MIC identity와 shader-map membership exact join
-> static permutation + sampler + render state exact join
-> parameter -> register -> channel -> opacity/emissive/output edge 복구
-> 독립 numeric oracle와 DXBC replay 대조
-> #9/#10 watertrail, #11 spritewave family별 runtime evaluator
-> occurrence review와 사용자 육안 승인
```

현재 정본 수치는 다음과 같다.

- installed ShaderCache export `1,596`, 구조 해독 primary shader object `271`, material shader map `25`,
  shader reference `534`, unique DXBC `240`
- Artist base Material join `0/23`, MIC static parameter-set join `0/24`
- cooked parent graph의 null expression slot `1,803`, unresolved edge `502`
- source-specific render/static/sampler execution readiness `0/255`

따라서 DDS가 연결되고 현재 DXBC를 읽을 수 있다는 사실만으로 Artist F의 material A를 주장하지 않는다.
Track A가 열리는 최소 조건은 같은 build identity의 `EFEngine/LOSTARK`, `Engine/Core/EFGame`, target UPK,
ShaderCache/material map, SystemSettings를 하나의 manifest로 묶거나, 동일 build의 인증된 fixed-input
numeric capture를 확보하는 것이다. 캐시가 다른 revision이면 현재 설치본 output을 source 값으로
승격하지 않는다.

Track A 중단선은 다음 세 조건을 모두 조사한 뒤에만 판정한다.

1. 접근 가능한 원본 archive와 별도 설치/백업에서 source-revision ShaderCache 또는 inline material
   shader payload가 없는지 raw package 구조로 재검색한다.
2. 기존 `0/23`, `0/24`가 단순 16-byte subsequence 검색의 한계인지 확인하기 위해 전체 cache의
   `FMaterialShaderMap` key/static-parameter serialization을 구조적으로 디코드하고 source Material/MIC와
   비교한다.
3. material export native tail 또는 관련 cooked package에 독립 DXBC/bytecode가 inline되어 있는지,
   기존 extractor가 보존하지 않은 native record가 있는지 검사한다.

세 경로가 모두 명시적 BLOCK으로 닫히기 전에는 `MANUAL_TUNED`로 전환하지 않는다. 모두 닫힌 경우에도
수동 분기는 별도 사용자 승인과 revision을 요구하며 `ADMITTED_EXACT` 통계와 영구 분리한다. Artist F의
승인 revision이 생기기 전까지 Artist R, LanceMaster BA, 4직업 공용 family 변경은 계속 중지한다.

## 2026-08-12 V5: 복원 우선 증거 탐색 트랙

V3의 `main 실패 시 복원 종료 후 MANUAL_TUNED 전환`은 더 이상 Artist F의 최종 탐색 정책으로
사용하지 않는다. 화면 보정본은 비교 가능한 canary revision으로 보존하지만, cooked 원본에서 더
회수할 수 있는 증거를 소진하기 전에 이를 Material 정본이나 복원 완료로 승격하지 않는다.

탐색은 다음 순서로 계속한다.

```text
exact Material/MIC tagged properties와 surviving graph
-> Material export native FMaterialResource/legacy resource 구조
-> 같은 revision의 FMaterialShaderMap/ShaderCache identity join
-> 안전한 offline DXBC reflection/replay와 고정 입력 numeric oracle
-> 같은 family의 Material/MIC 차분 및 occurrence 교차 제약
-> 위 증거가 닫힌 family부터 runtime evaluator 교체
-> 자동 packet/draw 검증
-> 사용자 직접 화면 승인
```

각 단계가 음성 결과여도 복원 전체를 종료하지 않는다. 대신 검색 root, raw byte identity, 해석한
구조, 해석하지 못한 구간과 다음 증거원을 receipt에 고정한 뒤 다음 단계로 이동한다. 다만
source-exact가 아닌 결과를 `ADMITTED_EXACT`로 표시하지 않는 fail-closed 경계는 유지한다.

### V5-1 첫 탐색 단위: Material native resource inventory

현재 23개 base Material의 native tail은 단순 미확인 blob이 아니다. 현재 실측과 UE Viewer의 UE3
`UMaterial3::Serialize` 구현을 대조하면 다음 앞부분을 구조적으로 읽을 수 있다.

```text
version-868 material mask
FMaterialResource legacy string array / expression map / legacy integer
material state GUID와 resource field
ReferencedTextures array
legacy resource flags
FLegacyTextureLookup[] { TexCoordIndex, TextureIndex, UScale, VScale }
remaining inline-resource trailer
```

첫 구현은 23개 source-exact base Material export만 입력으로 사용해 위 구조를 fail-closed로
파싱한다. 출력 receipt에는 package/export/serial/native-tail SHA-256, state GUID, exact texture
object path, legacy texture lookup, 아직 이름을 확정할 수 없는 dword와 trailer를 원시 offset과 함께
기록한다. 미확정 dword는 의미 있는 boolean이나 shader-map 상태로 임의 승격하지 않는다.

성공 조건은 다음과 같다.

- 23/23 export가 동일한 bounded parser로 tail 끝까지 소비된다.
- 모든 package reference가 유효한 texture object path로 resolve된다.
- lookup의 texture index가 해당 Material의 ReferencedTextures 범위 안이고 UV scale이 finite다.
- raw tail 변조, 잘못된 count/reference/index, trailing byte 추가를 테스트가 거부한다.
- 기존 opaque 상태보다 새로 확인된 exact field를 source Material contract가 소비할 수 있는 형태로
  분리하되, arithmetic graph나 Product admission을 자동으로 열지 않는다.

첫 결과의 직접 효용은 cooked 시점의 texture membership과 UV lookup index/scale을 확정해 sampler
및 texture-coordinate 제약을 강화하는 것이다. inline shader bytecode나 uniform-expression tree가
이 tail에 없다는 결과가 나오면 그 음성 증거도 고정하고, 다음 순서인 같은-revision ShaderCache
획득 및 구조적 shader-map join으로 이동한다.

### V5-2 구현 파일과 검증

- 신규 extractor/test는 `Tools/LevelPlacementExtractor`에 두고 source package를 수정하지 않는다.
- receipt는 `Data/Effects/Imported/Artist/Materials`에 두며 23개 family denominator와 외부 raw package
  identity를 고정한다.
- 기존 `extract_artist_31470_shader_cache_oracle.py`의 state-key 및 MIC static-parameter 결과는
  재사용하되, 새 receipt가 그 결과를 자기 인증하거나 graph exact로 승격하지 못하게 한다.
- focused unit test, receipt `--check`, JSON parse, 관련 ProjectAudit check, `git diff --check`를 실행한다.
- Client는 에이전트가 실행하거나 조작하지 않는다. runtime evaluator가 실제로 바뀌는 후속 단계에서
  빌드와 자동 draw packet까지 준비하고, 최종 화면 판정은 사용자가 직접 수행한다.

### V5-3 `FLegacyTextureLookup`과 MIC override의 유효 입력 판정

베이스 Material의 `ReferencedTextures`와 `FLegacyTextureLookup`은 원작 cooked metadata이지만,
그 자체가 특정 31470 occurrence의 최종 texture binding은 아니다. 같은 parameter를 active MIC가
덮어쓴 경우에는 `base default -> MIC override` 우선순위를 적용한 뒤에만 실제 입력을 판정한다.

```text
base Material lookup texture index
-> cooked graph named texture parameter
-> active 31470 recipe의 MIC texture override
-> 같은 logical texture 유지 / 다른 texture로 치환 / override 없음 분류
-> effective texture만 runtime-binding receipt와 결합
```

현재 7개 lookup texture의 고정 denominator는 다음과 같다.

- 3개는 active 31470에서 logical texture가 그대로 유효하다.
- 4개(`wp_wgdh_01s_d/_n/_s`, `fx_d_cloud_006`)는 active MIC가 각각
  `wp_mn_lrcn_01_d/_n/_s`, `fx_m_smokesq_01`로 치환한다.
- 추가 1개 lookup은 MIC가 같은 logical texture를 다시 지정하는 identity override이므로, override가
  있다는 이유만으로 base texture를 제거하지 않는다.
- effective texture와 치환 대상의 runtime binding 누락은 모두 0이어야 한다.

치환된 네 base-default DDS는 원본 UPK와 UModel identity를 고정한 fresh extraction으로 외부 증거
보관소에만 저장한다. 이 단계에서는 `Client/Bin/Resources`에 배포하거나 현재 renderer binding을
교체하지 않는다. 이후 다른 occurrence 또는 static permutation에서 해당 default가 실제로 선택된다는
증거가 생길 때만 별도 승인·transaction 단위로 runtime asset을 추가한다.

### V5-4 main-first `FMaterialShaderMapId` derivation과 archive 전수 join

native resource 단계는 `#9/#10/#11` 두 family 모두 legacy lookup 0, effective texture membership
delta 0의 음성 증거로 닫는다. 이후 23-family 전체 texture closure를 더 확장하지 않고 main 두
family만 다음 순서로 조사한다.

1. source base Material state GUID와 active MIC native tail의 `FStaticParameterSet` raw bytes를 고정한다.
2. LostArk package version 868/licensee 16에서 쓰는 shader-map identity serialization을 로컬 binary와
   구조적 package evidence로 유도한다.
3. 독립 구현으로 확인되기 전 명칭은 `LOSTARK_V868_BOUNDED_SHADER_MAP_ID_HYPOTHESIS`로 유지한다.
4. 각 후보에는 입력 field뿐 아니라 source file/export/tail offset, exact raw hex/SHA-256, byte order,
   derivation algorithm과 negative variant를 기록한다.
5. source archive 1,813 UPK, installed 1,596 ShaderCache export, global Material, 동일 leaf duplicate를
   derived identity로 전수 structural join한다.
6. 같은 revision의 Engine/Core/SystemSettings/EFEngine/LOSTARK binary와 ShaderCache bundle을 raw
   identity manifest로 탐색한다.
7. join 성공 시에만 MeshParticle VF/pass DXBC reflection, uniform-expression/register/texture/channel/
   dynamic binding, fixed-input offline replay로 이동한다.
8. numeric oracle이 닫힌 main family부터 별도 runtime evaluator transaction을 수행한다.

수동 HLSL draft와 현재 V4 tuning은 이 트랙 동안 동결한다. join 실패는 후보 수·탐색 byte 범위·
manifest identity를 포함한 음성 증거로 남기고 다음 안전한 acquisition 단계가 있으면 계속 진행한다.

### V5-5 current binary ABI corroboration과 exact join 성공선

설치된 hash-pinned `EFEngine.dll`의 export ABI는 이 LostArk 빌드에서
`FMaterialShaderMap::GIdToMaterialShaderMap`이 `TMap<FStaticParameterSet, FMaterialShaderMap*>`,
`FindId`가 `(const FStaticParameterSet&, EShaderPlatform)`, `GetMaterialId`가
`const FStaticParameterSet&`, `UShaderCache::FindStaticShaderMap`이
`const FStaticParameterSet&`를 사용함을 노출한다. 따라서 main 두 family의 구조 join key는 별도
16-byte MIC tail 후보가 아니라 `BaseMaterialId + 네 static parameter array 전체 + platform`이다.
이 ABI는 current installed binary corroboration이므로 source package와 single-revision manifest가
결합되기 전에는 source-exact로 승격하지 않는다.

구현은 다음 증거를 별도 focused receipt로 고정한다.

1. source Material state GUID와 active MIC `FStaticParameterSet`의 package/export/serial/tail absolute
   offset, raw bytes/SHA-256, semantic projection을 기록한다.
2. semantic equality는 parameter array kind와 순서, FName 문자열, value, `bOverride`,
   ExpressionGUID 전체 exact equality이며 package-local FName index가 들어간 raw SHA 단독 비교는
   cross-package join key로 사용하지 않는다.
3. `EFEngine.dll` raw SHA-256, Authenticode/version, export decorated name, RVA, bounded function byte
   SHA를 고정한다. `GetMaterialId`가 object offset `0xA4`의 set reference를 반환하는 기계어와
   `FindId`/`FindStaticShaderMap`의 lookup ABI는 derivation corroboration이다.
4. cache의 기존 `mapSerialCandidate`는 identity가 아니라
   `exportSerialOffset + nativeTailOffset + mapOffset + mapByteSize`인 absolute map-end boundary로
   25/25 확인하고 이름을 승격하지 않는다.
5. source archive 1,813 UPK와 installed ReleasePC 33,889 UPK는 header/package GUID/NameTable/
   import/export table을 streaming decode한다. `ShaderCache` export와 target parameter-name full set을
   후보화한 뒤에만 serial을 decode하되, 후보 parse 실패·중복·platform 불일치는 음성 결과가 아니라
   fail-closed blocker로 남긴다.
6. source/global/same-leaf Material·MIC duplicate도 같은 semantic key로 비교해 revision·alias 범위를
   닫는다. raw-content duplicate는 alias로 묶되 서로 다른 content를 생략하지 않는다.

Track A의 시각 진전 성공선은 낮추지 않는다. exact `FStaticParameterSet`가 동일 source revision
shader map에 join되고, 그 map의 실제 MeshParticle VF/pass DXBC에서 parameter/register/texture/channel/
opacity-dissolve 경로와 fixed-input numeric replay까지 닫혀야 runtime evaluator transaction으로
넘어간다. join 또는 DXBC가 끝내 없으면 runtime HLSL을 바꾸지 않고
`TRACK_A_ACQUISITION_EXHAUSTED`로 봉인하며, 현재 결과와 미복원식을 Track C 입력 계약으로 전달한다.
