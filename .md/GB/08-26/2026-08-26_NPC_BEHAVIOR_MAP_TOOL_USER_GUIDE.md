# NPC 행동·순찰·일괄 배치 Map Tool 사용 설명서

## 1. 이 설명서로 할 수 있는 것

이 문서는 Debug Client의 Map Tool에서 NPC를 직접 배치하고 다음 행동을 설정하는 방법을 설명한다.

- 제자리에 서서 생활 애니메이션 반복
- 지정한 지점을 따라 순찰
- 배치 지점 주변을 무작위 배회
- 다른 NPC를 바라보며 대화·작업 애니메이션 표현
- 여러 NPC 종류를 선택 영역에 한꺼번에 분산 배치

Map Tool은 저작 데이터를 만드는 화면이다. Map Editor 안에서는 모델, 애니메이션, 경로 선과 batch
ghost를 미리 볼 수 있지만 Server가 실제로 NPC를 움직이는 모습은 실행하지 않는다. 실제 이동과 행동
전환은 저장·publish·Server 재시작 후 제품 Bern 맵에서 확인한다.

NPC 이름과 원작 용도는 [NPC 카탈로그 이름·용도 목록](2026-08-26_NPC_CATALOG_REFERENCE.md)을
참고한다.

현재 target 기준으로 선택 가능한 supported NPC는 75종이다. Bern authoring에는 전체 placement
16개 중 NPC 10개가 이미 배치되어 있지만, v6 초기 이관 상태에서는 10개 모두
`behavior: null`이다. 특정 NPC에 순찰·생활 행동 예제가 미리 설정되어 있다고 가정하지 않는다.

## 2. 가장 짧은 사용 순서

처음에는 아래 순서만 따라 하면 된다.

1. Visual Studio를 `x64 Debug`로 두고 `Server + Client` profile을 `Ctrl+F5`로 실행한다.
2. Lobby에서 `Test`를 선택해 Map Editor workspace로 들어간다.
3. `F1 -> Map Tool`을 누른다.
4. 상단 `Area`에서 `Bern`을 선택하고 `Focus Area`를 누른다.
5. 모드 줄에서 `World Gameplay`를 선택한다.
6. 새 NPC라면 `NPC -> NPC Archetype -> Placement ID -> Arm World Placement -> 맵 클릭` 순서로
   배치한다.
7. placement 표에서 NPC를 선택하고 `Enable Behavior`를 켠다.
8. `Stationary`, `Patrol`, `Wander` 중 하나를 설정한다.
9. `Save Gameplay`를 누른다. 유효한 행동 draft는 저장 직전에 자동으로 적용된다.
10. Client와 Server를 종료하고 Server를 다시 빌드해 publisher를 실행한다.
11. `Server + Client`를 다시 실행하고 Lobby의 `Bern`으로 들어가 실제 동작을 확인한다.

`Apply NPC Behavior`는 여러 설정을 시험하면서 현재 draft를 인메모리 문서에 먼저 확정할 때 쓰는
선택 기능이다. 누르지 않았더라도 유효한 draft는 `Save Gameplay`가 자동 적용한다. 저장 뒤 실행 중인
Server를 그대로 두면 Server에는 반영되지 않으므로 publish와 Server 재시작은 여전히 필요하다.

처음 기능을 확인할 때는 Bern placement 표에서 현재 배치된 NPC 하나를 선택한다. 처음에는
`behavior: null`이므로 `Enable Behavior`를 켠 다음 가장 단순한 `Stationary` 행동 하나부터
저작하는 편이 쉽다. Aylara/Beda를 포함한 특정 placement에 미리 작성된 행동 샘플은 없다.

## 3. Map Tool 열기

### 3-1. 필요한 실행 조건

- Client는 `Debug` 구성으로 실행해야 한다. F1 Developer Tools는 Debug 기능이다.
- Map Tool 버튼은 Lobby의 `Test`로 들어간 Map Editor workspace에서만 활성화된다.
- Bern이나 Valtan 제품 맵에서 F1을 열어도 `Map Tool` 버튼으로 Map Editor를 시작할 수는 없다.

### 3-2. 화면 진입

```text
Lobby
  -> Test
  -> F1
  -> LostArk Developer Tools
  -> Map Tool
  -> Area: Bern
  -> Focus Area
  -> World Gameplay
```

상단의 `Show Bern Landscape`는 Bern 배경 표시만 켜고 끈다. 저작 데이터는 지워지지 않는다.

