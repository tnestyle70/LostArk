# 베른·캐릭터 선택 지형 원본 복원 구현 계획

## G00. 목표와 적용 경계

베른의 `(163,49,-111)`, `(192,49.19,-136)` 주변을 포함해 누락 지형을 원본 패키지와 대조하고, 캐릭터 선택 맵에서 이름에 따라 숨겨졌던 지면을 복원한다. 원본 배치·구멍·초기 시퀀스 상태를 유지하며 기존 `CModel -> CMaterial`과 Area publisher를 사용한다. 원본의 빈 공간에 임의 바닥을 만들지 않는다.

사용자는 실행 중인 Client/Server를 유지하고 직접 빌드·화면 확인한다. 에이전트는 Client/UI를 실행하거나 조작하지 않고 제품 전체 빌드를 수행하지 않는다. G07의 격리 C++·shader 컴파일과 WARP 수치 검사는 제품 실행 파일·CSO를 교체하지 않는다. 데이터 후보와 리소스를 검증한 뒤 최신 저장본의 stable ID·변경 필드만 원자적으로 반영한다. 실제 완료와 화면 검증은 [RESULT](2026-09-30_BERN_AND_CHARACTER_SELECT_TERRAIN_RESTORATION_RESULT.md)에서 분리한다.

최종 반영 상태: 원본 초기 Kismet 감사까지 통과한 Bern66·Character Select7 배치(1차 Bern64와 2차 도어2), spotlight·도어 재질을 설치했다. 이어 `(250,15,-168)`에서 확인한 Landscape 흐림을 원본 UV·반복 텍스처·레이어 혼합으로 수정하고42개 지형에 연결했다. 공식 Area publisher의 최종 Bern 50,021배치/24shard/53파일과 Character Select 804배치/5파일 생성·검증을 통과하고 hash CAS·백업·원자적 교체로 필요한 게시 파일을 반영했다. GBResources의 이번 Bern·Character Select 복원 의존성은 중복 제거371개·85,445,734B이며 설치본과 hash가 같다. 사용자 요청에 따라 이 범위에서 변경을 마쳤고 제품 전체 빌드와 새 실행 화면 확인은 사용자 또는 다른 세션에 인계한다.

## G01. 원본 가시성과 형상 검증

`extract_ue3_placements.py` schema 3으로 Bern 기존 추출 범위 16패키지의 StaticMesh 배치 32,324개, Character Select SL00 803개를 재추출한다. actor/component 인스턴스뿐 아니라 archetype/CDO를 조회하고 현재 authoring의 source placement ID와 대조한다. 1차 정본에는 각각 50,019개와 804개의 배치가 있으며 이 기존 16패키지 범위에서는 source ID 누락이 없다. 전체 PS 스트리밍 범위의 추가 감사는 아래 G05에서 구분한다.

가시성은 actor/CDO만으로 확정하지 않는다. 원본 LevelStreaming의 실제 연결, 초기 `LevelLoaded`/`LevelStartup` Kismet과 `ToggleHidden`/Toggle 상태를 함께 대조한다. Bern SCENE03E는 AlwaysLoaded여도 초기 Kismet이 45개 InterpActor를 숨기므로 기존 숨김을 유지한다. EVENT01은 PS의 활성 스트리밍 참조가 확인되지 않아 1,127개를 현재 맵에 일괄 표시하지 않는다. `LV_MODULE`, nav, water, FX, SCENE라는 이름은 숨김 근거가 아니다.

Bern Landscape 42개는 원본 visible이며, 구형 WModel의 tile-local Z가 원본과 반대다. 현행 `LandscapeExtractor`의 좌표계 변환으로 42개를 재생성하고 원본 321,970개 삼각형, 463개 hole quad, 정점 높이와 winding을 비교한다. 원본 tile anchor와 현재 배치의 stable ID, 위치·회전·스케일을 그대로 사용한다.

## G02. 배치·재질·리소스 변경

