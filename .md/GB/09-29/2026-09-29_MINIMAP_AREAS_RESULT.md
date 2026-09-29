# 2026-09-29 미니맵 4개 지역(항구·마하라카·도서관·성 내부) 결과

## 구조 요약
1. 미니맵은 `CUI_Sprite` 하나("Minimap_Map")가 지역 이미지의 UV 창을 스크롤/줌하는 방식이다.
2. 지역 정의 정본은 `Data/UI/Minimap/MinimapAreas.json`(schema `lostark.minimap-areas` v1)이며 `level` 키로 BERN/VALTAN_ARENA/KAKULSAYDON_ARENA/MAHARAKA를 구분한다.
3. 좌표는 retail cm이다: x = clientX*100, y = -clientZ*100. 이미지 오른쪽 = +y, 위 = +x. 스냅샷은 XZ만 싣는다.
4. `CMinimapView::Update`가 로컬 플레이어 위치로 `Find_Area(level, x, z)`를 호출해 지역을 고른다. 이전에는 레벨의 첫 항목만 썼다.
5. 선택 순서는 JSON 항목 순서(우선순위)다. 실내·항구를 실외 기본 앞에 둔다.
6. 신규 필드: `default: true`(어느 선택 상자에도 안 들면 쓰는 지역), `selectMinCm/selectMaxCm`(선택 상자. 없으면 기존처럼 이미지 상자 `worldMinCm/worldMaxCm`).
7. M키 월드맵(`CWorldMapWindowView`)도 같은 규칙으로 지역을 고르며 라벨·패널 등 다른 데이터는 그대로다.
8. `CMainApp::Update_Minimap`에 MAHARAKA 분기를 추가했다. 마하라카는 `CLevel_Development` 셸이므로 `Get_Active(LEVEL::MAHARAKA)`에서 마커를 모은다.
9. 이미지는 원작 텍스처(UModel로 EFMinimap_* 패키지 export)만 사용했다.
10. 지역을 더 늘리거나 바다를 키우는 일은 JSON 항목 추가만으로 가능하다.

## 지역별 원인과 수정
- **성 내부/도서관**: 원인은 Bern 레벨이 야외 vol0 이미지 하나만 참조한 것. 원작 zone 11102는 야외 1 + 실내 10 볼륨이다(클라 Z 112~376m). 수정: `BernCastleIndoor_{2,3,5,6,7,8,9,11,12,14}.png` 10개를 실내 항목으로 추가. 도서관은 vol7("베른 성 (도서관)", library.2가 이 볼륨에 속함이 트리거로 증명), 성 내부는 vol9(castle.2). 나머지 볼륨은 이름을 "베른 성 (내부)"로 중립 표기.
- **항구**: 원작에서 크로나 항구는 별도 zone 11111(vol1, 6x6 타일)이라 Bern 야외 이미지에 없다. 수정: `BernHarbor.png` + 선택 상자 `selectMin [20500,16000] / selectMax [29500,26000]`(항구 배치 4개만 포함함을 확인). 이미지 매핑은 `worldMin [512,-256] / worldMax [31232,30464]`.
- **마하라카**: 원인은 지역 항목, 레벨 파서, MainApp 분기가 모두 없던 것. 수정: `Maharaka_0.png`(zone 57009 vol0, 768x768, worldMin [3072,94022] max [13312,104262]) 항목, `Parse_Level` "MAHARAKA", `MainApp.cpp Update_Minimap` MAHARAKA 분기, `Level_Development.h Collect_MinimapMarkers` 추가.
- 코드: `Client/Private/MinimapView.cpp`(Find_Area, default, select box), `Client/Public/MinimapView.h`, `Client/Private/WorldMapWindowView.cpp`, `Client/Public/WorldMapWindowView.h`, `Client/Public/Level_Development.h`, `Client/Private/MainApp.cpp`.

