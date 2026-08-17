# 2026-08-17 sourceRecipe 소유권 이전과 spawn/velocity 스키마 구현 계획

`2026-08-17_SECOND_RESTORATION_HANDOFF.md` §4의 G1을 실측으로 재범위화한 문서다.
배경은 `2026-08-17_TRACK_A_POSTMORTEM_AND_SECOND_RESTORATION_PLAN.md`,
실제 수행 결과는 `2026-08-17_EFFECT_SOURCE_RECIPE_TRANSPLANT_RESULT.md`다.

## G00. 인계 문서와 다른 실측 세 가지

계획을 쓰기 전에 현재 코드와 데이터를 다시 측정했다. 세 항목이 인계 문서와 다르다.

### G00.1 formatVersion 14는 이미 다른 계약이 쓰고 있다

```text
Client/Public/Effect_AuthoringDocument.h:16   EFFECT_AUTHORING_FORMAT_VERSION        = 13
Client/Public/Effect_AuthoringDocument.h:18   EFFECT_SOURCE_CONTRACT_FORMAT_VERSION  = 14
```

`version: 14`는 `purpose: "source_contract"`를 요구하는 native 소스 계약 문서다.
`Effect_DocumentCodec.cpp:10557`의 `bSourceContract` 판정이 그 숫자 하나로 갈린다.
저작 formatVersion을 14로 올리면 모든 저작 문서가 source contract로 해석된다.

15로 올리는 것도 비용이 크다. `Effect_Catalog.cpp:3982`, `Effect_OccurrenceTuning.cpp:1090`,
`Effect_DocumentCodec.cpp:10429`가 `iLoadedFormatVersion == EFFECT_AUTHORING_FORMAT_VERSION`을
정확히 요구하고, `Publish-Effects.ps1:1768`의 허용 목록이 `@(5..13)`, `1780`이
`DIRECT_AUTHORED_DOCUMENT_V13`에 대해 13을 강제한다. 디스크의 저작 문서는 v10 6개, v11 1개,
v12 132개, v13 193개이고 `Serialize`는 `Document.iLoadedFormatVersion`을 그대로 다시 쓴다.
즉 버전을 올리려면 332 문서 재작성과 catalog sha256 재발급이 함께 필요하다.

따라서 **버전은 올리지 않는다.** 두 블록은 `detail.particle`의 optional 필드로 추가한다.
이미 `modelPreScale`과 `dynamicParameter*` 세 필드가 같은 방식으로 들어가 있다.
블록이 없으면 `point` + `fixed`이고, 기본값이면 직렬화도 하지 않아 기존 문서는 byte 동일하다.

### G00.2 Detail은 비어 있지 않다

source 소유 particle element 1,876개의 Detail 실측이다.

```text
축                              기본값이 아닌 element
maxParticles                    1876 / 1876
spawnRatePerSecond              1876 / 1876     (그중 1621개가 0 = burst 전용)
lifeTimeSeconds                 1865 / 1876
startSize                       1822 / 1876
timing.lifeTimeSeconds          1847 / 1876
endSize                         1427 / 1876
transform.position               731 / 1876
initialPosition box              807 / 1876
initialVelocity box              532 / 1876
color.multiply                     1 / 1876
```

Track A의 정적 추출이 lifetime/size/spawnRate/maxParticles를 이미 심어 놓았다.
이 축을 다시 이식하면 맞는 값을 다시 계산한 값으로 덮어쓴다.
그래서 이식 규칙은 `모듈이 말하는 값을 쓴다`가 아니라
**`Detail이 아직 codec 기본값일 때만 쓴다`**로 바꾼다.

실제로 비어 있는 축은 color(1,875개 기본값), initialVelocity box(1,344개), spawn 형태다.

### G00.3 over-life는 새 스키마가 필요 없다 — 이미 host가 있다

```text
Effect_Playback.cpp:4950   Size = SourceRecipe ? Particle.vSize
                                  : lerp(vStartSize, vEndSize, ParticleT)
Effect_Playback.cpp:5006   ElementColor = Evaluate_Color(Element, ParticleT).vColorMultiply
Effect_Playback.cpp:4748   Evaluate_Color: LinearLerp.bColorMultiply 이면
                                  lerp(Color.vColorMultiply, endColorMultiply, ParticleT)
```

