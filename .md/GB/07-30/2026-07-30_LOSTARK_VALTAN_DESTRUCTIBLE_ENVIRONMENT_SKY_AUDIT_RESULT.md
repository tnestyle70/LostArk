# 발탄 파괴 구조물·배경 텍스처·점프 구간 하늘 전수 조사 결과

- 작성일: 2026-07-30
- 대상 레벨: `LV_LUT_HEARTRB_ED_PS`, `LV_LUT_HEARTRB_ED_SL00`~`SL05`
- 조사 범위: DeployData 파괴 오브젝트, 쇠사슬 재질, 흰색 유동 배경, 발탄 점프 구간의 붉은 블랙홀형 하늘
- 기준 문서: `2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md`, `gotchas.md`

## 1. 결론

1. 파괴 가능한 구조물은 정적 레벨의 돌 메시 몇 개가 아니라 `DeployData_37051.loa`가 소유하는 별도 배치 집합이다.
   - 메시에 의해 확인되는 주요 파괴 클래스는 12종, 배치는 113개다.
   - 원형 전투장 반경 2,000 cm 안에는 모델 배치 85개와 모델 없는 상태 레코드 1개가 있다.
   - DB의 상태·파괴 규칙을 넓게 적용하면 전체 상태성 레코드는 121개다. 이 값은 113개와 정의가 다르므로 서로 대체해서 사용하면 안 된다.
2. 쇠사슬은 원본 텍스처가 없는 것이 아니다. 정확한 D/N/S 텍스처가 존재하지만 긴 추출 경로에서 UModel의 `.props.txt` 쓰기가 실패해 현행 런타임 패키지에 텍스처 0개로 들어갔다.
3. 화면 아래의 은은한 흰색 유동 배경은 `bg_lut_zamount_cloudplane_sm_old` 2장과 다수의 cloud/mist ParticleSystem을 합성한 결과다. CloudPlane의 정확한 텍스처와 UV 이동값을 찾았다.
4. 발탄 점프 구간의 붉은 블랙홀은 단일 스카이박스가 아니다. 다음 세 층으로 구성된 동적 하늘 연출로 보는 것이 현재 증거와 가장 잘 맞는다.
   - 기본 하늘 구체: `sky_mirror_sm`
   - 중앙 청백색 공간 구멍: `par_d_spacehole_03`
   - 붉은 중심·링·전기·구름 층: `par_d_hugechaosgate_01`
5. 현재 다른 작업 세션이 `Shader_Deferred.hlsl`, `Shader_VtxMeshBinary.hlsl`, `Renderer.cpp`, `MapAssetObject.cpp`, 맵 배치 파일을 수정하고 있다. 충돌을 피하기 위해 이번 조사에서는 해당 런타임 파일을 수정하지 않았다. 적용 가능한 원본과 정확한 파라미터, 구현 순서까지 이 문서에 확정했다.

## 2. 증거 등급

| 등급 | 의미 | 이번 조사에서의 사용 |
|---|---|---|
| `DEPLOYDATA_EXACT` | `DeployData.loa`와 `EFTable_Prop.db`로 배치·상태를 직접 확인 | 파괴 구조물 169개 레코드와 113개 주요 모델 배치 |
| `UE3_LEVEL_EXACT` | HeartRB 레벨 export에서 실제 Actor/Component를 확인 | 쇠사슬, CloudPlane, sky mirror, spacehole, hugechaosgate |
| `MATERIAL_EXACT` | MaterialInstance와 부모 Material의 파라미터·텍스처를 직접 확인 | 쇠사슬 D/N/S, CloudPlane UV, 붉은 chaos gate |
| `VIDEO_MATCH` | 원작 영상과 색·구도·운동이 일치 | 흰색 유동 배경과 붉은 블랙홀의 최종 화면 대응 |
| `PROVISIONAL` | 라이브 원본에는 있으나 HeartRB 직렬화 참조가 없음 | `BG_SHS_RCARENA_SKY01_SM_OLD` 등 후보 |

