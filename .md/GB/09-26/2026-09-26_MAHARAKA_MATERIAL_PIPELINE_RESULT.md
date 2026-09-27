# 2026-09-26 마하라카 재질·조명 파이프라인 가운데 단

추출기가 RNM 4,077개를 주는데 manifest adapter가 버리는 문제, 그리고 Effect 재질 15종이 MIC라서
추출되지 않는 문제를 다뤘다. **제품 파일은 하나도 바꾸지 않았다.** 도구도 한 줄 고치지 않았다.

## 0. 한 줄 결론

`_RNM_` variant는 **지오메트리 폴더가 아니라 catalog 행**이다. 베른의 15,483 variant가 전부 base
mesh의 wmodel을 공유한다. 따라서 베른의 hex 해시를 재현할 필요가 없고, 마하라카는 variant
**883개**만 있으면 된다. compiler가 요구하는 전체 조명 payload를 만들어 **교차검사 위반 0건**으로
검증했다. 남은 관문은 lightmap DDS **136개를 Resources에 설치**하는 것 하나다.

## 1. 실제 바꾼 파일

없음. 모든 산출물은 `out/MaharakaContinuation_20260926_193455/MaterialPipeline/` 아래에만 있다.

| 파일 | 성격 |
|---|---|
| `verify_uv1.py` / `uv1-verification.json` | 1단계 UV1 검증 |
| `compare_wmodel_uv1.py` / `wmodel-uv1-comparison.json` | Area별 WModel 차이 판별 |
| `derive_rnm_hex.py` / `rnm-hex-derivation.json` | 3단계 베른 hex 역산 시도 |
| `measure_variant_need.py` / `variant-need.json` | 마하라카 variant 필요량 실측 |
| `build_lighting_payload.py` / `lighting-payload-candidate.json` (2.1 MB) | 3·4·5단계 payload 후보 |
| `lightmap-dds/*.dds` 2개 | 2단계 추출 결과 |

`Tools/**` 무변경(`git status`로 확인, 보이는 `M`은 전부 병행 작업 것). adapter
`build_source_map_material_inputs.py`는 **읽기만** 했고 sha256 `f7dd9bc2635ce0c8` 그대로다.
`build_map_material_variants.py`는 지시대로 손대지 않았다. `Data/**`, `Client/Bin/DataFiles/**`,
`Client/Bin/Resources/**` 무변경. 빌드 0회, publisher 0회.

UModel은 저장소의 `졸업팀폴` 한글 경로를 깨뜨리므로 ASCII 스크래치
`C:/LostArkExtract/MhpLightmapProbe_20260926/`에서 실행하고 결과만 출력 폴더로 복사했다.

## 2. 1단계 UV1 ≠ UV0 — **완료**

대표 자산 `MAP_640F6740C1FA_BG_LUT_LUCASTLE_HOUSEINBORDER01_SM_PHS`의 설치 glTF에
`TEXCOORD_1`이 실재하고, UV0의 복제가 **아니다.**

| 항목 | 값 |
|---|---|
| 정점 | 76 |
| UV0와 완전히 같은 정점 | **0** |
| 최대 \|dU\| / \|dV\| | 1.000732 / 0.837887 |
| UV0 범위 | u[−0.1077, 1.5566] v[0.0038, 0.9995] — **단위정사각형 밖** (타일링 UV) |
| UV1 범위 | u[0.0020, 0.9990] v[0.0024, 0.9976] — **단위정사각형 안** (atlas UV) |
| 고유값 | UV0 52 / UV1 72 |

UV0는 타일링이라 [0,1]을 벗어나고 UV1은 atlas에 패킹돼 [0,1] 안에 있다. lightmap 언랩이 봉합선을
더 쪼개므로 고유값도 UV1이 많다. 교과서적 구분이다. 게이트 통과.

`geometry.receipt.json`도 `hasTexcoord1 = True`, `formatVersion 1.2`, `vertexStride 56`로 일치한다.

### 2.1 정정 — "lightmap UV1은 Area별"이 아니다

같은 mesh가 베른과 마하라카 양쪽에 설치돼 있고 WModel이 둘 다 14,720 bytes인데 508 bytes
다르다. 이 차이를 **UV1이라고 본 것은 틀렸다.** 실측하면 UV0 76쌍·UV1 76쌍이 **양쪽 바이너리에
전부 그대로 들어 있다.**

