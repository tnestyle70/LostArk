# 2026-09-18 발탄 컷신 배우 원본 모델·재질 확인 RESULT

읽기·조사 전용. 저장소 소스·데이터·Resources 는 수정하지 않았다. 측정 스크립트와 결과는
`out/ValtanCutsceneActorMaterial20260918/`, 원본 추출은 `C:\LostArkExtract\ValtanColorless_20260918\`(추출만, 쿠킹·설치 없음).

## 사용자 지적

"컷신에서 발탄이 유령으로 나오는 게 말이 안 된다. 원본에 유령으로 나오라고 되어 있나?"
대상은 `발탄 등장` 0~13.0s(무채색 대역)와 `발탄 최후` 전 구간.

## 1. 원본 데이터 (확인)

근거: 원본 패키지 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages`,
`Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py` 의 `extract_scene`/`group_actor`,
umodel `-dump`/`-list`/`-export -nameresolve`. 덤프: `dump_actor_materials.txt`, `mi_dump.txt`,
`preset_dead_dump.txt`, `master_blend.txt`, `param_keys.txt`, `ghost_preset_params.txt`, `compare_psk.txt`, `final_checks.txt`.

### 발탄 최후 (`978T90T8XHW4FTFB8IWP8IMIWNW6Y4.upk`, matinee export 24, 그룹 `발탄` export 45)

- actor export 25 `efskeletalmeshactorlookinfomat_0`, SkeletalMeshComponent export 278
- SkeletalMesh = import -120 `mn_rpbf_02.mesh.mn_rpbf_02_sk`, AnimSets = `mn_rpbf_00.ani.mn_rpbf_00_ani`
- **Materials 덮어쓰기** = `mn_rpbf_01.mat.mn_rpbf_01-1_mi`, `mn_rpbf_01_2-1_mi`, `mn_rpbf_01_1-1_mi`
- `mn_rpbf_01-1_mi` 부모 = `mastermaterial_ch_preset.ghost.preset_ghost_msk`
  - 최상위 `Material3 preset_ghost_msk`(`QX5YGQAYNQ9IY32NV9O3O4UQ.upk` export 159) **`blendmode = blend_translucent`**
  - MI 값: opacity 1, opacity_power 10, diffuse_color (0.1, 0.6, 0.5), object_color (0.6, 1, 1), rimlight_color (0.1, 1, 0.6), texture_diffuse `tex.mn_rpbf_01_ghost_d`, texture_normal `tex.mn_rpbf_01_n`
  - `mn_rpbf_01_1-1_mi` 도 같은 유령 프리셋 부모
- 재질 트랙: `dead` 0(3.87s) → 1(15.90s) → 1(17.56s) — 본체 3슬롯과 무기 2슬롯(`wp_mn_rpbf_01-1_mi`, `wp_mn_rpbf_01_1-1_mi`) 각각.
  유령 프리셋에 `dead` 스칼라 파라미터가 선언돼 있다(`ghost_preset_params.txt`). 그 파라미터가 화면에 무엇을 하는지는 [미확인].
- `hit_color` 트랙 1.92~16.22s, skelcontrol `h_up` 강도 0→0.5→0 (2.22~5.15s).

### `mn_rpbf_02_sk` 메시 자체

- PSK `MATT` 기본 재질 슬롯 = `mn_rpbf_01-1_mi`, `mn_rpbf_01_2-1_mi`, `mn_rpbf_01_1-1_mi` — **메시 기본값부터 유령 프리셋 재질**이다.
- 같은 패키지(`MN_RPBF_01`)에 `mn_rpbf_02_mi`(부모 `monster_base_msk_high`, 일반 본체 텍스처)도 있으나 메시 기본 슬롯은 아니다. 어디서 쓰는지는 [미확인].
- 판정: **retail 에서 `mn_rpbf_02` 는 유령 전용으로 세팅된 메시**다(기본 재질이 반투명 유령 프리셋). 최후 컷신은 그 유령 재질을 그대로 명시해 쓴다.

### 발탄 등장 (`978T90T8XHW4FTFB8IWP8IMIWNW6C4.upk`, matinee export 53)

