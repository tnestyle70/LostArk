# PR 494~496 통합·콜로세움 대기·이동 피킹 결과

## G00. 통합 범위

원본 `Desktop/LostArk`의 미커밋 상태와 실행 중 프로그램은 보존하고 별도 worktree에서
PR #494의 내구도·수리, #495의 경기 표현·승패·외형 복제, #496의 이동·Guide·용병·Waterpang을 합쳤다.
Kouku 인형·칼날·랜덤 출구는 `out/KoukuIntegrationAudit20261001`의 실제 소스/저작 diff 범위로 반영했다.
게시 데이터는 옛 snapshot을 덮지 않고 최종 통합 소스에서 publisher로 다시 생성했다.
통합 PR은 https://github.com/tnestyle70/LostArk/pull/497 이다. 원격 병합 상태는 PR에서,
빌드·ZIP 완료 근거와 사용자 화면 확인 범위는 아래 검증 표에서 구분한다.

## G01. 60 FPS 이하 이동과 근거의 범위

기존 이동 클릭은 `CPicking::Read_Pixel`에서 1 pixel을 복사한 직후 `Map(READ, flags=0)`으로
GPU 완료를 기다렸다. 새 이동 입력은 같은 CPicking에 클릭 당시 pixel과 요청 ID를 제출하고,
`Poll_Picking`의 `DO_NOT_WAIT`로 준비 여부를 확인한다. 결과가 준비되지 않았으면 프레임을 진행한다.
해제된 클릭도 한 번 완료하지만, 새 클릭·후속 이동/스킬·UI/capture·포커스/텍스트 입력·sink/캐릭터 교체·
350ms 만료는 이전 결과를 폐기한다. 기존 편집기용 동기 Picking과 선박의 수면 평면 입력은 유지했다.

실제 게임 근거는 원본 작업 폴더의 `Client/Bin/ProfilerCaptures/릴리즈_도서관_20261001_010211_148_frame7471_28900_1.json`이다.
120 frame 중 피킹 27회에서 `Picking.MapWait` 평균 5.924ms, p95 13.222ms, 최대 41.083ms가 기록됐다.
최대 대기 frame 7360은 CPU 75.286ms 중 MapWait가 41.083ms이며 GPU frame은 36.044ms였다.
`Picking.CopyPixel` 평균은 0.0197ms다. 따라서 기존 실행에서 복사 명령 제출보다 동기 읽기 대기가
주 스레드를 길게 막았다는 근거가 있다. 이 캡처는 사용자의 최신 증상 직후 기록이나 수정 후 비교 기록은 아니다.
분석 근거: `out/IntegrationValidation/picking-existing-captures-20261001.json`.

동일 생산 함수의 headless D3D11 WARP 비교에서는 128/256/512회 render clear 부하 뒤 기존 Read_Pixel이
각각 32.672/69.639/72.541ms, 새 Request+Poll은 0.056/0.101/0.084ms에 반환했다.
이는 합성 부하 실험이며 실게임 성능 수치로 사용하지 않는다.
`out/AsyncPickingBeforeAfter/result.json`의 46개 검사와 12회 미완료 GPU copy가 통과했다.
생산 Controller 3함수의 22개 입력 수명 검사는 `out/AsyncMoveDispatchRegression/result.json`,
40/60 FPS 연타·정지·방향 변경·지연 변화의 30개 예측 조건은 `out/MovementFinal/result.log`에 있다.
30조건 모두 연타 전후 추가 위치 차이 0이며 기존 표시 속도 예산을 넘지 않았다.
최종 게임 화면에서의 끊김 해소는 사용자 확인 항목이다.

## G02. 콜로세움과 통신

Server의 10초 마감에 대기 중인 인간 1~4명을 입장 순서의 짝수/홀수로 양 팀에 배정한다.
대기 화면 아래에 남은 초, 오른쪽에 N/4를 표시한다. 인간이 없는 팀은 용병 4명을 채우고,
인간 팀은 기존 초대 경로로 팀 총원 4명을 맞춘다. Server가 로딩·모집·진입 3초·소개 8.6초·
경기 전 10초·120초 경기·결과를 결정한다. Client의 도열·HUD·결과 배우는 실제 8인 명단을 소비한다.
기존 40줄 HP PvP 수치, 서버 피해 판정, atomic transfer와 Guide packet을 보존했다.
Release F1의 기존 직접 진입도 유지하며, 정식 경기 대기열과 다른 미리보기 경로임을 명확히 했다.

Protocol은 132다. 120 용병 초대, 121 경기 상태, 122 Guide 제어를 보존하고
123 로딩 완료, 124 경기 귀환을 추가했다. match ID는 uint64이며 인간 roster 1~4,
선택 참가자 최대 8, 후보 포함 player 정보 최대 14를 별도로 검증한다.
새 Server와 새 Client를 함께 사용해야 한다.

## G03. 쿠크 후속 검증

Release 1인 카드 미로는 서버 시작 계획의 Debug 전용 분기 때문에 거부됐다.
기존 1인 관찰자·사냥꾼 겸임을 양 빌드로 연결했으며, 다인 방에서 한 명만 살아남은 경우의 규칙은 보존했다.
실제 Q command→타격 tick→상자 파괴→다음 Q→역할·시점 bit·문양 snapshot을 1/2/4인으로 검사했다.
이 거부는 랜덤 탈출 문양이나 편집기 Sequence 3/5 기대값 불일치와 별개였다.

