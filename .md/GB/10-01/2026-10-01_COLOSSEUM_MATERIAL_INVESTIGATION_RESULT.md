# Colosseum 원본 재질 조사 및 PR 492 통합 경계

## G00. 이번 작업의 완료 범위

2026-10-01 사용자 범위 조정에 따라 콜로세움 재질은 조사만 수행했다. PR 492의 기존 입장·레벨·관중·리소스 통합은 유지하며, 새 재질 복원과 새 셰이더는 이번 merge에 추가하지 않는다. 이 조사에서 C++·HLSL·Data·Resources·Git index를 변경하거나 Client를 실행하지 않았다. 원본 추출과 감사 산출물은 `out/ColosseumMaterialRestore_20261001`에만 기록했다.

검토 PR은 `096f653656bf532ec0a304272abba98928e22118`, 해당 PR의 기준 commit은 `4fce79521e7a48f57c6d3351ab272dcbd0e67246`이다. 전체 diff 116파일이며 HLSL/HLSLI/FX 및 vcxproj/filters 변경은 각각 0이다. GitHub 파일 목록 100개 표시만으로 전체 변경 범위를 판정하지 않았다.

기존 인계는 [Colosseum 레벨 반입 결과](../09-30/2026-09-30_COLOSSEUM_LEVEL_IMPORT_RESULT.md), 공통 복원 절차는 [렌더링·이펙트 복원](../렌더링이펙트복원V2.md), [LevelPlacementExtractor](../../../Tools/LevelPlacementExtractor/README.md), [Area 데이터 계약](../../TEAM/AREA_DATA_LAYER_GUIDE.md)을 따른다. 이 문서는 실제 조사 결과이며, 전체 원본 재질이나 사용자 화면 확인 완료를 뜻하지 않는다.

## G01. 현재 설치 리소스와 GBResources 전달 목록

`Client/Bin/Resources/Map/LV_PVP_COLOSSEUM/.lostark-area-install.receipt.json`의 ownedFiles 772개를 실제 파일과 SHA-256으로 비교했다. 모두 일치하며 총 211,300,460 byte다. 영역에는 모델 103개, DDS 627개, PNG 42개와 별도 관리 receipt 1개가 있다. 원본 receipt의 admission은 `geometry-preview-partial-material`이고, texture dependency closure는 103개 모두 complete지만 texture slot은 96 complete/7 incomplete, material은 103개 모두 incomplete다. 참조된 texture 파일이 존재한다는 사실을 원본 material coverage 완료로 바꾸지 않았다.

관중은 PR Gameplay.world.json의 40배치·12 archetype이며, 현재 NpcCatalog의 모델 12개와 animation set 4개를 사용한다. 실제 WModel WANM 이름을 읽어 각 placement의 cheer/clap idle clip 40개를 확인했고 누락은 없다. 이 경로의 모델·animation set·texture 의존 파일은 중복 제거 후 77개다.

기존 설치 map 103개, spectator 12개, 항구 shipwright/harbormaster 2개를 합친 WModel 117개에서 WMAT/WMA2/WMA3 texture 경로를 직접 읽었다. 556참조·517개 고유 해석 경로가 모두 존재한다. Colosseum UI layout 4개에서 사용하는 UI 리소스 39개도 모두 존재한다.

통합 담당자가 사용할 Resources-relative 경로, 크기, SHA-256 목록은 다음과 같다.

| 감사 파일 | 범위 |
|---|---|
| `out/ColosseumMaterialRestore_20261001/colosseum-gbresources-delivery-files.json` | map 772 + spectator 77 + UI 39 = 888파일, 누락 0 |
| `out/ColosseumMaterialRestore_20261001/colosseum-map-resource-files.json` | map ownedFiles별 receipt hash 일치 여부 |
| `out/ColosseumMaterialRestore_20261001/colosseum-spectator-resource-files.json` | 관중 모델·animation set·texture |
| `out/ColosseumMaterialRestore_20261001/colosseum-spectator-clips.json` | 40배치의 실제 clip 제공 파일 |
| `out/ColosseumMaterialRestore_20261001/colosseum-ui-resource-files.json` | layout의 UI texture 의존성 |
| `out/ColosseumMaterialRestore_20261001/117-model-texture-closure.json` | 117모델의 embedded texture 경로 전수 조사 |
| `out/ColosseumMaterialRestore_20261001/resource-audit-summary.json` | 위 검사 요약 |

