# 2026-09-27 마하라카 G03 — 워터팡 무대 원본 재질 RESULT

설계: `.codex/worktrees/7395/LostArk/.md/GB/09-27/2026-09-27_MAHARAKA_VISUAL_PARITY_EXECUTION_PLAN.md` G03.
실행 저장소 `C:/Users/USER/source/졸업팀폴/LostArk`, branch `codex/main-ship-maharaka-0926`, HEAD `a84bbcd4`.

**결론 요약:** 무대가 회색인 이유를 원본 바이트로 확정했고, 그것을 막고 있던 **공유 도구 회귀 1건을
고쳤다.** 두 MIC는 이제 완전히 resolve된다. 그러나 **mapmaterials 행은 만들지 않았다** — 마지막
관문이 공유 계약 판단을 요구하며, 근거 없이 고르는 것을 설계가 금지하기 때문이다.

---

## 1. 관찰

- 무대 18조각이 배치돼 있으나 두 모델의 런타임 mapmaterials 행이 **0개**.
- 화면상 무대가 회색. 설치 manifest는 `materialComplete=false`.

## 2. 확인한 원인

### 2.1 무대가 회색인 직접 원인 (원본 값으로 확정)

두 MIC를 완전히 resolve한 결과, **무대 색은 텍스처가 아니라 `diffuse_color` 벡터가 만든다.**

| 항목 | A `bg_ocn_etc_floor01a_mi_lnh` | B `bg_ocn_etc_floor01b_mi_lnh` |
|---|---|---|
| terminal | `efbasematerial_prologue.bg.base.bg_base_opa` | 동일 |
| baseId | `0426f753b735d246a1824db75d78ab52` | 동일 |
| `texture_diffuse` | **None** (`packageIndex 0`) | **None** (`packageIndex 0`) |
| `diffuse_color` | `[0.9674, 0.5337, 1.0, 1.0]` | `[0.9407, 0.1517, 1.0, 1.0]` |
| `diffuse_brightness` | **2.0** | 1.0 |
| `reflection_color` | `[0.9820, 0.7528, 1.0, 1.0]` | `[0.9717, 0.5955, 1.0, 1.0]` |
| `reflection_intensity` | 0.2 | 0.2 |
| `specular_power` / `intensity` | 150.0 / 1.0 | 150.0 / 1.0 |
| `texture_normal` | `bg_ocn_etc_g.tex.bg_ocn_etc_floor01a_n_lnh` | 동일(A의 노멀 공유) |
| switches ON | `1.use_normalmap`, `1.use_reflection`, `1.use_specular`, `1.use_specular_texture` | 동일 |

즉 A는 연분홍/라벤더(밝기 2배), B는 자홍/보라다. **9+9 교대 배치의 두 색이 이 두 값이다.**
diffuse 텍스처가 원본에 없으므로, 이 벡터가 행으로 실리지 않으면 모델은 회색으로 렌더된다.
설계 G03-1의 "diffuse가 flat_gray라고 실패로 단정하지 말 것. tint로 색을 만드는 원본일 수 있다"가
정확히 맞았다. `sourceOnlyUnsupported`에 `vector:diffuse_color`가 들어 있는 것도 같은 사실이다.

`materialComplete=false`는 손상이 아니다. 이 필드들은 `.wmodel`이 실을 수 없는 source 전용 값이고
mapmaterials 행이 실어야 한다. cook 단계에서는 정상적으로 false다.

### 2.2 그것을 막고 있던 공유 도구 회귀 (고침)

두 MIC를 추출하려 했으나 A가 실패했다.

```
status FAILED_OUTPUT_PRESERVED
failures: lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01a_mi_lnh — "normal override boolean is invalid"
```

B는 성공, A는 실패. 차이는 패키지가 아니라 **`normalParameters` 배열의 존재 여부**였다.
A는 3개(`texture_normal`, `texture_detail_normal`, `texture_overlay_normal`), B는 0개.

원본 바이트에서 layout을 확정했다(A, baseId offset 108):

```
[0] texture_normal         eb030000 00000000 01 01000000 637fbb65360aa644aab816f5c7257fa1
[1] texture_detail_normal  e4030000 00000000 01 00000000 1d6e464035c00a4990475172e70c1d64
[2] texture_overlay_normal ee030000 00000000 01 00000000 104329d41ae5574da5bd4b31d2aeb67a
```

