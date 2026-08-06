# 차원술사 2050550 S 원본 계약 100% 복원 구현 계획

## 1. 문서 목적

차원술사 S `skillId=2050550`, `effectId=effect.dimensionmaster.skill.2050550` 하나를 대상으로
원본 UE3 데이터에 기록된 의미를 손실 없이 파싱하고 기존 LostArk Effect 런타임에서 동일하게 실행하는
수직 슬라이스를 닫는다. 이 S에서 확정한 변환 계약을 이후 차원술사 스킬에 재사용한다.

이 계획의 정본 우선순위는 다음으로 고정한다.

```text
원본 UE3 데이터
Animation Sequence / Notify / Cascade / Material Instance / Mesh
        ↓
손실 없는 Imported Recipe
        ↓
동일 의미의 기존 Effect 런타임 확장
        ↓
Imported baseline + Authored override 병합
        ↓
실행용 Effect 문서 자동 생성
        ↓
고정 카메라 PNG A/B 결과 검증
```

PNG 프레임은 원본 시간, 속도, 회전, 수명 또는 피벗을 역산하는 입력으로 사용하지 않는다. 원본 수치를
모두 실행한 뒤 렌더러 차이에서 생기는 밝기, 블렌딩, 노출, 텍스처 매핑 문제를 찾는 시각 회귀 자료로만
사용한다.

## 2. 완료 정의와 비목표

### 2.1 완료 정의

이 작업의 “100%”는 원본 데이터 계약의 해석과 실행 coverage를 뜻한다.

```text
EXACT                  완료
APPROXIMATION          미완료
UNSUPPORTED            미완료
MISSING_RESOURCE       미완료
DEFAULT_NEEDS_TUNING   미완료
```

S는 Animation/Notify/Cascade/Material/Mesh의 모든 소비 필드가 `EXACT`이고, 실행되지 않은 원본 필드와
fallback resource가 0일 때만 원본 복원 완료로 표시한다.

### 2.2 비목표

- PNG를 sprite sheet 또는 평면 동영상으로 재생하지 않는다.
- 175개 실행 layer를 눈으로 다시 만드는 수동 복각을 하지 않는다.
- 원작 영상에서 눈대중으로 시간을 재어 source timeline을 압축하지 않는다.
- 원본 수치가 있는데 현재 Effect Detail의 좁은 필드에 맞춰 start/end, XY size 또는 상수값으로 축약하지 않는다.
- 원작 클라이언트와 픽셀 단위 동일성을 자동으로 주장하지 않는다. 카메라, HDR, exposure, shader 구현 차이는
  source coverage가 닫힌 뒤 별도 A/B 결과로 판정한다.
- 새 Effect runtime 또는 두 번째 모델 runtime을 만들지 않는다. 기존 `CEffectPlayback`,
  `CEffectDocumentRenderer`, `CModel -> CMaterial` 경로를 확장한다.

## 3. 확인된 원본 시간 계약

S에서 아래 사건은 서로 다른 정본이다. 하나를 다른 사건에 맞추기 위해 임의 배속하거나 이동하지 않는다.

| 사건 | 원본 값 | 정본 |
|---|---:|---|
| body animation | 151 frames / 30fps = 5.0s | ActorX PSA sequence report |
| clock/initial field | 0.010s | Animation Notify |
| background/time pause | 0.950s | Animation Notify |
| weapon group | 1.500s | Animation Notify |
| hand group | 2.178s | Animation Notify |
| hole/crack | 3.000s | Animation Notify |
| final visual occurrence | 3.700s, 3.750s | Animation Notify |
| gameplay hit | 1.730s | `DimensionMaster.skilltiming` |
| cancel windows | 3.800~5.000s | Animation Notify/InputTiming |

현재 character WModel의 S clip이 24 ticks/sec로 쿠킹되어 6.25초로 읽히는 상태는 원본 값이 아니다.
`actionDurationMs=5000`으로 나눈 `1.25`를 보정값으로 유지하지 않고, source sequence rate를 보존해 다시
쿠킹한 WModel이 정상 속도 `1.0`에서 5.0초를 재생하게 한다. `PlayerSkills.hitTimeMs`의 현재 3750도 최종
시각 이펙트와 혼동된 값이므로 원본 gameplay timing인 1730으로 교정한다.

