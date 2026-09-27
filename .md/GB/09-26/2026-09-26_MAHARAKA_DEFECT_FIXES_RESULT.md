# 2026-09-26 마하라카 파이프라인 결함 수정 결과

오늘 확인된 결함만 수정했다. 새 기능·새 도구를 만들지 않았다.
정본 `Data/**`, 게시본 `Client/Bin/DataFiles/**`, `Client/Bin/Resources`에 한 바이트도 쓰지 않았고
publisher와 빌드를 한 번도 실행하지 않았다.

| 결함 | 결과 | 요약 |
|---|---|---|
| A. `renderMode Water` 생성 불가 | **고침** | 허용값에 `Water` 추가. 기존 Area 출력 변화 0 |
| B. adapter가 `source-absent`를 거짓으로 단정 | **부분 고침** | 측정에서 유도하도록 수정. 컴파일 출력은 byte 동일 |
| C. MIC 15종 미추출 | **안 고침** | 상수 확장은 틀린 수정. 올바른 설계만 기록 |

## 1. 결함 A — `renderMode Water` (고침)

### 확인한 원본

`Tools/LevelPlacementExtractor/build_maptool_scene.py`의 `render_profile_text()`가
`("Opaque", "Alpha", "Sky", "Additive")`만 허용했다. catalog의 renderMode 토큰은 이 함수 하나로만
생성되므로(`:712` 정규 자산, `:753` overlay 자산), catalog를 재생성하면 `Water` 행이 기본값
`Opaque`로 떨어진다.

`Water`가 정당한 값이라는 근거 네 가지를 실제 데이터에서 확인했다.

1. **publisher가 이미 받는다.** `Tools/MapPipeline/Publish-MapAuthoring.ps1:182`가 catalog의
   `tokens[11]`을 renderMode로 읽고 `:184`가
   `@('Opaque','Alpha','Sky','Additive','Water')`를 허용한다.
2. **현재 catalog에 실재한다.** `LV_OCN_EVENTIS_MHP`의 Imported·게시 catalog 양쪽에
   `Opaque 397 / Water 10 / Sky 1`이 있다.
3. **그 10행이 물 재질과 1:1로 이어진다.** catalog의 Water 10개 assetId 집합이
   `mapmaterials.json`에서 `family = source.map.water-42.v1`, `renderMode = translucent`인
   10행의 assetId 집합과 **정확히 일치**한다(차집합 양방향 0).
4. **그 물 행의 작성자가 확인된다.** `author_ocean_water_rows.py:60`이 water family 행을
   `renderMode='translucent'`로 만든다.

### 실제 구현

`build_maptool_scene.py` 한 곳, **+256 bytes**. 허용 tuple에 `"Water"`를 더하고 영문 주석으로
근거를 남겼다. 기존 네 값의 처리·기본값·cullMode·수치 검증은 한 글자도 바꾸지 않았고
알 수 없는 값은 계속 거부한다.

### 다른 Area 영향 — 없음

`Data/` 전체에서 `"renderMode": "Water"`를 가진 `renderprofiles.json`이 **0건**이다.
실측 분포는 `LV_BER_BERNCASTLE` = Sky 1 / Alpha 3, `LV_OCN_EVENTIS_MHP` = Sky 1 / 나머지 미지정,
전체 합계 `{Sky: 2, Alpha: 3, None: 306}`이다. 허용 집합을 넓힌 것은 순수 확대이므로
어떤 Area의 기존 생성 결과도 바뀔 수 없다.

### 검사

`test_build_maptool_scene.py`에 `RenderProfileTextTests` 5개 추가(**+1,907 bytes**).
Water 입출력, 기존 4종 불변, 기본값 `Opaque Back`, 알 수 없는 값 거부(`water`·`WATER`·
`Translucent`·`Masked`·빈 문자열), Water일 때도 cullMode·수치 검증이 계속 동작.
**31 → 36 tests OK.**

## 2. 결함 B — adapter의 거짓 `source-absent` (부분 고침)

### 확인한 원본

`build_source_map_material_inputs.py`가 모든 slot에 `lightingEvidence='source-absent'`를
하드코딩하고 `placementLighting=[]`를 썼다. module docstring도 그 사실을 적어 두었다.
그런데 shadow gate 수정 뒤 마하라카 원본은 RNM component **4,077개**를 가진다.
`source-absent`는 "원본에 baked lighting이 없다"는 **적극적 주장**이므로 이것은 데이터에
기록된 거짓이었다.

compiler 계약을 읽고 판단 근거를 세웠다.

- `build_source_map_materials.py:374`가 `lightingEvidence`를 필수로 요구하고
  `('source-bound','source-absent')` 둘만 허용한다. "모름"을 표현할 값이 없다.
- `:376`이 `source-absent`일 때만 바인딩 금지를 강제한다. 이 guard는 **근거 없는 부재 주장을
  막으려고** 있는 것이므로, 측정 없이 그 값을 넣으면 guard 자체가 무력해진다.
