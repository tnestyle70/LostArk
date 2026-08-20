# 2026-08-21 Effect runtime simplification and BA regression implementation result

## 0. 결과 요약

- Debug Effect Tool, All Effects 진입점, Save Hot Reload, selected publish transaction을 Client build와 저장소 소스에서 제거했다.
- Product cue approval/admission token과 sidecar, direct-authored runtime row의 content/dependency SHA equality를 제거했다. runtime은 Effect ID, authoring version, 안전한 sealed document path와 document 내부 identity만 확인한다.
- Character Select는 선택 class Effect 준비를 map/model loader와 병행하고, 선택 target이 준비되면 다른 background target을 기다리지 않는다. animevents 사전 scan과 full parse의 중복도 한 번의 parse로 합쳤다.
- 차원술사 BA 연계가 UI input block에서 끊기던 원인을 수정했다. COMBO mouse-up은 continuation을 취소하지 않고, HOLD만 release packet을 소비한다.
- 차원술사 R은 원본 cue 기준 `0.5 / 0.7 / 1.05초`, 검격 `1 / 1 / 3`과 Helix TypeData pitch `-90도` exactly-once 계약을 복원했다.
- T는 이중 제출이 아니었다. full `Play All`과 model-only의 GPU pipeline 계수가 동일했고 elements-only 제출은 0이었다. 잘못 추가됐던 OPAQUE 강제를 되돌리고 원본 MASKED 0.333 cutoff를 유지했다. 사용자 T Authored JSON은 수정하지 않았다.

## 1. 실제 변경

### 1.1 Effect Tool과 Hot Reload 제거

- `CMainApp`의 Effect Tool include, 상태, update/render, F1 버튼과 lazy creation을 제거했다.
- `Client.vcxproj/.filters`와 ClientFrontendHarness에서 Effect Tool 및 Direct Authored Source Index 등록/의존을 제거했다.
- `Client/Private/Effect_Tool.cpp`, `Client/Public/Effect_Tool.h`, `Effect_DirectAuthoredSourceIndex.*`를 삭제했다.
- `PublishSelectedDirect`, transaction receipt, rollback/finalize mode와 `publish_selected_direct_effect.py`를 삭제했다.
- 혼동을 주던 이전 Save Hot Reload PLAN/RESULT는 이 문서로 대체하고 삭제했다.

### 1.2 runtime catalog 단순화

- full publisher의 direct-authored row는 `payloadKind`, `effectAssetId`, `authoringFormatVersion`, `authoredDocumentPath` 네 필드만 출력한다.
- 기존 checked-in 6-field catalog를 한 번에 깨뜨리지 않도록 Client loader는 4-field와 과거 6-field를 transitional하게 읽되 과거 SHA/dependency 값은 소비하지 않는다.
- sealed path는 `Authored/<effectAssetId>.<64 lowercase hex>.effect.json` 문법, catalog root 내부 regular file, 16MiB 상한을 확인한다. 이는 content approval이 아니라 경로 이탈·비정상 입력 방지 계약이다.
- direct document는 read/parse/version/ID 확인을 한 번 수행한다. 중복 content SHA, dependency equality, 추가 drawable validation은 제거했다.
- renderer prepared key는 document 전체 JSON 재직렬화 대신 immutable document identity와 Effect ID를 사용한다.
- ProductCue approval/admission source, runtime sidecar, token, spawn/binding 검사와 hot-reload snapshot/reprepare API를 제거했다.

### 1.3 Character Select 준비 경로

- Loading 진입 직후 선택 class와 encounter target을 우선 queue에 등록해 map/model loading과 겹친다.
- activation은 현재 catalog revision의 선택 target만 본다. 관계없는 global pending queue는 Character Select 교체를 막지 않는다.
- `Load_ForProductPrewarm`은 animevents를 선행 line scan한 뒤 다시 읽지 않고 한 번 parse한 결과에서 clip과 Product cue를 수집한다.
- 실제 character spawn의 model clip/bone/anchor 검증과 prepared-only/no-I/O spawn 계약은 유지한다.

