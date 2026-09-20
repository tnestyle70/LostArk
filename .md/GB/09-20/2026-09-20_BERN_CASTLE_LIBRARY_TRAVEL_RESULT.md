# 베른 castle↔castle.2 / library↔library.2 왕복 이동 (2026-09-20)

## 현재 상태 한 줄

**코드와 테스트는 저장소에 반영했고, 데이터 파일 적용·게시·제품 빌드는 아직 하지 않았다.** 작업 시점에 VS(devenv)와 Server가 실행 중이었고, 그 상태에서는 데이터 파일을 편집하지 않고 빌드·게시를 하지 않기로 한 지시를 따랐다. 게임 화면에서 확인한 것은 하나도 없다.

## 요구 (사용자가 확정)

- `castle`을 밟으면 `castle.2` 자리로, `castle.2`를 밟으면 `castle` 자리로. `library`와 `library.2`도 같은 방식. 몇 번이든 왕복.
- 밟는 순간 화면이 어두워지고, 어두운 동안 플레이어가 이동하고, 도착하면 다시 밝아진다. 매번 같다.
- 반복 이동 방지: 도착 직후 서 있는 상자는 발동하지 않고, 한 번 나갔다 다시 들어와야 발동(권고안 2번, 베른 한정).

## 구현 내용

| 영역 | 파일 | 내용 |
|---|---|---|
| Shared | `Shared/Public/Network/PacketMessages.h` | `BERN_TRAVEL_FADE_OUT_MS`(300), `BERN_TRAVEL_HOLD_MS`(500) 상수만 추가. 프로토콜 94 그대로 |
| Server | `Server/Private/ServerTriggerSystem.cpp` | `AUTO_ENTRY_RULES`에 `{BERN, MOVE_PLAYER, "castle"}`, `"library"` 행(G 없이 밟자마자 발동). `Update_PlayerMotion`에 hold 구간(제자리 유지). `Run_Action`이 Bern movePlayer에 hold를 설정. `Evaluate_Entries`가 Bern에서만 방금 착지한 플레이어를 그 틱의 진입 발동에서 제외 |
| Server | `Server/Public/ServerPlayer.h` | `SERVER_TRIGGER_MOVE`에 `fHoldSeconds`, `fHeldSeconds` (기본 0 = 기존과 동일) |
| Server | `Server/Public/ServerTriggerSystem.h` | 착지 판정용 `m_TriggerMoveInFlight` |
| Server | `Server/Private/GameRoom_Replication.cpp` | hold 동안 스냅샷 이동 상태를 IDLE로(제자리에서 달리기 애니메이션이 나오지 않게) |
| Client | `Client/Private/SongCastGaugeView.cpp`, `Client/Public/SongCastGaugeView.h`, `Client/Private/MainApp.cpp` | 스퀘어홀 암전에 쓰는 전역 검은 화면을 Bern 레벨에서만 `TRIGGER_MOVE` 동안 300ms로 어둡게, 액션이 끝나면 0.5초에 걸쳐 밝게 |
| Test | `Server/Private/ServerGameplayContractTests_WorldTriggers.cpp` | 마지막에 Bern 왕복 테스트 블록 17개 검사 추가 |
| 문서 | `.md/GB/gotchas.md` | 예외 규칙과 함정 한 항목 |

타이밍: 서버는 액션이 시작되면 500ms 동안 제자리에 두고 그 뒤 이동한다. 클라이언트는 같은 스냅샷 흐름에서 액션을 보는 순간부터 300ms에 걸쳐 검게 만든다. 설계상 위치 변화는 화면이 완전히 어두워진 뒤 약 200ms 뒤에 온다(500ms - 300ms, 실측한 값이 아님). 이동 자체는 0.05초(게시기 최소값)다.

Kouku 아레나의 속도 기반 트리거 암전은 그대로이고, 새 암전은 `Get_CurrentLevelID()==BERN`일 때만 켜지므로 이중 암전은 없다. 스퀘어홀은 `SQUAREHOLE_SONG` 액션이라 `TRIGGER_MOVE` 조건과 겹치지 않는다.

## 데이터 (아직 적용 안 함)

`Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`의 네 트리거는 현재 `enabled:false, triggerOnce:true, events:[]` 그대로다.

적용할 내용은 후보 파일로 준비했다: `C:\Users\USER\.claude\jobs\46aea322\tmp\Gameplay.world.bern_travel.candidate.json`. 네 트리거에서 `enabled:true`, `triggerOnce:false`, `movePlayer` 이벤트 하나(짝 자리, 지속 0.05초, 호 높이 0)를 넣고 `revision`을 890에서 891로 올린다. 다른 배치는 바이트 단위로 그대로다.

| 트리거 | 이동 목표(x, y, z) | 서버 네비 영역 |
|---|---|---|
| castle | 53.5649986, 37.0986061, 367.517609 | Bern2 |
| castle.2 | 137.009995, 54.04319, -168.600006 | Bern |
| library | 41.7480011, 35.8277779, 167.568085 | Bern2 |
| library.2 | 110.569, 52.7690468, -169.77121 | Bern |

목표 Y는 짝 자리의 서버 네비 높이를 썼다(트리거 상자 Y와 최대 0.13m 차이). 네 자리 모두 걸을 수 있는 칸이다.