- 따라서 값을 **측정에서 유도**하고, 유도할 수 없으면 조용히 `source-absent`로 떨어뜨리지 않고
  해당 slot을 제외하며 이유를 남기는 쪽이 계약에 맞다.

join은 실재한다. component 레코드는 `sourcePlacementId`로 keying되고 `assetId`가 없지만
`sourceMesh.objectPath`를 가진다. asset inventory는 assetId마다 `fullPath`를 가진다.
한 mesh가 여러 `_OVR_` 자산을 뒷받침하므로 mesh → 자산은 1:N이다.

### 실제 구현

`build_source_map_material_inputs.py`, **+3,851 bytes**, 앵커 7곳.

- `--component-lighting`(nargs='+')을 **필수**로 추가했다. 측정 없이 이 manifest를 만들 수 없다.
- `derive_lighting_evidence()`를 추가했다. RNM component가 하나라도 있으면 `source-bound`,
  모든 component가 `NO_LOD_LIGHTING_DATA`면 `source-absent`, 그 외(미판정 status 포함,
  component 자체가 없음 포함)는 **unresolved**로 두고 해당 slot을 `skip`으로 빼며 이유를
  `slot_report.json`에 남긴다.
- manifest에 `lightingEvidenceSummary`를 추가해 측정 출처(logical package 목록)와
  bound/absent/unresolved 수를 기록한다.
- docstring과 `projectApproximations` 문구를 실제 동작으로 고쳤다.

**`Adapter.__init__`은 건드리지 않았다.** `author_ocean_water_rows.py`가
`adapter_module.Adapter(args)`를 자기 namespace로 직접 만들기 때문이다. 유도 로직은 전부
`main()` 안에 있고, import와 `Adapter` 시그니처가 그대로임을 확인했다.

### 하지 않은 것과 그 이유

`placementLighting`은 **여전히 빈 배열**이다. `:437`이 인스턴스마다 그 자산의 **모든** 행에
`bakedLighting` 텍스처 쌍을 요구하고, 그 텍스처는 `auxiliaryTextures`로 등록돼 있어야 한다.
즉 채우려면 텍스처 등록 경로까지 함께 만들어야 하고 그것은 수정이 아니라 기능이다.
`source-bound`는 "원본이 가지고 있다"는 진술이고 "이 빌드가 바인딩했다"는 진술이 아니며,
그 구분을 `projectApproximations`에 명시했다.

### 회귀 실측 — 출력 byte 동일

09-19 staging 입력으로 수정 전과 같은 인자에 `--component-lighting` 3개만 더해 재실행했다.

| 항목 | 수정 전 | 수정 후 |
|---|---|---|
| bound assets | 241 / 382 | 241 / 382 |
| slots | 319 | 319 |
| textures | 361 | 361 |
| placementLighting | 0 | 0 |
| `lightingEvidence` | `source-absent` 319 | **`source-bound` 315 / `source-absent` 4** |
| unresolved | — | **0** |

slot 집합이 동일하고(추가 0·제거 0), `lightingEvidence`를 뺀 slot 내용과 textures 배열이
완전히 같다. `lightingEvidenceSummary`는 inventory 382자산에 대해
`sourceBound 377 / sourceAbsent 5 / unresolved 0`이다.

그 입력으로 compiler를 돌린 결과가 **수정 전 컴파일 출력과 sha256 동일**
(`ed15293b3c44a9c5`, 592,425 bytes, materials 319). compiler가 `lightingEvidence`를 출력 행에
복사하지 않기 때문이며, 게시 Bern 행에도 그 필드가 없는 것과 일치한다.

### 쿠크 영향 — 없음

쿠크 `build.receipt.json`의 `sourceMaterialBuild`도 `null`이고, 쿠크 게시 mapmaterials는
materials 1,734 / baked 1,355 / placementLighting 2,368이다. 이 adapter는 baked와
placementLighting을 0개 만들므로 **쿠크 행을 만든 주체가 아니다.** 따라서 이 수정이 쿠크
게시본을 바꿀 수 없다.

### 검사

새 파일 `test_build_source_map_material_inputs.py` **8 tests OK**.
RNM → bound, NO_LOD만 → absent, RNM이 NO_LOD 형제를 이김, 미판정 status → unresolved,
component 없음 → unresolved, mesh join 대소문자 무시와 `_OVR_` 다중 자산, `fullPath` 없는 자산은
주장하지 않음, summary가 측정 출처를 기록.

## 3. 결함 C — MIC 15종 (안 고침)

`extract_ue3_material_graph.py:34`의 `EFFECT_MATERIAL_CLASSES`에
`materialinstanceconstant`를 더하는 단순 수정은 **틀린 수정**이다.

`:129`의 class 검사를 통과시켜도 바로 다음 `:139~142`가 `Expressions` 배열을 필수로 요구한다.
MIC는 expression graph를 직렬화하지 않고 `Parent` 참조와 파라미터 override만 가지므로,
상수만 늘리면 "Material Expressions array is missing"이라는 **원인과 다른 오류**로 실패하거나,
운 나쁘게 배열이 잡히면 잘못된 그래프를 조용히 만든다.

