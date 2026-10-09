# LostArk IOCP·JobSystem 비교 후보 검증 결과

2026-10-09 · 수동 반영용 후보 · 기준 commit 45faba4ba0806cbfbcfd02b213404fb98901b961

## G01. 완료 상태와 적용 판단

최초 비교 작업에서는 제품 Server/Shared/Client 소스를 수정하지 않았다. 기존 구조를 유지하며 select/IOCP와 serial/Chase–Lev scheduler를 선택하는 후보 30파일(신규18·수정12)을 만들고 분리된 복사본에서 컴파일·측정했다. PLAN에는 수정 후 전체 코드, 신규 H/CPP, 프로젝트/필터 등록과 실행 스크립트 전문이 있다. patch는 당시 기준 소스에서 git apply --check만 수행했고 실제 적용하지 않았다.

2026-10-09 통합 전 재확인 시 제품 `Server/Public/IocpService.h`는 작성 중인 선언이고 `Server/Private/IocpService.cpp`는 빈 파일이다. 두 파일의 프로젝트 등록은 있지만 `ClientSession`·`ServerApp`의 IOCP 연결은 구현하지 않았다. 현재 학습 입구는 [CodeWalkthrough README](../../../Study/CodeWalkthrough/README.md)와 [학습용 솔루션](../../../Study/CodeWalkthrough/CodeWalkthrough.sln)이다. 아래 비교·빌드 수치는 최초 작업 당시의 기록이며 이번 통합에서 재측정한 결과가 아니다. 당시 `out/ServerConcurrency20261009`의 후보·원시 측정·로그는 현재 디스크에 없어 다시 대조할 수 없으며 Git 전달 대상에도 포함하지 않는다.

**현 시점 기본 선택은 기존 select + serial 유지다.** 이 후보의 IOCP는 2·6워커에서 스레드 증가를 억제했지만, 측정한 reliable echo 처리량·RTT·frame당 CPU 비용이 기존 select 경로보다 나빴다. IOCP 일반의 한계나 모든 Windows 서버의 우열로 확대하지 않는다. fanout 병렬화는 transport·연결 수·payload에 따라 이득과 회귀가 섞였다. 도입 자체와 성능 개선을 구분한다.

## G02. 실제 코드에서 확인한 기존 구조

| 영역 | 확인한 파일·위치 | 현재 코드 사실 |
|---|---|---|
| Server 실행 | Server/Private/ServerApp.cpp:2627,2708,5017 | accept1 + room1; 30Hz room thread가 모든 simulation을 순차 Tick |
| TCP | Server/Private/ClientSession.cpp:217,220,619 | 세션별 recv/send thread2개, nonblocking socket와 select 대기 |
| 송신 큐 | Server/Public/ClientSession.h:183 | 4096frame/8MiB, reliable reserve, snapshot coalescing, reliable FIFO |
| packet | Shared/Public/Network 및 Shared/Private/Network | 자체 PacketWriter/Reader와 stream parser; 현재 LostArk는 FlatBuffers/Protobuf codec이 아님 |
| Engine CPU pool | Engine/Private/CpuJobPool.cpp:68,140 | Windows threadpool + CAS index 분배; 서버가 소비하는 Chase–Lev pool은 아님 |
| 이전 병렬 측정 | .md/GB/10-04/2026-10-04_BERN_SPATIAL_CHUNK_HLOD_RESULT.md G22 | 실제 맵 CPU 준비에서 안정적 병렬 순이득 미입증, 기본 OFF |

제품의 명시적 스레드 수는 main 포함 기존 3+2N, IOCP 선택 시 3+W이며 job worker J를 켜면 +J다. OS 내부 스레드는 별도다. 아래 하네스 관측 스레드 수에는 하네스 제어·표본 수집·OS 스레드도 포함되므로 이 식과 숫자를 혼용하지 않는다.

