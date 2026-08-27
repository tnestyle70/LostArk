# Valtan Phase 2 패턴 기믹 구현 결과

## 2026-08-27 모아치기 Draft Product 승격 및 저장본 재감사

사용자가 마지막으로 지정한 `effect.valtan.sequence.charge` 한 건만 Product로 승격했다. authored
파일은 쓰거나 재생성하지 않았고 SHA-256
`4E767B51C60741E13282F48DB08D90BF32E04B1D9C03F26291258016CDE5F2FC`와 사용자 element 2개를
그대로 보존했다.

- Effect Catalog: `DIRECT_AUTHORED_DOCUMENT`와 exact authored path 등록
- source cue: `VALTAN_CHARGE / STEP_01 / valtan.sequence.charge.step-01.clip-01 / t=0`
- generated Product cue: 같은 pattern/stage/action/clip/effect ID로 PublishV2 투영
- 기존 `effect.valtan.project-tuned.sequence.charge` authored 파일과 Catalog row는 비교 입력으로
  보존하되 Product cue만 제거해 이중 재생을 막음
- sidecar: CHARGE row만 제거하고 `effect.valtan.sequence.rush` 한 건은 요청대로
  `DRAFT_ATTACHED`로 보존
- Client Data 프로젝트 등록: `2375 files / 219 filters` 동기화 및 Check PASS

사용자가 당일 저장한 주요 Product를 다시 parse하고 source cue와 generated cue를 exact occurrence로
대조했다. 피자 11 elements, 점프 찍기 후 휠윈드 6, 모아치기 2, 모아치기2 2, 사자후 위로
모아치기 7, 3연속 내려치기 6, 잡아채서 불어날리기 3, 카운터 내려치기 2,
전방-후방-전방 20, 워프 15 elements/8 occurrences가 모두 Catalog·파일·Product cue 일치로
확인됐다. 당일 custom Product 중 cue가 0인 것은 의도적으로 교체·보존한 기존 8-element
`effect.valtan.project-tuned.sequence.charge` 하나뿐이다.

회귀 방지를 위해 두 authoring generator도 CHARGE의 단일 STEP_01 Product cue를 유지하도록
맞췄다. 전체 requested-effect `Apply`는 실행하지 않았다. 현재 사용자 튜닝 Product 9개에 generator가
35개 element를 추가하려는 상태이므로, 해당 전역 Apply는 사용자의 기존 수치를 보존하기 위해 계속
금지한다.

추가 자동 검증:

- Valtan Phase 2 authoring Validate: PASS
- Valtan split PublishV2: PASS, changed 1/7 Product artifact
- Valtan split ValidateV2: PASS
- Valtan Pattern Master V2: 49/49 PASS
- Valtan Pattern Tree: 20/20 PASS
- Product unlink: 7/7 PASS
- saved Product rows: 32 PASS, 환경 조건부 7 skip
- Draft ownership/All Effects/promotion focused: 23 PASS
- Effect Data project registration Check: PASS

`Validate-EffectSources.ps1`의 JSON, Catalog, authored identity 검사는 통과했지만 기존 Git 미추적 DDS
5개 때문에 최종 exit 1인 경계는 이전과 동일하다. 이 실패는 이번 CHARGE 승격이나 사용자 저장
누락이 아니다.

## 결과

2026-08-27 사용자 요구를 Server 권위 패턴으로 연결했다. 패턴 이름이나 애니메이션만 추가한
상태가 아니라 `Data/Valtan -> Gameplay.bootstrap v24 -> CValtanBrain/BossCombatRuntime ->
Shared snapshot -> Client presentation` 경로를 사용한다.

Client와 UI는 에이전트가 실행하거나 조작하지 않았다. 아래 자동 검증은 통과했지만 실제 화면의
Effect 크기, 색, 카메라 구도와 체감 타이밍은 사용자가 새 Debug Client로 확인하기 전까지
`visual PASS`로 처리하지 않는다.

## 구현 상태