| 정본 | 변경 책임 |
|---|---|
| `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements` | 지형 42개와 지원 재질을 가진 기존 가시성 override 22개의 visible 필드 복원. 22개는 원본37패키지 초기 시퀀스 감사를 통과 |
| `Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.renderprofiles.json` | 위 22개 stable source ID의 importer 가시성 override 정합성 유지 |
| `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements` | source export 315, 322, 326, 327, 328, 376 지면과 563 spotlight의 visible 복원 |
| 같은 Area의 `.mapmaterials.json` | 563의 원본 additive·two-sided spotlight 1행 연결 |
| `Data/Maps/Imported/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapassets` | spotlight의 동일 asset에 Additive/None 적용 |
| 기존 Bern Landscape 리소스 경로 208개 | 42 WModel, 84 baked PNG, 82 cliff DDS의 현행 추출 결과 설치·GBResources 전달 |

Character Select 지면 6개는 기존 원본 PBR/overlay 재질·RNM을 유지한다. spotlight 563은 실제 WModel 슬롯 `bfx_spotlight_nonparticle_01_01_ad`에 원본 MI `lv_kur_death_06.mat_v.lv_spotlight_nonparticle_01_06_ad_mi_01`을 연결한다. 이미 사용 중인 `source.map.spotlight.v1` program 33과 동일한 원본 부모 재질이며, leaf의 color/dust density/dust speed/opacity만 원본 값으로 공급한다. 새 셰이더를 만들지 않는다.

409 상공 평면의 기본 재질은 `enginematerials.defaultmaterial`이며 원본 UModel에서도 해당 import를 찾지 못한다. 이 항목은 지면보다 약 40m 위에 있고 원본 재질 근거가 부족하므로 임의 회색 재질로 표시하지 않는다. 기존 숨김을 유지하고 미지원 근거를 RESULT에 기록한다.

1차 Landscape의 flat/cliff는 기존 결정적 layer bake와 WModel embedded diffuse/normal을 사용했고 cliff 원본 DDS의10개 mip을 보존했다. 이후 G07에서 원본 painted layer로 교체했다. 어느 단계도 UE3 지형의 전체 조명·반사 동일성을 주장하지 않는다.

## G03. 보존 및 반영 순서

최종 교체 직전에 authoring source hash를 다시 확인하고 백업한다. 배치 수·stable ID·source ID·asset ID·TRS를 대조하고 허용된 visible 필드와 spotlight 재질 필드만 병합한다. 무관한 조명, 바다, navigation, 렌더링 옵션과 다른 세션 수정은 보존한다. publisher가 생성하는 런타임 데이터를 직접 편집하지 않는다.

원본 topology와 리소스 검증 → Area candidate Validate/Publish → 최신 저장본 CAS → 설치 리소스와 authoring 원자적 반영 → 정상 Area publish → 게시 파일·리소스 hash 재검증 → GBResources에 누락·변경 의존성 전달 순서다. 파일 설치와 실행 중 메모리 Reload, 사용자 화면 판정은 별개다.

`build_bern_castle_shards.py`의 SCENE03E 기본 숨김은 원본 초기 Kismet 근거가 있으므로 유지한다. 후보 조사 중 검토했던 해당 차단 제거는 최종 변경에 포함하지 않는다. 가시성·도어 복원인1·2차에는 신규 C++·HLSL·프로젝트 항목이 없다. 이후 원본 지형 재질을 지원하는3차 코드 변경은 G07에 구분한다.

## G04. 종료 증거

- 원본 source visibility와 초기 Kismet 감사, 기존 16패키지 source ID 누락 0과 추가 스트리밍 범위의 판정을 구분.
- 42개 Landscape 정점·삼각형·hole·winding·tile seam 대조, 원본 placement TRS 일치.
- 현재 WModel의 실제 material slot과 mapmaterials, texture/RNM/static-shadow 경로 전수 존재·hash 대조.
- 가시성 후보의 역변환이 원본 파일과 byte-exact이며 비대상 배치와 TRS가 보존됨.
- JSON parse, Area publisher 검증·게시 및 `git diff --check`.
- GBResources 의존성 전달 후 원본과 SHA256 일치.
- 제품 전체 빌드·Client/UI 실행·화면 PASS는 기록하지 않음. G07의 격리 검사와 사용자의 직접 빌드·화면 확인을 구분함.

