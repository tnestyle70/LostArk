# 베른 원본 geometry·RNM·환경·정적 shadow 연결 결과

## G00. 현재 구현 상태

native geometry와 사용되는 Resources, source 광원과 다섯 환경 volume, SDF shader/instance/typed channel을 실제 소비자에 연결했다. 843 MIC의 고유 기본 1,789행과 원본 RNM/SDF variant를 최종 authoring에 설치했고 Area Validate 및 실제 C++ 전체 catalog/placement parse를 통과했다. 최종 물 재질을 포함한 Area Publish/Check도 통과했다. Product 빌드는 root 결과를 따른다. 사용자의 화면 확인은 아직 수행되지 않았다.

## G01. 원본 geometry와 instance

16개 원본 package의 32,324 StaticMeshComponent와 1,697 InstancedStaticMeshComponent를 조사했다. Instanced component의 실제 80-byte record 17,651개를 배치 source ID·위치와 전수 대조했다. 원본 Instanced VF는 UV1×componentScale+perInstanceBias를 사용한다. 70개 component의 사용되지 않는 bias에는 NaN이 있었으나 실제 instance bias는 전부 유한하며 그 값을 연결했다.

static 950개와 instanced 전용 11개를 합친 961개 모델의 native triangle corner에서 UV1·COLOR0·tangent W를 복원했다. component COLOR override는 287개 고유 WModel과 879개 placement에 대응한다. 17개 모델의 원본 parallel N/T basis는 native byte/corner 증거가 있는 경우에만 명시 flag로 보존하고 binormal 0을 유지한다. 일반 finite/normalization/index 검증은 유지되며 셰이더는 zero basis를 NaN으로 만들지 않는다. sky는 별도 native UV2가 보존된 1.4 WModel로 설치했다. 모델의 물리 규모와 placement ID/transform/visibility는 이 작업에서 변경하지 않았다.

원본 component RNM은 static 31,746개와 foliage 17,651개, 합 49,397개다. material signature/atlas/원본 component color를 구분한 15,483 variant를 설치했다. 최종 catalog는 23 shard, 16,741 unique asset/19,152 선언행이다. 같은 WModel은 기존 CModel clone으로 immutable GPU mesh를 공유한다. texture가 달라진 variant마다 geometry를 새로 cook하지 않는다.

최종 material 문서는 23,153행, placementLighting 49,047개, SDF 소비 placement 40,247개다. 원본 RNM 중 asset 전체 미소비 350개는 이미 source hidden인 navmesh 335개, native no-Baked water39 14개, native unlit53 1개다. mixed asset을 포함한 실제 slot 기준 미소비는 water38 10개, water39 46개, unlit53 1개와 hidden335개다. water40~43의 19개 variant material 행/25 slot occurrence를 추가 연결했으며 전체 placement 수는 6개 증가했다. 원본 hiddengame=true navmesh 전체 336개는 기존 authoring부터 visible=0이어서 그대로 보존했다. 이 두 hidden MIC의 6 slot만 전수 재질 행 조건에서 명시적인 source-hidden 예외로 분리했다.

publisher의 shard 내부 asset 참조 계약을 유지하기 위해 RNM asset 소유 shard로 Imported baseline 배치 49,397개를 옮겼다. stable ID, source ID, world transform, visibility와 authoring 배치 payload는 그대로다. 기존 Bern MakeFull 제품 로딩 범위를 유지하며 기존 parser를 완화하지 않았다.

## G02. 원본 조명과 환경

원본 local light 315개 중 RNM LightmapGuid에 포함된 285개는 SOURCE_CHARACTER receiver이고 나머지 30개는 ALL이다. 원래 입구 10개 source 광원의 ID는 유지하며 315개 안에서 갱신했다. RNM에 포함되지 않은 원본 dominant directional brightness 2.4와 방향/색을 기존 scene main light로 연결했다. skybox-only Skylight는 일반 map/character에 적용하지 않았다.

