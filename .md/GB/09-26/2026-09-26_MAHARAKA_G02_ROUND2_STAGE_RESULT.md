# 2026-09-26 마하라카 G02 2차 — 워터팡 무대 정체성 RESULT

1차가 후보 0개로 막히며 남긴 7개 검색 위치를 전부 추적했다. 조사 범위이며 정본 `Data/**`,
`Client/Bin/Resources`, 게시본, 코드를 **하나도 바꾸지 않았다.**

결론을 먼저 적는다. **무대를 찾았다.** 1차가 못 찾은 이유는 원본이 삭제돼서가 아니라
**프로젝트가 마하라카 서브레벨 14개 중 3개만 추출했기 때문**이다. 워터팡 무대는 한 번도
추출하지 않은 서브레벨 `lv_ocn_eventis_mhp_scene03b`에 있다.

## 1. 이번 G에서 실제 바꾼 것

코드·데이터·리소스·게시본 변경 **없음**. 새로 만든 파일은
`out/MaharakaContinuation_20260926_193455/G02_round2/` 24개뿐이다(스크립트 7, 결과 JSON 17).

참고로 21:01(내 시작) 이후 `Client/Private/Npc.cpp`, `Client/Public/Npc.h`,
`NpcActionEffectCueDocument.*`, `Tools/LevelPlacementExtractor/extract_source_map_component_lighting.py`와
그 테스트가 수정됐는데 **내 작업이 아니다.** 병렬로 도는 다른 작업의 것이며 나는 읽지도 않았다.

## 2. 가장 큰 발견 — 무대는 미추출 서브레벨에 있다

### 2.1 서브레벨 14개 중 3개만 추출됐다

`LV_OCN_EVENTIS_MHP_PS` package의 Kismet에서 `eflevelstreamingalwaysloaded` 13개와
`seqact_multilevelstreaming` 6개를 읽어 마하라카가 참조하는 서브레벨 전체를 열거했다.

| 논리 이름 | 물리 UPK | 추출 여부 |
|---|---|---|
| `lv_ocn_eventis_mhp_sl01` | `645QRFK5UT4TKQLJ5FDEY5KJ63A.upk` | **추출됨** |
| `lv_ocn_eventis_mhp_land01` | `867STHM7WV6VMSNL7HFG07M83MO5C.upk` | **추출됨** |
| (`_PS`) | `423OPDI3SR2RIOJH3DBCW3IWH.upk` | **추출됨** |
| `lv_ocn_eventis_mhp_scene02a` | `A89UVJO9YX8XOUPN9JHI29ONJXOX7L5.upk` | 미추출 |
| `lv_ocn_eventis_mhp_scene02b` | `A89UVJO9YX8XOUPN9JHI29ONJXOX7LC.upk` | 미추출 |
| `lv_ocn_eventis_mhp_scene03a` | `A89UVJO9YX8XOUPN9JHI29ONJXOX7S5.upk` | 미추출 |
| **`lv_ocn_eventis_mhp_scene03b`** | **`A89UVJO9YX8XOUPN9JHI29ONJXOX7SC.upk`** | **미추출 — 무대** |
| `lv_ocn_eventis_mhp_scene04a` | `A89UVJO9YX8XOUPN9JHI29ONJXOX7Z5.upk` | 미추출 |
| `lv_ocn_eventis_mhp_scene06a` | `A89UVJO9YX8XOUPN9JHI29ONJXOX7D5.upk` | 미추출 |
| `lv_ocn_eventis_mhp_scene07a` | `A89UVJO9YX8XOUPN9JHI29ONJXOX7K5.upk` | 미추출 |
| `lv_ocn_eventis_mhp_sl02` | `645QRFK5UT4TKQLJ5FDEY5KJ63H.upk` | 미추출 |
| `lv_ocn_eventis_mhp_envnpc03` | `A89UVJO9YX8XOUPN9JHI29OXO8O2J7S.upk` | 미추출 |
| `lv_ocn_eventis_mhp_soundstream` | `DBCXYMRC10B0RXSQCMKL5CRQY4RTQXJ08K.upk` | 미추출 |
| `lv_ocn_eventis_mhp_music` | `756RSGL6VU5ULRMK6GEFZ6LEYKMG.upk` | 미추출 |
| `standard_track` | `ELWFHW7H0AL7WAUDYV96B9.upk` | 미추출 |

