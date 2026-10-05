# Bern 공간 청크와 HLOD 구현 계획

## G00. 목표와 기준점

2026-10-04 사용자가 공간 청크, 정적 geometry 병합, 반복 배치 유지와 HLOD 적용을 승인했다. 현재 texture MinLOD 변경은 맵 순회, 재질 설정, draw 제출을 줄이지 않는다. 이번 변경은 이 비용을 실제로 줄이는 것이 목표다. 기존 CModel → CMaterial 경로를 확장하며 원본 placement, picking, 그림자, MapTool 저장 계약은 보존한다.

기준 캡처 `하_20261004_181710_906_frame309_81064_0.json`의 308개 양수 frame interval 평균은 47.744522ms다. CPU 평균 46.943237ms, GPU timestamp 구간 평균 47.707206ms이며 후자를 GPU 연산 점유율로 해석하지 않는다. Map.Batch.Render 평균 15.739020ms, 전체 draw 평균 1001.566개다. 최상 캡처는 저장되지 않아 품질별 A/B 결론은 아직 없다.

설치 Bern에는 50,021 placements, 1,291개 고유 WModel이 있다. 전체 맵의 32m origin cell과 native 표면 입력 동일성으로 계산한 SOURCE_BG 후보는 6,929개 bank8 제출이다. 이는 카메라별 실제 draw 수나 성능 향상 측정이 아니다. 전체 geometry 전개는 메모리를 크게 늘리므로 생성 상한과 원본 fallback을 둔다.

## G01. Engine 공간 후보 탐색

`GameObject.h/.cpp`, `Layer.h/.cpp`에 선택적인 최종 카메라 bounds 계약과 Layer 소유 BVH를 추가한다. dirty 알림으로 bounds를 갱신하고 원래 membership 순서로 생존 callback을 제출한다. 잘못된 bounds와 카메라는 원본 순회로 처리하고 shadow caster는 light가 활성일 때 camera cull로 누락시키지 않는다. Clone은 원본 Layer owner를 복사하지 않는다.

검증은 실제 Layer 코드를 사용하는 no-UI native fixture로 경계 접촉, grace, 이동, 제거, clone, 그림자와 대규모 callback 감소를 확인한다.

## G02. CModel 정적 geometry 병합과 HLOD

`Model.h/.cpp`, `Mesh.h/.cpp` 및 필요한 범용 helper를 기존 Engine 프로젝트에 등록한다. source mesh와 placement의 VTXMESHINSTANCE를 입력받아 동일한 활성 surface 입력끼리 묶는다. RNM과 static shadow의 원본 SRV는 기존 최대 8개 bank로 보존한다. 기존 인스턴싱 비교의 의미는 바꾸지 않고 청크 전용 활성 표면 비교를 사용한다.

GPU 정점은 VTXMESH와 4byte source index를 사용하고 source별 224byte instance payload를 structured buffer에 둔다. UV, vertex color, normal/TBN, signed/nonuniform transform은 기존 shader 연산을 재사용한다. 근거리는 병합된 원본 index, 원거리는 world-space 오차가 제한된 축약 index를 사용한다. material seam과 서로 다른 source 경계를 보존한다. 축약에 실패하거나 이득이 없으면 원본 index를 사용한다. 실패한 생성은 output을 교체하지 않는다.

## G03. Client 청크 수명과 shader 연결

`MapPlacementRuntime`, `MapStaticBatchObject`와 새 `MapStaticChunkObject`가 Bern의 공간 그룹과 파생 모델을 소유한다. 물·투명·풍향 변형처럼 지원하지 않는 표면은 기존 경로를 사용한다. 식생은 기존 instancing과 공간 후보 탐색을 유지하고 NPC는 애니메이션과 Server identity 계약을 유지한다.

원본 placement와 batch는 picking·그림자·편집을 위해 유지한다. 병합이 완료된 mesh만 파생 draw에 위임한다. 위치, 가시성, stage/camera suppression 변경은 해당 파생 claim을 즉시 무효화해 원본으로 복귀한다. 범위 밖의 렌더링 설정과 authoring JSON을 변경하지 않는다. Load/Clear는 파생 claim과 Layer membership을 함께 정리한다.

새 `Shader_VtxMeshMapChunk.hlsl`은 기존 MapInstance shader의 VS 계산과 source BG bank pixel 계산을 재사용한다. Loader prototype, Client.vcxproj, filters와 shader 배포 목록을 함께 등록한다. 원거리 선택은 camera 밖/안과 화면상 world error를 사용하며 통계로 근거리·원거리 제출을 구분한다.

## G04. 검증과 결과 기록

사용자의 추가 요청에 따라 World Level Tool에 실제 생성된 청크의 bounds, 활성/무효 상태, 같은 활성 shader 입력으로 묶인 material과 원본 placement/asset 목록을 표시한다. 청크 선택·Focus·원본 inspect와 경계 overlay는 읽기 전용 진단이며 기존 stable placement ID로 원본 도구에 연결한다. 청크 번호는 이번 로드 안에서만 유효한 진단 번호다. bounds overlay는 near plane에서 clip하고 도구를 닫으면 그리지 않는다. 목록 복사는 매 프레임 수행하지 않으며 성능 캡처는 overlay를 닫은 조건으로 비교한다.

동일 카메라·해상도·렌더링 옵션에서 원본, 청크 근거리, 청크+HLOD를 비교할 수 있는 Debug 제어와 계측을 제공한다. 캡처마다 Reset한 독립 JSON을 사용한다. CPU scope, draw, triangle, GPU timestamp, 메모리와 생성 시간을 함께 본다.

최소 CPP 컴파일, shader 컴파일/입력 계약, 실제 설치 asset 기반 geometry 검사, 변경 project XML parse와 git diff --check를 먼저 통과시킨다. 필요한 Debug/Release 제품 빌드를 수행한다. Client 실행과 실제 화면 확인은 사용자가 수행한다. RESULT에는 자동 증거와 사용자 화면/캡처 미확인을 분리하고 실제 FPS 개선이 나오기 전에 개선율을 포트폴리오 성과로 쓰지 않는다.

## G05. 실제 캡처 후 비용 교정

병합 OFF/ON 실측에서 전체 draw 감소가 약4%이고 제출 index와 batch+chunk 렌더 CPU가 증가했다. 생성된 청크를 무조건 사용하지 않고 최종 카메라의 원본 instance visibility와 원본 mesh LOD index 수를 먼저 확인한다. source와 chunk 중 누가 먼저 요청하더라도 같은 프레임의 선택을 한 번만 확정한다. 기존 lighting-bank instancing으로 합칠 수 있는 원본은 보수적으로 한 draw로 세며, 원본 최소3draw를 대체하고 제출 indices가 원본 이하인 경우에만 청크를 사용한다. 이 조건은 구조적 비용 제한이며 FPS 개선 보장은 아니다.

