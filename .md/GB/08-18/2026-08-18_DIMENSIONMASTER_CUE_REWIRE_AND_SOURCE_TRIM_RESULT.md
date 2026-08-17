# 2026-08-18 차원술사 cue 재배선과 Source Trim 확장 결과

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
대응 계획: [구현 계획서](2026-08-18_DIMENSIONMASTER_CUE_REWIRE_AND_SOURCE_TRIM_IMPLEMENTATION_PLAN.md)

사용자 요청 7건 중 5건을 구현했고, T 모델 잘림은 원인을 수치로 좁힌 진단 상태다.
화면 판정은 사용자 몫으로 남겼다.

---

## G01. 차원술사 cue 재배선 (완료)

`Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents` 만 바꿨다.
Effect 문서, skillbindings, PlayerSkills 는 건드리지 않았다.

```text
battle_1_01  ...ba1.unified      유지
battle_1_02  ...ba2 -> ba1.unified
battle_1_03  ...ba3.unified      유지
battle_1_04  ...ba4 -> ba1.unified

overslash_02 ...clip2 -> clip4.unified
overslash_03 ...clip3 -> clip4.unified
overslash_04 ...clip4.unified    유지

willowrend   startms=0 1행 -> startms 250 / 500 / 750 / 1000 4행
             네 행 모두 payload ...2050210.unified
```

헤더의 선언 행 수도 `1540 -> 1543` 으로 갱신했다. publisher 가 이 값을 검사한다.

`ba2 / ba4 / clip2 / clip3` 저작 문서는 삭제하지 않았다. animevents 가 더 이상 가리키지
않으므로 runtime catalog 에서만 빠졌고 Effect Tool 에서는 계속 열 수 있다.

```text
publish  196 Effects -> 192 Effects
```

---

## G02. Source Trim 확장 (완료)

`EFFECT_PARTICLE_SOURCE_SCALE_DESC` 를 3축에서 7축으로 늘렸다.

```text
fCount       기존   spawn rate / burst / 상한
fSize        기존   start size
fLifeTime    기존   particle lifetime
fSpeed       신규   spawn velocity.  vVelocity 와 vBaseVelocity 양쪽에 곱한다.
fRotation    신규   회전 속도.  vRotationRateScale 과
                    vSourceMeshRotationRateScale 에 곱한다.
fAlpha       신규   색 alpha.  vBaseColor.w 와 vColor.w
fSpawnDelay  신규   SourceRecipe.fEmitterDelaySeconds 배율
```

- `fSpeed` 와 `fRotation` 은 음수를 허용한다. 안쪽으로 모으거나 반대로 돌리는 저작이
  가능해야 하기 때문이다. 크기 상한은 기존 축과 같은 16이다.
- `fAlpha` 와 `fSpawnDelay` 는 0 이상 16 이하다.
- 네 필드 모두 codec 에서 optional 이므로 세 키만 가진 기존 문서가 그대로 읽힌다.
  `Is_Default()` 가 참이면 직렬화에서 블록 자체를 생략하는 기존 동작도 유지된다.

수정 파일

```text
Client/Public/Effect_AuthoringDocument.h    struct 4필드, Is_Default()
Client/Private/Effect_DocumentCodec.cpp     optional read, write, 범위 검증
Client/Private/Effect_Playback.cpp          spawn 시 4축 적용, emitter delay 배율
Client/Private/Effect_Tool.cpp              DragFloat 4개와 설명 문구
```

### sprite particle 을 Effect Detail 에서 못 고치는 이유

`bSourceParticleControlsReadOnly` 가 참이면 spawn rate, size, lifetime 입력이 read-only 가
된다. SourceRecipe 모듈 스택이 값의 정본이기 때문이며 Source Trim 이 그 위에 곱하는
저작 답이다. 구조가 맞고, 해결은 입력을 여는 것이 아니라 Trim 축을 늘리는 것이다.

---

## G03. billboard particle 회전 (완료)

### 원인

`Effect_DocumentRenderer.cpp:1414` 의 `Make_ParticleSpriteWorld()` 는 billboard particle 의
월드 행렬을 카메라 기준으로 다시 만든다. 따라서 `detail.transform.rotationDegrees` 는
billboard particle 에서 원리상 버려진다. 실제 회전은 한 곳에서만 온다.

```text
Effect_Playback.cpp
  Evaluated.fSpriteRotationDegrees =
      Particle.vRotationDegrees.z + Element.Detail.Sprite.fBillboardRollDegrees;
```

`fx_a_fragment_002` Element 는 둘 다 0이고 저작자가 `transform.rotationDegrees` 를 216.5로
올려 두었기 때문에 화면이 반응하지 않았다. 게다가 roll 입력은 standalone Sprite Element
에서만 노출되고 particle Element 에서는 보이지도 않았다.

### 변경

