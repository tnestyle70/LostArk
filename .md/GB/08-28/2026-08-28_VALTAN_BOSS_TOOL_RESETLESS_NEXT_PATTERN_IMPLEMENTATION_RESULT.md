# Valtan 파괴 벽·돌진·Next·Trash 정본 반영 결과

작성일: 2026-08-28.

## G00. 현재 적용 위치와 검증 상태

이 세션과 관련 Effect 작업의 구현을 **Desktop LostArk 정본 작업 트리**에 보존 통합했다.
사용자 Debug 빌드의 컴파일 오류 없음 확인 뒤 Release 빌드와 자동 회귀를 재개했다.
Desktop의 Server Release와 모든 검증용 실행 파일의 Debug/Release 빌드는 통과했다.
Server 계약은 두 구성 모두 failures 0이며, Release 4인 접속·격리·2인/4인 파티 이동도 통과했다.
Client Release 첫 링크의 old/new object 혼재 오류를 최신 소스 재빌드로 해소했다.
최신 Client·Server Debug/Release 빌드와 회전 수정의 실제 Server tick 회귀는 모두 통과했다.
정본 전체 Debug 자동화는 08:14 exit 0으로 완료했다. 뒤이어 실행한 Release 전체 검사는
사용자가 저장한 Effect element 삭제와 기존 고정 fixture의 불일치에서 중단됐다.
저장값은 유지하고 fixture를 수정한 뒤 아래 104개 focused 검사를 통과했다.
UI PR #252 통합 뒤의 전체 Release 재검증은 아직 완료하지 않았다.
사용자 시각 검증은 대신 완료 처리하지 않는다.

| 항목 | 실제 상태 |
|---|---|
| 최종 솔루션 | `C:/Users/user/Desktop/LostArk/Framework.sln` |
| 현재 브랜치 | `codex/valtan-arena-next-desktop` |
| 현재 기준 HEAD | `a6871a2fddd2a9e1c06091cb4e8cfdde77d6b024`, PR #251 병합 main |
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
| Desktop Client Debug·Release 최신 재빌드 | PASS, 각 exit 0. 등록 정렬 수정 뒤 08:05 재확인에서도 3363개 입력 hash 동일; `desktop-final-client-Debug.log` / `desktop-final-client-Release.log` |
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
| Desktop 전체 Debug 정본 자동 회귀 | PASS, 08:05:45~08:14:23, exit 0. 검사 전후 3363개 입력 hash 동일 |
| Desktop 전체 Release 정본 자동 회귀 | 08:14~08:20 실행 exit 1. 실행 중 floor-wipe 19→18개 등 사용자 저장이 발생해 기존 saved-rows fixture와 불일치. 최신 저장값을 보존한 focused 회귀는 아래에서 재확인; 전체 Release 완료 아님 |
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

## G05. 남은 검증과 문서

다른 작업의 최신 Effect 시작 기준·빈 발악 Draft와 이번 점프찍기 휠윈드 방향 수정을 포함해
영향받은 대상을 D/R로 재빌드했다. Desktop 정본 전체 Debug는 통과했고, 이후 사용자 저장 변경을
반영한 전체 Release 및 UI PR #252 통합 후 빌드·회귀는 별도 확인이 필요하다.
사용자는 08:13에 정본 Debug Server/Client를 다시 실행했다. 이 사용자 process는 그대로 두고,
자동 live 검증은 별도 구성·격리 포트와 에이전트 소유 process로만 수행한다.
완료되지 않은 Release 전체 검사와 사용자 시각 검증을 PASS로 기록하지 않는다.

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
  실제 첫 birth 이후 유효 수명 안으로 계산하도록 보완했으며, 이 최신 native fixture의 재빌드는 아직 별도다.

로그: `_work/valtan-arena-next/desktop-git-ready-focused-tests.log`,
`desktop-git-ready-source-validation.log`, `conflict-preflight-20260828-083117/result.json`.
이 결과는 UI PR #252 통합 후 전체 빌드나 사용자 육안 검증을 대신하지 않는다.
