# 2026-09-03 쿠크세이튼 모델·애니메이션 인벤토리 분석 (Composition 착수 전 실측)

브랜치 `GB/valtan-bugfix-koukusaydon-pattern`. 분석 문서이며 파일을 바꾸지 않았다.
실측 도구: `Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe info`, wmodel skeleton/material section 문자열 스캔,
`Data/Animation/Reference/KakulSaydon/*.actionreference.json` 파싱,
`C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829` 추출 receipt/umodel 로그.
09-03 `KAKUL_SAYDON_PATTERN_INTAKE_AND_COMPOSITION_SAVE_AUDIT.md` §1의 action 매핑은 여기서 반복하지 않고 참조한다.

---

## 1. Resources 실측: 폴더 9개, wmodel 11개

`Client/Bin/Resources/Character/KoukuSaton/` 아래에는 Valtan과 달리 `anims/`나 `AnimSets/`가 없다.
모든 clip은 각 본체 wmodel 안에 section type=4로 들어 있다.

| 폴더 / 파일 | 크기 | clip | skeleton | material | 정체 (근거) |
|---|---|---|---|---|---|
| `MN_RPCT_00/MN_RPCT_00.wmodel` | 190 MB | 249 (`rpct00_`) | 165 bone | `mn_rpct_00`, `mn_rpct_00_parts`, `mn_rpct_01` | 세이튼 기본 본체. 05와 skeleton·clip 249개가 완전히 동일 |
| `MN_RPCT_05/MN_RPCT_05.wmodel` | 197 MB | 249 (`rpct00_`) | 165 bone, 00과 동일 | `mn_rpct_05`, `05-1`, `05-2`, `05-3`(emissive), **`mn_rpcz_00`(쿠크 텍스처)** | 쿠크 파츠가 붙은 세이튼 본체. 추출 receipt: 05 cook은 MN_RPCT_00의 PSA 두 개(main 176 + evt2 73)를 그대로 썼다 |
| `MN_RPCT_06/MN_RPCT_06.wmodel` | 17 MB | 34 (`mn_rpct_06_sk.ao_`) | 106 bone | `mn_rpct_05`, `05-1`, `05-2` 재사용 | 2관문 대형 세이튼. 전용 skeleton, 05 텍스처 공유 |
| `MN_RPCZ_00/MN_RPCZ_00.wmodel` | 28 MB | 91 (`rpcz00_`) | 122 bone, `b_wp_1/2/3` 3개 무기 소켓 | `mn_rpcz_00` 1개 | 1관문 광대 쿠크 |
| `WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel` | 1.2 MB | 1 (`wp_mn_rpct_05_sk.ao_att_battle_17_01`) | 있음(`b_rpct_05`) | `wp_mn_rpct_05` | 세이튼 무기. `att_battle_17` = 아드레날린 무기 A/B·사라진 후 내려찍기·화염링 소환 |
| `WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel` | 1.0 MB | 16 (`wprpct06_att_battle_01/03/6/8_*`, `dmg_critical_*`) | 있음(`b_rpct_00~03`) | `wp_mn_rpct_06`(emissive) | 대형 세이튼 무기. 본체 06의 clip family 1/3/6/8과 이름이 1:1이라 **무기도 같은 이름 clip을 같이 재생해야** 한다 |
| `WP_MN_RPCT_07/wp_mn_rpct_07l_sk`, `07r_sk` | 1.7 MB ×2 | 0 | bone001 하나 | l=`wp_mn_rpct_08_mi`, r=`wp_mn_rpct_07_mi` | MN_RPCT_05 패키지에서 같이 export된 좌우 한 쌍 정적 메시. 3관문 칼날 계열 추정(`att_battle_29` 칼날 처형식, `att_battle_24` 무기뇌격) |
| `WP_MN_RPCT_08/wp_mn_rpct_08_sk`, `08_1_sk` | 0.2 MB ×2 | 0 | b_root 하나 | 본체 `mn_rpct_05-2`(emissive) | 05 본체 텍스처를 쓰는 소형 파츠 두 변형. 역할 미확인 |
| `WP_WGDH_02S/WP_WGDH_02S.wmodel` | 45 KB | 0 | 없음 | `wp_wgdh_02` (.tga) | RPCT 계열 명명이 아니고 추출 Packages receipt에도 없다. 출처·역할 미확인 |