```text
EFFECT_SPRITE_DETAIL_DESC 에 fBillboardRollDegreesPerSecond 추가 (optional, 기본 0)
Evaluated.fSpriteRotationDegrees 에
  + fBillboardRollDegreesPerSecond * Particle.fAgeSeconds
Effect Detail
  Sprite Element 에 Roll Degrees Per Second 추가
  Particle Element 에서 billboard 가 켜져 있으면 Roll Degrees 와 Roll Degrees Per Second
  를 함께 노출하고, Transform rotation 이 적용되지 않는다는 문구를 남긴다.
```

이제 `fx_a_fragment_002` 는 Effect Detail 에서 Roll Degrees Per Second 로 회전시키고,
Source Trim 의 `Rotation x` 가 source rotation module 위에 곱해진다.

---

## G04. 차원 감옥 T 모델 잘림 (진단만)

경로를 바꾸기 전에 원인을 좁혔다. 계획대로 렌더 경로는 아직 교체하지 않았다.

```text
effect.dimensionmaster.skill.2050500.unified
  element 61 = particle 59 + decal 2
  mesh 반송 22개, 서로 다른 mesh 13종
  modelPreScale        22개 전부 0.01
  transform.scale      1.2 x 19,  1.0 x 3
  useModelMaterial     22개 전부 false
  renderProfile        additive_one_sided 13 + alpha_one_sided 9
                       -> 22개 전부 one_sided
  mesh 종류            cylinder, plane, square, helix, hemisphere, ring, sphere
```

가장 유력한 원인은 **one-sided backface culling** 이다. 22개가 모두 one_sided 이고 mesh 가
전부 닫힌 원기둥·반구·구·링 같은 형상이다. 카메라가 그 안쪽이나 측면을 볼 때 앞면이
culling 되어 형상의 경계가 잘린 것처럼 보인다. 캐릭터 애니메이션 셰이더로 그릴 때 정상으로
보였다는 관찰과도 일치한다. 그 경로는 모델 자체 material 의 양면 설정을 따르기 때문이다.

확인 방법은 22개 Element 의 renderProfile 을 `*_two_sided_depth_read` 로 바꿔 한 번 보는
것이다. 데이터 전용 변경이고 되돌리기 쉽다. 다만 이 값들은 4캐릭터 source 파이프라인이
넣은 값이라 임의로 바꾸면 source 근거에서 벗어나므로, 사용자 확인 뒤 적용 여부를 결정한다.

셰이더 경로 교체는 이 확인 이후에도 문제가 남을 때만 검토한다.

---

## G05. 발탄 확인 항목

```text
애니메이션  Play_ValtanStageClip 이 stage clip 을 m_SynchronizedAnimationClips 에 commit 해
            Effect preview 시계가 포즈를 seek 한다. 4캐릭터 Product Play 와 같은 계약이다.
            보스 모델은 MN_RPBF_01 + AnimSet 173 clip 을 CModel 로 로드한다.
렌더링      발탄 Effect 는 4캐릭터와 같은
            CEffectPresentationService -> CEffectObject -> CEffectDocumentRenderer 를 쓴다.
            blend / modelPreScale / billboard 세 계약을 4캐릭터와 동일하게 맞췄다.
리소스      99개 문서의 resource 참조 10,807개가 Client/Bin/Resources 에서 전부 해석된다.
            미해석 0건.
publish     192 Effects, 1 Component, visual-program sidecar published. exit 0.
```

---

## G06. 실행한 검증

```text
python -m unittest Tools.EffectPipeline.test_build_valtan_stage_effects
  7 tests / OK

Publish-Effects.ps1 -Mode Publish
  192 Effects / 1 Component / PASS

Client x64 Debug
  빌드와 link 성공. Client.exe 생성, runtime DLL 배포 완료.

ClientFrontendHarness (Debug 재빌드 후)
  --effect-incremental-prewarm-fast        failures : 0
  --valtan-pattern-effects-fast            failures : 0
  --character-select-valtan-prewarm-fast   failures : 0

git diff --check
  exit 0
```

harness 의 `Published Product Set Has 196 Runtime Members` 단정은 catalog 가 실제로 192로
줄었으므로 192로 갱신했다. 이는 G01 재배선의 의도된 결과다.

---

## G07. 남은 경계

```text
T 모델 잘림       one-sided 가설까지만 확정. 사용자 육안 확인 후 적용 여부 결정.
effect.source_material 승격   발탄은 여전히 3,106 / 3,109 가 sourceProfile disabled.
발탄 emitter 분포  추출본에 ParticleModuleSize / ParticleModuleLifetime 이 없어 복원 불가.
217 unboundResources   8슬롯 초과 텍스처. 수동 재매핑 필요.
visual PASS       크기·색·밀도·타이밍의 최종 판정은 사용자 육안 확인이다.
```