`ParticleT`는 element 시간이 아니라 **각 particle의 정규화 나이**다.
즉 `vStartSize -> vEndSize`가 `particlemodulesizemultiplylife`의 host이고,
`LinearLerp.colorMultiply`가 `particlemodulecolorscaleoverlife` /
`particlemodulecoloroverlife`의 host다. 새 curve 타입을 만들 필요가 없다.

단 두 가지 제약이 있다.

```text
제약 1  LinearLerp는 start -> end 직선뿐이다.
        도달 가능한 control point가 3개 이상인 curve는 투영하지 않는다.
제약 2  Effect_Playback.cpp:5019 가 저작 경로에서만
        Evaluated.Color.w *= (1 - ParticleT) 를 강제한다.
        source 경로에는 없는 alpha fade이므로 alpha는 원본과 정확히 같아질 수 없다.
```

`lookupTableTimeScale`이 0이면 `CEffectDistribution::Evaluate`가 entry 0에 고정된다.
control point가 2개여도 timeScale이 0이면 그것은 ramp가 아니라 상수다.
`colorscaleoverlife` 596, `coloroverlife` 332, `lifemultiplier` 653개가 이 경우다.

## G01. 목표

```text
1  source 소유 particle element 1,876개의 재생 소유권을 저작으로 옮긴다.
2  옮기기 전에, 읽을 수 있고 Detail이 비어 있는 축만 심는다.
3  기존 Detail이 표현하지 못하는 spawn 형태와 방사 속도만 새 필드로 추가한다.
4  sourceRecipe는 지우지 않는다. 읽지 못한 모듈의 유일한 근거다.
```

## G02. 새 필드가 필요한 실제 근거

```text
element 1,876개 중
  392  location primitive(sphere 166 / cylinder 138 / circle 98) 보유   <- spawnShape 필요
  290  primitive의 velocity=true 로 방사 속도 생성                       <- initialVelocity 필요
1,068  particlemodulelocation.startlocation 만 보유                      <- 기존 box로 충분
  592  particlemodulevelocity.startvelocity 만 보유                      <- 기존 box로 충분
```

두 블록은 필요하다. 다만 근거는 인계 문서가 시사한 1,900개가 아니라 위 392 / 290이다.
`point|sphere|ring|box` 네 kind는 cylinder를 담지 못하므로,
높이가 0인 cylinder만 ring으로 받고 실제 높이를 가진 것은 손대지 않는다.

## G03. 변경 파일

```text
Client/Public/Effect_AuthoringDocument.h    enum 2개, struct 2개, EFFECT_PARTICLE_DESC 멤버 2개
Client/Public/Effect_Playback.h             저작 spawn 표본 함수 2개 선언
Client/Private/Effect_DocumentCodec.cpp     token 2쌍, optional parse 2개, 조건부 serialize, Validate
Client/Private/Effect_Playback.cpp          Spawn_Particles의 저작 분기가 두 표본 함수를 호출
Client/Private/Effect_Tool.cpp              Detail Particle에 Spawn Volume / Emission Direction 블록
Tools/EffectPipeline/transplant_source_recipe.py   신규. 모듈 -> Detail 이식과 소유권 이전
Tools/EffectPipeline/Publish-Effects.ps1    Assert-EffectDetail이 두 optional 블록을 검사
```

새 C++ 파일이 없으므로 `.vcxproj` / `.vcxproj.filters` 등록 변경은 없다.

## G04. 데이터와 호출 흐름

### G04.1 읽기

`Read_V5Detail`이 `detail.particle`을 읽는 자리에서 두 블록을 optional로 읽는다.
`kind` / `mode`는 블록이 존재하면 필수이고, 나머지 수치는 optional이다.
블록이 없으면 struct 기본값 `POINT` / `FIXED`가 남는다.

### G04.2 재생

