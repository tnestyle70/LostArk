# 발탄 벽 파괴 반도넛 발판·Boss Tool 무리셋 Live Flow 구현 계획

## 2026-08-28 추가 승인: 9시 점프도 패턴 0초에 연결

사용자가 9시에도 같은 수정을 요청했다. 아래 최초 수정에서 보존했던 9시 IMPACT cue를
`TAKEOFF` 첫 clip의 `sourceStartMs=0`으로 옮긴다. 현재 사용자가 저장한 반원 Element의
회전 `[-90, -180, 0]`, Start Delay 1.3초와 착지 Element의 Start Delay 3.5초를 포함해
Effect 문서 전체 bytes는 변경하지 않는다. 정본 presentation과 생성된 runtime cue,
생성기 기본값과 기존 회귀 테스트만 변경한다. C++와 exe 재빌드는 필요 없다.

## 2026-08-28 최종 사용자 범위: 빈 발악 Effect와 두 패턴의 0초 기준

최종 요청은 다음 두 항목이다. 아래 과거 계획의 복구·DDS·Live Flow 범위를 다시 실행하지 않는다.
작업·빌드 대상은 `C:/Users/user/Desktop/LostArk/Framework.sln`이다.

- `VALTAN_STRUGGLING`의 기존 aggregate 문서를 백업한 뒤 Element와 model cue를 비우고,
  동일 asset ID의 Draft 하나로 연결한다. 사용자가 삭제한 39개 Element나 Product cue를 복원하지 않는다.
  재생 가능한 Product와 편집 가능한 빈 Draft를 구분하고, 생성기 재실행도 빈 Draft를 보존한다.
- 사자후 composite는 `STEP_03`에서 `STEP_01`, 지형 파괴 3시 semicircle은 `IMPACT`에서
  `TAKEOFF`로 옮겨 각각 첫 clip의 `sourceStartMs=0`에 연결한다. 두 Effect의 Start Delay 0이
  전체 패턴 Timeline 0과 일치하도록 실제 cue 원점을 바꾼다. 아래의 “cue 시점은 바꾸지 않는다”는
  이전 방침은 이 최종 요청으로 대체한다. Element의 저장된 지연·수명·크기·색은 유지한다.

9시의 IMPACT cue와 native emitter 지연 `0.230894초`, 사용자 DDS와 1·2페이즈 4연속 Effect는
변경하지 않는다. 전체 Start Delay UI를 다른 시간 단위로 바꾸지 않는다. 기존 공용 Timeline
변환과 Tool 호출을 연결하고, 실제 authoring에서 생성한 runtime cue에도 동일 변경을 반영한다.
새 C++ 파일이나 프로젝트 등록은 필요 없다.

검증은 Desktop Debug Client 컴파일·링크, 두 native harness, 관련 Python 계약 테스트와
Effect/Valtan validator로 수행한다. Client/UI는 실행하거나 조작하지 않으며 최종 화면 확인은
사용자가 한다. 최종 결과는
`../08-28/2026-08-28_VALTAN_EMPTY_STRUGGLING_AND_PATTERN_ZERO_RESULT.md`에 기록한다.

## 2026-08-28 후속 승인: 재생 시계와 실제 반원 DDS

사용자가 Start Delay와 애니메이션 0초의 차이, 조기 소멸을 확인한 뒤 수정을 승인했다.
또한 UV 절반 선택 대신 Warlord의 `FX_TEX_01/fx_c_symbol_003.dds`를 속 빈 반원으로
만들어 Valtan 텍스처에 추가하고, 그 DDS를 사용하는 Element를 추가하도록 요청했다.
이 후속 요청이 아래 초기 실행 범위의 절단 방식보다 우선한다. 기존 `diffuse.dds`,
원본 Warlord DDS, 사용자 추가/튜닝 행은 덮어쓰지 않는다.

### 시간 수정의 소유 경계

- 지형 파괴 3시의 IMPACT 시작은 전체 패턴 3.4초이며, Effect 로컬 0초다. clip trim과
  cue source start는 모두 0.2초라 두 값을 중복 가산하지 않는다. Server/cue 시점은 바꾸지 않는다.
- Effect Tool은 전체 timeline, Effect local sample, Effect 0초의 timeline 위치를 함께 표시한다.
  Element Start Delay는 Effect 로컬 값으로 설명하고 native emitter delay는 별도로 표시한다.
- 새 도넛 두 행의 Timing Life 5초와 particle life 2초 불일치는 두 행에 한정하여 5/5로 맞춘다.
  생성기 재실행으로 기존 사용자 튜닝을 덮어쓰지 않는 계약은 유지한다.
- `CEffectPlayback::Calculate_ElementEndSeconds`를 runtime Stage와 Tool이 함께 사용한다.
  SourceRecipe/SourcePresentation/SourceVisual이 없는 수동 단발 particle만
  `Start Delay + particle life`로 종료한다. 연속 방출, 원본 emitter, trail 계약은 보존한다.
  정확한 종료 시점으로 Seek했을 때 한 fixed tick의 잔상이 남지 않는 실행 검증을 추가한다.
- `CActionPresentationTimeline::Resolve_CuePreviewDuration`은 기존 source/wall 변환으로
  NATURAL Effect의 꼬리를 포함한다. 3.4초에 시작한 5초 Effect는 전체 timeline 8.4초까지
  authoring할 수 있다. CUE_END는 기존 cue end를 넘겨 보이지 않게 한다. 마지막 animation
  pose에서 wall clock을 풀어 주는 기존 경로를 재사용한다.

기존 C++/header 및 하네스 파일 안에서 구현하며 새 C++ 파일과 프로젝트 등록은 필요 없다.
수치/실행 검증은 EffectRenderContractHarness와 ActionPresentationTimelineHarness,
Effect domain validator, 관련 Python 계약 테스트로 수행한다. 실행 중인 Debug Client/Server는
종료하지 않는다. 새 Client의 컴파일·링크는 별도 출력 디렉터리에서 검증하고, 현재 실행 중인
Client에 변경이 적용됐다고 보고하지 않는다. 실제 화면과 최종 방향·크기는 사용자 확인으로 남긴다.

## 2026-08-28 현재 요청으로 좁힌 실행 범위

사용자가 실린더 복구를 보류하고 이 세션에서 **점프 후 지형 파괴 3시**의 Element 추가를
요청했다. 현재 실행은 아래 범위를 따른다. 이후 본문의 3시/9시 동시 교체와 Live Flow는
원래 계획으로 남기며, 이번 구현 완료에 포함하지 않는다.

| 구분 | 이번 변경 |
|---|---|
| 대상 | `effect.valtan.project-tuned.terrain-destruction-3.semicircle` 하나 |
| 현재 실측 | `requested.20260827.terrain-3.landing.01` 하나, 시작 시간 0초 |
| 반도넛 1 | `donut.telegraph.outer.red`를 복사한 바깥 테두리 |
| 반도넛 2 | `donut.telegraph.inner.grow`를 복사한 안쪽 확장 도넛 |
| 전기 부채꼴 | 사진의 `source.d1bf9016f12267f99040` 복사본에 sector04 마스크 추가 |
| 보존 | 기존 착지 row 전체, 9시, 원본 Effect, cue/catalog, Boss Tool, gameplay |