| 그룹 | 메시 | 재질 덮어쓰기 | 부모 / 블렌드 | 재질 트랙 |
|---|---|---|---|---|
| `발탄무채색` | `mn_rpbf_00.mesh.mn_rpbf_00_sk` | `mn_rpbf_00.mat.mn_rpbf_01_mi_dead`, `mn_rpbf_01_parts_mi_dead` | `monster_dead_msk_high` → `pbr_base_msk` **`blend_masked`** | `dead` 1(4.00s)→0(5.57s)→0(12.32s)→1(15.06s), `ibl_intensity` 1 |
| `마수군단장발탄` | `mn_rpbf_01.mesh.mn_rpbf_01_sk_loc_int` | `mn_rpbf_01_mi_dead`, `_2_mi_dead`, `_1_mi_dead` | `monster_dead_msk_high` → `pbr_base_msk` masked | `dead` 1(12.06s)→0(15.06s), `ibl_intensity` 0(13.06s)→1(15.06s) |
| `발탄동기화` | `mn_rpbf_01_sk_loc_int` | `mn_rpbf_01_mi`, `_2_mi`, `_1_mi` | `monster_base_msk_high` → masked | `dead`(다른 MI 대상), `ibl` |

- 무채색 재질 텍스처: diffuse `mn_rpbf_00-1_d_loc_int`(평균 채도 **0.009**, 사실상 회색), normal `mn_rpbf_00_n_loc_int`, spec `mn_rpbf_00-1_s_loc_int`, color_fx `mn_rpbf_00_cm_loc_int`, 효과 텍스처 `fx_a_ice_003`. 비교: `mn_rpbf_00_d_loc_int` 0.268, 본체 `mn_rpbf_01_d` 0.133.
- **판정: 등장의 무채색 발탄은 유령이 아니다.** 불투명(masked) 사망/빙결 계열 재질의 회색 발탄이며 `dead` 파라미터로 상태가 바뀐다.

### 버러지·포효 (`...C4.upk` matinee 54, 55)

`발탄`·`발탄동기화` 모두 `mn_rpbf_01_sk_loc_int` + `mn_rpbf_01_mi`/`_2_mi`/`_1_mi`(부모 `monster_base_msk_high`, masked). 재질 트랙은 `hit_color` 뿐(포효 한 트랙은 off).

### 1관문 입장 (`...J4.upk` matinee 24)

두 늑대 모두 `mn_rprs_02.mesh.mn_rprs_02_sk`. 흰늑대 재질 `mn_rprs_02-2_mi`, `mn_rprs_02-3_mi`(+ `dead`, `hit_color` 트랙), 검늑대 `mn_rprs_02_mi`, `mn_rprs_02-1_mi`.

### 뼈대

`mn_rpbf_00_sk`(점 13,670) · `mn_rpbf_01_sk_loc_int`(8,885) · `mn_rpbf_02_sk`(8,153) 모두 뼈 84개, 이름·순서 동일(해시 `36c62ff8e02d`). 원본 컷신 배우는 모두 같은 AnimSet `mn_rpbf_00_ani` 를 쓴다.

## 2. 현재 저장소 연결과 대조

| 컷신 배우 | 원본 메시 · 재질 | 현재 저장소 | 판정 |
|---|---|---|---|
| 등장 무채색 (0~13.0s) | `mn_rpbf_00_sk` + 회색 사망 재질(masked) | `Ghost/MN_RPBF_02.wmodel`(반투명 유령) 대역 | **불일치** — 메시·재질 모두 다름 |
| 등장 본체 (13.06s~) | `mn_rpbf_01_sk_loc_int` + 사망 재질, 13.06~15.06s 에 회색→정상 전환 | `MN_RPBF_01.wmodel`(정상 재질) | 메시 일치, **재질 전환 없음** |
| 최후 | `mn_rpbf_02_sk` + 유령 프리셋(반투명) + `dead` 0→1 | `Ghost/MN_RPBF_02.wmodel` 재질 `mn_rpbf_01-1_mi`/`_1-1_mi`/`_2-1_mi`, program 84 반투명 | 메시·재질 이름·반투명 **일치**. `dead`·`hit_color` 변화 없음, diffuse 텍스처 해시가 원본 추출과 다름 |
| 버러지·포효 | `mn_rpbf_01_sk_loc_int` + 정상 재질 | `MN_RPBF_01.wmodel` | 일치(`hit_color` 번쩍임 없음) |
| 1관문 흰늑대 | `mn_rprs_02_sk` + `-2/-3_mi` | 루가루 `mn_rprs_02_mi`/`-1_mi`(검늑대 재질) | **흰늑대 재질 불일치** |
| 1관문 검늑대 | `mn_rprs_02_sk` + `_mi`/`-1_mi` | 같음 | 일치 |
| 전 배우 무기 | `wp_mn_rpbf_*` 무기 재질이 트랙 대상에 있음 | 컷신 배우에 무기 없음 | 불일치(fork B 보고와 같음) |

