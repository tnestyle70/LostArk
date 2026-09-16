## 요약

캐릭터 머리 위 이름표·HP바와 채팅 말풍선을 원작 `headstatus.gfx` 배치로 다시 만들고, 칭호 시스템(원작 `honortitle.gfx` 축소 창 + Server 소유 칭호 id)을 추가한다. protocol **87 → 88** — Server와 Client를 같은 커밋으로 함께 빌드·재시작해야 한다. (09-16 origin/main 리베이스 완료. 충돌 3건: `PacketType.h` 버전 줄, `PlayerController.cpp`·`MainApp.cpp`의 탈것 스킬/전투분석기 추가와 인접한 위치 — 모두 양쪽 유지.)

## 동작

- **이름표**: 베른·발탄·쿠크세이튼에서 모든 플레이어 머리 위에 HP바(82×9 프레임 + 78×5 fill, 본인/파티원/타인 별 원작 아트)와 이름(`YG760` 12px `#ECECEC`, 중앙). 칭호가 있으면 `칭호 이름` 한 줄. 캐릭터 선택은 닉네임을 정하기 전이라 그대로 없음.
- **말풍선**: 원작 `HeadStatus_Balloon` normalBG 9-slice, 폭 200px 자동 줄바꿈, 15자 이하 한 줄은 중앙, 이름판 바로 위에 붙는다.
- **칭호**: P창 칭호 행 오른쪽 「칭호변경」 → 칭호 창(완성형 목록 12행·휠 스크롤, 「사용중」 배지, 「현재 칭호 : ○○」, 적용/해제, Esc/X, 제목 드래그). 목록은 원작 완성형 17개 + 프로젝트 칭호 6개(161기 최후의 4인 / 최고의 팀장 / 이펙트의 천재 / 맵 제작의 신 / 161기 최고령 / 161기 반장). 착용 조건 없음. 월드 이동 후에도 유지.

## 데이터 경로

```
Data/Titles/HonorTitles.json ──Publish-HonorTitles.ps1──▶ Server/Bin/DataFiles/HonorTitles/HonorTitles.bootstrap
Client 칭호 창 → CPlayerController::Request_HonorTitle → C2S_SET_HONOR_TITLE
  → CGameRoom::Handle_SetHonorTitle (bootstrap 멤버십 검사) → S2C_SET_HONOR_TITLE_RESULT
  → PLAYER_SNAPSHOT.iHonorTitleId → CClientReplication → 이름표 / HUD_PLAYER_STATE → P창·칭호 창
```

- Server는 `HonorTitles.bootstrap`이 없으면 방을 준비하지 않는다(탈것 bootstrap과 같은 정책). Server host는 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server`(domain `honortitles.catalog`)로 만든다.
- 월드 이동은 `SERVER_WORLD_TRANSFER_REQUEST.iHonorTitleId` → `ROOM_COMMAND.iCarriedHonorTitleId` → 목적지 방 bootstrap에 있는 id만 유지.

## 파일

- 신규: `Client/Public|Private/HonorTitleCatalog.*`, `HonorTitleWindowView.*`, `Server/Public|Private/HonorTitleCatalog.*`, `Server/Private/GameRoom_HonorTitle.cpp`, `Data/Titles/HonorTitles.json`, `Data/UI/HeadStatus/*`, `Data/UI/HonorTitle/*`, `Tools/LpkPipeline/build_head_status_ui.py`, `build_honor_title_data.py`, `Tools/GameplayPipeline/Publish-HonorTitles.ps1`, `.md/TJ/09-14/2026-09-14_이름표_{PLAN,RESULT}.md`.
- 수정: Shared `NetworkIds.h`·`PacketMessages.h/.cpp`·`PacketType.h`; Server `GameRoom*.cpp/.h`·`ServerApp.cpp`·`ServerPlayer.h`·`RoomCommand.h`·`ServerTriggerSystem.h`·vcxproj; Client `WorldPlayerNameplateView.*`·`WorldPlayerChatBubbleView.*`·`Level_Bern/ValtanArena/KakulSaydonArena.*`·`CharacterInfoWindowView.*`·`MainApp.*`·`PlayerController.*`·`NetworkManager.*`·`NetworkPlayerCommandSink.*`·`PlayerCommandSink.h`·`ClientReplication.*`·`CombatHUDViewModel.*`·vcxproj; `Data/UI/CharacterInfo/CharacterInfo_Layout.json`(버튼 slot 1개 append)·`CharacterInfoDisplay.json`(표시용 title 제거); `Tools/Build/BuildDomains.json`·`Invoke-BuildDomainOwner.ps1`·`test_build_domain_pipeline_receipts.py`.

## 리소스 (Drive)

`Client/Bin/Resources/UI/HeadStatus/`(9 PNG), `UI/HonorTitle/`(11 PNG). 새 사운드 없음.

## 검증

- `cl /Zs`: Shared·Server·Client 변경 파일 전부 오류 0. `git diff --check` OK. JSON parse OK. `Publish-HonorTitles.ps1` Validate/Publish OK(23행).
- `Tools/Build/test_*.py` 54개: 실패 5 + 오류 2는 `origin/main`에서도 같은 항목. publisher 기대 목록에 `vehicles.profiles`(기존 누락)와 `honortitles.catalog`를 추가해 2건이 통과로 바뀌었다.
- 사용자 화면 확인: (빌드 후 기록)

## 남은 것

- 길드·functional(자리비움 등) 줄, 원작 관계별 색 프리셋, 머리 본 위치 앵커, 칭호 창 스크롤바 아트·앞/뒤 조합·획득 조건.

🤖 Generated with [Claude Code](https://claude.com/claude-code)

https://claude.ai/code/session_019zmgoFGPiTr9zKr4z1fY3J
