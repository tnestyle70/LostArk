# NPC 행동·순찰·일괄 배치 수직 슬라이스 RESULT

Map Tool의 실제 버튼 순서와 설정값 설명은
[NPC 행동·순찰·일괄 배치 Map Tool 사용 설명서](2026-08-26_NPC_BEHAVIOR_MAP_TOOL_USER_GUIDE.md)를
따른다.

## 1. 실제 프레임워크 통합 범위

기존 `NpcCatalog -> Gameplay.world.json -> world publisher -> Server world entity ->
Shared snapshot -> CNpc` 경로에 NPC 행동 저작과 제품 runtime을 연결했다. 별도 로컬 AI나 두 번째
NPC 생성 경로는 만들지 않았다. 아래 항목은 실제 실행 프레임워크
`C:\Users\USER\source\졸업팀폴\LostArk`의 기존 미커밋 변경을 보존하며 의미 병합한 범위다.

- Gameplay authoring `formatVersion 6`
- Server `worldbootstrap` v7
- Client `npcpresentation` v2
- 제자리 생활 행동, waypoint 순찰, spawn 반경 배회
- loop, ping-pong, once 경로 정책과 지점별 대기·도착 방향
- sequence/weighted 생활 행동, 실제 model clip 선택과 preview
- Server navigation 기반 위치·방향·semantic action 권위
- collisionBox·살아 있는 NPC/monster/boss body 충돌과 층 높이 분리
- Client snapshot 보간과 `(actionId, actionStartTick)` 재생 edge
- 다중 archetype pool의 ghost 일괄 배치, Confirm/Discard 원자 반영
- navigation, collision, NPC 간격, stable ID와 entity 예산 검증
- 초기 active runtime blocker와 반경 내부 wander 경로를 Server와 같은 기준으로 검증

## 2. 데이터와 책임 분리

Map Tool은 placement별 행동 의미, 경로, 속도, timing과 clip binding을 함께 저작한다. publisher가
이를 두 제품 산출물로 분리한다.

```text
Gameplay.world.json v6
  -> Server worldbootstrap v7: mode/path/speed/seed/semantic action ID
  -> Client npcpresentation v2: idle/walk/action clip/loop/rate/blend
```

Server 산출물에는 model path나 clip 이름이 들어가지 않는다. Server가 fixed tick에서 NPC transform과
현재 semantic action을 결정하고, Client는 placement별 binding으로 실제 clip을 재생한다. 없는 clip이나
손상된 action binding은 해당 NPC의 idle 표현으로만 격리한다.

## 3. Map Tool에서 가능한 작업

NPC를 선택하면 다음 항목을 편집할 수 있다.

- `stationary`, `patrol`, `wander` 행동 모드
- 이동속도, 반복/왕복/1회, 시작 지연, idle 시간, 결정적 seed
- waypoint 추가·삭제·재정렬·맵 클릭 선택, 지점별 대기와 도착 yaw
- sequence/weighted action 추가·삭제·재정렬
- 현재 NPC model이 실제 보유한 idle/walk/action clip 선택
- loop, duration, wait, weight, playback rate, blend
- preview Play/Restart/Pause와 Apply/Revert
- 다른 enabled NPC 바라보기

일괄 배치는 여러 archetype을 pool에 넣고 중심, 반경, 수량, 최소 간격, seed와 random yaw를 지정한다.
후보를 ghost로 먼저 표시하고 모든 검증이 통과한 경우에만 Confirm으로 document와 preview를 함께
commit한다. 선택 NPC의 적용된 behavior를 batch preset으로 복사할 수도 있다.

행동 Apply, placement 이동, batch 사전검사와 Confirm은 모두 현재 navigation 및 초기
runtime blocker graph에서 spawn/waypoint/loop 연결성을 다시 검사한다. wander는 목적지만 반경 안인
것으로 끝내지 않고 경로의 모든 cell이 spawn 반경 안에 있을 때만 승인한다. batch ghost가 있으면
Save, Reload, placement 선택 전환을 거부하므로 `Confirm` 또는 `Discard` 없이 임시 결과가 사라지지
않는다.

