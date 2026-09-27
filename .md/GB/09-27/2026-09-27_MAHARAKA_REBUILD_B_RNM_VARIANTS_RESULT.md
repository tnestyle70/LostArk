# 2026-09-27 마하라카 재구축 B단계 — 무대 해금, RNM 재산출, 변형 세트, 라이트맵/그림자 추출

A단계(`2026-09-27_MAHARAKA_REBUILD_A_MATERIAL_INPUTS_RESULT.md`)를 입력으로 B1~B5를 수행했다.
모든 산출물은 `out/MaharakaRebuild_B/` 아래에 있다. `Data/**`와 `Client/Bin/DataFiles/**`는
쓰지 않았고 publisher도 실행하지 않았다. 게시는 C단계가 한 세트로 수행한다.

## 0. 단계별 상태

| 항목 | 상태 | 근거 |
|---|---|---|
| B1 무대 해금 | **완료** | 슬롯 330 → 355, 무대 3자산 실제 행 출력, 제거 0 / 값 변경 0 |
| B2 RNM 재산출 | **완료** | placement 4,669 전수 분류, canon 231 인스턴스 동일성 위반 0 |
| B3 네 문서 한 세트 | **완료** | publisher 규칙 위반 0, 도구·스크립트 산출 바이트 동일 |
| B4 LightMapTexture2D mip | **완료** | 클래스 게이트 수정 후 `SOURCE_MIP_CHAIN_VALIDATED`로 실증 |
| B4 ShadowMap2D | **부분** | 계약·런타임은 이미 완비, `pf_g8` 무압축 때문에 payload 추출이 남음 |
| B5 테스트 | **완료** | 신규 14개 + 기존 회귀 7스위트 전부 통과 |

**무대 두 자산 + 노랑 원반이 후보에 실제 행으로 나오는가: 예.** 세 자산 모두 행 1개씩 생성됐다.

## 1. B1 — 무대를 막고 있던 것

### 1.1 근본 원인 (게이트 3개가 직렬로 있었다)

PNG는 사고가 아니라 추출기 형식 게이트의 결과였다.

1. **`extract_ue3_texture_mips.py:56`** — 참조 mip0을 legacy BC DDS로만 파싱한다.
   `Restore/mips/batch.results.json`에서 이 4종 중 3종(4번째는 09-26 신규 cook이라 이 배치에 없음)이
   `failed / "expected a legacy BC DDS"`로 기록돼 있다. 같은 사유 실패가 전체 7종이다.
   원본이 BC가 아니므로 UModel이 PNG로 내보냈다.
2. **`build_source_map_material_inputs.py:113`** — 설치 텍스처를 sha256 → 경로로 역인덱싱할 때
   `path.lower().endswith('.dds')`인 항목만 넣는다. PNG 설치는 구조적으로 해석될 수 없고,
   한 슬롯 거부가 그 자산 전체를 skip시킨다(`another slot of the same asset is not compiled`).
3. **`build_source_map_material_inputs.py:170~175`** — mip 체인 receipt가 없으면 colorSpace를
   `srgb`로 기본 처리한다. normal lane은 linear를 요구하므로 DDS로 바꾼 직후에도
   `normal lane texture_normal is not linear in the source` 21건으로 사유만 바뀐다.

추가로 무대 쐐기 2자산은 네 번째 게이트에 걸려 있었다:
`lighting evidence unresolved: no component lighting record for ...floor01a_sm_lnh`.
쐐기는 `LV_OCN_EVENTIS_MHP_SCENE03B` 서브레벨 소속인데 조명 receipt가 LAND01·PS·SL01
세 개뿐이었다. 노랑 원반은 SL01이라 이 게이트에 걸리지 않았다.

### 1.2 원본 실측 — 형식 판단의 근거

추출기의 첫 단계는 패키지와 무관하므로 우회하고 원본 export를 직접 읽었다
(`out/MaharakaRebuild_B/b1_source_texture_properties.json`).

| 텍스처 | srgb | format | compressionSettings | native mip |
|---|---|---|---|---|
| `bg_ocn_etc_floor01_n_lnh` (노랑 원반) | **false** | **pf_a8r8g8b8** | `tc_normalmap` | 10 |
| `bg_ocn_etc_floor01a_n_lnh` (쐐기 A/B) | **false** | **pf_a8r8g8b8** | `tc_normalmap` | 10 |
| `bg_lut_lucastle_houseinfloor01_n_ysi` | **false** | **pf_a8r8g8b8** | `tc_normalmapuncompressed` | 10 |
| `bg_rad_abrelshud_floor18_n_ksr` | **false** | **pf_a8r8g8b8** | `tc_normalmapalpha` | 10 |

네 종 모두 `compressionnone=true`, `lodgroup=texturegroup_worldnormalmap`이다.
**원본은 압축 텍스처가 아니었다.** 그래서 무압축 A8R8G8B8 DDS로 변환했다. ATI2/BC5(형제 182종 중
173종)로 갔다면 원본에 없던 압축 손실을 넣는 것이 된다. 헤더 구성은 소비자인 어댑터 자신의
`build_dds_a8r8g8b8`(A8R8G8B8, 1×1까지 전체 mip 체인, BGRA, 동일 flag/mask)를 그대로 따랐다.

