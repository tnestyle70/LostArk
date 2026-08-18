# 2026-08-18 notify HIT 없는 스킬의 히트 셰이프 스탬프 PLAN

작성자: JS · branch `feature/timed-hit-shape-stamping` (main `fd54742`)
선행: `../08-17/2026-08-17_MONSTER_HIT_FLASH_KNOCKBACK_RESULT.md` 3.1

## 목표

원본 클립 notify에 HIT가 없어 `fill_animevents_hit_shapes.py`가 셰이프를 못 붙인 스킬(창술사 T 34650,
ALT_V 34630, 단창 S 34590 등)에 `.animevents` HIT 행을 생성해 Server 셰이프 판정과 Client Debug 와이어가
붙게 한다. 대상은 4직업(창술사·워로드·도화가·차원술사)만.

## 실측 (2026-08-18)

- 4직업 binding 중 damage profile이 있고 chain 클립에 HIT notify가 하나도 없는 스킬은 27개
  (LM 3, WL 4, Artist 12, DM 10 — 아래 표). skilltiming 참조 행이 있는 스킬은 전부 area>0 셰이프를 가진다.
- skilltiming `timed=1 t=<ms>`가 실제 타격과 맞는 것은 창술사 34650(t=1240 → dragoncleave_03 507 ms,
  SHAKE 500)·34630(t=1445 → squalllance_01 1445, SHAKE 1444)뿐이다. 나머지는 `1445/1451/1657/1730`
  SkillEffect 공용 기본값이거나(도화가/차원술사), HOLD 34590은 loop 클립 안에 떨어지고, DM 2050520은
  t=4130~4730이 chain 총 2533 ms 밖이다.
- `w`는 모든 행에서 `t/19`(65/1240, 76/1445, 87/1657 …)라 실제 판정 창이 아니라 추출 파생값이다.
- `Data/Balance/PlayerSkills.json`의 `hitTimeMs`/`comboStages[i].hitTimeMs`는 receipt에서 전부
  `PROJECT_TUNED animation-reference-hit-timing-v1`이며 창술사는 t와 동일(1240/1445), DM은 0.85 규칙 등으로
  이미 튜닝돼 있다. Server는 셰이프 없는 스킬을 이 시각에 `maximumRange` 원형 단일 판정한다.
- Animation Tool 로더(`Animation_Tool.cpp` `if ("orig" != value) return fail`)는 `src=orig` 외 태그를 거부한다.
- Client Debug 와이어(`CCharacter::Draw_SkillHitAreaDebug`)는 `[startms, endms]` 안에서만 그린다.

## 결정

| 항목 | 값 |
|---|---|
| 대상 스킬 | binding chain의 어느 클립에도 `.animnotify` HIT가 없고 skilltiming 참조 행(id 일치 → base 일치 변형)에 area>0 히트가 있는 스킬 |
| 시각 | **skilltiming t가 아니라 현재 Server 계약 `hitTimeMs`**. ACTIVE는 skill `hitTimeMs`, COMBO/HOLD/COUNTER는 `comboStages[i].hitTimeMs>0`인 stage마다. 스킬(stage) 시간 → clip 로컬 시간은 `build_hitshapes.stage_hits`와 같은 산술(`.wmodel` ticks/30, playMs 상한, playRate)의 역산. 타이밍이 지금과 동일하므로 순수 additive |
| 셰이프 | 참조 행의 히트 중 `timed=1`을 우선, 없으면 전체에서 `g=0` 우선으로 **첫 area>0 행 하나** (같은 t의 여러 행은 트라이포드/그룹 변형). push/pushr도 그 행 값 |
| endms | `startms + w`(timed) — w는 파생값이지만 유일한 원본 폭. Debug 와이어 가시성은 Client 최소 표시 폭 300 ms로 보완 |
| 태그 | `src=orig` (Animation Tool 호환). Tool에서 해당 클립 `Import original`을 다시 하면 이 행도 지워지고 notify만 남는다 — 스크립트 재실행으로 복구 |
| 재실행 | `rewrite()`가 `src=orig` HIT를 클립 단위로 교체하므로 idempotent |
| chain 밖 | hitTimeMs가 chain 총 길이 이상이면 마지막 클립 끝에 두고 경고 출력 |

## 변경 파일

