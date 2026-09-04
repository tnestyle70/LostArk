# 2026-09-03 쿠크세이튼 패턴 인테이크 · Composition Save 병목 · Valtan Server 세션 종료 원인

브랜치 `GB/valtan-bugfix-koukusaydon-pattern`. 이 문서는 조사·감사 문서이며 구현 계획서가 아니다.
같은 폴더의 `2026-09-03_VALTAN_GRIP_AND_PATTERN_REGRESSION_IMPLEMENTATION_PLAN.md`는 다른 세션이
소유하는 왼손 부착·패턴 회귀 범위이고, 이 문서와 겹치지 않는다.

읽은 정본: `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/GB/계획서하네스규칙.local.md`,
`.md/TEAM/README.md`, `.md/GB/09-02/2026-09-02_VALTAN_PATTERN_COLLIDER_AND_COMPOSITION_BOTTLENECK_PLAN.md`.

세 범위 모두 **현재 저장소 파일과 실제 실행 로그 실측**이다. 기억이나 이전 문서의 결론을 옮기지 않았다.

---

## 0. 세 줄 요약

```text
1. 쿠크세이튼은 4 profile · 349 action의 animation reference만 있고 Server 전투 계약은 0이다.
   원작 3관문 기믹은 이미 전부 action으로 추출돼 있어 매핑표만 있으면 바로 저작에 들어간다.
2. Composition Save를 막는 것은 Save_Reload의 16개 게이트가 아니라 그 앞단이다.
   41 pattern 중 11개가 canonical이라 collider Add/Remove와 stageKind가 잠겨 있고,
   193 stage 중 42개는 loop slot이라 Animation lane 자체가 read-only다.
3. "Valtan replication observed a disconnected Server session"의 원인은 네트워크가 아니다.
   Six Pizza의 장식용 돌기둥 4개 링(r=9.9m)이 초기 navigation 밖이라 room 전체가 죽고,
   Server가 정상 FIN을 보낸 것이다. 현재 anchor에서 실패 확률은 yaw 무관 100%다.
```

---

## 1. 쿠크세이튼 — 지금 있는 것과 없는 것

### 1.1 실측 인벤토리

| 축 | 현재 상태 | 정본 위치 |
|---|---|---|
| Level / World / Room | **있음** | `LEVEL::KAKULSAYDON_ARENA`, `WORLD_ID::KAKULSAYDON_ARENA`, `Client/Private/Level_KakulSaydonArena.cpp`(1,138줄), `GameRoom.cpp:3088 Handle_DebugEnterKakulSaydonArena` |
| Map / Area | **있음** | `LV_LUT_MIDNIGHTC_ED` (Imported/Authoring/runtime), navigation 3종 |
| Gameplay world | **부분** | `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` formatVersion 6 — placement 15개(playerSpawn 9, triggerBox 6). **boss placement 0개** |
| Stage marker | **있음(의미 미확정)** | `StageMarkers.json` `semanticStatus=SOURCE_LEVEL_ID_ONLY`. SL01~ 복구 스테이지 이름만 있고 마리오/게이트 의미는 추론하지 않음 |
| Animation reference | **있음** | `Data/Animation/Reference/KakulSaydon/*.actionreference.json` — 4 profile / 349 action / 4,072 stage / 3,692 slot |
| Animation 저작 | **있음(REFERENCE_ONLY)** | `Data/Animation/Authored/KakulSaydon/*.patternbindings.json` — `authority=REFERENCE_ONLY`. Server Product가 아니다 |
| Boss archetype | **없음** | `Data/Actors/BossCatalog.json` formatVersion 6에는 `BOSS_VALTAN`, `BOSS_VALTAN_GHOST` 둘뿐 |
| Encounter / gameplay / presentation source | **없음** | `Data/Encounters/`에 `Bern`, `Valtan`만. `Data/Valtan/` 대응 폴더 없음 |
| combat object / damage profile / boss profile | **없음** | `Valtan.combatobjects.json` 대응물 없음, `BossProfiles.json`에 Kakul 항목 없음 |
| Server brain | **없음** | `ValtanBrain.cpp`에 대응하는 Kakul brain 없음 |

즉 **"보스가 아레나에 서고 패턴을 재생하는 데 필요한 것 중 애니메이션만 준비돼 있다."**

### 1.2 4 profile의 정체

| profile | model | action | 원작 대응 |
|---|---|---|---|
| `MN_RPCZ_00` | `Character/KoukuSaton/MN_RPCZ_00` | 85 | **1관문 광대 쿠크** — 무기 3종 전환 보스 |
| `MN_RPCT_05` | `Character/KoukuSaton/MN_RPCT_05` | 115 | **2관문 세이튼(소형/무대 연출)** — 서커스 미니게임 진행자 |
| `MN_RPCT_06` | `Character/KoukuSaton/MN_RPCT_06` | 47 | **2관문 대형 세이튼** — 실제 전투 본체 |
| `MN_RPCT_07` | `MN_RPCT_05` 모델 alias | 102 | **3관문 합체 쿠크세이튼** |

