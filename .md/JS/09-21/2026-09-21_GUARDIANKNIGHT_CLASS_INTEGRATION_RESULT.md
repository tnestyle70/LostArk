# 가디언 나이트 클래스 추가 — 1차 구현 결과

작성자: JS · 2026-09-21 · 브랜치 `feature/player-guardian-knight` (base `feature/player-dodge-cancel-window` HEAD 5234b80e)
계획서: `2026-09-21_GUARDIANKNIGHT_CLASS_INTEGRATION_PLAN.md`

일곱 번째 playable class를 추출부터 Server 판정까지 연결했다. 워로드(08-07) 파이프라인을 그대로
썼고, 실측으로 드러난 차이 네 가지가 이 문서의 핵심이다. **인게임 화면 확인은 아직 없다.**

## 1. 클래스 좌표

```text
EFTable_PC  PK 702  Name='DragonKnight'  →  tip.name.enum_playerclass_dragon_knight = 가디언나이트
  BaseClass = 701 DragonHuman (PC_DK) → 베이스 바디 pc_dk_00_sk / 에셋 코드 ddk / 여성
  TownDefaultWeapon = 1071002 → EFTable_Item.Model = WP_WDDK_04 (할버드)
  IdentityPartsLookInfo = SK_DDK_WIN_00 (화신화 날개)
스킬 블록   LearnClass=702 → 49xxx      Action  XmlData/Action/DRAGONKNIGHT.loa
```

`PCPreview`/`GameMsg`에만 GuardianKnight가 있고 PC 테이블은 DragonKnight다. 코드·데이터 ID는
한글명 기준 `GUARDIANKNIGHT = 7` / `GuardianKnight` / 접두 `ddk_`.

## 2. 앞 여섯 클래스와 다른 점 넷

### 2.1 바디는 건슬링어 계열, 마스터 아마추어는 방어구

`pc_dk_00_sk` MATT: `pc_ft_15_hair_mi / pc_dl_av_018a_upper1_mi / pc_dl_av_018a_upper_mi /
pc_dl_face_mi_high / pc_dl_eyeao_mi / pc_dl_eye_mi`. 얼굴·눈·머리를 바디가 그리고 재질이
건슬링어와 같아 `_fixed_tex_gdh_f` 보정본을 그대로 썼다. 피부는 바디에 없고(아바타 상의
`pc_dl_av_018a_*`) 방어구가 `pc_dk_av_base_*` 슬롯으로 피부를 다시 그린다.

바디 278본 < 방어구 282본(`b_add_tail_1_01~04`). 다른 클래스와 반대라 `build_character_part.py`에
`'body'` 키를 새로 열어 **master를 `pc_ddk_00_upper_sk.psk`(282본)로 잡고 바디를 파츠처럼
재바인딩**했다. 결과 팔레트는 285본(282 + 노드), 6파츠 전부 `bindDiff=0 maxDelta=0`.

### 2.2 소켓은 `b_wp_1`, 할버드 헤드는 하나

리그는 `b_wp_1/2/3` 계열(`b_weapon_*` 없음). 바디 소켓 dump: `wp_ddk_r_battle → b_wp_1`,
`wp_ddk_r_normal → bip001-spine1 (+14.8)`, `wp_ddk_l_battle → b_wp_2`. 무기 psk는 10본이지만
가중치는 `b_head_type1 / b_body_* / b_chain_*`뿐이고 `b_head_type2`는 0 → 정적 쿠킹, identity 부착.

### 2.3 날개는 `SK_DDK_WIN_00`, 스탠스 전용 장비 파츠

`PC_DDK_00/pc_ddk-wing_sk`는 바디+날개 프리뷰 전신(8재질, 가중치 절반이 바디 본)이라 제외.
`sk_ddk_win_00_sk`가 정답(2재질, 가중치 전부 `b_wing_*`, 278본 바디 팔레트).
스탠스별 표시는 무기에만 있어서 `EQUIPMENT_PART_SPEC::eRequiredStance`와
`EQUIPMENT_SLOT_KIND::IDENTITY`(아바타 방어구에 안 가려짐)를 새로 열고
`Apply_DefaultEquipmentVisibility`·`Apply_NetworkStance`의 장비 루프에서 판정한다.

### 2.4 공용 클립은 Delain 가족 psa

