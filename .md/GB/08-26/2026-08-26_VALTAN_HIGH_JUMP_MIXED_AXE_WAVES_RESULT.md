# 발탄 HIGH_JUMP 혼합 도끼 3웨이브 구현 결과

## 1. 완료한 Server 실행 계약

`VALTAN_HIGH_JUMP/AIRBORNE`를 한 번의 `4000ms` stage로 유지하면서 stage 시작 기준
`0ms`, 약 `1333ms`, 약 `2667ms`에 총 세 웨이브를 생성하도록 구현했다. 30Hz fixed tick에서는
각각 stage start의 `0/40/80` tick이다.

각 웨이브는 그 시점의 생존·전투 가능 플레이어를 다시 조회해 플레이어마다 추적 도끼 1개를
만들고, 같은 transaction에서 아레나 랜덤 도끼 4개를 추가한다. 플레이어가 웨이브 사이에
사망하거나 전투 불가능 상태가 되면 다음 웨이브의 추적 도끼 수에 반영되며, 랜덤 도끼 4개는
플레이어가 없어도 생성된다.

아레나 랜덤 도끼의 중심은 발탄의 현재 이동 좌표가 아니라 navigation projection 뒤 보존한
저작 `boss spawn position`이다. 후보는 중심 반경 `14m`, 중심과 높이 차 `1m` 이하,
정확한 중심점 walkable 조건을 통과하고 네 점끼리 최소 `7m` 떨어진다. pattern sequence,
action identity와 wave ordinal에 기반한 deterministic hash를 사용하므로 같은 입력은 같은 좌표를
재현하며 process RNG와 wall clock에는 의존하지 않는다.

플레이어 도끼는 기존처럼 첫 `1200ms` pulse까지 대상 좌표를 추적한 뒤 고정된다. 아레나 도끼는
가짜 player target 없이 처음부터 고정 world pose를 사용한다. 한 웨이브의 추적 묶음과 랜덤 묶음은
용량, navigation, spacing 또는 catalog join 중 하나라도 실패하면 모두 rollback하며 wave counter도
실제 commit 뒤에만 증가한다.

## 2. 데이터·도구 연결

- Source 정본은 `spawnSchedule { INTERVAL, count: 3, firstOffsetMs: 0, intervalMs: 1333 }`와
  `arenaRandom { RANDOM_NAVIGABLE_CIRCLE, BOSS_SPAWN_POSITION, count: 4, radiusM: 14,
  heightToleranceM: 1 }`를 소유한다.
- Product에는 schedule/random 여섯 필드를 flat action field로 투영하고 Server bootstrap version을
  `20`으로 올렸다. combat object 수명과 AIRBORNE duration은 모두 `4000ms`, 한 웨이브의
  최대 수용량은 `36`이다.
- gameplay projector, publisher, Server catalog/runtime, Client pattern/reference parser와 Balance Tool의
  save/apply 보존 경로를 같은 계약으로 갱신했다.
- 기존 radial draft candidate와 Product-only row의 편집 가능 범위는 유지하고 현재 packaged
  HIGH_JUMP row에만 요청 수치를 strict admission한다.
- 사용자가 편집 중인 `effect.valtan.sky-axe.active.effect.json`의 element 값은 덮어쓰지 않았다.

## 3. 자동 검증

- Valtan official projection `PublishV2`: PASS, 7개 artifact 생성·검증
- Valtan tuning runtime set Validate: PASS
- gameplay balance publisher Validate: PASS
- Valtan pattern tree focused test: 17/17 PASS
- Balance Tool contract test: 23/23 PASS
- HIGH_JUMP lossless projection, strict join, radial draft 보존과 bootstrap version focused test: PASS
- Server Release build 및 `Server.exe --contract-test`: PASS, failures 0
- Client Release 전체 build/link: PASS, errors 0
- JSON 4개 parse, Python compile, PowerShell AST parse와 `git diff --check`: PASS

Server contract는 첫 웨이브 `생존 플레이어 수 + 4`, 웨이브별 생존 플레이어 재집계,
`0/40/80` tick, 반경·높이·7m 간격, 추적 후 고정, static arena target, deterministic 재현과
snapshot 수용량을 확인한다. 스케줄 focused case는 내부 scheduler를 직접 호출하므로 실제
`Update_WorldEntities` call site 자체까지 실행하는 회귀 한 건은 후속 강화 여지로 남겼지만,
runtime call 연결은 코드 감사에서 확인했다.

전체 `test_valtan_pattern_master_v2.py` 40개 중 feature 관련 bootstrap 기대값 실패는 수정 후 focused
재검증했다. 남은 `v1 migration` 한 건은 이 worktree에서 동시에 진행 중인 다른 발탄 animation/manual
audition Product drift 때문에 실패하며 이번 HIGH_JUMP 변경으로 해당 타 작업 값을 되돌리지 않았다.

## 4. 빌드와 수동 화면 검증 경계

현재 사용자가 실행 중인 Debug `Server.exe`와 `Client.exe`가 각 출력 파일을 점유하고 있어 Debug
최종 link는 `LNK1104`로 중단됐다. 실행 중인 프로세스는 종료하지 않았다. 따라서 현재 열려 있는
발탄 아레나는 이전 Debug 바이너리이며, 새 동작을 보려면 사용자가 Client와 Server를 종료한 뒤
Debug를 다시 빌드하고 `Server + Client` profile로 재시작해야 한다.

Effect Tool과 실제 발탄 아레나에서의 빨간 원 농도, 확장 속도, 도끼 낙하와 폭발의 시각 품질은
사용자가 직접 확인하기 전까지 visual PASS로 기록하지 않는다.

## 5. Effect 타이밍 주의점

Server hit은 각 도끼 생성 시점에서 `1200ms` 뒤 발생한다. 현재 sky-axe Effect의 일부 element는
개별 도끼 기준 delay가 `4.0s`, `6.2s`, `6.3s`로 남아 있어 Server hit과 맞지 않고 object의
`4.0s` 수명 때문에 늦은 element가 보이지 않을 수 있다. 이 값들은 사용자의 Effect 저작 변경을
보존하기 위해 자동 수정하지 않았다. 폭발용 element라면 각 object 기준 약 `1.2s`에 시작하도록
Effect Tool에서 delay를 맞춘 뒤 사용자가 육안 승인해야 한다.

## 6. 보존한 동시 작업

worktree의 `manualAuditions`, `VALTAN_SEQUENCE_*`, animation binding, Effect JSON과 occurrence inventory
변경은 다른 진행 작업으로 보고 되돌리거나 stage하지 않았다. 이번 결과는 mixed-axe 관련 코드,
정본·생성 데이터, 검증 도구와 문서만 대상으로 한다.