## 4. 현재 실측 baseline

| 항목 | 현재 실측 | 완료 목표 |
|---|---:|---:|
| source particle systems | 24 | 24 |
| source effect notify | 28 | 28개 모두 typed channel로 accounting |
| resolved particle notify | 26 | 26 |
| source unsupported notify | 2 | 0 |
| related ViewShake | 2 | 2개 실행 |
| external Cascade module request | 1,647/1,647 | 유지 |
| source emitter partition | 200 | 200 |
| emitted runtime Element | 175 | 수가 아니라 source occurrence/partition coverage로 판정 |
| approximation | 163 | 0 |
| unsupported emitter | 37 | 0 |
| material binding | 138/140 | 140/140 |
| runtime resource binding | 190/210 | 210/210 |
| fallback white texture | 존재 | 0 |
| WModel source rate | 전체 24 ticks/sec | sequence별 원본 rate |
| Cascade curve | min/max 또는 start/end 축약 | 모든 key/interpolation 보존 |

현재 `unsupported emitter 37`의 직접 원인은 texture binding 없는 renderer 31, Light 4,
screen-space post 2다. 외부 모듈 참조는 모두 해결됐으므로 다시 눈으로 만드는 문제가 아니라, 파싱 데이터가
가진 원본 의미를 받을 recipe와 runtime 기능을 확장해야 하는 상태다.

## 5. 제품 데이터 구조

### 5.1 세 문서의 역할

```text
Imported Recipe
  원본에서 언제든 다시 생성 가능한 lossless baseline
  source package/object/occurrence/module/distribution/material provenance 소유

Authored Override
  사용자가 명시적으로 조정한 stable source ID별 delta만 소유
  원본 parser 재실행으로 덮어쓰지 않음

Published Runtime Document
  publisher가 Imported + Authored를 parse → validate → stage → commit으로 병합
  기존 CEffectPlayback과 CEffectDocumentRenderer가 소비
```

Imported를 Authored로 통째로 복사한 뒤 수동 편집하는 구조를 source 100% 경로로 사용하지 않는다. override는
source system/emitter/module/occurrence의 stable ID에 결합하며, source revision이 달라져 대상이 사라지거나
중복되면 publish를 실패시킨다.

### 5.2 S의 사용자 관점 구조

```text
S | 찰나
├─ Body Animation
├─ Timeline
├─ Clock / Initial Field
├─ Background / Time Pause
├─ Weapon
│  ├─ Superweapon Mesh
│  ├─ DMC01 Material Instance
│  └─ Glow / Decoration emitter
├─ Hand
├─ Hole / Crack
├─ Final Impact
├─ Light / Camera / Post / Sound
└─ Coverage
   ├─ Exact
   ├─ Approximation
   ├─ Unsupported
   ├─ Missing Resource
   └─ Default Needs Tuning
```

스킬은 Effect 문서 하나이며 그 아래 의미 있는 occurrence group과 emitter/layer가 존재한다. 200 partition 또는
175 Element를 물리적으로 한 Particle로 합치지 않는다. 대신 전체 multiplier는 document/system group에서,
원본 수치와 정밀 override는 emitter/module에서 조정한다.

## 6. 구현 단계

### G00. Animation source rate와 gameplay timing 복원

1. `build_actorx_fbx.py`가 PSA sequence별 fps를 report에만 쓰지 않고 export/cook 결과에서도 보존하게 한다.
2. FBX 하나에 서로 다른 source rate가 섞일 때 rate가 24fps scene에 재샘플링되지 않도록 sequence duration과
   key time을 원본 초 단위로 고정한다.
3. Model cook 결과의 animation별 `durationTicks / ticksPerSecond`를 ActorX report의
   `(frames - source endpoint policy) / fps`와 대조하는 receipt를 생성한다.
