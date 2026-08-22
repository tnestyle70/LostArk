# Missing Effect Family ABI 복원 구현 계획

branch: `codex/missing-effect-family-recovery`

base: `origin/main@7fb8f8139f62657914228070ebe2a9860287b577`

## 0. 현재 실제 반영 상태와 이번 경계

이번 작업은 스킬 전체를 다시 손튜닝하거나 `translucent/additive/mask` 세 종류로 모든 Material을
평균내는 작업이 아니다. 원본 occurrence에서 시작해 carrier, child MIC, parent Material,
`FStaticParameterSet`, exact ShaderMap/DXBC와 runtime binding을 순서대로 닫고, 한 family의 단일
occurrence를 사용자가 solo로 판정한 뒤 같은 family의 다른 occurrence로 확대한다.

현재 Product의 공통 `effect.ue3.grouped-translucent.v1`은 texture role을 몇 개의 공용 lane과 하나의
UV scale/pan으로 축약한다. 이 경로는 fallback으로는 유효하지만 parent마다 다른 alpha 경계,
여러 UV domain, dynamic parameter, masked clip, scene depth와 MRT를 원본과 같게 재생할 수 없다.
따라서 suffix의 `tr/ad/msk`는 blend/cutout 상태의 단서일 뿐 family 수식의 정답으로 사용하지 않는다.

복원 단위는 다음과 같다.

```text
carrier
  SpriteParticle / MeshParticle / LocalDecal / Trail / ScreenPost / CModel

child MIC
  texture·scalar·vector override와 static switch를 소유한다.

parent Material family
  UV, mask/coverage, radiance, distortion, dissolve와 출력 수식을 소유한다.

static permutation
  같은 parent 안에서도 실제 ShaderMap과 DXBC를 선택한다.

runtime ABI
  CB lane, SRV/sampler, vertex varying, render state, scene input과 MRT를 연결한다.
```

carrier가 없으면 texture를 바꿔도 decal이나 dragon이 생기지 않는다. parent 수식이 없으면 정확한
texture와 mesh를 연결해도 카드 사각형, 잘못된 UV scroll과 과노출이 남는다. DXBC만 있더라도 CB,
SRV/sampler와 VF/pass가 틀리면 원본 수식은 잘못된 입력으로 실행된다.

## 1. 요청 스킬의 실제 scope

| 캐릭터·슬롯 | skillId와 현재 문서 | 현재 핵심 판정 |
|---|---|---|
| 워로드 F | `17140.unified` | 56행 전부 particle이다. WaterTrail 6, Simple01 6, LensFlare 1을 제외한 43행은 grouped fallback이다. `decmaster` 5행도 Sprite이지 LocalDecal이 아니다. |
| 워로드 T 풀배럴 | `17240.ba1/ba2/ba3.unified` | 원본 BA3에는 LocalDecal 4개가 있으나 현재 Product BA3 8행에는 decal이 0개다. carrier 누락과 DecMaster material ABI 누락을 분리한다. |
| 차원술사 W | `2050120.clip2/clip3.unified` | clip3의 Glasshole/FluidNinja/Slice/Helix가 남아 있다. 5 family는 exact DXBC까지 복구됐고 Product ABI가 미완료다. |
| 차원술사 S | `2050220.unified` | circle/shine와 grouped family 14행이다. 원본의 SpriteWave translucent 2개는 현재 Product에서 빠졌다. |
| 차원술사 F | `2050230.unified` | 현재 8행에는 bounded Fluid 2행만 남았다. 원본 69행의 Glasshole/Slice/SpriteWave와 screen-space shard는 별도 복구가 필요하다. |
| 도화가 T 미르세김 | `31950.unified` | effect 쪽은 `fm_j_helixline_1` MakeFlow03 mesh 3행이다. 용 본체는 effect sprite가 아니라 source combined mesh animation의 CModel 소유다. |
| 창술사 V | `34610.clip1/clip2/clip3.unified` | screw 핵심은 `fm_m_helix_006` MakeFlow02 mesh twin이다. strict profile 36은 있으나 해당 child dynamic 4 lane 의미가 ShaderMap으로 닫히지 않았다. |
| 창술사 T 창 폼 용 | `34650.clip1/clip2.unified` | Z가 아니라 T다. 용은 sprite가 아니라 `fm_x_flm_gdr_01(_dragon).wmodel` mesh particle 2행이다. 원본 parent는 Masked인데 현재 grouped alpha two-sided로 잘못 admission된다. |

창술사 V screw의 첫 row는
`authored.source-particle.3d1e2651b12b85107f415cbb`, twin은
`authored.source-particle.10e234d088d783b587a5d398`이다. parent MakeFlow02는 전체 corpus에서
현재 EffectCatalog-admitted 문서 기준 31 occurrence/16문서에 재사용된다.