`srgb=false`는 UE3가 기본값을 직렬화하지 않기 때문에 명시 저장된 값이다. 즉 어댑터의 srgb 기본값이
이 4종에 대해서만 틀렸던 것이고, linear가 원본 사실이다. 추측이 아니다.

`bg_rad_abrelshud_floor18_n_ksr`만 알파에 데이터가 있다(min 52 / max 204 / 고유값 152, RGB의 B는
254 고정). `tc_normalmapalpha`가 이를 뒷받침한다. 이 종만 mip 생성 시 RGB 재정규화를 하지 않고
4채널 box 평균만 적용했다. 나머지 3종은 실제 접선공간 법선(길이 0.995±0.003)이라 레벨마다
단위길이로 재정규화했다.

### 1.3 적용한 변경

- **Resources 추가 21개** (`out/MaharakaRebuild_B/b1_dds_install.receipt.json`)
  - 설치명은 `<새 sha12>_<leaf>.dds`이고 기존 PNG는 `<PNG sha12>_<leaf>.png`이므로 파일명이
    달라 **덮어쓰기가 구조적으로 불가능**했다. 충돌 0건, 백업 0건, PNG 21개 전부 보존.
  - 폴더 분포: `floor01` 1곳, `floor01a` 2곳, `houseinfloor01` 12곳, `floor18` 6곳. 합 28.0 MB.
  - 각 파일 512×512, mip 10, 1,398,228 bytes(= 128 헤더 + 1,398,100 payload, 검산 일치).
    512 크기 기존 DDS normal 형제들의 mip 분포(mip 10이 41종)와 같다.
- **receipt 4개 추가** (`out/MaharakaRebuild_B/chains_b1/`, 기존 561개 복사 + 4개 = 565개)
  - status는 `SOURCE_PROPERTIES_ONLY`다. 복구된 mip 체인을 검증한 것이 아니므로
    `SOURCE_MIP_CHAIN_VALIDATED`라고 쓰지 않았다. 이 4종은 `recovered` 집합에 없으므로
    어댑터가 계속 `mipEvidence=project-generated`로 표기한다.
- **SCENE03B 조명 증거 추출** (`out/MaharakaRebuild_B/ComponentLighting/SCENE03B.*`)
  - `A89UVJO9YX8XOUPN9JHI29ONJXOX7SC.upk`, 컴포넌트 60개, **전부 `NO_LOD_LIGHTING_DATA`**.
  - 즉 무대 쐐기는 원본에 라이트맵이 없다. RNM을 붙이면 안 되고 `source-absent`로 해석돼야 한다.
- 원본 staging 파일(`mip-catalog-merged.json`, `runtime-384.json`)은 수정하지 않았다.
  갱신본을 `out/MaharakaRebuild_B/inputs/`에 따로 썼고 어댑터 인자로 넘겼다.

### 1.4 결과

먼저 A단계 호출을 그대로 재현해 내 명령이 정확한지 확인했다:
슬롯 330 / 텍스처 364 / 조명 요약 동일 / **슬롯 본문 바이트 동일 / 텍스처 본문 동일**.

| 지표 | A단계 | B1 | 변화 |
|---|---|---|---|
| 재질 슬롯 | 330 | **355** | +25 (직접 21 + 연쇄 4, 사전 예측치와 일치) |
| 결합 자산 | 252 | **273** | +21 |
| 텍스처 | 364 | 375 | +11 |
| skip 슬롯 | 190 | 165 | −25 |
| 제거된 슬롯 | — | **0** | — |
| 값이 바뀐 슬롯 | — | **0** | — |
| `not a DDS` 실패 | 21 | **0** | — |

무대 3자산 행 (`out/MaharakaRebuild_B/compiled/b1.mapmaterials.json`):

| 자산 | 역할 | `diffuseColor` | brightness |
|---|---|---|---|
| `MAP_1AF968D24AD5_BG_OCN_ETC_FLOOR01_SM_LNH_OVR_8EB1D0D03456` | 노랑 중앙 원반 | `[0.882022500038147, 0.8211935758590698, 0.07928290218114853, 1.0]` | 1.5 |
| `MAP_167F91F6940F_BG_OCN_ETC_FLOOR01A_SM_LNH_OVR_0636FCF08A2E` | 쐐기 A | `[0.967422604560852, 0.533707857131958, 1.0, 1.0]` | 2.0 |
| `MAP_AF1951C8B827_BG_OCN_ETC_FLOOR01B_SM_LNH` | 쐐기 B | `[0.9407327175140381, 0.1516854166984558, 1.0, 1.0]` | 1.0 |

쐐기 A/B는 A단계가 기록한 G03 실측값과 **자리수까지 일치**(스크립트가 1e-9 비교로 확인).
노랑 원반은 실제로 노란색이다(R 0.88 / G 0.82 / B 0.079). 세 행 모두
`textureColorSpace.normal = "linear"`이고 `normalTexture`가 새로 설치한 DDS를 가리킨다.

