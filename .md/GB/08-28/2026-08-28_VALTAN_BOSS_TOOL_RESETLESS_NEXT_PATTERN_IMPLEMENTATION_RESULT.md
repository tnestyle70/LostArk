# Valtan 저장 Flow·실시간 Next·재생 시간 반영 결과

작성일: 2026-08-28.

현재 요청의 구현·검증은 **G08~G11**에 기록한다. G00~G07은 PR #253까지의 이전 작업
이력이며, 당시의 isolated 전용 Next, 문서만 저장하는 Flow, protocol 42 설명은 현재 계약이 아니다.
현재 작업 브랜치는 `codex/valtan-flow-reload-next-fix`, 시작 HEAD는 main과 같았던
`38cc82c85c3e78b900cde1ab27c92523d34b947b`이다. 작업 중 추가된 main PR #255는
`1a9bce42ac00f75b2e1b6fa24ed0de1071d4c050`까지 동기화했다. 전체 빌드의 최종 상태는 G10을 따른다.

## G00. 현재 적용 위치와 검증 상태

이 세션과 관련 Effect 작업의 구현을 **Desktop LostArk 정본 작업 트리**에 보존 통합했다.
사용자 Debug 빌드의 컴파일 오류 없음 확인 뒤 Release 빌드와 자동 회귀를 재개했다.
Desktop의 Server Release와 모든 검증용 실행 파일의 Debug/Release 빌드는 통과했다.
Server 계약은 두 구성 모두 failures 0이며, Release 4인 접속·격리·2인/4인 파티 이동도 통과했다.
Client Release 첫 링크의 old/new object 혼재 오류를 최신 소스 재빌드로 해소했다.
최신 Client·Server Debug/Release 빌드와 회전 수정의 실제 Server tick 회귀는 모두 통과했다.
UI PR #252와 이 기능의 PR #253 통합 뒤 Desktop 정본 Release 빌드·전체 회귀를 exit 0으로 완료했다.
사용자 Debug Client 빌드와 후속 Debug 계약·native도 통과했다. 사용자 Server/Client가 실행 중이므로
Debug 전체 live 재실행은 보류했으며, 08:14의 전체 Debug PASS 이력과 구분한다. 자세한 최종 증거는 G07을 따른다.
사용자 시각 검증은 대신 완료 처리하지 않는다.

| 항목 | 실제 상태 |
|---|---|
| 최종 솔루션 | `C:/Users/user/Desktop/LostArk/Framework.sln` |
| 현재 브랜치 | `codex/valtan-arena-next-desktop` |
| 검증한 구현 HEAD | `eec64f8dcdcd2e2d62671a4f0c8d02a259832042`, UI PR #252 + 기능 PR #253 병합 main |
| main 직접 수정 | 하지 않음. main 기반 별도 브랜치에서 작업 |
| 이전 구현 worktree | `C:/w/valtan-arena-next`, `codex/valtan-arena-navigation-next` |
| 다른 작업 구현 | `C:/w/valtan-half-ring-clock`의 동결된 60개 구현·데이터·하네스·DDS 파일 통합 |
| Desktop 원본 보존 | dirty 55파일을 bytes로 백업하고 아래 stash 보존 |
| 사용자 Effect 저장 | 최초 통합의 8문서·3시 기존 4개 element·삭제 cue 4건을 보존했고, 후속 저장된 3시 현재 2개 element와 빈 발악 Draft도 유지. FOUR는 P1/P2 분리 |
| 통합 기준 | UI PR #252가 포함된 `origin/main c813f5b8`과 현재 변경의 비파괴 merge-tree 검사 exit 0, 충돌 없음 |

Desktop 기존 브랜치 `codex/team-endpoint-10-207-18-103`의 HEAD는 d77d7e02였다.
이전 Desktop 변경 중 이미 main에 들어간 부분은 중복 적용하지 않았으며,
Effect Tool의 진행 중 unlink transaction guard 등 최신 main의 안전장치도 유지했다.

보존 stash는 `e49792d0d188e5217346fc9bd56d0a61553b91f6`이다.
원본 백업과 적용 대응표는 `C:/w/valtan-arena-next/_work/valtan-arena-next/` 아래
`desktop-before-lerp-20260828-063408`, `desktop-integration-staged/manifest.json`에 있다.
PR #251 통합 전 별도 안전 stash `9fb5beb46aae1d7a9320b362d53729295a3b888f`도 유지했다.

## G01. 이 세션의 구현

### 파괴된 벽·벽돌과 돌진

입구 벽의 source collider를 제거해도 sibling receiver collider 두 개가 남아
보이지 않는 기존 벽이 돌진과 이동을 막는 문제를 수정했다.
벽 그룹의 정확한 source/receiver 소유권을 publisher가 검증한다.

99개 벽의 collider 141개가 각각 하나의 파괴 mutation에 속한다.
같은 바닥 높이와 겹치는 벽 98개에 Nav 영역을 연결했고, 기존 바닥 붕괴 6개를 합쳐
동적 영역 104개를 유지한다. 벽의 Nav 참조는 3905개, 고유 셀은 2109개다.
상부 벽 하나에는 같은 높이의 바닥 셀이 없으므로 임의 Nav를 만들지 않았다.

BREAKING 중에는 아직 벽이 이동을 막고, 실제 DESPAWNED tick에만 해제된다.
다른 벽을 먼저 해제하거나 파괴되지 않은 입구를 자동으로 열지 않는다.
Server LOS도 DDA/supercover로 지나가는 셀과 모서리를 검사하도록 보강했다.
기본 392×312 / 0.5m Nav의 21524 walkable 셀과 기존 바닥 영역의 순서·극성은 보존했다.

### 발탄 콜라이더

Server body radius를 **3.0m → 1.4m**, 지름을 **6.0m → 2.8m**로 맞췄다.
실제 idle/run 99프레임의 다리 최대 반경 1.3781m를 근거로 잡았으며,
몸체의 이동·접촉과 Client Debug mirror가 같은 반경을 사용한다.
모델 presentationScale 1.0과 공격 hit shape는 함께 축소하지 않았다.
수치 provenance는 PROJECT_TUNED로 기록했다.

### Boss Tool Next Pattern

현재 isolated A에 다음 B **한 칸**을 Server 권위로 예약한다.
최초 isolated Play의 준비/reset은 유지하고, Next 예약·교체·취소·승격은 reset하지 않는다.
플레이어·발탄 위치, HP, 자원, cooldown, 파괴된 벽·바닥·prop 상태를 유지한다.

A의 마지막 hit/attachment/prop/world commit과 실제 COMPLETED가 확정되어야
다음 fixed tick에 B가 시작한다. 살아 있는 플레이어가 없으면 WAITING_FOR_PLAYER를 유지하며,
자동 revive·중앙 이동·15초 timeout을 만들지 않았다.
owner/boss/room epoch/선행 occurrence/command sequence/token을 검증하고,
정확한 재전송은 멱등 처리하며 stale 요청은 거부한다.

Repeat·Ordered Flow와 동시 실행을 막는다. 저장 Flow를 수정하지 않으며,
Save/Reload Flow는 dirty 확인을 거치는 로컬 저작 동작이다.
Tool을 닫아도 공용 audition service가 상태를 소비한다.
한 번 소비된 B가 PENDING에서 WAITING으로 돌아가도 교체·취소를 다시 허용하지 않는다.

