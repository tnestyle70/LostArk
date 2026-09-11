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

Character Select는 최신 선택 의도를 유지하며 모델과 선택한 class의 Product Effect target 준비 뒤 같은 typed Server class-change command를 보낸다. 준비 중 기존 character의 이동·스킬 입력을 유지하는 후속 변경은 G08에 기록했다. Replication의 cold playable spawn/class replacement도 entity별 최신 snapshot을 보존한 채 준비를 기다리고 기존 transaction으로 교체한다. despawn/reset은 pending을 지우고 실패한 class를 매 snapshot마다 다시 로드하지 않는다. 기존 character는 실패·대기 동안 보존한다. 여섯 class 전체를 일괄 선로드하지 않는다.

SkillBindings/EffectCues/FaceSliders의 prepared 문서와 FaceMorph CPU 입력도 같은 generation의 모델과 함께 공개한다. FaceMorph immutable target 자료는 공유하고 instance별 weight 상태는 분리한다. 명시적인 Animation Tool Reload는 기존 검증을 유지하며 성공한 skill binding을 prepared snapshot에도 반영한다.

Loader::Initialize는 main에서 Server 승인 class와 immutable authoring 입력을 캡처한다. worker는 EffectCatalog 전역 컨테이너를 다시 읽지 않고 snapshot의 membership으로 EffectCues를 검증한다. PlayerSkillCatalog는 MainApp 초기화에서도 한 번 준비해 Release의 첫 진입이 Debug F1 도구 초기화에 의존하지 않게 했다. Loader에서 알 수 없는 class를 LanceMaster로 바꾸는 fallback도 제거했다.

취소 요청이 binary decode 하나를 즉시 중단시키지는 않는다. 실제 Warlord 9개 모델을 준비한 headless 검사에서 stale generation 결과는 19,618.9ms 뒤 거절됐고, 이미 준비된 결과의 취소·해제는 요청 뒤 101.63ms, 첫 body 준비 도중 취소는 요청 뒤 10,202ms가 걸렸다. 이 취소 완료 간격은 resource 준비·정리를 포함하므로 순수 WANM decode 시간으로 해석하지 않는다. 실제 단계별 후속 측정은 G09에 기록했다. 따라서 정상 레벨 전환은 main에서 worker를 join하지 않는다. MainApp이 전환 요청을 보존한 채 Cancel_AllAsyncPreparations를 요청하고 Has_ActivePreparations가 끝날 때까지 현재 프레임을 진행한다. pending 전환 중 새로운 class job은 시작하지 않으며 큰 payload 해제 뒤에만 drain 완료를 공개한다. process 종료의 bounded join/fail-fast 계약은 유지한다.

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

이 절의 제품 빌드 기록은 2026-09-11 21:15까지의 선행 작업 결과다. 이후 입력 유지·Registry·Material 변경의 소스/검증 상태와 현재 제품 빌드 경계는 G08~G10을 따른다.

소스 반영과 focused Debug compile, 수치·protocol 검증은 실제 완료한 항목만 위에 기록했다. 작업 중 사용자가 Client와 Server를 실행한 동안은 제품 파일 교체 빌드를 보류했고 사용자 process를 종료하지 않았다. 두 process가 종료된 것을 확인한 뒤 최종 정본 Product Debug 빌드를 수행했다. 실제 실행/FPS와 Client/UI 화면 검증은 별도이며, Client/UI 자율 실행·조작·화면 캡처와 visual PASS를 수행하지 않았다.

추가 확인이 필요한 경계는 새 빌드의 class 변경/쿠크 진입 main peak, class 준비 중 빠른 변경·레벨 이탈, first effect readiness, particle fixed-step/spawn/update/frame rebuild, GPU Lights/NonBlend/SSAO, 새 UI.Runtime 계측이다. NPC/Monster/Valtan/Clown의 별도 cold presentation service는 이번 playable 여섯 class 변경과 구분한다. 첫 Kouku patternbindings 준비, main scene-object commit, 자원 최초 생성과 화면 밖 animation 평가도 새 하위 수치를 기준으로 후속 판단해야 한다.

이번 변경은 worker 수 증가만으로 모든 비용을 없애는 구현이 아니다. 계산량을 제거할 수 있는 반복 작업, main의 I/O·GPU 대기, worker stage/main commit을 먼저 정리했다. Chase–Lev/work stealing/fiber, animation·map 제출 병렬화, GPU particle 전체 전환은 구현 완료로 기록하지 않는다. source fixed-step/RNG/렌더 순서의 의미를 바꾸는 최적화도 적용하지 않았다.

