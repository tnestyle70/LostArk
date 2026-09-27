# 2026-09-26 마하라카 G01 — action 4225601 원본 참조 사슬 RESULT

범위는 설계서 `2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md`의 **G01만**이다.
조사와 연결표 작성까지 수행했고 정본 `Data/**`·Resources·게시물은 **하나도 바꾸지 않았다.**

실행 저장소 `C:\Users\USER\source\졸업팀폴\LostArk`, branch `codex/main-ship-maharaka-0926`,
HEAD `a84bbcd45ff995c70bf847c683f1d159cf1821a6`. Visual Studio가 열려 있어(pid 30616) 빌드는 돌리지 않았다.

## 1. 이번 G에서 실제 바꾼 파일

정본 코드·데이터 변경 **없음**. 생성물은 전부 gitignore 대상인 새 후보 폴더
`out/MaharakaContinuation_20260926_193455/G01/`에만 있다.

| 파일 | 역할 |
|---|---|
| `MokoAction4225601/MN_ISMP_00.action-effects.json` | action 4225601 단독 추출 결과 |
| `MokoAction4225601/action-effect-source-manifest.json` | 추출 manifest |
| `action4225601.linkage.json` | **G01 연결표 정본 후보**(16 notify 전체 분류) |
| `skilleffect-4225601.json` | 원본 SkillEffect 7행 + SkillDecal 1행 |
| `decode_notify_payload.py` | payload 토큰 디코더(읽기 전용) |
| `list_wmodel_bones_clips.py` | 설치 WModel bone/clip 목록(읽기 전용) |
| `lookup_effect_ids.py`, `lookup_effect_focused.py` | 원본 DB 조회(`mode=ro`) |
| `scan_pawnmaterialparam.py` | 36 action 전체 PawnMaterialParam 대조 |
| `build_linkage_table.py` | 연결표 생성기 |

`Data/**`, `Client/Bin/Resources/**`, `Tools/**`의 dirty 항목은 모두 이전부터 있던 다른 작업의
미커밋 변경이며 내가 건드리지 않았다. `install_terrain.py`를 다시 실행하지 않았다.

## 2. 원본 근거

- 원본 LOA `out/MaharakaReaudit20260926/MN_ISMP_00.Action.loa`, 1,078,539 bytes,
  SHA-256 `251b1c56a9c4f7b4865b66780ef95bd9b96f052bb94307e66c59f22178e4a6a3`.
  설계서 기준값과 **일치**했다.
- action `4225601` `모코코 물벼락_물벼락 쏟아내기`, LOA byte offset `57009`,
  stage 1개(`Main`, offset `57107`), clip 1개, notify 16개.
- 원본 socket: `out/MaharakaReaudit20260926/ISMP_DDS/MN_ISMP_00/SkeletalMesh3/mn_ismp_00_sk.props.txt`
  및 `mn_ismp_00-1_sk.props.txt`의 `Sockets[11]`. 두 파일의 socket 블록은 2,608 byte **완전 동일**.
- 원본 테이블: `C:/LostArkExtract/MaharakaFunctions20260926/db/EFGame_Extra/ClientData/TableData`의
  `EFTable_SkillEffect.db`(213,326행/121열), `EFTable_SkillDecal.db`(173행). 전부 `mode=ro`로 열었다.

## 3. notify 16행 분류

집계: `source-bound` 7, `source-bound-id / unresolved-package` 1, `partially-resolved` 2,
`gameplay-reference` 6. 합 16. 시각·duration은 원본 JSON의 float을 그대로 옮겼다.

