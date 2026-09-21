# 가디언 나이트 클래스 추가 — 구현 계획

작성자: JS · 2026-09-21 · 예정 브랜치 `feature/player-guardian-knight` (현재 `feature/player-dodge-cancel-window` 위에 만들지 않고 `main`에서 분기)

일곱 번째 playable class를 추출부터 인게임 렌더·스킬 사용까지 연결한다. 워로드(08-07)와
같은 파이프라인을 쓰되, 실측으로 확인한 차이가 이 문서의 핵심이다. 실제 clip 이름·submesh
순서·bone 이름은 G01~G02 쿠킹 결과에서 읽어야 하므로 코드 전문은 각 G를 진행할 때 대화에서
현재 파일 기준으로 낸다. 이 문서는 좌표·판정·순서·검증의 정본이다.

## 0. 결론과 범위

| 단계 | 내용 | 완료 증거 |
|---|---|---|
| 1차 (이 계획) | 바디+기본 방어구+할버드 렌더, 로코모션, LMB 콤보 + 고정 퀵슬롯(§6.2), **화신화 스탠스(Z 토글, 스탠스별 슬롯 분기, identity 로코모션)**, Server 판정 | Lobby → Character Select → 가디언 나이트 렌더, 아레나에서 두 스탠스의 스킬 재생·데미지 |
| 2차 (후속) | 화신화 날개 파츠 표시·아이덴티티 게이지 조건, 원본 PBR 재질 program, 루트모션, 이펙트/사운드, 커스터마이징, 탈것, 장비 catalog, UI 아트 | 별도 PLAN |

2026-09-21 사용자 확정: 슬롯은 고정 바인딩이고 화신화 상태에서 A/S/F/SPACE만 바뀌며 나머지 슬롯은 일반 상태와
같다. T 딥 임팩트는 화신화 상태에서만 쓴다. 따라서 스탠스는 1차다.

1차 최소 집합은 **건슬링어·슬레이어가 지금 실제로 갖고 있는 것**과 같다. 두 클래스는 커스터마이징·
탈것·ClassSelect 상세·MVP 심볼·발탄 보상·projectiles·cancelwindows·rootmotion 없이 supported로
실행된다(fail-closed 조사 §8 참고).

## 1. 원본 좌표 (2026-09-21 실측)

```text
EFTable_PC  PK 702  Name='DragonKnight'  → tip.name.enum_playerclass_dragon_knight = 가디언나이트
  BaseClass  = 701 DragonHuman (PC_DK)    → 베이스 바디 pc_dk_00_sk
  LookInfo   = EFDLChar_PC_DDK.PC_DDK     → 에셋 코드 ddk
  GenderType = 1 (여성)
  Creatable  = 0, Released = 1            (이 DB 빌드에서 생성 불가 플래그, 데이터는 전부 있음)
  TownDefaultWeapon = 1071002 → EFTable_Item.Model = WP_WDDK_04 (숙련 해결사의 할버드)
  IdentityPartsLookInfo = SK_DDK_WIN_00   (화신화 날개, 2차)
  WeaponAttackSkillId 49000 / MoveSkillId1 49020 런지 / MoveSkillId2 49021 활공(홀딩)
  GetupSkillId 49030·49031 / Identity 49040 화신화·49041 해제
스킬 블록   LearnClass=702 → 49xxx (일반 19, 아이덴티티 8, 각성 49400·49410, 초각성 49420·49430)
Action      XmlData/Action/DRAGONKNIGHT.loa (3.78 MB, 클립 토큰 182)
SkillCam    Standard_SkillCam_DragonKnight
```

**`EFTable_PC.Name`은 DragonKnight이고 한글명은 가디언나이트다.** 코드·데이터 ID는 워로드 선례(Gunlancer→WARLORD)대로 한글명 기준 `GUARDIANKNIGHT`를 쓴다. 원본 파일명(`DRAGONKNIGHT.loa`, `PC_DDK`)은 추출 단계에서만 쓴다.

## 2. 앞 여섯 클래스와의 대조 — 판정 결과

