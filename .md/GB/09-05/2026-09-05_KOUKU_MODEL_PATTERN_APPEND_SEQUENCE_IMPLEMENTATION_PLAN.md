# 2026-09-05 쿠크 모델별 패턴·Append·Sequence 편집 구현 계획

작업 브랜치는 사용자 지정 `koukysaydon-pattern3`다. 이전 PR #317 위에서 작업하고,
검증 후 PR 병합과 원본 `C:/Users/user/Desktop/LostArk`의 main pull까지 수행한다.

## G00. 실패 원인과 모델 소유권

사용자 화면의 `MN_RPCT_05` Action `4219811`은 세이튼 동작이다. 기존 Resources는
모든 프로필을 보여 주지만 Append를 `MN_RPCZ_00`만 허용해서 해당 버튼이 비활성화된다.
또한 실제 reference Action 0을 미선택/RAW로 오인하는 UI와 validator 오류가 있다.

상위 발탄/쿠크세이튼 구분은 유지하고 쿠크세이튼 패턴을 실제 모델별로 나눈다.
쿠크는 RPCZ_00, 세이튼은 RPCT_05, 대형 세이튼은 RPCT_06이며 RPCT_07은 RPCT_05의
동작 프로필 별칭이다. 패턴의 `actorProfileId`가 해당 물리 모델을 저장한다.

Composition v2는 모델 owner를 명시한다. v1은 homogeneous occurrence에서 owner를 유도하고
빈 기존 Gate1 패턴은 쿠크로 읽는다. 서로 다른 모델이 섞인 패턴은 오류 항목으로 보존한다.
Append는 선택한 모델의 패턴에 연결하고, 해당 모델에 패턴이 없으면 같은 candidate에서 새 패턴을 만든다.
실패한 Append는 원래 draft를 보존한다. PRODUCT 배포의 실제 Gate1 boss 제한은 유지한다.

## G01. Sequence 추가·선택·삭제·Save

`KoukuSaydonActionWorkbench`의 기존 candidate/validate/commit과 Composition `Save_Atomic`을 사용한다.
추가한 Action의 Stage와 Animation은 즉시 Sequence에 표시하고 화면 맞춤을 요청한다.
순차 clip은 같은 Animation 행에 놓고 겹친 구간만 추가 행을 사용한다.
빈 공간의 사각 드래그와 Ctrl 선택으로 stable Stage/occurrence ID를 선택하고 Delete 키 또는
Delete Selected로 하나의 candidate에서 삭제한다. Stage를 지우면 포함된 clip도 같이 지운다.
Sequencer 상단은 Save/Play/Stop, 다음 행은 Zoom과 전체 lifetime(ms)/Apply/Fit으로 구성한다.
전체 lifetime은 기존 Stage clock 합계이며 마지막 Stage의 길이만 조절한다. 이미 배치한 clip의
끝보다 짧거나 600초를 넘는 값은 기존 draft를 보존하고 거절한다. animation 속도를 자동 변경하지 않는다.
Selected Box의 Delete/Duplicate는 사각 선택 전체에 적용한다. Stage 복제에는 자식을 포함하고
동시에 선택한 그 자식을 중복 복제하지 않는다. 새 stable ID를 발급하고 한 candidate로 commit한다.
Inspector 삭제와 Reload 후 실제로 남아 있는 ID만 선택에 유지한다.
저장·재로드와 CAS 실패 보존을 실제 native 메서드로 확인한다.

## G02. 대형 동작 미리보기 배율

추출된 Actor/Component scale 근거가 있으면 해당 원본값을 우선한다. 근거를 찾지 못한 대형 동작은
사용자가 요청한 100배를 미리보기 표시 배율로 사용하며 원본 Unreal 값으로 표기하지 않는다.
기존 CModel/preview actor의 Transform을 사용하고 일반 동작 전환 시 기본 배율로 되돌린다.
실제 적용할 배율과 근거·검증은 RESULT에 기록한다.

