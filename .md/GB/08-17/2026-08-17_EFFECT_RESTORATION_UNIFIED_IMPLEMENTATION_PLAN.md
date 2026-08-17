# 2026-08-17 캐릭터·발탄 Effect 복원 통합 구현 계획

`2026-08-17_EFFECT_SLOT_AND_PARTICLE_TUNING_IMPLEMENTATION_PLAN.md`와
`2026-08-17_VALTAN_PATTERN_EFFECT_TREE_AND_AUTHORING_IMPLEMENTATION_PLAN.md`를
하나로 묶고, 전수 실측으로 진단을 교정한 뒤 캐릭터와 발탄을 같은 규칙으로 수렴시킨다.

## G00. 실측이 진단을 바꿨다

사용자 가설은 "슬롯이 10개 넘게 중복돼서 난리가 났다"였다.
`Tools/EffectPipeline/audit_effect_slots.py`로 Authored 332 문서 8,219 element를 전수 조사한
결과는 다르다.

```text
전체                                    332 문서 / 8,219 element
materialTemplate
  effect.standard                       4,773
  effect.source_material                3,446
추가 레인 또는 비표준 슬롯을 가진 element      25
보이지 않는(visible=false) element             0
sourceRecipe.enabled = true             4,761   = 58%
```

차원술사만 보면 더 분명하다.

```text
차원술사        60 문서 / 2,387 element
  추가 슬롯                    0
  대표 element 의 resources    ["meshModel", "base", "noise", "mask"]
  sourceRecipe.enabled = true  1,946   = 82%
```

**슬롯은 이미 5개 이내다. 중복도 25건뿐이다.**
캐릭터 이펙트가 제대로 안 나오는 원인은 슬롯 개수가 아니라 다음 둘이다.

```text
원인 1  sourceRecipe.enabled = true 인 4,761 element 는 저작 Detail 이 무시된다.
        Effect_Playback.cpp:669 가 그 게이트다. 그래서 수치를 바꿔도 화면이 그대로다.
원인 2  effect.source_material 3,446 element 는 Shader_Artist31470* 재구성 경로로 간다.
        표준 5슬롯 셰이더가 아니라 g_SourceTexture0..6 을 읽는 다른 파이프라인이다.
```

사용자가 "워로드 Q, 창술사 BA, 도화가 F는 되는데 나머지는 안 된다"고 관찰한 것과 일치한다.
되는 것은 저작 축이 살아 있는 element이고, 안 되는 것은 원본 모듈이 재생을 소유해
사람이 손댈 수 없는 element다.

따라서 이 계획의 중심은 슬롯 정리가 아니라 **재생 소유권을 저작으로 되찾는 것**이다.

## G01. 목표

```text
1  8,219 element 중 튜닝이 막힌 4,761 개의 재생 소유권을 저작으로 옮긴다.
   옮길 때 원본 모듈에서 읽을 수 있는 값을 Detail 기본값으로 심는다.
2  effect.source_material 3,446 element 를 표준 5슬롯으로 수렴시킬지 판정하고 실행한다.
3  현재 저작 축에 없는 스폰 형태와 속도 방향을 채운다.
   이것이 없으면 "가운데로 모이는" 연출과 두루미/호랑이 mesh 비행을 손으로 만들 수 없다.
4  캐릭터와 발탄이 같은 규칙(표준 템플릿, 5슬롯, 저작 소유 재생)을 쓰게 한다.
```

발탄 74 문서는 이미 목표 상태다. `effect.standard`, 5슬롯, `sourceRecipe.enabled = false`.
즉 발탄이 기준선이고 캐릭터를 거기로 옮기는 작업이다.

## G02. 무엇이 진짜 병목인가 — 셰이더 실측

```text
Shader_EffectCommon.hlsli 가 표준 경로에서 실제 Sample 하는 것
  g_BaseTexture / g_NoiseTexture / g_MaskTexture / g_EmissiveTexture / g_DissolveTexture
  (131~155, 266~295 두 경로 모두 동일한 5개)

g_SourceTexture0..6 을 Sample 하는 파일
  Shader_Artist31470RuntimeMaterial.hlsli        34
  Shader_Artist31470Diagnostic.hlsli              6
  Shader_Artist31470Active*.hlsli                 7
  Shader_EffectLocalDecalAdapter.hlsli            5

런타임 배열
  Effect_DocumentRenderer.h:197  array<SRV,5>  Textures
  Effect_DocumentRenderer.h:198  array<SRV,7>  SourceTextures
  Effect_DocumentRenderer.h:257  array<opt,6>  TextureLanes
```

표준 경로는 5개로 완결된다. 사용자 판단이 맞다.
`SourceTextures`는 Track A 재구성 전용이며 3,446 element가 거기 걸려 있다.

## G03. Detail 이 소유하지 못하는 축

`Detail.Particle`이 소비되는 지점은 24곳이고 소유하는 값은 다음뿐이다.

```text
iMaxParticles / iBurstCount / fSpawnRatePerSecond
vLifeTimeSeconds / vStartSize
Detail.Transform.vVelocityPerSecond   (element 전체의 단일 속도)
```

없는 것은 두 가지다.

```text
스폰 형태     point / sphere / ring / box / mesh surface
초기 속도     outward / inward / cone / fixed + 속도 범위
```

`fx_c_glow_007`이 원주에서 생겨 중심으로 모이는 연출, 두루미와 호랑이 mesh가 궤도를 그리며
날아가는 연출은 전부 이 두 축이 만든다. 원본은 Cascade
`particlemodulelocation*` / `particlemodulevelocity*`가 했고 그 값이
`sourceRecipe.modules`에 그대로 남아 있다.

즉 **데이터는 이미 문서 안에 있고 저작 스키마가 그것을 표현하지 못한다.**
이것이 "매핑은 됐는데 해석이 안 된다"의 정확한 실체다.

