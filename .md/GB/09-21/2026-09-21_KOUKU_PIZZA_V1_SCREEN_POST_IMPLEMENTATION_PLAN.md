# 쿠크 피자 V1 ScreenPost 구현 계획

## 목표와 현재 근거

V2 `boss.kouku.pizza.e`의 ZoomBlur(3→0)와 ChromaticAberration(5→0)를 기존 V1 Effect의 ScreenPost 경로에서 각각 0.5초 동안 재생한다. 현재 V1은 ZoomBlur 프로필은 있으나 색수차 프로필과 수동 강도 보간 필드가 없다. Engine의 `CHROMATIC_ABERRATION_RECONSTRUCTED` 제출 경로는 이미 V2에서 사용한다. 기존 피자 14개 요소는 유지하고 신규 2개 요소만 추가하는 데이터 후보는 별도로 준비한다.

## G00: 계약과 변경 범위

`Client/Public/Effect_AuthoringDocument.h`의 ScreenPost enum에 새 값을 END 직전에 추가하여 기존 번호를 유지한다. `screen.chromatic-aberration.reconstructed.v1` 토큰을 추가하고 `intensityLerp=false`, `intensityEnd=0`을 선택형 필드로 정의한다. intensity는 시작 강도를 뜻한다. 종료 강도도 유한한 비음수여야 한다. 기존 문서가 필드를 생략하면 동작은 변하지 않는다.

## G01: 저장과 런타임 연결

`Effect_DocumentCodec_Internal.h`, `Effect_DocumentCodec_SourceRecipeIo.cpp`, `Effect_DocumentCodec_Validation.cpp`에서 토큰, 선택형 parse/write, 수치 검증을 연결한다. 보간이 꺼져 있고 종료값이 0인 기존 문서는 새 필드를 직렬화하지 않는다. `Effect_Playback.cpp`는 기존 정규화 lifetime으로 시작과 종료 강도를 선형 보간하고 기존 source dynamic/alpha curve 처리는 같은 순서로 유지한다. `Effect_Object.cpp`는 기존 Engine 색수차 프로필에 평가 결과를 제출한다. 새로운 렌더러나 별도 이펙트 런타임을 만들지 않는다.

## G02: 저작 UI

`Effect_Tool_Helpers.cpp`에 프로필 이름을 추가한다. `Effect_Tool_MaterialDetail.cpp`의 기존 ScreenPost 상세 UI에 보간 선택과 종료 강도를 추가하여 Apply/Save 경로에서 저장한다. 새로운 소스 파일이나 프로젝트 등록은 필요하지 않다. 각 파일의 기존 인코딩과 줄바꿈, 다른 작업의 변경은 보존한다.

## G03: 검증과 완료 경계

out 전용 native probe에서 실제 codec과 playback을 현재 헤더로 컴파일한다. 후보 parse/drawable/roundtrip, V2 원래 강도 함수와 시간별 수치 동등성, 0.5초 lifetime 종료, 기존 14개 요소 보존, 보간 필드 없는 기존 동작과 source curve 동작 보존, 잘못된 종료값과 필드형 거부를 확인한다. Effect_Object와 변경 UI TU를 최소 컴파일한다. 변경 파일 diff/check를 확인한다.

이 작업에서는 제품 전체 빌드/링크, Client 실행/UI 조작, 실데이터 교체를 수행하지 않는다. 제품 설치와 사용자의 최종 화면 확인은 부모 작업이 조정한다. 데이터 후보는 최신 저장본의 stable ID와 변경 필드 기준으로 병합하며 실행 중 도구의 메모리 갱신과 파일 반영을 구분한다.
