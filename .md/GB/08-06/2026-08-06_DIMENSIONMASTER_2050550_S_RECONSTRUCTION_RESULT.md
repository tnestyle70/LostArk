# 차원술사 2050550 S 원본 타임라인 복원 구현 결과

## 1. 결론

`C:\Users\user\Desktop\로스트아크이펙트이미지\차원술사`가 맞다. 이 폴더의
`차원술사_S00.png`부터 `차원술사_S06.png`까지 7장을 S 시각 정합성 참고 자료로 확인했다.
PNG를 sprite animation으로 재생하지는 않았고, 원본 animation notify와 Cascade graph를 실행 데이터로
사용했다.

DimensionMaster의 S 입력은 이제 기존 `2050210` 대신 `2050550 / 찰나` 하나로 연결된다.

```text
S 입력
  -> PlayerSkills 2050550
  -> Server 승인 action 5.000초 / hit 3.750초
  -> DimensionMaster.skillbindings
  -> pc_sp_m_00_sk_sk_super_instance
  -> authored animation Effect cue 0~5000ms, root follow, cue_end
  -> effect.dimensionmaster.skill.2050550
  -> 원본 notify 시각을 가진 175 runtime layers
```

Effect가 animation 계산에 종속된 구조는 아니다. Effect 문서는 단독 preview가 가능하다. 다만 제품
gameplay에서는 승인된 skill action이 body animation을 시작하고, 그 clip의 authored cue가 Effect asset을
시작하는 presentation 경로를 사용한다. 따라서 S를 눌렀을 때 animation과 Effect가 같은 action에서 함께
시작하고 5초 경계에서 함께 닫힌다.

현재 결과는 “원본과 완전히 동일”이 아니다. 원본 타임라인, 주요 mesh, texture, spawn/lifetime/size/
velocity/color 범위는 자동 복원했지만 Light, screen-space post effect, 3D rotation, curve 전체, dynamic
parameter 등은 현재 Effect 문서가 그대로 표현하지 못한다. 이 차이는 conversion receipt에 숨기지 않고
기록했다.

## 2. S binding와 animation 결과

| 계약 | 반영값 |
|---|---|
| input slot | `S` |
| skill | `2050550 / 찰나` |
| action ID | `dimensionmaster.skill.2050550` |
| action duration | `5000ms` |
| hit time | `3750ms` |
| body clip | `pc_sp_m_00_sk_sk_super_instance` |
| raw model clip | 150 ticks / 24 tps = 6.25초 |
| playback rate | `6.25 / 5.0 = 1.25` |
| Effect cue | `startms=0 endms=5000` |
| anchor/follow/stop | `root / follow / cue_end` |
| Effect asset | `effect.dimensionmaster.skill.2050550` |

Client Character와 Effect Tool은 특정 skill ID를 하드코딩하지 않고 PlayerSkills의
`actionDurationMs`와 binding clip의 실제 tick/tps를 비교해 ACTIVE clip playback rate를 계산한다. S는
1.25배로 재생되며 locomotion이나 다른 animation으로 복귀할 때 model speed는 1.0으로 돌아간다.

5초 `cue_end`는 중요하다. 원본 BGCrack particle 중 20초 lifetime이 존재하므로 문서 자체의 자연 종료는
23.699초다. gameplay cue가 natural stop이면 S action이 끝난 뒤에도 균열 Effect가 오래 남는다. 제품
S는 5초에 제거하고, Effect Tool의 문서 단독 preview에서는 긴 tail을 살펴볼 수 있게 원본 lifetime을
보존했다.

## 3. 추출과 최종 문서 구조

| 항목 | 실측 |
|---|---:|
| source ParticleSystem | 24 |
| 원본 EFFECT notify | 28 |
| particle graph로 해결된 EFFECT notify | 26 |
| unsupported source notify | 2 |
| 별도 camera shake | 2 |
| FIRST_LOD emitter partition | 200 |
| 변환 emitter | 163 |
| unsupported emitter | 37 |
| 최종 Elements | 175 |
| Particle layers | 174 |
| Decal layers | 1 |
| mesh-backed Particle layers | 59 |
| distinct runtime group ID | 19 |
| Particle runtime budget | 2,773 |
| 외부 module 참조 해결 | 1,647 / 1,647 |
| source material binding 해결 | 138 / 140 |
| runtime resource binding 해결 | 190 / 210 |

175는 서로 무관한 Effect 175개가 아니다. `effect.dimensionmaster.skill.2050550` 문서 하나 안에 원본
ParticleSystem group, emitter partition, 지연 burst, 동일 system의 반복 notify occurrence가 layer로
보존된 것이다. 동일 BGCrack 호출 두 건도 첫 호출 하나로 합쳐 버리지 않고 각각 독립 occurrence로
남겼다.

unsupported 37 partitions는 다음과 같다.