### 1.1 확인된 사실

- MN_RPCT_00과 MN_RPCT_05는 **같은 skeleton, 같은 249 clip**이다. 차이는 mesh/material뿐이다. 05에는 쿠크 텍스처
  `mn_rpcz_00_mi`와 emissive 파츠가 있으므로 "쿠크가 붙은 세이튼"이고, 00은 그것이 없는 기본 세이튼이다.
  현재 reference profile `MN_RPCT_05`(2관문 무대 세이튼 115 action)와 `MN_RPCT_07`(3관문 합체 102 action)은 **둘 다 05 모델**을 가리키고,
  Animation Tool은 00을 "clip donor" preview로만 연다(`Animation_Tool.cpp:3229`).
  2관문 세이튼을 00 본체로 보여 줄지 05로 보여 줄지는 09-02 RESULT §9 blocker 1번 그대로 사용자 결정이다.
- MN_RPCT_05 원본 패키지에는 본체 외에 정적 메시 `mn_rpct_05_01~04`, `wp_mn_rpct_07l/r`, `wp_mn_rpct_bullet`, 다른 몬스터 메시
  `mn_rhcn_00`, `mn_rhkp_06`이 함께 들어 있었다(umodel 로그). 이 중 Resources로 옮겨진 것은 07l/07r뿐이다.
- 쿠크(MN_RPCZ_00) skeleton에는 무기 소켓이 세 개(`b_wp_1`, `b_wp_2`, `b_wp_3`) 있지만 **`WP_MN_RPCZ_*` 무기 모델이 Resources에도
  추출 Packages에도 없다.** 바주카포·뿅망치·나팔이 body mesh에 포함돼 있는지, 별도 추출이 필요한지 확인해야 한다.
- 모든 본체는 `b_effectworldzero`, `b_effectroot`, `b_effectname` bone을 가진다. Valtan V2 binding의 `b_effectroot` anchor 관례를 그대로 쓸 수 있다.

---

## 2. 애니메이션 데이터 상태

| 문서 | 상태 |
|---|---|
| `Data/Animation/Reference/KakulSaydon/*.actionreference.json` 4개 | REFERENCE_ONLY. profile 4 / action 349 / stage 4,072 / slot 3,692 |
| `Data/Animation/Authored/KakulSaydon/*.actionbindings.json` 4개 | **bindings 0행** |
| `Data/Animation/Authored/KakulSaydon/*.patternbindings.json` 4개 | **patterns 0개**, `nextPatternOrdinal 1` |
| `Data/Compositions/Bosses/KakulSaydon.bosscomposition.json` (codex 세션 신규) | `status REFERENCE_ONLY`, `bossArchetypeId null`, `patterns []` |
| `Data/Compositions/Sequences/KakulSaydonArena.sequencer.json` (codex 세션 신규) | `status SHADOW`, track 2개(circus finale world sequence + shot.1) |

reference가 참조하는 runtimeClip과 wmodel 실물의 대조 결과다.

| profile | 본체 | 참조 clip | wmodel에 존재 | 누락 | wmodel에 있지만 미참조 |
|---|---|---|---|---|---|
| MN_RPCT_05 | 05 | 112 | 112 | 0 | 137 |
| MN_RPCT_06 | 06 | 32 | 32 | 0 | 2 (`evt2_rpct_move_01`, `evt2_rpct_respawn_01`) |
| MN_RPCT_07 | 05 | 122 | 122 | 0 | 127 |
| MN_RPCZ_00 | RPCZ_00 | 85 | 85 | 0 | 6 (`bound_hit_land`, `evt2_rpcz_*`) |

즉 **기믹 공격 clip은 전부 있다.** 없는 것은 원본에는 있으나 export되지 않은 공통 상태 clip이다(holdout 37~45개):
`Abn_Bug/EarthQuake/Fear/Stun`, `Bound`, `KnockDown`, `Respawn_1_x`, `Dead_3`, `StandUp_1`, `Idle/Run/Walk_Status1~3`, `Fast_Run_Battle_1`.
이 중 patterns에 실제로 필요한 것은 `Respawn`/`StandUp` 정도이고 나머지는 상태 이상 표현이다.