`MN_RPCT_07`은 별도 physical body 없이 `MN_RPCT_05` 모델을 alias한다(08-31 RESULT의 의도적 경계).

`HOLDOUT`은 9개 공통 액션 중 `대기 / 이동 / 낙하 / 피격 / 사망 / 생성` 계열에 걸려 있고,
Pattern 생성·저장이 거부된다. 나머지는 전부 `REVIEW_CANDIDATE`다.

### 1.3 원작 기믹 -> 실제 action 매핑

아래는 추측이 아니라 `actionreference.json`의 `displayName`을 그대로 옮긴 것이다.
`sourceActionId`가 exact join 키다.

#### 1관문 — 광대 쿠크 (`MN_RPCZ_00`, 85 action)

원작의 축은 **무기 3종 로테이션 + 광기 게이지 + 대폭발 방향 맞추기**다. 세 축이 전부 데이터에 있다.

| 기믹 | action (sourceActionId) |
|---|---|
| 바주카포 무기 | `4219701 백덤블링` `4219702 조준3연발` `4219703 부채꼴` `4219704 투사체` `4219705 공격하며후퇴` `4219740/41/42 액션_1~3` `4219745 이동액션_신규` `4219725 슈퍼 바주카` `4219777/78/79 _이지` |
| 뿅망치 무기 | `4219706 선이동A` `4219707 후방전방찍기` `4219708 휠윈드` `4219709 리프어택` `4219710 2연타` `4219747/48 액션1~2` `4219746 이동액션_신규` |
| 나팔 무기 | `4219711 선이동A` `4219712 전방직선` `4219713 좌우공격` **`4219714 피자`** `4219715 3연속도넛` `4219752 3연속도넛_안팍` `4219750/51 액션_1~2` |
| 광기 게이지 | `4219718 / 4219753 / 4219754 광기게이지 관리 스킬 1~3` |
| 광기 카운터 | `4219766/67/68 대형 쿠크_광기게이지_카운터_1~3` |
| **대폭발(시계/반시계 3연속)** | `4219722 시작(미사용)` `4219723 성공` `4219724 실패` + 6조합 `4219769~4219774` (`반시계_시계_반시계`, `시계_반시계_시계`, `시계_반시계_반시계`, `반시계_시계_시계`, `시계_시계_반시계`, `반시계_반시계_시계`) |
| 조커카드 찾기 | `4219756~4219762 소환1~7` |
| 저주받은 인형 | `4219721`, `4219775 _노말`, `4219730 대형 쿠크_종료` |
| 마법진 안전지대 | `4219717 붉은색` `4219728 푸른색` `4219729 반반색`(57 stage) |
| 훌라후프 | `4219726 A` `4219727 B` |
| 융단폭격 | `4219719 A` `4219720 B` `4219755 B_2` |
| 기타 | `4219716 저글링` `4219763 그로기` `4219776 돌진` `42197100~102 응집된 에너지 발산 / 성공 / 중앙이동` `4219780 대형 쿠크_사망스킬` `4219765 대형 쿠크_순간이동` |

`990378~990381 쿠크_선회_0/90/180/270도`는 `테스트용 임시` 표기다. 제품 회전에 쓰지 않는다.

#### 2관문 — 세이튼 무대 (`MN_RPCT_05`, 115 action)

2관문의 축은 **미니게임 6종**이다.

