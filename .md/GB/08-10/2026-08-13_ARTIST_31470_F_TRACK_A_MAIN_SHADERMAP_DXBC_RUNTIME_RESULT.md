# Artist F 31470 Track A Main ShaderMap/DXBC Runtime Result

## 0. 결론

Artist F의 main family인 occurrence `#9/#10` Watertrail과 `#11` Spritewave에 대해 다음 자동 복원 경로를 닫았다.

1. 공식 `45_975` 동일 배포 cohort에서 active MIC `FStaticParameterSet`를 RefShaderCache의 `FMaterialShaderMap` 2/2에 구조 join했다.
2. LocalVF BasePass pixel shader 2개와 연관 vertex permutation 4개, uniform expression, constant-buffer register, texture/sampler register를 복구했다.
3. 원본 PS DXBC를 재컴파일하지 않고 D3D11 WARP에서 실행해 21-case numeric replay를 통과시켰다.
4. 복구한 RT0 RGB/alpha·UV·dissolve 경로를 shipped `Shader_VtxEffectMeshPreview.hlsl`에 이식하고, 원본 DXBC와 runtime shader의 constant 19-case 및 nonuniform 4x4 spatial 15-case parity를 닫았다.
5. Debug/Release 모두 production Engine/ObjectManager/EffectObject 경로를 Microsoft WARP로 실제 first draw까지 실행해 `failures : 0`을 확인했다.

이 결과는 **main-family LocalVF BasePass PS의 RT0 candidate replay 및 nonProduct V4 review 구현 완료**다. 사용자 육안 `visual PASS`와 Product 승인은 수행하거나 대신 판정하지 않았으며 계속 `false`다.

## 1. 구현·자동 검증·수동 검증 상태

| 구분 | 상태 | 근거 |
|---|---:|---|
| source active MIC identity | 완료 | 2 family, 3 occurrence |
| official RefShaderCache structural join | 완료 | ShaderMap 2/2 |
| PS/VF/pass/DXBC recovery | 완료 | LocalVF BasePass PS 2, VS permutation 4 |
| uniform/CB/texture register closure | 완료 | uniform set 2, texture binding 7, CB admission true |
| original DXBC fixed-input replay | 완료 | WARP 21/21 |
| shipped runtime RT0 translation | 완료 | constant 19/19, spatial 15/15 |
| Debug/Release build | 완료 | Engine, UpdateLib, ClientFrontendHarness, Client 모두 PASS |
| Debug/Release actual WARP first draw | 완료 | 각 configuration `failures : 0` |
| full repository ProjectAudit | 미통과 | Track A 4개 check는 PASS, 기존/stale·무관 계약 30개가 함께 실패 |
| 사용자 육안 검증 | 대기 | 에이전트가 Client/UI를 실행하거나 visual PASS를 대신 판정하지 않음 |
| Product admission | 차단 | `product=false`, `visual=false` 유지 |

## 2. 공식 동일 배포 cohort provenance

정확한 표현은 `same revision`이 아니라 receipt가 봉인한 **official same distribution cohort**다.

| 입력 | identity |
|---|---|
| official manifest | `45_975.json`, 9,930,955 bytes, SHA-256 `331bfb3ef14cafc5a31f9006bc7590540589d61718527f2668eeb58ee7ec96e9` |
| RefShaderCache manifest row | file version 974, sequence 16, extracted 245,114,448 bytes, MD5 `1799ee1116fbb2811465576d8bb9e360` |
| RefShaderCache package | SHA-256 `83609267d9476f393cfe793d3d8cd265535c3d855e2f1f79774c618f85ed6455`, logical 919,307,435 bytes, UE868/licensee16/engine12097/cooker136 |
| official EFEngine | 19,929,968 bytes, SHA-256 `cc09e083c21bde705e4b8bc7a07d7014c618ef3f3a3ecf533e5b1fa326aa10e7`, Authenticode Valid |
| Water active MIC package | SHA-256 `404fe05a1de15e924795071ac56bd8ab2e5819b6e701363f0f26eed6a050c948` |
| Water base package | SHA-256 `2f729139288043929a9493f4f6c988754805bde8c2a8da44c90df170f41812ef` |
| Sprite source package | SHA-256 `6a46989680d244946e2c7910a444da7a403500c4e2f8af1665da196b05fadc3e` |

`EFEngine.dll` export ABI는 material shader-map lookup key가 별도 16-byte 후보가 아니라 `FStaticParameterSet + EShaderPlatform`임을 corroborate한다. Engine-equivalence key는 Base GUID와 switch/mask/terrain semantic equality를 사용하며 serialized row offset, FName table index, row order, `bOverride`를 cross-package identity로 오용하지 않는다.

## 3. FStaticParameterSet와 ShaderMap join

### 3.1 Active source identity