Bern의 정적 batch는 runtime 소유 공통 frame/time을 사용해 Update/LateUpdate 목록에서 제외한다. map 배치 개수와 placement 계측도 한 번에 제출한다. 같은 프레임의 가시성 준비는 camera revision과 frame generation으로 재사용해 cull grace를 중복 진행하지 않는다. 다른 맵과 기존 authoring Reload의 독립 batch 동작은 보존한다.

서로 다른 재질까지 베이킹한 대규모 구역 proxy와 다단계 HLOD는 이 교정의 구현 완료 범위로 기록하지 않는다. 새 제품에서 같은 구간의 CPU/draw/index 캡처를 받아 실제 효과를 확인한다.

## G06. 사용자 검증 중 발견한 컷신과 입장 결함

Bern 입장 cue의 공통 clock에0.5 playback rate를 적용한다. 첫 입장과 Debug replay가 같은 clock을 소비하며 카메라 key·FOV·보간·저장본은 보존한다.

차원술사 캐릭터 생성 class movie의 회색 shell은 기존 제외 목록과 동일 geometry의 복제 occurrence, native opacity 소비자를 대조한다. 실제 hair를 유지하고 사용자가 제거한 형상의 남은 복제본만 Data/Camera의 stable exclusion 목록에 병합한다. 최신 파일 freshness·writer lock·백업·원자 교체로 다른 편집을 보존한다.

첫 캐릭터 생성 뒤 다른 캐릭터를 생성하고 첫 슬롯으로 Bern에 재입장하는 오류는 실제 Release 진단에서 승인·로딩 정체·복원 실패 이유를 연결한다. Level_Bern의 복원 제한은 로딩 이전 frame delta가 아닌 실제 대기 시간으로 측정하고,5초 제한·입력 차단·실패 시 기존 슬롯 보존을 유지한다. 실제 이전/수정 제어 흐름으로 로딩 지연 재현과 정상 응답·timeout을 검증한다. 위 수정과 함께 Debug/Release Product 빌드를 수행하고 실제 Client 화면 확인은 사용자에게 남긴다.

## G07. 2026-10-05 재확인과 선택 불가능한 청크 준비 제거

G07 캡처의 병합 OFF/ON 비교를 다시 확인한다. 전체 draw 감소와 batch+chunk CPU 합계, 제출 indices, Ambient.Advance를 분리해 기존 병합이 프레임 개선으로 이어지지 않은 이유를 설명한다. G08 교정 이후 동일 조건 새 캡처가 없으면 그 효과를 측정 완료로 쓰지 않는다.

`MapStaticChunkObject.cpp`의 생성과 선택이 같은 최소 원본 draw 수3을 사용하도록 하나의 상수로 연결한다. 원본 member가2개인 그룹은 어떤 가시성·LOD에서도 원본3draw가 될 수 없으므로 파생 geometry 생성 전에 제외한다. 기존32m 구획·재질·조명·삼각형 선택, 실제 가시성 검사와 원본 fallback은 유지한다. 생성하지 않은 그룹의 원본 batch는 계속 기존 경로를 사용하며, 확보된 GPU 예산은 기존 순위의 다른 적격 그룹에만 사용한다.

준비 이후 material 전체를 불변이라고 가정하는 새 cache는 추가하지 않는다. 원본 mesh/material의 실제 호환성 검사는 매 선택 때 유지한다. 새 C++ 파일과 project/filter 변경은 없다. 기존 source resolver fixture와 수정 CPP 컴파일, 설치 inventory의2member 분포를 확인하고 실제 게임 FPS는 사용자 캡처가 필요한 항목으로 분리한다.

## G08. 2026-10-05 공간 후보 재사용과 상위 평면 판정 공유

`Engine/Private/Layer.cpp`, `Engine/Public/Layer.h`의 정적 공간 탐색 중복 비용을 줄인다. 카메라 평면과 그림자 상태가 같고 topology·bounds dirty가 없으며 reject grace가 끝났을 때 원래 순서의 후보 목록을 재사용한다. 후보의 `Submit_FinalCamera`는 매 프레임 계속 호출하므로 batch 가시성·geometry 선택·동적 소비자 동작을 고정하지 않는다. grace가 진행 중이면 다음 프레임에도 순회하며, 카메라·bounds·membership·그림자 변경과 실패 경로는 재사용을 무효화한다.

카메라가 움직일 때는 부모 AABB가 완전히 포함된 평면을 자손에서 재검사하지 않는다. 접촉·roundoff 여유를 포함한 보수적 포함 판정을 사용하고, shadow caster의 강제 통과를 공간 포함으로 오해하지 않는다. 원래 membership 순서, invalid camera fail-open과 기존 grace 의미는 유지한다. 새 소스 파일·프로젝트 등록은 없다.

기존 `out/SpatialLayer20261004`의 실제 생산 Layer 코드 native fixture를 재사용해 dirty/refit/remove/clone/shadow/grace/카메라 변경/경계 접촉을 검증하고, 동일 입력에서 이전·수정 코드의 정지·이동·전체 표시 CPU 비용을 비교한다. 이 변경은 CPU 후보 탐색 비용 감소이며 draw 감소나 실제 FPS 개선으로 대신 보고하지 않는다.

## G09. 반복 모델 제출과 애니메이션 계산 공유

동일 CMesh/CMaterial을 공유하는 정적 모델은 공간 셀별 가시성 결과를 유지하면서 연속한 렌더 목록의 instance payload를 하나로 모은다. 기존 lighting bank의 single-mesh/8개 제한을 동일 리소스 재사용에 적용하지 않는다. 각 submesh의 LOD 선택과 chunk claim이 일치하는 경우에만 결합하며, 반사 parity·render profile·공통 시간·진단 모드의 경계를 보존한다. 식생의 개별 world/wind payload도 복사하여 기존 shader를 그대로 사용한다. 업로드 실패는 원래 제출을 유지하고 실제 draw 이후 실패는 재제출하지 않는다. Profiler에는 결합 전 source draw와 실제 결합 draw를 구분해 기록한다. 이는 이전 버전 대비 감소율을 자동 의미하지 않는다.

정적 Layer의 카메라·bounds·topology가 같으면 수집된 후보를 재사용하고, 움직이는 카메라에는 부모가 완전히 포함된 frustum 평면 검사를 자손에서 생략한다. grace와 그림자 후보 및 객체 Submit 호출은 유지한다.

기존 CAnimation sample reuse 경로를 authored NPC에 연결해 같은 clip과 정확히 같은 track time의 local pose 계산을 공유한다. 각 NPC의 clock·blend·root motion·게임플레이 authority를 유지한다. 환경 파티클은 같은 particle-root 행렬의 inverse를 반복 계산하지 않도록 정확한 16float key의 thread-local 단일-entry cache를 사용한다. 입력이 다르면 기존 계산을 실행한다.

