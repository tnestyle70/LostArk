# 2026-08-11 R8 Offline Raw Resource Preparation Result

## 현재 완료 상태

첫 검증 단위인 `DENOMINATOR_CHECKPOINT`를 clean baseline
`7d3e957f4d93bfd1416fa6a05d5d7fa8f46c12a2`에서 동결했다. 이 결과는
`lostark.raw-resource-inventory-v1`의 offline 입력 증거이며 runtime authority가 아니다.
둘째 검증 단위에서 fresh raw export, DDS/WModel structural inspection과 final portable inventory도
완료했다. 남은 일은 GPU schema 동결 뒤의 별도 final binding이며 이번 lane에는 포함하지 않는다.
이 raw manifest SHA는 M0 task `019fef62-5979-7ab2-8b1b-13b6d6891ac8`와 분리된 입력 증거이며
M0 prerequisite가 아니다.

`Client/Bin/Resources`, runtime Catalog/DataFiles, shared C++, Artist F M0/GPU/Product 파일은
수정하지 않았고 MSBuild lease도 사용하지 않았다. admission은 `Execute`, `Submit`, `Render`,
`Product` 모두 false이며 `rendererReady`와 `runtimeAuthority`도 false다. GPU sampler/state schema와
final binding은 생성하지 않았다.

## 동결한 분모와 portable provenance

4-class 분모는 Artist 31470을 제외한 51 skill, 74 stage, 113 clip occurrence,
5,232 imported source occurrence다. product cue 101개는 별도 projection으로 유지했다.
occurrence-backed source system은 424개이고 normalized graph 전체 442개 중 18개는
`GRAPH_ONLY_INACTIVE_FOR_SELECTED_OCCURRENCES`로 분리했다.

| Class | Raw request | Mesh | Texture |
|---|---:|---:|---:|
| Artist | 253 | 31 | 222 |
| DimensionMaster | 414 | 58 | 356 |
| LanceMaster | 332 | 59 | 273 |
| Warlord | 328 | 47 | 281 |
| class consumer 합계 | 1,327 | 195 | 1,132 |
| casefold unique object | 835 | 135 | 700 |

Warlord는 98개 occurrence-backed source system으로 action-bound catalog를 다시 join했다.
graph binding에는 없지만 catalog가 Warlord 17820 consumer를 소유하는
`fx_tex_02.fx_d_environ_018`을 보존해 Warlord 분모를 328개로 만들었고, 이 차이는 blocker로
남겼다. 4-class physical UPK 27개는 exact-case filename, byteSize, SHA-256이 현재 설치본과
모두 일치하며 총 405,147,985 bytes다.

Valtan 분모는 170 action, 2,464 stage, 2,378 clip, 21,931 notify다. typed occurrence는
Particle 6,159, Decal 536, Trail 430, Material 606, Camera 1,022로 총 8,753개이며 generic
Effect 3,787개는 quarantine으로 유지했다. Action-only raw request는 377개, mesh 47개,
texture 330개다. graph/direct physical UPK 32개는 모두 exact-case filename, byteSize,
SHA-256이 일치하며 총 269,267,947 bytes다.

세 raw LOA는 basename, byteSize, SHA-256으로 portable pin했다.
raw LOA archive/entry provenance 입력은 `data3_reextract_20260805/manifest.json` 6,235,429 bytes,
SHA-256 `02d104c24b5df625b3bf9bf021513f734e763a666042726eeafd62192effff54`이다.
Valtan occurrence 분모 입력은 세 `*.action-effects.json` document이며,
`all_actions/action-effect-source-manifest.json`은 이 generator가 소비하지 않는 upstream index/reference다.

| Raw LOA | Byte size | SHA-256 |
|---|---:|---|
| `MN_RPBF_00.loa` | 7,450,184 | `f61df383bf20634ccdc5b0db3eb9dde1bc62c78717ca763a325e56285e370797` |
| `MN_RPBF_01-1.loa` | 366,062 | `6fea6fb228d95a019fa9c6d42e2a298ca334be20ed186a84c98027015846fc2a` |
| `MN_RPBF_02-2.loa` | 645,481 | `60b6b1b17633e76a7a02a3c2514f2e54eebdbc7625354da697d46b924488ea4d` |

## blocker와 PASS로 승격하지 않은 증거

