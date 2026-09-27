# 셰이더 컴파일 단위 분리 결과

## G00. 적용

기존41개 SourceCharacter program(1..32,80..88)을6묶음으로 나눴다. AnimMeshBinary·MeshBinary·Deferred가 각 묶음의 독립 CSO를 사용하며 CShader는 실제 g_SourceCharacterProgram 값으로 선택한다. 기본 CSO의 map forward33..65와 기존 material/alpha/Light pass를 유지한다. 별도 CModel 런타임은 만들지 않았다.

Effect scene color/depth 선언을 Cube/Slice 계산 include에서 분리했다. source generator는 원본 전체 텍스트 재구성과 무변경 write0를 지원하고 project/filter/SDK배포가 새 CSO를 포함한다. CShader binding revision을 공유해 clone의 상태·상수·SRV·bone 배열이 실제 선택 CSO에 전달되며 무변경 Begin의 전체 동기화를 건너뛴다.

## G01. 검증

Engine/Client 정규 FX 컴파일과 Engine Build/link PASS. headless WARP에서123program,6clone,9실패·reload보존,492Light pass 바인딩 검사 PASS(window0/draw0). const/texture/512bone 배열을 실제 GPU binding으로 대조했다. 생성기 roundtrip/no-op/단일leaf변경/미등록ID거절 테스트3개 PASS.

| timestamp를 갱신한 입력 | 재생성 CSO | Engine+Client FX target 경과 |
|---|---:|---:|
| 무변경 | 0 |0.583초|
| Base group009 |2|39.578초|
| Light group009 |2|87.121초|
| 공유 source 입력 |21|413.411초|
| 마지막 무변경 |0|0.604초|

위 시간은 파일 내용 변경 없이 해당 include의 수정 시각을 갱신한 실제 증분 무효화 검사다. 재질 수식 변경이 자기 leaf만 바꾸는지는 별도 생성기 테스트로 검증했다.

Light009의 두 출력은 Deferred와 투명머리카락 Light18을 실제 소비하는 AnimMesh이다. 캐릭터 공용입력 변경은 모든group에영향을주므로 전체재생성이정상이다. 동일입력의분리전수정빌드A/B는없으며, 이시간을기존전체빌드대비단축률로표현하지않는다.

CPU microbench bind+Begin은같은group무변경0.250→0.270µs, group전환무변경0.335→1.262µs,512bone+상수갱신5.063→6.279µs,갱신+group전환5.110→6.447µs다. 프레임FPS나화면동일성검증이아니다. 근거는out/ShaderProgramIsolation20260914/{incremental-measurements.json,warp/probe.log}.

## G02. 보류 범위

Character Select 추가 source material의 steady emission·metallic diffuse mask·masked/바람은조사중이었으며사용자중간마무리요청으로제품미적용이다. 현재변경은컴파일분리와실제기존소비자연결이다. 최종통합Product결과는캐릭터/아레나통합RESULT G06에기록한다.

## G03. 추가 컴파일 최적화 구현 — 2026-09-17

main `bddacace2` 동기화와 Loading 누락 선언 보완 후 Debug Product는 2,290,594ms에 성공했다.
Client 단계는 2,249,173ms, OBJ 220개·CSO 109개 갱신이었다. 이 성공본의 CSO 141개
(986,782,990 bytes)를 `out/ShaderBuildOptimization20260917/baseline/Client`에 보존했다.
이는 이번 셰이더 최적화 전 기준이며, 캐시 없는 동일 입력 A/B 빌드 시간은 아니다.

`codex/shader-build-isolation-20260917`에서 다음 소스를 반영했다.

- Native ModelCue pass 7/8/12/13은 `ProgramVariantPass` annotation으로 기본 FX가 소유한다.
  CShader가 정책을 검증하고 stale SourceCharacter selector가 있어도 기본 native PS를 선택한다.
  SourceGroup 6개는 같은 uniform/resource/default를 유지하고 native 함수·PS 컴파일을 제외한다.
  해당 그룹에서 native pass를 직접 호출하면 실패한다. pass 14개와 input signature·변수 타입
  검사를 유지하며 표면 85~89, 광원·반투명 pass도 보존한다.
- Artist facade를 Inputs/Common/Programs로 분리했다. 기존 27개 native 그룹과 53개
  Mesh/Particle wrapper는 자기 SelectedGroup만 포함한다. 공통 carrier의 가변 최대 ID 대신
  실제 guarded case 목록을 사용하며, 여러 그룹을 소비하는 Decal/Trail/ScreenPost는 전체 목록을
  유지한다. Kouku·Vehicle installer와 기존 전체 교체 generator도 같은 expand/write 경로를 쓴다.
