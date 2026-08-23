# 2026-08-23 Valtan 3연공격·회전공격 Server 분리와 Screen Presentation 결과

## 1. 결론

구현과 자동 검증은 완료했다. 기존 `VALTAN_FOUR_SLASH` 한 행에서 약 8초 동안
연속 재생하던 두 clip을 Server 권위 pattern 둘로 분리했고, All Effects도 같은
정본을 읽어 `3연공격`과 `회전공격`을 별도 행으로 표시한다.

- `VALTAN_TRIPLE_SLASH`
  - active clip: `mesh_att_battle_10_01`
  - active duration: `3500 ms`
  - Server hit: `1791 / 2300 / 3138 ms`
- `VALTAN_ROTATION_SLASH`
  - active clip: `mesh_att_battle_10_02`
  - active duration: `3167 ms`
  - Server hit: `600 / 1800 / 3000 ms`

사용자의 Client 육안 판정은 아직 수행하지 않았다. 화면 PASS는 사용자가 아래 절차로
직접 확인한 뒤에만 확정한다.

## 2. 실제 변경

### 2.1 Server 권위 pattern과 판정

`ValtanEncounter.json`의 기존 결합 pattern을 `VALTAN_TRIPLE_SLASH`와
`VALTAN_ROTATION_SLASH`로 나눴다. 선택 weight는 기존 12를 6+6으로 보존했다.

비균등 hit를 위해 optional `hitOffsetsMs`를 authoring, publisher, bootstrap, C++
catalog loader, `CValtanBrain`까지 전달했다. 배열이 없으면 기존
`hitDelayMs + index * hitIntervalMs` 경로를 그대로 사용한다. explicit 배열의 count,
strict ordering, stage duration, legacy 필드 혼용은 모두 fail-closed한다.

새 `PATTERNSTAGEHITOFFSET` row kind에 맞춰 gameplay bootstrap은 version 17로
publisher와 C++ loader를 함께 올렸고 version 16은 거부한다. gameplay boundary는
float 초 누적을 사용하지 않고 30 Hz integer stage clock으로 비교한다. 따라서
3연공격의 세 판정은 active tick `54 / 69 / 95`, 3500 ms 종료는 tick `105`에서,
회전공격은 `18 / 54 / 90`, 3167 ms 종료는 ceiling tick `96`에서 평가된다.

두 새 pattern은 이름 분기 없이 `iSourcePrimaryActionId`를 공용 cooldown family로
사용한다. 어느 하나가 시작돼도 원본 action `420609`의 10초 cooldown entry는 하나만
생기고, 다른 source action은 독립적이며 source action이 없는 synthetic pattern은 기존
stable pattern ID별 fallback을 유지한다.

### 2.2 animation·cue·All Effects

pattern binding은 다음처럼 하나씩만 소유한다.

- triple active → `mesh_att_battle_10_01`, loop `false`
- rotation active → `mesh_att_battle_10_02`, loop `false`

기존 effect asset ID와 stable clip occurrence ID는 보존하고 pattern/stage/action owner만
세 개의 명시적 successor transfer로 옮겼다. 역사적 `Valtan.actionbindings.json`은 새
제품 projection으로 덮어쓰지 못하게 보호했다.

Client의 Encounter reference, Valtan debug view, Pattern Tree와 Effect Tool이 explicit hit
offset을 읽는다. Pattern Tree parser는 필수 ID, stage kind, hit shape, timing,
finite scalar가 누락되면 기본값으로 통과시키지 않고 실패한다.

### 2.3 clip-01 마지막 ScreenPost

원본 notify의 ordered stable occurrence 두 개를 기존 typed ScreenPost renderer로 붙였다.
DDS 사각형이나 전용 Valtan renderer를 만들지 않았다.

1. `occurrence.c627ba06ba0a8d5d48086907`
   - FilmNoise
   - source time `3.144474983215332 s`
   - `PROJECT_TUNED_APPROX`
2. `occurrence.14794cdb89c73ee33f1dead3`
   - ZoomBlur
   - source time `3.144474983215332 s`
   - `BOUNDED_RECONSTRUCTED`

두 runtime pulse는 각각 `0.35 s`로 제한해 active edge `3.5 s` 안에서 끝난다. 원본 recipe의
1초 lifetime evidence는 source recipe에 그대로 보존했다.

Carrier materializer가 만든 기존 12행과 ScreenPost를 최초로 붙인 14행은 역사적
preimage/successor 증거로 유지했다. ScreenPost를 materializer 출력으로 허위 귀속하지
않고 별도 successor overlay proof로 봉인했다.

현재 authored JSON 전체를 14행으로 잠그지는 않는다. 그렇게 하면 Effect Tool에서 사용자가
Source 행을 숨기거나 삭제하고 수동 Mesh/Sprite/Decal을 추가한 정상 저장까지 다음 publish가
거부하기 때문이다. live migration check는 현재 문서를 직접 읽되 다음 경계를 사용한다.

- receipt의 역사적 12행 preimage와 최초 14행 successor hash는 변경하지 않음
- typed ScreenPost 두 stable ID만 유일성·상대 순서·runtime/identity semantic projection으로
  exact seal하고 모든 runtime float는 IEEE-754 f32 bit로 비교
- 나머지 non-ScreenPost authored 행은 Effect 문서 validator 범위 안에서 저장·삭제·튜닝 허용

C++ codec의 정상 저장은 `3.144474983215332 → 3.14447498`, `0.08 → 0.0799999982`처럼
JSON 표기를 바꾸고 disabled default object를 축약한다. raw JSON hash를 live 계약으로 사용하지
않으며, 이 codec normalization은 PASS하고 한 f32 bit의 실제 값 변화는 FAIL하는 회귀를 추가했다.