Bern은 visual placement가 약 50,000개라 Area를 처음 여는 데 시간이 걸릴 수 있다. 화면이 잠시 멈춘
것처럼 보여도 `Workspace status`가 Area commit 완료를 알릴 때까지 Area나 버튼을 반복해서 누르지
않는다.

`Focus Area`로 카메라를 Area 중심에 맞춘다. editor camera는 WASD로 이동하고 Tab으로 mouse-look을
전환한다. placement, waypoint와 batch center를 찍을 때는 ImGui 창이 아닌 실제 맵 표면을 왼쪽
클릭한다. 현재 무장된 pick은 `Esc`로 취소할 수 있다.

Area를 바꾸거나 `Exit to Lobby`를 누를 때 저장하지 않은 내용이 있으면 다음 창이 열린다.

- `Save and Continue`: 현재 저작 내용을 저장하고 이동
- `Discard and Continue`: 저장하지 않고 이동
- `Cancel`: 현재 Area에 남기

유효한 행동 draft는 `Save and Continue`가 자동 적용한다. 입력이 불완전한 draft는 Apply/Revert로
정리해야 하며, 확정하지 않은 batch ghost는 Confirm/Discard를 먼저 해야 저장할 수 있다.

## 4. NPC 한 명 배치하기

### 4-1. NPC 종류 선택

`World Gameplay Authoring`에서 `NPC` radio button을 선택한다.

1. `NPC Archetype`에서 원하는 NPC를 선택한다.
2. `Placement ID`를 확인한다.
3. `Arm World Placement`를 누른다.
4. 맵의 배치할 바닥을 마우스 왼쪽 버튼으로 클릭한다.

배치 mode를 취소하려면 `Cancel World Placement`를 누르거나 `Esc`를 누른다.

### 4-2. Placement ID 규칙

Placement ID는 맵 안에서 NPC 한 명을 식별하는 고유 ID다. 같은 NPC 외형을 여러 번 사용해도 각
placement에는 서로 다른 ID가 필요하다.

```text
npc.bern.market.beda.01
npc.bern.market.beda.02
npc.bern.guard.north.01
```

NPC Archetype을 고르면 기본 ID가 자동으로 제안된다. 이미 존재하는 ID로 배치를 시도하면 Tool이
`.2`, `.3`처럼 비어 있는 suffix를 자동으로 찾아 최종 고유 ID를 만든다. 다만 나중에 역할을 알아보기
쉽도록 자동 번호에 의존하기보다 `market`, `guard`, `north` 같은 의미와 번호를 직접 넣는 편이 좋다.
`.999`까지 사용 중이라 빈 suffix를 찾지 못할 때만 배치가 거부된다.

### 4-3. 배치 후 수정

아래 placement 표에서 ID를 선택하면 다음 값을 수정할 수 있다.

- `Position`: 월드 위치
- `Yaw Degrees`: NPC가 처음 바라보는 방향
- `Enabled`: 실제 제품 world entity로 활성화할지 여부
- `Delete Gameplay Placement`: placement 삭제

`Enabled`를 끄면 placement 행은 문서에 남지만 제품 Server에서는 해당 NPC를 spawn하지 않는다.

이동 행동이 있는 NPC의 Position을 직접 수정하면 현재 navigation에서 다시 검증된다. 이동할 수 없는
바닥, 다른 층의 잘못된 cell 또는 초기 벽으로 막힌 위치라면 수정이 반영되지 않는다.

다른 NPC가 `Look Target NPC`로 이 placement를 사용하고 있으면 비활성화하거나 삭제할 수 없다. 먼저
바라보기 참조를 해제해야 한다.

## 5. 기본 Idle 애니메이션 설정

placement를 선택하면 `NPC Preview Animation`이 나타난다.

1. `Preview Clip`에서 실제 모델이 가진 애니메이션을 선택한다.
2. `Play / Restart Preview Clip`으로 재생한다.
3. 필요하면 `Pause Preview Clip`으로 멈춘다.
4. 기본 대기 자세로 사용할 clip이면 `Use As Placement Idle`을 누른다.
5. 저장한 placement idle을 없애려면 `Clear Placement Idle`을 누른다.

placement idle을 비워 두면 `NpcCatalog.json`에 설정된 해당 모델의 기본 idle clip을 사용한다.

모델마다 보유한 clip이 다르다. clip 이름을 추측해 입력하지 말고 현재 모델의 dropdown에서 실제로
보이는 clip을 선택한다.