| # | type | 원본 시각(초) | duration(초) | 원본 identity | 분류 |
|---|---|---|---|---|---|
| 000 | Anim | 0.0 | 6.0 | clip `Att_Battle_1_01` | source-bound |
| 001 | PawnMaterialParam | 0.0 | 6.0 | 이름 `PetEmotion` / 파라미터 `opacity_intensity` | partially-resolved |
| 002 | PawnMaterialParam | 0.0 | 6.0 | 같은 파라미터, 다른 값/모드 | partially-resolved |
| 003 | PlayDecalEffect | 0.0 | 2.299999952316284 | SkillDecal 1006 → `GR_Mon_Donut_behit_01` | ID 확정 / 패키지 미해결 |
| 004 | AKEvent `Cast1` | 0.0 | 3.1333560943603516 | `MococoWater1_Attack01_Cast1` + 조건부 Water2 | source-bound |
| 005 | PlayParticleEffect `Face` | 0.5915589928627014 | 5.408441066741943 | `FX_MN_ISMP_00.Par_G_ISMP_Face_01` | source-bound |
| 006 | AKEvent `Shot1` | 2.200000047683716 | 3.799999952316284 | `MococoWater1_Attack01_Shot1` + 조건부 Water2 | source-bound |
| 007 | PlayParticleEffect `water` | 2.2925870418548584 | 2.7232859134674072 | `Par_G_ISMP_Attk01_Water_01` | source-bound |
| 008 | Effect | 2.299999952316284 | 0.10000000149011612 | SkillEffect `422560101` | gameplay-reference |
| 009 | Effect | 2.299999952316284 | 0.10000000149011612 | SkillEffect `422560102` | gameplay-reference |
| 010 | Effect | 2.299999952316284 | 0.10000000149011612 | SkillEffect `422560107` | gameplay-reference |
| 011 | Effect | 2.299999952316284 | 0.10000000149011612 | SkillEffect `422560106` | gameplay-reference |
| 012 | Effect | 2.299999952316284 | 0.10000000149011612 | SkillEffect `422560108` | gameplay-reference |
| 013 | Effect | 2.299999952316284 | 0.10000000149011612 | SkillEffect `422560104` | gameplay-reference |
| 014 | PlayParticleEffect `Ground` | 2.390213966369629 | 2.9024178981781006 | `Par_G_ISMP_Attk01_WaterGround_01` | source-bound |
| 015 | PlayParticleEffect (이름 없음) | 5.071868896484375 | **0.0** | `Par_G_ISMP_WaterFinish_01` | source-bound |

particle 4종의 full package는 모두 `FX_MN_ISMP_00`이다.

## 4. 개별 조사 결과

### 4.1 FX_01 — 해결

`FX_01`은 bone이 아니다. 설치 WModel의 bone은 10개
(`mn_ismp_00_sk`, `b_root`, `b_effectworldzero`, `b_ismp_root`, `b_ismp_26`, `b_ismp_24`,
`b_ismp_27`, `b_effectroot`, `b_effectname`, `b_cameratarget`)이고 `FX_01`은 없다.

원본 SkeletalMesh `Sockets[1]`이 정답이다.

```
SocketName       = fx_01
BoneName         = b_ismp_root
RelativeLocation = { X=0, Y=-50, Z=90 }      (cm)
RelativeRotation = { Yaw=0, Pitch=0, Roll=-16384 }
RelativeScale    = { X=1, Y=1, Z=1 }
```

- parent bone `b_ismp_root`는 설치 WModel skeleton에 **존재한다**.
- `Roll=-16384`은 UE FRotator 정수다. `-16384 / 65536 * 360 = -90.0도`. degree로 재해석하지 말 것.
- 원본 mesh는 소문자 `fx_01`, notify는 대문자 `FX_01`이다. **대소문자 무시 비교**가 필요하다.
- 나머지 socket 10개(`interactionkey`, `fx_buff_01`, `fx_down_01`, `fx_mark_01~04`, `fx_hit_01`,
  `fx_state_01`, `fx_state_02`)도 연결표에 기록했다.

### 4.2 얼굴/물/지면/종료의 attachment

| notify | socket | local translation | 비고 |
|---|---|---|---|
| 005 Face | `FX_01` | 0,0,0 | scale 1,1,1 |
| 007 water | `FX_01` | 0,0,0 | Face와 같은 socket |
| 014 Ground | **없음(count 0)** | 한 성분 `-200.0` cm | 얼굴 socket에 붙이면 안 됨 |
| 015 Finish | `FX_01` | 0,0,0 | |

Ground는 socket 문자열이 아예 없어 actor root 기준이며, notify-005와 바이트 정렬했을 때
translation 벡터의 세 번째 성분이 `-200.0` cm다. notify-005의 translation이 전부 0이라
축 지정은 정렬에 의한 판단이다. **X나 Y가 0이 아닌 다른 notify로 축을 한 번 더 확인해야 한다.**

