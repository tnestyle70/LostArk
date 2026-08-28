# Valtan·Bern·Party 통합 구현 계획

## G00. 통합 범위와 보존

목표는 PR #247의 카메라·BGM·벽 붕괴·몬스터·Bern 변경, PR #249의 파티 초대와
월드 이동, 사용자의 현재 Effect 저작 변경을 기존 제품 계약 위에서 함께 검증해 병합하는 것이다.
원래 작업 폴더의 브랜치, index, 미커밋 파일은 수정하지 않는다.

- 기준 main: `0a08b084` (네트워크 안내 PR #248 포함)
- PR #247 원래 head: `2775c34c`
- PR #249 원래 head: `167c66c4`
- 초기 로컬 변경 스냅샷: `384d83a3`, parent `d77d7e02`, 명시한 소스·문서 30개
- 다른 작업의 최종 인계 추가 스냅샷: `0acac6aa`, `e3185f78`
- 사용자 최종 Effect 저장 포함 스냅샷: `6ba598a6`, 총 37개 경로
- 통합 작업 폴더: `C:\w\p247`
- 통합 브랜치: `codex/valtan-party-integrated-review`

스냅샷은 별도 Git index에서 만들고 원래 index와 30개 파일의 SHA-256이 바뀌지
않았음을 확인했다. 최신 main에 이미 있는 네트워크 변경은 파일 전체 복사가 아니라
공통 조상 기준 merge로 합친다. 다른 작업이 끝난 뒤 추가 diff를 다시 확인한다.
08-27의 미구현 계획서도 보존하지만 구현 완료로 취급하지 않는다.

## G01. PR #247의 제품 계약

`LevelRegistry`, `Valtan`, `GameRoom`, `SpawnGroupBootstrap`, `MonsterBrain`,
Shared packet bound와 대응 publisher/harness가 이 단위의 변경 지점이다.

- 109 컷신은 외벽 30개와 남아 있는 내부 벽 67개를 같은 tick에 처리한다.
- 이미 무너진 벽은 다시 적용하지 않고, contact/stage 중복은 동일 payload일 때만
  하나의 transition/application 쌍으로 합친다. 서로 다른 payload는 commit 전에 거부한다.
- 128 states와 함께 전송할 수 있는 destruction event 상한은 106이다.
- 등장 카메라의 `8600 + 5800 + 5467 = 19867ms` Server gate, 무적, 마지막
  1000ms follow 복귀를 유지한다. 카메라만 시작하는 stage가 BGM M04를 건너뛰지 않는다.
- Bern은 렌더와 같은 카메라 snapshot, 실제 decoded vertex bounds와 기존
  conservative margin/reject grace를 유지한다. 컬링을 전역 우회하지 않는다.
- monster bootstrap의 비유한/과대 anchor는 stage 단계에서 거부하고 yaw 정규화는
  입력 크기에 비례하는 반복문을 사용하지 않는다.

종료 증거는 Debug/Release 핵심 빌드, Network/Server 계약, world/navigation/destruction
validation이다. 카메라 구도·벽 잔해·컬링의 최종 육안 판정은 사용자가 한다.

## G02. Server 파티와 월드 이동

`GameRoom`, `ServerApp`, `ClientSession`, Shared party messages와 기존
`ServerGameplayContractTests`, `NetworkProtocolHarness`를 연결한다.

- 초대 응답은 현재 대기 중인 inviter identity를 확인한 뒤에만 소비한다.
  교체된 옛 초대의 accept/decline은 새 초대를 지우지 않는다.
- 이동 batch는 mutable pending 목록의 첫 원소를 leader identity로 사용하지 않는다.
- leader의 Bern NPC entry 한 요청이 전체 2~4인 batch를 소유한다.
- 모든 session, source membership, target capacity/profile/navigation/identity와
  초기 reliable 송신 준비를 끝내기 전에는 source player/party/HP/binding을 바꾸지 않는다.
- 성공은 한 room-thread commit 단위로 처리한다. 중간 admission 실패는 전원 source에
  남기며 부분 이동·부분 ENTER_ACCEPTED를 만들지 않는다.
- solo Join과 party Join은 같은 admission 검사를 소비한다. 별도 Client 권위 경로는 없다.
- #247과 #249가 서로 다른 wire 변경에 각각 v40을 사용했으므로 통합 protocol은
  v41로 구분한다. v40 handshake 거부와 106-event bound를 함께 검사한다.

정상 2/4인 초대·응답·이동, nonleader 거부, 목적지 잔여 자리 부족, 중간 profile/nav/
session/outbound 실패, stale response와 disconnect cleanup을 실행형 계약으로 고정한다.

## G03. Client 입력·HP·음악

`PartyInteractionView`, `PlayerController`, `ClientReplication`, `PartyWindowView`,
Bern/Valtan level과 `MainApp`이 실제 소비자다. Engine `Sound_Manager`와
`GameInstance`는 기존 음악과 독립된 looping-SFX 채널만 확장한다.

- 파티 메뉴에서 소비한 LB/RB는 실제 버튼 release 전까지 gameplay로 재해석하지 않는다.
- roster는 기존 snapshot의 NetEntityId와 HP를 join한다. HP 미수신은 가득 찬 체력으로
  꾸미지 않고 숨긴다. stale tick, despawn, reconnect의 이전 HP가 남지 않게 한다.
- ItemUpgrade의 반복 효과음은 BGM 채널을 교체하거나 중지하지 않는다. 새 효과음의
  준비 실패 시 기존 음악·효과음은 유지하며 level 종료에는 효과음만 정리한다.
- sound cue parser는 명시적 `NONE`/빈 clip action에 남은 occurrence만 격리한다.
  unknown action, 재생 가능한 action의 잘못된 occurrence/each_loop는 실패로 남긴다.

순수 helper의 실행형 테스트와 parser fixture, Engine→UpdateLib→Client 빌드가 종료
증거다. 실제 음악 청취·파티 UI 화면 조작은 자동 검증으로 대체하지 않는다.

## G04. 사용자 Effect 저작과 Boss 연결

`BossTool`, `Effect_Tool`, `MainApp` routing과 Effect/Valtan pipeline을 사용한다.

- stable pattern/stage/cue occurrence/effect tuple을 one-shot으로 전달한다.
- fresh product tree에서 정확히 하나의 현재 occurrence를 찾은 뒤 기존 preview를 연다.
  clip-bound와 `STAGE_CLOCK`은 기존 각 경로를 재사용하고 자동 재생하지 않는다.
- unlink 진행 중에는 새 deep-link도 기존 Open/Refresh와 같은 잠금을 따른다.
- unlink는 source CAS → atomic publish → validate이며 실패하면 source와 product를
  baseline으로 돌린다. V1 migration이 사용자가 지운 cue를 다시 만들지 않는다.
- 사용자의 recovery Effect 1 element/0.001 scale 저장본과 29-slot audition 순서를
  보존한다. 이 데이터가 틀렸다는 이유로 원래 7 elements/28 slots로 되돌리지 않는다.
- 97 world members / 135 placements와 로컬 unlink subset 계약을 동시에 검사한다.

one-shot routing, tuple 오류, 잠금, preview 분기, Save/Discard 대기와 저장/rollback을
focused tests로 고정하고 source index 및 Valtan V2 suite를 재실행한다.

## G05. 검증·문서·병합

사용자의 최신 우선순위는 이미 확인된 치명적 문제를 수정한 뒤 우선 병합하고, 나머지
장시간 전체 회귀는 병합 후 실행하는 것이다. 아래 전체 검증을 모두 통과하기 전에는
전체 회귀 PASS 또는 사용자 화면 확인 완료로 기록하지 않는다.

1. JSON/XML parse, `git diff --check`, domain publisher Validate/Check와 focused tests.
2. 정본 `Invoke-BuildAndRegression.ps1`의 Debug/Release와 Engine→UpdateLib→
   Shared/Network/Action harness→Server→Client 순서.
3. 별도 loopback Server를 사용하는 4인/CharacterSelect isolation harness.
   실행 중인 사용자의 Server/Client는 건드리지 않는다.
4. 기존 테스트의 stale assertion도 실제 코드·데이터와 대조한 뒤 교정한다.
   환경 리소스 누락이나 미실행 항목은 PASS로 기록하지 않는다.
5. 다른 작업의 완료 및 추가 diff, remote PR head/main을 다시 확인한다.
   검증한 tree에 한해서 기존 PR과 통합 수정 PR의 안전한 병합 순서를 정한다.
6. RESULT에 실제 구현, 자동 검증, 사용자 수동 검증, 미구현 계획·물리 리소스 경계를
   분리하고 최종 main ancestry 및 변경 포함 여부를 확인한다.

신규 C++ 파일이 필요해지면 해당 프로젝트와 filters 등록도 같은 변경에 포함한다.
build 산출물, EngineSDK, private 규칙/로그는 commit하지 않는다.

## G06. 병합 뒤 실행 검증의 격리

PR #250은 `cd120501`로 main에 병합됐다. 후속 작업은
`codex/post-merge-valtan-party-regression`에서 수행한다.

- `ServerApp.h/.cpp`의 private mutex 생성/획득 helper를 이름 선택과 분리한다.
  제품 wrapper의 기존 Debug/Release 전역 이름과 단일 owner 보호는 바꾸지 않는다.
  friend contract fixture만 PID별 test-owned 이름으로 같은 Win32 경로의 최초 획득,
  다른 thread의 동시 owner 거부, release 뒤 재획득과 실패 handle 정리를 검사한다.
  durable pointer reset 검사는 별도 assertion으로 남긴다. CLI/env 우회는 추가하지 않는다.
- `EffectRenderContractHarness.cpp`는 명시적 `LOSTARK_RESOURCE_ROOT`를 하드코드된
  checkout 경로로 덮어쓰지 않고 기존 `CRuntimeAssetRoot`의 해석·검증 경로를 소비한다.
  source/compiled shader와 runtime resource root의 소유권은 분리해 유지한다.
- Action Release, 최종 Valtan V2/focused/data validators와 Release 실제 socket 기반
  2/4인 이동을 실행한다. 실패는 실제 입력·코드와 대조해 필요한 수정만 수행한다.
- 실제 socket 검사에서 solo/2인/4인의 가이드 접근을 한 process에 누적하면 기존
  55초 harness watchdog을 넘었다. native harness에 검증 전용 party-size 선택을 두고
  runner는 Core/Party2/Party4를 각각 새 owned Server로 순차 실행한다. 각 process의
  기존 bounded timeout과 Server의 60초 상한은 유지하고 timeout 직전 로그도 보존한다.
- 제품 Debug Server의 전역 owner가 이미 실행 중이면 새 Debug live Server는 계속
  거부되어야 한다. 이 경계를 테스트를 통과시키려는 목적으로 해제하지 않는다.
- 정본 자동화는 순서대로 실행하며, 사용자 Server와 UI는 종료·조작하지 않는다.
  자동 수치 검증을 사용자 visual/audio PASS로 기록하지 않는다.
- 실제 실행에서 호출자의 지역 `LASTEXITCODE=0`이 native 실패를 가리는 문제가
  확인됐다. 정본 runner, 세 native harness wrapper와 Valtan projector/test wrapper의
  native 상태 읽기/초기화를 전역 자동 변수로 통일한다. AST guard와 실제 invalid-resource
  native 호출을 지역 success sentinel 아래 실행해 실패 전파를 고정한다.

새 C++ 파일은 필요하지 않다. 리소스 경로의 8가지 native 검증을 담당하는
`Test-EffectRenderResourceRoot.ps1`만 기존 Effect harness project/filter의 Scripts에
등록하고 정본 회귀에서 실행한다. `Tools/Build/Test-NativeHarnessExitPropagation.ps1`은
정본 runner가 직접 소비한다. 최종 결과와 남는 환경 경계를 RESULT에 추가하고
후속 PR은 검증한 변경만 포함한다.
