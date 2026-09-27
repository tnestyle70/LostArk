# 2026-09-26 마하라카 G04 2차 — 재질·RNM 공통 병목의 뿌리

G03/G04의 RNM과 G06의 재질 15종이 같은 뿌리에 막혀 있었다. 그 뿌리를 특정했고, 가장 큰 항목은
해소 방법까지 데이터로 확정했다. **제품 파일은 하나도 바꾸지 않았다.**

## 0. 한 줄 결론

`sourceMaterialBuild: null`은 compiler가 없어서가 아니다. **compiler는 RNM을 이미 완전히
지원하고, 그 앞단 manifest builder에 RNM 경로가 아예 없다.** 그리고 마하라카에서 "원본에
baked lighting이 없다"고 분류될 뻔한 component 3,348개는 전부 **RNM 데이터를 온전히 갖고 있다.**
추출기가 ShadowMap2D 참조가 있으면 곧바로 거부하기 때문에 못 읽은 것이다.

## 1. 실제 바꾼 파일

없음. 조사·디코드·후보만 만들었다.

| 경로 | 성격 |
|---|---|
| `out/MaharakaContinuation_20260926_193455/G04_round2/decode_unsupported_tails.py` | 새 조사 스크립트 (gitignore 대상) |
| `out/MaharakaContinuation_20260926_193455/G04_round2/unsupported-tail-decode.json` | 디코드 결과 |
| 이 RESULT 문서 | 신규 |

`Data/**`, `Client/Bin/DataFiles/**`, `Client/Bin/Resources/**`, `Tools/**`, C++ 전부 무변경.
`build_source_map_material_inputs.py`(타 세션 미추적 파일)도 읽기만 했다.

## 2. 원본 근거: 베른이 쓴 실제 경로

### 2.1 베른의 RNM은 이 compiler가 만든 것이 아니다

`Data/Maps/Imported/LV_BER_BERNCASTLE/`에 `build.receipt.json`이 **없다.** 존재하는 Area는
`LV_LUT_MIDNIGHTC_ED`와 `LV_OCN_EVENTIS_MHP` 둘뿐이고 **둘 다 `sourceMaterialBuild: null`**이다.

베른의 shard receipt `LV_BER_BERNCASTLE.shards.receipt.json`은 `shardCount: 13`이고 나열된
shard는 `BASE, LANDSCAPE, SL00~SL10`이다. **`RNM00~RNM08`과 `MaterialExtra00`은 이 receipt에
없다.** 파일 mtime도 다르다 — receipt는 9/2 03:24, RNM shard는 9/12 12:05.

RNM shard와 베른 authoring `mapmaterials.json`(23,153행 / placementLighting 49,047행)을 추가한
커밋은 **`359412c4` "Bern Restore, Phase 2, Alt V Effect"** 하나다(`--diff-filter=A`로 확인).

### 2.2 그 경로는 문서로만 남아 있다

정본 기록은 `.md/GB/09-11/2026-09-11_BERN_LIGHTING_GEOMETRY_IMPLEMENTATION_RESULT.md`다.
거기 적힌 베른 실적:

- 원본 component RNM: static 31,746 + foliage 17,651 = **49,397개**
- material signature/atlas/원본 component color로 구분한 **15,483 variant** 설치
- 최종 catalog 23 shard / 16,741 unique asset
- material 23,153행, placementLighting 49,047개, SDF 소비 placement 40,247개
- 설치 Resources: geometry 961 + component color 287 + RNM 4,778 + shadow 2,265 = **8,291개 / 209,807,631 bytes**
- **ShadowMap2D 40,594개와 ShadowMapTexture2D 2,306개를 원본에서 해독**

그런데 그 작업의 증거 폴더가 **사라졌다.**

```
out/BernLightingRestore20260911   -> ABSENT
out/BernMaterialAudit20260911     -> ABSENT
```

