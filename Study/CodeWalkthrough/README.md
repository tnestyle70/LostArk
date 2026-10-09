# 한국어 주석으로 읽는 LostArk 코드

지금 첫 파일은 [IOCP 헤더 설명본](Server/Public/IocpService.h), 다음 파일은 [IOCP 구현 설명본](Server/Private/IocpService.cpp)이다. 제품 파일에 바로 덮어쓰는 자동 설치기는 아니다. 작성 중인 제품 소스와 질문 주석은 그대로 두고 여기서 함수와 호출 흐름을 먼저 읽는다.

## G01. 방금 작성한 헤더의 세 군데 보완

대상은 `C:/Users/tnest/Desktop/LostArk/Server/Public/IocpService.h`다. 2026-10-09에 저장한 헤더를 직접 읽었으며 클래스와 주요 선언은 들어왔다. 대응 CPP는 아직 비어 있다. 아래는 확인 시점의 세 가지 차이다.

**1. `IIocpParticipant`의 순수 가상 함수 이름을 바꾼다.**

`On_IocpMaintanance`를 아래 이름으로 교체한다. 이후 서비스와 세션이 동일한 함수 이름을 사용해야 한다.

```cpp
virtual void On_IocpMaintenance() noexcept = 0;
```

**2. `CIocpService`의 public 구역에서 `Post_Receive` 선언 직후, `Is_WorkerThread` 직전에 추가한다.**

```cpp
// 현재 프레임의 offset 이후를 비동기 송신한다.
// shared_ptr는 Windows가 송신을 끝낼 때까지 프레임 메모리를 유지한다.
bool Post_Send(SOCKET socket, const std::shared_ptr<IIocpParticipant>& owner,
    std::shared_ptr<const std::vector<std::uint8_t>> bytes,
    std::size_t offset, int& error);
```

**3. private 구역에서 `m_Pending/m_PeakPending/m_Posted/m_Completed` 선언 직후, `m_ReceivedBytes/m_SentBytes` 직전에 추가한다.**

```cpp
// 수신 완료, 송신 완료, 요청보다 짧게 완료된 송신의 횟수다.
// 여러 워커가 함께 증가시키므로 atomic으로 갱신 한 건의 유실을 막는다.
std::atomic<std::uint64_t> m_ReceiveCompletions{ 0 },
    m_SendCompletions{ 0 }, m_PartialSends{ 0 };
```

이 세 군데를 먼저 맞춘다. CPP의 함수 정의는 헤더에 선언한 이름과 타입을 그대로 사용하므로 누락 상태에서 CPP 전체를 붙이면 컴파일 오류가 난다. [설명본 헤더](Server/Public/IocpService.h)는 보완된 전체 선언과 질문에 대한 주석을 함께 담았다.

## G02. IocpService.cpp를 작성하는 순서

제품 작성 위치는 `C:/Users/tnest/Desktop/LostArk/Server/Private/IocpService.cpp`다. 사용할 전문은 [한국어 주석이 있는 CPP](Server/Private/IocpService.cpp)에 있다. 이미 두 제품 파일은 Server 프로젝트와 `01.Network` 필터에 등록되어 있어 XML을 다시 추가할 필요가 없다.

| 순서 | 함수·타입 | 먼저 이해할 책임 |
|---|---|---|
| 1 | `OPERATION` | Windows가 작업하는 동안 세션·버퍼·OVERLAPPED를 같이 살려 둔다. |
| 2 | `Start`, `Associate` | 공용 완료 포트와 워커를 만들고, accept된 소켓의 완료가 그 포트로 오게 한다. |
| 3 | `Post_Receive`, `Post_Send` | 한 번 맡길 작업과 메모리 범위를 준비한다. 게임 규칙은 처리하지 않는다. |
| 4 | `Submit` | pending을 먼저 늘리고 Windows에 맡긴다. 접수 실패와 정상 PENDING을 구분한다. |
| 5 | `Worker_Loop` | 완료된 작업을 회수하고 해당 세션의 `On_IocpCompleted`를 부른다. |
| 6 | `Complete_Pending`, `Stop` | 완료까지 끝난 작업을 정리하고 남은 작업이 없을 때 워커와 포트를 종료한다. |
| 7 | `Run_Maintenance`, 조회 함수 | 조용한 연결도 점검하고 완료·바이트 카운터를 읽는다. |

```text
ServerApp 시작(이후 연결할 부분)
  → CIocpService::Start
  → accept된 세션: Associate / Register / Post_Receive
  → Submit → WSARecv
  → Windows의 I/O 완료
  → Worker_Loop → GetQueuedCompletionStatus
  → 해당 세션의 On_IocpCompleted
  → 세션의 패킷 조립 → ServerApp의 명령 분배 → GameRoom의 명령 큐
```

여기서 `Post_Receive`가 성공했다는 말은 Windows가 작업을 접수했다는 뜻이다. 패킷이 이미 도착했다는 뜻은 아니다. IOCP의 completion packet도 게임 프로토콜 packet과 다른 개념이다. 전자는 I/O 작업 완료 알림이고 후자는 길이·종류·payload를 가진 게임 메시지다.