= FName 8 + 압축 1바이트 + bOverride uint32 4 + expression GUID 16 = **29바이트**. override 값은
1/0/0으로 모두 유효하다. 즉 원본은 정상이었다.

**회귀의 정체:**

| 위치 | 내용 |
|---|---|
| `extract_artist_31470_shader_cache_oracle.py:494~499` | `STATIC_PARAMETER_ARRAY_LAYOUTS`의 `normalParameters` entry_size |
| `:560~562` | 압축을 `<BI` 로 +8에서 읽고 GUID를 +13에서 읽음 (29바이트 계약) |
| `extract_source_map_material_parameters.py:81~84` (수정 전) | 네이티브 1바이트를 `struct.pack("<I", raw[8])`로 **uint32로 확장** → 엔트리당 **32바이트** 기록 |

`git log -L`로 확인한 시간선:

1. `560741ac` — map 추출기 생성. 당시 oracle entry_size는 **32**였으므로 확장이 맞았다.
2. 2026-09-19 04:15 — 이 Area의 `parameters.json` 추출 성공(291 재질, 그중 **154개가
   normalParameters 보유**, 총 459행).
3. **`88fa743d` (2026-09-25 14:44, "쿠크 관문·빙고 패턴과 5개 클래스 무비 복구 및 공용 저작 도구 개선")**
   — oracle의 `normalParameters`를 **32 → 29**로 바꾸면서 map 추출기의 확장을 함께 고치지 않았다.
4. 2026-09-27 — 09-25 이후 첫 map 재질 추출. 엔트리당 3바이트씩 어긋나 실패.

**이 회귀는 무대만의 문제가 아니다.** 이 Area의 154개 재질이 같은 경로를 탄다.

**결정적 확인:** 저장소에 이미 이 회귀를 잡는 테스트가 있었고 **빨간 상태로 방치돼 있었다.**
`test_extract_source_map_material_parameters.py::test_native_normal29_and_numbered_fname_preserve_original_identity`
(이름에 `normal29`가 있다). 수정 전 코드로 되돌려 실행하면 동일한
`ValueError: normal override boolean is invalid`로 **1 error**, 수정 후 **7/7 통과**다.
따라서 새 테스트를 추가하지 않았다. 기존 테스트가 이 계약을 정확히 덮는다.

### 2.3 남은 마지막 관문 (미해결, 근거는 확보)

수정 후 두 MIC 모두 `status PASS / materials=2 / failures=0`로 resolve된다. 그러나 adapter가
아직 이 슬롯을 만들지 못한다.

`build_source_map_material_inputs.py:185~193`의 `texture()`가 source 값이 `None`이면
`active texture %s is NULL or unresolved`로 거부한다. `bg_base_opa` 경로는
`tex('diffuseTexture','texture_diffuse','diffuse')`를 무조건 호출하고,
`1.use_specular_texture`가 ON이므로 `texture_specular`도 호출된다. 두 값이 모두 None이다.

그런데 그 None은 "미해결"이 아니다. `parameterSources`가
`kind: MIC_SERIALIZED_OVERRIDE`, **`packageIndex: 0`** 으로 기록한다 — MIC가 **명시적으로 None으로
덮어쓴** 것이다. 부모를 따로 resolve하면 실제 기본값이 나온다:

| 파라미터 | 부모 `efbasematerial_prologue.bg.base.bg_base_opa` 기본값 | kind |
|---|---|---|
| `texture_diffuse` | `efmaster_material_prologue.tex.diffuse` | `MATERIAL_EXPRESSION_DEFAULT` |
| `texture_specular` | `efmaster_material_prologue.tex.spec` | `MATERIAL_EXPRESSION_DEFAULT` |

**그리고 cook이 이미 정확히 그 두 개를 설치했다.** 설치 manifest의 파일 목록에
`9934688136d4_diffuse.dds`와 `9934688136d4_spec.dds`가 있다(두 파일의 sha256이 동일한 것은 두
엔진 placeholder 텍스처의 바이트가 같아 내용주소로 중복 제거된 결과다). 즉 **별개 소비자인
ModelAssetConverter cook은 이 None override를 "부모 expression 기본값으로 폴백"으로 해석했다.**

따라서 필요한 변경은 "MIC 텍스처 override가 `packageIndex 0`이면 부모 기본값을 덮지 않는다"이지만,
이것은 `assign()` 수준의 **공유 추출기 의미 변경**이며 291개 재질 전체(09-19 보고서의
`texture_diffuse NULL` 3건, `texture_specular NULL` 2건 포함)에 영향을 준다.
회귀 수정과 달리 이것은 의미 변경이므로 별도 검증 사이클이 필요하다.
**착수하지 않았다.** 절반만 바꿔 두는 것이 더 나쁘기 때문이다.