참고: 쐐기 B의 `normalTexture`는 쐐기 A 자산 폴더의 경로를 가리킨다. 두 자산이 같은 텍스처
sha를 공유하고 어댑터의 `installed`가 `setdefault`로 첫 경로를 유지하기 때문이다. 기존 동작이며
양쪽 폴더에 파일이 설치돼 있어 해석에 문제는 없다.

### 1.5 A단계 5종 대표 재검증

A단계는 5종 중 1종만 완전 통과였다. 전부 다시 확인했다.

| 종류 | A단계 | B1 재검증 |
|---|---|---|
| 텍스처 재질 | 통과 (252자산/364텍스처) | **통과** — 273자산 / 355슬롯 / 참조 리소스 374개 |
| 벡터 색(무대) | PNG 때문에 행 미생성 | **통과** — 3자산 행 생성, G03 실측 일치 확인 |
| 물 | 컴파일러 재현 불가 | **변화 없음** — 후보 0행 / canon 10행. 10행 전부 `family: source.map.water-42.v1`이고 `build_map_water_presentation.py`가 소유한다. C가 행 단위로 병합해야 하며 재질 컴파일러가 만들 수 없다 |
| 지형 | mapmaterials 경로 아님 | **변화 없음** — 후보 0 / canon 0. `LAND01_LC_*`는 baked PNG 경로이고, B2에서 이 16개가 조명 컴포넌트 레코드 없는 16건과 정확히 대응함을 재확인했다 |
| foliage | vertex wind 42슬롯 미검증 | **변화 없음** — 42건 그대로. 이번 변경과 무관한 별도 항목이다 |

물 10행의 판별자는 `materialName`이 아니라 `family`다. 이름은 `SLOT_000_dummy_material_0`,
`SLOT_000_bg_fat_stone_rock`처럼 물과 무관하게 보인다.

### 1.6 남은 skip 165건

| 사유 | 건수 |
|---|---|
| foliage vertex wind needs a verified binding | 42 |
| another slot of the same asset is not compiled | 29 |
| null material slot | 23 |
| 지원하지 않는 terminal (`ocean_trn` 12, `bg_base_trn` 11, `molding_trn` 2, `monster_base_opa` 2 등) | 29 |
| `'emissive_intensitymin'` | 10 |
| unhandled static switches | 10 |
| geometry channel missing in the cooked WModel | 3 |
| 기타 개별 재질 | 19 |

## 2. B2 — RNM 목록 재산출

조인은 `sourcePlacementId → 게시 assetId → 재질 슬롯 → 원본 컴포넌트 RNM 쌍`만 사용했다.
메시 leaf 이름 조인은 쓰지 않았다. **244나 베른 비율을 목표로 삼지 않았고** 원본에 조명이 없는
오브젝트에는 쌍을 붙이지 않았다.

입력: 게시 placement 4,669행, 조명 패키지 4개(LAND01 82 / PS 244 / SL01 3,823 / SCENE03B 60)
= 컴포넌트 레코드 4,209개, 후보 재질 355행/273자산.

### 2.1 placement 분류

| 분류 | 건수 |
|---|---|
| RNM 쌍 보유 | 3,751 |
| 컴포넌트 레코드 없음 | 828 |
| 원본 조명 없음 (부착 금지) | 90 |

레코드 없는 828건은 전부 설명된다: **foliage overlay 인스턴스 812건**
(`...:foliage:export:N:instance:M` — InstancedStaticMesh 인스턴스라 컴포넌트별 라이트맵 레코드가
애초에 없다) + **LAND01 landscape 컴포넌트 16건**(`...:landscape:export:N` — A단계가 확인한
baked PNG 지형 경로 16종과 개수가 일치). 추출 누락이 아니다.

### 2.2 재산출 표 (G04 대비)

| 구분 | 자산 | 인스턴스 | G04 |
|---|---|---|---|
| 재질 행 있음 · 아틀라스 쌍 1개 (즉시 게시 가능) | **111** | **261** | 99 / 231 |
| 재질 행 있음 · 쌍 여러 개 (변형 필요) | **156** | **2,482** | 141 / 2,322 |
| 재질 행 없음 · 쌍 1개 | 38 | 117 | (합 130 / 1,194) |
| 재질 행 없음 · 쌍 여러 개 | 69 | 891 | — |
| 합계 | **374** | **3,751** | — |

A단계의 coverage 확장으로 "재질 행 없음"이 130 → **107자산**으로 줄었고 즉시 게시 가능 자산이
99 → 111로 늘었다. 자산별 쌍 개수 분포는 1개 149 / 2개 68 / 3개 53 / 4개 22 / 5개 30 / 6개 19 /
7개 10 / 8개 11 / 9개 3 / 10개 3 / 11개 1 / 12개 4 / 13개 1이다.

쌍 1개 자산의 재질 행은 155행, 쌍 여러 개 자산은 기존 194행 → 변형 747행이 된다.

### 2.3 기존 출력은 틀리지 않았다

canon의 `bakedLighting` 142행 / `placementLighting` 231 인스턴스를 현재 게시 placement와 대조했다.

