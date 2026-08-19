# 2026-08-19 플레이어 피격 넉백·넉다운 RESULT

계획 정본: 같은 폴더 `2026-08-19_PLAYER_HIT_KNOCKBACK_KNOCKDOWN_PLAN.md`
branch: `feature/player-hit-knockback` (main `ef642ae` + 회귀 수정 3건 위)

## 0. 결론

발탄 33개 패턴 중 23개와 루가루·잡몹 공격에 원본 push/넉다운이 붙었다. Server가 피격
변위(당김 포함)와 KNOCKDOWN 상태를 권위로 판정하고, Client는 6직업 fall→land→lying→standup
클립으로 표현만 한다. protocol v24. **화면 육안 검증은 아직 안 했다.**

## 1. 데이터 (추출 근거)

- `Tools/CharacterAnimationIntake/fill_encounter_push.py`(신규): `Valtan.animnotify`의
  HIT PK(`actionId*100+n`) → `EFTable_SkillEffect`의 `PushMinRange(cm, 음수=당김)/
  PushMinTime(ms)/FallDown/HitTypeDownMin(ms)`. 패턴 대표 행 = `|PushMinRange|` 최대,
  동률 최저 PK. PushMinTime 0→150ms, FallDown인데 Down 0→2000ms. 멱등(`--check` changed 0).
- `ValtanEncounter.json` stage에 `pushRangeM/pushMs/knockdown/downMs` 4필드(124 stage 전부,
  비데미지 stage는 0). 대표값: 휘두르기 2.0m/242ms/down2000, 내려찍기 2.75m,
  **감금 사자후 -2.4m 당김**, 카운터(42060601) 4.9m/535ms. receipt의 `patterns[i].stages`
  33건을 같은 값으로 동기화(PROJECT_TUNED 유지).
- `MonsterProfiles.json`: `attackPushRangeM/attackPushMs/attackKnockdown/attackDownMs` —
  잡몹 0.5~0.6m/150ms, **루가루 2.0m/250ms + 넉다운 2000ms**(루가루는 원본 미추출 튜닝값).

## 2. 계층별 변경

| 계층 | 내용 |
|---|---|
| Publisher | GameplayBalance: stage 4필드 검증+PATTERNSTAGE 행 pack, bootstrap **v10**. WorldGameplay: profile 4필드, spawn bootstrap **v3**. Valtan destruction publisher의 stage exact-field 목록에 4필드 추가(한 줄, CY 로직 무관 — encounter 스키마 소비자라 필수) |
| Shared | `PLAYER_ACTION_STATE::KNOCKDOWN`(TRIGGER_MOVE 뒤), protocol **23→24**, snapshot 검증: KNOCKDOWN은 skillId 없음+actionStartTick 필수 |
| Server | `SERVER_PLAYER` 넉백 4필드+`iKnockdownEndTick`. `CPlayerSkillSystem::Arm_PlayerHitReaction`(공용 arm: 방향=소스→플레이어, 음수면 반전, 진행 중 재-arm 금지, TRIGGER_MOVE/DEAD 제외). `CGameRoom::Advance_PlayerKnockback`(walkable clamp+body 충돌, 벽이면 조기 종료)+만료 시 NONE 복귀. arm 지점: `ValtanBrain::ApplyPatternHit`(stage 미러 4필드), `MonsterBrain` PATTERN_ACTIVE. 이동/스킬은 기존 `NONE != eAction` 가드로 자동 거부. 부활/클래스변경/audition reset에서 초기화 |
| Client | `CHARACTER_ANIM` +KNOCKDOWN/KNOCKDOWN_LAND/DOWN_LOOP/STANDUP, 6직업 스펙 클립(DM은 down 루프 없어 land 최종 포즈 유지, 클립 실존은 wmodel 문자열로 확인). `CCharacter` KNOCKDOWN 분기: fall→land→lying 루프, 해제 시 standup 1회→locomotion. Balance Tool stage 파싱·저장·편집 UI 4필드 |
| 검증 코드 | contract test: arm 방향/당김 부호/재-arm 금지/KNOCKDOWN 중 스킬 거부/스윙·사자후 실데이터 로드/profile push 왕복. protocol harness: V24 핀, KNOCKDOWN 왕복·tick 없음 거부·skill 동반 거부 |

## 3. 실행한 검증

```text
Publish-GameplayBalance -Mode Validate      PASS (33 patterns / 124 stages)
Publish-WorldGameplay  -Mode Validate       PASS
fill_encounter_push.py --check              changed 0 (멱등)
Shared+NetworkProtocolHarness build/run     failures 0 (V24)
Server build (pre-build publish v10/v3)     PASS
Server.exe --contract-test                  failures 0
Client build                                PASS
git diff --check                            PASS
```

## 4. 미검증 / 남은 경계

- **화면 육안**: 발탄 휘두르기 피격 넉백, 사자후 당김, 루가루 강타 넉다운·기상 — 사용자 확인 필요.
- ClientFrontendHarness: 기존 Effect 59건(팀장 몫)과 분리해 신규 실패 0 확인 (실행 결과는 세션 로그).
- 발탄 나머지 10개 패턴은 원본 HIT 행에 push/FallDown이 없어 0 유지(원본 그대로).
- standup 무적, twistknockdown(회전 넉다운) 변형, DM down 루프 대체 클립은 후속.
- 회귀 수정 3건(protocol 핀, CSIso 접근-후-시전, vcxproj Effect sync)이 같은 worktree에 있다 —
  커밋 분리 예정.
