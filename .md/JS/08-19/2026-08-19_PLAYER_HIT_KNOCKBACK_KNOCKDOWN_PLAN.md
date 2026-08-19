# 2026-08-19 플레이어 피격 넉백·넉다운 PLAN

작성자: JS · base `main ef642ae` (+ 로컬 회귀 수정: protocol V23 하네스 핀, CSIso 접근-후-시전)
선행: `985ecce`(몬스터 넉백), `.md/JS/08-18/2026-08-18_SKILL_PROJECTILE_HITS_RESULT.md`

## 목표

루가루·잡몹의 근접 공격과 발탄 패턴에 플레이어가 맞으면 **원작 수치대로 밀려나고(push),
원작이 FallDown으로 저작한 패턴은 넉다운**시킨다. Server가 변위·상태를 권위로 판정하고
Client는 스냅샷 위치·action으로 표현만 한다. 사용자 결정: 넉다운 포함, 음수 push(당김) 포함.

## 원본 데이터 근거 (2026-08-19 실측)

- `EFTable_SkillEffect` HIT 행이 push를 소유: `PushMinRange/PushMaxRange`(cm, **음수 = 시전자
  쪽으로 당김**), `PushMinTime/PushMaxTime`(ms), `FallDown`(1 = 넉다운), `HitTypeDownMin/Max`(ms,
  넉다운 지속). 몬스터 넉백(985ecce)과 같은 규약.
- 발탄 HIT 행 PK는 `Valtan.animnotify`의 `kind=HIT src=Effect asset="<8자리>"` = `actionId*100+변형`.
  실측 예: 휘두르기 42060101 push 200~220cm/242~262ms FallDown 1, 내려찍기 42060202~04
  거리 밴드별 25~220cm FallDown 1, 감금 사자후 42060305~307 **-90~-460cm(당김)**/42060308~310
  +90~460cm(밀침) FallDown 1.
- 플레이어 4직업 모델에 표현 클립 실존: `flm/wgl/sdm_knockdown → _knockdown_land → _down(loop)
  → _standup_1`, DM은 `pc_sp_m_00_sk_knockdown/_land/_standup_1`(down 루프 없음 → land 마지막
  프레임 유지). 피격 경직용 `*_dmg_idle_1`도 전 직업 존재.
- Server 피격 지점은 두 곳뿐: `ValtanBrain.cpp ApplyPatternHit`(:507), `MonsterBrain.cpp`
  PATTERN_ACTIVE(:145). 몬스터 넉백 `Advance_Knockback`(:195)이 미러링 원형.

## 계약

| 계층 | 계약 |
|---|---|
| Data | `ValtanEncounter.json` stage에 `pushRangeM`(m, 음수=당김) `pushMs` `knockdown`(bool) `downMs` 추가. 값은 stage damage 근거 행과 같은 HIT 행에서 추출(`Tools/CharacterAnimationIntake/fill_encounter_push.py` 신규, DB 경로 인자). `MonsterProfiles.json`에 `attackPushRangeM/attackPushMs/attackKnockdown/attackDownMs`(잡몹 3종 소액 push, Lugaru는 원본 미확보 시 PROJECT_TUNED 튜닝값+knockdown) |
| Receipt | 추가 field는 publisher receipt 동기화에서 `PROJECT_TUNED`(coverage 검사 대상), 추출 근거 PK는 PLAN/RESULT에 기록 |
| Publisher | `Publish-GameplayBalance.ps1` boss pattern stage 행에 push 4필드 pack → `Gameplay.bootstrap` 버전 +1. `MonsterProfiles` → spawngroupsbootstrap 버전 +1 (hitKnockbackScale 전례) |
| Shared | `PLAYER_ACTION_STATE`에 `KNOCKDOWN`(DEAD 앞) 추가 → `NETWORK_PROTOCOL_VERSION` 23→24, Writer/Reader 검증 갱신, NetworkProtocolHarness 왕복·거부 테스트와 V24 핀 |
| Server | `SERVER_PLAYER`에 `fKnockbackDirectionX/Z, fKnockbackSpeed, fKnockbackRemainingSeconds, iKnockdownEndTick` 추가. `ApplyPatternHit`/`MonsterBrain`이 데미지 적용 시 arm: 방향 = 보스/몬스터 중심→플레이어 정규화(음수 push면 반전), 거리 = pushRangeM, 속도 = 거리/pushMs. knockdown이면 `eAction=KNOCKDOWN`, `iKnockdownEndTick=now+downMs`, 진행 중 스킬/이동/콤보 초기화. `Advance_PlayerKnockback`(GameRoom tick, 이동 입력 적용 전)이 몬스터판과 동일하게 walkable clamp+body 충돌로 전진, 벽이면 조기 종료. KNOCKDOWN 동안 `C2S_MOVE`/`C2S_USE_SKILL` 거부, 만료 tick에 `NONE` 복귀. DEAD 전이가 항상 우선 |
| Client | `CHARACTER_ANIM`에 `KNOCKDOWN_START/KNOCKDOWN_LAND/DOWN_LOOP/STANDUP` 추가, 4직업 스펙에 클립명, Gunslinger/Slayer는 nullptr → HIT 클립 fallback. `CCharacter` action 스위치에 KNOCKDOWN: start→land(non-loop 체인)→down loop, KNOCKDOWN→NONE 전이에 standup 1회 후 idle. 위치는 기존 스냅샷 보간 그대로 |
| 검증 | Server contract test: push arm 방향·당김 부호·벽 clamp·KNOCKDOWN 입력 거부·downMs 만료 복귀·DEAD 우선. NetworkProtocolHarness v24. 빌드 후 `Invoke-BuildAndRegression` + 사용자 육안(발탄 휘두르기 피격, 사자후 당김, 루가루 강타) |

## 열린 결정 → 채택

1. 거리 밴드 다중 push(내려찍기) — 발탄 stage damage는 Balance Tool 수작업(PROJECT_TUNED,
   official binding 없음, receipt 실측)이라 행 재사용 불가. 대표 행 = 패턴 sourceActionIds의
   HIT 행 중 `|PushMinRange|` 최대, 동률이면 최저 PK(트라이포드 전례). PushMinTime 0이면 150ms,
   FallDown=1인데 HitTypeDownMin 0이면 2000ms 기본.
2. push 값 min/max — 결정성 위해 min 사용(몬스터 넉백 전례와 동일).
3. KNOCKDOWN 중 추가 피격 — 데미지는 받되 넉백 재-arm 안 함(진동 방지), 원작 유사.
4. standup 무적 — 이번 범위 제외, 후속.