508 bytes의 정체는 두 가지다.

1. **tangent W 부호.** 차이가 offset 323부터 **56바이트(= vertexStride) 간격으로 반복**되고,
   값이 베른 `0x3F800000`(+1.0) 대 마하라카 `0xBF800000`(−1.0)이다. receipt의
   `tangentHandednessTransform = runtimeW=-sourceW_after_Z_reflection`과 맞는 cook 세대 차이다.
2. offset 248~255의 8바이트 해시.

즉 **UV1은 Area 공용**이고, variant가 구분하는 것은 지오메트리가 아니다. 이 정정이 3단계 결론의
근거가 된다.

## 3. 2단계 lightmap DDS — **완료(대표 1쌍)**

`extract_ue3_texture_mips.py`의 `--expected-mip0`는 **바이트 수가 아니라 파일 경로**다
(`expected_mip0.is_file()`, `read_bytes()`). 기존 mip0 DDS를 기준으로 width/height/fourCC를 잡고
native mip 레코드를 회전시켜 전체 체인을 복구하며, mip0 압축 바이트가 기준과 다르면 거부하는
**검증 도구**다. 따라서 새 텍스처는 **먼저 UModel로 mip0을 뽑아야** 이 도구를 쓸 수 있다.

1단계 UModel 직접 export 결과:

| 텍스처 | 치수 | 포맷 | 크기 | serialBytes |
|---|---|---|---|---|
| `normalizedaveragecolor0_58` | 1024×512 | DXT1 | 262,272 | 82,593 |
| `directionalmaxcomponent0_58` | 1024×512 | DXT1 | 262,272 | 135,993 |

DXT1 1024×512 = (1024/4)×(512/4)×8 = **262,144** + 128 헤더 = 262,272으로 정확히 맞는다.
그리고 component의 `coordinateScale [0.029296875, 0.05859375] × [1024, 512] = [30, 30]` 텍셀 —
정사각 30×30 lightmap 차트로 일관된다. `serialBytes`는 property stream + 전체 mip을 포함한
export 크기라 mip0과 직접 대응하지 않는다.

**남은 것:** 나머지 134개 텍스처 추출과, `extract_ue3_texture_mips.py`로 mip 체인까지 복구하는
2단계. 대표 1쌍은 mip0만 확보했고 mip 체인은 아직이다.

## 4. 3단계 `_RNM_<hex>` 규칙 — **재현 불가이며 재현할 필요 없음**

### 4.1 베른 해시는 게시본에서 역산되지 않는다

베른 게시본 `_RNM_` 행 21,364개, 고유 suffix 15,305개. 패턴은 `MAP_<12hex>_<name>_RNM_<12hex>`로
21,364/21,364 일치하고 hex 길이는 전부 12다.

suffix 그룹 안에서 값이 하나뿐인 필드를 세면:

| 필드 | 일정한 그룹 |
|---|---|
| `base`, `name` | **15,305 / 15,305** |
| `average`, `directional`, `colorSpace` | 15,277 / 15,305 |
| `shadowTexture`, `lightGuid`, `lightChannel` | 15,280 / 15,305 |

mesh 정체는 완전히 일정한데 조명 필드가 28그룹에서 변한다. 즉 해시 입력에 **게시본에 없는 값**이
섞여 있다. 09-11 문서가 "material signature/atlas/원본 component color로 구분"이라 적은 대로다.
payload 12종 × 알고리즘 5종(sha256/sha1/md5/blake2b/crc) × slice 5종을 표본 400행에 돌렸으나
**재현 조합 0건**이다. 생성 스크립트도 사라졌으므로 **역산 불가**로 기록한다.

### 4.2 그런데 재현할 필요가 없다

베른 `.mapassets`는 23개 shard로 나뉘고 `RNM00~08` 전용 shard를 포함해 16,905 자산을 admit한다.
그중 `_RNM_`이 **15,483개**(09-11 문서의 variant 수와 정확히 일치). 그 행들의 wmodel 경로를 읽으면:

```
assetId : MAP_006414294549_..._SM_KSS_RNM_149D482C9D2A
wmodel  : Map/LV_BER_BERNCASTLE/MAP_006414294549_..._SM_KSS/MAP_006414294549_..._SM_KSS.wmodel
assetId : MAP_006414294549_..._SM_KSS_RNM_333FE22E9D6B
wmodel  : (같은 base 경로)
assetId : MAP_006414294549_..._SM_KSS_RNM_837BA098187E
wmodel  : (같은 base 경로)
```