정본 폴더의 최신 수동 저작 non-ScreenPost 6행 전체와 feature의 ScreenPost 두 행을
의미적으로 합성했다. `valtan.clip01.weapon-slash.01` 계열과 최종 저장된
`impact.fragments.hit_007`을 포함한 수동 배열 순서는 그대로이며, 제품 runtime sidecar도
같은 8행이다. migration overlay와 focused ScreenPost admission은 이 문서로 PASS했다.

### 2.4 action edge의 natural tail

분리 전 6.667초 active 구간에서는 남아 있던 particle tail 일부가, 분리 후 3.5초 action
edge에서 잘리는 결함을 수정했다.

- 아직 queue에만 있는 이전 action cue: 취소
- 재생 중 `CUE_END`: action edge에서 종료
- 재생 중 `NATURAL`: document가 실제로 끝날 때까지 유지
- boss death/despawn/level teardown: `Stop_BossOwner`가 전부 강제 종료

이 정책은 effect/pattern ID를 검사하지 않는 공용 `EFFECT_STOP_POLICY` 처리다. 따라서
All Effects의 natural preview와 Valtan arena가 같은 tail 계약을 사용한다.

## 3. 자동 검증

- focused Python suite: `51 tests`, PASS
- owner reseal check: `RESEALED_SUCCESSOR`, changed `0`, transfers `3`
- occurrence migration check: `131 bindings / 137 clips / 44 cues`, PASS
- Effect Tool Valtan saved rows: `15 tests`, PASS
- ScreenPost 및 natural-tail contract: `13 tests`, PASS
- 두 전용 module은 표준 `Test-EffectPipeline.ps1` 회귀 목록에 등록
- Gameplay balance Validate:
  - `34 boss patterns / 131 stages / 67 Debug audition occurrences`, PASS
- Effect Validate:
  - `156 Effects / 171 material-program bindings / 5 registry-bound auditions`, PASS
- boss pattern Effect join: PASS
- Valtan world destruction Validate: PASS
- Server Debug/Release `--contract-test`: failures `0`
- actual 30 Hz Server hit consumer:
  - triple contact tick `54 / 69 / 95`, active end `105`, extra pulse `0`
  - rotation contact tick `18 / 54 / 90`, active end `96`, extra pulse `0`
  - shared source-action cooldown entry `1`, 다른 source 독립
- Client x64 Debug/Release build: PASS
- modified JSON parse 및 `git diff --check`: PASS

검증 산출물 SHA-256:

- Debug Client.exe:
  `3dfccc07099209f76bca0920adf280bac9c80e9e0184c0f9d84c8a706adc3165`
- Release Client.exe:
  `d702586d09ab5af43c957c14df7c49d9820bf254d38a1e15ee0c69c896a99a54`
- Debug Server.exe:
  `2a5009889fc3bf088a9a90375142304bdc44cbd06c7653cee8490babccd07713`
- Release Server.exe:
  `5b0701d565228db3315df778d15530c1d2323ec5d2969af7f030b4c512df9739`
- runtime Effect catalog:
  `a8903bc7d3b374b4df801f2051197e5bcf7e6abb6b44e91a91671adaf4669b6b`
- clip-01 runtime authored sidecar:
  `20e260c63ff51a9bbb75015abd455798ae52e92128fa00efb51c205ae5bafd24`

## 4. 수동 검증 절차

최종 main이 정본 물리 폴더에 반영된 뒤 `Framework.sln`에서 `Server + Client` profile을
`Ctrl+F5`로 실행한다.

1. Lobby에서 Valtan 진입
2. `F1`
3. Effect Tool → All Effects → Valtan
4. `3연공격`을 열어 `Play Full Effect` 또는 해당 saved effect 재생
5. `회전공격`을 별도로 열어 재생

확인할 항목:

- 두 pattern이 서로 다른 행으로 보이고 약 8초 결합 재생이 사라졌는가
- 3연공격 animation이 3.5초이며 세 타격이 `1791/2300/3138 ms` 순서로 맞는가
- 마지막 타격 직후 FilmNoise → ZoomBlur가 약 0.35초 보이는가
- 3.5초 이후 기존 particle의 natural tail이 잘리지 않고 자연 종료하는가
- 회전공격 animation이 3.167초이며 세 판정이 `600/1800/3000 ms`에 맞는가

## 5. 남은 fidelity 경계

- hit/clip/notify 시간과 occurrence/material identity는 source evidence에 맞췄다.
- 현재 Server hit geometry는 기존 project-tuned 공용 shape를 유지한다. 타격별 원본 proxy
  형상을 새로 source-exact 복원한 변경은 아니다.
- FilmNoise의 visible scalar는 원본 ABI가 없어 project-tuned다.
- ZoomBlur 계산은 기존 bounded reconstructed adapter다.
- 원본 ViewShake는 `3.1 s`, duration `0.4 s` evidence까지 확인했지만 일반 Effect용 typed
  camera-shake channel과 scalar ABI가 없어 이번 실행 경로에는 넣지 않았다. cinematic
  camera override나 ScreenPost로 가장하지 않았다.
- 사용자가 Effect Tool로 저장한 clip-01 JSON은 제품 입력이다. 정본 폴더의 동시 작업을
  자동 생성본으로 덮지 않고, 저장된 non-ScreenPost 행을 그대로 둔 채 ScreenPost 두 stable
  occurrence만 의미 단위로 병합한다. receipt는 역사적 proof라서 수동 행 변경에 맞춰 다시
  봉인하지 않는다.
