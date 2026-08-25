# Valtan Presentation Scale · HIGH_JUMP · Independent Preview 통합 결과

## 1. 구현 상태

현재 integration worktree에서 다음 계약이 코드와 데이터에 반영되어 있다.

- `BossCatalog.json` format v4의 Valtan `presentationScale`은 `0.75`이며 replicated spawn과
  Character/Boss Preview가 같은 값을 소비한다.
- `Valtan.presentation.json`의 managed cue는 15개이고 policy 분포는
  `OWNER_RELATIVE 4 / GAMEPLAY_FOOTPRINT 9 / ARENA_ABSOLUTE 2`다.
- HIGH_JUMP는 `TAKEOFF EXACT 1933 -> AIRBORNE LOOP -> LAND EXACT 3200 ->
  RECOVERY mesh_idle_battle_1 EXACT 400`이며 gameplay travel stage는 `LAND`다. AIRBORNE은 정점 hold다.
- cue join은 nullable `sourceEndMs`의 존재 여부를 먼저 비교한다.
- Effect Tool의 Valtan Product preview와 420633 source-bone history는 Arena runtime과 같은 cue scale-policy
  owner-basis 합성 함수를 사용한다. 420633 V1 별칭의 visible follow carrier 5개도 `b_effectroot`의 `0.01`
  import scale을 공용 source-bone builder로 제거한 뒤 effective owner root에 붙으므로 actor `0.75`를 다시
  상속하거나 `0.015`로 축소되지 않는다. V0/V1은 명시적인 typed identity predicate로 같은 420633
  exact-history binding에 join하며 Complete, Sample Time, loop/restart, Restore, Particle/Element audition은
  공용 exact-history seek를 사용하고 binding이 없으면 bare current-pose 재생으로 우회하지 않는다.
- Client animation presentation은 occurrence index와 local clock을 사용해 동일 AIRBORNE snapshot의 반복 수신으로
  loop가 매 tick 재시작되는 것을 막는다.
- combat-object 독립 preview는 staged Valtan의 rotation/translation만 snapshot하고 actor scale을 제거한 fixed
  world root에서 Effect local `0ms`로 시작한다.
- Effect Workbench/Arena의 Rendering 정본과 runtime projection은 exact `bloomScatter: 1.0`이다.
- BossCatalog이 참조하는 body, Parts1/Parts2, AnimSet, weapon 다섯 WModel과 그 material table의 TGA 12개/DDS
  8개는 pull-only 최소 dependency closure로 Git/LFS에 포함한다. 팀원이 별도 발탄 추출본을 복사할 필요가 없다.

## 2. 소유권과 제외 상태

이 통합은 Valtan 다섯 split source와 generated Product의 strict join, actor scale, cue transform policy,
animation/preview consumer만 소유한다. sky-axe의 authored mesh/particle/decal geometry와 속도·회전은 Effect V1
저작 데이터 소유이며 core가 `0.75`나 다른 숫자를 강제하지 않는다. Profiler, performance/reaper/navigation,
DimensionMaster BA는 포함하지 않는다.

## 3. 자동 검증 상태

정본 build entrypoint를 실제 실행해 다음을 확인했다.

- `Invoke-BuildAndRegression.ps1 -Configuration Release -ResourceRoot C:\Users\user\Desktop\LostArk\Client\Bin\Resources`:
  Engine, UpdateLib, Shared, Server, Client build/link와 전체 regression PASS
- Debug Engine/Shared/Server/Client build/link PASS. Debug full regression의 유일한 미완료는 사용자가 실행 중인
  physical Debug Server가 runtime mutex를 소유해 두 번째 contract process를 열 수 없었던 환경 조건이다.
- Valtan tuning runtime set 37/37, Balance Tool 22/22, pattern tree 16/16, Animation Tool 7/7,
  Effect Tool saved rows 29/29 PASS
- gameplay publisher Validate: 6 player profiles, 230 skills, 109 damage profiles, 33 boss patterns,
  131 stages, 52 audition rows PASS
- Effect direct source validation: 175 documents, 1,025 direct dependency resources,
  generated artifact 0; Effect project sync 2,349 files / 219 filters PASS
- Valtan model-view composition과 material dependency closure 9/9, Rendering authored/runtime/float32 gate 3/3 PASS
- 최종 cue scale-policy Tool/runtime 공용 helper 반영 뒤 Client Debug/Release 재컴파일·링크 오류 0
- Release Server contract, Valtan four-player live harness, Character Select isolation live harness failure 0

Client/UI는 에이전트가 실행하거나 조작하지 않았으므로 아래 visual fidelity 항목은 자동 PASS가 아니다.

## 4. 사용자 수동 검증 경계

Client/UI는 에이전트가 실행하거나 조작하지 않았고 visual PASS를 선언하지 않는다. 자동 검증과 빌드가 끝난 뒤
사용자가 직접 다음을 확인한다.

1. Character/Boss Preview와 Valtan Arena의 보스 크기가 같은 `0.75` presentation을 사용한다.
2. `All Effects -> Valtan -> VALTAN_HIGH_JUMP -> Play All`이 AIRBORNE에서 계속 loop한 뒤 LAND와 idle recovery로 간다.
3. `INDEPENDENT EFFECT -> target axe -> Play Independent Effect`가 현재 Valtan 위치/회전을 기준으로 즉시 보인다.
4. body-attached cue만 보스와 함께 줄고 footprint/arena cue는 authored world 크기를 유지한다.
5. sky-axe 크기, 이동 속도, 회전, decal 방향은 Effect Tool 저작값대로 보이는지 별도로 판정한다.