두 도넛의 base/mask는 기존 `FX_TEX_05/fx_m_ring_001_cl.dds`를 사용한다. UV는
`sequence=false`, `1 x 2`, `tileIndex=0`으로 위쪽 절반을 고정 선택하고, particle의
start/end size를 `[0.75, 0.375]`, initial position을 `[0, 0.1875, 0]`으로 맞춘다.
양쪽 pitch를 -90도로 맞추고 X/Z 중심 오프셋을 0으로 둔다. 색, 높이, 수명과 안쪽의 기존
scale 보간은 donor에서 유지한다. UV speed는 기존 0을 유지한다. 이 방식에서 UV pan을
추가하면 마스크도 함께 이동하므로, 정적 반원 절단과 별도 UV 애니메이션을 혼동하지 않는다.

사진의 전기 원본은 다음 read-only 자료에 남아 있다.

```text
Data/Effects/Imported/Valtan/ReviewedSourceFamilies/
  effect.valtan.floor-wipe-130.second-smash.reviewed-source-candidate.effect.json
source.d1bf9016f12267f99040
```

이 row는 조사 시 HEAD의 원본 row와 JSON 값이 같았다. base/noise/emissive, emissive 100,
sourceRecipe의 9개 module, EPAL_Z, billboard, size/color/lifetime, seed와 1x1 UV를 보존한다.
`FX_TEX_05/fx_o_sector_04.dds`를 mask에 추가하고 billboard roll만 180도로 맞춰
도넛과 같은 기준 반평면을 향하게 한다. 전기 row에는 도넛의 UV crop이나 pitch를 복사하지 않는다.
sector04는 위쪽 약 60도 fan이며, 기존 alpha profile에서 mask가 최종 alpha를 제한하므로
emissive를 포함한 framebuffer 기여도 같은 fan 안으로 제한된다. 절대 월드 3시 방향과
실제 크기·밝기의 최종 육안 판정은 사용자에게 남긴다.

생성기는 기존 `author_valtan_requested_effect_elements.py`에 `--scope terrain-3-floor`를
추가한다. 기존 JSON을 요구하고 정확한 이전 sector 3개 ID만 제거하며 새 stable ID 3개를
없는 경우에만 추가한다. 기존 row와 이후 사용자 튜닝은 유지하고, 같은 CAS/transaction
경로로 저장한다. 전체 projection도 3시에는 같은 생성 정의를 사용해 과거 sector/landing을
다시 만들지 않도록 한다. 이번 Apply는 반드시 좁힌 scope만 실행한다.

검증은 기존 테스트 파일에 crop 비율·방향, 원본 전기 설정 보존, 세 row ID, 재실행 튜닝 보존,
잘못된 version/ID/중복/누락 입력, 저장 직전 변경 감지와 실패 시 무변경을 추가한다.
Effect source validator와 관련 headless harness를 실행한다. 시작 시 전체 requested suite는
사용자 삭제·튜닝 때문에 이미 16 tests / 52 failures였으며, 이와 무관한 데이터를 복구해서
PASS로 만들지 않는다. 실행 중인 Client/Server를 종료하거나 UI를 조작하지 않는다.

## 0. 작업 결론

이 후속 작업은 선행 세션 PR이 `main`에 반영된 뒤 새 `codex/valtan-half-donut-live-flow` 브랜치에서
시작한다. 구현은 서로 독립 검증 가능한 두 커밋으로 나눈다.

1. 벽 파괴 3시/9시 Product Effect의 기존 sector 세 장을 제거하고, 이미 있는 도넛 Element 하나를
   `fx_m_ring_001_cl.dds`에 연결한 진짜 반도넛 한 장으로 교체한다.
2. `Boss Tool -> Pattern Flow`의 `Play`를 현재 Product 전투에 끼워 넣는 resetless live overlay로 바꾼다.
   현재 Pattern과 기존 pursuit가 끝난 안전 경계에서 Flow를 시작하고, Flow가 끝나면 저장해 둔 Product
   cursor로 복귀한다.

핵심 사용자 동작은 다음과 같다.

```text
Product Pattern N 실행 중
  -> Pattern Flow 탭에서 Play
  -> QUEUED_AFTER_CURRENT
  -> Product Pattern N 정상 완료
  -> Product의 기존 pursuit 완료
  -> 저장된 Flow slot 1..K를 현재 아레나 상태에서 실행
  -> Product cursor N+1과 Product pursuit로 복귀
```

Flow 시작·종료 어느 쪽에서도 player, Valtan, HP, 위치, cooldown, 파괴된 벽·바닥, encounter prop을
복구하지 않는다. Flow가 실제로 만든 피해와 지형 변화도 그대로 누적한다. `Preview Isolated`만 기존처럼
fresh 단일 Pattern 진단을 위한 resetful 경로로 남긴다.

이 계획은 구현 계획만 작성한다. 현재 다른 세션의 Effect loading 변경, Product JSON, C++ 또는 binary
asset은 수정하지 않는다.

## 1. 사용자 작업 계약

### 1.1 벽 파괴 반도넛

```text
대상 Product Effect
  effect.valtan.project-tuned.terrain-destruction-3.semicircle
  effect.valtan.project-tuned.terrain-destruction-9.semicircle

재사용 donor
  effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01
  -> donut.telegraph.outer.red

요청 texture
  Effect/Valtan/Textures/FX_TEX_05/fx_m_ring_001_cl.dds

절단 방식
  기존 도넛 texture의 위쪽 절반을 static UV tile로 crop
  + carrier 높이를 절반으로 축소
  + 중심을 원래 위쪽 절반의 중심으로 이동
```

- 새 반원 DDS, 새 mesh, 새 shader를 만들지 않는다.
- 기존 두 Product Effect asset ID, cue ID, Pattern/Stage binding과 gameplay footprint를 유지한다.
- 3시와 9시는 같은 반도넛 정의를 쓰고 yaw만 정확히 180도 반대로 둔다.
- 기존 landing 두 layer는 유지하고, 선행 projection이 만든 sector 세 row만 exact stable ID로 제거한다.
- 최종 크기·바닥 높이·기준 yaw의 육안 판정은 사용자가 Effect Tool과 실제 Pattern에서 수행한다.

### 1.2 Boss Tool Live Flow

```text
Play Flow          : 저장 Flow의 첫 slot부터 현재 전투에 queue
Play From Here     : 선택 slot을 포함한 suffix를 현재 전투에 queue
Cancel Queued Play : 아직 첫 Flow slot이 시작되지 않았으면 즉시 취소
Stop After Current : Flow가 시작됐으면 현재 Flow occurrence까지만 완료하고 Product로 복귀
Preview Isolated   : 기존 단일 Pattern resetful 진단
```

- Flow JSON의 stable `flowId`, `slotId`, 순서, 저장/reload 계약은 바꾸지 않는다.
- Client가 slot별 단일 재생 packet을 반복하지 않는다. Server가 기존
  `BOSS_PATTERN_SEQUENCE_DEFINITION` / `CValtanBrain` ordered runtime을 그대로 소비한다.
- 현재 Product occurrence는 중단하거나 재시작하지 않는다.
- Product cursor는 현재 occurrence가 성공 완료될 때 기존 `FinishPattern()`에서 정확히 한 번만 전진한다.
- Flow 완료 뒤 Product Pattern을 처음부터 재생하거나 cursor 0으로 되돌리지 않는다.
- 현재 파괴 상태 때문에 Flow Pattern이 자연스럽게 no-op, 실패 또는 reset-required가 되면 그 결과와 이유를
  그대로 보여 준다. Tool이 정상처럼 보이도록 지형을 복구하지 않는다.

