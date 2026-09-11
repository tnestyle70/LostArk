# 쿠크세이튼 현재 패턴·애니메이션·이펙트 런타임 전수 실측

조사일: 2026-09-11 KST. 현재 dirty worktree의 저장본을 읽었으며 기존 코드·게임 데이터·Resources를 수정하지 않았다. 문서 생성 외에 Client/UI 실행·조작·캡처, 빌드, publisher는 수행하지 않았다. 이 문서의 “게시됨”은 데이터와 소비 코드의 연결 확인이며 사용자 화면 PASS가 아니다.

## G01. 현재 재생 정본과 완료 범위

정본은 [KoukuSaydonComposition.json](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:1) revision **322**다. [KoukuSaydonEncounter.json](C:/Users/user/Desktop/LostArk/Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json:1)와 [KoukuSaydon.patternbindings.json](C:/Users/user/Desktop/LostArk/Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json:1)의 sourceRevision도 322/322로 일치한다. 파일명 `Gate1`과 모든 ID의 `G1` 접두사는 관문 의미가 아니다. 실제 관문은 각 row의 `gateId`로 판단한다. 09-05 survey의 profile 기반 관문 구분은 당시 조사 분류이며 현재 Gate 배치 정본을 대체하지 않는다.

| Gate | 저장 Pattern | 게시 Pattern | Stage/animation | 직접 Effect occurrence | 직접 고유 Effect ID |
| --- | --- | --- | --- | --- | --- |
| GATE1 | 7 | 6 | 66 | 45 | 14 |
| GATE2 | 19 | 16 | 105 | 40 | 14 |
| GATE3 | 2 | 2 | 24 | 0 | 0 |
| BINGO | 0 | 0 | 0 | 0 | 0 |

총 **28개 저장 / 24개 게시 / 195개 stage / 85개 직접 Effect occurrence / 28개 직접 고유 Effect ID**다. Bundle은 저장 10개 중 7개 게시, presentation resource는 52개(EFFECT 30, COLLIDER 18, LIGHT 3, CAMERA 1)다. 등록 EFFECT 30행은 shield 중복 정의를 포함하므로 고유 asset ID는 29개다. EFFECT 0은 WORLD·fear·카드 상태·빙고 같은 다른 경로의 시각 표현까지 없다는 의미가 아니다.

게시된 24개는 전부 **`selectionMode=AUDITION_ONLY`, `selectionWeight=0`**다. [project_kouku_saydon_composition.py](C:/Users/user/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:2820)가 이를 생성하고, [KoukuSaydonBrain.cpp](C:/Users/user/Desktop/LostArk/Server/Private/KoukuSaydonBrain.cpp:108)가 그 계약만 허용한다. Brain `Update`는 pattern ID가 없으면 IDLE로 끝난다([KoukuSaydonBrain.cpp](C:/Users/user/Desktop/LostArk/Server/Private/KoukuSaydonBrain.cpp:603)). 따라서 현재는 **Server 권위 수동 Complete Play/Bundle 및 내부 성공·실패·추가 패턴 연결**이며, 자동 레이드 AI가 HP/거리로 짤패턴을 선택하는 완성 상태가 아니다.

연결 구조: Composition stage `actionId → runtimeClip/sourceStartMs/playMs/playRate`와 pattern `presentationOccurrences → resourceId → EFFECT assetId`를 별도로 게시한다. Shared snapshot의 action/pattern clock을 Client가 소비한다. 원본 Action에 있던 모든 notify를 clip 이름만으로 자동 재생하는 구조는 아니다.

4개 profile의 `*.actionbindings.json`과 `*.patternbindings.json`은 모두 `authority=REFERENCE_ONLY`, 각 배열 0개다. 실제 제품은 통합 [KoukuSaydon.patternbindings.json](C:/Users/user/Desktop/LostArk/Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json:1)의 195 bindings를 사용한다. 반면 `Data/Effects/V2/Bindings/MN_RPCT_06.effectv2bindings.json`에는 별도 legacy NPC clip binding 2개가 남아 있다. 본문의 G06에서 구분한다.

## G02. 저장된 28개 패턴 전체

표의 P번호는 `KAKULSAYDON_G1_PATTERN_<번호>`를 줄인 것이다. 게시 상태의 모든 행은 Server 수동 audition이다. “미게시” 4개는 stage가 없어서 runtime 배열에서 제외되며 inventory에 사유가 보존된다.

| ID·근거 | Gate | 현재 이름 | 배우 profile | 연결 상태 | Stage | Effect box | WORLD box | 참고 source action | 직접 연결 Effect ID |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| [P1](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:1892) | GATE1 | 세이튼_무력화 시작 | MN_RPCT_05 | 게시·수동 | 7 | 21 | 0 | `4219945` | `boss.kouku.disarm.shield_1`, `boss.kouku.disarm.converge_1`, `boss.kouku.blur_1`, `boss.kouku.disarm.converge_2`, `boss.kouku.disarm.converge_3`, `boss.kouku.disarm.star_1`, `boss.kouku.disarm.star.smoke_1`, `boss.kouku.disarm.star.decal_1` |
| [P2](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:2960) | GATE1 | 세이튼_진짜세이튼찾기 | MN_RPCT_05 | 게시·수동 | 13 | 4 | 0 | `4219823`, `4219824`, `4219842` | `boss.kouku.find.heart`, `boss.kouku.find.core` |
| [P3](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:3537) | GATE1 | 세이튼_무력화실패 | MN_RPCT_05 | 미게시: stage 0 | 0 | 0 | 0 | 없음 | 없음 |
| [P4](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:3560) | GATE1 | 세이튼_무력화성공 | MN_RPCT_05 | 게시·수동 | 3 | 0 | 0 | `4219946` | 없음 |
| [P5](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:3650) | GATE1 | 세이튼_가짜세이튼 | MN_RPCT_05 | 게시·수동 | 13 | 4 | 0 | `4219857`, `4219891` | `boss.kouku.find.star`, `boss.kouku.find.core` |
| [P6](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:4113) | GATE1 | 세이튼_댄스타임 | MN_RPCT_05 | 게시·수동 | 21 | 16 | 1 | `4219895`, `4219897` | `boss.kouku.curtain_1`, `boss.kouku.dance`, `boss.kouku.dance.clap` |
| [P7](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:5366) | GATE1 | 세이튼_룰렛 | MN_RPCT_05 | 게시·수동 | 9 | 0 | 1 | `4219811`, `4219813` | 없음 |
| [P8](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:6694) | GATE2 | 쿠크_세이튼등장 | MN_RPCZ_00 | 게시·수동 | 1 | 0 | 1 | `4219719` | 없음 |
| [P9](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:6764) | GATE2 | 대형세이튼_세이튼등장 | MN_RPCT_06 | 게시·수동 | 3 | 1 | 0 | `15`, `0` | `boss.kouku.appear` |
| [P10](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:6893) | GATE2 | 쿠크_파1빨2 | MN_RPCZ_00 | 게시·수동 | 5 | 0 | 0 | `4219728` | 없음 |
| [P11](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:7027) | GATE2 | 대형세이튼_파1빨2 | MN_RPCT_06 | 게시·수동 | 6 | 7 | 0 | `4221813`, `0` | `boss.kouku.medusa.blue`, `boss.kouku.medusa.red`, `boss.kouku.medusa.laser` |
| [P12](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:7487) | GATE2 | 쿠크_조커찾기 | MN_RPCZ_00 | 게시·수동 | 2 | 0 | 0 | `0` | 없음 |
| [P13](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:7555) | GATE2 | 대형세이튼_조커찾기 | MN_RPCT_06 | 게시·수동 | 21 | 4 | 7 | `4221821`, `4221820`, `4221818` | `boss.kouku.joker.hammer` |
| [P14](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:8819) | GATE2 | 대형세이튼_조커찾기_성공 | MN_RPCT_06 | 게시·수동 | 1 | 0 | 0 | `4221816` | 없음 |
| [P15](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:8865) | GATE2 | 쿠크_거미카운터 | MN_RPCZ_00 | 게시·수동 | 15 | 3 | 0 | `4219776` | `effect.valtan.project-tuned.sequence.counter` |
| [P16](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:9521) | GATE2 | 대형세이튼_거미카운터 | MN_RPCT_06 | 미게시: stage 0 | 0 | 0 | 0 | 없음 | 없음 |
| [P17](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:9544) | GATE2 | 대형세이튼_잡기 | MN_RPCT_06 | 게시·수동 | 3 | 4 | 0 | `4221810` | `boss.kouku.joker.hammer` |
| [P25](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:9849) | GATE2 | 쿠크_피자 | MN_RPCZ_00 | 게시·수동 | 16 | 17 | 0 | `4219769` | `boss.kouku.pizza.aura`, `boss.kouku.pizza.star.cl`, `boss.kouku.pizza.star.ccl`, `boss.kouku.pizza.w`, `boss.kouku.pizza.s`, `boss.kouku.pizza.e`, `boss.kouku.pizza.n` |
| [P26](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:10872) | GATE2 | 대형세이튼_피자 | MN_RPCT_06 | 미게시: stage 0 | 0 | 0 | 0 | 없음 | 없음 |
| [P20](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:10895) | GATE2 | 대형세이튼_레이저 | MN_RPCT_06 | 미게시: stage 0 | 0 | 0 | 0 | 없음 | 없음 |
| [P21](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:10918) | GATE2 | 쿠크_레이저 | MN_RPCZ_00 | 게시·수동 | 3 | 0 | 1 | `4219740` | 없음 |
| [P22](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:11368) | GATE2 | 쿠크_그로기 | MN_RPCZ_00 | 게시·수동 | 3 | 0 | 0 | `4219763` | 없음 |
| [P23](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:11459) | GATE2 | 쿠크_팡파레 | MN_RPCZ_00 | 게시·수동 | 4 | 0 | 1 | `4219714` | 없음 |
| [P24](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:11732) | GATE2 | 쿠크_휠윈드 | MN_RPCZ_00 | 게시·수동 | 15 | 0 | 1 | `4219708` | 없음 |
| [P18](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:12146) | GATE3 | 3관문_외곽불회전_갈고리대각선_시각테스트 | MN_RPCT_05 | 게시·수동 | 12 | 0 | 78 | `0` | 없음 |
| [P19](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:15209) | GATE3 | 3관문_갈고리만_확인용_불없음 | MN_RPCT_05 | 게시·수동 | 12 | 0 | 18 | `0` | 없음 |
| [P27](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:16832) | GATE2 | 대형세이튼_불뿜기 | MN_RPCT_06 | 게시·수동 | 4 | 0 | 0 | `4221809` | 없음 |
| [P28](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:16984) | GATE2 | 쿠크_카드미로연출 | MN_RPCZ_00 | 게시·수동 | 3 | 4 | 0 | `4219751` | `boss.kouku.ball.smoke` |

중요한 현재 경계: P21 레이저는 대포 WORLD 및 속박 Trigger가 있으나 광선 Effect box 0, P23 팡파레는 트럼펫 WORLD만 있고 Effect box 0, P24 휠윈드는 망치 WORLD와 충돌 로직이 있으나 Effect box 0, P27 대형 세이튼 불뿜기는 body clip만 있고 Effect box·WORLD·Logic 0이다. P25 피자는 V2 17 occurrence가 연결돼 있지만 이것만으로 원본 모든 emitter가 복원됐다고 판정할 수 없다.

