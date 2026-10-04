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


## G08. 재설치 완료 후 현재 판정

사용자의 재설치 완료 이후 원본을 다시 읽었다. G05~G07의 다운로드 대기·원본 미확인 기록은 최초 조사 시점이며, 현재 원본 대조 상태는 이 절과 G09가 정본이다. **정적 원본 데이터 대조는 완료했지만 녹색 늘어짐을 고칠 근거가 되는 차이는 찾지 못했다.** 이번에는 UV·형상·재질·가시성이나 렌더링 옵션을 바꾸지 않았다. 화면 문제를 복구 완료로 판정하지 않는다.

재설치 LAND01 패키지 SHA256은 `a682e279fa5d39421d2b0c4936847e5fe73b1506ccee2c4f1020cf9b61e4abb2`다. 패키지 version868/licensee16, engine12097/cooker136이며 이 파일은 UE3 포맷이다. 이 대상 파일을 UE4 전환으로 설명할 근거는 없다. logical object path와 serial을 우선 비교했고 LC622의 export index622와 UE reference623을 구분했다. 현재 LC622 serial은 기존 `1d78a81e40aa012dd7847e217bb0f496c676bb202911caf6f9cb3b556a2556a3`와 정확히 같다.

| 입력 | 재설치 원본 대조 결과 |
|---|---|
| 높이 texture9134 | 5개 mip의 DDS payload 전부 동일 |
| 가중치 texture9139/9140 | 각각 7개 mip payload 전부 동일 |
| LC622 형상 | 3,969정점·7,688삼각형의 topology/winding 동일, 위치 최대 차이2.441406e-6m |
| static shader key | `4716a59af603af1335ac38d48ac2431515016c736ba18c029cd7a4448a6d55de` |
| 원본 VS | `c9f016c5efaba04a9c98794fa70eac1b`, section base를 더한 평면 XY 출력 |
| 원본 PS | `1214bdd3f0bc154b929471e76129689f`, XY에0.1/-0.5·회전·tiling 적용 |

해당 원본 VS/PS에서 높이에 따라 cliff UV를 만드는 경로를 찾지 못했다. `layercliff`의 weightIndex=-1도 일치한다. 따라서 임의 triplanar나 side projection, winding 반전을 원본 복구로 넣지 않았다. 원본 부모 재질의 tagged property에는 TwoSided=true가 없지만, native/CDO의 모든 기본값을 복원한 것은 아니므로 원본 cull 상태 전체를 증명했다고 확대하지 않는다.

native shader cache는 `9XUFAXIP8BXBAP1NIEG66EF.upk` export224에서 확인했고 대응 DXBC와38개 native disassembly를 보존했다. UModel의 일반 texture/mesh 출력만으로 끝내지 않고 static shader identity와 실제 VS/PS 입출력을 대조했다. 그래도 실행 중 선택된 material branch, 거리 LOD, 원본 게임의 같은 camera에서 가려지는 면과 최종 raster 값은 별개다.

근거는 `out/BernSourceReload20261004/lc622-uv-contract-evidence.json`, `lc622-winding-comparison.json`, `lc622-material-map.json`, `lc622-shader-extraction.json`, `land01-shadercache-identity.json`이다. Client/UI와 원본 게임을 실행하지 않았다. 이어지는 원인 판정에는 사용자가 보는 동일 camera의 실제 draw/material branch와 원작 화면 대조가 필요하며, 이번 정적 대조만으로 shader의 다른 값을 추측해 바꾸지 않는다.

## G09. 주변 geometry 누락과 표시 제어 대조

### 주변 StaticMesh와 초기 표시 계약의 재설치 원본 대조

2026-10-04 재설치가 완료된 원본에서 `LV_BER_BERNCASTLE_T_PS`의 `WorldInfo.streamingLevels` 배열을 해독했다. 실제 연결된 하위 레벨은 34개이며 PS를 포함한 35개 패키지만 조사했다. `AlwaysLoaded` 이름만으로 표시를 확정하지 않았고, 추출기는 actor/component의 instance → archetype → CDO 표시 값을 schema3으로 보존했다. package 범위를 Resources 파일명 검색으로 확장하지 않았다.

