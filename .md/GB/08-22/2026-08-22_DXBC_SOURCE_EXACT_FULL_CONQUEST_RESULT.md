# 2026-08-22 DXBC 전수 해석과 SOURCE_EXACT 전환 결과 — V1

계획은 `2026-08-22_DXBC_SOURCE_EXACT_FULL_CONQUEST_IMPLEMENTATION_PLAN.md`다. 이 문서는
V1에서 실제로 실행하고 확인한 것만 적는다. V2 이후는 착수하지 않았다.

## 1. 구현한 것

```text
Tools/EffectPipeline/build_effect_child_parent_resolution.py        신규
Tools/EffectPipeline/test_build_effect_child_parent_resolution.py   신규
Data/Effects/Contracts/effect-child-parent-resolution.v1.json       신규 생성물
```

authored corpus에서 parent material 링크를 잃은 element의 child MIC를 staged source pack에서
열어 `Parent` 참조를 읽고 root Material까지 걸어 join key를 복구한다.

세 가지를 fail-closed로 막는다.

- leaf 하나가 서로 다른 오브젝트를 가리키면 고르지 않고 `LEAF_SEARCH_RESOLVES_TO_DISAGREEING_OBJECTS`
- `Parent` 체인이 root Material에 닿지 않으면 깊이 8에서 `PARENT_CHAIN_DID_NOT_REACH_ROOT_MATERIAL`
- MIC가 `Parent`를 직렬화하지 않았으면 `PARENT_OBJECT_PROPERTY_ABSENT`

같은 material이 `package.group.leaf`와 `group.leaf` 두 철자로 분모에 두 번 잡히던 문제는
`canonical_object_path`가 실제 선언 package로 재수식해 합친다.

## 2. 실측 결과

```text
authored element            7,572
parent 보존                 2,746
parent 유실                 4,826   child path 있음 4,793 / 없음 33
orphan child 경로             809
  RESOLVED                    744   DECLARED_PACKAGE_EXPORT 479 / LEAF_NAME_SEARCH 265
  BLOCKED                      65   child leaf ambiguous 19 / disagreeing 34 / 모든 package에 없음 12
복구된 element              4,391
복구된 canonical parent       223   기존 분모 145 / 신규 78
그중 extracted DXBC 보유    3,646 element
전 package leaf sweep          2 회
```

`recoveredElementsWithExtractedDxbc`는 program blob 존재만 뜻한다. named mapping, runtime ABI
packet, carrier Adapter, Composition 또는 Product admission 수치가 아니다.

## 3. 이 결과가 뒤집은 이전 결론

이전 translation RESULT는 parent 없는 행을 사람이 만든 근사로 분류하고 Valtan을 family
복원 밖으로 뒀다. **두 결론 모두 틀렸다.** 현재 parent 유실 4,826행 중 4,793행이 child MIC
경로를 보존하고, 그중 4,391행의 parent를 staged pack에서 복구했다. Valtan을 family 경로로
복원할 수 없다는 판단은 폐기한다.

원인은 증거 부재가 아니라 intake가 join key를 버린 것이다.

## 4. 스킬별 수치를 읽는 방법

같은 skillId에 baseline/candidate/clip/unified 문서가 함께 존재하므로 폴더 파일을 합산하면
중복된다. 스킬별 현재 수치는 `EffectCatalog.json`이 admission한 문서만 고른 뒤 child-parent
receipt와 join해 다시 계산한다. 도화가 Z도 먼저 source occurrence와 Composition 완전성을
검사하고, 과거 문서의 element 수를 현재 Product 분모로 사용하지 않는다.

## 5. 자동 검증

실행하고 통과한 것만 적는다.

```text
python -m unittest Tools.EffectPipeline.test_build_effect_child_parent_resolution   43 tests OK
python Tools/EffectPipeline/build_effect_child_parent_resolution.py                  744/809 RESOLVED
python Tools/EffectPipeline/build_effect_child_parent_resolution.py --check          PASS
git diff --check                                                                     clean
```

`--check`는 809개 child를 처음부터 다시 걸어 `artifactSha256`이 checked-in 파일과 같은지
본다. 전 package leaf sweep 2회를 포함해 같은 pack에서 같은 결과가 나오는 것을 확인했다.

Engine/Client 빌드는 실행하지 않았다. V1은 C++과 `Client/Bin/ShaderFiles`를 한 줄도 바꾸지
않았다.

## 6. 이 결과가 admission하지 않는 것

