# Kouku Complete Play 응답과 수신 배치 결과

## G00. 확인한 원인과 범위

사용자가 보고한 2관문 전체 재생 미시작과 대형 세이튼 세 번 내려치기의 연결 종료는 서로 다른 실패다. 불뿜기 클립/이펙트의 중복 의심은 사용자가 보류했으므로 편집하지 않았다. 이전 Debug 로딩·F1 카메라·크기 Save 변경과 다른 작업의 미커밋 변경도 보존했다.

- `client-session-72676.jsonl` 444행: `CLIENT_EVENT_QUEUE_OVERFLOW`, `m_KoukuSaydonPatternAuditionLifecycleEvents depth=64 limit=64`. 445행이 사용자에게 표시된 disconnected Server session 오류다. raw queue 최고치는 1,305, main pump 최대 간격은 14,953ms였다. 이 수치를 특정 불뿜기 패턴의 발생량으로 해석하지 않는다.
- Action/Encounter revision은 1918이지만 Sequence 저작본은 154, 게시된 Server RAIDGATE는 120이었다. 실제 Server GameRoom에 154 START 요청을 제출하면 `epoch 0 / ABORTED`, `Published raid flow or Sequence revision is missing or stale`로 거절됐다. 같은 게시본의 120 대조 요청은 Gate2 `PREPARING -> READY -> CINEMATIC`에 도달했다.
- MainApp은 5초가 지나면 pending request ID를 삭제하여 늦은 거절을 잃었다. 별도로 ClientReplication의 단일 latest reply를 같은 배치의 후속 방송이 덮는 경계도 확인했다.

진단 원본 복사본과 SHA-256은 `out/KoukuCompletePlayReceive20260921/diagnostic-snapshot.json`, 게시 전 실제 Server 검증은 `out/CompletePlayAdmission20260921/result.json`과 `run.log`에 있다. 제품 Client/Server를 실행하거나 화면을 조작하지 않았다.

## G01. 수신 처리

`NetworkManager.h/.cpp`에서 한 Update가 최대 64개 raw frame을 처리한 뒤 기존 typed 소비자에게 제어를 반환한다. 다음 frame의 목적지 queue가 차면 해당 frame과 뒤의 입력을 원래 raw FIFO에 남긴다. `Handle_Frame`은 mutex 밖에서 호출하므로 기존 Close/Fail 경계와 충돌하지 않는다.

worker의 기존 raw enqueue/coalescing 코드를 private `Enqueue_InboundFrame`으로 옮겨 실제 실행 경로를 검증한다. reliable 입력의 폐기, lifecycle coalescing, queue 한도 확대는 하지 않았다. ENTER_ACCEPTED 뒤 같은 배치의 새 world spawn/snapshot을 보존하고 종료·실패 때는 원래 전체 정리를 따른다.

## G02. 요청 응답

`MainApp.cpp/.h`에서 timeout은 한 번 알리고 exact pending request를 계속 추적한다. 최종 거절 사유는 일반 raid presentation 상태와 별도로 보존한다. pending 중 새 START를 막고, 취소 후 늦은 승인에는 서버가 확인한 epoch에 STOP을 보낸다. world/session 변경 시 관련 준비·요청·응답을 정리한다. 요청 송신·timeout·최종 응답은 기존 session 진단에 기록한다.

`ClientReplication.h/.cpp`의 기존 reply 저장소는 현재 요청 하나의 첫 응답을 보존한다. `MainApp -> Level_KakulSaydonArena -> Expect_KoukuRaidReply`로 성공한 START/STOP의 request ID와 world generation을 등록한다. 같은 ID라도 다른 owner의 승인 방송은 보관하지 않으며 일반 raid state는 계속 갱신한다. MainApp이 응답을 값으로 복사한 뒤 slot을 비운다. world reset도 등록을 지운다. 대기 중 취소는 보관한 응답의 epoch·owner가 현재 상태와 일치할 때만 STOP한다.

## G03. 현재 저장본 게시

