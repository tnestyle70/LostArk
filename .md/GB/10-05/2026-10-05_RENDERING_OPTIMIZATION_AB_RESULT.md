# Rendering Workbench 최적화 A/B 구현 결과

## G00. 구현한 비교 경로

사용자가 요청한 최적화 비교를 기존 RenderingBenchmark의 session·Original/A/B·warmup·ABBA·조건 검사·JSON 저장에 연결했다. Debug는 F1 Rendering Workbench의 `최적화 A/B`, Release는 F1 `Optimization Benchmark`를 사용한다. Release에 일반 Profiler/F7·FPS overlay·저작 Save/Publish를 추가하지 않았다. 최적화12개를 각각 OFF/ON으로 비교하고 원래 설정으로 복원한다. 한 번에 모든 최적화를 끄는 과거 버전 복원이나 load-time 구조 변경은 제공하지 않는다.

| 항목 | A OFF의 실제 의미 | B ON의 실제 의미 |
|---|---|---|
| 프러스텀 | Layer final-camera BVH와 snapshot 기반 상세 가시성의 frustum 제외 우회 | 기존 지원 객체의 frustum 제외 |
| Occlusion | Bern 정적 NONBLEND 가림 제외 우회 | 검증된 opaque geometry의 CPU occlusion 및 cache |
| Distance | 작은 소품의 거리+pixel 제외 우회 | 현재 거리 scale/pixel 정책 사용 |
| Mesh LOD | 원본 index 선택 | 이미 생성된 화면 오차 LOD 선택 |
| 맵 instancing | 같은 buffer/payload/LOD를 instanceCount1로 각각 color/shadow 제출 | 기존 instanced 제출과 지원되는 추가 병합 |
| 동일 모델·재질 병합 | 인접 exact batch 병합 우회 | geometry·전체 material·profile·LOD 호환 병합 |
| Lighting bank | RNM bank 병합과 exact prefix handoff 우회 | 고유 bundle8개 이하의 원본 조명 보존 병합 |
| 정적 shadow cache | 정적 caster도 매번 그림자 계산 | 같은 caster 결과 재사용 |
| NPC pose reuse | opt-in animation 표본을 개별 계산 | 같은 clip/track 표본 재사용 |
| Particle root cache | 각 요청의 역행렬 계산 | 같은 root의 역행렬 재사용 |
| 맵 worker | owner에서 가시성·payload 계산 | 기존 coarse CPU job pool에 분배 |
| Particle worker | 기존 병렬 적격 묶음을 caller에서 계산 | 기존 적격 조건·최대1helper 유지 |

그림자 자체·원본 재질·상위 instancing 같은 전제 조건이 꺼진 항목은 이유를 표시한다. 실험을 위해 다른 품질값을 자동 변경하지 않는다. 맵 worker의 기본OFF도 유지한다. 프러스텀 스위치는 엔진의 모든 임의 culling call이나 authored Visible/Suppressed를 제거하는 전역 옵션이 아니다. instancing OFF는 객체 생성·32m 분할·asset 정렬 비용을 과거 구조로 되돌리지 않는다.

## G01. cache와 소유권

기존49개 field 뒤에12개 boolean field를 추가해61개와64bit mask를 사용한다. Engine의9개 RENDER_OPTIMIZATION_SETTINGS는 모두 기존 동작과 같은 true 기본값이며 기존 MAP_VISIBILITY_SETTINGS의3개 스위치를 재사용한다. 같은 설정 Apply는 revision·cache를 바꾸지 않는다.

ProfileService는 품질의 매 프레임 restore/apply와 분리한 base/lastApplied/ownedMask를 보관한다. flags-only 세션은 quality/shadow/fog setter를 호출하지 않는다. 외부 제어가 소유 필드를 바꾸면 비교를 해제하고 외부값과 미선택 필드를 보존한다. 두 setter의 일부 실패·rollback 실패·복원 실패에는 실제 현재값과 복원 정보를 남겨 재시도한다. 성공한 새 profile은 유지하면서 남은 최적화 복원을 다음 update에 재시도할 수 있다.

일반 닫기는 복원하며 `영상 촬영: F1로 숨겨도 최적화 A/B 화면 유지`를 켠 세션만 숨겨도 유지한다. 종료·취소·scene/region/Video 소유권 변경의 복원은 유지한다. MainApp은 Engine 해제 전에 Benchmark Shutdown을 호출한다. 영구적인 renderer 실패나 프로세스 강제 종료까지 복원 성공을 보장하는 계약은 아니며 저작 파일은 수정하지 않는다.

