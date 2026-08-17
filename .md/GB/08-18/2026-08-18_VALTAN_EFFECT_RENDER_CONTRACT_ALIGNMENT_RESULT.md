# 2026-08-18 발탄 Effect 렌더 계약 정렬 결과

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
대응 계획: [구현 계획서](2026-08-18_VALTAN_EFFECT_RENDER_CONTRACT_ALIGNMENT_IMPLEMENTATION_PLAN.md)

사용자가 발탄에서 관찰한 두 증상의 원인을 실측으로 특정하고, 발탄 Element 를 4캐릭터가
이미 쓰는 렌더 계약으로 정렬했다. 화면 판정은 사용자 몫으로 남겼다.

이 문서 작성 시점에 사용자의 `Client.exe` 와 `Server.exe` 가 실행 중이라 Effect publish 와
Client link 는 완료하지 못했다. 아래 `G03` 에 미완료로 분리해 기록한다.

---

## G00. 확정한 원인

```text
증상 1  애니메이션과 Effect 가 서로 다른 시계로 흐른다
원인    Effect_Tool.cpp Play_ValtanStageClip 이 Reset_SynchronizedAnimationSequence() 로
        동기 목록을 비운 뒤 Start_Animation + Set_AnimPaused(false) 를 호출했다.
        4캐릭터 Product Play 와 420633 canary 는 clip 을 m_SynchronizedAnimationClips 에
        stage 해서 Effect preview 시계가 포즈를 seek 한다.

증상 2  Mesh 가 거대하다
원인    build_valtan_stage_effects.py 의 SEED_SCALE = 1.0 이 transform.scale 에 들어가고
        detail.mesh.modelPreScale 은 한 번도 쓰이지 않아 유효 배율이 1.0 x 1.0 이었다.
        4캐릭터는 두 knob 중 하나로 0.01 을 적용한다. Cascade mesh 는 UE3 cm 이고
        런타임은 m 이므로 발탄 mesh 1,940개가 100배로 그려졌다.

증상 3  카메라 각도에 따라 sprite particle 이 사라진다
원인    Effect_DocumentRenderer.cpp:15457 은 Detail.Particle.bBillboard 가 참일 때만
        Make_ParticleSpriteWorld() 로 월드 행렬을 카메라 정면으로 다시 만든다. 거짓이면
        평면 쿼드가 고정 방향을 유지해 옆에서 보면 사라진다. 시더가
        skeleton["elements"][0] 인 mesh 용 템플릿(billboard=false)을 sprite 에도 복사해
        1,158개가 false 였다. 스켈레톤의 valtan.test.sprite_particle(billboard=true)
        템플릿은 쓰이지 않았다.

동반 결함  blend 평탄화
원인       같은 시더가 renderProfile 을 한 번도 대입하지 않아 skeleton 기본값
           alpha_two_sided_depth_read 가 남았다. 원본이 additive(`_ad`) 인 1,198개가
           alpha 로 그려졌다.
```

세 결함 모두 같은 계열이다. 시더가 skeleton 템플릿의 기본값을 Element 종류별로 덮어쓰지
않아 mesh 용 기본값이 sprite 에도, alpha 기본값이 additive 원본에도 남았다.

4캐릭터 실측 규칙은 예외 0건이다.

```text
particle.billboard   mesh 반송 1,284개 전부 false / sprite 3,232개 전부 true
```

`sprite particle` 의 `startSize` 중앙값은 발탄 1.0, 4캐릭터 1.0~2.0 으로 같은 범위였다.
크기 문제는 mesh 전용이며 sprite 는 손대지 않았다.

---

## G01. 구현 완료

### G01.1 시더 생성 규칙

`Tools/EffectPipeline/build_valtan_stage_effects.py`

- `resolve_render_profile()` 를 추가해 `sourceMaterialPath` 의 마지막 `_` 토큰으로 blend 를
  정한다. `_ad` 는 `additive_two_sided_depth_read`, 그 외는 `alpha_two_sided_depth_read` 다.