다음 공식 owner publish는 exit 0으로 완료됐다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1918
```

koukusaydon.product, map.kakulsaydon, world.gameplay, gameplay.balance가 모두 PASS했다. 로그는 `out/KoukuCompletePlayReceive20260921/publish.log`다.

- Action 저작본과 Sequence 저작본은 게시 전후 바이트 SHA-256이 같다. `before-publish.json`과 `after-publish.json`에 기록했다.
- Encounter의 네 관문 Sequence revision과 Server RAIDGATE가 모두 154로 일치한다. Gate1 연출 길이는 현재 저장본에 따라 62,919ms에서 61,662ms로 생성됐다.
- patternbindings는 JSON 내용이 동일하다. Gameplay.bootstrap에는 RAIDGATE 갱신 외에 재생성에 따른 일부 부동소수점 마지막 자리 차이가 있다. source pattern이나 clip은 편집하지 않았다.
- 게시 전 28개 실제 Server 검증은 모두 통과했다. 기존 154 요청의 거절 재현, 120 대조 요청의 연출 진입, 대형 세이튼 3연 내려치기 `PATTERN_87`의 실제 placement 생성·QUEUED admission·pending 소비 후 패턴 시작을 확인했다.
- 게시 후 22개 검증도 모두 통과했다. 최신 154 요청으로 실제 Gate2 `START -> PREPARING -> READY(mask=1) -> CINEMATIC(PATTERN_3, startTick=22)`과 P87 패턴 시작을 확인했다. `out/CompletePlayAdmission20260921/after-publish/result.json`과 `run.log`에 기록했다. 현재 제품 Server/Shared 산출물을 실제 GameRoom과 함께 링크한 격리 검증이며 TCP listener는 열지 않았다.

## G04. 검증과 남은 경계

| 검증 | 실제 결과 |
|---|---|
| NetworkManager 실제 TU | Debug/Release 격리 `/c` PASS |
| 최종 MainApp·ClientReplication·Level_KakulSaydonArena 실제 TU | 3개 TU 각각 Debug/Release 격리 컴파일 PASS |
| 실제 수신 함수·현재 packet codec 회귀 | Debug 8/8, Release 8/8 PASS. 동일 fixture의 기존 Update는 6/8 FAIL이며 실제 lifecycle queue overflow 재현 |
| 최종 MainApp + ClientReplication mailbox 회귀 | Debug 41/41, Release 27/27 PASS. 원본은 각각 30개·19개 실패; timeout만 수정한 중간본도 각각 11개·8개 실패 |
| 독립 코드 검토 | mailbox 응답 소실 지적 반영 후 추가 결함 없음 |
| Product Debug | Engine/Shared/Server/Client 정상 Build와 배포 PASS, exit 0. Client 68개 OBJ 재컴파일 및 링크 |
| 변경 JSON/XML parse | 7개 PASS |
| `git diff --check` | PASS |

수신 회귀는 `Tools/Network/test_client_receive_dispatch.py`로 재현한다. 증거는 `out/KoukuReceive20260921/{debug-test,release-test,baseline-test}.log`다. 실제 FIFO, queue capacity, close/reset, snapshot coalescing과 entry 뒤 같은 배치 spawn/snapshot을 실행한다. 표시 파일시스템 준비는 이 회귀에서 격리했다.

mailbox 회귀는 실제 관련 MainApp 함수, ClientReplication의 raid case·Expect·reset 내용을 추출하고 시간과 Engine/assets/socket 협력자를 stub으로 연결했다. 두 participant의 같은 request 번호, 한 배치의 거절 뒤 다른 owner 방송, late approval, queued STOP, world 변경을 검증한다. 최종 증거는 `out/KoukuCompletePlayReceive20260921/mainapp/`에 있다. 제품 화면 검증을 대신하지 않는다.

Product 명령은 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -BuildLogDirectory out/KoukuCompletePlayReceive20260921/product`다. 결과 receipt는 `out/BuildPipeline/runs/20260920T182407063Z-debug-product.json`, 로그는 `out/KoukuCompletePlayReceive20260921/product-build.log`다. 기존 소스 문자 집합 경고는 남아 있고 컴파일 오류는 없다. 실행 파일과 소스 hash는 `product-final-files.json`에 기록했다. Release 실행 파일 전체 빌드는 수행하지 않았다.

Client 화면의 Gate2 Complete Play와 대형 세이튼 세 번 내려치기는 사용자가 새 Debug Server/Client에서 직접 확인해야 한다. 파일 게시·서버 내부 검증을 Client 화면·GPU·음성 재생 완료로 기록하지 않는다.
