# 베른 프레임 병목 분해 및 화면 밖 작업 생략 결과

## G00. 근거와 판정

사용자가 저장한 DirectionalLightOff/On 캡처는 각 CPU 120 frame, valid GPU 116 frame이며 raw CPU drop은 0이다. 파일명과 사용자 설명으로 ON/OFF를 구분한다. 구형 capture metadata에는 directional flag가 없다. export camera는 같지만 history 전부가 같은 장면이라는 보장은 없다.

| 평균 | OFF | ON |
|---|---:|---:|
| 프레임 간격 | 69.240ms / 14.44fps | 68.235ms / 14.66fps |
| CPU frame | 68.585ms | 67.653ms |
| Client.Update | 22.976ms | 23.132ms |
| Engine.Update | 19.087ms | 19.167ms |
| NonBlend CPU | 21.392ms | 21.130ms |
| 전체 model animation | 4.109ms | 4.101ms |
| 환경 effect VisibleUpdate | 6.883ms | 6.963ms |
| ImGui BuildAndSubmit | 8.255ms | 7.351ms |
| 외부 viewport Present | 4.002ms | 2.814ms |
| Render.Lights CPU | 0.933ms | 0.921ms |

중첩 CPU scope는 합산하지 않는다. NonBlend GPU timestamp 경과는 28.888/28.149ms이나 CPU 제출 대기를 포함할 수 있으므로 GPU 사용률로 해석하지 않는다. 실제 shadow draw는 둘 다 0이다. 현재 근거에서 directional 연산만을 주병목으로 볼 수 없다.

맵 placement 50,021개, batch 16,424개, fallback object 1,222개, visible instance 973개다. NONBLEND queue는 16,844개지만 실제 전체 draw는 약 1,211회(그중 instanced 858회)다. 화면 밖 맵은 대부분 Render 진입 후 제거되며 16,424개 모두 GPU draw한 것은 아니다. stationary camera의 cullingCandidates=0은 revision cache 재사용을 뜻한다.

NPC는 기존 코드에서 authored visibility만 검사하여 camera 밖에서도 animation/pose와 render 제출을 계속했다. Bern 저작본에는 enabled 53 placements/48 models가 있다. 같은 모델의 중복은 5쌍뿐이어서 단순 NPC instancing으로 전체 CPU 병목이 해결된다는 근거는 없다.

## G01. 변경 범위

초기 live source에는 고빈도 누적 profiler를 추가했다. 고정 배열 13분류로 map visibility/material/pass/draw, 개별 map/water, NPC update/late/render, ambient visibility/advance/submit의 calls와 inclusive ms를 수집한다. 상세 raw scope cap과 별도로 동작하고 capture OFF에서는 clock/string intern/lock/heap allocation이 없다. JSON v3 `cpuWork`와 F7 Workload 표에 연결했다. layer별 네 update phase와 final camera submission도 계측한다.

추가 최적화는 사용자 빌드 중 `out/BernVisibilityCandidate20260930`에 격리 준비했으며, 후속 사용자 지시로 소스 반영 및 Debug/Release Product build가 승인됐다. 설치·빌드의 실제 결과는 G03에 기록한다.

- GameObject/Layer 기존 경로에 opt-in 최종 카메라 제출을 추가한다. frame providers 뒤 world pass 전에 실행하며 별도 object manager나 모델 런타임을 만들지 않는다.
- 맵 배치와 개별 맵은 최종 camera에서 판정한 뒤 보이는 render group만 제출한다. 정적 맵 16,424개의 빈 Priority/PostPhysics 호출 32,848회를 매 frame 생략한다. shadow OFF는 caster 준비/제출도 생략하고 ON은 light volume으로 별도 판정한다.
- town NPC는 실제 WModel 모든 clip key와 skeleton/skin 영향 bounds에서 계산한 보수적 sphere로 판정한다. 화면 밖에서는 animation clock만 진행하고 pose/bone 계산과 render 제출을 생략한다. 다시 보이는 frame에 현재 시각의 pose를 평가한다.
- 외부 bone 소비자, 무기/모자, action/effect cue, afterimage, 지원하지 않는 geometry/material는 기존 경로를 유지한다. network interpolation/collider/gameplay clock을 camera로 중단하지 않는다. 물과 바람의 scalar shader clock도 유지한다.
- 신규 counter는 map cache/rebuild/empty/visible/upload, NPC culling candidates/culled/deferred pose evaluations, ambient unbounded updates다. final submission으로 이동한 visibility 시간은 MapBatchRender와 분리해서 읽는다.

