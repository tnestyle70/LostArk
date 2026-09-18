# 창술사 기본 갑옷 원본 재질 복원 RESULT

2026-09-18. 브랜치 `feature/character-customizing-rendering`. 계획은 같은 폴더의 PLAN을 따른다.

## 구현 완료

| G | 상태 | 내용 |
|---|---|---|
| G01 | 완료 | program 90 `source.character.classic-armor-skin.v1` 설치 |
| G02 | 완료 | program 91 `source.character.classic-armor-plain.v1` 설치 |
| G03 | 완료 | CSO 그룹 84~89 → 84~91, `Model.cpp` 상한 89 → 91, 테스트 센티넬 90 → 92 |
| G04 | 완료 | `pc_flm_00_helmet_m`, `pc_flm_00_upper_d_loc_int`, `pc_flm_00_lower_d_loc_int` 설치 |
| G05 | 완료 | `CharacterCatalog.json`에 다섯 행 추가 |

## 실행한 검증

### 생성기 게이트

설치 전에 이미 설치돼 있던 program 2를 원본 MIC에서 다시 만들어 바이트 일치를 확인했다.

```
verify pc_wgl_00_lower_mi --family source.character.classic-variation.v1 --program 2
  base EXACT 639 lines / light EXACT 641 lines / configure EXACT
```

### 설치 후 다섯 슬롯 되검증

```
pc_flm_00_upper_mi     -> program 90   base EXACT 651 / light EXACT 721 / configure EXACT
pc_flm_00_lower_mi     -> program 90   base EXACT 651 / light EXACT 721 / configure EXACT
pc_flm_00_arm_mi       -> program 90   base EXACT 651 / light EXACT 721 / configure EXACT
pc_flm_00_shoulder_mi  -> program 91   base EXACT 623 / light EXACT 625 / configure EXACT
pc_flm_00_helmet_mi    -> program  2   base EXACT 639 / light EXACT 641 / configure EXACT
```

헬멧이 신규 program 없이 기존 program 2로 정확히 재현된다.

### 그 밖

- `test_source_character_program_groups.py` 3 tests OK
- `CharacterCatalog.json` JSON parse 정상, characters 6
- `git diff --check` clean
- catalog diff는 1520줄 삽입 / 0줄 삭제 — 재직렬화가 기존 내용을 보존했다

### 재질 행 커버리지 (도구 대조)

```
LanceMaster_Upper.wmodel      materials=1 covered=1
LanceMaster_Lower.wmodel      materials=1 covered=1
LanceMaster_Arm.wmodel        materials=1 covered=1
LanceMaster_Shoulder.wmodel   materials=1 covered=1
LanceMaster_Helmet.wmodel     materials=1 covered=1
```

`LanceMaster.wmodel`의 `pc_ft_01_arm_mi`(2030정점)와 `pc_ft_08_hair_mi`(1239정점)는
`COVERED_BY_ARMOUR`/`BAKED_HAIR`로 숨겨진 submesh라 행이 없어도 화면에 나오지 않는다.

## 빌드

Product Debug 빌드 PASS, 오류 0. Engine OBJ 2 / CSO 7, Client OBJ 4 / CSO 21 / 링크 2.
CSO 28개는 그룹 084를 include하는 FX가 program 90/91 추가로 재컴파일된 것이다.
아래 `추가 조사`의 변경분은 그 뒤 두 번의 Client 빌드로 각각 반영했다.

## 팀 전달 필요

`Client/Bin/Resources`는 Git 비추적 물리 폴더다. 아래 TGA 3개가 이번에 새로 추가됐으므로
팀원에게 Drive로 따로 전달해야 한다.

```
Character/LanceMaster/textures/pc_flm_00_helmet_m.tga
Character/LanceMaster/textures/pc_flm_00_upper_d_loc_int.tga
Character/LanceMaster/textures/pc_flm_00_lower_d_loc_int.tga
```

기존 18개 TGA는 바이트가 이미 일치해 교체하지 않았다.

## 이번에 확인한 재사용 가능한 사실

- 게임 패키지가 이 PC의 `C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages`에
  있다(`.upk` 33,909개). 원본 재질 추출은 막혀 있지 않다. 이전에 "추출본이 없다"고 본 것은
  umodel 출력물이 없었던 것이지 원본 패키지 부재가 아니었다.
- `build_vehicle_source_material.py`는 이름과 달리 탈것 전용이 아니다. GPU-skin
  BasePass/directional light PS를 쓰는 SourceCharacter 재질 전반에 쓸 수 있다.
- 셰이더 맵 해시가 같으면 다른 클래스의 MIC라도 같은 설치 program을 재사용할 수 있다.
  이름이나 파라미터 일치가 아니라 해시로 판정한다.

## 범위 밖으로 남긴 것

- `EquipmentPresentationCatalog.json`의 커스터마이징 장비 281개 중 279개가 원본 행 없이
  일반 경로로 그려진다. 별개 건이다.
- 도화가·차원술사는 얼굴 `half_lambert_skin` R=1.0, 몸 R=0.851252로 데이터상 15% 단차가
  있다. 창술사에는 이 단차가 없다. 별개 건이다.
