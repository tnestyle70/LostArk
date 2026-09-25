# 쿠크 Sequence Source In 준비 실패와 이전 OBJ 복구 계획

## G00. 실제 원인과 범위

Sequence revision171에는 effectSourceStartMs 필드가 없다. 정상 parser 기본값은0이다.
18:49와19:01 두 Client의 Complete Play 준비는 동일 Action2254/Sequence171과 gameplay
pin으로 들어왔지만 첫 Sequence의 Source In 수명 검사에서 실패했다. 현재 document OBJ를
연결한 native 실행은 실제10개 Sequence를 모두 Load·Expand한다.

Debug Animation_Tool.obj는07:29의 이전 occurrence 복사 생성자·대입 연산자를 남겼다.
그 TU의 CL.read 기록에는 소스와 Client.pch만 있고 변경한 composition header가 없다.
새 Source In 멤버는 기존 alignment padding인0x144에 들어갔다. 이전 복사자는0x140 다음
0x148을 복사하므로 새 필드가 미초기화로 남는다. 구조체 크기가 같아 링크가 성공해도
서로 다른 inline 복사자가 같은 이름으로 선택될 수 있다.

## G01. 복구 대상과 순서

정확한 이전 Animation_Tool.obj와 read/write/command tracking을 저장소 밖이 아닌 Git 제외
out/KoukuSequenceLifetime20260924/previous-debug-objects에 보존한다. 해당 OBJ만 이 격리
디렉터리로 옮긴 뒤 기존 VS18/v143 정상 Debug ClCompile 또는 Product Build를 수행한다.
header·소스 timestamp, PCH, 전체 tracking을 지우거나 전체 Clean/Rebuild하지 않는다.

Source In validation과 Sequence/Action JSON, 클라이언트 준비 acknowledgement는 유지한다.
이 건에는 C++/H/JSON 변경이나 새 프로젝트·filters 등록이 없다. 링크는 root 통합 작업이
실행 중 파일 점유와 다른 변경을 함께 판단한다. Client·Server 실행과 종료는 하지 않는다.

## G02. 검증

재컴파일한 TU가 composition header를 CL.read로 추적하는지 확인한다. COFF 복사 생성자가
다른 최신 TU와 같은 필드를 복사하는지 직접 대조한다. 실제 저장된10개 Sequence의 native
Reload·Expand와 source-in 기본값 보존을 확인하고 source hash 불변을 기록한다.

root 통합 Debug/Release Build와 사용자 Complete Play 화면 확인은 각각 독립된 결과다.
빌드·native 검사 성공을 사용자 화면이나 서버 연동의 최종 통과로 기록하지 않는다.
