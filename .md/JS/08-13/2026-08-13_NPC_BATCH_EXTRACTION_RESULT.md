# 2026-08-13 NPC 일괄 추출·쿠킹 확장 RESULT

작성자: JS · 2026-08-13

PLAN: `2026-08-13_NPC_BATCH_EXTRACTION_PLAN.md`

## 1. 구현 완료

- 패키지 95개 `umodel_lostark_v7 -groups` 재추출 (`_export_npc_batch_g`, 95/95 exit 0).
- `npc_lookinfo.py` 93종 일괄 해석 — 미해석·경고 0.
- 신규 공유 애니셋 2종 쿠킹·배포:
  - `Character/NPC/AnimSets/HM_MA02/HM_MA02.wmodel` — 146클립, 74.0MB
  - `Character/NPC/AnimSets/HM_FE03/HM_FE03.wmodel` — 51클립, 39.0MB
  - 대상 NPC들이 실제 참조하는 psa만 포함(무기 트랙 `wp_*` 제외). 그룹 간 동명 클립은
    빌더 규약대로 later-wins (MA02 123건 / FE03 62건 — com 셋이 그룹마다 반복 선언되는
    구조라 예상 범위). 두 셋 모두 `npc_idle_normal_1` 보유.
  - 기존 HM_MA01/HM_FE04 애니셋은 재빌드하지 않았다(기존 배치 클립 안정성 유지).
- NPC 63종 mesh-only 빌드(`--no-anim --no-graft`)·쿠킹·배포 — 63/63 exit 0,
  `validate_wmodel` 전부 통과, 총 252.1MB(텍스처 포함, wmodel 자체는 개당 ~0.5MB).
  텍스처 파일명 충돌 6건은 `cook_npc.py` 규약대로 그룹 접미사로 분리 처리됨.
- skeletonHash 정합: 63종 전부 소속 애니셋과 `Compare-Skeletons` ALIGNED(127본),
  `Compare-InverseBind` IDENTICAL(maxDelta 0). 기존 MA01/FE04 셋과도 일치.
- `Data/Actors/NpcCatalog.json` — formatVersion 2 유지, 63 entry 추가(총 67).
  `archetypeId=NPC_<PK>`, `idleClip=npc_idle_normal_1`.

코드 변경 없음(카탈로그 v2가 이미 animationSetId 지원). vcxproj/filters 변경 없음.

## 2. 자동 검증 (실행함)

- `NpcCatalog.json` JSON parse OK (67 entries).
- `Publish-WorldGameplay.ps1` 전 Area 통과 (BERN 7 / VALTAN 12 / TRAINING 4 /
  CHARACTER_SELECT 5 placements, spawn group 3+2).
- 애니셋·NPC 전 산출물 `validate_wmodel` OK, 본 대조 위 §1.

## 3. 수동 검증

- MapTool NPC kind 콤보 표출과 신규 체형 배치·표시 확인 — 사용자 확인 완료 (2026-08-13).
- 미실행: placement Save → publish → Server 재시작 → 제품 snapshot 경로 표시 확인.
  실제 Area에 신규 NPC를 배치·저장하는 시점에 함께 확인한다.

## 4. 이번 범위에서 제외 (후속)

- **재질 오버라이드 사용 30종** — LookInfo가 공용 머리를 `MaterialInstanceConstant`로
  재도색하는 케이스. `cook_npc.py`가 아직 적용하지 못해 fail-closed로 제외했다.
  이제 실물 샘플 30종이 확보됐으므로 슬롯 대응 규칙 검증 후 지원 추가.
- 자체 rig 42종, 소수 체형(MA03/04/05, FE01/02/05, NP_* 개인 rig) — 필요 시 같은 절차.
- NPC 개인 패키지 psa(예: `NP_ASSL_00\ani`)는 공유 애니셋에 넣지 않았다. 해당 NPC의
  전용 idle이 필요해지면 per-NPC 결정 필요.
- MA01 추가 그룹(ani_0645/1400)을 참조하는 신규 NPC들은 해당 클립 없이
  `npc_idle_normal_1`로 동작한다(기존 셋 미재빌드 결정의 트레이드오프).

## 5. 부록 — archetypeId ↔ 이름 ↔ 체형 (63종)

