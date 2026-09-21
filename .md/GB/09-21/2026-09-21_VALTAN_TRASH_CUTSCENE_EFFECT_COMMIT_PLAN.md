# 발탄 버러지 컷신 V1 이펙트 복구 계획

## 문제

MapTool `발탄 스킬 연출(버러지)`의 667ms `effect.valtan.cinematic.trash.actor106.at667`은 처음에는 새 World-root handle이 pending 상태에서 seek되어 실패했다. 즉시 commit을 추가한 뒤에는 683ms에서 다음 검증 오류가 드러났다.

`Bounded source loops require admitted source sprite/mesh or native Cascade Ribbon emitters and EmitterLoops=0.`

해당 원본은 반복하는 sprite/mesh/ribbon emitter 7개와, 같은 Cascade source의 보조 light emitter 4개를 함께 가진다. 기존 bounded-loop 검사가 모든 visible element를 particle/ribbon이어야 한다고 판정하여 light를 잘못 거절했다.

## 변경

- MapTool의 새 World-root V1 handle은 같은 편집 프레임의 seek 전에 scoped commit한다.
- `CEffectPlayback::Set_SourceLoopEndSeconds`는 admitted sprite/mesh/ribbon 반복 emitter를 필수 주체로 유지한다.
- 같은 admitted source program의 `LIGHT`/`light` carrier는 보조 표현으로 허용한다. light만으로 loop 조건을 충족시키지는 않는다.
- 원본 effect JSON, `EmitterLoops`, `loopEffectToDuration`, track duration은 바꾸지 않는다.

## 검증

- `Effect_Playback.cpp`과 기존 MapTool/World Sequence 수정이 Debug Client compile 및 Product Build를 통과해야 한다.
- actor106의 반복 primary 7개와 supplemental light 4개가 새 계약에 맞는지 JSON으로 확인한다.
- 실제 MapTool 재생에서 667~683ms의 `World actor failed` 및 `Every actor was released`가 사라지는지는 사용자 화면에서 확인한다.