# Release 무비 Profiler·Bern 컬링 범위·컷신 FOV 결과

## G01. 반영한 실행 계약

- Debug/Release F7이 같은 Profiler 창을 열고 닫는다. Capture 수집과 이름 있는 JSON 저장은 창 안에서 선택하며 창을 숨겨도 수집은 유지된다. Release 저작 도구 제한은 유지한다.
- 직업 무비의 ClassMovie.Update/SampleFrame/SampleEffects 세 CPU scope를 Release에서도 수집한다. Capture OFF에서는 기존 collecting 검사 직후 반환한다.
- MainApp이 현재 Level==BERN을 GameInstance::Render(bool)→Renderer::Draw(bool)→Render_NonBlend(bool)에 매 프레임 전달한다. 다른 Level은 Cull_StaticOcclusion 호출부터 생략한다. Engine에 Client enum을 넣거나 사용자 OcclusionEnabled/A-B ownership을 덮어쓰지 않는다.
- F7 `컬링 비용·제외량`은 기존 수집 데이터를 읽는다. 분석 범위의 CPU 전체/자체 ms·호출 평균과 최근 CPU 완료 프레임의 검사·제외·캐시·draw/index 수를 분리한다. Map.Batch.CullAndPack은 상세 모드의 실제 scope이며 고정 MapBatchVisibility work는 상세 OFF에서도 볼 수 있다. 부모/자식·worker 시간과 서로 다른 index 단위를 합산하지 않는다.
- 기존 프레임 변화·기준 A/B 탭으로 이동하는 버튼과 세션 컬링 제어를 연결했다. 미관측을0ms 측정으로 만들지 않는다. capture context의 기존 option map에 가림 정책 허용·프러스텀·Bern FOV override 상태를 기록하며 별도 collector/schema는 추가하지 않았다.

## G02. Bern 컷신과 F1

사용자 승인에 따라 Data/Encounters/Bern/BernEntranceCamera.json의16키 FOVY만60→45도로 변경했다. stable scene ID, eye/lookAt, timeMs, 보간,16초 cue와 runtime0.5배 속도는 보존했다. 일반 gameplay/ship의 Data/Camera/Bern.camera.json은 변경하지 않았다.

최종 사용자 지정 위치는 Debug/Release F1 → `Bern Entrance Camera`다.20~60도 slider와30/35/40/45도 preset, `Use authored FOV`를 제공한다. F7은 컬링 비용과 캡처를 소유한다.

Level_Bern의 메인 스레드 process-session optional override는 유한10~120도만 허용하고 실패 입력에서 이전 값을 보존한다. 기존 Sample_Cue 성공 후 pose의 FOV만 바꾸며 Apply_PresentationPose 실패·ESC·정상 종료의 기존 복원 흐름을 유지한다. Lobby에서 미리 선택할 수 있고 EXE 종료 또는 명시 reset 전까지 유지된다. 일반 이동·선박·직업 무비에는 적용하지 않고 JSON으로 저장하지 않는다.

45도는 같은 거리의 시야 폭·높이가60도의71.74%, 단면적은51.47%다. 실제 draw/FPS 비율은 geometry 분포·LOD에 따라 달라지므로 이 수치를 성능 개선율로 제시하지 않는다. 이미 메모리에 들어온 cue는 디스크 변경으로 자동 reload하지 않는다.

데이터 설치는 최신 SHA 재확인, 원본 backup, 원자적 File.Replace, 결과 SHA 검증을 수행했다. FOV 치환을 역으로 돌리면 원본 text와 exact 일치한다. 별도 publisher가 없는 직접 로드 문서다.

- 변경 전 SHA256: `9064E10E1A76E8E14679C4D32DD0B4E9AEE6754C2B492A326DE1BDAB4D9AA477`
- 변경 후 SHA256: `58400C067D6D011E140380FBD917D2ADC00C8864BC8B074898C7CA7E38A3EAEF`
- 설치 receipt/backup: `out/ReleaseMovieProfiler20261007/bern-camera-install.json`, `BernEntranceCamera.before.json`