헤더와 CPP를 작성해도 기존 `ClientSession`과 `ServerApp` 연결 전까지 제품 통신 경로는 그대로다. 지금은 서비스 한 단위를 이해하고 컴파일하는 단계다. 주석에 있는 이후 호출 흐름과 현재 이미 연결된 코드를 구분한다.

## 헤더에 남긴 질문의 답

- **Participant:** 완료 통지를 받는 참여자다. 이후 `CClientSession`이 이 인터페이스를 구현한다. 서비스가 게임 세션 구현 전체에 의존하지 않고 콜백 계약만 알게 한다.
- **virtual / =0:** 호출할 약속을 기반 타입에 두고 실제 처리는 세션이 구현한다. 기반 타입으로 파괴해도 파생 클래스 정리가 가능하도록 소멸자도 virtual로 선언한다.
- **shared_ptr / weak_ptr:** 작업이 끝나기 전 세션이 사라지면 콜백이 해제된 메모리를 읽는다. 작업의 shared_ptr는 완료까지 살리고, 점검 목록의 weak_ptr는 종료한 세션을 목록 때문에 영구 보존하지 않는다.
- **uint32_t와 uint64_t:** 한 번의 완료 bytes는 Windows의 32비트 값에 맞춘다. 누적 합계는 장시간 커지므로 64비트다. 32비트 누적 바이트는 약 4GiB에서 넘친다.
- **Stop의 5000:** 5초 뒤 종료를 예약하는 값이 아니다. 지금 종료를 시작하면서 기다릴 상한이다. 이 구현은 drain과 join에 별도로 사용한다.
- **[[nodiscard]]:** 의미 있는 조회 반환값을 무시하면 컴파일러가 경고하도록 권한다. 실행 중 잠금을 걸거나 반환값 사용을 강제하는 기능은 아니다.
- **private와 OPERATION 전방 선언:** 외부가 pending/포트를 직접 바꿔 접수·회수 순서를 깨지 않게 한다. 작업의 구체적인 메모리 배치는 CPP에서만 알도록 숨긴다.
- **atomic과 mutex:** atomic은 한 변수의 연산을 다룬다. 여러 변수의 일관된 상태나 vector 변경까지 자동 보호하지 않는다. mutex는 보호 범위를 직접 정해야 한다. 어떤 헤더도 포함만으로 데드락을 방지하지 않는다.
- **condition_variable:** 작업이 없을 때까지 CPU를 태우며 반복하지 않고, 마지막 완료가 조건을 바꾸면 잠든 종료 담당자를 깨운다.
- **카운터를 여러 줄로 구분:** 작업 수, 완료 종류별 수, 바이트 합계라는 의미를 나눈 것이다. 줄바꿈이 동기화 범위를 나누는 것은 아니다.

## G03 이후 읽을 사본

`Reference`는 현재 제품 소스를 복사하고 설명 주석만 더한 읽기·수정 연습용 파일이다. 헤더와 의존 파일 전체를 복제한 제품 프로젝트가 아니므로 참조 사본 전체를 그대로 빌드하는 구성은 아니다. 각 manifest는 기준 원본 hash, 인코딩, 줄 수, 주석 외 토큰 동일 여부를 기록한다. 이후 제품 소스가 바뀌어도 사본이 자동 동기화되지는 않는다.

- [서버 호출 흐름](../../.md/GB/10-09/2026-10-09_SERVER_CALL_FLOW_STUDY.md): 패킷 수신부터 명령 처리·snapshot 송신까지.
- [프로파일러 호출 흐름](../../.md/GB/10-09/2026-10-09_PROFILER_CALL_FLOW_STUDY.md): CPU/GPU 계측 위치, 대기, 표본과 percentile의 의미.
- [셰이더 호출 흐름](../../.md/GB/10-09/2026-10-09_SHADER_CALL_FLOW_STUDY.md): CPU 입력 바인딩, 수식, 중간 렌더 타깃과 최종 색.

## Visual Studio에서 여는 방법

같은 폴더의 `CodeWalkthrough.sln`을 열고 `00.Start/README.md`부터 본다. `01.IOCP`는 컴파일할 두 파일, `02.ServerReference`·`03.ProfilerReference`·`04.ShaderReference`는 설명용 사본이다.

`CodeWalkthrough` 프로젝트 빌드는 IOCP 두 파일의 선언·정의만 확인하는 정적 라이브러리를 만든다. 실행 EXE가 없으므로 F5 실행 대상이 아니다. 제품 Server/Client, 셰이더, 리소스 변환을 빌드하지 않는다. 제품에 적용한 뒤의 서버 통신 검증과는 별개다.

## 성능 설명을 작성하는 기준

한 사례마다 실제 문제·기준 조건 → 선택한 코드 경로 → 비용을 줄이는 원리 → 새로 늘어나는 비용 → 같은 조건의 전후 결과 → 적용/보류 판단 순서로 설명한다. 코드량 비중, 빌드 시간, CPU 처리 시간, GPU 시간, FPS를 서로 대신 쓰지 않는다. 현재 파일이 없어진 과거 out 측정 로그는 재확인한 원시 증거로 표시하지 않고 당시 RESULT의 기록으로 구분한다.

Windows API 동작은 [IOCP](https://learn.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports), [WSARecv](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecv), [GetQueuedCompletionStatus](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus)의 공식 계약과 대조했다.