`out/`은 gitignore 대상이라 정리 시점에 없어졌다. 그리고 `Tools/`에서 `BernLightingRestore`나
`installed_resources`를 참조하는 committed 도구는 없다(유일한 grep 히트
`Tools/EffectPipeline/build_kouku_pattern_native.py`는 쿠크 이펙트용으로 무관하다).

**즉 베른의 21,321 baked 행은 커밋되지 않은 일회성 스크립트가 만들었고 그 스크립트는 지금 없다.**
베른 결과물(authoring JSON + shard + Resources DDS 7,043개)만 남아 있다.

### 2.3 Resources 대비

| | 베른 | 마하라카 |
|---|---|---|
| `Client/Bin/Resources/Map/Lighting/<Area>/` 파일 수 | **7,043** | **1** |
| 그 1개의 정체 | — | `lv_ocn_dookyis_lut.dds` (색보정 LUT, lightmap 아님) |

## 3. 진짜 병목: manifest builder에 RNM 경로가 없다

committed 3단 구조를 실제 코드로 확인했다.

| 단계 | 파일 | RNM 지원 |
|---|---|---|
| ① component 추출 | `extract_source_map_component_lighting.py` | **있음** — `status: RNM_TEXTURE_LIGHTMAP`, average/directional/coordScale/coordBias까지 파싱 |
| ② manifest 생성 | `build_source_map_material_inputs.py` | **없음** |
| ③ compiler | `build_source_map_materials.py` | **있음** — `bakedLighting` 3필드 검증(`:183`), `placementLighting` 4필드 검증(`:424`), `status == 'RNM_TEXTURE_LIGHTMAP'` 요구(`:430`), 텍스처 출처 대조(`:439`) |

②의 실제 출력(`:294~297`):

```python
format='lostark-source-map-material-input', formatVersion=1, areaId=args.area_id,
textures=[...], slots=slots, placementLighting=[],
```

- `placementLighting=[]` — **항상 빈 배열**
- slot마다 `lightingEvidence='source-absent'` 하드코딩(`:259`)
- `bakedLighting`, `auxiliaryTextures` 자체를 만들지 않음
- 자기 주석이 인정함(`:288`): *"The original components carry ShadowMap2D and RNM lightmaps that
  this build does not consume; baked lighting stays unbound."*

**따라서 이 adapter를 마하라카에 돌리면 RNM이 실재하는 component에 "원본에 baked lighting 없음"이
기록된다.** 돌리지 않았다.

## 4. `UNSUPPORTED_NATIVE_LAYOUT` 3,348개는 `unresolved`다 — 데이터로 확정

이것이 이번 최대 성과다.

### 4.1 거부 이유는 RNM 부재가 아니다

3,348개 전부의 `reason`이 단일 문자열이다.

```
"shadow-map or vertex-shadow payload is not implemented"
```

`extract_source_map_component_lighting.py:70~71`:

```python
if shadows or shadow_vertices:
    raise UnsupportedNative("shadow-map or vertex-shadow payload is not implemented", reader.offset, header)
```

`shadowReferences`가 비어 있지 않으면 **lightmap을 읽기도 전에** 거부한다. 거부된 component의
`decodedPrefix`는 전부 `{lodCount, shadowReferences, vertexShadowCount}` 한 형태이고
`offsetInNativeTail: 16`, 즉 shadow 참조 직후에서 멈춘다.

### 4.2 그 뒤에 RNM이 온전히 들어 있다

`nativeTailHex`를 추출기 자신의 필드 순서로 디코드했다. 순서는 깨끗한 RNM component
`LV_OCN_EVENTIS_MHP_SL01:export:5265`(`shadowRefCount 0`,
`nativeTailCompletelyConsumed true`)와 추출기 코드(`:71~127`)로 확정했다.

```
u32 lodCount
u32 shadowRefCount / i32 shadowPackageIndex x N / u32 vertexShadowCount
u32 lightMapKind                       (2 == RNM texture lightmap)
u32 bakedLightGuidCount / 16b guid x N
{ i32 packageIndex, 3x f32 scale } x 3  (average, directional, null third)
2x f32 coordinateScale / 2x f32 coordinateBias
1b  hasColor
if hasColor: u32 stride, vertices, elementSize, count  then count x 4b BGRA
4b  terminal                           (반드시 0 네 바이트)
```

