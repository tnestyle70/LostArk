# 쿠크 콜라이더 선택 그룹 결과

## G00. 구현 상태

Kouku Action Workbench에 Set Group/Ungroup, 그룹 전체 선택과 공통 중심 이동·Y 회전을 반영했다. 선택 그룹은 BOSS 기준 Collider 두 개 이상이며 동일 bone/boneTarget/follow를 요구한다. Follow=false면 고정 기준 시각도 같아야 한다. 사용자가 배치한 무지개댄스 Collider 5개는 BOSS/follow=true여서 대상이다. 원본 Data의 박스나 그룹은 에이전트가 직접 변경하지 않았다.

## G01. 문서·선택·공동 변환

- `KoukuSaydonCompositionDocument.h/.cpp`: optional `selectionGroupId`를 sparse 저장한다. 기존 aggregate 초기화 순서를 유지하도록 필드 끝에 추가했다. collider-only, stable ID, Pattern별 2+ 멤버와 동일 기준 검증을 연결했다.
- `KoukuSaydonActionWorkbench.h/.cpp`: Set Group/Ungroup, 한 멤버 클릭·Ctrl 토글·마키의 그룹 확장, 평균 중심 기준 XYZ 이동·Y 회전을 연결했다. 회전은 각 중심의 XZ offset과 yaw에 적용하고 시간·수명·크기·Logic을 유지한다. 그룹 선택 중 개별 박스 시간 drag는 하지 않으며, 개별 편집은 Ungroup 후 수행한다.
- 복제는 그룹 전체를 새 stable group ID로 복사하고 기존 ID와 충돌을 검사한다. 명시 삭제 목록의 의미는 유지하면서 한 멤버만 남은 그룹은 해제한다.
- group geometry 전체를 먼저 검사하고 작은 row overlay만 stage한다. invalid/overflow에서 앞 멤버만 이동하지 않는다. 기존 Preview queue가 batch를 소비하고 Save는 기존 단일 candidate와 원자 저장을 사용한다.
- Python projector는 optional authoring 필드를 검증하고 제품 presentation projection에서는 제외한다. Parent 반복·잘림의 파생행에서도 제거하여 편집 그룹이 runtime 참조나 잘린 singleton으로 남지 않는다.

기존 단일 박스 편집의 원자 저장·CAS·외부 변경 거절을 완화하지 않았다. 별도 그룹 파일, 부모 Transform, 중첩 그룹 또는 Server 충돌 runtime은 추가하지 않았다.

## G02. 비용 검토

최초 조사 시점 원본은 572,190 bytes, 48 Pattern, presentation occurrence 242개와 Collider 94개였다. 사용자가 계속 저장 중이므로 고정된 최종 파일 크기는 아니다. 실제 무지개 5개에 짧은 그룹 ID를 저장하는 추가량은 약 350 bytes, 조사 시점 전체의 약 0.06%다.

저장은 기존 전체 문서 validate/serialize/temp-reparse/CAS/reopen 비용을 유지하고 그룹 검증은 Pattern별 행 순회와 ID map으로 수행한다. 드래그마다 멤버별 전체 문서 commit이나 파일 저장은 하지 않는다. 5개 그룹 때문에 구조가 크게 복잡해지거나 저장 I/O 병목이 새로 생길 근거는 없다. 실제 사용 중 Save 지연 시간 자체를 계측한 결과는 아니므로 latency 개선으로 기록하지 않는다.

## G03. 검증

- 변경된 Workbench·Document와 기존 native editor test의 Debug 컴파일·격리 test EXE 링크 성공. 정본 제품 출력은 사용하지 않고 `out/KoukuColliderGroups20260913` 아래의 IntDir/OutDir을 사용했다.
- 기존 native harness의 `--kouku-collider-group-contract` 성공: 5개 Set Group, 한 멤버 클릭 뒤 공통 중심 90도 회전과 이동, Save/Reopen, 크기·시각·기타 필드 보존, 복제 그룹 분리, Ungroup, 삭제 후 singleton 정리, NaN 입력 거절, 외부 저장 충돌 시 draft/overlay/디스크 보존.
- Python 신규 그룹 관련 4개와 관련 기존 2개 테스트 성공. 전체 module 154개 실행은 실패 11/error 59였고 대표 기존 실패 3개는 변경 전 HEAD에서도 같은 원인으로 재현했다. 현재 Data와 기존 광역 fixture 기대값 불일치를 이 기능 수정으로 덮지 않았다.
- native 테스트의 live Effect geometry 기대값은 이미 바뀐 Effect placement 계약(geometry와 anchor를 함께 stage)을 반영하도록 교정했다. 이펙트의 bone을 무조건 미적용 필드로 보던 기존 기대값이 현재 구현과 달랐다. 별도 native fixture의 live inventory/world 조회는 기존 CPU-only fail guard 방식으로 차단했다.
- 기대값 교정 후 기존 `--kouku-preview-transport-contract` 전체도 성공했다. 기존 단일 Collider/Effect geometry, Apply/Save/CAS, 선택 동기화, 시간 유지, 다른 lane 복제 회귀를 함께 확인했다. 증거는 `native-tests-final.log`다.

증거는 `out/KoukuColliderGroups20260913/build-final.log`, `native-groups.log`에 있다. 제품 Client EXE 링크와 사용자의 실제 화면 판정은 이 격리 검사에 포함되지 않는다.

## G04. 사용자 확인

사용자가 직접 컴파일 중이고 Client/Server도 실행 중이므로 에이전트는 종료·Reload·제품 출력 교체를 하지 않았다. 새 제품 빌드 실행 후 `F1 → Action Composition Workbench → KoukuSaydon → 무지개댄스 → Collider 5개 Ctrl/마키 선택 → Box Detail → Set Group → 한 멤버 클릭 → Group center offset / Group Y rotation → Save`로 확인한다. 다시 열었을 때 그룹 선택이 유지되고 개별 시작 시각과 간격·크기가 유지되는지 확인한다.

이번 회전 입력은 기존 Box Detail 숫자 드래그 방식이다. 별도 화면 회전 gizmo는 추가하지 않았다. 최종 시각·입력 판정은 사용자 확인 전으로 남긴다.