- publisher 동일성 제약(`$sourcePlacements[id] -cne $lighting.assetId`) **위반 0건**
- 게시 placement에 없는 `sourcePlacementId` **0건**
- canon 조명 행의 자산 99개가 후보에도 99/99 존재하고, RNM 재산출에도 99/99 등장

즉 **기존 231 인스턴스는 유효하며 내 111자산의 부분집합이다.** "설치된 136개가 잘못된 후보에
의해 선택되고 있었다"는 것과 "파일 자체가 틀렸다"는 것은 다르다는 지시가 데이터로 확인됐다.

### 2.4 아틀라스 가용성

재산출이 요구하는 원본 라이트맵 객체는 **108개**이고, canon의 파일명 규칙
(`sourceObject.replace('.','_').lower() + '.dds'`)대로 **108개 전부 이미 설치돼 있다(미설치 0)**.
따라서 B3는 조명용 Resources 추가 설치 없이 산출 가능하다.

설치 137개의 쓰임: canon 참조 44 / 재산출 요구 108 / 어느 쪽도 쓰지 않는 29
(LAND01 라이트맵 다수와 `lv_ocn_dookyis_lut.dds`).

### 2.5 RNM을 붙이면 안 되는 대상

원본 조명 없음: **19자산 / 90 placement**. 여기에 **무대 쐐기 18조각이 포함된다**
(`floor01a` 9 + `floor01b` 9). 크리스마스 풍선 3종(27건), `bg_ocn_etc_deco03`(8건) 등이다.
9개 자산은 같은 자산의 일부 배치만 조명이 없으므로 **자산 단위가 아니라 placement 단위로**
판정해야 한다.

## 3. B3 — catalog · placements · materials · placementLighting 한 세트

### 3.1 변형 키

계약을 먼저 실측했다. 재질 행의 `bakedLighting`은 **아틀라스 쌍만** 담고
(`averageTexture`, `directionalTexture`, `colorSpace` — publisher가 `Assert-ExactJsonProperties`로
이 3필드만 허용), per-placement 값은 전부 `placementLighting` 인스턴스가 담는다
(`coordinateScale`, `coordinateBias`, `averageScale`, `directionalScale`).

그러므로 **변형 키는 아틀라스 쌍**이고, base 자산 identity + 그 쌍의 원본 객체 identity로 만든다:

```
변형 assetId = <baseAssetId>_LM_<sha256(baseAssetId|average|directional)[:12] 대문자>
```

base assetId는 placement가 가장 많은 쌍을 유지한다. 변형 catalog 행은 base 행을 복사해
token 0(assetId)과 token 3(prototype tag)만 바꾼다. **token 2(모델 경로)는 base 그대로**이므로
geometry 재cook도, 변형 이름의 새 Resources 폴더도 필요 없다(446/446 공유 확인).

### 3.2 산출

| 문서 | 변경 |
|---|---|
| `LV_OCN_EVENTIS_MHP.mapassets` | 408 → **854행** (변형 446개 추가, 전부 base `.wmodel` 재사용) |
| `LV_OCN_EVENTIS_MHP.mapplacements` | 4,669행 유지, **1,470건 assetId 재지정** |
| `LV_OCN_EVENTIS_MHP.mapmaterials.json` `materials` | 355 → **908행** (변형 553행), `bakedLighting` 보유 **902행** |
| 같은 문서 `placementLighting` | 231 → **2,743 인스턴스** |

base 유지 267자산 + 변형 446자산. 제외는 재질 행 없는 **107자산 / 1,008 인스턴스**
(3,751 − 2,743 = 1,008로 일치). publisher가 `bakedLighting` 없는 자산의 `placementLighting`을
거부하므로 이들은 게시할 수 없다.

행 수 검산: 902 = 변형 553 + base 349. 355 − 349 = 6은 RNM이 없는 6자산(273 − 267)의 행이다.

### 3.3 자체 검증

읽어낸 publisher 규칙 전부를 코드로 재검사했다: catalog assetId 중복, 모델 경로 충돌, 재질 행
중복, catalog에 없는 재질 자산, `bakedLighting` 필드 집합·colorSpace, 라이트맵 경로 형식과
파일 실재, `placementLighting` 필드 집합·`bakedAssets` 소속·`sourcePlacementId` 유일성·게시
placement 존재·assetId 동일성·벡터 성분 수와 부호·아틀라스 경계(`scale+bias ≤ 1.00001`).

**위반 0건.** 원본 조명 없는 90 placement에 RNM 누출 **0건**.

### 3.4 C가 즉시 게시할 수 있는가

**아니다. 행 단위 병합이 먼저 필요하다.** 후보는 canon을 대체하는 문서가 아니다.

| canon 대비 | 행 수 | C가 해야 할 일 |
|---|---|---|
| 교집합 · 값 동일 | 177 | 그대로 |
| 교집합 · 값 상이 | 142 | canon만 `bakedLighting`을 갖고 있던 행이다. B3가 이 142행에도 쌍을 채우므로 재산출 값으로 대체 가능하되, 같은 쌍인지 확인 후 교체 |
| canon 전용 | 10 | **반드시 보존.** 물 행이고 재질 컴파일러가 만들 수 없다 |
| 후보 전용 | 36 | 추가 (A단계 11 + B1 25) |

