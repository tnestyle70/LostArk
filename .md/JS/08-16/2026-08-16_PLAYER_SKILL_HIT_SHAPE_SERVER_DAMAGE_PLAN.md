# 2026-08-16 플레이어 스킬 히트 셰이프 Server 판정 PLAN

작성자: JS · branch `feature/skill-hit-shape-official-import`
선행: `2026-08-16_SKILL_HIT_SHAPE_OFFICIAL_IMPORT_RESULT.md` (Client Debug 와이어까지 완료)

## 목표

Client가 그리는 히트 셰이프(fan/circle/box, `.animevents` HIT 행)와 같은 시각·형상으로 Server가
몬스터/보스 피해를 판정한다. 데미지는 `DamageProfiles.json`의 스킬당 비율을 히트 수로 균등 분할해
합계를 유지한다. Server는 Client 문서(`.animevents`, `.wmodel`)를 직접 읽지 않는다.

## 데이터 경로 (RootMotion 선례와 동일)

1. `Tools/CharacterAnimationIntake/build_hitshapes.py`
   입력: `<Asset>.animevents` HIT 행, `<Asset>.skillbindings.json`(clip 순서·playMs·playRate),
   `<Asset>.wmodel`(clip tick, 30 tick/s), `PlayerSkills.json`(actionDurationMs, comboStages, damage profile).
   출력: `Data/Animation/HitShapes/<Asset>.hitshapes.json`
   ```
   { "schema": "lostark.animation-hit-shapes", "formatVersion": 1, "animationAssetId", "characterClass",
     "skills": [
       { "skillId": 34040, "hits": [ { "timeMs", "repeatCount", "repeatMs", "areaType", "range", "angle",
                                        "height", "offset", "inner", "maxTargets" } ] },
       { "skillId": 34010, "stages": [ { "stageIndex": 0, "hits": [ ... ] } ] } ] }
   ```
   timeMs = 스테이지(액션) 시계: 앞 clip들의 `min(clipDur, playMs)/playRate` 합 + `startms/playRate`,
   `actionDurationMs`로 clamp. 거리 단위는 m(원작 단위 × 0.01). `area=0` 행은 제외.
   damage profile이 없는 스킬은 제외.
2. `Publish-GameplayBalance.ps1`: 문서 strict 검증(exact properties, 시간·개수 한계, 스테이지 수 일치,
   sub-hit ≤ 64) 후 `SKILLHIT <id> <count> <packed>` / `SKILLSTAGEHIT <id> <stage> <count> <packed>` 행 추가.
   packed = `timeMs:rep:repMs:area:range:angle:height:offset:inner:maxt,...`
3. `CGameplayCatalog`: `PLAYER_SKILL_HIT` 벡터를 skill/stage에 파싱(fail-closed).

## Server 판정

`CPlayerSkillSystem::Update`: 스테이지에 shape hit가 있으면 기존 단일 `hitTimeMs` 원형 판정 대신
각 sub-hit(timeMs + repMs·k)를 액션 시계로 한 번씩 발사한다.
- 원점 = 플레이어 위치 + aim·offset, 전방 = `fSkillAimDirection`.
- area 1 → `Circle_IntersectsForwardBox(length=range, halfWidth=height/2)`,
  area 2 → `Circle_IntersectsCone(range, angle)`(angle 0/≥360은 원),
  area 3 → `Circle_IntersectsRing(inner, range)`(+angle 제한 시 cone 교차 병행).
- 대상 = 조건을 만족하는 살아 있는 boss/monster 전부(가까운 순, `maxTargets`>0이면 그 수만).
- sub-hit당 데미지 = `Resolve_Damage(AP, rate) / subHitCount`(최소 1). 몬스터는 방어 적용.
- 발사 여부는 `SERVER_PLAYER::iAppliedHitMask`(uint64) 비트로 추적, 스테이지 전환/종료 시 0.
  마지막 sub-hit이 발사되면 `hasAppliedSkillDamage=true`(콤보 캔슬 규칙 유지).
- shape hit가 없는 스킬/스테이지는 기존 경로 그대로.

## 검증

- `Publish-GameplayBalance.ps1 -Mode Validate/Publish`, Server 빌드, `Server.exe --contract-test`
  (기존 34090/콤보 데미지 테스트가 새 경로로 통과하는지 + 셰이프 밖 대상 미적중 테스트 추가),
  NetworkProtocolHarness 불변, Client 재빌드 불필요(프로토콜 무변경).
- 수동: Character Select에서 몬스터 소환 후 스킬 → 와이어 안 대상만 HP 감소·damage event.

## 경계

- Client 표현·프로토콜 변경 없음. Server가 Client 문서를 읽지 않으며 publisher가 정본을 생성한다.
- 데미지 균등 분할은 PROJECT 규칙(원작 per-hit 비율은 별도 조사).