LookInfo `PC_DDK`가 `PC_DL_00_Ani / PC_DK_00_Ani / PC_DDK_00_Ani`를 참조한다. 클래스 psa(164)에
로코모션·기상·abn은 있지만 에스더·점프·음악·customizing은 `PC_DL_00/pc_dl_00_ani.psa`(1,067, 233본)에
있다. 애니셋 3종과 음악 루프는 Blender 없이 `append_psa_clip_to_wmodel` codec 경로(지형 점프
builder와 같은 carrier+append+subset)로 만들었고 skeleton hash가 바디와 byte-동일하다.
카드미로 망치 클립(`pr_it_gstfp_*`)은 Delain psa에 없어 MazeHammer 애니셋은 1차 제외.

## 3. 로스터 (사용자 확정, 고정 바인딩 · 종류는 09-21 인게임 관찰로 2차 조정)

| 키 | 일반 (HUMAN) | 화신화 (DRAGON) | 종류 | 배율 |
|---|---|---|---|---|
| LMB | 49000 기본 공격 (HUMAN) | 49001 기본 공격 (화신, DRAGON) | COMBO 3단 `att_battle_1_01~03` · COMBO 3단 `att_identity1_1_01~03`(원본 49000 계열 490003~490006 참조, 셰이프는 49000 캐스터 셰이프 복사) | 100 / 100 |
| Q | 49100 클리브 | 같음 | ACTIVE | 1049 |
| W | 49110 와일드 어퍼 | 같음 | **COMBO 3단** `wildupper_01~03` | 376 |
| E | 49120 쓰러스트 | 같음 | ACTIVE 3클립 | 1328 |
| R | 49130 길로틴 스핀 | 같음 | ACTIVE 3클립 | 666 |
| A | 49200 리벤지 블로우 | 49210 리벤지 스피어 | **HOLD** `revenge_01/04/02` · **HOLD** `brutalmarch_01/02/03`(04 제외) | 3098 / 191 |
| S | 49220 스피닝 플레임 (4.5m) | 49230 아바돈 플레임 | ACTIVE · **ACTIVE+지점 지정 4.0m** | 538 / 344 |
| D | 49150 퀘이크 스매시 | 같음 | **HOLD** `grandfinale_01` / `grandfinale_02` **holdLastPose** / `grandfinale_03` | 6198 |
| F | 49260 블레이즈 스윕 | 49270 블레이즈 플래시 | ACTIVE · **COMBO 3단 `playRate 1.5`** | 453 / 1084 |
| T | — | 49330 딥 임팩트 | **ACTIVE+지점 지정 7.0m** (DRAGON 전용) | 397 |
| V | 49400 가디언 백래시 | 같음 | ACTIVE | 397 |
| ALT+V | 49420 브레스 오브 엠버레스 (8.0m) | 같음 | ACTIVE | 397 |
| SPACE | 49020 런지 `sk_lunge`만 | 49021 활공 | ACTIVE · **무피해 HOLD** [`flying_01` 1867ms, 분기 550ms] → [`flying_02`×3 = 최대 3초 루프] → [`flying_03`]. 유지 단계 클립은 제자리라 루트모션 곡선에 3.0 m/s 전진을 손으로 넣음 | — |
| Z | 49040 화신화 → DRAGON | 49041 해제 → HUMAN | ACTIVE 스탠스 토글 | — |