## 3. 바뀐 파일

| 파일 | 변경 |
|---|---|
| `Tools/LevelPlacementExtractor/extract_source_map_material_parameters.py` | **+5/−5줄 1곳.** `normalParameters` 확장 제거, 네이티브 1바이트 유지. 영문 주석. LF·ASCII·무BOM·개행종료 보존 (22,962 → 23,148 bytes) |

백업: `out/MaharakaVisualParity_G03/backup/extract_source_map_material_parameters.py.before`

`Data/**`, `Client/Bin/DataFiles/**`, `Client/Bin/Resources`에 **한 바이트도 쓰지 않았다.**
게시·빌드·publisher 모두 실행하지 않았다(VS devenv pid 26212 실행 중, 빌드 금지 준수).

조사 산출물(전부 `out/MaharakaVisualParity_G03/`): `diag_normalparams.py`,
`diag_stage_mic.py`, `verify_fix_inmemory.py`, `fix-inmemory-summary.json`,
`patch_normalparams_stride.py`. 추출 결과는 ASCII 경로 스크래치
`C:/LostArkExtract/G03scratch/`(UModel이 한글 경로를 깨뜨리므로): `stage-parameters.json`,
`stage-parameters.receipt.json`, `parents-out.json`, `parents.receipt.json`.

## 4. 실제 소비자

수정한 함수는 `MapStaticSetDecoder.decode`이고 소비 경로는
`extract_source_map_material_parameters.py` → `parameters.json` →
`build_source_map_material_inputs.py`(adapter) → `build_source_map_materials.py`(compiler) →
Authoring `mapmaterials.json` → `Publish-MapAuthoring.ps1`. 이번에 1~2단계를 복구했고
3단계 이후는 2.3의 관문 때문에 실행하지 않았다.

## 5. 자동 검사 결과

| 검사 | 결과 |
|---|---|
| `test_extract_source_map_material_parameters` | **exit 0 (7 tests)** — 수정 전 1 error였다 |
| `test_build_map_material_variants` | exit 0 (20 tests) |
| `test_build_maptool_scene` | exit 0 (36 tests) |
| `test_build_source_map_materials` | exit 0 (13 tests) |
| `test_extract_source_map_component_lighting` | exit 0 |
| 무대 두 MIC 추출 | `status PASS / materials=2 / failures=0` |
| 부모 2개 추출 | `status PASS / materials=2 / failures=0` |
| Resources `Map/LV_OCN_EVENTIS_MHP` | 384 자산 / 6,323 파일 / wmodel 384 (불변) |
| Resources `Map/Lighting/Maharaka` | 137 파일 (불변) |
| 게시 catalog / placements | 408 / 4,669 (불변) |

빌드 0회. publisher 0회. 잔류 python/umodel 프로세스 0개.

## 6. 조율자 질문에 대한 답 — `materialName`은 선택이 아니다

**근거:** `Tools/MapPipeline/Publish-MapAuthoring.ps1:2811~2812`

```powershell
if (-not $modelNames[$modelPath].ContainsKey($row.materialName) -or $modelNames[$modelPath][$row.materialName] -lt 1) {
    throw "Map diffuse sampler material does not exist: $($row.assetId)/$($row.materialName)"
}
```

publisher가 `materialName`을 **cook된 `.wmodel`이 실제로 선언한 재질 이름 집합**과 대조한다
(`:2794` 필수 필드, `:2803` `(assetId, materialName)` 중복 금지, `:2800` 63바이트 상한).
compiler 쪽은 `build_source_map_materials.py:318`에서 슬롯 키로 그대로 받는다.
따라서 이름은 스타일 문제가 아니고 **모델마다 cook이 정한 값**이다.

**조율자의 전제를 정정한다.** "조사한 행 전부 dummy"는 사실이 아니다. 실측:

- 329행 중 `SLOT_000_dummy_material_0` **23행**, 실제 이름 **306행**.
- 예: `MAP_1ED56B72D40F_BG_ATM_LEONHART_ROADPROP02_SM_OLD` → `SLOT_000_bg_shs_common_deco03_mi_old`,
  그 wmodel이 선언한 이름도 동일.