WorldInfo의 environment color/intensity와 PS exponential fog, 원본 EF volume 다섯 개의 6개 convex plane 및 blend duration을 기존 RenderingProfile로 연결했다. camera 지역 선택과 선형 전환은 쿠크에서 연결한 동일 소비자를 사용한다. native BSP vertex 40개와 저장 bounds의 최대 오차는 0.00373 cm다. 6,952개 camera/region/scene 범위 sample에서 무한 far plane을 쓴 보수 frustum 최대 광원 수는 299개다. 공유 transient 상한은 384, map budget 376이며 effect 여유 8개를 둔다. 이 수치는 GPU 프레임 시간 측정이 아니다.

## G03. 원본 정적 그림자

ShadowMap2D 40,594개와 ShadowMapTexture2D 2,306개를 원본에서 해독했다. 모든 ShadowMap2D는 native tail 0 bytes이며 bIsShadowFactorTexture=false다. 따라서 G8 sample을 직접 밝기로 곱하지 않고 원본 distance-field PS transfer를 사용한다. native LZ4 block/mip 크기를 검증하여 G8 전 mip을 DDS로 만들었으며 41,399,200개 mip0 pixel의 전체 256-bin histogram을 보관했다.

현재 source placement에 대응하는 40,552개는 배치당 정확히 하나이며 main directional GUID 38,225개와 12개 local dominant GUID 2,327개로 나뉜다. 나머지 42개는 LandscapeComponent의 실제 shadow다. material은 원본 texture/GUID, placement는 별도 shadow UV scale/bias를 소비한다. foliage 17,651개의 per-instance shadow bias도 해당 ShadowMap2D와 전수 일치했다.

shader는 원본 형태 `pow(saturate((sample+bias)*scale),exponent)`를 사용한다. Lightmass CDO의 exponent 2는 원본 확정값이다. CPU bias/scale는 원본 protected disk image에서 확정하지 못해 normalized center 0.5/penumbraWidth 0.05의 PROJECT_ADAPTER로 명시한다. 이는 원본 penumbra 복구 완료라는 뜻이 아니다.

staticShadow.lightChannel 1~15와 maplight staticShadowChannel을 typed 입력으로 검증한다. 현재 scene main은 1, native local 12개는 2~13이다. RGBA32_FLOAT PickPos.W exponent에 channel을 저장하고 normal/RNM mantissa 23 bits를 보존한다. MRT4 alpha의 occlusion은 matching light의 direct diffuse/specular에만 적용한다. 원본 RNM/ambient/emissive에 shadow를 재곱하지 않는다. native monster 80~83도 같은 helper와 MRT4 alpha를 소비한다.

## G04. 실행한 검증

- source geometry native corner/channel와 DDS mip/byte count, 실제 설치 byte를 확인했다. 설치한 static/foliage Resources는 geometry 961+component color 287+RNM 4,778+shadow 2,265, 합 8,291개/209,807,631 bytes다. `out/BernLightingRestore20260911/installed_resources.json`이 파일별 근거다. sky UV2는 동명 mesh가 아닌 정확한 assetId MAP_99EDEB6669F1에만 적용했으며 lv_matte의 동명 mesh는 자기 원본 geometry를 보존한다. 설치한 atlas 중 최종 material 문서가 실제 참조하는 것은 RNM 4,618개/SDF 2,194개다. 원본 hidden/no-Baked에만 대응하는 나머지 설치 atlas와 구분한다.
- RenderingProfiles publisher Publish를 통과했다. 실제 C++ probe에서 Bern 5개 volume/source fog Save→Parse, 잘못된 fog 방향 입력 rollback, 315개 light/285개 receiver와 12개 source shadow channel serialize→parse, channel16 및 unknown receiver rollback을 통과했다.
- shadow 변경 8개 C++ /Zs와 채널 확장 뒤 최소 C++를 통과했다. 최종 MapInstance/MeshBinary/Deferred FXC도 통과했다. 기존 원본 DXBC wrapper division warning과 Surface 경고는 root/native 담당이 별도 검토한다. Product 빌드는 root 결과를 따른다.
- 최종 source authoring을 실제 C++ CMapAssetCatalog/MapPlacementDocument로 읽어 23 shard/16,741 assets/23,153 materials/21,321 baked material rows/16,421 shadow material rows, 50,017 placements/49,047 baked instances를 통과했다. 근거는 `out/BernLightingRestore20260911/final_catalog_run.log`다. 최종 material의 실제 UTF-8 문서는 80,533,449 bytes이며 mapmaterials에 한정한 128 MiB/8M value 파서 한도 안에 있다.
- Area Validate와 최종 water 포함 Publish/Check는 50개 runtime 출력/23 shard/50,017 placements를 통과했다. `out/BernLightingRestore20260911/area_validate.log`에 기록했다.
- 모든 mantissa 2²³×15 channel=125,829,120개 조합에서 float32 finite/nonzero, channel decode와 normal/RNM mantissa 보존을 확인했다. CPicking.cpp의 실제 소비자는 W!=0을 확인하고 XYZ를 반환하며 W=1을 복원한다.
- root는 원본 full SDF PS와 실제 helper를 256 gray tone+실제 G8 DDS 5개×width .02/.05/.1, 783 fixture/801,792 pixel로 대조했다. nonfinite 0, mismatch 0, 최대 relative error 0이다. DDSTextureLoader가 실제 DDS 5개를 R8_UNORM으로 읽는 것도 확인했다. 근거는 `out/BernMaterialAudit20260911/shadow_gpu/gpu.log`와 `shadow_results.csv`다. 이 PS 동치 검사는 penumbra 선택 자체를 원본값으로 입증하지 않는다.

