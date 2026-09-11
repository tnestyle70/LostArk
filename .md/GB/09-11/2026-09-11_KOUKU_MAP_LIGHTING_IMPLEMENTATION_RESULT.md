# 쿠크 전체 맵 조명·geometry·환경 연결 결과

## G00. 실제 반영

PS/SL01~05의 원본 StaticMeshComponent 3,081개, LightMap2D 2,645개, LightMapTexture2D 640개와 광원 116개를 직접 읽었다. 실제 저작 배치와 대응하는 RNM 2,517개 중 지원 BG/overlay 2,364개에 atlas, coefficient, UV scale/bias를 연결했다. 사용자 승인 바닥 2개는 재질 행과 parameter를 그대로 유지하고 RNM에서 제외했다. 나머지 151개는 현재 지원 BG 프로그램 밖이다. LightMap1D를 임의의 2D atlas로 바꾸지 않았다.

305개 현재 static 모델에 native triangle-corner 대조를 거친 UV1·tangent handedness·COLOR0을 WModel 1.2로 설치했다. component 고유 COLOR override 8개는 별도 모델로 분리했다. UV0의 기존 payload와 WMAT section은 유지했다. 사용되는 RNM DDS 526개와 geometry 313개, 총 839개 Resources 파일/26,401,946 bytes가 실제 설치됐다. 동일한 역할의 두 번째 모델 runtime은 없다.

원본 source asset 330개 catalog에 RNM variant 971개를 추가해 catalog 1,301개가 됐다. 실제 사용 asset 310개 중 static 305개가 geometry 대상이며 나머지 Mario 5개는 skeletal source 경로다. material 행 1,732개(기본 377+variant 1,355), placementLighting 2,364개가 현재 Data 정본과 Area publish 출력에 연결된다. 원본 3,368개 placement의 stable ID·transform·visibility는 보존했다. 단일 catalog 제한은 2,048개, aggregate는 기존 4,096개다.

원본 local light 115개를 기존 사용자 광원 4개와 함께 연결했다. RNM GUID에 들어간 84개는 SOURCE_CHARACTER receiver여서 실제 보스·컷신·공의 native material pass가 수광하고 구운 BG에는 직접광을 중복 합산하지 않는다. 독립 local 31개는 ALL이다. 원본 Spot CDO에서 inner cone의 실제 기본 0도를 확인해 생략된 네 광원에 반영했고, parser·publisher·Engine에서 outer>0 및 0<=inner<=outer의 동일 계약을 소비한다. 원본 방향광은 RNM GUID에 한 번도 나타나지 않으며 기존 scene 방향광 하나에 source 방향/brightness를 연결했다. 기존 사용자 광원 4개의 수치는 보존했다.

## G01. fog와 환경

원본 Engine/EFGame .u CDO도 복호화해 EFEnvironmentInfoData의 기본값을 확인했다. 예를 들어 누락된 지역 inscatteringBrightness의 실제 기본값은 1이며 PS 전역 fog의 0을 상속하지 않는다. 11개 volume은 원본 BSP 6평면씩과 AABB, source CDO/instance fog 값, directional override와 WLE indirect brightness를 실제 RenderingProfiles에 포함한다. source BSP 88개 교차 vertex와 stored bounds 오차는 최대 0.001826 cm다.

MainApp의 camera update 뒤에 convex plane 내부 여부를 검사하고 가장 작은 겹침 volume을 선택한다. source BlendTimeIn/Out(기본 1초)를 사용해 현재 fog/scene light에서 새 값으로 선형 전환한다. fog와 light 적용 실패 시 기존 renderer 값으로 rollback한다. 정적 scene profile의 선택·품질·exposure와 원본 지역 데이터는 별도로 보존된다.