그리고 `.mapassets`는 `Data/Maps/Imported/`, `.mapplacements`는 `Data/Maps/Authoring/` 소유이며
둘 다 Git LFS다. Area당 담당 1명 규칙이 있으므로 catalog 446행 추가와 placement 1,470건 재지정은
맵 담당자의 승인이 필요하다. 게시는 `Publish-MapAuthoring.ps1` 한 트랜잭션으로 수행한다.

### 3.5 재실행 가능한 도구로 승격

일회성 스크립트를 C가 실행할 수 있는 도구로 만들었다.

- `Tools/MapPipeline/build_map_rnm_variant_set.py` — 변형 계획, 네 문서 산출, publisher 규칙 자체 검증
- `Tools/MapPipeline/test_build_map_rnm_variant_set.py` — 14개 테스트

실제 데이터로 도구를 실행한 결과가 임시 스크립트 산출과 **세 문서 모두 sha256 일치**했다
(`b3.mapmaterials.json`, `b3.mapassets`, `b3.mapplacements`). 독립 두 경로가 같은 답을 냈다.

```
python Tools/MapPipeline/build_map_rnm_variant_set.py --area-id LV_OCN_EVENTIS_MHP \
  --catalog Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapassets \
  --placements Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements \
  --materials out/MaharakaRebuild_B/compiled/b1.mapmaterials.json \
  --rnm out/MaharakaRebuild_B/b2_rnm_placements.json \
  --lightmap-directory Map/Lighting/Maharaka --resources-root Client/Bin/Resources \
  --output-directory <출력>
```

## 4. B4 — LightMapTexture2D mip 추출과 ShadowMap2D

### 4.1 LightMapTexture2D — 완료

`extract_ue3_texture_mips.py:129`가 export 클래스를 문자열 `"texture2d"`와 동등 비교했다.
이것이 라이트맵 mip 추출을 막던 유일한 원인인지 먼저 실측했다: 게이트를 우회해
`lightmaptexture2d` export를 직접 읽으면 `parse_tagged_properties`와 `parse_native_mips`가
**둘 다 성공하고 완전한 native mip 체인을 돌려준다.** 레이아웃 차이가 아니라 과도하게 좁은
동등 비교였다.

원본 라이트맵 실측(`out/MaharakaRebuild_B/b4_lightmap_probe.json`):
`format pf_dxt1`, 256×256 → native mip **9단**, 1024×512 → **11단**.
**원본은 mip을 가지고 있다.** `source-unmipped`는 성립하지 않는다.

적용한 변경 — `Tools/LevelPlacementExtractor/extract_ue3_texture_mips.py`:

```python
TEXTURE2D_CLASSES = ("texture2d", "lightmaptexture2d", "shadowmaptexture2d")
...
require(package.cls(index) in TEXTURE2D_CLASSES, f"unsupported source class {package.cls(index)}")
```

두 앵커 모두 1회 일치로 패치했고 파일은 순수 ASCII·LF·BOM 없음을 유지했다(17,721 → 18,076 bytes).
백업은 `out/MaharakaRebuild_B/backup/extract_ue3_texture_mips.py.before-b4-class-gate`.
그 외 클래스는 여전히 이름으로 거부한다.

실증(`out/MaharakaRebuild_B/b4_lightmap_mip_extraction.json`) — 기존 rotate-and-export 기계를
그대로 쓴다:

| 라이트맵 | 참조 mip0 | 복구 결과 | 원본 native | receipt status |
|---|---|---|---|---|
| `normalizedaveragecolor0_158` | 256×256 DXT1 mip 1, 32,896 B | 256×256 DXT1 **mip 9**, 43,832 B | 9 | `SOURCE_MIP_CHAIN_VALIDATED` |
| `directionalmaxcomponent0_158` | 256×256 DXT1 mip 1, 32,896 B | 256×256 DXT1 **mip 9**, 43,832 B | 9 | `SOURCE_MIP_CHAIN_VALIDATED` |

mip 개수 일치, 레벨별 크기 일치 모두 확인됐다. 라이트맵이 `pf_dxt1`이라 기존 BC 경로가 그대로
동작하고, 막던 것은 클래스 게이트 하나였다.

**현재 설치본은 mip이 없다.** 설치 137개 전부 `mip 1`이고, 재산출이 요구하는 108개도 전부
`mip 1`이다. 원본은 9~11단이므로 mip 절단이 실재하는 결함이다.

기존 `Tools/MapPipeline/Build-DdsMipChain.py`는 **DXT1만** 지원하고 Pillow로 **재인코딩**하는
근사 경로다(`encode_dxt1`). B4의 원본 체인 복구는 재인코딩 없이 원본 mip 바이트를 가져오므로
이 도구를 대체한다. 또한 이 도구는 `pf_a8r8g8b8`인 B1의 4종을 거부하므로 B1에 쓸 수 없었다.