`UE3_LEVEL_EXACT`와 `MATERIAL_EXACT`가 없는 후보를 원작 정확 에셋으로 승격하지 않는다.

## 3. 파괴 가능한 구조물

### 3.1 DeployData 전체 현황

| 항목 | 수량 |
|---|---:|
| DeployData 레코드 | 169 |
| 고유 Deploy ID | 169 |
| 고유 Prop 정의 | 27 |
| 모델 경로가 해석된 레코드 | 125 |
| 상태·파괴 레코드(넓은 DB 규칙) | 121 |
| 게임플레이 볼륨·블로커 | 25 |
| 비시각 레코드 | 14 |
| 일반 시각 Prop | 9 |
| 전투장 반경 2,000 cm 안의 레코드 | 112 |
| 전투장 안의 시각 모델 | 85 |
| 전투장 안의 상태·파괴 레코드 | 86 |

전투장 중심은 DeployData 실측 기본값 `(15627.9, 12197.7, 2324.2)`를 사용했다.

### 3.2 주요 파괴 모델 12종

| 모델 | Prop ID | 배치 수 | 형태 | 전투장/외곽 |
|---|---:|---:|---|---|
| `ITR_00323` | 375334 | 10 | Skeletal | 외곽 10 |
| `ITR_02306` | 375325 | 5 | Static + Fractured | 전투장 5 |
| `ITR_02307` | 375326 | 4 | Static + Fractured | 전투장 4 |
| `ITR_02308` | 375327 | 6 | Static + Fractured | 전투장 6 |
| `ITR_02309` | 375328 | 2 | Static + Fractured | 전투장 2 |
| `ITR_02310` | 375329 | 3 | Static + Fractured | 전투장 3 |
| `ITR_02311` | 375330 | 2 | Static + Fractured | 전투장 2 |
| `ITR_02315` | 375331 | 28 | Static + Fractured | 전투장 28 |
| `ITR_02316` | 375332 | 27 | Static + Fractured | 전투장 27 |
| `ITR_02326` | 375333 | 8 | Skeletal | 전투장 8 |
| `ITR_02332` | 375337 | 13 | Static + Fractured | 외곽 13 |
| `ITR_02333` | 375338 | 5 | Static + Fractured | 외곽 5 |
| 합계 |  | **113** |  | 전투장 85 / 외곽 28 |

예를 들어 `ITR_02306`은 intact `itr_02306_sk`와 fractured `itr_02306_sk_fractured`가 모두 존재하며 `hitMeshType=1`이다. `ITR_02307`~`02316`, `02332`, `02333`도 동일 계열의 정적/파편 쌍으로 확인된다.

분류 테이블에는 있지만 이 레벨에 배치되지 않은 추가 후보도 있다.

- `ITR_02296`: Static + Fractured
- `ITR_02297`: Static + Fractured
- `ITR_02328`: Static intact
- `ITR_02652`: 모델 미해결
- `ITR_10131`: Static

이 다섯 종은 이번 HeartRB 재현의 배치 목록에 넣으면 안 된다.

### 3.3 113, 121, 86의 차이

- **113**: 메시와 파괴형 정의가 확인된 주요 12종의 모델 배치 합계다.
- **121**: DB의 상태성 규칙까지 포함한 전체 레코드 수다. 일반 상태 Prop과 모델 없는 레코드가 섞인다.
- **86**: 전투장 반경 안의 상태·파괴 레코드다. 모델 배치 85개와 모델 없는 상태 레코드 1개다.

모델 없는 전투장 상태 레코드는 Prop `375303`, 위치 `(14411.3887, 10956.6045, 2287.3833)`, DeployData ID `0x100000BE`다. 이 레코드를 억지로 빈 메시로 생성하지 말고 트리거·상태 데이터로 보존해야 한다.

### 3.4 런타임 저장 계약

