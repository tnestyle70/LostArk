# 쿠크 Complete Play 준비 버전 실패 표시 구현 계획

## G00. 실제 실패와 수정 범위

PID 33120의 08:33:32 준비 실패는 resources.final_revision에서 저장 Action 2238과 Server pin 2237이 달랐기 때문이다. BossTool Reload는 미지원 6패턴과 2묶음을 목록에 남긴 뒤 성공했지만, 성공 안내 문자열이 후속 revision 불일치의 실패 이유로 사용됐다. 08:34:54 다음 START는 공식 publisher의 08:30:58~08:35:37 transaction 도중이었고 당시 Server bootstrap은 2237이었다. 현재 로그에는 Server 내부 분기가 없어 lock 실패와 candidate revision 불일치를 확정해서 구분할 수 없다.

## G01. Client 준비 경로

Client/Private/MainApp.cpp의 UpdateKoukuGateCompletePlay에서 파일 Reload 실패와 pin 비교 실패를 분리한다. Action, Sequence, published Action의 불일치는 expected/current revision을 표시한다. 기존 resources.prepare와 resources.final_revision 진단 stage, 실패 acknowledgement, exact source와 gameplay pin 및 resource closure 검증은 유지한다. 성공 Reload의 목록 안내가 실패 이유를 대신하지 않게 한다.

## G02. Server 승인 경로

Server/Private/GameRoom_KoukuRaidFlow.cpp의 Begin_KoukuRaidPreparation에서 published load 실패, non-Kouku baseline 불일치, requested/published Action 불일치와 이전 revision 거절을 구분한다. 실제 CGameplayCatalog status를 실패 이유에 보존한다. 늘어난 이유 문자열은 기존 192-byte wire 계약에 맞게 UTF-8 문자 경계를 보존하여 전송한다. admission 조건과 기존 pinned generation 보존은 변경하지 않는다.

## G03. 검증

같은 원본 로그와 publisher 시각을 RESULT에 남긴다. 실제 파일의 encoding과 diff, git diff --check를 확인한다. 기존 published reload lock 검사 및 raid admission 회귀 검사는 root 통합 빌드와 검증에서 수행한다. Client/UI 실행, publisher, 프로세스 조작 및 자체 빌드는 수행하지 않는다. 새 C++ 파일이나 프로젝트 등록은 없다.

## G04. bundle 재로드 검사의 Retail 필드 보정

최종 Debug bundle 실행에서 기존 unrelated damage 변경 검사가 실패했다. fixture는 DAMAGE의 마지막 필드를 rate로 가정하지만 Retail의 6열 마지막 필드는 spread이며 99999를 넣으면 정상 validation이 실패한다. ServerGameplayContractTests_KoukuBundles.cpp의 fixture를 stable damage ID와 세 번째 rate 필드 기준으로 수정한다. 변경된 디스크 catalog 자체의 유효성과 새 rate를 확인한 뒤 실제 admission이 기존 rate/coefficient/addend/spread를 유지하는지 확인한다. 제품 runtime과 허용 범위는 변경하지 않는다.