WintersEngine의 Engine/Public/Core/JobSystem/WorkStealingDeque.h, Engine/Private/Core/JobSystem.cpp, Engine/Private/ECS/SystemScheduler.cpp에는 owner/thief deque와 외부 injection queue, 시스템 접근 충돌을 나누는 Submit/Wait 소비가 있다. Client/Private/Network/Client/CommandSerializer.cpp는 FlatBuffers builder를 사용한다. 하지만 Server/Private/Game/GameRoomTick.cpp의 gameplay는 상태 mutex 아래 직렬이며 서버 JobSystem의 존재를 실제 room 병렬 가속 성과로 바꿔 말하면 안 된다.

저장된 외부 팀 포트폴리오 원문에서 Google 기술로 소개된 항목은 Protobuf이며, 사용자 Winters의 .fbs/FlatBuffers와 구분한다. Notion 링크 본문은 접근하지 못했으므로 읽거나 수정했다고 기록하지 않는다.

## G03. 후보의 변경 경계

```text
select recv thread / IOCP receive completion
 → 동일 Shared parser → 기존 On_SessionFrame → room command queue
 → 단일 30Hz gameplay Tick
 → snapshot encode 1회
 → serial / job fanout (session별 framing·enqueue·IOCP 초기 게시)
 → select sender / IOCP send completion
```

프로토콜, 서버 권위, gameplay single writer, reliable transaction, 큐 상한·coalescing은 유지한다. IOCP는 receive1개/send1개씩 게시하며 OVERLAPPED별 session 강한 참조와 buffer 수명을 유지한다. 모든 session posting 중단·Stop 완료 뒤 service Stop을 호출한다. CIocpService 자체의 임의 concurrent Submit/Stop을 허용하는 계약은 아니다. 종료 중 세션이 service보다 늦게 소멸하는 경우의 재진입, Start 실패 중복 통지도 후보에서 보완했다.

Chase–Lev의 고정 deque는 worker owner만 bottom을 다루며 thief는 top CAS를 사용한다. scheduler에는 bounded mutex injection queue, overflow caller-runs, nested Wait의 caller helping, exception 전달, C++20 atomic wait/notify가 있다. 전체 서버 또는 scheduler 전체가 lock-free인 것은 아니다. Fiber는 이번 후보에 없다.

fanout은 payload·recipient 목록을 고정하고 job 완료를 join한 뒤 owner가 오류를 처리한다. 각 job은 자기 session과 결과 index만 사용한다. 현재 room thread의 root 제출은 외부 injection 경로이며 측정96회 모두 localPushes/steals=0이었다. 아래 병렬 성과는 외부 큐를 통한 작업 분배의 효과이고 Chase–Lev steal 연산의 가속을 증명하지 않는다. deque 경합 정확성은 별도 검증했다.

## G04. 실행한 검증과 남은 범위

- 분리한 Server/Shared 전체 x64 Release 컴파일·링크 PASS. 원본 제품 출력·데이터 publisher·Client/UI 실행 없음. 기존 ValtanBrain.cpp 변환 경고2건이 전체 빌드에 남았고 최종 증분은 오류0. Debug 전체 제품 빌드와 실제 게임 실행은 미실시.
- 최종 native harness x64 Release /O2 컴파일·링크 PASS.
- JobSystem 마지막 원소 경합20,000회: owner9,748/thief10,252, violations0. exactly-once10,000, 다중 외부 제출·중첩 Wait, bounded overflow, job exception, 다른 pool 호출, 자기 counter 대기 거부, shutdown accepted nested drain PASS.
- 두 transport × 두 worker 조건의 correctness 총4실행 PASS. 매 실행 실제 수신을 확인한46개 세션의 accepted/closed=46/46. fragmentation/coalescing, 큰 frame FIFO, reconnect, 동시 close와 server shutdown 검증.
- IOCP 2워커 correctness: posted=1176, completed=1176, pending=0. 실제 partial-send completion=0; 이 실행으로 부분 완료 분기 재현 성공을 주장하지 않는다.
- IOCP 6워커 correctness: posted=1205, completed=1205, pending=0. 실제 partial-send completion=0; 이 실행으로 부분 완료 분기 재현 성공을 주장하지 않는다.
- transport 성능48실행(두 worker 조건 각각24), fanout96실행의 payload·개수·checksum/순서·queue 유효성 gate PASS. fanout coalesced/dropped0.
- candidate project/filter XML 및 ProjectReference GUID, JSON, PowerShell parse PASS. review patch 적용 가능 검사와 CRLF를 허용한 whitespace 검사 PASS.

