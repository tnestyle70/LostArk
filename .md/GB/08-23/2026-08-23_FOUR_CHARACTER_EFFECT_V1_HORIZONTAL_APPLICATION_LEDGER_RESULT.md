# 2026-08-23 4캐릭터 Effect V1 수평 application ledger 결과

branch: `codex/four-character-effect-v1-horizontal`

base: `origin/main@d6b27084e5853fb8919685c8f3853c5fc6cadbe7`

최종 화면 판정자: 사용자

상위 계획: [`4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획`](../08-22/2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md)

## 1. 완료 범위

Artist, DimensionMaster, LanceMaster, Warlord의 current Product `99 assets / 1,885 occurrences`를
current HEAD 입력에서 다시 생성하고, 모든 행에 source identity, 원본과 현재 render state, Program,
Layout, Descriptor, Adapter, Binding, blocker와 manual review 상태를 부여했다.

이 ledger는 application 판정과 다음 publisher의 입력이다. Authored Effect, material-program registry,
Client runtime과 화면은 수정하지 않는다. Track B의 checked-in tuple inventory는 runtime proof로 사용하지
않고 builder를 메모리에서 재실행한다.

## 2. 수평 분모

| fine renderer | occurrence | S/M/D application scope |
|---|---:|---|
| SpriteParticle | 1,337 | 포함 |
| MeshParticle | 493 | 포함 |
| DecalParticle | 41 | 포함 |
| standalone Sprite Rect | 2 | feature deferred |
| Ribbon | 12 | feature deferred |
| **합계** | **1,885** | **S/M/D 1,871** |

## 3. 배타적 application 상태

| state | occurrence | 의미 |
|---|---:|---|
| `CURRENT_BOUND_INLINE_EXACT` | 1 | Track A Artist F Sprite Binding |
| `INLINE_MIRROR_CANDIDATE` | 43 | enabled typed inline packet, 공용 registry batch 대기 |
| `SOURCE_EXACT_SIMPLE_RT0_PACKET_PENDING` | 64 | occurrence-exact Program과 기본 RT0 근거, packet/actual tuple 대기 |
| `SOURCE_EXACT_PACKET_PENDING` | 596 | exact Program은 있으나 scene/MRT/WPO/time/state/packing 후속 필요 |
| `PROJECT_RECONSTRUCTION_PENDING` | 711 | admitted reconstruction을 typed Program/packet으로 구현해야 함 |
| `FEATURE_DEFERRED` | 14 | standalone Sprite 2 + Ribbon 12 |
| `EVIDENCE_BLOCKED` | 456 | source identity/state/Program/Descriptor 등 필수 근거 미폐쇄 |
| **합계** | **1,885** | |

현재 registry Binding identity를 가진 행은 정확히 1개다. 나머지 1,884행은 `bindingIdentity:null`이고
`runtimeProofStatus:NOT_PROVEN`이다. 정적 cohort나 candidate ID를 actual runtime proof로 승격하지 않는다.

## 4. 원본 state와 V0 평탄화

committed `FourClassCombat.source-material-contract.json`과 exact source material path로 join되는 행은
1,512개고 미가입은 373개다. 원본 render state는 1,204행이 resolve되고 681행은 unresolved다.

### 4.1 원본 SpriteParticle

| source state | occurrence |
|---|---:|
| additive, one-sided, depth-test off | 20 |
| additive, one-sided, depth-read | 378 |
| additive, two-sided, depth-read | 24 |
| masked, one-sided, depth-read | 1 |
| translucent, one-sided, depth-read | 411 |
| translucent, two-sided, depth-read | 48 |
| unresolved | 455 |

현재 V0 SpriteParticle은 additive one-sided 500, additive two-sided 272, alpha one-sided 420,
alpha two-sided 145로 저작돼 있다.

### 4.2 원본 MeshParticle

| source state | occurrence |
|---|---:|
| additive, one-sided, depth-read | 75 |
| additive, two-sided, depth-read | 12 |
| masked, one-sided, depth-read | 40 |
| masked, two-sided, depth-read | 12 |
| translucent, one-sided, depth-read | 71 |
| translucent, two-sided, depth-read | 112 |
| unresolved | 171 |

