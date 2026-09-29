# Release 전환 리소스 준비 재사용과 Server 송신 큐 확대 결과

## G00. 적용 범위와 상태

2026-09-29, `codex/kouku-release-sequence-ready`, 시작 HEAD `d6a9cc2240257779c285110bac9b73714349291a` 기준이다. Release 제품 빌드와 마지막 Client 추가 컴파일·링크를 완료했다. Client/UI 및 파티 플레이를 에이전트가 실행하지 않았다.

최종 상태 확인 중 별도 편집으로 `Data/Effects/Authored/effect.artist.skill.31950.full.restore.effect.json` 변경이 나타났다. 이번 작업의 수정·검증 범위에 포함하지 않고 그대로 보존했다.

Release에서 쿠크 첫 트리거와 발탄 Complete Play 직전에 이미 준비한 리소스를 다시 준비하며 프레임을 기다리던 경로를 수정했다. 필요한 초기 로딩, Server 승인·참가자 READY·시작 tick 및 revision 검사는 유지한다. 실행 직전 중복 준비 제거와 실제 화면상의 0ms 시작은 같은 보장이 아니다.

## G01. 쿠크 입장 준비와 첫 트리거

`Client/Private/Level_KakulSaydonArena.cpp`는 기존 Complete Play 본문을 공통 함수로 사용한다. Release Initialize에서 게시 inventory의 전체 패턴과 source revision으로 actor/V2/WORLD, binding과 clone pool을 준비한다. Loader가 이미 로드한 WORLD를 다시 Load_Area하지 않으며, V1은 실제 catalog current·settled·prepared·실패 상태를 확인한다. 호출 수는 dependency 개수로 제한하고 worker 완료를 기다리는 무한 루프를 만들지 않는다.

완료된 준비 상태와 숨김 G1 책·환경을 첫 Server PREPARING에서 재사용한다. 현재 게시 입력은 source revision 2468, PlayAll 114개, WORLD revision 2285, enabled instance 344개였으며 네 pool의 크기는 card 6, joker 1, circus ball 4, odd doll 4다. 이 값은 조사 시점 입력이며 코드에 고정하지 않았다.

Debug 저작과 명시적인 준비 reset/reload는 기존 갱신 절차를 유지한다. `Level_KakulSaydonArena_WorldObjects.cpp`에서 성공한 명시 WORLD reload는 아직 사용하지 않은 G1 준비 객체와 조명을 함께 폐기한다. 실패한 로드·지연된 reload·진행 중인 cinematic 상태는 유지한다.

## G02. 발탄과 캐릭터 선택

`Client/Private/Level_ValtanArena.cpp`의 Complete Play에서 Release는 준비된 V2/WORLD 목록을 같은 호출 안에서 확인한다. Debug는 기존 리소스당 한 프레임 진행을 유지한다. generation 불일치, V1 미완료 및 V1/V2/WORLD 실패 검사는 그대로다. F1 요청을 다음 Update에서 처리하는 경계와 Server 시작 승인은 유지된다.

Character Select는 현재 `CHARACTER_SELECT_CLASSES`의 7개 직업 모델·장비·애니메이션, 전체 roster의 skill Effect와 intro/loop WORLD를 이미 선로딩한다. 실제 교체 소비자가 준비 결과를 사용하는 것을 확인했으며 해당 코드는 수정하지 않았다. 기존 09-24 Release Character Select preload 결과의 후속 확인이다. 다른 gameplay map의 원격 class lazy-load 정책은 변경하지 않았다.

## G03. 카드 회전 패턴의 두 연결 종료

`Server/Bin/Release/Diagnostics/server-session-36460.jsonl` 2~3행에서 session 3/player 2와 session 4/player 3이 2026-09-29 05:54:57.876 KST에 동시에 종료됐다. 두 연결 모두 `SERVER_RELIABLE_OUTBOUND_OVERFLOW`, `queuedFramesAtClose=128`, `queuedBytesAtClose=20118`, `reliableRejected=1`, `sendFailures=0`이었다. `server-room-perf-36460.log` 21행은 `P48 세이튼_빙글빙글돌며카드던지기`에서 참가자 이탈로 중단된 흐름을 연결한다.