올바른 설계는 이미 저장소에 있다. `Tools/EffectPipeline/build_effect_child_parent_resolution.py`가
docstring `:17~22`에 "선언 package를 열어 MIC의 `Parent`를 읽고 MIC → MIC로 walk한다"고
적힌 해석기를 구현한다. 필요한 것은 그 walk를 재사용하는 **범용 entry point**이며,
현재 `main()`은 `--authored` corpus와 전용 contract schema에 묶여 있다(`:1394~1406`).
이것은 한 줄 수정이 아니라 설계 검토가 필요한 변경이라 이번 범위에서 구현하지 않았다.

## 4. 타 세션과 조정이 필요한 항목

- **`build_source_map_material_inputs.py`는 git 미추적(`??`) 신규 파일이며 다른 세션의 작업일 수
  있다.** 이번에 수정했다(sha256 `f7dd9bc2635ce0c8…` → `4c17179ba5d80506…`).
  백업은 `out/MaharakaContinuation_20260926_193455/DefectFixes/backup/`에 있다.
  **`--component-jsonlighting`이 아니라 `--component-lighting`이 필수 인자로 늘었으므로
  기존 호출 스크립트가 있으면 인자를 추가해야 한다.** 저장소 안의 유일한 Python 소비자인
  `author_ocean_water_rows.py`는 `Adapter`를 직접 만들어 쓰므로 영향이 없음을 확인했다.
- `build_maptool_scene.py`의 `Water` 허용은 쿠크·베른을 포함한 모든 Area의 catalog 재생성에
  영향을 주는 공용 계약이다. 현재 입력에 Water가 없어 출력은 바뀌지 않지만, 팀 검토 대상이다.
- 결함 C의 범용 MIC entry point는 `Tools/EffectPipeline/` 소유자와 조정이 필요하다.

## 5. 수정한 파일

| 파일 | 변화 | 줄바꿈·인코딩 |
|---|---|---|
| `Tools/LevelPlacementExtractor/build_maptool_scene.py` | 61,716 → 61,972 (+256) | LF 유지, 비ASCII 0, BOM 없음 |
| `Tools/LevelPlacementExtractor/test_build_maptool_scene.py` | 40,788 → 42,695 (+1,907) | LF 유지, 비ASCII 0 |
| `Tools/LevelPlacementExtractor/build_source_map_material_inputs.py` | 16,993 → 20,844 (+3,851) | LF 유지, 비ASCII 0, BOM 없음 |
| `Tools/LevelPlacementExtractor/test_build_source_map_material_inputs.py` | 신규 | LF, ASCII |

새 코드 줄에 줄 끝 한글 주석 없음. `.vcxproj`/`.filters` 미수정.

## 6. 실행한 검사

| 검사 | 수정 전 | 수정 후 |
|---|---|---|
| `test_build_maptool_scene` | 31 OK | **36 OK** |
| `test_build_map_material_variants` | 20 OK | 20 OK |
| `test_extract_source_map_component_lighting` | 10 OK | 10 OK |
| `test_build_source_map_materials` | 13 OK | 13 OK |
| `test_extract_ue3_material_graph` | 5 OK | 5 OK |
| `test_build_source_map_material_inputs` | 없음 | **8 OK** |
| `test_merge_maptool_landscape` | — | 13 OK |
| `test_build_bern_castle_shards` | — | 12 OK |
| `test_extract_ue3_placements` | — | 13 OK |

adapter 재실행 exit 0, compiler 재실행 exit 0, 컴파일 출력 sha256 동일.

## 7. live 설치 / 게시 / 빌드 — 전부 없음

`Client/Bin/Resources` 미변경(읽기만, install/sync 경로 미사용).
`Data/**`·`Client/Bin/DataFiles/**` 미변경. publisher `-Mode Publish` 0회. 빌드 0회.
`git status`의 다른 항목과 `Client/Bin/ShaderFiles`·`Engine/**` 변경은 mtime 09-26 17:28:03,
`DataFiles/Navigation`은 11:33:37로 이 작업(00:17~00:23) 이전의 병행 작업 것이다.

## 8. 사용자 수동확인

**없다.** 이번 수정은 도구 계약과 manifest 내용만 바꿨고 게시본·Resources·실행 파일을
건드리지 않았으므로 화면에 나타나는 변화가 없다.

## 9. 미완료

- 결함 C 범용 MIC entry point 미구현.
- adapter는 여전히 `placementLighting`과 `bakedLighting`을 만들지 않는다(기능 범위).
- 조명 행 반영의 선결 조건인 `_RNM_` variant catalog admission은 이번 범위 밖이다.
- `renderprofiles.json`에 물 10행의 `renderMode: "Water"`를 실제로 기록하는 것은 데이터 작업이며
  하지 않았다. generator는 이제 그 값을 낼 수 있지만, 입력에 값이 없으면 여전히 재생성 시
  `Opaque`가 된다. **결함 A 수정만으로 재생성 안전이 보장되지 않는다.**