이 원본 집합의 StaticMeshComponent 배치는 31,246개이며 property 오류와 unresolved placement는 모두 0이다. 913개 고유 StaticMesh의 native `FBoxSphereBounds`를 읽고 source actor/component TRS와 기존 UE3→Client 좌표 변환을 적용했다. component에는 독립 `CachedLocalBox/Bounds`가 없으므로 mesh의 원본 bounds를 사용했다. 표본의 native bounds는 UModel `-dump` 값과도 일치한다.

| 검사 | 결과 | 범위 |
|---|---|---|
| PS + SL08 원본 배치 | 3,272개, 누락 0 | source ID·catalog 원본 object path·visibility 모두 일치, TRS 최대 차이 `5e-7` |
| 실제 streaming closure | 35패키지, 31,246배치 | PS의 정확한 `streamingLevels` 참조에서만 확장 |
| LC622 world AABB와 겹치는 배치 | 1,387개 | PS18, SL08 458, SL00 182, SL06 205, SL04 524 |
| 위 1,387개의 현재 배치·WModel | 누락 0 | catalog의 정확한 원본 object path 불일치 0 |
| 위 1,387개의 source/runtime visible | 불일치 0 | 표시 1,370개 + 원본부터 숨겨진 PS culling box17개 |
| 제보 위치에서 AABB 거리 10m 이내 | 208개, 배치 누락 0 | 이름에 cliff/rock이 없는 구조물도 포함 |
| 다른 패키지의 현재 미배치 항목 | 47개, LC622와 겹침 0 | source actor TRS 기준 최소 AABB 거리36.44m, 기존 연출/owner/camera 경계는 유지 |
| 원점 거리65m 밖의 큰 mesh | SL00 export5059 검출 | water mesh 원점468.18m, bounds는 LC622와 겹치며 현재 표시 배치 존재 |

47개 미배치 연출 항목을 독립 지형으로 바꾸거나 복원하지 않았다. 위 거리는 저작된 초기 actor/component TRS 기준이며 연출 재생 중 parent/camera 이동을 재현한 값이 아니다. 원본 cached AABB는 넓은 후보 범위를 잡는 근거일 뿐 실제 삼각형의 가림이나 GPU 표시 성공을 뜻하지 않는다.

PS의 `LevelLoaded` export519(UE reference520)는 Loaded and Visible 출력에서 `AkPostEvent`, `AkStartAmbientSound`, Music/SoundStream용 `MultiLevelStreaming` 세 액션으로 연결된다. PS에는 `ToggleHidden`이 없고 SL08에는 Kismet sequence action/event가 없다. 연결35패키지의 import table에서 LC622와 겹치는 actor/component의 정확한 외부 참조1,730경로를 대조했으며 외부 직접 참조는0이다. 이 결과를 원본 엔진의 모든 동적 숨김·서버·태그 제어를 재현했다는 뜻으로 확대하지 않는다.

### 가까운 구조물 5개의 실제 형상·재질 표본

배치 존재만으로 가림이 정상이라고 결론내리지 않도록 가까운 벽·구조물과 기존 cliff/floor 기준 모델을 원본에서 다시 glTF로 추출했다. 원본 glTF는 meter 단위이고 설치 WModel은 기존 `geometryPreScale≈0.01`과 Z 반전을 소비한다. 이 기존 변환을 적용한 실제 정점과 triangle index를 비교했다.

| source placement | 모델 | 원본/설치 정점 | 원본/설치 삼각형 | 정점 최대 차이(m) |
|---|---|---:|---:|---:|
| SL00:5358 | `bg_ber_berncastle_wall06b_sm_ksr` | 2,248 / 2,248 | 1,536 / 1,536 | `4.76353e-7` |
| SL08:5414 | `bg_ber_berncastle_structure06_sm_ksr` | 3,129 / 3,129 | 2,470 / 2,470 | `4.67084e-7` |
| SL08:6584 | `bg_ber_kronap_wall02_sm_asj` | 200 / 200 | 136 / 136 | `7.43866e-7` |
| SL08:6148 | `bg_ber_stone_cliff03_sm_ksr` | 1,026 / 1,026 | 1,670 / 1,670 | `1.31205e-7` |
| SL08:4983 | `bg_eud_moray_floor01b_sm_ysi` | 90 / 90 | 96 / 96 | `1.14441e-7` |

