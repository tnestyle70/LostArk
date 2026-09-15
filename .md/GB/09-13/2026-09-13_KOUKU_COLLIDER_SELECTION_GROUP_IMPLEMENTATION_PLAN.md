# 쿠크 콜라이더 선택 그룹 구현 계획

## G00. 목표와 현재 실측

무지개댄스에 배치한 Collider 박스들을 Box Detail의 `Set Group`으로 묶는다. 이후 한 멤버를 클릭하면 그룹 전체를 선택하며, 공통 중심을 기준으로 위치와 Y축 회전을 조절한다. `Ungroup`으로 개별 편집으로 돌아간다.

현재 `KAKULSAYDON_G1_PATTERN_38`에는 Collider 5개가 있고 모두 BOSS/followBoss=true, 빈 bone, BODY 기준이다. 각 박스의 시작 시각은 다르므로 그룹 생성과 공간 변환은 시각·수명·크기·연결 Logic을 바꾸지 않는다. 현재 Workbench에는 회전 gizmo가 없고 Box Detail의 숫자 드래그가 기존 공간 편집 경로다. 전투 영역은 Shared XZ 판정이므로 공동 회전도 Y축으로 제공한다.

기존 작업 브랜치 `codex/kouku-donut-ball-motion`의 미커밋 이펙트·Composition 변경을 보존한다. 사용자가 실행 중인 Client/Server와 편집 문서는 종료·Reload·덮어쓰지 않는다. 사용자 원본 5개를 자동 그룹화하지 않고 사용자가 `Set Group`으로 선택한 범위를 저장한다.

## G01. 문서와 저장 계약

`Client/Public/KoukuSaydonCompositionDocument.h`의 presentation occurrence에 optional JSON `selectionGroupId`에 대응하는 문자열을 둔다. 빈 값은 그룹 없음이며 기존 문서는 그대로 읽힌다. 기존 default equality가 dirty/undo/save 비교를 함께 소비한다. 별도 그룹 문서, 부모 transform, 중첩 group은 추가하지 않는다.

`Client/Private/KoukuSaydonCompositionDocument.cpp`는 Collider 전용 stable ID, 같은 Pattern, 두 개 이상 멤버와 공통 BOSS anchor/follow/bone 기준을 검증한다. 첫 버전은 기존 Collider가 지원하는 보스 기준만 묶는다. MAP Collider는 기존 런타임이 지원하지 않고 WORLD의 비균일 scale은 공동 강체 회전을 보장하지 못하므로 이번 그룹 범위에 넣지 않는다. 서로 다른 frozen anchor 시각은 같은 좌표 기준으로 보장할 수 없으므로 거절한다. 생성·복제·삭제에서 그룹 membership을 정리하고 실패 시 기존 draft를 유지한다.

`Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`는 같은 optional authoring 계약을 검증하되 runtime projection에서는 그룹 ID를 제외한다. 기존 각 Collider의 확정 위치·회전만 runtime이 소비한다. Save와 Publish는 계속 분리한다.

현재 실측은 원본 572,190 bytes, 48 Pattern, 242 presentation occurrence, 94 Collider다. 추가 비용은 grouped row의 짧은 ID와 Pattern별 선형 검사다. 기존 `Save_Atomic`의 validate → 임시 직렬화 → 재파싱 → freshness 비교 → 교체를 유지하고 멤버별 별도 Save를 호출하지 않는다. 기존 큰 문서 복사·검증 비용은 남지만 그룹 계층 탐색이나 별도 파일 I/O는 추가하지 않는다.

## G02. Workbench 선택과 공동 변환

`Client/Public/KoukuSaydonActionWorkbench.h`와 `Client/Private/KoukuSaydonActionWorkbench.cpp`의 기존 stable occurrence 선택, geometry overlay와 candidate commit 경로를 확장한다.

1. 다중 Collider 선택 시 Box Detail에 `Set Group`을 제공한다. 다른 lane과 혼합 선택하거나 anchor가 다른 선택은 명시적인 이유로 거절한다.
2. timeline click과 외부 `Select_PresentationBoxById`는 같은 그룹 멤버 전체로 선택을 확장한다. Ctrl 선택·마키도 그룹 단위 membership을 유지한다.
3. 공동 중심은 선택한 Collider 중심 좌표의 산술 평균이다. Y 회전 delta로 pivot에 대한 XZ offset을 회전하고 각 Collider yaw에 같은 delta를 더한다. 이동은 같은 delta를 모든 중심에 더한다. 크기·시각·수명·Logic은 보존한다.
4. geometry는 기존 staged overlay에 한 batch로 보관하며 drag마다 멤버별 전체 문서 commit을 반복하지 않는다. Save/Preview는 해당 overlay를 소비한다.
5. 복제는 새 그룹 ID를 사용해 원본과 분리하고, 삭제로 한 멤버만 남으면 해당 그룹을 해제한다. `Ungroup`은 위치를 바꾸지 않는다.

