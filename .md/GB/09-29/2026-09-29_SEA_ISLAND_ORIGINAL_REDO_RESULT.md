# 2026-09-29 바다 위 마하라카 섬 — 원본(EFDLProp_ISL_00072)으로 교체 RESULT

## 0. 결론 (증거로 (가)/(나) 판정)

**(가)에 해당한다.** 바다 위에 보이는 2026 마하라카 섬은 설치 클라이언트에 실재하는 **바다 지도의 Deploy 소품(Prop) 한 개**이고, 놀이 섬 맵(LV_OCN_EVENTIS_MHP)을 축소한 것이 아니다.
이전 fork A의 "섬 전용 모델은 없고 마하라카 레벨을 0.22배로 줄인 것"이라는 결론은 **틀렸다**(바다 레벨의 StaticMesh 배치만 뒤져서 Deploy 소품 경로를 놓쳤다).

| 확인 항목 | 사실 | 근거 |
|---|---|---|
| 설치 클라이언트 시점 | 패키지 최신 수정 2026-09-28(패치 718개), data/leveldata lpk 2026-09-28 13:40~42, game manifest local_version 991 | `Packages/*.upk` mtime 집계, `combinedata_manifest/GameManifest_45.upf` |
| 마하라카 시즌 존 | 57009(2021), 57017(2022), 57025(2023), 57037(2026 "마하라카 리턴즈 베이스 캠프")가 전부 바다 존 30703의 같은 좌표 (-36086, 50366)로 이어진다 | `EFTable_ZoneFallback` |
| 바다 지도 Deploy | 존 30703 `DeployData.loa`의 그 좌표에 `CEFDeployActor_Prop`이 있다 | `leveldata1.lpk`의 `MapData/30703/DeployData.loa`, 레코드 오프셋은 쿠크 규칙(+0x00 위치 cm, +0x10 yaw, +0x38 스케일 %, +0x64 PropDefinitionId)을 30703에서 재측정해 확인 |
| 현재 유효한 섬 소품 | Prop **1041048 → `EFDLProp_ISL_00072`**, 위치 UE (-36070, 50710), yaw 2496(13.7°), 스케일 100 | `EFTable_Prop`(data2.lpk) |
| 예전 해 소품 | 같은 자리에 1041019/1041021/1041027 (yaw -17°대)이 남아 있으나 **현재 EFTable_Prop에 없다** → 2021~2023 섬 소품은 더 이상 렌더되지 않는다. 그래서 2021 섬의 바다 모델은 설치본에서 복원할 수 없다. | 같은 DeployData vs Prop DB 대조 |
| 닻/입구 | Prop 1040158 → `EFDLProp_ITR_10066` 이 (-36565, 50542)에 있다. 섬 원점 기준 서쪽 4.95 m, UE y 방향 -1.68 m | 위와 동일 |
| 섬 모델 | LookInfo `EFDLProp_ISL_00072` → `StaticMesh'ISL_00072.Mesh.ISL_00072_SK'`(패키지 `GE10PYYYBCDRYTKKK3AH7K.upk`, 논리명 ISL_00072). 재질 5종(isl_00037_01~04, isl_00072) | `data4.lpk` LookInfo, umodel export |
| 사용자 스크린샷과 일치 | atlas 텍스처에 MAHARAKA 글자, 칵테일 표지, 무지개, 별 문양, 풀장 타일이 있고, 메시 렌더는 불규칙 모래 해안 + 뒤쪽 큰 침몰선 + 풀장·미끄럼틀 + 녹색 패드를 보인다. | `isl72_d.png`, `isl72_render.png`(작업 폴더) |
| 사용자 정정 | 기에나의 바다는 사용자가 항해 방식을 몰라 임의로 고른 바다이며 단서가 아니다(반영). 위 좌표 (-36086, 50366)이 바다 지도 안의 섬 위치다. | 사용자 메시지 |

이 소품이 어떻게 화면에 그려지는지는 "서버 소환"이 아니라 **바다 지도 Deploy에 상시 배치된 정적 소품**이다.
`EFTable_VoyageDynamicIslandSpawn`에는 마하라카 행이 없다(142행 전수 확인). 회전형/동적 섬과는 별개다.

이미지 #23은 이 fork에 전달되지 않았다. 사용자가 이후 보낸 원본 클라이언트 스크린샷
`Screenshot_260929_225222/225304/225307/225321/225323.jpg`를 열어 확인했다: 월드맵의 섬 툴팁이 "**마하라카 썸머 캠프**"(권장 아이템 레벨 250, 비프로스트 외부 이동만 가능)이고, 섬 남서쪽 나무 부두 앞 바다에 노란 닻 표식이 있으며 G 키 안내 "마하라카 썸머 캠프"가 그 닻 위에 뜬다. 큰 침몰선은 섬 북쪽(화면 위)에 있다.

## 1. 무엇을 어디까지 검색했나

