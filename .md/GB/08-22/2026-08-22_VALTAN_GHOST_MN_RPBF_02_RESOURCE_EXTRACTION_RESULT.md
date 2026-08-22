# 발탄 망령화 모델 원본 리소스 추출 결과

## 1. 완료 상태

발탄 망령화 형태의 원본 SkeletalMesh와 전용 ghost 머티리얼·텍스처를 추출·쿠킹해 다음 런타임
폴더에 설치했다.

```text
Client/Bin/Resources/Character/Valtan/Ghost/
  MN_RPBF_02.wmodel
  textures/  (DDS 6개)
```

설치본에는 `.wmodel` 1개와 그 WModel이 실제 참조하는 DDS 6개만 있다. UPK, PSK, PSA, FBX,
Blend, 조사 로그는 런타임 폴더에 넣지 않았다.

## 2. 대상 확정 근거

이름 유사도가 아니라 NPC DB 행에서 mesh → material → texture 참조를 끝까지 따라가 확정했다.

```text
EFTable_Npc PrimaryKey 480008
-> Model EFDLCHAR_MN_RPBF_02.MN_RPBF_02
-> UModel -nameresolve 물리 UPK 9G1M8U4W1BZDE7JTR764SIP.upk
-> SkeletalMesh mn_rpbf_02_sk
-> Materials[3] = mn_rpbf_01-1_mi / mn_rpbf_01_2-1_mi / mn_rpbf_01_1-1_mi
-> 세 MaterialInstance 모두 Parent = mastermaterial_ch_preset.ghost.preset_ghost_msk
-> texture_diffuse = mn_rpbf_01_ghost_d
```

`preset_ghost_msk`는 원본이 ghost 전용으로 둔 master material preset이고 `mn_rpbf_01_ghost_d`는
발탄 패키지 안의 ghost 전용 diffuse다. 두 근거가 함께 성립하므로 `MN_RPBF_02`가 망령화 형태다.

발탄 본체와의 대응은 슬롯 단위로 1:1이며 `-1` 접미사가 ghost 변형을 뜻한다.

| slot | 발탄 `mn_rpbf_01_sk_loc_int` | 망령 `mn_rpbf_02_sk` |
|---:|---|---|
| 0 | `mn_rpbf_01_mi` | `mn_rpbf_01-1_mi` |
| 1 | `mn_rpbf_01_2_mi` | `mn_rpbf_01_2-1_mi` |
| 2 | `mn_rpbf_01_1_mi` | `mn_rpbf_01_1-1_mi` |

DB 행도 본체와 같은 등급·크기·액션 그룹을 가진다.

| NPC | Model | Grade | ModelSize | ActionObjectGroup |
|---:|---|---:|---:|---|
| 480007 발탄 | `EFDLCHAR_MN_RPBF_01.MN_RPBF_01` | 10 | 140 | `MN_RPBF_00` |
| 480008 망령 발탄 | `EFDLCHAR_MN_RPBF_02.MN_RPBF_02` | 10 | 140 | `MN_RPBF_00` |

## 3. 정확한 머티리얼 연결

`.props.txt`가 실제로 지정한 파라미터만 remap했다. ghost preset에는 specular와 emissive 슬롯이
없으므로 비워 두고 다른 슬롯에 임의로 채우지 않았다.

| 머티리얼 슬롯 | Base | Normal | Specular | Emissive |
|---|---|---|---|---|
| `mn_rpbf_01-1_mi` | `mn_rpbf_01_ghost_d.dds` | `mn_rpbf_01_n.dds` | 없음 | 없음 |
| `mn_rpbf_01_2-1_mi` | `mn_rpbf_01-2_d_loc_int.dds` | `mn_rpbf_01-2_n_loc_int.dds` | 없음 | 없음 |
| `mn_rpbf_01_1-1_mi` | `mn_rpbf_01-1_d_loc_int.dds` | `mn_rpbf_01-1_n_loc_int.dds` | 없음 | 없음 |

ghost preset이 추가로 가진 `opacity`, `opacity_power`, `diffuse_color`, `object_color`,
`rimlight_color`는 현재 WMA2/CMaterial에 대응 슬롯이 없어 WModel에 싣지 않았다. 원본 값은
아래와 같으며 반투명·림라이트 표현을 복원할 때 필요하다.

```text
mn_rpbf_01-1_mi     diffuse_color 0.1,0.6,0.5   object_color 0.6,1,1     rimlight_color 0.1,1,0.6
mn_rpbf_01_1-1_mi   diffuse_color 0.2,1.2,1     object_color 0.6,1,1     rimlight_color 0.1,1,0.6
mn_rpbf_01_2-1_mi   diffuse_color 0.2,1.2,1     object_color 0.6,1,1     rimlight_color 0.1,1,0.6
```

