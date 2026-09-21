# Exposure Multiplier 연속 슬라이더 결과

## G01. 완료

MainApp Rendering Workbench의 Live rendering comparison에 있던 0.5x/1x/2x radio button을 0.5~2.0 연속 SliderFloat로 교체했다. 소수점 세 자리 배율을 표시하고 Ctrl+click 직접 입력과 AlwaysClamp를 지원한다. 변경값은 기존 Set_ComparisonOptions→다음 Update의 Apply_CameraEnvironment 경로로 현재 Character Select를 포함한 활성 화면에 적용된다. 기존 baseline 복원 후 한 번만 곱하는 방식이므로 프레임마다 배율이 누적되지 않는다.

비교값은 기존과 같이 저장하지 않는다. Reset, Workbench 닫기와 Level 변경은 기존 저작값으로 복귀한다. Scene profile, authoring/runtime JSON과 공통 적용 서비스는 변경하지 않았다. 새 C++ 파일·프로젝트 등록·리소스 배포가 없다.

## G02. 검증과 실행 경계

실제 MainApp 전체 translation unit을 현재 Debug 제품 compiler switches로 컴파일해 exit0을 확인했다. 출력은 out/ExposureMultiplierSlider20260921/MainApp.obj 및 compile.pdb로 격리했다. 기존 C4819 경고는 남아 있다. MainApp의 원래 UTF-8 no BOM/CRLF 및 다른 dirty 변경을 보존했다. 변경 diff 검사 통과.

Client와 Server가 실행 중이고 사용자가 직접 빌드/화면 검증을 수행하는 상태여서 제품 EXE/DLL 링크·교체나 프로세스 종료는 수행하지 않았다. 이번 슬라이더는 다음 Client 빌드와 재실행부터 나타난다. Client/UI 자동 실행 및 GPU 화면 확인은 하지 않았다.

근거: out/ExposureMultiplierSlider20260921/compile.log, compile.rsp, scope.diff. 기존 비교 소비·저장 경로는 09-20_KOUKU_RENDERING_OBJECT_REPAIR_RESULT의 G09와 대조했다.
