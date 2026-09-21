# 가디언 나이트 외형 복원·데이터 동기화 — 구현 계획

작성자: JS · 2026-09-21 · 브랜치 `feature/player-guardian-knight` (a9a703ca 위에 이어서)
선행: `2026-09-21_GUARDIANKNIGHT_CLASS_INTEGRATION_{PLAN,RESULT}.md`

1차에서 클래스·스킬·Server 판정까지 연결됐고, 사용자 화면에서 세 가지가 남았다.
**대머리, 흰 갑옷, 흰 무기.** 그리고 앞 네 직업(창술사·워로드·도화가·차원술사)이 가진
애니셋·데이터 계약 중 가디언 나이트에 없는 것을 같은 형식으로 채운다.

## 0. 실측 결과와 원인

| 증상 | 실측 | 원인 | 선례 |
|---|---|---|---|
| 대머리 | 바디 submesh 0 `pc_ft_15_hair_mi`가 있고 `BAKED_HAIR = 1<<0`. `Character.cpp:2689`가 `iBodyHairMeshMask`를 **항상** 숨긴다("Hair is worn, never drawn by the body"). 가디언 나이트는 `_Hair` 장비 파츠가 없다 | 바디 헤어를 무조건 숨기는데 대신 그릴 헤어 파츠가 없음 | 건슬링어 `BAKED_HAIR = 0u` (헤어 세트가 없는 클래스는 바디 헤어를 그대로 그림) |
| 흰 갑옷 | `pc_ddk_00_*_d.tga` 9장 평균 RGB (190~213, R=G=B), 무채색. 워로드 `pc_wgl_00_upper_d`는 (55,46,44) | 가디언 나이트 갑옷은 realpbr 계열이라 `_d`가 무채색 베이스이고 색은 원본 MIC의 dye/mask 파라미터(`diffusecolor_a~d`, `_cm`)가 준다. `CharacterCatalog.json` `PLAYER_GUARDIANKNIGHT`에 `modelMaterialOverrides`가 **0행** | 차원술사 `source.character.realpbr-avatar-v2.v1`(program 3) |
| 흰 무기 | `wp_wddk_04_d.tga` (181,182,181) | 위와 같음, 무기용 realpbr | 차원술사 `WP_WSWP_M_06` `realpbr-weapon.v1`(8) / `-variation.v1`(9) |
| 날개 | `sk_ddk_win_00(-1)_d` (64~71,38,37) — 유채색 | 원본 재질 행 없음. 일반 경로로도 색은 나오지만 원본 program으로 통일 | 워로드 `classic-*` 행 |

바디 슬롯(얼굴 `pc_dl_face_mi_high`, 눈 `pc_dl_eye_mi`, 속눈썹 `pc_dl_eyeao_mi`, 헤어
`pc_ft_15_hair_mi`)도 원본 행이 없다. 지금은 일반 Blinn-Phong으로 그려진다.

애니셋: 1차 RESULT §2.4의 "카드미로 망치 클립은 Delain psa에 없다"는 **틀렸다**.
`PC_DL_00/AnimSet/pc_dl_00_ani.psa`(1,067)에 `pr_it_gstfp_00_att_1_01/1_02/2_01/3_*/4_01`,
`ride_horse/hoverboard/heavywalker_bm9/tube`가 전부 있고 `pc_dl_00_vehicle_ani.psa`(44)에
`ride_swing`, `ride_dragon_2`가 있다. 6종 탈것 + 망치 애니셋을 전부 만들 수 있다.

## 1. 앞 네 직업과 가디언 나이트의 데이터 차이 (2026-09-21 grep)

| 계약 | 워로드 | 가디언 나이트 | 이번 범위 |
|---|---|---|---|
| `CharacterCatalog.json` `modelMaterialOverrides` | 14행 | 0행 | **G02** |
| `animationSetModels` | Esther, Customizing, Ride×6, MazeHammer, TerrainJump (10) | Esther, Customizing, TerrainJump (3) | **G03** |
| `VehicleCatalog.json` riders/skills | 6탈것 전부 | 없음 → H 탑승 시 `no rider pose for this class` 거절 | **G03** |
| `Authored/<Class>.interactionbindings.json` | MAZE 망치 Q/LMB, terrainJump | 없음 → `[Clown] Interaction binding unavailable` | **G03** |
| `UI/HUD/SkillSlotMarks.json` | 콤보/홀딩 마크 | 없음 | **G04** |
| `UI/HUD/HudBuffSources.json` stance | `WARLORD_DEFENSE` 아이콘 | 없음 | **G04** (아이콘 atlas 필요) |
| `UI/MVP/MvpClassSymbols.json` classes | class_9 | 없음 | **G04** (라벨 사용자 지정 필요) |
| `Actors/EquipmentLoadoutPresets.json` | null 6슬롯 | 없음 | **G04** |
| `Valtan.clearrewards.json` + `ItemCatalog.json` 명예의 속삭임 6부위 | 있음 | 없음 | **G05** (아이템 ID·아이콘 조사 필요) |
| `Sound/CharacterSoundCatalog.json` | 있음 | 없음 | **G06** (원본 wav 라이브러리 필요, 이 PC에 없음) |
| `EquipmentPresentationCatalog.json` 커스터마이징 세트 | 있음 | 없음 | 범위 밖 (커스터마이징은 별도 PLAN) |
| projectiles / HUD IdentityAnimation | 있음 | 없음 | 범위 밖 (가디언 나이트 원작에 투사체 없음·아이덴티티 UI는 별도) |

