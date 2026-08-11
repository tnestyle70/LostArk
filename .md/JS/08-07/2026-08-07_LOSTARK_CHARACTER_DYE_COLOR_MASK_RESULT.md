# 캐릭터 의상 색 누락 — 원인 조사 결과

작성자: JS · 2026-08-07 · 브랜치 `feature/dimensionmaster-skill-detail`

차원술사와 도화가 옷이 흰색으로 나오고 창술사·워로드는 색이 나온다는 보고에서 시작했다.
결론은 텍스처 매핑 오류가 아니라 **염색(dye) 경로가 통째로 없는 것 하나**이며, 네 클래스 전부
같은 버그다. 클래스마다 티가 다르게 나는 이유까지 원본 머티리얼 파라미터로 확정했다.

이 문서는 조사 결과만 기록한다. 구현은 아직 하지 않았다.

> **2026-08-10 구현 완료.** WMA3 포맷 + wmodel 패치 도구 + 셰이더 합성으로 닫혔다.
> `../08-10/2026-08-10_LOSTARK_CHARACTER_DYE_RUNTIME_RESULT.md` 참조. 6절의 미확인
> 두 항목도 그 문서에서 해소됐다(차원술사 파라미터 추출, 합성식은 lerp-곱 근사 채택).

## 1. 원본 머티리얼이 하는 일

로스트아크 캐릭터 파츠는 `_d`에 색을 굽지 않고 **색 영역 마스크 + 영역별 틴트**로 칠한다.
umodel이 뽑아 둔 `MaterialInstanceConstant` 덤프에 계약이 그대로 있다.

```text
_export_sdm_psk/PC_SDM_00/MaterialInstanceConstant/pc_sdm_00_upper_mi.props.txt
```

```text
Parent = MaterialInstanceConstant'mastermaterial_ch.pc.parts.sp_parts_msk_high'

texture_diffuse        = pc_sdm_00_upper_d
texture_normal         = pc_sdm_00_upper_n
texture_specular       = pc_sdm_00_upper_s
texture_color_fx_skin  = pc_sdm_00_upper_cm      색 영역 마스크
texture_mask_variation = pc_sdm_00_upper_m

diffusecolor   = (1,      1,      1,      1     )
diffusecolor_a = (0.7528, 0.1380, 0.2088, 0.7640)
diffusecolor_b = (0.1742, 0.1742, 0.1742, 0.3596)
diffusecolor_c = (0.7022, 0.5258, 0.5010, 0.5281)
mask_variation_visible = (0, 0, 0, 1)
1.use_dyeing_sp        = 0
```

`_cm`의 채널이 염색 영역을 고르고 `diffusecolor_a/b/c`가 그 영역의 색이며 `diffusecolor`가
전체 곱이다. `_m`은 variation 마스크인데 `mask_variation_visible = 0`이라 이 변종에서는 꺼져 있다.

## 2. 우리 런타임에는 이 경로가 세 군데 다 없다

| 계층 | 현재 상태 |
|---|---|
| 추출물 | `Client/Bin/Resources/Character/*/textures`에 `_cm`이 있는 클래스는 차원술사뿐(7개). 도화가·창술사·워로드·건슬링어·슬레이어는 0개 |
| `.wmodel` 머티리얼 | 실린 경로는 `_d/_n/_s` 또는 `_d/_n/_orm`뿐. `_cm` 항목 자체가 없다 |
| 자료 구조 | `MODEL_MATERIAL_DATA`(ModelAssetData.h:11-19)와 `MATERIAL_ENTRY_V2`(WFormatTypes.h:113-127) 모두 색 마스크 슬롯이 없다 |
| 머티리얼 | `CMaterial::Initialize`(Material.cpp:295-328)가 바인딩하는 9슬롯에 색 마스크가 없다 |
| 셰이더 | `Shader_VtxAnimMeshBinary.hlsl:4-5`가 `g_DiffuseTexture`, `g_NormalTexture` 둘만 선언한다 |

즉 차원술사는 `_cm`을 뽑아 놓고도 쓰지 않고 있다. 나머지는 뽑지도 않았다.

## 3. 클래스별 실측과 원본 파라미터 대조

`_d`의 픽셀 통계는 `Client/Bin/Resources/Character/*/textures`의 TGA를 직접 디코드해 계산했다.
채도는 `max(RGB) - min(RGB)`이고 유채색은 그 값이 25 이상인 픽셀이다.

| 클래스 | `_d` 평균 RGB | sat<10 | 유채색 | 원본 틴트 | 화면 |
|---|---|---|---|---|---|
| 차원술사 upper | (199,199,199) | **100.0%** | **0%** | 미확인 | 완전 흰색 |
| 차원술사 lower | (205,205,205) | **100.0%** | **0%** | 미확인 | 완전 흰색 |
| 도화가 upper | (155,158,160) | 91.9% | 7.5% (청록) | **있음** | 희끄무레 |
| 도화가 lower | (142,137,136) | 75.4% | 3.8% | **있음** | 희끄무레 |
| 창술사 upper | (99,96,95) | 89.8% | 4.0% | **있음** | 어두워서 그럴싸 |
| 창술사 shoulder | — | 65.7% | 1.8% (보라) | **있음** | 어두워서 그럴싸 |
| 워로드 upper | (54,46,44) | 70.4% | 22.9% (적갈) | 사실상 없음 | 정상 |