| family | occurrence | BaseMaterialId | active FStaticParameterSet | raw serialized SHA-256 |
|---|---|---|---:|---|
| Watertrail | `#9/#10` | `06e9a0ae14b09646b949a245fc42aa3c` | switch 13, mask/terrain 0 | `2aed9a10...` |
| Spritewave | `#11` | `b1f4ebf9dc948c41bd2830d999ba16cc` | switch 20, mask/terrain 0 | `6c65f31c...` |

Runtime recipe의 static row `14/9`는 packet 소비 denominator이고, source `FStaticParameterSet`의 `13/20`과 다른 수치다. 두 수치를 하나의 denominator로 합치지 않는다.

### 3.2 Joined shader objects

| family | shader-map 결과 | pixel shader | uniform/CB |
|---|---|---|---|
| Watertrail `#9/#10` | exact map 1 | shader ID `70bf...`, DXBC SHA `b16e274c...`, 4,232 bytes, 103 instructions | 5 vector / 37 scalar / 3 texture, CB0[16] |
| Spritewave `#11` | exact map 1 | shader ID `39f7...`, DXBC SHA `7e8dbb70...`, 4,024 bytes, 98 instructions | 11 vector / 52 scalar / 4 texture, CB0[24] |

Current installed RefShaderCache와 official v974 cache 사이에서 semantic map equality 2/2와 PS DXBC equality 2/2도 확인했다. 초기 identity receipt의 `join=0`은 cache 획득 전 단계의 역사 기록이며, 현재 최종 join 결론은 후속 RefShaderCache receipt의 2/2다.

## 4. Numeric replay와 runtime transaction

### 4.1 Canonical receipts

| receipt | canonical receipt SHA-256 | 결과 |
|---|---|---|
| main ShaderMap identity | `df1d348112ebc820e2ae90530a25d00e16c5b1c3ccfff7eb3e5533db5895afd1` | 2 family, 3 occurrence, pre-acquisition join 0 |
| main RefShaderCache | `34c5d9d52cd8f3e60c69271866dc6738c037158dd5d089fef3b6f10fe418a305` | current join 2/2, PS 2, VS 4, texture binding 7 |
| original DXBC replay | `19c76f22624fb58c43357fbfd71d0b94a7f2c2bc7ac395abed601ea31a512523` | original WARP 21/21 |
| runtime source replay | `66e303e2398259256352567af4db058ce8eaa076e22dee8f06ecf75bb14f6b7c` | constant 19/19, spatial 15/15 |

Original replay는 receipt의 raw DXBC bytes를 `CreatePixelShader`에 직접 전달한다. 별도 compiled carrier VS만 사용하며, parameter/register/texture/channel closure와 one-RTV/6-RTV control을 함께 확인한다.

Runtime replay는 실제 shipped `Shader_VtxEffectMeshPreview.hlsl`의 `PS_MAIN`을 컴파일해 `$Globals` CB0[110], source texture `t5..t8`, sampler `s5..s8` register contract를 검사한다. 결과는 다음과 같다.

- constant mutation: 19/19, maximum error `1.3969838619e-8`
- nonuniform 4x4 POINT_CLAMP spatial translation: 15/15, maximum error `0`
- runtime compiled PS DXBC: SHA-256 `88183a1d...`, 65,744 bytes

POINT_CLAMP는 원본과 runtime을 같은 fixture에서 비교하기 위한 수치 실험 조건이다. 이를 source-exact sampler policy라고 승격하지 않는다.

## 5. Runtime 구현 범위

복구 식은 다음 소스 경계에 연결했다.

- `Shader_Artist31470RuntimeMaterial.hlsli`: `#9/#10` Watertrail RT0 translation
- `Shader_Artist31470Active011OuterMaterial.hlsli`: `#11` Spritewave RT0 translation
- `Effect_DocumentRenderer.cpp`: active recipe packet, exact consumed/suppressed masks, texture semantic remap, dynamic/particle carrier, finite/fail-closed guard
- `verify_artist_31470_main_runtime_source_replay.py`: raw original DXBC와 shipped runtime shader의 independent WARP parity

Water의 original `t1` normal texture는 RT0가 아니라 original `o2` normal output에서 사용된다. 이번 transaction은 RT0 SceneColor RGB/alpha·UV·dissolve 경로이며, original MRT `o2..o5` 전체 복원이라고 기록하지 않는다.

## 6. D/R production-path WARP first draw

제품 기본 driver는 계속 `D3D_DRIVER_TYPE_HARDWARE`다. `ClientFrontendHarness`의 해당 mode만 `D3D_DRIVER_TYPE_WARP`와 Microsoft adapter `VendorId=0x1414`, `DeviceId=0x008c`를 요구한다.

Headless mode는 다음 실제 경로를 사용한다.

```text
production Engine initialization
  -> ObjectManager layer add
  -> EffectObject clone/stage
  -> zero-time render
  -> 1.5-second submitted draw
```

Debug와 Release 결과는 동일했다.

