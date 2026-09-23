# 쿠크 Collider 상세·상승 원통 구현 결과

## G00. 반영 범위

Collider 선택 시 Pattern의 Display Name·Parent·Category 편집을 표시하지 않고
`1. Lifetime / 2. Motion / shape / 3. Trigger / 4. Logic` 선택 화면을 연다.
시작·수명, offset·rotation·scale, anchor·bone과 종료 위치·크기는 1~4 선택과 관계없이 항상 표시한다.
기존 preset에 Cylinder를 추가했으며 Shape 변경은 선택 박스의 resource만 분리한다.

수명 동안 시작/끝 위치와 크기를 선형 보간한다. 원통은 실제 높이를 검사하고 자체 이동
원통·박스는 Server fixed tick 사이의 통과도 검사한다. Client wire도 같은 보간을 사용한다.
`Keep bottom fixed while growing`은 시작 바닥 높이를 유지하도록 끝 중심을 계산한다.
`Follow anchor=false`인 BOSS Collider는 Logic 시작 틱의 위치·방향을 보존한다.
플레이어를 향하는 초기 보스 회전은 기존 회전/추적 Logic으로 설정한다.

접촉은 플레이어별 단발·재진입·넉백 종료 후 반복·일정 간격 중 하나다. 일정 간격은
Collider 수명 끝에서 추가 타격하지 않는다. 기존 Logic을 선택한 것만으로 반복 정책을
바꾸지 않도록 입력 변경 여부를 구분했다. Damage Result는 상승 높이와 비행 시간을
따로 설정하며 수직 상승도 지원한다. 낙사 비허용은 navigation·collision 경계 안에서
착지한다. Gate1 Raid와 관문 ID가 생략된 단독 재생 모두 낙사를 제한한다.

카드 병정의 패턴 종료·30초 강제 소멸을 제거했다. 기존 몬스터 AI가 계속 추적·공격하고
자기 사망·소환자 사망/소멸·방 정리에서 제거한다. 방 상한48마리를 유지한다.
뿅망치는 기존 BODY/WEAPON 본 bake 경로를 사용하며 Follow=true를 유지한다.

## G01. 코드와 저장 계약

Workbench, Composition codec, PresentationPlayer/HitAreaWire, 기존 Composition projector,
Gameplay publisher/catalog와 Kouku Logic·Player knockback 소비자를 함께 수정했다.
`colliderMotion`, `colliderEndPositionOffset`, `colliderEndScale`, `repeatIntervalMs`,
`pushHeightM`을 기존 저장 경로에 추가했다. 기본 STATIC은 기존 동작을 유지하며,
이동을 껐을 때도 이미 조절한 종료값은 저장·재로드로 보존한다.

제품 C++ 신규 파일은 없어 project/filter 추가가 필요하지 않다. 기존 C++ UTF-8/CRLF를
보존했다. 사용자 Composition·Effect JSON은 이 작업에서 수정하거나 게시하지 않았다.
작업 시작 시 존재하던 다른 문서와 Composition의 미커밋 변경은 그대로 보존했다.

## G02. 자동 검증

최종 Debug Product 컴파일·링크·배포가 통과했다. 실행 명령은
`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`이며,
최종 receipt는 `out/BuildPipeline/runs/20260923T012238426Z-debug-product.json`이다.
빌드 로그는 `out/KoukuColliderDetail-product-debug-final-pass.log`에 있다.
최신 Client/Server 실행 파일을 갱신했고 이 빌드에서는 데이터를 게시하지 않았다.

최신 Server의 `--kouku-object-overlap-contract-test`는 failures 0이다.
상승·하강·바닥 고정 성장, 33ms 사이 얇은 콜라이더 통과, 플레이어별 반복 간격과 수명 끝,
생성 시 위치·방향 고정, Mario 특수 소비자의 고정 anchor·높이, 수직 발사·상승 높이·착지,
빈 Scope 관문 ID로 재생한 Gate1의 펜스를 검사했다. 실제 bootstrap parser로 새 행을
읽고 잘못된 종료 크기·속이 빈 원통 입력을 거부하면서 이전 catalog를 보존함도 확인했다.
로그는 `out/KoukuColliderDetail-overlap-debug.log`에 있다.