합계6,693정점·5,908삼각형이며 모든 index list가 기존 handedness 변환 후 정확히 일치한다. 즉 표본에서 삼각형 손실·잘못된 scale·추가 winding 반전은 발견되지 않았다. 가장 가까운 큰 벽 SL00:5358의 world AABB는 `(92.4064,39.5327,-102.1504)`~`(112.0136,49.3473,-97.4419)`이며 제보 위치와 AABB 거리는1.23963m다. `cliff03`은 y14.31~17.54m로 해당 높은 위치의 벽 자체가 아니다.

5개 모델이 실제 사용하는9개 material slot은 모두 설치 WModel의 이름과 typed source row가 연결되어 있다. 정본 authoring과 게시된 runtime의9개 행은 동일하고 모든 해당 DDS가 존재한다. 소비 family는 `bg-source-opaque-masked`와 `bg_base_opa_overlay`이며 현재 `MapAssetCatalog`가 각각 `SOURCE_BG_OPAQUE_MASKED`와 `SOURCE_OVERLAY_OPAQUE`로 처리하는 지원 경로다. 일반 catalog opacity/color alpha는1, 해당 surface diffuseColor alpha도1이며 source flag64(alpha clip)는9개 모두OFF다. `Shader_MapMaterialSurface.hlsli`의 BG/overlay clip 조건과 대조했을 때 이 표본에서 alpha0 또는 미지원 family로 전체가 사라지는 설정은 없다.

glTF 추출에는 `-notex`를 사용했다. 따라서 glTF가 기록한 `dummy_material_*` 색은 원본 재질 근거로 사용하지 않았다. material identity는 원본 UModel dump, 실제 WModel slot 및 현재 typed source 행으로 확인했다. 각 MIC 전체 shader를 새로 GPU 실행한 검증은 아니며 실행 중 `bUseSourceMaterials` 값과 실제 draw call 성공도 확인하지 않았다.

### 결론과 증거 경계

현재 조사에서는 **원본의 별도 절벽/벽 StaticMesh가 빠져서 LC622의 녹색 면이 드러났다는 증거를 찾지 못했다.** 임의 mesh 추가, 숨김 해제, winding/two-sided 변경, side projection 또는 triplanar 보정을 적용하지 않았다. 원작의 같은 카메라 화면과 현재 draw call을 대조하지 않았으므로 해당 Landscape 픽셀의 실제 가림·색·UV 표시가 원작과 같다고 판정할 수는 없다.

이 추가 조사는 원본·현재 설치본의 정적 수치 대조까지 완료한 상태다. 코드, 정본 Data, Resources, 렌더링 저장값, 원작 파일은 변경하지 않았다. Client/UI, 원작 게임, GPU 캡처, Product build, 자동 Reload도 실행하지 않았다.

재실행 근거는 `out/BernCliffCoverage20261004` 아래에 있다.

- `streaming-closure.json`, `source/*.placements.json`, `expand-streaming.log`: 실제 WorldInfo 참조와35패키지 schema3 원본 추출.
- `source-mesh-bounds.json`, `closure-mesh-bounds.json`, `closure-placement-bounds.json`: 원본 bounds, 물리 package/header hash, 실제 TRS와 overlap 목록.
- `source-runtime-comparison.json`, `closure-runtime-comparison.json`, `runtime-links.json`, `final-evidence.json`: 현재 stable source ID·TRS·visible·catalog/model/texture 존재 대조.
- `source-control-properties.json`, `external-controls.json`: PS 초기 액션과 SL08 Kismet 및35패키지 import table 대조.
- `source-geometry/`, `*.dump.log`, `geometry-sample-audit.json`, `final-evidence.log`: 원본5개 재추출, 실제 WModel6,693정점/5,908tri 및9재질 슬롯 검증.
- `expand_streaming.py`, `audit_closure_bounds.py`, `compare_closure_runtime.py`, `export_geometry_samples.py`, `audit_geometry_samples.py`, `audit_external_controls.py`, `finalize_evidence.py`: 위 수치의 읽기 전용 재현 스크립트.

## G10. 분류·실행 배치·재질과 누락 Decal 독립 감사