물리 이름은 `extract_ue3_placements.resolve_physical_package`(UModel `-nameresolve`)로 해석했고
**12개 전부 실제 파일로 열려서 export 표까지 읽혔다.** 추정이 아니다.

### 2.2 무대 실체 — `scene03b`

이 서브레벨은 export 740개이고 내용이 정확히 무대다.

| class | 개수 | 의미 |
|---|---|---|
| `interpgroup` | 98 | Matinee 그룹 (`t01`~`t14` 각 4개) |
| `interptrackmove` | **87** | 이동·회전 트랙 |
| `interpactor` | **60** | 움직이는 액터 |
| `emitter` | 48 | 파티클 (48개 전부 좌표 있음) |
| `interptrackfloatmaterialparam` | 19 | 재질 파라미터 애니메이션 |
| `interptrackfloatprop` | 9 | 속성 애니메이션 |
| `efseqact_matinee` | **6** | Matinee 6개 (index 1~6, index 2만 looping) |
| `interptrackakevent` | 6 | 사운드 이벤트 트랙 |
| `interptracktoggle` | 5 | 가시성 토글 |
| `seqevent_remoteevent` / `efseqact_endremoteevent` | 5 / 5 | 외부 신호 수신 |

**interpactor 60개의 메시 내역(합계 정확히 60):**

| 개수 | 메시 |
|---|---|
| 35 | `bfx_sm_00.bfm_planbottom_01` |
| **9** | **`bg_ocn_etc_g.mesh.bg_ocn_etc_floor01a_sm_lnh`** |
| **9** | **`bg_ocn_etc_g.mesh.bg_ocn_etc_floor01b_sm_lnh`** |
| 6 | `bfx_sm_00.bfm_mossfog_001` |
| 1 | `fx_sm_00.fm_e_halfsphere_001` |

### 2.3 원형 무대 바닥의 실측 기하

floor01a 9개 + floor01b 9개 = **18조각이 교대로 원을 이룬다.**

- 중심: project `(75.07, -984.31)` — POOL01·워터캐논 NPC 570911과 **같은 XZ**
- 반경 **6.160~6.215 m** (18개 전부, 중앙값 6.172)
- 높이 **Y = 22.40 m** (18개 전부 동일)
- yaw 17종: `-19.69, 19.69, 39.38, 59.72, 79.85, 99.95, 119.69, 140.03, 160.31, 180.0,
  199.69, 219.38, 239.44, 259.65, 280.13, 300.48, 320.62`
- 인접 yaw 간격이 `19.69~20.48`로 **20° 등간격 18분할**(360/18)
- `floor01a` 9개는 재질 override `lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01a_mi_lnh`
  (**마하라카 전용 MIC**), `floor01b` 9개는 override 없음(메시 기본 재질)
- `hiddenGame=True`인 interpactor는 **0개**

**18조각 전부가 `InterpActor`다.** 즉 원본에서 움직이도록 만든 액터다.

좌표 매핑은 실측으로 유도했다: `project = (src.x/100, src.z/100, -src.y/100)`.
`lv_ocn_eventis_mhp_pool01_sm`의 원본 `(7505.0, 98432.0, 1992.0)`가 프로젝트 설치본의
`(75.05, 19.92, -984.32)`와 일치하는 것으로 검산했다.

### 2.4 1차의 아레나 중심이 틀렸다

1차는 아레나 중심을 NPC 570942 군집인 `(64.55, -982.97)`로 잡았다. 실제 무대 중심은
`(75.07, -984.31)`이고 두 점은 **10.6 m 떨어져 있다.** 1차가 "중심 8 m 안에 울타리 5개와
선베드 2개뿐"이라고 한 것은 잘못된 중심 기준이다.

## 3. 7개 검색 위치별 결과

### 위치 1 — Landscape 페인트 레이어: **조사하지 않음 (불필요해짐)**