## G05. 아직 남은 경계

Landscape 42개는 기존 heightfield·albedo atlas 파생 모델이다. 원본 RNM texture 82개와 ShadowMap2D 42개/atlas 41개의 소유자는 확인했으나 Landscape VF localXY→lightmapUV와 현재 파생 geometry의 대응은 아직 연결되지 않았다. 실제 42개 Landscape MIC를 부모 landscape_base까지 resolve했고 live shader cache에 해당 GUID의 196개 map이 있음을 확인했다. 그러나 활성 terrain layer index를 포함한 정확한 engine-equality map은 42개 모두 발견되지 않았다. 가장 가까운 map의 layer index는 전부 -1이며 이를 실제 원본 shader로 승인하지 않았다. 자료는 `out/BernLightingRestore20260911/LandscapeNative`에 있다.

952×1,612 ushort인 원본 dominant static depth grid도 존재하지만 동적 캐릭터 수광에 필요한 원본 CPU world-to-shadow 및 receiver setup의 실제 소비는 아직 복구하지 않았다. 현재 캐릭터의 일반 realtime shadow와 정적 map placement의 SDF atlas 소비를 구분한다.

source WorldInfo 환경을 연결했지만 원본 CPU SH packing, penumbra CPU setup, 전체 PP/LUT/tonemap, EF camera override의 활성화 및 source easing과 동일성은 아직 확정되지 않았다. 최종 Data Publish/Check는 통과했으며 Product build와 사용자의 Client 화면 관찰은 각각 별도 상태로 구분한다. Client/UI 실행이나 화면 캡처는 하지 않았다.

색 입력도 원본 값과 CPU 해석을 구분한다. source light/environment RGB byte를 255로 나누어 밝기를 곱한 것은 현재 project linear adapter다. 원본 native FColor→linear CPU packing을 확인한 결과로 표시하지 않는다. RNM 4,860개는 원본에서 모두 PF_DXT1이며 SRGB tagged property가 생략되어 있다. Engine 일반 Texture CDO의 SRGB=true와 Texture2D의 추가 override 부재는 확인했으나 intrinsic ULightMapTexture2D의 class/CDO는 Engine/EFGame 패키지에 직렬화되지 않았다. 원본 DLL의 해당 생성자 export와 InitializeIntrinsicPropertyValues를 찾았지만 보호된 packed image여서 실제 초기화 명령을 해석하지 못했다. 그러므로 일반 Texture의 값을 임의 상속시키거나 sRGB로 바꾸지 않았으며 기존 RNM linear SRV 계약은 아직 원본 gamma 확정이 아닌 adapter다. 근거는 `lightmap_gamma_class_audit.json`과 `lightmap_native_exports.json`이다.