checkpoint status는 `FROZEN_WITH_BLOCKERS`이고 evidence-derived blocker는 18개다.

- Artist 31930 imported document expected/actual SHA mismatch 1개
- Warlord 17110 normalized graph expected/actual SHA mismatch 1개
- stage manifest가 가진 absolute external source hint 14개
- Warlord action catalog와 normalized graph binding의 ownership 차이 1개
- pin된 historical `data3.lpk`와 현재 설치 archive의 byte drift 1개

기존 runtime resolver에서 LanceMaster 82개와 Artist 109개, 총 191개가 missing으로 보이지만
필요 physical UPK는 모두 존재한다. 이 191개는 raw package missing이 아니라
`PREEXISTING_RUNTIME_RESOLVER_GAP` 힌트로만 기록했다. 기존 export receipt, absolute path,
재사용 staging, scale-100 WModel과 TGA 재인코딩 결과는 PASS 입력으로 승격하지 않았다.

validator는 report 배열을 신뢰하지 않고 pinned artifact의 `pinStatus/pathKind`, Warlord consumer
ownership, physical package status, Valtan LOA/archive evidence에서 blocker projection을 다시
계산한다. GPU schema boundary 변조와 evidence/report/status를 함께 바꾸는 coordinated false
promotion도 frozen blocker projection SHA로 거부한다.

## frozen checkpoint identity

| Identity | Value |
|---|---|
| file | `Data/Effects/Imported/RawResourceInventory/R8.raw-resource-denominator.checkpoint.json` |
| byte size | 21,107,545 |
| raw file SHA-256 | `42b437d59bf56c713aa5e53dcd73d7d2ad20fb510dfca73a4c2cacccabe16db5` |
| canonical self digest | `8b90b1cfa917d4fc102d619468016d812fce6c73f21b0a1547eda504b31a07fb` |
| blocker evidence SHA-256 | `1c8ed35f509e16281c8214d6920881714bdffcbaac451c2ddb3b05c1acdf8a4d` |

이 SHA는 denominator checkpoint의 identity다. 사용자가 요청한 최종 frozen raw manifest SHA는
fresh extraction과 structural inspection이 끝난 뒤 별도로 기록한다.

## 실행 검증

다음 검증은 checkpoint 파일이 안정된 뒤 다시 실행했다.

```powershell
python -m py_compile Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py Tools/LevelPlacementExtractor/test_prepare_r8_raw_resource_inventory.py
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate --manifest Data/Effects/Imported/RawResourceInventory/R8.raw-resource-denominator.checkpoint.json
python -m unittest Tools.LevelPlacementExtractor.test_prepare_r8_raw_resource_inventory -v
git diff --check
```

- strict checkpoint validation: PASS
- focused unit test: 11/11 PASS
- independent checkpoint review: PASS; 4-class/Valtan denominator, Warlord 328 rows,
  59 physical UPK identities와 세 digest를 별도로 재현
- `git diff --check`: PASS

`Tools/ProjectAudit/Invoke-ProjectAudit.ps1`도 실행했으나 repository baseline의 18개 기존 audit
group 때문에 exit 1이었다. 내부 Python suite의 91/91, 21/21, 6/6, 3/3 test는 통과했다.
실패 group은 기존 map extracted-runtime-root/data visibility/G09 path, Artist authority lane branch
guard와 exact-DDS/WFX/readiness, 기존 four-class Artist 31210 stage/binding mismatch, canonical
character asset 누락, 미빌드 WModel harness였다. 이번 offline checkpoint가 shared C++, Catalog,
Product 또는 canonical Resources를 수정한 결과는 아니다.

## fresh extraction과 raw 구조 검사 완료

checkpoint raw SHA prefix와 run ID를 넣은 previously-nonexistent staging을 한 번 만들었다.

```text
external:RawResourceInventory/Runs/R8-42b437d59bf56c71-20260811T1630KST-v1
```

이 run 전에 같은 경로는 존재하지 않았다. 기존 effect export staging은 읽거나 재사용하지 않았다.
UModel `-nooverwrite`는 이 fresh root 안에서만 사용했다. 4-class 27개와 Valtan 32개 package
identity를 현재 설치본 bytes와 다시 대조했고 59/59가 일치했다. raw object를 실제로 가진
logical package는 UModel `-list -nameresolve`가 출력한 physical UPK filename까지 checkpoint의
exact-case mapping과 일치한 뒤에만 export했다.