## 신규 에셋 (12개)
`Client/Bin/Resources/UI/Minimap/Maps/` 아래 `Maharaka_0.png`, `BernCastleIndoor_{2,3,5,6,7,8,9,11,12,14}.png`, `BernHarbor.png`.
`C:\Users\USER\OneDrive\바탕 화면\CY_Resources\UI\Minimap\Maps\`에 같은 상대 경로로 복사 완료(12개 확인).

## 실행한 검증
- `cl /Zs` 구문 검사: MinimapView.cpp, WorldMapWindowView.cpp, Level_Development.cpp, MainApp.cpp 모두 cl_exit=0(오류 0).
- `git diff --check`: 공백 오류 없음(autocrlf 안내만). 대상 파일 줄바꿈은 기존 CRLF 유지, JSON parse OK(15개 지역).
- 시뮬레이션(`simulate_minimap.py`, 코드 선택 규칙을 그대로 재현): Bern library.2/castle.2/항구 NPC/plaza/ship, Maharaka 경기장·점프 지점 모두 이미지 안. 빌드·Client 실행·화면 판정은 하지 않았다.

## 게임에서 확인할 것
1. 베른: 항구(배 타는 곳)로 가면 이름이 "크로나 항구"이고 항구 지도가 나오며 내 표시가 제자리에 있어야 한다.
2. 도서관 안: "베른 성 (도서관)" 지도. 성 내부(castle.2 부근): "베른 성 (성 내부)". 야외로 나오면 "베른 성" 야외.
3. 마하라카 입장 후: "마하라카 파라다이스" 섬 지도에 내 위치와 마커가 나오는지, M키 월드맵도 같은지.

## 미해결
- `island.dock.to.maharaka`(414,-424)는 fork 1이 넓힌 바다 쪽이라 원작 이미지 밖이다. 바다 범위가 확정되면 새 이미지와 항목(`selectMinCm/selectMaxCm` 포함)을 데이터로 추가해야 한다.
- `npc.bern.ship.shipwright.2`(22961,19650)는 두 원작 이미지 모두 빈 공간이다.
- 실내 볼륨 7·9 외에는 어느 방인지 근거가 부족해 이름이 중립이다.
- M키 월드맵의 라벨/패널이 새 지역에서 올바른지는 지역 선택 외에는 확인하지 못했다.
- 빌드 전이므로 실제 화면 확인은 사용자가 한다. 커밋은 하지 않았다.

## SEA — 확장된 베른 앞바다 미니맵 (fork 5)

fork 1이 넓힌 배 바다(BernSea 창 x 140~600, z -650~-156, 460x494 m)에서 미니맵과 M키 월드맵이 이미지를 못 잡던 문제를 데이터로 해결했다. 코드는 바꾸지 않았다(fork 4의 `selectMinCm/selectMaxCm`과 `default` 규칙 그대로).

### 원인
- 바다 창의 대부분이 항구 선택 상자(client x 205~295, z -260~-160) 밖이고 성 야외 이미지 범위(client x 14.7~270.7, z -220.5~35.5)와도 거의 겹치지 않는다. 이전에는 기본 지도 `베른 성`이 잡혀 UV가 이미지 밖(u<0 또는 v<0)으로 나가 내 표시 밑에 지도가 없었다.
- 접안 지점 (414.25, -423.75)과 섬 중심 (439.78, -479.87)은 어느 기존 이미지의 범위도 아니었다.

### 이미지 출처
- 원작 바다 미니맵은 쓰지 않았다. 원작 미니맵 패키지(`EFMinimap_BER`, `EFMinimap_OCN`)는 존별 마을/섬 이미지이고, 이 프로젝트의 작은 바다는 원작 대항해 좌표와 대응하지 않아 그대로 맞지 않는다. 원본 테이블 전체 스캔으로 대항해용 미니맵 이름을 찾는 조사는 시간이 길어 중단했다(원본이 있어도 좌표 불일치라 채택하지 않음).
- 우리 데이터로 위에서 내려다본 이미지를 생성했다: `Tools/ShipPipeline/build_sea_minimap.py`(신규). 재료는 fork 1과 같은 단일 정본인 `bern_island_layout.py`의 마하라카 Landscape 16타일(수면 위 정점 → 섬 실루엣, 중앙 웅덩이는 옅은 물색), 베른 Landscape 타일(서쪽 항구 해안), 바다 창 사각형.
- 결과: `Client/Bin/Resources/UI/Minimap/Maps/BernSea.png` 1335x1250, 2.5 px/m(항구 지도와 같은 관례), 30,381 bytes. `CY_Resources\UI\Minimap\Maps\BernSea.png`에 같은 해시로 복사했다.
- 원작 그림이 아니라 생성 그림이다. 섬 웅덩이는 실제 지형(수면 0.19 m 아래 평지)이며 원작 수영장 위치와 대응한다.

### JSON 항목 (`Data/UI/Minimap/MinimapAreas.json`, `크로나 항구` 뒤 · `베른 성`(default) 앞)
- `areaName` "베른 앞바다" (원본 존 이름 데이터에 이 바다의 이름이 없어 중립 이름. 근거 없음)
- `image` `UI/Minimap/Maps/BernSea.png`, `imageSize` [1335, 1250]
- `worldMinCm` [12000, 13600], `worldMaxCm` [62000, 67000] (창 + 20 m 여백, 스크립트가 출력한 값)
- `selectMinCm` [14000, 15600], `selectMaxCm` [60000, 65000] (창 그대로)
- 바다를 키우면 이 여섯 값과 이미지를 `build_sea_minimap.py`의 SEA_X/SEA_Z로 다시 만들면 된다.

### 선택 시뮬레이션 (`simulate_sea.py`, `Find_Area` 규칙 재현)
| 지점 | 변경 전 | 변경 후 | 이미지 안 |
|---|---|---|---|
| 항구 상자 안 (250,-210), NPC (225,-190) | 크로나 항구 | 크로나 항구(불변) | IN |
| 배 스폰 (300,-235) | 베른 성 | 베른 앞바다 | IN |
| 접안 지점 (414.25,-423.75) | 베른 성 | 베른 앞바다 | IN (u .54 v .41) |
| 섬 중심 (439.78,-479.87) | 베른 성 | 베른 앞바다 | IN (u .64 v .36) |
| 바다 창 네 모서리 근처·중앙 | 베른 성 | 베른 앞바다 | 전부 IN |
| 성 광장 (100,-100), 성 야외 (120,-200) | 베른 성 | 베른 성(불변) | IN |
| 도서관, 성 내부 | 실내 지도 | 불변 | IN |
| 발탄·쿠크·마하라카 격자 | - | 선택 이미지 변화 0건 | - |

- 바다 창 5 m 격자 샘플 9,207개: 베른 앞바다 8,808, 크로나 항구 399(항구 상자와 겹치는 부분은 항구가 우선). 바다 지도의 표시 이미지 밖 샘플 0개.
- 창 서쪽(x<140) 베른 격자에서 선택 이미지가 바뀐 점 0개.

### M키 월드맵
- 이름과 이미지는 같은 JSON을 읽어 자동 반영된다. 라벨(`WorldMapLabels.json`)·구멍·NPC 아이콘·패널은 지역별이 아니라 레벨(BERN)별 좌표 문서다. 바다 창 안 항목: 라벨 2개("근위대", "군사 지구"가 서쪽 가장자리에 걸림), 사각 구멍 0, NPC 아이콘 0. 새 지역 항목은 따로 필요 없다.
- 확인하지 못한 것: 바다 지도 서쪽 끝에 성 지명 라벨 2개가 표시되는 것이 어색하지 않은지는 화면으로만 판단 가능하다.

### 바뀐 파일
- `Data/UI/Minimap/MinimapAreas.json` (항목 1개 추가, 삭제 0줄, CRLF 유지)
- `Tools/ShipPipeline/build_sea_minimap.py` (신규, ASCII)
- 리소스: `Client/Bin/Resources/UI/Minimap/Maps/BernSea.png`(+ CY_Resources 복사)
- 백업: `C:\Users\USER\.claude\jobs\46aea322\tmp\minimap_sea_backup\MinimapAreas.json`

### 게임에서 확인할 것
1. 베른에서 배를 타고 동쪽 바다로 나가면 미니맵이 파란 바다 지도로 바뀌고 내 표시가 지도 위에 정확히 있는지. 서쪽 부두(x 205~295)로 돌아오면 "크로나 항구" 지도로 돌아오는지.
2. 접안 지점 근처에서 모래색 섬이 미니맵에서 화면 안쪽(북동쪽)에 보이고, 섬 가운데 옅은 웅덩이가 보이는지.
3. M키 월드맵에서도 같은 이름·이미지가 나오는지.

### 미해결
- 생성 이미지라 원작 미니맵과 그림이 다르다. 화면 판정은 사용자가 한다(빌드·실행 안 함).
- 섬 소품(나무·건물)이 나중에 추가되면 이미지는 다시 생성해야 한다(스크립트 재실행).
- 배 스폰 (300,-235)은 항구 선택 상자에서 5 m 동쪽 밖이라, 바다 지도가 된다(이전에는 기본 성 지도). 항구 부두로 오면 항구 지도로 돌아온다.

## REVERT — 항구·바다·마하라카 미니맵 제거 (사용자 요청)

사용자가 항구·바다·마하라카 미니맵이 이상하게 나온다고 하여 이 세 가지만 걷어냈다. 성 내부(도서관 포함) 실내 지도와 위치 기반 지역 선택은 그대로 둔다. 빌드·실행·화면 판정은 하지 않았다.

### 제거한 것
- `Data/UI/Minimap/MinimapAreas.json`에서 항목 3개: `크로나 항구`(BernHarbor.png), `베른 앞바다`(BernSea.png), `마하라카 파라다이스`(Maharaka_0.png). 16개 → 13개(실내 10 + 베른 성 default + 발탄 + 쿠크). CRLF 유지, JSON parse OK.
- `Client/Private/MinimapView.cpp`와 `Client/Private/WorldMapWindowView.cpp`의 레벨 파서 `"MAHARAKA"` 줄 각 1줄.
- `Client/Private/MainApp.cpp` `Update_Minimap`의 MAHARAKA 분기 블록(9줄). 이 파일은 이제 HEAD와 diff가 없다.
- `Client/Public/Level_Development.h`의 `Collect_MinimapMarkers`(3줄). 이 파일도 HEAD와 diff가 없다.
- `Tools/ShipPipeline/build_sea_minimap.py`(미추적, 바다 이미지 전용, 다른 곳에서 호출하지 않음)를 삭제했다. `bern_island_layout.py`는 fork 1의 섬 배치가 쓰는 정본이라 유지했다.
- `C:\Users\USER\OneDrive\바탕 화면\CY_Resources\UI\Minimap\Maps\`에서 `Maharaka_0.png`, `BernHarbor.png`, `BernSea.png` 3장 삭제(전달 대상 제외). 실내 이미지 10장은 남아 있다.
- 로컬 `Client/Bin/Resources/UI/Minimap/Maps/`의 세 이미지는 참조가 없어져 해가 없으므로 지우지 않았다.

### 남긴 것
- 실내 지역 10개(도서관 vol7 `베른 성 (도서관)`, 성 내부 vol9 `베른 성 (성 내부)`, 나머지 8개 `베른 성 (내부)`)와 그 이미지.
- 위치 기반 선택: `Find_Area(level, x, z)`, `default`, `selectMinCm/selectMaxCm`(MinimapView/WorldMapWindowView 코드 그대로).
- `Client/Private/Level_Development.cpp`의 diff는 fork 2(서버 승인 월드 이동)의 것이라 건드리지 않았다.

### 마하라카 레벨은 안전하게 미니맵 없음이 된다 (코드 경로)
- `MainApp.cpp Update_Minimap`에 MAHARAKA 분기가 없으므로 `bHasSnapshot=false`가 유지되고 `m_pMinimapView->Update(..., nullptr)`가 불린다.
- `MinimapView.cpp:251-256`: 스냅샷이 null이면 `pArea`가 null이라 `Hide_All()` 후 return한다. `Find_Area`는 호출되지 않는다.
- `WorldMapWindowView.cpp:702-705`: 같은 방식으로 `pArea==nullptr`이면 닫힘/숨김 처리한다(빈 텍스처를 그리지 않는다).
- JSON에 MAHARAKA 항목이 없으므로 파서의 레벨 문자열 분기도 필요 없다.

### 선택 시뮬레이션 (`Find_Area` 규칙 재현, 제거 전후 JSON 비교)
비교 지점: Bern Gameplay 배치 74개 + 임의 지점 8개(총 76). 선택이 바뀐 점은 모두 제거한 상자 안이었다.

| 지점 | 제거 전 | 제거 후 |
|---|---|---|
| 조선소 NPC 2, 항만관리인, 배(`ship`), 항구 상자 (250,-210) | 크로나 항구 | 베른 성 |
| 배 스폰 (300,-235), 접안 지점 (414.25,-423.75), 섬 중심, 바다 창 모서리 2, `island.dock.to.maharaka` | 베른 앞바다 | 베른 성 |
| 그 밖의 모든 지점(도서관, 성 내부, 광장, 성 야외 포함) | 변화 없음 | 변화 없음 |

- 성/실내 격자(x 0~300, z -260~60, 2 m 간격)에서 항구·바다 상자 때문이 아닌 선택 변화: 0개.
- 실내 항목 10개: 제거 전과 동일. default: `베른 성` 1개. 발탄·쿠크 항목: 동일. MAHARAKA 항목: 0개.

### 검증
- `cl /Zs`(출력 없음) 4개: MinimapView.cpp, WorldMapWindowView.cpp, Level_Development.cpp, MainApp.cpp 모두 `cl_exit=0`, 오류 0.
- `git diff --check`: 공백 오류 없음(autocrlf 안내만). 백업: `C:\Users\USER\.claude\jobs\46aea322\tmp\minimap_revert_backup`.

### 게임에서 볼 것 (사용자 확인)
1. 베른 항구·바다·접안 지점에서는 오늘 이전처럼 `베른 성` 야외 지도가 나온다(항구와 바다에서는 내 표시가 이미지 밖에 놓일 수 있다. 오늘 이전의 동작이다).
2. 도서관 안은 `베른 성 (도서관)`, 성 내부는 `베른 성 (성 내부)`, 야외로 나오면 `베른 성`으로 돌아온다.
3. 마하라카에서는 미니맵과 M키 월드맵이 나오지 않는다(오늘 이전과 같다).