통합 후 대상은 **split 29 / 공유 Tool 27**이다. 초기 통합에서는 퇴출된 `slot.000017`만 제거해
저장 Flow 28슬롯/`nextSlotOrdinal=30`을 보존했다. 이후 사용자가 저장한 현재 Flow는
**22슬롯/`nextSlotOrdinal=31`**이다. 새 `slot.000030`, 삭제·재정렬한 슬롯을 그대로 보존하며
초기 28슬롯 상태로 되돌리지 않는다.
Shared wire는 protocol **42**, gameplay bootstrap은 **v25**이며,
`PATTERNAUTHORINGMANAGED`로 실제 split 관리 집합을 전달한다.
Release는 Debug Next 명령을 허용하지 않는다.

### 버러지 포획·counter·전멸

`VALTAN_TRASH`를 14-stage 분기로 구현했다.

- `13_04` 실제 명중으로 capture를 commit한다.
- NONE은 `13_05-2` miss 뒤 재조준한다.
- PARTIAL/ALL은 `13_05-1`의 처음 200ms 동안 counter를 받는다.
- 성공한 counter는 실제 420631 GROGGY와 해제를 수행한다.
- PARTIAL은 1500ms impact에서 잡힌 대상만 damage·해제하고 3000ms tail 뒤 재조준한다.
- ALL은 impact에 참가자를 재검증하고 HP 0·DEAD·detach를 원자적으로 commit한다.
- 전멸 후에도 tail이 끝나야 COMPLETED가 되며, 예약된 Next는 명시적 revive까지 기다린다.

빈 참가자 집합, 다른 boss/slot, stale occurrence, 잘못된 transform은 ALL로 인정하지 않는다.
모든 대상과 event capacity를 먼저 검증하여 중간 실패 시 일부만 죽거나 풀리지 않도록 했다.
GRABBED 동안 일반 command를 막고, 해제 후에는 키·마우스를 실제로 놓은 뒤 다시 눌러야 입력된다.

보조 Trash 세 정의와 기존 Effect/Sound authoring은 보존했다.
canonical grip pose bake와 보조 cue 이관은 별도 미완료 경계다.

## G02. 함께 발견하고 수정한 오류

| 문제 | 실제 수정 |
|---|---|
| Desktop의 `EFFECT_DETAIL_DESC::Lerp` 컴파일 오류 | `Effect_Tool.cpp`의 잘못된 `Detail.Lerp.bScale`을 실제 멤버 `Detail.LinearLerp.bScale`로 수정 |
| Client가 새 Trash 문서 로드를 거부 | `CEncounterPatternReference`에 ANY_PLAYER_GRABBED 및 typed DAMAGE/EXECUTE admission 연결 |
| prop stage 실패가 정상 완료로 이어짐 | 실패 전파, ABORTED/not-ready 처리 및 Next 승격 차단 |
| 포획 transaction 실패 뒤 cleanup이 attachment를 해제 | ENTER/EXIT preflight 실패에서 기존 boss·attachment·receipt 보존 |
| 공개 Revive/DebugKill/Move 입력의 GRABBED 우회 | 일반 입력과 같은 admission 적용 |
| 파괴 publisher가 검증용 숫자까지 반환 | counterProxy 숫자 반환 소비, artifact set이 정확히 하나인지 ContractTest 추가 |
| PowerShell이 정상 native stderr를 실패로 오인 | 실제 exit code 보존, 경로·환경·preference 복구 |
| PS5의 Process.Kill(true) 미지원 | 에이전트가 소유하는 console process만 bounded 종료 |
| Animation promotion이 Trash를 8단계로 다시 생성해 14단계 보존 검증에 실패 | 기존 `author_trash_capture_flow`를 재사용해 분기 6개를 연결하고 identity·시간·clip 검증 및 저작값 보존 유지 |

Client parser 회귀는 실제 최신 Trash 로드와 잘못된 입력 15건에서 기존 reference 보존을 검사한다.
실제 audition service CPP를 사용하는 Next harness도 프로젝트·solution·정본 자동화에 등록했다.
기존 Server fixture의 새 body 크기·motion·retired ID 기대를 실제 계약에 맞췄으며,
잘못된 반경이나 stale receiver를 허용하도록 판정을 완화하지 않았다.

## G03. 다른 작업에서 받아 함께 반영한 내용

`Add phase 2 floor effect elements`의 동결된 60파일을 base a6871a2f 기준으로 병합했다.
서로 다른 변경은 결합하고, 충돌한 Tree/Server fixture/authoring 파일에서는 양쪽 계약을 보존했다.

- 사용자 Effect 저장 8문서와 삭제 cue 4개를 보존했다.
- 최초 통합에서 3시 반원 Effect의 기존 4개 element를 유지하고 새 반원 element를 더했다.
  이후 3개, 다시 2개 element로 저장됐으며, 이 후속 편집을 초기 병합 상태로 되돌리지 않았다.
- `VALTAN_SEQUENCE_FRONT_BACK_FRONT`를 퇴출하고 2페이즈 4방향 정본을 정리했다.
- Boss Tool/All Effects의 pattern identity summary를 연결했다.
- Effect delay·clock·tail·Create/Open과 1 tick 미만 burst 수명 보강을 받았다.
- Sound의 실제 511행/skip 0 기대와 별도 3행 skip fixture를 우리 Trash parser 검사와 함께 보존했다.

사용자가 발견한 1페이즈 4연속/2페이즈 4방향 Effect 공유 파일 문제도 다른 작업에서 복구했다.
Desktop 실제 파일을 다시 읽어 P1 clip-01의 원래 7개 element 복원, clip-02의 2개 element 무변경,
P2의 기존 21개 element를 새 `effect.valtan.project-tuned.sequence.four` 파일로 분리한 것을 확인했다.
P1의 SLASHES/SPIN cue는 그대로이며 P2 STEP_01만 새 파일을 참조한다.
이 복구 뒤에는 이전 동결 인계본의 21개 element clip-01을 다시 복사하지 않는다.
복구 증거는 Desktop `_work/valtan-four-product-recovery/20260828-070956/receipt.json`이다.

새 resource dependency는 아래 DDS 한 파일이다. Desktop의 실물과 고정 인계본이 동일하다.

`Client/Bin/Resources/Effect/Valtan/Textures/EFMASTER_MATERIAL_PROLOGUE/fx_c_symbol_003_half_ring.dds`

이 파일은 512×512 BC1, 131200bytes다. 전체 resource 팩을 복사하거나 교체하지 않았다.

## G04. 실제 검증 결과

worktree의 개별 Debug/Release 성공은 사전 검증이며,
Desktop 최종 완료 여부는 다음 표를 따른다.

