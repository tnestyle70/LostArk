# 2026-08-24 Effect Element 재사용·Motion/Hold·Sky Axe 시간 확장 결과

## 1. 결론과 현재 상태

Effect Detail에서 Mesh의 이동·회전 시간과 전체 생존 시간을 분리했다. `Element Life`를
늘려도 기존 이동 속도가 같이 느려지는 문제는 새 optional `Transform Motion Duration`이
root transform clock만 소유하도록 해결했다. Motion이 끝나면 Mesh는 마지막 위치와 회전에
고정되고, Hold 동안 가시성·색·UV·dissolve clock은 계속 진행한다.

Data Files의 Saved Skill Effects는 Effect 전체 행 아래에 Effect Tool과 같은
`Effect -> Family -> Element` 트리를 표시한다. 선택한 Element 하나는
`Load Saved Element for Editing`으로 현재 NEW/AUTHORED Effect에 unsaved 복사본으로만
추가된다. 원본 Effect 전체를 여는 기존 버튼과 동작은 그대로 유지했다.

Valtan HIGH_JUMP는 도끼와 빨간 원을 저작하는 AIRBORNE/WORLD 구간을 8초로 늘렸다.
플레이어별 target axe는 Server hit 1.2초를 유지하고 WORLD owner로 AIRBORNE 전체 8초간
생존한다. LAND의 실제 착지 stage는 3.2초 그대로이며, 도끼 Mesh 자체가 보이고 박혀 있는
시간은 Effect Detail의 Motion/Life가 소유한다.

| 구분 | 판정 |
|---|---|
| 구현 | 완료 |
| focused test | PASS |
| Effect/Gameplay publisher Validate | PASS |
| Effect 실행형 harness | PASS |
| Server Debug build/contract | PASS, `failures : 0` |
| Client Debug compile/link | PASS |
| 사용자 visual fidelity | `PENDING_USER_VISUAL_GATE` |
| PR/merge/main 동기화 | 결과 기록 대기 |

## 2. Transform Motion과 Hold 분리

`EFFECT_TIMING_DESC`에 optional `fTransformMotionDurationSeconds`를 추가했다.

```json
"timing": {
  "startDelaySeconds": 0.96,
  "lifeTimeSeconds": 3.24,
  "transformMotionDurationSeconds": 0.24,
  "afterImageSeconds": 0,
  "dissolveStartNormalized": 1
}
```

- 값이 없거나 `0`이면 기존처럼 Life가 transform clock도 소유한다.
- 명시값은 `0 < Motion Duration <= Life`만 허용한다.
- standalone Mesh와 Mesh Particle root의 position, rotation, scale, velocity,
  revolution만 Motion 종료 시 고정한다.
- 개별 Particle의 Initial Velocity/Acceleration은 particle life clock을 계속 사용한다.
- serializer는 `0`을 생략하므로 기존 authored 문서 identity와 동작을 바꾸지 않는다.
- C++ codec과 Effect publisher가 같은 finite/range/carrier 규칙을 검증한다.

Effect Detail에는 다음 항목을 추가했다.

```text
Separate Transform Motion From Life
Motion Duration
Hold After Motion
Motion End
```

분리를 처음 켜면 Motion Duration은 현재 Life를 복사한다. 그 상태에서 Hold만 늘리면
`Life = Motion + Hold`가 되므로 보이던 이동 속도는 유지된다.

## 3. Saved Effect의 Element 단위 재사용

Data Files refresh는 AUTHORED 문서를 read-only cache로 parse한 뒤 Saved Skill Effects에
다음 구조를 투영한다.

```text
Saved Skill Effects
└─ Effect
   ├─ Mesh Particle
   │  ├─ Mesh Particle 01
   │  └─ Mesh Particle 02
   └─ Sprite Particle
      ├─ Sprite Particle 01
      └─ Sprite Particle 02
```

Element 복사는 다음 transaction을 사용한다. Effect 전체를 열거나 Save/Create/Promote로
다른 asset을 선택하는 모든 경로는 이전 saved Element 선택도 함께 지워, 우연히 같은 Element
ID를 가진 다른 Effect가 암묵적으로 선택되지 않게 한다.

1. 현재 문서가 NEW/AUTHORED이고 unapplied Detail draft가 없는지 확인한다.
2. 클릭 시 원본 파일을 다시 parse하고 Effect ID와 stable Element ID 하나를 확인한다.
3. source와 current Effect의 Particle System 전역 multiplier 4개가 다르면 fail-close한다.
4. `Build_GenericAuthoredElementStartingCopy`로 WModel/DDS/material/detail/timing을 보존하고
   source ownership, recipe, attachment, inheritance, presentation을 제거한다.