- 일반 shader owner 11개와 V2 common/소비자 6개의 같은 compile 식을 공유했다. 기본 owner와
  V2 확장 기준 170개, Deferred/MeshBinary의 SourceGroup 6개씩을 포함하면 344개를 줄인다.
  AnimMeshBinary의 반투명 pass 9/10 공유는 이 344개와 별도다. 함수·pass 상태·순서는 유지한다.
- 새 HLSLI 30개를 기존 Client project/filter의 None 항목으로 등록했다. 신규 FX와 C++ 파일은 없다.
  Resources 구조·원본 리소스와 제품 재질 수식은 바꾸지 않았다.

## G04. 소스 검증과 통합 빌드 대기

Artist 전체 authoring 텍스트의 펼친 결과는 기존 bytes와 같다
(`057db8686ad15ac2ea713f894b77260bc98c985f1dd090cef5b04a166ba5e6a4`).
일반/V2 공유 변경 18개 파일은 공유 선언을 원래 compile 식으로 되돌리면 기준 원본 bytes와 같다.
근거는 `out/ShaderBuildOptimization20260917/pass-sharing/reversible-proof.json`과
`variant-structural-check.json`이다. 원본 보존 검사는 실제 컴파일·실행 검증과 구분한다.

`test_artist_shader_input_isolation.py` 6개와 기존 `test_source_character_program_groups.py` 3개가
PASS다. roundtrip, 내용·수정시각 no-op, 그룹 추가·case 수정의 제한된 변경 범위, carrier guard,
legacy full-reset, 잘못된 selector의 저장 전 거부를 검사했다. 변경 generator 4개 Python 문법,
Client project/filter XML parse 및 diff check도 통과했다. 기존 headless CShader probe에는 native
기본 FX 선택·clone 공유·상수/texture/bones 144건과 직접 그룹 호출 거부 8건을 추가했다.

00:36 KST 최적화 후 Product를 시도했으나 실행 중인 Debug Client PID 2756과 Server PID 18308을
`ProductOutputGuard.psm1`이 발견해 컴파일 전에 중단했다.
근거는 `out/BuildPipeline/runs/20260916T153644949Z-debug-product.json`이다.
현재 실행 EXE와 CSO는 G03의 최적화 전 성공본이다. 에이전트는 프로세스를 종료하거나 실행하지
않았고 사용자에게 저장·종료 확인을 요청했다. 최적화 후 FXC/Product, CShader probe, 컴파일된
shader closure·bytecode 비교와 실제 증분 시간 측정은 아직 실행하지 않았다.

## G09. 09-17 Effect V1 우선 요청에 따른 후속 분할 보류

후속 조사에서 기존 MSBuild 변경 감지는 131개 중 변경된7개만 선택했고, 느린 지점은 base animated FX와 큰 native family compile임을 확인했다. PLAN G09~G11의 granular composite runtime과 family별 추가 분할은 사용자 요청으로 중단했다. 시험했던 Shader.h/.cpp 추가분만 사전 byte snapshot으로 복원했으며 이전 shader isolation 변경은 보존했다. 백업은 out/ShaderCompositeDesign20260917/paused-composite에 있다. 이번 후속 작업에서 새 generator/MSBuild 변경이나 정상 Product 빌드는 수행하지 않았다. 작은 metadata-only FX 시험만 out에서 수행했다. 이 후속 분할 최적화는 구현 완료가 아니다. 현재 우선 작업은 세이튼 Effect V1 재생·편집 오류 수정이다.

## G12. 09-27 재개 기준과 전체 입력 조사

사용자가 전체 셰이더·C++ 빌드 병목 조사·수정을 다시 요청했다. 기존 기능의 정식 Debug
`20260926T215804868Z-debug-product.json`과 Release
`20260926T223356797Z-release-product.json`은 PASS이며 각각33분07.884초/34분48.111초다.
큰 비용은 공통 dispatcher 등록으로 다시 컴파일한 Engine24개+Client48개 FX다.
Client shaderWrites72 중24개는 Engine 배포 복사다. 기존 C++ PCH/증분 및 FXC4-worker는
유지되며, 이 성공본의72개 CSO bytes는 Debug/Release가 같고71,904,148bytes를
`out/BuildOptimization20260927/baseline`에 독립 복사했다.