4. DimensionMaster 154개 clip을 재쿠킹하고 S가 5.0초인지 우선 검사한다.
5. `CCharacter::Calculate_ActiveChainPlaybackRate`와 Effect Tool의 동일 보정을 source rate 오류 보정에 사용하지
   않는다. 정상 WModel은 rate 1.0을 사용한다.
6. `PlayerSkills.json`의 2050550 `hitTimeMs`를 원본 1730으로 교정하고 balance receipt/publisher 검증을 함께
   갱신한다.

완료 증거는 source report, cooked model inspection, character runtime cursor가 같은 S landmark를 오차 1ms
이내로 가리키는 자동 검사다.

### G01. Lossless Effect Recipe와 provenance

1. Effect authoring format을 확장해 Constant, Uniform, ConstantCurve, UniformCurve, seeded distribution을
   구분한다.
2. curve의 모든 key time/value, tangent, interpolation과 distribution의 min/max를 순서대로 보존한다.
3. emitter duration/delay/loop, burst time/count, local/world space, LOD, random seed 정책을 원본 타입으로 둔다.
4. 각 occurrence/system/emitter/module에 stable source ID와 package/object path를 저장한다.
5. 모든 source field에 mapping status와 소비 runtime field를 기록한다. 값이 존재하지만 소비자가 없으면
   `EXACT`가 될 수 없다.
6. v8 flat Detail 문서는 호환 입력으로만 읽고, source-exact S는 새 lossless format으로 publish한다.

### G02. Cascade emitter scheduler와 particle simulator

기존 `CEffectPlayback`에 S가 사용하는 모듈을 spawn/update 단계별로 추가한다.

- emitter delay/duration/loop와 모든 burst
- Lifetime, Spawn rate curve
- Location / Direct / Emitter Location
- Sphere / Cylinder / Circle surface spawn
- Velocity, Velocity Over Life, Acceleration curve
- Orbit, Vortex, Local Vector Field
- 3D Size, Size By Life, seeded distribution
- Rotation, Rotation Rate, Rotation Rate Over Life, Axis Lock
- Camera Offset
- Color/Alpha/Emissive 전체 curve
- SubUV sequence
- Particle Event Generator/Receiver
- deterministic random stream

모듈 적용 순서와 random draw 순서를 source module stack과 동일하게 고정한다. `Seek`, restart, loop, save/reload
후 같은 seed와 sample time은 같은 particle state를 생성해야 한다.

### G03. Mesh-backed particle 실행

S의 검은 `fx_sm_04.fm_s_swp_superweapon_01`을 사용하는 mesh-backed particle로 유지한다. 별도 수동 Model Cue로
대체하지 않는다.

- Mesh TypeData와 mesh material override
- per-particle 3D position/scale/rotation
- mesh rotation/rate/rate-over-life
- camera offset/orbit/velocity
- dynamic material parameter
- bone/world anchor와 project unit conversion

`EFFECT_PARTICLE_STATE`와 evaluated particle에 mesh transform 및 dynamic parameter를 추가하고,
`CEffectDocumentRenderer`가 기존 `CModel -> CMaterial` 경로로 각 particle mesh를 그린다. 원본 0.01 단위
변환은 변환기 상수가 아니라 source coordinate contract로 이름을 부여하고, position/velocity/size에 중복 적용되지
않는 단위 테스트를 둔다.

### G04. Material Instance closure와 shader 실행

1. Material Instance parent chain을 root Material까지 재귀적으로 닫는다.
2. Texture/Scalar/Vector parameter, static switch, override 여부와 상속 결과를 lossless recipe에 저장한다.
3. parent Material expression 중 S가 사용하는 Panner, Rotator, Fresnel, noise, distortion, dissolve,
   emissive 조합을 typed shader template/node contract로 변환한다.
4. blend mode, two-sided, depth test/write, soft particle, render alignment를 보존한다.
5. texture parameter가 없다는 이유로 `fx_a_blankwhite_01`을 연결하지 않는다. procedural material이면 해당
   expression을 실행하며, closure가 없으면 `MISSING_RESOURCE`로 publish를 거부한다.
