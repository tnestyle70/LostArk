# 쿠크 카드미로 — 문양 몬스터 4인 배정·랜덤 생성·개인 3마리 집계 RESULT

작성: 2026-09-08. 대응 계획서: `2026-09-08_KOUKU_CARD_MAZE_SUIT_TARGETS_PLAN.md`.

## 구현 완료 (저장소 반영)

| 구분 | 파일 | 내용 |
|---|---|---|
| Shared | `PacketType.h` | `NETWORK_PROTOCOL_VERSION` 68 → 69 |
| Shared | `PacketMessages.h` | `CARD_MAZE_ROLE {NONE, TELESCOPE, HUNTER}`, `PLAYER_SNAPSHOT.eCardMazeRole/eCardMazeSuit/iCardMazeKills/iCardMazeKillTarget` (`iMarioStage` 뒤) |
| Shared | `PacketMessages.cpp` | validate 불변식(HUNTER만 suit≠NONE, target≠0, kills≤target), write/read U8 4개, 실패 시 출력 보존 |
| 하네스 | `NetworkProtocolHarness.cpp` | 프로토콜 단언 69, `Test_CardMazeSnapshotProtocol` (전체 실행·`--mario-controls-only` 등록) |
| Server | `ServerPlayer.h` | 같은 4필드 + `Clear_CardMazeState()` |
| Server | `WorldBootstrap.h/.cpp` | `WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE`, `claimCardMazeTelescope <targetId>` 파서(Kouku + `requiresInteract` 상자만) |
| Server | `ServerTriggerSystem.cpp` | `Initialize`의 enabled trigger allow-list와 `Run_Action` 콜백 조건에 새 kind 추가 |
| Server | `KoukuSaydonLogicRuntime.h/.cpp` | `CKoukuCardMazeRuntime` — Plan/Register_Target/Commit/Abort/On_TargetHit/Remove_Player/Reset. 새 파일 없음 |
| Server | `GameRoom.h/.cpp` | `m_KoukuCardMaze`, `Begin_CardMaze`(망원경 청구 → 배정·통로 랜덤 생성, 부분 시작 없음), `Resolve_CardMazeHammerHit`(누른 지 12tick, 전방 120°·2.4m, raw 100, 첫 유효 타격에 `cardmiro.march.instance.from3/6/9/12` 재생), `Despawn_CardMazeTargets`/`Reset_CardMaze`, snapshot 채움, 세션 종료·Debug despawn-all 정리 |
| Client | `CombatHUDViewModel.h/.cpp` | `HUD_KOUKU_GIMMICK_STATE`에 4필드 복사 |
| Client | `Level_KakulSaydonArena.cpp` | `Render()`에 `[ TELESCOPE ]` / `HEART 1 / 3` 텍스트 (y 0.68) |
| Client | `KoukuSaydonPresentationPlayer.cpp` | `Card_Asset()`: 룰렛 카드 없을 때 HUNTER 문양을 같은 머리 위 카드로(하트·다이아 red, 스페이드·클로버 black) |
| publisher | `Publish-WorldGameplay.ps1` | `claimCardMazeTelescope` event 검증·직렬화 |
| Data | `MonsterCatalog.json` | `MONSTER_KOUKU_CARD_HEART/DIAMOND/CLUB/SPADE` — `CardMiro_Monster_*` 모델, scale 0.015(스페이드 0.01), yaw -90 |
| Data | `MonsterProfiles.json` | 같은 4행: HP 300, engage/move 최소값(정지), knockback 0, despawn 2500ms |
| Data | `LV_LUT_MIDNIGHTC_ED/SpawnGroups.world.json` | 잠자는 group `spawn.kouku.cardmaze.profiles`(프로파일 배포용, 트리거 참조 없음) rev 194 |
| Data | `LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` | `cardmaze.telescope` 상호작용 트리거 (0.28, -0.01, 1351.65), 반폭 1.5m |

기본값(상수): `KILL_TARGET 3`, `TARGETS_PER_SUIT 3`, `HAMMER_HIT_TICK_OFFSET 12`, `HAMMER_RANGE_M 2.4`, `HAMMER_HALF_ANGLE_COS 0.5`, `HAMMER_RAW_DAMAGE 100`, `CENTER_KEEPOUT_M 5`, `SPAWN_SPACING_M 3`, `PLAYER_KEEPOUT_M 4` — 모두 `CKoukuCardMazeRuntime`의 `static constexpr`.

