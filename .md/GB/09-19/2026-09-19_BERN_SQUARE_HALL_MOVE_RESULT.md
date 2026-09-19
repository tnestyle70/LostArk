# 베른 스퀘어홀 이동(노래 → 암전 → 착지) RESULT

작성일: 2026-09-19
브랜치: `feature/maharaka-island-level`
범위: 베른에서 M 키 지도의 스퀘어홀을 누르면 노래가 끝나는 시점에 화면이 어두워지고, 어두운 동안 플레이어가 스퀘어홀 위치로 실제로 이동한다.

이 문서의 표는 소스를 읽고, 데이터를 파싱하고, 구문 검사(`cl /Zs`)를 한 결과다. **빌드·링크·게임 실행은 하지 않았다.** 화면에서의 암전 타이밍과 이동 결과는 사용자가 직접 확인해야 한다.

## 0. 가정과 확정하지 못한 것

1. **착지 좌표는 원작의 확정 착지점이 아니다.** 지도에 그려지는 스퀘어홀 아이콘 세 곳(`Data/UI/WorldMap/WorldMapSquareHoles.json`)의 좌표를 그대로 썼다. 이 값은 팀 문서(`.md/TJ/09-17/2026-09-17_월드맵창_PLAN.md`)대로 원작 월드맵 캡처에서 아이콘을 템플릿 매칭해 라벨 적합식으로 cm에 환산한 것이며 잔차는 0.5px 미만이다. 조화의 광장은 발탄 입장 트리거(144.02, −147.13)에서 약 3m다. 원작 DB(`EFTable`)에 실제 착지점이 있는지는 이 PC에 추출본이 없어(`D:/ClaudeWork/Extracted`) 확인하지 못했다.
2. **노래 길이 정본은 `SQUAREHOLE_SONG_DURATION_MS = 3000`이다.** Server의 잠금과 Client의 게이지가 이미 함께 쓰던 값이다. 실제 노래 wav 길이는 이 PC에 파일이 없어(`Client/Bin/Resources/Sound/UI/SquareHole` 없음) 재보지 못했다. wav가 3초와 다르면 이 상수 하나만 바꾸면 Server 잠금·게이지·암전 시작 시점이 함께 따라간다.
3. **프로토콜 번호는 바꾸지 않았다(94 유지).** 패킷 형식은 그대로이고 Shared 상수 두 개를 추가했다. 다만 잠금 길이가 3.0초에서 3.4초로 늘었으므로 Server와 Client를 같은 소스로 빌드해야 한다.
4. 착지 데이터는 **Bern 월드 데이터를 다시 게시해야** Server가 읽는다. 게시 전에는 스퀘어홀을 눌러도 Server가 요청을 거절해서 노래도 나오지 않는다(아래 6절).

## 1. 원래 왜 이동하지 않았나 (확인 사실)

- `Server/Private/GameRoom_PlayerCommands.cpp`의 `Handle_UseSquareHole`(변경 전 633~637행)이 홀 번호를 `(void)useSquareHole;`로 버리고 주석에 "the teleport itself is not implemented"라고 적혀 있었다. 노래 액션(`SQUAREHOLE_SONG`)만 걸었다.
- `Server/Private/GameRoom_PlayerSimulation.cpp`의 `Update_Players`는 `SQUAREHOLE_SONG_TICKS`가 지나면 액션을 `NONE`으로 풀기만 했다.
- `Shared/Public/Network/PacketMessages.h` 주석도 "the Server only uses it to admit the request … because the teleport itself is not implemented yet"라고 되어 있었다.
- Client는 목적지를 몰라도 되는 구조다. `WorldMapWindowView` → `Take_SquareHoleRequest` → `CPlayerController::Request_UseSquareHole` → `C2S_USE_SQUAREHOLE`까지는 이미 연결되어 있었다. 끊긴 곳은 서버의 "이동" 단계뿐이다.
- 목적지 데이터도 서버에 없었다. 홀 좌표는 Client UI 문서에만 있었다.

## 2. 바꾼 것

