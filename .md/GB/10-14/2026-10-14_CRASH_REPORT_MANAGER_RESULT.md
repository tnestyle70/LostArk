# CrashReport_Manager 최소화와 제품 연결 결과

2026-10-14. 사용자의 실제 구현·제품 빌드 요청에 따라 앞선 설명용 후보를 제품 코드로 연결했다. PLAN은 최종 파일 전문과 책임·실패·검증 계약을 담는다.

## 완료한 구현

- Engine의 CCrashReport_Manager는 public static Install 하나만 공개한다. terminate/SIGABRT 콜백, 이전 콜백, 파일 HANDLE, 고정 출력 버퍼, Interlocked 진입 표식과 CrashText를 private 구현으로 묶었다.
- 익명 namespace의 독립 함수를 실제 클래스 멤버로 연결했다. m_ 이름과 Allman 중괄호, 짧은 한국어 처리 블록 주석을 적용했다. GameInstance 소유·singleton·프레임 Update는 없다.
- ClientDiagnostics H/CPP의 PrepareCrashReport 하나가 EXE 위치로 로그 경로를 만들고 Install을 호출한다. Client 프로젝트와 기존 00.MainApp 필터에 등록했다.
- Client.cpp의 wWinMain 시작에 설치 호출을 연결했다. ClientStartup/ClientExit 기록, MainApp의 InitializeStage wrapper와 시작 로그 전용 GPU adapter 조회를 제거했다. HRESULT 검사·실패 반환·오류 메시지·초기화 순서는 유지했다. 기존 디스크 로그 파일은 삭제하지 않았다.
- ClientWindowDisplay의 상태 8개와 보조 함수 3개를 private static 멤버로 이동했다. public 함수 8개와 창/Engine 수명, 재진입 방지, resize 예약·실패 복구 순서를 보존했다. 전체 전문은 기존 CODE_READABILITY_PLAN의 2026-10-14 절에 있다.
- 작성 규칙은 .md/TEAM/LOSTARK_ENGINE_CONVENTIONS.md, 실제 편집기 스니펫은 CodeStudy/Snippets와 .vscode/cpp.code-snippets로 연결했다. local.md는 해당 정본을 참조한다. 익명 namespace를 일괄 금지하거나 제거 자체를 성능 개선이라고 설명하지 않는다.

## 호출과 저장 위치

시작: wWinMain → Client::PrepareCrashReport → Engine::CCrashReport_Manager::Install → CreateFileW / signal / set_terminate.
종료: CRT → OnTerminate 또는 OnAbort → RecordFatalDiagnostic → CrashText가 고정 버퍼 작성 → WriteFile / FlushFileBuffers → 기존 종료 처리.

Client/Bin/Debug/Client.exe와 Client/Bin/Release/Client.exe 모두 EXE 파일명·구성 폴더·Bin을 제거하고 Default/ClientCrash.user.log를 붙인다. 저장소의 실제 경로는 C:/Users/tnest/Desktop/LostArk/Client/Default/ClientCrash.user.log다. 경로 구성 실패 때만 현재 작업 폴더의 ClientCrash.user.log를 사용한다. 정상 경로에서 파일 열기 실패는 false로 알리고 게임 시작을 이어간다.

설치 시 파일을 열지만 본문은 종료 콜백 때만 쓰므로 정상 실행의 빈 파일은 가능하다. logPath는 경로 문자열의 입력 주소이며 파일 내용 출력 버퍼가 아니다. Engine은 그 포인터를 계속 보관하지 않는다. 파일은 append로 열고 프로세스 종료 때 OS가 handle을 회수한다.

## 검증

- 최종 Engine 후보의 MSVC x64 /Zs 문법검사 exit 0. 이전 설명 후보 검증과 실제 제품 빌드를 구분한다.
- 실제 Client 프로젝트와 .filters XML parse 통과.
- 실제 변경 C++/프로젝트/local 규칙의 git diff --check 통과.
- Engine H/CPP의 UTF-8 BOM 없음/CRLF, Display H/CPP의 UTF-8 BOM/CRLF, Client.cpp/MainApp.cpp의 기존 UTF-8을 유지했다. 기준 hash 확인 뒤 적용했다.
- 독립 검토에서 Display의 public 계약·적용/복구 순서와 Client/MainApp의 초기화 순서·실패 반환을 확인했다.
- 첫 Debug Product 빌드는 TriggerBox/new 공통 헤더 정리 요청으로 범위가 확대되어 중단했다.
  최종 Debug x64 Product 빌드는 exit 0이며 Engine·Shared·Server·Client가 모두 PASS했다.
  이어 빈 new 보호 구문 제거와 최신 CrashReport 소스까지 증분 재검증했고 exit 0이었다.
  이 재검증에서 CrashReport_Manager.cpp/Physics_Manager.cpp 두 OBJ를 다시 생성했다.
  본빌드 로그는 out/CrashReportGuide20261014/product-debug-final.log,
  최종 확인 로그는 out/CrashReportGuide20261014/product-debug-confirm.log다.
  제품 검증 결과는 out/BuildPipeline/runs/20261014T075124779Z-debug-product.json 및
  20261014T075146664Z-debug-product.json이며 둘 다 result=PASS, skippedBuild=false다.
  컴파일·링크 오류는 없었으며 인코딩 C4819/C4828, 외부 PDB LNK4099 및 기존 shader 경고는 남았다.