정적 맵 placement와 DeployData placement를 합치지 않는다. Catalog는 생성 가능한 정의를, placement 문서는 인스턴스를 소유해야 한다.

필수 데이터는 다음과 같다.

- 안정적인 `assetId`, `placementId`, 원본 `deployId`, `propId`
- intact/fractured 또는 skeletal model 경로
- 위치·회전·스케일
- 초기 상태, 피격 타입, 파괴 후 상태
- 레이드 진행 상태 또는 트리거 바인딩

로드는 `parse -> validate -> stage -> commit` 순서를 지키고, 하나라도 실패하면 stage에서 만든 객체를 전부 rollback해야 한다. 신규 경로는 반드시 기존 `CModel -> CMaterial` 통합 경로를 확장한다.

## 4. 쇠사슬 재질 누락

### 4.1 정확 에셋과 배치

| 에셋 ID | UE3 오브젝트 | 배치 |
|---|---|---:|
| `MAP_2F7659F7259C_BG_FAT_KARLAJAVIL_CHAIN01A_SM` | `bg_fat_karlajavil_chain01a_sm` | SL00 1 + SL03 3 = 4 |
| `MAP_9ACE87B2E07E_BG_FAT_KARLAJAVIL_CHAIN01B_SM` | `bg_fat_karlajavil_chain01b_sm` | SL03 1 + SL04 8 = 9 |
| 합계 |  | **13** |

두 모델은 같은 MaterialInstance `bg_fat_karlajavil_elevator01a_mi_khb`를 사용한다. 부모는 `mastermaterial_bg.base.bg_base_opa`다.

정확한 재질 값은 다음과 같다.

- Diffuse: `bg_fat_karlajavil_elevator01a_d_khb`
- Normal: `bg_fat_karlajavil_elevator01a_n_khb`
- Specular: `bg_fat_karlajavil_elevator01a_s_khb`
- `specular_intensity = 1.25`
- `specular_power = 60`
- `diffuse_color = (1, 1, 1, 1)`

두 쇠사슬에서 추출한 D/N/S 파일의 SHA-256도 각각 일치한다. 서로 다른 임의 텍스처로 대체할 이유가 없다.

### 4.2 누락 원인

현재 런타임 디렉터리에는 두 모델 모두 `.WModel`만 있고 텍스처가 없다. 기존 cook receipt도 `textures=0`이다. 원인은 원본 부재가 아니라 긴 staging 경로다.

UModel 로그에는 MaterialInstance의 `.props.txt` 파일을 만들지 못했다는 `Error creating file`이 남아 있다. 그 뒤 파이프라인이 오류를 치명적으로 처리하지 않고 재질 슬롯 0개인 결과를 `reused/recovered`로 확정했다.

### 4.3 수정 방법

1. 추출 staging root를 짧은 고정 경로로 바꾸고, 긴 에셋명 대신 8~12자리 해시 폴더를 사용한다.
2. UModel 출력에 `Error creating file`이 있으면 성공 receipt를 쓰지 말고 전체 에셋을 실패 처리한다.
3. `textures=0`이면서 원본 MIC가 존재하는 기존 receipt를 무효화하고 재추출한다.
4. D/N/S를 기존 `CMaterial` 계약에 연결하고 atomic install한다.
5. 런타임이 별도 specular texture를 아직 지원하지 않는다면 기존 물리 재질 계약에 맞게 값을 소비한다. 이를 위해 두 번째 모델/재질 런타임을 만들면 안 된다.

## 5. 은은한 흰색 유동 배경

### 5.1 CloudPlane 정확 에셋

- 에셋: `MAP_3CC7E67937A0_BG_LUT_ZAMOUNT_CLOUDPLANE_SM_OLD`
- 오브젝트: `bg_lut_zamount_cloudplane_sm_old`
- 레벨: persistent PS
- 배치: 2개
- 형상: 4 vertex, 2 triangle, 10.24 m 평면
- MaterialInstance: `bg_lut_zamount_cloudplane_inst_02_old`
- 부모 Material: `skyplane_cinema_trn`
- Blend mode: `BLEND_Translucent`