### 1.4 차원술사 BA

- filtered mouse state를 물리 mouse-up으로 오인하지 않도록 raw physical LMB 상태를 별도 조회한다.
- 첫 LMB는 즉시, hold는 180ms 뒤 100ms 간격으로 재전송한다. UI block은 command eligibility만 막고 실제 hold 상태를 release로 바꾸지 않는다.
- Client는 COMBO mouse-up에 release packet을 보내지 않는다. Server `Release`는 HOLD만 처리한다.
- 기존 BA1 `1400ms`, `comboAdvanceMs`, multi-hit 완료 gate, pending MOVE/SKILL full action-duration boundary와 explicit command 우선순위는 유지한다.

### 1.5 차원술사 R과 sprite 경계

- R source 검격 occurrence를 `0.5s x1`, `0.7s x1`, `1.05s x3`으로 materialize한다.
- 검격은 SpriteWave profile 20과 source main/dissolve/UV-noise/inherited-emissive lane을, 바닥 sprite는 ParticleMaster profile 19의 C coverage/D warp/E-F emission lane을 사용한다.
- Helix는 source rotation/rate와 TypeData pitch `-90도`를 한 번만 적용한다.
- `fluidtile`, `mirnoise`, `noise` 같은 filename만으로 공통 radial/slice mask를 강제하지 않는다. 실제 material family와 named lane이 일치할 때만 profile을 공유한다.

### 1.6 차원술사 T 정정

- T Authored 문서는 read-only로 유지했다. 최종 확인 SHA-256은 `8113D5A26E9D71E01A18B9F5B38C19EFA4E57C402A96C8B198665ED66BB26FB5`다.
- materializer의 잘못된 `PROJECT_TUNED_CHARACTER_SURFACE_OPAQUE` 강제를 제거하고 기존 `RAW_UE3_PARENT_BLEND_MASKED_EXACT`를 복구했다.
- T exact model cue는 `Shader_VtxAnimMeshBinary` pass 3의 0.333 alpha cutoff를 사용한다.
- GPU query에서 model-only와 full `Play All`은 각각 `6801 pixels / 13689 IA primitives / 28112 PS invocations`로 동일했다. elements-only는 draw 0건이었다. 따라서 source/product, base/projected 또는 model cue의 이중 제출은 없었다.

## 2. 유지한 안전 경계와 남은 구조

- JSON schema/version/finite range, Resources-relative path, root escape 차단, byte/count 상한, stable ID와 document identity, GPU stage 실패 격리는 유지했다. 모두 없애면 성능 최적화가 아니라 잘못된 파일 하나로 process를 손상시키는 경로가 된다.
- `EffectVisualPrograms.runtime.json`은 아직 제거하지 않았다. 현재 gameplay에서 `effect.valtan.pattern.420633.active` 하나가 409-sample baked-edge history와 supplemental packet을 실제 소비한다. 해당 attachment를 ordinary runtime document로 materialize하기 전 삭제하면 발탄 whirlwind trail/light가 사라진다.
- Effect authoring은 이제 외부 JSON 편집 후 full `Publish-Effects.ps1 -Mode Publish`와 Client 재시작을 사용한다. 저장 즉시 hot reload는 지원하지 않는다.

### 2.1 현재 runtime publish 상태

- 코드와 publisher는 새 4-field 계약으로 전환됐지만 checked-in `EffectCatalog.runtime.json`은 아직 이전 6-field catalog다. Client loader가 이를 transitional하게 읽으므로 기존 runtime은 계속 열리지만 최신 R/T Authored payload는 아직 gameplay runtime에 반영되지 않았다.
- full Validate를 두 번 실행했으나 다른 세션/실행 중 Client의 Artist Authored 저장과 겹쳤다. 첫 실행은 `effect.artist.skill.31480.unified`, 두 번째는 `effect.artist.skill.31210.ba1.unified`의 byte/SHA pin 변경을 감지해 fail-closed 중단했다. partial runtime commit은 없었다.
- 사용자 요청에 따라 다른 세션의 Effect 작업을 계속 진행시켰고 global Effect 파일 동결 요청은 철회했다. 따라서 full Validate/Publish는 모든 Authored 저장이 자연스럽게 끝난 뒤 한 번 실행해야 한다.
- Valtan v2 baked-edge attachment의 7-field/detached identity를 C++ codec이 거부하던 문제는 v2에만 한정해 수정했다. full sidecar parse와 Artist projection은 PASS했다. 별도로 현재 Valtan authored base와 generated `typedCodecSha`는 외부 변경으로 drift 상태라 다음 full publish에서 함께 재생성해야 한다.