MSBuild의 실제 `/getItem:FxCompile` 평가에서 Engine25개/Client208개 active target을
확인했다. 모든 target의 현재 FXC.read 의존성과 출력 크기는
`out/BernValtan20260927/shader-inventory.json`에 기록했다. 가장 큰 입력 묶음은 native
ScreenPost36.1MB,Decal20.7MB,Trail18.4MB이며 기존 Debug는 해당 FX의 최적화를 끈 설정이다.
이 크기는 compiler 시간 측정이 아니며 이번 원래 기능 빌드에서는 이 세 FX가 바뀌지 않았다.
runtime에서 실제 여러 family를 쓰는 owner이므로 단순히 include를 삭제할 수 없다.

현재 원인 확정 범위는 공통 SourceCharacter 등록의72개 FX 전파,큰084/1088 묶음,
비0 Static의 사용하지 않는 Movie 함수 파싱 입력이다. 전체 CPU 사용률이 낮은 관측은
별도로 남겼지만 전원/EcoQoS/affinity에서 강제 제한을 확인하지 못했으므로 원인을 단정하지
않는다. 후속 후보와 실제 반영·검증은 이어지는 G에서 구분한다.

## G13. 그룹 내부 dispatch leaf 적용과 전처리 검증

기존 Base/Light Group leaf에 함수 본문/dispatch case 모드를 함께 두고 공통 facade는
group include만 유지하도록 반영했다. Engine/Client96개 HLSLI와 writer/test2개를 최신
hash 재확인·백업·원자적 교체로 적용했다. 새 HLSLI와 runtime C++는 추가하지 않았다.
원본 전체 expand bytes가 같으며 actual Vehicle install helper로1527 등록 시 Base1472와
Light1472 leaf만 바뀌고 공통 facade와 무관한 파일의 mtime은 유지된다. 수식 수정은
해당 stage의 leaf1개만 바뀐다.

관련 unit7개,기본/미정의/23개 등록그룹/unknown selector의 Base·Light52개 실제 x64 FXC
`/P` 전처리 토큰 동치가 PASS다. `out/SourceDispatchLeaf20260927`의 apply-report,
candidate-report,fxc-preprocess-report가 근거다. 이 단계는 전처리·쓰기 범위의 검증이며
후속 큰 그룹 분할과 최종 제품 컴파일·GPU ABI 검증은 별도로 진행한다.

## G14. 비0 Static의 map/movie 함수 파싱 제외

Engine/Client SourceMapForwardPrograms와 Client SourceMapDirectPrograms/WaterPrograms의
함수 본문만 group0 조건으로 제한했다. 전역 Texture/Sampler/cbuffer/상수 선언은 기존 위치와
기본값을 유지한다. 원문에서 추가 guard만 제거하면 bytes가 같고 group0의 실제 FXC `/P`
전체 토큰이 같다.084/1088 그룹은 도달하지 않는212개 함수만 빠지며 남은 함수·전역 선언·
shader object·technique/pass 토큰과 순서가 같다.

actual `/P` 출력은084에서4,394,854→1,890,510bytes,1088에서5,692,171→3,187,827bytes다.
전처리 크기 감소를 컴파일 시간 단축으로 대신 보고하지 않는다. 근거는
`out/ShaderCompileAudit20260927/static-map-guards`의 source-parity와
fxc-preprocess/token-parity JSON이다. 최종 CSO·Effects11 ABI 검증은 제품 빌드 뒤 기록한다.

## G15. 큰 그룹 분할 반영과 독립 검토

기존84~112 및1088~1151의2그룹을7그룹으로 나눴다. 전체23→28개 그룹이며 Anim/Static/
Deferred의 독립 CSO producer는15개 추가된다. registry가 소유한120개 추가 stable ID는
같고 모든 range 합집합713값과 비중첩 조건을 보존했다. Base/Light 전체 함수·case 원문은
직전 G13과 bytes가 같다. CShader runtime 구현은 바꾸지 않았다.

등록 helper의 범위 정책,wrapper·project·filter·배포와 Engine/Client mirror를 함께
갱신했다. Python11개,모든28개 그룹/기본/unknown의60개 FXC `/P` 결과 검사,PASS다.
독립 검토 중 FXC가 추가한 공백 때문에 검사 정규식이 빈 집합을 허용하던 fixture 오류를
발견했다. 공백 허용과 nonempty/count guard를 추가하고 같은 실제 `/P` 파일을 다시 검사해
Base/Light 각256개 program을 확인했다.7개 분할 그룹의 함수/case 수는 각각
12/12/5/4/16/16/16이며 expected ID와 같다. 빈 집합 PASS를 최종 근거로 사용하지 않는다.

