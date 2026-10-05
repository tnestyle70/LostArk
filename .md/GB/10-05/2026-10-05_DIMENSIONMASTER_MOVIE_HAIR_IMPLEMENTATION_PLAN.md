# 차원술사 무비 헤어 표시 구현 계획

## G00. 정상 머리와 무비 머리의 실측

사용자는 선택창·체험하기에서 정상인 머리를 무비에도 사용하도록 요청했다. 두 모델은 같은 hair55이며 16,956정점·60,927인덱스다. basis 변환 뒤 위치·UV0와 diffuse/normal RGBA가 동일하다. 무비의 골격과 Intro/Loop를 버리는 파일 경로 교체 대신 같은 머리를 정상 장착 경로와 동일하게 그린다.

정상 CPart_Equipment는 불투명 core의 양면 masked base/pass6 뒤 soft forward/pass9를 사용한다. CWorldSequenceObject는 NONBLEND에서 머리를 건너뛰고 pass9만 쓴다. 추가로 native program7의 source[26].x 강제0이 caller의 masked 선택을 지워 원래 alpha discard를 끈다. 이 두 소비자 결함을 함께 수정한다. 별도 shell 제외는 이전 dirty 변경이며 유지한다.

## G01. 파일과 호출 흐름

- Client/Public/DeferredMaterialRenderUtils.h와 대응 CPP: 기존 장착 헤어의 program별 masked 선택·상수 binding·pass6 draw를 공용 함수로 옮긴다. 준비된 world/bone 입력을 소비하고 지원하지 않는 재질은 S_FALSE로 돌려준다. 성공·실패 뒤 masked flag를 복원하고 CMaterial 상수를 수정하지 않는다.
- Client/Private/Part_Equipment.cpp: 기존 masked 분기를 공용 함수로 교체해 정상 장착과 무비가 같은 계약을 사용한다.
- Client/Private/WorldSequenceObject.cpp: 기존 NONBLEND의 animated forward 재질 분기에서 공용 masked draw를 시도한다. 지원하는 헤어만 core 깊이를 기록하고 기존 forward edge/다른 재질은 유지한다.
- Engine/Client Shader_SourceCharacterBaseGroup001.hlsli: program7의 source[26].x 강제0만 제거하여 draw-owned masked 입력을 소비한다. 관련 생성 경로가 있으면 함께 수정한다.

새 C++ 파일·project/filter 등록·schema·Data·Resources 교체는 없다. 정상 donor의 골격/재질/이미지와 무비 원본 animation, 기존 제외 목록 및 렌더링 옵션을 보존한다.

## G02. 검증과 설치

실제 설치 hair의 geometry/texture 비교, shader native alpha0/중간/불투명 coverage와 depth readback, 공용 draw의 지원 범위·실패 reset을 확인한다. 기존 probe를 재사용하거나 out의 한정 fixture로 실제 함수를 실행하고 새 제품 하네스는 만들지 않는다. 변경 C++/shader는 정상 증분 Product Build로 배포한다. 실행 중 Client가 출력물을 점유하면 그 구성의 링크는 종료가 필요한 사실을 보고하고 가능한 독립 검증과 비점유 구성을 먼저 완료한다. 사용자 프로세스는 자동 종료하지 않는다.

인코딩 보존과 git diff --check를 확인하고 실제 수행한 검증만 RESULT에 기록한다. 최종 무비의 정수리·뒤통수 화면 판정과 실행 중 Client 재시작은 사용자가 직접 한다.