## 4. 현재 target 데이터 기준

`NpcCatalog.json`에는 `runtimeStatus=supported` NPC archetype 75종이 있다. Bern
`Gameplay.world.json`은 기존 revision을 유지한 채 전체 placement 16개, 그중 NPC 10개를 그대로
보존했다. 이번 v6 초기 이관에서는 기존 NPC를 삭제·추가·이동하지 않았고 모두
`behavior: null`로 두었다.

따라서 특정 Aylara/Beda placement에 순찰이나 생활 행동이 미리 들어 있다고 가정하면 안 된다.
Map Tool에서 실제 배치된 NPC 하나를 선택해 `Enable Behavior`부터 켜고 직접 저작해야 한다.
`behavior: null`은 기존 정적 idle NPC와 같은 의미다.

전체 archetype 이름·용도는 `2026-08-26_NPC_CATALOG_REFERENCE.md`에 정리했다. 같은 archetype을
여러 placement에 사용하되 각 placement ID는 고유해야 한다.

## 5. 검증 상태

### 5-1. 데이터와 소스 정합성

- publisher PowerShell 구문 parse: PASS
- 네 Area Gameplay JSON parse와 v6 필드 존재: PASS
- target 대비 placement ID, 순서, transform, revision 보존: PASS
  - Bern 16 placements / NPC 10
  - Training 4, Character Select 5, Valtan 154 placements
- v6와 NPC `behavior: null` 추가를 제외한 JSON 의미 동일성: PASS
- 담당 staging 파일 `git diff --no-index --check`: PASS

### 5-2. 실제 프레임워크 자동 검증

- `Publish-WorldGameplay.ps1 -Mode Validate`: PASS
  - Bern 16, Valtan 154, Training 4, Character Select 5 placements
  - Valtan/Character Select spawn group와 Valtan prop 계약 포함
- `Publish-WorldGameplay.ps1 -Mode Publish`: PASS
- `Publish-ValtanWorldDestruction.ps1 -Mode Validate`: PASS
- `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest`: PASS
- Engine, Shared, Server, Client x64 Debug build/link: PASS
- Engine, Shared, Server, Client x64 Release build/link: PASS
- NetworkProtocolHarness Debug/Release: `failures: 0`
- Server Debug/Release NPC 행동 계약: 전 항목 PASS
  - v7 parse와 잘못된 action/waypoint/weight/look-target 거부
  - once/loop/ping-pong, 지점 wait/yaw, sequence action edge
  - navigation, collisionBox, 같은 층 NPC body와 다른 층 분리
  - 결정적 wander와 목적지 탐색 실패 경로

Server 전체 계약 실행 결과는 NPC와 무관한 기존 Bern 기대값 두 항목 때문에 `failures: 2`다.
테스트는 예전 Bern `9 placements / NPC 2 / trigger 2`를 기대하지만 현재 저작 데이터는
`16 placements / NPC 10`이며 `trigger.bern.to-valtan`이 없다. 현재 사용자의 맵 배치를 삭제하거나
테스트 통과만을 위해 옛 trigger를 되살리지 않고 이 차이를 그대로 보존했다.

정본 전체 회귀 자동화는 기존 Warlord Effect 작업의 runtime DDS 세 파일이 Git 추적 상태가 아니라는
검증에서 중단된다. NPC 소스의 Debug/Release compile/link 및 관련 계약 검증은 그 이전과 별도 실행에서
통과했으며, 이 RESULT는 해당 무관한 실패를 전체 PASS로 숨기지 않는다.

### 5-3. 사용자 수동 확인 대기

Map Tool Apply/Save/Reload와 Server+Client Bern 화면의 이동·대기·생활 animation 품질은 사용자가
직접 실행해 판정해야 한다. 에이전트는 Client/UI를 자율 실행하지 않았으므로 visual PASS로 기록하지
않는다.

