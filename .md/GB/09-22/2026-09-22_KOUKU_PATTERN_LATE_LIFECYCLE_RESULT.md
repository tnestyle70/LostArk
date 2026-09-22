# 쿠크 Pattern 늦은 lifecycle 수신 수정 결과

## G00. 확인한 결함

`쿠크_피자`는 `KAKULSAYDON_G1_PATTERN_25`, `GATE2`, `boss.kakulsaydon.g2.kouku`다. 현재 저장 Composition revision은 2166이며 패턴 시간은 19,914ms다. Client의 기존 오류는 실제 Server 거절이 아니라 `KoukuSaydonPatternAuditionService::Update`가 5초 또는 15초 뒤 자체 생성한 ABORTED 상태였다. 이후 `Apply_Result`와 `Apply_Lifecycle`이 `Is_InFlight=false`를 보고 정확한 늦은 응답까지 버렸다.

현재 NetworkManager는 수신 FIFO를 한 Update에 최대 64 frame만 배달한다. 실제 함수와 codec으로 64개 앞선 frame 뒤에 정상 P25 결과·PENDING·ACTIVE를 넣고 25,656ms의 main pump 지연을 적용하면 이전 서비스가 요청을 폐기한다. 같은 입력에서 수정 서비스는 다음 Update의 정확한 응답으로 ACTIVE가 된다. 이 지연 수치는 사용자 session의 최대 pump 간격과 같지만 해당 로그에 Pattern 요청/결과가 없으므로 실제 사용자 클릭의 패킷 원인까지 확정했다고 기록하지 않는다.

## G01. 반영한 코드

`Client/Private/KoukuSaydonPatternAuditionService.cpp`와 대응 H에서 deadline을 지연 안내로 바꾸고 exact request/scope/epoch/revision 소유권을 보존했다. 원래 5초와 15초 값은 유지했다. 새 재생은 미해결 요청을 덮지 않으며 ACTIVE나 완료 상태는 실제 Server lifecycle만 설정한다. 늦은 거절은 실제 사유를 표시한다.

승인 전 Stop을 단일 Pattern에도 연결했다. 서버 epoch를 추측해서 보내지 않고 exact 승인 뒤 STOP을 제출한다. 늦은 한 배치에서 이미 자연 완료됐어도 그 epoch의 잔존 투사체를 중단한다. disconnect와 world generation 변경은 기존처럼 terminal이며 다른 request/Gate/boss/epoch/revision은 거부한다. Server, Shared protocol, 패턴 길이·판정 데이터와 로컬 전투 권위는 변경하지 않았다.

기존 C++ UTF-8 BOM 없음/CRLF를 보존했다. 제품 CPP 추가와 프로젝트 등록은 없다. 원본 백업·SHA와 검증 파일은 `out/KoukuPatternLifecycle20260922`에 있다.

## G02. 집중 검증

`Tools/Network/test_kouku_pattern_late_lifecycle.py`는 실제 NetworkManager FIFO·typed 소비·Shared packet codec·AuditionService 전체 소스를 연결하고 clock, socket 연결 상태와 송신만 격리한다. Client/게임 main/UI를 실행하지 않는다.

| 검사 | 결과 |
|---|---|
| 실제 raw FIFO 뒤 exact ACTIVE, queued 15초 경계 | 수정 전 실패, 수정 후 통과 |
| 늦은 ACTIVE→PATTERN_COMPLETED→COMPLETED 한 배치, 추가 ACTIVE 무시 | 수정 전 실패, 수정 후 통과 |
| 늦은 실제 source revision 거절 사유 표시 | 수정 전 실패, 수정 후 통과 |
| 다른 request/Gate/source/gameplay/epoch/boss 및 superseded 입력 | 수정 후 통과 |
| 승인 전 Stop, 한 배치 완료 뒤 retained tail Stop | 수정 전 실패, 수정 후 통과 |
| disconnect/world 변경 종료 | 전후 통과 |

수정 전 8개 중 7개 실패, 수정 후 8개 모두 통과했다. `before.log`, `fixed.log`에 보존했다. 기존 Flow probe도 현재 서비스로 혼합 Pattern/Bundle 순서, bundle 전체 완료, follow-up PENDING, wait/Stop, stale 응답, 전송 실패와 world/Gate 변경을 통과했다(`flow/run.log`). 과거 fixture의 자연 완료 뒤 Stop 기대값은 현재 retained-projectile 계약에 맞게 out 사본에서만 갱신했다. Python 구문 검사와 변경 범위 `git diff --check`가 통과했다.

## G03. 배포와 남은 확인

root의 최신 Product 빌드가 만든 Server/Shared object를 out 전용 probe에 연결해 현재 게시 bootstrap을 새로 읽었다. 실제 CGameplayCatalog source revision 2166, World/navigation/profile, Gate2 쿠크 placement 생성, 실제 Handle_KoukuSaydonPatternAudition 송신 frame의 codec 해석, 다음 고정 tick ACTIVE가 모두 통과했다. 현재 Client 단일 Play_Selected와 같은 빈 gate scope와 명시 GATE2 두 경우 모두 요청/Gate/placement/archetype/gameplay/source revision 및 epoch를 보존했다. 21개 검사 failures 0, 전체 2,623ms이며 server_run.log에 기록했다. 이 검증에는 TCP listener나 Client/UI 실행이 없다. Server 수정은 필요하지 않았다.

전체 Debug Engine/Shared/Server/Client Build와 deploy는 root 통합 작업에서 exit 0으로 완료됐다. receipt는 out/BuildPipeline/runs/20260922T011037720Z-debug-product.json이며 missing/invalid runtime input은 0이다. 발탄 돌 데이터 반영/게시는 대응 돌 RESULT를 따른다. 실제 원격 Server 갱신 여부와 사용자 화면의 피자 시작은 이 socket 없는 검사로 대신하지 않는다. 초기 out 전용 링크에서 대소문자 Main.obj 제외가 누락되어 중복 main으로 실패했으며, 해당 probe 입력만 교정한 최종 링크/실행은 성공했다.
