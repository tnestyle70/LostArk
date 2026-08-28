# 발탄 Cinematic Camera Tool 구현 결과

## 0. 결론

기존 F1 `Camera Tool`과 `CValtanCinematicCameraController`를 정본으로 확장했다. 별도의
`CameraCuts.json`이나 두 번째 컷신 런타임은 만들지 않았다.

Camera Tool에서 LookAt Dummy, 현재 카메라 Capture, stable scene 목록 관리,
`LINEAR/CATMULL_ROM` 보간, 구간 목표 속도 기반 시간 재배치, preview 속도, strict JSON 저장/불러오기를
사용할 수 있다. 제품 런타임도 Tool과 동일한 sampler와 tracking 변환을 소비한다.

이 작업은 우선 요청된 Tool 기반을 발탄 카메라 문서에 완성한 범위다. 베른 첫 입장용
`LEVEL_ENTRY` cue와 범용 Area 컷신 실행기는 아직 이 문서에 연결하지 않았다. 또한 Dummy는 제품
월드 충돌이 없는 Debug sphere collider이며, 현재 위치 조정은 수치 입력 방식이다. viewport에서
마우스로 끄는 transform gizmo는 이 범위에 포함하지 않았다.

## 1. 구현 내용

### 1.1 LookAt Dummy와 Capture

- `Prototype_Component_Collider_WorldEntity`를 복제한 Debug 전용 sphere를 만든다.
- Dummy는 render/debug 표시만 하며 gameplay collision, physics, Server authority에 참여하지 않는다.
- 선택 scene의 LookAt에 Dummy를 동기화하고 world 위치를 직접 수정하면 선택 scene과 preview에 즉시 반영한다.
- 잘못된 scene, level/cue 변경, 비정상 pose, 동기화 실패 시 Dummy를 끄고 collider 상태를 초기화한다.
- Dummy 반경은 0.05~5m로 제한하며 NaN/무한값은 기존 정상값을 보존한 채 거부한다.
- 현재 camera의 Eye/Look/FOV를 선택 scene에 덮어쓰거나 새 scene으로 Capture할 수 있다.
- tracking cue는 제품의 inverse tracking을 사용해 world camera pose를 authored 좌표로 되돌려 저장한다.

### 1.2 scene 목록과 spline

- 각 keyframe은 전 문서에서 유일한 `sceneId`를 가진다.
- 자동 생성 ID는 `camera.scene.auto.<n>`이며 저장 순서나 vector index를 identity로 사용하지 않는다.
- scene 선택, 이동, 현재 시각 sampled scene 삽입, Capture New, Replace, interior scene 삭제를 제공한다.
- 첫 scene은 0ms, 마지막 scene은 cue duration과 같고 중간 시간은 엄격한 오름차순이다.
- `LINEAR`는 기존 화면을 보존한다.
- `CATMULL_ROM`은 양 끝점을 clamp한 4-point spline을 사용한다. Eye와 LookAt이 겹치거나 비정상 pose가
  되면 같은 구간의 linear 결과로 안전하게 되돌아간다.
- 제품 Controller와 Tool preview가 같은 public `Sample_Cue`를 호출한다.

### 1.3 속도 제어

- 선택 scene의 직전 구간을 제품 sampler로 세분화해 world arc length를 계산한다.
- `목표 속도(m/s)`를 적용하면 거리/속도로 새 segment duration을 계산하고 선택 scene 이후 시간을 함께
  이동한다. 즉 제품 데이터의 실제 timeMs와 cue duration을 바꾸며 별도 runtime 배속을 만들지 않는다.
- uint32 overflow, 최소 시간 간격, stage duration 초과를 사전에 거부해 draft를 부분 변경하지 않는다.
- Preview Rate는 저작 확인용이며 저장 데이터나 제품 재생 속도에는 섞이지 않는다.

### 1.4 문서·저장 계약

- `ValtanCinematicCamera.json`은 camera-only formatVersion 6이다.
- cue에는 `interpolation`, keyframe에는 stable `sceneId`가 필수다.
- scene/cue ID, pattern/stage tuple, duration, easing, tracking, shake, Eye/Look/FOV, cue/key 개수 상한을
  C++ parser와 publisher가 같은 규칙으로 검사한다.
