# Bern 절벽 UV 조사 결과

## G00. 현재 판정과 변경 범위

2026-10-04 사용자가 제시한 Bern `(98.73, 49.13, -103.39)` 주변의 녹색 절벽을 조사했다. 해당 좌표를 포함하는 Landscape는 **LAND01 LC622**다. 설치된 모델은 급경사 표면에서도 수평 grid UV를 사용하므로 세로로 늘어나는 조건이 실제 수치로 존재한다. 다만 현재 모델·설치 Height DDS·저장된 원본 해독 계약 사이의 불일치는 찾지 못했다. 이것만으로 원본 UV가 잘못 복원됐거나 특정 엔진 버전 때문에 깨졌다고 확정할 수 없다.

**제품 수정은 하지 않았다.** 이번 산출물은 이 RESULT와 `out/BernTerrain20261004`의 읽기 전용 조사 증거다. C++/HLSL, Resources, authoring/runtime 데이터, 사용자 렌더링 설정, 배치 가시성은 변경하지 않았다. 임의 triplanar, 경사별 cliff 투영, 옛 256px bake를 원본 복원으로 적용하지 않는다. 원본 재설치 후 LC622의 실제 VS·static shader set·주변 지형을 가리는 StaticMesh와 가시성 제어를 대조한 뒤 증명된 차이만 수정한다.

조사 후 문서 작성 기준 HEAD는 `1cea418bcaa79a61c605a5ee54ad9f290eaaee7e`다. 병행된 [Rendering Workbench 결과](2026-10-04_RENDERING_WORKBENCH_PRESENTATION_RESULT.md)의 기본 재질/단계별 미리보기는 이 절벽을 고친 결과가 아니며, 같은 문제로 묶어 완료 처리하지 않는다.

## G01. 위치와 실제 설치 모델

사용자 두 번째 Bern 화면의 F1에 `98.73, 49.13, -103.39`가 표시됐다. 최초 수치 조사는 화면의 반올림 좌표 `(98.7, 49.1, -103.4)`를 사용했다. 아래 삼각형까지의 거리는 이 반올림 지점 기준이며 정확한 화면 픽셀의 ray pick 결과는 아니다.

| 항목 | 확인값 |
|---|---|
| Area | `LV_BER_BERNCASTLE` |
| stable placement ID | `15689846671518173995` |
| source ID | `LV_BER_BERNCASTLE_T_LAND01:landscape:export:622` |
| asset ID | `MAP_4EC52FE2DC08_LAND01_LC_00622` |
| placement TRS | position `(78.72, 0, -78.72)`, quaternion `(0, 0, 0, 1)`, scale `(1, 1, 1)` |
| CModel geometry 단위 | cm → m, preScale `0.01` |
| world bounds | X `78.72..118.4`, Y `-8.54..50.05`, Z `-118.4..-78.72` |
| geometry | WINT 1.0, 정점 `3,969`, 삼각형 `7,688`, painted material slot `1` |
| slot 이름 | `LANDSCAPE_BAKED` — 이름과 별개로 source ON 경로는 typed DDS를 소비 |
| material family | `bg-source-landscape-opaque`, runtime `SOURCE_LANDSCAPE_OPAQUE` / marker `14` |
| source material | `lv_ber_berncastle.landscape.lv_ber_berncastle_land_01_mi` |
| component grid | `[186, 186, 62, 31]` |
| active painted layers | `1, 2, 3, 4, 5`, weightmap `2`개 |
| native PS ID | `1214bdd3f0bc154b929471e76129689f` |
| native shader map key | `4716a59af603af1335ac38d48ac2431515016c736ba18c029cd7a4448a6d55de` |

실제 조사 파일은 다음과 같다.

- 배치: `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements`, LC622 행 `16091`.
- catalog: `Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_LANDSCAPE.mapassets`.
- 모델: `Client/Bin/Resources/Map/LV_BER_BERNCASTLE_T/Landscape/MAP_4EC52FE2DC08_LAND01_LC_00622/MAP_4EC52FE2DC08_LAND01_LC_00622.wmodel`.
- authoring 재질: `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapmaterials.json`.
- 게시 재질: `Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE.mapmaterials.json`.
- 원본 해독 계약: `Tools/LandscapeExtractor/SourceContracts/BernSourceLandscape.v1.json`.
- shader: `Client/Bin/ShaderFiles/Shader_SourceLandscapeSurface.hlsli`, `SourceLandscapeLayerUV` 및 `EvaluateMapSourceLandscapeSurface`.
- 바인딩: `Engine/Private/Material.cpp`, `Bind_SourceLandscapeSurface`.