`--card-maze-contract-test`도 failures 0이다. 카드 병정의 30초 이후 유지, 다음 패턴에서
추가 생성, 패턴 정리 후 유지, 소환자 사망 시 정리와 무관한 몬스터 보존을 검사했다.
로그는 `out/KoukuColliderDetail-card-maze-debug.log`에 있다.

Python 최종 집중 검사 5개가 통과했다
(`out/KoukuColliderDetail-projection-focused.log`). 실제 Composition codec의
파싱·동등성·Save_Atomic·revision·재로드·잘못된 입력 보존 17개 검사도 통과했다.
실제 Gameplay publisher 본문에서 새 tick/motion/원통/상승 행 생성이 통과했다.
Codec과 publisher 증거는 `out/KoukuCollider20260923/schema/`에 있다.

첫 Product 빌드의 원통 overlap overload 오류는 명시 타입으로 수정했다. 집중 테스트에서
발견한 기존 Logic 선택 시 반복 정책 덮어쓰기와 Mario의 생성 anchor 소비 누락도 수정하고
재검증했다. Gate1 통합 fixture에는 실제 제품 재생과 같은 source revision pin을 추가했다.

광역 projector 회귀는 현재 설치 데이터 의존 fixture의 기존 애니메이션 높이 기대값과
playAllPatternIds 조건 불일치로 전체 통과하지 못했다. 이를 새 Collider의 성공 증거로
사용하지 않았으며 위 집중 검증 결과와 구분한다. Python·PowerShell 구문 검사,
기존 Client/Server 프로젝트·필터 XML parse와 변경 범위 `git diff --check`가 통과했다.

## G03. 사용자 확인

최신 Debug `Client/Bin/Debug/Client.exe`에서 F1 → Action Composition Workbench →
Collider 선택 → 2 Motion / shape → Cylinder → Movable / growing을 선택한다.
시작 크기·종료 크기를 조절하고 바닥 고정 성장은 `Keep bottom fixed while growing`을
사용한다. 이동만 하려면 크기는 같게 두고 End position의 Y를 변경한다.
3 Trigger에서 단발/반복 간격, 4 Logic에서 피해·상승 높이·비행 시간을 설정한 뒤
Apply → Save → Publish로 Server 재생에 반영한다.

Client/UI 실행·조작과 화면 판정은 수행하지 않았다. 화염기둥 Effect와 Collider의 최종
크기·위치·타이밍은 사용자가 확인한다. WorldTrack을 가진 움직임은 기존 지원 sweep을
사용하며, 이번 자체 선형 sweep은 별도 WorldTrack이 없는 원통·박스에 적용한다.

## G04. 사용자 화면 피드백 후 보완

초기 구현은 Lifetime 선택에서 transform을 숨겨 공통 필드가 사라져 보이는 회귀가 있었다.
Start/Lifetime, position/rotation/scale, anchor/bone, end position/end size를 section 조건 밖의
공통 영역으로 이동했다. STATIC의 종료값도 비활성 입력으로 계속 보인다.
Cylinder가 preset label에는 있으나 6회 반복문에서 생성되지 않던 오류를 7개 생성으로 수정하고,
잘못 7개로 늘어난 Timeline label 반복문은 기존 6개로 복구했다.

단일 Collider 상세에 Set Group/Ungroup과 Duplicate를 표시한다. 그룹 생성은 Ctrl/Shift로
기존 동일 BOSS 기준 Collider 2개 이상을 선택한다. 단일 Duplicate는 현재 상세값을 Apply한 뒤
같은 시각에 복제하고, 여러 Collider 선택 화면에도 동일 시각 복제 버튼을 제공한다.
기존 clone 경로가 연결 Logic 창과 공유 Collider를 함께 복제하고 새 occurrence/group/region
ID와 Logic 연결을 재매핑한다. 복제 후에는 새 Collider만 선택한다. Effect와 일반 Ctrl+D의
동작은 유지한다. 다른 종류의 lane까지 소유한 복잡한 연결은 기존 전체 복제를 안내한다.

