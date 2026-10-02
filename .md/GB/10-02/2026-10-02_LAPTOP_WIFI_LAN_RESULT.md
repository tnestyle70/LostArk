# 노트북 Wi-Fi Server endpoint 반영 결과

2026-10-03 PR 통합 확인: Guide/Nav/Movie 후속 Debug Product는 2026-10-02 17:40 KST에
완료됐다. 근거는 `out/BuildPipeline/runs/20261002T084006819Z-debug-product.json` 및 외부
`Build-Debug-GuideNavMovie-Exit.json`이다. 아래 마지막 절의 Debug 대기 상태는 이 영수증으로
해소됐고 Release Product는 아직 실행 중이다. 기존 queue의 HEAD·입력 검증을 보존하기 위해
별도 worktree에 변경본을 복사해 PR을 준비했으며 원래 빌드 작업 폴더는 변경하지 않았다.
endpoint의 activeThroughKst는 2026-10-02 23:59:59 KST로 만료됐다. 이번 PR은 기존 변경의
Git 전달이며 기간 연장이나 현재 LAN 접속 성공을 뜻하지 않는다. 만료 우회 없이 다음 사용 전에
endpoint 정본과 public 계약을 함께 갱신해야 한다.

## G00. 소스·설정 반영

최종 재연결 endpoint는 `192.168.0.14:7777`이다. 아래 최초 `192.168.200.142` 적용은
같은 작업에서 이동 전 확인한 기록이며, G07의 후속 갱신이 현재 정본이다.

사용자가 노트북 Wi-Fi 기준 Server/Client 실행을 명시적으로 요청했다. 실제 `Wi-Fi`
주소 `192.168.200.142/24`가 Preferred 상태임을 확인하고 endpoint 정본, Client의
직접 실행 fallback과 공유 x64 Debug/Release debugger 값, 새 portable 생성기의 HOST와
fixture·실행 안내를 `192.168.200.142:7777`로 맞췄다. 임시 계약은
`2026-10-02T23:59:59+09:00`까지다. Server bind는 기존 `0.0.0.0:7777`이다.

AGENTS/CLAUDE와 현재 팀 사용서의 날짜·주소·노트북 Wi-Fi 소유 설명을 함께 갱신했다.
`Framework.slnLaunch`에는 주소 literal이 없으므로 기존 Server + Client profile을 유지했다.
새 C++ 파일과 project/filter 등록은 없다. NetworkManager.cpp는 주소 상수 한 줄만 변경했고
UTF-8(BOM 없음), CRLF와 나머지 소스를 보존했다.

10-01 PLAN/RESULT, Release/zipRelease.md와 기존 ZIP의 주소·경로·hash 기록은 보존했다.
새 portable ZIP은 생성하지 않았다. Data/DataFiles, Resources, 렌더링 설정, toolset은
이번 endpoint 변경에서 수정하지 않았다.

## G01. 실행한 자동 검증

- 번들 Python으로 `Tools/Network/test_team_lan_endpoint_contract.py`: 3 tests, exit 0.
- Endpoint JSON 전체 값과 현재 만료 전 상태 확인.
- Client/Server project 및 filters XML 4개 parse, Client x64 Debug/Release 환경값 2개 일치.
- Framework.slnLaunch JSON parse, Server + Client profile의 기존 두 project 경로 확인.
- C++ diff는 기존 HEAD 대비 endpoint literal만 변경. UTF-8/BOM/CRLF 확인.
- Packaging Python AST parse와 builder HOST·manifest fixture 일치 확인.
  잘못된 endpoint를 거부하는 기존 `192.168.0.14` fixture는 유지했다.
- 10-01 endpoint PLAN/RESULT와 기존 Release ZIP 문서 내용 보존 확인.
- 현재 public 문서의 옛 주소 잔여는 직전 배포 기록 두 곳뿐이다. 현재 만료일은 모두 10-02,
  만료 뒤 첫 세션 안내는 10-03으로 맞췄다.
