# 2026-08-17 Track A 사후 분석과 2차 복원 계획

"왜 도화가 F만 간신히 됐고 나머지는 안 됐는가"에 데이터로 답하고,
원본 동일이 아니라 **데이터 획득**을 목표로 하는 2차 복원의 범위를 정한다.

## G00. 결론 — 실패는 매핑이 아니라 소유권에서 났다

Track A는 원본 데이터를 정확히 뽑아 정확히 매핑했다. 실측이 그것을 증명한다.

```text
UNRESOLVED_ASSET     0건    모든 asset 경로가 실물로 해석된다
ZERO_SCALE / SIZE    0건    크기가 0이라 안 보이는 element 는 없다
HIDDEN               0건    visible=false 로 꺼진 element 도 없다
```

**매핑은 성공했다.** 그런데 화면에 의도대로 안 나왔다. 이유는 셋이다.

## G01. 원인 1 — 제품이 보는 문서는 저작할 수 없는 문서였다

같은 스킬에 문서가 네 갈래로 존재한다.

```text
effect.X.effect.json                    v12   sourceRecipe.enabled = 0     저작 소유
effect.X.authored-baseline.effect.json  v12   sourceRecipe.enabled = 0     저작 소유
effect.X.restoration-candidate...json   v13   sourceRecipe.enabled = 0     저작 소유
effect.X.unified.effect.json            v13   sourceRecipe.enabled = 전부  원본 모듈 소유
```

그리고 런타임 catalog가 무엇을 싣는가.

```text
EffectCatalog.runtime.json   effects 99개
  .unified        98
  그 외            1
```

**제품은 `.unified`만 본다.** 그런데 `.unified`는 `sourceRecipe.enabled=true`라
저작 수치가 무시된다. 반대로 저작 가능한 `.effect.json` / `.authored-baseline` /
`.restoration-candidate`는 catalog에 없어서 **화면에 아예 나오지 않는다.**

즉 지난 작업의 상당 부분은 **아무도 읽지 않는 문서를 저작한 것**이었다.
이것이 "원본 데이터 추출하고 적용시킨 작업이 의미가 없었던" 직접적인 이유다.

## G02. 원인 2 — 축마다 소유자가 다르다

`.unified` 문서에서도 전부 안 먹는 것은 아니다. 재생 코드가 축을 나눠 읽는다.

```text
Effect_Playback.cpp:4577-4592
    Detail.Transform.vPosition / vRotationDegrees / vRevolutionDegreesPerSecond
    -> 게이트 없이 읽는다

Effect_Playback.cpp:669 외 6곳
    if (!bSourceVisualProgramActive || !Element.SourceRecipe.bEnabled) return false;
    -> Particle 축은 원본 모듈이 소유하면 저작값을 버린다
```

**이것이 워로드와 다른 스킬의 차이를 설명한다.**

```text
워로드에서 사용자가 한 것   크기 조정, 위치 조정        -> Transform 축, 게이트 없음 -> 먹었다
다른 스킬에서 시도한 것     particle 수 조정            -> Particle 축, 게이트 있음 -> 안 먹었다
```

사용자가 "워로드는 되는데 왜 다른 건 안 되지"라고 느낀 것은 스킬의 차이가 아니라
**건드린 축의 차이**였다. 워로드 문서도 `17030.unified`는 9개 element 전부
`sourceRecipe.enabled=true`다.

## G03. 원인 3 — 중복이 판단을 불가능하게 만들었다

```text
element  7,861 -> 3,042   중복 4,468 + 빈 것 351 제거 (61%)
크기     210.9 MB -> 84.9 MB
```

Track A import가 **distinct binding당 하나가 아니라 source occurrence당 하나**로
element를 만들었다. 한 스킬이 같은 시각 요소를 5~6번 들고 있었다.

```text
lancemaster 34570.clip2    144 -> 21    85% 중복
warlord     17040.baseline  91 -> 14    85%
lancemaster 34150.unified  186 -> 13    83%
dimensionmaster 2050540    228 -> 54    76%
```

겹쳐 그려지며 밝기가 누적되고, 무엇이 무엇인지 눈으로 구분할 수 없었다.
이 원인은 커밋 `26de16ec`로 해소됐다.

## G04. 그러면 도화가 F는 왜 됐는가

`effect.artist.skill.31470.unified`는 18 element 중 16이 source 소유다.
다른 스킬과 다르지 않다. **다른 것은 문서가 아니라 셰이더다.**

```text
Shader_Artist31470RuntimeMaterial.hlsli        34 sample
Shader_Artist31470Active003RibbonMaterial.hlsli 2
Shader_Artist31470Active011OuterMaterial.hlsli  4
Shader_Artist31470Active022DecalMaterial.hlsli  1
Shader_Artist31470Diagnostic.hlsli              6
```

도화가 F만 **원본 머티리얼을 해석하는 전용 셰이더를 사람이 직접 작성**했다.
그래서 `sourceRecipe`가 소유한 채로도 화면이 나왔다.

이것이 Track A의 실제 비용 구조다.

```text
스킬 하나 = 원본 추출 + 매핑 + 전용 셰이더 5개
도화가 F 하나에 그 비용을 다 썼고, 나머지 스킬에는 셰이더가 없다.
셰이더가 없는 스킬은 원본 모듈이 소유한 채 표준 셰이더로 그려져
의도한 그림이 나오지 않았다.
```

**즉 Track A는 스킬 수만큼 셰이더를 쓰지 않으면 완결되지 않는 구조였다.**
이것이 "원본과 동일"을 목표로 했을 때의 필연적 귀결이다.

## G05. 2차 복원의 방향 — 원본 동일이 아니라 데이터 획득