현재 보완 코드는 Client ClCompile을 통과했다
(`out/KoukuColliderDetail-group-duplicate-compile-debug.log`). 별도 UI 없는 집중 하네스도
빌드와 `--kouku-collider-duplicate-contract` 실행이 exit 0으로 통과했다. 같은 시각의
CYLINDER 그룹/단일 복제, 공유 Logic 소유 연결 복제와 새 occurrence/group/region/window ID,
시작·수명·종료 geometry 유지, 원본 보존, 복제본 선택·단일 상세 focus, Save/Reopen을 검사했다.
로그는 `out/KoukuColliderDetail-duplicate-harness-focused.log`와
`out/KoukuColliderDetail-group-duplicate-harness-build-final.log`에 있다.

기존 `--kouku-collider-group-contract`는 외부 displayName/revision만 바뀌어도 저장이 거부돼야
한다는 오래된 CAS 기대에서 실패했다. 현재 Save_Atomic은 stable ID 기준 독립 필드를 병합한다.
이 기존 기대는 수정하지 않았으며 새 복제 집중 검사 성공과 구분한다. 테스트는 기존 하네스의
두 CPP에 추가해 새 제품 파일이나 project/filter 등록은 없다. `git diff --check`도 통과했다.

최초 보완 시에는 Debug Client가 기존 EXE를 점유해 설치를 대기했다. 이후 Client 종료를
확인하고 Product 빌드를 재개했으나 Visual Studio도 같은 Client의 동일 CSO들을 빌드하고
있음을 프로세스 부모 계보와 출력 경로로 확인했다. 출력 충돌을 피하기 위해 이 작업에서
시작한 MSBuild와 그 자식만 중단했고 사용자의 devenv 빌드는 유지했다.
`out/KoukuColliderDetail-ui-followup-product-debug.log`는 이 중단 시도이며 PASS가 아니다.
G04 소스 컴파일·집중 하네스 PASS와 최종 EXE 설치를 구분하며, VS 빌드 완료·새 실행 파일의
최종 화면 판정은 아직 확인하지 않았다. 데이터 게시·Client 실행·UI 조작은 하지 않았다.

## G06. 실제 화염파동 연결 후보와 메모리 Draft 재생

P58의 실제 `effect.kouku.common.flame.wave.full` 두 occurrence와 설치 Effect의
`manual.flame-wave.r1.c1` 계열 14개 그룹을 읽어 후보를 생성했다. 각 회차는 2/3/4/5개
불기둥으로 구성하며, 두 회차에 STATIC CYLINDER 28개와 행 그룹·ENTER_AREA 창 8개를
연결했다. 기존 사용자 BOX `KAKULSAYDON_G1_PATTERN_58.presentation.8`을 포함한
기존 occurrence와 무관한 데이터는 그대로 보존한다. 단일 Effect 안의 element 위치에
현재 occurrence의 scale·yaw·offset을 적용했으며, 별도 alias Effect의 정렬된 격자를 사용하지 않았다.

| 회차 | Effect 시작 ms | 행별 접촉 시작 ms | 각 행의 원통 수 |
|---|---:|---|---|
| 1 | 1904 | 3104 / 3404 / 3704 / 4004 | 2 / 3 / 4 / 5 |
| 2 | 6117 | 7317 / 7617 / 7917 / 8217 | 2 / 3 / 4 / 5 |

각 창은 500ms 동안 같은 행의 Once 판정을 공유한다. 초기 원통은 반경 1.5m·높이 6m이며
바닥에서 시작하도록 중심 Y를 3m 올렸다. 피해 10%와 비행 2161ms는 정면 바람방구의
현재 Result를 기준으로 하고, 수평 거리는 0m, 상승 높이는 기존 중력 9.8과 같은 비행 시간의
포물선 정점인 5.720653225m로 설정했다. 낙사 허용은 끄고 Gate1 펜스를 소비한다.
반경·높이·활성 시간·상승 높이는 화면 확인 전의 PROJECT_TUNED 초기값이다.

`anchorPresentationOccurrenceId`는 같은 Pattern의 fixed BOSS Effect만 참조하며,
Effect 시작이 Collider보다 늦거나 참조가 없으면 거부한다. projector는 이를 region의
`captureStartMs`와 `PATTERNLOGICREGIONCAPTURE`로 투영한다. Server는 창이 열리기 전에
해당 boss basis를 저장한다. Client도 참조 Effect와 모든 Collider에 같은 고정 프레임을 쓰고,
Server와 같은 30Hz 올림 틱이 준비되기 전에는 그 Effect를 생성하지 않는다.
1904ms는 58틱, 6117ms는 184틱에 기준을 고정하며, 각 행에서 움직인 보스를 다시 잡지 않는다.
Effect 수명이 끝나도 같은 실행의 고정 프레임은 유지한다.