결과:

| 항목 | 값 |
|---|---|
| `lightMapKind == 2`인 component | **3,348 / 3,348 (100%)** |
| 꼬리를 정확히 소비하고 terminal이 0 네 바이트 | **3,348 / 3,348 (100%)** |
| `hasColor == 0` | 3,245 |
| `hasColor == 1` (원본 vertex color override) | 103 |

`hasColor == 1`인 103개도 추출기가 이미 가진 vertex color stream 형식
(`stride == elementSize == 4`, `vertices == count`)으로 정확히 소비된다.

대표 사례 `LV_OCN_EVENTIS_MHP_SL01:export:5220`:

- `lightMapKind = 2`, `bakedLightGuidCount = 1`,
  GUID `c0bb17497749e244848520b99b0cea75`
- coefficient 0 packageIndex 335 scale `[1,1,1]`,
  coefficient 1 packageIndex 283 scale `[2.071852922439575, 2.204793930053711, 2.240915298461914]`,
  coefficient 2 packageIndex 0 scale `[2,2,2]`
- `coordinateScale [0.0546875, 0.0546875]`, `coordinateBias [0.00390625, 0.94140625]`
- terminal 4바이트 전부 0

이 GUID는 깨끗한 RNM component 5265의 `bakedLightGuids`와 **같은 값**이다. 같은 baked light에
속한 이웃 component다.

### 4.3 그래서 회수 가능한 양

| 상태 | 수 | 판정 |
|---|---|---|
| `RNM_TEXTURE_LIGHTMAP` (이미 성공) | 729 | source-bound |
| `UNSUPPORTED_NATIVE_LAYOUT` (shadow gate) | 3,348 | **unresolved — 회수 가능** |
| `NO_LOD_LIGHTING_DATA` (`lodCount == 0`) | 72 | source-absent 후보 |
| skipped | 3 | 미확인 |
| 합 | 4,149 | |

**729 + 3,348 = 4,077 / 4,149 = 98.3%가 원본 RNM을 갖는다.** 1차에서 "3,348개는 RNM 유무 미확인"
이라고 남긴 항목이 이로써 해소됐다.

## 5. `lightingEvidence` 스키마 설계 — 결론이 바뀌었다

1차에서 지적한 스키마 공백(`source-bound`/`source-absent` 둘만 허용, `unresolved` 없음)은
`build_source_map_materials.py:374`에서 실재를 확인했다. 그런데 4절 결과 때문에 **`unresolved`
값을 새로 추가하는 것이 올바른 수정이 아니다.**

- 마하라카의 4,077개는 `unresolved`가 아니라 **`source-bound`가 되어야 한다.** 데이터가 있다.
- 진짜 수정 지점은 상류 두 곳이다.
  1. `extract_source_map_component_lighting.py:70~71`의 shadow gate를 들어내고
     shadow 참조를 기록한 뒤 lightmap 파싱을 계속한다(4.2의 필드 순서가 이미 검증됐다).
  2. `build_source_map_material_inputs.py`가 `bakedLighting`/`placementLighting`/
     `auxiliaryTextures`를 실제로 만든다.
- `source-absent`는 `NO_LOD_LIGHTING_DATA` 72개에만 쓴다.

즉 스키마를 넓히는 대신 **추출기의 gate를 정확히 여는 것**이 최소 변경이다. 이번에 코드는
바꾸지 않았고, 이 판단의 근거만 데이터로 남겼다.

## 6. 대표 자산 end-to-end — 착수하지 못했다

다음이 남아 있어 대표 1개도 완주하지 못했다. 정직하게 미완료로 적는다.

1. **`_RNM_<hex>` variant assetId를 만드는 committed 도구가 없다.** 베른 형식은
   `<base>_RNM_<12 hex>`(예: `MAP_006414294549_..._SM_KSS` →
   `..._SM_KSS_RNM_149D482C9D2A`)인데 `Tools/` 전체에서 이 이름을 **생성**하는 코드가 0건이다.
   유일한 히트 `build_source_sequences.py:250`은 `'_RNM_' not in r[0]`으로 **걸러내는** 쪽이다.
