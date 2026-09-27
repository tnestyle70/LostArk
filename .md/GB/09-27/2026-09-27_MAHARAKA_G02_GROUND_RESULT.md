# 2026-09-27 마하라카 G02 — 비어 보이는 지면 분류 결과

대상: `LV_OCN_EVENTIS_MHP`. 설계서 `2026-09-27_MAHARAKA_VISUAL_PARITY_EXECUTION_PLAN.md` G02.

**결론부터: G02 범주의 결함은 발견되지 않았다. 지면은 누락되지 않았고, 파란 영역은 원본 그대로 배치된
물이다. 실제 원인은 물 재질의 표현(G06)과 지형 색(G05)이며, 이번 단계에서는 아무것도 수정하지 않았다.**

설계서 G02-4는 "무작정 전체 바다를 내리거나 투명하게 하지 않는다"고 명시한다. 수면 높이나 불투명도를
여기서 바꾸는 것이 바로 그 금지 행위이므로 하지 않았다.

## 0. 시작 상태 (재측정)

`Audit-MaharakaVisualInputs.ps1` 재실행 결과가 설계서 실측치와 **완전히 일치**했다.

| 항목 | 값 |
|---|---|
| catalog 자산 | 408 |
| placement | 4,669 |
| material 행 | 329 |
| 무대 placement / material 행 | 18 / **0** |
| bakedLighting / placementLighting | 0 / 0 |
| meshNamesWithMultipleVariants | 49 |
| 이전 RNM 후보 base 불일치 / 미게시 | 955 / 35 |

저장소 `codex/main-ship-maharaka-0926`, HEAD `a84bbcd45ff995c70bf847c683f1d159cf1821a6`.
`devenv.exe`(pid 26212) 실행 중이므로 빌드하지 않았다.

## 1. 관찰

현재 스크린샷에서 섬 중앙부 넓은 영역이 진한 파란색이다. 정원·길·배 옆 바닥 주변이 물에 잠긴 것처럼
보이고, 사용자는 이를 "지면이 비어 보인다"로 보고했다.

원본 이미지는 홍보 합성본이므로 픽셀 비교 기준으로 쓰지 않았다. 다만 같은 위치에 원본도 물이 있고,
그 물이 **밝은 청록색이며 바닥이 비쳐 보인다**는 점은 구도와 무관하게 읽을 수 있다.

## 2. 확인한 원인 — 네 가지 후보를 각각 판정

### 후보 A. 지형 누락 / 홀 → **기각**

원본 landscape는 46개다. `Landscape2/maharaka_landscape_manifest.json`의 각 component에 대해
world bounds = `geometry.boundsMin/Max` + `position`으로 환산하면 두 구역으로 갈린다.

| 구역 | sectionBase Y | engine Z | component |
|---|---|---|---|
| 섬 | 1426 ~ 1612 | −912.6 ~ −1071.4 | **16** |
| 다른 구역 | 0 ~ 310 | 0 ~ −238.1 | 30 |

무대가 `(75.06, 22.40, −984.33)`으로 Z −984에 있으므로 섬은 **16개 구역**이 맞다. 설계서의 16/30 분할을
독립적으로 재현했다.

게시된 `LV_OCN_EVENTIS_MHP.mapassets`/`.mapplacements`의 `LAND01_LC_*` 자산은 **정확히 16개**이고,
manifest의 섬 16개 집합과 **완전히 동일**하다(차집합 0). 16개 placement 모두 scale `(1,1,1)`,
quaternion `(0,0,0,1)`, position y `0`, 가시성 플래그 `1`이다.

`holeQuadCount`가 0이 아닌 component는 `LC_01145`(211), `LC_01144`(284), `LC_01148`(16),
`LC_01149`(15) 네 개인데 **네 개 모두 다른 구역**이다. 섬 16개는 전부 `holeQuadCount = 0`이다.

→ 섬 지형은 빠짐없이 게시되어 있고 구멍도 없다.

### 후보 B. 뒤집힌 면 / 음수 scale cull → **기각**

게시 placement 4,669개 중 음수 축이 홀수여서 determinant가 뒤집힌 것이 **1,109개**다. 적은 수가 아니라
실제로 확인했다.

- `Client/Private/MapAssetObject.cpp:518` — `m_bMirrored = signedScale.x * signedScale.y * signedScale.z < 0.f;`
- `Client/Private/MapAssetRenderUtils.cpp:718~729` — `Select_Pass()`가 `mirrored`이고 `TWO_SIDED`가
  아니면 `CULL_BACK` ↔ `CULL_FRONT`를 교환한다.

→ 런타임이 determinant 부호를 정확히 보상한다. 음수 scale 때문에 지면이 사라지는 경로는 없다.

### 후보 C. 잘못된 숨김 상태 → **기각**