| 판정 항목 | 실측 | 같은 선례 |
|---|---|---|
| 바디가 얼굴·눈·머리를 그리는가 | `pc_dk_00_sk` MATT: `pc_ft_15_hair_mi / pc_dl_av_018a_upper1_mi / pc_dl_av_018a_upper_mi / pc_dl_face_mi_high / pc_dl_eyeao_mi / pc_dl_eye_mi` → **셋 다 그림** | 건슬링어·도화가·슬레이어 (`build_character_part.py` 계보) |
| face/hair 별도 패키지 | `PC_DK_00_FACE`, `PC_DK_00_HAIR` 모두 **없음** | 건슬링어 |
| 얼굴·눈 재질 | 건슬링어와 **동일한** `pc_dl_face_mi_high / pc_dl_eye_mi` → `_fixed_tex_gdh_f`의 `pc_dl_00_face_d.tga`, `pc_dl_00_eye_baked_d.tga` 재사용 | 건슬링어 override 그대로 |
| 리그 | 바디 278본, 방어구 파츠 282본(`b_add_tail_1_01~04` 추가), 클래스 psa 278본 | 슬레이어의 역방향(바디가 부분집합) — §4.2 |
| 무기 소켓 계열 | `b_wp_1 / b_wp_2 / b_wp_3`, `b_weapon_*` 없음. 바디 소켓 `wp_ddk_r_battle → b_wp_1`, `wp_ddk_r_normal → bip001-spine1 (+14.8 Y)`, `wp_ddk_l_battle → b_wp_2` | 건슬링어·도화가 |
| 무기 | `wp_wddk_04_sk` 1메시, 10본이지만 가중치는 `b_head_type1/b_body_01/b_body_end/b_chain_01~05`뿐 (`b_head_type2` 0) → 정적 쿠킹, `XMMatrixIdentity` | 도화가 붓(체인 본 무시) |
| 스탠스 | `GUARDIANKNIGHT_HUMAN`(기본) / `GUARDIANKNIGHT_DRAGON`. Z가 49040 화신화(HUMAN→DRAGON)·49041 해제(DRAGON→HUMAN). DRAGON 로코모션은 `idle_identity1_1 / run_identity1_1`, 기상은 `sk_standup_identity1_1` | 워로드 방어 태세(Z 토글 + `STANCE_LOCOMOTION_SPEC`), 창술사(스탠스별 슬롯 분기) |
| 재질 | `_d/_n/_m/_orm/_cm/_e` PBR. `.mat`의 명시 슬롯은 Diffuse/Normal(/Emissive)뿐, ORM은 `Other[]` | 차원술사 `source.character.realpbr-avatar-v2.v1` (2차) |
| 아이덴티티 게이지 | 49200~49290이 `CostIgauge0 -4/-5/-6`으로 기운을 소모, 49100~49170이 회복 | 1차는 `maximumIdentity 0`로 비활성 |

바디의 피부는 `pc_dl_av_018a_upper/upper1`(아바타 상의)로 들어 있고, 방어구 파츠가 각자
`pc_dk_av_base_upper_mi` 슬롯으로 피부를 다시 그린다. 따라서 `COVERED_BY_ARMOUR`는 바디의
`pc_dl_av_018a_*` submesh 두 개이며, 실제 비트는 **쿠킹된 wmodel의 재질 순서에서 읽는다**.

## 3. 이름 계약

| 항목 | 값 |
|---|---|
| `CHARACTER_CLASS_ID` | `GUARDIANKNIGHT = 7` (`WARLORD = 6` 뒤, `END` 앞) |
| JSON 문자열 | `"GUARDIANKNIGHT"` (`characterClass`, `networkClassId`, receipt `player:GUARDIANKNIGHT`) |
| asset ID | `GuardianKnight` (`animationAssetId`, `assetId`, 폴더명) |
| 아마추어/클립 접두 | `ddk` → `ddk_idle_battle_1` 등 |
| Resources | `Character/GuardianKnight/GuardianKnight.wmodel`, `GuardianKnight_{Upper,Lower,Arm,Shoulder,Helmet}.wmodel`, `textures/`, `Character/WP_WDDK_04/WP_WDDK_04.wmodel` |
| actionId | `guardianknight.skill.49xxx` |
| archetype / animset | `PLAYER_GUARDIANKNIGHT`, `ANIM_GUARDIANKNIGHT` |
| Prototype tag | `Prototype_Component_Model_GuardianKnight[_Upper…]`, `..._Weapon` |
| 리그 family tag | `"DK"` |
| 표시명 (09-21 확정) | Lobby `"Guardian Knight"`, ClassSelect 분류·클래스명 모두 `"가디언나이트"`, 분류 심볼 `nullptr` |
| 날개 | `Character/GuardianKnight/GuardianKnight_Wing.wmodel` ← `SK_DDK_WIN_00/sk_ddk_win_00_sk` (재질 `sk_ddk_win_00_mi`, `sk_ddk_win_00-1_mi`, 278본 바디 팔레트, 가중치 전부 `b_wing_*`). `PC_DDK_00/pc_ddk-wing_sk`는 바디+날개 프리뷰 전신이라 쓰지 않는다 |

## 4. G01 — 추출과 조립·쿠킹 (저장소 밖, `buildScript/`)

### 4.1 umodel 추출

```powershell
$u = "C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe"
$game = "C:\ProgramData\Smilegate\Games\LOSTARK\EFGame"
$stage = "C:\Users\95jus\Downloads\umodel_win32\_export_ddk_psk"
foreach ($p in 'PC_DK_00','PC_DDK_00','SK_DDK_WIN_00','WP_WDDK_04','PC_DL_00') {
  & $u -path="$game" -game=lostark -kr -nameresolve -export -psk -uncook -out="$stage" $p
}
```

- `-notex`를 쓰지 않는다. 2026-09-21 probe에서 `-notex`로 뽑은 psk의 MATT0000이 전부 `material_N`으로 나왔다.
- `PC_DL_00`은 공용 psa(`act_estherskill_1`, `act_jump_lope_1`, 음악) 전용이며 메시는 쓰지 않는다. LookInfo `PC_DDK`가 `PC_DL_00_Ani`, `PC_DK_00_Ani`, `PC_DDK_00_Ani` 세 AnimSet을 참조한다.
- probe 결과(`_export_ddk_probe`)는 참고용이며 정본 스테이징은 새로 뽑는다.

