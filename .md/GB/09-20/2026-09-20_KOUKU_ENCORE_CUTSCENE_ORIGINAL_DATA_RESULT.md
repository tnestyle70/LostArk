# 쿠크세이튼 앵콜(빙고 진입) 컷신 — 원본 데이터 조사 결과

작성일: 2026-09-20
요청: 사용자가 첨부한 영상 `C:\Users\USER\OneDrive\바탕 화면\앵콜 컷신 .mp4`(17.9초, 2160x1440, 30fps)가 "빙고로 들어가는 앵콜 컷신"인데 원본 데이터에 어떻게 되어 있는지 찾는다.
범위: **원본 데이터 조사만** 했다. 저장소의 코드·데이터·Resources는 바꾸지 않았고 빌드·publisher·Client/Server 실행도 하지 않았다. 원본 설치 폴더는 읽기만 했다(쓰기·삭제 없음).

이 문서는 세 등급으로 나눠 적는다.
- **[실측]** 원본 파일(TriggerMapData.loa, GameMsg/SceneReplay/Npc DB, 컷신 UPK)을 이번 세션에서 직접 읽은 값
- **[인용]** 09-10 조사 문서(`.md/GB/09-10/2026-09-10_KOUKU_ENCORE_FINAL_CUTSCENES_PLAN.md`, 바탕 화면 `앵콜컷신 진짜 마무리 컷신.txt`)가 읽은 값을 이번에 다시 재측정하지 않고 가져온 것
- **[추정]** 위 근거로 추론한 것

## 결론

**찾았다. 영상은 원작의 공식 컷신 "클리어를 미루는 쿠크와 세이튼"이고, 3관문 전투가 끝난 직후 빙고판으로 넘어가는 이동 연출의 한가운데에 들어 있다.** 해석("클리어처럼 보이는 가짜 연출 → 빙고 국면")은 원본이 뒷받침한다.

- 자막 세 줄은 `EFTable_GameMsg`의 `cin.37081_12_01/02/03`이다 **[실측]**.
- 컷신 정의는 UPK `B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk`(SCENE07A)의 Matinee `efseqact_matinee_23`(interpdata_23, 23.333초)이다 **[실측]**. 재생 신호 이름은 `37081_331`이다.
- 언제 재생되는지는 존 37081 `TriggerMapData.loa`의 유닛 3123(최초)/3133(2회 이상)에 있다 **[실측]**. 3관문 보스(NPC 480631)의 AI 이벤트 `Event_07`이 시작 조건이다.
- 이 컷신이 끝나면 같은 유닛이 플레이어를 빙고판 (−3.04, 0, 1149.96)으로 옮기고 39명의 NPC를 스폰한다 **[실측]**.
- 프로젝트에는 이 컷신의 재생·가짜 클리어 UI·깨진 유리·서버 전이가 **아직 없다**. 09-10에 계획서만 있다.
- 못 정한 것은 6절에 모았다. 특히 `FakeUI` 신호를 누가 받아 엠블럼을 그리는지, AI가 `Event_07`을 언제 내보내는지는 이번 자료로 확정하지 못했다.

## 1. 자막 원문과 메시지 ID

표: `EFTable_GameMsg`(706,743행, 열 `KEY`, `MSG`, `SOUND`…). data2.lpk의 DB를 복호화한 사본에서 바이트 검색해 찾았다 **[실측]**.

| KEY | 원문 | 컷신 안 시각 / 길이 |
|---|---|---|
| `cin.37081_12_01` | 누구 맘대로 끝을 내?! | 10.233초 / 1.70초 |
| `cin.37081_12_02` | 무효야, 전부 무효! | 12.367초 / 3.60초 |
| `cin.37081_12_03` | 진짜 시작은 지금부터라고! | 16.700초 / 3.05초 |

시각·길이는 SCENE07A의 `efinterptracksubtitle_0`(export 15) `subtitleinfoarr`에서 읽었다. 세 항목 모두 `msgtype=egamemsg_cinematic`, `msgid=-1`, 위치 `normal`이다.