### 5-4. 반복 배치 간소화 후속 작업

NPC 한 명마다 ID 입력, placement arm, behavior 재작성과 animation loop 설정을 반복하지 않도록
기존 Map Tool 경로에 다음 저작 보조 기능을 추가했다. Gameplay/Server/Client schema는 바꾸지 않고
기존 v6 -> v7/v2 계약을 그대로 사용한다.

- `Quick NPC Brush`
  - 선택 archetype을 한 번 arm한 뒤 맵을 클릭할 때마다 계속 NPC를 생성한다.
  - placement ID는 `npc.<area>.<archetype>.<번호>`의 첫 빈 번호로 자동 생성한다.
  - random yaw, 256 entity budget, navigation과 문서 validation을 placement마다 다시 검사한다.
  - `Esc` 또는 Stop 버튼 전까지 연속 배치하며 실패한 한 번의 click만 거부한다.
- `Capture Selected NPC As Brush Preset`
  - 선택 NPC의 idle clip, stationary/patrol/wander, route loop, waypoint, wait, action과 animation binding을
    brush preset으로 snapshot한다.
  - patrol preset은 새 spawn 위치만큼 waypoint를 평행 이동하고 모든 지점을 navigation에 다시 투영한다.
  - 새 placement에는 ID 기반 seed를 다시 부여해 원본 placement identity를 복제하지 않는다.
- `Easy NPC Setup`
  - 실제 preview에서 선택한 clip을 사용해 `1-Click: Stationary Idle Loop` 또는
    `1-Click: Wander With Preview Walk`를 적용한다.
  - clip 이름을 추측하지 않으며 현재 model이 실제 가진 preview clip만 사용한다.
  - advanced behavior/action editor와 기존 batch ghost 경로는 그대로 남겨 세부 조정이 가능하다.

후속 작업 검증 결과는 다음과 같다.

- Client x64 Debug build/link: PASS
- Client x64 Release full rebuild/link: PASS (`MapTool.cpp` 실제 재컴파일, 오류 0)
- `Publish-WorldGameplay.ps1 -Mode Validate`: PASS
  - Bern 16, Valtan 154, Training 4, Character Select 5 placements
  - Valtan/Character Select spawn group와 Valtan prop 계약 포함
- `git diff --check`: PASS
- 화면에서 연속 click, preset 복제, Save/Reload와 Bern 제품 runtime 이동 확인: 사용자 수동 확인 대기

## 6. 사용자가 확인할 경로

에이전트는 Client/UI를 자율 실행하지 않았다. 사용자는 다음 순서로 직접 화면을 확인한다.

1. Visual Studio에서 `Server + Client` profile을 `Ctrl+F5`로 시작한다.
2. Lobby에서 `Test`로 들어가 F1 Developer Tools의 Map Tool을 연다.
3. Area를 Bern으로 두고 `World Gameplay -> NPC`에서 현재 배치된 NPC 하나를 선택한다.
4. 초기 `behavior: null` 상태에서 `Enable Behavior`를 켜고 path와 action을 작성한 뒤
   `Apply NPC Behavior -> Save Gameplay` 순서를 확인한다.
5. batch pool에 NPC를 추가하고 중심을 찍어 Generate한 뒤 ghost를 확인하고 Confirm 또는 Discard한다.
6. publisher를 실행하고 Server를 재시작한 뒤 Lobby -> Bern에서 이동·대기·생활 animation을 육안 확인한다.

Client/UI는 에이전트가 실행하지 않았으므로 마지막 화면 품질 판정은 사용자 수동 확인으로 남는다.

## 7. 이번 범위에 포함하지 않은 기능

대화창, 상점·교환 command, 퀘스트 반응, 의자·책·빗자루 같은 prop socket 자동 부착은 행동 재생과
다른 gameplay 계약이라 포함하지 않았다. 현재 clip은 선택할 수 있지만 clip이 요구하는 별도 소품은
자동 생성되지 않는다. 이 기능들은 실제 consumer와 Server authority를 포함한 별도 수직 슬라이스로
구현해야 한다.