## G03. Bundle 전체

| Bundle ID | Gate | 이름 | 멤버 시작 | 현재 상태 |
| --- | --- | --- | --- | --- |
| kakulsaydon.bundle.1 | GATE2 | 세이튼등장_동시 | `KAKULSAYDON_G1_PATTERN_8 @0ms`, `KAKULSAYDON_G1_PATTERN_9 @0ms` | 게시·수동 |
| kakulsaydon.bundle.2 | GATE2 | 파1빨2_동시 | `KAKULSAYDON_G1_PATTERN_10 @0ms`, `KAKULSAYDON_G1_PATTERN_11 @0ms` | 게시·수동 |
| kakulsaydon.bundle.3 | GATE2 | 조커찾기_동시 | `KAKULSAYDON_G1_PATTERN_12 @0ms`, `KAKULSAYDON_G1_PATTERN_13 @0ms` | 게시·수동 |
| kakulsaydon.bundle.4 | GATE2 | 카드미로_동시 | `KAKULSAYDON_G1_PATTERN_28 @0ms` | 게시·수동 |
| kakulsaydon.bundle.5 | GATE2 | 쿠크피자_동시 | `KAKULSAYDON_G1_PATTERN_25 @0ms`, `KAKULSAYDON_G1_PATTERN_26 @0ms` | Bundle child is unavailable: KAKULSAYDON_G1_PATTERN_26: PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_26 |
| kakulsaydon.bundle.6 | GATE2 | 쿠크거미카운터_동시 | `KAKULSAYDON_G1_PATTERN_15 @0ms`, `KAKULSAYDON_G1_PATTERN_16 @0ms` | Bundle child is unavailable: KAKULSAYDON_G1_PATTERN_16: PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_16 |
| kakulsaydon.bundle.7 | GATE2 | 거대세이튼잡기_동시 | `KAKULSAYDON_G1_PATTERN_17 @0ms` | 게시·수동 |
| kakulsaydon.bundle.8 | GATE2 | 쿠크레이저_동시 | `KAKULSAYDON_G1_PATTERN_20 @0ms`, `KAKULSAYDON_G1_PATTERN_21 @0ms` | Bundle child is unavailable: KAKULSAYDON_G1_PATTERN_20: PRODUCT pattern has no stages: KAKULSAYDON_G1_PATTERN_20 |
| kakulsaydon.bundle.9 | GATE2 | 쿠크휠윈드_동시 | `KAKULSAYDON_G1_PATTERN_24 @0ms` | 게시·수동 |
| kakulsaydon.bundle.10 | GATE2 | 쿠크팡파레_동시 | `KAKULSAYDON_G1_PATTERN_23 @0ms` | 게시·수동 |

## G04. 28개 패턴의 실제 Stage와 Effect 시각 전체

각 패턴의 animation 표는 저장 순서 전부를 보여 준다. 누적 시각은 authored duration의 단순 합이며 Server 30 Hz tick의 반올림을 재현한 실측값은 아니다. source action/stage/slot은 저작 참고 출처이고 runtime은 생성 action ID와 clip을 사용한다. Effect 표는 발생 box마다 모두 기록하며 그룹의 하위 leaf는 G07에서 펼친다.

### KAKULSAYDON_G1_PATTERN_1 — GATE1 / 세이튼_무력화 시작

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g1.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:1892).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_1.stage.39 | 0–3000 | rpct00_idle_battle_1 | MN_RPCT_07/4219945/stage-000/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_1.stage.43 | 3000–5267 | rpct00_att_battle_6_04 | MN_RPCT_07/4219945/stage-004/animation-000 | 0/2267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_1.stage.51 | 5267–7934 | rpct00_att_battle_6_02 | MN_RPCT_07/4219945/stage-005/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_1.stage.53 | 7934–10601 | rpct00_att_battle_6_02 | MN_RPCT_07/4219945/stage-005/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_1.stage.54 | 10601–13268 | rpct00_att_battle_6_02 | MN_RPCT_07/4219945/stage-005/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_1.stage.55 | 13268–15935 | rpct00_att_battle_6_02 | MN_RPCT_07/4219945/stage-005/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_1.stage.52 | 15935–18135 | rpct00_att_battle_6_03 | MN_RPCT_07/4219945/stage-006/animation-000 | 0/2200/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_1.presentation.1 | boss.kouku.disarm.shield_1 / LEAF | 5263–15947 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.2 | boss.kouku.disarm.converge_1 / LEAF | 7263–14263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.3 | boss.kouku.blur_1 / LEAF | 7263–14263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.4 | boss.kouku.disarm.converge_2 / LEAF | 7263–14263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.5 | boss.kouku.disarm.converge_3 / LEAF | 7263–14263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.6 | boss.kouku.disarm.star_1 / LEAF | 7263–8263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.7 | boss.kouku.disarm.star_1 / LEAF | 6263–7263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.8 | boss.kouku.disarm.star_1 / LEAF | 8263–9263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.9 | boss.kouku.disarm.star_1 / LEAF | 9263–10263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.10 | boss.kouku.disarm.star_1 / LEAF | 10263–11263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.11 | boss.kouku.disarm.star.smoke_1 / LEAF | 6263–7863 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.12 | boss.kouku.disarm.star.smoke_1 / LEAF | 7263–8863 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.13 | boss.kouku.disarm.star.smoke_1 / LEAF | 8263–9863 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.14 | boss.kouku.disarm.star.smoke_1 / LEAF | 9263–10863 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.15 | boss.kouku.disarm.star.smoke_1 / LEAF | 10263–11863 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.16 | boss.kouku.disarm.shield_1 / LEAF | 5263–15947 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.17 | boss.kouku.disarm.star.decal_1 / LEAF | 6263–15263 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.18 | boss.kouku.disarm.star.decal_1 / LEAF | 7263–15947 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.19 | boss.kouku.disarm.star.decal_1 / LEAF | 8263–15947 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.20 | boss.kouku.disarm.star.decal_1 / LEAF | 9263–15947 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_1.presentation.21 | boss.kouku.disarm.star.decal_1 / LEAF | 10263–15947 | BOSS/BODY//True |

Logic 연결: `kakulsaydon.g1.logic.1` 세이튼로직_방패무력화(STAGGER_WINDOW) ×1.

### KAKULSAYDON_G1_PATTERN_2 — GATE1 / 세이튼_진짜세이튼찾기

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g1.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:2960).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_2.stage.1 | 0–2067 | rpct00_att_phase1_1_01 | MN_RPCT_05/4219823/stage-000/animation-000 | 0/2067/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.2 | 2067–3900 | rpct00_att_phase1_1_06 | MN_RPCT_05/4219824/stage-000/animation-000 | 0/1833/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.3 | 3900–4767 | rpct00_att_battle_20_02 | MN_RPCT_05/4219842/stage-000/animation-000 | 0/867/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.4 | 4767–6767 | rpct00_att_battle_20_03 | MN_RPCT_05/4219842/stage-001/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.5 | 6767–9167 | rpct00_att_battle_20_05 | MN_RPCT_05/4219842/stage-002/animation-000 | 0/2400/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.6 | 9167–11167 | rpct00_att_battle_20_03 | MN_RPCT_05/4219842/stage-003/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.7 | 11167–13167 | rpct00_att_battle_20_03 | MN_RPCT_05/4219842/stage-004/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.8 | 13167–15567 | rpct00_att_battle_20_05 | MN_RPCT_05/4219842/stage-005/animation-000 | 0/2400/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.9 | 15567–17567 | rpct00_att_battle_20_03 | MN_RPCT_05/4219842/stage-006/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.10 | 17567–19567 | rpct00_att_battle_20_03 | MN_RPCT_05/4219842/stage-007/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.11 | 19567–21967 | rpct00_att_battle_20_05 | MN_RPCT_05/4219842/stage-008/animation-000 | 0/2400/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.12 | 21967–23967 | rpct00_att_battle_20_03 | MN_RPCT_05/4219842/stage-009/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_2.stage.13 | 23967–26134 | rpct00_att_battle_20_04 | MN_RPCT_05/4219842/stage-010/animation-000 | 0/2167/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_2.presentation.1 | boss.kouku.find.heart / GROUP | 6767–9167 | BOSS/BODY/b_effectroot/True |
| KAKULSAYDON_G1_PATTERN_2.presentation.2 | boss.kouku.find.heart / GROUP | 13167–15567 | BOSS/BODY/b_effectroot/True |
| KAKULSAYDON_G1_PATTERN_2.presentation.3 | boss.kouku.find.heart / GROUP | 19567–21967 | BOSS/BODY/b_effectroot/True |
| KAKULSAYDON_G1_PATTERN_2.presentation.6 | boss.kouku.find.core / GROUP | 3900–23967 | BOSS/BODY//True |

Logic 연결: `kakulsaydon.g1.logic.9` 세이튼로직_순간이동(REAL_GAZE_TELEPORT) ×1; `kakulsaydon.g1.logic.16` 세이튼로직_진짜세이튼찾기(GAZE_REAL_BOSS) ×1.

### KAKULSAYDON_G1_PATTERN_3 — GATE1 / 세이튼_무력화실패

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g1.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:3537).

Stage·animation·Effect가 없는 저장 항목이다.

### KAKULSAYDON_G1_PATTERN_4 — GATE1 / 세이튼_무력화성공

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g1.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:3560).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_4.stage.2 | 0–1167 | rpct00_dmg_critical_start_1 | MN_RPCT_07/4219946/stage-001/animation-000 | 0/1167/1/EXACT |
| KAKULSAYDON_G1_PATTERN_4.stage.3 | 1167–2834 | rpct00_dmg_critical_loop_1 | MN_RPCT_07/4219946/stage-002/animation-000 | 0/1667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_4.stage.4 | 2834–4167 | rpct00_dmg_critical_end_1 | MN_RPCT_07/4219946/stage-003/animation-000 | 0/1333/1/EXACT |

직접 Effect occurrence 없음.

### KAKULSAYDON_G1_PATTERN_5 — GATE1 / 세이튼_가짜세이튼

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g1.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:3650).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_5.stage.38 | 0–1833 | rpct00_att_phase1_1_06 | MN_RPCT_05/4219857/stage-001/animation-000 | 0/1833/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.25 | 1833–2700 | rpct00_att_battle_20_02 | MN_RPCT_05/4219891/stage-000/animation-000 | 0/867/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.26 | 2700–4700 | rpct00_att_battle_20_03 | MN_RPCT_05/4219891/stage-001/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.27 | 4700–7100 | rpct00_att_battle_20_06 | MN_RPCT_05/4219891/stage-002/animation-000 | 0/2400/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.28 | 7100–9100 | rpct00_att_battle_20_03 | MN_RPCT_05/4219891/stage-003/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.29 | 9100–11100 | rpct00_att_battle_20_03 | MN_RPCT_05/4219891/stage-004/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.30 | 11100–13500 | rpct00_att_battle_20_06 | MN_RPCT_05/4219891/stage-005/animation-000 | 0/2400/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.31 | 13500–15500 | rpct00_att_battle_20_03 | MN_RPCT_05/4219891/stage-006/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.32 | 15500–17500 | rpct00_att_battle_20_03 | MN_RPCT_05/4219891/stage-007/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.33 | 17500–19900 | rpct00_att_battle_20_06 | MN_RPCT_05/4219891/stage-008/animation-000 | 0/2400/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.34 | 19900–21900 | rpct00_att_battle_20_03 | MN_RPCT_05/4219891/stage-009/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.35 | 21900–23900 | rpct00_att_battle_20_03 | MN_RPCT_05/4219891/stage-010/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_5.stage.36 | 23900–26900 | rpct00_idle_battle_1 | MN_RPCT_05/4219891/stage-011/animation-000 | 0/3000/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_5.presentation.1 | boss.kouku.find.star / GROUP | 6166–7100 | BOSS/BODY/b_effectroot/False |
| KAKULSAYDON_G1_PATTERN_5.presentation.2 | boss.kouku.find.star / GROUP | 12566–13500 | BOSS/BODY/b_effectroot/False |
| KAKULSAYDON_G1_PATTERN_5.presentation.3 | boss.kouku.find.star / GROUP | 18966–19900 | BOSS/BODY/b_effectroot/False |
| KAKULSAYDON_G1_PATTERN_5.presentation.4 | boss.kouku.find.core / GROUP | 1833–23833 | BOSS/BODY//True |