## G05. PS 스트리밍 전체 범위의 추가 도어 복원

원본 PS의 스트리밍 34패키지 중 기존 16패키지 조사와 겹치는 것은 14개다. 남은 20개를 전수 조회하면 13개에는 StaticMeshComponent가 없고 7개에는 49개가 있다. actor/component/CDO, 초기 Kismet, 외부 참조, Base 부착과 Matinee 소비를 함께 확인한 결과 초기 숨김24개, 독립 도어2개, owner 부착 또는 연출용23개로 나뉜다. 23개를 원본 hidden이라고 기록하거나 임의의 독립 고정 지형으로 배치하지 않는다.

`LV_BER_BERNCASTLE_T_SCENE07B:export:355/356`은 기본 visible, AlwaysLoaded이며 Base·Matinee·Toggle 제어가 없다. 현재 정본에는 해당 source ID가 없고 같은 원본 door01 모델의 기존 두 배치는 약175m 떨어져 있어 중복이 아니다. 원본 signed scale·quaternion을 그대로 사용해 두 배치만 추가한다. 각 component의 원본 native lighting은 `lodCount=0`이므로 다른 배치의 RNM이나 static shadow를 복사하지 않는다.

동일 WModel은 재사용하되 default MI `bg_ber_berncastle_c.mat.bg_ber_berncastle_door01_mi_ksy`에 맞는 variant를 구분한다. 기존 base binding은 다른 SL04 배치의 override `lv_ber_berncastle.mat.bg_ber_berncastle_door01_mi_ksy_01`이며 specular3/power60인 원본 default와 값이 다르다. 기존 material을 덮어쓰지 않고 원본 graph·native switch·texture mip/색공간을 검증한 새1행만 연결한다.

두 source ID는 Authoring과 Imported의 `MaterialExtra00.mapplacements`에 함께 보관한다. 해당 catalog의 새 variant1개와 mapset의 count만 늘리고 24shard 구조를 유지한다. 현재 publisher는 Imported baseline의 stable ID별 shard 소속을 보존하므로 반복 publish 때 빠지지 않는다. 구형 `build_bern_castle_shards.py`의 13shard/32,324배치 전체 재생성은 현재 24shard/RNM 저작본을 갱신하는 절차가 아니다. 전체 재추출 시 기존 Imported·Authoring과 이 source ID를 병합해야 하며 SCENE07B 전체를 include해 부착된 transporter2개까지 독립 생성하지 않는다. importer 및 runtime 코드는 변경하지 않는다.

이 2차 작업은 `out/BernTerrainRestore_20260930/phase2-door`에서 1차 설치 이후의 최신 source hash를 기준으로 후보를 만들었다. 별도 격리 publisher와 hash CAS 반영을 완료했으며 설치9파일·GBResources7개의 근거는 `doors-phase2/installed-data.json`, `resource-delivery.json`, `final-verification.json`에 있다.

## G06. 사용자 화면에서 확인한 Landscape 흐림 보완

`(250,15,-168)`의 새 화면 검토에서 지형 표면 흐림이 확인됐다. 1차 지형 복원은 원본 geometry/hole/좌표계 및 기존 결정적 bake 경로를 회복했으나 39.68m 타일의256px 색상 bake는 원본 반복 텍스처의 세부 묘사를 보존하지 못한다. 원본 layer texture·UV·tiling과 고해상도 bake 후보를 대조하고 현재 `CModel -> CMaterial` 내 레이어 소비 지원 범위도 확인한다. 화면 증상과 원본 UV 관계의 최종 판정 전 임의의 전역 sampler·mip bias나 렌더링 옵션으로 덮지 않는다. 이 보완의 파일 변경·검증·화면 결과는 확인한 단계만 RESULT에 기록한다.


