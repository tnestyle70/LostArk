# Bern foliage·grass native 표면 구현 계획

## G00. 목표와 현재 입력

원본 950개 모델·32,324 component의 section/material slot을 조인한 Bern 조사 결과에서 foliage 107 MIC와 grass 4 MIC를 기존 CModel → CMaterial → MapMaterialSurface 경로에 연결한다. 원본 native shader map과 선택된 DXBC를 기준으로 17개 pixel permutation의 diffuse tint, saturation, alpha clip, normal, specular, transmission, emissive, RNM baked lighting을 보존한다. 단풍 텍스처만 갈아 끼우지 않고 배치별 원본 MIC가 가진 색과 입력을 함께 복구한다.

## G01. 모델·재질 계약

`Engine/Public/BinaryAsset/ModelAssetData.h`의 surface family에 SOURCE_FOLIAGE_MASKED=9, SOURCE_GRASS_MASKED=10을 추가한다. 기존 공통 색·밝기·normal·specular·emissive 값을 재사용하고, static switch bitset sourceFoliageFlags, transmission RGB/intensity, 독립 foliage-mask 경로를 추가한다. family 9/10은 원본 specularPower=0도 유효하게 처리한다. CMaterial이 선택된 텍스처만 생성하고, CModel override 검증은 Resources 경로 경계와 유효 수치를 확인한 후 commit한다. 실패 시 기존 모델은 보존한다.

## G02. 원본 표면과 조명 소비

새 `Shader_SourceFoliageSurface.hlsli`는 stateless native 식만 소유한다. foliage mask R은 specular, B는 transmission, A는 tint/saturation이다. grass는 mask 없이 diffuse tint를 적용하며 원본 transmission color를 diffuse에 곱한 독립 간접항을 추가한다. foliage direct transmission은 `NoL*(1-q)+q*q`이며 grass direct diffuse는 1이다. 두 family의 specular는 원본 abs(NoH), exponent, RGB cap 2를 사용한다. RNM baked diffuse는 foliage transmission과 합성하며 grass는 세 coefficient의 평균이다. 기존 MRT의 CharacterSurface RGB에 transmission을 기록하고 depth marker 9/10으로 deferred 조명 함수에 분기한다.

기존 `MapAssetRenderUtils.cpp`가 선택 texture와 scalar를 bind한다. 새 C++ 파일은 없다. HLSL include는 실제 shader 프로젝트의 기존 include 등록 방식에 따라 필요한 항목만 추가한다. 전체 Product 빌드는 root가 단독 진행한다.

## G03. 데이터 작성과 분업

111 MIC 행은 원본 effective input으로 작성해 `out/BernFoliageNative20260911/material_rows.json`로 전달한다. Map 담당자는 parser·catalog·placement variant·texture payload 설치를 소유한다. 동일 파일의 generic per-slot render/cull 변경은 보존한다. Resources payload는 Git에 포함하지 않는다.

## G04. 검증과 남은 경계

선택 native Base/Light/Baked PS와 helper를 headless WARP의 같은 상수·텍스처·시야·조명 fixture로 비교한다. native shader의 unowned hemisphere/scene CB와 wind CPU setup은 임의 상수로 복원 완료 처리하지 않는다. 원본 wind VS 두 개의 선택·입력은 별도 조사하되, CPU scene wind와 mesh bounds 계약이 닫히기 전에는 흔들림을 추정 구현하지 않는다. HLSL 최소 컴파일, 실제 parser/CMaterial 입력, JSON parse, git diff --check를 확인한다. Client/UI 실행과 화면 캡처는 하지 않는다. 사용자의 최종 화면 확인은 별도 상태로 둔다.

## G05. Bern 정적 monster 재질 5종

원본 monster_base 계열을 사용하는 drop_abilitystone01, drop_bottle01, drop_devilstone01, itr_01289, wp_np_lrrt_00의 5 MIC는 고유 Base/Light 네 쌍이다. 기존 0~32 native PS와 같은 pair는 없으므로 예약한 program 80~83에만 추가한다. `SourceCharacterMaterialParameters.h`의 strict named packing과 기존 NativeInput/CMaterial 경로를 재사용한다. static map 좌표와 B 부호는 root의 map adapter에 합류하며, legacy Light UV2/light3/view5/position6를 원본 signature대로 연결한다. 원본 material texture gamma와 Resources ID를 보존한 행을 Map 담당자에게 전달하고 원본 DXBC 대조를 수행한다.

원본 RNM 연결을 확장한다. 이 5 MIC가 쓰인 정적 배치 중 60개(8 assets)에 RNM이 존재한다. source character80~83만 optional bakedLighting을 허용하고, 같은 baseConstants 배열의 미사용 상단32~58에 native Baked PS의 material rows를 패킹한다. 기존0~32와80~83의 non-baked rows는 유지한다. source NativeInput의 lightmap sample 두 벡터와 별도 enabled 값만 기존 geometry 호출에 추가하고, 이미 사용 중인 per-placement UV scale/bias·average/directional scale을 소비한다. shadow와 선택된 direct PS는 기존 경로를 유지한다.

## G06. 원본 instanced foliage의 추가 입력

원본 instanced component 재조인으로 확인한 foliage 10 MIC와 grass 6 MIC를 기존 111행에 합쳐 127행으로 연결한다. 16개 중 15개는 검증한 pixel permutation을 그대로 사용하며, specular를 끈 grass의 신규 Base/Baked/Light 한 조합만 원본 DXBC와 추가 대조한다. 11개의 신규 원본 DDS는 Map 담당자의 같은 texture 설치 경로를 소비하며 같은 이름의 다른 패키지 texture로 대체하지 않는다. 기존 family 9/10 계약과 parser를 그대로 사용하므로 새 제품 파일이나 shader family는 추가하지 않는다.

