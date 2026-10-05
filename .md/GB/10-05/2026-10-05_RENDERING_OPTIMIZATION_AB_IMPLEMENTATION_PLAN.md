# Rendering Workbench 최적화 A/B 구현 계획

## G00. 목표와 현재 계약

사용자는 기존 Rendering Workbench의 복원·기법 비교와 같은 흐름으로 프러스텀, occlusion, LOD, batch, 그림자·애니메이션·동일 재질 등의 효과를 Debug/Release에서 측정하고 영상·기술소개서에 사용할 증거를 요청했다. 기존 RenderingBenchmark의 A/B 세션·ABBA·warmup·조건 검사·원래값 복원·비동기 JSON 저장을 확장한다. 별도 profiler나 제품 renderer를 만들지 않는다. 베른의 앞선 배치·LOD 변경과 함께 검증하되 측정 도구 구현을 실제FPS 개선으로 표현하지 않는다.

현재49개 실험 필드는61개로 확장한다. 기존 ID/순서와64bit field mask를 유지한다. 최적화 설정은 저작 rendering profile·Video·Resources·Data를 저장하지 않는 세션 상태다. Debug↔Release와 D3D debug device 변경은 프로세스 별도 실행이며, 같은 실행 중 체크박스로 build를 바꾼다고 표시하지 않는다.

## G01. 실제 작업을 켜고 끄는 Engine 계약

Engine_RenderTypes의 RENDER_OPTIMIZATION_SETTINGS와 Renderer→GameInstance의 Get/Apply API를 추가한다. FrustumEnabled, MeshLodEnabled, MapInstancingEnabled, IdenticalBatchEnabled, LightingBankEnabled, StaticShadowCacheEnabled, NpcPoseReuseEnabled, ParticleRootCacheEnabled, ParticleWorkersEnabled는 현재 구현과 같은 true가 기본이다. 기존 MAP_VISIBILITY_SETTINGS의 OcclusionEnabled/DistanceEnabled/ParallelPreparationEnabled와 거리 수치는 그대로 사용한다. 같은 입력의 Apply는 revision·cache를 건드리지 않는다.

프러스텀 OFF는 Layer의 공간 후보와 Client 상세 맵 가시성 양쪽에 적용하고 가시성 cache를 무효화한다. 거리·occlusion과 별도 변수다. LOD OFF는 실제 CMesh 선택기에서 원본 index를 선택하여 draw·bank 호환·occluder 판정이 같은 결론을 사용한다. 일반 instancing OFF는 같은 visible payload를 instanceCount1·224byte offset의 개별 draw로 제출하고 인접 병합을 우회한다. 이는 객체 생성 구조 전체를 과거 코드로 되돌리는 실험이 아니다.

Exact/lighting bank는 별도 gate로 비교하고 bank OFF에서는 exact prefix handoff도 끈다. 그림자 cache OFF는 그림자를 없애지 않고 정적 caster를 다시 계산하며, 그림자 자체가 OFF인 장면은 비교 조건을 충족하지 않는다. NPC는 기존 opt-in animation sample 재사용만 우회하고 clip/clock/event/Server 상태는 유지한다. particle root inverse와 particle worker OFF는 같은 계산의 직렬 원래 경로를 사용한다. 맵 worker의 기존 기본OFF는 유지한다.

수정 위치는 Engine의 Renderer/GameInstance/Layer/Mesh/Animation, Client의 MapStaticBatchObject/MapAssetRenderUtils/Effect_Playback/Effect_ParticleUpdatePool 및 필요한 기존 헤더다. 별도 source 파일·project 등록은 기본적으로 추가하지 않는다.

## G02. 최적화값의 세션 소유권과 복원

RenderingProfileService에12개 boolean field를 끝에 추가한다. 품질은 현재 camera environment에서 매 프레임 restore/apply되므로 최적화값은 그 cycle에 넣지 않는다. 별도 entryBase·lastApplied·ownedMask를 보관하고 실제 A/B 값이 바뀔 때만 setter를 호출한다. 매 프레임 cache를 비워 cache ON의 성능을 훼손하지 않는다.

같은 실행의 종료·취소·profile/level/video/region owner 변경에서 소유한 필드만 복원한다. 다른 제어가 소유 필드를 변경했으면 그 새 값을 덮어쓰지 않고 비교 변경/종료를 처리한다. 거리 scale·pixel 값과 선택하지 않은 모든 최적화는 보존한다. 두 설정 setter의 일부 실패는 이전값으로 rollback하고 복원 실패를 성공으로 지우지 않는다. LOD/occlusion/worker를 현재값으로 읽고 staging한 frame 경계와 warmup 뒤 실효값을 측정한다.

