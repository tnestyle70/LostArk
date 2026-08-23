# 2026-08-23 Valtan 3연공격·회전공격 Server 분리와 Screen Presentation 계획

## 1. 목적

기존 `VALTAN_FOUR_SLASH` 한 패턴에 연속 재생되던 `mesh_att_battle_10_01`과
`mesh_att_battle_10_02`를 실제 Server 권위 패턴 둘로 분리한다. All Effects도 같은
Server pattern/action/stage/clip 정본을 읽어 `3연공격`과 `회전공격`을 별도 행으로
표시한다.

clip-01 마지막 타격에는 원본 notify가 가진 FilmNoise와 ZoomBlur Screen Post를
stable occurrence로 연결한다. 화면 DDS를 사각형으로 까는 별도 renderer는 만들지
않고 기존 typed Screen Post renderer를 사용한다.

## 2. 원본 근거와 fidelity 경계

- clip-01 `mesh_att_battle_10_01`
  - source clip duration: `3500 ms`
  - Server hit: `1791 / 2300 / 3138 ms`
  - 마지막 impact: `3134 ms`
  - FilmNoise 후 ZoomBlur: 둘 다 `3144.474983 ms`
  - ViewShake: `3100 ms`, duration `400 ms`
- clip-02 `mesh_att_battle_10_02`
  - source clip duration: `3167 ms`
  - Server hit: `600 / 1800 / 3000 ms`

hit/clip/notify 시간과 occurrence/material identity는 `SOURCE_EXACT` 근거다. 현재
ZoomBlur 셰이더 계산은 기존 `BOUNDED_RECONSTRUCTED` adapter이고 FilmNoise scalar와
카메라 진폭·주파수는 원본 ABI 근거가 없어 `SOURCE_EXACT`가 아니다. 근거가 없는
값을 exact로 표시하거나 cinematic camera override를 일반 Effect로 재사용하지 않는다.

## 3. 구현 계약

1. Encounter에 `VALTAN_TRIPLE_SLASH`와 `VALTAN_ROTATION_SLASH`를 추가하고 기존
   `VALTAN_FOUR_SLASH`를 제품 선택 경로에서 제거한다.
2. 비균등 hit를 위한 optional `hitOffsetsMs`를 authoring → publisher bootstrap →
   Server catalog → `CValtanBrain`까지 수직으로 전달한다. 배열이 없으면 기존
   `hitDelayMs + index * hitIntervalMs` 계약을 그대로 유지한다.
   gameplay bootstrap에 새 row kind가 생기므로 publisher와 C++ loader version을
   함께 올리고 이전 bootstrap을 거부한다. 판정 시간은 float 누적이 아니라 30 Hz
   integer tick의 rational 비교로 계산한다.
3. pattern binding은 clip-01과 clip-02를 각 action에 하나씩 연결하고 loop를 끈다.
   Server active stage도 각각 `3500 ms`, `3167 ms`로 제한해 기존 약 8초 결합 재생을
   제거한다.
   Product cue의 effect asset과 stable clip occurrence는 보존하되 pattern/stage/action
   owner만 새 정본으로 교체한다.
4. rotation, debug audition, wall-contact allowlist와 Server contract test를 둘로
   교체한다. 기존 selection weight 12는 6+6으로 보존한다.
   두 pattern은 같은 원본 `sourceActionId`의 cooldown family를 공유하고 다른 source
   action 및 source 없는 synthetic pattern의 기존 동작은 유지한다.
5. All Effects saved-row harness에서 두 pattern의 독립 clip/effect 소유권과 옛 pattern
   부재를 검증한다.
6. clip-01 Screen Post는 exact occurrence/order/time을 보존한다. ZoomBlur는 기존 typed
   adapter로 bounded 연결하고, FilmNoise는 보이는 값을 넣을 경우에만
   `PROJECT_TUNED_APPROX`로 명시한다. clip-02 Screen Post를 섞지 않는다.
7. Camera는 일반 Effect runtime channel과 원본 scalar ABI가 모두 닫힌 경우에만 슬롯에
   승격한다. 이번 변경에서 그 계약을 닫지 못하면 원본 ViewShake evidence는 보존하되
   실행 가능한 것처럼 노출하지 않고 RESULT에 별도 blocker로 기록한다.
8. action edge에서 아직 queue에만 있는 cue는 취소한다. 이미 재생 중인 cue는
   `stopPolicy`를 소비해 `CUE_END`만 즉시 종료하고 `NATURAL`은 문서의 실제 끝까지
   유지한다. 특정 pattern/effect ID 분기를 만들지 않으며 death/despawn/level teardown의
   `Stop_BossOwner`는 계속 모든 tail을 강제 종료한다.

## 4. 실패 계약

- `hitOffsetsMs`는 hitCount와 길이가 같고 strictly increasing이며 마지막 값이 stage
  duration보다 작아야 한다.
- explicit schedule과 legacy interval/delay를 동시에 쓰면 publisher와 Server loader가
  fail-closed한다.
- source millisecond threshold와 stage duration은 30 Hz ceiling tick에서 판정하며
  float 누적 오차나 server tick wrap으로 한 tick 밀리지 않아야 한다.
- pattern/action/clip/cue join이 하나라도 맞지 않으면 Product publish가 실패한다.
- Screen Post occurrence/profile이 admission predicate와 맞지 않으면 조용히 다른
  profile로 fallback하지 않는다.
- Carrier materializer의 12행 receipt를 ScreenPost 산출물로 다시 쓰지 않는다. 기존
  12행과 최초 14행 successor는 역사적 proof로 보존하고, live authored 문서는 두 protected
  ScreenPost stable ID의 순서·runtime/identity semantic projection만 exact seal한다. float는
  C++ codec round-trip과 같은 IEEE-754 f32 bit로 비교한다. 나머지 non-ScreenPost 행은 Effect
  Tool의 수동 저장·삭제·튜닝을 허용하고 publisher가 일반 문서 계약으로 검증한다.
- `NATURAL` tail을 action edge에서 지우거나, 반대로 `CUE_END` cue를 남기면 실패다.

## 5. 검증

- gameplay balance Validate/publish 및 balance provenance 동기화
- Effect publisher Validate/publish
- Server contract test Debug/Release
- Effect Tool Valtan saved-row focused harness
- 새 explicit hit schedule의 정상/중복/역순/count mismatch/legacy 보존 테스트
- Debug/Release Engine, UpdateLib, Server, Client build
- 사용자는 `Server + Client`로 Valtan 진입 후 F1 → All Effects에서 두 pattern을 각각
  열고 재생한다. 최종 visual PASS는 사용자만 판정한다.
