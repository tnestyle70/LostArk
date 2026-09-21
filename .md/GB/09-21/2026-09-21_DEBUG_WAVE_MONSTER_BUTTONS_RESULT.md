# Debug 웨이브 몬스터 트리거 억제와 F1 재소환 버튼 결과

## G00. 요청과 범위

- Kouku `Book1_Monsters`(`spawn.kouku.book1`, max alive 22), `Book2_Monsters`(`spawn.kouku.book2`, 15)와 Valtan `Stage_1`(`spawn.valtan.stage01`, 10), `Stage_2`(`spawn.valtan.stage03`, 10)를 **Debug Server에서는 밟아도 시작하지 않게** 하고 **Release Server는 지금처럼** 두었다. Debug/Release 구분은 컴파일 시 `_DEBUG`이며 실행 중 전환하는 설정이 아니다.
- Debug F1에 버튼을 추가했다. 쿠크는 `KoukuSaydon Arena` → `Bingo Board` 구역의 `Normal Monster 1/2`, 발탄은 `KoukuSaydon Arena` 바로 아래의 `Valtan Arena` 헤더(발탄 아레나 안에서만 표시)의 같은 이름 버튼이다. 버튼은 그룹의 살아 있는 monster를 제거하고 그룹을 처음부터 다시 시작한다.
- 이번에 **하지 않은 것**(범위 밖): `Stage_MiniBoss_Spawn`(`spawn.valtan.stage02.miniboss`)은 Debug에서도 밟으면 시작한다. `Stage_3`(G로 움직이는 이동 상자, `Stage_2`와 다른 기능), `Stage_Boss`, `Stage_Boss_ArenaEntry`, 다른 월드의 트리거, 월드·spawn group 데이터는 바꾸지 않았다.

## G01. 변경한 파일

커밋 `a6020164`(Shared + Server + 테스트), `9c473666`(Client), 이 문서를 포함한 문서 커밋으로 나눴다.

| 영역 | 파일 | 내용 |
|---|---|---|
| Shared | `PacketType.h` | `C2S_DEBUG_RESUMMON_WAVE_MONSTERS`를 enum 끝에 추가, `NETWORK_PROTOCOL_VERSION` 98 → 99 |
| Shared | `PacketMessages.h/.cpp` | `WAVE_MONSTER_BUTTON`(1/2), 요청 구조체와 Write/Read(7 byte: sequence 4 + world 2 + button 1) |
| Server | `ServerTriggerSystem.h/.cpp` | `WAVE_MONSTER_BUTTON_ROW` 표, `Set_SuppressWaveMonsterTriggers`, `Find_WaveMonsterButton`, `Is_WaveMonsterTrigger`, 진입·G 경로의 억제 |
| Server | `GameRoom.cpp` | Debug에서만 `Set_SuppressWaveMonsterTriggers(true)` 호출, 명령 dispatch |
| Server | `GameRoom_PartyWorld.cpp`, `GameRoom.h` | `Handle_DebugResummonWaveMonsters` |
| Server | `RoomCommand.h`, `ServerApp.cpp` | 새 room command와 payload 검사 |
| Server 테스트 | `ServerGameplayContractTests_SpawnGroups.cpp/_Runner.cpp/_Runner.h` | `Run_WaveMonsterButtons` (기존 파일에 추가, 새 파일·vcxproj 변경 없음) |
| Client | `NetworkManager`, `IPlayerCommandSink`(`PlayerCommandSink.h`), `NetworkPlayerCommandSink`, `PlayerController` | `CPlayerController -> IPlayerCommandSink -> CNetworkManager` 송신 체인 |
| Client | `MainApp.cpp/.h` | 쿠크 Bingo Board 구역 버튼 2개, `RenderValtanArenaControls`와 버튼 2개, 툴팁 |
| 문서 | `CLAUDE.md`, `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`(4.1.1), `gotchas.md`, 이 문서 | protocol 표기와 Debug 동작 |

## G02. 경로별 동작