- Product 파이프라인의 런타임 파일 존재·Navigation 참조·Item/Valtan reward catalog 검사가 통과했다.
  데이터를 publish하거나 화면·오디오를 확인한 결과는 아니다. Release 빌드는 이번에 실행하지 않았다.
- 사용자 추가 빈 줄과 Install 앞 경로 질문 주석은 보존했다. 그 질문의 답은 시작 시 Install이
  경로를 CreateFileW에 전달하고 나중 종료 시 보관한 HANDLE로 WriteFile한다는 것이다.

Client/UI는 실행하지 않았다. 의도적 크래시와 실제 파일 기록은 실행 검증하지 않았으며 컴파일·링크 성공으로 대신하지 않는다. 지원 범위는 terminate/SIGABRT다. SEH·minidump·외부 강제 종료 감시는 없으며 스택 주소 해석에는 같은 빌드의 EXE/DLL과 PDB가 필요하다.

## Trigger_Box DirectXTK 질문 조사

처음 읽기 조사한 뒤 사용자가 실제 분리를 요청해 Engine CWireBoxRenderer와 Client Trigger_Box에 반영했다. 아래 조사 내용은 분리 전 직접 의존의 근거다. Trigger_Box.cpp는 PrimitiveBatch/BasicEffect/InputLayout을 만들고 위치·색 정점과 12변을 제출하며 view/projection을 설정한다. 현 구현에서는 PrimitiveBatch.h, VertexTypes.h, Effects.h가 필요하다. H 전방 선언만으로 실제 객체 생성과 메서드 호출을 대신할 수 없다.

push/undef/pop은 프로젝트 Debug new 매크로를 외부 헤더 파싱 중 잠시 껐다 복원한다. 이 세 헤더에서 직접 raw new 토큰은 찾지 못했으므로 제거 시 반드시 현재 빌드가 깨진다고 단정하지 않는다.

Engine의 DebugDraw.cpp에는 범용 OBB 그리기가 이미 있고 Collider.cpp가 같은 GPU 준비를 한다. 이 구현을 재사용하는 Engine 선 그리기 경계를 마련하고 Client에는 placement·선택 색·저작 표시 정책을 남기는 분리가 적절하다. 기존 DX::Draw는 PrimitiveBatch 인수를 요구하고 DLL export 경계가 없어서 include만 옮기면 의존이 사라지지 않는다. CCollider를 대신 끼우면 교차검사 상태와 고정 빨강/초록 정책도 가져오므로 동일 기능으로 보지 않는다. NONLIGHT 호출 위치를 바꾸면 UI 전후 렌더 순서도 달라질 수 있다. 최종 제품에서는 Trigger_Box의 GPU 자원과 DirectXTK include를 제거하고 CWireBoxRenderer에 맡긴다. 선택·표시·descriptor·NONLIGHT는 Client에 남는다. 세부 전문은 기존 CODE_READABILITY_PLAN의 같은 날 후속 절에 있다.

## new 매크로 후속 정리

Engine_Defines의 전역 new/DBG_NEW 치환과 Engine·Client의 new 전용 보호 구문을 제거했다. min/max 보호·외부 include·CRT Debug 보고/할당번호 중단은 유지했다. 표준 new의 CRT 추적은 별도 콘솔 실험으로 확인했고 자동 원본 파일·행 정보가 붙지 않는 차이를 기록했다. 실제 new 실행문을 다른 매크로로 치환하지 않았다.

## 컨벤션 재적용과 최종 확인

CCrashReport_Manager의 클래스 전체 ENGINE_DLL을 제거하고 public Install 선언에만 적용했다.
최종 Engine.dll의 dumpbin /exports에서 CCrashReport_Manager 심벌은 Install 하나다.
private 상태와 콜백은 Engine 내부에 남고 기존 호출 계약과 프로세스 수명은 유지한다.
Number/Address는 실제 날짜·스레드 ID·스택 주소·모듈 상대 주소를 고정 버퍼에 쓰는 내부 구현으로 유지했다.
사용자가 작성한 Install 앞 질문 주석과 빈 줄을 보존하고 질문 주석 끝의 공백 한 곳만 제거했다.

컨벤션 적용 뒤 Debug x64 Product 본빌드 결과는 20261014T080535356Z-debug-product.json이다.
확인 빌드가 중간에 끊겨 재실행했고 20261014T081540195Z-debug-product.json이 PASS했다.
마지막 주석 공백 정리 후 CrashReport_Manager.cpp도 재컴파일한 최종 결과는
out/BuildPipeline/runs/20261014T081652767Z-debug-product.json이며 result=PASS, skippedBuild=false다.
Engine·Shared·Server·Client 네 프로젝트가 모두 통과했고 컴파일·링크 오류는 없다.
로그는 out/CrashReportGuide20261014/Conventions/product-debug-final.log다.
기존 인코딩·외부 PDB 경고, Release 미검증, Client/UI·실제 fatal 기록 미실행의 경계는 위와 같다.

최종 scoped diff 검사 104경로와 신규 파일 17개 공백 검사, 프로젝트 XML 4개 parse가 통과했다.
CrashReport_Manager/WireBoxRenderer/Engine_Defines의 Public 원본과 EngineSDK 사본 SHA256이 같다.
상세 검증은 Conventions/final-scoped-validation.json과 engine-exports.txt에 남겼다.
코드 전문 PLAN은 이 최종 제품 파일 기준으로 갱신했다.