### 4.2 마스터 아마추어 — 이 클래스만 다른 점

바디 `pc_dk_00_sk`가 278본, 모든 방어구 파츠가 282본이고 차이는 `b_add_tail_1_01~04`
(허리띠 꼬리) 네 개다. `build_character_part.py`는 `master`(바디)의 아마추어에 모든 파츠를
재바인딩하므로 그대로 두면 방어구의 add_tail 가중치가 사라진다.

선택지는 둘이다. G01에서 실측 후 결정한다.

1. **master를 282본 파츠(`pc_ddk_00_upper_sk`)로 잡고 바디도 이름 기준 재바인딩** — 바디 278본은 282의 완전 부분집합이어야 한다(`Compare-Skeletons`로 확인). psa는 278본이므로 add_tail 네 본은 bind pose로 남고 워로드처럼 `BONE_CHAIN_SPEC`으로 흔든다.
2. add_tail 가중치를 부모 본으로 이관하고 master=바디 유지 — 꼬리가 굳는다.

1안이 기본이다. `CLASSES['ddk']['master']`가 바디가 아니게 되므로 스크립트의 `IS_MASTER`
판정(`TARGET == MASTER`)과 `body` 타깃 분리를 손봐야 한다. 이 한 줄 변경이 다른 클래스 결과를
바꾸지 않는지 `gdh_f helmet` 재빌드 바이트 동일로 확인한다(08-01 선례).

### 4.3 CLASSES 항목

`build_character_part.py`:

```python
    # Guardian Knight / 가디언나이트, EFTable_PC PK 702 DragonKnight, base class 701 DragonHuman.
    # pc_dk_00_sk draws face, eyes and hair itself (pc_dl_face_mi_high / pc_dl_eye_mi /
    # pc_ft_15_hair_mi -- the GunSlinger set), so no face/hair part. Body 278 bones, every
    # armour part 282: the four extra are b_add_tail_1_01..04, so the master is an armour psk.
    'ddk': {
        'armature': 'ddk',
        'master': 'PC_DDK_00/SkeletalMesh3/pc_ddk_00_upper_sk.psk',
        'body':   'PC_DK_00/SkeletalMesh3/pc_dk_00_sk.psk',
        'psa':    'PC_DDK_00/AnimSet/pc_ddk_00_ani.psa',
        'parts': {
            'upper':    'PC_DDK_00/SkeletalMesh3/pc_ddk_00_upper_sk.psk',
            'lower':    'PC_DDK_00/SkeletalMesh3/pc_ddk_00_lower_sk.psk',
            'arm':      'PC_DDK_00/SkeletalMesh3/pc_ddk_00_arm_sk.psk',
            'shoulder': 'PC_DDK_00/SkeletalMesh3/pc_ddk_00_shoulder_sk.psk',
            'helmet':   'PC_DDK_00/SkeletalMesh3/pc_ddk_00_helmet_sk.psk',
            # Identity wings: shown only in the DRAGON stance. Same 278-bone palette as
            # the body, every weight on b_wing_*, so it rebinds onto the 282 master by name.
            'wing':     'SK_DDK_WIN_00/SkeletalMesh3/sk_ddk_win_00_sk.psk',
        },
    },
```

`upper1 / arm1 / arm2 / helmet1`은 염색 변형(`pc_ddk_00-N_*_mi`)이라 1차 제외. `pc_dk_shadow_sk`,
`pc_ddk-wing_sk` 제외. 날개 본(`b_wing_*`, `b_armwing_*`, `b_wing_memb_*`)은 바디 팔레트에 있고 클래스 psa의
identity 클립이 키를 갖는다(`pc_ddk-wing_physics`는 쓰지 않음).

`cook_character.py`:

```python
    'ddk': {
        'fixedTex': '_fixed_tex_gdh_f',   # same pc_dl face/eye set as GunSlinger
        'parts': {
            'GuardianKnight':          'PC_DK_00/SkeletalMesh3/pc_dk_00_sk.psk',
            'GuardianKnight_Upper':    'PC_DDK_00/SkeletalMesh3/pc_ddk_00_upper_sk.psk',
            'GuardianKnight_Lower':    'PC_DDK_00/SkeletalMesh3/pc_ddk_00_lower_sk.psk',
            'GuardianKnight_Arm':      'PC_DDK_00/SkeletalMesh3/pc_ddk_00_arm_sk.psk',
            'GuardianKnight_Shoulder': 'PC_DDK_00/SkeletalMesh3/pc_ddk_00_shoulder_sk.psk',
            'GuardianKnight_Helmet':   'PC_DDK_00/SkeletalMesh3/pc_ddk_00_helmet_sk.psk',
            'GuardianKnight_Wing':     'SK_DDK_WIN_00/SkeletalMesh3/sk_ddk_win_00_sk.psk',
        },
        'weapons': {
            'WP_WDDK_04': 'WP_WDDK_04/SkeletalMesh3/wp_wddk_04_sk.psk',
        },
        'overrides': {
            ('pc_dl_face_mi_high', 'Diffuse'): 'pc_dl_00_face_d',
            ('pc_dl_eye_mi', 'Diffuse'): 'pc_dl_00_eye_baked_d',
        },
    },
```

