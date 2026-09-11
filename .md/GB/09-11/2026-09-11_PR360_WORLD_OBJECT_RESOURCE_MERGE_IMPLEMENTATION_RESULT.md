# PR360 World Object·리소스 통합 구현 결과

## G00. 현재 상태와 기준선

PR #360의 코드·저작 데이터 통합, domain publisher, protocol 검사와 로컬 리소스 복사 및 **2026-09-11 23:23:36 KST Debug Product 빌드·배포를 완료했다.** 최신 사용자 지시는 버그 수정 EXE를 먼저 직접 검증한 뒤 PR merge와 리소스를 공유하는 순서다. 따라서 PR merge/main 동기화와 외부 리소스 공유를 보류했다. 맵 진입 오류·Workbench assertion·호랑이 root motion 수정의 수치·빌드 검증을 사용자 화면 성공으로 확대하지 않는다.

시작점은 `7795a68d867a8df5a840aa3d1f544cef8d825d48`, origin/main은 `1b07290f`였다. 작업 브랜치 `codex/pr360-main-resource-sync`에서 origin/main을 합친 기준은 `89477fd78b24e111789a6dd0aabbcc57c50c08fd`이며, 통합 대상 PR head는 `35ca86d615c2b40eedce908d25c8e3e82a6da259`다. 공통 기준은 `397aab6c`다. Git index의 unresolved 파일은 0개다. 사용자 추가 지시는 빌드한 수정본을 PR에 올리고 EXE 검증 후 사용자가 직접 merge하는 것이다. 기존 PR #360의 head에 후속 merge/기능 commit을 정상 push하는 범위까지 진행하며 에이전트는 merge하지 않는다. PR #361의 오늘 작업도 7795의 ancestry로 이 통합본에 포함된다.

계획은 [구현 계획](2026-09-11_PR360_WORLD_OBJECT_RESOURCE_MERGE_IMPLEMENTATION_PLAN.md)을 따른다. 실행 근거는 `out/PR360ResourceMerge20260911`에 있으며 별도 제품 manifest나 배포 완료 조건을 추가한 것은 아니다.

## G01. World Object와 Client·wire 계약

World Object의 기존 `objectMotion.emissions`가 개별 위치 offset, yaw, 지연을 소유하도록 PR 변경을 통합했다. 기존 seeded jitter, spawnHalfExtents, BOSS/PLAYER anchor basis와 async character replacement를 보존한다. collider는 `worldEmissionIndex`로 같은 WORLD occurrence의 정확한 emission을 선택한다. Character의 attachment slot 소비, Mario curse notice와 기존 status 표시 순서를 함께 유지했다.

두 branch가 서로 다른 wire 변경에 protocol 79를 사용했으므로 통합본은 **80**이다. 오늘의 CardMaze ENTRY_HIDDEN/VALID_FLAGS31과 PR의 Mario popped-ball/curse-release mask를 같은 writer/reader 및 Shared/Server 소비자에 연결했다. 실제 NetworkProtocolHarness 실행은 `protocol_run.log`의 `failures : 0`으로 끝났다. 이전 protocol 79 실행 중인 Client/Server가 새 계약을 소비했다고 기록하지 않는다.

PR의 Mario pop smoke 3종은 첫 pop의 main-thread JSON 로드에서 기존 `WorldSequence::Prepare_AreaLoad` worker의 immutable snapshot 준비로 옮겼다. runtime은 같은 `m_EffectSnapshots`의 준비 결과를 조회한다. 실패·취소·대상 불일치가 commit 전에 처리되는 정적 경계와 실제 JSON parse를 확인했다. `Play_Leaf`의 최초 renderer/GPU 준비까지 이동하거나 실제 프레임 개선을 측정한 결과는 아니다.

Client 수식 대조는 기존 조합 1,200개와 authored WORLD 조합 400개에서 최대 행렬 오차 0이었다. 이는 Python/Numpy 수식 및 source 연결 검사이며 C++ 실행·draw 검증이 아니다. 자료는 `client_merge_review.md`, `client_matrix_and_preservation.json`, `smoke_boundary_checks.json`이다.

## G02. 정본 데이터 병합과 게시