| 상황 | Debug Server | Release Server |
|---|---|---|
| 네 상자(Book1/Book2/Stage_1/Stage_2)를 밟음 | 그룹 시작 안 함, G 안내도 없음 | 이전과 동일하게 시작 |
| 네 상자에서 G | 실행 안 함 | 이전과 동일 |
| `Normal Monster 1/2` 요청 | 그 그룹의 monster 제거 → `Reset_Group` → `Activate` (첫 wave부터) | 무시(아무것도 안 바뀜) |
| `Stage_MiniBoss_Spawn`, `Stage_3`, `Stage_Boss`, 다른 월드 상자 | 이전과 동일 | 이전과 동일 |
| F1 World Playback(`Debug_Activate`)으로 상자 ID 실행 | 이전과 동일(`_DEBUG`에서만 동작) | `DISABLED`(기존) |

재소환 요청의 거절 조건(모두 아무것도 바꾸지 않고 Server 콘솔 `[WaveMonsters]` 줄과 `m_strStatus`에만 남는다):

| 조건 | 결과 |
|---|---|
| 요청 world가 이 방과 다름, 이 world에 그 버튼이 없음, 이 session에 player 없음 | 거절 |
| Valtan 패턴 audition(`m_ValtanTimelineAudition`)이 진행 중 | 거절 (이때 상자 진입도 평가하지 않는 기존 Debug 규칙과 같은 조건) |
| spawn group 정의가 bootstrap에 없음 | 거절 |
| 그 외 | 그 group의 `MONSTER`만 제거(다른 group·NPC·boss는 그대로), combat object lifecycle 방송, `Reset_Group` + `Activate` |

## G03. 실제로 확인한 것

제품 MSBuild 빌드와 Client 링크는 **하지 않았다**(사용자의 Visual Studio와 Server가 실행 중이었고 그 프로세스는 종료하지 않았다). 아래는 저장소 밖(`.claude/jobs/.../tmp`)에서 같은 소스를 `cl`로 컴파일·링크한 결과이며 저장소에는 산출물을 쓰지 않았다.

| 검증 | 실제 출력 |
|---|---|
| Server+Shared 96개 TU를 Debug/Release로 각각 컴파일 | 두 구성 모두 `EXITCODE=0`, 오류 0, 경고 0 |
| 변경한 Client 4개 TU(MainApp, NetworkManager, NetworkPlayerCommandSink, PlayerController) `cl /Zs` | Debug/Release 모두 `EXITCODE=0`, 오류 0. 일부러 틀린 코드를 넣은 대조군은 `EXITCODE=2` (문법 검사만이며 링크·실행은 아님) |
| `Run_WaveMonsterButtons` 단독 실행 (실제 `Server/Bin/DataFiles`) | Debug 53 PASS / 0 FAILURE, Release 32 PASS / 0 FAILURE |
| 변경 전(HEAD)과 현재 트리를 각각 Release 옵션으로 컴파일한 probe로 6개 월드의 trigger 상자 61개(진입 실행이 있는 상자 30개 포함)를 밟았을 때의 동작 출력 비교 (억제 flag 미설정) | 68줄, 바이트 단위 동일 |

`Run_WaveMonsterButtons`가 실제로 실패를 잡는지 확인하려고 소스를 일부러 되돌린 변형을 만들어 같은 테스트를 돌렸다(원본 소스는 바꾸지 않고 임시 사본에서만 수행).

| 변형 | 결과 |
|---|---|
| 억제 판정이 항상 false | 8 FAILURE |
| 억제 판정이 그 월드의 모든 상자와 맞음(mini boss 포함) | 3 FAILURE |
| 재소환이 살아 있는 monster를 제거하지 않음 | 4 FAILURE |
| 재소환이 `Reset_Group`을 건너뜀 | 3 FAILURE |
| 월드/session 검사 제거 | 3 FAILURE |
| Valtan 2번 버튼이 잘못된 그룹에 연결 | 5 FAILURE |
| Valtan audition 검사 제거 | 1 FAILURE |
| G 경로(`Activate_Here`)의 억제 제거 | 2 FAILURE |
| G 안내(`Activate_Interact`)의 억제 제거 | 2 FAILURE |
| 진입 경로(`Evaluate_Entries`)의 억제 제거 | 8 FAILURE |
| Release에서도 억제 flag를 켬 (Release 구성) | 4 FAILURE |

## G04. 가정과 미확인

