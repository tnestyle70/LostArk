# 쿠크 피자 마지막 내려찍기 고정 판정 계획

## G00. 현재 근거

Composition2496의 P26(대형세이튼_피자)에는 P86(두번내려치기)의 마지막 collider.283,
Logic507(KNOCKBACK)→522(최대HP10%,10m/1500ms, 강제 탄도 넉백)이 이미 연결돼 있다.
P26 Collider는 WEAPON/b_rpct_01을 따라16997~17097ms에 재생된다. 마지막 source action4221819
stage007의 ao_att_battle_8_03은2600ms이며 현재 저작에서는15000ms에 시작한다.
충격 Effect16962ms와 사운드16966ms가 같은 타격을 가리킨다. 노란 MAP 원은
[4.340000152587891,10,323.1000061035156]에 고정돼15577~17643ms 재생된다.
따라서 실제 타격이 끝나도 노란 예고가 남고 collider는 무기 본에 종속돼 있다.

## G01. 변경 전체 필드

P26.presentation.7의 anchorKind=MAP, followBoss=false, bone="", boneTarget=BODY,
positionOffset=[4.340000152587891,10,323.1000061035156]으로 바꾼다.
기존 cylinder resource, scale[1.399999976158142,1,1], radius4.899999916553497m,
half-height1.25m, Logic507→522,16997ms 시작·100ms 판정 창은 보존한다.
P26.presentation.2 노란 예고 durationMs만2066→1420으로 바꿔 타격 시작16997ms에 끝낸다.
Effect 자체·다른 occurrence·P86·관문 rendering option·FXAA는 보존한다. source revision은1증가한다.
새 C++/schema/프로젝트 항목 없이 기존 MAP collider→WORLD region→Server ENTER_AREA 결과 경로다.

## G02. 반영과 검증

최신 디스크의 stable occurrence 필드만 변경하고 writer lock·freshness·백업·ReplaceFileW 원자 교체를
사용한다. 실제 projector로 MAP/WORLD region과 기존 피해·넉백을 확인하며 소스 나머지 구조의 동일성과
수치 시각을 검사한다. 후보가 준비되면 통합 담당과 게시 충돌이 없는지 조율한 뒤 Kouku projector를
게시한다. GameplayPublish와 제품 빌드는 통합 담당이 수행한다. Client/UI는 실행하지 않는다.