## 2. G 목록과 종료 증거

| G | 내용 | 종료 증거 |
|---|---|---|
| G01 | 헤어 표시 | Client Debug 빌드 → 사용자 화면에서 머리카락 확인 |
| G02 | 원본 재질 program 17 MIC 추출·검증·설치·행 추가 | `verify` 전 슬롯 EXACT, `test_source_character_program_groups.py` OK, Product 빌드, 사용자 화면 확인 |
| G03 | 애니셋 7종 + VehicleCatalog + interactionbindings | `compare_attach.py` skeletonHash 일치, JSON parse, H 탑승·카드미로 Q를 사용자 확인 |
| G04 | HUD/MVP/Loadout 데이터 | JSON parse, `git diff --check`, 사용자 HUD 확인 |
| G05 | 발탄 보상·아이템 | `Publish-ItemCatalog.ps1`/`Publish-ValtanClearRewards.ps1` Validate PASS |
| G06 | 사운드 catalog | 원본 라이브러리 확보 후 `build_sound_catalog.py --dry-run` |

G01~G03이 이번 요청의 본체다. G04~G06은 같은 형식으로 채우되 입력이 이 PC에 없는 항목은
표에 적은 의존성을 먼저 받는다.

## 3. G01 — 헤어 표시

### 3-1. `C:/Users/95jus/Desktop/TeamProject/LostArk/Client/Private/Logic_GuardianKnight.cpp`

변경 종류: 상수 교체
적용 위치: `constexpr uint32_t BAKED_HAIR = (1u << 0);`와 그 주석

`Apply_DefaultEquipmentVisibility`와 `Ready_Parts`(`Character.cpp:2689`, `:3848`)가
`iBodyHairMeshMask`를 조건 없이 숨긴다. 헤어 장비 파츠가 없는 클래스는 건슬링어처럼 0으로 두어
바디 헤어를 그린다. 헬멧(`pc_ddk_00_helmet`)은 원작 규칙대로 머리카락을 숨기지 않는다
(일반 헬멧에 IgnoreHair 플래그 없음, 09-18 실측). 생성 화면은 HEAD 슬롯만 숨기므로 헤어는 그대로 보인다.

```cpp
	/* The hair this cooked body draws by itself (pc_ft_15_hair, submesh 0). Kept at
	zero on purpose, like GunSlinger: this class has no cooked hair sets, so nothing can
	take over from the body's own hair and hiding it leaves the head bare. Put the bit
	back once its hairstyle sets exist and it gets a hair part like Warlord. */
	constexpr uint32_t BAKED_HAIR = 0u;
```

교체 후 파일 전문은 현재 파일에서 위 5줄만 다르다(현재 55~57행). 인코딩 UTF-8(BOM 없음) 유지.

검증: Client Debug `Build` → 사용자가 Character Select에서 머리카락·헬멧 동시 표시를 확인한다.
헬멧이 머리카락을 크게 관통하면 도화가 선례(헬멧 `isHidden=true`)를 사용자 결정으로 적용한다.

## 4. G02 — 원본 재질 program 설치와 catalog 행

### 4-1. 대상 MIC 17개 (쿠킹된 wmodel 슬롯 이름 기준, `patch_wmodel_dye.py list` 실측)