대형 세이튼은 기존 `WP_MN_RPCT_06` animated CModel을 기존 `CPart_Body`로 생성하여
오른손 `b_wp_1`에 붙인다. body와 다른 weapon skeleton을 유지하며 확인된 16개 clip을
원본 seconds 기준으로 동기화한다. 대응 clip이 없으면 bind pose를 쓰고, 짧은 weapon clip은
마지막 pose를 유지한다. 생성 실패 시 새 body/weapon을 rollback하여 기존 target을 보존한다.

## G03. 검증과 반영

기존 native harness에 focused entry만 추가해 screenshot Action, Action 0, alias, 모델별 패턴,
batch delete, Save/reload, 잘못된 owner·선택·CAS 실패 보존을 검사한다. 새 harness는 만들지 않는다.
기존 테스트 프로젝트에 필요한 실제 product CPP 등록과 XML parse를 포함한다.
변경한 C++ 최소 컴파일, 해당 publisher 검사, JSON/XML parse와 `git diff --check`를 수행한다.
Client/UI의 실행·화면 확인은 사용자가 한다. 에이전트는 PR/merge/pull과 실행 준비를 완료하고
현재 프로세스 상태와 사용자가 누를 경로를 보고한다.

## G04. 병합 후 Client Product 컴파일 복구

PR #318 병합 기준 `955f7971`의 Client Debug 빌드에서
`KoukuSaydonActionWorkbench.cpp:3080`의 `ImVec2 max(...)` 선언이 C2440을,
뒤의 `AddRectFilled`와 `AddRect` 호출이 C2664를 발생시킨다.
후속 수정은 `codex/kouku-shield-stagger`에서 수행한다.
Client의 Windows include 경로에서는 `max(a, b)` 매크로가 같은 이름의
직접 초기화 선언에 확장된다. 기존 native harness에는 `NOMINMAX`가 정의되어 있으므로
그 컴파일 성공만으로 Product 설정의 오류를 검출할 수 없다.

`CKoukuSaydonActionWorkbench::Render_Timeline`의 사각 선택 좌표 지역 변수
`min`과 `max`를 `marqueeMin`과 `marqueeMax`로 바꾼다. 두 그리기 호출과
hit box 교차 검사도 같은 이름으로 연결한다. 좌표 계산식과 선택 상태 변경은 유지한다.
새 C++ 파일, public 선언, project/filter 등록, JSON/XML 변경은 없다.

검증은 실제 Client 설정을 사용하는 Debug Product compile/link와 `git diff --check`다.
UI 사각 선택의 직접 조작과 화면 확인은 사용자가 수행한다. 이 수정의 코드와 대응 PLAN/RESULT만
PR에 포함하고 기존 미커밋 조사 문서 및 SHIELD_STAGGER 구현 계획서는 보존한다.

## G05. Full lifetime 입력의 ImGui assertion 수정

사용자가 전달한 `imgui_widgets.cpp`의
`(flags & ImGuiInputTextFlags_EnterReturnsTrue) == 0` assertion은
`Render_Timeline`의 Full lifetime `InputInt` 호출과 일치한다. 숫자 입력 함수가
지원하지 않는 플래그를 제거하고, 해당 입력이 활성 또는 비활성화된 프레임의
Enter/Keypad Enter를 별도로 검사해 기존 Apply 버튼과 같은 확정 명령으로 연결한다.
입력 중 값은 session buffer에만 두며 일반 타이핑이나 다른 곳 클릭만으로 draft를 바꾸지 않는다.

기존 CPP의 이 입력 블록만 수정한다. public 선언과 JSON/XML, project/filter 변경은 없다.
Client Debug 최소 컴파일과 `git diff --check`를 확인하고 UI 실행은 사용자가 수행한다.
이번 사용자 요청은 로컬 반영까지이며 commit/push/PR 생성/병합은 수행하지 않는다.
