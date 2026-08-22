# Effect V1 horizontal Sprite canary 구현 결과

## 1. 결론과 작업 경계

Track A의 자동 구현·검증은 끝났다. Binding 0 registry spine 위에
`effect.artist.skill.31470.unified / sprite.2b3dc6842507e910` 한 occurrence만
Binding 1로 admission했고, registry packet과 기존 inline `Material.Execution` packet의
bit-exact dual resolve, 실제 `GameInstance::Render -> MRT_SceneHDR -> Render_Particles`
draw, Debug/Release focused evidence를 닫았다. 남은 gate는 사용자가 직접 수행하는
Artist F first-pixel A/B 판정뿐이다.

| 역할 | identity |
|---|---|
| golden control | `effect.artist.skill.31470.unified` |
| first bound occurrence | `sprite.2b3dc6842507e910` |
| unbound shadow fixture | `sprite.c65181324417a1a8` |
| A baseline | `086acab531acb0c258b91660b981cdbf3ca99dfc` |
| B verified implementation | `4a9fb689684fcba5e9c0213ac6b3385129d43a7c` (`89dc49a1c845ce94a26b066a18d41a360dbc13cb` binding + LF-stable source authority) |

Track A는 계획 merge `6f047be2`, Binding 0 구현 `a2ffeb3b` 및 merge PR #168
`693a1fba`, focused harness PR #171 `086acab5`의 순서로 닫았다. 최신 main에 먼저
병합된 Track B PR #170 `f99adc03`은 candidate의 Git ancestry일 뿐이다. Track B의 tuple
cohort와 adapter-packet inventory를 Track A dual-resolve, actual draw 또는 Full35 runtime
proof로 소급 사용하지 않았다.

child-parent, G00 inventory/docs, 추가 DXBC 추출, 4캐릭터/Valtan cohort,
DimensionMaster BA/A, Artist R decal, Mesh/Decal/Trail/Glass/WPO/Presentation은 변경하지
않았다.

## 2. Binding 0에서 Binding 1로의 수직 연결

`Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json`은 program, layout,
descriptor, compiled adapter를 가진 source 정본이다. publisher는 이를 catalog format v4의
immutable `materialPrograms` payload로 봉인한다. catalog revision과 registry generation은
catalog에서 prewarm target, prepared document까지 같은 snapshot으로 전달된다.

Binding 0에서는 `bindings=[]`이고 기존 inline execution draw가 그대로 유지된다. Binding 1은
다음 한 tuple만 추가한다.

| field | stable ID |
|---|---|
| program | `effect.program.runtime-material-v2.opcode-6.artist-f-sprite.v1` |
| layout | `effect.layout.runtime-material-v2.artist-f-sprite.v1` |
| descriptor | `effect.descriptor.artist-f.sprite-2b3dc6842507e910.v1` |
| adapter | `effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1.alpha-two-sided.v1` |

registry packet과 inline golden packet의 exact 비교는
`EFFECT_MATERIAL_EXECUTION_DESC`의 모든 field, ordered texture lane과 그 sampler,
ordered scalar/vector/artist parameter/color, 모든 float bit pattern, pass와 authored render
state를 포함한다. Program/Layout/Descriptor/Adapter identity와 compiled carrier/MRT는 이
execution 비교에 섞지 않고, binding allowlist와 actual `Render_Particles` 검증에서 별도로
확인한다.

draw 전 exact check는 두 번 수행된다. catalog stage에서 registry Binding의 `Execution`을
inline `Element.Material.Execution` golden mirror와 비교하고, resource stage 뒤에는
`Build_MaterialExecutionSnapshot` 결과를 registry `Execution`과 다시 비교한다. 둘 중 하나라도
다르면 draw 전에 fail-closed한다. dangling ID, duplicate ownership, revision/generation 혼합도
같이 거부하며, binding이 있는 occurrence를 조용히 inline으로 되돌리는 fallback은 없다.
shadow fixture는 끝까지 product binding을 갖지 않는다.

8-test registry suite는 product source를 바꾸지 않는 in-memory two-binding fixture를 만들어
first와 shadow가 같은 packet으로 materialize되는지 다시 bit-exact 비교한다. float drift
`0.100000001 -> 0.10000002`와 `+0.0/-0.0` signed-zero 차이도 각각 fail-closed임을 확인한다.
따라서 shadow는 두 번째 product admission이 아니라 dual-resolve의 독립 증명 후보로만 남는다.

