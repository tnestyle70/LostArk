# 2026-09-06 캐릭터 커스터마이징 원작 추출 REFERENCE

추출물 위치: `D:\ClaudeWork\Extracted\CustomizeGfx_Extracted\<movie>\{*.gfx, *.xml(swf2xml), as3/, trace_*.txt, symbols.txt, images.txt}`,
텍스처 PNG와 대조 시트 `png\_contact_sheet.png`, 문자열 `strings_pccreate.txt`(318줄, `pccreate.*` 한글).
아직 구현하지 않은 수집 단계. 우리 프로젝트의 `LEVEL::CHARACTER_SELECT`(캐릭터 생성) 재작업 시 입력으로 쓴다.

## 패키지와 movie

| movie | 패키지 hash | 역할 | AS3 |
|---|---|---|---|
| `charactercustomizing` | `SZWK4EED0B0EPSBEWIPQCKVKJ6.upk` | 현행 커스터마이징 화면(좌 의상/동작/레벨 패널, 우 탭 패널, 하단 생성/이름 확인, 상단 안내) | `ark.ui.characterCustomizing.*` 374 |
| `charactercustomize` | `QXUI2CCBY9YCNQ9CUGNOAITQ.upk` | 구형 로비 커스터마이징(`ark.ui.lobby.characterCreate.*`, 설정 renderer 세트) | 390 |
| `createcharacter` | `NURFZ996NVKN98V6V9KN6.upk` | 클래스 선택/생성(좌 클래스 정보, 중앙 프리뷰·생성 버튼, 우 클래스 목록, 시험 소환 프레임) | `ark.ui.createCharacter.*` 290 |
| `colorpicker` | `OVSG0AAM1M7TGAUO7DE2YW.upk` | 색상환 피커 창 2종(`ARKColorPickerWnd`, `ARKColorPicker2Wnd`) | 253 |
| `colorselect` | `OVSG0AAM1M7EO1OALDE2YW.upk` | 색 슬롯 선택 창(`ColorSelectWndContent`, 리스트 아이템) | 233 |

텍스처(`D:\로아 리소스\텍스쳐,메쉬\EFUI_*`): `charactercustomizing_i1`(1024² 조각), `_i10b`/`_i1b6`(512×2048 좌·우 패널 배경),
`charactercustomize_i13`(1024²)/`_i2`(1024×256)/`_i273`/`_iea`, `createcharacter_i1`(1024²)/`_i62`/`_i63`/`_iab`(512×2048)/`_id7`(2048×256),
`colorpicker_ie`(1024×256 색상환), `colorselect_i4`(512×64 슬롯 프레임).

## `charactercustomizing` 루트 배치 (`CharCustomFrame_Main`, 1920×1080)

| 패널 | 위치 | 내용 (`trace_panels.txt`, 세부 `trace_tabs_depth3.txt`) |
|---|---|---|
| `leftPanel` `CharCustom_LeftPanel` | (0,0) | 배경 `CharCustom_LeftBG`, 라벨 3개 (35,50)/(35,171)/(35,296) 18px `#a4afb7` (`pccreate.customizing_label_costume` 의상 / `_action` 동작 / `pccrate.customizing_lv_select_title`), 타일 리스트 `leftDressList`(37,77) `leftActionList`(37,197) `leftBgList`(37,323), 하단 페이지 `bottomPage`(33,426) = 저장 슬롯·스타일북 버튼, `leftResetButton`(245,36) |
| `rightPanel` `CharCustom_RightPanel` | (1920,0), 내부는 음수 x | 배경 `characterCustomizing_I1B6` (0,0)-(447,1080) @(−445,0), 탭 리스트 `rightTabList`(−433,73), 제목 라벨(−442.9,18), 탭 페이지 7개 모두 (−437,192): default/face/hair/eye/skin/adorn/voice |
| `bottomPanel` `CharCustom_BottomPanel` | (0,1080), 내부는 음수 y | `createPage`(660,−175) `createOutGamePage`(660,−175) `nameConfirmPage`(624,−208), 버튼 back(28,−99) setting(128,−99) helpDesk(128,−99) resetAll(220,−116) |
| `topPanel` `CharCustom_TopPanel` | (563,−1) | 배경 `characterCustomizing_I1` (404,679)-(970,730) @(0,73), 안내 라벨(12,32) |
| `noticePanel` `CharCustom_NoticePanel` | (563,191) | 배경 `I1` (404,0)-(970,170) 566×170, 변경 기간 라벨 18px `#ffd200` |

우측 탭 내부(얼굴 기본/세부 파트 0~5, 머리 기본/투톤, 눈, 피부, 장식(데코·엠블럼·문신·메이크업), 음성)는 `CharCustom_Right_Tab*` 심볼과
`ARKDefaultSlider_V2_CharacterCustomizing`(슬라이더), `CharCustom_ColorPickerButton`, `CharCustom_DefaultCheckBox_V2`, `Shared_RollingTileList_*` 조합이다.

## `createcharacter` 루트 배치

| 프레임 | 자식 |
|---|---|
| `CreateCharacterSelectFrame` | 배경 `listBgMc`(266,0), 제목 배경(264,10)·라벨(298,24), 클래스 롤링 리스트 `bindClassList`(324,93), 프리뷰 타일 `bindPreviewList`(0,149)·`bindPreviewSingleList`(330,146.5), 접기 `foldMc`(291,613) |
| `CreateCharacterCenterFrame` | 생성 버튼 `select_btn`(786,992), 제목 배경·라벨(660,14), `preview_btn`(799,948) `try_btn`(961,948) |
| `CreateCharacterClassInfoFrame` | `classInfoMc`(0,0) 클래스 설명/아이덴티티/태그, 하단 prev(38,981) setting(128,981) story(223,981) |
| `CreateCharacterExamFrame` | 접기 토글(336,−3), 소환 `summonBtn`(7,968) `bossBtn`(92,968) `cancelSummonBtn`(178,968), 생성 `createBtn`(339,982) 노랑 |

클래스 썸네일은 `ARKIcon_Class_Thumbnail/2/3`, 설명 문자열은 `sys.pccreate.class_desc_<class>`, 난이도 `sys.pccreate.class_desc_difficulty_<class>`.

## 다음 단계 후보

1. 우리 `LEVEL::CHARACTER_SELECT`(현재 클래스 선택 + 생성) 화면을 `createcharacter` 배치로 재구성.
2. 커스터마이징은 우리 캐릭터 파츠 시스템(AVATAR_HEAD/ARMOR 정도)에 맞는 탭만 골라 `charactercustomizing` 배치로 축소 구현.
3. 색상 피커는 `colorpicker_ie` 색상환 + `ARKColorPickerWnd` 배치 그대로 재사용 가능.