현재 V0 MeshParticle은 additive one-sided 76, additive two-sided 26, alpha one-sided 81,
alpha two-sided 309, opaque/back/depth-write 1로 저작돼 있다. 특히 원본 masked와 one-sided Mesh가
V0 alpha two-sided로 평탄화된 행은 별도 Adapter/state tuple로 복구해야 한다.

### 4.3 DecalParticle

Decal 41행의 committed source render state는 전부 unresolved다. 현재 V0 profile은 alpha one-sided 2,
alpha two-sided 39다. 이를 원본 state라고 추정하지 않고 LocalDecal Program/projector/depth evidence가
추가될 때까지 fail-closed한다.

비교 가능한 축의 전체 결과는 match 1,125, mismatch 79, unresolved 681이다. mismatch 축은 중복 집계로
blend 54, two-sided 34, depth-test 20이다. source contract가 depth-write를 캡처하지 않았으므로 모든 source
depth-write는 `null / NOT_CAPTURED`이고 masked 53행도 depth-write를 발명하지 않는다.

## 5. 대표 4스킬 acceptance composition

| 대표 스킬 | Product occurrence | 현재 application 분류 |
|---|---:|---|
| DimensionMaster R `2050180` | 10 | simple exact 3 + reconstruction 7 |
| Artist A `31460` | 18 | reconstruction 14 + blocked 4 |
| LanceMaster D `34110` | 88 | exact packet 27 + reconstruction 31 + blocked 30 |
| Warlord R `17110` clip2/3 | 15 | exact packet 7 + reconstruction 6 + blocked 2 |
| **합계** | **131** | |

다섯 Product 문서 131행에는 enabled inline Material.Execution이 0개이고 registry Binding도 0개다.
따라서 공용 spine 병합 전 authored JSON에 임의 packet을 추가하지 않는다. 첫 packet materialization
우선순위는 DimensionMaster R exact Sprite 3행, 그다음 scene/MRT/time capability를 요구하는 LanceMaster D
exact Sprite 5행이다. 스킬 이름은 renderer selection key가 아니라 전체 batch 뒤 사용자 composition
검증 표본이다.

`four-character-representative-packet-plan.v1.json`은 다섯 문서의 raw SHA와 131행을 byte-stable하게
봉인한다. DimensionMaster R 3 + LanceMaster D lensflare 3 + noise 2, 합계 8행만
`PACKET_MATERIALIZATION_PENDING`이고 나머지 123행은 명시적 `BLOCKED`다. 모든 행의
`proposedEnabledExecution`은 null이고 authored/registry/C++ writer와 runtime admission은 false다.
이 plan도 checked-in Track B inventory를 읽지 않고 current HEAD inventory를 메모리에서 다시 생성하며,
현재 in-memory artifact SHA `e53323119cf5fe9cede0ee40bf1ac71384f8447647a92063f1e5edf6de21b61c`를
입력 identity로 고정한다.

### 5.1 공용 S6/M3/D14 merge 직후 첫 fragment

현재 enabled typed inline 44행은 Sprite 20, Mesh 22, Decal 2다. 공용 PR의 실제 compiled 교집합인
Sprite runtimeMaterialV2/6 alpha-two, Mesh runtimeMaterialV2/3 alpha-two, LocalDecal/14 alpha-one과 exact
match하는 행은 Artist F의 5행뿐이다.

- 공용 fragment 소유: `sprite.2b3dc6842507e910`, `mesh.062366ee9f9655d3`,
  `decal.f3b5c3b63b4a7e34`
- 4캐릭터 첫 fragment 신규 Binding: `sprite.c65181324417a1a8`,
  `decal.6f78bff02c657a14`

두 신규 행은 공용 Program/Layout/Descriptor/Adapter를 그대로 재사용하며 in-memory 공용 validator에서
carrier와 모든 packet field/float32 bit-exact 비교를 통과했다. 공용 merge 직후 ledger는 bound 3/candidate
41, 첫 two-Binding fragment 뒤 bound 5/candidate 39가 되어야 한다.

