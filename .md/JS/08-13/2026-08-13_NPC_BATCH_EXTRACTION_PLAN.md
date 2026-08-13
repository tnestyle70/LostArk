# 2026-08-13 NPC 일괄 추출·쿠킹 확장 PLAN

작성자: JS · 2026-08-13

08-12 NPC 배치 툴 RESULT의 "다음 단계 — 추출 범위 결정" 후속. 상호작용 NPC
(ShopNpc/NpcFriendship/ItemAssemblyNpc/NpcInteractionFunction 조인) 중 인간형 `NP_` 193종을
LookInfo로 해석한 결과를 근거로, 사용자가 "새 체형까지 확장" 범위를 선택했다.

## 1. 범위

| 체형 애니셋 | 후보 | 이번 쿠킹 | 오버라이드 보류 |
|---|---:|---:|---:|
| HM_MA01 (기존) | 32 | 22 | 10 |
| HM_FE04 (기존) | 2 | 2 | 0 |
| HM_MA02 (신규) | 31 | 22 | 9 |
| HM_FE03 (신규) | 28 | 17 | 11 |
| 계 | 93 | **63** | 30 |

- `cook_npc.py`가 LookInfo `MaterialInstanceConstant` 오버라이드(공용 머리 재도색)를 아직
  적용하지 못하므로 오버라이드 사용 30종은 이번 범위에서 제외한다(잘못된 얼굴 텍스처로
  구울 바에 fail-closed). 이제 실물 샘플이 30종 생겼으니 슬롯 대응 검증 후 후속 작업.
- 자체 rig 42종, 기타 소수 체형(MA03/04/05, FE01/02/05 등)도 후속.

## 2. 명명 규약

- 이름 키가 전부 `npc_townsfolk_<PK>`라 영문 변별력이 없어 stable ID는 `NPC_<PK>`.
- 폴더/모델: `Character/NPC/Npc_<PK>/Npc_<PK>.wmodel`
- 애니셋: `Character/NPC/AnimSets/HM_MA02/HM_MA02.wmodel`, `HM_FE03` 동일.
- 한글 이름 ↔ PK 대응표는 RESULT에 부록으로 남긴다(MapTool 콤보는 archetypeId 표시).

## 3. 절차 (기존 파이프라인 그대로, 신규 코드 없음)

1. `umodel_lostark_v7 -groups`로 패키지 95개 → `_export_npc_batch_g` 스테이징.
2. `npc_lookinfo.py` 93종 일괄 해석 → `npc_batch.json`, 경고 0 확인.
3. 신규 애니셋 2종: `build_npc_animset.py`(캐리어 삼각형·아마추어명 `npc` 규약) → 쿠킹 →
   `validate_wmodel` → 해당 체형 바디와 skeletonHash 일치 → `npc_idle_normal_1` 클립 존재 확인.
4. NPC 63종: `build_npc.py --no-anim --no-graft` → `cook_npc.py`(그룹 2단 추적·텍스처 충돌
   처리 내장) → validate → 애니셋과 skeletonHash 대조.
5. 산출물을 `Client/Bin/Resources/Character/NPC/`에 배치, `NpcCatalog.json`에 63 entry 추가
   (formatVersion 2 유지, `idleClip=npc_idle_normal_1`).
6. 검증: JSON parse, `Publish-WorldGameplay.ps1` 전 Area, Client 실행으로 MapTool 콤보·미리보기
   확인(사용자), RESULT 작성.

## 4. 실패 처리

- 해석 경고·쿠킹 오류·validate 실패·해시 불일치 NPC는 개별 제외하고 RESULT에 사유 기록.
  일부 실패가 배치 전체를 막지 않는다.
- 원본은 Smilegate 저작물 — 스테이징·중간 산출물은 Git에 올리지 않는다.