## G03. 프레임 저하 조사에서 확인한 것

기존 animation sample reuse는 WorldSequenceObject와 Animation 소비자에 남아 있고 NpcPoseReuseEnabled 기본값도true다. 최근 #532는 Sequencer/Workbench UI 레인 변경, #531은 정적 lighting bank 중복 검사 축소다. RenderingProfiles는10월2일 기준과 비교해 Bern fog/revision만 달랐고 Character Select profile 변경은 없었다. 무비 JSON 변경은 차원술사 제외 목록 증가와 워로드 카메라 키였다.

이전 `profiler_20261007_112004_306_frame390_3504_7.json`의 무비251프레임에서 Render.MapOcclusion 평균0.02856ms, p95 0.0694ms, 최대0.1293ms이며 후보·검사·래스터·제외 수는 모두0이다. Bern frame state가 없는 객체는 descriptor를 거절하므로 다른 Level에서는 불필요한 후보 순회만 있었다. 이 캡처는 오전 Debug/AMD840M 실행이므로 현재 Release의 FPS 원인 확정에 대신 쓰지 않는다. 이번 gate는 남아 있던 순회까지 제거한다.

사용자가 실행했던 Release PID16056의17:47:43 Graphics.Adapter는 RTX4050이다. 같은 session 로그에는 가용 RAM 약0.8~1.5GiB와8초·1.5초 메인 펌프 간격이 있었다. 별도 OS 표본에서는 가용631MiB와 실제 system page-in 증가도 관측했다. private commit·working set·VRAM은 서로 다른 값이며 메모리 압박만으로 해당 무비 프레임의 단독 원인을 확정하지 않았다.

사용자가 충전기 미연결 가능성을 추가로 제기했다. 이후 읽은 Windows 상태는 Online/Charging이지만 느렸던 순간의 AC 상태와 같은 장면의 전후 FPS는 미수집이다. 전원 조건을 맞춘 새 Release 캡처가 필요하다. SSAO 정적 instruction 증가 등 다른 후보도 현재 실행의 GPU 병목으로 확정하지 않았다.

Bern 컷신의 기존 Debug 캡처에는 진입 경계 draw2216→10004, VS2.831M→11.958M, NONBLEND CPU42.123→254.011ms가 있었다. 이 자료는 넓은 시야의 제출량을 줄이는 방향의 근거이며 현재 노트북 Release의 개선량은 아니다. 과거 상세 근거는10-04 BERN_SPATIAL_CHUNK_HLOD_RESULT를 따른다.

## G04. 검증 증거와 남은 확인

초기 F7 변경3개 CPP와 Engine GameInstance/Renderer의 선택 Release 컴파일은 성공했다. FOV UI 최초 컴파일에서 Level_Bern include 누락을 발견해 직접 include로 수정했다.

첫 Release Product는 `out/BuildPipeline/runs/20261007T091621851Z-release-product.json`으로 PASS했다. Engine/Shared/Server/Client 컴파일·링크·배포와 runtime input 확인이 완료됐으며 Client 단계316.014초, OBJ206개·PCH0·CSO0 쓰기였다.

F1 배치와 최종 캡처 메타데이터까지 반영한 Release Product도 `out/BuildPipeline/runs/20261007T091819771Z-release-product.json`으로 PASS했다. 전체94.337초, Client 단계90.526초이며 OBJ2개·PCH0·CSO0·binary1개 쓰기를 기록했다. 최종 Client/Bin/Release/Client.exe는2026-10-07 18:18:16 KST에 갱신됐다. EngineSDK GameInstance.h와 소스, Engine과 Client에 배포된 Release Engine.dll의 SHA256 일치를 확인했다. 기존 인코딩 경고와 DirectXTK PDB 경고는 있으나 최종 빌드 오류는 없다.

