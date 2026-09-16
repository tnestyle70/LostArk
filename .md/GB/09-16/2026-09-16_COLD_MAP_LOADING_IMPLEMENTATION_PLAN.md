# 전체 맵 최초 진입과 캐릭터 모델 준비 개선 구현 계획

## G00. 목표와 실측

목표는 EXE를 새로 실행할 때 Character Select/Bern/Valtan/KoukuSaydon의 최초 진입 준비 시간을 줄이는 것이다. 재진입 GPU residency는 이번 변경의 목표가 아니며 관련 Prototype registry/Model API 후보는 원복했다. Resources 디렉터리, asset ID, WModel/WMAT/DDS와 맵 배치 범위를 유지한다. 별도 업로드 리소스는 추가하지 않는다.

현재 Bern은 50,017 placements, 15,589 used model/material variants, 1,275 physical model paths다. Character Select는 804/139/58, Valtan은 13,184/3,760/448이다. Loader가 전역 catalog를 무차별 로드하는 것이 아니라 필요한 variant를 준비하지만, 작업은 대부분 직렬이고 동일 경로 검증을 반복한다.

실제 catalog CPU 분해 표본은 catalog 5.07초 + material read 1.96초 + JSON parse 11.41초 + validation 33.23초다. 한 번의 load 내 path dedup 후보에서 validation은 9.10초로 줄었다. 실제 DLL과 hardware Debug D3D11 device의 Bern 첫 2,000 variants는 직렬 22.36초(Material 19.56초, Texture.FileAndUpload 17.10초), geometry group 4-thread 후보는 2.57초였다. 서로 다른 실행의 파일 캐시와 부하가 통제되지 않았으므로 전체 시간/보장 개선율로 사용하지 않는다.

Bern 실제 LOD 생성 대상은 5 submesh이고 현재 source headless 합계 약112ms이므로 신규 LOD 디스크 캐시를 우선 구현하지 않는다. TGA는 legacy unique texture 3,420개 중20개이며 나머지는 DDS3,316/PNG84다. native override DDS를 사용해도 legacy 입력은 shadow/preview/source toggle에서 소비되므로 임의 제거하지 않는다.

## G01. MapAssetCatalog의 load 단위 파일 검증

Client/Private/MapAssetCatalog.cpp에서 동일 입력의 Resources-relative resolve, containment, regular-file 검사를 한 번의 admission 안에서 공유한다. nested shard Load가 같은 transaction을 사용하며 다른 thread/load와 결과를 공유하지 않는다. 정상 검증을 생략하거나 실패를 성공으로 바꾸지 않는다. 다음 load는 파일/경로 변경을 다시 검사한다.

실제 동일 catalog/재질/lighting 결과와 malformed/missing/path escape, 다음 load의 파일 변경을 비교한다. hot parser의 Debug 최적화는 동일 입력/출력 실측 후 필요한 TU에만 적용한다. 기존 release/toolset/output 설정은 유지한다.

## G02. Loader 모델 준비의 bounded parallel stage

Client/Public/Loader.h와 Client/Private/Loader.cpp에서 required entries를 physical model path별 작업 묶음으로 구성한다. 각 묶음의 catalog 첫 entry가 기존과 동일한 base geometry/LOD를 만들고, 나머지는 CModel::Create_MaterialVariant를 사용한다. 각 worker는 고정된 staging vector의 독립 index만 기록한다. 같은 geometry의 원형 수명과 모델/재질/텍스처 내용은 바꾸지 않는다.

작업 수와 thread 수를 분리한다. Loader owner를 포함한 제한된 worker가 atomic queue에서 묶음을 가져간다. CPU 수와 D3D single-thread flag를 확인하고 실제 비교로 worker 수를 정한다. 수천 thread를 생성하지 않는다. immediate context를 worker에서 사용하지 않는다. worker별 COM 초기화와 기존 texture same-key single-flight를 유지한다.

모든 worker join, 모든 결과 및 취소 상태 확인 뒤 기존 Add_Prototypes로 batch를 한 번에 commit한다. 중간 실패는 첫 실패 asset ID와 HRESULT를 보존하고 staging 전체를 해제한다. 새 Registry/API/cache 경로를 만들지 않는다.

## G03. 맵과 캐릭터의 공통 실행기

