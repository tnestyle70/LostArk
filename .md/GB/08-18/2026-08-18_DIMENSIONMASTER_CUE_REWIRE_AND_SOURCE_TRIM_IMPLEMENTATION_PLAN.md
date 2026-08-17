# 2026-08-18 차원술사 cue 재배선과 Source Trim 확장 구현 계획

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
선행: [발탄 Effect 렌더 계약 정렬](2026-08-18_VALTAN_EFFECT_RENDER_CONTRACT_ALIGNMENT_RESULT.md)

사용자 요청 7건을 데이터 재배선, 저작 계약 확장, 렌더 진단 세 갈래로 나눈다.
사용자 결정은 아래 G00.2에 그대로 기록한다.

---

## G00. 실측과 사용자 결정

### G00.1 현재 상태

```text
DimensionMaster animevents (effectref=asset)
  battle_1_01 -> ...2050010.ba1.unified
  battle_1_02 -> ...2050010.ba2.unified
  battle_1_03 -> ...2050010.ba3.unified
  battle_1_04 -> ...2050010.ba4.unified
  overslash_02 -> ...2050160.clip2.unified
  overslash_03 -> ...2050160.clip3.unified
  overslash_04 -> ...2050160.clip4.unified
  willowrend    -> ...2050210.unified          startms=0, 1행
  dimensionprison -> ...2050500.unified

skillbindings clips
  2050010  4 stage, stage 당 1 clip
  2050160  overslash_01 / 02(playMs 250) / 03(250) / 04(250) / 05
  2050210  willowrend 단일 clip. SUPERARMOR endms=1700
  2050500  dimensionprison 단일 clip

Source Trim (Effect_Tool.cpp:7220)
  SourceRecipe 를 가진 Element 에서만 표시된다.
  EFFECT_PARTICLE_SOURCE_SCALE_DESC 는 fCount / fSize / fLifeTime 3개다.

fx_a_fragment_002 를 쓰는 Element
  effect.dimensionmaster.skill.2050100.unified
  id authored.source-particle.6f1230cc5a961f5d5475e151
  kind particle, particle.billboard = true
  detail.transform.rotationDegrees = [0, 216.5, 0]
  detail.transform.revolutionDegreesPerSecond = [0, 0, 0]
  detail.sprite.billboardRollDegrees = 0

차원 감옥 T (2050500.unified)
  element 61 = particle 59 + decal 2
  mesh 반송 22, sourceProfile 활성 59
```

### G00.2 사용자 결정

```text
BA        battle_1_01 / 02 / 04 를 ba1.unified 로 연결하고 03 만 ba3.unified 유지
E         clip 4 의 Effect 를 사용한다
A         0.25초에서 시작해 0.25초 간격으로 4회. 250 / 500 / 750 / 1000 ms
Source Trim  가능한 값 전부 확장
T         summon 모델이 셰이더 경로를 타면 모델 경계가 잘린다. 캐릭터 애니메이션
          셰이더로 그릴 때는 정상이었다. 가능하면 그 경로로, 또는 summon 을 별도 shader
          분기로 태운다. 이전 수정 시도는 전부 실패했다.
```

### G00.3 rotation 이 안 먹는 실제 이유

`Effect_DocumentRenderer.cpp:1414` 의 `Make_ParticleSpriteWorld()` 는 billboard particle 의
월드 행렬을 카메라 기준으로 **다시 만든다.** 따라서 `detail.transform.rotationDegrees` 는
billboard particle 에서 원리상 버려진다. 실제로 적용되는 회전은 하나뿐이다.

```text
Effect_Playback.cpp:5064
  Evaluated.fSpriteRotationDegrees =
      Particle.vRotationDegrees.z + Element.Detail.Sprite.fBillboardRollDegrees;
```

즉 회전은 `sprite.billboardRollDegrees`(저작 상수)와 source rotation module 이 만드는
per-particle `vRotationDegrees.z` 두 곳에서만 온다. fragment_002 Element 는 둘 다 0이고
저작자는 `transform.rotationDegrees` 를 216.5로 올려 두었기 때문에 화면이 반응하지 않는다.

