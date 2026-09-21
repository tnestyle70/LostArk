# 가디언 나이트 외형 복원·데이터 동기화 — 결과

작성자: JS · 2026-09-21 · 브랜치 `feature/player-guardian-knight` (a9a703ca 위)
계획서: `2026-09-21_GUARDIANKNIGHT_RESTORE_SYNC_PLAN.md`
MVP 직업 심볼은 사용자 결정으로 범위에서 제외했다(UI 담당자가 나중에 넣음).

## 구현 상태

| G | 상태 | 내용 |
|---|---|---|
| G01 헤어 | 완료 | `Logic_GuardianKnight.cpp` `BAKED_HAIR = 0u` + program 99를 pass 9(forward 반투명 양면)로 라우팅 |
| G02 재질 | 완료 | MIC 16개 추출, program 94~99 신설 + 1 재사용, 텍스처 복사, catalog 15행 |
| G03 애니셋 | 완료 | Ride×6 + MazeHammer 7개 codec 경로로 생성·설치, VehicleCatalog 27행, interactionbindings |
| G04 HUD 데이터 | 완료 | SkillSlotMarks 8행, HudBuffSources 화신화 스탠스 + 아이콘, LoadoutPresets, 도구 상수 |
| G05 아이템·발탄 보상 | 완료 | 명예의 속삭임 6부위 ItemCatalog + clearrewards, 아이콘 6장, 두 publisher Publish 완료 |
| G06 사운드 | 미착수 | 원본 wav 라이브러리가 이 PC에 없음 |

## G02 실측

### 해시 분류 (`extract` shaderId, base / light)

| MIC | base | light | 판정 |
|---|---|---|---|
| `pc_ddk_00_{upper,upper1,upper2,upper3,lower,lower1,arm}_mi` | 814abef0… | 7a029052… | 신규 **94** `source.character.realpbr-avatar-ddk.v1` |
| `pc_ddk_00_{shoulder,helmet}_mi` | 1192070f… | 4e96a2f1… | 신규 **95** `…realpbr-avatar-ddk-plate.v1` |
| `pc_dk_av_base_upper_mi` | 7d17844b… | c9cc424f… | 기존 **1** `classic-skin.v1` verify EXACT |
| `wp_wddk_04_mi` | 8932162a… | 7c61b712… | 신규 **96** `…realpbr-weapon-ddk.v1` (program 8은 light만 EXACT, base MISMATCH) |
| `sk_ddk_win_00_mi` | 6134b047… | 5d8a7cf3… | 신규 **97** `…realpbr-wing-ddk.v1` |
| `sk_ddk_win_00-1_mi` | 6a904bb0… | d8fb612a… | 신규 **98** `…realpbr-wing-ddk-membrane.v1` |
| `pc_dl_eye_mi` | 89aca021… | 2c76bd80… | 기존 **5** `eye.v1` verify EXACT — **행 제거**: `Model.cpp`가 program 5에 UV1·UV2를 요구하는데 GK 바디는 WModel 1.0(extra UV 없음)이라 바디 전체가 `Class model preparation failed (E_FAIL)`로 거부됐다(사용자 첫 실행에서 확인) |
| `pc_ft_15_hair_mi` | dea8fd54… | 3b0966d4… | 신규 **99** `…hair-ddk.v1` (7/18/19/20 전부 MISMATCH) |
| `pc_dl_eyeao_mi` | 9859dede… | eed43b78… | `verify`/`generate`가 `IndexError`로 중단(생성기 한계) → 행 없음 |
| `pc_dl_face_mi_high` | — | — | `extract` 실패: "referenced texture table is not at the static-resource offset" (도구가 이 MIC의 tail 레이아웃을 못 읽음) |

차원술사 program 3(realpbr-avatar-v2)은 갑옷 7슬롯 전부 base/light MISMATCH였다. 같은 realpbr 이름이라도
해시가 달라 재사용하지 않았다.

### 설치 검증

```
verify 94  base EXACT 787 / light EXACT 503 / configure EXACT
verify 95  base EXACT 835 / light EXACT 503 / configure EXACT
verify 96  base EXACT 797 / light EXACT 467 / configure EXACT
verify 97  base EXACT 779 / light EXACT 419 / configure EXACT
verify 98  base EXACT 707 / light EXACT 419 / configure EXACT
verify 99  base EXACT 450 / light EXACT 517 / configure EXACT
test_source_character_program_groups  3 tests OK (센티넬 92 → 100)
```

그룹 경계 `84~93 → 84~99`: `native_shader_dispatch.py`, `Shader.cpp`, `Model.cpp`(상한 93 → 99).
`install`은 그룹 등록 전에는 `needs a registered CSO cohort`로 거부하므로 경계를 먼저 올려야 한다.

### 텍스처