- 저장은 `draft serialize -> strict reparse -> process 간 sidecar lock -> exact-byte 사전 비교 -> sibling
  temporary write/flush -> temporary strict reparse -> 두 번째 사전 비교 -> ReplaceFileW` 순서다.
- `ReplaceFileW`가 실제로 밀어낸 destination을 고유 backup으로 보존한 뒤 baseline과 다시 비교한다. lock을
  따르지 않는 외부 편집기가 비교와 교체 사이에 쓴 경우 새 저장을 성공 처리하지 않고, 그 bytes를 원위치로
  rollback하거나 고유 recovery 파일로 남긴다. 동시 저작 bytes를 조용히 덮어쓰지 않는다.
- temporary/backup/recovery 이름은 `CREATE_NEW`로 원자 예약하고 충돌하면 다음 후보로 재시도한다. Client
  재시작과 PID 재사용 뒤에도 이전에 보존한 recovery 파일을 선삭제하거나 덮어쓰지 않는다.
- 실패 시 draft와 preview를 보존하며, 충돌 복구 자체가 실패해도 외부 bytes가 든 backup/recovery 경로를
  상태 메시지에 남긴다.
- 저장 성공 뒤 실행 중 encounter를 hot reload하지 않는다. 다음 발탄 Level 진입부터 새 파일을 소비한다.

### 1.5 camera owner 복구

- Authoring Preview 10, Reference Audition 20, Server Cinematic 100 우선순위를 사용한다.
- Server cinematic이 Tool을 선점해도 최초 camera world/FOV를 유지하고 최종 owner 종료 때 복구한다.
- Tool 닫기, F1 전환, Level 전환, sampler/pose 실패 때 Tool 소유 override만 반환한다.

## 2. 자동 검증

| 검증 | 결과 |
|---|---|
| `test_valtan_camera_tool_contract.py` | PASS |
| Win32 `ReplaceFileW` displaced-backup 실행 probe | PASS |
| GameplayBalance canonical Validate | PASS |
| ActionPresentationTimelineHarness Debug build/run | PASS |
| ActionPresentationTimelineHarness Release build/run | PASS |
| format v6 roundtrip, legacy/null/실수형 정수 rejection, duplicate ID/tuple rejection | PASS |
| LINEAR/CATMULL sampler 및 crossing linear fallback | PASS |
| BOSS_XZ/BOSS_FACING/PLAYER_BOSS_FRAME Apply/Remove roundtrip | PASS |
| Client x64 Debug build | PASS |
| Client x64 Release build | PASS |
| camera JSON parse / PowerShell parse / `git diff --check` | PASS |

Client/UI는 에이전트가 실행하거나 조작하지 않았다. framing, spline 감각, Dummy 위치와 이동 속도의 최종
visual fidelity는 사용자가 직접 확인해야 한다.

## 3. 사용자 확인 순서

1. 실행 중인 Client를 완전히 종료하고 최신 Debug Client를 실행한다.
2. Lobby에서 Server 승인을 받아 Valtan Arena에 들어간다.
3. `F1 -> Camera Tool`을 연다.
4. cue와 scene을 선택하고 `Show LookAt Dummy`를 켠다.
5. Dummy world position을 조절하거나 free camera를 잡은 뒤 `Capture/Replace` 또는
   `Capture New Scene`을 사용한다.
6. cue interpolation을 `CATMULL_ROM`으로 바꾸고 Play/Scrub으로 경로를 확인한다.
7. 첫 scene이 아닌 scene을 선택하고 Segment Target Speed를 입력한 뒤 `Apply Segment Speed`를 누른다.
8. `Save`를 누른 뒤 발탄 Level을 나갔다가 다시 들어와 제품 컷신으로 확인한다.

기존 cue는 화면이 갑자기 달라지지 않도록 전부 `LINEAR`로 이관했다. 사용자가 선택한 cue만
`CATMULL_ROM`으로 바꾸면 된다.

## 4. 다음 확장 경계

- Bern 첫 입장, Valtan 등장, Valtan 사망처럼 trigger 종류가 다른 컷신을 한 Tool에서 만들려면 다음
  수직 슬라이스에서 범용 camera document adapter와 `LEVEL_ENTRY / ENCOUNTER_START /
  ENCOUNTER_COMPLETE` binding을 추가한다.
