# Character Select 유휴 Debug 성능 개선 결과

## G00. 사용자 목표와 조사 근거

같은 commit·Debug x64·같은 해상도·Ctrl+F5에서 이 PC 약 50fps, 다른 PC 약 10fps라는 사용자 관찰을 기준으로 조사했다. 후속 목표는 이 PC에서 스킬 미사용 Character Select 100fps 이상이다. 로컬 장비는 i5-13500 / RTX 4070이다. 다른 PC의 장비·새 실행 캡처가 없으므로 PC 간 5배 차이의 단일 원인은 확정하지 않는다.

기존 `profiler_20260911_200348_320_frame4119.json`의 frames 2920–3804와 `profiler_20260911_200949_603_frame17219.json`의 frames 16020–16922에서 이펙트 호출이 없는 연속 구간을 분리했다. CPU 평균은 각각 19.195/21.367ms, InputAndUI self는 4.964/4.616ms였다. GPU Render.Draw는 참고 구간에서 약 5.95/6.67ms이며 CPU와 더하지 않는다. 애니메이션은 한 모델만 약 0.54ms 평가해 숨긴 모델 대량 갱신이 원인이라는 근거는 없다.

이 파일들은 동일 세션의 중첩 기록이며 JSON 자체에 장면·class·build·장비 정보가 없다. 일부 profiler 최적화보다 이전 기록이고 패널도 열려 있으므로 현재 빌드의 고정 유휴 성능이나 다른 PC 측정으로 사용하지 않는다. 이후 이미 수정된 Profiler 집계 병목은 중복 구현하지 않았다. 상세 분석은 `out/CharacterSelectPerf20260912/capture-analysis/README.md`, `summary.json`, `analyze.py`에 있다.

## G01. 메인 루프의 60fps 상한 제거

`Client/Default/Client.cpp::wWinMain`의 1/60초 누적 조건과 Timer_Default busy polling을 제거했다. Windows 메시지 큐를 먼저 처리하고 실제 frame delta로 Update/Render를 한 번 호출한다. 기존 `Timer_60` 이름은 RaidEntryPreviewView의 frame clock 소비 때문에 유지했다. Server fixed tick이나 animation elapsed-time 계산을 FPS 횟수 기준으로 바꾸지 않았다. `RaidEntryPreviewView.cpp`와 `CLAUDE.md`의 설명도 현재 경로로 교정했다.

기존 limiter는 Profiler Begin_Frame 밖에 있었으므로 CPU frame ms가 10ms 미만이어도 실제 100fps에 도달할 수 없었다. 제한 제거 자체가 다른 PC 10fps 문제를 해결했다는 뜻은 아니다.

실제 새 루프 본문을 쓰되 Windows 메시지·GameInstance·MainApp만 stand-in으로 대체한 CPU 검사 7개를 통과했다. 200회 5ms delta, 프레임당 Update/Render와 profiler 시작/종료 짝, 입력 큐·accelerator, 진입 전/후 WM_QUIT, render 실패 종료를 확인했다. 이 검사의 200회는 실제 FPS 측정이 아니다. 실제 두 제품 CPP도 Debug `/MDd /Od /RTC1` 별도 컴파일을 통과했다. 근거는 `out/CharacterSelectPerf20260912/mainloop/compile.log`, `run.log`, `verify_loop.py`다.

## G02. UI 슬롯 반복 검색 제거

`CUILayoutRuntime`은 문서 순서의 `m_Slots`를 유지하면서 runtime 전용 `ID -> ordered indices` 검색 표를 소유한다. Load와 Ensure_RuntimeSlot의 append는 함께 인덱스를 갱신한다. 숫자 index는 저장 계약으로 노출하지 않으며 vector 재할당 뒤에도 유효하다. 일반 setter의 첫 ID 일치, flipbook/keyframe의 첫 적격 항목, unknown ID와 null sprite 동작을 보존했다.

닫힌 AvatarBook 504개와 CharacterInfo 178개 슬롯의 Hide만으로 기존 매 프레임 143,191회 문자열 비교가 발생했다. Inventory의 실제 Hide 318조회가 약 30,620회를 더해 약 173,811회가 됐다. 이 작업은 스킬과 F1 도구 가시성에 관계없이 존재했다.

닫힌 세 창의 Hide는 `Set_AllSlotsVisible(false)` 한 번으로 교체해 개별 ID 구성과 검색도 없앴다. CharacterInfo 178개, AvatarBook 504개, Inventory 217개 authored ID는 모두 unique이며 기존 Hide가 전체를 숨기는 것을 확인했다. 해당 세 owner의 Ensure_RuntimeSlot 추가는 없다. 기존 포인터·hover·tooltip 등 비가시성 외 정리 상태는 유지했다.

실제 변경 전/후 메서드와 세 Hide 본문을 사용한 CPU 검사 각각 **32개**를 통과했다. unknown 값 보존, duplicate의 적격 항목 순서, base/extra/keyframe visibility, tint, null, 동적 animation 변경, 4,096개 append/reallocation과 세 caller의 숨김 결과·정리 상태를 확인했다. 같은 Debug `/MDd /Od` 5회 중앙값은 **1.509183ms → 0.009408ms**였다. sprite·D3D는 stand-in이므로 약 1.500ms 절감을 전체 프레임 개선율로 환산하지 않는다. 실제 UILayoutRuntime와 세 창 CPP, 총 4개 Debug 컴파일도 통과했다. 근거는 `out/CharacterSelectPerf20260912/ui/receipt.json`, `before/run.log`, `after/run.log`, `probe_body.cpp`, `compile.log`다.

