# 쿠크 포커판 텍스처 AddressU 복구 계획

## G01. 확인한 원인과 범위

첨부한 바닥의 거대한 단색 녹색 구역과 우측 테두리 매핑 어긋남은 SL03 export747의
MAP_6CAA005B80DF_BG_RAD_KOUKUSATON_FLOOR15_SM_HHT_OVR_10E1C8701F8A에서 조사한다.
원본 package 542N9YJ2RWOUWUGYNOH2R4FT.upk의 Texture2D floor15_d_hht,
floor15a_d_hht_loc_int, floor15b_d_hht_loc_int는 모두 AddressX=TA_Mirror다.
현재 diffuse sampler의 AddressU=WRAP이 이 설정을 잃었다. 두 번째 반쪽 mesh의 U는
0.9995..1.9941, 테두리 mesh는 0.1039..1.8955로 실제 두 번째 texture tile을 사용한다.
동일 UV에서 50,000개 면적 가중 CPU 표본을 읽으면 두 번째 반쪽의 여백 녹색 비중이
WRAP 28.748%에서 MIRROR 0.15%로 바뀐다. 이 수치는 실제 화면 PASS가 아니다.

## G02. 구현할 기존 경로

Area authoring mapmaterials의 materials에 diffuse-sampler family 세 행을 추가한다.
assetId/materialName/sourceMaterial/sourceTexture/addressU를 정확히 검사하며 addressU는
WRAP 또는 MIRROR다. 기존 MapAssetCatalog → MODEL_MATERIAL_OVERRIDE → CModel →
CMaterial 경로가 주소 방식만 전달한다. texture 경로·픽셀·색·UV·mesh와 표면 파라미터는 보존한다.
CMaterial의 diffuse bind는 매 draw 해당 모드를 바인딩하여 다른 재질에 값이 남지 않게 한다.
기존 공용 HLSL include의 MirrorU sampler를 static/mapinstance diffuse와 shadow alpha가 소비한다.

MapAssetCatalog.cpp, ModelAssetData.h, Model.cpp, Material.h/cpp, Shader_MapMaterialSurface.hlsli,
Shader_VtxMeshBinary.hlsl, Shader_VtxMeshMapInstance.hlsl과 기존 Map publisher만 확장한다.
MapAssetRenderUtils.cpp는 diffuse override draw에서도 Mirror 상태를 0으로 초기화한다.
다른 Character Material 작업의 mip 생성·source emissive 변경을 보존하고 같은 파일의 수정은
담당자 완료 인계 뒤 국소 적용한다. 새 C++ 파일·project/filter 등록이나 새 모델 런타임은 없다.

## 검증과 남길 경계

source JSON parse, 세 source texture의 실제 원본 AddressX와 runtime DDS 동일성, named material
join, 지원하지 않는 모드/중복 행 거절, scoped diff 검사를 수행한다. 필요한 C++·shader 컴파일과
Map publish는 통합 담당자가 실행한다. Client/UI 실행·화면 캡처·GPU 실행은 하지 않고 최종
포커판 무늬와 테두리 복원 확인은 사용자가 한다. Resources binary는 변경하거나 재쿠킹하지 않는다.