## 4. ActorX 처리와 보존 경계

```text
exact PSK + exact PSA
-> PSA bone 배열을 PSK 순서로 재배열
-> 30fps sequence만 subset
-> Blender 3.0 ActorX importer
-> FBX
-> ModelAssetConverter
-> WModel
```

Skeletal 모델이므로 정적 메시용 `--pretransform`은 사용하지 않았다.

### 4.1 bone order 재배열

`mn_rpbf_02_sk`와 `mn_rpbf_00_ani`는 bone 이름 집합과 개수가 84개로 완전히 같지만 배열 순서가
45곳 달랐다. 08-06 루가루와 같은 상황이며 같은 `reorder_actorx_psa_bones.py`로 PSA의 BONENAMES와
각 프레임 key 순서만 PSK 순서로 재배열했다. `mn_rpbf_00_evt2_ani`는 처음부터 순서가 일치해
재배열하지 않았다.

`mn_rpbf_02_sk`의 84개 bone은 발탄 본체 `mn_rpbf_01_sk_loc_int`와 이름·개수·순서가 모두 같다.
즉 망령화는 발탄과 같은 스켈레톤을 쓰는 별도 외형 메시다.

### 4.2 제외한 원본 sequence

한 FBX의 Scene FPS를 하나로 유지하고 원본 timing을 변조하지 않기 위해 실제 30fps가 아닌 6개
sequence를 제외했다. 132 + 14 = 146개 중 140개를 포함했다.

| 출처 | 제외 sequence | 원본 FPS |
|---|---|---:|
| `mn_rpbf_00_ani` | `idle_battle_1` | 28.714286804 |
| `mn_rpbf_00_ani` | `att_battle_6_01_start` | 28.695652008 |
| `mn_rpbf_00_evt2_ani` | `evt2_idle_action_01` | 28.928571701 |
| `mn_rpbf_00_evt2_ani` | `evt2_finalfight_02_5` | 28.695652008 |
| `mn_rpbf_00_evt2_ani` | `evt2_reveal_01` | 28.548387527 |
| `mn_rpbf_00_evt2_ani` | `evt2_finalfight_02_6` | 28.928571701 |

### 4.3 WModel animation 이름 길이 한계

WModel의 animation 이름 필드는 39자에서 잘린다. ActorX importer가 만드는 이름은
`<psk 파일 stem>.ao_<clip>`이므로 stem이 `mn_rpbf_02_sk`(13자)이면 최장 clip
`evt1_att_battle_5_01_start`(26자)가 43자가 되어 잘리고 검증이 실패한다. stem을 `rpbf_02`(7자)로
줄여 최장 37자로 맞췄다. 최종 animation 이름은 `rpbf_02.ao_<clip>`이다.

원본 PSK/PSA와 중간 산출물은 `C:\LostArkExtract\GhostValtan_20260822`와
`C:\LostArkExtract\GhostValtanCook_20260822`에 보존했다.

## 5. 설치 결과

| 파일 | bytes | SHA-256 (앞 32자) |
|---|---:|---|
| `MN_RPBF_02.wmodel` | 42,166,008 | `EC69F9E46B73A65C949048B1D7FF0ACF` |
| `textures/mn_rpbf_01_ghost_d.dds` | 524,416 | `88D35E72FC9558BA94B3BF32A98554AF` |
| `textures/mn_rpbf_01_n.dds` | 1,048,704 | `BF20A79B2166E8F18DBC5F2A05705DF4` |
| `textures/mn_rpbf_01-1_d_loc_int.dds` | 524,416 | `8CD0F5226B48B8DEC213235AAD7C2DDA` |
| `textures/mn_rpbf_01-1_n_loc_int.dds` | 1,048,704 | `DD65D13B1E9A9636CA3530A5E7AFFC1E` |
| `textures/mn_rpbf_01-2_d_loc_int.dds` | 524,416 | `A7FBF08775C69E9479016AFDAA7E5117` |
| `textures/mn_rpbf_01-2_n_loc_int.dds` | 1,048,704 | `020929679027864E6B6F5DB6ADE4A1B8` |

총 7개 파일, 런타임 payload 46,885,368 bytes다.

메시 실측값은 point 8,153 / vertex 9,395 / triangle 14,472 / material 3 / bone 84다. 발탄 본체는
point 8,885 / triangle 15,906이므로 망령 메시는 같은 스켈레톤의 별도 축소 외형이다.

## 6. 검증 결과