| 검증 | 현재 결과 |
|---|---|
| 통합 전 Desktop Lerp 수정 후 Engine/UpdateLib/Shared/NetworkProtocolHarness/Server/Client Debug | PASS, 2026-08-28 06:45 Client 링크 exit 0 |
| Desktop 통합 split PublishV2 | PASS |
| Desktop 통합 root-motion 생성과 provenance 갱신 | PASS |
| Desktop 통합 파괴·Nav·balance publisher | PASS |
| 통합 후 데이터 | split 29, Trash 14-stage, Encounter 53 patterns / 234 stages |
| 통합 후 Desktop 자동 Debug 전체 | 사용자 VS 빌드와 중복 방지를 위해 Action harness 빌드 중 중지; 전체 PASS 아님 |
| 사용자가 시작한 Desktop Debug 빌드 | 사용자 서면 확인: 컴파일 오류 없이 완료. 이후 Client 07:14:19 / Server 07:15:42 최신 exe도 확인. VS UI는 자동 조작하지 않음 |
| Desktop Release Engine / UpdateLib / Shared / Protocol / NextService / FourPlayer / Isolation 빌드 | PASS, 각 exit 0 |
| Desktop Action / Effect / PointLight 하네스 Debug·Release 빌드 | PASS, 각 exit 0 |
| Desktop Server Release 빌드 | PASS, exit 0 |
| Desktop Client Debug·Release 최신 빌드 | PASS. UI 병합 후 Debug는 사용자 08:44 링크 로그의 오류 0을 확인했고, Release는 G07 정본 빌드에서 exit 0 |
| Desktop Protocol / NextService Debug·Release 실행 | PASS, protocol failures 0 / NextService 14개 모두 통과 |
| Desktop Server 계약 Debug·Release 실행 | PASS, 최신 회전 수정 포함 두 구성 모두 failures 0 |
| Desktop Release FourPlayer / Core / Party2 / Party4 live 실행 | PASS, 사용자 Debug Server와 다른 격리 포트 사용 |
| Desktop Action Debug·Release / Effect·PointLight Debug 실행 | PASS, 각 exit 0 |
| Native exit/stderr/timeout cleanup 회귀 Debug·Release | PASS, 각 exit 0 |
| CompiledShaderClosure / EffectRenderResourceRoot Debug·Release | PASS, resource root는 구성별 8개 native 사례 |
| EffectRender / PointLightFalloff Release | PASS, 각 exit 0 |
| Six Pizza camera Python / 최신 Action native Debug·Release | PASS, Six Pizza cue·invocation 부재 및 다른 카메라 보존 |
| Artist oracle unit·receipt·HLSL/WARP, floor emissive, body composition, ground-target, Rendering Validate·Python | PASS, 8개 gate 모두 exit 0 |
| Boss / Flow / Tree / Balance Python | PASS, 각각 21 / 23 / 25 / 25 tests |
| Master V2 Python, FOUR 복구 고정 후 재실행 | PASS, 53 tests. 복구 중 source 변경으로 실패한 앞선 실행은 최종 증거에서 제외 |
| Animation-chain promotion / Saved Flow 최신 Python | PASS, 15 + 23 = 38 tests. Trash14 보존 및 휠윈드 방향 영속 회귀 포함 |
| Desktop 전체 Debug 정본 자동 회귀 | 통합 전 이력: 08:05:45~08:14:23 exit 0, 3363개 입력 hash 동일. UI 병합 후 Debug 빌드·계약·native는 G07; 전체 live는 사용자 실행 세션 보호로 재실행하지 않음 |
| Desktop 전체 Release 정본 자동 회귀 | PASS, UI 병합 후 08:40~08:49 실제 exit 0. 이전 08:20 saved-rows 불일치는 사용자 저장값 보존 및 fixture 수정으로 해소. 추가 저장값 재검증은 G07 |
| Client/UI 실행·화면 캡처·visual PASS | 에이전트가 수행하지 않음; 사용자 판정 필요 |

자동 빌드 중지의 exit 4294967295는 사용자 빌드와 겹치지 않도록
에이전트 자신의 process tree를 종료한 기록이다. 컴파일 오류로 단정하지 않는다.
Visual Studio 18 Insiders와 그 자식 process는 종료하지 않았다.

사용자 빌드 증거는 `Client/Default/x64/Debug/Client.log`와
`Server/Intermediate/x64/Debug/Server.log`다. C4819/LNK4099 경고는 남아 있지만
C++ error/fatal error는 없고 두 exe의 링크 결과를 확인했다.
Server 로그의 `pwsh.exe` 탐색 메시지는 Visual Studio vcpkg target이 PowerShell 7을
찾은 뒤 시스템 Windows PowerShell로 fallback하는 단계에서 발생했다. 프로젝트의 C++ 오류와 구분한다.

로그는 `C:/w/valtan-arena-next/_work/valtan-arena-next/` 아래에 있다.

- `desktop-lerp-debug-build.log`: Desktop Lerp 수정 후 실제 Debug 빌드 성공
- `desktop-integrated-publish.log`: 최종 통합 authoring의 publisher 전 단계 exit 0
- `desktop-canonical-debug.log`: 사용자 빌드 시작으로 중지한 자동 검증
- `desktop-auto-build-stopped-for-user.json`: 중지한 에이전트 소유 process 목록

재개 후 실제 Desktop 로그는 `C:/Users/user/Desktop/LostArk/_work/valtan-arena-next/` 아래에 있다.

- `desktop-release-core-build.log`, `desktop-aux-builds.log`: 완료된 Release 선행 빌드와 D/R 하네스 빌드
- `desktop-server-release-build.log`, `desktop-client-release-build.log`: Server/Client Release 실제 빌드
- `desktop-server-contract-Debug.log`, `desktop-server-contract-Release.log`: 계약 failures 0
- `desktop-release-live-regression.log`: FourPlayer/Core/Party2/Party4 실제 실행
- `desktop-native-regression-first-pass.log`, `desktop-native-exit-propagation.log`: native 회귀
- `desktop-release-followup-python-master-v2-after-four-recovery.log`: 복구 고정 후 Master 53개 재검증
- `chain-promotion-trash14-20260828-071532.log`: Trash promotion 14개 회귀

## G04-A. 후속 수정과 검증

Six Pizza의 STEP_03 camera invocation과 직접 런타임에서 검색하는
`camera.valtan.six-pizza-106.landing` cue를 모두 제거했다. 해당 cue의 shake도 함께 제거됐으며
다른 11개 camera cue와 deathCue는 변경하지 않았다. Camera Python과 최신 Timeline helper가
링크된 Action native Debug/Release는 모두 통과했다. 사용자 Client 재시작 뒤의 화면 판정은 별도다.

Client Release 최초 링크 오류는 새 `Resolve_CuePreviewTimelineTime` 호출을 가진 Effect_Tool.obj와
변경 전 ActionPresentationTimeline.obj의 혼재였다. 함수 선언/정의가 모두 존재하고 최신 Action
Debug/Release가 실제 링크·실행됐다. 이후 실제 Client Release 재빌드도 exit 0으로 링크·배포까지 완료했다.

점프찍기 후 휠윈드의 방향 변경은 animation local AI가 아니라 Server의 매 tick 타깃 추적 설정이었다.
`LOCK_NEAREST_ON_START + LOCK_FACING_ON_START`로 STEP_01 점프·STEP_02 준비·STEP_03 회전 중
방향을 고정하고, 무피해 STEP_04 종료 동작 ENTER에서 기존 `RETARGET_RANDOM_ALIVE`로 살아 있는
플레이어를 한 번 재선택해 바라본다. 여러 명이면 기존 이벤트의 Server 난수 규칙으로 한 명을 고르며,
가장 가까운 대상만 다시 고르는 규칙은 아니다. 이 종료 동작에서도 계속 추적하지 않는다.
Server 실제 Room tick 회귀는 정확한 STEP_03 종료 시점, 첫 3단계 방향 잠금, 재실행, 재조준 후 잠금,
대상 부재 시 ABORTED 및 유한 yaw 보존을 Debug/Release 모두 통과했다. 나머지 패턴·타격 시간·
애니메이션·Effect를 변경하지 않았다.

