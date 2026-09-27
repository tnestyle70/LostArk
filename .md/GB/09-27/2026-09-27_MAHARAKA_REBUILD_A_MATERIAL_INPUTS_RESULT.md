# 2026-09-27 마하라카 재구축 A단계 — 재질 입력 계층 RESULT

범위: 입력 계층(추출 → 어댑터 → 컴파일러 후보)까지. **게시는 하지 않았다.**
`Data/**`, `Client/Bin/DataFiles/**`, `Client/Bin/Resources/**`에 쓰지 않았고 publisher도 빌드도 실행하지 않았다(devenv pid 26212 열려 있음).

---

## 1. 관찰

- 382개 자산 전부 `geometry-preview-partial-material`, `sourceMaterialBuild: null`.
  09-19 `rebuild_all.sh`에 source 재질 빌드 단계가 아예 없었다.
- 게시 정본 `mapmaterials.json`은 329행인데 catalog는 408 자산. 130개 자산에 확장할 행조차 없다.
- 증분 패치가 매번 같은 벽에 부딪혔다: 어댑터가 명시적 None 텍스처를 거부해서 슬롯이 만들어지지 않는다.
- 사용자 화면: 무대가 회색. 원본 홍보 이미지 확대분은 분홍/자홍과 **노랑**이 번갈아 나오는 방사형 쐐기 + **중앙 노란 원반**.

## 2. 확인된 원인

### 2.1 `packageIndex 0`은 직렬화된 NULL 객체 참조다 (추측 아님)

근거 체인 전부 실측:

| 근거 | 위치 | 내용 |
|---|---|---|
| 부모가 먼저 해석된다 | `extract_source_map_material_parameters.py:213` | `result = copy.deepcopy(self.resolve(parent))` — 자식 override 적용 전에 부모 `MATERIAL_EXPRESSION_DEFAULT`가 이미 들어 있다 |
| 0은 None으로 변환된다 | 같은 파일 `:103~107` | `source_reference()`가 `reference == 0`에 `return None` |
| None이 부모값을 지운다 | 같은 파일 `:117~122` | `assign()`이 무조건 덮어써서 상속된 기본값이 사라진다 |
| 출시된 소비자는 반대로 해석한다 | `Client/Bin/Resources/Map/KakulSaydon/**` | 게시된 Area 자산들이 자산별 `9934688136d4_diffuse.dds` / `_spec.dds`를 갖고 있다 — 부모 expression default가 실제로 해석됐다 |
| 09-19 catalog도 그 정체를 인정한다 | `admitted.mip-catalog.json` | 맨 이름 `tex.diffuse` / `tex.spec`가 **같은 sha `9934688136d4`**로 등재돼 있다 |
| 어댑터에 이미 그 통로가 있다 | `build_source_map_material_inputs.py:153` | `'.tex.' in key` → `catalog.get('tex.' + ...)` fallback. 엔진 패키지 부모 기본값이 `texture()`에 도달할 것을 전제한 코드이고, docstring이 이 alias를 "허용된 유일한 이름 추론"으로 명시한다 |

결론: `packageIndex 0`은 "이 인스턴스는 이 슬롯에 텍스처를 공급하지 않음"이며 부모값이 유지돼야 한다.
이것은 cooker가 부모 텍스처를 설치했다는 사실 **하나만으로** 내린 판단이 아니다. 위 6개 근거의 교차 확인이다.

### 2.2 네 경우를 구분했다 (조건 2)

코디네이터 지시대로 **0·false를 부모값으로 되돌리지 않도록** 경우를 나눴다.
측정 대상은 `LV_OCN_EVENTIS_MHP` 전체(main 291 재질 + foliage 8).