- 쿨다운은 테스트값 3000(LMB 0), `resourceCost 0`, `identityCost 0`, `maximumIdentity 0`.
- **분기형 HOLD(Server 계약 확장)**: 시작 단계의 `comboAdvanceMs < actionDurationMs`이면 분기 시각까지 누르고 있을 때 그 시점에 유지 단계로 넘어가고, 그 전에 놓으면 시작 클립(자체 착지 포함)을 끝까지 재생한 뒤 유지·종료 단계 없이 끝난다. `comboAdvanceMs == actionDurationMs`인 기존 차지형(워로드 풀배럴)은 그대로. `PlayerSkillSystem.cpp` `holdBranchStart / holdBranchesToLoop / holdEndsAfterStart`. 활공 분기 550ms는 `flying_01`의 원본 InputTiming(0.55s부터 전 구간 캔슬)에서 잡았다.
- **`holdLastPose` 바인딩 키(신설)**: HOLD 유지 단계 마지막 클립은 기본 루프인데 `grandfinale_02`는 첫↔끝 프레임이 최대 114° 다른 "들어 올리고 멈춤" 동작이라 루프하면 1.7초마다 다시 들어 올리고 놓는 순간 100°+ 포즈에서 03으로 블렌드돼 "뚝" 끊겼다. `{ "clip": ..., "holdLastPose": true }`면 한 번 재생 후 마지막 포즈 유지(non-loop 클립은 끝 프레임에서 멈춤). `revenge_04`·`brutalmarch_02`·`flying_02`는 이음새 1° 미만이라 루프 유지. 실측 도구: 스크래치 `clip_pose_diff.py`(wmodel ANIM 키의 뼈 회전각 비교).
- **분기형 HOLD 게이지**: 시작 단계가 잘리면 기존 게이지가 조기 해제로 오판해 숨겨서, 분기형은 유지 단계 진행률(활공 3초)만 표시하도록 `Update_ChargeGauge`/`RenderChargeGaugeText`에 분기 추가.
- HOLD는 [시작, 유지, 종료] 3단이고 판정은 종료 단계에만(300ms 또는 클립 HIT). 무피해 HOLD는 종료 단계 hitTimeMs 300 + 상위 hitTimeMs 0으로 publisher를 통과하고 Server는 damage profile이 비어 있어 아무것도 안 때린다(활공). COMBO 비마지막 단계는
  600ms 미만이면 자동 진행, 아니면 open 200 / close dur-300.
- 지점 지정은 `Data/Balance/PlayerSkillTargeting.json` `GROUND_POINT`, 미리보기 텍스처는 차원술사
  `fx_e_ring_028`/`fx_k_fsm_magiccircle_02` 재사용(RUNTIME_RESOURCE).
- 배율은 `EFTable_SkillEffect skillId×10+변형` SK=10 `ValueA` 최저 변형.
- `mode=`가 COMBO로 추론된 스킬은 인게임 관찰이 우선했다(E/R은 ACTIVE, W/화신 F는 COMBO).

## 4. 걸린 것

**skilltiming v1이 timed 히트 없는 스킬을 버린다.** 49000/49100/49120/49220/49420에
skilltiming 행이 없어 `fill_animevents_hit_shapes.py`가 셰이프를 못 붙였고 hitshapes에서 빠졌다.
`extract_player_skilltiming_v2.py fill GuardianKnight <ids>`로 5행을 채운 뒤 재생성해 15/15.

**hitshapes v4는 `migrate_player_hit_results.py`로 올린다.** builder는 v3만 쓰고 계약 테스트는
v4(collider/logic/result ID)를 강제한다.

**Bash에서 `"$F\\$n.fbx"`가 `$n.fbx` 한 파일로 덮어썼다.** 파츠 6개를 forward slash로 다시 굽는 데
4분 낭비. 경로는 forward slash로 넘긴다.

**`cook_character.py`는 상대 경로 컨버터를 못 찾는다.** `subprocess`가 cwd 무관하게
`Tools\...exe`를 해석 못 해 `WinError 2`. 절대 경로로 넘긴다.

**계약 테스트 literal 3곳**: `test_player_hitshape_coverage_contract.py` 클래스 튜플·합계 76→91,
`test_valtan_fast_combat_tuning_contract.py` `EXPECTED_CLASSES`·LMB 7→8·비평타 87→105.

**Publish-GameplayBalance Validate가 25분 걸린다.** receipt 5만 줄을 PowerShell JSON으로 읽는다.
백그라운드로 돌리고 다른 작업을 이어간다.

## 5. 산출물

```text
Client/Bin/Resources/Character/GuardianKnight/   178 MB
  GuardianKnight.wmodel            106 MB  submesh 6 / 285본 / 165클립 (164 + ddk_act_music_loop_1)
  GuardianKnight_{Upper,Lower,Arm,Shoulder,Helmet,Wing}.wmodel
  AnimSets/GuardianKnight_{Esther,Customizing,TerrainJump}AnimSet.wmodel
  textures/ 25
Client/Bin/Resources/Character/WP_WDDK_04/       1 MB
Data/Animation/Reference/GuardianKnight/   clipmap 127 / clipseq 75체인·41스킬 / animnotify 2,955행 / skilltiming 52
Data/Animation/Authored/GuardianKnight/    animevents 2,203 / skillbindings 19
Data/Animation/HitShapes/GuardianKnight.hitshapes.json   v4, 15스킬 28히트
Data/Balance  PlayerSkills +19 / DamageProfiles +15 / PlayerProfiles +1 / receipt +458행 (3491→3949)
Data/Actors/CharacterCatalog.json  PLAYER_GUARDIANKNIGHT (equipment 6, weapon 1, animset 3)
Data/UI/ClassSelect/ClassSelect_Layout.json  classes + ClassList_Row6 / ClassList_Symbol6
```