## 6. NPC Behavior 공통 설정

placement를 선택한 뒤 `NPC Behavior Authoring -> Enable Behavior`를 켠다.

### 6-1. Behavior Mode

| 화면 값 | 용도 |
|---|---|
| `Stationary` | 이동하지 않고 제자리 생활 행동을 반복 |
| `Patrol` | 작성한 waypoint를 순서대로 이동 |
| `Wander` | 최초 배치 위치를 중심으로 지정 반경 안을 배회 |

Mode를 바꾸면 서로 맞지 않는 데이터가 정리된다.

- `Stationary`로 바꾸면 waypoint와 wander radius가 제거된다.
- `Patrol`로 바꾸면 wander radius가 제거된다.
- `Wander`로 바꾸면 waypoint가 제거된다.

### 6-2. 공통 필드

| 필드 | 의미 | 추천 시작값 |
|---|---|---|
| `Route Mode` | Patrol의 마지막 waypoint 이후 진행 방식 | `Loop` 또는 `Ping Pong` |
| `Action Selection` | 생활 행동을 고르는 방식 | 자연스러운 군중은 `Weighted` |
| `Move Speed` | Server 기준 이동속도 m/s | 일반 보행 `0.8~1.5` |
| `Wander Radius` | Wander가 최초 배치 위치에서 벗어날 수 있는 반경 | 좁은 광장 `3~6` |
| `Random Seed` | 행동 순서와 목적지를 결정하는 seed | NPC마다 다른 양수 |
| `Start Delay (ms)` | room 시작 후 첫 행동까지 지연 | `0~3000` |
| `Idle Minimum (ms)` | 행동 사이 최소 idle 시간 | `1000` |
| `Idle Maximum (ms)` | 행동 사이 최대 idle 시간 | `3000` |
| `Walk Clip` | Patrol/Wander 이동 중 재생할 실제 모델 clip | 모델의 걷기 clip |
| `Look Target NPC` | 멈춰 있을 때 바라볼 다른 enabled NPC | 필요 없으면 `<none>` |

`Route Mode`는 Patrol에서만 사용한다. `Wander Radius`는 Wander에서만 나타난다. Patrol과 Wander는
반드시 `Walk Clip`이 있어야 Apply할 수 있다.

같은 모델을 여러 명 배치할 때 Random Seed를 다르게 주면 모두 같은 순간에 같은 행동을 하는 현상을
줄일 수 있다. 같은 설정과 같은 seed는 Server가 다시 시작돼도 같은 결정 순서를 만든다.

Tool이 허용하는 입력 범위는 `Move Speed 0.1~10`, `Wander Radius 0.5~100`, 시간 값
`0~600000ms`다. `Idle Minimum`은 `Idle Maximum`보다 크게 두지 않는다.

## 7. 제자리 생활 행동 만들기

경비, 상인, 청소, 책 읽기, 기도, 박수처럼 위치를 옮기지 않는 NPC에 사용한다.

1. `Enable Behavior`를 켠다.
2. `Behavior Mode`를 `Stationary`로 둔다.
3. `Action Selection`을 선택한다.
4. 아래 `Ambient Actions`에 실제 animation을 추가한다.
5. 필요하면 `Apply NPC Behavior`로 중간 상태를 확정한다.
6. `Save Gameplay`를 누른다. 적용하지 않은 유효한 draft도 이때 자동 적용된다.

행동이 하나도 없으면 NPC는 placement idle만 유지한다.

## 8. Ambient Actions 설정

### 8-1. 행동 추가

1. 위쪽 `Preview Clip`에서 사용할 clip을 선택한다.
2. `Add Action From Preview Clip`을 누른다.
3. 생성된 action의 값을 조정한다.
4. 각 action의 `Preview Action`으로 표현을 확인한다.

`Up Action`, `Down Action`으로 순서를 바꾸고 `Delete Action`으로 제거한다.

### 8-2. Action 필드

| 필드 | 의미 |
|---|---|
| `Action ID` | Server와 Client가 공유하는 고유한 의미 행동 ID |
| `Action Clip` | 이 NPC 모델에서 실제 재생할 clip |
| `Loop` | action이 유지되는 동안 clip을 반복할지 여부 |
| `Duration (ms)` | Server가 이 action 상태를 유지하는 시간 |
| `Wait After (ms)` | action 종료 후 추가 대기시간 |
| `Weight` | `Weighted` 선택에서 이 action이 뽑힐 상대 비중 |
| `Playback Rate` | Client animation 재생속도. `1.0`이 원래 속도 |
| `Blend Seconds` | 이전 animation에서 전환하는 혼합 시간 |