| 파일 | 내용 |
|---|---|
| `Shared/Public/Network/PacketMessages.h` | 주석을 실제 계약으로 교정하고 `SQUAREHOLE_BLACKOUT_FADE_MS = 600`, `SQUAREHOLE_BLACKOUT_HOLD_MS = 400` 추가 |
| `Server/Private/GameRoom_Internal.h` | `SQUAREHOLE_LOCK_TICKS`(노래 90틱 + 검은 화면 유지 12틱 = 102틱) 추가 |
| `Server/Public/ServerPlayer.h` | `iSquareHoleId`(노래 중인 홀 번호, 0이면 없음) |
| `Server/Public/GameRoom.h` | `Resolve_SquareHoleDestination`, `Finish_SquareHoleSong` 선언 |
| `Server/Private/GameRoom_PlayerCommands.cpp` | `Handle_UseSquareHole`이 시작 전에 착지점을 검증하고 홀 번호를 저장. 위 두 함수 구현 |
| `Server/Private/GameRoom_PlayerSimulation.cpp` | 잠금이 끝나는 틱에 착지, 노래가 다른 이유로 끝났으면 홀 번호 폐기 |
| `Client/Public/SongCastGaugeView.h`, `Client/Private/SongCastGaugeView.cpp` | 화면 암전(전체 화면 검정 슬롯) 추가. 캡션은 어두워지면 함께 숨김 |
| `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` | 비활성 `triggerBox` 세 개 `squarehole.1~3`(단일 `movePlayer` 목적지) 추가, `revision` 552 → 553 |
| `Server/Private/ServerGameplayContractTests_DebugTeleport.cpp` | 위 동작의 계약 단언 추가(실행하지 않음) |
| `.md/TEAM/AREA_DATA_LAYER_GUIDE.md` | 베른 행에 `squarehole.<n>` 데이터 계약 한 줄 |

새 파일과 프로젝트/필터 변경은 없다. 암전은 새 문서 없이 쿠크 트리거 이동이 이미 쓰는 `Data/UI/KakulFade/KakulFadeUI.json`의 전체 화면 검정 슬롯을 재사용했다.

착지 데이터 세 행(높이는 게시된 베른 네비에서 각 지점이 속한 셀의 높이를 읽었다):

| id | 이름 | x | y | z |
|---|---|---|---|---|
| `squarehole.1` | 조화의 광장 | 145.65 | 50.13 | −149.9 |
| `squarehole.2` | 상업 지구 | 224.72 | 43.73 | −56.92 |
| `squarehole.3` | 제작 지구 | 52.25 | 42.26 | −85.55 |

## 3. 타이밍이 실제로 어떻게 이어지나 (Server 30Hz 틱 기준)

0. 사용자가 M → 스퀘어홀 → 확인. Client 창이 닫히고 `C2S_USE_SQUAREHOLE(홀 번호)`가 나간다.
1. Server가 착지점을 먼저 확인한다. 통과해야 `SQUAREHOLE_SONG` 액션을 걸고 시작 틱을 기록한다(노래 소리·게이지·연주 모션은 기존 그대로).
2. 노래 시작 후 2.4초(72틱)에 Client가 화면을 검게 만들기 시작한다. 0.6초에 걸쳐 진행되어 노래가 끝나는 3.0초에 완전히 어두워진다.
3. 3.0~3.4초는 화면이 검은 채로 유지된다. Server는 액션을 유지한 채 3.4초(102틱)에 착지를 확정한다.
4. 그 틱에 Server가 위치를 스퀘어홀로 바꾸고 액션을 푼다. 위치와 "액션 종료"가 같은 스냅샷으로 Client에 간다.
5. Client는 액션이 끝난 것을 보면 0.5초에 걸쳐 밝아진다.

검은 화면 유지 0.4초는 스냅샷 지연 여유다. Client의 암전은 서버 틱으로 시작을 정하고 이후엔 프레임 시간으로 진행하므로, 스냅샷이 잠깐 끊겨도 어중간하게 어두운 채 위치가 바뀌지 않는다.

## 4. 예외 처리

- **시작 조건**: 살아 있고, 넉백·패턴 속박·탑승 중이 아니며, 액션이 `NONE`이어야 한다. 노래 중 연타는 액션이 이미 걸려 있어 무시된다.
- **목적지 없음/걸을 수 없음/막힘**: `Validate_DebugTeleportDestination`(GateProgress가 파티 이동에 쓰는 것과 같은 걷기·높이(1m)·NPC/정적 충돌 검사)를 통과하지 못하면 노래도 시작하지 않는다. 노래만 나오고 이동하지 않는 상태를 다시 만들지 않기 위해서다. 이유는 서버 상태 문자열에 남는다.
- **노래 중 사망·피격으로 액션이 바뀜**: 액션이 `SQUAREHOLE_SONG`가 아닌 첫 틱에 홀 번호를 폐기한다. 나중에 뒤늦게 이동하지 않는다.
- **착지 시점에 그 자리가 막혀 있음**: 다시 검증해서 실패하면 플레이어는 제자리에 남고 잠금만 풀린다.
- **노래 중 이동·스킬 명령**: 액션이 `NONE`이 아니므로 기존 규칙대로 무시된다(콤보 버퍼 대상도 아님). 착지 시 `Reset_PlayerForDebugTeleport`가 남은 이동 목표와 대기 명령도 지운다.
- **다른 플레이어**: 각자 자기 홀 번호로 자기만 이동한다. 이 충돌 검사는 플레이어끼리는 막지 않으므로 여러 명이 같은 스퀘어홀에 도착할 수 있다.
- **접속 끊김**: 플레이어 상태가 사라지므로 남는 것이 없다. **월드 전환**: 새 방의 입장 절차가 플레이어 상태를 새로 만들므로(`GameRoom_Admission.cpp`의 `STAGED_PLAYER_ENTRY entry{}` → `Stage_PlayerEntry`) 홀 번호가 넘어가지 않는다.
- **Client 쪽**: 액션이 끝나거나 플레이어 정보가 없어지면(월드 전환 등) 암전은 0.5초에 걸쳐 걷힌다.
- **거절**: Client는 요청이 거절되어도 화면에 아무것도 띄우지 않는다(지금은 서버 상태 문자열로만 남는다).

