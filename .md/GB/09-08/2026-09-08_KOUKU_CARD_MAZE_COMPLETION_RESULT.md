# 쿠크 카드미로 진행·관전 적용 결과

후속 변경: 같은 날짜 `2026-09-08_KOUKU_CARD_MAZE_FLOOR_MARKERS_RESULT.md`에서 발밑/출구 문양을 실제 등록하고 Debug 망원경 담당 문양 겸임을 제거했다. 아래 이전 단계의 미등록 이펙트·혼자 겸임 설명보다 후속 결과를 우선한다.

2026-09-08. 기존 Claude 작업과 다른 dirty 변경은 보존했고 자동 stage/commit/push는 하지 않았다.

## 구현 상태

| 사용자 항목 | 현재 반영 |
|---|---|
| 망치 한 방·즉시 사라짐 | 자기 문양 등록 목표만 타격 허용. hit adapter가 남은 HP 전체를 소비하고 GameRoom이 즉시 despawn. 3스택 전에는 다음 목표 1마리 보충 |
| 36개 행진 반복 | from3/6/9/12 각각 lane1..9. WorldSequence의 선형 좌표/지연/기간을 worldbootstrap v10에 배포, 서버 시작 tick과 51,722ms 주기를 snapshot으로 복제 |
| 세토 접촉 | 이전/현재 플레이어와 세토 경로의 swept 상대 이동 판정. 초기 합산 반경 1.2m, 높이 차 2m 미만, 재접촉 유예 30tick. 본인 스택·출구 취소 후 목표 교체 |
| 중앙 안전 | 중앙 (0.28, -0.01, 1351.65) XZ 반경 5m에서는 세토 접촉 무시 |
| 3스택 개인 출구 | 중앙과 경로로 연결되는 랜덤 보행 통로에 서버 좌표 확정. 반경 1m, 높이 차 1m 미만의 본인 진입만 허용 |
| 중앙 이동 암전 | 36tick=1.2초. 0..12 fade-in, 12..24 검정, 24..36 fade-out. 18tick 서버 위치 commit, 이동 중 입력 차단 |
| 탈출자 망원경 공유 | 최초 담당과 탈출자가 중앙 상자를 망치로 가격하면 각자의 관전 flag 토글. Debug 재타격 재시작 제거 |
| 전원 완료 | 생존 참가자의 개인 탈출과 중앙 집결 및 암전 종료 확인 후 2관문으로 동일 암전 이동. 전멸 시 미로 상태 정리 |
| 기본 카메라 | `cardmaze.follow`: follow offset (0,14,-10), FoV 50. 첨부 구도를 참고한 초깃값 |
| 망원경 카메라 | `cardmaze.telescope`: eye (0.28,62,1315.65), center look-at, FoV 50, priority 1000. 관전 flag에만 반응 |

## 사용자가 나중에 지정할 항목

- **출구 효과는 아직 없다.** 서버 출구 판정과 Effect GROUP 소비 경로는 구현되어 있고 현재는 HUD `EXIT (x,z)` 좌표만 보인다. 기존 Effect catalog에서 `cardmaze.exit.heart`, `cardmaze.exit.spade`, `cardmaze.exit.club`, `cardmaze.exit.diamond`를 연결·배포 후 다시 시작한다. alias가 없으면 다른 이펙트로 대체하지 않는다.
- 최종 복귀는 사용자 답변대로 기존 2관문 좌표 (3.38,10.56,323.92)를 사용했다. MapTool → World Gameplay → `cardmaze.return`의 movePlayer 목적지를 나중에 수정한다. 설정 전용 행이므로 disabled를 유지한다. 수정 후 WorldGameplay publish 및 Server 재시작.
- 두 카메라는 MapTool → Camera에서 해당 shot ID로 조정하고 Save Shots → MapAuthoring publish. 실제 첨부 화면과의 일치 판정은 사용자에게 남긴다.