05/07 profile이 05 wmodel의 249 clip 중 절반만 참조하는 이유는 `evt2_*` 73개가 컷신 연기 clip이고 여러 `att_battle_*`가 action 정의에 안 묶였기 때문이다.

---

## 3. 기믹 ↔ 모델 ↔ clip family

`actionreference`의 `displayName`과 각 action이 참조하는 `att_battle_N` family를 묶은 것이다.
전체 349 action의 원문은 audit §1.3, family별 상세는 이 분석의 실행 로그에 있다.

### 3.1 1관문 광대 쿠크 (MN_RPCZ_00, 무기 소켓 3개)

| 축 | clip family | action |
|---|---|---|
| 바주카포 | `att_battle_1_01~11` | 4219701~05, 4219740~42, 4219745, 4219777~79 |
| 뿅망치 | `att_battle_2_01~13` | 4219706~10, 4219746~48 |
| 나팔 | `att_battle_3_01~10` | 4219711~15, 4219750~52 (피자 = `3_01, 3_07, 3_09` ×2) |
| 저글링 | `att_battle_4` | 4219716 |
| 마법진 안전지대 | `att_battle_5` + idle | 4219717/28/29 (반반색 57 stage) |
| 광기 게이지 / 돌진 | `att_battle_6` | 4219718/53/54, 4219776(24 stage) |
| 융단폭격 | `att_battle_7`, `att_battle_8` | 4219719/20/55 |
| 저주받은 인형 | `att_battle_9` | 4219721/75/30 |
| 대폭발 | `att_battle_10_01~04` | 4219769~74 (6조합, 18 stage) / 성공 `dmg_critical_*` / 실패 `10` |
| 슈퍼 바주카 | `att_battle_11_start/loop/end` | 4219725 |
| 훌라후프 | `att_battle_12_*`, `13_*` (start/loop/end) | 4219726/27 |
| 응집된 에너지 | `att_battle_14` | 42197100~102 |
| 그로기 / 카운터 | `dmg_critical_start/loop/end` | 4219763, 4219766~68 |

### 3.2 2관문 세이튼 무대 (profile MN_RPCT_05 → 모델 05 또는 00)

| 축 | clip family |
|---|---|
| 불뿜기 쇼 `14`, 종이비둘기 `15`, 카드낙하 `16`, 아드레날린 무기 `17`(WP_MN_RPCT_05), 서커스공 `18/23/26`, 레이저 쇼 `19_01`(30 stage), 진짜를 찾아봐 `20`, 카드무늬/주사위 `21`(75 stage), 슬라이딩 태클 `22`, 트럼프 카드 `25_05`, 화염 파동 `10`, 화염분출 `9`, 광기의 불길 `30`, 고속이동 `34`, 눈 레이저 `35`, 바람 방구 `36`, 리프어택 `37`, 순간이동 `att_phase1_1` |
| 함께 춤을 4/3/2초 `25_03/04/05/06` + `25` (각 **249 stage**), 서커스 룰렛 `12/13`, 무력화 `6` + `dmg_critical_*` |

### 3.3 2관문 대형 세이튼 (MN_RPCT_06 + WP_MN_RPCT_06)

| 축 | clip family |
|---|---|
| 내려찍기 `1_01~05`, 바람불기 `2`, 전방 3연타 `3`, 조커카드 `4`, 불뿜기 `5`, 잡기/던지기 `6_01~04`, 번뜩이는 공포 `7`, 기 모아서 내려찍기 `8_01~03`, 종이비둘기 `9`, 광선 `10_01~03`, 무력화 `dmg_critical_*` |
| 무기 clip `wprpct06_att_battle_01/03/6/8_*`가 본체와 같은 번호를 쓴다 |

### 3.4 3관문 합체 쿠크세이튼 (profile MN_RPCT_07 → 모델 05)

