# 2026-08-17 Effect 슬롯 구조와 Particle 튜닝 구현 계획

사용자가 캐릭터 이펙트를 직접 열어 튜닝하면서 제기한 질문에 코드 실측으로 답하고,
슬롯 정리와 튜닝 차단 해제의 실행 범위를 정한다.

## G00. 질문에 대한 답 — 전부 코드 실측

### G00.1 슬롯은 5개가 아니라 세 종류다

Effect 문서에서 "슬롯이 10개가 넘는다"고 보이는 이유는 **성격이 다른 세 배열이 한 화면에
섞여 나오기 때문**이다.

```text
Client/Public/Effect_DocumentRenderer.h:197
    std::array<ComPtr<ID3D11ShaderResourceView>, 5>  Textures;
Client/Public/Effect_DocumentRenderer.h:198
    std::array<ComPtr<ID3D11ShaderResourceView>, 7>  SourceTextures;
Client/Public/Effect_DocumentRenderer.h:257
    std::array<std::optional<EFFECT_MATERIAL_TEXTURE_LANE_DESC>, 6u>  TextureLanes;
```

| 배열 | 개수 | 정체 | 셰이더 이름 |
|---|---:|---|---|
| `Textures` | 5 | 표준 저작 슬롯 | `g_BaseTexture` … `g_DissolveTexture` |
| `SourceTextures` | 7 | Track A 재구성 소스 레인 | `g_SourceTexture0` … `g_SourceTexture6` |
| `TextureLanes` | 6 | material execution 레인 메타 | (위 둘로 해석돼 들어감) |

즉 사용자가 본 "6번째 슬롯의 lensflare"는 표준 슬롯의 6번째가 아니라
**`SourceTextures` 레인**이다. 표준 슬롯은 처음부터 5개뿐이다.

### G00.2 표준 경로는 정말 5개만 샘플링한다

`Shader_EffectCommon.hlsli`가 12개를 **선언**하지만, 표준 파티클/스프라이트 경로에서
실제로 `Sample()`하는 것은 이름 있는 5개뿐이다.

```text
Shader_EffectCommon.hlsli
  131  g_NoiseTexture.Sample
  136  g_BaseTexture.Sample
  138  g_MaskTexture.Sample
  143  g_DissolveTexture.Sample
  155  g_EmissiveTexture.Sample
  266~295  두 번째 경로에서 같은 5개 반복
```

`g_SourceTexture0~6`을 실제로 샘플링하는 파일은 다음뿐이다.

```text
Shader_Artist31470RuntimeMaterial.hlsli    34건
Shader_Artist31470Diagnostic.hlsli          6건
Shader_Artist31470Active011OuterMaterial.hlsli  4건
Shader_Artist31470Active003RibbonMaterial.hlsli 2건
Shader_Artist31470Active022DecalMaterial.hlsli  1건
Shader_EffectLocalDecalAdapter.hlsli        5건
```

**전부 Track A 재구성 경로와 decal adapter다.** 일반 저작 Effect는 5개로 완결된다.

따라서 사용자의 판단이 맞다. 5개로 캐릭터 이펙트를 매핑할 수 있다.
다만 "지워도 되는가"의 답은 조건부다. 아래 G00.3을 본다.

### G00.3 그래서 지워도 되는가 — 조건부로 그렇다

```text
지워도 되는 것
  material.execution.textureLanes 와 material.sourceMaterial.textures 중
  해당 element의 material.templateId 가 effect.standard 인 경우.
  이 경우 렌더러는 Textures[5]만 바인딩하고 SourceTextures는 건드리지 않는다.

지우면 안 되는 것
  templateId 가 reconstructed-* / effect.source_material 인 element.
  이쪽은 Shader_Artist31470* 이 SourceTextures 를 실제로 읽는다.
  Artist F 복원 결과가 여기 걸려 있다.
```

즉 정리는 **일괄이 아니라 templateId 기준 조건부**여야 한다.

### G00.4 6번째 슬롯 바인딩 시 abort 하는 이유

`TextureLanes`가 `std::array<..., 6u>`이고 `SourceTextures`가 7이다.
저작 UI가 레인 인덱스를 경계 검사 없이 그대로 인덱싱하면 그 지점에서 터진다.
`Effect_DocumentRenderer.cpp:3069`는 경계를 검사하지만
`3326`의 `Staged.SourceTextures[static_cast<size_t>(iIndex)]`는 검사가 없다.

```text
Effect_DocumentRenderer.cpp:3069   if (iLane >= Staged.SourceTextures.size() || ...)   검사 있음
Effect_DocumentRenderer.cpp:3326   Staged.SourceTextures[static_cast<size_t>(iIndex)]  검사 없음
```

