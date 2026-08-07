# 2026-08-08 차원술사 A 2050210 원본 복구 결과

## 1. 결론

차원술사 A의 정본을 수동 16-layer candidate가 아니라 원본 추출 계약 기반 117 Elements로
되돌렸다. 수동 `body/rim/highlight/afterimage` 문서는 삭제하지 않았지만 시각 참고용이며 제품
정본이 아니다.

이번 결과는 원본 Geometry, Cascade module, event time, anchor, named Material input을 보존하고
실행하는 단계까지 완료했다. cooked package에서 소실된 Material graph 연산은 근사 profile이므로
원작 픽셀 완료 또는 `RUNTIME_EXACT`로 승격하지 않았다.

## 2. 반영된 원본 계약

| 항목 | 결과 |
|---|---:|
| canonical Elements | 117 |
| Particle container | 100 |
| Mesh-backed Particle | 48 |
| Sprite | 52 |
| Light | 4 |
| Screen Post | 13 |
| LinearFlow occurrence | 12 |
| LinearFlow required named inputs | 12 occurrences 모두 7/7 |
| Material `RUNTIME_EXACT` | 0 |

원본 네 타격의 source event는 다음과 같다.

| Hit | Source time | Root-relative position |
|---:|---:|---|
| 1 | `0.25 s` | `(0.5, -0.5, -0.9)` |
| 2 | `0.60 s` | `(0.5, -0.5, 0.8)` |
| 3 | `0.90 s` | `(0.5, 0.3, -0.9)` |
| 4 | `1.30 s` | `(0.5, 0.6, -0.8)` |

각 타격의 핵심 LinearFlow Mesh는 `fm_m_trail_002`, `fm_h_swing_02`, `fm_m_trail_01`이고,
각 occurrence가 원본 부모의 named texture 입력을 직접 보존한다.

## 3. Material 복구

원본 package `ZHJ4TC4PCK4PR4J22HIXEYUXBU.upk`에서 부모 Material 증거를 직접 추출했다.
`fx_m.fx_j_pa_linearflow_02_tr`의 실측은 다음과 같다.

```text
Expression entries       344
Non-null                 110
Null                     234
Named textures             7
Unresolved input edges     45
Topology status          COOKED_PARTIAL
Runtime exact eligible   false
```

문서와 런타임은 `diff_tex`, `diff_noise_tex`, `a_mask_tex`, `a_noise_01_tex`, `b_mask_tex`,
`b_noise_01_tex`, `dissolve_tex`를 이름 기준으로 stage한다. 누락되거나 잘못된 리소스는 숨김과
명시적 오류로 처리하며 white fallback을 사용하지 않는다.

Publisher가 이 texture들을 manifest dependency로 게시하는 것과 C++ runtime이 compiled
Assembly에서 수집하는 dependency 집합도 동일하게 맞췄다. 이 불일치가 있으면 런타임이 최신
Catalog 전체를 거부하던 버그도 함께 해결했다.

## 4. Animation과 Effect 시간축

A animation clip `pc_sp_m_00_sk_sk_willowrend`의 authored `playRate`를 Effect playback rate에도
전달한다. 따라서 Animation 담당자가 일정 배속을 바꾸더라도 Effect 문서의 source event 시간을
다시 수동 환산하지 않는다.

```text
effect source time = animation source time
wall-clock time    = source time / playRate
```

일반적인 A layer는 각 event 프레임의 Character root를 snapshot한 뒤 재생한다. 원본에서
`follow=true`인 layer만 현재 anchor를 계속 추적한다.

## 5. 생성물과 검증

- canonical A SHA-256:
  `D8D5C8AB6160DE820A56019D6BC36C7DC95037391EED3821F4635B1C7B06CC76`
- Assembly/WFX: 16 Effects / 182 Components / 947 Emitters
- Runtime Catalog publish: PASS
- Python: 34 tests PASS
- Effect pipeline: PASS
- Client x64 Debug build: 오류 0
- ClientFrontendHarness: `failures 0`
- Effect Tool final audit: PASS
- JSON parse와 `git diff --check`: PASS
- ProjectAudit: FAIL 1
  - `projects.data-source-visibility: expected=550 project=547 filters=547`
  - 공유 작업트리의 DataFiles 프로젝트 등록 불일치이며 A 문서·Assembly·Runtime stage는 통과했다.

## 6. 수동 GPU 검증

수동 GPU A/B는 아직 PASS 처리하지 않았다. 다음 실행에서 Effect Tool의 수동 candidate가 아니라
canonical `effect.dimensionmaster.skill.2050210`을 선택한다.

1. DimensionMaster의 A animation을 재생한다.
2. Screen Post를 끄고 `0.25 / 0.60 / 0.90 / 1.30초`를 차례로 확인한다.
3. 첫 타격에서 emitter 14, 15, 20을 Solo한다.
   - emitter 14: `fm_m_trail_002`
   - emitter 15: `fm_h_swing_02`
   - emitter 20: `fm_m_trail_01`
4. opaque card가 없는지, Mesh 방향과 root snapshot이 맞는지, Mask/Dissolve/Flow가 동작하는지
   확인한다.
5. Geometry와 Material을 확인한 뒤 Screen Post를 켜고 네 타격의 누적과 마지막 잔상을 본다.

캡처에는 Active Effect ID, sample time, selected emitter ID, Screen Post ON/OFF, camera/FOV를
함께 남긴다.

## 7. 남은 경계

- `RUNTIME_EXACT=0`이다. 45개 unresolved graph edge가 복구되거나 GPU A/B로 finite 식이 검증되기
  전에는 exact라고 부르지 않는다.
- 현재 LinearFlow HLSL은 원본 7개 texture와 MI scalar/vector를 소비하는 source-driven
  reconstruction이다. 원본 parent graph의 모든 Multiply/Lerp/Panner 연결과 동일하다는 증거는 없다.
- A 수동 candidate는 비교 자료일 뿐 canonical publish에 사용하지 않는다.
- 향후 base11 자동 promotion은 canonical A SHA가 기존 receipt와 다르면 의도된 수동 override로
  거부한다. A를 덮기 전에 이번 canonical을 명시적으로 보존해야 한다.
