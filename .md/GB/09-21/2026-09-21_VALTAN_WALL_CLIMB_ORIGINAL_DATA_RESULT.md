# 발탄 벽타기(Stage_3) — 원본 데이터 조사 결과와 구현안

작성일: 2026-09-21
요청: "Stage_3 벽타기가 원본에서는 어떻게 처리되어 있는지 먼저 원본 데이터에서 찾아보고, 그걸 바탕으로 우리 쪽에서 어떻게 구현할지 정하자."
범위: **읽기 전용 조사와 설계**다. 게임 데이터·코드·Resources를 바꾸지 않았고 빌드·publisher·Client/Server 실행·git 상태 변경도 하지 않았다. 새로 만든 것은 이 문서, `out/ValtanWallClimb20260921/wall_climb_original.json`, `C:\LostArkExtract\ValtanClimb_20260921\` 아래 스크립트·추출물뿐이다.

근거 표기: **[실측]** 이번 세션에서 원본 파일이나 저장소 코드를 직접 읽고 계산한 값, **[추정]** 실측 값에서 추론했지만 원본 실행 코드를 못 봐서 확정하지 못한 것, **[미확인]** 확인하지 못한 것.

## 1. 결론

원본의 벽타기는 **"위치는 미리 만들어 둔 시간표(TrackMove 트랙)로 움직이고, 그 위에 플레이어의 등반 애니메이션을 얹는 방식"**이다. 스킬처럼 애니메이션 루트 모션이 캐릭터를 올리는 방식이 아니라고 보는 이유는 아래와 같다.

- 발탄 존(37051)의 `Stage_3`에 해당하는 것은 `TriggerMapData.loa`의 트리거 유닛이 아니라 **배치된 "볼륨 프랍"** 하나(`DeployData` 액터 `0x10000026`)다. 이 프랍의 정의(Prop 375319)는 상호작용 키 아이콘이 `climb`, 문구가 `tip.name.interactionkey_rise_1`("오르기")이다 **[실측]**.
- 프랍 레코드 안 `+0x1BC` 값이 **TrackMove 트랙 번호**다. 이 프랍은 3번이고 `TrackMove.loa`의 트랙 3의 시작 좌표가 프랍 좌표와 x·y는 같고 높이만 0.2cm 다르다(트랙 1·4는 각각 높이 1.2cm·5.2cm 차이) **[실측]**. 시작 부분의 뛰어넘기 프랍 두 개(트랙 1·4)도 같은 규칙으로 맞물린다 **[실측]**.
- 트랙 3은 **6.0초, 30Hz 180키, 총 5.09m 상승**하는 위치 시간표다. 앞 0.43초에 벽 쪽으로 0.78m 붙고, 3.57초 동안 초속 0.83m로 수직 상승하고, 마지막 2.0초에 턱을 넘어 올라선다 **[실측]**.
- 트랙은 4.0초를 지나면서(키 119→120 구간) 올라가는 속도가 초속 0.83m에서 1.14m로 바뀌고, 키 119부터 끝까지가 61개 키(60구간, 2.0초)다. 이것이 6가지 클래스 공통의 `act_creep_up_end_1` 클립 프레임 수(61)와 같다. 가운데 등반 루프 `act_creep_up_1`은 루트 이동이 0인 제자리 클립이다 **[실측]**. 그런데 `act_creep_up_end_1`의 루트 상승량은 클래스마다 1.68~1.91m로 다르고 트랙은 클래스와 무관한 하나의 값이므로 **위치의 주인은 트랙이고 클립은 표현**이라고 판단한다 **[추정, 값으로 뒷받침]**.

우리 `Stage_3`(0.8초, 직선, 누른 자리에서 시작, 착지 고정)와 비교하면 시간이 7.4배 빠르고 경로 모양과 시작점이 다르다. 애니메이션은 지금 점프 클립이 붙는다.

권장 구현은 원본과 같은 구조를 그대로 옮기는 **"TrackMove 시간표 + 스타일 태그 + 등반 클립"** 이다(5장의 안 C). 서버는 시간표로 위치를 정하고, Client는 표현만 한다.

## 2. 어디를 어떻게 뒤졌는가

| 경로 | 결과 |
|---|---|
| `ZoneBase.db` | 발탄 레이드 존 37051(`LevelFileName=LV_LUT_HeartRB_ED_PS`), 같은 레벨을 쓰는 37052·37811 확인 [실측] |
| `leveldata1/2.lpk`의 `MapData/37051` | `TriggerMapData.loa` 83개 유닛, `DeployData.loa` 프랍 169개, `TrackMove.loa` 트랙 3개 추출 [실측] |
| 트리거 유닛 전체 목록 | 이름에 "기어오르기 트랙이동 활성화"(유닛 3101)와 "시작 부분 트랙이동 활성화"(유닛 2000)가 있음. 이동 자체를 하는 노드는 없고 프랍 속성만 바꿈 [실측] |
| `EFTable_Prop.db` | 375318(`jump`, "뛰기"), 375319(`climb`, "오르기"), 375317(`singleLine`, 줄타기; 발탄 맵에는 배치되지 않음) [실측] |
| 794개 EFTable 바이트 검색 (`creep_up`, `act_climb`) | **애니메이션 클립 이름을 적어 둔 표는 없음** [실측] |
| GameMsg enum 이름 | `tip.name.enum_propvolumetype_trackmove`, `enum_movestopreasontype_start_trackmove`, `enum_edataunittype_trackmove`가 있음 [실측] |
| 클래스 6개의 기본 AnimSet | `act_creep_*` 계열이 6개 클래스 모두에 있음 [실측] |
| **못 본 것** | 원본 실행 파일(EFEngine.dll)의 트랙 재생 코드는 열어 보지 않았다. 그래서 카메라·입력 잠금·재생 세부는 데이터로 확정하지 못했다 |

## 3. 원본에서 찾은 것

### 3.1 연결 관계

```
DeployData 프랍 0x10000026  (프로젝트 좌표 97.03, 15.35, -86.92)
  ├ 프랍 정의 375319: VolumeType 6, 모델 ITR_00280, 아이콘 climb, 문구 rise_1("오르기")
  ├ 레코드 +0x1BC = 3  ─────────►  TrackMove.loa 트랙 3 (시작점이 위 좌표와 같음)
  └ 트리거 유닛 3101 "기어오르기 트랙이동 활성화": Condition_Player(Signal) → 이 프랍의 속성을 2 → 0으로 바꿈