2026-10-04 지정점 `(98.73,49.13,-103.39)` 주변을 현재 게시된 `LV_BER_BERNCASTLE.mapset`의 실제24개 shard에서 다시 읽었다. G09의 authoring 배치 존재를 화면 성공으로 대신하지 않았다. **LV_MODULE 이름이나 package 분류 때문에 이 지점의 벽·풀을 제외한 증거는 찾지 못했다. 별도로 원본 static Decal의 실행 연결 누락을 확인했지만 지정점은 그 투영 범위 밖이다.** RNM/static shadow 수정·빌드 결과는 이 절의 감사 범위에 포함하지 않는다.

### 로드·표시 경로와 실제 설치 재질

`Client/Private/LevelRegistry.cpp`의 `MakeBernMapScope`는 전체 범위를 포함하고 excluded asset group을 지정하지 않는다. `MapPlacementRuntime::Apply_LoadScope`는 이 같은 scope를 사용한다. `build_maptool_scene.py`의 module/package 진단은 이름만으로 숨기지 않으며 `sourceVisibility`와 source ID별 명시 override를 소비한다. `Loader::Ready_MapArea`는 필요한 stable asset ID 집합을 준비하고 CModel 또는 재질 생성 실패 시 전체 stage를 실패시킨다. 필수 asset을 조용히 건너뛰는 경로는 발견하지 못했다.

| 직접 다시 확인한 분모 | 결과 |
|---|---|
| 현재 runtime mapset24 shard | 배치50,021개, catalog16,743개 |
| LC622 AABB와 겹치는 원본 StaticMeshComponent1,387개 | runtime source ID 누락0, object path 불일치0, visible 불일치0, WModel 누락0. visible1,370개와 원본 hidden17개 유지 |
| overlap 안 LV_MODULE | SL00 export5059 `lv_module.mesh.lv_module_water02_512`1개, visible=true. 물 평면Y10.714266m이며 지정점 AABB 거리38.415734m |
| LAND01/SL08/SL00/SL04/SL06 원본 foliage instance | 각각6,559/240/811/328/183개, 합계8,121개. runtime 누락0, visible 불일치0, TRS 최대오차5e-7 |
| 지정점10m 이내 foliage origin | 189개, hidden0 |
| 원본 static AABB거리10m 미만 + 위 foliage의 visible 모델 | 고유 WModel130개, 실제 사용 material slot206개,92,863삼각형. 빈 geometry·없는 slot·typed override 미연결·texture 파일 누락0 |
| 위206재질의 설치 입력 | typed texture193개 존재. opacity·diffuse alpha 모두1. deferred/back202개, translucent/back4개 |
| BG alpha clip + foliage/grass diffuse34개 DDS | decode 오류0, 전체 texel이 cutoff 아래인 texture0, 각각 최대alpha255 |

foliage 비교는 원본 v868 instance array의80-byte stride와 전체 suffix 길이를 검증하고 기존 `decode_instance_matrix`로 읽은 TRS만 사용했다. RNM numeric validation과 분리했으며 이 검사를 foliage 조명 복원 성공이라고 기록하지 않는다. 모델130개의 family는 BG100, foliage76, grass12, overlay14, water1, translucent3개의 실제 사용 slot으로 분해했다. DDS에 통과 texel이 있다는 것은 해당 mesh UV의 모든 sample이나 화면 pixel 성공을 뜻하지 않는다.

`MapAssetObject::Submit_FinalCamera/Render_Group`와 `MapStaticBatchObject::Render`의 runtime 표시·presentation opacity·scene environment replacement·camera culling·material render group·draw 실패 조건도 대조했다. 이들은 현재 live camera와 런타임 상태를 소비한다. 이번 정적 감사에서는 해당 상태를 실행 관측하지 않았으므로 실제 submit·culling 통과·GPU draw·가림을 PASS로 쓰지 않는다.

### 원본에서 표시되지만 연결되지 않은 static Decal

실제 streaming35패키지의 native export를 다시 집계한 결과 DecalComponent는86개, material은11종이다. LAND01/02는14/21개, SL03/05/06/07/08/09는2/3/1/3/40/2개다. LC622를 receiver로 지정한 것은 LAND01 export14/23/26 세 개다. 가장 가까운26은 다음 계약이다.

