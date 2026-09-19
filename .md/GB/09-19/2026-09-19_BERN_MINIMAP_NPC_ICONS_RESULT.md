# 베른 M 키 지도의 NPC 아이콘 RESULT

작성일: 2026-09-19
브랜치: `feature/maharaka-island-level` (마하라카 작업과 같은 작업 폴더, 이 문서의 변경은 마하라카 파일과 무관)
범위: 베른에서 M 키로 여는 지도에 NPC별 아이콘(강화 NPC는 망치)이 뜨게 한다.

## 1. 결론 먼저

- 구현되지 않은 기능이 아니었다. M 키 → 월드맵 창 → NPC 기능 심볼 그리기는 이미 구현돼 있었다.
- 끊긴 곳은 **데이터**였다. 아이콘 사전 `WorldMapNpcSymbols.json`에 베른 NPC 49개 중 **1개**(`npc.bern.schmidt`, 망치)만 있었고, 나머지는 코드가 사전에 없다는 이유로 건너뛰었다. 이전 작성자는 "나머지는 기능 없는 마을 사람"이라고 가정했지만 원본 `EFTable_Npc.MapSymbolIndex`를 보면 사실이 아니었다.
- 이번에 원본 테이블로 38개의 심볼을 확정해 사전을 채우고, 필요한 아이콘 14종을 원본 아틀라스에서 잘라 설치하고, 슬롯 상한을 8에서 48로 올렸다.
- **미해결**: 이전 상태에서도 망치 1개는 그려져야 했는데 사용자는 "아이콘이 안 나온다"고 보고했다. 코드 경로로는 이 증상의 이유를 찾지 못했다(4절). 실제 화면은 사용자가 확인해야 한다.

## 2. M 키가 여는 것

M → `CMainApp`이 `CWorldMapWindowView::Toggle()`(`MainApp.cpp:1455-1470`)을 호출한다. 이 창이 "지도"이며 미니맵(`CMinimapView`, 우상단 HUD)과는 별개다. 베른은 `Update_Minimap`(`MainApp.cpp:4948-`)에서 지원 레벨이고 지도 정의는 `Data/UI/Minimap/MinimapAreas.json`의 `BERN`이다.

## 3. 원인 추적 (확인 사실)

| 단계 | 결과 | 근거 |
|---|---|---|
| 서버 스폰 | 베른 NPC 49개가 `WORLD_ENTITY_KIND::NPC`로 스폰되고 배치 문서의 `placementId`가 그대로 실린다 | `GameRoom_Helpers.cpp:609`, `GameRoom_WorldEntities.cpp:128` |
| 스냅샷 | 클라이언트가 `NPC_MARKER{x,z,placementId}`로 모은다 | `ClientReplication.cpp:1672-1683` |
| 그리기 | `m_NpcSymbols`(사전)에 있는 배치만 그린다. 없으면 `continue` | `WorldMapWindowView.cpp` NPC 루프 |
| 사전 | 이전 상태 1개 (`npc.bern.schmidt` → `Minimap_Symbol_1_207`) | `WorldMapNpcSymbols.json` 이전 내용 |
| 슬롯 | 이전 8개. 사전이 커져도 8개까지만 그렸다 | `NPC_SYMBOL_SLOT_COUNT = 8` |
| 아이콘 이미지 | 이전 Resources에 10종뿐(NPC용은 175와 망치 `1_207`) | `Client/Bin/Resources/UI/WorldMap/Symbols` |

### 강화 NPC의 실제 식별
프로젝트가 정의한 강화 NPC는 `Level_Bern.cpp:1127`의 `ITEM_UPGRADE_NPC_PLACEMENT_ID = "npc.bern.schmidt"` 하나다(장비 강화 창을 연다). 원본 `EFTable_MapSymbol`에는 "강화"라는 NPC 서브카테고리가 없고, 망치는 아이템 카테고리의 `sys.map.filter_item_enhance`이며 `Minimap_Symbol_1_207`이다. 이미지를 직접 열어 망치 모양임을 확인했다. 이름을 보고 추측하지 않았다.

### 원본 조회 규칙
`EFTable_Npc.MapSymbolIndex` → `EFTable_MapSymbol.IconName1` → `IconInfo.loa`(페이지·좌표). 배치 49개 중:

- 38개: 원본 심볼 확정, `IconInfo`에서 전부 조회됨 (16종 아이콘)
- 7개: 원본 `MapSymbolIndex=0`(마을 사람) → 원본에도 아이콘이 없다
- 4개(`npc.bern.aylara`, `beda.guide`, `plaza.01`, `plaza.02`): 원본 NPC 행이 없는 수동 archetype → 추측하지 않고 등록하지 않았다

### 파일명이 비슷한 다른 아이콘
`Minimap_Symbol_207`(원본 25007 스이에)과 망치 `Minimap_Symbol_1_207`은 서로 다른 이미지다(픽셀 비교 `False`). 페이지가 다르다(`Minimap_Symbol`=0번대, `Minimap_Symbol_1`=1번대).

## 4. 미해결: 망치 1개도 안 나왔다는 관찰

이전 코드와 데이터로도 `npc.bern.schmidt`는 그려져야 한다. 아래를 코드·데이터로 배제했다.

- 지도 경계: 좌표 `(143.07, -104.17)`는 경계(X 14.7~270.7, Z -220.5~35.5) 안
- 범례 필터: `npc` 기본 켜짐(`Is_LegendChecked` 기본 true)
- 레이아웃: `WM_NpcSym_0~7` 존재, `m_SlotIds`로 매 프레임 켜짐
- 텍스처 교체: `Set_SlotTexture` 경로 정상
- 사전 읽기: `CProjectDataRoot::Resolve`로 저장소 `Data/`에서 직접 읽음(배포 복사본 없음)
- 파서: 파일이 작고 ASCII

확정하지 못한 가설 두 가지가 남는다.

1. 사용자가 본 빌드가 이 데이터·이미지가 들어오기 전의 것이었을 가능성(아이콘 이미지는 Git 비추적이라 다른 PC에는 없을 수 있다).
2. 화면에서만 재현되는 문제(그리는 순서, 스냅샷 `hasLocal`, 창 상태). 이 세션은 Client를 실행하지 않았다.

이 두 가설 중 어느 쪽인지 이 세션에서는 판정하지 못했다. 이번 수정 후에도 망치가 안 나오면 6절의 진단을 따라 달라.

## 5. 변경

| 파일 | 변경 |
|---|---|
| `Data/UI/WorldMap/WorldMapNpcSymbols.json` | 사전 1개 → 38개 (원본 조회 결과만, CRLF). 심볼이 없는 배치 11개는 의도적으로 없다 |
| `Data/UI/WorldMap/WorldMap_Layout.json` | `WM_NpcSym_*` 슬롯 8 → 48 (기존 블록 텍스트 복제, 다른 슬롯·최상위 필드 불변 확인) |
| `Client/Private/WorldMapWindowView.cpp` | `NPC_SYMBOL_SLOT_COUNT` 8 → 48, 슬롯 ID를 한 번만 만드는 `Get_NpcSymbolSlots()`, `#include <array>` |
| `Tools/LpkPipeline/build_worldmap_ui.py` | `NPC_SYMBOL_SLOTS` 48, NPC 사전을 저장소 JSON에서 읽는 `load_npc_symbols()`(이중 정본 방지) |
| `Client/Bin/Resources/UI/WorldMap/Symbols/` (Git 비추적) | 아이콘 14종 신규 설치 |

신규 아이콘: `Minimap_Symbol_151, 152, 153, 158, 162, 176, 206, 207, 228, 29, 35`, `Minimap_Symbol_1_147, 1_203, 1_526`. 기존 `175`, `1_207`은 새로 잘라 픽셀 비교해 동일함을 확인했다(컷팅이 기존 도구와 같은 결과).

원본 추출 산출물(저장소 밖): `C:\LostArkExtract\MinimapSymbols_20260919` (아틀라스 4페이지 DDS, `IconInfo.loa`, 잘라낸 아이콘, 조회 결과 `bern_npc_symbols.resolved.json`).

C++/Data 파일의 인코딩·줄바꿈은 기존과 같다(C++ CRLF·UTF-8, U+FFFD 0).

## 6. 검증 (단계별)

