# Debug 로딩 CPU 처리 최적화 결과

## G00. 구현 범위

`DATA_JSON_VALUE`는 모든 노드에 string/vector/map/order를 생성하던 구조에서 활성 종류의 payload만 생성하는 variant로 바뀌었다. 공개 JSON 값 API, ARRAY/OBJECT 별칭, number token, object 입력 순서와 deep copy 의미는 유지한다. Debug MSVC의 값 크기는 152에서 88 bytes로 줄었다. 파일 캐시나 필수 검증을 생략하지 않았으며 runtime 데이터·Resources는 변경하지 않았다.

`Effect_PresentationService.cpp`는 기존 profiler에 `Effect.Prepare.Document`, `Metadata`, `Renderer`, `Commit`을 추가했다. 50ms 이상 걸린 단계는 기존 `EffectFailure.user.log`에 `V1.prepare.document/metadata/renderer/commit`으로 기록한다. Document는 읽기·파싱·디코딩·projection의 합이고 Renderer는 CPU의 device resource 준비 시간이다. GPU 실행 시간이나 전체 맵 진입 시간으로 해석하지 않는다. 기존 전체 `V1.prepare.slow`와 병렬 구간 시간을 합산하지 않는다.

`ProfilerTool.cpp`의 CPU scope catalog에도 네 이름을 등록해 아직 관측되지 않은 단계도 `Show unobserved sections`에서 확인할 수 있다. 해당 TU의 Debug/Release 최소 컴파일과 diff 검사를 통과했고 로그는 `out/ProfilerScopeCatalog20260921`에 있다. 사용자 측정 경로는 F1의 Profiler에서 Capture를 켜고 맵에 입장한 뒤 CPU sections의 `Effect.Prepare`를 확인하고 Save JSON으로 보존하는 흐름이다.

## G01. 실제 입력 비교

`out/DebugLoading20260921/datajson-inputs`에 현재 설치 입력 119개, 221,707,126 bytes를 복사했다. 각 파일의 읽기 전 원본·복사본·읽기 후 원본 hash가 같음을 확인해 동일 입력으로 비교했다. 이전 09-20 입력의 220.8MB와 같은 snapshot으로 설명하지 않는다.

양쪽 실제 DataJson을 같은 MSVC 14.44의 /O2 /MDd로 컴파일했다. 문서를 먼저 메모리에 읽은 뒤 parse·semantic digest·해제를 측정했으며 파일 읽기, effect codec, renderer/GPU 및 전체 맵 진입을 포함하지 않는다. 무거운 별도 compile과 측정은 겹치지 않았다. 사용자 Client/Server는 실행 중이므로 완전한 무부하 실험은 아니다.

| 작업자 수 | 기존 | 활성 payload | 감소 |
|---|---:|---:|---:|
| 1 | 23.726초 | 10.187초 | 약57% |
| 3 | 76.490초 | 31.436초 | 약59% |

119개 전체의 값 종류·숫자 bit/token·문자열·배열·객체 key/value·입력 순서를 포함한 semantic digest는 `b2d9f31bb517b475`로 일치하고 실패는 0이었다. 1worker 프로세스 peak working set은 약952MB에서497MB로, 3worker는1,697,894,400에서846,946,304bytes로 줄었다. 이는 프로세스 전체 최대값이며 제품 Client 메모리 감소량으로 환산하지 않는다.

후보에서도 3worker 문서 처리만은 1worker보다 느렸다. 전체 renderer 준비를 포함하지 않은 결과로 전역 worker 수를 임의로 늘리거나 줄이지 않았다. 후속 실제 단계 로그로 문서 처리와 renderer 병목을 분리한다.

## G02. 검증과 제품 반영

현재 source의 JSON 기본값·잘못된 타입 getter·object 입력 순서·Find·deep copy/move·숫자 token·Unicode·배열 확장·duplicate key·문법/크기/깊이/값 개수 실패에서 이전 출력 보존 검사를 통과했다. 계측을 추가한 실제 Effect_PresentationService TU는 정본 MSVC14.44/SDK10.0.26100.0의 기존 제품 compiler 옵션으로 out에 격리해 Debug/Release 컴파일을 통과했다.

공개 JSON 클래스의 메모리 배치가 바뀌므로 DataJson OBJ만 기존 EXE에 교체해서는 안 된다. 정상 증분 Product Build로 모든 header 의존 Client TU를 다시 컴파일해야 한다. Product 링크·설치와 새 실행 파일의 맵 진입 시간은 아직 확인 전이다. Client/UI 실행·조작·화면 판정은 수행하지 않았다.

역순 1worker 확인에서도 후보9.908초, 기존23.205초로 같은 경향을 확인했다. 별도 Debug CRT allocation hook으로 parse 기간만 계측한 할당 횟수는142,947,759에서55,276,434회로61.3% 감소했고, 누적 요청 bytes는8,948,479,599에서2,992,707,241로66.6% 감소했다. 이 hook 실행의 시간은 성능 표에서 제외했다.

