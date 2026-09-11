# 베른성 원본 재질·조명 복원 구현 계획

최초 계획: 2026-08-25. 현재 구현 범위 재확정: 2026-09-11. 사용자가 제공한 원본과 현재 장면 비교, 전체 재질·환경·원본 크기 복원 요청을 반영한다. 기존 단계 계획을 재사용하며 실제 구현과 검증은 날짜별 RESULT에서 분리한다. Client 실행·조작·캡처와 최종 시각 판단은 사용자 소유다.

## G00. 현재 실측과 이전 판단의 교정

최초 조사 정적 원본은 16개 package의 32,324개 component, 950개 StaticMesh다. 추가 instanced foliage 1,697개 component의 17,651개 배치를 합치면 원본 배치는 49,975개, 고유 StaticMesh는 961개다. 기존 landscape 42개와 합친 authoring 배치는 50,017개다. 착수 당시 950개 WModel의 사용 material slot은 1,397개이며 Bern authoring/runtime에 mapmaterials 문서가 없었다. 기존 mapwater 1행과 render profile은 일반 표면 shader 전체를 복원한 계약이 아니다.

원본 StaticMesh section 순서·index 개수와 실제 WModel submesh/material index를 대조했다. 정적 component의 nullable material override와 native default는 1,197개 material signature, 원본 MIC 829개였다. instanced-only 자산과 override까지 더한 최종 대상은 1,216개 signature, MIC 845개, 실제 material slot 1,797개다. 같은 원본 MIC가 두 material entry에 반복된 2모델은 named row를 하나로 저장하고 실제 두 slot 모두 적용한다. 단순 이름 비교에서 명시적인 원본 override와 다른 current WMAT 슬롯은 14,607개 slot-occurrence, 221개 asset에 해당한다.

08-25 계획의 “정원나무의 이름이 needletree이므로 Bern 전용 override가 이미 구워졌다”는 판단은 성립하지 않는다. 원본 Bern은 needletree 전용 _01/_02 MIC를 명시하지만 현재 WMAT은 일반 MIC를 사용한다. 재질 이름이 유사해도 원본 component의 정확한 package/object identity가 필요하다.

수로 `MAP_5387B1504BDD_LV_BER_BERNCASTLE_WATER01_SM`의 실제 원본 override는 `lv_ber_berncastle.mat.lv_ber_berncastle_water_01_mi`다. 현재 WMAT/mapwater는 Koilsv 기본 MIC 이름을 사용한다. 다만 원본 Bern도 녹색 diffuse_color를 가지고 있으므로 diffuse 색 하나로 오염 여부를 판단하지 않는다. opacity·depth bias·specular·reflection의 실제 원본 계산과 장면 입력을 함께 복원한다.

원본 native geometry와 현재 WModel의 크기는 별도 전수 비교에서 일치했다. 전체 배율을 임의로 키우지 않는다. 카메라 거리·투영, 재질 반사와 환경 차이는 크기 판정과 구분한다.

## G01. 원본 재질과 선택 shader 입력

`out/BernMaterialAudit20260911`에서 native section, component override, parent chain, shader cache의 equality key, 선택 Base/Light/Baked/VS 프로그램과 uniform/texture binding을 결합한다. 기본 reference 0은 component override가 있으면 이를 사용하며, 같은 이름의 localized export는 StaticMesh class까지 확인한다. leaf 이름 일치만으로 자동 admission하지 않는다.

기존 쿠크 BG8와 overlay7은 실제 source branch와 식이 일치하는 재질에 재사용한다. 기본 BG 563종과 subspecular·rim·saturation·panning 추가 19종, overlay 53종, foliage 117종과 grass 10종을 구분한다. 물 17종과 나머지 forward 48종, monster 5종, snow/ice 4종·vertex blend 3종·wet 1종의 선택 Base/Baked/Light와 texture를 각각 연결한다. 원본 hidden navmesh 336배치의 두 MIC는 원작 visual 대상에서 제외하고 hidden 상태를 보존한다. 검증된 새 family만 Engine의 기존 표면 계약에 추가한다.