| 기믹 | action |
|---|---|
| 서커스 룰렛 | `4219811 시작` `4219812 대기`(24) `4219813 춤A` `4219814 춤B`(24) + 리허설 `4219892/93/94` |
| 진짜를 찾아봐 | `4219808 시작` / 하트셋트 `4219832 진짜` `4219833 가짜` / 총잡이셋트 `4219842 진짜` `4219843 가짜` + 리허설 `4219888~4219891` |
| 주사위(카드 무늬) | `4219810 검은 스페이드`(76) `4219834 붉은 스페이드` `4219835 검은 클로버`(75) `4219836 붉은 클로버` `4219837 검은 하트` `4219838 붉은 하트`(75) `4219839 검은 다이아` `4219840 붉은 다이아`(75) |
| 레이저 쇼 | `4219807 A` `4219829 B` `4219830 C` `4219831 D` (각 30 stage) |
| 카드 짝 맞추기 | `4219899 Start` `42198101 ing` `42198100 공통무늬 찾기 준비` |
| **쿠크세이튼과 함께 춤을** | `4219879 준비` `4219884 대기` `4219880 4초` `4219881 3초` `4219882 2초` `4219883 2초_B` (각 **249 stage**) + 리허설 `4219895~4219898` |
| 폭탄 해체 | `4219885 폭탄해체 놀이` |
| 무력화(카운터) | `4219816 시작`(38) `4219817 성공` `4219818 실패` |
| 소환 요청 | `4219844/45 갈고리 A/B` `4219846 괴상한 인형` `4219847~50 화염링 A~D` `4219851 거대 망치` `4219815 랜덤박스` `4219828 트럼프 카드 발급` |
| 레벨 트리거 | `4219852~4219856`, `4219862` (`01~06_레벨트리거 요청`) |
| 서커스공 | `4219806 날리기` `4219864 스매쉬` `4219865 스매쉬_스피드` `4219866 탑승`(39) |
| 일반 공격 | `4219801 불뿜기 쇼` `4219802 마술쇼_종이비둘기` `4219803 카드낙하` `4219804/05 아드레날린 무기A/B` `4219819 회전하며 카드 날리기` `4219820 화염 파동` `4219821/22/77 내려치기A/B/C` `4219825 화염분출` `4219863 슬라이딩 태클` `4219875 태클_2연속` `4219867 리프어택` `4219868 사라진후 내려찍기` `4219870 순간이동 공격` `4219871/72 광기의 불길A/B` `4219873/78 초강력 바람 방구 전/후방` `4219874 인형 대상자 집중 공격` `4219876 눈에서 레이져빔` `42198102 메두사 공격` |
| 이동 | `4219823/24 순간이동 사라지기/나타나기` `4219857 센터로 이동` `4219826/27 선회 좌/우` `4219858~61 고속이동 전후좌우` `42198103~106 고속이동 _가짜이동` |

#### 2관문 — 대형 세이튼 (`MN_RPCT_06`, 47 action)

| 기믹 | action |
|---|---|
| 조커카드를 찾아라 | `4221807 시작` `4221816 성공!` |
| 카드뒤집기 | 근/중/원거리 `4221823/20/24`, 타겟팅 `4221821`, 타겟없음 `4221825/26/27`, 헬모드 `4221832~4221837`, 제거 `4221839` |
| 기 모아서 내려찍기 | `4221814 본체` `4221818 카드 찾기` `4221819 대폭발`(8) `4221822 카드 삭제` |
| 잡기 / 던지기 | `4221810 잡기`(8) `4221812 던지기` |
| 무력화 | `4221811` |
| 일반 공격 | `4221801/02/03 내려찍기` `4221808/09 불뿜기 A/B` `4221829/30 불뿜기 _이지` `4221804 바람불기` `4221831 바람불기_이지` `4221805/06 전방3연타_A` `4221813 번뜩이는 공포` `4221815 거대 종이비둘기 날리기` `4221838 광기깃든 광선 발사` `4221817 대상 추적 회전` |

#### 3관문 — 합체 쿠크세이튼 (`MN_RPCT_07`, 102 action)

| 기믹 | action |
|---|---|
| 광기의 불길 | `4219914 A` `4219915 B` `4219916 C`(20) |
| 광기의 표식 | `4219917 기본` + 강화 4방향 `4219968~4219971` |
| 압도적인 환영공포 | `4219918 3초` `4219933 5초` `4219934 7초` `4219935 9초` `4219936 11초` |
| 칼날 처형식 | `4219913 시작` `4219976~4219979 공격 1~4단계` `4219990~4219993 _싱글모드` `4219963 칼날 소환`(15) |
| 군단장 시그니쳐 | `4219912 스킬` `4219937 소환수 스킬` `4219939 스킬_공격`(34) `4219985 _리허설` |
| 제물 의식 | `4219911 A` `4219932 B` |
| 무기뇌격 | `4219903 본체`(16) `4219904 분신공격` `4219962 평타 공격` |
| 본체/가짜 분신 | `4219964/65 본체 좌·우 이동 후 화염 파동` `4219966/67 가짜 좌·우` `4219972~75 고속이동_가짜이동` |
| 갈고리 | `4219980/81/82 소환 1~3단계` |
| 빙고 | `4219905 빙고맵 오프닝` `4219984 빙고 폭탄 소환` |
| 함께 춤을 | `4219906 힘자랑` `4219907 찌르기` `4219908 만세` `4219909 브레이크댄스` |
| 기타 | `4219901 슬라이딩 태클 후 내려찍기` `4219902 서커스공 스매쉬_A` `4219910 서커스공 탑승`(39) `4219919 서커스 폭탄 투하A` `4219924 눈에서 레이져빔` `4219925 초강력 바람 방구` `4219926 리프어택` `4219927 원기옥 발사`(16) `4219931 조롱하며 사라지기` `4219938 센터로 순간이동` `4219940~4219961` (불뿜기/종이비둘기/아드레날린/랜덤박스/무력화 3종/화염 파동/내려치기 A~C/화염분출/선회/화염링/인형 집중/메두사) |

