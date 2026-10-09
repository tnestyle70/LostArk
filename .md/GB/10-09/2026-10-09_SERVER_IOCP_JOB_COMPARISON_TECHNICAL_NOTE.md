# LostArk 서버의 작업 분배·IOCP 비교와 도입 판단

2026-10-09 · 비교 후보의 구현·검증 기록 · 제품 기본 경로는 기존 구현 유지

## 1. 문제와 목표

기존 LostArk 서버는 연결마다 수신·송신 스레드를 만들고, 하나의 30Hz room thread가 게임 상태를 갱신한다. 연결 수가 늘 때의 스레드 비용과 여러 세션에 같은 snapshot을 전달하는 비용을 따로 확인하고자 했다.

서버 권위와 패킷 계약을 유지하면서 전송 방식은 `select / IOCP`, snapshot 송신 준비는 `serial / JobSystem`으로 선택하는 비교 후보를 구성했다. 기술 도입 여부는 구현 가능성만으로 결정하지 않고 동일한 작업의 지연·처리량·CPU 비용·정확성을 측정하여 판단했다.

이번 구현은 제품과 분리된 소스 복사본에서 컴파일·실험했다. 제품 Server/Shared 코드는 변경하지 않았으며 실제 게임에서의 성능 개선을 주장하는 자료가 아니다.

## 2. 설계 범위

```text
기존 수신 스레드 또는 IOCP 완료
  → 동일한 패킷 파서와 ServerApp 라우팅
  → Room 명령 큐
  → 기존 단일 30Hz 게임 상태 갱신
  → Snapshot 직렬화 1회
  → 직렬 또는 JobSystem의 세션별 frame 작성·송신 큐 등록
  → 기존 송신 스레드 또는 overlapped 송신
```

게임 상태 변경을 worker로 옮기지 않았다. CPU 작업 분배의 소비자는 snapshot fanout으로 한정했다. owner가 고정한 payload와 세션 목록을 작업 완료까지 유지하고, 각 job은 자기 세션의 송신 큐와 자기 결과만 변경한다. owner는 작업을 join한 뒤 오류와 통계를 처리한다.

IOCP 경로의 송신 준비는 최초 `WSASend` 비동기 게시도 포함한다. 따라서 이 측정은 순수 메모리 복사나 deque 단일 연산의 microbenchmark가 아니다. 작업을 게시하는 스레드가 실제 송신 완료를 기다리지는 않는다.

## 3. JobSystem 구현과 검증 대상

JobSystem은 다음 요소로 구성했다.

- worker별 고정 크기 Chase–Lev deque: owner의 bottom push/pop과 다른 worker의 top steal.
- 외부 제출용 bounded injection queue: 서버 room thread와 같은 외부 호출자의 작업 수용.
- JobCounter와 `Submit / Wait`: 작업 완료 추적, 대기 중 다른 작업을 실행하는 caller helping.
- 큐 포화 시 caller-runs, 작업 예외의 완료 후 전달, 협력 종료와 accepted work drain.
- C++20 atomic wait/notify를 사용하는 worker 대기.

전체 scheduler는 mutex를 포함하므로 전체를 lock-free라고 표현하지 않는다. 이번 후보에는 fiber를 추가하지 않았다.

정확성 검사에서 마지막 deque 원소에 대한 owner/thief 경합 20,000회, exactly-once 10,000개 작업, 중첩 대기, 외부 동시 제출, 큐 포화, 예외와 종료 시 수명 검사를 통과했다. 이는 실행한 검사 범위의 결과이며 모든 가능한 스레드 순서에 대한 무결성·무교착 증명은 아니다.

## 4. 비교 방법

환경은 AMD Ryzen AI 5 340, 논리 프로세서 12개, Windows 10.0.26200, MSVC v143 x64 Release다. 실제 실행 파일 hash와 원시 결과는 [검증 RESULT](2026-10-09_SERVER_IOCP_JOB_COMPARISON_RESULT.md)에 연결했다.

| 비교 | 고정한 조건 | 바꾼 조건 | 주요 지표 |
|---|---|---|---|
| 전송 | loopback, 256-byte payload, 연결당 window 8, 동일 frame codec | select / IOCP 2·6워커, 연결 4·32·128 | frame/s, RTT p95/p99, 서버 CPU/유효 frame, 관측 thread 수 |
| 송신 준비 | 동일 fanout 함수, payload 1KiB·16KiB, 연결 4·32·128, batch마다 전체 수신 확인 | 같은 transport에서 serial / JobSystem 2워커 | enqueue p95, 전체 수신 완료 p95, 실패·누락·coalescing |

전송은 warmup 1초 뒤 3초 동안 측정하고, fanout은 warmup 100batch 뒤 1,000batch를 측정했다. 각 조건을 A–B–B–A 순서로 두 번 반복하여 방식별 4trial을 얻었다. 아래 percentile은 각 trial에서 구한 percentile의 중앙값이다. 모든 sample을 합친 percentile이나 통계적 유의성 검정 결과가 아니다.

전송은 별도 서버·클라이언트 프로세스이고 CPU는 서버만 측정한다. fanout은 송수신이 같은 프로세스이므로 CPU는 합산 값이다. 일반 데스크톱 환경이며 CPU affinity·전력 정책·모든 background 부하를 통제한 전용 시험은 아니다.

