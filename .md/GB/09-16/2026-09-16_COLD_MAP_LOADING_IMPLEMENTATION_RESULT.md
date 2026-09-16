# 전체 맵 최초 진입과 캐릭터 모델 준비 개선 결과

## G00. 현재 반영 상태

2026-09-16 22:33 KST Debug Product compile/deploy를 완료했다. 맵 admission 중복 경로 검증 제거, 실제 physical model별 bounded parallel 준비, 캐릭터 본체/추가 animation set/장비/무기 병렬 준비를 제품 코드에 연결했다. Resources 구조와 원본 asset/placement/material 데이터는 변경하지 않았다. 재진입 residency cache와 두 번째 모델 런타임은 추가하지 않았다.

`MapAssetCatalog.cpp`는 한 load transaction의 성공한 resolve/containment/regular-file 검사만 공유한다. nested shard는 같은 scope를 사용하고 다른 thread와 다음 load는 새로 검증한다. `Client.vcxproj`에서 이 파일과 `DataJson.cpp`, `RuntimeAssetRoot.cpp`만 Debug 최적화 및 PDB를 사용한다. 이미 존재하던 타 작업의 Engine/DeployPropObject 설정은 보존했다.

## G01. 준비·등록·취소 경계

새 `AssetPreparationBatch.h/.cpp`는 owner 포함 최대 4 worker와 고정 task index를 사용한다. D3D single-thread device는 1 worker다. 첫 실패 HRESULT/index를 반환하며 모든 child를 join한 뒤에만 호출자에게 돌아온다. protected handle 목록이 취소와 close를 직렬화한다. 중복 Run은 ERROR_BUSY이며 thread 생성 실패 시 owner와 생성된 child가 큐를 처리한다. H/CPP를 Client project와 기존 Loader filter에 등록했다.

Loader는 physical model path별로 catalog 순서를 보존해 base geometry와 material variant를 준비한다. 각 group은 독립 stage slot에만 기록한다. navigation까지 준비하고 성공·취소 여부를 확인한 뒤 기존 Add_Prototypes에 batch를 등록한다. 고정 phase 이름으로 진행률을 보고해 개별 모델마다 elapsed가 초기화되는 현상도 제거했다.

PlayableCharacterAssetService는 owner에서 immutable description을 수집하고 본체/animation set/장비/무기를 독립 준비한다. join 뒤 animation set을 authored 순서대로 붙이고 DimensionMaster palette 검증을 유지한다. prototype 순서, presentation 준비, generation 검사와 기존 commit 경계를 유지했다. Loader 초기 선택 class와 lazy async class가 이 경로를 함께 사용한다. Loader와 async job의 취소를 공통 batch child에도 전달했다. 기존 bounded 종료와 timeout process fail-fast는 유지한다.

## G02. 자동 검증과 측정

실제 installed catalog CPU 단계의 기존 `/Od`와 최종 dedup + 관련 3 TU `/O2` 비교다. 모델/GPU 생성 및 전체 진입 시간이 아니다.

| Area | 기존 | 변경 | 결과 |
|---|---:|---:|---|
| Bern | 49.873초 | 16.136초 | 전체 field digest 일치 |
| Character Select | 0.593초 | 0.197초 | 전체 field digest 일치 |
| Valtan | 11.368초 | 3.185초 | 전체 field digest 일치 |
| KoukuSaydon | 3.035초 | 1.002초 | 전체 field digest 일치 |

근거는 `out/MapColdLoadCpu20260916/summary.json`, `digest_fields.json`, `failure-*.log`다. 잘못된 재질·root 탈출·누락 모델·다음 load에서 삭제된 DDS를 거부하고, 실패 시 기존 catalog 보존과 복구 후 reload를 확인했다. nested/thread/next-load scope 격리도 통과했다.

Bern 첫 2,000 variants/226 geometry의 같은 process worker sweep은 1개 4.449초, 2개 2.531초, 4개 2.441초, 8개 3.194초, 16개 4.463초였다. 각 round의 대표 모델 signature가 일치했고 실패 0이었다. 파일 cache는 warm이며 실행 순서가 고정된 표본이다. 이 sweep의 실행기는 임시 probe이며 최종 공통 helper의 전체 맵 성능을 증명하지 않는다. 근거는 `out/MapColdLoad20260916/worker_sweep.log`다.

실제 공통 helper를 별도로 링크한 headless WARP 검사에서 empty/invalid, 96개 작업 각 1회, worker 상한, single-thread device, 첫 실패와 index, bad_alloc/일반 예외, 시작 전/중간 취소, 중복 Run 거부, 재사용 및 CancelSynchronousIo/CloseHandle 경합을 통과했다. 근거는 `out/AssetPreparationBatch20260916/probe_run.log`다. thread 생성 실패와 COM 초기화 실패의 강제 주입은 수행하지 않았다.