- **runtime 없음.** 복구된 join key는 저작 contract이며 렌더러가 읽지 않는다.
- **authored 문서 무변경.** 4,391 element에 parent를 써 넣는 마이그레이션은 하지 않았다.
- cooked program, vertex factory, sampler state, render state, MRT는 이 V1의 범위 밖이다.
- **육안 검증 없음.** 사용자의 서면 관찰 전에는 어떤 visual PASS도 기록하지 않는다.

## 7. 남은 경계와 다음 단위

현재 bulk family program denominator인 169 DXBC와 169 HLSLI는 main에 추적됐다. 다만
child-parent가 찾은 신규 parent 78개는 shader-map/cooked denominator에 아직 합쳐지지 않았다.
join-key receipt가 program, runtime ABI 또는 Product admission을 자동으로 열지 않는다.

V2 대상은 다음과 같다.

```text
A. 현재 retained parent denominator             205 family
B. child-parent가 찾은 신규 parent 후보          78 family
C. 복구됐지만 현재 extracted DXBC가 없는 행      746 element
D. 현재 cooked extraction blocker                13 family
```

---

# 2026-08-23 Material Program 정본화 기반 결과

## 1. 이번 변경에서 실제로 닫은 범위

전체 occurrence를 다시 갈아엎지 않고 확장할 수 있도록 JSON 정본과 파생 CSV로 구성한
Material Program conquest ledger를 추가했다. ledger는 tuple inventory와 public registry를
같은 프로세스에서 다시 생성·검증하고, 모든 입력 hash와 projection hash를 봉인한다.

```text
전체 authored occurrence                         7,566
Product occurrence                               2,554
  4캐릭터 Product                                1,885
  Valtan Product                                   669
program row                                      1,325
source program candidate                           171
typed runtime program                               18
literal DXBC program                               169
Product distinct typed program                      17
Product translated exact literal program           111
Product untranslated exact program                   1 / 4 occurrence
public Program allocation                            3
```

공개 S6/M3/D14 tuple은 캐릭터 이름을 ID에서 제거했다. Program ID는
`backend + opcode`, Layout ID는 `backend + opcode + canonical ABI hash`로 고정했으며,
generator와 Client registry가 별칭 Program ID와 가짜 ABI suffix Layout ID를 거부한다.

```text
S6  effect.program.runtime-material-v2.opcode-6.v1
M3  effect.program.runtime-material-v2.opcode-3.v1
D14 effect.program.local-decal.opcode-14.v1
```

ledger artifact SHA-256는
`f96e2fbac871bec8b39b199ca405d0c98850aa873f56d246312cd94421c9f85c`, public registry
SHA-256는 `2e96d1acc5faf5c3062f578e3813611bbc05352fe09202055666c82995db461f`다.

## 2. 실행한 검증

```text
python -m unittest Tools.EffectPipeline.test_build_effect_material_program_registry
  16 tests OK
python -m unittest Tools.EffectPipeline.test_build_effect_material_program_conquest_ledger
  17 tests OK
python Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py --check
  PASS
python Tools/EffectPipeline/build_effect_material_program_conquest_ledger.py --check
  PASS
Publish-Effects.ps1 -Mode Validate
  145 catalog entries / 3 material bindings PASS
Engine / Shared / Server / Client / EffectRenderContractHarness x64 Debug
  build PASS
Server.exe --contract-test
  failures 0
EffectRenderContractHarness Debug -ExpectedBindingCount 3
  S6 / M3 / D14 draw와 rollback PASS
```

Release 전체 빌드는 사용자의 검증 종료 요청에 따라 진행 중 중단했다. Client는 실행하지 않았고
화면 캡처, `manual first pixel`, 육안 PASS는 수행하거나 기록하지 않았다.

## 3. 의도적으로 구현하지 않은 경계

169개 material 계산식의 runtime backend, 확장 ABI packet, Adapter, Descriptor, occurrence Binding은
이 변경에서 만들지 않았다. raw sealed DXBC만 SOURCE_EXACT이며, 번역 HLSLI를 재컴파일한 결과는
동등성 검증 전까지 bounded reconstruction이다. 따라서 ledger의 `READY`는 equation evidence의
준비 상태이지 Product draw admission이 아니다.

다른 세션이 4캐릭터 기준 material 계산식과 ABI를 한 수직 슬라이스로 닫고 사용자가 실제 화면을
판정할 때까지 이 브랜치는 위 정본화 기반에서 멈춘다.