## 5. 결과와 비채택 판단

### JobSystem: 조건별 개선과 회귀

| transport | 연결·payload | serial enqueue p95 | jobs enqueue p95 | 해석 |
|---|---|---:|---:|---|
| select | 4·1KiB | 12.40μs | 16.95μs | 작은 작업에서는 분배·완료 대기를 포함한 총비용 증가 |
| select | 4·16KiB | 58.20μs | 55.85μs | enqueue는 소폭 감소하지만 전체 수신 p95는 114.70→117.00μs |
| select | 128·16KiB | 805.55μs | 1,039.55μs | 연결 수 증가만으로 병렬화 이득을 보장하지 않음 |
| IOCP | 4·1KiB | 31.45μs | 29.65μs | 일부 작은 부하에서도 측정 중앙값은 개선 |
| IOCP | 128·1KiB | 1,031.95μs | 534.35μs | 이 합성 조건에서는 송신 준비 지연 감소 |

**현재 제품의 기본 select 경로와 4인 규모를 모사한 부하에서 일관된 채택 근거를 확보하지 못해, 제품 기본 실행에는 JobSystem을 채택하지 않았다.** “모든 조건에서 효과가 없었다”는 결론은 내리지 않는다. 작업 분배로 줄어드는 실행 시간과 제출·wakeup·동기화·join 비용을 함께 평가해야 한다.

성능 측정의 JobSystem 48trial에서 제출된 작업 2,624,000개는 모두 외부 injection queue를 사용했다. `localPushes=0`, `steals=0`이었다. 따라서 **Chase–Lev를 포함하는 scheduler를 구현했지만, 이번 fanout 성능 수치는 Chase–Lev steal 자체의 속도를 측정한 결과가 아니다.** deque 경합 정확성 검사와 scheduler 성능 비교를 분리하여 설명한다.

### IOCP: thread 수 감소와 처리 비용의 교환

같은 6워커 비교 배치의 128연결 결과는 다음과 같다.

| 지표 | select | IOCP 6워커 |
|---|---:|---:|
| 처리량 | 659,163 frame/s | 268,272 frame/s |
| RTT p95 | 1.0750ms | 3.9624ms |
| 서버 CPU ms/1,000 유효 frame | 9.09 | 18.89 |
| 관측 최대 서버 thread | 261 | 11 |

이 후보는 연결 수에 따른 thread 증가를 억제했지만, 해당 부하에서 처리량과 frame당 CPU 효율이 낮아졌다. 기본 transport도 select를 유지했다. 이 결과를 IOCP 일반의 우열로 확대하지 않는다. operation 할당, 완료 처리와 계측, worker 수, posting 경계는 다음 분석 후보이며 개별 원인은 CPU profile 없이 확정하지 않았다.

## 6. 검증 범위와 후속 판단

비교 후보 Server/Shared 전체 Release 빌드, 전송 correctness 4실행, 전송 성능 48실행, fanout 성능 96실행을 완료했다. 정확성 실행마다 실제 통신한 46개 세션의 accepted/closed 수가 일치했고 IOCP pending은 종료 후 0이었다. 성능 실행의 frame 개수·checksum·순서와 queue 유효성 검사를 통과했다.

실제 IOCP partial-send completion은 이번 정확성 실행에서 0이었다. 큰 frame의 FIFO 송신 성공을 부분 완료 분기의 실제 재현으로 표현하지 않는다. 별도의 lock-order graph 기반 deadlock profiler도 구현하지 않았다.

다음 채택 판단에는 실제 4인 packet 크기·빈도와 동시 room 수, RoomPerf의 tick 지연, 고정 유입률 부하를 사용해야 한다. 현재 loopback 합성 결과를 실제 레이드 tick·LAN 수용량·FPS 향상으로 환산하지 않는다.

## 7. 면접에서의 설명

> 기존 서버의 권위 구조를 유지하면서 snapshot fanout에 JobSystem 비교 구현을 연결했습니다. Chase–Lev deque와 외부 제출 큐, 완료 counter를 구현하고 정확성을 별도로 검증했습니다. 기존 select의 4연결·1KiB 조건에서는 송신 준비 p95가 12.40에서 16.95μs로 늘었고, 다른 조건에서는 개선도 나타났습니다. 현재 제품의 목표 부하에서 일관된 순이득을 확인하지 못해 기본 실행에는 채택하지 않았습니다. 실제 fanout은 외부 injection 경로를 사용했으므로 이 결과를 Chase–Lev stealing 자체의 성능 성과로 소개하지는 않습니다.

“왜 적용하지 않았나?”에는 구현이 실패해서가 아니라, 정확성을 확보한 비교 후보를 측정한 뒤 현재 제품에 대한 채택 근거가 부족했다고 설명한다. “통계적으로 유의미하지 않았다”는 표현은 통계 검정을 실시하지 않았으므로 사용하지 않는다.

실제 코드와 직접 반영 순서는 [GUIDE](2026-10-09_SERVER_IOCP_JOB_COMPARISON_GUIDE.md), 전체 구현은 [PLAN](2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md), 전체 수치와 실행 근거는 [RESULT](2026-10-09_SERVER_IOCP_JOB_COMPARISON_RESULT.md)에 보관한다.