- `git diff --check`: PASS.

상세 설정 검증은 저장소 밖
`C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Wifi-Endpoint-Verification.json`에 기록했다.
이 검사는 게임/launcher를 시작하지 않았다.

## G02. 개인 설정과 LAN 방화벽

현재 개인 Client/Server 설정을 저장소 밖 `LostArkTransfer/Sync-20261002/Recovery-20261002-1315`에
보존한 뒤 `Sync-TeamLanEndpoint.ps1 -EndpointMode Team -Role Server`를 실행했다.
첫 비관리자 실행은 debugger 설정 반영 뒤 방화벽 권한으로 실패했다. 후속 관리자 실행은
2026-10-02 13:26 KST에 exit 0으로 끝났으며 `server-host`, Client `192.168.200.142:7777`,
Server bind `0.0.0.0:7777`, TCP 7777 LocalSubnet 방화벽 ready를 확인했다.
당시 Server는 아직 시작하지 않아 probe는 `not-listening`이었다. `-AllowExpired`는 사용하지 않았다.
로그는 위 외부 디렉터리의 `Wifi-Lan-Admin-Sync.log`와 `Wifi-Lan-Admin-Sync-Exit.json`이다.

## G03. 삭제된 Resources 복구

사용자가 삭제한 뒤 비어 있던 `Client/Bin/Resources`에 휴지통의 Map/Sound/UI 실물
57,598개·19,439,764,717 bytes를 같은 볼륨에서 복원했다. 휴지통 메타데이터는 외부 백업에 보존했다.
ZIP 중앙목록 대조로 Map 안의 939개까지 포함한 누락 16,713개·14,656,456,196 bytes를 확정하고,
보존된 `Downloads/Resources.zip`에서 `C:/LARecovery-20261002`로 CRC 검사하며 추출했다.
기존 파일을 덮어쓰지 않는 이동으로 설치한 뒤 전체 74,311개·34,096,220,913 bytes의 파일명,
크기, ZIP 시각과 일곱 최상위 폴더를 검증했다. Resources 중첩과 reparse 항목은 없고 정책 검사는
Git 추적 0개로 통과했다. 원본 SHA 대조나 게임 화면 검증을 뜻하지 않는다.
ZIP과 이전 검증·백업은 보존했고 게시 DataFiles와 렌더링 설정은 변경하지 않았다.

## G04. 빌드와 실행 준비

VS18 Insiders에 version-specific `Microsoft.VisualStudio.Component.VC.14.44.17.14.x86.x64`
추가 설치가 exit 0으로 끝났고 v143 Toolset.props/targets 등록을 확인했다. 기존 MSB8020 실패는
등록 누락 때문이었다. 표준 Product Debug Build를 `-MaxCompilerProcesses 4`로 실행해
완료했다(G09). Release Build와 Release Server/Client 실행은 아직 수행하지 않았다.
다른 PC의 LAN 접속과 사용자의 최종 게임 화면은 별도 확인 항목이다.

## G05. Effect 사전 검사에서 확인한 기존 불일치

Python v15 history 검사는 `playbackClampSeconds == 마지막 sample 시각`을 거부했지만,
기존 C++ codec/corpus와 생성기는 닫힌 시간 구간의 같은 끝점을 허용한다. 검사기를 C++의
첫 sample 1e-6, 끝 sample와 sourceEnd 5e-5 및 clamp 구간 조건과 일치시키고 정상 끝점,
허용 오차, 잘못된 구간 회귀를 추가했다. Effect 검사 단위 테스트 56개가 통과했다.
C++와 Data/Resources는 이 교정에서 변경하지 않았다.