실제 production 함수 기반 수치 fixture로 결합 개수·LOD·claim·실패 보존, visibility 후보 동등성, animation/행렬 결과 동등성을 확인한다. 전체 Client/UI 실행과 FPS 검증은 실행하지 않는다. 기존 C++ 파일만 확장하여 project 항목 추가는 없다.

## G10. 동일 geometry의 여러 mesh에 조명 bank 인스턴싱 확장

기존 조명 bank는 하나의 mesh 모델만 합쳐, 같은 geometry를 쓰는 다중 mesh 모델이 경로에서 제외된다. `CModel::Can_BatchStaticLightingWith`는 모든 mesh의 동일한 immutable geometry 및 대응 material의 기존 호환성을 확인하고, `Bind_StaticLightingBank`는 선택 mesh index의 material을 사용한다. Client의 인접 prefix 경로는 모든 mesh의 chunk claim 부재와 원래 개별 LOD 일치를 준비 단계에서 확인한 뒤 mesh별로 기존 bank shader를 제출한다. 일부 mesh만 지원되는 모델은 원래 경로를 유지한다.

이미 성공한 mesh draw 뒤의 실패를 원본 전체 재제출로 처리하지 않는다. 첫 draw 전 실패는 원본 fallback, 이후 실패는 오류와 bank 상태 복구로 처리한다. 기존 최대8개·인접 순서·small bank·profile·clock·mirroring 경계는 유지한다. 기존 실제 함수 추출 fixture를 확장해 mesh별 material/LOD/index, prefix barrier, claim, 중간 실패와 payload 보존을 검사한다. 설치 inventory의 지원 가능 모델과 실제 게임 draw 감소를 구분한다.


## G11. 반복 geometry의 정적 batch 생성 순서 정리

1. 목표와 종료 증거: 같은 geometry의 RNM 변형이 asset ID와 mirror 순서 때문에 서로 떨어지는 장벽을 줄인다. 설치 전체맵의32m batch를 기준으로 현재 동일-instance 우선+조명 bank 후보18,526draw가 geometry/mirror 순서의 제한 재배열에서16,823draw로 줄었다. 동일 LOD·chunk 미claim·전체 저작 visible 조건의 구조적 계산이며 카메라 FPS가 아니다. 실제 생성 순서와 stable placement ID 보존을 검증한다.

2. 수정 파일: `Client/Private/MapPlacementRuntime.cpp`의 `Stage_PlacementRuntime`만 바꾼다. Level 소유 map layer 생성 순서가 책임이며 Renderer의 전역 제출 정렬은 추가하지 않는다.

3. H 계약: public/private 선언과 저장 계약은 바꾸지 않는다. 기존 Bern `spatialChunks` 입력이 false인 다른 맵·authoring 재생성은 기존 순서를 유지한다.

4. include·상태: 기존 map의 entry pointer 목록과 정렬 가능한 slot index를 함수 local vector에 보관한다. 참조 대상은 같은 함수의 `groups`와 변경하지 않는 catalog이며 함수 밖에 보존하지 않는다. 추가 include·enum·멤버는 없다.

5. 함수 책임: `Stage_PlacementRuntime`은 기존 asset+mirror+32m cell grouping 뒤 source BG deferred 후보의 생성 위치만 재배열한다. material override가 있고 모든 override가 SOURCE_BG_OPAQUE_MASKED인 group만 후보이며 나머지 group의 slot과 상대 순서는 그대로 둔다. 실제 조명 bank 허용 여부는 기존 CModel/CMaterial 비교가 계속 판정한다.

6. 호출 흐름: Load_Area → 기존 group 생성/검증 → Bern에서 후보 entry만 modelRelativePath·mirror·기존 key 순 stable_sort → 원래 후보 slot에 대입 → 기존 clone/Build_StaticInstance/Add_GameObject_to_Layer → 기존 stable-ID lookup/outPlacements commit이다. geometry·cell·instance payload·LOD·재질·그림자·실패 rollback을 바꾸지 않는다.

7. 작성 순서: iterator 목록을 만든 후 후보 slot과 immutable catalog entry를 수집하고 정렬한다. 기존 생성 루프는 정렬된 iterator 목록만 소비한다. 새 모델 복제·material hash·GPU buffer는 추가하지 않는다.

8. 검증: 실제 production 정렬 블록을 native fixture로 확인하고 설치 전체 group/placement ID가 정확히 한 번 유지되는지, unsupported slot·mirror·cell·다른 맵 순서와 빈/단일/혼합 입력을 검사한다. 입력 inventory와 분석은 `out/BernMaterialChunkAudit20261005/stage-order.json`에 보존한다. 기존 CPP만 바꾸므로 project/filter 추가가 없고 Debug·Release 제품 빌드는 통합 담당자가 수행한다. `git diff --check`와 UTF-8/CRLF 보존을 확인하며 Client/UI 실행·최종 화면·FPS는 미실행으로 기록한다.


## G12. 서로 다른 표면의 atlas proxy와 source별 가시 index 준비

Engine은 기존 `CModel` 경로 안에 `Model_StaticProxy.inl`을 추가하고 `Model.h`에 전용 build·source range·atlas/metadata·visible index 계약을 선언한다. 원본 `STATIC_CLUSTER_SOURCE` 입력과 geometry freshness 검사를 재사용하며 각 source의 CModel pretransform, UV0/1/2, TBN, color와 instance world/조명224byte를 보존한다. xatlas를 MIT 고정 commit의 원본 source/header/license로 포함하고 project/filter에 필요한 항목만 추가한다. 연결된 chart의 xref로 기존 vertex 채널을 복원하고 별도12byte `atlasUV + sourceIndex` stream을 만든다. 서로 다른 source 재질을 같은 atlas에 담되 source별 bake draw 범위와 stable ID를 유지한다.

한 proxy는 최대256source·geometry64MiB를 넘지 않고 고정 해상도 한 atlas로 pack되지 않으면 원본으로 복귀한다. GPU bake 자체와 shader는 Client 준비 단계의 별도 소유이며 builder는 geometry/layout만 반환한다. atlas SRV와8byte source metadata는 명시적 setter로 stage/commit하고 runtime bind는 준비 여부를 확인한다. 원본 picking과 authoring 데이터는 유지한다.

near 원본 index를 source별 연속 범위로 보존한다. Client가 이미 준비한 실제 visible source slot 목록을 전달하면 선택 범위만 dynamic IB에 복사해 제출한다. 같은 정렬된 선택 목록은 비교만 하고 재할당/재업로드하지 않는다. Map 실패·할당 실패에는 이전 selection과 buffer를 유지하며 빈 목록은0draw다. 첫 버전은 원본 near geometry만 지원하므로 원본이 낮은LOD를 선택하면 Client가 기존 배치로 복귀한다. CModel clone은 immutable geometry만 공유하고 선택/동적IB는 독립이다.