핵심 텍스처는 다음과 같다.

| 역할 | 텍스처 |
|---|---|
| Cloud diffuse | `bg_lut_zamount_c_d3_old` |
| Normal | `bg_lut_zamount_c_n3_old` |
| Overlay/gradient | `bg_lut_zamount_untitled-1_old` |
| 부모 opacity fallback | `plane_cloudtex_01_d` |
| 부모 sky/floor 보조 | `floorsky_02`, `floorsky_03_n`, `t_tds_specular04` |

정확한 파라미터는 다음과 같다.

| 파라미터 | 값 |
|---|---:|
| `brightness` | 3 |
| `bumpoffset` | 0.09 |
| `normal_intensity` | 0.3 |
| `opacity` | 2 |
| `opacity_density` | 0.45 |
| `opacity_depthbias` | 2 |
| `opacity_power` | 5 |
| `rimlight_power` | 6 |
| UV tiling/panning | `(2, 2.5, 0.015, 0)` |
| ambient color | `(0.287282, 0.206264, 0.173427, 1)` |
| light color | `(1, 1, 1, 1)` |
| cloud color | `(1, 0.819871, 0.784538, 1)` |
| rim color | `(1, 0.702881, 0.303893, 1)` |

`bg_lut_zamount_c_d3_old`는 회백색의 조밀한 구름이며 `plane_cloudtex_01_d`는 밝은 청백색 구름이다. 원작 화면의 아래쪽 흰색 흐름과 직접 대응한다.

CloudPlane도 쇠사슬과 동일하게 긴 경로에서 `.props.txt` 생성이 실패했다. 현재 런타임에는 `.WModel`만 있고 텍스처가 0개다.

### 5.2 ParticleSystem 전수 집계

HeartRB PS와 SL00~SL05의 Emitter Actor를 전부 파싱했다.

| 레벨 | Emitter 수 |
|---|---:|
| PS | 20 |
| SL00 | 19 |
| SL01 | 64 |
| SL02 | 47 |
| SL03 | 68 |
| SL04 | 266 |
| SL05 | 25 |
| 합계 | **509** |

- Component 509개, 고유 ParticleSystem template 23종
- 파싱 오류 0개
- 주요 구름·안개 template 합계:
  - `par_c_ul_volumecloud_01`: 35
  - `par_d_cloud_001`: 14
  - `par_d_cloud_006`: 1
  - `par_d_cloud_031`: 15
  - `par_d_ocn_cloud_01`: 22
  - `par_l_groundmist_01`: 6
  - smoke 계열: volume 9, guardian 7, regular 8, thin 1, faten 3

따라서 원작의 흰 배경은 정적 diffuse 한 장이 아니라 **CloudPlane 2장 + translucent UV panning + cloud/mist particle layer**다. 현재 static-only map pipeline은 509개 emitter를 모두 제외하고 있으므로 모델에 diffuse만 붙여도 원작의 움직임은 재현되지 않는다.

### 5.3 필요한 엔진 확장

- `CMaterial`에 범용 translucent, opacity, UV tiling/panning, time 입력을 표현한다.
- CloudPlane을 기존 `CModel -> CMaterial` 경로로 렌더링한다.
- ParticleSystem 전체를 한 번에 임의 복제하기보다 영상에 직접 보이는 cloud/mist template부터 정확 데이터로 단계 적용한다.
- ImGui는 선택과 생성 명령만 전달하고 매 프레임 재질 파일을 읽거나 디코드하지 않는다.

## 6. 발탄 점프 구간의 붉은 블랙홀형 하늘

### 6.1 단일 스카이박스가 아닌 세 겹 구조