## 8. 작업 트리 경계

기존 다른 미커밋 변경과 생성물은 보존했다. 이 작업에서는 자동 stage, commit, push를 하지 않았다.

## 9. 2026-08-27 Bern 긴 회랑 순찰 적용

사용자가 파란 화살표로 지정한 긴 회랑을 따라 `npc.bern.25287`이 시계 방향으로 계속 순찰하도록
Bern authoring revision을 460에서 461로 올렸다. 기존 `wander` 반경 5 m를 `patrol / loop`로
교체했으며 이동 속도 1.5와 `npc_walk_normal_1` 표현은 유지했다. 네 waypoint는 모두
`LV_BER_BERNCASTLE`의 실제 walkable cell에 맞췄다.

```text
top-right    138.238007, 42.1963768,  -62.688004
bottom-right 138.238007, 49.9256287, -146.188004
bottom-left  135.738007, 49.9256287, -146.188004
top-left     135.738007, 42.1963806,  -62.688004
```

아래쪽에 고정 배치된 `npc.bern.25287_2`와 겹치지 않도록 반환 지점을 약 3.7 m 앞에 두었다.
publisher 이후 Server bootstrap은 같은 네 waypoint와 `patrol / loop`를 보유하며 Client
presentation은 `npc_walk_normal_1`을 사용한다.

자동 검증 결과는 다음과 같다.

- `Publish-WorldGameplay.ps1 -Mode Validate`: PASS, Bern 14 placements
- `Publish-WorldGameplay.ps1 -Mode Publish`: PASS
- Server x64 Debug/Release build: PASS
- Server Debug/Release `--contract-test`: `failures: 0`
- JSON parse와 변경 파일 `git diff --check`: PASS
- Server 제품 계약은 현재 Bern 14 placements / enabled NPC 8개와 이 placement의 4-point
  patrol loop를 명시적으로 검사한다.

Client 화면은 에이전트가 실행하지 않았다. Server를 재시작하고 Lobby -> Bern에서 실제 이동선,
회전 품질과 다른 NPC 간격을 사용자가 육안 확인해야 한다.

## 10. 2026-08-27 Bern 광장 제자리 생활 NPC 20명 적용

revision 462에서는 Bern player spawn 네 지점이 있는 `X≈136.9, Z≈-23.7` 원형 공간을 목표
광장으로 잘못 판단했다. 사용자 런타임 스크린샷으로 실제 목표가 석상 앞 큰 팔각형 광장임을
확인한 뒤 revision 463에서 `npc.bern.plaza.01`부터 `.20`까지 전부 이동했다. 새 중심은 맵 배치의
석상과 중앙 화단으로 확인한 `X≈136.9, Z≈-80.35`다. revision 463의 반경 8.69~11.68 m 외곽 링은
실제 광장 위치는 맞았지만 참고 사진보다 너무 넓고 규칙적으로 보였다. 사용자 런타임 스크린샷과
참고 사진을 비교해 revision 464에서는 반경 4.77~6.78 m로 압축하고 위·좌·우·아래에 2~4명씩
작은 군중으로 보이도록 간격을 불규칙하게 조정했다. 기존 Bern NPC와 겹치지 않는 archetype 20개를
사용하며 revision 464의 NPC 사이 최소 XZ 간격은 약 1.581 m였다. 모든 배치 좌표와 Y 높이는
`LV_BER_BERNCASTLE`의 실제 walkable navigation cell에서 가져왔다.

