# 2026-08-21 도화가 E 붓 Trail 및 차원술사 BA Cue 미리보기 구현 계획

## 0. 문서 상태

- 문서 종류: `IMPLEMENTATION_PLAN`
- 작업 브랜치: `codex/artist-e-trail-ba-cue-0821`
- 대상:
  - 도화가 E `skillId 31480`의 붓 휘두름 구간에 붙는 흰백색 ribbon trail
  - 차원술사 LMB `skillId 2050010`의 BA1/BA2/BA3/BA4 Effect Tool 미리보기 분리
  - Effect Tool의 `Duplicate Selected` 저작 명령과 차원술사 R `skillId 2050180` helix 3회 occurrence 분리
  - 차원술사 A `skillId 2050210` voronoi/slash의 cast-root occurrence 정합과 Q `skillId 2050100` 비교 진단
- 보존 대상:
  - 현재 Product cue 매핑 `BA1/BA2/BA4 -> ba1.unified`, `BA3 -> ba3.unified`
  - PR #129 이후의 캐릭터 Effect, BA 조작감, 발탄 renderer/runtime 변경
  - 사용자가 동시에 손튜닝 중인 모든 Authored Effect JSON
- 제외 범위:
  - Client/UI 자율 실행과 visual fidelity 자동 판정
  - 임의 시간 scrub에서 과거 animation bone pose 전체를 재구축하는 offline trail history
  - animevents 텍스트를 직접 치환하는 비원자 cue writer
  - texture 파일명만으로 다른 스킬 전체에 shader profile을 확장하는 변경

작업 시작 시 사용자 소유 dirty 문서는 다음과 같이 고정한다. Q/S 문서는 수정하지 않으며, R 문서는
현재 손튜닝 내용을 보존한 채 helix occurrence에 필요한 최소 변경만 적용한다.

```text
Data/Effects/Authored/effect.dimensionmaster.skill.2050100.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050180.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050220.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050230.unified.effect.json
```

## 1. 실측 원인

### 1.1 차원술사 BA Effect 데이터는 이미 정상이다

제품 정본 `DimensionMaster.animevents`의 네 cue는 다음과 같다.

```text
BA1 clip -> effect.dimensionmaster.skill.2050010.ba1.unified
BA2 clip -> effect.dimensionmaster.skill.2050010.ba1.unified
BA3 clip -> effect.dimensionmaster.skill.2050010.ba3.unified
BA4 clip -> effect.dimensionmaster.skill.2050010.ba1.unified
```

각 cue의 `startms=0`은 스킬 전체 공통 시간이 아니라 해당 BA clip의 local time이다. Character runtime은
Server가 확정한 현재 combo stage의 clip에 속한 cue만 조회하므로 이 네 줄이 동시에 재생되는 구조가 아니다.
따라서 animevents 매핑을 다시 나누거나 ba2/ba4 문서를 새로 만드는 것은 금지한다.

회귀는 Effect Tool의 Open Editor 경계에 있다.

1. All Effects에서 정확한 BA3 Product cue를 고른다.
2. `Try_LoadDocumentPathStaged()`가 문서를 열면서 선택된 Product cue context를 지운다.
3. `Synchronize_LoadedSkillPreview()`가 문서 ID만으로 owner를 재추론한다.
4. exact mapped clip 후보를 못 찾으면 skillbinding의 네 BA clip 전체를 fallback으로 펼친다.
5. 그 결과 BA3 문서를 편집하면서 BA1/BA2/BA3/BA4 애니메이션이 이어져 보여 cue가 겹친 것처럼 보인다.

### 1.2 도화가 E Trail은 GPU buffer 부재가 아니라 CPU/GPU 계약 분리다

현재 renderer에는 trail point를 매 update tick에 모아 vertex/index를 만들고 dynamic GPU buffer를 갱신하는
경로가 이미 있다. 별도의 mesh-particle식 두 번째 buffer renderer를 만들 필요가 없다.

도화가 E의 세 source ribbon은 exact FlowRibbon material/profile을 선택하지만, direct-authored Tool preview는
두 경계에서 끊긴다.