HEIGHT_FOG_SETTINGS의 optional sourceExponential 모델을 `Renderer -> Shader_SceneHeightFog`에 연결했고, Deferred combine과 source forward material의 원본 v5가 같은 premultiplied fog RGB/투과도를 소비한다. 원본 global Fog PS의 exp2 ray 적분과 hemisphere pow 구조를 대조했다. 렌더링 profile Save의 9자리 float32 경계 값이 다시 parse될 때 거부되던 기존 문제도 입력을 실제 float32로 반올림한 후 같은 범위를 검증하도록 수정했다.

## G02. 실행한 검증

- source geometry native corner join, UV1, tangent W, COLOR byte, material section 보존과 실제 Resources 설치 byte를 확인했다. 세부 근거는 `out/KoukuLightingRestore20260911/geometry_handoff.json`, `installed_resources.json`이다.
- Area publisher Validate/Publish/Check는 현재 3,368 placement/8개 출력으로 통과했다. 최종 source mirror20 행과 forward Alpha/Sky renderMode 수정도 publish에 포함했다.
- RenderingProfiles publisher Publish를 통과했다.
- 실제 C++ parser/serializer probe가 11개 convex region/source fog Save→Parse, 119개 광원/84개 receiver serialize→parse를 통과했다. 잘못된 fog 방향과 모르는 receiver를 넣었을 때 기존 parsed 문서 보존도 통과했다. 근거는 `out/KoukuLightingRestore20260911/probe_run.log`다.
- Engine/Client 변경 10개 C++ 파일의 최소 /Zs 검사를 통과했다. 기존 C4819 인코딩 경고는 남아 있다.
- Deferred FXC 검사를 통과했다. root가 실제 공통 fog helper와 원본 Fog PS를 WARP에서 720 fixture/737,280 pixel로 대조했고 non-finite 0, tolerance 1e-3 mismatch 0, 최대 relative error 4.17233e-7을 확인했다. 비교는 같은 명시적 CPU 상수 입력에 대한 PS 동치 검증이다. `out/KoukuFullRestore20260911/fog_gpu.log`를 근거로 한다. Client/UI 실행이나 캡처는 하지 않았다.
- `git diff --check`를 통과했다(기존 LF/CRLF 안내만 발생).

## G03. capacity와 시각 확인 경계

같은 날 이어진 Bern 전체 복원에서 공유 capacity가 추가 확장됐다. 현재 aggregate catalog/material row 상한은 32,768, 단일 shard는 2,048, map light authoring은 512, transient는 384/map budget 376이다. 아래 128/120 수치는 쿠크 자체 조사에서 결정한 당시 요구량이며 현재 공유 상한과 구분한다.

저작 camera shot·전체 keyframe·11개 volume 중심 8방향을 포함한 292개 sample에 무한 far plane의 보수적인 frustum 계산을 적용했다. 최대 101개, 예전 map budget 56개 초과 10개가 실제로 발견됐다. 그래서 transient 상한 128/map budget120으로 확장하고 effect 여유8개를 유지했다. 일반 pass는 SOURCE_CHARACTER 광원을 CPU에서 제외한다. 팝업북 기본 shot은19개, 1관문 피날레4개, 카드 미로 follow/telescope는2개다. 상세는 `camera_light_region_numeric.json`이다. 이 수치는 Client 실행 측정이나 GPU 프레임 시간 측정이 아니다.

source RNM·UV1·광원·fog field 연결과 원본 전체 엔진의 동일성을 구분한다. 원본 environment cube는 현재 조사한 package에서 발견하지 못했다. WorldInfo environment color/intensity와 WLE lit brightness는 기존 ambient adapter가 소비하지만 원본 SH/contrast, shadowed SH, reflection capture 자체를 재현했다고 주장하지 않는다. Fog CPU packing의 .001 단위와 terminator exponent, 겹침 volume 우선순위와 blend easing 역시 프로젝트 adapter다. Postprocess/LUT/tonemapper와 native fog GPU 대조의 후속 연결은 해당 담당 결과를 따른다. 최종 화면 일치 여부는 사용자의 실행·서면 관찰 전까지 미확인이다.