바디 submesh 순서 `0 hair / 1 av_upper1 / 2 av_upper / 3 face / 4 eyeao / 5 eye` →
`COVERED_BY_ARMOUR = 1|2`, `BAKED_HAIR = 0`. 방어구 `lower/lower1/upper1/upper3` 디퓨즈에
12~38% 알파가 있어 원본대로 뒀다(구멍이 보이면 `Strip-TgaAlpha`).

**Drive 전달 필요**: 위 Resources 폴더 두 개(Git 비추적).

## 6. 코드

- Shared: `CHARACTER_CLASS_ID::GUARDIANKNIGHT = 7`, `PLAYER_STANCE_ID::GUARDIANKNIGHT_HUMAN/DRAGON`,
  `Is_Supported_Playable_Character_Class` 편입. protocol version 유지.
- Client 신규: `Logic_GuardianKnight.h/.cpp`(Weapons `b_wp_1`, Equipment 6, StanceLocomotion
  `ddk_idle_identity1_1/ddk_run_identity1_1`, BoneChains 20). `CharacterSpec.h`
  `EQUIPMENT_PART_SPEC::eRequiredStance`, `EQUIPMENT_SLOT_KIND::IDENTITY`; `Character.cpp` 장비
  스탠스 판정 2곳.
- 배열 확장: `ArenaCameraProfile` 7→8(안 늘리면 `Character.cpp` index 7 OOB), `MainApp.cpp`
  classNames, `Level_CharacterSelect.h` SUPPORTED_CLASSES 6→7, `Effect_DirectAuthoredSourceIndex`
  6→7, `EquipmentAuthoringTool` 6→7, `EffectAuthoringResourceTree.h` 9→10.
- 파서·표시명 switch 30여 파일(에이전트 위임, 바이트 단위 exact replace, 인코딩 보존 확인).
- Server: `GameplayCatalog` 클래스·스탠스 파서, `GameRoom_Inventory`, `ValtanClearRewards`, `PlayerSkillSystem` 분기형 HOLD.
- Tools: `Publish-GameplayBalance.ps1` 클래스·스탠스 allowlist, `Publish-ValtanClearRewards.ps1`,
  `NetworkProtocolHarness` roster assert(Seven), `build_terrain_jump/card_maze` SOURCES
  `"GuardianKnight": "PC_DL_00"`.
- 프로젝트: `Client.vcxproj/.filters` 새 필터 `06. GuardianKnight`, Logic 2파일, Data 9파일.
- buildScript(저장소 밖): `build_character_part.py` `ddk` 항목 + `'body'` 키, `cook_character.py` `ddk` 항목.

## 7. 검증

```text
umodel 추출         5패키지 exit 0 (2.2GB)
psa 임포트          164 시퀀스, 경고 0, 클립 이름 충돌 0
쿠킹                8개 exit 0, validate_wmodel 전부 OK
팔레트 공유         6파츠 bindDiff=0 maxDelta=0 (nameDiff=1은 메시 노드명, 정상)
애니셋              3종 + 음악 루프, skeleton section byte-동일 assert 통과
Engine / Shared / Server / Client Debug   PASS (OBJ 179 → 증분 16)
Gameplay balance Validate   PASS: 7 profiles / 276 skills / 124 damage profiles, hit-shape 91/91
test_player_hitshape_coverage_contract   OK (5)
test_valtan_fast_combat_tuning_contract  OK (3)
NetworkProtocolHarness   Accept Seven Playable Character Classes PASS
                         failures 12 = Protocol 84/80 고정 fixture(기존, 이번 변경과 무관)
Server.exe --contract-test   (본문 §8)
git diff --check    통과
```