2. lightmap DDS 추출은 `extract_ue3_texture_mips.py`가 있고 `--expected-mip0`를 요구한다.
   SL01 component-lighting 출력의 `textures` 50개에 `serialOffset`/`serialBytes`/`serialSHA256`이
   있어 입력은 마련돼 있으나, mip0 기대 크기를 확정하지 않았다.
3. **UV1이 UV0 복제가 아닌지 숫자 검증을 하지 못했다.** 채널 존재만 1차에서 확인됐다.
   `_RNM_` variant는 Area별 lightmap UV1을 갖는 별도 cook이므로 이 검증이 선행 조건이다.

## 7. `*_mi` MaterialInstanceConstant 15종 — 원인 확정, 수정 미착수

`Tools/LevelPlacementExtractor/extract_ue3_material_graph.py:34`:

```python
EFFECT_MATERIAL_CLASSES = frozenset(("material", "decalmaterial"))
```

`:42`의 판정이 이 집합만 통과시키므로 `materialinstanceconstant`는 구조적으로 못 찾는다.
G06의 15건 exit 1이 이 때문이다.

한편 `materialinstanceconstant`를 실제로 다루는 committed 도구는 따로 있다.

```
Tools/EffectPipeline/build_effect_child_parent_resolution.py
Tools/EffectPipeline/evaluate_ue3_material_uniform_expressions.py
Tools/EffectPipeline/extract_ue3_material_shader_maps.py
Tools/EffectPipeline/build_valtan_source_material_evidence.py
Tools/BernCastlePipeline/build_bern_castle_decal_probe.py
그 외 5개
```

**`EFFECT_MATERIAL_CLASSES`에 `materialinstanceconstant`를 그냥 추가하는 것이 맞는지 판단하지
못했다.** MIC는 parent chain과 uniform expression을 갖는 다른 자료구조여서, graph 추출기가
`material` 가정으로 파싱하면 조용히 틀린 결과를 낼 수 있다. 위 EffectPipeline 도구들이
이미 parent resolution을 구현하고 있으므로 그쪽 경로를 재사용하는 것이 후보다. 근거를 더
읽어야 하므로 이번에는 변경하지 않았다.

## 8. 실행한 명령과 검증

| 명령 | 결과 |
|---|---|
| `git log --diff-filter=A -- .../LV_BER_BERNCASTLE.mapmaterials.json` | `359412c4` 단일 커밋 |
| `git log -- .../LV_BER_BERNCASTLE_RNM00.mapassets` | `359412c4` |
| `test -d out/BernLightingRestore20260911` | **ABSENT** |
| `test -d out/BernMaterialAudit20260911` | **ABSENT** |
| `grep -rln "BernLightingRestore\|installed_resources" Tools/` | 무관한 1건만 |
| `ls Client/Bin/Resources/Map/Lighting/Bern \| wc -l` | **7043** |
| `ls Client/Bin/Resources/Map/Lighting/Maharaka` | `lv_ocn_dookyis_lut.dds` 1개 |
| `python -B out/.../G04_round2/decode_unsupported_tails.py` | `RNM_DECODED_EXACT: 3348`, exit 0 |

**빌드는 돌리지 않았다**(다른 작업이 빌드 잠금을 사용). publisher도 `-Mode Publish`로 실행하지
않았다. 기존 회귀 `test_build_source_map_materials.py`는 이번에 코드를 바꾸지 않았으므로
재실행하지 않았다 — 1차에서 13 tests OK로 기록돼 있다.

## 9. 사용자 확인이 남은 것

없다. 화면에 나타나는 변경을 하지 않았다.

## 10. 다음 작업의 정확한 순서