전체 Effect 검증은 이후 `EffectAuditionCatalog.json`의 기존 후보 4개가 가진 오래된 source
pin에서 실패했다. mirror-particle-canary와 DimensionMaster BA1/BA2/BA3의 원본 `localSpace`
변경 뒤 pin이 갱신되지 않은 상태이며, 현재 JSON과 catalog는 HEAD에 이미 같은 내용으로 존재한다.
줄바꿈만의 차이가 아니므로 pin 재해시나 데이터 재게시로 freshness 검사를 우회하지 않았다.
Product loader와 audition registry는 분리되어 있고 C++는 해당 후보를 freshness-lock한다.
전체 Effect 검증은 FAIL로 남기며, 후보 재승인은 별도 저작 검토 범위다.

## G06. 최신 main 대조와 로컬 빌드 연속 실행

작업 중 `git fetch origin`으로 확인한 최신 main은 `a0ff0185cc7fea171835cde15777c0f520d9d0ae`다.
기준 `c44e8a6666ea9b4592ebf4873287444facc8db92`보다 병합 커밋 4개가 앞서지만 두 tree는
`4c4e20d25e5bd1e8ebd4fdf7561325bde667ee1b`로 같고 파일 차이는 0개다.
현재 노트북용 변경은 별도 `codex/laptop-wifi-lan-20261002` 브랜치의 미커밋 변경으로 유지했다.

사용자가 이동 중 Wi-Fi 연결 종료를 문의하여 Debug 성공 receipt를 기다렸다가 표준 Release
Product Build를 시작하는 로컬 PowerShell 대기 프로세스를 준비했다. 실패 시 Release를 시작하지
않고 로그를 남기며 다른 빌드와 중복 실행하지 않는다. 이 프로세스는 게임을 실행하지 않는다.
현재 덮개 닫기 동작은 AC/DC 모두 절전이므로 노트북 종료·절전 중 빌드가 지속된다고 안내하지 않았다.

## G07. 이동 후 Wi-Fi 재연결

사용자가 재연결 상태 확인을 요청하여 실제 Wi-Fi Up/Preferred 주소 `192.168.0.14/24`를 확인했다.
사용자가 선택한 노트북 Wi-Fi 기준을 유지해 endpoint JSON, Client fallback·Debug/Release 설정,
packager와 활성 문서/PLAN을 새 주소로 갱신했다. 만료는 오늘 23:59:59 KST, bind는 0.0.0.0 그대로다.
기존 부정 fixture 주소가 새 실주소와 같아져 직전 `192.168.200.142:7777`를 부정 사례로 사용했다.
역사 ZIP 기록은 보존했다. endpoint 테스트 3개·구조 검증 7개와 diff check가 통과했다.

현재 Debug Client는 FXC 단계였고 Client OBJ와 cl 프로세스가 없음을 확인한 뒤 NetworkManager.cpp의
주소 상수를 먼저 갱신했다. 이후 `Sync-TeamLanEndpoint.ps1 -EndpointMode Team -Role Server`는
exit 0, server-host, 새 Client endpoint와 TCP 7777 LocalSubnet ready를 반환했다.
Server 실행 전 probe는 계속 not-listening이다. 후속 로그는 외부 백업의
`Wifi-Reconnected-192-168-0-14-Sync.log`와 `Wifi-Endpoint-Reconnected-Verification.json`이다.

## G08. Debug 실행 확인 우선과 포트폴리오 조사

사용자가 Debug 완료 직후 Server/Client 실행 확인을 우선 요청했다. 기존 Release 대기
프로세스는 아직 Release를 시작하지 않은 상태임을 확인한 뒤 종료했고, 현재 Debug 빌드는
그대로 유지했다. 후속 `Continue-Release-After-DebugReview.ps1`은 Debug 성공 및 실제 launch
receipt를 기다린 다음 사용자가 Debug Client와 Server를 모두 종료한 뒤 표준 Release Product
빌드를 시작한다. ProductOutputGuard를 우회하거나 사용자 게임 프로세스를 자동 종료하지 않는다.

