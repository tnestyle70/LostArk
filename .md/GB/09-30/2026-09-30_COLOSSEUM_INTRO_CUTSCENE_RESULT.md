# 콜로세움 입장 컷신(원본 복원) + 진영 대기 구역 스폰 RESULT

작성: 2026-09-30. 상태: **소스·데이터 반영과 문법 검사까지 완료, 빌드·서버 재시작·화면 확인은 사용자 몫**(이 작업에서 빌드·게임 실행·git 쓰기 없음).

## 1. 결론: 원본 컷신 데이터는 있다 (부분 추론 포함)

| 항목 | 결과 |
|---|---|
| 원본 존재 | **있음.** `LV_PVP_COLOSSEUM_SCENE01A`(`756RZ5Z6VGS7SKKUYE6RKGULU4B2.upk`)의 Matinee `interpdata_3`(8초)가 사용자 영상과 일치 |
| 근거 | 영상 13.83초: 0~2초 대기 구역 화면 → 2.0초 페이드 인 → 2.5~5초 경기장 항공 샷 → 5~9.5초 3대3 도열 샷 + VS → 10초 페이드 아웃 → 대기 구역 화면. 원본은 페이드 0→0.674초, 항공 카메라 c2 0~3초, c1로 컷(3초), 컷 이후 VS·이름 줄 이벤트(3.596~4.036초 등장, 7.278~7.748초 퇴장), 7.768초에 페이드 아웃 시작, 8초 종료. 영상 시각에서 정확히 2.0초를 빼면 원본 시각과 일치(컷 3.0 → 5.0, VS 3.6 → 5.6~6.0, 페이드 아웃 7.77 → 9.77) |
| 같은 패키지의 다른 시퀀스 | `interpdata_1`(14초, 2명씩 3번 순서 소개 = 개인전 계열), `interpdata_31`/`33`(배우 6명/3명, 다른 모드), SCENE02A(1대1 계열). 사용자 영상과 맞는 것은 `interpdata_3` 하나 |
| 추출 방법 | 저장소 리더(`Tools/LevelPlacementExtractor/extract_ue3_placements`)를 붙여 예전 Matinee 추출기를 되살렸다. `Tools/LpkPipeline/extract_scene_matinee.py`(같은 출력 재현 확인, 바이트 동일) |
| 없는 것 | 카메라 액터(`cameraactor_*`)와 배우 액터(`efskeletalmeshactor_*`)는 `Location` 속성이 없다. 즉 위치는 전부 Matinee 트랙 값이다 |

### 추론이 들어간 부분 (사실과 구분)
- **좌표계**: Matinee 값은 X/Z가 경기장 좌표와 같고 높이만 다르다. 배우 발 높이가 0.21m인데 경기장 바닥은 12.64m이므로 **Y에 +12.45m**를 더했다(다른 시퀀스 `interpdata_31`의 배우가 바닥+0.03m에 서 있는 것과 맞춘 추정). 이 12.45는 측정이 아니라 추정이다.
- **x/z 오프셋 없음**: 영상 속 메달리온이 화면 중앙보다 약 2% 왼쪽이라 경기장 중심 x -0.65와 맞는다. 정밀 검증은 아님.
- **카메라 c1은 붐(boom)의 자식**: c1의 이동 트랙 값(-1.097, 1.865)은 로컬 오프셋으로 보이고, 붐 `interpgroup_11`(yaw 90도)의 위치에 더했다. 붐 위치·시야 폭(수평 75도)으로 도열 6명이 화면 66%에 들어오는 것이 영상과 맞아 채택.
- **이벤트 이름의 의미**: `showsequence3`을 VS, `0/1/2`를 도열 뒤→앞 행의 이름 줄로 읽었다. 시각은 영상과 맞지만 이름 자체가 원본에서 무엇을 가리키는지는 확인하지 못했다.

## 2. 적용한 방식

### 2.1 컷신 흐름 (기본 흐름, Debug 전용 아님)
콜로세움 입장(어느 경로든 `LEVEL::COLOSSEUM`) 직후:

1. 검은 화면으로 로컬 캐릭터의 서버 스냅샷을 기다린다(최대 6초. 못 받으면 컷신 없이 페이드 인).
2. 원본 8초 재생: 페이드 인 → 항공 샷(3초) → 도열 샷(5초) → 페이드 아웃. 레터박스(2.35:1)를 넣고 원본이 수평 75도라서 창 종횡비와 무관하게 수평 각을 유지한다.
3. 끝나면 0.6초 동안 검정에서 게임 화면으로 밝아지고 기본 follow 카메라, 서버가 놓은 대기 구역 위치로 돌아온다.
4. 컷신 동안 입력은 막힌다(`CPlayerController`에 입력 전달 안 함 + MainApp의 cinematic HUD 억제로 창·HUD 숨김).

