# 2026-08-06 발탄 원본 이펙트 추출 결과

작성일: 2026-08-06  
범위: 발탄 이펙트 원본 조사와 리소스 추출  
제외: 패턴 AI, 애니메이션 연결, 콜라이더, 피해 판정, Server authority

## 1. 결론

발탄 Action 3종과 Action 밖의 BaseBuff/Aura 19종을 함께 조사했다. 명시적으로
ParticleSystem object를 가리키는 원본은 총 193개이며, 이 193개가 사용하는 UE3 graph,
Material/MaterialInstance parameter, Effect Mesh와 Texture를 모두 연결했다.

최종 직접 리소스는 Mesh 52개와 Texture 346개다. 398개 전부 원본 object 이름으로
export했고 `Client/Bin/Resources/Effect/Valtan`에 cook했으며 누락과 변환 실패는 0개다.

다만 이것은 **원본 구성요소와 리소스를 복구한 결과**다. Cascade graph를 우리
Effect Document Element로 자동 변환하고 월드에서 완성 발탄 이펙트를 재생하는 작업은
아직 남아 있다.

## 2. Action 이펙트 원본

다음 세 Action 문서를 전수 해석했다.

```text
MN_RPBF_00.loa
MN_RPBF_01-1.loa
MN_RPBF_02-2.loa
```

| 항목 | 합계 |
|---|---:|
| Action object | 170 |
| Stage | 2,464 |
| Animation clip occurrence | 2,378 |
| Notify | 21,931 |
| 명시적 unique ParticleSystem | 165 |
| Particle event occurrence | 6,159 |
| Decal event | 536 |
| Trail/TrailGhost event | 430 |
| Material change/parameter event | 606 |
| ViewShake event | 1,022 |

Action 결과는 다음 경로에 보존했다.

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/VALTAN/all_actions/
  MN_RPBF_00.action-effects.json
  MN_RPBF_01_1.action-effects.json
  MN_RPBF_02_2.action-effects.json