이는 Lobby 승인 5초 timeout이 아니라 작은 reliable event가 송신 큐의 개수 상한을 채운 사례다. 당시 최대 send 소요는 약 2.76초였다. 원격 Client가 읽기를 멈춘 원인까지 확인한 것은 아니다.

사용자 요청에 따라 `Server/Public/ClientSession.h`의 연결당 큐 한도를 128 → 4096 frame, 512 KiB → 8 MiB로 확대했다. reliable FIFO, 최신 snapshot 병합, reliable reserve와 한도 초과 시 해당 연결 종료는 유지한다. 네 연결이 바이트 상한을 모두 사용하면 payload만 최대 32 MiB이며 별도 metadata가 필요하다. 저장 공간은 큐 사용량에 따라 증가한다. 무제한 큐나 송신 timeout 연장은 적용하지 않았다.

## G04. 실행한 검증

| 검증 | 결과와 근거 |
|---|---|
| Release Product build | `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release` PASS. Engine/Shared/Server/Client 및 배포 성공, 약 103초. Receipt: `out/BuildPipeline/runs/20260928T211206870Z-release-product.json` |
| 마지막 WORLD reload 수정 | 같은 MSBuild·기존 x64 Release 출력 경로로 Client 증분 Build 및 링크 PASS. `out/KoukuReleaseSequenceReady20260929/client-release-final.log` |
| 제품 runtime 입력 | Product receipt의 41개 검사 PASS, missing/invalid runtime input 모두 0 |
| Server queue | 실제 ClientSession 및 기존 계약 테스트 TU Release 컴파일 PASS. 실제 큐를 사용하는 FIFO/snapshot, 512-event burst, 8 MiB byte overflow, frame overflow/reserve 네 사례 PASS. `out/KoukuTransportBurst20260929/receipt.json`, `queue-probe.run.log` |
| 변경 전 queue 비교 | 원래 128 frame/512 KiB 상한으로 같은 사례 실행 시 512-event burst만 실패. `out/KoukuTransportBurst20260929/baseline-receipt.json`, `baseline-probe.run.log` |
| 쿠크 준비 진행 | 실제 세 함수 본문을 추출한 CPU 검사 21/21 PASS. 첫 준비·재사용·4개 pool·G1 stage·revision/실패/호출 상한·명시 reset 등을 확인. `out/KoukuReleasePrewarmReview20260929/probe.log`, `manifest.json` |
| 발탄 준비 진행 | 실제 함수 진행·실패 검사 구간을 추출한 Release/Debug 검사 모두 failures=0. `out/KoukuReleaseSequence20260929/valtan-policy/policy-results.log` |
| 독립 코드 검토 | 쿠크 초기 준비와 성공한 WORLD reload의 G1 정리 경계를 검토했으며 추가 P1/P2 발견 없음 |
| 변경 형식 | C++ 기존 인코딩과 줄바꿈 유지. 새 C++/project/filter/JSON/XML 변경 없음. `git diff --check` PASS |

CPU 추출 검사는 실제 함수 본문과 통제된 협력 객체를 사용하므로 GPU·실제 parser·네트워크·Client 화면의 성공을 뜻하지 않는다. 전체 Server gameplay contract suite를 실행한 것으로도 기록하지 않는다. 빌드의 기존 코드 페이지·DirectXTK PDB 경고는 남아 있으며 컴파일/링크 오류는 없었다.

## G05. 사용자 실행 확인 경계

출력은 `Client/Bin/Release/Client.exe`와 `Server/Bin/Release/Server.exe`다. 작업 중 사용자가 실행한 Debug Client/Server는 유지했으며 변경된 Release 프로그램을 자동 실행하거나 실행 중 프로세스에 반영하지 않았다. Data publisher와 rendering option 변경은 없다.

Release Server와 각 PC의 Release Client로 쿠크 입장 후 첫 G1 트리거, 중단·재입장, 발탄 Complete Play, 캐릭터 교체 및 카드 회전 패턴의 다인 플레이를 확인해야 한다. 화면 시작 지연과 원격 Client 정체 해소는 아직 미확인이다. 확대된 큐는 일시 정체 허용량을 늘리지만 무기한 정체의 원인을 해결했다는 보장은 아니다.