Action ID는 같은 NPC behavior 안에서 중복되면 안 된다. `npc.idle`, `npc.move.walk`은 runtime 예약
ID이므로 사용하지 않는다. 다음처럼 역할을 알아볼 수 있는 ID가 좋다.

```text
npc.ambient.guard.scan
npc.ambient.market.organize
npc.ambient.conversation.talk
```

`Sequence`는 표의 위에서 아래 순서대로 행동한다. `Weighted`는 Weight 비율로 하나를 선택한다.

```text
idle 계열       Weight 50
주변 둘러보기   Weight 25
옷 정리         Weight 15
인사             Weight 10
```

Weight는 확률 숫자를 정확히 100으로 맞출 필요가 없다. `50:25:15:10`처럼 상대 비율만 사용한다.

`Loop`를 켜도 action 자체는 `Duration`이 끝나면 다음 상태로 넘어간다. 긴 작업 동작이라면 clip 반복과
Duration을 함께 충분히 길게 설정한다.

한 NPC에는 Ambient Action을 최대 32개까지 넣을 수 있다. `Duration`은 `1~600000ms`,
`Wait After`는 `0~600000ms`, `Weight`는 `1~100000`, `Playback Rate`는 `0.1~4.0`,
`Blend Seconds`는 `0~2.0` 범위다.

## 9. 지정 경로 순찰 만들기

### 9-1. 기본 설정

1. `Behavior Mode`를 `Patrol`로 선택한다.
2. `Walk Clip`에서 걷기 animation을 선택한다.
3. `Move Speed`를 정한다.
4. `Route Mode`를 선택한다.

| Route Mode | 실제 진행 |
|---|---|
| `Loop` | 마지막 지점에서 첫 지점으로 돌아가 계속 반복 |
| `Ping Pong` | 마지막 지점에 도착하면 역순으로 되돌아감 |
| `Once` | 마지막 지점에 도착한 뒤 이동 종료 |

### 9-2. Waypoint 찍기

1. `Add Waypoint From Map`을 누른다.
2. 이동시킬 바닥을 마우스 왼쪽 버튼으로 클릭한다.
3. 다음 지점마다 버튼을 다시 누르고 한 번씩 클릭한다.
4. 최소 2개의 waypoint를 만든다.

클릭할 때마다 `wp.01`, `wp.02` 같은 stable ID가 자동으로 만들어진다. 맵에는 현재 draft 경로가
하늘색 선과 지점 marker로 표시된다. `Loop`면 마지막 지점과 첫 지점을 잇는 선도 표시된다.

각 waypoint에서 설정할 수 있는 값은 다음과 같다.

- `Waypoint ID`: 해당 지점의 고유 ID
- `Waypoint Position`: 월드 위치
- `Wait (ms)`: 도착 행동 뒤 추가로 기다리는 시간
- `Use Arrival Yaw`: 도착 후 지정 방향을 사용할지 여부
- `Arrival Yaw`: 도착 방향
- `Up`, `Down`: 순서 변경
- `Delete`: 지점 삭제

Waypoint는 navigation의 walkable cell 위에 있어야 하고 NPC 시작점부터 도달할 수 있어야 한다.
`Loop`는 마지막 지점에서 첫 지점으로 돌아가는 길까지 도달 가능해야 한다.

Waypoint는 NPC당 최대 64개이며 새 waypoint의 기본 `Wait (ms)`는 `1000`이다. 한 번 바닥을 찍으면
waypoint pick이 자동으로 해제되므로 다음 지점을 찍을 때마다 `Add Waypoint From Map`을 다시 누른다.

생활 action이 있으면 NPC는 지점에 도착한 뒤 action을 실행하고 waypoint wait, action의 Wait After,
Idle Minimum/Maximum을 적용한 다음 다음 지점으로 간다. action이 없으면 idle과 waypoint wait만
적용한다.

`Look Target NPC`가 설정되어 있으면 멈춰 있는 동안 상대를 바라보는 방향이 우선한다. 이동 중에는
이동 방향을 바라본다.

## 10. 범위 배회 만들기

1. `Behavior Mode`를 `Wander`로 선택한다.
2. `Walk Clip`을 선택한다.
3. `Move Speed`를 정한다.
4. `Wander Radius`를 정한다.
5. 필요하면 `Ambient Actions`를 추가한다.
6. `Save Gameplay`로 저장한다. 중간 확정이 필요할 때만 먼저 `Apply NPC Behavior`를 누른다.

