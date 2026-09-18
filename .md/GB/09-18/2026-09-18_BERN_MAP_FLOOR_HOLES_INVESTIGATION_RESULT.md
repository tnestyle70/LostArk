# 2026-09-18 베른성 상공 바닥 검은 구멍 — 원인 측정과 복원안 RESULT

사용자 관찰: 베른성을 F6 자유 카메라로 위에서 보면 건물·테라스·길 사이가 순수 검정으로 뚫려 있다(스크린샷 4장).
요청: 원인을 찾고, 잘 나온 광장 바닥 같은 재질로 맵 어디에도 뚫린 곳이 없게 메운다. 원본에 있던 것은 원본대로 복원하고, 원본에도 비어 있는 곳만 광장 바닥으로 채운다.

## 원인

### 측정 방법

- 게시본 `Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_*` 23샤드 50,017배치의 WModel 삼각형을 런타임과 같은 변환(`local*0.01*signedScale*R(quat)+position`, `Loader.cpp:973`, `MapPlacementRuntime.cpp:969`)으로 월드에 옮겨, 위에서 본 1m 칸 커버리지를 만들었다(`out/BernMapHoles20260918/coverage2.py`).
- 하늘 구 `SKY_MIRROR_SM`(7.7km)은 카메라 위에서 모든 칸을 덮으므로 제외했다. 첫 측정은 이것 때문에 100% 덮임으로 잘못 나왔다.
- 원본 가시성은 원본 패키지에서 `Tools/LevelPlacementExtractor/extract_ue3_placements.py --schema-version 3`으로 SL00~SL10 11개 레벨을 다시 추출해 인스턴스·아키타입·CDO의 `bHidden`/`HiddenGame` 체인으로 판정했다(`out/BernMapHoles20260918/src_v3*/`, 30,800배치, 속성 오류 0, 미해석 0).

### 결과 (네비 영역 x 15~300, z −245~35, 79,800칸)

| 항목 | 칸 |
|---|---:|
| 보이는 바닥 | 39,450 (49.4%) |
| 구멍 | 39,974 (50.1%) |
| └ 숨긴 지형(LANDSCAPE) 아래 | 20,743 |
| └ 숨긴 물 평면 아래(지형 없음) | 19,193 |
| └ 그 밖의 숨김 아래 | 32 |
| └ 원본에도 아무것도 없음 | 6 |

구멍의 99.9%는 원본에 보이는 형상이 있는데 우리 데이터에서 `visible=0`인 곳이다. 모델 파일 누락은 0건(1,290개 wmodel 전부 존재), 컬링은 경계 구 중심·반지름이 음수 스케일에도 보수적이라(`MapPlacementRuntime.cpp:1001-1008`) 원인이 아니다.

### 숨김이 들어간 경위 (git·원본 판정)

1. **`LV_MODULE_*` 272배치 — 원본은 보임, 우리는 처음부터 숨김**
   - 바다 `LV_MODULE_WATER02_512` 3개(y≈10.8, 수 km, 재질 `source.map.water-43`)
   - 마을 수면 `LV_MODULE_WATER01_1024` 8개(y 34~52, 재질 `source.map.water-41`)
   - 검정·그림자 평면 `LV_MODULE_MESH02_512_512` 261개(북쪽 z 76~403, 재질 `source.map.black`/`shadow-modulate`)
   - 원본 판정: SL00~SL10 전부 `visible=True`. 우리 저작본은 08-04(`69191f83`)부터 `visible=0`이고 `visibilityOverrides`에도 없다. 당시 1판 추출은 가시성 기록이 없는 형식이었다. 저장소 규칙("LV_MODULE, water 이름만으로 숨기지 않는다")에 어긋나는 상태다.
   - 물 재질은 09-11에 런타임 연결됨(`SourceMapWaterMaterialParameters.h:106, 195`), 검정·그림자는 텍스처 없는 보조 재질로 처리됨(`MapAssetCatalog.cpp:637`).
2. **지형 42배치 — 원본은 보임, 우리가 일부러 숨김**
   - 08-04 Imported·저작본은 `visible=1`. 08-05 사용자 화면에서 절벽 초록 세로 늘어짐이 FAIL(`.md/GB/08-04/..._LANDSCAPE_VISUAL_RECOVERY_RESULT.md` 6절) → 08-25 `d3b72bdc`(KCY)에서 저작본 숨김 → 09-11 `359412c4`(tnestyle70)에서 Imported도 숨김.
   - 현재 설치본(09-03 조리)을 측정했다. 절벽(normal.y<0.3) 삼각형의 UV 면적/월드 면적 중앙값이 평지의 **7.9%** — 절벽 텍스처가 면적 기준 약 12배 늘어난다. **결함이 아직 남아 있다.** 지형 면적의 64%(109,104m²)가 경사면이다.
3. **의도적 숨김 20배치** — 08-25 회색 가림막 수정의 `visibilityOverrides`(해안 특수 메시 14, 원판·바닥·장식 6). 재질 파라미터가 없어 회색으로 나오는 결함 때문이므로 유지한다.

