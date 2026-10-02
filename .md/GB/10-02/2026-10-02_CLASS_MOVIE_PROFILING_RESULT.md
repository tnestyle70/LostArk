# Character Select Movie CPU 구간 계측 결과

## G01. 소스 반영 범위

2026-10-02 부모 작업의 독립 검토를 받은 계획과 10줄 패치를 반영했다.
`Client/Private/ClassSelectionPresentation.cpp`에 `Profiler.h` include와 기존
`Engine::CProfilerScope` 세 선언만 추가했다. 세 선언은 `_DEBUG`로 감싸며 Release에는
새 계측 호출이 들어가지 않는다. 함수 서명·본문의 기존 제어 흐름·public ABI는 유지했다.

| 실제 함수 | 새 고정 이름 | 현재 코드 줄 |
|---|---|---:|
| `Update` | `ClassMovie.Update` | 1434 |
| `Sample_Frame` | `ClassMovie.SampleFrame` | 1067 |
| `Sample_Effects` | `ClassMovie.SampleEffects` | 1110 |

세 scope는 함수 호출 전체를 측정하므로 조기 반환과 비활성 Movie의 짧은 Update도 포함한다.
정상 재생에서는 Update 안의 SampleFrame, 그 안의 SampleEffects가 중첩된다.
inclusive 시간을 더하지 않고 Self 또는 같은 범위의 전후 값을 비교해야 한다.
Play/Seek에서 직접 실행하는 SampleFrame도 같은 이름으로 측정한다.
Capture OFF 비용은 0이 아니라 기존 Begin_Scope의 collecting 검사와 짧은 return이다.
instance·Effect ID·입자·본별 문자열이나 scope를 추가하지 않았다.

현재 Movie·Effect 재생 값, Resources, Data/게시 JSON, 렌더링 옵션과 Server 상태는 이 변경에서
수정하지 않았다. 새로운 manager·계측 runtime·capture schema도 만들지 않았다.
scope 이름은 기존 Intern_Name과 ProfilerTool::Refresh의 Get_ScopeNames 소비로 발견되므로
별도 scope catalog나 UI를 수정하지 않았다. 이 소비 경로는 정적 확인이며 실제 UI 표시 검증은 아니다.

## G02. 실행한 정적 검증

| 검증 | 결과 |
|---|---|
| 적용 직전 원본 SHA-256 재확인 | PASS, 계획 작성 시 원본과 동일 |
| 코드 diff | PASS, 기존 C++ 1파일 10줄 추가만 존재 |
| 추가 include와 세 guard/scope 제거 후 원본 바이트 비교 | PASS, 원본과 완전히 동일 |
| C++ 인코딩·줄바꿈 | PASS, UTF-8 BOM 없음, CRLF 1567줄, bare LF 0 |
| PLAN 전체 fenced code와 실제 C++ | PASS, Markdown LF를 CRLF로 복원한 바이트가 실제 파일과 동일 |
| 세 이름·Debug guard 중복 | PASS, 각각 정확히 1개 |
| Client.vcxproj/filters XML와 기존 등록 | PASS, 두 파일에서 기존 ClCompile 등록 각각 1개; 등록 변경 없음 |
| 변경 C++ git diff --check | PASS |

원본 SHA-256:
`ca3d127232bc08641a2de703f561d995440d8bb8d94fba303f6c79e1d91b009c`

최종 SHA-256:
`40c5e88d9bf77818bbf929a6ecf4db38d932968f88b8e16c4621915fc7e52cbd`

상세 정적 영수증과 적용 전 원본은 저장소 밖에 보존했다.

- `C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/ClassMovie-Profiling-Static-Verification.json`
- `C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/ClassSelectionPresentation.movie-profiling.before.cpp`

## G03. 빌드·실행·성능 확인 경계

이번 작업에서는 C++ compile/link, 테스트 바이너리, Product 빌드, publisher를 실행하지 않았다.
Client/Server의 실행·종료·UI·렌더 옵션을 조작하지 않았으며 commit도 하지 않았다.
실행 중인 기존 EXE에는 이번 소스 변경이 아직 반영되지 않는다.

사용자가 보고한 Character Select 약 20FPS와 Movie 한자리수 FPS는 증상 기록이다.
이번 작업의 실제 Profiler 캡처, CPU/GPU 병목 판정, FPS 개선 측정은 아직 없다.
소스에 scope가 있다는 사실을 성능 개선이나 화면 PASS로 기록하지 않는다.

다음 확인은 사용자 앱 종료 뒤 부모 작업의 표준 Debug Product 빌드,
사용자의 F7 → Capture → Movie 재생 → GPU pending 회수 → Save JSON 순서다.
idle과 같은 Movie 구간을 같은 viewport·설정으로 비교하고 Debug 전용 검사,
첫 준비 지연, 지속 재생, ImGui·Present·GPU pass 시간을 분리한다.

연결 계획: `2026-10-02_CLASS_MOVIE_PROFILING_IMPLEMENTATION_PLAN.md`.


## 후속 실제 컴파일

2026-10-02 공통 Movie 샘플 재사용과 통합하면서 변경된 ClassSelectionPresentation.cpp를
정상 프로젝트의 Debug/Release에서 실제 ClCompile하여 둘 다 오류0을 확인했다.
새 Engine/Client Product 링크·DLL 배포·새 계측 JSON 및 사용자 FPS는 아직 확인하지 않았다.
현재 진행과 컴파일 영수증은
[공통 Movie 구현 RESULT](2026-10-02_MOVIE_ANIMATION_SAMPLE_REUSE_RESULT.md)를 따른다.