첫 음원은 앞선 continuous 변경 뒤에도 전진 시각 차이가 250ms를 넘으면 재시작하는 조건이 남아 있었다.
연속 전진은 채널을 유지하고 명시 scrub·역방향 이동만 재생 위치를 바꾼다.
실제 설치 WAV와 동일 FMOD의 no-output mixer에서 기존 2 FPS 8초 Play16/Stop15,
3 FPS Play24/Stop23을 재현했고 수정 후 각각 Play1/Stop0이었다.
세부 근거는 기존 KOUKU_PLAYTEST_RECOVERY RESULT 및 `out/KoukuEntryAudioContinuous20261001`에 있다.
사용자가 보류한 Release 서버 상태 문구는 추가 수정하지 않았다.

## G04. 리소스와 게시

Kouku owner publish는 source revision 2498 / sequence revision 183으로 성공했다.
PowerShell 5.1의 긴 경로 오류가 발생한 첫 시도는 owner transaction이 이전 게시본으로 rollback했고,
같은 worktree를 짧은 `L:\LostArk` 경로로 매핑한 정상 publisher 재실행이 373438ms에 성공했다.
게임 소스나 리소스 저장 위치는 옮기지 않았다.

`GBResources2`에는 PR #495의 신규 UI 1848개와 승리 AnimSet 7개를 추가했다.
기존 #496의 1116개를 포함한 총 2971개 / 436602224 bytes가 원본과 SHA256 일치했다.
기존 파일 교체는 0개다. GBResources2는 추가 배포분이므로 완전한 Resources 폴더에 합쳐 사용한다.
ZIP은 Resources를 포함하지 않고 기존 완전한 Resources만 읽는다.

## G05. 검증과 남은 전달

| 항목 | 현재 결과 |
|---|---|
| Release NetworkProtocolHarness | 1681 검사, failures 0 |
| 이동 예측 / 비동기 입력 / 실제 WARP | Debug·Release 각 45 조건 / 22 검사 / 46 검사 통과 |
| Client 콜로세움 UI·소개·결과 검사 | 20 검사 통과 |
| Kouku tuning Python | 15 검사 통과 |
| Release Kouku overlap / product / world playback | 모두 failures 0 |
| Release 카드 미로 실제 Q·snapshot·랜덤 출구 | failures 0 |
| Debug/Release 최종 통합 빌드 | 모두 정상 Product PASS, 누락·무효 런타임 입력 0 |
| Server 회귀 | Release focused 955 / Debug 1210 검사, failures 0 |
| 원격 통합 | 원본 3개 PR head ancestry를 보존한 통합 PR #497 |
| ZIP CRC·manifest·launcher 검사 | 모두 PASS, 게임 프로세스 실행 없음 |
| Client 화면·실청·실제 다인 LAN | 사용자 확인 필요 |

원본 작업 폴더의 다른 세션 변경을 reset/stash/checkout하지 않았다.

## G06. Waterpang과 가이드 최종 반영

Waterpang 첫 AI 생성에서 Loader가 준비하지 않은 직업·모코코 외형·NPC를 주 스레드가
동기로 읽는 경로를 확인했다. 마하라카 Loader에서 두 AI 직업, 모코코 head/outfit 24개,
공유 정본 NPC 8종과 물총 모델·재질을 준비하도록 옮겼다. NPC 외형은 Character 전용
물총 경로에서 제외돼 있었으므로 기존 CPart_Equipment와 실물 right-hand 본에 연결했다.
8종 모두 손의 최종 basis 0.01과 4126 vertex 물총 및 부착 transform을 검사했다.
실제 사용자 장면의 6초 전체를 계측한 것은 아니며, 준비 단계 이동과 최종 화면은 구분한다.
상세 결과는 WATERPANG_PRELOAD_AND_NPC_WEAPON_RESULT에 기록했다.
서버 20개 슬롯은 `Waterpang AI 1`부터 `Waterpang AI 20`까지 안정된 번호를 복제한다.

가이드는 같은 Bern 스퀘어홀·출항 준비 이동에도 도보 복귀만 적용되어 장거리·고도차 뒤에
남는 경로를 수정했다. 이동 완료 때 같은 가이드와 owner를 유지하고 서버 navigation/collision으로
검증한 소유자 주변에 착지한다. 실제 승선·레이드의 베른 대기와 다른 월드 복귀 계약을 보존했다.
Data/Guide, collider, Guide ID와 저작 대사 변경은 없다. 최종 Debug/Release Guide는 각 76개,
Maharaka는 Release 203개 / Debug 243개(추가 Debug WorldPlayback 40개 포함) 통과했다.

## G07. 통합 빌드 준비 근거

별도 worktree의 최초 shader 전체 재컴파일이 길어져 해당 Client 빌드만 종료한 뒤,
동일 664개 shader 입력과 598개 include, 실제 FXC 설정 및 기존 산출물 hash를 대조했다.
동일성이 확인된 CSO 507개와 경로를 바꾼 FXCompile 추적 파일 12개만 재사용했다.
CPP/OBJ/PCH/EXE/DLL은 복사하지 않았으며 정상 MSBuild로 변경 C++를 컴파일·링크한다.
`out/ShaderCacheReuse20261001/receipt.json`은 캐시 검증 근거이며 빌드 성공 증거가 아니다.
최종 빌드 성공은 Product receipt로 별도 판정한다.

## G08. 사용자 Debug 20 FPS 관찰에 대한 비교

