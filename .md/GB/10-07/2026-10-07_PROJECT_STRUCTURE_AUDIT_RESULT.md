# 프로젝트 전체 구조 해설과 솔루션 등록 정리 결과

## 조사와 적용 범위

사용자의 전체 구조·파일·include·C++ 문법·Data·시퀀서 해설 요청과 추가 프로젝트 필요성 점검 요청에 따라 현재 작업 트리의 소스를 조사했다. 기존 미커밋 변경은 보존했다.

- 제품 C/C++ 소스와 헤더·INL 등 1,053개를 색인했다. Client 712, Engine 166, Server 147, Shared 28개이며 외부 SDK/vendor는 이 수량에서 제외했다.
- Git 추적 코드 확장자 파일 3,527개의 경로·크기·include와 제품 소스 문법 선언을 색인했다. 자동 문법 색인은 macro 전개/컴파일 구성을 재현하지 않으며 모든 함수의 의미를 수동 검증한 것은 아니다.
- 물리 파일 91,556개의 경로·크기 목록을 생성했다. Resources 바이너리는 파일별 payload를 역분석한 것이 아니다.
- 핵심 실행 경로는 실제 H/CPP와 데이터·publisher를 대조하여 별도로 해설했다. 현재 protocol133, gameplay bootstrap format38, COLOSSEUM Level, Client 이동 예측을 포함한다.

상세 해설과 검색 산출물은 [전체 코드 해설](C:/Users/tnest/.codex/visualizations/2026/10/07/01a11628-dd48-7a41-8a4d-46786a0c3ed4/LostArk-전체구조-해설.md), [파일 탐색기](C:/Users/tnest/.codex/visualizations/2026/10/07/01a11628-dd48-7a41-8a4d-46786a0c3ed4/lostark-code-atlas.html), [프로젝트별 필요성 조사](C:/Users/tnest/.codex/visualizations/2026/10/07/01a11628-dd48-7a41-8a4d-46786a0c3ed4/projects_audit.md)에 있다. 이 링크는 현재 PC의 대화 산출물이며 팀 저장소의 이식 가능한 public 계약 경로는 아니다.

## 실제 변경

`Framework.sln`에서 SolutionItems/NestedProjects가 없는 빈 DataFiles SolutionFolder 등록 2줄만 제거했다. Data 폴더, Client의 None 항목, 제품 4개와 하네스 6개의 프로젝트 등록 및 빌드 구성은 유지했다.

현재 하네스 6개는 모두 Core/FullDiagnostic runner의 직접 vcxproj/EXE/script 호출 경로에 연결되어 있다. 기본 Solution Build는 이미 제품 4개만 Build.0에 포함하므로 하네스를 제거해도 일반 제품 빌드의 컴파일 대상이 줄지 않는다. 현재 회귀 수단을 삭제할 근거가 없어 유지했다.

`Tools/Build/README.md`의 Core 프로필 설명에 실제 실행되는 ValtanPatternAuditionService와 composition/presentation 검사를 추가했다. 기존 파일 인코딩·CRLF를 유지했다. 게임 C++·Data·vcxproj·filters는 변경하지 않았다.

## 검증

- 제품 4개만 기본 빌드하는 기존 계약 검사 PASS.
- Core의 실제 Valtan canonical loader 연결 검사 PASS.
- 솔루션이 참조하는 vcxproj 10개와 대응 filters의 XML parse 및 경로 확인 PASS.
- 이번 변경 파일 `git diff --check` PASS.
- 기존 `Tools/Build/test_build_profile_contract.py` 전체 실행은 18개 중 14 PASS, 4 FAIL.

기존 전체 검사의 실패 원인은 Valtan 프로젝트의 `/MP /utf-8` 문자열 고정 assertion, 현재 Client의 Data/Effects None 항목과 과거 금지 assertion의 충돌, WARP probe 1,211줄과 1,200줄 미만 한도의 충돌, Product output guard 임시 fixture의 BuildIncrementalDiagnostics.psm1 누락이다. 마지막 항목은 최초 CP949 출력 해석 오류 뒤 UTF-8로 재확인하여 모듈 누락을 확인했다. 해당 검사 입력 소스는 HEAD에서 변경되지 않았으며 이번 빈 SolutionFolder 제거·README 수정과 무관하다. 무관한 assertion·fixture·제품 코드는 수정하지 않았다.

Client/UI와 실제 Server를 실행·조작하지 않았으며 C++/FX 재빌드는 하지 않았다. 구조 검사 내부의 process fixture만 실행했다. 이 결과는 실제 화면·게임play·모든 하네스 runtime PASS 증거가 아니다.

## HLSL 집계

추적 코드 확장자의 실제 파일 바이트 합계 156,924,941 중 HLSL/HLSLI는 78,086,130바이트로49.76%다. 물리 줄 수는1,343,936줄로43.60%이며 주석/빈 줄을 포함한다. GitHub Linguist의 generated/vendor 판정을 동일 재현한 수치는 아니다. native program별 대형 HLSLI와 동일 내용 복사분17,763,166바이트가 비중에 기여한다. 이 비율은 실행 시간·GPU 비용·개발 기여 비율이 아니다.
