# 로딩 정지·전투 프레임 비용과 이름 있는 Profiler 캡처 결과

## G00. 사용자 캡처로 확인한 원인

2026-09-11 사용자가 저장한 JSON 8개와 첨부한 Windows 응답 없음 화면을 분석했다. 화면은 운영체제의 응답 없음 표시이며 이것만으로 process crash를 판정하지 않는다. 저장된 main-thread scope가 긴 정지를 확인한다. Scope 부모와 자식의 inclusive 시간은 합산하지 않았다.

| 캡처 / 프레임 | 확인한 비용 | 의미 |
|---|---:|---|
| 20:09:05~10 / 15598 | Engine.LevelUpdate 23,321.96ms | Character Select의 동기 class 모델 준비 경로 |
| 같은 캡처 / 15611 | Effect.Prewarm.Advance 13,302.61ms | target 하나 전체를 main에서 준비해 incremental 호출도 멈춤 |
| 같은 캡처 / 15614 | Effect.Prewarm.Advance 3,511.17ms | 다른 target에서도 반복 |
| 20:06:19~23 / 11470 | MainApp.LevelAndEnvironment.Update 3,360.57ms | 쿠크 activation의 미분리 비용; 이번에 하위 계측 추가 |
| 20:06:23 / 11591 | Profiler.Panel.Refresh 173.12ms | 프로파일러의 반복 정렬·집계 간섭 |

최신 차원술사 ALT V 파일은 `profiler_20260911_200949_603_frame17219.json`이다. 16020~17219의 1200프레임이며 CPU/GPU scope와 model sample drop은 0이다. 전체 CPU 평균 27.96ms, P95 65.84ms다. Profiler refresh가 없는 프레임 중 Particle.Update가 1ms 이상인 148프레임은 CPU 평균 53.86ms, Spawn 7.389ms, Update 9.184ms, FrameRebuild 2.560ms, Engine.Update self 7.273ms다. 비교용 가벼운 871프레임은 CPU 평균 20.06ms, Engine.Update self 2.539ms다. 이 분류는 입력 키나 skill ID가 기록된 자동 구간 라벨이 아니라 측정된 입자 부하 기준이다.

같은 heavy 구간에서 pending 3프레임을 제외한 valid 145프레임의 GPU Render.Draw는 평균 12.930ms이고 Lights 6.390ms, NonBlend 2.792ms, SSAO 1.391ms, SceneHDR 1.079ms다. GPU 전체 interval 54.69ms는 CPU의 명령 공급 공백을 포함할 수 있으므로 GPU가 54.69ms 동안 연산했다고 해석하지 않는다. FrameIntervalMs는 이전 frame-start 간격이므로 현재 CpuFrameMs와 긴 프레임 번호가 한 프레임 어긋날 수 있다.

## G01. 저장 파일 계약과 툴

F1 → Open Composition Profiler에서 Save name에 한글을 포함한 이름을 입력하고 Save JSON으로 저장한다. 빈 이름도 허용한다. timestamp·frame·process ID·sequence를 붙여 같은 이름으로 반복 저장해도 다른 파일을 만들고, 최종 commit은 기존 파일 덮어쓰기를 거부한다. 기존 파일을 누적 수정하는 방식이 아니라 저장 시점마다 최근 최대 1200프레임의 독립 snapshot을 추가한다.

Saved JSON 탭은 최초 열기, Refresh files, 저장·삭제 완료 때 목록을 갱신한다. 선택한 일반 JSON 파일만 Delete selected JSON으로 삭제하며, 파일 identity/크기/수정 시각과 capture root를 실제 handle로 재검증한다. stale 선택, 외부 교체, 경로 이탈, I/O 실패는 기존 파일·정상 목록을 보존한다. 폴더를 매 프레임 검색하지 않는다. 사용자 캡처는 에이전트가 삭제하지 않았다.

실제 ProfilerCaptureIO.cpp를 Debug로 컴파일한 임시 폴더 검사에서 한글·1000개 동일 이름/프레임의 고유 경로·비동기 no-replace·목록 필터·stale 선택 거부·임시 파일 삭제·실패 보존·single flight·종료 처리를 통과했다. symlink 생성 2개 검사는 Windows 권한 1314로 실행하지 못했으며 PASS로 기록하지 않는다. Picking 추가 후 JSON counter는 43개다.

## G02. 모델·캐릭터 준비 경계

PlayableCharacterAssetService의 기존 CModel 경로를 Prepare/Commit으로 나눴다. 필요한 class의 모델 decode, device-only 자원 생성과 authoring 문서 준비는 worker에서 수행하고, main Poll이 level generation을 확인한 뒤 기존 Add_Prototypes batch를 commit한다. ready mutex는 긴 모델 decode를 감싸지 않는다. 실패·취소·오래된 결과의 큰 모델과 문서 해제도 worker가 수행한다.