| 요구 패턴 | 현재 실행 계약 |
|---|---|
| 돌진 후 갑옷 파괴 | `VALTAN_DASH_CHARGE`가 벽 접촉 뒤 `RECOVERY`로 들어간다. 이 취약 구간의 실제 player skill part hit만 `PART_DESTROYED -> GROGGY -> PART_BREAK`로 진행한다. 취약 구간 밖 hit와 이미 파괴된 부위는 거부한다. |
| 모아치기 | `VALTAN_CHARGE`는 시작 시 한 생존 플레이어를 잠그고 같은 PlayerId의 현재 위치를 매 tick 추적한다. 발탄 yaw, CONE 판정과 모아치기 Effect가 같은 방향을 사용한다. |
| 점프 후 3시/9시 지형 파괴 | `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK`, `VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK` 두 독립 패턴을 추가했다. 둘 다 2초 `AIRBORNE` 뒤 착지 world pose에 고정된 빨간 반원 cue, Server hit와 해당 floor world-event-set을 같은 `IMPACT` edge에서 실행한다. |
| 중앙 이동 후 2페이즈 컷씬 | `VALTAN_ARENA_BREAK_109`의 중앙 leap, 외벽 30-group 파괴와 비산물 경로를 유지했다. `BOSS_FACING` rear wide shot 진입에 250ms smooth transition을 추가하고 `PLAYER_BOSS_FRAME`을 거쳐 gameplay follow로 복귀한다. |
| 피자 점프 착지 카메라 | `VALTAN_SIX_PIZZA_106/STEP_03` 착지 tuple에 발탄과 local player를 함께 잡는 `PLAYER_BOSS_FRAME` cue를 연결했다. |
| 순간이동 반복 돌진 | `VALTAN_WARP`의 STEP_02~09마다 새 생존 플레이어를 Server에서 다시 선택하고 그 yaw로 기존 authored root motion과 BOX hit를 실행한다. 각 leg는 발탄 정면 +Z 3m에서 같은 `effect.valtan.project-tuned.sequence.warp.portal` Product를 반복 재생한다. Product 하나 안에 portal 14 layer와 3구르기 이후 전방 rush mesh 1개가 함께 있으며 별도 portal-enter/rush Product cue는 없다. |
| 버러지 반복 돌진/카운터 | 전방 grab hit가 플레이어를 `GRABBED/LEFT_HAND`으로 만들고 boss-local offset을 snapshot한다. Client는 최초 player world와 `bip001-l-hand` world matrix로 hand-local offset을 계산해 뼈를 따라간다. boss-local 왼손 counter proxy에서 성공하기 전에는 rush를 반복하고, 성공하면 잡힌 플레이어 전원을 해제한 뒤 성공/groggy 경로로 진행한다. |
| 카운터 내려찍기 | `VALTAN_COUNTER`의 WINDUP 중 실제 counter hit만 groggy로 진행한다. TIMEOUT은 내려찍기 판정을 실행한 뒤 terminal finish하며 groggy를 공유하지 않는다. |
| 뒤쪽 잡아채서 날리기 | `VALTAN_CATCH_BREATH`는 시작 시 발탄 뒤 sector의 임의 생존 플레이어를 잠근다. 후방 노란 CONE hit로 왼손에 붙이고 hold 뒤 `BOSS_OPPOSITE` knockback으로 해제한다. |

## 코드와 데이터 연결

- `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`가 위 pattern/stage/branch와
  presentation tuple을 재생성하고 `Validate`에서 drift를 검사한다.