1. **`extract_source_map_component_lighting.py`의 shadow gate를 연다.** 4.2의 필드 순서로
   shadow 참조를 기록하고 lightmap 파싱을 계속한다. 3,348개가 `RNM_TEXTURE_LIGHTMAP`이 되는지
   재추출로 확인하고 단위검사를 붙인다. 이것이 가장 작고 효과가 큰 변경이다.
2. **UV1 ≠ UV0 숫자 검증**을 대표 자산 1개에서 끝낸다. 실패하면 `_RNM_` variant cook 자체가
   성립하지 않으므로 이것이 다음 관문이다.
3. `_RNM_<hex>` variant 이름 규칙을 확정한다. 베른 결과물에서 hex 12자리가 무엇의 해시인지
   역산한다(후보: average+directional 텍스처 쌍 + component color signature).
4. lightmap DDS를 `extract_ue3_texture_mips.py`로 대표 1쌍 추출한다. `--expected-mip0`를
   component-lighting `textures`의 `serialBytes`에서 유도한다.
5. `build_source_map_material_inputs.py`(또는 그 후속)에 `bakedLighting`/`placementLighting`/
   `auxiliaryTextures` 생성을 구현한다. `lightingEvidence`는 4,077개를 `source-bound`,
   `NO_LOD_LIGHTING_DATA` 72개를 `source-absent`로 쓴다.
6. MIC 15종은 EffectPipeline의 parent resolution 경로 재사용 가능성을 먼저 읽는다.
   `EFFECT_MATERIAL_CLASSES` 단순 확장은 근거가 확인되기 전에는 하지 않는다.

## 11. 이번에 정정한 1차 기록

| 1차 기록 | 정정 |
|---|---|
| "베른은 이 compiler가 돌았고 마하라카는 안 돌았다" | 베른은 `build.receipt.json` 자체가 없다. 베른의 RNM은 **커밋되지 않은 별도 스크립트**가 만들었고 그 스크립트와 증거 폴더는 사라졌다 |
| "`UNSUPPORTED_NATIVE_LAYOUT` 3,348개의 RNM 유무 미확인" | **3,348개 전부 `lightMapKind=2`이고 꼬리를 정확히 소비한다. `unresolved`이며 회수 가능하다** |
| "schema에 `unresolved`를 추가해야 한다" | 마하라카 4,077개는 `source-bound`가 되어야 한다. 올바른 수정은 추출기 shadow gate를 여는 것이고, `source-absent`는 `NO_LOD_LIGHTING_DATA` 72개에만 쓴다 |
| `lightingEvidence`가 게시 출력에 있다 | compiler **입력** 전용이다. 베른 게시본 baked 행에는 이 필드가 없고 `bakedLighting` 4필드만 있다 |

---

# 3차: shadow gate 수정

10절 1번을 실제로 구현했다. 결과는 예측과 정확히 일치했고, **쿠크에서 예상하지 못한 잠복
고장을 함께 고쳤다.**

## 3.1 수정 전 숫자 확인

열기 전에 gate에 걸린 3,348개의 조건을 숫자로 확인했다.

| 필드 | 분포 |
|---|---|
| `lodCount` | `{1: 3348}` |
| `shadowRefCount` | `{1: 3348}` |
| `vertexShadowCount` | **`{0: 3348}`** |

`vertexShadowCount`가 전부 0이므로 vertex-shadow 경로는 이 데이터에 없다. 따라서 vertex shadow는
계속 거부하고 shadow-map 참조만 통과시키는 것이 안전하다.

## 3.2 실제 변경

`Tools/LevelPlacementExtractor/extract_source_map_component_lighting.py` 한 곳.
18,070 → 18,469 bytes (**+399**), ASCII·LF 유지.
백업: `out/.../G04_round2/backup/extract_source_map_component_lighting.py.before-shadow-gate`

변경 전:

```python
    if shadows or shadow_vertices:
        raise UnsupportedNative("shadow-map or vertex-shadow payload is not implemented", reader.offset, header)
```

변경 후:

```python
    if shadow_vertices:
        raise UnsupportedNative("vertex-shadow payload is not implemented", reader.offset, header)
    if shadows:
        # The ShadowMap2D payload itself is still not decoded; only the reference list above is
        # recorded. The RNM lightmap is serialized after that reference, so the parse continues.
        # The terminal check at the end of this function still requires the native tail to end
        # exactly, which is what keeps a mis-read from passing silently.
        header["shadowPayloadDecoded"] = False
```

설계 판단 세 가지.

1. **shadow payload를 읽었다고 기록하지 않는다.** `shadowReferences`와 `vertexShadowCount`는
   그대로 남기고, 새 필드 `shadowPayloadDecoded: false`로 "shadow는 안 읽고 lightmap만 읽었다"를
   명시한다. 나중에 ShadowMap2D를 복원할 때 이 표시로 대상을 찾는다.
2. **marker를 shadow가 있을 때만 추가한다.** shadow가 없는 component의 출력은 이전과
   바이트 동일하게 유지된다. 실측으로 확인했다 — 마하라카에서 marker를 가진 것이 정확히
   3,348개이고 나머지 801개(= 기존 RNM 729 + NO_LOD 72)에는 키가 없다.
3. **안전망은 기존 검사가 그대로 한다.** 좌표 ≤ 1, average/directional 텍스처 non-null,
   텍스처 이름이 `normalizedaveragecolor` / `directionalmaxcomponent`로 시작해야 함,
   terminal 4바이트가 0. 특히 텍스처 이름 검사 때문에 우연히 통과할 수 없다.

## 3.3 단위검사 4개 추가

`Tools/LevelPlacementExtractor/test_extract_source_map_component_lighting.py`
5,562 → 8,398 bytes, ASCII·LF 유지. 헬퍼 `shadowed()`로 shadow 참조 1개를 앞에 붙인 꼬리를 만든다.

| 검사 | 내용 |
|---|---|
| `test_shadow_reference_still_reads_the_rnm_lightmap_that_follows_it` | shadow 참조가 있어도 RNM을 읽고 `shadowPayloadDecoded=false`를 남긴다 |
| `test_shadow_reference_with_inexact_tail_is_still_unsupported` | 꼬리가 남거나 잘리거나 미지 스트림이면 계속 거부 |
| `test_vertex_shadow_payload_is_still_refused_even_with_a_valid_lightmap` | 유효한 lightmap이 뒤에 있어도 vertex shadow는 거부, marker도 안 붙음 |
| `test_components_without_shadows_do_not_gain_the_shadow_marker` | 일반 RNM·`NULL_LIGHTMAP`·`NO_LOD_LIGHTING_DATA` 출력 불변 |

회귀 결과:

```
test_extract_source_map_component_lighting.py   Ran 10 tests  OK   (기존 6 + 신규 4)
test_build_source_map_materials.py              Ran 13 tests  OK   (기존 그대로)
```

## 3.4 마하라카 재추출 — 예측과 정확히 일치

세 package를 다시 돌렸다. 출력은 `out/.../G04_round2/ComponentLighting_after/`.

| status | 전 | 후 |
|---|---|---|
| `RNM_TEXTURE_LIGHTMAP` | 729 | **4,077** |
| `UNSUPPORTED_NATIVE_LAYOUT` | 3,348 | **0** |
| `NO_LOD_LIGHTING_DATA` | 72 | 72 |
| skipped | 3 | 3 |

level별 후: SL01 `{NO_LOD 71, RNM 3752}`, PS `{NO_LOD 1, RNM 243}`, LAND01 `{RNM 82}`.
예측(729 → 4,077 / 3,348 → 0 / 72 유지)과 **한 건도 어긋나지 않았다.**

독립 검증으로 이전에 gate에 걸렸던 `LV_OCN_EVENTIS_MHP_SL01:export:5220`을 내 디코더 예측과
대조했다. 전부 일치한다.

