# 아레나·베른·Workbench 공통 성능 구현 계획

## G00. 목표와 확인한 입력

발탄과 쿠크의 편집 프레임 저하를 현재 profiler와 실제 코드로 분리한다. 베른에도 같은 맵·그림자 경로의 개선을 적용하고 저장 캡처를 대조한다. 목표는 16.7ms/frame이지만 사용자 화면의 동일 조건 재측정 전에는 60fps 달성으로 기록하지 않는다. 사용자의 최신 요청에 따라 최종 정상 Debug Product EXE 빌드까지 수행한다. Client/UI 실행은 사용자가 한다.

9월 27일 캡처의 유효 아레나 평균 frame은 발탄 baseline 36.24ms, 발탄 상세 47.60ms(28frame), 쿠크 상세 36.14ms다. 발탄 23:10:19 파일은 LOADING, map/animation 0이므로 아레나 비교에서 제외한다. 모두 Debug/iterator debug 2/D3D debug layer 사용이며 baseline만 상세 CPU scope가 꺼져 있다. CPU/GPU elapsed와 부모/자식 scope를 더하지 않는다.

## G01. ImGui 플랫폼 창 제출

`Engine/External/imgui/backends/imgui_impl_dx11.cpp`의 secondary swapchain Present는 프레임당 합계 6.25~10.95ms이며 특정 호출 하나에 집중된다. 메인 Present는 0.035~0.058ms다. busy일 때 UI 제출을 무기한 기다리지 않는 DXGI_PRESENT_DO_NOT_WAIT 경로를 사용하고 다음 프레임의 최신 UI를 제출한다. `Engine/Private/ImGuiLayer.cpp`에서 secondary 제출 순서를 순환하여 같은 후순위 창만 계속 밀리지 않게 한다. 메인 swapchain, DPI, 줌, dock/undock 동작, 렌더링 옵션은 보존한다.

`Engine/Public/Profiler.h`, `Engine/Private/Profiler.cpp`는 main-thread의 bounded viewport 제출 표본(ID, 위치/크기, duration, flags, HRESULT)과 성공/busy/실패 counter를 소유한다. `Client/Private/ProfilerCaptureIO.cpp`, `ProfilerTool.cpp`가 저장/표시한다. capture off는 표본을 수집하지 않는다. 기존 schema v3의 additive 필드이며 overflow는 별도로 기록한다. DXGI 공식 문서의 nonblocking Present 계약을 따르고 retry spin/강제 동기 대기/매 프레임 GPU flush를 넣지 않는다.

## G02. 공통 맵과 그림자

`MapAssetObject.h/.cpp`, `MapStaticBatchObject.h/.cpp`에서 최종 camera 이후의 기존 가시성 판정과 shadow admission을 재사용한다. batch의 camera cache는 이미 있으므로 같은 역할을 중복 구현하지 않는다. fallback의 camera revision/world bounds/hysteresis가 안정된 경우만 가시성 결과를 재사용한다. shadow의 불변 source material 조건을 준비 단계에서 캐시하고 변하는 override/morph는 다시 검사한다. camera 이동, bounds 변경, bypass, 숨김, material override, morph가 기존 동작과 일치해야 한다. 베른/발탄/쿠크에 같은 조건을 적용한다.

메시 밀도·LOD·재질·제출 수를 검토하되 사용자가 조율한 화질을 낮춰 성능 개선으로 대체하지 않는다. Deploy 실제 draw와 fallback 방문 비용을 구분한다.

## G03. 이펙트와 Workbench

`Effect_Playback.cpp`의 발탄 FrameRebuild 3.35ms를 실제 설치 이펙트의 기존 비UI 재생 검사에서 세분한다. 원인 확인 후 동일 입자/mesh/trail 출력과 외부 timeline 시계·pause/seek/history를 보존하는 중복 계산만 제거한다. spawn/preparation 병목 증거 없이 pool/preload를 새로 만들지 않는다.

`KoukuSaydonActionWorkbench.h/.cpp` Patterns/Timeline 약 5.2ms와 `KoukuSaydonPresentationPlayer` Prepare 경로는 실제 반복 계산·문서 변경 세대를 확인한다. UI 이벤트/문서 변경 때 캐시를 무효화하고 동일 문서를 매 frame 다시 해석하는 비용을 제거한다. source pin/freshness·저장 실패 보존을 건너뛰지 않는다.

## G04. 검증과 완료

