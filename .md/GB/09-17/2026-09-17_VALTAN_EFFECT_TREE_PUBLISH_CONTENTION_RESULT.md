# 발탄 이펙트 트리와 게시 잠금 경합 수정 결과

## G00. 원인과 현재 정본 상태

사용자가 보고한 문구는 `Effect_Tool_Valtan.cpp::Refresh_ValtanPatternTree`에서 canonical
shared read admission 획득이 거절됐을 때 나온다. Product/receipt 내용 검증보다 앞의 분기다.
해당 시각 Kouku owner의 Gameplay publisher가 실행 중이었다. publisher 종료 뒤 동일한
CreateFileW/LockFileEx 공유 byte 잠금은 즉시 성공했고 active-generation journal은 없었다.
기존 writer와 shared reader의 실제 충돌은 Win32 ERROR_LOCK_VIOLATION33으로 재현했다.
사용자가 전달한 문구에는 뒤쪽 상세 진단이 없으므로 그때의 Win32 오류까지 직접 관측했다고
기록하지 않는다. 현 증거는 일시적 publisher 경합 경로와 일치한다.

Effect Catalog의 `effect.valtan.*`211개 ID는 중복이 없고 저작 파일 경로도211개 모두 존재한다.
이는 메타데이터 확인이며 모든 문서의 재질·GPU 표시·Product 연결 검증을 대신하지 않는다.
기존 화면 코드는 resource 목록과 exact authored 문서를 pattern admission 분기 전에 렌더하고,
선택한 문서만 Open 시 decode한다. 독립 Effect 목록까지 Product Play 권한으로 차단하지 않는다.

## G01. 변경

- `Publish-GameplayBalance.ps1`: 독립 Kouku 선검증을 발탄 writer 획득 앞으로 옮겼다.
  발탄 snapshot·generation·bootstrap 교체와 실패 정리는 기존 writer 안에 유지했다.
  외부 caller가 이미 소유한 writer의 수명을 줄이거나 검증 자체를 생략하지 않는다.
- `Effect_Tool_Valtan.cpp`: 먼저 비차단 shared admission을 시도한다. writer busy 재시도 중에는
  catalog와 FullRestore animation index를 다시 읽지 않고 이전 인덱스를 보존한다.
  WRITER_BUSY는 게시 대기/자동 재시도로 표시한다. exact-save revision 요청은 해당 revision
  pin을 유지하며 임의 최신 세대로 자동 전환하지 않는다. 실제 손상 진단과 원자성은 보존한다.

공통 잠금을 지우거나 검증 없는 Product fallback을 추가하지 않았다. Client·Server 종료,
UI 실행·조작·캡처 또는 발탄 정본 데이터의 덮어쓰기는 하지 않았다.

## G02. 검증과 한계

변경 `Effect_Tool_Valtan.cpp`는 VS18 Insiders14.44의 x64 격리 컴파일에 통과했다. 기존 CP949
Engine 헤더를 UTF-8 모드로 읽는 C4828 warning은 보존했으며 compile error는 없다.
실제 임시 writer/shared admission 검사2개, publisher header/admission 검사4개 및 action 도중
입력 변경 시 receipt 거절 검사1개가 통과했다. publisher의 나머지 bytes가 그대로이며 검증
인자·실패 guard가 동일함을 비교했고 PowerShell 전체 AST parse도 통과했다.

초기 관련52개 source 계약 검사는47통과/5실패였다. 실패는 이번에 수정하지 않은 Save overload
선택, MainApp case 범위, Workbench 함수 이름, Update dirty guard 문자열 기대의 불일치다.
관련 없는 검사나 제품 코드를 통과 목적으로 변경하지 않았으며 전체 PASS로 기록하지 않는다.
최종 세부 검사 결과와 로그는 `out/ValtanTreeRead20260917/`에 보존한다.

publisher 잠금 변경 후 전체 게시를 다시 실행하거나 시간 개선을 실측하지는 않았다.
Kouku 선검증의 중복 작업 자체는 남아 있고 직접 획득한 발탄 lock 밖으로 이동한 것이다.
제품 전체 링크·Client 화면 확인은 사용자 빌드와 Refresh/재생 확인으로 남는다.

근거: `out/ValtanTreeRead20260917/admission-after-publisher.json`,
`admission-lock-test.log`, `catalog-metadata.json`, `Effect_Tool_Valtan.log`,
`focused-contract-tests.log`, `out/CardDiceScale20260917/publisher-lock/verification.json`.


최종 focused 확인: 직접 관련 기존 계약7개가 모두 통과했다. busy 이전 metadata/clip I/O
미수행, 기존 cache 보존, typed 진단, exact-save unpinned 재시도 차단, independent authored
목록 선행 표시의8항목 독립 검토에도 blocker가 없었다. `review-and-compile.json`과
`targeted-contract-tests.log`를 최종 근거에 추가한다. 전체 제품 빌드 성공으로 확대하지 않는다.