| 경우 | 직렬화 바이트 패턴 | 처리 | 실측 건수 |
|---|---|---|---|
| ① 파라미터 자체가 없음 | `textureParameters` 배열에 해당 `FName` 행이 **부재** | 부모 기본값 상속(기존 동작 유지) | **1,632** 슬롯 |
| ② 명시적 null 텍스처 | `FTextureParameterValue` 행 존재 + `ParameterValue` 필드의 `packageIndex == 0` (int32 리터럴 0) | **부모 기본값 유지**, `micNullOverridesIgnored`에 무시 기록 | **15** 슬롯 |
| ③ 참조 해석 실패 | `packageIndex != 0`이지만 import/export 범위를 벗어남 → `package_ref_path()`가 `break`하고 `""` 또는 끝이 `.`인 이름 반환 | **오류**(`require()`로 즉시 실패). 값으로 배정하지 않는다 | **0**건(오늘 데이터에는 없음, 가드만 추가) |
| ④ 숫자 0 / 불리언 false | `FScalarParameterValue`의 float `0.0`, `FStaticSwitchParameter`의 byte `0` — **객체 참조가 아니라 값 필드** | **원본 값으로 보존**. 내 수정 경로가 이 필드를 건드리지 않는다 | 스칼라 0.0 **99**, 전부-0 벡터 **58**, switch false **7,901** |

③은 실제 발생이 0건이지만 경로가 조용히 `""`를 만들어 한참 뒤 "recooked catalog에 텍스처 없음"으로만 실패했다. 디코드 지점에서 오류가 되게 가드를 넣었다(A-1d).

### 2.3 0 / false 보존 수치 증명 (조건 2)

- 내 diff에 `scalarParameters` / `vectorParameters` / `staticSwitchParameters`를 언급하는 줄: **0줄**
- `values`(스칼라·벡터)가 달라진 재질: **0개**
- `switches`(불리언)가 달라진 재질: **0개**
- 스칼라 0.0 개수: **99 → 99**
- 전부-0 벡터 개수: **58 → 58**
- switch `false` 개수: **7,901 → 7,901**
- 텍스처 슬롯 값 변화 유형: **전부 `None -> value` 15건뿐.** `value -> *` 변화 0건

즉 0·false를 가진 재질은 하나도 바뀌지 않았다.

### 2.4 무대가 회색인 진짜 이유 — PNG로 설치된 normal map

무대 슬롯을 끝까지 따라가니 `packageIndex 0`과 **별개인** 두 번째 차단이 나왔다.

어댑터 `_installed_texture()`는 runtime manifest의 **`.dds` 파일만** sha256으로 색인한다
(`build_source_map_material_inputs.py`, `relative = self.installed.get(entry['sha256'])` → None이면 거부).
그런데 다음 4종 normal map이 Resources에 **PNG로 설치돼 있다**:

| 텍스처 | 설치 파일 | 막는 슬롯 |
|---|---|---|
| `bg_lut_lucastle_f.tex.bg_lut_lucastle_houseinfloor01_n_ysi` | `.png` | 12 |
| `bg_rad_abrelshud_e.tex.bg_rad_abrelshud_floor18_n_ksr` | `.png` | 6 |
| `bg_ocn_etc_g.tex.bg_ocn_etc_floor01_n_lnh` | `f5b4fe6ad756_..._n_lnh.png` | 1 ← **중앙 노란 원반** |
| `bg_ocn_etc_g.tex.bg_ocn_etc_floor01a_n_lnh` | `a33a13c31444_..._n_lnh.png` | 2 ← **무대 쐐기 A/B** |

합계 **21슬롯**이 이 한 가지 이유로 막혀 있고, **무대 전체(중앙 원반 + 18 쐐기)가 그 안에 있다.**
슬롯 하나가 거부되면 자산 전체가 불합격이므로(전 슬롯 통과 규칙) 무대는 mapmaterials 행을 **0개** 갖는다.

무대가 회색인 이유는 "원본에 색이 없어서"가 아니고 "임의로 칠해야 해서"도 아니다.
**normal map이 DDS로 설치되지 않아 슬롯 전체가 거부되기 때문**이다. 원본 색 값은 추출본에 온전히 있다.