독립 검토는 registry·ghost84 분기·XML476개 항목·신규40개 metadata·재등록 staged0까지
확인했다. 근거는 `out/SourceGroupGranularity20260927`과
`out/CppBuildAudit20260927/g14-independent-review.json`이다.

기준의 실제 CShader probe는 old72개 CSO와 보존 Engine DLL로 통과했다. 영향81ID×3owner의
243개 routing/clone/fallback,기존126program/6clone/9missing-corrupt 실패보존/504lightpass/
144native/10unavailable/46numeric draw를 검사했다. WARP의 창 없는 GPU·ABI 검사이며
Client 화면 판정은 아니다. 기준 owner Create는 Anim1019.090ms/Static692.635ms/
Deferred1150.917ms의 각1회 표본이다. 변경본의 정식 Product 재실행·크기·load 비교를
확인한 후 최종 완료 상태를 덧붙인다.

## G16. 이번 마무리에서 보류한 ScreenPost 후보

사용자가 현재 범위를 마무리하고 다음 방향을 다시 정하자고 요청하여 PLAN G15의
ScreenPost 추가 축소는 제품에 적용하지 않았다. Artist/ALTV의2개 HLSLI는 out 후보뿐이며
임시 helper의 SCREEN_POST 처리2곳3줄도 정확히 회수했다. SourceCharacter G13/G15와
정적 map G14 변경은 유지한다. `large-effects/parked-status.json`에 경계를 기록했다.

후보는 도달하지 않는204개 함수를 제외하고 ALTV155/156과 모든 declaration/pass·retained
body를 보존하는 실제 `/P`/generator roundtrip을 통과했다. 다만 제품 FXC·CSO·runtime
검증은 하지 않았다. Decal/Trail은 실제 전처리에서 미사용 native 함수0개이며 이미 VS/PS
공유가 적용돼 있어 추가 수정하지 않았다. 원문 총크기와 실제 컴파일 본문 크기를 구분한다.

## G17. 최적화 반영 Debug 제품과 실제 셰이더 소비자

정규 Debug Product의 Engine/Shared/Server/Client 컴파일·링크·배포가 PASS다.
`out/BuildPipeline/runs/20260926T231700570Z-debug-product.json`의 전체 시간은
23분46.122초이며 Engine363.620초,Client1059.418초다. 직전 기능 성공본은
33분07.884초,Engine524.627초,Client1461.580초였다. 양쪽 모두 같은 정상 runner와
toolchain을 사용했지만 OBJ는20→64개,실제 FXC 출력은72→87개로 다르므로 동일 입력의
반복 A/B나 모든 빌드에 보장되는 단축률로 표현하지 않는다. PCH 재생성은 양쪽0이다.
`out/BuildOptimization20260927/debug-build-comparison.json`에 단계별 집계를 보존했다.

새 Debug DLL/CSO를 out에 독립 복사하여 동일 probe EXE를 재컴파일 없이 실행했다.
영향81ID×3owner의243개 선택·clone·fallback과7개 분할 cohort,기존126program/6clone/
9실패 rollback/504lightpass/144native/10unavailable/46numeric draw가 PASS다.
`out/SourceGroupGranularity20260927/candidate-probe.log`가 실제 CShader 결과다.
이 검사는 창 없는 WARP 경로이며 Client 최종 화면/FPS 확인은 아니다.

기준과 새 CSO의 비교에서 분할 대상인 기존6개만 달라지고15개가 추가됐다. 나머지66개는
SHA256까지 같다. 따라서 map 함수 guard와 leaf dispatch 변경의 분할 무관 출력은 실제
bytecode도 보존된다. 총크기는71,904,148→80,163,693bytes로8,259,545bytes 증가했다.
`product-comparison.json`에 전체 파일별 증거를 기록했다. 그룹 추가의 초기 로딩 비용은
반복 측정 결과와 Release 검증을 확인한 뒤 별도로 정리한다.