## G07. 베른 지형 원본 레이어 재질 연결

G06의 원본 ShaderCache 대조에서 전체 42컴포넌트의 native static key와 원본 PS가 일치했다. LC762의 원본 layer06 무늬는 39.68m 타일에서22.32회 반복되지만 기존 파생 bake는3.6회였다. 원본은 layer01~06 중 최대6개를 paint weight와 diffuse alpha로 혼합하며, diffuse/specular와 normal의 height blend 선택이 다르다. 원본 PS에는 layercliff 샘플이 없다. 256px bake 해상도만 높이면 원본 반복 텍스처의 세부 정보가 계속 손실되므로 기존 map surface 경로에 bg-source-landscape-opaque 재질을 연결한다.

범위는 먼저 사용자가 지적한 LAND02 LC762, 이후 동일 원본 계약을 검증한 베른42컴포넌트다. 다른 맵 재질·조명·렌더링 옵션으로 확장하지 않는다. 원본 geometry, source hole, 배치 ID/TRS를 보존한다. legacy cliff 분리 대신 모든 면이 component UV를 가진 기존 source_painted_surface glTF/WModel 경로를 사용한다.

기존 CModel -> CMaterial의 map surface 경로에 SOURCE_LANDSCAPE_OPAQUE family14를 추가한다. 최대6개 layer의 원본 반복 D/N, 최대2개 weightmap, Heightmap BA의 pixel normal을 bounded 입력으로 받는다. 새 renderer, 별도 모델 타입, terrain-specific GameObject는 만들지 않는다. 기존 MAP_SURFACE_SAMPLE과 deferred source-specular 소비 경로를 재사용하며 별도 native character program/group은 추가하지 않는다. layer/specular/normal/색공간과 원본 mip은 근거가 확인된 값만 전달한다.

| 파일 | 변경 책임 |
|---|---|
| ModelAssetData, CMaterial, CModel, MapAssetCatalog, MapAssetRenderUtils | family14의 bounded 입력·텍스처 로드/바인딩, strict parser 및 기존 map surface 경로 연결 |
| Shader_SourceLandscapeSurface.hlsli, MapMaterialSurface, 두 mesh pass, Engine Deferred 및 필요한 프로젝트·filters | 동일 CModel draw의 지형 surface/basis 평가와 marker14의 기존 deferred light 연결, 신규 include만 등록 |
| Tools/MapPipeline/Publish-MapAuthoring.ps1 | Client와 같은 strict schema 및 texture/parameter 범위·필수 입력 검증 |
| Tools/LandscapeExtractor와 원본 입력 산출물 | 검증된 layer 계약의 재현 가능한 native 후보, 단일 painted surface geometry 생성 |
| 기존 Bern mapmaterials 및 Landscape 리소스 | 원본 D/N/weight/height 입력 연결과 기존42개 asset의 material binding, stable placement 보존 |

먼저 LC762 입력·원본 수식·shader 컴파일을 격리 산출물에서 대조하고, 같은 계약의42개 topology·holes·원본 mip과 publisher 검증을 마친 후보만 CAS/백업/원자 교체한다. 테스트용 컴파일 산출물은 제품 EXE/DLL이나 실행 중 shader를 교체하지 않는다. 사용자 실행 화면과 현재 바이너리에 새 코드가 들어갔는지는 별도 검증 상태로 유지한다. 공개 스키마 소비 계약과 실제 완료 증거는 TEAM Area guide 및 RESULT에 반영한다.

## G08. 10-04 원본 Landscape 조명·그림자 입력 복원

