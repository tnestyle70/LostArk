# 2026-09-26 마하라카 G02 — 워터팡 무대 정체성 조사 RESULT

설계서 `2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md`의 G02만 수행했다.
조사와 연결표까지이며 정본 `Data/**`, `Client/Bin/Resources`, 게시본, 코드를 **하나도 바꾸지 않았다.**

결론을 먼저 적는다. **G09로 보낼 수 있는 무대 조각은 0개다.** 승인 조건의 사슬
`zone → actor/Prop ID → 원본 object path → mesh/material → pivot/transform → signal 수신 대상 →
intact/rotate/destroy/repair 상태` 에서 마지막 `intake/destroyed 모델 쌍`이 23개 ID 전부에서 끊긴다.
원본이 없다고 단정하지는 않는다. 어디까지 찾았는지 아래에 적었다.

## 1. 이번 G에서 실제 바꾼 것

코드·데이터·리소스·게시본 변경 **없음**. 아래 파일만 새로 만들었다.

| 파일 | 내용 |
|---|---|
| `out/MaharakaContinuation_20260926_193455/G02/g02_schema_and_join.py` | DB schema 덤프 + 57011 배치 ID 양쪽 테이블 조인 |
| `.../G02/g02_small_tables.py` | PropReplace/PropMinorProperty/PropJoint 전량 덤프, 삭제 ID 타 테이블 조회 |
| `.../G02/g02_trigger_probe.py` | TriggerMapData.loa 문자열·클래스 오프셋 추출 |
| `.../G02/g02_trigger_nodes.py` | 붕괴/복구 유닛의 ChangeProp* 노드 payload 추출 |
| `.../G02/g02_resolve_targets.py` | 대상 핸들 → 배치 레코드 해석 |
| `.../G02/g02_linkage_table.py` | 최종 연결표 생성 |
| `.../G02/db_schema.json` | 11개 DB의 실제 table/column/rowcount |
| `.../G02/placements_57011_joined.json` | 배치 ID별 Npc/Prop 조인 결과 |
| `.../G02/small_tables.json` | 작은 테이블 전량 + 삭제 ID 조회 결과 |
| `.../G02/trigger_strings_57011.txt` | 1,724개 ANSI FString과 오프셋 |
| `.../G02/trigger_prop_nodes_57011.json` | ChangeProp* 노드별 payload |
| `.../G02/rim_targets_resolved.json` | 붕괴/복구 대상 전체 해석 |
| `.../G02/rim_targets.log`, `linkage_table.log` | 위 실행 출력 |
| `.../G02/linkage_table_57011.json` | 최종 연결표 |

기존 사용자 변경 보존: `git status --short`의 161개 항목과 75 tracked 파일을 건드리지 않았다.
`out/MaharakaReaudit20260926/*`의 기존 결과도 덮지 않았다. `audit_props.py`는 **읽기만** 했고
재실행하지 않았다(그 스크립트는 `prop-table-{zone}.json`을 다시 쓴다). `install_terrain.py`도
실행하지 않았다. 모든 SQLite 연결은 `mode=ro`다.

## 2. 원본 근거

| 원본 | 경로 | 식별자 |
|---|---|---|
| 아레나 트리거 | `C:/LostArkExtract/MaharakaFunctions20260926/mapdata/Common_Extra/MapData/57011/TriggerMapData.loa` | 74,848 bytes, SHA-256 `fff384630cc9540a2326526f33f57630df823c039db6c25b785b185beb9ae116` |
| 아레나 배치 | `out/MaharakaFunctions20260926/deploy_57011.json` | 148 레코드 (NPC 22, Prop 118, TrackMove 7, PortalPoint 1) |
| 복호화 테이블 | `.../db/EFGame_Extra/ClientData/TableData` | 796개. 이번에 연 것 11개 |
| 레벨 배치 staging | `C:/LostArkExtract/LV_OCN_EVENTIS_MHP_20260919/Placements/` | SL01 3,823행, SHA-256 `3212d9e3fe4fbfd4a53eb266c3c3bfa4192162db6d750a529903b4770b89c1bf` |
| 추출 제외 기록 | 같은 staging의 `admission.exclusions.json` | 아래 4절 |
| 무대 controller Action | `out/MaharakaReaudit20260926/IstmAction/MN_ISTM_00.action-effects.json` | 265 actions |
| 프로젝트 설치본 | `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/*.mapassets/.mapplacements` | 406 에셋, 4,651 배치 |

