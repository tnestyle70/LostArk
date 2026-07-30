# 발탄 동적 환경·Deploy 런타임 구현 결과

- 작성일: 2026-07-30
- 대상: `LV_LUT_HEARTRB_ED`, zone `37051`
- 상태: 구현·데이터 생성·Debug/Release 빌드·AssetTest smoke 완료
- 최종 시각 판정: 대상 카메라 수동 QA 대기

## 1. 결론

발탄 맵은 이제 정적 PS/SL 배치, 환경 재질, 수동 phase sky, gameplay Deploy Prop을
서로 다른 데이터 layer로 읽는다.

```text
정적 exact asset                    260
환경/복원 overlay asset               9
Map Catalog v3 합계                  269

정적 exact placement             13,091
overlay placement                    12
Map placement 합계                13,103

Deploy catalog                        9
Deploy visual placement              85
  static intact/fractured            77
  skeletal ITR_02326                  8
```

정적 Map placement에 파괴 Prop을 억지로 섞지 않았다. Loader는 먼저 Map/Deploy 모델을
`CModel` prototype으로 등록하고, AssetTest의 MapTool이 placement를 clone한다. Map은
`Layer_Map_*`, gameplay Prop은 `Layer_DeployProps`를 사용한다. Deploy 로드는
`parse -> validate -> stage -> commit`이며 실패하면 새 layer만 rollback한다.

## 2. 외부 비평 반영 결과

### P1. Emissive occlusion leak

`MRT_GameObject`에 `Target_Emissive`를 추가한 뒤 같은 MRT를 쓰는 모든 G-buffer
writer가 `SV_TARGET4`를 기록하도록 수정했다.

```text
Shader_VtxMesh.hlsl             0 출력
Shader_VtxAnimMesh.hlsl         0 출력
Shader_VtxAnimMeshBinary.hlsl   0 출력
Shader_VtxNorTex.hlsl           0 출력
Shader_VtxMeshBinary.hlsl       asset emissive 출력
```

따라서 뒤쪽 crack 발광이 먼저 그려진 뒤 전경 비발광 오브젝트가 그려져도 emissive
RT가 남지 않는다. binary map shader는 4 render mode와 3 cull mode 조합 12 pass를
갖는다.

### P1. Deploy summary 불일치

`valtan_arena.deployprops.json`은 112개 subset을 다시 summarize한다.

```text
records.length                 112
summary.recordCount            112
globalSummary.recordCount      169
global records.length          169
```

### P2. Trigger 표현

111은 구조 파싱된 Trigger node 직접 참조 수가 아니다. deploy actor ID의 little-endian
4바이트 패턴이 `TriggerMapData.loa`에서 2~6회 발견된 레코드 수다. 필드명과 문서를
`triggerBinaryOccurrenceCount`/`binary ID occurrence`로 교정했다.

### P2. Emissive intensity

전역 `CMapAssetObject` 상수 `0.35`를 제거했다. emissive intensity는 Catalog v3의
asset별 render profile이 소유한다. crack 재구성 asset만 `0.35`를 쓰고 다른 authored
emissive asset은 독립 값을 유지한다.

### P2. Intact/fractured provenance

LookInfo가 직접 가리키는 mesh reference 77개는 모두 `FracturedStaticMesh`다. intact
77개는 같은 package의 sibling export에서 찾았다. JSON과 문서에서 direct reference와
same-package sibling을 분리했다.

### P3. 상태·경로·로그

overlay 상태는 `intact-arena-reconstruction-overlay-deploydata-audited`로 갱신했다.
결과 문서는 `C:/Users/user/Desktop/Resource_LostArk/...` 절대 경로를 사용하며 모든
빌드 로그와 종료 코드를 아래에 기록했다.

## 3. 쇠사슬·구름·스카이

### 쇠사슬

exact ImportTable/placement와 exact material texture를 사용했다.

```text
MAP_2F7659F7259C_BG_FAT_KARLAJAVIL_CHAIN01A_SM   placement 4
MAP_9ACE87B2E07E_BG_FAT_KARLAJAVIL_CHAIN01B_SM   placement 9
texture: bg_fat_karlajavil_elevator01a_{d,n,s}_khb
```

