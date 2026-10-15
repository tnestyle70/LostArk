# 베른 컷신 거리·geometry 계측 결과

## 현재 완료 범위

사용자가 HLOD 선택 조건을 계속 바꾸는 대신 실제 거리·draw 작업량을 먼저 기록하도록
방향을 변경했다. F7 컷신 기록, FPS 옆 GPU geometry 표시, 기존 JSON exporter 연결을 반영했다.
Release/Debug x64 Product 빌드·배포와 소스/바이너리 대조를 완료했다.

기본 카메라가 복구됐다는 것은 사용자의 이전 Release 확인이다. 컷신 초·중반 드랍이 남았다는
것도 사용자 확인이며 새 Profiler JSON은 저장되지 않았다. 이번 변경은 그 원인을 재현 가능한
표본으로 남기는 계측이다. 실제 베른 FPS 개선·HLOD 활성 성공을 이번 구현 성공으로 대신하지 않는다.

## 사용 경로와 저장값

1. 베른에서 F7 → `컷신 재생 + 거리 기록`. 이미 재생 중이면 현재 위치부터 기록한다.
2. 기본 간격100ms, 거리0~250m. 창을 닫아도 기록하고 종료 뒤 자동 저장한다.
3. 저장 성공 경로는 Profiler 창에 표시한다. 기본 폴더는 `Client/Bin/ProfilerCaptures`,
   파일 이름은 `bern-distance` 또는 입력 이름에 이 접미사가 붙는다.
4. `최근 표본 거리 / draw 상세`에서 거리·직전 표본 대비 변화, 실제 제출 경로·LOD·인스턴스·indices를 본다.

`bernCutsceneGeometry`는 실제 관측 wall time과 예약 tick, 건너뛴 tick, 실제 Level cue 시간,
카메라 행렬과 위치, source별 거리·변화량·속도, CPU/GPU/HLOD 경로 및 draw 결과를 보존한다.
일반1200-frame history와 독립되어 첫부분 퇴출을 구분한다. 최대120초/1200표본/200만source행,
표본당 source/draw각2만개 제한과 손실을 기록한다. 제한 도달·중단도 자동 저장하며 실패하면
불변 표본을 유지하고 다시 저장할 수 있다.

거리 기준은 placement/model bounding sphere 중심(m)이며 sphere 근접 거리도 함께 저장한다.
submesh 표면 최단거리를 계산한 값은 아니다. 거리 필터는 source 목록에만 적용하고 mapDraws는
전체 static map color 제출이다. standalone fallback은 개수로 구분한다. GPU source별 생존/LOD는
args만으로 알 수 없어 후보 거리와 cohort/bin 실제 집계를 분리한다. CPU·HLOD source ID 역시
기존 visible index로 확인된 경우만 출력하고 unknown은 null로 보존한다.

노란 FPS 오른쪽 `Vtx(VS)`는 GPU vertex shader 실제 실행 횟수(TK/UI 포함), `Idx`는 Engine이
계측한 indexed draw의 인스턴스 포함 참조 수(TK/ImGui 내부 draw 제외)다. 두 수의 범위는 다르다.
버퍼 용량·고유 정점·최종 보이는 픽셀 수가 아니다. `GPU -NF`로 비동기 완료 지연을 표시하고
유효하지 않은 수는 `--`로 표시한다. Capture OFF에도 갱신한다. 평소 cinematic HUD 숨김을
유지하되 사용자가 시작한 컷신 거리 기록 동안 진단 줄을 표시한다.

## 실제 변경 파일과 코드 위치