6. DMC01 Material Instance의 상속 결과와 runtime constant/texture binding을 receipt에서 필드별 대조한다.

### G05. Particle 밖 presentation channel

원본 notify와 renderer가 Particle이 아닌 항목은 별도 typed cue로 실행한다.

- Point/particle Light 4
- RGB Noise 1
- Zoom Blur 1
- ViewShake 2
- assetless Effect notify 2
- Sound occurrence
- animated Model cue가 있는 다른 effect의 공통 경로

각 cue는 Effect timeline의 동일 clock을 소비한다. screen-space post를 임의 sprite로 만들거나 Light를 emissive
particle로 대신해 `EXACT` 처리하지 않는다. 제품 gameplay authority와 분리된 presentation service가 cue를
stage/stop하고 Effect document load 실패 시 부분 commit 없이 rollback한다.

### G06. Effect Tool source-aware 편집 UX

Effect Tool을 `Skill → Occurrence Group → Emitter → Module` 계층으로 확장한다.

- 원본 값: read-only
- runtime 평가 값: 현재 sample time 기준
- mapping status와 이유
- Authored override
- 원본 값으로 되돌리기
- group/element solo
- occurrence/emitter 시작 시각 seek
- 전체 scale/yaw/direction/speed multiplier
- Apply to memory와 Save required 상태 분리
- dirty 문서 전환 시 Save/Discard/Cancel

Particle layer 수는 `Particle Layers`, max particle 합은 `Budget`, mesh-backed 수는 별도 표시한다. 선택된 것이
완성 Effect인지 group인지 emitter인지 module인지 Detail 상단에 stable ID와 함께 명시한다.

### G07. Publisher와 coverage fail-closed

`Publish-Effects.ps1`이 다음 순서로 실행한다.

```text
parse Imported Recipe
→ validate source IDs, distributions, resources
→ parse Authored Override
→ validate override target/revision
→ stage merged runtime document
→ compile shader/material bindings
→ compute field-level coverage
→ commit only when policy passes
```

S exact profile은 approximation, unsupported, missing resource, fallback, unused source field가 하나라도 있으면
publish를 실패시킨다. 이전 runtime 문서는 유지한다. receipt는 숫자 합계뿐 아니라 source field → runtime
consumer mapping을 기록한다.

### G08. S 재생성, 검증, PNG A/B

1. source receipt와 external module closure를 재생성한다.
2. lossless Imported Recipe를 생성한다.
3. 필요한 Authored override가 없는 identity 상태로 먼저 publish한다.
4. fixed-step playback으로 원본 landmark와 particle state를 검사한다.
5. Debug/Release build, Effect pipeline, Client smoke, ProjectAudit를 통과한다.
6. 그 뒤에만 `C:\Users\user\Desktop\로스트아크이펙트이미지\차원술사\차원술사_S00.png`부터
   `차원술사_S06.png`까지를 고정 카메라 A/B로 사용한다.
7. A/B에서 발견한 renderer 차이는 source mapping 결함과 renderer calibration으로 분류한다. source 값을
   바꾸지 않고 해결 가능한 exposure/bloom/blend 차이는 renderer profile로 닫고, 의도적인 보정만 stable
   Authored override로 기록한다.

## 7. 주요 변경 파일

### Animation과 gameplay

- `Tools/CharacterAnimationIntake/build_actorx_fbx.py`
- animation cook/inspection 경로와 대응 test
- `Client/Private/Character.cpp`
- `Client/Private/Effect_Tool.cpp`
- `Data/Balance/PlayerSkills.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents`

### Effect document와 runtime