이것은 저작 중 crash이므로 우선순위가 가장 높다.

### G00.5 Particle 수치를 바꿔도 반영되지 않는 이유

`sourceRecipe.enabled`가 true면 재생이 **원본 Cascade 모듈**을 따라간다.
저작 `Detail.Particle` 값은 그때 무시된다.

```text
Effect_Playback.cpp:669
    if (!bSourceVisualProgramActive || !Element.SourceRecipe.bEnabled)
        return false;
Effect_Playback.cpp:541 / 611 / 655 / 789 / 953 / 1391  같은 게이트
Detail.Particle.* 를 읽는 지점                          24건
```

Track A로 들어온 캐릭터 element는 대부분 `sourceRecipe.enabled = true`다.
그래서 사용자가 particle 수를 바꿔도 화면이 그대로였다.

반대로 이번에 생성한 발탄 문서 74개는 전부 `enabled = false`다.

```text
effect.valtan.whirlwind.active  ->  sourceRecipe.enabled = [False, False]
```

**발탄 문서는 지금 그대로 튜닝이 먹는다.** 캐릭터 문서만 막혀 있다.

### G00.6 Mesh Particle과 Sprite Particle의 차이

둘 다 `EFFECT_ELEMENT_KIND::PARTICLE`이고, 차이는 **파티클 하나를 무엇으로 그리는가**다.

```text
Sprite Particle   파티클마다 카메라를 향하는 사각형 하나. 정점은 런타임이 만든다.
                  meshModel 슬롯이 비어 있다. 원본 시스템 479개가 이쪽이다.
Mesh Particle     파티클마다 WModel 하나를 인스턴스로 그린다.
                  meshModel 슬롯에 .wmodel 이 필요하다. 원본 178개가 이쪽이다.
```

그래서 `Try_CreateElementDraft`가 Mesh Particle 생성 전에 WModel 시드를 요구한다.
생성 후에는 family가 고정된다.

불꽃 파편처럼 **덩어리가 회전하며 날아가는 것**은 Mesh Particle이고,
빛무리처럼 **항상 카메라를 향하는 것**은 Sprite Particle이다.

### G00.7 glow가 생성돼서 가운데로 모이는 연출

`fx_c_glow_007.dds` 같은 Sprite Particle이 안쪽으로 수렴하는 것은
텍스처가 아니라 **속도 방향**이 만든다.

```text
Detail.Particle 이 소유하는 것 (24 지점에서 소비)
  iMaxParticles / iBurstCount / fSpawnRatePerSecond
  vLifeTimeSeconds / vStartSize
Detail.Transform
  vVelocityPerSecond
```

바깥에서 안쪽으로 모이려면 스폰 위치가 원주에 흩어지고 속도가 중심을 향해야 한다.
현재 저작 `Detail`에는 **스폰 형태(원주/구/박스)와 방향 필드가 없다.**
원본은 Cascade의 `particlemodulelocation*` / `particlemodulevelocity*` 모듈이 그 역할을 했고,
그것이 `sourceRecipe.modules`에 남아 있다.

따라서 이 연출은 현재 저작 슬롯만으로는 재현할 수 없다.
G03의 범위 결정이 필요한 지점이다.

## G01. 사용자 추측에 대한 판정

```text
"슬롯 5개면 충분하다"                          맞다. 표준 경로 셰이더가 5개만 샘플링한다.
"나머지는 중복이다"                            부분적으로 맞다. templateId 가 effect.standard 면
                                               중복이고, reconstructed-* 면 실사용이다.
"매핑만 되면 워로드처럼 튜닝으로 복원된다"      Detail 로 표현 가능한 축은 맞다.
                                               스폰 형태와 속도 방향은 현재 저작 축에 없다.
"도화가 F는 매핑이 아니라 중복/비가시 element 문제"  맞다. be5a4059 가 같은 문제를 정리했다.
"모든 스킬이 동일한 상황"                       맞다. 원인이 element 단위가 아니라
                                               sourceRecipe 게이트와 슬롯 표시라서 전 클래스 공통이다.
```

## G02. 변경할 파일

```text
Client/Private/Effect_DocumentRenderer.cpp   SourceTextures 인덱스 경계 검사
Client/Private/Effect_Tool.cpp               슬롯 출처 표시, sourceRecipe 게이트 표시
Client/Public/Effect_Tool.h                  상태 멤버
Tools/EffectPipeline/audit_effect_slots.py   신규. 문서별 슬롯 사용 실측 리포트
```

## G03. 구현 순서

### G03.1 crash 차단 (최우선)

