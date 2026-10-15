# 쿠크 전체 패턴 데이터 지도

읽는 순서: [00 처음부터 읽기](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_FULL_PATTERN_WALKTHROUGH_LEARNING_RESULT.md) → [01 Python 파싱·투영](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_JSON_PIPELINE_LEARNING_RESULT.md) → [02 Server 구조체·판정](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_SERVER_RUNTIME_LEARNING_RESULT.md) → [03 Client 저작·애니메이션](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_CLIENT_ANIMATION_LEARNING_RESULT.md) → [04 Collider·좌표·피해](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_COLLIDER_RUNTIME_LEARNING_RESULT.md) → [05 전체 패턴 지도](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_PATTERN_ATLAS_LEARNING_RESULT.md)

2026-10-14 현재 디스크 저장본을 자동 추출했다. 실행·화면 검증이나 사용자의 구현 완료를 의미하지 않는다.
Composition revision `2500`: **121개 저장**, Encounter sourceRevision `2500`: **114개 투영**.
저장본 DRAFT는 게시 불가의 뜻이 아니다. publisher는 비공개 후보의 status를 PRODUCT로 바꾸고 의존 패턴 묶음을 검증한다.
아래 순서는 저장된 patterns 배열 순서다. 실제 레이드 순서는 마지막 Flow/raidGates 표를 별도로 본다.

## 표 읽는 법

- P13은 `KOUKUSAYDON_G1_PATTERN_13`, L22는 `koukusaydon.g1.logic.22`의 표시 약칭이다. 원래 ID는 JSON에 보존한다.
- stage 순서는 배열 순서이며 STAGE 숫자순이 아니다. 구간은 누적 stage 시작·끝 ms이고, clip의 시작 offset은 해당 stage 기준이다.
- source는 `sourceActionId/sourceStageId/sourceSlotId`; 소스 시작·재생 길이·배속·종료 정책은 별도 값이다.
- Logic 표는 enabled=false도 보존한다. 결과 S/F/T는 성공/실패/시간초과이며 `L번호:결과종류`로 표시한다.
- Encounter 표본 track의 모든 key를 재인쇄하지 않는다. inventory JSON에는 count/first/last, 원본에는 전체 샘플이 있다.
- 상세 파라미터·전체 presentation/world/summon/scene occurrence는 [pattern_inventory.json](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/pattern_inventory.json)의 동일 patternId에서 확인한다.

## 전체 색인

| 패턴 | Gate | 이름 | owner placement | stage | Logic | 투영 |
|---|---|---|---|---:|---:|---|
| [P1](#p1) | GATE1 | 세이튼_무력화 시작 | boss.koukusaydon.g1.saydon | 6 | 1 | 예 |
| [P2](#p2) | GATE1 | 세이튼_진짜세이튼찾기 | boss.koukusaydon.g1.saydon | 13 | 2 | 예 |
| [P3](#p3) | GATE1 | 세이튼_무력화실패 | boss.koukusaydon.g1.saydon | 0 | 0 | 아니오 |
| [P4](#p4) | GATE1 | 세이튼_무력화성공 | boss.koukusaydon.g1.saydon | 3 | 0 | 예 |
| [P5](#p5) | GATE1 | 세이튼_가짜세이튼 | boss.koukusaydon.g1.saydon | 13 | 0 | 예 |
| [P6](#p6) | GATE1 | 세이튼_댄스타임 | boss.koukusaydon.g1.saydon | 21 | 4 | 예 |
| [P7](#p7) | GATE1 | 세이튼_룰렛 | boss.koukusaydon.g1.saydon | 9 | 5 | 예 |
| [P8](#p8) | GATE2 | 쿠크_세이튼등장 | boss.koukusaydon.g2.kouku | 1 | 11 | 예 |
| [P9](#p9) | GATE2 | 대형세이튼_세이튼등장 | boss.koukusaydon.g2.big-saydon | 3 | 0 | 예 |
| [P10](#p10) | GATE2 | 쿠크_파1빨2 | boss.koukusaydon.g2.kouku | 5 | 0 | 예 |
| [P11](#p11) | GATE2 | 대형세이튼_파1빨2 | boss.koukusaydon.g2.big-saydon | 6 | 10 | 예 |
| [P12](#p12) | GATE2 | 쿠크_조커찾기 | boss.koukusaydon.g2.kouku | 2 | 0 | 예 |
| [P13](#p13) | GATE2 | 대형세이튼_조커찾기 | boss.koukusaydon.g2.big-saydon | 21 | 17 | 예 |
| [P14](#p14) | GATE2 | 대형세이튼_조커찾기_성공 | boss.koukusaydon.g2.big-saydon | 1 | 0 | 예 |
| [P15](#p15) | GATE2 | 쿠크_거미카운터 | boss.koukusaydon.g2.kouku | 15 | 9 | 예 |
| [P17](#p17) | GATE2 | 대형세이튼_잡기 | boss.koukusaydon.g2.big-saydon | 3 | 11 | 예 |
| [P25](#p25) | GATE2 | 쿠크_피자 | boss.koukusaydon.g2.kouku | 15 | 14 | 예 |
| [P26](#p26) | GATE2 | 대형세이튼_피자 | boss.koukusaydon.g2.big-saydon | 16 | 1 | 예 |
| [P20](#p20) | GATE2 | 대형세이튼_레이저 | boss.koukusaydon.g2.big-saydon | 0 | 0 | 아니오 |
| [P21](#p21) | GATE2 | 쿠크_레이저 | boss.koukusaydon.g2.kouku | 3 | 8 | 예 |
| [P22](#p22) | GATE2 | 쿠크_그로기 | boss.koukusaydon.g2.kouku | 3 | 0 | 예 |
| [P23](#p23) | GATE2 | 쿠크_팡파레 | boss.koukusaydon.g2.kouku | 6 | 12 | 예 |
| [P24](#p24) | GATE2 | 쿠크_휠윈드 | boss.koukusaydon.g2.kouku | 23 | 17 | 예 |
| [P18](#p18) | GATE3 | 3관문_외곽불회전_갈고리대각선_시각테스트 | boss.koukusaydon.g3.saydon | 12 | 0 | 예 |
| [P19](#p19) | GATE3 | 3관문_갈고리만_확인용_불없음 | boss.koukusaydon.g3.saydon | 12 | 0 | 예 |
| [P27](#p27) | GATE2 | 대형세이튼_불뿜기 | boss.koukusaydon.g2.big-saydon | 1 | 2 | 예 |
| [P28](#p28) | GATE2 | 쿠크_카드미로연출 | boss.koukusaydon.g2.kouku | 3 | 5 | 예 |
| [P31](#p31) | GATE3 | 3관문 칼날 바닥  | boss.koukusaydon.g3.saydon | 2 | 0 | 예 |
| [P33](#p33) | GATE3 | 세이튼_마리오_2페이즈 | boss.koukusaydon.g3.saydon | 15 | 4 | 예 |
| [P34](#p34) | GATE3 | 세이튼_마리오_1페이즈 | boss.koukusaydon.g3.saydon | 1 | 4 | 예 |
| [P35](#p35) | GATE3 | 세이튼_쇼타임 | boss.koukusaydon.g3.saydon | 21 | 118 | 예 |
| [P36](#p36) | GATE1 | 세이튼_1관문연출 | boss.koukusaydon.g1.saydon | 13 | 5 | 예 |
| [P38](#p38) | GATE3 | 쿠크세이튼_무지개댄스 | boss.koukusaydon.g3.saydon | 15 | 21 | 예 |
| [P39](#p39) | GATE3 | 쿠크세이튼_알비온_감전장판 | boss.koukusaydon.g3.saydon | 6 | 19 | 예 |
| [P40](#p40) | GATE3 | 쿠크세이튼_십자화염폭발 | boss.koukusaydon.g3.saydon | 2 | 1 | 예 |
| [P41](#p41) | GATE3 | 쿠크세이튼_갈고리_칼날댄스 | boss.koukusaydon.g3.saydon | 12 | 0 | 예 |
| [P42](#p42) | GATE3 | 쿠크세이튼_무력화성공  | boss.koukusaydon.g3.saydon | 3 | 0 | 예 |
| [P43](#p43) | GATE3 | 쿠크세이튼_백스텝불뿜기_화염링 | boss.koukusaydon.g3.saydon | 3 | 8 | 예 |
| [P44](#p44) | GATE3 | 쿠크세이튼_분신소환_기분나빠 | boss.koukusaydon.g3.saydon | 4 | 0 | 예 |
| [P45](#p45) | GATE3 | 쿠크세이튼_분신소환_십자화염폭발 | boss.koukusaydon.g3.saydon | 0 | 0 | 아니오 |
| [P46](#p46) | GATE3 | 쿠크세이튼_벡스텝후감전빔 | boss.koukusaydon.g3.saydon | 2 | 1 | 예 |
| [P47](#p47) | GATE1 | 세이튼_트럼펫장판소환 | boss.koukusaydon.g1.saydon | 9 | 2 | 예 |
| [P48](#p48) | GATE1 | 세이튼_빙글빙글돌며카드던지기 | boss.koukusaydon.g1.saydon | 8 | 7 | 예 |
| [P49](#p49) | GATE3 | 쿠크세이튼_우측이동화염파동 | boss.koukusaydon.g3.saydon | 3 | 14 | 예 |
| [P50](#p50) | GATE3 | 쿠크세이튼_분신소환_기분나빠_전방 | boss.koukusaydon.g3.saydon | 7 | 2 | 예 |
| [P51](#p51) | GATE3 | 쿠크세이튼_분신소환_기분나빠_전방 | boss.koukusaydon.g3.saydon | 0 | 0 | 아니오 |
| [P52](#p52) | GATE3 | 쿠크세이튼_분신소환_기분나빠 | boss.koukusaydon.g3.saydon | 0 | 1 | 예 |
| [P53](#p53) | GATE3 | 쿠크세이튼_분신소환_기분나빠_후방 | boss.koukusaydon.g3.saydon | 7 | 1 | 예 |
| [P54](#p54) | GATE3 | 쿠크세이튼_분신소환_기분나빠_왼쪽 | boss.koukusaydon.g3.saydon | 7 | 1 | 예 |
| [P55](#p55) | GATE3 | 쿠크세이튼_분신소환_기분나빠_오른쪽 | boss.koukusaydon.g3.saydon | 7 | 1 | 예 |
| [P56](#p56) | GATE3 | 세이튼_마리오_1페이즈 | boss.koukusaydon.g3.saydon | 0 | 0 | 예 |
| [P57](#p57) | GATE3 | 세이튼_마리오_2페이즈 | boss.koukusaydon.g3.saydon | 0 | 0 | 예 |
| [P58](#p58) | GATE1 | 세이튼_화염 파동 | boss.koukusaydon.g1.saydon | 3 | 38 | 예 |
| [P59](#p59) | GATE3 | 쿠크세이튼_화염 파동 | boss.koukusaydon.g3.saydon | 3 | 28 | 예 |
| [P60](#p60) | GATE3 | 기분나빠 \| 브레스 | boss.koukusaydon.g3.saydon | 1 | 1 | 예 |
| [P61](#p61) | GATE3 | 저주의식 \| 왼손 트레일 | boss.koukusaydon.g3.saydon | 1 | 0 | 예 |
| [P62](#p62) | GATE3 | 세이튼_눈 레이저 | boss.koukusaydon.g3.saydon | 2 | 2 | 예 |
| [P63](#p63) | GATE3 | 쿠크세이튼_분신소환_십자화염폭발_왼쪽 | boss.koukusaydon.g3.saydon | 2 | 1 | 예 |
| [P64](#p64) | GATE3 | 쿠크세이튼_분신소환_십자화염폭발_오른쪽 | boss.koukusaydon.g3.saydon | 2 | 0 | 예 |
| [P65](#p65) | GATE3 | 쿠크세이튼_분신소환_십자화염폭발 | boss.koukusaydon.g3.saydon | 0 | 0 | 아니오 |
| [P66](#p66) | GATE3 | 쿠크세이튼_분신소환_3갈레불뿜기 | boss.koukusaydon.g3.saydon | 0 | 0 | 예 |
| [P67](#p67) | GATE3 | 쿠크세이튼_분신소환_3갈레불뿜기_전방 | boss.koukusaydon.g3.saydon | 3 | 1 | 예 |
| [P68](#p68) | GATE3 | 쿠크세이튼_분신소환_3갈레불뿜기_후방  | boss.koukusaydon.g3.saydon | 3 | 1 | 예 |
| [P69](#p69) | GATE3 | 쿠크세이튼_분신소환_3갈레불뿜기_왼쪽 | boss.koukusaydon.g3.saydon | 3 | 1 | 예 |
| [P70](#p70) | GATE3 | 쿠크세이튼_분신소환_3갈레불뿜기_오른쪽 | boss.koukusaydon.g3.saydon | 3 | 1 | 예 |
| [P71](#p71) | GATE3 | 쿠크세이튼_분신소환_십자화염폭발_전방 | boss.koukusaydon.g3.saydon | 2 | 0 | 예 |
| [P72](#p72) | GATE3 | 쿠크세이튼_분신소환_십자화염폭발_후방 | boss.koukusaydon.g3.saydon | 2 | 0 | 예 |
| [P73](#p73) | GATE2 | 2관문_진입컷씬 | boss.koukusaydon.g2.kouku | 1 | 0 | 예 |
| [P74](#p74) | GATE2 | 2관문클리어_3관문진입 | boss.koukusaydon.g2.kouku | 1 | 0 | 예 |
| [P75](#p75) | BINGO | 빙고_최종엔딩씬 | boss.koukusaydon.bingo.saydon | 1 | 0 | 예 |
| [P76](#p76) | GATE3 | 쇼타임_연출 | boss.koukusaydon.g3.saydon | 2 | 0 | 예 |
| [P77](#p77) | GATE2 | 카드미로연출 | boss.koukusaydon.g2.kouku | 1 | 6 | 예 |
| [P78](#p78) | GATE1 | 세이튼_카드맞추기_주사위패턴 | boss.koukusaydon.g1.saydon | 11 | 6 | 예 |
| [P79](#p79) | GATE1 | 세이튼_공먹고휘리릭_노란장판펑펑 | boss.koukusaydon.g1.saydon | 4 | 6 | 예 |
| [P80](#p80) | GATE1 | 세이튼_돌진_카운터 | boss.koukusaydon.g1.saydon | 3 | 2 | 예 |
| [P81](#p81) | GATE1 | 세이튼_공굴리기_카운터 | boss.koukusaydon.g1.saydon | 18 | 4 | 예 |
| [P82](#p82) | GATE1 | 세이튼_마술쇼_종이비둘기 | boss.koukusaydon.g1.saydon | 4 | 10 | 예 |
| [P83](#p83) | GATE1 | 세이튼_쓰리투원투하! | boss.koukusaydon.g1.saydon | 5 | 4 | 예 |
| [P84](#p84) | GATE2 | 쿠크_훌라후프_레드 | boss.koukusaydon.g2.kouku | 9 | 2 | 예 |
| [P85](#p85) | GATE2 | 쿠크_슈퍼바주카 | boss.koukusaydon.g2.kouku | 4 | 3 | 예 |
| [P86](#p86) | GATE2 | 대형세이튼_두번내려치기 | boss.koukusaydon.g2.big-saydon | 3 | 2 | 예 |
| [P87](#p87) | GATE2 | 대형세이튼_3번내려치기 | boss.koukusaydon.g2.big-saydon | 1 | 3 | 예 |
| [P88](#p88) | GATE3 | 1마리오_1페이즈 | boss.koukusaydon.g3.saydon | 1 | 7 | 예 |
| [P89](#p89) | GATE3 | 쿠크세이튼_1마리오 | boss.koukusaydon.g3.saydon | 1 | 4 | 예 |
| [P90](#p90) | GATE2 | 쿠크_훌라후프_화이트 | boss.koukusaydon.g2.kouku | 9 | 2 | 예 |
| [P91](#p91) | GATE3 | 2마리오_1페이즈 | boss.koukusaydon.g3.saydon | 1 | 7 | 예 |
| [P92](#p92) | GATE3 | 3마리오_1페이즈 | boss.koukusaydon.g3.saydon | 1 | 7 | 예 |
| [P93](#p93) | GATE3 | 4마리오_1페이즈 | boss.koukusaydon.g3.saydon | 1 | 7 | 예 |
| [P94](#p94) | BINGO | 메두사공포 | boss.koukusaydon.bingo.saydon | 3 | 1 | 예 |
| [P95](#p95) | GATE3 | 즉사 칼날 \| Object 판정 테스트 | boss.koukusaydon.g3.saydon | 4 | 0 | 예 |
| [P97](#p97) | BINGO | 앵콜컷신 | boss.koukusaydon.bingo.saydon | 1 | 0 | 예 |
| [P99](#p99) | GATE2 | 대형세이튼_불어날리기 | boss.koukusaydon.g2.big-saydon | 3 | 1 | 예 |
| [P100](#p100) | GATE1 | 세이튼_정면바람방구 | boss.koukusaydon.g1.saydon | 2 | 1 | 예 |
| [P101](#p101) | GATE1 | 세이튼_플레이어추적 | boss.koukusaydon.g1.saydon | 2 | 1 | 예 |
| [P102](#p102) | GATE1 | 세이튼_백스텝후불뿜기화염링 | boss.koukusaydon.g1.saydon | 3 | 11 | 예 |
| [P103](#p103) | GATE1 | 세이튼_카드비 | boss.koukusaydon.g1.saydon | 5 | 2 | 예 |
| [P104](#p104) | GATE1 | 세이튼_1초추적 | boss.koukusaydon.g1.saydon | 1 | 1 | 예 |
| [P105](#p105) | GATE2 | 쿠크_나팔액션 | boss.koukusaydon.g2.kouku | 3 | 1 | 예 |
| [P106](#p106) | GATE2 | 쿠크_저글링액션 | boss.koukusaydon.g2.kouku | 2 | 6 | 예 |
| [P107](#p107) | BINGO | 빙고_이동_첫클립_메두사_블랙홀 | boss.koukusaydon.bingo.saydon | 0 | 2 | 예 |
| [P108](#p108) | BINGO | 앵콜세이튼_분신소환_기분나빠 | boss.koukusaydon.bingo.saydon | 7 | 1 | 예 |
| [P109](#p109) | BINGO | 앵콜세이튼_기본공격 | boss.koukusaydon.bingo.saydon | 1 | 1 | 예 |
| [P110](#p110) | GATE3 | 앵콜세이튼_백스텝후화염링 | boss.koukusaydon.g3.saydon | 0 | 0 | 아니오 |
| [P111](#p111) | GATE3 | 앵콜세이튼_십자화염폭발 | boss.koukusaydon.g3.saydon | 0 | 0 | 아니오 |
| [P112](#p112) | BINGO | 앵콜세이튼_십자화염폭발 | boss.koukusaydon.bingo.saydon | 2 | 1 | 예 |
| [P113](#p113) | BINGO | 앵콜세이튼_정면바람방구 | boss.koukusaydon.bingo.saydon | 2 | 2 | 예 |
| [P114](#p114) | BINGO | 앵콜세이튼_백스텝후화염링 | boss.koukusaydon.bingo.saydon | 3 | 8 | 예 |
| [P115](#p115) | BINGO | 앵콜세이튼_화염파동 | boss.koukusaydon.bingo.saydon | 3 | 28 | 예 |
| [P116](#p116) | GATE3 | 쿠크세이튼_정면바람방구 | boss.koukusaydon.g3.saydon | 2 | 2 | 예 |
| [P117](#p117) | GATE3 | 쿠크세이튼_돌진_카운터 | boss.koukusaydon.g3.saydon | 3 | 2 | 예 |
| [P118](#p118) | GATE3 | 쿠크세이튼_공굴리기_카운터 | boss.koukusaydon.g3.saydon | 18 | 20 | 예 |
| [P119](#p119) | GATE3 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | boss.koukusaydon.g3.saydon | 4 | 6 | 예 |
| [P120](#p120) | BINGO | 앵콜세이튼_알비온_감전장판 | boss.koukusaydon.bingo.saydon | 6 | 19 | 예 |
| [P122](#p122) | BINGO | 앵콜세이튼_공먹고휘리릭노란장판펑펑 | boss.koukusaydon.bingo.saydon | 4 | 6 | 예 |
| [P123](#p123) | BINGO | 앵콜세이튼_레이저 | boss.koukusaydon.bingo.saydon | 2 | 2 | 예 |
| [P124](#p124) | GATE3 | 쿠크세이튼_플레이어추적 | boss.koukusaydon.g3.saydon | 2 | 1 | 예 |
| [P125](#p125) | GATE3 | 쿠크세이튼_1초추적 | boss.koukusaydon.g3.saydon | 1 | 1 | 예 |
| [P126](#p126) | BINGO | 앵콜세이튼_플레이어추적 | boss.koukusaydon.bingo.saydon | 2 | 1 | 예 |
| [P127](#p127) | BINGO | 빙고_블랙홀_첫클립 | boss.koukusaydon.bingo.saydon | 1 | 0 | 예 |
| [P128](#p128) | BINGO | 빙고_블랙홀_13초_폭발 | boss.koukusaydon.bingo.saydon | 5 | 1 | 예 |
| [P129](#p129) | BINGO | 빙고_보드시작 | boss.koukusaydon.bingo.saydon | 1 | 1 | 예 |

<a id="p1"></a>
## P1 · 세이튼_무력화 시작

`KOUKUSAYDON_G1_PATTERN_1` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=15156 / stage 합=15135ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":6,"COLLIDER":2,"SOUND":11}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"STAGGER_WINDOW":1}`; mechanic trigger: `{}`.
서버 stage 6, rootMotion 보유 6; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_43 (ACTIVE) | 0–2267 | P1.stage.43 | rpct00_att_battle_6_04 / P1.animation.43 | 4219945/stage-004/animation-000 | 0/0/2267/1/EXACT |
| 2/STAGE_51 (ACTIVE) | 2267–4934 | P1.stage.51 | rpct00_att_battle_6_02 / P1.animation.51 | 4219945/stage-005/animation-000 | 0/0/2667/1/EXACT |
| 3/STAGE_53 (ACTIVE) | 4934–7601 | P1.stage.53 | rpct00_att_battle_6_02 / P1.animation.53 | 4219945/stage-005/animation-000 | 0/0/2667/1/EXACT |
| 4/STAGE_54 (ACTIVE) | 7601–10268 | P1.stage.54 | rpct00_att_battle_6_02 / P1.animation.54 | 4219945/stage-005/animation-000 | 0/0/2667/1/EXACT |
| 5/STAGE_55 (ACTIVE) | 10268–12935 | P1.stage.55 | rpct00_att_battle_6_02 / P1.animation.55 | 4219945/stage-005/animation-000 | 0/0/2667/1/EXACT |
| 6/STAGE_52 (ACTIVE) | 12935–15135 | P1.stage.52 | rpct00_att_battle_6_03 / P1.animation.52 | 4219945/stage-006/animation-000 | 0/0/2200/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P1.logic.1 | L1 세이튼로직_방패무력화 | DURATION/STAGGER_WINDOW | 2263+10684 / True | S: L7:FOLLOWUP_PATTERN; T: L2:INSTANT_DEATH |

의존 패턴: P4 ← onSuccess:koukusaydon.g1.logic.7 (P1.logic.1).

<a id="p2"></a>
## P2 · 세이튼_진짜세이튼찾기

`KOUKUSAYDON_G1_PATTERN_2` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=26139ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":4,"LIGHT":1,"SOUND":20}`; World 0, Summon 1, Scene 1.
서버 창 종류: `{"GAZE_REAL_BOSS":1}`; mechanic trigger: `{"REAL_GAZE_TELEPORT":1}`.
서버 stage 13, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2067 | P2.stage.1 | rpct00_att_phase1_1_01 / P2.animation.1 | 4219823/stage-000/animation-000 | 0/0/2067/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2067–3900 | P2.stage.2 | rpct00_att_phase1_1_06 / P2.animation.2 | 4219824/stage-000/animation-000 | 0/0/1833/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3900–4767 | P2.stage.3 | rpct00_att_battle_20_02 / P2.animation.3 | 4219842/stage-000/animation-000 | 0/0/867/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 4767–6767 | P2.stage.4 | rpct00_att_battle_20_03 / P2.animation.4 | 4219842/stage-001/animation-000 | 0/0/2000/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 6767–9167 | P2.stage.5 | rpct00_att_battle_20_05 / P2.animation.5 | 4219842/stage-002/animation-000 | 0/0/2400/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 9167–11167 | P2.stage.6 | rpct00_att_battle_20_03 / P2.animation.6 | 4219842/stage-003/animation-000 | 0/0/2000/1/EXACT |
| 7/STAGE_7 (ACTIVE) | 11167–13167 | P2.stage.7 | rpct00_att_battle_20_03 / P2.animation.7 | 4219842/stage-004/animation-000 | 0/0/2000/1/EXACT |
| 8/STAGE_8 (ACTIVE) | 13167–15567 | P2.stage.8 | rpct00_att_battle_20_05 / P2.animation.8 | 4219842/stage-005/animation-000 | 0/0/2400/1/EXACT |
| 9/STAGE_9 (ACTIVE) | 15567–17567 | P2.stage.9 | rpct00_att_battle_20_03 / P2.animation.9 | 4219842/stage-006/animation-000 | 0/0/2000/1/EXACT |
| 10/STAGE_10 (ACTIVE) | 17567–19567 | P2.stage.10 | rpct00_att_battle_20_03 / P2.animation.10 | 4219842/stage-007/animation-000 | 0/0/2000/1/EXACT |
| 11/STAGE_11 (ACTIVE) | 19567–21967 | P2.stage.11 | rpct00_att_battle_20_05 / P2.animation.11 | 4219842/stage-008/animation-000 | 0/0/2400/1/EXACT |
| 12/STAGE_12 (ACTIVE) | 21967–23967 | P2.stage.12 | rpct00_att_battle_20_03 / P2.animation.12 | 4219842/stage-009/animation-000 | 0/0/2000/1/EXACT |
| 13/STAGE_13 (ACTIVE) | 23967–26139 | P2.stage.13 | rpct00_att_battle_20_04 / P2.animation.13 | 4219842/stage-010/animation-000 | 0/0/2167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P2.logic.1 | L9 세이튼로직_순간이동 | TRIGGER/REAL_GAZE_TELEPORT | 1988+24146 / True | — |
| P2.logic.3 | L16 세이튼로직_진짜세이튼찾기 | DURATION/GAZE_REAL_BOSS | 19749+5627 / True | F: L3:INSTANT_DEATH |

의존 패턴: P5 ← clonePatternId (P2.logic.1).

<a id="p3"></a>
## P3 · 세이튼_무력화실패

`KOUKUSAYDON_G1_PATTERN_3` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=0ms / category=MECHANIC / 투영=아니오.

미투영 근거: `PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_3`.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.

<a id="p4"></a>
## P4 · 세이튼_무력화성공

`KOUKUSAYDON_G1_PATTERN_4` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=4167ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1167 | P4.stage.2 | rpct00_dmg_critical_start_1 / P4.animation.2 | 4219946/stage-001/animation-000 | 0/0/1167/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1167–2834 | P4.stage.3 | rpct00_dmg_critical_loop_1 / P4.animation.3 | 4219946/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 2834–4167 | P4.stage.4 | rpct00_dmg_critical_end_1 / P4.animation.4 | 4219946/stage-003/animation-000 | 0/0/1333/1/EXACT |
상위 참조: P1 via onSuccess:koukusaydon.g1.logic.7; P80 via onSuccess:koukusaydon.g1.logic.91; P81 via onSuccess:koukusaydon.g1.logic.7.

<a id="p5"></a>
## P5 · 세이튼_가짜세이튼

`KOUKUSAYDON_G1_PATTERN_5` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=26900ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":4,"SOUND":15}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 13, rootMotion 보유 12; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_38 (ACTIVE) | 0–1833 | P5.stage.38 | rpct00_att_phase1_1_06 / P5.animation.38 | 4219857/stage-001/animation-000 | 0/0/1833/1/EXACT |
| 2/STAGE_25 (ACTIVE) | 1833–2700 | P5.stage.25 | rpct00_att_battle_20_02 / P5.animation.25 | 4219891/stage-000/animation-000 | 0/0/867/1/EXACT |
| 3/STAGE_26 (ACTIVE) | 2700–4700 | P5.stage.26 | rpct00_att_battle_20_03 / P5.animation.26 | 4219891/stage-001/animation-000 | 0/0/2000/1/EXACT |
| 4/STAGE_27 (ACTIVE) | 4700–7100 | P5.stage.27 | rpct00_att_battle_20_06 / P5.animation.27 | 4219891/stage-002/animation-000 | 0/0/2400/1/EXACT |
| 5/STAGE_28 (ACTIVE) | 7100–9100 | P5.stage.28 | rpct00_att_battle_20_03 / P5.animation.28 | 4219891/stage-003/animation-000 | 0/0/2000/1/EXACT |
| 6/STAGE_29 (ACTIVE) | 9100–11100 | P5.stage.29 | rpct00_att_battle_20_03 / P5.animation.29 | 4219891/stage-004/animation-000 | 0/0/2000/1/EXACT |
| 7/STAGE_30 (ACTIVE) | 11100–13500 | P5.stage.30 | rpct00_att_battle_20_06 / P5.animation.30 | 4219891/stage-005/animation-000 | 0/0/2400/1/EXACT |
| 8/STAGE_31 (ACTIVE) | 13500–15500 | P5.stage.31 | rpct00_att_battle_20_03 / P5.animation.31 | 4219891/stage-006/animation-000 | 0/0/2000/1/EXACT |
| 9/STAGE_32 (ACTIVE) | 15500–17500 | P5.stage.32 | rpct00_att_battle_20_03 / P5.animation.32 | 4219891/stage-007/animation-000 | 0/0/2000/1/EXACT |
| 10/STAGE_33 (ACTIVE) | 17500–19900 | P5.stage.33 | rpct00_att_battle_20_06 / P5.animation.33 | 4219891/stage-008/animation-000 | 0/0/2400/1/EXACT |
| 11/STAGE_34 (ACTIVE) | 19900–21900 | P5.stage.34 | rpct00_att_battle_20_03 / P5.animation.34 | 4219891/stage-009/animation-000 | 0/0/2000/1/EXACT |
| 12/STAGE_35 (ACTIVE) | 21900–23900 | P5.stage.35 | rpct00_att_battle_20_03 / P5.animation.35 | 4219891/stage-010/animation-000 | 0/0/2000/1/EXACT |
| 13/STAGE_36 (ACTIVE) | 23900–26900 | P5.stage.36 | rpct00_idle_battle_1 / P5.animation.36 | 4219891/stage-011/animation-000 | 0/0/3000/1/EXACT |
상위 참조: P2 via clonePatternId.

<a id="p6"></a>
## P6 · 세이튼_댄스타임

`KOUKUSAYDON_G1_PATTERN_6` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=31467ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"resetBossYawDegrees":237}`.
표현 수: `{"EFFECT":16,"LIGHT":2,"SOUND":39}`; World 1, Summon 0, Scene 1.
서버 창 종류: `{"POSE_INPUT":4}`; mechanic trigger: `{}`.
서버 stage 21, rootMotion 보유 20; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_503 (ACTIVE) | 0–2000 | P6.stage.503 | rpct00_att_battle_25_02 / P6.animation.503 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 2/STAGE_584 (ACTIVE) | 2000–4000 | P6.stage.584 | rpct00_att_battle_25_02 / P6.animation.584 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 3/STAGE_570 (ACTIVE) | 4000–5000 | P6.stage.570 | rpct00_att_battle_25_04_start / P6.animation.570 | 4219895/stage-067/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_571 (ACTIVE) | 5000–5500 | P6.stage.571 | rpct00_att_battle_25_04_loop / P6.animation.571 | 4219895/stage-068/animation-000 | 0/0/500/1/EXACT |
| 5/STAGE_572 (ACTIVE) | 5500–6500 | P6.stage.572 | rpct00_att_battle_25_04_end / P6.animation.572 | 4219895/stage-069/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_573 (ACTIVE) | 6500–7500 | P6.stage.573 | rpct00_att_battle_25_07 / P6.animation.573 | 4219895/stage-070/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_585 (ACTIVE) | 7500–9500 | P6.stage.585 | rpct00_att_battle_25_02 / P6.animation.585 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 8/STAGE_586 (ACTIVE) | 9500–11500 | P6.stage.586 | rpct00_att_battle_25_02 / P6.animation.586 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 9/STAGE_574 (ACTIVE) | 11500–12500 | P6.stage.574 | rpct00_att_battle_25_05_start / P6.animation.574 | 4219895/stage-071/animation-000 | 0/0/1000/1/EXACT |
| 10/STAGE_576 (ACTIVE) | 12500–13500 | P6.stage.576 | rpct00_att_battle_25_05_end / P6.animation.576 | 4219895/stage-073/animation-000 | 0/0/1000/1/EXACT |
| 11/STAGE_582 (ACTIVE) | 13500–15500 | P6.stage.582 | rpct00_att_battle_25_02 / P6.animation.582 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 12/STAGE_581 (ACTIVE) | 15500–17500 | P6.stage.581 | rpct00_att_battle_25_02 / P6.animation.581 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 13/STAGE_577 (ACTIVE) | 17500–18500 | P6.stage.577 | rpct00_att_battle_25_06_start / P6.animation.577 | 4219895/stage-074/animation-000 | 0/0/1000/1/EXACT |
| 14/STAGE_578 (ACTIVE) | 18500–19300 | P6.stage.578 | rpct00_att_battle_25_06_loop / P6.animation.578 | 4219895/stage-075/animation-000 | 0/0/800/1/EXACT |
| 15/STAGE_579 (ACTIVE) | 19300–20300 | P6.stage.579 | rpct00_att_battle_25_06_end / P6.animation.579 | 4219895/stage-076/animation-000 | 0/0/1000/1/EXACT |
| 16/STAGE_583 (ACTIVE) | 20300–22300 | P6.stage.583 | rpct00_att_battle_25_02 / P6.animation.583 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 17/STAGE_589 (ACTIVE) | 22300–23300 | P6.stage.589 | rpct00_att_battle_25_03_start / P6.animation.589 | 4219897/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 18/STAGE_590 (ACTIVE) | 23300–23800 | P6.stage.590 | rpct00_att_battle_25_03_loop / P6.animation.590 | 4219897/stage-002/animation-000 | 0/0/500/1/EXACT |
| 19/STAGE_591 (ACTIVE) | 23800–24800 | P6.stage.591 | rpct00_att_battle_25_03_end / P6.animation.591 | 4219897/stage-003/animation-000 | 0/0/1000/1/EXACT |
| 20/STAGE_587 (ACTIVE) | 24800–26800 | P6.stage.587 | rpct00_att_battle_25_02 / P6.animation.587 | 4219895/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 21/STAGE_580 (ACTIVE) | 26800–31467 | P6.stage.580 | rpct00_att_battle_13_01 / P6.animation.580 | 4219895/stage-077/animation-000 | 0/0/4667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P6.logic.1 | L10 세이튼로직_슈퍼맨 | DURATION/POSE_INPUT | 3969+3606 / True | F: L5:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT; T: L5:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT |
| P6.logic.2 | L11 세이튼로직_양팔벌리기 | DURATION/POSE_INPUT | 11492+2061 / True | F: L4:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT; T: L4:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT |
| P6.logic.3 | L12 세이튼로직_한다리올리기 | DURATION/POSE_INPUT | 17489+2829 / True | F: L4:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT; T: L4:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT |
| P6.logic.4 | L13 세이튼로직_양팔모으기 | DURATION/POSE_INPUT | 22295+2536 / True | F: L6:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT; T: L6:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT |

<a id="p7"></a>
## P7 · 세이튼_룰렛

`KOUKUSAYDON_G1_PATTERN_7` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=34000 / stage 합=33669ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"resetBossYawDegrees":237}`.
표현 수: `{"COLLIDER":26,"SOUND":24,"LIGHT":1,"EFFECT":1}`; World 1, Summon 0, Scene 1.
서버 창 종류: `{"ROULETTE_CARD_MATCH":3,"AREA_OVERLAP":1,"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 9, rootMotion 보유 9; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4333 | P7.stage.1 | rpct00_att_battle_12_06 / P7.animation.1 | 4219811/stage-000/animation-000 | 0/0/4333/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 4333–9000 | P7.stage.3 | rpct00_att_battle_13_01 / P7.animation.10 | 4219813/stage-001/animation-000 | 0/0/4667/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 9000–11667 | P7.stage.4 | rpct00_att_battle_13_02 / P7.animation.4 | 4219813/stage-002/animation-000 | 0/0/2667/1/EXACT |
| 4/STAGE_10 (ACTIVE) | 11667–16334 | P7.stage.10 | rpct00_att_battle_13_01 / P7.animation.12 | 4219813/stage-001/animation-000 | 0/0/4667/1/EXACT |
| 5/STAGE_11 (ACTIVE) | 16334–19001 | P7.stage.11 | rpct00_att_battle_13_02 / P7.animation.13 | 4219813/stage-002/animation-000 | 0/0/2667/1/EXACT |
| 6/STAGE_12 (ACTIVE) | 19001–23668 | P7.stage.12 | rpct00_att_battle_13_01 / P7.animation.14 | 4219813/stage-001/animation-000 | 0/0/4667/1/EXACT |
| 7/STAGE_13 (ACTIVE) | 23668–26335 | P7.stage.13 | rpct00_att_battle_13_02 / P7.animation.15 | 4219813/stage-002/animation-000 | 0/0/2667/1/EXACT |
| 8/STAGE_14 (ACTIVE) | 26335–31002 | P7.stage.14 | rpct00_att_battle_13_01 / P7.animation.17 | 4219813/stage-001/animation-000 | 0/0/4667/1/EXACT |
| 9/STAGE_15 (ACTIVE) | 31002–33669 | P7.stage.15 | rpct00_att_battle_13_02 / P7.animation.18 | 4219813/stage-002/animation-000 | 0/0/2667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P7.logic.1 | L14 세이튼_룰렛_1회차 | DURATION/ROULETTE_CARD_MATCH | 5690+4793 / True | F: L15:MADNESS_GAUGE_ADD_PERCENT, L5:MAX_HP_PERCENT_DAMAGE; T: L15:MADNESS_GAUGE_ADD_PERCENT, L5:MAX_HP_PERCENT_DAMAGE |
| P7.logic.2 | L17 세이튼_룰렛_2회차 | DURATION/ROULETTE_CARD_MATCH | 12734+4755 / True | F: L8:CLOWN_TRANSFORM, L5:MAX_HP_PERCENT_DAMAGE; T: L8:CLOWN_TRANSFORM, L5:MAX_HP_PERCENT_DAMAGE |
| P7.logic.3 | L18 세이튼_룰렛_3회차 | DURATION/ROULETTE_CARD_MATCH | 20098+4914 / True | F: L8:CLOWN_TRANSFORM, L5:MAX_HP_PERCENT_DAMAGE; T: L8:CLOWN_TRANSFORM, L5:MAX_HP_PERCENT_DAMAGE |
| P7.logic.5 | L19 세이튼로직_룰렛4회차 | DURATION/AREA_OVERLAP | 27487+4744 / True | F: L6:MAX_HP_PERCENT_DAMAGE, L15:MADNESS_GAUGE_ADD_PERCENT |
| P7.logic.6 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 31728+420 / True | S: L511:MAX_HP_PERCENT_DAMAGE |

<a id="p8"></a>
## P8 · 쿠크_세이튼등장

`KOUKUSAYDON_G1_PATTERN_8` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=9950 / stage 합=7400ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootVerticalScale":0.8,"bossMotion":{"startMs":1870,"endMs":5780,"startPosition":[2.04,10.56,316.95],"endPosition":[11.79,10.56,326.79],"yawDegrees":314.7368}}`.
표현 수: `{"EFFECT":3,"SOUND":12,"COLLIDER":10}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":10}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_3 (ACTIVE) | 0–7400 | P8.stage.3 | rpcz00_att_battle_7_01 / P8.animation.3 | 4219719/stage-000/animation-000 | 0/0/7400/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P8.logic.1 | L135 세이튼등장_플레이어위치하강공 | DURATION/SHOWTIME_PLAYER_TARGETS | 2800+2001 / True | — |
| P8.logic.2 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3408+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.3 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3588+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.4 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3768+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.5 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3948+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.6 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4128+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.7 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4308+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.8 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4488+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.9 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4668+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.10 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4848+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P8.logic.11 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 5028+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
Bundle 소속: koukusaydon.bundle.1 @0ms.

<a id="p9"></a>
## P9 · 대형세이튼_세이튼등장

`KOUKUSAYDON_G1_PATTERN_9` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=11603 / stage 합=7981ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":4,"SOUND":4,"SUBTITLE":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2647 | P9.stage.1 | mn_rpct_06_sk.ao_respawn_1 / P9.animation.1 | 15/stage-002/animation-000 | 647/0/2000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2647–5314 | P9.stage.2 | mn_rpct_06_sk.ao_idle_normal_1 / P9.animation.2 | 0/stage-003/animation-000 | 0/0/2667/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 5314–7981 | P9.stage.3 | mn_rpct_06_sk.ao_idle_battle_1 / P9.animation.3 | 0/stage-006/animation-000 | 0/0/2667/1/EXACT |
Bundle 소속: koukusaydon.bundle.1 @0ms.

<a id="p10"></a>
## P10 · 쿠크_파1빨2

`KOUKUSAYDON_G1_PATTERN_10` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=None / stage 합=9900ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":9}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 5, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2000 | P10.stage.1 | rpcz00_idle_battle_1 / P10.animation.1 | 4219728/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2000–4000 | P10.stage.2 | rpcz00_idle_battle_1 / P10.animation.2 | 4219728/stage-001/animation-000 | 0/0/2000/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 4000–6667 | P10.stage.3 | rpcz00_att_battle_5_01 / P10.animation.3 | 4219728/stage-002/animation-000 | 0/0/2667/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 6667–7667 | P10.stage.4 | rpcz00_att_battle_5_02 / P10.animation.4 | 4219728/stage-003/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 7667–9900 | P10.stage.5 | rpcz00_att_battle_5_03 / P10.animation.5 | 4219728/stage-004/animation-000 | 0/0/2233/1/EXACT |
Bundle 소속: koukusaydon.bundle.2 @0ms.

<a id="p11"></a>
## P11 · 대형세이튼_파1빨2

`KOUKUSAYDON_G1_PATTERN_11` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=20077 / stage 합=20001ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":41,"SOUND":6,"COLLIDER":7}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"GAZE_REAL_BOSS":3,"ENTER_AREA":3,"INVULNERABILITY_ZONE":4}`; mechanic trigger: `{}`.
서버 stage 6, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4000 | P11.stage.1 | mn_rpct_06_sk.ao_att_battle_7_01 / P11.animation.1 | 4221813/stage-000/animation-000 | 0/0/4000/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 4000–6667 | P11.stage.3 | mn_rpct_06_sk.ao_idle_normal_1 / P11.animation.3 | 0/stage-003/animation-000 | 0/0/2667/1/EXACT |
| 3/STAGE_2 (ACTIVE) | 6667–10667 | P11.stage.2 | mn_rpct_06_sk.ao_att_battle_7_01 / P11.animation.2 | 4221813/stage-001/animation-000 | 0/0/4000/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 10667–13334 | P11.stage.4 | mn_rpct_06_sk.ao_idle_battle_1 / P11.animation.4 | 0/stage-006/animation-000 | 0/0/2667/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 13334–17334 | P11.stage.5 | mn_rpct_06_sk.ao_att_battle_7_01 / P11.animation.5 | 4221813/stage-001/animation-000 | 0/0/4000/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 17334–20001 | P11.stage.6 | mn_rpct_06_sk.ao_idle_battle_1 / P11.animation.6 | 0/stage-006/animation-000 | 0/0/2667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P11.logic.1 | L36 세이튼로직_파1빨2무적 | DURATION/GAZE_REAL_BOSS | 0+710 / True | F: L6:MAX_HP_PERCENT_DAMAGE, L37:FEAR |
| P11.logic.2 | L36 세이튼로직_파1빨2무적 | DURATION/GAZE_REAL_BOSS | 7943+756 / True | F: L37:FEAR, L6:MAX_HP_PERCENT_DAMAGE |
| P11.logic.3 | L36 세이튼로직_파1빨2무적 | DURATION/GAZE_REAL_BOSS | 14526+787 / True | F: L37:FEAR, L6:MAX_HP_PERCENT_DAMAGE |
| P11.logic.4 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 2585+34 / True | S: L3:INSTANT_DEATH |
| P11.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 9252+34 / True | S: L3:INSTANT_DEATH |
| P11.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 15919+34 / True | S: L3:INSTANT_DEATH |
| P11.logic.7 | L101 파1빨2 파랑 원 정확히 1명 무적 | DURATION/INVULNERABILITY_ZONE | 0+17506 / True | — |
| P11.logic.8 | L547 파1빨2 빨강 원 정확히 2명 무적 | DURATION/INVULNERABILITY_ZONE | 0+17491 / True | — |
| P11.logic.9 | L101 파1빨2 파랑 원 정확히 1명 무적 | DURATION/INVULNERABILITY_ZONE | 0+17491 / True | — |
| P11.logic.11 | L547 파1빨2 빨강 원 정확히 2명 무적 | DURATION/INVULNERABILITY_ZONE | 0+17491 / True | — |
Bundle 소속: koukusaydon.bundle.2 @0ms.

<a id="p12"></a>
## P12 · 쿠크_조커찾기

`KOUKUSAYDON_G1_PATTERN_12` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=None / stage 합=5867ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 2, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3867 | P12.stage.1 | rpcz00_idle_normal_1 / P12.animation.1 | 0/stage-003/animation-000 | 0/0/3867/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3867–5867 | P12.stage.2 | rpcz00_idle_battle_1 / P12.animation.2 | 0/stage-006/animation-000 | 0/0/2000/1/EXACT |
Bundle 소속: koukusaydon.bundle.3 @0ms.

<a id="p13"></a>
## P13 · 대형세이튼_조커찾기

`KOUKUSAYDON_G1_PATTERN_13` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=32118 / stage 합=31907ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"COLLIDER":13,"EFFECT":6,"SOUND":20}`; World 7, Summon 0, Scene 0.
서버 창 종류: `{"OBJECT_CONTACT":6,"EXTERNAL_SIGNAL":1,"ENTER_AREA":4}`; mechanic trigger: `{"BOSS_RANDOM_TARGET_PRESENTATION":3}`.
서버 stage 21, rootMotion 보유 12; worldSequences 7.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2667 | P13.stage.1 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.1 | 4221821/stage-000/animation-000 | 0/0/2667/1/EXACT |
| 2/STAGE_7 (ACTIVE) | 2667–3664 | P13.stage.7 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.8 | 4221821/stage-000/animation-000 | 0/0/997/1/EXACT; blend=150 |
| 3/STAGE_3 (ACTIVE) | 3664–4664 | P13.stage.3 | mn_rpct_06_sk.ao_att_battle_8_01 / P13.animation.4 | 4221820/stage-000/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 4/STAGE_5 (ACTIVE) | 4664–5664 | P13.stage.5 | mn_rpct_06_sk.ao_att_battle_1_03 / P13.animation.6 | 4221820/stage-002/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 5/STAGE_13 (ACTIVE) | 5664–6266 | P13.stage.13 | mn_rpct_06_sk.ao_att_battle_1_01 / P13.animation.16 | 4221820/stage-001/animation-000 | 0/0/602/1/EXACT; blend=150 |
| 6/STAGE_6 (ACTIVE) | 6266–8933 | P13.stage.6 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.7 | 4221821/stage-000/animation-000 | 0/0/2667/1/EXACT; blend=150 |
| 7/STAGE_15 (ACTIVE) | 8933–9930 | P13.stage.15 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.18 | 4221821/stage-000/animation-000 | 0/0/997/1/EXACT; blend=150 |
| 8/STAGE_8 (ACTIVE) | 9930–10930 | P13.stage.8 | mn_rpct_06_sk.ao_att_battle_8_01 / P13.animation.9 | 4221820/stage-000/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 9/STAGE_9 (ACTIVE) | 10930–11930 | P13.stage.9 | mn_rpct_06_sk.ao_att_battle_1_03 / P13.animation.10 | 4221820/stage-002/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 10/STAGE_11 (ACTIVE) | 11930–12560 | P13.stage.11 | mn_rpct_06_sk.ao_att_battle_1_01 / P13.animation.12 | 4221820/stage-001/animation-000 | 0/0/630/1/EXACT; blend=150 |
| 11/STAGE_14 (ACTIVE) | 12560–15227 | P13.stage.14 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.17 | 4221821/stage-000/animation-000 | 0/0/2667/1/EXACT; blend=150 |
| 12/STAGE_20 (ACTIVE) | 15227–17894 | P13.stage.20 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.23 | 4221821/stage-000/animation-000 | 0/0/2667/1/EXACT; blend=150 |
| 13/STAGE_21 (ACTIVE) | 17894–18891 | P13.stage.21 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.24 | 4221821/stage-000/animation-000 | 0/0/997/1/EXACT; blend=150 |
| 14/STAGE_22 (ACTIVE) | 18891–19891 | P13.stage.22 | mn_rpct_06_sk.ao_att_battle_8_01 / P13.animation.25 | 4221820/stage-000/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 15/STAGE_23 (ACTIVE) | 19891–20891 | P13.stage.23 | mn_rpct_06_sk.ao_att_battle_1_03 / P13.animation.26 | 4221820/stage-002/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 16/STAGE_24 (ACTIVE) | 20891–21521 | P13.stage.24 | mn_rpct_06_sk.ao_att_battle_1_01 / P13.animation.27 | 4221820/stage-001/animation-000 | 0/0/630/1/EXACT; blend=150 |
| 17/STAGE_25 (ACTIVE) | 21521–24188 | P13.stage.25 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.28 | 4221821/stage-000/animation-000 | 0/0/2667/1/EXACT; blend=150 |
| 18/STAGE_2 (ACTIVE) | 24188–25185 | P13.stage.2 | mn_rpct_06_sk.ao_idle_battle_1 / P13.animation.3 | 4221821/stage-000/animation-000 | 0/0/997/1/EXACT; blend=150 |
| 19/STAGE_16 (ACTIVE) | 25185–26185 | P13.stage.16 | mn_rpct_06_sk.ao_att_battle_8_01 / P13.animation.19 | 4221818/stage-000/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 20/STAGE_17 (ACTIVE) | 26185–27185 | P13.stage.17 | mn_rpct_06_sk.ao_att_battle_8_02 / P13.animation.20 | 4221818/stage-001/animation-000 | 0/0/1000/1/EXACT; blend=150 |
| 21/STAGE_18 (ACTIVE) | 27185–31907 | P13.stage.18 | mn_rpct_06_sk.ao_att_battle_8_03 / P13.animation.21 | 4221818/stage-002/animation-000 | 0/0/2600/1/EXACT; blend=150 |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P13.logic.2 | L22 대형세이튼로직_카드뒤집기 | TRIGGER/OBJECT_CONTACT | 4858+1044 / True | S: L24:PLAY_CONTACT_WORLD_OBJECT_MOTION, L26:COMPLETE_LOGIC_WINDOW |
| P13.logic.3 | L23 대형세이튼로직_카드들썩임 | TRIGGER/OBJECT_CONTACT | 4858+1044 / True | S: L25:PLAY_CONTACT_WORLD_OBJECT_MOTION |
| P13.logic.6 | L20 2관문_거대세이튼_조커찾기 | DURATION/EXTERNAL_SIGNAL | 4573+24717 / True | S: L21:FOLLOWUP_PATTERN; T: L2:INSTANT_DEATH |
| P13.logic.14 | L23 대형세이튼로직_카드들썩임 | TRIGGER/OBJECT_CONTACT | 11244+1045 / True | S: L25:PLAY_CONTACT_WORLD_OBJECT_MOTION |
| P13.logic.17 | L22 대형세이튼로직_카드뒤집기 | TRIGGER/OBJECT_CONTACT | 11244+1045 / True | S: L24:PLAY_CONTACT_WORLD_OBJECT_MOTION, L26:COMPLETE_LOGIC_WINDOW |
| P13.logic.22 | L23 대형세이튼로직_카드들썩임 | TRIGGER/OBJECT_CONTACT | 20205+1045 / True | S: L25:PLAY_CONTACT_WORLD_OBJECT_MOTION |
| P13.logic.23 | L22 대형세이튼로직_카드뒤집기 | TRIGGER/OBJECT_CONTACT | 20205+1045 / True | S: L24:PLAY_CONTACT_WORLD_OBJECT_MOTION, L26:COMPLETE_LOGIC_WINDOW |
| P13.logic.26 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4855+506 / True | S: L521:MAX_HP_PERCENT_DAMAGE |
| P13.logic.27 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 11225+500 / True | S: L521:MAX_HP_PERCENT_DAMAGE |
| P13.logic.28 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 20212+449 / True | S: L521:MAX_HP_PERCENT_DAMAGE |
| P13.logic.29 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 29085+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P13.logic.30 | L546 조커 찾기 랜덤 표적 설정 | TRIGGER/BOSS_RANDOM_TARGET | 0+34 / False | — |
| P13.logic.31 | L546 조커 찾기 랜덤 표적 설정 | TRIGGER/BOSS_RANDOM_TARGET | 6266+34 / False | — |
| P13.logic.32 | L546 조커 찾기 랜덤 표적 설정 | TRIGGER/BOSS_RANDOM_TARGET | 12560+34 / False | — |
| P13.logic.33 | L557 조커찾기_랜덤표적설정과시선이동 | DURATION/BOSS_RANDOM_TARGET | 0+2658 / True | — |
| P13.logic.34 | L557 조커찾기_랜덤표적설정과시선이동 | DURATION/BOSS_RANDOM_TARGET | 6244+2658 / True | — |
| P13.logic.35 | L557 조커찾기_랜덤표적설정과시선이동 | DURATION/BOSS_RANDOM_TARGET | 12587+5323 / True | — |

의존 패턴: P14 ← onSuccess:koukusaydon.g1.logic.21 (P13.logic.6).
Bundle 소속: koukusaydon.bundle.3 @0ms.

<a id="p14"></a>
## P14 · 대형세이튼_조커찾기_성공

`KOUKUSAYDON_G1_PATTERN_14` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=None / stage 합=2333ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":2}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2333 | P14.stage.1 | mn_rpct_06_sk.ao_att_battle_4_02 / P14.animation.1 | 4221816/stage-000/animation-000 | 0/0/2333/1/EXACT |
상위 참조: P13 via onSuccess:koukusaydon.g1.logic.21.

<a id="p15"></a>
## P15 · 쿠크_거미카운터

`KOUKUSAYDON_G1_PATTERN_15` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=None / stage 합=21501ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"COLLIDER":6,"EFFECT":15,"SOUND":24}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"COUNTER_WINDOW":3,"ENTER_AREA":6}`; mechanic trigger: `{}`.
서버 stage 15, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2500 | P15.stage.1 | rpcz00_att_battle_6_01 / P15.animation.1 | 4219776/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2500–3500 | P15.stage.2 | rpcz00_att_battle_6_02 / P15.animation.2 | 4219776/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 3/STAGE_26 (ACTIVE) | 3500–4500 | P15.stage.26 | rpcz00_att_battle_6_02 / P15.animation.26 | 4219776/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_3 (ACTIVE) | 4500–5667 | P15.stage.3 | rpcz00_att_battle_6_03 / P15.animation.3 | 4219776/stage-002/animation-000 | 0/0/1167/1/EXACT |
| 5/STAGE_4 (ACTIVE) | 5667–7167 | P15.stage.4 | rpcz00_att_battle_6_04 / P15.animation.4 | 4219776/stage-003/animation-000 | 0/0/1500/1/EXACT |
| 6/STAGE_28 (ACTIVE) | 7167–9667 | P15.stage.28 | rpcz00_att_battle_6_01 / P15.animation.28 | 4219776/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 7/STAGE_29 (ACTIVE) | 9667–10667 | P15.stage.29 | rpcz00_att_battle_6_02 / P15.animation.29 | 4219776/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 8/STAGE_30 (ACTIVE) | 10667–11667 | P15.stage.30 | rpcz00_att_battle_6_02 / P15.animation.30 | 4219776/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 9/STAGE_31 (ACTIVE) | 11667–12834 | P15.stage.31 | rpcz00_att_battle_6_03 / P15.animation.31 | 4219776/stage-002/animation-000 | 0/0/1167/1/EXACT |
| 10/STAGE_33 (ACTIVE) | 12834–14334 | P15.stage.33 | rpcz00_att_battle_6_04 / P15.animation.33 | 4219776/stage-003/animation-000 | 0/0/1500/1/EXACT |
| 11/STAGE_34 (ACTIVE) | 14334–16834 | P15.stage.34 | rpcz00_att_battle_6_01 / P15.animation.34 | 4219776/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 12/STAGE_35 (ACTIVE) | 16834–17834 | P15.stage.35 | rpcz00_att_battle_6_02 / P15.animation.35 | 4219776/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 13/STAGE_36 (ACTIVE) | 17834–18834 | P15.stage.36 | rpcz00_att_battle_6_02 / P15.animation.36 | 4219776/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 14/STAGE_37 (ACTIVE) | 18834–20001 | P15.stage.37 | rpcz00_att_battle_6_03 / P15.animation.37 | 4219776/stage-002/animation-000 | 0/0/1167/1/EXACT |
| 15/STAGE_39 (ACTIVE) | 20001–21501 | P15.stage.39 | rpcz00_att_battle_6_04 / P15.animation.39 | 4219776/stage-003/animation-000 | 0/0/1500/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P15.logic.1 | L29 2관문_쿠크_거미카운터_카운터 | DURATION/COUNTER_WINDOW | 2500+2000 / True | S: L30:FOLLOWUP_PATTERN |
| P15.logic.2 | L29 2관문_쿠크_거미카운터_카운터 | DURATION/COUNTER_WINDOW | 9667+2000 / True | S: L30:FOLLOWUP_PATTERN |
| P15.logic.3 | L29 2관문_쿠크_거미카운터_카운터 | DURATION/COUNTER_WINDOW | 16834+2000 / True | S: L30:FOLLOWUP_PATTERN |
| P15.logic.4 | L31 2관문_트리거_거미카운터_공포 | TRIGGER/ENTER_AREA | 4500+1167 / True | S: L35:FEAR |
| P15.logic.5 | L31 2관문_트리거_거미카운터_공포 | TRIGGER/ENTER_AREA | 11667+1167 / True | S: L35:FEAR |
| P15.logic.6 | L31 2관문_트리거_거미카운터_공포 | TRIGGER/ENTER_AREA | 18834+1167 / True | S: L35:FEAR |
| P15.logic.7 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4600+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P15.logic.8 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 11767+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P15.logic.9 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 18934+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

의존 패턴: P22 ← onSuccess:koukusaydon.g1.logic.30 (P15.logic.1); P22 ← onSuccess:koukusaydon.g1.logic.30 (P15.logic.2); P22 ← onSuccess:koukusaydon.g1.logic.30 (P15.logic.3).
Bundle 소속: koukusaydon.bundle.6 @0ms.

<a id="p17"></a>
## P17 · 대형세이튼_잡기

`KOUKUSAYDON_G1_PATTERN_17` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=None / stage 합=7500ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"COLLIDER":7,"EFFECT":4,"SOUND":9}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":5,"ATTACHMENT_HOLD":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4000 | P17.stage.1 | mn_rpct_06_sk.ao_att_battle_6_01 / P17.animation.1 | 4221810/stage-000/animation-000 | 0/0/4000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 4000–5500 | P17.stage.2 | mn_rpct_06_sk.ao_att_battle_6_02 / P17.animation.2 | 4221810/stage-001/animation-000 | 0/0/1500/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 5500–7500 | P17.stage.3 | mn_rpct_06_sk.ao_att_battle_6_04 / P17.animation.3 | 4221810/stage-002/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P17.logic.1 | L32 대형세이튼_잡기판정 | TRIGGER/ENTER_AREA | 2029+788 / True | S: L34:CAPTURE_PLAYER |
| P17.logic.2 | L33 대형세이튼_잡기유지 | DURATION/ATTACHMENT_HOLD | 2029+5455 / True | — |
| P17.logic.3 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4280+34 / False | S: L105:MAX_HP_PERCENT_DAMAGE |
| P17.logic.4 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7156+111 / False | S: L105:MAX_HP_PERCENT_DAMAGE |
| P17.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4257+177 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P17.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4810+177 / False | S: L98:MAX_HP_PERCENT_DAMAGE |
| P17.logic.7 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5317+177 / False | S: L98:MAX_HP_PERCENT_DAMAGE |
| P17.logic.8 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7148+177 / False | S: L98:MAX_HP_PERCENT_DAMAGE |
| P17.logic.9 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4820+177 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P17.logic.10 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5332+177 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P17.logic.11 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7148+177 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
Bundle 소속: koukusaydon.bundle.7 @0ms.

<a id="p25"></a>
## P25 · 쿠크_피자

`KOUKUSAYDON_G1_PATTERN_25` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=19914 / stage 합=19093ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"resetBossYawDegrees":216.5}`.
표현 수: `{"EFFECT":20,"SOUND":20,"COLLIDER":13}`; World 0, Summon 1, Scene 0.
서버 창 종류: `{"ENTER_AREA":13}`; mechanic trigger: `{"BOSS_TELEPORT_GROUNDED":1,"SUMMON_PATTERNS":1}`.
서버 stage 15, rootMotion 보유 15; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_64 (ACTIVE) | 0–1500 | P25.stage.64 | rpcz00_att_battle_10_01 / P25.animation.64 | 4219769/stage-002/animation-000 | 0/0/1500/1/EXACT |
| 2/STAGE_4 (ACTIVE) | 1500–3500 | P25.stage.4 | rpcz00_att_battle_10_02 / P25.animation.4 | 4219769/stage-003/animation-000 | 0/0/2000/1/EXACT |
| 3/STAGE_6 (ACTIVE) | 3500–4500 | P25.stage.6 | rpcz00_att_battle_10_03 / P25.animation.6 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_19 (ACTIVE) | 4500–5500 | P25.stage.19 | rpcz00_att_battle_10_03 / P25.animation.19 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_20 (ACTIVE) | 5500–6500 | P25.stage.20 | rpcz00_att_battle_10_03 / P25.animation.20 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_21 (ACTIVE) | 6500–7500 | P25.stage.21 | rpcz00_att_battle_10_03 / P25.animation.21 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_22 (ACTIVE) | 7500–8500 | P25.stage.22 | rpcz00_att_battle_10_03 / P25.animation.22 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 8/STAGE_23 (ACTIVE) | 8500–9500 | P25.stage.23 | rpcz00_att_battle_10_03 / P25.animation.23 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 9/STAGE_24 (ACTIVE) | 9500–10500 | P25.stage.24 | rpcz00_att_battle_10_03 / P25.animation.24 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 10/STAGE_25 (ACTIVE) | 10500–11500 | P25.stage.25 | rpcz00_att_battle_10_03 / P25.animation.25 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 11/STAGE_99 (ACTIVE) | 11500–12500 | P25.stage.99 | rpcz00_att_battle_10_03 / P25.animation.99 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 12/STAGE_100 (ACTIVE) | 12500–13500 | P25.stage.100 | rpcz00_att_battle_10_03 / P25.animation.100 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 13/STAGE_101 (ACTIVE) | 13500–14500 | P25.stage.101 | rpcz00_att_battle_10_03 / P25.animation.101 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 14/STAGE_102 (ACTIVE) | 14500–15500 | P25.stage.102 | rpcz00_att_battle_10_03 / P25.animation.102 | 4219769/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 15/STAGE_60 (ACTIVE) | 15500–19093 | P25.stage.60 | rpcz00_att_battle_10_04 / P25.animation.60 | 4219769/stage-016/animation-000 | 0/0/2667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P25.logic.2 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3721+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.3 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4628+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.4 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5514+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6548+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7560+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.7 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8617+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.8 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 9442+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.9 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 10486+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.10 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 11582+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.11 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 12427+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.12 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 13409+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.13 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 14409+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P25.logic.14 | L103 쿠크_중앙이동 | TRIGGER/BOSS_TELEPORT_GROUNDED | 0+1000 / True | — |
| P25.logic.15 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 15409+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

의존 패턴: P84 ← summon.patternSpawns (P25.summon.1).
Bundle 소속: koukusaydon.bundle.5 @0ms.

<a id="p26"></a>
## P26 · 대형세이튼_피자

`KOUKUSAYDON_G1_PATTERN_26` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=19498 / stage 합=17600ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"resetBossYawDegrees":226.5}`.
표현 수: `{"EFFECT":2,"SOUND":3,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 16, rootMotion 보유 16; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1000 | P26.stage.2 | mn_rpct_06_sk.ao_att_battle_8_01 / P26.animation.2 | 4221819/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1000–2000 | P26.stage.3 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.3 | 4221819/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 2000–3000 | P26.stage.4 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.4 | 4221819/stage-003/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_5 (ACTIVE) | 3000–4000 | P26.stage.5 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.5 | 4221819/stage-004/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_6 (ACTIVE) | 4000–5000 | P26.stage.6 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.6 | 4221819/stage-005/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_7 (ACTIVE) | 5000–6000 | P26.stage.7 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.7 | 4221819/stage-006/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_9 (ACTIVE) | 6000–7000 | P26.stage.9 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.9 | 4221819/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 8/STAGE_10 (ACTIVE) | 7000–8000 | P26.stage.10 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.10 | 4221819/stage-003/animation-000 | 0/0/1000/1/EXACT |
| 9/STAGE_14 (ACTIVE) | 8000–9000 | P26.stage.14 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.14 | 4221819/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 10/STAGE_16 (ACTIVE) | 9000–10000 | P26.stage.16 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.16 | 4221819/stage-003/animation-000 | 0/0/1000/1/EXACT |
| 11/STAGE_18 (ACTIVE) | 10000–11000 | P26.stage.18 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.18 | 4221819/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 12/STAGE_19 (ACTIVE) | 11000–12000 | P26.stage.19 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.19 | 4221819/stage-003/animation-000 | 0/0/1000/1/EXACT |
| 13/STAGE_17 (ACTIVE) | 12000–13000 | P26.stage.17 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.17 | 4221819/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 14/STAGE_15 (ACTIVE) | 13000–14000 | P26.stage.15 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.15 | 4221819/stage-003/animation-000 | 0/0/1000/1/EXACT |
| 15/STAGE_11 (ACTIVE) | 14000–15000 | P26.stage.11 | mn_rpct_06_sk.ao_att_battle_8_02 / P26.animation.11 | 4221819/stage-004/animation-000 | 0/0/1000/1/EXACT |
| 16/STAGE_8 (ACTIVE) | 15000–17600 | P26.stage.8 | mn_rpct_06_sk.ao_att_battle_8_03 / P26.animation.8 | 4221819/stage-007/animation-000 | 0/0/2600/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P26.logic.1 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 16997+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
Bundle 소속: koukusaydon.bundle.5 @0ms.

<a id="p20"></a>
## P20 · 대형세이튼_레이저

`KOUKUSAYDON_G1_PATTERN_20` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=None / stage 합=0ms / category=MECHANIC / 투영=아니오.

미투영 근거: `PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_20`.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
Bundle 소속: koukusaydon.bundle.8 @0ms.

<a id="p21"></a>
## P21 · 쿠크_레이저

`KOUKUSAYDON_G1_PATTERN_21` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=6492 / stage 합=5833ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootHorizontalScale":0}`.
표현 수: `{"EFFECT":7,"SOUND":7,"COLLIDER":7}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":7}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–2333 | P21.stage.2 | rpcz00_att_battle_1_03 / P21.animation.2 | 4219740/stage-001/animation-000 | 0/0/2333/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 2333–4833 | P21.stage.3 | rpcz00_att_battle_1_04 / P21.animation.3 | 4219740/stage-002/animation-000 | 0/0/2500/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 4833–5833 | P21.stage.4 | rpcz00_att_battle_1_07 / P21.animation.4 | 4219740/stage-003/animation-000 | 0/0/1000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P21.logic.9 | L120 트리거_플레이어타겟설정 | TRIGGER/None | 0+356 / True | — |
| P21.logic.10 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 1196+231 / True | S: L502:MAX_HP_PERCENT_DAMAGE |
| P21.logic.11 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 1555+222 / True | S: L502:MAX_HP_PERCENT_DAMAGE |
| P21.logic.12 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 1896+231 / True | S: L502:MAX_HP_PERCENT_DAMAGE |
| P21.logic.13 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 3535+231 / True | S: L502:MAX_HP_PERCENT_DAMAGE |
| P21.logic.14 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 3894+231 / True | S: L502:MAX_HP_PERCENT_DAMAGE |
| P21.logic.15 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 4241+231 / True | S: L502:MAX_HP_PERCENT_DAMAGE |
| P21.logic.16 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 5250+231 / True | S: L502:MAX_HP_PERCENT_DAMAGE |
Bundle 소속: koukusaydon.bundle.8 @0ms.

<a id="p22"></a>
## P22 · 쿠크_그로기

`KOUKUSAYDON_G1_PATTERN_22` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=None / stage 합=3500ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1000 | P22.stage.1 | rpcz00_dmg_critical_start_1 / P22.animation.1 | 4219763/stage-000/animation-000 | 0/0/1000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1000–2333 | P22.stage.2 | rpcz00_dmg_critical_loop_1 / P22.animation.2 | 4219763/stage-001/animation-000 | 0/0/1333/1/LOOP_TO_WINDOW |
| 3/STAGE_3 (ACTIVE) | 2333–3500 | P22.stage.3 | rpcz00_dmg_critical_end_1 / P22.animation.3 | 4219763/stage-002/animation-000 | 0/0/1167/1/EXACT |
상위 참조: P15 via onSuccess:koukusaydon.g1.logic.30.

<a id="p23"></a>
## P23 · 쿠크_팡파레

`KOUKUSAYDON_G1_PATTERN_23` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=16869 / stage 합=15945ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"animationRootHorizontalScale":0}`.
표현 수: `{"COLLIDER":15,"EFFECT":9,"SOUND":29}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":7}`; mechanic trigger: `{"BOSS_TELEPORT_GROUNDED":1}`.
서버 stage 6, rootMotion 보유 2; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_7 (ACTIVE) | 0–2500 | P23.stage.7 | rpcz00_att_battle_3_01 / P23.animation.7 | 4219715/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_8 (ACTIVE) | 2500–5700 | P23.stage.8 | rpcz00_att_battle_3_08 / P23.animation.8 | 4219715/stage-001/animation-000 | 0/0/3200/1/EXACT |
| 3/STAGE_9 (ACTIVE) | 5700–6700 | P23.stage.9 | rpcz00_att_battle_3_09 / P23.animation.9 | 4219715/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_1 (ACTIVE) | 6700–9200 | P23.stage.1 | rpcz00_att_battle_3_01 / P23.animation.1 | 4219714/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 5/STAGE_2 (ACTIVE) | 9200–13867 | P23.stage.2 | rpcz00_att_battle_3_07 / P23.animation.2 | 4219714/stage-001/animation-000 | 0/0/4667/1/EXACT |
| 6/STAGE_3 (ACTIVE) | 13867–15945 | P23.stage.3 | rpcz00_att_battle_3_09 / P23.animation.3 | 4219714/stage-002/animation-000 | 0/0/1000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P23.logic.1 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6206+305 / True | S: L150:MAX_HP_PERCENT_DAMAGE |
| P23.logic.2 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6703+288 / True | S: L150:MAX_HP_PERCENT_DAMAGE |
| P23.logic.3 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7323+270 / True | S: L150:MAX_HP_PERCENT_DAMAGE |
| P23.logic.4 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 10425+34 / True | S: L113:MAX_HP_PERCENT_DAMAGE |
| P23.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 11385+34 / True | S: L114:MAX_HP_PERCENT_DAMAGE |
| P23.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 12318+34 / True | S: L115:MAX_HP_PERCENT_DAMAGE |
| P23.logic.7 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 13278+34 / True | S: L116:MAX_HP_PERCENT_DAMAGE |
| P23.logic.8 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 10463+240 / False | S: L108:MAX_HP_PERCENT_DAMAGE |
| P23.logic.9 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 11404+231 / False | S: L108:MAX_HP_PERCENT_DAMAGE |
| P23.logic.10 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 12324+231 / False | S: L108:MAX_HP_PERCENT_DAMAGE |
| P23.logic.11 | L41 2관문_쿠크_레이저 | TRIGGER/ENTER_AREA | 13319+231 / False | S: L108:MAX_HP_PERCENT_DAMAGE |
| P23.logic.12 | L103 쿠크_중앙이동 | TRIGGER/BOSS_TELEPORT_GROUNDED | 0+523 / True | — |
Bundle 소속: koukusaydon.bundle.10 @0ms.

<a id="p24"></a>
## P24 · 쿠크_휠윈드

`KOUKUSAYDON_G1_PATTERN_24` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=11000 / stage 합=10873ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootHorizontalScale":0}`.
표현 수: `{"COLLIDER":17,"EFFECT":1,"SOUND":7}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":17}`; mechanic trigger: `{}`.
서버 stage 23, rootMotion 보유 0; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2500 | P24.stage.1 | rpcz00_att_battle_2_01 / P24.animation.1 | 4219708/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2500–4000 | P24.stage.2 | rpcz00_att_battle_2_04 / P24.animation.2 | 4219708/stage-001/animation-000 | 0/0/1500/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 4000–4267 | P24.stage.3 | rpcz00_att_battle_2_05 / P24.animation.3 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 4/STAGE_11 (ACTIVE) | 4267–4534 | P24.stage.11 | rpcz00_att_battle_2_05 / P24.animation.11 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 5/STAGE_12 (ACTIVE) | 4534–4801 | P24.stage.12 | rpcz00_att_battle_2_05 / P24.animation.12 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 6/STAGE_13 (ACTIVE) | 4801–5068 | P24.stage.13 | rpcz00_att_battle_2_05 / P24.animation.13 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 7/STAGE_14 (ACTIVE) | 5068–5335 | P24.stage.14 | rpcz00_att_battle_2_05 / P24.animation.14 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 8/STAGE_15 (ACTIVE) | 5335–5602 | P24.stage.15 | rpcz00_att_battle_2_05 / P24.animation.15 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 9/STAGE_16 (ACTIVE) | 5602–5869 | P24.stage.16 | rpcz00_att_battle_2_05 / P24.animation.16 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 10/STAGE_17 (ACTIVE) | 5869–6136 | P24.stage.17 | rpcz00_att_battle_2_05 / P24.animation.17 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 11/STAGE_18 (ACTIVE) | 6136–6403 | P24.stage.18 | rpcz00_att_battle_2_05 / P24.animation.18 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 12/STAGE_19 (ACTIVE) | 6403–6670 | P24.stage.19 | rpcz00_att_battle_2_05 / P24.animation.19 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 13/STAGE_20 (ACTIVE) | 6670–6937 | P24.stage.20 | rpcz00_att_battle_2_05 / P24.animation.20 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 14/STAGE_21 (ACTIVE) | 6937–7204 | P24.stage.21 | rpcz00_att_battle_2_05 / P24.animation.21 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 15/STAGE_22 (ACTIVE) | 7204–7471 | P24.stage.22 | rpcz00_att_battle_2_05 / P24.animation.22 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 16/STAGE_23 (ACTIVE) | 7471–7738 | P24.stage.23 | rpcz00_att_battle_2_05 / P24.animation.23 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 17/STAGE_24 (ACTIVE) | 7738–8005 | P24.stage.24 | rpcz00_att_battle_2_05 / P24.animation.24 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 18/STAGE_25 (ACTIVE) | 8005–8272 | P24.stage.25 | rpcz00_att_battle_2_05 / P24.animation.25 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 19/STAGE_26 (ACTIVE) | 8272–8539 | P24.stage.26 | rpcz00_att_battle_2_05 / P24.animation.26 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 20/STAGE_27 (ACTIVE) | 8539–8806 | P24.stage.27 | rpcz00_att_battle_2_05 / P24.animation.27 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 21/STAGE_28 (ACTIVE) | 8806–9073 | P24.stage.28 | rpcz00_att_battle_2_05 / P24.animation.28 | 4219708/stage-002/animation-000 | 0/0/267/1/EXACT |
| 22/STAGE_4 (ACTIVE) | 9073–9973 | P24.stage.4 | rpcz00_att_battle_2_06 / P24.animation.4 | 4219708/stage-003/animation-000 | 0/0/900/1/EXACT |
| 23/STAGE_32 (ACTIVE) | 9973–10873 | P24.stage.32 | rpcz00_att_battle_2_06 / P24.animation.32 | 4219708/stage-003/animation-000 | 0/0/900/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P24.logic.1 | L38 2관문_쿠크_휠윈드 | TRIGGER/ENTER_AREA | 4000+5078 / True | S: L39:MAX_HP_PERCENT_DAMAGE |
| P24.logic.4 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4077+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4344+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4611+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.7 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4878+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.8 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5145+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.9 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5412+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.10 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5679+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.11 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5946+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.12 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6213+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.13 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6480+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.14 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6747+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.15 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7014+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.16 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7601+67 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.17 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7302+47 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.18 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8033+1 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P24.logic.19 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7810+25 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
Bundle 소속: koukusaydon.bundle.9 @0ms.

<a id="p18"></a>
## P18 · 3관문_외곽불회전_갈고리대각선_시각테스트

`KOUKUSAYDON_G1_PATTERN_18` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=36000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":12}`; World 7, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":18}`; mechanic trigger: `{}`.
서버 stage 12, rootMotion 보유 0; worldSequences 7.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P18.stage.1 | rpct00_idle_normal_1 / P18.animation.1 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–6000 | P18.stage.2 | rpct00_idle_normal_1 / P18.animation.2 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 6000–9000 | P18.stage.3 | rpct00_idle_normal_1 / P18.animation.3 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 9000–12000 | P18.stage.4 | rpct00_idle_normal_1 / P18.animation.4 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 12000–15000 | P18.stage.5 | rpct00_idle_normal_1 / P18.animation.5 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 15000–18000 | P18.stage.6 | rpct00_idle_normal_1 / P18.animation.6 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 7/STAGE_7 (ACTIVE) | 18000–21000 | P18.stage.7 | rpct00_idle_normal_1 / P18.animation.7 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 8/STAGE_8 (ACTIVE) | 21000–24000 | P18.stage.8 | rpct00_idle_normal_1 / P18.animation.8 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 9/STAGE_9 (ACTIVE) | 24000–27000 | P18.stage.9 | rpct00_idle_normal_1 / P18.animation.9 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 10/STAGE_10 (ACTIVE) | 27000–30000 | P18.stage.10 | rpct00_idle_normal_1 / P18.animation.10 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 11/STAGE_11 (ACTIVE) | 30000–33000 | P18.stage.11 | rpct00_idle_normal_1 / P18.animation.11 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 12/STAGE_12 (ACTIVE) | 33000–36000 | P18.stage.12 | rpct00_idle_normal_1 / P18.animation.12 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |

<a id="p19"></a>
## P19 · 3관문_갈고리만_확인용_불없음

`KOUKUSAYDON_G1_PATTERN_19` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=36000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":12}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":18}`; mechanic trigger: `{}`.
서버 stage 12, rootMotion 보유 0; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P19.stage.1 | rpct00_idle_normal_1 / P19.animation.1 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–6000 | P19.stage.2 | rpct00_idle_normal_1 / P19.animation.2 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 6000–9000 | P19.stage.3 | rpct00_idle_normal_1 / P19.animation.3 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 9000–12000 | P19.stage.4 | rpct00_idle_normal_1 / P19.animation.4 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 12000–15000 | P19.stage.5 | rpct00_idle_normal_1 / P19.animation.5 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 15000–18000 | P19.stage.6 | rpct00_idle_normal_1 / P19.animation.6 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 7/STAGE_7 (ACTIVE) | 18000–21000 | P19.stage.7 | rpct00_idle_normal_1 / P19.animation.7 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 8/STAGE_8 (ACTIVE) | 21000–24000 | P19.stage.8 | rpct00_idle_normal_1 / P19.animation.8 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 9/STAGE_9 (ACTIVE) | 24000–27000 | P19.stage.9 | rpct00_idle_normal_1 / P19.animation.9 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 10/STAGE_10 (ACTIVE) | 27000–30000 | P19.stage.10 | rpct00_idle_normal_1 / P19.animation.10 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 11/STAGE_11 (ACTIVE) | 30000–33000 | P19.stage.11 | rpct00_idle_normal_1 / P19.animation.11 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 12/STAGE_12 (ACTIVE) | 33000–36000 | P19.stage.12 | rpct00_idle_normal_1 / P19.animation.12 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |

<a id="p27"></a>
## P27 · 대형세이튼_불뿜기

`KOUKUSAYDON_G1_PATTERN_27` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=None / stage 합=5500ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"COLLIDER":4,"EFFECT":3,"SOUND":2}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–5500 | P27.stage.2 | mn_rpct_06_sk.ao_att_battle_5_02 / P27.animation.2 | 4221809/stage-001/animation-000 | 0/0/5500/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P27.logic.1 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 2471+34 / False | S: L105:MAX_HP_PERCENT_DAMAGE |
| P27.logic.3 | L540 Breath: three contact ticks | DURATION/AREA_OVERLAP | 2366+2340 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |

<a id="p28"></a>
## P28 · 쿠크_카드미로연출

`KOUKUSAYDON_G1_PATTERN_28` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=None / stage 합=7840ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":9,"SOUND":8}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"CARD_MAZE_ENTER":1,"CARD_MAZE_HIDE_NEXT":4}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_27 (ACTIVE) | 0–2500 | P28.stage.27 | rpcz00_att_battle_3_01 / P28.animation.27 | 4219751/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_28 (ACTIVE) | 2500–4967 | P28.stage.28 | rpcz00_att_battle_3_05 / P28.animation.28 | 4219751/stage-001/animation-000 | 0/0/2467/1/EXACT |
| 3/STAGE_29 (ACTIVE) | 4967–7840 | P28.stage.29 | rpcz00_att_battle_3_10 / P28.animation.29 | 4219751/stage-002/animation-000 | 0/0/382/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P28.logic.1 | L40 2관문_카드미로진입 | TRIGGER/CARD_MAZE_ENTER | 5136+186 / True | — |
| P28.logic.2 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 3744+236 / True | — |
| P28.logic.3 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 4182+236 / True | — |
| P28.logic.4 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 4529+236 / True | — |
| P28.logic.5 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 4824+236 / True | — |
Bundle 소속: koukusaydon.bundle.4 @0ms.

<a id="p31"></a>
## P31 · 3관문 칼날 바닥

`KOUKUSAYDON_G1_PATTERN_31` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=6000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":1}`; World 5, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":40}`; mechanic trigger: `{}`.
서버 stage 2, rootMotion 보유 0; worldSequences 5.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P31.stage.1 | rpct00_idle_normal_1 / P31.animation.1 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–6000 | P31.stage.2 | rpct00_idle_battle_1 / P31.animation.2 | 0/stage-006/animation-000 | 0/0/3000/1/EXACT |

<a id="p33"></a>
## P33 · 세이튼_마리오_2페이즈

`KOUKUSAYDON_G1_PATTERN_33` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=27830 / stage 합=27464ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":5,"SOUND":9}`; World 3, Summon 0, Scene 0.
서버 창 종류: `{"STAGGER_WINDOW":1,"BOSS_DAMAGE_REDUCTION":1,"ENTER_AREA":55}`; mechanic trigger: `{"MARIO_PHASE2_PLAYERS":1,"BOSS_TELEPORT_FACE_CENTER":1}`.
서버 stage 15, rootMotion 보유 15; worldSequences 3.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_20 (ACTIVE) | 0–2793 | P33.stage.20 | rpct00_att_battle_29_02 / P33.animation.20 | 4219993/stage-004/animation-000 | 1126/0/1667/1/EXACT |
| 2/STAGE_21 (ACTIVE) | 2793–4460 | P33.stage.21 | rpct00_att_battle_29_02 / P33.animation.21 | 4219993/stage-005/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_22 (ACTIVE) | 4460–6127 | P33.stage.22 | rpct00_att_battle_29_02 / P33.animation.22 | 4219993/stage-004/animation-000 | 0/0/1667/1/EXACT |
| 4/STAGE_23 (ACTIVE) | 6127–7794 | P33.stage.23 | rpct00_att_battle_29_02 / P33.animation.23 | 4219993/stage-005/animation-000 | 0/0/1667/1/EXACT |
| 5/STAGE_24 (ACTIVE) | 7794–9461 | P33.stage.24 | rpct00_att_battle_29_02 / P33.animation.24 | 4219993/stage-004/animation-000 | 0/0/1667/1/EXACT |
| 6/STAGE_25 (ACTIVE) | 9461–11128 | P33.stage.25 | rpct00_att_battle_29_02 / P33.animation.25 | 4219993/stage-005/animation-000 | 0/0/1667/1/EXACT |
| 7/STAGE_26 (ACTIVE) | 11128–12795 | P33.stage.26 | rpct00_att_battle_29_02 / P33.animation.26 | 4219993/stage-004/animation-000 | 0/0/1667/1/EXACT |
| 8/STAGE_27 (ACTIVE) | 12795–14462 | P33.stage.27 | rpct00_att_battle_29_02 / P33.animation.27 | 4219993/stage-005/animation-000 | 0/0/1667/1/EXACT |
| 9/STAGE_28 (ACTIVE) | 14462–16129 | P33.stage.28 | rpct00_att_battle_29_02 / P33.animation.28 | 4219993/stage-004/animation-000 | 0/0/1667/1/EXACT |
| 10/STAGE_29 (ACTIVE) | 16129–17796 | P33.stage.29 | rpct00_att_battle_29_02 / P33.animation.29 | 4219993/stage-005/animation-000 | 0/0/1667/1/EXACT |
| 11/STAGE_30 (ACTIVE) | 17796–19463 | P33.stage.30 | rpct00_att_battle_29_02 / P33.animation.30 | 4219993/stage-004/animation-000 | 0/0/1667/1/EXACT |
| 12/STAGE_31 (ACTIVE) | 19463–21130 | P33.stage.31 | rpct00_att_battle_29_02 / P33.animation.31 | 4219993/stage-005/animation-000 | 0/0/1667/1/EXACT |
| 13/STAGE_32 (ACTIVE) | 21130–22797 | P33.stage.32 | rpct00_att_battle_29_02 / P33.animation.32 | 4219993/stage-004/animation-000 | 0/0/1667/1/EXACT |
| 14/STAGE_33 (ACTIVE) | 22797–24464 | P33.stage.33 | rpct00_att_battle_29_02 / P33.animation.33 | 4219993/stage-005/animation-000 | 0/0/1667/1/EXACT |
| 15/STAGE_59 (ACTIVE) | 24464–27464 | P33.stage.59 | rpct00_att_battle_25_01 / P33.animation.59 | 4219905/stage-001/animation-000 | 0/0/3000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P33.logic.1 | L56 쿠크세이튼_마리오_무력화 | DURATION/STAGGER_WINDOW | 0+24497 / True | S: L57:FOLLOWUP_PATTERN; T: L2:INSTANT_DEATH |
| P33.logic.2 | L89 마리오_2페이즈_플레이어오른쪽이동 | TRIGGER/MARIO_PHASE2_PLAYERS | 1091+1000 / True | — |
| P33.logic.3 | L90 마리오_2페이즈_쿠크세이튼왼쪽이동 | TRIGGER/BOSS_TELEPORT_FACE_CENTER | 1091+1000 / True | — |
| P33.logic.4 | L558 마리오_2페이즈_무력화_대미지감소 | DURATION/BOSS_DAMAGE_REDUCTION | 0+24493 / True | — |

의존 패턴: P42 ← onSuccess:koukusaydon.g1.logic.57 (P33.logic.1).
상위 참조: P34 via onSuccess:koukusaydon.g1.logic.60; P88 via onSuccess:koukusaydon.g1.logic.93; P91 via onSuccess:koukusaydon.g1.logic.94; P92 via onSuccess:koukusaydon.g1.logic.95; P93 via onSuccess:koukusaydon.g1.logic.96.

<a id="p34"></a>
## P34 · 세이튼_마리오_1페이즈

`KOUKUSAYDON_G1_PATTERN_34` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=39738ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"COLLIDER":3,"SOUND":3}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"PATTERN_COMPLETION_COUNT":1,"ENTER_AREA":1,"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_19 (ACTIVE) | 0–39738 | P34.stage.19 | rpct00_att_battle_27_01 / P34.animation.19 | 4219911/stage-000/animation-000 | 0/0/4667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P34.logic.1 | L55 쿠크세이튼_마리오_랜덤패턴 | DURATION/PATTERN_COMPLETION_COUNT | 4650+35088 / True | S: L60:FOLLOWUP_PATTERN |
| P34.logic.2 | L58 마리오_괴기스러운인형_공_랜덤스폰 | TRIGGER/None | 3058+761 / True | — |
| P34.logic.3 | L59 마리오_입장 | TRIGGER/ENTER_AREA | 4635+11435 / True | S: L61:MARIO_ENTER; T: L92:INSTANT_DEATH |
| P34.logic.5 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3138+14413 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |

의존 패턴: P38 ← patternIds (P34.logic.1); P39 ← patternIds (P34.logic.1); P43 ← patternIds (P34.logic.1); P33 ← onSuccess:koukusaydon.g1.logic.60 (P34.logic.1).

<a id="p35"></a>
## P35 · 세이튼_쇼타임

`KOUKUSAYDON_G1_PATTERN_35` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=58386ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":391,"SOUND":150,"COLLIDER":156}`; World 7, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":111}`; mechanic trigger: `{"BOSS_TELEPORT_XZ":1,"BOSS_TRACK_TARGET":4,"BOSS_TELEPORT_GROUNDED":1}`.
서버 stage 21, rootMotion 보유 20; worldSequences 7.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–933 | P35.stage.1 | rpct00_att_battle_28_05_start / P35.animation.1 | 4219985/stage-000/animation-000 | 0/0/933/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 933–4100 | P35.stage.2 | rpct00_att_battle_28_05_loop_a / P35.animation.2 | 4219985/stage-001/animation-000 | 0/0/3167/1/LOOP_TO_WINDOW |
| 3/STAGE_13 (ACTIVE) | 4100–5500 | P35.stage.13 | rpct00_att_battle_28_05_end / P35.animation.13 | 4219985/stage-012/animation-000 | 0/0/1400/1/EXACT |
| 4/STAGE_57 (ACTIVE) | 5500–7118 | P35.stage.57 | rpct00_att_battle_28_08 / P35.animation.56 | 4219939/stage-006/animation-000 | 0/1024/448/1/EXACT; sourceEnd=1472 |
| 5/STAGE_22 (ACTIVE) | 7118–10789 | P35.stage.22 | rpct00_att_battle_28_08 / P35.animation.22 | 4219939/stage-006/animation-000 | 50/1230/3603/1/EXACT; sourceEnd=4833 |
| 6/STAGE_59 (ACTIVE) | 10789–12499 | P35.stage.59 | rpct00_att_battle_28_08 / P35.animation.58 | 4219939/stage-006/animation-000 | 0/1024/448/1/EXACT; sourceEnd=1472 |
| 7/STAGE_60 (ACTIVE) | 12499–16124 | P35.stage.60 | rpct00_att_battle_28_08 / P35.animation.59 | 4219939/stage-006/animation-000 | 0/1208/3625/1/EXACT; sourceEnd=4833 |
| 8/STAGE_4 (ACTIVE) | 16124–24846 | P35.stage.4 | rpct00_att_battle_28_06 / P35.animation.4 | 4219985/stage-003/animation-000 | 0/1011/8722/1/EXACT; sourceEnd=9733 |
| 9/STAGE_58 (ACTIVE) | 24846–26977 | P35.stage.58 | rpct00_att_battle_28_08 / P35.animation.57 | 4219939/stage-006/animation-000 | 0/1024/448/1/EXACT; sourceEnd=1472 |
| 10/STAGE_14 (ACTIVE) | 26977–30504 | P35.stage.14 | rpct00_att_battle_28_08 / P35.animation.14 | 4219985/stage-013/animation-000 | 0/1306/3527/1/EXACT; sourceEnd=4833 |
| 11/STAGE_61 (ACTIVE) | 30504–32590 | P35.stage.61 | rpct00_att_battle_28_08 / P35.animation.60 | 4219939/stage-006/animation-000 | 0/1024/448/1/EXACT; sourceEnd=1472 |
| 12/STAGE_62 (ACTIVE) | 32590–35955 | P35.stage.62 | rpct00_att_battle_28_08 / P35.animation.61 | 4219985/stage-013/animation-000 | 0/1468/3365/1/EXACT; sourceEnd=4833 |
| 13/STAGE_7 (ACTIVE) | 35955–38188 | P35.stage.7 | rpct00_att_battle_28_09 / P35.animation.7 | 4219985/stage-006/animation-000 | 0/0/2233/1/EXACT |
| 14/STAGE_8 (ACTIVE) | 38188–39188 | P35.stage.8 | rpct00_att_battle_24_03 / P35.animation.8 | 4219985/stage-007/animation-000 | 0/0/1000/1/EXACT |
| 15/STAGE_9 (ACTIVE) | 39188–41521 | P35.stage.9 | rpct00_att_battle_28_10_start / P35.animation.9 | 4219985/stage-008/animation-000 | 0/0/2333/1/EXACT |
| 16/STAGE_10 (ACTIVE) | 41521–44854 | P35.stage.10 | rpct00_att_battle_28_10_loop / P35.animation.10 | 4219985/stage-009/animation-000 | 0/0/3333/1/LOOP_TO_WINDOW |
| 17/STAGE_54 (ACTIVE) | 44854–48187 | P35.stage.54 | rpct00_att_battle_28_10_loop / P35.animation.53 | 4219985/stage-009/animation-000 | 0/0/3333/1/LOOP_TO_WINDOW |
| 18/STAGE_55 (ACTIVE) | 48187–51520 | P35.stage.55 | rpct00_att_battle_28_10_loop / P35.animation.54 | 4219985/stage-009/animation-000 | 0/0/3333/1/LOOP_TO_WINDOW |
| 19/STAGE_56 (ACTIVE) | 51520–54853 | P35.stage.56 | rpct00_att_battle_28_10_loop / P35.animation.55 | 4219985/stage-009/animation-000 | 0/0/3333/1/LOOP_TO_WINDOW |
| 20/STAGE_11 (ACTIVE) | 54853–55986 | P35.stage.11 | rpct00_att_battle_28_10_end / P35.animation.11 | 4219985/stage-010/animation-000 | 0/0/1133/1/EXACT |
| 21/STAGE_12 (ACTIVE) | 55986–58386 | P35.stage.12 | rpct00_att_battle_28_11 / P35.animation.12 | 4219985/stage-011/animation-000 | 0/0/2400/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P35.logic.6 | L63 세이튼로직_쇼타임_왼쪽위순간이동 | TRIGGER/BOSS_TELEPORT_XZ | 39209+1000 / True | — |
| P35.logic.7 | L64 쿠크세이튼_쇼타임_추적고정조준점타겟 | DURATION/SHOWTIME_PLAYER_TARGETS | 41315+17071 / True | — |
| P35.logic.11 | L550 쇼타임_추적회전_폭탄판정_1 | DURATION/BOSS_TRACK_TARGET | 5742+1513 / True | — |
| P35.logic.12 | L551 쇼타임_추적회전_폭탄판정_2 | DURATION/BOSS_TRACK_TARGET | 11066+1469 / True | — |
| P35.logic.14 | L552 쇼타임_추적회전_폭탄판정_3 | DURATION/BOSS_TRACK_TARGET | 25163+1848 / True | — |
| P35.logic.16 | L553 쇼타임_추적회전_폭탄판정_4 | DURATION/BOSS_TRACK_TARGET | 30791+1875 / True | — |
| P35.logic.17 | L100 Showtime center return to server floor | TRIGGER/BOSS_TELEPORT_GROUNDED | 55869+1000 / True | — |
| P35.logic.18 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 933+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.19 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 1233+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.20 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 1533+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.21 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 1833+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.22 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 2133+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.23 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 2433+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.24 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 2733+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.25 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3033+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.26 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3333+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.27 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3633+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.28 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7438+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.29 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7738+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.30 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8038+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.31 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8338+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.32 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8638+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.33 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8938+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.34 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 12791+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.35 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 13091+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.36 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 13391+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.37 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 13691+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.38 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 13991+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.39 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 14291+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.40 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 16213+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.41 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 16513+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.42 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 16813+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.43 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 17113+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.44 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 17413+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.45 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 17713+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.46 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 18013+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.47 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 18313+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.48 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 18613+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.49 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 18913+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.50 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 19213+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.51 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 19513+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.52 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 19913+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.53 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 20213+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.54 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 20513+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.55 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 20813+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.56 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 21113+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.57 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 21413+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.58 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 21713+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.59 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 21913+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.60 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 22113+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.61 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 22313+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.62 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 22513+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.63 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 22713+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.64 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 22913+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.65 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 27171+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.66 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 27471+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.67 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 27771+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.68 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 28071+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.69 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 28371+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.70 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 28671+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.71 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 32622+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.72 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 32922+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.73 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 33222+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.74 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 33522+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.75 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 33822+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.76 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 34122+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.77 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 1502+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.78 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 1500+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.79 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 10821+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.80 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4620+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.81 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 17690+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.82 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 18875+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.83 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 26686+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.84 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 20794+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.85 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 28507+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.86 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 28505+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.87 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 37803+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.88 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 37801+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.89 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6472+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.90 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 10798+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.91 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 14589+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.92 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 16006+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.93 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 22774+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.94 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 24897+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.95 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8690+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.96 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8688+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.97 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 12865+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.98 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 31679+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P35.logic.99 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 46849+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.100 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 47249+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.101 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 47549+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.102 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 43820+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.103 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 44220+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.104 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 44520+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.105 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 45143+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.106 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 45543+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.107 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 45843+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.108 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50447+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.109 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50847+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.110 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 51147+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.111 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 47954+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.112 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 48354+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.113 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 48654+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.114 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50178+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.115 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50578+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.116 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50878+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.117 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 51492+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.118 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 51892+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.119 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 52192+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.120 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 52438+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.121 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 52838+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.122 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 53138+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.123 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50971+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.124 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 51371+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.125 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 51671+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.126 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 49704+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.127 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50104+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P35.logic.128 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 50404+34 / True | S: L538:MAX_HP_PERCENT_DAMAGE |

<a id="p36"></a>
## P36 · 세이튼_1관문연출

`KOUKUSAYDON_G1_PATTERN_36` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=30375ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"bossMotion":{"startMs":0,"endMs":4997,"startPosition":[6.43,1.3,730],"endPosition":[-0.07,1.3,737.53],"yawDegrees":-130.801213684}}`.
표현 수: `{"SOUND":17}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 13, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4997 | P36.stage.1 | rpct00_walk_normal_1 / P36.animation.1 | 0/RAW/rpct00_walk_normal_1 | 0/0/4997/1/LOOP_TO_WINDOW |
| 2/STAGE_2 (ACTIVE) | 4997–9272 | P36.stage.2 | rpct00_idle_normal_1 / P36.animation.2 | 0/RAW/rpct00_idle_normal_1 | 0/0/4275/0.699999988079071/LOOP_TO_WINDOW |
| 3/STAGE_3 (ACTIVE) | 9272–11719 | P36.stage.3 | rpct00_idle_phase1_start_1 / P36.animation.3 | 0/RAW/rpct00_idle_phase1_start_1 | 0/0/2447/0.800000011920929/HOLD_LAST_POSE |
| 4/STAGE_4 (ACTIVE) | 11719–14880 | P36.stage.4 | rpct00_att_battle_25_01 / P36.animation.4 | 0/RAW/rpct00_att_battle_25_01 | 0/0/3161/1/HOLD_LAST_POSE |
| 5/STAGE_5 (ACTIVE) | 14880–16973 | P36.stage.5 | rpct00_att_battle_13_02 / P36.animation.5 | 0/RAW/rpct00_att_battle_13_02 | 0/0/2093/1/HOLD_LAST_POSE |
| 6/STAGE_6 (ACTIVE) | 16973–19670 | P36.stage.6 | rpct00_att_battle_13_04 / P36.animation.6 | 0/RAW/rpct00_att_battle_13_04 | 0/0/2697/1/LOOP_TO_WINDOW |
| 7/STAGE_7 (ACTIVE) | 19670–19967 | P36.stage.7 | rpct00_evt2_blk6 / P36.animation.7 | 0/RAW/rpct00_evt2_blk6 | 0/248/286/1/HOLD_LAST_POSE; sourceEnd=534 |
| 8/STAGE_8 (ACTIVE) | 19967–20999 | P36.stage.8 | rpct00_evt2_rpct_talk_01 / P36.animation.8 | 0/RAW/rpct00_evt2_rpct_talk_01 | 0/933/1024/1/HOLD_LAST_POSE; sourceEnd=1957 |
| 9/STAGE_9 (ACTIVE) | 20999–22605 | P36.stage.9 | rpct00_walk_normal_1 / P36.animation.9 | 0/RAW/rpct00_walk_normal_1 | 0/0/1598/1/LOOP_TO_WINDOW |
| 10/STAGE_10 (ACTIVE) | 22605–23504 | P36.stage.10 | rpct00_idle_phase1_start_1 / P36.animation.10 | 0/RAW/rpct00_idle_phase1_start_1 | 0/0/899/1/HOLD_LAST_POSE |
| 11/STAGE_14 (ACTIVE) | 23504–27562 | P36.stage.14 | rpct00_att_battle_13_01 / P36.animation.14 | 0/RAW/rpct00_att_battle_13_01 | 0/0/4058/0.8999999761581421/HOLD_LAST_POSE |
| 12/STAGE_12 (ACTIVE) | 27562–28365 | P36.stage.12 | rpct00_att_battle_13_02 / P36.animation.12 | 0/RAW/rpct00_att_battle_13_02 | 0/74/803/1/HOLD_LAST_POSE; sourceEnd=877 |
| 13/STAGE_13 (ACTIVE) | 28365–30375 | P36.stage.13 | rpct00_idle_battle_1 / P36.animation.13 | 0/RAW/rpct00_idle_battle_1 | 0/1000/2010/1/HOLD_LAST_POSE |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P36.logic.1 | L62 세이튼_1관문연출_애니메이션_블렌딩 | TRIGGER/ANIMATION_BLEND | 14767+1000 / True | — |
| P36.logic.2 | L62 세이튼_1관문연출_애니메이션_블렌딩 | TRIGGER/ANIMATION_BLEND | 16850+1000 / True | — |
| P36.logic.3 | L62 세이튼_1관문연출_애니메이션_블렌딩 | TRIGGER/ANIMATION_BLEND | 19648+276 / True | — |
| P36.logic.4 | L62 세이튼_1관문연출_애니메이션_블렌딩 | TRIGGER/ANIMATION_BLEND | 20984+276 / True | — |
| P36.logic.5 | L62 세이튼_1관문연출_애니메이션_블렌딩 | TRIGGER/ANIMATION_BLEND | 28310+699 / True | — |

<a id="p38"></a>
## P38 · 쿠크세이튼_무지개댄스

`KOUKUSAYDON_G1_PATTERN_38` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=15200ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":2,"COLLIDER":20,"SOUND":31}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":20}`; mechanic trigger: `{"BOSS_TELEPORT_XZ":1}`.
서버 stage 15, rootMotion 보유 14; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2067 | P38.stage.1 | rpct00_att_phase1_1_01 / P38.animation.1 | 4219938/stage-000/animation-000 | 0/0/2067/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2067–3900 | P38.stage.2 | rpct00_att_phase1_1_06 / P38.animation.2 | 4219938/stage-001/animation-000 | 0/0/1833/1/EXACT |
| 3/STAGE_13 (ACTIVE) | 3900–4900 | P38.stage.13 | rpct00_att_battle_25_04_start / P38.animation.13 | 4219907/stage-004/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_14 (ACTIVE) | 4900–5400 | P38.stage.14 | rpct00_att_battle_25_04_loop / P38.animation.14 | 4219907/stage-005/animation-000 | 0/0/500/1/LOOP_TO_WINDOW |
| 5/STAGE_15 (ACTIVE) | 5400–6400 | P38.stage.15 | rpct00_att_battle_25_04_end / P38.animation.15 | 4219907/stage-006/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_41 (ACTIVE) | 6400–7400 | P38.stage.41 | rpct00_att_battle_25_05_start / P38.animation.41 | 4219908/stage-000/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_42 (ACTIVE) | 7400–7900 | P38.stage.42 | rpct00_att_battle_25_05_loop / P38.animation.42 | 4219908/stage-001/animation-000 | 0/0/500/1/LOOP_TO_WINDOW |
| 8/STAGE_43 (ACTIVE) | 7900–8900 | P38.stage.43 | rpct00_att_battle_25_05_end / P38.animation.43 | 4219908/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 9/STAGE_25 (ACTIVE) | 8900–9900 | P38.stage.25 | rpct00_att_battle_25_06_start / P38.animation.25 | 4219909/stage-000/animation-000 | 0/0/1000/1/EXACT |
| 10/STAGE_26 (ACTIVE) | 9900–10700 | P38.stage.26 | rpct00_att_battle_25_06_loop / P38.animation.26 | 4219909/stage-001/animation-000 | 0/0/800/1/LOOP_TO_WINDOW |
| 11/STAGE_27 (ACTIVE) | 10700–11700 | P38.stage.27 | rpct00_att_battle_25_06_end / P38.animation.27 | 4219909/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 12/STAGE_16 (ACTIVE) | 11700–12700 | P38.stage.16 | rpct00_att_battle_25_07 / P38.animation.16 | 4219907/stage-007/animation-000 | 0/0/1000/1/EXACT |
| 13/STAGE_57 (ACTIVE) | 12700–13700 | P38.stage.57 | rpct00_att_battle_25_04_start / P38.animation.57 | 4219907/stage-004/animation-000 | 0/0/1000/1/EXACT |
| 14/STAGE_58 (ACTIVE) | 13700–14200 | P38.stage.58 | rpct00_att_battle_25_04_loop / P38.animation.58 | 4219907/stage-005/animation-000 | 0/0/500/1/LOOP_TO_WINDOW |
| 15/STAGE_59 (ACTIVE) | 14200–15200 | P38.stage.59 | rpct00_att_battle_25_04_end / P38.animation.59 | 4219907/stage-006/animation-000 | 0/0/1000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P38.logic.1 | L47 중앙이동 | TRIGGER/BOSS_TELEPORT_XZ | 1990+674 / True | — |
| P38.logic.2 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5359+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.3 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5159+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.4 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5759+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.5 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5559+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.6 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5959+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.7 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7059+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.8 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6859+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.9 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7459+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.10 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7259+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.11 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7659+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.12 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 11500+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.13 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 11300+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.14 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 11100+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.15 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 10900+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.16 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 10700+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.17 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 13200+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.18 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 13000+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.19 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 12800+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.20 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 12600+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P38.logic.21 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 12400+1000 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
상위 참조: P34 via patternIds; P91 via patternOccurrence.

<a id="p39"></a>
## P39 · 쿠크세이튼_알비온_감전장판

`KOUKUSAYDON_G1_PATTERN_39` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=13378ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true}`.
표현 수: `{"EFFECT":16,"SOUND":13,"COLLIDER":17}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":10}`; mechanic trigger: `{"ALBION_AIRBORNE":8,"ALBION_BLUE_CIRCLE":1}`.
서버 stage 6, rootMotion 보유 6; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_4 (ACTIVE) | 0–2500 | P39.stage.4 | rpct00_att_battle_24_01 / P39.animation.4 | 4219903/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_5 (ACTIVE) | 2500–3500 | P39.stage.5 | rpct00_att_battle_24_02 / P39.animation.5 | 4219903/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 3/STAGE_6 (ACTIVE) | 3500–4500 | P39.stage.6 | rpct00_att_battle_24_03 / P39.animation.6 | 4219903/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_7 (ACTIVE) | 4500–6918 | P39.stage.7 | rpct00_att_battle_24_04 / P39.animation.7 | 4219903/stage-003/animation-000 | 918/0/1500/1/EXACT |
| 5/STAGE_8 (ACTIVE) | 6918–9323 | P39.stage.8 | rpct00_att_battle_24_04 / P39.animation.8 | 4219903/stage-004/animation-000 | 905/0/1500/1/EXACT |
| 6/STAGE_11 (ACTIVE) | 9323–13378 | P39.stage.11 | rpct00_att_battle_24_05 / P39.animation.11 | 4219903/stage-007/animation-000 | 1427/0/2500/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P39.logic.1 | L53 로직_랜덤플레이어탐색 | TRIGGER/ALBION_AIRBORNE | 5432+230 / True | — |
| P39.logic.3 | L53 로직_랜덤플레이어탐색 | TRIGGER/ALBION_AIRBORNE | 7817+291 / True | — |
| P39.logic.5 | L66 쿠크세이튼_알비온_점프 | TRIGGER/ALBION_AIRBORNE | 3483+294 / True | — |
| P39.logic.7 | L68 쿠크세이튼_알비온_순간이동_등장 | TRIGGER/ALBION_AIRBORNE | 5432+225 / True | — |
| P39.logic.8 | L67 쿠크세이튼_알비온_순간이동_사라지기 | TRIGGER/ALBION_AIRBORNE | 6913+233 / True | — |
| P39.logic.9 | L69 쿠크세이튼_알비온_순간이동_중앙 | TRIGGER/ALBION_AIRBORNE | 9292+241 / True | — |
| P39.logic.10 | L70 쿠크세이튼_알비온_내려치기_중앙 | TRIGGER/ALBION_AIRBORNE | 11808+302 / True | — |
| P39.logic.11 | L68 쿠크세이튼_알비온_순간이동_등장 | TRIGGER/ALBION_AIRBORNE | 7817+225 / True | — |
| P39.logic.12 | L72 쿠크세이튼_알비온_플레이어추적_파란장판 | TRIGGER/ALBION_BLUE_CIRCLE | 4426+1000 / True | — |
| P39.logic.13 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6473+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P39.logic.14 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6474+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P39.logic.15 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6453+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P39.logic.16 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6444+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P39.logic.17 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6445+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P39.logic.18 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6424+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P39.logic.19 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6383+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P39.logic.20 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6490+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P39.logic.21 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8849+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P39.logic.22 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 12241+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
상위 참조: P34 via patternIds; P92 via patternOccurrence; P93 via patternOccurrence.

<a id="p40"></a>
## P40 · 쿠크세이튼_십자화염폭발

`KOUKUSAYDON_G1_PATTERN_40` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=7267ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":3,"EFFECT":3,"COLLIDER":4}`; World 0, Summon 1, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{"SUMMON_PATTERNS":1}`.
서버 stage 2, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P40.stage.1 | rpct00_idle_battle_1 / P40.animation.1 | 4219948/stage-000/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–7267 | P40.stage.2 | rpct00_att_battle_10_02 / P40.animation.2 | 4219948/stage-001/animation-000 | 0/0/4267/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P40.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 6113+1154 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |

의존 패턴: P63 ← summon.patternSpawns (P40.summon.1); P64 ← summon.patternSpawns (P40.summon.1).
상위 참조: P93 via patternOccurrence.

<a id="p41"></a>
## P41 · 쿠크세이튼_갈고리_칼날댄스

`KOUKUSAYDON_G1_PATTERN_41` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=19868ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":14}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 12, rootMotion 보유 4; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2067 | P41.stage.1 | rpct00_att_battle_9_04 / P41.animation.1 | 4219951/stage-000/animation-000 | 0/0/2067/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2067–3734 | P41.stage.2 | rpct00_att_battle_9_01 / P41.animation.2 | 4219951/stage-001/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3734–4734 | P41.stage.3 | rpct00_att_battle_9_02 / P41.animation.3 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 4734–6734 | P41.stage.4 | rpct00_att_battle_9_03 / P41.animation.4 | 4219951/stage-003/animation-000 | 0/0/2000/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 6734–8334 | P41.stage.5 | rpct00_att_battle_34_04 / P41.animation.5 | 4219951/stage-004/animation-000 | 0/0/1600/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 8334–9934 | P41.stage.6 | rpct00_att_battle_34_03 / P41.animation.6 | 4219951/stage-005/animation-000 | 0/0/1600/1/EXACT |
| 7/STAGE_7 (ACTIVE) | 9934–12001 | P41.stage.7 | rpct00_att_battle_9_04 / P41.animation.7 | 4219951/stage-006/animation-000 | 0/0/2067/1/EXACT |
| 8/STAGE_8 (ACTIVE) | 12001–13668 | P41.stage.8 | rpct00_att_battle_9_01 / P41.animation.8 | 4219951/stage-007/animation-000 | 0/0/1667/1/EXACT |
| 9/STAGE_9 (ACTIVE) | 13668–14668 | P41.stage.9 | rpct00_att_battle_9_02 / P41.animation.9 | 4219951/stage-008/animation-000 | 0/0/1000/1/EXACT |
| 10/STAGE_10 (ACTIVE) | 14668–16668 | P41.stage.10 | rpct00_att_battle_9_03 / P41.animation.10 | 4219951/stage-009/animation-000 | 0/0/2000/1/EXACT |
| 11/STAGE_11 (ACTIVE) | 16668–18268 | P41.stage.11 | rpct00_att_battle_34_04 / P41.animation.11 | 4219951/stage-010/animation-000 | 0/0/1600/1/EXACT |
| 12/STAGE_12 (ACTIVE) | 18268–19868 | P41.stage.12 | rpct00_att_battle_34_03 / P41.animation.12 | 4219951/stage-011/animation-000 | 0/0/1600/1/EXACT |

<a id="p42"></a>
## P42 · 쿠크세이튼_무력화성공

`KOUKUSAYDON_G1_PATTERN_42` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=4167ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1167 | P42.stage.2 | rpct00_dmg_critical_start_1 / P42.animation.2 | 4219946/stage-001/animation-000 | 0/0/1167/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1167–2834 | P42.stage.3 | rpct00_dmg_critical_loop_1 / P42.animation.3 | 4219946/stage-002/animation-000 | 0/0/1667/1/LOOP_TO_WINDOW |
| 3/STAGE_4 (ACTIVE) | 2834–4167 | P42.stage.4 | rpct00_dmg_critical_end_1 / P42.animation.4 | 4219946/stage-003/animation-000 | 0/0/1333/1/EXACT |
상위 참조: P33 via onSuccess:koukusaydon.g1.logic.57; P117 via onSuccess:koukusaydon.g1.logic.140; P118 via onSuccess:koukusaydon.g1.logic.57.

<a id="p43"></a>
## P43 · 쿠크세이튼_백스텝불뿜기_화염링

`KOUKUSAYDON_G1_PATTERN_43` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=12554 / stage 합=6434ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":8,"SOUND":4,"COLLIDER":10}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":8}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_5 (ACTIVE) | 0–1600 | P43.stage.5 | rpct00_att_battle_34_04 / P43.animation.5 | 4219951/stage-004/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_15 (ACTIVE) | 1600–3267 | P43.stage.15 | rpct00_att_battle_14_01 / P43.animation.15 | 4219940/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_16 (ACTIVE) | 3267–6434 | P43.stage.16 | rpct00_att_battle_14_02 / P43.animation.16 | 4219940/stage-003/animation-000 | 0/0/3167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P43.logic.1 | L541 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1555+4894 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
| P43.logic.2 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3378+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P43.logic.3 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3362+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P43.logic.4 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3385+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P43.logic.5 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3361+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P43.logic.6 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3338+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P43.logic.7 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3315+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P43.logic.8 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3315+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P34 via patternIds; P92 via patternOccurrence.

<a id="p44"></a>
## P44 · 쿠크세이튼_분신소환_기분나빠

`KOUKUSAYDON_G1_PATTERN_44` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=6400ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":8}`; World 0, Summon 1, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 4, rootMotion 보유 4; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P44.stage.1 | rpct00_att_battle_34_01 / P44.animation.1 | 4219920/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–3200 | P44.stage.2 | rpct00_att_battle_34_02 / P44.animation.2 | 4219921/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3200–4800 | P44.stage.3 | rpct00_att_battle_34_04 / P44.animation.3 | 4219922/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 4800–6400 | P44.stage.4 | rpct00_att_battle_34_03 / P44.animation.4 | 4219923/stage-000/animation-000 | 0/0/1600/1/EXACT |

<a id="p45"></a>
## P45 · 쿠크세이튼_분신소환_십자화염폭발

`KOUKUSAYDON_G1_PATTERN_45` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=0ms / category=MECHANIC / 투영=아니오.

미투영 근거: `PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_45`.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.

<a id="p46"></a>
## P46 · 쿠크세이튼_벡스텝후감전빔

`KOUKUSAYDON_G1_PATTERN_46` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=3600ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":6,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P46.stage.1 | rpct00_att_battle_34_02 / P46.animation.1 | 4219921/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_5 (ACTIVE) | 1600–3600 | P46.stage.5 | rpct00_att_battle_1_01 / P46.animation.5 | 4219959/stage-000/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P46.logic.2 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 2600+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p47"></a>
## P47 · 세이튼_트럼펫장판소환

`KOUKUSAYDON_G1_PATTERN_47` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=14409 / stage 합=10933ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":2,"SOUND":8,"COLLIDER":2}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":2}`; mechanic trigger: `{}`.
서버 stage 9, rootMotion 보유 9; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1333 | P47.stage.2 | rpct00_att_battle_19_01_start / P47.animation.2 | 4219807/stage-001/animation-000 | 0/0/1333/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1333–2500 | P47.stage.3 | rpct00_att_battle_19_01_loop / P47.animation.3 | 4219807/stage-002/animation-000 | 0/0/1167/1/LOOP_TO_WINDOW |
| 3/STAGE_37 (ACTIVE) | 2500–3667 | P47.stage.37 | rpct00_att_battle_19_01_loop / P47.animation.37 | 4219807/stage-002/animation-000 | 0/0/1167/1/LOOP_TO_WINDOW |
| 4/STAGE_51 (ACTIVE) | 3667–4700 | P47.stage.51 | rpct00_att_battle_19_01_end / P47.animation.51 | 4219807/stage-012/animation-000 | 0/0/1033/1/EXACT |
| 5/STAGE_70 (ACTIVE) | 4700–6033 | P47.stage.70 | rpct00_att_battle_19_01_start / P47.animation.69 | 4219807/stage-001/animation-000 | 0/0/1333/1/EXACT |
| 6/STAGE_71 (ACTIVE) | 6033–7200 | P47.stage.71 | rpct00_att_battle_19_01_loop / P47.animation.70 | 4219807/stage-002/animation-000 | 0/0/1167/1/LOOP_TO_WINDOW |
| 7/STAGE_72 (ACTIVE) | 7200–8367 | P47.stage.72 | rpct00_att_battle_19_01_loop / P47.animation.71 | 4219807/stage-002/animation-000 | 0/0/1167/1/LOOP_TO_WINDOW |
| 8/STAGE_73 (ACTIVE) | 8367–9400 | P47.stage.73 | rpct00_att_battle_19_01_end / P47.animation.72 | 4219807/stage-012/animation-000 | 0/0/1033/1/EXACT |
| 9/STAGE_4 (ACTIVE) | 9400–10933 | P47.stage.4 | rpct00_att_battle_5_01 / P47.animation.4 | 4219807/stage-003/animation-000 | 0/0/1533/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P47.logic.11 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3624+231 / True | S: L554:MAX_HP_PERCENT_DAMAGE |
| P47.logic.12 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 8495+231 / True | S: L554:MAX_HP_PERCENT_DAMAGE |

<a id="p48"></a>
## P48 · 세이튼_빙글빙글돌며카드던지기

`KOUKUSAYDON_G1_PATTERN_48` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=15261 / stage 합=15233ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":7,"SOUND":6,"COLLIDER":6}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":6}`; mechanic trigger: `{}`.
서버 stage 8, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2833 | P48.stage.1 | rpct00_att_battle_4_04 / P48.animation.1 | 4219819/stage-000/animation-000 | 0/0/2833/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2833–3633 | P48.stage.2 | rpct00_att_battle_4_02 / P48.animation.2 | 4219819/stage-001/animation-000 | 0/0/800/1/EXACT |
| 3/STAGE_7 (ACTIVE) | 3633–4433 | P48.stage.7 | rpct00_att_battle_4_02 / P48.animation.7 | 4219819/stage-001/animation-000 | 0/0/800/1/EXACT |
| 4/STAGE_8 (ACTIVE) | 4433–5233 | P48.stage.8 | rpct00_att_battle_4_02 / P48.animation.8 | 4219819/stage-001/animation-000 | 0/0/800/1/EXACT |
| 5/STAGE_9 (ACTIVE) | 5233–6033 | P48.stage.9 | rpct00_att_battle_4_02 / P48.animation.9 | 4219819/stage-001/animation-000 | 0/0/800/1/EXACT |
| 6/STAGE_10 (ACTIVE) | 6033–6833 | P48.stage.10 | rpct00_att_battle_4_02 / P48.animation.10 | 4219819/stage-001/animation-000 | 0/0/800/1/EXACT |
| 7/STAGE_11 (ACTIVE) | 6833–11733 | P48.stage.11 | rpct00_att_battle_4_02 / P48.animation.11 | 4219819/stage-001/animation-000 | 0/0/4900/1/LOOP_TO_WINDOW |
| 8/STAGE_3 (ACTIVE) | 11733–15233 | P48.stage.3 | rpct00_att_battle_4_03 / P48.animation.3 | 4219819/stage-002/animation-000 | 0/0/3500/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P48.logic.1 | L74 세이튼_회전카드_원작12발 | DURATION/PURSUIT_PROJECTILES | 3533+8200 / True | — |
| P48.logic.4 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3033+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P48.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3833+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P48.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4633+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P48.logic.7 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5433+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P48.logic.8 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6233+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P48.logic.9 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7033+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p49"></a>
## P49 · 쿠크세이튼_우측이동화염파동

`KOUKUSAYDON_G1_PATTERN_49` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=12604 / stage 합=8867ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":5,"COLLIDER":14}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":14}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P49.stage.1 | rpct00_att_battle_34_03 / P49.animation.1 | 4219964/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–5867 | P49.stage.2 | rpct00_att_battle_10_02 / P49.animation.2 | 4219964/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 5867–8867 | P49.stage.3 | rpct00_idle_battle_1 / P49.animation.3 | 4219964/stage-002/animation-000 | 0/0/3000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P49.logic.1 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.2 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.3 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.4 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5304+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.5 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5304+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.6 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5304+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.7 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5604+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.8 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5604+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.9 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5604+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.10 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5604+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.11 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.12 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.13 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5304+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P49.logic.14 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5604+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p50"></a>
## P50 · 쿠크세이튼_분신소환_기분나빠_전방

`KOUKUSAYDON_G1_PATTERN_50` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=9267 / stage 합=9267ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":4,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 7, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P50.stage.1 | rpct00_att_battle_34_01 / P50.animation.1 | 4219920/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1600–3267 | P50.stage.3 | rpct00_att_battle_9_01 / P50.animation.3 | 4219951/stage-001/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 3267–4267 | P50.stage.4 | rpct00_att_battle_9_02 / P50.animation.4 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_14 (ACTIVE) | 4267–5267 | P50.stage.14 | rpct00_att_battle_9_02 / P50.animation.14 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_15 (ACTIVE) | 5267–6267 | P50.stage.15 | rpct00_att_battle_9_02 / P50.animation.15 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_16 (ACTIVE) | 6267–7267 | P50.stage.16 | rpct00_att_battle_9_02 / P50.animation.16 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_5 (ACTIVE) | 7267–9267 | P50.stage.5 | rpct00_att_battle_9_03 / P50.animation.5 | 4219951/stage-003/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P50.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3203+4279 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P50.logic.2 | L548 트리거_가장가까운플레이어방향회전 | TRIGGER/BOSS_TRACK_TARGET | 3203+34 / True | — |
상위 참조: P52 via summon.directionPatternIds.

<a id="p51"></a>
## P51 · 쿠크세이튼_분신소환_기분나빠_전방

`KOUKUSAYDON_G1_PATTERN_51` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=0ms / category=MECHANIC / 투영=아니오.

미투영 근거: `PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_51`.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
Bundle 소속: koukusaydon.bundle.11 @0ms.

<a id="p52"></a>
## P52 · 쿠크세이튼_분신소환_기분나빠

`KOUKUSAYDON_G1_PATTERN_52` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=9416 / stage 합=0ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 1, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1,"CROSS_DIRECTION_CLONES":1}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P52.logic.1 | L537 기분나빠 분신 전 최근접 플레이어 조준 | TRIGGER/BOSS_TRACK_TARGET | 0+34 / True | — |

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_1 / P52_ACTION_1 | 0–9416 | P52.idle.1 / rpct00_idle_battle_1 | 0/0/9416/1.0/LOOP_TO_WINDOW |

의존 패턴: P50 ← summon.directionPatternIds (P52.summon.1); P53 ← summon.directionPatternIds (P52.summon.1); P54 ← summon.directionPatternIds (P52.summon.1); P55 ← summon.directionPatternIds (P52.summon.1).
상위 참조: P92 via patternOccurrence.

<a id="p53"></a>
## P53 · 쿠크세이튼_분신소환_기분나빠_후방

`KOUKUSAYDON_G1_PATTERN_53` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=9267 / stage 합=9267ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":4,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 7, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P53.stage.1 | rpct00_att_battle_34_02 / P53.animation.1 | 4219921/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1600–3267 | P53.stage.3 | rpct00_att_battle_9_01 / P53.animation.3 | 4219951/stage-001/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 3267–4267 | P53.stage.4 | rpct00_att_battle_9_02 / P53.animation.4 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_14 (ACTIVE) | 4267–5267 | P53.stage.14 | rpct00_att_battle_9_02 / P53.animation.14 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_15 (ACTIVE) | 5267–6267 | P53.stage.15 | rpct00_att_battle_9_02 / P53.animation.15 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_16 (ACTIVE) | 6267–7267 | P53.stage.16 | rpct00_att_battle_9_02 / P53.animation.16 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_5 (ACTIVE) | 7267–9267 | P53.stage.5 | rpct00_att_battle_9_03 / P53.animation.5 | 4219951/stage-003/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P53.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3228+4223 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P52 via summon.directionPatternIds.

<a id="p54"></a>
## P54 · 쿠크세이튼_분신소환_기분나빠_왼쪽

`KOUKUSAYDON_G1_PATTERN_54` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=9267 / stage 합=9267ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":4,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 7, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P54.stage.1 | rpct00_att_battle_34_04 / P54.animation.1 | 4219922/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1600–3267 | P54.stage.3 | rpct00_att_battle_9_01 / P54.animation.3 | 4219951/stage-001/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 3267–4267 | P54.stage.4 | rpct00_att_battle_9_02 / P54.animation.4 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_14 (ACTIVE) | 4267–5267 | P54.stage.14 | rpct00_att_battle_9_02 / P54.animation.14 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_15 (ACTIVE) | 5267–6267 | P54.stage.15 | rpct00_att_battle_9_02 / P54.animation.15 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_16 (ACTIVE) | 6267–7267 | P54.stage.16 | rpct00_att_battle_9_02 / P54.animation.16 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_5 (ACTIVE) | 7267–9267 | P54.stage.5 | rpct00_att_battle_9_03 / P54.animation.5 | 4219951/stage-003/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P54.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3252+4272 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P52 via summon.directionPatternIds.

<a id="p55"></a>
## P55 · 쿠크세이튼_분신소환_기분나빠_오른쪽

`KOUKUSAYDON_G1_PATTERN_55` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=9267 / stage 합=9267ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":4,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 7, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P55.stage.1 | rpct00_att_battle_34_03 / P55.animation.1 | 4219923/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1600–3267 | P55.stage.3 | rpct00_att_battle_9_01 / P55.animation.3 | 4219951/stage-001/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 3267–4267 | P55.stage.4 | rpct00_att_battle_9_02 / P55.animation.4 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_14 (ACTIVE) | 4267–5267 | P55.stage.14 | rpct00_att_battle_9_02 / P55.animation.14 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_15 (ACTIVE) | 5267–6267 | P55.stage.15 | rpct00_att_battle_9_02 / P55.animation.15 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_16 (ACTIVE) | 6267–7267 | P55.stage.16 | rpct00_att_battle_9_02 / P55.animation.16 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_5 (ACTIVE) | 7267–9267 | P55.stage.5 | rpct00_att_battle_9_03 / P55.animation.5 | 4219951/stage-003/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P55.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3251+4172 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P52 via summon.directionPatternIds.

<a id="p56"></a>
## P56 · 세이튼_마리오_1페이즈

`KOUKUSAYDON_G1_PATTERN_56` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=15000 / stage 합=0ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_1 / P56_ACTION_1 | 0–15000 | P56.idle.1 / rpct00_idle_battle_1 | 0/0/15000/1.0/LOOP_TO_WINDOW |

<a id="p57"></a>
## P57 · 세이튼_마리오_2페이즈

`KOUKUSAYDON_G1_PATTERN_57` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=15000 / stage 합=0ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_1 / P57_ACTION_1 | 0–15000 | P57.idle.1 / rpct00_idle_battle_1 | 0/0/15000/1.0/LOOP_TO_WINDOW |

<a id="p58"></a>
## P58 · 세이튼_화염 파동

`KOUKUSAYDON_G1_PATTERN_58` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=11700 / stage 합=11534ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":2,"SOUND":4,"COLLIDER":56}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":36}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4267 | P58.stage.1 | rpct00_att_battle_10_02 / P58.animation.1 | 4219820/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 4267–8534 | P58.stage.3 | rpct00_att_battle_10_02 / P58.animation.3 | 4219820/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 3/STAGE_2 (RECOVERY) | 8534–11534 | P58.stage.2 | rpct00_idle_battle_1 / P58.animation.2 | 4219820/stage-003/animation-000 | 0/0/3000/1/LOOP_TO_WINDOW |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P58.logic.1 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3104+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P58.logic.2 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P58.logic.3 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P58.logic.4 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P58.logic.5 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7317+500 / True | S: L519:MAX_HP_PERCENT_DAMAGE |
| P58.logic.6 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7617+500 / True | S: L519:MAX_HP_PERCENT_DAMAGE |
| P58.logic.7 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7917+500 / True | S: L519:MAX_HP_PERCENT_DAMAGE |
| P58.logic.8 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 8217+500 / True | S: L519:MAX_HP_PERCENT_DAMAGE |
| P58.logic.9 | L120 트리거_플레이어타겟설정 | TRIGGER/None | 0+340 / True | — |
| P58.logic.10 | L120 트리거_플레이어타겟설정 | TRIGGER/None | 4237+340 / True | — |
| P58.logic.11 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3104+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.12 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.13 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.14 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.15 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.16 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.17 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.18 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.19 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.20 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.21 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3104+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.22 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.23 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.24 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.25 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7317+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.26 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7617+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.27 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7617+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.28 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7917+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.29 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7917+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.30 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7917+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.31 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8217+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.32 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8217+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.33 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8217+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.34 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8217+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.35 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7317+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.36 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7617+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.37 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7917+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P58.logic.38 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8217+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p59"></a>
## P59 · 쿠크세이튼_화염 파동

`KOUKUSAYDON_G1_PATTERN_59` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=15247 / stage 합=11534ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":2,"SOUND":5,"COLLIDER":28}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":28}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_4 (ACTIVE) | 0–4267 | P59.stage.4 | rpct00_att_battle_10_02 / P59.animation.3 | 4219820/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 2/STAGE_5 (ACTIVE) | 4267–8534 | P59.stage.5 | rpct00_att_battle_10_02 / P59.animation.4 | 4219820/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 3/STAGE_6 (RECOVERY) | 8534–11534 | P59.stage.6 | rpct00_idle_battle_1 / P59.animation.5 | 4219820/stage-003/animation-000 | 0/0/3000/1/LOOP_TO_WINDOW |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P59.logic.1 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3104+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.2 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.3 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.4 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.5 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.6 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.7 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.8 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.9 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.10 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.11 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3104+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.12 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.13 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.14 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.15 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7347+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.16 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7647+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.17 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7647+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.18 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.19 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.20 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.21 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.22 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.23 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.24 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.25 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7347+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.26 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7647+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.27 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
| P59.logic.28 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L556:MAX_HP_PERCENT_DAMAGE |
상위 참조: P88 via patternOccurrence; P91 via patternOccurrence.

<a id="p60"></a>
## P60 · 기분나빠 | 브레스

`KOUKUSAYDON_G1_PATTERN_60` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=5167ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":5,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–5167 | P60.stage.1 | rpct00_att_battle_31_01 / P60.animation.1 | 4219917/stage-000/animation-000 | 0/0/5167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P60.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 1989+3000 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |

<a id="p61"></a>
## P61 · 저주의식 | 왼손 트레일

`KOUKUSAYDON_G1_PATTERN_61` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=4667ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4667 | P61.stage.1 | rpct00_att_battle_27_01 / P61.animation.1 | 4219911/stage-000/animation-000 | 0/0/4667/1/EXACT |

<a id="p62"></a>
## P62 · 세이튼_눈 레이저

`KOUKUSAYDON_G1_PATTERN_62` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=3582 / stage 합=3507ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":4,"COLLIDER":2,"EFFECT":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1000 | P62.stage.1 | rpct00_att_battle_35_01 / P62.animation.1 | 4219983/stage-000/animation-000 | 0/0/1000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1000–3507 | P62.stage.2 | rpct00_att_battle_35_04 / P62.animation.2 | 4219983/stage-001/animation-000 | 0/0/2500/1/HOLD_LAST_POSE |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P62.logic.1 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 2050+1000 / True | S: L134:MAX_HP_PERCENT_DAMAGE |
| P62.logic.2 | L548 트리거_가장가까운플레이어방향회전 | TRIGGER/BOSS_TRACK_TARGET | 0+74 / True | — |

<a id="p63"></a>
## P63 · 쿠크세이튼_분신소환_십자화염폭발_왼쪽

`KOUKUSAYDON_G1_PATTERN_63` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6575 / stage 합=6575ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"SOUND":5,"COLLIDER":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_18 (ACTIVE) | 0–1600 | P63.stage.18 | rpct00_att_battle_34_04 / P63.animation.18 | 4219922/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–6575 | P63.stage.2 | rpct00_att_battle_10_02 / P63.animation.2 | 4219948/stage-001/animation-000 | 0/0/4267/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P63.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 4713+1862 / True | S: L515:FIXED_DAMAGE |
상위 참조: P40 via summon.patternSpawns; P65 via summon.directionPatternIds.

<a id="p64"></a>
## P64 · 쿠크세이튼_분신소환_십자화염폭발_오른쪽

`KOUKUSAYDON_G1_PATTERN_64` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6525 / stage 합=1601ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":2}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_18 (ACTIVE) | 0–1600 | P64.stage.18 | rpct00_att_battle_34_03 / P64.animation.18 | 4219923/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–1601 | P64.stage.2 | — / — | —/—/— | —/—/—/—/— |

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_18 / P64.stage.18 | 0–1601 | P64.animation.18 / rpct00_att_battle_34_03 | 0/0/1600/1/EXACT |
상위 참조: P40 via summon.patternSpawns; P65 via summon.directionPatternIds.

<a id="p65"></a>
## P65 · 쿠크세이튼_분신소환_십자화염폭발

`KOUKUSAYDON_G1_PATTERN_65` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6692 / stage 합=0ms / category=MECHANIC / 투영=아니오.

미투영 근거: `Cross direction child needs its clone end Stage before the full ending and must fit the owning window duration`.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 1, Scene 0.

의존 패턴: P71 ← summon.directionPatternIds (P65.summon.1); P72 ← summon.directionPatternIds (P65.summon.1); P63 ← summon.directionPatternIds (P65.summon.1); P64 ← summon.directionPatternIds (P65.summon.1).

<a id="p66"></a>
## P66 · 쿠크세이튼_분신소환_3갈레불뿜기

`KOUKUSAYDON_G1_PATTERN_66` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=30000 / stage 합=0ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 1, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"CROSS_DIRECTION_CLONES":1}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_1 / P66_ACTION_1 | 0–30000 | P66.idle.1 / rpct00_idle_battle_1 | 0/0/30000/1.0/LOOP_TO_WINDOW |

의존 패턴: P67 ← summon.directionPatternIds (P66.summon.1); P68 ← summon.directionPatternIds (P66.summon.1); P69 ← summon.directionPatternIds (P66.summon.1); P70 ← summon.directionPatternIds (P66.summon.1).

<a id="p67"></a>
## P67 · 쿠크세이튼_분신소환_3갈레불뿜기_전방

`KOUKUSAYDON_G1_PATTERN_67` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6434 / stage 합=6434ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":5,"COLLIDER":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P67.stage.1 | rpct00_att_battle_34_01 / P67.animation.1 | 4219920/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_7 (ACTIVE) | 1600–3267 | P67.stage.7 | rpct00_att_battle_14_01 / P67.animation.7 | 4219940/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_8 (ACTIVE) | 3267–6434 | P67.stage.8 | rpct00_att_battle_14_02 / P67.animation.8 | 4219940/stage-003/animation-000 | 0/0/3167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P67.logic.1 | L542 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1452+4982 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P66 via summon.directionPatternIds.

<a id="p68"></a>
## P68 · 쿠크세이튼_분신소환_3갈레불뿜기_후방

`KOUKUSAYDON_G1_PATTERN_68` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6434 / stage 합=6434ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":5,"COLLIDER":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P68.stage.1 | rpct00_att_battle_34_02 / P68.animation.1 | 4219921/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_11 (ACTIVE) | 1600–3267 | P68.stage.11 | rpct00_att_battle_14_01 / P68.animation.7 | 4219940/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_12 (ACTIVE) | 3267–6434 | P68.stage.12 | rpct00_att_battle_14_02 / P68.animation.8 | 4219940/stage-003/animation-000 | 0/0/3167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P68.logic.1 | L543 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1600+4834 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P66 via summon.directionPatternIds.

<a id="p69"></a>
## P69 · 쿠크세이튼_분신소환_3갈레불뿜기_왼쪽

`KOUKUSAYDON_G1_PATTERN_69` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6434 / stage 합=6434ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":5,"COLLIDER":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P69.stage.1 | rpct00_att_battle_34_04 / P69.animation.1 | 4219922/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_5 (ACTIVE) | 1600–3267 | P69.stage.5 | rpct00_att_battle_14_01 / P69.animation.5 | 4219940/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_6 (ACTIVE) | 3267–6434 | P69.stage.6 | rpct00_att_battle_14_02 / P69.animation.6 | 4219940/stage-003/animation-000 | 0/0/3167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P69.logic.1 | L543 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1600+4834 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P66 via summon.directionPatternIds.

<a id="p70"></a>
## P70 · 쿠크세이튼_분신소환_3갈레불뿜기_오른쪽

`KOUKUSAYDON_G1_PATTERN_70` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6434 / stage 합=6434ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":5,"COLLIDER":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P70.stage.1 | rpct00_att_battle_34_03 / P70.animation.1 | 4219923/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_5 (ACTIVE) | 1600–3267 | P70.stage.5 | rpct00_att_battle_14_01 / P70.animation.5 | 4219940/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_6 (ACTIVE) | 3267–6434 | P70.stage.6 | rpct00_att_battle_14_02 / P70.animation.6 | 4219940/stage-003/animation-000 | 0/0/3167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P70.logic.1 | L543 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1600+4834 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
상위 참조: P66 via summon.directionPatternIds.

<a id="p71"></a>
## P71 · 쿠크세이튼_분신소환_십자화염폭발_전방

`KOUKUSAYDON_G1_PATTERN_71` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6575 / stage 합=1601ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":2}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_18 (ACTIVE) | 0–1600 | P71.stage.18 | rpct00_att_battle_34_01 / P71.animation.18 | 4219920/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–1601 | P71.stage.2 | — / — | —/—/— | —/—/—/—/— |

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_18 / P71.stage.18 | 0–1601 | P71.animation.18 / rpct00_att_battle_34_01 | 0/0/1600/1/EXACT |
상위 참조: P65 via summon.directionPatternIds.

<a id="p72"></a>
## P72 · 쿠크세이튼_분신소환_십자화염폭발_후방

`KOUKUSAYDON_G1_PATTERN_72` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=6575 / stage 합=1601ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":2}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_18 (ACTIVE) | 0–1600 | P72.stage.18 | rpct00_att_battle_34_02 / P72.animation.18 | 4219921/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–1601 | P72.stage.2 | — / — | —/—/— | —/—/—/—/— |

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_18 / P72.stage.18 | 0–1601 | P72.animation.18 / rpct00_att_battle_34_02 | 0/0/1600/1/EXACT |
상위 참조: P65 via summon.directionPatternIds.

<a id="p73"></a>
## P73 · 2관문_진입컷씬

`KOUKUSAYDON_G1_PATTERN_73` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=27000 / stage 합=27000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"CAMERA":7,"SOUND":3,"SUBTITLE":2}`; World 31, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 31.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–27000 | P73.stage.1 | — / — | —/—/— | —/—/—/—/— |

<a id="p74"></a>
## P74 · 2관문클리어_3관문진입

`KOUKUSAYDON_G1_PATTERN_74` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=35368 / stage 합=35368ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"CAMERA":18,"EFFECT":3,"SOUND":3,"SUBTITLE":6}`; World 3, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 3.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–35368 | P74.stage.1 | — / — | —/—/— | —/—/—/—/— |

<a id="p75"></a>
## P75 · 빙고_최종엔딩씬

`KOUKUSAYDON_G1_PATTERN_75` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=55275 / stage 합=55275ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"CAMERA":12,"SOUND":2,"SUBTITLE":5,"EFFECT":2}`; World 7, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 7.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–55275 | P75.stage.1 | — / — | —/—/— | —/—/—/—/— |

<a id="p76"></a>
## P76 · 쇼타임_연출

`KOUKUSAYDON_G1_PATTERN_76` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=7474 / stage 합=5000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"resetBossYawDegrees":234.4921875}`.
표현 수: `{"CAMERA":4,"EFFECT":1,"SUBTITLE":1,"SOUND":1}`; World 3, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 2, rootMotion 보유 2; worldSequences 3.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4430 | P76.stage.1 | rpct00_evt2_rpct_showtime_01 / P76.animation.1 | 0/RAW/rpct00_evt2_rpct_showtime_01 | 0/0/4430/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 4430–5000 | P76.stage.2 | rpct00_att_battle_28_05_start / P76.animation.2 | 0/RAW/rpct00_att_battle_28_05_start | 0/0/570/1/EXACT; blend=570 |

<a id="p77"></a>
## P77 · 카드미로연출

`KOUKUSAYDON_G1_PATTERN_77` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=12023 / stage 합=11950ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootVerticalScale":0,"bossMotion":{"startMs":0,"endMs":11950,"startPosition":[6.8837676,10.5405245,320.769958],"endPosition":[6.36000013,10.5600004,321.290009],"yawDegrees":-134.9999994235791,"keys":[{"timeMs":0,"position":[6.8837676,10.5405245,320.769958]},{"timeMs":367,"position":[6.883764734018988,10.5405245,320.769960865982]},{"timeMs":400,"position":[6.883481001898874,10.5405245,320.7702445982035]},{"timeMs":433,"position":[6.88266259909413,10.5405245,320.77106300130066]},{"timeMs":467,"position":[6.881455609000053,10.5405245,320.77226999182597]},{"timeMs":500,"position":[6.880033852151479,10.5405245,320.77369174918255]},{"timeMs":533,"position":[6.87838320628617,10.5405245,320.7753423956376]},{"timeMs":566,"position":[6.8764491389090585,10.54053415769594,320.77727646370346]},{"timeMs":600,"position":[6.874034749085823,10.540545882995696,320.77969085438656]},{"timeMs":633,"position":[6.871077577569849,10.540550984929093,320.7826480269579]},{"timeMs":667,"position":[6.867151506729175,10.54055646548334,320.78657409920004]},{"timeMs":700,"position":[6.862204792778762,10.540561970155826,320.79152081491657]},{"timeMs":733,"position":[6.8554517054457,10.540542107190403,320.7982739046672]},{"timeMs":766,"position":[6.845975637449561,10.540524848131088,320.80774997605323]},{"timeMs":800,"position":[6.832943177916749,10.540524500000002,320.82078244024257]},{"timeMs":833,"position":[6.817100565914839,10.540524500000004,320.836625057905]},{"timeMs":866,"position":[6.798539981168742,10.540524500000004,320.8551856492827]},{"timeMs":900,"position":[6.777318563117904,10.540524500000004,320.87640707491585]},{"timeMs":933,"position":[6.75534301096771,10.540524500000005,320.8983826349178]},{"timeMs":967,"position":[6.730922931489566,10.540524500000005,320.92280272312115]},{"timeMs":1000,"position":[6.70402571846027,10.540524500000007,320.9496999457607]},{"timeMs":1033,"position":[6.671985938735949,10.540524500000009,320.9817397369327]},{"timeMs":1034,"position":[6.670673719734037,10.540524500000009,320.9830519564035]},{"timeMs":1066,"position":[6.62322164629651,10.54052450000001,321.0305040467954]},{"timeMs":1067,"position":[6.621650099591709,10.54052450000001,321.0320755940617]},{"timeMs":1100,"position":[6.563936877281333,10.540524500000013,321.0897888369928]},{"timeMs":1133,"position":[6.533648841701359,10.540524500000014,321.1200768833946]},{"timeMs":1134,"position":[6.533288695232261,10.540524500000014,321.12043702999233]},{"timeMs":1166,"position":[6.530686772875583,10.540524500000014,321.1230389532787]},{"timeMs":1167,"position":[6.530647959450804,10.540524500000014,321.1230777667173]},{"timeMs":1200,"position":[6.53217189525827,10.540524500000014,321.1215538303654]},{"timeMs":1334,"position":[6.530259941534052,10.540524500000014,321.1234657847727]},{"timeMs":1400,"position":[6.529528919722993,10.540524500000014,321.12419680684496]},{"timeMs":1500,"position":[6.528961285132998,10.540524500000014,321.12476444163775]},{"timeMs":1534,"position":[6.5294151713187905,10.540524500000014,321.12431055528975]},{"timeMs":1567,"position":[6.531366204403617,10.540524500000014,321.12235952150786]},{"timeMs":1600,"position":[6.536409520642709,10.540524500000014,321.1173162034668]},{"timeMs":1633,"position":[6.5447584529034755,10.540524500000014,321.108967268223]},{"timeMs":1667,"position":[6.553864516693262,10.540524500000013,321.0998612011797]},{"timeMs":1700,"position":[6.559369308576508,10.540524500000013,321.0943564073296]},{"timeMs":1733,"position":[6.556523030806339,10.540524500000013,321.0972026861167]},{"timeMs":1734,"position":[6.556266050679384,10.540524500000013,321.0974596663355]},{"timeMs":1767,"position":[6.544959193061182,10.540524500000014,321.10876652799357]},{"timeMs":1800,"position":[6.532723273980288,10.540524500000014,321.1210024514463]},{"timeMs":1833,"position":[6.529648194614316,10.540524500000014,321.124077531911]},{"timeMs":1834,"position":[6.529835705035463,10.540524500000014,321.1238900214229]},{"timeMs":1866,"position":[6.540327152821959,10.540524500000014,321.1133985698878]},{"timeMs":1867,"position":[6.540758698111196,10.540524500000014,321.1129670244444]},{"timeMs":1900,"position":[6.561843070686029,10.540524500000013,321.0918826443362]},{"timeMs":1933,"position":[6.589345907633788,10.540524500000013,321.0643797975618]},{"timeMs":1967,"position":[6.622354082052369,10.54052450000001,321.0313716113495]},{"timeMs":2000,"position":[6.657097097687563,10.540524500000009,320.99662858330083]},{"timeMs":2033,"position":[6.695833014234707,10.540524500000007,320.9578926529135]},{"timeMs":2066,"position":[6.74078799712217,10.540524500000005,320.9129376539638]},{"timeMs":2067,"position":[6.742200711296917,10.540524503241329,320.9115249392843]},{"timeMs":2100,"position":[6.792022529100705,10.540524824132257,320.8617031036793]},{"timeMs":2133,"position":[6.841640691104375,10.540525934731646,320.812084923947]},{"timeMs":2134,"position":[6.8428924866322225,10.540526254161517,320.8108331279718]},{"timeMs":2166,"position":[6.878921381300481,10.54054104832286,320.7748042204271]},{"timeMs":2167,"position":[6.879826644442275,10.540541496529913,320.7738989569617]},{"timeMs":2200,"position":[6.895138100226652,10.54055535607045,320.75858749570335]},{"timeMs":2233,"position":[6.884467730983456,10.54057710679004,320.7692578687538]},{"timeMs":2234,"position":[6.883787243291276,10.54057735758536,320.76993835668907]},{"timeMs":2267,"position":[6.855410955012415,10.540578681159218,320.79831465510637]},{"timeMs":2300,"position":[6.82435611725182,10.540558892136337,320.8293695039674]},{"timeMs":2333,"position":[6.800605234202688,10.540543434659197,320.8531203955063]},{"timeMs":2334,"position":[6.800048558099729,10.540543115534236,320.85367707180825]},{"timeMs":2367,"position":[6.78441933192478,10.5405350749429,320.8693063035694]},{"timeMs":2400,"position":[6.77376844664929,10.54052971520904,320.87995719265166]},{"timeMs":2433,"position":[6.767524204922868,10.540526060464945,320.88620143661]},{"timeMs":2467,"position":[6.766112794392717,10.540524513223382,320.8876128476448]},{"timeMs":2500,"position":[6.7683983460627815,10.540524500000005,320.8853272951581]},{"timeMs":2533,"position":[6.771883515639235,10.540524500000005,320.88184212433646]},{"timeMs":2566,"position":[6.776941772163983,10.540524500000004,320.8767838660044]},{"timeMs":2600,"position":[6.7841939062803185,10.540524500000004,320.8695317292969]},{"timeMs":2633,"position":[6.7936908698474365,10.540524500000004,320.86003476233657]},{"timeMs":2666,"position":[6.810027981225809,10.540524500000004,320.84369764512104]},{"timeMs":2667,"position":[6.810627474396069,10.540524500000004,320.84309815173657]},{"timeMs":2700,"position":[6.837163480641876,10.540524500000002,320.81656213600957]},{"timeMs":2733,"position":[6.868318555833258,10.5405245,320.78540704968657]},{"timeMs":2766,"position":[6.894585459884683,10.5405245,320.7591401362501]},{"timeMs":2767,"position":[6.895233495561148,10.5405245,320.7584921003421]},{"timeMs":2800,"position":[6.906954978022941,10.540524499999998,320.74677061369226]},{"timeMs":2833,"position":[6.888419056413044,10.5405245,320.765306541925]},{"timeMs":2834,"position":[6.887146296657417,10.5405245,320.76657930213537]},{"timeMs":2866,"position":[6.835040941895164,10.540524500000002,320.81868467551465]},{"timeMs":2867,"position":[6.833246141735,10.540524500000002,320.82047947631605]},{"timeMs":2900,"position":[6.7630282200758565,10.540524500000005,320.8906974230638]},{"timeMs":2933,"position":[6.68018833696954,10.548629910410167,320.97353733383596]},{"timeMs":2934,"position":[6.678916494730047,10.551990892171444,320.9748091757285]},{"timeMs":2966,"position":[6.658032859043572,10.709388120756302,320.9956927813502]},{"timeMs":2967,"position":[6.657970347226884,10.71580847904826,320.9957552916585]},{"timeMs":3000,"position":[6.694854175978174,11.026792165995797,320.95887137558447]},{"timeMs":3033,"position":[6.737997921519618,11.26032782124508,320.91572755894873]},{"timeMs":3034,"position":[6.739320407880238,11.265653755429769,320.91440507084576]},{"timeMs":3066,"position":[6.7818815735335285,11.408069187689101,320.8718438560311]},{"timeMs":3067,"position":[6.78318718904033,11.412273861474132,320.8705382390553]},{"timeMs":3100,"position":[6.824660720053706,11.534804737157405,320.82906466401]},{"timeMs":3133,"position":[6.853285546670699,11.587726906856512,320.8004398145478]},{"timeMs":3134,"position":[6.853868302541973,11.588607010159258,320.7998570582585]},{"timeMs":3167,"position":[6.868351469810339,11.605738638134214,320.78537388173083]},{"timeMs":3200,"position":[6.877852802059311,11.62557500129503,320.7758725413577]},{"timeMs":3233,"position":[6.887057758956173,11.64153487329509,320.7666675773669]},{"timeMs":3267,"position":[6.8962390334301436,11.654669184246565,320.757486296481]},{"timeMs":3300,"position":[6.904859328955297,11.667418276910041,320.74886599483625]},{"timeMs":3333,"position":[6.913191589418882,11.681730867760566,320.7405337279832]},{"timeMs":3367,"position":[6.921481730560971,11.699659472706843,320.7322435796045]},{"timeMs":3400,"position":[6.92924433265163,11.720687004845356,320.724480969727]},{"timeMs":3434,"position":[6.936947720234162,11.744917149381815,320.71677757361516]},{"timeMs":3467,"position":[6.944148948781745,11.769425755777593,320.7095763366513]},{"timeMs":3500,"position":[6.951076688055403,11.793762786057385,320.70264858909997]},{"timeMs":3533,"position":[6.957731180703242,11.816723869654437,320.6959940886002]},{"timeMs":3567,"position":[6.964308940682813,11.83770897940322,320.6894163212671]},{"timeMs":3600,"position":[6.970425062599043,11.854327877659987,320.6833001932034]},{"timeMs":3633,"position":[6.97627535003292,11.866046254964926,320.67744990088534]},{"timeMs":3666,"position":[6.981865755374781,11.8716965968637,320.6718594921989]},{"timeMs":3667,"position":[6.982032448430177,11.87179993794876,320.6716927990593]},{"timeMs":3700,"position":[6.987359553946771,11.870853859029406,320.66636569186494]},{"timeMs":3734,"position":[6.99257680245861,11.863846637174793,320.6611484431597]},{"timeMs":3767,"position":[6.9973876231621475,11.852528320606147,320.6563376234357]},{"timeMs":3800,"position":[7.001948431041239,11.83791253215878,320.65177681741176]},{"timeMs":3833,"position":[7.006258711605595,11.821211131154573,320.64746653928927]},{"timeMs":3867,"position":[7.010444428318917,11.803114459548254,320.643280825395]},{"timeMs":3900,"position":[7.01426190164099,11.785881058069176,320.63946335481774]},{"timeMs":3933,"position":[7.017836579692807,11.768975676134229,320.63588867951927]},{"timeMs":3967,"position":[7.021271927892803,11.75032025459903,320.6324533345396]},{"timeMs":4000,"position":[7.024368283133608,11.729083620669593,320.6293569832557]},{"timeMs":4033,"position":[7.027229902978242,11.702751446362914,320.6264953686667]},{"timeMs":4034,"position":[7.027311924661848,11.701394692848067,320.62641334727726]},{"timeMs":4066,"position":[7.029861513429049,11.649037638950748,320.62386377008204]},{"timeMs":4067,"position":[7.029885411856064,11.646780247684966,320.62383987218465]},{"timeMs":4100,"position":[7.026992829298184,11.531284936035766,320.6267324833123]},{"timeMs":4133,"position":[7.012743063845946,11.278610741510585,320.64098231409815]},{"timeMs":4134,"position":[7.0121411116486145,11.2696353499469,320.64158426865043]},{"timeMs":4166,"position":[6.99015638336567,10.961325255265944,320.6635690782953]},{"timeMs":4167,"position":[6.989457522295238,10.952019675982475,320.66426794183405]},{"timeMs":4200,"position":[6.965613778406095,10.666656897198703,320.68811176227825]},{"timeMs":4233,"position":[6.947553390371509,10.549181112320301,320.7061721847742]},{"timeMs":4234,"position":[6.9473317120531215,10.547845088466008,320.7063938634903]},{"timeMs":4266,"position":[6.945447704544827,10.54067389976461,320.7082778733815]},{"timeMs":4267,"position":[6.945532793617592,10.540524499999998,320.70819278431395]},{"timeMs":4300,"position":[6.9578423773072275,10.540524499999997,320.69588319622613]},{"timeMs":4333,"position":[6.965727595948012,10.540524499999997,320.687997974768]},{"timeMs":4334,"position":[6.9656436354058915,10.540524499999997,320.6880819353401]},{"timeMs":4366,"position":[6.957790393073411,10.540524499999997,320.69593518047856]},{"timeMs":4367,"position":[6.957449443641407,10.540524499999997,320.69627613003235]},{"timeMs":4400,"position":[6.9398927621836854,10.540524499999998,320.71383281776303]},{"timeMs":4433,"position":[6.920679334254856,10.540524499999998,320.73304625255673]},{"timeMs":4466,"position":[6.903661910515342,10.5405245,320.7500636823765]},{"timeMs":4467,"position":[6.903196084886635,10.5405245,320.7505295081716]},{"timeMs":4500,"position":[6.891069378711649,10.5405245,320.7626562186794]},{"timeMs":4533,"position":[6.8851317257246825,10.5405245,320.7685938737879]},{"timeMs":4567,"position":[6.8837676,10.5405245,320.769958]},{"timeMs":6866,"position":[6.8837666774285715,10.5405235,320.769928]},{"timeMs":6900,"position":[6.881913108189339,10.5405235,320.7717815424729]},{"timeMs":6933,"position":[6.8755610340214695,10.5405235,320.7781336189104]},{"timeMs":6934,"position":[6.87527590334084,10.5405235,320.77841874969283]},{"timeMs":6967,"position":[6.864291798409053,10.5405235,320.78940285854924]},{"timeMs":7000,"position":[6.848782707669475,10.540523500000003,320.8049119548301]},{"timeMs":7033,"position":[6.828892534082035,10.540523351500003,320.82480213552424]},{"timeMs":7067,"position":[6.804356266866999,10.540523198500004,320.849338411506]},{"timeMs":7100,"position":[6.777364951983446,10.540523050000004,320.8763297360334]},{"timeMs":7133,"position":[6.748301887558454,10.540522901500005,320.9053928108425]},{"timeMs":7134,"position":[6.747761773835447,10.540522897000004,320.90593292475853]},{"timeMs":7166,"position":[6.735927497882634,10.540522753000005,320.91776720493965]},{"timeMs":7167,"position":[6.735293975973735,10.540522748500004,320.9184007270749]},{"timeMs":7200,"position":[6.696983501145395,10.540522600000006,320.9567112155914]},{"timeMs":7233,"position":[6.439991431453948,10.540584270433007,321.2137033770903]},{"timeMs":7234,"position":[6.429693245987852,10.543872915081861,321.22400156545183]},{"timeMs":7240,"position":[6.360372492219489,10.573465110518546,321.2933223369329]},{"timeMs":7266,"position":[6.2441867855982505,10.701697648660845,321.4094964966841]},{"timeMs":7267,"position":[6.239103850193788,10.715095295451379,321.41457898617927]},{"timeMs":7300,"position":[6.030826192728971,11.715948961661399,321.6228418099109]},{"timeMs":7333,"position":[5.8042712808361205,12.147903652682972,321.8493820302371]},{"timeMs":7334,"position":[5.797505198861407,12.15515923012892,321.85614766836835]},{"timeMs":7366,"position":[5.582578069841324,12.29399437006946,322.07106061608357]},{"timeMs":7367,"position":[5.576135676810708,12.300671853551542,322.0775025652929]},{"timeMs":7400,"position":[5.381625970383673,12.675395242150596,322.27199758234866]},{"timeMs":7410,"position":[5.393014245482673,12.732015705265923,322.260614782001]},{"timeMs":7433,"position":[5.395080397043708,12.862242399875617,322.25853361342365]},{"timeMs":7434,"position":[5.396858689581595,12.872685698493399,322.2567546662288]},{"timeMs":7466,"position":[5.480779408575122,13.283371548940007,322.17281297032025]},{"timeMs":7467,"position":[5.484004209082099,13.294848346054982,322.169587514393]},{"timeMs":7500,"position":[5.630172976348659,13.584011693891226,322.02339712541215]},{"timeMs":7533,"position":[5.858700661779331,13.87602764534327,321.7948477881605]},{"timeMs":7534,"position":[5.865293936425119,13.885561481203586,321.7882538573541]},{"timeMs":7566,"position":[6.070969817395949,14.201602110885668,321.58255697853104]},{"timeMs":7567,"position":[6.075980160809166,14.209728606776435,321.57754597985837]},{"timeMs":7590,"position":[6.126034777425548,14.31614841807121,321.52747633475576]},{"timeMs":7600,"position":[6.103647752548613,14.362418062353964,321.54985597939714]},{"timeMs":7633,"position":[5.904788118734744,14.443879254722965,321.7486881786763]},{"timeMs":7634,"position":[5.898379113548858,14.445832103358923,321.7550963527724]},{"timeMs":7666,"position":[5.6871636567785595,14.500072502190912,321.96628521882724]},{"timeMs":7667,"position":[5.680556662942543,14.504592939204196,321.9728913810321]},{"timeMs":7700,"position":[5.462096152765588,14.840245380928462,322.19132440307385]},{"timeMs":7733,"position":[5.28135998878467,15.009239427901345,322.37203310517543]},{"timeMs":7734,"position":[5.276187445325371,15.013134907321186,322.3772048166401]},{"timeMs":7766,"position":[5.11553501454248,15.118181411850337,322.5378306265282]},{"timeMs":7767,"position":[5.110599096230044,15.116468319440564,322.54276571409855]},{"timeMs":7800,"position":[4.953288821788781,14.730213755281202,322.70004865067204]},{"timeMs":7833,"position":[4.859252110795529,14.207879785963016,322.7940904314324]},{"timeMs":7834,"position":[4.856811657527403,14.191266877274359,322.7965310383707]},{"timeMs":7840,"position":[4.843396370022742,14.089235605378086,322.80994724801906]},{"timeMs":7866,"position":[4.773727168866178,12.670245710009397,322.8796504337682]},{"timeMs":7867,"position":[4.7711707921319375,12.610315182622603,322.88220811883457]},{"timeMs":7900,"position":[4.6949420829933945,10.279244244188764,322.95848008427816]},{"timeMs":7933,"position":[4.675060873443854,7.983917336591124,322.9784045214782]},{"timeMs":7934,"position":[4.6778215155103595,7.917825327573864,322.9756451873131]},{"timeMs":7950,"position":[4.748896612270011,6.888059997849899,322.90459100075805]},{"timeMs":7966,"position":[4.767181003575114,6.459436227398663,322.886306705112]},{"timeMs":7967,"position":[4.767075659592003,6.441238597448292,322.8864120534714]},{"timeMs":8000,"position":[4.681223491130445,6.407746285473723,322.97226426059245]},{"timeMs":8033,"position":[4.652054368023532,6.407760124656026,323.00143339411784]},{"timeMs":8034,"position":[4.651393632013852,6.407758464708452,323.002094130364]},{"timeMs":8067,"position":[4.633276402138334,6.407670241605218,323.02021136673426]},{"timeMs":8100,"position":[4.615613995182486,6.407666680000279,323.03787378000123]},{"timeMs":8133,"position":[4.601323136524586,6.407666680000279,323.05216464376514]},{"timeMs":8166,"position":[4.593717617364438,6.407666680000279,323.0597701656431]},{"timeMs":8167,"position":[4.59353838828325,6.407740395536543,323.05994939477074]},{"timeMs":8200,"position":[4.590869312065885,6.415038233626584,323.06261847020164]},{"timeMs":8233,"position":[4.58879971170378,6.421633095855181,323.0646880697308]},{"timeMs":8234,"position":[4.5887451895295595,6.4215745286322825,323.06474259193845]},{"timeMs":8267,"position":[4.587085027240631,6.415365747301161,323.06640275630093]},{"timeMs":8300,"position":[4.585805610034697,6.4079542429997725,323.06768217573074]},{"timeMs":8334,"position":[4.584863159867185,6.40766668000028,323.06862462630386]},{"timeMs":8367,"position":[4.58428211012216,6.40766668000028,323.06920567625656]},{"timeMs":8400,"position":[4.584008523863895,6.40766668000028,323.0694792626122]},{"timeMs":8434,"position":[4.584027965882098,6.40766668000028,323.06945982058727]},{"timeMs":8467,"position":[4.584308786001374,6.40766668000028,323.0691790003675]},{"timeMs":8500,"position":[4.584825381229974,6.40766668000028,323.0686624049545]},{"timeMs":8534,"position":[4.585583701215237,6.40766668000028,323.0679040846981]},{"timeMs":8567,"position":[4.586509960572968,6.40766668000028,323.06697782501]},{"timeMs":8600,"position":[4.58760269627435,6.40766668000028,323.0658850889181]},{"timeMs":8667,"position":[4.590236163142709,6.40766668000028,323.0632516211085]},{"timeMs":8733,"position":[4.593209402035821,6.40766668000028,323.0602783811532]},{"timeMs":8767,"position":[4.598407524683535,6.407666680000279,323.05508025664795]},{"timeMs":8800,"position":[4.608019253662573,6.407666680000279,323.04546852423465]},{"timeMs":8833,"position":[4.618455764743356,6.407666680000278,323.0350320094252]},{"timeMs":8867,"position":[4.625990294528343,6.407666680000278,323.0274974769484]},{"timeMs":8900,"position":[4.62946805866777,6.407666680000278,323.02401971156604]},{"timeMs":8933,"position":[4.631313156578926,6.407666680000278,323.02217461299557]},{"timeMs":8967,"position":[4.6320653562301475,6.407666680000278,323.0214224130759]},{"timeMs":9000,"position":[4.632107198834186,6.407666680000278,323.02138057045664]},{"timeMs":9033,"position":[4.63191890711539,6.407666680000278,323.02156886224276]},{"timeMs":9066,"position":[4.631430320747401,6.407666680000278,323.0220574487853]},{"timeMs":9100,"position":[4.630171124957996,6.407666680000278,323.02331664502486]},{"timeMs":9133,"position":[4.628054050119585,6.407666680000278,323.02543372061956]},{"timeMs":9167,"position":[4.624817271094843,6.407666680000278,323.0286705008008]},{"timeMs":9200,"position":[4.618864355984869,6.407666680000278,323.0346234180374]},{"timeMs":9233,"position":[4.609918890011014,6.407666680000279,323.04356888720787]},{"timeMs":9267,"position":[4.600178715291074,6.407666680000279,323.05330906540786]},{"timeMs":9300,"position":[4.592742801729123,6.40766668000028,323.06074498162627]},{"timeMs":9333,"position":[4.589918425947091,6.40766668000028,323.0635693584175]},{"timeMs":9366,"position":[4.594199961479044,6.407666680000279,323.059287821356]},{"timeMs":9367,"position":[4.594415644335394,6.407666680000279,323.0590721384226]},{"timeMs":9400,"position":[4.607060157451824,6.407666680000279,323.0464276207881]},{"timeMs":9433,"position":[4.626263498773437,6.407666680000278,323.0272242726055]},{"timeMs":9466,"position":[4.650194290341259,6.407666680000277,323.0032934724872]},{"timeMs":9500,"position":[4.677929239494069,6.407666680000276,322.9755585134251]},{"timeMs":9533,"position":[4.706129545379247,6.407666680000275,322.94735819746376]},{"timeMs":9567,"position":[4.734631905611662,6.407666680000274,322.918855827048]},{"timeMs":9600,"position":[4.763576124719152,6.407666680000273,322.8899115975985]},{"timeMs":9633,"position":[4.795763547983393,6.407666680000271,322.8577241628338]},{"timeMs":9667,"position":[4.830490296320768,6.40766668000027,322.82299740208884]},{"timeMs":9700,"position":[4.863897994762793,6.407666680000268,322.7895896917107]},{"timeMs":9733,"position":[4.8951383267745205,6.4076666800002675,322.75834934853646]},{"timeMs":9734,"position":[4.896003699778329,6.407667756143086,322.7574839752232]},{"timeMs":9767,"position":[4.923160179798683,6.407720988289555,322.7303274854873]},{"timeMs":9800,"position":[4.944503528918477,6.407770601979409,322.70898412873004]},{"timeMs":9833,"position":[4.9563461929045,6.407726405844543,322.6971414605233]},{"timeMs":9834,"position":[4.956513908758407,6.407726402382043,322.6969737446095]},{"timeMs":9867,"position":[4.9588375347165545,6.407748200882876,322.69465011781574]},{"timeMs":9900,"position":[4.9555774325841595,6.407757438450199,322.6979102211108]},{"timeMs":9933,"position":[4.9506444309534885,6.407737262005468,322.7028432245087]},{"timeMs":9967,"position":[4.947927250430242,6.4077262787899345,322.7055604060055]},{"timeMs":10000,"position":[4.947829885909015,6.407725886801347,322.7056577705616]},{"timeMs":11949,"position":[4.947829885909015,6.407725886801347,322.7056577705616]},{"timeMs":11950,"position":[6.36000013,10.5600004,321.290009]}]}}`.
표현 수: `{"CAMERA":7,"SOUND":1,"EFFECT":5}`; World 2, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"CARD_MAZE_ENTER":1,"CARD_MAZE_HIDE_NEXT":4,"CARD_MAZE_STAGE_PLAYERS":1}`.
서버 stage 1, rootMotion 보유 0; worldSequences 2.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–11950 | P77.stage.1 | kouku.gate2.maze.kouku / P77.animation.1 | 0/RAW/kouku.gate2.maze.kouku | 0/0/11950/1/HOLD_LAST_POSE; sourceEnd=11950 |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P77.logic.1 | L40 2관문_카드미로진입 | TRIGGER/CARD_MAZE_ENTER | 8097+344 / True | — |
| P77.logic.2 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 3245+236 / True | — |
| P77.logic.3 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 3426+236 / True | — |
| P77.logic.4 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 3695+236 / True | — |
| P77.logic.5 | L43 2관문_트리거_카드미로 | TRIGGER/CARD_MAZE_HIDE_NEXT | 3930+269 / True | — |
| P77.logic.7 | L112 카드미로_플레이어이동 | TRIGGER/CARD_MAZE_STAGE_PLAYERS | 0+1000 / True | — |

<a id="p78"></a>
## P78 · 세이튼_카드맞추기_주사위패턴

`KOUKUSAYDON_G1_PATTERN_78` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=19900 / stage 합=19831ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":5,"SOUND":17}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"CARD_DICE_BIND":1}`; mechanic trigger: `{}`.
서버 stage 11, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_4 (ACTIVE) | 0–5167 | P78.stage.4 | rpct00_att_battle_11_02 / P78.animation.4 | 4219840/stage-000/animation-000 | 0/0/5167/1/EXACT |
| 2/STAGE_5 (ACTIVE) | 5167–7000 | P78.stage.5 | rpct00_att_battle_21_01 / P78.animation.5 | 4219840/stage-001/animation-000 | 0/0/1833/1/EXACT |
| 3/STAGE_6 (ACTIVE) | 7000–8333 | P78.stage.6 | rpct00_att_battle_21_02 / P78.animation.6 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 4/STAGE_76 (ACTIVE) | 8333–9666 | P78.stage.76 | rpct00_att_battle_21_02 / P78.animation.76 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 5/STAGE_77 (ACTIVE) | 9666–10999 | P78.stage.77 | rpct00_att_battle_21_02 / P78.animation.77 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 6/STAGE_78 (ACTIVE) | 10999–12332 | P78.stage.78 | rpct00_att_battle_21_02 / P78.animation.78 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 7/STAGE_79 (ACTIVE) | 12332–13665 | P78.stage.79 | rpct00_att_battle_21_02 / P78.animation.79 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 8/STAGE_80 (ACTIVE) | 13665–14998 | P78.stage.80 | rpct00_att_battle_21_02 / P78.animation.80 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 9/STAGE_83 (ACTIVE) | 14998–16331 | P78.stage.83 | rpct00_att_battle_21_02 / P78.animation.83 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 10/STAGE_84 (ACTIVE) | 16331–17664 | P78.stage.84 | rpct00_att_battle_21_02 / P78.animation.84 | 4219840/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 11/STAGE_7 (ACTIVE) | 17664–19831 | P78.stage.7 | rpct00_att_battle_21_03 / P78.animation.7 | 4219840/stage-003/animation-000 | 0/0/2167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P78.logic.2 | L86 세이튼_카드맞추기_플레이어속박 | DURATION/CARD_DICE_BIND | 5161+14632 / True | — |
| P78.logic.3 | L87 세이튼_카드맞추기_플레이어속박  | TRIGGER/None | 5185+14578 / False | — |
| P78.logic.9 | L125 세이튼_카드출력_하트 | TRIGGER/PURSUIT_PROJECTILES | 8447+1000 / True | — |
| P78.logic.10 | L126 세이튼_카드출력_스페이드 | TRIGGER/PURSUIT_PROJECTILES | 10937+1000 / True | — |
| P78.logic.11 | L127 세이튼_카드출력_클로버 | TRIGGER/PURSUIT_PROJECTILES | 13604+1000 / True | — |
| P78.logic.12 | L128 세이튼_카드출력_다이아 | TRIGGER/PURSUIT_PROJECTILES | 16270+1005 / True | — |

<a id="p79"></a>
## P79 · 세이튼_공먹고휘리릭_노란장판펑펑

`KOUKUSAYDON_G1_PATTERN_79` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=12966ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":9,"SOUND":12,"COLLIDER":3}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":2}`; mechanic trigger: `{"ALBION_AIRBORNE":4}`.
서버 stage 4, rootMotion 보유 4; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3167 | P79.stage.1 | rpct00_att_battle_17_01 / P79.animation.1 | 4219804/stage-000/animation-000 | 0/0/3167/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3167–4334 | P79.stage.2 | rpct00_att_battle_17_06 / P79.animation.2 | 4219804/stage-001/animation-000 | 0/0/1167/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 4334–7200 | P79.stage.3 | rpct00_att_battle_17_07 / P79.animation.3 | 4219804/stage-002/animation-000 | 0/0/2833/1/EXACT |
| 4/STAGE_5 (ACTIVE) | 7200–12966 | P79.stage.5 | rpct00_att_battle_17_05 / P79.animation.5 | 4219804/stage-004/animation-000 | 3099/0/2667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P79.logic.1 | L75 세이튼_공먹고휘리릭_사라지기 | TRIGGER/ALBION_AIRBORNE | 7148+589 / True | — |
| P79.logic.2 | L76 세이튼_공먹고휘리릭_플레이어추적 | TRIGGER/ALBION_AIRBORNE | 8883+746 / True | — |
| P79.logic.4 | L70 쿠크세이튼_알비온_내려치기_중앙 | TRIGGER/ALBION_AIRBORNE | 8896+153 / True | — |
| P79.logic.5 | L80 세이튼_공먹기_시작위치고정 | TRIGGER/ALBION_AIRBORNE | 8883+4083 / True | — |
| P79.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3267+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P79.logic.7 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5871+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p80"></a>
## P80 · 세이튼_돌진_카운터

`KOUKUSAYDON_G1_PATTERN_80` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=4366ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"SOUND":3,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"COUNTER_WINDOW":1,"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1000 | P80.stage.1 | rpct00_att_battle_22_01 / P80.animation.1 | 4219863/stage-000/animation-000 | 0/0/1000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1000–2833 | P80.stage.2 | rpct00_att_battle_22_04 / P80.animation.2 | 4219863/stage-001/animation-000 | 0/0/1833/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 2833–4366 | P80.stage.3 | rpct00_att_battle_34_05 / P80.animation.3 | 4219863/stage-002/animation-000 | 0/0/1533/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P80.logic.1 | L82 카운터 | DURATION/COUNTER_WINDOW | 0+1332 / True | S: L91:FOLLOWUP_PATTERN |
| P80.logic.2 | L133 세이튼_정면바람방구_타격영역 | TRIGGER/ENTER_AREA | 1200+500 / True | S: L134:MAX_HP_PERCENT_DAMAGE |

의존 패턴: P4 ← onSuccess:koukusaydon.g1.logic.91 (P80.logic.1).

<a id="p81"></a>
## P81 · 세이튼_공굴리기_카운터

`KOUKUSAYDON_G1_PATTERN_81` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=15528 / stage 합=13528ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":7,"SOUND":9,"COLLIDER":1}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"COUNTER_WINDOW":1,"AREA_OVERLAP":1}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 18, rootMotion 보유 18; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P81.stage.1 | rpct00_att_battle_26_01 / P81.animation.1 | 4219866/stage-000/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–3533 | P81.stage.2 | rpct00_att_battle_26_02 / P81.animation.2 | 4219866/stage-001/animation-000 | 0/0/533/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3533–4066 | P81.stage.3 | rpct00_att_battle_26_02 / P81.animation.3 | 4219866/stage-002/animation-000 | 0/0/533/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 4066–4599 | P81.stage.4 | rpct00_att_battle_26_02 / P81.animation.4 | 4219866/stage-003/animation-000 | 0/0/533/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 4599–5132 | P81.stage.5 | rpct00_att_battle_26_02 / P81.animation.5 | 4219866/stage-004/animation-000 | 0/0/533/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 5132–5665 | P81.stage.6 | rpct00_att_battle_26_02 / P81.animation.6 | 4219866/stage-005/animation-000 | 0/0/533/1/EXACT |
| 7/STAGE_7 (ACTIVE) | 5665–6198 | P81.stage.7 | rpct00_att_battle_26_02 / P81.animation.7 | 4219866/stage-006/animation-000 | 0/0/533/1/EXACT |
| 8/STAGE_8 (ACTIVE) | 6198–6731 | P81.stage.8 | rpct00_att_battle_26_02 / P81.animation.8 | 4219866/stage-007/animation-000 | 0/0/533/1/EXACT |
| 9/STAGE_9 (ACTIVE) | 6731–7264 | P81.stage.9 | rpct00_att_battle_26_02 / P81.animation.9 | 4219866/stage-008/animation-000 | 0/0/533/1/EXACT |
| 10/STAGE_40 (ACTIVE) | 7264–7797 | P81.stage.40 | rpct00_att_battle_26_02 / P81.animation.40 | 4219866/stage-001/animation-000 | 0/0/533/1/EXACT |
| 11/STAGE_41 (ACTIVE) | 7797–8330 | P81.stage.41 | rpct00_att_battle_26_02 / P81.animation.41 | 4219866/stage-002/animation-000 | 0/0/533/1/EXACT |
| 12/STAGE_42 (ACTIVE) | 8330–8863 | P81.stage.42 | rpct00_att_battle_26_02 / P81.animation.42 | 4219866/stage-003/animation-000 | 0/0/533/1/EXACT |
| 13/STAGE_43 (ACTIVE) | 8863–9396 | P81.stage.43 | rpct00_att_battle_26_02 / P81.animation.43 | 4219866/stage-004/animation-000 | 0/0/533/1/EXACT |
| 14/STAGE_44 (ACTIVE) | 9396–9929 | P81.stage.44 | rpct00_att_battle_26_02 / P81.animation.44 | 4219866/stage-005/animation-000 | 0/0/533/1/EXACT |
| 15/STAGE_45 (ACTIVE) | 9929–10462 | P81.stage.45 | rpct00_att_battle_26_02 / P81.animation.45 | 4219866/stage-006/animation-000 | 0/0/533/1/EXACT |
| 16/STAGE_46 (ACTIVE) | 10462–10995 | P81.stage.46 | rpct00_att_battle_26_02 / P81.animation.46 | 4219866/stage-007/animation-000 | 0/0/533/1/EXACT |
| 17/STAGE_47 (ACTIVE) | 10995–11528 | P81.stage.47 | rpct00_att_battle_26_02 / P81.animation.47 | 4219866/stage-008/animation-000 | 0/0/533/1/EXACT |
| 18/STAGE_10 (ACTIVE) | 11528–13528 | P81.stage.10 | rpct00_att_battle_26_04 / P81.animation.10 | 4219866/stage-009/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P81.logic.1 | L85 세이튼_공굴리기_카운터판정 | DURATION/COUNTER_WINDOW | 0+2837 / True | S: L7:FOLLOWUP_PATTERN |
| P81.logic.2 | L84 세이튼_공굴리기_추적이동 | DURATION/BOSS_TRACK_TARGET | 3000+8582 / True | — |
| P81.logic.22 | L512 트리거_대미지 | TRIGGER/ENTER_AREA | 2994+8585 / False | S: L515:FIXED_DAMAGE |
| P81.logic.23 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3002+9659 / True | S: L515:FIXED_DAMAGE |

의존 패턴: P4 ← onSuccess:koukusaydon.g1.logic.7 (P81.logic.1).

<a id="p82"></a>
## P82 · 세이튼_마술쇼_종이비둘기

`KOUKUSAYDON_G1_PATTERN_82` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=9924 / stage 합=9890ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":11,"SOUND":9,"COLLIDER":36}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":9}`; mechanic trigger: `{"BOSS_TELEPORT_XZ":1}`.
서버 stage 4, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_6 (ACTIVE) | 0–2833 | P82.stage.6 | rpct00_att_battle_15_01 / P82.animation.6 | 4219941/stage-000/animation-000 | 0/0/2833/1/EXACT |
| 2/STAGE_7 (ACTIVE) | 2833–2866 | P82.stage.7 | rpct00_att_battle_15_02 / P82.animation.7 | 4219941/stage-001/animation-000 | 0/0/33/1/EXACT |
| 3/STAGE_8 (ACTIVE) | 2866–8357 | P82.stage.8 | rpct00_att_battle_15_03 / P82.animation.8 | 4219941/stage-002/animation-000 | 1758/0/3733/1/EXACT |
| 4/STAGE_9 (ACTIVE) | 8357–9890 | P82.stage.9 | rpct00_att_battle_5_01 / P82.animation.9 | 4219941/stage-003/animation-000 | 0/0/1533/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P82.logic.4 | L79 세이튼_비둘기마술쇼_중앙이동 | TRIGGER/BOSS_TELEPORT_XZ | 4587+1000 / True | — |
| P82.logic.5 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8996+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.6 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8994+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.7 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8980+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.8 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8969+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.9 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8953+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.10 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8953+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.11 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8965+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.12 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8941+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P82.logic.13 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8939+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p83"></a>
## P83 · 세이튼_쓰리투원투하!

`KOUKUSAYDON_G1_PATTERN_83` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=20797 / stage 합=16334ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":20,"SOUND":9,"COLLIDER":9}`; World 2, Summon 0, Scene 0.
서버 창 종류: `{"ATTACHMENT_HOLD":1,"ENTER_AREA":19}`; mechanic trigger: `{}`.
서버 stage 5, rootMotion 보유 5; worldSequences 2.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2500 | P83.stage.1 | rpct00_att_battle_18_01 / P83.animation.1 | 4219806/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2500–3833 | P83.stage.2 | rpct00_att_battle_18_02 / P83.animation.2 | 4219806/stage-001/animation-000 | 0/0/1333/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3833–8000 | P83.stage.3 | rpct00_att_battle_18_03 / P83.animation.3 | 4219806/stage-002/animation-000 | 0/0/4167/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 8000–12167 | P83.stage.4 | rpct00_att_battle_29_01 / P83.animation.4 | 4219806/stage-003/animation-000 | 0/0/4167/1/EXACT |
| 5/STAGE_9 (ACTIVE) | 12167–16334 | P83.stage.9 | rpct00_att_battle_29_01 / P83.animation.9 | 4219806/stage-003/animation-000 | 0/0/4167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P83.logic.1 | L81 세이튼_쓰리투원투하_오른손모자 | DURATION/ATTACHMENT_HOLD | 1573+5930 / True | — |
| P83.logic.2 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3175+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P83.logic.3 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4438+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P83.logic.4 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5975+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p84"></a>
## P84 · 쿠크_훌라후프_레드

`KOUKUSAYDON_G1_PATTERN_84` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=None / stage 합=17333ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"ALBION_AIRBORNE":2}`.
서버 stage 9, rootMotion 보유 9; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_19 (ACTIVE) | 0–2000 | P84.stage.19 | rpcz00_att_battle_12_start / P84.animation.19 | 4219726/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 2/STAGE_20 (ACTIVE) | 2000–4000 | P84.stage.20 | rpcz00_att_battle_12_loop / P84.animation.20 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 3/STAGE_25 (ACTIVE) | 4000–6000 | P84.stage.25 | rpcz00_att_battle_12_loop / P84.animation.25 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 4/STAGE_29 (ACTIVE) | 6000–8000 | P84.stage.29 | rpcz00_att_battle_12_loop / P84.animation.29 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 5/STAGE_26 (ACTIVE) | 8000–10000 | P84.stage.26 | rpcz00_att_battle_12_loop / P84.animation.26 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 6/STAGE_27 (ACTIVE) | 10000–12000 | P84.stage.27 | rpcz00_att_battle_12_loop / P84.animation.27 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 7/STAGE_28 (ACTIVE) | 12000–14000 | P84.stage.28 | rpcz00_att_battle_12_loop / P84.animation.28 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 8/STAGE_21 (ACTIVE) | 14000–15333 | P84.stage.21 | rpcz00_att_battle_12_end / P84.animation.21 | 4219726/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 9/STAGE_22 (ACTIVE) | 15333–17333 | P84.stage.22 | rpcz00_att_battle_12_start / P84.animation.22 | 4219726/stage-003/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P84.logic.1 | L77 쿠크_훌라후프_상단시작 | TRIGGER/ALBION_AIRBORNE | 0+1000 / True | — |
| P84.logic.3 | L83 쿠크_훌라후프_하강착지 | TRIGGER/ALBION_AIRBORNE | 0+889 / True | — |
상위 참조: P25 via summon.patternSpawns.

<a id="p85"></a>
## P85 · 쿠크_슈퍼바주카

`KOUKUSAYDON_G1_PATTERN_85` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=8339 / stage 합=8333ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":4,"SOUND":11,"COLLIDER":2}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":2}`; mechanic trigger: `{}`.
서버 stage 4, rootMotion 보유 3; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_3 (ACTIVE) | 0–2000 | P85.stage.3 | rpcz00_att_battle_11_start / P85.animation.3 | 4219725/stage-002/animation-000 | 0/0/2000/1/EXACT |
| 2/STAGE_4 (ACTIVE) | 2000–3000 | P85.stage.4 | rpcz00_att_battle_11_loop / P85.animation.4 | 4219725/stage-003/animation-000 | 0/0/1000/1/LOOP_TO_WINDOW |
| 3/STAGE_5 (ACTIVE) | 3000–6333 | P85.stage.5 | rpcz00_att_battle_11_end / P85.animation.5 | 4219725/stage-004/animation-000 | 0/0/3333/1/EXACT |
| 4/STAGE_6 (ACTIVE) | 6333–8333 | P85.stage.6 | rpcz00_idle_battle_1 / P85.animation.6 | 4219725/stage-005/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P85.logic.2 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3083+1634 / True | S: L117:MAX_HP_PERCENT_DAMAGE |
| P85.logic.3 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5222+1634 / True | S: L117:MAX_HP_PERCENT_DAMAGE |
| P85.logic.4 | L120 트리거_플레이어타겟설정 | TRIGGER/None | 0+356 / True | — |

<a id="p86"></a>
## P86 · 대형세이튼_두번내려치기

`KOUKUSAYDON_G1_PATTERN_86` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=5000 / stage 합=4700ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"resetBossYawDegrees":226.5}`.
표현 수: `{"EFFECT":4,"SOUND":4,"COLLIDER":2}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":2}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1500 | P86.stage.1 | mn_rpct_06_sk.ao_att_battle_1_01 / P86.animation.1 | 4221801/stage-000/animation-000 | 0/0/1500/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1500–2500 | P86.stage.2 | mn_rpct_06_sk.ao_att_battle_1_02 / P86.animation.2 | 4221801/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 2500–4700 | P86.stage.3 | mn_rpct_06_sk.ao_att_battle_1_05 / P86.animation.3 | 4221801/stage-002/animation-000 | 0/0/2200/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P86.logic.1 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 1700+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |
| P86.logic.2 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 3533+100 / True | S: L522:MAX_HP_PERCENT_DAMAGE |

<a id="p87"></a>
## P87 · 대형세이튼_3번내려치기

`KOUKUSAYDON_G1_PATTERN_87` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=4178 / stage 합=3733ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":true,"resetBossYawDegrees":226.5}`.
표현 수: `{"EFFECT":6,"SOUND":4,"COLLIDER":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":3}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3733 | P87.stage.1 | mn_rpct_06_sk.ao_att_battle_3_02 / P87.animation.1 | 4221806/stage-000/animation-000 | 0/0/3733/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P87.logic.1 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 1566+100 / True | S: L521:MAX_HP_PERCENT_DAMAGE |
| P87.logic.2 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 2200+100 / True | S: L521:MAX_HP_PERCENT_DAMAGE |
| P87.logic.3 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 2866+100 / True | S: L521:MAX_HP_PERCENT_DAMAGE |

<a id="p88"></a>
## P88 · 1마리오_1페이즈

`KOUKUSAYDON_G1_PATTERN_88` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=41109 / stage 합=4667ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"playChildrenSequentially":true,"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":9,"COLLIDER":7,"SOUND":3}`; World 4, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1,"AREA_OVERLAP":4,"BOSS_DAMAGE_REDUCTION":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 4.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–4667 | P88.stage.2 | rpct00_att_battle_27_01 / P88.animation.1 | 4219911/stage-000/animation-000 | 0/0/4667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P88.logic.1 | L59 마리오_입장 | TRIGGER/ENTER_AREA | 8083+28157 / True | S: L93:MARIO_ENTER; T: L92:INSTANT_DEATH |
| P88.logic.2 | L58 마리오_괴기스러운인형_공_랜덤스폰 | TRIGGER/None | 3418+1000 / True | — |
| P88.logic.3 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+37744 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P88.logic.4 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+37744 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P88.logic.5 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+37744 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P88.logic.6 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+37744 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P88.logic.7 | L559 마리오_1페이즈_대미지감소 | DURATION/BOSS_DAMAGE_REDUCTION | 0+41074 / True | — |

직접 자식 배치: P59@4667+15247ms repeat=False (P88.pattern.4); P125@19914+1000ms repeat=False (P88.pattern.5); P116@20914+4533ms repeat=False (P88.pattern.6); P124@25447+2134ms repeat=False (P88.pattern.7); P118@27581+13528ms repeat=False (P88.pattern.8).

의존 패턴: P59 ← patternOccurrence (P88.pattern.4); P125 ← patternOccurrence (P88.pattern.5); P116 ← patternOccurrence (P88.pattern.6); P124 ← patternOccurrence (P88.pattern.7); P118 ← patternOccurrence (P88.pattern.8); P33 ← onSuccess:koukusaydon.g1.logic.93 (P88.logic.1).
서버 parentPatternSequence: `{"loopStartOccurrenceId":"","entries":[{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_88.pattern.4","patternId":"KOUKUSAYDON_G1_PATTERN_59","startMs":4667,"durationMs":15247,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_88.pattern.5","patternId":"KOUKUSAYDON_G1_PATTERN_125","startMs":19914,"durationMs":1000,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_88.pattern.6","patternId":"KOUKUSAYDON_G1_PATTERN_116","startMs":20914,"durationMs":4533,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_88.pattern.7","patternId":"KOUKUSAYDON_G1_PATTERN_124","startMs":25447,"durationMs":2134,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_88.pattern.8","patternId":"KOUKUSAYDON_G1_PATTERN_118","startMs":27581,"durationMs":13528,"repeat":false}]}`.

<a id="p89"></a>
## P89 · 쿠크세이튼_1마리오

`KOUKUSAYDON_G1_PATTERN_89` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=76104ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"SOUND":3,"COLLIDER":4}`; World 3, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":3}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 3.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_5 (ACTIVE) | 0–76104 | P89.stage.5 | rpct00_att_battle_27_01 / P89.animation.5 | 4219911/stage-000/animation-000 | 0/0/4667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P89.logic.1 | L58 마리오_괴기스러운인형_공_랜덤스폰 | TRIGGER/None | 3372+1000 / True | — |
| P89.logic.3 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3372+36366 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P89.logic.4 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3372+36366 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P89.logic.5 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3343+36366 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |

<a id="p90"></a>
## P90 · 쿠크_훌라후프_화이트

`KOUKUSAYDON_G1_PATTERN_90` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=69332 / stage 합=17333ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"SOUND":17}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"ALBION_AIRBORNE":2}`.
서버 stage 9, rootMotion 보유 9; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2000 | P90.stage.1 | rpcz00_att_battle_12_start / P90.animation.1 | 4219726/stage-000/animation-000 | 0/0/2000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2000–4000 | P90.stage.2 | rpcz00_att_battle_12_loop / P90.animation.2 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 3/STAGE_3 (ACTIVE) | 4000–6000 | P90.stage.3 | rpcz00_att_battle_12_loop / P90.animation.3 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 4/STAGE_4 (ACTIVE) | 6000–8000 | P90.stage.4 | rpcz00_att_battle_12_loop / P90.animation.4 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 5/STAGE_5 (ACTIVE) | 8000–10000 | P90.stage.5 | rpcz00_att_battle_12_loop / P90.animation.5 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 6/STAGE_6 (ACTIVE) | 10000–12000 | P90.stage.6 | rpcz00_att_battle_12_loop / P90.animation.6 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 7/STAGE_7 (ACTIVE) | 12000–14000 | P90.stage.7 | rpcz00_att_battle_12_loop / P90.animation.7 | 4219726/stage-001/animation-000 | 0/0/2000/1/LOOP_TO_WINDOW |
| 8/STAGE_8 (ACTIVE) | 14000–15333 | P90.stage.8 | rpcz00_att_battle_12_end / P90.animation.8 | 4219726/stage-002/animation-000 | 0/0/1333/1/EXACT |
| 9/STAGE_9 (ACTIVE) | 15333–17333 | P90.stage.9 | rpcz00_att_battle_12_start / P90.animation.9 | 4219726/stage-003/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P90.logic.1 | L77 쿠크_훌라후프_상단시작 | TRIGGER/ALBION_AIRBORNE | 0+1000 / True | — |
| P90.logic.2 | L83 쿠크_훌라후프_하강착지 | TRIGGER/ALBION_AIRBORNE | 0+1000 / True | — |

<a id="p91"></a>
## P91 · 2마리오_1페이즈

`KOUKUSAYDON_G1_PATTERN_91` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=43748 / stage 합=4667ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"playChildrenSequentially":true,"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":10,"COLLIDER":7,"SOUND":3}`; World 5, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":19,"AREA_OVERLAP":4,"BOSS_DAMAGE_REDUCTION":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 5.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_3 (ACTIVE) | 0–4667 | P91.stage.3 | rpct00_att_battle_27_01 / P91.animation.2 | 4219911/stage-000/animation-000 | 0/0/4667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P91.logic.1 | L59 마리오_입장 | TRIGGER/ENTER_AREA | 8083+30480 / True | S: L94:MARIO_ENTER; T: L92:INSTANT_DEATH |
| P91.logic.2 | L58 마리오_괴기스러운인형_공_랜덤스폰 | TRIGGER/None | 3418+1000 / True | — |
| P91.logic.3 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40383 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P91.logic.4 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40383 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P91.logic.5 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40383 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P91.logic.6 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40383 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P91.logic.7 | L559 마리오_1페이즈_대미지감소 | DURATION/BOSS_DAMAGE_REDUCTION | 0+43622 / True | — |

직접 자식 배치: P38@4667+15200ms repeat=False (P91.pattern.4); P124@19867+2134ms repeat=False (P91.pattern.5); P59@22001+15247ms repeat=False (P91.pattern.6); P124@37248+2134ms repeat=False (P91.pattern.7); P117@39382+4366ms repeat=False (P91.pattern.8).

의존 패턴: P38 ← patternOccurrence (P91.pattern.4); P124 ← patternOccurrence (P91.pattern.5); P59 ← patternOccurrence (P91.pattern.6); P124 ← patternOccurrence (P91.pattern.7); P117 ← patternOccurrence (P91.pattern.8); P33 ← onSuccess:koukusaydon.g1.logic.94 (P91.logic.1).
서버 parentPatternSequence: `{"loopStartOccurrenceId":"","entries":[{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_91.pattern.4","patternId":"KOUKUSAYDON_G1_PATTERN_38","startMs":4667,"durationMs":15200,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_91.pattern.5","patternId":"KOUKUSAYDON_G1_PATTERN_124","startMs":19867,"durationMs":2134,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_91.pattern.6","patternId":"KOUKUSAYDON_G1_PATTERN_59","startMs":22001,"durationMs":15247,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_91.pattern.7","patternId":"KOUKUSAYDON_G1_PATTERN_124","startMs":37248,"durationMs":2134,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_91.pattern.8","patternId":"KOUKUSAYDON_G1_PATTERN_117","startMs":39382,"durationMs":4366,"repeat":false}]}`.

<a id="p92"></a>
## P92 · 3마리오_1페이즈

`KOUKUSAYDON_G1_PATTERN_92` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=44283 / stage 합=4667ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"playChildrenSequentially":true,"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":10,"COLLIDER":7,"SOUND":3}`; World 5, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":19,"AREA_OVERLAP":4,"BOSS_DAMAGE_REDUCTION":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 5.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_3 (ACTIVE) | 0–4667 | P92.stage.3 | rpct00_att_battle_27_01 / P92.animation.2 | 4219911/stage-000/animation-000 | 0/0/4667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P92.logic.1 | L59 마리오_입장 | TRIGGER/ENTER_AREA | 8083+31214 / True | S: L95:MARIO_ENTER; T: L92:INSTANT_DEATH |
| P92.logic.2 | L58 마리오_괴기스러운인형_공_랜덤스폰 | TRIGGER/None | 3418+1000 / True | — |
| P92.logic.3 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40918 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P92.logic.4 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40918 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P92.logic.5 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40918 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P92.logic.6 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+40918 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P92.logic.7 | L559 마리오_1페이즈_대미지감소 | DURATION/BOSS_DAMAGE_REDUCTION | 0+44103 / True | — |

직접 자식 배치: P43@4667+12554ms repeat=False (P92.pattern.4); P124@17221+2134ms repeat=False (P92.pattern.5); P39@19355+13378ms repeat=False (P92.pattern.6); P124@32733+2134ms repeat=False (P92.pattern.7); P52@34867+9416ms repeat=False (P92.pattern.8).

의존 패턴: P43 ← patternOccurrence (P92.pattern.4); P124 ← patternOccurrence (P92.pattern.5); P39 ← patternOccurrence (P92.pattern.6); P124 ← patternOccurrence (P92.pattern.7); P52 ← patternOccurrence (P92.pattern.8); P33 ← onSuccess:koukusaydon.g1.logic.95 (P92.logic.1).
서버 parentPatternSequence: `{"loopStartOccurrenceId":"","entries":[{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_92.pattern.4","patternId":"KOUKUSAYDON_G1_PATTERN_43","startMs":4667,"durationMs":12554,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_92.pattern.5","patternId":"KOUKUSAYDON_G1_PATTERN_124","startMs":17221,"durationMs":2134,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_92.pattern.6","patternId":"KOUKUSAYDON_G1_PATTERN_39","startMs":19355,"durationMs":13378,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_92.pattern.7","patternId":"KOUKUSAYDON_G1_PATTERN_124","startMs":32733,"durationMs":2134,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_92.pattern.8","patternId":"KOUKUSAYDON_G1_PATTERN_52","startMs":34867,"durationMs":9416,"repeat":false}]}`.

<a id="p93"></a>
## P93 · 4마리오_1페이즈

`KOUKUSAYDON_G1_PATTERN_93` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=42546 / stage 합=4667ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"playChildrenSequentially":true,"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":10,"COLLIDER":7,"SOUND":3}`; World 5, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":19,"AREA_OVERLAP":4,"BOSS_DAMAGE_REDUCTION":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 5.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_3 (ACTIVE) | 0–4667 | P93.stage.3 | rpct00_att_battle_27_01 / P93.animation.2 | 4219911/stage-000/animation-000 | 0/0/4667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P93.logic.1 | L59 마리오_입장 | TRIGGER/ENTER_AREA | 9574+30267 / True | S: L96:MARIO_ENTER; T: L92:INSTANT_DEATH |
| P93.logic.2 | L58 마리오_괴기스러운인형_공_랜덤스폰 | TRIGGER/None | 3418+1000 / True | — |
| P93.logic.3 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+39181 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P93.logic.4 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+39181 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P93.logic.5 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+39181 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P93.logic.6 | L514 불뿜기_인형_공_틱피해 | DURATION/AREA_OVERLAP | 3365+39181 / True | S: L516:FIXED_DAMAGE, L523:MADNESS_GAUGE_ADD_PERCENT |
| P93.logic.7 | L559 마리오_1페이즈_대미지감소 | DURATION/BOSS_DAMAGE_REDUCTION | 0+42501 / True | — |

직접 자식 배치: P119@4667+12966ms repeat=False (P93.pattern.4); P124@17633+2134ms repeat=False (P93.pattern.5); P40@19767+7267ms repeat=False (P93.pattern.6); P124@27034+2134ms repeat=False (P93.pattern.7); P39@29168+13378ms repeat=False (P93.pattern.8).

의존 패턴: P119 ← patternOccurrence (P93.pattern.4); P124 ← patternOccurrence (P93.pattern.5); P40 ← patternOccurrence (P93.pattern.6); P124 ← patternOccurrence (P93.pattern.7); P39 ← patternOccurrence (P93.pattern.8); P33 ← onSuccess:koukusaydon.g1.logic.96 (P93.logic.1).
서버 parentPatternSequence: `{"loopStartOccurrenceId":"","entries":[{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_93.pattern.4","patternId":"KOUKUSAYDON_G1_PATTERN_119","startMs":4667,"durationMs":12966,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_93.pattern.5","patternId":"KOUKUSAYDON_G1_PATTERN_124","startMs":17633,"durationMs":2134,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_93.pattern.6","patternId":"KOUKUSAYDON_G1_PATTERN_40","startMs":19767,"durationMs":7267,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_93.pattern.7","patternId":"KOUKUSAYDON_G1_PATTERN_124","startMs":27034,"durationMs":2134,"repeat":false},{"occurrenceId":"KOUKUSAYDON_G1_PATTERN_93.pattern.8","patternId":"KOUKUSAYDON_G1_PATTERN_39","startMs":29168,"durationMs":13378,"repeat":false}]}`.

<a id="p94"></a>
## P94 · 메두사공포

`KOUKUSAYDON_G1_PATTERN_94` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=6428 / stage 합=5034ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootVerticalScale":0,"animationRootHorizontalScale":0}`.
표현 수: `{"EFFECT":2,"SOUND":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"GAZE_REAL_BOSS":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_4 (ACTIVE) | 0–867 | P94.stage.4 | rpct00_att_battle_20_02 / P94.animation.4 | 42198102/stage-000/animation-000 | 0/0/867/1/EXACT |
| 2/STAGE_5 (ACTIVE) | 867–2867 | P94.stage.5 | rpct00_att_battle_20_03 / P94.animation.5 | 42198102/stage-001/animation-000 | 0/0/2000/1/EXACT |
| 3/STAGE_6 (ACTIVE) | 2867–5034 | P94.stage.6 | rpct00_att_battle_20_04 / P94.animation.6 | 42198102/stage-002/animation-000 | 0/0/2167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P94.logic.1 | L555 메두사공포_활성시야판정 | DURATION/GAZE_REAL_BOSS | 3167+3261 / True | F: L37:FEAR |
상위 참조: P107 via patternOccurrence.

<a id="p95"></a>
## P95 · 즉사 칼날 | Object 판정 테스트

`KOUKUSAYDON_G1_PATTERN_95` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=22000 / stage 합=12000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 4, rootMotion 보유 0; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P95.stage.1 | rpct00_idle_normal_1 / P95.animation.1 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–6000 | P95.stage.2 | rpct00_idle_normal_1 / P95.animation.2 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 6000–9000 | P95.stage.3 | rpct00_idle_normal_1 / P95.animation.3 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 9000–12000 | P95.stage.4 | rpct00_idle_normal_1 / P95.animation.4 | 0/stage-003/animation-000 | 0/0/3000/1/EXACT |

<a id="p97"></a>
## P97 · 앵콜컷신

`KOUKUSAYDON_G1_PATTERN_97` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=21322 / stage 합=18333ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"CAMERA":1,"EFFECT":2,"SOUND":2,"SUBTITLE":3}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–18333 | P97.stage.1 | — / — | —/—/— | —/—/—/—/— |

<a id="p99"></a>
## P99 · 대형세이튼_불어날리기

`KOUKUSAYDON_G1_PATTERN_99` · GATE2 · `MN_RPCT_06` · `boss.koukusaydon.g2.big-saydon`
저작 durationMs=5735 / stage 합=5201ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":7,"SOUND":3,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1867 | P99.stage.1 | mn_rpct_06_sk.ao_att_battle_2_01 / P99.animation.1 | 4221804/stage-000/animation-000 | 0/0/1867/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1867–3534 | P99.stage.2 | mn_rpct_06_sk.ao_att_battle_2_02 / P99.animation.2 | 4221804/stage-001/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3534–5201 | P99.stage.3 | mn_rpct_06_sk.ao_att_battle_2_03 / P99.animation.3 | 4221804/stage-002/animation-000 | 0/0/1667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P99.logic.2 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 1610+1645 / True | S: L549:MAX_HP_PERCENT_DAMAGE |

<a id="p100"></a>
## P100 · 세이튼_정면바람방구

`KOUKUSAYDON_G1_PATTERN_100` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=4533ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"COLLIDER":1,"SOUND":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–3000 | P100.stage.2 | rpct00_att_battle_36_01 / P100.animation.2 | 4219873/stage-001/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 3000–4533 | P100.stage.3 | rpct00_att_battle_5_01 / P100.animation.3 | 4219873/stage-002/animation-000 | 0/0/1533/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P100.logic.1 | L133 세이튼_정면바람방구_타격영역 | TRIGGER/ENTER_AREA | 2000+500 / True | S: L134:MAX_HP_PERCENT_DAMAGE |

<a id="p101"></a>
## P101 · 세이튼_플레이어추적

`KOUKUSAYDON_G1_PATTERN_101` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=2134ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootHorizontalScale":0}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 2, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1067 | P101.stage.2 | rpct00_run_battle_1 / P101.animation.2 | 1/stage-002/animation-000 | 0/0/1067/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1067–2134 | P101.stage.3 | rpct00_run_battle_1 / P101.animation.3 | 1/stage-002/animation-000 | 0/0/1067/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P101.logic.1 | L132 세이튼_플레이어추적_이동 | DURATION/BOSS_TRACK_TARGET | 0+2134 / True | — |

<a id="p102"></a>
## P102 · 세이튼_백스텝후불뿜기화염링

`KOUKUSAYDON_G1_PATTERN_102` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=16519 / stage 합=6434ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":8,"SOUND":4,"COLLIDER":10}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":8}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P102.stage.1 | rpct00_att_battle_34_04 / P102.animation.1 | 4219951/stage-004/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–3267 | P102.stage.2 | rpct00_att_battle_14_01 / P102.animation.2 | 4219940/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3267–6434 | P102.stage.3 | rpct00_att_battle_14_02 / P102.animation.3 | 4219940/stage-003/animation-000 | 0/0/3167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P102.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3322+2000 / False | S: L515:FIXED_DAMAGE |
| P102.logic.2 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3378+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.3 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3362+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.4 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3385+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.5 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3361+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.6 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3338+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.7 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3315+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.8 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3315+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.9 | L541 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1555+4894 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.10 | L541 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1555+4894 / False | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
| P102.logic.11 | L541 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1555+4894 / False | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |

<a id="p103"></a>
## P103 · 세이튼_카드비

`KOUKUSAYDON_G1_PATTERN_103` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=10590 / stage 합=10535ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":2,"SOUND":7}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"CARD_RAIN_SOLDIERS":1}`.
서버 stage 5, rootMotion 보유 5; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3267 | P103.stage.1 | rpct00_att_battle_16_01 / P103.animation.1 | 4219803/stage-000/animation-000 | 0/0/3267/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3267–5134 | P103.stage.2 | rpct00_att_battle_16_02 / P103.animation.2 | 4219803/stage-001/animation-000 | 0/0/1867/1/EXACT |
| 3/STAGE_4 (ACTIVE) | 5134–7001 | P103.stage.4 | rpct00_att_battle_16_02 / P103.animation.4 | 4219803/stage-001/animation-000 | 0/0/1867/1/EXACT |
| 4/STAGE_5 (ACTIVE) | 7001–8868 | P103.stage.5 | rpct00_att_battle_16_02 / P103.animation.5 | 4219803/stage-001/animation-000 | 0/0/1867/1/EXACT |
| 5/STAGE_3 (ACTIVE) | 8868–10535 | P103.stage.3 | rpct00_att_battle_16_03 / P103.animation.3 | 4219803/stage-002/animation-000 | 0/0/1667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P103.logic.1 | L129 카드비_카드병정소환 | TRIGGER/CARD_RAIN_SOLDIERS | 3264+426 / True | — |
| P103.logic.3 | L131 카드비_카드낙하 | DURATION/SHOWTIME_PLAYER_TARGETS | 3250+5637 / True | — |

<a id="p104"></a>
## P104 · 세이튼_1초추적

`KOUKUSAYDON_G1_PATTERN_104` · GATE1 · `MN_RPCT_05` · `boss.koukusaydon.g1.saydon`
저작 durationMs=None / stage 합=1000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootHorizontalScale":0}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1000 | P104.stage.2 | rpct00_run_battle_1 / P104.animation.2 | 1/stage-002/animation-000 | 0/0/1000/1/LOOP_TO_WINDOW |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P104.logic.1 | L132 세이튼_플레이어추적_이동 | DURATION/BOSS_TRACK_TARGET | 0+1000 / True | — |

<a id="p105"></a>
## P105 · 쿠크_나팔액션

`KOUKUSAYDON_G1_PATTERN_105` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=5500 / stage 합=5400ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":12,"EFFECT":8,"COLLIDER":6}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2500 | P105.stage.1 | rpcz00_att_battle_3_01 / P105.animation.1 | 4219713/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2500–4400 | P105.stage.2 | rpcz00_att_battle_3_06 / P105.animation.2 | 4219713/stage-001/animation-000 | 0/0/1900/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 4400–5400 | P105.stage.3 | rpcz00_att_battle_3_09 / P105.animation.3 | 4219713/stage-002/animation-000 | 0/0/1000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P105.logic.1 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3600+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p106"></a>
## P106 · 쿠크_저글링액션

`KOUKUSAYDON_G1_PATTERN_106` · GATE2 · `MN_RPCZ_00` · `boss.koukusaydon.g2.kouku`
저작 durationMs=15550 / stage 합=11600ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":28,"EFFECT":14}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"ALBION_AIRBORNE":6}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–5800 | P106.stage.1 | rpcz00_att_battle_4_01 / P106.animation.1 | 4219716/stage-000/animation-000 | 0/0/5800/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 5800–11600 | P106.stage.2 | rpcz00_att_battle_4_01 / P106.animation.2 | 4219716/stage-001/animation-000 | 0/0/5800/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P106.logic.1 | L530 저글링 투척 직전 플레이어 위치 포착 | TRIGGER/ALBION_AIRBORNE | 2200+34 / True | — |
| P106.logic.2 | L531 저글링 투척 직전 플레이어 위치 포착 | TRIGGER/ALBION_AIRBORNE | 3700+34 / True | — |
| P106.logic.3 | L532 저글링 투척 직전 플레이어 위치 포착 | TRIGGER/ALBION_AIRBORNE | 5200+34 / True | — |
| P106.logic.4 | L533 저글링 투척 직전 플레이어 위치 포착 | TRIGGER/ALBION_AIRBORNE | 8000+34 / True | — |
| P106.logic.5 | L534 저글링 투척 직전 플레이어 위치 포착 | TRIGGER/ALBION_AIRBORNE | 9500+34 / True | — |
| P106.logic.6 | L535 저글링 투척 직전 플레이어 위치 포착 | TRIGGER/ALBION_AIRBORNE | 11000+34 / True | — |

<a id="p107"></a>
## P107 · 빙고_이동_첫클립_메두사_블랙홀

`KOUKUSAYDON_G1_PATTERN_107` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=32628 / stage 합=0ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootVerticalScale":0,"animationRootHorizontalScale":0}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"BINGO_COMPLETED_LINES":1,"GAZE_REAL_BOSS":1}`; mechanic trigger: `{"BOSS_TELEPORT_FACE_CENTER":1,"BINGO_DETONATION":1}`.
서버 stage 10, rootMotion 보유 0; worldSequences 0.

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P107.logic.1 | L138 빙고_블랙홀빔이동과중앙블랙홀 | TRIGGER/BOSS_TELEPORT_FACE_CENTER | 0+100 / True | — |
| P107.logic.2 | L525 빙고_새_빨간_가로세로_1줄_완성 | DURATION/BINGO_COMPLETED_LINES | 0+32628 / True | S: L526:PLAYER_INVULNERABILITY |

저장 stage와 투영 stage의 차이: 아래는 실제 Encounter와 patternbindings의 대응이다. 파생 binding에는 원본 sourceActionId가 없으므로 출처는 위 저장 stage/자식 패턴과 함께 읽는다.

| runtime Stage / action | 누적 구간 ms | binding occurrence / clip | offset / sourceStart / playMs / rate / end |
|---|---|---|---|
| STAGE_1 / P107_ACTION_1 | 0–5400 | P107.pattern.1.r0.animation.1 / rpct00_att_battle_12_02 | 0/0/5400/1/HOLD_LAST_POSE |
| STAGE_2 / P107_ACTION_2 | 5400–6267 | P107.pattern.2.r0.animation.1 / rpct00_att_battle_20_02 | 0/0/867/1/EXACT |
| STAGE_3 / P107_ACTION_3 | 6267–8267 | P107.pattern.2.r0.animation.2 / rpct00_att_battle_20_03 | 0/0/2000/1/EXACT |
| STAGE_4 / P107_ACTION_4 | 8267–10434 | P107.pattern.2.r0.animation.3 / rpct00_att_battle_20_04 | 0/0/2167/1/EXACT |
| STAGE_5 / P107_ACTION_5 | 10434–11828 | P107.idle.5 / rpct00_idle_battle_1 | 0/0/1394/1.0/LOOP_TO_WINDOW |
| STAGE_6 / P107_ACTION_6 | 11828–24828 | P107.pattern.3.r0.animation.1 / rpct00_att_battle_12_04 | 0/0/13000/1/LOOP_TO_WINDOW |
| STAGE_7 / P107_ACTION_7 | 24828–25028 | P107.pattern.3.r0.animation.2 / rpct00_att_battle_12_05 | 0/0/200/1/HOLD_LAST_POSE |
| STAGE_8 / P107_ACTION_8 | 25028–26195 | P107.pattern.3.r0.animation.3 / rpct00_dmg_critical_start_1 | 0/0/1167/1/HOLD_LAST_POSE |
| STAGE_9 / P107_ACTION_9 | 26195–31295 | P107.pattern.3.r0.animation.4 / rpct00_dmg_critical_loop_1 | 0/0/5100/1/LOOP_TO_WINDOW |
| STAGE_10 / P107_ACTION_10 | 31295–32628 | P107.pattern.3.r0.animation.5 / rpct00_dmg_critical_end_1 | 0/0/1333/1/HOLD_LAST_POSE |

직접 자식 배치: P127@0+5400ms repeat=False (P107.pattern.1); P94@5400+6428ms repeat=False (P107.pattern.2); P128@11828+20800ms repeat=False (P107.pattern.3).

의존 패턴: P127 ← patternOccurrence (P107.pattern.1); P94 ← patternOccurrence (P107.pattern.2); P128 ← patternOccurrence (P107.pattern.3).

<a id="p108"></a>
## P108 · 앵콜세이튼_분신소환_기분나빠

`KOUKUSAYDON_G1_PATTERN_108` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=None / stage 합=9267ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":1,"SOUND":4,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 7, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P108.stage.1 | rpct00_att_battle_34_01 / P108.animation.1 | 4219920/stage-000/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–3267 | P108.stage.2 | rpct00_att_battle_9_01 / P108.animation.2 | 4219951/stage-001/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3267–4267 | P108.stage.3 | rpct00_att_battle_9_02 / P108.animation.3 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 4267–5267 | P108.stage.4 | rpct00_att_battle_9_02 / P108.animation.4 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 5267–6267 | P108.stage.5 | rpct00_att_battle_9_02 / P108.animation.5 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 6267–7267 | P108.stage.6 | rpct00_att_battle_9_02 / P108.animation.6 | 4219951/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 7/STAGE_7 (ACTIVE) | 7267–9267 | P108.stage.7 | rpct00_att_battle_9_03 / P108.animation.7 | 4219951/stage-003/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P108.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3203+4279 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |

<a id="p109"></a>
## P109 · 앵콜세이튼_기본공격

`KOUKUSAYDON_G1_PATTERN_109` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=2100 / stage 합=2000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":4,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2000 | P109.stage.1 | rpct00_att_battle_1_01 / P109.animation.1 | 4219959/stage-000/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P109.logic.1 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 1122+112 / True | S: L538:MAX_HP_PERCENT_DAMAGE |

<a id="p110"></a>
## P110 · 앵콜세이튼_백스텝후화염링

`KOUKUSAYDON_G1_PATTERN_110` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=0ms / category=MECHANIC / 투영=아니오.

미투영 근거: `PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_110`.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.

<a id="p111"></a>
## P111 · 앵콜세이튼_십자화염폭발

`KOUKUSAYDON_G1_PATTERN_111` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=0ms / category=MECHANIC / 투영=아니오.

미투영 근거: `PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_111`.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.

<a id="p112"></a>
## P112 · 앵콜세이튼_십자화염폭발

`KOUKUSAYDON_G1_PATTERN_112` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=7282 / stage 합=7282ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":2,"EFFECT":3,"COLLIDER":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":1}`; mechanic trigger: `{}`.
서버 stage 2, rootMotion 보유 1; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–4282 | P112.stage.2 | rpct00_att_battle_10_02 / P112.animation.2 | 4219948/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 2/STAGE_4 (RECOVERY) | 4282–7282 | P112.stage.4 | rpct00_idle_battle_1 / P112.animation.3 | 4219820/stage-003/animation-000 | 0/0/3000/1/LOOP_TO_WINDOW |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P112.logic.1 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3113+4086 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |

<a id="p113"></a>
## P113 · 앵콜세이튼_정면바람방구

`KOUKUSAYDON_G1_PATTERN_113` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=None / stage 합=4533ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"COLLIDER":1,"SOUND":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P113.stage.1 | rpct00_att_battle_36_01 / P113.animation.1 | 4219873/stage-001/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–4533 | P113.stage.2 | rpct00_att_battle_5_01 / P113.animation.2 | 4219873/stage-002/animation-000 | 0/0/1533/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P113.logic.1 | L133 세이튼_정면바람방구_타격영역 | TRIGGER/ENTER_AREA | 2000+500 / True | S: L134:MAX_HP_PERCENT_DAMAGE |
| P113.logic.2 | L65 쿠크세이튼_쇼타임_추적회전 | DURATION/BOSS_TRACK_TARGET | 0+100 / True | — |

<a id="p114"></a>
## P114 · 앵콜세이튼_백스텝후화염링

`KOUKUSAYDON_G1_PATTERN_114` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=7969 / stage 합=6434ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":8,"SOUND":4,"COLLIDER":10}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"AREA_OVERLAP":8}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1600 | P114.stage.1 | rpct00_att_battle_34_04 / P114.animation.1 | 4219951/stage-004/animation-000 | 0/0/1600/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1600–3267 | P114.stage.2 | rpct00_att_battle_14_01 / P114.animation.2 | 4219940/stage-002/animation-000 | 0/0/1667/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3267–6434 | P114.stage.3 | rpct00_att_battle_14_02 / P114.animation.3 | 4219940/stage-003/animation-000 | 0/0/3167/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P114.logic.1 | L541 Breath: three contact ticks | DURATION/AREA_OVERLAP | 1555+4894 / True | S: L515:FIXED_DAMAGE, L539:MADNESS_GAUGE_ADD_PERCENT |
| P114.logic.2 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3378+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P114.logic.3 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3362+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P114.logic.4 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3385+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P114.logic.5 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3361+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P114.logic.6 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3338+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P114.logic.7 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3315+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |
| P114.logic.8 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3315+4584 / True | S: L515:FIXED_DAMAGE, L544:MADNESS_GAUGE_ADD_PERCENT |

<a id="p115"></a>
## P115 · 앵콜세이튼_화염파동

`KOUKUSAYDON_G1_PATTERN_115` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=15247 / stage 합=11534ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":2,"SOUND":4,"COLLIDER":28}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":28}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–4267 | P115.stage.1 | rpct00_att_battle_10_02 / P115.animation.1 | 4219820/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 4267–8534 | P115.stage.2 | rpct00_att_battle_10_02 / P115.animation.2 | 4219820/stage-001/animation-000 | 0/0/4267/1/EXACT |
| 3/STAGE_3 (RECOVERY) | 8534–11534 | P115.stage.3 | rpct00_idle_battle_1 / P115.animation.3 | 4219820/stage-003/animation-000 | 0/0/3000/1/LOOP_TO_WINDOW |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P115.logic.1 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3104+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.2 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.3 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.4 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.5 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.6 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.7 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.8 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.9 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.10 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.11 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3104+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.12 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3404+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.13 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 3704+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.14 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 4004+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.15 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7347+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.16 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7647+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.17 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7647+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.18 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.19 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.20 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.21 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.22 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.23 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.24 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.25 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7347+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.26 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7647+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.27 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 7947+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P115.logic.28 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8247+500 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p116"></a>
## P116 · 쿠크세이튼_정면바람방구

`KOUKUSAYDON_G1_PATTERN_116` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=4533ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"COLLIDER":1,"SOUND":4}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P116.stage.1 | rpct00_att_battle_36_01 / P116.animation.1 | 4219873/stage-001/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–4533 | P116.stage.2 | rpct00_att_battle_5_01 / P116.animation.2 | 4219873/stage-002/animation-000 | 0/0/1533/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P116.logic.1 | L133 세이튼_정면바람방구_타격영역 | TRIGGER/ENTER_AREA | 2000+500 / True | S: L134:MAX_HP_PERCENT_DAMAGE |
| P116.logic.2 | L527 플레이어 방향 즉시 바라보기 | TRIGGER/BOSS_TRACK_TARGET | 0+34 / True | — |
상위 참조: P88 via patternOccurrence.

<a id="p117"></a>
## P117 · 쿠크세이튼_돌진_카운터

`KOUKUSAYDON_G1_PATTERN_117` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=4366ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":3,"SOUND":3,"COLLIDER":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"COUNTER_WINDOW":1,"ENTER_AREA":1}`; mechanic trigger: `{}`.
서버 stage 3, rootMotion 보유 3; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1000 | P117.stage.1 | rpct00_att_battle_22_01 / P117.animation.1 | 4219863/stage-000/animation-000 | 0/0/1000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1000–2833 | P117.stage.2 | rpct00_att_battle_22_04 / P117.animation.2 | 4219863/stage-001/animation-000 | 0/0/1833/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 2833–4366 | P117.stage.3 | rpct00_att_battle_34_05 / P117.animation.3 | 4219863/stage-002/animation-000 | 0/0/1533/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P117.logic.1 | L82 카운터 | DURATION/COUNTER_WINDOW | 0+1332 / True | S: L140:FOLLOWUP_PATTERN |
| P117.logic.2 | L133 세이튼_정면바람방구_타격영역 | TRIGGER/ENTER_AREA | 1200+500 / True | S: L134:MAX_HP_PERCENT_DAMAGE |

의존 패턴: P42 ← onSuccess:koukusaydon.g1.logic.140 (P117.logic.1).
상위 참조: P91 via patternOccurrence.

<a id="p118"></a>
## P118 · 쿠크세이튼_공굴리기_카운터

`KOUKUSAYDON_G1_PATTERN_118` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=13528ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":7,"SOUND":9,"COLLIDER":18}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"COUNTER_WINDOW":1,"ENTER_AREA":17,"AREA_OVERLAP":1}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 18, rootMotion 보유 18; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3000 | P118.stage.1 | rpct00_att_battle_26_01 / P118.animation.1 | 4219866/stage-000/animation-000 | 0/0/3000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3000–3533 | P118.stage.2 | rpct00_att_battle_26_02 / P118.animation.2 | 4219866/stage-001/animation-000 | 0/0/533/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3533–4066 | P118.stage.3 | rpct00_att_battle_26_02 / P118.animation.3 | 4219866/stage-002/animation-000 | 0/0/533/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 4066–4599 | P118.stage.4 | rpct00_att_battle_26_02 / P118.animation.4 | 4219866/stage-003/animation-000 | 0/0/533/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 4599–5132 | P118.stage.5 | rpct00_att_battle_26_02 / P118.animation.5 | 4219866/stage-004/animation-000 | 0/0/533/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 5132–5665 | P118.stage.6 | rpct00_att_battle_26_02 / P118.animation.6 | 4219866/stage-005/animation-000 | 0/0/533/1/EXACT |
| 7/STAGE_7 (ACTIVE) | 5665–6198 | P118.stage.7 | rpct00_att_battle_26_02 / P118.animation.7 | 4219866/stage-006/animation-000 | 0/0/533/1/EXACT |
| 8/STAGE_8 (ACTIVE) | 6198–6731 | P118.stage.8 | rpct00_att_battle_26_02 / P118.animation.8 | 4219866/stage-007/animation-000 | 0/0/533/1/EXACT |
| 9/STAGE_9 (ACTIVE) | 6731–7264 | P118.stage.9 | rpct00_att_battle_26_02 / P118.animation.9 | 4219866/stage-008/animation-000 | 0/0/533/1/EXACT |
| 10/STAGE_10 (ACTIVE) | 7264–7797 | P118.stage.10 | rpct00_att_battle_26_02 / P118.animation.10 | 4219866/stage-001/animation-000 | 0/0/533/1/EXACT |
| 11/STAGE_11 (ACTIVE) | 7797–8330 | P118.stage.11 | rpct00_att_battle_26_02 / P118.animation.11 | 4219866/stage-002/animation-000 | 0/0/533/1/EXACT |
| 12/STAGE_12 (ACTIVE) | 8330–8863 | P118.stage.12 | rpct00_att_battle_26_02 / P118.animation.12 | 4219866/stage-003/animation-000 | 0/0/533/1/EXACT |
| 13/STAGE_13 (ACTIVE) | 8863–9396 | P118.stage.13 | rpct00_att_battle_26_02 / P118.animation.13 | 4219866/stage-004/animation-000 | 0/0/533/1/EXACT |
| 14/STAGE_14 (ACTIVE) | 9396–9929 | P118.stage.14 | rpct00_att_battle_26_02 / P118.animation.14 | 4219866/stage-005/animation-000 | 0/0/533/1/EXACT |
| 15/STAGE_15 (ACTIVE) | 9929–10462 | P118.stage.15 | rpct00_att_battle_26_02 / P118.animation.15 | 4219866/stage-006/animation-000 | 0/0/533/1/EXACT |
| 16/STAGE_16 (ACTIVE) | 10462–10995 | P118.stage.16 | rpct00_att_battle_26_02 / P118.animation.16 | 4219866/stage-007/animation-000 | 0/0/533/1/EXACT |
| 17/STAGE_17 (ACTIVE) | 10995–11528 | P118.stage.17 | rpct00_att_battle_26_02 / P118.animation.17 | 4219866/stage-008/animation-000 | 0/0/533/1/EXACT |
| 18/STAGE_18 (ACTIVE) | 11528–13528 | P118.stage.18 | rpct00_att_battle_26_04 / P118.animation.18 | 4219866/stage-009/animation-000 | 0/0/2000/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P118.logic.1 | L85 세이튼_공굴리기_카운터판정 | DURATION/COUNTER_WINDOW | 0+2837 / True | S: L57:FOLLOWUP_PATTERN |
| P118.logic.2 | L84 세이튼_공굴리기_추적이동 | DURATION/BOSS_TRACK_TARGET | 3000+8582 / True | — |
| P118.logic.3 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3000+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.4 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3533+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4066+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.6 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 4599+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.7 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5132+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.8 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 5665+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.9 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6198+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.10 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 6731+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.11 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7264+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.12 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 7797+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.13 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8330+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.14 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 8863+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.15 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 9396+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.16 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 9929+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.17 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 10462+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.18 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 10995+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.19 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 12028+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P118.logic.20 | L513 불뿜기_일반틱피해 | DURATION/AREA_OVERLAP | 3002+8554 / True | S: L515:FIXED_DAMAGE |

의존 패턴: P42 ← onSuccess:koukusaydon.g1.logic.57 (P118.logic.1).
상위 참조: P88 via patternOccurrence.

<a id="p119"></a>
## P119 · 쿠크세이튼_공먹고휘리릭노란장판펑펑

`KOUKUSAYDON_G1_PATTERN_119` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=12966ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":9,"SOUND":12,"COLLIDER":3}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":2}`; mechanic trigger: `{"ALBION_AIRBORNE":4}`.
서버 stage 4, rootMotion 보유 4; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3167 | P119.stage.1 | rpct00_att_battle_17_01 / P119.animation.1 | 4219804/stage-000/animation-000 | 0/0/3167/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3167–4334 | P119.stage.2 | rpct00_att_battle_17_06 / P119.animation.2 | 4219804/stage-001/animation-000 | 0/0/1167/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 4334–7200 | P119.stage.3 | rpct00_att_battle_17_07 / P119.animation.3 | 4219804/stage-002/animation-000 | 0/0/2833/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 7200–12966 | P119.stage.4 | rpct00_att_battle_17_05 / P119.animation.4 | 4219804/stage-004/animation-000 | 3099/0/2667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P119.logic.1 | L75 세이튼_공먹고휘리릭_사라지기 | TRIGGER/ALBION_AIRBORNE | 7148+589 / True | — |
| P119.logic.2 | L76 세이튼_공먹고휘리릭_플레이어추적 | TRIGGER/ALBION_AIRBORNE | 8883+746 / True | — |
| P119.logic.3 | L70 쿠크세이튼_알비온_내려치기_중앙 | TRIGGER/ALBION_AIRBORNE | 8896+153 / True | — |
| P119.logic.4 | L137 세이튼_공먹기_시작위치고정 (copy) | TRIGGER/ALBION_AIRBORNE | 8883+4083 / True | — |
| P119.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3267+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P119.logic.6 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5871+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
상위 참조: P93 via patternOccurrence.

<a id="p120"></a>
## P120 · 앵콜세이튼_알비온_감전장판

`KOUKUSAYDON_G1_PATTERN_120` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=None / stage 합=15864ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":16,"SOUND":13,"COLLIDER":17}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":10}`; mechanic trigger: `{"ALBION_AIRBORNE":8,"ALBION_BLUE_CIRCLE":1}`.
서버 stage 6, rootMotion 보유 6; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–2500 | P120.stage.1 | rpct00_att_battle_24_01 / P120.animation.1 | 4219903/stage-000/animation-000 | 0/0/2500/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 2500–3500 | P120.stage.2 | rpct00_att_battle_24_02 / P120.animation.2 | 4219903/stage-001/animation-000 | 0/0/1000/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 3500–4500 | P120.stage.3 | rpct00_att_battle_24_03 / P120.animation.3 | 4219903/stage-002/animation-000 | 0/0/1000/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 4500–6918 | P120.stage.4 | rpct00_att_battle_24_04 / P120.animation.4 | 4219903/stage-003/animation-000 | 918/0/1500/1/EXACT |
| 5/STAGE_5 (ACTIVE) | 6918–9323 | P120.stage.5 | rpct00_att_battle_24_04 / P120.animation.5 | 4219903/stage-004/animation-000 | 905/0/1500/1/EXACT |
| 6/STAGE_6 (ACTIVE) | 9323–15864 | P120.stage.6 | rpct00_att_battle_24_05 / P120.animation.6 | 4219903/stage-007/animation-000 | 1427/0/2500/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P120.logic.1 | L53 로직_랜덤플레이어탐색 | TRIGGER/ALBION_AIRBORNE | 5432+230 / True | — |
| P120.logic.2 | L53 로직_랜덤플레이어탐색 | TRIGGER/ALBION_AIRBORNE | 7817+291 / True | — |
| P120.logic.3 | L66 쿠크세이튼_알비온_점프 | TRIGGER/ALBION_AIRBORNE | 3483+294 / True | — |
| P120.logic.4 | L68 쿠크세이튼_알비온_순간이동_등장 | TRIGGER/ALBION_AIRBORNE | 5432+225 / True | — |
| P120.logic.5 | L67 쿠크세이튼_알비온_순간이동_사라지기 | TRIGGER/ALBION_AIRBORNE | 6913+233 / True | — |
| P120.logic.6 | L139 앵콜세이튼_알비온_빙고중앙 | TRIGGER/ALBION_AIRBORNE | 9292+241 / True | — |
| P120.logic.7 | L70 쿠크세이튼_알비온_내려치기_중앙 | TRIGGER/ALBION_AIRBORNE | 11808+302 / True | — |
| P120.logic.8 | L68 쿠크세이튼_알비온_순간이동_등장 | TRIGGER/ALBION_AIRBORNE | 7817+225 / True | — |
| P120.logic.9 | L72 쿠크세이튼_알비온_플레이어추적_파란장판 | TRIGGER/ALBION_BLUE_CIRCLE | 4426+1000 / True | — |
| P120.logic.10 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6473+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P120.logic.11 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6474+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P120.logic.12 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6453+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P120.logic.13 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6444+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P120.logic.14 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6445+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P120.logic.15 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6424+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P120.logic.16 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6383+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |
| P120.logic.17 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 6490+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P120.logic.18 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 8849+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P120.logic.19 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 12241+100 / True | S: L538:MAX_HP_PERCENT_DAMAGE |

<a id="p122"></a>
## P122 · 앵콜세이튼_공먹고휘리릭노란장판펑펑

`KOUKUSAYDON_G1_PATTERN_122` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=None / stage 합=12966ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"EFFECT":9,"SOUND":12,"COLLIDER":3}`; World 1, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":2}`; mechanic trigger: `{"ALBION_AIRBORNE":4}`.
서버 stage 4, rootMotion 보유 4; worldSequences 1.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–3167 | P122.stage.1 | rpct00_att_battle_17_01 / P122.animation.1 | 4219804/stage-000/animation-000 | 0/0/3167/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 3167–4334 | P122.stage.2 | rpct00_att_battle_17_06 / P122.animation.2 | 4219804/stage-001/animation-000 | 0/0/1167/1/EXACT |
| 3/STAGE_3 (ACTIVE) | 4334–7200 | P122.stage.3 | rpct00_att_battle_17_07 / P122.animation.3 | 4219804/stage-002/animation-000 | 0/0/2833/1/EXACT |
| 4/STAGE_4 (ACTIVE) | 7200–12966 | P122.stage.4 | rpct00_att_battle_17_05 / P122.animation.4 | 4219804/stage-004/animation-000 | 3099/0/2667/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P122.logic.1 | L75 세이튼_공먹고휘리릭_사라지기 | TRIGGER/ALBION_AIRBORNE | 7148+589 / True | — |
| P122.logic.2 | L76 세이튼_공먹고휘리릭_플레이어추적 | TRIGGER/ALBION_AIRBORNE | 8883+746 / True | — |
| P122.logic.3 | L70 쿠크세이튼_알비온_내려치기_중앙 | TRIGGER/ALBION_AIRBORNE | 8896+153 / True | — |
| P122.logic.4 | L136 세이튼_공먹기_시작위치고정 (copy) | TRIGGER/ALBION_AIRBORNE | 8883+4083 / True | — |
| P122.logic.5 | L97 [source attack] source damage contact | TRIGGER/ENTER_AREA | 3267+34 / True | S: L98:MAX_HP_PERCENT_DAMAGE |
| P122.logic.6 | L508 트리거_대미지 | TRIGGER/ENTER_AREA | 5871+100 / True | S: L98:MAX_HP_PERCENT_DAMAGE |

<a id="p123"></a>
## P123 · 앵콜세이튼_레이저

`KOUKUSAYDON_G1_PATTERN_123` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=3574 / stage 합=3500ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false}`.
표현 수: `{"SOUND":4,"COLLIDER":2,"EFFECT":1}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{"ENTER_AREA":1}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 2, rootMotion 보유 2; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–1000 | P123.stage.1 | rpct00_att_battle_35_01 / P123.animation.1 | 4219983/stage-000/animation-000 | 0/0/1000/1/EXACT |
| 2/STAGE_2 (ACTIVE) | 1000–3500 | P123.stage.2 | rpct00_att_battle_35_04 / P123.animation.2 | 4219983/stage-001/animation-000 | 0/0/2500/1/HOLD_LAST_POSE |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P123.logic.1 | L507 트리거_대미지_넉백 | TRIGGER/ENTER_AREA | 2050+1000 / True | S: L134:MAX_HP_PERCENT_DAMAGE |
| P123.logic.2 | L548 트리거_가장가까운플레이어방향회전 | TRIGGER/BOSS_TRACK_TARGET | 0+74 / True | — |

<a id="p124"></a>
## P124 · 쿠크세이튼_플레이어추적

`KOUKUSAYDON_G1_PATTERN_124` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=2134ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootHorizontalScale":0}`.
표현 수: `{"SOUND":6}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 2, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1067 | P124.stage.2 | rpct00_run_battle_1 / P124.animation.2 | 1/stage-002/animation-000 | 0/0/1067/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1067–2134 | P124.stage.3 | rpct00_run_battle_1 / P124.animation.3 | 1/stage-002/animation-000 | 0/0/1067/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P124.logic.1 | L132 세이튼_플레이어추적_이동 | DURATION/BOSS_TRACK_TARGET | 0+2134 / True | — |
상위 참조: P88 via patternOccurrence; P91 via patternOccurrence; P92 via patternOccurrence; P93 via patternOccurrence.

<a id="p125"></a>
## P125 · 쿠크세이튼_1초추적

`KOUKUSAYDON_G1_PATTERN_125` · GATE3 · `MN_RPCT_05` · `boss.koukusaydon.g3.saydon`
저작 durationMs=None / stage 합=1000ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootHorizontalScale":0}`.
표현 수: `{"SOUND":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1000 | P125.stage.2 | rpct00_run_battle_1 / P125.animation.2 | 1/stage-002/animation-000 | 0/0/1000/1/LOOP_TO_WINDOW |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P125.logic.1 | L132 세이튼_플레이어추적_이동 | DURATION/BOSS_TRACK_TARGET | 0+1000 / True | — |
상위 참조: P88 via patternOccurrence.

<a id="p126"></a>
## P126 · 앵콜세이튼_플레이어추적

`KOUKUSAYDON_G1_PATTERN_126` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=None / stage 합=2134ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootHorizontalScale":0}`.
표현 수: `{"SOUND":6}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BOSS_TRACK_TARGET":1}`.
서버 stage 2, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_2 (ACTIVE) | 0–1067 | P126.stage.2 | rpct00_run_battle_1 / P126.animation.2 | 1/stage-002/animation-000 | 0/0/1067/1/EXACT |
| 2/STAGE_3 (ACTIVE) | 1067–2134 | P126.stage.3 | rpct00_run_battle_1 / P126.animation.3 | 1/stage-002/animation-000 | 0/0/1067/1/EXACT |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P126.logic.1 | L132 세이튼_플레이어추적_이동 | DURATION/BOSS_TRACK_TARGET | 0+2134 / True | — |

<a id="p127"></a>
## P127 · 빙고_블랙홀_첫클립

`KOUKUSAYDON_G1_PATTERN_127` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=5400 / stage 합=5400ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootVerticalScale":0,"animationRootHorizontalScale":0}`.
표현 수: `{"SOUND":3}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_33 (ACTIVE) | 0–5400 | P127.stage.33 | rpct00_att_battle_12_02 / P127.animation.33 | 4219927/stage-002/animation-000 | 0/0/5400/1/HOLD_LAST_POSE |
상위 참조: P107 via patternOccurrence.

<a id="p128"></a>
## P128 · 빙고_블랙홀_13초_폭발

`KOUKUSAYDON_G1_PATTERN_128` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=20800 / stage 합=20800ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootVerticalScale":0,"animationRootHorizontalScale":0}`.
표현 수: `{"EFFECT":2,"SOUND":7}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BINGO_DETONATION":1}`.
서버 stage 5, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_34 (ACTIVE) | 0–13000 | P128.stage.34 | rpct00_att_battle_12_04 / P128.animation.34 | 4219927/stage-003/animation-000 | 0/0/13000/1/LOOP_TO_WINDOW; sourceEnd=1042 |
| 2/STAGE_35 (ACTIVE) | 13000–13200 | P128.stage.35 | rpct00_att_battle_12_05 / P128.animation.35 | 4219927/stage-004/animation-000 | 0/0/200/1/HOLD_LAST_POSE |
| 3/STAGE_36 (RECOVERY) | 13200–14367 | P128.stage.36 | rpct00_dmg_critical_start_1 / P128.animation.36 | 4219927/stage-006/animation-000 | 0/0/1167/1/HOLD_LAST_POSE |
| 4/STAGE_37 (RECOVERY) | 14367–19467 | P128.stage.37 | rpct00_dmg_critical_loop_1 / P128.animation.37 | 4219927/stage-007/animation-000 | 0/0/5100/1/LOOP_TO_WINDOW |
| 5/STAGE_38 (RECOVERY) | 19467–20800 | P128.stage.38 | rpct00_dmg_critical_end_1 / P128.animation.38 | 4219927/stage-005/animation-000 | 0/0/1333/1/HOLD_LAST_POSE |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P128.logic.1 | L524 빙고_블랙홀_폭발_13줄 | TRIGGER/BINGO_DETONATION | 13000+34 / True | — |
상위 참조: P107 via patternOccurrence.

<a id="p129"></a>
## P129 · 빙고_보드시작

`KOUKUSAYDON_G1_PATTERN_129` · BINGO · `MN_RPCT_05` · `boss.koukusaydon.bingo.saydon`
저작 durationMs=34 / stage 합=34ms / category=MECHANIC / 투영=예.
상태/이동 설정: `{"resetBossToSpawn":false,"animationRootVerticalScale":0,"animationRootHorizontalScale":0}`.
표현 수: `{}`; World 0, Summon 0, Scene 0.
서버 창 종류: `{}`; mechanic trigger: `{"BINGO_BOARD":1}`.
서버 stage 1, rootMotion 보유 0; worldSequences 0.

| 순서/Stage | 누적 구간 ms | action ID | clip / animation ID | source | offset / sourceStart / playMs / rate / end |
|---|---|---|---|---|---|
| 1/STAGE_1 (ACTIVE) | 0–34 | P129.stage.1 | — / — | —/—/— | —/—/—/—/— |

| occurrence | 정의/이름 | 종류 | 시작+길이 ms / enabled | S / F / T 결과 |
|---|---|---|---|---|
| P129.logic.1 | L99 빙고_바닥_망치생성 | DURATION/BINGO_BOARD | 0+34 / True | — |

## Bundle 전체

| Bundle | Gate/이름 | 구성 패턴@시작 ms | 투영/이유 |
|---|---|---|---|
| koukusaydon.bundle.1 | GATE2 / 세이튼등장_동시 | P8@0; P9@0 | 예 |
| koukusaydon.bundle.2 | GATE2 / 파1빨2_동시 | P10@0; P11@0 | 예 |
| koukusaydon.bundle.3 | GATE2 / 조커찾기_동시 | P12@0; P13@0 | 예 |
| koukusaydon.bundle.4 | GATE2 / 카드미로_동시 | P28@0 | 예 |
| koukusaydon.bundle.5 | GATE2 / 쿠크피자_동시 | P25@0; P26@0 | 예 |
| koukusaydon.bundle.6 | GATE2 / 쿠크거미카운터_동시 | P15@0 | 예 |
| koukusaydon.bundle.7 | GATE2 / 거대세이튼잡기_동시 | P17@0 | 예 |
| koukusaydon.bundle.8 | GATE2 / 쿠크레이저_동시 | P20@0; P21@0 | 아니오: Bundle child is unavailable: KOUKUSAYDON_G1_PATTERN_20: PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_20 |
| koukusaydon.bundle.9 | GATE2 / 쿠크휠윈드_동시 | P24@0 | 예 |
| koukusaydon.bundle.10 | GATE2 / 쿠크팡파레_동시 | P23@0 | 예 |
| koukusaydon.bundle.11 | GATE3 / 쿠크세이튼_분신소환_기분나빠 | P51@0 | 아니오: Bundle child is unavailable: KOUKUSAYDON_G1_PATTERN_51: PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_51 |

## 실제 레이드 Flow

각 행의 나열 순서가 저장된 entries 순서다. 동일 패턴의 반복 사용도 그대로 남긴다. group 경계는 포함 범위이다. `—`는 저장 필드가 없다는 뜻으로, 반복 횟수나 HP 조건을 임의로 채우지 않는다.

### GATE1 · koukusaydon.flow.gate1

entries=166, entryGroups=13, loopStartEntryId=`—`
추가 Flow 설정: `{}`.

| 순서 | entry ID | kind/target | 이름 | waitAfterMs | 그룹 |
|---:|---|---|---|---:|---|
| 1 | koukusaydon.flow.gate1.restored.1 | PATTERN/P58 | 세이튼_화염 파동 | 0 | 일반 패턴 반복 160→130줄 |
| 2 | koukusaydon.flow.gate1.restored.2 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 3 | koukusaydon.flow.gate1.restored.3 | PATTERN/P102 | 세이튼_백스텝후불뿜기화염링 | 0 | 일반 패턴 반복 160→130줄 |
| 4 | koukusaydon.flow.gate1.restored.4 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 5 | koukusaydon.flow.gate1.restored.5 | PATTERN/P81 | 세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 160→130줄 |
| 6 | koukusaydon.flow.gate1.restored.6 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 7 | koukusaydon.flow.gate1.restored.7 | PATTERN/P82 | 세이튼_마술쇼_종이비둘기 | 0 | 일반 패턴 반복 160→130줄 |
| 8 | koukusaydon.flow.gate1.restored.8 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 9 | koukusaydon.flow.gate1.restored.9 | PATTERN/P47 | 세이튼_트럼펫장판소환 | 0 | 일반 패턴 반복 160→130줄 |
| 10 | koukusaydon.flow.gate1.restored.10 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 11 | koukusaydon.flow.gate1.restored.11 | PATTERN/P100 | 세이튼_정면바람방구 | 0 | 일반 패턴 반복 160→130줄 |
| 12 | koukusaydon.flow.gate1.restored.12 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 13 | koukusaydon.flow.gate1.restored.13 | PATTERN/P80 | 세이튼_돌진_카운터 | 0 | 일반 패턴 반복 160→130줄 |
| 14 | koukusaydon.flow.gate1.restored.13.follow.1s | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 160→130줄 |
| 15 | koukusaydon.flow.gate1.restored.14 | PATTERN/P103 | 세이튼_카드비 | 0 | 일반 패턴 반복 160→130줄 |
| 16 | koukusaydon.flow.gate1.restored.15 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 17 | koukusaydon.flow.gate1.restored.16 | PATTERN/P48 | 세이튼_빙글빙글돌며카드던지기 | 0 | 일반 패턴 반복 160→130줄 |
| 18 | koukusaydon.flow.gate1.restored.17 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 160→130줄 |
| 19 | koukusaydon.flow.gate1.restored.18 | PATTERN/P79 | 세이튼_공먹고휘리릭_노란장판펑펑 | 0 | 일반 패턴 반복 160→130줄 |
| 20 | koukusaydon.flow.gate1.restored.19 | PATTERN/P78 | 세이튼_카드맞추기_주사위패턴 | 0 | 일반 패턴 반복 160→130줄 |
| 21 | koukusaydon.flow.gate1.restored.20 | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 160→130줄 |
| 22 | koukusaydon.flow.gate1.hp.160-130.entry.21 | PATTERN/P83 | 세이튼_쓰리투원투하! | 0 | 일반 패턴 반복 160→130줄 |
| 23 | koukusaydon.flow.gate1.restored.21 | PATTERN/P1 | 세이튼_무력화 시작 | 0 | 130줄 세이튼_무력화 시작 |
| 24 | koukusaydon.flow.gate1.restored.22 | PATTERN/P104 | 세이튼_1초추적 | 0 | 130줄 세이튼_무력화 시작 |
| 25 | koukusaydon.flow.gate1.hp.130-110.entry.01 | PATTERN/P58 | 세이튼_화염 파동 | 0 | 일반 패턴 반복 130→110줄 |
| 26 | koukusaydon.flow.gate1.hp.130-110.entry.02 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 27 | koukusaydon.flow.gate1.hp.130-110.entry.03 | PATTERN/P102 | 세이튼_백스텝후불뿜기화염링 | 0 | 일반 패턴 반복 130→110줄 |
| 28 | koukusaydon.flow.gate1.hp.130-110.entry.04 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 29 | koukusaydon.flow.gate1.hp.130-110.entry.05 | PATTERN/P81 | 세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 130→110줄 |
| 30 | koukusaydon.flow.gate1.hp.130-110.entry.06 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 31 | koukusaydon.flow.gate1.hp.130-110.entry.07 | PATTERN/P82 | 세이튼_마술쇼_종이비둘기 | 0 | 일반 패턴 반복 130→110줄 |
| 32 | koukusaydon.flow.gate1.hp.130-110.entry.08 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 33 | koukusaydon.flow.gate1.hp.130-110.entry.09 | PATTERN/P47 | 세이튼_트럼펫장판소환 | 0 | 일반 패턴 반복 130→110줄 |
| 34 | koukusaydon.flow.gate1.hp.130-110.entry.10 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 35 | koukusaydon.flow.gate1.hp.130-110.entry.11 | PATTERN/P100 | 세이튼_정면바람방구 | 0 | 일반 패턴 반복 130→110줄 |
| 36 | koukusaydon.flow.gate1.hp.130-110.entry.12 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 37 | koukusaydon.flow.gate1.hp.130-110.entry.13 | PATTERN/P80 | 세이튼_돌진_카운터 | 0 | 일반 패턴 반복 130→110줄 |
| 38 | koukusaydon.flow.gate1.hp.130-110.entry.13.follow.1s | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 130→110줄 |
| 39 | koukusaydon.flow.gate1.hp.130-110.entry.14 | PATTERN/P103 | 세이튼_카드비 | 0 | 일반 패턴 반복 130→110줄 |
| 40 | koukusaydon.flow.gate1.hp.130-110.entry.15 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 41 | koukusaydon.flow.gate1.hp.130-110.entry.16 | PATTERN/P48 | 세이튼_빙글빙글돌며카드던지기 | 0 | 일반 패턴 반복 130→110줄 |
| 42 | koukusaydon.flow.gate1.hp.130-110.entry.17 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 130→110줄 |
| 43 | koukusaydon.flow.gate1.hp.130-110.entry.18 | PATTERN/P79 | 세이튼_공먹고휘리릭_노란장판펑펑 | 0 | 일반 패턴 반복 130→110줄 |
| 44 | koukusaydon.flow.gate1.hp.130-110.entry.19 | PATTERN/P78 | 세이튼_카드맞추기_주사위패턴 | 0 | 일반 패턴 반복 130→110줄 |
| 45 | koukusaydon.flow.gate1.hp.130-110.entry.20 | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 130→110줄 |
| 46 | koukusaydon.flow.gate1.hp.130-110.entry.21 | PATTERN/P83 | 세이튼_쓰리투원투하! | 0 | 일반 패턴 반복 130→110줄 |
| 47 | koukusaydon.flow.gate1.restored.23 | PATTERN/P2 | 세이튼_진짜세이튼찾기 | 0 | 110줄 세이튼_진짜세이튼찾기 |
| 48 | koukusaydon.flow.gate1.restored.24 | PATTERN/P104 | 세이튼_1초추적 | 0 | 110줄 세이튼_진짜세이튼찾기 |
| 49 | koukusaydon.flow.gate1.hp.110-85.entry.01 | PATTERN/P58 | 세이튼_화염 파동 | 0 | 일반 패턴 반복 110→85줄 |
| 50 | koukusaydon.flow.gate1.hp.110-85.entry.02 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 51 | koukusaydon.flow.gate1.hp.110-85.entry.03 | PATTERN/P102 | 세이튼_백스텝후불뿜기화염링 | 0 | 일반 패턴 반복 110→85줄 |
| 52 | koukusaydon.flow.gate1.hp.110-85.entry.04 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 53 | koukusaydon.flow.gate1.hp.110-85.entry.05 | PATTERN/P81 | 세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 110→85줄 |
| 54 | koukusaydon.flow.gate1.hp.110-85.entry.06 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 55 | koukusaydon.flow.gate1.hp.110-85.entry.07 | PATTERN/P82 | 세이튼_마술쇼_종이비둘기 | 0 | 일반 패턴 반복 110→85줄 |
| 56 | koukusaydon.flow.gate1.hp.110-85.entry.08 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 57 | koukusaydon.flow.gate1.hp.110-85.entry.09 | PATTERN/P47 | 세이튼_트럼펫장판소환 | 0 | 일반 패턴 반복 110→85줄 |
| 58 | koukusaydon.flow.gate1.hp.110-85.entry.10 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 59 | koukusaydon.flow.gate1.hp.110-85.entry.11 | PATTERN/P100 | 세이튼_정면바람방구 | 0 | 일반 패턴 반복 110→85줄 |
| 60 | koukusaydon.flow.gate1.hp.110-85.entry.12 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 61 | koukusaydon.flow.gate1.hp.110-85.entry.13 | PATTERN/P80 | 세이튼_돌진_카운터 | 0 | 일반 패턴 반복 110→85줄 |
| 62 | koukusaydon.flow.gate1.hp.110-85.entry.13.follow.1s | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 110→85줄 |
| 63 | koukusaydon.flow.gate1.hp.110-85.entry.14 | PATTERN/P103 | 세이튼_카드비 | 0 | 일반 패턴 반복 110→85줄 |
| 64 | koukusaydon.flow.gate1.hp.110-85.entry.15 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 65 | koukusaydon.flow.gate1.hp.110-85.entry.16 | PATTERN/P48 | 세이튼_빙글빙글돌며카드던지기 | 0 | 일반 패턴 반복 110→85줄 |
| 66 | koukusaydon.flow.gate1.hp.110-85.entry.17 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 110→85줄 |
| 67 | koukusaydon.flow.gate1.hp.110-85.entry.18 | PATTERN/P79 | 세이튼_공먹고휘리릭_노란장판펑펑 | 0 | 일반 패턴 반복 110→85줄 |
| 68 | koukusaydon.flow.gate1.hp.110-85.entry.19 | PATTERN/P78 | 세이튼_카드맞추기_주사위패턴 | 0 | 일반 패턴 반복 110→85줄 |
| 69 | koukusaydon.flow.gate1.hp.110-85.entry.20 | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 110→85줄 |
| 70 | koukusaydon.flow.gate1.hp.110-85.entry.21 | PATTERN/P83 | 세이튼_쓰리투원투하! | 0 | 일반 패턴 반복 110→85줄 |
| 71 | koukusaydon.flow.gate1.restored.25 | PATTERN/P6 | 세이튼_댄스타임 | 0 | 85줄 세이튼_댄스타임 |
| 72 | koukusaydon.flow.gate1.restored.26 | PATTERN/P104 | 세이튼_1초추적 | 0 | 85줄 세이튼_댄스타임 |
| 73 | koukusaydon.flow.gate1.hp.85-60.entry.01 | PATTERN/P58 | 세이튼_화염 파동 | 0 | 일반 패턴 반복 85→60줄 |
| 74 | koukusaydon.flow.gate1.hp.85-60.entry.02 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 75 | koukusaydon.flow.gate1.hp.85-60.entry.03 | PATTERN/P102 | 세이튼_백스텝후불뿜기화염링 | 0 | 일반 패턴 반복 85→60줄 |
| 76 | koukusaydon.flow.gate1.hp.85-60.entry.04 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 77 | koukusaydon.flow.gate1.hp.85-60.entry.05 | PATTERN/P81 | 세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 85→60줄 |
| 78 | koukusaydon.flow.gate1.hp.85-60.entry.06 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 79 | koukusaydon.flow.gate1.hp.85-60.entry.07 | PATTERN/P82 | 세이튼_마술쇼_종이비둘기 | 0 | 일반 패턴 반복 85→60줄 |
| 80 | koukusaydon.flow.gate1.hp.85-60.entry.08 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 81 | koukusaydon.flow.gate1.hp.85-60.entry.09 | PATTERN/P47 | 세이튼_트럼펫장판소환 | 0 | 일반 패턴 반복 85→60줄 |
| 82 | koukusaydon.flow.gate1.hp.85-60.entry.10 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 83 | koukusaydon.flow.gate1.hp.85-60.entry.11 | PATTERN/P100 | 세이튼_정면바람방구 | 0 | 일반 패턴 반복 85→60줄 |
| 84 | koukusaydon.flow.gate1.hp.85-60.entry.12 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 85 | koukusaydon.flow.gate1.hp.85-60.entry.13 | PATTERN/P80 | 세이튼_돌진_카운터 | 0 | 일반 패턴 반복 85→60줄 |
| 86 | koukusaydon.flow.gate1.hp.85-60.entry.13.follow.1s | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 85→60줄 |
| 87 | koukusaydon.flow.gate1.hp.85-60.entry.14 | PATTERN/P103 | 세이튼_카드비 | 0 | 일반 패턴 반복 85→60줄 |
| 88 | koukusaydon.flow.gate1.hp.85-60.entry.15 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 89 | koukusaydon.flow.gate1.hp.85-60.entry.16 | PATTERN/P48 | 세이튼_빙글빙글돌며카드던지기 | 0 | 일반 패턴 반복 85→60줄 |
| 90 | koukusaydon.flow.gate1.hp.85-60.entry.17 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 85→60줄 |
| 91 | koukusaydon.flow.gate1.hp.85-60.entry.18 | PATTERN/P79 | 세이튼_공먹고휘리릭_노란장판펑펑 | 0 | 일반 패턴 반복 85→60줄 |
| 92 | koukusaydon.flow.gate1.hp.85-60.entry.19 | PATTERN/P78 | 세이튼_카드맞추기_주사위패턴 | 0 | 일반 패턴 반복 85→60줄 |
| 93 | koukusaydon.flow.gate1.hp.85-60.entry.20 | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 85→60줄 |
| 94 | koukusaydon.flow.gate1.hp.85-60.entry.21 | PATTERN/P83 | 세이튼_쓰리투원투하! | 0 | 일반 패턴 반복 85→60줄 |
| 95 | koukusaydon.flow.gate1.mechanic.hp60.entry.01 | PATTERN/P1 | 세이튼_무력화 시작 | 0 | 60줄 세이튼_무력화 시작 |
| 96 | koukusaydon.flow.gate1.mechanic.hp60.entry.02 | PATTERN/P104 | 세이튼_1초추적 | 0 | 60줄 세이튼_무력화 시작 |
| 97 | koukusaydon.flow.gate1.hp.60-50.entry.01 | PATTERN/P58 | 세이튼_화염 파동 | 0 | 일반 패턴 반복 60→50줄 |
| 98 | koukusaydon.flow.gate1.hp.60-50.entry.02 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 99 | koukusaydon.flow.gate1.hp.60-50.entry.03 | PATTERN/P102 | 세이튼_백스텝후불뿜기화염링 | 0 | 일반 패턴 반복 60→50줄 |
| 100 | koukusaydon.flow.gate1.hp.60-50.entry.04 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 101 | koukusaydon.flow.gate1.hp.60-50.entry.05 | PATTERN/P81 | 세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 60→50줄 |
| 102 | koukusaydon.flow.gate1.hp.60-50.entry.06 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 103 | koukusaydon.flow.gate1.hp.60-50.entry.07 | PATTERN/P82 | 세이튼_마술쇼_종이비둘기 | 0 | 일반 패턴 반복 60→50줄 |
| 104 | koukusaydon.flow.gate1.hp.60-50.entry.08 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 105 | koukusaydon.flow.gate1.hp.60-50.entry.09 | PATTERN/P47 | 세이튼_트럼펫장판소환 | 0 | 일반 패턴 반복 60→50줄 |
| 106 | koukusaydon.flow.gate1.hp.60-50.entry.10 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 107 | koukusaydon.flow.gate1.hp.60-50.entry.11 | PATTERN/P100 | 세이튼_정면바람방구 | 0 | 일반 패턴 반복 60→50줄 |
| 108 | koukusaydon.flow.gate1.hp.60-50.entry.12 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 109 | koukusaydon.flow.gate1.hp.60-50.entry.13 | PATTERN/P80 | 세이튼_돌진_카운터 | 0 | 일반 패턴 반복 60→50줄 |
| 110 | koukusaydon.flow.gate1.hp.60-50.entry.13.follow.1s | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 60→50줄 |
| 111 | koukusaydon.flow.gate1.hp.60-50.entry.14 | PATTERN/P103 | 세이튼_카드비 | 0 | 일반 패턴 반복 60→50줄 |
| 112 | koukusaydon.flow.gate1.hp.60-50.entry.15 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 113 | koukusaydon.flow.gate1.hp.60-50.entry.16 | PATTERN/P48 | 세이튼_빙글빙글돌며카드던지기 | 0 | 일반 패턴 반복 60→50줄 |
| 114 | koukusaydon.flow.gate1.hp.60-50.entry.17 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 60→50줄 |
| 115 | koukusaydon.flow.gate1.hp.60-50.entry.18 | PATTERN/P79 | 세이튼_공먹고휘리릭_노란장판펑펑 | 0 | 일반 패턴 반복 60→50줄 |
| 116 | koukusaydon.flow.gate1.hp.60-50.entry.19 | PATTERN/P78 | 세이튼_카드맞추기_주사위패턴 | 0 | 일반 패턴 반복 60→50줄 |
| 117 | koukusaydon.flow.gate1.hp.60-50.entry.20 | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 60→50줄 |
| 118 | koukusaydon.flow.gate1.hp.60-50.entry.21 | PATTERN/P83 | 세이튼_쓰리투원투하! | 0 | 일반 패턴 반복 60→50줄 |
| 119 | koukusaydon.flow.gate1.restored.27 | PATTERN/P7 | 세이튼_룰렛 | 0 | 50줄 세이튼_룰렛 |
| 120 | koukusaydon.flow.gate1.restored.28 | PATTERN/P104 | 세이튼_1초추적 | 0 | 50줄 세이튼_룰렛 |
| 121 | koukusaydon.flow.gate1.hp.50-30.entry.01 | PATTERN/P58 | 세이튼_화염 파동 | 0 | 일반 패턴 반복 50→30줄 |
| 122 | koukusaydon.flow.gate1.hp.50-30.entry.02 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 123 | koukusaydon.flow.gate1.hp.50-30.entry.03 | PATTERN/P102 | 세이튼_백스텝후불뿜기화염링 | 0 | 일반 패턴 반복 50→30줄 |
| 124 | koukusaydon.flow.gate1.hp.50-30.entry.04 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 125 | koukusaydon.flow.gate1.hp.50-30.entry.05 | PATTERN/P81 | 세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 50→30줄 |
| 126 | koukusaydon.flow.gate1.hp.50-30.entry.06 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 127 | koukusaydon.flow.gate1.hp.50-30.entry.07 | PATTERN/P82 | 세이튼_마술쇼_종이비둘기 | 0 | 일반 패턴 반복 50→30줄 |
| 128 | koukusaydon.flow.gate1.hp.50-30.entry.08 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 129 | koukusaydon.flow.gate1.hp.50-30.entry.09 | PATTERN/P47 | 세이튼_트럼펫장판소환 | 0 | 일반 패턴 반복 50→30줄 |
| 130 | koukusaydon.flow.gate1.hp.50-30.entry.10 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 131 | koukusaydon.flow.gate1.hp.50-30.entry.11 | PATTERN/P100 | 세이튼_정면바람방구 | 0 | 일반 패턴 반복 50→30줄 |
| 132 | koukusaydon.flow.gate1.hp.50-30.entry.12 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 133 | koukusaydon.flow.gate1.hp.50-30.entry.13 | PATTERN/P80 | 세이튼_돌진_카운터 | 0 | 일반 패턴 반복 50→30줄 |
| 134 | koukusaydon.flow.gate1.hp.50-30.entry.13.follow.1s | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 50→30줄 |
| 135 | koukusaydon.flow.gate1.hp.50-30.entry.14 | PATTERN/P103 | 세이튼_카드비 | 0 | 일반 패턴 반복 50→30줄 |
| 136 | koukusaydon.flow.gate1.hp.50-30.entry.15 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 137 | koukusaydon.flow.gate1.hp.50-30.entry.16 | PATTERN/P48 | 세이튼_빙글빙글돌며카드던지기 | 0 | 일반 패턴 반복 50→30줄 |
| 138 | koukusaydon.flow.gate1.hp.50-30.entry.17 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 50→30줄 |
| 139 | koukusaydon.flow.gate1.hp.50-30.entry.18 | PATTERN/P79 | 세이튼_공먹고휘리릭_노란장판펑펑 | 0 | 일반 패턴 반복 50→30줄 |
| 140 | koukusaydon.flow.gate1.hp.50-30.entry.19 | PATTERN/P78 | 세이튼_카드맞추기_주사위패턴 | 0 | 일반 패턴 반복 50→30줄 |
| 141 | koukusaydon.flow.gate1.hp.50-30.entry.20 | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 50→30줄 |
| 142 | koukusaydon.flow.gate1.hp.50-30.entry.21 | PATTERN/P83 | 세이튼_쓰리투원투하! | 0 | 일반 패턴 반복 50→30줄 |
| 143 | koukusaydon.flow.gate1.mechanic.hp30.entry.01 | PATTERN/P2 | 세이튼_진짜세이튼찾기 | 0 | 30줄 세이튼_진짜세이튼찾기 |
| 144 | koukusaydon.flow.gate1.mechanic.hp30.entry.02 | PATTERN/P104 | 세이튼_1초추적 | 0 | 30줄 세이튼_진짜세이튼찾기 |
| 145 | koukusaydon.flow.gate1.hp.30-0.entry.01 | PATTERN/P58 | 세이튼_화염 파동 | 0 | 일반 패턴 반복 30→0줄 |
| 146 | koukusaydon.flow.gate1.hp.30-0.entry.02 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 147 | koukusaydon.flow.gate1.hp.30-0.entry.03 | PATTERN/P102 | 세이튼_백스텝후불뿜기화염링 | 0 | 일반 패턴 반복 30→0줄 |
| 148 | koukusaydon.flow.gate1.hp.30-0.entry.04 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 149 | koukusaydon.flow.gate1.hp.30-0.entry.05 | PATTERN/P81 | 세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 30→0줄 |
| 150 | koukusaydon.flow.gate1.hp.30-0.entry.06 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 151 | koukusaydon.flow.gate1.hp.30-0.entry.07 | PATTERN/P82 | 세이튼_마술쇼_종이비둘기 | 0 | 일반 패턴 반복 30→0줄 |
| 152 | koukusaydon.flow.gate1.hp.30-0.entry.08 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 153 | koukusaydon.flow.gate1.hp.30-0.entry.09 | PATTERN/P47 | 세이튼_트럼펫장판소환 | 0 | 일반 패턴 반복 30→0줄 |
| 154 | koukusaydon.flow.gate1.hp.30-0.entry.10 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 155 | koukusaydon.flow.gate1.hp.30-0.entry.11 | PATTERN/P100 | 세이튼_정면바람방구 | 0 | 일반 패턴 반복 30→0줄 |
| 156 | koukusaydon.flow.gate1.hp.30-0.entry.12 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 157 | koukusaydon.flow.gate1.hp.30-0.entry.13 | PATTERN/P80 | 세이튼_돌진_카운터 | 0 | 일반 패턴 반복 30→0줄 |
| 158 | koukusaydon.flow.gate1.hp.30-0.entry.13.follow.1s | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 30→0줄 |
| 159 | koukusaydon.flow.gate1.hp.30-0.entry.14 | PATTERN/P103 | 세이튼_카드비 | 0 | 일반 패턴 반복 30→0줄 |
| 160 | koukusaydon.flow.gate1.hp.30-0.entry.15 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 161 | koukusaydon.flow.gate1.hp.30-0.entry.16 | PATTERN/P48 | 세이튼_빙글빙글돌며카드던지기 | 0 | 일반 패턴 반복 30→0줄 |
| 162 | koukusaydon.flow.gate1.hp.30-0.entry.17 | PATTERN/P101 | 세이튼_플레이어추적 | 0 | 일반 패턴 반복 30→0줄 |
| 163 | koukusaydon.flow.gate1.hp.30-0.entry.18 | PATTERN/P79 | 세이튼_공먹고휘리릭_노란장판펑펑 | 0 | 일반 패턴 반복 30→0줄 |
| 164 | koukusaydon.flow.gate1.hp.30-0.entry.19 | PATTERN/P78 | 세이튼_카드맞추기_주사위패턴 | 0 | 일반 패턴 반복 30→0줄 |
| 165 | koukusaydon.flow.gate1.hp.30-0.entry.20 | PATTERN/P104 | 세이튼_1초추적 | 0 | 일반 패턴 반복 30→0줄 |
| 166 | koukusaydon.flow.gate1.hp.30-0.entry.21 | PATTERN/P83 | 세이튼_쓰리투원투하! | 0 | 일반 패턴 반복 30→0줄 |

| group ID/이름 | 시작 entry (순서) | 끝 entry (순서) | repeatUntilHealthBars | transitionAt | repeatCount |
|---|---|---|---:|---|---:|
| koukusaydon.flow.gate1.group.hp.160-130 / 일반 패턴 반복 160→130줄 | koukusaydon.flow.gate1.restored.1 (1) | koukusaydon.flow.gate1.hp.160-130.entry.21 (22) | 130 | PATTERN_END | — |
| koukusaydon.flow.gate1.group.mechanic.hp130 / 130줄 세이튼_무력화 시작 | koukusaydon.flow.gate1.restored.21 (23) | koukusaydon.flow.gate1.restored.22 (24) | — | — | — |
| koukusaydon.flow.gate1.group.hp.130-110 / 일반 패턴 반복 130→110줄 | koukusaydon.flow.gate1.hp.130-110.entry.01 (25) | koukusaydon.flow.gate1.hp.130-110.entry.21 (46) | 110 | PATTERN_END | — |
| koukusaydon.flow.gate1.group.mechanic.hp110 / 110줄 세이튼_진짜세이튼찾기 | koukusaydon.flow.gate1.restored.23 (47) | koukusaydon.flow.gate1.restored.24 (48) | — | — | — |
| koukusaydon.flow.gate1.group.hp.110-85 / 일반 패턴 반복 110→85줄 | koukusaydon.flow.gate1.hp.110-85.entry.01 (49) | koukusaydon.flow.gate1.hp.110-85.entry.21 (70) | 85 | PATTERN_END | — |
| koukusaydon.flow.gate1.group.mechanic.hp85 / 85줄 세이튼_댄스타임 | koukusaydon.flow.gate1.restored.25 (71) | koukusaydon.flow.gate1.restored.26 (72) | — | — | — |
| koukusaydon.flow.gate1.group.hp.85-60 / 일반 패턴 반복 85→60줄 | koukusaydon.flow.gate1.hp.85-60.entry.01 (73) | koukusaydon.flow.gate1.hp.85-60.entry.21 (94) | 60 | PATTERN_END | — |
| koukusaydon.flow.gate1.group.mechanic.hp60 / 60줄 세이튼_무력화 시작 | koukusaydon.flow.gate1.mechanic.hp60.entry.01 (95) | koukusaydon.flow.gate1.mechanic.hp60.entry.02 (96) | — | — | — |
| koukusaydon.flow.gate1.group.hp.60-50 / 일반 패턴 반복 60→50줄 | koukusaydon.flow.gate1.hp.60-50.entry.01 (97) | koukusaydon.flow.gate1.hp.60-50.entry.21 (118) | 50 | PATTERN_END | — |
| koukusaydon.flow.gate1.group.mechanic.hp50 / 50줄 세이튼_룰렛 | koukusaydon.flow.gate1.restored.27 (119) | koukusaydon.flow.gate1.restored.28 (120) | — | — | — |
| koukusaydon.flow.gate1.group.hp.50-30 / 일반 패턴 반복 50→30줄 | koukusaydon.flow.gate1.hp.50-30.entry.01 (121) | koukusaydon.flow.gate1.hp.50-30.entry.21 (142) | 30 | PATTERN_END | — |
| koukusaydon.flow.gate1.group.mechanic.hp30 / 30줄 세이튼_진짜세이튼찾기 | koukusaydon.flow.gate1.mechanic.hp30.entry.01 (143) | koukusaydon.flow.gate1.mechanic.hp30.entry.02 (144) | — | — | — |
| koukusaydon.flow.gate1.group.hp.30-0 / 일반 패턴 반복 30→0줄 | koukusaydon.flow.gate1.hp.30-0.entry.01 (145) | koukusaydon.flow.gate1.hp.30-0.entry.21 (166) | 0 | PATTERN_END | — |

### GATE2 · koukusaydon.flow.gate2

entries=85, entryGroups=13, loopStartEntryId=`—`
추가 Flow 설정: `{}`.

| 순서 | entry ID | kind/target | 이름 | waitAfterMs | 그룹 |
|---:|---|---|---|---:|---|
| 1 | koukusaydon.flow.gate2.restore.initial.7f4de4493f141aa9 | PATTERN/P106 | 쿠크_저글링액션 | 1000 | 일반 패턴 반복 140→125줄 |
| 2 | koukusaydon.flow.gate2.restore.initial.7f4de4493f141aa9.trumpet | PATTERN/P105 | 쿠크_나팔액션 | 1000 | 일반 패턴 반복 140→125줄 |
| 3 | koukusaydon.flow.gate2.raid.10 | PATTERN/P85 | 쿠크_슈퍼바주카 | 1000 | 일반 패턴 반복 140→125줄 |
| 4 | koukusaydon.flow.gate2.entry.4 | BUNDLE/koukusaydon.bundle.6 | 쿠크거미카운터_동시 | 1000 | 일반 패턴 반복 140→125줄 |
| 5 | koukusaydon.flow.gate2.entry.4.fanfare | PATTERN/P23 | 쿠크_팡파레 | 1000 | 일반 패턴 반복 140→125줄 |
| 6 | koukusaydon.flow.gate2.entry.7 | PATTERN/P21 | 쿠크_레이저 | 1000 | 일반 패턴 반복 140→125줄 |
| 7 | koukusaydon.flow.gate2.entry.8 | BUNDLE/koukusaydon.bundle.9 | 쿠크휠윈드_동시 | 1000 | 일반 패턴 반복 140→125줄 |
| 8 | koukusaydon.flow.gate2.entry.1 | BUNDLE/koukusaydon.bundle.1 | 세이튼등장_동시 | 1000 | 125줄 세이튼 등장 |
| 9 | koukusaydon.flow.gate2.hp.125-110.entry.02 | PATTERN/P106 | 쿠크_저글링액션 | 1000 | 일반 패턴 반복 125→110줄 |
| 10 | koukusaydon.flow.gate2.hp.125-110.entry.02.trumpet | PATTERN/P105 | 쿠크_나팔액션 | 1000 | 일반 패턴 반복 125→110줄 |
| 11 | koukusaydon.flow.gate2.restore.loop.5 | PATTERN/P85 | 쿠크_슈퍼바주카 | 1000 | 일반 패턴 반복 125→110줄 |
| 12 | koukusaydon.flow.gate2.hp.125-110.entry.05 | BUNDLE/koukusaydon.bundle.6 | 쿠크거미카운터_동시 | 1000 | 일반 패턴 반복 125→110줄 |
| 13 | koukusaydon.flow.gate2.hp.125-110.entry.05.fanfare | PATTERN/P23 | 쿠크_팡파레 | 1000 | 일반 패턴 반복 125→110줄 |
| 14 | koukusaydon.flow.gate2.hp.125-110.entry.06 | PATTERN/P21 | 쿠크_레이저 | 1000 | 일반 패턴 반복 125→110줄 |
| 15 | koukusaydon.flow.gate2.restore.loop.3 | BUNDLE/koukusaydon.bundle.9 | 쿠크휠윈드_동시 | 1000 | 일반 패턴 반복 125→110줄 |
| 16 | koukusaydon.flow.gate2.group.hp.125-110.grab | PATTERN/P17 | 대형세이튼_잡기 | 1000 | 일반 패턴 반복 125→110줄 |
| 17 | koukusaydon.flow.gate2.restore.initial.e4be643321af977a | PATTERN/P99 | 대형세이튼_불어날리기 | 1000 | 일반 패턴 반복 125→110줄 |
| 18 | koukusaydon.flow.gate2.raid.11 | PATTERN/P86 | 대형세이튼_두번내려치기 | 1000 | 일반 패턴 반복 125→110줄 |
| 19 | koukusaydon.flow.gate2.raid.12 | PATTERN/P87 | 대형세이튼_3번내려치기 | 1000 | 일반 패턴 반복 125→110줄 |
| 20 | koukusaydon.flow.gate2.restore.loop.6 | PATTERN/P27 | 대형세이튼_불뿜기 | 1000 | 일반 패턴 반복 125→110줄 |
| 21 | koukusaydon.flow.gate2.entry.2 | BUNDLE/koukusaydon.bundle.2 | 파1빨2_동시 | 1000 | 110줄 파1빨2 |
| 22 | koukusaydon.flow.gate2.hp.110-95.entry.02 | PATTERN/P106 | 쿠크_저글링액션 | 1000 | 일반 패턴 반복 110→95줄 |
| 23 | koukusaydon.flow.gate2.hp.110-95.entry.02.trumpet | PATTERN/P105 | 쿠크_나팔액션 | 1000 | 일반 패턴 반복 110→95줄 |
| 24 | koukusaydon.flow.gate2.hp.110-95.entry.04 | PATTERN/P85 | 쿠크_슈퍼바주카 | 1000 | 일반 패턴 반복 110→95줄 |
| 25 | koukusaydon.flow.gate2.hp.110-95.entry.05 | BUNDLE/koukusaydon.bundle.6 | 쿠크거미카운터_동시 | 1000 | 일반 패턴 반복 110→95줄 |
| 26 | koukusaydon.flow.gate2.hp.110-95.entry.05.fanfare | PATTERN/P23 | 쿠크_팡파레 | 1000 | 일반 패턴 반복 110→95줄 |
| 27 | koukusaydon.flow.gate2.hp.110-95.entry.06 | PATTERN/P21 | 쿠크_레이저 | 1000 | 일반 패턴 반복 110→95줄 |
| 28 | koukusaydon.flow.gate2.hp.110-95.entry.07 | BUNDLE/koukusaydon.bundle.9 | 쿠크휠윈드_동시 | 1000 | 일반 패턴 반복 110→95줄 |
| 29 | koukusaydon.flow.gate2.group.hp.110-95.grab | PATTERN/P17 | 대형세이튼_잡기 | 1000 | 일반 패턴 반복 110→95줄 |
| 30 | koukusaydon.flow.gate2.restore.initial.6dbc5b9a58cfc10d | PATTERN/P99 | 대형세이튼_불어날리기 | 1000 | 일반 패턴 반복 110→95줄 |
| 31 | koukusaydon.flow.gate2.restore.loop.1 | PATTERN/P86 | 대형세이튼_두번내려치기 | 1000 | 일반 패턴 반복 110→95줄 |
| 32 | koukusaydon.flow.gate2.restore.loop.2 | PATTERN/P87 | 대형세이튼_3번내려치기 | 1000 | 일반 패턴 반복 110→95줄 |
| 33 | koukusaydon.flow.gate2.hp.110-95.entry.11 | PATTERN/P27 | 대형세이튼_불뿜기 | 1000 | 일반 패턴 반복 110→95줄 |
| 34 | koukusaydon.flow.gate2.entry.3 | BUNDLE/koukusaydon.bundle.3 | 조커찾기_동시 | 1000 | 95줄 조커 찾기 |
| 35 | koukusaydon.flow.gate2.hp.95-80.entry.02 | PATTERN/P106 | 쿠크_저글링액션 | 1000 | 일반 패턴 반복 95→80줄 |
| 36 | koukusaydon.flow.gate2.hp.95-80.entry.02.trumpet | PATTERN/P105 | 쿠크_나팔액션 | 1000 | 일반 패턴 반복 95→80줄 |
| 37 | koukusaydon.flow.gate2.hp.95-80.entry.04 | PATTERN/P85 | 쿠크_슈퍼바주카 | 1000 | 일반 패턴 반복 95→80줄 |
| 38 | koukusaydon.flow.gate2.hp.95-80.entry.05 | BUNDLE/koukusaydon.bundle.6 | 쿠크거미카운터_동시 | 1000 | 일반 패턴 반복 95→80줄 |
| 39 | koukusaydon.flow.gate2.hp.95-80.entry.05.fanfare | PATTERN/P23 | 쿠크_팡파레 | 1000 | 일반 패턴 반복 95→80줄 |
| 40 | koukusaydon.flow.gate2.hp.95-80.entry.06 | PATTERN/P21 | 쿠크_레이저 | 1000 | 일반 패턴 반복 95→80줄 |
| 41 | koukusaydon.flow.gate2.hp.95-80.entry.07 | BUNDLE/koukusaydon.bundle.9 | 쿠크휠윈드_동시 | 1000 | 일반 패턴 반복 95→80줄 |
| 42 | koukusaydon.flow.gate2.group.hp.95-80.grab | PATTERN/P17 | 대형세이튼_잡기 | 1000 | 일반 패턴 반복 95→80줄 |
| 43 | koukusaydon.flow.gate2.hp.95-80.entry.08 | PATTERN/P99 | 대형세이튼_불어날리기 | 1000 | 일반 패턴 반복 95→80줄 |
| 44 | koukusaydon.flow.gate2.hp.95-80.entry.09 | PATTERN/P86 | 대형세이튼_두번내려치기 | 1000 | 일반 패턴 반복 95→80줄 |
| 45 | koukusaydon.flow.gate2.hp.95-80.entry.10 | PATTERN/P87 | 대형세이튼_3번내려치기 | 1000 | 일반 패턴 반복 95→80줄 |
| 46 | koukusaydon.flow.gate2.hp.95-80.entry.11 | PATTERN/P27 | 대형세이튼_불뿜기 | 1000 | 일반 패턴 반복 95→80줄 |
| 47 | koukusaydon.flow.gate2.entry.10 | PATTERN/P77 | 카드미로연출 | 1000 | 80줄 카드미로 |
| 48 | koukusaydon.flow.gate2.hp.80-55.entry.02 | PATTERN/P106 | 쿠크_저글링액션 | 1000 | 일반 패턴 반복 80→55줄 |
| 49 | koukusaydon.flow.gate2.hp.80-55.entry.02.trumpet | PATTERN/P105 | 쿠크_나팔액션 | 1000 | 일반 패턴 반복 80→55줄 |
| 50 | koukusaydon.flow.gate2.hp.80-55.entry.04 | PATTERN/P85 | 쿠크_슈퍼바주카 | 1000 | 일반 패턴 반복 80→55줄 |
| 51 | koukusaydon.flow.gate2.hp.80-55.entry.05 | BUNDLE/koukusaydon.bundle.6 | 쿠크거미카운터_동시 | 1000 | 일반 패턴 반복 80→55줄 |
| 52 | koukusaydon.flow.gate2.hp.80-55.entry.05.fanfare | PATTERN/P23 | 쿠크_팡파레 | 1000 | 일반 패턴 반복 80→55줄 |
| 53 | koukusaydon.flow.gate2.hp.80-55.entry.06 | PATTERN/P21 | 쿠크_레이저 | 1000 | 일반 패턴 반복 80→55줄 |
| 54 | koukusaydon.flow.gate2.hp.80-55.entry.07 | BUNDLE/koukusaydon.bundle.9 | 쿠크휠윈드_동시 | 1000 | 일반 패턴 반복 80→55줄 |
| 55 | koukusaydon.flow.gate2.group.hp.80-55.grab | PATTERN/P17 | 대형세이튼_잡기 | 1000 | 일반 패턴 반복 80→55줄 |
| 56 | koukusaydon.flow.gate2.hp.80-55.entry.08 | PATTERN/P99 | 대형세이튼_불어날리기 | 1000 | 일반 패턴 반복 80→55줄 |
| 57 | koukusaydon.flow.gate2.hp.80-55.entry.09 | PATTERN/P86 | 대형세이튼_두번내려치기 | 1000 | 일반 패턴 반복 80→55줄 |
| 58 | koukusaydon.flow.gate2.hp.80-55.entry.10 | PATTERN/P87 | 대형세이튼_3번내려치기 | 1000 | 일반 패턴 반복 80→55줄 |
| 59 | koukusaydon.flow.gate2.hp.80-55.entry.11 | PATTERN/P27 | 대형세이튼_불뿜기 | 1000 | 일반 패턴 반복 80→55줄 |
| 60 | koukusaydon.flow.gate2.mechanic.hp55.entry.01 | BUNDLE/koukusaydon.bundle.5 | 쿠크피자_동시 | 0 | 55줄 피자 |
| 61 | koukusaydon.flow.gate2.hp.55-25.entry.02 | PATTERN/P106 | 쿠크_저글링액션 | 1000 | 일반 패턴 반복 55→25줄 |
| 62 | koukusaydon.flow.gate2.hp.55-25.entry.02.trumpet | PATTERN/P105 | 쿠크_나팔액션 | 1000 | 일반 패턴 반복 55→25줄 |
| 63 | koukusaydon.flow.gate2.hp.55-25.entry.04 | PATTERN/P85 | 쿠크_슈퍼바주카 | 1000 | 일반 패턴 반복 55→25줄 |
| 64 | koukusaydon.flow.gate2.hp.55-25.entry.05 | BUNDLE/koukusaydon.bundle.6 | 쿠크거미카운터_동시 | 1000 | 일반 패턴 반복 55→25줄 |
| 65 | koukusaydon.flow.gate2.hp.55-25.entry.05.fanfare | PATTERN/P23 | 쿠크_팡파레 | 1000 | 일반 패턴 반복 55→25줄 |
| 66 | koukusaydon.flow.gate2.hp.55-25.entry.06 | PATTERN/P21 | 쿠크_레이저 | 1000 | 일반 패턴 반복 55→25줄 |
| 67 | koukusaydon.flow.gate2.hp.55-25.entry.07 | BUNDLE/koukusaydon.bundle.9 | 쿠크휠윈드_동시 | 1000 | 일반 패턴 반복 55→25줄 |
| 68 | koukusaydon.flow.gate2.group.hp.55-25.grab | PATTERN/P17 | 대형세이튼_잡기 | 1000 | 일반 패턴 반복 55→25줄 |
| 69 | koukusaydon.flow.gate2.hp.55-25.entry.08 | PATTERN/P99 | 대형세이튼_불어날리기 | 1000 | 일반 패턴 반복 55→25줄 |
| 70 | koukusaydon.flow.gate2.hp.55-25.entry.09 | PATTERN/P86 | 대형세이튼_두번내려치기 | 1000 | 일반 패턴 반복 55→25줄 |
| 71 | koukusaydon.flow.gate2.hp.55-25.entry.10 | PATTERN/P87 | 대형세이튼_3번내려치기 | 1000 | 일반 패턴 반복 55→25줄 |
| 72 | koukusaydon.flow.gate2.hp.55-25.entry.11 | PATTERN/P27 | 대형세이튼_불뿜기 | 1000 | 일반 패턴 반복 55→25줄 |
| 73 | koukusaydon.flow.gate2.mechanic.hp25.entry.01 | BUNDLE/koukusaydon.bundle.3 | 조커찾기_동시 | 1000 | 25줄 조커 찾기 |
| 74 | koukusaydon.flow.gate2.hp.25-0.entry.02 | PATTERN/P106 | 쿠크_저글링액션 | 1000 | 일반 패턴 반복 25→0줄 |
| 75 | koukusaydon.flow.gate2.hp.25-0.entry.02.trumpet | PATTERN/P105 | 쿠크_나팔액션 | 1000 | 일반 패턴 반복 25→0줄 |
| 76 | koukusaydon.flow.gate2.hp.25-0.entry.04 | PATTERN/P85 | 쿠크_슈퍼바주카 | 1000 | 일반 패턴 반복 25→0줄 |
| 77 | koukusaydon.flow.gate2.hp.25-0.entry.05 | BUNDLE/koukusaydon.bundle.6 | 쿠크거미카운터_동시 | 1000 | 일반 패턴 반복 25→0줄 |
| 78 | koukusaydon.flow.gate2.hp.25-0.entry.05.fanfare | PATTERN/P23 | 쿠크_팡파레 | 1000 | 일반 패턴 반복 25→0줄 |
| 79 | koukusaydon.flow.gate2.hp.25-0.entry.06 | PATTERN/P21 | 쿠크_레이저 | 1000 | 일반 패턴 반복 25→0줄 |
| 80 | koukusaydon.flow.gate2.hp.25-0.entry.07 | BUNDLE/koukusaydon.bundle.9 | 쿠크휠윈드_동시 | 1000 | 일반 패턴 반복 25→0줄 |
| 81 | koukusaydon.flow.gate2.group.hp.25-0.grab | PATTERN/P17 | 대형세이튼_잡기 | 1000 | 일반 패턴 반복 25→0줄 |
| 82 | koukusaydon.flow.gate2.hp.25-0.entry.08 | PATTERN/P99 | 대형세이튼_불어날리기 | 1000 | 일반 패턴 반복 25→0줄 |
| 83 | koukusaydon.flow.gate2.hp.25-0.entry.09 | PATTERN/P86 | 대형세이튼_두번내려치기 | 1000 | 일반 패턴 반복 25→0줄 |
| 84 | koukusaydon.flow.gate2.hp.25-0.entry.10 | PATTERN/P87 | 대형세이튼_3번내려치기 | 1000 | 일반 패턴 반복 25→0줄 |
| 85 | koukusaydon.flow.gate2.hp.25-0.entry.11 | PATTERN/P27 | 대형세이튼_불뿜기 | 1000 | 일반 패턴 반복 25→0줄 |

| group ID/이름 | 시작 entry (순서) | 끝 entry (순서) | repeatUntilHealthBars | transitionAt | repeatCount |
|---|---|---|---:|---|---:|
| koukusaydon.flow.gate2.group.hp.140-125 / 일반 패턴 반복 140→125줄 | koukusaydon.flow.gate2.restore.initial.7f4de4493f141aa9 (1) | koukusaydon.flow.gate2.entry.8 (7) | 125 | PATTERN_END | — |
| koukusaydon.flow.gate2.group.mechanic.hp125 / 125줄 세이튼 등장 | koukusaydon.flow.gate2.entry.1 (8) | koukusaydon.flow.gate2.entry.1 (8) | — | — | — |
| koukusaydon.flow.gate2.group.hp.125-110 / 일반 패턴 반복 125→110줄 | koukusaydon.flow.gate2.hp.125-110.entry.02 (9) | koukusaydon.flow.gate2.restore.loop.6 (20) | 110 | PATTERN_END | — |
| koukusaydon.flow.gate2.group.mechanic.hp110 / 110줄 파1빨2 | koukusaydon.flow.gate2.entry.2 (21) | koukusaydon.flow.gate2.entry.2 (21) | — | — | — |
| koukusaydon.flow.gate2.group.hp.110-95 / 일반 패턴 반복 110→95줄 | koukusaydon.flow.gate2.hp.110-95.entry.02 (22) | koukusaydon.flow.gate2.hp.110-95.entry.11 (33) | 95 | PATTERN_END | — |
| koukusaydon.flow.gate2.group.mechanic.hp95 / 95줄 조커 찾기 | koukusaydon.flow.gate2.entry.3 (34) | koukusaydon.flow.gate2.entry.3 (34) | — | — | — |
| koukusaydon.flow.gate2.group.hp.95-80 / 일반 패턴 반복 95→80줄 | koukusaydon.flow.gate2.hp.95-80.entry.02 (35) | koukusaydon.flow.gate2.hp.95-80.entry.11 (46) | 80 | PATTERN_END | — |
| koukusaydon.flow.gate2.group.mechanic.hp80 / 80줄 카드미로 | koukusaydon.flow.gate2.entry.10 (47) | koukusaydon.flow.gate2.entry.10 (47) | — | — | — |
| koukusaydon.flow.gate2.group.hp.80-55 / 일반 패턴 반복 80→55줄 | koukusaydon.flow.gate2.hp.80-55.entry.02 (48) | koukusaydon.flow.gate2.hp.80-55.entry.11 (59) | 55 | PATTERN_END | — |
| koukusaydon.flow.gate2.group.mechanic.hp55 / 55줄 피자 | koukusaydon.flow.gate2.mechanic.hp55.entry.01 (60) | koukusaydon.flow.gate2.mechanic.hp55.entry.01 (60) | — | — | — |
| koukusaydon.flow.gate2.group.hp.55-25 / 일반 패턴 반복 55→25줄 | koukusaydon.flow.gate2.hp.55-25.entry.02 (61) | koukusaydon.flow.gate2.hp.55-25.entry.11 (72) | 25 | PATTERN_END | — |
| koukusaydon.flow.gate2.group.mechanic.hp25 / 25줄 조커 찾기 | koukusaydon.flow.gate2.mechanic.hp25.entry.01 (73) | koukusaydon.flow.gate2.mechanic.hp25.entry.01 (73) | — | — | — |
| koukusaydon.flow.gate2.group.hp.25-0 / 일반 패턴 반복 25→0줄 | koukusaydon.flow.gate2.hp.25-0.entry.02 (74) | koukusaydon.flow.gate2.hp.25-0.entry.11 (85) | 0 | PATTERN_END | — |

### GATE3 · koukusaydon.flow.gate3

entries=72, entryGroups=11, loopStartEntryId=`—`
추가 Flow 설정: `{}`.

| 순서 | entry ID | kind/target | 이름 | waitAfterMs | 그룹 |
|---:|---|---|---|---:|---|
| 1 | koukusaydon.flow.gate3.0923.1 | PATTERN/P52 | 쿠크세이튼_분신소환_기분나빠 | 0 | 일반 패턴 반복 180→155줄 |
| 2 | koukusaydon.flow.gate3.0923.3 | PATTERN/P119 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | 0 | 일반 패턴 반복 180→155줄 |
| 3 | koukusaydon.flow.gate3.hp.180-155.entry.03 | PATTERN/P59 | 쿠크세이튼_화염 파동 | 0 | 일반 패턴 반복 180→155줄 |
| 4 | koukusaydon.flow.gate3.0923.5 | PATTERN/P116 | 쿠크세이튼_정면바람방구 | 0 | 일반 패턴 반복 180→155줄 |
| 5 | koukusaydon.flow.gate3.hp.180-155.entry.05 | PATTERN/P118 | 쿠크세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 180→155줄 |
| 6 | koukusaydon.flow.gate3.0923.8 | PATTERN/P39 | 쿠크세이튼_알비온_감전장판 | 0 | 일반 패턴 반복 180→155줄 |
| 7 | koukusaydon.flow.gate3.0923.16 | PATTERN/P38 | 쿠크세이튼_무지개댄스 | 0 | 일반 패턴 반복 180→155줄 |
| 8 | koukusaydon.flow.gate3.hp.180-155.entry.08 | PATTERN/P117 | 쿠크세이튼_돌진_카운터 | 0 | 일반 패턴 반복 180→155줄 |
| 9 | koukusaydon.flow.gate3.hp.180-155.entry.08.follow.1s | PATTERN/P125 | 쿠크세이튼_1초추적 | 0 | 일반 패턴 반복 180→155줄 |
| 10 | koukusaydon.flow.gate3.0923.14 | PATTERN/P43 | 쿠크세이튼_백스텝불뿜기_화염링 | 0 | 일반 패턴 반복 180→155줄 |
| 11 | koukusaydon.flow.gate3.group.hp.180-155.laser | PATTERN/P62 | 세이튼_눈 레이저 | 0 | 일반 패턴 반복 180→155줄 |
| 12 | koukusaydon.flow.gate3.0923.4 | PATTERN/P88 | 1마리오_1페이즈 | 0 | 155줄 1마리오 |
| 13 | koukusaydon.flow.gate3.0923.6 | PATTERN/P52 | 쿠크세이튼_분신소환_기분나빠 | 0 | 일반 패턴 반복 155→125줄 |
| 14 | koukusaydon.flow.gate3.0923.15 | PATTERN/P119 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | 0 | 일반 패턴 반복 155→125줄 |
| 15 | koukusaydon.flow.gate3.hp.155-125.entry.03 | PATTERN/P59 | 쿠크세이튼_화염 파동 | 0 | 일반 패턴 반복 155→125줄 |
| 16 | koukusaydon.flow.gate3.hp.155-125.entry.04 | PATTERN/P116 | 쿠크세이튼_정면바람방구 | 0 | 일반 패턴 반복 155→125줄 |
| 17 | koukusaydon.flow.gate3.hp.155-125.entry.05 | PATTERN/P118 | 쿠크세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 155→125줄 |
| 18 | koukusaydon.flow.gate3.0923.17 | PATTERN/P39 | 쿠크세이튼_알비온_감전장판 | 0 | 일반 패턴 반복 155→125줄 |
| 19 | koukusaydon.flow.gate3.hp.155-125.entry.07 | PATTERN/P38 | 쿠크세이튼_무지개댄스 | 0 | 일반 패턴 반복 155→125줄 |
| 20 | koukusaydon.flow.gate3.hp.155-125.entry.08 | PATTERN/P117 | 쿠크세이튼_돌진_카운터 | 0 | 일반 패턴 반복 155→125줄 |
| 21 | koukusaydon.flow.gate3.hp.155-125.entry.08.follow.1s | PATTERN/P125 | 쿠크세이튼_1초추적 | 0 | 일반 패턴 반복 155→125줄 |
| 22 | koukusaydon.flow.gate3.hp.155-125.entry.09 | PATTERN/P43 | 쿠크세이튼_백스텝불뿜기_화염링 | 0 | 일반 패턴 반복 155→125줄 |
| 23 | koukusaydon.flow.gate3.group.hp.155-125.laser | PATTERN/P62 | 세이튼_눈 레이저 | 0 | 일반 패턴 반복 155→125줄 |
| 24 | koukusaydon.flow.gate3.0923.9 | PATTERN/P91 | 2마리오_1페이즈 | 0 | 125줄 2마리오 |
| 25 | koukusaydon.flow.gate3.0923.18 | PATTERN/P52 | 쿠크세이튼_분신소환_기분나빠 | 0 | 일반 패턴 반복 125→90줄 |
| 26 | koukusaydon.flow.gate3.hp.125-90.entry.02 | PATTERN/P119 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | 0 | 일반 패턴 반복 125→90줄 |
| 27 | koukusaydon.flow.gate3.hp.125-90.entry.03 | PATTERN/P59 | 쿠크세이튼_화염 파동 | 0 | 일반 패턴 반복 125→90줄 |
| 28 | koukusaydon.flow.gate3.hp.125-90.entry.04 | PATTERN/P116 | 쿠크세이튼_정면바람방구 | 0 | 일반 패턴 반복 125→90줄 |
| 29 | koukusaydon.flow.gate3.hp.125-90.entry.05 | PATTERN/P118 | 쿠크세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 125→90줄 |
| 30 | koukusaydon.flow.gate3.hp.125-90.entry.06 | PATTERN/P39 | 쿠크세이튼_알비온_감전장판 | 0 | 일반 패턴 반복 125→90줄 |
| 31 | koukusaydon.flow.gate3.hp.125-90.entry.07 | PATTERN/P38 | 쿠크세이튼_무지개댄스 | 0 | 일반 패턴 반복 125→90줄 |
| 32 | koukusaydon.flow.gate3.hp.125-90.entry.08 | PATTERN/P117 | 쿠크세이튼_돌진_카운터 | 0 | 일반 패턴 반복 125→90줄 |
| 33 | koukusaydon.flow.gate3.hp.125-90.entry.08.follow.1s | PATTERN/P125 | 쿠크세이튼_1초추적 | 0 | 일반 패턴 반복 125→90줄 |
| 34 | koukusaydon.flow.gate3.hp.125-90.entry.09 | PATTERN/P43 | 쿠크세이튼_백스텝불뿜기_화염링 | 0 | 일반 패턴 반복 125→90줄 |
| 35 | koukusaydon.flow.gate3.group.hp.125-90.laser | PATTERN/P62 | 세이튼_눈 레이저 | 0 | 일반 패턴 반복 125→90줄 |
| 36 | koukusaydon.flow.gate3.0923.10 | PATTERN/P76 | 쇼타임_연출 | 0 | 90줄 쇼타임 |
| 37 | koukusaydon.flow.gate3.0923.11 | PATTERN/P35 | 세이튼_쇼타임 | 0 | 90줄 쇼타임 |
| 38 | koukusaydon.flow.gate3.hp.90-80.entry.01 | PATTERN/P52 | 쿠크세이튼_분신소환_기분나빠 | 0 | 일반 패턴 반복 90→80줄 |
| 39 | koukusaydon.flow.gate3.hp.90-80.entry.02 | PATTERN/P119 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | 0 | 일반 패턴 반복 90→80줄 |
| 40 | koukusaydon.flow.gate3.hp.90-80.entry.03 | PATTERN/P59 | 쿠크세이튼_화염 파동 | 0 | 일반 패턴 반복 90→80줄 |
| 41 | koukusaydon.flow.gate3.hp.90-80.entry.04 | PATTERN/P116 | 쿠크세이튼_정면바람방구 | 0 | 일반 패턴 반복 90→80줄 |
| 42 | koukusaydon.flow.gate3.hp.90-80.entry.05 | PATTERN/P118 | 쿠크세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 90→80줄 |
| 43 | koukusaydon.flow.gate3.hp.90-80.entry.06 | PATTERN/P39 | 쿠크세이튼_알비온_감전장판 | 0 | 일반 패턴 반복 90→80줄 |
| 44 | koukusaydon.flow.gate3.hp.90-80.entry.07 | PATTERN/P38 | 쿠크세이튼_무지개댄스 | 0 | 일반 패턴 반복 90→80줄 |
| 45 | koukusaydon.flow.gate3.hp.90-80.entry.08 | PATTERN/P117 | 쿠크세이튼_돌진_카운터 | 0 | 일반 패턴 반복 90→80줄 |
| 46 | koukusaydon.flow.gate3.hp.90-80.entry.08.follow.1s | PATTERN/P125 | 쿠크세이튼_1초추적 | 0 | 일반 패턴 반복 90→80줄 |
| 47 | koukusaydon.flow.gate3.hp.90-80.entry.09 | PATTERN/P43 | 쿠크세이튼_백스텝불뿜기_화염링 | 0 | 일반 패턴 반복 90→80줄 |
| 48 | koukusaydon.flow.gate3.group.hp.90-80.laser | PATTERN/P62 | 세이튼_눈 레이저 | 0 | 일반 패턴 반복 90→80줄 |
| 49 | koukusaydon.flow.gate3.0923.12 | PATTERN/P92 | 3마리오_1페이즈 | 0 | 80줄 3마리오 |
| 50 | koukusaydon.flow.gate3.hp.80-55.entry.01 | PATTERN/P52 | 쿠크세이튼_분신소환_기분나빠 | 0 | 일반 패턴 반복 80→55줄 |
| 51 | koukusaydon.flow.gate3.hp.80-55.entry.02 | PATTERN/P119 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | 0 | 일반 패턴 반복 80→55줄 |
| 52 | koukusaydon.flow.gate3.hp.80-55.entry.03 | PATTERN/P59 | 쿠크세이튼_화염 파동 | 0 | 일반 패턴 반복 80→55줄 |
| 53 | koukusaydon.flow.gate3.hp.80-55.entry.04 | PATTERN/P116 | 쿠크세이튼_정면바람방구 | 0 | 일반 패턴 반복 80→55줄 |
| 54 | koukusaydon.flow.gate3.hp.80-55.entry.05 | PATTERN/P118 | 쿠크세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 80→55줄 |
| 55 | koukusaydon.flow.gate3.hp.80-55.entry.06 | PATTERN/P39 | 쿠크세이튼_알비온_감전장판 | 0 | 일반 패턴 반복 80→55줄 |
| 56 | koukusaydon.flow.gate3.hp.80-55.entry.07 | PATTERN/P38 | 쿠크세이튼_무지개댄스 | 0 | 일반 패턴 반복 80→55줄 |
| 57 | koukusaydon.flow.gate3.hp.80-55.entry.08 | PATTERN/P117 | 쿠크세이튼_돌진_카운터 | 0 | 일반 패턴 반복 80→55줄 |
| 58 | koukusaydon.flow.gate3.hp.80-55.entry.08.follow.1s | PATTERN/P125 | 쿠크세이튼_1초추적 | 0 | 일반 패턴 반복 80→55줄 |
| 59 | koukusaydon.flow.gate3.hp.80-55.entry.09 | PATTERN/P43 | 쿠크세이튼_백스텝불뿜기_화염링 | 0 | 일반 패턴 반복 80→55줄 |
| 60 | koukusaydon.flow.gate3.group.hp.80-55.laser | PATTERN/P62 | 세이튼_눈 레이저 | 0 | 일반 패턴 반복 80→55줄 |
| 61 | koukusaydon.flow.gate3.0923.13 | PATTERN/P93 | 4마리오_1페이즈 | 0 | 55줄 4마리오 |
| 62 | koukusaydon.flow.gate3.hp.55-0.entry.01 | PATTERN/P52 | 쿠크세이튼_분신소환_기분나빠 | 0 | 일반 패턴 반복 55→0줄 |
| 63 | koukusaydon.flow.gate3.hp.55-0.entry.02 | PATTERN/P119 | 쿠크세이튼_공먹고휘리릭노란장판펑펑 | 0 | 일반 패턴 반복 55→0줄 |
| 64 | koukusaydon.flow.gate3.hp.55-0.entry.03 | PATTERN/P59 | 쿠크세이튼_화염 파동 | 0 | 일반 패턴 반복 55→0줄 |
| 65 | koukusaydon.flow.gate3.hp.55-0.entry.04 | PATTERN/P116 | 쿠크세이튼_정면바람방구 | 0 | 일반 패턴 반복 55→0줄 |
| 66 | koukusaydon.flow.gate3.hp.55-0.entry.05 | PATTERN/P118 | 쿠크세이튼_공굴리기_카운터 | 0 | 일반 패턴 반복 55→0줄 |
| 67 | koukusaydon.flow.gate3.hp.55-0.entry.06 | PATTERN/P39 | 쿠크세이튼_알비온_감전장판 | 0 | 일반 패턴 반복 55→0줄 |
| 68 | koukusaydon.flow.gate3.hp.55-0.entry.07 | PATTERN/P38 | 쿠크세이튼_무지개댄스 | 0 | 일반 패턴 반복 55→0줄 |
| 69 | koukusaydon.flow.gate3.hp.55-0.entry.08 | PATTERN/P117 | 쿠크세이튼_돌진_카운터 | 0 | 일반 패턴 반복 55→0줄 |
| 70 | koukusaydon.flow.gate3.hp.55-0.entry.08.follow.1s | PATTERN/P125 | 쿠크세이튼_1초추적 | 0 | 일반 패턴 반복 55→0줄 |
| 71 | koukusaydon.flow.gate3.hp.55-0.entry.09 | PATTERN/P43 | 쿠크세이튼_백스텝불뿜기_화염링 | 0 | 일반 패턴 반복 55→0줄 |
| 72 | koukusaydon.flow.gate3.group.hp.55-0.laser | PATTERN/P62 | 세이튼_눈 레이저 | 0 | 일반 패턴 반복 55→0줄 |

| group ID/이름 | 시작 entry (순서) | 끝 entry (순서) | repeatUntilHealthBars | transitionAt | repeatCount |
|---|---|---|---:|---|---:|
| koukusaydon.flow.gate3.group.hp.180-155 / 일반 패턴 반복 180→155줄 | koukusaydon.flow.gate3.0923.1 (1) | koukusaydon.flow.gate3.group.hp.180-155.laser (11) | 155 | PATTERN_END | — |
| koukusaydon.flow.gate3.group.mechanic.hp155 / 155줄 1마리오 | koukusaydon.flow.gate3.0923.4 (12) | koukusaydon.flow.gate3.0923.4 (12) | — | — | — |
| koukusaydon.flow.gate3.group.hp.155-125 / 일반 패턴 반복 155→125줄 | koukusaydon.flow.gate3.0923.6 (13) | koukusaydon.flow.gate3.group.hp.155-125.laser (23) | 125 | PATTERN_END | — |
| koukusaydon.flow.gate3.group.mechanic.hp125 / 125줄 2마리오 | koukusaydon.flow.gate3.0923.9 (24) | koukusaydon.flow.gate3.0923.9 (24) | — | — | — |
| koukusaydon.flow.gate3.group.hp.125-90 / 일반 패턴 반복 125→90줄 | koukusaydon.flow.gate3.0923.18 (25) | koukusaydon.flow.gate3.group.hp.125-90.laser (35) | 90 | PATTERN_END | — |
| koukusaydon.flow.gate3.group.mechanic.hp90 / 90줄 쇼타임 | koukusaydon.flow.gate3.0923.10 (36) | koukusaydon.flow.gate3.0923.11 (37) | — | — | — |
| koukusaydon.flow.gate3.group.hp.90-80 / 일반 패턴 반복 90→80줄 | koukusaydon.flow.gate3.hp.90-80.entry.01 (38) | koukusaydon.flow.gate3.group.hp.90-80.laser (48) | 80 | PATTERN_END | — |
| koukusaydon.flow.gate3.group.mechanic.hp80 / 80줄 3마리오 | koukusaydon.flow.gate3.0923.12 (49) | koukusaydon.flow.gate3.0923.12 (49) | — | — | — |
| koukusaydon.flow.gate3.group.hp.80-55 / 일반 패턴 반복 80→55줄 | koukusaydon.flow.gate3.hp.80-55.entry.01 (50) | koukusaydon.flow.gate3.group.hp.80-55.laser (60) | 55 | PATTERN_END | — |
| koukusaydon.flow.gate3.group.mechanic.hp55 / 55줄 4마리오 | koukusaydon.flow.gate3.0923.13 (61) | koukusaydon.flow.gate3.0923.13 (61) | — | — | — |
| koukusaydon.flow.gate3.group.hp.55-0 / 일반 패턴 반복 55→0줄 | koukusaydon.flow.gate3.hp.55-0.entry.01 (62) | koukusaydon.flow.gate3.group.hp.55-0.laser (72) | 0 | PATTERN_END | — |

### BINGO · koukusaydon.flow.bingo

entries=35, entryGroups=0, loopStartEntryId=`koukusaydon.flow.bingo.independent.30`
추가 Flow 설정: `{"bingoSpecialPatternId":"KOUKUSAYDON_G1_PATTERN_107"}`.

| 순서 | entry ID | kind/target | 이름 | waitAfterMs | 그룹 |
|---:|---|---|---|---:|---|
| 1 | koukusaydon.flow.bingo.board-start | PATTERN/P129 | 빙고_보드시작 | 0 | — |
| 2 | koukusaydon.flow.bingo.independent.6 | PATTERN/P108 | 앵콜세이튼_분신소환_기분나빠 | 0 | — |
| 3 | koukusaydon.flow.bingo.independent.8 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 4 | koukusaydon.flow.bingo.independent.9 | PATTERN/P109 | 앵콜세이튼_기본공격 | 0 | — |
| 5 | koukusaydon.flow.bingo.independent.10 | PATTERN/P113 | 앵콜세이튼_정면바람방구 | 0 | — |
| 6 | koukusaydon.flow.bingo.independent.11 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 7 | koukusaydon.flow.bingo.independent.12 | PATTERN/P109 | 앵콜세이튼_기본공격 | 0 | — |
| 8 | koukusaydon.flow.bingo.independent.13 | PATTERN/P112 | 앵콜세이튼_십자화염폭발 | 0 | — |
| 9 | koukusaydon.flow.bingo.independent.14 | PATTERN/P114 | 앵콜세이튼_백스텝후화염링 | 0 | — |
| 10 | koukusaydon.flow.bingo.independent.44 | PATTERN/P123 | 앵콜세이튼_레이저 | 0 | — |
| 11 | koukusaydon.flow.bingo.independent.15 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 12 | koukusaydon.flow.bingo.independent.16 | PATTERN/P109 | 앵콜세이튼_기본공격 | 0 | — |
| 13 | koukusaydon.flow.bingo.independent.17 | PATTERN/P108 | 앵콜세이튼_분신소환_기분나빠 | 0 | — |
| 14 | koukusaydon.flow.bingo.independent.20 | PATTERN/P114 | 앵콜세이튼_백스텝후화염링 | 0 | — |
| 15 | koukusaydon.flow.bingo.independent.21 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 16 | koukusaydon.flow.bingo.independent.22 | PATTERN/P108 | 앵콜세이튼_분신소환_기분나빠 | 0 | — |
| 17 | koukusaydon.flow.bingo.independent.23 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 18 | koukusaydon.flow.bingo.independent.25 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 19 | koukusaydon.flow.bingo.independent.26 | PATTERN/P112 | 앵콜세이튼_십자화염폭발 | 0 | — |
| 20 | koukusaydon.flow.bingo.independent.27 | PATTERN/P120 | 앵콜세이튼_알비온_감전장판 | 0 | — |
| 21 | koukusaydon.flow.bingo.independent.28 | PATTERN/P122 | 앵콜세이튼_공먹고휘리릭노란장판펑펑 | 0 | — |
| 22 [loop 시작] | koukusaydon.flow.bingo.independent.30 | PATTERN/P108 | 앵콜세이튼_분신소환_기분나빠 | 0 | — |
| 23 | koukusaydon.flow.bingo.independent.32 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 24 | koukusaydon.flow.bingo.independent.33 | PATTERN/P109 | 앵콜세이튼_기본공격 | 0 | — |
| 25 | koukusaydon.flow.bingo.independent.34 | PATTERN/P113 | 앵콜세이튼_정면바람방구 | 0 | — |
| 26 | koukusaydon.flow.bingo.independent.35 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 27 | koukusaydon.flow.bingo.independent.36 | PATTERN/P109 | 앵콜세이튼_기본공격 | 0 | — |
| 28 | koukusaydon.flow.bingo.independent.37 | PATTERN/P112 | 앵콜세이튼_십자화염폭발 | 0 | — |
| 29 | koukusaydon.flow.bingo.independent.38 | PATTERN/P114 | 앵콜세이튼_백스텝후화염링 | 0 | — |
| 30 | koukusaydon.flow.bingo.independent.45 | PATTERN/P123 | 앵콜세이튼_레이저 | 0 | — |
| 31 | koukusaydon.flow.bingo.independent.39 | PATTERN/P126 | 앵콜세이튼_플레이어추적 | 0 | — |
| 32 | koukusaydon.flow.bingo.independent.40 | PATTERN/P109 | 앵콜세이튼_기본공격 | 0 | — |
| 33 | koukusaydon.flow.bingo.independent.41 | PATTERN/P108 | 앵콜세이튼_분신소환_기분나빠 | 0 | — |
| 34 | koukusaydon.flow.bingo.independent.42 | PATTERN/P120 | 앵콜세이튼_알비온_감전장판 | 0 | — |
| 35 | koukusaydon.flow.bingo.independent.43 | PATTERN/P122 | 앵콜세이튼_공먹고휘리릭노란장판펑펑 | 0 | — |

## Logic 정의 210개 전체와 사용처

각 ID 링크는 작은 별도 logic_definitions.json의 해당 행으로 이동하며, 모든 필드 실제값과 ID별 참조를 보존했다. 아래 사용처는 occurrence 직접 연결(D)과 결과 참조(S/F/T)를 구분한다. 비활성 occurrence의 참조도 포함한다.
500자 이하의 추가 파라미터는 표에 값까지 표시했다. 큰 배열은 필드 목록과 ID 상세 링크를 사용한다. `미참조` 정의를 삭제 후보로 판정하지 않는다.

### DURATION (50)

| ID (전체값 링크) | 한글 이름 | kind | 추가 파라미터 실제값 | 참조 occurrence |
|---|---|---|---|---|
| [L1](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:5) | 세이튼로직_방패무력화 | STAGGER_WINDOW | {"threshold":1150000,"shieldArcDegrees":71.737272,"endsPatternOnSuccess":true,"normalYawOffsetDegrees":90} | P1.logic.1:D |
| [L10](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:333) | 세이튼로직_슈퍼맨 | POSE_INPUT | {"poseIndex":1} | P6.logic.1:D |
| [L11](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:352) | 세이튼로직_양팔벌리기 | POSE_INPUT | {"poseIndex":2} | P6.logic.2:D |
| [L12](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:371) | 세이튼로직_한다리올리기 | POSE_INPUT | {"poseIndex":3} | P6.logic.3:D |
| [L13](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:390) | 세이튼로직_양팔모으기 | POSE_INPUT | {"poseIndex":0} | P6.logic.4:D |
| [L14](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:409) | 세이튼_룰렛_1회차 | ROULETTE_CARD_MATCH | {"regionIds":["roulette.round.1.sector.1","roulette.round.1.sector.2","roulette.round.1.sector.3","roulette.round.1.sector.4","roulette.round.1.sector.5","roulette.round.1.sector.6","roulette.round.1.sector.7","roulette.round.1.sector.8"],"sectorCount":8,"sectorSymbols":["SPADE","CLUB","DIAMOND","HEART","CLUB","SPADE","HEART","DIAMOND"],"centerX":-0.319,"centerZ":737.531,"outerRadiusM":8,"worldSequenceInstanceId":"world.sequence.instance.8"} | P7.logic.1:D |
| [L16](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:518) | 세이튼로직_진짜세이튼찾기 | GAZE_REAL_BOSS | {"halfAngleDegrees":45,"maxDistanceM":30,"insideOutcome":"SUCCESS"} | P2.logic.3:D |
| [L17](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:539) | 세이튼_룰렛_2회차 | ROULETTE_CARD_MATCH | {"regionIds":["roulette.round.2.sector.1","roulette.round.2.sector.2","roulette.round.2.sector.3","roulette.round.2.sector.4","roulette.round.2.sector.5","roulette.round.2.sector.6","roulette.round.2.sector.7","roulette.round.2.sector.8"],"sectorCount":8,"sectorSymbols":["SPADE","CLUB","DIAMOND","HEART","CLUB","SPADE","HEART","DIAMOND"],"centerX":-0.319,"centerZ":737.531,"outerRadiusM":8,"worldSequenceInstanceId":"world.sequence.instance.8"} | P7.logic.2:D |
| [L18](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:582) | 세이튼_룰렛_3회차 | ROULETTE_CARD_MATCH | {"regionIds":["roulette.round.3.sector.1","roulette.round.3.sector.2","roulette.round.3.sector.3","roulette.round.3.sector.4","roulette.round.3.sector.5","roulette.round.3.sector.6","roulette.round.3.sector.7","roulette.round.3.sector.8"],"sectorCount":8,"sectorSymbols":["SPADE","CLUB","DIAMOND","HEART","CLUB","SPADE","HEART","DIAMOND"],"centerX":-0.319,"centerZ":737.531,"outerRadiusM":8,"worldSequenceInstanceId":"world.sequence.instance.8"} | P7.logic.3:D |
| [L19](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:625) | 세이튼로직_룰렛4회차 | AREA_OVERLAP | {"insideOutcome":"FAIL"} | P7.logic.5:D |
| [L20](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:644) | 2관문_거대세이튼_조커찾기 | EXTERNAL_SIGNAL | {"endsPatternOnSuccess":true} | P13.logic.6:D |
| [L36](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:949) | 세이튼로직_파1빨2무적 | GAZE_REAL_BOSS | {"halfAngleDegrees":45,"maxDistanceM":30,"insideOutcome":"FAIL"} | P11.logic.1:D; P11.logic.2:D; P11.logic.3:D |
| [L29](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1035) | 2관문_쿠크_거미카운터_카운터 | COUNTER_WINDOW | {"endsPatternOnSuccess":true} | P15.logic.1:D; P15.logic.2:D; P15.logic.3:D |
| [L33](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1161) | 대형세이튼_잡기유지 | ATTACHMENT_HOLD | {} | P17.logic.2:D |
| [L55](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1684) | 쿠크세이튼_마리오_랜덤패턴 | PATTERN_COMPLETION_COUNT | {"completionCount":3,"patternIds":["KOUKUSAYDON_G1_PATTERN_38","KOUKUSAYDON_G1_PATTERN_39","KOUKUSAYDON_G1_PATTERN_43"]} | P34.logic.1:D |
| [L56](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1708) | 쿠크세이튼_마리오_무력화 | STAGGER_WINDOW | {"threshold":1150000,"shieldArcDegrees":0,"endsPatternOnSuccess":true,"normalYawOffsetDegrees":0} | P33.logic.1:D |
| [L64](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1981) | 쿠크세이튼_쇼타임_추적고정조준점타겟 | SHOWTIME_PLAYER_TARGETS | 큰 값: fixedHits, trackingHits, randomVolleyHits, fixedSelectionGroupId, trackingPresentationOccurrenceId, spawnIntervalMs, followSpeedScale, randomVolleyOccurrenceSets, randomSpawnIntervalMs, randomArenaRadiusM, randomArenaHeightToleranceM, randomAnchorKind, randomScaleMin, randomScaleMax — ID 링크에서 원문 확인 | P35.logic.7:D |
| [L65](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2389) | 쿠크세이튼_쇼타임_추적회전 | BOSS_TRACK_TARGET | {} | P113.logic.2:D |
| [L73](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2672) | 세이튼_카드맞추기_플레이어추적 | PURSUIT_PROJECTILES | 큰 값: projectileHits, visualIds, cardSymbols, contactVisualId, speedMps, contactRadiusM, spawnRadiusM, lifetimeMs, spawnIntervalMs, homing, countPerWave — ID 링크에서 원문 확인 | 미참조 |
| [L74](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2723) | 세이튼_회전카드_원작12발 | PURSUIT_PROJECTILES | 큰 값: projectileHits, visualIds, contactVisualId, speedMps, contactRadiusM, spawnRadiusM, lifetimeMs, spawnIntervalMs, homing, countPerWave, maxDistanceM — ID 링크에서 원문 확인 | P48.logic.1:D |
| [L81](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3029) | 세이튼_쓰리투원투하_오른손모자 | ATTACHMENT_HOLD | {} | P83.logic.1:D |
| [L82](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3047) | 카운터 | COUNTER_WINDOW | {"endsPatternOnSuccess":true} | P80.logic.1:D; P117.logic.1:D |
| [L84](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3108) | 세이튼_공굴리기_추적이동 | BOSS_TRACK_TARGET | {"followSpeedScale":1} | P81.logic.2:D; P118.logic.2:D |
| [L85](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3135) | 세이튼_공굴리기_카운터판정 | COUNTER_WINDOW | {"endsPatternOnSuccess":true} | P81.logic.1:D; P118.logic.1:D |
| [L86](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3162) | 세이튼_카드맞추기_플레이어속박 | CARD_DICE_BIND | {} | P78.logic.2:D |
| [L99](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:6866) | 빙고_바닥_망치생성 | BINGO_BOARD | {} | P129.logic.1:D |
| [L101](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:6907) | 파1빨2 파랑 원 정확히 1명 무적 | INVULNERABILITY_ZONE | {"threshold":1} | P11.logic.7:D; P11.logic.9:D |
| [L121](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7426) | 세이튼_카드출력_하트 | — | {} | 미참조 |
| [L122](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7434) | 세이튼_카드출력_스페이드 | — | {} | 미참조 |
| [L123](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7442) | 세이튼_카드출력_클로버 | — | {} | 미참조 |
| [L124](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7450) | 세이튼_카드출력_다이아 | — | {} | 미참조 |
| [L131](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7709) | 카드비_카드낙하 | SHOWTIME_PLAYER_TARGETS | 큰 값: randomVolleyHits, fixedSelectionGroupId, trackingPresentationOccurrenceId, spawnIntervalMs, followSpeedScale, randomVolleyOccurrenceSets, randomSpawnIntervalMs, randomArenaRadiusM, randomArenaHeightToleranceM, randomAnchorKind, randomScaleMin, randomScaleMax — ID 링크에서 원문 확인 | P103.logic.3:D |
| [L132](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7767) | 세이튼_플레이어추적_이동 | BOSS_TRACK_TARGET | {"followSpeedScale":1} | P101.logic.1:D; P104.logic.1:D; P124.logic.1:D; P125.logic.1:D; P126.logic.1:D |
| [L135](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7942) | 세이튼등장_플레이어위치하강공 | SHOWTIME_PLAYER_TARGETS | {"fixedSelectionGroupId":"KOUKUSAYDON_G1_PATTERN_8.gate2restore.drop.group","trackingPresentationOccurrenceId":"","spawnIntervalMs":2000,"followSpeedScale":0.5} | P8.logic.1:D |
| [L513](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10149) | 불뿜기_일반틱피해 | AREA_OVERLAP | {"insideOutcome":"SUCCESS","repeatIntervalMs":100} | P40.logic.1:D; P43.logic.2:D; P43.logic.3:D; P43.logic.4:D; P43.logic.5:D; P43.logic.6:D; P43.logic.7:D; P43.logic.8:D; P50.logic.1:D; P53.logic.1:D; P54.logic.1:D; P55.logic.1:D; P60.logic.1:D; P63.logic.1:D; P81.logic.23:D; P102.logic.1:D [off]; P102.logic.2:D; P102.logic.3:D; P102.logic.4:D; P102.logic.5:D; P102.logic.6:D; P102.logic.7:D; P102.logic.8:D; P108.logic.1:D; P112.logic.1:D; P114.logic.2:D; P114.logic.3:D; P114.logic.4:D; P114.logic.5:D; P114.logic.6:D; P114.logic.7:D; P114.logic.8:D; P118.logic.20:D |
| [L514](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10425) | 불뿜기_인형_공_틱피해 | AREA_OVERLAP | {"insideOutcome":"SUCCESS","repeatIntervalMs":100} | P34.logic.5:D; P88.logic.3:D; P88.logic.4:D; P88.logic.5:D; P88.logic.6:D; P89.logic.3:D; P89.logic.4:D; P89.logic.5:D; P91.logic.3:D; P91.logic.4:D; P91.logic.5:D; P91.logic.6:D; P92.logic.3:D; P92.logic.4:D; P92.logic.5:D; P92.logic.6:D; P93.logic.3:D; P93.logic.4:D; P93.logic.5:D; P93.logic.6:D |
| [L525](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11620) | 빙고_새_빨간_가로세로_1줄_완성 | BINGO_COMPLETED_LINES | {"threshold":1} | P107.logic.2:D |
| [L540](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12787) | Breath: three contact ticks | AREA_OVERLAP | {"insideOutcome":"SUCCESS","repeatIntervalMs":780} | P27.logic.3:D |
| [L541](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12807) | Breath: three contact ticks | AREA_OVERLAP | {"insideOutcome":"SUCCESS","repeatIntervalMs":1632} | P43.logic.1:D; P102.logic.9:D; P102.logic.10:D [off]; P102.logic.11:D [off]; P114.logic.1:D |
| [L542](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12859) | Breath: three contact ticks | AREA_OVERLAP | {"insideOutcome":"SUCCESS","repeatIntervalMs":1661} | P67.logic.1:D |
| [L543](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12879) | Breath: three contact ticks | AREA_OVERLAP | {"insideOutcome":"SUCCESS","repeatIntervalMs":1612} | P68.logic.1:D; P69.logic.1:D; P70.logic.1:D |
| [L547](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13211) | 파1빨2 빨강 원 정확히 2명 무적 | INVULNERABILITY_ZONE | {"threshold":2} | P11.logic.8:D; P11.logic.11:D |
| [L550](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13300) | 쇼타임_추적회전_폭탄판정_1 | BOSS_TRACK_TARGET | {"bombPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.638","bombExplosionPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.639","bombSectorPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.99","bombSectorRadiusM":11,"bombSectorHalfAngleDegrees":22.5} | P35.logic.11:D |
| [L551](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13323) | 쇼타임_추적회전_폭탄판정_2 | BOSS_TRACK_TARGET | {"bombPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.652","bombExplosionPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.653","bombSectorPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.648","bombSectorRadiusM":11,"bombSectorHalfAngleDegrees":22.5} | P35.logic.12:D |
| [L552](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13346) | 쇼타임_추적회전_폭탄판정_3 | BOSS_TRACK_TARGET | {"bombPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.756","bombExplosionPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.757","bombSectorPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.700","bombSectorRadiusM":11,"bombSectorHalfAngleDegrees":22.5} | P35.logic.14:D |
| [L553](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13369) | 쇼타임_추적회전_폭탄판정_4 | BOSS_TRACK_TARGET | {"bombPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.779","bombExplosionPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.780","bombSectorPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_35.presentation.716","bombSectorRadiusM":11,"bombSectorHalfAngleDegrees":22.5} | P35.logic.16:D |
| [L555](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13425) | 메두사공포_활성시야판정 | GAZE_REAL_BOSS | {"halfAngleDegrees":45,"maxDistanceM":30,"insideOutcome":"FAIL","gazeDuringWindow":true} | P94.logic.1:D |
| [L557](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13721) | 조커찾기_랜덤표적설정과시선이동 | BOSS_RANDOM_TARGET | {"trackingPresentationOccurrenceId":"KOUKUSAYDON_G1_PATTERN_13.presentation.55"} | P13.logic.33:D; P13.logic.34:D; P13.logic.35:D |
| [L558](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13756) | 마리오_2페이즈_무력화_대미지감소 | BOSS_DAMAGE_REDUCTION | {} | P33.logic.4:D |
| [L559](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13774) | 마리오_1페이즈_대미지감소 | BOSS_DAMAGE_REDUCTION | {} | P88.logic.7:D; P91.logic.7:D; P92.logic.7:D; P93.logic.7:D |

### TRIGGER (83)

| ID (전체값 링크) | 한글 이름 | kind | 추가 파라미터 실제값 | 참조 occurrence |
|---|---|---|---|---|
| [L9](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:303) | 세이튼로직_순간이동 | REAL_GAZE_TELEPORT | {"faceCenterYawOffsetDegrees":-90,"teleportPosition":[0.604172031,1.3,729.87770021],"clonePatternId":"KOUKUSAYDON_G1_PATTERN_5","clockHours":[4,7,10]} | P2.logic.1:D |
| [L22](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:684) | 대형세이튼로직_카드뒤집기 | OBJECT_CONTACT | {"targetWorldOccurrenceIds":["KOUKUSAYDON_G1_PATTERN_13.world.5","KOUKUSAYDON_G1_PATTERN_13.world.6","KOUKUSAYDON_G1_PATTERN_13.world.7","KOUKUSAYDON_G1_PATTERN_13.world.8","KOUKUSAYDON_G1_PATTERN_13.world.9","KOUKUSAYDON_G1_PATTERN_13.world.10","KOUKUSAYDON_G1_PATTERN_13.world.11"],"targetRadiusM":1.25,"contactPriority":200,"contactGroupId":"joker.hammer.contact"} | P13.logic.2:D; P13.logic.17:D; P13.logic.23:D |
| [L23](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:730) | 대형세이튼로직_카드들썩임 | OBJECT_CONTACT | {"targetWorldOccurrenceIds":["KOUKUSAYDON_G1_PATTERN_13.world.5","KOUKUSAYDON_G1_PATTERN_13.world.6","KOUKUSAYDON_G1_PATTERN_13.world.7","KOUKUSAYDON_G1_PATTERN_13.world.8","KOUKUSAYDON_G1_PATTERN_13.world.9","KOUKUSAYDON_G1_PATTERN_13.world.10","KOUKUSAYDON_G1_PATTERN_13.world.11"],"targetRadiusM":1.25,"contactPriority":100,"contactGroupId":"joker.hammer.contact"} | P13.logic.3:D; P13.logic.14:D; P13.logic.22:D |
| [L31](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1107) | 2관문_트리거_거미카운터_공포 | ENTER_AREA | {"bossChargeDistanceM":7,"chargeYawOffsetDegrees":-90} | P15.logic.4:D; P15.logic.5:D; P15.logic.6:D |
| [L32](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1143) | 대형세이튼_잡기판정 | ENTER_AREA | {} | P17.logic.1:D |
| [L27](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1248) | 3관문_갈고리_접촉 | ENTER_AREA | {} | 미참조 |
| [L38](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1269) | 2관문_쿠크_휠윈드 | ENTER_AREA | {"repeatAfterKnockback":true,"bossChargeDistanceM":10,"chargeYawOffsetDegrees":-90} | P24.logic.1:D |
| [L40](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1316) | 2관문_카드미로진입 | CARD_MAZE_ENTER | {"teleportPosition":[0.28,-0.01,1351.65]} | P28.logic.1:D; P77.logic.1:D |
| [L41](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1347) | 2관문_쿠크_레이저 | ENTER_AREA | {"repeatAfterKnockback":true} | P21.logic.10:D; P21.logic.11:D; P21.logic.12:D; P21.logic.13:D; P21.logic.14:D; P21.logic.15:D; P21.logic.16:D; P23.logic.8:D [off]; P23.logic.9:D [off]; P23.logic.10:D [off]; P23.logic.11:D [off] |
| [L43](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1461) | 2관문_트리거_카드미로 | CARD_MAZE_HIDE_NEXT | {} | P28.logic.2:D; P28.logic.3:D; P28.logic.4:D; P28.logic.5:D; P77.logic.2:D; P77.logic.3:D; P77.logic.4:D; P77.logic.5:D |
| [L44](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1535) | 알비온_플레이어장판 | ALBION_BLUE_CIRCLE | {"countPerPlayer":1,"radiusM":0,"effectLifetimeMs":7000} | 미참조 |
| [L45](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1547) | 3관문_트리거_쇼타임180도회전 | — | {} | 미참조 |
| [L46](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1555) | 쇼타임_플레이어추적 | — | {} | 미참조 |
| [L47](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1563) | 중앙이동 | BOSS_TELEPORT_XZ | {"teleportPosition":[-0.0700000003,1.32000005,942.330017]} | P38.logic.1:D |
| [L52](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1618) | 애니메이션_블렌딩 | — | {} | 미참조 |
| [L53](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1626) | 로직_랜덤플레이어탐색 | ALBION_AIRBORNE | {"airbornePhase":"SELECT_PLAYER","airborneHeightM":0,"airborneDurationMs":0,"teleportPosition":[0,0,0]} | P39.logic.1:D; P39.logic.3:D; P120.logic.1:D; P120.logic.2:D |
| [L54](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1676) | 쿠크세이튼_알비온_하늘순간이동 | — | {} | 미참조 |
| [L58](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1759) | 마리오_괴기스러운인형_공_랜덤스폰 | — | {} | P34.logic.2:D; P88.logic.2:D; P89.logic.1:D; P91.logic.2:D; P92.logic.2:D; P93.logic.2:D |
| [L59](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1816) | 마리오_입장 | ENTER_AREA | {} | P34.logic.3:D; P88.logic.1:D; P91.logic.1:D; P92.logic.1:D; P93.logic.1:D |
| [L62](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1908) | 세이튼_1관문연출_애니메이션_블렌딩 | ANIMATION_BLEND | {} | P36.logic.1:D; P36.logic.2:D; P36.logic.3:D; P36.logic.4:D; P36.logic.5:D |
| [L63](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1958) | 세이튼로직_쇼타임_왼쪽위순간이동 | BOSS_TELEPORT_XZ | {"teleportPosition":[2.57,1.3,952.27]} | P35.logic.6:D |
| [L66](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2407) | 쿠크세이튼_알비온_점프 | ALBION_AIRBORNE | {"airbornePhase":"JUMP","airborneHeightM":13.678813305200356,"airborneDurationMs":200,"teleportPosition":[0,0,0]} | P39.logic.5:D; P120.logic.3:D |
| [L67](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2441) | 쿠크세이튼_알비온_순간이동_사라지기 | ALBION_AIRBORNE | {"airbornePhase":"DISAPPEAR","airborneHeightM":0,"airborneDurationMs":0,"teleportPosition":[0,0,0]} | P39.logic.8:D; P120.logic.5:D |
| [L68](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2475) | 쿠크세이튼_알비온_순간이동_등장 | ALBION_AIRBORNE | {"airbornePhase":"APPEAR_PLAYER","airborneHeightM":3.2783114316320474,"airborneDurationMs":0,"teleportPosition":[0,0,0]} | P39.logic.7:D; P39.logic.11:D; P120.logic.4:D; P120.logic.8:D |
| [L69](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2525) | 쿠크세이튼_알비온_순간이동_중앙 | ALBION_AIRBORNE | {"airbornePhase":"CENTER","airborneHeightM":0,"airborneDurationMs":0,"teleportPosition":[-0.0700000003,1.32000005,942.330017]} | P39.logic.9:D |
| [L70](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2551) | 쿠크세이튼_알비온_내려치기_중앙 | ALBION_AIRBORNE | {"airbornePhase":"SLAM","airborneHeightM":0,"airborneDurationMs":0,"teleportPosition":[0,0,0]} | P39.logic.10:D; P79.logic.4:D; P119.logic.3:D; P120.logic.7:D; P122.logic.3:D |
| [L72](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2617) | 쿠크세이튼_알비온_플레이어추적_파란장판 | ALBION_BLUE_CIRCLE | {"fixedHits":[{"hitId":"skill.421990318.impact","trigger":"TIMED","shape":"CIRCLE","damageKind":"MAX_HP_PERCENT","damageProfileId":"","atMs":2000,"endMs":0,"repeatCount":1,"repeatIntervalMs":0,"damagePercent":10,"radiusM":1.6,"innerRadiusM":0,"lengthM":0,"halfWidthM":0,"angleDegrees":0,"offsetForwardM":0,"offsetRightM":0,"yawOffsetDegrees":0,"forcePush":true,"pushRangeM":4,"riseHeightM":2,"pushMs":1000}],"countPerPlayer":1,"radiusM":0,"effectLifetimeMs":7000} | P39.logic.12:D; P120.logic.9:D |
| [L75](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2778) | 세이튼_공먹고휘리릭_사라지기 | ALBION_AIRBORNE | {"airbornePhase":"JUMP","airborneHeightM":13.678813305200356,"airborneDurationMs":200,"teleportPosition":[0,0,0]} | P79.logic.1:D; P119.logic.1:D; P122.logic.1:D |
| [L76](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2820) | 세이튼_공먹고휘리릭_플레이어추적 | ALBION_AIRBORNE | {"airbornePhase":"APPEAR_PLAYER","airborneHeightM":13.678813305200356,"airborneDurationMs":0,"teleportPosition":[0,0,0]} | P79.logic.2:D; P119.logic.2:D; P122.logic.2:D |
| [L77](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2862) | 쿠크_훌라후프_상단시작 | ALBION_AIRBORNE | {"airbornePhase":"JUMP","airborneHeightM":10.890836241085884,"airborneDurationMs":0,"teleportPosition":[0,0,0]} | P84.logic.1:D; P90.logic.1:D |
| [L78](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2896) | 세이튼_비둘기마술쇼_사라지기 | — | {} | 미참조 |
| [L79](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2904) | 세이튼_비둘기마술쇼_중앙이동 | BOSS_TELEPORT_XZ | {"teleportPosition":[-0.07,1.32,737.53]} | P82.logic.4:D |
| [L80](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2927) | 세이튼_공먹기_시작위치고정 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P79.logic.5:D |
| [L83](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3074) | 쿠크_훌라후프_하강착지 | ALBION_AIRBORNE | {"airbornePhase":"SLAM","airborneHeightM":0,"airborneDurationMs":0,"teleportPosition":[0,0,0]} | P84.logic.3:D; P90.logic.2:D |
| [L87](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3180) | 세이튼_카드맞추기_플레이어속박  | — | {} | P78.logic.3:D [off] |
| [L88](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3197) | 쿠크_피자_쿠크시작위치이동 | BOSS_TELEPORT_XZ | {"teleportPosition":[6.36000013,10.5600004,321.290009]} | 미참조 |
| [L89](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3211) | 마리오_2페이즈_플레이어오른쪽이동 | MARIO_PHASE2_PLAYERS | {"teleportPosition":[-5.814691867476644,1.3200000524520874,934.4608532917484]} | P33.logic.2:D |
| [L90](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3234) | 마리오_2페이즈_쿠크세이튼왼쪽이동 | BOSS_TELEPORT_FACE_CENTER | {"teleportPosition":[5.96,1.3,950.59]} | P33.logic.3:D |
| [L97](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3419) | [source attack] source damage contact | ENTER_AREA | {} | P11.logic.4:D; P11.logic.5:D; P11.logic.6:D; P15.logic.7:D; P15.logic.8:D; P15.logic.9:D; P17.logic.3:D [off]; P17.logic.4:D [off]; P17.logic.5:D; P17.logic.6:D [off]; P17.logic.7:D [off]; P17.logic.8:D [off]; P17.logic.9:D; P17.logic.10:D; P17.logic.11:D; P25.logic.2:D; P25.logic.3:D; P25.logic.4:D; P25.logic.5:D; P25.logic.6:D; P25.logic.7:D; P25.logic.8:D; P25.logic.9:D; P25.logic.10:D; P25.logic.11:D; P25.logic.12:D; P25.logic.13:D; P25.logic.15:D; P23.logic.1:D; P23.logic.2:D; P23.logic.3:D; P23.logic.4:D; P23.logic.5:D; P23.logic.6:D; P23.logic.7:D; P24.logic.4:D; P24.logic.5:D; P24.logic.6:D; P24.logic.7:D; P24.logic.8:D; P24.logic.9:D; P24.logic.10:D; P24.logic.11:D; P24.logic.12:D; P24.logic.13:D; P24.logic.14:D; P24.logic.15:D; P24.logic.16:D; P24.logic.17:D; P24.logic.18:D; P24.logic.19:D; P27.logic.1:D [off]; P35.logic.18:D; P35.logic.19:D; P35.logic.20:D; P35.logic.21:D; P35.logic.22:D; P35.logic.23:D; P35.logic.24:D; P35.logic.25:D; P35.logic.26:D; P35.logic.27:D; P35.logic.28:D; P35.logic.29:D; P35.logic.30:D; P35.logic.31:D; P35.logic.32:D; P35.logic.33:D; P35.logic.34:D; P35.logic.35:D; P35.logic.36:D; P35.logic.37:D; P35.logic.38:D; P35.logic.39:D; P35.logic.40:D; P35.logic.41:D; P35.logic.42:D; P35.logic.43:D; P35.logic.44:D; P35.logic.45:D; P35.logic.46:D; P35.logic.47:D; P35.logic.48:D; P35.logic.49:D; P35.logic.50:D; P35.logic.51:D; P35.logic.52:D; P35.logic.53:D; P35.logic.54:D; P35.logic.55:D; P35.logic.56:D; P35.logic.57:D; P35.logic.58:D; P35.logic.59:D; P35.logic.60:D; P35.logic.61:D; P35.logic.62:D; P35.logic.63:D; P35.logic.64:D; P35.logic.65:D; P35.logic.66:D; P35.logic.67:D; P35.logic.68:D; P35.logic.69:D; P35.logic.70:D; P35.logic.71:D; P35.logic.72:D; P35.logic.73:D; P35.logic.74:D; P35.logic.75:D; P35.logic.76:D; P35.logic.77:D; P35.logic.78:D; P35.logic.79:D; P35.logic.80:D; P35.logic.81:D; P35.logic.82:D; P35.logic.83:D; P35.logic.84:D; P35.logic.85:D; P35.logic.86:D; P35.logic.87:D; P35.logic.88:D; P35.logic.89:D; P35.logic.90:D; P35.logic.91:D; P35.logic.92:D; P35.logic.93:D; P35.logic.94:D; P35.logic.95:D; P35.logic.96:D; P35.logic.97:D; P35.logic.98:D; P35.logic.99:D; P35.logic.100:D; P35.logic.101:D; P35.logic.102:D; P35.logic.103:D; P35.logic.104:D; P35.logic.105:D; P35.logic.106:D; P35.logic.107:D; P35.logic.108:D; P35.logic.109:D; P35.logic.110:D; P35.logic.111:D; P35.logic.112:D; P35.logic.113:D; P35.logic.114:D; P35.logic.115:D; P35.logic.116:D; P35.logic.117:D; P35.logic.118:D; P35.logic.119:D; P35.logic.120:D; P35.logic.121:D; P35.logic.122:D; P35.logic.123:D; P35.logic.124:D; P35.logic.125:D; P35.logic.126:D; P35.logic.127:D; P35.logic.128:D; P46.logic.2:D; P48.logic.4:D; P48.logic.5:D; P48.logic.6:D; P48.logic.7:D; P48.logic.8:D; P48.logic.9:D; P79.logic.6:D; P105.logic.1:D; P118.logic.3:D; P118.logic.4:D; P118.logic.5:D; P118.logic.6:D; P118.logic.7:D; P118.logic.8:D; P118.logic.9:D; P118.logic.10:D; P118.logic.11:D; P118.logic.12:D; P118.logic.13:D; P118.logic.14:D; P118.logic.15:D; P118.logic.16:D; P118.logic.17:D; P118.logic.18:D; P118.logic.19:D; P119.logic.5:D; P122.logic.5:D |
| [L100](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:6884) | Showtime center return to server floor | BOSS_TELEPORT_GROUNDED | {"teleportPosition":[-0.0700000003,1.32000005,942.330017]} | P35.logic.17:D |
| [L102](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:6934) | 대형세이튼_뿅망치넉백 | ENTER_AREA | {} | 미참조 |
| [L103](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:6943) | 쿠크_중앙이동 | BOSS_TELEPORT_GROUNDED | {"teleportPosition":[4.66,10.56,322.94]} | P25.logic.14:D; P23.logic.12:D |
| [L112](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7177) | 카드미로_플레이어이동 | CARD_MAZE_STAGE_PLAYERS | {"playerEntryEffectOccurrenceIds":["KOUKUSAYDON_G1_PATTERN_77.presentation.21","KOUKUSAYDON_G1_PATTERN_77.presentation.22","KOUKUSAYDON_G1_PATTERN_77.presentation.23","KOUKUSAYDON_G1_PATTERN_77.presentation.24"]} | P77.logic.7:D |
| [L120](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7385) | 트리거_플레이어타겟설정 | — | {} | P21.logic.9:D; P58.logic.9:D; P58.logic.10:D; P85.logic.4:D |
| [L125](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7458) | 세이튼_카드출력_하트 | PURSUIT_PROJECTILES | 큰 값: projectileHits, visualIds, cardSymbols, contactVisualId, speedMps, contactRadiusM, spawnRadiusM, lifetimeMs, spawnIntervalMs, homing, countPerWave — ID 링크에서 원문 확인 | P78.logic.9:D |
| [L126](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7512) | 세이튼_카드출력_스페이드 | PURSUIT_PROJECTILES | 큰 값: projectileHits, visualIds, cardSymbols, contactVisualId, speedMps, contactRadiusM, spawnRadiusM, lifetimeMs, spawnIntervalMs, homing, countPerWave — ID 링크에서 원문 확인 | P78.logic.10:D |
| [L127](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7566) | 세이튼_카드출력_클로버 | PURSUIT_PROJECTILES | 큰 값: projectileHits, visualIds, cardSymbols, contactVisualId, speedMps, contactRadiusM, spawnRadiusM, lifetimeMs, spawnIntervalMs, homing, countPerWave — ID 링크에서 원문 확인 | P78.logic.11:D |
| [L128](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7620) | 세이튼_카드출력_다이아 | PURSUIT_PROJECTILES | 큰 값: projectileHits, visualIds, cardSymbols, contactVisualId, speedMps, contactRadiusM, spawnRadiusM, lifetimeMs, spawnIntervalMs, homing, countPerWave — ID 링크에서 원문 확인 | P78.logic.12:D |
| [L129](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7674) | 카드비_카드병정소환 | CARD_RAIN_SOLDIERS | {"soldierCounts":[3,3,3],"spawnRadiusMinM":3,"spawnRadiusMaxM":10,"soldierMaxHp":69000,"soldierDamage":13200} | P103.logic.1:D |
| [L130](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7701) | 카드비_카드낙하 | — | {} | 미참조 |
| [L133](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7818) | 세이튼_정면바람방구_타격영역 | ENTER_AREA | {} | P80.logic.2:D; P100.logic.1:D; P113.logic.1:D; P116.logic.1:D; P117.logic.2:D |
| [L136](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7964) | 세이튼_공먹기_시작위치고정 (copy) | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P122.logic.4:D |
| [L137](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8066) | 세이튼_공먹기_시작위치고정 (copy) | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P119.logic.4:D |
| [L138](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8168) | 빙고_블랙홀빔이동과중앙블랙홀 | BOSS_TELEPORT_FACE_CENTER | {"teleportPosition":[2.9,0,1143]} | P107.logic.1:D |
| [L139](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8191) | 앵콜세이튼_알비온_빙고중앙 | ALBION_AIRBORNE | {"airbornePhase":"CENTER","airborneHeightM":0,"airborneDurationMs":0,"teleportPosition":[0,0,1146.88]} | P120.logic.6:D |
| [L142](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8255) | 화염 파동_1회차 1열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L143](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8264) | 화염 파동_1회차 2열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L144](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8273) | 화염 파동_1회차 3열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L145](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8282) | 화염 파동_1회차 4열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L146](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8291) | 화염 파동_2회차 1열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L147](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8300) | 화염 파동_2회차 2열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L148](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8309) | 화염 파동_2회차 3열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L149](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8318) | 화염 파동_2회차 4열_접촉 1회 | ENTER_AREA | {} | 미참조 |
| [L501](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8340) | 불뿜기_접촉 500ms 반복 | ENTER_AREA | {"repeatIntervalMs":500} | 미참조 |
| [L505](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8510) | 콜라이더_원형_상승대미지 | — | {} | 미참조 |
| [L507](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8532) | 트리거_대미지_넉백 | ENTER_AREA | {"colliderDamageContactRole":"KNOCKBACK"} | P7.logic.6:D; P8.logic.2:D; P8.logic.3:D; P8.logic.4:D; P8.logic.5:D; P8.logic.6:D; P8.logic.7:D; P8.logic.8:D; P8.logic.9:D; P8.logic.10:D; P8.logic.11:D; P13.logic.26:D; P13.logic.27:D; P13.logic.28:D; P13.logic.29:D; P26.logic.1:D; P47.logic.11:D; P47.logic.12:D; P58.logic.1:D; P58.logic.2:D; P58.logic.3:D; P58.logic.4:D; P58.logic.5:D; P58.logic.6:D; P58.logic.7:D; P58.logic.8:D; P59.logic.1:D; P59.logic.2:D; P59.logic.3:D; P59.logic.4:D; P59.logic.5:D; P59.logic.6:D; P59.logic.7:D; P59.logic.8:D; P59.logic.9:D; P59.logic.10:D; P59.logic.11:D; P59.logic.12:D; P59.logic.13:D; P59.logic.14:D; P59.logic.15:D; P59.logic.16:D; P59.logic.17:D; P59.logic.18:D; P59.logic.19:D; P59.logic.20:D; P59.logic.21:D; P59.logic.22:D; P59.logic.23:D; P59.logic.24:D; P59.logic.25:D; P59.logic.26:D; P59.logic.27:D; P59.logic.28:D; P62.logic.1:D; P86.logic.1:D; P86.logic.2:D; P87.logic.1:D; P87.logic.2:D; P87.logic.3:D; P99.logic.2:D; P123.logic.1:D |
| [L508](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:9039) | 트리거_대미지 | ENTER_AREA | {"colliderDamageContactRole":"DAMAGE"} | P38.logic.2:D; P38.logic.3:D; P38.logic.4:D; P38.logic.5:D; P38.logic.6:D; P38.logic.7:D; P38.logic.8:D; P38.logic.9:D; P38.logic.10:D; P38.logic.11:D; P38.logic.12:D; P38.logic.13:D; P38.logic.14:D; P38.logic.15:D; P38.logic.16:D; P38.logic.17:D; P38.logic.18:D; P38.logic.19:D; P38.logic.20:D; P38.logic.21:D; P39.logic.13:D; P39.logic.14:D; P39.logic.15:D; P39.logic.16:D; P39.logic.17:D; P39.logic.18:D; P39.logic.19:D; P39.logic.20:D; P39.logic.21:D; P39.logic.22:D; P49.logic.1:D; P49.logic.2:D; P49.logic.3:D; P49.logic.4:D; P49.logic.5:D; P49.logic.6:D; P49.logic.7:D; P49.logic.8:D; P49.logic.9:D; P49.logic.10:D; P49.logic.11:D; P49.logic.12:D; P49.logic.13:D; P49.logic.14:D; P58.logic.11:D; P58.logic.12:D; P58.logic.13:D; P58.logic.14:D; P58.logic.15:D; P58.logic.16:D; P58.logic.17:D; P58.logic.18:D; P58.logic.19:D; P58.logic.20:D; P58.logic.21:D; P58.logic.22:D; P58.logic.23:D; P58.logic.24:D; P58.logic.25:D; P58.logic.26:D; P58.logic.27:D; P58.logic.28:D; P58.logic.29:D; P58.logic.30:D; P58.logic.31:D; P58.logic.32:D; P58.logic.33:D; P58.logic.34:D; P58.logic.35:D; P58.logic.36:D; P58.logic.37:D; P58.logic.38:D; P79.logic.7:D; P82.logic.5:D; P82.logic.6:D; P82.logic.7:D; P82.logic.8:D; P82.logic.9:D; P82.logic.10:D; P82.logic.11:D; P82.logic.12:D; P82.logic.13:D; P83.logic.2:D; P83.logic.3:D; P83.logic.4:D; P85.logic.2:D; P85.logic.3:D; P109.logic.1:D; P115.logic.1:D; P115.logic.2:D; P115.logic.3:D; P115.logic.4:D; P115.logic.5:D; P115.logic.6:D; P115.logic.7:D; P115.logic.8:D; P115.logic.9:D; P115.logic.10:D; P115.logic.11:D; P115.logic.12:D; P115.logic.13:D; P115.logic.14:D; P115.logic.15:D; P115.logic.16:D; P115.logic.17:D; P115.logic.18:D; P115.logic.19:D; P115.logic.20:D; P115.logic.21:D; P115.logic.22:D; P115.logic.23:D; P115.logic.24:D; P115.logic.25:D; P115.logic.26:D; P115.logic.27:D; P115.logic.28:D; P119.logic.6:D; P120.logic.10:D; P120.logic.11:D; P120.logic.12:D; P120.logic.13:D; P120.logic.14:D; P120.logic.15:D; P120.logic.16:D; P120.logic.17:D; P120.logic.18:D; P120.logic.19:D; P122.logic.6:D |
| [L512](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10129) | 트리거_대미지 | ENTER_AREA | {"colliderDamageContactRole":"DAMAGE","repeatIntervalMs":500} | P81.logic.22:D [off] |
| [L517](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11137) | 트리거_상승넉백대미지 | — | {} | 미참조 |
| [L524](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11602) | 빙고_블랙홀_폭발_13줄 | BINGO_DETONATION | {} | P128.logic.1:D |
| [L527](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11660) | 플레이어 방향 즉시 바라보기 | BOSS_TRACK_TARGET | {} | P116.logic.2:D |
| [L528](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11678) | 트럼펫 문양장판 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | 미참조 |
| [L529](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11724) | 트럼펫 문양장판 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | 미참조 |
| [L530](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11770) | 저글링 투척 직전 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, selectedFlightMs, selectedFlightArcHeightM, selectedFlightSourceOffset, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P106.logic.1:D |
| [L531](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11827) | 저글링 투척 직전 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, selectedFlightMs, selectedFlightArcHeightM, selectedFlightSourceOffset, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P106.logic.2:D |
| [L532](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11884) | 저글링 투척 직전 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, selectedFlightMs, selectedFlightArcHeightM, selectedFlightSourceOffset, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P106.logic.3:D |
| [L533](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11941) | 저글링 투척 직전 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, selectedFlightMs, selectedFlightArcHeightM, selectedFlightSourceOffset, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P106.logic.4:D |
| [L534](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11998) | 저글링 투척 직전 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, selectedFlightMs, selectedFlightArcHeightM, selectedFlightSourceOffset, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P106.logic.5:D |
| [L535](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12055) | 저글링 투척 직전 플레이어 위치 포착 | ALBION_AIRBORNE | 큰 값: fixedHits, airbornePhase, airborneHeightM, airborneDurationMs, teleportPosition, selectedFlightMs, selectedFlightArcHeightM, selectedFlightSourceOffset, airborneTargetPositionPolicy, selectedEffectGroupId — ID 링크에서 원문 확인 | P106.logic.6:D |
| [L536](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12112) | 십자 화염 폭발 접촉 | ENTER_AREA | {} | 미참조 |
| [L537](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12121) | 기분나빠 분신 전 최근접 플레이어 조준 | BOSS_TRACK_TARGET | {} | P52.logic.1:D |
| [L546](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13177) | 조커 찾기 랜덤 표적 설정 | BOSS_RANDOM_TARGET | {} | P13.logic.30:D [off]; P13.logic.31:D [off]; P13.logic.32:D [off] |
| [L548](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13238) | 트리거_가장가까운플레이어방향회전 | BOSS_TRACK_TARGET | {} | P50.logic.2:D; P62.logic.2:D; P123.logic.2:D |

### RESULT (77)

| ID (전체값 링크) | 한글 이름 | kind | 추가 파라미터 실제값 | 참조 occurrence |
|---|---|---|---|---|
| [L2](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:27) | 세이튼로직_전원전멸 | INSTANT_DEATH | {"percent":0,"durationMs":0,"followupPatternId":""} | P1.logic.1:T; P13.logic.6:T; P33.logic.1:T |
| [L3](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:64) | 세이튼로직_즉사 | INSTANT_DEATH | {"percent":0,"durationMs":0,"followupPatternId":""} | P2.logic.3:F; P11.logic.4:S; P11.logic.5:S; P11.logic.6:S |
| [L4](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:109) | 세이튼로직_최대체력20%감소 | MAX_HP_PERCENT_DAMAGE | {"percent":20,"durationMs":0,"followupPatternId":""} | P6.logic.2:F; P6.logic.2:T; P6.logic.3:F; P6.logic.3:T |
| [L5](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:140) | 세이튼로직_최대체력10%감소 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":""} | P6.logic.1:F; P6.logic.1:T; P7.logic.1:F; P7.logic.1:T; P7.logic.2:F; P7.logic.2:T; P7.logic.3:F; P7.logic.3:T |
| [L6](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:189) | 세이튼로직_최대체력50%감소 | MAX_HP_PERCENT_DAMAGE | {"percent":50,"durationMs":0,"followupPatternId":""} | P6.logic.4:F; P6.logic.4:T; P7.logic.5:F; P11.logic.1:F; P11.logic.2:F; P11.logic.3:F |
| [L7](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:243) | 세이튼로직_무력화성공 | FOLLOWUP_PATTERN | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_4"} | P1.logic.1:S; P81.logic.1:S |
| [L8](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:272) | 세이튼로직_삐에로변신 | CLOWN_TRANSFORM | {"percent":0,"durationMs":0,"followupPatternId":""} | P7.logic.2:F; P7.logic.2:T; P7.logic.3:F; P7.logic.3:T |
| [L15](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:452) | 세이튼로직_광기게이지50% | MADNESS_GAUGE_ADD_PERCENT | {"percent":50,"durationMs":0,"followupPatternId":""} | P6.logic.1:F; P6.logic.1:T; P6.logic.2:F; P6.logic.2:T; P6.logic.3:F; P6.logic.3:T; P6.logic.4:F; P6.logic.4:T; P7.logic.1:F; P7.logic.1:T; P7.logic.5:F |
| [L21](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:663) | 2관문_거대세이튼_조커찾기성공 | FOLLOWUP_PATTERN | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_14"} | P13.logic.6:S |
| [L24](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:776) | 대형세이튼결과_카드뒤집기_유지 | PLAY_CONTACT_WORLD_OBJECT_MOTION | 큰 값: percent, durationMs, followupPatternId, contactMotions — ID 링크에서 원문 확인 | P13.logic.2:S; P13.logic.17:S; P13.logic.23:S |
| [L25](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:843) | 대형세이튼결과_카드들썩임_Idle | PLAY_CONTACT_WORLD_OBJECT_MOTION | 큰 값: percent, durationMs, followupPatternId, contactMotions — ID 링크에서 원문 확인 | P13.logic.3:S; P13.logic.14:S; P13.logic.22:S |
| [L26](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:910) | 대형세이튼결과_조커중앙성공 | COMPLETE_LOGIC_WINDOW | {"percent":0,"durationMs":0,"followupPatternId":"","targetLogicOccurrenceId":"KOUKUSAYDON_G1_PATTERN_13.logic.6","contactTargetWorldOccurrenceId":"KOUKUSAYDON_G1_PATTERN_13.world.11"} | P13.logic.2:S; P13.logic.17:S; P13.logic.23:S |
| [L37](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:986) | 2관문_거대세이튼_파1빨2공포 | FEAR | {"percent":0,"durationMs":3000,"followupPatternId":"","sceneProfileId":"koukusaydon.g1.sceneprofile.1","effectResourceId":"","lightResourceId":"koukusaydon.g1.presentation.17","effectDelayMs":0} | P11.logic.1:F; P11.logic.2:F; P11.logic.3:F; P94.logic.1:F |
| [L30](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1070) | 2관문_거미카운터_카운터성공 | FOLLOWUP_PATTERN | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_22"} | P15.logic.1:S; P15.logic.2:S; P15.logic.3:S |
| [L34](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1179) | 대형세이튼_왼손부착 | CAPTURE_PLAYER | {"percent":0,"durationMs":0,"followupPatternId":"","attachmentSlot":"BOSS_LEFT_HAND","gripLocalOffset":{"forwardM":0,"upM":0,"rightM":0}} | P17.logic.1:S |
| [L35](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1206) | 2관문_거미카운터_공포얼굴 | FEAR | {"percent":0,"durationMs":3000,"followupPatternId":"","sceneProfileId":"koukusaydon.g1.sceneprofile.2","effectResourceId":"koukusaydon.effect.94502eeace8ae6ae07bf","lightResourceId":"","effectDelayMs":1000,"soundResourceId":"sound.kouku.018b3ad1ae968ba781cc"} | P15.logic.4:S; P15.logic.5:S; P15.logic.6:S |
| [L28](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1257) | 3관문_갈고리끝_매달려끌려감 | GRAB_TO_WORLD_OBJECT | {"percent":0,"durationMs":0,"followupPatternId":""} | 미참조 |
| [L39](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1290) | Whirlwind hit + knockback | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":2,"pushMs":242,"forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | P24.logic.1:S |
| [L42](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1446) | Laser hit + boss-forward knockback | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":2,"pushMs":242,"pushDirection":"BOSS_FORWARD"} | 미참조 |
| [L48](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1586) | 결과_피해데미지 | — | {} | 미참조 |
| [L49](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1594) | 결과_무력화 | — | {} | 미참조 |
| [L50](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1602) | 결과_화상 | — | {} | 미참조 |
| [L51](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1610) | 결과_카운터 | — | {} | 미참조 |
| [L57](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1730) | 쿠크세이튼_무력화성공 | FOLLOWUP_PATTERN | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_42"} | P33.logic.1:S; P118.logic.1:S |
| [L60](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1866) | 마리오_2페이즈패턴진입 | FOLLOWUP_PATTERN | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_33"} | P34.logic.1:S |
| [L61](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:1887) | 마리오_서버입장 | MARIO_ENTER | {"percent":0,"durationMs":0,"followupPatternId":""} | P34.logic.3:S |
| [L71](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:2609) | 갈고리납치 | — | {} | 미참조 |
| [L91](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3257) | 세이튼_돌진카운터_그로기 | FOLLOWUP_PATTERN | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_4"} | P80.logic.1:S |
| [L92](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3278) | 마리오_실패_전원즉사 | INSTANT_DEATH | {"percent":0,"durationMs":0,"followupPatternId":""} | P34.logic.3:T; P88.logic.1:T; P91.logic.1:T; P92.logic.1:T; P93.logic.1:T |
| [L93](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3331) | 마리오_1단계_입장 | MARIO_ENTER | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_33","marioStage":1} | P88.logic.1:S |
| [L94](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3353) | 마리오_2단계_입장 | MARIO_ENTER | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_33","marioStage":2} | P91.logic.1:S |
| [L95](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3375) | 마리오_3단계_입장 | MARIO_ENTER | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_33","marioStage":3} | P92.logic.1:S |
| [L96](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:3397) | 마리오_4단계_입장 | MARIO_ENTER | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_33","marioStage":4} | P93.logic.1:S |
| [L98](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:4957) | [source attack] max HP 10 percent (tuning) | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":""} | P15.logic.7:S; P15.logic.8:S; P15.logic.9:S; P17.logic.5:S; P17.logic.6:S; P17.logic.7:S; P17.logic.8:S; P17.logic.9:S; P17.logic.10:S; P17.logic.11:S; P25.logic.2:S; P25.logic.3:S; P25.logic.4:S; P25.logic.5:S; P25.logic.6:S; P25.logic.7:S; P25.logic.8:S; P25.logic.9:S; P25.logic.10:S; P25.logic.11:S; P25.logic.12:S; P25.logic.13:S; P25.logic.15:S; P24.logic.4:S; P24.logic.5:S; P24.logic.6:S; P24.logic.7:S; P24.logic.8:S; P24.logic.9:S; P24.logic.10:S; P24.logic.11:S; P24.logic.12:S; P24.logic.13:S; P24.logic.14:S; P24.logic.15:S; P24.logic.16:S; P24.logic.17:S; P24.logic.18:S; P24.logic.19:S; P35.logic.18:S; P35.logic.19:S; P35.logic.20:S; P35.logic.21:S; P35.logic.22:S; P35.logic.23:S; P35.logic.24:S; P35.logic.25:S; P35.logic.26:S; P35.logic.27:S; P35.logic.28:S; P35.logic.29:S; P35.logic.30:S; P35.logic.31:S; P35.logic.32:S; P35.logic.33:S; P35.logic.34:S; P35.logic.35:S; P35.logic.36:S; P35.logic.37:S; P35.logic.38:S; P35.logic.39:S; P35.logic.40:S; P35.logic.41:S; P35.logic.42:S; P35.logic.43:S; P35.logic.44:S; P35.logic.45:S; P35.logic.46:S; P35.logic.47:S; P35.logic.48:S; P35.logic.49:S; P35.logic.50:S; P35.logic.51:S; P35.logic.52:S; P35.logic.53:S; P35.logic.54:S; P35.logic.55:S; P35.logic.56:S; P35.logic.57:S; P35.logic.58:S; P35.logic.59:S; P35.logic.60:S; P35.logic.61:S; P35.logic.62:S; P35.logic.63:S; P35.logic.64:S; P35.logic.65:S; P35.logic.66:S; P35.logic.67:S; P35.logic.68:S; P35.logic.69:S; P35.logic.70:S; P35.logic.71:S; P35.logic.72:S; P35.logic.73:S; P35.logic.74:S; P35.logic.75:S; P35.logic.76:S; P35.logic.77:S; P35.logic.78:S; P35.logic.79:S; P35.logic.80:S; P35.logic.81:S; P35.logic.82:S; P35.logic.83:S; P35.logic.84:S; P35.logic.85:S; P35.logic.86:S; P35.logic.87:S; P35.logic.88:S; P35.logic.89:S; P35.logic.90:S; P35.logic.91:S; P35.logic.92:S; P35.logic.93:S; P35.logic.94:S; P35.logic.95:S; P35.logic.96:S; P35.logic.97:S; P35.logic.98:S; P39.logic.20:S; P39.logic.21:S; P46.logic.2:S; P48.logic.4:S; P48.logic.5:S; P48.logic.6:S; P48.logic.7:S; P48.logic.8:S; P48.logic.9:S; P49.logic.1:S; P49.logic.2:S; P49.logic.3:S; P49.logic.4:S; P49.logic.5:S; P49.logic.6:S; P49.logic.7:S; P49.logic.8:S; P49.logic.9:S; P49.logic.10:S; P49.logic.11:S; P49.logic.12:S; P49.logic.13:S; P49.logic.14:S; P58.logic.11:S; P58.logic.12:S; P58.logic.13:S; P58.logic.14:S; P58.logic.15:S; P58.logic.16:S; P58.logic.17:S; P58.logic.18:S; P58.logic.19:S; P58.logic.20:S; P58.logic.21:S; P58.logic.22:S; P58.logic.23:S; P58.logic.24:S; P58.logic.25:S; P58.logic.26:S; P58.logic.27:S; P58.logic.28:S; P58.logic.29:S; P58.logic.30:S; P58.logic.31:S; P58.logic.32:S; P58.logic.33:S; P58.logic.34:S; P58.logic.35:S; P58.logic.36:S; P58.logic.37:S; P58.logic.38:S; P79.logic.6:S; P79.logic.7:S; P82.logic.5:S; P82.logic.6:S; P82.logic.7:S; P82.logic.8:S; P82.logic.9:S; P82.logic.10:S; P82.logic.11:S; P82.logic.12:S; P82.logic.13:S; P83.logic.2:S; P83.logic.3:S; P83.logic.4:S; P105.logic.1:S; P115.logic.1:S; P115.logic.2:S; P115.logic.3:S; P115.logic.4:S; P115.logic.5:S; P115.logic.6:S; P115.logic.7:S; P115.logic.8:S; P115.logic.9:S; P115.logic.10:S; P115.logic.11:S; P115.logic.12:S; P115.logic.13:S; P115.logic.14:S; P115.logic.15:S; P115.logic.16:S; P115.logic.17:S; P115.logic.18:S; P115.logic.19:S; P115.logic.20:S; P115.logic.21:S; P115.logic.22:S; P115.logic.23:S; P115.logic.24:S; P115.logic.25:S; P115.logic.26:S; P115.logic.27:S; P115.logic.28:S; P118.logic.3:S; P118.logic.4:S; P118.logic.5:S; P118.logic.6:S; P118.logic.7:S; P118.logic.8:S; P118.logic.9:S; P118.logic.10:S; P118.logic.11:S; P118.logic.12:S; P118.logic.13:S; P118.logic.14:S; P118.logic.15:S; P118.logic.16:S; P118.logic.17:S; P118.logic.18:S; P118.logic.19:S; P119.logic.5:S; P119.logic.6:S; P120.logic.17:S; P120.logic.18:S; P122.logic.5:S; P122.logic.6:S |
| [L104](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:6974) | Kouku laser and bazooka hit + 6m knockback | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":90} | 미참조 |
| [L105](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:6992) | Big Saydon contact hit + forced 16m knockback | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":16,"pushMs":242,"forcePush":true,"pushCanLeaveArena":true} | P17.logic.3:S; P17.logic.4:S; P27.logic.1:S |
| [L106](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7033) | Kouku angled laser hit + 6m knockback (135.9000015258789 deg) | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":135.9000015258789} | 미참조 |
| [L107](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7051) | Kouku angled laser hit + 6m knockback (45 deg) | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":45} | 미참조 |
| [L108](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7069) | Fanfare hit + 6m knockback | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | P23.logic.8:S; P23.logic.9:S; P23.logic.10:S; P23.logic.11:S |
| [L109](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7120) | Laser immediate 6m knockback (90 deg) | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":90} | 미참조 |
| [L110](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7139) | Laser immediate 6m knockback (135.9000015258789 deg) | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":135.9000015258789} | 미참조 |
| [L111](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7158) | Laser immediate 6m knockback (45 deg) | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":45} | 미참조 |
| [L113](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7201) | Fanfare sector immediate 6m push 1 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":90} | P23.logic.4:S |
| [L114](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7229) | Fanfare sector immediate 6m push 2 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | P23.logic.5:S |
| [L115](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7256) | Fanfare sector immediate 6m push 3 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":-75.69999694824219} | P23.logic.6:S |
| [L116](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7284) | Fanfare sector immediate 6m push 4 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":242,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true,"pushYawOffsetDegrees":-180} | P23.logic.7:S |
| [L117](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7312) | 바주카_피해_넉백 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":12,"pushMs":1200,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushHeightM":3,"pushBallistic":true,"pushYawOffsetDegrees":90} | P85.logic.2:S; P85.logic.3:S |
| [L118](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7349) | Big Saydon slam: contact-center ballistic throw | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":30,"pushMs":1500,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | 미참조 |
| [L119](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7367) | Big Saydon wind: high ballistic throw | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":16,"pushMs":1500,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | 미참조 |
| [L134](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:7868) | 세이튼_정면바람방구_포물선날림 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":5.1,"pushMs":2161,"forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | P62.logic.1:S; P80.logic.2:S; P100.logic.1:S; P113.logic.1:S; P116.logic.1:S; P117.logic.2:S; P123.logic.1:S |
| [L140](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8217) | 쿠크세이튼_돌진카운터_3관문그로기 | FOLLOWUP_PATTERN | {"percent":0,"durationMs":0,"followupPatternId":"KOUKUSAYDON_G1_PATTERN_42"} | P117.logic.1:S |
| [L141](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8238) | 화염 파동_불기둥 피해와 수직 띄우기 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":0,"pushMs":2161,"forcePush":true,"pushHeightM":5.720653225000001,"pushBallistic":true} | 미참조 |
| [L500](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8327) | 불뿜기_매 틱 고정 피해 500 | FIXED_DAMAGE | {"percent":0,"durationMs":0,"followupPatternId":"","damageAmount":500} | 미참조 |
| [L502](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8350) | 레이저_피해 및 상승 하강 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":1200,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushHeightM":3,"pushBallistic":true,"pushYawOffsetDegrees":90} | P21.logic.10:S; P21.logic.11:S; P21.logic.12:S; P21.logic.13:S; P21.logic.14:S; P21.logic.15:S; P21.logic.16:S |
| [L503](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8427) | 레이저_피해 및 상승 하강 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":1200,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushHeightM":3,"pushBallistic":true,"pushYawOffsetDegrees":135.9000015258789} | 미참조 |
| [L504](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8447) | 레이저_피해 및 상승 하강 | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":6,"pushMs":1200,"pushDirection":"BOSS_FORWARD","forcePush":true,"pushCanLeaveArena":true,"pushHeightM":3,"pushBallistic":true,"pushYawOffsetDegrees":45} | 미참조 |
| [L150](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8467) | Contact hit: vertical rise 3m / fall 1200ms | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":0,"pushMs":1200,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushHeightM":3,"pushBallistic":true} | P23.logic.1:S; P23.logic.2:S; P23.logic.3:S |
| [L506](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:8518) | 콜라이더_원형_상승데미지 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":2,"pushMs":500} | 미참조 |
| [L509](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10074) | 콜라이더_원형_상승데미지 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":1,"pushMs":250} | 미참조 |
| [L510](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10088) | 콜라이더_원형_상승데미지 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":1,"pushMs":250,"pushHeightM":2,"pushBallistic":true} | 미참조 |
| [L511](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10104) | 콜라이더_원형_상승데미지 Damage 20% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":20,"durationMs":0,"followupPatternId":"","pushRangeM":1,"pushMs":250,"pushHeightM":2,"pushBallistic":true} | P7.logic.6:S |
| [L515](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10597) | 일반불뿜기피해 | FIXED_DAMAGE | {"percent":0,"durationMs":0,"followupPatternId":"","damageAmount":100} | P27.logic.3:S; P40.logic.1:S; P43.logic.1:S; P43.logic.2:S; P43.logic.3:S; P43.logic.4:S; P43.logic.5:S; P43.logic.6:S; P43.logic.7:S; P43.logic.8:S; P50.logic.1:S; P53.logic.1:S; P54.logic.1:S; P55.logic.1:S; P60.logic.1:S; P63.logic.1:S; P67.logic.1:S; P68.logic.1:S; P69.logic.1:S; P70.logic.1:S; P81.logic.22:S; P81.logic.23:S; P102.logic.1:S; P102.logic.2:S; P102.logic.3:S; P102.logic.4:S; P102.logic.5:S; P102.logic.6:S; P102.logic.7:S; P102.logic.8:S; P102.logic.9:S; P102.logic.10:S; P102.logic.11:S; P108.logic.1:S; P112.logic.1:S; P114.logic.1:S; P114.logic.2:S; P114.logic.3:S; P114.logic.4:S; P114.logic.5:S; P114.logic.6:S; P114.logic.7:S; P114.logic.8:S; P118.logic.20:S |
| [L516](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:10963) | 인형공불뿜기피해 | FIXED_DAMAGE | {"percent":0,"durationMs":0,"followupPatternId":"","damageAmount":10} | P34.logic.5:S; P88.logic.3:S; P88.logic.4:S; P88.logic.5:S; P88.logic.6:S; P89.logic.3:S; P89.logic.4:S; P89.logic.5:S; P91.logic.3:S; P91.logic.4:S; P91.logic.5:S; P91.logic.6:S; P92.logic.3:S; P92.logic.4:S; P92.logic.5:S; P92.logic.6:S; P93.logic.3:S; P93.logic.4:S; P93.logic.5:S; P93.logic.6:S |
| [L518](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11145) | [source attack] KOUKUSAYDON_G1_PATTERN_79.5.421980404.1.008.10599 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":1,"pushMs":1200,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushHeightM":2,"pushBallistic":true} | 미참조 |
| [L519](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11163) | 화염 파동_불기둥 원통 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":0,"pushMs":2161,"forcePush":true,"pushHeightM":3,"pushBallistic":true} | P58.logic.5:S; P58.logic.6:S; P58.logic.7:S; P58.logic.8:S |
| [L520](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11213) | [source attack] KOUKUSAYDON_G1_PATTERN_87.1.422180601.1.005.1500 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":15,"pushMs":1500,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | 미참조 |
| [L521](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11231) | [source attack] KOUKUSAYDON_G1_PATTERN_87.1.422180601.1.005.1500 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":8,"pushMs":1500,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | P13.logic.26:S; P13.logic.27:S; P13.logic.28:S; P87.logic.1:S; P87.logic.2:S; P87.logic.3:S |
| [L522](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11298) | [source attack] KOUKUSAYDON_G1_PATTERN_86.3.422180104.1.009.3500 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":10,"pushMs":1500,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushCanLeaveArena":true,"pushBallistic":true} | P8.logic.2:S; P8.logic.3:S; P8.logic.4:S; P8.logic.5:S; P8.logic.6:S; P8.logic.7:S; P8.logic.8:S; P8.logic.9:S; P8.logic.10:S; P8.logic.11:S; P13.logic.29:S; P26.logic.1:S; P86.logic.1:S; P86.logic.2:S |
| [L523](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11429) | 인형·공_100ms당 광기5% | MADNESS_GAUGE_ADD_PERCENT | {"percent":5,"durationMs":0,"followupPatternId":""} | P34.logic.5:S; P88.logic.3:S; P88.logic.4:S; P88.logic.5:S; P88.logic.6:S; P89.logic.3:S; P89.logic.4:S; P89.logic.5:S; P91.logic.3:S; P91.logic.4:S; P91.logic.5:S; P91.logic.6:S; P92.logic.3:S; P92.logic.4:S; P92.logic.5:S; P92.logic.6:S; P93.logic.3:S; P93.logic.4:S; P93.logic.5:S; P93.logic.6:S |
| [L526](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:11639) | 빙고_완성_30초_무적 | PLAYER_INVULNERABILITY | {"percent":0,"durationMs":30000,"followupPatternId":""} | P107.logic.2:S |
| [L538](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12139) | Playtest hit: 10% HP and ballistic knockdown | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":4,"pushMs":1000,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushHeightM":2,"pushBallistic":true} | P35.logic.99:S; P35.logic.100:S; P35.logic.101:S; P35.logic.102:S; P35.logic.103:S; P35.logic.104:S; P35.logic.105:S; P35.logic.106:S; P35.logic.107:S; P35.logic.108:S; P35.logic.109:S; P35.logic.110:S; P35.logic.111:S; P35.logic.112:S; P35.logic.113:S; P35.logic.114:S; P35.logic.115:S; P35.logic.116:S; P35.logic.117:S; P35.logic.118:S; P35.logic.119:S; P35.logic.120:S; P35.logic.121:S; P35.logic.122:S; P35.logic.123:S; P35.logic.124:S; P35.logic.125:S; P35.logic.126:S; P35.logic.127:S; P35.logic.128:S; P38.logic.2:S; P38.logic.3:S; P38.logic.4:S; P38.logic.5:S; P38.logic.6:S; P38.logic.7:S; P38.logic.8:S; P38.logic.9:S; P38.logic.10:S; P38.logic.11:S; P38.logic.12:S; P38.logic.13:S; P38.logic.14:S; P38.logic.15:S; P38.logic.16:S; P38.logic.17:S; P38.logic.18:S; P38.logic.19:S; P38.logic.20:S; P38.logic.21:S; P39.logic.13:S; P39.logic.14:S; P39.logic.15:S; P39.logic.16:S; P39.logic.17:S; P39.logic.18:S; P39.logic.19:S; P39.logic.22:S; P109.logic.1:S; P120.logic.10:S; P120.logic.11:S; P120.logic.12:S; P120.logic.13:S; P120.logic.14:S; P120.logic.15:S; P120.logic.16:S; P120.logic.19:S |
| [L539](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12694) | Breath contact madness 1% | MADNESS_GAUGE_ADD_PERCENT | {"percent":1,"durationMs":0,"followupPatternId":""} | P27.logic.3:S; P43.logic.1:S; P67.logic.1:S; P68.logic.1:S; P69.logic.1:S; P70.logic.1:S; P102.logic.9:S; P102.logic.10:S; P102.logic.11:S; P114.logic.1:S |
| [L544](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:12915) | Breath lingering flame: no additional madness | MADNESS_GAUGE_ADD_PERCENT | {"percent":0,"durationMs":0,"followupPatternId":""} | P40.logic.1:S; P43.logic.2:S; P43.logic.3:S; P43.logic.4:S; P43.logic.5:S; P43.logic.6:S; P43.logic.7:S; P43.logic.8:S; P50.logic.1:S; P53.logic.1:S; P54.logic.1:S; P55.logic.1:S; P60.logic.1:S; P102.logic.2:S; P102.logic.3:S; P102.logic.4:S; P102.logic.5:S; P102.logic.6:S; P102.logic.7:S; P102.logic.8:S; P108.logic.1:S; P112.logic.1:S; P114.logic.2:S; P114.logic.3:S; P114.logic.4:S; P114.logic.5:S; P114.logic.6:S; P114.logic.7:S; P114.logic.8:S |
| [L545](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13160) | 화염 파동 수직 넉백 3m | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":0,"pushMs":500,"forcePush":true,"pushHeightM":3,"pushBallistic":true} | 미참조 |
| [L549](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13272) | 대형세이튼_바람불기 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":8,"pushMs":1500,"pushDirection":"AWAY_FROM_CONTACT","forcePush":true,"pushCanLeaveArena":true,"pushHeightM":1,"pushBallistic":true} | P99.logic.2:S |
| [L554](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13392) | 문양장판폭발 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":5,"pushMs":500,"forcePush":true,"pushBallistic":true} | P47.logic.11:S; P47.logic.12:S |
| [L556](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/logic_definitions.json:13447) | 화염 파동_불기둥 원통 Damage 10% max HP | MAX_HP_PERCENT_DAMAGE | {"percent":10,"durationMs":0,"followupPatternId":"","pushRangeM":0,"pushMs":2000,"forcePush":true,"pushHeightM":3,"pushBallistic":true} | P58.logic.1:S; P58.logic.2:S; P58.logic.3:S; P58.logic.4:S; P59.logic.1:S; P59.logic.2:S; P59.logic.3:S; P59.logic.4:S; P59.logic.5:S; P59.logic.6:S; P59.logic.7:S; P59.logic.8:S; P59.logic.9:S; P59.logic.10:S; P59.logic.11:S; P59.logic.12:S; P59.logic.13:S; P59.logic.14:S; P59.logic.15:S; P59.logic.16:S; P59.logic.17:S; P59.logic.18:S; P59.logic.19:S; P59.logic.20:S; P59.logic.21:S; P59.logic.22:S; P59.logic.23:S; P59.logic.24:S; P59.logic.25:S; P59.logic.26:S; P59.logic.27:S; P59.logic.28:S |

## 데이터 근거와 한계

동일 revision은 저장본 식별의 일치 근거이며 projection 전체를 새로 실행한 검증을 대신하지 않는다. 이번에는 이미 저장된 Encounter/patternbindings를 읽었다.
전체 패턴 ID와 개수·중복을 대조했으며, 현재 제품 소스·JSON·PPT·빌드는 변경하지 않았다.

| 입력 | SHA-256 |
|---|---|
| [Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json) | `427575c883f3b56969b71c83cfc6f03e4dace0733faa8c6d034609b3ec2ef70d` |
| [Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json](C:/Users/tnest/Desktop/LostArk/Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json) | `b856d7a05547960c0ff2c3712f6f140ea8edf87b8cfe526d15306d8a80dbbe72` |
| [Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json) | `7fb9ed6888043d3f7e811f4a0f6b58c15a0f7667852530358e0e1e9b34ccba0c` |
