# 2026-08-20 에스더 웨이·바훈투르 슬롯 활성화 PLAN

작성자: JS · branch `feature/esther-cutin`
선행: `../08-15/2026-08-15_ESTHER_SILIAN_SUMMON_EXTRACTION_RESULT.md`(§4 "같은 절차 재적용"),
`2026-08-20_ESTHER_CUTIN_SCREEN_MODEL_RESULT.md`

## 목표

에스더 슬롯 2(Ctrl+X, 웨이)와 슬롯 3(Ctrl+C, 바훈투르)을 실리안과 같은
추출→쿠킹→카탈로그→Server roster 계약으로 활성화한다. 컷인은 `actionClips` 기반이라
데이터 등록만으로 함께 붙는다.

## 원본 실측 (2026-08-20, 언팩 LPK)

| 항목 | 웨이 (슬롯 2) | 바훈투르 (슬롯 3) |
|---|---|---|
| EpicSkill 1010 | Action2 `53201`, `EstherSkill.Waye` | Action3 `53202`, `EstherSkill.Bahunturr` |
| 소환 NPC | `58700`(에기르_웨이) | `59060`(모르둠_바훈투르) |
| 모델 | `EFDLChar_NP_DPWI_00.NP_DPWI_00` | `EFDLChar_MN_YOBR_00-1.MN_YOBR_00-1`(-1은 MI 재질 변형) |
| 소환 스킬 블록 | 연합군 541000 / 증원 541010 / 퇴장 541020 / 일반 541030·541040 | 542800 / 542810 / 542820 / 542830·542840 |
| 연합군 클립 | `SK_Dochul` (213f@30 = 7.10s) | `Att_Battle_9` → psa `att_battle_09_01` (151f@30 = 5.03s) |
| idle | `idle_battle_1` (46f) | 순수 idle 없음 → `sk_esthereffect_1`(131f, 에스더 발동 자세)로 대체 |
| 무기 | 없음(맨손) | `wp_mn_yobr_00.Mesh.WP_MN_YOBR_00_SK` 별도 패키지 |
| 애니 psa | `NP_DPWI_00\ani\np_dpwi_00_ani.psa` (83트랙) | `MN_YOBR_00\ani\mn_yobr_00_ani.psa` (102트랙) |

실리안(3200ms=94f 반올림)과 같은 기준으로 strike 단계 길이를 클립 길이로 반올림:
웨이 7100ms, 바훈투르 5100ms.

## 계약

| 계층 | 계약 |
|---|---|
| 추출/쿠킹 | `umodel -groups` → `_export_esther_wb_g` 스테이징(NP_DPWI_00, MN_YOBR_00, WP_MN_YOBR_00) → `npc_lookinfo` → `build_npc --no-anim --no-graft` + `build_npc_animset --clips 2종` → `cook_npc`/converter → validate + Compare-Skeletons/InverseBind |
| Resources | `Character/NPC/Npc_58700`, `Npc_59060`, `AnimSets/NP_DPWI_00`, `AnimSets/MN_YOBR_00` |
| Data | `NpcCatalog.json`에 2 entry(+actionClips `esther.strike` → `npc_sk_dochul` / `npc_att_battle_09_01`). `EstherUI.json` 슬롯 2·3 잠금 레이어 제거 |
| Server | `ESTHER_ROSTER[]`(slot→archetype+strikeMs)로 슬롯 하드코딩 대체, `Try_Consume`이 strikeMs 출력, `SERVER_WORLD_ENTITY.iEstherStrikeMs`로 슬롯별 strike 단계 길이. 전역 `ESTHER_STRIKE_MS` 제거 |
| Client | Valtan 로더 선로드 3 archetype 확장, READY 라벨 `Ctrl+Z/X/C`. 입력(Ctrl+X/C→슬롯2/3)과 컷인은 기존 경로 그대로 |
| 검증 | contract test: 슬롯2/3 소모(archetype+strikeMs+게이지 0), roster 밖 슬롯 거부 유지. Server/Client 빌드, JSON parse, `Publish-WorldGameplay -Mode Validate`, 육안(사용자) |

## 결정

1. 웨이 strike 클립은 연합군 `SK_Dochul`(등장 돌출+연타 추정, 7.1s) — 원작 연합군
   액션이 참조하는 클립 그대로. 실제 화면 확인 후 부적합하면 교체.
2. 바훈투르 idle은 `sk_esthereffect_1` 대체(appear 0.8s/leave 1.5s 구간만 노출).
3. `-1`/기본 LookInfo의 MaterialInstance 재도색은 실리안과 같은 한계로 미적용(기본
   재질 쿠킹) — cook_npc 로그로 확인, RESULT에 편차 기록.