108개 전체 배치는 `out/MaharakaRebuild_B/b4_lightmap_mips/`로 실행했다(§6에 결과).
**설치하지 않았다.** 복구본의 파일명은 설치본과 동일하므로 설치는 기존 Resources 파일 108개를
교체하는 행위가 된다. 그 교체는 한 세트를 게시하는 C단계의 판단에 맡긴다.

### 4.2 ShadowMap2D — 부분

#### 계약과 런타임은 이미 완비돼 있다

"현재 도구 미지원"이 "계약 미지원"이 아님을 확인했다.

| 계층 | 위치 | 내용 |
|---|---|---|
| 저작 schema | `Publish-MapAuthoring.ps1:2778~2790` | `bakedLighting.staticShadow` = `texture`, `lightGuid`(32 hex), `lightChannel`(정수 1~15), `penumbraWidth`(0<x≤1), `penumbraBasis`(`PROJECT_ADAPTER` 고정), `shadowExponent`(0<x≤128) |
| 저작 schema | 같은 파일 `:3400~3412` | `placementLighting.shadowCoordinateScale`/`shadowCoordinateBias` 쌍, 해당 자산이 `staticShadow`를 가질 때만 허용, 아틀라스 경계 검사 |
| Client 파싱 | `MapAssetCatalog.cpp:562~583` | 같은 6필드 검증, `surface.hasStaticShadow`, `staticShadowChannel`, `staticShadowTransfer = float4(width*0.5-0.5, 1/width, exponent, 0)` |
| Client 파싱 | `MapAssetCatalog.cpp:1763~1783` | shadow 좌표 파싱 |
| Engine 구조 | `ModelAssetData.h:221~223`, `262`, `304` | surface 필드와 텍스처 경로 |
| Engine 바인딩 | `Material.cpp:533`, `691`, `995~997` | DDS 로드, `g_StaticShadowChannel` 바인딩 |
| Engine 광원 | `Light.cpp:98`, `Light_Manager.cpp:270` | 광원별 `staticShadowChannel` → `record.flags[1]` |
| 셰이더 | `Shader_Deferred.hlsl`, `Shader_VtxMeshBinary.hlsl`, `Shader_VtxMeshMapInstance.hlsl`, `Shader_MapMaterialSurface.hlsli` | 소비 |

`penumbraBasis`가 `PROJECT_ADAPTER` 고정인 것은 penumbra 폭과 중심이 원본 값이 아니라 프로젝트
어댑터 값이라는 뜻이다. 원본에서 가져올 수 없는 값임이 계약에 명시돼 있다.

#### 원본 저장 구조 (실측)

`out/MaharakaRebuild_B/b4_shadowmap2d_probe.json`. SL01 패키지에
`shadowmap2d` **3,027개**, `shadowmaptexture2d` **52개**, `lightmaptexture2d` 104개, `shadowmap1d` 3개.
컴포넌트 3,027개가 정확히 1개씩 참조하고 796개는 참조가 없다.

`shadowmap2d` export는 193 bytes, 속성 5개로 전부 읽힌다:

| 속성 | 타입 | 예시 |
|---|---|---|
| `texture` | objectproperty | export 3592 (→ `shadowmaptexture2d`) |
| `coordinatescale` | structproperty | `{x: 0.02734375, y: 0.0546875}` |
| `coordinatebias` | structproperty | `{x: 0.423828125, y: 0.12890625}` |
| `lightguid` | structproperty | 16 bytes, `5d9093ae8f7f5f438eae4d867e63b073` |
| `bisshadowfactortexture` | boolproperty | false |

`shadowmaptexture2d`는 **`format pf_g8`** — 단일 채널 8bit다. 1024×512 → native mip 11단,
256×256 → 9단이며 **넓힌 게이트를 통과한다**. UV는 lightmap UV와 같은 채널을 쓰고 배치별
`coordinatescale`/`coordinatebias`로 아틀라스 안의 자기 영역에 매핑된다(구조가 RNM과 동일하다).
채널은 G8 단일 채널이 그림자 계수다.

#### 막힌 지점

`pf_g8`은 무압축이므로 UModel이 DDS가 아니라 PNG/TGA로 내보낸다. 즉 **B1의 4종 법선과 정확히
같은 벽**(`extract_ue3_texture_mips.py:56`의 BC 전용 참조 파싱과 `block_bytes`의 DXT1/DXT5/ATI2
제한)에 걸린다. 조명 receipt의 `shadowPayloadDecoded: false`가 이것으로 설명된다.

따라서 남은 작업은 추출기에 비-BC(무압축) 참조·조립 경로를 추가하는 것 하나이고, 그것을
고치면 B1의 법선 4종과 그림자 아틀라스가 **함께** 원본 정확도로 복구된다.

#### 이중 조명 방지 — 설계는 이미 있고 측정으로 확인했다

새로 설계할 필요가 없었다. 기존 채널 기구가 정확히 이 목적의 장치다.

표본 400개를 실측한 결과 **400/400에서 `shadowmap2d.lightguid`가 그 컴포넌트의
`bakedLightGuids`와 서로소였다. 겹침 0건.** 그리고 400개 전부 같은 단일 shadow 광원
`5d9093ae8f7f5f438eae4d867e63b073`를 가리켰다(컴포넌트의 baked 조합은 74종).