창술사 T dragon의 두 row는
`authored.source-particle.0a019ebaff2bb55941d23ab8`와
`authored.source-particle.65b74589de96c3f44e625f24`다. parent
`fx_m_mi_00.fx_m.fx_d_me_master_01_ph_msk`는 현재 EffectCatalog-admitted 문서 기준
26 occurrence/7문서에 재사용된다. parent props는
`BLEND_Masked`, one-sided, opacity mask clip `0.166`이지만 현재 두 row는 grouped translucent와
alpha two-sided로 실행된다.

기존 `effect-family-manifest.v1.json`은 현재 builder `--mode check`에서 stale이다. 그러므로 위 수치는
stale manifest summary가 아니라 현재 EffectCatalog와 Authored 문서를 직접 join한 값이다. broad manifest
재생성은 이번 Glasshole texture closure 커밋에 섞지 않고 별도 검증 단위로 처리한다.

## 2. 도화가 F와 이번 family의 차이

도화가 F는 DXBC에서 몇 개의 색상 상수만 읽어 HLSL을 새로 만든 것이 아니다.

```text
active MIC FStaticParameterSet
-> 동일 배포 cohort의 exact FMaterialShaderMap
-> LocalVF BasePass PS와 VS permutation
-> native shader-object binding과 CB/SRV/sampler register
-> raw 원본 PS DXBC WARP numeric replay
-> 원본 RT0 수식의 shipped HLSL translation
-> 원본 DXBC와 runtime shader의 constant/spatial parity
-> Debug/Release production-path WARP first draw
```

이 경로로 WaterTrail/SpriteWave main family의 exact map 2/2, raw PS 21/21,
runtime constant 19/19와 spatial 15/15를 닫았다. 그래도 source-exact sampler, non-RT0 MRT,
native VF/pass 전부를 완료했다고 기록하지 않았다. 즉 hash와 byte boundary는 identity를 고정하지만
수식의 의미와 runtime input ABI를 대신하지 않는다.

차원술사 W glass는 같은 추출 경로가 이미 ShaderMap/DXBC/native binding까지 성공했다. 실패 지점은
추출 이전이 아니라 source sampler/default, scalar-group packing, renderer-owned CB/scene-depth와
actual VF/pass 뒤쪽이다. 따라서 glass family 복원은 불가능하지 않다.

## 3. 복원 증거 ladder와 admission

모든 family는 다음 순서를 건너뛰지 않는다.

1. 현재 Product occurrence와 carrier가 존재한다.
2. child MIC와 parent Material identity가 exact다.
3. effective `FStaticParameterSet`가 exact ShaderMap 하나를 고른다.
4. native shader object, DXBC와 CB/SRV/sampler wire가 exact다.
5. source texture bytes와 named texture role이 exact다.
6. source sampler/address/filter/color-space와 scalar/vector packing이 닫힌다.
7. 실제 carrier VF/pass varying, render state, engine scene input과 MRT가 닫힌다.
8. raw DXBC fixed/source-value replay와 translated HLSL parity가 통과한다.
9. 한 occurrence만 Authored canary로 fail-closed 실행한다.
10. Debug/Release focused build와 first draw 뒤 사용자가 solo visual을 판정한다.
11. 같은 `(parent + static permutation + carrier + render state + role set)` cohort만 확대한다.

`runtimeShaderProfileId`를 catalog에 추가하는 시점은 1~8의 근거가 family contract에 고정된 뒤다.
skill ID나 texture filename switch로 family를 선택하지 않는다. exact 증거가 없는 family는 기존 grouped
fallback을 유지하고 `exact`라고 이름 붙이지 않는다.

## 4. 첫 canary: 차원술사 W Glasshole02

첫 canary는 현재 Product에 살아 있는 다음 단일 Sprite다.

```text
effectAssetId  effect.dimensionmaster.skill.2050120.clip3.unified
elementId      authored.source-particle.40e1b48e2f0f88dcfeff1549
child MIC      fx_m_mi_k_00.fx_mi.fx_k_pa_glasshole_02_01_tr
parent         fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr
profile        ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2
PS DXBC        7,584 bytes
PS SHA-256     e2ba1c1ef87cdd52cc74a8e661f8613d0b17f2cc7b9b1d0d7ab6ed80ec6e775b
```

현재 exact PS는 material SRV 7개 `t0,t1,t3..t7`와 engine-owned `t2/s0`을 사용한다. source texture
identity는 7/7 exact지만 runtime DDS 두 개가 빠져 있었다. 두 source export는 receipt와 byte/hash가
정확히 일치함을 다시 확인했다.

