# 2026-08-13 발탄 패턴별 원작 클립 바인딩 RESULT

작성자: JS · 2026-08-13 · branch `feature/lancemaster-charge-aim`

선행: `2026-08-13_VALTAN_ORIGINAL_CLIP_EVENT_EXTRACTION_RESULT.md` (애니셋 attach ①단계 후속)

## 1. 구현 완료

- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` 신규
  (`lostark.valtan-pattern-bindings` v1) — 패턴 stage `actionId` → `mesh_*` 원작 클립 112건.
  Reference `Valtan.clipseq`에서 규칙 생성: sourceActionId 오름차순 × 최저 seq 중 필러 클립
  (idle/turn/walk/run/sc/spawn/dead/evt1) 제외 후 stage 수와 일치(FIT)하는 체인 우선,
  없으면 첫 체인의 처음/중간/끝 배분(APPROX). 휘두르기 windup/active(`mesh_att_battle_1_01`)와
  고공점프 recovery(`mesh_att_battle_8_01_end`) 2건 수동 보정.
- `CValtan::Load_PatternBindings` — Initialize에서 문서를 parse→validate(스키마·버전·중복
  actionId)→commit. 실패 시 문서 전체 거부 + 카탈로그 공용 클립 폴백(스폰 비차단, 표현 격리).
- `CValtan::Apply_NetworkState` — 패턴 상태에서 snapshot `strActionId`로 바인딩 클립을 선택.
  같은 entity action 종류의 연속 stage(예: 지진찍기 IMPACT→DELAYED_QUAKE, 둘 다
  PATTERN_ACTIVE)도 actionId 변화로 클립 전환. 비패턴 상태와 미바인딩 actionId는 기존
  카탈로그 클립 유지.

## 2. 방침 변경 — duration 교정은 이번 범위에서 제외

기계 교정 초안 검토 결과, 원작 loa 액션의 스테이지 체인(루프·AI 전이 포함)이 인카운터의
WINDUP/ACTIVE/RECOVERY 3~4단 모델과 1:1이 아니어서 `len=`을 그대로 넣으면 서버 회피
타이밍이 붕괴한다(예: 내려찍기 windup 650ms→3000ms). `ValtanEncounter.json`은 무변경으로
되돌렸고(publish/receipt 불필요), stage duration vs 원작 클립 길이 대조표를 근거로 원작 영상
확인(②)과 묶어 패턴별 저작으로 진행한다.

## 3. 검증

- Client Debug 빌드 오류 0. 바인딩 JSON parse OK(112건, 중복 없음).
- 수동(사용자, 2026-08-13): 실전 발탄이 패턴마다 서로 다른 원작 모션으로 재생됨 확인.
  stage가 클립보다 짧아 모션이 중간에 끊기는 것은 duration 미교정에 따른 예상 상태.

## 4. 남은 것

- stage duration·hit 시각의 패턴별 저작 교정(원작 영상 대조, Balance Tool 경로로 publish).
- APPROX 바인딩 패턴들의 육안 검수(어색하면 JSON 한 줄 수정).
- HIT 원본 좌표 추적, 패턴 hit shape Debug wire 표시(③) — 선행 RESULT의 남은 항목 그대로.
