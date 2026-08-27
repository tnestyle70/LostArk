# 발탄 130 전멸기 도끼 회수 시퀀스 구현 계획

## 1. 목표

`VALTAN_FLOOR_WIPE_130`의 첫 지면 강타 뒤 빠져 있던 도끼 회수 동작을 복원한다.
역재생이나 신규 animation asset을 만들지 않고 action 420630 sequence 2에 포함된 정방향 회수 clip
`mesh_att_battle_5_04`와 후속 `15_*` chain을 사용한다. Server stage duration과 Effect cue stable ID는
바꾸지 않는다. 이어서 이 패턴이 130줄 강제 mechanic과 Debug stable-ID audition 양쪽에서 실제 Server stage
전체를 재생하고 완료되는 계약을 검증한다.

## 2. 현재 실측과 선택한 chain

- source cut은 `5_02_end`의 534ms에서 지면 강타를 끝내고 `5_04`로 넘어간다.
- source stage marker는 `5_04` 400ms, `15_03` 450ms지만 실제 clip 길이는 각각 500ms다.
  Product에서는 hard-cut runtime의 끝 포즈 연결과 고정 stage budget을 위해 두 clip을 500ms까지 재생한다.
- 실제 WModel에서 `5_04`는 이 구간 동안 도끼를 지면에서 머리 위로 들어 올린다.
- `5_04` 뒤 기존 `5_02_loop`로 복귀하면 hard cut 포즈 차이가 크다.
- `15_02` 끝과 `15_03` 시작은 같은 머리 위 포즈이며 `15_03`은 실제 두 번째 강타 notify를 가진다.
- 현재 runtime에는 clip-edge blend가 없어서 `5_02_end` 마지막 포즈와 `5_04` 첫 포즈 사이 약 0.37m의
  major-skeleton 차이는 남는다. 회수 clip 누락은 복원하되 이 첫 경계는 사용자 육안 검토 대상으로 둔다.

따라서 Product chain은 다음으로 고정한다.

```text
WINDUP       1800ms  5_02_loop 반복
FIRST_SMASH   800ms  5_02_end 534ms + 마지막 지면 포즈 유지
INTERVAL     2000ms  5_04 500ms -> 15_02 1000ms + 마지막 머리 위 포즈 유지
SECOND_SMASH  500ms  15_03 500ms exact
RECOVERY     1500ms  15_04 반복
```

## 3. 변경 파일

- `Data/Valtan/Valtan.presentation.json`: animation occurrence 정본과 stage end policy를 수정한다.
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`: V2 projector가 runtime animation binding으로
  재생성한다. retired V1 master인 `Data/Valtan/Valtan.pattern.json`은 읽거나 수정하지 않는다.
- `Data/Animation/RootMotion/Valtan.rootmotion.json`: 변경된 explicit multi-clip binding에서 Server가 소비할
  stage root-motion curve를 재생성한다.
- `Tools/ValtanActionExtractor/build_valtan_rootmotion.py`: explicit finite multi-clip chain이 중간에 이동한 뒤
  원점으로 돌아오더라도 sampled 최대 이동이 threshold를 넘으면 curve를 보존한다. single/natural 경로의 기존
  final-displacement 판정은 바꾸지 않는다.
- `Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py`: clip 순서, source window와 hold/exact
  계약을 고정한다.
- `Tools/ValtanActionExtractor/test_build_valtan_rootmotion.py`: explicit multi-clip migration 허용 목록에
  FLOOR_WIPE `INTERVAL`을 고정한다.
- `Server/Private/ServerGameplayContractTests.cpp`: `Play Server Pattern`과 같은 stable-ID 요청으로
  WINDUP부터 RECOVERY 완료까지 product room tick을 실행하는 회귀 계약을 추가한다.
- `.md/TEAM/발탄인수인계서.md`: 애니메이션 담당 public handoff의 실제 Product chain을 갱신한다.

신규 C++ 파일과 project/filter 등록은 없다.

## 4. 검증

1. `Project-ValtanPatternMaster.ps1 -Mode PublishV2`, 이어서 `-Mode ValidateV2`
2. `build_valtan_rootmotion.py`, 이어서 `--check`와 explicit-chain admission focused root-motion unit
3. Animation Tool과 Valtan V2 focused harness
4. Valtan tuning runtime-set 및 gameplay publisher Validate
5. JSON parse와 `git diff --check`
6. gameplay Publish 뒤 실행 중인 표준 Server를 건드리지 않고 isolated Debug에서 새 FLOOR_WIPE room-tick 계약,
   isolated Release에서 전체 `Server.exe --contract-test`를 검증한다.
7. 사용자가 Animation Tool 또는 `F1 > Balance Tool > Gimmicks > 6방향 후 전멸 패턴 > Play Server Pattern`으로
   첫 강타, 회수, 머리 위 유지, 두 번째 강타의
   실제 연결감을 확인한다. 자동 검증은 visual PASS를 대신하지 않는다.

## 5. Server 선택 계약

`VALTAN_FLOOR_WIPE_130`은 일반 weighted selection set에 넣지 않는다. `HEALTH_BAR_CROSSING 130`,
`oncePerEncounter: true`, `triggerOrder: 1`인 forced mechanic이며 pending mechanic이 130~109 일반 window보다
먼저 선택된다. 같은 패턴을 weighted rotation에도 추가하면 130줄 이후 랜덤 재등장할 수 있으므로 잘못된 중복이다.

자동 전투에서는 131줄 초과에서 130줄 이하로 내려갈 때 한 번 queue한다. Debug에서는 기존 stable-ID audition이
같은 Server pattern을 boss reset 뒤 직접 queue하므로, 데이터의 trigger 의미를 바꾸지 않고 반복 검토할 수 있다.