| 입력 | 원본 실측 |
|---|---|
| source actor/component | `LV_BER_BERNCASTLE_T_LAND01.theworld.persistentlevel.decalactor_36.decalcomponent_0`, component index26 |
| receiver | UE ref623 = export index622 = `landscapeproxy_0.landscapecomponent_15` |
| 중심 | `(94.963017578125,50.709301757812504,-104.8772265625)`m, 지정점과4.346975m |
| width/height/far plane | 215.625/215.625/315.625cm. NearPlane은 CDO에도 없고 zero default |
| 초기 표시 | actor/component instance→archetype→CDO hidden=false. LAND01 sequence action/event/variable/interp track0,35패키지의 해당 actor 외부 직접참조0 |
| material | `lv_decal_01.mat.lv_common_decal_05_mi` → `efbasematerial_lv_prologue.decals.decal_translucent` |
| 선택 입력 | `texture_diffuse=lv_decal_01.tex.lv_common_decal_05_d`; native static switch `1.use_opacity_texture=false` 두 entry를 확인, diffuse alpha branch 사용 |

이 material chain과 branch는 기존8월 문서만 인용하지 않고 현재 설치된 native3패키지에서 `source_evidence`를 다시 실행해 확인했다. Bern authoring/runtime Map 문서, Effect 데이터 및 Resources에서 이 source decal의 연결을 찾지 못했다. 기존 `08-01/2026-08-01_BERN_CASTLE_FULL_RECONSTRUCTION_RESULT.md`도185~200행에서 이 첫 fixture를 probe만 완료했고 runtime projection은 아직 구현하지 않았다고 구분한다.

지정점의 원본 HitTangent/HitBinormal/HitNormal 좌표는 `(0.0442784,4.0496958,-1.5793018)`m다. 사각형 반폭은1.078125m이므로 지정점은 footprint 밖2.9715708m다. 나머지85개에도 지정점을 포함하는 투영 사각형이 없다. 이 누락은 **주변 LC622 장식이 미복원인 실제 사례**이지만 녹색 늘어짐 지점을 직접 덮던 절벽이나 데칼을 찾은 것은 아니다. 전체 원본 게임의 동적 서버·태그 제어를 재현했다는 뜻도 아니다.

### 복원 후보의 현재 경계

기존 `CMapEffectPresentationRuntime::Load_AmbientArea`와 `CEffectDocumentRenderer::Render_Decal`은 Bern의 게시 Map Effect에서 depth projector까지 연결할 수 있다. 그러나 `EFFECT_DECAL_RECEIVER_MODE`는 ALL_OPAQUE/UPWARD_SURFACES만 제공한다. 기존 shader는 actor를 제외하지만 이 원본이 지정한 특정 LandscapeComponent receiver 목록을 제한하지 못한다. CModel material family에도 이 static Decal 전용 계약은 없다.

따라서 같은 diffuse를 일반 quad로 얹거나 모든 opaque 면에 투영하는 것을 원본 복구 완료로 처리하지 않는다. 기존 projector에 stable receiver 대상 계약과 검증된 원본 재질을 연결하거나, 원본 receiver 삼각형에 정확히 clip한 decal geometry를 기존 CModel/CMaterial로 전달하는 후보를 검토할 수 있다. native parent shader와 blend/depth/receiver 계약을 확인한 뒤 선택할 후속 후보이며 이번 감사에서 구현·설치하지 않았다. 원본26의 native suffix는44byte여서 이 payload만으로 복구할 완성 decal vertex buffer가 있다고 가정하지 않았다.

### 이번 실행과 증거

원본·runtime·WModel·DDS는 읽기만 했고 Client/UI, 원작 게임, GPU capture, Product build와 Reload는 실행하지 않았다. 이번 추가 파일 변경은 이 RESULT와 `out`의 감사 증거뿐이다. G09의 자료와 다음 기록을 함께 사용한다.

- [streaming-closure.json](../../../out/BernCliffCoverage20261004/streaming-closure.json), [closure-placement-bounds.json](../../../out/BernCliffCoverage20261004/closure-placement-bounds.json): 실제35패키지 정본 범위와 overlap1,387개 source identity·TRS·표시 근거.
- [geometry-sample-audit.json](../../../out/BernCliffCoverage20261004/geometry-sample-audit.json): G09의 원본5모델 및 실제 WModel 대응 증거. G10의130모델은 현재 설치 material/geometry 유효성 검사이며130개 전부의 원본 재추출을 뜻하지 않는다.
- [audit_native_decals.py](../../../out/BernCliffCoverage20261004/audit_native_decals.py), [native-decal-audit.json](../../../out/BernCliffCoverage20261004/native-decal-audit.json): native86개 위치·receiver·material·원본 header/serial hash, 가장 가까운26의 CDO visibility와 현재 material chain 재검증, 지정점 footprint 배제.