## 자동 검증 (실행함)

- 앵커 치환 21개 파일 `--check` 유일 매칭 후 적용. 적용 결과가 구문검사한 임시 복사본과 SHA-256 동일(21/21).
- UTF-8 BOM 없음 유지, CRLF 유지. `WorldBootstrap.h/.cpp`, `ServerTriggerSystem.cpp`는 원래부터 LF 줄이 섞인 파일(4/20/27줄)이며 그 수 그대로.
- `cl /Zs`(산출물 없음): `PacketMessages.cpp`, `NetworkProtocolHarness.cpp`, `GameRoom.cpp`, `KoukuSaydonLogicRuntime.cpp`, `WorldBootstrap.cpp`, `ServerTriggerSystem.cpp`, `CombatHUDViewModel.cpp`, `Level_KakulSaydonArena.cpp`, `KoukuSaydonPresentationPlayer.cpp` 9개 exit 0.
- `Publish-WorldGameplay.ps1 -Mode Publish` 성공: `KAKULSAYDON_ARENA.worldbootstrap` 104행(telescope 포함), `KAKULSAYDON_ARENA.spawngroupsbootstrap` PROFILE 4행.
- `git diff --check` 깨끗.

## 적용 후 발견·수정 (2026-09-08 16:3x)

- 첫 빌드 후 Server가 시작 직후 종료: `World simulation failed to initialize. World=5, Status=Enabled trigger requires one supported action: cardmaze.telescope`. `CServerTriggerSystem::Initialize`의 enabled trigger allow-list에 `CLAIM_CARD_MAZE_TELESCOPE`가 빠져 있었다(bootstrap 파서·Run_Action·GameRoom 콜백은 있었음). `ServerTriggerSystem.cpp` 조건에 한 항목 추가로 수정, `cl /Zs` 통과. 그 이전 binary로 돌린 `Server.exe --contract-test`의 `[FAILURE]` 23개는 전부 KoukuSaydon 계열이며 같은 world init 실패의 결과다 — 재빌드 후 재실행이 필요하다.
- Client가 `127.0.0.1:7777`로 접속한 것은 이 PC의 Windows 사용자 환경변수 `LOSTARK_SERVER_HOST=127.0.0.1` 때문이다(`Client.vcxproj.user`의 192.168.0.4보다 우선). 로컬 Server를 띄우는 경우 정상이며 바꾸지 않았다. `Sync-TeamLanEndpoint.ps1` 결과 이 PC의 역할은 `client`, 팀 endpoint 192.168.0.4:7777은 `not-listening`.
- 실패 재현 방법: `Server.exe --bind-address 127.0.0.1`을 10초 띄워 listener·stderr 확인(자동, 종료 후 잔류 process 없음).

- 두 번째: 맵툴(Test)에서 쿠크 Area가 `Gameplay trigger event identity is invalid`로 열리지 않았다. Client `CWorldGameplayDocument`가 `claimCardMazeTelescope`를 모르는 타입으로 거부한 것. `WorldGameplayDocument.h/.cpp`(enum·파서·작성기·검증기·문자열 매핑), `MapTool.cpp`(Action 콤보 6번과 설명 섹션), `MainApp_SequenceViewer.cpp`(라벨)에 추가. `cl /Zs` 통과 후 적용.

## 미검증 (사용자)

- Visual Studio 빌드: Shared → Server → Client → NetworkProtocolHarness. protocol 69라 Server/Client 동시 빌드·재시작 필수.
- `NetworkProtocolHarness.exe` failures 0 확인.
- 런타임: Server+Client → 쿠크 진입 → F1 HUD mode `Card Maze` → F1 카드미로 이동 → 중앙 `G` → 텍스트·머리 위 카드·통로 병사 9마리 → `Q` 망치 → 첫 타격 세토 행진 → 3마리 처치 `3 / 3`.
- 화면 판정(병사 크기·위치·문양 카드 색)은 사용자 관찰이 정본.

## 후속 (이번 G 밖)

중앙 무적 구역, 망원경 전체보기 카메라, 완료자 관전 합류, 행진 접촉 피해, 성공 후 복귀, 4인 실제 검증 — 설계 검토서 G05~G08.
