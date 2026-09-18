# 발탄 원본 카메라 교체 — G03 재추출 / G04 매핑 / G05 표현 판정

작성일: 2026-09-17. 브랜치 `feature/kouku-cutscene-camera-map-0915`, HEAD `bddacace`.
입력 계획서: Codex 작성 `2026-09-17_VALTAN_SOURCE_CAMERA_REPLACEMENT_PLAN.md`
(`C:/Users/USER/.codex/worktrees/7395/LostArk/.md/GB/09-17/`). 그 문서의 G03~G05를 실제 저장소에서 수행한 결과다.

**이 문서의 상태: 조사·판정까지다.** `Data/Encounters/Valtan/ValtanCinematicCamera.json`,
`Data/Valtan/*.json`, Client C++을 수정하지 않았고 빌드하지 않았다. 설치는 아래 G04 승인 게이트를
사용자가 결정한 뒤에 한다. 추출 완료 ≠ 설치 완료 ≠ 전투 연결 완료 ≠ 원작 영상 일치.

---

## 1. G03 — 추출기 교체와 재추출

### 1-1. 기존 추출기의 결함 (실제 코드에서 확인)

`out/ValtanCameraReplace20260917/extract_all_cameras.py`에서 확인한 손실 경로다.

| 줄 | 코드 | 손실 |
|---|---|---|
| 65 | `for step in range(5)` | 컷 길이와 무관하게 5점만 남김 |
| 76 | `entry["cameras"][target] = samples` | 같은 카메라가 두 번 나오면 앞 컷을 덮어씀 |
| 55 | `start = max(0.0, float(cut["time"]))` | 원본 음수 컷 시작을 버림 |
| 45 | `directors[0]` | 활성 director가 여럿일 때 임의 선택 |
| 60 | `actors[0]` | 그룹 actor가 여럿일 때 임의 선택 |
| 62 | `fov_tracks[-1]` | 활성 fovangle 트랙이 여럿일 때 임의 선택 |
| 70 | `base.curve(pts, t, 50.0)` | FOV 트랙이 없으면 50도를 지어냄 |
| 61 | `if not actors: continue` | actor 누락을 조용히 건너뜀 |

### 1-2. 새 추출기

`out/ValtanCameraReplace20260917/extract_all_cameras_v2.py` (신규, 기존 파일은 그대로 둠).
기존 `ORIGINAL_CAMERAS.json`은 `$CLAUDE_JOB_DIR/tmp/ORIGINAL_CAMERAS.backup.json`에 백업했고
새 결과는 `out/ValtanCameraReplace20260917/ORIGINAL_CAMERAS_V2.json`이다.

바뀐 계약:

- 컷 단위 저장. 키는 `<scene>.matinee<export>.cut<NN>.<camera>`이므로 같은 카메라가 재등장해도 덮이지 않는다.
- `rawStartSeconds`(음수 보존)와 `effectiveStartSeconds`(0 클램프)를 따로 기록한다.
- 활성 director / 그룹 actor / 활성 fovangle 트랙이 1개가 아니면 **임의 선택 대신 실패로 기록**한다.
- FOV 트랙이 없으면 기본값을 넣지 않고 `fovSource: "absent"`와 `null`을 남긴다.
- 위치·회전·FOV의 **원본 키 전체**를 `interpMode`, `arrivetangent`, `leavetangent`까지 보존한다.
- `moveFrame`, `lookuptrack` 이름, actor의 `base`/`basebonename`을 기록한다.
- 포즈는 `world_pose`가 주는 클라 좌표계 회전행렬에서 0열 forward, 1열 up, 2열 −right로 뽑아
  **up/right까지** 남긴다. 기존 추출은 forward만 남겨 롤을 못 봤다.

### 1-3. 재추출 결과

카메라 컷 **16개** + 게임플레이 복귀 컷 3개. 실패 0건.
카메라 없는 3개(벽·기둥, 바닥 좌/우)는 의도대로 실패로 기록됐다.