## 2. 현재 코드·데이터 실측

### 2.1 선행 세션과 projection baseline

2026-08-27 실측 시 현재 worktree에는 다른 세션이 소유한 Effect loading responsiveness 변경이 남아 있다.
이 계획서 외에는 해당 변경을 건드리지 않는다. 구현 시작은 그 세션 PR이 merge된 뒤로 고정한다.

현재 HEAD에서 다음 명령은 terrain 3/9를 포함한 여러 requested output이 stale이라 실패한다.

```powershell
python Tools/ValtanPipeline/author_valtan_requested_effect_elements.py --mode Validate
```

따라서 선행 PR merge 후에도 먼저 같은 Validate를 다시 실행한다. 계속 실패하면 이번 반도넛 변경에 무관한
전체 output `Apply`를 몰래 섞지 않는다. 누락된 선행 projection 결과를 정본 변경 단위로 먼저 맞추거나,
선행 PR의 의도와 현재 output을 조사한 뒤 baseline을 확정한다.

### 2.2 실제 도넛 donor와 texture

`Data/Effects/Authored/effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01.effect.json`의
`donut.telegraph.outer.red`는 다음 특성을 가진 direct-authored floor particle이다.

- `kind = particle`, `sourceRecipe.enabled = false`
- `material.templateId = effect.standard`
- `renderProfile = alpha_two_sided_depth_read`
- Element transform position `[0, 0.28, 0]`, rotation `[-90, 0, 0]`, scale `[22, 22, 22]`
- particle `startSize = endSize = [0.75, 0.75]`
- `localSpace = true`, particle `billboard = false`, burst 1
- 현재 base/mask는 모두 `fx_c_ring_002.dds`

요청 texture는 이미 Git 관리 대상이다.

```text
Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_05/fx_m_ring_001_cl.dds
```

512x512 RGBA이며 새 resource 배포나 `.vcxproj/.filters` 등록이 필요하지 않다.

### 2.3 반으로 자르는 정확한 수치

현재 `Mesh Ring Fill`은 안쪽 반지름에서 바깥 반지름으로 채우는 radial thickness 기능이라 각도 기준 반원
절단이 아니다. `Linear Reveal`도 시간에 따라 전체로 복귀하므로 정적 반원으로 사용하지 않는다.

현재 particle quad는 local `+Y`가 texture `V=0`, local `-Y`가 `V=1`이다.
`CEffectDocumentRenderer`는 UV Sequence가 꺼져 있어도 `tileColumns/tileRows/tileIndex`를 적용한다. 따라서
도넛의 위쪽 절반을 찌그러뜨리지 않고 잘라 내는 첫 값은 다음과 같다.

```text
UV Sequence           false
UV Tile Columns       1
UV Tile Rows          2
UV Tile Index         0          // texture V 0.0 .. 0.5

기존 particle size    [0.75, 0.75]
반쪽 carrier size     [0.75, 0.375]
Initial Position Y    +0.1875
Position Min/Max      [0, +0.1875, 0]
```

일반식은 다음과 같다.

```text
halfSizeY = originalSizeY / 2
offsetY   = originalSizeY / 4 = halfSizeY / 2
```

즉 carrier 자체의 높이도 절반으로 줄이고 원래 도넛의 위쪽 절반 중심으로 옮기므로, 반쪽 texture를 다시
정사각형에 늘여 그리는 방식이 아니다. 크기 조절은 우선 Element root scale을 균일하게 조정하고,
particle size를 조정해야 할 때도 `sizeX == 2 * sizeY`, `positionY == sizeY / 2`를 유지한다.

### 2.4 현재 terrain Product 연결

다음 stable Product 연결은 이미 존재하므로 변경하지 않는다.

| Pattern | Stage | Effect asset ID | Cue ID |
|---|---|---|---|
| `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK` | `IMPACT` | `effect.valtan.project-tuned.terrain-destruction-3.semicircle` | `cue.valtan.requested.20260827.terrain-3.semicircle` |
| `VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK` | `IMPACT` | `effect.valtan.project-tuned.terrain-destruction-9.semicircle` | `cue.valtan.requested.20260827.terrain-9.semicircle` |

두 cue는 `sourceStartMs = 200`, boss root snapshot, once/natural, gameplay footprint
`[1.5, 1.5, 1.5]` 계약을 이미 사용한다. 이 변경은 cue나 실제
`worldeventset.valtan.terrain-destruction-*` 파괴 계약을 수정하지 않는다.

현재 projection script의 `_terrain_elements()`는 `fx_o_sector_04.dds` sector 세 장을 yaw
`-60/0/60`으로 겹쳐 반원을 흉내 내고, 9시에 +180도를 더한다. 이 세 장을 한 장의 진짜 반도넛으로
교체하는 것이 이번 effect delta다.

### 2.5 현재 Pattern Flow가 resetful인 이유

현재 구현은 사용자가 요구한 live insertion이 아니다.

- `CValtanPatternFlowService::Start()`는 Server가 “one arena reset”을 수행한다고 표시한다.
- `CGameRoom::Evaluate_ValtanPatternFlowStart()`는 owner player와 boss를 새 값으로 stage하고
  `Reset_ValtanAuditionState()`를 호출한다.
- 시작 commit은 player 위치/전투 상태, boss, world destruction, encounter prop, combat object를 fresh
  audition 상태로 바꾼다.
- `Finish_ValtanPatternFlow()`는 `bAutomaticPatternSequenceAuditionHold = true`로 Product를 멈춘다.
- owner 이탈의 `Abort_ValtanPatternFlowForOwner()`도 arena reset을 호출한다.
- wire lifecycle은 `PENDING -> ACTIVE -> COMPLETED_HOLD/STOPPED_HOLD` 의미다.

반면 기존 `CGameRoom::Update_WorldEntities()`는 이미 optional
`BOSS_PATTERN_SEQUENCE_DEFINITION*`을 `CValtanBrain::Update()`에 넘긴다. Brain은 stable pattern lookup,
ordered cursor, 성공 완료 후 cursor 전진, inter-step pursuit, 사망 pause/revive를 이미 소유한다. 따라서
새 Pattern runtime을 만들 필요는 없고, GameRoom이 안전 경계에서 sequence view와 scheduler cursor만
일시 교체하면 된다.

## 3. G00 — 선행 PR 뒤 시작 gate

1. 선행 세션 PR을 merge한 최신 `main`에서 `git status --short`, `git fetch`, 현재 HEAD를 기록한다.
2. `Tools/Network/Sync-TeamLanEndpoint.ps1`을 실행하고 출력 role만 RESULT에 기록한다. Client/UI는
   에이전트가 실행하지 않는다.
3. 새 `codex/valtan-half-donut-live-flow` 브랜치를 만든다.
4. 다음 baseline을 실행한다.

```powershell
python Tools/ValtanPipeline/author_valtan_requested_effect_elements.py --mode Validate
python Tools/EffectPipeline/test_valtan_requested_effect_elements.py
python Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py
```