| 모델 | 슬롯 | 원본 MIC (package.mat.name) | 예상 family | 예상 program |
|---|---|---|---|---|
| `GuardianKnight_Upper.wmodel` | `pc_ddk_00_upper_mi` | `pc_ddk_00.mat.pc_ddk_00_upper_mi` | realpbr-avatar-v2 | 3 재사용 또는 신규 |
| ″ | `pc_ddk_00_upper1_mi` | `pc_ddk_00.mat.pc_ddk_00_upper1_mi` | ″ | ″ |
| ″ | `pc_ddk_00_upper2_mi` | `pc_ddk_00.mat.pc_ddk_00_upper2_mi` | ″ | ″ |
| ″ | `pc_ddk_00_upper3_mi` | `pc_ddk_00.mat.pc_ddk_00_upper3_mi` | ″ | ″ |
| ″, `_Arm.wmodel` | `pc_dk_av_base_upper_mi` | `pc_dk_av_basebody.mat.pc_dk_av_base_upper_mi` | classic-skin | 1 재사용 |
| `GuardianKnight_Lower.wmodel` | `pc_ddk_00_lower_mi` | `pc_ddk_00.mat.pc_ddk_00_lower_mi` | realpbr-avatar-v2 | 3 또는 신규 |
| ″ | `pc_ddk_00_lower1_mi` | `pc_ddk_00.mat.pc_ddk_00_lower1_mi` | ″ | ″ |
| `GuardianKnight_Arm.wmodel` | `pc_ddk_00_arm_mi` | `pc_ddk_00.mat.pc_ddk_00_arm_mi` | ″ | ″ |
| `GuardianKnight_Shoulder.wmodel` | `pc_ddk_00_shoulder_mi` | `pc_ddk_00.mat.pc_ddk_00_shoulder_mi` | ″ | ″ |
| `GuardianKnight_Helmet.wmodel` | `pc_ddk_00_helmet_mi` | `pc_ddk_00.mat.pc_ddk_00_helmet_mi` | ″ | ″ |
| `GuardianKnight_Wing.wmodel` | `sk_ddk_win_00_mi` | `sk_ddk_win_00.mat.sk_ddk_win_00_mi` | 해시로 판정 | 신규 가능 |
| ″ | `sk_ddk_win_00-1_mi` | `sk_ddk_win_00.mat.sk_ddk_win_00-1_mi` | ″ | ″ |
| `WP_WDDK_04.wmodel` | `wp_wddk_04_mi` | `wp_wddk_04.mat.wp_wddk_04_mi` | realpbr-weapon | 8 재사용 또는 신규 |
| `GuardianKnight.wmodel` | `pc_dl_face_mi_high` | `pc_dl_00_face.mat.pc_dl_face_mi_high` | classic-head | 4 재사용 |
| ″ | `pc_dl_eye_mi` | `pc_dl_00_face.mat.pc_dl_eye_mi` | eye | 5 재사용 |
| ″ | `pc_dl_eyeao_mi` | `pc_dl_00_face.mat.pc_dl_eyeao_mi` | eyelash | 6 재사용 |
| ″ | `pc_ft_15_hair_mi` | `pc_ft_15_hair.mat.pc_ft_15_hair_mi` | hair / hair-two-tone | 7 또는 18 재사용 |

- 바디의 `pc_dl_av_018a_upper(1)_mi`는 `COVERED_BY_ARMOUR`로 항상 숨겨지므로 행을 만들지 않는다.
- MIC variant: wmodel은 WMA2(dye 틴트 미보존)이고 슬롯 이름이 base다. 텍스처 폴더에
  `pc_ddk_00-1_lower1_d.tga`가 있으므로 `lower1`은 `extract` dump의 텍스처 참조로 base MIC가 그
  텍스처를 쓰는지 확인하고, 아니면 `pc_ddk_00.mat.pc_ddk_00-1_lower1_mi`를 대신 추출해 `rows`에
  `@pc_ddk_00_lower1_mi`로 슬롯을 지정한다(09-18 창술사 `-1` variant 선례).
- **어느 program을 쓸지는 이름이 아니라 `extract`가 찍는 base/light 셰이더 맵 해시로 판정한다.**
  해시가 기존 program과 같으면 `verify`로 EXACT를 확인하고 재사용한다.

### 4-2. 실행 명령

```powershell
$py = 'C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe'
$tool = 'Tools/VehiclePipeline/build_vehicle_source_material.py'
$umodel = 'C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe'
$d3d = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\d3dcompiler_47.dll'
$work = 'out/GuardianKnightMaterial20260921'

# 1) 추출 (17 MIC). 출력 한 줄마다 base/light shaderId 해시가 찍힌다.
& $py $tool extract --umodel $umodel --d3dcompiler $d3d --out "$work/dumps" `
  pc_ddk_00.mat.pc_ddk_00_upper_mi pc_ddk_00.mat.pc_ddk_00_upper1_mi `
  pc_ddk_00.mat.pc_ddk_00_upper2_mi pc_ddk_00.mat.pc_ddk_00_upper3_mi `
  pc_ddk_00.mat.pc_ddk_00_lower_mi pc_ddk_00.mat.pc_ddk_00_lower1_mi `
  pc_ddk_00.mat.pc_ddk_00_arm_mi pc_ddk_00.mat.pc_ddk_00_shoulder_mi `
  pc_ddk_00.mat.pc_ddk_00_helmet_mi `
  pc_dk_av_basebody.mat.pc_dk_av_base_upper_mi `
  sk_ddk_win_00.mat.sk_ddk_win_00_mi sk_ddk_win_00.mat.sk_ddk_win_00-1_mi `
  wp_wddk_04.mat.wp_wddk_04_mi `
  pc_dl_00_face.mat.pc_dl_face_mi_high pc_dl_00_face.mat.pc_dl_eye_mi pc_dl_00_face.mat.pc_dl_eyeao_mi `
  pc_ft_15_hair.mat.pc_ft_15_hair_mi

# 2) 게이트: 이미 설치된 program을 원본 MIC로 재현해 EXACT 확인 (차원술사 program 3)
& $py $tool verify --dump "$work/dumps/pc_sp_m_01.mat.pc_sp_m_01_upper_mi.json" `
  --family source.character.realpbr-avatar-v2.v1 --program 3