- `element_carries_mesh()` 를 추가해 `meshModel` 슬롯 보유 여부를 한 곳에서 판정한다.
- `build_element()` 가 `material["renderProfile"]` 을 대입하고, mesh 를 반송하는 Element 에만
  `detail["mesh"]["modelPreScale"] = 0.01` 을 쓴다. `transform.scale` 은 사용자가 Effect
  Detail 에서 조절하는 값이므로 1.0 그대로 둔다.
- `build_element()` 가 `detail["particle"]["billboard"] = mesh is None` 을 대입한다.
  sprite 는 카메라를 향하고 mesh 는 자기 방향을 유지한다.
- one-sided / two-sided 는 발탄 추출본에 근거가 없어 전부 two-sided 를 유지했다.

### G01.2 기존 99개 문서 적용

같은 파일에 `--migrate-existing` 모드를 추가했다. 원본 graph 를 다시 읽지 않고 이미 저작된
문서를 열어 `material.renderProfile` 과 `detail.mesh.modelPreScale` 두 필드만 바꾼다.

전체 재시딩을 쓰지 않은 이유는 실행 중이던 Effect Tool 세션이 이미 문서를 저장하고 있었기
때문이다. 실제로 작업 중 `effect.valtan.entrance-whirlwind.windup` 이 `Create Effect` 로 새로
생겼고, DimensionMaster 문서 하나가 계속 다시 저장됐다.

```text
1차 (blend + modelPreScale)   documents needing edit 94
2차 (billboard)               documents needing edit 88
각 차수 재실행                 documents needing edit 0   -> idempotent
```

적용 후 실측:

```text
_tr      -> alpha_two_sided_depth_read       1818
_ad      -> additive_two_sided_depth_read    1198
그 외     -> alpha_two_sided_depth_read         82
저작 가이드 -> alpha_two_sided_depth_read          8
mesh modelPreScale                          {0.01: 1940}
particle.billboard                          {sprite: True 1158, mesh: False 1940}
```

### G01.3 authoring-only 문서를 금지하던 두 단정 교정

`Create Effect` 로 만든 문서 하나 때문에 test 와 publisher 가 모두 실패했다. CLAUDE.md 는
authoring-only 문서를 허용하고 runtime catalog 에만 들어오지 않게 하는 계약이므로, 단정
쪽을 실제 불변식으로 좁혔다.

- `test_build_valtan_stage_effects.py`
  `authored == cues | canary` 등식을 `cues | canary ⊆ authored` 포함 관계로 바꾸고,
  cue 에 없는 문서는 catalog 에도 없어야 한다는 단정을 추가했다.
- `Tools/EffectPipeline/Publish-Effects.ps1`
  on-disk 문서 집합과 cue 집합의 완전 일치 요구를 두 조건으로 바꿨다.
  cue 에 문서가 없으면 실패(`withoutDocument`), catalog 에 실린 문서에 cue 가 없으면
  실패(`cataloguedWithoutCue`), catalog 에 없는 on-disk 문서는 authoring-only 초안으로 통과.

### G01.4 새 회귀 계약

`Tools/EffectPipeline/test_build_valtan_stage_effects.py` 에 두 test 를 추가했다.

- `test_blend_mode_follows_the_original_material_name_suffix`
  규칙 함수와 저작된 99개 문서 전체를 검사하고 additive 1,198 을 고정한다.
- `test_mesh_carriers_use_the_centimetre_to_metre_pre_scale`
  mesh 반송 Element 1,940개가 `modelPreScale = 0.01` 이고 비-mesh Element 에는 그 필드가
  없음을 고정한다.
- `test_sprite_particles_billboard_and_mesh_particles_do_not`
  sprite particle 1,158개가 `billboard = true`, mesh particle 1,940개가 `false` 임을
  고정한다.

### G01.5 Effect Tool 시계 동기화

`Client/Private/Effect_Tool.cpp` 의 `Play_ValtanStageClip` 만 바꿨다.