## G03. 기존 Benchmark의 최적화 비교 표

RenderingBenchmark에 공용 Render_OptimizationSection을 추가한다. 각 행에 기법, 실제 소비 경로, 적용 범위, A/B 적용, 측정, 원래값 복원, CPU/GPU 시간과 작업량 차이를 제공한다. 개별 항목은 OFF/ON 비교이며 다른 rendering options는 그대로 둔다. 각 비교의 나머지 옵션은 세션 시작 상태를 기준으로 보존한다. 맵 worker는 자기 항목의 명시적 비교에서만 켜며 전체 묶음 토글은 이번 구현 범위에 두지 않는다. 현재 source material/그림자/상위 instancing 조건이 없으면 이유와 비활성 상태를 표시한다.

runtime 선택 가능한12개와 준비 단계 항목을 구분한다.32m 공간 분할·생성 순서·geometry/atlas bake·LOD 생성 하한과 cold/warm cache는 맵 준비/로딩 실험이며 즉시 동작하지 않는 가짜 checkbox를 만들지 않는다. 현재 atlas 기본OFF/미검증 상태도 그대로 표시한다.

## G04. 비교의 실질적인 증거

기존 CPU/GPU/interval 평균·P50/P95/P99/max와 pass 통계에 전체 profiler counter의 안정 이름·유효 표본 수, CPU Work ledger의 시간/호출, IA/VS 수를 추가한다. source draw와 실제 draw, LOD 원본/제출 index, frustum/occlusion/distance 제외, shadow cache hit/miss, worker 참여를 구분한다. 대상이0인 항목은 현재 표본에서 효과를 입증하지 못했음을 표시한다. nested CPU/GPU/worker 구간은 합산하지 않는다.

field mask로 선택한 변수만 공통 조건에서 제외한다. 그 외 최적화 상태·거리 수치·camera·viewport·build·iterator debug·D3D debug layer·adapter·FPS limit·설정은 조건에 포함한다. 이름·measurement ID·A/B·반복 번호·frame range·warmup·GPU pending/invalid·조건 변경 사유를 JSON에 보존한다. 각 단계의 sample이 이력에서 퇴출되기 전에 증거를 보관하며 비교 결과와 연결한다.

기존 자동 ABBA는 고정 카메라 조건이다. 카메라나 장면 조건이 움직이는 컷신을 같은 조건의 통제 실험으로 승인하지 않는다. 고정 시점 측정과 움직이는 시퀀스 관찰 capture의 차이를 화면과 결과에 명시한다. NPC·이펙트의 시간 진행까지 완전히 결정적 replay된다고 주장하지 않는다.

## G05. Debug/Release와 영상 촬영 진입

MainApp의 기존 RenderingBenchmark 객체·Update·복원 수명을 공용으로 연결한다. Debug Workbench에 최적화 A/B 탭을 추가하고 두 build의 F1에서 Optimization Benchmark를 열 수 있게 한다. Release에 map/material 저작 Save/Publish·F7 profiler·일반FPS overlay를 추가하지 않는다. Benchmark 측정할 때만 기존 profiler를 켜고 이전 capture 상태를 복원한다.

영상용 숨김 유지가 명시적으로 선택된 최적화 세션만 F1로 UI를 숨겨도 현재 A/B 화면을 유지한다. 종료/Restore·level 변경·오류 시 복원은 계속 적용한다. 기존 렌더링 비교의 닫기 복원 계약은 바꾸지 않는다. 객체는 Engine 해제 전에 정리한다.

## G06. 구현 분담과 검증

Engine/실제 gate, ProfileService 소유권, Benchmark UI/집계, MainApp 진입을 파일별로 나눠 병행한다. 기존 개인 변경과 다른 기능의 dirty 파일을 보존한다. CPP별 기존 인코딩을 유지한다.

기존 production 함수 fixture를 확장해 각OFF가 실제 작업을 우회하는지, instance별 제출이 원본 payload/LOD를 보존하는지, cache transition·동일값 Apply·partial failure rollback·external owner·종료 복원·field fingerprint·ABBA sample 귀속·pending GPU·JSON parse를 검사한다. Debug/Release 최소 CPP와 정상 Product 빌드, XML parse, git diff --check를 수행한다. Client/UI를 자율 실행하지 않고 화면·영상·실제FPS 비교는 사용자 실행 단계로 구분한다. 하네스 성공이나 checkbox 생성만으로 실제 성능 향상을 주장하지 않는다.
