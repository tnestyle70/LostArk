# 2026-08-15 에스더 실리안 소환 연출 리소스 추출 PLAN

작성자: JS · 2026-08-15

발탄 레이드용 에스더 스킬 연출 작업의 1단계. 사용자 결정: **실리안부터, 소환
연출(3D 캐릭터가 필드에 나타나 스킬 시전)만 먼저**. 화면 최상단 컷인 UI
(`EstherSkill.Silian` Scaleform MC)와 서버 게이지/발동 계약은 이번 범위에서 제외한다.

## 1. 원본 근거 (2026-08-15 실측, 언팩 LPK TableData)

에스더 스킬의 내부명은 **EpicSkill**이다. 실리안 체인:

| 계층 | 정본 | 실측 값 |
|---|---|---|
| 스킬 세트 | `EFTable_EpicSkill.db` | 슬롯1 Action `53200`(실리안) / 슬롯2 `53201`(Waye) / 슬롯3 `53202`(Bahunturr). 컷인 `SkillMC=EstherSkill.Silian`, 음성 `PC_COMMON_ESTHER.*` |
| 발동 액션 | `EFTable_CommonAction.db` PK 53200 | `CommonActionType=20`, Cooltime 5000ms, SecondaryKey=레벨. UseEffect/UseSkillEffect 비어 있음 — 연출은 loa notify 체인 소유 |
| 액션 notify | `data3/.../XmlData/Action/COMMONACTION.loa` | 미해석. `extract_action_loa.py`로 53200 스테이지·notify 추출 예정 |
| 소환 NPC | `EFTable_Npc.db` PK 59017, 59030~59039 | 이름 키 `tip.name.npc_summon_5903x` = "실리안". Comment1 `모르둠_실리안`(59017/59030/59031) / `종막_실리안`(59032~59039) |
| 소환 모델 | Npc.Model | `EFDLChar_NP_LRSA_01-3.NP_LRSA_01-3`, ModelSize 100 |
| 소환 행동 | Npc row | `Skill1=542630`(50%) `Skill2=542640`(50%), 퇴장기 `DestroySkillIndex=542620`, `SummonLifetime=30000ms`, `SummonSpawnAction=1`, `OriginalActionObjectGroupName=NP_LRSA_00` |
| 소환 스킬 | `EFTable_Skill.db` 542620/542630/542640 | MaxRange 150, Cooltime 2000, `SetDirectionToTargetAngle=180`. 타이밍·판정은 `EFTable_SkillEffect.db` 조인 |

주의: 물리 소환 NPC(59030대)는 모르둠·종막 레이드 데이터다. 발탄 시대(EpicSkill
1010/1020, ClassifyIndex 3705100)의 실리안 연출이 별도 시각 체인(거대 검 이펙트,
COMMONACTION.loa notify)일 수 있다. 이번 작업의 목표는 사용자가 지정한 "실리안이
캐릭터처럼 나타나 스킬을 쓰는" 연출이므로 **소환 NPC(NP_LRSA_01-3 + 스킬 542630/542640)
를 기준 연출로 삼고**, COMMONACTION.loa 53200 해석은 캐스트 연출 타이밍 참고로만 쓴다.

## 2. 패키지 실측 (umodel `-list "NP_LRSA*"`)

- `NP_LRSA_01`: `np_lrsa_01_sk`, `np_lrsa_01_sk-1`, `np_lrsa_01_body_sk`, `np_lrsa_01_face_sk`
  (+ ride_horse). **`sk-3` 메시는 목록에 없음** → `-3`은 LookInfo/재질 변형일 가능성.
  `npc_lookinfo.py`로 해석해 실물 슬롯을 확정한다.
- `NP_LRSA_00`: `np_lrsa_00_sk`, **`np_lrsa_00_ani`(+evt1/evt2)** — 소환 NPC의
  `OriginalActionObjectGroupName`이 가리키는 액션 그룹 애니셋.
- 에스더 이펙트 후보: `*ESTHER*` 매칭 46패키지는 에스더 무기 발광(par_x/y_esther_<class>,
  par_y_fif_silian_00~03 등)이라 소환 연출과 무관.
- `NP_LRSA_00.loa` 파싱(2026-08-15)으로 스킬별 클립·이펙트 참조를 확정했다.
  **이펙트는 이번 작업에서 추출하지 않는다(사용자 결정). 이펙트 담당 인계용으로 아래
  위치만 기록한다.**

