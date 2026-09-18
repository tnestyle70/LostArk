# 창술사 기본 갑옷 원본 재질 복원 PLAN

2026-09-18. 브랜치 `feature/character-customizing-rendering`.

## 문제

사용자가 커스터마이징 창에서 목 뒤에 뚜렷한 경계선을 관찰했다. 얼굴 아래 피부·갑옷이
얼룩져 보인다.

## 실측한 원인

`Logic_LanceMaster.cpp:57`이 body의 맨살 submesh 0/1/2(arm/upper/lower)를 항상 숨긴다.

```cpp
constexpr uint32_t COVERED_BY_ARMOUR = (1u << 0) | (1u << 1) | (1u << 2);
```

`Character.cpp:2472`와 `3625`가 `iBodyHiddenMeshMask`로 이 값을 무조건 적용하므로,
얼굴 아래 화면에 보이는 피부는 전부 갑옷 파츠가 소유한다. 그 다섯 파츠에
`CharacterCatalog.modelMaterialOverrides` 행이 없어 `MODEL_SURFACE_FAMILY::SOURCE_CHARACTER`가
되지 못하고 일반 `PS_MAIN` Blinn-Phong으로 그려진다.

정점 Z 범위(음수가 위)로 경계 위치를 확정했다.

| 메시 | Z 범위 | 경로 |
|---|---|---|
| `pc_ft_face_mi` | -126.5 .. -112.1 | source program 4 |
| `pc_flm_00_upper_mi` | -114.7 .. -19.0 | 행 없음 |

`pc_flm_00_upper_mi`가 목 피부를 들고 있고 얼굴 메시가 끝나는 높이에서 시작한다.
목 뒤 경계선은 두 셰이딩 경로의 경계다.

원본 스위치가 이 추론을 독립적으로 확인한다. `1.use_skin`이 창술사 상의·하의·팔에서만
`True`다.

## 원본 재질 대조

`extract_source_map_material_parameters.py`로 9개 MIC를 뽑아 대조했다.

| 클래스 | 마스터 | terminal |
|---|---|---|
| 도화가 `pc_sdm_00_*` | `sp_parts_msk_high` | `pbr_base_msk` |
| 워로드 `pc_wgl_00_*` | `wr_parts_msk_high` | `pbr_base_msk` |
| 창술사 `pc_flm_00_*` | `ft_parts_msk_high` | `pbr_base_msk` |

창술사와 워로드는 파라미터 95 / 텍스처 22 / 스위치 35의 이름 집합이 완전히 같다.
35개 스위치 중 갈리는 것은 넷뿐이다.

| switch | wgl_up | wgl_lo | sdm_up | sdm_lo | flm_up | flm_lo | flm_arm | flm_sh | flm_hel |
|---|---|---|---|---|---|---|---|---|---|
| `1.use_dyeing` | F | F | T | T | F | F | F | F | F |
| `1.use_emissive` | T | F | F | F | F | F | F | F | F |
| `1.use_mask_variation` | T | T | T | T | F | F | F | F | T |
| `1.use_skin` | F | F | F | F | T | T | T | F | F |

`build_vehicle_source_material.py extract`의 셰이더 맵 해시가 같은 분할을 준다.

```
flm_upper / lower / arm   base=4c19dc6b… light=4693c9c0…
flm_shoulder              base=0d0eb237… light=3ffaa7d2…
flm_helmet                base=c4bf6016… light=020eea47…
wgl_lower                 base=c4bf6016… light=020eea47…
```

헬멧과 워로드 하의가 컴파일된 셰이더까지 같다. 따라서 헬멧은 기존 program 2
(`classic-variation.v1`)를 그대로 쓰고, 신규 program은 두 개만 필요하다.

## 적용 계획

| G | 내용 |
|---|---|
| G01 | program 90 `source.character.classic-armor-skin.v1` 설치 (상의·하의·팔) |
| G02 | program 91 `source.character.classic-armor-plain.v1` 설치 (어깨) |
| G03 | CSO 그룹 경계와 `Model.cpp` 상한을 91까지 확장 |
| G04 | 필요한 원본 TGA 3개 추가 설치 |
| G05 | `CharacterCatalog.json`에 다섯 행 추가 |