사용자의 방향 전환이 이 분석과 정확히 맞는다.

```text
1차   원본과 동일하게 만든다            -> 스킬마다 전용 셰이더가 필요 -> 1개에서 멈춤
2차   원본에서 데이터만 얻는다          -> 표준 셰이더 + 저작 수치로 만든다 -> 확장 가능
```

2차에서 원본으로부터 가져올 것은 다음뿐이다. 전부 이미 문서 안에 있다.

```text
어떤 element 로 구성되는가        element 목록 (중복 제거 완료)
어떤 mesh / texture 를 쓰는가      resources 슬롯 (asset 미해결 0건)
어떤 slot 에 들어가는가            base / noise / mask / emissive / dissolve
어떤 수치에서 시작하는가            sourceRecipe.modules 의 lifetime / size / spawn / color
```

가져오지 않을 것은 원본 머티리얼 그래프의 완전 재현이다. 그것이 셰이더를 요구한다.

## G06. 실행 범위

### G06.1 소유권 이전과 기본값 이식

`.unified` 문서의 element를 저작 소유로 바꾸되, 끄는 순간 화면이 사라지지 않도록
`sourceRecipe.modules`에서 읽을 수 있는 값을 Detail로 옮긴다.

```text
particlemodulelifetime   lifetime   -> Detail.Particle.vLifeTimeSeconds
particlemodulesize       startsize  -> Detail.Particle.vStartSize
particlemodulespawn      spawnrate  -> Detail.Particle.fSpawnRatePerSecond
particlemodulerequired   emitterdelay / emitterloops -> Detail.Timing
particlemodulecolor*     색          -> Detail.Color
particlemodulelocation*  스폰 형태   -> G06.3 의 새 필드
particlemodulevelocity*  초기 속도   -> G06.3 의 새 필드
```

읽을 수 없는 모듈은 건드리지 않는다. `sourceRecipe`는 지우지 않고 근거로 보존한다.

```text
대상  .unified 문서의 sourceRecipe.enabled=true element 1,909개
      (중복 제거 후 남은 수)
```

### G06.2 templateId 정리

`effect.source_material` element 중 Artist31470 셰이더가 실제로 필요한 것만 남기고
나머지는 `effect.standard`로 내린다. 표준 셰이더는 5슬롯을 정확히 샘플링한다.

```text
판정 근거  Shader_Artist31470* 이 읽는 sourceProfile 을 가진 element 인가
남길 것    도화가 F 계열
내릴 것    그 외
```

### G06.3 Detail 스키마 확장 (v14)

현재 저작 축에 없어서 손으로 만들 수 없는 두 가지를 추가한다.

```text
spawnShape       point | sphere | ring | box + radius / innerRadius / extents / arcDegrees
initialVelocity  fixed | outward | inward | cone + speed[min,max] + coneAngleDegrees
```

`fx_c_glow_007`이 원주에서 중심으로 모이는 연출, 두루미와 호랑이 mesh의 비행 궤도가
전부 이 두 축이다. v13 문서는 `point` + `fixed`로 읽어 현재 동작을 유지한다.

### G06.4 catalog 정본 일원화

같은 스킬에 문서가 네 갈래인 상태를 끝낸다.

```text
남길 것    .unified   catalog 가 이미 참조하고 animevents cue 가 가리킨다
정리할 것  .effect.json / .authored-baseline / .restoration-candidate
           catalog 에 없어 화면에 나오지 않는다
```

지우기 전에 `.unified`에 없는 정보가 그 안에 있는지 대조한다. 있으면 옮긴다.

## G07. 검증 방법

각 단계는 화면으로 판정한다. 자동 검증은 회귀 방지용이다.

```text
G06.1  워로드 Q element 에서 particle 수를 바꾸면 화면이 바뀐다
       버튼 직후 화면이 누르기 직전과 같다
G06.2  도화가 F 가 지금과 같게 보인다
G06.3  원주에서 중심으로 모이는 element 를 손으로 만들 수 있다
G06.4  catalog 참조가 전부 해석되고 제품 재생이 유지된다
```

## G07.5 인터뷰 확정 사항 (2026-08-17)

```text
값 이식 범위    읽힐 것만 이식하고 나머지는 sourceRecipe 에 보존한다.
                해석 못 하는 모듈은 건드리지 않고 기본값으로 남긴다.
문서 일원화     .unified 에 없는 정보가 다른 갈래에 있는지 대조한 뒤 .unified 로 합치고
                나머지 세 갈래를 지운다.
셀이더 경로     도화가 F 계열만 Artist31470 을 유지하고
                나머지 effect.source_material 은 effect.standard 로 내린다.
v14 시점        소유권 이전과 같은 변경 단위에서 넣는다.
                location/velocity 모듈을 새 필드로 바로 내려받는다.
```

이 결정으로 G06 의 순서가 확정된다.

```text
1  v14 스키마 확장 + 소유권 이전 + 값 이식        한 변경 단위
2  templateId 를 effect.standard 로 내리기
3  문서 네 갈래 대조 후 .unified 일원화
4  화면 검증과 손 튜닝
```

1을 한 변경으로 묶는 이유는 v14 없이 이전하면 location/velocity 를 받을 곳이 없어
그 축 정보가 이전 시점에 버려지기 때문이다. 두 번 하지 않으려면 같이 가야 한다.

## G08. 하지 않는 것

```text
스킬별 전용 셰이더 추가        1차가 그 비용으로 멈췄다
원본 머티리얼 그래프 재현      같은 이유
sourceRecipe 삭제              근거로 보존한다
수치 완전 복원                 기본값을 심고 손 튜닝으로 마무리한다
```