**인게임 확인(사용자, 09-21 로컬 Server+Client)**: Character Select 렌더·입장·Q~F/V/ALT+V·Z 화신화·활공 홀딩(짧게/길게)·D 홀드 후 내려치기 이어짐까지 정상 관찰. 스킬 종류는 관찰대로 조정(§3).

**인게임 확인 전 상태였던 서술은 아래로 대체됨.** 확인 경로: Lobby → Character Select에서 `가디언나이트` 행 →
렌더 → Bern/Valtan 입장 → Q~F·V·ALT+V → Z 화신화(날개 표시, identity idle/run) → 화신화 A/S/F/T/SPACE.

## 8. 남은 것

- `Server.exe --contract-test`: 1차 실행 `failures : 23`이었으나 개별 항목이 출력 필터에 안 잡혔고 재실행은 시간 문제로 중단. 실패가 이번 변경과 관련인지 미판정.
- Server 부트스트랩 배포는 `-Mode Publish`로 완료했고 로컬 Server/Client로 사용자 확인 진행 중. 첫 시도는 allowlist 헤더가 첫 빌드 도중 편집돼 `GameRoom_PlayerCommands.obj`가 stale이었고(`Server rejected an unsupported class`), 헤더 mtime 갱신 후 재빌드로 해결.
- 루트 모션 `Data/Animation/RootMotion/GuardianKnight.rootmotion.json`(19스킬, 스테이지별)과 캔슬 창 `CancelWindows/GuardianKnight.cancelwindows.json`(75창) 추가. 첫 관찰 "부자연스럽다"의 원인은 루트 모션 부재였다.
- W 1단(333ms 준비 동작)에 HIT notify가 없어 300ms 판정이 들어가 있다. 어색하면 0으로.
- 2차: 아이덴티티 게이지·만충 시 Z, 화신화 LMB 콤보(`att_identity1_1_*`), 활공 HOLD, 루트모션
  (런지·활공·리벤지 스피어), 원본 PBR 재질 program(`realpbr-avatar-v2` 해시 대조), 이펙트·사운드,
  ClassSelect 아트, 커스터마이징, 탈것(`PC_DL_00_Vehicle_Ani`), 장비 catalog, MazeHammer 애니셋.
- `.skilltiming` 49000 행 이름은 v2 fill이 비워 `기본 공격`으로 손수 채웠다.

## 추가: 스킬 사운드·카메라 쉐이크 (2026-09-21 밤)

- 원인 1: `CharacterSoundCatalog.json`에 `GuardianKnight` 클래스가 없고 wav도 배포되지 않았다. 기존
  `build_sound_catalog.py`는 D: raw wav 덤프 전용이라 새 클래스를 못 다룬다.
- 원인 2: `ddk_sk_deepimpact_03`의 SHAKE 행 `fov=5,-1`(음수 주파수)을 `CCameraShakeService::Parse_PayloadSpec`이
  거부해 GK cue 문서 전체(EFFECT/SOUND/SHAKE)가 `Character Effect cue load isolated`로 로드 실패하고 있었다.
- 수정: `Parse_Oscillator`가 음수 주파수를 진폭 부호로 접어 받는다(`sin(-f t) = -sin(f t)`).
  `Tools/SoundPipeline/build_character_sound_catalog.py` 신규 — `.animevents` SOUND 행을 Wwise 패키지
  (`SOUND_PC_DRAGONKNIGHT*`, Common은 `SOUND_PC_COMMON*` + 클래스 뱅크)에서 resolve → FMOD PCM wav →
  `Client/Bin/Resources/Sound/Character/GuardianKnight/` 1056개(259MB, Drive 전달 대상) + 카탈로그 갱신.
- 커버리지: 219 이벤트 중 216 매칭. 미매칭 `PC_DragonKnight_F_Stop1`(Stop 전용), `DragonicResonance1_Vox4_1/2`,
  `PC_Common_Dual1_DragonKnight_F_Vox1_1`(Play 대상이 배포 뱅크에 없음). Common에 새 이벤트 2개 추가, 다른 클래스 항목 변경 없음.
- 검증: 카탈로그 JSON parse·클래스별 diff 확인. Client 컴파일·실제 청취/쉐이크 확인은 사용자 몫(Client 재시작 필요, 카탈로그는 부팅 시 로드).