### 2.2 실제 캐릭터와 컷신 배우 (이중 표시 방지)
컷신용 복제 배우를 만들지 않았다. 서버가 복제하는 **실제 캐릭터**를 도열 슬롯에 그대로 보이게 한다.
- `CCharacter::Set_CutscenePoseOverride(position, yaw)`: 표현 transform만 매 `Update`마다 덮어쓴다. 스냅샷은 계속 받으므로 서버 위치는 그대로다.
- 컷신이 끝나면 `Clear_CutscenePoseOverride()`. 다음 프레임에 스냅샷 보간 위치(대기 구역)로 돌아가며, 페이드 아웃으로 가려진 순간이라 순간이동이 보이지 않는다.
- 팀 판정: 서버가 놓은 대기 구역 쪽(경기장 축 x -0.65의 서쪽 = A팀, 도열 왼쪽 / 동쪽 = B팀). 팀 안에서는 NetEntityId 순서로 슬롯 배정. 첫 사람은 가운데 슬롯. 슬롯이 부족하면 남는 사람은 컷신에 나오지 않는다.
- 가이드 AI 등 사람이 아닌 플레이어는 제외.
- 서버 protocol 변경 없음(126 그대로).

### 2.3 라벨 (등급 없음, 직업 + 닉네임만)
- 각 캐릭터의 발 위치를 화면에 투영해 그 아래에 2줄: 위 줄 직업 이름(연한 파란색 13px), 아래 줄 닉네임(흰색 15px). 1280x720 기준 px이며 창 높이에 따라 배율 적용. 검은 8방향 외곽선.
- 원본처럼 도열 뒤 → 앞 행 순서로 3.683 / 3.882 / 4.036초에 나타나 7.395 / 7.583 / 7.748초에 사라진다(150ms 페이드).
- 직업 이름은 캐릭터 선택 창과 같은 6클래스 이름(창술사·건슬링어·슬레이어·도화가·차원술사·워로드).

### 2.4 VS
로딩 화면의 VS(`vs_glow / beam / sparks / flare / v / s`) **텍스처를 그대로 재사용**. 새 리소스 없음. 로딩 레이아웃의 슬롯을 복사해 1.25배로 키워 화면 중앙(y 326/720)에 놓았고, 3.596초에 1.35배에서 줄어들며 나타나 7.278초에 사라진다.

## 3. 대기 구역 스폰 이동

`Data/Worlds/LV_PVP_COLOSSEUM/Gameplay.world.json` revision 2 → 3, `Publish-WorldGameplay.ps1 -Mode Publish -WorldId COLOSSEUM` 1회 게시(46 placements).

| 슬롯 | 위치 (x, y, z) | yaw |
|---|---|---|
| A팀 1/2/3 | (-24.4, 12.22, -2.6 / 0.3 / 3.0) | 90 (경기장 중심 향함) |
| B팀 1/2/3 | (23.0, 12.22, -2.6 / 0.3 / 3.0) | -90 |

- 게시된 `COLOSSEUM.worldbootstrap`에서 6행 확인. 서로 2.9m 이상 떨어져 있다.
- 내비: 위 6점 모두 `LV_PVP_COLOSSEUM.navsource`의 걷기 셀(높이 12.22)이다.
- 소품 회피: 대기 구역 소품의 월드 AABB를 계산해서 골랐다. 지면 높이 소품(상자 `BOX01H/E`, 화물 `CARGO01H`, 무기 걸이 `ARMS01E`, 짚 아래 `DECO01E`)은 x -22.9 ~ -20.3 쪽이고, 아치 `ARENAARCH01A`는 x -23.54 ~ -22.92 폭 0.6m로 z 전체에 걸쳐 있다. 뒤 벽 안쪽 x -25.4 ~ -23.7 띠(너비 1.7m)에는 지면 소품이 없어 그 가운데 x -24.4를 썼다. 횃불·펜스·데코는 높이 14.7m 이상이라 지면에서 겹치지 않는다. 짚(`FARM01A`)은 바닥면이라 서도 된다.
- 한계: **아치가 낮은 벽인지 위에 걸린 구조인지는 확정하지 못했다**(정점 분포로는 z 전체에 낮은 정점이 있음). 스폰이 그 뒤쪽 띠에 있으므로, 화면에서 캐릭터가 아치 뒤쪽에 갇혀 보이면 x를 -22.4 쪽으로 옮겨야 한다.
- 서버는 게시된 bootstrap을 시작할 때 읽으므로 **Server 재시작 필요**.

## 4. 변경 파일

