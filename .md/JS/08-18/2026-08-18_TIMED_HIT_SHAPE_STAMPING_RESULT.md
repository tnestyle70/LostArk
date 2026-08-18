# 2026-08-18 notify HIT 없는 스킬의 히트 셰이프 스탬프 RESULT

작성자: JS · branch `feature/timed-hit-shape-stamping` (main `fd54742`)
PLAN: `2026-08-18_TIMED_HIT_SHAPE_STAMPING_PLAN.md`

## 1. 완료

| 항목 | 내용 | 상태 |
|---|---|---|
| 스크립트 | `fill_animevents_hit_shapes.py`에 `synthesize_timed_rows()` 추가. binding chain에 `.animnotify` HIT가 하나도 없는 damage 스킬을 찾아, skilltiming 참조 행의 **caster-key(1/2) 셰이프 한 행**(timed=1 우선, g=0 우선)을 **`PlayerSkills.json hitTimeMs`(stage면 `comboStages[i].hitTimeMs`)를 clip 로컬 시간으로 역산한 위치**에 `HIT ... src=orig` 행으로 생성. `load_skilltiming`이 `timed/t/g/w/key` 보존. 재실행 idempotent | 완료 |
| 데이터 | 4직업 `.animevents` 재스탬프 → `build_hitshapes.py` → `hitshapes.json`. LM +3, WL +3, Artist +15, DM +8 행 (아래 표) | 완료 |
| Client | `CCharacter::Draw_SkillHitAreaDebug` 와이어 최소 표시 폭 300 ms (`MIN_VISIBLE_HIT_WINDOW_MS`, Debug 전용) | 완료 |

| AreaType 정정 | 원본 `AreaType` 1=원/링, 2=전방 박스(`AreaAngle`=폭 cm), 3=부채꼴(도)로 재매핑. #110은 1=box/2=fan/3=circle이라 type 2 박스(나선창 3.2×1.4 m, 적룡포 8×1.7 m, 한획긋기 6×3 m…)가 폭 cm를 각도로 읽은 부채꼴로 판정·표시돼 "원작보다 넓다"(사용자 관찰). `hitshapes.json` v1→v2(`width` 필드), `Gameplay.bootstrap` v7→v8(SKILLHIT 13필드), `PLAYER_SKILL_HIT::fWidth`, Server `Hit_ShapeOverlaps`·Client `HitAreaWire`·Animation Tool 라벨 정정. 근거는 PLAN 추가 G의 DB 통계 | 완료 |

Shared/protocol 무변경.

## 2. 결정: 시각은 skilltiming `t`가 아니라 `hitTimeMs`

PLAN 실측대로 `t`는 창술사 34650/34630에서만 실제 타격(SHAKE 500/1444)과 맞고, 도화가/차원술사/워로드는
`1445/1451/1657/1730` 공용 기본값이거나 chain 밖(2050520)이며 HOLD 34590은 loop 안에 떨어진다. 반면
`hitTimeMs`는 receipt에서 전부 `animation-reference-hit-timing-v1`로 이미 튜닝된 현재 Server 판정 시각이고
창술사는 t와 같다(1240/1445). 따라서 **타이밍은 지금과 동일하고 원형 단일 판정 → 셰이프 판정만 바뀐다.**
`hitshapes.timeMs == hitTimeMs`가 정확히 왕복됨을 확인했다(34650 1240, 34630 1445, 34590 stage2 600).

셰이프 선택 규칙: 참조 행(id 일치 → base 일치 변형)의 area>0 히트 중 `key ∈ {1,2}`(caster 발동)만 후보. `key≥7`은
투사체/소환물 등 자식 오브젝트가 발동하는 행이라 caster 셰이프가 아니다. 후보 중 `timed=1` 우선, `g=0` 우선, 첫 행 하나.
`timed=1` 행의 `t`가 여러 개(타임라인)면 단일 hitTimeMs로 표현할 수 없어 건너뛴다.

## 3. 스탬프 결과 (셰이프 표기는 원본 AreaType 정정 후 기준: fan→도, box→길이×폭)