| runtime asset ID | bytes | SHA-256 |
|---|---:|---|
| `Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_atypical_094_ycl.dds` | 65,664 | `8097e1011480df43f56ad42a0ab849c74b9d8a29c17c867556f1df68dd071041` |
| `Effect/DimensionMaster/Textures/FX_TEX_04/fx_f_aura_004_1.dds` | 32,896 | `80a7797447d457de7e56594951e2d91c12899d144a7e0c730a4a8da14fdca896` |

이번 첫 변경 단위는 이 두 immutable DDS의 source-exact runtime parity와 texture/sampler receipt 재생성만
닫는다. raw DXBC canary admission, Product profile 변경, authored tuning 변경은 같은 커밋에 섞지 않는다.

그 다음 변경 단위는 scalar-group lane/padding, `TF_Default`의 해당 revision TextureLODSettings,
생략된 Texture2D CDO default, `fparticle` BasePass varying과 external opacity/scene-depth owner를 닫는다.
이 증거가 닫히기 전에는 기존 bounded profile 29를 exact 원본이라고 승격하지 않는다.

## 5. 다음 canary 순서

1. **Glasshole02**: 이미 exact ShaderMap chain이 가장 깊고 현재 occurrence가 살아 있어 복원 방법 자체를 닫는다.
2. **창술사 T dragon masked mesh**: parent Masked/cull/clip과 7 texture lane, mesh UV/SubUV를 exact map으로 복구한다. catalog는 이 family가 닫힌 뒤 추가한다.
3. **창술사 V MakeFlow02 screw**: distinct opacity/diff/flow UV domain과 4 dynamic lane의 실제 ordinal을 복구한다.
4. **워로드 T LocalDecal emitter55**: LocalDecal carrier/projection을 먼저 한 행으로 복구하고, DecMaster01 lane/channel equation은 별도 family gate로 닫는다.
5. **차원술사 F Fluid와 screen shard**: world-space Fluid sprite와 screen-space textured shard carrier를 분리한다.
6. **도화가 T 미르세김**: effect helix family와 CModel 용 애니메이션 owner를 분리해 각각 검증한다.
7. 각 family가 통과할 때 차원술사 S/F, 워로드 F와 동일 family occurrence만 cohort 확대한다.

워로드 T LocalDecal의 첫 source row는 BA3 emitter55다. child는
`fx_m_mi_04.fx_mi.fx_d_de_master_01_81_tr`, parent는
`fx_m_mi_04.fx_m.fx_d_de_master_01_tr`이며 emitter55~58 네 행이 한 cohort다. 현재 Product BA3에는
carrier 자체가 없으므로 texture만 연결하는 수정은 금지한다.

## 6. 이번 G에서 추가·수정할 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_atypical_094_ycl.dds` | Git 제외 공유 runtime root에 배포하는 Glasshole exact PS의 `t1/s2` source texture |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/DimensionMaster/Textures/FX_TEX_04/fx_f_aura_004_1.dds` | Git 제외 공유 runtime root에 배포하는 Glasshole exact PS의 `t6/s6` source texture |
| 갱신 | `C:/Users/user/Desktop/LostArk-missing-effect-family-recovery/Data/Effects/Imported/DimensionMaster/Materials/skill.2050120.clip3.exact-texture-sampler-closure.receipt.json` | runtime DDS 7/7 parity를 재검증하는 생성 receipt |
| 갱신 | `C:/Users/user/Desktop/LostArk-missing-effect-family-recovery/Data/Effects/Contracts/ue3-exact-cooked-shader-variants.v1.json` | upstream texture receipt를 다시 소비하되 sampler/VF/Product admission은 false 유지 |
| 수정 | `C:/Users/user/Desktop/LostArk-missing-effect-family-recovery/Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py` | 과거 missing DDS를 불변식으로 고정한 parity denominator를 source-exact 5/5 상태로 갱신하고 tracked input path를 worktree-neutral로 봉인 |
| 수정 | `C:/Users/user/Desktop/LostArk-missing-effect-family-recovery/Tools/EffectPipeline/test_extract_ue3_material_texture_sampler_closure.py` | tracked receipt의 5/5 parity와 Glasshole admission을 회귀로 고정 |
| 추가 | `C:/Users/user/Desktop/LostArk-missing-effect-family-recovery/.md/GB/08-22/2026-08-22_MISSING_EFFECT_FAMILY_ABI_RECOVERY_RESULT.md` | 구현·자동 검증·사용자 수동 상태를 분리 기록 |