### 4.3 PlayDecalEffect — ID 사슬 해결, 패키지 미해결

notify는 object path가 아니라 **두 개의 테이블 ID**를 싣는다. 그래서 추출기가
`NOTIFY_HAS_NO_EXPLICIT_OBJECT_REFERENCE`로 표시한 것은 정상이다.

- payload offset 91: `1006` → `EFTable_SkillDecal.PrimaryKey=1006`
  `DecalDesc '도넛_몬스터_피격이상'`, `DecalArchetype 'GR_Mon_Donut_behit_01'`,
  `DecalKey 'Donut'`, `DecalLookType 4`, `DecalBlindnessType 1`, `SourceRow 84`, `Milestone OBT.0`
- payload offset 151: `422560100` → `EFTable_SkillEffect.PrimaryKey=422560100 SecondaryKey=1`
  `Key 38`, `AreaType 3`(부채꼴), `AreaRange 700`cm, `AreaRemoveRange 300`cm, `AreaAngle 90`도,
  `AreaHeight 100`, `Target 2`, `HitType 1`, `FreezeTime 325`
- 꼬리 float 5개: `5.0`, `0.10000000149011612`, `0.0`, `2.0`, `0.20000000298023224`

decal duration 2.299999952316284는 6개 gameplay Effect의 발생 시각과 정확히 같다. 즉 t=0부터
타격 순간까지 띄우는 **예고 표시**이고, 내부 3 m~외부 7 m·90° 부채꼴이 `DecalKey "Donut"`과 맞는다.

**미해결**: `GR_Mon_Donut_behit_01` archetype이 사는 패키지와 실제 decal material/texture 경로.

### 4.4 gameplay Effect 6개

전부 `EFTable_SkillEffect.PrimaryKey`이고 payload offset 90에 있다.
`422560101`, `422560102`, `422560107`, `422560106`, `422560108`, `422560104`.
ID는 `actionId × 100 + n` 규칙이다(`4225601 × 100 = 422560100`).

- `422560101`: Key 38, 부채꼴 400cm/90도, `HitType 8`, `ChainIndex 422560108`, `HitTypeTime 500~520`
- `422560102`: Key 38, 700cm/inner 400cm, `HitType 5`, `FallDown`, `ChainIndex 422560108`, `HitTypeTime 1814~1834`
- `422560104`: Key 7, 30도, `AreaOffsetX -15`, `HittedSetSkillKey 'SkHit_Claw1'`, `ValueA 9625`
- `422560106`: Key 7, 90도, `AreaOffsetX -15`, `HittedSetSkillKey 'SkHit_Claw1'`, `ValueA 422560108`
- `422560107`: Key 7, 700cm, `HitType 4`, `FallDown`, Push 200→220, `ValueA 422560105`
- `422560108`: Key 7, 400cm, `HitType 4`, `FallDown`, Push 200→220

`422560107`의 `ValueA`가 가리키는 `422560105`는 notify가 직접 참조하지 않는 연쇄 행이다.
이 6행은 **Server 판정 입력**이며 particle로 만들거나 Client 피해 판정으로 구현하지 않는다.

### 4.5 사운드 조건 — 해결

notify-004/006은 기본 event 하나와 **길이 1의 조건부 override 배열**을 싣는다.

```
기본            : AkEvent 'S_Mob_MococoWater1.MococoWater1_Attack01_Cast1' / ..._Shot1
조건            : pawn DLChar == 'EFDLChar_MN_ISMP_00-1.MN_ISMP_00-1'
조건 일치 시     : AkEvent 'S_Mob_MococoWater1.MococoWater2_Attack01_Cast1' / ..._Shot1
```

설치된 두 NPC가 이 조건에 정확히 대응한다.

| archetype | modelAssetId | 조건 | 재생 |
|---|---|---|---|
| `NPC_MAHARAKA_MOKOMOKO` | `.../MN_ISMP_00/MN_ISMP_00.wmodel` | 불일치 | **Water1** |
| `NPC_MAHARAKA_WATERCANNON` | `.../MN_ISMP_00-1/MN_ISMP_00-1.wmodel` | 일치 | **Water2** |

