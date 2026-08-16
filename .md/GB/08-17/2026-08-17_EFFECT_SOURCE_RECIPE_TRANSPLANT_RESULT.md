# 2026-08-17 sourceRecipe 소유권 이전과 spawn/velocity 스키마 결과

대응 계획서는 `2026-08-17_EFFECT_SOURCE_RECIPE_TRANSPLANT_IMPLEMENTATION_PLAN.md`다.
구현, 자동 검증, 사용자 수동 검증, 남은 경계를 분리해 기록한다.

branch `feature/effect-tool-texture-kind-filter`, 직전 커밋 `9e32e712`.

## 1. 실제로 구현한 것

### 1.1 스키마

`Client/Public/Effect_AuthoringDocument.h`

```text
enum EFFECT_PARTICLE_SPAWN_SHAPE      POINT / SPHERE / RING / BOX / END
enum EFFECT_PARTICLE_VELOCITY_MODE    FIXED / OUTWARD / INWARD / CONE / END
struct EFFECT_PARTICLE_SPAWN_SHAPE_DESC       eKind, fRadius, fInnerRadius, vExtents, fArcDegrees
struct EFFECT_PARTICLE_INITIAL_VELOCITY_DESC  eMode, vSpeedRange, fConeAngleDegrees
EFFECT_PARTICLE_DESC 에 SpawnShape / InitialVelocity 멤버 추가
```

**formatVersion은 올리지 않았다.** 14는 이미
`EFFECT_SOURCE_CONTRACT_FORMAT_VERSION`이 쓰고 있고, 15로 올리면
`Effect_Catalog.cpp:3982`, `Effect_OccurrenceTuning.cpp:1090`,
`Effect_DocumentCodec.cpp:10429`의 정확 일치 검사와 `Publish-Effects.ps1`의
`@(5..13)` 허용 목록, `DIRECT_AUTHORED_DOCUMENT_V13` 강제가 전부 깨진다.
두 블록은 `modelPreScale` / `dynamicParameter*`와 같은 optional 필드로 넣었고,
기본값이면 직렬화하지 않으므로 기존 v10~v13 문서의 byte 동일성이 유지된다.

### 1.2 codec

`Client/Private/Effect_DocumentCodec.cpp`

```text
PARTICLE_SPAWN_SHAPE_TOKENS / PARTICLE_VELOCITY_MODE_TOKENS
Read_ParticleSpawnShape / Read_ParticleInitialVelocity   (Read_V5Detail에서 호출)
Write_Detail             기본값이 아닐 때만 두 블록을 emit
Validate                 bSpawnShapeValid / bInitialVelocityValid 추가
```

Validate가 거부하는 값: 음수 반경, `innerRadius > radius`, 0 이하이거나 360 초과인 arc,
빈 box extents, `speed[1] < speed[0]`, 0~180 밖의 cone 각도,
particle이 아닌 kind가 POINT/FIXED 이외를 선언하는 경우.

### 1.3 재생

`Client/Public/Effect_Playback.h`, `Client/Private/Effect_Playback.cpp`

```text
Sample_AuthoredSpawnPosition       POINT는 기존 initialPosition box와 난수 순서 그대로,
                                   SPHERE는 부피 균일, RING은 XZ 면적 균일, BOX는 half-extent
Sample_AuthoredInitialVelocity     FIXED는 기존 velocity box,
                                   OUTWARD/INWARD는 spawn 위치 기준 방사,
                                   CONE은 element +Y 기준 원뿔
```

`Spawn_Particles`의 저작 분기가 이 둘을 호출한다. source 분기는 변경하지 않았다.

### 1.4 저작 UI

`Client/Private/Effect_Tool.cpp`의 Detail Particle에
`Particle Spawn Volume`과 `Particle Emission Direction` 두 블록을 추가했다.
kind/mode를 바꾸면 필요한 수치만 나타나고, 기존 read-only 게이트를 그대로 따른다.

### 1.5 이식 도구