즉 RNM 라이트맵이 구운 광원 집합과 그림자를 저장한 광원이 원본에서 분리돼 있다. 그림자 광원
하나에만 `staticShadowChannel`을 배정하면 `Light_Manager.cpp:270`의
`record.flags[1] = light.staticShadowChannel != 0 ? ... : ...` 경로에서 채널이 일치하는 광원만
static shadow를 받으므로 이중 감쇠가 발생하지 않는다.

남은 확인 항목은 `5d9093ae...`가 이 Area의 어느 저작 광원인지 대응시켜 `lightChannel` 1~15 중
하나를 배정하는 것이다. 이 Area는 `sourceLights`/`lights` pair를 선언하지 않으므로
(`.md/TEAM/AREA_DATA_LAYER_GUIDE.md`의 선택 레이어) 광원 문서 신설 여부가 선행 결정이다.

#### 정리

ShadowMap2D는 **부분 복원**이다. 계약·런타임·셰이더는 완비, 원본 메타데이터는 전부 읽힘,
아틀라스는 게이트 통과, 이중 조명 방지 설계는 확인됨. 미완은 `pf_g8` payload 추출과
`lightGuid` → `lightChannel` 배정이다. 이 항목을 제외한 채 B4를 완료로 적지 않는다.

## 5. B5 — 테스트

신규 `Tools/MapPipeline/test_build_map_rnm_variant_set.py` **14개 전부 통과**.

| 지시 항목 | 테스트 |
|---|---|
| 정상 multi-OVR | `test_distinct_ovr_assets_keep_their_own_atlas_pair` — OVR 접미사만 다른 두 자산이 쌍을 섞지 않음 |
| multi-RNM 쌍 | `test_multiple_atlas_pairs_produce_one_variant_each`, `test_variant_ids_are_stable_and_pair_specific` |
| 다중 슬롯 | `test_every_slot_of_a_multi_slot_asset_is_carried_into_the_variant` |
| 컴포넌트 누락 | `test_placement_without_a_component_record_gets_no_lighting`, `test_source_absent_placement_is_never_given_a_pair`, `test_validator_rejects_rnm_on_a_source_absent_placement` |
| 중복 ID | `test_validator_rejects_a_duplicate_source_placement_id` |
| 잘못된 텍스처 경로 | `test_validator_rejects_a_bad_lightmap_path`(`..` 탈출 / 절대 경로 / 비-DDS), `test_validator_reports_a_missing_lightmap_file` |
| 중간 실패 보존 | `test_asset_without_a_material_row_is_excluded_and_others_survive`, `test_variant_base_missing_from_catalog_fails_without_partial_output`, `test_input_documents_are_not_mutated` |
| 경계 | `test_validator_rejects_out_of_atlas_coordinates` |

기존 회귀 (내 변경 이후 실행):

| 스위트 | 결과 |
|---|---|
| `test_extract_ue3_texture_mips` | 6 OK |
| `test_build_source_map_material_inputs` | 8 OK |
| `test_build_source_map_materials` | 13 OK |
| `test_extract_source_map_component_lighting` | 10 OK |
| `test_map_surface_depth_contract` | OK |
| `test_camera_only_publish` | OK |
| `test_build_map_rnm_variant_set` (신규) | 14 OK |

## 6. 108개 라이트맵 원본 체인 배치

재산출이 요구하는 108개 전부를 넓힌 게이트로 복구했다
(`out/MaharakaRebuild_B/b4_lightmap_batch.json`, 약 520초).

| 지표 | 값 |
|---|---|
| 요청 | 108 |
| `recovered` | **108** (실패 0) |
| 클래스 | `lightmaptexture2d` 108 |
| fourCC | DXT1 108 |
| 원본 native mip 개수 일치 | **108 / 108** |
| 레벨별 크기 일치 | **108 / 108** |
| 복구본 합계 | 11.1 MB |

복구 mip 단수 분포: 4단 4개 / 6단 14개 / 7단 6개 / 8단 16개 / 9단 36개 / 10단 10개 / 11단 22개.
크기는 1024×1024 4개, 1024×512 18개, 256×256 24개 등으로 원본 아틀라스 크기를 그대로 따른다.

설치본은 108개 모두 `mip 1`이므로 이 배치는 **mip 절단을 원본 체인으로 되돌린다**. 다만 복구본의
파일명이 설치본과 같아 설치는 교체가 되므로, 실제 교체는 C단계가 백업·원자 교체·목록 보고와 함께
수행한다. 이 단계에서는 설치하지 않았다.

## 7. 추가한 Resources 상대 경로

**추가만 했다.** 삭제·수정·mirror·prune은 없다. 기존 PNG 21개는 전부 그 자리에 있다.