`Tools/VehiclePipeline/GuardianKnight.texture-map.json` 74항목. `textures`가 기존 25장과 바이트 일치를 확인하고
`_cm/_m/_orm/_e` 등을 새로 복사했다. 공용 lookup `fx_d_noise_033_ta.tga`(날개 97)는
`Character/SourceMaterials/efmaster_material_prologue/`에 없어 스테이징에서 복사했다.
base MIC `pc_ddk_00_lower1_mi`가 `pc_ddk_00-1_lower1_d`를 참조하므로 1차 쿠킹의 텍스처 선택은 맞았다.

### catalog

`PLAYER_GUARDIANKNIGHT.modelMaterialOverrides` 15행(Body 헤어 1 / Upper 5 / Lower 2 / Arm 2 / Shoulder 1 / Helmet 1 / Wing 2 / Weapon 1).
텍스트 삽입이라 다른 클래스 행은 그대로다. JSON parse, characters 7, 모든 행이 `equipmentModels/weaponModels`에 있음.

바디는 헤어(99) 한 행만 넣었다. 눈(5)은 바디를 1.3+로 재쿠킹해 UV1/UV2를 보존해야 쓸 수 있다. 얼굴 `pc_dl_face_mi_high`는 `extract`가, 속눈썹 `pc_dl_eyeao_mi`는 `generate`가
도구 한계로 실패해 일반 경로로 남는다(건슬링어와 같은 상태). 두 MIC의 도구 보강은 별개 건.

### 헤어 program 99 추가 조치 (사용자 화면 확인 후)

1. 검정: 헤어 program(6/7/18)은 `Part_Body`/`Part_Equipment`가 pass 9로 보내고 deferred를 건너뛰는데 99가 목록에 없었다.
   `Resolve_TranslucentSourcePass` 두 곳, `Shader_VtxAnimMeshBinary.hlsl` 반투명 PS 허용·`SourceCharacterLight99` dispatch,
   `Shader_SourceCharacterMaterial.hlsli` 헤어 입력 레이아웃·coverage, `Shader_Deferred.hlsl` shadow adapter에 99 추가.
2. 빡빡이: 반투명 PS는 opacity 0이면 discard. 99 base의 알파가 `텍스처 알파 × cb0[1].w`인데 cb0[1]은 엔진이 안 채우는 행이라 0.
   program 7과 같은 손편집 줄 `source[0].x=1.0; source[1].w=1.0;`을 `Shader_SourceCharacterBaseGroup084.hlsli`의 Base99에 넣었다
   (generator가 base 단계의 leading unowned 행을 기본값으로 채우지 않는다 — 7도 같은 손편집). 이 줄 때문에 `verify 99`는 이제 MISMATCH가 정상.
3. 염색: 일반 경로에서는 `pc_ft_15_hair_d`가 2톤 마스크라 시안/마젠타로 보인다. 값은 `XmlData/CharacterCustomizing/EFDLCHAR_PC_DK.PC_DK_00.loa`
   프리셋 1번: 기본색 (1.0, 0.839, 0.863), 2톤색 (0.658, 0.638, 0.584), 2톤 on, range 0.42/0.546, edge 1.0, hardness 0.5, spec 0.804/0.898.
   프리셋 00~24가 같은 폴더에 있어 색을 바꾸려면 번호만 고르면 된다.
4. 빌드 3·4차 PASS(Engine CSO 7 / Client CSO 21, 이후 Client CSO 2). 빌드 가드는 Server도 종료를 요구한다.

## G03 실측

- 1차 RESULT §2.4의 "Delain psa에 망치 클립 없음"은 틀렸다. `pc_dl_00_ani.psa`(1,067)에 `pr_it_gstfp_00_att_1_01/2_01`과
  horse/hoverboard/heavywalker_bm9/tube 전부, `pc_dl_00_vehicle_ani.psa`(44)에 swing/dragon_2가 있다.
- 새 builder `Tools/ActorXAssetCooker/build_vehicle_rider_animations.py`: VehicleCatalog의 WARLORD 행에서 클립 집합을 읽고
  카드미로 builder의 `carrier/subset`을 재사용한다. 40바이트 section 이름 절단(`feelsgoodspeedin`, `powershould`)은
  참조 이름이 이미 잘린 경우 그 접두를 가진 유일한 원본 클립으로 되맞춘다.
- `build_card_maze_player_animations.py`에 `--classes` 추가(지형 점프 builder와 같은 방식).
- `compare_attach.py` 7개 전부 validate OK, 285본 table_match, skeletonHash 일치.
- `VehicleCatalog.json` GUARDIANKNIGHT 27행 = WARLORD 행의 `wgl_ → ddk_`. 모든 clip 이름이 설치된 애니셋 section에 존재함을 스크립트로 확인.

## G04 실측

- `SkillSlotMarks.json` 8행: 49000/49001/49110/49270 combo, 49021/49150/49200/49210 holding. `retailType`은
  언팩 `EFTable_Skill.db` 실측(Type 1/5/6).