| Tool | Byte size | SHA-256 |
|---|---:|---|
| `umodel_lostark_v7.exe` | 1,766,400 | `b9573cdcbb7e9d26dbf60a0e3af47fb5af8543140873da8483c26d58cf40b249` |
| `ModelAssetConverter.exe` | 169,984 | `0aa6b7e659f0c10d72574aa2e5c9f1cf1c47a5753a81d9d0e1e4d700e3b07a63` |

converter companion DLL도 basename, byteSize, SHA-256으로 receipt에 함께 pin했다. Mesh는 raw
glTF/bin을 보존한 뒤 `--pretransform --scale 1 --no-auto-textures`로 별도 WModel staging에만
썼다. 이 명령은 `OFFLINE_TRANSIENT_COOK` provenance 사실일 뿐 final `geometryPreScale`이나
runtime renderer policy가 아니다.

extraction receipt는 1,212개 request를 모두 유지한다. UModel export invocation은 71개,
direct raw package `-list -nameresolve` preflight는 44개이고 process failure는 0이다. 추가 Valtan
source package pin 15개는 byte identity를 다시 대조했지만 direct raw object export 대상은 아니다.
stage의 receipt 제외 1,961 files / 119,564,185 bytes를 relative path, byteSize, SHA-256으로
재검증했다.

| Payload kind | Alias 포함 count |
|---|---:|
| DDS | 1,026 |
| glTF | 182 |
| glTF buffer | 182 |
| WModel | 182 |
| TGA | 4 |
| 합계 | 1,576 |

DDS 1,026개는 requested texture primary이고 DXT1/DXT5/ATI2의 header, dimensions, mip count,
block payload와 top-level linear size가 모두 일치했다. fresh stage 전체의 DDS 1,045개도 같은
strict parser를 통과했다. DX10 parser는 raw DXGI/resource fields와 typeless block byte layout만
기록하며 colourspace/SRV를 결정하지 않는다.

scale-1 WModel 182개는 모두 WINT/WMOD/WMSH 1.0, exact two-section static container,
contiguous submesh vertex/index ranges, finite 12-float vertex channel, local index bounds,
WMA2/WMAT entry indices와 embedded/derived bounds 검사를 통과했다. parser 결과에는 sampler,
colourspace, material packing, geometry preScale, cache, renderer packet 결정이 없다.

request 1,208개는 `EXPORTED_AND_STRUCTURALLY_INSPECTED`다. 다음 네 texture만 UModel이 TGA로
내보냈고 bytes를 그대로 보존한 `UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY` blocker다.

| Corpus | Source object | Byte size | SHA-256 |
|---|---|---:|---|
| FourClass | `efmaster_material_prologue.tex.flat_gray` | 30 | `f8ab42c2fd476f9b429b30285bd08598f2d90c08bfd86fc3e2594804382d68c5` |
| FourClass | `fx_tex_nomipmap_00.fx_c_flow_004` | 92,082 | `fd8fb70e3877fd53e2f93517b1c76a7f5b8ec724f7a15da655c6888eb8bcd8a8` |
| Valtan | `efmaster_material_prologue.normal` | 30 | `b3e5eb285090805d36c88fe1a519fc5fd48166f72e429cd5e3b404a8747a4c95` |
| Valtan | `efmaster_material_prologue.flat_red` | 30 | `fdcdd9f3be5f5bde30d8fbe7e8e4537ba01299177b2ea371ef9f2ec98868483a` |

이 네 파일은 DDS로 재인코딩하지 않았다. raw missing, output ambiguity, structural failure,
case-only candidate collision과 same-candidate/different-byte collision은 각각 0개다. basename-only
collision은 mesh `fm_k_helix_01`과 texture `sk_wgl_gdd_01_d` 두 group이며 full source object path를
candidate ID에 보존해 disambiguate했다.

1,576 payload alias는 1,277 unique SHA로 모였고 273개 shared-byte group에 모든 request alias를
보존했다. dedup 때문에 1,212 request denominator나 class/Valtan consumer를 제거하지 않았다.
UModel이 같은 fresh run에서 만든 denominator 밖 sidecar/dependency 88개도 unassociated fresh output
identity로 따로 남겼고 runtime candidate로 올리지 않았다.