사용자가 지정한 `(98.73,49.13,-103.39)`의 LAND01 LC622를 다시 대조했다. 앞선 NoLightmapPolicy 수식 대조는 표면 레이어에 한정됐으며 실제 component가 보유한 RNM과 정적 그림자를 검증하지 못했다. 재설치 원본의 현재42개 component 모두 FLightMap2D와 정적 그림자1개를 가진다. 현재 family14 parser와 CMaterial은 이 입력을 거부한다. 원본 DistanceFieldShadowedDynamicLightDirectionalLightmapTexturePolicy와 현재 설치 EFEngine의 LandscapeLightmapScaleBias CPU 계산을 연결한 증거를 바탕으로 이 누락을 보완한다.

기존 bakedLighting/placementLighting 계약을 family14에 허용한다. 각 component 자신의 RNM2장·G8 shadow와 원본 coefficient 및 atlas 좌표를 사용한다. 원본 CPU grid/padding 좌표를 현재 정규화된 mesh UV0 기준으로 환산한 값과 atlas scale/bias를 합성하여 기존 placementLighting에 저장한다. LC622는 scale0.953125, bias0.015372984111309052이며4,096정점의 원본 subsection 식과 float32 최대오차5.96e-8이다. 다른 component는 자신의 해상도·원본 상수를 검증한 뒤 같은 식을 적용한다. UV0 표면 반복과 배치 TRS는 바꾸지 않는다.

ordinary/instanced VS는 family14의 조명·그림자 좌표에 UV0를 명시적으로 사용한다. RNM view-dependent specular는 Heightmap BA에서 재구성한 pixel TBN을 사용하며 VS가 운반하는 world axis를 tangent basis로 오인하지 않는다. 기존 RNM·정적 그림자 draw와 deferred 경로를 확장하고 새 renderer나 모델 형식을 만들지 않는다. 그림자의 penumbraWidth/중심은 기존 PROJECT_ADAPTER임을 유지하며 원본 CPU 복원값이라고 주장하지 않는다.

| 파일·영역 | 이번 변경 책임 |
|---|---|
| MapAssetCatalog, CModel, CMaterial, MapAssetRenderUtils | family14의 strict baked input 검증, 경로 보존·로드·기존 draw 바인딩 |
| MapMaterialSurface, SourceLandscapeSurface, 두 mesh shader | pixel TBN 전달, 기존 RNM 평가 및 UV0 기반 lighting 좌표 |
| Publish-MapAuthoring 및 landscape 계약 검사 | 동일 schema 허용, texture/placement 의존성과 실패 시 이전 게시본 보존 |
| LandscapeExtractor 후보 도구·Bern mapmaterials·Resources | 원본42개별 texture mip/계수/좌표를 재현하고 최신 저장본에 필요한 행만 병합 |

원본 추출·CPU/셰이더 수치 비교 → focused shader/계약 검증 → 최소 제품 증분 컴파일 → 최신 정본 hash 재확인과 백업·원자 설치 → 공식 publisher → 파일·DDS 로드 재검증 순서로 진행한다. 기존 G00/G07의 제품 빌드 제외는09-30 작업 기록이며 이번 추가 복원의 코드 소비 확인에는 필요한 증분 빌드를 사용한다. Client/UI 실행과 화면 확인은 사용자가 한다. 렌더링 옵션을 변경하지 않는다. 사용자 요청의 캐릭터 선택·Bern 전체 추가 감사는 근거 있는 누락과 미확인 후보를 RESULT에 구분한다.

캐릭터 선택 SL00의 추가 감사에서는 사용 중인 RNM56개가 원본4~11mip 중 mip0만 설치된 것을 확인했다. 기존 mip0는56개 모두 fresh UModel 결과와 byte-exact다. 동일 원본 decoder로 각 하위 mip을 회수해 원본 크기·포맷·top mip·색 공간을 보존한 DDS 후보를 만든다. 후보의 모든 mip과 실제 로더 입력을 검증하고 기존 Resources56개만 hash CAS/백업/원자 교체한다. 재질/배치/렌더링 옵션은 변경하지 않는다. Bern 일반 모델의 같은 현상은 별도로 원본 및 설치 header를 감사하며 미조사 전체를 일괄 변환하지 않는다.