## 이번 수정

**저장소 파일은 바꾸지 않았다.** 게시 스크립트 수정이 자동 모드 분류기에서 `Modify Shared Resources`로 거부됐다. 그래서 같은 작업을 `out/BernMapHoles20260918/sandbox/`의 격리 프로젝트 사본(`-ProjectRoot`)에서 끝까지 실행·검증했다.

### 준비된 변경 1 — 게시 범위 `-Scope Placements`

`Tools/MapPipeline/Publish-MapAuthoring.ps1`에 visual 배치만 게시하는 범위를 더한다. 패치 스크립트:
`C:\Users\USER\.claude\jobs\45c8ba77\tmp\patch_publish_placements_scope.py <ps1 경로>`
(BOM·CRLF 보존, 앵커 정확히 1회 일치 검사, 쓰기 직전 해시 재확인, 임시 파일→교체)

- `ValidateSet`에 `'Placements'` 추가
- 효과·재질·물 선언 검사 블록 조건을 `Area`·`Placements`로 확장 — **재질 선언 플래그가 켜져야 카탈로그 머리글이 v5(재질 문서 참조)로 유지된다.** 이 줄이 없던 첫 격리 시험은 23개 카탈로그를 v4로 낮춰 썼다(실제 게시였다면 재질 연결이 끊겼다).
- 월드시퀀스 배치 참조 검사가 준비된 파일을 읽도록 조건 2곳 확장, `Assert-PlacementScopeSequenceTargets` 추가(월드시퀀스 파일 자체는 쓰지 않음)
- 샤드·단일 두 경로에서 `Placements`면 조명·이펙트·물·재질·월드시퀀스·카메라샷·deploy 추가 없이 게시하고 반환
- 출력: 샤드 카탈로그 23 + 샤드 배치 23 + mapset = 47개. `maplights/mapmaterials/mapeffects/mapwater/camerashots/worldsequences/deploy`는 쓰지 않는다.

### 준비된 변경 2 — 저작본 272배치 `visible 0→1`

`C:\Users\USER\.claude\jobs\45c8ba77\tmp\unhide_bern_lv_module.py <프로젝트 루트> [--dry-run]`
- 대상: 원본 `visible=True`인데 우리가 숨긴 292개 중 `visibilityOverrides` 20개를 뺀 272개(`out/BernMapHoles20260918/hidden_but_source_visible.json`). 행의 마지막 visible 토큰만 바꾸고, 대상이 현재 숨김이 아니거나 asset이 `LV_MODULE_`가 아니면 거부한다. 지형 42개는 대상이 아니다.

## 자동 검증 (격리 사본, 명령·exit)

| 단계 | 결과 |
|---|---|
| 패치된 스크립트 PowerShell 파서 | parse errors 0 |
| 수정 없는 저작본 `-Mode Check -Scope Placements` | EXIT 0, 47파일이 현재 게시본과 바이트 동일 |
| 272 visible 적용 | targets 272, changed 272, overrides 유지 20, 줄끝 LF 보존, 크기 불변 |
| `-Mode Validate -Scope Placements` | EXIT 0, PlacementCount 50,017, FileCount 47 |
| `-Mode Publish -Scope Placements`(격리 출력) | EXIT 0 |
| 격리 게시본 vs 실제 게시본 | 바뀐 파일 12개(배치만), visible 0→1 272줄, 그 밖의 변경 0. 조명·재질·이펙트·물 파일 바이트 동일 |
| 최신(17:04) 실제 스크립트 사본에 패치 재적용 | 같은 해시 `4c78cdcf3e08`, parse errors 0 |

### 구멍 전후 커버리지

| 영역 | 수정 전 구멍 | 수정 후 구멍 |
|---|---:|---:|
| 네비 영역 x 15~300, z −245~35 | 39,974 (50.1%) | **5,650 (7.1%)** |
| 남쪽 마을 밀집 x 10~300, z −240~30 | 38,530 (49.2%) | 5,185 (6.6%) |
| 북쪽 덩어리 x 0~300, z 90~400 | 79,416 (85.4%) | 586 (0.6%) |

수정 후 남는 네비 영역 구멍 5,650칸: 숨긴 지형 아래 5,612, 의도적 숨김 아래 32, 원본에도 없음 6. 가장 큰 덩어리는 스폰 북쪽 x 103~198, z −28~35(2,814칸, 전부 지형 자리). 바다 평면은 섬 부분을 비워 두므로 이 칸은 지형 말고 채울 원본 형상이 없다.
지도: `out/BernMapHoles20260918/holes_coverage2.npz.png`(전), `holes_coverage_after.npz.png`(후). 회색=보이는 바닥, 주황=숨긴 지형 아래 구멍, 파랑=숨긴 물 아래 구멍, 자홍=그 밖의 숨김, 검정=없음.

### 성능

현재 보이는 배치 48,167개·삼각형 20,166,546개에 272배치·삼각형 16,882개(+0.08%) 추가. 바다 평면은 화면을 넓게 덮는 물 셰이더라 픽셀 비용은 측정하지 않았다. (참고: 지형 42개를 풀면 삼각형 321,970개, +1.6%.)