Shader 검사 첫 실행의 `Get-FileHash` 오류는 PS7에서 상속한 PSModulePath가 Windows PS5의
Utility module보다 먼저 잡힌 환경 문제였다. 해당 검증 child process의 module path만 Windows
PowerShell 경로로 지정한 재실행에서 Debug/Release shader·resource root 검사가 모두 통과했다.
시스템/user 환경이나 제품 코드는 이를 위해 수정하지 않았다.

정본 Debug 전체 회귀의 첫 재시도는 새 P2 FOUR Effect의 프로젝트 등록 정렬 검사에서 중단됐다.
`Client.vcxproj`의 한 줄과 `.filters`의 해당 세 줄 블록만 생성기가 요구하는 위치로 옮겼다.
기존 파일 목록·필터·GUID와 Effect 문서 내용은 그대로이며, byte 비교로 이 이동 외 변경이 없음을 확인했다.
`Sync-EffectDataProject.ps1 -Check`는 files 2396 / filters 219로 통과했고,
이후 Client Debug/Release 재빌드도 source hash 변경 없이 각각 exit 0이다.
앞선 중단 로그는 `desktop-canonical-final-Debug.log`에 보존하고 전체 회귀를 다시 실행한다.

후속 로그는 같은 Desktop `_work/valtan-arena-next/` 아래에 있다.

- `six-pizza-camera-removal/native-build-regression.log`: 최신 Action D/R 빌드·실행
- `six-pizza-camera-removal/final-validation.log`: camera 제거 후 split validation
- `desktop-six-pizza-camera-removed-python.log`: camera Python PASS
- `desktop-shader-resource-release-native.log`: shader·resource root D/R 및 Release native
- `desktop-common-gates.log`: 공통 domain 8개 gate
- `desktop-latest-publish-and-client-release.log`: 최신 PublishV2/ValidateV2·Client Release 빌드
- `desktop-client-debug-latest-build.log`: 최신 Client Debug 빌드
- `desktop-server-whirlwind-build-regression.log`: Server D/R 빌드·회전 포함 전체 계약
- `desktop-whirlwind-promotion-flow-python.log`: 38개 회귀
- `desktop-final-structure-check.json`: JSON 37 / XML 6 / Python 17 parse, 변경 범위 diff check 0
- `effect-project-registration/applied-receipt.json`, `check.log`: FOUR 등록 위치만 이동·정본 검사
- `desktop-final-client-Debug.log`, `desktop-final-client-Release.log`: 등록 정렬 수정 후 재빌드

## G05. 실행·수동 검증 경계

다른 작업의 최신 Effect 시작 기준·빈 발악 Draft와 이번 점프찍기 휠윈드 방향 수정을 포함해
영향받은 대상을 D/R로 재빌드했다. UI PR #252 통합 후 전체 Release와 후속 Debug native 결과는
G07에 기록했다. 사용자 실행 중인 Debug 전체 live 재실행과 시각 검증은 별도다.
사용자는 08:13에 정본 Debug Server/Client를 다시 실행했다. 이 사용자 process는 그대로 두고,
자동 live 검증은 별도 구성·격리 포트와 에이전트 소유 process로만 수행한다.
Release 전체 검사는 완료했지만 Debug 전체 live 재실행과 사용자 시각 검증을 대신 PASS로 기록하지 않는다.

- [Next PLAN](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_PLAN.md)
- [벽·Nav RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_DESTROYED_WALL_NAVIGATION_IMPLEMENTATION_RESULT.md)
- [Trash RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_VALTAN_TRASH_CAPTURE_COUNTER_SERVER_FLOW_RESULT.md)

## G06. main 병합 직전 최신 저장값 검증

2026-08-28 08:36 기준 현재 파일을 검증했다. 사용자 요청대로 저장된 9시 Effect의
첫 `TAKEOFF` 클립 0초 연결도 포함하며, Effect의 -180도 회전과 Element 값은 변경하지 않았다.
사용자가 추가한 TWOHAND·WHIRLWIND Draft도 보존하고 프로젝트의 Data 등록만 동기화했다.

- Saved rows / requested elements / authoring document / All Effects: 총 104 tests, 실패 0, 기존 skip 7.
- Effect source validator: direct 197 / reference 271 / resource 1029, exit 0.
- Effect project registration: files 2398 / filters 219, exit 0.
- 변경 파일 JSON 40 / XML 6 / Python 17 구문 검사 통과.
- 사용자 half-ring Start Delay 1.36초를 보존했다. native 검사 시점을 고정 0.5초 대신
  실제 첫 birth 이후 유효 수명 안으로 계산하도록 보완했다. 이 시점의 미실행 native 재빌드·검증은 이후 G07에서 통과했다.

로그: `_work/valtan-arena-next/desktop-git-ready-focused-tests.log`,
`desktop-git-ready-source-validation.log`, `conflict-preflight-20260828-083117/result.json`.
이 결과는 UI PR #252 통합 후 전체 빌드나 사용자 육안 검증을 대신하지 않는다.

## G07. PR #253 병합과 정본 Release 완료

구현 커밋 `2bbf7322`에 현재 저장된 131파일을 담고, UI PR #252의 main `c813f5b8`을
`35558ffa`로 자동 병합했다. 실제 충돌은 0건이다. PR #253은 `eec64f8d`로 main에 병합됐고,
08:38 Desktop에서 `git pull --ff-only origin main` 후 HEAD와 origin/main이 일치했다.
main이 다른 로컬 worktree에 checkout되어 있어 Desktop의 `codex/valtan-arena-next-desktop`
브랜치는 유지했다. 다른 worktree나 기존 safety stash는 변경하지 않았다.

실제 merge tree에서 우리 전용 129파일은 구현 부모, UI 전용 10파일은 UI 부모와 blob·mode가
같다. 공통 project/filter에는 RaidClear layout과 신규 Effect 문서 3개가 각각 한 번 등록됐다.
P1 FOUR 7+2 / P2 FOUR 21 분리, Six Pizza camera 0 / 다른 camera 11 보존,
9시 TAKEOFF 첫 clip 0초와 당시 저장된 -180도 회전도 부모 커밋과 동일하다.

| 검사 | 실제 결과 |
|---|---|
| Desktop 정본 Release 전체 build/regression | 08:40:21~08:49:51, 실제 PowerShell child exit 0 |
| Release 빌드 | Engine → UpdateLib → Shared/Protocol/Next/FourPlayer/Isolation/Action → Server/Client → Effect/PointLight 모두 통과 |
| Release 자동 회귀 | Master 53, Animation 11, Saved rows 33(skip 7), domain publishers, protocol/Next/Server 계약, FourPlayer/Core/Party2/Party4 live, Action/Effect/PointLight 모두 통과 |
| 사용자 Desktop Debug Client 빌드 | 08:44:17 링크·배포 완료, compiler/link errors 0. HUDLayoutTool/HUDRuntimeView/Level_ValtanArena/MainApp 재컴파일 로그 확인 |
| 후속 Debug 검증 | Protocol, Next service, Server `--contract-test`, Action, Effect, PointLight 모두 exit 0 |
| 최신 Effect native | Debug fixture 재빌드 및 Debug/Release 실제 실행 모두 exit 0 |
| 08:49 이후 추가 저장된 9시 문서 | 104개 focused tests(기존 skip 7)·Effect source 검사 통과. 검사 전후 SHA256 동일 |
| Debug 전체 live 재실행 | 사용자 Debug Server/Client가 실행 중이므로 이번에는 실행하지 않음. 08:14 전체 Debug 통과 이력과 구분 |