원본 틴트 판정 근거는 각 파츠의 `diffusecolor*` 값이다.

```text
도화가 pc_sdm_00_upper_mi
  diffusecolor_a = (0.7528, 0.1380, 0.2088)   붉은색
  diffusecolor_b = (0.1742, 0.1742, 0.1742)   짙은 회색
  diffusecolor_c = (0.7022, 0.5258, 0.5010)   밝은 살구

창술사 pc_flm_00-1_upper_mi                    우리가 쓰는 변종
  diffusecolor   = (0.9627, 0.8619, 1.0   )   연보라 전체 곱
  diffusecolor_a = (0.3425, 0.0681, 0.1375)   짙은 자주
  diffusecolor_b = (0.7514, 0.1507, 0.4147)   마젠타

워로드 pc_wgl_00_upper_mi
  diffusecolor   = (1, 1, 1)
  diffusecolor_a = (1, 1, 1)
  diffusecolor_b = (0.9131, 1.0, 0.8630, A=0.3333)   거의 항등
```

**워로드만 틴트가 항등에 가깝다.** 그래서 diffuse만 그려도 원본과 같아 보인다. 나머지는 전부
색이 빠진 상태이고, `_d`가 어두울수록 덜 티가 날 뿐이다.

## 4. 초기 가설에서 정정한 것

조사 도중 두 번 틀렸고 둘 다 기록해 둔다.

**"네 클래스 전부 무채색"은 과했다.** 전체 평균 채도만 봐서 국소 유채색을 뭉갰다. 채도 히스토그램으로
다시 보니 워로드 22.9%, 도화가 7.5%, 창술사 4.0%로 실제 색이 있다. 사용자가 본 창술사 보라색은
shoulder `_d`에 구워진 hue 240~300° 픽셀이 맞다.

**"창술사는 원본 의도대로 나오는 중"도 틀렸다.** 우리가 shipping하는 변종은 기본 `pc_flm_00_upper_mi`가
아니라 `-1` 변종(`pc_flm_00_upper_d_1_loc_int.tga`)이다. 기본 변종은 `diffusecolor_a/b`가 전부
(1,1,1)이라 틴트가 없지만 `-1` 변종에는 자주/마젠타가 걸려 있다. 창술사도 색이 빠진 상태다.

**도화가는 "원래 흰 옷일 가능성"을 남겨 뒀는데 아니었다.** `diffusecolor_a`가 붉은색이라 명확히 틀렸다.

## 5. 재현 절차

```powershell
# 텍스처 채도 통계 — TGA를 직접 디코드해 sat 히스토그램을 낸다
#   대상: Client\Bin\Resources\Character\<Class>\textures\*_d.tga

# .wmodel이 참조하는 텍스처 경로 (UTF-16으로 저장돼 있어 Unicode로 읽어야 보인다)
$b = [IO.File]::ReadAllBytes('...\DimensionMaster_Character.wmodel')
[regex]::Matches([Text.Encoding]::Unicode.GetString($b), '[\x20-\x7E]{5,}') |
  ForEach-Object { $_.Value } | Where-Object { $_ -match '\.(tga|dds)' } | Sort-Object -Unique

# 원본 머티리얼 파라미터
#   C:\Users\95jus\Downloads\umodel_win32\_export_<code>_psk\PC_<CODE>_00\MaterialInstanceConstant\*.props.txt
```

## 6. 확정한 것과 남은 것

확정:

- 원인은 색 마스크 + 영역 틴트 경로 부재 하나이며 UV·슬롯·매핑 오류가 아니다.
- `CMaterial`의 1×1 회색 폴백(Material.cpp:302, `0xff4d4d4d`)은 타지 않았다. 폴백이면 흰색이 아니라
  중간 회색으로 보인다. `.wmodel`의 diffuse 경로는 전부 유효하고 파일도 존재한다.
- 도화가·창술사는 색이 빠진 상태이고, 워로드는 틴트가 항등이라 정상이다.

미확인:

- **차원술사 파라미터.** `pc_sp_m_01` 스테이징 export가 없다. 다른 클래스와 달리 `_orm` 기반
  신세대 머티리얼이라 파라미터 이름이 다를 수 있어 `PC_SP_M_01` 재추출이 필요하다.
  `_d`가 100.0% 무채색이고 `_cm`(평균 (152,25,22), 채도 184)이 존재한다는 사실까지만 확인했다.
- **정확한 합성 공식.** 부모 머티리얼 `pbr_base_msk`는 노드 917개짜리 그래프이고 umodel 덤프는
  노드 목록만 주고 연결은 주지 않는다. 원본 공식 그대로의 복원은 어렵고, `_cm` 채널로 a/b/c를
  섞어 diffuse에 곱하는 근사로 가야 할 것으로 본다. 아직 근사식을 정하지 않았다.

구현 시 손댈 곳(설계 확정 전 목록):

```text
추출        나머지 다섯 클래스의 _cm (+ 필요시 _m)
저장 계약   MODEL_MATERIAL_DATA / MATERIAL_ENTRY_V2에 색 마스크 슬롯과 틴트 색 추가
                → .wmodel 포맷 버전이 올라가므로 ModelAssetConverter 재생성 필요
바인딩      CMaterial::Initialize의 슬롯 추가. aiTextureType_UNKNOWN은 ORM이 이미 쓰는 중
셰이더      Shader_VtxAnimMeshBinary.hlsl에 마스크 샘플링과 영역 틴트 합성
```
