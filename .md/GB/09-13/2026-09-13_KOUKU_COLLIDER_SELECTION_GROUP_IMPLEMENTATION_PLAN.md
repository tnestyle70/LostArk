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