재현 명령은 `python out/BernCliffCoverage20261004/audit_native_decals.py`다. 이 스크립트는 감사 JSON만 `out`에 기록한다. 최초 대화형 Python 실측의 runtime/foliage/130모델 결과는 위 표에 기록했으며 이 Decal 재현 스크립트가 그 별도 검사까지 실행한다고 확대하지 않는다.
## G11. 실제 Landscape 조명 입력 누락 확인과 구현

G08/G09의 형상·표면 UV·주변 StaticMesh 대조는 그대로 유효하지만, **원본과의 차이를 찾지 못했다는 당시 결론은 지형 조명 입력을 빠뜨린 불완전한 조사였다.** 지정 위치 `(98.73,49.13,-103.39)`의 LAND01 LC622에는 원본 FLightMap2D의 RNM2개와 ShadowMap2D1개가 있고 현재 family14는 이 입력을 거부하고 있었다. 재설치 원본의 현재42개 지형도 모두 자신의 RNM·shadow를 가진다. 이 누락은 실제 코드와 원본의 차이다. 조명 누락 확인을 planar UV 늘어짐 자체의 원인 확정으로 확대하지 않는다.

LC622의 lightmapped native policy는 `DistanceFieldShadowedDynamicLightDirectionalLightmapTexturePolicy`이며 PS는 `aebf747d7945b3428e591c1aba105ed8`이다. 앞선 G08은 같은 material의 NoLightmapPolicy만 대조했다. 현재 설치 EFEngine의 생성자·padding 계산·subsection vector·VF binding과 source shader constant 위치를 추적해 lightmap UV의 CPU 입력을 확인했다. DLL을 격리 helper에서 로드하고 메모리를 읽었으며 원본 엔진 함수·게임 EXE·Client/UI는 실행하지 않았다.

LC622의 현재 정규화된 mesh UV0에는 `UV * 0.953125 + 0.015372984111309052`를 적용한 뒤 각 texture의 원본 atlas scale/bias를 적용한다. 원본4개 subsection×32²정점과 float32 대조 최대오차는 `5.96e-8 UV`다. 원본42개의 component62/subsection31/2개·StaticLightingResolution4를 각각 확인했고, atlas가 다른 항목에는 자기 atlas 값을 합성했다. Geometry·hole·surface tiling·placement TRS는 변경하지 않는다.

family14 parser, CModel의 material 복사·경로 검증, CMaterial의 RNM/G8 shadow 로드, MapAssetRenderUtils의 기존 조명 binding을 연결했다. 일반·instanced VS는 이 family의 lighting 좌표에 UV0를 사용한다. PS는 Heightmap BA에서 복원한 pixel TBN으로 RNM의 view-dependent 입력을 계산하며 VS world axes를 그대로 사용하지 않는다. source shadow GUID는 `79ad76d6208c814790c50f5f74b7b8a0`이며 기존 channel1·penumbraWidth0.05·exponent2의 `PROJECT_ADAPTER` 경계를 유지한다. 이 transfer 상수를 원본 CPU 복원값이라고 하지 않는다. CoordinateBias `[0,0]`은 원본 ShadowMap2D→Core.Object CDO 체인까지 확인했다.

| 검증 | 확인한 범위 |
|---|---|
| 원본 native tail 독립 재파싱 |42개 own RNM reference·coefficients·atlas·ShadowMap2D ref 일치 |
| 원본 texture 후보 |126개, 7,357,320B, 원본 전체 mip, 업스케일·재압축·mip 생성0 |
| 설치 후 DirectXTK WARP |126개 Texture/SRV mip 범위 및 모든 GPU mip payload 일치 |
| RNM pixel basis 수치 |7,260조건, 원본 수식 대조 최대오차4.66e-15; CPU float64 검증 |
| 일반·instanced shader focused compile |VS_MAIN/PS_MAIN 합계4/4 통과 |
| 공식 publisher 계약 테스트 |4개 test method 통과; 잘못된 texture/좌표/source ID 입력에서 이전 게시본 보존 |
| 후보 JSON 독립 대조 |기존 source에 bakedLighting42필드·placementLighting42행만 추가; 나머지 JSON 값·행순서 변경0 |
| 교차 코드 리뷰 |일반 모델 UV1·기존 BG lighting bank·Resources 경계 보존 확인 |