아레나 지면이 지형 텍스처일 가능성을 보려던 것인데, 2.2~2.3에서 무대 바닥이 **명시적인
StaticMesh 18조각**으로 확인됐으므로 이 가설은 필요 없어졌다. `extract_ue3_landscape.py`를
돌리지 않았다. LAND01의 `landscapecomponent` 46개 대 설치 16개 차이는 그대로 남아 있고
이것은 G04 범위다.

### 위치 2 — 삭제된 10개 ID의 타 컬럼 재검색: **없다 (전수 확인)**

두 단계로 확인했다.

1. 10개 테이블(`LookInfoSet`, `ReplacementLookInfo`, `NpcSignal`, `NpcAiStateAction`,
   `ContentsTriggerSignal`, `PropertyOriginNotice`, `PropReplace`, `PropMinorProperty`,
   `PropJoint`, `NpcStat`)의 **모든 컬럼**을 값으로 검색 → `NpcStat.PrimaryKey` 10건 외 **0건**.
2. **DB 794개 794테이블 전수**를 40.3초에 SQL로 검색(`scan_all_dbs.py`) → 링 Prop 5개
   (570947/570992/571055/571056/571057)는 `EFTable_NpcStat::NpcStat::PrimaryKey`에**만** 존재.
   대조군 570941/570911/570942는 `EFTable_Npc::Npc::PrimaryKey`에도 있다.

즉 이 5개의 **정의 행(Model 포함)은 배포 테이블 집합에서 실제로 삭제됐다.** 한 테이블만 보고
내린 판단이 아니라 794개 전수 결과다.

### 위치 3 — InterpActor / FracturedStaticMesh / Matinee: **찾았다**

- `fracturedstaticmesh`는 세 레벨 어디에도 **0개**. 붕괴는 파편 메시 방식이 아니다.
- SL01: `interpactor` 12, `efseqact_matinee` 1, `interpdata`/`interpgroup`/`interptracktoggle` 각 1.
  SL01 Matinee는 그룹 이름이 **`fx`**이고 Toggle 트랙 하나(키 3개: `0.0`, `2.4001383781433105`,
  `4.387824058532715`, 전부 `etta_trigger`)뿐이며 `seqevent_levelloaded`에서 시작해 `bLooping=true`다.
  **상시 FX 트리거이지 무대 장치가 아니다.**
- SL01 interpactor 12개 실체: `pool04_sm` 7개(3개씩 0.84 m 간격 수직 적층 2세트 + 1),
  `lv_module_water02_512` 3개, `bg_att_parched_bigpipe01f_sm_psy` 2개. PS의 1개는 `sky_mirror_sm`.
- `efactormotionrotationcyclic` 12개와 `efactormotionlocationcycle` 53개는 `efmotionstaticmeshactor`
  59개에 붙은 **이미 설치된 자체 모션 장식**이다(`fMotionRange` 1.0, `fMotionCycle` 5/10초,
  크리스마스 풍선·해변 장식). 무대 회전이 아니다.
- **진짜 Matinee와 이동 트랙은 `scene03b`에 있다**(2.2절).

### 위치 4 — `SceneEvent` payload: **찾았다**

`TriggerMapData.loa`에 `CEFSeqTN_Action_SceneEvent` 노드가 **41개** 있고 payload는 object path가
아니라 **이름 문자열**이다. 회수한 이름:

`mode_start`, `warter_show`, `warter_hide`, `warter_ground_show`, `ground_destroy_shake`,
`ground_destroy`, `ground_destroy_Repair`, `mococo_cam`, `cam_Repair`, `TA_GroundDestroy`,
그리고 안내용 `S_Systems_Global.SYS_ANNOUNCE_CHAOSGATE_FAIL1` / `ST_ANNOUNCE_CHAOSGATE`.

이 이름들을 **DB 794개 전수 검색**했고 **0건**이다. 즉 SceneEvent 정의는 테이블에 없다.
대신 `_PS` package Kismet에서 **같은 이름의 RemoteEvent**를 찾았다.

| PS export | class | eventname |
|---|---|---|
| 315 / 102 | `seqevent_remoteevent` / `efseqact_endremoteevent` | `warter_ground_show` |
| 316 / 103 | 〃 | `warter_ground_hide` |
| 317 / 104 | 〃 | `warter_show` |
| 318 / 105 | 〃 | `warter_hide` |