Water1과 Water2를 동시에 재생하는 구현을 만들면 안 된다. notify-004의 꼬리 float은
`0.6000000238418579`가 두 개다.

### 4.6 WaterFinish duration 0 — 원본에 명시적 0

duration 슬롯은 다른 notify가 양수를 싣는 같은 위치이고, notify-015는 거기에 **명시적으로 0.0**이
들어 있다. 삭제나 임의 1초로 바꾸지 말 것. 이 notify는 material parameter도 함께 싣는다.

- `Color_1` type 3, 값 `[1.0, 1.0, 1.0]`, curve `None`
- `Alpha_1` type 1, 값 `[1.0]`, curve `None`

주의: notify-015는 stage의 마지막이라 payload가 stage/action 경계까지 확장된다. 꼬리의 `6.0`과
`-1`은 notify 필드가 아닐 수 있으므로 notify 값으로 쓰지 말 것.
실제 수명은 particle 자신의 lifetime/stop이 정하므로 **G06에서 `FX_MN_ISMP_00` graph를 추출해 확정**해야 한다.

### 4.7 PawnMaterialParam — 부분 해결, 이름 해석을 정정한다

같은 LOA의 다른 action과 대조해서 **설계서의 전제를 정정한다.** 두 문자열은 `(재질 slot, 파라미터)`가
아니라 `(notify 이름, 파라미터 이름)`이다.

| action | labels |
|---|---|
| 19 DESPAWN | `MDEC` / `FX_Dead_EdgeColor` |
| 19 DESPAWN | `MDWO` / `FX_Dead_WorldOffset` |
| 19 DESPAWN | `MDTT` / `Dead_Texture_Tiling` |
| 13 DIE | `MParam` / `Dead` |
| **4225601** | **`PetEmotion` / `opacity_intensity`** |

payload 배치도 이를 뒷받침한다. particle notify는 `이름`,`그룹("FX")`,`100` 순서이고
PawnMaterialParam은 `PetEmotion`,`100`,... ,`opacity_intensity` 순서다. 즉 `PetEmotion`이 이름,
`opacity_intensity`가 파라미터다.

두 행의 차이는 offset 62의 enum(1 / 2)과 값 블록이다.

- notify-001: `count 2, 0,0, 0,0,0, 2.5, 1.0` — `action 19 DESPAWN`(`MParam`/`Dead`)과 **꼬리 패턴이 동일**
- notify-002: `count 1, 1.0, 0,0,0, int 2`

**미해결**: 값·곡선·대상의 의미. `CEFActionNotify_PawnMaterialParam` 클래스 레이아웃이 확정되지
않았다. 이름만 보고 상수 1로 적으면 안 된다.

### 4.8 설치 clip 대조 — 일치

`att_battle_1_01`이 두 WModel에 모두 있고 `duration_ticks 180.0`, `ticks_per_second 30.0`,
즉 **6.000초**로 원본 clip 길이 `6.0`과 일치한다. `NpcCatalog.json`의 두 archetype `actionClips`에도
`att_battle_1_01`이 등록되어 있다.

### 4.9 particle offset 47 플래그는 attachment가 아니다

`PlayParticleEffect` payload offset 47의 int를 MN_ISMP_00 전체 1,500건에서 socket 유무와 교차했다.

| flag | socket 있음 | 건수 |
|---|---|---|
| 0 | 없음 | 107 |
| 0 | 있음 | 1,292 |
| 1 | 없음 | 80 |
| 1 | 있음 | 21 |

네 조합이 모두 나오므로 이 값은 socket 유무와 **독립**이다. attachment 유무로 해석하면 안 된다.
4225601에서는 Face=1, water=0, Ground=0, Finish=1이다. follow 여부로 단정하기 전에 클래스
레이아웃을 확인해야 한다.

## 5. ms 변환 규칙

원본은 초 단위 float이다. 소비자 계약은 다음과 같다.

- `Data/Effects/NpcActionCues/*.npcactioncues.json` formatVersion 1은 `startMs`/`durationMs`를
  정수 ms로 읽는다.
- `Client/Private/NpcActionEffectCueDocument.cpp`의 `ReadUInt`는 `static_cast<std::uint32_t>(raw)`로
  **소수를 절삭**하고 상한이 600000이다.