읽은 테이블의 실제 schema는 `db_schema.json`에 있다. 요약:
`Prop`(29,060행/153열), `Npc`(38,586행/215열), `NpcStat`, `PropReplace`(28행),
`PropMinorProperty`(13행), `PropJoint`(12행), `NpcAiStateAction`(12,706행), `NpcSignal`(414행),
`LookInfoSet`(2,641행), `ReplacementLookInfo`(341행), `ContentsTriggerSignal`(332행),
`PropertyOriginNotice`(343행).

## 3. 정정한 이전 가정

| 이전 서술 | 실측 결과 |
|---|---|
| 300004는 모델 없는 충돌 Prop | **Prop 테이블 행에만 Model이 비어 있다.** 같은 ID의 `EFTable_Npc.Model`은 `EFDLChar_MN_KZDW_02-1.MN_KZDW_02-1`이다. 57011의 "Prop" 레코드는 NPC 테이블 ID를 들기 때문에 Npc 쪽이 맞는 모델 출처다 |
| 118 Prop 중 39개만 정의가 있고 79개는 미발견 | 양쪽 테이블에 조인하면 **배치 수 기준** Npc+Prop 37, Npc만 34, Prop만 8, 양쪽 없음 69다. ID 기준으로는 23개 중 11개가 정의를 갖는다 |
| 정의 없는 ID는 원본 리소스 삭제 가능 | 10개(570947/570953/570987/570992/570997/570998/570999/571055/571056/571057)는 **`EFTable_NpcStat`에 그대로 있다.** HP 242,160~387,089, `StatScaleKey=NormalDUN_NAMED_III_1`, BalanceLevel이 ID 끝자리와 일치. 즉 삭제된 것은 `Npc`/`Prop` **정의 행**이고 전투 능력치는 남았다 |
| 570942 20개가 "링 모양"으로 서 있다 | 실측 중심 `(64.55, -982.97)`, 반경 **0.27~4.33 m(평균 2.19)**, Y 19.84~20.94. 원형 링이 아니라 5 m 남짓의 **선형 군집**이다 |
| `MN_ISTM_00.Action.loa` 265 actions가 무대 controller 내용 | 참조 particle 46종 중 자기 것은 `FX_MN_ISTM_00.Par_L_ISTM_Thunder_01`과 `Par_L_ISTM_Thunder_Area_01` **2종(각 6회)뿐**이다. 나머지는 Kazeroth3·Albion·Abrellshude·PPAB·CUAS·YoneNeria·MoguroBoss·PTPK·RRMT·LPDA·PAVR 등 다른 보스 것이다. sound 17종도 전부 다른 몬스터다. `MN_Empty_00_SK`를 쓰는 **공용 controller rig**이며 마하라카 전용 무대 데이터가 아니다 |
| 워터팡 무대는 분홍·보라 줄무늬 원형 무대와 노란 원판 | 이 서술의 출처는 `2026-09-26_MAHARAKA_SOURCE_FUNCTIONS_REPORT.md` 25행이 밝힌 **사용자 첨부 홍보 조감도**(`스크린샷 2026-09-25 192953.png`)다. 원본 데이터 근거가 아니다. 설계서 G02-02 7항의 "홍보 이미지 유사성으로 확정하지 않는다"에 해당한다 |

## 4. 붕괴·복구가 실제로 무엇을 건드리는가 (새 사실)

`TriggerMapData.loa`의 두 유닛을 바이너리에서 직접 읽었다.

- **붕괴 유닛 #04** `@0x2D19..0x3B72`, 조건 `Condition_CooperationQuest`,
  신호 `ground_destroy_shake`(`@0x317E`)·`ground_destroy`(`@0x32EA`),
  안내 `tip.desc.se_announce_50` = "경기장 외곽이 곧 무너집니다".
  노드: `ChangePropState` 2개, `ChangePropProperty` 12개. 설계서/이전 요약의 개수와 일치한다.
- **복구 유닛 #09** `@0x6E68..0x8DA2`, 신호 `ground_destroy_Repair`,
  `ChangePropProperty` 33개, `ChangePropState` 6개.
- `TA_GroundDestroy` 문자열은 `@0x11212`에 1회 있고 위 두 유닛 밖이다.

**대상 지정 방식을 확정했다.** 각 노드 payload는
`[2, seq, 1, 4, 7632207, 1, hasNext, nextSeq, param, 0, 1, 0, COUNT, target…]` 형태이고
target 값은 `0x10000000 | DeployData 레코드 index`다. `hasNext`가 0인 노드는 레이아웃이 한 칸
밀리므로, "값 N 뒤에 정확히 N개의 핸들이 오는 위치"로 COUNT를 찾아야 한다. 이 규칙을 적용하면
핸들이 아닌 값이 0개가 된다(`rim_targets.log`에 `핸들 아님` 0건).