### G03의 손편집 세 곳

`Tools/EffectPipeline/native_shader_dispatch.py:405`

```python
SOURCE_CHARACTER_PROGRAM_GROUPS = ((1, 8), (9, 16), (17, 24), (25, 32), (80, 83), (84, 91))
```

`Engine/Private/Shader.cpp:447`

```cpp
		constexpr uint32_t ranges[][2] = { {1u,8u}, {9u,16u}, {17u,24u}, {25u,32u}, {80u,83u}, {84u,91u} };
```

`Engine/Private/Model.cpp:1829`

```cpp
            if (source.program == 0u || source.program > 91u || (source.program > 65u && source.program < 80u) ||
```

`Tools/EffectPipeline/test_source_character_program_groups.py:48`의 미등록 센티넬은 90이
등록되므로 92로 올린다.

```python
        source = source.replace('SourceCharacterBase88(', 'SourceCharacterBase92(')
```

기존 그룹 84~89를 84~91로 늘리므로 새 FX 파일과 project 등록은 생기지 않는다.
이는 `CLAUDE.md`의 "CSO 변형 그룹 경계와 `Model.cpp` 상한을 함께 늘린다"를 따른 것이다.

### 실행 명령

```powershell
$py = 'C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe'
$tool = 'Tools/VehiclePipeline/build_vehicle_source_material.py'
$umodel = 'C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe'
$d3d = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\d3dcompiler_47.dll'
$work = 'out/LanceMasterArmour20260918'

& $py $tool extract --umodel $umodel --d3dcompiler $d3d --out "$work/dumps" `
  pc_flm_00.mat.pc_flm_00_upper_mi pc_flm_00.mat.pc_flm_00_lower_mi `
  pc_flm_00.mat.pc_flm_00_arm_mi pc_flm_00.mat.pc_flm_00_shoulder_mi `
  pc_flm_00.mat.pc_flm_00_helmet_mi pc_wgl_00.mat.pc_wgl_00_lower_mi

& $py $tool verify --dump "$work/dumps/pc_wgl_00.mat.pc_wgl_00_lower_mi.json" `
  --family source.character.classic-variation.v1 --program 2

& $py $tool generate --dump "$work/dumps/pc_flm_00.mat.pc_flm_00_upper_mi.json" `
  --family source.character.classic-armor-skin.v1 --program 90 --out "$work/generated90"
& $py $tool install --generated "$work/generated90" `
  --family source.character.classic-armor-skin.v1 --program 90

& $py $tool generate --dump "$work/dumps/pc_flm_00.mat.pc_flm_00_shoulder_mi.json" `
  --family source.character.classic-armor-plain.v1 --program 91 --out "$work/generated91"
& $py $tool install --generated "$work/generated91" `
  --family source.character.classic-armor-plain.v1 --program 91

& $umodel -export -uncook -game=lostark -kr -nameresolve `
  -path='C:\ProgramData\Smilegate\Games\LOSTARK\EFGame' -out="$work/staging" PC_FLM_00
& $py $tool textures --texture-map Tools/VehiclePipeline/LanceMasterArmour.texture-map.json `
  --staging "$work/staging"
```

`rows`는 모델마다 한 번씩 실행해 `$work/rows/{upper,lower,arm,shoulder,helmet}.json`을 만들고
`CharacterCatalog.json`의 `PLAYER_LANCE_MASTER.modelMaterialOverrides`에 합친다.

## 검증 기준

- `verify`가 다섯 슬롯 전부 `base/light/configure EXACT`
- `test_source_character_program_groups.py` 통과
- `CharacterCatalog.json` parse와 `git diff --check`
- Product 빌드
- 사용자의 실제 커스터마이징 화면 확인

## 범위 밖

- body의 `pc_ft_01_arm_mi`, `pc_ft_08_hair_mi`는 `COVERED_BY_ARMOUR`/`BAKED_HAIR`로 숨겨져
  화면에 나오지 않으므로 이번에 다루지 않는다.
- 커스터마이징 장비 281개(`EquipmentPresentationCatalog.json`)는 별개 건이다.
- 도화가·차원술사의 얼굴↔몸 `half_lambert_skin` 단차도 별개 건이다.