5. 실패가 선행 PR의 미반영 output인지 이번 변경 전부터 존재한 별도 회귀인지 구분한다. baseline이 설명되지
   않은 상태에서 C++나 두 terrain JSON을 patch하지 않는다.
6. 구현 시작 시점의 `NETWORK_PROTOCOL_VERSION`, Product sequence ID/count, boss cursor 필드와 현재 Flow
   lifecycle enum을 다시 실측한다. 다른 PR이 protocol을 올렸으면 고정 숫자 40을 덮어쓰지 않고
   “then-current + 1”을 적용한다.

이번 계획의 실측 baseline은 protocol 39다. wire lifecycle 의미가 바뀌므로 현재 baseline이 유지되면 40으로
올린다.

## 4. G01 — sector 세 장을 한 장의 반도넛으로 교체

### 4.1 projection source

`Tools/ValtanPipeline/author_valtan_requested_effect_elements.py`에 다음 책임을 둔다.

- 기존 `DONOR_FIST_IN_OUT`과 donor key `fist`를 재사용한다.
- donor Element ID 상수 `donut.telegraph.outer.red`와 요청 texture 상수를 둔다.
- `_half_donut_element(...)`가 donor를 clone하고 아래 필드만 명시적으로 바꾼다.

```text
stable element ID
role/display label
base + mask = fx_m_ring_001_cl.dds
UV = 1 column, 2 rows, index 0, sequence false
particle start/end size = [0.75, 0.375]
particle initialPositionMin/Max = [0, 0.1875, 0]
yaw = baseYaw 또는 baseYaw + 180
start delay = 0
```

donor의 floor rotation, height, material/render profile, color, burst, local-space와 non-billboard 계약은
보존한다. `sourceRecipe.enabled`를 다시 켜거나 source SubUV가 authored crop을 덮게 하지 않는다.

새 stable row ID는 다음으로 고정한다.

```text
requested.20260827.terrain-3.semicircle.half-donut
requested.20260827.terrain-9.semicircle.half-donut
```

각 target의 generated closure는 다음 세 row다.

```text
half-donut 1
landing.01 1
landing.02 1
```

### 4.2 exact migration과 사용자 튜닝 보존

projection은 append-preserving이므로 새 반도넛을 추가만 하면 sector가 같이 보인다. 다음 obsolete ID만
transactional stage에서 exact match로 제거한다.

```text
requested.20260827.terrain-3.semicircle.sector-01..03
requested.20260827.terrain-9.semicircle.sector-01..03
```

prefix 전체 삭제나 target 전체 재생성은 금지한다. landing row와 사용자가 추가한 unrelated Element를
보존한다.

새 stable half-donut row는 최초 생성 뒤 기존 `_append_preserving()` 정책대로 사용자 소유가 된다.
Effect Tool에서 root scale, 바닥 높이, 공통 기준 yaw, 색을 튜닝한 뒤 재실행한 `Apply/Validate`가 이를
덮어쓰지 않아야 한다. focused test는 절대 크기 하나를 강제하기보다 다음 절단 불변식을 검증한다.

- base/mask가 모두 exact `_cl.dds` asset ID다.
- static `1 x 2 / index 0` crop이다.
- `startSize == endSize`, `sizeX == 2 * sizeY`다.
- position min/max가 같고 `positionY == sizeY / 2`, X/Z는 0이다.
- `sourceRecipe.enabled = false`, `localSpace = true`, particle `billboard = false`다.
- 3시와 9시의 yaw 차이는 modulo 360에서 정확히 180도다.
- old sector stable ID가 하나도 없다.
- landing 두 row의 기존 timing과 Product cue binding이 유지된다.

### 4.3 변경하지 않는 것

- `Data/Effects/EffectCatalog.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Data/Valtan/Valtan.presentation.json`
- 두 effect의 `.vcxproj/.filters` 등록
- `fx_m_ring_001_cl.dds` binary
- wall/floor destruction world-event data

asset/cue ID가 변하지 않으므로 위 파일에 의미 없는 churn이 생기면 원인을 조사하고 반도넛 커밋에서
제외한다.

## 5. G02 — Effect Tool에서 반으로 자르는 작업 가이드

### 5.1 구현 후 일반 튜닝

projection Apply가 끝난 뒤에는 donor를 다시 추가하지 않는다.

1. `F1 -> Effect Tool -> Data Files`에서 3시 또는 9시 target Effect를 선택한다.
2. `Load Saved Effect for Editing`으로 Current Effect를 연다.
3. Elements에서 `...semicircle.half-donut`을 선택한다.
4. Resource가 base/mask 모두
   `Effect/Valtan/Textures/FX_TEX_05/fx_m_ring_001_cl.dds`인지 확인한다.
5. `Effect Details -> UV`에서 Sequence off, Columns 1, Rows 2, Index 0을 확인한다.
6. particle Start/End Size가 2:1이고 Initial Position Min/Max Y가 반높이의 절반인지 확인한다.
7. 먼저 Element root scale을 균일 조정해 실제 발판 반지름을 맞춘다. z-fighting/부유 보정은
   `Element Transform Position Y`로만 하고, crop 중심을 소유한 `Particle Initial Position Y`는 독립적으로
   움직이지 않는다.
8. 9시 target은 3시 기준 yaw와 정확히 180도 차이를 유지한다.
9. `Solo` 또는 `Play All`로 Current Effect를 확인하고, 원하는 값이 정해진 뒤에만 `Save Changes`한다.

### 5.2 donor에서 절단값을 확인하는 scratch 가이드

`Load Saved Element for Editing`은 Product stable ID가 아니라 `authored.copy.*` 새 ID를 발급한다.
projection은 unrelated row를 보존하므로 이 복사본을 저장한 뒤 Apply하면 stable half-donut과 복사본이
동시에 보일 수 있다. 따라서 다음 절차는 값 확인·미리보기용 scratch 작업으로만 쓴다.

1. 먼저 target을 `Load Saved Effect for Editing`으로 Current Effect에 연다.
2. Data Files에서
   `effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01`을 펼친다.
3. `donut.telegraph.outer.red`를 선택하고 `Load Saved Element for Editing`을 누른다.
4. 복사된 Element의 base와 mask를 Resource Library의 `fx_m_ring_001_cl.dds`에 각각 `Bind Selected`한다.
5. 다음 값을 입력한다.

```text
Particle Billboard       off
Local Space              on
Element rotation X       -90 degrees
UV Sequence              off
UV Tile Columns          1
UV Tile Rows             2
UV Tile Index            0
Start Size               [0.75, 0.375]
End Size                 [0.75, 0.375]
Initial Position Min     [0, 0.1875, 0]
Initial Position Max     [0, 0.1875, 0]
```

6. 3시 값을 기준으로 9시는 yaw만 +180도 한다.
7. 필요한 root scale, Element Transform Position Y, 기준 yaw와 색 값을 기록한다.
8. `Save Changes`를 누르지 않고 target을 Reload해 `authored.copy.*` scratch row를 폐기한다.
9. projection `Apply`로 exact `...semicircle.half-donut` row를 만든다.
10. 생성된 stable row를 다시 열어 기록한 튜닝값만 옮기고 저장한다. old generated sector 세 row는
    projection migration이 제거하며 landing 두 row는 남긴다.
11. projection `Validate`를 실행해 stable ID와 사용자 튜닝이 재실행에도 보존되는지 확인한다.