사용자는 원본 Debug EXE에서 20 FPS 이동이 의도대로 부드럽다고 보고했다.
실행 중 Client PID 16728과 Server PID 57648은 모두 원본 폴더의 Debug 경로다.
Debug Client는 05:39, Release Client는 05:40 Product 빌드에서 Controller/Character/Replication을
다시 컴파일한 결과다. 둘 다 이번 통합의 비동기 picking 이전 코드다.

Server Room_Loop의 30 Hz steady_clock, 고정 1/30초 이동, 위치 snapshot 전송과
Client의 입력·예측·위치 보정·QPC frame timer에는 구성별 이동 알고리즘 분기가 없다.
실제 컴파일 추적은 Debug /Od /RTC1 /MDd, Release /O2 /Oi /GL /MD이며 양쪽 /fp:precise다.
Debug의 D3D11_CREATE_DEVICE_DEBUG와 도구·진단 workload는 실제 차이다.
Bern 실행 순서는 QPC delta → Character 이동 갱신 → Level snapshot 적용 → Controller의
동기 picking/명령 → render이며, GPU 대기는 현재 명령·render를 늦추고 다음 frame delta에 포함된다.

같은 생산 LocalMovePrediction에 같은 입력 시각을 주고 /Od·_DEBUG와 /O2·NDEBUG를 따로
컴파일한 20/40/60 FPS 45조건은 양쪽 모두 통과했고 출력 byte가 동일했다.
20 FPS 15조건의 속도비는 1..1, 반복 클릭 여부의 최대 위치 차이는 0이다.
근거는 `out/MovementConfigurationCompare/comparison.json`이다.
이는 예측 수식의 구성 차이를 재현하지 못했다는 뜻이며 사용자 실행 증상을 부정하거나
Debug 그래픽 검사 비용이 GPU 대기를 가렸다는 가설을 확정하지 않는다.
실제 실행의 클릭별 CPU/GPU 기록 비교는 F7 capture로 구분한다.

사용자가 현재 실행 중인 Debug에서 새로 저장한
`디버그이동_20261001_065025_156_frame173_16728_0.json`은 120 frame / readback 63회다.
SHA256은 `bf3feb6c99a9f1752aecc67454139540ab2cadcbec54ebd8b5f2125868fa59d8`이다.

| 실제 capture | Debug 이동 | 기존 Release 도서관 |
|---|---:|---:|
| 평균 FPS (frame interval 기준) | 18.96 | 20.44 |
| frame interval 평균 / p95 / 최대 ms | 52.738 / 56.157 / 58.651 | 48.933 / 91.713 / 132.401 |
| Picking.MapWait 평균 / p95 / 최대 ms | 0.2097 / 0.4847 / 1.0982 | 5.924 / 13.222 / 41.083 |
| Picking.Readback 평균 / 최대 ms | 0.2433 / 1.1255 | 5.985 / 41.122 |
| viewport | 1920×1080 | 2560×1440 |

낮은 평균 FPS에서도 Debug는 frame 간격이 비교적 고르고 클릭 추가 대기가 작다는
사용자 관찰을 실제 기록이 뒷받침한다. Release는 동기 Map 대기와 큰 frame 편차가 기록돼 있다.
서로 해상도·카메라가 달라 통제된 구성 A/B는 아니며, profiler에는 순간 좌표·snapshot 이력이
없으므로 GPU 대기만을 모든 위치 튐의 유일 원인으로 확정하지 않는다.
분석은 `out/IntegrationValidation/debug-live-capture-16728-analysis.json`에 보존했다.

별도 읽기 전용 12.177초 계측에서는 실제 Debug Client/Server와 실행 파일 hash를 확인했다.
빌드 프로세스가 없는 5표본에서 Server CPU 평균은 한 논리 코어 기준 0.8%, Client GPU 3D는
28.4%였다. 이는 frame별 hitch 판정 자료가 아니며 위 F7 capture와도 같은 시각 표본이 아니다.

## G09. 최종 실행 파일과 ZIP

정상 Product 빌드는 Release 230968ms, Debug 296193ms에 통과했다.
각 receipt는 `out/BuildPipeline/runs/20260930T214859834Z-release-product.json`과
`20260930T215618535Z-debug-product.json`이며 skip 없이 실제 컴파일·링크한 결과다.
기존 파일 인코딩 및 vendor PDB 관련 경고는 있었지만 컴파일·링크 오류는 없었다.
최종 변경 JSON 44개 parse와 git diff check, numeric 원본 6개 Git checkout byte hash도 통과했다.

`LostArk-Release-20261001-PR494-496.zip`은 소스 commit
`7721f0e955a18f5d3f6e3d0c2b3054312a44b1f2`의 Release 산출물과 최종 데이터를 담는다.
이후 완료 기록 변경은 실행 코드·데이터에 영향을 주지 않는다.
ZIP은 169735298 bytes이며 SHA256은
`c78ff6603affb0f709e1674095209fef630902cc5a23ac20a699aef73b52d661`이다.
기본 payload 2820개 파일(Data 2178, shader 254), numeric 587 fields,
Kouku source 2498 / sequence 183 / protocol 132를 검증했다.
ZIP CRC·중복 경로·전체 manifest 파일 SHA256과 실제 launcher --check가 통과했다.
Client/Server 실행은 0회이며 Resources는 외부 완전한 폴더를 선택한다.
내부 `release-ready.receipt.json`은 실제 EXE/DLL hash와 소스 commit을 기록한다.
증거는 `out/ReleasePackaging/portable-delivery.receipt.json`과
`preflight-20261001-pr494-496-final.json`에 있다.