- `HudBuffSources.json` `GUARDIANKNIGHT_DRAGON` 행 + `UI/HUD/Buff/buff_stance_guardianknight_dragon.png`(64×64).
  아이콘은 이 PC의 `LPK/data3/.../IconInfo.loa`(`DDK_Skill_01_23` → `ddk_skill_1` 페이지 (384,64))와
  umodel `EFUI_ICONATLAS_D` export에서 순수 python으로 잘랐다. TJ PC 입력 없이 가능했다.
- `MainApp.cpp` stance buff 판정이 `WARLORD_DEFENSE` 하드코딩이라 `GUARDIANKNIGHT_DRAGON` 분기를 추가했다.
- `build_quickslot_hud_ui.py` `STANCE_BUFFS`/`CLASS_DIR`에 가디언 나이트 추가(다음 실행 시 KeyError 방지).
- `EquipmentLoadoutPresets.json` GUARDIANKNIGHT 행. 기존 파일에 WARLORD 행이 두 번 있다(손대지 않음).

## 빌드

1차 Product Debug(94~98, 경계 98): Engine/Shared/Server/Client PASS. Client OBJ 6 / CSO 21 / 링크 2, tracking identity 변경 없음.
2차 Product Debug(99 추가, 경계 99): Engine OBJ 2 / CSO 7, Client OBJ 4 / CSO 21 / 링크 2, 전부 PASS. 오류 0.
실행 파일: `Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`(Server는 변경 없음, 재시작만 필요).

## 사용자 확인

확인 완료(2026-09-21 사용자 서면): 갑옷·무기 색, 화신화 날개, 헤어 표시·프리셋 00 색, 탈것 탑승, 화신화 버프 아이콘.

남은 확인:

1. Character Select → 가디언나이트: 헬멧 관통 여부, 헤어 각짐 여부
3. 쿠크 카드미로 MAZE: Q 뿅망치, LMB
4. HUD: W/F 콤보 마크, A/D/SPACE 홀딩 마크

## Drive 전달 (Git 비추적)

```
Client/Bin/Resources/Character/GuardianKnight/textures/            (신규 _cm/_m/_orm/_e 등 22장)
Client/Bin/Resources/Character/WP_WDDK_04/textures/                (wp_wddk_04_{cm,e,orm}.tga)
Client/Bin/Resources/Character/SourceMaterials/efmaster_material_prologue/fx_d_noise_033_ta.tga
Client/Bin/Resources/Character/GuardianKnight/AnimSets/GuardianKnight_{RideHorse,RideSwing,RideHoverboard,RideHeavywalkerBm9,RideTube,RideDragon2,MazeHammer}AnimSet.wmodel
Client/Bin/Resources/UI/HUD/Buff/buff_stance_guardianknight_dragon.png
Client/Bin/Resources/UI/Items/GuardianKnight/honorwhisper_{weapon,helmet,shoulder,top,pants,gloves}.png
```

## G05 실측

- `EFTable_GameMsg` `명예의 속삭임` + `EFTable_Item` Model `WP_WDDK_03-11` / `PC_DDK_03-2_*` → 133611710~133611715
  (할버드·투구·상의·하의·장갑·견갑), 아이콘 `DDK_Item_{70,54,50,51,53,52}` → `ddk_item_0` 페이지에서 crop.
- `ItemCatalog.json` 6행(`characterClass: GuardianKnight`, `grade: ancient`), `Valtan.clearrewards.json` `GUARDIANKNIGHT`.
- `Publish-ItemCatalog.ps1` Validate 52 items → Publish, `Publish-ValtanClearRewards.ps1` Validate 30 items → Publish.
  publisher의 class 표에는 1차 커밋 때 이미 GUARDIANKNIGHT가 있었다. **Server 재시작 후 적용.**

## 남은 것

- G06 사운드 catalog: 원본 wav 라이브러리(`D:\로아 리소스`)가 이 PC에 없다.
- 얼굴 `pc_dl_face_mi_high`(extract), 속눈썹 `pc_dl_eyeao_mi`(generate) 도구 보강.
- MVP 직업 심볼: UI 담당자 몫(사용자 결정).
- 쿨타임: 3000은 테스트값. 원작 `EFTable_Skill.Cooltime`은 이 PC 테이블에만 있고 receipt에 근거 행 없음. 전 클래스 일괄 복원 때 같이 한다(사용자 결정).
  그때 `CooltimeGroup`도 필요: SPACE 49020/49021 = 그룹 49020, Z 49040/49041 = 그룹 49040으로 쿨 공유(원작·사용자 확인). 프로젝트에는 아직 그룹 개념이 없다.
- 딜량 16개는 `EFTable_SkillEffect` SK=10 최저 변형 `ValueA`와 일치(LMB만 프로젝트값 100). R 49130은 원작 변형 0~3이 각 666이라 다단 합이 더 클 수 있음.
- 콜라이더: hitshapes v4 16스킬, HIT 86행 전부 `src=orig`, bootstrap `SKILLHIT` 9 + `SKILLSTAGEHIT` 13 게시 확인.