## G03. Deferred 조명의 비대상 픽셀 조기 제외

`Shader_Deferred.hlsl`의 directional/local entry에서 기존 source material row/marker 검사를 world-position·shadow 계산 앞으로 이동했다. Renderer가 row마다 조명을 제출할 때, 다른 row의 대부분 화면 픽셀에 그림자·좌표 계산을 수행하던 낭비를 줄인다. 표시 대상 픽셀의 native 재질 식, 조명·shadow·attenuation과 pass/binding은 그대로다.

원본/변경 FX를 제품과 같은 `/O1`으로 컴파일했다. RTX 4070의 별도 offscreen device에서 일반 경로와 native 37개 program, directional/point/spot, receiver 3종, 혼합 row 경계를 포함한 **1,026조건**을 비교했다. 33,619,968개 finite component가 정확히 같았고 비유한 값과 불일치는 0이었다.

1280×720에서 제외되는 32개 row를 제출하는 별도 fixture의 GPU 7회 중앙값은 directional **1.790→0.318ms**, point **0.988→0.318ms**, spot **1.007→0.319ms**였다. D3D debug layer가 없는 독립 device의 중복 계산 검사이며 실제 Character Select GPU 시간이나 FPS가 아니다. Client의 D3D debug layer 설정은 변경하지 않았다. 근거는 `out/CharacterSelectPerf20260912/render/probe-result.json`, `deferred_probe.cpp`와 baseline/after CSO·로그다.

UI `Shader_VtxTex`도 조사했지만 단순 `/O1` 전환은 채택하지 않았다. 현재 `/Od`가 조건부로 실행하는 arc atan2를 `/O1`이 무조건 계산하는 bytecode로 바꾸므로, CSO 크기만으로 성능 향상을 확정할 수 없었다. 맵 culling, 기존 instance upload cache, 재질 해상도·수·표시 범위는 수정하지 않았다.

## G04. 빌드·변경 분리

기존 파일의 UTF-8/CRLF를 보존했다. 제품 JSON/XML과 프로젝트 등록 항목 변경은 없다. 현재까지 변경 코드의 focused Debug compile, CPU/셰이더 비교와 전체 `git diff --check`를 통과했다. Client 메인 루프와 CUILayoutRuntime 변경의 별도 읽기 전용 검토에서 조치할 구체적 회귀는 보고되지 않았다.

사용자 최종 지시는 다른 세션 완료를 기다리지 말고 수정 후 빌드 가능한 시점을 알려 달라는 것이다. 이번 작업의 최종 소스에 대해 실제 C++ 6개와 Deferred FX를 전용 out 경로에서 컴파일했고, 확인된 오류는 없다. 전체 제품 EXE 링크·배포는 이번 작업에서 실행하지 않았으며 기존 EXE를 최신 수정의 완료 증거로 사용하지 않는다. 최종 상태는 **소스 수정·관련 컴파일 검증 완료, 사용자 Debug 제품 빌드 가능**이다.

사용자가 Visual Studio에서 Engine/Shared/Server/Client 제품을 Debug x64로 Build하거나 아래 정본 명령을 실행하면 SDK·CSO·runtime DLL과 변경 header 소비 CPP를 함께 반영한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

작업 브랜치는 `codex/character-select-idle-performance`, 시작 HEAD는 `50e4c2579d98cedf3dd25a01a6428463f9623965`다. 다른 작업의 이펙트·animevents·generator·기존 문서 변경은 보존하고 index/commit/push에 포함하지 않았다.

## G05. 사용자 실행 확인

실제 100fps 달성, 다른 PC 10fps 회복, 사용자 화면은 아직 측정하지 않았다. 에이전트는 Client/UI 실행·조작·화면 캡처를 하지 않았다.

이 PC는 LAN 설정상 server-host다. 최종 빌드 후 Visual Studio `Debug | x64`, `Server + Client` profile을 Ctrl+F5로 시작한다. Lobby → Character Select → 같은 class·기본 camera에서 준비 완료 후 스킬 없이 대기한다. 기본 FPS 비교는 F1/Profiler 창을 닫고 창 제목의 FPS를 사용한다. 별도 상세 기록은 F1 → Open Composition Profiler → Capture를 켜고 창을 닫아 약 10초 수집한 뒤 다시 열어 Save JSON으로 저장한다. 결과는 `Client/Bin/ProfilerCaptures`에 쌓인다.

100fps는 프레임 간격 10ms 이하를 뜻한다. Capture off/on, 패널 닫힘/열림은 별도 조건으로 구분하고 기존 기록과 같은 카메라·해상도를 유지한다. CPU 처리 시간만으로 목표 달성을 선언하지 않는다. 저장 이후 실제 기록에 큰 비용이 남으면 그 구간을 다음 최적화의 근거로 사용한다.