- 방어구 `.mat`에 Specular가 없다(PBR). 기본 쿠킹은 diffuse+normal만 붙는다. `_orm`은 컨버터 `--orm-remap`으로 실을 수 있지만 런타임 소비자는 2차 SourceCharacter program이므로 1차에서는 넣지 않는다.
- 텍스처 알파는 `Inspect-TgaAlpha.ps1`로 다시 잰다. 얼굴·눈은 건슬링어 보정본이 그대로 맞고, 방어구 `_d`의 discard%가 0.3 이상이면 `Strip-TgaAlpha.ps1`.

### 4.4 명령 순서

```powershell
$blender = "C:\Program Files\Blender Foundation\Blender 5.0\blender.exe"
$py = "C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe"
$fbx = "C:\Users\95jus\Downloads\umodel_win32\_fbx_ddk"
foreach ($t in 'body','upper','lower','arm','shoulder','helmet','wing') {
  $anim = if ($t -eq 'body') { '--anim' } else { '' }
  & $blender --background --factory-startup --python buildScript\build_character_part.py -- ddk $stage $t "$fbx\$t.fbx" $anim
}
& $blender --background --factory-startup --python buildScript\build_weapon.py -- "$stage\WP_WDDK_04\SkeletalMesh3\wp_wddk_04_sk.psk" "$fbx\wp_wddk_04.fbx"
& $py buildScript\cook_character.py ddk $stage $fbx "Client\Bin\Resources\Character\GuardianKnight" Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe
```

검증(워로드와 동일 기준): psa 임포트 164 시퀀스 경고 0, 클립 이름 충돌 0(probe에서 36자 이상
이름 없음 확인), 쿠킹 exit 0, `validate_wmodel` OK, `Compare-InverseBind` 6파츠 `bindDiff=0`,
`Compare-Skeletons`로 바디⊂282 확인, `info`로 `animations=164 skeleton=yes`.

### 4.5 애니셋 (Esther / Customizing / TerrainJump / MazeHammer)

클래스 psa에는 없고 공용에 있다.

| 애니셋 | 원본 psa | 클립 |
|---|---|---|
| `GuardianKnight_EstherAnimSet` | `PC_DL_00/pc_dl_00_ani.psa` | `act_estherskill_1` |
| `GuardianKnight_CustomizingAnimSet` | `PC_DK_00/pc_dk_00_ani.psa` (233본) | `idle_charactercustomizing_1` |
| `GuardianKnight_TerrainJumpAnimSet` | `PC_DL_00` | `act_jump_lope_1` — `build_terrain_jump_player_animations.py`·`build_card_maze_player_animations.py`의 map에 `"GuardianKnight": "PC_DL_00"` 추가 |
| 음악 루프 | `PC_DK_00`의 `sc_music_1` (다른 클래스는 `act_music_loop_1`) | `append_psa_clip_to_wmodel.py --name ddk_act_music_loop_1` |

`build_npc_animset.py --clips`로 만들고 skeletonHash가 바디 wmodel과 같은지 `compare_attach.py`로 대조한다.
psa 본 수(233/278)와 바디(282 master)가 달라도 이름 매핑이면 되지만, 실패하면 1차에서는 애니셋을
비워 둔다(`animationSetModels: []`도 supported로 실행된다 — 건슬링어 선례).

## 5. G02 — 애니메이션 참조·저작 데이터

```powershell
$tables = "C:\Users\95jus\Downloads\SourceData\SourceData\LPK\data2\EFGame_Extra\ClientData\TableData"
$loa = "C:\Users\95jus\Downloads\SourceData\SourceData\LPK\data3\EFGame_Extra\ClientData\XmlData\Action\DRAGONKNIGHT.loa"
& $py buildScript\extract_action_loa.py $loa Client\Bin\Resources\Character\GuardianKnight\GuardianKnight.wmodel GuardianKnight ddk_ Data\Animation\Reference\GuardianKnight $tables
& $py buildScript\extract_player_skilltiming_v2.py fill GuardianKnight     # 또는 extract_skilltiming.py $tables GuardianKnight 49000 49999
& $py buildScript\extract_projectiles.py ...                              # 49140·49210·49220·49230이 MaxRange를 가짐 → 투사체/장판 후보
```

산출: `Data/Animation/Reference/GuardianKnight/GuardianKnight.{clipmap,clipseq,animnotify,skilltiming,projectiles}`,
`Data/Animation/Authored/GuardianKnight/GuardianKnight.animevents`.

주의:
- `.clipseq`의 `mode=`는 추론값이다. 홀딩(49021 활공)·콤보 판정은 인게임 동작이 우선.
- 기준 체인은 스킬별 최저 `seq=`. `custom_`은 트라이포드 표식이 아니다.
- `Idle_Normal_1 / Att_Normal_2_* / Act_Jump_Lope_1 / Act_Fall_Loop`는 공용 패키지 소속이라 모델에 없다고 나오는 게 정상.