이들이 `seqact_multilevelstreaming`으로 서브레벨을 스트리밍한다:
`_1/_2/_3 → lv_ocn_eventis_mhp_scene02b`, `_5/_6 → lv_ocn_eventis_mhp_scene04a`,
`_0 → soundstream + music`. **RemoteEvent 번호와 streaming 노드의 정확한 1:1 결선은
outputLinks 그래프를 덤프에서 제외했으므로 아직 확정하지 않았다.**

### 위치 5 — `ChangePropProperty` trailing 정수: **어휘는 확정, 의미는 미확정**

세 유닛의 노드 꼬리 값을 전수 집계했다.

| 노드 | 꼬리 | 등장 |
|---|---|---|
| `ChangePropProperty` | `(0, 1, V, 0)`, V ∈ {0,1,2,3,4,5} | destroy 12, repair 32, welcome 15 |
| `ChangePropState` | `(0, S)`, S ∈ {0, 2} | destroy 2, repair 6, welcome 1 |
| `DespawnProp` | `(0,0,0)`, `(0,0,0,0)`, `(0,0,1)` | welcome 3 |

V는 0~5의 작은 열거값, S는 0 또는 2다. **어떤 속성인지는 클래스 레이아웃 없이는 이름 붙일 수
없어 확정하지 않았다.** 중요한 것은 **모델 교체가 한 건도 없다는 점**이다. 원본 붕괴는 이미
스폰된 개체의 property/state 서수만 바꾼다.

1차가 보지 않은 **`welcome_unit01`**을 이번에 확인했고 거기에만 `DespawnProp`이 있다.

### 위치 6 — `TA_GroundDestroy @0x11212`: **찾았다 (모델 아님)**

`0x1119B`의 `CEFSeqTN_Action_SceneEvent` 노드 payload 문자열이다. 앞에
`CEFSeqTN_Action_Delay`가 있고 뒤 `0x11307`에서 다음 `CEFSequenceTriggerPack`이 시작한다.
즉 **씬 이벤트 이름**이지 메시나 파편 참조가 아니다. 위치 4와 같은 갈래로 수렴한다.

### 위치 7 — 57009/57010/57011 ID 대역 대조: **찾았다**

`ints_0x30_0x68[13]` 기준으로 세 존을 대조했다.

| 존 | 레코드 | 570900~571100 대역 |
|---|---|---|
| 57009 (섬) | 1,773 | NPC 2종, Prop 54종 |
| 57010 (레이싱) | 310 | NPC 9종, Prop 26종 |
| 57011 (아레나) | 148 | NPC 3종(570911/570941/570942), Prop 10종 |

삭제된 10개 ID는 **57011에만** 배치된다. 배치 수는 570947 **38개**, 571057 7, 571055 6,
571056 6, 570992 4, 나머지 1개씩이다. 이 중 **570987·570997·570998만 57009(섬)에도** 1개씩 있고
570947·570953·570992·570999·571055·571056·571057은 **아레나 전용**이다.

## 4. 1차 판단의 정정

| 1차 서술 | 이번 실측 |
|---|---|
| 무대 메시가 추출된 배치 3,823행에 없다 → 원본 확인 불가 | 맞다. 다만 원인은 **서브레벨 11개 미추출**이다. 무대는 `scene03b`에 있다 |
| 아레나 중심 `(64.55, -982.97)` | NPC 군집 중심이다. 무대 중심은 `(75.07, -984.31)`로 10.6 m 떨어져 있다 |
| 회전곡선 0종 | SL01의 `efactormotionrotationcyclic` 12개는 장식 흔들림이 맞다. 그러나 `scene03b`에 **`interptrackmove` 87개와 Matinee 6개**가 있다 |
| 57011의 "Prop ID 필드"가 `ints_0x30_0x68[13]` | 레코드 자체의 `propId`는 **1**이고 `propName`은 `tip.name.prop_1`, `propModel`은 빈 문자열이다. index 13 값은 **NPC 테이블 ID**이며 `EFTable_NpcStat`에서만 해석된다. 570947은 HP 242,160 / `StatScaleKey=NormalDUN_NAMED_III_1`로 **때려 부수는 개체**다 |

## 5. G09 승인 판정