## G10. 원본 Visual Studio 작업 폴더 protocol132 반영

2026-10-01 사용자 요청으로 Desktop/LostArk의 기존 branch를 7013e90ad에서
b83d646be(main PR497 병합 완료)로 fast-forward했다. Shared packet·직렬화, Server/Client
소비자와 project 등록을 함께 반영했다. 최초 57개 작업 파일은
`out/VSProtocol132Sync/20261001-071523`에 byte/mtime/패치 백업하고,
`safety-vs-protocol132-sync-20261001-071523` stash도 보존했다. stash 복원 뒤
Guide·Profiler 작업을 유지했고 미해결 충돌은 0개다. bootstrap은 공식 generation
validator가 현재 디스크에서 계산한 e22201 generation을 사용했다. 충돌은 generation ID
한 줄뿐이고 나머지 109865 gameplay 행은 upstream/local이 같았다.

VS18 Insiders의 정상 Release Product Build가 통과했다. Engine·Shared·Server·Client를
빌드했고 `out/BuildPipeline/runs/20260930T222020084Z-release-product.json`에 기록했다.
Release Server 산출물 시각은 07:18:00, Client는 07:20:18이다. 별도 loopback 임시 포트의
3초 headless Server 시작·종료는 exit0이고 실제 session diagnostic의
networkProtocolVersion132를 확인했다. Client/UI는 실행하지 않았다.

사용자가 “확인했어 다 됐어”라고 완료를 확인해 추가 작업을 마무리했다.
Debug는 소스132 반영까지 완료됐고 별도 Product Build는 실행하지 않았다. 기존 Debug
실행 파일은 재빌드 전 상태이며 Debug 빌드 완료로 기록하지 않는다.
빌드 완료 뒤 관측한 Character/NavPathFollower/GameRoom_PlayerCommands의 다른 세션 변경은
건드리지 않았다. 최종 증거는 `out/VSProtocol132Sync/completion.json`에 있다.

## G11. 인간 준비 표시와 명시적 용병 모집 교정

사용자가1인 입장에서 관찰한4/5는 Loader 단계가 아니라 인간1명과 자동 참가 처리된
상대AI4명을 함께 센 준비 표시였다. 2인 입장의1/2도 같은 표시 경로다. Server의 실제
READY 장벽은 인간 session만 기다렸으므로 표시·선발 계약과 EXE 종료 원인은 구분한다.

`Level_Development`는 같은 match의 MATCH_FOUND 인간 수와 인간 arrival 범위의 bReady를
사용해 준비 수를 표시한다. 인간 입장 순서에 따른 좌우 교대 배정은 유지한다.
`GameRoom_Colosseum`은 양팀 각각 다섯 용병을 모두 미선택 후보로 만들고 자동 상대4명
참가 처리를 제거했다. 같은 팀의 명시적 고용이4명 파티를 완성해야 경기 입장이 시작된다.
3인 입장의 좌측 인간2명·용병2명, 우측 인간1명·용병3명도 같은 기존 command 경로다.
1인 입장은1/1 준비 후 모집할 수 있지만 상대팀 고용 주체가 없어 자동 전투를 시작하지 않는다.

양팀4명 완료 뒤 ENTRY_COUNTDOWN은300tick, 즉10초로 바뀌었다. 화면의
“전투 아레나에 진입합니다.”10초 뒤 기존 도열 INTRO8.6초, 원본 창살 전투 COUNTDOWN10초,
ACTIVE120초가 이어진다. 입장과 전투 준비의10초 두 구간을 하나로 합치지 않았다.
Shared protocol132와 새 project/filter 등록 변경은 없다. 팀 사용서의 대응 계약도 갱신했다.

변경 Server2CPP와 Client1CPP의 고유 out 최소 /c 컴파일, UTF-8 BOM 없음·CRLF 보존 및
대상 git diff check가 통과했다. 독립 읽기 검토에서 human arrival와 고용 자리 범위의 비중복,
빈 팀 파티의 staged transaction 보존을 확인했다. Client 기존 CP949 헤더 경고는 별도다.

최종 Release Product `out/BuildPipeline/runs/20260930T225620980Z-release-product.json`
이후 실제 `Server/Bin/Release/Server.exe --colosseum-match-contract-test`는
178 PASS, failures0, exit0이다.1~3인 원자 admission, 모든 후보 자동선택0명, 단독 모집 유지,
2·3인 양팀 고용,4인 고용과300tick, 기존PvP/rollback/HP 계약을 검사했다.
실행 전후 Server SHA256은 `dda6ba915b53788b2ba52f8786c5225de9091af0d84d70dd46f2c3fe012b75cc`
로 동일했다. 상세 로그·소스/바이너리 hash는
`out/MotionAuditFollower20261001/colosseum_candidate`에 있다.
Debug Product `out/BuildPipeline/runs/20260930T225733710Z-debug-product.json` 이후 같은
focused 계약도178 PASS, failures0, exit0이다. Debug Server SHA256은
`d7376eb9b4bd3f3023df7b13ac596bef9d5d1a6c4d4b836619f1dcd51cf294d3`이며 실행 전후 동일했다.
Release6.99초·Debug78.55초는 실제 계약 테스트 경과 시간이며 제품 frame 성능 측정은 아니다.
실제 Client 입장·다인 화면은 자동 실행하지 않았다.
관찰된 c0000409/subcode7 종료가 이 수정으로 해결됐다고 판정하지 않는다.

