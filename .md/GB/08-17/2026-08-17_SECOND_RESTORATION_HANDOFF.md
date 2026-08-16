# 2026-08-17 2차 복원 인계 문서

새 세션은 이 문서를 먼저 읽는다. 배경은
`2026-08-17_TRACK_A_POSTMORTEM_AND_SECOND_RESTORATION_PLAN.md`,
전체 그림은 `2026-08-17_EFFECT_RESTORATION_UNIFIED_IMPLEMENTATION_PLAN.md`다.

branch: `feature/effect-tool-texture-kind-filter`
직전 커밋: `8dc2cde5`

## 1. 지금 어디까지 왔나

```text
26de16ec  중복 element 제거          7,861 -> 3,042   210.9 MB -> 84.9 MB
c602dde6  슬롯 abort 차단 + (src) 배지
4f8a3951  런타임 문서 compaction     122.2 MB -> 54.8 MB
e541f5e8  발탄 74 문서 생성          191 element
5814fb65  CValtanPatternTree
8dc2cde5  Track A 사후 분석과 2차 복원 계획
```

현재 element 3,042개의 상태다.

```text
1,909   sourceRecipe 가 재생 소유    <- 저작 수치가 무시된다. 이번 작업 대상
  234   material fail-closed
  899   저작 소유                    <- 지금 그대로 튜닝이 먹는다
```

## 2. 1번 작업이 정확히 무엇인가

세 가지를 **한 변경 단위**로 넣는다. 나누면 정보가 버려진다.

### 2.1 v14 스키마 확장

`Detail.Particle`에 두 블록을 추가하고 formatVersion을 14로 올린다.

```text
spawnShape
    kind            point | sphere | ring | box
    radius          sphere / ring 의 바깥 반경
    innerRadius     ring 의 안쪽 반경
    extents         box 의 반쪽 크기 [x,y,z]
    arcDegrees      ring / sphere 의 각도 범위. 360 이면 전체

initialVelocity
    mode            fixed | outward | inward | cone
    speed           [min, max]
    coneAngleDegrees
```

v13 문서는 두 블록이 없으면 `point` + `fixed`로 읽어 현재 동작을 그대로 유지한다.

### 2.2 소유권 이전

`sourceRecipe.enabled`를 true에서 false로 내린다.
그 순간부터 재생이 `Detail`을 읽는다. 즉 **튜닝이 먹기 시작한다.**

### 2.3 값 이식

그냥 내리면 element가 기본값으로 리셋되어 화면이 사라진다.
내리는 동시에 `sourceRecipe.modules`에서 읽을 수 있는 값을 `Detail`로 옮겨야 한다.

```text
particlemodulerequired    emitterdelay        -> Detail.Timing.fStartDelaySeconds
                          emitterloops        -> Detail.Timing 루프
particlemodulelifetime    lifetime            -> Detail.Particle.vLifeTimeSeconds
particlemodulesize        startsize           -> Detail.Particle.vStartSize
particlemodulespawn       spawnrate           -> Detail.Particle.fSpawnRatePerSecond
particlemodulecolor*      color               -> Detail.Color
particlemodulelocation*   스폰 형태            -> Detail.Particle.spawnShape      (v14)
particlemodulevelocity*   초기 속도            -> Detail.Particle.initialVelocity  (v14)
```

해석하지 못하는 모듈은 **건드리지 않는다.** `sourceRecipe`는 지우지 않고 근거로 보존한다.

### 2.4 왜 한 변경이어야 하는가

v14 없이 2.2를 먼저 하면 `location` / `velocity` 모듈을 받을 필드가 없다.
그러면 그 축은 이전 시점에 버려지고, 나중에 v14를 넣어도 원본 값은 이미 사라진 뒤다.
`sourceRecipe`를 보존하므로 이론상 재이식이 가능하지만, 두 번 하지 않는 편이 낫다.

## 3. 그래서 "전체 스킬 복원"이 실제로 어떤 절차인가

원본과 동일하게 만드는 것이 아니라, **모든 element를 손으로 만질 수 있는 상태로 만든 뒤
눈으로 맞추는 것**이다.