## G02. 결과의 의미와 증거

고정 카메라에서 A1/B1/B2/A2를 측정한다. 최적화는 Engine/NPC/particle update 뒤에 적용되는 frame 경계를 고려해 최소2frame warmup을 요구하며 기본60frame이다. 기존 품질 실험의0frame 허용은 유지한다. CPU·interval·GPU 분포와 실제 draw/indices/IA/VS, 전체 named counter, CPU work ledger, pass 통계를 보관한다. Animation sample requests/reuse, particle inverse requests/reuse, particle caller/worker/assistant의7개 실제 생산자 counter를 추가했다. 파티클 inner loop는 local 누적 후 scope 종료에만 profiler에 반영한다.

각 단계가 끝나면 그 단계의 정확한 frame 범위를 기존 LostArkProfilerCapture.v3로 저장한다. snapshot 상한64MiB이며 저장이 끝난 뒤 다음 warmup을 시작한다. 요약은 measurement ID·반복 번호·raw 경로/성공 여부·frame 범위를 함께 저장한다.4단계 raw 저장이 모두 성공하지 않거나 조건/적용값이 다르면 완료된 ABBA 이득으로 표시하지 않는다. 취소된 결과와 저장 실패도 남긴다. 종료 시 완료한 결과의 마지막 요약을 저장하고 아직 확인하지 못한 raw는 미완료로 표기한다.

비교 대상 field만 공통 조건에서 제외한다. 나머지 최적화·거리 수치·camera·viewport·scene/region·build/iterator debug·D3D device flags·adapter·FPS 제한과 도구 표시 상태를 비교한다. 대상0과 warm cache 상태를 구분하고, 특히 정지 맵 worker의 준비/worker 작업0을 성능 이득의 증거로 사용하지 않는다. 부모 CPU scope와 자식, worker/dispatch/join을 합산하지 않는다. GPU pending/invalid는0ms가 아니다.

이 측정은 움직이는 컷신·NPC·환경 이펙트 시계를 결정적으로 재생하는 benchmark가 아니다. 컷신의 고정 시점 비교와 이동 관찰 capture를 구분한다. 현재 GUI·원본 프레임 검증을 Client에서 자율 실행하지 않았고 새 Bern FPS 개선율은 측정하지 않았다.

## G03. 즉시 비교와 재준비 경계

12개 runtime 선택은 같은 프로세스에서 A/B 가능하다. Debug↔Release와 D3D debug device는 각각 별도 실행이 필요하다.32m 분할·material cohort 정렬·geometry/atlas bake·LOD 생성 하한·파생 cache cold/warm은 맵 준비 단계이며 이 표에 가짜 실시간 스위치를 만들지 않았다. 재진입 시 prototype/cache가 재사용되면 새 준비를 보장하지 않으므로 해당 준비 경로의 명시적 무효화 또는 별도 실행이 필요하다. 프로세스 재시작만으로 OS 파일 cache cold를 보장하지 않는다. 큰 atlas는 기본OFF와 미검증 범위를 유지한다.

## G04. 완료한 함수·경로 검증

Runtime 실제 함수 fixture는 Debug/Release 각각254,101 assertions PASS였다. bank+color/shadow155,720, actual Layer+CpuJobPool69,714, settings/LOD/animation/inverse/wrapper28,034, camera/Stage/Compute/Upload/frustum633이다. count1 제출의 원본224byte payload/LOD·claim, bank OFF handoff, frustum/거리 독립, cache revision·Map 실패 rollback, NPC clock/pose와1,000개 TRS·nested/4thread inverse 동등성을 확인했다. Engine/Client11CPP 양 빌드 최소 컴파일도 PASS다. GPU·객체 dependency 대역과 실제 함수 경계는 `out/RenderingOptimizationRuntime20261005/verification-receipt.json`에 기록했다.

ProfileService의 수정하지 않은 production helper/method와 실제 public header를 소비한 renderer boundary fixture는 양 빌드 각각3,948검사 PASS였다.12개 각각300frame 동안 setter/revision 추가변경0, 원래 worker OFF 복원, field ownership 변경·외부 수정·level/profile/region/video 전환·setter/rollback/restore 실패와 다음frame 재시도를 확인했다. 실제 Service CPP 양 빌드 최소 컴파일도 PASS다. `out/OptimizationAB20261005/service-validation.json`에 source hash를 보관했다.