각 변경은 기존 실제 함수/데이터를 사용하는 비UI 동등성·최소 컴파일 검사로 확인한다. 관련 JSON/XML 구조와 `git diff --check`, 정상 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -MaxCompilerProcesses 4`를 수행한다. 새 Shared 헤더는 프로젝트에 등록하고 신규 Effect JSON은 Client의 96.DataFiles None 항목과 filters에 등록한다. 현재 다른 작업의 미커밋 변경을 보존한다. RESULT는 원본 캡처, 자동 검사/국소 측정, 빌드, 미실행인 사용자 화면/FPS 확인을 분리한다.

## G05. 사용자 추가 요청: 휠윈드 잔상과 피자 전체 Preview

기본 `VALTAN_WHIRLWIND/SPIN`에는 직접 저작한 `effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01`(visible mesh particle 2, hidden trail 1)와 `effect.valtan.pattern.420633.active`(visible baked trail 3, particle 5, light 1)가 함께 연결돼 있다. 사용자는 Effect Tool 첫 Product만 자신의 깔끔한 휠윈드라고 확인했다. `Remove-ValtanPatternEffectLink.ps1`의 stable pattern/effect/cue ID, writer lock, raw-byte CAS, backup/rollback 및 PublishV2를 사용해 `cue.valtan.whirlwind.active` 연결만 제거한다. shared Effect asset/다른 패턴의 동일 asset 연결/사용자의 저작 mesh particle은 보존한다. 실제 셰이더는 mesh particle 경로이며 decal로 바꾸지 않는다.

피자는 full restore 점프와 착지 사이의 sector 표시·대상 선택 회전·착지 sector의 소스와 Preview 소비자를 확인한다. 기존 Server 선택/방향 권위를 보존하며 local Preview는 해당 표현을 timeline으로 재현해야 한다. 없는 단계는 실제 원본·기존 Effect carrier로 연결하고, 후보 검증과 stable-ID 병합 후 공식 projector/publisher로 제품 문서를 갱신한다. 에셋을 임의로 만든 것이나 이름만 연결한 상태를 실제 Preview 완료로 기록하지 않는다.

## G06. 확정된 피자와 외곽 돌 범위

대상은 `VALTAN_SIX_PIZZA_106`이다. `Valtan.cpp/.h`의 local Preview target-follow는 기존 typed SceneCharacter와 월드 root 경로를 사용하며 대상의 수명·교체를 검사한다. 동일 timeline cursor에서는 회전을 유지하고 진행·seek 표본에서만 갱신한다. 기존 sector composite를 유지하며 STEP_01/03/04/10의 원본 Full Restore를 연결한다. STEP_03의 V2 landing과 STEP_04의 V2 stomp 연결을 제거해 중복을 피한다.

피자 전용 rock active 문서의 stable mesh element 크기를 2배로 하고 독립 항목 이름을 `피자 패턴 | 외곽 4개 돌`로 맞춘다. 사자후·발악의 별도 돌 문서와 Server의 위치·개수·충돌 크기는 이 외형 조정에 포함하지 않는다. 최신 원본을 stable ID로 병합하고 writer admission, CAS, 백업, 원자 교체를 지킨다.

추가 요청인 돌 전조·폭발은 원본 후보를 조사하되 네 돌의 실제 호출 근거가 미확정이므로 Full Restore로 분류하지 않는다. 사용자의 마무리 요청에 따라 기존 손저작의 전조·폭발을 보존하고 쿠크와 같은 카테고리 메타데이터 방식을 재사용한다. 피자 카테고리 안에서 `돌 폭발 전조`, `돌 폭발`, `돌 폭발 전체`를 각각 저작·재생할 수 있게 구성한다. 원본 emitter·재질·단위와 사용자 요청 배율을 따로 기록하며 사진만 보고 임의 색상이나 형태를 복원값으로 만들지 않는다. 현재 고정 시각에 네 돌을 함께 폭발시키는 경로와 사용자 요청의 피격 돌 선폭발·잔여 돌 약 1.5초 후 폭발을 대조하고, 필요한 판정 변경은 Server가 소유하며 Client Preview도 동일 데이터 계약을 소비한다.

## G07. 연쇄 폭발의 Client 표현 계약

`ActorCatalog.h/.cpp`는 optional `armedPresentationEventId`와 `armedEffectAssetId`를 쌍으로 읽고 `stopActiveOnHit`의 기본값을 false로 둔다. Server의 owner-hit policy와 stable armed event ID를 publisher에서 대조하며 asset ID를 Server 판정에 전달하지 않는다. `Valtan.cpp`의 기존 HIT_PULSE 소비자는 정확한 armed ID에 전조 asset을, 나머지 hit에 terminal asset을 선택한다. Sound 바인딩과 V2 단일 경로 계약은 보존한다.

`ClientReplication.cpp`는 source/archetype/revision을 확인한 실제 hit에서만 active visual을 종료한다. `CombatObjectProjectionRuntime.h`의 완료 상태는 authoritative object를 남겨 두면서 다음 snapshot의 재시도·Update가 종료된 돌을 되살리지 않게 한다. despawn/reset은 기존 책임을 유지한다. active/armed/terminal Effect를 기존 Level 및 Effect Tool 준비 경로에서 함께 수집한다. optional 필드 부재는 기존 오브젝트의 동작을 바꾸지 않는다. codec, 완료 후 snapshot·중복 event·despawn, 최소 컴파일을 검증한다.


## G08. SIX_PIZZA 네 돌의 실제 모아치기 반응과 지연 폭발

현재 STEP01에서 네 돌을 패턴 1000 ms에 생성하지만 고정 TIMED hit가 19500 ms 뒤 네 돌을 동시에 폭발시키고, 20700 ms 수명으로 제거한다. 마지막 모아치기 STEP11의 cone 첫 타격은 패턴 28750 ms다. 이 서로 다른 시계 때문에 마지막 모아치기까지 돌이 남지 않는다. 원본 client clip12_11 notify Atk_08_04 약 0.163 s와 실제 원작 공략의 타겟 방향부터 연쇄 폭발 설명은 방향성 근거이며, 원작 Server 코드나 정확한 서버 지연값을 확보한 것은 아니다. 1500 ms는 사용자 요청값이다.

combat object의 optional ownerHitChain은 triggerActionId, delayMs, armedPresentationEventId를 가진다. 지정 owner action의 실제 cone hit가 하나 이상의 돌 cover circle과 겹치면 같은 source entity / pattern sequence / archetype / spawn tick 그룹에서 맞은 돌은 즉시, 나머지는 delayMs 뒤 기존 HIT_PULSE와 피해 경로를 사용한다. 한 번 armed된 그룹은 이후 반복 타격으로 시간을 덮어쓰지 않는다. arm 전에는 기존 TIMED clock을 실행하지 않으며, 해당 객체의 명시 수명은 마지막 타격과 지연을 수용한다. 다른 사자후·발악 돌에는 이 정책을 추가하지 않는다.

Shared/Public/Gameplay/CombatObjectHitChain.h의 순수 cone-circle 분류를 Server와 local Preview가 공유한다. 새 헤더는 Shared.vcxproj와 filters에 등록한다. CValtanBrain은 실제 피해 호출과 같은 pose 및 action의 typed hit 기록을 내보낸다. GameRoom은 이를 기존 CombatObjectRuntime에 전달한다. Runtime의 일반 Update가 boss brain보다 먼저 실행되는 순서는 유지하고, arm 직후 해당 tick의 새로 armed된 객체만 delta 0으로 기존 pulse/damage 처리부를 소비하여 첫 폭발을 한 tick 늦추지 않는다. 지연은 Server 정수 tick으로 계산한다. cover는 첫 타격 판정까지 유효하고 실제 폭발 뒤 해제한다.

Canonical combat object → product projection → Gameplay bootstrap의 optional 정책을 검증하고 Client PatternTree의 제품 로드와 Workbench draft overlay에도 동일하게 전달한다. Preview는 같은 stage/global clock과 cone 방향을 사용하여 active / armed / hit 효과를 샘플링하며, pause 및 역방향 seek에서도 같은 시점 계약을 유지한다. ActorCatalog의 optional armedPresentationEventId / armedEffectAssetId pair와 stopActiveOnHit는 해당 정책과 exact ID join으로 검증한다. 남은 돌의 전조는 arm 시점, 실제 폭발 효과와 피해는 1500 ms 뒤다. Client projection은 폭발한 active 외형을 완료 상태로 전환해 후속 snapshot/retry에서 부활시키지 않고 terminal tail은 끝까지 허용한다.

소스 파일은 기존 GameRoom_BossSimulation, ValtanBrain, CombatObjectRuntime, GameplayCatalog, ValtanPatternTree, Valtan, ActorCatalog 및 기존 publisher/validator를 확장한다. Data 변경은 out 후보에만 준비하고 root가 최신 저장본 stable ID 병합과 명시 publish를 직렬 수행한다. 기존 미커밋, 저장 x/y, rendering option, 다른 패턴을 보존한다. 검증은 Shared geometry와 Server runtime의 hit/45 tick delay/no duplicate/source isolation/cover 보존, 실제 JSON roundtrip과 malformed policy rejection, local Preview의 pause/seek 시간 계약, 변경 CPP 최소 컴파일, XML/JSON parse 및 diff check다. Client/UI 실행과 최종 화면 판정은 사용자가 수행한다.


## G09. 09-28 피자 외곽 돌의 외부 시계 재적분 제거

새 `컷씬_프레임드랍_20260928_071303_453_frame592_11844_1.json`의120frames는 평균88.185ms다.41개의 CPU scope overflow frame에서 FixedStep이 평균2,125회 이상 기록됐다. `CValtan::Sync_LocalPatternCombatObjectPreview`는 외곽 돌과 전조/폭발의 각 handle에 매 frame provider 없는 Seek_WorldRoot를 호출한다. 서비스는 이 경우 역사 표본을 무효화하고 Set_SampleTime→Reset→0부터현재시간까지 다시 적분한다.29초의 네 돌만으로도 매 frame 약6,960 step까지 반복할 수 있다.

기존 `Client/Private/Valtan.cpp`의 해당 SampleRoot caller에 고정 Root를 값으로 소유하는 EFFECT_FIXED_STEP_TRANSFORM_PROVIDER를 전달한다. 기존 Commit_ExternalTransformHistorySample은 첫 표본·역방향·0.5초 초과 jump에서만 Seek하고 전진/동일 cursor는 증분/hold 처리한다. 새로운 manager/API나 별도 시계를 만들지 않는다. Root, spawn ID/generation, owner-hit-chain과 finished handle의 reverse-seek 수명은 유지한다. fixed sample의 SourceAnchorWorlds/ParticleParameters는 기존 fixed world-root 효과처럼 비어 있다. 일반 캐릭터/다른 caller의 raw Seek 계약은 변경하지 않는다.

수정 전 dirty bytes를 백업하고 해당 ASCII 블록만 교체한다. 실제 ValTan CPP 최소 컴파일, 현재 실제 효과 문서와 CEffectPlayback을 사용하는 fixed-root seek/advance/hold/rewind/largejump 출력 비교를 수행한다. 반복 current-time Seek 대비 수행 step 또는 경과시간을 구분해 기록한다. 자동 검증은 실제 Client FPS를 대신하지 않는다. 사용자는 편집 중이므로 데이터·Resources 교체 및 Client/UI 재시작은 하지 않는다.

## G10. V1 Effect Tool의 동일 attachment 그룹 재구축 비용

같은 캡처의 CPU scope가 완전한 79 frame에서 `EffectTool.Render`는 평균 38.392 ms이며 AuthoringWindow 19.887 ms, DetailWindow 13.972 ms다. 현재 두 창은 각각 `Build_AttachmentElementGroups`를 호출한다. helper는 동일 socket의 16개 float를 포함한 문자열 key를 모든 Element에서 반복 포맷하고, 각 그룹 member를 전체 문서에서 두 번 다시 찾는다. Authoring 행도 member마다 같은 선형 검색을 반복한다. 실제 248 Element 발탄 stage007 문서는 한 그룹이지만 같은 attachment key를 248번 만든다. member 검색만 제거한 격리 후보는 이 문서에서 개선되지 않아 key 중복 생성도 함께 제거한다.

`Client/Private/Effect_Tool_Helpers.cpp`의 함수 안에서 attachment/group/inheritance의 typed identity를 모아 동일 그룹의 문자열 key와 표시 이름을 한 번만 만든다. float의 정확한 bit를 사용해 양수·음수 0 구분을 보존하며 기존 key 문자열, 첫 등장 그룹 순서, member stable ID 순서, center, 회전 가능 여부와 이유는 유지한다. member pointer view는 이 함수 호출 안에만 존재하며 Document를 저장하거나 다음 frame까지 보관하지 않는다. 각 member의 중복 전체 검색을 이 view로 대체한다.

`Client/Private/Effect_Tool_Detail.cpp`의 그룹 행은 해당 render 호출에서 만든 stable ID 인덱스를 재사용한다. Active Document 변경은 기존처럼 모든 그룹 행 제출 뒤 commit하며 미저장 Detail draft, 선택과 접힘 상태, 그룹/solo 명령을 보존한다. 행 높이·popup·선택에 대한 동등성 증거가 없는 clipper 변경과 cross-frame cache는 이번 범위에서 제외한다. 새 H/CPP나 public 계약은 없으므로 프로젝트 등록은 추가하지 않는다.

수정 직전 dirty bytes와 hash를 보존하고 기존 인코딩과 줄바꿈을 유지한다. 원래 helper와 수정 helper를 현재 대형 문서의 실제 소비 필드로 비교하여 모든 그룹 출력과 selected-only 결과를 확인한다. attachment 변화, manual 그룹, inheritance, source 계약, 양수·음수 0, 문서 교체에 대한 비교와 Debug 국소 측정, 두 CPP 최소 컴파일, diff check를 수행한다. capture에는 선택 asset ID가 없으므로 국소 감소값을 실제 전체 38 ms 회복으로 바꾸어 기록하지 않는다. Data·Resources 교체, Client/UI 실행과 reload는 하지 않는다.

## G11. 발자국 다섯 회와 같은 시계의 Source Model Preview

사용자는 전체 저장과 Client 종료 후 반영·빌드 검증을 승인했다. 대상은 `effect.valtan.action.420624.stage001.full.restore`의 b_effectroot 발자국 그룹이다. 최신 저장본의 dash 5개와 기존 발자국 14개 및 사용자가 편집한 좌우 위치·크기·재질을 보존한다. 원본 notify의 0/150 ms와 500 ms animation cycle을 따라 500/650/1000 ms에 좌/우/좌 7개씩 추가하여 발자국은 35개, 문서는 40개 Element가 된다. 복제는 기존 portable `authored-copy:` origin 계약과 새 stable ID를 사용하며 source emitter recipe의 내부 시각을 덧셈하지 않는다.

`Data/Effects/ValtanFullRestoreAnimations.json`의 원본 animationClips는 mesh_att_battle_18_02, 500 ms, loop false, 원본 stage 400 ms를 그대로 둔다. 같은 행의 optional `authoredPreview`는 `mappingBasis=PROJECT_AUTHORED`, `loop=true`, `previewWallMs=1140`을 가진다. 마지막 1000 ms 접지는 재생하고 다음 1150 ms 접지 직전에 모델 시계를 hold하며 남은 Effect tail은 기존 Effect 시계가 소비한다. 이 값은 사용자 저작 override이며 원본 provenance로 기록하지 않는다.

`ValtanPatternTree.h/.cpp`의 Load_FullRestoreSourceClips에 기본 false인 editor override 선택 인자를 추가한다. 모든 원본 항목과 override를 검증한 뒤 index를 commit하고 실패 시 이전 출력은 유지한다. `Effect_Tool_Valtan.cpp`의 standalone FullRestore Open만 이를 true로 호출하며 override index를 원본 pattern matching cache에 저장하지 않는다. Product의 기존 animation 반복과 stage, Server 권위는 보존하며 cue 위상은 아래 계약으로 맞춘다. 새 H/CPP가 없어 프로젝트 등록은 없다.

`build_valtan_full_restore.py`는 기존 `authoredPreview`의 PROJECT_AUTHORED 표식과 loop bool, 1~600000 ms wall budget을 검증한 뒤 재생성 결과에 그대로 보존한다. 원본 animationClips의 변경은 기존 검증처럼 거절하며 잘못된 override도 파일 교체 전에 거절한다. metadata 회귀는 원본 receipt와 override의 동시 보존 및 잘못된 값의 무변경 실패를 확인한다.

검증은 최신 dirty baseline 백업·hash/CAS·원자 교체·자기 변경 rollback, 기존 19 Element 보존과 새 21개 origin/좌우 TRS/재질·timing 비교, 실제 JSON loader의 원본/override/잘못된 입력 보존, 실제 animation timeline의 다섯 접지·1140 ms hold, Effect codec/native consumer 검사와 변경 CPP 최소 컴파일로 수행한다. 최종 Product 빌드와 데이터 게시의 owner는 root로 한정하고, Client/UI 실행과 최종 화면 판정은 사용자가 한다.

Product에서도 다섯 접지와 Effect 시계를 맞춘다. 기존 VALTAN_WARP/STEP_02의 1600 ms loop 및 처음 300 ms body hidden은 유지하고 exact cue `cue.valtan.composition.valtan_warp.step_02.01`만 307~1831 ms에서 500~2024 ms로 평행 이동하여 사용자 box 길이 1524 ms를 보존한다. 500 ms는 실제 clip 첫 loop 경계다. runtime Sync가 이미 지원하는 composition+loop+ONCE의 unwrapped source clock을 Product reload 검증에도 동일하게 적용하며 일반 native/each_loop의 source window 제한은 보존한다. 최신 Valtan.cpp의 G09/G13 변경은 보존하고 이 admission 블록만 수정한다. 실제 Product reload와 500 ms 경계 발화, 다섯 발자국을 native consumer로 확인한 뒤 root가 다시 게시·빌드한다.


## G12. 최종 저장본과 게시 소비자 정합성

2026-09-28 사용자가 모두 저장 후 종료했고 전체 반영을 승인했다. `out/FinalApply20260928/saved-before`에 Data와 Client/Server 게시 파일 2,629개를 교체 전 보존한다. 사용자 저장 후 발견한 실패를 저장 내용을 되돌리지 않고 소비 계약에서 고친다.

- Composition publisher의 Pattern Sound validator에 실제 C++ 소비자와 동일한 optional `playbackOffsetMs`(0~600000), `playbackDurationMs`(1~600000)를 허용하고 타입/범위를 검증한다. 기존 clip/window/identity 검증은 유지한다. 저장된 cue를 payload에 그대로 보존한다.
- `VALTAN_BIND_SLOT/STEP_01`에서 사용자가 삭제한 `composition.clip.05`의 V2 binding `binding.valtan.migrated.016.37d21bb6553ce6e9`만 제거한다. 최신 4개 클립과 RECOVERY의 별도 shout.burst binding은 그대로 보존한다. 삭제 전 백업과 hash 재확인, 원자적 교체를 수행한다.
- 최신 canonical source에서 공식 PublishV2 → Composition Publish → Gameplay Publish를 수행하고 source/게시 payload 일치를 검증한다. 모든 변경은 saved-before와 필드 단위로 다시 비교한다. 렌더링 option 값은 변경하지 않는다.
- 검증: 저장된 sound optional field 수용/범위 거절 회귀, V2 identity closure, split projection/clip/hit validator, 게시 JSON parse 및 Product Debug 빌드.

최종 Gameplay Publish에서 발견한 stale root-motion projection도 공식 `Tools/ValtanActionExtractor/build_valtan_rootmotion.py`로 현재 저장된 Encounter/PatternBindings와 설치 AnimSet의 b_root를 다시 적분한다. 사용자 source clip 삭제·4107 ms stage는 보존하고 `Data/Animation/RootMotion/Valtan.rootmotion.json` 파생 파일만 교체한다. 모든 120개 stage를 fresh bake와 대조하고 실제 Publisher의 root-motion 검증 블록을 독립 실행하여 전체 길이·stable stage·sample packing을 확인한 뒤 hash/CAS·백업·원자 교체한다. 원본 gameplay/presentation의 사용자 timing을 되돌리지 않는다.


G12 추가 구현 범위: native에서 유효한 Effect V1 `ONCE/CUE_END` 잔여 재생을 정규화할 때 `payload.sourceClock.endMs`에 저작 source 종료 시각을 보존한다. stage 종료 시각으로 자르지 않고 해당 animation occurrence의 누적 시작, sourceStartMs, playRate와 기존 half-up 반올림을 적용해 정규화 `endMs`를 계산한다. detached cue도 sourceClock의 명시 종료값을 보존한다. projected invariant의 stage 끝 초과 예외는 `EFFECT_V1 + ONCE + CUE_END`에만 적용하며 시작 시각과 다른 kind/repeat/stop 검사는 그대로 유지한다. 독립 fixture로 occurrence 앞 구간과 2배속, source 시작 오프셋, 소수 배율 반올림, detached 보존, stage 시작 범위 및 다른 정책의 초과 거절을 검증한다. 현재 dirty publisher의 앞선 G12 수정을 byte 백업하고 두 Python 파일의 이 변경만 추가한다.


## G13. 명시적인 Composition seek의 Effect 역사 재구성

현재 실제 AnimationTool → local Valtan → EffectPresentationService → CEffectPlayback 비UI probe에서 29.346초 전진은 후반 red sector 1개(alpha 0.254336)를 만들지만 같은 시점 명시 seek는 11초의 yellow/red 2개(alpha 0.908842)를 만든다. seek는 이전 stage 끝들을 차례로 재구성하는데 서비스는 큰 시간 이동을 일반 Advance_Preview로 전달한다. Playback의 실시간 한 frame 최대 60 fixed step 제한 때문에 요청 시계만 29초로 가고 실제 입자 적분은 뒤에 남는다. 컷신 suppression은 0이며 단독 Playback.Seek에는 이 차이가 없다.

`Animation_Tool.h`와 `Animation_Tool_ValtanPlayback.cpp`의 Apply/Activate 함수에 기본 false인 `bRebuildEffectHistory`를 추가한다. Seek_ValtanPatternMasterPreview의 bResetPresentationTransport가 true인 재구성에서만 이전 stage와 목적 stage에 true를 전달한다. 일반 Update와 자연스러운 stage 경계는 false여서 기존 증분 재생을 유지한다.

`Valtan.h/.cpp`의 Apply_LocalPatternPresentationSample은 이 뜻을 기존 Sample_LocalBossPreview에 전달한다. `Effect_PresentationService.h/.cpp`의 같은 함수는 명시 재구성인 active local boss cue에 기존 bPendingInitialSeek를 설정한다. 이미 같은 시점이어도 force 표본은 생략하지 않는다. 기존 commit/update 소비자가 Set_SampleTime → Playback.Seek로 정확한 요청 시각까지 재구성하며 pending spawn은 기존 initial seek를 사용한다. 일반 큰 frame delta를 자동 seek로 바꾸거나 실시간 catch-up 제한을 제거하지 않는다. 새 상태 owner·JSON·H/CPP 파일은 추가하지 않으므로 프로젝트 등록 변경은 없다.

현재 dirty bytes를 백업하고 인코딩·줄바꿈을 보존한다. 실제 resource와 local boss 소비자의 12/18/24/29.346초 전진·명시 seek carrier/alpha/world를 비교하고 STEP_06 target 회전, pause hold, resume, 역방향 seek, 컷신 suppression과 root handle 보존을 확인한다. 일반 forward와 RootHistory 돌 최적화를 함께 검증하고 변경 TU 최소 컴파일과 diff check를 수행한다. 수치 검증은 GPU 화면 가시성 확인을 대신하지 않으며 최종 build/publish는 root가 직렬 수행한다.


## G14. 저장된 후속 V2 효과의 역할 정합성

최종 저장본의 새 `binding.valtan.authored.ecd61b19f927cb9f`는 `VALTAN_FLOOR_WIPE_130/FIRST_SMASH`의 224 ms에 `boss.valtan.six.sonic.after`를 재생한다. 게시 gate의 missing role은 삭제한 그룹의 stale 행이나 피자 복원 누락이 아니라, 기존 Library 그룹을 처음 패턴에 연결한 뒤 역할 ledger가 빠진 경우다. 원본 그룹과 binding은 저장 전 백업에도 동일하다.

`Data/Effects/V2/EffectRoles.json`에 이 exact GROUP 한 행을 `STATE/NONE`으로 추가한다. FIRST_SMASH의 실제 hit는 0 ms이며 후속 그룹은 0.3초 확장 ripple, 1.2초 smoke, 1초 파편, 3초 fade ground decal, 0.5초 screen blur로 구성된다. 독립적인 추가 공격 판정을 만들거나 이미 저장한 224 ms를 0 ms로 옮기지 않는다. 기존 주 충격 `boss.valtan.six.sonic`의 `ATTACK/BINDING_START`와 모든 다른 역할·판정·binding·TRS는 보존한다. 이는 현재 후속 연출의 프로젝트 분류이며 원본 Server 판정의 증거로 삼지 않는다.

변경 전 raw bytes와 hash를 out에 보존하고 최신 hash 재확인 후 한 행만 원자적으로 교체하며 실패 시 자기 변경만 rollback한다. JSON parse, exact role coverage, 기존 18행 불변, 해당 새 binding 불변과 missing/stale 역할·잘못된 ATTACK 시각 거절을 확인한다. 전체 validator가 다른 저장본 정합성 문제로 진행하면 해당 오류를 별도 owner에게 전달하며 validator 우회나 무관한 사용자 timing 변경으로 닫지 않는다. 신규 코드/파일/public API가 없어 프로젝트 등록과 C++ 컴파일은 불필요하며 최종 domain publish는 root가 수행한다.


G14 추가 실측과 승인 범위: 사용자 저장본의 `binding.valtan.authored.ff796eec4443072d`는 `VALTAN_SILENCE_SLOT/STEP_01`의 786 ms에 기존 `boss.valtan.shout.burst`를 별도 연출로 사용한다. stage에는 hit가 없고 유일한 clip occurrence는 PROJECT_AUTHORED다. shared shout.burst의 ATTACK 역할이나 기존 CLIP_TEMPLATE 정책을 낮추지 않는다.

`validate_valtan_hit_presentation_alignment.py`의 기존 allowlist에 `PROJECT_AUTHORED_PRESENTATION_ONLY`를 추가한다. 이 rule만 기존 exact scope/binding ID/빈 expectedHitOffsetsMs/reason에 `resource`, `clock`, `mappingBasis`를 필수로 가진다. resource와 STAGE/ONCE/null occurrence/startMs clock 전체가 해당 binding과 정확히 같고 mappingBasis는 PROJECT_AUTHORED이며, 실제 stage에 hit와 잡기 피해 event가 없고 그 stage의 실제 clip occurrence가 모두 PROJECT_AUTHORED일 때만 해당 한 binding을 presentation-only로 센다. 사용되지 않은 receipt와 모든 drift는 실패한다. 이름이나 effect ID 접두사로 자동 예외를 만들지 않는다.

`Data/Valtan/Valtan.hitalignment-allowlist.json`에는 위 SILENCE stable binding 한 행만 기록한다. 기존 source V2 binding, sound cue, gameplay와 모든 다른 예외는 그대로 둔다. 실제 data를 읽는 focused test에서 현재 receipt, 시간/resource/scope/identity 변화, 새 stage hit, mappingBasis 변화, stale receipt를 확인한다. 나머지 sound 정합성 5개 scope는 root 소유로 별도 처리하며 이 rule로 우회하지 않는다. validator의 root 소유 optional sound window field 변경을 보존한다.


### G12 소비자 대조로 확인한 추가 범위

게시 코드와 실제 native parser를 대조해 Effect V1의 optional playbackOffsetMs, once cue_end의 clip 경계를 넘는 잔여 수명, World Sequence v3 soundTracks도 같은 계약으로 검증한다. once end의 normalized Stage clock 변환에서는 원본 종료 시각을 잃지 않도록 sourceClock.endMs를 보존한다. looping each_loop의 clip 경계 제한은 유지한다. World sound의 path/identity/time/volume/64 track 상한도 native와 동일하게 검증한다.

삭제된 clip05의 template waiver는 제거하고, 사용자가 RECOVERY Shot7을 900ms에서 100ms로 저장한 사실은 해당 occurrence의 SOUND waiver에 정확히 기록한다. 효과·소리·hit의 사용자 저장값을 이전 template로 덮어쓰지 않는다. 새 V2의 role 및 alignment 검토는 G14에 기록한다.


G14 sound receipt: 기존 `STAGE_HIT_SOUND_TRACK`는 5회 이상·100 ms 이하 고빈도 hit 전용이다. 이번 WHIRLWIND/SPIN, FOUR_SLASH/SLASHES, FOUR_SLASH/SPIN, HIGH_JUMP/LAND, BIND_SLOT/RECOVERY에는 이 조건을 완화하지 않고 별도 `PROJECT_AUTHORED_SOUND_TIMING` rule을 사용한다. 기존 exact scope/전체 expectedHitOffsetsMs/reason에 `expectedSoundCues`를 필수로 두고 해당 scope의 현재 저장된 sound cue payload 전체를 고정한다. identity, event, source clock, clip occurrence와 optional 재생 offset/duration을 포함한 행 추가·삭제·값 변경 또는 hit offset 변경은 모두 재검토를 요구한다. 현재 sound row를 삭제하거나 과거 시각으로 돌리지 않는다. 사용되지 않는 receipt는 기존 stale 검사로 실패한다. focused regression은 두 새 rule과 기존 고빈도 rule 경계, unknown/missing field, 모든 exact payload drift를 검증한다.


G14 독립 검토 보완: sound source payload만 고정하면 animation rate 또는 선행 clip 길이 변경이 stage-wall 재생 시각을 바꾸어도 예외가 남는다. 각 sound receipt의 expectedAnimation에 현재 stage animation 전체를 추가하고 sourceStartMs/playMs/playRate, occurrence 순서 및 repeat/endPolicy를 포함해 exact 비교한다. rate·source trim·선행 clip 길이·순서 변경 회귀를 추가한다. 사용자 animation 원본은 바꾸지 않는다.