**15,483/15,483 전부 base 폴더의 wmodel을 가리킨다.** 베른 catalog 행 19,110개가 정상 해석되고
미해석은 42개(Landscape 경로 형태가 다른 별건)뿐이다.

따라서 `_RNM_` suffix는 **하나의 mesh가 여러 조명 바인딩을 갖게 하는 catalog 행 구분자**일 뿐이고,
그 hex 값 자체에는 런타임 의미가 없다. 결정적이고 충돌만 없으면 된다.

> **정정:** 조사 중 "베른 variant 15,483개가 하나도 설치돼 있지 않다"고 본 중간 수치가 있었다.
> assetId를 폴더명과 직접 비교한 측정 오류였다. wmodel 경로를 따라가면 전부 해석된다.
> 베른 Resources는 온전하다.

### 4.3 compiler 계약이 이를 뒷받침한다

`build_source_map_materials.py`는 material 행의 `bakedLighting`에 **`{averageTexture,
directionalTexture, colorSpace}` 3필드만** 허용한다(`:183`). 인스턴스별 atlas 배치는
`placementLighting`이 `(sourcePlacementId, assetId)` 키로 따로 갖고(`:419`), component와
`coordinateScale`/`coordinateBias`/`averageScale`/`directionalScale` 일치를 강제한다(`:430~434`).
**assetId에 `_RNM_`을 요구하는 검사는 없다.**

한 행이 텍스처 쌍 하나만 담으므로, **같은 mesh가 두 개 이상의 atlas 쌍에 걸릴 때만** 추가 행이
필요하다. 그것이 variant의 진짜 정의다.

(베른 게시본 행에는 `staticShadow`가 더 붙어 있는데 현재 compiler는 그 필드를 허용하지 않는다.
베른 행이 이 compiler 산출물이 아니라는 2차 결론과 일치한다.)

### 4.4 채택한 규칙

```
assetId = baseAssetId                                   (base가 atlas 쌍 1개만 쓸 때)
assetId = baseAssetId + "_RNM_" + suffix                (쌍이 2개 이상일 때)
suffix  = upper(sha256("<average소스객체>|<directional소스객체>" casefold)[:12])
```

마하라카 실데이터 검증: **충돌 0개**, compiler의 assetId 문자 제약(`[A-Za-z0-9_.:-]+`) 통과.

## 5. 4·5단계 compiler 입력 payload — **후보 완성, 설치 게이트 남음**

adapter 원본을 고치지 않고 **독립 후보 생성기**로 구현했다. adapter는 미추적 타 세션 파일이고,
필요한 산출물이 무엇인지가 먼저 확정되어야 최소 변경을 제안할 수 있기 때문이다.

### 5.1 실측 규모

| 항목 | 값 |
|---|---|
| RNM component | 4,077 |
| 고유 source mesh | 303 |
| 고유 atlas 텍스처 쌍 | 122 |
| 고유 lightmap 텍스처 | 244 |
| 쌍이 2개 이상인 mesh | 195 |
| **material 행** | **990** (그중 `_RNM_` variant **883**) |
| **placementLighting** | **3,786** |
| **auxiliaryTextures** | **136** |

mesh 하나가 쓰는 atlas 쌍 수는 1개(108종)에서 47개(1종)까지 분포한다.

베른이 15,483 variant인데 마하라카는 883개다. **5.7%** 규모다.

### 5.2 교차검사 결과

compiler가 파일 없이 할 수 있는 검사를 그대로 재실행했다 — 중복 `(placementId, assetId)`,
4필드 길이, component status·`nativeTailCompletelyConsumed`, 네 필드의 component 값 일치,
유한성, `assetId`의 binding 존재. **위반 0건.**

`unmatchedMeshes` 5종은 catalog에 base 자산이 없어 제외했다:
`bg_rhd_stone_rock01_sm_ksr`, `bg_tot_movillage_decoprop07f_sm_artree`,
`lv_common_mesh_cul_box_1/4/8`. 섬 범위 규칙에서 빠진 바위, 법선 퇴화로 제외된 장식,
컬링 헬퍼 박스다. 기존 admission 기록과 일관된다. 이 때문에 component 4,077개 중 291개가
payload에 들어가지 않았다.

### 5.3 남은 게이트