근거 파일은 `out/LoadingFreeze20260911/capture_peak_analysis.json`, `dimensionmaster_steady_analysis.json`, `aggregate_run.log`, `scope_coverage.json`, `shader_world_stage_receipt.json`, `world_json_cost.json`, `root_compile/*.log`, `out/ProfilerFileManagement20260911/*`, `out/EffectWorkerReview20260911/*`에 있다. 대규모 혼합 dirty worktree의 다른 변경은 되돌리거나 자동 stage/commit/push하지 않았다.

Character 경계의 최신 8개 translation unit Debug focused compile과 main startup catalog 연결 compile은 통과했다. 추가 Model.cpp, Profiler.cpp/ProfilerTool.cpp/ProfilerCaptureIO.cpp, Picking.cpp/GameInstance.cpp, Shader와 WorldSequence 및 Effect worker 변경도 각 최신 focused compile을 통과했다. 계측 enum 43개가 반영된 실제 JSON 저장 검사도 재실행했다. 근거는 `out/CharacterSelectAsync20260911/authoring_compile/final_compile.log`, `verification/run.log`, `out/LoadingFreeze20260911/particle/`, `picking/picking_probe_receipt.json`과 `out/ProfilerFileManagement20260911/capture_io_output_eb6d8247ec6b47dd980823c10658ab1c/`에 있다. 제품 link·실행과 source compile의 완료 범위를 혼동하지 않는다.