검증은 실제 xatlas와 생산 helper를 이용한 native fixture, 실제 설치 WModel geometry·UV/TBN/xref·source ranges·subset index 보존, atlas 경계/단일 page·예산 실패·동일 signature/빈 선택·잘못된 slot·업로드 실패 보존을 확인한다. 변경 XML parse와 git diff check, 최소CPP 및 통합 Debug/Release 제품 빌드를 수행한다. UI/Client 실행은 하지 않고 화면 품질·실제FPS는 사용자 확인 항목이다.


## G14. 큰 baked proxy의 Client 소유와 가시 source 선택

1. 목표: 작은 파생 청크를 함께 유지하지 않고64m 구역의 큰 proxy만 준비한다. 설치 데이터 계산에서는 raw64m 병합이 저장 카메라의 indices를19~51배로 늘렸으므로 proxy도 기존 카메라에 보이는 source의 index 범위만 제출한다. 준비 전에 현재 인스턴싱 이후 최소16draw, runtime에서 최소8draw를 대체하고 원본 선택 LOD의 indices를 넘기지 않는 경우만 사용한다.

2. 수정 파일: `MapStaticChunkObject.h/.cpp`가 coarse owner·proxy member 선택·기존 claim 수명을 소유한다. `MapStaticBatchObject.h/.cpp`는 이미 계산한 visible payload에 대응하는 local instance index를 transactionally 공개한다. `MapPlacementRuntime.cpp`는 Bern에서 새 proxy Stage만 호출하며 기존 fine Stage를 건너뛴다. GPU bake/cache 서비스와 CModel proxy ABI는 통합 담당자의 기존 Engine/Client 수직 경계로 연결한다.

3. H 계약: batch의 visible index view는 같은 frame/camera에 준비된 committed payload와 일대일 대응하며 다음 성공 준비 전까지 유지된다. coarse claim은 member별 active bit와 frameActive를 함께 확인한다. member는 batch+mesh 전체이고 부분 source를 대체하지 못하면 그 member 전체를 원본으로 되돌린다.

4. 상태: 기존 owner당 source-slot mapping·재사용 visible-slot vector·member bit vector를 추가한다. placement별 GameObject는 만들지 않는다. source slot은 cache/model 안에서만 쓰며 저장 identity는 stable placement ID와 mesh index다. 편집·가시성·suppression 변경은 기존 claim 무효화로 원본을 보존한다.

5. 함수 책임: `Stage_Proxy`는64m 후보를 source/geometry/atlas 예산으로 한정하고 모델과 bake 성공 뒤에만 object/claim을 commit한다. `Resolve_ProxySelection`은 기존 batch visibility cache와 원본 LOD를 재사용해 source slots를 만들며 Engine의 compact index 준비 성공 뒤에만 claim을 켠다. `Render`는 proxy의 완성된 atlas와 compact IB를 기존 CModel 경로로 제출한다.

6. 흐름: Bern Load → 기존 source batch 생성 → coarse grouping → CModel proxy geometry 준비 → GPU baker/cache 성공 → coarse object 생성 → claim publish → frame/camera당 source visibility 재사용 → 최소8draw·indices 비증가 확인 → compact IB 준비 → proxy draw다. 어느 준비 단계라도 실패하면 원본 optimized instancing을 유지하고 fine chunk를 추가 생성하지 않는다.

7. 순서: visible index와 claim 멤버 계약, owner Stage/selection, baker/Engine 연결, Bern 단독 호출 순서로 반영한다. source shadow·picking·stable ID lookup과 다른 맵은 기존 경로를 유지한다.

8. 검증: production visibility/selection 본문으로 payload-index transaction·cache hit·Map failure·LOD fallback·최소8draw·source 중복/누락·편집 invalidation을 검사한다. 실제 Engine compaction/bake 검증과 통합 Debug·Release 빌드는 해당 구현 담당자가 수행한다. Client/UI 실행·시각 품질·FPS는 자동 완료로 기록하지 않는다. 형식은 UTF-8/CRLF를 보존하고 새 service/shader 등록은 통합 담당자가 처리한다.

9. 기본 활성화 경계: 실제6개 저장 카메라에서16texel/m·1texel/pixel 보수적 품질 조건을 통과한 source가0개였다. 따라서 `MAP_CHUNK_POLICY::atlasEnabled=false`로 명시적으로 비활성화하고 신규 atlas 준비·owner 생성·source index 추적을 수행하지 않는다. Bern은 기존 fine Stage도 호출하지 않으며 최적화된 원본 인스턴싱을 사용한다. 이 후보의 소스·ABI를 보존하지만 성능 완료로 기록하지 않는다. 사용자 우선순위인 가려진 geometry의 current-frame occlusion 검토로 후속 작업을 옮긴다.

## G15. 정적 재질 입력 atlas 준비와 파생 cache

서로 다른 SOURCE_BG 재질은 원래 diffuse/normal/specular와 RNM diffuse indirect를 UV 공간에서 한 번 평가해 여섯 atlas로 준비한다. 시점에 따라 달라지는 RNM specular는 원래 TBN과 카메라로 계속 계산한다. 반사·parallax·rim·subspecular·masked·wind·시간 변화 재질은 원본을 사용한다. 팀장 렌더링 옵션을 변경하지 않는다.

Client의 새 MapStaticProxyBaker는 기존 CModel과 CMaterial 바인딩을 소비한다. D3D11 context state를 격리하여 UV draw 후 복구하고, chart 경계 texel 확장과 제한된 mip을 준비한다. geometry/UV identity, source 입력과 실제 texture 파일 SHA256, compiled shader를 cache key로 사용한다. 파생 cache는 개인 LocalAppData/LostArk/MapProxy에 두고 실패·손상·지원 불가면 원본으로 돌아간다. 최초 준비와 cache 재사용 비용을 구분하며 매 프레임 재베이크하지 않는다.

추가 H/CPP와 proxy HLSL을 Client 프로젝트와 기존 filter에 등록한다. geometry·visibility·shader·cache 무결성 검증 후 Debug/Release 제품 빌드를 수행한다. GPU 수치 probe는 제품 화면 및 FPS 확인과 구분한다. 원본 데이터와 렌더링 설정은 쓰지 않는다.

## G16. 현재 카메라의 보수적 CPU occlusion kernel

1. 목표: GPU readback 대기 없이 현재 프레임의 불투명 원본 삼각형으로 가림 버퍼를 만들고, 확실하게 가려진 world AABB만 제출에서 제외한다. 카메라 이동·near plane·불확실 입력에는 visible로 복귀한다.

2. 파일: 새 `Engine/Public/OcclusionCuller.h`, `Engine/Private/OcclusionCuller.cpp`와 Apache-2.0 Intel MaskedOcclusionCulling 고정 commit `1fd7974456cffa481a1a534328a1d02523d19ce8`의 source/header/inl/license/README를 추가한다. Engine vcxproj와 기존 필터에 필요한 항목만 등록한다. 기존 CModel CPU picking geometry 차용 adapter와 Renderer/Client 선택은 해당 소유자가 별도로 연결한다.