모델 SHA256은 `480c20c1fb2488d89f515112a794ffcbb6176a9d39b9a91cf9336e26e5907421`이며 문서 작성 직전 다시 확인했다. Height DDS는 `Map/LV_BER_BERNCASTLE_T/Landscape/NativeLayers/Height/765a88d81953_land01_09134.dds`, weight DDS는 같은 `NativeLayers/Weights`의 `4fa1594e3070_land01_09139.dds`와 `858238d7d8ac_land01_09140.dds`다. 전체 설치 의존성 hash는 `lc622-audit.json`에 보존했다.

## G02. 설치본에서 직접 확인한 값

`Tools/ModelAssetConverter/cook_wmodel_geometry_contract.py`의 실제 WModel decoder로 정점과 index를 읽었다. Height DDS의 RG를 16bit 높이로 복원하고 같은 component grid의 정점과 비교했다. 재질 행은 runtime, authoring, Git 관리 source contract를 대조했다.

| 검사 | 결과 | 해석 범위 |
|---|---|---|
| LC622 runtime 재질 행 = authoring 재질 행 | 동일 | 게시 누락으로 서로 다른 행을 읽는 상황은 아님 |
| active layer UV·색·specular·weight·blend·factor | 계약 대비 차이 `0` | 비활성 normal의 intensity `0`은 기존 builder 계약대로 처리 |
| `UV0 = component grid / 62` | 최대 오차 `4.2300070502e-8` | 설치된 `3,969`정점 전수 |
| XZ = grid × `0.64m` | 최대 오차 `2.4414062523e-6m` | Z 반전과 cm→m를 각각 기존 경로대로 한 번 적용 |
| 설치 Height DDS RG → mesh Y | 최대 오차 `2.4414062523e-6m` | 재설치 원본 패키지와의 독립 대조는 아님 |
| `abs(normal.y) < 0.2`인 삼각형 | `739 / 7,688` | 수평 투영에 비해 급경사인 면이 실제 존재 |
| 면적 / 수평 XZ 투영 면적 최대 비 | `108.556546` | `1 / abs(normal.y)`인 기하 지표이며 GPU filtering 측정값은 아님 |
| 가장 가까운 위 조건 삼각형 | index `4656`, 중심 `(100.6933, 48.3967, -102.6133)` | 반올림 제보 위치에서 `2.2554m`; 면적비 `5.0281` |

shader의 layer UV는 아래 순서로 계산된다.

```text
localGrid = rawUV * grid.z
gridXY = localGrid + grid.xy
centered = gridXY * 0.1 - 0.5
angle = sourceRotationScalar * 3.140000104904175
layerUV = (rotate(centered, angle) + 0.5) * tiling
```

이 식에는 높이 Y가 들어가지 않는다. 거의 같은 XZ에서 Y가 크게 변하는 절벽은 작은 texture 좌표 변화가 긴 표면에 걸쳐 보이게 된다. 설치 데이터에 그 조건이 있다는 것은 확인했지만, 원본에서도 해당 Landscape 면이 보이는지와 원본 VS가 실제 PS 입력을 어떻게 만드는지는 추가 대조가 필요하다. nearest paint 값은 texture의 가까운 texel을 확인한 보조 자료이며 bilinear·height blend·mip·조명까지 실행한 최종 픽셀 값이 아니다.

## G03. 디스크 설정과 실행 중 선택의 경계

위 결과는 **디스크에 설치된 LC622와 source material ON 경로**에 대한 조사다. 사용자 화면에는 `MATERIAL_RENDER_SETTINGS.bUseSourceMaterials`의 현재 실행값이 없으며 실행 중 Client 값을 읽거나 UI를 조작하지 않았다.

`Recovered material equations`가 OFF이면 원본 family14 대신 기존 embedded `LANDSCAPE_BAKED` PNG 경로가 사용될 수 있다. 이 경우 화면과 위 native layer 계산은 다른 경로다. 재검증 때는 현재 live bool과 실제 family14 바인딩 여부를 먼저 기록한다. 디스크 JSON이 정상이라는 이유로 실행 중 source 경로가 활성화됐다고 단정하지 않는다. 사용자의 렌더링 저장값을 원인 확인 없이 켜거나 덮어쓰지 않는다.

## G04. 이전 복원 기록과 이번 조사의 연결