- `Client/Public/Effect_AuthoringDocument.h`
- `Client/Public/Effect_Distribution.h` 신규
- `Client/Private/Effect_Distribution.cpp` 신규
- `Client/Public/Effect_Playback.h`
- `Client/Private/Effect_Playback.cpp`
- `Client/Public/Effect_DocumentRenderer.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Public/Effect_DocumentCodec.h`
- `Client/Private/Effect_DocumentCodec.cpp`
- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`
- effect shader pass files
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

신규 `Effect_Distribution.h/.cpp`는 별도 runtime이 아니라 기존 playback이 source distribution을 평가하는 공용
구성요소다. 물리 폴더를 정본으로 프로젝트와 filter에 필요한 항목만 등록한다.

### Extractor, publisher, test

- `Tools/LevelPlacementExtractor/extract_ue3_particle_module_closure.py`
- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Tools/LevelPlacementExtractor/extract_umodel_material_dependencies.py`
- 대응 Python tests
- `Tools/EffectPipeline/Publish-Effects.ps1`
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1`

## 8. 자동 검증 계획

### 8.1 Parser와 recipe

- Constant/Uniform/Curve/UniformCurve/seeded round-trip
- 모든 key, tangent, interpolation 순서 보존
- duplicate stable ID, invalid path, non-finite value, 잘못된 version 거부
- source occurrence 28개 accounting
- external module 1,647/1,647
- emitter partition 200/200
- material/resource closure 140/140, 210/210

### 8.2 Playback와 renderer

- fixed seed + fixed time particle state snapshot
- emitter delay/duration/loop/burst 경계
- local/world space와 0.01 unit 변환
- 3D mesh transform/rotation/size/camera offset
- dynamic parameter와 material inheritance 결과
- Light/Post/Shake cue start/stop/seek
- seek/restart/loop/save/reload 결정성

### 8.3 Pipeline failure

- invalid source version
- unresolved override target
- unsupported module
- missing material parent/resource
- white fallback 또는 default tuning 상태
- shader stage 중간 실패 rollback
- publish 실패 시 기존 runtime 문서 유지

### 8.4 제품 회귀

1. Engine x64 Debug/Release
2. `UpdateLib.bat` Debug/Release
3. Shared/NetworkProtocolHarness Debug/Release 및 실행
4. Server Debug/Release와 `Server.exe --contract-test`
5. Client x64 Debug/Release
6. Client Effect Tool S load/play/seek/solo/apply/save/reload smoke
7. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
8. `git diff --check`

## 9. S 원본 복원 완료 게이트

```text
Animation source rate                         EXACT
Body duration                                 5.000s
Gameplay hit                                  1.730s
Notify occurrence accounting                  28/28
External module closure                       1,647/1,647
Emitter partition execution                   200/200
Material binding closure                      140/140
Runtime resource binding                      210/210
Approximation                                 0
Unsupported                                   0
Missing resource                              0
Default needs tuning                          0
Fallback white texture                        0
Timeline landmark drift                       ≤ 1ms
Deterministic seek/restart/save/reload         PASS
```

이 게이트 통과 전에는 “S 원본 복원 완료”로 보고하지 않는다. PNG A/B와 수동 화면 검증은 이 게이트 이후의
렌더러 정합성 단계로 별도 기록한다.

## 10. 단계별 산출물

| 단계 | 산출물 | 완료 증거 |
|---|---|---|
| G00 | source-rate WModel, timing 교정 | 5.0s/1.730s 검사 |
| G01 | lossless recipe와 codec | round-trip/field coverage |
| G02 | Cascade simulator 확장 | module fixed-step tests |
| G03 | mesh-backed particle | transform/material state snapshot |
| G04 | Material Instance closure | 140/140, 210/210 |
| G05 | Light/Post/Shake/Sound cue | typed cue timeline tests |
| G06 | source-aware Effect Tool | load/select/seek/solo/apply/save smoke |
| G07 | exact publisher | fail-closed/rollback tests |
| G08 | S exact publish와 A/B | exact gate + 수동 renderer 결과 |

구현은 각 G의 코드, 데이터, test, 대응 RESULT 갱신을 같은 검증 단위로 묶는다. 한 단계가 approximation을
남기면 다음 단계에서 완료로 덮지 않고 coverage에 미완료로 유지한다.