- WModel은 `sections=143 animations=140 skeleton=yes`다.
- Blender Action 수 140과 WModel animation 수 140이 일치한다.
- 140개 animation 이름이 모두 잘리지 않았고 중복이 없다.
- PSK/PSA bone 이름·개수·순서 계약을 재배열 후 검증했다.
- 포함 sequence의 FPS는 모두 30.0이다.
- WModel의 texture 참조 6개가 모두 `textures/<file>.dds` 상대 경로이고 설치 폴더의 실제 파일로 해소된다.
- 설치 DDS 6개와 UModel export 원본의 SHA-256이 모두 같다.
- 설치 폴더에는 `.wmodel`과 `.dds` 외 확장자가 없다.
- `Client/Bin/Resources`는 `.gitignore` 대상이므로 이번 설치는 저장소 커밋에 포함되지 않는다.

## 7. 이번 범위 밖

- 런타임 연결. `Data/Actors/BossCatalog.json`은 아직 `VALTAN_MN_RPBF_01`만 가지며 망령 형태 entry,
  phase 전환 계약, Server authority 연동은 만들지 않았다.
- 반투명·림라이트 ghost 표현. 현재 WModel은 base와 normal만 싣는다. `preset_ghost_msk`의 opacity와
  rimlight를 살리려면 Shader/CMaterial 슬롯 확장이 별도로 필요하다.
- 무기. 망령 전용 무기 메시는 없다. `wp_mn_rpbf_01_sk` 하나를 `wp_mn_rpbf_01-1_mi` /
  `wp_mn_rpbf_01_1-1_mi` ghost preset으로 바꿔 쓰며 텍스처도 본체와 동일하다. 기존
  `Character/Valtan/ValtanWeapon.wmodel`이 이미 그 형상을 가진다.
- 분신·축소 변형. `MN_RPBF_02-1`(summon, ModelSize 140)과 `MN_RPBF_02-2`(485xxx, ModelSize 120)은
  별도 mesh 패키지로 해소되지 않는다. `mn_rpbf_01-1a_mi` 계열이 같은 ghost preset에 `object_color`만
  0.2,0.7,0.7로 낮춘 흐린 변형이므로 material override 경로로 보이지만 LookInfo로 확증하지 않았다.
- `mn_rpbf_02_sk_chn`. 같은 형상의 bone 1개 rigid 사본이라 스키닝 재생에 쓰지 않아 제외했다.
- `mn_rpbf_02_dead_sk`. 84 bone 사망 메시지만 머티리얼이 ghost preset이 아니라
  `mn_rpbf_01_*_mi_dead`이므로 망령 전용이 아니다.
- 시각 판정. 화면에서의 형상·색·투명도 확인은 사용자가 직접 수행한다.

## 8. Animation Tool 프리뷰 연결과 반투명 근사

### 8.1 프리뷰 등록

`Client/Public/AnimationPreviewAssets.h`의 `ANIMATION_PREVIEW_ASSETS`에 항목 하나를 추가했다.
`CLoader::Ready_AnimationPreviewModels`와 `CCharacterPreviewPanel::Select_Asset`이 이 배열을 그대로
순회하므로 별도 코드 경로는 만들지 않았다. Character Select에서는 선택 시점에 lazy로 준비되고
Development(Test)에서는 진입 시 선등록된다.

`fPreviewScale`은 `0.01f`다. 발탄 본체 항목의 `0.0001f`를 따르지 않은 이유는 두 `.wmodel`의
쿠킹 경로가 다르기 때문이며 PSK point 좌표 범위로 실측해 확정했다.

| PSK | X | Y | Z |
|---|---:|---:|---:|
| `mn_rpbf_02_sk` (망령) | 290.6 | 265.8 | 286.3 |
| `mn_rpbf_01_sk_loc_int` (발탄) | 290.6 | 265.8 | 300.0 |
| `mn_rprs_02_sk` (루가루) | 143.1 | 251.1 | 311.4 |

세 PSK 모두 같은 cm 단위이고 루가루가 `0.01f`를 사용하므로 망령도 같은 값을 쓴다.

### 8.2 첫 실행에서 확인된 결함

첫 실행 결과 메시·스킨·본은 정상이었으나 화면이 망령이 아니라 평범한 발탄으로 보였다.
원인은 ghost 외형이 텍스처가 아니라 전적으로 머티리얼이라는 데 있다.

`mn_rpbf_01_ghost_d`와 발탄 본체 `mn_rpbf_01_d`를 픽셀 단위로 비교하면 1,048,576개 중 21,844개(2%)만
다르고 최대 채널 편차가 29다. 즉 ghost 전용 diffuse는 사실상 같은 살결 텍스처이며 망령 외형은
`preset_ghost_msk`가 만든다.

```text
BlendMode        = BLEND_Translucent
opacity          = 1        opacity_power = 10
opacity_falloff  = 3        opacity_range = 1
diffuse_color    = 0.1, 0.6, 0.5
object_color     = 0.6, 1, 1
rimlight_color   = 0.1, 1, 0.6
rimlight_power   = 1
wave_intensity   = 0.2
```

