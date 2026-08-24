# 2026-08-24 Valtan Whirlwind legacy Revolution clock 회귀 수정 계획

branch: `codex/valtan-whirlwind-revolution-legacy-clock`

base: `origin/main@1d6b63b1`

## G01. Motion Duration 미지정 문서의 legacy transform clock 복원

### 원인 실측

`transformMotionDurationSeconds`가 도입되기 전 `Evaluate_ElementWorld`는 다음 두 시계를
구분했다.

- position/rotation/scale lerp 정규화: `Clamp01(localTime / Element Life)`
- velocity/revolution 누적: 제한하지 않은 `localTime`

Motion/Hold 구현은 optional 값 `0`을 `Element Life`로 대체한 뒤 모든 transform 시간을
`min(localTime, Element Life)`로 잘랐다. 따라서 Element Life가 `1 / 60초`이고 Particle
Life가 `1.05초`인 Valtan Whirlwind Mesh Particle은 `1300 deg/s` Revolution이 약
`21.67도`만 진행한 뒤 고정됐다.

### 목표 계약

- `Motion Duration > 0`: 기존 Motion/Hold 기능대로 root transform을 명시 시점에 고정한다.
- `Motion Duration == 0`: 도입 전 legacy 동작을 그대로 유지한다.
  - velocity/revolution은 제한하지 않은 local time으로 누적한다.
  - position/rotation/scale lerp는 Element Life로 정규화하고 Life 뒤에는 end 값으로 유지한다.
- Element Life, Particle Life, visibility와 Valtan animation/pattern 계약은 변경하지 않는다.

### 수정 파일

- `Client/Private/Effect_Playback.cpp`
  - explicit Motion Duration의 존재 여부를 분기한다.
  - explicit 값에만 clamped motion time을 사용한다.
- `Client/Public/Effect_AuthoringDocument.h`, `Client/Private/Effect_DocumentCodec.cpp`
  - optional `0`의 legacy lerp와 velocity/Revolution 시계 계약을 주석에 정확히 기록한다.
- `Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp`
  - Element Life보다 오래 생존하는 local-space Mesh Particle을 구성한다.
  - optional duration `0` 상태에서 두 Life 이후 표본이 정확한 Y Revolution 예상 좌표와
    일치하는지 검증한다.
  - explicit Motion Duration의 기존 hold 검증은 유지한다.

## 검증

1. EffectRenderContractHarness Debug build/run
2. Client x64 Debug build
3. `git diff --check`

## 사용자 수동 검증

merge와 정본 실행 파일 갱신 뒤 Valtan Whirlwind Effect를 열어 Mesh Particle이 Particle Life
동안 계속 회전하는지 사용자가 직접 판정한다. 에이전트는 Client를 실행하거나 visual PASS를
대신 기록하지 않는다.