`[사용안함]` 접두 4개(`4219928/29/30`, `4219957`)는 원본이 폐기 표기한 액션이다. 제품 후보에서 뺀다.

### 1.4 저작에 들어가기 전에 결정해야 하는 것

발탄과 같은 계약으로 가려면 쿠크세이튼도 아래 6종을 새로 만들어야 한다.
지금은 하나도 없으므로 **첫 패턴 하나를 끝까지 관통시키는 수직 슬라이스**부터 잡는 것이 맞다.

```text
Data/Actors/BossCatalog.json          archetypeId / bodyModel / preScale 추가        (formatVersion 6 -> 7?)
Data/Balance/BossProfiles.json        HP / 이동 / 충돌 반경 / phase 임계
Data/Kakul/Kakul.gameplay.json        Server stage · branch · motion · collider · reaction
Data/Kakul/Kakul.presentation.json    ordered animation occurrence · Effect/Camera invocation
Data/Kakul/Kakul.combatobjects.json   FIXED_AREA / MISSILE 정의
Data/Encounters/KakulSaydon/*.json    projector 생성물 (직접 편집 금지)
```

여기에 Server `GameplayCatalog` row tag, brain, `Publish-GameplayBalance.ps1` 검증,
`GAMEPLAY_BOOTSTRAP_FORMAT_VERSION` bump가 하나의 원자 커밋으로 따라온다.

**3장 결함이 여기에 직접 걸린다.** 아래 3.4를 먼저 읽고 첫 패턴을 고를 것.

---

## 2. Composition Workbench Save 병목 실측

09-02 PLAN의 A1~A4는 그 뒤 변경으로 상태가 달라졌다. 아래는 **현재 코드 재실측**이다.

### 2.1 A1~A4 현재 상태

| 09-02 항목 | 현재 | 근거 |
|---|---|---|
| A1 loop slot append 거부 | **여전히 있음** | `ActionCompositionWorkbench.cpp:4055` — timeline item의 editable이 `StageDraft.animationEditable && !Slot.repeatUntilStageEnd` |
| A2 WAIT stage 거부 | **현재는 무해** | `Valtan.presentation.json` 193 stage의 `sequenceRole` 집계에 `WAIT`가 **0개**다. `isWaitStage`는 지금 항상 false |
| A3 `3u == Clips.size()` 상수 | **제거됨** | `Clips.size()` 사용처에 상수 비교 없음 |
| A4 editable 3종 모두 false | **부분 해소** | `BalanceTool.cpp:1677~1700` — `durationEditable`은 무조건 true, `animationEditable`은 manual audition **또는** clip이 있는 canonical stage까지 열림. `stageKindEditable`만 manual audition 한정 |

### 2.2 지금 실제로 저작을 막는 두 경계

#### (1) canonical pattern 11개는 collider Add/Remove와 stageKind가 잠겨 있다

`Data/Valtan/Valtan.gameplay.json`의 `decisionModel.manualAuditions`는 30개다.
전체 pattern은 41개이므로 **11개가 canonical**이고, `BalanceTool.cpp:1677/1684/1687`이
`pattern.bManualServerAudition`을 요구하는 세 admission이 전부 닫힌다.

```text
VALTAN_ENTRANCE_CINEMATIC              VALTAN_ARENA_BREAK_109
VALTAN_WHIRLWIND                       VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK
VALTAN_DASH_CHARGE                     VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK
VALTAN_FOUR_SLASH                      VALTAN_TRIPLE_COUNTER
VALTAN_FIST_IN_OUT
VALTAN_HIGH_JUMP
VALTAN_FLOOR_WIPE_130
```

`colliderTuneAdmitted`만 canonical에도 열려 있다(기존 collider의 제자리 튜닝).
즉 **"collider가 없는 canonical stage에 collider를 새로 붙이는 것"이 구조적으로 불가능**하다.
09-02의 R 항목(스킬 94개 중 43개 미저작)과 T2/T3가 여기서 막힌다.

`manualAuditions` 30개의 내부 구성은 `MANUAL_SERVER_AUDITION` 23 + `DERIVED_SERVER_PATTERN` 7이다.
`ValtanPatternTree.cpp:6272`는 둘을 구분하지 않고 목록에 있으면 `bManualServerAudition = true`로 둔다.
따라서 실제 경계선은 admissionState가 아니라 **"manualAuditions 목록 안에 있는가"** 하나다.

#### (2) loop slot이 있는 42개 stage는 Animation lane이 read-only다