- Effect Tool preview update가 source-anchor request를 playback에 공급하지 않는다. 세 ribbon은
  `WP_WSDM_09.B_Body_03` / `b_wp_1` follow anchor를 요구하므로 point가 한 개도 만들어지지 않는다.
- gameplay/hot reload에서는 anchor가 공급되지만 CPU playback이 VisualProgram projection admission이 없다는
  이유로 generic trail point만 만든다. 이후 GPU renderer는 exact FlowRibbon payload를 요구하므로 CPU가
  만든 generic payload를 거부한다.

결과는 다음과 같다.

```text
Tool: 붓 bone anchor 미공급 -> TrailPoints 0
Product: 붓 bone sampling과 dynamic trail buffer는 존재
-> CPU payload는 generic
-> renderer/shader는 FlowRibbon exact payload를 요구
-> object-local fail-close
-> 화면에는 trail이 보이지 않음
```

즉 뿌리 원인은 anchor, tick, buffer 자체가 아니라 direct-authored와 Product projection 사이의 admission
불일치다.

### 1.3 `fx_m_trail_010.dds`의 의미

`fx_m_trail_010.dds`는 도화가 E 31480의 source-exact texture가 아니라 도화가 D 31490 SpriteWave
occurrence의 main texture다. 따라서 E에 쓰는 경우에는 source-exact 복원이 아니라 사용자가 선택한
`PROJECT_TUNED` 흰백색 붓 trail overlay다.

해당 DDS의 alpha는 전 영역 1이라 alpha만 샘플하면 사각 strip이 된다. E overlay shader는 RGB luminance를
coverage로 사용하고, trail point age와 dissolve를 곱해 경계를 잘라야 한다. texture 이름만 같은 다른
sprite/ribbon에 이 규칙을 공통 적용하지 않는다.

### 1.4 차원술사 R helix는 한 occurrence의 burst 3개로 동시에 생성된다

현재 R 문서의 `fx_e_atypical_031` sprite 세 행은 각각 `0.50 / 0.70 / 1.05s`에 독립 occurrence로
배치되어 1타/2타/3타 시점을 표현한다. 반면 helix mesh는 하나의 element가 `0.01s`에 burst count 3을
동시에 생성하므로, 검격 3타와 시간축이 결합되지 않는다.

해결은 renderer에 helix 전용 반복 로직을 넣는 것이 아니라 동일 문서 element를 서로 다른 stable ID를 가진
세 occurrence로 나누는 것이다. 각 occurrence는 같은 mesh/material/sourceRecipe를 재사용하되 start time은
`0.50 / 0.70 / 1.05s`, burst count는 각각 1로 고정한다. 원본 source lifetime `0.6..1.0s`를 그대로 두면
세 occurrence가 화면에 누적되므로, 이 세 helix에만 `sourceScale.lifeTime=0.1`을 적용해 각 타격의 visible
particle tail을 다음 타격 전에 닫는다. 이는 generic particle lifetime 의미를 바꾸는 runtime 패치가 아니라
R의 세 project-authored occurrence에 한정된 trim이다. 이 저작 패턴을 Effect Tool에서 안전하게 만들 수
있도록 기존 `Delete Selected`와 같은 transaction 경계에 `Duplicate Selected`를 추가한다.

### 1.5 A/Q voronoi는 같은 DDS지만 같은 occurrence가 아니다

Q `2050100`과 A `2050210`은 모두 `fx_j_voronoi_tile_01.dds`를 쓰지만 stable element, local transform,
animation cue가 서로 다르다. A의 Play All 문서를 열고 Q를 시전한 결과가 다른 것은 anchor 누락이 아니라
서로 다른 스킬 occurrence를 비교한 결과다. Q의 slash와 voronoi는 같은 0.27초 root snapshot을 공유하므로
한 root 안에서 둘만 따로 어긋날 수 없다.

A에는 별도 제품 회귀가 있다. 현재 animevents가 동일 unified 문서를 250/500/750/1000ms에 네 번 재생하고
각 cue가 `follow`로 owner root/yaw를 다시 샘플한다. Tool Play All은 문서를 한 번만 재생하므로 gameplay와
생성 횟수·root history가 다르다. player anchor는 위치만이 아니라 yaw를 포함한 full affine transform이므로,
시전 중 방향이 바뀌면 각 full-document 복제의 local offset도 서로 다른 방향으로 회전한다.