## G12. 카메라 좌표 범위 초과 수정과 종료·메인 지연의 증거 경계

사용자가 제시한 `ColosseumIntroCutscene.h::Vector`의 좌표 경계 오류를 현재 소스에서
확인했다. 입력 배열이 정확히 3개인지 검증하고 출력 포인터도 x/y/z 3개만 만들었지만,
루프는 4회 실행해 정상 카메라 JSON에서도 `values[3]`을 읽었다. 이 잘못된 원소가
숫자로 취급되면 이어지는 `pOut[3]` 접근도 범위를 벗어난다. 기존 `i < 4u`를
`i < values.size()`로 바꿔 이미 검증한 3개 좌표만 처리한다. 팀당 4명, 이름 4행,
양팀 도열 슬롯과 카메라 데이터는 변경하지 않았다.

제품 헤더의 Field/Number/Array/Vector/Load_Document 전체 함수와 DOCUMENT 구조,
실제 DataJson 파서·값 구현을 추출한 독립 native fixture에서 전후를 검증했다.
Engine 공통 include만 primitive type alias로 대체했으며 Client·UI는 실행하지 않았다.
현재 `Data/Camera/ColosseumIntro.cutscene.json`과 기존 Protocol132 재배포·0802 번들의
실제 동일 경로 JSON 세 개를 각각 읽었다. 세 문서 SHA256은 모두
`6c1f0ae52e43d46b5d76022b3517ea851cdab6e6ab1bfe2c6839d1086fbbb466`이다.

| native 구성 | 수정 전 실제 문서 3개 | 수정 후 실제 문서 3개 |
|---|---|---|
| MSVC Debug checked STL | 3회 모두 vector subscript out of range | 각 45 checks PASS, exit0 |
| MSVC AddressSanitizer | 3회 모두 Vector → Is_Number의 container-overflow, READ 4 bytes | 각 45 checks PASS, exit0 |

수정 후 실제 6개 벡터의 유한 좌표와 원래 값, 문서 전체 로드, 2개 shot,
팀 슬롯 4/4와 이름 4행을 확인했다. 잘못된 shape·타입·범위·NaN/무한대 15조건은
예외로 거부하고 호출자의 반환값 대입 대상과 전후 guard를 보존했다.
범위 끝값 ±100000도 정상 승인했다. 이전 fixture의 Debug 종료 91과 ASan 종료 92는
독립 child에서 실패를 수집하기 위해 지정한 값이며 사용자 Client의 종료 코드가 아니다.
확인된 메모리 접근 결함은 수정했지만, 이전 사용자 프로세스의 덤프가 없으므로
과거 c0000409/subcode7 종료를 이 호출 하나로 역추적한 것으로 기록하지 않는다.

헤더는 UTF-8 BOM 없음·CRLF를 유지했고 제품 변경은 루프 경계 1줄뿐이다.
수정 후 SHA256은 `db049c46254dd31552ac7cfd580e77f634bdf234a961313191cf583846a7571e`이다.
대상 diff check와 native 컴파일이 통과했다. 원문 전후, 파서·입력 hash, 빌드 명령,
실패 stack 및 전후 12회 실행 결과는
`out/FramePacingAudit20261001/colosseum_vector`에 있다.
이 검증 완료 시점에는 통합 Product 재빌드와 사용자 화면 재확인이 별도 단계로 남아 있다.

앞서 추가한 `Client/Default/Client.cpp` 종료 진단은 그대로 유지한다. 시작 시
`Client/Default/ClientCrash.user.log` append handle을 준비하고 terminate/SIGABRT에서
활성 std::exception의 what, PID/TID·시각, 최대 64개 현재 stack 주소와 모듈 base/RVA를
고정 버퍼로 기록·flush한 뒤 기존 종료 handler/abort 동작을 따른다. 작업 스레드에는
메인 스레드의 terminate handler가 자동 전파되지 않아 process SIGABRT hook도 연결했다.
독립 child의 메인 예외, 작업 스레드 예외, 활성 예외 없는 terminate 세 경우에서
로그·주소 계산·기존 종료 동작 호출을 확인했다. 기존 ExitDiagnostic은 보존했다.
직접 fast-fail처럼 이 경로를 통과하지 않는 종료까지 수집한다고 보장하지 않는다.
증거는 `out/FramePacingAudit20261001/terminate_diagnostic`이며 진단 자체를 원인 수정으로
간주하지 않는다.

성능 근거는 종료와 분리했다. 실제
`Client/Bin/Release/Diagnostics/client-session-50060.jsonl`에서 Colosseum world7의
07:42:01.704와 07:42:07.044 KST에 main-pump 간격 1610ms·2218ms가 기록됐고,
그때 마지막 수신 경과는 각각 15ms·30ms였다. 로그의 maxMainPumpGapMs=9141은
같은 연결 generation의 앞선 Bern world1 구간에서 누적된 최대값이므로
Colosseum에서 새 9.1초 멈춤을 측정한 것으로 읽지 않는다.
이 계측은 메인 네트워크 pump 호출 사이 간격이며 개별 함수 CPU 시간이나 GPU 대기 시간이
아니다. 1초 이상 간격을 세되 stall 기록에는 5초 간격 제한도 있어 모든 hitch 목록도 아니다.
마지막 표본의 가용 물리 메모리는 약 38.8GiB이며, 현재 자료는 OOM 판정 근거가 아니다.