Release 검증 중 08:49:07에 9시 Effect의 landing delay(3.5→3.4000001)와 반원 yaw(-180→0)가
추가 저장됐다. 이는 PR 병합 이후 로컬 편집 1파일이며 이 문서 커밋에 함께 넣거나 되돌리지 않는다.
전체 Release 실행 전후 모든 입력이 동일했다고 주장하지 않는다. 후속 focused 검사와 Effect native
D/R에서 현재 저장값을 다시 확인했고, 이 세션은 해당 회전/Element 값을 직접 수정하지 않았다.

08:49 사용자 Debug Server PID 76684와 Client PID 78808은 그대로 유지했다.
Release child의 실제 exit 0을 관찰한 뒤 에이전트의 순차 실행 driver만 종료해, 이어질 Debug 빌드가
사용 중인 출력물을 덮어쓰려 하지 않도록 했다. 이 driver의 종료 exit 1은 컴파일·Release 실패가 아니다.
후속 Debug Server 검사는 listener를 열지 않는 `--contract-test` 경로만 사용했다.

UI PR의 RaidClear 이미지 370개와 사운드 1개는 Git에 들어 있지 않고 Desktop에도 없다.
사용자가 별도로 받겠다고 확인했으므로 임의 대체 자산을 만들거나 이번 Valtan 소스 PR에
추측한 파일을 추가하지 않았다. UI 실행·시각 판정은 여전히 사용자 소유다.

검증 로그는 Desktop `_work/valtan-arena-next/` 아래에 있다.

- `desktop-main-sync.result.json`: PR #253 이후 main pull, ahead/behind 0/0.
- `desktop-postmerge-canonical-Release.log`, `desktop-postmerge-release-observer.json`: 정본 전체 Release와 실제 exit 관찰.
- `desktop-postmerge-debug-focused.result.json`: 사용자 Server와 겹치지 않은 Debug 계약·native 결과.
- `desktop-postmerge-effect-native.result.json`: 최신 fixture Debug 빌드와 D/R native 실행.
- `desktop-postmerge-latest-save-focused.result.json`: 후속 9시 저장값 검사와 전후 hash.
- `desktop-raid-clear-resource-presence.json`: UI 미전달 resource 목록.

## G08. 저장 Flow와 실제 Product 발탄 연결

기존 Boss Tool은 `ValtanBossAuditionFlows.json`을 저장했지만 Product 재생 순서는 별도
`Valtan.gameplay.json`의 inline `scriptedSequence.patternIds`에서 읽었다. 따라서 Tool에서
수정한 순서를 저장해도 실제 발탄의 다음 실행에는 기존 순서가 남았다. Next도 isolated audition의
epoch가 있어야만 열려 Product 발탄이나 Ordered Flow를 보는 상태에서는 선택할 수 없었다.

현재 Product의 `scriptedSequence`는 `flow.valtan.boss-tool.default`를 참조한다. 고정 위치의
저장 Flow를 publisher와 Client Pattern Tree가 같은 규칙으로 검증·해석하며, 생성된 Product
순서는 저장 배열과 일치한다. stable slot ID가 희소하거나 같은 pattern ID가 두 번 나와도 순서를
바꾸거나 중복을 제거하지 않는다. 현재 22슬롯과 `nextSlotOrdinal=31`은 편집하지 않았다.
main의 CROSS 추가도 보존해 현재 split 30 / 공용 Tool 28 / Product 54 정의다. CROSS를 사용자
저장 Flow에 자동 삽입하지 않았고, 기존 inventory 소비자와 검사의 기대값만 현재 main에 맞췄다.

저장 파일의 SHA256은
`3a0e831a8454ef09ae05c1023b251d607cce60d0702e3cb732fb7e9b952b79d5`이다.
기본 Flow의 raw revision을 source manifest와 immutable authoring snapshot에 포함한다.
candidate는 자기 snapshot의 Flow만 읽으며 현재 workspace 파일로 조용히 대체하지 않는다.
잘못된 version, 중복 slot, 없는 pattern/flow, 빈 기본 Flow, 누락 파일은 기존 상태를 보존하고 거부한다.
구형 immutable inline sequence 입력은 계속 읽을 수 있지만 inline과 flow reference 혼합은 거부한다.

`Save Flow`는 파일 저장 후 공용 `CValtanTuningCommandService`를 통해 기존 PublishV2와
Server 2PC에 연결된다. Balance Tool의 Apply도 같은 서비스와 요청 sequence를 사용한다.
파일의 `SAVED`와 Server의 `COMMITTED`/`ALREADY_ACTIVE`를 구분하고, publish 또는 apply 실패 시
저장 파일과 이전 runtime을 유지한다. `Apply Saved Flow`로 저장된 revision을 다시 적용할 수 있다.
도구 창을 닫아도 MainApp이 요청 진행·실패를 소비한다. 120초 지연은 미확정 경고로 표시하며,
내부 Product publisher도 강제 종료하지 않아 durable commit/rollback을 끝내게 한다.

실행 중 Product sequence는 패턴 사이의 대기와 마지막 idle까지 같은 catalog revision을 유지한다.
새 encounter 또는 명시적 reset부터 최신 저장 순서를 소비한다. 실행 도중 다음 slot만 다른 revision의
정의로 바꾸지 않는다. `VALTAN_WHIRLWIND`와 `VALTAN_SEQUENCE_WHIRLWIND`는 서로 다른
정의로 유지했으며, 취소된 Ordered Slots의 현재/다음 표시 행은 추가하지 않았다.

## G09. Next·Reload와 요청한 두 시간 조정

Shared protocol은 `NETWORK_PROTOCOL_VERSION`의 현재 값이다. Product, 본인 소유 Flow, idle에서 관찰한 실제 boss occurrence를
`QUEUE_NEXT_LIVE_PATTERN_ID`로 예약할 수 있다. 기존 isolated Next의 epoch/token 검증은 유지한다.
다른 소유자, stale occurrence, 잘못된 ID는 거부하고 정확한 재전송만 멱등 처리한다.
현재 패턴의 마지막 hit/world commit과 COMPLETED 뒤 다음 fixed tick에 예약 패턴을 시작한다.
Next는 HP·위치·파괴 상태를 reset하거나 플레이어를 자동 부활시키지 않는다.
Flow 도중 Next는 현재 패턴만 유지하고 남은 재생 순서를 예약 패턴으로 교체하며, 저장 파일은 변경하지 않는다.

`Reload Flow`는 저장 파일을 다시 검증한 뒤 **화면의 첫 배열 원소 01**을 선택해 재생한다.
문자열 `slot.000001`이 존재한다고 가정하지 않는다. 본인 실행을 교체할 때에도 전체 preflight가
성공하기 전에 기존 실행을 지우지 않는다. Client는 제출 대기와 Server가 승인한 snapshot을 분리하고
거절·송신 실패·지연 응답 시 기존 실행 identity를 보존한다. 완료 hold를 reset한 경우에도 정확한
ABORTED lifecycle을 보내 Reload 이후 오래된 Next epoch가 남지 않도록 했다.