`Effect_DocumentRenderer.cpp:3326`의 무검사 인덱싱에 경계 검사를 넣는다.
`3069`가 이미 쓰는 패턴을 그대로 따른다. 범위를 벗어난 레인은 바인딩을 거부하고
사유를 상태 문자열로 남긴다. 저작 중 abort 는 어떤 이유로도 허용하지 않는다.

```text
검증  6개 초과 레인을 가진 element 를 열고 슬롯을 눌러도 죽지 않는다
```

### G03.2 슬롯이 어디서 온 것인지 화면에 표시

현재 Declared Resources는 세 출처를 구분 없이 나열한다. 이것이 혼란의 직접 원인이다.

```text
[표준]   base / noise / mask / emissive / dissolve      항상 편집 가능
[소스]   SourceTexture0..6                              templateId 가 reconstructed 일 때만 유효
[레인]   material execution lane                        위 둘로 해석되는 메타
```

카드에 출처 배지를 붙이고, `effect.standard` element의 소스/레인 카드는
"이 element 의 셰이더는 이 레인을 읽지 않는다"로 비활성 표시한다.
삭제는 하지 않는다. 표시만으로 사용자가 무엇을 지워도 되는지 판단할 수 있다.

### G03.3 sourceRecipe 게이트 표시와 해제

Detail 패널 상단에 현재 element 의 재생 소유자를 명시한다.

```text
sourceRecipe.enabled = true   "원본 모듈이 재생을 소유합니다. Detail 수치는 무시됩니다."
                              [Take Authoring Control] 버튼
sourceRecipe.enabled = false  "Detail 수치가 재생을 소유합니다."
```

`Take Authoring Control`은 `sourceRecipe.enabled`를 false로 내리고
현재 재생 중인 값에서 읽을 수 있는 것(lifetime, size, burst)을 `Detail.Particle`에
한 번 복사한 뒤 commit 한다. 복사할 수 없는 축은 건드리지 않는다.

이것이 사용자가 막혔던 지점의 직접 해제다.

```text
검증  워로드 Q element 에서 버튼을 누르고 particle 수를 바꾸면 화면이 바뀐다
```

### G03.4 슬롯 사용 실측 리포트

`audit_effect_slots.py`가 Authored 문서 전체를 훑어 다음을 낸다.

```text
문서 / element / templateId / 표준 슬롯 사용 / 소스 레인 사용 /
sourceRecipe.enabled / 셰이더가 실제로 읽는 레인 수
```

이 리포트가 있어야 "중복이니 지운다"를 문서 단위가 아니라 **근거 단위**로 할 수 있다.
지우는 작업 자체는 리포트를 보고 별도 변경으로 한다.

### G03.5 스폰 형태와 속도 방향 (범위 결정 필요)

`fx_c_glow_007`이 가운데로 모이는 연출처럼 **스폰 형태와 초기 속도 방향**이 필요한 표현은
현재 저작 축에 없다. 두 선택지가 있다.

```text
A  Detail.Particle 에 스폰 형태(point/sphere/ring/box)와 방향 모드(outward/inward/fixed)를 추가
   장점  Cascade 없이 저작으로 그 연출을 만들 수 있다
   비용  Detail 스키마 확장 + 재생 소비 + 문서 버전
B  sourceRecipe 의 location/velocity 모듈만 읽어 Detail 로 투영
   장점  원본 값이 근거가 된다
   비용  모듈 종류별 해석이 필요하고 실패 시 조용히 틀린다
```

이번 계획에서는 결정하지 않는다. G03.1~G03.4를 끝내고 실제로 몇 개 element가
이 축을 필요로 하는지 리포트로 센 뒤 정한다.

## G04. 발탄과의 관계

발탄은 이 문제에서 자유롭다. 생성한 74개 문서가 전부
`sourceRecipe.enabled = false`, `templateId = effect.standard`,
표준 슬롯 5개만 사용이다.

```text
family: Sprite Particle 162 / Mesh Particle 29
슬롯:   base 189 / noise 184 / mask 151 / emissive 106 / dissolve 73 / meshModel 29
```

즉 사용자가 정한 "발탄은 5개 슬롯으로만 간다"는 이미 그렇게 되어 있다.
캐릭터 쪽만 Track A 잔재를 정리하면 두 축이 같은 규칙으로 수렴한다.

## G05. 이 계획이 하지 않는 것

```text
소스 레인 일괄 삭제        리포트 없이 지우지 않는다
Cascade 모듈 해석기         G03.5 결정 전
Detail 스키마 확장          G03.5 결정 전
Artist F 재구성 경로 제거   Shader_Artist31470* 이 실제로 읽고 있다
```