### KAKULSAYDON_G1_PATTERN_6 — GATE1 / 세이튼_댄스타임

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g1.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:4113).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_6.stage.503 | 0–2000 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.584 | 2000–4000 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.570 | 4000–5000 | rpct00_att_battle_25_04_start | MN_RPCT_05/4219895/stage-067/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.571 | 5000–5500 | rpct00_att_battle_25_04_loop | MN_RPCT_05/4219895/stage-068/animation-000 | 0/500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.572 | 5500–6500 | rpct00_att_battle_25_04_end | MN_RPCT_05/4219895/stage-069/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.573 | 6500–7500 | rpct00_att_battle_25_07 | MN_RPCT_05/4219895/stage-070/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.585 | 7500–9500 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.586 | 9500–11500 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.574 | 11500–12500 | rpct00_att_battle_25_05_start | MN_RPCT_05/4219895/stage-071/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.576 | 12500–13500 | rpct00_att_battle_25_05_end | MN_RPCT_05/4219895/stage-073/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.582 | 13500–15500 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.581 | 15500–17500 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.577 | 17500–18500 | rpct00_att_battle_25_06_start | MN_RPCT_05/4219895/stage-074/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.578 | 18500–19300 | rpct00_att_battle_25_06_loop | MN_RPCT_05/4219895/stage-075/animation-000 | 0/800/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.579 | 19300–20300 | rpct00_att_battle_25_06_end | MN_RPCT_05/4219895/stage-076/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.583 | 20300–22300 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.589 | 22300–23300 | rpct00_att_battle_25_03_start | MN_RPCT_05/4219897/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.590 | 23300–23800 | rpct00_att_battle_25_03_loop | MN_RPCT_05/4219897/stage-002/animation-000 | 0/500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.591 | 23800–24800 | rpct00_att_battle_25_03_end | MN_RPCT_05/4219897/stage-003/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.587 | 24800–26800 | rpct00_att_battle_25_02 | MN_RPCT_05/4219895/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_6.stage.580 | 26800–31467 | rpct00_att_battle_13_01 | MN_RPCT_05/4219895/stage-077/animation-000 | 0/4667/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_6.presentation.1 | boss.kouku.curtain_1 / LEAF | 0–2671 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.4 | boss.kouku.dance / GROUP | 0–26800 | BOSS/BODY//False |
| KAKULSAYDON_G1_PATTERN_6.presentation.6 | boss.kouku.dance.clap / GROUP | 3556–5556 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.7 | boss.kouku.dance.clap / GROUP | 7951–9951 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.8 | boss.kouku.dance.clap / GROUP | 9039–11039 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.9 | boss.kouku.dance.clap / GROUP | 10013–12013 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.10 | boss.kouku.dance.clap / GROUP | 10965–12965 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.11 | boss.kouku.dance.clap / GROUP | 14046–16046 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.12 | boss.kouku.dance.clap / GROUP | 15043–17043 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.13 | boss.kouku.dance.clap / GROUP | 15971–17971 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.14 | boss.kouku.dance.clap / GROUP | 17059–19059 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.15 | boss.kouku.dance.clap / GROUP | 20774–22774 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.16 | boss.kouku.dance.clap / GROUP | 21794–23794 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.17 | boss.kouku.dance.clap / GROUP | 25328–27328 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.18 | boss.kouku.dance.clap / GROUP | 26370–28370 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_6.presentation.19 | boss.kouku.dance.clap / GROUP | 2515–4515 | BOSS/BODY//True |

WORLD 연결: `kakulsaydon.g1.world.2` 월드_커튼 ×1.

Logic 연결: `kakulsaydon.g1.logic.10` 세이튼로직_슈퍼맨(POSE_INPUT) ×1; `kakulsaydon.g1.logic.11` 세이튼로직_양팔벌리기(POSE_INPUT) ×1; `kakulsaydon.g1.logic.12` 세이튼로직_한다리올리기(POSE_INPUT) ×1; `kakulsaydon.g1.logic.13` 세이튼로직_양팔모으기(POSE_INPUT) ×1.

### KAKULSAYDON_G1_PATTERN_7 — GATE1 / 세이튼_룰렛

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g1.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:5366).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_7.stage.1 | 0–4333 | rpct00_att_battle_12_06 | MN_RPCT_05/4219811/stage-000/animation-000 | 0/4333/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.3 | 4333–9000 | rpct00_att_battle_13_01 | MN_RPCT_05/4219813/stage-001/animation-000 | 0/4667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.4 | 9000–11667 | rpct00_att_battle_13_02 | MN_RPCT_05/4219813/stage-002/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.10 | 11667–16334 | rpct00_att_battle_13_01 | MN_RPCT_05/4219813/stage-001/animation-000 | 0/4667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.11 | 16334–19001 | rpct00_att_battle_13_02 | MN_RPCT_05/4219813/stage-002/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.12 | 19001–23668 | rpct00_att_battle_13_01 | MN_RPCT_05/4219813/stage-001/animation-000 | 0/4667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.13 | 23668–26335 | rpct00_att_battle_13_02 | MN_RPCT_05/4219813/stage-002/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.14 | 26335–31002 | rpct00_att_battle_13_01 | MN_RPCT_05/4219813/stage-001/animation-000 | 0/4667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_7.stage.15 | 31002–33669 | rpct00_att_battle_13_02 | MN_RPCT_05/4219813/stage-002/animation-000 | 0/2667/1/EXACT |

직접 Effect occurrence 없음.

WORLD 연결: `kakulsaydon.g1.world.1` 월드_룰렛 ×1.

Logic 연결: `kakulsaydon.g1.logic.14` 세이튼_룰렛_1회차(ROULETTE_CARD_MATCH) ×1; `kakulsaydon.g1.logic.17` 세이튼_룰렛_2회차(ROULETTE_CARD_MATCH) ×1; `kakulsaydon.g1.logic.18` 세이튼_룰렛_3회차(ROULETTE_CARD_MATCH) ×1; `kakulsaydon.g1.logic.19` 세이튼로직_룰렛4회차(AREA_OVERLAP) ×1.

### KAKULSAYDON_G1_PATTERN_8 — GATE2 / 쿠크_세이튼등장

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:6694).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_8.stage.3 | 0–7400 | rpcz00_att_battle_7_01 | MN_RPCZ_00/4219719/stage-000/animation-000 | 0/7400/1/EXACT |

직접 Effect occurrence 없음.

WORLD 연결: `kakulsaydon.g1.world.6` 월드오브젝트_공 ×1.

### KAKULSAYDON_G1_PATTERN_9 — GATE2 / 대형세이튼_세이튼등장

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:6764).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_9.stage.1 | 0–2000 | mn_rpct_06_sk.ao_respawn_1 | MN_RPCT_06/15/stage-002/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_9.stage.2 | 2000–4667 | mn_rpct_06_sk.ao_idle_normal_1 | MN_RPCT_06/0/stage-003/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_9.stage.3 | 4667–7334 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/0/stage-006/animation-000 | 0/2667/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_9.presentation.1 | boss.kouku.appear / GROUP | 0–3000 | BOSS/BODY//True |

### KAKULSAYDON_G1_PATTERN_10 — GATE2 / 쿠크_파1빨2

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:6893).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_10.stage.1 | 0–2000 | rpcz00_idle_battle_1 | MN_RPCZ_00/4219728/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_10.stage.2 | 2000–4000 | rpcz00_idle_battle_1 | MN_RPCZ_00/4219728/stage-001/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_10.stage.3 | 4000–6667 | rpcz00_att_battle_5_01 | MN_RPCZ_00/4219728/stage-002/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_10.stage.4 | 6667–7667 | rpcz00_att_battle_5_02 | MN_RPCZ_00/4219728/stage-003/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_10.stage.5 | 7667–9900 | rpcz00_att_battle_5_03 | MN_RPCZ_00/4219728/stage-004/animation-000 | 0/2233/1/EXACT |

직접 Effect occurrence 없음.

### KAKULSAYDON_G1_PATTERN_11 — GATE2 / 대형세이튼_파1빨2

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:7027).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_11.stage.1 | 0–4000 | mn_rpct_06_sk.ao_att_battle_7_01 | MN_RPCT_06/4221813/stage-000/animation-000 | 0/4000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_11.stage.3 | 4000–6667 | mn_rpct_06_sk.ao_idle_normal_1 | MN_RPCT_06/0/stage-003/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_11.stage.2 | 6667–10667 | mn_rpct_06_sk.ao_att_battle_7_01 | MN_RPCT_06/4221813/stage-001/animation-000 | 0/4000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_11.stage.4 | 10667–13334 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/0/stage-006/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_11.stage.5 | 13334–17334 | mn_rpct_06_sk.ao_att_battle_7_01 | MN_RPCT_06/4221813/stage-001/animation-000 | 0/4000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_11.stage.6 | 17334–20001 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/0/stage-006/animation-000 | 0/2667/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_11.presentation.4 | boss.kouku.medusa.blue / GROUP | 2495–20001 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_11.presentation.5 | boss.kouku.medusa.red / GROUP | 2510–20001 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_11.presentation.6 | boss.kouku.medusa.blue / GROUP | 2510–20001 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_11.presentation.7 | boss.kouku.medusa.blue / GROUP | 2480–20001 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_11.presentation.8 | boss.kouku.medusa.laser / GROUP | 1312–4312 | BOSS/BODY/bip001-head/True |
| KAKULSAYDON_G1_PATTERN_11.presentation.9 | boss.kouku.medusa.laser / GROUP | 7745–10745 | BOSS/BODY/bip001-head/True |
| KAKULSAYDON_G1_PATTERN_11.presentation.10 | boss.kouku.medusa.laser / GROUP | 14453–17453 | BOSS/BODY/bip001-head/True |

Logic 연결: `kakulsaydon.g1.logic.36` 세이튼로직_파1빨2무적(GAZE_REAL_BOSS) ×3.

### KAKULSAYDON_G1_PATTERN_12 — GATE2 / 쿠크_조커찾기

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:7487).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_12.stage.1 | 0–3867 | rpcz00_idle_normal_1 | MN_RPCZ_00/0/stage-003/animation-000 | 0/3867/1/EXACT |
| KAKULSAYDON_G1_PATTERN_12.stage.2 | 3867–5867 | rpcz00_idle_battle_1 | MN_RPCZ_00/0/stage-006/animation-000 | 0/2000/1/EXACT |

