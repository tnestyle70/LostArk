# 취업준비 1일차 결과와 다음 측정

## G01. 프로젝트 실행과 LAN 확인

Debug Product 전체 빌드와 ShipNpc 정보 로그 제거 후의 후속 빌드가 PASS했다.
사용자가 Smart App Control 설정을 직접 변경한 후 Debug Server/Client가 실행됐고 사용자가 정상 동작을 확인했다.
Resources 복구 및 빌드 근거는 [노트북 복구 RESULT](2026-10-02_LAPTOP_WIFI_LAN_RESULT.md)에 보존한다.

MSBuild가 평가한 정본 Client Debug/Release x64의 debugger environment는
`LOSTARK_SERVER_HOST=192.168.0.14`, Server의 arguments는 `--bind-address 0.0.0.0`이다.
이는 실제 Wi-Fi 주소·endpoint JSON과 맞으며 Debug Server PID36548의 TCP7777 listener와
Debug/Release LocalSubnet firewall 규칙을 확인했다. VS 화면에서 선택한 시작 profile은 확인하지 않았다.
이미 Server가 실행 중인 상태에서는 Framework.slnLaunch의 `Client Only (Server Already Running)`을 사용할 수 있다.
현재 endpoint 계약은 2026-10-02 23:59:59 KST까지다.

## G02. 노트북 FPS 증상과 실제 설정 변경

사용자 관찰은 Debug Character Select 약20FPS, 직업 소개 Movie 한자리수 FPS다.
데스크톱과 동일 조건의 capture 비교나 최초 실행 당시의 GPU 사용은 측정하지 못했다.
따라서 원인이 순수 하드웨어 성능이라고 확정하지 않는다.

확인한 하드웨어는 Ryzen AI 5 340(6 cores/12 threads), Radeon 840M 및 RTX4050 Laptop(6141 MiB)이다.
Game device 생성은 기본 adapter를 사용하고 Debug D3D11 검사가 켜져 있다.
Model/Animation/Effect_Playback 일부는 Debug에서도 이미 최적화하므로 Debug 전체가 /Od라고 설명하지 않는다.

2026-10-02 다음 설정을 사용자 요청 범위에서 적용하고 읽어 검증했다.

| 항목 | 적용 전 → 후 | 확인 범위 |
|---|---|---|
| 정본 Debug/Release Client의 사용자 GPU 선호 | 전용 값 없음 → GpuPreference=2 | 두 절대 EXE 경로의 registry readback |
| 현재 Balanced 구성표의 AC CPU EPP | 20 → 0 | powercfg 성공 및 readback |
| 배터리 DC CPU EPP | 90 유지 | readback |
| 다시 실행한 Debug Client | PID29032, Lobby ready 15:13:10 | startup log와 NVIDIA process 확인 |

15:13:40에 nvidia-smi가 PID29032 Client.exe를 RTX4050의 C+G process로 표시했다.
Windows의 해당 PID 3D/Copy GPU engine 활동도 관찰했다. 이는 NVIDIA 사용 확인이며 FPS 개선 측정은 아니다.
개인 렌더링 옵션, 팀 rendering 정본, thermal/driver/security 설정은 변경하지 않았다.
AC EPP는 전원 연결 시 성능을 우선하는 설정이며 전력·발열이 늘 수 있다. 최소 CPU clock을100%로 고정한 것은 아니다.