## G04. 실행 단계

### G04.1 저작 중 crash 차단

```text
Effect_DocumentRenderer.cpp:3326  Staged.SourceTextures[static_cast<size_t>(iIndex)]
```

경계 검사가 없다. 바로 위 `3069`가 이미 쓰는 패턴을 그대로 넣는다.
범위를 벗어난 레인은 바인딩을 거부하고 사유를 상태 문자열로 남긴다.

```text
검증  레인 인덱스가 큰 element 에서 슬롯을 눌러도 죽지 않는다
```

### G04.2 sourceRecipe 게이트 표시

Detail 패널 상단에 재생 소유자를 명시한다. 지금은 사용자가 왜 안 먹는지 알 방법이 없다.

```text
enabled = true   "원본 모듈이 재생을 소유합니다. Detail 수치는 무시됩니다."  [Take Authoring Control]
enabled = false  "Detail 수치가 재생을 소유합니다."
```

### G04.3 재생 소유권 이전과 기본값 이식

`Take Authoring Control`은 `sourceRecipe.enabled`를 false로 내리기만 하면 안 된다.
그러면 element가 기본값으로 리셋되어 사용자가 본 화면이 사라진다.
**끄는 동시에 원본 모듈에서 읽을 수 있는 값을 Detail 로 옮겨야 한다.**

`sourceRecipe.modules`에서 투영할 축은 다음과 같다. 전부 className으로 식별되며
`Effect_Playback.cpp`가 이미 같은 문자열로 모듈을 찾는다.

```text
particlemodulerequired          emitterdelay, emitterloops, buselocalspace
particlemodulelifetime          lifetime            -> Detail.Particle.vLifeTimeSeconds
particlemodulesize              startsize           -> Detail.Particle.vStartSize
particlemodulespawn             spawnrate           -> Detail.Particle.fSpawnRatePerSecond
particlemodulelocation*         스폰 형태            -> G04.4 의 새 필드
particlemodulevelocity*         초기 속도            -> G04.4 의 새 필드
particlemodulecolor*            색                  -> Detail.Color
```

읽을 수 없는 모듈은 건드리지 않고 그대로 둔다. 조용히 기본값으로 덮지 않는다.

```text
검증  워로드 Q element 에서 버튼을 누른 뒤 particle 수를 바꾸면 화면이 바뀐다
      버튼 직후의 화면이 누르기 직전과 같다
```

### G04.4 Detail 스키마 확장 — 스폰 형태와 속도

`Detail.Particle`에 두 블록을 추가하고 formatVersion을 14로 올린다.

```text
spawnShape   kind: point | sphere | ring | box
             radius / innerRadius / extents / arcDegrees
initialVelocity
             mode: fixed | outward | inward | cone
             speed: [min, max]
             coneAngleDegrees
```

v13 문서는 두 블록이 없으면 `point` + `fixed`로 읽어 현재 동작을 그대로 유지한다.
codec, 재생, Tool Detail UI, publisher 검증을 같은 변경 단위로 닫는다.

이것이 이번 계획에서 **유일하게 새로 만드는 구조**다. 나머지는 전부 기존 축의 정리다.

### G04.5 effect.source_material 3,446 element 판정

`audit_effect_slots.py --csv`로 element 단위 표를 만든 뒤 세 갈래로 분류한다.

```text
A  표준 5슬롯으로 표현 가능      -> effect.standard 로 전환, SourceTextures 제거
B  Artist31470 셰이더가 실제로 필요 -> 그대로 유지
C  판정 불가                     -> 유지하고 목록으로 남긴다
```

전환은 문서를 다시 쓰는 작업이므로 리포트 없이 진행하지 않는다.
Artist F 복원 결과가 B에 걸려 있다.

### G04.6 발탄 트리 완성

`CValtanPatternTree`는 커밋 `5814fb65`로 들어갔다. 남은 것은 UI다.

```text
All Effects 에 페이즈/패턴/스테이지 3단 트리
  Gimmick 8 (triggerHealthBar 내림차순) / Rotation 24
  스테이지 행에 durationMs, hitShape, damage profile 표시
스테이지 선택 시 Resource Library 를 그 스테이지의 원본 후보로 좁힘
LIGHT 저작 family 추가
```

`hitShape`를 행에 띄우는 이유는 그것이 곧 시각 설계이기 때문이다.
`CIRCLE r=7` 다음 `RING outer=14 inner=7`은 안쪽 원과 바깥 고리를 그리라는 뜻이고,
`VALTAN_FLOOR_WIPE_130`의 `CROSS len=14 hw=2.2` 다음 `CIRCLE r=100`은
6방향 일자 다음 전멸이라는 뜻이다.

## G05. 캐릭터와 발탄이 만나는 지점

두 축은 같은 규칙으로 수렴한다.

```text
              발탄 74 문서            캐릭터 332 문서
템플릿        effect.standard 100%    standard 4,773 / source_material 3,446
슬롯          5 이내 100%             5 이내 사실상 전부 (예외 25)
재생 소유      저작 100%               저작 3,458 / 원본 모듈 4,761
```

발탄이 이미 목표 상태이므로 **발탄을 기준선으로 삼고 캐릭터를 옮긴다.**
G04.3과 G04.4가 끝나면 두 축의 저작 경험이 같아진다.

## G06. 하지 않는 것

```text
근거 없는 소스 레인 일괄 삭제      G04.5 리포트 이후에만
Artist31470 셰이더 경로 제거       3,446 element 가 실제로 읽는다
Cascade 모듈 완전 해석기           읽을 수 있는 축만 투영하고 나머지는 보존
수치 완전 복원                     기본값을 심고 손 튜닝으로 마무리한다
```