## 광장 바닥 채우기(B)

원본에도 형상이 없는 칸은 네비 영역 6칸, 북쪽 586칸뿐이라 광장 바닥 배치를 추가하지 않았다. 남는 5,612칸은 원본에 지형이 있는 자리라 B 대상이 아니다. 광장 바닥 후보 asset(스크린샷 5번 위치를 특정하지 못해 미확정): `BG_BER_BERNCASTLE_FLOOR06_SM_KSR`(18배치, 6,070m², 1,904삼각형), `BG_EUD_MORAY_FLOOR01_SM_YSI`, `BG_LUT_LUCASTLE_HOUSEINFLOOR01_SM_YSI`(12삼각형). 스폰 지점 바로 아래에는 포장 바닥이 없고 납작하게 눌린 나무 메시(`BG_ATM_TREE_BROADLEAFTREE01B_SM_KYO`, scale y 0.3)와 숨긴 지형만 있다.

## 네비 영향

게시된 Bern 네비(13:44)는 데이터가 따로라 이 변경으로 바뀌지 않는다. 나중에 Map Tool에서 재bake하면 마을 수면 8개(y 34.4~51.6)가 bake 높이 범위(33~56) 안이라 **물 위가 걸을 수 있는 바닥으로 잡힌다**. 재bake 시 Block으로 칠할 곳: (168~196, −140~−111) y47.0 / (70~81, −34~−23) y34.4 / (84~92, −161~−152) y51.6 / (83~95, −148~−130) y48.3 / (190~238, −56~−8) y38.2 / (130~144, −147~−133) y49.9 두 장. 바다(y≈10.8)와 북쪽 검정 평면(y 7~27)은 범위 밖이다.

## 사용자 확인

실제 반영(저장소 루트, Server·Client 종료 상태):
```powershell
python C:\Users\USER\.claude\jobs\45c8ba77\tmp\patch_publish_placements_scope.py Tools/MapPipeline/Publish-MapAuthoring.ps1
python C:\Users\USER\.claude\jobs\45c8ba77\tmp\unhide_bern_lv_module.py . --dry-run
python C:\Users\USER\.claude\jobs\45c8ba77\tmp\unhide_bern_lv_module.py .
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Validate -Scope Placements
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Publish -Scope Placements
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Check -Scope Placements
```
- 빌드 불필요(데이터만). Client 재실행 → 베른 → F6 자유 카메라.
- 기대: 마을 사이 검은 구멍 대부분이 바다(y≈10.8)나 연못 물로 보인다. 북쪽은 원본 검정 평면이라 여전히 검게 보인다(원본과 같음).
- 스폰 북쪽(x 103~198, z −28~35) 등 지형 자리 5,612칸은 여전히 검정이다.

## 남은 것 (사용자 결정 필요)

1. **지형 42개**: (a) 그대로 풀기 — 구멍은 사라지나 절벽 텍스처 늘어짐(UV 밀도 7.9%)이 지형 면적 64%에 다시 보인다. 08-05에 사용자가 FAIL 판정한 결함. (b) 지형 재조리 — 절벽 삼각형을 별도 재질(원본 `layercliff`)과 측면 투영 UV로 분리해야 하며, 재질 슬롯이 늘어 `mapmaterials.json` 게시가 필요하다. `mapmaterials`는 지금 `Area` 범위(조명 동반)로만 게시되므로 재질만 게시하는 범위도 따로 있어야 한다. (c) 숨김 유지.
2. **베른 맵 담당**: Bern 저작·Imported·게시본 커밋 작성자는 KCY 6건, tnestyle70 4건이며 최신 대규모 재생성(09-11 `359412c4`)은 tnestyle70이다. 저작 배치와 게시 배치는 LFS라 병합이 불가능하다. 반영 전 팀장과 담당을 맞춰야 한다.
3. 우리 쪽 `visible=1`인데 원본은 숨김인 배치 1개(`bg_eud_tree_u06_sm_old`)가 있다. 구멍과 무관해 손대지 않았다.
4. 적용 스크립트 두 개가 세션 임시 폴더에 있어 잡을 지우면 사라진다(`out/`로 복사하는 명령도 분류기에서 거부됐다).

## 내 실수

1. 첫 커버리지에서 하늘 구를 빼지 않아 전 구역 100% 덮임이라는 틀린 결과를 냈다.
2. 측정 스크립트 재사용 시 잘라낼 위치를 잘못 잡아 `quat_matrix` 미정의로 한 번 실패했고, `sed`로 `\n`을 넣다 문자열을 깨뜨렸다.
3. 차이 비교 스크립트에서 bytes/str을 섞어 한 번 실패했다.
4. 첫 `Placements` 설계가 재질 선언 플래그를 놓쳐 카탈로그를 v4로 낮춰 쓰는 결함이 있었다. 격리 사본 비교로 잡았고 실제 게시는 하지 않았다.