| 요청 | 실제 변경 | 유지한 값 |
|---|---|---|
| 중앙 이동 후 6방향 피자 첫 착지 | `VALTAN_SIX_PIZZA_106.serverMotion.travelEndMs` 700 → 267 | 상승 800~1100ms, 높이 10, STEP_03 1200ms, 이후 점프와 animation clip/rate |
| 점프 후 플레이어 추적 도끼 중간 대기 +2.5초 | `VALTAN_HIGH_JUMP/AIRBORNE.durationMs` 4000 → 6500 | wave 0/1333/2666ms, 각 도끼의 1200ms 단일 hit, TAKEOFF/LAND 이동 창 |

도끼 중간 표현은 기존 `LOOP_TO_STAGE_END`이므로 presentation에 임의의 빈 clip을 추가하지 않고
실제 Server 단계 시간을 조정했다. 기존 publisher가 owner 단계에서 파생하는 target-axe `lifeMs`도
4000 → 6500이 된다. 추가 wave·추가 hit는 만들지 않는다. 구형 4000ms immutable candidate를
거부하지 않도록 admission의 최소 4000ms 제약은 유지한다.

저작 정본 수정 뒤 publisher로 `ValtanEncounter.json`, `ValtanPatternRotations.json`,
`ValtanCombatObjects.json`을 갱신했다. 생성 bootstrap을 직접 수정하지 않았다.
공유 regeneration helper도 같은 수치를 사용하도록 수정했다. 사용자 Effect 3문서와 맵 emissive,
맵 runtime 산출물의 별도 미커밋 변경은 이 기능의 편집·커밋 범위에서 제외한다.

## G10. 현재 자동 검증 증거

| 검사 | 현재 관찰 결과 |
|---|---|
| main 동기화 전 focused Python 7개 모듈 | 210 tests, 실패 0, 기존 skip 7, 335.256초 |
| main 동기화 후 focused Python 6개 모듈 | 151 tests, 실패 0, 기존 skip 7, 5.049초. MasterV2 59개는 정본 자동화에서 별도 실행 |
| main 동기화 후 MasterV2 | 59 tests, 실패 0, 300.162초. 실제 publisher 호출의 timeout 강제 종료 방지 검사 포함 |
| 실제 Client 명령 서비스 Debug harness | Next 22 + Flow 10 + 저장 적용 13, 총 45개 통과 |
| NetworkProtocolHarness Debug | 현재 `NETWORK_PROTOCOL_VERSION` 포함 failures 0 |
| 최신 Server Debug 계약 실행 | source/main 통합 및 fixture 수정 후 실제 `Server.exe --contract-test` exit 0, failures 0 |
| 정본 Debug 빌드 | Engine/Shared/Server/Client 및 모든 native harness 컴파일·링크 통과 |
| 정본 Debug 전체 회귀 | main의 신규 Effect 7파일 등록 수정 뒤 `-SkipBuild` 전체 재실행 exit 0. protocol, 서비스 45건, Server, 4인/Character Select 격리, animation, Effect, PointLight 하네스까지 완료 |
| 정본 Release 전체 build/regression | 초기 include 순서/산출물 점유 문제 해결 뒤 현재 Desktop 소스로 최종 재실행 중. 완료 전 전체 PASS로 기록하지 않음 |
| main 통합 후 Product ValidateV2와 Server pre-build publish | sourceManifest `fda56c3235e494732994d30b4b4e4442dc48cbcbdbce65d826c483dc46dee4df`, 54 patterns / 235 stages, 검증·publish 통과 |
| 실제 PublishSavedFlow | candidate `1ad74e01050a662b9b28d07f3a96d4f0e44941b0f6941b51f176bca9711feb78`, HOT_RELOAD, split join 검증 통과, activeRuntimeChanged=false |
| Effect 프로젝트 등록 | 정본 생성기로 main의 V2 JSON 7개 None 항목만 추가. files 2405 / filters 219, Check 통과 |
| UI/시각 확인 및 실제 사용자 Server 2PC | 사용자가 직접 실행·판정해야 하며 에이전트 PASS 아님 |

첫 Server 실행의 실패는 단계를 늘리기 전의 고정 `lifeMs=4000` 기대값, direct Brain fixture의
누락 catalog generation 두 건, 완료 hold reset으로 추가된 ABORTED lifecycle 기대값 한 건이었다.
production의 실패 검사를 완화하지 않고 실제 새 계약의 identity와 시간을 검사하도록 fixture를 고쳤다.
수정 후 Server 전체 계약을 failures 0으로 재검증했다. 이후 Debug 정본 회귀는 신규 Effect 파일의
project/filter 등록 누락을 검출했다. 기존 C++ 등록과 filter GUID/순서를 유지하면서 누락된
`boss.valtan.hand_1`~`hand_6` 및 `BOSS_VALTAN.effectv2bindings.json`만 정본 생성기로 등록했다.
이는 사용자 authored Effect 내용을 변경하지 않는다.

수정 뒤 정본 Debug 회귀는 `Regression completed: Debug`와 실제 process exit 0을 확인했다.
이 실행의 MasterV2 59개는 323.157초, 실패 0이며 나머지 Python 검사와 native/네트워크
하네스도 모두 완료했다. Release 검증은 별도 폴더의 다른 작업과 출력물을 공유하지 않는다.
또한 공식 스크립트 소스를 바꾸지 않고 PowerShell 기본 인자로 테스트 전용 `HarnessPort=18787`을
전달해 콘솔 네트워크 하네스의 포트 충돌을 피한다. 제품 LAN endpoint는 변경하지 않는다.

최신 candidate의 immutable `Authoring/Valtan.gameplay.json`에서 AIRBORNE 6500ms와 첫 착지 267ms를
읽고, snapshot-local Flow의 raw bytes 및 22개 Product 순서가 현재 저장본과 같음을 확인했다.
WHIRLWIND 두 occurrence도 보존한다. `Intermediate/ValtanTuningRuntime`은 없으며 사용자 Server의
실제 2PC를 임의 실행하거나 활성 runtime pointer를 바꾸지 않았다.

로그는 `Intermediate/ValtanFlowReloadNextFix/` 아래에 있다.

- `focused-python-final.log`: 210개 focused 테스트.
- `focused-python-post-main.log`: main 통합 후 151개 focused 테스트.
- `debug-build-regression.log`: 첫 정본 Debug 빌드, 구형 Server fixture 4건 실패.
- `debug-build-regression-final.log`: main 통합 후 전체 Debug 빌드와 Python 회귀 통과, Effect 등록 누락 검출.
- `debug-regression-complete.log`: Effect 등록 수정 후 정본 Debug 전체 회귀 exit 0.
- `post-main-publish.log`: 두 작업의 split/Product 결합이 이미 일치해 changed=0인 PublishV2 결과.
- `server-debug-contract-first.log`: fixture 수정 전 실제 Server 계약 결과.
- `server-debug-contract-final.log`: main 통합과 fixture 수정 뒤 Server failures 0.
- `saved-flow-candidate-final.log`, `saved-flow-candidate-final-inspection.json`: 실제 후보 생성과 저장 순서·시간의 snapshot 검증.
- `release-build-regression.log`, `release-build-regression-2.log`, `release-build-regression-3.log`: 최초 실패 기록.
- `release-build-regression-final.log`, `release-build-regression-final.result.json`: 현재 Desktop 소스의 최종 Release 실행 로그와 종료 관찰 결과.

## G11. 사용자 실행 경계와 남은 확인