- 설치 패키지 33,941개 헤더 전수 스캔(이름표·export 클래스 집계, 470 s): 마하라카 계열 이름(mhp/mhp22/mhp23/returns)이 든 레벨 패키지 277개를 골라 배치를 전부 추출.
- 바다 월드 레벨 136개(LV_OCN_World* 전부)의 StaticMesh 배치 129,366개와 InstancedStaticMesh 43,769개, 지정 좌표 반경 3,000 / 8,000 / 20,000 cm 안의 액터 전수. → 좌표 3,000 안 배치 0개, 8,000 안은 바위·절벽·컬링 상자뿐. **바다 레벨 배치에는 섬이 없다**(이전 결론의 절반만 옳았다).
- `LV_OCN_World_PS` 스트리밍 129개 이름 확인(전부 lv_ocn_world*). 마하라카 레벨은 스트리밍되지 않는다.
- EFTable 798개를 현재 data2.lpk에서 다시 풀어(기존 2026-09-26 추출본 대신) ZoneBase/ZoneFallback/Prop/VoyageDynamicIslandSpawn 대조.
- Deploy 소품 레코드를 30703에서 재측정해 위치·yaw·prop ID를 읽었다.

## 2. 교체한 것

- 이전 축소 섬(v2/v3, 지형 16타일 + 소품 3,789개, 부두·야자 문기둥)을 `install_bern_island.py --remove --apply`로 전부 제거(ISLAND00 shard, 배치 3,808행, 재질 841행, 조명 1,974행). 틈 메우기 물 평면은 유지.
- 새 섬 = ISL_00072 한 개 배치. `Tools/ShipPipeline/bern_isl72.py`(상수·좌표), `install_sea_island_isl72.py`(설치기), `build_sea_nav.py`(내비, 패치).

| 항목 | 값 |
|---|---|
| Asset ID | `MAP_0D832EC70895_ISL_00072_SK` |
| Resources | `Client/Bin/Resources/Map/LV_BER_BERNCASTLE/MAP_0D832EC70895_ISL_00072_SK/…wmodel` + `SourceMaterials/`의 텍스처 9장 |
| 배치 | 원점 Bern (440, 10.8, -480), yaw 13.7°(UE yaw 2496 → 기존 `convert_rotation`), 균일 스케일 2.0 |
| 스케일 근거 | 원본 스케일은 100 %(메시 11.6 × 12.7 m, 물 위 모래 껍질 8.5 × 6.7 m). 같은 원본 스크린샷에서 섬 폭이 배 길이의 3.2~4.0배이고 프로젝트 배가 4.43 m라, 껍질 폭 기준 ×1.9~3.2, 껍질 면적 기준 ×2.5 → **2.0**. 오차 약 ±25 %, `ISLAND_SCALE` 한 상수다. |
| 재질 | 4슬롯: `isl_00037_01`(모래 원반, 불투명), `_02`(마스크), `_03`(불투명), `isl_00072`(PBR atlas: D/N/ORM) |
| 입항 트리거 | `island.dock.to.maharaka` 위치 (430.1, 10.95, -476.64) = 원본 닻 소품 위치, halfExtents [8,4,8], yaw 111.9° |
| 닻 표식 | 트리거 중심에 그려지므로 원본 닻 위치로 이동(`Level_Bern.cpp` 변경 없음) |
| 배 정박점 | (427.75, 10.95, -475.25) — 육지에서 6.19 m(안전 여유 6 m), 닻에서 2.7 m |
| 바다 내비 | BernSea 865,329칸(이전 863,383). 육지 금지 영역은 ISL_00072의 물 위 삼각형을 배치 변환해 래스터화 |
| 복귀 | 항구 복귀 스폰 (299.75, 10.95, -235.25) 변경 없음 |
| 공유 파일 | `C:\Users\USER\.claude\jobs\46aea322\tmp\island_dock.json` `FINAL_V3_ISL72`로 갱신 |

## 3. 하얗게 날아가는 문제 — 실측 결과

- 이전 원인: 마하라카 지형 타일의 baked 알베도(선형 평균 0.65)가 베른 씬 조명(diffuse 2.4, ambient 0.72; 램버트 배율 약 2.3)에서 1을 넘어 포화.
- 새 모델의 모래 원반 텍스처 `isl_00037_01_d` 선형 평균 알베도는 0.134/0.148/0.153이다. 같은 램버트 추정으로 약 0.31~0.35이고, 원본 스크린샷의 모래 실측 선형 0.27~0.46 범위 안이다. 즉 **새 모델은 베른 조명에서 포화하지 않는다고 계산된다**.
- 그래서 이전에 걸었던 0.36 tint는 **새 모델에 적용하지 않았다**(카탈로그 tint = 1, 재질 diffuseBrightness = 1). 렌더링 옵션(`Data/Rendering/**`)은 건드리지 않았다.
- 이 값은 계산이며 화면 확인이 아니다. 색이 다르면 첫 후보는 슬롯 재질의 `diffuseBrightness`(원본 파라미터가 아닌 조정)다.

## 4. 근사·미복원 (숨김 없이)