해석 결과:

- 붕괴 유닛이 건드리는 **고유 배치 44개**
- 복구 유닛이 건드리는 **고유 배치 64개**

대상의 정체는 무대 파편이 아니라 **관객·장식 캐릭터와 충돌 Prop**이다. 모델이 해석되는 것들:

| ID | 배치 수 | 모델 | 출처 |
|---|---|---|---|
| 300004 | 25 | `EFDLChar_MN_KZDW_02-1.MN_KZDW_02-1` | `EFTable_Npc.Model` |
| 300001 | 5 | `EFDLChar_MN_KZDM_01-1.MN_KZDM_01-1` | `EFTable_Npc.Model` |
| 300006 | 7 | `EFDLChar_MN_0005_02-3.MN_0005_02-3` (Npc) / `EFDLProp_ITR_00280.ITR_00280` (Prop) | 양쪽 |
| 15020 | 6 | `EFDLChar_PR_GSTSA_04-1.PR_GSTSA_04-1` | `EFTable_Npc.Model` |
| 15030 | 6 | `EFDLChar_NP_SHSVM_00-5.NP_SHSVM_00-5` | `EFTable_Npc.Model` |
| 3000 | 2 | `EFDLProp_ITR_10131.ITR_10131` (공용 텔레포트 프랍) | `EFTable_Prop.Model` |
| 570941 | 1 | `EFDLChar_MN_ISMP_00.MN_ISMP_00` ModelSize 170 | `EFTable_Npc.Model` |
| 570911 | 1 | `EFDLChar_MN_ISMP_00-1.MN_ISMP_00-1` ModelSize 50 | `EFTable_Npc.Model` |
| 570942 | 20 | `EFDLChar_MN_ISTM_00.MN_ISTM_00` → `MN_Empty_00_SK` | `EFTable_Npc.Model` + LookInfo |

`PropReplace`(28행)·`PropMinorProperty`(13행)·`PropJoint`(12행)을 전량 확인했고 **아레나 ID는
한 건도 없다.** 즉 붕괴/복구는 소품 교체 테이블이 아니라 트리거가 배치 핸들에 직접
property/state를 바꾸는 방식이다. 따라서 **intact/destroyed 모델 쌍이 원본 테이블에 없다.**

570941(ModelSize 170)과 570911(ModelSize 50)은 아레나 존이 가진 **자체 모코모코/워터캐논 배우**다.
설계서 G00-02가 경고한 대로 부모 존 57009의 `actor100`/`actor188`과 중복되므로 합치지 않아야 한다.

## 5. 무대 메시를 어디까지 찾았고 무엇이 없는가

추출 제외 기록(`admission.exclusions.json`)의 실제 내용:

- 규칙: `island scope = source world y > 50000 cm` (레이싱 트랙은 y 0~35000이라 제외)
- `LV_OCN_EVENTIS_MHP_SL01`: kept **3,820**, excluded **3**
- `LV_OCN_EVENTIS_MHP_LAND01`: kept 2, excluded 80 (섬 범위 밖)
- `LV_OCN_EVENTIS_MHP_PS`: kept 1(`lv_matte.mesh.sky_mirror_sm`), excluded 243

SL01에서 제외된 3개를 직접 확인했다. 전부 같은 에셋
`bg_tot_movillage_a.mesh.bg_tot_movillage_decoprop07f_sm_artree`(토토이크 마을 장식 프롭)이고
제외 이유는 "source glTF normal/tangent basis가 평행/퇴화"다. 위치는 프로젝트 좌표로
`(52.7, 20.5, -1020.8)`, `(97.9, 20.5, -1022.5)`, `(43.9, 20.5, -1008.6)`이다. **무대가 아니다.**

프로젝트 설치본 대조:

- 아레나 중심 `(64.55, -982.97)` **18 m** 안에 배치 203개. 전부 울타리·수풀·화분·집·선베드·장식이다.
- 같은 중심 **8 m** 안에는 7개뿐: `LV_ATT_STERN_FENCE01` 4개(Y 19.84), `LV_ATT_STERN_FENCE04` 1개,
  `BG_OCN_COMMON_SUNBED` 2개(Y 20.48).