해결은 anchor를 position-only로 만드는 것이 아니다. A의 outer cue는 cast-start snapshot 한 건으로 고정하고,
원본 source occurrence의 서로 다른 시간/위치를 문서 안의 explicit element occurrence로 복원해 slash와
voronoi가 같은 root를 정확히 한 번 합성하도록 한다. Q 데이터는 A와 공유하지 않으며 현재 손튜닝 문서를
자동 치환하지 않는다.

## 2. 고정할 계약

1. BA Product mapping은 `[ba1, ba1, ba3, ba1]`을 유지한다.
2. Open Editor가 선택한 Product cue와 로드 문서의 Effect ID가 같으면 exact stage/clip context를 보존한다.
3. Saved direct-authored Player Effect는 animevents와 skillbindings에서 실제 Product cue 후보만 얻는다.
4. Product cue 후보가 0개인 direct-authored Player Effect는 전체 스킬 chain으로 fallback하지 않고
   `UNMAPPED`로 격리한다.
5. `ba1.unified`의 세 후보 BA1/BA2/BA4는 자동 연속 재생하지 않는다. Tool에서 한 stage를 명시적으로
   선택하고 한 clip만 재생한다.
6. `ba3.unified`는 BA3 clip 하나만 재생한다.
7. cue `startMs`는 현재 문서에서 read-only로 표시한다. Effect Tool에서 timing을 저장하려면 Animation Tool의
   atomic event writer를 공유 API로 추출해야 하므로 이번 변경에서 안전하지 않은 줄 치환 writer를 만들지 않는다.
8. 도화가 E의 existing exact FlowRibbon 세 행은 direct-authored와 Product에서 동일한 CPU source payload를
   만든다.
9. Effect Tool은 preview character의 실제 model bone에서 `b_wp_1` anchor를 resolve한 경우에만 source anchor
   world를 playback에 공급한다. root나 임의 transform fallback은 금지한다.
10. E 붓 trail은 `b_wp_1` brush-tip attachment를 20ms 이하 간격으로 sample하고 기존 dynamic trail buffer를
   사용한다.
11. `fx_m_trail_010.dds` overlay는 E의 정확한 stable element/effect ID에서만 전용 profile을 선택하며,
    RGB luminance coverage, 흰백색 emissive, point-age fade, dissolve를 적용한다.
12. 잘못된 material tuple, 다른 skill/effect, filename만 같은 element에는 profile을 확장하지 않는다.
13. renderer 준비나 hot reload가 실패하면 기존 prepared Product와 현재 편집 문서를 보존한다.
14. `Duplicate Selected`는 선택 element를 새 stable ID로 deep-copy하고 전체 문서 검증 성공 뒤에만 draft에
    commit한다. 실패 시 문서와 선택 상태를 그대로 보존한다.
15. R helix는 동일 asset을 공유하는 세 독립 occurrence이며 `0.50 / 0.70 / 1.05s`에 각 1개만 생성한다.
    세 occurrence의 source lifetime trim은 각각 `0.1`이고 이전 타격 particle이 다음 타격에 누적되지 않는다.
16. Q와 A의 voronoi는 DDS만 공유하고 effect/element/transform identity는 공유하지 않는다.
17. A Product는 unified document를 네 번 재생하지 않고 cast-start cue 한 건과 explicit inner occurrences를 쓴다.
18. owner yaw는 root world에 한 번만 합성하며, Tool과 gameplay가 같은 local occurrence 배열을 소비한다.

## 3. 구현 단위

### G00. 정본과 실패 회귀 고정

- BA skillbinding 네 stage와 animevents exact tuple을 실행형 harness에서 고정한다.
- Product cue selection 후 Open Editor에서 context가 사라지는 상태와 Saved direct-authored 전체-chain
  fallback을 실패 계약으로 고정한다.
- 도화가 E의 세 existing ribbon stable ID, material family, brush anchor, sample interval과 현재 CPU/GPU
  payload 차이를 fixture로 고정한다.
- 사용자 dirty JSON의 시작 hash/status를 기록하고 모든 자동 materializer/publisher rewrite에서 제외한다.

