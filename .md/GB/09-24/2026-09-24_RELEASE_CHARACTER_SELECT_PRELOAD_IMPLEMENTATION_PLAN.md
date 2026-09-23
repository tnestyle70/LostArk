# Release Character Select 전 class 준비 구현 계획

## G00. 현재 소비자와 수정 범위

Release와 Debug 모두 `CLoader::Ready_For_CharacterSelect`가 Server 승인 class 한 개만 준비한다. `Ready_Character_Rendering`은 입력 class가 그 한 명과 같은지 검사한다. `CLevel_Loading::Advance_TargetEffectPreparation`도 선택 class 한 명의 효과만 수집한다. 따라서 class 클릭은 기존 async model 준비와 Product Effect 준비를 마친 뒤 Server class-change command를 보낸다. 사용자 요청대로 Release의 이 대기 비용을 입장 Loading으로 옮긴다.

현재 화면 roster는 Guardian Knight를 포함한7 class다. Server 승인 class와 입장 identity는 유지하고, 전체 roster는 준비할 Client presentation 목록으로만 사용한다. Debug의 on-demand 편집 흐름과 Bern·raid의 선택 class 준비 범위는 유지한다.

## G01. CharacterCatalog.h와 Level_CharacterSelect.h

`CCharacterCatalog`의 공개 constexpr `CHARACTER_SELECT_CLASSES`가 기존7 class 순서를 소유한다. 기존 Level의 `SUPPORTED_CLASSES`는 이 값을 참조한다. Loader와 Effect target 수집도 같은 목록을 사용하여 화면·model·effect 준비가 갈라지지 않게 한다. 신규 파일이나 project/filter 등록은 없다.

## G02. Loader.h와 Loader.cpp

`m_pCharacterAuthoringInput` 한 개를 class ID별 immutable authoring 입력 map으로 바꾼다. `Initialize`는 기존 owner thread에서 승인 class를 검증한 뒤 Release Character Select이면 전체 roster, 그 외에는 승인 class 한 명의 입력을 캡처한다. worker는 이 snapshot만 읽는다.

`Ready_For_CharacterSelect`는 Release에서 전체 roster를 `Ready_Character_Rendering`으로 전달한다. 함수는 각 class의 입력을 조회하고 기존 `CPlayableCharacterAssetService::Ensure_Prototypes`를 사용해 body·part·animation·presentation을 준비한다. 기존 animated shader/variant 준비, worker 취소, `CLevelResourceRollbackScope`와 staging/commit 계약을 유지한다.

## G03. Level_Loading.cpp

Release Character Select의 class 효과 수집을 같은 roster 반복으로 확장한다. 각 class의 skillbinding 기반 Effect cue를 기존 우선 준비 큐와 Loading worker에 합치고 최종 target dedup·settled admission을 유지한다. 손상된 optional class action은 해당 표현만 격리하며 나머지 class와 effect 수집을 계속한다. 이후 class 변경은 기존 model readiness와 exact effect target 검사, Server 승인, transactional presentation 교체를 계속 사용한다.

## G04. 확인 범위

기존 소스 인코딩·CRLF를 유지한다. roster7개의 중복/지원 여부, Release/Debug 준비 범위, per-class 입력과 effect 수집 consumer를 확인하고 변경 C++의 Debug/Release 최소 컴파일을 수행한다. 제품 빌드·게시·commit은 통합 작업이 한 번만 수행한다. 새 데이터와 프로토콜은 없으며 Client/UI는 실행하지 않는다.

입장 지연·메모리 증가와 변경 클릭 지연 감소는 실제 Release 사용자 화면에서 확인한다. 네트워크 밀림·더블클릭 순간이동·Bern FPS 저하는 이 prewarm 완료로 해결됐다고 기록하지 않고, 현재 코드와 기존 진단 로그의 확정 근거만 RESULT에 별도로 남긴다.

## G05. 생성 LOD가 없는 batch의 추가 envelope 계산 제거

09-22 추가 `VIEW_LOD_ENVELOPE`는 모든 batch의 가시성 재계산에서 view scale과 visible instance의 투영 상계를 만든다. `CMesh::Render_Instanced`는 실제 `m_StaticLod`가 없으면 이 값을 사용하지 않는다. `CModel::Has_StaticMeshLod` read-only 조회로 현재 모델에 준비된 LOD가 하나라도 있는지 batch 생성 시 캐시한다. 없는 batch는 tight view envelope만 건너뛴다.

기존 world envelope, culling/visible payload, material admission, LOD 품질 한계와 profiler의 LOD0/source/submitted 분모는 유지한다. 모델/mesh 런타임 경로와 geometry 생성은 바꾸지 않는다. `Engine/Public/Model.h`, `Engine/Private/Model.cpp`, `Client/Public/MapStaticBatchObject.h`, `Client/Private/MapStaticBatchObject.cpp`를 수정하고 새 프로젝트 항목은 없다. public Engine 조회가 추가되므로 통합 Product는 Engine과 Client를 함께 빌드한다. CPU guard 검증과 최소 컴파일을 실제 게임 FPS 확인과 분리한다.

## G06. 지연 snapshot의 정상 이동을 강제 snap으로 오인하는 경로

`CLocalMovePrediction::IsDiscontinuous`는 인접 Server snapshot 사이 tick 차이를 0.35초로 제한한다. main-thread stall과 snapshot coalescing 뒤에도 Server는 계속 정상 속도로 이동하므로, 1초 동안 2.95m 이동한 정상 snapshot이 허용 거리 1.7825m를 넘어 teleport RESET으로 분류된다. 실제 header의 Debug/Release CPU 재현에서 30/53/57 tick 정상 이동이 모두 강제 snap임을 확인했다.

예측을 계속 허용하는 client freshness timeout은 0.35초로 유지한다. Server의 이동 가능 거리 계산만 검증된 forward tick의 실제 경과 시간을 사용한다. 처음 snapshot, 제어 불가 상태, 실제 순간이동, visual/server 10m 초과 disagreement, class/world reset은 기존 RESET 경계를 유지한다. duplicate/backward/half-range tick은 기존 ApplySnapshot 검사에서 제외하고 정상 uint32 wrap은 허용한다.

기존 `ClientPresentationPrimitiveContractTests.cpp`에 빠른 두 클릭 뒤 지연된 정상 ACK, 실제 teleport, wrap·invalid snapshot 경계를 추가한다. native Debug/Release 전체 movement contracts를 실행한다. 사용자 더블클릭 현상과 원인이 같다고 단정하지 않는다.
