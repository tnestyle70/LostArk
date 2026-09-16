# feat(ui): 머리 위 이름표·칭호 시스템과 퀵슬롯 HUD 보강(탈것 6종·특수 슬롯·버프 바·스킬 표식)

브랜치 `feature/quickslot-buff-hud` (protocol **89**, Server·Client 함께 빌드). 두 슬라이스를 담는다.

## 1. 이름표·말풍선·칭호 (`.md/TJ/09-14/2026-09-14_이름표_RESULT.md`)

- 머리 위 이름표를 원작 `headstatus.gfx` 배치로 다시 만들었다. 앵커는 몸 모델 눈 본을 표현 루트(클래스 스케일·좌석 오프셋 포함)로 변환한 머리 위치라 탈것에 타도 함께 올라간다. `$YG760` 12px×1.2, 칭호 `#3AAFFF` + 닉네임 `#EBE8C8` 한 줄. HP 게이지는 원작 화면에 없어 두지 않는다.
- 채팅 말풍선을 원작 `HeadStatus_Balloon` 9-slice로 다시 만들었다(이름판 위에 스택).
- 칭호 시스템: `Data/Titles/HonorTitles.json`(원작 추천 17 + 프로젝트 6) → `Publish-HonorTitles.ps1` → `Server/Bin/DataFiles/HonorTitles/HonorTitles.bootstrap`. P창 「칭호변경」 → 칭호 창(원작 `honorTitleWnd`, 스크롤바·휠) → `C2S_SET_HONOR_TITLE` → Server 판정 → `PLAYER_SNAPSHOT.iHonorTitleId` → 이름표·P창. 월드 이동 시 칭호 유지.
- UI 휠 입력을 `WM_MOUSEWHEEL`로 받는다(`CUIInputRouter::Get_MouseWheelNotches`). hover 시 마우스를 점유해 DirectInput 휠이 0이 되던 문제로, 커스터마이즈 화면의 휠 스크롤도 같이 살아난다.
- 로딩바를 단계 합산 진행도로 바꿨다(Loader `Declare_Phases`, 이펙트 lane 25% 가중).
- 칭호 창이 열리면 P창 라벨이 그 아래로 가려진다(`Set_TopWindowRect` + `Set_Covered`).

## 2. 퀵슬롯 HUD 보강 (`.md/TJ/09-16/2026-09-16_퀵슬롯_버프_HUD_RESULT.md`)

- 탈것 창을 main의 6종(`VehicleCatalog`)으로, 스킬은 `VehicleProfiles.json skills[]`(SPACE/Q/W/E)에서 읽는다. 탑승 HUD는 Q/W/E에 탈것 스킬 아이콘 + 복제 쿨타임 파이·초, R/A/S/D/F 잠금.
- Space 특수 슬롯: 원작 `QuickSlotSpecialSkillBar` 자리(엠블렘 위 중앙)에 탈것 SPACE 또는 클래스 이동기를 **쿨타임이 도는 동안만** 표시.
- 버프/디버프 바: 원작 `BuffSlotManagerEx` 배치·아트(`Shared_BuffSlot_Common`)로 복제 상태만 표시 — 탑승, 워로드 방어 태세, 침묵·속박·공포(남은 초).
- 스킬 표식: 원작 `Shared_SkillSlotType_Icon`(연계 `>>`, 키다운 ↓, 퍼펙트 콤보). 프로젝트 `skillKind`(HOLD/COMBO) 우선, 그 외는 `EFTable_Skill.Type`(테이블 값 4/5→키다운, 6/7→연계). 연계 창은 `inputOpenMs..inputCloseMs`의 남은 비율 파이.
- 새 데이터: `Data/UI/HUD/SkillSlotMarks.json`, `HudBuffSources.json`, `HUD_Layout.json`에 슬롯 56개 append.

## Shared/Server

- `NETWORK_PROTOCOL_VERSION` 88 → **89**: `PLAYER_SNAPSHOT.iHonorTitleId`, `C2S_SET_HONOR_TITLE`, `S2C_SET_HONOR_TITLE_RESULT`.
- Server `CHonorTitleCatalog`(bootstrap 필수, 탈것과 같은 정책), `GameRoom_HonorTitle.cpp`, 월드 이동 시 칭호 전달. `HUD_PLAYER_STATE`에 `iFearEndTick`·`Cooldowns` 추가.
- pull 후 `Invoke-BuildDomainOwner.ps1 -Owner Server`를 한 번 실행해야 한다(`honortitles.catalog` 포함).

## 리소스 (Drive, `Client/Bin/Resources/UI/` 전체 업로드)

`UI/HeadStatus/`(말풍선), `UI/HonorTitle/`(18), `UI/HUD/Buff/`, `UI/HUD/Common/SkillType_*`, `UI/Skill/<Class>/<id>_Space.png`, `UI/Vehicle/` 갱신.

## 검증

- Debug 제품 빌드 PASS(최신 main e71dd131 리베이스 뒤 전체, 이후 증분 2회). `cl /Zs` 오류 0, JSON parse OK, `git diff --check` OK.
- `Tools/Build/test_*.py` 54개: 실패 5·오류 2로 main과 동일. `test_build_domain_pipeline_receipts` 기대 목록에 `vehicles.profiles`·`honortitles.catalog` 추가.
- 화면 확인(사용자): 베른에서 칭호 창·적용/해제·이름표 반영, 탈것 창 6행, 탑승 Q/W/E 쿨타임, 버프 바(탑승·방어 태세) 확인. 이름표 색·위치 수정본과 탑승 시 위치, 스킬 표식은 베른 로딩 시간 문제로 팀장 로딩 최적화가 들어온 뒤 다시 확인한다.

## 알고 있는 한계

- `PLAYER_SNAPSHOT.Cooldowns`는 16개까지라 클래스 쿨타임이 많이 돌면 탈것 쿨타임이 빠질 수 있다.
- 버프 바는 Server가 복제하는 상태만 보인다. 길드명·자리비움 줄은 Server 데이터가 없어 없다.
- 칭호 창의 앞/뒤 조합·획득 조건·검색은 두지 않았다. 이름표 색은 원작 host 프리셋 값이 없어 캡처 추정치다.

🤖 Generated with [Claude Code](https://claude.com/claude-code)

https://claude.ai/code/session_019zmgoFGPiTr9zKr4z1fY3J