| 텍스처 | 설치명 | 폴더 수 |
|---|---|---|
| `bg_ocn_etc_floor01_n_lnh` | `57238c312a8a_bg_ocn_etc_floor01_n_lnh.dds` | 1 |
| `bg_ocn_etc_floor01a_n_lnh` | `6111f2e97788_bg_ocn_etc_floor01a_n_lnh.dds` | 2 |
| `bg_lut_lucastle_houseinfloor01_n_ysi` | `8a26b1d07f92_bg_lut_lucastle_houseinfloor01_n_ysi.dds` | 12 |
| `bg_rad_abrelshud_floor18_n_ksr` | `7d321264f40e_bg_rad_abrelshud_floor18_n_ksr.dds` | 6 |

경로 형태는 `Map/LV_OCN_EVENTIS_MHP/<자산 폴더>/textures/<파일명>`이다. 전체 21개 목록은
`out/MaharakaRebuild_B/b1_dds_install.receipt.json`의 `installedFiles`에 있다.

조명용 Resources 추가는 **없다**. 필요한 아틀라스 108개가 이미 설치돼 있다.

## 8. 변경한 저장소 파일

| 파일 | 변경 | 백업 |
|---|---|---|
| `Tools/LevelPlacementExtractor/extract_ue3_texture_mips.py` | 클래스 게이트를 Texture2D 파생 3종 멤버십으로 확대 (+355 bytes) | `out/MaharakaRebuild_B/backup/extract_ue3_texture_mips.py.before-b4-class-gate` |
| `Tools/MapPipeline/build_map_rnm_variant_set.py` | 신규 | — |
| `Tools/MapPipeline/test_build_map_rnm_variant_set.py` | 신규 | — |
| `.md/GB/09-27/2026-09-27_MAHARAKA_REBUILD_B_RNM_VARIANTS_RESULT.md` | 이 문서 | — |

`Data/**`, `Client/Bin/DataFiles/**`, `Data/Rendering/**`, `Client/Bin/ShaderFiles/**`,
`Renderer.*`, `GameInstance.*`, `UI_Sprite.cpp`, `.vcxproj`/`.filters`는 건드리지 않았다.
publisher도 실행하지 않았다. `materialComplete`·`originalVisualFidelityVerified`를 손으로 바꾸지
않았다. 공유 staging(`RebuildA_scratch`의 manifest·mip-catalog)도 원본을 수정하지 않고
갱신본을 별도 경로에 썼다.

## 9. C단계로 넘기는 것

1. **행 단위 병합** — canon 전용 물 10행 보존, 교집합 상이 142행 교체, 후보 전용 36행 추가.
   후보로 canon을 통째 대체하면 물이 사라진다.
2. **맵 담당자 승인** — catalog 446행 추가와 placement 1,470건 재지정. 두 파일은 Git LFS이고
   Area당 담당 1명 규칙이 있다.
3. **라이트맵 mip 교체 판단** — 108개 원본 체인 복구본이 준비됐지만 파일명이 설치본과 같아
   교체가 된다. 교체하면 백업·원자 교체·목록 보고가 필요하다.
4. **`Publish-MapAuthoring.ps1` 한 트랜잭션 게시** 후 사용자 화면 확인.

## 10. 최종 재확인

이 절은 실제 산출물을 다시 읽어 위 기술과 대조한 결과다.

| 주장 | 확인 방법 | 결과 |
|---|---|---|
| A단계 호출 정확 재현 | `adapter_baseline/inputs.json`과 `adapter_stage/inputs.json` 슬롯·텍스처 본문 직렬화 비교 | 바이트 동일 |
| 슬롯 330 → 355 | `compiled/b1.mapmaterials.json` 행 수와 `after2.mapmaterials.json` 대조 | 355 / 추가 25 / 제거 0 / 값 변경 0 |
| 무대 3자산 행 존재 | 세 assetId로 행 조회 | 각 1행, 색 G03 실측과 1e-9 이내 일치 |
| Resources 추가만 | 설치 전 충돌 검사 + 설치 후 sha 재검증 + PNG 실재 확인 | 충돌 0 / 21개 해시 일치 / PNG 21개 보존 |
| canon 231 인스턴스 유효 | 게시 placement와 assetId 동일성 전수 대조 | 위반 0 |
| 아틀라스 108개 설치 | 파일명 규칙으로 디스크 확인 | 108/108 존재 |
| B3 publisher 규칙 통과 | 읽어낸 규칙 전부 코드 재검사 | 위반 0, RNM 누출 0 |
| 도구 = 스크립트 | 세 문서 sha256 비교 | 전부 동일 |
| 라이트맵 원본 mip 존재 | native mip 표 직접 파싱 | 256² 9단 / 1024×512 11단 |
| 설치 라이트맵 mip 없음 | 137개 DDS 헤더 `dwMipMapCount` 판독 | 전부 1 |
| 기존 회귀 유지 | 7스위트 실행 | 전부 OK |

**사용자 화면 확인은 하지 않았다.** 에이전트는 Client를 실행·조작하지 않았고 캡처도 만들지
않았다. `manual first pixel`, `eye smoke`, `visual PASS`, occurrence 승인은 이 문서에 없다.
무대가 실제로 보이는지는 C단계 게시 후 사용자가 직접 판정한다.

B4 ShadowMap2D는 **부분**이다. 완료로 적지 않았다.