3. public 계약: `Begin(view, projection, viewportWidth, viewportHeight)`는 현재 카메라·viewport 픽셀과 같은 표본의 최대4096×4096 버퍼를 준비/clear한다. `Rasterize(positions, vertexCount, strideBytes, indices, world, cullMode)`는 수명 동안 차용한 검증된 원본 positions/index만 소비한다. `TestBounds(worldMin, worldMax)`는 occluded인 경우에만 true를 반환한다. 입력/실제 raster triangle 수를 분리해 비용과 가림 기여를 기록한다.

4. 상태: pimpl이 MOC와 재사용 clip/triangle/index scratch를 소유한다. 한 프레임 입력 삼각형은 최대100,000개로 제한하고 near-plane 교차/뒤쪽/비유한 삼각형은 가림 입력에서 제외한다. vector는 다음 프레임에도 capacity를 재사용한다. opaque 자격과 실제 표시 LOD의 일치는 호출자가 검증한다.

5. 안전성: perspective 행렬만 허용하고 MOC의1/w 깊이가 D3D z 깊이와 같은 순서인지 검증한다. 낮은 해상도의 픽셀 중심만 채우면 작은 틈을 덮을 수 있으므로 처음의 개별 삼각형 축소안은 공유 내부 경계의 인위적 틈 때문에 제외하고 viewport와 같은 픽셀 중심에서 rasterize한다. 저장소를32×8단위로 올림한 경우 clip XY를 viewport/storage 비율과 offset으로 변환하여 원래 픽셀 중심을 유지한다. 기여 depth는 해당 삼각형의 가장 먼 w를 사용한다. 원본에 없는 면적이나 더 가까운 깊이를 occluder로 만들지 않는다. 가림 대상 사각형은2pixel 확장하고 가장 가까운 w보다 앞쪽으로 bias한다. near-plane과 교차하는 대상은 visible다.

6. 구현: D3D 좌표·PRECISE_COVERAGE와 CLIP_ALL을 사용한다. 기본 x64/SSE2와 AVX2 TU를 분리하고 runtime CPU feature 선택을 사용한다. AVX512는 빌드 설정으로 끄고 원본 fallback TU를 등록하여 optional ISA가 기본 TU에 섞이지 않게 한다. allocation/지원 불가에는 그 프레임 가림을 비활성화한다.

7. 흐름: final camera → Begin/clear → bounded opaque source raster → world bounds query → 원본 제출 목록에서 알려진 가림만 제거다. shadow·picking·stable ID·저장 데이터·팀장 렌더링 옵션에는 쓰지 않는다.

8. 검증: 실제 생산 kernel과 vendor를 native로 컴파일하고 정면벽/틈/near-plane/뒤쪽/잘못된 행렬·index/카메라 이동·budget 경계를 검사한다. SSE2와 AVX2 결과를 대조하고 설치 geometry의 카메라 fixture에서 제출 후보 수와 kernel 시간을 기록한다. XML parse, diff check, UTF-8 인코딩 유지 및 통합 제품 빌드를 수행하되 Client/UI/FPS 검증으로 대신 주장하지 않는다.

## G17. Bern의 가려진 배치와 먼 소품 제외

1. 목표: 작은 파생 청크를 추가하지 않고 이미 제출될 원본 정적 batch를 사용한다. Renderer의 NONBLEND 목록에서 현재 frame의 실제 불투명 geometry로 가림을 판정하고, 확실하게 가려진 batch 전체만 제외한다. shadow queue와 picking·저장 배치·Server authority는 그대로 유지한다.

2. Client 파일: `MapStaticBatchObject.h/.cpp`의 virtual occlusion callback과 기존 visible-upload loop를 확장한다. Engine의 CGameObject descriptor, CModel CPU geometry view, COcclusionCuller와 Renderer filtering은 통합 담당자와 Engine 담당자가 연결한다. Client에 별도 per-placement owner나 새 데이터 문서를 만들지 않는다.

3. callback 계약: `Try_GetStaticOcclusionDesc`는 Bern shared-frame의 성공한 committed visible payload와 보수적 bounds를 제공한다. 모든 mesh가 변형 없는 static SOURCE_BG인 batch를 occludee로 허용하며 masked 표면도 포함한다. 가림 근거인 occluder는 opacity texture·alpha clip·wind가 없는 불투명 표면만 허용한다. visible instance의 local AABB를 world로 보수적으로 변환한 합집합을 사용한다. `Rasterize_StaticOccluder`는 실제 visible instance·material/cull·선택LOD를 확인하고 실제 CPU 원본 triangle을 제한 budget 안에서 제출한다. 원본이 LOD1/2를 선택한 mesh는 LOD0으로 가리는 오류를 피하기 위해 occluder에서 제외한다. CPU geometry는 기존 immutable picking geometry를 차용하며 GPU readback이나 새 geometry 복사를 하지 않는다.

4. 거리 계약: transient `MAP_VISIBILITY_SETTINGS`의 enabled/scale/revision을 소비한다. Bern의 변형 없는 정적 소품에만 거리 조건을 넣으며 큰 bounds·landscape·background와 gameplay object는 제외한다. 카메라 또는 설정 revision이 바뀌면 같은 camera에서도 visibility를 다시 준비한다. source geometry·texture 품질과 팀장 저장 옵션은 바꾸지 않는다. 입출력 판정은 sphere 표면까지의 실제 거리로 계산하고 invalid 수치는 fail-open한다.

5. 기존 transaction: distance/frustum을 통과한 payload·visible bounds·source index를 기존 staging에서 준비하고 성공한 GPU upload 뒤에 함께 commit한다. 실패하면 이전 committed payload를 보존한다. 거리 counter는 검사/제외 instance 및 제외 원본 indices를 구분하며 indices는 draw 절감이나 GPU 시간으로 표현하지 않는다.

6. 검증: 실제 production callback/predicate를 추출한 native 검사로 camera 변경·설정 toggle·revision·near plane·반사·비균일 scale·지원하지 않는 material·원본LOD·실패 보존을 확인한다. 설치6개 export camera에서 거리/radius 및 occlusion 후보 분포를 확인하며 이는 실제 frame trajectory/FPS가 아니다. 최소 CPP 컴파일·통합 제품 빌드 후 실제 화면 판정은 사용자에게 남긴다.


## G18. Occlusion culling 우선 구현과 거리 컬링 계측

사용자 우선순위에 따라 atlas 활성화를 중단하고 현재 프레임의 가림 제거를 먼저 구현한다. 기존 atlas 후보는 저장된6카메라에서 요구 texel 밀도를 충족한 source가0개였으므로 기본 활성화하지 않는다. 기존 작은 청크도 함께 생성하지 않는다.