RNM을 사용하는 정적 native80~83은 원본 Baked PS에 간접광을 이미 포함하므로 source-character deferred의 기존 project ambient 근사항을 중복 적용하지 않는다. CMaterial이 같은 Baked enabled 값을 light pass에도 전달하고, 해당 네 program의 baked row만 ambient shade를 0으로 둔다. 기존 캐릭터와 non-baked 경로는 그대로 유지한다.

## G07. Bern BG 기본 표면의 미연결 19 MIC

source BG8에서 subspecular 13개, rimlight 6개, specular saturation 1개, UV panning 1개를 사용하는 비-overlay 19 MIC의 원본 PS를 대조한다. 중복 MIC는 한 번만 연결한다. Subspecular는 direct light의 두 번째 lobe가 아니라 Base/Baked의 독립 시선 반사항이다. `pow(saturate(dot(N,V))²,subspecularPower)*resolvedSpecular*subspecularIntensity`를 MRT4 간접항에 넣고, 원본 threshold 1e-6을 유지한다. Rimlight는 `rimColor*intensity*pow(abs((1-abs(dot(N,V)))*(1-abs(V.z))),power)`를 계산해 BG8의 비어 있는 MRT6 RGB에 기록한다. Deferred는 원본처럼 geometric normal의 뒷면 입사량 및 shadow·light color를 한 번 곱한다.

MODEL_SURFACE_PARAMETERS의 기존 BG8 뒤에 sourceBgSubspecular(float2), sourceBgRimlight(float4), sourceBgSpecularSaturation(float), sourceBgPanning(float2)을 추가한다. 기본값은 기존 경로에 영향이 없는 0/60, 0/0/0/3, 1, 0/0이다. 새 flag나 family는 만들지 않는다. UV는 기존 변환 뒤에 native presentation time*panning speed를 더하고, emissive가 없는 map도 동일 elapsed time을 bind한다. specular saturation은 raw selected specular RGB에 먼저 적용한다.

JSON optional fields는 sourceSubspecular[2], sourceRimlight[4], sourceSpecularSaturation, sourcePanning[2]이고 Map 담당자가 parser/publisher를 연결한다. named parameter/texture 원본 입력에서 19행을 작성한다. 같은 제품 helper와 원본 Base/Baked/Light DXBC를 WARP에서 대조하며, 새로운 C++ 파일은 없고 기존 프로젝트 구조를 유지한다. 공용 surface helper의 overlay·wet·snow 확장은 같은 파일의 별도 함수로 진행 중이므로 국소 편집만 한다.

## G08. Bern의 ice·vertexblend·wet 및 특수 표면

남은 13 MIC의 원본 선택 PS와 static switch를 조사했다. snowice 4 MIC는 2개 PS 조합, vertexblend 3 MIC는 2개 조합, wet 1 MIC와 interpactor 1 MIC는 각각 한 조합이다. 원본 ice core/outer 색, 환경 texture, 시차 UV와 emissive, vertex RGBA에 따른 2층/4층 diffuse·normal·specular 혼합, wet의 반사색과 specular power 혼합은 기존 overlay와 다르므로 별도 stateless 표면 helper로 연결한다. Map surface enum 11/12/13을 ice/vertexblend/wet에 배정하고 기존 CModel·CMaterial·MapAssetRenderUtils·MRT deferred와 RNM 경로를 확장한다. 파일별 변경은 다른 담당의 overlay, static shadow와 UV2 변경을 보존한 국소 편집으로 한다.

제품 구현에 앞서 원본 texture/parameter의 정확한 의미를 복원하고 기존 packed MRT에 resolved diffuse·normal·specular·power·emissive를 기록할 수 있는지 수치로 확인한다. interpactor는 native Base/Baked/Light가 기존 BG8과 동치이면 재사용하며 원본 wind CPU 미확정 경계는 기존 foliage와 같이 별도 기록한다. black·navmesh 두 종류·shadow 재질은 원본 blend/visibility와 사용자에게 보이는 배치를 구분한다. 편집용 원본 배치를 임의 가시화하지 않는다. 명확한 source 근거와 실제 소비자가 있는 행만 catalog 담당에게 전달한다.

새 C++ 파일은 만들지 않는다. 새 HLSL helper는 Engine/Client의 기존 include 등록에 추가하고 원본 DXBC 대비 Base/Baked/Light와 여러 vertex color/시야/시간 fixture를 headless WARP로 검증한다. 전체 Product 빌드와 publisher는 root와 map 담당이 수행한다.

## G09. Bern 물 표면 40~43의 원본 Baked PS

기존 source.map.water-40~43의 9 MIC에 선택된 Baked PS 네 개를 연결한다. native CB0 크기는 각각 16/16/23/15 float4로 기존 baseConstants 상단32~59에 들어가며, program43의 scale63은 보존한다. 기존 Base/Direct 함수는 바꾸지 않고 Baked 함수 네 개만 같은 water include에 추가한다. per-placement RNM UV, average/directional texture와 scale은 기존 MapAssetRenderUtils 바인딩을 소비한다. Baked PS가 이미 포함한 간접광에 scene ambient를 중복해서 더하지 않는다.

먼저 out에서 native expression으로 strict C++ packing과 HLSL을 생성하고 원본 DXBC에 같은 texture·RNM·시간·시야를 넣어 비교한다. 검증 후 기존 파일의 해당 함수와 program40~43 선택 분기만 연결한다. 새 런타임 타입이나 C++ 파일은 추가하지 않는다. Catalog/publisher의 baked eligibility와 실제 RNM 배치 연결은 map·lighting 담당자가 기존 계약으로 합류한다. 최종 Product는 root가 실행한다.