## 3. 자동 검증

- BA: Server contract 신규 tap/repeated-click/hold/pending/multi-hit/reset 계약 PASS, Client command-buffer focused harness `8/8 PASS`.
- R: isolated Debug ClientFrontendHarness build PASS, `--effect-dm-r-boundary-fast` `failures: 0`, materializer `status: stable`.
- T: isolated Debug ClientFrontendHarness build PASS, `--effect-dm-t-isolation-fast` `failures: 0`; model-only/full pipeline 통계 동일, elements-only 0.
- Character Select: target-only activation, current revision, Valtan auto-request와 non-Valtan path focused assertions PASS.
- Effect publisher: simplified `Test-EffectPipeline.ps1` PASS, Python direct runtime validator unit tests PASS.
- Catalog migration: 4-field와 transitional 6-field load, unsafe/non-64hex/cross-effect path 거부, lazy payload identity 실패 시 기존 revision 보존 등 `6 PASS / failures: 0`.
- Engine x64 Debug/Release, Shared x64 Debug/Release, Server x64 Debug/Release isolated build PASS.
- 같은 스냅샷의 Engine/Shared를 사용한 Client x64 Debug isolated relink와 ClientFrontendHarness relink PASS.
- T MASKED pass 3/TRANSLUCENT pass 4를 포함한 current Effect HLSL full/VS/PS 10조합과
  Catalog/Renderer/MainApp 선택 TU compile은 모두 errors `0`.
- Server Debug/Release `--contract-test` 모두 `failures: 0`.
- NetworkProtocolHarness의 protocol 24 하드코드를 현재 v27로 교정했다. isolated Debug rebuild/run에서 `World Destruction Protocol V27 Packet Types`를 포함해 `failures: 0`으로 PASS했다.
- Client x64 Debug isolated full compile/link PASS, ClientFrontendHarness x64 Debug full build/link PASS.
- 표준 `EngineSDK` Debug import library는 새 `Get_DIMouseStateRaw` symbol 이전 버전이라 격리 Client link에는 같은 검증에서 빌드한 최신 Engine import library를 사용했다. 일반 출력으로 다시 빌드하기 전 `UpdateLib.bat Debug`가 필요하다.
- 최종 focused 실행: catalog migration `6/6`, player command buffer `8/8`, DimensionMaster R, DimensionMaster T, matched Artist source visual-program `8/8` 모두 PASS.
- Valtan 420633 authored base를 현재 sidecar에 직접 projection하는 검사는 `Visual-program base document typed codec identity is stale`로 별도 실패한다. 다음 Valtan full publish에서 typed codec seal을 재생성해야 한다.
- Release Client/CFH는 사용자의 “현재 검증만 끝내기” 지시에 따라 추가 실행하지 않았다. Engine/Shared/Server Release는 앞선 격리 빌드에서 PASS했다.
- 검증 시작 때 존재하던 Client PID 39176/Server PID 4736은 종료 확인 시 이미 사라져 있었다. 이 세션과 검증 agent는 두 process를 종료·재시작·조작하지 않았다.
- Client/UI는 실행하지 않았다. 실제 조작감과 visual fidelity PASS는 사용자가 새 binary/data로 판정한다.

## 4. 사용자 확인 항목