코드에서 `PlayableCharacterAssetService::Commit_Models`는 준비된 prototype과
작은 readiness 항목을 등록하며 여기서 모델 decode를 다시 하지 않는다.
Poll은 worker 완료를 0ms로 검사하고 이미 끝난 worker만 join한다. 따라서 함수 이름이나
worker 준비 총시간만으로 1.6~2.2초 지연을 main commit 비용으로 단정하지 않는다.
더 구체적인 후보는 `ClientReplication::Advance_PlayerAssetPreparation`이 한 프레임에서
준비된 pending spawn과 presentation을 개수·시간 budget 없이 모두 반영하는 경계다.
그 아래 실제 Character clone/Initialize, 바인딩 admission과 저장 외형 적용이 이어진다.
이는 동기 작업이 모일 수 있는 구조 근거이며 아직 해당 호출의 초 단위 실측은 아니다.

사용자가 다음 F7 capture를 저장하면 `Network.PlayerAssets.Advance`,
`CharacterAssets.Commit`, `Network.PlayerPresentation.CommitSpawn/Replace`,
`Character.Initialize` 및 Character admission scope의 해당 프레임을 먼저 비교한다.
부모·자식 scope와 worker 시간을 합산해 메인 지연으로 만들지 않는다.
07:42:07.044 이후 실제 종료 직전의 함수별 기록은 기존 session 로그에 없으므로,
로딩 중 지연과 뒤의 모집 화면 전환·종료가 같은 원인이라는 결론은 남겨 두지 않는다.

## G13. 피킹·모집·입장/승리 컷신 최종 통합 빌드

다른 사용자 세션의 `ColosseumMatchView.cpp` 변경도 같은 Desktop 정본에서 통합했다.
이미 있던 승리4자리와 Server participant/winningTeam 선택은 유지하며, 실제 승리 Character와
Transform이 아직 없으면 카메라를 복원하고 다음 frame에서 다시 조회한다. 서버 결과 시계는
연장하거나 되감지 않는다. 미고용 후보를 대체 배우로 생성하지 않는다.

최신 실제 Intro/MatchView 배우 선택·정리 함수와 적용 루프를 native로 추출한545검사가
통과했다. 양팀4명, 승리A/B의 실제1~4명, valid team/arrival을 가진 미고용 후보6명 제외,
두 stable ID 일치, 늦은 표현·같은 ID body 교체·퇴장과 승리0명/Transform 미준비를 확인했다.
실제 camera JSON의4자리를 읽었으며 검사 전후 소스·JSON hash가 같다.
`out/ColosseumRosterAudit20261002`에 manifest와 결과가 있다. Replication/Character/Camera
경계는 대역이므로 실제 Client 입장·애니메이션·GPU 표시 성공을 대신하지 않는다.

최종 정상 Product build는 Release
`out/BuildPipeline/runs/20260930T230834768Z-release-product.json`, Debug
`out/BuildPipeline/runs/20260930T230907094Z-debug-product.json` 모두 PASS다.
SkipBuild·Clean·Rebuild·tracking 삭제 없이 기존 build pipeline을 실행했다.
최종 Client Release SHA256은
`94a649dd7bea0363cc99e95285928609eb70ea55e13966ee792ea31fde3bee30`이다.
대응 EXE/PDB는 `out/ReleasePackaging/20261001-final-symbols`에 함께 보존했다.
최종 Intro header hash는 G12와 같고 MatchView hash는
`c84057f75b006c4a20a6e74d45065390c314d7eacb9f9cdad316070048378bb5`다.
Client/UI를 자동 실행하지 않았으며 사용자 재현 성공 판정은 남아 있다.

피킹 변경의 상세 수치·실제 이동 소비 검사는9월22일 성능 RESULT의G21/G22에 있다.
Server 모집 계약은 각 구성178검사, 이동 입력은 각 구성27검사, 실제 Engine follower와
Character 소비 회귀는 각 구성189검사, 컷신 파서 수정 후 Debug/ASan은 실제 문서별45검사다.
Client 컷신 수정 뒤 Server 바이너리는 바뀌지 않아 동일 계약 검사를 반복하지 않았다.

쿠크 최초 입장음은 기존 두 수정이 Protocol132-v2에도 포함된 것을 확인했다.
이번 조사에서 최초 입장음의 추가 수정 원인을 확정하지 못했으므로 음향 문제가 완전히
해결됐다고 기록하지 않는다. 별도 cardmaze 시계 중복 결함은 현재 패키지 수정 범위에 넣지 않았다.
상세 증거는 기존 KOUKU_PLAYTEST_RECOVERY RESULT와
`out/KoukuAudioClockAudit20261002`에 있다.

최종 공유 파일은 Desktop의 `LostArk-Release-20261001-Movement-Colosseum.zip` 한 개다.
169,700,299bytes, SHA256
`19e4d80d084f68cfba71cbf97fa63d166fd563f59285ae680a6210eb01815e0d`다.
Release Product receipt는 위 G13의08:08 빌드이며 포함된 Client hash도 일치한다.
패키징20검사, source bindings587필드, launcher --check, ZIP CRC와 모든 manifest 파일 hash를
통과했다. Resources는0개이며 외부 기존 LostArk Resources와192.168.0.22:7777을 사용한다.