`skillbindings.json`(formatVersion 3)은 `PlayerSkills.json`의 이 클래스 스킬과 1:1이어야 하며
**로스터·damage·바인딩·receipt를 한 변경으로** 넣는다. LMB 49000은 `att_battle_1_01~03` 3단 COMBO.

## 6. G03 — 밸런스

### 6.1 PlayerProfiles.json (14 필드 정확히)

```json
    {
      "characterClass": "GUARDIANKNIGHT",
      "maximumHp": 50000,
      "maximumResource": 1000,
      "resourceRegenPerSecond": 500,
      "attackPower": 1000,
      "defense": 130,
      "moveSpeed": 2.95,
      "defenseStanceMoveSpeedScale": 1.0,
      "maximumIdentity": 0,
      "identityRegenPerSecond": 0,
      "identityDrainPerSecond": 0,
      "identityStanceSwitchCost": 0,
      "identityCyclic": 0,
      "defaultStance": "GUARDIANKNIGHT_HUMAN"
    }
```

`moveSpeed`는 `EFTable_PC.MoveSpeed 295`/100. `attackPower 1000`은
`test_valtan_fast_combat_tuning_contract.py:67`이 전 클래스에 강제한다. 나머지는 워로드 값 복사이며
`PROJECT_TUNED`로 receipt에 기록한다. 아이덴티티 게이지는 1차에서 0(화신화 토글 무조건 허용)이고,
원본의 "게이지 만충 시 Z" 조건은 2차에서 `maximumIdentity`와 `identityStanceSwitchCost`로 붙인다.

### 6.2 로스터 (2026-09-21 사용자 확정, 고정 바인딩)

DB 값은 `EFTable_Skill` SK=10 기준. 배율은 `EFTable_SkillEffect` `skillId×10+변형` `ValueA`이며
G03에서 각 행을 확인한다(49100 → 491000 s10 = 1049 확인, 49300 계열은 행 없음 → 툴팁 매크로 참조).

| 키 | 일반 (`GUARDIANKNIGHT_HUMAN`) | 화신화 (`GUARDIANKNIGHT_DRAGON`) | 쿨 | 비고 |
|---|---|---|---|---|
| LMB | 49000 기본 공격 | 같음 | — | COMBO 3단 `att_battle_1_01~03`. 화신화 콤보 `att_identity1_1_01~03`은 2차 |
| Q | 49100 클리브 | 같음 | 6s | `requiredStance: NONE` |
| W | 49110 와일드 어퍼 | 같음 | 8s | |
| E | 49120 쓰러스트 | 같음 | 8s | 49121은 트라이포드 변형 |
| R | 49130 길로틴 스핀 | 같음 | 24s | |
| A | 49200 리벤지 블로우 (HUMAN) | 49210 리벤지 스피어 (DRAGON, 3m) | 15s / 21s | 같은 슬롯 두 스킬 → 양쪽 다 non-NONE 스탠스 |
| S | 49220 스피닝 플레임 (HUMAN, 4.5m) | 49230 아바돈 플레임 (DRAGON, 4m) | 10s / 18s | |
| D | 49150 퀘이크 스매시 | 같음 | 40s | |
| F | 49260 블레이즈 스윕 (HUMAN) | 49270 블레이즈 플래시 (DRAGON) | 24s / 24s | |
| T | — | 49330 딥 임팩트 (DRAGON 전용) | — | `requiredStance: GUARDIANKNIGHT_DRAGON`, HUMAN에서는 슬롯 비어 있음 |
| V | 49400 가디언 백래시 | 같음 | — | 각성(`LearnAwakening=1`). 드래곤 소환 메시(`SK_DDK_DRG_00`) 표현은 2차, 판정만 1차 |
| ALT+V | 49420 브레스 오브 엠버레스 | 같음 | — | 초각성(`UltimateSkill=1`) |
| SPACE | 49020 런지 (HUMAN, 8m) | 49021 활공 (DRAGON, 12m) | — | `MoveSkillId1/2`. 활공의 원본 "홀딩 시 자유 활공"은 HOLD 모델 대상이며 1차는 일반 ACTIVE 12m로 넣고 사용자 관찰 후 조정 |
| Z | 49040 화신화 (HUMAN→DRAGON) | 49041 화신화 해제 (DRAGON→HUMAN) | — | 워로드 17800/17810과 같은 `requiredStance`/`setsStance` 쌍. 클립 `sk_transformdragon` / `sk_transformhuman` |

- 화신화 상태에서 안 바뀌는 슬롯은 `requiredStance: NONE` 하나로 둔다. 슬롯 충돌 규칙은 같은 슬롯에 두 스킬이 있을 때만 양쪽 non-NONE을 요구한다(워로드 08-07 §2.3).
- 기운 소모 스킬(49200~49290 `CostIgauge0 -4~-6`)은 1차에서 `identityCost 0`.
- 49210/49220/49230은 `MaxRange`가 있어 `PlayerSkillTargeting.json` 지점 지정 후보다. 1차는 캐스터 히트로 넣고 `.loa` projectile 추출 결과를 보고 결정.