Character Select는 최신 선택 의도를 유지하며 모델 준비 뒤 같은 typed Server class-change command를 보낸다. Replication의 cold playable spawn/class replacement도 entity별 최신 snapshot을 보존한 채 준비를 기다리고 기존 transaction으로 교체한다. despawn/reset은 pending을 지우고 실패한 class를 매 snapshot마다 다시 로드하지 않는다. 기존 character는 실패·대기 동안 보존한다. 여섯 class 전체를 일괄 선로드하지 않는다.

SkillBindings/EffectCues/FaceSliders의 prepared 문서와 FaceMorph CPU 입력도 같은 generation의 모델과 함께 공개한다. FaceMorph immutable target 자료는 공유하고 instance별 weight 상태는 분리한다. 명시적인 Animation Tool Reload는 기존 검증을 유지하며 성공한 skill binding을 prepared snapshot에도 반영한다.

Loader::Initialize는 main에서 Server 승인 class와 immutable authoring 입력을 캡처한다. worker는 EffectCatalog 전역 컨테이너를 다시 읽지 않고 snapshot의 membership으로 EffectCues를 검증한다. PlayerSkillCatalog는 MainApp 초기화에서도 한 번 준비해 Release의 첫 진입이 Debug F1 도구 초기화에 의존하지 않게 했다. Loader에서 알 수 없는 class를 LanceMaster로 바꾸는 fallback도 제거했다.

취소 요청이 binary decode 하나를 즉시 중단시키지는 않는다. 실제 Warlord 9개 모델을 준비한 headless 검사에서 stale generation 결과는 19,618.9ms 뒤 거절됐고, 이미 준비된 결과의 취소·해제는 요청 뒤 101.63ms, decode 도중 취소는 요청 뒤 10,202ms가 걸렸다. 따라서 정상 레벨 전환은 main에서 worker를 join하지 않는다. MainApp이 전환 요청을 보존한 채 Cancel_AllAsyncPreparations를 요청하고 Has_ActivePreparations가 끝날 때까지 현재 프레임을 진행한다. pending 전환 중 새로운 class job은 시작하지 않으며 큰 payload 해제 뒤에만 drain 완료를 공개한다. process 종료의 bounded join/fail-fast 계약은 유지한다.

같은 실제 모델 검사에서 main Poll 4,834회가 계속 진행됐고 최대 관측 Poll wall time은 13.8126ms였다(OS scheduling 포함). 종료 후 readiness와 pending DTO, active preparation이 남지 않았다. 이는 의도적으로 stale generation의 main prototype commit을 거부한 CPU/WARP 검사이며 실제 Server class 교체 성공이나 FPS 실측은 아니다. 실제 EffectCues 26 targets/26 cues의 snapshot parser 일치·잘못된 clip rollback, FaceMorph 52 targets의 immutable 입력 공유·독립 weights/reset도 통과했다. 총 로딩 계산량이 사라지거나 20초 준비가 즉시 완료된 것으로 보고하지 않는다.

## G03. Effect worker와 로딩 전환

Loader의 EffectLoadPreparationJob 실행 루프를 공용 Run_ProductPreparationWorker로 옮겼다. Runtime prewarm은 단일 지속 worker와 bounded 슬롯으로 같은 stage/result/ACK 경로를 사용한다. main Advance는 결과 확인과 commit을 수행한다. 단순히 프레임당 target 한 개를 처리하면서 한 target의 13초 decode를 main에서 실행하던 경로를 제거했다.

카메라 준비와 새 문서의 metadata I/O도 worker stage에 포함한다. worker가 원본 파일의 read lease를 유지해 stage와 commit 사이 쓰기·교체를 막고, commit은 catalog/document/projection/registry/revision identity를 확인한다. Loader가 runtime 작업을 인계받을 때 nonblocking cancel/drain 완료를 확인한 뒤 새 owner를 연다. 실패 receipt와 원인 문자열을 보존하고 실패 target의 매 프레임 재시도를 막는다. 취소·종료는 협력 취소, 동기 I/O 취소와 bounded join을 사용한다.

기존 mailbox/queue transaction 검사와 현재 worker 코드를 추출한 headless protocol 검사에서 지속 thread 재사용, 단일 슬롯, ACK backpressure, target 계측의 ACK 대기 제외, 취소 시 worker 자원 해제, 잘못된 ACK, 예외, COM 실패, 종료, 실패 후 100프레임 재시도 없음이 통과했다. 실제 파일 lease의 읽기 공유·쓰기/rename 차단·해제·stale metadata 거부도 통과했다. 이 검사는 실제 게임의 전체 Effect 시각 실행 성공을 대신하지 않는다.

## G04. 쿠크 activation과 shader

