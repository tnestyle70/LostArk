# Valtan Presentation Scale · HIGH_JUMP · Independent Preview 통합 계획

## 1. 현재 실제 반영 상태와 경계

Effect V1 정본 위에 발탄 actor presentation을 기존 1.5의 절반인 `0.75`로 내리고, 바닥 공격 Effect의
world footprint는 보스 크기와 분리한다. 같은 통합 단위에서 HIGH_JUMP의 animation occurrence/clock 회귀와
`SERVER_COMBAT_OBJECT` 독립 preview의 world-root 계약을 닫는다.

이 변경은 Effect authored element를 일괄 축소하거나 sky-axe geometry 값을 추측해 덮어쓰지 않는다.
`Data/Effects/Authored/effect.valtan.sky-axe.active.effect.json`의 실제 mesh/particle/decal 속성은 Effect V1
저작 데이터와 사용자의 수동 튜닝이 계속 소유한다.

## 2. 정본과 생성물

| 구분 | C:/Users/user/Desktop/LostArk 기준 경로 | 역할 |
|---|---|---|
| 정본 | `Data/Valtan/Valtan.gameplay.json` | HIGH_JUMP `serverMotion.travelStageId: LAND` |
| 정본 | `Data/Valtan/Valtan.presentation.json` | HIGH_JUMP occurrence와 managed cue `scalePolicy` |
| 정본 | `Data/Valtan/Valtan.combatobjects.json` | combat-object geometry/damage |
| 정본 | `Data/Valtan/Valtan.worldeventsets.json` | stable world-event membership |
| 정본 | `Data/Valtan/Valtan.legacy-compatibility.json` | unmanaged migration closure |
| 정본 | `Data/Actors/BossCatalog.json` | format v4 Valtan `presentationScale: 0.75` |
| 정본 | `Data/Rendering/Authored/RenderingProfiles.json` | Effect Workbench/Arena 공용 `bloomScatter: 1.0` |
| 생성물 | `Data/Encounters/Valtan/*`, `Data/Animation/Authored/Valtan/Valtan.pattern*.json`, Server bootstrap | publisher가 strict join한 read-only Product |

`Data/Valtan/Valtan.pattern.json`은 migration fixture이며 이번 값을 저장하지 않는다. split source
`formatVersion: 1`, Encounter product v4, pattern effect cue product v3, Server gameplay bootstrap v19를 같은
version 번호로 해석하지 않는다.

## 3. 구현 계약

1. replicated Arena와 Character/Boss Preview는 모두 `BossCatalog.presentationScale`을 소비한다.
   `BossProfiles.collisionRadius`와 Server hit geometry는 바꾸지 않는다.
2. managed cue 15개는 `OWNER_RELATIVE` 4, `GAMEPLAY_FOOTPRINT` 9, `ARENA_ABSOLUTE` 2다.
   owner-relative만 actor scale을 상속하고 나머지는 owner scale을 authored `worldScale`로 교체한다.
   Arena runtime과 Effect Tool authoring timeline은 같은 validated owner-basis 합성 함수를 사용하며, source-bone
   history도 같은 effective owner root를 사용한다.
3. HIGH_JUMP는 TAKEOFF `EXACT 1933ms`, AIRBORNE `LOOP_TO_STAGE_END`, LAND `EXACT 3200ms`,
   RECOVERY `mesh_idle_battle_1 EXACT 400ms`다. Server travel stage는 LAND이며 AIRBORNE은 정점 hold다.
4. Client는 occurrence index와 local presentation clock으로 동일 snapshot action의 반복 수신을 구분한다.
   AIRBORNE loop를 매 snapshot마다 재시작하지 않되, 같은 model animation index를 쓰는 다른 occurrence는 전이한다.
5. nullable `sourceEndMs`는 has-value를 먼저 비교하고 값이 있을 때만 숫자를 비교한다.
6. combat-object 독립 preview는 현재 staged Valtan root의 translation/rotation을 한 번 snapshot하고 actor scale을
   제거한다. fixed world root에서 Effect local `0ms`로 시작하며 pattern TAKEOFF offset을 흉내 내지 않는다.
7. BossCatalog이 직접 참조하는 body, Parts1/Parts2, AnimSet, weapon 다섯 WModel과 material table이 참조하는
   TGA 12개/DDS 8개는 pull-only 최소 dependency closure로 Git/LFS에 포함한다. 전체 Resources pack이나 미참조
   추출본은 포함하지 않는다.
8. Rendering authored/runtime는 exact `bloomScatter: 1.0` identity를 유지하며 publisher와 focused test가
   float32 경계 및 idempotence를 검증한다.

## 4. 검증 계획

- 모든 변경 JSON parse와 `git diff --check`
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`
- `test_valtan_pattern_master_v2.py`, `test_valtan_pattern_tree_contract.py`
- `test_effect_tool_valtan_saved_rows.py`, focused Valtan model-view composition test
- Rendering publisher Validate와 `test_publish_rendering_profiles.py`
- `ActionPresentationTimelineHarness` Debug/Release
- Client x64 Debug/Release build

자동 검증은 구조와 실행 준비만 증명한다. 사용자가 직접 `All Effects -> Valtan`에서 HIGH_JUMP 전체 재생,
독립 도끼, owner-relative/footprint/arena cue, Arena의 실제 보스 크기와 위치를 확인하기 전에는 visual PASS가 아니다.

## 5. 명시적 제외

- Effect V1 authored sky-axe element geometry를 `0.75`로 강제하는 변환
- Profiler, Server performance/reaper/navigation, DimensionMaster BA
- 카메라·벽·도끼의 최종 visual fidelity 자동 판정