추가 최소 Debug 검증은 Engine GameInstance.cpp·Renderer.cpp와 Client MainApp.cpp·ProfilerTool.cpp·ClassSelectionPresentation.cpp·Level_Bern.cpp의6개 번역 단위 ClCompile이며 모두 exit0이다. Debug 전체 링크·배포는 실행하지 않았다. 로그는 `out/ReleaseMovieProfiler20261007/compile-debug-*.log`에 있다.

기존 Bern entrance 계약 테스트4개, JSON parse·FOV외 exact 보존, Client/Engine 프로젝트·filters XML4개 parse, 코드 독립 검토와 git diff --check를 통과했다. 신규 C++/프로젝트 항목은 없으며 기존 C++ 인코딩·CRLF를 유지했다. Renderer.h는 기존 CP949, 나머지 변경 C++은 UTF-8 BOM 없음이다. PLAN에는 변경10개 C++ 파일 전체 코드와 카메라 JSON을 보존한다.

Client/Server를 자율 실행하거나 종료·UI 조작·화면 캡처하지 않았다. 기존 프로세스 종료를 확인한 뒤 빌드했다. 실제 F1/F7 표시·JSON 저장·컷신 구도·FPS 개선은 사용자 확인 전이며 드랍 해결로 기록하지 않는다.

## G05. 2026-10-09 main 동기화 검증

사용자가 `git status`의 수정27개·신규문서5개를 모두 보존해 main에 반영하도록 승인했다.
Release Profiler/Bern 카메라, 카메라 키 목록, 프로젝트 조사, 기존 코드 학습 주석,
Customization 필터 변경을 기능별5개 커밋으로 나누고 최신 main `0164be5a`를 충돌 없이 병합했다.
PR은 [#538](https://github.com/tnestyle70/LostArk/pull/538)이며 아래 빌드 대상은 `b69de645`다.

초기32개 파일을 Git 제외 `out/MainSync20261009/backup`에 보존했다. 백업 대조에서
25개는 바이트 동일, 소스4개의 줄 끝 공백17줄과 전체코드 PLAN의 줄 끝 공백16줄만 정리했다.
gotchas와 팀 사용서에는 원격 main의 독립 변경만 추가됐으며 원래 수정 내용도 유지했다.
원본 MainApp.cpp의 인코딩 관련 주석 끝 공백은 변경하지 않았다.

| 정본 Product 검증 | 전체 시간 | 보고서 |
|---|---:|---|
| Debug x64 | 291.739s | `out/BuildPipeline/runs/20261009T011844823Z-debug-product.json` |
| Release x64 | 323.312s | `out/BuildPipeline/runs/20261009T012425216Z-release-product.json` |

두 구성 모두 Engine/Shared/Server/Client 컴파일·링크·배포가 PASS이며 실제 빌드다.
Client OBJ 갱신은 Debug215개·Release213개, 두 구성 PCH·CSO 갱신은0개다.
필수 runtime 파일·Navigation 참조·Item/Valtan 보상 검사에서 누락·무효 입력이 없었다.
기존 C4819/C4828과 외부 DirectXTK LNK4099 경고는 남아 있으며 경고0건을 의미하지 않는다.

BernEntranceCamera JSON의16키 FOV45도와 나머지 값 보존, Client project/filter XML parse,
파일등록2934개 일치·미정의 filter0·C++ 물리경로 누락0, 빈 SolutionFolder2줄만 제거된 것을
확인했다. 최초32개와 PR 변경 경로가 같고 바이너리·Resources·빌드 산출물은 포함하지 않았다.
전체 변경의 `git diff --check`가 통과했으며 셰이더 입력668개의 바이트와 수정시각을 보존했다.

데이터 publish·Core/FullDiagnostic·Client/Server 실행·UI 조작·화면 캡처는 수행하지 않았다.
기존 Undo/Redo 구현 `1ef2616b`와 PR #530은 통합 이력에 남아 있다.
실제 F1/F7·컷신 구도·카메라 목록·게임 화면 확인은 사용자 검증으로 남는다.
