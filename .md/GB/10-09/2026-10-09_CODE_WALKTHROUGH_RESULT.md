# 한국어 주석과 코드 학습 사본 작성 결과

## 2026-10-09 전체 작업 통합 검증

사용자가 전체 미커밋 변경의 확인과 Git 전달·병합을 요청했다. BossToolTests 이름 변경, 한국어 설명 규칙, 학습 솔루션·문서, Engine 주석·공백 정리와 Server IOCP 작성 중 파일·프로젝트 등록을 함께 전달한다. 제품 IOCP는 선언 중인 헤더와 빈 CPP이며 기존 ClientSession/ServerApp 전송 경로는 유지한다. IOCP 구현·연결 완료로 기록하지 않는다.

- 정본 Product runner의 x64 Debug Engine → Shared → Server → Client 컴파일·링크·배포가 성공했다. 최종 소스 기준 재확인도 exit 0이며 결과는 `out/BuildPipeline/runs/20261009T125241384Z-debug-product.json`이다. 최종 재확인에서 CSO 쓰기 0, runtime 입력 누락·유효성 오류 0이다. 기존 문자셋 경고는 남아 있다.
- 이름 변경 관련 기존 Python 검사 3개와 Build runner PowerShell 구문 검사가 통과했다. 기존 별도 검사 1개의 `/MP /utf-8` 문자열 기대 실패는 이름 변경 전후 동일하며 이번 변경의 새 실패가 아니다.
- 변경 project/filter XML 6개, 학습 JSON 4개, Python AST 9개와 학습 원본·사본 manifest hash를 확인했다. 새 소스를 포함한 전체 전달 범위의 `git diff --check`를 통과하도록 후행 공백을 정리했다.
- BossToolTests Debug 빌드·그래프 11/11 및 IOCP 학습 정적 라이브러리 빌드의 기존 동일 날짜 기록도 대조했다. Client/UI 실행, Release 제품 빌드와 실제 IOCP 서버 동작은 이번 통합에서 검증하지 않았다.
- 과거 `out/ServerConcurrency20261009`와 렌더링 조사 폴더는 현재 없으므로 해당 문서에서 과거 실험 기록과 현재 사용할 수 있는 학습 소스를 구분했다. 컴파일 산출물·Resources·out 자료는 Git 전달에서 제외한다.

2026-10-09, `codex/server-iocp-job-comparison` 작업 트리의 현재 소스와 이전 계획서·실측 기록을 구분해 정리했다.

## 반영한 규칙

CLAUDE의 새 C++ 영문 주석 기본 규칙과 기존 주석 언어에 맞추라는 규칙을 한국어 설명 기본으로 바꿨다. AGENTS와 `.md/GB/local.md`에는 존재 이유, 호출자·입출력, 상태·소유권·수명, 스레드 경계, 실패 정리, 수식의 단위·좌표계·다음 소비자를 설명하도록 명시했다. 기존 소스 인코딩 보존과 무관한 동작 변경 금지는 유지했다. 변경 전부터 있던 CLAUDE의 BossToolTests 이름 변경도 보존했다.

## 저장된 IOCP 헤더 검토

처음에는 디스크 헤더에 pragma만 있었다. 사용자에게 저장 상태를 확인한 뒤 실제 저장된 전체 선언과 질문 주석을 다시 읽었다. 확인 시점의 차이는 다음 세 가지다.

1. `On_IocpMaintanance`는 `On_IocpMaintenance`와 철자가 다르다.
2. public `Post_Send` 선언이 없다.
3. private `m_ReceiveCompletions`, `m_SendCompletions`, `m_PartialSends` 선언이 없다.

제품 `IocpService.cpp`는 여전히 빈 파일이고 ClientSession/ServerApp은 기존 전송 경로다. 헤더만으로 IOCP 실행이 연결됐다고 기록하지 않았다. 작성 중인 제품 헤더의 질문 주석과 사용자 변경을 덮어쓰지 않고, 보완 위치를 `Study/CodeWalkthrough/README.md`에 정확한 기존 선언 기준으로 제시했다.

## 작성한 사본과 안내

입구는 `Study/CodeWalkthrough/README.md`, VS 솔루션은 `Study/CodeWalkthrough/CodeWalkthrough.sln`이다.

- IOCP 선언·구현 설명본: `Server/Public/IocpService.h` 176줄, `Server/Private/IocpService.cpp` 301줄. 합계 477줄.
- 서버 패킷·세션·snapshot 설명 사본 4개.
- Profiler 선언·구현 설명 사본 2개.
- 재질 표면·GBuffer·deferred 합성 셰이더 설명 사본 3개.
- 코드 11개 전체 합계 9,690줄. 각 분야의 `*_CALL_FLOW_STUDY.md`는 실제 호출 순서, 원본/사본 파일 링크, 선택 이유와 계측 범위를 설명한다.

