# 2026-08-10 Artist 31470 F ShaderCache Oracle Result

## 1. 결론

설치본 UE868 ShaderCache의 선행 native code table은 fail-closed parser로 닫았다. primary
`sc_lv_eflobby_sl_class`의 271개 code record는 전부 raw LZ4 block에서 해제되었고, DXBC total
size와 chunk bounds 및 고정한 `d3dcompiler_47.dll`의 disassembly를 271/271 통과했다. 중복 DXBC를
제외하면 240개다.

그러나 23개 Material과 cache shader map의 exact identity join은 0/23이다. 24개 static-permutation
MIC tail도 direct/endian/hash key join이 0/24이며, 모든 4-byte 정렬 nonzero 16-byte window 4,816개와
primary descriptor shader-ID candidate 271개의 교집합도 0이다. 따라서 arithmetic evaluator는 0,
execution/Product admission은 false로 유지한다.

## 2. 실제 해독 경계

ShaderCache serial의 검증된 선행 구조는 다음과 같다.

1. `netIndex=-1`, `None` property terminator, native header, shader platform ordinal
2. shader-type group count
3. group별 FName, 24-byte descriptor 배열, 같은 수의 code record 배열
4. code record별 `uncompressedSize`, `compressedSize`, raw LZ4 block
5. DXBC container header/chunk와 D3D disassembly
6. code table 뒤의 shader-object/material-map native tail

6번의 variable record 구조는 아직 해독하지 않았다. 압축 block 안의 우연한 `DXBC` marker를 직접
disassemble하면 실패하며, raw marker를 uncompressed container로 세탁하지 않는다.

## 3. Material과 MIC join 결과

- base Material native-tail key: 23개
- direct/reverse-all/u32 endian/GUID field endian/u32 word-order cache match: 0/23
- recipe: 27개
- MIC: 25개
- static-permutation native tail이 있는 MIC: 24개
- MIC key의 endian/order 및 MD5/SHA-1/SHA-256 cache match: 0/24
- MIC aligned nonzero 16-byte windows: 4,816개
- primary descriptor shader-ID candidates: 271개, unique 271개
- exact window intersection: 0개
- 판정: `MIC_TAIL_CONTAINS_NO_DIRECT_SHADER_OBJECT_ID`

이 결과는 direct bridge가 없다는 bounded evidence다. native tail field 구조를 해독했다는 뜻이 아니며,
shader permutation join 또는 SOURCE_EXACT evaluator로 승격하지 않는다.

## 4. 중복 Material topology 조사

중복 후보 탐색 범위는 reconstructed DimensionMaster material report 30개로 제한했다. 이 inventory는
21/23 family, 22 Material row를 찾았고 같은 package·같은 leaf의 alternate object 1개를 찾았다.
report는 후보 검색에만 사용하며 topology 수치의 근거로 사용하지 않았다.

실제 raw package에서 비교한 matrix는 source exact dependency 23개, current DKV cross-revision 23개,
alternate object 1개로 총 47개 row다. DKV의 expression/null/unresolved-edge 수는 source exact와 23/23
모두 같았다. alternate `bfx_mi.bfx_d_pa_circ_01_ad`는 46 entry / 30 null / 14 unresolved edge로,
source `bfx_m.bfx_d_pa_circ_01_ad`의 45 / 29 / 14보다 완전하지 않았다. source와 같은 expression-entry
denominator에서 null과 unresolved edge를 동시에 악화시키지 않는 strict Pareto 개선 후보는 0개다.

이 inventory는 전체 설치 package export inventory가 아니다. 따라서
`DUPLICATE_MATERIAL_INVENTORY_NOT_INSTALLATION_EXHAUSTIVE`를 유지한다.

## 5. 고정한 외부 identity

- ShaderCache UPK: 270,965,156 bytes,
  SHA-256 `be77e8af4443c4cca5614bec0545c0c735ab04a8b68a3781fb9dfb5a5f2123ad`
- global Material DKV: 141,154,941 bytes,
  SHA-256 `c0c3e35b48d8589d2e5014c99c64c0c32e05eace7ae02cfc8e6566f4eaf40150`
- primary cache serial: export 318, 1,034,215 bytes,
  SHA-256 `ad990dc604d784cef5aebee575aed3798e6acb7ab78b7654a0e4c5cc8f2a2b28`
- `d3dcompiler_47.dll`: 4,916,800 bytes, version `10.0.22621.5040`,
  SHA-256 `ce013eb1639f8e2620a509e73b33029108f55a293e304e33e38b72fd65c531b8`

외부 UPK/DLL/report는 raw-byte hash, Git tracked parser dependency는 LF/no-BOM canonical text hash를
사용한다.

## 6. 남은 blocker와 최소 요구 증거

- `MATERIAL_SHADER_MAP_KEY_UNRESOLVED`
- `MIC_STATIC_PARAMETER_SET_NATIVE_TAIL_UNPARSED`
- `MIC_TAIL_TO_SHADER_OBJECT_IDENTITY_UNRESOLVED`
- `SHADER_OBJECT_AND_MATERIAL_MAP_NATIVE_TAIL_UNPARSED`
- `DXBC_REGISTER_BINDINGS_NOT_MATERIAL_PARAMETER_NAMES`
- `ARITHMETIC_GRAPH_TO_SHADER_INVERSION_UNPROVEN`
- `DETERMINISTIC_NUMERIC_SAMPLE_ORACLE_UNAVAILABLE`

다음 승격에는 UE868/licensee16 shader-object/material-map/static-parameter-set decoder, source-revision
Material state에서 shader-map key로 가는 derivation, 23/23 permutation identity join, permutation별
constant/texture/sampler semantic layout, fixed-input/output numeric sample oracle가 필요하다.

## 7. 검증

- Python unit: 13 tests PASS
- shallow focused audit: PASS
- deep focused audit: 실제 16 source UPK + DKV + ShaderCache UPK + 30 report + DLL 재검증 PASS
- full `Invoke-ProjectAudit.ps1`: 새 `effect.artist-31470-shader-cache-oracle` check PASS. 전체는 이
  worktree에 없는 runtime resource, 기존 source/material receipt mismatch, 미빌드 geometry harness,
  four-class rollout stage mismatch 등 기존 13개 check 때문에 exit 1
- receipt JSON parse: PASS
- `git diff --check`: PASS
- 이미지·육안 검증: 수행하지 않음