새 C++ 파일은 추가하지 않으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다. 기존 파일 인코딩을 감지해 유지한다.

## G03. 종료 증거

기존 Python 테스트에서 optional field, 잘못된 타입/멤버/anchor, runtime metadata 제외를 확인한다. 기존 native editor 검증 경로에서 그룹 생성·선택·공동 회전·복제·삭제·해제·Save/Reopen과 실패 시 이전 상태 보존을 확인한다. 사용자 원본 대신 out의 임시 입력을 사용한다. 변경 C++는 최소 컴파일, Python/문서는 필요한 parse와 `git diff --check`를 수행한다.

정상 Product 빌드는 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`다. Client/Server가 실행 중이면 out에 컴파일 산출물을 격리하고 제품 링크 완료와 구분한다. 실행 중 편집 보존 후 종료할 시점은 사용자에게 맡긴다.

사용자 확인 경로는 `F1 → Action Composition Workbench → KoukuSaydon → 무지개댄스 → Collider 다중 선택 → Box Detail → Set Group → 한 멤버 선택 → Group center / Y rotation → Save → Reopen`이다. 최종 화면과 입력 조작은 사용자가 직접 확인하며 에이전트는 Client/UI를 실행·조작·캡처하지 않는다.

## G04. Effect 선택 그룹과 시간 이동 확장 (2026-09-14)

기존 `selectionGroupId`를 Effect에도 허용한다. Effect 그룹은 같은 Pattern의 Effect 두 개 이상이며 각 멤버의 Boss/World/Map anchor, bone, worldOccurrenceId, offset과 회전을 그대로 보존한다. 왼 총과 오른 총처럼 서로 다른 anchor를 묶을 수 있다. Collider의 공통 BOSS 좌표계와 공간 변환 제약은 유지한다.

Workbench의 기존 선택 확장·그룹 ID 발급·복제·singleton 정리·timeline drag를 공동 presentation 선택 계약으로 확장한다. Effect 다중 선택에 Set Group/Ungroup을 표시하며 그룹 또는 임시 다중 선택을 가운데 드래그하면 시간 간격을 유지한 채 이동한다. 범위 경계는 멤버 전체에 같은 delta를 clamp하고, 세대나 선택이 바뀌면 기존 timing을 보존한다. Effect의 부모 World 박스나 anchor transform은 이동하지 않는다.

Document와 Python projector는 같은 종류 그룹과 두 개 이상 멤버를 검증한다. Effect에는 Collider의 공간 anchor 동등성 제약을 적용하지 않으며 runtime projection에서는 기존처럼 metadata를 제거한다. 기존 Data는 자동 그룹화하지 않고 사용자가 새 빌드에서 선택한 그룹만 Save한다. 변경 C++의 최소 컴파일, native editor Save/Reopen·복제·해제·실패 보존과 시간 이동 검증, Python 계약 검증을 수행한다. 새 소스 파일이나 프로젝트 항목은 추가하지 않는다.

## G06. Shift 클릭 추가 선택 별칭 (2026-09-14)

Composition Sequencer의 기존 Ctrl-click/빈 영역 Ctrl-drag 선택 modifier에 Shift를 같은 별칭으로 추가한다. Render_Timeline 시작에서 `additiveSelection = io.KeyCtrl || io.KeyShift`를 한 번 계산하고 Stage/Animation/Logic/Summon/World/Scene Profile/child Pattern/Effect의 8개 클릭 경로, modifier 중 drag 차단, Effect 기존 선택 유지와 마키 clear에 동일하게 사용한다. Set Group·Duplicate의 데이터 및 anchor/ID 동작, Ctrl+D와 방향키 분기는 바꾸지 않는다. 안내 문구에 Ctrl/Shift를 표시한다. 기존 Workbench.cpp 한 파일만 수정하고 해당 TU를 out에서 최소 컴파일한다. JSON과 Client/UI는 조작하지 않는다.

### G06.1. Effect 그룹 중심 이동·같은 시각 복제·저장 (2026-09-14)

사용자가 Composition에 묶어 둔 Effect들을 Box Detail에서 함께 공간 이동하고, 같은 시각에
그룹을 복제한 뒤 위치를 바꾸어 저장할 수 있게 한다. 이번 공간 입력은 XYZ translation이며
기존 Collider의 Y 회전 기능을 Effect에 추가하지 않는다. 그룹 선택과 타임라인 이동은 기존
`selectionGroupId`를 사용하고 새 schema·그룹 runtime·부모 Transform은 만들지 않는다.

`Client/Public/KoukuSaydonActionWorkbench.h`와 `Client/Private/KoukuSaydonActionWorkbench.cpp`의
기존 Effect group detail과 presentation geometry overlay 경로를 확장한다. 선택된 각 Effect의
저장 occurrence에 현재 staged placement를 적용하여 유효 위치를 읽고, 공통 좌표 기준인지
검사한다. 고정 MAP은 시작 시각이 달라도 같은 world 좌표를 쓰므로 함께 이동할 수 있다.
BOSS/WORLD는 같은 anchor 종류·bone/target·follow와 해당 WORLD occurrence/emission 기준을
요구하며, frozen anchor는 시작 시각도 같아야 한다. 서로 다른 좌표 기준은 공간 이동만
이유를 표시하고 거절하며 기존 그룹 선택·시간 이동·복제 기능은 유지한다.

표시하는 group center는 멤버 PositionOffset의 산술 평균이다. 새 중심과 기존 중심의 차이를
`std::array<double, 3u>` translation으로 계산하여 모든 멤버의 위치에 똑같이 더한다.
전체 후보의 stable ID·resource·placement와 유한 범위를 검사한 뒤 작은 geometry overlay를
함께 stage한다. invalid 입력에서는 일부 멤버만 바꾸지 않는다. 각 Effect의 회전·크기·anchor·
시작 시각·수명과 부모 WORLD·Logic은 보존한다. 기존 geometry preview queue와 Player의
현재 anchor/history 소비 경로를 재사용하고, 표시 준비 실패를 문서 저장 실패와 혼동하지 않는다.

Effect group detail의 `Duplicate Group (same time)`은 기존 `Duplicate_TimelineSelection`에
기본값 false인 optional `atOriginalTime`을 전달한다. 기존 호출과 Ctrl+D는 계속 원래의
후속 시각 복제를 사용한다. 새 모드는 기존 그룹·소유 참조의 선택 확장을 마친 뒤 Stage가 없고
최종 선택이 Effect뿐인지 검증한다. 유효하면 `insertMs=first`, `delta=0`으로 각 멤버의 원래
시작 시각과 상대 간격을 보존한다. 기존 occurrence 발급과 group ID remap으로 복제본을 원본과
분리하고, 이미 구현된 old→new occurrence 대응의 staged placement 복사를 그대로 사용한다.
따라서 Save 전 이동한 geometry도 복제본에 반영된다. Effect만 선택한 경우 기존 WORLD 총은
계속 공유하며 새 WORLD 객체를 자동 생성하지 않는다.

같은 detail에 노출하는 `Save`는 기존 Workbench `Save`를 호출한다. 저장은 현재 draft와
staged geometry 전체를 한 candidate에 적용한 뒤 기존 `Save_Atomic`으로 처리하며, 별도 그룹
파일이나 멤버별 저장은 없다. validation 또는 freshness/CAS 실패에는 draft·overlay·기존
디스크를 유지한다. 사용자 Effect/Composition JSON은 에이전트가 외부에서 수정하지 않는다.

새 C++ 파일과 project/filter 등록은 없다. 집중 검증은 MAP의 서로 다른 시작 시각 이동,
BOSS/WORLD 공통 기준·frozen 시각 제약, 전체 후보 invalid/overflow 보존, 기존 회전·크기·
시각 불변, Save 전 geometry의 같은 시각 복제·새 group ID·WORLD 개수 불변, Save/Reopen과
외부 저장 충돌 보존을 확인한다. 현재 소스 컴파일과 preview 실패 결과의 실제 소비는 구현 후
RESULT에 기록하며 기존 검사 결과를 새 기능의 PASS로 대신하지 않는다. 사용자 입력 경로는
Effect 그룹 선택 → Group center XYZ → Duplicate Group (same time) → 새 그룹 위치 조절 →
Save다. 실제 UI 조작과 화면 판정은 사용자가 수행한다.

같은 시각 복제 뒤에는 신규 occurrence ID를 모르는 기존 preview snapshot을 재사용하지 않는다.
현재 pending/live/cursor 시각과 paused 상태를 보존한 Request_PatternPreview를 다시 요청하여
새 그룹과 staged geometry 전체를 준비한다. 준비 실패는 복제 자체와 구분해 표시한다.