`Reference` 9개는 전체 원본에 한국어 주석을 보완한 사본이다. 모든 의존 헤더·include를 복제한 제품 빌드 패키지는 아니다. 학습 프로젝트에서 `None`으로 노출하고 원본 제품에는 등록하지 않았다. 셰이더 `FxCompile` 항목은 0개다. 직접 수정 연습을 해도 제품 코드가 자동 교체되지 않는다.

IOCP 두 파일은 이전 out 후보 폴더가 현재 디스크에 없어 남아 있는 SERVER_IOCP_JOB_COMPARISON_PLAN의 전체 코드에서 복구했다. 한국어 설명 외 변경은 `<exception>` 직접 include, 접수 즉시 실패의 `delete raw`를 `unique_ptr` 회수·즉시 파괴로 표현한 두 부분이다. 원래 해제 후 pending 감소 순서를 보존한다. 실제 제품 전송 연결이나 재측정을 수행한 변경은 아니다.

기존 IOCP PLAN의 해당 H/CPP 전문도 설명본과 같게 갱신했다. GUIDE의 현재 학습 입구를 새 솔루션으로 고치고 없어진 out 후보 링크는 남아 있는 PLAN 코드 위치로 연결했다. 이전 실험 폴더의 재실행 명령은 현재 사용할 수 없음을 명시했다. 이전 RESULT의 측정 기록을 새로 재실행한 증거로 바꾸지 않았다.

## 검증

- IOCP 학습 프로젝트 x64 Debug 정적 라이브러리 컴파일 성공, exit 0. 실행 EXE나 서버 성능 검증이 아니다. `out/CodeWalkthrough20261009/iocp-build.log`에 기록했다. 이후 독립 리뷰로 고친 내용은 설명 주석뿐이다.
- project/filter XML parse, 항목 일치·물리 경로·필터 연결 확인: ClCompile 1, ClInclude 1, None 13.
- 서버 4개, profiler 2개, shader 3개 사본의 주석·공백 외 코드 토큰 동일성과 원본 SHA 보존을 검사했다. 각 `*-copy-manifest.json`을 파싱하고 원본·사본 hash를 다시 검증했다. Git 전달 점검에서 Deferred·PacketFrame·PacketStreamParser 학습 사본의 후행 공백과 마지막 빈 줄을 정리하고 사본 hash·줄 수를 갱신했다. 제품 원본은 그대로 보존했다.
- IOCP 두 파일과 PLAN 전문 일치를 확인했다.
- 학습 README와 세 분야 문서의 로컬 링크 152개 및 지정 행 범위를 확인했다. 문서 UTF-8과 후행 공백을 확인했다.
- IOCP 독립 읽기 리뷰에서 송신 owner의 null 금지, const 참조 외 별칭 수정 금지, pending 0과 최종 worker join의 구분을 보완했다.
- 변경 규칙 문서의 `git diff --check`를 확인했다. 제품 전체 빌드·광역 회귀 검사는 실행하지 않았다.

최종 집계·경로 검증은 `out/CodeWalkthrough20261009/validation.json`, 사본의 기준 hash는 `Study/CodeWalkthrough/*-copy-manifest.json`에 있다.

## 렌더링·계측에서 확인한 구분

PPT의 50%는 2026-10-07 집계 코드 156,924,941 bytes 중 HLSL/HLSLI 78,086,130 bytes, 약 49.76%라는 규모 지표다. 해당 shader 줄 수 1,343,936줄에는 주석·빈 줄과 복사된 include가 포함된다. GPU 실행 비중이나 개선율이 아니다.

15분은 2026-09-29 빌드 기록의 FXC task 15분 45.697초이며 개별 shader command→CSO 경과와 겹치는 구간이 있다. CModel의 런타임 asset 로딩, CShader의 CSO 읽기, FXC 소스 컴파일, 매 프레임 GPU 실행은 서로 다른 비용이다. 이번에 15분 빌드를 재실행하지 않았다.

Profiler는 수동 CPU scope의 QPC 경과와 D3D11 GPU timestamp/disjoint query를 기록한다. GPU Pending을 0ms로 취급하지 않고 CPU/GPU·부모/자식 중첩을 합산하지 않는다. deadlock detector가 아니며 현재 서버 packet benchmark의 증거도 대신하지 않는다. 구체적인 측정 지점·분모·p95 정의 차이·면접 답변은 PROFILER_CALL_FLOW_STUDY에 기록했다.

PPT, 제품 C++/HLSL, 사용자 렌더 설정과 Data/Resources는 변경하지 않았다. 기존 Engine/GameInstance 변경, IOCP 수동 작성, BossToolTests 이름 변경을 보존했다. Client/UI 실행·화면 판정·새 GPU A/B·서버 통신 실험·commit/push는 하지 않았다. 현재 제품 반영과 원본을 바탕으로 만든 학습 사본을 분리한다.