- dummy 23행도 오류가 아니다. 예: `MAP_EA9C52CC4464_LV_BER_BERNILF_FLOOR01_SM_OVR_6A0EFBCC18F9`의
  wmodel이 실제로 `SLOT_000_dummy_material_0`만 선언한다. 행이 cook을 정확히 반영한 것이다.

**따라서 무대는 실제 이름을 써야 한다.** wmodel 바이트에서 확인:

- `MAP_167F91F6940F_..._OVR_0636FCF08A2E` → `SLOT_000_bg_ocn_etc_floor01a_mi_lnh`
- `MAP_AF1951C8B827_BG_OCN_ETC_FLOOR01B_SM_LNH` → `SLOT_000_bg_ocn_etc_floor01b_mi_lnh`

설치 manifest의 `runtimeName`과 일치한다. 일관성 문제는 없다.

## 7. 동시 편집 관찰 (내가 하지 않은 변경)

작업 중 `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapmaterials.json`이
바뀌었다. 시작 시 sha256 `8ad7ecdb3eb926c1` / placementLighting 0 → 03:09:44 시점
sha256 `ab9a47763076fe84` / **placementLighting 231**. rows 329와 water 10은 유지.
병행 G04(RNM join) fork의 쓰기로 보인다. **나는 이 파일을 읽지도 쓰지도 않았다.**
내 변경은 `Tools/` 1파일뿐임을 `git diff --numstat`(5/5)으로 확인했다.
다음 작업자는 행을 쓸 때 반영 직전 최신 내용을 다시 읽고 재병합해야 한다.

## 8. 사용자 확인 결과 / 미확인

**전부 미확인.** 화면에 나타나는 변경을 만들지 않았다. 무대는 여전히 회색이다.
게시물·Resources·실행 파일을 건드리지 않았으므로 지금 들어가도 이전과 동일하다.
visual PASS를 선언하지 않는다. 최종 색 판정은 사용자가 한다.

기대값을 미리 적어 둔다(다음 단계가 행을 실은 뒤 확인할 것): 무대 중심 노랑이 아니라
**A는 연분홍/라벤더(밝기 2배), B는 자홍/보라**가 9+9 교대로 나타나야 하고, `reflection_intensity`
0.2의 약한 반사가 함께 보여야 한다. 설계 G10-2의 "노랑 중심/핑크·보라"와 대조해
노랑이 필요하면 그 근거를 다시 찾아야 한다 — 현재 원본 값에 노랑은 없다.

## 9. 다음 항목

1. **`packageIndex 0` 텍스처 override 의미 결정** (최우선, 공유 계약). cook이 이미 부모 expression
   기본값으로 폴백한 선례가 있다. 적용 범위는 291 재질 전체이므로 09-19 slot_report 대비 전후
   비교와 단위검사가 필요하다. 풀리면 무대 2슬롯 + 09-19에서 같은 이유로 skip된 5슬롯이 함께 열린다.
2. 그 뒤 adapter → compiler → Authoring `mapmaterials` → `Publish-MapAuthoring.ps1`
   Validate → Publish → Check. 게시 전후 배치 4,669 / 자산 408 / material 329행 이상 / 물 10행
   유지를 수치로 확인. **G04와 동시 편집이므로 반영 직전 재병합 필수.**
3. `88fa743d`가 oracle을 32→29로 바꿀 때 map 추출기를 함께 고치지 않은 점은 팀에 공유해야 한다.
   이 계약을 공유하는 다른 소비자가 더 있는지 확인이 필요하다.
4. Scene03B Matinee의 material parameter track 19개(설계 G03-4)는 착수하지 않았다. 정지 상태 값과
   회전/붕괴 중 변화를 구분해야 하며, 정지 상태 행이 먼저 있어야 의미가 있다.

## 10. source-exact / project-tuned 구분

- **source-exact:** 2.1 표의 모든 값(terminal, baseId, diffuse_color, diffuse_brightness,
  reflection_color/intensity/contrast/tiling, specular_*, normal_intensity, uv_tiling,
  diffuse_saturation, 텍스처 참조, switches), 2.2의 29바이트 layout과 override 1/0/0,
  2.3의 부모 기본값 2개. 전부 원본 패키지 바이트에서 읽었다.
- **project-tuned:** 없음. 이번에 값을 만들어 넣지 않았다.
- **미해결:** `packageIndex 0` override의 엔진 의미(cook 선례는 있으나 계약으로 확정되지 않음).