그룹의 Edit member와 Back to Group은 group ID를 유지하며 개별 Box Detail을 연다.
개별 geometry와 Damage / knockback의 Horizontal 값을 편집할 수 있다. 개별 피해 설정을
바꾸면 연결 창·정의를 분리해 다른 멤버의 값을 보존한다. Existing Logic에서 명시적으로
공유 창을 선택한 경우에는 공유 결과를 유지한다. 원본 보존과 독립 Horizontal 편집은
기존 Workbench 집중 하네스에서 확인했다.

Workbench의 Server 재생은 저장·게시를 요구하지 않고 클릭 시점의 메모리 draft와 staged
geometry를 snapshot으로 복사한다. `prepare_kouku_draft_play.py`와
`Prepare-KoukuDraftPlay.ps1`은 repository/out 아래에만 임시 입력·결과를 만들며,
정식 publisher와 공유하는 `KoukuBootstrapRows.ps1`의 emitter를 사용한다.
16MiB 이하의 gameplay rows는 request sequence와 SHA-256으로 전송하고, Server의 기존
catalog parser가 현재 non-Kouku 데이터와 합친 후보를 검증한다. 후보는 승인된 run에만
pin하며 정식 catalog와 설치 Product 파일은 교체하지 않는다.

Client는 기존 presentation·animation binding·dependency parser로 같은 임시 JSON을
검증한다. 준비 중인 후보와 현재 실행 표현을 분리하고, 자기 요청의 승인 hash·source revision·
room epoch가 일치할 때만 후보를 활성화한다. 같은 source revision이나 같은 gameplay hash를
가진 시각 표현 변경도 새 draft 실행 epoch에서 다시 읽는다. 이전 실행의 중복 응답은 기존
표현을 유지하고, 모르는 hash는 거부한다. 다음 정식 실행은 설치 Product를 다시 사용한다.
관문 준비 중 world generation·요청·gameplay revision이 바뀌거나 사용자가 취소하면 제출하지 않는다.

2026-09-23 최종 후보 재생성은 사용자가 저장한 revision 2233,
SHA-256 `f38f79a3ab5cb80fe15d0c8cf2e6eb3c90e0d849987c08c4f0d6939890351140`을 기준으로 했다.
후보 revision은 2234이며 `out/KoukuCollider20260923/flamewave/`의 candidate·base·manifest에
보관했다. 최신 후보의 선택 P58 Draft CLI는 exit 0, 101행·24096바이트를 생성했다.
이 단계에서는 live Composition과 Effect를 쓰지 않았으며 최종 데이터 적용·EXE 설치는 별도 기록한다.

Shared Debug build, 별도 출력의 Server build와 Client ClCompile이 통과했다.
PresentationPlayer TU에는 컴파일 section 상한에 필요한 `/bigobj`를 해당 TU에만 추가했다.
실제 선택 P58 draft를 읽는 Server의 13개 검사는 failures 0으로, checksum·영역 외 행·chunk
크기·순서·오래된 요청·world·만료·run pin·응답 hash·재시도와 정식 catalog 보존을 확인했다.
Server overlap contract도 failures 0이다. 로그는
`out/KoukuColliderDetail-draft-server-actual-contract.log`,
`out/KoukuColliderDetail-draft-server-overlap-contract.log`에 있다.

그룹 복제·member focus·독립 Horizontal·Back to Group·Save/Reopen 집중 하네스가 통과했다
(`out/KoukuColliderDetail-draft-workbench-contract.log`). 최신 revision 2234 후보를 실제
Composition codec으로 읽고 28개 anchor 참조·8개 창·저장 형식 왕복 동등성·없는 참조 거부를
포함한 24개 검사가 통과했다
(`out/KoukuCollider20260923/schema/candidate-codec-latest.log`). 선택 Draft CLI 로그는
`out/KoukuCollider20260923/flamewave/latest-draft.log`이다. 최종 Client 별도 출력 링크와
EXE 설치·사용자 화면 확인은 이 기록 시점에 완료로 처리하지 않았다.