## G02. 정본 variant와 Resources 연결

정의는 기존 `.mapassets` shard와 formatVersion 2 `.mapmaterials.json`, 인스턴스는 기존 stable sourcePlacementId/placement ID를 사용한다. native material signature로 결정적인 variant asset ID를 만들고 실제 named WModel material 슬롯에 원본 수치·SRV를 연결한다. current slot/geometry가 여러 native section을 합쳤으면 effective MIC가 동일한지 검사하고, 다르면 원본 section 기준 cook으로 구분한다.

원본 위치·회전·scale·visible 및 사용자 배치를 보존한다. 현재 authoring의 stable source ID가 native와 일치하는 항목에서만 asset reference를 교체한다. 원본에 없는 placement와 사용자 변경은 별도 집계한다. 새 placement format이나 두 번째 모델 런타임을 만들지 않는다.

Resources는 `Client/Bin/Resources/Map/LV_BER_BERNCASTLE/SourceMaterials`와 source geometry 하위에 실제 참조 파일만 설치한다. 원본 top mip·sRGB·address·texture identity를 보존하고 mip 생성은 기존 texture 도구를 사용한다. 원본 추출 pack과 Resources binary는 Git에 추가하지 않는다.

## G03. 실제 draw 경로

`CMapAssetCatalog → MODEL_ASSET_LOAD_DESC → CModel → CMaterial → MapAssetRenderUtils`에 새 입력을 연결한다. catalog mode와 실제 원본 material BlendMode를 일치시켜 deferred/alpha/sky/additive/water의 올바른 pass로 제출한다. 한 모델에 서로 다른 blend가 존재하면 submesh의 기존 CModel material 정보를 이용해 실제 제출을 구분한다. source row만 기록하고 draw가 기존 legacy를 사용하는 상태를 완료로 처리하지 않는다.

MapAssetCatalog와 publisher의 strict version/field/path/numeric 검증을 유지한다. 새 프로그램/texture가 실패하면 기존 parse → validate → stage → commit 실패 계약을 유지하며 정상 재질을 대체 기본값으로 조용히 바꾸지 않는다.

`CGameObject::Render_Group`의 기본 동작은 기존 Render에 위임하고, MapAssetObject만 material별 group을 골라 기존 CModel mesh를 제출한다. per-material render/cull/Shadow를 유지한다. forward source program은 SourceCharacter의 deferred frame 256-row 등록을 거치지 않고 기존 forward pass에서 native Base/Baked와 21개 Light PS의 조명 배열을 소비한다. 물은 기존 SceneColor snapshot과 SceneDepth를 사용해 원본 굴절을 계산하며 기존 mapwater 연산을 중복 적용하지 않는다.

동일 physical WModel와 pre-transform의 variant는 기존 CModel prototype의 mesh buffer를 공유하고 CMaterial만 별도로 구성한다. atlas·source texture SRV는 device/path/color space/file metadata가 같은 경우 weak cache로 공유한다. variant 수가 늘어도 같은 geometry와 texture의 GPU 복제가 늘지 않도록 실제 로딩 경로에서 처리한다.

## G04. geometry·RNM·환경 연결

재질 signature variant의 sourceId mapping을 조명 복원 작업에 넘긴다. native COLOR0·UV1·tangent handedness와 component color를 실제 geometry에 연결한 뒤 component RNM atlas/scale/bias variant를 구성한다. 동일 mesh의 서로 다른 atlas/색은 각각의 source placement 입력을 유지한다. native에 없는 COLOR/UV를 임의 복제하지 않는다.

원본 환경·directional/point/spot·안개는 기존 Engine 기능과 scene authoring을 사용한다. baked 광원과 캐릭터/동적 물체의 수광은 기존 receiver 경계로 분리해 맵에 광원을 중복 합산하지 않는다. camera/original size 확인은 같은 장면의 source 위치·투영 조건과 함께 보고한다.