승인 사슬
`zone → actor/Prop ID → 원본 object path → mesh/material → pivot/transform → signal 수신 대상 →
intact/rotate/destroy/repair 상태`

### 5.1 통과 — 무대 바닥 18조각 (G09 후보 **있음**)

| 사슬 칸 | 값 |
|---|---|
| zone | 서브레벨 `lv_ocn_eventis_mhp_scene03b` (`A89UVJO9YX8XOUPN9JHI29ONJXOX7SC.upk`, ver 868) |
| actor ID | export 105 등 `interpactor_*` 18개 |
| 원본 object path | `bg_ocn_etc_g.mesh.bg_ocn_etc_floor01a_sm_lnh` ×9, `...floor01b_sm_lnh` ×9 |
| material | `floor01a`는 `lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01a_mi_lnh`, `floor01b`는 메시 기본 |
| pivot/transform | 중심 `(75.07, -984.31)`, 반경 6.160~6.215 m, Y 22.40, yaw 20° 18분할 |
| signal 수신 대상 | 같은 서브레벨의 Matinee 6개 + `interptrackmove` 87 + RemoteEvent 5 |
| intact 상태 | `hiddenGame` 0개 → 기본 표시 |

**모델·재질·배치·초기 가시성까지 원본 값으로 확보했다.** 이것은 G09로 보낼 수 있다.

### 5.2 미통과 — 회전 곡선과 붕괴 상태 모델

- **회전/이동 키 값 미확보.** `interptrackmove` 87개가 존재하지만 `PosTrack`/`EulerTrack`이
  tagged property로 직렬화되지 않아(87/87 모두 없음) 현재 리더로 키를 읽지 못했다.
  **트랙이 있다는 사실만 확정했고 각속도·경로는 확정하지 않았다.** 임의 회전값을 만들지 않았다.
- **`scene03b`의 RemoteEvent 5개는 `eventname`이 직렬화되지 않았다**(archetype 기본값).
  어떤 신호가 어느 Matinee를 켜는지 아직 확정하지 못했다.
- **붕괴 상태 모델 없음.** 링 개체 5종(570947 등)의 정의 행이 794개 DB 전수에서 삭제 확인됐고,
  트리거는 property/state 서수만 바꾸므로 intact/destroyed 모델 쌍이 원본에 없다.

원본 미확정 항목을 임의 원통·분홍 재질·임의 각속도로 대체하지 않았다.

## 6. 상태 구분

- **[확인한 원본]** 2·3·4·5절 전부. 서브레벨 14개와 물리 UPK 12개 해석, `scene03b`의 무대
  18조각 메시·재질·기하, SceneEvent 41개 이름, RemoteEvent 4개와 스트리밍 연결,
  DB 794개 전수 검색 결과, 세 존 ID 대역.
- **[실제 구현]** 없음.
- **[live 설치]** 없음.
- **[게시]** 없음. 어떤 publisher도 실행하지 않았다.
- **[빌드/자동검사]** **빌드 안 했다.** G08이 빌드 잠금을 쓰고 있어 MSBuild를 일절 실행하지
  않았다. 실행한 것은 내 python 스크립트 7개뿐이고 전부 exit 0이다. 새 단위검사는 만들지 않았다.
- **[사용자 수동확인]** 해당 없음. 화면에 나타난 변경이 없다.
- **[미완료와 다음 조사 위치]** 7절.

## 7. 미해결과 다음 조사 위치

1. **`interptrackmove` 87개의 키 값.** `PosTrack`/`EulerTrack`이 tagged property로 안 나온다.
   UE3 `InterpCurveVector`/`InterpCurveFloat`는 native 직렬화라 `parse_tagged_properties`가
   못 읽는다. `scene03b` export 중 `interptrackmove`의 serial을 native 구조로 직접 파싱해야 한다.
2. **`scene03b` RemoteEvent 5개의 `eventname`.** archetype 기본값이라 export에 없다.
   archetype 참조를 따라가거나 같은 class의 PS 쪽 직렬화 패턴과 대조해야 한다.
3. **RemoteEvent ↔ `seqact_multilevelstreaming` 1:1 결선.** 내 덤프가 `outputlinks`를
   의도적으로 제외했다. 다시 덤프하면 확정된다.