## 2026-09-23 정식 Publish Parent 소비자 정합성

실제 Publish 로그에서 World 단계가 `parentPatternSequence`를 알 수 없는 필드로 거부했다.
Gameplay emitter의 Parent 검증을 `KoukuParentSequenceContract.ps1` 한 곳으로 분리하고
World도 같은 검증을 소비하게 했다. 참조 자식의 boss·gate·actor 일치, 전체 수명 보존,
시간 순서, 반복·중첩 금지와 Bingo loop 참조 검증을 유지했다. BuildDomains의 World 및
Gameplay tool 의존성에도 helper와 공유 emitter를 등록했다.

실제 World 함수만 불러오는 `Test-KoukuParentSequenceContract.ps1`에서 저장 Product
revision 2222(113개 Pattern·5개 Parent)와 최신 P58 선택 후보 revision 2234(1개 Pattern)가
통과했다. 공유 Parent 검증은 유효한 두 모드와 잘못된 입력 11종의 거부를 확인했다.
저장 Product의 canonical emitter도 37,716행을 메모리에서 생성했고 11.798초에 통과했다.
변경 PowerShell parse, BuildDomains JSON/tool 경로 확인 및 `git diff --check`가 통과했다.
검증은 authoring·설치 runtime 파일을 쓰지 않았으며 정식 Publish 성공으로 기록하지 않는다.

실패 로그의 Map 88.751초는 fingerprint 비용이 아니다. 동일 입력의 read-only fingerprint는
282ms였고, receipt 확인은 13ms였으며 기존 `map.kakulsaydon.receipt.json`이 없었다.
실패 transaction rollback 뒤에도 receipt가 없으므로 Map action이 다시 실행되는 상태였다.
Map cache·publisher·receipt를 변경하지 않았다. 첫 성공 Publish 뒤 동일 입력·출력의 receipt
재사용 여부와 사용자 화면 판정은 별도 확인 대상이다.

## G07. 승인 후 실제 설치와 검증 범위

사용자가 저장 및 Client·Server 종료를 확인하고 최신 저장본 반영을 승인했다. 다시 읽은
저장본은 revision 2233 / `f38f79a3…0351140`으로 검증한 baseline과 같았다.
기존 editor와 같은 `.writer.lock`의 exclusive Win32 handle을 잡고, 원본 backup·Effect 해시·
교체 직전 byte 비교를 거친 뒤 revision 2234 / `34c2efa5…fd2a956` 후보로 원자 교체했다.
재개방 JSON·byte 일치를 확인했고 실패 시 자기 변경만 rollback하는 경계를 유지했다.
적용 영수증은 `out/KoukuCollider20260923/flamewave/applied-receipt.json`이다.
실제 저장 문서에서 28개 화염 Cylinder·8개 group·8개 접촉 창과 기존 BOX 보존을 확인했다.

Client·Server는 별도 출력의 compile/link를 먼저 통과한 뒤, 사용자 종료 확인 후 정식
`Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`에 링크했다. 마지막 공유 접촉 창
안내 UI까지 Client ClCompile 및 링크 exit 0이다. 로그는
`out/KoukuColliderDetail-final-client-link-console.log`,
`out/KoukuColliderDetail-final-server-link-console.log`에 있다.
설치된 Server EXE로 최신 revision 2234 선택 draft를 읽는 13개 계약도 failures 0이다
(`out/KoukuColliderDetail-final-server-contract.log`). 변경 vcxproj XML 및 실제 JSON parse와
`git diff --check`를 확인했다. Client를 자율 실행하거나 UI를 조작하지 않았고 화면 판정은 사용자가 한다.
정식 Product 전체 빌드나 정식 Publish 실행을 이번 최소 컴파일·링크 증거로 대신 기록하지 않는다.

## G08. Publish 측정 및 중복 검증 감소

저장 revision 2233의 119개 Pattern을 동일 입력으로 측정한 projector 작업은 96.85초에서
66.06초로 31.8% 감소했다. 같은 pinned 입력의 native WModel hash를 반복 계산하지 않고
동일 후보의 성공 검증을 재사용한다. 최종 exact-content freshness 검사는 유지했다.
출력 encounter 26,496,680바이트와 bindings 3,742,906바이트가 수정 전후 byte 단위로 같았다.
이는 projector 단일 단계 측정이며 전체 Publish 완료 시간이 아니다.