| 연출 | matinee | 길이 | 활성/비활성 director | 컷 |
|---|---|---|---|---|
| 1관문 입장(늑대) | SCENE07A m24 | 13.000001s | 1 / 2 | cam1 (−0.033~13.000) |
| 발탄 등장(부활) | SCENE06A m53 | 24.707817s (edSectionEnd 23.385742) | 1 / 1 | c(−0.051) → c1(13.059) → c(18.966) → 복귀(23.386) |
| 버러지 | SCENE06A m54 | 6.374481s | 1 / 1 | c(−0.137) → c1(1.073) → 복귀(5.736) |
| 포효 | SCENE06A m55 | 7.002931s | 1 / 0 | c(−0.293) → c1(1.461) → c(3.621) → 복귀(5.001) |
| 도약·내려찍기 | SCENE06A m56 | 6.723904s | 1 / 0 | c(−0.293) → c1(4.105) → c(6.265) |
| 발탄 최후 | SCENE04A m24 | 23.0s | 1 / 2 | cam1(0.000) → cam2(3.650) → cam3(7.780) → cam6(15.410) |
| 아레나 벽·기둥 | SCENE02A m52 | 5.5s | **0** / 0 | 활성 director 없음 |
| 바닥 붕괴(왼쪽) | SCENE02A01 m13 | 2.5s | 1 / 0 | **컷 0개** |
| 바닥 붕괴(오른쪽) | SCENE02A02 m12 | 2.5s | **0** / 0 | 활성 director 없음 |

확인된 사실:

- **모든 컷의 `transitiontime`이 0이다.** 컷 경계에 원본 블렌드가 없다. 계획서 G05 주장과 일치한다.
- **보간 모드에 `cim_linear`가 하나도 없다.** 위치·회전은 전부 `cim_curveautoclamped`(발탄 등장 c1만
  `cim_curveauto` 혼합), FOV는 `cim_curveautoclamped`와 `cim_constant` 혼합이다.
  즉 원본은 전 구간 곡선이고, v6의 LINEAR 재생은 근사다.
- **음수 시작 컷 5개**: 늑대 −0.033, 등장 −0.051, 버러지 −0.137, 포효 −0.293, 도약 −0.293.
- 컷 구간 내 소스 키 수는 최대 23개(최후 cam3)로 v6의 64키 상한 안이다.

---

## 2. G05 — v6로 표현 가능한가

### 2-1. 판정에 쓴 제약 (실제 코드)

`Client/Private/ValtanCinematicCameraDocument.cpp` 기준이다.

- `Read_Tracking`(231행): `trackingMode`/`trackingOrigin`이 **둘 다 없을 때만** WORLD. 명시 문자열 `"WORLD"`는 거부.
- provenance(424행)는 `"PROJECT_AUTHORED"`만, 직렬화(690행)도 하드코딩.
- `Read_Fov`(262행): **10.0 미만 / 120.0 초과 거부.** `CameraTool.cpp:212` 포즈 검증도 같은 범위.
- cue 32개, cue당 키 2~64개. `cue.durationMs > stage.durationMs`면 거부(480행).
- 키 객체는 `Is_ExactObject`로 `{sceneId, timeMs, eye, lookAt, fovYDegrees}` **정확히 그것만**. `up`/`roll` 불가.
- `sceneIds`(441행)는 cue와 deathCue가 공유 → **sceneId는 문서 전체에서 유일**.
- 첫 키 `timeMs == 0` 필수, 이후 **강한 증가**(같은 ms 두 키 불가), 마지막 키 == cue.durationMs, `eye != lookAt`.

### 2-2. 적응형 키 배치 결과

허용오차를 위치 0.05m / forward 0.5도 / fovY 0.25도로 두고, 컷 경계와 구간 내 소스 키에서 시작해
1/120초 기준 최대오차 지점에 키를 하나씩 삽입했다. 재생은 v6 LINEAR 가정이다.