5. `authored.copy.<source-id>.<N>` 유일 ID를 발급해 staged current document에 append한다.
6. `Try_CommitDocument`가 성공한 뒤에만 current document를 교체하고 새 Element를 선택한다.

이 경로는 원본 Effect를 열거나 저장하지 않으며 current Effect도 자동 저장하지 않는다.
따라서 3연격 바닥 Sprite Particle 두 개를 하나씩 선택해 Sky Axe Effect에 복사한 뒤
직접 위치·시간·색을 튜닝할 수 있다.

## 4. HIGH_JUMP 서버와 WORLD 시간 계약

적용 값은 다음과 같다.

| 항목 | 값 |
|---|---:|
| TAKEOFF | 1933 ms |
| AIRBORNE / WORLD owner | 8000 ms |
| LAND | 3200 ms |
| RECOVERY | 400 ms |
| target axe hit | 1200 ms, 1회 |
| target axe WORLD owner life | 8000 ms |

AIRBORNE ENTER의 기존 `LOCKED_TARGET_PER_ALIVE_PLAYER` 도끼 1회 생성은 유지했다. Server
owner를 8초 유지해도 damage pulse는 1.2초에 한 번뿐이며, Mesh Element는 자체 Life가
끝나면 Server owner보다 먼저 사라질 수 있다.

`[WORLD]` Effect를 연 뒤 첫 Element를 생성하는 authoring transaction도 owner duration을
snapshot/restore한다. preview stage나 atomic save가 실패할 때뿐 아니라 성공했을 때도 8초
timeline이 Effect 자체 길이로 축소되지 않는다.
반복 투척 schedule과 arena-center 공용 원은 이번 변경에 넣지 않았다. 플레이어별 WORLD
Effect에 공용 원을 넣으면 생존 플레이어 수만큼 겹쳐 그려지므로 별도 Server-owned world
visual owner가 필요하기 때문이다.

Pattern 행의 `Play Server Pattern`은 이번 변경 전에 이미 정본에 구현되어 있다. 이번 데이터는
그 버튼으로 HIGH_JUMP를 실제 Server fixed-tick 경로에서 재생할 때 소비된다.

## 5. 자동 검증

| 검증 | 결과 |
|---|---|
| `test_effect_tool_saved_element_clone.py` | 8 tests PASS |
| `test_effect_tool_valtan_saved_rows.py` | 22 tests PASS |
| Effect publisher `-Mode Validate` | 163 Effects / 171 bindings PASS |
| Gameplay publisher `-Mode Validate` | 34 patterns / 131 stages PASS |
| 변경 JSON 3개 parse | PASS |
| Server x64 Debug build | PASS |
| `Server.exe --contract-test` | PASS, `failures : 0` |
| EffectRenderContractHarness Debug build/run | PASS, bindings `171/171` |
| Engine x64 Debug + `UpdateLib.bat Debug` | PASS |
| Client x64 Debug compile/link | PASS |
| `git diff --check` | PASS |

Effect publisher는 분리 worktree에 Git 비관리 Resources가 없으므로 정본
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources`를 명시해 검증했다. resource fallback을
추가한 것은 아니다.

## 6. 사용자 수동 검증

에이전트는 Client/UI를 실행하거나 visual PASS를 대신 판정하지 않는다. merge와 정본 동기화
뒤 사용자가 다음을 직접 확인해야 한다.

1. `F1 > Effect Tool > Data Files > Saved Skill Effects`에서 3연격 Effect를 펼쳐
   Family와 각 Element가 보이는지 확인한다.
2. Sky Axe Effect를 current Effect로 연 상태에서 바닥 Element를 선택하고
   `Load Saved Element for Editing`을 눌러 Element 하나만 append되는지 확인한다.
3. 도끼 Mesh에서 먼저 `Separate Transform Motion From Life`를 켜고 Hold만 늘렸을 때
   낙하 속도는 그대로이고 바닥 위치에서 정지해 있는지 확인한다.
4. `All Effects > Valtan > VALTAN_HIGH_JUMP`의 `[WORLD]` timeline이 8초까지 열리는지
   확인한다.
5. HIGH_JUMP Pattern 행의 `Play Server Pattern`으로 실제 Server pattern을 재생해
   player별 도끼, 1.2초 hit와 planted visual 시간을 최종 판정한다.