| 이유 | 수 |
|---|---:|
| runtime base texture binding 없음 | 31 |
| Light renderer 미지원 | 4 |
| fullscreen RGB Noise / Zoom Blur 미지원 | 2 |

## 4. 원본 타임라인 반영값

| 시각 | 원본 역할 | 최종 layer 수 |
|---:|---|---:|
| 0.010s | Clock / 초기 field / ExMove | 21 |
| 0.950s | ClockExp / BG / Light / TimePause | 32 |
| 1.500s | Weapon Group | 16 |
| 2.178s | Hand Group | 12 |
| 2.900s | Light | 내부 layer delay 포함 |
| 3.000s | Hole / BGCrack 두 occurrence | 16 |
| 3.700s | Atk 02 | 5 |
| 3.750s | Atk 01/03, Back, Dust, Localdust, post/light 요청 | 55 |

Emitter 내부 delay 때문에 실제 Element 시작값은 0.01~3.85초에 분포한다. importer는
`animation notify global time + Required.EmitterDelay`를 `startDelaySeconds`로 저장한다.

기존 변환기는 `timing.lifeTimeSeconds`에 particle lifetime까지 포함하고, runtime이 다시 particle tail을
더해 20초 BGCrack이 약 43초가 되는 이중 계산이 있었다. 이를 다음처럼 분리했다.

```text
timing.lifeTimeSeconds
  = max(Required.EmitterDuration, notify duration, 0.1)

particle.lifeTimeSeconds
  = 원본 ParticleModuleLifetime min/max

runtime document duration
  = start delay + emitter active duration + particle max lifetime
```

보정 후 emitter active duration은 0.1~5.0초, particle lifetime은 0.05~20.0초이며 문서 최대 자연
종료는 BGCrack의 `3.0 + 0.699 + 20.0 = 23.699초`다. gameplay는 별도 cue 계약으로 5초에 닫는다.

## 5. Weapon Group 실측

1.5초 Weapon Group은 16 layers다. 검 본체 두 layer와 cross plane/ring/helix/box 및 glow/deco
particle 14 layers가 함께 있다.

| 값 | Weapon emitter 1 | Weapon emitter 2 |
|---|---|---|
| mesh | `fm_s_swp_superweapon_01.wmodel` | 동일 |
| start/emitter duration | 1.5s / 3.1s | 1.5s / 3.1s |
| particle lifetime | 1.5s | 1.5s |
| spawn/burst/max | 0 / 1 / 3 | 0 / 1 / 3 |
| initial position | `[-0.18,-0.18,-0.18]~[0,0,0]` | 동일 |
| start/end size | `[0.027,0.027] -> [0,0]` | 동일 |
| start RGBA | `[1,1,1,1]` | `[1,1.5,1,1]` |
| mapped emissive | `1.5` | `1.0` |
| base | `pc_dl_av_219a_paning.dds` | `sk_swp_dmc_01_d.dds` |
| noise/normal slot | 없음 | `sk_swp_dmc_01_n.dds` |

Emitter 1의 원본 material evidence는 `sk_swp_dmc_01_mi_dead`다. 이전 휴리스틱은 이 material의
`fx_panning_intensity=100`과 `distortion_intensity`까지 emissive로 오인해 검 밝기를 100으로 덮었다.
이를 수정해 명시적인 `emissive_intensity=1.5`만 선택하고 distortion은 별도 필드로 보낸다.
`flicker/speed/tile/pan/min/max/power` 보조 scalar는 emissive 대표값으로 선택하지 않는다.

다만 원본 `ParticleModuleMeshRotation`, `CameraOffset`, `ParameterDynamic`은 두 검 emitter 모두 현재
문서에서 표현되지 않는다. 따라서 검 mesh/resource와 크기·수명은 근거가 있지만 3D 방향, 카메라 기준
오프셋, material의 동적 parameter animation은 아직 원작 동일하다고 할 수 없다.

## 6. Effect parameter 대조

### 6.1 Transform와 scale

현재 175 Elements의 상위 `detail.transform`은 모두 다음 identity다.

```text
position             [0,0,0]
rotationDegrees      [0,0,0]
revolution/sec       [0,0,0]
scale                [1,1,1]
velocity/sec         [0,0,0]
```

원본 Location module은 상위 Transform이 아니라 particle spawn range로 변환된다. UE source unit에
`0.01`을 곱하며, 최종 전체 범위는 X `-17~11`, Y `-2~11`, Z `-1.2~11`이다. 모든 S layers는 현재
local-space로 저장됐다.

Particle start size는 X `0.01~50`, Y `0.01~100`, end size는 X `0~75`, Y `0~120`이다. Z size와
mesh 3D scale curve는 복원하지 않는다. 이 값은 캐릭터나 summon이 10,000배 작다는 의미가 아니고 각
particle quad/mesh의 runtime 크기다. 전체 Particle System scale은 현재 1.0이며 사용자가 요청한 대로
임의 수동 보정하지 않았다.

