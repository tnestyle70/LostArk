# Exposure Multiplier 연속 조정 구현 계획

## G00. 현재 소비 경로

Rendering Workbench의 Live rendering comparison은 현재 0.5/1/2 배 radio button이다. RenderingProfileService는 이미 0.5~2 사이 모든 finite float를 허용하고 매 Update의 Apply_CameraEnvironment에서 원래 품질에 한 번 곱한다. Character Select도 같은 경로다.

## G01. UI 변경

MainApp.cpp의 세 preset만 SliderFloat 0.5~2.0, 소수점 세 자리 표시로 교체한다. AlwaysClamp로 직접 입력도 범위를 지킨다. 기존 comparisonChanged와 Set_ComparisonOptions 경로로 다음 렌더 프레임에 반영한다. Ctrl+click 직접 입력 안내를 추가하고 runtime comparison의 reset/close/Level전환 해제 및 비저장 계약을 보존한다. 새 C++ 파일이나 프로젝트 등록은 없다.

## G02. 검증

MainApp 전체 TU를 별도 out object로 컴파일하고 변경 diff를 확인한다. 현재 사용자가 Client/Server를 실행하며 검증 중이므로 EXE/DLL 교체나 프로세스 종료는 하지 않는다. 신규 테스트와 데이터 게시는 이 UI 변경에 필요하지 않다. 화면 확인은 사용자 빌드 후 수행한다.