9,813,963-byte WorldSequence 문서는 Loader의 기존 map/deploy 준비 뒤 같은 parser/validator로 typed stage를 만든다. activation은 실제 map/deploy 대상의 metadata가 일치하는지 확인하고 문서를 move한다. missing optional, invalid, cancelled, target mismatch는 별도 실패 상태로 보존하며 main parse fallback을 만들지 않았다. 명시적인 authoring Reload는 기존 Load_Area를 유지한다. 초기 visibility의 placement 반복 선형 검색도 한 번 만든 lookup으로 줄였다.

실제 Debug CDataJson parser를 headless worker에서 실행한 비용은 parse 1232.52ms, JSON tree 해제 208.866ms였다. 원본은 172 templates/208 instances다. 이는 JSON 부분의 수치이며 전체 typed validation·레벨 activation이나 게임 FPS 개선 실측이 아니다.

Debug/Release runtime은 이미 CSO만 읽는다. 이번 문제를 runtime HLSL compile으로 진단하지 않았다. Shader는 prototype 내 exact input signature가 같은 pass의 immutable input layout을 공유하고 CSO read/FX11 create/binding/layout 비용을 계측한다. 조사한 5개 CSO의 66 passes에는 unique input signature가 합계 6개였다.

## G05. 피킹과 Profiler 반복 비용

기존 Picking.Update는 매 프레임 전체 Target_PickPos를 staging으로 복사하고 Map으로 GPU 완료를 기다린 뒤 전체 화면을 CPU 메모리로 복사했다. 1280×720×float4 기준 프레임마다 14,745,600 bytes이며 RowPitch도 무시했다. 실제 소비자는 MapTool 배치/브러시, Effect Tool pivot, F1 Move Player다. 일반 gameplay 이동은 별도 ray 경로다.

이제 실제 Picking 요청 때 현재 cursor와 현재 render target에서 1×1 pixel, 16 bytes만 읽는다. 요청이 없으면 readback을 하지 않는다. 지연된 다른 좌표의 결과를 재사용하지 않고 no-hit과 출력 position W 계약을 유지한다. 클릭/브러시의 동기 Map 대기는 남아 있으며 Picking.MapWait으로 확인할 수 있다. CPU Readback/CopyPixel/MapWait/ReadPixel, GPU CopyPixel과 readback 횟수/bytes를 추가했다.

실제 Initialize/Read_Pixel 함수로 만든 WARP 검사에서 idle readback 0, 요청 4회/64 bytes, 좌표 변경·no-hit·resize·잘못된 입력 및 다른 thread 거부가 통과했다. HWND/UI wrapper와 GPU Map 실패 주입은 검사하지 않았다.

CPU aggregate는 매 refresh의 raw sample 정렬과 map lookup 대신 thread별 completion 순서와 dense name ID를 이용한다. Debug STL의 sample별 stack push/pop도 bounded 배열 인덱스로 대체했다. 추가 background 집계 thread나 매 프레임 전체 집계는 만들지 않았다. 실제 캡처 8개를 같은 Debug /Od 조건에서 비교해 0/1/17/120/1200/9999 window의 Inclusive/Self/Max/Calls가 허용 오차 내 일치했다. nested/zero/sibling/interleaved thread/orphan/window synthetic 검사도 통과했다.

차원술사 캡처의 native 비교 중앙값은 120프레임 집계 22.2969→1.4497ms, 1200프레임 집계 159.018→11.1101ms였다. 게임 전체 FrameMs/FPS가 그 비율로 개선된다는 주장이 아니다. source Debug compile 설정은 바꾸지 않았다. UI.Runtime의 HUD/minimap/skill/icon 등 15개 구간과 Engine input/sound/camera도 계측해 남은 self 비용을 다음 캡처에서 분리한다.

Model.Load.Binary 아래 Decode/Meshes/Materials/Bones/Animations 구간도 추가했다. 최종 Profiler Tool 목록은 CPU 253구간, GPU 26구간과 counter 43개다. 목록에 존재하지만 해당 캡처에서 실행되지 않은 구간은 관측되지 않은 상태로 표시하며 시간 0으로 확정하지 않는다.

## G06. 입자 반복 작업 제거

Effect_Playback의 기존 prepared resources에 SourceSpawnPerUnit module index와 source death-event generator element ID를 캐시했다. 문서가 stage될 때 계산하고 restage 시 다시 구성한다. 생성량 또는 잔여 capacity가 0인 Spawn은 world/inverse matrix 계산 전에 종료한다. source RNG, 고정 step, spawn/event 순서는 유지했다.

차원술사 ALT V의 실제 309 emitters/4098 modules를 움직이는 임시 root/anchor에서 60Hz·1322프레임 평가했다. 전후 128,968 particle rows, peak 710과 평가 field hash `8493b70b4987e439`가 같았다. 같은 Debug /Od 단일 실행에서 CPU 시간은 3165.48→2567.41ms로 18.89% 줄었다. 실제 모델 본·GPU·Client FPS를 측정한 수치는 아니다. 기존 death/spawn/hidden-provider/MacroUV 19검사와 SpawnPerUnit 이동/비활성/재활성/삭제/restage 실패 보존 5검사도 전후 통과했다.

