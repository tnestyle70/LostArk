# Valtan Imported Effect Sources

이 폴더는 발탄 원본 이펙트의 저작 입력과 리소스 카탈로그를 보관한다. 패턴, 애니메이션,
피격 판정, 콜라이더, Server authority는 이 폴더의 범위가 아니다.

## 정본 파일

- `Valtan.basebuff-effect-source.json`
  - `ParticleSoundNew`의 발탄 BaseBuff 19개 문서에서 직접 읽은 typed object reference다.
  - ParticleSystem 29개와 직접 Material 1개를 보존한다.
- `Valtan.effect-resource-catalog.json`
  - 발탄 Action 3개 문서와 BaseBuff 문서를 합친 최종 리소스 카탈로그다.
  - ParticleSystem 193개, 직접 Material 1개, Mesh 52개, Texture 346개를 소유한다.
- `Valtan.action-particle-resource-catalog.json`
  - BaseBuff를 합치기 전 Action-only 조사 결과다. 비교 증거이며 최종 소비 대상은 아니다.

## 최종 불변식

```text
Action-bound ParticleSystem          165
BaseBuff unique ParticleSystem        29
BaseBuff 중 Action과 중복               1
합산 unique ParticleSystem           193
직접 Material reference                1
Particle graph package                17
Graph property error                   0
Missing emitter target                 0
Runtime Mesh                          52
Runtime Texture                      346
Runtime cook failure                    0
```

`EngineMaterials.DefaultParticle` 한 건은 외부 패키지 누락이 아니라 UE3 Engine 기본
fallback이다. 카탈로그는 이를 `ENGINE_DEFAULT_PARTICLE_FALLBACK`으로 명시한다.

## 아직 자동 복원되지 않은 원본 이벤트

- `PlayDecalEffect` 536건은 시간과 Action 소유권은 추출됐지만 직렬화된 notify에
  Material/ParticleSystem object reference가 없어 빨간 장판 asset ID가 아직 없다.
- 일반 `Effect` 3,787건과 `DefaultParticle` 133건도 같은 이유로
  `Unsupported / Unresolved`에 남는다.
- Trail 계열 430건과 Material 변경 계열 606건은 이벤트로 추출했지만 아직 우리
  Effect Document Element로 자동 변환하지 않았다.

따라서 이 결과는 원본 graph와 직접 리소스의 복구 완료이며, 발탄 완성 이펙트가 현재
런타임에서 자동 재생된다는 뜻은 아니다. 다른 세션은 이 카탈로그를 입력으로 사용해
원본 node를 Mesh/Sprite/Particle/Decal/Trail Element와 Material Template으로 변환한다.

## 외부 상세 증거

대용량 원본 graph와 export/cook receipt는 다음 폴더에 있다.

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/VALTAN/
  all_actions/
  all_packages/particle_graphs/
  all_packages/resource-export-receipt.json
  all_packages/runtime-cook-receipt.json
```

실제 런타임 리소스는 Git payload가 아니라 다음 로컬 경로에 있다.

```text
Client/Bin/Resources/Effect/Valtan/
```