재현 도구는 `Tools/LandscapeExtractor/build_source_landscape_lighting_candidate.py`이며 기존 builder의 optional `--lighting-candidate`와 연결한다. `out/BernLandscapeLightingReview20261004/LightingCandidate/candidate-manifest.json`, `mapmaterials.patch.json`, `independent-candidate-audit.json`, `installed-landscape-gpu-load.json`에 대상별 근거를 보존했다. CPU 원본 경로는 `out/BernLandscapeLightmapUvReview20261004/lc622-lightmap-uv-cpu-proof.json`에 있다. source shader의 기존 X4000 경고는 수정 전 helper에서도 재현되며 이번 변경으로 새로 만든 경고가 아니다.

최종 제품 빌드·authoring 반영·실제 게시 결과와 사용자 화면 확인은 다음 절에서 별도로 기록한다. 설치 파일의 GPU load 성공은 실제 베른 camera의 draw·가림·색·UV 화면 PASS가 아니다.

## G12. 10-04 최신 저장본 반영과 제품 게시

G11 후보를 최신 authoring 저장본의42개 stable asset ID와 source placement ID에 병합했다. 기존 값·배치 순서는 유지하고 bakedLighting42개와 placementLighting42행만 추가했다. 교체 직전 SHA256을 다시 확인하고 백업·원자적 교체·자기 변경 rollback 절차를 사용했다. authoring SHA256은 `1356438aa912774aaef98737bf579d48564fab67ebba321863e4677c0a32806b`에서 `446e171b803a4a4a47a65cabf17f99a8ff12f2be2b8ac08bbea0b4b5f7cd513c`로 바뀌었다.

공식 `Publish-MapAuthoring.ps1`의 전체 Area 검증으로53개 후보 파일을 생성·검증한 뒤 같은 publisher의 `Complete-MapPublish`와 `Invoke-FileSetTransaction`으로 material 문서1개만 게시했다. 전체 게시의 mapeffects 개행 정규화가 무관한 파일까지 바꾸는 것을 확인했으므로 이번 변경에는 포함하지 않았다. 제품 publisher에 별도 scope나 두 번째 게시 경로를 추가하지 않았다. runtime material SHA256은 `8dc924d73ab3971037677a7270a8d6e160396e89a32e822d50549e6c1921ea8d`이고 authoring과 JSON 값이 같다. 나머지 runtime 파일 bytes는 유지됐다. 첫 시도는 검증 도중 중단되어 자기 authoring 변경을 rollback했으며 최종 성공 receipt만 완료 근거로 삼는다.

`Client/Private/MapPlacementDocument.cpp`의 기존 `Find_PlacementLighting` 조회에는 family14 제외 조건이 없음을 확인했다. 원본126개 DDS도 설치 후 실제 DirectXTK WARP의 Texture/SRV mip 범위·GPU payload 검증을 통과했다. Client/UI 실행·Reload·저장된 렌더링 옵션 변경은 하지 않았다. 이 반영은 원본 지형 조명 입력 복구이며 지정점의 녹색 늘어짐이나 최종 화면 색을 사용자 대신 PASS로 판정하지 않는다.

Debug와 Release Product의 Engine/Shared/Server/Client compile·link·deploy가 모두 PASS다. receipt는 `out/BuildPipeline/runs/20261004T002815359Z-debug-product.json`, `20261004T005228448Z-release-product.json`이며 현재 작업공간의 기존 변경도 포함한 통합 빌드다. Release Client 단계는 변경 OBJ2개·CSO59개·binary2개를 기록한다. 기존 shader X4000·SDK 인코딩·DirectXTK PDB 누락 경고가 있으나 컴파일·링크 오류는 없다. 이 빌드 도구 자체는 이번 map 데이터를 게시하지 않았으며 앞서 기록한 공식 map publisher의 게시 receipt와 구분한다.