이 수정은 Resources 설치/쿠킹 영역이라 A단계 경계(Resources 읽기 전용) 밖이다. B단계 항목으로 남긴다.

## 3. 수정한 파일

내가 수정한 파일은 **정확히 2개**다. `git diff --check` 결과 공백 오류 없음.

| 파일 | 변경 | 내용 |
|---|---|---|
| `Tools/LevelPlacementExtractor/extract_source_map_material_parameters.py` | +45 | MIC 텍스처 override 판단을 모듈 수준 `apply_mic_texture_override()`로 분리. 경우 ②는 부모값 유지 + `micNullOverridesIgnored` 기록, 상속값이 없으면 종전처럼 None 배정(슬롯이 조용히 사라지지 않고 미해결로 남음). 경우 ③ `require()` 가드 |
| `Tools/LevelPlacementExtractor/test_extract_source_map_material_parameters.py` | +68 | `MicNullTextureOverrideTest` 6개 추가(7 → 13개) |

LF·ASCII·BOM 없음 유지, 앵커 1회 일치 python 패치로만 적용. 백업 5개는 `out/MaharakaRebuild_A/backup/`.

어제 들어간 4개 수정은 **되돌리지 않았다**: `extract_source_map_material_parameters.py`(stride),
`extract_source_map_component_lighting.py`(shadow gate), `build_map_material_variants.py`(prune 안전장치),
`build_maptool_scene.py`(Water 허용). `build_source_map_material_inputs.py`(다른 세션 미추적 파일)는 **읽기만** 했다.

## 4. 실제 consumer

- 추출 `parameters.json` → 어댑터 `build_source_map_material_inputs.py` → 컴파일러 `build_source_map_materials.py` → Authoring `mapmaterials.json` → `Publish-MapAuthoring.ps1` → 런타임.
- `materialName`은 cook이 정한다. publisher `:2811~2812`가 `.wmodel`이 선언한 이름과 대조한다. 내 후보 행 이름은 전부 그 규칙으로 생성됐다.
- 어댑터·컴파일러 두 단계 모두 실행해 후보까지 흘려 봤다. 게시 단계는 실행하지 않았다.

## 5. 자동검사 결과

### 5.1 회귀 6개 스위트 — 전부 exit 0, 합계 100개

| 스위트 | 개수 | exit |
|---|---|---|
| `test_extract_source_map_material_parameters` | 13 (7에서 증가) | 0 |
| `test_build_source_map_materials` | 13 | 0 |
| `test_build_map_material_variants` | 20 | 0 |
| `test_extract_source_map_component_lighting` | 10 | 0 |
| `test_build_maptool_scene` | 36 | 0 |
| `test_build_source_map_material_inputs` | 8 | 0 |

### 5.2 커버리지 before/after — "분류됨"과 "렌더링 경로 확인됨"은 다른 열이다 (조건 3)

동일 입력(같은 component lighting `G04_round2/ComponentLighting_after`)으로 before/after를 재서 delta를 유효하게 만들었다.
before 실행이 어제 기준선(241자산 / 319슬롯)을 정확히 재현하는 것을 먼저 확인했다.

| 지표 | BEFORE | AFTER |
|---|---|---|
| 결합된 자산 | 241 / 382 | **252 / 382** |
| 슬롯 | 319 | **330** |
| 텍스처 | 361 | **364** |
| None 텍스처 슬롯(main 291) | 15 | **0** |
| None 텍스처 슬롯(foliage 8) | 1 | **0** |
| NULL 텍스처 차단(`texture_normal`/`_diffuse`/`_specular`) | 16 / 3 / 3 = 22 | **0 / 0 / 0** |

`packageIndex 0` 차단군은 **완전히 닫혔다**. 다만 408 슬롯 전체가 해결된 것은 아니다.