Git worktree에는 `Client/Bin/Resources`가 materialize되지 않으므로 두 DDS는 사용자가 실행하는 정본
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources`에 배포한다. source commit에는 binary나 복사본을
강제로 추가하지 않는다.

이 G에는 새 C++/HLSL이 없다. 따라서 `.vcxproj`와 `.vcxproj.filters` 변경도 없다. 다음 ABI G는
ShaderMap/source-default 실측 뒤 이 PLAN을 먼저 갱신하고 기존 C++/HLSL의 정확한 교체 블록을 고정한
뒤 구현한다.

### 6.1 `validate_receipt` 교체 블록

`Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py`의 기존 `validate_receipt` 함수 전체를
다음으로 교체한다. 5/5는 sampler나 Product admission이 아니라 runtime DDS byte parity만 뜻한다.

```python
def validate_receipt(receipt: dict[str, Any]) -> None:
    require(receipt.get("schema") == SCHEMA, "receipt schema mismatch")
    require(receipt.get("formatVersion") == FORMAT_VERSION, "receipt version mismatch")
    sealed = dict(receipt)
    claimed = sealed.pop("receiptSha256", None)
    require(claimed == canonical_json_sha256(sealed), "receipt digest mismatch")
    summary = receipt.get("summary", {})
    require(summary.get("exactTargetCount") == 5, "W exact target denominator changed")
    require(summary.get("upstreamBlockedTargetCount") == 1, "W blocked denominator changed")
    require(summary.get("uniformTextureBindingCount") == 24, "W texture wire denominator changed")
    require(summary.get("sourceExactTextureBindingCount") == 5, "exact texture closure failed")
    require(summary.get("uniqueEffectiveTextureCount") == 23, "effective texture denominator changed")
    require(summary.get("sourceExactSamplerTargetCount") == 0, "sampler blocker unexpectedly changed")
    require(summary.get("runtimeDdsParityTargetCount") == 5, "runtime DDS parity closure regressed")
    require(
        receipt.get("scope", {}).get("runtimeAdmission") is False
        and receipt.get("scope", {}).get("visualAdmission") is False,
        "runtime or visual admission must remain false",
    )
```

### 6.2 `source_descriptor` 교체 블록

기존 함수는 tracked input의 절대 worktree 경로를 receipt digest에 넣어 같은 commit도 worktree마다 stale로
만들었다. repository 안의 입력은 repository-relative path로, 외부 immutable source만 absolute path로
기록한다.

```python
def source_descriptor(path: Path, role: str) -> dict[str, Any]:
    resolved = path.resolve()
    try:
        display_path = resolved.relative_to(REPOSITORY_ROOT.resolve()).as_posix()
    except ValueError:
        display_path = resolved.as_posix()
    return {
        "path": display_path,
        "byteSize": resolved.stat().st_size,
        "sha256": sha256_file(resolved),
        "role": role,
    }
```

### 6.3 tracked receipt test 추가 블록

`TextureSamplerClosureTests.test_tracked_receipt_is_sealed_and_keeps_runtime_visual_closed`에서
`closure.validate_receipt(receipt)` 바로 아래에 다음 assertion을 추가한다.

```python
        self.assertEqual(receipt["summary"]["runtimeDdsParityTargetCount"], 5)
        glasshole = next(
            row for row in receipt["targets"]
            if row.get("targetId") == "dimensionmaster-w-glasshole-02"
        )
        self.assertTrue(glasshole["runtimeDdsParityAdmission"])
```

같은 test에 tracked extractor path가 absolute가 아님을 추가한다.

```python
        self.assertEqual(
            receipt["inputs"]["extractor"]["path"],
            "Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py",
        )
```

## 7. 검증

```powershell
python Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py --help
python Tools/EffectPipeline/test_extract_ue3_material_texture_sampler_closure.py
python Tools/EffectPipeline/materialize_ue3_exact_cooked_shader_variants.py --check
python Tools/EffectPipeline/test_materialize_ue3_exact_cooked_shader_variants.py
git diff --check
```

생성 도구는 현재 repository root의 두 DDS가 source byte/hash와 일치하는지 확인해야 한다. 성공해도
`sourceExactSampler`, `sourceValueReplay`, `actualVfPass`, `productRuntime`, `visual`은 false를 유지한다.
사용자 화면 판정은 raw canary가 실제로 연결되는 후속 G 전까지 요청하지 않는다.

## 8. 완료 조건

이번 G의 완료 조건은 Glasshole 7 texture의 runtime byte parity와 receipt 재검증이다. family 전체 완료
조건은 exact source-value replay, actual VF/pass, translated runtime parity, Debug/Release first draw와
사용자의 단일 occurrence visual 판정이다.

다음 항목은 완료로 기록하지 않는다.

- DXBC 파일이 존재한다는 이유만으로 exact family라고 부르는 것
- bounded profile 29가 사각형을 줄였다는 이유로 원본 수식이라고 부르는 것
- 창술사 dragon을 sprite family로 catalog에 추가하는 것
- 미르세김 CModel 용 본체를 MakeFlow03 particle로 대체하는 것
- LocalDecal carrier가 없는 문서에서 texture만 바꾸는 것
- sampler/VF/pass가 열린 raw DXBC를 Product 기본 경로로 켜는 것
- 사용자의 서면 관찰 없이 visual PASS를 기록하는 것