## frozen raw-resource-inventory-v1 identity

| Identity | Value |
|---|---|
| file | `Data/Effects/Imported/RawResourceInventory/R8.raw-resource-inventory-v1.json` |
| byte size | 26,308,032 |
| raw file SHA-256 | `578fadda10903e4935bb633947843aeefe709c55dfa4767fa2fb62ad4817e500` |
| canonical self digest | `d6ec9db6dc7542b45eaecd9afb31e55aa817c25382b86362dbdb532843f14a82` |
| extraction receipt raw SHA-256 | `cf58ad0b26b7f24e0d0cd91e713f14bf96a3c9c14d4284a60415512977980137` |
| raw resource projection SHA-256 | `c7d9b7d7a8fc9b82627accf34b5ce6b2f21e20eb76b6458b81ba31c67a800e4b` |
| raw blocker evidence SHA-256 | `c08c2f3db4204da5d453bb9050dcfa901477faf6a3bd0684028ea76c84b0694b` |

inventory status는 기존 corpus/provenance blocker 18개와 raw TGA blocker 4개를 합친
`FROZEN_WITH_BLOCKERS` 22개다. final raw manifest는 `OFFLINE_INVENTORY_ONLY`이고
`runtimeAuthority=false`, `rendererReady=false`, Execute/Submit/Render/Product=false다.

final validator는 extraction receipt raw SHA와 모든 request/raw payload/inspection/package/tool
projection을 frozen SHA로 대조한다. report, request status, blocker evidence와 summary를 함께 고쳐
false promotion하고 self digest를 다시 계산해도 거부한다.

## 최종 검증

```powershell
python -m py_compile Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py Tools/LevelPlacementExtractor/test_prepare_r8_raw_resource_inventory.py
python -m unittest Tools.LevelPlacementExtractor.test_prepare_r8_raw_resource_inventory -v
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate-final --manifest Data/Effects/Imported/RawResourceInventory/R8.raw-resource-inventory-v1.json
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py verify-final-stage --manifest Data/Effects/Imported/RawResourceInventory/R8.raw-resource-inventory-v1.json --staging-root C:\Users\user\Desktop\Resource_LostArk\05_Reports\RawResourceInventory\R8-42b437d59bf56c71-20260811T1630KST-v1
git diff --check
```

- focused unit test: 20/20 PASS
- final portable manifest validation: PASS
- external frozen stage 1,961 artifact identity verification: PASS
- fresh stage 전체 DDS 1,045 / WModel 182 strict structural inspection: PASS
- final binding/materializer generation: 실행하지 않음
- canonical Resources/Catalog/DataFiles/shared C++/Product 변경: 없음
- MSBuild lease/build: 사용하지 않음
- ProjectAudit: exit 1, checkpoint 실행과 같은 18개 existing failure group; PASS로 기록하지 않음

final unit에서 ProjectAudit을 다시 실행했다. 내부 suite 91/91, 21/21, 6/6, 3/3은 통과했고
전체 audit은 기존 18개 group으로 exit 1이었다. `projects.data-source-visibility`는 offline final JSON을
포함해 `expected=1523/project=1497/filters=1497`이고, Artist authority lane branch guard는 이번 별도
R8 PLAN을 out-of-scope로 본다. 그 밖에는 기존 map extracted roots, G09 path/contract, Artist exact
DDS/WFX/readiness, Artist 31210 stage/binding mismatch, 미빌드 WModel harness, canonical character
asset 누락이다. Product project와 runtime DataFiles를 이 lane에서 수정하지 않는 사용자 경계를 지켜
이 실패들을 PASS로 바꾸거나 범위를 넓히지 않았다.

## 남은 경계

Artist 31930과 Warlord 17110 pin mismatch, 14 absolute source hint, Warlord catalog/graph ownership
차이와 current `data3.lpk` drift를 별도 authority lane에서 해소해야 한다. 네 TGA는 GPU schema와
texture policy가 동결된 뒤에만 명시적 변환/format 결정을 할 수 있다. 그 전에는 이 inventory의
candidate ID나 scale-1 WModel을 runtime binding, materializer, Catalog 또는 Product authority로
승격하지 않는다.
