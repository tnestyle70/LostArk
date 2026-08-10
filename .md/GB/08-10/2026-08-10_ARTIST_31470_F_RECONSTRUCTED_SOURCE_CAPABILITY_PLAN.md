# 2026-08-10 Artist 31470 F Reconstructed Source Capability Plan

## 목표

Source execution receipt가 차단한 29개 module occurrence를 원본과 동일하다고 승격하지 않고,
`RECONSTRUCTED_APPROVED_V1` 정책으로 실행 의미를 검토할 수 있는 7개 폐쇄형 capability family에
정확히 배치한다. 이번 단계는 capability evidence만 생성한다. Runtime C++ 실행과 Product admission은
계속 false다.

## 입력 정본과 금지 경계

- Source execution semantics: `c927e397811d4e5718efd27b187eb59775023685`
- Source oracle acquisition: `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3`
- 현재 revision의 script/default/native metadata는 `CURRENT_REVISION_CROSS_REVISION_EVIDENCE`로만
  보존한다.
- `sourceExact`는 모든 family와 occurrence에서 false다.
- unknown class, ownerless row, numeric sample 없는 READY, generic fallback은 모두 생성 실패다.
- Runtime C++, Material, Geometry 파일은 수정하지 않는다.

## G01. 29행 capability receipt

### 파일 역할

- `build_artist_31470_reconstructed_source_capability.py`: 세 frozen receipt를 검증하고 29행을
  7 family에 bind한 뒤 fixed seed/time numeric sample을 생성·검증한다.
- `test_build_artist_31470_reconstructed_source_capability.py`: family/property/hash/duplicate/non-finite/
  admission 반례를 실제 builder와 validator에 적용한다.
- `skill.31470.reconstructed-source-capability.receipt.json`: Runtime이 이후 소비할 수 있는
  source-only typed capability evidence다.
- `Test-Artist31470ReconstructedSourceCapability.ps1`: unit, generator stale check, upstream source audit,
  denominator와 fail-closed gate를 한 번에 확인한다.

### 7 family 분모

| family | occurrence |
|---|---:|
| seeded | 11 |
| cylinder-spin | 5 |
| ground | 2 |
| decal | 3 |
| light | 1 |
| velocity | 4 |
| ef-vector-multiply | 3 |

### 불변식

1. 29개 `moduleOccurrenceId`와 `sourceRecordSha256`는 c927 receipt와 일치한다.
2. 각 exact class는 하나의 family/variant만 가진다.
3. 각 family는 closed typed input/output schema, implementation ID/version/contract SHA,
   fixed seed/time, finite absolute/relative tolerance를 가진다.
4. 각 READY_FOR_RECONSTRUCTED_REVIEW 행은 세 개의 finite numeric sample을 가진다.
5. 모든 행은 upstream evidence blocker와 `SOURCE_EXACT_NOT_CLAIMED`를 보존한다.
6. capability evidence가 준비되어도 `runtimeExecutionAdmission=false`, `productAdmission=false`다.
7. 15개 exact variant는 recursive closed input/output key·type schema를 가지며 87개 sample을 전부 검증한다.
8. blocked module의 148개 property는 reconstructed input 소비 117개와 upstream oracle irrelevant 31개로 완전 분류한다.
9. 65개 distribution binding의 누락된 source-era identity는 true 기본값 없이 explicit false로 보존한다.
10. module/property/literal/distribution/payload/sample ID는 전역 unique이고 exact occurrence owner를 벗어나지 않는다.
11. tracked JSON은 LF/no-BOM canonical text hash, external binary evidence는 raw hash domain을 사용한다.
12. implementation identity는 evaluator, random stream, acquisition validator, strict loader와 family semantic digest를 포함한다.

## 검증

- Python unit mutation suite
- generator `--check`
- focused shallow/deep ProjectAudit
- JSON duplicate key, root/version, self hash, upstream raw/self identity mutation
- 7 family property/sample/implementation mutation
- `alphascaleoverlife`, `surfaceonly`, `positive_y`, `negative_z`, `positive_z` output mutation
- recursive schema unknown key/type/presence wrapper mutation
- source-era identity/foreign owner/global duplicate ID mutation
- tracked JSON LF/CRLF parity와 BOM/semantic mutation
- execution dependency와 reconstructed default provenance mutation
- unknown class, duplicated occurrence, missing/non-finite sample, blocker 제거, Product 승격 거부
- `git diff --check`

## 전체 반영 코드

검증된 반영 파일은 다음과 같다. 새 파일은 모두 이 checkpoint에 완전한 실행 코드와 생성 JSON을
포함하며 생략/placeholder가 없다.

- `Tools/LevelPlacementExtractor/build_artist_31470_reconstructed_source_capability.py`
- `Tools/LevelPlacementExtractor/test_build_artist_31470_reconstructed_source_capability.py`
- `Tools/ProjectAudit/Test-Artist31470ReconstructedSourceCapability.ps1`
- `Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-source-capability.receipt.json`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`의 `effect.artist-31470-reconstructed-source-capability` 등록

각 파일의 전체 코드와 데이터는 위 경로가 정본이며, RESULT에는 실행한 검증과 아직 열리지 않은
Runtime/Product gate를 분리 기록한다.