S의 유일한 Decal은 3.75초 Dust group의 `particlespriteemitter_6`이며 `decal.size=[0.01,16]`,
active duration 0.1초다. Particle System의 global scale/yaw/speed modifier는 Particle layers에만
적용되고 이 Decal에는 적용되지 않는다. Decal은 Element Detail에서 별도로 튜닝해야 한다.

### 6.2 Velocity와 force

`ParticleModuleVelocity.StartVelocity`의 min/max range를 각 축에 0.01 단위 변환해 저장한다.

| 축 | 최종 전체 velocity 범위 |
|---|---:|
| X | -12~3 |
| Y | -12~1 |
| Z | -8~5 |

51 converted emitters가 initial velocity mapping을 갖는다. 현재 runtime force는 constant acceleration
하나뿐이다. 원본 random/curve acceleration은 대표 vector 하나로 축약됐고 4 layers만 non-zero다.

| group/layer | acceleration |
|---|---|
| Atk 02 emitter 56 | `[0,0,-0.5]` |
| Dust emitter 8 | `[0,0,-0.5]` |
| Hand emitter 43 | `[0,0,-0.35]` |
| Localdust emitter 71 | `[0,0,-5.0]` |

VelocityOverLife 44, Vortex 4, Orbit 4, primitive sphere/cylinder/circle surface, vector field 계열,
camera offset는 현재 같은 force field로 재현되지 않는다. 따라서 “방향과 초기 속도”는 조정 가능하지만
원본의 복합 힘 modifier 전체가 복원된 상태는 아니다.

### 6.3 Spawn, burst, lifetime, size modifier

| 필드 | 최종 범위 |
|---|---:|
| spawn rate/sec | 0~230 |
| burst count | 0~64 |
| max particles/layer | 3~64 |
| Particle budget | 2,773 |
| particle lifetime | 0.05~20.0s |
| emitter active duration | 0.1~5.0s |

`PeakActiveParticles`는 layer당 최대 64로 budget clamp한다. Burst time이 0보다 크면 독립 deterministic
layer로 분리한다. SizeMultiplyLife와 ColorOverLife의 다중 key curve는 전체 key를 유지하지 않고 시작/
마지막 sample의 linear lerp로 축약한다.

### 6.4 RGBA와 emissive

95 converted emitters가 StartColor/StartAlpha mapping을 갖고, 최종 문서에서는 반복 occurrence까지
포함해 171 layers가 end-color linear lerp를 사용한다.

| component | start 범위 | end 범위 |
|---|---:|---:|
| R | 0~100 | 0~1,500 |
| G | 0~600 | 0~400 |
| B | 0~300 | 0~400 |
| A | 0.05~100 | 0~3 |

이 값은 원본 Cascade/material이 0~1 범위만 사용하지 않기 때문에 그대로 높은 값이 존재한다. 현재
renderer는 이를 색 multiplier로 직접 사용한다. 또 particle render 단계에서 alpha에 항상
`1 - normalizedAge`를 한 번 더 곱한다. 원본 curve가 이미 fade-out을 소유한 경우에는 추가 감쇠가 되어
원본과 차이가 난다.

원본 material evidence의 emissive/bloom/glow/intensity 관련 unique scalar는 97개이며 -10~366 범위다.
현재 변환은 명시적인 emissive strength/intensity 후보 31개만 선택하고 runtime 안전상 0~100으로
clamp한다. 최종 Element emissive 범위는 0.1~100이고 25 layers가 기본값 1.0과 다르다. Material
Instance shader의 core strength, exponent, flicker, min/max 조합을 단일 float 하나로 완전히 재현하는
것은 아니므로 emissive는 PNG A/B 튜닝 우선 대상이다.

### 6.5 Noise, dissolve, UV

| runtime resource slot | binding 수 |
|---|---:|
| Base | 172 |
| Noise | 111 |
| Mask | 67 |
| Emissive | 43 |
| Dissolve | 28 |
| Mesh Model | 59 |

원본 material의 noise 관련 unique scalar는 182개(-1~25), dissolve 관련 scalar는 102개
(-6.28~100)다. 현재는 texture parameter 이름으로 slot을 선택하는 휴리스틱이며 독립 noise power,
tile, color, 여러 texture별 panner를 그대로 표현하지 못한다. UV speed는 한 쌍만 소유해 X
`-0.2~1.0`, Y `-0.6~2.0`이고 서로 다른 texture panner 중 마지막 매핑이 대표값이 된다. SubUV sequence는
6 layers다.

