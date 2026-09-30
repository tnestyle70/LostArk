# 콜로세움 경기 시작 카운트다운 글씨 + 창살 하강 RESULT

작성: 2026-09-30. 상태: **소스·데이터 반영과 문법 검사(`cl /Zs`)까지 완료. 빌드·게임 실행·화면 확인은 사용자 몫**(이 작업에서 빌드·게임 실행·캡처·git 쓰기 없음).
선행 문서: `2026-09-30_COLOSSEUM_INTRO_CUTSCENE_RESULT.md`, `..._INTRO_CUTSCENE_FIX_RESULT.md`.

## 1. 영상·사진 시간표 (프레임 관찰)

입력: `콜로세움 창.mp4`(2560x1600, 30 fps, 3.23초, 97프레임, 게임 화면은 그 안의 초광폭 영역), 사진 `스크린샷 2026-09-30 222238.png`(3초 시점), `...222250.png`(배너 확대). 프레임은 OpenCV로 전부 뽑아 픽셀 단위로 쟀다(한글 경로라 `np.fromfile`+`imdecode`).

| 영상 시각 | 프레임 | 내용 |
|---|---|---|
| (사진) | — | 배너 "경기 시작까지 03 초 남았습니다." + 붉은 글씨 "3초 후 경기가 시작됩니다", 바 길이 33.9 % |
| 0.00 ~ 0.67 | f0~f20 | "2초 후…", 배너 "02 초", 바 12.3 % 고정 |
| **0.70** | f21 | 숫자가 **1**로 바뀜(배너·붉은 글씨 동시) |
| 0.70 ~ 1.67 | f21~f50 | "1초 후…", 바가 12.3 % → 0으로 직선 감소(1.6초쯤 빈 바) |
| **1.70** | f51 | 붉은 글씨가 **"경기 시작!"**으로 바뀜, 배너 자리에 TIME 240 점수판 |
| 1.83 | f55 | 창살 온전 |
| **1.87** | f56 | 창살 끝만 남음(높이의 약 85 % 아래로) |
| 1.90 | f57 | 끝이 조금 더 내려감 |
| **1.93** | f58 | 창살 사라짐(바닥에 어두운 홈만 남음) |
| 1.97 ~ 3.0 | f59~f90 | "경기 시작!" 계속 표시(영상 끝까지 최소 1.5초 유지) |

카운트 시계로 옮기면 3초 시작 = 0, "2초" = 1.0초, "1초" = 2.0초(영상 0.70초에 해당), **시작 = 3.0초**(영상 1.70초), 창살 = 시작 후 0.13 ~ 0.23초.

## 2. 원본 데이터에서 찾은 것 / 측정으로 채운 것

### 원본에서 나온 것
| 항목 | 출처 |
|---|---|
| 배너·카운트 UI가 있는 무비 | `EFUI_COLOSSEUM`(패키지 `OVSG0AAM1MEEOS8DY2YWW8.upk`)의 `colosseumplaying_loc_int`. `waitGroup_mc`(`colosseumMapTitle_lb`, `remainTimeStr_lb`, `remainProgress`), `countDownAnnounce_mc`(`str_lb`), `start_mc`. `dump_upk_movie.py` + `gfx_scene_extract.py`로 덤프 |
| 배너 배경 | 이미지 sub-389: 10x182 세로 그라데이션(위 alpha 174 → 아래 0), 무비가 2560x182로 늘려 그림 |
| 진행 바 | 틀 = sub-348(321x10, 어두운색), **채움 = sub-352(317x6, 앰버색 그라데이션. 무비 이름은 `track`이지만 채움 이미지)** |
| 붉은 글씨 뒤 얼룩 | 공용 무비 `localresource_loc_int`(`EFUI_LOCALRESOURCE`)의 링크 클래스 `colossenumPlaying_countDownAnnounce_bg` = sub-369, 306x94 |
| 위치 | 무비 좌표(1920x1080 스테이지). `waitGroup_mc`는 x -320에 있어 자식 x에서 320을 빼야 화면 좌표가 됨 |
| 카운트 3, 2, 1 · "경기 시작!" | 무비 문자열 `ST_SYS_COLOSSEUM_PVPStartCountdown`, `ST_Common_Count1/2/3`, `ST_Common_CountStart`에 대응. 표시 문구는 사진 그대로 |
| 창살 | 원본 레벨 `LV_PVP_COLOSSEUM_PS`의 `BG_LUT_LUCASTLE_ARENADOOR01_SM_BJH`. 양쪽 대기 구역 입구에 각 5장씩(A: x -21.6 export 3188~3192, B: x 20.2 export 2571·2572·2574·2575·2576, 폭 2.05 m 간격 z -4.14~3.98) |