1. 새 Server/Client에서 LMB 연타와 hold가 BA1 -> BA2 -> BA3 -> BA4로 이어지는지 확인한다.
2. BA 중 RMB 또는 다른 skill을 누르면 현재 stage가 끝난 뒤 continuation보다 먼저 실행되는지 확인한다.
3. Character Select 최초 진입과 class 변경 시 선택 class Effect 준비 대기가 줄었는지 확인한다.
4. R이 첫 두 타에 검격 하나씩, 마지막 타에 세 개를 재생하고 Helix가 임의 원형 회전을 하지 않는지 확인한다.
5. T에서 잘못 노출되던 OPAQUE 면이 사라지고 원본 MASKED silhouette가 보이는지 확인한다.

## 5. 도화가 후속 요청 읽기 전용 감사

### 5.1 두루미

- 두루미 cue의 `0.01`은 전역 Effect 누락 원인이 아니다. `SK_SDM_RCC_00_SK_FX_01.wmodel`의 `rcc` rest scale이 XYZ 100이라 cue scale 0.01이 이를 상쇄한다. sprite/ribbon/mesh particle은 각자 별도의 UE3 cm-to-m 및 model pre-scale 계약을 이미 가진다.
- `rcc_sk_flyinheaven` animation은 WModel에 실제 존재한다. 32 ticks/30Hz, 41 channels/4059 keys이며 날개 bone rotation도 약 44~57도 변한다. renderer도 track time, animation evaluation, bone matrices를 매 frame 적용하므로 “clip 미연결”은 아니다.
- 현재 이동은 `velocityPerSecond=(0,0,9)`가 +Z로 약 10m 이동하고 model rotation은 orientation에만 적용된다. 그래서 모델을 돌려도 경로는 회전하지 않는다.
- 후속 패널은 새 schema 없이 기존 velocity를 사용해 `Trajectory Yaw`, horizontal/vertical speed, travel distance와 optional `Align model yaw to trajectory`만 노출하면 된다. 제거한 Effect Tool을 되살리지 말고 살아 있는 authoring panel에 붙이는 별도 작업으로 둔다.
- 감사 시점 current Authored는 scale 0.01과 7 elements였고 runtime은 scale 1, 39 elements의 오래된 sealed 문서였다. 사용자의 이후 저장을 덮어쓰지 않기 위해 이번 작업에서는 수정·publish하지 않았다.

### 5.2 `fx_t_ink_01.dds`

- 대상은 Artist R `31210 콩콩이` 마지막 단계 `sdm_sk_skykongkong_02`, Product `effect.artist.skill.31210.ba4.unified`다.
- 최소 occurrence는 `Skl_01 / ParticleSpriteEmitter_2 / source-event-055` 하나다. document delay 0.5333초와 내부 burst +0.03초로 약 0.5633초에 생성되며, local-space/EPAL_Z 수평 카드, 수명 2초다.
- 핵심 lane은 `alpha_tex=fx_t_ink_01.dds`, `uv_dissolve_tex=fx_d_noise_030.dds`, dissolve hardness 100이다. 같은 계열 숨김 row 여섯 개를 모두 켜면 중복되므로 하나만 targeted 복원해야 한다.
- 현재 해당 row들은 `fallback-blocked`, `execution.enabled=false`, `failClosed=true`라 visible만 켜서는 재생되지 않는다. 기존 generic sprite path로 mask+dissolve 두 lane만 소비하는 좁은 material 계약이 후속 구현 단위다.

### 5.3 `fx_o_symbol_14.dds`

- 사용자가 설명한 중앙 회전 후 size 0 소멸은 T가 아니라 Artist Z `31050 저무는 달`, clip `sdm_sk_harmonyofyin_02`, Product `effect.artist.skill.31050.clip2.unified`의 기존 visible row다.
- occurrence delay 0.0291초, 수명 0.8초, 지면 수평 EPAL_Z, 회전률 -2 turns/sec(-720도/sec), size-over-life 1 -> 0 계약이 SourceRecipe에 이미 있다.
- T `31950`에는 이 texture가 없고, D에서는 dissolve mask 역할이므로 filename만 보고 T/D에 base symbol로 추가하면 잘못된 매핑이다. 이번 작업에서는 데이터를 바꾸지 않았다.
