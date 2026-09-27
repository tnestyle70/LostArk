# 2026-09-27 마하라카 G04 — RNM 조명을 올바른 배치·재질에 연결

대상: `LV_OCN_EVENTIS_MHP`. 실행 저장소 `C:\Users\USER\source\졸업팀폴\LostArk`,
branch `codex/main-ship-maharaka-0926`, HEAD `a84bbcd4`.
설계 근거: `.codex/worktrees/7395/LostArk/.md/GB/09-27/2026-09-27_MAHARAKA_VISUAL_PARITY_EXECUTION_PLAN.md` G04.

## 관찰

시작 감사(읽기 전용): catalog 408 / placement 4,669 / material 329 /
bakedLighting 0 / placementLighting 0 / meshNamesWithMultipleVariants 49.
기존 후보 `lighting-payload-candidate.json`은 placementLighting 3,786행 중
base asset 불일치 955, 게시 배치에 없는 sourcePlacementId 35.

## 확인한 원인

### 1. 후보 join이 mesh leaf name이었다 (실제 버그)

`out/MaharakaContinuation_20260926_193455/MaterialPipeline/build_lighting_payload.py`의
`load_catalog_assets()` 47행이 `by_source[fields[1].casefold()] = fields[0]` — 즉
**source mesh 이름 → assetId를 평범한 dict 대입**으로 만든다. 이 Area에는 한 mesh 이름이
여러 `_OVR_` 자산을 뒷받침하는 경우가 **49건** 있어 마지막 catalog 행이 조용히 이긴다.
그 뒤 86~88행이 component의 `sourceMesh.objectPath` leaf name으로 그 표를 조회한다.

실측 예: `LV_OCN_EVENTIS_MHP_SL01:export:5276`은 게시본에서
`..._HOUSEINFLOOR02_SM_MSJ_OVR_7B83806A61B6`를 쓰는데 후보는 `..._OVR_FD1426E105BC`를 골랐다.

### 2. publisher가 같은 키를 이미 강제한다 (결정적)

`Tools/MapPipeline/Publish-MapAuthoring.ps1`:

- `:3392~3395` — `$sourcePlacements`를 **Authoring placements**에서 `SourcePlacementId → AssetId`로 만든다.
- `:3415` — `-not $bakedAssets.Contains($lighting.assetId)`
- `:3417~3418` — `-not $sourcePlacements.ContainsKey(...)` 또는
  `$sourcePlacements[$lighting.sourcePlacementId] -cne $lighting.assetId` 이면 throw.

즉 **`placementLighting.assetId`는 그 배치의 게시 assetId와 정확히 같아야 한다.**
따라서 후보의 955 불일치는 게시 단계에서 거부됐을 것이고, 더 중요하게
**`_RNM_<hex>` variant assetId는 배치 자체가 그 assetId를 쓸 때만 합법이다.**
"배치는 그대로 두고 조명만 variant로 붙인다"는 구조적으로 불가능하다.

### 3. 베른이 실제로 배치를 variant로 쓴다 (수치 확인)

`Data/Maps/Imported/LV_BER_BERNCASTLE/*.mapplacements` 전수:

| shard | 배치 | `_RNM_` assetId 사용 |
|---|---|---|
| RNM00~RNM08 (9개) | 49,397 | **49,397** |
| BASE/LANDSCAPE/SL00~SL10 등 | 620 | 0 |
| 합계 | **50,017** | **49,397 (98.8%)** |

고유 `_RNM_` assetId **15,483개**. 베른의 RNM은 **Area 전체 re-shard**이며
조명만 얹은 추가 레이어가 아니다. 마하라카에 같은 방식을 적용하려면
catalog·placements·materials를 한 세트로 교체해야 한다(규모는 아래).

### 4. 정식 adapter는 같은 버그가 아니다

`Tools/LevelPlacementExtractor/build_source_map_material_inputs.py`의 mesh 기반 조회는
`derive_lighting_evidence()`(`:211~216`)에서 **per-asset evidence 집계**에만 쓰이고
주석이 "one mesh can back several _OVR_ assets"를 이미 명시한다. 그리고 `:348`이
"this adapter still emits no bakedLighting pair and no placementLighting instances"라고
스스로 기록한다. 즉 per-instance 바인딩을 하지 않으므로 이 join 버그가 없다. **수정하지 않았다.**