직접 Effect occurrence 없음.

### KAKULSAYDON_G1_PATTERN_13 — GATE2 / 대형세이튼_조커찾기

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:7555).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_13.stage.1 | 0–2667 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.7 | 2667–3664 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/997/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.3 | 3664–4664 | mn_rpct_06_sk.ao_att_battle_8_01 | MN_RPCT_06/4221820/stage-000/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.5 | 4664–5664 | mn_rpct_06_sk.ao_att_battle_1_03 | MN_RPCT_06/4221820/stage-002/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.13 | 5664–6266 | mn_rpct_06_sk.ao_att_battle_1_01 | MN_RPCT_06/4221820/stage-001/animation-000 | 0/602/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.6 | 6266–8933 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.15 | 8933–9930 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/997/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.8 | 9930–10930 | mn_rpct_06_sk.ao_att_battle_8_01 | MN_RPCT_06/4221820/stage-000/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.9 | 10930–11930 | mn_rpct_06_sk.ao_att_battle_1_03 | MN_RPCT_06/4221820/stage-002/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.11 | 11930–12560 | mn_rpct_06_sk.ao_att_battle_1_01 | MN_RPCT_06/4221820/stage-001/animation-000 | 0/630/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.14 | 12560–15227 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.20 | 15227–17894 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.21 | 17894–18891 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/997/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.22 | 18891–19891 | mn_rpct_06_sk.ao_att_battle_8_01 | MN_RPCT_06/4221820/stage-000/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.23 | 19891–20891 | mn_rpct_06_sk.ao_att_battle_1_03 | MN_RPCT_06/4221820/stage-002/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.24 | 20891–21521 | mn_rpct_06_sk.ao_att_battle_1_01 | MN_RPCT_06/4221820/stage-001/animation-000 | 0/630/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.25 | 21521–24188 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.2 | 24188–25185 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221821/stage-000/animation-000 | 0/997/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.16 | 25185–26185 | mn_rpct_06_sk.ao_att_battle_8_01 | MN_RPCT_06/4221818/stage-000/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.17 | 26185–27185 | mn_rpct_06_sk.ao_att_battle_8_02 | MN_RPCT_06/4221818/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_13.stage.18 | 27185–30353 | mn_rpct_06_sk.ao_att_battle_8_03 | MN_RPCT_06/4221818/stage-002/animation-000 | 0/2600/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_13.presentation.23 | boss.kouku.joker.hammer / GROUP | 11337–14337 | BOSS/WEAPON/b_rpct_01/True |
| KAKULSAYDON_G1_PATTERN_13.presentation.24 | boss.kouku.joker.hammer / GROUP | 20244–23244 | BOSS/WEAPON/b_rpct_01/True |
| KAKULSAYDON_G1_PATTERN_13.presentation.25 | boss.kouku.joker.hammer / GROUP | 29193–30353 | BOSS/WEAPON/b_rpct_01/True |
| KAKULSAYDON_G1_PATTERN_13.presentation.26 | boss.kouku.joker.hammer / GROUP | 4868–7868 | BOSS/WEAPON/b_rpct_01/True |

WORLD 연결: `kakulsaydon.g1.world.4` 월드오브젝트_카드 ×6; `kakulsaydon.g1.world.5` 월드오브젝트_조커카드 ×1.

Logic 연결: `kakulsaydon.g1.logic.22` 대형세이튼로직_카드뒤집기(OBJECT_CONTACT) ×3; `kakulsaydon.g1.logic.23` 대형세이튼로직_카드들썩임(OBJECT_CONTACT) ×3; `kakulsaydon.g1.logic.20` 2관문_거대세이튼_조커찾기(EXTERNAL_SIGNAL) ×1.

### KAKULSAYDON_G1_PATTERN_14 — GATE2 / 대형세이튼_조커찾기_성공

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:8819).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_14.stage.1 | 0–2333 | mn_rpct_06_sk.ao_att_battle_4_02 | MN_RPCT_06/4221816/stage-000/animation-000 | 0/2333/1/EXACT |

직접 Effect occurrence 없음.

### KAKULSAYDON_G1_PATTERN_15 — GATE2 / 쿠크_거미카운터

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:8865).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_15.stage.1 | 0–2500 | rpcz00_att_battle_6_01 | MN_RPCZ_00/4219776/stage-000/animation-000 | 0/2500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.2 | 2500–3500 | rpcz00_att_battle_6_02 | MN_RPCZ_00/4219776/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.26 | 3500–4500 | rpcz00_att_battle_6_02 | MN_RPCZ_00/4219776/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.3 | 4500–5667 | rpcz00_att_battle_6_03 | MN_RPCZ_00/4219776/stage-002/animation-000 | 0/1167/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.4 | 5667–7167 | rpcz00_att_battle_6_04 | MN_RPCZ_00/4219776/stage-003/animation-000 | 0/1500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.28 | 7167–9667 | rpcz00_att_battle_6_01 | MN_RPCZ_00/4219776/stage-000/animation-000 | 0/2500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.29 | 9667–10667 | rpcz00_att_battle_6_02 | MN_RPCZ_00/4219776/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.30 | 10667–11667 | rpcz00_att_battle_6_02 | MN_RPCZ_00/4219776/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.31 | 11667–12834 | rpcz00_att_battle_6_03 | MN_RPCZ_00/4219776/stage-002/animation-000 | 0/1167/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.33 | 12834–14334 | rpcz00_att_battle_6_04 | MN_RPCZ_00/4219776/stage-003/animation-000 | 0/1500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.34 | 14334–16834 | rpcz00_att_battle_6_01 | MN_RPCZ_00/4219776/stage-000/animation-000 | 0/2500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.35 | 16834–17834 | rpcz00_att_battle_6_02 | MN_RPCZ_00/4219776/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.36 | 17834–18834 | rpcz00_att_battle_6_02 | MN_RPCZ_00/4219776/stage-001/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.37 | 18834–20001 | rpcz00_att_battle_6_03 | MN_RPCZ_00/4219776/stage-002/animation-000 | 0/1167/1/EXACT |
| KAKULSAYDON_G1_PATTERN_15.stage.39 | 20001–21501 | rpcz00_att_battle_6_04 | MN_RPCZ_00/4219776/stage-003/animation-000 | 0/1500/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_15.presentation.4 | effect.valtan.project-tuned.sequence.counter / V1_ELEMENT | 1600–3280 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_15.presentation.5 | effect.valtan.project-tuned.sequence.counter / V1_ELEMENT | 8767–10447 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_15.presentation.6 | effect.valtan.project-tuned.sequence.counter / V1_ELEMENT | 15934–17614 | BOSS/BODY//True |

Logic 연결: `kakulsaydon.g1.logic.29` 2관문_쿠크_거미카운터_카운터(COUNTER_WINDOW) ×3; `kakulsaydon.g1.logic.31` 2관문_트리거_거미카운터_공포(ENTER_AREA) ×3.

### KAKULSAYDON_G1_PATTERN_16 — GATE2 / 대형세이튼_거미카운터

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:9521).

Stage·animation·Effect가 없는 저장 항목이다.

### KAKULSAYDON_G1_PATTERN_17 — GATE2 / 대형세이튼_잡기

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:9544).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_17.stage.1 | 0–4000 | mn_rpct_06_sk.ao_att_battle_6_01 | MN_RPCT_06/4221810/stage-000/animation-000 | 0/4000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_17.stage.2 | 4000–5500 | mn_rpct_06_sk.ao_att_battle_6_02 | MN_RPCT_06/4221810/stage-001/animation-000 | 0/1500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_17.stage.3 | 5500–7500 | mn_rpct_06_sk.ao_att_battle_6_04 | MN_RPCT_06/4221810/stage-002/animation-000 | 0/2000/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_17.presentation.2 | boss.kouku.joker.hammer / GROUP | 4264–4594 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_17.presentation.3 | boss.kouku.joker.hammer / GROUP | 4783–5113 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_17.presentation.4 | boss.kouku.joker.hammer / GROUP | 5308–5638 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_17.presentation.5 | boss.kouku.joker.hammer / GROUP | 7143–7473 | BOSS/BODY//True |

Logic 연결: `kakulsaydon.g1.logic.32` 대형세이튼_잡기판정(ENTER_AREA) ×1; `kakulsaydon.g1.logic.33` 대형세이튼_잡기유지(ATTACHMENT_HOLD) ×1.

### KAKULSAYDON_G1_PATTERN_25 — GATE2 / 쿠크_피자

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:9849).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_25.stage.1 | 0–2000 | rpcz00_idle_battle_1 | MN_RPCZ_00/4219769/stage-000/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.64 | 2000–3500 | rpcz00_att_battle_10_01 | MN_RPCZ_00/4219769/stage-002/animation-000 | 0/1500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.4 | 3500–5500 | rpcz00_att_battle_10_02 | MN_RPCZ_00/4219769/stage-003/animation-000 | 0/2000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.6 | 5500–6500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.19 | 6500–7500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.20 | 7500–8500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.21 | 8500–9500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.22 | 9500–10500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.23 | 10500–11500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.24 | 11500–12500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.25 | 12500–13500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.99 | 13500–14500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.100 | 14500–15500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.101 | 15500–16500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.102 | 16500–17500 | rpcz00_att_battle_10_03 | MN_RPCZ_00/4219769/stage-005/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_25.stage.60 | 17500–20167 | rpcz00_att_battle_10_04 | MN_RPCZ_00/4219769/stage-016/animation-000 | 0/2667/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_25.presentation.1 | boss.kouku.pizza.aura / GROUP | 5465–18306 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.2 | boss.kouku.pizza.star.cl / GROUP | 1985–2457 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.3 | boss.kouku.pizza.star.cl / GROUP | 3009–3407 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.4 | boss.kouku.pizza.star.ccl / GROUP | 2466–3000 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.5 | boss.kouku.pizza.w / GROUP | 7707–8207 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.6 | boss.kouku.pizza.s / GROUP | 6758–7258 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.7 | boss.kouku.pizza.e / GROUP | 5845–6317 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.8 | boss.kouku.pizza.n / GROUP | 8657–9157 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.9 | boss.kouku.pizza.e / GROUP | 9684–10156 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.10 | boss.kouku.pizza.n / GROUP | 10637–11137 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.11 | boss.kouku.pizza.w / GROUP | 11611–12111 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.12 | boss.kouku.pizza.s / GROUP | 12610–13110 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.14 | boss.kouku.pizza.e / GROUP | 13586–14058 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.15 | boss.kouku.pizza.s / GROUP | 14526–15026 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.16 | boss.kouku.pizza.w / GROUP | 15495–15995 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.17 | boss.kouku.pizza.n / GROUP | 16506–17006 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_25.presentation.18 | boss.kouku.pizza.e / GROUP | 17519–17991 | BOSS/BODY//True |

### KAKULSAYDON_G1_PATTERN_26 — GATE2 / 대형세이튼_피자

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:10872).

Stage·animation·Effect가 없는 저장 항목이다.

### KAKULSAYDON_G1_PATTERN_20 — GATE2 / 대형세이튼_레이저

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:10895).

Stage·animation·Effect가 없는 저장 항목이다.