외부 복구 로그 디렉터리에 `Portfolio-Tools-Inventory.json`(29개 항목),
`Portfolio-EngineClient-Inventory.json`(19개 항목), `Portfolio-ServerGameplay-Inventory.json`
(11개 항목)을 준비했다. 현재 코드의 함수·소비자·저장 경계와 근거 파일/줄을 조사했으며,
이 목록은 팀 구현의 정적 조사다. 개인 기여도, 실제 도구 화면 성공, 플레이 검증, 성능 수치를
뜻하지 않는다. 이력서 문구는 담당 범위와 실제 시연 근거를 추가 확인한 뒤 작성한다.

## G09. Debug 전체 Product 빌드 완료

2026-10-02 14:45:55 KST에 표준 Debug Product Build가 exit 0으로 완료됐다.
Engine, Shared, Server, Client 네 프로젝트가 모두 PASS다. 실제 object 작성 수는 각각
79, 9, 101, 361개다. Engine의 셰이더 30개와 Client 등록 셰이더 224개를 컴파일했고,
Engine 셰이더 배포를 포함한 Client 출력 CSO는 254개다. Client 런타임 DLL 6개도 배포됐다.
강제 Clean/Rebuild가 아니라 기존 표준 `/t:Build` 경로로 필요한 전체 제품 출력을 만든 결과다.

`out/BuildPipeline/runs/20261002T054555671Z-debug-product.json`은 configuration Debug,
profile Product, result PASS, skippedBuild false, missingRuntimeInputs/invalidRuntimeInputs
빈 배열과 runtimeDataChecks 42개 PASS를 기록한다. 이 과정에서 publisher나 navigation bake는
실행하지 않았다. C4819·기존 shader 경고와 DirectXTK PDB 관련 LNK4099 경고는 남았다.

## G10. ShipNpc 정보 출력 제거와 초기 DLL 차단

사용자 요청으로 `Server/Private/GameRoom_WorldEntities.cpp`의 ShipNpc 생성 목록 및 개수
정보 출력 전용 블록 17줄을 제거했다. 실제 NPC 생성·등록과 오류 처리는 그대로이며 UTF-8
BOM 없음·CRLF를 보존했다. 표준 Debug Product 후속 빌드는 14:49:59 KST에 PASS이고
Server object 1개와 EXE만 갱신했다. 후속 evidence는
`out/BuildPipeline/runs/20261002T054959419Z-debug-product.json`이다. 실행 데이터 누락·불일치는
계속 없고 `git diff --check`가 통과했다.

최초 Server PID 11900은 TCP 7777 Listen에 도달했으나 Client PID 25060은 startup log 이전에
0xC0E90002로 종료됐다. 같은 시각 CodeIntegrity 3077/3118은 Smart App Control의
VerifiedAndReputableDesktop 정책이 서명 없는 PhysXCommon_64.dll을 차단했다고 기록했다.
DLL은 3,518,464 bytes, SHA256
`36ea11e88bdfc3f27a07b5f3e03d6f932599b65d38d11f18ad1f4cdf65fd340f`이며 Engine 원본·실제
Git LFS object·HEAD LFS OID와 일치한다. x64 PE 구조와 직접 종속성도 확인했다.
Resources 누락·부분 다운로드·x86 DLL 혼입의 증거는 없다.

사용자가 Windows 보안 설정 화면을 열어 달라고 요청하여 해당 화면만 열었다. 14:51 KST에
사용자가 Smart App Control을 끈 상태(VerifiedAndReputablePolicyState=0)를 확인한 뒤 새
실행 시도를 시작했다. 에이전트가 보안 정책·레지스트리를 수정한 것은 아니다.
최초 실패 receipt와 중단된 Release queue 로그는 보존했다. 상세 증거는 외부 복구 디렉터리의
`PhysX-SmartAppControl-Diagnosis.json`과 `Debug-Launch-Receipt.json`에 있다.

## G11. Debug 정상 실행과 정본 VS 재확인

사용자가 보안 설정을 변경한 뒤 14:52:28 KST의 두 번째 launch는 성공했다.
Server PID36548은 TCP7777 Listen, Client PID17220은14:52:27 Lobby ready에 도달했다.
사용자가 Client/Server가 정상 실행된다고 확인했다. 새 receipt는 외부 복구 폴더의
`Debug-Launch-AfterPolicy-Receipt.json`이다. 사용자가 종료한 Client를 임의로 복구하지 않았다.

