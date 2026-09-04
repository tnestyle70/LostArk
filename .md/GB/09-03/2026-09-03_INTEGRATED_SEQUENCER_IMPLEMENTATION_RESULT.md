# 2026-09-03 Boss Composition · Arena Sequencer 통합 구현 결과

## 1. 결론

발탄과 쿠크세이튼을 같은 저작 모델로 확장하기 위한 `Boss Composition`과 `Arena Sequencer`의
첫 수직 슬라이스(C00~C03)를 구현했다.

- 발탄은 기존 typed owner 전체를 `42 pattern / 194 stage`로 strict join한다.
- 쿠크세이튼은 현재 존재하는 `4 profile / 349 action` reference를 lossless하게 묶는다.
- Arena 문서는 발탄과 쿠크세이튼 각각 하나씩 추가했다.
- Composition 전용 validator, transactional publisher, receipt, BuildDomains 연결을 추가했다.
- Valtan Composition Save/Create transaction에 descriptor를 포함해 부분 저장과 revision drift를 막았다.
- Client Sequencer에서 두 Boss/Arena 문서를 선택하고 last-good pair로 열 수 있다.
- Native parser도 Python validator와 같은 고정 role/path closure를 검사하며 실패 시 이전 문서를 유지한다.

현재 문서 상태는 의도적으로 발탄 `SHADOW`, 쿠크세이튼 `REFERENCE_ONLY`, 두 Arena 문서
`SHADOW`다. `runtimeEligible=false`이며 아직 Server gameplay 정본을 Composition으로 바꾼 것이 아니다.
기존 split owner와 Server fixed-tick authority를 유지하면서, 손실 없는 통합 read model과 안전한 저장 경계를
먼저 닫은 상태다.

## 2. 데이터 구조

```text
전문 typed owner
  ├─ gameplay / stage / branch / motion / hit
  ├─ animation occurrence
  ├─ Effect V1 cue / Effect V2 binding
  ├─ Sound / Camera shake
  ├─ combat object / world event
  └─ Kakul action reference / map world sequence / camera shot
                    │
                    │ stable ID + exact role/path + source hash
                    ▼
Data/Compositions
  ├─ Bosses/Valtan.bosscomposition.json
  ├─ Bosses/KakulSaydon.bosscomposition.json
  ├─ Sequences/ValtanArena.sequencer.json
  └─ Sequences/KakulSaydonArena.sequencer.json
                    │
                    │ validate -> stage -> commit -> receipt last
                    ▼
Client/Bin/DataFiles/Compositions
  ├─ Bosses/*.bosscomposition.json
  │    ├─ resolved unified pattern/reference graph
  │    └─ detached cue inventory
  ├─ Sequences/*Arena.sequencer.json
  └─ Composition.publish.receipt.json
                    │
                    ├─ 현재: Client Sequencer inspection/source facade
                    └─ 현재: 기존 Valtan Workbench Play/Save를 그대로 연결

Server gameplay
  기존 split Product + pinned gameplay revision + fixed-tick authority 유지

최종 목표
  Composition authoritative generation
    -> Server gameplay projection
    -> Client Animation/V1/V2/Sound/Camera/UI/Hit adapters
    -> 같은 occurrence clock, stop policy, revision을 소비
```

Composition은 Effect element, WAV, Camera keyframe, map placement 본문을 복제하는 만능 파일이 아니다.
각 전문 owner의 stable ID와 호출 계약을 같은 pattern/stage/occurrence 시간축으로 join하는 상위 정본이다.
이 원칙을 지켜야 Effect Tool, Camera Tool, Map Tool의 두 번째 writer를 만들지 않으면서도 한 화면에서
전체 pattern을 편집할 수 있다.

## 3. 이번에 실제 저장되는 정보

### 3.1 Boss Composition source

`lostark.boss-composition` v1은 다음을 저장한다.

- `compositionId`, `status`, `revision`, boss/encounter/area identity
- 고정 `(role, path)` source document closure
- 기대 pattern/stage 또는 profile/action coverage와 identity hash
- stable pattern ID inventory

발탄 source는 gameplay, presentation, combat object, world event, animation binding, Effect V1 cue/alias,
Effect V2 binding, pattern Sound/Shake, combat-object Sound owner를 참조한다.

쿠크세이튼 source는 네 model profile의 action reference, action binding, pattern binding을 참조한다.
아직 Server boss product가 없으므로 `bossArchetypeId`, `encounterId`를 지어내지 않고 null로 유지한다.

### 3.2 Arena Sequencer source

`lostark.arena-sequencer` v1은 area, boss composition, duration, source closure와 typed track을 저장한다.

- Valtan Arena: 현재 track 0개인 안전한 SHADOW 문서
- KakulSaydon Arena: `WORLD_SEQUENCE`와 `CAMERA_SHOT` 두 track, 21,010 ms

