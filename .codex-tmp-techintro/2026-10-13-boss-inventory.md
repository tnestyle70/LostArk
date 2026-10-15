## 2026-10-13 보스 패턴 전체 범위와 현재 데이터 목록

조사일 2026-10-13. LostArk 기준 HEAD는 `01518032dde86b5558976a2fbe5fd94ad625cfd1`, 작업 브랜치는 `codex/warlord-altv-radial-render-fix`다. 작업 폴더에는 기존 미커밋 변경이 있으며 아래 내용은 조사 당시 디스크 저장본과 현재 소스를 기준으로 한다. 과거 commit의 파일 내용이나 행 번호와 동일하다고 주장하지 않는다.

여기서 “게시 행 있음”은 현재 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`에서 해당 PATTERN ID를 찾았다는 뜻이다. 실행 중 Server가 그 파일을 승인·로드했는지, 자동 흐름에 진입했는지, 해당 공격이 실제 명중하거나 Client 화면에서 올바르게 표시됐는지는 이번 조사로 확인하지 않았다. 아래 목록에는 공격 외에 등장·사망 연출, 추적, 성공·실패 후속, 테스트와 호환 정의가 포함되므로 행 수를 독립 전투 기믹 수로 표현하지 않는다.

### 전체 범위

현재 BossProfiles 및 게시 bootstrap의 BOSS 행에는 발탄·유령 발탄, 쿠크 1관문의 쿠크·세이튼, 2관문의 대형 세이튼·쿠크, 3관문 세이튼, 빙고의 앵콜 세이튼까지 8개 archetype이 있다. 쿠크의 6개 archetype은 `ENCOUNTER_KAKULSAYDON_G1`이라는 공용 encounter를 참조한다. 문자열 `G1`이나 저장 폴더 `Gate1`만으로 실제 관문을 추정하지 않고 각 pattern의 `gateId`, `targetBossPlacementId`, `actorProfileId`와 실행 Flow를 함께 읽는다.

| 범위 | 저장/생성 정의 | 게시 PATTERN 행 | 구분할 점 |
| --- | --- | --- | --- |
| 발탄·유령 발탄 | gameplay 44개; Encounter 67개 | 67개 | 저작 44개 외 23개는 legacy compatibility 쪽 정의. compatibility 파일 25개 중 retiredPatternIds에 든 2개는 현재 Encounter에서 제외됐다. |
| 쿠크 전체 | Composition 121개; Encounter 114개 | 114개 | 저장 상태 DRAFT/PRODUCT와 게시 가능 여부는 별개다. |
| 루가루 | MonsterProfiles의 MINIBOSS_LUGARU | 별도 보스 PATTERN 없음 | 공용 CMonsterBrain의 추적·기본 공격을 사용한다. |

### 기술소개서 본문으로 압축할 설명

발탄과 쿠크세이튼은 보스별 저작 형식에서 출발하지만, 게시된 gameplay 정의와 Server가 소유하는 실행 상태를 분리하는 공통 구조로 연결된다. 발탄은 순서 있는 stage와 판정 schedule, 월드 이벤트 및 체력 구간별 진행을 사용한다. 쿠크는 재사용 Logic 정의와 occurrence의 시각·공간 배치를 결합해 서버용 Logic window를 만들고, 관문 Flow에서 Pattern·Bundle·기믹을 진행한다. Server가 판정과 상태 변화를 확정하고 Client는 전달된 상태와 게시 표현 데이터를 사용한다. 데이터화는 새 C++ 로직을 자동 생성하는 기능이 아니며, 지원한 enum·필드·결과를 기존 실행 코드가 해석하는 구조다.

발탄 대표 사례는 WHIRLWIND의 SPIN 판정 schedule과 소비 횟수다. 정의에는 1200ms 동안 0·350·700·1050ms의 네 판정 기회가 있고, 보스 인스턴스는 처리한 기회 수를 기억한다. 쿠크 대표 사례는 `KAKULSAYDON_G1_PATTERN_109`의 Logic occurrence다. 1122ms에 시작해 112ms 지속하는 배치를 reusable Logic과 Collider에 연결하며, Server는 30Hz에서 시작 경계 34tick·종료 경계 38tick으로 변환한다. 이 숫자는 데이터와 코드의 정적 계산이며 이번 실행 측정값이 아니다.

저장→projection→게시 파일→Server 승인된 정의→실행→Client 화면은 별도 경계다. 쿠크는 편집 메모리 draft를 검증한 runtime row로 보내 공용 parser로 실행하는 audition 경로도 있으므로, 디스크 파일을 읽는 제품 경로와 구분해서 설명한다. 구현의 존재, 이번 정적 조사, 사용자 breakpoint 확인, 당시 개인 기여와 성능 결과는 각각 다른 근거로 남긴다.

### 발탄 전체 정의 67개

출처: [Data/Valtan/Valtan.gameplay.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json), [Data/Valtan/Valtan.legacy-compatibility.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.legacy-compatibility.json), [Data/Encounters/Valtan/ValtanEncounter.json](C:/Users/tnest/Desktop/LostArk/Data/Encounters/Valtan/ValtanEncounter.json). “저작”은 현재 gameplay 정본에 존재, “호환”은 현재 gameplay에는 없고 legacy compatibility에 존재한다. `selectionMode`는 Encounter 필드 원문이며 이 값만으로 현재 자동 전투에서 선택되는 것을 보장하지 않는다. 현행 저작 `decisionModel.scriptedSequence.mode`는 `HEALTH_BAR_ROTATIONS`다.

| patternId | 표시 이름 | 정의 출처 | stage 수 | selectionMode | 게시 행 |
| --- | --- | --- | ---: | --- | --- |
| VALTAN_SWING | 휘두르기 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_DOWN_SMASH | 내려찍기 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_IMPRISON_ROAR | 감금 사자후 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_DASH_CHARGE | 3회 땅 치기 후 돌진 | 저작 | 3 | NORMAL | 있음 |
| VALTAN_EARTHQUAKE_SMASH | 지진 찍기 | 호환 | 4 | NORMAL | 있음 |
| VALTAN_PARRY | 큰 베기 반격 | 호환 | 4 | NORMAL | 있음 |
| VALTAN_MAGIC_CHOICE | 마력기운 양자택일 | 호환 | 4 | NORMAL | 있음 |
| VALTAN_FOUR_SLASH | 4연속 공격 | 저작 | 4 | NORMAL | 있음 |
| VALTAN_HIGH_JUMP | 점프 후 플레이어 추적 도끼 | 저작 | 4 | NORMAL | 있음 |
| VALTAN_STOMP | 발구르기 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_BIND_CHARGE_SMASH | 구속 돌진 잡기 후 내려찍기 | 호환 | 4 | NORMAL | 있음 |
| VALTAN_GROUND_WAVE_SMASH | 지진파 내려찍기 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_SUPER_SMASH | 초강력 내려찍기 콤보 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_JUMP_SPIN | 점프 찍기 후 휠윈드 | 호환 | 4 | NORMAL | 있음 |
| VALTAN_PORTAL_RUSH | 워프 돌진 콤보 | 호환 | 4 | NORMAL | 있음 |
| VALTAN_CHARGE_GRAB_ROAR | 돌진 잡기 후 사자후 | 호환 | 4 | NORMAL | 있음 |
| VALTAN_WHIRLWIND | 휠윈드 1.2초 | 저작 | 3 | NORMAL | 있음 |
| VALTAN_BACKSTEP_ATTACK | 공격하면서 뒤로 빠지기 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_RED_BLADE_WAVE | 붉은 검기 날리기 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_FRONT_BACK_FRONT | 앞뒤앞 내려찍기 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_FIST_IN_OUT | 도넛 공격 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_LEDGE_ROAR | 낙사 사자후 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_TRIPLE_COUNTER | 3연속 내려치기 - 카운터 | 저작 | 7 | NORMAL | 있음 |
| VALTAN_ARMOR_BREAK_OPENING | 오프닝 외벽 충돌 부위 파괴 | 호환 | 4 | AUDITION_ONLY | 있음 |
| VALTAN_FLOOR_WIPE_130 | 6방향 후 전멸 패턴 | 저작 | 5 | HEALTH_BAR | 있음 |
| VALTAN_FOUR_PILLARS_105 | 100줄 지목 착지와 4기둥 추적 원뿔 | 호환 | 4 | AUDITION_ONLY | 있음 |
| VALTAN_ENTRANCE_WHIRLWIND | 첫 등장 회전 돌진 | 호환 | 3 | NORMAL | 있음 |
| VALTAN_ARENA_BREAK_109 | 중앙 이동 후 2페이즈 컷씬 | 저작 | 6 | HEALTH_BAR | 있음 |
| VALTAN_MAGIC_ORB_STAGGER_76 | 73줄 남북·동서 마력구 폭발 | 호환 | 5 | AUDITION_ONLY | 있음 |
| VALTAN_CENTER_GRAB_COUNTER_64 | 62줄 3페이즈 중앙 표적 폭발과 잡기 카운터 | 호환 | 5 | AUDITION_ONLY | 있음 |
| VALTAN_GHOST_TRANSITION_15 | 14줄 포탈·최종 지면파괴와 망령화 | 호환 | 7 | AUDITION_ONLY | 있음 |
| VALTAN_SIX_PIZZA_106 | 중앙이동 후 6방향 공격 후 피자 패턴 | 저작 | 11 | HEALTH_BAR | 있음 |
| VALTAN_ATTACK_WHIRLWIND | 점프찍기 후 휠윈드 | 저작 | 4 | AUDITION_ONLY | 있음 |
| VALTAN_CHARGE | 모아치기 | 저작 | 3 | AUDITION_ONLY | 있음 |
| VALTAN_SEQUENCE_FOUR | 2페이즈 4방향 공격 | 저작 | 1 | NORMAL | 있음 |
| VALTAN_ROAR_CHARGE | 사자후 후 위로 모아치기 | 저작 | 6 | AUDITION_ONLY | 있음 |
| VALTAN_SEQUENCE_RUSH | 2페이즈 돌진 연계 | 저작 | 3 | AUDITION_ONLY | 있음 |
| VALTAN_THREE | 3연속 내려치기 | 저작 | 3 | AUDITION_ONLY | 있음 |
| VALTAN_TERRAIN_DESTRUCTION | 2페이즈 지형파괴 패턴 | 저작 | 14 | AUDITION_ONLY | 있음 |
| VALTAN_WARP | 워프 패턴 | 저작 | 10 | NORMAL | 있음 |
| VALTAN_SEQUENCE_TWOHAND | 2페이즈 양손 내려치기 | 저작 | 2 | AUDITION_ONLY | 있음 |
| VALTAN_SEQUENCE_WHIRLWIND | 2페이즈 휠윈드 연계 | 저작 | 3 | AUDITION_ONLY | 있음 |
| VALTAN_TRASH | 버러지 패턴 | 저작 | 22 | HEALTH_BAR | 있음 |
| VALTAN_TRASH_CATCH_SUCCESS | 버러지 패턴 잡기 성공 | 저작 | 4 | AUDITION_ONLY | 있음 |
| VALTAN_TRASH_CATCH_FAIL | 버러지 패턴 잡기 실패 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_TRASH_CATCH_IF | 버러지 패턴 잡기 분기 | 저작 | 16 | AUDITION_ONLY | 있음 |
| VALTAN_CATCH_BREATH | 잡아채서 불어 날리기 | 저작 | 4 | NORMAL | 있음 |
| VALTAN_COUNTER | 카운터 쳐야 하는 내려치기 | 저작 | 3 | AUDITION_ONLY | 있음 |
| VALTAN_CHARGE_2 | 모아치기 2 | 저작 | 3 | AUDITION_ONLY | 있음 |
| VALTAN_STRUGGLING | 3페이즈 전 발악패턴 | 저작 | 18 | HEALTH_BAR | 있음 |
| VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK | 점프후지형파괴3시 | 저작 | 13 | HEALTH_BAR | 있음 |
| VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK | 점프후지형파괴9시 | 저작 | 13 | HEALTH_BAR | 있음 |
| VALTAN_ENTRANCE_CINEMATIC | 발탄 등장 컷신 | 저작 | 3 | NORMAL | 있음 |
| VALTAN_CROSS | 십자 돌 공격 | 저작 | 1 | NORMAL | 있음 |
| VALTAN_GHOST_FINALE | 유령 발탄 · 포탈 최종 패턴 | 저작 | 10 | AUDITION_ONLY | 있음 |
| VALTAN_GHOST_RESPAWN_AUDITION | 3페이즈 망령화 발탄 부활 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_GHOST_DEATH_AUDITION | 3페이즈 발탄 사망 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_GROUND_ROAR | 땅구르기사자후 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_STAGGER_SLOT | 마력구 파괴 패턴 | 저작 | 2 | NORMAL | 있음 |
| VALTAN_BIND_SLOT | 속박 패턴 | 저작 | 2 | NORMAL | 있음 |
| VALTAN_SILENCE_SLOT | 침묵 패턴 | 저작 | 2 | NORMAL | 있음 |
| VALTAN_GHOST_PORTAL_ONCE | 망령 포탈 돌진 1회 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_PART_BREAK | 부위 파괴 | 저작 | 2 | AUDITION_ONLY | 있음 |
| VALTAN_GROGGY_FOLLOWUP | 발탄 공용 그로기 후속 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_FIST_IN_OUT_LARGE | 큰 도넛 | 저작 | 1 | AUDITION_ONLY | 있음 |
| VALTAN_3H_FLOOR_BREAK_ROCK_ROAR | 발탄_3시지면파괴_땅구르기_사자후 | 저작 | 9 | AUDITION_ONLY | 있음 |
| VALTAN_ROAR_3H_DESTROY | 지형파괴사자후 | 저작 | 15 | AUDITION_ONLY | 있음 |

### 발탄 보존·제외 정의

`Valtan.legacy-compatibility.json`에는 25개 정의가 보존되어 있으나, `VALTAN_ARENA_BREAK_84`(84줄 외곽 바닥 붕괴)와 `VALTAN_ARENA_BREAK_33`(30줄 두 번째 지형 절반 파괴)은 gameplay의 `retiredPatternIds`에 들고 현재 Encounter 및 게시 PATTERN 행에는 없다. 따라서 현재 게시 합계는 gameplay 44 + 호환 23 = 67이다. 두 저장 목록의 ID가 겹쳐서 줄어든 것이 아니다.

현재 retiredPatternIds 전체는 `VALTAN_TRIPLE_SLASH`, `VALTAN_ROTATION_SLASH`, `VALTAN_ARENA_BREAK_84`, `VALTAN_ARENA_BREAK_33`, `VALTAN_SEQUENCE_FRONT_BACK_FRONT`, `VALTAN_DASH_CHARGE_GROGGY`, `VALTAN_COUNTER_GROGGY`다. 폐기 ID 보존을 현재 활성 패턴 존재로 세지 않는다.

### 루가루의 실제 구현 경계

루가루는 [Data/Balance/MonsterProfiles.json](C:/Users/tnest/Desktop/LostArk/Data/Balance/MonsterProfiles.json)의 `MINIBOSS_LUGARU` profile과 [Data/Actors/MonsterCatalog.json](C:/Users/tnest/Desktop/LostArk/Data/Actors/MonsterCatalog.json)의 표현 정의를 사용한다. 현재 profile은 HP 25000, 공격력 300, 공격 반경 값 3.2m, windup 720ms, active 280ms, recovery 1050ms를 담는다. 공격 반경 값은 최종 판정 반경과 같다고 단정하지 않는다. `CMonsterBrain::Update`는 `fAttackRange + fCollisionRadius`로 공격 원을 구성하고 플레이어 몸체 원과 overlap을 확인한다.

`CGameRoom`의 MONSTER 분기→`CMonsterBrain::Update`에서 IDLE/CHASE→PATTERN_WINDUP→PATTERN_ACTIVE→PATTERN_RECOVERY로 전이한다. ACTIVE에서는 `hasAppliedPatternDamage`로 공격 기회를 한 번만 소비하고 `CServerCombatHitRuntime::Apply_WorldToPlayer`로 피해를 적용한다. 범위 밖에서 실제 피해가 0이어도 해당 판정 기회는 소비한다. 루가루 전용 Encounter나 구슬·색상 분화 같은 원작 레이드 기믹이 구현됐다는 근거는 이 경로에 없으므로 기술소개서에 추가 구현으로 적지 않는다.

근거: [Server/Private/GameRoom_BossSimulation.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp)의 MONSTER 분기(조사 시 3059행), [Server/Private/MonsterBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/MonsterBrain.cpp)의 Update(280행), ACTIVE 처리(515행 부근).

### 쿠크 전체 저장 정의 121개

출처: [Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json) (formatVersion 3, revision 2500), [Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json](C:/Users/tnest/Desktop/LostArk/Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json) (sourceRevision 2500). “논리창”은 생성된 Encounter의 `logicWindows.length`이며 공격 횟수·명중 횟수가 아니다. “직접 Flow”는 네 patternFlows의 PATTERN 항목에서 targetId로 직접 참조하는 관문만 표시한다. Bundle 내부·Logic 결과·소환·후속 참조까지의 전체 도달성은 뜻하지 않는다.

#### GATE1 — 22개

| patternId | 표시 이름 | targetBossPlacementId | 저장 상태 | Encounter / 게시 행 | 논리창 | 직접 Flow |
| --- | --- | --- | --- | --- | ---: | --- |
| KAKULSAYDON_G1_PATTERN_1 | 세이튼_무력화 시작 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 1 | GATE1 |
| KAKULSAYDON_G1_PATTERN_2 | 세이튼_진짜세이튼찾기 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 1 | GATE1 |
| KAKULSAYDON_G1_PATTERN_3 | 세이튼_무력화실패 | boss.kakulsaydon.g1.saydon | DRAFT | 없음 / 없음 | — | — |
| KAKULSAYDON_G1_PATTERN_4 | 세이튼_무력화성공 | boss.kakulsaydon.g1.saydon | PRODUCT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_5 | 세이튼_가짜세이튼 | boss.kakulsaydon.g1.saydon | PRODUCT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_6 | 세이튼_댄스타임 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 4 | GATE1 |
| KAKULSAYDON_G1_PATTERN_7 | 세이튼_룰렛 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 5 | GATE1 |
| KAKULSAYDON_G1_PATTERN_36 | 세이튼_1관문연출 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_47 | 세이튼_트럼펫장판소환 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 2 | GATE1 |
| KAKULSAYDON_G1_PATTERN_48 | 세이튼_빙글빙글돌며카드던지기 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 6 | GATE1 |
| KAKULSAYDON_G1_PATTERN_58 | 세이튼_화염 파동 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 36 | GATE1 |
| KAKULSAYDON_G1_PATTERN_78 | 세이튼_카드맞추기_주사위패턴 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 1 | GATE1 |
| KAKULSAYDON_G1_PATTERN_79 | 세이튼_공먹고휘리릭_노란장판펑펑 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 2 | GATE1 |
| KAKULSAYDON_G1_PATTERN_80 | 세이튼_돌진_카운터 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 2 | GATE1 |
| KAKULSAYDON_G1_PATTERN_81 | 세이튼_공굴리기_카운터 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 2 | GATE1 |
| KAKULSAYDON_G1_PATTERN_82 | 세이튼_마술쇼_종이비둘기 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 9 | GATE1 |
| KAKULSAYDON_G1_PATTERN_83 | 세이튼_쓰리투원투하! | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 20 | GATE1 |
| KAKULSAYDON_G1_PATTERN_100 | 세이튼_정면바람방구 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 1 | GATE1 |
| KAKULSAYDON_G1_PATTERN_101 | 세이튼_플레이어추적 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 0 | GATE1 |
| KAKULSAYDON_G1_PATTERN_102 | 세이튼_백스텝후불뿜기화염링 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 8 | GATE1 |
| KAKULSAYDON_G1_PATTERN_103 | 세이튼_카드비 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 0 | GATE1 |
| KAKULSAYDON_G1_PATTERN_104 | 세이튼_1초추적 | boss.kakulsaydon.g1.saydon | DRAFT | 있음 / 있음 | 0 | GATE1 |

#### GATE2 — 29개

| patternId | 표시 이름 | targetBossPlacementId | 저장 상태 | Encounter / 게시 행 | 논리창 | 직접 Flow |
| --- | --- | --- | --- | --- | ---: | --- |
| KAKULSAYDON_G1_PATTERN_8 | 쿠크_세이튼등장 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 10 | — |
| KAKULSAYDON_G1_PATTERN_9 | 대형세이튼_세이튼등장 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_10 | 쿠크_파1빨2 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_11 | 대형세이튼_파1빨2 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 10 | — |
| KAKULSAYDON_G1_PATTERN_12 | 쿠크_조커찾기 | boss.kakulsaydon.g2.kouku | PRODUCT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_13 | 대형세이튼_조커찾기 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 11 | — |
| KAKULSAYDON_G1_PATTERN_14 | 대형세이튼_조커찾기_성공 | boss.kakulsaydon.g2.big-saydon | PRODUCT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_15 | 쿠크_거미카운터 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 9 | — |
| KAKULSAYDON_G1_PATTERN_17 | 대형세이튼_잡기 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 6 | GATE2 |
| KAKULSAYDON_G1_PATTERN_25 | 쿠크_피자 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 13 | — |
| KAKULSAYDON_G1_PATTERN_26 | 대형세이튼_피자 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_20 | 대형세이튼_레이저 | boss.kakulsaydon.g2.big-saydon | DRAFT | 없음 / 없음 | — | — |
| KAKULSAYDON_G1_PATTERN_21 | 쿠크_레이저 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 7 | GATE2 |
| KAKULSAYDON_G1_PATTERN_22 | 쿠크_그로기 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_23 | 쿠크_팡파레 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 7 | GATE2 |
| KAKULSAYDON_G1_PATTERN_24 | 쿠크_휠윈드 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 17 | — |
| KAKULSAYDON_G1_PATTERN_27 | 대형세이튼_불뿜기 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 1 | GATE2 |
| KAKULSAYDON_G1_PATTERN_28 | 쿠크_카드미로연출 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_73 | 2관문_진입컷씬 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_74 | 2관문클리어_3관문진입 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_77 | 카드미로연출 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | GATE2 |
| KAKULSAYDON_G1_PATTERN_84 | 쿠크_훌라후프_레드 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_85 | 쿠크_슈퍼바주카 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 2 | GATE2 |
| KAKULSAYDON_G1_PATTERN_86 | 대형세이튼_두번내려치기 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 2 | GATE2 |
| KAKULSAYDON_G1_PATTERN_87 | 대형세이튼_3번내려치기 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 3 | GATE2 |
| KAKULSAYDON_G1_PATTERN_90 | 쿠크_훌라후프_화이트 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_99 | 대형세이튼_불어날리기 | boss.kakulsaydon.g2.big-saydon | DRAFT | 있음 / 있음 | 1 | GATE2 |
| KAKULSAYDON_G1_PATTERN_105 | 쿠크_나팔액션 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 1 | GATE2 |
| KAKULSAYDON_G1_PATTERN_106 | 쿠크_저글링액션 | boss.kakulsaydon.g2.kouku | DRAFT | 있음 / 있음 | 0 | GATE2 |

#### GATE3 — 53개

| patternId | 표시 이름 | targetBossPlacementId | 저장 상태 | Encounter / 게시 행 | 논리창 | 직접 Flow |
| --- | --- | --- | --- | --- | ---: | --- |
| KAKULSAYDON_G1_PATTERN_18 | 3관문_외곽불회전_갈고리대각선_시각테스트 | boss.kakulsaydon.g3.saydon | PRODUCT | 있음 / 있음 | 18 | — |
| KAKULSAYDON_G1_PATTERN_19 | 3관문_갈고리만_확인용_불없음 | boss.kakulsaydon.g3.saydon | PRODUCT | 있음 / 있음 | 18 | — |
| KAKULSAYDON_G1_PATTERN_31 | 3관문 칼날 바닥  | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 40 | — |
| KAKULSAYDON_G1_PATTERN_33 | 세이튼_마리오_2페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 57 | — |
| KAKULSAYDON_G1_PATTERN_34 | 세이튼_마리오_1페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 3 | — |
| KAKULSAYDON_G1_PATTERN_35 | 세이튼_쇼타임 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 111 | GATE3 |
| KAKULSAYDON_G1_PATTERN_38 | 쿠크세이튼_무지개댄스 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 20 | GATE3 |
| KAKULSAYDON_G1_PATTERN_39 | 쿠크세이튼_알비온_감전장판 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 10 | GATE3 |
| KAKULSAYDON_G1_PATTERN_40 | 쿠크세이튼_십자화염폭발 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_41 | 쿠크세이튼_갈고리_칼날댄스 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_42 | 쿠크세이튼_무력화성공  | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_43 | 쿠크세이튼_백스텝불뿜기_화염링 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 8 | GATE3 |
| KAKULSAYDON_G1_PATTERN_44 | 쿠크세이튼_분신소환_기분나빠 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_45 | 쿠크세이튼_분신소환_십자화염폭발 | boss.kakulsaydon.g3.saydon | DRAFT | 없음 / 없음 | — | — |
| KAKULSAYDON_G1_PATTERN_46 | 쿠크세이튼_벡스텝후감전빔 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_49 | 쿠크세이튼_우측이동화염파동 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 14 | — |
| KAKULSAYDON_G1_PATTERN_50 | 쿠크세이튼_분신소환_기분나빠_전방 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_51 | 쿠크세이튼_분신소환_기분나빠_전방 | boss.kakulsaydon.g3.saydon | DRAFT | 없음 / 없음 | — | — |
| KAKULSAYDON_G1_PATTERN_52 | 쿠크세이튼_분신소환_기분나빠 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | GATE3 |
| KAKULSAYDON_G1_PATTERN_53 | 쿠크세이튼_분신소환_기분나빠_후방 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_54 | 쿠크세이튼_분신소환_기분나빠_왼쪽 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_55 | 쿠크세이튼_분신소환_기분나빠_오른쪽 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_56 | 세이튼_마리오_1페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_57 | 세이튼_마리오_2페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_59 | 쿠크세이튼_화염 파동 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 28 | GATE3 |
| KAKULSAYDON_G1_PATTERN_60 | 기분나빠 \| 브레스 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_61 | 저주의식 \| 왼손 트레일 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_62 | 세이튼_눈 레이저 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | GATE3 |
| KAKULSAYDON_G1_PATTERN_63 | 쿠크세이튼_분신소환_십자화염폭발_왼쪽 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_64 | 쿠크세이튼_분신소환_십자화염폭발_오른쪽 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_65 | 쿠크세이튼_분신소환_십자화염폭발 | boss.kakulsaydon.g3.saydon | DRAFT | 없음 / 없음 | — | — |
| KAKULSAYDON_G1_PATTERN_66 | 쿠크세이튼_분신소환_3갈레불뿜기 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_67 | 쿠크세이튼_분신소환_3갈레불뿜기_전방 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_68 | 쿠크세이튼_분신소환_3갈레불뿜기_후방  | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_69 | 쿠크세이튼_분신소환_3갈레불뿜기_왼쪽 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_70 | 쿠크세이튼_분신소환_3갈레불뿜기_오른쪽 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_71 | 쿠크세이튼_분신소환_십자화염폭발_전방 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_72 | 쿠크세이튼_분신소환_십자화염폭발_후방 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_76 | 쇼타임_연출 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | GATE3 |
| KAKULSAYDON_G1_PATTERN_88 | 1마리오_1페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 6 | GATE3 |
| KAKULSAYDON_G1_PATTERN_89 | 쿠크세이튼_1마리오 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 3 | — |
| KAKULSAYDON_G1_PATTERN_91 | 2마리오_1페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 24 | GATE3 |
| KAKULSAYDON_G1_PATTERN_92 | 3마리오_1페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 24 | GATE3 |
| KAKULSAYDON_G1_PATTERN_93 | 4마리오_1페이즈 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 24 | GATE3 |
| KAKULSAYDON_G1_PATTERN_95 | 즉사 칼날 \| Object 판정 테스트 | boss.kakulsaydon.g3.saydon | PRODUCT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_110 | 앵콜세이튼_백스텝후화염링 | boss.kakulsaydon.g3.saydon | DRAFT | 없음 / 없음 | — | — |
| KAKULSAYDON_G1_PATTERN_111 | 앵콜세이튼_십자화염폭발 | boss.kakulsaydon.g3.saydon | DRAFT | 없음 / 없음 | — | — |
| KAKULSAYDON_G1_PATTERN_116 | 쿠크세이튼_정면바람방구 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 1 | GATE3 |
| KAKULSAYDON_G1_PATTERN_117 | 쿠크세이튼_돌진_카운터 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 2 | GATE3 |
| KAKULSAYDON_G1_PATTERN_118 | 쿠크세이튼_공굴리기_카운터 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 19 | GATE3 |
| KAKULSAYDON_G1_PATTERN_119 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 2 | GATE3 |
| KAKULSAYDON_G1_PATTERN_124 | 쿠크세이튼_플레이어추적 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_125 | 쿠크세이튼_1초추적 | boss.kakulsaydon.g3.saydon | DRAFT | 있음 / 있음 | 0 | GATE3 |

#### BINGO — 17개

| patternId | 표시 이름 | targetBossPlacementId | 저장 상태 | Encounter / 게시 행 | 논리창 | 직접 Flow |
| --- | --- | --- | --- | --- | ---: | --- |
| KAKULSAYDON_G1_PATTERN_75 | 빙고_최종엔딩씬 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_94 | 메두사공포 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 1 | — |
| KAKULSAYDON_G1_PATTERN_97 | 앵콜컷신 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_107 | 빙고_이동_첫클립_메두사_블랙홀 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 2 | — |
| KAKULSAYDON_G1_PATTERN_108 | 앵콜세이튼_분신소환_기분나빠 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 1 | BINGO |
| KAKULSAYDON_G1_PATTERN_109 | 앵콜세이튼_기본공격 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 1 | BINGO |
| KAKULSAYDON_G1_PATTERN_112 | 앵콜세이튼_십자화염폭발 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 1 | BINGO |
| KAKULSAYDON_G1_PATTERN_113 | 앵콜세이튼_정면바람방구 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 1 | BINGO |
| KAKULSAYDON_G1_PATTERN_114 | 앵콜세이튼_백스텝후화염링 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 8 | BINGO |
| KAKULSAYDON_G1_PATTERN_115 | 앵콜세이튼_화염파동 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 28 | — |
| KAKULSAYDON_G1_PATTERN_120 | 앵콜세이튼_알비온_감전장판 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 10 | BINGO |
| KAKULSAYDON_G1_PATTERN_122 | 앵콜세이튼_공먹고휘리릭노란장판펑펑 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 2 | BINGO |
| KAKULSAYDON_G1_PATTERN_123 | 앵콜세이튼_레이저 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 1 | BINGO |
| KAKULSAYDON_G1_PATTERN_126 | 앵콜세이튼_플레이어추적 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 0 | BINGO |
| KAKULSAYDON_G1_PATTERN_127 | 빙고_블랙홀_첫클립 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_128 | 빙고_블랙홀_13초_폭발 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 0 | — |
| KAKULSAYDON_G1_PATTERN_129 | 빙고_보드시작 | boss.kakulsaydon.bingo.saydon | DRAFT | 있음 / 있음 | 0 | BINGO |

### 쿠크에서 Encounter에 없는 저장 정의 7개

아래 사유는 현재 생성된 `patternInventory.patterns[].unavailableReason`의 원문이다. 문구의 PRODUCT는 검증기 메시지이며 Composition의 저장 상태가 PRODUCT라는 뜻으로 바꾸지 않는다. 이번에 projector를 재실행해서 새로 판정한 결과가 아니다.

| patternId | 표시 이름 | 저장본에 기록된 게시 불가 사유 |
| --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_3 | 세이튼_무력화실패 | PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_3 |
| KAKULSAYDON_G1_PATTERN_20 | 대형세이튼_레이저 | PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_20 |
| KAKULSAYDON_G1_PATTERN_45 | 쿠크세이튼_분신소환_십자화염폭발 | PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_45 |
| KAKULSAYDON_G1_PATTERN_51 | 쿠크세이튼_분신소환_기분나빠_전방 | PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_51 |
| KAKULSAYDON_G1_PATTERN_65 | 쿠크세이튼_분신소환_십자화염폭발 | Cross direction child needs its clone end Stage before the full ending and must fit the owning window duration |
| KAKULSAYDON_G1_PATTERN_110 | 앵콜세이튼_백스텝후화염링 | PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_110 |
| KAKULSAYDON_G1_PATTERN_111 | 앵콜세이튼_십자화염폭발 | PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_111 |

### 관문 진행의 현재 데이터

Composition `patternFlows`는 4개다. GATE1 166항목, GATE2 85항목, GATE3 72항목, BINGO 35항목이다. 반복·추적·Bundle 호출이 섞인 실행 목록의 길이이며 고유 패턴 수가 아니다. 아래 체력 줄은 각 entryGroup의 저장 표시와 조건을 근거로 정리했다. 원작 규칙 또는 이번 플레이의 성공 관측으로 확대하지 않는다.

| Flow | 주요 기믹 구간 | 일반 진행 |
| --- | --- | --- |
| GATE1 | 130 무력화 / 110 진짜 세이튼 / 85 댄스 / 60 무력화 / 50 룰렛 / 30 진짜 세이튼 | 체력 경계까지 일반 패턴 구간 반복; PATTERN_END 전환 |
| GATE2 | 125 세이튼 등장 / 110 파1빨2 / 95 조커 / 80 카드미로 / 55 피자 / 25 조커 | 저장 흐름의 첫 일반 구간은 140→125줄 |
| GATE3 | 155 1마리오 / 125 2마리오 / 90 쇼타임 / 80 3마리오 / 55 4마리오 | 저장 흐름의 첫 일반 구간은 180→155줄 |
| BINGO | 129 보드시작; specialPatternId는 107 | loopStartEntryId=kakulsaydon.flow.bingo.independent.30 |

### 소스에서 확인할 소비 지점

| 역할 | 실제 파일과 심볼 |
| --- | --- |
| 발탄 저작 결합·호환 정의 통합 | Tools/ValtanPipeline/valtan_tuning_pipeline.py; legacy compatibility와 gameplay를 함께 사용 |
| 쿠크 projection | Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py::_prepare_publication (2876), project_encounter (5000), project_presentation (6204) |
| 게시 행 파싱 | Server/Private/GameplayCatalog.cpp::Load_BootstrapBytes (1781), PATTERNLOGIC 분기 (3135) |
| 발탄 실행 | Server/Private/GameRoom_BossSimulation.cpp의 CValtanBrain 선택·Update (2503 부근); Server/Private/ValtanBrain.cpp |
| 쿠크 시간·판정·결과 | Server/Private/KoukuSaydonLogicRuntime.cpp::Build (129), Apply_Result (1438), Update (1750) |
| 쿠크 호출자 | Server/Private/GameRoom_KoukuAudition.cpp (1588 Logic Update / 1599 Brain Update) |
| 루가루 공용 실행 | Server/Private/MonsterBrain.cpp::Update (280), GameRoom_BossSimulation.cpp::m_MonsterBrain.Update (3073) |
| 화면 표현 | Client/Private/ClientReplication.cpp, Client/Private/Valtan.cpp::Apply_NetworkState, Client/Private/KoukuSaydonPresentationPlayer.cpp |

이 목록을 기반으로 다음에는 같은 패턴 하나의 include·H 선언·CPP 정의, 입력과 반환값, 변수의 소유자와 수명, ID 조회, tick 변환, 실패와 정리를 읽는다. 전체 목록의 존재는 모든 패턴의 내부·화면·네트워크 동작을 이번에 검증했다는 뜻이 아니다. 이번 목록 조사에서는 게임 실행, C++ 빌드, 모든 패턴 재생, 사용자 breakpoint 관찰을 수행하지 않았다.

<!-- boss-inventory-source-sha256:begin -->
### 조사 입력의 SHA-256

동시 편집 후 목록을 다시 반영할 때 아래 입력이 바뀌었으면 현재 저장본으로 다시 대조한다.

| 입력 파일 | SHA-256 |
| --- | --- |
| Data/Valtan/Valtan.gameplay.json | 18346b43298cddb027f2d76c0ddf7135c6861f58a2159c608f96e38a3422b601 |
| Data/Valtan/Valtan.legacy-compatibility.json | d23891c0e4c26b7066f0391b412855639fdc7116c8186eed26eb2ccb99056eed |
| Data/Encounters/Valtan/ValtanEncounter.json | 0c1d58db456a207e604e2ef38f11c535b31faa94e36644431a7e67101f2d9dc7 |
| Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json | 68fe58f0a92d461c412d128313650cbf668c65584d897eaf6364611daabeaa6c |
| Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json | 9c180c1daafc09f30caa40da3d71158fa29a561d700fbc46d84650e84fdb9bd4 |
| Data/Balance/BossProfiles.json | bbc168df293024a27502bde79c8df7792d75666429095a69d8cfde7a9c44ed6f |
| Data/Balance/MonsterProfiles.json | 8b613e18ff57efcae0d5c8bb92af7b7f568817145948f7d74f0ebd61282c9a03 |
| Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap | af7a99955266ec582e38a87456ff0de5cf5c335006071111cb2362f3adb0bc4d |

<!-- boss-inventory-source-sha256:end -->