**408 자산 분류 (조건 3의 두 열 분리)**

| 구분 | 분류됨 | 렌더링 경로 확인됨 | 비고 |
|---|---|---|---|
| (a) 완전 해결 — mapmaterials 경로 | 252 자산 / 330 슬롯 | ✅ 확인 | 컴파일러가 실제로 행을 생성 |
| (b) 부모 기본값 fallback으로 해결 | 위 252에 포함, 15 슬롯이 이 경로 | ✅ 확인 | 경우 ②, provenance 기록됨 |
| (c) 미해결 — 이름 있는 이유 | 188 슬롯 | — | 5.4 표 |
| (d) **분류됐지만 mapmaterials가 그 경로의 입력이 아님** | 지형 16 자산 | ❌ **다른 경로** | 5.3 |

### 5.3 다른 재질 경로를 쓰는 자산 — 형식적으로 행만 추가하면 안 되는 케이스

코디네이터가 경고한 케이스가 실재했다.

**지형 `LAND01_LC_*` 16 자산**
- catalog에 16행 존재(분류됨 ✅). 자체 리소스 트리 `Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/`에 16폴더 설치됨.
- 게시 정본 mapmaterials 행: **0**. 내 후보 행: **0**. 어댑터 슬롯: **0**.
- textures 내용: **`baked_diffuse.png`, `baked_normal.png`** + 원본 레이어 `lv_common_ocn_10_n.dds`, `oceansand_test01_001.dds`.
- 즉 변환기가 landscape layer weight blend를 **미리 하나의 baked 텍스처로 구워** 넣었고, 지형은 그 baked 경로로 그려진다.
  **mapmaterials는 지형 경로의 입력이 아니다.** 여기에 행을 추가해도 렌더링은 바뀌지 않는다.
- 사용자 화면 전경의 겨자색/초록/적갈색 패치는 이 `baked_diffuse.png`가 결정한다. 무대 노랑과 **별개 문제**다.

**물 10행 — mapmaterials 경로이지만 생성기가 다르다**
- 게시 정본에 `family=source.map.water-42.v1` 10행이 있다. 내 컴파일러 후보는 **0행**.
- `specialresource.mat.ocean_trn`이 컴파일러의 미지원 terminal(12슬롯 거부).
- 따라서 그 10행은 `build_source_map_materials.py`가 아니라 **다른 생성기**가 만든 행이다. 후보로 정본을 갈아치우면 **사라진다.**
- 10행 중 3행은 `BG_FAT_STONE_ROCK01/02/03` — 물이 아닌 바위 자산에도 water family가 붙어 있다. 사실로 기록만 한다(G06 소유, A단계에서 건드리지 않음).

### 5.4 남은 차단 188슬롯 (518 중 non-ok) — 이름과 이유

| 차단 이유 | 슬롯 |
|---|---|
| foliage vertex wind 바인딩 미검증 | 42 |
| 같은 자산의 다른 슬롯이 미컴파일(연쇄) | 33 |
| 미지원 terminal `bg_base_trn*` | 28 |
| null material slot | 23 |
| **설치 텍스처가 DDS 아님 (PNG)** | **21** ← 무대 전체 포함 |
| `specialresource.mat.ocean_trn` | 12 |
| 미처리 static switch | 11 |
| `'emissive_intensitymin'` | 10 |
| cooked WModel에 geometry 채널 없음 | 3 |
| `monster_base_opa` 2, `molding_trn` 2, invalid emissive bounds 1, `preset_flag_vertical_msk` 1, `sky_opa` 1 | 7 |

수정으로 새로 열린 슬롯들은 그다음 단계 이유로 넘어갔다(정상): `'emissive_intensitymin'` 6→10,
연쇄 27→33, DDS 아님 18→19(무대 2 포함 시 21).

### 5.5 후보 대 정본 비교 — 갈아치우면 안 된다