가정(사용자가 확정하지 않은 것):

1. 발탄 `Normal Monster` 버튼은 `Valtan Arena` 헤더를 새로 만들고 발탄 아레나(`LEVEL::VALTAN_ARENA`) 안에서만 보이게 했다. 쿠크 헤더가 쿠크 아레나에서만 열리는 방식과 맞췄다.
2. `Stage_MiniBoss_Spawn` 버튼은 만들지 않았다(사용자가 답하지 않음). Debug에서 이 상자는 계속 밟으면 시작한다.
3. 재소환에는 결과 메시지가 없다(기존 Bingo 버튼과 같은 방식). 거절 사유는 Server 콘솔에서만 볼 수 있고 F1은 "보냈다"만 안다.
4. 방에 있는 어느 player든 누를 수 있다. 쿠크 raid(Complete Play) 진행 중 차단이나 발탄 pattern flow 중 차단은 넣지 않았다(발탄은 timeline audition 중에만 거절).
5. 제거 대상에는 죽은 뒤 아직 despawn되지 않은 그 group의 monster도 포함된다.

확인하지 못한 것:

- 게임 안에서 monster가 실제로 나오는지, F1 화면 모양, 툴팁 표시. 사용자가 직접 확인해야 한다.
- Release 구성의 실제 실행 동작. 위 표의 Release 결과는 Release 옵션으로 컴파일한 테스트 실행이다.
- 제품 MSBuild 빌드, Client 링크. 변경한 Client TU 4개 외에 `MainApp.h`·`PlayerController.h`·`PlayerCommandSink.h`를 include하는 다른 Client TU는 컴파일하지 않았다.
- `Tools/NetworkProtocolHarness`는 실행하지 않았다. 이 하네스는 소스에 `NETWORK_PROTOCOL_VERSION == 84u` 검사가 남아 있어(예: 2285행 부근) 이번 변경과 무관하게 이미 현재 버전(98)과 맞지 않는다.

## G05. protocol 번호 주의

이 브랜치의 protocol 99(`C2S_DEBUG_RESUMMON_WAVE_MONSTERS`)는 `origin/main`의 99(`f291f886`, 무적 구역 연출 펄스)와 **다른 wire**다. 두 변경은 같은 번호를 다른 의미로 쓴다. main을 병합하면 `Shared/Public/Network/PacketType.h`의 버전 줄이 충돌하며 병합 결과의 올바른 번호는 **100**이다. 그때 `CLAUDE.md`, 팀 사용서 4.1.1, `gotchas.md`의 "protocol 99" 문구와 `Run_WaveMonsterButtons`의 버전 검사(`NETWORK_PROTOCOL_VERSION >= 99u`)도 함께 확인한다. 이번 작업에서는 main을 병합하지 않았고 번호도 바꾸지 않았다. 서로 다른 protocol의 Server/Client는 섞어 실행하지 않는다.

## G06. 사용자가 직접 확인할 순서

Server와 Client를 같은 소스로 함께 빌드·재시작한다(protocol이 바뀌었다).

1. Debug 쿠크: Lobby → KoukuSaydon 진입, `Book1_Monsters` 상자를 밟는다 → monster가 나오지 않아야 한다. F1 → `KoukuSaydon Arena` → `Bingo Board`의 `Normal Monster 1`을 누른다 → Server 콘솔에 `[WaveMonsters] Wave monsters re-summoned: spawn.kouku.book1`이 찍히고 monster가 나온다. 한 번 더 누르면 남은 monster가 사라지고 새로 나와야 한다. `Normal Monster 2`는 `spawn.kouku.book2`다.
2. Debug 발탄: Valtan 진입, F1의 `Valtan Arena` 헤더에서 `Normal Monster 1`(`spawn.valtan.stage01`), `2`(`spawn.valtan.stage03`)를 같은 방식으로 확인한다. `Stage_1`, `Stage_2` 상자는 밟아도 나오지 않아야 하고 `Stage_MiniBoss_Spawn` 상자는 밟으면 나와야 한다.
3. Release: Book1/Book2/Stage_1/Stage_2 상자를 밟으면 예전처럼 monster가 나와야 하고 F1 버튼은 없다.
