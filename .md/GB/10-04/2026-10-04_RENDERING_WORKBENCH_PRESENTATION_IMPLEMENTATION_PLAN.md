# Rendering Workbench 촬영 흐름 구현 계획

## G00. 목표와 현재 실측

복원 단계 버튼과 한 기법 A/B를 기본 화면으로 제공한다. 현재 Workbench는 복원 프로필,
43개 실험 필드, 성능 측정, 참고 사전, 저장 설정을 한 화면에 나열한다.
기존 Benchmark/RenderingProfileService의 snapshot과 preview transaction을 유지해 UI만
서로 다른 owner로 갈라지지 않게 한다. 사용자 authored/runtime 렌더링 값은 변경하지 않는다.

현재 WModel에는 복원된 텍스처·형상·스켈레톤·재질 binding이 이미 들어 있다. 기본 단계는
이 자산의 기본 재질 비교이며 최초 EXE나 과거 asset 상태를 그대로 재현하는 기능은 아니다.
Native/forward 재질은 fallback이 유효한 범위만 연결하고 나머지는 보존한다.

## G01. 시연과 비교

- `RenderingBenchmark.h/.cpp`: 현재 실효 값을 보관하고 기본 재질 → 원본 재질 → 환경·baked
  조명 → 그림자·공간 효과 → tone·LUT → 현재 완성 설정 순서로 임시 후보를 구성한다.
  단계는 현재 값을 복원하는 누적 비교이며 사용자가 꺼 둔 기능을 임의로 켜지 않는다.
- 같은 owner의 Original/A/B로 단계·기법 비교를 전환한다. 검증과 적용에 성공한 뒤에만
  표시 단계와 A/B draft를 바꾸며 실패하면 기존 화면과 선택을 보존한다.
- `MainApp.cpp`: 복원 시연 / 기법 A/B / 측정·분석 / 저장 설정 탭으로 분리한다.
  원래 화면 복귀를 공통 위치에 둔다. 고급 수치·긴 설명은 기본으로 접는다.
- `RenderingProfileService.h/.cpp`: `SOURCE_MATERIALS`를 whitelist 끝에 추가한다.
  기존 field ID·순서를 보존하고 재질 selector도 setter 실패 rollback과 close/owner-change
  복원을 사용한다. 새로운 public runtime/data schema는 만들지 않는다.
- `DeferredMaterialRenderUtils.cpp`, `MapAssetRenderUtils.cpp`: 검증 가능한 textured deferred
  재질에서만 기본 shader 경로를 허용한다. native forward/hair를 근거 없이 재분류하지 않는다.

## G02. 근거가 있는 추가 복구

WARLORD의 실사용 native700/701에서 primitive opacity prefix가 0으로 남아 Base/Light alpha를
소멸시키는 후보를 원본 명령·생성기·리소스 binding과 대조한다. 확인된 prefix만 기존 복구 계약으로
연결한다. 개별 원본 근거와 GPU/수치 검증은 별도 RESULT에 남긴다. Bern은 추가 확정 결함이
없으면 임의의 밝기·GI·재질 옵션 조정 없이 현재 상태를 유지한다.

## G03. 원본 조사와 검증

공식 Lost Ark DX11 공지, Epic UE3 자료, Gildor 게임별 extractor 지원과 로컬 보존 artifact를
대조한다. DX11/PBR 존재를 UE4 전환 증거로 사용하지 않는다. 원본 미보유·추출기 미지원·
현재 소비자 미연결을 구분해 `RENDERING_SOURCE_EVIDENCE_RESULT`에 기록한다.

현재 source 기반 세션 검증으로 단계 순서/직접 점프, 현재 복귀, 실패 보존, A/B 전환,
소유권 해제, SOURCE_MATERIALS 조건 fingerprint를 확인한다. 변경 C++/shader를 컴파일하고
가능한 Debug·Release Product 빌드 및 git diff --check를 완료한다. 새 C++ 파일이 없으므로
vcxproj/filter 신규 등록은 없다. 실제 촬영 화면은 사용자가 확인하며 Client를 자동 실행하지 않는다.