수동 복사본의 자동 발급 ID를 Product stable ID처럼 임의 rename하거나 target에 저장하지 않는다. 최종
Product 저장은 projection이 소유한 exact half-donut ID와 한 row만 남긴 상태여야 한다.

## 6. G03 — Shared lifecycle을 Live Flow 의미로 교체

Flow authoring JSON과 Start/Stop packet payload는 그대로 쓸 수 있다. wire state의 의미는 바뀌므로
`Shared/Public/Network/PacketType.h`의 protocol을 then-current + 1로 올리고 codec/harness를 함께 바꾼다.

### 6.1 lifecycle

Client-local request 전송 전후의 짧은 상태만 `REQUEST_PENDING`이다. Server lifecycle은 다음처럼 분리한다.

```text
QUEUED_AFTER_CURRENT
  -> QUEUED_FOR_REVIVE -> QUEUED_AFTER_CURRENT
  -> ACTIVE <-> PAUSED_FOR_REVIVE
  -> BETWEEN_SLOTS -> ACTIVE ...
  -> COMPLETED_RESUMED

QUEUED_AFTER_CURRENT -> STOPPED_RESUMED       // 첫 slot 전 cancel
QUEUED_FOR_REVIVE    -> STOPPED_RESUMED       // revive 대기 중 cancel
ACTIVE/BETWEEN_SLOTS -> STOPPED_RESUMED       // current 완료 또는 gap에서 stop

any in-flight -> REJECTED | ABORTED
```

`QUEUED_FOR_REVIVE`는 첫 Flow slot이 시작되기 전에 파티는 남아 있지만 engageable player가 없는 상태다.
기존 wire `PENDING`, `COMPLETED_HOLD`, `STOPPED_HOLD`를 위 의미로 교체한다. unknown enum, truncated packet,
oversize reason/ID는 destination을 부분 mutation하지 않고 decode 실패한다.

`S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE`의 ordinal 의미도 고정한다.

- `QUEUED_AFTER_CURRENT`: 실행 예정 첫 slot
- `QUEUED_FOR_REVIVE`: 실행 예정 첫 slot, revive 대기
- `ACTIVE`: 실제 실행 중인 slot
- `BETWEEN_SLOTS`: 다음에 실행할 slot
- terminal: 마지막으로 완료했거나 취소된 slot

Start result의 `QUEUED`는 “Server가 reset을 끝냈다”가 아니라 “live insertion 요청을 검증하고 queue했다”는
뜻이다. Stop packet의 flow ID/room epoch/control sequence stale 방지는 그대로 유지한다.

### 6.2 packet과 harness 범위

수정 범위는 다음이다.

```text
Shared/Public/Network/PacketType.h
Shared/Public/Network/PacketMessages.h
Shared/Private/Network/PacketMessages.cpp
Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp
```

새 packet type이나 두 번째 control channel은 만들지 않는다. 1/32-slot, duplicate pattern 허용, duplicate
slot semantic reject와 Start/Stop identity 계약도 유지한다.

## 7. G04 — Client service와 Pattern Flow UI

### 7.1 `CValtanPatternFlowService`

`Client/Public/ValtanPatternFlowService.h`와
`Client/Private/ValtanPatternFlowService.cpp`의 snapshot state를 wire 의미와 맞춘다.

- `Is_InFlight()`는 `REQUEST_PENDING`, `QUEUED_AFTER_CURRENT`, `QUEUED_FOR_REVIVE`, `ACTIVE`,
  `BETWEEN_SLOTS`, `PAUSED_FOR_REVIVE`를 포함한다.
- `Is_TerminalHold()`은 제거하거나 `Is_Terminal()`로 바꾼다. completed/stopped는 hold가 아니다.
- Start 송신 뒤 raw result verdict만 bounded timeout을 적용한다.
- Server가 queue를 수락한 뒤에는 긴 Product Pattern, pursuit, 사망 pause 때문에 15초가 지나도 Client가
  임의 ABORT하지 않는다.
- world inbound generation 변경, disconnect, malformed/mismatched lifecycle은 기존처럼 fail closed한다.
- Stop result timeout은 Start verdict와 별도 one-shot clock을 유지한다.
- status에서 reset/hold 문구를 모두 제거한다.

대표 status는 다음 의미를 직접 보여 준다.

```text
Queued after the current Server pattern; boss, player and arena state are preserved.
Flow slot 2/5 is live.
Flow is between slots; the next slot is 3/5.
Flow completed; Product pattern playback resumed.
Queued Flow was cancelled; Product playback was not changed.
```

### 7.2 `Boss Tool -> Pattern Flow`

`Client/Private/BossTool.cpp`의 화면 action을 다음처럼 바꾼다.

| 현재 label | 변경 label | 의미 |
|---|---|---|
| `Start First` | `Play Flow` | 첫 slot부터 live queue |
| `Start Here` | `Play From Here` | 선택 slot부터 live queue |
| `Stop After Current` | queued일 때 `Cancel Queued Play` | Flow 시작 전 즉시 취소 |
| `Stop After Current` | active/between일 때 동일 | 현재 Flow occurrence까지만 실행 |

- slot list는 실제 occurrence에만 `[LIVE]`, 대기 첫 slot에 `[QUEUED]`, gap의 다음 slot에 `[NEXT]`를 표시한다.
- 상단 설명에 “현재 boss/player/arena state preserved; no reset”을 명시한다.
- `Preview Isolated` 아래에는 “fresh 단일 Pattern 진단을 위해 arena reset을 사용한다”는 반대 경계를
  명시한다.
- Flow가 queued/active인 동안 기존 단일 audition과 authoring mutation을 잠그는 현재 경계는 유지한다.
- Boss Tool은 packet을 직접 쓰지 않고 service snapshot만 소비한다.
- `Data/Encounters/Valtan/ValtanBossAuditionFlows.json` schema와 slot 저장 로직은 수정하지 않는다.

## 8. G05 — Server의 resetless sequence overlay

### 8.1 GameRoom state

`Server/Public/GameRoom.h`의 Flow phase를 다음처럼 바꾼다.

```text
INACTIVE
QUEUED_AFTER_CURRENT
ACTIVE
```

`BETWEEN_SLOTS`는 boss의 `bAutomaticPatternSequenceStepRunning == false`와 Flow cursor로 도출해 lifecycle로
표현하고, 별도 두 번째 runtime phase를 만들지 않는다.

Flow state에는 scheduler 복귀에 필요한 최소 context만 둔다.

```text
ProductResumeRevision
Product sequence/rotation stable ID
Product rotation step index
Product inter-step pursuit ticks
Product sequence expected count 또는 immutable identity 검증값
resume context captured 여부
owner detached / stop-after-current 상태
```

HP, 위치, phase, armor, cooldown, pattern sequence counter, world destruction, encounter prop, combat object,
player state의 복사본은 저장하지 않는다. 그것들은 복귀 대상이 아니라 현재 전투 정본이다.

### 8.2 Start는 validate + queue만 한다

`CGameRoom::Evaluate_ValtanPatternFlowStart()`는 다음만 먼저 validate/stage한다.

