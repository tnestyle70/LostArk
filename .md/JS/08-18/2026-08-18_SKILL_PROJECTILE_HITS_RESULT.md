# 2026-08-18 플레이어 스킬 투사체/소환물 히트 RESULT

작성자: JS · branch `feature/skill-projectile-hits` (`feature/timed-hit-shape-stamping` `c38b208` 위)
PLAN: `2026-08-18_SKILL_PROJECTILE_HITS_PLAN.md`

## 1. 완료

| 항목 | 내용 | 상태 |
|---|---|---|
| Phase 1 파서 | `buildScript/extract_projectiles.py`(Git 미추적 관례) → `Data/Animation/Reference/<Asset>/<Asset>.projectiles` v1. 17,920 파일 중 97.3% 모션 블록 해석, `"Notify"` 앵커 PK 읽기, entry trailer로 예약 시각 | 완료 |
| Authored | `Tools/CharacterAnimationIntake/fill_projectiles.py` → `<Asset>.projectiles.json` v1: LM 0·Artist 7(6스킬)·DM 6(4)·WL 0 | 완료 |
| HitShapes v3 / bootstrap v9 | `build_hitshapes.py` `projectiles[]`, `Publish-GameplayBalance.ps1` `SKILLPROJ/SKILLSTAGEPROJ`(23행) | 완료 |
| Server | catalog 파싱, `SERVER_SKILL_PROJECTILE` 스폰·이동·접촉/예약 히트·만료, damage 분할, `fSkillAimDistance` | 완료 |
| Client | `.projectiles.json` optional 로드 + Debug 예측 와이어(주황) | 완료 |
| Caster 중복 제거 | 투사체가 적용하는 셰이프는 caster 스탬프에서 제외, stale `src=orig` 행 제거 | 완료 |
| 문서 | CLAUDE.md 히트 셰이프 계약 문장, PLAN/RESULT | 완료 |

Shared/protocol 무변경. Client 판정 없음.

## 2. 4직업에 실제로 붙은 오브젝트 (스킬 최저 clipseq 그룹 = 트라이포드 없는 base만, 같은 시각은 최저 PK)

사용자 지적(2026-08-18): 반월섬이 날아가는 것은 원작 base가 아니다 → 처음 규칙(클립별 최저 seq)은 base 그룹에 스폰이
없으면 트라이포드 그룹(반월섬 seq 4의 파동, 선풍참혼 seq 3의 창 던지기, 흩뿌리기 seq 4)을 집어 왔다. 이제 스킬의
최저 seq 그룹 행만 쓴다 — 창술사는 base 스폰이 없어 0개.

| 클래스 | 스킬 | 오브젝트 |
|---|---|---|
| 창술사 | — | base 그룹 스폰 없음(반월섬 파동·선풍참혼 창·연환섬/이연격/청룡출수 장판은 전부 트라이포드) |
| 도화가 | 난치기 31420 | FIXAREA 7 m @695, box 7×3 ×6/500 ms from 500 |
| | 호접몽 31460 | FIXAREA 7 m @543(box 4×2.5 ×2/300 from 300)·@1150(×3) |
| | 한획긋기 31470 | FIXAREA 7 m @1460, box 6×3 ×6/700 ms |
| | 두루미나래 31480 | MISSILE @1000, 9 m/s 3~10 m 수명 30 s, contact box 1.5×3.5 ×2/250 ms push |
| | 범가르기 31490 | MISSILE @1147, 6.5 m/s 11 m 3 s, contact box 1.5×2.5 |
| | 미르새김 31950 | FIXAREA 7 m @600, circle 3 m @1200 push |
| 차원술사 | 일침 2050100 | FIXAREA 7 m @280, circle 1 m @0 push |
| | 너머베기 2050180 | FIXAREA 7 m ×3(@500/700/1050) box 1.5~2.6×3~4.5 |
| | 일점관통 2050220 | FIXAREA 7 m @1100, box 6.5×2.5 @700 |
| | 시간의굴레 2050520 | FIXAREA 7 m @1550, 7단계 수축 링 @300~2100 |
| 워로드 | — | 리프 어택 FIXAREA는 데미지 행 없음(visual-only) |

흩뿌리기 31430은 base에 스폰이 없어 caster 스탬프(box)로 돌아갔고, 일침·일점관통·시간의굴레는 FIXAREA가 같은
셰이프를 적용하므로 caster 스탬프에서 빠졌다.

## 3. 검증

자동:
- `extract_projectiles.py` 4직업 재추출(LM 52행/WL 28/Artist 95/DM 98), `fill_projectiles.py`, `fill_animevents_hit_shapes.py`(idempotent), `build_hitshapes.py` OK
- `Publish-GameplayBalance.ps1 -Mode Validate` OK; Server pre-build Publish → bootstrap v9(SKILLPROJ 13)
- Server Debug 빌드 OK, `Server.exe --contract-test` **failures 0** — 추가 require 11건: 범가르기 missile 정의 로드, 스폰 후 도달 전 무피해, 8 m 대상 tick 61~63 접촉 1회, 데미지 caster/missile 절반 분할, 거리 소멸·액션 해제, 뒤쪽 대상 무피해, 일침 FIXAREA 정의, 12 m 조준 시 7 m clamp 지점 히트(9.5 m 대상 O, 12 m 대상 X). 연환섬 기존 require(3 sub-hit/361)는 base에 장판이 없어 그대로
- Client Debug 빌드 OK
- ClientFrontendHarness: 실행 결과는 아래 4절

