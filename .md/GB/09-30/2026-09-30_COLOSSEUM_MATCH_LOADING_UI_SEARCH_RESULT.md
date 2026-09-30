# 2026-09-30 '섬멸전 - 콜로세움' 매치 로딩 화면 원본 데이터 조사 RESULT

조사 전용이다. 게임 코드, Data, Resources는 수정하거나 설치하지 않았다(이 문서 하나만 저장소에 만들었다). 추출물과 스크립트는 작업 폴더 `C:\Users\USER\.claude\jobs\46aea322\tmp\colosseum_match_loading\` 아래에만 있다. 원본 게임은 실행하지 않았고, 사용자가 준 이미지 `images\41.jpg`만 읽었다.

## 0. 결론

**있음.** 이 화면은 원본 UI 무비 `colosseumloadings3`(Scaleform GFX)이고, 설치 클라이언트에 미술 30장과 무비 구조, 문자열 키, 계급 표까지 실제로 있다. 다만 아래 셋은 확정하지 못했다.

- 금색 **VS 글자**: 30장의 텍스처 아틀라스에 글자 이미지가 없다. 무비의 벡터 도형이나 파서가 풀지 못한 배치일 가능성이 있다(추론).
- 팀당 슬롯 **개수를 정하는 로직**: 무비는 슬롯 개수를 고정하지 않고 리스트 컨트롤(RollingTileList)이 데이터 개수만큼 반복한다. 그 로직은 무비 안의 ActionScript(DoABC)에 있고 이번에 분석하지 않았다.
- 서버 이름, KDA, 계급을 **원본이 어디서 받아 채우는지**: 서버가 보내는 값이라 클라이언트 데이터에는 없다.

## 1. 항목별 판정

| # | 항목 | 판정 | 근거 |
|---|---|---|---|
| 1a | 제목 '섬멸전 - 콜로세움' 장식과 밑줄 | 부분 | 상단 배경 `cLoadingS3LoadingTopbg_01/02`(I57 1148x88, I58 1920x284). 제목 글자는 텍스트 필드(edit)이고 밑줄 장식이 I57 안에 있는 것으로 보인다. 문구는 `sys.colosseum.teamdeathmatch_title`(섬멸전)과 `tip.name.zonebase_30201`(콜로세움)의 조합(추론). |
| 1b | 팀 패널 배경 프레임 | 있음 | `cLoadingS3textureborder_01`(I69 776x912 어두운 패널과 부드러운 가장자리), 좌우 패널 `leftInfoPanel`(72,81)·`rightInfoPanel`(1107,81), 제목 띠 `leftInfoPanel_titlebg_blue/red`(I46/I47 732x132). |
| 1c | '주목할 캐릭터' 라벨 | 있음 | 문자열 `sys.colosseum.loading_character_watch`=주목할 캐릭터, `loading_player_watch`=주목할 플레이어, `noticable_title`=주목할 플레이어. 무비에 라벨용 텍스트 필드 `TitleTip`(InfoGroup0) 있음. |
| 1d | 금색 VS 엠블럼, 빛 줄기, 반짝임 | 부분 | 그룹 `ColosseumLoadingS3_VersusGroup`(프레임 라벨 `show`)과 효과 스프라이트 `VS_effect01`(35프레임), `VS_effect02`(29프레임), `VS_particle2`(98프레임), `VS_effectMc_02`(28프레임). 효과 미술은 I1, I2, I20~I28, I2D. 마스코트 아이콘은 없음(화면의 작은 새싹 아이콘은 이 무비 밖의 공용 커서/UI 요소로 보임, 미확인). **VS 글자 이미지는 못 찾음.** |
| 1e | 슬롯 프레임(일반/강조/게이지) | 있음 | 슬롯 렌더러 `ColosseumLoadingS3_LineupListRenderer`(90프레임, 상태 라벨 up/over/down/disabled/out/selected_*): 배경 `LineupBg`(I6 228x64, IE 260x184), 강조 `KeyPlayerHighlight`(ID 228x68, 주황 테두리)와 `KeyPlayerLightBall`(IC 192x120), 게이지 `LineupProgress`(I3 주황/I4 빨강/I5 파랑 204x40, 트랙 3프레임). |
| 1e' | 계급, 직업 아이콘 | 부분 | 슬롯에는 아이콘 자리 `deathIconMc`, `rateIconMc`만 있고 계급은 **글자**(`gradeTextField`, '4극' 등)다. 직업도 글자(`classTextField`). |
| 1f | 팁 라벨, 진행바 | 있음 | 팁 무비클립 `loadingTipMc`(353,886): `TitleTip`(팁), `tipTF`(문구), 프레임 도형 1210x189. 진행바 `progressBar`(59,1034): 배경, 트랙(115프레임, 라벨 normal/progress/interrupted), 진행 표식(라벨 normal/progress/completed/interrupted). 미술: I5D(356x40 빛), I5E(1804x24 트랙), I5F(1800x8 배경), I68(1176x152 푸터 장식). |
| 2 | 문자열 | 있음 | 3절 참고(`EFTable_GameMsg`). |
| 3 | 3D 캐릭터 렌더 | 있음(방식 확인) | 무비의 `texture_L_0`(59,128)와 `texture_R_0`(1092,128) 두 스프라이트는 도형이 없고 클래스 이름만 `ColosseumLoadingS3Texture_L_0/R_0`이다. 엔진이 캐릭터를 렌더 타깃으로 그려 이 자리에 넣는 방식으로 보인다(추론, MVP 화면과 같은 구조). 우리 프로젝트에 같은 용도의 `CCharacterPortraitRenderer`가 있다(5절). |
| 4 | 인원 수 변형 | 부분 | 무비에 슬롯 개수 고정값이 없다. 좌우 `leftLineupList`/`rightLineupList`는 리스트 컨트롤(`RollingTileList_ColosseumLoadingS3_LineupList`)이고, 개수는 데이터로 정해진다(추론). 표 `EFTable_Colosseum`은 모드 행마다 시작 위치 `ReadySpotPC1~6`과 연출 위치 `IntroSpotPC1~6`으로 **최대 6인**을 담는다. |
| 5 | 채울 값 | 있음 | 6절. |
| 6 | 추출 파일 | 있음 | 2절. |

## 2. UI 패키지와 추출 파일

- 이 화면 = 논리 패키지 `EFUI_COLOSSEUMLOADINGS3` = 파일 `Packages\QXUI2CCO3OGGQUA3OYJIH4GL.upk`(1,137KB, 무비 1개 `colosseumloadings3` + 텍스처 30장). 이름은 `Tools/MoviePipeline/deobfuscate_names.py`로 패키지 파일 37,064개를 전부 복호해서 찾았다(`all_decoded_names.txt`).
- 무비는 스테이지 1920x1080, 40fps, 루트 1프레임, 스프라이트 56, 도형 11, 편집 텍스트 16, 외부 이미지 30. 루트에 `ColosseumLoadingS3FrameMc`(id 115) 하나가 놓인다.
- 같은 계열 패키지(참고): `EFUI_COLOSSEUM`(`OVSG0AAM1MEEOS8DY2YWW8.upk`, 14MB, 무비 11개: `colosseumloading`(이전 세대 로딩: 파티 HP바 목록형), `colosseumprogresstdm`(섬멸전 진행 HUD), `colosseumprogressffa`, `colosseumprogresseli`, `colosseumprogressobserver`, `colosseumentrance`, `colosseumheadstatus`, `colosseumresultscore`, `colosseumshare`, `colosseumcustomobserver`, `colosseumplaying_loc_int`), `EFUI_COLOSSEUMCHARACTERSELECT`(출전 캐릭터 선택), `EFUI_COLOSSEUMSCORE`(`colosseumplayscore`), `EFUI_COLOSSEUMLEVELINFO`, `EFUI_COLOSSEUMPROGRESSODM`. 스크린샷은 `...S3`(시즌 3형) 무비다.
- 추출물(작업 폴더, 설치 안 함):
  - `movie\colosseumloadings3.gfx`(577,993바이트), `movie\colosseumloadings3.export.bin`, `movie\parsed.json`(파서 결과), `movie\tree115.txt`(배치 트리)
  - `movie_old\colosseumloading.gfx`(이전 세대, 참고)
  - `tex\EFUI_COLOSSEUMLOADINGS3\Texture2D\*.tga` 30장(UModel 30/30 성공), 미리보기 `preview\*.png` 30장과 `preview_contact_sheet.png`
  - 문자열 `gamemsg_colosseum.txt`, 전체 패키지 복호 이름 `all_decoded_names.txt`
- 텍스처 아틀라스와 쓰임(크기): I1 672x1040 VS 주황 빛 쐐기, I2 764x936 VS 빛, I20 512x172 렌즈 플레어, I21 128x128 초록 링, I22 460x124 입자, I23 480x644 금빛 광선, I24~I27 입자 4종(312x324~344x392), I28 776x712 금빛 광채, I2D 1024x128 오브와 플레어 시트, I3/I4/I5 204x40 주황/빨강/파랑 게이지, I6 228x64 슬롯 프레임, IC 192x120 빛구슬, ID 228x68 강조 테두리, IE 260x184 슬롯 아래 문양, I46/I47 732x132 파랑/빨강 제목 띠, I57 1148x88 상단 장식, I58 1920x284 상단 그라데이션, I59 24x20 관전자 아이콘, I5D 356x40 진행 빛, I5E 1804x24 트랙, I5F 1800x8 트랙 배경, I68 1176x152 푸터 장식, I69 776x912 패널 배경, I6A 1920x916 메인 배경(쌍둥이 석상 부조).

## 3. 문자열 위치 (`EFTable_GameMsg`, UTF-8, 키는 ASCII)

`EFTable_GameMsg.db`는 이미 복호된 사본이 `tmp\proving_ground\tables\EFGame_Extra\ClientData\TableData\`에 있다. 화면 문구의 키:

| 화면 문구 | 키 |
|---|---|
| 섬멸전 | `sys.colosseum.teamdeathmatch_title` |
| 콜로세움 | `tip.name.zonebase_30201` (`sys.map.filter_entrance_pvp_arena`도 같은 글자) |
| 주목할 캐릭터 / 주목할 플레이어 | `sys.colosseum.loading_character_watch` / `loading_player_watch`, `noticable_title` |
| 팁 | `sys.colosseum.tip_title`, `sys.hint.loadingbar_title_tip` |
| KDA 1.6 | `sys.colosseum.loading_kda` = `KDA {0}` |
| 계급 표기 | `sys.colosseum.loading_pc_pvplevel_name` = `계급 {0}`, 계급 이름 `tip.name.pvpname_1~40` |
| 대표 캐릭터 / 최고 승률 | `loading_ceo_character_title` (`대표 캐릭터 <{0}>`), `loading_bast_win_rate_value` (`{0} 시 {1}%`) |
| MMR | `loadingpage_mmr_value`(`{0}점`), `loadingpage_*_mmr_ranking*` |
| 관전자 로딩 | `loading_observer_name`, `loading_observer_value`(`{0} / {1}`) |
| 대기 중 문구 7종 | `sys.colosseum.loading_1~7` (예: '투기장 바닥을 청소 중..', 끝에 '입장을 취소할 경우 페널티가 적용됩니다.') |
| 팁 문구 | `sys.hint.zone_colosseum_001~034`. 화면의 '상대팀 PvP 계급이 너무 높나요? 걱정 마세요. 실력 평점은 비슷합니다.'는 `sys.hint.zone_colosseum_010` |
| 직업 이름 | `tip.name.claas01`(버서커), `claas04`(인파이터), `claas06`(블래스터), `claas08`(소서리스) 등 일부만 이 표에 있고, 나머지 직업 키는 확인하지 못함 |

- PvP 계급 표 `EFTable_PvPLevelInfo`(40행): 이름은 `tip.name.pvpname_N`이고 순서는 20급~1급(1~20), 초단, 2단~10단(21~30), 초극, 2극~9극(31~39), 무극(40). 스크린샷의 '8단', '4극', '2단'이 여기에 있다. `ShowLoadingImage` 열은 1~20(급)만 1이라 급 구간에서만 로딩 이미지를 보인다는 뜻으로 보이며 이 매치 화면과의 관계는 미확인.
- 로딩 이미지 표 `EFTable_LoadingImageGroup`: 존 30201에 `PVP_20220727_1~4`(PvP 계급 1~20 구간, `PvpType` 2 또는 -1). 이 배경 이미지는 이미 넣은 `PVP_LUTERAN_30201`과 다른 것이고, 이 매치 화면(무비)과는 별개 경로다.
- 서버 이름은 이 표에 없다. 우리 로비 데이터 `Data/UI/Lobby/LobbyServers.json`에 쥬신, 루페온, 카단, 카제로스, 실리안, 아만이 있다.

## 4. 인원 수 변형 (사실과 추론)

- 사실: `EFTable_Colosseum` 18행이 존(30201~30206)과 모드 키(SecondaryKey 0, 2, 3, 4, 6, 7)별로 `ReadySpotPC1~6`, `RoundStartSpotRed/Blue`, `IntroSpotPC1~6`, 승리 연출 위치, 카메라 ID를 가진다. 6인 슬롯 구조이므로 3대3이 최대다. 30201은 모드 키 2와 4가 시작 위치 7~12, 3이 1~6을 쓴다.
- 사실: `sys.colosseum.modtooltip_teamdeathmatch`가 섬멸전을 '3명이 팀을 이루어 제한시간 동안 많은 킬을 내는 쪽이 승리'로 설명하고, 관전자는 최대 8명(`custom_tooltip_pvpmode`).
- 추론: 이 UI가 1대1, 2대2에서 슬롯 수를 줄이는 방식은 리스트 컨트롤이 데이터 개수를 렌더하는 것이다. 무비에는 인원별 프레임 라벨이 없다.
- 미확인: 모드 키(2, 3, 4, 6, 7)가 각각 섬멸전, 1대1 투혼전, 대장전 중 무엇인지는 표로 확정하지 못했다.
- 무비에는 정보 패널이 두 그룹(`infoGroup0`, `infoGroup1`)으로 나뉘어 있고 각각 `InfoList` 리스트(`ColosseumLoading_InfoListRenderer`)를 가진다. 섬멸전(주목할 캐릭터)과 다른 모드(주목할 플레이어, 대표 캐릭터, 최고 승률)를 다르게 보이려는 구조로 보인다(추론).

## 5. 3D 캐릭터 렌더와 우리 프로젝트

- 원본: `texture_L_0`, `texture_R_0` 자리에 엔진이 렌더 타깃을 넣는 구조(1절 3).
- 우리 프로젝트에 이미 같은 구조가 있다(코드로 확인): `CCharacterPortraitRenderer`(`Client/Public/CharacterPortraitRenderer.h`)는 복제된 캐릭터를 별도 렌더 타깃에 그리고, UI는 `CUILayoutRuntime::Set_SlotTextureSRV(slotId, SRV)`로 그 결과를 슬롯에 붙인다. 캐릭터 정보창, 아바타북, MVP 상 페이지(`MvpResultView`, `Data/UI/MVP/MvpResult_Layout.json`)가 쓰고, 카메라 거리, 높이, FOV, 방향(`CAMERA`)을 조절한다.
- 재사용 가능한 나머지: `CUILayoutRuntime`의 `Set_SlotVisible`, `Set_SlotCaption`(글자와 폰트), `Set_SlotTexture`, `Set_SlotFillRatio`, `Set_SlotRect`, `Set_SlotAnimation`(flipbook), 레이아웃 정본 형식(`lostark.ui-layout` formatVersion 1). 로딩 레이아웃은 `Data/UI/Loading/LoadingLayout.json`(16슬롯)이고 `Level_Loading.cpp`가 레벨별로 배경, 제목, 팁을 바꾼다(콜로세움 분기는 이미 있고 배경만 다르다).
- 캐릭터 6종(창술사, 건슬링어, 슬레이어, 도화가, 차원술사, 워로드)은 `Character Select` 연출용 프리뷰 경로(`ClassSelectionPresentation`, `CharacterPreviewPanel`)로 이미 다룬다. 외형(시바견 인형탈 같은 아바타 복장)은 우리 아바타 시스템에 있는 것만 표현된다.

## 6. 채워야 할 값 (원본 슬롯 필드)

슬롯 렌더러의 텍스트 필드 이름과 화면 대응(무비의 `nameTextField` 등, 화면 스크린샷과 대조):

| 필드 | 화면 | 원본 값 | 우리에게 있나 |
|---|---|---|---|
| classTextField | 직업명(버서커, 기상술사, 블레이드, 브레이커, 인파이터) | 직업 이름 문자열 | 부분. 우리는 6직업 이름만 있고 원본 직업은 다름. 표시는 우리 직업 이름으로 대체 필요 |
| gradeTextField | 계급(4극, 8극, 2단, 8단, 7단, 6단) | `PvPLevelInfo` 행의 `tip.name.pvpname_N` | **없음**(계급 시스템이 없음). Debug용 고정값이나 서버 값 필요 |
| nameTextField | 닉네임 | 플레이어 닉네임 | **있음**(서버가 스폰 시 복제하는 닉네임, `Test-####` 등) |
| serverTextField | 서버 이름(카제로스, 실리안, 아만) | 캐릭터의 서버 이름 | 부분. 이름 목록은 `LobbyServers.json`에 있으나 플레이어별 값 없음 |
| killTextField / deathTextField / rateTextField | 무비에는 킬/데스/승률 필드. 이 화면은 `KDA {0}` 한 값만 표시 | `loading_kda`=`KDA {0}` | **없음**(KDA 통계 없음) |
| deathIconMc, rateIconMc | 데스/승률 아이콘 | 무비 내 | 이번 화면에서는 안 보임 |
| 주목 캐릭터 강조 | 슬롯 하나가 주황(좌)/적갈색(우) 게이지로 강조 | `KeyPlayer*` 강조 스프라이트, 선정 규칙은 서버 | **없음**(선정 규칙 미확인, 임의 지정 가능) |
| 팀 패널 상단 | 서버 이름과 닉네임(주목 캐릭터) | 주목 캐릭터 정보 | 위 값의 조합 |
| 진행바 | 왼쪽에서 오른쪽으로 파란 빛이 차오름 | 로딩 진행률 | **있음**(우리 로더 진행률) |