- `Tools/CharacterAnimationIntake/fill_animevents_hit_shapes.py`: `load_skilltiming`이 `timed`/`g` 보존,
  `synthesize_timed_rows(asset, notify)`가 skillbindings + PlayerSkills + CharacterCatalog body `.wmodel`
  ticks(`build_hitshapes.read_clip_ticks` import)로 클립별 HIT 행 생성, `build_rows`가 notify 결과에 합침.
- `Client/Private/Character.cpp` `Draw_SkillHitAreaDebug`: `fWidthMs = max(fWidthMs, 300)` (Debug 전용).
- `Data/Animation/Authored/{LanceMaster,Warlord,Artist,DimensionMaster}/*.animevents`,
  `Data/Animation/HitShapes/*.hitshapes.json`: 재생성물.

## 검증

fill 4직업 → build_hitshapes 4직업 → `Publish-GameplayBalance.ps1 -Mode Validate/Publish` → Server 빌드 →
`Server.exe --contract-test` 0 fail → Client Debug 빌드. 수동: Character Select에서 창술사 T/ALT_V/단창 S 와이어와
셰이프 판정 확인(사용자).

## 추가 G (2026-08-18 오후): 원본 `AreaType` 의미 정정 — 사용자 관찰 "전체적으로 원작보다 넓다"

### 실측 (`EFTable_SkillEffect.db`, AreaRange>0 행)

| AreaType | 행 수 | AreaAngle>0 | AreaAngle 최대 | AreaRemoveRange>0 |
|---|---|---|---|---|
| 1 | 74,668 | 1,785 | — | 2,276 |
| 2 | 51,004 | 50,904 | 6000 | 526 |
| 3 | 27,011 | 26,986 | 360 | 5,195 |

- 1 = 원(링은 `AreaRemoveRange`), 2 = 전방 박스(`AreaAngle` = 폭 cm, 최대 6000이라 각도 불가), 3 = 부채꼴(도, ≤360).
- 예: 나선창 34540 `2/320×140` = 3.2 m×1.4 m 찌르기, 적룡포 34590 `2/800×170`, 한획긋기 31470 `2/600×300`,
  건너찌르기 2050160 `2/450×120 ax=-225`, 맹룡난무 34640 `1/320` 원, 도화가 LMB 31000 `3/230/160°`.
- #110의 프로젝트 매핑은 `1=box(폭=AreaHeight), 2=fan, 3=circle+cone`이라 가장 많은 type 2 박스가 폭 cm를 각도로
  읽은 부채꼴로 판정·표시됐다.

### 변경

| 파일 | 내용 |
|---|---|
| `build_hitshapes.py` | `formatVersion` 1→2. `angle`은 type 3만(도, 0~360 clamp), 새 `width`(m)는 type 2만(`aa`×0.01). |
| `Publish-GameplayBalance.ps1` | hit 필드에 `width` 추가, 타입별 extent 검증(2는 width>0·inner=0, 3 외 angle=0, 2 외 width=0), SKILLHIT packed 12→13(`…:angle:width:height:…`), 헤더 `LOSTARK_GAMEPLAY_BOOTSTRAP 8`. |
| `Server/Public/GameplayCatalog.h` | `PLAYER_SKILL_HIT::fWidth`. |
| `Server/Private/GameplayCatalog.cpp` | 13필드 파싱·검증, bootstrap 8만 허용. |
| `Server/Private/PlayerSkillSystem.cpp` `Hit_ShapeOverlaps` | 1→`Circles_Overlap`/`Circle_IntersectsRing`, 2→`Circle_IntersectsForwardBox(range, width/2)`, 3→기존 ring∧cone. |
| `ServerGameplayContractTests.cpp` | bootstrap fixture 7→8, obsolete 6→7. |
| `Client/Private/HitAreaWire.cpp` | case 2 박스(폭=`iAreaAngle` cm), case 1/3 arc(1은 항상 360, inner 링). |
| `Client/Private/Animation_Tool.cpp`, `.h` | `Area_Name` 1 circle/2 box/3 fan, box는 `width` 슬라이더, inner는 1/3. |
| `CLAUDE.md` | 셰이프 계약 문장 갱신. |

`.animevents` 원본 필드는 그대로이며 `hitshapes.json` 4직업만 재생성한다.