나머지 inline 39행은 state보다 먼저 opcode capability에서 deferred된다. `artistVisualV4` 7행과
RuntimeMaterialV2 opcode 1/8/11/13/17/18/19의 32행이다. 이 중 8행은 현재 세 Adapter state와 맞지만
Program/Layout ABI가 없고, 30행은 추가 state Adapter도 필요하며, opaque Mesh 1행은 Adapter ID부터 없다.

대표 4스킬 131행의 Program candidate와 공용 S6/M3/D14 세 Program의 교집합은 0이다. 따라서 공용 merge와
위 two-Binding fragment만으로 대표 스킬 화면은 바뀌지 않는다. 대표 화면을 열 첫 후속 capability는
DimensionMaster R `fx_mm_basic_01_ad` exact Sprite Program/Layout/Descriptor와 additive one-sided actual
Adapter다.

## 6. 병렬 세션 경계

- 공용 세션은 S6/M3/D14 registry fragment merge, compiled Adapter, exact dual-resolve, Binding 0/1 actual
  draw와 Debug/Release harness만 소유한다.
- 이 branch는 4캐릭터 ledger, four-character fragment와 evidence-closed authored packet만 소유한다.
- Valtan branch는 669 ledger, Valtan fragment와 authored packet만 소유한다.
- 최종 integration owner가 두 domain fragment 병합 뒤 단일 runtime registry/catalog를 publish한다.

공용 PR이 병합되면 이 ledger를 최신 main에서 다시 생성하고, stale input SHA가 있으면 `--check`가
실패해야 한다. 그 뒤 S6/M3/D14와 exact match하는 inline 행을 첫 fragment batch로 생성한다.
ledger는 base registry file SHA와 별도로 `build_registry()`가 반환한 fully validated registry의 canonical
SHA를 기록하므로 base가 그대로이고 fragment만 바뀌어도 stale이 된다.

## 7. 산출물

- `Data/Effects/Contracts/four-character-effect-v1-horizontal-application.v1.json`
- `Tools/EffectPipeline/build_four_character_effect_v1_horizontal_application.py`
- `Tools/EffectPipeline/Schemas/lostark.four-character-effect-v1-horizontal-application.schema.json`
- `Tools/EffectPipeline/test_build_four_character_effect_v1_horizontal_application.py`
- `Data/Effects/Contracts/four-character-representative-packet-plan.v1.json`
- `Tools/EffectPipeline/materialize_four_character_representative_exact_packets.py`
- `Tools/EffectPipeline/Schemas/lostark.four-character-representative-packet-plan.schema.json`
- `Tools/EffectPipeline/test_materialize_four_character_representative_exact_packets.py`

artifact SHA-256:

```text
756d6492a5edba24851347017126b8b9ded0245580290ea4476e4175d3cf49fa
```

대표 packet plan canonical artifact SHA-256:

```text
498e5c64e0a02a0ec1288c6fc35c99d646e7f67f9738dea1f1deeedc43916f91
```

## 8. 자동 검증

실행 결과:

```text
py -3 -m py_compile \
  Tools/EffectPipeline/build_four_character_effect_v1_horizontal_application.py \
  Tools/EffectPipeline/test_build_four_character_effect_v1_horizontal_application.py

python Tools/EffectPipeline/test_build_four_character_effect_v1_horizontal_application.py
........... 11 tests OK

py -3 Tools/EffectPipeline/build_four_character_effect_v1_horizontal_application.py --check
PASS

py -3 Tools/EffectPipeline/test_materialize_four_character_representative_exact_packets.py
............. 13 tests OK

py -3 Tools/EffectPipeline/materialize_four_character_representative_exact_packets.py --check
PASS: 131 occurrences, 8 pending, 0 enabled, 0 admitted

powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
PASS: 145 Effect catalog entries / 1 material-program Binding

git diff --check
PASS
```

두 generated Data contract는 `Client.vcxproj`와 `.filters`의 `96.DataFiles\Effects\Contracts`에 최소 등록했다.
전체 `Sync-EffectDataProject.ps1 -Check`는 이 변경 이전부터 main의 다른 evidence/DXBC 등록 누락 때문에
stale이며, 해당 수백 파일을 이 4캐릭터 PR에 섞지 않는다.

이번 결과에는 Client/UI 실행, 화면 캡처, visual PASS, authored Product mutation과 새로운 runtime Binding이
없다.
