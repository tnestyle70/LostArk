# 2026-08-22 Material Family Shader Recovery 결과

대응 계획서: `2026-08-22_MATERIAL_FAMILY_SHADER_RECOVERY_IMPLEMENTATION_PLAN.md`

## 1. 구현 완료

### G00 — Family Shader Inventory

```text
Tools/EffectPipeline/build_effect_family_shader_inventory.py        신규
Tools/EffectPipeline/test_build_effect_family_shader_inventory.py   신규
Data/Effects/Contracts/effect-family-shader-inventory.v1.json       생성물
```

저작 문서의 `material.sourceMaterialPath`를 PR #164 child-parent receipt와 정확 일치시키고,
해소되지 않은 행만 `material.sourceProfile.parentMaterialPath`를 그대로 쓴다. 그 parent를
shader map, cooked pixel shader, named mapping, HLSL translation과 join해 occurrence별 program
증거를 분류한다. leaf 이름 fallback은 쓰지 않는다.

가장 중요한 교정은 parent family의 대표 DXBC를 그 family의 모든 occurrence에 전파하지
않는 것이다. `SINGLE_PERMUTATION_FAMILY`이거나 source child가 cooked representative child와
같고 source carrier까지 같을 때만 occurrence Program exact로 센다.

실행 결과다.

```text
effect.lancemaster.skill.34110.unified.effect.json   source= 83  exact= 27  pending=31  exactNamed=19
effect.lancemaster.skill.34150.unified.effect.json   source=186  exact=104  pending=55  exactNamed=95
effect.artist.skill.31470.unified.effect.json        source=  0  execution=17

target denominator                              unique families=25  occurrences=269
PROGRAM_EXACT_NAMED_MAPPING_ONLY                family coverage=10  occurrences=114
PROGRAM_EXACT_NAMED_MAPPING_MISSING             family coverage= 2  occurrences= 17
PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_ONLY  family coverage=12  occurrences= 86
SHADERMAP_FOUND_DXBC_MISSING                    family coverage= 3  occurrences= 34
PARENT_ONLY                                     family coverage= 1  occurrences= 18
UNKNOWN / PARENT_RESOLVED_PROGRAM_MISSING       family coverage= 0  occurrences=  0
runtime ABI closed                                                occurrences=  0
Product admitted                                                  occurrences=  0
```

Program exact는 131행이지만 이것도 selected pixel program의 구조적 일치만 뜻한다. VF/pass,
sampler state, runtime packet, Adapter와 Product는 0이다. family coverage는 mixed family 3개가
exact와 pending 양쪽에 잡히므로 합계가 unique family 25와 같지 않은 non-disjoint 지표다.

### G00 판정자 충족

```text
분류 불명 row 0            UNKNOWN 0 으로 충족
golden 대조군 분리 확인     31470 은 source=0 / execution=17로 계수되고,
                          sourceProfile + execution 동시 enabled는 validator가 거부한다.
증명 경계                   program / named mapping / runtime ABI / Product가 분리됨
```

## 2. G00이 계획 초안을 정정한 것

계획 초안 §1.7과 초기 G00은 `fx_m_pa_trail_01_4_tr` 29행을 `PARENT_ONLY`로 남겼다.
PR #164 receipt를 직접 소비한 최신 실측은 이 판단을 다시 교정한다.

```text
fx_m_mi_m_00.fx_mi.fx_m_pa_trail_01_4_tr   29행
  parent join     fx_m_mi_03.fx_mi.fx_m_pa_trail_01_tr 로 RESOLVED
  cooked program  있음, named mapping 있음
  정확한 상태     PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_ONLY
  blocker         target child와 family 대표로 추출한 child static permutation이 다름

bfx_m.bfx_d_pa_circ_01_ad                   18행
  resolvedBy      LEAF_NAME_AMBIGUOUS
  cookedEvidence  BASE_MATERIAL_ID_UNRESOLVED
```

따라서 trail은 parent 미해결이 아니라 **occurrence permutation 미추출** 문제다. circ 18행만
현재 `PARENT_ONLY`로 남는다.

