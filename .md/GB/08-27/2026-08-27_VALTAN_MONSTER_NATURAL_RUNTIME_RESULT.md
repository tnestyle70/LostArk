# Valtan Monster Natural Runtime Result

## 완료 상태

Lobby에서 Valtan Arena에 진입해 첫 일반 몬스터 웨이브를 만날 때 발생하던 즉시 snapshot transform, 공격 clip edge 누락, 매 tick 최근접 타깃 교체, 즉시 방향 전환, 첫 spawn 동기 model load를 기존 제품 경로 안에서 보강했다.

자동 구현과 자동 검증은 완료됐다. Client 화면의 최종 자연스러움은 사용자 전용 visual 판정 경계이므로 `Server + Client`를 재시작한 뒤 `Lobby -> Valtan`에서 직접 확인해야 한다.

## 실제 구현

### Client presentation

- `CNpc`의 root-motion 억제와 network-transform 보간을 독립 정책으로 분리했다.
- Server 권위 일반 몬스터는 기존 2-tick `CNpcNetworkTransformInterpolator`를 사용한다.
- 순수 `CMonsterPresentationContract`가 snapshot action을 `IDLE/CHASE/ATTACK/DEAD` 표현과 occurrence edge로 투영한다.
- `WINDUP`에서 공격 clip을 0초부터 한 번 시작하고 `ACTIVE/RECOVERY`는 같은 clip을 이어서 재생한다.
- 다음 `WINDUP`은 같은 clip이어도 새 occurrence로 재시작하고, late join과 DEAD hold도 명시했다.
- 실제 `.wmodel` clip 목록을 확인해 `MonsterCatalog.json` formatVersion 2에 archetype별 1~3개 `attackPresentations`를 등록했다. Server entity ID와 공격 occurrence tick으로 결정적으로 선택하므로 모든 Client가 같은 공격을 표시하면서 연속 공격의 반복감은 줄어든다.
- 공격별 `playbackRate`는 실제 clip 길이를 Server attack duration에 맞추는 presentation 전용 값이며 Server timing은 바꾸지 않는다.
- IDLE/CHASE 중 damage event를 받으면 실제 hit clip을 짧게 한 번 재생하고 원래 idle/chase clip으로 복귀한다. 공격과 DEAD는 피격 표현이 덮어쓰지 않는다.
- Valtan Loader가 지원 monster prototype을 첫 spawn 전에 준비한다. 한 archetype 준비 실패는 Arena 전체가 아니라 그 archetype만 격리한다.

### Server authority

- `MonsterProfiles.json` formatVersion 2에 target release range, turn speed, acceleration, deceleration, arrival slowdown radius를 추가했다.
- publisher와 spawn-group bootstrap을 v4로 올리고 strict parse/range validation을 연결했다.
- 현재 타깃이 살아 있고 release range 안이면 최근접 대상이 바뀌어도 유지한다.
- WINDUP에서 타깃과 공격 방향을 고정해 ACTIVE/RECOVERY 중 방향이 흔들리지 않는다.
- A* 결과에 기존 navigation line-of-sight string pulling을 적용한다.
- entity ID 기반 결정적 접근 지점, tick당 최대 회전각, 가속/감속, 목표 근처 감속을 적용했다.
- 기존 `CServerCollisionSystem` 원형 body sweep/slide를 monster move와 knockback에 사용하고 같은 tick의 body 위치를 갱신해 겹침을 줄였다.

### 계약과 회귀

- monster occurrence projection과 결정적 attack-pool 선택 계약을 `ActionPresentationTimelineHarness`에 추가했다.
- Server 계약에 target retention/release/attack lock과 spawn bootstrap v4 필드 검증을 추가했다.
- Valtan audition exact join은 selectable pattern action만 비교하고 `respawn/dead` lifecycle chain을 제외하도록 기존 validator의 실제 데이터 계약을 교정했다.

## 검증 증거

- JSON parse: PASS
- `Publish-WorldGameplay.ps1 -Mode Validate`: PASS
- World gameplay publish: PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
- `ActionPresentationTimelineHarness` Debug/Release: PASS
- Server x64 Debug/Release build: PASS
- `Server.exe --contract-test` Debug/Release: `failures : 0`
- Client x64 Debug/Release build: PASS
- `git diff --check`: PASS

### 2026-08-27 전체 재검증

- 정본 `Invoke-BuildAndRegression.ps1`로 Engine -> UpdateLib -> Shared ->
  Network harness -> Server -> Client를 Debug/Release 모두 다시 빌드했다.
- 발탄 split master 43개 failure/crash/rollback 테스트, animation 8개,
  effect 32개를 Debug/Release 실행에서 각각 통과했다.
- NetworkProtocolHarness Debug/Release, ValtanFourPlayerHarness Debug/Release,
  CharacterSelectIsolationHarness Debug/Release가 모두 `failures : 0`이다.
- Character Select 하네스는 움직이는 몬스터의 최신 snapshot을 추적하고 실제
  200ms hit probe를 사용한다. 두 private arena의 spawn/damage 격리, 재입장,
  Bern shared room, Bern NPC 확인을 통한 Valtan 전환 뒤 명령 처리까지 검사한다.