2026-09-11 21:15 KST에 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`가 종료 0으로 완료됐다. Engine→Shared→Server→Client 네 프로젝트 모두 PASS, 컴파일·링크 error 0이며 SDK·Engine DLL·compiled shaders·runtime dependency 배포도 완료됐다. 총 650,981ms(약 10분 51초)였고 대부분 Client 셰이더/C++ 컴파일이었다. `Client/Bin/Debug/Client.exe` 생성 시각은 21:15:08, `Server/Bin/Debug/Server.exe`는 21:05:48이다. Engine/Public과 EngineSDK/Inc의 Profiler.h 및 Engine/Client 출력 Engine.dll의 SHA 일치를 확인했다. 기존 인코딩·shader·PDB warnings는 남아 있으므로 warning-free 빌드라고 부르지 않는다. 최종 `git diff --check`는 통과했다.

최종 제품 증거는 `out/BuildPipeline/runs/20260911T121509244Z-debug-product.json`과 `out/LoadingFreeze20260911/product_debug_final.log`다. 새로 요청된 호랑이/말·ALT V 통합도 같은 제품 빌드에 포함됐다. 종료 시 Client와 Server는 실행하지 않은 상태이며 사용자는 Visual Studio의 `Server + Client`, Debug x64에서 Ctrl+F5로 실행한다. Release 재빌드와 사용자의 실제 FPS 비교는 수행하지 않았다.


## G08. 준비 중 입력 유지와 선택한 Effect 준비

후속 사용자 요청은 직업 준비가 약 30초 걸려도 기존 character의 UI·이동·스킬을 계속 사용하고, 준비가 끝난 뒤 Server 권위로 교체하는 것이다. `Level_CharacterSelect.h/.cpp`에서 로컬 준비 상태와 이미 Server에 변경을 보낸 상태를 분리했다. 기존의 넓은 `Is_ClassPresentationPreparationPending()`은 로컬 requested/worker까지 포함해 `Update_ServerArena()`가 준비 전체 동안 `PlayerController::Update()`를 건너뛰게 했다. 반대로 outbound class-change pending은 포함하지 않아 Server가 profile을 바꿀 수 있는 구간의 입력 경계도 불완전했다.

`Advance_ClassAssetPreparation()`은 기존 class 모델 준비 뒤 immutable prepared EffectCues의 선택 target을 `Queue_ProductCues_Priority()`로 등록한다. 요청 class와 catalog revision마다 한 번 등록하고 이후에는 그 target들의 완료 상태만 확인한다. 준비 중에는 정상 gameplay 입력을 처리하며, 모든 선택 target이 ready 또는 failed/unavailable의 terminal 격리 상태가 된 뒤 기존 `IPlayerCommandSink::Request_ChangeCharacterClass()`를 한 번 호출한다. unrelated background Effect queue는 이 선택의 완료 분모가 아니다. `Is_AuthoritativeClassReplacementPending()`은 outbound request, deferred snapshot, 실제 presentation commit 동안만 gameplay 입력을 막는다. 기존 authoring/modal/spawn의 별도 안전 경계와 최신 선택·취소·레벨 이탈 처리는 유지했다.

필수 prepared 모델/문서 묶음 자체가 없거나 queue 등록이 실패하면 로컬 선택 요청을 취소하고 기존 character와 입력, 원인 상태를 보존한다. optional `HasSkillBindings=false` 또는 `HasEffectCues=false`는 새 class 선택을 거부하지 않는다. 잘못된 Effect 문서는 빈 선택 target으로 처리하고 기존 post-snapshot 격리 경로가 경고를 보존한다. `CCharacter::Initialize()`도 `Load_ClipChains()`/`Load_EffectCues()` 실패로 spawn과 Server gameplay를 거부하지 않는 기존 계약이다. 개별 target 실패는 무한 입력 대기가 아니며, catalog에 없어 parser에서 격리된 Effect ID의 개수와 첫 ID, optional skill 오류를 class commit 경고에 함께 남긴다.

창술사 `LanceMaster.animevents`의 Product cue 43개와 고유 target 43개의 현재 catalog 등록·저작 문서 존재를 확인했다. ALT_V 34630은 `flm_sk_super_squalllance_01`에서 단일 `effect.lancemaster.skill.34630.full.restore`를 참조하고 V 34610의 세 clip cue도 유지한다. 말 model cue 변경만으로 다른 슬롯 문서가 사라졌다는 근거는 없었다. 이 검사는 파일·ID 연결 확인이며 사용자가 실행한 세션의 실제 스킬 표시 성공이나 말 변경의 무관함을 시각적으로 증명하지 않는다.

현재 실제 `Level_CharacterSelect.cpp`의 Debug focused compile은 PASS다. 실제 변경 함수 본문을 추출한 native 상태 검사에서 모델/Effect 준비 중 입력 유지, 준비 전 송신 없음, 단일 송신, 매 프레임 재등록 없음, revision 변경 재검증, terminal 실패 target 통과, optional skill/Effect 오류 통과, 필수 준비/등록 실패 시 기존 입력 보존, snapshot 대기·commit 이후 해제, 이전 worker 취소 뒤 최신 선택 유지와 레벨 이탈 취소가 PASS였다. Client/Server UI를 실행한 검사는 아니다. 근거는 `out/PR360ResourceMerge20260911/class_input/INPUT_PREPARATION_RESULT.md`, `input_transition_probe.cpp`, `input_transition_run.log`, `focused_compile/Level_CharacterSelect.log`, `lance_data_closure.json`이다.

## G09. Registry 잠금 분리와 Material 준비 병렬화

`ModelDecoderRegistry.cpp/.h`는 등록된 decoder 소유자를 유지한 채 짧은 mutex 구간에서 `const IModelDecoder*` 목록만 snapshot한다. 파일 probe와 `CanDecode()`/`Decode()`는 그 잠금 밖에서 실행한다. 등록은 append-only이므로 snapshot의 pointee 수명은 유지되고, decode의 가변 입력·출력은 각 호출이 소유한다. `Get_LastReport()`는 함수 지역 `thread_local MODEL_DECODE_REPORT`를 사용해 다른 worker의 실패가 현재 호출자의 진단을 덮지 않는다. 실제 Registry CPP를 연결한 두 thread 검사에서 동시 decoder 진입 최대 2, 각각 다른 오류 문자열과 호출하지 않은 main의 빈 report 보존을 확인했다. 이것은 잠금·진단 계약 검사이며 실제 모델 처리 시간이 두 배 빨라졌다는 뜻은 아니다.

`Material.cpp::LoadSharedTexture()`는 전체 texture 파일 준비·device 생성을 감싸던 map mutex를 lookup/entry 삽입에만 사용한다. 서로 다른 key는 동시에 준비할 수 있고 같은 key는 entry의 mutex로 최초 생성을 합친다. key의 device, 정규화 path, sRGB/forceLinear, 파일 크기·수정 시각과 weak resource 수명은 유지한다. 실패한 load는 성공 cache나 owner로 공개하지 않는다. 256회 삽입마다 수행하는 정리는 추가 owner가 없는 entry만 대상으로 하며 entry mutex의 `try_lock` 성공 뒤 expired 상태를 검사한다. 실제 함수 본문 기반 12검사는 서로 다른 key 병행, 동일 key 한 번 생성·같은 SRV, weak owner 해제 뒤 재로드, 오류/재시도·출력 보존, 520회 삽입 중 active entry 보존, key 분리/경로 정규화/파일 변경 무효화를 통과했다. actual Material CPP focused compile도 PASS다.

`Model.cpp::Ready_Materials()`는 skinned asset의 material이 3개 이상일 때 기존 `CMaterial::Create()`를 Windows default threadpool에 나눈다. 동시 모델 전체의 추가 callback 한도는 `min(3, 논리 processor 수 - 2)`이고 processor가 2개 이하이면 0이다. owner가 직접 수행하는 작업은 이 추가 callback 수와 별도다. 작은/static asset, `D3D11_CREATE_DEVICE_SINGLETHREADED`, worker 생성 불가 경로는 직렬로 처리한다. owner도 같은 atomic index 큐를 처리하고 각 작업은 독립 material slot만 쓴다. callback의 COM 초기화 실패는 owner가 남은 큐를 처리하며, 모든 작업이 종료되고 모든 slot이 성공했을 때만 material vector를 교체한다. 실패하면 staged 자원만 폐기한다. callback 계측 중 예외도 실패로 보존하고 RAII join으로 callback/permit이 남지 않게 정리한다. 내부 texture 준비 도중의 즉시 취소나 장비 모델 9개의 병렬화는 추가하지 않았다.

실제 `Ready_Materials()` 본문과 Windows threadpool을 사용하는 별도 CPU 검사에서 5개 동시 owner, 추가 worker 최대 3, owner help, slot 결과·순서, 직렬 분기, 실패 rollback, COM 실패 drain, callback 예외 뒤 permit 복구가 PASS였다. 근거는 `out/MaterialReadyConcurrency20260911/run.log`와 `out/MaterialTextureCache20260911/run.log`, `result.md`, `material_compile.log`다. 이번 구현은 새로운 Chase–Lev deque나 JobSystem/Scheduler를 추가한 것이 아니라 기존 모델 준비 안의 독립 material 작업을 제한적으로 나눈 것이다.

순수 Warlord WModel decode는 90,112,852-byte 입력의 193 clips/41,688 channels/5,046,624 keys를 처리했다. 실제 6개 decoder/reader CPP를 `/Od /MDd`로 컴파일한 wall time은 482.673ms였고 Animation 386.584ms, File 81.654ms였다. 기존 Engine Debug tlog의 `/ZI /JMC /RTC1`까지 맞춘 실행은 720.733ms였으며 geometry/skeleton/animation 해시와 count가 같았다. 기본 `/Od`의 clip별 median 1.793ms, P95 3.687ms, 최대 5.947ms로 한 clip이 수초를 차지하지 않았다. 따라서 이번에 WANM clip 병렬화는 적용하지 않았다.

## G10. 실제 Catalog WARP 비교와 최신 완료 경계

D3D11 WARP headless 환경에서 실제 `CActorCatalog::Build_ModelLoadDescription()`의 materialOverrides를 포함해 Warlord 본체·장비 6개·무기 2개와 animation set 2개를 생성했다. 생성된 모델을 유지해 texture 공유 수명을 보존했고 set은 기존 `Attach_AnimationSet()`으로 본체에 연결했다. Client, 창, swapchain, draw, 화면 캡처는 만들지 않았다. 초기 실제 DLL 실행은 생성 합계 29,521.7ms 중 `Model.Load.Materials` 28,139.9ms(95.3%), Decode 1,034.6ms였다. materialOverrides를 생략한 단순 body 생성 4,098.7ms는 제품과 다른 입력이므로 같은 성능 근거로 사용하지 않는다.

| 동일 Catalog 검사 | 기존 DLL + 재질 확인 | 변경 소스 + 기존 DLL |
|---|---:|---:|
| 11개 CModel 생성 합계 | 29,609.9ms | 21,064.0ms |
| 본체 CModel 생성 | 11,801.6ms | 7,268.89ms |
| 본체 Material 준비 | 10,714.4ms | 6,427.68ms |
| 최종 본체 clip 수 | 199 | 199 |
| mesh/material 확인 행 수 | 16 | 16 |

16개 행의 material 이름, 원본 material index, surface family, native program index와 대표 legacy SRV 존재 mask가 일치했다. 이는 자원 생성과 바인딩 입력 확인이며 모든 texture pixel이나 실제 화면 검사가 아니다. 변경본은 `Model.Load.MaterialWorker` 3개를 실제 기록했고 마지막 Join은 .0022ms였다. 전체 Material의 inclusive 합계는 여전히 20,055.2ms였다. 병행한 `Texture.Load.FileAndUpload`의 합계 37,158.3ms는 누적 작업 시간이므로 elapsed load 시간이나 Material 부모 scope에 더하지 않는다. 단일 material을 가진 장비/무기 8개는 여전히 모델별 직렬 준비다.

Before는 22:12:56 수정 시각의 기존 Engine DLL 복사본이고 After는 변경된 Model/Material과 비공개 Bone/Animation/Channel 5개 실제 TU를 기존 Debug tlog 옵션으로 컴파일해 그 DLL에 연결한 결과다. SDK/Profiler header와 import library/DLL은 같은 snapshot으로 고정했다. linker map에서 `CModel::Create()`가 새 local Model object, `CGameInstance::Get()`/`Get_Profiler()`가 기존 DLL import에 연결됐음을 확인했다. 이후 추가된 callback 예외·RAII join 정리 보강까지 이 시간으로 측정했다고 주장하지 않는다. CPU 동시 부하, 파일 cache와 혼합 링크의 code generation/COMDAT 차이를 통제하지 않았으므로 관측된 약 8.55초 감소를 최종 제품의 보장된 개선율로 사용하지 않는다.

검증 자료는 `out/PR360ResourceMerge20260911/model_decode/`의 `baseline_source.json`, `product_flags/flags_receipt.json`, `actual_dll/binary_receipt.json`, `actual_dll/catalog_warlord_materials.json`, `after_local/source_compile_receipt.json`, `after_local/probe.map`, `after_local/comparison.json`, `after_local/AFTER_RESULT.md`에 보존했다. Registry 검사는 같은 경로의 `registry/run.log`다. out의 임시 실행 파일만 실행했고 EngineSDK나 제품 출력은 이 검사에서 쓰지 않았다.

이번 후속 변경은 소스 반영·focused compile·위 수치/실패 경계 검증과 정본 Product Debug 통합 빌드·배포까지 완료했다. 최종 빌드는 2026-09-11 23:23:36 KST에 PASS했으며 총 867,183ms였다. Engine 124,818ms, Shared 1,342ms, Server 26,404ms, Client 714,496ms의 네 프로젝트가 모두 PASS이고 `missingRuntimeInputs`는 빈 배열이다. 완료 receipt는 `out/BuildPipeline/runs/20260911T142336444Z-debug-product.json`, 로그는 `out/PR360ResourceMerge20260911/product_debug.log`다. 이는 위 G07의 21:15 선행 빌드와 구분되는 후속 변경의 최종 제품 빌드다.

최종 출력은 `Client/Bin/Debug/Client.exe` 54,017,536 bytes(23:23:36), Engine DLL 8,583,168 bytes(23:11:14), `Server/Bin/Debug/Server.exe` 13,252,608 bytes(23:11:41)다. Engine/Client 출력 Engine.dll의 SHA256은 `E59B1122866CFBBA0DB633033337960765F6C3E4370134A7878808010A3AABE4`로 일치하고 Engine/Public과 EngineSDK/Inc의 Profiler.h는 `9C63E97342F8E73D92754E18C7877BA11970FD289ECA3DD82B8B3F5B78B3D014`로 일치한다. 새 제품 DLL로 같은 Catalog의 성능을 재측정하지 않았으므로 위 혼합 링크 결과를 최종 DLL 실측으로 바꾸지 않는다. 사용자가 직접 수행하는 Character Select 준비 중 이동·스킬/빠른 재선택/Server commit 이후 입력, 실제 준비 시간과 FPS, 창술사 슬롯별 이펙트 표시와 Client/UI 화면 확인은 남아 있다.

사용자의 최신 순서는 먼저 버그 수정 EXE를 직접 검증한 뒤 PR merge와 리소스 공유를 진행하는 것이다. 따라서 현재 완료 범위는 빌드·검증 준비까지이며 사용자 확인 이전의 merge/main 동기화와 리소스 공유는 보류한다. Client/UI 자율 실행·조작·캡처 또는 visual PASS는 수행하지 않았다. 별도 Profiler 계측 최적화의 변경·검증은 해당 담당 문서가 소유한다.