가시성 플래그가 `1`이 아닌 placement는 4,669개 중 **정확히 1개**다.

```
LV_OCN_EVENTIS_MHP_SL01:export:5942
MAP_3954EA2AC0EC_LV_COMMON_MESH_CUL_BOX_7
pos (103.86, 27.63, -962.40)  scale (0.8, -0.8, 0.8)  flag 0
```

보이지 않는 컬링 헬퍼 박스이므로 숨겨진 것이 정상이다.

### 후보 D. 물이 지면을 덮음 → **확인됨. 단, 배치는 원본 그대로다**

지배적인 수면은 `MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512_OVR_3CB26CF55D22`이다.

| 항목 | 값 |
|---|---|
| sourcePlacementId | `LV_OCN_EVENTIS_MHP_SL01:export:5210` |
| source class | `interpactor` (`interpactor_21`) |
| 게시 position | `(75.0706641, 19.6299988, -993.332578)` |
| 게시 scale | `(100, 100, 100)` |
| mesh bounds | X/Z `−2.56 ~ +2.56` (5.12 × 5.12 m), 2,116 verts / 4,050 tris, **구멍 없는 solid plane** |

원본 대조 (`Placements/LV_OCN_EVENTIS_MHP_SL01.placements.json`):

- `actor.drawScale = 100.0`, `actor.drawScale3D = (1,1,1)` → 게시 scale 100과 일치
- `actor.location.z = 1962.9998779296875` cm → `/100 = 19.62999878` m → 게시 y와 일치
- `sourceVisibility.visible = true` (`actorHidden`/`componentHiddenGame` 모두 zero-default false)

**즉 이 평면은 추출·변환 결함이 아니라 원본이 지정한 그대로다.**

유효 footprint는 2.56 × 100 = 256 m 반경, 즉 **512 × 512 m**:

```
X −180.9 ~ 331.1      Z −1249.3 ~ −737.3      Y 19.63 (평면)
섬:  X 0 ~ 158.7       Z −1071.4 ~ −912.6
```

섬 전체가 이 평면 아래에 들어간다. 섬 지형 높이와 대조하면:

| 판정 | component 수 | 예 |
|---|---|---|
| 전부 수면 아래 | **3** | `LC_01164` 14.3~18.2, `LC_01181` 14.2~16.2, `LC_01168` 14.9~17.8 |
| 일부 수면 아래 | **13** | `LC_01177` 14.7~22.1 등 |
| 전부 수면 위 | 0 | — |

→ y 19.63 m를 넘는 지형만 마른 땅으로 보인다. 이것이 "정원·길 주변이 물"의 기하학적 원인이다.

랜드마크 높이가 이를 뒷받침한다.

| 대상 | 위치 | y |
|---|---|---|
| 수면 | (75.07, −993.33) | **19.63** |
| POOL01 (무대 아래) | (75.05, −984.32) | 19.92 (수면 +0.29) |
| 정원 20배치 centroid | (69.3, −1008.2) | 20.46 (수면 +0.83) |
| 파라솔 17배치 | (84.6, −997.7) | 19.4 ~ 22.2 (**일부는 수면 아래**) |
| 무대 | (75.06, −984.33) | 22.40 |

정원은 수면보다 겨우 0.8 m 높다. 그래서 정원 자체는 드러나지만 그 **주변 지면은 잠긴다.** 사용자가
"정원 안팎이 물"이라고 관찰한 것과 정확히 일치한다.

## 3. 그래서 진짜 원인은 무엇인가

지면은 있다. 물도 원본 위치·높이 그대로다. 남는 차이는 **물이 어떻게 보이는가**다.

그 평면의 material 행(`Data/Maps/Authoring/.../LV_OCN_EVENTIS_MHP.mapmaterials.json`)은
`family = source.map.water-42.v1`, `renderMode = translucent`이고 다음 값을 갖는다.

```
fresnel_color     [0.0, 0.111302, 0.735955, 1.0]
fresnel_intensity 1200.0
fresnel_power     5.0
sky_color         [0.0, 0.260838, 0.88947, 1.0]
sky_intensity     1.5
opacity           2.0
opacity_power     0.6
```

포화된 파란 fresnel 항에 intensity 1200이 곱해지면 출력이 그 항에 지배되어 불투명한 진청색 판처럼
읽힌다. 원본에서 같은 수영장이 밝은 청록이고 바닥이 비치는 것과 대비된다.

**단, 이 값들이 원본 MIC 값인지 변환 과정의 오류인지는 이번 단계에서 검증하지 않았다.** 설계서 G06-1/2가
source MIC와 static switch, VS/PS permutation, depth fade를 대조하도록 지정한 범위다. 근거 없이
`opacity`나 `fresnel_intensity`를 조정하는 것은 설계서가 금지한 "무작정 투명하게"에 해당하므로 하지 않았다.