main 동기화 전에 소유 변경 60개만 safety stash
`bcbf6aba274e4fcee9c2dc3adf19f2ae74dffe81`로 보존했다. protected 8파일은 stash에 넣지 않았고
동기화·복원 전후 raw SHA256이 같다. 충돌은 provenance receipt 1파일뿐이었다. main의 pattern
count 54와 이 작업의 AIRBORNE/object life 6500을 의미 단위로 합쳐, main 대비 숫자 4줄만 바뀐다.
stash는 삭제하지 않았고 현재 index에는 미검증 변경을 올려 두지 않았다.

이 PC의 LAN 설정은 `server-host`이며, endpoint는 `192.168.0.20:7777`이다.
에이전트는 Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 콘솔 계약 하네스만 실행했다.
사용자는 빌드 완료 뒤 `Framework.sln`의 **Server + Client** profile을 `Ctrl+F5`로 시작한다.
Client 작업 디렉터리는 `Client/Default`다.

1. Lobby → Valtan → F1 → Boss Tool → Pattern Flow에서 순서를 저장한다.
2. 파일 `SAVED`뿐 아니라 runtime `COMMITTED` 또는 `ALREADY_ACTIVE`를 확인한다.
3. `Reload Flow`가 화면 01에서 시작하는지 확인하고, 재진입/다음 실행의 실제 발탄 순서를 확인한다.
4. 실제 Product/Flow 도중 Next Pattern을 예약해 현재 패턴 완료 뒤 reset 없이 연결되는지 확인한다.
5. 추적 도끼 중간 대기 6.5초와 피자 첫 착지의 체감 속도는 사용자가 최종 판정한다.

현재 자동 검증과 Git 반영을 마무리하기 전이므로 이 절은 main 병합 완료나 사용자 visual PASS를 의미하지 않는다.

## G15. 고정 개수 제거와 공용 패턴 목록 최종 결과

### 구현 결과

Boss Tool이 받아야 하는 패턴 수를 29개나 다른 특정 숫자로 제한하던 계약을 제거했다. 현재 선택 목록의
정본은 `Data/Valtan/Valtan.gameplay.json.patterns`와 같은 stable `patternId`의 presentation을 strict join한
결과다. `CValtanPatternTree::Build_PlayablePatternInventory`가 이 결과를 한 번 만들고 Boss Verification의
`Play Selected`, `Repeat`, Pattern Flow의 추가 목록, `Next Pattern...`, Effect Tool의 Valtan `All Effects`가
같은 membership을 사용한다. Core/Animator/Derived의 개수는 화면 표시와 분류 결과일 뿐 admission 조건이 아니다.

Python Product/Flow resolver도 고정 Core ID 배열과 20/26/28/29/31개 equality를 사용하지 않는다. gameplay와
presentation에 같은 stable ID가 있고 저작 소유자가 유효하면 현재 개수와 무관하게 포함한다. promotion
`patterns`가 0개인 유효한 manual lineage도 허용하고 derived 목록은 특정 ID 집합이나 개수로 비교하지 않는다.
한 Flow의 용량은 문서·publisher·Product·Client packet·Server catalog가 공유하는 1~255슬롯 U8 계약이며
256슬롯은 transactionally 거부한다. 이 값은 전체 등록 패턴 수 제한으로 사용하지 않는다.
frozen v1 identity와 sealed source-occurrence 33/32/1 수치는 migration 증거만
검사하며 live 목록의 상한으로 해석하지 않는다.

고정 개수 제거 뒤 다른 작업의 패턴 확장까지 합쳐 현재 ValidateV2는 `managedPatterns=33`을 읽었다. 31개에서
33개로 늘어난 데이터에 추가 count patch나 C++ allowlist 변경 없이 Client Debug/Release 빌드, 공용 inventory
native 검사와 Product Validate가 통과했다. 이는 이후 패턴 확장도 같은 stable-ID 계약으로 합류할 수 있다는
실제 현재 데이터 증거다.

### 저장 Flow, Reload와 Next의 최종 의미

저장 기본 순서의 정본은 `Data/Encounters/Valtan/ValtanBossAuditionFlows.json`의
`flow.valtan.boss-tool.default`다. `Save Flow`는 배열 순서, 반복 pattern occurrence, sparse slot ID와
inter-step pursuit를 그대로 저장한 뒤 기존 Product publisher와 Server revision apply 경로를 사용한다.
적용된 기본 순서는 다음 encounter 또는 명시적 reset부터 Boss Verification이 관찰하는 실제 발탄에 사용된다.
진행 중 sequence는 시작 때 pin한 이전 revision을 끝까지 유지한다.

`Reload Flow`는 저장 문서를 다시 검증하고 slot ID 숫자를 정렬하지 않은 채 JSON 배열의 첫 원소, 즉 화면 01부터
FLOW_START를 제출한다. `Next Pattern...`은 현재 Product/Flow/isolated pattern 또는 idle 다음에 한 패턴만 예약한다.
현재 occurrence가 완료된 다음 fixed tick에 reset 없이 실행하며 남은 Flow는 교체하지만 저장 배열은 수정하지 않는다.

현재 저장본은 26슬롯, `nextSlotOrdinal=38`, 첫 패턴 `VALTAN_WHIRLWIND`, 마지막 패턴
`VALTAN_GHOST_FINALE`이며 SHA256은
`0c6d6da3a16eebe2985806ac737fe498cca388a4e84ddff419e1b6d500768fcd`다. 이번 작업은 이 순서와 사용자의
Effect/맵 저장값을 자동 보충, 재정렬 또는 되돌리지 않았다.

### 최종 2중 감사에서 닫은 실패 경로

| 감사 항목 | 최종 처리 |
|---|---|
| entry-only cinematic을 제외한 native expected count | 별도 합산을 없애고 공용 playable inventory의 실제 크기를 사용한다. |
| Python의 live gameplay 31개 고정 검사 | gameplay/presentation stable-ID 집합의 non-empty, unique, exact equality 검사로 교체했다. |
| graph reload가 열린 Flow 검증 전에 새 graph를 commit | staged graph로 inventory를 만든 뒤 현재 Flow를 먼저 검증하고, 성공했을 때만 graph/inventory를 함께 commit한다. 실패 시 이전 목록과 Flow를 유지한다. |
| entry cinematic을 첫 슬롯 밖으로 이동 가능 | slot 이동을 복사본에서 수행하고 전체 Flow 검증 뒤 commit한다. 실패하면 기존 draft가 그대로다. |
| manual promotion 0개 거부 | `patterns`는 배열이어야 한다는 형식만 선행 검사하고, 빈 집합을 포함한 실제 manual/promotion exact lineage가 최종 admission을 결정한다. |
| 저장 Flow와 무관한 과거/IDLE tuning snapshot으로 Start 가능 | 현재 저장 revision, terminal apply 결과, candidate revision, 현재 connection/world generation과 live Server active revision이 모두 정확히 일치할 때만 Start First/Here와 Reload 재시작을 허용한다. cached 표시값만 신뢰하지 않아 `Update()` 전 revision drift도 즉시 fail-close한다. |
| 디스크의 빈 Flow가 Reload에서 기존 유효 문서를 교체 | persisted document 검증에서 0슬롯을 거부한다. Load/Reload는 staged 검증 전에 baseline/draft/revision을 쓰지 않으며 native 하네스가 실패 뒤 기존 dirty draft와 source revision 보존을 확인한다. 편집 중 마지막 slot 제거 후 다시 추가하는 임시 draft는 유지된다. |
| manual/derived 패턴 목록을 Python 상수로 고정 | Animation Tool과 Pattern Tree 테스트의 전체 ID/name allowlist를 제거하고 decision lineage, promotion manifest, debug source chain, gameplay/presentation stable-ID join을 동적으로 검증한다. 빈 manual promotion도 허용하며 derived는 특정 전체 목록/개수를 요구하지 않는다. |

