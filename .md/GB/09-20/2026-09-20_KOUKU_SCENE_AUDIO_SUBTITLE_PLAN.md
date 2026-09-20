# 쿠크 연출 사운드·자막 원본 연결

## G1. 원본과 현재 소비자

기존 Matinee 추출본 `out/KoukuFireworks20260911`에 AkEvent와 `efinterptracksubtitle` 트랙이 있다. 설치 게임 `data2.lpk`의 `EFTable_GameMsg.db`에서 `cin.37081_*` 한국어 원문을 확인했다. 기존 쇼타임 연출 P76에는 SOUND가 없으며 카드미로 P77/Sequence P6에는 scene soundtrack 하나가 있다. 카드미로 BGM의 별도 event는 원본 Music container와 control action을 다시 확인한다.

## G2. 기존 Composition 확장

`KoukuSaydonCompositionDocument`의 presentation resource에 `SUBTITLE` kind와 `subtitleText`, `subtitlePosition=NORMAL|UPPER`를 추가한다. `assetId`는 원본 GameMsg stable ID다. 기존 occurrence의 start/duration, 편집, 저장, Reset, Seek를 재사용한다. 텍스트는 유효한 제한 길이 UTF-8이며 빈 문자열, 제어문자와 다른 kind의 자막 필드를 거부한다. 원본 태그가 있는 전체 문장은 추출 receipt에 보존하고 표시문은 줄바꿈과 문자만 사용한다.

`KoukuSaydonPresentationPlayer`는 기존 active row 수명을 사용해 자막을 보유하고 읽기 전용 `Collect_Subtitles()` view를 제공한다. MainApp은 기존 제품 `Render_Font` 경로로 표시하며 ImGui를 제품 화면에 사용하지 않는다. projector와 Product reader도 같은 계약을 검증한다. 기존 C++ 파일만 수정하므로 프로젝트 소스 등록은 추가하지 않는다.

## G3. 후보와 검증

사운드·자막 연결은 실제 source track과 현재 camera prefix로 대상 pattern을 resolve하고, 전체 문서 교체가 아닌 stable ID 추가 후보로 만든다. 원본 세계/카메라/애니메이션 행은 변경하지 않는다. 빙고 진입 유리깨짐은 원본 source reference와 timing을 조사해 추출 후보로만 준비한다. 최종 Data/Resources 설치는 통합 담당의 최신 디스크 CAS 및 기존 publisher가 수행한다.

최소 검증은 JSON/source join, codec/projector roundtrip·invalid input, 실제 C++ 소비자 컴파일, 재생 row의 종료·되감기 경계 검사다. Client 실행과 사용자 화면/청취는 수행하지 않는다.