이후 성능 설정 적용과 사용자 요청에 따른 재확인을 위해 기존 Server를 유지하고 Debug Client PID29032를
실행했으며15:13:10 Lobby ready를 확인했다. `Debug-HighPerformance-Launch-Receipt.json`에 기록했다.
게임 UI 조작과 gameplay 화면 검증은 사용자가 수행했다.

현재 Wi-Fi192.168.0.14와 endpoint JSON, MSBuild가 평가한 Client Debug/Release x64의
LOSTARK_SERVER_HOST=192.168.0.14, Server의 --bind-address0.0.0.0이 일치한다.
Debug/Release Server firewall은 해당 EXE·TCP7777·LocalSubnet Allow다.
Framework.slnLaunch에는 Server+Client 및 기존 Server를 이용하는 Client Only profile이 있다.
VS의 현재 화면 선택 상태를 직접 확인한 것은 아니다.

## G12. 취업준비와 성능 작업으로 이어지는 현재 상태

사용자가 Profiler·Rendering Workbench·Publish 구조 이해와 실제 FPS 개선을 요청했다.
하드웨어 설정·캡처 수치·소스 분석은 [Day 1 RESULT](2026-10-02_JOB_PREPARATION_DAY01_RESULT.md)와
[Movie 성능 RESULT](2026-10-02_CLASS_MOVIE_PERFORMANCE_RESULT.md)에 분리했다.
기존 Release 대기 PID30792는 아직 빌드를 시작하지 않았음을 확인하고 종료했다.
새 Movie 계측과 Bone Debug 최적화 설정을 포함한 후속 빌드는 사용자 실행 검토가 끝난 뒤 이어간다.
새 C++ 계측 코드가 기존 성공 Debug EXE에 이미 반영됐다고 기록하지 않는다.
Release는 현재까지 미완료다. 기존 Release launch helper PID29356은 새 Release 성공 receipt만 기다린다.
검토된 후속 build queue PID24808이 새 Movie 계측·Bone Debug 최적화 설정을 대상으로
사용자 앱 종료 후 Debug → Release를 진행하도록 대기한다. 외부 MovieProfile-Build-Queue.log와
Build-Debug-MovieProfile-Exit.json, Build-Release-Exit.json이 후속 실행 근거다.


## 후속 Guide·Bern nav·공통 Movie 변경의 현재 빌드 경계

위 PID24808의 이전 빌드 대기는 소스 범위가 늘어나 idle 상태에서 중지했다.
Guide의 연결된 항구 착지와 castle/library 동행, Debug/Release 공통 Movie channel 재사용을
구현했다. Movie 변경 TU10개의 실제 양 구성 컴파일, 실제 v143 모델 수치 검증,
복구 nav의 Navigation58개와 Guide111개 계약 검사가 통과했다.
Bern/Bern3 누락 바닥의 paint 및 Client/Server 산출물8개를 백업 후 정본에 반영했다.
현재 실행 중인 게임을 새 코드·nav로 재시작한 상태는 아니다.

최종 입력47개 SHA를 고정한 `GuideNavMovie-Build-Ready.json`과
`Continue-GuideNavMovie-Debug-Then-Release.ps1`이 현재 통합 빌드 경로다.
실행 중 Client/Server를 임의 종료하지 않으며 사용자 저장·종료 이후 Product Debug→Release를
진행한다. 새 Product 성공과 실제 화면/FPS는 아직 완료로 기록하지 않는다.
세부 증거는 [Bern nav RESULT](2026-10-02_BERN_NAV_GROUND_RECOVERY_RESULT.md)와
[Movie 공통 구현 RESULT](2026-10-02_MOVIE_ANIMATION_SAMPLE_REUSE_RESULT.md)를 따른다.