| 항목 | 값 |
|---|---|
| `status` | `RNM_TEXTURE_LIGHTMAP` |
| `shadowReferences[0].className` | `shadowmap2d` |
| `shadowPayloadDecoded` | `false` |
| `bakedLightGuids` | `["c0bb17497749e244848520b99b0cea75"]` |
| `averageTexture` | `LV_OCN_EVENTIS_MHP_SL01.normalizedaveragecolor0_158` |
| `directionalTexture` | `LV_OCN_EVENTIS_MHP_SL01.directionalmaxcomponent0_158` |
| `directionalScale` | `[2.071852922439575, 2.204793930053711, 2.240915298461914]` |
| `coordinateScale` / `coordinateBias` | `[0.0546875, 0.0546875]` / `[0.00390625, 0.94140625]` |
| `nativeTailCompletelyConsumed` / `terminalZeroBytes` | `true` / `4` |

텍스처 이름이 실제 export 이름이고 semantic 검사를 통과했다는 점이 우연한 파싱이 아님을 뒷받침한다.

## 3.5 receipt status는 `PASS`가 됐다 — 위장이 아닌 이유

`:277`의 공식은 `FAILED_OUTPUT_PRESERVED if failures else PARTIAL_UNSUPPORTED if unsupported else PASS`다.
`unsupported` 목록이 비었으므로 세 package 모두 `PASS`다.

남은 72개는 숨겨지지 않는다.

- 전부 `lodCount == 0`이고 `nativeTailCompletelyConsumed: true`다(72/72 확인).
- 즉 "읽지 못한 데이터"가 아니라 **원본에 lighting 레코드가 없다**는 해독된 상태다. 이것이
  `source-absent`의 정당한 근거다.
- receipt의 `statusCounts`에 `NO_LOD_LIGHTING_DATA: 71/1/0`으로 계속 표시된다.

`PASS`는 "미해독 잔여 0"을 뜻하고 그것은 이제 사실이다. 다만 **이것이 원본 시각 동일성 PASS는
아니다.** lightmap DDS 추출과 `_RNM_` variant cook은 아직 남아 있다(10절 2~5번).

## 3.6 쿠크 회귀 — 바뀌었다. 그리고 그것이 고장을 고친 것이다

`LV_LUT_MIDNIGHTC_ED`의 staging 폴더(`LV_LUT_MIDNIGHTC_ED_20260829`)도 베른처럼 사라져서
논리→물리 package 쌍을 얻을 수 없었다. `Tools/LevelPlacementExtractor/extract_ue3_particle_module_closure.py`의
`obfuscate_package_name()`으로 계산했고, 마하라카 기존 3쌍에 대해 전부 일치함을 먼저 확인한 뒤 사용했다.

```
LV_LUT_MIDNIGHTC_ED_PS   -> 534P5WP4TCKLJK6DPE4PSL4PXI
LV_LUT_MIDNIGHTC_ED_SL01 -> 756R7YR6VEMNLM8FRG6RUN6RK74B
LV_LUT_MIDNIGHTC_ED_SL02 -> 756R7YR6VEMNLM8FRG6RUN6RK74I
LV_LUT_MIDNIGHTC_ED_SL03 -> 756R7YR6VEMNLM8FRG6RUN6RK74P
LV_LUT_MIDNIGHTC_ED_SL04 -> 756R7YR6VEMNLM8FRG6RUN6RK74W
LV_LUT_MIDNIGHTC_ED_SL05 -> 756R7YR6VEMNLM8FRG6RUN6RK743
```

백업본(원본 추출기)과 수정본을 같은 6 package에 각각 돌려 component 단위로 비교했다.
출력은 `Kouku_before/`, `Kouku_after/`.

component 2,951개의 top-level `status` 전이(compiler가 실제로 읽는 필드):

| 전이 | 수 |
|---|---|
| `UNSUPPORTED_NATIVE_LAYOUT` → `RNM_TEXTURE_LIGHTMAP` | **1,324** |
| `RNM_TEXTURE_LIGHTMAP` → `RNM_TEXTURE_LIGHTMAP` | 1,193 |
| `NO_LOD_LIGHTING_DATA` → `NO_LOD_LIGHTING_DATA` | 365 |
| `UNSUPPORTED_NATIVE_LAYOUT` → `UNSUPPORTED_NATIVE_LAYOUT` | 69 |