`Valtan.presentation.json` 193 stage 중 `endPolicy` 분포는 다음과 같다.

```text
EXACT             144
LOOP_TO_STAGE_END  42
HOLD_LAST_POSE      5
(none)              2
```

`repeatUntilStageEnd=true` occurrence를 가진 stage는 정확히 **42개(21.8%)**이고,
그 stage의 Animation timeline item은 `editable=false`가 되어 Append/이동/교체가 전부 막힌다.
사용자가 자주 건드리는 것들이 여기에 몰려 있다.

```text
VALTAN_WARP           STEP_02~STEP_09 (8개)
VALTAN_GHOST_FINALE   STEP_02~STEP_09 (8개)
VALTAN_TERRAIN_DESTRUCTION STEP_02/05/08/13
VALTAN_SIX_PIZZA_106  STEP_06, STEP_10
VALTAN_STAGGER_SLOT   CHANNEL
VALTAN_FOUR_SLASH / WHIRLWIND / FLOOR_WIPE_130  WINDUP·RECOVERY
...
```

### 2.3 Save_Reload 자체의 게이트 16개

`ActionCompositionWorkbench.cpp:4674` `Save_Reload()`는 순서대로 아래에서 실패한다.
어떤 문구가 떴는지가 곧 원인 지목이므로, 재현 시 **status 문자열 전문을 그대로 남기는 것**이 가장 빠르다.

| # | 실패 문구 앞머리 | 조건 |
|---|---|---|
| 1 | `[Pattern] Save is not ready.` | `m_eAdmission != ADMITTED` 또는 BalanceTool 없음 |
| 2 | `[Pattern] The Pattern files changed after this window opened.` | pin한 authoring/canonical revision 불일치 -> `STALE_PRESERVED`로 강등 |
| 3 | (성공) `There are no staged ... changes to save.` | Pattern/Sound/EffectV2 draft가 전부 clean |
| 4 | `[Pipeline] Save is not ready because the Boss Tool ...` | `m_pBossTool == nullptr` |
| 5 | `[Pattern] ... Animation Tool is unavailable` | Pattern dirty인데 Animation Tool 없음 |
| 6 | `[Sound] The changed Sound owner is unavailable` | Sound dirty인데 Animation Tool 없음 |
| 7 | `[Sound] Save is pinned while a Server Pattern occurrence owns ...` | Server가 현재 Sound generation을 점유 |
| 8 | `[Pattern] Pattern data could not be prepared` | `Get_ValtanPatternDraft` 실패 |
| 9 | `[Pattern] The Animation links are invalid` | `Validate_ValtanCompositionAnimationGraphMutations` 실패 |
| 10 | `[Sound] The Sound links are invalid` | Sound graph dependency 실패 |
| 11 | `[Sound] Sound data could not be staged` | `Prepare_ValtanCompositionPatternSoundSave` 실패 |
| 12 | `[Sound] Sound data changed while Save was being prepared` | dirty 플래그 경합 |
| 13 | `[EffectV2] Effect V2 data could not be staged` | `Prepare_BossValtanBindingDraftSave` 실패 |
| 14 | `[EffectV2] Effect V2 data changed while Save was being prepared` | dirty 플래그 경합 |
| 15 | `[Pipeline] Nothing was saved.` | transaction 자체 실패 |
| 16 | `[Pipeline] Saved the canonical files. Boss Tool could not reopen ...` | 저장은 됐고 reload만 실패 |

2번이 특히 잘 걸린다. **다른 창(Balance Tool / Effect Tool / Boss Tool)이나 다른 세션이 같은
source를 저장하면 이 창의 pin이 즉시 stale이 되고, 그 뒤 모든 Save가 1번으로 떨어진다.**
Workbench를 열어 둔 채 다른 도구로 저장했다면 Workbench를 닫았다 다시 여는 것이 정상 절차다.

### 2.4 정리 — 무엇을 고쳐야 "save가 되는 상태"가 되나

```text
막는 순서
  (a) manualAuditions 목록 밖 11 pattern  -> collider Add/Remove·stageKind 불가         [데이터 경계]
  (b) loop slot 42 stage                  -> Animation lane 자체가 read-only            [코드 경계]
  (c) revision pin stale                  -> Save 전부 1번으로 낙하                     [운영 절차]
  (d) Save_Reload 게이트 16개             -> 위 셋을 통과한 뒤에야 도달                 [정상 검증]
```

(a)는 `manualAuditions`에 항목을 추가하는 데이터 변경이지만, 그 목록은 **Server 저작 권한 경계**이므로
09-02 S2 교정처럼 "canonical에도 열 것 / manualAudition으로 승격할 것"을 항목별로 정해야 한다.
승격은 topology 변경 권한까지 같이 열리므로 무조건 넓히면 안 된다.