저장소 유령 출처: `C:\LostArkExtract\GhostValtanCook_20260822\MN_RPBF_02\MN_RPBF_02.wmodel.info.txt` — 재질 3개, first=`mn_rpbf_01-1_mi`, base=`textures/mn_rpbf_01_ghost_d.dds`. normal 텍스처 3개는 원본 추출(`GhostValtan_20260822/textures`)과 SHA 동일, diffuse 3개(`mn_rpbf_01_ghost_d`, `-1_d`, `-2_d`)는 원본 추출·`textures_tinted` 어느 쪽과도 해시가 다르다. 수정 경위는 [미확인].
제품: `BossCatalog` `BOSS_VALTAN` = `MN_RPBF_01.wmodel` + Parts1/2 + 무기, `BOSS_VALTAN_GHOST` = `Ghost/MN_RPBF_02.wmodel` + 무기.

## 3. 본체 대체 가능성

- 뼈대 동일(84본, 해시 동일). 최후 템플릿 클립 5개(`mesh_abn_groggy_1_start/loop/end`, `mesh_evt1_att_battle_5_01_end`, `mesh_dead_1`)가 본체 `MN_RPBF_01_AnimSet.wmodel`, 유령 AnimSet 모두에 있다 — **기술적으로는 본체로 교체 가능**.
- 그러나 원본 최후 배우는 유령 프리셋 재질이므로 본체로 바꾸면 **원본과 달라진다**.

## 4. 원본과 같게 만드는 선택지

- **최후**: 현재 유령 연결이 원본과 같은 메시·재질 계열이다. 모델 교체는 불필요. 원본과 남은 차이는 `dead` 0→1(3.87~15.90s), `hit_color`, `h_up` skelcontrol, 무기, diffuse 텍스처 해시 차이. 재질 파라미터 트랙은 World Sequence 계약에 없으므로 구현하려면 별도 승인·설계 필요.
- **등장 무채색 (b 원본 메시 추출·쿠킹 필요)**: 추출본 `C:\LostArkExtract\ValtanColorless_20260918\MN_RPBF_00\SkeletalMesh3\mn_rpbf_00_sk.psk` + 컷신 재질 텍스처 `...\cutscene_override\MN_RPBF_00\Texture2D\mn_rpbf_00-1_d_loc_int.tga`, `mn_rpbf_00_n_loc_int.tga`, `mn_rpbf_00-1_s_loc_int.tga`, parts `mn_rpbf_00-1_parts_d_loc_int/_parts_n/-1_parts_s/-1_parts_em`. PSK 기본 슬롯 이름은 `mn_rpbf_00_mi_loc_int`, `mn_rpbf_00_parts_mi` 이므로 쿠커 `-MaterialRemap` 로 컷신 재질 텍스처(`-1_` 회색판)를 지정해야 한다. 단위는 본체 `MN_RPBF_01.wmodel`(preScale 0.0001)과 맞추려면 `-NoScaleDown`(cm), 유령처럼 0.01 을 쓰려면 기본 쿠킹 — 어느 쪽이든 worldsequences objectResources 의 `modelPreScale` 과 짝을 맞춰야 한다. AnimSet 은 뼈대가 같아 본체 AnimSet 재사용 가능. 수정 파일: `Client/Bin/Resources/Character/Valtan/...` 신규 wmodel, `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json` objectResources·`entrance.colorless` 바인딩, WorldSequences 게시. 위험: `fx_a_ice_003` 빙결 효과와 `dead` 전환은 표준 D/N/S 쿠킹으로 재현되지 않는다(회색 불투명 발탄까지만).
- **등장 본체 (c 재질 전환)**: 메시는 맞다. 13.06~15.06s 회색→정상 전환은 재질 파라미터 트랙이 필요해 (최후와 같은) 별도 설계 대상.
- **1관문 흰늑대 (b/c)**: `mn_rprs_02-2_mi`/`-3_mi` 텍스처로 별도 쿠킹 또는 재질 슬롯 교체가 필요.

추천: 등장 무채색을 원본 `mn_rpbf_00` 회색 재질로 쿠킹해 유령 대역을 교체(사용자 지적과 직접 일치하는 불일치). 최후는 원본도 유령이므로 모델 유지.

## 5. 확인 못 한 것

- 유령 프리셋의 `dead` 파라미터와 사망 재질 `dead` 가 실제 화면에서 어떤 변화(투명해짐·빙결·색 복원)를 만드는지. 마스터 재질 노드 그래프는 해석하지 않았다.
- 저장소 유령 diffuse 텍스처가 원본과 다른 이유.
- `mn_rpbf_02_mi`(일반 재질)의 사용처.

## 6. 실수

- 첫 덤프에서 재질 트랙 키 시각 필드를 `invalue` 로 잘못 읽어 전부 0초로 출력했다(실제 필드는 `inval`). 두 번째 스크립트로 바로잡았다.
- umodel 로 `mastermaterial_ch_preset` 논리 패키지를 직접 덤프하려다 실패했고, MI 로드 체인에서 물리 upk 를 찾아 파서로 읽었다.
