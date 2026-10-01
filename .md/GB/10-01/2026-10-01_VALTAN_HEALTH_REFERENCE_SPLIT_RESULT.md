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

Release Product 빌드는 PASS다. 마지막 소스 변경 뒤 세 개의 Server 검사 CPP를
증분 컴파일한 최종 receipt는
`out/BuildPipeline/runs/20261001T024301377Z-release-product.json`이다.
로그는 `out/ReleaseF1Restore20261001/product-final.log`에 있다.
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
Server 계약 검사와 ZIP 결과는 완료 후 아래에 추가한다.
Client/UI 실행 및 실제 플레이 화면·음향 확인은 수행하지 않았다.