### CloudPlane

`MAP_3CC7E67937A0_BG_LUT_ZAMOUNT_CLOUDPLANE_SM_OLD` 2개 exact placement를 쓴다.
diffuse/normal/opacity가 exact이며 Translucent, UV scale `(2, 2.5)`, UV speed
`(0.015, 0)`, opacity `0.45` profile로 흐른다.

### 기본 sky mirror

`MAP_EDDEDF2CF6A1_SKY_MIRROR_SM` 한 개 exact placement와 `lv_sky_0161_d` exact
texture를 쓴다. Background render mode와 Front cull로 구 내부 면을 렌더한다.

### SpaceHole/ChaosGate

원본 ParticleSystem material에서 exact texture 6개를 복구했다.

```text
SpaceHole: fx_d_cloud_031, fx_d_atypical_019, fx_a_cloud_017
ChaosGate: fx_d_shockwave_001_ycl, fx_k_cloudtilie_01, fx_k_electile_02
```

현재 geometry는 video-matched proxy plane이며 원본 UE3 ParticleSystem topology라고
주장하지 않는다. 6개 placement는 기본 hidden이다. MapTool의
Baseline/SpaceHole/ChaosGate radio는 시각 검증용이며 TriggerMap/Matinee의 정확한
시점과 fade를 대체하지 않는다.

## 4. Crack material

Floor01A/B의 exact diffuse/normal을 유지하고 원작 영상에서 보이는 녹색 틈빛만
`VIDEO_MATCH_RECONSTRUCTION` emissive mask로 분리했다. diffuse에 녹색을 구워 넣지
않는다. deferred emissive와 asset별 intensity를 사용하며 넓은 bloom halo는 별도
post-process 경계다.

## 5. Deploy Prop와 AnimModel

아레나 112개 중 시각 모델이 있는 85개를 별도 runtime 문서로 만들었다.

```text
ITR_02306   5     ITR_02307   4
ITR_02308   6     ITR_02309   2
ITR_02310   3     ITR_02311   2
ITR_02315  28     ITR_02316  27
ITR_02326   8 skeletal
```

Static family는 intact/fractured WModel 두 개를 등록하고 MapTool에서
Intact/Fractured/Despawned를 전환한다. `ITR_02326`은 exact skinned WModel이다.
skinned source에는 `--pretransform`을 쓰지 않아 skeleton을 보존했다.

단, source glTF는 `meshes=1`, `skins=1`, `animations=0`이고 WModel도
`skeleton=yes`, `animations=0`이다. LookInfo에
`AnimSet'ITR_02326.Ani.ITR_02326_Ani'` 문자열은 있으나 해당 AnimSet export는 아직
확보되지 않았다. 현재는 exact bind pose이며 fabricated animation은 넣지 않았다.

## 6. 저장 파일

### Runtime 입력

- `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapassets`
- `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements`
- `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.deployassets`
- `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.deployplacements`
- `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/LostArk/Map/LV_LUT_HEARTRB_ED`
- `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/LostArk/Map/ValtanPhase`
- `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/LostArk/Deploy/LV_LUT_HEARTRB_ED`

### 생성기와 런타임 코드

- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_valtan_environment_runtime.py`
- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_valtan_phase_layers.py`
- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_deployprop_runtime.py`
- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_maptool_scene.py`
- `C:/Users/user/Desktop/LostArk/Client/Public/DeployPropCatalog.h`
- `C:/Users/user/Desktop/LostArk/Client/Private/DeployPropCatalog.cpp`
- `C:/Users/user/Desktop/LostArk/Client/Public/DeployPropObject.h`
- `C:/Users/user/Desktop/LostArk/Client/Private/DeployPropObject.cpp`

### Receipt

- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/placements/valtan_environment_runtime_receipt.json`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/placements/valtan_phase_runtime_receipt.json`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/placements/maptool_scene_receipt_G5_dynamic_environment.json`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/deploydata/deployprop_runtime_receipt.json`

