# 2026-08-22 Valtan 3연격 Core-Three Missing Family Runtime ABI 복원 결과

## 1. 결론

3연격 `effect.valtan.front-back-front.windup`의 다음 세 family를
`TOOL_RENDERER_RUNTIME_ADMITTED_BOUNDED_RT0` 단계까지 연결했다.

| 역할 | exact child / parent | Tool carrier | exact occurrence |
|---|---|---|---:|
| Masked Dissolve stone | `fx_n_me_dissolve_04_011_ma` / `fx_d_pa_dissolve_01_ma` | bounded Local Mesh, `fm_a_stone_001` | 1 |
| Ground Decal | `fx_n_de_ground_04_30_tr` / `fx_d_de_ground_04_tr` | bounded fullscreen depth-projector Local Decal | 1 |
| Valtan Crack translucent | `fx_o_me_crack_01_01_tr` / `fx_d_me_crack_01_tr` | bounded Local Mesh, exact crackline model | 7 |

즉 세 family는 이제 단순히 DXBC나 번역 HLSL이 존재하는 상태가 아니다. All Effects에서
명시적으로 canary를 켜면 실제 Winters Tool renderer가 정확한 occurrence에 대해 shader,
material CB0/time, DDS, sampler, Target Depth와 carrier를 한 draw로 공급한다. 일반 Effect,
Product cue와 read-only source preview에는 자동 적용되지 않는다.

## 2. DXBC, HLSL, runtime ABI가 이번에 결합된 방식

```text
exact child/parent + shader-map identity
  -> exact cooked PS DXBC
  -> literal translated HLSL, WARP output mismatch 0
  -> exact material CB0 + live local time
  -> exact register/color-space DDS 19/19
  -> bounded sampler defaults + Target_Depth
  -> bounded Local Mesh / Local Decal Tool carrier
  -> exact 9/9 Authored gate, filtered preview subset packet
  -> Tool RT0 draw
```

다음 경계는 의도적으로 exact라고 부르지 않는다.

- 원본 `ParticleVF` / `FLocalDecalVertexFactory`와 BasePass 자체는 실행하지 않는다.
- source sampler의 address 명시값은 지키지만 package의 hardware filter default는 미해석이다.
- Ground와 Crack의 engine-owned scene CB2 lane은 source capture가 없어 neutral
  `{0, 0, 0, 1}` bounded 값으로 명시적으로 공급한다.
- 원본 MRT2/3/4/5와 Masked coverage output은 실행하지 않고 RT0만 출력한다.
- Product admission, visual admission, actual VF/pass, sampler exact는 모두 false다.

따라서 이번 완료 명칭은 `SOURCE_EXACT_PRODUCT`나 원본 완전 visual parity가 아니다.
정확한 pixel equation과 material/runtime 입력을 실제 Tool draw로 합친 첫 발탄 runtime canary다.

## 3. exact occurrence gate와 손튜닝 경로

전체 Authored 문서에서 다음 9개 ID가 중복 없이 모두 존재하고 material, source-disabled,
`modelPreScale=0.01` 계약이 맞아야 canary를 켤 수 있다.

```text
par_n_rpbf_atk_01_02.em07   Masked Dissolve stone
par_n_rpbf_atk_01_02.em14   Ground Decal
par_n_rpbf_atk_04_11.em00   Crack
par_n_rpbf_atk_04_11.em01   Crack
par_n_rpbf_atk_04_12.em00   Crack
par_n_rpbf_atk_04_12.em01   Crack
par_n_rpbf_atk_04_12.em02   Crack
par_n_rpbf_atk_04_13.em00   Crack
par_n_rpbf_atk_04_13.em01   Crack
```

carrier 교체는 `Build_PreviewDocument`의 비영구 투영이다. Authored JSON과 Save/Publish 입력은
바꾸지 않는다. em07만 stone model로, em14만 Local Decal로 투영하며 Crack 7개는 원래
crackline model identity를 유지한다. Complete뿐 아니라 target Element Solo에서도 먼저
원본 9/9 gate를 확인한 뒤 현재 필터에 남은 non-empty exact subset만 stage한다.

사용자 수동 검증 순서는 다음과 같다.

1. Visual Studio에서 Debug Client를 사용자가 직접 실행한다.
2. All Effects에서 `effect.valtan.front-back-front.windup` Authored 항목을 연다.
3. `Translated Valtan Core-Three Canary`를 명시적으로 켠다.
4. Complete와 위 9개 Element Solo를 번갈아 재생한다.
5. stone spike/dissolve, Ground Decal 투영·크기, Crack 7개의 방향·스케일·시간을 손튜닝한다.

사용자의 서면 육안 판정 전 visual PASS로 승격하지 않는다.

## 4. 런타임 회귀 격리

- canary 기본값은 OFF이며 enable은 Authored exact document의 pre-stage에서만 허용한다.
- Product/prepared/read-only/reconstructed/diagnostic/Clear 경로는 canary runtime을 제거한다.
- 다른 Effect ID, 중복 target, 변경된 material/model/carrier, `0.01` 회귀, DDS byte/SHA 불일치는
  fail-close한다. family-lite fallback으로 떨어지지 않는다.