- 406 에셋 카탈로그의 맵 전용 메시는 `LV_OCN_EVENTIS_MHP_` 접두 8종뿐이다:
  `POOL01`(1배치, 워터캐논 자리 `(75.05,19.92,-984.32)`), `POOL02`(1), `POOL03`(4),
  `POOL04`(**81**, 세로로 얇은 scale `0.3/0.9/0.3`·`0.066/0.32/0.066`),
  `WATER02`/`WATER03`, `FLOOR01`(1, `(75.05,31.21,-990.58)`), `FLOOR02`(2, 같은 XZ Y 31.0/31.21),
  `TENT01`(10), `TENT02`(6), `DECOBANNER01`(1).
- 배치의 논리 패키지는 `SL01` 3,820 / `LAND01` 830 / `PS` 1 셋뿐이다.

**따라서 원형 무대/노란 원판에 해당하는 별도 StaticMesh는 추출된 2021 레벨 배치
3,823행 안에 없다.** `FLOOR01`/`FLOOR02`는 Y 31 m로 아레나 지면(Y 19.8~20.5)보다 11 m 높고
XZ가 `(75.05, -990.58)`이어서 모코모코 어트랙션 쪽 구조물로 보인다(이건 **추론**이다).

이 사실을 "원본 삭제"로 단정하지 않는다. 아직 확인하지 않은 곳을 8절에 적었다.

## 6. 연결표와 G09 승인 판정

`linkage_table_57011.json`이 23개 (kind,ID) 전부에 대해 사슬 6칸을 판정한다.
`linkage_table.log`가 사람이 읽는 형태다.

| 사슬 칸 | 통과 | 실패 |
|---|---|---|
| `zone` | 23/23 | — |
| `idDefinition` (Npc 또는 Prop 행 존재) | 11 | 12 |
| `objectPath` (모델 문자열 확보) | 9 | 14 |
| `pivotTransform` (DeployData 위치·yaw) | 23/23 | — |
| `signalTarget` (붕괴/복구 대상) | 17 | 6 |
| `stateModel` (intact/destroyed 모델 쌍) | **0** | **23** |

**G09 후보: 없음.** 모델과 신호 대상이 모두 확보된 ID(300004, 300001, 300006, 15020, 15030, 3000,
570941)조차 "무너진 상태의 모델/조각"이 원본 어디에도 없다. 이들은 무대 파편이 아니라 관객·장식이며,
원본 붕괴는 이들의 property/state만 바꾼다. 원통이나 Valtan 격자 파편으로 대체하지 않았고
회전 각속도도 만들지 않았다.

모델은 확보했지만 회전/붕괴 곡선이 없는 경우를 따로 적는다.
**모델 확보 9종 / 회전곡선 0종 / 붕괴 대체 모델 0종.**

## 7. 상태 구분

- **[확인한 원본]** 위 2·4·5절 전부. 트리거 두 유닛의 노드 구조와 대상 핸들 규칙, 배치 ID의
  모델 출처, 추출 제외 3건의 정체, 아레나 중심의 실제 지오메트리.
- **[실제 구현]** 없음. 이 G는 조사 범위다.
- **[live 설치]** 없음.
- **[게시]** 없음.
- **[빌드/자동검사]** 빌드 **안 했다**(Visual Studio pid 30616이 열려 있어 금지됨).
  실행한 것은 python 조사 스크립트 6개뿐이고 전부 exit 0이다. 새 단위검사는 만들지 않았다.
- **[사용자 수동확인]** 해당 없음. 화면 판정 대상이 아니다.
- **[미완료와 다음 조사 위치]** 8절.

## 8. 미해결과 다음 조사 위치

무대 메시 후보가 아직 나올 수 있는 곳을 이름으로 남긴다. 검색 완료를 주장하지 않는다.

1. **Landscape 페인트 레이어**: 아레나 지면이 별도 메시가 아니라 **지형 텍스처**일 가능성.
   아레나 중심 `(64.55, -982.97)`을 덮는 컴포넌트의 layer02~07 페인트 가중치를 확인해야 한다.
   입력은 이미 있다: `Tools/LandscapeExtractor/extract_ue3_landscape.py`,
   설치된 16 컴포넌트, `ReleasePC/Packages/867STHM7WV6VMSNL7HFG07M83MO5C.upk`. 이건 G04 범위와 겹친다.
