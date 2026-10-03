# World Movie 자유 카메라 포즈 캡처 FOV 수정 계획

## G00. 원인과 범위

WARLORD Intro/Loop camera.2는 수평 FOV17도다. 16:9에서 수직 FOV는9.610678도이며
Movie runtime과 F6 자유 시점 인계는 이 값을 정상 소비한다. Capture_ViewPose는 기존
Camera Tool의 저작 제한10~120도를 재사용해 정상 포즈도 거부한다.
Use free cam pos는 Eye/LookAt/Up만 복사하고 선택 key의 FOV를 유지하는 소비자다.

## G01. CameraTool의 공통 캡처

Client/Private/CameraTool.cpp의 Capture_ViewPose는 runtime과 같은1<FOV<179 범위와
유효한 Eye/LookAt/Up을 검사한다. 입력 projection·시선·Up의 유한성, 영벡터와 평행 basis를
확인하고 local candidate가 모두 통과할 때만 출력 포즈를 교체한다.
기존 Is_ValidAuthoringPose의10~120도 제한은 Valtan 저작 문서용으로 보존한다.
Capture_CurrentPose도 이 검사를 outKeyframe 대입 전에 수행해 넓어진 캡처가 기존 문서에서
거부될 때 키를 부분 변경하지 않도록 한다.
위치100000 범위와 기존 시선 길이10도 유지한다. FOV를 clamp하거나 사용자 JSON을 바꾸지 않는다.
Client/Public/CameraTool.h의 공통 캡처 주석에 runtime 범위와 실패 시 출력 보존을 명시한다.
새 C++ 파일·project/filter 등록과 wire·Server 변경은 없다.

## G02. 검증과 실행 반영

실제 캡처 본문을 사용하는 작은 native probe로17도 horizontal 변환, 일반·넓은 FOV,
무효 projection/basis 거부와 출력 보존을 확인한다. 기존 저작 제한이 유지되는지도 대조한다.
변경 TU를 out 출력으로 먼저 컴파일한다. 실행 중 Client는 자동 종료하지 않는다.
최종 EXE 교체 직전에 저장·종료를 확인하고 기존 Product와 같은 옵션의 Debug Client 프로젝트
증분 Build로 반영한다. Engine/Shared/Server는 변경하지 않아 기존 산출물을 재사용한다.
사용자가 직접 동일 Movie의 자유 시점에서 키를 적용·저장하고 화면을 확인한다.
