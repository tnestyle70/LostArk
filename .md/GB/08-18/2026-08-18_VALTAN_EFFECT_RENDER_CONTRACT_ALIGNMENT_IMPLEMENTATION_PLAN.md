# 2026-08-18 발탄 Effect 렌더 계약을 기존 4캐릭터와 동일하게 맞추는 구현 계획

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`

발탄 99개 Product Effect는 문서·cue·catalog 계약은 4캐릭터와 같지만, Element가 실제로
화면에 그려지는 계약 세 가지가 4캐릭터 경로와 다르게 저작됐다. 이 계획은 그 세 가지를
4캐릭터가 이미 쓰고 있는 규칙으로 되돌린다.

새 렌더 경로, 새 shader, 새 문서 schema를 만들지 않는다. 전부 기존 계약에 맞추는 작업이다.

---

## G00. 실측 근거

### G00.1 사용자 관찰

```text
증상 1  Effect Tool에서 애니메이션 재생 프레임과 Effect 재생 프레임이 서로 독립적으로 흐른다.
        4캐릭터는 같은 시계로 움직인다.
증상 2  Mesh가 화면을 덮을 만큼 거대하게 나온다.
```

### G00.2 저장소 실측

`Data/Effects/Authored/*.effect.json` 전량과 `Client/Private/Effect_Tool.cpp` 현재 코드를 직접 집계했다.

```text
mesh 반송 Element의 유효 배율          modelPreScale x transform.scale
  lancemaster                          (absent -> 1.0) x 0.01   또는  0.01 x 1.0
  warlord / dimensionmaster / artist   동일
  valtan                               (absent -> 1.0) x 1.0     <- 1,942개 전부

sprite particle startSize 중앙값
  4캐릭터   1.0 ~ 2.0
  valtan     1.0            <- 정상 범위. 문제는 mesh 전용이다.

sourceMaterialPath 접미사 -> renderProfile
  4캐릭터   _tr -> alpha_*      4,444 / 4,458   (99.7%)
            _ad -> additive_*   2,966 / 2,976   (99.7%)
  valtan    _tr -> alpha        1,818
            _ad -> alpha        1,198           <- 전부 alpha로 평탄화

Effect Tool 애니메이션 시계
  4캐릭터   m_SynchronizedAnimationClips 에 clip을 stage
            -> Seek_SynchronizedAnimationSequence(m_fPreviewTimeSeconds) 가 매 프레임 구동
  420633 canary  같은 방식 (Effect_Tool.cpp:20754)
  99개 stage 행  Play_ValtanStageClip 이 Reset_SynchronizedAnimationSequence() 로 목록을
                 비운 뒤 Start_Animation + Set_AnimPaused(false)  <- 자유 재생
```

### G00.3 원인 위치

```text
증상 2  Tools/EffectPipeline/build_valtan_stage_effects.py:51
          SEED_SCALE = 1.0
        같은 파일 :261
          detail["transform"]["scale"] = [SEED_SCALE, SEED_SCALE, SEED_SCALE]
        modelPreScale 은 한 번도 쓰지 않는다. 두 배율 모두 1.0 이므로 UE3 cm 단위의
        mesh 가 m 단위 world 에 100배로 들어간다.

blend    같은 파일 :255-256
          material["sourceMaterialPath"] = material_path
        renderProfile / templateId / sourceProfile 을 한 번도 대입하지 않아
        skeleton 기본값 alpha_two_sided_depth_read 가 3,106개에 그대로 남았다.

증상 1  Client/Private/Effect_Tool.cpp:9576
          Reset_SynchronizedAnimationSequence();
        직후 Start_Animation / Set_AnimPaused(false).
```

### G00.4 원본 데이터로 복원할 수 없는 것

`Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json`의 `graph.resourceBindings`는
`role: material` 과 `role: mesh` 두 종류만 가진다. 4캐릭터가 쓰는
`Data/Effects/Imported/<Class>/Modules/`의 `ParticleModuleSize`, `ParticleModuleLifetime`
같은 emitter module 분포가 발탄 추출본에는 존재하지 않는다.

```text
복원 가능  material objectPath 접미사 _tr / _ad          -> blend
           mesh objectPath 와 UE3 cm 단위 계약           -> modelPreScale
복원 불가  emitter 별 initial size / lifetime / spawn rate
           one-sided / two-sided 구분
```

따라서 이 계획은 blend 와 mesh 단위 두 가지만 원본 근거로 복원하고, 크기·수명·개수는
계속 손 튜닝 시작값으로 남긴다. 이를 "원본 Cascade 완전 복원"이라고 기록하지 않는다.

---

## G01. Valtan Element 렌더 계약을 4캐릭터 규칙으로 정렬

### G01.1 목표와 종료 증거

```text
목표  발탄 Element 가 4캐릭터와 같은 blend 와 같은 mesh 단위로 그려진다.
종료  _ad 원본 Element 의 renderProfile 이 additive_two_sided_depth_read 다.
      mesh 반송 Element 전부가 detail.mesh.modelPreScale = 0.01 을 가진다.
      Publish-Effects.ps1 Validate/Publish 가 196 entries 로 통과한다.
```

### G01.2 수정 파일

```text
Tools/EffectPipeline/build_valtan_stage_effects.py
  생성 규칙의 정본. 여기를 고치지 않으면 다음 재시딩에서 같은 결함이 돌아온다.

Data/Effects/Authored/effect.valtan.*.effect.json   99개
  이미 사용자가 Effect Tool 에서 손 튜닝한 값이 들어 있을 수 있으므로
  전체 재시딩으로 덮어쓰지 않는다. 두 필드만 제자리 치환한다.

Tools/EffectPipeline/test_build_valtan_stage_effects.py
  새 계약 두 개를 실행형으로 고정한다.
```

### G01.3 blend 규칙

4캐릭터가 이미 쓰는 규칙을 그대로 옮긴다. `sourceMaterialPath`의 마지막 `_` 뒤 토큰이
판정 근거다.

```text
_ad  -> additive_two_sided_depth_read
_tr  -> alpha_two_sided_depth_read
그 외 (_ma, _ts, 접미사 없음) -> alpha_two_sided_depth_read
```

one/two-sided 는 발탄 추출본에 근거가 없으므로 전부 `two_sided` 를 유지한다. 4캐릭터가
one-sided 를 쓰는 근거는 module 데이터이며 발탄에는 그 입력이 없다.

`build_floor_lane_guides` 와 `build_arena_wipe_guide` 가 만드는 저작 가이드 8개는
`sourceMaterialPath` 가 빈 문자열인 project-authored 행이므로 규칙 대상이 아니고 현재
alpha 를 유지한다.

### G01.4 mesh 단위 규칙

`detail.mesh.modelPreScale` 은 `Effect_DocumentCodec.cpp:4089` 의 optional float 이고
`Effect_AuthoringDocument.h:457` 의 기본값이 `1.f` 다. `Effect_DocumentRenderer.cpp:4128` 이
`0 < value <= 100` 을 검증한다.

```text
mesh 를 반송하는 Element    detail.mesh.modelPreScale = 0.01
                            detail.transform.scale 은 1.0 그대로 둔다.
mesh 가 없는 Element        modelPreScale 을 쓰지 않는다.
```

4캐릭터는 `modelPreScale = 0.01` 과 `transform.scale ~= 0.01` 두 방식을 혼용하지만 유효
배율은 같은 0.01 이다. 발탄은 `transform.scale` 을 사용자가 Effect Detail 에서 직접
조절하는 값으로 남겨야 하므로 단위 보정은 `modelPreScale` 쪽에 둔다.

### G01.5 기존 99개 문서 적용 방식

`build_valtan_stage_effects.py` 에 `--migrate-existing` 모드를 추가한다. 이 모드는 원본
graph 를 다시 읽지 않고 `Data/Effects/Authored/effect.valtan.*.effect.json` 을 열어
아래 두 필드만 바꾼 뒤 같은 atomic writer 로 되쓴다.

```text
읽는 것    document["elements"][*]["material"]["sourceMaterialPath"]
           document["elements"][*]["resources"][*]["slotId"] == "meshModel"
바꾸는 것  material["renderProfile"]
           detail["mesh"]["modelPreScale"]
보존하는 것 그 외 모든 필드. id, displayName, groupId, resources, unboundResources,
           transform, color, uv, timing, particle, trail, decal 값 전부.
제외        PROTECTED_ASSET_IDS 의 420633 canary
```

같은 입력에 두 번 실행해도 결과가 같은 idempotent 연산이며, 사용자가 이미 저장한
`effect.valtan.parry.stance` 같은 문서의 손 튜닝을 잃지 않는다.

### G01.6 검증

```powershell
python Tools/EffectPipeline/build_valtan_stage_effects.py --migrate-existing --write
python -m unittest Tools.EffectPipeline.test_build_valtan_stage_effects -v
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish
```

기대: unittest 7 tests OK, Validate/Publish 196 entries PASS.

### G01.7 구현 중 추가로 확인된 sprite billboard 규칙

사용자가 "카메라 각도에 따라 sprite particle 이 사라진다"고 보고해 같은 계열의 세 번째
결함을 찾았다. 범위에 포함한다.

`Effect_DocumentRenderer.cpp:15457` 은 `Detail.Particle.bBillboard` 가 참일 때만
`Make_ParticleSpriteWorld()` 로 월드 행렬을 카메라 정면으로 다시 만든다. 거짓이면 평면
쿼드가 고정된 월드 방향을 유지하므로 옆에서 보면 두께가 0이 되어 사라진다.

```text
4캐릭터 실측 (예외 0건)
  mesh 반송 particle  1,284개 전부 billboard = false
  sprite particle     3,232개 전부 billboard = true

발탄
  mesh 반송 particle  1,942개 false            (우연히 일치)
  sprite particle     1,158개 false            <- 결함
                          1개 true             (손으로 만진 1개)
```

원인은 앞의 두 결함과 같다. 시더가 `skeleton["elements"][0]` 인 mesh 용 템플릿을 sprite
Element 에도 deep-copy 하고 종류별로 덮어쓰지 않았다. 스켈레톤에 이미
`valtan.test.sprite_particle` (billboard = true) 템플릿이 있으나 사용되지 않았다.

```text
규칙   detail.particle.billboard = (mesh 를 반송하지 않으면 true, 반송하면 false)
적용   build_element() 와 --migrate-existing 양쪽
대상   kind == "particle" 인 Element 만. 저작 가이드 8개는 kind == "decal" 이라 제외된다.
```

---

## G02. Effect Tool 발탄 stage clip 을 Effect 시계에 종속시킨다

### G02.1 목표와 종료 증거

```text
목표  발탄 stage 행의 Open/Play 가 4캐릭터 Product Play 및 420633 canary 와 같은
      단일 시계를 쓴다.
종료  Play_ValtanStageClip 이후 m_SynchronizedAnimationClips 가 비어 있지 않고,
      Sample Time 슬라이더와 Timeline 이 모델 포즈를 함께 움직인다.
```

### G02.2 수정 파일

```text
Client/Private/Effect_Tool.cpp
  Play_ValtanStageClip 한 함수만 바꾼다. 헤더 선언과 호출자는 그대로다.
```

### G02.3 변경 계약

기존 -> 변경

```text
Reset_SynchronizedAnimationSequence();              -> 삭제
pModel->Start_Animation(clip, m_bPreviewLoop);         유지
pModel->Set_AnimationSpeed(1.f);                       유지
pModel->Set_AnimPaused(false);                         유지
                                                    -> 추가
m_SynchronizedAnimationClips = { { clip, 0u, 1.f } };
m_iSynchronizedAnimationClipIndex = 0u;
m_iSynchronizedAnimationTargetGeneration =
    CAnimationTargetService::Resolve_TargetGeneration();
```

`Start_Animation` 이 실패하면 `Reset_SynchronizedAnimationSequence()` 로 목록을 되돌리고
기존 상태를 유지한다. 이는 `Effect_Tool.cpp:20760` 의 canary 경로가 이미 쓰는 실패 처리와
같다.

### G02.4 호출 흐름

```text
Render_ValtanAuthoringOpenButton
  -> Valtan Model View target stage
  -> Play_ValtanStageClip(strRuntimeClipName)
       m_SynchronizedAnimationClips 에 stage clip 한 개 commit
  -> 이후 매 프레임
       Update_SynchronizedAnimationSequence / Seek_SynchronizedAnimationSequence(
           m_fPreviewTimeSeconds)
     가 Effect preview 시계로 모델 포즈를 seek 한다.
```

### G02.5 검증

```powershell
MSBuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64
Tools\ClientFrontendHarness\Bin\Debug\ClientFrontendHarness.exe --valtan-pattern-effects-fast
```

기대: Client Debug link 성공, harness `failures : 0`.

화면 판정은 사용자 전용이다. 에이전트는 빌드와 harness 까지만 수행하고 사용자가 누를
경로를 보고한 뒤 멈춘다.

---

## G03. 통합 검증

```text
python -m unittest Tools.EffectPipeline.test_build_valtan_stage_effects -v
Publish-Effects.ps1 -Mode Validate / -Mode Publish
Publish-GameplayBalance.ps1 -Mode Validate
Client x64 Debug, Release
ClientFrontendHarness --valtan-pattern-effects-fast
ClientFrontendHarness --character-select-valtan-prewarm-fast
ClientFrontendHarness --effect-incremental-prewarm-fast
git diff --check
```

Server, Shared, Encounter, cue 문서, catalog row 수는 이 작업에서 바뀌지 않는다.
따라서 Server contract test 는 회귀 확인 목적으로만 재실행한다.

---

## G04. 이 계획이 하지 않는 것

```text
effect.source_material 승격
  4캐릭터는 절반가량이 복원된 UE3 material graph 에 붙어 UV 흐름·디졸브·왜곡을 실행한다.
  발탄은 3,106 / 3,109 가 sourceProfile disabled 다. 승격하려면 발탄 material graph
  복원 상태를 먼저 확정해야 하므로 별도 수직 슬라이스로 남긴다.

emitter 크기·수명·개수의 원본 복원
  발탄 추출본에 module 분포가 없다. 복원하려면 추출 단계에서 ParticleModuleSize /
  ParticleModuleLifetime 을 먼저 확보해야 한다.

one-sided / two-sided 구분
  같은 이유로 근거가 없다. 전부 two_sided 를 유지한다.

217개 unboundResources 재매핑
  8슬롯을 넘긴 96개 Element 의 초과 텍스처다. 의미별 수동 재매핑이 필요하다.
```

이 네 항목은 이번 변경으로 좋아지지 않으며 RESULT 의 남은 경계에 그대로 기록한다.
