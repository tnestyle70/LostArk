# Sequencer 카메라 문서 로드와 프레임 급락 수정

## G00. 범위와 현재 호출 경로

사용자의 마지막 우선순위에 따라 Sequencer를 열기만 해도 한 자리 FPS로 떨어지는 결함을 먼저 수정한다.
연출 추가, Sequence/전투 보스 통합, 조커와 인형 재질·골격 복원은 후속 작업으로 남긴다.
그룹 저작 위치는 현재 Object Tool과 실제 저장 데이터를 조사해 RESULT에 안내한다.

시작 작업 트리는 clean이며 HEAD는 `2f289461461434bcc0442bae5a71b795e7b34c62`다.
기존 `KoukuSaydon-Sequencer-Pattern3`에서 `codex/sequencer-camera-load-performance`를 만든다.

10:00 저장 profiler는 CPU 약738ms, 관측된 `ImGui.Tool.SequenceBenchmark.Build` 약668ms를 기록했다.
`CSequencerTool::Render` → `CKoukuSaydonActionWorkbench::Render_Timeline` →
`Find_AuthoringCamera` → `CLevel_KakulSaydonArena::Ensure_CameraShotAuthoring`가 카메라 정보를 조회한다.
현재 실패는 기억하지 않으므로 같은 문서를 매 조회마다 읽고 파싱한다.
같은 `Parse_CameraShots`는 게시 카메라 로드에도 사용되며 실패 시 Level은 follow camera를 유지한다.

## G01. Level 카메라 로드 소유자

`Client/Private/Level_KakulSaydonArena.cpp`의 카메라 전용 JSON 값 개수 한도를
지원하는 다중 shot/keyframe 문서를 수용하도록 조정한다. 256KiB, 깊이12, shot64,
shot당 key64, stable ID, 유한값, 방향과 시간 검증은 유지한다. Stage Marker 한도는 변경하지 않는다.

`Client/Public/Level_KakulSaydonArena.h`에 카메라 저작 로드 시도 여부와 실패 이유를 추가한다.
Level 수명 동안 성공한 문서는 기존 캐시로 읽고, 실패한 최초 로드는 같은 프레임 및 후속 프레임에서
자동 재시도하지 않는다. `Reload_CameraShotAuthoring`가 명시 재시도를 소유한다.
미저장 camera draft가 있으면 Reload를 거부하고, 파싱 실패 시 이전 shot·baseline·draft를 보존한다.
새 읽기가 모두 성공한 뒤에만 staged shot과 baseline을 교체한다.

## G02. 명시 재시도와 계측

`Client/Private/KoukuSaydonActionWorkbench.cpp`의 Composition Camera 창에 Reload Cameras를 연결한다.
최초 로드가 실패해도 이 버튼과 실패 이유는 표시되어야 한다. 기존 Play/Save consumer를 재사용한다.
카메라 저작 디스크 읽기는 기존 Profiler에 이름 있는 scope로 연결해 다음 사용자 JSON에서 재발 여부를
구분할 수 있게 한다. 툴을 여는 것만으로 publisher나 preview를 실행하지 않는다.

## G03. 검증과 실행 준비

실제 저장 camera JSON과 실제 CDataJson/Level camera parser를 사용하는 비UI CPU 검사로 이전 한도 실패,
수정 후 성공, 잘못된 입력의 기존 출력 보존과 실패 후 반복 로드 차단을 확인한다.
Client/UI 또는 ImGui context를 실행하지 않으며 화면 캡처하지 않는다.
기존 C++ 인코딩·줄 끝을 보존하고 새 C++ 파일이나 project/filter 항목은 추가하지 않는다.
변경 CPP 최소 Debug 컴파일·링크, `git diff --check`, 실제 실행 중 프로세스와 EXE 준비 상태를 확인한다.
사용자가 새 실행에서 Sequencer 열기 및 G1/G2 카메라 재생을 확인할 경로를 RESULT에 남긴다.