첫 단회 Create 합계는2862.642→3377.942ms였으나,독립 복사한 기존/새 EXE를 교대로
각3회 실행한 추가 표본에서는 평균2737.053→2785.043ms(+47.990ms),중앙값
2726.450→2794.236ms(+67.786ms)다. 기준 범위2723.971~2760.738ms,새 범위
2727.102~2833.791ms이며6회 모두 정상 종료했다. 첫0.5초 차이는 반복되지 않았다.
Release 빌드와 동시에 측정했고 OS cache·CPU/I/O 경합을 통제하지 않은 WARP 표본이므로
고정 로딩 지연이나 실제 게임 FPS로 환산하지 않는다. CSO 용량 증가는 확정된 비용이며
초기 생성 시간 개선을 주장하지 않는다. `load-ab-report.json`과6개 원시 로그가 근거다.
현재 분할을 더 확대하지 않고 컴파일 전파 감소·완료 시간과 이 비용을 함께 전달한다.

최종 Debug FXC.read에서 BaseGroup1472는 Anim1472/Static1472 두 개,LightGroup1472는
그 둘과 Deferred1472 세 개만 소비한다. 두 leaf의 합집합은 정확히 해당 그룹의3개 wrapper다.
공통 BasePrograms/LightPrograms의 실제 소비자는 각각58/87개이므로 기존 그룹의 case를
leaf에 두는 것이 광역 무효화를 막는 경계다. 새 그룹 자체를 추가하거나 공통 입력을 바꿀
때는 전체 소비자의 재컴파일이 여전히 필요하다. 파일 시각을 바꾸거나 compiler를 강제하지
않고 실제 의존 기록을 읽었다. `debug-dependency-audit.json`에 원본 SHA와 소비자를 남겼다.

설치 MSBuild의 BinaryLogReplayEventSource로4개 binlog의 TargetStarted/Finished 시각을
읽은 결과 Debug23분46.122초 중 FxCompile1347.018초(94.45%),ClCompile69.656초(4.88%),
Link4.797초(0.34%),나머지4.650초(0.33%)다. 주요 단계의 시간 구간이 겹치지 않음을
확인한 wall time이며 여러 FXC worker의 CPU 시간을 합한 값이 아니다. 여전히 큰 비용은
공통/대형 shader compile이다. `out/build-phase-timings.json`이 근거다.

## G18. Release 완료와 최종 검증 범위

정규 Release Product `out/BuildPipeline/runs/20260926T234347878Z-release-product.json`은
Engine/Shared/Server/Client 컴파일·링크·배포 PASS다. 전체25분31.568초,Engine362.957초,
Client1165.749초이며 OBJ2+77/PCH0,실제 FXC 출력87개다. 이전 기능 성공본34분48.111초와
비교한 단계별 값은 `out/BuildOptimization20260927/release-build-comparison.json`에 있다.

Release의87개 CSO는 보존한 최적화 Debug 후보와 전부 SHA256/크기가 같고80,163,693bytes다.
현재 Debug 제품을 읽는 대신 독립 보존한 probe EXE와 Debug Engine DLL을 새 out runtime에
복사한 뒤 Release CSO만 넣어 검사했다. CShader ABI/Create/clone/243개 추가 경로와 기존
실패 보존·numeric draw가 모두 PASS다. 이는 Release FX11 bytecode 소비 검증이며 Release
C++ 제품 실행이나 Client 화면 확인은 아니다. `release-cso-comparison.json`과
`release-cso-probe.log`가 근거다.

이번 작업의 소스는 동결했지만 같은 폴더의 다른 작업은 진행됐다. Release 도중08:32:50에
Movie/Kouku HLSLI5개,08:39:53에 ValtanActionWorkbench.cpp가 갱신됐고,08:40부터 별도
Client Debug 빌드가 실행됐다. 다른 작업의 변경과 프로세스는 보존했다. 따라서 Release
전체시간은 동일 입력·무경합 A/B가 아니며,전체 workspace가 무변경이라는 전제로 추가
Product를 실행해 출력0을 주장하지 않는다. 이번 변경의 Debug/Release 완료·87개 CSO 동치·
실제 leaf 의존 경계와 C++ 단일 owner 증거로 검증 범위를 닫는다. 공유 폴더의 별도 변경까지
모두 검증했거나 그 별도 작업을 완료했다고 표현하지 않는다.

최종 정적 검토는 `out/BuildOptimization20260927/final-static-review.json`에 있으며
JSON11개/XML4개 parse,신규CPP3개/INL/셰이더40개 등록·mirror·배포,보류 G15 제품 미반영을
확인했다. 후속 ScreenPost/기본 FX의 추가 분할은 계속 보류다.