같은 존의 다른 컷신 자막(`cin.37081_*`, 25행)도 함께 확인했다.
- `_22_01/02`: 1관문 → 다음 장 이동 컷신(패키지 8T6, 21.9초·35.0초)
- `_25_01/02`: 2관문 진입 전반(패키지 806, SCENE04A)
- `_29_01`, `_31_01~06`: 장르를 바꾸는 쿠크와 세이튼(패키지 8M6)
- `_32_01`: 렛츠 쇼타임(패키지 8MD)
- `_33_01~05`: 작별 인사(패키지 8FD와 8FK, 14.5~43.4초)
- `_49_01~05`: `_33`과 글자가 완전히 같은 행. 이 행을 쓰는 패키지는 찾지 못했다 **[실측: 사용처 미발견]**

"앵콜/빙고" 문자열 **[실측]**
- `tip.name.monster_480635` = **앵콜을 외친 쿠크세이튼** (빙고판에서 싸우는 그 보스)
- `tip.name.skillbuff_4219964` = 빙고
- `sys.achievement.name_10010083` = 빙고!, `desc` = 생존한 상태에서 빙고 완성하기, `obj_10010083_1` = 빙고 25회 완성
- 그 밖의 "앵콜"(아이템 옵션, 다른 존 대사 `cin.10401_15_02`)은 이 컷신과 무관하다.

## 2. 컷신 정의 위치와 구조

### 2.1 공식 이름과 재생 목록 **[실측]**
`EFTable_SceneReplay`(컷신 다시보기 표) PK **3708104 "클리어를 미루는 쿠크와 세이튼"**, ZoneId 37081, `TriggerUnitIndex` 10004, 그룹 `contents_commanderraid_37081_1`(광기군단장 쿠크세이튼), SceneIndex 3 / SubIndex 1.

같은 그룹의 나머지 행: 3708100 서커스의 시작(10000), 3708101 다음 장으로 이동하는 세이튼(10001), 3708102 장르를 바꾸는 쿠크와 세이튼(10002), 3708105 쿠크와 세이튼의 작별 인사(10005). 3708103 행은 없다. 유닛 10003 "[3119] 리허설을 끝내는 쿠크와 세이튼"은 다시보기 표에 행이 없다.

### 2.2 재생 방식 **[실측]**
`TriggerMapData.loa`(213,635바이트, 유닛 160개)의 유닛 **10004**(다시보기용): `Condition_Player(Signal)` → `SpawnTrap` → `SceneEvent`(이름 없음) → `SceneEvent "37081_331"`.

실제 진행에서는 유닛 **3123**(최초, 라벨에 "스킵불가")과 **3133**(2회 이상, "스킵가능")이 같은 `SceneEvent` 사슬을 가진다. 유닛 3198은 "보관용" 복사본이다.

두 유닛의 머리 부분 3개 u32(word 3: 2 대 0, word 8·9: 0,1 대 1,0)가 다르다. 스킵 여부·발동 횟수 플래그로 보이지만 의미는 해독하지 못했다 **[추정]**.

### 2.3 시작 조건 **[실측 + 추정]**
유닛 3123의 첫 노드는 `Condition_NPC`로, 배치 액터 **52**(NPC **480631** 쿠크세이튼_3페이즈, 모델 `EFDLChar_MN_RPCT_07.MN_RPCT_07`, 위치 (0.00, 1.31, 942.08) m)의 AI 이벤트 **`Event_07`**이 나오면 사슬이 시작된다.
- NPC 표에서 480631과 480635(앵콜을 외친 쿠크세이튼)는 같은 모델 `MN_RPCT_07`을 쓴다 **[실측]**.
- 이 `Event_07`을 보스 AI 스크립트가 언제 내보내는지(HP 조건인지 페이즈 종료인지)는 읽지 않았다 **[미확인]**.