## 4. 바꾼 파일

**없다.**

`Data/**`, `Client/Bin/DataFiles/**`, `Client/Bin/Resources/**`, 코드, publisher 실행 모두 0건이다.
이번 단계 산출물은 아래 두 개뿐이며 전부 `out/` 아래다.

- `out/MaharakaVisualParity_G02/build_analysis.py` (읽기 전용 분석 스크립트)
- `out/MaharakaVisualParity_G02/ground-classification.json` (판정 결과)
- `out/MaharakaVisualParity_G02/audit_start.json` (시작 상태 감사 출력)

## 5. 실제 consumer

이번 단계는 판정만 했으므로 새로 연결한 consumer가 없다. 판정에 사용한 런타임 소비 경로는 다음과 같다.

- 지형/배치: `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapassets` · `.mapplacements`
  → `CMapPlacementRuntime` → `CMapAssetObject` → `CModel`
- cull 판정: `CMapAssetObject::m_bMirrored` → `CMapAssetRenderUtils::Select_Pass`
- 물 재질: `mapmaterials.json` → `CMapAssetObject::Get_MaterialRenderProfile` (surface override가
  catalog renderMode보다 우선)

## 6. 자동 검사 결과

| 검사 | 결과 |
|---|---|
| 시작 상태 감사 재현 | 설계서 수치와 **완전 일치** |
| 섬 landscape 게시 집합 == manifest 섬 집합 | **True** (차집합 0) |
| 섬 component hole quad | **0 / 16** |
| 다른 구역 hole quad 보유 component | 4개 (섬 아님) |
| determinant 뒤집힌 placement | 1,109 — 런타임 보상 코드 확인 |
| 가시성 플래그 != 1 | 1개 (컬링 헬퍼, 정상) |
| 거대 수면 transform vs 원본 | drawScale·location 모두 **일치** |
| 수면 아래 섬 component | 전부 3 / 일부 13 / 위 0 |

## 7. 사용자 미확인

이 단계는 화면에 나타나는 변경을 만들지 않았으므로 **확인을 요청할 항목이 없다.** 현재 화면은 작업 전과
동일하다. 파란 영역이 실제로 물 재질 때문인지는 G06 수정 후에 사용자가 판정한다.

## 8. source-exact / project-tuned 구분

**source-exact (원본에서 직접 읽음)**

- 섬 landscape 16개의 world bounds, hole quad 0
- 거대 수면의 `drawScale 100.0`, `drawScale3D (1,1,1)`, `location.z 1962.9998779296875` cm
- 수면 mesh bounds ±2.56 m, 4,050 삼각형, 구멍 없음
- `sourceVisibility.visible = true`
- 물 material 행의 파라미터 값 (원본 MIC 대조는 **미수행** — G06 범위)

**project-tuned**

- 이번 단계에서 새로 정한 값 **없음**

## 9. 다음 항목

| 넘길 곳 | 내용 |
|---|---|
| **G06 (물)** | 거대 수면의 `fresnel_intensity 1200.0` / `fresnel_color` / `sky_color` / `opacity 2.0`가 원본 MIC 값인지 대조. 원본 수영장이 바닥이 비치는 이유(depth fade, refraction, scene texture 입력)를 확인. 수면 높이 19.63은 원본 값이므로 건드리지 말 것 |
| **G03 / G04 (재질 slot)** | 거대 수면 행의 `materialName`이 `SLOT_000_dummy_material_0`이다. 실제 source slot 이름으로 해석되지 않았다. 무대 두 모델의 slot 문제와 같은 계열일 수 있음 |
| **G05 (지형 색)** | 전경 노랑/초록 얼룩은 placement가 아니라 landscape layer 색이다. G02 범주가 아님 |
| **G08 (무대/Matinee)** | 거대 수면은 `interpactor`다. Matinee가 런타임에 이 평면을 움직이거나 토글하는지 확인 필요. 확인 전에는 정적 배치로 단정하지 말 것 |

### 미해결로 남긴 것

- `(88.70, 42.72, −1047.51)`의 `LV_MODULE_WATER02_512_OVR_BDD0873DA029`(scale 6.28×1×3, 약 32×15 m)는
  지형 최고점 22.1 m보다 20 m 높은 곳에 떠 있다. 원본 `staticmeshcollectionactor` component transform과
  **일치함을 확인**했으므로 배치 오류는 아니다. 사용자가 보고한 "하늘 쪽 얇은 갈색 판"이 이것인지는
  화면 확인이 필요하며, 원본에서 어떤 역할인지(폭포 상단·슬라이드 저수부 등) 미확인이다.
- 물 family 10행 중 3행이 `BG_FAT_STONE_ROCK01/02/03`이다. 바위가 water-42 재질을 갖는 이유는
  확인하지 않았다.