새 `Client/Public/AssetPreparationBatch.h`와 `Client/Private/AssetPreparationBatch.cpp`는 작업 큐 실행과 child thread 수명만 소유한다. 모델, catalog, prototype registry는 소유하지 않는다. `Run`은 owner를 포함한 최대 4개 worker로 고정 index 작업을 처리하고, 모든 child가 종료한 뒤 첫 실패 HRESULT와 작업 index, 실제 worker 수를 반환한다. `Cancel_SynchronousIo`는 보호된 handle 목록으로 취소를 전달한다. 같은 instance의 중복 Run은 `ERROR_BUSY`로 거부한다. thread 생성 실패 때는 owner와 이미 생성한 child가 남은 큐를 처리한다.

`Client/Public/Loader.h`는 batch instance를 소유하며 맵과 초기 선택 캐릭터 준비에 전달한다. `PlayableCharacterAssetService.h/.cpp`의 async job은 자신의 batch instance를 소유한다. 호출자는 취소 flag와 기존 bounded 종료 deadline을 유지한다. D3D single-thread device에서는 owner만 작업한다. 새 H/CPP는 Client.vcxproj와 기존 Loader filter에 등록하고 독립 컴파일 및 Product build로 확인한다.

## G04. Character Select lazy class 준비

`CPlayableCharacterAssetService::Prepare_Models`에서 본체·추가 animation set·장비·무기의 입력을 먼저 owner에서 검증하고 immutable task 배열로 만든다. worker는 기존 CModel 경로로 독립 모델을 준비하고 자신의 고정 결과 slot만 쓴다. 추가 animation set은 기존 path overload를 그대로 사용한다.

모든 준비와 join 성공 뒤 authored 순서대로 animation set을 본체에 붙인다. DimensionMaster 장비의 skeleton/bone/rest palette 검사를 유지하며 본체·장비·무기의 prototype tag와 등록 순서를 유지한다. 기존 presentation authoring 준비, generation 검사, batch commit을 그대로 소비한다. 중간 실패와 취소는 stage 전체를 해제하고 기존 class presentation을 유지한다. Loader 초기 선택 class와 Character Select lazy class가 같은 함수를 사용한다.

개수만 늘리는 병렬화가 실제로 빨라진다고 가정하지 않는다. 제품 빌드 부하와 분리해 실제 설치 캐릭터의 직렬/병렬 시간과 skeleton/clip/material 결과를 대조한다. 기존 CModel의 material 내부 worker 제한도 함께 고려한다.

## G05. 취소·진행률·계측

공통 batch가 child handle을 보호된 목록으로 소유하며 Loader Free와 캐릭터 async 취소의 협력 취소·CancelSynchronousIo가 child에도 전달된다. join/close/취소가 race하지 않게 한다. 기존 bounded join과 ERROR_TIMEOUT process fail-fast를 유지한다.

진행률은 완료 개수와 고정 phase 이름을 사용하여 매 모델마다 phase elapsed가 초기화되지 않게 한다. catalog/scope/model 준비/commit/navigation의 실제 phase와 variant/geometry/worker/elapsed를 기록한다. 전체 완료 시간을 모델 하나의 시간으로 과장하지 않는다.

## G06. 검증과 종료

각 기능의 실제 source/설치 데이터로 수치 및 실패 경계를 검사한다. 정상 모델 내용·count/order·mesh sharing이 같고, 실패/취소/중복 batch에서 부분 등록이 없음을 확인한다. 공통 실행기는 단 한 번 실행·worker 상한·single-thread device·예외·취소·중복 Run·재사용 경계를 headless로 확인한다. Debug Product Build로 Engine/Shared/Server/Client, SDK/CSO/DLL 배포를 확인한다. 변경 XML parse와 git diff --check를 수행한다. 새 제품 H/CPP만 기존 project/filter에 추가하고 기존 항목을 재배치하지 않는다.

현재 dirty worktree의 타 세션 변경을 보존하며 자동 stage/commit하지 않는다. Client/UI를 실행하거나 캡처하지 않는다. 사용자 실제 네 맵 최초 진입·재진입·종료와 화면 확인은 별도이며, headless 결과를 제품 전체 진입 시간이나 visual PASS로 기록하지 않는다.