| 층 | 정확 에셋 | 배치·색 근거 | 판정 |
|---|---|---|---|
| 기본 하늘 | `sky_mirror_sm` | PS 1개, SCENE04A 동일 scene copy 1개; `lv_sky_0161_d`는 푸른 낮 하늘 | 기본 배경일 뿐 붉은 원인은 아님 |
| 중앙 공간 구멍 | `par_d_spacehole_03` | PS 중심축 상공, 청백색 emission과 cloud/noise material | 블랙홀 구조의 보조·기본 층 |
| 붉은 Chaos Gate | `par_d_hugechaosgate_01` | SL00 실제 Emitter, 붉은 중심 `(3, 0.1, 0.1, 3)`, ring/cloud/electric 12 emitter | 붉은 블랙홀의 가장 강한 정확 후보 |

### 6.2 기본 하늘 `sky_mirror_sm`

- 에셋: `MAP_EDDEDF2CF6A1_SKY_MIRROR_SM`
- 크기: 81.92 m 구체
- 텍스처: `lv_sky_0161_d`
- MIC: `sky_base_opa`, 부모 `sky_opa`
- `base_brightness = 0.7`

부모 Material에는 `cloud_01_n`, `cloud_01_m`, `floorcloud_01_d`, cloud speed와 bottom-cloud panning이 있지만 실제 base diffuse는 푸른 낮 하늘이다. 따라서 이 메시를 붉게 tint하는 것만으로 원작을 재현하는 접근은 근거가 약하다.

SCENE04A에도 동일 sky actor identity가 보이므로 scene layering을 구현할 때 PS 배치와 중복 생성하지 않아야 한다.

### 6.3 중앙 `par_d_spacehole_03`

- 경로: `bfx_high_01.valhatron.par_d_spacehole_03`
- 위치: `(15657.4375, 12197.296875, 6004.3125)`
- 회전: Roll 180°
- DrawScale: `0.12`, DrawScale3D `(3, 3, 4)`
- SpriteEmitter 5개, LOD 2개, subtree 25 objects, 파싱 오류 0
- Mesh: `bfx_sm_00.fm_d_supercell_07`
- 주요 텍스처: `fx_d_cloud_031`, `fx_d_atypical_019`, `fx_a_noise_012_n`, `fx_d_normal_085`, `fx_d_atypical_043`, `fx_a_cloud_017`

정확한 emission은 `(0.6, 2, 5, 10)` 계열의 청백색이며 cloud color도 `(0.7, 0.8, 1, 1)`이다. 위치와 형상은 중앙 공간 구멍과 일치하지만 붉은색의 직접 원인은 아니다.

### 6.4 붉은 `par_d_hugechaosgate_01`

- 경로: `bfx_high_00.chaosgate.par_d_hugechaosgate_01`
- 레벨: SL00 실제 Emitter export
- 위치: `(15842, 12512, 13840)`
- DrawScale: `0.5`
- SpriteEmitter 12개, LOD level 12개, subtree 100 objects, 파싱 오류 0
- Mesh: `bfx_sm_00.bfm_d_supercell_02`, `fx_sm_00.fm_d_electric_02`

주요 material은 다음과 같다.

- `bfx_d_hugechaosgate_00_01_tr`
- `bfx_d_pa_smoke_ulit_01_12_dt_tr`
- `bfx_d_pa_master_01_23_ad`
- `bfx_d_me_master_01_26_ad`
- ring master 3종
- `fx_d_pa_dark_05_09_dt_tr`

전용 material `bfx_d_hugechaosgate_00_01_tr`는 translucent이며 다음 값을 갖는다.

- `color_base = (1, 1, 1, 0.4)`
- `color_center = (3, 0.1, 0.1, 3)`
- 핵심 텍스처: `fx_k_cloudtilie_01`, `fx_d_cloud_031`, `fx_d_normal_070`
- 추가 ring/cloud/electric 텍스처: `fx_e_cloud_009`, `fx_d_shockwave_001_ycl`, `fx_m_wave_001_ycl`, `fx_k_electile_02` 등