Wander의 중심은 NPC 최초 placement 위치다. Server는 navigation과 벽을 검사해 반경 안에서 실제로
도달 가능한 목적지만 고른다. 목적지가 반경 안에 있어도 이동 경로가 반경 밖으로 돌아가야 한다면
그 경로는 사용하지 않는다.

좁거나 장애물이 많은 곳에서 반경이 너무 작으면 도달 가능한 목적지가 없어 Apply가 거부될 수 있다.
이때는 반경을 키우거나 NPC 시작 위치를 옮긴다.

## 11. 두 NPC가 서로 대화하는 장면 만들기

현재 기능은 대화창 gameplay가 아니라 두 NPC가 서로 바라보며 대화 animation을 재생하는 생활 연출이다.

1. NPC A와 NPC B를 서로 가까운 위치에 배치한다.
2. 둘 다 `Behavior Mode = Stationary`로 설정한다.
3. NPC A의 `Look Target NPC`를 NPC B placement ID로 지정한다.
4. NPC B의 `Look Target NPC`를 NPC A placement ID로 지정한다.
5. 각 NPC의 실제 talk/listen 계열 clip을 Ambient Action으로 추가한다.
6. 각 draft에서 따로 `Apply NPC Behavior`를 누른다.
7. `Save Gameplay`를 누른다.

두 NPC의 action timeline을 완전히 동기화하는 conversation event는 아직 없다. Start Delay, Duration,
Idle 시간과 Sequence를 비슷하게 설정하면 생활 배경 연출로 맞출 수 있다.

## 12. NPC 여러 명 일괄 배치하기

`NPC Batch Placement` header를 펼친다.

### 12-1. Archetype pool 만들기

1. 위의 `NPC Archetype`에서 한 종류를 고른다.
2. `Add Selected Archetype To Pool`을 누른다.
3. 필요한 종류마다 1~2번을 반복한다.

목록의 `Remove`로 한 종류만 빼고 `Clear Archetype Pool`로 전부 비운다.

### 12-2. 배치 설정

| 필드 | 의미 |
|---|---|
| `Batch ID Prefix` | 생성될 placement ID의 공통 앞부분 |
| `Batch Count` | 만들 NPC 수. 한 번에 최대 128 |
| `Batch Radius` | 중심으로부터 분산시킬 범위 |
| `Minimum Spacing` | NPC·collisionBox와 확보할 최소 간격 |
| `Batch Seed` | 같은 입력에서 같은 분산 결과를 만드는 seed |
| `Random Yaw` | 각 NPC의 최초 방향을 무작위로 설정 |
| `Copy Applied Behavior From Selected NPC` | 선택 사항. placement 표에서 선택한 NPC의 적용 완료 behavior를 batch에 복사 |

처음 펼쳤을 때 기본값은 `Prefix = npc.batch`, `Count = 8`, `Radius = 10`,
`Minimum Spacing = 1.5`, `Seed = 1`, `Random Yaw = 켜짐`이다. 허용 범위는 Count `1~128`,
Radius `0.5~100`, Minimum Spacing `0.25~20`이다.

예를 들어 Prefix가 `npc.bern.market.crowd`면 다음 ID가 생성된다.

```text
npc.bern.market.crowd.001
npc.bern.market.crowd.002
npc.bern.market.crowd.003
```

기존 placement와 겹치지 않는 고유 Prefix를 사용한다.

### 12-3. Ghost 생성과 확정

1. `Pick Batch Center On Map`을 누른다.
2. 배치 중심 바닥을 클릭한다.
3. `Stage NPC Batch Ghost`를 누른다.
4. 맵에 표시되는 분홍색 십자와 초록색 방향선을 확인한다.
5. 결과가 좋으면 `Confirm NPC Batch`를 누른다.
6. 취소하려면 `Discard NPC Batch Ghost`를 누른다.
7. Confirm 뒤 `Save Gameplay`를 누른다.

Ghost는 아직 Gameplay 문서에 들어간 NPC가 아니다. Confirm하지 않고 Save, Reload 또는 다른 placement
선택을 할 수 없도록 막혀 있다. 반드시 Confirm 또는 Discard로 끝낸다.

