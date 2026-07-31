# LostArk NavGrid·NavPathFollower·Character·Valtan 이동 구조 계획서

- 작성일: 2026-07-31
- 갱신일: 2026-07-31
- 기준 커밋: `origin/main` `4096bdc` (`feature/valtan-navgrid-runtime` PR #19 병합)
- 작업 브랜치: `codex/valtan-nav-movement-architecture-plan`
- 대상 레벨: `LEVEL::ASSET_TEST`
- 문서 유형: 시스템 이해·구조 결정 계획서
- 이번 문서 범위: 현재 구현을 실제 코드와 `.navgrid` 값으로 닫고, 플레이어 좌클릭 이동과
  발탄 추적·패턴 이동이 같은 Navigation 기반을 사용하도록 책임과 적용 순서를 확정한다.
- 구현 범위: 이 문서에서는 C++를 수정하지 않는다. 아래 단계별 완료 게이트를 통과할 때
  기존 문서를 구현 계획으로 갱신하고 변경되는 완전한 코드 단위를 확정한다.

> 결론부터 말하면 `CNavPathFollower`는 좌클릭이나 보스 패턴을 판단하는 클래스가 아니다.
> 좌클릭을 해석한 플레이어와 타깃·패턴을 판단한 발탄이 같은 follower에 이동 목표를
> 전달한다. follower는 경로 요청의 성공 결과를 보관하고 Transform을 따라가게 하는
> Engine 공용 실행기다.

## 1. C1~C8 관점

| 관점 | 이번 작업에서 확인한 사실 | 중요도 |
|---|---|---:|
| C1 기준계 | baker, `.navgrid`, `CNavGrid`, 피킹, Character, Valtan은 모두 같은 월드 X/Z와 meter 단위를 쓴다. `Cell_ToWorld`만 셀 중심과 저장 height를 복원한다. | ★★★ |
| C2 이동>계산 | glTF·placement 결합, 경사 판정, XZ rasterize는 Python에서 한 번 끝낸다. 런타임은 World→Cell, A*, waypoint 소비만 수행한다. | ★★★ |
| C3 공유는 비싸다 | `CNavGrid`는 Navigation Prototype과 clone이 공유하고, `CPathFinder` scratch와 `CNavPathFollower` waypoint는 actor 인스턴스마다 분리한다. | ★★★ |
| C4 수명은 선언된다 | Loader가 Navigation/Character/Valtan Prototype을 만들고, Level이 clone을 배치한다. Character와 Valtan은 각자 Navigation clone과 follower를 소유한다. | ★★★ |
| C5 이산화와 오차 | `floor((world-origin)/cellSize)`가 연속 좌표를 한 셀에 귀속한다. 셀 크기는 0.5m이고 goal의 Y가 아니라 저장된 cell height가 최종 waypoint Y다. | ★★★ |
| C6 가지치기 | 범위 밖, non-walkable, 높이 차 초과, 대각 corner cut, 확장 한도 초과를 A* 전에 또는 탐색 중 제외한다. 입력·AI·패턴은 follower 안으로 넣지 않는다. | ★★★ |
| C7 권위와 정합성 | floor mesh와 placement가 bake 입력 정본이고 `.navgrid`는 파생 runtime 데이터다. Client actor의 현재 path는 저장 정본이 아닌 일시 상태다. | ★★★ |
| C8 검증이 병목 | binary 독립 검산, Engine/Client 빌드, 좌클릭 이동, 발탄 추적, 실패 시 기존 경로 정책, 레벨 재진입까지 직접 확인해야 한다. | ★★★ |

핵심 축은 `오프라인 공간 해석`, `공유 grid/인스턴스별 탐색 상태`, `Engine 이동 원시 기능`,
`Client 의도 결정` 네 가지다.

## 2. 문제 해결 ①~⑤

① 문제·제약: 현재 NavGrid와 A*는 동작하지만 AssetTest는 플레이어가 없고 좌클릭이
발탄의 `Request_Move()`를 직접 호출한다. `CValtan`의 target chase 코드는 존재하지만
`pTargetTransform`이 전달되지 않아 실제 플레이어 추적은 실행되지 않는다.

② 단순 해법의 문제: follower가 좌클릭, 피킹, 플레이어 찾기, 보스 패턴, 애니메이션까지
처리하면 Engine이 Client 게임 규칙을 알게 된다. 반대로 Character와 Valtan이 waypoint
추종 코드를 각각 가지면 같은 역할의 두 번째 runtime 경로가 생긴다.

③ 해결 방식: `CNavPathFollower`는 Engine에 유지하고 path 요청·성공 결과 commit·waypoint
소비만 담당한다. Level은 좌클릭을 world goal로 바꾸고 Character에 전달한다. `CValtan`은
weak target을 보고 chase 여부와 재탐색 시점을 결정한 뒤 같은 follower를 사용한다.

④ 비교: 최초 `CValtanNavigationRuntime` 구상은 입력, ImGui, debug draw, Valtan Transform,
속도까지 한 Client 클래스에 묶은 기능 검증 harness였다. 현재 구조는 그중 재사용 가능한
path follow만 Engine으로 추출하고 Valtan/입력 규칙을 Client에 남겼다.

⑤ 대가: Character와 Valtan이 Navigation clone과 follower를 각각 하나씩 가진다. 대신
불변 grid는 공유되고 A* scratch/path cursor만 분리된다. follower를 Component나 범용 AI
framework로 승격하는 것은 actor 수·요구가 실제로 늘어날 때 다시 판단한다.

## 3. 자료구조·알고리즘 핵심

### 3.1 현재 `.navgrid`

실제 `Client/Bin/DataFiles/Navigation/ValtanArena.navgrid`를 독립적으로 읽은 값이다.

```text
파일 크기: 19,550 bytes
width × height: 62 × 63 = 3,906 cells
cellSize: 0.5m
origin X/Z: (140.5, -137.5)
walkable: 2,843
non-walkable: 1,063
walkable height: 20.95236 ~ 23.29906
SHA-256: 7FDCEAA76A24AFF6D58AB1904C28D0CA99D779DAE486938BEA281EB8E0275638
```

현재 binary는 다음 순서이며 magic/version/checksum은 없다.

```text
uint32 width
uint32 height
float cellSize
float originX
float originZ
uint8 walkable[width * height]
float height[width * height]
```

### 3.2 핵심 owner와 수명

| 상태 | 타입 | owner | writer | reader | 수명·불변식 |
|---|---|---|---|---|---|
| 공간 격자 | `shared_ptr<const CNavGrid>` | Navigation Prototype에서 만들고 clone들과 공유 | Python baker가 파일 작성, `CNavGrid::Load`가 메모리 commit | `CNavigation`, `CPathFinder` | 레벨 Prototype 수명. runtime에서 불변 |
| A* scratch | `unique_ptr<CPathFinder>` | Navigation Prototype 또는 각 clone | 해당 clone의 `Find_Path` | 같은 Navigation clone | actor별 분리. query generation으로 재사용 |
| actor 경로 | `vector<float3_t>` | actor 안의 `CNavPathFollower` | 성공한 `Request_Path`만 commit | follower `Update` | 다음 성공 요청·Cancel·actor 파괴까지 |
| path cursor | `size_t m_iNextWaypoint` | `CNavPathFollower` | Request/Update/Cancel | Has_Path/Update | `0 <= cursor <= waypoints.size()` |
| 플레이어 입력 edge | `bool m_bLeftMouseDown` | `CLevel_AssetTest` | Level Update | Level Update | AssetTest 수명. follower가 읽지 않음 |
| 발탄 target | `weak_ptr<CTransform>` | `CValtan` | clone desc로 한 번 설정 | Valtan Update | target actor보다 오래 소유하지 않음 |
| 발탄 chase 상태 | timer/last goal/state | `CValtan` | Valtan Update | Valtan/Body | actor 수명. Engine이 의미를 모름 |

### 3.3 `CNavPathFollower`

```text
표현하는 상태:
  마지막 성공 경로의 world waypoint, 다음 waypoint index,
  마지막 query 결과와 expanded node 수.

선택 이유:
  Character와 Valtan이 같은 path commit/소비 절차를 재사용한다.

owner:
  각 Client actor의 값 멤버. 별도 Prototype/Clone이 아니다.

writer:
  Request_Path, Update, Cancel.

reader:
  actor의 상태 전환, debug 통계.

유지해야 할 불변식:
  실패한 새 query는 현재 성공 경로를 덮지 않는다.
  성공한 경로만 한 번에 commit한다.
  다음 waypoint는 start point를 건너뛰기 위해 보통 index 1에서 시작한다.

규모:
  현재 최대 grid cell 3,906개보다 클 수 없다.

프레임당 빈도:
  Update 1회/actor.
  플레이어 query는 좌클릭 edge마다 1회.
  발탄 query는 최대 약 2.86회/초(0.35초 주기)다.
```

### 3.4 `CPathFinder::Find_Path`

```text
입력:
  불변 CNavGrid, start/goal cell index, maxStepHeight, maxExpandedNodes.

출력:
  PATH_RESULT_CODE, start→goal 순서의 cell index path, expanded node 수.

처리:
  query 검증
  -> generation 시작
  -> binary heap open set
  -> 8방향 이웃 검사
  -> walkable/step/corner-cut 가지치기
  -> goal에서 parent 역추적
  -> reverse
  -> 결과 반환.

종료:
  goal pop, open set 고갈, expansion limit.

시간 복잡도:
  O((V + E) log V), 현재 V=3,906, E<=8V.

공간 복잡도:
  O(V). `m_Nodes`와 heap vector는 query 사이에 재사용한다.

실패 전파:
  명시적 `PATH_RESULT_CODE`. 실패 결과에는 소비 가능한 새 path가 없다.
```

`reverse`는 `CNavigation`이나 actor가 하는 것이 아니다.
`CPathFinder::Build_Result()`가 goal에서 parent를 따라 start까지 넣은 뒤 내부에서 수행한다.

### 3.5 실제 값 하나의 전체 흐름

현재 중앙 셀은 다음 값이다.

```text
cell (31,31)
index 31 * 62 + 31 = 1953
world (156.25, 22.99751, -121.75)
walkable = 1
```

플레이어 검증 spawn 후보는 다음 값이다.

```text
cell (21,31)
index 1943
world (151.25, 22.96835, -121.75)
walkable = 1
```

동쪽 goal 후보 `(161.25, 22.92836, -121.75)`는 cell `(41,31)`, index `1963`이다.
같은 row의 `(37,31)`은 non-walkable이므로 A*는 직선 index 나열이 아니라 주변 row로
우회해야 한다.

```text
LMB edge
 -> Picking world goal (161.25, *, -121.75)
 -> CLevel_AssetTest::Update_ClickMove
 -> CCharacter::Request_Move
 -> Character의 CNavigation::Find_Path
 -> World_ToCell: 1943 -> 1963
 -> CPathFinder A*: cell (37,31)을 제외하고 parent 기록
 -> Build_Result에서 reverse
 -> CNavigation이 cell path를 world waypoint로 변환
 -> simplify/round가 통과한 결과만 반환
 -> CNavPathFollower가 성공 경로를 commit
 -> 매 frame speed * delta만큼 Character Transform 이동
 -> path 종료 시 run -> idle
```

## 4. Git과 구현 상태

### 4.1 Git 동기화

`git fetch origin` 뒤 확인한 상태다.

```text
origin/main: 4096bdc Merge pull request #19
feature/valtan-navgrid-runtime: 24ef7ee
두 tree의 diff: 없음
차이: origin/main에 merge commit 1개가 더 있음
```

로컬 `main`은 `origin/main`으로 fast-forward했다. 계획서 변경은 병합이 끝난 feature
브랜치를 재사용하지 않고 `codex/valtan-nav-movement-architecture-plan`에서 수행한다.

### 4.2 완료된 부분

- Python baker가 floor geometry와 placement를 월드 삼각형으로 복원한다.
- CUL_BOX로 grid 범위를 결정한다.
- `.navgrid`를 원자적 임시 파일 교체 방식으로 저장한다.
- `CNavGrid` load와 World↔Cell 변환이 존재한다.
- `CPathFinder` 8방향 A*, 높이 차, corner-cut 방지가 존재한다.
- `CNavigation::Create_NavGrid()`와 Prototype/Clone 경로가 존재한다.
- `CNavPathFollower`가 Engine에 등록되고 Engine DLL public API로 노출된다.
- `CValtan`이 Navigation clone과 follower를 소유한다.
- `CValtan`에 target chase, 0.35초 repath, 2.5m stop, idle/run 전환이 존재한다.

### 4.3 아직 연결되지 않았거나 남은 부분

- AssetTest에 `CCharacter`가 spawn되지 않는다.
- 입력은 좌클릭이다.
- 좌클릭 goal의 소비자는 Character가 아니라 Valtan이다.
- Valtan desc의 `pTargetTransform`은 현재 null이라 자동 chase가 실행되지 않는다.
- `CValtan::Request_Move()`는 click test와 boss navigation 양쪽 의미로 쓰이고 있다.
- follower의 `LookAt`은 즉시 회전이라 `fRotationPerSec=180`을 실제 회전에 사용하지 않는다.
- `CNavigation` Debug는 non-walkable 셀의 표시 높이를 주변 평균으로 만들어 낸다.
  A* 데이터는 바꾸지 않지만 화면이 저장 근거처럼 보일 수 있다.
- F1 수동 NavGrid paint/source 분리는
  `2026-07-31_LOSTARK_VALTAN_NAVGRID_DEBUG_SMOOTH_FACING_PLAN.md`에 계획만 있고 현재 코드에는 없다.
- `.navgrid`에는 magic/version/checksum과 trailing-byte 검증이 없다.

## 5. 현재 수직 호출 흐름

### 5.1 오프라인 bake

```text
Floor01/A/B glTF
  + overlay의 각 2개 placement
Main floor glTF
  + mapplacements의 4개 placement
Center floor glTF
  + mapplacements의 1개 placement
 -> 실제 walkable 후보 월드 삼각형

CUL_BOX_8 glTF
  + mapplacements의 1개 placement
 -> XZ grid bounds만 결정

월드 삼각형
 -> slope <= 45°
 -> 셀 중심 barycentric 포함 검사
 -> 겹친 표면 중 가장 높은 Y
 -> walkable byte + height float
 -> ValtanArena.navgrid
```

따라서 “CUL_BOX를 굽는가, Floor A/B와 중앙을 굽는가?”의 답은 다음과 같다.

> CUL_BOX는 범위만 정한다. 실제 Walkable과 height는 Floor01/A/B, main floor 4분할,
> center floor의 월드 삼각형을 굽는다.

### 5.2 Loader와 Component clone

```text
CLoader::Ready_For_Level_AssetTest
 -> CNavigation::Create_NavGrid
 -> Initialize_NavGrid_Prototype
 -> CNavGrid::Load
 -> Navigation Prototype 등록

CValtan::Ready_Components
 -> Add_Component
 -> Navigation Prototype Clone
 -> shared_ptr<const CNavGrid> 공유
 -> clone 전용 CPathFinder 생성
```

Character 연결 뒤에도 같은 Prototype을 clone한다. grid를 다시 읽거나 복제하지 않는다.

### 5.3 Path query

```text
actor current Transform + world goal
 -> CNavPathFollower::Request_Path
 -> CNavigation::Find_Path
 -> World_ToCell
 -> CPathFinder::Find_Path
 -> cell result reverse
 -> Cell_ToWorld
 -> start/goal XZ 보정
 -> line-of-sight simplify
 -> corner rounding
 -> staged world waypoints
 -> success일 때만 follower path 교체
```

### 5.4 현재 AssetTest 소비

```text
LMB edge
 -> CGameInstance::Picking
 -> CLevel_AssetTest
 -> CValtan::Request_Move
 -> Valtan follower
 -> Valtan Transform
```

이 흐름은 NavGrid 기능 검증용으로는 동작하지만 최종 플레이 구조는 아니다.

### 5.5 현재 Valtan target chase

target이 실제로 연결됐다고 가정하면 다음 흐름이다.

```text
CValtan::Update
 -> weak target lock
 -> XZ distance <= 2.5m이면 path cancel + IDLE
 -> 0.35초마다 target goal 변화량 검사
 -> 0.5m 이상 이동했거나 path가 없으면 Request_Move
 -> follower Update
 -> path가 남으면 CHASE/run, 끝나면 IDLE
```

코드는 존재하지만 현재 Level이 target을 전달하지 않아 이 경로는 비활성이다.

## 6. NavPathFollower를 Client에서 시작한 이유와 Engine으로 옮겨도 된 이유

### 6.1 Client에서 시작한 이유

최초 계획의 `CValtanNavigationRuntime`은 다음 책임을 한 파일에 가지고 있었다.

- Valtan이라는 게임 고유 이름.
- LMB 입력과 피킹.
- ImGui panel.
- debug grid/path rendering.
- agent Transform.
- move speed와 arrival distance.
- Navigation 생성과 path query.
- waypoint follow.

이 클래스는 Client 규칙과 debug UI를 직접 알았으므로 Client가 맞았다. 당시 목적은
재사용 엔진 기능보다 발탄 아레나에서 end-to-end 동작을 빨리 검증하는 것이었다.

### 6.2 추출 뒤 Engine이 맞는 이유

현재 `CNavPathFollower`는 다음 Engine 타입만 안다.

- `CNavigation`
- `CTransform`
- `PATH_RESULT_CODE`
- world waypoint와 delta time
- profiler counter

반대로 다음은 전혀 모른다.

- `CValtan`, `CCharacter`, `LEVEL::ASSET_TEST`
- 마우스 버튼과 피킹
- player target과 boss pattern
- animation clip
- ImGui

따라서 dependency는 `Client actor -> Engine follower -> Engine Navigation/Transform` 한 방향이다.
Engine이 Client header를 include하지 않으므로 순환 의존도 없다. `ENGINE_DLL`로 export하고
Engine 프로젝트에 등록한 뒤 `UpdateLib`로 header/lib를 배포했기 때문에 Client 링크에도
문제가 없다.

### 6.3 현재 위치에 대한 최종 결정

`CNavPathFollower`는 Engine에 유지한다. 다만 의미는 아래 한 문장으로 제한한다.

> 성공한 Navigation path를 actor별로 보관하고, 시간과 속도에 따라 Transform이 waypoint를
> 따라가게 한다.

입력, 타깃 선택, 패턴 선택, 애니메이션 선택을 추가하지 않는다. 별도 Component로도
승격하지 않는다. 현재는 actor 값 멤버가 가장 작은 올바른 수명 경계다.

## 7. 권장 책임 구조

```text
[Client 입력]
CLevel_AssetTest
  LMB edge + Picking
        |
        v
CCharacter::Request_Move(worldGoal)
  Navigation clone + NavPathFollower
        |
        v
Character Transform + idle/run

[Client 보스 판단]
CValtan
  weak player target
  chase/stop/repath/pattern state
        |
        v
CValtan::Request_Move(worldGoal)
  Navigation clone + NavPathFollower
        |
        v
Valtan Transform + idle/run

[Engine 공용]
CNavigation
  CNavGrid(shared immutable)
  CPathFinder(per clone scratch)
        |
        v
CNavPathFollower(per actor path cursor)
```

### 7.1 각 클래스의 역할과 비역할

| 클래스 | 역할 | 비역할 |
|---|---|---|
| `CNavGrid` | grid desc, walkable, height, 좌표 변환 | A*, 입력, actor 이동 |
| `CPathFinder` | cell graph의 8방향 A* | 파일 I/O, Transform, 게임 상태 |
| `CNavigation` | Component 수명, mode 검증, world query façade, path 후처리 | 마우스, target, animation |
| `CNavPathFollower` | 성공 path commit, cursor, Transform follow | player/boss 구분, 패턴 판단 |
| `CLevel_AssetTest` | 검증용 spawn, LMB edge, Picking, target 연결 | A*, waypoint, actor 상태 |
| `CCharacter` | 자기 Navigation/follower, 이동 상태와 공통 locomotion | 직접 마우스 polling, 발탄 AI |
| `CValtan` | target 거리, chase/repath, 패턴별 이동 정책, animation state | input polling, grid 파일 파싱 |
| `CBody_Valtan` | parent state에 맞는 model animation clock과 render | target, A*, path cursor |
| `CLoader` | Prototype과 불변 resource 등록 | 실제 actor spawn, target 연결 |

## 8. 최종 runtime 흐름

### 8.1 플레이어 좌클릭 이동

```text
사용자 LMB
 -> Level이 input blocked와 rising edge 확인
 -> Picking 성공
 -> CCharacter::Request_Move(worldGoal)
 -> follower가 Character의 Navigation clone에 query
 -> 성공 path만 교체
 -> Character::Update에서 follower 이동
 -> 이동 시작 시 RUN, 종료/Cancel 시 IDLE
```

Level이 follower를 직접 만지지 않고 Character의 명령 API만 호출한다.

### 8.2 발탄 플레이어 추적

```text
Level이 Character Transform을 Valtan desc에 전달
 -> Valtan은 weak_ptr로 보관
 -> Valtan Update가 거리와 repath 조건 판단
 -> chase가 필요할 때 target world position으로 Request_Move
 -> Valtan follower가 같은 Engine 경로를 소비
 -> stop distance 진입 시 Cancel + IDLE
```

Level은 매 프레임 플레이어 위치를 보스에게 밀어 넣지 않는다. target Transform의 수명만
연결하고 추적 정책은 Valtan이 가진다.

### 8.3 보스 패턴 이동

모든 패턴 이동을 A* follower 하나로 강제하지 않는다.

| 이동 종류 | owner | follower 사용 |
|---|---|---:|
| 일반 추적·재배치·지정 지점 접근 | `CValtan` chase/pattern 판단 | 사용 |
| 직선 돌진·밀치기·root motion 공격 | 해당 Valtan pattern state | 기본적으로 미사용 |
| 순간이동 | 해당 pattern state | 미사용 |
| 공격 중 위치 고정 | 해당 pattern state | follower Cancel |

직선 돌진은 telegraph 방향, 충돌 sweep, 낙사 경계 같은 규칙이 A* waypoint 추종과 다르다.
패턴 state가 시작될 때 기존 follower를 Cancel하고, 패턴 종료 뒤 chase가 다시 path를
요청한다. 실제 첫 패턴이 구현되기 전에는 movement mode enum이나 범용 BossMovement
클래스를 미리 만들지 않는다.

## 9. 현재 → 변경 → 결과

### 9.1 입력

```text
현재:
  Level LMB -> Valtan Request_Move

변경:
  Level LMB -> Character Request_Move

결과:
  입력은 Client Level, 이동 상태는 Character, 공용 실행은 Engine follower가 소유한다.
```

### 9.2 Character

```text
현재:
  Test2에서 조립·animation만 검증.
  Navigation/follower 없음.

변경:
  AssetTest에서도 같은 Character Prototype 세트를 등록.
  Navigation clone과 CNavPathFollower를 Character 값 멤버로 소유.
  world goal만 받는 Request_Move/Cancel_Move/Is_Moving 계약 추가.

결과:
  예전 CPlayer를 되살리지 않고 새 CCharacter 통합 경로에서 click-to-move가 완성된다.
```

### 9.3 Valtan

```text
현재:
  click test actor이자 target chase 후보.
  target은 실제로 연결되지 않음.

변경:
  Level의 click consumer 역할 제거.
  Character Transform을 target으로 전달.
  기존 chase/repath/follower 흐름을 보스 이동 소비자로만 사용.

결과:
  Valtan은 플레이어 의도가 아니라 보스 AI 의도로 움직인다.
```

### 9.4 Engine

```text
현재:
  NavGrid, PathFinder, Navigation, NavPathFollower가 이미 책임별로 분리됨.

변경:
  1차 Character/Valtan 연결에서는 구조 변경 없음.
  follower에 input/AI API를 추가하지 않음.

결과:
  Engine public header 변경과 두 번째 navigation runtime 경로를 피한다.
```

## 10. 후속 수정 파일과 계약

1차 구현은 새 C++ 파일을 만들지 않는다.

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Loader.h` | Character Prototype 공통 등록 helper 선언 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | Test2 전용 LanceMaster 등록을 level index 기반 helper로 재사용하고 AssetTest에도 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Character.h` | Navigation tag, follower, 이동 명령·상태 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Character.cpp` | Navigation clone, path request/update/cancel, idle/run 전환 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_AssetTest.h` | Character 참조와 LMB edge 상태 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_AssetTest.cpp` | Character spawn, LMB goal 전달, Character Transform을 Valtan target으로 연결 |
| 조건부 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Valtan.h` | 실제 연결 중 기존 `Request_Move` 이름이 pattern 명령과 혼동될 때만 `Request_NavMove`로 명확화 |
| 조건부 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Valtan.cpp` | 이름 변경 또는 query 실패 정책을 Client AI 의미로 명시할 때만 수정 |
| 유지·검증 | `C:/Users/user/Desktop/LostArk/Engine/Public/NavPathFollower.h` | Engine 공용 path follow 계약 유지 |
| 유지·검증 | `C:/Users/user/Desktop/LostArk/Engine/Private/NavPathFollower.cpp` | input/AI를 추가하지 않고 기존 성공 시 commit 정책 검증 |

새 파일이 없으므로 1차 구현에서는 `.vcxproj`와 `.vcxproj.filters` 변경이 없어야 한다.
물리 폴더가 정본이며 기존 등록을 재사용한다.

### 10.1 `CCharacter` 최소 public 계약

구체 코드 작성 전 의미는 아래로 고정한다.

```cpp
PATH_RESULT_CODE Request_Move(fvector_t vGoalPosition);
void Cancel_Move();
bool_t Is_Moving() const;
shared_ptr<CTransform> Get_Transform() const;
```

- `Request_Move`: 입력을 읽지 않는다. 현재 Transform과 world goal로 query한다.
- 성공: follower path를 교체하고 locomotion을 RUN으로 전환한다.
- 실패: 기존 성공 path를 보존하고 실패 code를 호출자에게 반환한다.
- `Cancel_Move`: path를 비우고 locomotion을 IDLE로 전환한다.
- `Is_Moving`: follower path 존재 여부를 반환한다.
- `Get_Transform`: Level이 Valtan target 수명만 연결할 수 있게 typed handle을 제공한다.

`CHARACTER_DESC`에는 기존 level/spec/position과 함께 navigation Prototype tag만 추가한다.
클릭 버튼, target, boss 정보는 넣지 않는다.

### 10.2 Loader 공통 등록

`Ready_For_Test_Level2()`에 복사돼 있는 다음 등록을 level index를 받는 기존 Loader private
helper 하나로 옮긴다.

- LanceMaster body model.
- equipment 5개 model.
- weapon model.
- `CPart_Body`.
- `CPart_Equipment`.
- `CCharacter`.

AssetTest와 Test2의 shader 등록은 이미 각 레벨에 있으므로 helper가 중복 등록하지 않는다.
Loader는 spawn하지 않는다.

### 10.3 Level 연결 순서

```text
Ready_Lights
 -> Ready_Camera
 -> Ready_Character
 -> Ready_Valtan(character Transform)
```

Character 검증 spawn은 실제 walkable 값인
`(151.25, 22.96835, -121.75)`를 사용한다. Valtan은 기존 중앙
`(156.25, 22.99751, -121.75)`를 유지한다.

## 11. 적용 단계

### 단계 0. NavGrid runtime 기준선 고정

- 현재 `.navgrid` hash와 62×63/2,843 값을 기록한다.
- CUL_BOX는 bounds, floor 집합은 walkability라는 설명을 코드와 맞춘다.
- 기존 F1 authoring 개선은 별도 NavGrid 문서에서 진행하고 이 연결 작업과 섞지 않는다.

완료 게이트:

- binary 독립 검산 값이 위 3.1과 일치한다.
- center와 Character spawn cell이 walkable이다.

### 단계 1. Character를 AssetTest에 재사용

- Loader의 Test2 Character Prototype 등록을 level index helper로 묶는다.
- AssetTest에 같은 Prototype을 등록한다.
- `CCharacter`가 Navigation clone과 follower를 소유한다.
- target은 아직 Valtan에 연결하지 않는다.

완료 게이트:

- AssetTest에서 body/equipment/weapon 조립이 Test2와 같다.
- Character가 지정 spawn에 서고 Valtan은 움직이지 않는다.

### 단계 2. LMB click-to-move

- Level의 LMB→Valtan 경로를 제거한다.
- `DIM::LB` rising edge와 input blocked를 확인한다.
- Picking 성공 goal을 Character에 전달한다.
- Character가 follower를 Update하고 idle/run을 전환한다.

완료 게이트:

- LMB 1회당 query 1회다.
- 누르고 있는 동안 매 frame 재탐색하지 않는다.
- walkable goal 성공, non-walkable/영역 밖 실패가 명시된다.
- 실패한 클릭이 현재 성공 경로를 끊지 않는다.

### 단계 3. Valtan target chase

- Level이 Character Transform을 Valtan desc에 전달한다.
- 기존 0.35초 repath와 2.5m stop 흐름을 활성화한다.
- click은 더 이상 Valtan을 직접 움직이지 않는다.

완료 게이트:

- Character가 움직이면 Valtan이 새 goal로 재탐색한다.
- stop distance 안에서는 path가 취소되고 idle이다.
- Character/Valtan의 path와 A* scratch가 서로 덮이지 않는다.
- Character를 파괴하거나 레벨을 이탈할 때 weak target이 안전하게 만료된다.

### 단계 4. 첫 실제 보스 패턴과 이동 정책 연결

첫 패턴 코드가 존재할 때만 진행한다.

- chase가 아닌 이동인지 먼저 분류한다.
- nav 이동이면 기존 `Request_Move`를 사용한다.
- 돌진/root motion/순간이동이면 follower를 Cancel하고 pattern이 이동을 소유한다.
- pattern 종료 시 chase state가 새 path를 요청한다.

완료 게이트:

- 동시에 두 writer가 Valtan Transform을 변경하지 않는다.
- pattern 전환마다 follower 보존/Cancel 정책이 명시된다.
- 패턴 이름이나 state가 Engine header에 등장하지 않는다.

## 12. 빌드·실행 검증

### 12.1 1차 Client 전용 변경

Engine public header를 바꾸지 않는 1차 구현은 기존 EngineSDK가 최신이라는 전제에서 Client
Debug/Release를 빌드한다. 그러나 clean clone 또는 EngineSDK 불확실 시 팀 표준 전체 순서를
사용한다.

```text
1. Engine x64 Debug
2. UpdateLib.bat Debug
3. Client x64 Debug
4. Engine x64 Release
5. UpdateLib.bat Release
6. Client x64 Release
```

### 12.2 Debug 실행

```text
실행 파일: Client/Bin/Debug/Client.exe
working directory: Client/Bin
레벨: LEVEL::ASSET_TEST
```

검증 순서:

1. Character와 Valtan spawn 위치·height를 확인한다.
2. LMB로 직선 goal을 지정하고 Character idle→run→idle을 확인한다.
3. cell `(37,31)`을 사이에 둔 goal로 우회 이동을 확인한다.
4. non-walkable과 grid 밖을 클릭해 기존 path 보존을 확인한다.
5. Valtan target 연결 후 Character를 이동시켜 chase/repath를 확인한다.
6. Character와 Valtan이 서로 다른 path cursor를 갖는지 Watch로 확인한다.
7. F5 debug path가 실제 query clone의 path와 일치하는지 확인한다.
8. 레벨 이탈·재진입 후 stale target/path가 남지 않는지 확인한다.

### 12.3 Breakpoint·Watch

권장 breakpoint:

```text
CLevel_AssetTest::Update_ClickMove
CCharacter::Request_Move
CNavPathFollower::Request_Path
CNavigation::Find_Path
CPathFinder::Build_Result
CNavPathFollower::Update
CValtan::Update
```

Watch:

```text
Character:
  m_pNavigationCom.get()
  m_PathFollower.m_Waypoints.size()
  m_PathFollower.m_iNextWaypoint

Valtan:
  m_pNavigationCom.get()
  m_pTargetTransform.expired()
  m_fRepathTime
  m_PathFollower.m_Waypoints.size()
  m_PathFollower.m_iNextWaypoint

공유/분리 확인:
  Character Navigation의 m_pNavGrid.get()
  Valtan Navigation의 m_pNavGrid.get()       // 같아야 함
  Character Navigation의 m_pPathFinder.get()
  Valtan Navigation의 m_pPathFinder.get()    // 달라야 함
```

## 13. 현재 하지 않는 것

- follower 내부에서 DirectInput 또는 Picking 호출.
- Engine에 `CValtan`, player, boss pattern enum 추가.
- 예전 `CPlayer` navigation 경로 재사용.
- 범용 AI controller, blackboard, behavior tree 선도입.
- `CNavPathFollower`의 Component/Prototype 승격.
- 모든 보스 이동을 A*로 강제.
- 동적 장애물, agent radius, 다층 NavMesh.
- NavGrid authoring 계획과 Character/Valtan 연결을 한 커밋에 혼합.
- 근거 없이 `.navgrid` format을 확장하거나 Recast로 교체.

## 14. 설명 완료 기준

### 14.1 30초 답변

> 발탄 NavGrid는 Python이 floor mesh와 placement를 결합해 미리 굽고, CUL_BOX는 범위만
> 정합니다. 런타임에서는 Navigation Prototype의 불변 grid를 Character와 Valtan clone이
> 공유하고, A* scratch와 follower path는 actor별로 분리합니다. 좌클릭은 Level이
> Character 명령으로 바꾸고, 발탄은 target과 패턴을 Client에서 판단합니다.
> NavPathFollower는 둘의 성공 경로를 공통으로 소비할 뿐 입력이나 AI를 알지 않습니다.

### 14.2 90초 답변

> `CNavGrid`는 어디를 걸을 수 있는지, `CPathFinder`는 어떤 cell 순서로 갈지,
> `CNavigation`은 world 좌표 query와 Component 수명을, `CNavPathFollower`는 actor별
> waypoint 실행을 담당합니다. A*는 goal에서 parent를 역추적한 뒤 PathFinder 내부에서
> reverse하므로 외부 `OutPath`는 start에서 goal 순서입니다. 최초 Client runtime은
> 입력·ImGui·Valtan Transform까지 묶인 검증 harness라 Client가 맞았지만, 그중
> Navigation과 Transform만 사용하는 follower를 분리한 뒤에는 Engine 공용 기능이
> 됐습니다. 최종 구조에서는 Level이 LMB와 Picking만 처리하고 Character에 goal을
> 전달합니다. Valtan은 weak player target을 보고 chase/repath를 결정해 같은 follower를
> 사용합니다. 다만 돌진이나 root motion 같은 패턴 이동은 A*와 규칙이 다르므로 패턴이
> follower를 취소하고 직접 소유합니다.

### 14.3 최종 한 문장

> Engine은 길과 경로 실행 방법을 알고, Client의 Character와 Valtan은 누가 왜 어디로
> 이동할지를 결정한다.