설정의 근거: [Windows 앱별 GPU 선호](https://support.microsoft.com/en-us/windows/hardware/display-graphics/optimizations-for-windowed-games-in-windows-11),
[Microsoft CPU EPP 계약](https://learn.microsoft.com/en-us/windows-hardware/customize/power-settings/options-for-perf-state-engine-perfenergypreference).
백업과 현재 값이 자기 적용값일 때만 복원하는 스크립트는 외부 복구 폴더에 둔다.

`C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/Laptop-Performance-Preferences.json`

`C:/Users/tnest/Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/Restore-Laptop-Performance-Preferences.ps1`

## G03. Movie 계측 코드

기존 Profiler를 재사용하여 ClassMovie.Update, ClassMovie.SampleFrame, ClassMovie.SampleEffects의
Debug CPU scope를 추가했다. 상세 변경·원본 백업·정적 검증은
[Movie 계측 RESULT](2026-10-02_CLASS_MOVIE_PROFILING_RESULT.md)에 있다.
현재 사용자가 확인 중인 EXE는 앞서 빌드한 버전으로 이 계측은 아직 반영되지 않았다.
이번 코드의 compile/link 및 실제 capture 출력은 아직 미검증이다.

Bone.cpp에는 기존 Animation/Channel의 방식과 같은 Debug|x64 최적화를 프로젝트 항목에 반영했다.
MSBuild 평가에서 Debug의6개 metadata만 바뀌고 Release의98개 metadata는 전후 동일하다.
기존 native intermediate3개·build profile2개 검사가 통과했다. C++ 수식·ABI·_DEBUG·Debug CRT는 유지한다.
단 Bone.cpp의 /RTC1·JMC·Edit and Continue는 해제되며 최적화 코드의 PDB를 사용한다.
[Bone PLAN](2026-10-02_BONE_DEBUG_OPTIMIZATION_PLAN.md)에 교체 블록과 디버깅 차이를 기록했다.
컴파일 및 동일 조건 FPS 비교는 아직 수행하지 않았다.

## G04. 구조 조사와 포트폴리오 자료

[Save/Publish 가이드](2026-10-02_SAVE_PUBLISH_RUNTIME_GUIDE.md)에 Rendering, Map, Gameplay,
Valtan, Effect V1의 실제 owner·파일·게시·runtime 적용 경계를 정리했다.
Rendering 저작본과 게시본의 revision90/29 profiles 및 의미상 동일성을 실측했다.
Gameplay.bootstrap의 v38/109,866 rows/32,214,003 bytes 헤더·크기를 reader bound와 대조했다.

외부 Portfolio-Inventory-Index.json에 연결된 59개 항목은 실제 코드의 정적 조사다.
개인 기여도와 사용자의 최종 시연 결과를 대체하지 않는다.
과거 publish 최적화 수치와 오늘의 새 측정은 구분했다. 이 초기 분석에서는 publisher나 navigation bake를 실행하지 않았다. 후속 Bern nav 복구에서는 Area 한정 공식 publisher를 외부 stage에서 실행했다.

## G05. 다음 실행과 미완료 경계

사용자가 설정 변경 뒤 훨씬 좋아졌다고 보고했고 F7 capture 세 개를 저장했다.
JSON의 RTX4050·Debug·1920×1080·FPS 제한0을 확인했다. 변경 전 GPU 기록은 없고 GPU·AC EPP를
함께 바꿨으므로 기존 내장 GPU 사용이나 각 변경의 기여도를 확정할 수 없다.

| 저장 이름 | 구간 | frame interval 평균 / p95 / 최대 | 수집 경계 |
|---|---|---|---|
| 무비_프레임측정 | 464~583, 120 frames, 약1.99초 | 16.578 / 25.564 / 41.519 ms | Detailed OFF, GPU116 valid·4 pending, 20FPS 지속 구간 없음 |
| 무비_프레임측정_도화가 | 11410~11529, 120 frames, 약5.88초 | 48.994 / 67.323 / 83.206 ms | Detailed ON, GPU120 valid, 전체 frame이33.333ms 초과 |
| 무비_프레임측정_차원술사_가디언나이트 | 12117~12236, 120 frames, 약3.63초 | 30.291 / 52.736 / 62.284 ms | Detailed ON, GPU116 valid·4 pending, 보고된8FPS 구간 없음 |

scope/animation drop은 세 파일 모두0이다. metadata는 export 시점이며 각 과거 frame의 Movie class·camera를
보증하지 않는다. 파일 이름만으로 서로 다른 Movie 구간을 확정하지 않는다.
도화가에서는 main CPU Update30.438ms, Render18.524ms, Animation.Play13.255ms/frame(38회)이 관찰됐다.
Channels.Update11.516ms는 Animation.Play에 포함된다. GPU Render.Draw15.574ms 중 Lights9.520ms다.
CPU/GPU inclusive를 더하거나 GPU full interval49.638ms를 GPU busy 시간으로 해석하지 않는다.

변경 전후 동일 조건의 정량 개선 배율·차원술사 첫 카메라의8FPS 원인·Release 비교는 아직 없다.
Detailed OFF와1200-frame window로 느린 첫 카메라를 다시 수집하도록 안내했다.

기존 idle queue24808은 Guide/Nav/공통 Movie 수정의 검증을 위해 중지했다. 이전8개 입력만을
검사하는 queue는 재사용하지 않는다. 현재 Movie 공통5개 TU의 Debug/Release 컴파일10개와
v143 수치 검사, Guide111개, 새 Bern nav의 Navigation58개 계약 검사가 통과했다.
Nav는 검증된 두 paint 및 Client/Server 산출물8파일을 백업 후 반영했다. 실제 범위와 별도 실내
입구 미구현은 [Bern nav RESULT](2026-10-02_BERN_NAV_GROUND_RECOVERY_RESULT.md)를 따른다.

새 통합 Product 빌드는 GuideNavMovie-Build-Ready.json의 최종 입력으로 다시 준비한다.
현재 게임은 유지했으며 사용자에게 EXE/DLL 링크를 위한 저장·종료 방법을 한 번 확인했다.
Product Debug/Release 링크·실행과 FPS는 아직 미확인이다. 기존 Release launch helper는
검증된 성공 receipt·현재 endpoint·출력 비점유를 모두 확인한 후에만 이전 실행 요청을 수행한다.
이 상태는 외부 Laptop-Recovery-Build-Checkpoint.json에 보존한다.
3개 capture의 원본은 외부 복구 폴더에 SHA256를 대조하여 추가 보관했다.
상세 분석·반복 포즈 구조·수집 한계는 [Movie 성능 RESULT](2026-10-02_CLASS_MOVIE_PERFORMANCE_RESULT.md)를 따른다.

Rendering Workbench의 동기 Publish 대기와 Profiler의 복수 Capture 제어 경로를 확인했다.
비동기 게시·Capture 제어 통합·domain 입력 축소는 아직 구현하거나 성능을 측정하지 않았다.
연결 계획: [Day 1 PLAN](2026-10-02_JOB_PREPARATION_DAY01_PLAN.md).

## G06. 영상 자막과 기술 소개서 구성 조사

이전 ‘포트폴리오와 엔진 수준 평가’ 대화와 사용자가 제공한 Notion 포트폴리오를 읽고,
현재 작업 폴더의 기능 영역 59개와 대표 구현·소비 경로를 대조했다. 모든 코드 줄이나
실행 결과를 전수 검증한 것은 아니다. 단일 shadow map, ribbon의 조건부 Hermite 보간,
서버 가입 처리 순서의 party roster, Effect V1의 직접 authored 소비 등 자막 사실을 교정했다.

LostArkTransfer의 지정 MP4 3개(합계 01:59:54.550)에서 메타데이터와 표본 프레임 48개를
확인하고 입장·레이드·워터팡·콜로세움·용 탑승의 장면 후보를 연결했다. 원본 영상은 변경하지
않았다. 표본 위치는 최종 컷 경계가 아니며, 표본에서 저작 도구 화면을 확인하지 못했다는
결과를 전체 영상의 부재 증거로 사용하지 않았다.

상세 원문: [영상·기술서 조사 RESULT](2026-10-02_VIDEO_TECHNICAL_PORTFOLIO_AUDIT_RESULT.md).
43쪽 PDF는 `output/pdf/LostArk_Technical_Audit_and_Video_Plan_2026-10-02.pdf`에 생성했다.
43쪽 전체 렌더와 육안 검토, 텍스트 경계·대체문자 검사 및 새 원문의 whitespace 검사를
완료했다. 코드·런타임 데이터·렌더링 옵션 변경이 없으므로 게임 빌드는 수행하지 않았다.

이번 PDF는 조사·편집·집필 구성안이다. 최종 인·아웃 컷, 타이밍이 있는 SRT, 편집본 export,
개인 기여 확정 및 제출용 포트폴리오 PDF는 아직 완료하지 않았다. 기존 여러 프로젝트를
묶는 약 200쪽 상세 기술서와 별도 요약 포트폴리오의 방향을 유지했다.