## G04. 사용자 요청: Bern 기본 Fog OFF

Bern 진입이 선택하는 `scene.bern.neutral-day.v1`의 `fog.enabled`만 false로 바꾸고 revision을
증가시킨다. 지역 안개는 기존 `profile.Fog.bEnabled && region.Fog.bEnabled` gate를 사용한다.
밀도·색·지역 원본값과 비교용 source profile, 다른 맵 설정을 보존한다. 최신 디스크를 다시 읽고
hash 확인·백업·원자적 교체 후 정식 Rendering publisher로 게시한다. 구조 비교로 위 두 필드만
변경됐는지 확인하며 실행 중 메모리 적용이나 화면 검증으로 기록하지 않는다.

## G05. 복원 순서와 행별 실측 비용

발표 흐름은 하이라이트 → 기술 및 설명 → 렌더링 → 이펙트 툴 → Rendering Workbench 비교로 둔다.
Restoration에서 초기 WModel의 기본 재질 근사부터 현재 복원 설정까지 누적 단계를 한 표에 나열한다.
원본 재질의 고정 복구(UV·vertex color·native binding·BG RNM)는 재질 복귀에 포함하며, 독립 토글이
없는 과거 결함을 정확한 역사 재현으로 표시하지 않는다. 지원 MapPBR의 RNM·환경 반사·원본 간접광,
그림자, SSAO, 안개, source tone, LUT, Bloom, FXAA, 실험 SSGI/SSR은 실제 소비 범위로 분리한다.
처음 보관한 설정의 OFF와 원본 자산은 유지하고 이전/다음 및 각 행 A/B로 진행한다.

`RenderingBenchmark.h/.cpp`의 기존 snapshot·Begin/Update/Finalize·AB/BA 반복을 재사용한다.
각 행은 이전 단계 A와 현재 단계 B를 동일한 선택 mask로 측정하며 첫 행은 동일 baseline 반복이다.
기법 A/B도 목록과 측정 버튼을 같은 화면에 제공한다. 행 옆에는 CPU/GPU A·B 평균과 B-A ms,
표본 및 상태를 보여준다. 한 측정 묶음의 stable row ID·pair ID·variant 값·공통 조건이 일치할 때만
차이를 표시하고, 미계측·부분 GPU·조건 변경·현재 세션과 다른 결과는 구분한다.
측정 차이는 해당 장면의 프레임 차이며 pass 자체의 고유 비용이나 고정 성능 보증이 아니다.

기존 MainApp 탭과 Benchmark의 profiler 접근을 유지한다. 프로젝트/셰이더/렌더링 데이터 schema
변경과 신규 C++ 파일은 없다. 기존 session fixture를 확장해 단계 순서·현재 복귀·원래 OFF·LUT 조건·
실패 보존과 결과 짝짓기를 검증하고 변경 TU 컴파일 및 Debug Product 빌드를 수행한다.
화면 확인과 실제 장면 성능 측정은 사용자가 Workbench에서 직접 수행한다.


## G06. 원본 재설치 대조와 추가 GI 비교 연결

현재 기능의 A/B를 모두 실제 소비자와 대조한다. 절반 해상도 SSGI는 기존 구현을 확장한 별도 custom
품질 선택이며 `SSGI_HALF_RESOLUTION`을 whitelist 끝에 연결한다. 구체적 renderer·shader 계획과
검증은 [SSGI 계획](2026-10-04_SSGI_HALF_RESOLUTION_IMPLEMENTATION_PLAN.md)에 두고 중복하지 않는다.
원본 복구 근거는 [원본 조사 결과](2026-10-04_RENDERING_SOURCE_EVIDENCE_RESULT.md)의 재설치 대조로
갱신한다. 미구현 Lumen/DXR·전체 source scene owner는 설명과 남은 경계로 유지한다.