- Debug Server / Valtan Arena / exact boss placement / owner session과 player 존재
- flow/start slot/stable ID/count/pursuit 범위
- 모든 pattern이 pinned Flow gameplay revision에서 exact resolve
- 다른 single audition, timeline, Fight page, Flow와 충돌 없음
- boss가 살아 있고 room flow epoch/revision pin을 만들 수 있음
- owner가 `iCurrentHp > 0`, `isCombatReady`, action이 `DEAD/FALLING`이 아닌 현재 Brain의 engageable
  predicate를 만족함

성공 commit은 Flow metadata, suffix sequence, flow revision/pin, owner, epoch와
`QUEUED_AFTER_CURRENT` lifecycle만 저장한다. 다음 호출을 모두 제거한다.

```text
Prepare_TimelineAuditionPlayer
Place_PlayerAtValtanAuditionBait
Build_ValtanBossOnlyAuditionReset
Reset_ValtanAuditionState
owner combat-state replacement
combat-object/source cancellation
world destruction / encounter prop reset
```

Start 전후 boss/player/topology/prop/event sequence 상태가 byte-for-byte 동일해야 한다. 이미 일부 벽이나
바닥이 파괴됐다는 이유로 Start를 거부하지 않는다.

### 8.3 안전 경계 activation

새 `Prepare_ValtanPatternFlowBeforeBrain()`을 boss occurrence catalog resolve보다 먼저 호출한다. 이 위치는
중요하다. queued 상태에서 현재 Product occurrence는 기존 pin을 계속 써야 하고, ACTIVE로 바뀐 첫 tick은
Flow pin과 Flow sequence를 함께 써야 한다.

첫 Flow slot activation 조건은 다음을 모두 만족할 때다.

```text
boss alive
strPatternId.empty()
!bAutomaticPatternSequenceStepRunning
action is IDLE or CHASE
PendingPatternIds.empty()
!bMechanicLedgerRequiresReset
!bAutomaticPatternSequencePausedForRevive
iAutomaticPatternSequencePursuitTicksRemaining == 0
at least one player satisfies iCurrentHp > 0, isCombatReady,
  action != DEAD and action != FALLING
```

따라서 Play를 Product Pattern 도중 눌러도 현재 occurrence가 기존 Brain에서 정상 완료되고 Product cursor가
정확히 한 번 전진한다. 기존 Product pursuit countdown도 0까지 소비한 다음 activation한다.

activation은 그 시점의 Product resume revision/sequence ID/cursor/pursuit를 검증해 캡처한 뒤 scheduler
필드만 Flow 값으로 바꾼다.

파티가 비어 있지 않지만 위 engageable predicate를 만족하는 player가 없으면 Product pursuit/idle 중
`bAutomaticPatternSequencePausedForRevive`가 false여도 activation하지 않는다. Flow phase는 queued로 유지하고
`QUEUED_FOR_REVIVE`를 보낸다. revive 뒤 같은 predicate가 다시 true가 된 안전 경계에서만 활성화한다.

```text
strRotationId                                  = Flow sequence ID
iRotationStepIndex                             = 0
iAutomaticPatternSequenceInterStepPursuitTicks = Flow authored ticks
iAutomaticPatternSequencePursuitTicksRemaining = 0
bAutomaticPatternSequenceAuditionOverride      = false
bAutomaticPatternSequenceAuditionHold          = false
MovePath.clear()                                // 새 목표 replan만 유도
```

`Resolve_ValtanPatternFlowSequence()`와 `Resolve_ValtanGameplayCatalog()`은 ACTIVE에서만 Flow sequence/Flow pin을
반환한다. QUEUED에서는 null/current Product occurrence pin을 반환한다.

`Build_RequiredPinnedGameplayRevisions()`은 queue 수락 시점부터 Flow pin을, activation 뒤에는 Product resume
pin도 함께 보존한다.

queued lifecycle은 boss의 Product `iRotationStepIndex`를 절대 Flow slot index로 사용하지 않는다.

```text
QUEUED_AFTER_CURRENT / QUEUED_FOR_REVIVE
  slotIndex = flow.iStartSlotIndex

ACTIVE / BETWEEN_SLOTS
  slotIndex = flow.iStartSlotIndex + boss.iRotationStepIndex

terminal
  slotIndex = 마지막으로 실제 완료·취소한 Flow slot
```

`Play From Here`의 queued ordinal은 전체 저장 Flow 안의 원래 ordinal을 유지한다. 첫 Flow
`iPatternSequence` 예상값도 Product cursor로 미리 계산하지 않고 activation 시 현재 monotonic
`boss.iPatternSequence`에서 확정한다.

### 8.4 완료와 Product 복귀

마지막 Flow occurrence의 stage/world mutation commit까지 끝난 뒤 `Refresh_ValtanPatternFlowState()`가 복귀를
commit한다.

- saved Product revision과 현재 active revision/sequence identity가 같은지 확인한다.
- Product sequence ID와 next cursor를 복구한다.
- Product inter-step pursuit를 한 번 arm해 Flow 뒤에도 자연스러운 간격을 둔다.
- `bAutomaticPatternSequenceAuditionOverride/Hold`를 모두 false로 둔다.
- pause clock을 해제하고 `MovePath`를 clear해 다음 Product pursuit가 replan하게 한다.
- boss `PinnedDefinitionRevision`을 검증된 Product resume revision으로 바꾼다.
- lifecycle `COMPLETED_RESUMED` 또는 `STOPPED_RESUMED`를 보낸 뒤 Flow metadata/pin을 해제한다.

다음 값은 복구하지 않는다.

- Flow 중 감소한 boss/player HP
- player와 boss 위치
- armor/phase/cooldown/resource/stance
- 파괴된 벽·바닥과 encounter prop
- Flow가 만든 world event와 pattern sequence 증가

즉 “복귀”는 전투 rewind가 아니라 Product scheduler cursor의 반환이다.

### 8.5 Stop, death, leave와 실패

- queued + Stop: 첫 Flow slot을 한 번도 실행하지 않고 metadata만 지운다. 현재/다음 Product Pattern은
  그대로다.
- active + Stop: 현재 Flow occurrence는 끝까지 실행하고 그 stage/world mutation 뒤 Product로 복귀한다.
- between slots + Stop: 다음 Flow slot을 시작하지 않고 즉시 Product로 복귀한다.
- queued 중 player death: queue를 유지하고 `QUEUED_FOR_REVIVE`를 표시한다. active Product occurrence는 기존
  pause 계약으로 멈춘다. Product pursuit/idle에서 죽은 경우에도 별도 engageable predicate가 activation을
  막고, revive 뒤 안전 경계에서만 Flow가 시작된다.
- active 중 player death: 기존 ordered step의 stage/action/hit clock과 cursor를 그대로 pause하고 revive 뒤
  같은 Flow slot을 잇는다.
- owner leave, 다른 player 존재: queued면 metadata만 취소한다. active면 current occurrence drain 후 Product로
  복귀한다. arena reset은 하지 않는다.
- 마지막 player leave: 기존 room-empty cleanup이 room을 정리한다. 이를 Flow reset으로 중복 호출하지 않는다.
- boss death: ABORTED lifecycle 뒤 metadata/pin만 해제한다. boss를 revive하거나 Product를 재개하지 않는다.
- mechanic reset latch, sequence identity 손실, missing pin: exact reason으로 ABORTED하고 정상 Product resume
  경로를 호출하지 않는다. Flow `strRotationId`를 노출한 채 metadata만 지우면 다음 Product selector가
  sequence mismatch를 보고 cursor 0으로 재초기화할 수 있으므로, boss를 explicit reset-required quarantine에
  둔다. 이 실패 경로만 `bMechanicLedgerRequiresReset`과 기존 Debug audition failure hold를 사용해 selector를
  막고 `MovePath`를 비운다. scheduler cursor를 복구하거나 0으로 바꾸지 않으며 자동 reset도 하지 않는다.