stable ID별 세 버전을 비교해 오늘의 관문 Flow, 컷신·카메라·이펙트와 PR의 fire/hook emission, Mario timing, 칼날·빙고 망치를 합쳤다. revision 숫자만으로 문서를 선택하지 않았다.

| 항목 | 통합 결과 |
|---|---:|
| WorldSequence revision | 672 |
| objectResources / templates / instances | 38 / 178 / 233 |
| Composition revision | 331 |
| 저장 patterns / worlds | 31 / 36 |
| Map placements | 3,368 |

오늘의 PATTERN27은 Gate2 불쏘기로 유지하고, PR에서 같은 번호를 쓴 Gate3 칼날은 PATTERN31과 world19로 옮겨 내부 occurrence ID까지 함께 연결했다. 오늘 Gate2 intro의 23 templates/23 instances와 18개 추가 resource, PATTERN27~30, 두 patternFlows를 보존했다. PR 빙고 망치의 4방향·20 anchors, 심어진 폭탄의 4 slots와 칼날 motion도 기존 WorldSequence에 연결했다.

PATTERN18은 WORLD78개를 7개로, PATTERN19는 18개를 1개로 묶으면서 원래 emission을 보존한다. 두 패턴의 총 36 collider가 각각 hook emission0..17을 한 번씩 선택하고 시작 지연과 5,600 ms 수명을 유지하는 것을 검사했다. Map은 오늘의 assetId/transform을 보존하며 PR이 숨긴 SL05 장식 조명 72개에 visible0만 적용했다. placement 수와 stable ID는 유지했다.

정본 JSON parse·ID uniqueness·reference closure·오늘 행 equality와 실제 38개 model 파일 존재 검사는 PASS다. 기존 WorldSequence authoring 검사 30개도 PASS다. 상세는 `SEMANTIC_DATA_MERGE_RESULT.md`, `semantic/reference_closure.json`, `world_sequence_tests.log`에 있다.

명시 KoukuSaydon publisher의 네 domain은 모두 PASS였다.

| domain | 실제 출력 |
|---|---|
| koukusaydon.product | source revision331, saved31 / Product27 patterns, 202 stages |
| map.kakulsaydon | Area 8파일, v2 placement3,368개 |
| world.gameplay | KAKULSAYDON_ARENA world112개, spawn-group5개 및 Client 표시 출력 |
| gameplay.balance | 통합 gameplay/boss 수치 출력 |

저장 패턴 31개 중 Product는 27개다. publisher exit0을 모든 저장 패턴의 제품 admission으로 확대하지 않는다. `publish.log`에는 기존 player hit-shape 부분 coverage 68/76 경고도 남아 있으며 warning-free 결과가 아니다.

## G03. CY·오늘 변경 리소스 팩과 실제 설치

사용자가 확정한 출력은 **`C:/Users/user/Desktop/GB_Resources`**다. `C:/Users/user/Desktop/GB/GB_Resources`와 `C:/Users/user/Downloads/CY_Resources` 원본은 보존했다. 이 팩은 CY 실행 입력과 오늘 오후 2시 이후 변경분이며 전체 Resources 사본은 아니다.

| 선택 | 파일 수 | 근거 |
|---|---:|---|
| CY 실행 입력 채택 | 3,665 | 유효 WModel/texture 입력 |
| CY와 겹친 로컬 맵 우선 | 305 | 기존 GB와 SHA 동일, WMSH 및 일부 material 교정 보존 |
| 오늘 신규·변경 로컬 추가 | 63 | 수정시각 후보43 + 과거 mtime이 보존된 신규 Gate2Intro DDS20 |
| 최종 출력 | **4,033** | **1,755,169,993 B** |

확장자는 WModel484, DDS3,216, TGA332, PNG1이다. TGA/PNG는 기존 material loader가 지원하고 일부 모델이 직접 참조하므로 실행 입력으로 유지했다. 원본/backup 40개는 팩에 넣지 않았으며 CY에서 삭제하지 않았다. 현재 직접 참조가 없는 신규 맵 실행 자산도 사용자 요청에 따라 보존했다.

변경된 Character 모델 4개는 로컬 WANM의 duration/tick rate/모든 key time을 기존 retimer로 30 Hz 정규화하면 **파일 전체 SHA가 CY와 동일**했다. 총137개 clip의 시간 정규화 차이이며 geometry/skeleton/material과 quaternion·position·scale 값은 유지된다. 이 근거로 다음 4개를 백업 후 CY bytes로 교체했고 칼날 1개를 추가했다.

