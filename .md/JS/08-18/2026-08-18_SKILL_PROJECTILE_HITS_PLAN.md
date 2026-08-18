# 2026-08-18 플레이어 스킬 투사체/소환물 히트 PLAN

작성자: JS · branch `feature/skill-projectile-hits` (`feature/timed-hit-shape-stamping` `c38b208` 위)
선행: `2026-08-18_TIMED_HIT_SHAPE_STAMPING_RESULT.md`

## 목표

원작에서 시전자가 아니라 **투사체/소환물/장판이 데미지를 주는 스킬**(도화가 범가르기·두루미나래, 창술사 반월섬·
연환섬·적룡필살, 차원술사 일침 등)의 히트를 caster 고정 셰이프가 아니라 이동/고정 오브젝트로 Server가 판정하고
Debug 와이어로 표시한다. 대상은 4직업(창술사·워로드·도화가·차원술사).

## Phase 1 — 파서 검증 (2026-08-18 완료)

### 원본 데이터 위치와 연결 고리

1. 클래스 `.loa`(`data3/.../XmlData/Action/<CLASS>.loa`)의 `CEFActionNotify_Effect` = 스테이지 clip 시각에 SkillEffect
   PK를 발동. **PK는 record의 `"Notify"` 태그 뒤 +12 바이트**에 있다(라벨 문자열 길이가 가변이라 기존 `+39` 고정 읽기는
   창술사 170/375, 도화가 46/319만 맞음. `"Notify"` 앵커는 창술사 375/375, 차원술사 235/235, 워로드 394/404, 도화가 252/319).
2. `EFTable_SkillEffect` 행이 `HitType=1`이면 데미지가 아니라 오브젝트 스폰이고, 같은 PK의
   **`data1/Common_Extra/XMLData/Projectile/<PK>.loa`**(17,920개)가 오브젝트 정의다.
3. Projectile 파일 = `CEFSequenceSummonsProjectile{Missile|FixArea|Grenade|Trace}` 헤더 + particle data 3블록 +
   **모션 블록** + `CEFSequenceSummonsAction*` 시퀀스(`SkillEffect`가 오브젝트가 적용하는 실제 데미지 PK, `Timer`,
   `Accel`, `SkeletalMeshFX`(범가르기 호랑이 `SK_SDM_TIG_00`), `CreateFX`, `AkEvent`, `CameraShake`).

### 모션 블록 (11 int32, actions count 직전) — 17,920 파일 검증

```
v2  fScale fRadius fHeight  iMinDist iMaxDist  fLife  iSpeed  i i i  iActions   (16,431)
v1         fRadius fHeight  iMinDist iMaxDist  fLife  iSpeed  i i i i iActions   (1,007)
none/hdr  저PK 레거시(1000.loa 등)                                            (482)
```

| 종류 | 수 | speed>0 | life>0 | maxDist>0 | SkillEffect≥1 | 해석 |
|---|---|---|---|---|---|---|
| MISSILE | 2,832 | 100% | 99.5% | 79% | 98% | 전방 직선 이동체. `speed×life ≥ maxDist` 91% |
| FIXAREA | 13,046 | 0.3% | 99.8% | 74% | 95% | 위치 고정 장판(대부분 `700-700 r25`, timer로 적용) |
| GRENADE | 914 | 99% | 99.9% | 88% | 93% | 포물선 |
| TRACE | 646 | 99% | 98% | 54% | 98% | 추적형 |

- 단위: 거리/반경 cm, life 초, speed cm/s. 범가르기 314900 = r150 1100cm 3.0s 650cm/s → 11 m를 1.7 s.
  두루미나래 314800 = 300~1000cm 30 s 900cm/s + Accel. 적룡필살 346500 = 1600cm 1.0 s 700.
- min/maxDist가 다르면 조준 거리 clamp 범위로 추정(두루미나래 3~10 m, 연환섬 8~20 m).
- `Timer` record는 int 하나(1~21)만 가진다 — 시각이 아니라 인덱스/이벤트 번호. FIXAREA는 SkillEffect가 대부분
  timer 1~5 아래, MISSILE은 대부분 timer 0(비행 중 겹침 판정)이다. **timer→시각 매핑은 미해결**.
- 오브젝트가 적용하는 SkillEffect(예: 범가르기 314901 `key=2 box 150×250 az=80`)는 `key`가 1/2여도 caster 행이
  아니다. **`key`는 발동 주체를 뜻하지 않는다** — 어제 스탬프 규칙(`key∈{1,2}` = caster)의 전제가 틀렸고, 실제 판별은
  "어느 노티/오브젝트가 그 PK를 발동하는가"다. 08-18 스탬프 결과 중 두루미나래(314801)·범가르기(314901)는
  투사체 셰이프를 caster에 박은 것이라 Phase 2에서 옮긴다.
- 같은 (skill, seq, clip, t)에 PK가 여러 개면 트라이포드 변형(범가르기 314900/905/907/909) — 최저 PK를 기준으로 본다.

### 산출물

- `C:\Users\95jus\Desktop\buildScript\extract_projectiles.py <CLASS.loa> <ASSET> <PREFIX> <OUTDIR> <TABLES>` (Git 미추적 관례)
- `Data/Animation/Reference/<Asset>/<Asset>.projectiles` v1: 행 = `<skill> "<name>" pk kind seq clip t scale radius
  height mindist maxdist life speed layout accel mesh`, 하위 `  e pk timer key hittype area ar aa ah ax az arem maxt rep
  repms push pushr origin self` = 오브젝트가 적용하는 SkillEffect 셰이프. LM 52행/10스킬, WL 28/5, Artist 95/15, DM 98/13.

