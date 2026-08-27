# Animation Tool 발탄 presentation 동일 재생 결과

## 1. 완료 상태

Animation Tool의 Valtan Pattern Master 재생을 `CModel` 직접 단일-occurrence sampler에서
`CValtan` Product presentation sampler 호출로 교체했다.

- Arena replicated Valtan의 `Apply_NetworkState`와 local preview가
  `CValtan::Apply_PatternPresentationSample`을 함께 사용한다.
- 공용 함수가 `Valtan.patternbindings.json`의 action binding을 resolve하고 model native
  duration, `sourceStartMs`, `playMs`, `playRate`, loop, occurrence transition을 동일하게
  적용한다.
- Animation Tool은 split authoring에서 검증한 stage/action wall age만 local preview boss에
  전달한다. 모델 clip을 직접 선택하거나 별도 sampler를 소유하지 않는다.
- 버튼은 `Play Arena Presentation Locally`로 바꿔 실제 Server/world 실행과 구분했다.
- local 재생은 network, Effect, Sound, hit/damage, navigation과 Server transform을 실행하지
  않는다. 재생 종료 시 idle과 local occurrence 상태를 초기화한다.

## 2. 변경 파일

- `Client/Private/Valtan.cpp`, `Client/Public/Valtan.h`
  - Server와 local authoring이 공유하는 Product pattern presentation sampler
- `Client/Private/Animation_Tool.cpp`, `Client/Public/Animation_Tool.h`
  - 선택 stage wall clock을 local `CValtan` 공용 sampler에 제출
- `Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py`
  - 두 호출자가 같은 sampler를 쓰고 Tool의 직접 sampler가 제거됐음을 검증
- 대응 PLAN/RESULT

## 3. 자동 검증

- `test_animation_tool_valtan_pattern_master.py`: 8/8 PASS
- `test_valtan_pattern_tree_contract.py`: 17/17 PASS
- `test_valtan_model_view_composition.py`: 9/9 PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
  - managed 27, legacy 26, world members 30, errors 0
- `ActionPresentationTimelineHarness` Debug build/run: PASS
- Client x64 Debug/Release build: PASS
  - `Client/Bin/Debug/Client.exe`, `Client/Bin/Release/Client.exe` 생성
  - 기존 CP949 C4819, DirectXTK PDB와 Release Shared PDB 경고만 발생,
    compile/link error 0
- scoped `git diff --check`: PASS

## 4. 사용자 육안 검증 대기

최종 visual PASS는 아직 기록하지 않는다. 다음 순서로 사용자가 확인한다.

1. Debug Server + Client 실행 후 Lobby -> Character Select
2. F1 -> Animation Tool -> Target `Valtan`
3. Valtan Pattern Master의 Pattern 선택, Preview speed `1.00x`
4. `Play Arena Presentation Locally`

우선 확인 패턴은 다음과 같다.

- `VALTAN_DASH_CHARGE`, Normal
  - WINDUP의 동일 clip 세 source segment, CHARGE의 `sourceStartMs=2450`, `0.6x`,
    RECOVERY의 `1.5366667x` 전환을 확인한다.
  - 20m 전진은 Server motion 권위라 이 animation-only preview의 합격 항목이 아니다.
- `VALTAN_FLOOR_WIPE_130`
  - loop WINDUP, FIRST_SMASH의 534ms 뒤 hold, INTERVAL의 두 순차 occurrence,
    SECOND_SMASH와 loop RECOVERY 전환을 확인한다.
- `VALTAN_SEQUENCE_JUMP_WHIRLWIND_ROAR_ROAR_CHARGE`
  - P2 animation-first 14 stage의 긴 chain과 1333/1467/4433/1067ms 경계를 확인한다.
- `VALTAN_SEQUENCE_WARP`
  - 같은 `mesh_att_battle_18_02`가 STEP_02~09 각 stage 시작마다 다시 시작하고
    마지막 `mesh_att_battle_18_03-2`로 전환되는지 확인한다.

clip 순서, source 시작점, 속도, loop/hold, 같은 clip occurrence restart가 모두 맞으면
animation presentation 작업자는 이 경로를 Arena runtime과 같은 기준으로 사용할 수 있다.
실제 boss world 이동, 벽 충돌, 피해와 Effect는 Server Arena audition의 별도 검증 항목이다.