(b)는 `!Slot.repeatUntilStageEnd`가 "loop slot 뒤에 clip을 이어 붙일 수 없다"는 의미로 걸려 있다.
loop slot은 stage 끝까지 늘어나므로 **뒤에 붙일 수 없는 것이 맞다.** 그러나 현재 코드는
`editable=false`를 slot 자체에 걸어 **loop slot 앞에 붙이거나 loop slot 자신을 교체하는 것까지** 막는다.
"마지막 loop slot 뒤에만 금지"로 좁히는 것이 정확한 규칙이다.

---

## 3. "Valtan replication observed a disconnected Server session" 원인

### 3.1 사용자가 본 두 문구의 실제 출처

```text
Lobby 화면 "Server entry failed."
  -> Client/Private/Level_Lobby.cpp:89
     Lobby가 recovery snapshot을 소비할 때 무조건 찍는 고정 문구다.
     입장이 거절됐다는 뜻이 아니라 "직전 세션이 비정상 종료돼 Lobby로 돌아왔다"는 뜻이다.

Lobby 화면 "Failure detail: Valtan replication observed a disconnected Server session."
  -> Client/Private/Level_ValtanArena.cpp:539
     Valtan Level이 소켓 단절을 관측했을 때 붙이는 문구다.
```

두 문구 모두 **원인이 아니라 결과**다. 실제 원인은 다른 곳에 기록돼 있다.

### 3.2 실제 로그 증거

Client 진단(`Client/Bin/Debug/Diagnostics/client-session-*.jsonl`) — 최근 Valtan 세션 4건:

```text
reason = CLIENT_PEER_CLOSED
detail = "Server completed an orderly TCP close (FIN)."
```

즉 Client 파싱 실패도, 네트워크 끊김도, Server 크래시(RST)도 아니다. **Server가 정상적으로 세션을 닫았다.**
(비교: 다른 1건은 `CLIENT_RECEIVE_ERROR wsa=10054`로 이건 실제 프로세스 소멸 케이스다.)

Server 진단(`Server/Bin/Debug/Diagnostics/server-session-*.jsonl`)에 원인이 그대로 있다.

```text
reason  = SERVER_ROOM_RUNTIME_FAILED
context = assigned room stopped after a runtime failure;
          enqueueResult=REJECTED_ROOM_NOT_READY worldId=2 roomReady=false
          firstFailureTick=10255
          firstFailureSource=world-update.pattern-scheduled-spawn-wave
          firstFailureDetail=Boss-relative combat object leaves navigable arena:
            patternId=VALTAN_SIX_PIZZA_106
            actionId=valtan.sequence.center-six-pizza-charge.step-01
            combatObject=combatobject.valtan.six-pizza.rock-pillar
            ordinal=0 bossX=156.03 bossZ=-122.06 spawnX=157.724091 spawnZ=-112.306534
```

같은 형태로 `VALTAN_STRUGGLING` / `combatobject.valtan.struggling.rock-pillar` ordinal=3도 잡혔다.
**사용자의 추측(피자 패턴이 나올 때)이 로그로 정확히 확인된다.**

### 3.3 인과 사슬

```text
VALTAN_SIX_PIZZA_106 / STEP_01 ENTER
  event.valtan.six-pizza.rock-pillars, firstOffsetMs=1000  (지연 wave)
    -> tick +30에서 Apply_BossPatternScheduledSpawnWave
    -> Stage_BossPatternStageActions의 BOSS_RELATIVE RADIAL 검사
       degrees = bossYaw + 45 + 90*ordinal, r = 9.8994949366
    -> Is_PointWalkableExact(x, z) == false
    -> m_strStatus 설정 후 return false                      (GameRoom.cpp:10015)
    -> Apply_BossPatternScheduledSpawnWave -> false
    -> Mark_RuntimeFailure("world-update.pattern-scheduled-spawn-wave")  (GameRoom.cpp:13479)
    -> m_isReady = false                                     (GameRoom.cpp:1372)
    -> room이 snapshot 송신·command 수신을 모두 중단
    -> ServerApp이 REJECTED_ROOM_NOT_READY를 보고 세션을 SERVER_ROOM_RUNTIME_FAILED로 close
    -> Client가 FIN 관측 -> CLIENT_PEER_CLOSED -> Lobby 복귀 -> "Server entry failed."
```

한 발의 장식용 돌기둥 배치 거절이 **방 전체를 죽이고 접속자 전원을 튕긴다.**

### 3.4 왜 좌표가 navigation 밖인가 — 데이터 실측

거절된 좌표는 static navgrid 기준으로는 전부 **walkable**이다.
실제로 막은 것은 `LV_LUT_HEARTRB_ED.navblockers`의 **아직 부서지지 않은 벽 region**이다.

