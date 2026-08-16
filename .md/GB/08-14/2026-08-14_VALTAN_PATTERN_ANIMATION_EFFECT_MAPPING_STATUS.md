# 2026-08-14 발탄 패턴·애니메이션·Effect 매핑 현황

## 1. 문서 역할과 결론

이 문서는 사용자가 제공한 나무위키 `주의 패턴` 발췌를 현재 프로젝트의 발탄
encounter, source Action, model clip, Effect 저작 상태와 대조한 **2026-08-14 audit
snapshot**이다. snapshot의 merged baseline은 commit
`d5270a69ffb9db9ba2a34494ba98974393a63bd7`이며, 다른 세션이 merge 준비 중인 pending Track A
worktree는 별도 층으로 표시한다. 이 문서를 merged 제품 정본이나 구현 완료 RESULT로 사용하지
않는다. 2026-08-13 corpus RESULT와 2026-08-06 패턴 문서의 당시 실행 증거는 역사 기록으로
유지하되, 현재 encounter 분모와 drift 평가는 이 문서가 대체한다.

현재 결론은 다음과 같다.

- Server encounter는 `31 patterns / 117 semantic stages / 63 source Action IDs`다.
- `Valtan.patternbindings.json`은 112행이지만 현재 encounter를 덮는 행은 `108/117`이다.
  `VALTAN_LEDGE_ROAR` 3단계와 `VALTAN_ARENA_BREAK_109` 6단계가 비어 있고,
  폐기된 `ARENA_BREAK_80` 4행이 orphan으로 남아 있다.
- 기존 112행이 참조하는 unique model clip 58개는 모두 실제 Valtan AnimSet에 존재한다.
  기존 행의 문제는 물리 clip 부재가 아니라 semantic stage·branch·source stage 정확성이다.
  새 `ARENA_BREAK_109`처럼 아직 binding이 없는 stage의 전용 clip 부재는 별도 문제다.
- pending Track A worktree에서 source Action 420633 `레이드 발탄_휠윈드` stage 2, runtime clip과 authored binding
  identity까지 위치가 고정된 것은 `VALTAN_WHIRLWIND / SPIN` 한 건이다. 다만 preceding branch
  condition payload가 unresolved이므로 이 한 건도 이 계획의 `SOURCE_EXACT` gate를 통과하지 않았다.
- merged HEAD에는 Valtan pattern Effect mapping과 boss-owner Effect service가 없다. pending
  Track A worktree에는 Whirlwind 한 binding과 boss-owner service seam이 있으며, 9 carrier 중
  3개만 visible executable이고 6개는 fail-closed다. 두 층 모두 `EffectCatalog` 제품 entry와
  `CValtan` gameplay consumer는 0이다.
- 제공 자료는 발탄의 모든 일반 패턴과 망령 페이즈 전체가 아니라 `주의 패턴` 구간이다.
  따라서 자료에 없는 동작을 같은 이름이나 비슷한 HP bar만으로 자동 합치지 않는다.
- 나무위키 이름과 HP bar는 검색·설명용 guide reference다. runtime 정본은 기존 stable
  `patternId / stageId / actionId`와 Server encounter다.

이 문서는 구현 완료나 visual PASS를 선언하지 않는다. Client 화면과 Effect fidelity는
사용자가 직접 확인하고 서면 승인해야 한다.

## 2. 정본과 증거 우선순위