| archetypeId | 이름 | 체형 | 머리 |
|---|---|---|---|
| NPC_11592 | 헬리 | HM_FE03 | MN_HEAD_FE03_012 |
| NPC_11831 | 헬라 | HM_FE03 | MN_HEAD_FE03_012 |
| NPC_12025 | 헬렌 | HM_FE03 | MN_HEAD_FE03_012 |
| NPC_12312 | 클리카 | HM_FE03 | MN_HEAD_FE03_004 |
| NPC_12905 | 리아네 | HM_FE03 | MN_HEAD_FE03_019 |
| NPC_16773 | 헬레나 | HM_FE03 | MN_HEAD_FE03_012 |
| NPC_19244 | 루이지 | HM_FE03 | MN_HEAD_FE03_006 |
| NPC_19368 | 브로콜리 | HM_FE03 | MN_HEAD_FE03_012 |
| NPC_20630 | 소진 | HM_FE03 | MN_HEAD_FE03_010 |
| NPC_25016 | 스텔리아 | HM_FE03 | MN_HEAD_FE03_012 |
| NPC_44935 | 리마 | HM_FE03 | MN_HEAD_FE03_025 |
| NPC_44936 | 티아고 | HM_FE03 | MN_HEAD_FE03_004 |
| NPC_50738 | 서머스 | HM_FE03 | MN_HEAD_FE03_021 |
| NPC_50739 | 라머 | HM_FE03 | MN_HEAD_FE03_019 |
| NPC_80021 | 에린 | HM_FE03 | MN_HEAD_FE03_008 |
| NPC_82090 | 귀족부인 파벨린 | HM_FE03 | MN_HEAD_FE03_027 |
| NPC_83307 | 제니퍼 | HM_FE03 | MN_HEAD_FE03_023 |
| NPC_11749 | 빌마 | HM_FE04 | MN_HEAD_FE04_001 |
| NPC_18394 | 소피 | HM_FE04 | MN_HEAD_FE04_002 |
| NPC_11748 | 빌헬름 | HM_MA01 | MN_HEAD_MA01_011 |
| NPC_12249 | 니켈 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_19545 | 암거래 상인 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_19571 | 앨드리지 | HM_MA01 | MN_HEAD_MA01_027 |
| NPC_19741 | 섬광의 연마사 에버포스 | HM_MA01 | MN_HEAD_MA01_004 |
| NPC_19815 | 샌더스 | HM_MA01 | MN_HEAD_MA01_010 |
| NPC_44929 | 브렌 | HM_MA01 | MN_HEAD_MA01_027 |
| NPC_44931 | 아베도 | HM_MA01 | MN_HEAD_MA01_004 |
| NPC_50576 | 토르테로 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_50944 | 리타르 | HM_MA01 | MN_HEAD_MA01_010 |
| NPC_71751 | 다파라 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80002 | 벤 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80003 | 버트 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80013 | 제프리 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80014 | 녹스 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80015 | 루카스 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80016 | 맥 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80017 | 말론 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80018 | 모리스 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80019 | 올리버 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80020 | 피터 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_80059 | 레이니 | HM_MA01 | MN_HEAD_MA01_012 |
| NPC_11752 | 아르민 | HM_MA02 | MN_HEAD_MA02_008 |
| NPC_11816 | 에두르 | HM_MA02 | MN_HEAD_MA02_042 |
| NPC_12250 | 이튼 | HM_MA02 | MN_HEAD_MA02_052 |
| NPC_12911 | 제딘 | HM_MA02 | MN_HEAD_MA02_037 |
| NPC_12913 | 젠트 | HM_MA02 | MN_HEAD_MA02_042 |
| NPC_12917 | 카로스 | HM_MA02 | MN_HEAD_MA03_015 |
| NPC_13203 | 샤인 | HM_MA02 | MN_HEAD_MA02_044 |
| NPC_19249 | 마도프 | HM_MA02 | MN_HEAD_MA02_038 |
| NPC_19364 | 볼타 | HM_MA02 | MN_HEAD_MA02_050 |
| NPC_19377 | 수집가 테리온 | HM_MA02 | MN_HEAD_MA02_039 |
| NPC_19546 | 암거래 상인 | HM_MA02 | MN_HEAD_MA02_026 |
| NPC_19645 | 여행자 헤로도트 | HM_MA02 | MN_HEAD_MA02_052 |
| NPC_25008 | 케실리 | HM_MA02 | MN_HEAD_MA02_041 |
| NPC_43998 | 에반 | HM_MA02 | MN_HEAD_MA02_052 |
| NPC_44908 | 가렌토 | HM_MA02 | MN_HEAD_MA02_051 |
| NPC_44923 | 로다르 | HM_MA02 | MN_HEAD_MA02_052 |
| NPC_50460 | 그랑카 | HM_MA02 | MN_HEAD_MA02_034 |
| NPC_50583 | 욕심쟁이 스티브 | HM_MA02 | MN_HEAD_MA02_014 |
| NPC_50589 | 데니스 | HM_MA02 | MN_HEAD_MA02_001 |
| NPC_50593 | 알리치오 | HM_MA02 | MN_HEAD_MA02_044 |
| NPC_50780 | 데니스 | HM_MA02 | MN_HEAD_MA02_050 |
| NPC_62014 | 관찰자 앤더슨 | HM_MA02 | MN_HEAD_MA02_045 |

원본은 Smilegate 저작물 — 스테이징(`_export_npc_batch_g`)과 FBX 중간 산출물은 Git에
올리지 않는다. runtime Resources 배포본은 팀장 관리 물리 폴더 규약을 따른다.