deadlock lock-order graph/순환 탐지기는 구현하지 않았다. 종료 timeout·join·잠금 순서 검토와 스트레스 성공을 “deadlock profiling 구현”으로 표현하지 않는다. 실제 lock wait 분석이 필요하면 ETW/WPA 등 스케줄링 관측과 lock-level 계측을 별도 설계한다. 무교착의 수학적 증명이나 모든 스케줄 검증도 아니다.

## G05. 환경·측정 방법

CPU AMD Ryzen AI 5 340 w/ Radeon 840M, 논리 프로세서12, RAM 15.0GiB, Microsoft Windows NT 10.0.26200.0. Visual Studio 18 Insiders MSBuild18.11 / v143 / x64 Release. EXE SHA256 `C587B66EE677649B85E24E649E550EFFAB9D6D50E861D1E8B2AC2EFC6B53DDB7`. 측정 시작 전 MSBuild/cl/link/다른 하네스가 없는지 확인했고 CPU affinity·전력정책 고정·전용 OS 환경은 사용하지 않았다. 일반 데스크톱의 다른 background 부하는 완전히 통제하지 못했다.

Transport는 별도 서버/클라이언트 프로세스, loopback, 256-byte payload, 연결당 window8, warmup1초+측정3초, ABBA2회다. 각 조건4trial의 처리량과 percentile 중앙값을 표로 표시한다. 표의 p95/p99는 합친 모든 sample의 percentile이 아니다. closed-loop 처리량이며 고정 offered-load의 수용량이나 실제 LAN/게임 tick 결과가 아니다. select도 같은 후보 EXE의 보존된 backend이므로 과거 원본 EXE 대 새 EXE 비교라고 부르지 않는다.

CPU 시간은 모든 서버 thread의 user+kernel 합계여서 wall time보다 클 수 있다. 낮은 CPU 총량과 적은 처리량을 동시에 보인 IOCP를 효율 향상으로 오해하지 않도록 1,000 유효 frame당 CPU ms를 비교한다. 서버 marker 인지와 client 계측 구간의 작은 차이는 raw elapsed에 남는다.

### IOCP 2워커와 같은 배치의 select

| 연결 | backend | frames/s | RTT p95 ms | RTT p99 ms | CPU ms/1,000frame | 관측 최대 thread |
|---:|---|---:|---:|---:|---:|---:|
| 4 | select | 344,980 | 0.0982 | 0.1380 | 11.83 | 13 |
| 4 | iocp | 201,148 | 0.1706 | 0.2241 | 12.32 | 7 |
| 32 | select | 642,762 | 0.2593 | 0.4675 | 9.49 | 69 |
| 32 | iocp | 216,795 | 1.3005 | 1.6031 | 12.04 | 7 |
| 128 | select | 645,728 | 1.2495 | 1.8292 | 9.27 | 261 |
| 128 | iocp | 164,852 | 6.9498 | 8.4116 | 16.18 | 7 |

### IOCP 6워커와 같은 배치의 select

| 연결 | backend | frames/s | RTT p95 ms | RTT p99 ms | CPU ms/1,000frame | 관측 최대 thread |
|---:|---|---:|---:|---:|---:|---:|
| 4 | select | 347,476 | 0.0970 | 0.1361 | 11.77 | 13 |
| 4 | iocp | 228,599 | 0.1428 | 0.2138 | 20.47 | 11 |
| 32 | select | 622,130 | 0.3279 | 0.4851 | 9.73 | 69 |
| 32 | iocp | 327,151 | 0.8130 | 0.9718 | 15.18 | 11 |
| 128 | select | 659,163 | 1.0750 | 1.4271 | 9.09 | 261 |
| 128 | iocp | 268,272 | 3.9624 | 4.5154 | 18.89 | 11 |