수동(사용자, 미실행): Server + Client 재기동 후 Character Select에서 도화가 범가르기·두루미나래·호접몽, 차원술사 일침·시간의굴레(창술사는 base 투사체 없음) — 주황 와이어가 캐릭터를 떠나 이동/장판으로 서고 그 위치의 몬스터만 맞는지.

## 4. 남은 경계

- FIXAREA 위치는 조준점 clamp(min 무시) 추정. Debug 와이어는 aim을 몰라 최대 거리 끝에 그린다(Server와 다를 수 있음).
- TRACE/GRENADE는 직진 근사(현재 base 4직업엔 해당 없음). 트라이포드 그룹의 스폰(반월섬 파동 등)은 트라이포드 시스템이 생기기 전까지 제외.
- 원본 `SkillEffect` `MultiHitCount/Time`을 접촉 히트의 count/every로 썼고 예약 히트는 trailer 값만 쓴다.
- Client 예측 와이어와 Server 판정 위치의 오차(방향은 facing, Server는 aim)는 Debug 참고용.

## 5. 사용자 지적 후속 (2026-08-18 저녁): 스폰 위치와 4직업 히트 시각 정합

**위치** — 스폰 SkillEffect 행의 `AreaOrigin`이 오브젝트가 나타나는 기준이었다(0 = 시전자, `AreaOffsetX/Y` cm만큼 전방/우측;
1 = 조준점). FIXAREA 스폰 행 9,670개가 0, 2,060개가 1이고 GRENADE는 1이 다수, MISSILE은 0/1 반반(=방향만).
처음 구현은 FIXAREA를 전부 조준점 7 m에 두었으나 일침(`origin 0 ax=80` → 시전자 앞 0.8 m), 일점관통(0.05/우 0.5),
시간의굴레(0.25), 도화가 장판 4종은 시전자 기준이고 너머베기 3장판만 AIM이다. Reference `origin/ox/oy` → Authored
`origin CASTER|AIM`, `offsetForward/Right` → SKILLPROJ 필드 3개 추가 → Server `PLAYER_PROJECTILE_ORIGIN`
(CASTER: 시전자 + 전방/우측 offset, AIM: 조준 거리를 maxDistance로 clamp) → Client 와이어도 CASTER는 정확한 위치.
contract test의 일침 케이스를 "시전자 앞 0.8 m, 4 m 대상 O / 9.5 m 대상 X"로 갱신.

**히트 시각** — hitTimeMs 스탬프가 클립의 타격 큐(SHAKE + `*_Atk/*_Exp` FX)와 150 ms 넘게 어긋난 스킬을 4직업 전체에서
검색해 `Data/Balance/PlayerSkills.json`과 receipt(`animation-reference-hit-timing-v1`, PROJECT_TUNED)를 함께 갱신했다.
차원술사 값은 0.85×duration 자리표시자였다.

| 스킬 | 이전 | 이후 | 근거 |
|---|---|---|---|
| 창술사 적룡포 34590 stage2 | 600 | 322 | lastwhisper_end SHAKE 322 + ChehuExp 324 |
| 워로드 파이어 불릿 17060 | 700 | 330 | FireBullet_01 FX 328 |
| 워로드 가디언의 낙뢰 17140 | 1000 | 1200 | GuardianLightning 1200 |
| 도화가 몽유도원 31910 | 3000 | 3400 | 2SuperMove_05_Exp 3400 |
| 도화가 몽중백화원 31930 | 1426(기본값) | 4633 | 2nd clip FlowerGarden_Exp 2900 (+1733) |
| 차원술사 분절 2050120 | 1927 | 950 | leap_03 TentDevider_Atk 250 (+700) |
| 차원술사 시간 분쇄 2050230 | 1983 | 700 | Chrono_Atk 700 |
| 차원술사 경계 돌파 2050240 | 1700 | 1047 | thrust_04 FlickerThrust 547 (+500) |
| 차원술사 업의 경계 2050500 | 3712 | 2858 | DimensionPrison_exp 2858 |
| 차원술사 무간의 옥 2050540 | 4335 | 3950 | TW_Exp/ShockWave 3950 |

나머지(창술사·워로드·도화가 대부분, DM LMB·분광·건너찌르기)는 이미 큐와 ±150 ms 안이라 두었다. 일침·일점관통·너머베기·
시간의굴레는 FIXAREA 예약 시각(원본)이 판정 시각이다. 검증: publisher Validate OK(receipt 정합), Server 빌드·contract-test
0 fail, `.animevents` 재스탬프 idempotent.