Dissolve texture 28개는 연결됐지만 175 Elements의 `dissolveStartNormalized`가 모두 1.0이다. 현재
shader 계산에서는 dissolve amount가 계속 0이므로 원본 dissolve progression은 아직 실행되지 않는다.
즉 texture가 연결된 것과 원본 dissolve animation이 복원된 것은 별개다.

### 6.6 변환 정확도 영수증

| mapping 판정 | 수 |
|---|---:|
| EXACT | 326 |
| EXACT_RANGE | 83 |
| APPROXIMATION | 1,050 |
| PARAMETER_NAME_HEURISTIC | 344 |
| BUDGET_CLAMP | 163 |
| DEFAULT_NEEDS_TUNING | 3 |

Resource 쪽은 mesh runtime binding 53건만 exact이며 texture slot 392건은 parameter-name heuristic이다.
주요 미표현 module은 ParameterDynamic 130, Rotation 69, CameraOffset 55, MeshRotation 49,
MeshRotationRate 33, EF VelocityOverLife 31 등이다. 전체 원문과 Element별 값은
`skill.2050550.element-conversion-receipt.json`에서 source object path까지 추적할 수 있다.

## 7. 현재 Effect Tool에 보이는 의미

All Effects에서 S를 선택하면 175개를 별도 파일 175개로 여는 것이 아니라 Authored Effect 문서 하나를
연다. 그 아래에서 complete Particle System과 child layer를 선택한다.

Particle System 선택 시 다음 공통 modifier가 있다.

```text
Uniform Scale Multiplier       1.0
System Yaw Offset              0 degrees
Emission Direction Yaw         0 degrees
Initial Speed Multiplier       1.0
```

숫자 drag는 live preview, `Apply Particle System`은 active Document memory commit, `Save`는 JSON 영구
저장이다. System yaw는 Particle layout 전체, direction yaw와 speed multiplier는 initial velocity에만
적용한다. non-particle Decal에는 적용하지 않는다. Source group ID 19개는 child label과 receipt 추적에
사용하지만 group별 multiplier 편집기는 아직 없어서 세부 조정은 Element 단위다.

Element Detail의 top-level Transform/Velocity는 원본 particle spawn range와 다른 값이다. 원본 복원값을
조정하려면 Particle의 `Initial Position`, `Initial Velocity`, `Acceleration`, `Start/End Size`,
`Lifetime`, `Spawn/Burst`를 봐야 한다. top-level Transform을 조정하면 emitter 전체 배치에 추가 offset을
주는 authoring override가 된다.

## 8. 자동 검증

- importer unit tests: 3 tests PASS
  - repeated ParticleSystem occurrence 보존
  - emitter duration과 particle tail 분리
  - 미표현 CameraOffset receipt 보존
  - panning/distortion intensity를 emissive로 오인하지 않음
- external module closure: 1,647 / 1,647 resolved, property error 0
- Effect Validate/Publish: 11 Effects PASS
- Effect pipeline: PASS
- Effect Tool final audit: PASS
  - S 175 layers
  - 0.010/0.950/1.500/2.178/3.000/3.700/3.750초 landmark
  - Weapon Group 16 layers
  - Superweapon mesh + DMC01 texture
  - 0~5000ms root follow/cue_end
- ClientFrontendHarness x64 Debug: `failures : 0`
  - `DimensionMaster S Resolves 2050550 Super Instance At Five Seconds` PASS
  - 모든 PlayerSkills effectId와 Authored 문서 join PASS
- Client x64 Debug build: PASS
- Server x64 Debug build: PASS
- `Server.exe --contract-test`: `failures : 0`
- ProjectAudit: 75 checks PASS
- `git diff --check`: PASS; 기존 dirty worktree의 line-ending warning만 존재

## 9. 수동 검증과 남은 범위

자동 검증은 S 입력 ID, server action, body clip, Effect cue, 문서 load, resource 존재, timeline과 parameter
계약을 닫았다. 실제 화면에서 S00~S06과 같은 구도·크기·밝기인지에 대한 frame A/B 수동 판정은 아직
완료하지 않았다.

다음 수동 튜닝은 원본을 덮어쓰기 전에 다음 순서로 제한한다.

1. Complete S를 0~5초 skill window로 재생해 body/weapon/impact timing을 확인한다.
2. 1.5초 Weapon Group에서 mesh rotation과 pivot 차이를 먼저 확인한다.
3. 3.0초 BGCrack과 3.75초 final impact를 audition한다.
4. size와 emissive를 PNG S00~S06에 맞춰 보수적으로 조정한다.
5. Light/Post/rotation/dynamic parameter/dissolve curve는 지원 기능 구현과 시각 수동 보정을 구분한다.

현재 근거로는 PNG만 보고 처음부터 손으로 재제작하는 것보다, 이 원본 baseline에서 receipt가
`APPROXIMATION` 또는 `UNREPRESENTED`로 표시한 부분만 보정하는 방향이 더 빠르고 추적 가능하다.