### 2.4 유닛 3123 노드 순서 **[실측]** (좌표는 UE cm → (x, z, −y)×0.01 m)
1. `Condition_NPC` 액터 52 `Event_07`
2. `SetTriggerUnitEnableState` (유닛 3121, 3212, 3214, 3216, 3218, 3221 등 10개 끔)
3. `CancelInstanceTimer`, 4. `AddInvincible`
5. `ChangePropProperty` 36개, 6. `DespawnProp` 6개
7. `Delay` **4.0초**, 8. `RemoveContentsGauge` 3708100, 9. `AddContentsBuff` 3708101
10. `DespawnNPC` 11개(액터 53~60, 70, 335, 551 = NPC 480651 ×8, 480652, 480692, 480744)
11. `TeleportPlayer` → **(−6.10, 1.31, 948.11)** (3관문 서커스 천막 안)
12~13. `RemoveBuff` 411100, 4220061
14. `SceneEvent "37081_320"` (패키지 8F6에서 카메라 대상 지정 + `SetMaterialEffectProperties`로 이어짐. 정확한 역할은 **[미확인]**)
15. `Delay` 1.0초, 16. `CancelInstanceTimer`
17. `SceneEvent`(이름 없음, 파라미터 블록이 다름 — 의미 **[미확인]**)
18. **`SceneEvent "37081_331"` = 앵콜 컷신**
19~20. `RemoveContentsBuff` / `AddContentsBuff` 3708101
21~22. `ChangeStoredRevivePosition`, `ChangeDefaultWarpPosition` → **(−3.00, 0, 1150.00)** (빙고판)
23. `TeleportPlayer` → **(−3.04, 0.00, 1149.96)**
24. `RemoveContentsBuff`, 25. `RemoveInvincible`, 26. `AddContentsGauge` 3708100
27. `SpawnNPC` 39명 (아래 3절)
28~29. `SpawnProp` (268435930, 268435943)
30~33. `Check_ZoneLevel`(값 0 / 4) → `RemoveBuff` 422202314

SceneEvent가 컷신이 끝날 때까지 다음 노드를 막는지는 노드 바이트에서 확정하지 못했다. 다만 컷신 Matinee가 끝나면 `EndRemoteEvent 37081_331`을 보내도록 짜여 있어(2.5절), 대기 신호는 있다 **[추정: 대기 구조]**.

### 2.5 컷신 본체 (패키지 8L6 = SCENE07A) **[실측]**
`ReleasePC\Packages\B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk` (이름 554, import 145, export 228). 이름 표에 `37081`, `fakeui`, `bgm_midnightc_ed_m18_scene_fakeclear`, `scene_midnightc_ed_koukustopclearingdungeon`, `efinterptracksubtitle`이 들어 있다.

Kismet 연결
- `seqevent_remoteevent "37081_331"` → `AttachToActor_0` → `SetCameraTarget_2` → **`efseqact_matinee_23`** (interpdata_23, 길이 **23.3333초**, group 23개), 그리고 `ToggleHidden_0`.
- `efseqact_matinee_23`의 `Completed` → `Destroy`, `AttachToActor_1`, **`efseqact_endremoteevent "37081_331"`**, `ToggleHidden_0`. `S1` 출력 → `CameraShake_0`. **`FakeUI` 출력은 이 패키지 안에 연결된 노드가 없다**(원본 코드/UI가 받는 것으로 보이며 **[미확인]**).
- 다른 Matinee `efseqact_matinee_0`(3.067초, group 2)은 원격 이벤트 `37081_111`로 도는 별개 연출이다(1관문 종이무대 펼침).
- `seqevent_touch` 2개와 `eflocaltrigger`가 있으나 테스트용 잔재로 보이며 실제 발동 경로는 원격 이벤트다 **[추정]**.

트랙 값
- Director 컷: 1개 (−0.133초, 카메라 그룹 `cam`, shotnumber 10). 다른 컷은 없다.
- Event 트랙: `fakeui`@0, `s1`@12.500, `s1`@13.533, `s1`@15.433.
- **PlaySWF** `efseqact_playswf_0`: swfobject = `vs.epicgatecommanderresulttest`, unloadtime **15.45초**. "던전 클리어" 엠블럼 UI의 정체로 보이나 SWF 내용은 열지 않았다 **[추정]**.
- **깨진 유리** post-render 재질 `fx_mi.fx_d_brokenglass_01_tr`(패키지 `bfx_mi_bg_00`)
  - `opacity`: 12.467초=0 → 12.500초=1 → 15.400초=1 → 15.433초=0 (`cim_constant`)
  - `type`: 12.500초=0 → 13.533초=1 (`cim_constant`)