- `Tools/ValtanPipeline/author_valtan_requested_effect_elements.py`는 기존 authored element를
  보존한다. WARP는 단일 15-element Product를 사용하며 다른 작업이 조정하는 CHARGE authored
  수치를 재튜닝하지 않는다.
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`와
  `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`이 source version 24의
  `partDamagePolicy`, `counterProxy`, rear target policy와 28-step ordered sequence를
  제품 bootstrap으로 투영한다.
- `Server/Private/GameplayCatalog.cpp`, `ValtanBrain.cpp`,
  `BossCombatRuntime.cpp`, `ServerCombatHitRuntime.cpp`가 typed data를 읽고 fixed tick에서
  target, hit, counter, part, grab/release와 world-event를 판정한다.
- `Shared/Public/Network/PacketMessages.h`의 grabbed attachment owner/slot/offset을
  `Client/Private/ClientReplication.cpp`가 소비해 실제 왼손 bone presentation을 만든다.
- `Client/Private/EncounterPatternReference.cpp`는 새 optional stage field와 rear target policy를
  fail-closed로 읽는다.
- `Client/Private/ValtanPatternTree.cpp`도 `partDamagePolicy`와 `counterProxy`를 split 원본,
  joined master, Product encounter, 최종 Tool view까지 typed optional field로 보존한다. 2026-08-27
  Effect Tool에서 이 두 필드를 unknown property로 판단해 전체 Valtan inventory와
  `Create Effect`가 함께 사라진 strict-join 회귀를 수정했다. 값, 범위, branch/flag 결합과
  Product parity가 맞지 않으면 계속 fail-closed한다.
- `Data/Encounters/Valtan/ValtanCinematicCamera.json`과 Client cinematic controller가
  `BOSS_FACING`, `PLAYER_BOSS_FRAME`, bounded lerp와 live follow 복귀를 처리한다.

## 자동 검증

다음 검증을 2026-08-27 현재 작업공간에서 실행했다.

- Phase-two authoring `Validate`: PASS
- Requested Effect authoring `Validate`: PASS, 17 targets, changed 0
- Valtan Pattern Master V2: 49/49 PASS
- Valtan Pattern Tree: 20/20 PASS
- Requested Effect focused tests: 15/15 PASS
- Pattern Tree + Effect Tool inventory/saved-row focused tests: 68 PASS,
  환경 조건부 7 skip
- Camera Tool contract: PASS
- World destruction transition contract: 8/8 PASS
- World destruction publisher `Validate`: PASS,
  105 groups / 157 bindings / 105 emitters
- Gameplay balance publisher `Validate`: PASS,
  53 boss patterns / 228 stages / 52 audition timeline rows
- Effect data project registration: PASS, 2374 files / 219 filters
- NetworkProtocolHarness Debug: PASS, failures 0
- Server Debug `--contract-test`: PASS, failures 0
- ActionPresentationTimelineHarness Debug: PASS
- EffectRenderContractHarness Debug: PASS, exit 0
- Client x64 Debug compile/link: PASS,
  `Client/Bin/Debug/Client.exe`

Debug 빌드의 기존 shader/PDB/코드페이지 경고는 남아 있으나 이번 변경의 compile/link error는 없다.
Release 빌드와 Client 수동 smoke는 이번 검증에서 실행하지 않았다.

## 남은 경계

`validate_effect_sources.py`는 런타임에 실제 존재하지만 Git이 추적하지 않는 DDS 다섯 개 때문에
실패한다.

- `Effect/Valtan/Textures/FX_TEX_02/fx_d_cloud_031.dds`
- `Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_04.dds`
- `Effect/Valtan/Textures/FX_TEX_05/fx_o_sector_05.dds`
- `Effect/Warlord/Textures/FX_TEX_00/fx_a_hit_007.dds`
- `Effect/Warlord/Textures/FX_TEX_00/fx_a_ring_001.dds`

`Client/Bin/Resources`는 팀장이 관리하는 입력이므로 이 작업에서 자동 Git 추가하지 않았다.
Server contract 실행을 위해 기존 `Server.exe --bind-address 127.0.0.1` 프로세스는 종료했으며
자동 재시작하지 않았다.

## 사용자 수동 확인

1. Visual Studio의 `Server + Client` profile로 실행하고 Valtan Arena에 진입한다.
2. Boss Tool에서 위 표의 패턴을 하나씩 Server Play/Repeat한다.
3. Dash는 벽 접촉 전 hit가 아닌 RECOVERY의 부위 공격만 갑옷과 mesh를 파괴하는지 확인한다.
4. CHARGE는 이동하는 target, 발탄 방향, CONE과 Effect가 같이 도는지 확인한다.
5. 3시/9시는 2초 체공, 올바른 쪽 빨간 반원과 floor 파괴가 일치하는지 확인한다.
6. 109와 피자는 카메라 진입/복귀가 튀지 않고 player와 boss를 의도한 구도로 잡는지 확인한다.
7. WARP는 여덟 leg마다 새 방향으로 같은 15-element Product가 재생되는지 확인한다.
8. TRASH/CATCH는 실제 왼손 부착, counter 해제와 boss 반대 방향 knockback을 확인한다.

사용자 관찰 전에는 위 항목을 visual 완료로 기록하지 않는다.