`check_texture`가 `auxiliaryTextures`의 각 DDS를 **resources_root에서 실제로 읽는다** — 치수,
mip 체인 바이트 길이, `mipCount` 일치, `colorSpace`, `mipEvidence`를 전부 검사한다.
현재 `Map/Lighting/Maharaka/`에는 lightmap이 **0개**다(있는 DDS 1개는 lightmap이 아닌 LUT).

따라서 **136개 DDS를 `Client/Bin/Resources/Map/Lighting/Maharaka/`에 설치해야** compiler가
돈다. 1024×512 DXT1 기준 개당 262,272 bytes = 약 **35 MB**. 이 설치는 팀장 Drive 관리 폴더에
쓰는 일이라 이번 범위에서 하지 않았다.

`mipEvidence`는 후보에 `source-chain`으로 적었는데, 이는 **mip 체인까지 복구했을 때만 정당하다.**
mip0만 뽑은 상태로 설치하면 `source-unmipped`여야 하고 그때는 `mipCount == 1`이어야 한다.
3절의 mip 체인 복구가 이 필드의 선행 조건이다.

## 6. 6단계 MIC 15종 — **원인·경로 확정, 구현 미착수**

### 6.1 단순 확장은 틀린 수정이다

대상 패키지를 UModel `-list`로 열어 클래스를 직접 확인했다. `fx_m_mi_00`의 객체들은
**`materialinstanceconstant`**다(`fx_a_me_et_01_1_ad`, `fx_d_pa_atta_11_06_ad` 등). 실제 MIC
경로는 `fx_m_mi_00.fx_mi.fx_a_pa_gl_01_2_ad` 형태로 G06 `dependencies.json`에 남아 있다.

`extract_ue3_material_graph.py:34`의 `EFFECT_MATERIAL_CLASSES = {"material","decalmaterial"}`에
`materialinstanceconstant`를 **더하면 안 된다.** MIC는 expression graph를 갖지 않는다 —
`Parent` 객체 참조와 파라미터 override만 직렬화한다. root `Material`로 가정해 파싱하면
빈 그래프나 잘못된 그래프를 **조용히** 만들 위험이 있다.

### 6.2 올바른 경로는 이미 저장소에 있다

`Tools/EffectPipeline/build_effect_child_parent_resolution.py`가 정확히 그 작업을 한다.
`INSTANCE_MATERIAL_CLASS = "materialinstanceconstant"`, `ROOT_MATERIAL_CLASS = "material"`을 두고
MIC의 `Parent`를 읽어 **MIC → MIC → root Material**까지 걸어 올라간다. blocker 분류가
`DECLARING_PACKAGE_NOT_STAGED`, `CHILD_EXPORT_ABSENT_IN_PACKAGE`, `CHILD_EXPORT_LEAF_AMBIGUOUS`,
`TAGGED_PROPERTY_STREAM_UNREADABLE`, `PARENT_OBJECT_PROPERTY_ABSENT`,
`PARENT_CHAIN_DID_NOT_REACH_ROOT_MATERIAL`, `LEAF_ABSENT_IN_EVERY_STAGED_PACKAGE`로 갖춰져 있고,
`group.leaf` 형태로만 오는 참조도 모든 staged 패키지를 검색해 해결하며 **불일치는 하나를 고르지
않고 ambiguous로 남긴다.**

따라서 권고 순서는 이렇다.

1. MIC의 `Parent`를 읽어 root `Material`까지 walk
2. root에 기존 `extract_ue3_material_graph.py`를 적용
3. MIC의 scalar/vector/texture override를 root 그래프 위에 적용

**막힌 점:** 이 도구의 CLI는 `--authored`로 Effect 저작 corpus를 받는 corpus 전용이다. 임의 MIC
경로를 넣는 범용 입구가 없다. 재사용에는 walk 부분을 분리하거나 작은 범용 entry point를
추가해야 하며, 이는 한 줄 상수 수정이 아니라 검토가 필요한 변경이다. **이번에 구현하지 않았다.**

## 7. 6단계 각각의 상태