## 5. 검증한 것과 하지 않은 것

| 항목 | 티어 | 내용 |
|---|---|---|
| 원인 | 소스를 읽어 확인 | 1절의 세 곳 |
| M 키 → 요청 전송 경로 | 소스를 읽어 확인 (변경 없음) | `WorldMapWindowView.cpp:754-760`, `MainApp.cpp:5102-5110`, `PlayerController.cpp:982-998` |
| 노래 재생·게이지 | 소스 diff로 확인 (기존 그대로) | `SongCastGaugeView.cpp`의 소리 재생·채움 계산은 바꾸지 않았다. wav는 이 PC에 없어 소리는 확인 불가 |
| 노래 끝나는 시점에 암전 | 소스를 읽어 확인, 구문 검사만 통과 | 시작 72틱, 0.6초, 완료 = 노래 끝 |
| 암전 후 그 위치로 이동 | 소스를 읽어 확인, 구문 검사만 통과 | `Handle_UseSquareHole` → `Resolve_SquareHoleDestination` → `Update_Players` → `Finish_SquareHoleSong` |
| 착지 데이터 | 데이터 확인 | JSON 파싱, 58개 배치(기존 55 + 3), ID 중복 0, `Publish-WorldGameplay.ps1 -Mode Validate` 통과(읽기 전용) |
| 세 지점이 걸을 수 있는 곳인가 | 데이터 확인 | 게시된 `LV_BER_BERNCASTLE.Bern.navgrid`(09-19 17:16)의 해당 셀이 모두 걷기 가능, 높이 50.13/43.73/42.26 |
| NPC 몸체와 겹치는가 | 데이터 확인 | 합산 반경 0.901m, 가장 가까운 NPC 1.9m / 13.5m / 10.4m |
| C++ 컴파일 | 구문 검사만 통과 | `cl /Zs` Server 3개 파일(Debug·Release), Client 1개 파일(Debug·Release) 모두 exit=0. 링크는 안 했다 |
| 추가한 서버 테스트 | **실행 안 함** | 재게시된 `BERN.worldbootstrap`이 있어야 통과한다 |
| 화면의 암전 타이밍·이동·소리 | **게임에서 실행 안 함** | 사용자가 직접 확인해야 한다 |

## 6. 지금 상태와 남은 작업

- 지금은 VS가 켜져 있어 빌드와 publisher를 실행하지 않았다. **소스와 데이터만 바뀌었고 실행 중인 Server/Client에는 아무 영향이 없다.**
- 코드만 빌드하고 데이터를 게시하지 않으면 Bern에서 스퀘어홀을 눌러도 Server가 "squarehole.N 없음"으로 거절해 아무 일도 일어나지 않는다(노래도 없음). 반대로 데이터만 게시하고 코드를 빌드하지 않으면 예전처럼 노래만 나온다.

VS를 끈 뒤:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

첫 명령은 모든 월드를 현재 원본 기준으로 다시 게시하므로 다른 작업 중인 월드 변경도 함께 반영된다. 그다음 Server를 재시작하고 Client로 베른 → M → 스퀘어홀 → 확인을 눌러 본다. 추가한 서버 테스트는 사용자가 원하면 `Server.exe --contract-test`로 실행할 수 있다.

## 7. 사용자가 결정할 것

- 착지점이 캡처 환산값이어도 되는가, 원작의 정확한 착지점이 필요한가. 조정은 MapTool World Gameplay에서 `squarehole.N` 위치를 바꾸고 다시 게시하면 된다.
- 암전 길이(0.6초 / 유지 0.4초 / 밝아짐 0.5초)와 잠금이 3.0초에서 3.4초로 늘어난 것.
- 노래 wav 길이가 3초가 아니면 `SQUAREHOLE_SONG_DURATION_MS`를 wav 길이에 맞출지.
- 운임(지도의 38실링)은 여전히 차감하지 않는다. 이번 요청 범위 밖이라 그대로 뒀다.