missing Flow/occurrence pin은 일반 `Resolve_ValtanGameplayCatalog()` 실패로 흘려 room을 이유 없이
`m_isReady = false`로 만들기 전에 pre-Brain hook에서 잡는다. ABORTED lifecycle과 exact missing revision을
기록하고, 이후 tick은 active catalog를 실행용 fallback으로 쓰지 않는 quarantine hold에서 Brain/selector를
건너뛴다. 명시적 isolated audition/reset 또는 room-empty cleanup만 이 실패 hold를 해제할 수 있다.

Gameplay Hot Reload와 Product sequence migration은 이번 범위가 아니다. ACTIVE 동안 active Product revision이
바뀌면 cursor 0으로 조용히 재시작하거나 새 vector index에 매핑하지 않는다. immutable identity가 일치하지
않으면 fail closed하고 Debug reset이 필요하다는 이유를 표시한다.

### 8.6 `CValtanBrain` 보호 경계

현재 `CValtanBrain::SelectPattern()`과 `FinishPattern()`이 이미 ordered cursor, 성공 완료, pursuit,
pause/revive를 소유한다. 우선 구현은 `Server/Public/ValtanBrain.h`와
`Server/Private/ValtanBrain.cpp`를 수정하지 않는다.

실제 구현 중 GameRoom hook만으로 닫히지 않는 구체적 증거가 생길 때만 최소 public seam을 추가하고,
두 번째 selector/runner나 Flow 전용 Pattern 실행기를 만들지 않는다.

## 9. G06 — 수정 파일과 커밋 경계

### 9.1 커밋 1: half-donut projection/data

```text
Tools/ValtanPipeline/author_valtan_requested_effect_elements.py
Tools/EffectPipeline/test_valtan_requested_effect_elements.py
Data/Effects/Authored/effect.valtan.project-tuned.terrain-destruction-3.semicircle.effect.json
Data/Effects/Authored/effect.valtan.project-tuned.terrain-destruction-9.semicircle.effect.json
```

새 C++/DDS/effect asset ID가 없으므로 project/filter 변경은 없다.

### 9.2 커밋 2: resetless live Flow

```text
Shared/Public/Network/PacketType.h
Shared/Public/Network/PacketMessages.h
Shared/Private/Network/PacketMessages.cpp
Client/Public/ValtanPatternFlowService.h
Client/Private/ValtanPatternFlowService.cpp
Client/Private/BossTool.cpp
Client/Public/BossTool.h                                      // helper/state rename이 필요할 때만
Server/Public/GameRoom.h
Server/Private/GameRoom.cpp
Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp
Server/Private/ServerGameplayContractTests.cpp
Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py
.md/TEAM/보스툴.md
.md/GB/08-27/2026-08-27_VALTAN_WALL_BREAK_HALF_DONUT_AND_RESETLESS_LIVE_FLOW_RESULT.md
```

기존 packet type, Client service, GameRoom state를 확장하므로 새 C++ 파일과 project/filter 등록은 없다.
`Client/NetworkManager`, `ServerApp`, `RoomCommand`, `ValtanBrain`은 현재 호출 seam이 충분하면 수정하지 않는다.

두 커밋은 한 follow-up PR에 올리되 각각 독립 검증 가능하게 유지한다. 첫 커밋이 반도넛 Product data만,
둘째가 protocol/runtime/UI/contract와 최종 RESULT만 소유하게 한다.

## 10. G07 — 자동 검증

### 10.1 반도넛 projection/data

```powershell
python Tools/ValtanPipeline/author_valtan_requested_effect_elements.py --mode Apply
python Tools/ValtanPipeline/author_valtan_requested_effect_elements.py --mode Validate
python Tools/EffectPipeline/test_valtan_requested_effect_elements.py
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
python Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
python Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
python Tools/WorldPipeline/test_valtan_floor_destruction_transition_contract.py
```

focused contract는 다음 실패를 반드시 잡는다.

1. old sector ID 잔존 또는 half-donut 중복
2. base/mask 중 한쪽만 `_cl.dds`인 경우
3. `2 x 1`처럼 잘못된 축 crop 또는 sequence가 켜진 경우
4. carrier를 절반으로 줄이지 않아 늘어난 반원
5. position offset 누락으로 원래 원 중심이 이동한 경우
6. 3시/9시 yaw가 180도 반대가 아닌 경우
7. landing/cue/IMPACT/footprint/world-event drift
8. Apply 재실행이 기존 half-donut 사용자 튜닝을 덮는 경우

### 10.2 Shared/Client static contract

```powershell
python -m unittest `
  Tools.ValtanPipeline.test_valtan_pattern_tree_contract `
  Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract `
  Tools.ValtanPipeline.test_valtan_boss_tool_contract `
  Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract
```

기존 Flow test의 다음 reset/hold assertion은 삭제만 하지 않고 반대 불변식으로 교체한다.

- `Reset_ValtanAuditionState()` 호출 요구 -> Start state invariant와 reset 호출 부재
- “one arena reset” 문구 요구 -> “queued after current / state preserved” 문구
- `COMPLETED_HOLD/STOPPED_HOLD` -> `COMPLETED_RESUMED/STOPPED_RESUMED`
- accepted queue에 Start timeout 적용 -> raw verdict에만 timeout

### 10.3 protocol harness

- then-current + 1 protocol exact assertion
- 모든 새 lifecycle round trip
- unknown lifecycle atomic reject
- Start/Stop identity와 stale epoch
- 1/32 slot round trip, empty/33/truncated/oversize reject
- duplicate pattern ID 허용, duplicate slot ID Server semantic reject
- Release Server explicit rejection/no mutation

### 10.4 Server 실행 계약

`ServerGameplayContractTests.cpp`에 다음 resetless 시나리오를 실제 room fixed tick으로 추가하거나 기존 Flow
두 contract를 교체한다.

1. Product Pattern 중 Play를 queue해 Start 직후 boss/player/world/prop/event state가 완전히 동일하다.
   `Play Flow`와 `Play From Here`의 queued lifecycle은 각각 exact start slot ID와 저장 Flow 원래 ordinal을
   표시하며 Product `iRotationStepIndex` 값에 영향받지 않는다.
2. 현재 Product occurrence가 완료되고 Product cursor가 한 번만 전진하며, 기존 pursuit가 끝난 뒤 첫 Flow
   slot이 시작된다.
3. 이미 파괴된 벽·바닥/prop, 감소한 boss HP, 변경된 player 위치·HP·cooldown이 Start와 Flow 완료 뒤에도
   유지되고 Flow mutation만 추가된다.
4. Flow 자연 완료 뒤 captured Product sequence/cursor로 복귀하고 다음 Product Pattern이 replay/skip 없이
   시작된다.