| 연출 | 컷 | 길이 | 필요 키 | 판정 | 잔차(위치/fwd/fovY) |
|---|---|---|---|---|---|
| 늑대 | cam1 | 13.000s | 55 | 충족 | 0.0493m / 0.000° / 0.000° |
| 등장 | c | 13.059s | 32 | 충족 | 0.0452m / 0.302° / 0.000° |
| 등장 | c1 | 5.907s | 64 | **미달** | 0.0661m / 0.476° / 0.000° |
| 등장 | c | 4.420s | 8 | 충족 | 0.0327m / 0.000° / 0.000° |
| 버러지 | c | 1.073s | 10 | 충족 | 0.0452m / 0.259° / 0.000° |
| 버러지 | c1 | 4.663s | 47 | 충족 | 0.0451m / 0.483° / 0.222° |
| 포효 | c | 1.461s | 20 | 충족 | 0.0390m / 0.233° / 0.000° |
| 포효 | c1 | 2.160s | 17 | 충족 | 0.0343m / 0.065° / 0.232° |
| 포효 | c | 1.379s | 10 | 충족 | 0.0067m / 0.451° / 0.000° |
| 도약 | c | 4.105s | 51 | 충족 | 0.0479m / 0.441° / 0.000° |
| 도약 | c1 | 2.160s | 10 | 충족 | 0.0495m / 0.106° / 0.134° |
| 도약 | c | 0.459s | 19 | 충족 | 0.0478m / 0.175° / 0.000° |
| 최후 | cam1 | 3.650s | 7 | 충족 | 0.0498m / 0.038° / 0.000° |
| 최후 | cam2 | 4.130s | 12 | 충족 | 0.0361m / 0.455° / 0.000° |
| 최후 | cam3 | 7.630s | 29 | 충족 | 0.0471m / 0.483° / 0.000° |
| 최후 | cam6 | 7.590s | 9 | 충족 | 0.0177m / 0.009° / 0.000° |

**16컷 중 15컷이 64키 안에서 허용오차를 충족한다.** 미달 1컷(등장 c1)도 64키에서 위치 6.6cm로,
허용오차 5cm를 1.6cm 넘긴 정도다. 키 상한은 이 작업의 실질 제약이 아니다.

균등 배치로는 16컷 중 9컷만 충족했다(늑대 64키에서 0.285m 잔차, 포효 c1 fovY 28.4도 잔차).
차이는 `cim_constant` FOV 유지 구간과 고곡률 구간 때문이며, **적응형 배치가 필수**라는 뜻이다.

### 2-3. v6가 막는 것 — 실측으로 좁힌 3가지

1. **FOV 하한 10도 — 2컷.**
   포효 c1은 fovY 최소 **9.6107도**(fovH 17), 도약 c1은 **8.6936도**(fovH 15.3918).
   나머지 14컷은 11.33도 이상이라 통과한다. 즉 이 제약은 두 컷의 클로즈업 구간만 막는다.

2. **롤 — 2컷.**
   카메라 right 벡터의 월드 수직 성분으로 잰 롤은 **발탄 등장 c(10.018도), 등장 c1(19.977도)** 두 컷에만 있다.
   나머지 14컷은 0.04도 미만으로 사실상 롤이 없다.
   v6 키는 `{sceneId,timeMs,eye,lookAt,fovYDegrees}`만 허용하므로 이 두 컷의 기울기는 표현할 수 없다.
   **발탄 등장을 원본대로 넣으려면 parser·serializer·CameraTool·sampler·카메라 적용까지 up 지원이 필요하다.**
   나머지 연출은 up 없이도 원본과 같다.

3. **음수 시작 — 5컷.**
   첫 키가 0ms여야 하므로 원본 음수 구간을 그대로 저장할 수 없다. 해당 컷은 t=0에서 곡선을 평가한 값을
   첫 키로 넣는 변환이 필요하다. 추출물의 `rawStartSeconds`/`effectiveStartSeconds`와
   `boundarySamples`가 그 변환의 근거다. 음수 구간을 0으로 클램프하고 그 사실을 적지 않으면 안 된다.

**하드컷**은 v6가 같은 ms 두 키를 금지하므로 cue 내부에서 표현할 수 없다.
컷마다 cue를 따로 두면 되지만, 그 경우 cue는 stage에 붙으므로 stage가 컷 수만큼 필요해진다 —
이는 계획서가 금지한 전투 stage 증설이다. **컷 경계를 document/controller의 별도 개념으로 추가하는 것이
유일하게 남는 방법이며, 이 작업은 아직 하지 않았다.**

---

## 3. G04 — 원본 이벤트와 현재 발동 경로 매핑

### 3-1. 원본 쪽 트리거 (기존 실측: `out/ValtanCutsceneSource20260916/SOURCE_REPORT.md`)