첫 ZIP 생성 후 안내문의 이전 GPU picking/자동 상대AI 설명을 교정해 재포장했다.
교체 직전 WinError32로 멈췄고 사용자가 .partial.zip 업로드 화면을 전달했다. 임시 ZIP의
전체 CRC·각 파일 hash·최신 안내문을 다시 검사한 뒤 정식 이름으로 원자 교체했다.
이전 ZIP은 이름 있는 backup으로 보존했다. 로컬 검증은 업로드 서버의 파일 내용 검증이
아니므로 공유본을 최종 정식 이름의 ZIP으로 교체하도록 안내했다. 최종 영수증은
`out/ReleasePackaging/20261001-final-delivery.receipt.json`이다.


### G12 후속. 서버 참가자 도열과 승리4인 준비

입장은 Server `Participants`의 실제 PlayerId+NetEntityId를 조인하고 양팀 각각4개
자리 중 arrivalIndex/2 자리를 사용한다. 정상 경기의 인간+직접 고용 용병8명이 대상이며,
미고용 후보를 컷신 배우로 만들지 않는다. 매 frame 실제 roster를 재조합하여 늦은 모델을
추가하고 이탈한 참가자를 제거하되 다른 참가자의 자리 번호를 당기지 않는다.

승리 JSON·생성기·runtime은 이미4개 자리였다. 현재 정본과 기존 Protocol132-v2의
승리 JSON은 SHA256 `0158fb321ec339f8cc8b0bec866bd4e52c6f7692156505f88c051b9e197f0f6a`로
같으며1.4m 간격, 카메라4m 후퇴·0.5m 상승,151개 key를 유지했다. 최초 MATCH_FLOW_RESULT의
2인 설명은 당시 구현 기록임을 명시하고 현재4대4 정본 링크를 추가했다.

`ColosseumMatchView.cpp`는 실제 winner Character/Transform과 유효한 자리 유무를
`Has_PresentWinner`로 검사한다. 승리 배우가 전혀 없으면 카메라·배우 override를 해제하고
다음 frame에 다시 시도한다. 결과 Server 시각·귀환 시점은 멈추지 않는다. 정상4인 승리,
일부 이탈, 무승부와 연출 뒤 기존 visibility/pose/animation 복원 경로를 유지한다.

`python -B Tools/LpkPipeline/test_colosseum_cutscene_roster.py --out out/ColosseumCutsceneRoster20261001`
실행은5그룹381개 assertion, failures0으로 통과했다. 생산 ACTOR·Build_Actors·Clear_Actors·
Has_PresentWinner 본문을 그대로 추출한 C++20 fixture이며 MSVC14.51의 checked STL,
/RTC1, /W4 /WX로 컴파일했다.0~8명, 양쪽 승리팀4자리, 후보제외, 두 ID 정확조인,
후발·교체·이탈·expired body, 자리 유지, 원래 suppression 복원, 승리 배우 준비를 검사했다.
Character/replication은 모의 타입이며 Sample_Presentation 전체, GPU/animation·실제 입장은
이 fixture로 실행하지 않았다. 이 인원 검사는 Server의 모집·시작 조건을 바꾸지 않는다.

변경 CPP의 별도 Release /c 컴파일과 기존 UI/resource validator(2 layouts,3 timelines,
1800 image refs,151 camera keys,7 cheer), Python parse, diff check도 통과했다.
C++ UTF-8 BOM 없음·CRLF를 유지했다. 같은 저장본의 다른 세션 Product Release receipt
`out/BuildPipeline/runs/20260930T230834768Z-release-product.json`은 PASS이며 최종 수정 뒤
ColosseumMatchView.obj와 Level_Development.obj를 실제 재컴파일한 기록을 확인했다.

최종 소스 hash·명령·로그는 `out/ColosseumCutsceneRoster20261001/verification.json`,
`receipt.json`, `client_compile.log`, `run.log`에 있다. 이 세션은 Client/UI를 실행하지 않았다.
사용자 요청대로 ZIP은 다른 세션에서 같은 저장본 기준으로 묶으며, 실제 다인 화면·환호
구도 확인은 사용자에게 남는다. 기존 Protocol132-v2 EXE를 이 세션에서 덮어쓰지 않았다.


## G14. 단독 입장의 상대4명 자동 배정과 1~4인 Server 계약

사용자는 인간1인일 때만 상대 용병4명 자동 배정을 승인했다. `GameRoom_Colosseum.cpp`는 인간 roster 크기가 정확히1이고 반대 팀 인간이 없을 때만 상대 후보5명 중4명을 참가자로 확정한다. 본인 팀3명은 직접 고용하고, 인간2~4인은 기존 양팀 직접 고용을 유지한다. 기존10초 입장 시계·권위·stable identity는 보존한다.

`ServerGameplayContractTests_ColosseumMatch.cpp`는 인간1~4인마다 준비·모집·입장·전투120초·결과·승리4자리·인간 전원의 typed Bern 귀환을 실제 CGameRoom fixture에서 검사한다. 격리 후보 Server.exe를 현재174개 런타임 데이터로 실행한248개 검사가 통과했다. Child의 LOSTARK_SERVER_DATA_ROOT만 명시했으며 데이터와 기존 제품 바이너리·중간 산출물을 바꾸지 않았다.