MainApp은 Debug/Release 최소 컴파일과 구성별13개 source/macro route 검사를 통과했다. 새 Benchmark 단일 생성·Update·복원·device 입력·Engine 이전 Shutdown, 기존 F7/FPS/저작 경계 보존을 확인했다. `out/RenderingOptimizationMainApp20261005/compile-results.json`, `route-checks.json`이다. 이 검사는 실제 버튼 조작·화면 판정이 아니다.

Benchmark와 실제 Service 함수의 session/Prepare/Begin/Update/Finalize/ABBA cost/JSON/Shutdown fixture는 Debug/Release 각각835검사를 통과했다. JSON·인코딩·hash26검사도 PASS였다. 최소2frame warmup, 조건 변경·GPU pending/invalid·raw 비동기 완료 전환·미완성ABBA의 delta=null·new counter 이름/합계/분모·cached coverage·최종 요약 저장을 확인했다. renderer/user/profiler delivery와 raw exporter completion은 대역이고 실제 summary writer 파일을 기록·파싱했다. 실제 Client/UI/GPU를 실행한 검사가 아니다. `out/RenderingOptimizationAB20261005/benchmark/receipt.json`에 기록했다.

첫 제품 Debug 빌드는 기존 ProfilerTool의 독립 COUNTER_LABELS 표에서7개 신규 이름 누락을 static_assert로 발견해 실패했다.122개 기존 순서 뒤7개를 추가했고129개 enum/표가 일치한다. 전체 source에서 다른 고정표 누락이 없는지 확인했으며 ProfilerTool CPP Debug/Release 최소 컴파일·diff check PASS다. `out/RenderingOptimizationRuntime20261005/profiler-tool-consumer-fix.json`과 첫 실패 `out/BuildPipeline/runs/20261005T001829736Z-debug-product.json`을 보존했다. raw/summary 검토에서는 도구 표시 조건·4단계 raw 성공·중단된ABBA delta·종료 직전 대기 요약 누락을 보완했다.

## G05. 최종 제품 빌드와 완료 경계

최종 소스92개를 고정한 뒤 정상 Product runner로 Engine→Shared→Server→Client를 컴파일·링크·배포했다. Debug는28,567ms, Release는217,603ms로 모두 PASS다. 최종 Debug는 앞선 빌드의 유효 산출물을 재사용하고 Client4OBJ/2binary를 갱신했으며 Release는Client224OBJ/2binary를 갱신했다. 전체 TU가 두 번 모두 신규 컴파일됐다는 뜻은 아니다. Clean/Rebuild·tracking 삭제·새 Client/UI 실행 없이 기존 증분 빌드 계약을 사용했다.

- Debug: `out/BuildPipeline/runs/20261005T002159035Z-debug-product.json`
- Release: `out/BuildPipeline/runs/20261005T002658238Z-release-product.json`
- 통합 검증: `out/OptimizationAB20261005/final-validation.json`
- 입력 고정: `out/OptimizationAB20261005/product-source-freeze.json`

92개 소스 hash는 최종 두 빌드 전후 같았다. project/filter4개 XML, BuildDomains JSON과6개 검증 영수증 parse, final Service/Benchmark fixture source hash 일치와 git diff --check를 확인했다. 기존 코드 페이지·변환·외부 라이브러리 경고는 남고 최종 오류는0이다. rendering/Data/Resources publisher는 실행하지 않았다. AGENTS·CLAUDE·팀 사용서는 Release benchmark 진입과 세션 소유권의 바뀐 public 계약만 갱신했다.

구현·함수 검사·제품 빌드는 완료했다. 사용자 화면·영상 촬영·실제 Bern의 새 A/B FPS 수치는 아직 측정하지 않았다. 컷신 deterministic replay, 준비 단계 chunk/atlas/LOD-generation의 실시간 A/B, 모든 최적화를 한 번에 끄는 전체 묶음 비교도 이번 완료 범위가 아니다. 이 도구가 저장하는 실제 사용자 run을 근거로 각 기법의 순이득·적용률·CPU/GPU 병목을 판정한다. 무관한 미커밋 변경과 source 파일 인코딩을 보존했고 일괄 stage·commit·push는 하지 않았다.