## 자동 검증 증거

- Product Debug Engine → Shared → Server → Client 컴파일/링크/SDK·DLL·shader 배포 성공. 마지막 기록 `out/BuildPipeline/runs/20260908T092611809Z-debug-product.json`.
- 이후 중앙 밖으로 나간 탈출자에게도 접촉 초기화가 적용되도록 Server 2개 CPP를 보정하고 Server Debug 최소 재컴파일 및 동일 카드미로 계약 검사를 다시 통과했다. 안전 보장은 중앙 반경이며 탈출 flag 자체가 전역 무적은 아니다.
- 기존 NetworkProtocolHarness를 확장하고 `--mario-controls-only` 실행 exit 0. protocol70 문양·출구·행진/암전 clock 왕복, unknown flags, 불완전 clock, 잘린 snapshot의 기존 상태 보존 통과.
- 기존 Server 계약 검사에 `--card-maze-contract-test` 범위를 추가하고 실행 exit 0, failures 0. 실제 publish된 36lane/bootstrap/nav 로드, 기존 2관문 목적지 보행면, 네 플레이어 3문양/동시 1마리, HP300·방어99999 한 방 처치, 중복 집계 차단, 토글, 출구 취소, 5m 안전 경계, 집결 및 미완료 암전 차단, reset 검사 통과.
- WorldGameplay Validate/Publish 성공. Kouku bootstrap v10 revision8470, placements105, sequences131, lanes36. 생성 bootstrap 직접 편집 안 함.
- MapAuthoring Area Validate/Publish 성공: placements3231, files8. camerashots runtime revision70 확인. Python을 현재 PowerShell PATH에 둔 뒤 publisher 실행. 앞서 별도 powershell 자식에서 PATH를 못 찾은 실패는 최종 성공으로 해소.
- 변경 JSON은 publisher strict validation 및 JSON parse, 프로젝트 XML은 parse, `git diff --check` 통과. 기존 코드 페이지/PDB 경고는 남아 있으며 warning-free라고 판정하지 않는다.
- 독립 read-only 검토의 초기 한 방 처치/Effect consumer 누락 지적은 최신 실제 파일 및 위 실행 검사로 반증했고 검토자가 철회했다. 재검토한 범위에서 추가 확인된 blocker 없음.

## 아직 수행하지 않은 확인

Client/UI를 에이전트가 실행·조작·캡처하지 않았다. 실제 4인 연결, 망치 입력, 세토 시각 위치와 접촉 체감, 반복 경계, 두 카메라 구도, 암전 체감과 지연 네트워크에서의 전원 이동은 사용자 런타임 확인이 남았다. 서버 콘솔 계약 검사 성공은 이 화면 검증을 대신하지 않는다.

테스트 순서: protocol70 Server/Client 재시작 → Lobby KoukuSaydon → F1 카드미로 진입 → MAZE 망치로 중앙 상자 가격 → 담당 관전 토글 → 자기 문양 처치 3회(세토 접촉 시 초기화 확인) → HUD 좌표의 개인 출구 진입 → 중앙 망원경 공유 → 마지막 생존 사냥꾼 탈출 후 2관문 복귀 확인. Debug 혼자서는 담당이 문양도 받으므로 상자를 다시 쳐 관전을 끄고 사냥한다.

이 PC는 팀 LAN client 설정이다. 사용자는 Client 프로젝트 Ctrl+F5로 시작하고, 공유 Server PC도 같은 protocol70 소스/실행 파일과 배포 데이터를 사용해야 한다. 작업 종료 시 Client/Server 상주 프로세스는 실행하지 않은 상태다.

## 리소스 인계

