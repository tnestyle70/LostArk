# 쇼타임 시퀀서 병목 분석·개선 결과

## G00. 입력과 원인

입력은 `Client/Bin/ProfilerCaptures/디버그_쇼타임_시퀀서_20261003_140615_881_frame179_53880_0.json`이다.
SHA256은 `19c20569211c2209b133041fba53166f59c93b37eb0ce5f3e5d43b234ca18490`이며 원본을 변경하지 않았다.
Debug PID53880, MSVC1944, iterator debug2, D3D debug layer ON, RTX4070의120프레임(60~179)이다.

| 관측 구간 | 측정 |
|---|---:|
| CPU frame60~63 | 1408.6271~1537.2406ms |
| 같은 프레임 BundleSample | 1109.1039~1132.8912ms |
| frame63 BundleSample Self | 1129.5366ms |
| frame65~179 CPU 평균 | 41.991ms |
| 같은 구간 Timeline 평균 | 20.307ms |
| Timeline Layout / Draw 평균 | 12.474ms / 7.077ms |
| 같은 구간 분리 창5개 Present 합 평균 | 4.790ms |

frame60~62는8192 CPU scope 상한에 도달해22985개가 누락됐다. frame63은 누락 없이
BundleSample의 마지막 Animation 자식 종료 뒤1127.7022ms가 남았다. frame64부터 BundleSample이
없어져 이 이후를 정상 패턴 재생과 동일한 조건의 성능 비교로 사용할 수는 없다.
frame64의1408ms interval은 이전 frame63의 비용이며 frame64 CPU 자체는58.074ms다.
GPU timestamp에는 CPU 제출 대기가 포함될 수 있어 이를 셰이더의 순수 실행 시간으로 보지 않는다.

실제 `KAKULSAYDON_G1_PATTERN_35`는 presentation697개, logic occurrence118개다.
전체 문서에는 logic definition210개와 presentation resource1139개가 있다.
기존 `Random_TargetWindow`는 presentation의 history/재생/분기 루프에서 반복 호출되며
각 호출마다118개 logic와210개 definition을 선형 탐색했다. 시간 밖 occurrence도 이 탐색을 했다.
쇼타임에는 해당 BOSS_RANDOM_TARGET 연결이0개여서 이 반복 탐색이 모두 빈 결과를 반환했다.
시퀀서 Layout도 문서 변경이 없어도 매 frame12개 lane의 interval 생성·정렬·행 할당을 반복했다.

## G01. 소스 반영

`KoukuSaydonPresentationPlayer.cpp`의 Sample은 호출마다 현재 문서의 definition 연결과
random target window, selected airborne group을 한 번 resolve한 뒤 ID로 조회한다.
view/pointer 수명은 해당 Sample 안으로 제한해 문서 교체와 미저장 편집 다음 호출에 다시 반영한다.
기존 첫 definition, disabled/누락 정의, 반열린 시간 구간, 마지막 중첩 window 우선 의미를 보존했다.
기존 중복 selected-group helper의 선언/정의를 대응 header와 LogicPreview.cpp에서 제거했다.
다음 캡처에 `Kouku.Presentation.Sample`과 `Kouku.Presentation.ResolveLogics`를 추가했다.

`KoukuSaydonActionWorkbench.cpp/.h`는 draft generation, pattern ID, World inventory generation,
최소 box 시간, World animation 편집 가능 여부, 실제 camera blend-out 값을 key로 행 배치를 재사용한다.
픽셀 좌표와 선택/drag/marquee는 계속 현재 프레임에서 계산하며 화면 밖 box는 그리기만 생략한다.
기존 InvisibleButton과 hit box는 유지한다. `ImGui.Composition.Timeline.Layout.Rebuild`로 cache miss를 계측한다.
독립 리뷰에서 현재 mutation/Reload/Save/World 갱신/camera 편집 경로의 무효화 누락은 발견하지 못했다.