증거: `out/ColosseumSolo20261001/run-082442-e6389bc6/contract-data-root-receipt.json` 및 `colosseum-match-contract-data-root.log`. 후보 SHA256 `7af7affec2885cef8013bae238b70c0376f1ae443260aaa7630fb60ea568b944`. 이 최초 격리 검증 시점에는 G13 ZIP에 아직 포함되지 않았다. 후속 제품 빌드·패키지 상태는 아래 기록을 따른다.

## G15. 사용자 실제4인 종료: 모집 문구의 누락 glyph 예외

사용자는 새 EXE로4인 입장해 모든 Client가 종료됐다고 보고했다. 로컬 PID62428의08:28:24.543 종료 로그는 `CXX_TERMINATE`, `std::exception what=Character not in font`를 기록했다. 정확한 G13 Client/Engine과 PDB에서 해석한 stack은 `SpriteFont::FindGlyph -> SpriteFont::MeasureString -> CGameInstance::Measure_Text -> CLevel_Development::Render_PartyInviteText -> CMainApp::Render`다. 로컬 Client 종료의 직접 원인이 확정됐으며 다른 PC의 stack은 아직 수집하지 않았다.

RECRUITING 진입 때 처음 표시하는 `용병 모집 · 1팀 … · 2팀 …`의 U+00B7 두 개는 설치 YoonGasiIIM 및 소형 파생 font에 없고 defaultCharacter도0이다. 전체40개 설치 font의 glyph table과 hash를 조사했다. 해당 문구를 ASCII `|`로 변경한다. 공통 CustomFont fallback 후보는 작성·오프스크린 검증했지만, 사용자가 최종적으로 글자 수정만 우선하여 ZIP을 요청했으므로 후보를 out에 보존하고 제품 소스에서는 본인 변경만 제거했다. 이번 ZIP의 Engine font 구현은 기존과 같다.

08:28:25 WER는 같은 PID·Client.exe의 ucrtbase.dll+0x7F6FE, c0000409/subcode7이다. 최근 Server WER는 없으며 모든 PC 종료를 Server crash로 단정하지 않는다. 실제 dump는 남아 있지 않아 현재 stack 주소를 해당 PDB로 해석했다. binary/PDB·원본 fatal log·WER·symbolized stack·font coverage는 `out/ColosseumFourCrash20261001`에 보존했다.

사용자는 우선 글자 수정본 ZIP을 요청했다. 준비 UI의 이름/HP 누수·임시 illustration·모델 lookup mip 경로와 Bern GPU 최적화는 후속 조사로 남긴다. 해당 시각의30.6ms Bern GPU 병목과 이번 C++ 예외를 같은 원인으로 설명하지 않는다. 실제 Client4개 화면 검사와 서버/폰트 자동 검사를 구분한다.


### G15 최소 수정 검증

최종 범위는 모집 문구의 `\u00B7` 두 곳을 ASCII `|`로 바꾼1줄이다. 준비 UI/HUD/portrait 개선은 `out/ColosseumLoading20261001/pending-ui.patch`, 공통 font fallback은 `out/ColosseumFourCrash20261001/font-fallback-deferred.patch`에만 보존했고 해당 제품 소스 변경은 제거했다.

원래 fallback=0인 실제 YoonGasiIIM atlas와 DirectXTK를 D3D WARP 오프스크린에서 검사했다. 이전 문구의 `Character not in font` 예외를 재현하고, 새 문구의 glyph 전부 존재·MeasureString·DrawString·GPU 픽셀 출력을 확인했다(8검사, 실패0). 근거는 `out/ColosseumFourCrash20261001/font-regression/separator-validation.json`이다. 별도 공통 font 후보의529검사는 이번 제품 변경의 검증 수로 합치지 않는다.

최종 최소 범위 Release Product는 `out/BuildPipeline/runs/20260930T233923226Z-release-product.json` PASS이며, 그 앞의 `20260930T233836949Z-release-product.json`은 공통 font 후보가 포함됐던 중간 빌드라 배포 정본으로 사용하지 않는다. 정상 build pipeline으로 source 복원 후 재컴파일·배포했으며 SkipBuild는 사용하지 않았다.

실제 배포 Server도 `--colosseum-match-contract-test`로248검사 PASS였다. SHA256 `6942170ffc6fb765d85ac71cf896ddf1835388fab0ee7176aa99c6156c40ca8b`, runtime174파일과 EXE hash/mtime 전후 불변이다. G14의1인 상대 용병4명 자동 배정이 이번 새 Server에 포함됐다. Client·실제4인 화면은 자동 실행하지 않았다. 포장 도구20검사 PASS, numeric source bindings587필드 유효성 검사도 통과했다.

최종 ZIP을 08:41 KST에 같은 Desktop 경로로 원자 교체했다. `LostArk-Release-20261001-Movement-Colosseum.zip`, 169,702,573bytes, SHA256 `edd7a774e1c0f513a6a69ac5393906331735c2c78060df569ee52b4831c1bcc2`. 이전 G13 ZIP은 `LostArk-Release-20261001-Movement-Colosseum.backup-20261001-084153-842434.zip`으로 보존했다. 모든 ZIP entry CRC와 manifest payload SHA, launcher --check가 PASS이며 Client/Server는 시작하지 않았다. 새 배포 영수증은 `out/ColosseumFourCrash20261001/delivery.receipt.json`, 정확한 새 EXE/DLL/PDB는 동 폴더 `final-symbols`에 함께 보존했다. 이 G15 파일이 G13 배포 ZIP을 대체하는 현재 정본이다.