Effect Detail 이 billboard particle 에서도 `transform.rotationDegrees` 를 편집 가능하게
보여 주는 것이 오해의 원인이다.

---

## G01. 차원술사 cue 재배선

`Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents` 만 바꾼다.
Effect 문서, skillbindings, PlayerSkills 는 건드리지 않는다.

```text
battle_1_01  payload  ...ba1.unified   (유지)
battle_1_02  payload  ...ba2.unified -> ...ba1.unified
battle_1_03  payload  ...ba3.unified   (유지)
battle_1_04  payload  ...ba4.unified -> ...ba1.unified

overslash_02 payload  ...clip2.unified -> ...clip4.unified
overslash_03 payload  ...clip3.unified -> ...clip4.unified
overslash_04 payload  ...clip4.unified   (유지)

willowrend   startms=0 1행 -> startms 250 / 500 / 750 / 1000 4행
             payload 는 네 행 모두 ...2050210.unified
```

`ba2 / ba4 / clip2 / clip3` 문서는 삭제하지 않는다. animevents 가 더 이상 가리키지 않으면
publisher 가 runtime catalog 에서 자동으로 빠지므로 catalog row 수가 줄어든다. 저작
문서로는 남아 Effect Tool 에서 계속 열 수 있다.

검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish
```

기대: catalog entry 수가 196에서 줄고 실패 0. A 는 같은 clip 에 4개 cue 가 붙는다.

---

## G02. Source Trim 확장

### G02.1 확장 대상

`EFFECT_PARTICLE_SOURCE_SCALE_DESC` 에 SourceRecipe 가 소유해 Effect Detail 에서 직접 고칠
수 없는 축을 추가한다.

```text
fCount      기존. spawn rate, burst, 상한
fSize       기존. start size
fLifeTime   기존. particle lifetime
fSpeed      신규. 초기 속도와 가속도 배율
fRotation   신규. per-particle 회전 속도 배율
fAlpha      신규. 색 alpha 배율
fSpawnDelay 신규. spawn 시작 지연 배율
```

전부 `1.0` 이 기본이며 `Is_Default()` 는 일곱 값이 모두 1.0 일 때 참이다.

### G02.2 수정 파일

```text
Client/Public/Effect_AuthoringDocument.h
  struct 에 네 필드와 Is_Default() 갱신

Client/Private/Effect_DocumentCodec.cpp
  Read_ParticleSourceScale 에 optional 네 필드 추가. 없으면 1.0.
  직렬화는 기존 sourceScale 블록에 네 키를 추가.
  기존 문서는 세 키만 있어도 그대로 읽힌다.

Client/Private/Effect_Playback.cpp
  기존 fCount / fSize / fLifeTime 적용 지점 옆에서 네 배율을 적용한다.

Client/Private/Effect_Tool.cpp
  Source Trim 블록에 DragFloat 네 개 추가.
```

### G02.3 sprite particle 이 Effect Detail 에서 안 잡히는 이유

`bSourceParticleControlsReadOnly` 가 참이면 spawn rate, size, lifetime 입력이 read-only 가
된다. SourceRecipe 가 값의 정본이기 때문이며 Source Trim 은 그 위에 곱하는 저작 답이다.
따라서 "튜닝을 못하는 구조"가 맞고, 해결은 입력을 여는 것이 아니라 Trim 축을 늘리는 것이다.

---

## G03. billboard particle 회전

### G03.1 계약

billboard particle 에서 회전의 정본은 `sprite.billboardRollDegrees` 와 source rotation
module 이다. `transform.rotationDegrees` 는 적용되지 않는다.

```text
추가  detail.sprite.billboardRollDegreesPerSecond   신규 optional, 기본 0
      Evaluated.fSpriteRotationDegrees 에
        + fBillboardRollDegreesPerSecond * (파티클 경과 시간)
      을 더해 실제로 도는 회전을 만든다.