### G01. BA Open Editor stage context 보존

- `CAnimationEffectCueDocument` 경계에 skillbinding ordered clips와 Product cues를 결합하는 순수 resolver를 둔다.
- resolver 결과는 `stageIndex`, `stageClipIndex`, `clipName`, `startMs`, `effectAssetId`를 가진 후보 목록이다.
- Effect Tool이 Product cue에서 Open Editor로 넘어갈 때 같은 Effect ID라면 기존 exact context를 유지한다.
- context가 없는 Saved Effect는 resolver 결과만 사용한다.
- 후보가 여러 개인 `ba1.unified`에는 BA1/BA2/BA4 stage selector를 노출하고 기본값은 첫 exact 후보 하나다.
- 후보가 없는 player direct-authored 문서는 animation을 바꾸지 않고 명시적인 unmapped status를 표시한다.
- generic non-player draft에만 기존 authoring preview fallback을 허용한다.

### G02. 도화가 E direct-authored FlowRibbon 경로 통합

- Effect Tool preview character/model에서 requested source bone을 resolve하고 socket local transform을 적용한
  world anchor만 `Set_SourceAnchorWorlds`에 전달한다.
- `Play All`의 순차 update와 loop seek 직후 pose를 지원한다. 임의 scrub/저FPS catch-up의 모든 fixed step에
  서로 다른 과거 bone pose를 재구축하는 기능은 별도 historical-pose 슬라이스로 남긴다.
- missing model/bone/anchor는 trail occurrence만 격리하고 root fallback으로 눈속임하지 않는다.
- `CEffectPlayback::Sample_Trail()`이 exact TypeDataRibbon + exact FlowRibbon typed material contract를 확인하면
  VisualProgram projection 유무와 무관하게 Cascade/FlowRibbon CPU payload를 계산한다.
- element ID나 texture filename만으로 admission하지 않는다. source material, carrier, required texture roles,
  typed modules가 모두 일치해야 한다.
- source color/dynamic parameter/width/UV/dissolve payload를 renderer profile 35가 기대하는 형태로 한 번만
  계산한다.
- Product projection 경로와 direct-authored 경로가 동일 point count, anchor history, typed payload를 내는지
  비교한다.

### G03. `fx_m_trail_010` 흰백색 brush override

- E의 기존 두 source-exact FlowRibbon은 유지하고, variant 1의 한 stable ribbon occurrence에만
  `PROJECT_TUNED` resource override를 적용한다. 별도 중복 trail occurrence는 만들지 않는다.
- override는 compiler source `fx_k_auraline_02`와 runtime authored `fx_m_trail_010`을 함께 기록해
  source provenance와 사용자의 project-tuned 선택을 분리한다.
- element는 기존 brush-tip attachment와 trail geometry 계약을 재사용하고, renderer는 정확한
  `(effectAssetId, stableElementId, sourceNode, compiler/runtime resource tuple)`에서만 profile 35를 유지한다.
- 기존 profile 35 HLSL이 DDS RGB luminance를 coverage로 만들고 source DynamicParameter dissolve,
  vertex point-age alpha를 적용한다. 따라서 alpha가 전부 1인 DDS의 사각 경계를 그대로 사용하지 않는다.
- width, lifetime, sample interval, emissive와 dissolve 시작은 Effect Tool에서 기존 document field로 조절할 수
  있게 유지한다. 새 persisted schema는 만들지 않는다.

### G04. 검증과 빌드

- 새 `--effect-tool-preview-fast`에서 BA exact tuple, candidate counts `3/1/0`, stage selector와 fail-closed를
  검증한다.
- Artist E focused mode에서 direct-authored/Product FlowRibbon payload parity, brush anchor tick history,
  selected profile, wrong tuple rejection, luminance edge alpha와 dissolve 종료를 검증한다.
- Effect pipeline Validate, JSON/XML/PowerShell parse와 scoped `git diff --check`를 실행한다.
- ClientFrontendHarness x64 Debug와 Client x64 Debug를 격리 출력으로 빌드한다.
- 실행 중 Client/Server가 있으면 종료하거나 출력물을 교체하지 않는다.

