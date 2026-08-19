# 2026-08-19 넉다운 기상기 RESULT

계획 정본: 같은 폴더 `2026-08-19_KNOCKDOWN_STANDUP_SKILL_PLAN.md`
branch: `feature/player-hit-knockback`

## 0. 결론

KNOCKDOWN 중 SPACE가 4직업 공식 기상기(34030/17025/31030/2050030)를 시전한다. Server가
넉다운 상태에서만 승인하고 즉시 해제하며, ~4m 구르기 root motion과 30초 공식 쿨타임을
적용한다. protocol 범프 없음. **화면 육안 검증은 아직 안 했다.**

## 1. 실제 변경

- Data: `PlayerSkills.json` STANDUP 4행(SPACE, 쿨 30000 = 공식 `EFTable_Skill.Cooltime`,
  duration = 클립 길이 1167/1267/1300/1100ms). `skillbindings.json` 4파일에
  `*_sk_standup_normal_1`(DM `_04`) 바인딩. `<class>.rootmotion.json` 재추출
  (기상기 전진 LM 3.99 / WL 3.83 / Artist 4.13 / DM 4.03 m, 기존 항목 변화 없이 additive).
  receipt +72항목(balance-tool authored override 정책, 공식 Cooltime 근거 note),
  skillDefinitionCount 90→94.
- Shared: `PLAYER_SKILL_KIND::STANDUP = 4` (wire 미직렬화 — protocol v24 유지).
- Publisher: kind 허용 +STANDUP, STANDUP 계약(무데미지·무스테이지·쿨 필수·스탠스 NONE),
  슬롯 중복 검사에서 STANDUP은 별도 도메인(같은 SPACE 키를 상태로 나눠 쓰므로).
- Server: catalog kind 파서 +STANDUP. `Try_Start`: `STANDUP인지 == KNOCKDOWN인지`가 다르면
  거부(기상기는 다운 중 전용, 일반 스킬은 다운 중 거부 유지), 승인 시
  `iKnockdownEndTick/넉백 window` 정리 후 기존 SKILL 시작 경로(쿨·root motion·nav clamp 공유).
- Client: 카탈로그 kind 파싱·STANDUP 슬롯 도메인·검증, `Find_BySlot(..., isKnockedDown)`
  — 넉다운이면 STANDUP 행만, 아니면 STANDUP 행 제외. `CPlayerController::Poll_SkillSlots`가
  `CCombatHUDViewModel`의 복제 `eAction`으로 플래그 전달. 표현은 기존 SKILL edge 경로가
  skillbindings 클립을 그대로 재생(넉다운 step 초기화는 선행 슬라이스에서 구현).

## 2. 실행한 검증

```text
Publish-GameplayBalance -Mode Validate   PASS (136 skills)
extract_rootmotion.py 4직업              additive only (git diff +894/-0)
Server build (pre-build publish)         PASS
Server.exe --contract-test               failures 0
  (신규: STANDUP 로드+root motion, 넉다운 중 기상 승인+상태 해제, 기립 상태 거부)
NetworkProtocolHarness                   failures 0
Client build                             PASS
git diff --check                         PASS
```

## 2.1 육안 회귀와 수정 (2026-08-19)

첫 육안에서 넉백·넘어짐은 정상이나 기상 애니메이션이 안 보였다. 원인은 Server
`Try_Start`의 `iComboStage = (ACTIVE ? 0 : 1)` 규칙에 STANDUP이 걸려 comboStage 1로
복제됐고, Client `Play_Skill`이 comboStage>0을 스테이지형 체인으로 해석해 단일 클립
체인 매칭에 실패(idle 대체)한 것. STANDUP도 0으로 시작하도록 고치고 contract test에
`0u == iComboStage` 단언을 추가했다(failures 0 재확인).

## 2.2 기상 그레이스 (2026-08-19 2차 육안 후)

루가루가 인접에서 ~2초 주기로 계속 때려 기상 즉시 재넉다운되는 체인 스턴이 확인됐다.
원작 기상 무적을 본떠 `PLAYER_HIT_REACTION_GRACE_TICKS = 60`(2초)를 추가: 넉다운이
만료되거나 기상기로 끝난 시점부터 2초간 새 push/넉다운을 arm하지 않는다(데미지는 그대로).
부활·클래스 변경·audition reset에서 초기화. contract test 2건 추가, failures 0.

루가루 공격 모션이 안 보이는 것은 몬스터 표현(타 담당 레인) 이슈로 전달 대상.

## 3. 미검증 / 남은 경계

- 육안: 넉다운 중 SPACE → 구르며 기상, 쿨타임 30초 — 사용자 확인 필요.
- 원작의 기상 무적 프레임, 다운 직후 사용 불가 유예, twistknockdown 변형은 후속.
- HUD에 SPACE 슬롯이 없어 기상기 아이콘 표시는 없음(쿨은 스냅샷 Cooldowns에 실림).
