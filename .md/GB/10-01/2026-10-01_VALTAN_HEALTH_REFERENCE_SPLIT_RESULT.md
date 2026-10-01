# 발탄 21억 HP·160줄과 피해 기준 분리 결과

## G00. 반영된 소스

발탄 본체의 Retail 최대 HP는 2,100,000,000, 최대 줄수는 160으로 변경했다.
사용자의 마지막 요청인 21억을 적용했다. 기존 741,285,439의 정확한 세 배는
2,223,856,317이므로 최종 값은 정확한 세 배가 아니다.

optional damageReferenceHp=741285439를 별도로 보존한다. 일반 공격과
bossHealthBarDamage 공격의 절대 피해 기준, 새 콜로세움의 인간·용병 HP92660680과
피해 기준은 유지된다. 유령 발탄197222731/40줄은 변경하지 않았다.

BOSS bootstrap은 기존 11필드와 optional 마지막 참조값을 가진 12필드를 읽는다.
미지정 참조값은 기존 maximumHp를 사용하며 명시한 값은 양수 uint32여야 한다.
Server profile·entity·spawn·numeric migration·유령 전환·hot reload 비교와 기존
numeric 저장 경로에 연결했다. 네트워크 패킷·프로토콜은 변경하지 않았다.
따라서 새 Server와 게시 데이터는 같은 ZIP으로 전달한다.

## G01. 컴파일과 검사

Release Product 빌드는 PASS다. 마지막 스킬 검사 CPP 변경 뒤 증분 컴파일한
최종 receipt는 `out/BuildPipeline/runs/20261001T025551380Z-release-product.json`이다.
로그는 `out/ReleaseF1Restore20261001/product-package-final.log`에 있다.
패키징 도구 22개 검사는 PASS이며 같은 폴더의 package-tests.log에 기록했다.

쿠크 Encounter와 patternbindings는 원본 JSON 항목·값 차이가 0개였으며,
Git checkout의 CRLF와 생성기의 LF만 달랐다. 검사기를 CRLF/LF만 허용하도록
수정하고 실제 값 변경·중복 키 거절은 유지했다. 최소 회귀 3개가 PASS다.
기존 두 쿠크 파일의 bytes와 1관문 입장 컷씬 음소거는 변경하지 않았다.
canonical Gameplay Publish는 PASS이며 로그는
`out/ValtanHp2x20261001/canonical-publish.log`다. 109866행 중 의미상 변경은
발탄 본체 BOSS 한 행뿐이다. 게시 bootstrap SHA256은
`05775828d3d8a2e2b6dc9b9395d96173015c8ac02aa23285cb62ee6b7ddd5596`이다.
Producer 16개 사례와 저장 transaction 16개 검사도 PASS다.
Client/UI 실행 및 실제 플레이 화면·음향 확인은 수행하지 않았다.

## G02. Server 실행 검사

새 게시 데이터와 Release Server로 Valtan lifecycle, Colosseum match/combat가
모두 PASS했다. 실제 공격 네 경로의 피해 총량162156190, 기본 피해 보존,
21억 spawn·160줄, 유령 전환의 기존 HP·참조 초기화, 콜로세움 인간·용병
HP92660680과 numeric reload에서 기존 경기 참조 유지가 통과했다.
로그는 `out/ReleaseF1Restore20261001/valtan-lifecycle-final.log`,
`colosseum-match.log`, `colosseum-combat.log`다.

새로 연결한 기존 스킬 검사는 최초 실행 때 이벤트 수를 6회로 고정해 네 항목이
실패했다. 현재 게시 ALT_V는 반복을 포함해 15회이므로 원본 반복 횟수의 합을
기대값으로 사용했다. 정확한 HP 총량·바·포화 경계 검사는 유지한 채 재실행했다.
제품 피해 계산을 테스트에 맞춰 바꾸지 않았다.

numeric-balance 전체 검사는 기존 무력화 기대값 한 항목이 실패했다.
원본과 bootstrap은 부모커밋8eca9cbfb부터50000이며 이전6a988d7037에서 변경됐다.
변경하지 않은 RevisionProtocol 검사만40000을 기대한다. 실제 저장·적용·복원은
50000→51000→50000으로 PASS했다. 이 기존 테스트 불일치를 전체 PASS로 기록하지
않으며 이번 요청에서 제품 무력화값을 바꾸지 않았다. 로그는 numeric-balance.log다.

## G03. 최종 ZIP

- 경로: `C:/Users/user/Desktop/LostArk-Release-20261001-F1-VALTAN-21EOK.zip`
- 169161542 bytes, SHA256 `d65fe9e0df52017e98b2df62d7edb5893d4ba6d7a6893474c7ec03d2671dbda6`
- 소스 commit053978b34와 위 최종 Release receipt를 사용했다. Deploy CPU 피킹,
  Release F1, 발탄21억/160줄, 기존 PvP·유령 체력 및 피해 기준을 함께 포함한다.
- 패키저 PASS: 전체 ZIP CRC, 각 payload hash, 중복 경로, 데이터 revision,
  launcher `--check` 통과. Resources는 기존 외부 폴더를 사용하며 Client/Server를
  실행하지 않았다. stage는 `out/ReleasePackaging/20261001-f1-valtan-21eok`이고
  receipt는 `out/ReleasePackaging/portable-delivery.receipt.json`이다.
- 최종 Valtan lifecycle181개, Colosseum match319개, combat95개 PASS.
  numeric의 기존 고정 기대값 오류 한 건은 G02에 별도로 기록했다.