Pool, Prefix, Count, Radius, Spacing, Seed, Random Yaw 또는 behavior copy 설정을 바꾸면 이전 ghost는
새 설정과 맞지 않으므로 자동으로 지워진다. 설정을 바꾼 뒤에는 중심을 확인하고 다시 Stage한다.

### 12-4. Behavior 복사 주의점

`Copy Applied Behavior From Selected NPC`를 쓰려면 먼저 behavior가 적용된 NPC placement를 표에서
선택해야 한다. 편집 중인 draft가 있다면 Apply 또는 Revert를 먼저 한다.

이 체크박스는 `Random Yaw` 바로 아래에 있다. 위쪽 `NPC Archetype` dropdown에서 모델만 고르는 것으로는
복사 원본이 정해지지 않는다. 아래 placement 표에서 기존 NPC 행을 선택하고 그 NPC의 behavior를
`Apply NPC Behavior`로 적용해야 체크박스가 활성화된다. 조건을 만족하지 않으면 회색으로 비활성화된다.

이 옵션은 일괄 배치 자체에 필수는 아니다. 체크하지 않으면 NPC의 위치·방향·외형만 일괄 생성되며,
Confirm 후 필요한 NPC를 선택해 행동을 따로 설정할 수 있다. 항목 자체가 화면에 전혀 보이지 않는다면
실행 중이던 Client를 종료하고 최신 `x64 Debug` Client를 빌드한 뒤 다시 실행한다.

- Stationary/Wander preset은 여러 위치에 복사하기 쉽다.
- Patrol preset은 원본 NPC 기준 waypoint 전체를 새 위치만큼 평행 이동한다.
- 번역된 waypoint가 navigation 밖이거나 벽으로 막히면 batch 전체가 거부된다.
- pool의 다른 NPC 모델이 같은 walk/action clip을 보유하지 않으면 batch 전체가 거부된다.

서로 animation set이 다른 모델을 섞을 때는 behavior 복사를 끄고 배치 후 개별 설정하는 편이 안전하다.

### 12-5. Batch가 생성되지 않을 때

`NPC batch could not satisfy navigation and minimum spacing`이 나오면 어떤 NPC도 변경되지 않는다.
다음 순서로 조정한다.

1. `Batch Radius`를 키운다.
2. `Batch Count`를 줄인다.
3. `Minimum Spacing`을 조금 줄인다.
4. 벽과 계단에서 떨어진 넓은 바닥으로 중심을 옮긴다.

enabled NPC와 Boss를 합친 world entity 수는 256을 넘을 수 없다.

## 13. Apply, Save, Publish의 차이

세 단계는 반드시 구분한다.

```text
Apply NPC Behavior
  현재 화면 draft를 Map Tool의 Gameplay document에 미리 반영하는 선택 단계

Save Gameplay
  유효한 draft를 자동 적용하고 Data/Worlds/<AreaId>/Gameplay.world.json v6 원본에 저장

Server build/publish + restart
  Server worldbootstrap v7과 Client npcpresentation v2 생성
  새 Server room에서 실제 NPC 이동·행동 실행
```

### 13-1. 편집을 확정하거나 취소하기

- 행동 편집을 유지: `Apply NPC Behavior`
- 현재 draft를 마지막 Apply 상태로 되돌리기: `Revert NPC Behavior`
- batch 유지: `Confirm NPC Batch`
- batch 취소: `Discard NPC Batch Ghost`
- 파일에 기록: `Save Gameplay`
- 저장 파일을 다시 읽기: `Reload Gameplay`

행동 draft가 수정된 상태에서도 `Save Gameplay`는 유효성을 검사한 뒤 자동 적용하여 저장한다. draft가
불완전하면 상세 오류와 함께 저장하지 않고 기존 파일을 유지하므로 입력을 고치거나 Revert한다.
`Reload Gameplay`와 placement 선택 변경은 draft를 잃지 않도록 계속 막힌다. Batch ghost가 있으면
Confirm 또는 Discard를 먼저 한다.

`Reload Gameplay`은 마지막으로 디스크에 저장한 Gameplay 파일을 다시 읽는다. behavior draft나 ghost가
아니더라도 아직 Save하지 않은 일반 Position, Yaw, Enabled 변경은 사라지므로 되돌릴 의도가 있을 때만
사용한다.

`Revert NPC Behavior`는 마지막 디스크 저장본이 아니라 마지막 Apply된 인메모리 behavior로 돌아간다.
이미 Apply한 변경까지 파일 저장본으로 되돌리려면 Save 전에 `Reload Gameplay`을 사용해야 하며, 이때
다른 unsaved Gameplay 변경도 함께 사라진다.

