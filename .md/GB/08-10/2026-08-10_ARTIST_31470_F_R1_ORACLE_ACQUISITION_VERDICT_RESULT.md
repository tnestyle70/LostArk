# Artist F 31470 F R1 Oracle Acquisition Verdict 결과

날짜: 2026-08-10

branch: `codex/artist-f-r1-acquisition-verdict-v1`

기준 계획: `7ffb8a3bf123703ea451cbe53a178f449f102fbe`

## 결론

R1의 provider acquisition과 evidence audit는 접근 가능한 범위에서 끝났다. Source와 Material 모두
행·owner·blocker·검증 경계를 손실 없이 동결했고 evidence integrity는 독립 review PASS다. 그러나
source-era actual-output/state provider는 확보하지 못했다.

```text
Source execution readiness     0/29 BLOCK
Material execution readiness   0/255 BLOCK
Product admission              false / 0/35
R2-R8                           NO-GO
```

따라서 이 결과는 복원 완료가 아니다. final materializer, Playback, renderer, Tool/Catalog Product
transaction, Client runtime smoke는 시작하지 않았다. 화면에 보이는 legacy effect나 current default를
Artist F 완전 복원으로 판정하지 않는다.

## Frozen 입력과 독립 판정

| 영역 | exact commit | evidence-integrity 판정 | execution 판정 |
|---|---|---|---|
| Source corrective | `c927e397811d4e5718efd27b187eb59775023685` | current-only 승격 철회 PASS | provider acquisition 전 BLOCK |
| Source acquisition | `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3` | accessible-scope PASS | `0/29` BLOCK |
| Material evidence corrective | `cde8f3bddea2f9415f682b387d2705fd25794075` | exact-tree PASS, P0/P1/P2 `0/0/0` | `0/255` BLOCK |

Material lineage의 `627ddc76`은 source-specific full sampler descriptor가 없는 4건을 exact로 승인해
supersede됐다. `ab76b7ec`과 `d39097c3`도 각각 row/evaluator와 acquisition/runtime top-level schema의
coordinated reseal gap 때문에 최종 정본이 아니다. 최종 Material 정본은 `cde8f3bd` 하나다.

## Source 29 최종 판정

Source matrix는 기존 blocker 29개를 동일 ID로 보존하고 15 exact class, 7 native family로 축약했다.
모든 행에 final owner가 있으며 ownerless row는 0이다.

```text
source-era provider                 0
actual mutated-output pilot         0
numeric output oracle               0
tolerance-bearing READY row         0
resolved blocker delta              29 -> 29
execution readiness                 0/29 BLOCK
```

current EFEngine wrapper/dataflow와 fixed input digest는
`CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`로만 보존한다. input digest parity는 output oracle이
아니며 source-era `SOURCE_EXACT` 또는 READY로 승격하지 않았다.

Source frozen 검증은 acquisition unit `21/21`, deterministic rebuild `--check`, focused shallow/deep,
receipt/hash/JSON/diff 검사를 통과했다. lane 외 기존 14개 ProjectAudit 실패를 전체 green으로 기록하지
않았다.

## Material 255 최종 판정

| matrix | source value 확보 | execution READY | 남은 execution blocker |
|---|---:|---:|---:|
| render-state | 0/89 | 0/89 | 89 |
| static selection | 23/94 | 0/94 | 94 |
| strict sampler descriptor | 0/72 | 0/72 | 72 |
| 합계 | 23/255 | 0/255 | 255 |

static 23행은 exact source MIC의 `bOverride=true` value를 확보했다. 이것은 source value provenance이며
actual state-output pilot과 final consumer가 없으므로 execution READY가 아니다. 나머지 static은
`bOverride=false` semantics 미확정 43행과 exact GUID join 없음 28행으로 분리했다.

이전 exact sampler 3 instance+1 parent는 DDS/texture binding identity만 exact했다. source-era
Texture2D CDO, TextureGroup filter, sRGB/default chain이 없어 full descriptor exact claim을 철회했다.
최종 분모는 instance 71+parent 1의 strict sampler 72이며 exact full descriptor는 0/72다.

Material `cde8f3bd` 검증 결과:

```text
source-value acquisition tests      12/12 PASS
runtime receipt tests               23/23 PASS
raw source rebuild                  PASS
deep archive audit                  1,813/624 PASS
WARP numeric/state replay           200 samples / 4 pilots PASS
focused/scoped-deep ProjectAudit    PASS
execution readiness                 0/255 BLOCK
Product                             false
```

WARP 결과는 evaluator와 state oracle plumbing의 재현성을 증명할 뿐 source-era 값을 제공하지 않는다.
최종 semantic projection은 acquisition의 search/VSS/capture/source identity/row owner/summary/admission과
runtime의 root/sourceEvidence/summary/admission/family/occurrence schema를 고정한다. 독립 exact-tree
review는 관련 negative fixture를 모두 거부하고 P0/P1/P2 0으로 판정했다.

## 조사 범위와 남은 외부 경계

접근 가능한 local filesystem, backup 후보, current install, Git remote/LFS/reachable·unreachable object,
source archive와 읽을 수 있는 driver cache 범위에서는 qualifying source-era provider를 찾지 못했다.
이 결과는 전역 소진을 주장하지 않는다.

```text
VSS inventory                       PERMISSION_UNCHECKED
NVIDIA DXCache                      548/561 readable
NVIDIA share-locked files           13
globalExhaustionClaim               false
safe source-era standalone capture  unavailable
```

현재 설치본의 capture가 가능해져도 `CURRENT_REVISION_OBSERVED`일 뿐 source-era fidelity를 닫지 못한다.
지원된 source-era UCC/commandlet/debug API와 paired runtime bundle이 없으므로 독립 controlled capture도
provider로 승인하지 않았다. process injection, hooking, anti-cheat 우회는 시도하거나 허용하지 않았다.

## Runtime 검증 상태

Artist F numeric Product candidate가 없으므로 Server+Client 실제 F 입력 smoke와 눈 검증은 실행하지 않았다.
이것은 미실행 항목이며 PASS도 실패도 아니다. R6 smoke는 Source 29/29, Material 255/255, corrected
materializer, immutable IR consumer, geometry/material renderer, Tool/Catalog transaction과 Product 35/35가
모두 닫힌 뒤에만 실행한다.

## R1 재개 계약

`SOURCE_EXACT` 경로를 다시 열려면 같은 revision의 다음 묶음이 필요하다.

- `EFEngine.dll`, `LOSTARK.exe`, `Engine.u`, `Core.u`, `EFGame.u`
- target UPK와 source-revision ShaderCache/material map
- SystemSettings TextureGroup configuration
- 위 파일을 하나의 build로 인증하는 identity manifest

대안은 같은 build identity와 fixed seed/time/world/parameter input, pre/post full numeric state,
expected output과 tolerance를 가진 authenticated source-era capture다.

최대 reconstruction admission 분기는 사용자의 별도 명시 승인 전에는 시작하지 않는다. 기존 23 arithmetic
evaluator와 WARP replay는 비제품 oracle plumbing이며 source value나 execution row를 승격하지 않는다.
승인되더라도 행별 독립 same-input output oracle을 통과한 값만 `RECONSTRUCTED_NUMERICALLY_VERIFIED`로
표시하고 evidence blocker와 Product false를 보존한다. current default, class 이름, 이미지 비교, 공통
fallback은 승인 근거가 아니다.

R2 진입 predicate는 Source `29/29`, Material render `89/89`·static `94/94`·sampler `72/72`, unresolved
execution row 0이다. 현재 이 predicate는 false이므로 R2~R8은 `NO-GO`다. Corrected materializer는 이
입력을 받은 뒤 R2에서 새로 작성하고 R2 종료 조건으로 독립 PASS해야 한다.

## Combined verdict 검증

PLAN/RESULT UTF-8 검증과 `git diff --check`, Source/Material exact commit 및 remote branch identity 확인은
PASS했다. 전체 `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`도 실행했으나 exit 1이며 13개 기존 통합
category가 남았다. map runtime root, Character Select manifest, Data project visibility, G09 경계,
미빌드 Source/Geometry harness, WFX/representative rollout, actor resource 등이 포함된다. 이 R1 문서
변경은 해당 실패를 전체 green으로 기록하지 않으며 R2 진입 근거로 사용하지 않는다.
