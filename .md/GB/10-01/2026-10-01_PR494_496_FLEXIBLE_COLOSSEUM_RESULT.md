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