- 이때도 sampler, camera owner, scene ID와 atomic save 계약은 이번 구현을 그대로 재사용한다.
- 신규 cue 생성/삭제와 presentation invocation 편집은 여러 정본을 한 transaction으로 저장해야 하므로
  기존 cue 내부 scene 편집과 분리해 구현한다.
- viewport transform gizmo는 Debug Dummy의 typed world transform 명령으로 추가할 수 있다.

PR, commit, push는 수행하지 않았다.

## 5. 2026-08-28 Camera Cut 직접 저작 확장

참고 영상의 Camera Tool처럼 사용자가 컷 단위로 카메라를 직접 저작할 수 있도록 기존 F1
`Camera Tool`을 확장했다. 별도 컷 포맷이나 두 번째 런타임은 만들지 않았고, 기존 cue의 scene 목록에서
1ms 간격인 인접 scene 쌍을 하드 컷 경계로 해석한다.

- `Camera Cut List`: 현재 cue의 Cut 1..N과 각 시간 범위를 표시하고 선택 즉시 해당 컷 첫 장면을
  제품 sampler로 preview한다.
- `New Cut From Current Camera`: timeline의 T-1ms에 기존 진행 pose를 보존하고 T에 현재 free camera의
  Eye/LookAt/FOV를 캡처한다.
- `Delete Selected Cut`: 첫 컷, 중간 컷, 마지막 컷을 삭제해도 남은 컷의 시작 0ms와 마지막 duration,
  strict time 순서를 다시 연결한다.
- `Release Camera / Keep Time`: timeline cursor는 유지하면서 preview의 camera 소유권만 반환해 사용자가
  free camera를 움직인 뒤 새 컷을 캡처할 수 있게 한다.
- 기존 `Camera Position List / Saved Scenes`, LookAt Dummy, Capture/Replace, segment speed,
  Play/Pause/Stop, Save/Reload/Validate 기능은 같은 cue와 JSON을 계속 사용한다.
- 1ms 하드 컷 경계는 구간 속도 편집 대상에서 제외한다. 따라서 속도 변경 때문에 하드 컷이 보간 구간으로
  바뀌지 않는다.
- 새 컷은 ID 두 개를 모두 사전 확보하고 pose 캡처까지 성공한 뒤에만 draft에 삽입한다. 시간 충돌,
  scene 상한, camera 캡처 실패 시 기존 draft를 유지한다.

### 직접 저작 순서

1. `F1 -> Camera Tool`을 열고 cue를 선택한다.
2. `Time (s)`로 새 컷을 넣을 시점을 맞춘다.
3. `Release Camera / Keep Time`을 누른다.
4. F6 free camera를 원하는 Eye 위치와 방향으로 움직인다. 필요하면 LookAt Dummy를 켜고 목표점을
   수치로 조절한다.
5. `New Cut From Current Camera`를 눌러 새 하드 컷을 만든다.
6. 컷 안의 이동 장면은 `Camera Position List / Saved Scenes`에서 Capture New/Replace하고, 뒤쪽 장면을
   선택해 `Apply Segment Speed`로 이동 시간을 조절한다.
7. Play/Pause/Scrub으로 확인하고 `Save`한다. 제품 런타임은 hot reload하지 않으므로 발탄 Level을 나갔다가
   다시 들어와야 저장 결과를 소비한다.

### 추가 검증

| 검증 | 결과 |
|---|---|
| Cut List/New/Delete/Release/source rollback 계약 | PASS |
| 발탄 클리어 cue 13.967초 및 1ms 하드 컷 2개 계약 | PASS |
| GameplayBalance `Validate` | PASS (`54 patterns / 230 stages`) |
| ActionPresentationTimelineHarness Debug build/run | PASS |
| ActionPresentationTimelineHarness Release build/run | PASS |
| Client x64 Debug build | PASS |
| Client x64 Release build | PASS |

Client 화면의 조작감과 최종 카메라 framing은 사용자 육안 확인 대상이다. 에이전트가 Client를 실행하거나
visual PASS로 대신 판정하지 않았다.

## 6. Cue List에서 새 cue 만들기와 삭제

