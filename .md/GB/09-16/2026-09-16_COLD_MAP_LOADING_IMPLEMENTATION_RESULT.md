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