```

`.mat` outer 이름이 다르면 `umodel -list -game=lostark -kr -nameresolve -path=... PC_DDK_00`으로
MIC의 outer를 읽어 `<package>.<outer>.<name>`으로 바꾼다.

3) 해시 비교. dump마다 `programs[BASE_TYPE].shaderId`/`[LIGHT_TYPE].shaderId`를 기존 dump
(`out/LanceMasterArmour20260918/dumps`, 차원술사·워로드 설치 당시 dump)와 대조한다.

- 같은 해시 → `verify --family <기존> --program <번호>` EXACT면 재사용.
- 다른 해시 → 신규. 번호는 **94부터** 순서대로(갑옷 realpbr 변형 → 무기 → 날개 → 헤어 순).
  family 이름은 `source.character.realpbr-avatar-ddk.v1`처럼 실제 차이를 나타내는 이름을 쓴다.

```powershell
# 4) 신규 program마다 (예: 갑옷 94)
& $py $tool generate --dump "$work/dumps/pc_ddk_00.mat.pc_ddk_00_upper_mi.json" `
  --family source.character.realpbr-avatar-ddk.v1 --program 94 --out "$work/generated94"
& $py $tool install --generated "$work/generated94" `
  --family source.character.realpbr-avatar-ddk.v1 --program 94

# 5) 텍스처: 1차 umodel 스테이징(_export_ddk_psk)을 그대로 쓴다. -notex 없이 뽑은 것.
& $py $tool textures --texture-map Tools/VehiclePipeline/GuardianKnight.texture-map.json `
  --staging 'C:\Users\95jus\Downloads\umodel_win32\_export_ddk_psk'

# 6) 행 생성 (모델마다 한 번)
& $py $tool rows --model Character/GuardianKnight/GuardianKnight_Upper.wmodel `
  --texture-map Tools/VehiclePipeline/GuardianKnight.texture-map.json --out "$work/rows/upper.json" `
  "$work/dumps/pc_ddk_00.mat.pc_ddk_00_upper_mi.json=source.character.realpbr-avatar-ddk.v1" `
  "$work/dumps/pc_ddk_00.mat.pc_ddk_00_upper1_mi.json=source.character.realpbr-avatar-ddk.v1" `
  "$work/dumps/pc_ddk_00.mat.pc_ddk_00_upper2_mi.json=source.character.realpbr-avatar-ddk.v1" `
  "$work/dumps/pc_ddk_00.mat.pc_ddk_00_upper3_mi.json=source.character.realpbr-avatar-ddk.v1" `
  "$work/dumps/pc_dk_av_basebody.mat.pc_dk_av_base_upper_mi.json=source.character.classic-skin.v1"
```

`GuardianKnight.texture-map.json`은 dump의 `textures[].sourceObject` 이름을 키로,
`Character/GuardianKnight/textures/<name>.tga`(무기는 `Character/WP_WDDK_04/textures/`)를 값으로
갖는 JSON이다. dump를 다 뽑은 뒤 참조 텍스처 이름을 모아 작성한다. `_cm`/`_orm`/`_m`/`_e`처럼
1차 쿠킹이 복사하지 않은 텍스처가 여기서 처음 Resources에 들어온다.

### 4-3. 신규 program이 생길 때 손편집 세 곳 + 테스트

`install`은 `Shader_SourceCharacter{Base,Light}Programs.hlsli`와
`SourceCharacterMaterialParameters.h`만 갱신한다. 마지막 그룹 `84~93`을 `84~<마지막 번호>`로
늘린다. 아래는 신규 4개(94~97)를 가정한 예시이며 실제 개수로 맞춘다.

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/EffectPipeline/native_shader_dispatch.py`

변경 종류: 상수 교체 · 적용 위치: `SOURCE_CHARACTER_PROGRAM_GROUPS` (405행)

```python
SOURCE_CHARACTER_PROGRAM_GROUPS = ((1, 8), (9, 16), (17, 24), (25, 32), (80, 83), (84, 97))
```

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Engine/Private/Shader.cpp`

변경 종류: 상수 교체 · 적용 위치: `constexpr uint32_t ranges[][2]` (447행)