```

이 파일에는 Action ID, Stage, clip, notify 시작/끝 시간, ParticleSystem 참조와
`Unsupported / Unresolved`가 함께 들어 있다. 다른 세션의 패턴·애니메이션 작업은
필요할 때 이 시간축을 읽을 수 있지만, 이 세션에서는 gameplay로 변환하지 않았다.

## 3. Action 밖의 Aura와 상태 이펙트

`ParticleSoundNew`의 `BaseBuff_Valtan_*` 19개 문서를 이름이 아니라 typed reference로
읽었다. ParticleSystem occurrence는 47건, unique ParticleSystem은 29개다. 이 중 한 개는
Action 목록과 중복되므로 전체 합산에서는 28개가 새로 추가되어 `165 + 28 = 193`이 된다.

| BaseBuff source | 직접 참조한 구성 |
|---|---|
| Attack_Protection | 보호 이펙트 3종 |
| Barrier / Part_Weak | Shield 2종 |
| CounterDebuff_SilenceWave | Buff 2종 |
| Fetter | HoldBuff 2종 |
| LeapAttack_FakeTarget / RealTarget | FakeTarget 2종, RealTarget 1종 |
| PartsGuide | LockOnMark 1종 |
| Part_PowerUp | PowerUp 1종, 공용 Light 1종 |
| Rage | Rage 1종 |
| Spirit_Absorption / Wave | Spirit 3종 |
| Spirit_Brand / Explosion | SpiritBrand 2종, 공용 Light 1종 |
| Spirit_Burn | SpiritBurn 3종, 공용 Light 1종 |
| StrongRoar_Confinement / Stone | StrongRoar 4종 |
| ValtanFuryCurse | Eye mark 1종 |
| SpecialSkill_PP | Particle이 아닌 직접 Material 1종 |

`SpecialSkill_PP`는
`FX_M_MI_O_00.FX_M.FX_O_RPBF_Expost_01`을 직접 가리킨다. 파티클로 오분류하지
않고 Material reference로 보존했으며 부모 Material과 Texture parameter도 연결했다.

정본은 `Data/Effects/Imported/Valtan/Valtan.basebuff-effect-source.json`이다.

## 4. Particle graph와 Material 결과

최종 graph package는 17개다.

```text
FX_BS_00, FX_BS_01, FX_BS_03, FX_BS_04
FX_CM_00, FX_CM_01, FX_CM_02, FX_CM_03
FX_MN_BOSS_01
FX_MN_RPBF_00_D, FX_MN_RPBF_00_N, FX_MN_RPBF_00_O, FX_MN_RPBF_00_S
FX_MN_RPBF_02_G, FX_MN_RPRS_00_V, FX_MN_TSLC_07_G
FX_Post
```

| 검증 | 결과 |
|---|---:|
| Source ParticleSystem 요청 | 193 |
| Source ParticleSystem graph 해소 | 193 |
| 누락 Source ParticleSystem | 0 |
| Graph property parse error | 0 |
| Missing emitter target | 0 |
| Material binding | 335 |
| Material parent binding | 123 |
| Engine fallback | 1 |
| 해결되지 않은 물리 패키지 | 0 |

유일한 fallback은 `EngineMaterials.DefaultParticle`이다. Lost Ark Effect asset 누락이
아니므로 `ENGINE_DEFAULT_PARTICLE_FALLBACK`으로 별도 분류했다.

최종 카탈로그는
`Data/Effects/Imported/Valtan/Valtan.effect-resource-catalog.json`이다.

## 5. 실제 리소스와 런타임 cook

| 종류 | 요청 | 완료 | 실패 |
|---|---:|---:|---:|
| Effect Mesh | 52 | 52 | 0 |
| Effect Texture | 346 | 346 | 0 |
| 합계 | 398 | 398 | 0 |

원본 export 결과는 다음 경로에 있다.

```text
C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Effect/VALTAN/ActionBound/
```

런타임 결과는 다음 경로에 있다.

```text
Client/Bin/Resources/Effect/Valtan/Meshes/<PACKAGE>/*.wmodel
Client/Bin/Resources/Effect/Valtan/Textures/<PACKAGE>/*.dds
```

상세 증거:

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/VALTAN/all_packages/
  particle_graphs/particle_graph_manifest.json
  material-dependency-receipt.json
  resource-export-receipt.json
  runtime-cook-receipt.json
```

## 6. 빨간 장판과 아직 해소되지 않은 경계

`PlayDecalEffect` 536건의 Action ID와 시간은 추출했다. 그러나 해당 notify의 직렬화
payload에는 Material 또는 ParticleSystem object reference가 없다. 따라서 현재 결과는
**언제 장판을 호출하는지는 알지만 어떤 장판 asset을 쓰는지는 아직 모르는 상태**다.

같은 이유로 다음 항목은 `Unsupported / Unresolved`에 남겼다.

```text
generic Effect notify   3,787
PlayDecalEffect           536
DefaultParticle           133
합계                    4,456
```

이 항목을 이름으로 추측해 기존 Mesh/Particle/Decal에 강제로 넣지 않았다. 빨간 장판의
정확한 시각 asset은 SkillEffect/Decal Material 또는 별도 effect binding 원본을 찾아
추가 연결해야 한다.

## 7. 다른 세션으로 넘기는 경계

다른 세션은 다음만 담당한다.

1. 패턴과 Animation 소유권을 Server/Character 계약에 연결한다.
2. 이 카탈로그의 193개 graph root를 실제 사용 패턴별로 좁힌다.
3. graph node를 Effect Document의 Mesh/Sprite/Particle/Decal/Trail Element로 변환한다.
4. Material Instance parameter를 Material Template과 Effect Detail 값으로 변환한다.
5. unresolved Decal binding을 추가 조사해 빨간 장판 asset ID를 확정한다.

이 세션에서는 패턴 코드, Animation binding, Collider, damage, red-zone 판정을 수정하지
않았다.

## 8. 검증 결과

```text
BaseBuff typed reference extractor test     PASS
Action notify extractor test                PASS
Particle graph package                      PASS (17/17)
Graph property error                        PASS (0)
Missing emitter target                      PASS (0)
Source ParticleSystem resolution            PASS (193/193)
Physical package resolution                 PASS (854 + Engine fallback 1)
Exact resource export                       PASS (398/398)
Runtime resource cook                       PASS (398/398, failure 0)
Python extraction unit test                 PASS (29/29)
Generated JSON parse                        PASS (29 files)
Runtime receipt/file-size cross-check       PASS (398/398)
Client project/filter XML parse             PASS
ProjectAudit                                PASS (69 checks)
git diff --check                            PASS
```

이 결과로 발탄의 **명시적으로 참조 가능한 Particle/Material/Mesh/Texture 원본 추출**은
닫혔다. 완성 이펙트 재생과 unresolved Decal 복원은 후속 구현 범위다.