- **사운드**(AkEvent 이름만 읽음)
  - BGM 트랙 `AkEvent_BGM`: 0.0초 `bgm_midnightc_ed_m18_scene_fakeclear`, **23.322초 `..._fakeclear_skip`**
  - 0.0초 `scene_midnightc_ed_popup1`
  - 2.1초 `scene_midnightc_ed_koukustopclearingdungeon`
- 보조 트랙: `efinterptrackautoblendfloatprop` 2개(8.267초 4000→1200, 1→3 — DOF로 보이나 **[추정]**), `hit_color` 벡터 재질 파라미터(7.8~15.43초 사이 흰색 1), 색상 트랙.

액터·이펙트 목록 **[인용, 09-10 문서 G05·G23; 이번에 export 클래스 개수만 재확인: interpgroup 24, emitter 17, particlesystemcomponent 18, skeletalmeshcomponent 4]**
- 카메라 `cam`(부모 `cameraactor_32`), 조명 `쿠크라이트`
- 보스 `쿠크세이튼_03`(LookInfo `MN_RPCT_07`, 실제 mesh `mn_rpct_05_sk`, 부모가 카메라라서 화면에 가까이 온다). `쿠크세이튼_02`는 배우가 연결되지 않은 group
- 이펙트: `spark0`=`par_c_ring_001`, `spark`=`par_q_colorpaper_01`(색종이), `disappear2`=`par_q_rpct_exp_01`, `move`=`par_y_cmdgr_03-1_spawn_01_loc_int`, `break`~`break12`=`par_e_shot_01` / `par_g_icebomb_01_pr` (15.433초에 12개 동시 TRIGGER)

### 2.6 영상 구간과 원본 요소의 대응
영상은 23.333초 컷신의 뒷부분이다. 영상 길이가 17.9초라 차이가 5.43초이고, 아래 4개 사건이 모두 "영상 시각 ≈ 원본 시각 − 5.4초"로 맞는다 **[추정: 프레임 표본 간격 0.9초라 ±0.9초 오차]**.

| 영상 구간 | 원본 시각(추정) | 원본 요소 |
|---|---|---|
| 0~6초 엠블럼, 2.7초 흰 천 | 5.4~11.8초 | 2D 클리어 UI(`fakeui` 0초, SWF `vs.epicgatecommanderresulttest`), 3D 보스 `쿠크세이튼_03`이 카메라 부모로 다가옴, `hit_color` 7.8초~ |
| 자막 "누구 맘대로…" 약 5.4~6.3초 | 10.233초(1.7초) | `cin.37081_12_01` |
| 7.2초 유리 균열 시작, 8.1초 큰 파편 | 12.5초 / 13.53초 | 깨진 유리 재질 `opacity` 0→1, `type` 0→1, `S1`(카메라 흔들림) |
| 자막 "무효야, 전부 무효!" 7.2~9.8초 | 12.367초(3.6초) | `cin.37081_12_02` |
| 9.8초 흰 섬광 | 15.43초 | 유리 `opacity` 1→0, `break` 12개 동시 TRIGGER, `spark`(색종이·링) TRIGGER |
| 10.7~14.3초 카드 왕관 보스 + 색종이 | 16.1~19.7초 | 보스 `쿠크세이튼_03` 연기, 색종이 |
| 자막 "진짜 시작은…" 약 11.6~14.3초 | 16.700초(3.05초) | `cin.37081_12_03` |
| 15.2초 분홍 섬광, 장갑 낀 손 | 20.2~20.9초 | `move` 이펙트 TRIGGER 20.233·20.700초 |
| 16.1~17.9초 광대 얼굴 | 21.5~23.3초 | 보스 클로즈업, 페이드 22.5→23.333초 **[인용]** |