`Tools/EffectPipeline/transplant_source_recipe.py` (신규)

`CEffectDistribution::Evaluate`를 그대로 옮겨 구현했다. 쿠킹 테이블 앞의 range 값 2개,
chunk 배치, `lookupTableTimeScale <= 0`일 때 entry 0 고정까지 포함한다.
전체 문서를 문자열로 직렬화한 뒤에만 파일에 쓰고, 쓰기 실패는 5회까지 재시도한 뒤
남은 문서 수와 복구 명령을 출력하고 exit 1 한다.

### 1.6 publisher

`Tools/EffectPipeline/Publish-Effects.ps1`의 `Assert-EffectDetail`이
두 optional 블록의 kind/mode 집합과 수치 범위를 검사한다. 블록이 없으면 통과한다.

### 1.7 데이터

126 문서 1,876 element의 `sourceRecipe.enabled`를 false로 내렸다.
`sourceRecipe`는 한 건도 지우지 않았다.

## 2. 자동 검증 — 실행한 것

```text
Client x64 Debug 빌드                 성공. Client.exe 생성. 신규 경고 없음
                                      (LNK4099 PDB 경고는 기존과 동일)
이식 도구 dry run                     아래 축별 표
survey_effect_elements.py 전후        SOURCE_OWNS_PLAYBACK 1,909 -> 33
문서 스윕                             element 3,400 보존, 고아 transform master 0,
                                      codec/publisher 불변식 위반 0,
                                      새 블록 spawnShape 249 / initialVelocity 45
git diff --check                      깨끗
Publish-Effects.ps1 -Mode Validate    변경 전후 모두 effect.artist.skill.31210.ba4 의
                                      Assembly hash mismatch 에서 동일하게 멈춘다 (2절 4항)
```

### 2.1 축별 이식 건수

```text
burst.transplanted                      1730     burst.alreadyAuthored            13
color.transplanted                      1505     color.alreadyAuthored             1
                                                 color.noModule                  370
colorOverLife.transplanted               366     colorOverLife.curveTooComplex  1384
shape.transplanted                       249     shape.cylinderHasHeight         117
                                                 shape.multiplePrimitives          8
                                                 shape.zeroRadius                 14
position.transplanted                     86     position.alreadyAuthored       1069
                                                 position.noModule               714
velocity.transplanted                     58     velocity.alreadyAuthored        532
                                                 velocity.noModule              1286
outward.transplanted                      45     outward.velocityBoxOccupied     115
emitterDelay.transplanted                  7     emitterDelay.alreadyAuthored     60
sizeOverLife.transplanted                  1     sizeOverLife.alreadyAuthored   1639
                                                 sizeOverLife.curveTooComplex    123
                                                 sizeOverLife.startNotIdentity    88
                                                 sizeOverLife.identity            25
emitterLoops.notRepresentable             19
skipped.notParticleKind                   33
```

`burst`가 가장 중요한 축이다. 1,876개 중 1,621개가 `spawnRatePerSecond == 0`인
burst 전용 emitter이므로, burstCount를 옮기지 않고 소유권만 내렸다면
그 element들은 입자를 하나도 내지 않는다.

`sizeOverLife`가 1건인 것은 실패가 아니라 Track A 추출이 이미
1,639개의 endSize를 채워 두었기 때문이다.

### 2.2 소유권 분포

```text
              전         후
SOURCE_OWNS_PLAYBACK    1909       33      (decal 27 + light 6, 의도적 보존)
MATERIAL_FAIL_CLOSED     234      234
element 합계            3400     3400
```

## 3. 사용자 수동 검증 — 아직 없음

에이전트는 Client를 실행하지 않았고 화면을 판정하지 않았다.
아래는 사용자가 직접 수행할 경로다.

