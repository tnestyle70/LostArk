# 2026-08-17 중복 판정 교정과 source trim 결과

되돌리기 커밋 `413e4e36` 이후의 재작업이다. 대응 배경 문서는
`2026-08-17_TRACK_A_POSTMORTEM_AND_SECOND_RESTORATION_PLAN.md`와
`2026-08-17_SECOND_RESTORATION_HANDOFF.md`이며, 그 두 문서의 1번 작업 계획은
아래 3절 실측으로 **폐기한다.**

branch `feature/effect-tool-texture-kind-filter`, 직전 커밋 `413e4e36`.

## 1. 되돌리기 이후 확인한 기준선

```text
element 8,219   document 331
SOURCE_OWNS_PLAYBACK   4,761
MATERIAL_FAIL_CLOSED     691
NO_RESOURCES             351

워로드 17030   element 21, 손튜닝 2건 보존
               [0.57, 1, 0.53] / [0.51, 1, 0.91]
발탄            74 document 191 element, source 소유 3
Publish-Effects.ps1 -Mode Validate   PASS
```

`Publish-Effects.ps1`은 직전 RESULT 4.3에서 `effect.artist.skill.31210.ba4`의 Assembly
hash mismatch로 red였다. 되돌리기가 문서를 `5ffbdfe0` 시점으로 복원하면서 hash가 다시
맞았고 지금은 green이다. Assembly 재발급은 별도 작업으로 남겨둘 필요가 없어졌다.

## 2. 중복 판정은 전제부터 틀렸다

되돌린 `26de16ec`의 signature는 `(slotId, assetId)` 집합뿐이었다. 같은 corpus를 세 기준으로
다시 셌다.

```text
판정 기준                      중복 판정 수    비율
binding만 (되돌린 규칙)            4,759      57.9%
binding + transform               4,003      48.7%
element 전체 (id/displayName 제외)     6       0.1%
```

`binding + transform`으로도 4,003개가 남는다는 것은 이 element들이 transform 외의 축에서
다르다는 뜻이다. 되돌린 규칙이 합쳤을 4,471쌍을 열어 실제 차이를 셌다.

```text
detail.particle   11,855      sourceRecipe.modules  1,895
sourceNode         4,449      detail.transform      1,386
detail.timing      2,847      detail.decal          1,269
groupId            2,220      material.sourceProfile 1,199
kind                 109  <- element 종류 자체가 다른 쌍
```

전체 4,471쌍 중 실제로 동일한 것은 6쌍이다. `61% 중복`, `210.9 MB -> 84.9 MB`는 전부 이
잘못된 signature가 만든 수치이며 제거 대상이 실재했다는 증거가 아니다.

`NO_RESOURCES` 351개 일괄 삭제도 틀렸다. kind별 실측은 다음과 같다.

```text
kind         전체   resource 있음   없음
light          44        16          28   <- 텍스처를 붙이지 않는 것이 정상
screenPost     56        17          39   <- 같음
particle    4,801     4,517         284
```

light 28개와 screenPost 39개는 설계상 바인딩이 없다. 되돌린 규칙은 작동하는 조명을 지우고
있었다.

### 2.1 구현

`Tools/EffectPipeline/prune_duplicate_effect_elements.py`

```text
signature   (slotId, assetId) 집합  ->  element 전체에서 id/displayName만 제외
resource 0  일괄 삭제               ->  삭제하지 않고 kind별로 분류해 보고만 한다
```

dry run 결과는 6 element / 6 document이고 전부 발탄 문서다.

```text
effect.valtan.floor-wipe-130.interval          4 -> 3
effect.valtan.ghost-transition-15.tracking     5 -> 4
effect.valtan.imprison-roar.recovery           5 -> 4
effect.valtan.magic-choice.inner               4 -> 3
effect.valtan.magic-choice.outer               4 -> 3
effect.valtan.red-blade-wave.active            2 -> 1
```

**쓰지 않았다.** 동일한 element가 additive로 겹쳐 그려지고 있으므로 하나를 지우면 그
element의 밝기가 절반이 된다. 지금 화면이 발탄 저작의 기준선이라 사용자 결정 사항이다.

## 3. 소유권 이전은 축 손실이다 — 인계 문서 1번 작업 폐기