**RNM을 잃은 component 0개, 새로 unsupported가 된 component 0개, `NO_LOD` 수 완전 동일.**
남은 69개는 다른 원인이며 내 수정이 실패한 것이 아니다 — vertex-shadow 39개,
`lightmap kind 1` 18개, `lightmap kind 0` 12개다.

### 그리고 여기서 잠복 고장이 드러났다

쿠크는 마하라카와 달리 **이미 게시된 baked lighting을 갖고 있다**
(`LV_LUT_MIDNIGHTC_ED.mapmaterials.json`: 1,734행 중 `bakedLighting` 1,355행,
`placementLighting` 2,368개). 그 2,368개가 의존하는 component의 상태를 조사했다.

| 게시본이 의존하는 component의 상태 | 수 |
|---|---|
| 수정 전 `UNSUPPORTED_NATIVE_LAYOUT` → 수정 후 `RNM_TEXTURE_LIGHTMAP` | **1,194** |
| 수정 전·후 모두 `RNM_TEXTURE_LIGHTMAP` | 1,170 |
| 내 추출 범위에 없음 | 4 |

`build_source_map_materials.py:430`은 각 `placementLighting` instance에 대해
`original['status'] == 'RNM_TEXTURE_LIGHTMAP'`을 요구한다. 따라서 **수정 전 추출기로 쿠크
material build를 다시 돌리면 2,368개 중 1,194개가
`RNM source component is missing/unsupported`로 실패한다.**

즉 committed 추출기는 쿠크 게시본을 만든 (사라진) 스크립트보다 기능이 후퇴한 상태였고,
내 수정이 그 재현성을 복구한다. **쿠크 결과가 바뀐 것은 맞지만 방향이 회수(gain)뿐이고,
내가 만든 회귀가 아니라 기존 잠복 고장을 고친 것이다.** 조건을 더 좁힐 이유가 없다고 판단했다.
게시본 자체는 다시 만들지 않았으므로 `Client/Bin/DataFiles`와 `Data/Maps`는 무변경이다.

## 3.7 실행한 명령

| 명령 | 결과 |
|---|---|
| `patch_shadow_gate.py` | +399 bytes, ASCII·LF 유지, 백업 생성 |
| `patch_shadow_gate_tests.py` (+ index 수정) | 5,562 → 8,398 bytes |
| `unittest ... test_extract_source_map_component_lighting.py` | **Ran 10, OK** |
| `unittest ... test_build_source_map_materials.py` | **Ran 13, OK** |
| 마하라카 3 package 재추출 | 3× exit 0, 전부 `PASS` |
| 쿠크 6 package × (before, after) | 12회 실행, 전부 exit 0 |

**빌드는 돌리지 않았다**(다른 작업이 빌드 잠금 사용). publisher `-Mode Publish` 미실행.
`Client/Bin/DataFiles`, `Client/Bin/Resources`, `Data/**` 무변경.

## 3.8 남은 것 (10절과 동일, 1번만 완료)

1. ~~shadow gate 열기~~ → **완료.** 마하라카 4,077 / 쿠크 +1,324 회수.
2. UV1 ≠ UV0 숫자 검증 — 미착수. 다음 관문.
3. `_RNM_<hex>` variant 이름 규칙 역산 — 미착수. 생성 도구가 저장소에 없다.
4. lightmap DDS 추출(`extract_ue3_texture_mips.py`, `--expected-mip0` 유도) — 미착수.
5. manifest builder에 `bakedLighting`/`placementLighting`/`auxiliaryTextures` 구현 — 미착수.
   `lightingEvidence`는 4,077개 `source-bound`, 72개 `source-absent`.
6. `*_mi` MIC 15종 — 원인만 확정, 수정 미착수.

추가로 이번에 알게 된 항목 하나: **쿠크 material build를 재현하려면 이 수정이 필요하다.**
쿠크 게시본을 재생성할 계획이 있으면 이 수정이 선행 조건이다.