| 파일 | 변경 |
|---|---|
| `Client/Public/ColosseumIntroCutscene.h` (신규, 헤더 전용) | 문서 로드·카메라·페이드·VS·라벨·배우 관리. `.vcxproj` 등록 필요 없음(헤더만이라 컴파일 단위가 늘지 않음) |
| `Client/Public/Character.h`, `Client/Private/Character.cpp` | `Set/Clear_CutscenePoseOverride` (+15줄, Update의 네트워크 transform 직후 적용) |
| `Client/Public/Level_Development.h`, `Client/Private/Level_Development.cpp` | 콜로세움 컷신 소유·Update·Render·`Is_ColosseumIntroActive`, 컷신 중 PlayerController 입력 차단 |
| `Client/Private/MainApp.cpp` | `Sync_CinematicUI`에 콜로세움 컷신 중 HUD/창 억제 4줄 |
| `Data/Camera/ColosseumIntro.cutscene.json` (신규) | 카메라·페이드·도열 슬롯·타이밍 |
| `Data/UI/Colosseum/IntroCutscene_Layout.json` (신규) | VS 슬롯(로딩 것 복사), 레터박스, 페이드 판 |
| `Data/Worlds/LV_PVP_COLOSSEUM/Gameplay.world.json` + 게시본 | 스폰 6곳 |
| `Tools/LpkPipeline/extract_scene_matinee.py`, `build_colosseum_intro_cutscene.py` (신규) | 추출기 복원, 위 두 JSON 생성기 |

`.vcxproj` / `.filters` 변경 없음. `Data/Rendering/**` 변경 없음. 다른 레벨의 컷신·카메라·입력 코드는 건드리지 않았다(MainApp은 콜로세움 조건이 붙은 한 절만 추가).

## 5. 리소스 (CY_Resources)
**새 리소스 없음.** VS는 기존 `UI/Colosseum/MatchLoading/*.png`를 재사용하고, 컷신 카메라·라벨·레터박스는 코드와 JSON이다. 이미 배포된 콜로세움 리소스로 충분하므로 `CY_Resources`에 추가한 파일이 없다. 팀원에게는 Git 데이터(`Data/*`, `Server/Bin/DataFiles/*`)와 소스만 전달된다.

## 6. 검증 (실제로 실행한 것만)
- `cl /Zs` 문법 검사: `Level_Development.cpp`, `Character.cpp`, `MainApp.cpp` 오류 0 (C4819 코드페이지 경고는 기존과 동일). 제품 빌드가 아니다.
- JSON parse: `ColosseumIntro.cutscene.json`, `IntroCutscene_Layout.json`, `Gameplay.world.json` OK. 게시 bootstrap에서 스폰 6행 확인.
- `git diff --check` 이상 없음. 수정한 기존 파일은 바이트 단위 삽입이라 개행·인코딩이 보존되었고 diff에 삽입 줄만 나온다.
- **하지 않은 것**: 빌드, Server/Client 실행, 화면 확인, 서버 재시작, 멀티 클라이언트 확인. 컷신의 카메라 각도, 도열 위치, 라벨 위치, Y 오프셋 12.45는 **화면에서 아직 확인되지 않았다.**

## 7. 위험과 한계
- Y 오프셋과 c1 붐 해석은 추정이다. 도열 캐릭터가 바닥에서 뜨거나 파묻히면 `Data/Camera/ColosseumIntro.cutscene.json`의 `floorY`(캐릭터), 두 샷의 `eye[1]`(카메라)을 조정하면 된다(재빌드 불필요, 재입장으로 확인).
- 캐릭터는 컷신 중 idle 상태의 실제 애니메이션이다. 원본 배우의 `idle_normal_1` 포즈와 다를 수 있다.
- 팀은 서버 스폰 위치로 판정하므로, 혼자 들어가면 A팀 첫 슬롯 하나만 서고 나머지 5칸은 비어 있다. 다인 확인은 하지 못했다.
- **Debug 더미 2대2/3대3 미리보기는 만들지 않았다.** 다른 캐릭터를 콜로세움 안에 만드는 일이 별도 서버 소환 경로가 필요해 "저렴하지 않다"고 판단했다.
- 컷신 중 F1 Debug 도구와 F6 자유 카메라는 막지 않았다.

## 8. 다음 단계 제안 (이번 범위 아님)
- 카운트다운 배너 `경기 시작까지 N초 남았습니다`(스크린샷 174309 상단), 좌우 킬 목록(3대3 점수판).
- 3대3 팀 편성·경기 규칙·서버 권위 승패. 지금 콜로세움은 PvP 규칙이 없는 걷기 아레나다.
- 다인 접속에서 팀 배정이 pen 위치 기준인 현재 방식을 서버가 명시적으로 복제하는 팀 ID로 바꾸기.
- 도열 카메라 이후의 승리 컷신(같은 패키지에 `scene_pvp_victory` 계열이 있음).

## 9. 사용자 확인 순서
1. Visual Studio에서 Debug/x64 빌드(Client와 Server). 서버를 다시 시작한다(스폰 게시 데이터 반영).
2. 로비 `Colosseum` 버튼 또는 베른 성 내부 NPC로 입장한다.
3. 검은 화면 → 항공 샷 → 도열 샷 + VS + 직업/닉네임 → 페이드 아웃 → 대기 구역에서 시작하는지 본다.
4. 컷신 중 키·마우스가 안 먹는지, HUD가 숨는지, 끝나면 돌아오는지 본다.
5. 대기 구역 위치가 소품과 겹치거나 아치에 갇혀 보이지 않는지 본다.