- 물 재질 슬롯 `isl_00037_04_mi`(76 삼각형, 원본 `preset_water_dungeonriver`)는 재질 family를 새로 만들지 않기 위해 **빼고 cook했다**. 풀장 물은 atlas의 파란 타일이 그리고 별도 반사 수면은 없다.
- 슬롯 `_02`(마스크)는 `bg_simple_msk`에 대응하는 플래그(64)를 준 근사이고, `isl_00072` PBR 재질의 normalIntensity/saturation은 마스터 기본값을 알 수 없어 1.0으로 뒀다(ISL_00072 MIC는 스칼라를 덮어쓰지 않는다).
- geometry contract cook(WModel 1.1 이상, tangent handedness 보존)은 provenance 영수증 체인이 없어 생략하고 변환기 기본 산출물(WINT 1.0)을 썼다. 미러 UV 섬에서 법선 밝기가 뒤집힐 수 있다.
- DDS는 umodel 출력 그대로다(밉 단일, DXT1/DXT5/ATI2). 축소 시 반짝임 가능성이 있다.
- 부두·MAHARAKA 아치·침몰선은 메시에 포함돼 있다고 atlas와 렌더로 판단했으나 모양은 화면에서 확인하지 않았다(에이전트는 Client를 실행하지 않는다).
- 2021 버전 섬의 바다 모델은 예전 소품이 DB에서 제거돼 설치본에서 복원 불가. 화면 기준은 2026 섬이다.

## 5. 실행한 명령과 검증

- `install_bern_island.py --remove --apply` → `install_sea_island_isl72.py --apply` → `build_sea_nav.py --write`
- 게시(한 번씩): `Publish-ServerNavigation.ps1 -Mode Publish`(약 40 s, BernSea walkable 865,329), `Publish-WorldGameplay.ps1 -Mode Publish -WorldId BERN`(약 20 s), 그 뒤 `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Publish -Scope Area`(로그 `tmp/sea_island/isl72_publish.log`; 결과는 아래 6절).
- 설치 전후 점검: 카탈로그 자산 대비 배치 자산 누락 0, 재질 rows 텍스처 경로 실재 확인, JSON parse, 배치 좌표·쿼터니언 유한성.

## 6. 게시 결과

- `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Publish -Scope Area` — 23:07:28 시작, 23:34:49 종료(약 27분), EXIT=0. 별도 Validate 선행 없이 한 번만 실행했다.
- 결과: 배치 50,019행(Bern 50,018 + 섬 1), shard 24, 파일 53개, `LV_BER_BERNCASTLE.mapset` SHA256 `4268e5b4…7cc`.
- 게시본 확인(직접 대조): `LV_BER_BERNCASTLE_ISLAND00.mapassets`(자산 1개, 재질 파일 참조 v5), `…ISLAND00.mapplacements`(섬 1행, 스케일 2.0, yaw 쿼터니언 0.119/0.993), 게시본 재질 23,157행 = 저작본 23,157행, 섬 재질 4행, 참조 텍스처 누락 0, 스테이징/롤백 잔재 없음.
- 서버/클라이언트 내비 `LV_BER_BERNCASTLE.BernSea.navgrid/.navblockers` 갱신(Server·Client 둘 다), `BERN.worldbootstrap`·`LV_BER_BERNCASTLE.viewer.world.json` 갱신.
- Check 모드는 돌리지 않았다(과거 26분 소요). 실제 화면·로딩은 사용자가 확인한다.

## 7. 사용자 확인 필요

- Server + Client를 프로토콜 121로 함께 빌드·재시작한 뒤 베른 배 탑승, 항해로 섬 접근: 섬 모양·크기(스케일 2.0이 맞는지), 밝기, 부두·아치·침몰선, 닻 표식 위치, G 입항.
- 색이 이상하면 결과 문서 3절의 절차로 진단(렌더링 옵션 파일은 사용자/팀장 소유).

## 8. Resources 인계

- 물리 Resources는 Git 비추적이며 사용자가 `CY_Resources`를 직접 Drive/홈페이지로 보낸다.
- 새 리소스는 `Client/Bin/Resources/Map/LV_BER_BERNCASTLE/…`에 설치했고, `C:\Users\USER\OneDrive\바탕 화면\CY_Resources\Map\LV_BER_BERNCASTLE\{MAP_0D832EC70895_ISL_00072_SK, SourceMaterials}`에 같은 상대 경로로 복사본만 넣었다(wmodel 1개 + 텍스처 9장, 약 8 MB).
- 복사 스크립트 `Copy_ResourceDistribution_2026-09-29_SeaIslandISL72.ps1`(바탕 화면)은 만들었으나 **불필요하다. 사용자 판단으로 삭제해도 된다**(삭제하지 않았다).
- 중간 산출물: `C:\LostArkExtract\SeaIslandISL72_20260929`(cook), `C:\Users\USER\.claude\jobs\46aea322\tmp\sea_island\`(스캔·추출·렌더·백업).

## 9. 다음 명령

1. VS를 닫고 Debug/x64 솔루션 빌드(코드 변경 없는 이 작업만으로는 빌드가 필요 없으나, 프로토콜 121 Server/Client를 함께 다시 시작해야 한다).
2. Server + Client 실행 → 베른 배 탑승 → 북동쪽 바다의 섬으로 항해 → 닻 표식에서 G.