### 4직업 damage 스킬 중 투사체 사용 (트라이포드 최저 PK 기준)

| 클래스 | 스킬 → 오브젝트 |
|---|---|
| 창술사 | 반월섬 34110 MISSILE 18 m/1.8 s/1000 · 연환섬 34120 base clip은 FIXAREA(트라이포드 clip만 MISSILE) · 선풍참혼 34140 MISSILE 3~14 m/600 · 적룡필살 34650 MISSILE 16 m/1.0 s/700 · 이연격 34040·사두룡격 34550 FIXAREA 7 m |
| 워로드 | 리프 어택 17110 FIXAREA |
| 도화가 | 범가르기 31490 MISSILE 11 m/3 s/650 · 두루미나래 31480 MISSILE 3~10 m/900 Accel · 흩뿌리기 31430 MISSILE 10 m/600 · 호접몽 31460 FIXAREA+TRACE 4 m/1000 · 난치기 31420·한획긋기 31470·미르새김 31950 FIXAREA |
| 차원술사 | 일침·분절·건너찌르기·너머베기·일점관통·시간분쇄·업의경계·시간의굴레 FIXAREA(700-700 r25) · 경계돌파 2050240 MISSILE 14 m/365 |

## Phase 2 — 계약과 구현 (2026-08-18 완료, 상세는 RESULT)

사용자 결정: "원작에 맞는 방식" — MISSILE과 FIXAREA를 함께 1차에 넣고, TRACE/GRENADE는 MISSILE과 같은 직진으로
근사한다(4직업 damage 스킬엔 TRACE 호접몽만 있고 그마저 base seq에선 FIXAREA라 실사용 없음).

| 계층 | 계약 |
|---|---|
| Reference | `<Asset>.projectiles` v1 (Git 추적). `e` 행에 `at/count/every`(스폰 기준 예약) 또는 `contact=1`, `dmg`(ValueA 데미지율, Target 2·자기참조 아님이면 >0), `target` |
| Authored | `Tools/CharacterAnimationIntake/fill_projectiles.py` → `<Asset>.projectiles.json` v1: binding chain 클립만, **스킬의 최저 clipseq 그룹(트라이포드 없는 base) 행만**, 같은 시각은 최저 PK, `dmg>0 & area>0` 히트가 없는 오브젝트는 제외. m/ms 단위 |
| HitShapes | `build_hitshapes.py` v3: skill/stage `projectiles[]` = `timeMs`(stage-local spawn), kind, speed, min/maxDistance, lifeMs, radius, `hits[]`(trigger CONTACT/TIMED, atMs, count, everyMs, 13필드 셰이프). `hits`가 비고 `projectiles`만 있는 skill/stage 허용 |
| Publisher | `SKILLPROJ <skill> <index> <timeMs> <kind> <speed> <min> <max> <lifeMs> <radius> <hitCount> <hits>` / `SKILLSTAGEPROJ <skill> <stage> …`, hit = `trigger:atMs:count:everyMs:` + 10필드 extent. `Assert-HitShapeExtent`/`Format-HitShapeExtent` 공용. bootstrap **v9** |
| Server | `PLAYER_SKILL_PROJECTILE{iTimeMs,eKind,fSpeed,fMin/MaxDistance,iLifeMs,fRadius,Hits[PLAYER_PROJECTILE_HIT{isContact,Hit}]}` on skill/stage. `SERVER_PLAYER::Projectiles`, `iSpawnedProjectileMask`, `fSkillAimDistance`(Try_Start/Update_Aim/콤보 buffered aim). `CPlayerSkillSystem::Update`가 spawn 시각에 생성: MISSILE 이동거리 = clamp(aim, min, max) (max 0이면 수명만), FIXAREA 위치 = caster + aim×min(aimDistance, max) (max 0이면 caster). `Update_Projectiles`(매 tick, 액션 없어도)가 이동 → 접촉 히트(대상·히트별 count/every 마크) → 예약 히트(마스크) → 거리/수명 만료 제거. damage = 스킬 rate를 caster sub-hit + 투사체 sub-hit로 분할(`DamageOfSubHit`), 넉백 기준점은 오브젝트 위치. 사망·class 변경 시 clear |
| Client | `CAnimationEffectCueDocument::Load_Projectiles`가 `.projectiles.json`을 optional로 읽어 `ANIMATION_PROJECTILE_CUE`(cm 셰이프)로 보관, `CCharacter::Update_SkillProjectileDebug`가 clip 시계가 startMs를 지날 때 facing 방향으로 예측 와이어(주황)를 스폰·이동·소멸. FIXAREA는 aim을 몰라 최대 거리 끝에 표시(근사) |
| Caster 스탬프 | `fill_animevents_hit_shapes.py`가 `.projectiles`의 dmg 셰이프와 동일 tuple인 skilltiming 행을 caster 후보에서 제외(두루미나래·흩뿌리기·미르새김·일점관통), 재실행 시 더 이상 생성되지 않는 클립의 `src=orig` HIT 행은 제거 |

## 열린 질문 → 결정

1. FIXAREA 1차 포함 — 포함(사용자 결정).
2. TRACE/GRENADE — Server는 MISSILE 이동으로 처리, 별도 곡선/추적은 후속.
3. Timer→시각 — 해결됨: entry trailer `0 1 1 1 1 0 fStart iRepeat fInterval 0`가 스폰 기준 예약 시각·반복이고, trailer 없는 SkillEffect가 접촉 히트.