적용 스크립트: `C:\Users\USER\.claude\jobs\46aea322\tmp\apply_bern_travel_data.py`. `--check`는 저장소를 건드리지 않고, `--apply`는 devenv/Client/Server/MSBuild가 켜져 있으면 거부하고 파일 해시가 준비 중에 바뀌었으면 거부한다.

## 검증 (범위를 나눠서)

**파일에 적힌 것 (확인함)**
- 코드·테스트 파일 9개와 문서 2개(`gotchas.md`, 이 문서). 코드·테스트 9개는 앵커 패치 후 바이트 검사로 BOM 없음, 줄끝 CRLF 유지, UTF-8 유효를 확인했고 서버·공유 변경은 `git diff`로 내용을 검토했다.

**컴파일 (문법·타입 검사만)**
- `cl /Zs`로 변경한 Server 소스 3개(`ServerTriggerSystem.cpp`, `GameRoom_Replication.cpp`, `ServerGameplayContractTests_WorldTriggers.cpp`)와 `ServerPlayer.h`를 직접 초기화하는 `ServerGameplayContractTests_ValtanAudition.cpp`, `GameRoom_PlayerSimulation.cpp`, Client 소스 2개(`SongCastGaugeView.cpp`, `MainApp.cpp`) 모두 종료 코드 0.
- 이건 링크하지 않은 검사다. **제품 빌드(`Invoke-BuildAndRegression.ps1`)는 실행하지 않았다.**

**실제로 실행해 확인한 것**
- 저장소 밖 임시 하네스에서 실물 `ServerTriggerSystem.cpp`와 방금 추가한 테스트 블록을 그대로 컴파일·실행: **17개 검사 전부 통과**(밟자마자 발동, 대기 동안 제자리, 짝 자리 도착, 도착 직후 재발동 없음, 나갔다 들어오면 재발동, 왕복 반복, 다른 월드는 G 대기·hold 없음, Bern의 다른 id는 G 대기).
- 변이 검사: 도착 직후 발동 방지를 끄면 6개, hold를 끄면 1개, 자동 발동 규칙을 끄면 7개 검사가 실패한다. 즉 테스트가 결함을 실제로 잡는다.
- 게시기 `Publish-WorldGameplay.ps1 -Mode Validate`를 스크래치 루트에서 후보 파일로 실행: 종료 코드 0, `Validated BERN: 63 placements`. 지속 시간을 0.01로 망친 후보는 `movePlayer timing or arc is out of range: castle`로 거부됐다. 검증 전후 저장소 `Data` 항목 수(2056)와 월드 파일 해시가 그대로임을 확인했다.

**확인하지 못한 것**
- 제품 빌드, `Server.exe --contract-test` 전체 실행(기존 테스트 행의 회귀 포함). 다른 월드 동작은 코드 상 변경이 없음(hold 0, `landed`는 Bern에서만, 새 규칙 행은 Bern 한정)만 확인했고 기존 행을 실제로 돌리지는 못했다.
- 데이터 적용, `Publish-WorldGameplay.ps1 -Mode Publish`, 베른 worldbootstrap 재생성.
- 게임 안 동작 전부: 암전이 실제로 이동 전에 완전히 어두워지는지, 540m 이동에서 카메라와 캐릭터가 어떻게 보이는지, 밝아지는 시점, 짝 자리 도착 후 다시 밟기. 화면 판정은 사용자 몫이다.

## 이어서 할 일 (VS·Client·Server를 모두 끈 뒤 이 순서로)

```powershell
python C:\Users\USER\.claude\jobs\46aea322\tmp\apply_bern_travel_data.py --apply
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
Server\Bin\Debug\Server.exe --contract-test
```

그 뒤 Server 재시작, Client 재실행.

## 사용자가 눈으로 확인할 항목

1. 베른에서 `castle` 상자를 밟자마자 화면이 어두워지고, 어두운 동안 `castle.2` 위치(Bern2 영역)로 이동한 뒤 밝아지는지. 이동하는 모습이 화면에 보이면 안 된다.
2. 도착한 뒤 그 자리에서 바로 되돌려 보내지지 않는지, 상자 밖으로 나갔다 다시 들어가면 `castle`로 돌아가는지. `library`와 `library.2`도 같은지.
3. 이동을 여러 번 반복해도 같은지.
4. 어두워지기 전에 제자리에서 달리기 동작이 나오지 않는지.
5. 베른의 스퀘어홀(M키)과 쿠크 아레나 트리거 암전이 이전과 같은지.
6. `G` 키 아이콘이 이 네 상자 위에 뜨지 않는지.

## 남은 위험과 메모

- 이동 거리가 540m라서 클라이언트 캐릭터·카메라가 큰 위치 변화를 어떻게 받아들이는지는 실행해 봐야 안다. 발탄·쿠크에도 장거리 트리거 이동이 있지만 그 화면을 이번에 확인한 것은 아니고, 0.05초에 이동하는 경우는 코드에 없던 조합이다.
- 이 두 쌍은 진입 위치가 상자 중심이라, 도착 직후에는 발동하지 않고 상자를 벗어났다 다시 들어와야 한다(합의된 동작).
- 이전 세션의 미커밋 변경(마리오, 컷신, 앵콜, Bern2 네비 등)과 이 작업은 같은 작업 트리에 섞여 있다. 커밋하지 않았다.