### 8.3 채택한 해법: 색만 diffuse에 굽는다

반투명을 먼저 시도했으나 화면에서 지나치게 비쳐 보였고, 사용자 결정으로 불투명을 유지하고 색만
조정하기로 했다. 불투명 deferred 경로는 조명·노멀맵·그림자를 그대로 쓰지만 tint를 걸 수 없다.
`Shader_VtxAnimMeshBinary.hlsl`의 dye tint는 `if (0 != g_HasDyeMask)` 안에 있고
`DeferredMaterialRenderUtils`가 `aiTextureType_BASE_COLOR` 마스크가 있을 때만 그 플래그를 세우는데,
망령 머티리얼에는 color mask가 없다.

따라서 셰이더와 WModel 머티리얼 계약을 바꾸지 않고 원본이 런타임에 하던 `diffuse * diffuse_color`
곱셈을 diffuse 텍스처에 미리 구웠다. 앞서 만들었던 반투명 코드 경로는 소비자가 없어지므로
`Part_Body`와 `CharacterPreviewPanel` 변경을 전부 되돌렸다. 현재 Client 코드 변경은
`AnimationPreviewAssets.h`의 프리뷰 항목 하나뿐이다.

굽는 도구는 `Tools` 정본이 아닌 세션 스크립트이며 다음 위치에 있다.

```text
<scratchpad>/tint_ghost.py
python tint_ghost.py <desat> <r> <g> <b> <gain> <out_dir>
```

현재 적용값은 `desat 0.85`, `rgb 0.62 / 0.95 / 0.90`, `gain 1.50`이다. 휘도로 탈색한 뒤 연한 청록을
곱하므로 균열·혹·갑주 문양 같은 표면 디테일이 남는다. 원본 `diffuse_color`(본체 0.1/0.6/0.5,
갑주 0.2/1.2/1.0)를 그대로 쓰면 너무 어두워 사용자 요청대로 연한 청록으로 조정한 값이다.

세 diffuse 슬롯을 모두 칠했다. 본체만 칠하면 갑주가 금색으로 남는다.

### 8.4 설치 결과 갱신

diffuse 3장이 UModel 원본에서 tint 적용본으로 교체됐다. 압축 없이 A8R8G8B8로 쓰므로 DXT1 재압축
의존이 없다. normal 3장은 원본 ATI2 그대로다.

| 파일 | bytes | 상태 |
|---|---:|---|
| `mn_rpbf_01_ghost_d.dds` | 4,194,432 | tint 적용, A8R8G8B8 |
| `mn_rpbf_01-1_d_loc_int.dds` | 4,194,432 | tint 적용, A8R8G8B8 |
| `mn_rpbf_01-2_d_loc_int.dds` | 4,194,432 | tint 적용, A8R8G8B8 |
| `mn_rpbf_01_n.dds` | 1,048,704 | 원본 ATI2 |
| `mn_rpbf_01-1_n_loc_int.dds` | 1,048,704 | 원본 ATI2 |
| `mn_rpbf_01-2_n_loc_int.dds` | 1,048,704 | 원본 ATI2 |

5절의 SHA-256은 diffuse 3장에 대해 더 이상 유효하지 않다. UModel 원본은
`C:\LostArkExtract\GhostValtan_20260822\texdds\MN_RPBF_01\Texture2D`에 그대로 보존돼 있고 그 값이
5절과 일치한다. 되돌리려면 그 폴더를 다시 복사한다.

`.wmodel`의 texture 참조 6개는 파일 이름을 바꾸지 않았으므로 그대로이고 재쿠킹이 필요 없다.

### 8.5 남은 경계

- 원본의 프레넬 반투명(`opacity_power 10`), 외곽 발광(`rimlight_color`), 흔들림(`wave_intensity 0.2`)은
  구현하지 않았다. 현재는 불투명 단색 tint다.
- 색이 텍스처에 구워져 있으므로 런타임에서 조절할 수 없다. 바꾸려면 위 스크립트를 다시 실행한다.
- 원본이 `object_color`(0.6/1/1)를 diffuse와 어떻게 결합하는지는 base Material3 dump에 vector
  파라미터 이름이 직렬화되지 않아 확인하지 못했다.

### 8.6 검증 상태

- 자동: DDS 헤더 128바이트/A8R8G8B8/1024x1024 확인, `.wmodel` texture 참조 6개가 설치 파일로 해소됨을
  확인, 인코딩(ASCII/CRLF) 보존과 `git diff --check` 확인.
- 미검증: 빌드와 화면 색 판정은 사용자가 수행한다.
