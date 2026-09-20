# Debug 로딩 CPU 처리 최적화

## G00. 현재 기준과 완료 조건

사용자는 Debug 로딩 최적화를 지금 적용하고, 병목이 불명확한 부분은 계측 후 바로 개선하도록 요청했다. 기준은 `pattern-bug-fix-bingo`, `7d5e27ea4`다. 실행 데이터 전달·빌드 작업의 다른 미커밋 변경은 보존한다. 이 작업은 데이터 저장·게시나 필수 리소스 준비 범위를 바꾸지 않는다.

기존 근거는 `../09-16/2026-09-16_COLD_MAP_LOADING_IMPLEMENTATION_RESULT.md`와 `../09-20/2026-09-20_KOUKU_PUBLISH_AND_EFFECT_LOADING_RESULT.md`다. Debug DataJson은 이미 /O2지만 같은 119개 JSON 처리에서 1 worker 32.695초, 3 workers 87.466초였다. CPU 문서 처리 표본이며 전체 맵 진입 시간은 아니다.

## G01. JSON 값의 활성 payload

`Client/Public/DataJson.h`, `Client/Private/DataJson.cpp`에서 scalar마다 생성하던 string/vector/map/order 컨테이너를 실제 JSON 종류의 활성 payload만 생성하는 저장 구조로 변경한다. 공개 ARRAY/OBJECT alias, type/number token, 잘못된 타입 getter의 빈 값, object 입력 순서와 Find, deep copy/move를 유지한다. parser의 문법·깊이·크기·값 개수 검사, 중복 key 거절, 실패 시 이전 출력 보존을 유지한다.

원본 H/CPP와 hash를 out 아래에 보존하고 동일 MSVC /O2 /MDd로 baseline/candidate를 각각 컴파일한다. 실제 설치 effect JSON의 전체 semantic digest, 오류·copy/move·getter 계약과 전후 할당량/시간을 비교한다. 후보가 느리거나 결과가 다르면 원인을 수정하고 개선이 확인된 변경만 남긴다.

## G02. Effect CPU 단계와 선택 최적화

기존 Effect_DocumentCodec의 실제 parse/decode/validate를 분리한 비UI probe를 재사용한다. 같은 DataJson 기준으로 /Od와 /O2의 결과와 시간을 비교해 개선이 확인된 decoder TU만 기존 Debug hot-TU 정책으로 컴파일한다. /MDd, _DEBUG, PDB와 수치 의미는 유지하며 CRT/STL ABI 설정을 파일별로 혼합하지 않는다. Client 프로젝트의 다른 dirty metadata를 보존한다.

`Effect_PresentationService.cpp`의 기존 preparation과 진단 경계에 문서 준비·renderer 준비·commit 단계의 시간을 필요한 범위로 추가한다. 기존 profiler를 사용하고 느린 target 요약은 기존 진단 로그에 남긴다. FIFO·revision·rollback·필수 준비 완료 장벽은 유지한다. Debug worker 수는 새 JSON 후보의 실제 비교 후 결정하며 오래된 수치만으로 전역 직렬화를 강제하지 않는다.

## G03. 검증과 제품 반영

새 제품 C++ 파일은 계획하지 않으므로 project/filter 신규 등록은 없다. 계약 검사는 out 아래 격리된 콘솔로 수행하고 성능 비교와 무거운 컴파일을 겹치지 않는다. 현재 실행 중 Client/Server는 유지한 채 소스·격리 검증을 먼저 완료한다. 최종 Product 링크에 실제 EXE/DLL 점유 해제가 필요할 때만 저장·종료를 요청한다.

변경 XML parse, 관련 Debug/Release 컴파일, `git diff --check`와 실제 데이터 동등성·전후 성능을 확인한다. 표준 Debug Product Build로 설치한 결과와 실행 파일 시각을 RESULT에 기록한다. Client/UI 실행·화면 판정은 사용자가 직접 하며 비UI 표본의 개선율을 전체 맵 입장 개선율로 확대하지 않는다.