6워커 추가 비교는 최초2워커의 낮은 처리량을 확인한 뒤 수행한 탐색 측정이다. 위 두 배치의 select를 각각 함께 기록했다. 2→6만으로 모든 비용을 없애지는 못했으며 더 많은 worker가 최선이라는 결론도 내리지 않는다. 처리량 목표보다 thread 예산이 중요한 조건에서의 선택은 별도의 목표·부하 기준이 필요하다.

## G06. 실제 fanout 함수의 executor 비교

IOCP worker2, job worker2. 연결4/32/128, payload1,024/16,384byte, warmup100batch+측정1,000batch, 각 조합ABBA2회. batch마다 모든 수신을 확인한다. synthetic S2C_WORLD_SNAPSHOT frame을 사용하지만 실제 gameplay snapshot payload의 내용이나 월드 상태는 생성하지 않는다. enqueue에는 framing/queue/IOCP 초기 WSASend 게시/Submit/Wait가 포함되고 socket 송신 완료 대기를 뜻하지 않는다. batch 완료는 모든 수신을 확인한 시간이다.

| backend | 연결 | payload B | serial enqueue p95 μs | jobs enqueue p95 μs | serial batch p95 μs | jobs batch p95 μs |
|---|---:|---:|---:|---:|---:|---:|
| select | 4 | 1024 | 12.40 | 16.95 | 38.55 | 51.00 |
| select | 4 | 16384 | 58.20 | 55.85 | 114.70 | 117.00 |
| select | 32 | 1024 | 92.50 | 73.85 | 155.30 | 150.55 |
| select | 32 | 16384 | 247.25 | 243.15 | 349.30 | 399.95 |
| select | 128 | 1024 | 300.65 | 264.65 | 441.25 | 429.10 |
| select | 128 | 16384 | 805.55 | 1,039.55 | 993.95 | 1,423.45 |
| iocp | 4 | 1024 | 31.45 | 29.65 | 32.85 | 30.10 |
| iocp | 4 | 16384 | 98.45 | 91.05 | 118.40 | 115.45 |
| iocp | 32 | 1024 | 242.90 | 153.40 | 249.20 | 172.95 |
| iocp | 32 | 16384 | 341.75 | 265.90 | 357.05 | 300.10 |
| iocp | 128 | 1024 | 1,031.95 | 534.35 | 1,037.80 | 565.30 |
| iocp | 128 | 16384 | 1,392.05 | 892.20 | 1,405.95 | 941.80 |

예를 들어 기존 select·4명·1KiB의 enqueue p95는12.40→16.95μs로 늘어 분배 비용이 더 컸다. IOCP·128명·1KiB는1,031.95→534.35μs로 줄었다. 두 결과는 조건이 다르며 현재4인 gameplay가 빨라졌다는 근거로 대체할 수 없다. select·128명·16KiB에서는805.55→1,039.55μs로 회귀했다. 무조건 병렬화하거나 단일 연결 수 threshold를 최선으로 고정할 증거는 없다.

fanout CPU는 송수신이 같은 process인 합산 비용이다. 특히 작은4연결 trial은 시간이 짧고 process CPU 계측 약15.6ms 단위의 영향이 커 세밀한 CPU 절감률로 사용하지 않는다. 원시 sample과 combinedCpuMs는 보관했다. 실제 서버 RoomPerf의 tick/scheduler lateness·fanout 비용, 실전 packet 크기/빈도와 동시 방 수, 고정 유입률 open-loop 부하를 다음 적용 판단에 사용해야 한다.