| 단계 | 상태 | 다음 검색 위치 |
|---|---|---|
| 1. UV1 ≠ UV0 | **완료** | — |
| 2. lightmap DDS | **부분** — 대표 1쌍 mip0 확보(1024×512 DXT1) | 나머지 134개 추출 + `extract_ue3_texture_mips.py`로 mip 체인 복구 |
| 3. `_RNM_` 규칙 | **완료(재정의)** — 베른 해시는 역산 불가, 재현 불필요. 결정적 규칙 채택, 충돌 0 | — |
| 4. manifest 단 | **부분** — 필요한 payload 형태와 내용을 확정하고 후보로 생성 | adapter `:259` `lightingEvidence` 하드코딩, `:297` `placementLighting=[]`에 최소 변경. 타 세션 파일이라 조정 필요 |
| 5. end-to-end | **막힘** — 교차검사 0 위반까지 갔으나 `check_texture`가 실제 DDS를 요구 | DDS 136개를 `Client/Bin/Resources/Map/Lighting/Maharaka/`에 설치(약 35 MB) |
| 6. MIC 15종 | **부분** — 원인 확정, 올바른 경로 특정, 단순 확장이 틀린 이유 증거 확보 | `build_effect_child_parent_resolution.py`의 walk 분리 또는 범용 entry point |

## 8. 실행 상태 구분

- **확인한 원본:** UV1/UV0 수치, Area별 WModel 차이의 정체, 베른 `_RNM_` 15,483행의 wmodel 공유,
  compiler 계약 3곳, lightmap DDS 치수·포맷, MIC 클래스
- **실제 구현:** 없음. 후보 생성기와 검증 스크립트만
- **live 설치:** 없음
- **게시:** 없음
- **빌드/자동검사:** 빌드 0회. 회귀 `test_extract_source_map_component_lighting.py` **10 OK**,
  `test_build_source_map_materials.py` **13 OK**(도구를 고치지 않았으므로 불변 확인). payload
  교차검사 위반 0건
- **사용자 수동확인:** 없음. 화면에 나타나는 변경을 하지 않았다
- **미완료:** 7절 표

## 9. 다음 작업에 넘기는 판단

가장 작고 효과가 큰 다음 단계는 **lightmap DDS 136개 추출·설치**다. payload는 이미 검증된
형태로 준비돼 있으므로, DDS가 Resources에 들어가면 compiler를 바로 돌릴 수 있다. 단
`mipEvidence`를 `source-chain`으로 주장하려면 mip 체인 복구가 선행되어야 하고, mip0만 설치할
경우 `source-unmipped` + `mipCount 1`로 정직하게 내려야 한다.

adapter 수정은 그 뒤가 낫다. 필요한 출력이 무엇인지 이 문서로 확정됐으니, 그때 최소 변경을
제안하고 타 세션과 조정하면 된다.

---

# DDS 설치

사용자 승인으로 lightmap DDS를 Resources에 **추가**했다. 이어서 조명 행을 정본에 반영하려 했으나
**구조적 차단**을 만나 쓰지 않고 멈췄다. 아래 4절이 그 이유다.

## D1. 설치 결과 — 완료

| 항목 | 값 |
|---|---|
| 대상 | `Client/Bin/Resources/Map/Lighting/Maharaka/` |
| 설치 전 파일 | **1개** (`lv_ocn_dookyis_lut.dds`) |
| 복사 | **136개** |
| 건너뜀(이미 존재) | 0개 |
| 설치 후 파일 | **137개** (기대 137) |
| 추가 바이트 | **9164416** (8.74 MB) |
| LUT 보존 | 크기 16512 bytes, sha256 설치 전후 **동일** |

덮어쓴 파일 0건이다. 어떤 도구의 `install`/sync 경로도 쓰지 않고 파일 단위 복사로만 했다.
복사마다 sha256을 대조하고 `.partial` 임시 파일을 원자 교체했다.

**예상 35 MB가 아니라 8.74 MB다.** 35 MB 추정은 136개가 전부 1024×512라는 가정이었는데,
실측하면 8×8부터 1024×1024까지 분포하고 1024×512는 18개뿐이다.

## D2. 136개 실측 — 전부 DXT1, mip0만

| 치수·포맷 | 개수 |
|---|---|
| 256x256 DXT1 | 30 |
| 1024x512 DXT1 | 18 |
| 256x128 DXT1 | 16 |
| 32x32 DXT1 | 16 |
| 128x128 DXT1 | 12 |
| 512x256 DXT1 | 10 |
| 128x64 DXT1 | 10 |
| 64x64 DXT1 | 8 |
| 64x32 DXT1 | 4 |
| 8x8 DXT1 | 4 |
| 1024x1024 DXT1 | 4 |
| 32x16 DXT1 | 2 |
| 512x512 DXT1 | 2 |