```text
active=28 candidate=26 attempted=28 submitted=25
suppressed=3 failed=0 completed=1 committed=1
failures : 0
```

추가로 Lance BA1 continuity draw도 두 fixed step에서 각각 `attempted=1 submitted=1 failed=0 committed=1`로 유지됐다.

Harness의 process-wide noninteractive error scope는 GPU material mode 진입 직후부터 적용되고 종료 시 이전 상태로 복원된다. Engine 초기화 실패는 생성된 manager와 output COM pointer를 rollback하며, Picking/Shadow를 device보다 먼저 해제한다. 제품 `MainApp`은 interactive HARDWARE 기본을 유지한다.

## 7. 실행한 검증

### 7.1 PASS

```powershell
# Engine x64 Debug/Release
MSBuild Engine/Default/Engine.vcxproj /p:Configuration=<Debug|Release> /p:Platform=x64

# public header/runtime deployment
UpdateLib.bat Debug
UpdateLib.bat Release

# ClientFrontendHarness x64 Debug/Release
MSBuild Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj /p:Configuration=<Debug|Release> /p:Platform=x64

# Client x64 Debug/Release
MSBuild Client/Default/Client.vcxproj /p:Configuration=<Debug|Release> /p:Platform=x64

# production-path WARP first draw, Debug/Release
Tools/ClientFrontendHarness/Bin/<Configuration>/ClientFrontendHarness.exe --effect-reconstructed-gpu-material Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json

# Track A deep chain
Tools/ProjectAudit/Test-Artist31470MainRuntimeSourceReplay.ps1 -Deep

# focused runtime contract
Tools/ProjectAudit/Test-Artist31470RuntimeMaterialV2.ps1
```

Deep chain은 identity target 2 / occurrence 3, RefShaderCache map 2, original DXBC 21 cases, runtime constant 19 + spatial 15를 통과했다. Focused runtime contract는 recipe 9/9, occurrence 10/10, CPU opcode 20/20을 통과했다.

### 7.2 Full ProjectAudit 경계

`Invoke-ProjectAudit.ps1` 전체 실행은 수행했다. 최종 report에서 다음 Track A check 4개는 모두 PASS했다.

- `effect.artist-31470-main-shader-map-identity`
- `effect.artist-31470-main-original-dxbc-replay`
- `effect.artist-31470-main-runtime-source-replay`
- `effect.artist-31470-main-warp-first-draw-contract`

다만 대규모 기존 dirty tree의 stale receipt/tool identity, unrelated map/rendering/Artist31210/WModel harness 계약 등 30개가 함께 실패해 전체 exit code는 1이었다. 이는 Track A의 deep replay 및 D/R first draw PASS를 Product/visual PASS로 승격하지 않는 별도 repository 상태다.

## 8. 아직 열지 않은 경계

- `MeshParticle -> LocalVF` native call-chain과 occurrence-selected runtime admission은 `false`다.
- original `c0.x` external opacity의 source-exact owner는 미복구이며 runtime parity는 neutral `1` 경계를 명시한다.
- pass/fog carrier는 identity-neutral fixture다.
- source-exact sampler policy는 미확정이다.
- original non-RT0 MRT 전체는 이번 복원 범위가 아니다.
- Product admission과 occurrence-level 사용자 visual admission은 모두 `false`다.

## 9. 사용자 수동 화면 검증 경로

자동 gate 이후 사용자가 직접 확인할 경로는 다음과 같다.

```text
Client Debug 직접 실행
  -> F1 Developer Tools
  -> Effect Tool
  -> All Effects
  -> "Skill | F | V4 Material Composition | Main Review + Diagnostics"
  -> "Play V4 Main Review (3)"
```

확인 대상은 `#9/#10` Watertrail core와 `#11` Spritewave outer carrier의 합성, dissolve 경계, main texture R-channel mask, dynamic parameter 반응, 재생 시작/종료 timing이다. 이 관찰 결과는 사용자가 서면으로 판정하기 전까지 `PENDING`이다.

## 10. 2026-08-13 후속 사용자 material 관찰

이 RESULT의 자동 checkpoint 뒤 사용자가 동일 main review 화면을 첨부하고 “이번 부분은 합격”이라고
서면 판정했다. 후속 상태는 다음처럼 분리한다.

| 항목 | 상태 |
|---|---:|
| `#9/#10` Watertrail + `#11` Spritewave RT0 material composition | `USER MATERIAL PASS` |
| anchor·orientation·mesh pivot | `PENDING` |
| lifetime 전체 timing·RNG·clock origin | `PENDING` |
| original non-RT0 MRT, sampler, pass/fog | `PENDING` |
| occurrence 전체 승인 | `0/35` |
| Product admission | `false` |

따라서 위 9절의 `PENDING`은 자동 checkpoint 종료 당시의 기록으로 보존하되, 현재 material sub-check는
사용자 합격으로 갱신한다. 이를 occurrence 전체 `visual PASS`나 Product 승인으로 승격하지 않는다.