새 binary resource는 추가하지 않았다. 기존 `MONSTER_KOUKU_CARD_*` catalog 모델과 `cardmiro.march.seto` OBJECT_RESOURCE의 Resources-relative 경로를 그대로 사용한다. 이번 기능의 별도 Drive binary 전달물은 없고, 출구 효과는 팀에서 선정·등록 후 그 효과가 참조하는 기존 Resources/Effect 파일을 전달한다. nav 재베이크나 새 nav 셀 수정도 하지 않았다.

물리 루트는 `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/`이며 필요한 기존 modelAssetId는 아래와 같다. 텍스처 등 각 모델 폴더의 기존 의존 파일도 그대로 필요하다.

- `Character/KoukuSaton/CardMiro_Monster_Heart/CardMiro_Monster_Heart.wmodel`
- `Character/KoukuSaton/CardMiro_Monster_Diamond/CardMiro_Monster_Diamond.wmodel`
- `Character/KoukuSaton/CardMiro_Monster_Clover/CardMiro_Monster_Clover.wmodel`
- `Character/KoukuSaton/CardMiro_Monster_Spade/CardMiro_Monster_Spade.wmodel`
- `Character/KoukuSaton/MN_PPCT_00/MN_PPCT_00.wmodel` (세토)

## 후속 보정 (2026-09-08, 사용자 관찰 3건)

사용자가 실제 런타임에서 관찰한 세 항목을 원인 확정 후 수정했다.

### 1. 병사를 때리면 `300`이 뜨는 것

원인은 `Server/Private/ServerCombatHitRuntime.cpp`의 한 방 처치 규칙이다.
`damage = target.strSpawnGroupId == "cardmaze.targets" ? target.iCurrentHp : Apply_Defense(...)`로
카드미로 목표는 남은 HP 전액을 데미지로 삼기 때문에, 그 값이 공개 체력 300 그대로
`PushDamageEvent`로 들어가 snapshot의 `DamageEvents`를 타고 `CMainApp::RenderDamageNumbers`가 그린다.
망치 데미지 상수 `HAMMER_RAW_DAMAGE`는 100이므로 300은 병사의 최대 체력이지 망치 수치가 아니다.

해결은 같은 파일에서 `cardmaze.targets` 목표일 때 `PushDamageEvent`를 건너뛰는 것이다.
`KILLED` 반환과 집계는 그대로이고 띄우는 숫자만 생략한다. 보스·일반 몬스터는 영향 없다.

### 2. 스택 텍스트를 문양별 한글로

`Client/Private/Level_KakulSaydonArena.cpp`의 `HEART 1 / 3`을
`하트 조각 x 1`, `다이아 조각 x 2`, `스페이드 조각 x 3`, `클로버 조각 x N` 형식으로 바꿨다.
이 파일은 `/utf-8` 옵션이 없으므로 한글을 raw UTF-8로 넣지 않고 wide literal의 universal character name
(`L"\uD558\uD2B8"`)으로 썼다. 파일 바이트는 그대로 ASCII 구간을 유지한다.
`Client/Bin/Resources/Fonts/YoonGasiIIM.spritefont`를 직접 파싱해 glyph 11361개 중 한글 음절 11172자가
모두 있고 사용한 13자가 전부 포함된 것을 확인했다.

### 3. Q로 때린 뒤 움직이지 못하는 딜레이

원인을 상수까지 추적했다.

1. `Shared/Public/Network/PacketMessages.h`의 `KOUKU_INTERACTION_ACTION_MS`가 3000ms다.
2. `Server/Private/GameRoom.cpp`의 `KOUKU_INTERACTION_TICKS`가 30Hz에서 90tick, 즉 3.0초가 된다.
3. Q 입력은 `C2S_INTERACTION_SLOT` 핸들러에서 `eAction = INTERACTION`을 걸고 이동 목표와 경로를 지운다.
4. 망치 판정은 `iActionStartTick + HAMMER_HIT_TICK_OFFSET`, 즉 12tick(0.4초)에 끝난다.
5. 그러나 `interactionElapsed`는 90tick에야 참이 되어 그때 비로소 `eAction`이 NONE이 된다.
6. 이동 명령은 `Handle_MoveGoal`과 pending flush가 `PLAYER_ACTION_STATE::NONE != player.eAction`이면 거절한다.