### 6.3 나머지

- `DamageProfiles.json`: `damage.player.49xxx` 행 (배율은 §6.2 조회값).
- receipt: 새 항목을 **은퇴 슬롯 위치가 아니라 클래스 블록 단위로** 추가하되 LF 유지, `movementDistance` `0.0`.
- `HitShapes/GuardianKnight.hitshapes.json`: damaging 스킬이 하나라도 있으면 **필수**. `fill_animevents_hit_shapes.py GuardianKnight` → `build_hitshapes.py GuardianKnight`.
- `RootMotion`·`CancelWindows`·`projectiles.json`·`interactionbindings.json`은 선택(런지 49020은 루트모션 없으면 제자리 → 2차).

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
```

## 7. G04 — 코드 등록 (fail-closed 순서 준수)

**순서가 중요하다.** JSON에 `"GUARDIANKNIGHT"`를 먼저 넣고 C++ 파서를 빠뜨리면 `ActorCatalog`·
`EquipmentPresentationCatalog`가 **전 클래스 catalog를 통째로 거부**한다. enum 추가 →
allowlist 제외 상태로 빌드 → 파서 전부 추가 → 데이터 → allowlist 편입 순서로 간다.

### 7.1 Shared

| 파일 | 변경 |
|---|---|
| `Shared/Public/Network/PacketType.h` L126 | `GUARDIANKNIGHT = 7,` |
| 같은 파일 L153-163 `Is_Supported_Playable_Character_Class` | 마지막 단계에서 편입 |
| `PacketMessages.h` `PLAYER_STANCE_ID` L1433 | `GUARDIANKNIGHT_HUMAN`, `GUARDIANKNIGHT_DRAGON` (`WARLORD_DEFENSE` 뒤) |
| `Client/Public/CharacterSpec.h` `EQUIPMENT_PART_SPEC` L86-100 | `LostArk::Shared::PLAYER_STANCE_ID eRequiredStance = NONE;` 추가(무기 `WEAPON_PART_SPEC::eRequiredStance`와 같은 의미). `EQUIPMENT_SLOT_KIND`에 `IDENTITY` 추가 — 아바타 방어구가 있어도 가려지지 않는 파츠 |

protocol version은 snapshot 구조가 안 바뀌므로 유지. 스탠스 enum 값 추가는 wire 값이 아니라
기존 필드의 새 값이므로 Server/Client를 함께 빌드하면 된다.

### 7.2 Server

| 파일 | 변경 |
|---|---|
| `Server/Private/GameplayCatalog.cpp` L181, L199-202 | `"GUARDIANKNIGHT"` 클래스 파서 + `"GUARDIANKNIGHT_HUMAN"/"GUARDIANKNIGHT_DRAGON"` 스탠스 파서 |
| `Server/Private/ServerGameplayContractTests_PlayerCombos.cpp` L187/L607 | 스탠스 게이트 테스트에 A 슬롯(49200 HUMAN / 49210 DRAGON) 사례 추가 |
| `Server/Private/GameRoom_Inventory.cpp` L180 | `case GUARDIANKNIGHT: return "GuardianKnight";` |
| `Server/Private/ValtanClearRewards.cpp` L75 | 파서 (보상 JSON은 1차 미추가) |
| `Server/Private/ServerGameplayContractTests_PlayerSkillFixtures.h` L156-191 | 퀵슬롯·LMB 3단 fixture 행 |
| `Server/Private/ServerGameplayContractTests_CharacterAdmission.cpp` L559-578 | roster assert 추가 |

### 7.3 Client — 필수

| 파일 | 변경 |
|---|---|
| `Client/Public/Logic_GuardianKnight.h`, `Client/Private/Logic_GuardianKnight.cpp` (신규, UTF-8 BOM 없음) | `Spec_GuardianKnight`: Weapons `{Part_90_Weapon_R, ..._Weapon, "b_wp_1"}`, Equipment 6(Arm/Helmet/Lower/Shoulder/Upper + `{Part_10_Equip_Wing, ..._Wing, 0u, false, EQUIPMENT_SLOT_KIND::IDENTITY, EQUIPMENT_PRESENTATION_SLOT::END, GUARDIANKNIGHT_DRAGON}`), `COVERED_BY_ARMOUR`=쿠킹 후 실측, `BAKED_HAIR`=hair submesh 실측, 클립 `ddk_idle_battle_1 / ddk_run_battle_1 / ddk_dmg_idle_1 / ddk_dead_1 / ddk_knockdown / ddk_knockdown_land / ddk_down / ddk_standup_1 / ddk_act_estherskill_1 / idle_charactercustomizing_1 / ddk_abn_fear_1 / ddk_act_music_loop_1`, BoneChains(`b_skirt_*`, `b_capatcloth_*`, `b_cape_01`, `b_hair0N_*`, `b_add_tail_1_01` — 링크 수는 wmodel 본 테이블에서), `STANCE_LOCOMOTION_SPEC { GUARDIANKNIGHT_DRAGON, "ddk_idle_identity1_1", "ddk_run_identity1_1" }`, family `"DK"` |
| `Client/Private/Character.cpp` `Apply_DefaultEquipmentVisibility` L2698-2737 | 장비 루프에 `NONE != equipment.eRequiredStance && equipment.eRequiredStance != m_eStance → isVisible=false` 추가. `IDENTITY` kind는 `hasAvatarArmor`로 가리지 않음 |
| `Client/Private/Character.cpp` `Apply_NetworkStance` L2475-2516 | 무기 루프 뒤에 장비 루프를 같은 규칙으로 추가(preview 활성 여부와 무관하게 `eRequiredStance`가 NONE이 아닌 장비만 갱신). 포즈 교체는 기존 코드가 처리. 기상 클립은 DRAGON에서 `ddk_sk_standup_identity1_1`이 필요하면 `STANCE_LOCOMOTION_SPEC` 확장(1차는 공용 `ddk_standup_1` 유지) |
| `Client/Private/ActorCatalog.cpp` L222 등 | `equipmentModels`에 Wing 포함 6개. `EQUIPMENT_SLOT_KIND` switch가 있으면 `IDENTITY` case 추가(빌드로 확인) |
| 스탠스 문자열 파서 | `CombatHUDViewModel.cpp:63-66`, `PlayerSkillCatalog.cpp:145-148`, `EquipmentPresentationCatalog.cpp:126-127`, `EquipmentAuthoringTool.cpp:66-69`, `Effect_Tool_Helpers.cpp:880-881`, `BalanceTool.cpp:9546-9547`, `PlayableCharacterPreviewContract.cpp:24-25`(기본 preview 스탠스 HUMAN) |
| `Data/UI/HUD/HudBuffSources.json` | 선택: `"stance": "GUARDIANKNIGHT_DRAGON"` 버프 행 (아이콘 없으면 생략) |
| `Client.vcxproj` / `.filters` | 위 두 파일 + `02.GameObjects\00. Character\06. GuardianKnight` 필터 + `Data/Animation/**/GuardianKnight*` None 항목 |
| `Client/Private/CharacterCatalog.cpp` L9, L97 | include + `case` |
| `Client/Private/PlayableCharacterAssetService.cpp` L122-152 | `CHARACTER_PROTOTYPE_TAGS GUARDIANKNIGHT` (equipment 6 = Upper/Lower/Arm/Shoulder/Helmet/Wing, weapon 1 — catalog 배열 길이와 **정확히 일치**) |
| `Client/Private/Loader.cpp` L134, L1448 | 폴더명, prototype 목록 |
| `Client/Public/AnimationPreviewAssets.h` L109 | preview row (`164 clips`) |
| `Client/Private/ArenaCameraProfile.cpp` L21-22, `Client/Public/ArenaCameraProfile.h` L36, `MainApp.cpp` L10053 | **배열 7→8**, L117 count 6→7. 안 늘리면 `Character.cpp:1709`에서 index 7 OOB |
| `Client/Public/Level_CharacterSelect.h` L326-335 | `SUPPORTED_CLASSES` 6→7 |
| `Client/Private/Level_CharacterSelect.cpp` L100, L2300-2308 | 표시명 `"Guardian Knight"`, `CLASS_LIST_ENTRY{950,280,48, 6, "GuardianKnight", "가디언나이트", "가디언나이트", nullptr}` (UTF-8 바이트 이스케이프로 기존 행과 같게) |
| `Client/Private/Level_Lobby.cpp` L37 | `return "Guardian Knight";` |
| `Data/UI/ClassSelect/ClassSelect_Layout.json` | `classes` 배열 + `ClassList_Row6`, `ClassList_Symbol6` 슬롯 (없으면 행이 안 보임) |
| 문자열 파서 전부 | `ActorCatalog.cpp:201`, `AnimationSkillBindingDocument.cpp:80/93`, `CombatHUDViewModel.cpp:49`, `PlayerSkillCatalog.cpp:131`, `EquipmentPresentationCatalog.cpp:110`, `EquipmentAuthoringTool.cpp:39`, `MvpAwardCatalog.cpp:740/771`, `BalanceTool.cpp:9546` |
| 표시명 switch | `Level_Lobby.cpp:37`, `MainApp.cpp:357/382`, `SkillWindowView.cpp:28/42`, `PartyWindowView.cpp:31`, `AvatarBookWindowView.cpp:86`, `CharacterInfoWindowView.cpp:103/239`, `SongCastGaugeView.cpp:48`, `CharacterActionWorkbench.cpp:43`, `Effect_Tool_Helpers.cpp:2040/2057/2095/2171/3314`, `Effect_DirectAuthoredSourceIndex.cpp:104`, `Effect_Tool_DocumentIo.cpp:1724`, `EffectAuthoringResourceTree.h:54` |
| `Client/Private/PlayerController.cpp` | SlotKeys 변경 없음 (Q W E R A S D F T V ALT_V SPACE 기존) |

### 7.4 Data — 필수

| 파일 | 내용 |
|---|---|
| `Data/Actors/CharacterCatalog.json` | `PLAYER_GUARDIANKNIGHT` row: bodyModel, equipmentModels 6(Wing 포함), weaponModels 1, animationSetModels(성공한 것만), `runtimeStatus: supported` |
| `Data/Animation/Authored/GuardianKnight/GuardianKnight.skillbindings.json` | Character init이 파일 부재를 실패로 처리 |
| `Data/Balance/PlayerProfiles.json`, `PlayerSkills.json`, `DamageProfiles.json`, receipt | §6 |
| `Data/Animation/HitShapes/GuardianKnight.hitshapes.json` | damaging 스킬이 있으면 필수 |

### 7.5 Tools / 테스트 — 같이 움직여야 하는 literal

| 파일 | 변경 |
|---|---|
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` L1001, L1004 | `'GUARDIANKNIGHT'` — 넣는 순간 profile row 필수. `$knownStances`에 `'GUARDIANKNIGHT_HUMAN','GUARDIANKNIGHT_DRAGON'` |
| `Tools/GameplayPipeline/test_valtan_fast_combat_tuning_contract.py` L14-20, L56-59 | `EXPECTED_CLASSES` + LMB 개수 7→8 |
| `Tools/CharacterAnimationIntake/test_player_hitshape_coverage_contract.py` L8-15, L53-54, L74 | 클래스 튜플 + 합계 76 갱신 |
| `Tools/CharacterEquipmentPipeline/test_equipment_catalogs.py` L16-23, L145 | **1차 미변경**. 이 테스트는 클래스를 넣으면 visualSet·READY 무기·classPreset을 요구하므로 장비 catalog는 2차에서 함께 |
| `Tools/NetworkProtocolHarness/.../NetworkProtocolHarness.cpp` L557-578 | roster assert 추가 |
| `Tools/VehiclePipeline/build_vehicle_skills.py`, `build_sound_catalog.py`, `build_quickslot_hud_ui.py` | 2차 |