정식 projector의 성공한 publish/validate는 실제 consumed input, 없는 optional 입력,
여섯 projection 코드 파일과 두 출력 해시를 `out/KoukuSaydon/composition-validation.receipt.json`에
기록한다. 이어지는 Gameplay publisher의 Validate는 정확한 내용이 모두 같을 때 재투영을 생략한다.
수정 시각을 복원한 같은 길이의 변경·native 변경·optional 파일 생성·도구 변경·손상된 증명은
재사용을 거부하고 기존 검증으로 돌아간다. 게시의 writer lock·최종 재검사·rollback은 유지된다.

현재 revision 2234의 실제 P58와 두 native WModel을 사용하는 별도 out 측정에서
publish-to-out 1.552초, 정확한 증명 기반 validate 0.423초가 나왔고 출력 120,416바이트가 같았다.
이 측정은 canonical source·encounter·bindings·bootstrap을 쓰지 않았다. 전체 119개 Pattern의
증명 기반 Validate 시간이나 최초 cold Map 게시를 포함한 전체 Publish 시간으로 확대하지 않는다.
근거는 `out/KoukuColliderDetail-projector-profile-after.log`,
`out/KoukuColliderDetail-projector-byte-equality.log`, `out/KoukuColliderDetail-certificate-p58.log`이다.

최종 draft·certificate·cache 집중 계약 12개가 9.318초에 통과했다
(`out/KoukuColliderDetail-draft-publish-contracts-final.log`). 재무장/밀림의 emitter 검사도 통과했다.
별도로 실행한 기존 object-contact emitter fixture는 P1을 DRAFT로 둔 채 direct projection에
포함되기를 기대해 StopIteration이 발생했고, publish-inventory 임시 fixture는 P9 native 모델이
없는 상태에서 ready 2개를 기대해 1개와 불일치했다. 두 기존 fixture는 이번 범위에서 바꾸지 않았고
전체 suite 통과를 주장하지 않는다. 새 집중 계약과 실제 P58·네이티브 실행 계약 통과를 구분한다.

## G09. 관문 입장·피해 연결과 Release 통합

사용자의 최신 요청은 4인 Release에서 G1 입구부터 G3 종료·빙고 연출·Parent 반복까지 연결하고,
Collider 피해·낙사·부활을 함께 배포하는 것이다. G1·G3은 낙사를 금지하고 G2·Bingo만 최초
지지면 아래5m 사망면을 사용한다. 코드와 별도 검증은 BGM, G3/Bingo, fall/respawn RESULT에 기록했다.

FIXED_DAMAGE/damageAmount를 Document, Workbench, projector, row emitter, native catalog와 기존
피해 처리에 연결했다. 구조체 중간 필드 추가로 기존 aggregate 초기화가 깨진 push_back 컴파일
오류는 Server 필드를 끝에 추가하도록 교정했다. Collider Damage UI의 Dirty 누락도 수정했다.
AttackHitTemplates의 optional riseHeightM/pushMs는 기존25열과 추가27열을 함께 지원한다.
Pursuit cardSymbols는 생성된 카드와 플레이어의 문양만 비교하고 색상은 비교하지 않는다.

최신 디스크 revision2234를 stable ID/field 기준으로 병합한 후보2235는161개 필드 변경이다.
전용 writer lock, baseline SHA256 재확인, 백업·원자 교체·재개방 검사를 거쳐 반영했다.
P13의 카드 뒤집기 연결을 보존하며 피해9개를 실제 망치 WEAPON b_rpct_01 원통으로 맞췄다.
P86/P87의 접촉 시각은 본 접지 근거에 맞추고100ms 창으로 변경했다. 도넛·노란 장판12개는
전용 Result의 수직3m/1200ms, Albion 기존 fixed hit는 같은 높이/시간으로 연결했다.
레이저의 기존7개 Collider는 중복 생성하지 않고 상승 Result와 Collider group을 연결했다.