Warlord 실제 17개 모델의 직렬/공통 helper 병렬 결과는 bone 이름/parent/rest, clip 이름/duration/tick, bounds, material family/program/native constants/legacy SRV mask가 일치했다. 최초 표본 시간은 12.000초/20.458초로 병렬 쪽이 느렸으며 Product compile과 겹쳐 원인을 분리하지 못했다. 캐릭터 성능 개선으로 판정하지 않는다. 근거는 `out/PlayableCold20260916/warlord-{serial,parallel}.json`이다.

## G03. 제품 빌드와 남은 검증

실행 명령은 `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -BuildLogDirectory out/ColdLoadProductBuild20260916`다. 98.968초, PASS, 누락 runtime input 0이며 Client.exe와 Engine DLL/CSO 배포를 완료했다. 기존 코드 페이지/수치 변환/third-party PDB 경고가 있다. 근거는 `out/BuildPipeline/runs/20260916T133347581Z-debug-product.json`이다. Client project/filter XML parse와 변경 파일 `git diff --check`를 통과했다.

사용자가 22:34 KST 직접 Server/Client를 실행했다. 사용자 실측을 방해하지 않도록 추가 headless 부하 측정을 중단했다. 에이전트는 Client/UI를 실행·조작·캡처하지 않았다. 실제 네 맵 최초 진입·재진입·종료, class 변경 결과와 화면 판정은 아직 사용자 확인 전이다. 캐릭터의 30초가 얼마나 줄었는지, 최종 공통 helper의 모든 맵 전체 준비 시간 및 6개 class 성능은 미확정이다. 전체 진입 개선과 visual PASS로 기록하지 않는다.

기존 dirty worktree의 타 기능 변경은 보존했다. 이번 작업을 자동 stage/commit/push하지 않았다.

## G04. 이펙트 동시 준비 후속 소스 반영

후속 작업은 기존 변경이 저장된 `37cd95c4b`에서 시작했다. Loader가 level/model 준비와 Effect producer를 함께 시작하도록 연결했다. ActorCatalog의 lazy 초기화와 선택 캐릭터 입력 캡처는 owner에서 마친다. prototype registry를 쓰는 map/character 등록을 서로 다른 worker에서 동시에 실행하지 않는다. 두 producer가 종료한 뒤에만 Loader 성공을 알리며, 기존 5+5초 종료 제한 안에서 두 owner와 child I/O 취소를 처리한다.

공통 AssetPreparationBatch는 모든 instance의 실행 중 callback 합계를 최대 4개, 추가 child를 최대 3개로 제한한다. FIFO permit 대기, 취소, nested serial과 SINGLETHREADED device 독점을 적용한다. ACK 대기는 callback permit을 잡지 않는다. Engine CModel 내부의 기존 skinned-material threadpool(추가 최대 3개)은 이 Client helper 예산과 별개이므로 프로세스의 모든 thread 수가 4라는 뜻은 아니다.

Effect_LoadPreparationJob의 결과/ACK protocol은 유지한다. Effect_PresentationService가 최대 3개 target을 병렬 stage하고, 원래 FIFO 순서로 renderer candidate rebase → 결과 게시 → main commit → ACK를 처리한다. 최대 3개 미등록 후보와 1개 미ACK 결과만 유지한다. 완료 후보와 이전 자원 해제는 producer에서 수행한다. runtime lazy class도 정렬된 set 대신 실제 queue 앞 최대 3개를 캡처한다. 개별 target 실패는 격리하고 structural failure는 남은 owned FIFO를 순서대로 terminal 정산한다.

Effect_DocumentRenderer는 다른 target의 additive commit 뒤 worker가 현재 maps/session과 자기 후보를 다시 합칠 수 있게 했다. main commit의 정확한 generation 검사는 유지한다. full catalog 교체·authoring replacement·clear는 별도 replacement generation으로 후보를 무효화한다. 새 device/revision session의 최초 additive admission만 해당 window의 sibling이 따라갈 수 있게 구분하며, A→B→A로 돌아왔다는 이유로 옛 A 후보를 되살리지 않는다. Resources 데이터나 별도 모델 런타임을 추가하지 않았다.

## G05. Bern 사용자 시작 캡처와 반영 범위

기준은 `Client/Bin/ProfilerCaptures/profiler_20260916_224858_837_frame46_70436_0.json`이다. SHA256은 `ff2db109169d372c50eef5468e788b847a297cb0d464f9a95ea1f8a02b1a0569`이고, 46 CPU frame/drop 0과 42개 유효 GPU frame을 기록했다. 평균 interval 45.633ms(21.91fps), CPU 44.883ms, GPU interval 45.610ms다. NonBlend CPU 17.220ms/유효 GPU 27.158ms, Client.Update 16.348ms이며 부모·자식 inclusive 값은 더하지 않는다. GPU timestamp에도 CPU 공급 공백이 포함될 수 있다.