888목록에는 `.lostark-area-install.receipt.json` 관리파일을 포함하지 않는다. 영역 폴더 단위 전달 시 이 receipt를 함께 보존할 수 있다. 이 문서는 전달할 파일의 검증 결과이며 GBResources 복사 자체의 완료 증거는 통합 담당자의 별도 전달 receipt를 따른다.

## G02. 실제 원본 재질과 조명 입력

현재 설치 게임의 `LV_PVP_COLOSSEUM_PS` 원본 UPK를 schema3로 읽었다. source package는 `312NV1V2RCO3OGGQUA2NVGFT.upk`, SHA-256은 `98691b937153c988304ebcec9641a90a139e30955f7e624e4cdefaa4c145943e`다. source placement 1,302개, source StaticMesh 93종, material variant 106종이며 property error와 unresolved placement는 0이다. 현재 설치 1,294배치·103 variants는 기존 인계의 geometry 실패 메시 2종 8배치를 제외한 범위와 일치한다.

UModel의 실제 mesh default material과 actor/component override를 합쳐 설치 103 assets의 150 material slots를 조사했다. 149 slots가 67개 원본 MIC/Material에 연결되며, 공용 parameter 추출기로 67개 전부 부모 상속·명시 0/null·native static set을 읽었다. 추출 실패는 0이며 원본 texture object는 167개다. 남은 1 slot은 `MAP_CE33FC421017_LV_COMMON_MESH_CUL_BOX_3`의 UModel dummy material로 source material identity가 없다. 이름으로 숨김 또는 정상 재질을 추정하지 않는다.

| 원본 terminal | 고유 material | 설치 slot |
|---|---:|---:|
| `bg_simple_opa` | 27 | 77 |
| `bg_base_opa` | 25 | 49 |
| `bg_simple_msk` | 3 | 5 |
| `bg_base_msk` | 4 | 5 |
| `bg_base_trn_depthtest` | 3 | 8 |
| `preset_flag_vertical_msk` | 1 | 1 |
| `monster_base_opa` | 1 | 1 |
| `bg_multi-wet_opa` | 1 | 1 |
| `sky_udk_opa` | 1 | 1 |
| `preset_waterhigh_trn` | 1 | 1 |
| source identity 미해결 dummy | 해당 없음 | 1 |

기존 `source_map_surface.py`에 실제 effective 값과 switch를 넣은 구조 검사에서 기본 opaque/masked 59 materials·136 slots는 현재 branch를 사용했다. 이 검사는 symbolic source texture identity로 branch와 parameter 연결만 검사하며 실제 Resources 경로·원본 색공간·mip·render flag·전체 compiler admission을 승인하지 않는다. 나머지 8 materials·13 slots는 일반 source compiler가 지원하지 않는 terminal이다. Engine에는 wet family와 기존 native character/map program이 있으므로, 일반 compiler 미지원이라는 이유만으로 새 shader가 반드시 필요하다고 판정하지 않았다. 이후 복원에서는 정확한 selected permutation 및 vertex/pixel 입력을 기존 프로그램과 대조해야 한다.

원본 component lighting은 전체 1,302개 중 RNM 1,298개, NULL lightmap 2개, zero LOD 1개, unsupported native layout 1개다. 설치된 1,294배치에 한정하면 RNM 1,290개, NULL 2개, zero LOD 1개, unsupported 1개이며 RNM texture object는 96개다. unsupported는 `LV_PVP_COLOSSEUM_PS:export:2057`, `lv_module.mesh.lv_module_water02_512`의 native kind 0이다. 원본 조명이 없다고 바꾸지 않고 raw tail과 실패 사유를 보존했다. 공용 extractor는 이 자료를 출력했지만 부분 미지원으로 nonzero 종료했다. 오류 0인 placement/parameter 추출과 구분한다.

근거는 `source-placements/LV_PVP_COLOSSEUM_PS.placements.json`, `source-inventory.json`, `installed-inventory.json`, `source-material-slots.json`, `source-material-parameters.investigation.json`, `material-family-investigation.json`, `component-lighting.json`, `component-lighting.receipt.json`, `source-investigation-summary.json`이다. 모두 위 out 디렉터리 안에 있다.

