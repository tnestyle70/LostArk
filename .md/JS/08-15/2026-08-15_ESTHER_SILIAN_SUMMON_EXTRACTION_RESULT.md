# 2026-08-15 에스더 실리안 소환 연출 리소스 추출 RESULT

작성자: JS · 2026-08-15

PLAN: `2026-08-15_ESTHER_SILIAN_SUMMON_EXTRACTION_PLAN.md`

## 1. 구현 완료

- 원본 체인 실측 완료: EpicSkill(에스더) → CommonAction 53200 → 소환 NPC 59030
  (`모르둠_실리안`, 모델 `NP_LRSA_01-3`, 스킬 542630/542640, 디스폰 542620,
  애니 그룹 `NP_LRSA_00`). 상세는 PLAN §1~§2.
- `NP_LRSA_00.loa` 파싱으로 스킬별 클립·이펙트·사운드 참조 확정(PLAN §2 표).
  에스더 발동 연출은 542600(연합군): 텔레포트 등장 → `Att_Battle_7_01` 대검 일격.
- 추출: `NP_LRSA_00`(114 obj), `NP_LRSA_01`(110 obj), `WP_NP_LRSA_00`(54 obj, 검)
  → `_export_esther_silian_g` 스테이징, 전부 exit 0.
- `build_npc_animset.py`에 `--clips` 필터 추가(요청 클립 미존재 시 fail). 사용자
  결정대로 184클립 전체 대신 **`att_battle_7_01`, `idle_battle_1` 2클립만** 쿠킹.
- `cook_npc.py` 재질 해석 확장 2건:
  - realpbr 마스터의 `texture_basecolor` → diffuse 별칭 (실리안 갑옷 5슬롯 해소)
  - pc eye 마스터의 `texture_diffuse_base` → diffuse 별칭, 실제 `texture_diffuse`가
    있으면 그쪽 우선 (`setdefault`)
- `build_npc.py` 무기 소켓 인식 확장: `b_wp*` 외에 `b_weapon*` 본 허용 —
  실리안의 검이 `r_weapon` 소켓(`b_weapon_00`)에 부착됨.
- 산출물 (로컬 Resources 배치 완료, `Data/Actors/NpcCatalog.json` 68번째 entry 등록):
  - `Character/NPC/Npc_59030/Npc_59030.wmodel` — 9서브메시(검 포함), 47,085버텍스,
    본 104, 텍스처 포함 72.8MB
  - `Character/NPC/AnimSets/NP_LRSA_00/NP_LRSA_00.wmodel` — 클립 `npc_att_battle_7_01`,
    `npc_idle_battle_1`, 0.74MB
  - catalog: `archetypeId=NPC_59030`, `idleClip=npc_idle_battle_1`

## 2. 자동 검증 (실행함)

- `validate_wmodel` 바디·애니셋 모두 RESULT: OK
- `Compare-Skeletons` ALIGNED (104/104), `Compare-InverseBind` IDENTICAL (maxDelta 0)
- `NpcCatalog.json` parse OK (68 entries), `Publish-WorldGameplay.ps1 -Mode Validate`
  전 Area 통과 (BERN 7 / VALTAN 106 / TRAINING 4 / CS 5, spawn group 3+2)

## 3. 수동 검증

- Debug MapTool NPC 콤보에서 `NPC_59030` 배치 → 실리안 갑주+검 형상과 클립 2종
  (`npc_idle_battle_1`, `npc_att_battle_7_01`) 표출 — **사용자 확인 완료 (2026-08-15)**.

## 4. 알려진 편차와 후속

- **`-3` 재질 오버라이드 미적용**: LookInfo의 `NP_LRSA_01A~D_MI`/`NP_LRSA_01-3_MI`/
  `WP_NP_LRSA_00_MI` MaterialInstanceConstant repaint는 cook_npc.py가 아직 적용하지
  않는다(08-13 배치와 같은 한계). 현재는 기본 재질로 구웠으므로 에스더 발광 틴트가
  아닌 기본 갑주 색이다. 슬롯 대응 규칙 검증 후 후속.
- **이펙트는 추출하지 않음(사용자 결정)**: 이펙트 담당 인계 정본은 PLAN §2 표 —
  upk `FX_ESTHER_LRSA_00`(텔레포트/검 발광/검격/폭발), `FX_NPC_H_00`(일반기 swing),
  공용 `FX_Post`/`FX_CM_02`/`fx_cm_01`. 사운드는 Wwise `S_Mob_Silian1.*`.
- 런타임 소환 연출(발탄 아레나에서 트리거 → 스폰 → `npc_att_battle_7_01` 재생 →
  디스폰)은 Server/Client 계약 설계가 필요한 별도 수직 슬라이스 — 다음 PLAN.
- 스킬 3종 notify 타임라인의 `Data/Animation/Reference` 문서화(PLAN §3-5)는 런타임
  슬라이스에서 타이밍이 필요해지는 시점에 추출한다.
- 웨이·바훈투르는 같은 절차 재적용 대상.

원본은 Smilegate 저작물 — 스테이징(`_export_esther_silian_g`, `_cook_esther_silian`)과
FBX 중간 산출물은 Git에 올리지 않는다.
