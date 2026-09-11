# ImGui 툴 계측·최적화와 사용 종료 패널 정리

## G00. 요청과 현재 근거

다른 팀 PC에서 ImGui 툴을 쓰기 어려울 정도로 프레임이 떨어지는 증상을 조사한다. 특정 PC의 새
캡처가 없으므로 원인과 개선 FPS를 단정하지 않는다. 사용자 후속 설명에 따라 G1/G2/G3/Bingo
위치·크기 튜닝 패널은 사용 종료 기능으로 제거하고 관문 스폰/진행/Complete Play는 유지한다.

현재 CImGuiLayer는 NewFrame→각 Client 툴→Render→DX11 drawdata→platform viewport를 수행한다.
ImGui의 immediate API는 widget마다 GPU draw를 호출하는 구조가 아니다. 실제 drawdata 생성,
업로드, 배치 제출, 별도 OS 창 Present 및 툴이 수행하는 데이터 처리를 구분해야 한다.

## G01. 기존 CProfiler에 실제 ImGui 비용 연결

MainApp의 각 Tool Build/Update, F1 hub 하위 구간, open 초기화와 profiler 자체를 개별 CPU scope로
연결한다. ImGuiLayer는 DX11/Win32/core NewFrame, drawdata 완성, platform update/render를 측정한다.
기존 DX11 backend에는 optional profiler hook만 좁게 추가하고 실제 buffer growth/Map/upload,
state backup/restore/setup, DrawIndexed loop, texture create/update를 계측한다. hook 수명은
ImGuiLayer 소유이고 shutdown에서 끊는다. vendor를 교체하거나 별도 backend를 만들지 않는다.

기존 counter enum 뒤에 실제 draw list/vertex/index/command/drawcall/callback/window/viewport,
vertex/index/constant/texture upload bytes, buffer growth/texture create/update/deviceobject build/
Map 실패를 추가한다. 여러 viewport는 실제 backend 호출을 합산하고 main drawdata를 중복 세지 않는다.
같은 Counters 배열을 JSON과 Composition Profiler의 ImGui 탭이 소비한다. CPU 제출 비용과 GPU
timestamp 및 OS Present 대기는 서로 다른 지표이며, 부모·자식 시간을 더하지 않는다.

## G02. 확인된 낭비 제거

Effect unified tree는 family마다 반복 count/filter를 수행하므로 같은 frame의 단일 분류 결과를
재사용한다. 원본 요소 순서·mute·locked·selection과 pointer 수명은 유지한다. Effect sequencer는
화면 밖 box의 DrawBox/InvisibleButton 제출을 생략하되 active drag와 전체 scroll bounds를 유지한다.

DX11 backend의 empty drawdata는 필요한 texture update 이후 빠져나가고, index buffer Map 실패 시
이미 Map된 vertex buffer를 Unmap한다. main viewport만 있을 때 불필요한 platform render state
backup/restore를 생략하되 UpdatePlatformWindows로 detached window 생성/파괴 처리는 유지한다.

Composition Profiler는 필터 결과와 미관측 row를 변경 시에만 구성하고 단일 행 CPU 표에 clipper를
적용한다. ImGui filter 자체의 매 행 string allocation을 없앤다. 구형 Profiler Details의 동기 저장은
기존 Composition Profiler exporter로 합쳐 파일 쓰기가 UI thread를 막지 않게 한다.

실제 기존 cl command에서 ImGui core4개와 DX11/Win32 backend가 Debug /Od /RTC1 /ZI임을 확인했다.
Engine.vcxproj의 해당6개 ClCompile에만 Debug x64 /O2 /Zi와 compatible RTC/JMC 옵션을 적용한다.
_DEBUG·MDd·asserts를 유지하고 application/Engine 나머지의 Debug 빌드 정책은 바꾸지 않는다.
외부 ImGui 내부 stepping이 최적화된 코드 기준으로 바뀌는 tradeoff를 결과에 적는다.

## G03. 사용 종료 보스 튜닝 패널

RenderKoukuSaydonBossTuningControls와 단독 caller, 전용 baseline/load/save helper 및 header state만
제거한다. 매 프레임6회 WorldEntities 탐색과 변화 없는 setter/collider/weapon 행렬 갱신이 함께
사라진다. Data/NpcCatalog의 저장된 수치와 CNpc의 실제 표현·transform 기능은 유지한다. 원래 파일
읽기는 최초/Reload/Save 시에만 있었으므로 이를 매 프레임 파일 I/O 문제였다고 기록하지 않는다.

## G04. 검증과 완료 경계

기존 파일의 인코딩/줄바꿈과 dirty 변경을 보존한다. 새 제품 C++ 파일이나 runtime 경로를 만들지
않으므로 project/filter 항목 추가는 필요 없고 existing compile metadata만 바뀐다.
실제 Product Debug compile/link/deploy, scoped XML/JSON parse 및 diff 검사를 한다. family 분류·
순서/lock 동일성, active drag와 clipping 경계, backend counter/hook의 CPU-only 또는 headless
수치 검증을 필요한 범위에서 수행한다. Client/UI 실행·조작·화면 캡처는 하지 않는다.

다른 PC에서 F1만 연 경우, 각 툴, 대형 목록·타임라인, detached viewport의 새 캡처를 비교해 실제
남은 bottleneck과 FPS를 판단한다. 코드의 제거 작업량/컴파일 최적화를 실게임 FPS 수치로 대신하지 않는다.

## G05. Profiler 이름 등록 비용과 후속 실행 검증

계측 자체의 이름 등록 비용은 기존 `CProfiler::Intern_Name`에 function-local TLS cache를 추가해 줄인다. transparent string_view hit에서 할당·공용 mutex를 피하고, canonical ID는 기존 이름 표가 소유한다. 캐시는 최대512개로 제한하고 주소 대신 constructor의 monotonic instance ID로 재생성을 구분한다. Reset_History의 이름 보존과 End_Scope mutex는 유지한다. 동일 `/Od`의 실제 구현 baseline/after에서 이름·nested·multi-thread·Capture·Reset·재생성 및 비용을 비교하고, Capture off/on과 패널 닫힘/열림의 실제 프레임 영향은 사용자 수동 관찰로 분리한다.

사용자가 버그 수정 EXE를 직접 확인한 뒤 PR merge/push/main sync와 외부 리소스 공유를 진행한다. 현재는 실행 준비와 검증까지 수행하며 Client/UI 직접 조작·화면 판정은 사용자에게 남긴다.
