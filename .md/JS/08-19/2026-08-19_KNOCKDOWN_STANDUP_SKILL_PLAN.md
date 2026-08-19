# 2026-08-19 넉다운 기상기 PLAN

작성자: JS · branch `feature/player-hit-knockback` (`156634c` 위)
선행: `2026-08-19_PLAYER_HIT_KNOCKBACK_KNOCKDOWN_RESULT.md`

## 목표

KNOCKDOWN 중 SPACE를 누르면 기존 이동기 대신 **원작 기상기**를 시전해 즉시 일어나며
구른다. Server가 넉다운 해제·쿨타임·root motion 이동을 권위로 판정한다.

## 원본 실측 (2026-08-19)

- 4직업 공식 기상기: 창술사 34030, 워로드 17025, 도화가 31030("기상기_지그재그"),
  차원술사 2050030. 클립 = `*_sk_standup_normal_1`(DM `_04`).
- `EFTable_Skill`: 4개 모두 `Cooltime 30000` / `CostMp 0`. 클립 길이(animnotify len):
  LM 1167ms, WL 1267ms, Artist 1300ms, DM 1100ms.
- `PLAYER_SKILL_KIND`는 wire 직렬화 없음 → **protocol 범프 불필요**.
- SPACE 이중 바인딩 전례: 창술사 34020/34520이 stance로 해소. Controller는
  `CCombatHUDViewModel.Get_Player()`의 복제 상태(stance)로 이미 분기 — `eAction`도 같은
  뷰모델에 이미 있음.
- root motion은 `buildScript/extract_rootmotion.py`(wmodel+skillbindings → rootmotion.json)로
  재추출. STANDUP은 비스테이지 kind라 ACTIVE와 같은 flat curve 경로.

## 계약

| 계층 | 계약 |
|---|---|
| Data | `PlayerSkills.json`에 4행: `skillKind "STANDUP"`, `inputSlot "SPACE"`, `requiredStance NONE`, cooldown 30000(공식), duration = 클립 길이, cost/damage/hit 0. `skillbindings.json` 4파일에 skillId→standup 클립. `<class>.rootmotion.json` 재추출로 기상 구르기 변위 수록. receipt에 신규 행 field 항목 추가(Balance Tool authored override 정책, 쿨타임은 공식 Cooltime 근거 note), skillDefinitionCount 132→136 |
| Shared | `PLAYER_SKILL_KIND::STANDUP`(COUNTER 뒤, END 앞). wire 미직렬화라 버전 유지 |
| Publisher | skillKind 허용 목록 +STANDUP, 검증은 damageless ACTIVE와 동일 규칙 |
| Server | catalog kind 파서 +STANDUP. `Try_Start`: STANDUP은 **KNOCKDOWN 상태에서만** 승인(다른 상태에선 거부), 승인 시 `iKnockdownEndTick=0`·넉백 window=0 정리 후 기존 SKILL 시작 경로(쿨타임·root motion·nav clamp 공유). 일반 스킬의 KNOCKDOWN 중 거부는 기존 가드 유지 |
| Client | 카탈로그 kind 파싱 +STANDUP, `Find_BySlot(class, slot, stance, isKnockedDown=false)`: 넉다운이면 같은 슬롯의 STANDUP 행 우선, 아니면 STANDUP 행 제외. Controller `Poll_SkillSlots`가 뷰모델 `eAction==KNOCKDOWN`을 전달. 표현은 기존 SKILL edge 경로(skillbindings 클립)가 그대로 처리 — KNOCKDOWN→SKILL 전이 시 knockdown step 초기화는 이미 구현됨 |
| 검증 | contract test: 넉다운 중 기상기 승인+상태 해제+쿨 30000, 서서 누르면 거부, 넉다운 중 일반 스킬 여전히 거부. publisher Validate, rootmotion 재추출 멱등, Client 빌드, 육안(넉다운 중 SPACE→구르며 기상) |

## 결정

1. 기상기 사용 가능 구간 = KNOCKDOWN 액션 중 전체(원작의 다운 직후 무적·타이밍 제한은 후속).
2. 밀려나는 중(넉백 window, 다운 아님)에는 불가 — 원작과 동일.
3. HUD ACTIVE 슬롯 목록에 SPACE가 없어 HUD 변경 없음. 쿨타임은 스냅샷 Cooldowns로 자동 표시.