추출 136/136 성공, 실패 0. DDS 체인 길이가 선언 치수와 **136/136 일치**한다.
레벨 분포는 SL01 104 / PS 18 / LAND01 14이고 average 68 + directional 68이다.
SL01 패키지의 `lightmaptexture2d` 객체 수가 정확히 104개로 payload와 맞는다.

UModel이 저장소의 한글 경로를 깨뜨리므로 ASCII 스크래치
`C:/LostArkExtract/MhpLightmapInstall_20260926/`에서 추출하고 결과만 복사했다.
이 UModel 빌드는 `-class=`와 다중 객체명을 모두 거부하므로 객체당 한 번씩 호출했다.

## D3. `check_texture` 교차검사 — **통과 136 / 거부 0**

이번 작업의 핵심 검증이다. compiler 모듈을 직접 import해서 실제 `check_texture(item, resources_root)`를
설치된 DDS에 대해 돌렸다. 통과 **136**, 거부 **0**.

| 검사 항목 | 결과 |
|---|---|
| sha256 일치 | 136/136 |
| DDS 헤더·치수·mip 수 | 136/136 |
| `colorSpace` | `linear` 136 |
| `mipEvidence` | `source-unmipped` 136 |
| `mipCount` | 1 (136/136) |

**mip 체인은 복구하지 않았다.** mip0만 뽑았으므로 `mipEvidence`를 `source-unmipped`로,
`mipCount`를 1로 내렸다. compiler가 `mipEvidence != 'source-unmipped' or actual_mips == 1`을
직접 강제하므로 이 쌍은 도구가 검증한다. 후보에 `source-chain`으로 적혀 있던 값을 고쳤다.

중간에 거부 136건이 나온 실행이 한 번 있었는데 compiler 문제가 아니라 내 집계 코드가
`dict(assetId=..., **result)`로 키를 중복 전달한 버그였다. 고친 뒤 136/0이다.

## D4. 조명 행 반영 — **차단. 정본에 쓰지 않았다**

조율자가 준 전제는 "기존 329행과 내 990행의 `(assetId, materialName)` 키가 0건 겹치므로 순수
추가 가능"이었다. **이 전제는 성립하지 않는다.** 내 990행은 필드가 `assetId`와 `bakedLighting`
**2개**뿐인 compiler 입력 조각이고, 실제 material 행은 `family`, `diffuseTexture`,
`textureColorSpace`, `sourceMaterial` 등 **30개 필드**를 갖는다. 내 행에는 `materialName`이
아예 없으므로 `(assetId, materialName)` 비교는 무엇과도 겹치지 않는다 — 겹침 0은 안전 신호가
아니라 **비교가 성립하지 않았다는 신호**다.

990행을 실제 구성 가능성으로 나누면 이렇다.

| 구분 | 행 | 덮는 placementLighting | 상태 |
|---|---|---|---|
| A. 기존 행에 `bakedLighting` 필드만 추가 | 78 | **180 / 3,786 (4.8%)** | 가능 |
| B. 기존 행을 클론해 새 `assetId`로 생성 | 530 | 2,126 (56%) | `.mapassets` catalog 등록 필요 |
| C. 기존 material 행이 아예 없음 | 382 | 1,480 (39%) | **생성 불가** |

C가 막히는 이유가 근본이다. 이 Area는 `sourceMaterialBuild: null`이어서 **source material
정의가 한 번도 만들어진 적이 없다.** 382행은 복사할 원본 행조차 없으므로 값을 지어내지 않는 한
만들 수 없다.

B는 새 `assetId`를 만드는 일이고, 런타임이 그 ID를 bind하려면 `.mapassets` catalog가 그것을
admit해야 한다(베른이 `_RNM_` 15,483행을 base wmodel로 admit한 방식). 그 catalog는 scene
builder 생성물이므로 손으로 행을 넣는 것은 생성물 수동 편집 금지에 걸리고, 재생성은 손으로 쓴
329행을 덮을 위험이 있다 — 같은 날 무대 설치 작업이 바로 그 이유로 멈춘 지점이다.

A만 반영하면 조명이 **4.8%**만 붙는다. 게다가 A의 78자산 중 13개는 기존 material 행이
2~7개씩 있고, compiler가 `all(row.get('bakedLighting') for row in rows if row['assetId'] ==
instance['assetId'])`를 요구하므로 같은 `assetId`의 **모든** 행에 같은 조명을 붙여야 한다.

