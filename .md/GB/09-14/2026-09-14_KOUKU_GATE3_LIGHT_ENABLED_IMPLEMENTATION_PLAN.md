# 쿠크 3관문 조명 Enabled와 추가 광원 반영

## G00. 현재 실측과 원인

`Level_KakulSaydonArena.cpp::Prepare_GateMapLights`는 3관문에서 `.6` 광원만
강제로 켜고 다른 모든 광원을 끈다. 저작 문서 변경 비교에는 enabled·색상·개수가
이미 포함되므로 캐시 갱신 후에도 이 강제 상태가 다시 적용된다.
현재 청색 `.6`과 백색 `.7`의 저장 enabled는 모두 true다.

Engine은 매 프레임 transient 목록과 조명 RT를 비우고 현재 광원을 다시 업로드한다.
Spot 합성은 가산이며 이전 청색 조도를 저장해 백색을 가리는 경로가 아니다.
기존 관문 구현은 09-13 DOLL_VARIANTS_OBJECT_SEQUENCE_EFFECT PLAN G08과 RESULT를 따른다.

## G01. 관문 활성화 문서

수정 파일은 `Client/Private/Level_KakulSaydonArena.cpp`다. `Prepare_GateMapLights`의
3관문 분기를 단일 허용 ID에서 기존 배경 조명의 명시적 제외로 바꾼다.
`source_baked_character`, `source_direct`와 기존 1·2관문 저작 광원 `.1`~`.5`는
기존처럼 제외하고, `.6` 및 새로 생성한 광원은 원본 enabled·색상·밝기·위치를 보존한다.
3관문은 특정 청색 광원의 존재를 필수로 하지 않아 삭제·재생성도 허용한다.
1관문 popup 광원의 기존 강제 활성화와 실패 처리는 유지한다.

호출 흐름은 저작 Update → preview 문서 교체 → Same_MapLightSource 변경 감지 →
Prepare_GateMapLights 검증 → 성공 시 관문 문서 교체 → MapLightPresentationRuntime 제출이다.
실패 시 기존 관문 문서를 유지한다. 새 public API, C++ 파일, 프로젝트·필터 등록은 없다.
사용자 조명 JSON과 Scene Profile은 바꾸지 않는다.

## G02. 검증과 실행 준비

변경 전·후 실제 함수로 청색 off/on, 백색 on/off, 새 광원, 청색 삭제,
기존 배경 제외와 1관문 상태를 비교한다. 최소 Debug C++ 컴파일과 git diff --check를 실행한다.
현재 Client/Server가 Debug 제품 출력을 사용 중이므로 종료를 강제하지 않는다.
검증 완료 뒤 사용자가 편집을 저장하고 Client/Server를 종료하면 정상 Product Build로 링크·배포한다.
추가 실측에서 저작 광원은 122개지만 published 문서는 121개이며 백색 `.7`이 없다.
사용자 편집 종료 후 최신 저장본으로 Map publisher를 실행해 새 광원도 runtime에 반영한다.
Client/UI 실행·화면 캡처·최종 시각 판정은 사용자만 수행한다.