Renderer는 최종 카메라 준비 뒤 정적 맵의 실제 불투명 geometry만 제한된 CPU depth buffer에 래스터화하고, 전체 visible batch bounds가 가려진 경우 NONBLEND 제출 목록에서 제외한다. Intel MaskedOcclusionCulling의 보수적 SIMD 경로를 사용하며 GPU query/readback 대기는 없다. 실제 viewport의 픽셀 중심과 일치하는 full-resolution CPU 표본, 삼각형의 가장 먼 depth와2pixel 확장 query bounds를 사용한다. 개별 삼각형 축소는 공유 내부 경계에 틈을 만들므로 사용하지 않는다. near/far plane·카메라 내부·비정상 projection은 표시를 유지한다. 그림자·UI·게임플레이 객체는 이 목록 교체로 변경하지 않는다.

현재 화면의 후보만 처리하고 occluder64배치·전체 입력100,000삼각형·callback당16,384삼각형과 callback 사이1.5ms soft budget을 사용한다. 한 callback 자체와 clear/query 비용까지1.5ms 이내라고 보장하지 않는다. 카메라 view/projection·viewport·설정 revision·weak owner·geometry revision을 포함한 descriptor 전체가 같을 때만 결과를 재사용한다. 카메라나 배치가 바뀌면 현재 프레임 깊이를 다시 만든다. G-buffer를 그리기 전에 stable filter를 적용하여 기존 인접 인스턴싱이 남은 항목을 계속 합칠 수 있게 한다. 반투명·masked·wind·움직이는 geometry를 occluder로 쓰지 않으며 GPU가 선택한 LOD와 다른 geometry도 가림 근거로 쓰지 않는다.

Distance culling은 별도 단계다. Bern 정적 작은 소품·식생의 크기별 거리를 기준으로 제외하고 큰 지형·건물·배경을 보존한다. 거리35/45/60m와 투영 지름24pixel 조건을 함께 요구하며, 숨김 진입에는 거리1.1배·pixel0.9배의 hysteresis를 둔다. F7에 독립 ON/OFF·거리 scale·pixel 상한을 제공하고 기존 저작 rendering options는 수정하지 않는다. viewport 변경도 visibility cache를 무효화한다. 같은 카메라에서 옵션을 바꾸어도 visibility cache를 무효화한다. 거리 제거·가림 제거·오클루더 삼각형·CPU 시간을 분리 계측한다.

검증은 실제 코드의 보수성·빈/잘못된 입력·실패 시 원본 보존·카메라 이동·경계·그림자와 인스턴싱 유지, 설치 Bern 모델과 저장 camera 입력의 처리량 및 비용, Debug/Release 제품 빌드를 포함한다. Client/UI와 실제 FPS 확인은 자동 실행하지 않는다.


## G19. 공통 CPU 작업 풀

1. 목표: 기존 Effect particle의 동기식 caller-assist 실행기를 Engine의 공통 CPU job 경계로 옮긴다. 프레임 작업마다 thread를 새로 만들지 않으며 기존 particle의 한 helper 정책을 보존한다. NPC·Effect occurrence 전체를 자동으로 worker로 옮기지 않는다.

2. 파일: 새 `Engine/Public/CpuJobPool.h`와 `Engine/Private/CpuJobPool.cpp`가 public API와 persistent Windows thread pool을 소유한다. `Engine.vcxproj`와 기존 Utility filter에 두 파일을 등록한다. 기존 `Client/Private/Effect_ParticleUpdatePool.cpp`는 같은 public 함수와 profiler scope 이름을 유지하는 얇은 wrapper로 교체한다.

3. H 계약: `CPU_JOB_STATS`의 `CallerJobs`와 `WorkerJobs`는 각 lane에서 callback이 성공 반환한 수이며 `Assistants`는 실제 제출한 worker callback 수다. `Run_CpuJobs(count, context, execute, maxAssistants, profiler, workerScope, joinScope)`는 모든 실행 callback이 끝난 뒤 반환한다. 호출자가 입력 수명·각 index 출력의 독점 소유·최소 작업량 정책을 보장하며 worker는 Direct3D immediate context에 접근하지 않는다.

4. 상태: 프로세스 수명의 pool 하나가 최대3helper(작은 CPU에서는 processors−2 이하)와 제출 mutex·현재 batch atomic pointer를 소유한다. batch는 stack에 작업 수·callback·atomic cursor·성공 counter·첫 exception을 보관한다. profiler와 scope 문자열은 반환 전까지 유효한 차용 입력이다.

5. 실행: count0은 빈 결과, count1·maxAssistants0·nested call·pool 준비 실패는 caller에서 직렬 실행한다. caller와 worker 양쪽 callback에 thread-local 재진입 guard를 두어 중첩 작업이 자기 pool의 mutex/join을 기다리지 않게 한다. 동시 외부 호출은 제출 mutex에서 직렬화하고 각 caller는 자신의 batch에 참여한다.

6. 실패: Windows pool/work 생성 실패는 직렬 fallback이다. callback 예외는 첫 실패를 보관하고 새 job 획득을 중단한 뒤 이미 실행 중인 lane을 모두 join하고 caller에서 다시 던진다. 성공한 callback의 외부 쓰기를 되돌리지 않으므로 transaction이 필요한 소비자는 index별 staging을 유지한다. destructor는 pending callback을 정리한 뒤 work/environment/pool을 해제한다.

7. Client 연결: `Run_EffectParticleUpdates`는 maxAssistants1과 `Effect.Particle.Update.Worker/Join` scope를 전달한다. 기존 emitters4개·particles512개·portable/death event 배제와 spawn/provider/event 순서, cross-emitter DirectLoc/collision 이후 barrier는 변경하지 않는다. 모델·에셋 로딩의 COM/취소 I/O pool은 별도 계약을 유지한다.

8. 검증: 실제 production pool로 단일·다중·0/1job·maxAssistants0/1/3·중첩 caller/worker·예외 join·동시 외부 제출·API 실패 fallback을 검사한다. Debug/Release native stress와 기존 particle wrapper compile, project/filter XML parse·파일별 인코딩·diff check를 수행한다. broad workload/FPS 개선은 별도 소비자 benchmark와 사용자 capture 없이는 주장하지 않는다.

## G20. 정적 맵 batch의 CPU 준비와 GPU commit 분리

1. 목표: 확정된 카메라에서 기존 frustum·거리·LOD·instance packing 결과를 동일하게 유지하며 큰 후보 묶음만 공통 CPU pool로 처리한다. 작은 batch마다 thread/job을 따로 제출하지 않는다. 정지 cache hit는 reserve와 CPU 계산을 수행하지 않는다.

2. 파일: 기존 `MapStaticBatchObject.h/.cpp`에 opt-in 가상함수와 owner stage·독점 CPU execute·owner commit을 추가한다. `MapAssetRenderUtils.cpp`의 정규화 plane 검증 cache 네 변수만 thread-local로 바꾸며 camera capture 전역 cache는 owner 전용으로 유지한다. 새 제품 파일이나 project/filter 등록은 없다.

