# 2026-09-30 main 병합 · push · PR 결과

작업 브랜치 `codex/colosseum-pvp-entry-0930`(이전 PR #487 브랜치에서 분기)를 만들어 오늘 작업분을 커밋하고, `origin/main`(2019abe4f)을 병합해 충돌을 양쪽 보존으로 해결한 뒤 push했다. 빌드와 화면 확인은 하지 않았다.

## 1. 병합 전 상태

| 항목 | 값 |
|---|---|
| 시작 브랜치 / HEAD | `codex/maharaka-sea-island-waterpang-0930` / `6faf55189` |
| origin/main | `2019abe4f` (이 브랜치보다 15커밋 앞, 이 브랜치의 고유 커밋 0개) |
| 미커밋 변경 | 163개 (수정 67, 미추적 96) |
| 안전망 | 태그 `backup/pre-merge-0930` (= `6faf55189`) |

main에서 새로 들어온 것(15커밋, 파일 200개): PR #488(아바타: 모코코 AV_036 5개 직업, 마스크 헤어 깊이 패스, 클래스 중립 아바타 상점과 착용 복제), PR #489(레이드 회귀 수정: 발탄 마법구 무력화·갑옷 회복, 관전 HUD, 발탄 회귀 수정과 최종 빌드 문서), 마하라카 PR #487 병합 기록, 그리고 **protocol 126**(물총 발사·아바타 아이템·미니게임 마감을 통합한 레이아웃).

## 2. 충돌 예측과 실제 충돌

`git merge-tree --write-tree HEAD origin/main`(비파괴)로 먼저 예측한 결과와 실제 병합 결과가 같았다. 충돌은 3개 파일이다.

1. `Shared/Public/Network/PacketType.h`
2. `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` (두 hunk)
3. `CLAUDE.md`

양쪽이 함께 바꾼 파일은 이 3개를 포함해 13개다. 나머지 10개(`BalanceTestPanel.cpp`, `Character.cpp`, `Level_Lobby.cpp`, `Loader.cpp`, `MainApp.cpp`, `MainApp.h`, `GameRoom.cpp`, `GameRoom_Inventory.cpp`, `PacketMessages.cpp/.h`)는 자동 병합됐다. **게시 데이터·LFS 지도 파일은 양쪽에서 겹치지 않아** publisher 재실행이 필요하지 않았다.

## 3. 해결 방법(양쪽 보존)

- **protocol 번호.** main의 126(물총·아바타·미니게임 마감 통합)과 이 브랜치의 126(`WORLD_ID::COLOSSEUM`)·127(콜로세움 대기열 4패킷)은 서로 다른 wire 변경이 같은 번호를 쓰고 있었다. 두 쪽을 모두 유지하고 `NETWORK_PROTOCOL_VERSION`을 max+1인 **128**로 올렸다. 주석에는 125→126(main)→127(콜로세움 월드)→128(대기열) 순서를 남겼다. main은 `PACKET_TYPE` enum을 바꾸지 않았고 버전 줄만 바꿨으므로, 이 브랜치의 콜로세움 4패킷은 enum 끝에 append된 그대로다(기존 패킷 번호 재배정 없음).
- **NetworkProtocolHarness.** 두 충돌 hunk는 이쪽 문장을 128로 올려 하나로 합쳤다. 자동 병합돼 남아 있던 main의 126 비교 9곳과 이쪽 127 비교 1곳은 모두 128 기대값으로 맞췄고, 그 비교에 붙은 메시지 문구도 함께 바꿨다. "Protocol 126 preserves shop purse…"처럼 도입 시점을 설명하는 주석과 메시지는 그대로 뒀다. 하네스는 실행하지 않았다.
- **CLAUDE.md.** 같은 문장에서 protocol 표기만 달랐다(v127 대 v126). 이쪽 문장을 유지하고 v128로 맞췄다.

## 4. 병합 후 검증(실행한 것만)

- 병합 결과 대비 main: main이 병합 기준점 이후 추가한 줄이 양쪽 겹침 파일 10개에서 **0줄 누락**, main 대비 삭제된 파일 0개.
- 병합 결과 대비 이 브랜치: 이 브랜치가 추가한 줄의 누락은 protocol 번호 127→128로 의도해 바꾼 12줄뿐이다(`CLAUDE.md` 1, `PacketType.h` 3, 하네스 8).
- `cl /Zs` 구문 검사 22개 파일 모두 rc=0: `PacketMessages.cpp`, Server 7개(`GameRoom.cpp`, `GameRoom_Inventory.cpp`, `GameRoom_Admission.cpp`, `ServerApp.cpp`, `ServerTriggerSystem.cpp`, `WorldBootstrap.cpp`, `SpawnGroupBootstrap.cpp`), Client 13개(`MainApp.cpp`, `Loader.cpp`, `Character.cpp`, `Level_Lobby.cpp`, `Level_Bern.cpp`, `Level_Loading.cpp`, `Level_Development.cpp`, `NetworkManager.cpp`, `NetworkPlayerCommandSink.cpp`, `LevelTransitionService.cpp`, `BalanceTestPanel.cpp`, `RaidEntryPreviewView.cpp`, `WorldGameplayDocument.cpp`), `NetworkProtocolHarness.cpp`. 제품 빌드가 아니다.
- 이 브랜치가 바꾼 JSON 14개 파싱 통과, `git diff --cached --check` 경고 0, 충돌 마커 잔존 0.
- 소스 인코딩: 충돌 파일 3개는 앵커 기반 바이트 패치로 해결했고 U+FFFD 0, CRLF와 LF 개수가 일치한다.
- LFS: 스테이징된 `.mapassets`·`.mapplacements`·`.mapset`이 LFS 포인터(`version https://git-lfs.github.com/spec/v1`)로 올라가는 것을 확인했다.
- Data/Rendering은 main 대비 차이가 없다(팀장 정본 유지).

**실행하지 않은 것:** 제품 빌드, NetworkProtocolHarness 실행, Server 시작, publisher(게시) 재실행, Client 화면 확인. 참고로 `Server/Private/ServerGameplayContractTests_SpawnGroups.cpp:1285`에 `NETWORK_PROTOCOL_VERSION == 101u` 비교가 남아 있다. 이번 변경과 무관한 기존 값이고 손대지 않았다.

## 5. 커밋 구성

| 순서 | 커밋 | 내용 |
|---|---|---|
| 1 | `0bcbe8f87` | feat(colosseum): 입장 대기열, 매치 로딩, 컷신, 카운트다운, 로딩 초상 코드 (Shared/Server/Client 47개 파일, 하네스, CLAUDE.md) |
| 2 | `67e522b27` | feat(colosseum): 콜로세움 레벨 데이터, UI 레이아웃, 게시 데이터, 베른 바닥 보강 (49개 파일) |
| 3 | `ddccdb070` | docs: 09-30 결과 문서 19개 |
| 4 | `a5c26536d` | Merge origin/main |
| 5 | (이 문서 커밋) | 병합·PR 결과 문서 |

## 6. 제외한 파일과 이유

- 개인·임시 파일: `.claude/`, `.codex/`, `dev/`, `glaivier_skill_phase_1_3_reextract.*`, `mario_meshes.json`, `mesh_cook_list.txt`, `temp_effect_action_out/`, `Client/Default/Client.vcxproj.user.bak-20260913`, `Data/Navigation/LV_LUT_MIDNIGHTC_ED.navpaint.before-prune`.
- 예전 리소스 배포 목록: `Copy_ResourceDistribution_*.ps1` 20개, `Resource_Distribution_*.txt` 20개. 리소스는 Git 비추적이고 Drive로 전달한다.
- `Server/Bin/DataFiles/Gameplay/ValtanPresentationGenerations/ff38c13d…json`: 09-25 발탄 프레젠테이션 세대 파일로 이번 작업과 무관하다.
- `Data/Balance/NumericSourceBindings.json`: 다른 domain의 생성 해시 갱신(쿠크 문서 2개의 sha256)이 작업 폴더에 남은 것으로 이번 작업과 무관하고 main도 같은 파일을 바꿨다. 복사본을 `tmp/NumericSourceBindings.json.local_before_merge`에 두고 작업 폴더에서는 HEAD 상태로 되돌렸다.
- `Data/UI/Loading/LoadingLayout.json`, `Data/UI/ScreenUI/ScreenUI.json`: 줄바꿈만 다른 상태여서 내용 변경이 없어 커밋하지 않았다.

## 6-1. PR

- PR #492: https://github.com/tnestyle70/LostArk/pull/492 (base `main`, head `codex/colosseum-pvp-entry-0930`, 5커밋 + 이 링크 추가 커밋, 변경 파일 116개)
- 생성 직후 GitHub API 조회 결과: `mergeable=true`, `mergeable_state=clean`
- push: force 없이 새 브랜치로 올렸고 LFS 객체 14개(14MB) 업로드 성공

## 7. 알아둘 점

- **Server와 Client를 함께 빌드하고 재시작해야 한다.** protocol이 128이라 이전 버전 peer와는 접속이 거절된다.
- 콜로세움 서버 권위 PvP 규칙(경기 시작·이동 잠금·킬 집계·팀 기억)은 아직 없다. 서버가 정한 팀은 로딩 화면 표시용으로만 전달된다. 카운트다운과 창살 연출은 Client 로컬 표현이다.
- 새 리소스(PNG, 맵 모델 등)는 Git 비추적이며 Drive의 `CY_Resources`로 전달한다. 마하라카 맵 리소스는 팀원이 이미 갖고 있어서 제외했다.
- 게시 데이터는 재게시하지 않았다. 이 브랜치가 게시해 둔 콜로세움·베른 데이터를 그대로 포함한다.