조율자가 제시한 목표 수치 **materials 329 → 1,319, placementLighting 0 → 3,786**은
현재 입력으로 **도달할 수 없다.** 4.8%만 쓰고 목표 달성으로 보고하는 것은 거짓 보고이므로
정본을 건드리지 않고 멈췄다. `mapmaterials.json`은 sha256 `8ad7ecdb3eb926c1`,
materials 329, placementLighting 0으로 그대로다. 게시본도 백업과 바이트 동일하다.

## D5. `unmatchedMeshes` 5종

payload 생성 때 catalog에 base 자산이 없어 제외된 mesh 5종의 정체다.

| mesh | 정체 |
|---|---|
| `bg_rhd_stone_rock01_sm_ksr` | 섬 범위 규칙(`y > 50000`)에서 제외된 레이싱 트랙 바위 |
| `bg_tot_movillage_decoprop07f_sm_artree` | 법선·탄젠트 퇴화로 admission에서 제외된 장식 |
| `lv_common_mesh_cul_box_1/4/8` | 보이지 않는 컬링 헬퍼 박스 |

전부 기존 admission 기록과 일관된 정당한 제외다. 이 때문에 RNM component 4,077개 중 291개가
payload에 들어가지 않았다(4,077 → 3,786).

## D6. Resources 전달 목록

팀장 Drive 전달 대상은 `Map/Lighting/Maharaka/` 아래 **136개, 총 9164416 bytes (8.74 MB)**다.
전체 목록과 파일별 치수·포맷·sha256은
`out/MaharakaContinuation_20260926_193455/MaterialPipeline/lightmap-install-manifest.json`의
`files` 배열에 있다. 기존 `lv_ocn_dookyis_lut.dds`는 전달 대상이 아니다(이미 설치돼 있고 변경 없음).

처음 10개:

- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_directionalmaxcomponent0_120.dds` — 256×256 DXT1, 32896 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_directionalmaxcomponent0_125.dds` — 256×128 DXT1, 16512 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_directionalmaxcomponent0_129.dds` — 256×128 DXT1, 16512 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_directionalmaxcomponent0_138.dds` — 256×256 DXT1, 32896 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_directionalmaxcomponent0_49.dds` — 512×256 DXT1, 65664 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_directionalmaxcomponent0_51.dds` — 256×256 DXT1, 32896 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_directionalmaxcomponent0_8.dds` — 256×256 DXT1, 32896 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_normalizedaveragecolor0_120.dds` — 256×256 DXT1, 32896 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_normalizedaveragecolor0_125.dds` — 256×128 DXT1, 16512 bytes
- `Map/Lighting/Maharaka/lv_ocn_eventis_mhp_land01_normalizedaveragecolor0_129.dds` — 256×128 DXT1, 16512 bytes

## D7. 상태 구분

- **실제 구현:** 추출·설치·검증 스크립트 4개. 도구·제품 코드 무변경
- **live 설치:** lightmap DDS **136개 추가** (1 → 137파일, 8.74 MB). LUT 보존
- **게시:** 없음. publisher를 한 번도 실행하지 않았다
- **빌드/자동검사:** 빌드 0회. `check_texture` **136 통과 / 0 거부**. 회귀 10 OK / 13 OK
- **사용자 수동확인:** 없음. 조명이 화면에 어떻게 보이는지는 사용자가 판정한다
- **미완료:** 조명 행 반영(D4), mip 체인 복구, `sourceMaterialBuild`, MIC 15종

## D8. 다음에 필요한 선결 조건

조명을 4.8%가 아니라 전체로 붙이려면 **이 Area의 source material build를 먼저 돌려야 한다.**
그것이 382행의 material 정의를 만들고, 그 과정에서 scene builder가 catalog를 재생성하므로
B의 530 variant도 같은 transaction에서 admit된다. 즉 D4의 A/B/C가 한 번에 풀린다.
DDS는 이미 설치돼 있고 `check_texture`를 통과하므로 그 build의 입력 중 텍스처 쪽은 준비됐다.

그 build를 돌리기 전에 손으로 쓴 329행(물 10행과 사용자 override 포함)을 어떻게 보존할지
먼저 정해야 한다. 이것이 같은 날 무대 설치 작업과 공유하는 미결 항목이다.
