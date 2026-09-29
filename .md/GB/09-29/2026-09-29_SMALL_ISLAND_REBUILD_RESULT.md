# 소형 마하라카 섬 재제작 RESULT (2026-09-29)

## 요청
바다 위 마하라카 섬을 원작처럼 작게 다시 만든다. 기준 화면은 `스크린샷 2026-09-28 141921.png`(가까운 시점)이고,
풀장·미끄럼틀·침몰선 소품과 입구(부두)를 함께 보여야 한다. 안개는 원인이 아니므로 렌더링 자체를 바로잡는다.

## 원작 조사 결과 (사실)
- 바다 지도(`LV_OCN_World*` 하위 레벨 136개)를 전부 추출해 마하라카 섬 좌표 (-36086, 50366) 주변 ±150 m를 검색했다.
  근처에는 바위·절벽·`bg_ocn_stone_rock*`·배 몇 척뿐이며 섬 전용 모델은 없다. `voisland` 이름의 모델은 다른 섬의 나무 랜드마크다.
- `LV_OCN_Eventis_MHP22` 패키지는 메시·재질만 든 에셋 패키지이고, 배치 액터는 `...85J/85Q/85C`에 있으며 모두 놀이 섬 본체(풀장·파라솔·서프보드)다.
- 결론: 원작의 멀리서 보이는 작은 섬은 별도 모델이 아니라 마하라카 레벨 자체를 축소해서 본 것이다. 그래서 이미 설치된 마하라카
  지형 16타일과 소품을 균일 축소하는 방식으로 만들었다 (추출 신규 작업 없음, Resources 추가 복사 없음).

## 만든 방식
- 균일 축척 `SCALE = 0.22` (섬 전체 약 24 × 22 m, 원작 16 × 21 m 근처). 지형 타일만 세로로 `3.5`배 더 늘려
  타일 아래 판이 바다 아래에 숨도록 했다(`TERRAIN_STRETCH`). 소품은 늘어난 지면 높이를 따라 놓는다.
- 소품 267개: 풀장(POOL*)·미끄럼틀(DECOSLIDE 3종)·침몰선(PEYTO_SHIP*, SHIPFLOOR, LUT/ETC_SHIP*)·파라솔·서프보드·야자수·FLOORDECO.
  풀장 배관(REDSANDDST_PIPE) 수백 개와 물 모듈은 제외했다. 풀장 물 표면은 포함하지 않았다(빈 풀로 보일 수 있음).
- 입구: 섬 서쪽(항구 쪽)에 침몰선 갑판을 부두로 재사용(9 m × 4.6 m, 갑판 y 11.4)하고 부두 끝에 야자수 2그루를 문기둥처럼 세웠다.
  회전이 필요 없는 방향(서쪽)을 골라 쿼터니언 방향 오류 위험을 피했다.
- 재질: 소품 99종의 MHP 재질과 lightmap 정보 `placementLighting` 190행을 Bern 재질 정본에 이어 붙였다(기존 항목은 그대로, 뒤에 추가만).
  `BG_LUT_COMMON_SHIP04_SM_KBW`는 Bern에 이미 있는 ID라서 신규 카탈로그에서 뺐다(중복 ID 오류 방지).

## 바뀐 파일
- `Tools/ShipPipeline/bern_island_layout.py` (전면 재작성: 축척, 소품 선택, 지면 추종 높이)
- `Tools/ShipPipeline/install_bern_island.py` (전면 재작성: 소품·재질·조명 설치, `--remove` 되돌리기, `--dock-json` 출력)
- `Tools/ShipPipeline/build_sea_nav.py` (지형 축척 삼중값 사용, 부두 정박점 목표, 정박 거리 범위 9~14 m)
- `Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_ISLAND00.mapassets|.mapplacements`, `LV_BER_BERNCASTLE.mapset` (샤드 행 1개)
- `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements` (286행 추가, 총 50,304), `LV_BER_BERNCASTLE.mapmaterials.json` (추가만)
- `Data/Navigation/LV_BER_BERNCASTLE.BernSea.navsource` 및 게시본 navgrid/navblockers/navsurface
- 게시본: `Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE*` (Area 스코프)

## 입구 좌표 (부두 담당 fork B용)
`island_dock.json` (FINAL_V2, 잡 tmp 폴더): 섬 중심 (440, 10.8, -480), 부두 끝 x 420.25, 부두 동쪽 끝 x 429.28,
배 정박점/트리거 (417.25, 10.95, -480.25) yaw 89.2, 트리거 halfExtents [7, 4, 7], 항구 복귀 (299.75, 10.95, -235.25).

## 검증 (실행한 것만)
- `install_bern_island.py` dry run/apply, 되돌리기(`--remove`) 왕복. 재질 JSON은 HEAD와 비교해 뒤에 추가만 된 것을 확인.
- `Publish-ServerNavigation.ps1` Validate → Publish 성공 (BernSea 920×988, walkable 863,383).
- `Publish-MapAuthoring.ps1 -Scope Placements` Validate/Publish/Check 성공(재질 추가 전 상태).
- `Publish-MapAuthoring.ps1 -Scope Area`: 아래 "게시 상태" 참고.

## 사용자 확인 필요
- 섬 모양·크기·소품 배치·부두 방향은 화면 판정이 필요하다. 에이전트가 화면을 확인하지 않았다.
- 재질/lightmap은 게시 검증만 통과했다는 뜻이며 실제 밝기·색은 육안 확인이 필요하다.
- Server + Client를 같은 빌드로 재시작해야 한다 (Client는 빌드, Server는 navgrid 재시작).
