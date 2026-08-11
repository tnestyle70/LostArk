# 2026-08-11 R8 Offline Raw Resource Preparation Result

## 현재 완료 상태

첫 검증 단위인 `DENOMINATOR_CHECKPOINT`를 clean baseline
`7d3e957f4d93bfd1416fa6a05d5d7fa8f46c12a2`에서 동결했다. 이 결과는
`lostark.raw-resource-inventory-v1`의 offline 입력 증거이며 runtime authority가 아니다.
raw export, DDS/WModel inspection, final raw inventory는 다음 검증 단위에 남아 있다.

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

## 다음 검증 단위

checkpoint raw SHA를 run identity에 포함한 previously-nonexistent staging을 corpus별로 만들고,
현재 pin한 physical UPK에서만 exact object를 추출한다. DDS는 원본 bytes를 보존하고 TGA는
재인코딩하지 않은 format blocker로 남긴다. mesh는 source glTF/bin과 converter binary/options를
함께 pin한 `OFFLINE_TRANSIENT_COOK` scale-1 WModel만 별도 staging에 만든다.

그 뒤 DDS header/format/mip/payload와 WModel header/submesh/vertex/index/bounds를 parser-neutral로
검사하고, byte SHA dedup과 missing/case/name collision report를 생성한다. 최종 파일도 계속
`OFFLINE_INVENTORY_ONLY`, 모든 runtime admission false이며 GPU schema가 동결되기 전 final
candidate/materializer binding은 생성하지 않는다.