MapStaticBatchObject가 매 Render마다 232-byte 검증된 camera snapshot과 view/projection을 복사하던 경로를 즉시 소비하는 const view로 바꿨다. 기존 copy API, 실패 시 이전 출력 보존, 실제 camera 변경과 culling 입력은 유지한다. 화면에 그릴 객체·배치·재질·광원·Server tick은 줄이지 않았다. Particles/GeometryHelpers 두 TU는 캡처의 Sprite.InstanceBuild 2.719ms와 Particle.Render self 1.889ms에 연결된 Debug CPU 경로이므로 기존 hot-TU 정책의 `/O2 /Zi`, JMC/RTC/PCH 해제를 적용했다. `/MDd`, `_DEBUG`, `/fp:precise`와 Release 설정은 유지한다.

GPU NonBlend의 주 비용이 제거됐다는 증거는 없다. 기존 source BG pass는 opaque/masked가 같은 PS의 clip을 소비하므로 early-depth를 일괄 적용하지 않았다. 새 opaque 전용 shader/pass와 실제 occlusion/MRT parity 검사는 이번 변경에 포함하지 않았다. 실제 개선 FPS는 후속 사용자 캡처가 필요하다.

## G06. 후속 검증과 EXE 미반영 경계

- `out/ConcurrentPreparation20260916`: Loader/helper/Loading 실제 3TU 격리 컴파일, actual helper의 두 batch 합계 active 4/child 3, FIFO 대기, 중첩, single-thread exclusive, 대기 취소, 두 owner 실패/join 및 기존 실행기 경계 PASS.
- `out/EffectParallelPreparation20260916`: 실제 Effect 서비스/queue/job/helper 격리 컴파일과 3-slot window, 7-target FIFO/단일 ACK, producer 자원 해제, 개별 실패, stage·ACK 중 epoch 교체, 취소/예외/잘못된 ACK/structural failure 및 queue 우선순위·정산 PASS. resource stage와 renderer rebase는 이 protocol 검사에서 stub이므로 실제 이펙트 로딩/성능 증거는 아니다.
- `out/EffectParallel20260916`: 실제 renderer Catalog/CacheHelpers TU 격리 컴파일 PASS.
- `out/Bern2248Analysis20260916`: 실제 camera 함수 13조건, 정지/이동의 draw 순서 일치와 MapAssetRenderUtils/MapStaticBatchObject 격리 컴파일 PASS. renderer Rebase/Commit 실제 함수 기반 계약 검사에서 FIFO 병합과 기존 document/identity/resource 보존, stale 거부를 확인했다. 메모리 실패는 초기 throwable allocation 4지점만 확인했으며 모든 allocation 실패를 검증한 것은 아니다.
- 같은 RebaseContract의 최종 검사에는 새 revision에서 3개 병렬 후보를 순서대로 반영하고 replacement marker가 한 번만 바뀌는 조건, full catalog/개별 target 교체 거부, device/revision A→B→A의 옛 후보 거부도 포함한다.
- `out/EffectParticleMath20260916`: Particles/GeometryHelpers 실제 2TU `/O2` 격리 컴파일 PASS. 실제 helper와 header/native table의 `/Od`↔`/O2` 동일 process 비교에서 sprite 4,035건/clip 8,130건의 반환값·비유한값 분류와 수치 비교 PASS, 최대 절대차 `1.1921e-7`이었다. 7회 순서 교대 중앙값은 sprite 2,955개 `1.186725→0.541475ms`, clip 4,096개 `0.461158→0.088108ms`다. 이는 함수 CPU 표본이며 Bern FPS로 환산하지 않는다.

최초 RebaseContract 검사 프로그램의 전역 메모리 실패 주입에서 CRT 종료 팝업이 발생했다. 사용자가 본 `Bern…RebaseContract` 경로는 이 검사 프로그램과 일치하며, 당시 Client는 계속 실행 중이었다. 새 effect 병렬화는 제품에 배포되지 않았다. 이후 검사에는 CRT report/abort/Windows error UI 차단을 적용해 로그만 남겼다. 이를 Bern 프레임 드랍이나 실행 중 Client의 새 effect 병렬화 실패로 기록하지 않는다.

사용자가 Client를 계속 실행하며 **소스 검증만 마무리**하도록 명시했다. 따라서 G04 이후 Product 통합 빌드와 EXE/DLL 배포는 수행하지 않는다. 현재 `Client/Bin/Debug/Client.exe`는 22:33:45, Engine.dll은 22:32:25 빌드이며 이 후속 변경을 포함하지 않는다. 개별 컴파일 성공과 실행 중 EXE 적용을 구분한다. 사용자·다른 작업이 변경한 RenderingProfiles와 Kouku composition은 그대로 보존한다.