## 14. 저장 후 실제 게임에 반영하기

### 14-1. 가장 쉬운 방법

1. Map Tool에서 `Save Gameplay`를 누른다.
2. navigation도 수정했다면 `Navigation -> Save Navigation`을 누른다.
3. Map Editor와 실행 중인 Server/Client를 종료한다.
4. Visual Studio에서 Server를 Build한다. Server pre-build가 gameplay와 navigation publisher를 실행한다.
5. `Server + Client` profile을 다시 `Ctrl+F5`로 실행한다.
6. Lobby에서 `Bern`으로 들어간다.

### 14-2. Publisher를 직접 확인할 때

저장 원본 검증:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
```

제품 runtime 문서 생성:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
```

navigation도 바꿨다면 다음 검증을 함께 실행한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
```

publish 뒤에도 이미 실행 중인 Server는 자동으로 데이터를 다시 읽지 않는다. Server를 반드시
재시작한다.

## 15. Map Editor Preview와 실제 Bern 확인 차이

| 확인 위치 | 확인할 수 있는 것 | 확인할 수 없는 것 |
|---|---|---|
| Lobby `Test` Map Editor | 모델, 실제 보유 clip, 수동 preview, route 선, batch ghost | Server가 계산하는 실제 순찰·배회와 자동 action timing |
| Lobby `Bern` 제품 맵 | Server 위치·방향, 충돌, 순찰·배회, action 자동 전환과 Client animation | Map Tool 편집 UI |

Map Editor에서 `Preview Action`이 잘 보였더라도 제품 runtime에서 최종 이동속도, 간격, 충돌과 행동
timing을 다시 확인한다.

## 16. 추천 시작 설정

### 16-1. 제자리 경비병

```text
Behavior Mode       Stationary
Action Selection    Weighted
Start Delay         0~2000
Idle Minimum        2000
Idle Maximum        5000
Actions             기본 경계 60 / 주변 확인 30 / 자세 정리 10
```

### 16-2. 광장 순찰 NPC

```text
Behavior Mode       Patrol
Route Mode          Ping Pong
Move Speed          1.2
Start Delay         500~2000
Idle Minimum        1000
Idle Maximum        2500
Waypoints           3~5개
Waypoint Wait       500~1500
```

### 16-3. 시장 군중 배회

```text
Behavior Mode       Wander
Move Speed          0.9~1.2
Wander Radius       3~6
Idle Minimum        1500
Idle Maximum        4000
Random Seed         NPC마다 다르게
```

## 17. 자주 발생하는 오류

| 화면 메시지·증상 | 원인 | 해결 방법 |
|---|---|---|
| Map Tool 버튼이 비활성화됨 | Debug Map Editor workspace가 아님 | `x64 Debug -> Lobby -> Test -> F1` 순서로 진입 |
| `Draft is incomplete` | 이동 mode에 walk clip, waypoint 또는 radius가 부족 | Patrol은 walk clip+2 waypoint, Wander는 walk clip+radius 설정 |
| `NPC patrol waypoint is unreachable` | 시작점과 waypoint 사이 navigation이 끊김 | waypoint를 연결된 walkable 바닥에 다시 찍기 |
| `Moving NPC spawn is outside walkable navigation` | NPC 시작점이 이동 가능한 cell이 아님 | placement Position을 walkable 바닥으로 이동 |
| `clip missing from this animation set` | 선택 모델에 해당 clip이 없음 | 그 모델 dropdown에 실제 표시되는 clip으로 교체 |
| `NPC look target must reference another enabled NPC` | 자기 자신·비활성·없는 NPC를 참조 | 다른 enabled NPC를 선택하거나 `<none>` 사용 |
| Save가 되지 않음 | behavior draft가 불완전하거나 batch ghost 존재 | 표시된 draft 오류를 수정하거나 Revert하고, ghost는 Confirm/Discard 수행 |
| Reload/placement 선택이 되지 않음 | 저장하지 않은 behavior draft 존재 | Save, Apply 또는 Revert로 draft를 먼저 정리 |
| `batch ghost is stale` | ghost 생성 뒤 Gameplay 문서가 변경됨 | Discard 후 다시 Stage |
| `batch exceeds the 256 world entity budget` | enabled NPC+Boss 상한 초과 | 기존 entity 또는 batch count 감소 |
| Map Editor에서는 보이지만 Bern에서 안 움직임 | 저장만 했거나 Server를 재시작하지 않음 | Publish/Server Build 후 Server+Client 재시작 |
| 여러 NPC가 똑같이 움직임 | seed와 timing이 동일 | Random Seed와 Start Delay를 NPC별로 다르게 설정 |

## 18. 저장되는 파일

Bern 기준 원본과 생성물은 다음과 같다.

```text
저작 원본
Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json

