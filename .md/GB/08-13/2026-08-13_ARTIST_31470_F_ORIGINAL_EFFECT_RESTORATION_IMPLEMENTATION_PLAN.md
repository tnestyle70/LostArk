# 2026-08-13 도화가 31470 F V4 복원 구현 계획

## 목표

도화가 F의 transport draw와 source visual fidelity를 분리한다. Core33 각 occurrence가 stable
identity, material/static-set, shader equation, texture/channel, output MRT, family carrier를 명시적으로
소비하도록 만들고, 증거가 부족한 행은 generic fallback 없이 fail-closed한다.

## 구현 순서

1. stable occurrence shader registry를 35행 단일 정본으로 만든다.
2. official RefShaderCache에서 static-set, PS identity, DXBC와 uniform/resource wire를 회수한다.
3. DDS identity와 typed texture/channel 역할을 검증한다.
4. occurrence별 finite program을 Runtime Material V2 또는 Artist V4 opcode로 연결한다.
5. internal UV warp, SceneColor RT0, screen distortion pass를 서로 다른 output 역할로 유지한다.
6. source coverage 근거가 없는 opaque DDS alpha와 white/black fallback을 금지한다.
7. family/occurrence submission isolation으로 전체 schedule을 재stage하지 않고 원인을 분리한다.
8. Debug/Release build, focused contract, WARP, 60 Hz schedule sweep를 통과시킨다.
9. 사용자가 원본 `R_00/R_01`과 직접 비교해 visual fidelity를 판정한다.

## 완료 기준

- registry의 모든 35행이 exact / bounded / unresolved / forbidden 중 정확히 하나다.
- Core unresolved 행은 active/evaluated를 유지하되 material/pass/draw 전에 억제된다.
- draw-admitted 행은 registry가 지정한 opcode, texture mask, RT0/RT1 역할과 일치한다.
- `#32/#34`는 Core consumer 0이다.
- gameplay와 Effect Tool이 같은 prepared cache/document identity를 소비한다.
- 자동 검증 결과와 사용자 수동 visual 판정을 별도로 기록한다.
- native VF/pass가 닫히지 않은 semantic replay를 source-exact native 실행으로 보고하지 않는다.

현재 구현 및 검증 결과는
[2026-08-13_ARTIST_31470_F_ORIGINAL_EFFECT_RESTORATION_RESULT.md](2026-08-13_ARTIST_31470_F_ORIGINAL_EFFECT_RESTORATION_RESULT.md)에 기록한다.