3. H 계약: `Try_PrepareFinalCameraCpuJob`은 Bern의 기존 batch만 참여시키고 `FINAL_CAMERA_CPU_JOB`의 context는 해당 batch를 차용한다. Layer의 동기 join이 객체 수명을 보장한다. private preparation 상태는 최초 실제 rebuild 때만 생성하고 frame·camera revision·viewport·settings·inspection gate를 보존한다. camera 본문은 동기 CPU 실행이 끝날 때까지만 차용하고 READY/FAILED에는 pointer를 비운다.

4. 상태: preparation은 차용 camera pointer와 별도 revision·settings 값, CPU 완료 상태·HRESULT, 재사용 frustum scratch와 결과 bounds/counter를 소유한다. 기존 candidate GPU payload와 source index vector는 그대로 재사용한다. staged frustum와 거리 hysteresis도 성공한 GPU payload와 함께 commit하므로 Map 실패와 재시도에서 grace를 두 번 소비하지 않는다. authoring 변경은 준비 상태를 무효화한다.

5. 함수 책임: owner Stage는 전역 입력 포착·batch bounds rebuild·모든 scratch reserve를 끝낸다. 카메라 밖 broad reject는 상세 preparation 할당 전에 원래 빈 payload로 바로 commit한다. 확정된 빈 결과에는 GPU Map 실패점이 없고 source별 상태는 건드리지 않는다. camera-only BVH 후보만 Stage job 수집에 참여하며 shadow-only 제출은 기존 owner 경로로 보존한다. Execute thunk는 예외를 batch HRESULT로 저장하고 다른 job을 중단하지 않는다. CPU 본문은 배치 입력만 읽고 임시 payload·bounds·frustum·counter만 쓴다. owner Upload는 실행되지 않은 staged job을 같은 본문으로 한 번 실행하고 Map/Unmap 성공 후 결과를 교체한다.

6. 흐름: Layer BVH 후보 → opt-in Stage → 큰 연속 cohort의 CPU 계산 → join → 기존 순서 Submit → owner Map/commit이다. OFF·적은 작업·dispatch 실패도 같은 CPU 본문을 owner에서 사용한다. CPU 실패는 기존 committed payload를 보존하고 같은 frame 반복 소비 시 실패를 그대로 반환한다. 다음 frame/입력 변경은 다시 준비한다. GPU 업로드 실패는 완성된 CPU 결과를 유지해 같은 frame에 재계산 없이 재시도한다.

7. 구현 순서: 상태/선언 → 입력 snapshot과 cache → CPU 본문 이동 → 단일 commit → thread-local plane cache → Layer와 연결한다. shadow·picking·draw 순서·원본 인스턴싱·게임플레이 update는 옮기지 않는다.

8. 검증: 실제 production 함수와 공통 pool을 이용한 직렬/병렬 payload byte·source index·bounds·grace·거리 결과 비교, cache와 옵션/viewport/inspection 무효화, broad 빈 commit와 cachehit counter의 frame당1회, 미실행 staged fallback·CPU 예외·Map 실패 재시도·authoring 무효화를 확인한다. 설치 Bern의 실제 batch 크기 분포로 dispatch/join 포함 시간을 비교하고 최소 CPP 컴파일과 통합 Debug/Release 빌드를 수행한다. Client/UI·실FPS는 실행하지 않는다.
## G21. Layer의 큰 CPU 준비 작업과 소유 스레드 제출

1. 목표: 기존 final-camera BVH 후보만 대상으로 CPU 가시성·인스턴스 payload 준비를 분산한다. 후보 탐색·불변 입력 포착과 GPU Map/Draw·렌더 큐 변경은 소유 스레드에 남기며 정지 카메라 cache와 제출 순서를 보존한다. 작업 자체의 비용보다 분배 비용이 크지 않도록 작은 후보는 직렬 처리한다.

2. 파일: Engine GameObject.h의 opt-in job descriptor/virtual, Layer.h/.cpp의 후보 작업 묶기, Engine_RenderTypes/Renderer의 세션 설정과 Profiler counter를 확장한다. Client ProfilerTool/ProfilerCaptureIO는 A/B 설정과 실제 worker 작업량을 표시한다. 공통 pool과 MapStaticBatchObject CPU 분리는 G19/G20 소유자가 연결한다.

3. H 계약: FINAL_CAMERA_CPU_JOB은 수명 동안 빌린 Context·Execute·Cost를 담는다. Try_PrepareFinalCameraCpuJob은 소유 스레드에서만 호출하고 false면 기존 Submit 경로를 유지한다. Execute는 객체별로 독점인 CPU staging에만 쓰며 D3D·renderer·GameInstance mutable 상태를 건드리지 않는다. 모든 작업은 함수가 반환하기 전에 join되므로 Level 전환이나 다음 update와 겹치지 않는다.

4. 상태: Layer는 원래 camera/grace/shadow 제출 목록 외에 카메라 교차 CPU 후보 목록과 재사용 job/range vector를 소유한다. BVH 부모의 camera 교차 결과를 자식에게 전달해 shadow-only 하위 노드의 평면 재검사를 생략하고, 원래 그림자·grace 제출은 유지한다. 두 후보 목록은 같은 camera/dirty/shadow cache 경계에서 갱신한다. 수집 전에 capacity를 준비하고 입력 owner 수명을 기존 layer가 유지한다. count/cost가 작으면 같은 Execute를 직렬 호출한다. 대량인 경우 대략128instance 이상의 연속 배치 묶음, 최소8batch와 최대64batch 경계로 coarse range를 만든다. 총512cost 미만은 worker에 보내지 않는다. 실제 fixture에서 이득이 없으면 이 하한을 높인다.

5. 책임: Layer가 opt-in 작업 수집·pool 실행·join을 끝내고 원래 후보 순서로 Submit_FinalCamera를 호출한다. Client는 성공 CPU staging만 소비하고 GPU 업로드 성공 뒤 visible payload를 commit한다. 할당/작업 실패는 기존 소유 스레드 준비와 이전 commit을 보존하며 중복 gameplay update는 하지 않는다.

6. 흐름: final camera → 기존 BVH 후보/그림자 후보 → owner snapshot/stage → coarse CPU ranges → caller+최대3helper join → 기존 순서 GPU upload/queue submit → 가림/instancing/render다. parallel preparation은 Debug/Release 기본 OFF이며 OFF일 때 Layer의 job 수집·scope도 생략하고 기존 직렬 진입을 사용한다. 제품과 같은 최적화 옵션의 실제 Bern CPU fixture에서 ON의 안정적 순이득이 입증되지 않아 F7 비교용으로 유지하고 저작 rendering options를 저장하지 않는다.

7. 계측: F7 session switch와 capture metadata, opt-in batch/coarse job/caller job/worker job/submitted assistant counter, CPU Map.Visibility.Prepare/Dispatch/Join/Worker 구간으로 실제 참여와 대기를 구분한다. worker 개수만으로 전체 CPU 시간이 줄었다고 단정하지 않는다. 범용 fiber/work-stealing scheduler와 D3D deferred command list는 이번 변경 대상이 아니다.