- 워로드 얼굴은 `classic-head-legacy.v1`이고 `half_lambert_skin`이 없다. 미추적.

---

## 추가 조사 (같은 날, 사용자 화면 확인 이후)

### 복원한 파츠가 그려지지 않고 있었다

program 90/91 설치와 catalog 행 추가를 마친 뒤에도 사용자 화면은 변화가 없었다. 원인은
재질이 아니라 가시성이었다.

`Character.cpp`의 `Apply_DefaultEquipmentVisibility`:

```cpp
else if (EQUIPMENT_SLOT_KIND::DEFAULT == equipment.eSlotKind && hasAvatarArmor)
    isVisible = false;
```

창술사 spec이 `Part_15_Avatar_Head` / `Part_15_Avatar_Armor`(모코코 아바타)를 항상 들고
있었고, 아바타의 존재만으로 `DEFAULT` 파츠 전체가 숨겨진다. 따라서 이번에 복원한
`pc_flm_00_*` 다섯 파츠는 한 번도 draw되지 않았다. 재질 행을 두 번 고치는 동안 대상이
화면에 없었다.

교훈: 재질을 고치기 전에 그 파츠가 실제로 그려지는지부터 확인한다. `modelMaterialOverrides`
행이 붙었다는 것과 그 submesh가 보인다는 것은 별개다.

### MIC variant 오선택

처음에 upper/lower를 base MIC(`pc_flm_00_upper_mi`)로 바인딩했으나, 쿠킹된 wmodel은
`-1` variant에서 나왔다. 판정 근거는 wmodel WMA3에 구워진 dye 틴트다.

```
wmodel upper  diffuse=[0.963,0.862,1.0] A=[0.343,0.068,0.137] B=[0.751,0.151,0.415]
pc_flm_00-1_upper_mi  0.962724, 0.861884, 1 / 0.342529, 0.0681374, 0.137469 / 0.751354, 0.150715, 0.414711
```

upper/lower는 `-1`, arm/shoulder/helmet은 base다(후자는 틴트가 전부 1.0이고 `_d` 텍스처).
슬롯 이름은 메시 기본 이름이 유지되므로 `rows`에 `@pc_flm_00_upper_mi`로 지정했다.
`-1` variant의 셰이더 맵 해시는 base와 같아 program 90은 그대로다.

### 기본 착용에서 모코코 아바타 제거

사용자 결정으로 모코코 아바타는 런타임 장착 장비로 돌리고 기본은 미착용으로 바꿨다.
개수 계약이 세 곳에 묶여 있어 함께 줄였다.

| 파일 | 변경 |
|---|---|
| `Logic_LanceMaster.cpp` | `Part_15_Avatar_Head` / `Part_15_Avatar_Armor` 파츠 제거 (8 → 6) |
| `PlayableCharacterAssetService.cpp` | LANCE_MASTER 프로토타입 태그 2개 제거, `8u` → `6u` |
| `CharacterCatalog.json` | `equipmentModels`에서 모코코 2개 제거 (8 → 6) |

`ActorCatalog.cpp:222`가 `equipmentModels`에 없는 모델의 재질 행을 거부하므로 모코코
재질 행 3개도 함께 제거했고, 복구용으로
`out/LanceMasterArmour20260918/mokoko-avatar-rows.json`에 보존했다.
`AVATAR_HEAD`/`AVATAR_ARMOR` enum과 `AvatarBookWindowView` 경로는 그대로 두었다.

### 캐릭터 생성창의 의상 자동 착용 제거

`CustomizingView::Open()`이 매번 `m_bCostumeChanged = true`를 세워 의상 목록 0번을 즉시
입히고 있었다. 기본은 클래스 기본 장비, 의상은 클릭했을 때만 입도록 바꿨다.

- `COSTUME_NONE = -1`을 도입하고 `m_iSelectedCostume` 기본값을 `0`에서 여기로 옮겼다.
  `0`으로 두면 목록 0번 클릭이 "변경 없음"으로 무시된다.
- 저장 문서에 `costume` 키가 없을 때의 기본값도 `COSTUME_NONE`으로 맞췄다.
- `CLevel_CharacterSelect::Remove_CustomizingCostume()`을 추가해 선택 해제 시 그 클래스의
  의상 세트만 벗기고 머리 등 나머지 착용은 보존한다.

### 사용자 확인

사용자가 실제 화면에서 "지금 내가 원하는대로 작동 됨"으로 의상 착용 동작을 확인했다.
**목 뒤 경계의 해소 여부는 아직 서면으로 확인되지 않았다.**

### 남은 후보

경계가 계속 보이면 얼굴(program 4)과 갑옷 피부(program 90)의 다음 두 값이 남은 차이다.
두 값은 원작 추출값이며 body 맨살(`pc_ft_01_upper_mi`)은 갑옷 쪽과 같다.

| | 얼굴 | 갑옷 피부 / body 맨살 |
|---|---|---|
| `skin_specular_power` | 20.0 | 16.0 |
| `beckmannspecular_constant_max` | 3.5 | 3.35 |