즉, HeartRB 레벨의 정확 배치, 강한 붉은 중심 material, ring/cloud/electric emitter 조합이 모두 확인된다. 이 에셋이 점프 구간 붉은 블랙홀의 핵심 동적 레이어라는 판정은 높은 신뢰도다.

다만 이번 조사에서는 TriggerMapData/Matinee/Scene의 활성화 시점을 끝까지 연결하지 않았다. **에셋·배치·색은 exact, 점프 순간의 활성화 타이밍은 미확정**으로 구분한다.

### 6.5 추가 제공 화면과의 대조

사용자가 추가로 제공한 점프 구간 화면에는 다음 특징이 동시에 나타난다.

1. 하나의 매끈한 원판이 아니라 끊어진 적색·자홍색 발광 조각들이 원형으로 분포한다.
2. 원의 바깥과 안쪽에 서로 다른 속도로 흐르는 청회색·검은 구름층이 있다.
3. 중심축에는 청록색·흰색 점광원과 가느다란 수직 streak가 별도 층으로 보인다.
4. 큐브맵의 고정된 지평선이나 seam이 없고, 카메라가 중심으로 이동하면서 깊이감이 변하는 particle/mesh 합성 형태다.

이 특징은 `par_d_hugechaosgate_01`의 12개 emitter 및 ring/cloud/electric material 구성과 직접 대응한다.

| 제공 화면의 층 | 추출 데이터 대응 |
|---|---|
| 끊어진 적색 방사 링 | hugechaosgate ring master, shockwave/fluid texture, `color_center=(3, 0.1, 0.1, 3)` |
| 검고 회청색인 소용돌이 | `fx_d_cloud_031`, `fx_k_cloudtilie_01`, normal/noise layer |
| 중심의 청록·흰 점과 streak | `par_d_spacehole_03`의 청백색 emission 및 supercell/electric 계열 |
| 전체를 받치는 어두운 하늘 | `sky_mirror_sm`과 주변 cloud layer |

공간 배치도 이 해석을 지지한다. `par_d_hugechaosgate_01`과 `par_d_spacehole_03`의 XY 차이는 약 365 cm에 불과하고, 두 오브젝트 모두 전투장 중앙축 상공에 놓인다. Z는 각각 13,840과 6,004.3125로 분리되어 있어 점프/낙하 카메라가 같은 축의 서로 다른 깊이 레이어를 통과하는 구성과 맞는다.

따라서 추가 화면을 반영한 최종 판정은 다음과 같다.

- 붉은 방사형 블랙홀의 핵심: **`par_d_hugechaosgate_01` — 높은 신뢰도**
- 중심 청백색 깊이·입자 보조: **`par_d_spacehole_03` — 높은 신뢰도**
- `sky_mirror_sm` 단독 또는 붉은 cubemap 한 장이라는 가설: **기각**
- 남은 미확정 항목: TriggerMapData/Matinee가 이 두 ParticleSystem을 켜는 정확한 프레임과 페이드 시간

구현 시 화면을 한 장의 sky texture로 bake하지 말고, 최소한 붉은 ring/cloud 층과 청백색 center/streak 층을 분리해야 원작의 카메라 이동과 회전 운동을 유지할 수 있다.

### 6.6 제외 후보

`BG_SHS_RCARENA_SKY01_SM_OLD`는 환경 라이브러리에 존재하지만 HeartRB 레벨의 직렬화 참조가 없고 추출 메시에서 material도 확인되지 않았다. 따라서 이름이나 외형만으로 이 에셋을 정확 스카이박스로 배치하면 안 된다. 필요할 경우 `VIDEO_RECONSTRUCTED` 후보로만 다룬다.

## 7. 권장 적용 순서

1. DeployData importer를 별도 placement source로 추가한다.
   - 12종 모델 정의와 113개 배치를 안정 ID로 저장한다.
   - 전투장 85개 모델과 상태 레코드 1개를 우선 재현한다.