```text
1  Client/Bin/Debug/Client.exe 실행
2  F1 -> Effect Tool -> All Effects
3  워로드 17030(Q) 문서를 연다
4  particle element 하나를 골라 Detail의 상단 배지가
   "Authored Detail owns playback." 인지 확인한다
5  Max Particles 또는 Start Size를 바꿔 화면이 실제로 바뀌는지 확인한다
6  Detail에 Particle Spawn Volume / Particle Emission Direction 두 블록이
   보이고, spawnShape를 가진 element에서 값이 채워져 있는지 확인한다
```

이식 정확도의 A/B는 데이터만 되돌리면 되고 재빌드가 필요 없다.

```text
git stash push -- Data/Effects/Authored     이식 전 문서로 되돌린다
같은 스킬을 열어 재생한다
git stash pop                               이식 후 문서로 되돌린다
같은 스킬을 다시 열어 재생한다
```

## 4. 남은 경계와 발견한 결함

### 4.1 화면 동일성은 alpha 축에서 구조적으로 불가능하다

`Effect_Playback.cpp:5019`가 저작 경로에서만
`Evaluated.Color.w *= (1 - ParticleT)`를 강제한다. source 경로에는 없다.
따라서 소유권을 내린 element는 원본이 alpha를 일정하게 유지하더라도
수명에 걸쳐 선형으로 사라진다. 이 fade를 없애면 이미 저작 소유인 발탄 74 문서
185 element가 전부 바뀌므로 이번 변경에서는 건드리지 않았다.

### 4.2 이식하지 못한 축

```text
colorOverLife  control point 3개 이상        1,384 element
sizeOverLife   control point 3개 이상          123 element
spawnShape     높이를 가진 cylinder            117 element
spawnShape     primitive 2개 이상                8 element
emitterLoops   loop count != 1                 19 element
전 축          cameraoffset 732 / rotation 705 / meshrotation 497 /
               orientationaxislock 424 / parameterdynamic 1,361 등은
               저작 축 자체가 없어 sourceRecipe에만 남아 있다
```

전부 `sourceRecipe`에 원본이 보존되어 있으므로 재이식이 가능하다.

### 4.3 Effect publisher는 이번 작업 이전부터 red다

```text
Publish-Effects.ps1 -Mode Validate
  Effect Assembly source file hash mismatch: effect.artist.skill.31210.ba4
```

`Data/Effects/Assemblies/**` 101개 중 81개의 `sourceDocumentFileSha256`가
`Data/Effects/Authored`의 현재 파일과 이미 어긋나 있다. 이전 compaction과
중복 제거 커밋이 문서를 재작성하면서 Assembly를 재발급하지 않은 결과다.
이번 변경 전후로 같은 지점에서 멈추므로 이 작업이 만든 회귀는 아니다.
Assembly 재발급은 별도 변경 단위로 처리해야 한다.

### 4.4 빈 문서 1건

`Data/Effects/Authored/effect.artist.skill.31210.ba4.unified.effect.json`은
HEAD 시점부터 element 0개다. 이번 작업은 이 파일을 수정하지 않았다.
4.3의 첫 실패 지점과 같은 asset ID다.

### 4.5 인계 문서와 다른 실측

```text
element 총계        인계 3,042 -> 실측 3,400 (331 문서)
formatVersion 14    이미 source contract가 사용 중. 저작 버전은 13 유지
Detail 공백 가정    lifetime/size/spawnRate/maxParticles는 이미 채워져 있었다
velocity 0/1909     실측은 532개가 이미 비어 있지 않았다
over-life 새 스키마 불필요. vStartSize->vEndSize 와 LinearLerp.colorMultiply가
                    이미 ParticleT로 평가되는 host다
```

### 4.6 Take Authoring Control 버튼은 만들지 않았다

`2026-08-17_TRACK_A_POSTMORTEM..._PLAN.md` G04.2가 제안한 element 단위 버튼은
구현하지 않았다. 일괄 이식 후 source 소유 element가 33개(decal 27, light 6)만 남고,
그 33개는 눌러서는 안 되는 대상이다. 누르면 시뮬레이션 경로가 바뀐다.
버튼을 넣으면 모듈 투영 로직이 Python과 C++ 두 벌이 되므로 넣지 않았다.