Server 생성물
Server/Bin/DataFiles/World/BERN.worldbootstrap

Client 생성물
Client/Bin/DataFiles/World/BERN.npcpresentation.json
```

직접 편집해야 하는 정본은 `Data/Worlds/.../Gameplay.world.json`이며 보통 Map Tool로 저장한다.
`Server/Bin`과 `Client/Bin`의 생성물은 직접 수정하지 않고 publisher로 다시 만든다.

## 19. 현재 기능 경계

현재 Tool에서 생활·작업·대화처럼 보이는 animation은 설정할 수 있다. 다음 기능은 별도 gameplay
구현이 필요하다.

- 실제 대화창과 선택지
- 상점·교환 command
- 퀘스트 상태 반응
- 플레이어 접근 event에 따른 행동 우선순위 전환
- 의자, 책, 술잔, 빗자루 같은 prop socket 자동 부착
- 두 NPC action timeline의 완전한 동기화

따라서 현재는 `걷는 NPC`, `배회하다 멈춰 행동하는 NPC`, `제자리 작업 NPC`, `서로 바라보며 대화
animation을 하는 NPC`까지 같은 시스템으로 저작할 수 있다.

## 20. Quick NPC Brush로 여러 명을 빠르게 배치

반복 배치는 기존 `Arm World Placement`와 수동 ID 입력 대신 다음 순서만 사용한다.

1. `World Gameplay -> NPC`에서 배치할 `NPC Archetype`을 고른다.
2. 아무 설정도 복사하지 않을 때는 바로 `Start Continuous NPC Brush`를 누른다.
3. 같은 idle/이동/action을 복사하려면 먼저 설정이 끝난 기존 NPC 행을 선택하고
   `Capture Selected NPC As Brush Preset`을 한 번 누른다.
4. `Start Continuous NPC Brush`를 누른 뒤 world 바닥을 원하는 수만큼 계속 클릭한다.
5. 끝나면 `Esc` 또는 `Stop Continuous NPC Brush`를 누르고 `Save Gameplay`를 한 번 누른다.

각 click은 `npc.<area>.<archetype>.001`, `.002`처럼 비어 있는 ID를 자동 할당한다. preset은
캡처 순간의 idle clip, Stationary/Patrol/Wander, waypoint, action과 loop 설정을 복사한다. 새로
배치된 NPC가 선택되어도 캡처한 preset은 바뀌지 않는다. `Brush Random Yaw`를 끄면 모두 0도,
켜면 placement ID 기준의 결정적인 서로 다른 방향을 쓴다.

Patrol preset은 새 배치 위치만큼 경로 전체를 평행이동하고 각 waypoint 높이를 navigation에 다시
맞춘다. 경로가 nav 밖이거나 막혀 있으면 그 click 하나만 거부하고 기존 NPC와 이전 click 결과는
유지한다. Wander도 시작 cell과 반경 안의 도달 가능한 공간을 확인한 뒤에만 배치한다.

## 21. 애니메이션을 한 번에 적용

기존의 `Enable Behavior -> Mode -> Walk Clip -> timing -> Apply` 순서를 생략하려면 NPC 행을 선택한
뒤 `NPC Preview Animation`에서 실제 clip을 재생해 보고 `Easy NPC Setup`을 사용한다.

- `1-Click: Stationary Idle Loop`: 현재 Preview Clip을 계속 반복하는 제자리 NPC로 만든다. 기존 이동
  behavior는 제거한다.
- `1-Click: Wander With Preview Walk`: 현재 Preview Clip을 walk loop로 쓰고, `Easy Wander Radius`,
  `Easy Move Speed`, 기본 대기 1~3초와 placement별 seed까지 한 번에 만든다.

두 버튼은 모델에 실제 존재하는 현재 Preview Clip만 사용한다. clip 이름을 추측해 자동 선택하지
않는다. 적용 뒤 같은 설정을 여러 NPC에 찍고 싶으면 바로 `Capture Selected NPC As Brush Preset`을
누르고 Quick NPC Brush를 시작한다.