`out/MaharakaRebuild_A/compiled/after2.mapmaterials.json` (330행) 대 정본 (329행, `placementLighting` 231).

| 항목 | 값 |
|---|---|
| `(assetId, materialName)` 교집합 | 319 |
| 정본에만 있는 행 | **10 = 정확히 물 10행** (재현 불가) |
| 후보에만 있는 행 | 11 |
| 교집합에서 값이 동일한 행 | **177 / 319** |
| 값이 다른 행 | **142 = G04가 넣은 `bakedLighting`** |
| 후보의 `placementLighting` | **0** (정본 231) |

**단순 교체는 물 10행 + `bakedLighting` 142행 + `placementLighting` 231을 지운다.** C단계 병합은 행 단위여야 한다.

### 5.6 대표 재질 5종 검증 (조건 4 / 내 과제 4)

| 종류 | 대표 | 결과 |
|---|---|---|
| 텍스처 재질 | 일반 `bg_base_opa` 자산 252건 | ✅ 행 생성, 텍스처 364개 결합 |
| 벡터 색(무대) | `bg_ocn_etc_floor01a/b_mi_lnh` | ⚠️ **추출값은 G03 실측과 정확히 일치**하나 PNG normal 때문에 행 미생성 |
| 물 | `source.map.water-42.v1` 10행 | ❌ 컴파일러 재현 불가(`ocean_trn` 미지원). 다른 생성기 소유 |
| 지형 | `LAND01_LC_*` 16 | ❌ **mapmaterials 경로 자체가 아님** (baked PNG 경로) |
| foliage | 8 재질 | ⚠️ None 1→0 해결됐으나 vertex wind 42슬롯 미검증 |

**한 종류만 맞았다고 전체가 맞다고 하지 않는다.** 5종 중 완전 통과는 텍스처 재질 1종뿐이다.

무대 벡터 색은 추출 단계까지는 완성됐다(수정 후 MIC가 완전히 해석된다):
- `texture_diffuse` → `efmaster_material_prologue.tex.diffuse`, `texture_specular` → `.tex.spec`
- `texture_normal` → `bg_ocn_etc_g.tex.bg_ocn_etc_floor01a_n_lnh`
- A `diffuse_color [0.967422604560852, 0.533707857131958, 1.0, 1.0]` brightness 2.0
- B `diffuse_color [0.9407327175140381, 0.1516854166984558, 1.0, 1.0]` brightness 1.0
G03 실측과 자리수까지 일치한다. 막는 것은 색이 아니라 normal map 설치 형식이다.

## 6. 노랑 추적 (조건 6) — 가설 3개를 데이터로 갈랐다

코디네이터의 정정을 받는다. **원본에 노랑은 실재한다.** 내가 인용했던 "무대 두 재질뿐"은
SCENE03B 패키지만 본 **범위 오류**였고, 결론으로 쓸 수 없다.

### 가설 ①  중앙 노란 원반이 18조각과 별개 모델 — **확정**

무대 중심 `(75.064, -984.326)` 반경 25 m의 배치 590개를 좌표로 전수 훑었다.

반경 6.2 m 안쪽에서 쐐기와 **동일한 `y=22.40`, 동일한 `scale (0.95, 1.1875, 0.95)`**를 가진 별개 자산이 `d=0.02 m`에 있다:

```
MAP_1AF968D24AD5_BG_OCN_ETC_FLOOR01_SM_LNH_OVR_8EB1D0D03456
  패키지: LV_OCN_EVENTIS_MHP_SL01   (SCENE03B 아님)
  재질  : lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01_mi_lnh
  부모  : efmaster_material_prologue.mastermaterial_bg.base.bg_base_opa
```

이름이 `FLOOR01A`/`FLOOR01B`가 아니라 접미사 없는 **`FLOOR01`**이다. 그리고 18 쐐기는 `d=6.16 m`부터 시작한다 —
코디네이터가 말한 6.2 m 경계와 정확히 맞는다.