최종 설치·게시 근거는 `out/BernLandscapeLightingReview20261004/installed-publish-receipt.json`, `publish-installed.log`, `installed-landscape-gpu-load.json`이다. 복구 Resources의 전달용 물리 복사본은 `out/RenderingRestore20261004/Resources`에 준비했다. 이번 Bern126개·Character Select56개·Valtan2개를 합한184개 DDS, 12,715,152B이며 설치본과 hash가 같다. Drive 업로드는 수행하지 않았고 Git 제외 Resources를 강제 추가하지 않았다. Character Select·Valtan 원본 mip 복구의 개별 결과와 남은 후보는 각각 `2026-10-04_RENDERING_SOURCE_EVIDENCE_RESULT.md`, `2026-10-04_VALTAN_WAITING_STONE_MATERIAL_RESULT.md`를 따른다.

## G13. 동시 감사의 native mip 교정 반영과 최종 파일 대조

G11/G12의126개 GPU payload 일치는 당시 생성한 DDS가 GPU에 그대로 올라갔다는 검증이다. 별도 `Audit character select and Bern` 세션의 원본 bulk 전수 대조에서 LAND02 `ShadowMapTexture2D_199`의 mip4가 잘못 추출됐음이 확인됐다. 압축 컨테이너 길이와 해제 후 길이가 같을 때 기존 helper가 비압축으로 오판한 결함이다. 따라서 앞선 전체 원본 mip 일치 주장은 이1개 mip에 대해서는 정정한다.

별도 세션은 bulk flags로 압축 여부를 판정하도록 extractor를 수정하고 2026-10-04 10:31:49 KST에 해당 DDS를 교체했다. 이 세션의 마지막 readback에서 발견한 hash 차이는 그 정당한 교정본이었다. `out/BernNativeLightingMipRestoration20261004/install-receipt.json`의 원본 serial·수정 전후 hash·후보·설치본을 읽어 일치함을 확인했으며 제품 파일을 다시 덮어쓰지 않았다. 이 DDS의 최종 SHA256은 `ec9b9696824c8cc0df62e4a7b1860e168872fa9655fbe6ed6dfd5c9ccf33f35d`다. 같은 별도 receipt의 기존 Bern RNM4,616개 mip 복구는 그 세션의 작업이며 G11의126개 설치와 혼동하지 않는다.

이쪽 전달용 사본의 해당 DDS만 교정본으로 갱신하고 이전 bytes와 receipt를 `out/RenderingRestore20261004/reconciled-delivery-backup`에 보존했다. 워로드 작은 돌 WModel1개를 포함한 최종 전달본은185개(184DDS+1WModel),12,727,088B다. `out/RenderingRestore20261004/final-readback.json`에서185개 제품·전달 파일의 hash 일치, Bern authoring/runtime JSON 일치, 최종 Warlord V/Alt V3문서 hash, 기록한 Debug/Release 빌드 PASS를 재확인했다. 이 읽기 검증은 빌드 이후 다른 세션의 C++ 변경까지 재빌드했거나 실제 Client 화면을 확인했다는 뜻이 아니다. Drive 업로드와 실행 중 저작 도구의 Reload는 수행하지 않았다.

후보 생성기의 source/output hash 확인만으로는 이전 decoder가 만든 잘못된 G8 캐시를 배제하지 못하는 재발 경로도 확인했다. `build_source_landscape_lighting_candidate.py`는 PF_DXT1 RNM에만 기존 캐시를 허용하고 PF_G8 shadow는 항상 원본 flags로 재해석하도록 수정했다. 실제 LAND02 원본·이전 shadow199 DDS·유효한 이전 receipt를 넣은 회귀 검사에서 구 dispatch는잘못된350ab08d…를, 수정 dispatch는설치본과같은ec9b9696…를 만들었다. mip4의256byte 중229byte가 교정됐고 나머지8mip은동일했다. 실제RNM1개는 UModel 호출을 차단한 상태에서도 기존 캐시를 그대로 재사용했다. 제품쓰기0과 설치hash불변을 확인했으며 Python문법·scoped diff check를 통과했다. 근거는 `out/BernLandscapeLightingCacheReview20261004/cache-regression-receipt.json`이다.