- [08-04 Landscape 결과](../08-04/2026-08-04_BERN_CASTLE_LANDSCAPE_VISUAL_RECOVERY_RESULT.md)는 이전 grass/road bake에 cliff 입력이 빠진 문제와 side-projection 근사를 기록했다. 이후 08-05 사용자 화면 FAIL 정정이 있으므로 당시 산출물을 완료된 원본 복원으로 재적용할 근거가 되지 않는다.
- [08-14 제품 가시성 결과](../08-14/2026-08-14_BERN_LANDSCAPE_PRODUCT_VISIBILITY_RESULT.md)는 제품 load scope에서 빠진 Landscape를 포함한 변경이다. 이번 LC622의 UV를 복구한 기록은 아니다.
- [09-22 preview 결과](../09-22/2026-09-22_BERN_LANDSCAPE_PREVIEW_RECOVERY_RESULT.md)는 구형 flat/cliff 파생 모델의 미리보기 복구이며 이후 source-painted 정본을 대신하지 않는다.
- [09-30 terrain 결과 G08](../09-30/2026-09-30_BERN_AND_CHARACTER_SELECT_TERRAIN_RESTORATION_RESULT.md)은 현재 정본이다. 전체 42개 component의 native static key와 PS를 맞췄으며 원본 PS의 `layercliff` sample은 `0`개였다. 256px bake와 과거 경사별 projection을 다시 설치하면 G08 계약을 되돌린다.

G08의 수치 검증은 LC762 원본 PS와 제품 helper에 일정한 texture sample을 주입한 WARP 40건, 최대 오차 `2.3841858e-6`이었다. LOD0 UV register 168개 float 및 subsection 좌표 검증도 기록돼 있다. 그러나 원본 VS 전체 실행, 거리 LOD morph, 공간 UV/mip filtering, 실제 사용자 화면 PASS를 완료한 것은 아니다. 이번 LC622의 절벽은 그 남은 공간·가시성 경계를 확인해야 하는 사례다. 과거 LAND02 LC762의 PASS를 LC622 화면 성공으로 대체하지 않는다.

## G05. 원본 재설치 후 필요한 정확한 입력