### 5. mipEvidence는 판정 불가

`extract_ue3_texture_mips.py`는 class `lightmaptexture2d`를 **거부**한다
(`unsupported source class lightmaptexture2d`). 원본 bulk data는 압축돼 있어
serial 크기로 chain을 알 수 없다(serialBytes/mip0payload 비율 중앙값 0.637, 101/136이 1.0 미만).
따라서 **native mipCount를 확정하지 못했다.** `source-chain`도 `source-unmipped`도 주장하지 않는다.
이번 authoring 경로는 `mipEvidence`를 기록하지 않으므로 거짓 표기가 들어가지 않았다.
(어제 install manifest의 `source-unmipped` 표기는 근거 부족이며 이 문서가 그것을 정정한다.)

## 수정 파일

| 파일 | 변경 |
|---|---|
| `out/MaharakaVisualParity_G04/build_rnm_payload.py` | 신규. sourcePlacementId 정확 조인 builder/applier |
| `out/MaharakaVisualParity_G04/test_rnm_payload.py` | 신규. 10 tests |
| `out/MaharakaContinuation_20260926_193455/MaterialPipeline/build_lighting_payload.py` | docstring에 SUPERSEDED 경고 +1,176 bytes (앵커 1회, LF·구문검사 OK) |
| `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapmaterials.json` | 142행에 `bakedLighting` 추가, `placementLighting` 0 → 231 |
| `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.*` | publisher가 재생성 |

기존 후보 JSON(`lighting-payload-candidate.json`)은 **증거로만 보존**하고 게시하지 않았다.

## 게시 가능 범위와 제외 (전부 수치)

RNM component 4,077 → sourcePlacementId로 게시 배치와 조인 **3,751** / 미게시 **326**.
RNM을 가진 게시 자산 **374**.

| 분류 | 자산 | 인스턴스 | 이유 |
|---|---|---|---|
| **게시함** | **99** | **231** | 단일 atlas pair + 기존 material 행 있음 + 비물 + 텍스처 설치됨 |
| 제외 needs-placement-reshard | 141 | 2,322 | atlas pair가 여러 개 → 배치 assetId 교체 필요(`:3417`) |
| 제외 no-material-row | 130 | 1,194 | 기존 material 행이 없어 값을 지어내지 않으면 생성 불가 |
| 제외 water-owned-by-g06 | 4 | — | `source.map.water-42.v1`. program 42는 baked를 **지원하지만**(`:2962~2964`) 물 검증은 G06 소유 |

게시한 material 행 **142**개 family: `bg-source-opaque-masked` 136, `bg_base_opa_overlay` 6.
둘 다 `bakedLighting`을 optional field로 허용한다(`:3046`, `:3060`, 공통 검증 `:3258~3267`).

re-shard까지 간다면 추가로 필요한 variant 행은 **709개**이고, 그건 placements·catalog·materials
동시 교체다. 이번 범위에서 하지 않았다.

`955 / 35`는 **후보 파일의 수치이며 해소 방식은 그 후보를 쓰지 않는 것**이다.
새 경로는 정확 조인이므로 불일치가 구조적으로 발생하지 않고(publisher가 같은 키로 재검증),
검증 위반 **0**으로 통과했다. 감사 스크립트가 여전히 955/35를 보고하는 것은 superseded
후보 JSON을 읽기 때문이며, 게시본과 무관하다.

## 실제 consumer

`Data/Maps/Authoring/.../mapmaterials.json`
→ `Publish-MapAuthoring.ps1` (`Read-MapMaterialDocument` `:2732`, placementLighting `:3398~3420`)
→ `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapmaterials.json`
→ 런타임 map material/RNM 소비 경로.

`bakedLighting` 텍스처 284개 참조가 전부 `Client/Bin/Resources` 실제 파일로 resolve된다(미해결 0).

## 자동 검사 결과

| 검사 | 결과 |
|---|---|
| `build_rnm_payload.py --mode report` 내부 검증 | 위반 **0** |
| `test_rnm_payload.py` (신규) | **10 OK** |
| `test_build_map_material_variants` | 20 OK, exit 0 |
| `test_extract_source_map_component_lighting` | 10 OK, exit 0 |
| `test_build_source_map_materials` | 13 OK, exit 0 |
| `test_build_maptool_scene` | 36 OK, exit 0 |
| `test_build_source_map_material_inputs` | 8 OK, exit 0 |
| Map Validate / Publish / Check | 전부 **exit 0**, PlacementCount 4,669 |