5. queued cancel은 Flow occurrence 0회, Product cursor/현재 occurrence/다음 occurrence 무변경이다.
6. active stop은 current Flow slot 정확히 1회 완료, 다음 Flow slot 0회, Product resume다.
7. between-slot stop은 즉시 Product resume다.
8. queued death/revive와 active occurrence pause/revive가 같은 stage/cursor를 유지한다. Product occurrence가
   이미 끝난 pursuit/idle 중 사망한 경우도 `QUEUED_FOR_REVIVE`에서 첫 Flow slot을 시작하지 않는다.
9. Flow 시작 전후 `iPatternSequence`는 reset되지 않고 monotonic이다.
10. queued current Product occurrence는 원래 pin, ACTIVE는 Flow pin, terminal 다음 Product는 resume pin을 쓴다.
11. Flow pin과 Product resume pin이 required revision collection에 동시에 보존된다.
12. Product active revision/sequence drift는 cursor 0 fallback 없이 exact ABORTED/reset-required다. 그 뒤 여러
    tick에도 Product selector가 호출되지 않고 cursor가 그대로다. missing pin도 generic room failure 전에
    같은 quarantine과 exact lifecycle로 닫힌다.
13. owner leave + 다른 player는 arena reset 없이 drain/resume, 마지막 player leave는 기존 room cleanup만 쓴다.
14. boss death는 revive/reset/Product resume 없이 ABORTED다.
15. Flow가 없는 Product ordered sequence와 기존 single `Play Selected` resetful audition은 그대로 통과한다.

### 10.5 build/regression

```text
Shared + NetworkProtocolHarness x64 Debug/Release, harness 실행
Server x64 Debug/Release, Server.exe --contract-test
Client x64 Debug/Release
관련 JSON parse / project XML parse / git diff --check
Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

다른 프로세스가 Client/Server/PDB를 점유하면 사용자의 실행 상태를 확인하고 빌드를 조정한다. 에이전트가
Client/UI를 자율 실행·조작하지 않는다.

## 11. G08 — 사용자 수동 검증

### 11.1 반도넛

사용자가 Debug Server + Client에서 다음을 직접 판정한다.

1. `F1 -> Effect Tool`에서 3시 target의 half-donut만 Solo 재생한다.
2. 원형 비율이 찌그러지지 않고 직선 절단면이 정확히 반을 가르는지 확인한다.
3. root scale로 실제 벽 파괴 발판 반지름을 맞추고 바닥 z-fighting/부유가 없는지 확인한다.
4. 9시 target의 절단 방향이 3시와 정확히 반대인지 확인한다.
5. `F1 -> Boss Tool`에서 3시/9시 실제 Server Pattern을 각각 실행한다.
6. 반도넛이 실제로 남고 파괴되는 아레나 절반과 겹치고 landing layer가 유지되는지 확인한다.

### 11.2 Live Flow

1. Valtan Product 전투를 진행해 boss HP를 줄이고 player를 초기 위치 밖으로 이동한다.
2. 벽/바닥 또는 prop 하나가 이미 바뀐 상태를 만든다.
3. Product Pattern 재생 도중 `Pattern Flow -> Play Flow`를 누른다.
4. 현재 Pattern이 끊기지 않고 끝나는지, Play 순간 teleport/HP 회복/벽·바닥 복구가 전혀 없는지 확인한다.
5. 상태가 `[QUEUED] -> [LIVE]/[NEXT]`로 바뀌고 저장 slot 순서가 실제 Server Pattern으로 재생되는지 본다.
6. Flow 완료 뒤 `COMPLETED_RESUMED`가 표시되고 Product가 정확히 다음 cursor에서 이어지는지 확인한다.
7. 다시 queue한 직후 `Cancel Queued Play`를 눌러 Flow가 한 번도 시작되지 않는지 확인한다.
8. active slot 중 `Stop After Current`를 눌러 현재 slot만 끝나고 Product로 돌아오는지 확인한다.
9. player 사망 시 pause되고 기존 `Revive Player` 뒤 같은 occurrence를 이어 가는지 확인한다.
10. `Preview Isolated`는 별도로 fresh reset을 수행한다는 화면 설명과 실제 용도를 확인한다.

Animation/Effect의 최종 visual fidelity와 “눈으로 PASS”는 사용자의 서면 관찰이 있어야 RESULT에 기록한다.
에이전트는 화면 캡처나 visual PASS를 대신 만들지 않는다.

## 12. 위험과 대응

| 위험 | 대응 |
|---|---|
| UV 절단 축을 X로 잘못 잡아 좌우가 늘어남 | 실제 particle vertex/UV 계약대로 `1 x 2`, Y 반높이/Y 오프셋을 test로 고정 |
| append만 해서 기존 sector와 half-donut이 중첩 | obsolete six IDs만 exact retirement |
| generator가 수동 scale/yaw 튜닝을 되돌림 | stable row 존재 뒤 append-preserving, 절단 비율/180도 상대값만 검증 |
| Play가 현재 Product occurrence pin을 Flow pin으로 바꿈 | QUEUED에서는 Flow sequence/catalog resolve 금지, activation을 catalog resolve 전에 atomic 전환 |
| queued UI가 Product cursor를 Flow slot처럼 표시 | queued는 `iStartSlotIndex`, active/between만 Flow cursor를 사용 |
| Product pursuit/idle 중 전멸 뒤 Flow가 시작 | Brain pause flag와 별도로 exact engageable-player predicate를 activation gate로 사용 |
| Product cursor replay/skip | activation 시 next cursor 캡처, `FinishPattern()`의 기존 성공 전진을 단일 권위로 사용 |
| Flow 완료 뒤 IDLE hold | hold flag를 false로 하고 Product pursuit/cursor를 명시 복구 |
| active Hot Reload가 vector index를 바꿈 | exact revision/sequence identity 검사, 불일치 시 자동 remap/reset 없이 fail closed |
| ABORTED 뒤 Flow sequence ID가 Product cursor를 0으로 초기화 | reset-required quarantine이 selector를 막고 후속 tick cursor 무변경을 test |
| 기존 파괴 상태에서 Pattern이 성립하지 않음 | 이것이 live 검증 대상이다. 원문 실패 이유를 보여 주고 map 복구하지 않음 |
| 선행 PR 뒤 requested projection이 여전히 stale | G00에서 baseline 먼저 정리하고 unrelated broad Apply를 반도넛 커밋에 섞지 않음 |

## 13. 완료 조건

- 두 terrain Product Effect가 기존 sector 세 장 없이 각자 half-donut 한 장 + landing 두 장만 가진다.
- half-donut은 기존 donut donor와 `fx_m_ring_001_cl.dds` base/mask, static `1 x 2` UV crop을 사용한다.
- 3시/9시 cue, Pattern/Stage, gameplay footprint, world destruction 계약은 변하지 않는다.
- Pattern Flow Play는 현재 Product occurrence와 pursuit 뒤에 queue되고 Start 시 전투/맵 상태를 바꾸지 않는다.
- Flow 완료/Stop 뒤 Product next cursor가 replay/skip 없이 이어지며 terminal hold가 없다.
- queued cancel, death/revive, owner leave, boss death, pin/revision drift가 reset 없이 명시적으로 닫힌다.
- protocol, Client, Server contract와 Debug/Release build/regression이 통과한다.
- 구현 상태, 자동 검증, 사용자 visual 검증을 새 RESULT에서 분리한다.
- 사용자 육안 확인 전에는 반도넛 정렬이나 Live Flow 체감을 visual PASS로 기록하지 않는다.