entry cinematic은 사용할 경우 배열 첫 슬롯에 정확히 한 번만 둘 수 있지만 생략해도 자동 삽입하지 않는다.
일반 패턴의 반복 occurrence는 허용한다. graph, Flow 또는 publisher 검증 실패는 기존 유효 graph, 저장 문서와
실행 revision을 보존한다.

### 발탄 패턴 완성 작업의 확장 규칙

1. 새 패턴은 gameplay와 presentation에 같은 stable `patternId`를 등록한다.
2. 자동 선택/기믹 소유자 또는 `manualAuditions`의 `MANUAL_SERVER_AUDITION`/
   `DERIVED_SERVER_PATTERN`으로 실행 소유권을 정확히 하나 선언한다.
3. 기존 clip 조합도 독립 패턴이면 새 pattern/action/stage/occurrence ID를 사용한다. 단순 재생 순서 조합은
   Pattern Flow slot으로 표현한다.
4. 새 Server 행동 종류가 없다면 목록 개수, C++ allowlist, schema `maxItems`를 고치지 않는다. 새 행동 종류가
   필요할 때만 typed Server 실행 계약과 해당 harness를 함께 확장한다.
5. Boss Tool에서 Ordered Slots를 조절하고 `Save Flow` 뒤 `COMMITTED` 또는 `ALREADY ACTIVE`를 확인한다.
   다음 실행/reset은 저장 순서를 사용하며 `Reload Flow`는 배열 01부터, `Next Pattern...`은 저장 비변경 one-shot으로 동작한다.
6. 패턴 전체 수에는 별도 고정 상한을 두지 않는다. 한 Flow는 1~255슬롯을 허용하며 256슬롯은
   저장·Product 투영·Client 전송·Server catalog에서 기존 유효 상태를 보존한 채 거부한다.

### 최종 검증

| 검사 | 결과 |
|---|---|
| 최종 공용 inventory/Flow/Next/Effect Python 회귀 | 현재 26-slot 저장본 기준 156 tests PASS, 기존 skip 7 |
| Python compile 및 관련 JSON parse | PASS, 4개 스크립트 compile / 4개 JSON parse |
| EffectRenderContractHarness Debug/Release focused 빌드 | 컴파일·링크 성공, errors 0 |
| native `--validate-valtan-pattern-inventory` Debug/Release | `validated=true` |
| ValtanPatternAuditionServiceHarness Debug/Release | audition 23/23, Flow 12/12, tuning command 13/13 PASS. 255 허용/256 무전송 원상 보존 포함 |
| NetworkProtocolHarness Debug/Release | failures 0. 33/255 roundtrip, wire `0xff`, 256 writer 거부, 64 KiB frame budget 검사 포함 |
| Project-ValtanPatternMaster ValidateV2 | PASS, managed 33 / legacy 26 / projected artifacts 9 |
| Gameplay balance Validate | PASS, boss patterns 57 / stages 255 / Valtan audition rows 52 |
| Client Release 전체 빌드 | 최신 `BossTool`/Flow/Tree 포함 컴파일·링크·배포 성공, errors 0 |
| Server Release 전체 빌드 | 최신 catalog 및 255/256 계약 테스트 포함 컴파일·링크 성공, errors 0 |
| 변경 범위 `git diff --check` | PASS |
| MasterV2 | 72개 중 71개 정상 완료. PTY 출력이 오류 문구 중간을 줄바꿈해 문자열 비교 1개만 실패했으며 같은 test를 non-PTY로 즉시 재실행해 PASS |
| source occurrence 동적 coverage focused | 3 PASS, `jsonschema` 미설치 1 SKIP |
| 최신 Server Debug/Release contract | 새 255-step load와 256-step atomic reject는 두 구성 모두 PASS. 전체 suite는 `Finish the lethal floor wipe...` 시나리오 1건이 두 구성에서 동일하게 실패하여 전체 PASS로 기록하지 않음 |

전체 source-occurrence suite는 현재 별도 FIST reviewed selection이 제거된 product clip occurrence
`valtan.attack.fist-in-out.windup.clip.01`을 참조해 setUp에서 중단된다. 동적 stable-ID coverage focused 검사는
통과했으며 이 외부 selection drift를 이번 Flow 작업에서 수정하지 않았다. 전체 Effect harness의 일반 모드는
보호 중인 `effect.valtan.sequence.charge.effect.json`의 빈 override 때문에 별도 차단되지만, 이번 변경을 직접
검사하는 focused native 모드는 Debug/Release 모두 통과했다.

에이전트는 Client/UI를 실행하거나 시각 결과를 PASS로 판정하지 않았다. 공유 dirty worktree의 다른 작업은
stage, commit, reset하지 않았으며 이 결과는 현재 소스와 자동 검증 상태를 기록한다.

최종 감사 로그는 `Intermediate/ValtanFlowReloadNextFix/`의
`inventory-effect-harness-debug-final-build.log`, `inventory-effect-harness-debug-final-run.log`,
`inventory-effect-harness-release-final-retry-build.log`, `inventory-effect-harness-release-final-run.log`,
`inventory-audition-flow-harness-release-final-build.log`, `inventory-audition-flow-harness-release-final-run.log`,
`inventory-project-validate-v2-final-audit.log`, `inventory-gameplay-balance-validate-final-audit.log`에 남겼다.
최종 현재 저장본 Python 156개 결과는 `slot-expansion-focused-python-current-flow-final.log`에 있다.

## G16. 255-slot Debug 반영 재확인

사용자가 처음 확인했을 때 실행 중이던 Debug Client는 출력 파일을 점유하고 있어 최신 링크 결과로
교체되지 않은 상태였다. 이후 사용자가 다시 시작한 현재 `Client/Bin/Debug/Client.exe`의 파일 시각은
2026-08-28 13:30:01이고 process 시작은 13:30:02다. 현재 실행 바이너리에서
`Valtan Boss Flow cannot add this pattern or has reached 255 slots.` 문자열도 직접 확인했다.
따라서 **현재 실행 중인 Debug Client에는 255-slot 확장이 반영되어 있다.** Ordered Slots는 빈 행 255개를
미리 표시하지 않고 `Add from All Effects...`로 추가할 때마다 늘어나며, 255개에서만 추가를 거부한다.

현재 Debug Server도 최신 source 이후 생성된 13:23:02 바이너리를 13:30:02에 시작했다. Product sequence,
publisher, packet, Client document/service와 Server catalog는 모두 1~255를 공유하며 256 입력은 기존 문서,
pending command와 active catalog를 바꾸지 않고 거부한다. 전체 등록 패턴 수에는 별도 29개 상한이 없다.

13:21에 다른 발탄 패턴 작업이 저장 Flow를 26슬롯으로 확장한 사실을 마지막 검증에서 다시 관찰했다.
이 세션은 그 파일을 되돌리지 않았고 26개 exact 순서로 focused Python 156개, ValidateV2, Gameplay Balance
Validate와 Debug/Release inventory native를 다시 통과시켰다. Client/UI 화면의 실제 조작감과 순서 재생은
사용자가 직접 판정하며 이 문서는 visual PASS를 대신하지 않는다.
