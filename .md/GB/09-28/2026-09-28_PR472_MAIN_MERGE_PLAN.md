# G01 — PR472 양쪽 기능 보존 병합

PR472의 82455a53과 fetch한 main의 18개 충돌을 기능별로 결합한다.
main의 작은 public header/단일 CPP/generated INL 및 leaf dispatch 최적화를 유지하면서
마하라카의 여섯 신규 family packing·Base/Light 함수·named-vector binding을 이 구조에 추가한다.

main의 Guardian 의상1526, 모코모코1=1528, 모코모코2=1527은 유지한다.
PR 받침01=1528만 빈1532로 옮긴다. 받침02/03/04=1529/1530/1531과 주민1474/1475는 유지한다.
재질은 family로 연결되므로 기존 저작/게시 JSON·Resources 외형·배치·동작은 변경하지 않는다.
CPU/HLSL static 판정·입력 packing·registry·probe 기대값을 같은 번호로 변경한다.
main 함수는 그대로, PR 추가 함수는 필요한 식별번호 외 본문 그대로 보존했는지 대조한다.

프로젝트/filter와 문서 충돌은 양쪽 독립 추가 블록을 함께 보존한다. 자동 병합된 나머지 변경도
두 부모의 데이터·프로젝트 항목과 함수 집합을 대조한다. main 렌더링 튜닝·전투·가디언·Guide는 유지한다.
기존 단일 소유 C++ 구현을 되돌리거나 program 숫자를 전역 치환하지 않는다.

검증: JSON/XML, 부모별 함수·dispatch·mirror, 마하라카 publisher, 정상 Debug Product Build와
관련 headless probe. Client 화면은 사용자 확인이다. 검증 후 merge commit으로 같은 브랜치에
push하고 PR472 충돌 해소를 확인한다. Resources/개인 user 설정은 커밋하지 않는다.