| 파일 | 실제 변경 부분과 책임 |
|---|---|
| `Engine/Public/Profiler.h`, `Engine/Private/Profiler.cpp`, `Engine/Private/Profiler_Geometry.inl` | `Begin/End_GeometryFrame`, `Resolve_GeometryFrames`, `Get_GeometryStats*`: Capture와 독립된8-slot query/readback, 실제 frame 번호, scalar128/detail16 보관. direct index tally와 GPU VS/indirect args를 결합한다. |
| `Engine/Private/Mesh.cpp` | `Render_InstancedIndirect`: 성공한 GPU 명령의 argument buffer/offset을 관측기에 전달한다. |
| `Client/Public/MapGeometryDiagnostics.h` | 소유 snapshot, source 거리·현재 상태, actual draw와 GPU frame/buffer/offset 연결 자료형. |
| `Client/Public/MapPlacementRuntime.h`, `Client/Private/MapPlacementRuntime.cpp` | `Begin/End/Cancel_GeometryDiagnosticFrame`: sample 프레임에만 관측을 열고 render 후 카메라와 기존 source 상태를 복사한다. |
| `Client/Public/MapStaticBatchObject.h`, `Client/Private/MapStaticBatchObject.cpp` | frame-state의 optional diagnostic 소유권과 `Record_GeometryDiagnosticDraw`: 실제 normal/identical/lighting-bank draw 직후 같은 LOD view·인스턴스 수를 기록한다. |
| `Client/Private/MapGpuScene.cpp` | `Try_Render`: 성공한 indirect draw의 mesh/LOD/cohort/bin과 frame/buffer/offset을 기록한다. 실제 survivor 수는 readback 완료 후 결합한다. |
| `Client/Private/MapStaticChunkObject.cpp` | `Render`: 실제 native HLOD 제출 indices와 선택된 원본 placement ID를 기록한다. |
| `Client/Public/ProfilerCutsceneCapture.h` | 컷신 표본과 종료 사유의 소유 값. exporter로 넘긴 뒤 불변이다. |
| `Client/Public/ProfilerTool.h`, `Client/Private/ProfilerTool.cpp`, `Client/Private/ProfilerTool_Cutscene.inl` | 버튼·간격·범위 UI, 기존 replay 요청, 실제 cue 상태 관측, fixed wall tick, 미관측 tick 보존, drain·자동 저장·최근 표본 표. |
| `Client/Public/Level_Bern.h` | 기존 cue active/seconds/duration 및 Level-owned runtime의 관측 접근자. |
| `Client/Private/MainApp.cpp` | world render 전후 sample 연결, `RenderFpsText`의 Vtx(VS)/Idx·지연 표시, 기록 중 진단 줄 표시. |
| `Client/Public/ProfilerCaptureIO.h`, `Client/Private/ProfilerCaptureIO.cpp`, `Client/Private/ProfilerCaptureIO_Cutscene.inl` | 기존 async atomic exporter에 선택적 컷신 값을 추가. stable source 이전 관측과의 delta 및 exact-frame args 결합. |
| Client/Engine `.vcxproj`, `.filters` | 신규 header2개/inl3개만 등록. 별도 C++ 번역 단위를 추가하지 않는다. |

HLOD candidate/cost gate, GPU/CPU 선택,32m batch partition, 카메라/FOV, rendering option·fog/AA,
맵 visibility·Resources·Data JSON은 바꾸지 않았다. 새 거리 수집은 재visibility 준비나 lazy claim
선택을 호출하지 않는다. 전체 적용 후 코드 정본은 같은 날짜의 PLAN에 보존했다.

## 실행한 검증

- 실제 후보 Profiler를 링크한 WARP 콘솔: Release/Debug 각각40 assertions PASS.
  Capture OFF에서 실제 direct/indirect draw, 같은 args 반복, 같은 offset의 다른 buffer,
  frame join, invalid args의 unknown, 기존 Capture ON 중첩 pipeline query를 검증했다.
  입력 결과 VS27/direct indices3/indirect54/합계57, 다른 frame의 VS3/indices3을 확인했다.
  Client/Level/UI/스크린샷/Flush는 사용하지 않았다.
