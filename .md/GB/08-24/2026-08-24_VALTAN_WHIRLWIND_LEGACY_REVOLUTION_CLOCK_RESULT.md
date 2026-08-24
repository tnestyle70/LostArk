# 2026-08-24 Valtan Whirlwind legacy Revolution clock 회귀 수정 결과

## 1. 결론

Valtan Whirlwind Mesh Particle의 Revolution이 약 `21.67도`에서 멈추던 회귀를 수정했다.
원인은 Server animation이나 Valtan pattern이 아니라 공용 `CEffectPlayback`의 optional
Motion Duration fallback이었다.

| 구분 | 판정 |
|---|---|
| 구현 | 완료 |
| EffectRenderContractHarness Debug build/run | PASS, bindings `171/171` |
| Client x64 Debug compile/link | PASS, 오류 `0` |
| `git diff --check` | PASS |
| Valtan animation/pattern/data 변경 | 없음 |
| 사용자 visual fidelity | `PENDING_USER_VISUAL_GATE` |
| PR/merge | 검증 커밋 뒤 수행 |

## 2. 원인과 수정 계약

Whirlwind의 수동 Mesh Particle은 다음 시간을 사용한다.

```text
Element Life     0.0166667 s
Particle Life    1.05 s
Revolution Y     1300 deg/s
Motion Duration  0 (optional field omitted)
```

Motion/Hold 도입 전에는 Element Life가 position/rotation/scale lerp의 정규화만 소유했고,
velocity와 Revolution은 제한하지 않은 local time으로 계속 누적됐다. 도입 뒤 optional `0`을
Element Life로 바꾼 다음 `min(localTime, Element Life)`를 모든 root transform에 적용하면서
Revolution도 첫 프레임 뒤 정지했다.

`Evaluate_ElementWorld`는 이제 다음처럼 분기한다.

- `Motion Duration > 0`: 명시 시점에서 root transform을 고정한다.
- `Motion Duration == 0`: legacy 문서 동작을 보존한다.
  - velocity/Revolution은 local time 동안 계속 누적한다.
  - authored lerp는 Element Life로 정규화하고 end 값에서 유지한다.

따라서 Sky Axe처럼 Motion/Hold를 명시한 Mesh의 정지 동작은 그대로이고, Whirlwind처럼 optional
값이 없던 기존 Mesh Particle만 도입 전 회전 동작으로 돌아간다.

## 3. 회귀 테스트

기존 transform-motion hold 계약 테스트에 Whirlwind와 같은 수명 구조를 추가했다.

```text
Element Life     1 / 60 s
Particle Life    1.05 s
Local Space      true
Initial Position (1, 0, 0)
Revolution Y     1300 deg/s
Motion Duration  0
Samples          0.1 s, 0.2 s
```

다른 위치·속도·가속도는 모두 0으로 두므로 두 world-position 표본의 차이는 Element root
Revolution에서만 발생한다. 두 표본은 각각 `130도`, `260도` Y 회전의 예상 좌표와 수치로
비교한다. 이전 회귀 구현은 두 표본이 Element Life 위치에 고정되어 실패하고, 수정 구현은
정확한 좌표와 일치해 통과한다. 같은 테스트 함수의 explicit Motion Duration 표본은 motion
종료 뒤 standalone Mesh와 local-space Mesh Particle root가 고정되는 것도 계속 검증한다.

## 4. 자동 검증

```text
EffectRenderContractHarness Debug build  PASS
EffectRenderContractHarness Debug run    PASS
  expectedBindingCount                   171
  actualBindingCount                     171
  transform-motion-hold                  complete

Client x64 Debug build                   PASS
  initial full-build warnings             2920 (기존 SDK 인코딩/PDB 경고)
  final incremental warnings              248 (기존 경고)
  errors                                 0
  initial full-build elapsed              00:06:38.81
  final incremental elapsed               00:02:23.44

git diff --check                         PASS
```

분리 worktree의 Client/harness 빌드는 base와 같은 정본 Debug EngineSDK/Engine/Shared dependency
산출물을 사용하고 변경된 Client 소스는 해당 worktree에서 새로 컴파일·링크했다.

## 5. 사용자 수동 검증

merge 뒤 정본 Client 실행 파일을 갱신한 다음 Effect Tool에서 Valtan Whirlwind Effect를 열어
Mesh Particle이 Particle Life 동안 계속 회전하는지 사용자가 직접 판정해야 한다. 에이전트는
Client를 실행하거나 visual PASS를 대신 기록하지 않았다.