Debug/Release의 기존/후보를 각각 컴파일·링크하고 계약을 확인했다. Release도119개 전체의 파일별 semantic digest가 동일했다. Release 검사는 별도 컴파일과 겹쳤으므로 그 시간을 성능 수치로 사용하지 않는다. `datajson-results.json`, `datajson-release-verification.json`, `datajson-source-hashes.json`에 명령·결과·소스 기준을 보존했다.

현재 JSON parser를 실제 `UserSettingsDocument.cpp`와 함께 다시 컴파일해 기존 `Tools/UserSettingsContractHarness`의64개 검사를 통과했다. 저장·로드·문법/schema 거부, 외부 동시 저장과 실제 Win32 파일 잠금 실패의 기존 설정·파일 보존을 포함한다. 실제 사용자 설정 대신 out fixture만 사용했으며 로그는 `out/DebugLoading20260921/user-settings/results.txt`다.

## G03. Effect codec의 Debug 선택 최적화

기존 DataJson과 지원 코드를 고정하고 실제 codec 13개 TU를 /Od와 /O2로 각각 컴파일·링크했다. 양쪽 /MDd, _DEBUG와 수치 옵션은 같고, 실제 입력 세 개를 1 worker ABBA 순서로 실행했다. 26.11MB scene04a matinee2의 decode 평균은 433.88ms에서 305.30ms로 29.63%, 1.35MB paper.dove.group은 35.37ms에서 23.08ms로 34.75% 감소했다. 7KB counter.ring은 짧은 측정 편차 때문에 개선율 근거로 사용하지 않는다. parse와 dump, 파일 읽기를 포함한 전체 시간은 비교하지 않는다.

세 문서의 canonical serialization과 정렬된 resource 목록 SHA256는 네 실행 모두 같았다. 실제 resource 검증을 통과했고 작은 문서의 잘못된 입력 세 종류는 이전 출력도 보존했다. 별도 엔진/UI mock으로 성공을 만든 검사가 아니며 probe는 CRT/bcrypt/Kernel32만 링크한다. 로그·입력 hash·명령·실측은 `out/DebugLoading20260921/codec`에 보존했다. 첫 실행의 누락 ResourceRoot는 기존 공식 `LOSTARK_RESOURCE_ROOT`를 설정한 뒤 재검증했고 성공 표본에서 제외했다.

새 DataJson h/cpp와 codec 13개 TU 및 지원 코드도 `codec/current-candidate`에서 모두 새 OBJ로 컴파일·링크했다. 실제 세 문서의 parse/drawable 검사와 기존 baseline canonical/resource SHA 일치, 잘못된 입력 세 조건의 이전 출력 보존을 통과했다. 검사 뒤 현재 source와 snapshot 15개 hash가 모두 같은 것도 확인했다. 이 통합 확인의 한 번 실행 시간은 성능 개선율에 포함하지 않았다. 근거는 `current-candidate/verification-summary.json`과 `build_o2.log`다.

`Client.vcxproj`의 `Effect_DocumentCodec.cpp`와 `_Artist31470`, `_AuthoredElements`, `_AuthoringOverrides`, `_DetailIo`, `_JsonPrimitives`, `_MaterialIo`, `_MaterialValidation`, `_PortableRuntime`, `_RuntimeValidation`, `_SourceRecipeIo`, `_SourceValidation`, `_Validation` 13개 기존 항목에 Debug|x64 전용 /O2·/Zi·RTC 해제·JMC 해제·PCH 미사용을 적용했다. /MDd, _DEBUG와 STL ABI, Release 설정은 유지한다. 이 파일들은 지역 변수가 최적화될 수 있어 줄 단위 디버깅은 기존 /Od 파일보다 제한된다. 기존 프로젝트의 다른 미커밋 변경을 보존했고 XML parse와 diff 검사를 통과했다.

## G04. F1 연계와 현재 완료 경계

전 맵 자유 카메라 속도와 Character Size의 즉시 저장·적용은 `../09-20/2026-09-20_CHARACTER_SIZE_AND_ALTV_RESULT.md` G03에 기록했다. 변경 두 TU의 Debug/Release 컴파일 검사와 현재 DataJson을 사용하는 실제 profile 저장·재로드 152개 검사를 통과했다. camera owner·F6/컷신 상태·저장 실패와 profiler worker 수명 경계도 독립 코드 검토를 마쳤다.

현재 소스 및 격리 검증은 완료했다. 사용자가 실행 중 Client/Server를 저장 후 종료하고 Release 빌드를 시작했다. 제품 프로세스 종료와 Release shader compiler 실행을 확인했으며 공용 출력 경합을 피하기 위해 이 빌드가 끝난 뒤 표준 Debug Product 빌드를 진행한다. 새 실행 파일의 맵 진입 시간과 실제 화면 판정은 아직 수행하지 않았다. 실행 중 프로세스를 자동 종료하거나 Client/UI를 실행하지 않았다.