UI    Effect Detail 의 Sprite 블록에 Roll Degrees / Roll Degrees Per Second 를 노출하고,
      billboard 가 켜진 particle 에서 transform rotation 이 적용되지 않는다는 문구를 남긴다.
```

`fx_a_fragment_002` Element 는 `billboardRollDegreesPerSecond` 로 회전시키고,
Source Trim 의 `fRotation` 이 그 위에 곱해진다.

### G03.2 수정 파일

```text
Client/Public/Effect_AuthoringDocument.h      EFFECT_SPRITE_DESC 에 필드 1개
Client/Private/Effect_DocumentCodec.cpp       optional read/write
Client/Private/Effect_Playback.cpp            fSpriteRotationDegrees 계산에 합산
Client/Private/Effect_Tool.cpp                Sprite 블록 UI와 안내 문구
```

---

## G04. 차원 감옥 T 모델 경계 잘림 진단

사용자는 이전에 여러 번 실패했다고 했다. 따라서 이 G 는 경로 교체를 먼저 하지 않고
잘림의 실제 원인을 수치로 특정하는 것부터 시작한다.

```text
후보 1  effect mesh 경로의 bounding 계산이 model 실제 AABB 를 쓰지 않아 frustum culling 이
        모델 일부를 버린다.
후보 2  modelPreScale 과 transform scale 이 곱해진 뒤 near/far 또는 depth 범위를 벗어난다.
후보 3  renderProfile 의 one_sided 로 후면이 culling 된다. T 는 one_sided 51 / two_sided 10 이다.
후보 4  effect mesh 셰이더가 model 의 다중 mesh/material 중 첫 번째만 그린다.
```

진단은 다음 순서로 한다.

```text
1. T 의 mesh 반송 22개 Element 의 meshModel asset, modelPreScale, transform scale,
   renderProfile 을 표로 집계한다.
2. 해당 .wmodel 의 mesh 수와 material 수를 CModel 디코드 기준으로 확인한다.
3. Effect_DocumentRenderer 의 mesh draw 경로가 mesh 몇 개를 issue 하는지 확인한다.
```

원인이 후보 4 로 확정되면 mesh 전체를 순회하도록 고치는 것이 캐릭터 셰이더로 갈아타는
것보다 작고 안전하다. 원인이 확정되기 전에는 렌더 경로를 바꾸지 않는다.

이 G 는 진단 결과를 RESULT 에 수치로 남기고, 수정 범위가 확정되면 별도 변경 단위로 진행한다.

---

## G05. 발탄 확인 항목

사용자 질문에 대한 현재 사실을 RESULT 에 기록한다.

```text
애니메이션  Play_ValtanStageClip 이 stage clip 을 m_SynchronizedAnimationClips 에 commit 해
            Effect preview 시계가 포즈를 seek 한다. 4캐릭터 Product Play 와 같은 계약이다.
            보스 모델 자체는 MN_RPBF_01 + AnimSet 173 clip 을 CModel 로 로드한다.
렌더링      발탄 Effect 는 4캐릭터와 같은 CEffectPresentationService -> CEffectObject ->
            CEffectDocumentRenderer 경로를 쓴다. blend / modelPreScale / billboard 세 계약을
            4캐릭터와 동일하게 맞췄다.
리소스      99개 문서의 resource 참조 10,807개가 Client/Bin/Resources 에서 전부 해석된다.
publish     196 Effects published, exit 0.
```

---

## G06. 검증

```powershell
python -m unittest Tools.EffectPipeline.test_build_valtan_stage_effects
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish
MSBuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64
Tools\ClientFrontendHarness\Bin\Debug\ClientFrontendHarness.exe --valtan-pattern-effects-fast
Tools\ClientFrontendHarness\Bin\Debug\ClientFrontendHarness.exe --effect-incremental-prewarm-fast
git diff --check
```

화면 판정은 사용자 전용이다. 에이전트는 빌드와 harness 까지만 수행한다.