`sourceRecipe.enabled = false`는 축별 게이트가 아니다. `Effect_Playback.cpp`의 28개 지점이
이 플래그로 시뮬레이터 전체를 갈아탄다. 따라서 이식 도구가 "해석 못 하는 모듈은 건드리지
않는다"고 해도, flip 이후에는 그 모듈이 실행되지 않으므로 보존이 아니라 삭제다.

source 소유 particle 4,609개를 모듈 단위로 분류했다.

```text
모든 모듈이 저작 스키마로 표현 가능      109    2.4%
최소 한 축을 잃음                      4,500   97.6%

owner별 안전 비율
   dimensionmaster    47 / 1,840    3%
   lancemaster        46 / 1,755    3%
   warlord            13 /   684    2%
   artist              3 /   327    1%

무엇을 잃는가 (영향 element 수)
   parameterdynamic       3,058    셰이더 동적 파라미터
   cameraoffset           1,713
   rotation               1,614
   meshrotation           1,161
   orientationaxislock    1,023
   meshrotationrate         558
   velocityoverlifetime     823 (ef 포함)
   subuv 301 / acceleration 262 / orbit 253 / locationonground 242 ...
```

되돌린 `dca81719`가 1,876 element를 이전했으므로 그중 약 97%가 최소 한 축을 잃은 상태였다.
색 문제보다 이쪽이 크다.

### 3.1 색 도메인 진단은 틀렸다 — 철회한다

되돌리기 커밋 메시지는 `Detail.Color.multiply`에 도메인 변환 없이 원본 색을 써서 712개가
4배를 넘고 27개가 검정이 됐다고 기록했다. **이 진단은 잘못됐다.**

```cpp
// Effect_Playback.cpp ~4996
const float4_t ElementColor = Evaluate_Color(Element, ParticleT).vColorMultiply;
if (Element.SourceRecipe.bEnabled)
    Evaluated.Color = ElementColor * Particle.vColor;   // 저작값이 원본 위에 합성된다
else
    Evaluated.Color = ElementColor;
```

`Particle.vColor`는 `particlemodulecolor`의 `startcolor`를 그대로 담는다. 즉 7500배는
소유권과 무관하게 오늘도 화면에 적용되고 있으며, 이식이 그 값을 `Detail.Color.multiply`로
복사한 것은 충실한 동작이었다. 변환할 도메인 차이가 없다.

같은 코드가 더 중요한 사실을 말한다. **`Detail.Color.multiply`와 `Detail.Transform`은
source 소유 element에서도 이미 먹는다.** 워로드 Q 손튜닝이 그 증거다. 실제로 막혀 있던
것은 `Detail.Particle` 축뿐이다.

저작 소유로 내릴 때 생기는 진짜 색 변화는 따로 있다. 저작 분기에만
`Evaluated.Color.w *= 1 - ParticleT` 강제 fade가 있어서, 원본이 alpha를 일정하게 유지하는
element도 수명에 걸쳐 사라지게 된다.

## 4. 그래서 구현한 것 — source trim

소유권을 내리는 대신 저작 배율을 원본 결과 **위에 곱한다.** 해석하지 못한 모듈은 계속
실행된다.

### 4.1 스키마

`Client/Public/Effect_AuthoringDocument.h`

```text
struct EFFECT_PARTICLE_SOURCE_SCALE_DESC
    f32_t fCount     = 1.f    원본 spawn rate / burst / 상한
    f32_t fSize      = 1.f    원본 start size
    f32_t fLifeTime  = 1.f    원본 particle lifetime
    bool_t Is_Default() const

EFFECT_PARTICLE_DESC 에 SourceScale 멤버 추가
```

formatVersion은 올리지 않았다. `modelPreScale`, `spawnShape`, `initialVelocity`와 같은
optional 필드이며 기본값이면 직렬화하지 않으므로 기존 문서는 byte 동일하다.

### 4.2 codec

`Client/Private/Effect_DocumentCodec.cpp`

```text
Read_ParticleSourceScale       Read_V5Detail 에서 호출, 블록이 없으면 통과
Write_Detail                   Is_Default() 가 아닐 때만 emit
Validate                       bSourceScaleValid — 유한, 0 초과, 16 이하
```

### 4.3 재생

`Client/Private/Effect_Playback.cpp`

