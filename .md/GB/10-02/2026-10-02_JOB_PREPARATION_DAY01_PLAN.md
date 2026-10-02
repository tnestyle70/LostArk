# 취업준비 1일차: LostArk 구조 이해와 성능 개선 계획

## G01. 현재 정본과 이번 작업의 종료 조건

작업 저장소는 `C:/Users/tnest/Desktop/LostArk`, 브랜치는 `codex/laptop-wifi-lan-20261002`,
HEAD는 `c44e8a6666ea9b4592ebf4873287444facc8db92`다. 확인한 origin/main은
`a0ff0185cc7fea171835cde15777c0f520d9d0ae`이며 기준 tree는 같다. 노트북 LAN·로그 정리·검사기
교정 등 로컬 변경은 별도로 존재하므로 현재 작업 폴더 전체가 원격과 동일하다는 뜻은 아니다.

이 문서는 학습·측정·포트폴리오 근거를 관리한다. C++ 구현 코드 정본은 기능별 구현 PLAN에 둔다.
오늘의 완료 조건은 실제 Save/Publish 경로를 설명할 수 있는 자료, 코드 근거가 있는 기능 목록,
노트북 성능 설정의 적용·복원 기록, Movie 계측 코드와 검증 상태를 갖추는 것이다.
측정하지 않은 FPS 향상이나 확인하지 않은 개인 기여를 완료로 쓰지 않는다.

## G02. Profiler에서 낮은 FPS를 조사하는 순서

대상 파일은 `Client/Private/ClassSelectionPresentation.cpp`, `ProfilerTool.cpp`,
`ProfilerCaptureIO.cpp`, `Engine/Private/Profiler.cpp`다. 직업 소개 Movie는 MP4가 아니라
actor·camera·material·light·effect를 매 frame 샘플링하는 실시간 장면이다.

1. 같은 Debug EXE에서 Windows 고성능 GPU 선호를 적용하고 실제 NVIDIA 사용을 확인한다.
2. 전원 연결 시 CPU 성능 선호만 조정한다. 이전 값과 복원 명령을 저장한다.
3. 같은 viewport·카메라·직업·포커스 상태로 Character Select idle과 같은 Movie 구간을 수집한다.
4. 첫 재생 준비 지연과 두 번째 재생의 지속 비용을 나눈다. Detailed per-draw 계측은 끈다.
5. F7의 Capture로 수집하고 창을 숨겨 재현한다. Capture를 끈 뒤 GPU pending 결과가 해소되면 저장한다.
6. CPU Self, GPU 유효 표본, Present·대기·Debug 검사를 분리한다. 중첩 inclusive 시간을 더하지 않는다.
7. 캡처가 지목한 병목 하나를 수정하고 같은 조건의 전후 결과와 사용자 화면 확인을 RESULT에 기록한다.

이번 Movie 계측 코드는
[구현 PLAN](2026-10-02_CLASS_MOVIE_PROFILING_IMPLEMENTATION_PLAN.md)에 최종 전문을 보존한다.
새 C++ 파일이나 project/filter 등록은 필요 없다. 계측 추가 자체는 FPS 최적화 결과가 아니다.
Debug의 D3D11 debug layer와 animation history 검사를 무조건 제거하지 않는다.
Release 비교는 별도 빌드 결과와 외부 frame timing 또는 사용자 관찰로 구분한다. Release에 F7을 추가하지 않는다.

## G03. Rendering Workbench의 저장·비교·게시 경계

대상은 `Client/Private/RenderingProfileService.cpp`, `RenderingBenchmark.cpp`,
`Data/Rendering/Authored/RenderingProfiles.json`, `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`이다.

현재 저장 revision 90의 29개 profile과 게시 JSON의 의미상 동등성을 기준선으로 보존한다.
Workbench의 Save, Publish, Reload를 구분하고 세션 비교와 영구 저장 옵션을 분리해 설명한다.
특히 FXAA는 저장되는 scene quality이므로 임시 비교라고 가정해 바꾸지 않는다.

후속 구현 후보는 UI의 동기 Publish 대기를 기존 publisher를 사용하는 비동기 작업으로 바꾸는 것이다.
이 후보의 구현 전에 작업 수명·중복 요청·source 변경·완료 회수·실패 보존을 설계하고 별도 전체 코드 PLAN을 작성한다.
이 문서만으로 비동기 구현이나 성능 개선을 완료했다고 보지 않는다. 팀장의 렌더링 정본은 유지한다.

## G04. Save와 Publish를 실제 데이터로 설명하기

[구조 이해 가이드](2026-10-02_SAVE_PUBLISH_RUNTIME_GUIDE.md)를 따라 다음 경로를 하나씩 읽는다.

| 순서 | 실제 입력과 소비자 | 학습 결과 |
|---|---|---|
| Rendering | Authored RenderingProfiles → publisher → runtime JSON → service Reload | 저장·파일 게시·실행 메모리 적용의 차이 |
| Map | Imported catalog + Authoring placement → Map publisher → runtime catalog | stable ID, 교차 참조, staging과 실패 복구 |
| Gameplay | Balance·Encounter·Animation → Gameplay.bootstrap → Server catalog | 편집 형식과 제품 reader 계약의 차이 |
| Valtan | Source Save → immutable candidate; 파일 게시와 live admission은 별도 경로 | 미완성 저작본과 마지막 정상 Product를 함께 보존하는 이유 |
| Effect V1 | Authored document → 제품 loader | 이중 복사본을 제거한 직접 정본 소비의 예외 |

Publish 비용은 hash/lock, parse/validate, projection/bake, serialize/stage/commit으로 나누어 측정한다.
전체 FullDiagnostic 반복을 기본 작업으로 만들지 않는다. 캐시 단위·입력 범위·현재 도구 호출 경로부터 확인한다.
과거 Navigation 최적화 결과와 오늘의 실측은 분리한다. 오늘 완료한 C++ 빌드 시간을 Publish 시간으로 쓰지 않는다.

## G05. 이력서와 포트폴리오의 근거 만들기

실제 조사 목록은 저장소 밖
`C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/Portfolio-Inventory-Index.json`에 있다.
도구·게시·빌드 29개, Engine·Client 19개, Server·Shared·게임플레이 11개를 정리했다.
코드 존재, 실행 시연, 개인 담당, 개선 수치를 각각 별도 근거로 붙인다.

대표 설명 후보는 저작 도구의 안전한 저장·게시, 서버 권위 게임플레이, CPU/GPU 성능 진단이다.
각 사례를 증상 → 요구 조건 → 설계 선택 → 실제 데이터 흐름 → 실패 보존 → 검증으로 작성한다.
모든 목록을 이력서에 넣는 대신 본인이 설명하고 시연할 수 있는 사례부터 선택한다.

## G06. 이어서 갱신하는 방식

매 작업 후 [Day 1 RESULT](2026-10-02_JOB_PREPARATION_DAY01_RESULT.md)에 확인·구현·컴파일·사용자 시연을 나눠 기록한다.
새 병목이 확인되면 해당 기능 PLAN/RESULT를 연결하고 전후 capture 경로와 비교 조건을 보존한다.
데이터 변경이 필요할 때는 최신 저장본과 draft를 구분하고 기존 authoring CAS·publish 검증을 유지한다.
사용자 게임을 종료하거나 편집 draft를 Reload로 버리지 않는다. 실행 파일 교체는 사용자 검토가 끝난 뒤 진행한다.