```text
기존   Reset_SynchronizedAnimationSequence();
       pModel->Start_Animation(clip, m_bPreviewLoop);

변경   m_SynchronizedAnimationClips = { { clip, 0u, 1.f } };
       m_iSynchronizedAnimationClipIndex = 0u;
       m_iSynchronizedAnimationTargetGeneration =
           CAnimationTargetService::Resolve_TargetGeneration();
       pModel->Start_Animation(clip, m_bPreviewLoop);
       실패 시 Reset_SynchronizedAnimationSequence() 로 되돌린다.
```

이후 `Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds)` 가 매 프레임 모델 포즈를
Effect preview 시계로 seek 한다. 이는 `Effect_Tool.cpp` 의 420633 canary 경로가 이미 쓰는
계약과 같다. 헤더 선언과 호출자는 바뀌지 않았다.

---

## G02. 실행한 검증

```text
python -m unittest Tools.EffectPipeline.test_build_valtan_stage_effects
  7 tests / OK   (기존 4 + 신규 3)

build_valtan_stage_effects.py --migrate-existing (재실행)
  documents needing edit 0   -> idempotent 확인

ClientFrontendHarness --valtan-pattern-effects-fast
  11 PASS / failures : 0
ClientFrontendHarness --effect-incremental-prewarm-fast
  8 PASS / failures : 0

Client x64 Debug 컴파일
  Effect_Tool.cpp 컴파일 성공. error 없음. 기존 C4819 warning 만 발생.

git diff --check
  exit 0
```

---

## G03. 실행하지 못한 검증

사용자의 `Client.exe`(PID 32960) 와 `Server.exe` 가 실행 중이다.

```text
Publish-Effects.ps1 -Mode Validate / Publish
  BLOCKED. Valtan cue/document 검사는 통과했으나 그 뒤
  Data/Effects/Authored/effect.dimensionmaster.skill.2050010.ba1.unified.effect.json 이
  publisher 실행 중에 계속 다시 저장돼 source pin 이 3회 연속 실패했다.
    205227 -> 205252 -> 205260 bytes
  이 문서는 이번 작업에서 건드리지 않았다. 실행 중인 Effect Tool 세션의 저장과의 경합이다.

Client x64 Debug / Release link
  BLOCKED. LNK1104: '..\Bin\Debug\Client.exe' 파일을 열 수 없습니다.
  실행 중인 Client.exe 가 출력물을 점유한다.

ClientFrontendHarness --character-select-valtan-prewarm-fast
  미실행. 위 두 항목이 끝난 뒤 함께 실행한다.
```

publish 전까지 `Client/Bin/DataFiles/Effect` 런타임 payload 는 마이그레이션 이전 상태다.
따라서 제품 런타임(Lobby > Valtan)에는 아직 이번 수정이 반영되지 않는다. Effect Tool 의
저작 문서 경로는 source 를 직접 읽으므로 Client 를 다시 빌드하면 먼저 확인할 수 있다.

---

## G04. 남은 경계

```text
effect.source_material 승격
  4캐릭터는 절반가량이 복원된 UE3 material graph 위에서 UV 흐름·디졸브·왜곡을 실행한다.
  발탄은 3,106 / 3,109 가 sourceProfile disabled 다. 이번 변경으로 좋아지지 않는다.

emitter 크기·수명·개수의 원본 복원
  Data/Effects/Imported/Valtan 의 graph.resourceBindings 는 role=material 과 role=mesh 만
  가진다. 4캐릭터가 쓰는 ParticleModuleSize / ParticleModuleLifetime 분포가 발탄 추출본에
  없다. 복원하려면 추출 단계를 먼저 확장해야 한다.

one-sided / two-sided
  같은 이유로 근거가 없어 전부 two-sided 다.

217개 unboundResources
  8슬롯을 넘긴 96개 Element 의 초과 텍스처다. 의미별 수동 재매핑이 필요하다.

visual PASS
  크기·색·밀도·궤적의 최종 판정은 사용자 육안 확인이다. 자동 PASS 로 기록하지 않았다.
```