source registry는 raw-byte publisher authority이므로 `.gitattributes`에 LF를 고정했다. 이
규칙이 없으면 `core.autocrlf=true` fresh checkout에서 CRLF가 되어 publisher가 정본 byte를
거부하는 것을 재현했고, 한 파일의 LF 계약만 추가해 해결했다.

## 3. 왜 skill 전용 renderer 경로가 아닌가

stable effect/element 제한은 registry data의 한 Binding과 fail-closed admission에만 존재한다.
제품 renderer에는 `31470`, Artist F 또는 `missiletrail` 이름을 분기하는 switch를 추가하지
않았다. compiled adapter는 resolved Program × Layout × Adapter tuple에 대해 다음 일반
SpriteParticle draw 계약만 검증한다.

- shader `Shader_VtxEffectParticle.hlsl`, vertex layout `VTXEFFECT_PARTICLE`, pass 1
- `MRT_SceneHDR`, RT0 `SV_TARGET0` scene color, RT1 `SV_TARGET1` deterministic zero distortion
- `RS_Cull_None`, `DSS_ReadOnly`, `BS_EffectAlpha`, stencil reference 0
- 실제 D3D11 carrier/pass/state/sample mask와 MRT RT0/RT1/DSV identity, RT2~RT7 null

검증 뒤에는 새 shader나 새 renderer가 아니라 기존
`SpriteParticle/RuntimeMaterialV2::Render_Particles`를 호출한다. occurrence 전용 ID는 focused
harness의 고정 분모이기도 하지만 제품 dispatch 조건은 아니다.

## 4. Debug/Release focused A/B 증거

PR #171의 focused harness는 실제 WARP D3D11 context에서 제품
`GameInstance::Render -> MRT_SceneHDR -> Render_Particles`를 통과한다. Debug와 Release가
같은 결과를 냈다.

### 4.1 catalog와 prewarm

| 값 | A Binding 0 | B Binding 1 |
|---|---:|---:|
| catalog revision / registry generation | 1 / 1 | 1 / 1 |
| catalog binding count | 0 | 1 |
| prewarm binding / resolved element | 0 / 0 | 1 / 1 |
| first occurrence bound | false | true |
| shadow occurrence bound | false | false |

표의 A와 B `revision/generation=1/1`은 각각 독립된 first-load process에서 관찰한 값이다. 같은
process에서 content가 바뀌었는데 revision이 유지됐다는 뜻이 아니다.

focused harness는 같은 registry handle과 catalog revision을 prewarm target에 넣고, 그 identity로
`Find_Prepared`가 성공한 prepared resources만 실제 object stage에 전달한다. 마지막 prewarm
probe와 prepared document까지 B의 catalog -> prewarm target -> prepared identity가 모두
`revision/generation=1/1`, `binding/resolved=1/1`로 전달된다. count가 다르거나 prepared lookup이
null이면 draw 전에 실패한다.

### 4.2 Artist F first Sprite actual draw

| counter | A | B |
|---|---:|---:|
| configured / evaluated / active / candidate | 17 / 17 / 14 / 14 | 17 / 17 / 14 / 14 |
| attempted / submitted / suppressed / failed | 14 / 1 / 13 / 0 | 14 / 1 / 13 / 0 |
| target active / candidate / attempted / submitted / suppressed / failed | 1 / 20 / 1 / 1 / 0 / 0 | 1 / 20 / 1 / 1 / 0 / 0 |
| material / SRV / sampler binds | 1 / 12 / 2 | 1 / 12 / 2 |
| pass / VI bind / VI draw / issued draw / selection | 1 / 1 / 1 / 1 / 1 | 1 / 1 / 1 / 1 / 1 |
| actual compiled-adapter pipeline validations | 0 | 1 |
| completed / committed / draw selection diverged | true / true / false | true / true / false |

B에서 actual validation counter는 bound first occurrence 한 건만 1이고, Binding 0 A와 Lance
BA1의 실측값은 0이다. shadow는 product source에서 unbound이므로 actual draw target이 아니며,
in-memory dual-resolve fixture로만 사용했다. 집계의 13 suppressed는 focused harness가
`Set_TestPreviewElementIsolation`으로 first stable Element 하나만 실제 제출하도록 격리한 값이지,
제품 migration이 13개 draw를 잃었다는 뜻이 아니다. 따라서 metadata-only 연결이나 광역 adapter
승격이 아니다.

### 4.3 Full35와 legacy Lance BA1 회귀