```text
CEffectPlayback::Spawn_Particles
  SourceRecipe.bEnabled            -> Apply_SourceSpawnModules (기존 경로, 변경 없음)
  그 외                            -> Sample_AuthoredSpawnPosition
                                      Sample_AuthoredInitialVelocity
```

`POINT`는 기존과 같은 순서로 같은 세 개의 난수를 뽑아 `initialPosition` box를 그대로 만든다.
`SPHERE` / `RING` / `BOX`는 그 box 위에 volume offset을 더한다.
`FIXED`는 기존 velocity box, `OUTWARD` / `INWARD`는 spawn 위치 기준 방사,
`CONE`은 element `+Y` 축 기준 원뿔이다. spawn 위치가 원점이면 방사 방향이 정의되지 않으므로
무작위 단위 방향을 뽑는다. 조용히 0을 내보내지 않는다.

### G04.3 이식 도구

```text
Data/Effects/Authored/*.effect.json
  -> element 별 sourceRecipe.modules 를 읽는다
  -> Detail이 codec 기본값인 축만 계산해 stage 한다
  -> 전체 문서를 문자열로 직렬화한 뒤에만 파일에 쓴다
  -> kind == "particle" 인 element만 sourceRecipe.enabled 를 false 로 내린다
```

`decal` 27개와 `light` 6개는 내리지 않는다. `Effect_Playback.cpp:655`의
`Is_SourceVisualDecalParticle`이 `bEnabled`를 요구하므로, 내리면 시뮬레이션 경로 자체가 바뀐다.

이식 축과 규칙이다.

```text
축                     source                                     조건
initialPosition box    particlemodulelocation.startlocation       box가 (0,0,0)/(0,0,0)
initialVelocity box    particlemodulevelocity.startvelocity       box가 기본값 또는 전부 0
spawnShape             location primitive 1개                     블록 부재, 높이 0
initialVelocity 블록   primitive velocity=true 의 velocityscale    velocity box가 비어 있을 때
color.multiply         startcolor/startalpha, over-life 시작값     multiply가 (1,1,1,1)
LinearLerp colorMultiply  over-life 끝값                          colorMultiply가 false
endSize                sizemultiplylife 끝값                       endSize == startSize 이고
                                                                   curve 시작이 1.0
burstCount             sourceRecipe.bursts[0]                     burstCount가 0
timing.startDelay      sourceRecipe.emitterDelaySeconds           startDelay가 0
```

단위는 재생과 동일하다. 위치와 속도는 `UE3_CentimetersToClient`
즉 `(x, z, -y) * 0.01`, 반경은 `* 0.01`이다.
분포 평가는 `CEffectDistribution::Evaluate`를 그대로 옮겨 구현한다.
쿠킹 테이블 앞의 range 값 2개와 chunk 배치, `lookupTableTimeScale <= 0`의 entry 0 고정까지 포함한다.

## G05. 이식하지 않는 것

```text
lifetime / startSize / spawnRate / maxParticles / timing.lifeTime
        Track A 추출이 이미 채웠다. 덮어쓰지 않는다.
control point 3개 이상의 over-life curve
        LinearLerp는 직선뿐이다. 없던 모양을 만들지 않는다.
cameraoffset / orbit / rotation / meshrotation / subuv / parameterdynamic / vectorfield
        저작 축에 대응이 없다. sourceRecipe에 그대로 보존한다.
emitterLoopCount != 1
        Detail에 loop 개념이 없다.
sourceRecipe 삭제
        근거로 보존한다.
```

## G06. 검증

```text
Client x64 Debug 빌드                          MSBuild, 작업 디렉터리 Client/Default
이식 도구 dry run                              축별 이식 성공/실패 건수
survey_effect_elements.py 전후                 SOURCE_OWNS_PLAYBACK 이동량
문서 재파싱과 codec/publisher 불변식 스윕       element 수 보존, 고아 master 0
Publish-Effects.ps1 -Mode Validate             변경 전후 동일 지점에서 멈추는지
git diff --check
```

화면 판정은 사용자가 수행한다. 에이전트는 Client를 실행하거나 캡처하지 않는다.