사용자는 용량 때문에 원본을 삭제했다고 확인했으며 재설치 완료 보고는 없다. 최초 조사에서는 종전 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages`, `C:/Users/user/Desktop/Resource_LostArk`, `C:/Users/user/Desktop/Final_LostArk`, `_work`, `out/BernTerrainRestore_20260930/source-support`가 없었다. 2026-10-04 06:34 KST 재확인 때 Packages 폴더와 일부 새 UPK는 생성됐지만, 아래 LAND01 UPK와 ReleasePC의 component 기본값 패키지는 아직 없었다. 다운로드 중 일부 파일의 존재를 해당 원본 준비 완료로 판정하지 않았다. Git 관리 `BernSourceLandscape.v1.json`은 해독 결과를 보존하지만 원본 VS/DXBC와 원본 전체 geometry/control을 대신하지 않는다.

우선 필요한 입력을 아래 범위로 한정한다. 난독화된 물리 이름과 export 번호는 기존 설치 버전의 식별값이므로 재설치 버전의 logical package·object path·serial hash를 먼저 대조한다. 새 버전에서 같은 번호를 다른 object에 적용하지 않는다.

| 우선 입력 | 정확한 기존 식별값 | 확인할 내용 |
|---|---|---|
| LAND01 패키지 | logical `LV_BER_BERNCASTLE_T_LAND01`; 이전 물리 `ReleasePC/Packages/978TBWF8XBWFNI4MT9W8XT8T94NP6D.upk` | LC622, LandscapeProxy, height/weight, material instance, ShaderCache |
| LC622 object | export `622`; `theworld.persistentlevel.landscapeproxy_0.landscapecomponent_15` | serial SHA256 `1d78a81e40aa012dd7847e217bb0f496c676bb202911caf6f9cb3b556a2556a3`와 비교 |
| component 기본값 | 이전 물리 `ReleasePC/NE1FENCQ4UNE9ZPRENOQS.u`, export `18361`, `Default__LandscapeComponent` | actor/component/CDO의 가시성·그림자·부모 기본값; castShadow 증거 serial SHA256 `f45ae041f99acb915bdc9043a224156bff19382139dfe1ff50e8785a81cf37df` |
| 해당 static shader set | LC622 `shaderMapKey` 및 PS ID는 G01 표 참조 | 같은 static set의 실제 VS와 PS 입출력, LOD0/거리 LOD UV·height·basis 전달 |
| 재질 package | logical `LV_BER_BERNCASTLE`, object `landscape.lv_ber_berncastle_land_01_mi`; base material ID `2cf1c13888745046a6a2742eff3f6b86` | local component MIC의 parent chain·static switch와 master parameter 일치 |
| 주변 배치와 streaming | logical `LV_BER_BERNCASTLE_T_PS`, `LV_BER_BERNCASTLE_T_SL08` 및 PS가 실제로 참조하는 제어 package | 해당 bounds를 덮는 StaticMesh/절벽 geometry 누락 여부, actor/component/archetype/CDO, 초기 Hide/UnHide·Matinee/Toggle |

SL08의 가까운 위치 확인 기준은 `LV_BER_BERNCASTLE_T_SL08:export:4983`, stable placement `14594720950671088309`, 위치 `(99.52,49.13,-102.35)`다. 이 바닥 모델은 `MAP_ED6C570CC114_BG_EUD_MORAY_FLOOR01B_SM_YSI_OVR_DF92231E065C_RNM_443D3A5AB74C`다. 같은 근처의 export4982도 위치 확인 보조 자료다. 이 두 바닥을 문제가 보인 녹색 절벽 그 자체로 단정하지 않는다.

LC622 활성 texture 의존성의 logical object는 다음 7개다. 물리 UPK는 재설치 패키지 index에서 정확한 logical 이름으로 resolve한다.

```text
lv_grass_01.tex.lv_common_grass_24_d
lv_grass_01.tex.lv_common_grass_42_d
lv_ber_berncastle.tex.bg_pap_sforest_floor01d_d_ksr
bg_pap_sforest_a.tex.bg_pap_sforest_floor01d_d_ksr
bg_pap_sforest_a.tex.bg_pap_sforest_floor01a_n_ksr
bg_ber_berncastle_d.tex.bg_ber_berncastle_floor03a_d_ksr
bg_ber_berncastle_d.tex.bg_ber_berncastle_floor03_n_ksr
```

LAND02 전체 또는 Resources 전체를 먼저 다시 추출하지 않는다. LC622와 위 원본 참조에서 필요한 dependency만 확장한다. 원본 패키지와 유효한 `umodel_lostark_v7.exe` 경로가 확인되면 `Tools/LandscapeExtractor/README.md`의 source-only 후보 절차로 새 `out` 폴더에 추출하고 설치본은 보존한다.

## G06. 이어서 확인할 순서와 완료 기준

1. live `bUseSourceMaterials`와 LC622의 실제 draw/material branch를 확인한다. 실행 중 사용자 설정과 저장값을 별도로 기록한다.
2. 재설치 source의 logical package·object path·version·hash를 대조하고 LC622의 height, SectionBase, scale/bias, holes, UV와 설치본을 비교한다.
3. LC622 static shader set의 **VS와 PS를 함께** 대조한다. 원본의 실제 공간 좌표를 넣은 테스트로 LOD0 UV, weight/height 경계, mip derivative, 거리 LOD 입력까지 분리해 확인한다. constant texture sample만으로 UV 성공을 판정하지 않는다.
4. `(98.73,49.13,-103.39)`와 LC622 bounds 주변의 원본 StaticMesh coverage 및 actor/component 초기 표시 제어를 비교한다. 원본에서 다른 바위/절벽 mesh에 가려지는 Landscape인지, source component의 다른 재질 permutation이 필요한지 증거로 결정한다.
5. 입증된 코드 또는 stable ID/필드 차이만 후보로 만들고 검증한다. 데이터 교체가 필요하면 최신 저장본에 필드 단위로 병합하고 기존 편집·ID·TRS·렌더링 튜닝을 보존한다.
6. 실제 source 대조, 후보 설치/publish, 제품 빌드, 사용자 화면 확인을 각각 기록한다. 이번 RESULT는 **설치본 조사 완료, 원본 대조·수정·화면 확인 미완료**다.

## G07. 실행 증거

산출물은 `out/BernTerrain20261004/nearby-landscapes.json`, `nearby-materials.json`, `audit_lc622.py`, `lc622-audit.json`이다. 앞의 두 JSON은 조사 시 읽은 해당 범위의 snapshot이며 최신 전체 데이터 정본이 아니다. 재개 시 최신 디스크에서 같은 stable ID의 입력을 다시 읽고 사용한다.

실행한 수치 조사 명령:

```powershell
$env:PYTHONUTF8 = '1'
& 'C:/Users/user/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' `
  out/BernTerrain20261004/audit_lc622.py
```

이 실행은 설치 WModel·DDS·typed row의 수치 대조다. Client/UI, 원본 게임, Product build, GPU raster 비교, 자동 Reload는 실행하지 않았다. 사용자 화면의 정확한 draw call 식별과 수정 후 수동 확인도 아직 수행하지 않았다. 이번 변경 파일은 본 Markdown 하나뿐이며 `git diff --no-index --check -- /dev/null <RESULT>`로 whitespace를 확인했다.