그 재질의 원본 값:

| 필드 | 값 | 판정 |
|---|---|---|
| `diffuse_color` | `[0.882022500038147, 0.8211935758590698, 0.07928290218114853, 1.0]` | **노랑** (R .88 G .82 B .079) |
| `diffuse_brightness` | 1.5 | |
| `reflection_color` | `[0.6253446936607361, 0.5924380421638489, 0.08690125495195389, 1.0]` | 노란 반사 |
| `reflection_intensity` | 0.20000000298023224 | |
| `texture_normal` | `bg_ocn_etc_g.tex.bg_ocn_etc_floor01_n_lnh` | **PNG 설치 → 차단** |

**원본 노랑의 출처가 특정됐다.** 임의로 칠한 색이 아니고 추출된 원본 값이다.
그리고 이 자산도 게시 정본 0행 / 내 후보 0행이며, 이유는 무대 쐐기와 **완전히 같은 PNG normal 차단**이다.
09-26 무대 쿠킹이 SCENE03B 2자산만 다뤄서 SL01에 있는 중앙 원반을 건드리지 않았다.

### 가설 ②  Matinee 재질 파라미터 track이 색을 바꾼다 — **부정**

`scene03b`의 material param 트랙을 전수 확인했다.

- material param 트랙을 가진 그룹: 9개, 그룹 이름은 **`post`, `post02`, `post03`, `rec`, `smile`, `water`** 6종뿐
- floor / tile / plan / piece / bfm 계열 그룹: **0개**
- 두 집합의 교집합: **공집합**
- `floorPieces` 18개 전부 `materials: []` — per-placement 재질 override 없음
- `interptrackvectormaterialparam`은 2개 있으나 둘 다 그룹 `rec`에 붙는다(무대 타일 아님)

즉 무대 타일의 색은 Matinee가 바꾸지 않는다. 정지 상태든 회전 상태든 MIC 정적 값이 색을 결정한다.

### 가설 ③  버전 차이 — **불필요**

①로 사진의 노랑이 설명된다. 버전 차이를 끌어올 근거가 없다. 미확인으로 남기지 않고 닫는다.

### 남는 추적 항목 (결론 내리지 않음)

- 쐐기 A/B는 분홍 `[0.967,0.534,1.0]` + 자홍 `[0.941,0.152,1.0]`인데 사진 확대분은 분홍과 **노랑**이 번갈아 보인다.
  중앙 원반이 노랑임은 확정됐지만, **쐐기 링 자체에 노란 쐐기가 있는지**는 아직 미확인이다.
  18 placement가 A 9개 / B 9개로 번갈아 배치되므로 A/B만으로는 분홍+자홍이다. 사진의 링 노랑이
  (a) 중앙 원반이 크게 보이는 것인지 (b) 제3의 자산인지 (c) 홍보 이미지 합성인지 아직 가리지 못했다.
- 다른 MIC나 per-placement override: `floorPieces`에는 없음을 확인. SL01 쪽 override 여부는 미확인.

## 7. 사용자 미확인 (내가 판정하지 않은 것)

- 화면 결과 일체. Client/UI를 실행·조작·캡처하지 않았다. `visual PASS`, `manual first pixel`, occurrence 승인 없음.
- 무대 색이 실제로 원본처럼 보이는지. 후보 행조차 아직 생성되지 않았으므로 판정 대상이 아니다.
- 사용자가 첨부한 사진 분석은 코디네이터가 전달한 관찰을 진단 입력으로 사용했을 뿐, 완료 증거로 승격하지 않았다.
- 빌드 미실행(devenv 열림). C++/셰이더 변경 없음이므로 이번 변경에 빌드가 필요하지도 않다.

## 8. source-exact 대 project-tuned 분리