- 실제 serializer 본문 MSVC C++20 /W4 compile/run 및 JSON parse PASS.
  최초 delta null, -2m/400ms=-5m/s, 동일 frame args의15indices, 다른 frame unknown,
  HLOD 단일 LOD 부재, NaN·취소 거절, optional 미지정, source ID known/unknown을 확인했다.
- Map CPP4개 `/Zs` PASS. ProfilerTool/ProfilerCaptureIO Release `/Zs` PASS.
  후보/실제 헤더 경로가 섞인 첫 Debug 단독 검사에서는 재정의가 발생했으므로
  최종 실제 제품 Debug 빌드는 PASS하여 같은 물리 경로의 헤더 연결을 확인했다.
- 기존 `Tools/Profiler/test_analyze_capture.py`:6 tests PASS.
- 최종 후보29개 freshness/hash 및 scoped `git diff --check` PASS, project/filter XML parse.
  ProjectReference GUID metadata는 원래 저장본과 동일한지 별도 확인했다.
- 독립 검토에서 자동 저장 이름 길이와 관측 할당 예외를 보완했다. 긴 UTF-8 이름은
  중간 바이트를 자르지 않고 안전한 기본 파일 이름으로 저장한다. 관측 예외는 렌더로 전파하지 않는다.

증거 폴더: `out/BernCutsceneProfiler20261013`의 geometry probe 로그, serializer-native/result.json,
independent-review.md, root/map-syntax.log, analyzer-tests.log, final-candidate-hashes.json,
preapply-verification.json, applied-diff-check.txt, final-verification.json.

## 제품 빌드

정상 증분 `Invoke-BuildAndRegression.ps1 -Profile Product -MaxCompilerProcesses 2`를 사용한다.
첫 Release는 Engine/Shared/Server 이후 Client 프로젝트 등록 metadata 오류(MSB3107)로 실패했다.
새 ItemGroup을 최상위 Project 닫기 전에만 넣도록 등록 스크립트를 고치고 참조 GUID를 재확인했다.
Clean/Rebuild/캐시 삭제·Client/Server 종료를 사용하지 않았다.

| 구성 | 결과 | 최종 Product 시간 | receipt |
|---|---|---:|---|
| Release x64 | PASS | 338.507초 | [20261013T135415721Z-release-product.json](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261013T135415721Z-release-product.json) |
| Debug x64 | PASS | 560.766초 | [20261013T140346250Z-debug-product.json](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261013T140346250Z-debug-product.json) |

두 구성 모두 Engine→Shared→Server→Client 및 runtime 파일·navigation 참조·Item/Valtan reward
검사를 통과했다. 데이터 publish는 수행하지 않았다. 최종 Client 컴파일은 Release92 OBJ,
Debug101 OBJ이며0 CSO 갱신이다. 인코딩 C4819/C4828과 외부 PDB LNK4099 경고가 남아 있다.

설치 파일은 `Client/Bin/Release/Client.exe`, `Client/Bin/Debug/Client.exe`다.
두 구성의 Engine 원본 DLL과 Client 배포 DLL SHA256이 일치하며 `Engine/Public/Profiler.h`와
SDK 설치 헤더도 일치한다. 29개 최종 후보와 live source hash가 일치하고 실제 변경 범위의
diff check를 통과했다. [최종 검증 JSON](C:/Users/tnest/Desktop/LostArk/out/BernCutsceneProfiler20261013/final-verification.json)에
각 바이너리 크기·hash·mtime와 빌드 단계 결과를 보존했다. Client/Server/UI는 실행하지 않았다.

## 사용자 실행에서 남은 확인

실제 Release/Debug F7 버튼과 자동 생성 파일, 기본 카메라의 FPS, 동일 컷신 초·중반을 사용자가
확인해야 한다. native WARP와 serializer 검증은 이 실제 게임 화면·성능을 대신하지 않는다.
배경을 숨기는 변경은 반영하지 않았다. 다음 분석은 저장된 거리 표본과 실제 GPU/CPU 제출량을 근거로 한다.