쿠크 track은 기존 world sequence instance `world.sequence.instance.circusfinale`와 camera shot `shot.1`을
참조한다. 실제 resource 본문은 기존 Map/Camera owner에 남는다.

### 3.3 Product와 receipt

Publisher는 source 네 문서와 모든 transitive owner를 다시 읽어 정확히 검증한 뒤 임시 경로에 Product를
만든다. output lock과 durable journal을 사용하고 네 Product를 교체한 뒤 receipt를 마지막에 commit한다.
중간 실패나 비정상 종료는 이전 Product/receipt를 보존하거나 journal recovery로 복구한다.

현재 receipt:

```text
sourceManifestId = 279a5d84b79d0dfcbd0cb7694f008ff9061ead207457fa9c6f7282405f0f656c
transitive source files = 234
products = 4
Valtan detached cues = 304
```

detached cue는 누락이 아니다. 현재 42개 managed pattern/stage에 직접 붙지 않은 기존 V1/V2/Sound/Shake/
combat-object Sound를 버리지 않고 별도 inventory로 보존한 것이다.

## 4. 코드와 파이프라인 반영

### 4.1 Validator / Publisher

- `Tools/CompositionPipeline/composition_pipeline.py`
- `Tools/CompositionPipeline/Publish-Compositions.ps1`
- `Tools/CompositionPipeline/test_composition_pipeline.py`
- `Tools/CompositionPipeline/test_native_composition_facade_contract.py`

검증 범위는 exact schema, duplicate JSON key, stable ID, role/path closure, Valtan full owner join,
Kakul action reference coverage, Effect V1 alias/transform, Effect V2 binding, Sound catalog, Camera cue/keyframe,
Kakul world-sequence/camera-shot owner와 범위를 포함한다.

### 4.2 Save / Create transaction

- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
- `Tools/ValtanPipeline/promote_valtan_animation_chains.py`

Composition descriptor를 기존 canonical typed Save와 Create Pattern의 CAS/rollback transaction에 포함했다.
Save 성공 뒤 descriptor만 낡거나, descriptor만 commit되고 gameplay/presentation이 rollback되는 경로를 허용하지
않는다. Repository source revision에도 descriptor가 포함된다.

### 4.3 Client facade

- `Client/Public/BossCompositionDocument.h`
- `Client/Private/BossCompositionDocument.cpp`
- `Client/Public/SequencerTool.h`
- `Client/Private/SequencerTool.cpp`
- `Client/Public/ActionCompositionWorkbench.h`

Sequencer는 Valtan/Kakul 선택, Boss/Arena source pair 원자적 load, 상태·revision·source·pattern/profile·track
inventory 표시를 지원한다. 잘못된 role/path, 추가 source, Kakul profile source drift가 있으면 새 문서를
commit하지 않고 last-good pair를 유지한다.

Valtan 선택 시 기존 Workbench의 timeline, Play, Save 경로를 계속 사용한다. 이번 변경은 두 번째 Valtan
player/writer를 만들지 않았다.

### 4.4 Build / Harness

- `Tools/Build/BuildDomains.json`: `composition.presentation` domain 추가
- `Tools/Build/Invoke-BuildDomainOwner.ps1`: Client build에서 Composition domain 소비
- `RunFullPipeline.bat`: split publish -> post-condition -> domain/build/regression 순서 유지
- `Tools/Build/Run-FullPipeline.ps1`: child stderr와 exit code를 분리하고 PowerShell scriptblock 변수 충돌 수정
- `Tools/Build/Test-CompiledShaderClosure.ps1`: nested runner에서도 동작하는 .NET SHA-256 사용
- 기존 `ValtanPatternAuditionServiceHarness`에 Boss Composition native contract 7개 추가

별도 미래용 harness project를 만들지 않고 기존 제품 인접 harness를 확장했다.

## 5. 검증 결과

### 5.1 PASS

```text
Composition focused Python contracts                     127/127 PASS
Composition standalone Validate                          PASS
Composition standalone Publish                           PASS
BossCompositionDocument native contracts                   7/7 PASS
ValtanPatternAuditionServiceHarness                       30/30 PASS
ValtanPatternFlowServiceTests                              13/13 PASS
ValtanTuningCommandServiceTests                            11/11 PASS
NetworkProtocolHarness                                  failures 0
Server contract tests                                   failures 0
Character Select Core / Party2 / Party4                    PASS
Product Client/Server build + compiled shader closure      PASS
Physics / destruction / map / rendering contracts          PASS
```

Debug FullDiagnostic 전체 경로는 979.4초 동안 실행되어 build와 위 광역 계약을 모두 통과했다.
증거 파일은 `out/BuildPipeline/runs/20260903T143322989Z-debug-fulldiagnostic-d37c5d5e.json`이다.