8. 검증: 실제 production Layer scheduling/Client CPU 함수와 common pool을 사용해 직렬·병렬 결과/순서/서로 다른 camera/설정·cache·실패·동시·중첩 호출을 대조한다. 설치 Bern 분포를 반영한 workload에서 분배·join 포함 시간을 비교한다. 프로젝트 XML/diff check, Debug/Release product build를 수행하며 Client/UI·실FPS는 실행하지 않는다.




## G24. 새 컷신 캡처의 draw 폭증과 재질 호환 배치 정렬

2026-10-05 사용자 저장3종은 같은 Debug process39956에서 수집됐다. 컷신 frame253→254의 draw는2,216→10,004회, VS invocation은2.83M→11.96M으로 늘었고 PS invocation은30.01M→28.45M으로 줄었다. NONBLEND CPU는42.12→254.01ms다. 그림자 제출은0이며 가림 CPU 평균2.42ms를 주원인으로 단정하지 않는다. 첫 첨부 원경 이미지는 별도 JSON이 없어 해당 시점의 LOD 선택을 확정하지 않는다. export camera/settings는 프레임별 기록이 아니고 GPU elapsed에는 CPU 명령 공급 공백도 포함된다.

`MapPlacementRuntime.cpp`의 기존 source BG 후보 슬롯 정렬을 geometry·mirror·재질 호환 cohort·기존 stable key 순으로 보완한다. 변경 위치는 Bern spatialChunks Stage이며 unsupported slot과 placement별 payload/identity/32m visibility는 유지한다. 기존 CModel prototype clone을 asset ID당 한 번 준비하고 CModel::Can_BatchStaticLightingWith 실제 predicate로 호환 cohort를 정한다. 이 임시 model을 기존 group별 bounds 준비에도 재사용한다. 후보 간 비교는 같은 geometry·mirror·render profile 안으로 제한하고, 최종 허용은 기존 CModel/CMaterial runtime 비교가 계속 판정한다. 동일 geometry의 RNM 차이는 bank에서 처리하지만 다른 표면·시간·변형 입력은 같은 재질로 간주하지 않는다. 정렬키는 로드 준비에서 한 번 만들고 렌더 프레임에 비교 작업을 추가하지 않는다. material 필드를 복제하는 별도 serializer는 만들지 않는다.

설치 전체맵의 동일LOD·무claim·전체 authored-visible 조건에서 기존 순서16,822→material cohort 순서15,137 mesh draws의 구조적 여지를 확인했다. 실제 컷신의 추가 감소율이나 FPS로 환산하지 않는다. 실제 생산 정렬을 추출한 fixture로 unsupported slot·mirror·empty/single·stable placement 보존과 다른맵 무변경을 확인하고 설치 inventory를 재대조한다. 새 C++파일·project 항목은 없다.

## G25. 같은 RNM 조합의 bank 슬롯 재사용

`MapStaticBatchObject.cpp`와 `CModel`의 기존 인접 bank 제출을 확장한다. 최대8은 batch object 수가 아니라 shader에 바인딩하는 고유한 RNM average/directional/static-shadow 텍스처 조합 수로 적용한다. `Model.h/.cpp`의 비교 함수는 모든 submesh의 대응 material에 기존 Has_SameStaticLightingTextures를 적용한다. 한 source의 모든 mesh가 같은 representative와 일치할 때만 그 슬롯을 재사용하며 mesh별 slot의 의미가 달라지지 않게 한다.

최대8 representative model만 shader에 전달하고 더 많은 인접 batch의 visible instance payload를 같은 기존 dynamic buffer로 모은다. runtime geometry/material compatibility, profile·clock·mirror·LOD·claim과 buffer overflow 경계를 유지한다. 후보 수와 shader bank size를 구분하며 exact instance prefix의 handoff도 이 계약을 따른다. 모든 source가 RNM 하나로 모일 때의 pass 선택과 기존 원본 fallback을 검증한다. GPU 업로드 전 실패는 원본 제출, 일부 draw 이후 실패는 중복 재제출하지 않는 기존 계약을 지킨다.

actual 함수 fixture로9개 이상 같은 RNM, 혼합 RNM,8/9고유 조합 경계, 다중 mesh에서 한 material만 다른 조합, LOD/claim/mirror barrier, payload slot·small bank·실패 보존을 확인한다. 기존 shader ABI를 유지하고 새 source 파일이나 shader pass는 추가하지 않는다.

## G26. 유효한 중형 메시 LOD 확대와 파생 결과 재사용

현재8,192tri gate 아래 실제 설치 메시10종을512tri gate로 조사했다. 소형·중형7종은 기존 품질 조건에서 단순화되지 않았고,5,344/8,032/8,104tri의3종은 각각최대42.25/40.54/21.22% index 감소가 가능했다. 무조건512tri까지 확대하는 대신 `StaticMeshLod.h/.cpp`와 `Mesh.cpp`의 공통 최소값을4,096tri로 낮춘다.19개 정점 속성·UV 가중치100·LockBorder·기존 상대오차와 화면오차0.25pixel을 유지한다. 기존 material admission을 느슨하게 바꾸지 않는다. 추가 대조에서 sourceBgFlags64는 alpha clip이며 wind 배제가 아님을 확인했다. CModel 생성과 Client의 현재 draw 재질 재검사 양쪽에 sourceFoliageWind 배제를 명시해 변형된 표면에 정적 오차 추정을 적용하지 않는다.

동일 변환 geometry의 생성 성공 및 생성 불가 결과를 기존 CStaticMeshLod 안에서 재사용한다. persistent 파생 cache를 사용할 경우 원본 변환 정점·index와 알고리즘/품질 revision을 키로 검증하고, bounded payload·range/index/error·무결성 확인 뒤에만 immutable IB를 만든다. 쓰기는 개인 파생 저장소에서 임시 파일 후 원자적 교체로 처리하며 손상/실패는 원본 생성 또는 원본LOD로 복귀한다. Resources와 authored 데이터에는 쓰지 않는다. 초기 생성·warm 재사용·cache 손상·동시 준비를 구분하여 검증하며 저장소 실패가 맵 로드를 실패시키지 않게 한다.

실제 설치 geometry에 대해 동일 품질의 생성 결과, selection/invalid/near plane, cache invalidation·잘린 파일·잘못된 index·동시 요청과 Debug/Release 비용을 확인한다. 생성 성공은 실제 카메라의 low LOD 사용이나 FPS 개선을 의미하지 않는다. 기존 CPP/H만 확장하고 project/filter 신규 항목은 없다. 통합 담당자가 변경 source를 동결한 뒤 최소 컴파일과 가능한 Debug/Release product 빌드, XML/JSON parse 및 diff check를 수행한다. 실행 중 Client/Server는 종료·조작하지 않고 점유된 최종 바이너리 교체 제한은 결과에 별도로 남긴다.
