# Valtan 마력구 전멸 타이밍 정렬과 Sound 드래그 abort 수정 계획

기준일: 2026-09-04

기준 브랜치: `GB/KoukuSaydon-pattern1`

## 0. 목표와 현재 실측

사용자가 Boss Tool의 `마력구 파괴 패턴(VALTAN_STAGGER_SLOT)`에서 Effect V2 두 occurrence를
`FINAL_ATTACK` 로컬 `1000 ms`로 옮긴 화면을 기준으로 Sound와 Server 권위 전멸 판정을 같은
시각으로 정렬한다. 화면의 초록색 `G_Voltan2_Attack25_Shot2 [once]` box는 Pattern Sound cue다.

현재 Git 정본은 Effect V2, Sound, Server hit가 모두 `2900 ms`이고, 화면의 Effect V2 이동은
Client가 abort되어 저장되지 않았다. 따라서 세 정본을 한 변경 단위에서 `1000 ms`로 맞춘다.

Sound body drag의 abort는 FMOD 재생 문제가 아니다. `Patch_ValtanCompositionPatternSound()`가
현재 Sound document 안의 `occurrenceId` 문자열을 참조로 받은 뒤 staged document를 commit해
원본 vector/string을 교체하고, 이미 무효화된 참조로 성공 상태 문구를 만드는 수명 오류다.

## G00. 변경 전 경계 보존

### 직접 소비자와 데이터 흐름

```text
Data/Valtan/Valtan.gameplay.json
  -> Project-ValtanPatternMaster.ps1
  -> Data/Encounters/Valtan/ValtanEncounter.json
  -> Publish-GameplayBalance.ps1
  -> Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap
  -> Server fixed tick damage

Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
  -> Validate-EffectV2Runtime.ps1 / Publish-Compositions.ps1
  -> Client Effect V2 presentation

Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json
  -> Publish-Compositions.ps1
  -> Client/Bin/DataFiles/Compositions/Bosses/Valtan.bosscomposition.json
  -> Pattern Sound runtime
```

`Client/Bin/DataFiles`와 `Server/Bin/DataFiles` 생성물은 직접 편집하지 않고 publisher로만
갱신한다. 기존 dirty worktree의 Kakul/Map/Composition 변경은 되돌리거나 정리하지 않는다.

## G01. Sound 드래그 commit 수명 수정

### `Client/Private/Animation_Tool.cpp`

`CAnimation_Tool::Patch_ValtanCompositionPatternSound()` 진입 시 내부 document를 가리킬 수 있는
`strOccurrenceId`와 `strSoundEvent`를 값으로 snapshot한다. 검색, 후보 수정, commit 뒤 status는
그 stable copy만 사용한다.

한 줄 책임: owner 교체 이후 호출자 소유 문자열 참조를 다시 읽지 않는다.

### `Tools/ValtanPipeline/test_action_composition_sound_owner_contract.py`

Sound patch가 staged document commit 전에 alias 가능 입력을 snapshot하고, commit 뒤에는 stable
copy로 status를 만드는 source contract를 추가한다. 기존 parse/validate/stage/commit 및 occurrence
qualified 계약도 유지한다.

## G02. 마력구 실패 타이밍 정렬

다음 현재 정본과 재생성 helper/validator 기대값을 `FINAL_ATTACK` 로컬 `1000 ms`로 맞춘다.

```text
Data/Valtan/Valtan.gameplay.json
  hit.schedule.firstOffsetMs = 1000

Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
  boss.valtan.twohand startMs = 1000
  boss.valtan.six.sonic startMs = 1000

Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json
  G_Voltan2_Attack25_Shot2 startMs = 1000
```

Sound binding/occurrence ID의 기존 `impact-2900` 문자열은 저장 identity이므로 이름만 바꾸지 않는다.
수치와 identity를 동시에 바꾸면 기존 selection/외부 참조를 불필요하게 깨뜨리기 때문이다.

`author_valtan_phase_two_mechanics.py`, `build_valtan_pattern_sound_cues.py`, coverage/alignment test도
같은 `1000 ms` 계약으로 갱신해 다음 재생성이 2900으로 되돌리지 못하게 한다.

## G03. Server fixed-tick 회귀

`ServerGameplayContractTests.cpp`의 실패 branch는 `FINAL_ATTACK` 진입 뒤 1000 ms 직전에는 damage가
없고, 1000 ms 시점에는 두 player에게 wipe damage가 한 번씩 발생하는 것을 검증한다. Stage 전체
3000 ms와 종료/vertical offset 복원 계약은 유지한다.

## G04. Publish와 검증

### 집중 검증

```powershell
python -m unittest `
  Tools.ValtanPipeline.test_action_composition_sound_owner_contract `
  Tools.ValtanPipeline.test_valtan_pattern_sound_cue_contract `
  Tools.ValtanPipeline.test_valtan_hit_presentation_alignment `
  Tools.ValtanPipeline.test_valtan_status_pattern_contract

powershell -ExecutionPolicy Bypass -File `
  Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode Validate

powershell -ExecutionPolicy Bypass -File `
  Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2
```

publisher가 생성물을 갱신한 뒤 JSON parse와 `git diff --check`를 확인한다.

### 정본 회귀

Server gameplay 의미와 Client C++를 함께 바꾸므로 publisher만으로 완료하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File `
  Tools/Build/Run-FullPipeline.ps1 `
  -Configuration Debug -Profile FullDiagnostic
```

반복 저작 중에는 focused validator/publisher만 사용하고, 최종 저장 묶음에서 FullDiagnostic을 한 번
실행한다. fingerprint cache가 같은 domain의 불필요한 재실행을 줄인다.

## G05. 사용자 수동 smoke

에이전트는 Client/UI를 실행하거나 화면·사운드를 판정하지 않는다. 자동 검증 뒤 사용자가
`Server + Client` profile을 직접 실행해 다음을 확인한다.

```text
Lobby -> Valtan -> F1 -> Boss Tool -> 마력구 파괴 패턴
FINAL_ATTACK 로컬 1000 ms(전체 timeline 13000 ms)에
Effect V2 두 occurrence, G_Voltan2_Attack25_Shot2, Server wipe가 일치
값을 다시 편집하지 않고 패턴 재생만으로 timing 확인
별도 회귀 확인 시에만 Sound box body drag와 Save 후 abort가 재발하지 않음
```

육안/청각 timing과 실제 UI drag 재현은 사용자 판정 전까지 수동 미검증으로 남긴다.