```

- 프랍 정의 375319·375318이 모두 `VolumeType=6`이고, 게임 전체에서 `VolumeType=6`인 행 656개가 전부 이동형 상호작용(뛰기·내려가기·올라가기·로프·줄타기·미끄럼) 종류다. 한편 GameMsg에 `PropVolumeType`의 `trackmove` 항목이 있다. 이름이 정렬되어 있어 숫자 6과의 대응은 직접 못 봤지만, **VolumeType 6이 TrackMove 볼륨이라고 보는 것이 자연스럽다** [추정].
- `+0x1BC`가 트랙 번호라는 것은 세 프랍 모두에서 맞았다: `0x10000024`→1, `0x10000026`→3, `0x10000137`→4 [실측].
- 세 존(37051·37052·37811)의 `TrackMove.loa`는 트랙 3개가 모두 같은 좌표·같은 길이이고 트랙 3의 시작 방향 값만 -304와 -308(0.02도)로 다르다 [실측].
- 프랍은 처음부터 켜져 있지 않을 수 있다. 유닛 3101이 "활성화"를 하고 프랍 레코드 `+0x64` 값이 0인 프랍이 트리거 대상 중 다수인 경향이 있지만, 그 필드의 뜻과 속성 값 2→0의 의미, 그리고 **신호(Signal)를 누가 보내는지는 못 찾았다** [미확인].

### 3.2 트랙 3의 수치 (프로젝트 좌표로 환산)

환산은 `build_maptool_scene.convert_position`과 같다: `project = (x·0.01, z·0.01, −y·0.01)`.

- 시작점 (97.027, 15.352, −86.921), 끝점 (98.530, 20.443, −86.900). 총 이동은 앞(+X) 1.503m, 위 5.091m, 옆 0.021m.
- 6.000초, 30Hz, 180키. 모든 키의 방향 값이 같아서 **방향은 시작부터 끝까지 고정**이다. 서버 yaw로는 약 88.3도(+X 쪽) [실측].

구간은 세 개로 나뉜다.

1. **0~0.43초 (키 0~12)**: 시작점 기준 앞 0.78m, 위 0.23m. 앞으로 가는 속도가 최고 초속 2.2m 정도까지 올라갔다가 0.43초에 사라진다.
2. **0.43~4.0초 (키 12~119, 3.57초)**: 앞으로는 거의 안 가고(+0.014m) 위로만 2.96m. 위 속도는 초속 0.83m로 일정하다. 이 구간의 클립은 제자리 루프여야 한다.
3. **4.0~6.0초 (키 119~179, 2.0초)**: 위로 1.91m, 앞으로 0.70m. 4.0~4.5초에 초속 1.14m, 이후 속도가 죽었다가(5.03초 근처에서 3.6cm 내려가기까지 함) 5.2~5.6초에 초속 1.6~1.8m로 빠르게 오르고 앞으로도 붙는다. 턱을 짚고 올라서는 동작의 곡선처럼 보인다 [추정].

등반 구간 동안(0.43~5.2초) 플레이어의 앞 위치(x)는 97.81~97.83으로 거의 안 움직인다. 현재 우리 `Stage_3` 트리거 박스의 중심 x(97.76)와 가깝다.

전체 30Hz 키는 `out/ValtanWallClimb20260921/wall_climb_original.json`의 `track.keys`에 있다.

### 3.3 애니메이션 클립 (클래스별)

클래스 6개의 기본 AnimSet(PC_FT_00 등)을 umodel로 뽑아 PSA를 직접 읽었다. 아래 클립은 6개 클래스에 모두 있다 [실측]. `b_root` 루트 이동량은 프로젝트 좌표가 아니라 클립 로컬 값(cm)이다.

| 클립 | 프레임/길이 | 루트 이동 | 쓰임새 후보 |
|---|---|---|---|
| `act_creep_up_1` | 41 / 1.367초 | 없음 (제자리) | 등반 루프 |
| `act_creep_up_end_1` | 61 / 2.033초 | 앞 68~89, 위 168~191 (클래스마다 다름) | 턱 넘기 |
| `act_creep_down_1`, `_loop_1`, `_end_1` | 41 / 31 / 61 | 없음 | 내려가기(godown) 후보 |
| `act_zipline_loop_1`, `_end_1` | 51~121 / 31 | 끝 클립은 앞 3.8~4.0m | 줄 타기 후보 |

`act_creep_up_end_1`의 위 방향 루트 이동: 창술사 173, 건슬링어 168, 슬레이어 172, 도화가 169, 차원술사 175, 워로드 191cm. 트랙 3 마지막 2.0초(키 119~179)의 위 이동량은 191cm라서 워로드 값(191.3cm)과 가장 가깝다.

트랙과 클립의 곡선도 맞춰 봤다. 트랙에서 61키 창을 밀어가며 각 클래스의 `act_creep_up_end_1` 위 방향 곡선과 오차(RMS)가 가장 작은 위치를 찾았더니, 워로드·슬레이어·차원술사는 4.0초 창(118번째 키 근처)에서 최적이었고 오차는 7~10cm였다. **창술사·건슬링어·도화가는 3.2초 근처가 최적으로 나와 이 방법으로는 같은 결론이 나오지 않았다**(원인은 확인하지 않았다). 따라서 "트랙 마지막 2.0초 = 끝 클립"은 속도 변화 지점(4.0초)과 프레임 수 일치가 주된 근거이고 곡선 일치는 보조 근거일 뿐이다 [실측 + 추정].

**무엇이 이 클립들을 고르는지는 데이터로 못 찾았다.** 클립 이름을 적어 둔 표가 없고 실행 코드를 못 봤기 때문에, 등반 루프와 끝 클립의 조합은 이름·길이·루트 이동 일치에 기반한 추정이다 **[추정]**. 처음 0.43초 구간에 쓰는 클립도 특정하지 못했다 **[미확인]**.

### 3.4 메커니즘 판정

- (A) 루트 모션이 캐릭터를 올린다 — **아니라고 본다.** 끝 클립의 루트 상승량이 클래스마다 168~191cm로 다른데 트랙은 하나여서, 루트 모션이 위치의 주인이었다면 클래스마다 도착 높이가 달라야 한다 [추정].
- (B) 스크립트가 위치를 시간에 따라 옮기고 애니메이션은 표현만 한다 — **가장 가깝다.** 위치 시간표가 데이터로 있고 6초 전체를 덮는다 [실측 + 추정].
- (C) 순간이동·컷신 — 아니다. 트리거 유닛에 텔레포트·SceneEvent가 없고 트랙이 연속 경로다 [실측].
- **결론은 (B), 다만 트랙이 끝 클립의 루트 곡선을 본떠서 만들어져 마지막 구간에서는 겉으로 (A)처럼 보인다.**

### 3.5 시작 위치·방향·입력·카메라·파티

- **시작 위치**: 트랙은 상대 이동이 아니라 **절대 좌표**이고 시작점이 프랍 좌표와 같다 [실측]. 재생이 시작될 때 플레이어를 그 좌표로 옮기는지, 지금 위치 기준으로 상대 재생하는지는 데이터로 못 가른다 [미확인]. 프랍의 상호작용 볼륨은 작다(레코드 `+0x128`의 64·64·128 값이 세 프랍 모두 같음; 절반 크기 cm로 읽은 것은 추정) [추정].
- **방향**: 트랙의 방향 값은 고정 [실측].
- **입력 잠금**: `enum_movestopreasontype_start_trackmove`("이동 중단 사유: 트랙이동 시작")가 있어서 트랙이 시작될 때 이동이 멈추는 구조로 보인다 [추정]. 우리 서버도 `TRIGGER_MOVE` 동안 이동 명령을 거절한다(`GameRoom_PlayerCommands.cpp`의 이동 처리가 `eAction`이 `NONE`이 아니면 취소 가능한 스킬 행동이 아닌 한 돌려보냄) [실측].
- **카메라**: 원본이 카메라를 고정하는지 추적하는지 **[미확인]**.
- **파티**: 프랍·트랙에 인원 제한이나 순서 필드는 보이지 않는다. 우리 서버는 플레이어별로 독립 이동이라 4명이 동시에 같은 경로를 타면 서로 겹쳐서 보일 수 있다 [추정].

## 4. 우리 쪽 현재 상태와 원본 비교

우리 쪽 근거는 `origin/main`(`a3c0b5b6`)의 파일을 `git show`로 읽은 것이다.

| 항목 | 원본 | 우리 `Stage_3` |
|---|---|---|
| 시작점 | 프랍 좌표(97.03, 15.35, −86.92)로 보임 | G를 누른 자리. 트리거 박스가 2×2×7.6m라 벽을 따라 7.6m 어디서든 가능 |
| 끝점 | (98.53, 20.44, −86.90) | (100.42, 20.53, −86.95). 원본 끝보다 앞으로 1.89m |
| 상승 | 5.09m | 5.02m (트리거 중심 기준) |
| 시간 | 6.0초 | 0.8초 (7.4배 빠름) |
| 경로 | 3구간(붙기 → 수직 → 턱 넘기), 앞 방향은 벽 쪽에서 고정 | 시작→끝 직선 한 구간 |
| 방향 | 시작부터 끝까지 +X 쪽 고정 | 시작 순간에 시작→끝 방향으로 한 번 정하고 고정 |
| 애니메이션 | 등반 루프 + 끝 클립(추정) | 클래스 지형점프 클립(`terrain_jump_short`)을 서버 행동 시계로 재생하고 마지막 프레임 유지. 지형점프 AnimSet이 없는 클래스(건슬링어·슬레이어)는 달리기 루프 그대로 |

우리 코드 근거는 다음과 같다 [실측].

- `Server/Private/ServerTriggerSystem.cpp`: `Begin_MovePlayer`가 시작점을 그 순간의 플레이어 위치로 잡고 방향을 한 번 정한다. `Update_PlayerMotion`이 30Hz로 `start → target` 직선 보간을 하고 Y에만 `4·arcHeight·t·(1−t)`를 더한다. 이벤트 필드는 `targetPosition/durationSeconds/arcHeight`뿐이다.
- `Client/Private/Character.cpp`: `TRIGGER_MOVE`이면 이동 종류와 상관없이 `terrainJump` 클립 하나를 행동 나이로 seek하고 마지막 프레임을 유지한다. 벽타기인지 알려 주는 정보가 스냅샷에 없다.
- `Client/Private/InteractKeyPromptView.cpp`: 상승이 1m를 넘으면 올라가기 아이콘을 쓴다. 명시하려면 배치의 `interactAction:"climb"`를 쓸 수 있고 이 값은 **Client 표시 전용**이다(`Publish-WorldGameplay.ps1`이 Server bootstrap에 싣지 않는다).
- `Engine/Private/Model.cpp` `Apply_RootMotionTranslation`: 몸 모델은 루트 본의 수평 이동은 항상 버리고 **수직(Z) 이동은 그대로 반영**한다(`Set_RootMotionVerticalScale`로 0~1 조절). 등반 끝 클립의 위 방향 루트 이동(1.7~1.9m)을 그대로 두면 서버가 올린 높이에 **한 번 더 얹혀서 메쉬가 1.7~1.9m 떠 보인다**. 등반 재생 동안 이 값을 0으로 두어야 한다.

부수 발견: 원본 `player.spawn.editor`(28.11, 10.24, −28.87)와 `player_Move.1`(30.68, 10.04, −28.02)도 원본의 뛰기 프랍 `0x10000024`·`0x10000137`(트랙 1·4, 2.73초, 옆 6.0m)에 대응한다. 우리는 0.8초로 처리하고 있다. 이번 조사 범위는 아니어서 자세히 보지 않았다.

## 5. 구현안

### 대안 요약

| 안 | 내용 | 원작 충실도 | 작업량·위험 |
|---|---|---|---|
| A | 데이터만: `durationSeconds`를 6.0으로, 트리거 박스를 프랍 위치로 좁히고 목표를 원본 끝점으로 | 낮음 (직선이라 벽에서 멀어졌다 붙음, 점프 클립이 6초간 굳음) | 아주 작음. 타이밍 감을 보는 용도로만 |
| B | 서버에 "경유점 여러 개" 이동을 추가 (이벤트에 점 3~4개) | 중간 (수직 구간은 정확, 턱 넘기 곡선은 근사) | 중간. 이벤트 스키마·publisher·parser·MapTool을 건드리고 곡선은 손으로 맞춰야 함 |
| **C (권장)** | **원본 트랙을 그대로 데이터화한 TrackMove 시간표 + 스타일 태그 + 등반 클립** | 높음 (위치는 원본과 같음) | 큼. 하지만 기존 선례(탈것 스킬 루트 이동 표)를 재사용하고 계층별로 나눠 진행 가능 |

### 안 A가 좋지 않은 이유(수치)

- 직선으로 (97.03→98.53)를 6초에 가면 앞으로 초속 0.25m씩 계속 밀리는데, 원본은 구간 2 동안 x가 97.81~97.83에서 안 움직인다. 원본 x와 직선 x의 차이가 최대 0.68m(0.4초 지점)라서 벽에서 그만큼 떨어져 보인다 [실측: 계산]. 벽 면의 실제 위치는 확인하지 않았다.
- 지형점프 클립(1.03초)이 재생된 뒤 5초 가까이 착지 자세로 굳는다 [실측: 코드가 그렇게 동작].

### 권장안 C — 계층별 변경 목록

**데이터**
- 새 저작 문서 `Data/Worlds/LV_LUT_HEARTRB_ED/TrackMoves.world.json`(안): `trackMoveId`, `style:"climb"`, 시작 좌표·yaw, 총 길이(ms), 표본 목록(`시간ms, 앞, 옆, 위`). `SpawnGroups.world.json`처럼 Gameplay 문서와 분리하고 stable ID로 참조한다. 원본 표본은 이 조사의 `track.keys`로 만든다(30Hz 180개 또는 3프레임마다 60개).
- `Gameplay.world.json`의 `Stage_3` 이벤트에 `trackMoveId`(안)를 추가하고, 같은 배치에 `interactAction:"climb"`를 명시한다(이미 허용된 값). 트리거 박스는 프랍 좌표(97.03, 15.35, −86.92) 주변으로 좁힌다.
- 플레이어 애니메이션 바인딩 `Data/Animation/Authored/<Class>/<Class>.interactionbindings.json`에 `wallClimb`(루프 클립, 끝 클립, playRate)을 `terrainJump`와 같은 방식으로 추가한다.

**Tools**
- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`: `trackMoveId` 참조 검사와 표본 검증(시간 오름차순, 유한값, 총 길이), 이벤트의 새 필드 허용, `TRACKMOVE` 행 publish. 형식은 기존 `SKILLROOTMOTION`의 `시간:앞:옆:위` 문자열을 재사용한다(`Server/Private/GameplayCatalog.cpp`의 `Parse_RootMotionSamples`와 `ROOT_MOTION_SAMPLE{iTimeMs,fForward,fLateral,fUp}` 선례).
- `Tools/ActorXAssetCooker/build_wall_climb_player_animations.py`: `build_terrain_jump_player_animations.py`를 복제해 클립을 `wall_climb_loop←act_creep_up_1`, `wall_climb_end←act_creep_up_end_1`로, 접미사를 `_WallClimbAnimSet.wmodel`로 바꾼다. 6개 클래스의 원본 PSA는 이미 뽑아 두었다(아래 6장).
- 원본 표본 변환 스크립트(이 조사의 `trackmove.py`가 씨앗).