| 액션(skillId) | 용도 | 클립 | 이펙트/사운드 참조 (원본 위치) |
|---|---|---|---|
| 542600 연합군(에스더 발동 연출) | 텔레포트 등장→대검 일격→폭발 | `Att_Battle_7_01` → `Idle_Battle_1` | upk `FX_ESTHER_LRSA_00`: `Par_S_LRSA_Teleport_01`, `Par_D_LRSA_Sword_01~03_loop`, `Par_D_LRSA_SwordSwing_01`, `Par_D_LRSA_Exp_01/02/03_dust` + 공용 `FX_Post`(FilmNoise), `FX_CM_02.Light`, `fx_cm_01.Distortion`. Wwise `S_Mob_Silian1.Silian1_Attack9_*` |
| 542610 증원 | 캐스트→2단 베기 | `Att_Battle_5_01/02` | `FX_ESTHER_LRSA_00.Par_S_LRSA_Sk_02_*` 계열, `FX_Post` ZoomBlur. Wwise `Silian1_Attack8_*` |
| 542630 스킬1 | 일반 베기 | `Att_Battle_2_01` | `FX_NPC_H_00.Par_H_LRSA_Swing_02_LOC_INT`. Wwise `Silian1_Attack2/3_*` |
| 542640 스킬2 | 강공격 | `Att_Battle_3_01_Start/Loop/End` | `FX_NPC_H_00.Par_H_LRSA_ATK_03_01~03_LOC_INT`, `FX_BS_01.Trail.Par_D_Trail_07_1_LOC_INT`, `FX_CM_02.Light.Par_MP_Light_01`. Wwise `Silian1_Attack4_*` |
| 542620 디스폰 | 퇴장 | `Idle_Battle_1` | DESPAWN(19) 공통 재질 페이드(`FX_Dead_EdgeColor` 등) |

  액션 상태기 정본: `data3/.../XmlData/Action/NP_LRSA_00.loa`. 이펙트 원본 패키지는
  umodel `-game=lostark -kr -nameresolve`로 `FX_ESTHER_LRSA_00`, `FX_NPC_H_00` 지정 추출.

## 3. 절차 (기존 파이프라인 그대로, 신규 코드 없음)

1. `umodel_lostark_v7 -groups`로 `NP_LRSA_00`, `NP_LRSA_01`(변형 포함) →
   `_export_esther_silian_g` 스테이징.
2. `npc_lookinfo.py`로 소환 NPC 59030(`NP_LRSA_01-3`) 해석 — `-3` 변형의 실제
   mesh/재질 슬롯 확정. MaterialInstanceConstant 오버라이드면 08-13 배치와 같은 기준으로
   지원 여부 판단(불가면 기본 `np_lrsa_01_sk`로 굽고 RESULT에 편차 기록).
3. 애니셋: `NP_LRSA_00` 그룹 전체는 마을/스토리 공용 184클립이므로 굽지 않는다
   (사용자 결정). `build_npc_animset.py`에 `--clips` 필터를 추가해 에스더 연출 최소
   클립만 굽는다 — 1차는 `att_battle_7_01`(연합군 일격), `idle_battle_1` 2개.
   산출: `Character/NPC/AnimSets/NP_LRSA_00/NP_LRSA_00.wmodel` → `validate_wmodel` →
   바디와 `Compare-Skeletons` ALIGNED 확인.
4. 모델: `build_npc.py --no-anim --no-graft` → `cook_npc.py` →
   `Character/NPC/Npc_59030/Npc_59030.wmodel` → validate → 애니셋 skeletonHash 대조.
5. 스킬 타이밍: `extract_monster_skilltiming.py`(발탄 08-14와 동일 경로)로 스킬
   542620/542630/542640의 clip↔notify(effectref PK 포함)를 `Data/Animation/Reference/`
   문서로 추출. `extract_action_loa.py`로 COMMONACTION.loa의 53200 캐스트 타이밍도
   같은 위치에 참고 추출.
6. 산출물 배치와 `NpcCatalog.json` 등록(archetypeId `NPC_59030`)은 쿠킹 검증 뒤
   진행하고, 런타임 연출(소환 스폰·연출 재생) 연결은 별도 PLAN으로 나눈다 —
   Client GameObject/Server 계약 설계가 필요하므로 이 문서에 코드로 선반영하지 않는다.

## 4. 완료 기준과 검증

- 스테이징 추출 exit 0, `validate_wmodel` 통과, 바디·애니셋 skeletonHash ALIGNED.
- 소환 연출 필수 클립(등장 액션, 스킬 2종, 퇴장)이 쿠킹된 애니셋에 실존.
- 스킬 3종의 notify 타임라인 문서가 `Data/Animation/Reference/`에 생성되고 effectref
  PK가 SkillEffect 실제 행과 대응.
- 추출·중간 산출물(Smilegate 원저작물)은 Git에 올리지 않는다. runtime 배치는 팀장
  물리 폴더 규약을 따른다.

## 5. 범위 제외 (후속)

- 웨이·바훈투르 소환 연출 (같은 절차 재적용).
- 컷인 UI(`EstherSkill.*` MC → 일러스트 텍스처 추출 → `Data/UI` 계약 재현).
- 서버 에스더 게이지·특수 입력 발동 계약(`Data/Balance` + Shared + Server 수직 슬라이스).
- 소환 이펙트 추출·쿠킹 전체 — 이펙트 담당 몫으로 넘긴다(사용자 결정, 2026-08-15).
  필요한 원본 위치·참조는 §2의 표가 인계 정본이다.