영상 0초에 이미 엠블럼이 떠 있으므로 녹화는 컷신의 시작 부분(0~5.4초)을 잘라 냈다.
엠블럼과 자막은 UI, 유리는 화면 후처리 재질, 보스는 3D 스켈레탈 액터라서 서로 다른 세 층이다.

## 3. 컷신 직후 빙고 진입 흐름 **[실측]**

앵콜이 끝나면 유닛 3123이 이어서 다음을 한다(2.4절 19~33번).
- 플레이어 부활·워프 위치를 빙고판 (−3.00, 0, 1150.00)으로 바꾸고 전원을 (−3.04, 0.00, 1149.96)으로 이동
- `SpawnNPC` **39명**: 액터 71~108(38개)과 액터 150.
  - NPC 480653~480677 = 투명 몬스터 **빙고판 1~25번**
  - NPC 480678~480689 = **빙고담당 1~12번**
  - NPC 480690 = **빙고 관제탑**
  - 액터 150 = **NPC 480635 앵콜을 외친 쿠크세이튼**, 위치 (3.04, 0, 1143.80) m
  - (NPC 표에서 480724 = 해골 폭탄_빙고도 확인. 이번 스폰 목록에는 없다)
- 앵콜 전에는 3관문 쪽 NPC 11명이 사라진다(480651 ×8, 480652, 480692, 480744).

전투 종료와 진짜 클리어
- 유닛 **3151**: 액터 150(앵콜을 외친 쿠크세이튼)이 `Dead`이면 액터 152(NPC 480694)와 액터 52(480631)를 디스폰 = "3관문 전투종료 신호".
- 유닛 **3152**(클리어): `Condition_Dungeon`의 `Dungeon_Phase3_Clear` → `Check_ZoneLevel`(값 4)
  - `Yes` 가지: 액터 150과 액터 71~108을 지우고 `EndBossBattle` 등 → `SceneEvent 37081_341`
  - `No` 가지: 같은 정리 + `ReviveAllPlayer` → `SceneEvent 37081_342` → `StartTrophyAuction`
  - 패키지 확인: 341은 8FK(SCENE01C), 342는 8FD(SCENE01B), 둘 다 자막 `cin.37081_33_01~05`를 같은 시각에 쓴다.
  - 유닛 3125 라벨이 "[싱글난이도] 클리어 처리"라서 `Check_ZoneLevel` 값 4가 싱글 난이도 분기일 가능성이 있으나 열거값 의미는 읽지 못했다 **[추정]**.
- 던전 클리어 UI와의 선후: 앵콜의 엠블럼은 **가짜**이고, 보상(`StartTrophyAuction`)은 빙고 이후 유닛 3152에서만 나온다. 실제 클리어 UI가 먼저 뜨고 그 위를 덮어씌우는 구조가 아니라, 앵콜 컷신이 같은 종류의 UI를 스스로 재생한다 **[실측: 앵콜 트랙에 PlaySWF가 있음]**.

"[노말이상]" 라벨과 "[싱글난이도]" 클리어 라벨로 보아 빙고 국면은 노말 이상에서만 있다 **[추정]**.

## 4. 프로젝트에 이미 있는 것 / 없는 것