4. **나머지 미추출 서브레벨 9개의 내용.** `scene07a`(export 1,156, 링 근처 29),
   `scene04a`(616), `scene02a`(432), `scene06a`(493), `sl02`(export **9,881**, 메시 50종),
   `standard_track`(4,262) 등이 아직 프로젝트에 없다. **모코모코 어트랙션과 레인보우
   익스프레스가 여기 있을 가능성이 높다**(`standard_track`이 레이싱 트랙 이름이다).
5. **링 개체 5종의 모델.** 테이블에서는 794개 전수로 없음이 확정됐다. 남은 가능성은
   `scene03b` 외 서브레벨의 SkeletalMesh나 다른 리전 빌드다. 현재 근거로는 복원 불가다.
6. **위치 5의 property 서수 V(0~5)·state S(0/2) 의미.** 클래스 레이아웃이 필요하다.
7. **LAND01 landscape 46 대 설치 16.** G04 범위.

## 8. 다음 G를 시작해도 되는 근거

G02는 **더 이상 막혀 있지 않다.** 무대 바닥은 G09 후보로 보낼 수 있다(5.1). 다만 회전과
붕괴 연출은 7절 1·2·5가 풀려야 한다.

가장 큰 후속 영향은 G02 밖에 있다. **마하라카 추출 자체가 서브레벨 3/14로 불완전하다.**
이것은 G04(재질·RNM), G05(조명·상시 particle), 그리고 사용자가 보고한 "워터팡 아레나와
레인보우 익스프레스 물이 안 나온다, 모코모코 어트랙션이 없다"와 직접 연결된다.
다음 우선순위는 미추출 서브레벨 11개를 기존 `Publish-MapAuthoring` 경로로 들여오는 것이다.

## 최종 재확인

보고 전에 다시 확인한 결과다.

1. **정본 미변경** — `git status --short -- Data/Rendering Client/Bin/DataFiles/Rendering
   Engine/Private/Renderer.cpp Engine/Public/Renderer.h Client/Private/UI_Sprite.cpp` **빈 출력**.
   내가 만든 파일 24개는 전부 `out/.../G02_round2/` 아래다.
2. **다른 작업과의 구분** — 21:01 이후 수정된 추적 파일 6개(`Npc.cpp`, `Npc.h`,
   `NpcActionEffectCueDocument.*`, `extract_source_map_component_lighting.py`와 그 테스트)는
   병렬 작업의 것이며 내가 열지도 쓰지도 않았다.
3. **빌드 미실행** — MSBuild/`Invoke-BuildAndRegression.ps1`을 한 번도 부르지 않았다.
   종료 시점에 MSBuild·cl·fxc·link·devenv·Client·Server·umodel 프로세스 **0개**.
4. **읽기 전용** — 모든 SQLite 연결 문자열에 `?mode=ro`가 있다. `install_terrain.py`,
   `audit_props.py`를 실행하지 않았고 기존 `out/MaharakaReaudit20260926/*`를 덮지 않았다.
5. **좌표 매핑 검산** — `project = (src.x/100, src.z/100, -src.y/100)`를 `pool01_sm`의
   원본 `(7505.0, 98432.0, 1992.0)` ↔ 설치본 `(75.05, 19.92, -984.32)`로 확인했다.
   이 매핑 위에서 무대 반경·높이를 계산했다.
6. **18조각 수치 재확인** — `scene03b-full.json`에서 floor01 조각이 정확히 18개이고
   반경 6.160~6.215, Y 전부 22.40, 인접 yaw 간격 19.69~20.48임을 다시 읽었다.
   interpactor 60개의 메시 내역 합계가 35+9+9+6+1 = 60으로 맞는다.
7. **미확정을 확정으로 적지 않았다** — 회전 키 값, `scene03b` RemoteEvent 이름,
   RemoteEvent↔streaming 결선, property 서수 의미, 링 개체 모델을 전부 7절 미해결로 남겼다.
   임의 모델·회전값·재질을 만들지 않았다.
8. **"없다"의 근거 범위를 적었다** — 링 개체 모델과 SceneEvent 정의의 부재는 DB **794개
   794테이블 전수** 검색 결과이며, 그래도 "원본 삭제"라고 단정하지 않고 남은 후보 위치를 적었다.