단, 아래 §5.2를 진단하는 동안 Effect V1 lifetime assertion을 일시적으로 6초로 완화한 상태에서 이 전체
receipt가 만들어졌다. 교차 검토 뒤 assertion을 올바른 라이브 계약 5초로 복원했으므로, 이 evidence를
현재 최종 워킹트리의 완전한 clean FullDiagnostic admission으로 과장하지 않는다. Composition에 직접 관련된
127개 계약과 standalone publish는 복원 뒤 다시 PASS했다.

### 5.2 현재 정확히 남은 회귀 1건

최종 워킹트리에서 다음 전용 계약은 `18개 중 17 PASS, 1 FAIL`이다.

```text
python -m unittest \
  Tools.ValtanPipeline.test_valtan_combat_object_hit_effect_presentation_contract

FAIL
expected V1 active lifeTimeSeconds = 5.0
actual                              = 6.0
```

원인은 이번 Composition 코드가 아니라 작업 전부터 존재한
`Data/Effects/Authored/effect.valtan.ground-roar.rock.active.effect.json` 변경이다.

```text
Server combat object lifetime   6200 ms
Server explosion pulse          5000 ms
Product V2 active               0..5000 ms
Product V2 explosion            5000..6200 ms
V2 active leaf lifetime         5.0 s
현재 V1 authored active mesh    6.0 s
```

같은 V1 문서의 여러 element delay도 기존 `0 / 1.6 / 1.8초`에서 `4 / 5.6 / 5.8초`로 이동했다.
이 값은 Server fallback/disconnect를 발생시키지 않지만 Composition local V1 preview와 실제 live V2의
시각 결과를 다르게 만들고, 일부 element는 6.2초 carrier 종료에 잘릴 수 있다.

사용자 소유 Effect 변경을 임의로 되돌리지 않았다. 해결은 둘 중 하나를 명시적으로 선택해야 한다.

1. 기존 live 연출 유지: V1 lifetime/delay를 기존 5초 계약으로 복원한다.
2. 6초 재연출 승인: V2 group active duration, V2 active leaf lifetime, explosion overlap/stop 정책까지 같은
   변경 단위로 수정하고 전용 visual parity contract를 갱신한다.

테스트 숫자만 6으로 바꾸는 것은 회귀를 숨기므로 적용하지 않았다.

## 6. 아직 구현하지 않은 것

이번 C00~C03 완료와 다음 C04~C05를 구분한다.

- Animation/V1/V2/Sound/Hit mirror가 한 local playhead를 소비하는 generic Play transport
- Sound instance handle과 stop, Pause/Seek/Restart 중복 방지
- Stage/Pattern exit의 공통 stop policy 실행
- Arena `SCREEN_POST`, `LIGHT`, `UI`, `SPAWN` runtime adapter와 editor box writer
- 쿠크세이튼 첫 Server gameplay pattern, damage/collider/branch Product
- Valtan 전용 Server runner의 generic boss runner 승격
- Composition `AUTHORITATIVE` 전환과 split owner의 generated compatibility output 강등
- 피자 성장 Effect와 Server damage geometry의 공통 `geometryBindingId`
- 돌 mesh가 아니라 Server cover geometry로 피격을 차폐하는 typed cover contract

따라서 현재 Sequencer에서 쿠크 source/profile과 Arena track은 볼 수 있지만 쿠크 gameplay pattern을 Save/Play해
Server 전투로 실행하는 단계는 아니다. Valtan은 기존 Workbench 기능이 유지된다.

## 7. 사용자 수동 확인 경계

자동 검증은 Client를 실행하거나 화면·음향을 판정하지 않았다. 현재 PC는 팀 endpoint
`192.168.0.4:7777`의 `server-host`로 동기화되었고 probe `not-listening`은 Server가 꺼진 정상 상태다.

사용자 smoke 순서:

1. `Server + Client` profile로 시작한다.
2. F1의 Sequencer/Animation Composition에서 Valtan과 KakulSaydon을 각각 선택한다.
3. Valtan에서 42 pattern inventory와 기존 timeline lane이 유지되는지 확인한다.
4. KakulSaydon에서 4 profile / 349 action reference와 Arena track 2개가 보이는지 확인한다.
5. Valtan pattern 하나를 Save하고 publish 상태가 완료된 뒤 Server를 재시작한다.
6. Restart/Queue/Next가 기존 pinned revision과 occurrence identity로 동작하는지 확인한다.
7. `ground-roar`는 §5.2 선택을 닫기 전 visual PASS로 판정하지 않는다.

## 8. Git 상태

- 브랜치: `GB/sequencer-composition`
- 착수 HEAD = `origin/main`: `85cebb02a9e300a1113be33c527ae26939e8128d`
- 자동 commit/push는 하지 않았다.
- 기존 사용자 Effect 변경과 분석 문서는 보존했다.
- `Client/Bin/Resources` binary pack은 Git 변경 대상으로 다루지 않았다.