Data/Composition 저작·게시 문서, rendering options, 실제 패턴 시간·effect seek 방식은 변경하지 않았다.
사용자가 저장한 Gate1 문서 SHA는 `6c7ae0cb9e4ceb4ec663bb4de7352fd8f2d2e014beb9824b028f912681a5f3bf`다.

## G02. 실행한 검증

실제 lookup 함수를 추출해 정본 JSON2개(131 patterns)와 경계 입력을 비교한4858검사가 failures0이다.
disabled/누락/duplicate definition, overlapping window, 끝 시각,0duration, selected group,
다음 호출의 입력 변경을 포함한다. MSVC1944 `/Od /MDd /D_DEBUG /D_ITERATOR_DEBUG_LEVEL=2`의
쇼타임3×697조회 독립 실험은 이전404.087~441.813ms(중앙415.004ms), 후보는 index 구성 포함
0.3902~0.6214ms(중앙0.4009ms)였다. 함수가 읽는 필드만 가진 축소 struct를 사용한 CPU 실험이며
BundleSample 전체 또는 제품 FPS 개선량으로 환산하지 않는다.

실제 `KoukuSaydonCompositionDocument.h` 타입으로 같은4858검사를 반복해 failures0을 확인했다.
추가로 제품의 `/RTC1 /JMC`까지 포함한 독립 실험은 이전 중앙394.899ms, 후보는 resolve 포함
0.5200ms였다. 약1.1초 BundleSample 전부가 이 helper라고 확정하거나 잔여 비용이0이라고 보지는 않는다.
`actual_benchmark_result.json`, `rtc_benchmark_result.json`에 각각 기록했다.

UI는 실제 쇼타임 입력과 원래/후보 row allocation 본문으로15검사를 통과했다.
12개 lane의 occurrence row·group span·firstRow 동일성,100개 변경 없는 프레임의 재사용과
draft/World/pattern/zoom/callback/camera-tail 무효화를 확인했다. 행 배치 단독100회 평균은
기존6.08143ms, cache hit0.000234ms였다. 전체 Timeline Draw나 제품 FPS 측정값은 아니다.
근거는 `review-code/timeline_fixture_probe.run.log`와 `fixture_provenance.json`이다.

변경 CPP3개는 실제 Debug CL tracking 옵션을 사용한 집중 컴파일에 성공했다.
PCH만 끄고 forced include를 유지했으며 OBJ/PDB는 out에 생성했다. 기존 C4819 경고가 남고 오류는 없다.
Profiler 분석기6개와 preview source lifecycle1개 검사, UTF-8/BOM없음/CRLF 보존,
`git diff --check`를 통과했다. 분석기 첫 호출은 module import 경로 오류였으며 정식 Tools/Profiler
working directory에서 재실행해6개가 통과했다.

증거는 `out/ProfilerShowtime20261003/`의 `analysis.json`,
`review-capture/capture-analysis.json`, `review-preview/benchmark_result.json`,
`review-preview/fixture_metadata.json`, `compile/receipt.json`과 각 compile log다.

## G03. 제품 반영·화면 확인

사용자의 전체 반영·merge 요청 후 Client/Server가 종료된 상태를 확인했다.
정식 Debug Product 빌드는 exit0으로 성공했고 이번 수정·Object 초기 로드·타임라인 양식 통일을
함께 링크했다. 증거는 `out/BuildPipeline/runs/20261003T053410131Z-debug-product.json`이다.
Client 단계78957ms, OBJ55개·binary1개를 갱신했고 기존 CSO를 재사용했다.
Release Product 빌드도 exit0으로 성공했다. 근거는
`out/BuildPipeline/runs/20261003T053612726Z-release-product.json`이며 Client 단계85597ms,
OBJ52개·binary1개를 갱신했다. 두 구성 모두 Engine·Shared·Server·Client 단계가 PASS다.
저작 데이터 publish와 Client/UI 자동 실행은 하지 않았다.
새 EXE에서 같은 쇼타임 구간을 재생한 사용자 캡처로 BundleSample과 Timeline 비용을 다시 확인해야 한다.
그 전까지1FPS 현상의 최종 해소나 촬영 FPS를 확정하지 않는다.