2. **삭제된 10개 ID의 LookInfo**: `EFTable_ReplacementLookInfo`(341행)·`LookInfoSet`(2,641행)의
   PrimaryKey 조회에서는 안 나왔다. 두 테이블의 **다른 열**(Secondary/Group 키)로 다시 찾아야 한다.
   `EFTable_NpcSignal`(414행)과 `NpcAiStateAction`(12,706행)에서 570947/571055~57 등을
   행 값으로 검색하는 것도 남았다.
3. **원본 레벨 UPK 직접 조사**: staging은 SL01/LAND01/PS 3개 논리 패키지만 담았다.
   `archive_index.json`은 경로가 해시(예: `9XUFAXIP8BXBAP1NIEG66EF.upk`)여서 이름 검색이 0건이다.
   `ReleasePC` 안에서 57011이 참조하는 물리 패키지를 hash 대조로 열거해야 한다.
   InterpActor/FracturedStaticMesh/Matinee 트랙은 아직 한 건도 확인하지 않았다.
4. **`SceneEvent` payload**: 붕괴 유닛의 `SceneEvent` 3개와 복구 유닛 3개의 내용을 아직 안 읽었다.
   `S_Systems_Global.SYS_ANNOUNCE_CHAOSGATE_FAIL1` 같은 문자열만 확인했다.
   여기에 무대 연출 참조가 있을 수 있다.
5. **`ChangePropProperty`의 property 종류**: 노드 trailing 정수(`0,1,3,0` / `0,1,2,0` 등)가
   어떤 속성·값인지 아직 해석하지 않았다. `EFTable_PropertyOriginNotice`(343행)와 대조가 남았다.
6. **`TA_GroundDestroy`** `@0x11212`가 속한 유닛을 아직 분석하지 않았다.
7. **57010(레이싱)·57009(섬)의 같은 ID 대역**: 570947 등이 부모 섬에도 있는지 대조하면
   정체(관객/병사/무대)를 좁힐 수 있다.

## 9. 다음 G를 시작해도 되는 근거

G02는 **막혔다**. 무대 모델 정체가 미확정이므로 설계서 지시대로 G09(무대 연결)는 시작하지 않는다.
그러나 G03~G08은 이 결과에 의존하지 않는다. 특히 G01(action 4225601)·G06(particle)·G07(sound)은
모코모코/캐논 쪽이고 이번 조사에서 그 두 배우의 모델·ModelSize가 재확인됐다
(570941 `MN_ISMP_00` 170%, 570911 `MN_ISMP_00-1` 50%). G03/G04(렌더링·지형)도 독립이며,
8절 1항이 G04와 직접 연결된다.

## 최종 재확인

보고 전에 다시 확인한 결과다.

1. **정본 미변경 확인** — `git status --short`로 `Data/`, `Client/`, `Server/`, `Tools/`에
   이번 G02 때문에 생긴 변경이 없음을 확인했다. 새로 생긴 것은
   `out/MaharakaContinuation_20260926_193455/G02/` 13개 파일과 이 RESULT 문서뿐이다.
2. **핸들 파싱 정확성** — `rim_targets.log`에서 `핸들 아님` 발생 0건. 초기 버전은 `hasNext=0`
   노드에서 count를 한 칸 잘못 읽어 `count=268435575`를 출력했고, 이를 고친 뒤 재실행했다.
   고치기 전 집계를 그대로 쓰지 않았다.
3. **노드 개수 대조** — 붕괴 유닛의 `ChangePropProperty` 12개, `ChangePropState` 2개,
   복구 유닛의 33개/6개가 기존 `trigger_units_57011_summary.txt`의 개수와 일치한다.
4. **모델 출처 재확인** — 300004의 Prop 행에는 Model이 비었고 Npc 행에는
   `EFDLChar_MN_KZDW_02-1.MN_KZDW_02-1`이 있다. 두 테이블을 각각 조회해 확인했다.
5. **G09 후보 0개** — `linkage_table_57011.json`에서 `g09Eligible=true`인 항목이 없음을 확인했다.
   임의 모델·원통·회전값을 채우지 않았다.
6. **빌드 미실행** — Visual Studio가 열려 있어 어떤 MSBuild도 실행하지 않았다. 내가 띄운
   느린 python 프로세스 1개(pid 41516)만 PID로 종료했고 다른 프로세스는 건드리지 않았다.
7. **읽기 전용 준수** — 모든 SQLite 연결 문자열에 `?mode=ro`가 있다.
   `audit_props.py`·`install_terrain.py`는 실행하지 않았다.

미완료를 완료로 적지 않았다. 무대 모델은 **찾지 못했고**, 없다고 단정하지도 않았다.