| 클래스 | 스탬프된 스킬 (clip @ local ms → shape) | 건너뜀 |
|---|---|---|
| 창술사 | 34650 T 적룡필살 dragoncleave_03 @507 box 2.6×3.0m ax-1.5 · 34630 ALT_V 마룡합일섬 squalllance_01 @1445 box 4.0×2.5m · 34590 단창 S 적룡포 stage2 lastwhisper_end @600 box 8.0×1.7m ax-0.2 | — |
| 워로드 | 17060 파이어 불릿 @700 fan 3.8m/60° ax-0.2 push · 17100 방패 격동 @700 circle 2.3m ax0.8 push · 17140 가디언의 낙뢰 @1000 circle 3.0m push | 17240 (skilltiming 행 없음) |
| 도화가 | 31000 LMB 4 stage box 3.2×2.5m · 31200 먹물세례 @625 box 5.0×1.8m ax0.3 push · 31210 콩콩이 stage0/1 kongkong_01 @600, stage2 kongkong_02 @533 circle 2.0m · 31430 흩뿌리기 @562 fan 4.5m/110° push · 31460 호접몽 @540 **rep=2 500ms** box 4.0×2.5m · 31470 한획긋기 @1451 box 6.0×3.0m · 31480 두루미나래 @997 **rep=2 250ms** box 1.5×3.5m push · 31490 범가르기 @1000 box 1.5×2.5m · 31910 몽유도원 @3000 circle 4.0m push · 31930 몽중백화원 @1426 circle 5.0m · 31950 미르 새김 @1700 circle 3.0m push | — |
| 차원술사 | 2050100 일침 @1190 circle 1.0m push · 2050120 분절 leap_03 @1227 circle 1.5m ax0.5 push · 2050160 건너 찌르기 overslash_02 @250 box 4.5×1.2m ax-2.25 · 2050220 일점 관통 @1451 box 6.5×2.5m ax-0.2 · 2050230 시간 분쇄 @1983 circle 3.0m push · 2050240 경계 돌파 thrust_04 @1200 box 5.0×2.0m ax-0.2 · 2050500 업의 경계 @3712 circle 3.0m · 2050540 무간의 옥 @4335 circle 5.0m | 2050180 (행 없음), 2050520 시간의 굴레 (7단계 수축 링 타임라인) |

**인게임 판단이 필요한 항목** — 원본이 소환/투사체로 데미지를 주는 스킬은 caster 행 셰이프가 작아 현재
`maximumRange` 원형(12 m 등)보다 좁아진다: 도화가 31480 두루미나래(box 1.5×3.5 m), 31490 범가르기(box 1.5×2.5 m), 차원술사
2050100 일침(circle 1 m), 2050120 분절(circle 1.5 m). 원본 rep>0 행(31460 호접몽, 31480)은 sub-hit 2회로 나뉘어 총 데미지는
같고 타격이 2회로 분할된다. 과하면 해당 스킬만 되돌리는 정책이 필요하다(스크립트 skip 목록 또는 hitTimeMs 외
별도 필드) — 아직 없음.

## 4. 검증

자동:
- `fill_animevents_hit_shapes.py --check`/실행 4직업, `build_hitshapes.py` 4직업 OK (`hits` 60/11/17/12)
- `Publish-GameplayBalance.ps1 -Mode Validate` OK; Server pre-build Publish로 `Gameplay.bootstrap`에 SKILLHIT 34650/34630, SKILLSTAGEHIT 34590 stage2 등 확인(총 63행)
- Server Debug 빌드 OK, `Server.exe --contract-test` **failures 0** (AreaType 정정 후 재실행 포함, bootstrap v8 SKILLHIT 63행: 34540 `2:3.2:0:1.4`, 34650 `2:2.6:0:3:…:-1.5`)
- Client Debug 빌드 OK (AreaType 정정 후 재빌드 포함)
- Animation Tool 로더 호환: `src=orig`만 사용

수동(사용자, 미실행): Server(127.0.0.1) + Client 재기동 완료. Character Select → 창술사 T/ALT_V/단창 S 와이어가
`F1 Show Skill Hit Area`로 보이고 셰이프 밖 대상은 안 맞는지, 도화가/차원술사의 좁아진 항목(3절)이 허용 범위인지.

## 5. 남은 경계

- 소환/투사체 스킬(31480/31490/2050100/2050120 등)의 자식 오브젝트 히트는 모델링하지 않는다.
- 2050520 시간의 굴레 7단계 링 타임라인, 17240/2050180(skilltiming 참조 없음)은 원형 단일 판정 유지.
- Animation Tool에서 해당 클립 `Import original`을 다시 하면 이 합성 행도 imported 취급으로 지워진다. 스크립트 재실행으로 복구.
- 정확한 타격 시각 매칭(Effect notify +39 PK 재추출)은 여전히 별도 단계.