있는 것 **[실측: git grep 및 파일 열람]**
- 서버: `Update_KoukuBingo`, `m_KoukuBingo`(Fill/Start_Bomb), Debug 빙고 명령 3종, BINGO 관문 audition, `boss.kakulsaydon.bingo.saydon`(BOSS_KAKULSAYDON_BINGO_SAYDON, 비활성 배치)
- 보스 표시 이름 "앵콜을 외친 쿠크세이튼"(`Data/Balance/BossProfiles.json:124`), Workbench 보스 `KOUKU_SAYDON_ENCORE`("Encore Saydon", `SequencerTool.cpp`, `MainApp_WorldLevel.cpp:395`에서 BINGO 관문과 연결)
- 이펙트 `effect.kouku.bingo.encore.blackhole.beam.full.restore` = 빙고 전투에서 **앵콜 보스가 쏘는 블랙홀 빔**이다. 이 컷신과 이름만 같다.
- 작성된 패턴 `빙고_최종엔딩씬`(SCENE01B, 0~49083ms) = 진짜 마무리(작별 인사, 37081_342) 컷신
- 읽기 전용 참조 `KoukuSaydon.cinematicreference.json`의 `cinematic.encore.group.*` 2개 group(SCENE07A, matinee export 21, authority `REFERENCE_ONLY`)
- 실제 클리어 UI(`RaidClear_Kouku_Layout.json`, `Trigger_RaidClear`, `Level_KakulSaydonArena.cpp`)
- 09-10 설계서 G00~G26(가짜 클리어 화면 합성, 서버 전이 ENCORE_PREPARING → PLAYING → BINGO_HANDOFF)

없는 것 **[실측: 관련 이름 검색에 제품 hit 없음]**
- 앵콜 컷신(SCENE07A `interpdata_23`) 재생, `fakeui` 이벤트 처리, 깨진 유리 후처리, 가짜 클리어 UI 전용 뷰
- 서버의 ENCORE 상태, `Event_07`에 해당하는 전이 조건, 컷신 중 입력 잠금, 전원 빙고판 이동 커밋
- 사운드 후보 도구(`Tools/SoundPipeline/build_kouku_sound_candidates.py`)에는 `kouku.bingo.ending`(SCENE01B)만 있고 앵콜(SCENE07A)은 없다
- 3관문 → 빙고의 **제품** 전이 경로는 이번에 서버 코드를 따라가 보지 않았다(Debug 관문 이동은 있음) **[미확인]**

## 5. 이 프로젝트로 옮기는 방법 제안 (구현하지 않음)

재사용 가능
- 기존 Matinee → WorldSequence/Composition 변환 규칙과 카메라 shot 파이프라인(Matinee 이동 키는 계단식 순간이동, AnimControl은 층 구조 등, 메모리 `matinee-actor-to-worldsequence-conversion-rules`).
- 보스 몸체: `Character/KoukuSaton/MN_RPCT_05` 모델과 Composition의 MN_RPCT_07→05 alias(09-10 문서).
- 엠블럼 아트: `Data/UI/RaidClear/RaidClear_Layout.json`류 레이아웃 재사용. 단 앵콜용 SWF `vs.epicgatecommanderresulttest`가 Kouku 클리어 아트와 같은지는 확인하지 못했다 **[미확인]**.

새로 만들거나 추출할 것
- 3D: `쿠크세이튼_03`의 AnimControl 슬롯(a·b·fc1)과 float weight, 카메라 부모 체인 pose 샘플.
- 화면 층: 클리어 엠블럼(UI) → 깨진 유리(scene color를 쓰는 후처리인지 확인 필요) → 파편·색종이(파티클) → 자막. 09-10 문서가 권장한 합성 순서 그대로다.
- 파티클: `par_e_shot_01`, `par_g_icebomb_01_pr`, `par_q_colorpaper_01`, `par_c_ring_001`, `par_y_cmdgr_03-1_spawn_01_loc_int`. umodel이 ParticleSystem 내부를 못 읽으므로 이름만으로 Effect V2 자산을 새로 저작해야 한다.
- 사운드: AkEvent 4개 이름(`..._fakeclear`, `..._fakeclear_skip`, `..._koukustopclearingdungeon`, `..._popup1`)을 기존 Sound 자산으로 대응(이름만으로는 WAV 경로가 안 나온다).
- 자막: `cin.37081_12_01~03`을 10.233 / 12.367 / 16.700초에 띄우는 UI. 프로젝트에 자막 UI가 이미 있는지는 확인하지 못했다 **[미확인]**.