즉 망치는 0.4초에 맞히는데 몸은 3.0초까지 잠긴다. 남는 2.6초가 체감된 딜레이다.
이 3초는 마리오·댄스의 자세 유지용 상수를 망치가 그대로 물려받은 것이지 망치를 위해 고른 값이 아니다.
같은 이유로 `KOUKU_INTERACTION_COOLDOWN_MS` 3000ms 때문에 다음 타격도 3초 뒤에야 가능했다.

해결은 망치 전용 길이를 따로 둔 것이다.

- Shared에 `KOUKU_MAZE_HAMMER_ACTION_MS = 400`, `KOUKU_MAZE_HAMMER_COOLDOWN_MS = 400` 추가. wire 포맷은 그대로라 protocol은 70 유지.
- `GameRoom.cpp`에 `KOUKU_MAZE_HAMMER_TICKS`(12tick) 추가하고 `static_assert`로 판정 tick 이상임을 고정했다.
- `Update_Players`에 `mazeHammerElapsed`를 추가해 판정과 같은 tick에 잠금을 푼다. 판정 블록이 먼저 실행되므로 타격은 그대로 들어간다.
- 슬롯 쿨다운을 MAZE일 때만 400ms로 줄였고 Client HUD 쿨다운 링 길이도 같은 값을 읽도록 맞췄다.
- 탈출 이동이 같은 INTERACTION을 빌려 쓰기 때문에 `mazeHammerPress`를
  `0u == player.CardMaze.transferStartTick`으로 제한했다. 덤으로 1.2초 암전 중 12tick째에 헛스윙이 나가던 기존 경로도 닫혔다.

결과 타임라인은 누름 0s → 0.4s 망치 판정·처치 → 같은 tick에 이동 가능 → 0.4s 후 재타격 가능이다.
자세 클립은 판정 지점에서 끝나므로 마무리 동작은 재생되지 않는다. 더 길게 보고 싶으면 두 상수를 올리면 된다.

### 자동 검증 (이번 보정)

- 앵커 치환 5파일 `--check` 유일 매칭 후 적용. 적용본이 구문검사한 임시 복사본과 SHA-256 동일(5/5).
- `cl /Zs` 산출물 없는 구문검사 4개 묶음 exit 0:
  `PacketMessages.cpp`, `NetworkProtocolHarness.cpp`, `GameRoom.cpp`, `ServerCombatHitRuntime.cpp`,
  `KoukuSaydonLogicRuntime.cpp`, `CombatHUDViewModel.cpp`, `Level_KakulSaydonArena.cpp`.
- CRLF/LF 구성과 BOM 없음 그대로 유지. `git diff --check` 깨끗함.
- 한글 literal을 파일에서 다시 읽어 `하트`, `스페이드`, `클로버`, `다이아`, `문양`, ` 조각 x `로 디코드되는 것을 확인.

### 아직 수행하지 않은 확인

빌드와 런타임 확인은 사용자가 한다. Server와 Client를 다시 빌드해야 하며(protocol은 70 그대로),
확인할 것은 세 가지다. 병사를 때렸을 때 숫자가 안 뜨는지, HUD가 `다이아 조각 x 1`처럼 나오는지,
Q로 때린 직후 우클릭 이동이 바로 들어가는지. 자세 클립이 짧게 끝나는 느낌은 사용자 판단이 정본이다.

## 재보정 (2026-09-08, 사용자 관찰 2건)

22:15 빌드로 실행한 뒤 사용자가 두 가지를 지적했고 둘 다 앞 보정의 잘못이다.

### 1. 조각 수는 HUD 줄이 아니라 처치 위치에 떠야 한다