- enable 실패는 ordinary preview를 복구하는 transaction으로 처리한다.
- canary draw는 IA layout/VB/IB/topology, VS/GS/PS, VS/PS CB, PS SRV/sampler, OM RT/DS/blend,
  RS state/viewport/scissor를 보존·복구한다.
- Ground rectangle은 shader/sampler 뒤 `Bind_Resources -> Render` 순서를 명시한다.
- 일반 fail-closed resource skip은 Valtan canary가 켜진 동안 적용하지 않아 exact packet staging을
  우회하지 않는다.
- carrier projection은 실제 Authored canary stage가 eligible일 때만 적용하므로 stale UI flag가
  Product/read-only preview를 변형하지 않는다.

## 5. 구현과 증거

주요 구현은 다음 파일에 있다.

```text
Client/Public/Effect_ValtanTranslatedCanaryRuntime.h
Client/Private/Effect_ValtanTranslatedCanaryRuntime.cpp
Client/Public/Effect_DocumentRenderer.h
Client/Private/Effect_DocumentRenderer.cpp
Client/Public/Effect_Object.h
Client/Private/Effect_Object.cpp
Client/Public/Effect_Tool.h
Client/Private/Effect_Tool.cpp
Client/Bin/ShaderFiles/Shader_Ue3Valtan{Dissolve01,Ground04,Crack01}.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectUe3Valtan{Dissolve01,Ground04,Crack01}.hlsl
Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/
  Valtan.front-back-front-runtime-canary-contract.targets.v1.json
  Valtan.front-back-front-runtime-canary-contract.receipt.v1.json
```

runtime contract는 C++/HLSL 구현 파일의 path, byte size와 SHA-256을 봉인한다. 파일 누락이나
hash drift는 admission을 거부한다. 최종 receipt SHA-256은
`2a77fbd9232f14987250a518912912e126d0d34a46f2dbba2dd9eb3aab6cfd02`이다.
이 raw-byte 계약에 포함된 12개 C++/HLSL 경로는 `.gitattributes`에서 `text eol=lf`로 고정해
`core.autocrlf=true`인 새 checkout에서도 같은 SHA-256을 유지한다.

## 6. runtime DDS handoff

Crack의 다음 세 source-exact DDS는 기존 runtime Resources에 없어서 원본 material export에서
현재 PC의 팀 관리 Resources 폴더로 복사했다.

| runtime asset | bytes | SHA-256 |
|---|---:|---|
| `Effect/Valtan/Textures/FX_TEX_02/fx_d_normal_053.dds` | 65,664 | `55452d24b322f4271a0e896e9ce001bfb440170cf91d55937ad2331b4d1be06e` |
| `Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_039_1.dds` | 384 | `e2f7f0f1090cff2ced06f097d7965dd0632511662a7c786d82d20d4140fb7a80` |
| `Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_091_2_cl.dds` | 384 | `4992ae031ae7eccabcdd74e4ceb5059fff4bbc7f07086ef79c7b4d9a1f101cb2` |

`Client/Bin/Resources`는 Git 제외 팀 관리 runtime 입력이다. 현재 PC에서는 19/19 byte/SHA가
맞지만 새 checkout이나 다른 PC에서는 이 세 파일을 같은 Resources 채널로 전달하기 전 Arm이
의도대로 fail-close한다. 따라서 renderer runtime admission은 이 물리 asset handoff를 포함한
현재 PC 기준이다.

## 7. 실행한 검증

현재까지 통과한 검증은 다음과 같다.

- 세 exact DXBC -> HLSL WARP replay: mismatch `0`
- 세 wrapper HLSL Debug/Release VS/PS compile: PASS, RT0 only
- runtime DDS byte/SHA/color-space pins: 19/19 PASS
- Valtan family/source-value/texture-closure/runtime-contract focused unit tests: 91 PASS
- runtime contract deterministic `--check`: PASS
- runtime-sealed C++/HLSL 12개 경로의 Git `text/eol=lf` 계약: PASS
- Client x64 Debug full compile/link: PASS
- Effect publisher Validate: 204 catalog entries PASS
- Valtan render contract: 113 documents, 3,734 elements, changed documents/elements 0 PASS
- project/filter XML parse와 scoped `git diff --check`: PASS

## 8. 남은 경계

다음 단계는 이번 결과를 지우거나 generic family로 되돌리는 작업이 아니다.

1. 사용자가 9개 occurrence를 Complete/Solo로 육안 검증하고 scale, position, timing을 손튜닝한다.
2. source sampler default와 engine scene CB capture를 복원한다.
3. 실제 ParticleVF/FLocalDecalVertexFactory, BasePass MRT/coverage를 연결한다.
4. 위 세 조건과 사용자 A/B 승인이 닫힌 뒤에만 Product cue admission을 별도 transaction으로 한다.
5. 그 다음 3연격의 나머지 Sprite/mesh/decal family를 같은 단위로 확장한다.