- `Character/KoukuSaton/MN_CDMD_00/MN_CDMD_00.wmodel`
- `Character/Monster/MarioOriginal/CDMD/CDMD.wmodel`
- `Character/Monster/MarioOriginal/REUP/REUP.wmodel`
- `Character/Monster/MarioOriginal/RHKP/RHKP.wmodel`
- 신규 `Character/Monster/MarioOriginal/REUP/WP_MN_RHKP_07_Static.wmodel`

이전 4개는 `resource_backup`에 상대 경로 그대로 보관했다. source·기존 destination hash를 고정하고 임시 파일에서 원자 교체했으며 최종 5개 모두 CY hash와 일치했다. Tiger/Horse의 기존 quaternion 교정 모델을 이 파일들과 혼동해 덮지 않았다.

신규 맵 **697개(WModel101 / DDS590 / TGA6)**도 실제 `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED` 아래에 추가했다. 미확인 목적 파일 0, 새 팩·신규 맵 덮어쓰기 0, 모든 복사 후 SHA 검증 PASS다. 최종 재개 실행에서 팩의 이미 같은 97개는 건너뛰고 3,936개를 복사해 총4,033개를 확인했다. Character 4개 교체는 앞서 승인한 별도 작업이므로 이 덮어쓰기0 집계와 구분한다.

선택·검증 자료는 `resource_union_plan.json`, `resource_union_copy_result.json`, `runtime_character_install_result.json`, `character_timing_semantic_comparison.json`, `local_since_14kst_final.json`이다. Resources payload와 out 자료는 Git에 추가하지 않았다.

## G04. 통합 중 보고된 오류와 focused 검증

`client-session-70968.jsonl`의 world5 진입 두 번은 승인 후 약6초에 `CLIENT_LOAD_FAILED`로 돌아왔고 상세는 `Map: product load scope`뿐이었다. 최초 조사 당시 실행 `.mapplacements`가 HEAD/PR의 LFS pointer 충돌이어서 실제 header parser가 거부하는 상태였다. 같은 Area worldsequences에도 충돌이 남아 있었다. root publisher 이후 정상 v2 placement3,368개와 두 파일의 충돌 marker0을 확인했다. `Loader.cpp`에는 실제 placement parser 사유와 빈 scope 이유를 기존 recovery 진단까지 보존하는 변경을 추가했고, 고유 out의 Debug focused `/c`와 diff 검사는 PASS였다.

Action Workbench에서는 WORLD lane 추가 후 render order는 8개인데 subrow cache는 고정7개인 확정 소스 결함을 수정했다. cache는 enum COUNT, render order는 항목 수 추론 및 static_assert, 조회는 표시 ordinal 대신 lane 값을 사용한다. 관련 3개 CPP focused compile과 실제 packing 함수 기반 8-lane CPU probe가 PASS였다. 사용자 assertion의 파일·line/stack은 받지 못했으므로 사용자 스택과 동일하다는 판정은 아니다. 상세는 `out/ActionWorkbenchAssert20260911/result.md`다.

Artist D의 반복 역주행은 원본 `b_root`의 수평 이동과 cue의 authored 이동이 함께 적용되고 loop 경계에서 원본 root X 약3.417 m가 초기화되는 것으로 측정됐다. optional `suppressHorizontalRootMotionBone`을 기존 CModel에 연결하고 두 tiger cue만 `b_root`를 선택했다. X/Z rest 위치 유지·animated Y 보존과 shared/prepared cache invalidation을 검증했다. 실제 CModel 2-clone 3,017 pose 샘플에서 X/Z 잔차0, 원본 대비 Y 차이0, 최소 전진 step +0.00549889 m와 seek/loop 일치를 확인했다. Codec·Renderer focused compile 및 stage/rollback 검사는 PASS다. binary 재쿠킹이나 Engine의 별도 모델 경로는 추가하지 않았다. 상세는 `out/TigerRootMotion20260911/result.md`다.