| 우선순위 | 자료 | 역할 |
|---:|---|---|
| 1 | `Data/Encounters/Valtan/ValtanEncounter.json` | HP gate, pattern, stage, action, damage shape와 Server gameplay 정본 |
| 2 | `Data/Animation/Reference/Valtan/Valtan.clipseq`와 imported Action report | 원본 Action·branch·clip·source stage 후보 증거 |
| 3 | `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | 현재 Client animation presentation mapping |
| 4 | `Data/Animation/Authored/Valtan/Valtan.patterneffects.json` | pattern stage와 authored Effect occurrence의 mapping |
| 5 | `Data/Effects/Imported/Valtan/*` | ParticleSystem, material, WModel, texture와 notify source evidence |
| 6 | 사용자 제공 나무위키 발췌 | 한국어 표시명, 통용 별칭, 원작 mechanic 순서의 비교 기준 |

나무위키의 `_노멀 버전` 표기와 `ValtanEncounter.json`의 `selectionMode: NORMAL`은 같은
데이터 계약이 아니다. 난이도·branch가 확인되지 않은 occurrence는 `미확정`으로 둔다.

## 3. 현재 실측

### 3.1 Encounter와 animation

| 항목 | 현재 값 | 판정 |
|---|---:|---|
| Encounter patterns | 31 | 현재 정본 |
| Encounter semantic stages | 117 | 08-13 문서의 115를 대체 |
| Unique source Action IDs | 63 | 현재 정본 |
| Pattern binding rows | 112 | 파일의 물리 행 수 |
| Current encounter-covered rows | 108/117 | missing 9 |
| Stale orphan rows | 4 | `arena-break-80.*` |
| Binding unique clips | 58 | 58/58 AnimSet에 존재 |
| Valtan body model clips | 27 | 물리 모델 실측 |
| Valtan AnimSet clips | 146 | 물리 AnimSet 실측 |
| Pending source-stage located + authored binding identity | 1/117 | Whirlwind `SPIN`; branch condition unresolved |
| Pending `SOURCE_EXACT` | 0/117 | Action/stage/branch/clip/occurrence 전체 gate 기준 |

누락 9행은 다음과 같다.

- `valtan.attack.ledge-roar.{windup,active,recovery}`
- `valtan.mechanic.arena-break-109.{takeoff,drop,impact,impact-hold,wide-reveal,recovery}`

stale 4행은 다음과 같다.

- `valtan.mechanic.arena-break-80.{cutscene,landing,spin,recovery}`

`ARENA_BREAK_109`는 2026-08-14에 `ARENA_BREAK_80`을 대체한 project-tuned pattern이다.
Server가 root transform arc를 권위 있게 이동하며 전용 jump clip은 없다. 한편 source Action
420629에는 `mesh_att_battle_12_01/02/03` skeletal candidate가 있다. root motion 권위와
skeletal animation 선택은 서로 다른 축이므로, 6단계에 세 clip을 임의 복제해서 채우거나
Server transform을 이유로 사용 가능한 animation candidate를 자동 폐기하면 안 된다.

### 3.2 Effect source와 snapshot layer

| 항목 | merged HEAD | pending Track A worktree |
|---|---:|---:|
| ParticleSystem source graphs | 193 audit inventory | 193, merged HEAD와 동일 |
| Material bindings | 335 audit inventory | 335, merged HEAD와 동일 |
| WModels | 52 audit inventory | 52, merged HEAD와 동일 |
| Textures | 346 audit inventory | 346, merged HEAD와 동일 |
| `Valtan.patterneffects.json` | 없음 | Whirlwind binding 1 |
| Whirlwind carrier denominator | 없음 | 9 |
| Visible executable carriers | 없음 | 3 |
| Fail-closed carriers | 없음 | 6 |
| Valtan EffectCatalog product entries | 0 | 0 |
| `CValtan` product effect consumers | 0 | 0 |
| Boss-owner Effect service seam | 없음 | `pBossOwner`, `Stop_BossOwner` pending |
| Whirlwind canary Python test | 해당 파일 없음 | 14/14 PASS, product/visual PASS 아님 |

merged HEAD와 pending worktree의 `CValtan` 모두 animation binding만 소비하며
`Valtan.patterneffects.json`을 로드하거나 `CEffectPresentationService::Spawn`을 호출하지 않는다.
pending service seam에는 boss owner, initial sample age, prewarm/budget와 `Stop_BossOwner`가 있다.
현재 중복 억제는 active/pending Effect의 `(owner, actionStartTick, occurrenceId)` 비교이며,
완료 후 reconnect/replay까지 보장하는 durable dedupe는 아니다. 후속 구현과 harness가 이 경계를
닫아야 한다.

## 4. guide phase와 이름 정책

아래 phase는 제공 자료를 읽기 위한 taxonomy다. 현재 Server enum으로 간주하거나 곧바로
저장 ID로 쓰지 않는다.

| Guide phase | 의미 |
|---|---|
| `ARMORED_ENTRY` | 조우 시작과 갑옷 파괴 오프닝 |
| `OPEN_ARENA` | 외벽 전환 이후 첫 지형 파괴 전 |
| `PARTIAL_DESTRUCTION` | 첫 지형 파괴 이후 두 번째 지형 파괴 전 |
| `FINAL_PLATFORM` | 두 번째 지형 파괴 이후 최종 발악 전 |
| `DESPERATION_TRANSITION` | 원혼의 일격과 육신 붕괴 |
| `GHOST` | 망령 상태. 제공 자료에는 고정 돌진 잡기만 일부 설명됨 |

정식 한국어 표시명을 기본으로 사용하고 `임포스터`, `3카`, `도끼자루 안전`, `버러지`,
`1지파`, `2지파`, `연환파신권`, `영혼 페이즈`는 alias 검색어로만 둔다. runtime stable ID는
변경하지 않는다.

Action 420633의 이름 authority는 main PR #103 (`28fa75a2`)의
`Data/Animation/Reference/Valtan/Valtan.skilltiming`이며 source commit `72cc7629`가
`레이드 발탄_휠윈드`를 도입했다. `99b5e60e`의 `42063301/02/03`은 판정 shape reference이고
visual carrier가 아니다.

## 5. 제공 패턴과 현재 runtime crosswalk

판정 용어는 다음과 같다. Action/animation family 관계와 gameplay mechanic 관계는 서로 다른
축으로 판정한다.

- `EXACT_FAMILY`: 이름·동작과 원본 Action/animation family가 직접 대응한다. source stage exact나
  HP gate·hit shape·sequence 일치를 뜻하지 않는다.
- `CANDIDATE`: 형태는 강하게 대응하지만 HP bar, branch 또는 후속 sequence가 다르다.
- `PARTIAL`: 일부 동작만 현재 contract에 있다.
- `UNMAPPED`: 현재 encounter·state·world contract에 대응 항목이 없다.

| Guide 패턴 | Guide bar/phase | 현재 runtime 대응 | Action/animation 관계 | Gameplay gate/shape/sequence 관계 | Effect와 gameplay 경계 |
|---|---|---|---|---|---|
| 발탄의 갑옷 | 시작, 120 전환 | `VALTAN_ARMOR_BREAK_OPENING` 159 | 부위파괴 family `PARTIAL` | stack/buff `UNMAPPED` | 2 stack, 10/15% 감소, 개방 갑옷, 30초 주기 마수의 힘은 현재 미매핑 Server state다. |
| 조우 고정 시퀀스 | 입장 직후 | `WHIRLWIND` 420633, `DASH_CHARGE` 420604, `JUMP_SPIN` 420621/663; `HIGH_JUMP`, `SUPER_SMASH` 후보 | 구성 clip family `PARTIAL` | fixed sequence `UNMAPPED` | 고정 순서·반복·벽 target·로나운 lifecycle이 없다. 2페이즈 이후 일반 돌진의 두 번째 stomp counter window도 별도 variant로 빠져 있다. |
| 강력한 일격 | 130 | `VALTAN_FLOOR_WIPE_130` 130, Action 420630 | `EXACT_FAMILY` | `PARTIAL`: guide 6방향과 runtime `CROSS` 4축이 다름 | 두 타격 순서는 후보지만 shape가 다르다. 로나운·바훈투르 보호는 별도 player/buff mechanic이다. |
| 외벽 파괴 전환 | 120 | 독립 120 pattern 없음. 가장 가까운 것은 project-tuned `ARENA_BREAK_109` | `UNMAPPED` | `UNMAPPED` | 120을 109의 alias로 자동 등록하지 않는다. 벽 제거는 world mutation이고 Effect는 debris/impact만 표현한다. |
| 도약 충격파 | 110, 임포스터 | `VALTAN_FOUR_PILLARS_105` + `HIGH_JUMP` | `CANDIDATE` | `PARTIAL`: 110↔105, guide 90도 초과 cone↔runtime 45도 | 6방향 전조, 후속 cone, pillar hit/unhit 폭발과 actor lifetime이 빠져 있다. |
| 1차 지형 파괴 | 85, 1지파 | `ARENA_BREAK_109`, `WHIRLWIND`, `FOUR_PILLARS_105`, `LEDGE_ROAR`가 구성 후보 | 구성 family `CANDIDATE` | sequence `UNMAPPED` | 85↔109 불일치. 좌우 선택·낙사·비석·포효의 deterministic chain이 없다. |
| 3연속 카운터 | 55~30, 3카 | `VALTAN_TRIPLE_COUNTER`, Actions 420640~647 | `EXACT_FAMILY` | `PARTIAL`: runtime 선택 범위 90~30, round branch 불완전 | 현재 animation은 groggy clip으로 과도하게 단순화돼 실제 420642~647 attack branch가 사라진다. Hard 원혼불은 별도 variant다. |
| 반격기 | 첫 전멸기 이후, 도끼자루 안전 | `VALTAN_PARRY`, Actions 420606/607 | `EXACT_FAMILY` | `PARTIAL`: runtime은 1~160 일반 선택, threshold branch 없음 | 성공 branch `mesh_att_battle_9_01_end-2`가 빠져 있다. stagger threshold/분기부터 Server가 소유해야 한다. |
| 돌진 잡기 | 지파 이후 일반, 망령 39.5/26/13 | `VALTAN_CHARGE_GRAB_ROAR`, 일반 Actions 420623/631/632 | 일반 `EXACT_FAMILY`, 망령 `UNMAPPED` | 일반 `PARTIAL`, 망령 `UNMAPPED` | runtime 일반 pattern은 1~160에서 선택된다. 일반 이동형과 망령 고정형을 variant로 분리하고 target/grab은 Server가 판정한다. |
| 짓뭉개기 | 65, 버러지 | `VALTAN_CENTER_GRAB_COUNTER_64` | `CANDIDATE` | `PARTIAL`: 65↔64, 실패 loop 없음 | player별 장판, 짧은 counter, 실패 반복·재표적·wipe branch가 충분히 표현되지 않는다. |
| 2차 지형 파괴 | 30, 2지파 | `VALTAN_ARENA_BREAK_33` | `CANDIDATE` | `PARTIAL`: 30↔33, opposite-side state 없음 | 반대편 선택과 최종 platform commit은 Server world state다. |
| 원혼의 일격 | 15/16, 연환파신권 | `VALTAN_GHOST_TRANSITION_15` | broad `CANDIDATE` | `PARTIAL`: 순차 부채꼴 대신 runtime `CROSS`, cadence/actor 부족 | portal·추적·내외곽은 일부 대응한다. 연속 주먹, 3기둥, 포효, 붕괴와 encounter phase end는 6 stage로 부족하다. |

## 6. 31개 runtime pattern의 animation 매핑

clip 표에서는 공통 `mesh_` prefix를 생략했다. `candidate`는 clip이 실제 AnimSet과 해당
source Action family에 존재한다는 뜻이며 source stage·branch exact를 보증하지 않는다.

| Pattern / 표시명 | Source Actions | 현재 stage → clip | 현재 판정 |
|---|---|---|---|
| `VALTAN_SWING` / 휘두르기 | 420601, 420660 | WINDUP `att_battle_1_01`; SWEEP `att_battle_1_01`; RECOVERY `att_battle_1_02` | source-family candidate |
| `VALTAN_DOWN_SMASH` / 내려찍기 | 420602, 420661 | WINDUP `att_battle_2_01`; IMPACT `att_battle_2_03`; RECOVERY `att_battle_2_03` | candidate |
| `VALTAN_IMPRISON_ROAR` / 감금 사자후 | 420603 | WINDUP `5_01_start`; ROAR `5_01_loop`; RECOVERY `5_01_end` | ordered candidate |
| `VALTAN_DASH_CHARGE` / 대쉬 돌진 | 420604 | WINDUP `att_battle_4_01`; CHARGE `att_battle_4_01`; RECOVERY `att_battle_4_02` | strong ordered candidate |
| `VALTAN_EARTHQUAKE_SMASH` / 지진 찍기 | 420605, 420662 | WINDUP `att_battle_7_01`; IMPACT `att_battle_7_03`; DELAYED `att_battle_7_01`; RECOVERY `att_battle_7_03` | candidate |
| `VALTAN_PARRY` / 큰 베기 반격 | 420606, 420607 | STANCE `att_battle_9_01_start`; SLASH `att_battle_9_01_loop`; RECOVERY `att_battle_9_01_end` | branch gap: success `att_battle_9_01_end-2` 미매핑 |
| `VALTAN_MAGIC_CHOICE` / 마력기운 양자택일 | 420608 | WINDUP `att_battle_5_02_start`; INNER/OUTER `att_battle_5_02_loop`; RECOVERY `att_battle_5_02_end` | branch gap: inner/outer evidence 없음 |
| `VALTAN_FOUR_SLASH` / 4연속 베기 | 420609 | WINDUP `att_battle_10_01`; SLASHES/RECOVERY `att_battle_10_02` | candidate |
| `VALTAN_HIGH_JUMP` / 고공 점프 찍기 | 420610 | TAKEOFF `att_battle_8_01_start`; AIRBORNE `att_battle_8_01_loop`; LAND/RECOVERY `att_battle_8_01_end` | strong ordered candidate |
| `VALTAN_STOMP` / 발구르기 | 420611 | 전 단계 `att_battle_11_01` | coarse candidate |
| `VALTAN_BIND_CHARGE_SMASH` / 구속 돌진 잡기 후 내려찍기 | 420612~614 | LOCK `att_battle_12_01`; CHARGE `att_battle_13_01`; SMASH `att_battle_13_03`; RECOVERY `att_battle_12_02` | candidate |
| `VALTAN_GROUND_WAVE_SMASH` / 지진파 내려찍기 | 420615 | WINDUP `att_battle_1_01`; WAVE `att_battle_15_01`; RECOVERY `att_battle_15_05` | candidate |
| `VALTAN_SUPER_SMASH` / 초강력 내려찍기 콤보 | 420619, 420620, 420656, 420657 | WINDUP `att_battle_12_01`; IMPACTS `att_battle_12_02`; RECOVERY `att_battle_12_05` | coarse; source branch 다수 누락 |
| `VALTAN_JUMP_SPIN` / 점프 찍기 후 휠윈드 | 420621, 420663 | JUMP `att_battle_20_01`; LAND `att_battle_20_02`; SPIN `att_battle_20_03`; RECOVERY `att_battle_20_04` | strong ordered candidate |
| `VALTAN_PORTAL_RUSH` / 워프 돌진 콤보 | 420622 | PORTAL `att_battle_18_01`; RUSHES `att_battle_18_02`; FINISH `att_battle_18_03-1`; RECOVERY `att_battle_18_03-2` | strong ordered candidate |
| `VALTAN_CHARGE_GRAB_ROAR` / 돌진 잡기 후 사자후 | 420623, 420631, 420632 | WINDOW `att_battle_21_01`; CHARGE `att_battle_21_02`; ROAR `att_battle_21_03`; RECOVERY `att_battle_21_04-1` | strong family candidate; success/fail branch 필요 |
| `VALTAN_WHIRLWIND` / 레이드 발탄_휠윈드 | 420633 | WINDUP `att_battle_20_02`; SPIN `att_battle_20_03`; RECOVERY `att_battle_20_04` | strong ordered; SPIN authored identity만 위치 고정, branch unresolved |
| `VALTAN_BACKSTEP_ATTACK` / 공격하며 뒤로 빠지기 | 420635, 420664 | WINDUP `att_battle_20_03`; SWEEP `att_battle_20_02`; RECOVERY `att_battle_7_03` | source-family candidate |
| `VALTAN_RED_BLADE_WAVE` / 붉은 검기 | 420636 | WINDUP `att_battle_9_01_start`; PROJECTILE `att_battle_12_10`; RECOVERY `att_battle_12_11` | candidate |
| `VALTAN_FRONT_BACK_FRONT` / 앞뒤앞 내려찍기 | 420637, 420666 | WINDUP `att_battle_19_01`; SMASHES `att_battle_19_06`; RECOVERY `att_battle_2_03` | candidate |
| `VALTAN_FIST_IN_OUT` / 두 손 내려찍기 안밖 폭발 | 420638 | WINDUP `att_battle_19_02`; INNER/OUTER `att_battle_19_04`; RECOVERY `att_battle_1_01` | coarse; inner/outer branch 없음 |
| `VALTAN_LEDGE_ROAR` / 낙사 사자후 | 420639 | WINDUP/ROAR/RECOVERY `MISSING` | 3/3 missing; evt1 start/end/loop 순서 검증 필요 |
| `VALTAN_TRIPLE_COUNTER` / 연속 카운터 내려찍기 | 420640~647 | groggy start/loop/end로 7단계 구성 | semantic-incorrect; 실제 attack `14_01~14_04-*` branch 필요 |
| `VALTAN_ARMOR_BREAK_OPENING` / 오프닝 외벽 충돌 부위 파괴 | 420627, 420628, 420654, 420655 | parts start/loop/end | incomplete; wall charge·opening attack 의미를 표현하지 못함 |
| `VALTAN_FLOOR_WIPE_130` / 130줄 전멸기 | 420630 | `att_battle_1_01 → att_battle_1_02 → att_battle_5_02_loop → att_battle_5_02_end → att_battle_15_04` | strong family candidate; sourceActionStageIndex 필요 |
| `VALTAN_FOUR_PILLARS_105` / 105줄 4기둥 추적 원뿔 | 420610 | `att_battle_8_01_start → att_battle_8_01_loop → att_battle_8_01_end → att_battle_8_01_loop` | candidate; HIGH_JUMP와 Action 공유 |
| `VALTAN_ARENA_BREAK_109` / 109줄 아레나 붕괴 | 420629 | 6단계 모두 `MISSING` | Server-transform-only 설계 필요; guessed clip 금지 |
| `VALTAN_MAGIC_ORB_STAGGER_76` / 76줄 마력구 무력화 | 420617, 420618 | groggy start/loop/end | candidate |
| `VALTAN_CENTER_GRAB_COUNTER_64` / 64줄 잡기 카운터 | 420623, 420631 | `att_battle_21_01 → att_battle_21_02 → att_battle_21_03 → att_battle_21_04 → att_battle_21_04-1` | family candidate; branch 필요 |
| `VALTAN_ARENA_BREAK_33` / 33줄 2차 지형 파괴 | 420629 | `att_battle_12_01 → att_battle_12_02 → att_battle_12_02 → att_battle_12_03` | strong sequence candidate; duplicated stage evidence 필요 |
| `VALTAN_GHOST_TRANSITION_15` / 15줄 망령화 | 11 Actions | `abn_groggy_*`, `att_battle_5_01_*`, `att_battle_5_03`를 6단계에 압축 | semantic-incorrect/coarse; source branch 재구성 필요 |

## 7. Effect occurrence의 소유자 분류

같은 source Action에 포함돼도 occurrence의 소유자와 생명주기가 다르면 하나의 boss Effect로
합치지 않는다.

| 분류 | 예 | 구현 소유자 |
|---|---|---|
| `BOSS_BONE_FOLLOW` | 도끼 trail, 손·무기 hit glow | `CValtan` + 기존 Effect presentation boss anchor |
| `BOSS_ROOT_FOLLOW` | 갑옷 aura, 포효 aura | boss owner를 따라가는 지속 Effect |
| `BOSS_ROOT_SNAPSHOT` | 착지 충격, 그 시점의 바닥 균열 | authoritative action tick에서 world transform을 snapshot |
| `TARGET_ENTITY_FOLLOW` | 대상 추적 표식 | Server가 확정한 target entity를 따라가는 presentation |
| `WORLD_TRANSFORM_SNAPSHOT` | player 발밑 장판, 고정 cone, 충격파 origin | Server가 전달한 stable occurrence와 world transform |
| `WORLD_ACTOR_EXTERNAL` | 비석, 기둥, 원혼불, 로나운 구체 | Server world entity spawn/snapshot/despawn |
| `WORLD_MECHANIC_EXTERNAL` | 벽·지형 파괴, 낙사 영역, navigation commit | 기존 world destruction/replication runtime |
| `STATUS_PERSISTENT` | 갑옷 stack, 개방 갑옷, 마수의 힘 | Server status revision과 시작/종료 event |
| `CAMERA_EXTERNAL` | 컷신, 줌, 화면 흔들림 | cinematic/camera 계약. Effect asset이 gameplay camera를 주도하지 않음 |
| `VISUAL_EMPTY` | 원본상 시각 occurrence가 없는 semantic stage | 명시적 no-effect row; generic fallback 금지 |

## 8. 현재 열린 작업

1. 117개 semantic stage를 `(patternId, stageId, actionId)` triple로 current encounter와
   structural exact join한다.
2. animation row마다 `exact clip`, `candidate-preserved`, `deferred`, `visual-empty`를 명시하고,
   별도 `motionAuthority`로 Server transform을 기록한 뒤 stale 80 행을 제거한다.
3. `PARRY`, `TRIPLE_COUNTER`, `ARMOR_BREAK_OPENING`, `GHOST_TRANSITION_15`의 branch와
   source stage를 먼저 재구성한다.
4. boss-bone 전용인 현재 `patterneffects` schema를 target/world/status occurrence까지 표현하는
   typed mapping으로 확장한다.
5. `CValtan`을 기존 `CEffectPresentationService`에 연결하되 visual 실패가 Server snapshot 적용을
   막지 않게 한다.
6. 벽·지형·비석·기둥·grab/counter처럼 mechanic과 결합된 항목은 Server authority 수직
   슬라이스로 분리한다.
7. candidate 생성과 제품 mapping을 분리하고 occurrence별 사용자 visual 승인 뒤에만
   `EffectCatalog`와 boss runtime mapping을 승격한다.

상세 순서와 완료 gate는
`2026-08-14_VALTAN_PATTERN_ANIMATION_EFFECT_IMPLEMENTATION_PLAN.md`에서 정의한다.

## 9. 역사 문서와의 관계

- `2026-08-13_VALTAN_EFFECT_CORPUS_AND_PROJECT_AUDIT_RETIREMENT_RESULT.md`의
  `31/115`와 corpus 수치는 당시 실행 증거다. 현재 stage 분모만 이 문서의 `31/117`로 supersede한다.
- `2026-08-06_VALTAN_COMPLETE_ACTION_TUNING_REVIVE_RESULT.md`와 패턴 balance RESULT의
  80줄 arena break는 현재 `ARENA_BREAK_109` 도입 전 역사 상태다.
- `2026-08-14_FOUR_CLASS_VALTAN_EFFECT_RESTORATION_AND_RAID_PERFORMANCE_IMPLEMENTATION_PLAN.md`
  의 Whirlwind `WWind 3 + Dust 2` slice는 별도 세션 소유다. 본 문서는 그 산출물을 회귀
  baseline으로 계승하며 재구현하거나 자동 제품 승격하지 않는다.