## G03. 베른 원본 지형을 보존하는 PR 492 통합

PR 492의 Bern FLOORFILL은 원본 landscape를 복원한 geometry가 아니라 nav 위치에 기존 tile을 추가한 근사 채움 434개다. PR의 authoring 배치는 50,453개이고 source Landscape 42개는 최종 visible 토큰 0이다. 현재 복원 baseline은 50,021개, FLOORFILL 0개, source Landscape 42개 모두 visible 1이다. 통합 담당자는 현재 source 복원 baseline을 유지하고 근사 fill 434개를 제외하기로 했다.

Bern 변경은 authoring mapplacements, runtime mapset, runtime SL00~09 mapplacements의 총 12파일이다. 신규 Bern mapassets나 신규 shard는 없다. SL00~09의 변경을 기준 commit과 PR 실물 LFS 데이터로 비교한 결과 추가량은 순서대로 51/40/31/31/75/5/79/25/71/26이며, 모든 추가는 FLOORFILL이다. 기존 sourcePlacement 행 수정·삭제와 fill 이외 추가는 모두 0이다. 충돌 난 SL02/SL05만 처리하면 나머지 8개 자동 merge shard에 fill이 남으므로 10개 전체와 mapset·authoring 정본을 함께 보존해야 한다. MapCatalog의 Colosseum 등록은 별도 기능으로 유지한다.

근거는 `pr492-bern-fill-shards.json`과 `pr492-semantic-integration-audit.json`이다. 후자는 parent의 실제 merge 중 live 파일이 conflict pointer였으므로 stage 2(ours)의 실제 LFS payload를 읽어 baseline으로 기록했다. ours SHA-256은 `fdba76562ca82e6d84f79d8b98b8559f3af900bf91b9a2dc513c8d44d0fa512c`, PR payload는 `106870ae2ebb83d7fafaeafad732ece0c4870de16953d1988dd629d896ed54c6`이다. 이 조사 작업이 충돌 파일을 수정하지 않았다.

## G04. 성능 적용 범위와 다음 복원 경계

Colosseum 관중은 기존 `CClientReplication -> CNpc` 경로다. offscreen animation 최적화의 opt-in은 stable placement ID가 있는 town NPC와 non-esther 조건이며 Bern 월드 이름으로 제한하지 않는다. 따라서 40관중도 기존 opt-in 경로를 사용한다. native material, 외부 pose consumer, effect attachment 및 animation transition의 안전 guard는 그대로 적용된다. 관중 40명을 새 인스턴싱 경로로 바꾸지 않았으며 이번 조사로 FPS 개선량을 측정하지 않았다.

추후 전체 재질 복원은 59개 기본 branch의 실제 texture mip·색공간·render flag 검증, 8개 특수 material의 exact shader/permutation 대조, RNM 96 texture와 placement별 UV/scale/bias 연결, water native layout 해독, dummy slot의 원본 identity 확인이 필요하다. RNM atlas pair가 다른 배치는 기존 `build_map_rnm_variant_set.py` 계약에 따라 variant와 placementLighting을 나누며 원본 WModel의 UV1 보존도 확인해야 한다.

기존 import가 제외한 herostatue05 6배치와 castledeco01 2배치의 normal/tangent 실패, 미처리 BSP 7·decal 30·translucent volume 4는 재질 descriptor만 추가한다고 해결되지 않는다. 기존 water 평면의 일반 geometry 처리도 원본 water 표시 완료가 아니다. 원본 surface 복원, 원본 environment/lighting, geometry 누락, 제품 설치·게시·빌드, 사용자의 최종 화면 확인을 각각 별도 증거로 기록해야 한다.

이번에 실행한 검증은 원본 placement/parameter/component 추출, 실제 Resources 및 receipt hash, embedded texture closure, spectator clip 이름, PR sourcePlacement 행 비교, 조사 JSON parse와 문서 diff check다. 정규 Product 빌드·실행·GBResources 전달은 통합 담당자의 결과를 따르며 이 조사 문서에서 중복 완료로 기록하지 않는다.