사용자 런타임 확인에서 revision 464도 전체 윤곽이 원형으로 읽히는 문제가 확인됐다. revision 465는
원형 반경 배치 조건을 제거하고 참고 사진의 군중 구성을 직접 옮겼다. 좌상단 큰 군중 8명, 석상 앞
작은 군중 3명, 오른쪽 세로 군중 5명, 아래쪽 군중 4명으로 분리했으며 네 군중 사이에는 NPC가 없는
큰 공백을 남겼다. 20명 중 19명은 같은 군중의 다른 NPC를 look target으로 사용해 쌍 또는 삼각형으로
서로 바라보고, 기도 동작 한 명만 중앙 화단을 바라본다. 따라서 전체가 중앙을 보는 원형 대형이 아니라
각자 다른 대화·환호 무리로 읽히도록 구성했다.

revision 465의 `npc.bern.plaza.09`는 `npc.bern.25287`의 오른쪽 순찰 차선 `X=138.238007` 위에
정확히 놓여 있었다. 양쪽 NPC의 Server 충돌 반경 합계가 0.9 m이므로 순찰 병사는 이 지점에서
막힌 뒤 재시도하게 된다. revision 466에서는 해당 NPC를 같은 석상 앞 군중 안의 walkable cell
`(139.738007, 42.577061, -86.188004)`로 옮겨 차선과 1.5 m를 확보했다. 파란 화살표로 지정한
4-point 시계 방향 loop, 속도 1.5, `npc_walk_normal_1`은 그대로 유지했다. 제품 계약은 두 세로
차선과 모든 광장 NPC 사이에 충돌 반경보다 큰 0.95 m 이상의 여유가 있는지도 검사한다.

20명 모두 `stationary / loop`, waypoint 없음, wander 반경 0으로 저장했다. action은 600000 ms 동안
clip loop를 유지하고 idle/wait가 0이므로 위치를 바꾸지 않고 같은 생활 동작을 계속 반복한다.
시작 지연과 playback rate를 조금씩 달리해 동시에 같은 박자로 움직이지 않게 했다. 배치 방향은
광장 중앙을 향하며 `npc.bern.plaza.04`와 `.05`는 서로를 look target으로 참조해 대화 장면을 만든다.

사용한 표현은 환호 네 종류, 대화/긍정, 박수, 칭찬, 인사 두 종류, 웃음 두 종류, 기도,
친밀한 제스처, 사과, 승리 두 종류, 야유, 수줍음, 놀람이다. 모든 clip name은 각 archetype의
실제 animation-set `.wmodel`에서 직접 열거해 확인했다.

자동 검증 결과는 다음과 같다.

- authoring revision 466 / Bern 34 placements / enabled NPC 28
- 광장 NPC 20명, archetype 20개 고유, action clip 20개 고유: PASS
- 좌상단/석상 앞/오른쪽/아래쪽 군중 `8 / 3 / 5 / 4`, 원형 반경 조건 제거: PASS
- 군중 내부 look target 19명, 군중 사이 의도적인 빈 구역 유지: PASS
- 광장 최소 간격 1.000 m, navigation cell과 Y 높이 20/20 일치: PASS
- 병사 4-point loop 유지, 양쪽 세로 차선과 광장 NPC 최소 1.5 m 이격: PASS
- model asset 20/20 존재, animation-set clip membership 20/20: PASS
- `Publish-WorldGameplay.ps1 -Mode Validate/Publish`: PASS
- Client presentation 광장 entry 20개, loop 20/20: PASS
- Server bootstrap 광장 logical stationary action 20개: PASS
- Server x64 Debug/Release build: PASS
- Server Debug/Release `--contract-test`: `failures: 0`
- Client x64 Debug/Release build: PASS

Debug Client 링크에서 기존 DirectXTK PDB 누락 `LNK4099` 경고가 출력됐지만 링크는 성공했고
`Client/Bin/Debug/Client.exe`가 정상 생성됐다. Release Client는 경고 없이 생성됐다.
Release Server 링크에서는 기존 Shared PDB 형식 `LNK4020` 경고 7개가 출력됐지만 오류 없이
`Server/Bin/Release/Server.exe`가 생성됐고 Release `--contract-test`도 `failures: 0`으로 통과했다.