```text
navblockers region 104개  =  polarity 1 (부서지면 막힘) 6개
                             polarity 0 (부서지기 전까지 막음) 98개
ServerNavigation.cpp:262   conditionValue == bActivateWhenConditionTrue 이면 block
초기 condition은 전부 false  ->  polarity 0 region 98개가 전투 시작 시점에 전부 활성
```

거절된 세 좌표가 속한 region:

| 좌표 | 소속 region |
|---|---|
| pizza ordinal 0 `(157.72, -112.31)` | `navregion.valtan.wall.12956154234820310428` 외 4개 (polarity 0) |
| pizza ordinal 2 `(147.37, -126.86)` | `navregion.valtan.wall.15736179535184396369` 외 2개 (polarity 0) |
| struggling ordinal 3 `(147.67, -110.74)` | `navregion.valtan.wall.17695249952621776147` (polarity 0) |

아레나 중앙(피자 착지 anchor `156.03, -122.06`) 반경 15m 안에서
**static walkable 2,821 cell 중 1,893 cell(67.1%)이 서 있는 벽 region에 덮여 있다.**
초기 가용 지형은 십자 통로 형태이고, 벽이 부서지면서 넓어진다.

이 상태에서 링 반지름별 실패율을 boss yaw 0.1도 단위 3,600회로 전수 계산하면 다음과 같다.

```text
anchor (156.03, -122.06)
  r = 9.8994949366  (Six Pizza)          -> yaw 100.0 % 에서 방이 죽는다
  r = 4.9497475     (Ground Roar 표준)   -> yaw  75.1 % 에서 방이 죽는다
struggling 관측 위치 (152.30, -112.50)
  r = 4.9497474683                       -> yaw 100.0 % 에서 방이 죽는다
```

**간헐적 버그가 아니다. 벽이 살아 있는 동안 Six Pizza는 반드시 방을 죽인다.**
Ground Roar와 Part Break가 먼저 같은 문제를 냈기 때문에 그 둘만 예외 목록에 들어가 있었고,
같은 구조인 나머지 둘이 지뢰로 남아 있었다.

네 archetype은 구조가 완전히 동일하다.

```text
combatobject.valtan.ground-roar.rock        FIXED_AREA  dir NONE  hits 0  presentationEvents 1
combatobject.valtan.part-break.rock         FIXED_AREA  dir NONE  hits 0  presentationEvents 1
combatobject.valtan.six-pizza.rock-pillar   FIXED_AREA  dir NONE  hits 0  presentationEvents 1
combatobject.valtan.struggling.rock-pillar  FIXED_AREA  dir NONE  hits 0  presentationEvents 1
```

`hits`가 비어 있으므로 **gameplay 권한이 전혀 없는 순수 연출물**이다.
연출물에 navigation admission을 요구한 것 자체가 범주 오류다.

### 3.5 즉시 수정 — 다른 세션이 이미 반영 중

이 조사와 동시에 다른 세션이 `Server/Private/GameRoom.cpp:9969~9990`의
`visualCardinalRocksMayStartOffNavigation` 예외 목록에 `six-pizza.rock-pillar`와
`struggling.rock-pillar`를 추가했고, `ServerGameplayContractTests.cpp`와
`Tools/ValtanPipeline/test_valtan_rock_pillar_group_contract.py`에 계약 테스트를 붙였다.
그 diff는 위 인과 사슬을 정확히 끊는다. **이 문서에서는 같은 파일을 중복 수정하지 않았다.**

### 3.6 재발 방지 — 남은 두 가지

즉시 수정은 "이번 두 개"만 막는다. 같은 함정이 다음에 또 열린다.

#### 방지 1. 예외를 ID 목록이 아니라 구조 규칙으로

현재 예외는 `(archetypeId, patternId, actionId)` 3중 pin + `4u == count`다.
`ground-roar`와 `part-break`만 pin했을 때 `six-pizza`와 `struggling`이 지뢰가 된 것처럼,
**쿠크세이튼 패턴을 새로 저작하면 같은 지뢰가 다시 생긴다.**

구조 규칙은 이미 코드 안에 다 적혀 있다.

```cpp
BOSS_COMBAT_OBJECT_KIND::FIXED_AREA        == definition->eKind
BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE  == definition->eDirectionPolicy
definition->Hits.empty()
!definition->PresentationPulses.empty()
```

이 네 조건이면 그 오브젝트는 damage도 판정도 없는 연출물이다.
ID pin과 `4u == count`를 빼고 이 네 조건만 남기면, 앞으로 저작하는 모든 연출용 링이
자동으로 안전해진다. 반대로 `Hits`가 하나라도 생기는 순간 admission이 다시 엄격해진다.