| 단계 | 내용 | 결과 |
|---|---|---|
| (1) 코드 경로 | M 키 → 창 → 스냅샷 → 사전 조회 → 슬롯 배치를 끝까지 읽음 | 완료 |
| (2) 데이터 스크립트 | 사전 38개 배치 ID가 모두 베른 월드 문서에 존재 / 아이콘 경로 전부 디스크에 존재 / 망치 연결 / 레이아웃 슬롯 48 = C++ 상수 48 / 슬롯이 연속 / 저장된 JSON = 원본 조회 결과 | 통과 |
| (2) 데이터 스크립트 | 사전의 모든 NPC가 지도 경계 안에 있는가 | **1개 실패**: `npc.bern.src.42`(원본 25016 스텔리아) `(239.7, 275.6)`이 경계 밖 |
| (3) 컴파일 | `cl.exe /Zs`(구문 검사, 파일을 쓰지 않음)로 `WorldMapWindowView.cpp` 단일 파일 검사: 오류 0, 이전부터 있던 C4819 인코딩 경고만. 일부러 넣은 오타는 `C3861`로 잡혀 검사가 유효함을 확인 | 통과(구문만). **전체 빌드·링크는 실행하지 않았다** (VS가 켜져 있어 MSBuild 금지) |
| (3) 생성기 | `build_worldmap_ui.py` 파이썬 문법 검사 통과, 새 헬퍼는 실제 JSON으로 단독 실행해 38개·망치 확인 | 문법·헬퍼만. **생성기 전체는 이 PC에서 실행 불가**(원본 추출 폴더가 D 드라이브) |
| (4) 화면 | 실행하지 않았다 | **미확인, 사용자 확인 필요** |

### `npc.bern.src.42`
09-18 커밋 `d70a4fad`가 NPC 19명을 **원본 좌표로 복원**한 상태이고, 이 NPC의 원본 좌표가 베른 지도 이미지 범위 밖(성 밖 z=+275)이다. 지도 이미지 자체를 넓히는 일이라 이번 범위 밖이며, 배치 좌표는 건드리지 않았다. 원본 심볼(206)은 그대로 등록했고 **지도에서는 보이지 않는 것이 정상**이다.

## 7. 사용자 확인 절차

1. 아이콘 이미지는 Git 비추적이다. 이 PC 외에서 확인하면 `Client/Bin/Resources/UI/WorldMap/Symbols/`에 위 14종이 있어야 한다.
2. VS에서 솔루션 빌드(`WorldMapWindowView.cpp` 재컴파일). 사전·레이아웃은 창을 만들 때 한 번만 읽으므로 **Client를 다시 시작**해야 한다(Client가 켜져 있으면 링크가 막힌다).
3. Server+Client 실행 → 베른 입장 → **M**.
4. 기대 결과(이 세션이 화면으로 확인한 것이 아님):
   - 강화 NPC(슈미트, 광장 `(143, -104)` 부근)에 **망치** 아이콘
   - 창고·우편·수리·거래소·펫 등 원본 심볼이 있는 NPC마다 해당 아이콘(38개 중 지도 안 37개)
   - 스텔리아(요리사)는 지도 밖이라 안 보임
   - 원본 심볼이 없는 11개는 아이콘 없음
   - 지도 하단 범례에서 `NPC` 항목을 끄면 NPC 아이콘이 전부 사라진다(망치는 `NPC` 범례에 묶여 함께 사라진다)
5. 그래도 안 나오면 알려 달라: `Client/Bin/<구성>/Diagnostics/` 로그, 또는 VS 출력창의 `[WorldMapWindow] ... missing/parse failed` 줄.

## 8. 알려진 한계

- 망치가 NPC 범례(`npc`)에 묶여 있다. 원본 범례에서는 강화 아이콘이 아이템(`item`) 카테고리다. 이 세션은 동작을 바꾸지 않았다.
- 한 화면에 그릴 수 있는 NPC 아이콘은 48개다.
- 이 사전은 베른 전용이다(배치 ID 기준). 다른 레벨은 다루지 않았다.
- 심볼이 없는 NPC 11개, 지도 밖 NPC 1개는 위 이유로 아이콘이 없다.