| 축 | clip family |
|---|---|
| 무기뇌격 `24`, 서커스공 `23/26`, 함께 춤을 `25_03~06`, 제물 의식 `27`, 군단장 시그니쳐 `28` + `28_05_loop` + `28_10`, 칼날 처형식 `29`(WP_MN_RPCT_07 l/r 추정), 광기의 불길 `30`, 광기의 표식 `31`, 환영공포 `32`, 폭탄 투하 `33`, 고속이동/가짜이동 `34` + `att_phase1_1`, 눈 레이저 `35`, 바람 방구 `36`, 리프어택 `37`, 원기옥 `12` + `dmg_critical_*` |
| 2관문 공용 재사용: 불뿜기 `14`, 종이비둘기 `15`, 아드레날린 `17`, 랜덤박스 `8`, 무력화 `6`, 화염 파동 `10`, 화염분출 `9`, 메두사 `20` |

---

## 4. 지금 도구가 할 수 있는 것과 없는 것

| 경로 | 현재 |
|---|---|
| Animation Tool Resource Files에서 `MN_RPCT_05/06/RPCZ_00` 선택 | 가능. `Open_KakulProfile`이 profile을 열고 action/stage/slot을 나열, slot 단위 override(actionbindings), `Build_KakulPatternFromAction`으로 patternbindings pattern 생성, preview 재생 (`Animation_Tool.cpp:3224~`, `11499~`) |
| `MN_RPCT_00` 선택 | clip donor preview만. profile 없음 |
| Composition Workbench에서 쿠크 열기 | **불가.** `Resolve_ValtanCompositionNativeModel`(`Animation_Tool.cpp:1855`)이 asset 이름 `Valtan`을 요구하고 Save는 Valtan canonical만 커밋한다 |
| codex 세션의 `CBossCompositionDocument` / `SequencerTool` | SHADOW facade. gameplay·presentation을 실행하지 않고 join만 한다 |
| Server | Kakul brain, BossCatalog archetype, BossProfiles, Data/Kakul split source, Encounters Product, world boss placement 전부 0 (audit §1.1) |

`LV_LUT_MIDNIGHTC_ED`의 현재 Gameplay.world.json은 playerSpawn 9(파티 4 + SL01~05 stage marker), triggerBox 6(jump 3, paper 2, 1Stage_Final circus finale)이고 boss placement가 없다.
StageMarkers는 `SOURCE_LEVEL_ID_ONLY`라 SL01~05가 어느 관문인지 확정돼 있지 않다(SL01 (66,0,-102), SL03 (2,3.5,330), SL04 (-2,1.3,730), SL05 (-2,1.3,942), SL02는 (-1198,-11,-958) 원거리).

---

## 5. Composition에 올리기 전에 정해야 하는 것

1. **본체 identity**: 2관문 무대 세이튼을 00으로 보일지 05로 보일지. 3관문 합체는 05 확정. 대형 세이튼은 06, 쿠크는 RPCZ_00.
2. **무기 계약**: WP_MN_RPCT_06은 본체와 같은 이름 clip을 동시 재생하는 skinned weapon이라 Valtan `weaponModel` 계약(정적 부착)으로는 안 된다. 부착 소켓(`b_wp_1/2`)과 clip 동기 규칙이 BossCatalog 확장으로 필요하다.
3. **쿠크 무기 3종 리소스**: `b_wp_1/2/3` 소켓은 있는데 모델이 없다. 팀장 Drive 또는 추가 추출로 확인.
4. **미확인 파츠**: WP_MN_RPCT_08 두 변형, WP_WGDH_02S. 용도를 사용자가 화면에서 확인해야 한다.
5. **첫 수직 슬라이스 패턴**: audit §1.4의 6종 문서(BossCatalog, BossProfiles, Kakul.gameplay/presentation/combatobjects, Encounters)를 하나의 패턴으로 관통시킬 대상. 후보는 clip이 단순하고 무기 없이 되는 것: 쿠크 `4219714 피자`(6 stage, clip 3종) 또는 대형 세이튼 `4221801 내려찍기`(6 stage, `1_0x`).

이 문서는 여기까지가 범위다. Kakul split source 스키마와 generic boss runner 계획은 별도 PLAN에서 다룬다.