Full35 structural sentinel은 A/B Debug/Release 모두 다음 exact 값이다.

- sample `1.5s`, fixed step 90, active particle packets 156, exact disposition rows 25
- attempted/submitted/suppressed/failed `25/22/3/0`, committed `true`
- state projection SHA-256
  `e2fee36b3f4e9383fb5f60f9f6dfa62d1a47558b013610abc35f1c088b9ed704`
- frame projection SHA-256
  `95830deb2d54d576c97985e628c4e432d788eb7de16ca4b07e39054865f10ad4`

`full35StructuralSentinel`은 frozen candidate를 직접 읽는 CPU fixed-step 구조 회귀 분모다.
Full35 actual-renderer prewarm/draw, Mesh/Decal/Trail admission을 증명하지 않는다. 실제 D3D11
draw 증거는 first Sprite occurrence 한 건에만 한정한다.

Lance BA1도 A/B Debug/Release가 같다.

| sample | active/candidate/attempted/submitted/suppressed/failed | draw | committed |
|---|---|---|---|
| floor `0.5207s` | `1/0/1/0/1/0` | 0 | true |
| tick 32 `0.533333s` | `1/1/1/1/0/0` | 1 | true |

tick 32의 material/SRV/sampler/pass/VI bind/VI draw/issued draw/selection은
`1/12/1/1/1/1/1/1`이고 compiled-adapter actual validation은 0이다.

## 5. publisher, build와 회귀 상태

### 5.1 Track A 직접 증거

| 검증 | 결과 |
|---|---|
| registry focused Python | 8 tests PASS |
| direct-authored runtime validator | 6 tests PASS + runtime CLI PASS |
| Effect publisher Validate | 145 Effects, 1 binding PASS |
| Effect publisher Publish | 145 Effects, candidate catalog publish PASS |
| source/runtime JSON, project/filter XML | PASS |
| registry source project/filter exact registration | 각각 1개 PASS |
| Artist 31470 runtime-oracle | 23 tests, shallow receipt, HLSL/WARP 200 samples PASS |
| Engine -> UpdateLib -> Shared -> Server -> Client Debug | BUILD PASS |
| Engine -> UpdateLib -> Shared -> Server -> Client Release | BUILD PASS |
| focused effect render Debug/Release | PASS |
| `git diff --check` | PASS |

정본 `Invoke-BuildAndRegression.ps1`은 두 configuration 모두 Client build와 Effect publisher
Validate까지 성공한 뒤 `Sync-EffectDataProject.ps1 -Check`에서 중단됐다. 현재 main의
`Client.vcxproj/.filters`에 다수의 기존 Effect Data 등록 차이가 남아 있는 광역 stale
상태다. 이를 고치려면 Track B/G00 등 제외 범위 파일이 대량 변경되므로 Track A에 섞지
않았다. 이번 source registry의 두 project entry는 별도로 XML parse와 exact count로
검증했다. sync 뒤에 위치한 in-scope regression은 위 표처럼 직접 실행했다.

### 5.2 ambient canonical regression 관찰

아래 결과는 build/regression 환경의 동시 상태를 확인한 것이며 Track A binding admission이나
actual draw 증거로 사용하지 않았다.

| 검증 | 결과 |
|---|---|
| rendering profile / Valtan floor / ground-target gates | PASS |
| NetworkProtocolHarness Debug/Release | failures 0 PASS |
| Server `--contract-test` Debug/Release | failures 0 PASS |
| Valtan four-player live Debug/Release | PASS |

Character Select isolation live harness는 private-session 격리와 Bern 2-player convergence까지
PASS한 뒤, Track A와 무관한 Bern-to-Valtan probe가 Debug/Release 모두
`acceptances=1, world=1, position=(137.738007,-22.688004)`에서 20초 timeout으로 실패했다.
candidate는 Server/C++를 변경하지 않았고 A/B Server executable도 byte-identical이다. 이
candidate와 독립적인 전환 실패를 Track A runtime proof나 Artist F visual gate의 PASS로
사용하지 않았다.

## 6. A/B 산출물 identity

candidate에는 C++ diff가 없다. A와 B를 각각 측정한 결과 configuration별 executable SHA는
byte-identical했고 runtime catalog SHA만 달랐다.