**Server**
- `WorldBootstrap.h/.cpp`: `TRACKMOVE` 행 파싱과 이벤트에서의 참조 해석.
- `Server/Public/ServerPlayer.h`의 `SERVER_TRIGGER_MOVE`에 트랙 참조와 스타일을 추가.
- `ServerTriggerSystem.cpp`: `Begin_MovePlayer`는 트랙이 있으면 시작점을 트랙 시작 좌표로 잡고(0.2초 정도의 짧은 보간을 둘지는 화면 판정 필요), 방향은 트랙 yaw로 고정한다. `Update_PlayerMotion`은 행동 경과 시간으로 표본을 선형 보간해 위치를 정하고 끝에서 마지막 표본으로 스냅한다. 서버는 위치와 시간만 정하고 Client 클립·본은 모른다는 기존 원칙이 그대로 유지된다.
- 계약 테스트(`ServerGameplayContractTests_WorldTriggers.cpp`)에 표본 보간과 끝 스냅, 도중 사망·행동 충돌 처리 케이스를 추가한다.

**Shared (선택)**
- 스냅샷이 "이 이동은 벽타기다"를 실어야 Client가 클립을 정한다. 방법은 두 가지다. (1) `PLAYER_ACTION_STATE`에 값을 끝에 추가하거나 `TRIGGER_MOVE`의 `iSkillId`를 스타일 번호로 쓰는 명시 방식(권한 원칙에 맞고 Client가 추정하지 않음, **프로토콜 번호를 올려야 함**), (2) 프로토콜 변경 없이 Client가 자기가 가진 `Gameplay.world.json`의 `interactAction:"climb"` 배치 중 시작 위치를 포함하는 것을 찾는 추정 방식(작업은 가볍지만 박스가 겹치면 오작동). 베른 왕복(높이차 17m)에서 "높이 > 1m면 벽타기"로 추정하면 오작동하므로 **명시 표시가 필요**하다. 권장은 (1)이다.
- **프로토콜 번호 주의**: 이 조사 시점의 `origin/main`은 이미 `NETWORK_PROTOCOL_VERSION = 99`다(무적 구역 표시 펄스, 커밋 `f291f886`). 다른 작업이 98에서 99로 올리는 브랜치가 있으면 병합 때 충돌한다.