## G02. 집중 검증

- profiler core Debug/Release 격리 probe: 100,000 scopes 무누락, capture OFF QPC 0, worker/invalid/stale frame/epoch/instance guard, inclusive nesting, JSON 13분류 유한 값 PASS.
- Layer actual-method probe: 기본 phase, opt-in, 순서, 중복 한 건 제거, update 중 append, 16,424개 빈 phase 생략 PASS.
- map actual-method probe: final camera payload 한 번 준비, Render 재사용, empty queue 제거, camera 밖 shadow 보존, shadow OFF, invalid/upload failure의 기존 경로 보존 PASS.
- NPC lifecycle actual-method probe: 화면 밖 120 frame, 같은 frame 재진입, 중복 final, admission 해제, 외부 bone 소비, authored hide/reentry, final 단계 미실행 다음 frame 복구 PASS. model/camera는 stand-in이며 실물 envelope 검증과 구분한다.
- 후보 Model.cpp/Animation.cpp의 실제 translation unit Debug/Release 최소 컴파일 4건 PASS. 정규 Product build와 별개다.
- 최종 homogeneous 처리의 실물 검사: 설치 46/46 model, 2,305 clips, 77,627,067 keys admission PASS. 전 clip 5시점의 influence AABB corner 5,923,960개가 모두 보수적 sphere 안이다. 수학적 all-key bound와 유한 표본 비교를 구분한다. radius 1.373~8.52877m이며 실제 shader와 같은 positive homogeneous 좌표를 소비한다. 후속 Model.cpp Debug/Release 최소 컴파일 2건도 PASS다.
- 저장 ON/OFF export camera와 현재 placement의 오프라인 예상은 설치된 50개 중 48개 제외/2개 유지다. runtime counter나 FPS 측정은 아니다. 저작상 53개 중 3개가 쓰는 WModel 2종(`Npc_MN_RHKP_02_2`, `Npc_NP_LRKK_01`)은 현재 Resources에 없어 검사 대상에서 제외했다.
- 최초 envelope 계산은 격리 O2 probe에서 46종 합계 977.45ms, 중앙값 9.34ms(동시 빌드 중)이므로 steady frame 비용과 구분한다. root-motion 설정을 마친 CNpc 초기화에서 cache를 준비하고 평상시 Update는 재사용하도록 했다. 캐시 조회 probe 중앙값 3.394ns를 실제 전체 NPC Update 시간으로 해석하지 않는다.

## G03. 설치·빌드 상태

소스 반영 전 baseline SHA와 최신 파일을 비교한다. Model.cpp에서 다른 작업이 추가한 landscape material ambiguous 검사 1줄을 발견했으며 같은 후보에 보존하여 병합한다. 다른 팀원의 변경을 되돌리지 않는다.

초기 Debug Product 시도는 사용자의 동시 빌드가 Client CL.write.1.tlog를 점유해 D8040/MSB6003으로 실패했다. 당시 자동 종료·Clean/Rebuild·tracking 삭제는 하지 않았다. 기록은 `out/BuildPipeline/runs/20260930T141012613Z-debug-product.json`이다. 이것을 최적화 EXE 성공 근거로 사용하지 않는다.

후속 승인 뒤 24개 후보의 baseline SHA와 `git apply --check`를 확인하고 원자적 소스 교체를 완료했다. 기록은 `out/BernVisibilityCandidate20260930/installation-result.json`이다. 초기 profiler를 포함한 변경 파일의 `git diff --check`와 제품 4개 vcxproj XML parse도 PASS했다.