300을 막은 것은 맞았으나 그 자리가 비어 버렸다. 사용자가 원한 것은 그 자리에
`다이아 조각 x 1`처럼 콤보가 쌓이듯 올라가는 표시이고 최대 x3까지다. Client는 어느 문양이
죽었는지 알 수 없다. `WORLD_ENTITY_SNAPSHOT`에 archetype이 없기 때문이다. 그래서 Server가
이미 보내던 데미지 이벤트에 문양을 실어 보낸다. protocol 70에서 71로 올렸다.

- Shared `DAMAGE_EVENT`에 `eCardMazeSuit` 1바이트 추가. NONE이면 기존 데미지 숫자, 문양이면 `iAmount`가 누적 수다.
- 검증 불변식: 조각은 항상 outgoing이고 문양 enum은 범위 안이어야 한다.
- Server는 `Resolve_CardMazeHammerHit`의 `bKillCounted`에서 쓰러진 병사 좌표로 조각 이벤트를 보낸다.
- Client `RenderDamageNumbers`는 자기 문양의 조각만 띄우고 기존 상승·페이드 트윈을 그대로 쓴다. 색은 흰색이다.

### 2. Q가 아무것도 안 하는 것은 앞 보정의 회귀다

앞 보정은 판정 tick인 0.4초에 INTERACTION을 풀었다. 그런데 `Client/Private/Character.cpp`는
그 상태가 유지되는 동안만 action age로 클립을 감고, 상태가 풀리면 즉시 idle이나 run으로 스냅한다.
따라서 망치 클립이 앞부분만 재생되고 내려치는 장면이 나오지 않았다. Server 판정 자체는 살아 있었다.
`--card-maze-contract-test`의 `Q reaches the actual interaction action handler`와
`Center Q hit facing away starts solo telescope and actually spawns a target`가 22:15 바이너리에서 PASS다.

수정은 스윙을 자르지 않고 취소를 주는 방향으로 바꿨다.

- `KOUKU_MAZE_HAMMER_ACTION_MS`와 `KOUKU_MAZE_HAMMER_TICKS`, `mazeHammerElapsed` 제거. 3초 자세 종료를 다시 쓴다.
- `GameRoom.cpp`에 file-local `Cancel_MazeHammerRecovery` 추가. 판정 tick 이후의 MAZE 망치 회수만 풀며 탈출 이동은 건드리지 않는다.
- `Handle_MoveGoal`과 `Handle_InteractionSlot`의 거절 직전에서 호출한다. 우클릭 이동과 다음 Q가 곧바로 들어간다.
- MAZE 쿨다운 400ms는 유지해서 연타 간격을 정한다.

### 자동 검증 (재보정)

- 앵커 치환 7파일 `--check` 유일 매칭 후 적용. 적용본이 구문검사한 임시 복사본과 SHA-256 동일(7/7).
- `cl /Zs` 4개 묶음 exit 0: `PacketMessages.cpp`, `NetworkProtocolHarness.cpp`, `GameRoom.cpp`,
  `ServerCombatHitRuntime.cpp`, `MainApp.cpp`, `CombatHUDViewModel.cpp`, `Level_KakulSaydonArena.cpp`.
- 내가 삽입한 줄은 전부 CRLF다. 하네스의 LF 45줄은 이전 커밋에서 온 것으로, 내가 건드리지 않은 2690줄이 이미 LF임을 확인했다.
- `git diff --check` 깨끗함.

### 아직 수행하지 않은 확인

protocol 71이므로 Server와 Client를 다시 함께 빌드해야 한다. 하네스도 재빌드 대상이다.
화면 확인은 사용자가 한다. 병사를 잡을 때 그 자리에 `다이아 조각 x 1`이 떠오르는지,
x3까지 올라가는지, Q 스윙이 정상으로 보이고 때린 뒤 우클릭 이동과 다음 Q가 바로 들어가는지다.
