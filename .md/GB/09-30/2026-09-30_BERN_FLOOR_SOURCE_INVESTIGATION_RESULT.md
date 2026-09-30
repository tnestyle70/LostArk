# 2026-09-30 베른성 빠진 바닥의 원본 소스 조사 RESULT

읽기 전용 조사다. 저장소 파일·Resources·Data는 수정하지 않았고 빌드·게시·git·Client 실행도 하지 않았다. 이 문서만 새로 썼다.
스크래치(재현 스크립트와 산출물): `C:\Users\USER\.claude\jobs\46aea322\tmp\bern_floor_source\`
표기: [확인] 파일·값으로 직접 본 것, [추정] 확인한 값에서 이끌어 낸 것, [미확인] 아직 못 본 것.

## 1. 결론 (한눈에)

1. **바닥이 없는 자리의 정체는 이번 조사로도 확정하지 못했다.** 다만 "어떤 소스가 아니다"는 확실히 좁혔다 (2절).
2. 아닌 것 [확인]: 존 Deploy Prop, BSP `Model`, `DecalActor`, `EFTranslucentVolume`, 씬·스트리밍 레벨, 뒤집힌 평면, 읽지 못한 메시, 추출 이후 바뀐 원본 패키지.
3. 우리 추출의 StaticMesh 계열 배치는 **원본과 개수까지 정확히 일치한다** (원본 StaticMeshComponent 31,152개 = 우리 배치 31,152개) [확인]. 즉 "추출기가 배치를 흘렸다"는 가설은 배제된다.
4. 원본에는 그 자리에 걸을 수 있는 땅이 **있다** [확인]. 원본 내비메시(`NaviMesh.epf`) 정점 10,389개를 해독해 겹쳐 보면, 우리 메시 바닥이 없는 넓은 구역에도 정점이 그 높이(38.4~38.6m)에 놓여 있다.
5. 남은 유력 후보는 **숨겨 둔 Landscape 42개**다. 메시 바닥이 없는 내비 정점 중 Landscape가 밑에 있는 290개의 33%는 Landscape 높이와 ±3m 안에서 일치한다 [확인]. 나머지 2/3는 Landscape가 10~20m 위이거나 10m 이상 아래라서 이 가설로 설명되지 않는다 [확인].
6. 원본 바닥이 실제로 무엇인지는 [미확인]이다. 다음 단계는 6절에 적었다.

## 2. 소스별 판정

| 후보 | 결과 | 근거 |
|---|---|---|
| (a) 존 DeployData의 Prop | **아님** | 베른 존은 `11102`(`LV_BER_BernCastle_T_PS`). `leveldata1.lpk`의 `MapData/11102/DeployData.loa`에 `CEFDeployActor_Prop` 378개. 레코드 배치는 쿠크와 같은 계열(마커 끝 +0x00 위치 cm, +0x0C 회전, +0x30 액터 id, +0x38 스케일 %, +0x64 PropDefinitionId)이라 그대로 해독됨 [확인]. `EFTable_Prop`과 연결되는 것은 84개, 모델 40종이 전부 `EFDLProp_ITR_*`(상호작용·퀘스트·호감도·기억의 오르골 소품)다. 나머지는 모델 없는 볼륨(탈것 하차 볼륨 등). 같은 파일에 NPC 1,220, PathNode 249, QuestZone 239 등이 있다 [확인]. |
| (b) BSP `Model` 76개 | **아님(볼륨 브러시)** | PS의 `Model` 76 / `Polys` 76 / `BrushComponent` 77은 볼륨 액터 77개와 숫자가 맞는다: `EFAreaNameVolume` 22, `EFLevelStreamingVolume` 16, `EFMinimapVolume` 11, `EFIndoorVolume` 10, `EFEnvironmentInfoVolume` 5, `EFPathBlockingVolume` 3, `LightmassImportanceVolume` 3, `EFSearchPathObjectVolume` 2, `TriggerVolume` 2, `CullDistanceVolume` 1, `DDLExcludeVolume` 1, `EFCharPerfOptionOverrideVolume` 1 = 77 [확인]. 보이는 지오메트리가 아니다. |
| (c) `DecalActor` 86개 | **아님(바닥 자체 아님)** | SL08의 40개는 건물 창문 데칼(`bg_ber_berncastle_window_decal01/02`), Land01/02의 35개는 도로·흙 데칼(`lv_common_decal_05`, `bg_ocn_common_decal02`, `lv_lut_zamount_decal01`), 나머지 SL03/05/06/07/09에 11개. 데칼은 기존 지오메트리 위에 그리는 것이라 바닥을 만들지 못한다 [확인]. |
| (d) 지형 외 지형 액터 | 없음 | `LandscapeProxy` 2 (Land01/02)뿐. |
| (e) `InstancedStaticMeshComponent` 1,692개 | **바닥 아님(foliage)** | 별도 스크립트 `extract_ue3_foliage_placements.py`가 InstancedFoliageActor를 처리하고 결과는 overlay 배치(LAND01 6,559, LAND02 7,988, SL00 811 등)로 이미 들어 있다 [확인, README 548행 이하와 receipt]. |
| (f) 우리 카탈로그에 없는 서브레벨 | 바닥 없음 | PS가 스트리밍하는 35개 이름을 전부 조사. `Scene01A~07G` 16개는 배치가 0~45개(대부분 InterpActor·Emitter), `Standard_Track`은 Matinee 트랙 4,262 export, `Standard_MaterialChange`는 재질 트랙 77 export. 바닥이 될 메시 없음 [확인]. `EVENT01`(1,127), `SCENE03E`(45)는 이미 shard에 있고 기본 숨김이다. |
| `EFTranslucentVolume` 41개 | **아님** | 속성이 brush·collision·location·tag뿐이고 Model이 1.8KB인 작은 볼륨. 재질·표시 속성이 없다. 스폰 광장 높이(42.6m)에 11개가 있으나 지오메트리가 아니다 [확인]. |
| 뒤집힌(아래를 향한) 평면 | **아님** | 아래를 향한 평평한 삼각형이 광장 빈 면적을 덮는 비율이 3%(스폰 광장 62㎡ / 1,928㎡) [확인]. |
| 메시 로드 실패 | **아님** | 광장 일대 6,564종 자산이 모두 읽힌다(읽기 실패 0) [확인]. |
| 추출 후 바뀐 원본 | **아님** | 베른 레벨 패키지 mtime이 SL00~10은 07-27, PS는 08-03 09:00이다. 우리 shard 영수증은 08-03 23:37 [확인]. |

## 3. 빠진 곳의 규모 (다시 측정)

방법: 우리 배치(`Data/Maps/Authoring/LV_BER_BERNCASTLE.mapplacements`)와 실제 WModel 지오메트리로 x 95~185, z -85~5를 0.5m 격자로 래스터(위쪽 면, 카탈로그 경로 사용, 높이 38~47m). 윤곽은 원본 미니맵 `BernCastle.png`.

| 구역 | 윤곽 면적 | 메시 바닥 | 다른 고체 | 완전히 비어 있음 |
|---|---:|---:|---:|---:|
| 전체(95~185) | 3,996㎡ | 1,162 (29%) | 465 | 2,369 |
| 스폰 광장(100~180, -80~0) | 3,297㎡ | 1,012 (31%) | 357 | 1,928 (58%) |
| 룬 광장(137,-45) 반경 22m | 674㎡ | 422 (63%) | 27 | 225 (33%) |

- 이전 문서(스폰 광장 39% / 비어 있음 1,336㎡)와 방향이 같다. 제 값이 더 엄격하다(높이 38~47m 밴드, 이름 분류 사용). 수치 차이는 분류 기준 차이이고 결론은 같다.
- 지도(`plaza_raster.png`): 밝은 초록(메시 바닥)은 x 130~143 사이 남북 통로(폭 약 10m)와 아래쪽 쐐기뿐이다. 통로 좌우의 넓은 공간(서쪽 x 100~125, 동쪽 x 148~185)은 윤곽 안인데 바닥 메시가 없다 [확인].
- 룬 광장 반경 14m에는 배치 457개, 자산 72종이 있다. 분홍 포장은 `BG_RHD_BREEZE_BRIDGE01D_SM_MSJ`(17), `BG_EUD_MORAY_FLOOR01_SM_YSI`(6) 등 이름이 BRIDGE/FLOOR인 조각들이다 [확인].

## 4. 원본 내비메시와의 대조

`leveldata2.lpk`의 `Common/MapData/11102/NaviMesh.epf`(8,596,129 bytes)를 추출해 분석했다.

- 자기 서술형 이진 포맷이다: 헤더에 `mesh3D`, `verts`, `tris`, `majorRelease`, `minorRelease`, `maxStepHeight`, `sectionID`, `edge0/1/2StartVert`, `edge0/1/2Connection` 필드 이름이 있다 [확인].
- **정점은 완전히 해독했다** [확인]: 파일 오프셋 216부터 145,662까지 `04 04 <x:u16> 05 <y:i32> 06 <z:i32>` 레코드 10,389개가 틈 없이 이어진다. 값은 cm. Bern 좌표는 `x = X/100`, `z = -Y/100`, `높이 = Z/100`. 범위 x 21~260, z -177~379, 높이 27.8~54.0m.
- 좌표 매핑 검증: 메시 바닥 셀의 83%가 정점 4m 안에 있다. 부호를 반대로 하면 0%다 [확인].
- **삼각형(연결 정보) 해독은 실패했다** [확인]. 정점 뒤 레코드가 `태그 1바이트 + 값 2바이트` 반복으로 시작하지만 중간부터 규칙이 달라져 어긋난다(태그가 1~255로 퍼짐). 삼각형·면적은 아직 못 읽었다.
- 겹쳐 본 결과(`plaza_raster_navverts.png`): 통로에는 정점이 있고, 메시 바닥이 없는 동쪽 넓은 공간(x 148~185)에도 정점이 촘촘하다. 메시 바닥이 없는 자리에 정점이 4m 안에 있는 비율은 58%(2m 안 35%, 8m 안 89%)다 [확인]. 서쪽은 정점이 드문데, 큰 삼각형 몇 개로 덮인 구역이라 정점이 적을 수 있다 [추정].
- 지역별로 "정점 높이 ±1.5m 안에 우리 메시 표면이 없는 정점"은: 서쪽 41/284, 통로 8/135, 동쪽 304/544다. 즉 **동쪽 정점의 56%가 우리 메시 표면으로 지지되지 않는다** [확인].

## 5. Landscape 가설

- 숨겨 둔 Landscape 42개(`LV_BER_BERNCASTLE_LANDSCAPE`, 각 약 39.7m)는 이 구역의 89%(28,849/32,400셀)를 덮는다 [확인].
- 그러나 높이가 제각각이다. 지형 위쪽 면 높이 분포: -20~-5m 37%, 정확히 0.0m 14%, 30~36m 14%, 36~40m 7%, 48~60m 12% [확인]. 통로 아래에서는 중앙값 -8.5m라 통로는 지형 위에 뜬 구조물이다.
- 메시 바닥이 없는 윤곽 셀 9,481개 중 8,398개 밑에 지형이 있고, 그중 30~36m 22%, 36~40m 9%, 44~48m 5%, 48~60m 32%다. 마을 바닥 높이 밴드(38~47m)에 있는 셀은 12%뿐이다 [확인].
- 지형과 내비 정점을 직접 비교하면(메시 지지가 없는 정점 중 지형이 밑에 있는 290개): 지형-내비 높이 차 ±3m 안 95개(33%), 지형이 10~20m 위 100개(34%), 지형이 10m 이상 아래 88개(30%) [확인]. 동쪽 예: (182,-14) 내비 38.5m / 지형 36.5m, (179,-4) 내비 38.4m / 지형 0.0m.
- 해석 [추정]: 지형이 일부 구역에서는 실제 걷는 땅과 일치하고(약 1/3), 나머지는 지형 자체의 구멍(Landscape visibility/hole)이나 높이 해석 문제일 수 있다. 정확히 0.0m인 셀 14%는 데이터가 없는 곳이 0으로 채워진 흔적으로 보이며 [추정], 확인이 필요하다.

## 6. 추출 파이프라인의 어디가 놓치나 / 작업 목록 (구현 안 함)

코드 근거 [확인]
- `Tools/LevelPlacementExtractor/extract_ue3_placements.py` `PLACEMENT_ACTOR_CLASSES` = `staticmeshactor`, `interpactor`, `staticmeshcollectionactor`, `efmotionstaticmeshactor` 네 종류의 StaticMeshComponent만 배치로 뽑는다. `DecalActor`, `EFTranslucentVolume`, `LandscapeComponent`는 이 스크립트에 없다. Landscape는 별도 `Tools/LandscapeExtractor`, foliage는 `extract_ue3_foliage_placements.py`가 처리한다.
- 위 네 종류의 개수는 원본과 정확히 일치하므로, 이 필터 자체로 흘리는 바닥은 없다.

작업 목록과 난이도·위험
1. **`NaviMesh.epf` 삼각형 해독** (난이도 중·높음). 정점은 해독됐으므로 남은 것은 삼각형 레코드 규칙(태그별 값 폭, 섹션 구분)이다. 성공하면 원본의 걷는 다각형 전체와 높이를 얻어 "어디에 바닥이 있어야 하는가"를 면적 단위로 확정한다. 서버 내비 검증 기준으로도 쓸 수 있다. 위험: 규칙 추정 실패 시간 소모.
2. **Landscape 재검증** (난이도 중). Landscape의 visibility(구멍) 레이어와 높이 스케일/오프셋을 원본 컴포넌트 속성과 대조해, 정확히 0.0m인 셀과 ±3m 일치 구역을 설명한다. 08-25에 숨긴 이유는 절벽 UV 늘어짐이며 삼면 투영 보정이 이미 코드에 있다. 위험: 절벽 UV 문제 재현.
3. **선택적 Landscape 표시** (난이도 중). 내비 높이와 ±3m로 일치하는 구역에 한해 Landscape를 보이게 하는 방식. 위험: 구멍·높이 해석이 틀리면 공중 바닥.
4. **남은 2/3의 소스 추적** (난이도 높음). 메시 지지도 지형 일치도 없는 내비 정점(전체의 약 20%)이 무엇 위에 놓이는지. 후보: 다른 존이 같은 레벨을 공유하는 경우(`11121`, `11627`, `11692`), 서버/스크립트가 만드는 지오메트리, 우리가 아직 해독하지 못한 컴포넌트. 사용자의 스크린샷 좌표(월드 위치)가 있으면 크게 좁혀진다.
5. 재추출 범위: 지금은 필요 없다. 위 결과가 새 소스를 지목하기 전에는 패키지를 다시 추출하지 않는다. Landscape를 쓰게 되면 `LV_BER_BernCastle_T_Land01/Land02`만 대상이다.

## 7. 스크린샷 대조

- 원본 `Screenshot_260929_185422.jpg`(룬 광장)를 열었다. 룬 원형 장식 4기둥 주위가 분홍빛 불규칙 다각형 포장으로 이음매 없이 깔려 있고, 좌상단에 철문과 정원 울타리, 화단이 보인다 [확인].
- 위치는 룬 광장(137, 42, -35~-57) 근처로 본다 [추정, 이전 문서와 같음]. 이 위치는 통로 위이며 메시 바닥 63%가 있다. 사진에서 보이는 큰 분홍 포장 면적과 우리 메시 바닥(통로 폭 약 10m)의 차이는 눈으로도 크다.
- 우리 프레임워크 자유 카메라 8장(`스크린샷 2026-09-29 1903xx~1904xx.png`)은 이번 조사 시점에 폴더에서 찾을 수 없어 열지 못했다 [미확인]. 폴더에는 09-30 05:21~ 스크린샷 5장이 있으나 콜로세움 PvP 화면이라 이번 조사와 무관하다.
- 원본 15장 중 열어 본 것은 1장이다. 나머지는 열지 않았다 [미확인].

## 8. 확인하지 못한 것

- 원본 바닥의 정체(2/3의 내비 정점과 동쪽 넓은 구역).
- `NaviMesh.epf` 삼각형 규칙, 섹션(`sectionID`) 의미.
- Landscape의 구멍 레이어, 정확히 0.0m인 셀의 의미, 높이 스케일 검증.
- 스크린샷의 월드 좌표(±30m 추정 이상 확정 불가).
- 6절 작업은 시작하지 않았다.

## 9. 참고 (이번 조사에서 쓴 것)

- 조사한 것: 베른 레벨 패키지 39개 중 PS·SL00~10·Land01/02·EnvNpc02·Music·SoundStream·Event01·Scene 16개·Standard_* 2개의 클래스별 export 개수, 스폰 광장 상자 안 전 액터, DecalActor 86개의 위치와 재질, EFTranslucentVolume 41개.
- 스크립트(재현용, 저장소 밖): `deploy_probe.py`, `deploy_parse.py`, `survey_levels.py`, `survey_scenes.py`, `scan_decals.py`, `scan_box.py`, `scan_translucent.py`, `raster_region.py`/`raster_region2.py`, `analyze_raster.py`, `raster_landscape.py`, `navmesh_parse.py`.
- 내 실수·한계: ① 처음 카탈로그 첫 행을 건너뛰는 버그로 메시 로드 실패 6종을 잘못 보고했다가 고쳤다(실제 실패 0). ② `NaviMesh.epf` 삼각형 파서는 어긋난 채 폐기했고, 정점만 근거로 썼다. ③ 래스터의 높이 밴드(38~47m)와 이름 분류는 근사다.
