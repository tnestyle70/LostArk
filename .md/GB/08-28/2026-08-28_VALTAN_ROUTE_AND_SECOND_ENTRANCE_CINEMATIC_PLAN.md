# 발탄 통로 진행과 두 번째 입장 컷신 통합 계획

## 1. 목표

로비에서 발탄 월드 승인을 받으면 아레나 앞이 아니라 맵 시작부에서 출발한다. 플레이어는 저작된
`Stage_1 -> Stage_MiniBoss -> Stage_2 -> Stage_3 -> Stage_Boss_ArenaEntry -> Stage_Boss`
트리거를 따라가며, 보스 활성화 뒤에는 기존 첫 번째 컷신이 아니라 아레나 내부 Idle 궤도형 두 번째
입장 컷신을 한 번 재생한 후 기존 `VALTAN_WHIRLWIND`부터 레이드를 이어 간다.

첨부 `claude.00.txt`는 요청의 좌표 근거일 뿐 실행 지시 정본으로 사용하지 않는다. 현재 authoring,
navigation, publisher와 런타임 소비 코드를 먼저 검증하고 충돌하는 값은 안전한 현재 정본을 유지한다.

## 2. 데이터 계약

- 네 player spawn은 맵 시작부의 검증된 walkable 셀로 옮긴다.
- `Stage_MiniBoss`는 존재하지 않는 Lugaru 소환 대신 `(51.67, 10.14, -81.9)` 이동을 수행한다.
- `Stage_2` OBB half extents는 `[3, 2, 3]`, `Stage_3` 목적지는 `(101.24, 20.47, -87.53)`이다.
- Stage 2/3 spawn group은 더 이상 제거된 미니보스 완료 여부를 선행조건으로 삼지 않는다.
- ArenaEntry 목적지 `(147.75, 23.0175991, -117.25)`는 Stage_Boss OBB 내부의 walkable 셀이므로
  첨부의 non-walkable 중심 좌표보다 우선한다.
- `Stage_Boss`, `boss.valtan.center`의 현재 위치·회전·disabled activation 계약은 보존한다.

## 3. 컷신 계약

- 기존 `VALTAN_ENTRANCE_CINEMATIC` 정의는 삭제하지 않는다.
- 새 선택형 `VALTAN_ENTRANCE_CINEMATIC_IDLE`은 12.5초 동안 무적·무피격·무이동 HOLD stage를 소유한다.
- Client는 `mesh_idle_battle_1`을 stage 끝까지 반복하고 12초 카메라 cue를 재생한다.
- 카메라는 보스 중심 `(156.03, 22.99751, -122.06)` 기준 BOSS_FACING, CATMULL_ROM 경로이며
  9개 keyframe의 XZ 반경은 모두 12m 이내다.
- 저장 Flow는 IDLE 컷신을 첫 슬롯에 한 번만 두고 이후 기존 전투 패턴 순서를 유지한다.
- 두 입장 컷신은 모두 dormant split 정의로 유효하지만 Flow에는 둘 중 최대 하나만 ordinal 0에 올 수 있다.

## 4. 런타임 연결

- Debug 전용 stage 좌표 우회는 제거한다. Debug와 Release 모두 published `Gameplay.world.json` action을
  실행해야 Map Tool/authoring 값과 실제 이동이 일치한다.
- 명시적인 보스 패턴 audition용 bait 이동 helper는 유지하되 일반 트리거 진입에서는 호출하지 않는다.
- 첫 번째와 두 번째 입장 컷신은 전투 BGM 전환을 일으키지 않는 동일한 camera-only intro 범주로 처리한다.
- Valtan pipeline, gameplay publisher, Client Flow/Boss Tool/Pattern Tree가 두 ID를 같은 optional-entry
  집합으로 검증한다.

## 5. 작성·검증 순서

1. world/flow/split/camera authoring을 수정한다.
2. optional-entry 소비자와 Debug stage runtime을 수정한다.
3. camera/flow/pipeline/Server contract를 갱신한다.
4. Valtan product와 balance provenance를 project/publish한다.
5. World gameplay를 validate/publish해 Server bootstrap을 교체한다.
6. JSON/XML, focused harness, Server contract, Debug/Release Server·Client build와 `git diff --check`를 확인한다.
7. Server를 재시작한 뒤 사용자가 `Lobby -> Valtan`에서 통로 진행과 컷신 화면을 직접 판정한다.