2. 추출기의 긴 경로 실패를 고친 뒤 쇠사슬 A/B와 CloudPlane의 기존 `textures=0` receipt를 폐기하고 재cook한다.
3. 기존 `CMaterial`에 translucent, opacity, UV panning을 범용 기능으로 추가하고 CloudPlane 2장을 적용한다.
4. cloud/mist ParticleSystem을 필요한 template부터 기존 렌더 경로에 통합한다.
5. `sky_mirror_sm -> par_d_spacehole_03 -> par_d_hugechaosgate_01` 순으로 하늘 레이어를 구성한다.
6. TriggerMapData/Matinee/레이드 상태를 조사해 jump state에서 붉은 레이어를 활성화한다.
7. 실행 레벨에서 intact 생성, 타격, fractured 전환, 저장, 재로드, 실패 시 기존 상태 보존까지 검증한다.

현재 수정 중인 다른 세션의 shader/renderer/map 파일이 안정된 뒤 그 변경 위에 합치는 것이 안전하다. 특히 CloudPlane과 crack emissive가 같은 셰이더 파일을 건드릴 가능성이 높으므로 선후 통합이 필요하다.

## 8. 검증 체크리스트

- [ ] DeployData 169개 레코드가 중복·누락 없이 parse되는가
- [ ] 주요 12종 113개 배치와 전투장 85개 모델 배치가 일치하는가
- [ ] 모델 없는 Prop 375303을 빈 시각 객체로 만들지 않는가
- [ ] 파괴 실패 시 stage 객체가 모두 rollback되는가
- [ ] 쇠사슬 13개 모두 D/N/S를 동일 원본으로 사용하는가
- [ ] UModel `Error creating file` 발생 시 cook이 성공 처리되지 않는가
- [ ] CloudPlane 2장이 translucent로 렌더되고 UV `(0.015, 0)` 방향으로 움직이는가
- [ ] 카메라 깊이·정렬에서 translucent sorting artifact가 없는가
- [ ] sky mirror가 scene copy 때문에 중복 생성되지 않는가
- [ ] spacehole은 청백색, hugechaosgate 중심은 붉은색으로 유지되는가
- [ ] jump state 전후로 붉은 레이어 활성화가 저장/재로드 후에도 일치하는가

## 9. 조사 산출물

재현 가능한 중간 결과는 다음 위치에 보관했다.

- 전체 DeployData audit: `.codex_tmp/valtan_env_texture_audit_20260730/deploydata_props_audit.json`
- 전투장 audit: `.codex_tmp/valtan_env_texture_audit_20260730/deploydata_props_arena_audit.json`
- 쇠사슬 원본: `.codex_tmp/valtan_env_texture_audit_20260730/chainA`, `chainB`
- CloudPlane 원본: `.codex_tmp/valtan_env_texture_audit_20260730/cloud`
- 기본 sky 원본: `.codex_tmp/valtan_env_texture_audit_20260730/skyMirror`
- Chaos Gate material: `.codex_tmp/valtan_env_texture_audit_20260730/chaosgate_materials`
- Space Hole material: `.codex_tmp/valtan_env_texture_audit_20260730/spacehole_materials`
- 파괴 Prop 미리보기: `.codex_tmp/valtan_prop_previews/Valtan_Destructible_Prop_ContactSheet.png`

이 경로들은 조사·검증용 임시 산출물이다. 실제 배포 리소스는 경로 실패 수정 후 원본에서 다시 cook하여 `Client/Bin/Resources/LostArk/Map/LV_LUT_HEARTRB_ED`에 atomic install해야 한다.

## 10. 이번 작업에서 변경하지 않은 범위

- Engine/Client C++ 및 HLSL 소스
- 현재 다른 세션이 수정 중인 map placement와 overlay 데이터
- 정식 runtime resource 디렉터리
- `.vcxproj`, `.vcxproj.filters`

이번 결과는 조사 보고서와 임시 검증 산출물만 추가한다. 따라서 별도 빌드는 수행하지 않았다.