게시 전후 보존 대조(백업 대비):

- material 행 **329 → 329**, 키 삭제 0 / 추가 0
- 완전 동일 **187**행, `bakedLighting`만 추가 **142**행, **그 외 변경 0**
- 물 행 **10개 전/후 완전 동일**, 그중 `bakedLighting` 보유 **0**
- `placementLighting` **0 → 231**
- 게시 자산 **408**, 배치 **4,669**, renderMode `deferred` 319 / `translucent` 10
- JSON 직렬화 왕복이 원본과 **바이트 동일**함을 먼저 확인한 뒤에만 기록(LF·무BOM·2-space 유지)
- Resources 무변경: `Lighting/Maharaka` **137**개, LUT 16,512 bytes, Area 자산 **384** / 파일 **6,323** / wmodel **384**
- 금지 경로 diff 없음: `Data/Rendering`, `Client/Bin/DataFiles/Rendering`, `Renderer.*`,
  `GameInstance.*`, `UI_Sprite.cpp`, `.vcxproj`/`.filters`
- 빌드 **0회** (devenv pid 26212 실행 중, C++ 무변경)

동시 편집 보호: 적용 직전 문서를 다시 읽어 sha256이 분석 시점과 같은지 확인하고 다르면 중단하도록
구현했다. 이번 실행에서는 동일했다(`8ad7ecdb3eb926c1`).

## 사용자 미확인

**화면 판정은 하지 않았다.** 지금 반영된 것은 게시 자산 374개 중 99개에 붙은
원본 RNM 조명이며 RNM 인스턴스 기준 **231 / 3,751 = 6.2%**다. 나머지 93.8%는 위 표의
이유로 아직 붙지 않았으므로 **화면 전체가 원본처럼 밝아지지는 않는다.**
어느 오브젝트가 밝아졌는지는 사용자가 확인해야 한다. `originalVisualFidelityVerified`는
건드리지 않았다.

## 다음 항목

1. **no-material-row 130자산 / 1,194 인스턴스** — G03이 무대 MIC로 여는 경로와 같은 문제다.
   실제 슬롯과 effective source parameter를 material 입력에 넣어야 행이 생긴다.
2. **needs-placement-reshard 141자산 / 2,322 인스턴스 / variant 709행** — 베른식 RNM shard가
   필요하다. `build_maptool_scene.py`가 `_RNM_` 배치/카탈로그를 낼 수 있어야 하고
   catalog·placements·materials를 한 transaction으로 교체해야 한다. 규모가 커서 별도 승인 대상.
3. **미게시 배치 326 RNM component** — helper / 비게시 / 다른 scope 분류가 남았다.
   목록은 `rnm-join-analysis.json`의 `unpublishedPlacements`(앞 50개)에 있다.
4. **native mip chain** — `extract_ue3_texture_mips.py`가 `lightmaptexture2d`를 지원해야
   `source-chain` 여부를 판정할 수 있다. 그전에는 어떤 mipEvidence도 주장하지 않는다.
5. **설치된 lightmap 136개 / 필요 244개** — 108개가 아직 미설치다. 이전 설치가 잘못된 후보를
   따랐기 때문이며, re-shard 단계에서 필요한 것만 추가 설치해야 한다.
6. **ShadowMap2D** — `shadowPayloadDecoded=false` 경계가 그대로다. static shadow를 RNM과
   중복 가산하지 않도록 G07의 광원 분류와 함께 판단해야 한다.
7. 범위 밖 관찰 한 줄: `Data/Maps/MapCatalog.json`의 `placementCount 4651` / `assetCount 406`이
   현재 4,669 / 408과 다르다. mtime 09-26 11:26으로 **내 작업 이전**부터 그렇다. 내가 바꾸지 않았다.

산출물: `out/MaharakaVisualParity_G04/`
(`build_rnm_payload.py`, `test_rnm_payload.py`, `rnm-join-analysis.json`, `backup/`, `mipprobe/`)
