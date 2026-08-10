# 2026-08-10 Artist 31470 F ShaderCache Oracle Plan

## 1. 목표와 경계

이 문서는 최종 복원 계획의 G05-M 선행 증거 획득 단위다. 설치본의 전역 Material 패키지와
ShaderCache 패키지를 직접 해독해 23개 arithmetic family가 어느 shader permutation과 연결되는지
판정한다. 연결이 증명되지 않으면 evaluator를 추측하지 않고 Product를 계속 차단한다.

이미지·육안 비교는 수행하지 않는다. 이 단위는 Material evidence contract, runtime compiler,
renderer와 기존 G03 산출물을 수정하지 않는다.

## 2. 입력 정본

- `DKV6KRSCXY3T6D9CJIK3G.upk`: UE 868 전역 Material 패키지
- `9XUFAXIP8BXBAP1NIEG66EF.upk`: UE 868, ShaderCache export 1,596개
- `skill.31470.typed-material-evidence-contract.json`: 23 family identity denominator를 읽기만 한다.
- `extract_ue3_placements.py`: Lost Ark AES/LZ4/UE3 package decoder를 재사용한다.
- Windows SDK x64 `d3dcompiler_47.dll`: 압축 해제된 DXBC disassembly 검증에만 사용한다.

외부 UPK와 DLL은 raw byte SHA-256/size/version을 사용하고, Git tracked Python/JSON은 LF/no-BOM
canonical text SHA-256을 사용한다.

## 3. 구현 파일

### G05-M01. ShaderCache parser

`Tools/LevelPlacementExtractor/extract_artist_31470_shader_cache_oracle.py`

- export의 `netIndex=-1`, `None` property terminator, native header, shader-type group 수를 검증한다.
- 각 group의 FName, descriptor 24-byte row, code row 수를 순서대로 읽는다.
- code row를 `[uncompressedSize][compressedSize][raw LZ4 block]`로 해독한다.
- DXBC header total size/chunk bounds를 검사하고 SDK disassembly가 성공한 row만 decoded로 센다.
- DKV의 exact 23 family object를 class/path/export로 결합하고 native-tail 16-byte key를 보존한다.
- direct, reverse-all, per-u32 endian, GUID text-field endian, u32 word-order key를 cache 전체에 검색한다.
- 27개 recipe의 exact source package를 다시 읽어 25개 MIC와 24개 static-permutation native tail을
  고정한다. MIC key에는 endian/order 변형과 MD5/SHA-1/SHA-256 변형도 검색한다.
- 24개 MIC tail의 모든 4-byte 정렬 nonzero 16-byte window와 primary descriptor shader-ID 후보의
  교집합을 계산한다. hit가 있어도 native field boundary decoder 전에는 join으로 승격하지 않는다.
- 30개 reconstructed material report는 duplicate 후보를 좁히는 용도로만 사용한다. 후보 topology는
  report 값을 신뢰하지 않고 exact source package와 current DKV export에서 다시 해독한다.
- source exact 23개, current DKV 23개, 동일 package·동일 leaf alternate object를 completeness matrix로
  비교한다. expression-entry denominator가 같은 후보만 null/edge Pareto 개선 후보가 될 수 있다.
- 어느 변형도 shader-map identity가 아님을 전제로 하지 않고 결과가 0이면 blocker를 유지한다.
- 후반 variable native shader-object/material-shader-map table은 완전 decoder가 없으면 opaque hash와
  최소 요구 증거만 기록한다.

### G05-M02. Receipt

`Data/Effects/Imported/Artist/Materials/skill.31470.shader-cache-oracle.receipt.json`

- package/export/tool/DLL identity
- 23 Material native-tail key rows
- 27 recipe / 25 MIC / 24 static-permutation native-tail key rows
- bounded inventory 30 report / 21 family / 22 row 및 raw-package topology completeness matrix
- 선택한 class-select/lobby cache row
- primary `sc_lv_eflobby_sl_class` 271 code row 및 aggregate digest
- profile/resource/sampler/constant-buffer declaration summary
- Material key join denominator와 blocker set
- `executionAdmission=false`, `productAdmission=false`

### G05-M03. Mutation tests와 ProjectAudit

- `test_extract_artist_31470_shader_cache_oracle.py`
- `Test-Artist31470ShaderCacheOracle.ps1`

synthetic serial에서 정상 group, truncated block, forged compressed/uncompressed size, corrupt LZ4,
corrupt DXBC total size, descriptor/code count mismatch, raw compressed marker의 direct D3D 실패,
GUID endian 변형 미결합과 blocker 보존을 검증한다. deep audit는 실제 두 UPK와 DLL을 다시 읽어
tracked receipt와 비교한다. shallow audit는 receipt denominator와 Product 차단만 검사한다.

inventory version의 JSON integer 형식, 보고서 source package identity, topology denominator와
Pareto 판정의 mutation도 실제 helper와 receipt validator로 검증한다.

## 4. 종료 조건

- primary cache LZ4 decode 271/271
- DXBC total-size 271/271
- D3D disassembly 271/271
- unique DXBC 240
- 23 Material exact row 23/23
- recipe 27, MIC 25, static-permutation native-tail key 24
- bounded inventory 30 report / 21 family / 22 row / alternate object 1
- raw-package topology matrix 23 source + 23 DKV + bounded alternate, source보다 엄격히 개선된 후보 수 기록
- direct/common endian key join 결과를 receipt에서 재도출
- Material→shader permutation exact join이 없으면 `MATERIAL_SHADER_MAP_KEY_UNRESOLVED`
- numeric sample oracle가 없으면 `DETERMINISTIC_NUMERIC_SAMPLE_ORACLE_UNAVAILABLE`
- evaluator implemented 0, execution/Product false
- focused unit, shallow/deep audit, JSON parse, `git diff --check` PASS