### 측정으로 채운 것 (원본 값이 아님)
| 항목 | 값과 근거 |
|---|---|
| 배너 배율 0.81 | 영상의 바 길이 258 px(무비 317) — 바 길이, 바 행, 제목 행, 줄 행 4곳이 모두 0.81과 맞음 |
| 글씨 폭 | 제목 54, 배너 줄 160, 붉은 글씨 199 (스테이지 px, 사진·영상 실측). 글꼴 크기는 이 폭에 맞춰 실행 중 자동 계산 |
| 색 | 붉은 글씨 금주황(1.0, 0.74, 0.36) + 짙은 붉은 번짐, 제목 (0.82, 0.76, 0.63), 배너 줄 (0.78, 0.80, 0.80). 사진 픽셀 평균 |
| 바 길이 키 | 3.0초 남음 33.9 %(사진 1장), 1.7초·1.08초 남음 12.3 %(영상), 0.1초 남음 0 % |
| 창살 타이밍 | 시작 후 0.13초에 시작, 0.13초 동안, 3프레임 관찰 |

### 원본에 없거나 확정하지 못한 것 (추정)
- **창살에는 원본 움직임(Matinee/InterpActor/Kismet) 데이터를 찾지 못했다.** 레벨 PS의 InterpActor는 물과 하늘 둘뿐이고, SCENE01A/02A 시퀀스 22개를 전부 열었지만 문 트랙이 없다(카메라·페이드·배우뿐). 대기 구역 DeployData도 없다(`leveldata1.lpk`에 zone 30201 없음). 그래서 **창살이 어떻게 움직이는지는 영상 3프레임 측정값**이고, 어느 문짝인지는 원본 배치 데이터로 특정했다.
- 문짝 배치는 UE에서 roll -180°(뒤집힘)로 놓여 있고 뾰족한 쪽이 위다. 바닥 위로 나온 높이가 1.33 m로 영상의 창살 높이(약 1.4 m)와 맞아서, **레벨의 배치 자세가 '닫힌 상태'** 라고 판단했다(추정).
- 하강 깊이 1.6 m와 곡선(OUT_CUBIC)은 추정이다(영상에서 끝이 1.87초에 1.07 m, 1.93초에 사라짐이 보임).
- 붉은 "경기 시작!" 얼룩은 카운트 얼룩과 같은 이미지·같은 크기로 그렸다(영상에서 크기가 같음을 확인. 별도 원본 이미지는 못 찾음).
- "경기 시작!" 유지 시간(2초 + 0.4초 페이드)은 영상이 끝나서 모른다. 3초 시점 바 값도 사진 1장뿐이라 3.0~1.7초 구간은 직선 보간이다.
- 사운드(카운트, 시작음), 시작 후 TIME 240 점수판, 좌우 킬 목록은 만들지 않았다.

## 3. 구현

- 새 헤더 `Client/Public/ColosseumMatchStart.h`(헤더 전용, 컴파일 단위 추가 없음): 카운트 시계, 배너·붉은 글씨(`CUILayoutRuntime` 4슬롯 + `Draw_Text`), 창살 하강. 프레임 델타 200 ms 제한, 시작 프레임은 0으로 처리(레벨 진입 hitch 방지).
- 창살 이동은 새 경로를 만들지 않고 기존 `CMapPlacementRuntime::Apply_PlacementTransform`(Map Editor·월드 시퀀스가 쓰는 것)을 쓴다. 배치를 `sourcePlacementId`로 찾고, 원래 자세를 보관했다가 `Reset()`이 그대로 되돌린다. 배치 문서(LFS)는 수정하지 않았다.
- 슬롯은 무비 좌표를 스테이지(1920x1080) 기준, 화면 가운데 정렬, 높이 기준 균일 배율로 매 프레임 다시 놓는다(초광폭 창에서 이미지가 늘어나지 않게).
- `Level_Development`: 콜로세움 레벨에서 카운트 객체를 소유하고, **컷신이 끝나는 순간 자동 시작**(컷신이 없으면 입장 즉시), F1에서 컷신을 다시 재생하면 창살을 닫고 배너를 숨김.
- 수치는 전부 `Data/Camera/ColosseumMatchStart.json`(재빌드 없이 F1 Play로 다시 읽음). 이미지는 `Client/Bin/Resources/UI/Colosseum/Countdown/{top_gradient,bar_frame,bar_fill,announce_bg}.png`, 레이아웃 `Data/UI/Colosseum/MatchCountdown_Layout.json`, 생성기 `Tools/LpkPipeline/build_colosseum_countdown_ui.py`.