**Client**
- `WorldGameplayDocument.cpp/.h`: 이벤트 키 검사가 엄격해서 새 키(`trackMoveId`)를 받아 주도록 하고, MapTool(`MapTool_WorldGameplay*.cpp`)이 저장할 때 이 키를 지우지 않도록 한다.
- `Character.cpp/.h`: `wallClimb` 바인딩 읽기, `TRIGGER_MOVE`가 벽타기 스타일이면 행동 나이로 클립을 고른다. 나이가 (길이 − 끝 클립 길이) 이전이면 루프 클립을 반복 재생하고, 이후면 끝 클립을 그 시점부터 seek한다. 재생 동안 `Set_RootMotionVerticalScale(0)`을 걸고 끝나면 이전 값으로 되돌린다. 구현 선례는 `KoukuSaydonPresentationPlayer.cpp`의 같은 API 사용이다.
- `Data/Actors/CharacterCatalog.json`의 클래스별 AnimSet 목록에 `_WallClimbAnimSet.wmodel` 추가.

**Resources (팀장 Drive 물리 입력, Git 추적 안 함)**
- `Character/<Class>/AnimSets/<Class>_WallClimbAnimSet.wmodel` 6개. 현재 로컬에는 지형점프 AnimSet도 4개 클래스(창술사·도화가·차원술사·워로드)만 있고 건슬링어·슬레이어에는 없어서, 같은 작업 때 6개를 모두 구우면 된다.

**단계 나누기**
1. 데이터 저작 + 서버 시간표 재생 + 계약 테스트. 이 단계에서 클립 없이도 위치는 원본 곡선이 되고, 사용자가 서버·Client를 켜고 궤적을 확인할 수 있다.
2. 스타일 태그 + Client 클립 스텝 + 6개 클래스 클립 굽기.
3. 트리거 박스 좁히기, 시작 스냅 보간, 트랙 1·4(시작 부분 뛰기)와 베른 왕복에 같은 구조를 적용할지 결정.

## 6. 확인하지 못한 것과 사용자가 화면으로 판정할 것

**원본에서 확정하지 못한 것 [미확인]**
- 카메라 처리(고정/추적).
- 트랙 재생 시작 때 플레이어를 시작 좌표로 옮기는지, 지금 위치 기준 상대 재생인지.
- 처음 0.43초(벽 붙기) 구간에 쓰는 클립.
- 등반 루프의 반복 횟수와 끝 클립으로 넘어가는 이음매(수치상 3.57초를 루프 길이 1.333초로 나누면 약 2.7회).
- 프랍의 활성 신호(유닛 3101의 Signal)를 보내는 쪽과 속성 값 2→0의 뜻. 우리는 지금 항상 활성이다.
- 트랙 끝점(98.53, 20.44, −86.90)이 우리 발탄 navgrid에서 걸을 수 있는 셀인지. 현재 착지 목표 100.42는 그보다 1.89m 앞이다. F1 `Show Navigation` 또는 서버 nav 질의로 확인이 필요하다.
- 카메라와 네트워크: 4명이 동시에 같은 경로를 탈 때의 표현.

**사용자가 직접 화면으로 봐야 하는 것**
- 등반 클립이 위치 곡선과 발이 미끄러지지 않고 맞는지, 특히 등반 루프의 이음매와 턱 넘기 구간(끝 클립의 수직 루트를 0으로 눌렀을 때 몸이 어색하지 않은지).
- 6초 길이가 게임 감각에 맞는지, 시작 스냅이 튀지 않는지.
- 클래스마다 몸 크기 차이 때문에 손발이 벽에서 뜨거나 파고드는지.

**추가로 필요한 원본 추출**
- 없음. 6개 클래스의 원본 PSA를 이미 뽑아 두었다. 필요한 것은 굽기 작업이다.

## 7. 재현 방법과 산출물