## G07. 검증·남은 경계

소스 반영과 focused Debug compile, 수치·protocol 검증은 실제 완료한 항목만 위에 기록했다. 작업 중 사용자가 Client와 Server를 실행한 동안은 제품 파일 교체 빌드를 보류했고 사용자 process를 종료하지 않았다. 두 process가 종료된 것을 확인한 뒤 최종 정본 Product Debug 빌드를 수행했다. 실제 실행/FPS와 Client/UI 화면 검증은 별도이며, Client/UI 자율 실행·조작·화면 캡처와 visual PASS를 수행하지 않았다.

추가 확인이 필요한 경계는 새 빌드의 class 변경/쿠크 진입 main peak, class 준비 중 빠른 변경·레벨 이탈, first effect readiness, particle fixed-step/spawn/update/frame rebuild, GPU Lights/NonBlend/SSAO, 새 UI.Runtime 계측이다. NPC/Monster/Valtan/Clown의 별도 cold presentation service는 이번 playable 여섯 class 변경과 구분한다. 첫 Kouku patternbindings 준비, main scene-object commit, 자원 최초 생성과 화면 밖 animation 평가도 새 하위 수치를 기준으로 후속 판단해야 한다.

이번 변경은 worker 수 증가만으로 모든 비용을 없애는 구현이 아니다. 계산량을 제거할 수 있는 반복 작업, main의 I/O·GPU 대기, worker stage/main commit을 먼저 정리했다. Chase–Lev/work stealing/fiber, animation·map 제출 병렬화, GPU particle 전체 전환은 구현 완료로 기록하지 않는다. source fixed-step/RNG/렌더 순서의 의미를 바꾸는 최적화도 적용하지 않았다.

근거 파일은 `out/LoadingFreeze20260911/capture_peak_analysis.json`, `dimensionmaster_steady_analysis.json`, `aggregate_run.log`, `scope_coverage.json`, `shader_world_stage_receipt.json`, `world_json_cost.json`, `root_compile/*.log`, `out/ProfilerFileManagement20260911/*`, `out/EffectWorkerReview20260911/*`에 있다. 대규모 혼합 dirty worktree의 다른 변경은 되돌리거나 자동 stage/commit/push하지 않았다.

Character 경계의 최신 8개 translation unit Debug focused compile과 main startup catalog 연결 compile은 통과했다. 추가 Model.cpp, Profiler.cpp/ProfilerTool.cpp/ProfilerCaptureIO.cpp, Picking.cpp/GameInstance.cpp, Shader와 WorldSequence 및 Effect worker 변경도 각 최신 focused compile을 통과했다. 계측 enum 43개가 반영된 실제 JSON 저장 검사도 재실행했다. 근거는 `out/CharacterSelectAsync20260911/authoring_compile/final_compile.log`, `verification/run.log`, `out/LoadingFreeze20260911/particle/`, `picking/picking_probe_receipt.json`과 `out/ProfilerFileManagement20260911/capture_io_output_eb6d8247ec6b47dd980823c10658ab1c/`에 있다. 제품 link·실행과 source compile의 완료 범위를 혼동하지 않는다.

2026-09-11 21:15 KST에 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`가 종료 0으로 완료됐다. Engine→Shared→Server→Client 네 프로젝트 모두 PASS, 컴파일·링크 error 0이며 SDK·Engine DLL·compiled shaders·runtime dependency 배포도 완료됐다. 총 650,981ms(약 10분 51초)였고 대부분 Client 셰이더/C++ 컴파일이었다. `Client/Bin/Debug/Client.exe` 생성 시각은 21:15:08, `Server/Bin/Debug/Server.exe`는 21:05:48이다. Engine/Public과 EngineSDK/Inc의 Profiler.h 및 Engine/Client 출력 Engine.dll의 SHA 일치를 확인했다. 기존 인코딩·shader·PDB warnings는 남아 있으므로 warning-free 빌드라고 부르지 않는다. 최종 `git diff --check`는 통과했다.

최종 제품 증거는 `out/BuildPipeline/runs/20260911T121509244Z-debug-product.json`과 `out/LoadingFreeze20260911/product_debug_final.log`다. 새로 요청된 호랑이/말·ALT V 통합도 같은 제품 빌드에 포함됐다. 종료 시 Client와 Server는 실행하지 않은 상태이며 사용자는 Visual Studio의 `Server + Client`, Debug x64에서 Ctrl+F5로 실행한다. Release 재빌드와 사용자의 실제 FPS 비교는 수행하지 않았다.