## 7. 데이터 invariant

`08_Data_Invariants.json` 결과:

```text
mapAssetHeader/version/count       v3 / 269 / rows 269
mapPlacement count/rows            13,103 / 13,103
deployAsset count/rows             9 / 9
deployPlacement count/rows         85 / 85
arena records/summary              112 / 112
global summary/records             169 / 169
binary occurrence records          111
FracturedStaticMesh direct          77
intact same-package sibling         77
SkeletalMesh direct                  8
```

Map receipt는 negative-scale 5,042개, reflection 4,521개, nav helper hidden 139개,
phase overlay hidden 6개를 보존한다.

## 8. 빌드와 실행 검증

로그 root:

```text
C:/Users/user/Desktop/LostArk/.codex_tmp/validation/valtan_dynamic_runtime
```

| 단계 | 로그 | 종료 코드 | 결과 |
|---|---|---:|---|
| Engine x64 Debug Rebuild | `13_Engine_Debug_Rebuild.log` | 0 | PASS, 오류 0 |
| UpdateLib Debug | `14_UpdateLib_Debug_AfterRebuild.log` | 0 | PASS |
| Client x64 Debug Rebuild | `15_Client_Debug_Rebuild.log` | 0 | PASS, 오류 0 |
| Engine x64 Release | `04_Engine_Release.log` | 0 | PASS, 오류 0 |
| UpdateLib Release | `05_UpdateLib_Release.log` | 0 | PASS |
| Client x64 Release | `06_Client_Release.log` | 0 | PASS, 오류 0 |
| Python py_compile | `07_Python_PyCompile.exitcode.txt` | 0 | PASS |
| data invariant | `08_Data_Invariants.json` | 0 | PASS |
| 변경 HLSL 7개 최종 FXC | `19_HLSL_Final.log` | 0 | PASS |
| AssetTest smoke | `18_Runtime_AssetTest_Smoke.log` | 0 | PASS |

Debug/Release build가 HLSL을 실제 FXC `fx_5_0`으로 컴파일했고, 최종 source의 변경
shader 7개도 별도 FXC로 재검증했다. `Shader_VtxMeshBinary.cso`를 포함한 build output과
검증용 `.fxo`가 생성됐다. 경고는 기존
C4819/C4244/C4267, FXC deprecated, DirectXTK/Effects PDB LNK4099 계열이며 오류는 0이다.

AssetTest smoke 순서는 초기 Enter → Logo 확인 → F2 → asset loader → Enter → 창 제목
`Valtan WModel Asset Test` 확인 → 4초 생존이다. 테스트가 시작한 프로세스만 종료했다.
현재 `Client/Bin`은 마지막 Debug Rebuild 산출물 상태다.

## 9. 완료와 미완료 경계

완료:

- 정적 269/13,103 map parse·prototype·clone 경로
- 쇠사슬 exact D/N/S, CloudPlane 흐름, 기본 sky render profile
- crack emissive per-asset 처리와 G-buffer occlusion leak 수정
- Deploy 9/85 parse·validate·stage·commit·rollback 코드 경로
- static intact/fractured/despawned와 skeletal bind-pose render 경로
- 환경 phase와 Deploy state 수동 selector
- Debug/Release/FXC/data/runtime smoke

아직 원작 자동 재생으로 확정하지 않는 항목:

- TriggerMapData node/field 구조와 난이도·phase 조건
- Matinee의 정확한 활성 프레임, fade, particle spawn/despawn timing
- 원본 ParticleSystem topology와 emitter behavior
- `ITR_02326_Ani` exact AnimSet과 실제 animation clip
- crack 최종 체감 밝기, CloudPlane 속도, sky 화면 구성의 대상 카메라 시각 QA
- 파괴 particle/audio/hit/restore sequence와 navigation bake

따라서 현재 결과는 “맵과 gameplay visual state를 런타임에 올리는 기반”까지 완료다.
원작의 레이드 진행을 자동 재생하는 마지막 단계는 Trigger/Matinee/AnimSet 정본을 연결한
뒤에 닫는다.