왼쪽 `Cue List` 위에 다음 저작 기능을 추가했다.

- `New Cue ID`: stable ID를 직접 입력한다. 비우면 `camera.cue.auto.<n>`을 자동 생성한다.
- `New Cue Binding`: 아직 camera cue가 없는 Encounter의 `patternId / stageId`를 선택한다.
- `New Cue From Current Camera`: 현재 free camera를 시작/끝 scene으로 캡처한 새 cue를 만든다.
- `Delete Selected Cue`: 선택한 일반 cue를 draft에서 삭제한다.

새 cue는 선택 stage의 길이를 넘지 않는 기본 3초 cue로 생성된다. 생성 직후 오른쪽에서 Duration,
Tracking, interpolation, Cut, Saved Scene, LookAt, Speed를 계속 편집할 수 있다. 동일 ID나 동일 stage
binding, 32 cue 상한, 잘못된 camera pose는 기존 draft를 변경하지 않고 거부한다. 전용 Death cue와
마지막 일반 cue는 삭제할 수 없다.

사용 순서는 `F1 -> Camera Tool -> Release Camera / Keep Time -> F6 free camera 위치 조절 -> New Cue
ID/Binding 선택 -> New Cue From Current Camera -> 오른쪽 장면 편집 -> Validate -> Save`다.

Cue 생성·삭제 확장 뒤 Camera Tool 계약, GameplayBalance Validate, Debug/Release Client build와
Debug/Release ActionPresentationTimelineHarness를 다시 실행해 모두 PASS를 확인했다.

## 7. 캡처 버튼만 사용하는 간편 카메라 경로

`Easy Camera Path Capture`를 추가해 timeline 시간을 직접 계산하지 않고 다음 흐름으로 하나의 연속
카메라 무빙을 만들 수 있다.

```text
free camera를 첫 위치로 이동
-> Capture First Point / Start New Path
-> 다음 위치로 이동
-> Capture Next Point
-> 필요한 만큼 이동과 Capture Next Point 반복
-> Play Captured Path
```

`Point Interval (ms)`는 지점 하나에서 다음 지점까지 걸리는 시간이다. 기본값 1500은 지점 사이를
1.5초에 이동한다는 뜻이다. 값이 작을수록 빨라지고 클수록 느려진다. 첫 캡처는 선택 cue의 기존 scene
목록을 새 경로로 교체하므로, 기존 경로를 보존해야 하면 먼저 새 cue를 만들거나 잘못 누른 즉시 Save
전에 `Reload`한다.

첫 지점은 0ms, 두 번째 지점은 interval, 세 번째부터는 interval만큼 자동 증가한다. 사용자가 캡처할
때마다 `Key Time`이나 `Duration`을 손으로 맞출 필요가 없다. 간편 경로는 자동으로
`CATMULL_ROM / LINEAR`를 사용하고 `Play Captured Path`가 저장 전 draft를 0초부터 제품 sampler로
재생한다. 재생이 끝난 뒤 다시 카메라를 움직이려면 `Release Camera / Keep Time`을 누른다.

LookAt은 기본적으로 캡처 당시 카메라가 보는 방향을 저장한다. 특정 물체를 계속 바라보게 하려면 기존
`Show LookAt Dummy`를 켜고 Dummy를 목표 위치에 둔 뒤 각 지점을 캡처한다. 세밀한 하드 컷, 중간 장면
시간, 구간별 속도는 아래의 기존 `Camera Cut List`, `Saved Scenes`, `Apply Segment Speed`에서 추가
편집할 수 있다.

### 간편 경로 추가 검증

| 검증 | 결과 |
|---|---|
| 간편 캡처 UI/상태/상한 source contract | PASS |
| GameplayBalance `Validate` | PASS (`54 patterns / 230 stages`) |
| ActionPresentationTimelineHarness Debug build/run | PASS |
| ActionPresentationTimelineHarness Release build/run | PASS |
| Client x64 Debug compile/link | PASS |
| Client x64 Release build | PASS |
| focused `git diff --check` | PASS |

Client 화면에서의 실제 조작감과 최종 경로 framing은 사용자 육안 확인 대상이다. 에이전트는 Client를
실행하거나 visual PASS로 대신 판정하지 않았다.