```text
Scale_SourceCount()        익명 namespace 헬퍼. SourceRecipe.bEnabled 일 때만 적용하고
                           반올림 후 uint32 포화. 저작 소유 element 는 건드리지 않는다
Spawn_Particles            Apply_SourceSpawnModules 직후 vBaseSize / vSize / fLifeTimeSeconds 에 배율
                           iMaxParticles 상한도 fCount 로 함께 올린다
스폰 스케줄링              burst 는 난수 추출 이후에 배율 적용(난수 스트림 불변),
                           rate 는 fSpawnAccumulator 누적에 배율 적용
```

`vBaseSize`에 곱하는 이유는 `Apply_SourceUpdateModules`가 매 스텝 `vSize`를 `vBaseSize`에서
다시 유도하기 때문이다. lifetime 배율은 `fNormalizedAge = age / lifetime`을 통해 over-life
모듈 전체를 비례해서 늘린다.

### 4.4 저작 UI

`Client/Private/Effect_Tool.cpp`의 Detail Particle에서 기존 read-only 안내 바로 아래에
`Source Trim` 블록을 넣었다. source 소유 element에서만 보인다.

```text
Count x / Size x / Life x     0.01 ~ 16.00
Reset Source Trim             기본값이 아닐 때만 노출
```

read-only 게이트 **밖**에 둔 것이 핵심이다. 위쪽 필드는 원본이 소유해서 못 바꾸고,
이 세 개는 그 결과를 조절하는 저작 축이다.

### 4.5 publisher

`Tools/EffectPipeline/Publish-Effects.ps1`의 `Assert-EffectDetail`이 optional `sourceScale`의
세 배율을 codec과 같은 범위로 검사한다. 블록이 없으면 통과한다.

## 5. 자동 검증 — 실행한 것

```text
Client x64 Debug 빌드                    오류 0. 신규 경고 없음
                                         (LNK4099 PDB 경고는 기존과 동일)
Publish-Effects.ps1 -Mode Validate       PASS  99 catalog entry
  유효한 sourceScale 주입 후             PASS
  count=0.0 주입 후                      element ID를 지목하고 FAIL
  주입 문서 복원 후                      PASS
validate_boss_pattern_effects.py         PASS  1 binding
prune_duplicate_effect_elements.py       dry run 6 element / 6 document
git status Data/Effects/Authored         변경 0 — 저작 corpus 미변경
git diff --check                         내용 오류 없음 (기존 LF->CRLF 경고만)
```

주입/복원은 `effect.lancemaster.skill.34010.ba1.unified`에 했고 `git checkout --`으로
정확히 되돌린 뒤 다시 PASS를 확인했다.

## 6. 사용자 수동 검증 — 아직 없음

에이전트는 Client를 실행하지 않았고 화면을 판정하지 않았다. 아래가 사용자가 직접 누를
경로다.

```text
1  Client/Bin/Debug/Client.exe 실행
2  F1 -> Effect Tool -> All Effects
3  워로드 17030(Q) 문서를 연다
4  particle element 를 하나 고른다. 상단에 주황색 read-only 안내가 보인다
5  그 아래 Source Trim 의 Count x 를 2.0 으로 올린다
6  입자 수가 실제로 늘어나는지 본다
7  Size x / Life x 도 같은 방식으로 확인한다
8  Reset Source Trim 을 눌러 원래 화면으로 정확히 돌아오는지 본다
```

8번이 이 변경의 판정 기준이다. 기본값에서 화면이 변경 전과 같아야 한다.

## 7. 남은 경계

```text
발탄 진짜 중복 6개        지우지 않았다. 지우면 해당 element 밝기가 절반이 된다
저작 소유 전환            폐기. 표현 가능한 109개만 남았고 일괄 이전은 하지 않는다
표현 못 하는 축           rotation / meshrotation / cameraoffset / parameterdynamic /
                          subuv / orbit / acceleration 은 여전히 손으로 만들 수 없다.
                          필요해지면 source trim 과 같은 방식으로 축을 하나씩 추가한다
강제 alpha fade           저작 분기의 w *= (1-ParticleT) 는 그대로 두었다.
                          이미 저작 소유인 발탄 185 element 가 전부 바뀌므로 별도 작업이다
corpus 크기 217 MB        중복이 아니었으므로 dedup 으로는 줄지 않는다.
                          줄이려면 sourceRecipe.modules 저장 방식을 따로 다뤄야 한다
```