| 구분 | 내용 |
|---|---|
| **source-exact** | `packageIndex 0` 해석, 상속된 부모 expression default 텍스처, 무대 A/B·중앙 원반의 `diffuse_color`/`brightness`/`reflection_*` 값, 네 경우 구분, 스칼라·벡터·switch 원본 값 전부 |
| **project-tuned** | 없음. 이번 A단계에서 값을 하나도 지어내지 않았다 |
| **도구 정책(원본 아님)** | 어댑터의 DDS-only 규칙, 컴파일러의 미지원 terminal 목록, foliage vertex wind 검증 요구 — 원본 부재의 증거가 아니라 현재 도구 지원 한계다 |

## 9. B단계 시작 가능 여부와 차단 요소

**부분적으로 즉시 시작 가능하다.** A단계가 만든 입력(252자산 / 330슬롯)은 유효하고 회귀도 전부 통과한다.

B단계를 막는 항목:

| 차단 | 성격 | 영향 |
|---|---|---|
| **PNG normal map 4종 → DDS 설치** | Resources/쿠킹. A단계 경계 밖(읽기 전용 준수) | 21슬롯, **무대 전체**. 가장 먼저 풀어야 한다 |
| foliage vertex wind 바인딩 | 도구 지원 | 42슬롯 |
| 미지원 terminal `bg_base_trn*`, `ocean_trn` | 도구 지원 | 40슬롯 |
| `'emissive_intensitymin'`, static switch | 도구 지원 | 21슬롯 |
| 지형 baked 경로 | 설계 확인 필요 | mapmaterials로는 해결 불가. 행 추가 금지 |
| 물 10행 생성기 | 소유자 확인 필요(G06) | 교체 시 소실 위험 |

기록만 하고 시작하지 않은 B단계 항목(코디네이터 지시):
- 조건 4: 승인된 배치와 정확한 source identity로 lightmap 목록 재계산. 244를 목표 수치로 쓰지 않는다. 기존 136이 잘못된 후보로 선택됐다는 것이 파일 자체가 틀렸다는 뜻은 아니다. 원본 RNM이 없는 오브젝트에 비율 맞추려고 조명을 붙이지 않는다. lighting variant가 `assetId`를 바꾸면 기존 배치의 stable ID·좌표·사용자 편집·모션 바인딩을 유지해야 한다.
- 조건 5: `LightMapTexture2D` 실제 직렬화와 압축 bulk 해석, 원본 mip 수/크기/포맷/내용 확인, `ShadowMap2D` 저장 포맷·UV·채널과 실제 셰이더 소비 경로, RNM과 실시간 조명의 이중 적용 방지.
- mip·`ShadowMap2D`는 **현재 도구 지원 부재**이며 원본 부재나 복원 불가의 증거가 **아니다**.

## 10. 보존 확인 (조건 8)

- `Data/**`, `Client/Bin/DataFiles/**`: **쓰지 않았다.** publisher 미실행.
- `Client/Bin/Resources/**`: **읽기만.** 삭제·수정·mirror·prune·부분 manifest install 없음.
- 팀장 정본(`Data/Rendering/**`, `Renderer.*`, `GameInstance.*`, `Client/Bin/ShaderFiles/**`, `UI_Sprite.cpp`): 건드리지 않음. 쿠크 Mario FXAA OFF 유지.
- 물 10행, 무대 18 배치, 기존 배치, 모션 바인딩: **전부 정본에 그대로.** 후보는 별도 폴더에만 있다.
- 어제 4개 수정 보존 확인. `git reset/checkout/stash/add`, commit/push/PR 없음. `rm -rf` 없음.
- 출력은 `out/MaharakaRebuild_A/` 와 `C:/LostArkExtract/RebuildA_scratch/` 아래에만.

## 11. 다음 항목