스크립트(전부 읽기 전용, 결과는 `C:\LostArkExtract\ValtanClimb_20260921\` 아래로만 쓴다).

- `scripts\dbq.py`: 복호화된 EFTable DB 조회(`cols`, `grep`, `rows`).
- `scripts\pull_mapdata.py <zone…>`: `leveldata1/2.lpk`에서 `MapData\<zone>`을 `mapdata\`로 추출.
- `scripts\trig_units.py <zone> [unitId…]`: `TriggerMapData.loa` 유닛·노드 덤프.
- `scripts\deploy_props.py [near x y z r | id …]`: `DeployData.loa` 프랍과 Prop DB 조인.
- `scripts\trackmove.py <zone> [--json out]`: `TrackMove.loa` 파서. 파일 전체(9,756바이트)를 정확히 소비함을 확인.
- `scripts\psa_tool.py`, `scripts\class_clip_table.py`: PSA 클립 목록과 루트 이동 측정, 클래스별 표.
- `scripts\prop_enable_probe.py`: 프랍 활성 필드 교차 조사(결론은 미확정).
- `scripts\build_result_json.py`: 위 결과를 모아 `out\wall_climb_original.json`을 만든다.

산출물.

- `out/ValtanWallClimb20260921/wall_climb_original.json`(저장소 `out\`은 git 제외): 트리거·프랍·트랙 30Hz 키·클래스별 클립 표·비교.
- `C:\LostArkExtract\ValtanClimb_20260921\actorx\PC_FT_00, PC_GN_F_00, PC_WR_F_00, PC_SP_00, PC_SP_M_00\AnimSet\*.psa`: 굽기용 원본 PSA. 파일당 약 0.9~1.0GB로 합계 약 4.8GB이다. 워로드는 기존 `Kouku_CardCutscene_20260904\actorx\PC_WR_00`을 그대로 쓴다. 굽기가 끝나면 지워도 된다.
- `C:\LostArkExtract\ValtanClimb_20260921\out\`: `trackmove_37051.json`, `deploy_props_37051.json`, `class_clip_table.json`.

## 8. 이번 조사의 한계

- 원본 클라이언트 실행 코드를 읽지 않았다. 트랙이 어떤 클립과 함께 재생되는지, 카메라와 입력 처리는 이름·길이·값 일치로 추정한 것이다.
- 우리 쪽 코드 근거는 `origin/main`의 파일이고, 작업 폴더에서 다른 작업이 편집 중인 파일은 읽지 않았다.
- 등반 클립을 실제로 굽거나 서버·Client를 실행해 확인하지 않았다. 화면 판정은 사용자가 직접 한다.

## 9. 2차 조사 (2026-09-21 오후) — 1차에서 못 찾았거나 확인하지 않은 것

범위는 1차와 같은 읽기 전용이다. 저장소에서 새로 만들거나 바꾼 것은 이 절과 `out/ValtanWallClimb20260921/`의 JSON 두 개(`wall_climb_original.json` 갱신, `jump_tracks.json` 신규)뿐이고, 새 스크립트는 `C:\LostArkExtract\ValtanClimb_20260921\scripts\`에 있다.

원본 클라이언트 실행 코드는 이번에도 분석하지 않았다. `LOSTARK.exe`(57MB)에는 `TrackMove` 문자열이 없고, `EFEngine.dll`은 UE3 엔진 심볼(`UInterpTrackMove` 같은 매티니 클래스)만 내보내며 게임 고유 트랙 코드는 없었다 [실측: 두 파일의 ASCII/UTF-16 문자열 검색]. 대신 **게임 스크립트 패키지(`.u`)의 구조체·열거형 정의**와 발탄 레벨 패키지를 새로 읽었다. 열거형의 번호는 이름 표의 직렬화 순서에서 되살렸고, 발탄 데이터로 교차 검증했다(아래 9.1의 `PropVolumeType` 6).

### 9.0 결론 먼저

- **트랙 형식이 확정됐다.** 게임 스크립트의 `EFTrackMoveInfo`(`TrackIndex`, `InitLocation`, `InitRotation`, `bCanBeHit`, `MovementInfo[]`)와 `EFTrackMoveTransform`(`Time`, `Location`, `Rotation`) 필드 순서가 `TrackMove.loa`의 트랙 헤더와 키 형식과 정확히 일치한다. 그래서 1차에서 `w/rot/u`로 불렀던 값은 Rotator의 pitch/yaw/roll이고, `extra`는 `bCanBeHit`이다. **세 트랙(벽타기 1개, 뛰기 2개) 모두 `bCanBeHit=false`** [실측].
- **`VolumeType 6`은 `PROP_VOLUME_TYPE_TRACKMOVE`가 맞다** [실측]. 1차의 "자연스럽다"는 추정을 확정으로 바꾼다.
- **원본 상호작용 볼륨은 우리 `Stage_3` 박스보다 훨씬 작다.** 배치 레코드의 볼륨 크기가 (64, 64, 128)cm이고, 우리 박스(반크기 [1, 1, 3.8]m)는 벽 방향으로 최소 약 6배 길다. 반크기인지 전체 크기인지는 확정하지 못했다.
- **카메라는 벽타기 전용 처리를 찾지 못했다.** 프랍·트랙·GameAction 어디에도 카메라 필드가 없고, 발탄 레벨의 카메라 볼륨은 보스 구역 하나뿐이며 벽타기 지점은 그 밖이다.
- **트랙 끝점(98.530, 20.443, −86.900)은 우리 navgrid에서 걸을 수 있는 셀이고 셀 높이(20.4405)가 트랙 끝 높이와 2.5mm 차이로 일치한다** [실측].
- **시작 규칙은 "시작 자세로 놓고 시작"으로 보이지만 실행 코드로 확인하지 못했다** [추정].
- **활성화 신호의 발신자는 끝내 못 찾았다** [미확인].

### 9.1 트리거 볼륨(활성 영역)

| 항목 | 값 | 근거 |
|---|---|---|
| 볼륨 종류 | `PROP_VOLUME_TYPE_TRACKMOVE`(열거형 6) | `.u`의 `PropVolumeType` 열거형(33개)에서 6번. 배치 레코드 `+0xA4`가 Prop 표 `VolumeType`과 169개 전부 일치하고, 값 6은 세 이동 프랍에만 있다 [실측] |
| 세 이동 프랍만의 꼬리 | 레코드가 492바이트(다른 프랍은 480바이트). `+0x1B4=1`, `+0x1B8=1`, `+0x1BC=트랙 번호` | `EFDeployTrackMoveInfo`(`Index`, `TrackMoveIndex`, `NextTrackMoveVolumeSpawnIndex` 정수 3개)와 같은 크기. 트랙 번호가 `+0x1BC`임은 값 1·3·4로 확정. 나머지 둘은 세 프랍 모두 1이라 어느 필드인지 구분하지 못했다 [실측 + 미확인] |
| 볼륨 크기 | 레코드 `+0x128/+0x12C/+0x130` = (64, 64, 128)cm, 세 이동 프랍 동일 | Prop 표(375317/8/9)의 `VolumeExtentX/Y/Z`는 모두 0이라 **크기는 배치 레코드가 가진다**. 발탄의 볼륨 프랍 30개 전부 레코드에 크기가 있고 표의 값은 0 [실측] |
| 모양 | `VolumeShapeType=1` | 발탄 프랍 169개 전부 1이라 모양을 구분할 수 없다 [실측, 의미 미확인] |
| 상호작용 거리 | `InteractionRange=0`, `OnlyKeyInteraction=0`, `HideInteractionMark=0`, `PushOffPC=1` | Prop 표 값. 거리·바라보기 조건을 적은 열은 없다 [실측]. 값의 뜻과 실제 판정은 실행 코드를 못 봐서 [미확인] |
| 위치·방향 | 프랍 (97.027, 15.350, −86.921)m, 프랍 yaw 1.05° | 1차와 같음 |

단위(반크기/전체 크기)는 UE3 관례(BoxExtent=반크기)를 따르면 반크기이지만 이 게임의 배치 레코드가 그 관례를 따르는지 확인할 방법을 찾지 못했다 [추정]. 볼륨 축은 UE X = 프로젝트 X, UE Z = 높이, UE Y = 프로젝트 −Z다. 두 읽기의 박스는 다음과 같다(프랍 중심 기준, 프로젝트 m).

| 읽기 | 반크기 (X / 높이 / Z) | 범위 |
|---|---|---|
| 반크기 | 0.64 / 1.28 / 0.64 | X 96.387~97.667, 높이 14.070~16.630, Z −87.561~−86.281 |
| 전체 크기 | 0.32 / 0.64 / 0.32 | X 96.707~97.347, 높이 14.710~15.990, Z −87.241~−86.601 |

우리 `Stage_3`(중심 (97.761, 15.511, −87.987), 반크기 [1, 1, 3.8], 범위 X 96.761~98.761, 높이 14.511~16.511, Z −91.787~−84.187)와 비교하면 다음과 같다. 중심은 프랍에서 X +0.73m, 높이 +0.16m, Z −1.07m(거리 1.31m) 떨어져 있다. 반크기 비율(우리/원본)은 반크기 읽기로 X 1.56배, 높이 0.78배, **Z 5.94배**, 전체 크기 읽기로 X 3.12배, 높이 1.56배, **Z 11.88배**다. 원본은 프랍 앞 약 1m 안에서만 누를 수 있고 우리는 벽을 따라 7.6m 어디서든 누른다.

### 9.2 시작 위치 처리

근거는 다음과 같다.

- `EFTrackMoveInfo`에 **`InitLocation`과 `InitRotation` 필드가 있다** [실측]. 벽타기 트랙 3의 `InitLocation`은 (9702.664, 8692.060, 1535.203)cm이고 프랍은 (9702.664, 8692.060, 1535.0)cm로 높이만 0.2cm 다르다. 트랙 1은 1.2cm, 트랙 4는 5.2cm 다르다(1차와 같음).
- 키는 상대 이동이 아니라 **절대 좌표**이고, 첫 키(1/30초)는 `InitLocation`에서 앞으로 2.45cm, 높이 0.15cm만 벗어난다. 즉 트랙은 시작점부터 끊김 없이 이어진다 [실측].
- 그래서 "재생 시작 때 플레이어를 `InitLocation`/`InitRotation` 자세로 놓고 키를 재생한다"는 구조가 가장 자연스럽다 [추정]. 트랙에 시작 자세를 굳이 저장한 이유가 이것이다. 순간 스냅인지 짧은 보간인지는 실행 코드를 못 봐서 가르지 못했다 [미확인]. 볼륨이 작아서(9.1) 스냅 거리는 최대 1m 안팎이다 [추정, 단위 미확정].
- `EFPlayer`에 `TrackMoveStartPosition`(**float**)이 있다. 이름과 자료형만 확인했고 뜻은 모른다 [미확인]. 시작 "위치"가 아니라 하나의 float이므로 트랙 위의 시간/거리 오프셋일 가능성이 있지만 데이터로 뒷받침하지 못했다.
- `EFPlayer`에는 `ControlledByTrackMove`와 `ControlledByTrackMoveOnServer`(bool)도 있다. **서버 쪽에도 같은 이름의 플래그가 있다는 것**은 원본도 서버가 트랙 이동을 안다는 뜻으로 우리 구조(서버가 위치를 정함)와 방향이 같다 [실측 이름, 뜻은 추정]. 이 세 프로퍼티를 읽는 스크립트 함수는 1,026개 중 0개라 실제 처리는 네이티브 코드다.
- 트랙 시작과 관련된 열거형 이름이 있다 [실측 이름, 뜻은 추정]: `MOVE_STOP_REASON_TYPE_START_TRACKMOVE`(이동 중단 사유), `COMMON_ACTION_CANCEL_FORCED_REASON_START_TRACK_MOVE`와 `..._EXE_TRACK_MOVE`(공용 행동 강제 취소), `VEHICLE_GET_OFF_ORIGIN_TRACK_MOVE`(탈것 하차 사유), `NPC_ATTACH_REASON_TRACK_MOVE`, `SKILL_EFFECT_FILTER_CONDITION_TYPE_TRACK_MOVE`. 시작 순간에 이동 중단, 행동 취소, 탈것 하차가 일어나는 것으로 보인다.
- **`bCanBeHit=false`**(세 트랙 모두, 값 0) [실측]. 필드 이름대로면 트랙 이동 중 피격되지 않는다 [추정].

### 9.3 카메라

- 레코드 조사 [실측]: Prop 표 375319에 카메라 열이 없다(`FallDownCameraType=0`, `InteractionMatineeShow=0`). `EFTrackMoveInfo`에도 카메라 필드가 없다. `GameAction` 표의 `START_TRACK_MOVE`(열거형 31)는 행이 하나(PK 10021)이고 모든 매개변수가 0이다.
- 표 조사 [실측]: `EFTable_Camera.db`는 테이블이 하나도 없는 빈 파일이다. `CameraSetting`(244행)과 `CameraContentsSetting`(14행)은 클래스·콘텐츠별 카메라 설정이다. 앞쪽 40개 PK 중 클래스별 기본 카메라(PK 101~604, 28종)는 전부 FOV 50, pitch −45, yaw 45, 거리 1600cm(16m)이고 1001번대 같은 다른 콘텐츠용 행은 값이 다르다(나머지 행은 읽지 않았다). 트랙 이동이나 발탄 존에 연결되는 열은 찾지 못했다.
- **카메라 볼륨 [실측]**: 발탄 레벨 `LV_LUT_HeartRB_ED_PS`(`312N3UN2RBQY9N952CQJ2CVG.upk`)에 `EFChangePlayerCameraVolume`이 **하나**(export 27, `efchangeplayercameravolume_4`) 있다. 값은 `ViewDistance` 1800cm, `ViewFOV` 55°, 위치 UE (14417, 10887, 2293)cm = 프로젝트 (144.17, 22.93, −108.87)m다. 브러시 모델의 경계 상자는 프로젝트 X 105.36~199.30, Z −164.00~−70.39, 높이 −9.19~55.05m다. 경계값은 UE3 관례(`FBoxSphereBounds`: 중심, 반크기, 반지름)로 읽었고, 모델의 12~14개 정점 후보 배열의 좌표 극값(예: 3212.2, 5512.8, −3847.8, −3880.8)이 이 경계값과 일치해서 형식 해석은 뒷받침된다. 정확한 볼록 다면체는 복원하지 않았고 경계 상자가 그 바깥 한계다 [실측 + 형식 해석은 추정]. `SL00`~`SL05`에는 카메라 클래스가 없다. `_BASE` 논리 이름은 UModel `-nameresolve`가 패키지를 찾지 못했다(이 이름은 게시 도구의 파생 이름일 수 있다) [미확인].
- **벽타기 구간(X 97.03~98.53)은 이 볼륨의 서쪽 밖 7.5m 이상이다** [실측: 경계 상자 기준]. 보스 구역(`Stage_Boss`, `Stage_Boss_ArenaEntry`)은 안이다. 이 볼륨의 카메라 모드 프로퍼티는 저장돼 있지 않아 기본값(Default) 모드로 보인다 [추정]. 열거형은 `Default`/`Fixed`/`LockOn` 세 가지다.
- **우리 `Data/Camera/Valtan.camera.json`(55°, 18m)은 이 볼륨의 값과 같다.** 원본 규칙(볼륨 안만 55°/18m, 그 밖은 기본 50°/16m)을 그대로 옮기면 벽타기 구간의 원본 카메라는 기본값 50°/16m다 [추정]. 우리는 맵 전체에 55°/18m를 쓰고 있다. 이것이 의도한 결정인지 사용자 판단이 필요하다.
- 등반 중 카메라 특수 처리는 스크립트 수준에서 찾지 못했다: `ControlledByTrackMove` 등을 읽는 스크립트 함수가 0개이고 `EFCameraBase` 계열 클래스에 트랙 이동 참조가 없다. 네이티브 카메라 코드는 열지 못했다 [미확인]. 그러므로 원본도 일반 추적 카메라가 올라가는 캐릭터를 따라갔을 가능성이 높지만 추정이다.

### 9.4 처음 0.43초와 시작·끝 동작

- 플레이어 동작 이름을 적어 둔 표는 여전히 없다(1차의 794개 표 검색 결과와 같음). 프랍·트랙·`GameAction`에도 클립 이름 열이 없다 [실측].
- 클래스별 클립 표를 다시 확인했다. 정규식(`creep|climb|ladder|rope|lope|wall|hang|zipline|rise|vault|clamber|ledge|cliff|track`) 결과가 여섯 클래스 공통으로 9개다: `act_creep_down_1`, `act_creep_down_loop_1`, `act_creep_down_end_1`, `act_creep_up_1`, `act_creep_up_end_1`, `act_jump_lope_1`, `act_zipline_loop_1`, `act_zipline_end_1`, `sc_surprise_1`. 추가로 창술사만 `act_creep_loop_1`, `act_creep_down_2`, 건슬링어만 `evt1_sc_surprise_1`이 있다. **`act_creep_up_start*` 같은 시작·접근 클립은 어느 클래스에도 없다** [실측: 이름 검색].
- 워로드는 기본 AnimSet 외에 `pc_wr_00_ani_bk`, `pc_wr_00_vehicle_ani` 두 세트도 열어 봤고 등반 시작 클립은 없었다 [실측]. 다른 다섯 클래스는 기본 AnimSet만 추출했으므로 그 클래스의 다른 AnimSet은 확인하지 못했다.
- 그래서 **처음 0.43초(벽 붙기)에 쓰는 클립은 이번에도 특정하지 못했다** [미확인]. 유력 후보는 등반 루프 `act_creep_up_1`이 바로 시작되는 것이다 [추정].

### 9.5 활성화 신호와 다인 이용

- 유닛 3101의 노드 값을 `ChangePropPropertyType` 열거형(0 ENABLE, 1 DISABLE, 2 VISIBLE, 3 INVISIBLE, 4 BLOCKABLE, 5 UNBLOCKABLE, 6 SPAWN_ANIM)으로 읽으면 **프랍 `0x10000026`을 보이게(2) 한 뒤 활성화(0)** 한다 [추정: 노드의 값 필드가 이 열거형의 번호라는 정황 해석. 유닛 2000의 1→3(DISABLE→INVISIBLE)도 같은 방식으로 자연스럽다].
- **`Condition_Player`의 Signal 발신자는 못 찾았다** [미확인]. 확인한 것은 다음과 같다. 유닛 3101은 존 안의 어떤 `SetTriggerUnitEnableState`(유닛 10개에 노드 11개, 대상 6015·6001·5004 등)도 대상으로 삼지 않고, 다른 어떤 노드 꼬리에도 3101이 없다. 같은 `Condition_Player` 노드를 쓰는 나머지 두 유닛(10000 "죽음에서 돌아온 발탄", 10001 "찢겨진 발탄의 최후")은 **`SceneReplay` 표의 컷신 다시보기 트리거**(PK 3705100/3705101의 `TriggerUnitIndex`)라서, 이 노드의 Signal은 "이 유닛 번호로 직접 보내는 플레이어 신호"로 보인다. 3101은 `SceneReplay`에 없다. 그래서 발신자는 존 밖(엔진/서버)일 것이고 항상 켜져 있다고 보는 우리 현재 처리와 어긋나지 않는다 [추정].
- **시작 부분의 뛰기 프랍(`0x10000024`, `0x10000137`)은 조건이 다르다.** 유닛 2000은 `Condition_AssembleVolumeProp`(집합 볼륨 `0x1000010B`, `PROP_VOLUME_TYPE_ASSEMBLE`, 위치 (28.71, 10.52, −21.26)m)이 충족되면 두 프랍을 보이게+활성화하고, `0x10000068`(STATION 볼륨)과 `0x10000069`를 비활성화+숨긴다. 시작 지역에서 파티가 집합 볼륨에 모이면 뛰기 프랍이 켜지는 구조다 [추정: 값 해석].
- 이동 프랍을 조건으로 쓰는 노드는 없다: 발탄 존의 `Condition_VolumeProp` 14개 중 세 프랍을 참조하는 것이 없어 `TCS_TrackMoveVolumeProp_TrackMoveStart1~3/Failure` 상태는 발탄에서 쓰이지 않는다 [실측].
- 인원 제한·순서: `EFDeployTrackMoveInfo`와 `EFTrackMoveInfo`에 점유·순서 필드가 없고 프랍 표에도 없다 [실측]. `NextTrackMoveVolumeSpawnIndex`는 "다음 트랙 이동 볼륨 스폰" 번호로 연쇄 배치에 쓰이는 이름이다. 레코드의 두 정수(`+0x1B4`, `+0x1B8`)가 세 프랍 모두 1이라 이 필드가 둘 중 어느 쪽인지, 어떤 뜻인지는 구분하지 못했다 [미확인].
- `EFLinkTrackMoveInputKeyType`(기본/무작위)과 `EFLinkTrackMoveUIType`은 입력 키를 눌러 진행하는 "링크 트랙 이동" 종류로 보이고(GameMsg `sys.trackmove.announce_01`="무작위 키 선정"), 발탄 세 프랍에는 쓰이지 않는 것으로 보인다 [추정].

### 9.6 끝점 착지 가능 여부 (게시된 발탄 navgrid)

`Server/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid`(392×312, 셀 0.5m, 원점 (−6, −165), 보행 셀 21,524개, `.navpolicy` 최대 단차 0)를 읽었다. 형식은 `ServerNavigation.cpp`의 로더와 같고 파일 크기가 형식 계산과 정확히 일치한다 [실측]. 셀 인덱스는 `cellZ * width + cellX`이고 축은 프로젝트 X, Z다.

| 지점 | 셀 | 보행 | 셀 높이 |
|---|---|---|---|
| 프랍·트랙 시작 (97.027, 15.352, −86.921) | (206, 156) | 예 | 15.29 (트랙보다 6cm 낮음) |
| **트랙 끝 (98.530, 20.443, −86.900)** | **(209, 156)** | **예** | **20.4405 (트랙 끝 20.443과 2.5mm 차이)** |
| 우리 현재 착지 (100.42, 20.53, −86.95) | (212, 156) | 예 | 20.529 |
| 우리 `Stage_3` 트리거 중심 (97.761, 15.511, −87.987) | (207, 154) | 아니오(벽 셀, 막힘) | 18.08 |

- 끝점 주변 3×3 셀은 8개가 보행이고, 막힌 것은 서쪽(벽 쪽) 이웃 셀 (208, 156) 하나뿐이다. 시작점 주변 3×3은 전부 보행이다.
- **벽 면**은 x 98.0~98.5 셀(열 208)이 막혀 있다(셀 높이 20.18). 원본 등반 구간의 앞 위치 97.811~97.829m는 열 207(보행, 바닥 15.30)이라 벽 면과 0.17~0.19m 떨어져 있다. 벽을 따라 올라가는 궤적이라는 해석과 맞는다 [실측 + 추정].
- 위 데크(높이 19~22m)에는 끝점 8m 안에 보행 셀이 191개 있다(X 93.75~106.75, Z −94.75~−79.75).
- **막힌 벽 셀(열 208)을 트랙이 지나간다**(마지막 구간 x 97.83→98.53). 우리 서버의 스크립트 이동 `Update_PlayerMotion`은 시작→도착 보간으로 위치를 바로 정하고 navigation·충돌 검사를 하지 않는다(`HEAD`의 `ServerTriggerSystem.cpp` 167~230행) [실측]. 그래서 트랙 궤적이 막힌 셀을 지나도 막히지 않는다. 트랙을 구현할 때 이동 중 navigation 클램프를 넣지 않아야 한다.
- 발탄 런타임 blocker 영역 101개(`.navblockers`)는 모두 x셀 291~355, z셀 54~117 범위이고 벽타기 셀(206~212, 154~158)과 겹치지 않는다 [실측: 파일 전체를 읽음].

### 9.7 뛰기 트랙 1·4 추출 (구현 fork용 데이터)

`out/ValtanWallClimb20260921/jump_tracks.json`에 벽타기와 같은 형식(30Hz 키, 시작 대비 오프셋, 프로젝트 좌표)으로 넣었다. 두 트랙은 **모양이 같다**(키별 오프셋 차이 0.02m 미만).

- 길이 2.733초, 82키, 수평 6.083m(프로젝트 −Z 방향), 서버 yaw 171.39°, **높이 변화 0**(원본 트랙은 직선이고 도약 곡선은 트랙에 없다), `bCanBeHit=false`.
- 속도 곡선: 0~0.2초 초속 0.27m(준비), 0.23~1.03초 급가속(최대 약 17.8m/s)해 약 5.5m 이동, 1.03~2.0초 정지, 2.0~2.73초 약 0.46m를 더 이동. 25%는 0.37초, 50%는 0.50초, 90%는 0.73초에 도달한다.
- 트랙 1: 시작 (28.136, 10.266, −28.568) → 끝 (29.048, 10.265, −34.582). 시작 셀 (68, 272) 보행 10.244, 끝 셀 (70, 260) 보행 10.316.
- 트랙 4: 시작 (30.870, 10.253, −28.814) → 끝 (31.781, 10.253, −34.828). 시작 셀 (73, 272) 보행 10.246, 끝 셀 (75, 260) 보행 10.276.
- 우리 `player_Move.1`(중심 (30.679, 10.04, −28.017), 반크기 [1.1, 0.9, 1.2])은 0.8초에 수평 9.18m를 직선으로 가고 착지 (31.53, 10.34, −37.15)다. 원본 트랙 4의 끝(31.78, 10.25, −34.83)보다 2.3m 더 간다 [실측: 계산].

### 9.8 우리 쪽에 미치는 영향 (구현 fork가 반영할 값)

1. **트리거 박스**: 프랍 중심 (97.027, 15.350, −86.921)m, yaw 약 1.05°. 크기는 반크기 읽기로 반크기 (0.64, 1.28, 0.64)m를 출발점으로 삼되(전체 크기 읽기라면 절반), 단위를 확정하지 못했으므로 조작감은 사용자가 화면으로 판정해야 한다. 현재 [1, 1, 3.8]은 벽 방향으로 지나치게 길다.
2. **시작 규칙**: 시간표 첫 자세(`InitLocation` (97.027, 15.352, −86.921), 서버 yaw 88.3°)를 시작 위치·방향으로 쓰는 것이 원본 구조와 맞다 [추정]. 서버가 시간표의 첫 표본을 위치로 쓰면 자연스럽게 된다. 스냅 거리는 볼륨 크기 안이다.
3. **시간표**: 30Hz 180키(끝 6.0초). 끝점은 보행 셀이라 끝 스냅이 안전하다.
4. **이동 중 navigation 클램프를 넣지 않는다.** 궤적이 벽 면의 막힌 셀을 지난다(9.6).
5. **`bCanBeHit=false`**: 원본은 트랙 중 피격 불가로 보인다 [추정]. 우리 서버에서 무적 처리를 할지는 결정이 필요하다. 시작 순간 이동 중단·공용 행동 취소·탈것 하차가 일어나는 것으로 보이는데(9.2) 우리 서버의 현재 처리는 이번에 확인하지 않았다.
6. **카메라**: 등반용 카메라는 찾지 못했다. 다만 우리 발탄 카메라(55°/18m)는 원본에서 보스 구역 볼륨 안의 값이고 벽타기 구간의 원본 기본은 50°/16m로 보인다(9.3). 바꿀지는 사용자 결정이다.
7. **클립**: 시작 클립이 없으므로 등반 루프를 바로 시작하고 마지막 2.0초에 끝 클립을 붙이는 1차의 안을 유지한다 [추정].
8. **활성화**: 항상 활성이라는 현재 처리를 유지해도 원본과 어긋난다는 증거는 없다. 시작 부분 뛰기 프랍은 원본이 집합 볼륨 충족 뒤 켠다(9.5).

### 9.9 그래도 못 찾은 것과 뒤진 경로

| 항목 | 상태 | 뒤진 경로 |
|---|---|---|
| 원본 트랙 재생·카메라·입력의 네이티브 코드 | [미확인] | `LOSTARK.exe`·`EFEngine.dll` 문자열 검색(게임 고유 트랙 코드 없음), 스크립트 패키지 5개의 이름 표·export(구조체·열거형 정의만 있음, 프로퍼티를 읽는 스크립트 함수 0개). 네이티브 코드 분석은 하지 않았다 |
| Signal의 발신자 | [미확인] | 발탄 존 트리거 유닛 83개의 노드 전체, `SetTriggerUnitEnableState` 10개, `SceneReplay` 표, `ContentsTriggerSignal`(332행, 문자열 표라 발탄과 연결 안 됨), `GameAction` 표 |
| 처음 0.43초의 클립 | [미확인] | 클래스별 PSA 이름 검색, 워로드 AnimSet 3개, `Prop`·`GameAction`·`EFTable` 열 검색 |
| `TrackMoveStartPosition`(float)의 뜻 | [미확인] | 이름·자료형만 확인. 읽는 스크립트 함수 없음 |
| 볼륨 크기 단위(반/전체) | [미확인] | 발탄의 다른 볼륨 프랍 30개 크기와 우리 트리거를 위치로 대조했지만 우리 트리거 박스는 원본 볼륨에서 만든 값이 아니어서 판정에 쓸 수 없었다 |
| 상호작용 거리·바라보기 조건 | [미확인] | Prop 표 전체 열 |
| 카메라 볼륨의 정확한 볼록 다면체 | [미확인] | 경계 상자만 복원(벽타기는 그 밖) |
| `_BASE` 레벨 패키지 | [미확인] | UModel `-nameresolve` 실패 |
| 다른 다섯 클래스의 추가 AnimSet | [미확인] | 기본 AnimSet만 추출 |

### 9.10 2차 산출물

- 문서: 이 절.
- 기계용 데이터: `out/ValtanWallClimb20260921/wall_climb_original.json`(`round2` 항목 추가, 기존 필드 유지), `out/ValtanWallClimb20260921/jump_tracks.json`(신규).
- 추가 스크립트(`C:\LostArkExtract\ValtanClimb_20260921\scripts\`): `prop_record_dump.py`(프랍 레코드 필드 덤프), `prop_volume_probe.py`(볼륨 필드 교차 비교), `volume_vs_our_triggers.py`, `binary_strings.py`(실행 파일 문자열 검색), `u_names.py`, `u_structs.py`, `u_refs.py`, `u_enum_find.py`(스크립트 패키지의 이름·구조체·열거형·참조), `gm_search.py`(GameMsg 검색), `camera_volumes.py`, `camera_volume_brush.py`(카메라 볼륨), `navgrid_sample.py`(발탄 navgrid 표본), `jump_tracks.py`(뛰기 트랙 추출), `build_round2_json.py`(JSON 갱신).
- 추가 추출물(`C:\LostArkExtract\ValtanClimb_20260921\out\`): `valtan_camera_volumes.json`, `valtan_camera_volume_brush.json`, `navgrid_samples_valtan.json`, `jump_tracks.json`.

## 10. 1차 적용 상태 (2026-09-21, 애니메이션·카메라 보류)

- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`의 `Stage_3`에는 원본
  TrackMove 시작 표본을 포함한 **181개** 위치 표본(0~6000ms)을 저장했다. 목표점은
  마지막 표본 `(98.5295, 20.4432, -86.9001)`과 동일하고, 방향은 원본 서버 yaw
  `88.33°`다.
- publisher와 Server bootstrap은 `trackMove.style=wallClimb`을 Valtan에만
  허용하고, 표본 수(2~181), 0ms 첫 표본, 엄격한 시간 증가, 6초 길이 일치, 마지막
  표본과 목표점 일치를 모두 검사한다. 일반 `movePlayer`와 Kouku HUD 이동 payload는
  기존 형식을 유지한다.
- Server는 표본 사이를 보간한 transform만 snapshot으로 복제한다. 이동 중 네비게이션
  클램프·클라이언트 위치 보정은 넣지 않았다. `WALL_CLIMB`은 wire의 append-only
  player action state이므로 protocol은 101이다.
- Client는 `WALL_CLIMB`일 때 terrain jump 클립을 재생하지 않고 idle을 유지한다.
  등반/마무리 클립은 원본 연결 근거를 더 확보한 뒤 별도 단계에서 붙인다.
- 실행한 검증: `Publish-WorldGameplay.ps1 -Mode Validate -WorldId VALTAN_ARENA`
  통과, scratch bootstrap에서 `WALL_CLIMB`, payload 732, 181 표본, 0~6000ms,
  yaw 88.33을 확인했다. Server contract에는 bootstrap 파싱, 중간 보간, 마지막
  착지, 잘못된 마지막 시간 rollback 사례를 추가했다.
- 이 절을 기록한 시점에는 Server/Client product build, 사용자 화면 확인, runtime
  DataFiles publish, 원본 등반 애니메이션 설치, 원본 기본 카메라(50°/16m) 전환이
  남아 있었다. 애니메이션 설치 상태는 뒤의 11절로 갱신했다. Client build와 육안
  확인 전에는 시각 PASS를 주장하지 않는다.

## 11. 원본 등반·마무리 클립 연결 (2026-09-21, Client 빌드 대기)

- 원본 PSA에서 `act_creep_up_1`을 `wall_climb_loop`, `act_creep_up_end_1`을
  `wall_climb_end`로 6개 직업 각각에 구웠다. 결과는
  `Client/Bin/Resources/Character/<Class>/AnimSets/<Class>_WallClimbAnimSet.wmodel`이다.
  각 결과물은 두 클립만 가지며 설치된 몸 모델의 skeleton section이 byte-for-byte
  보존됨을 cook receipt로 확인했다.
- `CharacterCatalog.json`은 6개 AnimSet을 모두 등록했고, 각
  `<Class>.interactionbindings.json`에는 `wallClimb.loop/end/endStartSeconds=4`를
  추가했다. 한 직업만 리소스가 빠져 Idle로 격리되는 상태가 되지 않도록 6개를 같은
  변경 단위로 설치했다.
- `CCharacter`는 `WALL_CLIMB` action의 Server action clock으로 0~4초에 loop를
  반복 seek하고, 4~6초에는 end를 seek한다. Server TrackMove 위치만 권위가 있으므로
  재생 중 vertical root motion scale은 0, 다른 action snapshot을 받으면 1로 복원한다.
- 확인한 자동 검증: 6개 JSON parse, 새 cooker Python syntax, cooker receipt의 두
  clip 이름·원본 SHA-256·skeleton 보존이다. C++ Client build와 사용자 G-key 화면
  검증은 Visual Studio가 실행 중이라 아직 하지 않았다.