## 8. 1차에서 의도적으로 비우는 것 (건슬링어·슬레이어 선례로 안전 확인)

탈것 rider row, `CharacterSoundCatalog`, HUD identity 블록, `SkillSlotMarks`, `CharacterInfoDisplay`,
`PartyWindow` 심볼, `MvpClassSymbols`, `Customizing*` 8종(빈 블록을 넣으면 문서 전체가 실패하므로
**아예 생략**), `EquipmentPresentationCatalog`/`LoadoutPresets`, `ItemCatalog`, `Valtan.clearrewards`,
ClassSelect 아트(`IllustrationSmall.png`, `IdentitySymbol.png`는 없으면 placeholder), `Data/Camera`.

## 9. 2차 후보 (별도 PLAN)

1. **화신화 조건·나머지**: 아이덴티티 게이지(`maximumIdentity`, 49100~49170 회복 / 49200~49290 소모, 만충 시에만 Z), 화신화 LMB 콤보 `att_identity1_1_*`, 나머지 identity 스킬 49300~49370, 활공 HOLD, 날개 재질(`sk_ddk_win_00_e` emissive) program.
2. **원본 재질**: `build_vehicle_source_material.py extract`로 `pc_ddk_00_*_mi` 셰이더 해시를 찍어 차원술사 `realpbr-avatar-v2` program 재사용 여부 판정. 새 program이면 `native_shader_dispatch.py`·`Shader.cpp` ranges·`Model.cpp` 상한·테스트 센티넬을 함께 올린다.
3. 루트모션(런지·미티어 크래시·리벤지 스피어), 이펙트(`Effect/GuardianKnight`), 사운드, 탈것 `PC_DL_00_Vehicle_Ani`, 장비 catalog, 커스터마이징, UI 아트.