instanced foliage RNM은 원본 VS의 `UV1 * componentScale + instanceBias`를 사용하며, 사용되지 않는 component UV bias를 대신 적용하지 않는다. source candidate는 RNM 49,397배치, SDF 40,552배치다. 원본 SDF texture와 light GUID/channel을 기존 G-buffer와 광원에 연결한다. normalized SDF의 폭 0.05는 명시적인 PROJECT_ADAPTER이며 원본 shadow texture 자체와 구분한다.

물 program40~43의 선택 Baked PS 4개를 기존 native forward RNM 경로에 추가한다. 38/39는 Baked 비소비를 유지한다. Catalog/publisher와 CModel/CMaterial은 정확한 프로그램 범위만 허용하고, 실제 material과 placement에 baked 입력이 있을 때만 바인딩한다. 동일 조건에서만 이미 RNM에 포함된 SOURCE_CHARACTER receiver 광원을 제외한다. 현행 원본 배치에서는 19 variant material 행, 25 water slot-occurrence가 새로 RNM을 소비하며, 다른 BG 슬롯이 이미 RNM을 소비하는 mixed asset 때문에 전체 RNM placement 증가는 6개다. 검증과 커버리지는 asset 단위와 slot 단위를 함께 기록한다.

sky_mirror의 native UV2는 다른 UV로 대체하지 않는다. WMSH 1.4의 정적 optional UV2를 metadata evidence bit 17로 검증하며 이 sky만 재변환한다. 실제 2,208정점의 UV2를 유지하고, source material의 `requiredExtraUVMask`가 누락 geometry를 거부한다. skinned 1.3과 기존 정적 1.0~1.2 입력은 계속 읽는다.

## G05. 변경 파일과 등록

재질 parser/binding은 `Client/Private/MapAssetCatalog.cpp`, `MapAssetRenderUtils.cpp`, `Engine/Public/BinaryAsset/ModelAssetData.h`, `Engine/Private/Model.cpp`, `Material.cpp`와 기존 map surface/mesh/deferred shader를 확장한다. 새 family helper를 분리하면 실제 include 소비와 `Client.vcxproj`/`.filters` 등록을 같은 변경에 포함한다. 새 C++ 런타임 클래스를 목적 없이 추가하지 않는다.

Bern의 `Data/Maps/MapCatalog.json`에 sourceMaterials/materials pair를 선언하고 authoring/runtime 문서를 기존 publisher로 연결한다. 필요한 shard와 material 문서는 Client 프로젝트의 기존 DataFiles/None 항목에 등록한다. publisher 결과를 직접 편집하지 않는다.

## G06. 검증과 완료 경계

변경한 최소 C++ 컴파일, 실제 전체 material/placement 파서, FX compile, JSON/XML/PowerShell parse, 해당 Area Publish/Check, `git diff --check`를 수행한다. source DXBC와 실제 helper의 수치 비교는 검증한 texture/CB/view/시간 범위를 명시한다. 전체 material slot coverage, 원본 component override 일치, RNM 입력과 Resources 실물, 실제 render mode를 전수 집계한다.

최종 Product CSO와 Engine DLL로 기존 OUT CModel admission 검사를 실행한다. 실제 CShader가 Binary/MapInstance를 로드하고, 대표 material의 `Bind_SourceCharacter`, `Bind_SurfaceLighting`, `Bind_SourceSpecialSurface`와 해당 Effects pass Apply가 성공해야 한다. GameInstance 초기화, UI, 가상 장면 RT 또는 Draw 없이 실제 변수와 DDS/CB 바인딩 경계를 확인한다. 이 검사를 화면 결과나 전체 render pipeline의 대체 증거로 사용하지 않는다.

Product 빌드와 실행 준비 후 사용자가 직접 Bern을 열어 같은 광장·수로·나무·금속·카메라 구도로 판단한다. 에이전트는 구조와 수치 검증을 화면의 원작 일치 승인으로 바꾸지 않는다. RESULT에는 구현, 자동 검증, 사용자 화면 확인, 남은 원본 입력을 구분한다.