| 산출물 | last write UTC | A SHA-256 | B SHA-256 |
|---|---|---|---|
| Debug `Client.exe` (25,927,168 bytes) | `2026-08-22T18:57:46.0829460Z` | `f6620c9fa2f59b5458849e381b7016aac98877c74fcbc81b6f130e74db6ccfc4` | `f6620c9fa2f59b5458849e381b7016aac98877c74fcbc81b6f130e74db6ccfc4` |
| Release `Client.exe` (4,382,208 bytes) | `2026-08-22T19:10:26.8697155Z` | `d955abc37b2d9209db59e33a5d6c957b5b037af6db428401cb63da69c55b3c0e` | `d955abc37b2d9209db59e33a5d6c957b5b037af6db428401cb63da69c55b3c0e` |
| Debug `Server.exe` (5,784,064 bytes) | `2026-08-22T18:52:10.8664380Z` | `6c044c22926d6ec7e64e663f7624fe3338b5979f1f3d7559bf507e7cf74dc9ec` | `6c044c22926d6ec7e64e663f7624fe3338b5979f1f3d7559bf507e7cf74dc9ec` |
| Release `Server.exe` (1,036,800 bytes) | `2026-08-22T19:01:54.9702905Z` | `e563552b9a986beecde8c75e6593b01f88d33a89f56cd7a357de27829a62937a` | `e563552b9a986beecde8c75e6593b01f88d33a89f56cd7a357de27829a62937a` |
| `EffectCatalog.runtime.json` | A `2026-08-22T18:51:09.4720573Z`; B `2026-08-22T19:30:08.2032409Z` | `90e0559a62f47a57079a067f2a8846cac9dfe0ab2e9bf13f808017dc912edd4d` | `1f78af52be8c05e37dc28c9daa5b34d0924bb0a18636123deab55db492a3ff7e` |

A SHA는 baseline checkout에서 측정한 뒤 B로 전환했고, B SHA는 final implementation
`4a9fb689...`의 두 configuration build command 뒤 다시 측정했다. 같은 SHA라는 결론은 C++ diff
부재에서 추론한 것이 아니라 두 checkout에서 관찰한 결과다. executable의 물리 timestamp는
binding commit `89dc49a1...` 뒤, LF-only commit `4a9fb689...` 전 relink에서 만들어졌다. final
command에서는 link input이 바뀌지 않아 MSBuild가 Client/Server를 up-to-date로 판정했고,
catalog는 final Publish로 다시 교체한 뒤 SHA를 확인했다.

물리 경로는 `Client/Bin/<Debug|Release>/Client.exe`,
`Server/Bin/<Debug|Release>/Server.exe`,
`Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`이다.

## 7. 사용자 first-pixel A/B 인계

에이전트는 Client/UI를 실행·조작하거나 visual PASS를 선언하지 않았다. 사용자는 다음과
같이 같은 PC, 같은 configuration, 같은 카메라와 위치에서 직접 판정한다.

1. `git switch --detach 086acab531acb0c258b91660b981cdbf3ca99dfc`로 A를 선택한다. 위 표의
   Debug/Release Client·Server EXE SHA와 catalog SHA `90e055...edd4d`를 확인하고, 하나라도
   다르면 실행하지 말고 산출물 identity 차이를 먼저 조사한다.
2. Visual Studio에서 `Framework.sln`, `x64`, 원하는 `Debug` 또는 `Release`,
   `Server + Client` launch profile을 선택하고 `Ctrl+F5`로 실행한다. Client working
   directory는 `Client/Default`, endpoint는 Server/Client 모두 `127.0.0.1:7777`이다.
3. Lobby의 `Character Select`로 Server 승인을 받아 진입하고 `Artist`를 선택한다.
   free camera 상태라면 공식 전환키 `F6`으로 follow camera로 돌아온다. `F`는
   `skillId=31470`, `필법 : 한획긋기`다. 같은 위치·방향에서 F를 반복해 A를 기록한다.
4. 모두 종료한 뒤 `git switch --detach 4a9fb689684fcba5e9c0213ac6b3385129d43a7c`로 B를 선택하고
   EXE SHA가 A와 같고 catalog SHA가 `1f78af...3ff7e`인지 확인한 다음 같은 profile과 동작을
   반복한다.
5. 발생 시점, 위치/방향, 크기/수명, 색·coverage·noise·alpha, 반복/취소/재진입 후 잔상과
   first Sprite의 차이를 비교한다.

현재 수동 상태는 `PENDING_USER_VISUAL_GATE`다. 화면 변화가 없어야 하는 migration이므로
차이가 관찰되면 손튜닝으로 덮지 않고 occurrence, packet, state/MRT 또는 resource bind 원인을
다시 조사한다.