## 3. G00이 새로 찾은 것 — circ 이중 표기

같은 leaf가 저작 corpus에 두 표기로 저장되어 있고 한쪽만 해석된다.

```text
bfx_m.bfx_d_pa_circ_01_ad                 corpus 120행  LEAF_NAME_AMBIGUOUS
                                                        BASE_MATERIAL_ID_UNRESOLVED
bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad     corpus  17행  DECLARED_PACKAGE_EXPORT
                                                        COOKED_MATERIAL_MAPS_PRESENT
```

87.6%가 해석 불가능한 짧은 표기를 쓴다. 두 표기가 같은 export임이 확인되면 창술사 F 18행을
포함해 4캐릭터 120행이 한 번에 join 대상이 된다.

실측은 evidence 부재가 아니라 **경로 표기 문제**를 가리킨다. 회수 작업이 아니라 경로
해석 판정이 먼저다.

## 3.1 경로 해소 방법 진단 — shader-map denominator에는 아직 미반영

원본 package를 직접 열어 확정했다. 추론이 아니라 export table 실측이다.

`bfx_m_mi_00.upk`(`6YGC3DB3SBJ3S11G16MH6QMH8.upk`)에서 leaf `bfx_d_pa_circ_01_ad`를 가진
material export는 둘이다.

```text
class=material  idx=155  objectPath=bfx_m.bfx_d_pa_circ_01_ad   baseGuid=3502e06c8444c34f80e2b17bf17019eb
class=material  idx=435  objectPath=bfx_mi.bfx_d_pa_circ_01_ad  baseGuid=4562d7ba4bbba6409c1b7a09e46ae174
```

**서로 다른 material이다.** GUID가 다르고 group이 다르다. 그런데 저작 corpus가 쓰는 짧은
표기 `bfx_m.bfx_d_pa_circ_01_ad`는 export 155의 `objectPath`와 **정확히 일치**한다.
group 세그먼트가 두 export를 완전히 가른다.

즉 짧은 표기는 모호하지 않다. `resolve_parents`가 3 세그먼트 미만이면 declared-package
경로를 포기하고 leaf 이름 검색으로 내려가는데, leaf 검색이 group을 버리기 때문에 같은
package 안의 두 export가 후보로 잡혀 `LEAF_NAME_AMBIGUOUS`가 됐다.

같은 방식으로 `LEAF_NAME_AMBIGUOUS` 4건 전부를 실측했다.

```text
short path                      occ   material export   objectPath 정확 일치   판정
bfx_m.bfx_d_pa_circ_01_ad       120          2                  1            RESOLVABLE
fx_m.fx_d_pa_shine_02_tr         17          2                  2            AMBIGUOUS
fx_m.fx_f_pa_wind_05_tr           5          2                  2            AMBIGUOUS
fx_m.fx_d_pa_master_01_ad         4          2                  2            AMBIGUOUS
```

circ만 group 한정으로 유일해진다. 나머지 셋은 서로 다른 **package** 두 곳에 같은
`group.object`가 있어 package 이름 없이는 못 가른다. 저작 문서에 package 이름이 없으므로
추측하지 않고 BLOCKED로 유지한다.

```text
group.object exact match로 해소 가능한 retained occurrence   120
package evidence 부족으로 유지해야 하는 occurrence            26
```

이 진단은 현재 shader-map contract를 바꾸지 않았다. 따라서 G00에서 circ 18행은 계속
`PARENT_ONLY`이며, publisher가 denominator를 재생성하기 전에는 해소 또는 admission으로
기록하지 않는다.

### 필요한 수정과 그것을 이번에 하지 않은 이유

수정 지점은 `Tools/EffectPipeline/build_effect_family_shader_map_index.py`의
`resolve_parents`다. 2 세그먼트 경로를 leaf 검색으로 내리기 전에, 후보 export의
`objectPath`와 `group.object` 전체를 비교해 정확히 하나면 그것을 채택하고, 둘 이상이면
지금처럼 `LEAF_NAME_AMBIGUOUS`로 남기면 된다.