- `Client/Private/Npc.cpp::Update_ActionEffectCues`는 `fElapsedSeconds += fTimeDelta`로 프레임
  델타를 누적하고 `fDue = iStartMs * 0.001f`로 한 번만 되돌린다. pose clock과 동기화되어 있지 않다.

**위험**: `2.299999952316284 s → 2299.999952316284 ms → 절삭 2299 ms`. 반올림하면 2300 ms다.
변환 규칙(반올림/절삭)을 정하고 정수 검사를 추가하기 전에는 ms를 정본으로 저장하지 않는다.

cm→m 축 규칙은 기존 생성기
`Tools/EffectPipeline/build_esther_ninave_source_effects.py`가 UE `(x,y,z)` cm →
engine `[x/100, z/100, y/100]` m를 쓴다. 이 규칙이면 `fx_01`의 `(0,-50,90)` cm는
`(0.0, 0.9, -0.5)` m가 되지만 **실제 소비 경로는 G06/G08에서 확인해야 한다.**

## 6. 스키마 공백

현재 `lostark.npc-action-effect-cues` formatVersion 1은
`cueId/effectAssetId/clip/startMs/durationMs/bone/followBone`만 읽는다. 다음을 표현할 필드가 없다.

- socket local transform: 위치 `(0,-50,90)` cm, Roll `-90도`
- 조건부 sound 선택(DLChar → Water1/Water2)
- material parameter(`Color_1`, `Alpha_1`, `opacity_intensity`)
- gameplay-reference의 SkillEffect ID

임의 필드를 적어도 실행되지 않는다. version을 올릴 때 reader·validator·소비자·기존 v1 호환 회귀를
같은 변경으로 구현해야 한다.

## 7. 상태 구분

| 단계 | 상태 |
|---|---|
| 추출 완료 | 예 — action 4225601 단독 추출, 16 notify, exit 0 |
| 후보 생성 | 예 — `action4225601.linkage.json` 16행 분류 |
| live 설치 | **아니오** — 정본 `Data/**`·Resources 변경 없음 |
| 게시 | **아니오** — publisher 실행 없음 |
| 빌드 / 자동검사 | **아니오** — VS가 열려 있어 빌드 금지. 새 단위검사도 추가하지 않음 |
| 제품 소비자 연결 | **아니오** — G06/G08 범위 |
| 사용자 수동확인 | **아니오** — 화면에 나타난 것이 없으므로 확인할 대상 자체가 없음 |

## 8. 실행한 명령과 exit code

| 명령 | exit |
|---|---|
| `extract_action_effect_notifies.py --help` | 0 |
| `extract_action_effect_notifies.py --source MN_ISMP_00=... --action-id 4225601 --output .../G01/MokoAction4225601` | 0 |
| `decode_notify_payload.py` (001/002, 003/004, 005/007, 014/015) | 0 |
| `list_wmodel_bones_clips.py` | 0 |
| `lookup_effect_ids.py` | 0 |
| `lookup_effect_focused.py` | 0 |
| `scan_pawnmaterialparam.py` | 0 |
| `build_linkage_table.py` | 0 |

추출기 summary: `selectedNotifyCount 16`, `uniqueParticleSystemCount 4`,
`unsupportedUnresolvedCount 1`, `notifyTypeCounts {AKEvent 2, Anim 1, Effect 6, PawnMaterialParam 2,
PlayDecalEffect 1, PlayParticleEffect 4}`.

## 9. 미해결 입력과 다음 검색 위치