위 긴급 수정과 이어지는 로딩·World picking 작업은 최종 Debug Product 빌드에 포함됐고 Engine/Shared/Server/Client 전체 compile/link와 runtime DLL 배포가 통과했다. Character Select는 모델·Effect 준비 중 기존 캐릭터의 이동·스킬 입력을 유지하고 typed Server class-change 송신 이후 snapshot 교체까지 제한한다. 창술사 Product cue 43개와 authored 참조 43개가 연결됨을 확인했으나 개별 스킬의 화면 판정은 사용자 검증을 기다린다.

우클릭은 기존 CPlayerController의 Request_MoveGoal 성공 뒤 CClickMoveEffect가 준비된 World mouse_click을 재생한다. KoukuSaydon에서는 move_destination 황금 원·화살표 handle도 기존 EffectPresentationService로 재생하고 loop/도착/취소 수명을 관리한다. command 송신 성공은 Server navigation 승인이라는 뜻이 아니다. CPU 수명·연속 retarget 검사, 관련 네 CPP compile 및 project XML 검사는 통과했다. 실제 색·피킹 위치·깊이·크기는 사용자 화면 확인 대상이다. 상세는 같은 날짜의 WORLD_MARKERS, TIGER_HORSE_ANIMATION, LOADING_FREEZE, IMGUI_PROFILING 결과 문서를 따른다.

## G05. 통합 EXE와 남은 사용자 검증

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`의 실제 실행은 exit0, SkipBuild=false, 전체867,183 ms로 완료됐다. Engine124,818 ms, Shared1,342 ms, Server26,404 ms, Client714,496 ms의 각 MSBuild 단계는 모두 PASS다. `missingRuntimeInputs`는 빈 목록이며 SDK·shader·runtime DLL 배포까지 포함한다. 일반 Product 경로가 publisher나 런타임 진단을 실행한 것은 아니며 앞서 별도로 실행한 publisher/수치 검사와 구분한다. 기존 shader X4000/X4717 및 C4819 경고가 남아 있으므로 warning-free 빌드로 표시하지 않는다.

| 실행 산출물 | 크기 | 수정 시각 KST |
|---|---:|---|
| Client/Bin/Debug/Client.exe | 54,017,536 B | 23:23:36 |
| Client/Bin/Debug/Engine.dll | 8,583,168 B | 23:11:14 |
| Server/Bin/Debug/Server.exe | 13,252,608 B | 23:11:41 |

Engine/Bin/Debug과 Client/Bin/Debug의 Engine.dll SHA는 `E59B1122866CFBBA0DB633033337960765F6C3E4370134A7878808010A3AABE4`로 일치한다. Engine/Public/Profiler.h와 SDK/Inc/Profiler.h SHA도 `9C63E97342F8E73D92754E18C7877BA11970FD289ECA3DD82B8B3F5B78B3D014`로 일치해 constructor/member ABI 변경이 SDK에 반영됐다. 로그는 `out/PR360ResourceMerge20260911/product_debug.log`, 실행 receipt는 `out/BuildPipeline/runs/20260911T142336444Z-debug-product.json`이다. 별도 Resource manifest를 완료 조건으로 추가하지 않았다.

빌드 완료 시점 Client/Server process와 7777 listener는 실행 중이지 않았다. LAN 설정은 server-host `192.168.0.14`이며 사용자는 Debug/x64의 `Server + Client` profile을 Ctrl+F5로 시작한다. 사용자에게 실행 준비를 안내한 뒤 제품 소스·산출물을 동결했다. 최종 제품 DLL을 사용한 로딩 성능 재측정과 사용자 직접 버그 재현 검증은 아직 남아 있다. PR 게시 요청 이후에는 Git commit/push와 설명 갱신만 수행하며 제품 빌드나 실행을 다시 하지 않는다. 로컬 검증용 소스와 팩을 유지하고 PR merge·main 동기화·리소스 외부 공유는 사용자의 검증 이후로 남긴다.

Client/UI를 자율 실행·조작하거나 캡처하지 않았다. 사용자는 최종 EXE와 protocol80에 맞춘 Server/Client로 KoukuSaydon 진입, F1 Action Workbench 첫 열기, Artist D 반복 이동을 직접 확인해야 한다. publisher·수치 probe·focused compile PASS는 해당 화면의 visual PASS가 아니다.