shader-map builder는 이제 main에 추적된다. 다만 이 RESULT의 G00 PR은 denominator 변경을
같이 섞지 않는다. group.object 정책 변경은 map/cooked/translation downstream receipt를
같이 재생성하는 별도 변경 단위다.

## 4. 자동 검증

실행하고 통과한 것만 적는다.

```text
python -m unittest Tools.EffectPipeline.test_build_effect_family_shader_inventory
                                                                              43 tests OK
python -m py_compile Tools/EffectPipeline/build_effect_family_shader_inventory.py
                                                                              PASS
```

named mapping PR #162와 child-parent PR #164가 main에 들어온 뒤 G00 contract를 최신 입력으로
재생성했다. 생성 직후 실데이터 `--check`가 같은 bytes를 계산해 PASS했으며, 위 25 family /
269 occurrence 분류와 모든 upstream artifact/raw pin이 현재 contract에 고정됐다. #164 receipt는
G00의 직접 입력이며 exact source child로만 적용된다.

unit test가 고정한 분류 계약이다.

```text
selected child/static permutation + carrier 일치 -> PROGRAM_EXACT, runtime 미승격
family 대표 program만 일치 -> PROGRAM_PERMUTATION_PENDING
named mapping blocked -> exact program을 보존한 별도 status
translation missing/extra/중복 -> 전체 contract 생성 실패
cooked BLOCKED -> SHADERMAP_FOUND_DXBC_MISSING
base GUID 미해결 -> PARENT_ONLY
index 부재 -> UNKNOWN
parentMaterialPath 없음 -> UNKNOWN 행으로 보존, 조용히 버리지 않는다
같은 leaf 다른 package -> 두 행 유지
exact child receipt가 같은 known family를 가리키면 alias를 한 family로 합친다
material.execution 행 -> source와 별도 계수, 동시 enabled는 거부
입력 계약 부재 -> 출력 파일을 만들지 않고 중단
입력에 중복 parent/child 또는 child rowSha drift -> 거부
RESOLVED child와 families 집계의 known family/evidence 불일치 -> 거부
schema/self-hash/upstream raw pin/CRLF drift -> 거부
--check stale -> 기존 published bytes 보존
```

## 5. 미실행 / 미검증

```text
Engine / Client 빌드            이번 변경은 Python 과 문서뿐이라 실행하지 않았다
Publish-Effects.ps1             runtime data 를 바꾸지 않아 실행하지 않았다
Effect Tool 화면 확인           G02 이전이라 대상이 없다
Program/Layout/Descriptor/Adapter registry   후속 PR
Sprite RT0 사용자 A/B                       후속 PR, 사용자 전용
```

## 6. upstream evidence 상태

shader-map/cooked/DXBC는 PR #160, literal HLSL translation은 PR #161, named mapping은
PR #162, child-parent receipt는 PR #164로 main에 들어갔다. G00은 shader-map, cooked,
translation, named ABI, child-parent 다섯 evidence 계약의 raw/artifact identity와 dependency
pin을 검증한다. authored target은 canonical SHA-256과 element count로 고정한다. 현재 bulk
program denominator는 169이며 HLSLI도 169개지만 parent별 대표 permutation 한 개라는 경계를
occurrence exact로 과장하지 않는다.

## 7. 다음 단계

```text
1  DONE  child-parent PR #164 merge
2  DONE  최신 main에서 G00 contract 재생성 + --check
3  진행  별도 Track A에서 Program/Layout/Descriptor/Adapter registry 최소 구현
4  대기  Sprite RT0 도화가 F golden canary와 사용자 A/B
5  대기  다른 캐릭터 또는 Valtan 한 occurrence로 공용성 증명
6  대기  같은 tuple cohort -> Mesh -> Decal -> Trail -> Glass/MRT -> WPO -> Presentation
```

각 단계의 자동 PASS는 Product admission이 아니다. Composition과 사용자 서면 A/B는
occurrence마다 따로 닫는다.