서버가 정해야 할 것
- 시작 조건: 원본은 보스 AI의 `Event_07`이다. 프로젝트에서는 3관문 본체 종료 조건(서버 권위)으로 대체해야 한다.
- 가짜 클리어: 보상·클리어 플래그·돌아가기 버튼을 만들지 않는다(원본도 보상은 유닛 3152에서만).
- 컷신 동안 입력 잠금과 무적, 종료 시각(23.333초)에 전원 (−3.04, 0, 1149.96)로 한 틱에 이동, 빙고 보스 `boss.kakulsaydon.bingo.saydon` 활성화.
- 원본은 이동을 두 번 한다: 먼저 (−6.10, 1.31, 948.11)로 옮긴 뒤 컷신 320(카메라 준비로 보임)과 331을 재생하고, 끝나면 빙고판으로 이동한다.
- 최초는 스킵 불가, 2회 이상은 스킵 가능(스킵 시 BGM `..._fakeclear_skip`이 23.32초에 걸린다).

어려운 부분
1. 엠블럼까지 깨지는 화면 합성(UI와 후처리의 그리기 순서). 월드 이펙트만으로는 UI가 멀쩡히 남는다.
2. 파티클을 원본에서 못 뽑는다는 점.
3. `쿠크세이튼_02` group이 배우에 묶여 있지 않고, AnimControl·autoblend·skeletal control(h_dn, j_dn 등)의 최종 합성이 09-10 문서에서도 미해석이다.
4. `FakeUI` 신호의 수신처를 모른다.
5. 영상과 원본의 시간 대응이 표본 간격 때문에 ±0.9초 오차다.

## 6. 확인하지 못한 것

- 보스 AI가 `Event_07`을 내보내는 조건(AI 스크립트 미확인).
- `FakeUI` 출력의 수신처와 SWF `vs.epicgatecommanderresulttest`의 내용.
- `SceneEvent "37081_320"`의 정확한 효과와 노드 17(이름 없는 SceneEvent)의 파라미터 의미.
- 유닛 3123/3133 머리부 플래그의 의미(스킵·발동 횟수), `Check_ZoneLevel` 값의 의미.
- 컷신 안 페이드 곡선·FOV·카메라·조명 값은 이번에 재측정하지 않았다(09-10 문서 인용).
- 영상의 음악·효과음은 ffmpeg가 없어 분석하지 못했다. 원본 AkEvent 이름만 보고한다.
- 파티클 이미터 내부 수치는 umodel이 ParticleSystem 클래스를 거부해 읽지 못한다.
- 3관문에서 빙고로 가는 프로젝트 서버 전이의 현재 구현 여부.

## 7. 추출·조사한 파일 위치

- 스크립트 19개와 데이터: `C:\LostArkExtract\KoukuEncore_20260919\scripts\`, `...\data\`(트리거 유닛 스캔 `encore_trigger_scan.txt`, 패키지별 원격 이벤트·자막 `encore_all_scenes.json`, 영상 프레임 모음 `video_contact_sheet_0p895.jpg`). 폴더 이름의 날짜는 조사를 시작한 09-19이다.
- 이번 세션 작업 사본: `C:\Users\USER\.claude\jobs\46aea322\tmp\`(`data2\` 복호화 DB, `mapdata\Common_Extra\MapData\37081\` 존 데이터).
- 원본 읽기 경로: `C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages\B9AVB2VAZIQRPQCJVKAVYRAVOKYPY{8L6, 8FD, 8FK, 806, 8E6, 8F6, 8M6, 8MD, 8T6}.upk`, `data2.lpk`, `leveldata1.lpk`.
- 함께 본 기존 문서: `.md/GB/09-10/2026-09-10_KOUKU_ENCORE_FINAL_CUTSCENES_PLAN.md`(및 RESULT), `Data/Animation/Reference/KoukuSaydon/KoukuSaydon.cinematicreference.json`.
- 재실행: `python encore_grep_dbs.py`(자막 DB 검색), `encore_units.py 3123 10004 …`(트리거 노드), `encore_all_scenes.py`(패키지별 이벤트), `encore_tracks.py`, `encore_ak.py`(트랙·사운드), `encore_spawn_ids.py`(스폰 NPC).