```text
[1] 기계가 하는 일  ── 1,909 element
      sourceRecipe.modules 를 읽어 Detail 로 옮기고 소유권을 내린다
      결과: 모든 element 가 원본에서 온 시작값을 갖고, 튜닝이 먹는 상태

[2] 사람이 하는 일  ── 스킬 단위
      Effect Tool 에서 스킬을 연다
      element 를 하나씩 Solo 로 보며
        너무 크다 / 작다      -> Detail.Transform.scale
        너무 밝다 / 어둡다    -> Detail.Color
        너무 많다 / 적다      -> Detail.Particle.maxParticles, burstCount
        방향이 틀렸다         -> Detail.Particle.initialVelocity
        위치가 틀렸다         -> Detail.Transform.position, actionCueAttachment
        필요 없다             -> Ctrl/Shift 로 마크 후 Delete
      Save Changes

[3] 확인
      Model View 에서 Play All
      제품 재생은 publish 후
```

[2]가 워로드에서 이미 하신 작업이다. 지금은 그것이 **Transform 축에서만** 가능했고,
[1]이 끝나면 **모든 축에서** 가능해진다.

### 3.1 규모

```text
1,909 element / 대략 50 스킬 = 스킬당 평균 38 element
중복 제거 후이므로 전부 서로 다른 시각 요소다
스킬 하나를 눈으로 맞추는 데 걸리는 시간이 전체 일정을 정한다
```

기계가 [1]을 끝내면 그다음은 전부 [2]다. 그래서 [1]의 값 이식 품질이
[2]의 작업량을 좌우한다. 이식이 정확할수록 손댈 것이 적다.

## 4. 구현 순서와 파일

### G1  v14 스키마 + 이식 + 소유권 이전

```text
Client/Public/Effect_AuthoringDocument.h    EFFECT_PARTICLE_DETAIL 에 두 struct 추가
Client/Private/Effect_DocumentCodec.cpp     v14 parse / serialize, v13 기본값 승격
Client/Private/Effect_Playback.cpp          spawnShape / initialVelocity 소비
Client/Private/Effect_Tool.cpp              Detail UI 두 블록
Tools/EffectPipeline/transplant_source_recipe.py   신규. 모듈 -> Detail 이식
Tools/EffectPipeline/Publish-Effects.ps1    v14 허용
```

검증

```text
Client x64 Debug 빌드
이식 스크립트 dry run 으로 축별 이식 성공/실패 건수 리포트
워로드 Q element 에서 particle 수를 바꾸면 화면이 바뀐다
버튼 직후 화면이 누르기 직전과 같다   <- 이식이 맞았는지의 판정
```

### G2  templateId 를 effect.standard 로

```text
도화가 F 계열만 Artist31470 유지, 나머지 effect.source_material 을 내린다
검증: 도화가 F 가 지금과 같게 보인다
```

### G3  문서 네 갈래 대조 후 .unified 일원화

```text
.effect.json / .authored-baseline / .restoration-candidate 에만 있는 정보를 찾아
.unified 로 옮긴 뒤 나머지를 지운다
검증: catalog 참조가 전부 해석되고 제품 재생이 유지된다
```

### G4  스킬 단위 손 튜닝

사용자 작업. 에이전트는 화면을 판정하지 않는다.

## 5. 새 세션이 시작할 때 확인할 것

```text
git log --oneline -8                    8dc2cde5 가 보이는지
git status --short                      다른 세션의 미커밋 변경
python Tools/EffectPipeline/survey_effect_elements.py     현재 분포
Client/Bin/Debug/Client.exe 실행 여부    링크 전에 닫아야 한다
```

읽을 문서

```text
AGENTS.md, CLAUDE.md, .md/GB/gotchas.md
.md/GB/08-17/2026-08-17_TRACK_A_POSTMORTEM_AND_SECOND_RESTORATION_PLAN.md
.md/GB/08-17/2026-08-17_EFFECT_RESTORATION_UNIFIED_IMPLEMENTATION_PLAN.md
이 문서
```

## 6. 하지 않기로 한 것

```text
스킬별 전용 셰이더        1차가 그 비용으로 멈췄다
원본 머티리얼 그래프 재현  같은 이유
sourceRecipe 삭제         근거로 보존한다
수치 완전 복원            시작값만 심고 손 튜닝으로 마무리
```