실물 inverse-bind에 `_44=0.999999940395`가 있어 최초 envelope의 엄격한 affine 검사가 NPC를 전부 기존 경로로 돌리는 결함을 추가로 발견했다. tolerance를 임의로 완화하지 않고 positive homogeneous w로 정확하게 나누어 같은 skinning 좌표의 bounds를 계산하도록 수정했다. rest/pretransform은 기존 affine 계약을 유지한다. 첫 3개 실물 모델의 admission과 전 clip 5시점 influence AABB corner 검사가 PASS했으며 전체 모델 조사를 계속한다. 후속 설치는 `homogeneous-installation-result.json`이다.

첫 통합 Debug의 Engine/Shared는 PASS했으나 동시에 진행 중인 워터팡 AI 작업의 `GameRoom_MaharakaAI.cpp`가 프로젝트에 먼저 등록되고 아직 생성되지 않아 Server C1083으로 중단됐다. 로그는 `out/BuildPipeline/runs/20260930T143612682Z-debug-product.json`이다. 다른 작업의 프로젝트 항목을 제거하지 않았다. 이후 해당 파일이 생성된 것을 확인했고 Release 및 최종 Debug 재검증을 진행한다.

사용자의 추가 요청에 따라 마하라카 환경 조명 복원과 반사/mip 보완도 통합 범위로 확인했다. G08 설치 7파일·감사 입력 6파일 hash, 맵 정본/게시 4쌍, 현재 RenderingProfiles rev88 의미 일치 PASS다. 리소스 222개/26,887,404 bytes의 설치본과 GBResources hash·크기가 전부 일치하고 반사 DDS25개 full mip 및 TGA2개 입력이 정상이다. 최초 rev86 이후 저장된 마하라카 bloom OFF와 발탄 bloom 튜닝, scene fog gate를 보존했다.

제품 FxCompile 대상은 Engine 30개+Client 224개=254개/config다. Product runner는 별도 shader closure 전에 return하므로 통합 빌드 뒤 `Test-CompiledShaderClosure.ps1 -Configuration <Debug/Release> -Modules Product`를 별도로 실행한다. 동적 SourceGroup 소비자와 Engine→Client 30개 CSO 배포 hash, source/include freshness도 별도로 확인한다. 최종 결과는 진행 중이다.

베른·캐릭터 선택 지형 복원 인계도 포함한다. [해당 RESULT](2026-09-30_BERN_AND_CHARACTER_SELECT_TERRAIN_RESTORATION_RESULT.md)의 최신 receipt 우선 병합으로 설치 322경로 hash 불일치 0, 누계 리소스 371개/85,445,734 bytes의 설치본·GBResources 일치를 재확인했다. Landscape 리소스 134개와 기존 PNG84개, 정본/게시 23,200재질 중 family14 42행, 최종 shader source6파일 hash 및 ordinary/instanced 등록이 유지된다. Model.h/.cpp는 지형 final source와 일치하는 baseline에 이번 animation 최적화를 추가한 상태이며 지형 consumer를 보존했다. Bern runtime mapset은 인계 문서의 `d11bb2050ea951ceb7d46a4b8665243e1aa20141b8be12219e3d4ed34b2d6862`와 같다.

## G04. 남은 실측 경계

새 EXE의 FPS, 실제 NPC cull 수, 화면/그림자/재진입은 아직 사용자 실행으로 확인하지 않았다. 60/100fps를 보장하지 않는다. F7에서 같은 위치·camera·창 조건으로 120 frame을 저장하면 cpuWork, Layer scopes, queue/draw/animation counts로 비교할 수 있다. Detailed per-draw CPU scopes는 OFF를 유지한다.

환경 effect의 약 7ms와 ImGui의 약 7~8ms는 독립적인 잔여 비용이다. Bern 항구 anchor marker는 별도 sustained effect 경로여서 VisibleUpdate 수치만으로 비용을 귀속할 수 없다. unsupported mesh/native/trail 등 bounds 없는 effect는 기존 fail-open을 유지한다. 바다 shader와 항구 전체가 이번 변경으로 모두 중단된 것으로 보고하지 않는다.