### G05. Duplicate Selected 및 R helix occurrence 분리

- `Delete Selected` 옆에 `Duplicate Selected`를 배치하고 현재 선택 element 하나만 복제한다.
- 복제본은 새 stable ID를 발급하고 self/master transform 참조가 원본 ID를 가리키면 새 ID로 교정한다.
- source provenance, renderer, resource binding과 authoring override는 보존하고 start/lifetime/burst는 복제 후
  기존 편집 패널로 독립 조절할 수 있게 한다.
- R의 현재 helix element를 template로 사용해 세 occurrence를 만들고 start time을 atypical 3타와 같은
  `0.50 / 0.70 / 1.05s`, burst count를 각각 1로 설정한다.
- 세 helix에만 source lifetime trim `0.1`을 적용하고, generic source-particle lifetime 계약은 변경하지 않는다.
- R 문서의 나머지 손튜닝 element와 순서, stable ID, 리소스 값은 변경하지 않는다.
- focused harness에서 세 helix ID의 유일성, start/lifetime/burst, 동일 mesh/material/source rotation과 실제
  playback의 3회 분리 생성을 검증한다.

### G06. A voronoi/slash occurrence와 cast root 정합

- A animevents의 동일 full-document 4회 cue를 한 cast-start snapshot cue로 정리한다.
- legacy/source evidence의 distinct occurrence time/position을 stable authored elements로 명시적으로 복원한다.
- inner occurrence 시작 시각은 source/native SwingHit 기준 `0.25 / 0.60 / 0.90 / 1.30s`로 고정한다.
- 동일 타격에 속한 slash/voronoi는 같은 local timeline과 root snapshot을 사용한다.
- yaw 0/90/180/270 fixture에서 local offset이 root yaw로 정확히 한 번만 회전하고 Tool/gameplay 결과가
  일치하는지 검증한다.
- Q는 별도 effect identity임을 harness에서 고정하고 A transform을 Q에 복사하지 않는다.

## 4. 완료 조건

### 자동 검증

- BA cue tuple이 `[ba1, ba1, ba3, ba1]`이고 네 cue가 각각 해당 clip-local `startMs=0`이다.
- Open Editor BA3는 BA3 clip 하나만, ba1 문서는 선택한 BA1/BA2/BA4 중 하나만 재생한다.
- unmapped ba2/ba4 문서는 전체 BA chain을 재생하지 않는다.
- E exact source ribbons가 direct-authored preview에서도 renderer profile 35에 필요한 payload를 가진다.
- E project-tuned brush overlay만 새 profile을 사용하고 잘못된 effect/material/resource는 fail-closed한다.
- dynamic trail buffer가 update tick별 point를 받고, 흰백색 emissive/coverage/dissolve 수치 fixture가 통과한다.
- Duplicate Selected가 고유 ID를 만들고 실패 transaction에서 원본 draft를 보존한다.
- R helix가 `0.50 / 0.70 / 1.05s`에 하나씩만 생성되고 같은 tick에 3개가 겹치지 않는다.
- A는 한 outer cue와 explicit inner occurrences를 가지며 방향 전환 중에도 slash/voronoi 상대 배치가 유지된다.
- 관련 harness와 Debug compile/link가 통과한다.

### 사용자 수동 확인

- All Effects -> DimensionMaster -> LMB -> BA3 -> Open Editor에서 BA3 애니메이션만 재생되는지 확인한다.
- ba1 문서에서 BA1/BA2/BA4 selector가 각각 한 animation만 재생하는지 확인한다.
- Artist E에서 붓을 휘두르는 구간에 trail이 brush tip을 따라 연속 생성되는지 확인한다.
- trail이 흰백색 emissive로 보이고 사각 card가 아니라 양 끝/수명에 따라 dissolve되는지 확인한다.
- R에서 세 타격 시점마다 helix 검격이 하나씩 나타나며 서로 동시에 겹치지 않는지 확인한다.
- A를 서로 다른 시전 방향으로 사용해도 각 voronoi가 대응 검격 옆의 같은 상대 위치에 생성되는지 확인한다.

Client 실행과 최종 시각 충실도 판정은 사용자가 수행한다.
