# Character Select 상시 프레임 비용 최적화 구현 계획

작성일: 2026-09-12. 기준: `codex/lancemaster-effect-admission-fix`, `50e4c2579d98cedf3dd25a01a6428463f9623965`.

사용자 요청은 Debug x64 Character Select의 50FPS/다른 PC 10FPS 회귀 전수 조사와 수정이다. 앞선 캐릭터 조작감·Composition 계획의 코드 반영 금지와 별개로 이번 성능 수정은 승인되었다. 최신 요청에는 EXE 실행과 profiler 측정이 포함된다. 기존 미커밋 Effect 변경과 조작감 계획은 보존한다.

## G00. 현재 실행 비용과 측정 조건

`Client/Default/Client.cpp`의 main loop는 60FPS gate를 사용한다. 현재 상태로는 실표시 100FPS를 목표 증거로 사용할 수 없다. 목표는 같은 Debug x64·해상도·캐릭터·카메라에서 frame interval, CPU 처리 시간, GPU stage 시간을 줄이는 것이다. gate 해제만으로 최적화 성공을 주장하지 않는다.

Character Select의 `MAP_LOAD_SCOPE`는 X [-792,-750], Z [158,218]이며 Loader와 runtime이 동일 descriptor를 소비한다. 실제 runtime803 placement 중779개를 사용하고 visible746, static WModel58종, batch71개다. animated map model과 bounds 누락은0개이며 WModel 합계8.03MiB다. 맵 전체 로드 회귀, idle 파일 재로드, 초기 NPC 군중은 확인되지 않았다. 기존 cull/instance payload 재사용을 유지한다.

09-11 20:03:48 캡처는 mapPlacements779의1200 frame에서 CPU19.918ms, interval20.402ms, GPU frame20.313ms다. CPU InputAndUI5.219ms, Render.NonBlend3.512ms, Shadow1.922ms, profiler panel평균1.262ms를 관측했다. GPU Draw5.866ms, Lights1.950ms, SSAO0.475ms다. GPU frame 전체에는 CPU 제출 공백이 포함될 수 있으므로 GPU utilization으로 해석하지 않는다. 이 캡처는 그날23:31 변경 이전이며 현재·다른 PC의 측정으로 대체하지 않는다.

Profiler는09-11 확장되었으며 F1 visibility와 Capture enabled는 별개다. 명시 Capture OFF는 scope/counter 수집이 즉시 반환하고 steady-state GPU query를 발행하지 않는다. 격리 Debug CPU probe에서 OFF scope89.9ns, ON1024scope frame0.734ms를 관측했다. 이 마이크로벤치로 게임 FPS를 환산하지 않는다.

## G01. Shader_Deferred.hlsl에서 조명 수신 픽셀 먼저 선별

수정 파일은 `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`이다. 기존 Engine shader producer와 Client 배포를 사용하며 새 C++/project/filter 등록은 없다.

`CRenderer::Draw_Lights`는 ordinary light pass 외에 각 source material row마다 같은 light를 다시 제출한다. 지금 `PS_MAIN_DIRECTIONAL`, `Resolve_LocalLight`는 source row 불일치를 normal/depth sample, world reconstruction, static shadow 및 directional PCF 뒤에 판정한다. 결국 버리는 화면 대부분의 픽셀에 이 연산이 반복된다.

두 entry에서 기존 `Reject_LightReceiver` 뒤 point `Load`로 marker와 row를 먼저 판정한다. source pass의 marker5/동일 row가 아니면 기존 비교와 같은 `!=`로 discard하고 ordinary pass의 marker5는0을 반환한다. 뒤쪽 중복 depth 선언과 marker5 반환만 제거한다. 유효 픽셀의 linear sample, 재질 연산, 조명·반사·그림자 수치와 native attenuation 판정은 보존한다. NaN, 배경, spot 원점도 기존 blend/depth 계약 아래 같은 결과를 유지한다.

종료 증거는 fx_5_0 및 directional/point/spot ps_5_0 격리 컴파일, DXBC에서 비싼 sample보다 선행하는 row 분기 확인, 같은 조건의 runtime profiler 비교다. shader 검사만으로 visual fidelity나10FPS 해결을 승인하지 않는다.

## G02. Profiler 정지 상태의 반복 집계 제거

소유 파일은 `Client/Private/ProfilerTool.cpp`, `Client/Public/ProfilerTool.h`다. 기존 Engine profiler read model과 panel을 유지한다.

정지된 history를 패널이0.5초마다 다시 CPU/GPU 집계하는 확정 낭비를 제거한다. 표시 window 변경, reset, 새 completed frame 또는 늦게 회수된 GPU 결과처럼 실제 표시 데이터가 변한 경우에는 갱신한다. Capture를 끄는 것과 패널을 숨기는 의미, 저장 worker의 숨김 상태 완료 처리는 유지한다. 새 전역 runtime이나 별도 측정 framework는 만들지 않는다.

현재 probe에서120frame×1024scope 집계4.10ms,1200frame42.19ms다. 수정 후 동일 source probe 또는 기존 집계 경로를 사용해 정지 상태 재집계 횟수와 재개·reset·window 변경을 확인한다. 기존 profiler 저장 JSON 형식은 보존한다.

## G03. 제품 실행 검증과 결과 기록

현재 저장소에서 외부 Visual Studio/MSBuild가 진행 중이므로 같은 output에 빌드를 겹치지 않는다. 공유 빌드가 끝난 후 변경 producer를 포함하는 최소 Debug build와 필요 EngineSDK 동기화를 수행한다. 원본 및 배포 CSO 일치와 `git diff --check`를 확인한다.

실행 조건은 Lobby→Server 승인 Character Select, 같은 캐릭터·해상도·카메라·idle 구간이다. profiler OFF/F1 closed, ON/F1 closed, ON/panel open을 구분한다. capture는 파일로 남기고 실제 바이너리 시각·하드웨어·조건을 RESULT에 기록한다. 사용자가 요청한 profiler 실행 측정만 수행하며 원작 visual fidelity 승인은 사용자의 관찰로 남긴다.

UI 상시 비용은 실제 호출자와 frame scope를 조사한 후 근거가 있는 수정만 이 계획에 추가한다. 정적 map 상한이나 다른 날짜의 capture로 특정 PC의 원인을 단정하지 않는다. 결과 문서는 구현, 컴파일, 실제 실행 측정, 다른 PC 미확인 사항을 분리한다.