```cpp
		constexpr uint32_t ranges[][2] = { {1u,8u}, {9u,16u}, {17u,24u}, {25u,32u}, {80u,83u}, {84u,97u} };
```

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Engine/Private/Model.cpp`

변경 종류: 상수 교체 · 적용 위치: `source.program > 93u` (1829행)

```cpp
            if (source.program == 0u || source.program > 97u || (source.program > 65u && source.program < 80u) ||
```

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/EffectPipeline/test_source_character_program_groups.py`

변경 종류: 센티넬 교체 · 적용 위치: `test_unregistered_program_is_rejected_before_writing` (48행)

현재 센티넬 `92`는 이미 등록된 번호(92·93 monster-dead)라 이 테스트는 지금도 실패 상태일 수
있다. 작업 시작 시 먼저 실행해 현재 상태를 기록하고, 마지막 그룹 끝 +1로 올린다.

```python
        source = source.replace('SourceCharacterBase88(', 'SourceCharacterBase98(')
```

Engine/Client 공용 `.hlsli`가 바뀌므로 Product 빌드에서 그룹 084를 include하는 FX가
재컴파일된다(09-18에는 CSO 28개). 빌드 가드는 Client와 **Server 모두 종료**를 요구한다.

### 4-4. `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Actors/CharacterCatalog.json`

변경 종류: 배열 추가 · 적용 위치: `PLAYER_GUARDIANKNIGHT`의 `"runtimeStatus": "supported"` 뒤

```json
      "runtimeStatus": "supported",
      "modelMaterialOverrides": [
        ...rows/{body,upper,lower,arm,shoulder,helmet,wing,weapon}.json의 행을 이 순서로 이어 붙임...
      ]
```

행 본문은 `rows`가 생성하므로 손으로 쓰지 않는다. `ActorCatalog.cpp:222`가
`bodyModel/equipmentModels/weaponModels`에 없는 모델의 행을 거부하므로 모델 경로는 위 목록과
같아야 한다. 재직렬화하지 않고 텍스트 삽입으로 다른 클래스 행을 보존한다.

### 4-5. 검증

```text
verify        17슬롯 전부 base/light/configure EXACT
python -m unittest Tools/EffectPipeline/test_source_character_program_groups.py
CharacterCatalog.json parse, characters 7, GuardianKnight 행 수 == 17
git diff --check
Product Debug Build (Engine → Client, CSO 재컴파일 포함)
사용자: Character Select → 가디언나이트 → 갑옷 색·무기 색·얼굴·눈·헤어 → Z 화신화 날개
```

재질을 고치기 전에 그 파츠가 실제로 draw되는지 먼저 확인한다(09-18 교훈). 이 클래스는
`AVATAR_*` 파츠가 없어 DEFAULT 6파츠가 전부 그려지고, 날개는 DRAGON 스탠스에서만 그려진다.

## 5. G03 — 애니셋 7종과 탈것·카드미로 연결

### 5-1. 원본과 방식

| 애니셋 | 클립 (원본 이름, `ddk_` 접두로 쿠킹) | psa |
|---|---|---|
| RideHorse | `ride_horse_{idle,run}_normal_1`, `sk_dash`, `sk_victorypose`, `sk_relaxation`, `sk_backkick` | `pc_dl_00_ani.psa` |
| RideSwing | `ride_swing_{idle,run}_normal_1`, `sk_dash`, `sk_nightmoon`, `sk_swing_01~03`, `sk_night` | `pc_dl_00_vehicle_ani.psa` |
| RideHoverboard | `ride_hoverboard_{idle,run}_normal_1`, `sk_feelsgoodspeeding` | `pc_dl_00_ani.psa` |
| RideHeavywalkerBm9 | `ride_heavywalker_bm9_{idle,run}_normal_1`, `sk_powershoulder`, `sk_hifriend`, `sk_shyness`, `sk_cheerup` | `pc_dl_00_ani.psa` |
| RideTube | `ride_tube_{idle,run}_normal_1`, `sk_summerseason`, `sk_tubespin`, `sk_waterfall_01`, `sk_waterfall_02-2`, `sk_waterjump_01~03` (워로드 행과 같은 집합) | `pc_dl_00_ani.psa` |
| RideDragon2 | `ride_dragon_2_{idle,run}_normal_1`, `sk_dash`, `sk_breath_01/02`, `sk_roar`, `sk_look` | `pc_dl_00_vehicle_ani.psa` |
| MazeHammer | `pr_it_gstfp_00_att_1_01` → `maze_hammer_lmb`, `pr_it_gstfp_00_att_2_01` → `maze_hammer_q` | `pc_dl_00_ani.psa` |

각 탈것의 정확한 클립 집합은 `VehicleCatalog.json`의 WARLORD 행에서 `wgl_` 접두를 뗀 이름이
정본이고, 2026-09-21 대조 결과 Delain psa에 전부 존재한다(`feelsgoodspeedin`,
`powershould`는 40바이트 section 이름 절단이며 `ddk_` 접두도 4글자라 같은 이름으로 절단된다).

방식은 1차 Esther/Customizing/TerrainJump와 같은 **codec 경로**다. Blender를 쓰지 않고
`append_psa_clip_to_wmodel.py`로 바디 skeleton carrier에 클립을 붙인 뒤 subset한다.
MazeHammer는 기존 `build_card_maze_player_animations.py`가 이미 `"GuardianKnight": "PC_DL_00"`을
갖고 있으므로 그대로 실행한다.

```powershell
$py = 'C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe'
$src = 'C:\Users\95jus\Downloads\umodel_win32\_export_ddk_psk'
$out = 'out/GuardianKnightAnimSets20260921'

# MazeHammer (기존 builder, GuardianKnight만)
& $py Tools/ActorXAssetCooker/build_card_maze_player_animations.py --source $src --out $out --classes GuardianKnight
```

탈것 6종은 같은 파일의 `carrier` + `append_psa_clip_to_wmodel` + `subset`을 쓰는 새 builder
`Tools/ActorXAssetCooker/build_vehicle_rider_animations.py`로 만든다. 입력은
`VehicleCatalog.json`(WARLORD 행에서 클립 집합 도출) + `SOURCES`(클래스 → package, vehicle psa
이름) + `--classes GuardianKnight`. 출력 `<Class>_Ride<Mode>AnimSet.wmodel` 6개. 이 파일의 전체
코드는 G03 시작 시 `build_card_maze_player_animations.py`의 현재 함수 시그니처를 기준으로
대화에서 낸다(카드미로 builder의 `carrier/subset/sections`를 import해 재사용, 새 codec을 만들지 않는다).

검증: `out/VehicleTerpeion20260913/compare_attach.py GuardianKnight_EstherAnimSet.wmodel <candidate>`
로 bone table·skeletonHash 일치(285본), 클립 이름 `ddk_ride_<mode>_*`, 카드미로는 `maze_hammer_q/lmb`.

설치 위치: `Client/Bin/Resources/Character/GuardianKnight/AnimSets/` (Git 비추적, Drive 전달).

### 5-2. `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Actors/CharacterCatalog.json`

변경 종류: 배열 항목 추가 · 적용 위치: `PLAYER_GUARDIANKNIGHT.animationSetModels`

```json
      "animationSetModels": [
        "Character/GuardianKnight/AnimSets/GuardianKnight_EstherAnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_CustomizingAnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_RideHorseAnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_RideSwingAnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_RideHoverboardAnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_RideHeavywalkerBm9AnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_RideTubeAnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_RideDragon2AnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_MazeHammerAnimSet.wmodel",
        "Character/GuardianKnight/AnimSets/GuardianKnight_TerrainJumpAnimSet.wmodel"
      ],
```

### 5-3. `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Actors/VehicleCatalog.json`

변경 종류: 행 추가 · 적용 위치: 6탈것 각각의 `riders` 배열과 모든 `skills[].riders` 배열의
`DIMENSIONMASTER` 행 바로 뒤

워로드 행을 복사해 `WARLORD → GUARDIANKNIGHT`, 클립 접두 `wgl_ → ddk_`로 바꾼다. 총 27행
(워로드 행 수와 같음). 예: 테르페이온.

```json
        { "characterClass": "GUARDIANKNIGHT", "idleClip": "ddk_ride_horse_idle_normal_1", "runClip": "ddk_ride_horse_run_normal_1" }
```

```json
            { "characterClass": "GUARDIANKNIGHT", "clips": ["ddk_ride_horse_sk_dash"] }
```

탑승 자세는 `EFTable_Vehicle.RidingMode`를 읽으므로 추가 필드는 없다. `Publish-VehicleProfiles.ps1`은
Server 속도만 다루므로 재실행 불필요(Client 표현 catalog만 변경).

### 5-4. `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Animation/Authored/GuardianKnight/GuardianKnight.interactionbindings.json`

변경 종류: 새 파일

```json
{
  "schema": "lostark.character-interaction-bindings",
  "formatVersion": 1,
  "characterAssetId": "GuardianKnight",
  "bodyAssetId": "Character/GuardianKnight/GuardianKnight.wmodel",
  "terrainJump": {
    "clip": "terrain_jump_short",
    "playRate": 1
  },
  "modes": [
    {
      "mode": "MAZE",
      "skills": [
        {
          "clip": "maze_hammer_q",
          "playRate": 1,
          "effectAssetId": "effect.kouku.cardmaze.q"
        },
        {
          "clip": "maze_hammer_lmb",
          "playRate": 1,
          "effectAssetId": "effect.kouku.cardmaze.lmb"
        }
      ]
    }
  ]
}
```

`Client.vcxproj` `96.DataFiles`에 `<None Include="..\..\Data\Animation\Authored\GuardianKnight\GuardianKnight.interactionbindings.json" />`를
기존 `GuardianKnight.skillbindings.json` 항목 뒤에 추가한다(탐색용 링크, 필터는 같은 필터).

### 5-5. 검증

```text
JSON parse: CharacterCatalog / VehicleCatalog / interactionbindings
compare_attach.py 7개 skeletonHash 일치
Client Debug Build (데이터만이면 불필요, 새 builder는 Python)
사용자: Bern에서 F1 Vehicle Riding 6종 각각 H 탑승 → idle/run → Space/Q/W/E → R 하차
        쿠크 카드미로 MAZE 진입 → Q 뿅망치 → LMB
```

## 6. G04 — HUD·MVP·Loadout 데이터

이 값들은 원래 `Tools/LpkPipeline/build_quickslot_hud_ui.py`, `build_mvp_class_symbols.py`가
TJ PC의 `D:/ClaudeWork/Extracted`(IconInfo.loa, 아이콘 atlas, gfx 덤프)에서 생성한다. 이 PC에는
그 입력이 없다. 그래서 **DB에서 읽을 수 있는 값은 손으로 같은 형식의 행을 쓰고**, 아이콘 crop이
필요한 항목은 입력을 받은 뒤 도구로 만든다. 도구 자체에는 아래 두 줄을 미리 넣어 다음 실행에서
`KeyError`가 나지 않게 한다.

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/LpkPipeline/build_quickslot_hud_ui.py`

변경 종류: 상수 교체 · 적용 위치: `STANCE_BUFFS`(107행), `CLASS_DIR`(109행)

```python
# Warlord Z (17810 = 방어 태세 전환) icon stands for the WARLORD_DEFENSE stance buff;
# Guardian Knight Z (49040 = 화신화) for GUARDIANKNIGHT_DRAGON.
STANCE_BUFFS = [("WARLORD_DEFENSE", 17810, "Warlord"), ("GUARDIANKNIGHT_DRAGON", 49040, "GuardianKnight")]

CLASS_DIR = {"LANCE_MASTER": "LanceMaster", "WARLORD": "Warlord", "ARTIST": "Artist",
             "DIMENSIONMASTER": "DimensionMaster", "GUNSLINGER": "Gunslinger", "SLAYER": "Slayer",
             "GUARDIANKNIGHT": "GuardianKnight"}
```

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/UI/HUD/SkillSlotMarks.json`

변경 종류: 행 추가 · 적용 위치: `marks` 배열 끝. 규칙은 도구와 같다: `skillKind` HOLD→holding,
COMBO→combo, 그 외는 `EFTable_Skill.Type` (2026-09-21 DB 실측: 49000/49001/49020 Type 1,
49021/49150/49200/49210 Type 5, 49110/49270 Type 6, 나머지 Type 1 → 마크 없음).

```json
  {
   "skillId": 49000,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "LMB",
   "mark": "combo",
   "skillKind": "COMBO",
   "retailType": 1
  },
  {
   "skillId": 49001,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "LMB",
   "mark": "combo",
   "skillKind": "COMBO",
   "retailType": 1
  },
  {
   "skillId": 49021,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "SPACE",
   "mark": "holding",
   "skillKind": "HOLD",
   "retailType": 5
  },
  {
   "skillId": 49110,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "W",
   "mark": "combo",
   "skillKind": "COMBO",
   "retailType": 6
  },
  {
   "skillId": 49150,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "D",
   "mark": "holding",
   "skillKind": "HOLD",
   "retailType": 5
  },
  {
   "skillId": 49200,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "A",
   "mark": "holding",
   "skillKind": "HOLD",
   "retailType": 5
  },
  {
   "skillId": 49210,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "A",
   "mark": "holding",
   "skillKind": "HOLD",
   "retailType": 5
  },
  {
   "skillId": 49270,
   "characterClass": "GUARDIANKNIGHT",
   "inputSlot": "F",
   "mark": "combo",
   "skillKind": "COMBO",
   "retailType": 6
  }
```

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/UI/HUD/HudBuffSources.json`

변경 종류: 행 추가 · 적용 위치: `"stance": "WARLORD_DEFENSE"` 객체 바로 뒤

```json
  {
   "source": "stance",
   "stance": "GUARDIANKNIGHT_DRAGON",
   "kind": "buff",
   "iconAsset": "UI/HUD/Buff/buff_stance_guardianknight_dragon.png",
   "iconSource": {
    "table": "EFTable_Skill",
    "primaryKey": 49040,
    "icon": "DDK_Skill_01_23"
   }
  }
```

아이콘 PNG는 `IconInfo.loa` + `EFUI_ICONATLAS` 페이지에서 `crop_icon("DDK_Skill_01_23")`으로
잘라야 하며 입력이 이 PC에 없다. PNG가 없으면 HUD가 이 행을 fail-closed로 격리하는지
`CCombatHUD` 소비자를 먼저 확인하고, 격리되지 않으면 아이콘 확보 전까지 행을 넣지 않는다.

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/UI/MVP/MvpClassSymbols.json`

`classes`에 `GUARDIANKNIGHT` 행을 추가하려면 `class_N` 라벨을 알아야 한다. TJ 09-13 문서의
`_classPlay` 대조표에 DK가 없고(38·40·42~ 후보), 차원술사도 사용자가 40개 아이콘을 보고 직접
지정했다. 같은 절차로 **후보 아이콘을 사용자에게 보여 주고 지정받는다.** 지정 뒤 행:

```json
    {
      "networkClassId": "GUARDIANKNIGHT",
      "classKey": <N>,
      "label": "class_<N>",
      "icon": "MvpClassIcon_<NN>",
      "offsetX": <bigSymbols의 같은 라벨 offsetX>,
      "offsetY": <bigSymbols의 같은 라벨 offsetY>,
      "sourceCode": "DK",
      "smallAsset": "UI/MVP/ClassIcons/MvpClassIconSmall_<NN>.png"
    }
```

`MvpClassIcon_<NN>.png` / `Small` PNG는 `build_mvp_class_symbols.py`가 gfx 덤프에서 잘라야
하므로 TJ 입력이 필요하다.

#### `C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Actors/EquipmentLoadoutPresets.json`

변경 종류: 행 추가 · 적용 위치: 마지막 `"classId": "WARLORD"` 객체 뒤

```json
    {
      "classId": "GUARDIANKNIGHT",
      "slotSelections": {
        "HEAD": null,
        "SHOULDER": null,
        "UPPER": null,
        "LOWER": null,
        "HANDS": null,
        "WEAPON": null
      }
    }
```

같은 파일에 `WARLORD` 행이 두 번 있다(기존 데이터 오류). 손대지 않고 보고만 한다.

## 7. G05 — 발탄 클리어 보상·명예의 속삭임 아이템

앞 네 직업은 `ItemCatalog.json`에 `EQUIP_<CLASS>_HONORWHISPER_{WEAPON,HELMET,SHOULDER,TOP,PANTS,GLOVES}`
6행과 `Valtan.clearrewards.json` 매핑을 갖는다(건슬링어·슬레이어 없음, TJ 09-19).
가디언 나이트용 6행을 만들려면:

1. `EFTable_GameMsg.db`에서 `'명예의 속삭임'`을 포함하는 `tip.name.item_*` 중 `EFTable_Item`
   `ReClass1~6`이 702인 행 6개(부위별)를 찾는다. 2026-09-21 실측: 무기 모델 `WP_WDDK_04` 참조
   아이템이 1,165행이고 `Icon='DDK_Item'`, `IconIndex 71/72`이므로 아이콘 이름은 `DDK_Item_<IconIndex>`.
2. 아이콘 PNG `UI/Items/GuardianKnight/honorwhisper_*.png`는 `crop_icon`(IconInfo + atlas) 입력이
   필요하다 → G04와 같은 의존성.
3. `ItemCatalog.json` 6행(워로드 행 복사, `characterClass: "GuardianKnight"`, `grade: "ancient"`),
   `Valtan.clearrewards.json`에 `"GUARDIANKNIGHT": [ ...6 ID... ]`.
4. `Publish-ItemCatalog.ps1 -Mode Publish`, `Publish-ValtanClearRewards.ps1 -Mode Publish`,
   Server 재시작.

## 8. G06 — 사운드 catalog

`build_sound_catalog.py`는 `Data/Animation/Authored/<Class>/<Class>.animevents`의 SOUND 행을
원본 wav 라이브러리(`D:\로아 리소스\Sound\Character\<Class>`)에서 찾아 복사한다. 이 PC에 그
라이브러리가 없다. 도구 `CLASSES`에 `"GuardianKnight"`를 추가하고, 라이브러리를 받은 뒤
`--raw-sound-root <경로> --dry-run`으로 매칭률을 확인한 다음 실제 실행한다. 원본 이벤트 이름은
`PC_DragonKnight_*` 계열일 가능성이 높으나 animevents SOUND payload가 정본이다.

## 9. 적용 순서

1. G01 상수 교체 → Client Debug Build → 사용자 헤어 확인
2. G02 extract → verify 게이트 → 해시 분류 → generate/install → 그룹 경계 3곳 + 테스트 →
   textures → rows → catalog 삽입 → Product Debug Build → 사용자 화면 확인
3. G03 MazeHammer builder 실행 → 탈것 rider builder 작성·실행 → compare_attach → catalog/
   VehicleCatalog/interactionbindings → 사용자 탑승·카드미로 확인
4. G04 도구 상수 + DB 기반 행(SkillSlotMarks, Loadout) → 아이콘 입력 확보 후 HudBuffSources·MVP
5. G05·G06은 입력 확보 후

## 10. Drive 전달 목록 (Git 비추적)

```text
Client/Bin/Resources/Character/GuardianKnight/textures/*  (G02가 새로 넣는 _cm/_orm/_m/_e 등)
Client/Bin/Resources/Character/WP_WDDK_04/textures/*      (같음)
Client/Bin/Resources/Character/GuardianKnight/AnimSets/GuardianKnight_Ride{Horse,Swing,Hoverboard,HeavywalkerBm9,Tube,Dragon2}AnimSet.wmodel
Client/Bin/Resources/Character/GuardianKnight/AnimSets/GuardianKnight_MazeHammerAnimSet.wmodel
Client/Bin/Resources/UI/HUD/Buff/buff_stance_guardianknight_dragon.png   (G04, 입력 확보 후)
Client/Bin/Resources/UI/MVP/ClassIcons/MvpClassIcon{,Small}_<NN>.png       (G04, 입력 확보 후)
Client/Bin/Resources/UI/Items/GuardianKnight/honorwhisper_*.png            (G05, 입력 확보 후)
```

## 11. 범위 밖

- 커스터마이징 헤어/의상 세트(`EquipmentPresentationCatalog.json`), 아이덴티티 게이지 HUD
  (`UI/HUD/IdentityAnimation`), ClassSelect 일러스트·태그 아트, 이펙트·사운드 재생 연결.
- `BAKED_HAIR`를 다시 켜는 것은 헤어 세트가 생겼을 때 한다.