| 미해결 | 확인된 검색 범위 | 다음 검색 위치 |
|---|---|---|
| `GR_Mon_Donut_behit_01` 실제 패키지·material | `EFTable_SkillDecal`에서 ID·archetype 이름까지 확정 | `out/MaharakaReaudit20260926/archive_index.json`에서 `GR_Mon_Donut` 검색, 기존 도넛 decal 복원 경로 대조 |
| `CEFActionNotify_PawnMaterialParam` 값·곡선 의미 | 같은 LOA의 `MParam`/`MDEC`/`MDWO`/`MDTT` 5종과 꼬리 패턴 대조 | UE3 클래스 레이아웃, 또는 이 저장소에서 이미 복원한 캐릭터 material parameter 소비 규칙 |
| offset 47 플래그 의미 | 1,500건 교차표로 "attachment 아님"까지 확정 | 클래스 레이아웃. 또는 follow가 필요한 원본 사례와 비교 |
| Ground translation의 축 | notify-005(전부 0)와 정렬해 3번째 성분 `-200.0` | X/Y가 0이 아닌 다른 PlayParticleEffect notify |
| `Par_G_ISMP_WaterFinish_01`의 실제 수명(duration 0의 의미) | 원본에 명시적 0.0임을 확정 | **G06**: `extract_ue3_particle_graph.py`로 `FX_MN_ISMP_00` 추출 후 emitter lifetime/loop 확인 |
| `422560105`(체인 대상) | `422560107.ValueA`가 가리킴 | `EFTable_SkillEffect`에서 103/105 행 조회 |
| 다른 action의 `FX_Turn01~04` socket | 이 두 mesh의 socket 11개에는 없음 | 4225601 범위 밖. 해당 action 복원 시 별도 확인 |

## 10. 다음 G를 시작해도 되는 근거

G01의 목표(끊김 없는 연결표)는 16행 중 **13행이 원본 identity까지 확정**됐고, 나머지 3행
(PawnMaterialParam 2, decal 패키지 1)은 이름과 다음 검색 위치를 남긴 상태다.

- **G06**(particle 복구)은 시작할 수 있다. 4종 particle의 package/asset/시각/socket/scale이 확정됐고,
  `FX_01 → b_ismp_root + (0,-50,90)cm + Roll -90도`가 확보됐다. duration 0 해석에 필요한 graph 추출이
  G06의 첫 작업이다.
- **G07**(사운드)은 시작할 수 있다. 4개 event full path와 DLChar 조건, 설치 NPC 대응이 확정됐다.
- **G08**(CNpc 연결)은 시작할 수 있으나 **스키마 확장 결정이 선행**해야 한다(6절).
- decal은 패키지가 확정될 때까지 **적용하지 않고 미해결로 보존**한다. 임의 decal로 대신하지 않는다.
- PawnMaterialParam은 값 의미가 확정될 때까지 쓰지 않는다. 빼놓고 "전체 완료"라고 하지 않는다.

## 최종 재확인

주장한 것을 실제 출력으로 다시 확인했다.

- LOA SHA-256을 직접 계산해 설계서 기준값과 문자열 비교 → `True`.
- 추출기 exit 0이고 summary가 notify 16개·particle 4종·unresolved 1개를 보고했다. 파일 2개가
  실제로 생성됐음을 `find`로 확인했다.
- 16행 전부를 payload까지 디코드했다. 시각·duration은 손으로 옮기지 않고 생성기가 추출 JSON에서
  복사하게 했으며, 생성 후 재파싱해 16행임을 확인했다.
- `FX_01`이 bone이 아님을 설치 WModel bone 10개 목록으로 확인했고, socket 정의를 원본 props에서
  직접 읽었다. 두 mesh의 socket 블록이 2,608 byte 동일함을 프로그램으로 비교했다.
- `att_battle_1_01`의 180틱/30fps = 6.000초를 reader로 읽어 원본 6.0과 대조했다.
- 7개 ID가 `EFTable_SkillEffect.PrimaryKey`임을 실제 조회로 확인했고, decal 1006은
  `EFTable_SkillDecal`에서 1행 일치했다. 6개 Effect ID는 payload offset 90에서 다시 추출해 검증했다.
- offset 47 플래그 결론은 1,500건 교차표 숫자에 근거한다. 추측이 아니다.
- **정본 변경이 없음을 `git status --short -- Data Client/Bin/Resources Tools`로 확인했다.**
  나열된 항목은 모두 이전부터 있던 다른 작업의 변경이다. 내 산출물은 gitignore 대상
  `out/MaharakaContinuation_20260926_193455/G01/` 아래 10개 파일뿐이다.
- 빌드를 돌리지 않았고 Client/UI를 실행하지 않았다. visual 판정을 하지 않았다.
- 확정하지 못한 3건(decal 패키지, PawnMaterialParam 의미, 플래그 의미)을 완료로 적지 않았고
  임의 값으로 메우지 않았다.