불뿜기는 고정500 HP/500ms이며 기존 P27 부채꼴을 연결하고 중복 단발 창을 비활성화했다.
화염링은 실제 불길 emitter의2초 수명, 일반3갈래 불뿜기는 내부1.766667초 시작과2초 수명을
사용한다. 새 geometry는 PROJECT_TUNED 초깃값(부채꼴 반경9m/반각18도, 화염링 불길 BOX
너비1.8m/길이9m)이며 사용자 화면 튜닝이 남는다. clone 전용 자식67~70은 기존 Parent66의
leaf 표현 계약을 보존해 이번 데이터 확장 대상에서 제외했다. 기존 빈 placeholder6개는 그대로이며
Product113개/저장119개로 회귀 없이 전체 projection을 검증했다.

검증 증거는 out/KoukuCollider20260923 아래에 있다.

- Release Product 빌드 PASS: 최종 compile receipt20260923T044819665Z-release-product.json.
- 실제 서버 Raid974 assertions PASS: 2·3·4인 G1 일시 거절 재시도,4인 빙고 Parent 반복,
  클리어 후150tick 진입 포함. BGM selector native46 assertions PASS.
- Dice/FIXED_DAMAGE native55 assertions PASS, 동적 overlap·상승·낙사·부활 및 Valtan support failures0.
- 실제 P13/P39/P86/P87/P23 draft의 native admission, hash·rollback 검사 failures0.
- 별도 Release Workbench 하네스의 고정 피해/수직 상승 Apply→Save→Reopen, 공유 Result 보존,
  Dirty/reset, invalid rollback, AttackHitTemplates 왕복 PASS.
- Python attack/dice 집중16개 PASS. C++ 파일 인코딩 유지 및 git diff --check PASS.

이번 기능은 새 물리 Resources를 만들지 않았다. 이전 GBResources2의 쿠크 보완31개/407790972
바이트를 요청한 Desktop/GBResources에 상대 경로대로 추가했고 기존 파일 삭제·교체 없이
31개 SHA256을 확인했다. Client/UI는 실행하지 않았으며 실제4개 Client의 화면·사운드는
사용자 검증과 구분한다. Release 배포의 최종 게시·ZIP 영수증은 후속 G10에 기록한다.

## G10. 정식 게시와 최종 배포 검증

공식 Kouku projector Publish, Gameplay Publish, Composition Publish 및 Kouku World Publish가
모두 exit0으로 완료됐다. source/encounter/presentation/Server bootstrap revision2235,
Sequence167, protocol106을 맞췄다. 월드 트리거 실행 데이터만 함께 갱신했으며 맵 geometry
재게시를 실행하지 않았다. 게시 후 같은 최종 Release Server로 Raid974 assertions를 다시
실행해 failures0을 확인했다(`final-published-raid.log`).

Release binary pin은 `out/KoukuCollider20260923/release-ready.json`에 기록했다. Client SHA256은
4c92f7f3e0a80c7524ac38ee07998b334ff770bafba96505dc9f9e708f3eaaab,
Server는 ec77df7a2b1486c11a09394876da62c913335e7f7558929deaa623f0cf7c74b8이다.
ZIP은 정본 Data와 해당 실행 데이터, Release EXE/DLL/CSO를 같은 배포에 포함하고 별도
Resources 경로를 사용하는 기존 portable 실행 구조를 유지한다. 파일 hash뿐 아니라
protocol 및 Action/Sequence revision의 일치도 launcher preflight가 검사한다.

최종 Desktop/LostArk-Release-20260923.zip은127130057바이트이며 SHA256은
9fedd2a4b4047d3648e7b3f341235a6b73e7e95f9ff4f067c334b5b440512d33이다.
2410 ZIP entries,2407 manifest 파일,CRC·원본/ZIP hash·실제 게시 revision·launcher preflight가
독립 검사까지 PASS했다. 영수증은 `out/KoukuCollider20260923/release-stage-20260923-135445.verified.json`.
이전 ZIP은 Desktop/LostArk-Release-20260923.backup-20260923-135548-893886.zip으로 보존했다.
Resources는 ZIP에 중복 포함하지 않으며 README에 같은 PC에서4개 Client를 실행하는 절차도 적었다.
대용량 Encounter를 읽는 portable launcher의 JSON 상한은 명시적인64MiB로 제한했다.
