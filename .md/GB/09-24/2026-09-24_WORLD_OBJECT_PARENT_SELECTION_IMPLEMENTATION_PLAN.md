# World Object Parent와 범위 선택 구현 계획

## G00. 현재 실측과 목표

`CWorldObjectTool::Render_Resources`는 Map/Character/Boss 아래 Object를 원본 순서로 나열하고
단일 선택만 처리한다. Object 아래의 Motion은 실제 모델 binding을 소유한다.
`motionInstanceIds`는 여러 Motion을 함께 재생하는 합성 리소스이므로 정리용 Parent로 재사용하지 않는다.
`Create_Object`는 선택한 항목과 관계없이 root에 새 리소스를 추가한다.

Ctrl 클릭으로 개별 선택을 추가·해제하고, 첫 행 클릭 후 Shift 클릭으로 현재 펼쳐진 Object/Parent
행 사이를 모두 선택한다. 우클릭 시 기존 다중 선택을 보존하며 Create Parent와 그 아래
Move to Parent를 제공한다. 새 Parent는 이름을 입력받아 선택한 항목을 포함한다.
기존 Object도 정리용 자식을 가질 수 있게 하여 갈고리 Object를 선택한 Create Object와
기존 두 Object의 Move to Parent를 지원한다. 이 계층은 위치·개수·모델·Motion 상속을 만들지 않는다.

## G01. WorldSequenceDocument의 저장 계약

수정: `Client/Public/WorldSequenceDocument.h`, `Client/Private/WorldSequenceDocument.cpp`.

- `WORLD_SEQUENCE_OBJECT_FOLDER`는 stable folderId, displayName, anchorKind, parentId를 저장한다.
- optional root `objectFolders`, optional Object `parentId`를 기존 v3 codec에 추가한다.
- parentId는 같은 anchor의 Folder 또는 Object stable ID를 참조한다. 빈 값은 category root다.
- 중복 ID, 없는 부모, 자기 참조·순환, 64단계를 넘는 계층, 다른 anchor로 이동을 거부한다.
- `Validate_ObjectHierarchy`는 모델을 지정하기 전의 편집 draft에서도 조직 정보만 검증한다.
- Parse/Save/Clear/Is_Equivalent 및 기존 전체 Validate가 같은 정보를 소비한다.

수정: `Tools/MapPipeline/Publish-MapAuthoring.ps1` 및 기존
`Tools/MapPipeline/test_world_sequence_authoring_contract.py`.
publisher도 같은 optional 필드와 참조를 검증한다. 기존 문서는 변경 없이 열리며
실제 runtime 모델·Motion 소비 경로는 유지한다. 실제 저작 파일은 사용자의 Tool Save가 반영한다.

## G02. WorldObjectTool 입력과 트리

수정: `Client/Public/WorldObjectTool.h`, `Client/Private/WorldObjectTool.cpp`.

선택 집합과 범위 시작점은 stable ID를 저장하고, 화면에 펼쳐진 Object/Parent 행 순서를 사용한다.
Motion 클릭은 기존 상세 편집 경로를 사용한다. Parent/이동 팝업은 선택 snapshot과 대상 ID를
보유하며 확정 시 후보 document에만 반영하고 계층 검증 성공 후 commit한다.
상위와 자식을 함께 선택하면 최상위 선택만 이동하여 내부 계층을 유지한다.
실패·취소는 기존 document와 선택을 유지한다. Create Object는 선택한 Parent/Object를 기본
목적으로 삼고 같은 anchor를 사용한다. 검색은 일치한 자식의 조상도 표시한다.

조직 변경은 document revision/dirty만 갱신하고 기존 Motion 편집 표시를 추가하지 않는다.
기존 Save의 stage/readback/equality/CAS/rollback 및 Publish 경로를 그대로 사용한다.
Anchor 변경이 계층 제약을 깨뜨리면 기존 값을 보존하고 먼저 root로 옮기도록 안내한다.

## G03. 검증과 프로젝트 등록

새 C++ 파일이 없으므로 프로젝트·필터 등록 변경은 없다.
기존 publisher 계약 검사에 정상 폴더/기존 Object 부모, 알 수 없는 부모, 순환,
anchor 불일치와 기존 Motion 보존 사례를 추가한다. 실제 C++ codec의 저장/재로드,
도구 선택·이동 함수의 실패 보존을 격리된 out 산출물에서 확인한다.
변경 TU의 최소 컴파일 뒤 실행 중 출력물 점유 상태를 확인하고 정상 증분 Product Build를 수행한다.
Client/UI 실행·조작은 하지 않는다. `git diff --check`, PowerShell parse 및 인코딩을 확인한다.
실행한 검사와 사용자 확인 항목은 대응 RESULT에 구분한다.

사용자 확인: F1 → World Object Tool → Map → 첫 Object 클릭 → 열 번째 Object Shift 클릭 →
우클릭 Create Parent → 이름 입력. 다른 두 Object Ctrl 선택 → Move to Parent → 갈고리 선택.
갈고리 선택 후 Create Object가 그 아래 표시되고 Save/Reload 후에도 소속이 유지되는지 확인한다.