Client 화면은 에이전트가 실행하지 않았다. Server를 재시작하고 Lobby -> Bern에서 네 군중의
실제 밀도, 모델 크기, clip의 시각 의미와 서로 겹쳐 보이지 않는지를 사용자가 육안 확인해야 한다.

## 11. 2026-08-27 Bern 회랑 병사 2명 독립 왕복 순찰

MapTool 화면에서 체크한 `npc.bern.25287`과 새 `npc.bern.25287.2`를 회랑의 왼쪽·오른쪽
차선에 각각 배치했다. revision 466의 한 병사가 네 지점을 도는 직사각형 `loop`는 제거하고,
두 병사가 자기 차선의 시작점과 플레이어 최초 생성 구역 쪽 끝점 사이를 같은 길로 되돌아오는 `patrol / pingPong`으로
교체했다. 두 차선은 서로 2.5 m 떨어져 있으며 각 끝점에서 500 ms 멈춘 뒤 방향을 바꾼다.
오른쪽 병사는 500 ms 늦게 출발해 두 모델의 보행과 회전이 완전히 겹치지 않게 했다.

revision 467에서는 화면 방향을 반대로 해석해 끝점을 `Z=-146.188004`로 잘못 게시했다. 사용자
런타임 확인 직후 이 경로를 폐기했고 revision 468에서는 두 병사 모두 현재 위치 `Z=-62.688004`에서
플레이어 spawn `Z≈-22.46` 쪽의 `Z=-27.688004`까지 이동하도록 방향을 바로잡았다. 선택한 끝점은
spawn 약 5 m 앞의 실제 walkable cell이므로 생성 중인 플레이어 몸과 겹치지 않는다.

```text
왼쪽 npc.bern.25287
start        135.738007, 42.1963806, -62.688004
player-entry 135.738007, 42.1963768, -27.688004

오른쪽 npc.bern.25287.2
start        138.238007, 42.1963768, -62.688004
player-entry 138.238007, 42.1963730, -27.688004
```

두 병사는 속도 1.5와 `npc_walk_normal_1` 표현을 사용한다. 화면에서 새로 만든 점 표기 ID는
저장 정본에도 그대로 사용했으며, 기존 아래쪽 고정 병사 `npc.bern.25287_2`는 다른 배치이므로
삭제하거나 덮어쓰지 않았다. 최종 authoring revision은 468이고 Bern은 35 placements / enabled NPC
29개다. 제품 계약은 두 배치가 각각 두 waypoint의 `PING_PONG`인지, 두 경로가 30 m 이상인지,
두 번째 waypoint가 첫 번째보다 실제 `player_1` spawn에 가까운지, 두 평행 차선에 다른 NPC가
놓이지 않았는지를 함께 검사한다.

자동 검증 결과는 다음과 같다.

- `Publish-WorldGameplay.ps1 -Mode Validate/Publish`: PASS, Bern 35 placements
- Server bootstrap: 두 병사 모두 `patrol / pingPong`, waypoint 2개, 속도 1.5: PASS
- Client presentation: 두 병사 모두 `npc_walk_normal_1`: PASS
- Server x64 Debug/Release build: PASS
- Server Debug/Release `--contract-test`: `failures: 0`
- Client x64 Debug/Release build: PASS
- JSON parse와 변경 파일 `git diff --check`: PASS

Debug Client의 기존 DirectXTK PDB 누락 `LNK4099`와 Release Server의 기존 Shared PDB 형식
`LNK4020` 경고는 출력됐지만 양쪽 모두 링크 성공이며 이 데이터 변경과 관계없는 기존 경고다.

Client 화면은 에이전트가 실행하지 않는다. Publish와 Server/Client 재빌드 후 실행 중이던
Server와 Client를 완전히 종료하고 다시 시작한 다음 Lobby -> Bern에서 두 병사가 화살표대로
각자 직진·회전·복귀하는지는 사용자가 육안 확인해야 한다.