## G07. 후보를 다시 준비한 뒤의 수동 반영·재실행

현재 제품에는 아래 성능 하네스와 실행 스크립트가 없고 이전 `review.patch`도 남아 있지 않다. 아래는 PLAN에서 후보를 다시 준비한 뒤 사용할 절차이며, 이번 pull 직후 실행 가능한 검증 명령이 아니다. IOCP 학습 정적 라이브러리만으로 이 비교를 재현할 수 없다.

1. [PLAN](2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md)의30파일을 현재 코드와 비교해 후보와 검토 diff를 다시 준비한다. 새 H/CPP와 .vcxproj/.filters를 함께 반영한다. 기존 파일 전체 복사는 이후 다른 변경을 덮어쓰지 않는지 먼저 대조한다.
2. select+serial을 기본으로 제품 Release 빌드한다. 전송 효과는 IOCP+serial과 비교하고, executor 효과는 같은 transport에서 serial/jobs로 비교한다.
3. harness를 x64 Release 빌드하고 quick 후 portfolio profile을 실행한다. ComparisonProfiles.json에서 worker·연결·payload·반복을 명시한다. 최신 코드로 변경할 때마다 새 EXE hash와 raw data를 남긴다.
4. 실전4인 패킷 trace와 실제 동시 room 조건에서 CPU·tick p95/p99·응답 지연을 확인한 뒤 기본값을 선택한다. 이 후보를 게임 실행까지 검증한 구현 완료나 FPS 개선으로 제출하지 않는다.

```powershell
# 직접 코드 반영과 x64 Release 빌드 뒤 실행
powershell -ExecutionPolicy Bypass -File Tools/ServerConcurrencyHarness/Run-ComparisonProfile.ps1 -ExePath out/ServerConcurrencyHarness/Release/ServerConcurrencyHarness.exe -Profile portfolio

# worker 수를 바꾼 전송 비교 예
powershell -ExecutionPolicy Bypass -File Tools/ServerConcurrencyHarness/Run-TransportComparison.ps1 -ExePath out/ServerConcurrencyHarness/Release/ServerConcurrencyHarness.exe -OutputRoot out/ServerConcurrencyComparison/worker6 -IocpWorkers 6 -AbbaRepeats 2
```

## G08. 포트폴리오·면접 설명

> 기존 LostArk 서버는 연결마다 송수신 스레드를 만들고 단일30Hz room thread가 게임 상태를 갱신합니다. 이 권위 구조와 packet/queue 정책을 유지한 IOCP backend 후보를 만들고, 같은 EXE에서 기존 방식과 선택 비교했습니다. 로컬128연결 echo 실험에서2워커 IOCP는 관측 thread 수를261개에서7개로 줄였지만 처리량은약64.6만에서16.5만frame/s로 낮아져 기본값으로 채택하지 않았습니다. snapshot fanout의 job 분배도 별도로 측정했으며4명·1KiB·select에서는 p95가12.40에서16.95μs로 늘었습니다. 기술을 추가하는 것보다 workload별 순이득을 확인하고 기존 선택을 보존하는 데 초점을 두었습니다. 이 수치는 격리 합성부하이고 실제 게임 개선은 아직 측정하지 않았습니다.

“왜 IOCP인데 느린가?”에는 측정한 후보·worker 수·payload·부하 범위를 먼저 말한다. 현재 operation별 동적 할당, 완료 처리/계측, worker 수와 per-session posting 직렬화 등이 비용 후보지만 별도 CPU profile 없이는 원인을 확정하지 않는다. operation pool·completion batching·공유 immutable frame·batch jobs 등은 다음 가설이며 이번 구현의 성과로 적지 않는다.

“deadlock profiling과 Chase–Lev가 같은가?”에는 아니라고 답한다. 전자는 잠금/대기 관계의 관측·진단, 후자는 worker의 작업 분배 자료구조다. IOCP는 I/O 완료 전달, FlatBuffers/Protobuf는 packet 직렬화 형식이며 서로 다른 계층이다.