| 원본 event | 존 37051 트리거 유닛 | 체인 |
|---|---|---|
| `Scene_01` 늑대 | #61 | `Condition_AssembleVolumeProp` → `StartBossBattle` → … → `SceneEvent Scene_01` → `TeleportPlayer` |
| `Scene_02` 등장 | #1·#5 (#81도 발사) | `StartBossBattle` → 부활·워프 → `SpawnNPC` → `SceneEvent Scene_02` |
| `scene_03` 최후 | #4 (#82도 발사) | `Condition_Dungeon(Clear)` → `EndBossBattle` → `ReviveAllPlayer` → `SceneEvent scene_03` |
| `Event_05` 버러지 | #14 | `Condition_NPC(Event5)` → `SceneEvent Event_05` → `SendAISignal AS_020` |
| `Event_01` 포효 | #6 | `Condition_NPC(Event1)` → `SceneEvent Event_01` → `SendAISignal AS_010` |
| `scene_11` 도약 | **없음** | 두 존 트리거 문자열에서 발견 실패 (미확인) |
| `Event_02` 벽·기둥 | #9 | `Condition_NPC(Event2)` → `DestroyHitProp` → `DespawnNPC` → `SceneEvent Event_02` |

### 3-2. 현재 쪽 소비자

`sourceActionIds`는 매핑 근거가 못 된다. 실측 결과 20개 action ID가 여러 패턴에 중복되며,
`420633` → `VALTAN_ENTRANCE_CINEMATIC`, `VALTAN_WHIRLWIND` 2개,
`420629` → `ARENA_BREAK_109`, `TERRAIN_DESTRUCTION_3_OCLOCK`, `TERRAIN_DESTRUCTION_9_OCLOCK`,
`SIX_PIZZA_106`, `TERRAIN_DESTRUCTION`, `TRASH` 6개다.

현재 카메라 cue는 11개 + deathCue이고, `cameraInvocation`은 9개다.
`VALTAN_FOUR_PILLARS_105`는 gameplay 패턴 자체가 없고 invocation도 없어 **재생 경로가 없다.**
다만 `Valtan.actionbindings.json`, `patterneffectcues`, `patternshakecues`, `patternsoundcues`,
`rootmotion`, Effect 문서 2개, balance receipt가 이 ID를 참조하므로 **삭제하지 않는다.**

`VALTAN_ENTRANCE_CINEMATIC`은 `scriptedSequence` 52개 목록에 **없다**(ARENA_BREAK_109는 34번).
즉 현재 등장 컷신은 자동 재생되지 않고 수동 재생 경로만 있다.

### 3-3. 대상별 판정

| 원본 | 현재 소비자 | pattern/stage 또는 DEAD | cueId | 판정 |
|---|---|---|---|---|
| `Scene_02` 등장 (24.708s, 3컷+복귀) | 있음 | `VALTAN_ENTRANCE_CINEMATIC` / ESTABLISH·ARENA_REVEAL·HERO_HANDOFF | `camera.valtan.entrance.*` 3개 | **승인 게이트 A** (시간 충돌 + 롤 필요) |
| `scene_03` 최후 (23.0s, 4컷) | 있음 | DEAD 경로 deathCue | `camera.valtan.clear.wide` | **승인 게이트 C** (23.0s vs 2.09s) |
| `Scene_01` 늑대 (13.0s, 1컷) | **없음** (09-16에 팀장 보류로 철회) | — | — | **승인 게이트 B** (보류 해제 필요) |
| `Event_05` 버러지 (6.374s, 2컷+복귀) | 없음 | AS_020 ↔ 현재 패턴 연결 근거 없음 | — | **연결 근거 없음** |
| `Event_01` 포효 (7.003s, 3컷+복귀) | 없음 | AS_010 ↔ 현재 패턴 연결 근거 없음 | — | **연결 근거 없음** |
| `scene_11` 도약 (6.724s, 3컷) | 없음 | 원본 트리거 자체가 미확인 | — | **연결 근거 없음** |
| `Event_02` 벽·기둥 | — | `VALTAN_ARENA_BREAK_109` 6 cue | `camera.valtan.arena-break-109.*` | **원본에 카메라 컷 0개 → 교체 대상 아님, 현행 유지** |
| `Event_03`/`Event_04` 바닥 | — | — | — | 카메라 없음, 대상 아님 |
| — | 없음 | `VALTAN_FOUR_PILLARS_105` | `camera.valtan.four-pillars-105.*` 2개 | **재생 경로 없음이나 타 문서가 참조 → 유지** |