### KAKULSAYDON_G1_PATTERN_21 — GATE2 / 쿠크_레이저

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:10918).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_21.stage.2 | 0–2333 | rpcz00_att_battle_1_03 | MN_RPCZ_00/4219740/stage-001/animation-000 | 0/2333/1/EXACT |
| KAKULSAYDON_G1_PATTERN_21.stage.3 | 2333–4833 | rpcz00_att_battle_1_04 | MN_RPCZ_00/4219740/stage-002/animation-000 | 0/2500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_21.stage.4 | 4833–5833 | rpcz00_att_battle_1_07 | MN_RPCZ_00/4219740/stage-003/animation-000 | 0/1000/1/EXACT |

직접 Effect occurrence 없음.

WORLD 연결: `kakulsaydon.g1.world.15` 월드오브젝트_쿠크레이저대포 ×1.

Logic 연결: `kakulsaydon.g1.logic.41` 2관문_쿠크_레이저(ENTER_AREA) ×7.

### KAKULSAYDON_G1_PATTERN_22 — GATE2 / 쿠크_그로기

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:11368).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_22.stage.1 | 0–1000 | rpcz00_dmg_critical_start_1 | MN_RPCZ_00/4219763/stage-000/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_22.stage.2 | 1000–2333 | rpcz00_dmg_critical_loop_1 | MN_RPCZ_00/4219763/stage-001/animation-000 | 0/1333/1/LOOP_TO_WINDOW |
| KAKULSAYDON_G1_PATTERN_22.stage.3 | 2333–3500 | rpcz00_dmg_critical_end_1 | MN_RPCZ_00/4219763/stage-002/animation-000 | 0/1167/1/EXACT |

직접 Effect occurrence 없음.

### KAKULSAYDON_G1_PATTERN_23 — GATE2 / 쿠크_팡파레

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:11459).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_23.stage.1 | 0–2500 | rpcz00_att_battle_3_01 | MN_RPCZ_00/4219714/stage-000/animation-000 | 0/2500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_23.stage.2 | 2500–7167 | rpcz00_att_battle_3_07 | MN_RPCZ_00/4219714/stage-001/animation-000 | 0/4667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_23.stage.3 | 7167–8167 | rpcz00_att_battle_3_09 | MN_RPCZ_00/4219714/stage-002/animation-000 | 0/1000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_23.stage.4 | 8167–10667 | rpcz00_att_battle_3_01 | MN_RPCZ_00/4219714/stage-003/animation-000 | 0/2500/1/EXACT |

직접 Effect occurrence 없음.

WORLD 연결: `kakulsaydon.g1.world.14` 월드오브젝트_쿠크트럼펫 ×1.

### KAKULSAYDON_G1_PATTERN_24 — GATE2 / 쿠크_휠윈드

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:11732).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_24.stage.1 | 0–2500 | rpcz00_att_battle_2_01 | MN_RPCZ_00/4219708/stage-000/animation-000 | 0/2500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.2 | 2500–4000 | rpcz00_att_battle_2_04 | MN_RPCZ_00/4219708/stage-001/animation-000 | 0/1500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.3 | 4000–4267 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.11 | 4267–4534 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.12 | 4534–4801 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.13 | 4801–5068 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.14 | 5068–5335 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.15 | 5335–5602 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.16 | 5602–5869 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.17 | 5869–6136 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.18 | 6136–6403 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.19 | 6403–6670 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.20 | 6670–6937 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.21 | 6937–7204 | rpcz00_att_battle_2_05 | MN_RPCZ_00/4219708/stage-002/animation-000 | 0/267/1/EXACT |
| KAKULSAYDON_G1_PATTERN_24.stage.4 | 7204–8104 | rpcz00_att_battle_2_06 | MN_RPCZ_00/4219708/stage-003/animation-000 | 0/900/1/EXACT |

직접 Effect occurrence 없음.

WORLD 연결: `kakulsaydon.g1.world.18` 월드오브젝트_쿠크뿅망치 ×1.

Logic 연결: `kakulsaydon.g1.logic.38` 2관문_쿠크_휠윈드(ENTER_AREA) ×1.

### KAKULSAYDON_G1_PATTERN_18 — GATE3 / 3관문_외곽불회전_갈고리대각선_시각테스트

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g3.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:12146).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_18.stage.1 | 0–3000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.2 | 3000–6000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.3 | 6000–9000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.4 | 9000–12000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.5 | 12000–15000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.6 | 15000–18000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.7 | 18000–21000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.8 | 21000–24000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.9 | 24000–27000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.10 | 27000–30000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.11 | 30000–33000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_18.stage.12 | 33000–36000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |

직접 Effect occurrence 없음.

WORLD 연결: `kakulsaydon.g1.world.7` 3관문_추가외곽불_D_CW ×10; `kakulsaydon.g1.world.8` 3관문_추가외곽불_D_CCW ×10; `kakulsaydon.g1.world.9` 3관문_추가외곽불_E_CW ×10; `kakulsaydon.g1.world.10` 3관문_추가외곽불_E_CCW ×10; `kakulsaydon.g1.world.11` 3관문_추가외곽불_F_CW ×10; `kakulsaydon.g1.world.12` 3관문_추가외곽불_F_CCW ×10; `kakulsaydon.g1.world.13` 3관문_갈고리_대각선 ×18.

Logic 연결: `kakulsaydon.g1.logic.27` 3관문_갈고리_접촉(ENTER_AREA) ×18.

### KAKULSAYDON_G1_PATTERN_19 — GATE3 / 3관문_갈고리만_확인용_불없음