## G09. 현재 파일과 과거 근거의 보존 상태

[전체 코드 PLAN](2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md)과 [현재 학습용 솔루션](../../../Study/CodeWalkthrough/CodeWalkthrough.sln)은 저장소에 남아 있다.

다음은 당시 `out/ServerConcurrency20261009` 아래에 기록했던 경로다. 현재 해당 폴더가 없어 열 수 있는 근거 링크로 제공하지 않는다. G04~G06의 검증과 수치는 이 RESULT에 남은 과거 기록이며 원시 자료 재확인이나 이번 통합 검증으로 취급하지 않는다.

- 후보 ZIP·patch·SHA 목록: `server-concurrency-candidate.zip`, `server-concurrency-review.patch`, `install-manifest.json`.
- 환경·2워커 transport·fanout·CPU correctness: `measurements/comparison-20261009T031137751Z/` 아래의 `environment.json`, `transport-20261009T031139541Z-f22f5679/runs.json`, `fanout-runs.json`, `job-tests.stdout.log`.
- 6워커 transport: `measurements/worker6/transport-20261009T031415432Z-ffcfb773/runs.json`.
- 빌드·측정 묶음과 로그: `server-concurrency-evidence.zip`, `product-candidate-release-build.log`, `product-candidate-release-final.log`, `harness-release/build-final.log`.

기술 정의 확인: [Microsoft IOCP](https://learn.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports), [Microsoft Wait Chain Traversal](https://learn.microsoft.com/en-us/windows/win32/debug/wait-chain-traversal), [FlatBuffers](https://flatbuffers.dev/), [Protobuf](https://protobuf.dev/overview/), [Chase–Lev 원 논문](https://www.cs.wm.edu/~dcschmidt/PDF/work-stealing-dequeue.pdf). IOCP/Chase–Lev 자체를 최신 발명으로 소개하지 않는다.

## G10. Visual Studio에서 직접 이해하기 위한 구성

사용자가 diff와 vcxproj/filters 편집 없이 따라갈 수 있도록 [GUIDE](2026-10-09_SERVER_IOCP_JOB_COMPARISON_GUIDE.md)와 [기술소개서](2026-10-09_SERVER_IOCP_JOB_COMPARISON_TECHNICAL_NOTE.md)를 추가했다. GUIDE의 첫 작성 파일은 IocpService.h이며, ClientSession의 생성자와 Commit은 +/− 표시 없는 완성된 함수로 설명했다. 전체 구현 정본은 기존 PLAN을 유지한다.

최초 작업의 `out/ServerConcurrency20261009/build-view/ServerConcurrencyStudy.sln`은 build-view 소스에 Shared/Server/Harness 세 프로젝트와 변경 전 파일·문서를 등록했었다. 기본 solution Build는 Harness만 수행했고, 하네스 디버거 인자는 `--job-tests`였다. 당시 원본 Framework.sln과 제품 프로젝트는 변경하지 않았다. 이 이전 솔루션은 현재 디스크에 없다.

이전 솔루션의 x64 Release 빌드 PASS, padding C4324 경고2건·오류0은 당시 `study-solution-release.log`에 기록한 결과다. 그 로그도 현재 없으며 이번에 재실행하지 않았다. G05/G06 수치를 새 측정으로 바꾸지 않았다.

현재는 [CodeWalkthrough.sln](../../../Study/CodeWalkthrough/CodeWalkthrough.sln)을 연다. 이 프로젝트는 IOCP 설명본 H/CPP만 정적 라이브러리로 컴파일하고, 서버·프로파일러·셰이더 사본은 열람용 `None` 항목으로 제공한다. 실행 EXE·성능 하네스나 제품 IOCP 연결을 제공하지 않는다. 학습 사본의 작성·검증은 [CODE_WALKTHROUGH_RESULT](2026-10-09_CODE_WALKTHROUGH_RESULT.md)에 별도로 기록했다.