---

## 4. 승인 게이트 — 사용자 결정 필요

### 게이트 A. 발탄 등장

원본 카메라 소유 구간은 **23.386초**(그 뒤 1.322초는 게임플레이 복귀), 전체 matinee는 24.708초다.
현재 `VALTAN_ENTRANCE_CINEMATIC`의 stage 합은 **19.867초**(8600 + 5800 + 5467)이고
카메라 cue 합은 18.867초(8600 + 5800 + 4467 — HERO_HANDOFF는 transitionOut 1000ms와 합쳐 5467).

원본 컷 경계(0 / 13.059 / 18.966 / 23.386)와 현재 stage 경계(0 / 8.6 / 14.4 / 19.867)가 맞지 않는다.
추가로 이 연출의 두 컷에는 롤 10.0도·20.0도가 있어 **v6 키로는 표현할 수 없다.**

선택지:
- (A1) 현재 19.867초에 맞춰 원본 카메라를 재타이밍한다. 전투 시간 불변, 원본 컷 길이 비율이 달라진다.
- (A2) 원본 23.386초에 맞춰 encounter stage 길이를 늘린다. **Server 전투 계약 변경 — 별도 승인 필요.**
- (A3) 이 대상은 보류하고 현재 카메라를 유지한다.

롤은 어느 선택이든 별도다: (R1) up 필드를 v6에 추가(parser·serializer·CameraTool·sampler·적용 동시 확장),
(R2) 롤을 버리고 수평 카메라로 근사(원본과 다름을 기록).

### 게이트 B. 늑대 입장

`.md/GB/09-16/2026-09-16_VALTAN_LUGARU_ENTRANCE_CAMERA_RESULT.md` 8절에 팀장 보류로
`camerashots.json`과 `Level_ValtanArena` 카메라 코드를 전부 철회한 기록이 있다.
원본 재추출은 마쳤고(cam1, 13초, 55키로 4.9cm 이내) 데이터는 준비돼 있으나,
**보류 해제 없이는 설치하지 않는다.** 또한 이 구간은 발탄 보스가 없어 발탄 패턴 cue로는 재생할 수 없고,
쿠크식 Area camerashots 같은 별도 발동 수단이 필요하다.

### 게이트 C. 발탄 최후

원본은 23.0초 4컷이고 현재 `deathCue`는 2.09초 3키 1개다.
deathCue는 stage가 아니라 보스 DEAD 상태에서 재생되므로 stage 길이 제약은 받지 않지만,
23초 동안 카메라를 잡으려면 현재 사망·보상·퇴장 흐름과 entity/Level 수명이 그 시간을 버텨야 한다.
그 확인과 필요한 gameplay 변경은 이 작업 범위 밖이다.

선택지: (C1) 2.09초에 맞춰 원본 앞부분만, (C2) 23초 전체로 늘리고 수명 검토·변경을 별도 승인,
(C3) 현행 유지.

### 게이트 없이 통과 가능한 대상

**없다.** 버러지·포효·도약은 원본 카메라 추출은 끝났으나 현재 발동 경로에 연결할 근거가 없어
설치 대상이 아니다. 벽·기둥과 바닥은 원본에 카메라 컷이 없다.
따라서 **지금 승인 없이 교체할 수 있는 cue는 하나도 없다.**

---

## 5. 수행하지 않은 것

- `ValtanCinematicCamera.json`, `Valtan.presentation.json`, Client C++ 수정 없음.
- 빌드·publisher 실행 없음. Client 실행·화면 확인 없음.
- 컷 경계 표현, up 필드, FOV 하한 완화의 구현 없음. G05는 판정까지다.
- `scene_11`·`scene_12` 원본 트리거는 여전히 미확인이다.
- 원본 슬로모가 시계에 주는 영향은 확인하지 않았다. 위 시간은 matinee 시간축 기준이다.

## 6. 산출물

- `out/ValtanCameraReplace20260917/extract_all_cameras_v2.py` (신규 추출기)
- `out/ValtanCameraReplace20260917/ORIGINAL_CAMERAS_V2.json` (컷 16개 전체 곡선·탄젠트·up·경계 표본)
- 기존 `ORIGINAL_CAMERAS.json`은 변경하지 않았고 백업본은 작업 임시 폴더에 있다.