배우 `MN_RPCT_05`, 배치 `boss.kakulsaydon.g3.saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:15209).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_19.stage.1 | 0–3000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.2 | 3000–6000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.3 | 6000–9000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.4 | 9000–12000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.5 | 12000–15000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.6 | 15000–18000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.7 | 18000–21000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.8 | 21000–24000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.9 | 24000–27000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.10 | 27000–30000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.11 | 30000–33000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |
| KAKULSAYDON_G1_PATTERN_19.stage.12 | 33000–36000 | rpct00_idle_normal_1 | MN_RPCT_07/0/stage-003/animation-000 | 0/3000/1/EXACT |

직접 Effect occurrence 없음.

WORLD 연결: `kakulsaydon.g1.world.13` 3관문_갈고리_대각선 ×18.

Logic 연결: `kakulsaydon.g1.logic.27` 3관문_갈고리_접촉(ENTER_AREA) ×18.

### KAKULSAYDON_G1_PATTERN_27 — GATE2 / 대형세이튼_불뿜기

배우 `MN_RPCT_06`, 배치 `boss.kakulsaydon.g2.big-saydon`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:16832).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_27.stage.1 | 0–2667 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221809/stage-000/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_27.stage.2 | 2667–8167 | mn_rpct_06_sk.ao_att_battle_5_02 | MN_RPCT_06/4221809/stage-001/animation-000 | 0/5500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_27.stage.3 | 8167–10834 | mn_rpct_06_sk.ao_idle_battle_1 | MN_RPCT_06/4221809/stage-002/animation-000 | 0/2667/1/EXACT |
| KAKULSAYDON_G1_PATTERN_27.stage.4 | 10834–16334 | mn_rpct_06_sk.ao_att_battle_5_02 | MN_RPCT_06/4221809/stage-003/animation-000 | 0/5500/1/EXACT |

직접 Effect occurrence 없음.

### KAKULSAYDON_G1_PATTERN_28 — GATE2 / 쿠크_카드미로연출

배우 `MN_RPCZ_00`, 배치 `boss.kakulsaydon.g2.kouku`. [현재 저장본](C:/Users/user/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:16984).

| 생성 action ID | 누적 ms | 실제 runtimeClip | 원본 참고 profile/action/stage/slot | sourceStart/playMs/rate/end |
| --- | --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_28.stage.27 | 0–2500 | rpcz00_att_battle_3_01 | MN_RPCZ_00/4219751/stage-000/animation-000 | 0/2500/1/EXACT |
| KAKULSAYDON_G1_PATTERN_28.stage.28 | 2500–4967 | rpcz00_att_battle_3_05 | MN_RPCZ_00/4219751/stage-001/animation-000 | 0/2467/1/EXACT |
| KAKULSAYDON_G1_PATTERN_28.stage.29 | 4967–6634 | rpcz00_att_battle_3_10 | MN_RPCZ_00/4219751/stage-002/animation-000 | 0/1667/1/EXACT |

| Effect occurrence ID | asset ID / kind | 시작–종료 ms | anchor / bone / follow |
| --- | --- | --- | --- |
| KAKULSAYDON_G1_PATTERN_28.presentation.1 | boss.kouku.ball.smoke / GROUP | 3326–3878 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_28.presentation.5 | boss.kouku.ball.smoke / GROUP | 3660–4212 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_28.presentation.6 | boss.kouku.ball.smoke / GROUP | 3994–4546 | BOSS/BODY//True |
| KAKULSAYDON_G1_PATTERN_28.presentation.7 | boss.kouku.ball.smoke / GROUP | 4355–4907 | BOSS/BODY//True |

Logic 연결: `kakulsaydon.g1.logic.40` 2관문_카드미로진입(TRIGGER) ×1.

## G05. WORLD 소품·3관문·빙고의 별도 연결

WORLD 정본은 [LV_LUT_MIDNIGHTC_ED.worldsequences.json](C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json:1) revision 427, objectResources 24개다. 아래는 Composition이 등록한 18개 WORLD 정의 전부이며 실제 사용 여부를 함께 표시한다.

| WORLD ID | 이름 | 실제 소비 Pattern | 소품 animation | WORLD 자체 Effect | target ID |
| --- | --- | --- | --- | --- | --- |
| kakulsaydon.g1.world.1 | 월드_룰렛 | `KAKULSAYDON_G1_PATTERN_7` | 없음 | 없음 | `40` |
| kakulsaydon.g1.world.2 | 월드_커튼 | `KAKULSAYDON_G1_PATTERN_6` | 없음 | 없음 | `15287122004384700644`, `9692865147248617345`, `11270101561827041056`, `12007891572504947732`, `16275700054519776934`, `13139123088456067908`, `10624515348668145957`, `17635721713208030005`, `15001057136353074761`, `14189800010255709501`, `12384299562106759874` |
| kakulsaydon.g1.world.3 | 월드오브젝트_카드 / 카드_Idle | 없음 | `mn_rhoc_00_sk.ao_idle_normal_1` | 없음 | `world.object.kouku.card` |
| kakulsaydon.g1.world.4 | 월드오브젝트_카드 | `KAKULSAYDON_G1_PATTERN_13` | `mn_rhoc_00_sk.ao_idle_normal_1` | 없음 | `world.object.kouku.card` |
| kakulsaydon.g1.world.5 | 월드오브젝트_조커카드 | `KAKULSAYDON_G1_PATTERN_13` | `mn_rhoc_00_sk.ao_idle_normal_1` | 없음 | `world.object.kouku.joker_card` |
| kakulsaydon.g1.world.6 | 월드오브젝트_공 | `KAKULSAYDON_G1_PATTERN_8` | 없음 | `boss.kouku.ball.smoke` | `world.object.kouku.ball` |
| kakulsaydon.g1.world.7 | 3관문_추가외곽불_D_CW | `KAKULSAYDON_G1_PATTERN_18` | 없음 | 없음 | `world.object.kouku.g3.outer_fire.d` |
| kakulsaydon.g1.world.8 | 3관문_추가외곽불_D_CCW | `KAKULSAYDON_G1_PATTERN_18` | 없음 | 없음 | `world.object.kouku.g3.outer_fire.d` |
| kakulsaydon.g1.world.9 | 3관문_추가외곽불_E_CW | `KAKULSAYDON_G1_PATTERN_18` | 없음 | 없음 | `world.object.kouku.g3.outer_fire.e` |
| kakulsaydon.g1.world.10 | 3관문_추가외곽불_E_CCW | `KAKULSAYDON_G1_PATTERN_18` | 없음 | 없음 | `world.object.kouku.g3.outer_fire.e` |
| kakulsaydon.g1.world.11 | 3관문_추가외곽불_F_CW | `KAKULSAYDON_G1_PATTERN_18` | 없음 | 없음 | `world.object.kouku.g3.outer_fire.f` |
| kakulsaydon.g1.world.12 | 3관문_추가외곽불_F_CCW | `KAKULSAYDON_G1_PATTERN_18` | 없음 | 없음 | `world.object.kouku.g3.outer_fire.f` |
| kakulsaydon.g1.world.13 | 3관문_갈고리_대각선 | `KAKULSAYDON_G1_PATTERN_18`, `KAKULSAYDON_G1_PATTERN_19` | `Hook_idle_normal_1` | 없음 | `world.object.kouku.hook` |
| kakulsaydon.g1.world.14 | 월드오브젝트_쿠크트럼펫 | `KAKULSAYDON_G1_PATTERN_23` | 없음 | 없음 | `world.object.kouku.trumpet` |
| kakulsaydon.g1.world.15 | 월드오브젝트_쿠크레이저대포 | `KAKULSAYDON_G1_PATTERN_21` | 없음 | 없음 | `world.object.kouku.laser_cannon` |
| kakulsaydon.g1.world.16 | 월드오브젝트_세이튼총_왼손 | 없음 | 없음 | 없음 | `world.object.kouku.saydon_showtime_gun_left` |
| kakulsaydon.g1.world.17 | 월드오브젝트_세이튼총_오른손 | 없음 | 없음 | 없음 | `world.object.kouku.saydon_showtime_gun_right` |
| kakulsaydon.g1.world.18 | 월드오브젝트_쿠크뿅망치 | `KAKULSAYDON_G1_PATTERN_24` | 없음 | 없음 | `world.object.kouku.whirlwind_hammer` |

P18은 외곽불 WORLD 60개와 갈고리 WORLD 18개, P19는 갈고리 WORLD 18개를 재생한다. 두 패턴의 body animation은 `rpct00_idle_normal_1` 12회로 동일하다. 갈고리 자체 `Hook_idle_normal_1`과 이동 Transform이 별도 WORLD에서 동작하며 보스의 해당 원본 attack action을 연결한 완성 패턴은 아니다. Show Time 양손 총 WORLD 16/17은 정의가 있지만 현재 28개 Pattern에서는 소비하지 않는다.

빙고는 BINGO gate Pattern이 0개여도 별도 기능이 있다. 아래 행은 debug 입력으로 동작하는 Server 권위 상태와 V2 표현이다.

| 기능 | 현재 입력·Server 상태 | Client Effect | 남은 연결 |
| --- | --- | --- | --- |
| 빙고 판 | Bingo_Play1/Play2/Reset → DEBUG_BINGO_FILL → white/red mask | bingo.skull.white / bingo.skull.red | 보스 action/자동 빙고 진행 없음 |
| 빙고 폭탄 | Bingo_Bomb → MARKED → PLANTED → 십자 cell Detonate | bingo.bomb.mark → bingo.bomb | 현재 폭발은 판 mask 토글; attack clip·데미지·무력화 연결 없음 |
| 빙고 망치 | Bingo_Hammer → 20 anchor 중 선택 → RAISED/DESCENDING/SWEEPING | bingo.hammer | 별도 Server state의 pose 계산; boss pattern/action 및 damage 소비 없음 |

근거: [MainApp.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp:7685), [GameRoom.cpp](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:3420), [GameRoom.cpp](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:3506), [KoukuSaydonPresentationPlayer.cpp](C:/Users/user/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:2315). `CKoukuBingoRuntime::Is_Safe`는 현재 제품 호출자가 없고 contract test에서만 사용한다.

카드 미로는 별도 Server 상태의 player/target suit와 출구 상태를 Client가 `cardmaze.mark.*`, `cardmaze.exit.*` 8개 group으로 표현한다. 카드 문양 `boss.kouku.card.*` 8개 group도 Server card-state presentation에서 재생된다. 따라서 Composition의 직접 Effect 수에 이 그룹들을 임의 더해 원본 notify 복원 개수로 쓰면 안 된다.

## G06. clip binding, 수명과 full restore 경계

`MN_RPCT_05.effectv2bindings.json`은 빈 배열이다. `MN_RPCT_06.effectv2bindings.json`의 2개 legacy NPC binding은 [MN_RPCT_06.effectv2bindings.json](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Bindings/MN_RPCT_06.effectv2bindings.json:1)에 있다. `mn_rpct_06_sk.ao_att_battle_7_01` +1333ms에 `boss.kouku.medusa.laser`(left eye), `mn_rpct_06_sk.ao_att_battle_1_02` +100ms에 `boss.kouku.joker.hammer`(b_effectroot)를 실행하도록 선언돼 있고 둘 다 `stopWithClip=false`다. CNpc의 network animation 시작도 Notify_Clip을 호출한다([Npc.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Npc.cpp:251)). 이는 Composition box와 다른 소비 경로이며 full restore를 붙일 때 기존 row와 중복하는지 실제 owner/clip 기준으로 확인할 대상이다.

Composition 재생의 시간 정본은 clip 길이가 아니라 **Pattern occurrence의 startMs/durationMs**다. [KoukuSaydonPresentationPlayer.cpp](C:/Users/user/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:1055)는 box 바깥을 sample하지 않고, 범위에서 빠진 handle을 Stop_Group으로 정리한다. V2 particle은 emitter/loop 수명을 box 길이로 늘리지 않도록 duration=-1을 넘겨 원래 tail을 유지하되, box 종료 자체는 최종 정리 경계다([KoukuSaydonPresentationPlayer.cpp](C:/Users/user/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:1186)). 따라서 box를 너무 짧게 만들면 남아야 할 particle tail이 끊길 수 있다.

일반 legacy clip binding은 [EffectV2_Runtime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/EffectV2_Runtime.cpp:1395)에서 clip 전환 시 아직 생성되지 않은 Pending을 지운다. `stopWithClip=true`면 기존 활성 effect도 Finish한다. 즉 “짧은 clip 때문에 늦은 notify가 생성되기 전에 사라짐”이나 “clip/box 종료로 tail 중단”은 가능한 결함 유형이다. 그러나 이번 읽기 전용 조사에서 과거 캐릭터별 누락의 공통 원인으로 재현한 것은 아니다. 해당 action/clip/occurrence별 clock을 비교해야 한다.

현재 Kouku V2 leaf는 authored slots/params와 group child 조합이다. 예를 들어 무력화 방패는 Warlord mesh/texture를 함께 사용하며 수동 색·scale·lifetime을 갖는다. 이 구조·개수는 원본 패턴의 모든 particle system/emitter/material/notify를 원형 그대로 복구했다는 증거가 아니다. 복원은 원본 Action의 stage·notify occurrence를 identity로 삼고, body/weapon/child actor/WORLD 각 소유자를 보존한 상태로 animation box와 effect box를 연결해야 한다. 원본의 sourceStart/playRate, 시작 offset, emission 종료, tail, attach bone, world-space 전환, child actor 생명주기를 구분해 기록해야 한다.

## G07. 현재 Kouku·빙고·카드미로 V2 group/leaf 전체

파일명 prefix `boss.kouku.`, `bingo.`, `cardmaze.`로 실측한 group **41개**, leaf **84개**. 그룹 child 반복은 실제 반복 개수로 보존하며 leaf ID는 고유 개수와 함께 적었다. “직접 Pattern 연결 없음”은 preview/dynamic state/다른 group 소비까지 없다는 뜻이 아니다.

| Group·파일 | child 수 | 고유 leaf | 모든 child ID(반복수) | 직접 Pattern 소비 |
| --- | --- | --- | --- | --- |
| [bingo.bomb](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/bingo.bomb.effectv2group.json:1) | 1 | 1 | bingo.bomb_1 | 없음 |
| [bingo.bomb.mark](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/bingo.bomb.mark.effectv2group.json:1) | 1 | 1 | bingo.bomb.mark_1 | 없음 |
| [bingo.hammer](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/bingo.hammer.effectv2group.json:1) | 1 | 1 | bingo.hammer_1 | 없음 |
| [bingo.skull.red](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/bingo.skull.red.effectv2group.json:1) | 1 | 1 | bingo.skull.red_1 | 없음 |
| [bingo.skull.white](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/bingo.skull.white.effectv2group.json:1) | 1 | 1 | bingo.skull.white_1 | 없음 |
| [boss.kouku.appear](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.appear.effectv2group.json:1) | 2 | 2 | boss.kouku.appear.smoke_1; boss.kouku.appear.card_1 | `KAKULSAYDON_G1_PATTERN_9` |
| [boss.kouku.ball.smoke](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.ball.smoke.effectv2group.json:1) | 7 | 7 | boss.kouku.ball.smoke.green_1; boss.kouku.ball.smoke.yellow_1; boss.kouku.ball.smoke.orange_1; boss.kouku.ball.smoke.red_1; boss.kouku.ball.smoke.purple_1; boss.kouku.ball.smoke.blue_1; boss.kouku.ball.smoke.spread_1 | `KAKULSAYDON_G1_PATTERN_28` |
| [boss.kouku.card.clober.black](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.clober.black.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.clober.black_1 | 없음 |
| [boss.kouku.card.clober.red](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.clober.red.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.clober.red_1 | 없음 |
| [boss.kouku.card.dia.black](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.dia.black.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.dia.black_1 | 없음 |
| [boss.kouku.card.dia.red](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.dia.red.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.dia.red_1 | 없음 |
| [boss.kouku.card.heart.black](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.heart.black.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.heart.black_1 | 없음 |
| [boss.kouku.card.heart.red](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.heart.red.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.heart.red_1 | 없음 |
| [boss.kouku.card.spade.black](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.spade.black.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.spade.black_1 | 없음 |
| [boss.kouku.card.spade.red](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.card.spade.red.effectv2group.json:1) | 2 | 2 | boss.kouku.card.white_1; boss.kouku.card.spade.red_1 | 없음 |
| [boss.kouku.dance](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.dance.effectv2group.json:1) | 8 | 8 | boss.kouku.dance.radial.blue_1; boss.kouku.dance.radial.red_1; boss.kouku.dance.radial.green_1; boss.kouku.dance.radial.orange_1; boss.kouku.dance.spade_1; boss.kouku.dance.heart_1; boss.kouku.dance.clover_1; boss.kouku.dance.dia_1 | `KAKULSAYDON_G1_PATTERN_6` |
| [boss.kouku.dance.clap](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.dance.clap.effectv2group.json:1) | 2 | 2 | boss.kouku.dance.clap.smoke.l_1; boss.kouku.dance.clap.star_1 | `KAKULSAYDON_G1_PATTERN_6` |
| [boss.kouku.disarm](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.disarm.effectv2group.json:1) | 21 | 8 | boss.kouku.disarm.shield_1 ×2; boss.kouku.disarm.converge_1; boss.kouku.blur_1; boss.kouku.disarm.converge_2; boss.kouku.disarm.converge_3; boss.kouku.disarm.star_1 ×5; boss.kouku.disarm.star.smoke_1 ×5; boss.kouku.disarm.star.decal_1 ×5 | 없음 |
| [boss.kouku.fear.face](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.fear.face.effectv2group.json:1) | 1 | 1 | boss.kouku.fear.face_1 | 없음 |
| [boss.kouku.find.core](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.find.core.effectv2group.json:1) | 4 | 4 | boss.kouku.find.core.small_1; boss.kouku.find.core.small_2; boss.kouku.find.core.big_1; boss.kouku.find.core.big_2 | `KAKULSAYDON_G1_PATTERN_2`, `KAKULSAYDON_G1_PATTERN_5` |
| [boss.kouku.find.heart](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.find.heart.effectv2group.json:1) | 2 | 2 | boss.kouku.find.heart.big_1; boss.kouku.find.heart.small_1 | `KAKULSAYDON_G1_PATTERN_2` |
| [boss.kouku.find.star](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.find.star.effectv2group.json:1) | 6 | 4 | boss.kouku.find.star.red_1; boss.kouku.find.star.black_1; boss.kouku.find.star.radial_1 ×2; boss.kouku.find.star.bang_1 ×2 | `KAKULSAYDON_G1_PATTERN_5` |
| [boss.kouku.joker.hammer](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.joker.hammer.effectv2group.json:1) | 12 | 5 | boss.kouku.joker.hammer.decal_1; boss.kouku.joker.hammer.decal_2; boss.kouku.joker.hammer.burst_1 ×8; boss.kouku.joker.hammer.decal_3; boss.kouku.blur_2 | `KAKULSAYDON_G1_PATTERN_13`, `KAKULSAYDON_G1_PATTERN_17` |
| [boss.kouku.medusa.blue](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.medusa.blue.effectv2group.json:1) | 2 | 2 | boss.kouku.medusa.blue_1; boss.kouku.medusa.ball.blue_1 | `KAKULSAYDON_G1_PATTERN_11` |
| [boss.kouku.medusa.laser](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.medusa.laser.effectv2group.json:1) | 30 | 1 | boss.kouku.medusa.laser_1 ×30 | `KAKULSAYDON_G1_PATTERN_11` |
| [boss.kouku.medusa.red](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.medusa.red.effectv2group.json:1) | 2 | 2 | boss.kouku.medusa.red_1; boss.kouku.medusa.ball.red_1 | `KAKULSAYDON_G1_PATTERN_11` |
| [boss.kouku.pizza.aura](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.pizza.aura.effectv2group.json:1) | 1 | 1 | boss.kouku.pizza.aura_1 | `KAKULSAYDON_G1_PATTERN_25` |
| [boss.kouku.pizza.e](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.pizza.e.effectv2group.json:1) | 6 | 6 | boss.kouku.pizza.decal.n_1; boss.kouku.pizza.decal.s_1; boss.kouku.pizza.decal.w_1; boss.kouku.pizza.decal.e_2; boss.kouku.blur_3; boss.kouku.radial_1 | `KAKULSAYDON_G1_PATTERN_25` |
| [boss.kouku.pizza.n](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.pizza.n.effectv2group.json:1) | 6 | 6 | boss.kouku.pizza.decal.w_1; boss.kouku.pizza.decal.e_1; boss.kouku.pizza.decal.s_1; boss.kouku.pizza.decal.n_2; boss.kouku.blur_3; boss.kouku.radial_1 | `KAKULSAYDON_G1_PATTERN_25` |
| [boss.kouku.pizza.s](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.pizza.s.effectv2group.json:1) | 6 | 6 | boss.kouku.pizza.decal.e_1; boss.kouku.pizza.decal.w_1; boss.kouku.pizza.decal.n_1; boss.kouku.pizza.decal.s_2; boss.kouku.blur_3; boss.kouku.radial_1 | `KAKULSAYDON_G1_PATTERN_25` |
| [boss.kouku.pizza.star.ccl](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.pizza.star.ccl.effectv2group.json:1) | 2 | 2 | boss.kouku.pizza.star.front.ccl_1; boss.kouku.pizza.star.back.ccl_1 | `KAKULSAYDON_G1_PATTERN_25` |
| [boss.kouku.pizza.star.cl](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.pizza.star.cl.effectv2group.json:1) | 2 | 2 | boss.kouku.pizza.star.front.cl_1; boss.kouku.pizza.star.back.cl_1 | `KAKULSAYDON_G1_PATTERN_25` |
| [boss.kouku.pizza.w](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/boss.kouku.pizza.w.effectv2group.json:1) | 6 | 6 | boss.kouku.pizza.decal.e_1; boss.kouku.pizza.decal.n_1; boss.kouku.pizza.decal.s_1; boss.kouku.pizza.decal.w_2; boss.kouku.blur_3; boss.kouku.radial_1 | `KAKULSAYDON_G1_PATTERN_25` |
| [cardmaze.exit.club](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.exit.club.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.club | 없음 |
| [cardmaze.exit.diamond](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.exit.diamond.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.diamond | 없음 |
| [cardmaze.exit.heart](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.exit.heart.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.heart | 없음 |
| [cardmaze.exit.spade](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.exit.spade.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.spade | 없음 |
| [cardmaze.mark.club](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.mark.club.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.club | 없음 |
| [cardmaze.mark.diamond](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.mark.diamond.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.diamond | 없음 |
| [cardmaze.mark.heart](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.mark.heart.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.heart | 없음 |
| [cardmaze.mark.spade](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Groups/cardmaze.mark.spade.effectv2group.json:1) | 1 | 1 | cardmaze.symbol.spade | 없음 |

| Leaf·파일 | 형태 | lifetime s / loop | mesh asset | base texture | 현재 직접/group 소비 |
| --- | --- | --- | --- | --- | --- |
| [bingo.bomb.mark_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/bingo.bomb.mark_1.effectv2.json:1) | Mesh | 0.0/True | Character/KoukuSaton/MN_RHCN_01/MN_RHCN_01.wmodel | Character/KoukuSaton/MN_RHCN_01/textures/mn_rhcn_01_d_loc_int.dds | `bingo.bomb.mark` |
| [bingo.bomb_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/bingo.bomb_1.effectv2.json:1) | Mesh | 0.0/True | Character/KoukuSaton/MN_RHCN_01/MN_RHCN_01.wmodel | Character/KoukuSaton/MN_RHCN_01/textures/mn_rhcn_01_d_loc_int.dds | `bingo.bomb` |
| [bingo.hammer_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/bingo.hammer_1.effectv2.json:1) | Mesh | 0.0/True | Character/KoukuSaton/MN_UMAC_01/MN_UMAC_01.wmodel | Character/KoukuSaton/MN_UMAC_01/textures/mn_umac_01_d.dds | `bingo.hammer` |
| [bingo.skull.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/bingo.skull.red_1.effectv2.json:1) | Decal | 0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_d_symbol_105_loc_int.dds | `bingo.skull.red` |
| [bingo.skull.white_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/bingo.skull.white_1.effectv2.json:1) | Decal | 0/True | - | Effect/KoukuSaydon/Textures/Bingo/bingo_skull_white.dds | `bingo.skull.white` |
| [boss.kouku.appear.card_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.appear.card_1.effectv2.json:1) | Particle | 2.0/False | Effect/KoukuSaydon/Meshes/MN_RPCT_05/mesh/mn_rpct_05_01.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_blankwhite_01.dds | `boss.kouku.appear` |
| [boss.kouku.appear.smoke_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.appear.smoke_1.effectv2.json:1) | Particle | 2.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_cloud_020_cl.dds | `boss.kouku.appear` |
| [boss.kouku.ball.smoke.blue_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.ball.smoke.blue_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_cloud_020_cl.dds | `boss.kouku.ball.smoke` |
| [boss.kouku.ball.smoke.green_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.ball.smoke.green_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_cloud_020_cl.dds | `boss.kouku.ball.smoke` |
| [boss.kouku.ball.smoke.orange_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.ball.smoke.orange_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_cloud_020_cl.dds | `boss.kouku.ball.smoke` |
| [boss.kouku.ball.smoke.purple_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.ball.smoke.purple_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_cloud_020_cl.dds | `boss.kouku.ball.smoke` |
| [boss.kouku.ball.smoke.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.ball.smoke.red_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_cloud_020_cl.dds | `boss.kouku.ball.smoke` |
| [boss.kouku.ball.smoke.spread_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.ball.smoke.spread_1.effectv2.json:1) | Particle | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_fragment_007.dds | `boss.kouku.ball.smoke` |
| [boss.kouku.ball.smoke.yellow_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.ball.smoke.yellow_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_cloud_020_cl.dds | `boss.kouku.ball.smoke` |
| [boss.kouku.blur_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.blur_1.effectv2.json:1) | ScreenPost | 7.0/False | - | - | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.blur_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.blur_2.effectv2.json:1) | ScreenPost | 0.3/False | - | - | `boss.kouku.joker.hammer` |
| [boss.kouku.blur_3](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.blur_3.effectv2.json:1) | ScreenPost | 0.5/False | - | - | `boss.kouku.pizza.e`, `boss.kouku.pizza.n`, `boss.kouku.pizza.s`, `boss.kouku.pizza.w` |
| [boss.kouku.card.clober.black_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.clober.black_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090.dds | `boss.kouku.card.clober.black` |
| [boss.kouku.card.clober.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.clober.red_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090_1.dds | `boss.kouku.card.clober.red` |
| [boss.kouku.card.dia.black_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.dia.black_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090_1.dds | `boss.kouku.card.dia.black` |
| [boss.kouku.card.dia.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.dia.red_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090.dds | `boss.kouku.card.dia.red` |
| [boss.kouku.card.heart.black_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.heart.black_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090_1.dds | `boss.kouku.card.heart.black` |
| [boss.kouku.card.heart.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.heart.red_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090.dds | `boss.kouku.card.heart.red` |
| [boss.kouku.card.spade.black_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.spade.black_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090.dds | `boss.kouku.card.spade.black` |
| [boss.kouku.card.spade.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.spade.red_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_symbol_090_1.dds | `boss.kouku.card.spade.red` |
| [boss.kouku.card.white_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.card.white_1.effectv2.json:1) | Texture | 0.0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_m_cardalpha_01.dds | `boss.kouku.card.clober.black`, `boss.kouku.card.clober.red`, `boss.kouku.card.dia.black`, `boss.kouku.card.dia.red`, `boss.kouku.card.heart.black`, `boss.kouku.card.heart.red`, `boss.kouku.card.spade.black`, `boss.kouku.card.spade.red` |
| [boss.kouku.curtain_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.curtain_1.effectv2.json:1) | ScreenPost | 3.5/False | - | Effect/KoukuSaydon/Screen/fx_d_symbol_100_ycl.dds | `KAKULSAYDON_G1_PATTERN_6` |
| [boss.kouku.dance.clap.smoke.l_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.clap.smoke.l_1.effectv2.json:1) | Particle | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_fluid_013.dds | `boss.kouku.dance.clap` |
| [boss.kouku.dance.clap.star_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.clap.star_1.effectv2.json:1) | Particle | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_06/fx_x_symbol_018_1_cl.dds | `boss.kouku.dance.clap` |
| [boss.kouku.dance.clover_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.clover_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_2.dds | `boss.kouku.dance` |
| [boss.kouku.dance.dia_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.dia_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_3.dds | `boss.kouku.dance` |
| [boss.kouku.dance.heart_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.heart_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47.dds | `boss.kouku.dance` |
| [boss.kouku.dance.light.glow_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.light.glow_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_01/fx_c_glow_003.dds | 없음 |
| [boss.kouku.dance.light_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.light_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_glow_001.dds | 없음 |
| [boss.kouku.dance.radial.blue_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.radial.blue_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_glow_008.dds | `boss.kouku.dance` |
| [boss.kouku.dance.radial.green_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.radial.green_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_glow_008.dds | `boss.kouku.dance` |
| [boss.kouku.dance.radial.orange_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.radial.orange_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_glow_008.dds | `boss.kouku.dance` |
| [boss.kouku.dance.radial.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.radial.red_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_glow_008.dds | `boss.kouku.dance` |
| [boss.kouku.dance.spade_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.dance.spade_1.effectv2.json:1) | Decal | 40.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_1.dds | `boss.kouku.dance` |
| [boss.kouku.disarm.converge_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.disarm.converge_1.effectv2.json:1) | Mesh | 7.0/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_a_cylinder_002.wmodel | Effect/Esther/Thirain/Textures/FX_TEX_04/fx_i_shockwave_02_ycl.dds | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.disarm.converge_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.disarm.converge_2.effectv2.json:1) | Mesh | 7.0/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_a_cylinder_002.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_atypical_002_cl.dds | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.disarm.converge_3](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.disarm.converge_3.effectv2.json:1) | Mesh | 7.0/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_a_cylinder_002.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_atypical_002_cl.dds | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.disarm.shield_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.disarm.shield_1.effectv2.json:1) | Mesh | 10.0/False | Effect/Warlord/Meshes/FX_SM_03/fx_w_wgl_gdd_b_01.wmodel | Effect/Warlord/Textures/FX_TEX_03/fx_e_symbol_057.dds | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.disarm.star.decal_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.disarm.star.decal_1.effectv2.json:1) | Decal | 9.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_decal_020.dds | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.disarm.star.smoke_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.disarm.star.smoke_1.effectv2.json:1) | Particle | 0.7/False | - | - | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.disarm.star_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.disarm.star_1.effectv2.json:1) | Mesh | 1.0/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_d_plane_003.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_04/fx_i_atypical_01_ycl.dds | `KAKULSAYDON_G1_PATTERN_1`, `boss.kouku.disarm` |
| [boss.kouku.fear.face_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.fear.face_1.effectv2.json:1) | ScreenPost | 2.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_06/fx_g_rpcz_01.dds | `boss.kouku.fear.face` |
| [boss.kouku.find.core.big_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.core.big_1.effectv2.json:1) | Mesh | 20.0/False | Effect/KoukuSaydon/Meshes/fx_sm_01/fm_m_sphere_004.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_006.dds | `boss.kouku.find.core` |
| [boss.kouku.find.core.big_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.core.big_2.effectv2.json:1) | Mesh | 20.0/False | Effect/KoukuSaydon/Meshes/fx_sm_01/fm_m_sphere_004.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_006.dds | `boss.kouku.find.core` |
| [boss.kouku.find.core.small_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.core.small_1.effectv2.json:1) | Mesh | 20.0/False | Effect/KoukuSaydon/Meshes/fx_sm_01/fm_m_sphere_004.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_04/fx_j_risingforce_01.dds | `boss.kouku.find.core` |
| [boss.kouku.find.core.small_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.core.small_2.effectv2.json:1) | Mesh | 20.0/False | Effect/KoukuSaydon/Meshes/fx_sm_01/fm_m_sphere_004.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_04/fx_j_risingforce_01.dds | `boss.kouku.find.core` |
| [boss.kouku.find.heart.big_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.heart.big_1.effectv2.json:1) | Texture | 1.5/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_l_symbol_12_cl.dds | `boss.kouku.find.heart` |
| [boss.kouku.find.heart.small_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.heart.small_1.effectv2.json:1) | Particle | 1.5/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_l_symbol_12_cl.dds | `boss.kouku.find.heart` |
| [boss.kouku.find.star.bang_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.star.bang_1.effectv2.json:1) | Particle | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_fluid_013.dds | `boss.kouku.find.star` |
| [boss.kouku.find.star.black_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.star.black_1.effectv2.json:1) | Particle | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_06/fx_x_symbol_018_1_cl.dds | `boss.kouku.find.star` |
| [boss.kouku.find.star.radial_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.star.radial_1.effectv2.json:1) | Texture | 0.2/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_b_ring_004.dds | `boss.kouku.find.star` |
| [boss.kouku.find.star.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.find.star.red_1.effectv2.json:1) | Particle | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_06/fx_x_symbol_018_1_cl.dds | `boss.kouku.find.star` |
| [boss.kouku.joker.hammer.burst_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.joker.hammer.burst_1.effectv2.json:1) | Particle | 0.6/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_fluid_026.dds | `boss.kouku.joker.hammer` |
| [boss.kouku.joker.hammer.decal_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.joker.hammer.decal_1.effectv2.json:1) | Decal | 0.3/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_hit_002.dds | `boss.kouku.joker.hammer` |
| [boss.kouku.joker.hammer.decal_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.joker.hammer.decal_2.effectv2.json:1) | Decal | 0.3/False | - | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_b_ring_001.dds | `boss.kouku.joker.hammer` |
| [boss.kouku.joker.hammer.decal_3](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.joker.hammer.decal_3.effectv2.json:1) | Decal | 2.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_06/fx_x_decal_013_7_cl.dds | `boss.kouku.joker.hammer` |
| [boss.kouku.medusa.ball.blue_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.medusa.ball.blue_1.effectv2.json:1) | Mesh | 10.0/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_d_hemisphere_001.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_atypical_027.dds | `boss.kouku.medusa.blue` |
| [boss.kouku.medusa.ball.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.medusa.ball.red_1.effectv2.json:1) | Mesh | 10.0/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_d_hemisphere_001.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_atypical_027.dds | `boss.kouku.medusa.red` |
| [boss.kouku.medusa.blue_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.medusa.blue_1.effectv2.json:1) | Decal | 10.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_m_magicsymbol_001.dds | `boss.kouku.medusa.blue` |
| [boss.kouku.medusa.laser_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.medusa.laser_1.effectv2.json:1) | Mesh | 0.2/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_c_square_001.wmodel | Effect/Artist/Textures/fx_a_blankwhite_01.dds | `boss.kouku.medusa.laser` |
| [boss.kouku.medusa.red_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.medusa.red_1.effectv2.json:1) | Decal | 10.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_m_magicsymbol_001.dds | `boss.kouku.medusa.red` |
| [boss.kouku.pizza.aura_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.aura_1.effectv2.json:1) | Mesh | 3.0/False | Effect/KoukuSaydon/Meshes/fx_sm_00/fm_b_sphere_001.wmodel | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_cloud_027.dds | `boss.kouku.pizza.aura` |
| [boss.kouku.pizza.decal.e_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.e_1.effectv2.json:1) | Decal | 0.5/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_hit_010.dds | `boss.kouku.pizza.n`, `boss.kouku.pizza.s`, `boss.kouku.pizza.w` |
| [boss.kouku.pizza.decal.e_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.e_2.effectv2.json:1) | Decal | 0.3/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_008.dds | `boss.kouku.pizza.e` |
| [boss.kouku.pizza.decal.n_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.n_1.effectv2.json:1) | Decal | 0.5/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_hit_010.dds | `boss.kouku.pizza.e`, `boss.kouku.pizza.s`, `boss.kouku.pizza.w` |
| [boss.kouku.pizza.decal.n_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.n_2.effectv2.json:1) | Decal | 0.3/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_008.dds | `boss.kouku.pizza.n` |
| [boss.kouku.pizza.decal.s_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.s_1.effectv2.json:1) | Decal | 0.5/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_hit_010.dds | `boss.kouku.pizza.e`, `boss.kouku.pizza.n`, `boss.kouku.pizza.w` |
| [boss.kouku.pizza.decal.s_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.s_2.effectv2.json:1) | Decal | 0.3/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_008.dds | `boss.kouku.pizza.s` |
| [boss.kouku.pizza.decal.w_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.w_1.effectv2.json:1) | Decal | 0.5/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_hit_010.dds | `boss.kouku.pizza.e`, `boss.kouku.pizza.n`, `boss.kouku.pizza.s` |
| [boss.kouku.pizza.decal.w_2](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.decal.w_2.effectv2.json:1) | Decal | 0.3/False | - | Effect/KoukuSaydon/Textures/FX_TEX_02/fx_d_atypical_008.dds | `boss.kouku.pizza.w` |
| [boss.kouku.pizza.star.back.ccl_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.star.back.ccl_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_x_symbol_002_2_cl.dds | `boss.kouku.pizza.star.ccl` |
| [boss.kouku.pizza.star.back.cl_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.star.back.cl_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_x_symbol_002_2_cl.dds | `boss.kouku.pizza.star.cl` |
| [boss.kouku.pizza.star.front.ccl_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.star.front.ccl_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_x_symbol_002_2_cl.dds | `boss.kouku.pizza.star.ccl` |
| [boss.kouku.pizza.star.front.cl_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.pizza.star.front.cl_1.effectv2.json:1) | Texture | 1.0/False | - | Effect/KoukuSaydon/Textures/FX_TEX_05/fx_x_symbol_002_2_cl.dds | `boss.kouku.pizza.star.cl` |
| [boss.kouku.radial_1](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/boss.kouku.radial_1.effectv2.json:1) | ScreenPost | 0.5/False | - | - | `boss.kouku.pizza.e`, `boss.kouku.pizza.n`, `boss.kouku.pizza.s`, `boss.kouku.pizza.w` |
| [cardmaze.symbol.club](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/cardmaze.symbol.club.effectv2.json:1) | Decal | 0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_2.dds | `cardmaze.exit.club`, `cardmaze.mark.club` |
| [cardmaze.symbol.diamond](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/cardmaze.symbol.diamond.effectv2.json:1) | Decal | 0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_3.dds | `cardmaze.exit.diamond`, `cardmaze.mark.diamond` |
| [cardmaze.symbol.heart](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/cardmaze.symbol.heart.effectv2.json:1) | Decal | 0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47.dds | `cardmaze.exit.heart`, `cardmaze.mark.heart` |
| [cardmaze.symbol.spade](C:/Users/user/Desktop/LostArk/Data/Effects/V2/Authored/cardmaze.symbol.spade.effectv2.json:1) | Decal | 0/True | - | Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/fx_l_symbol_47_1.dds | `cardmaze.exit.spade`, `cardmaze.mark.spade` |

## G08. 이번 조사에서 수행한 검증과 미수행

실제 수행: LAN 스크립트 실행(server-host, firewall ready, endpoint not-listening), git status/현재 branch/git fetch 확인, 관련 코드·정본·게시 JSON 파싱과 row 집계·ID 대조, 이 보고서의 생성 및 diff whitespace 확인. 현재 브랜치는 `codex/kouku-full-material-lighting-restoration`이다.

미수행: Client/UI 실행, 화면 캡처, visual PASS 판정, 제품 컴파일·publisher 재실행, 자동 레이드 진행/원본과 픽셀 일치 검증. 코드와 데이터 변경을 하지 않았으므로 compile 검증을 새로 요구하거나 과거 결과를 오늘 PASS로 재기록하지 않았다.