트레이드오프: 데미지 없는 연출 오브젝트를 navigation 밖에 두는 것이 **암묵적으로 허용**된다.
현재 목록 pin은 "허용 대상을 사람이 하나씩 승인한다"는 의도였는데, 그 승인 비용이 곧
방 전체 다운으로 되돌아왔다.

#### 방지 2. 저작 내용 거절이 방을 죽이지 않게

더 중요한 쪽이다. **어떤 예외 규칙을 쓰든, 저작 데이터 한 줄 때문에 접속자 전원이 튕기면 안 된다.**

현재 `Stage_BossPatternStageActions`는 아래 둘을 같은 `return false`로 처리한다.

```text
(A) 런타임 불변식 위반   — 정말로 방을 멈춰야 하는 것
(B) 저작 배치 admission 거절 — 이번 건. 아무것도 commit되지 않았고 방 상태는 멀쩡하다
```

(B)는 `Stage_BossCombatObject`에 도달하기 전이라 **부분 commit이 없다.**
같은 함수 안에 이미 선례도 있다. `LOCKED_TARGET_UNTIL_FIRST_PULSE`에서 유효 타깃이 없으면
`break`로 빠져나가 "오브젝트를 만들지 않고 stage에 그대로 진입"한다.

navigation admission 거절도 같은 처리를 하면 된다.

```text
현재  m_strStatus = "...leaves navigable arena..."; return false;
       -> Mark_RuntimeFailure -> m_isReady=false -> 전원 접속 종료

제안  m_strStatus / std::cerr에 같은 문자열을 그대로 남기고
       그 volley 하나만 skip (switch case를 break)
       -> 방은 살아 있고, 돌기둥만 안 나오며, 원인은 그대로 로그에 남는다
```

`AGENTS.md`의 "실패 이유를 보존한다 / silent fallback 금지"는 지켜진다.
조용히 넘어가는 것이 아니라 **같은 진단을 남기되 폭발 반경만 그 이벤트로 좁히는 것**이다.

#### 방지 3(선택). publish 시점 검사

`Publish-GameplayBalance.ps1` 또는 `Tools/ValtanPipeline`에 다음을 추가하면
런타임에 도달하기 전에 저작 단계에서 잡힌다.

```text
BOSS_RELATIVE + RADIAL volley 전부에 대해
  해당 Area의 초기 navigation 상태(navgrid + polarity 0 blocker 전부 활성)에서
  yaw 0~360도 전수로 링 좌표를 계산하고
  Hits가 있는 volley가 한 각도라도 navigation 밖이면 publish 실패
```

이건 쿠크세이튼 패턴을 새로 만들 때 특히 값이 크다.

### 3.7 참고 — 같은 문구가 뜨는 다른 경로

`Level_ValtanArena.cpp`의 같은 문구는 Server room failure 말고도 아래에서 뜬다.
다음에 또 보이면 **Lobby 문구가 아니라 `Server/Bin/<Config>/Diagnostics/server-session-*.jsonl`의
`firstFailureSource`를 먼저 볼 것.** 원인이 거기 한 줄로 적혀 있다.

```text
CLIENT_PEER_CLOSED     Server가 정상 종료 -> 대부분 room runtime failure
CLIENT_RECEIVE_ERROR   wsa=10054  -> Server 프로세스 소멸/강제 종료
CLIENT_CONNECT_FAILED  애초에 접속 실패 (Lobby 단계)
```

Lobby 패널은 현재 `strSource`와 `strDetail`만 표시하고
`CLIENT_RECOVERY_DIAGNOSTIC::eReason`과 `wsaError`는 화면에 내보내지 않는다
(`Level_Lobby.cpp:515`). 위 세 가지를 구분하려면 지금은 JSONL을 봐야 한다.
Lobby 패널에 `eReason`/`wsaError` 한 줄을 더 찍는 것만으로도 다음 재현의 분류 시간이 크게 준다.

---

## 4. 검증 상태

| 항목 | 상태 |
|---|---|
| 쿠크세이튼 action 매핑 | `actionreference.json` 4종 직접 파싱. displayName 원문 그대로 |
| Composition 병목 수치 | `Valtan.gameplay.json` / `Valtan.presentation.json` / `BalanceTool.cpp` / `ActionCompositionWorkbench.cpp` 직접 실측 |
| 피자 원인 | Server/Client JSONL 실제 로그 + navgrid/navblockers 바이너리 전수 계산 |
| 코드 반영 | **이 문서에서는 없음.** 3.5의 즉시 수정은 다른 세션 소유, 3.6은 사용자 결정 대기 |
| 빌드 / 화면 | 미실행. Client 화면 판정은 사용자 전용 |