1. PNG normal map 4종을 DDS로 설치(Resources 소유자 작업) → 무대 21슬롯 해제 후 재컴파일. **무대 노랑·분홍이 나오는지는 그다음에 사용자 화면 확인.**
2. 쐐기 링 노랑의 정체 확정: SL01 반경 6.2~7 m 배치와 override 전수 확인.
3. C단계 병합은 행 단위로. 물 10행 + `bakedLighting` 142 + `placementLighting` 231 보존.
4. 지형은 baked 경로 설계 확인 후 별도 처리. mapmaterials 행 추가로 해결하려 하지 않는다.

---

## 최종 재확인

지시받은 3가지 정직 규칙에 따라 내가 실제로 한 것과 안 한 것을 다시 대조했다.

**하라는 대로 한 것**
- `packageIndex 0` 의미를 6개 근거 교차 확인으로 확정했다. 추측으로 정하지 않았다. cooker가 부모 텍스처를 설치했다는 사실 하나에 기대지 않았다.
- 네 경우를 구분하고 각각 다르게 처리했다. **0·false는 하나도 바뀌지 않았음을 수치로 증명했다**(0개 재질 변화, 99/58/7,901 동일). 이게 조건 2의 핵심이었고 지켰다.
- 회귀 6개 전부 exit 0(합계 100개), 변경 경로에 단위 테스트 6개를 새로 추가했다(7→13).
- 후보는 후보 폴더에만 만들었다. 정본에 쓰지 않았다. publisher·빌드 미실행.
- "분류됨"과 "렌더링 경로 확인됨"을 별도 열로 나눴고, 지형이라는 실제 반례를 찾아 보고했다.
- 노랑 가설 3개를 데이터로 갈랐다. 색을 임의로 만들지 않았다.

**못 한 것 (완료라고 쓰지 않는다)**
- **무대 행은 여전히 0개다.** 내 과제 4가 "무대 두 모델이 검증 기준"이라고 명시했으므로 **그 기준은 통과하지 못했다.** 추출 단계까지는 완성했고 원본 값이 G03 실측과 일치함을 확인했지만, PNG normal 때문에 mapmaterials 행이 생기지 않았다. 이걸 "벡터 색 경로 완성"으로 쓰지 않는다.
- 408 슬롯 전체 해결이 아니다. 252/382 자산, 330 슬롯이고 188 슬롯이 이름 있는 이유로 남아 있다.
- 대표 5종 중 완전 통과는 1종(텍스처 재질)뿐이다. 물·지형은 mapmaterials 경로 자체가 아니거나 재현 불가, foliage는 부분, 무대는 미생성.
- 쐐기 링의 노랑은 특정하지 못했다. 중앙 원반 노랑만 확정했다.
- 화면 판정은 하지 않았다. 사용자 몫이다.

**내 앞선 오류 정정**
- `9934688136d4_diffuse.dds`를 어댑터 합성 placeholder로 추정했으나 실측하니 64×64 DXT1 2,176바이트 실제 엔진 텍스처였다. 즉시 정정했다.
- 첫 어댑터 실행에 shadow gate **이전** component lighting을 써서 잘못된 기준선을 만들었다. `G04_round2`로 바꿔 어제 기준선(241/319)을 재현한 뒤 delta를 측정했다.
- "원본에 노랑 없음"은 SCENE03B만 본 범위 오류였다. 코디네이터 정정을 받아들이고 중앙 원반 노랑을 데이터로 찾았다.

**정직 규칙 대조 결과**: 이번 A단계는 `packageIndex 0` 차단군을 완전히 닫았고(22→0) 입력 계층을
241→252 자산으로 넓혔으나, **"408 자산의 모든 슬롯이 유효한 source 재질 정의를 갖게 만든다"는 A단계
목표는 달성하지 못했다.** 무대라는 명시된 검증 기준도 통과하지 못했다. 원인은 특정했고(PNG normal
4종, 21슬롯) 그 수정은 Resources 영역이라 이번 경계 밖이다. 부분 완료로 기록한다.
