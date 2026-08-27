# Animation Tool 발탄 presentation 동일 재생 구현 계획

## 1. 목표

Character Select/Development의 로컬 발탄 Preview에서 Animation Tool이 자체 단일-clip
sampler로 모델을 직접 재생하지 않고, Arena의 replicated `CValtan`이 Product
`Valtan.patternbindings.json`을 읽어 Server action wall clock을 clip source clock으로 바꾸는
동일 함수를 사용해 애니메이션만 재생한다.

## 2. 범위

- 기존 `Valtan.presentation.json -> Valtan.patternbindings.json` Product projection과
  `CActionPresentationTimeline`을 그대로 소비한다.
- `CValtan`의 pattern binding resolve, occurrence 전환, play rate, loop, source seek 코드를
  한 함수로 추출해 Server snapshot과 local authoring preview가 함께 호출한다.
- Animation Tool은 선택 pattern의 stage/action wall age만 공급한다.
- local preview의 기존 root-motion 표시 정책은 유지한다. Arena의 월드 이동은 Server transform
  권위이므로 animation sampler 공용화와 섞지 않는다.
- Server 접속, boss spawn/despawn, Effect, Sound, hit/damage, navigation과 world transform은
  실행하지 않는다.

## 3. 검증

1. Animation Tool focused contract test
2. Action presentation timeline harness
3. Valtan pattern tree contract와 V2 master validation
4. Client x64 Debug build
5. 사용자 육안: `VALTAN_DASH_CHARGE`, multi-occurrence/hold 패턴, animation-first P2 행을
   Character Select local Valtan에서 재생해 Arena clip 전환과 wall timing을 비교

월드 위치 이동은 계속 Server transform 권위다. 이번 결과의 PASS 범위는 clip, occurrence,
source seek, play rate와 loop/hold다.
