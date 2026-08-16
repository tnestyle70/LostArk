# 2026-08-16 플레이어 스킬 히트 셰이프 Server 판정 RESULT

작성자: JS · branch `feature/skill-hit-shape-official-import`
PLAN: `2026-08-16_PLAYER_SKILL_HIT_SHAPE_SERVER_DAMAGE_PLAN.md` (그대로 구현)

## 1. 완료

Client가 그리는 히트 셰이프와 같은 데이터로 Server가 몬스터/보스 피해를 판정한다.

| 계층 | 변경 |
|---|---|
| 데이터 | `Data/Animation/HitShapes/{LanceMaster,Artist,DimensionMaster,Warlord}.hitshapes.json` 신규(v1). LanceMaster 16 스킬 57 hit, Warlord 6/8, DimensionMaster 1/4, Artist 1/1. |
| 생성기 | `Tools/CharacterAnimationIntake/build_hitshapes.py`: `.animevents` HIT(area>0) × skillbindings 체인(playMs/playRate, wmodel clip tick 30/s) → 스테이지 시계 timeMs, m 단위, `actionDurationMs` clamp, angle 0~360 clamp, damage profile 없는 스킬 제외. `CharacterCatalog.json`의 bodyModel로 wmodel을 찾는다. |
| Publisher | `Publish-GameplayBalance.ps1`: 문서 strict 검증(exact fields, 정렬, sub-hit ≤ 64, staged/unstaged 일치, damage profile 필수) 후 `SKILLHIT <id> <n> <packed>` / `SKILLSTAGEHIT <id> <stage> <n> <packed>` 행. bootstrap v5 유지, 530행. |
| Server | `PLAYER_SKILL_HIT`(catalog), `SERVER_PLAYER::iAppliedHitMask`(uint64). `CPlayerSkillSystem::Update`: 스테이지에 shape hit가 있으면 sub-hit(timeMs + repMs·k)를 액션 시계로 한 번씩 발사, 원점 = 위치 + aim·offset, 전방 = `fSkillAimDirection`, box/fan/circle·ring을 Shared `Circle_IntersectsForwardBox/Cone/Ring` + `Circles_Overlap`으로 판정, 대상 = 겹치는 살아 있는 boss/monster 전부(가까운 순, `maxTargets`), sub-hit별 데미지 = 누적 분할(`total·(i+1)/n − total·i/n`, 합계 = 프로필 총량, 최소 1). 마지막 sub-hit 발사 시 `hasAppliedSkillDamage=true`(콤보 캔슬 규칙 유지). shape가 없는 스킬은 기존 `maximumRange` 원형 단일 판정. |
| Contract test | 34120: bootstrap에서 hit 3개 로드, 3 damage event 합 361, HP 9639; 뒤쪽(사거리 안, 부채꼴 밖) 대상 미적중. |
| 프로젝트 | Client.vcxproj `96.DataFiles\Animation\HitShapes` None 4개 등록. `CLAUDE.md` 리소스 경로 규칙 1줄. |

## 2. 검증

- `Publish-GameplayBalance.ps1 -Mode Validate` 성공. Server x64 Debug 빌드 exit 0(pre-build publish 포함).
- `Server.exe --contract-test`: **failures : 0** (기존 34090/콤보/카운터/발탄 테스트 포함).
- Client: 프로토콜·코드 무변경(vcxproj None 항목만). 이날 앞선 Client 빌드가 유효.
- 수동(사용자, 미실행): Server+Client 실행 → Character Select에서 몬스터/발탄 소환 → 스킬 사용 시
  와이어 안 대상만 damage event/HP 감소, 다단히트 스킬(34610 등)에서 여러 damage event.

## 3. 결정과 경계

- 데미지 분할은 PROJECT 규칙(원작 per-SkillEffect 비율은 별도 조사). `DamageProfiles.json` 총량은 유지.
- `PlayerSkills.json`의 `maximumRange`(예: 34120 = 8.0)는 셰이프가 있는 스킬에서 더 이상 판정에
  쓰이지 않는다(셰이프 2.2 m + 대상 반경). 기존 contract test의 보스 위치를 그에 맞게 옮겼다.
- 셰이프 데이터 커버리지는 앞선 RESULT의 한계 그대로(Artist 1, DM 4, Warlord 8 hit) — HIT notify
  재추출(Effect PK)이 되면 `build_hitshapes.py`만 다시 돌리면 된다.
- 다른 플레이어에게는 판정하지 않는다(PvE 전용, 기존과 동일).

## 4. 실행 절차

```
build_hitshapes.py <Asset>...  →  Publish-GameplayBalance.ps1 (Server pre-build 자동)  →  Server 재시작
```
