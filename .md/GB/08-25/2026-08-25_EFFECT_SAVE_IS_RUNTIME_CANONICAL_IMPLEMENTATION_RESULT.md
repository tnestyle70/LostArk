# 2026-08-25 Effect Save 단일 정본 런타임 전환 구현 결과

## 1. 결론

제품 Effect 입력을 `Data/Effects/EffectCatalog.json`과
`Data/Effects/Authored/<EffectAssetId>.effect.json` 한 벌로 통합했다. Effect Tool의 Save 성공 뒤 같은
Client의 다음 재생과 다음 Client 실행은 별도 publish 없이 같은 authored 문서를 읽는다. 기존
`Client/Bin/DataFiles/Effect` hash seal, runtime catalog와 `Data/Effects/VisualPrograms` sidecar는 제품
경로에서 제거했다.

| 구분 | 판정 |
|---|---|
| source-only catalog/runtime/save transaction | 완료 |
| V1 direct Product authored 문서 | 175개 |
| catalog 밖 저작 참고 문서 | 265개, 제품 입력 아님 |
| pull 재현용 Resources closure | 1,025개 / 73,709,096 bytes |
| generated Effect runtime artifact | 0개 |
| Effect source validator unit | PASS, 24/24 |
| Valtan saved-row unit 포함 focused unit | PASS, 53/53 |
| Effect data project registration | PASS, files 2,349 / filters 219 |
| EffectRenderContractHarness Debug/Release | PASS |
| Client x64 Debug/Release compile/link | PASS, 오류 0 |
| 사용자 visual fidelity | PASS — 사용자 서면 판정 `검증 완료` |

## 2. 회귀 원인과 닫은 계약

기존에는 Effect Tool Save와 제품 시작 입력이 갈라져 있었다.

```text
Editor Save
  -> Data/Effects/Authored/<ID>.effect.json

제품 Client 시작
  -> Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json
  -> Client/Bin/DataFiles/Effect/Authored/<ID>.<sha256>.effect.json
```

현재 계약은 다음과 같다.

```text
EffectCatalog.json의 DIRECT_AUTHORED_DOCUMENT row
  -> Data/Effects/Authored/<EffectAssetId>.effect.json parse/validate
  -> transient runtime projection과 GPU target stage
  -> 전체 성공 뒤 catalog/presentation commit

Effect Tool Save
  -> baseline compare
  -> authored 임시 파일 기록과 atomic replace
  -> 같은 Product stage/commit 경로 재사용
  -> 다음 occurrence부터 새 document 사용
```

진행 중 occurrence는 생성 당시 immutable document를 끝까지 사용한다. Save 뒤 Product 준비가 실패하면
disk bytes와 catalog/presentation 상태를 이전 revision으로 복원한다. 파일이 외부 writer에 의해 다시
바뀐 경우 compare-and-swap 조건이 맞지 않으므로 외부 변경을 덮어쓰지 않고 충돌을 보고한다.

## 3. V1 데이터와 pull 재현 계약

Catalog의 175개 direct Product가 실제로 참조하는 Resources-relative `assetId`, `modelAssetId`,
`textureAssetId` closure만 Git에 포함했다.

| 형식 | 파일 수 |
|---|---:|
| DDS, Git LFS | 879 |
| WModel, 일반 Git | 146 |
| 합계 | 1,025 |

전체 `Client/Bin/Resources` 팩, 미참조 파일, 추출 원본은 포함하지 않는다. 별도 resource manifest나
sidecar도 runtime 정본으로 추가하지 않았다. clone/pull 뒤 `git lfs pull`을 실행하면 이 V1 Effect Product가
필요로 하는 선택된 closure를 받는다.

`Validate-EffectSources.ps1`는 direct document와 optional screen overlay를 재귀 검사해 안전한
Resources-relative `.dds`/`.wmodel`만 허용한다. DDS/WModel magic, hydrated working bytes, DDS의 LFS
attribute와 staged pointer SHA-256/size, WModel의 staged blob identity까지 검사한다. 다음 Effect 변경에서 새
asset ID만 저장하고 팀원이 받을 리소스를 빠뜨리거나 다른 bytes를 stage하면 검증이 실패한다.

## 4. 제거한 두 번째 제품 경로

- `Client/Bin/DataFiles/Effect/**` generated runtime catalog와 hashed authored copy
- `Data/Effects/VisualPrograms/effect-visual-program-*.json` disk sidecar
- `Publish-Effects.ps1`, selected direct publisher와 Effect publish harness
- runtime catalog/hash seal/derived artifact만 만들던 builder와 rollout 도구

legacy payload 중 회귀 근거로 필요한 byte identity는
`Data/Effects/Imported/LegacyRuntimeDonors`의 read-only source evidence로 보존했다. Gameplay balance,
Map, Navigation, Item과 Valtan gameplay publisher는 Server 형식 변환을 소유하므로 제거 대상이 아니다.

## 5. 자동 검증

```text
Validate-EffectSources.ps1
  directSourceCount                    175
  unboundReferenceCount                265
  sourceBytes                           73,209,058
  resourceFileCount                   1,025
  resourceBytes                        73,709,096
  generatedArtifactCount              0

Python focused unit                    PASS, 53/53
Effect project sync -Check             PASS, files=2349 filters=219
Effect project registration harness    PASS, files=2349

EffectRenderContractHarness Debug      PASS, actual draw failed=0
EffectRenderContractHarness Release    PASS, actual draw failed=0
Client x64 Debug compile/link           PASS, errors=0
Client x64 Release compile/link         PASS, errors=0
```

Debug/Release 하네스는 source catalog, Artist canary와 decal/linear reveal, LanceMaster portable source,
Valtan typed carrier/history, screen overlay/world mark와 WARP actual draw를 검사했다. 빌드 경고는 기존 SDK
인코딩 및 PDB 경고이며 새 compile/link 오류는 없었다.

## 6. 사용자 수동 검증

사용자는 물리 정본 Debug Client에서 직접 인게임 검증을 수행한 뒤 2026-08-25에 `검증 완료`라고
서면 판정했다. 에이전트는 Client/UI를 자율 실행하거나 visual PASS를 대신 판정하지 않았다.

## 7. 분리 범위와 잔여 경계

- Valtan gameplay bootstrap v19 reader forward-fix는 별도 PR의 Server 계약이며 이 Effect 변경에 섞지 않는다.
- Server/Shared/GameRoom/Profiler/Sound/Balance Tool/DimensionMaster BA 별도 작업을 포함하지 않는다.
- Save transaction은 단일 Effect Tool writer와 외부 변경 감지를 지원한다. 서로 다른 process가 동일 파일을
  같은 순간에 쓰는 범용 OS file-lock protocol은 이번 계약에 추가하지 않았다.
