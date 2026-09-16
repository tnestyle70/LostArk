# 베른 전체 원본 geometry·조명 연결 계획

## G00. 원본과 기존 소비자

16개 원본 level의 StaticMeshComponent 32,324개와 InstancedStaticMeshComponent 1,697개, RNM texture 4,860개를 조사했다. static 950개와 instanced 전용 11개를 합친 961개 모델, static placement 32,324개와 foliage instance 17,651개가 원본과 대응한다. 전체 source placement 49,975개의 transform과 원본 규모는 root 비교에서 일치했으므로 변경하지 않는다. 기존 CModel, material variant, placementLighting 및 rendering profile 소비자를 확장한다. 별도 생성된 Landscape 42개는 원본 landscape component의 조명/UV 연결을 따로 조사한다.

## G01. geometry·RNM

961개 모델의 native triangle corner에 대응하는 UV1, COLOR0와 tangent W를 복원한다. 원본 component COLOR override는 해당 실제 색 스트림이 다른 geometry만 별도 모델로 저장한다. material signature와 RNM/SDF atlas가 다른 placement는 catalog variant로 연결하며 UV scale/bias와 RGB coefficient는 stable sourcePlacementId로 소비한다. Instanced 원본 VF의 UV1×componentScale+perInstanceBias 식을 그대로 사용하며, 사용되지 않는 component UV bias의 NaN을 실제 instance 값으로 잘못 해석하지 않는다. 모델의 GPU geometry는 동일 물리 WModel을 사용하는 material variant 사이에서 CModel clone을 통해 공유한다. 원본 퇴화 basis는 정상 basis로 위장하지 않고 실제 byte·corner 증거를 확인한 뒤 명시적인 보존 계약으로만 처리한다.

## G02. 환경·직접광

원본 PS의 WorldInfo, exponential fog, dominant directional light와 EF 환경 brush 다섯 개의 실제 convex 평면을 기존 scene profile로 연결한다. 생략된 값은 원본 class CDO로 resolve하며 현재 확인된 source 색·밝기를 유지한다. RNM GUID에 들어간 광원은 SOURCE_CHARACTER receiver로 연결하여 구운 배경과 중복하지 않는다. 기존 Bern 입구 10개 source 광원은 source ID 대응으로 갱신하며 중복 생성하지 않는다. Skybox 전용 skylight를 일반 배경이나 캐릭터 광원으로 확대하지 않는다.

## G03. 확인

native corner 대조, 원본·runtime geometry bounds 보존, source ID와 RNM atlas join, geometry 공유와 bounded catalog 수를 확인한다. Area publisher와 rendering profile publisher, 실제 C++ parse/Save, 수정 C++와 shader 최소 컴파일을 수행한다. Product 빌드는 root가 최종 수행하고 Client/UI 실행·화면 확인은 사용자가 직접 한다. 원본 CPU SH, postprocess 계수와 화면 동일성은 확인된 범위와 분리하여 결과에 기록한다.

## G04. 원본 정적 shadow

원본 ShadowMap2D의 texture, GUID와 placement별 UV를 실제 material/instance로 전달한다. source G8/LZ4 mip을 DDS R8_UNORM으로 보존하고 원본 distance-field PS의 bias/scale/exponent transfer를 사용한다. 원본 CPU penumbra setup은 보호된 실행 파일에서 확인할 수 없었으므로 width 0.05와 정규화 중심 가정은 명시적인 PROJECT_ADAPTER로 기록한다. ShadowExponent 2는 원본 Lightmass CDO 값이다.

각 source placement에는 dominant shadow가 정확히 하나다. scene main directional 1번과 12개 local dominant의 2~13번 channel을 authoring에서 연결한다. 기존 RGBA32_FLOAT PickPos.W의 normal/RNM mantissa는 유지하고 finite exponent에 channel을 저장한다. MRT4 alpha에는 occlusion만 저장하며 해당 channel의 direct diffuse/specular에만 곱한다. native RNM/ambient/emissive 값은 이 shadow를 중복 적용하지 않는다. 새 공통 shader include는 Engine/Client 프로젝트와 filters의 기존 shader None 옆에 등록한다.

## G05. 원본 환경 ParticleSystem 연결

현재 설치 원본의 실제 로드 16 level에서 ParticleSystemComponent 1,300개와 template 62개를 확인했다. 정적 물 재질과 별도로 누락된 물 환경 91개는 11개 ParticleSystem, 31개 emitter, 19개 MIC를 참조한다. 이 코호트에는 instance parameter와 actor base, sequence 활성화 참조가 없으며 source CDO의 bAutoActivate=true가 적용된다. 원본 TRS와 70m/100m CachedMaxDrawDistance를 mapeffects에 저장하고 기존 Effect source closure 및 native shader 복구 경로로 연결한다.

공통 MapEffectDocument에 LEVEL_ACTIVE 전용 SOURCE_LOOP/SOURCE_ONCE와 optional maxDrawDistanceMeters를 추가한다. source emitter의 EmitterLoops=0을 level 수명까지 지속하며 매 프레임 seek 또는 문서 duration 재시작을 하지 않는다. 기존 실제 level owner와 particle simulation을 확장하고 sprite/mesh 이외와 model cue는 이 정책에서 거부한다. SOURCE_LOOP은 최소 하나의 loop0이 필요하며 함께 있는 유한 emitter는 각자의 원본 loop 횟수에서 멈춘다. 전부 유한인 원본은 SOURCE_ONCE로 자연 재생하고 자동 반복하지 않는다. 문서 최대 배치 수는 2,048로 확장하되 기존 Effect scene 예산은 유지한다. 원본 거리값을 사용하는 actor-origin 거리 판정은 프로젝트의 activation adapter이며 원본 Primitive bounds culling과 동일하다고 판정하지 않는다. 범위 밖에서는 handle을 해제하고 재진입 시 새 occurrence를 시작한다.

CMapEffectPresentationRuntime의 동일 stage/commit 경로에 encounter 없는 Area 진입을 연결한다. ambient Area에서 surface owner나 Server window는 거부한다. 각 source loop는 실제 clone/prepared attach를 독립 검증하고, 활성 범위의 aggregate budget은 기존 runtime admission이 검사한다. Level loading에서 published mapeffects의 asset을 미리 준비하고 Level_Bern이 load/update/clear를 소유한다. 사용자 밝기 rollback profile과 현재 geometry/RNM 연결은 보존한다. C++ 최소 컴파일, 문서 roundtrip·publisher와 source native tuple/배치 join을 확인하며 화면 판정은 사용자에게 남긴다.

원본 fixed scalar/vector InstanceParameters는 기존 full restore의 DPM_NORMAL/ABS/DIRECT transfer로 fold한다. source template와 parameter signature로 stable Effect variant를 생성하며 root transform과 원본 recipe를 함께 보존한다. actor base, scripted activation, random/object parameter나 미지원 renderer를 default 값으로 위장하지 않는다. generic importer의 `0 or 1` 오류는 명시적인 무한 loop0과 누락값을 구분하고, CDO closure를 읽은 full restore에서는 생략된 native int0을 복원한다. 이미 편집된 문서는 덮어쓰지 않고 이번 작업에서 생성한 문서만 원래 candidate hash와 비교해 수정한다.