### 바꾼/추가한 파일
| 파일 | 변경 |
|---|---|
| `Client/Public/ColosseumMatchStart.h` | 신규 |
| `Client/Public/Level_Development.h` | 전방 선언 1, 멤버 3 |
| `Client/Private/Level_Development.cpp` | include 1, 생성 4줄, Update 연결 13줄, Render 1줄, F1 섹션 |
| `Data/Camera/ColosseumMatchStart.json` | 신규 |
| `Data/UI/Colosseum/MatchCountdown_Layout.json` | 신규(생성기 산출) |
| `Tools/LpkPipeline/build_colosseum_countdown_ui.py` | 신규 |
| 리소스 4개 | `Client/Bin/Resources/UI/Colosseum/Countdown/` + `Desktop\CY_Resources`의 같은 상대 경로 (sha1 일치, 신규만) |

`.vcxproj`/`.filters` 변경 없음, protocol·Server 변경 없음, 렌더링 옵션 변경 없음. 기존 파일의 편집은 바이트 삽입이며 삽입 위치의 개행(CRLF/LF)을 따랐다(백업: 작업 폴더 `tmp\gate\bak\*.before_matchstart`).

## 4. F1에서 재생하는 방법
1. 콜로세움 입장(Debug 로비 `Colosseum` 버튼 또는 베른 NPC) → 컷신이 끝나면 **자동으로 3초 카운트가 시작**된다.
2. 다시 보려면 F1 → `Colosseum Match Start (countdown + gate)`(컷신 섹션 바로 아래): `Play Countdown + Gate`(JSON을 다시 읽고 처음부터), `Pause`/`Resume`, `Stop / close gate`(배너를 숨기고 창살을 원위치), `Match time (s)` 슬라이더(창살도 시간에 맞춰 오르내림).
3. 컷신 재생 중에는 `Play Countdown + Gate`가 비활성이다. 수치가 마음에 안 들면 `ColosseumMatchStart.json`만 고치고 Play를 다시 누른다.
- 조정 항목: `bar`(바 길이 키), `layout`(글씨 위치·폭·배너 좌표), `colors`, `startTextHoldMs`/`startTextFadeMs`, `gate.delayAfterStartMs`/`durationMs`/`sinkMeters`/`curve`.

## 5. 한계
- **표현 전용이다. Server에 콜로세움 경기 규칙이 없어서** 카운트 시계는 Client 로컬(컷신 종료 시각 기준)이고, 창살이 열려도 대기 구역 이동을 막거나 풀어주는 Server 판정은 없다(대기 구역 이동은 이전 작업의 내비 그대로). 여러 명이 접속해도 카운트가 서로 동기화되지 않는다.
- 자동 시작은 컷신이 끝난 뒤 로컬에서만 일어난다. 서버가 경기 시작 tick을 내려주는 구조가 생기면 `Begin()`을 그 신호에 연결해야 한다.

## 6. 검증 (실제로 실행한 것만)
- `cl /Zs` 문법 검사: `Level_Development.cpp`(새 헤더 포함) rc=0. 제품 빌드가 아니다.
- JSON parse: `ColosseumMatchStart.json`, `MatchCountdown_Layout.json` OK. 게시된 `LV_PVP_COLOSSEUM.mapplacements`에서 창살 10개 export ID와 좌표(x -21.6 / 20.2, y 13.50, visible 1)를 확인.
- 시간 시뮬레이션(파이썬으로 C++ 샘플링 재현): 영상 0.70초 숫자 1, 바 0.114(측정 0.112), 1.0초 0.080(0.074), 1.87초 창살 1.07 m 하강, 1.93초 1.58 m — 측정 프레임과 일치.
- 생성 이미지 4장 sha1이 `CY_Resources` 사본과 일치. `git diff --check` 이상 없음.
- **하지 않은 것:** 빌드, 게임 실행, 화면 확인. 글씨 위치·크기·색, 얼룩 위치, 창살이 실제로 내려가는 모습과 깊이, 자동 시작 타이밍은 **화면에서 아직 확인되지 않았다.**

## 7. 사용자 확인 순서
1. Visual Studio에서 Debug/x64 빌드(Client). Server·protocol 변경 없음.
2. 콜로세움 입장 → 컷신 → 페이드 인 뒤 배너(콜로세움 / 경기 시작까지 03 초…)와 붉은 "3초 후 경기가 시작됩니다"가 나오고 1초마다 2, 1로 바뀌는지, 0에서 "경기 시작!"과 함께 배너가 사라지고 0.13초 뒤 양쪽 창살이 바닥으로 내려가는지 본다.
3. 글씨 위치·색·크기가 원본 사진과 다르면 `ColosseumMatchStart.json`의 `layout`/`colors`만 고치고 F1 `Play Countdown + Gate`.
4. 창살이 다 안 숨거나 너무 깊으면 `gate.sinkMeters`, 타이밍이 다르면 `gate.delayAfterStartMs`/`durationMs`.
5. 새 리소스 4개(`UI/Colosseum/Countdown`)는 `CY_Resources`에 들어 있으니 Drive 배포 때 함께 나간다.