서버가 보내야 하는 값: 방 안 참가자 목록(닉네임, 직업, 팀, 계급, KDA, 서버 이름, 주목 여부). 우리 프로토콜에는 nickname과 `CHARACTER_CLASS_ID`만 있고 계급, KDA, 서버 이름은 없다(`Shared/Public`에서 pvp, kda, serverName 검색 결과 없음). 지금 프로토콜 126은 콜로세움을 '걷는 아레나(PvP 규칙 없음)'로만 다룬다(`PacketType.h` 주석).

## 7. 구현 설계 초안 (구현하지 않았음)

1. **빌더**: `Tools/LpkPipeline/build_colosseum_match_loading_ui.py`(기존 `build_mvp_*`, `build_ocean_hud_ui.py` 패턴). `parsed.json`의 도형/비트맵 sub 사각형(`sub` 테이블, 예: shape#46 → `colosseumLoadingS3_I2D.tga [0,0,128,128]`)으로 아틀라스를 잘라 `Client/Bin/Resources/UI/Colosseum/MatchLoading/*.png`를 만들고, 배치 트리에서 좌표를 읽어 `Data/UI/Colosseum/MatchLoading_Layout.json`(`lostark.ui-layout` v1, 참조 1920x1080)을 생성한다.
2. **슬롯 ID 구성**: `Bg_Main`, `Bg_TopL/R`, `Panel_L/R`, `Title_L/R`(파랑/빨강), `Watch_Label_L/R`, `Watch_Server_L/R`, `Watch_Name_L/R`, `Slot_{L,R}_{0..2}_{Frame,Highlight,Fill,Class,Grade,Name,Server,Kda}`, `Vs_Glow_*`, `Vs_Text`, `Tip_Label`, `Tip_Text`, `Progress_{Bg,Track,Head}`, `Footer_Deco`, `Observer_Icon/Count`. 팀당 최대 3개 슬롯(최대 6인)을 만들고 인원에 맞춰 `Set_SlotVisible`과 `Set_SlotPosition`으로 가운데 정렬한다(1대1이면 슬롯 1개, 2대2는 2개).
3. **동적 텍스트**: `Set_SlotCaption`으로 닉네임, 직업, 계급(`tip.name.pvpname_N` 문자열), 서버 이름, `KDA {0}`, 팁(`sys.hint.zone_colosseum_NNN`)을 채운다. 원본 폰트 클래스(`$YG760`, `$YoonGasiIIM`)는 우리 폰트로 대체한다.
4. **3D 캐릭터**: `CCharacterPortraitRenderer` 두 개(좌/우)로 주목 캐릭터를 그려 `Panel` 슬롯에 `Set_SlotTextureSRV`. 캐릭터가 방에 없는 로딩 시점이라 로컬 프리뷰 캐릭터를 쓰거나(Debug) 서버가 준 외형 정보를 스폰해야 한다.
5. **VS 연출**: 효과 스프라이트(35/29/98/28프레임)를 flipbook(`Set_SlotAnimation`) 또는 키프레임 JSON(`MvpResult_*.keyframes.json` 방식)으로 옮긴다. VS 글자 이미지는 원본에서 못 찾았으므로 (a) 파서를 보강해 벡터 도형을 풀거나 (b) 화면 참고 이미지를 보고 우리가 그린 글자를 쓴다.
6. **표시 경로**: `Level_Loading.cpp`의 `LEVEL::COLOSSEUM` 분기에서 기본 로딩 레이아웃 대신 이 레이아웃을 열고 진행바만 로더 진행률에 연결한다. 이후 3대3 VS 연출 화면(회색 경기장 배경에 캐릭터 6명, 가운데 VS)은 별도 작업이다.
7. **서버**: 방 참가자 목록을 로딩 화면이 읽을 수 있도록 이미 복제되는 값(닉네임, 직업)을 우선 사용하고, 계급, KDA, 서버 이름은 Debug 고정값이나 새 필드(프로토콜 상승 필요)로 정한다. PvP 매치 규칙(팀 편성, 카운트다운, 점수)이 없으면 이 화면의 참가자는 지금은 방 안 플레이어뿐이다.

## 8. 작업량, 난이도, 위험

| 작업 | 난이도 | 비고 |
|---|---|---|
| 아틀라스 잘라 PNG와 레이아웃 JSON 생성(빌더) | 중 | 도구와 패턴이 이미 있음. 도형 좌표 변환 필요 |
| 슬롯, 패널, 팁, 진행바 등 정적 UI | 중 | 기존 레이아웃 런타임으로 가능 |
| 동적 텍스트, 인원별 배치 | 중 | `Set_SlotCaption`, `Set_SlotPosition` 있음 |
| 주목 캐릭터 3D 프리뷰 | 중·상 | 로딩 시점에 캐릭터 스폰과 모델 준비 필요 |
| VS 연출(flipbook 4종 + 글자) | 상 | 벡터 도형과 블렌드(가산) 표현, 글자 미확인 |
| 서버 값(계급, KDA, 서버 이름, 주목 규칙) | 중·상 | 프로토콜 상승과 PvP 시스템 부재 |

위험: (1) GFX 파서 한계 — 미해결 문자(`?char#...`), 그라데이션, 클리핑, 필터를 못 풀어 VS 글자와 일부 장식을 놓칠 수 있다. (2) ActionScript(DoABC)를 분석하지 않아 리스트 개수 규칙과 상태 전환(up/over/selected)을 추정해야 한다. (3) 원본 폰트 대체로 글자 폭이 달라져 슬롯 텍스트가 넘칠 수 있다. (4) 3D 프리뷰 성능과 조명. (5) 콜로세움이 아직 걷는 아레나뿐이라 이 화면이 보여줄 실제 참가자 데이터가 없다.

## 9. 확인하지 못한 것

- VS 글자 이미지의 실제 소스, 마스코트(새싹) 아이콘의 소스.
- 슬롯 개수를 결정하는 ActionScript 로직, 인원별 레이아웃 차이의 데이터.
- `EFTable_Colosseum` 모드 키(2, 3, 4, 6, 7)의 의미.
- 원본이 주목 캐릭터를 고르는 규칙, 서버 이름과 KDA의 출처(서버 값으로 추정).
- `colosseumloading`(이전 세대 파티 HP바 목록형 화면)이 지금 어떤 모드에서 쓰이는지.
- 이번 조사는 이미지 1장(`41.jpg`)만 기준으로 했고, 이후 3대3 VS 연출 화면(`052145.png`)이 어느 무비/씬인지는 조사하지 않았다.