## 10. 검증

```text
G01  umodel 4패키지 exit 0 / psa 164 경고 0 / 쿠킹 7개 exit 0 / validate_wmodel OK / bindDiff=0
G02  Reference 5파일 생성, 바인딩 클립 전부 모델에 존재, COMBO 3단 계약 일치
G03  Publish-GameplayBalance Validate PASS (7 profiles)
G04  Engine / Shared / Server / Client Debug Build 성공
     NetworkProtocolHarness failures 0 / Server.exe --contract-test failures 0
     python test_valtan_fast_combat_tuning_contract.py / test_player_hitshape_coverage_contract.py PASS
     git diff --check
사용자 화면 확인 (에이전트 대행 금지): Lobby → Character Select 렌더 → Bern/Valtan 진입 → 퀵슬롯 재생·데미지
```

빌드는 사용자가 지시할 때 실행한다. `NoDefaultCurrentDirectoryInExePath`와 Blender python PATH
선행은 08-07 문서와 같다.

## 11. 사용자 결정

1. 퀵슬롯 로스터 — **2026-09-21 확정** (§6.2 고정 바인딩, 화신화 상태에서 A/S/F/SPACE 분기, T는 화신화 전용).
2. **09-21 확정**: Character Select 분류·클래스명 `가디언나이트`, Lobby `Guardian Knight`, 날개 파츠 표시 1차 포함(`SK_DDK_WIN_00`, DRAGON 스탠스 전용 장비 파츠).