- 다섯 archetype이 참조하는 네 `.wmodel`을 직접 파싱했다. idle/chase/hit/dead와
  13개 attack presentation clip 누락은 0개이고, playbackRate 적용 뒤 Server
  attack 전체 시간과의 최대 차이는 0.269ms다.
- Debug Server를 숨김 프로세스로 `127.0.0.1:7777`에 띄워 exact PID가 LISTEN을
  소유하고 안정 구간 동안 살아 있음을 확인한 뒤 그 PID만 종료했다.

전체 회귀에서 함께 발견한 기존 회귀도 같은 작업 단위에서 교정했다.

- Windows PowerShell 함수 지역의 `$LASTEXITCODE`가 Python 실패를 성공처럼
  가리던 정본 빌드 스크립트를 global native exit code 캡처로 수정했다.
- CP949 콘솔에서 발탄 CLI 실패 출력을 decode하다 테스트 자체가 죽던 경로를
  replacement decode로 바꿨다.
- 130줄 floor wipe animation product, effect saved-row 예상치, ground-target
  preview scope 테스트를 현재 제품 계약과 다시 맞췄다.
- Bern의 `npc.bern.beda.guide`는 player spawn과 다른 navigation component에 있어
  3m 상호작용 반경으로 접근 불가능했다. 가장 가까운 연결 셀로 옮기고 계약
  테스트를 "둘 중 하나"가 아니라 Beda/Aylara 모두 접근 가능해야 통과하도록
  강화했다. 실제 live harness에서 Bern -> Valtan 전환 뒤 gameplay command까지
  통과했다.

### 환경상 남은 정본 게이트

이 PC에는 receipt가 고정한 Windows SDK `10.0.22621.0`의
`d3dcompiler_47.dll`이 없고 `10.0.26100.0`만 있다. 따라서 정본 회귀는 모든
빌드와 앞선 테스트를 통과한 뒤 Artist 31470 HLSL/WARP compiler identity
게이트에서 의도대로 실패한다. 고정 오라클을 완화하거나 receipt를 바꾸지는
않았다.

설치된 26100 compiler를 별도 read-only probe로 실행한 결과 compiled DXBC와
input hash는 고정 receipt와 같고, 최대 수치 오차 `1.1444091796875e-05`는 허용치
`2e-05` 안이며 WARP state provider도 같다. compiler raw hash와 output float32
byte hash는 고정 receipt와 다르므로 이를 정본 PASS로 기록하지 않는다.

Effect source closure에는 실제 Resources에 존재하지만 Git에서 ignore된 DDS 세
개가 필요하다. 전체 검증은 실제 index를 건드리지 않는 임시 index에서 세 파일을
추가해 통과시켰다. 팀 배포/PR 전에 아래 파일을 리소스 전달 범위에 포함해야 한다.

- `Effect/Warlord/Textures/FX_TEX_00/fx_a_hit_007.dds`
- `Effect/Warlord/Textures/FX_TEX_00/fx_a_ring_001.dds`
- `Effect/Warlord/Textures/FX_TEX_01/fx_c_symbol_003.dds`

기존 저장소의 CP949/PDB 관련 compiler/linker warning은 남아 있으나 새 compile/link error는 없다.

## 수동 확인 절차

1. 실행 중인 Server와 Client를 종료한다.
2. Visual Studio에서 `Server + Client` profile, x64 Debug를 선택하고 `Ctrl+F5`로 시작한다.
3. Client에서 `Lobby -> Valtan`으로 진입한다.
4. 첫 일반 몬스터 웨이브에서 다음을 확인한다.
   - snapshot마다 위치가 튀지 않고 이동이 이어지는지
   - 타깃 주변에서 급격히 좌우로 방향을 바꾸지 않는지
   - 여러 몬스터가 한 점에 완전히 겹치지 않는지
   - 공격마다 선택된 공격 clip이 처음부터 재생되고 ACTIVE/RECOVERY에서 되감기지 않는지
   - 같은 종류 몬스터가 연속 공격할 때 실제 보유한 공격 clip들이 섞여 보이는지
   - 추적/대기 중 피격하면 hit clip 뒤 원래 움직임으로 복귀하고 공격 중에는 공격 clip이 끊기지 않는지
   - 첫 몬스터 등장 순간에 model load hitch가 줄었는지

## 보존한 범위

작업 시작 전 존재하던 Bern/Valtan BGM·Sound Manager 변경과 다른 untracked 파일은 되돌리거나 stage하지 않았다. 이번 기능도 자동 commit/stage하지 않았다.

## 2026-08-28 PR 병합 전 교정

손상된 spawn bootstrap의 `ANCHOR` 좌표나 yaw가 비유한값 또는 지나치게 큰 값이어도
`from_chars`를 통과할 수 있었고, 기존 반복식 각도 정규화는 `1e20f`에서 진행하지 않아
Server fixed tick을 멈출 수 있었다. runtime loader는 이제 X/Y/Z/yaw가 finite이고
절댓값 100000 이하인지 stage 단계에서 검사해 실패 시 기존 committed catalog를 유지한다.
각도 정규화도 bounded remainder 계산을 사용한다. 정상 load 뒤 손상된 replacement를 넣어
revision/group/anchor가 바뀌지 않는 rollback 계약을 Server harness에 추가했다.
