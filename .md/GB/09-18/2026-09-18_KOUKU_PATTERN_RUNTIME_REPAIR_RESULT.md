# 쿠크 패턴 실제 재생 연결·복구 결과

최신 이펙트 선준비 후속 상태는 아래 **G10. 레이드 진입 전 Effect dependency 준비** 절에 기록한다. 이전 절의 빌드·게시 시점과 구분한다.

## 완료 범위

코드 수정과 최신 저장본 필드 병합을 완료했다. 수정 최초 설치는 Composition revision1660, World revision2094다. 이후 사용자 저장본1664에서도 모든 수정 필드를 재확인했다. source 교체는 최신 디스크 읽기 → stable ID별 필드 병합 → 백업 → hash 재확인 → 원자 교체로 진행했다. 첫 교체 시 사용자 동시 저장을 감지해 후보를 폐기했고, 최신1659 기준으로 재병합했다. 사용자가 작업 중 추가한 P33 World 박스 등 무관한 항목을 보존했다.

| 요청 | 원인 및 실제 변경 |
|---|---|
| 공굴리기 몸 회전 | 설치 MN_RPCT_05의 실제 +X 전방을 +Z로 취급했다. Preview/Server 모두 body yaw=목표 방향-90도, 전진=body yaw+90도로 맞췄다. 이동 추적 회전은180도/초로 전체 수명과 분리했다. |
| 공굴리기 Server 재생 | 공유 카운터82와 결과51이 이름만 있고 typed 실행 kind가 없어 P81이 게시 대상에서 제외됐다. P81 전용 COUNTER_WINDOW를 추가해 성공 시 패턴 종료하도록 연결했다. P80이 사용하는 기존82/51은 보존했다. |
| 불 앵커·수명 | P81과 P27의 shared fire occurrence를 bip001-mouth/BODY/BONE/follow로 연결했다. 실제 원본 breath socket과 설치 WModel basis를 입 본 frame으로 변환했으며 shared Effect/사용자 scale은 보존했다. shared fire의25개 emitter가 모두 finite이므로 Composition V1도 원래 속도 반복/전체 앵커 시계를 지원하도록 수정했다. loopEffectToDuration으로 저장된 재생창을 유지한다. |
| 본 선택 비활성 | Composition Preview CNpc는 전역 Animation Tool target에 바인딩되지 않는다. MainApp typed resolver가 해당 패턴의 실제 preview actor 또는 replicated actor BODY/WEAPON 모델을 제공하도록 고쳤다. |
| 피자10명 소환 | summon8 occurrence의 patternSpawns가 비어 있었다. 실제 2관문 table/navgrid로 검증한 MAP 좌표에 좌우5명씩 child84를 연결했다. child17333ms에 맞춰 summon17333ms/부모18794ms로 연장했다. Preview 배우 수명도 Server처럼 summon box 수명을 사용한다. |
| 왼손 흰 트레일 | white element에만 +100cm StartLocation과 segment clipping이 남았다. 정상 오른손 방식으로 해당4필드만 고쳤다. 실제 모델기준1.7m 어긋남을 제거하고 저장된24891ms 창을 source loop0 방출에 연결했다. |
| 칼날 복구·앵커 | Clear Admission 전 revision2037 보존본에서 즉사 칼날8개 emission을 복구했다. 최신 scale/speed/Collider를 유지하고 사용자 수정 normal/red source Effect를 동일 Object anchor에 붙였다. |
| Collider 게시 경계 | Effect가 있는 World의 생존 tail을 굽는 동안 부모 Pattern 종료를 넘긴 판정이 생성됐다. P31의40개 window가11000ms까지 나와6000ms 부모 범위를 위반했다. 부모 종료를 bake에 전달해 판정/track만 그 끝에서 닫고, 부모 생존 중 World birth 이후 tail은 보존했다. |
| 칼날 지속 재생 | World effectTracks optional loopEffectToDuration으로 원본 속도 반복을 지원한다. finite source는 전체 prepared 수명마다 반복하며 cycle 시작시각을 follow 시계에 포함한다. loop0 source는 기존 bounded emission을 사용한다. |
| 공 수명 | rolling ball template/hidden key7568→11832ms. 현재 WORLD box start740ms와 일치하며 pattern12572ms까지다. |
| 원본 갈고리·즉사칼날 Append | 기존 hook world36은 보존하고 원본 hook와 fatal blade World 정의를 추가했다. Composition World Resources의 Append Selected World가 선택한 정확한 stable World/state를 기존 Append_WorldBox 경로로 추가한다. |

## 저장된 시간과 소환 의미

피자 summon 시작1461ms, 수명17333ms, 부모 끝18794ms다. 10명은 장식 World Object가 아니라 실제 쿠크 clone이며, 각자 child84의 animation/Effect/JUMP/SLAM 시계를 갖는다. 이름만 있는 summon이나 child가 없는 항목은 자동 추정하지 않는다. 기존 기능처럼 박스 시작이 소환 시각이고 박스 길이가 생존창이며, child 길이 변경과 타임라인 자동 축소를 같은 의미로 설명하지 않는다.

공굴리기 track start3000ms/duration8582ms, 불 start3002ms/duration8646ms, 공 start740ms/duration11832ms의 사용자 저장값을 보존했다. P27의 세 불 occurrence 시작2366/2347/2348ms 및 duration5500ms도 보존했다. 본 좌표는 원본 breath notify(P81 FX_Prj_02, P27 FX_Prj_01)로 계산했다. FX_Mouth_01은 RPCT06에 존재하는 별도 cast socket으로 breath 근거와 구분한다.

## 실행한 검증

- Root Client3 TU(Workbench/PresentationPlayer/MainApp) MSVC 컴파일 PASS.
- World3 TU(Document/ObjectTool/Player_Objects) MSVC 컴파일 PASS, 실제 codec load/save/reload18 checks /0 failures.
- 실제 Server/Shared를 링크한 기존 SupportSurface191 PASS /failures0: 첫 tick회전,20초 수명에서도 같은 회전속도,10개 실제 actor commit,실패 전체rollback,child점프/착지,owner종료정리,빈counter결과종료 포함.
- Composition finite/native 반복의 실제 함수8 checks /0, 수정 PresentationPlayer TU 컴파일 PASS. GPU 표시에 대한 검증은 아니다.
- World Collider bake 기존·신규14 tests PASS(STOP/HOLD/LOOP, DAMAGE/HOOK_CAPTURE, 부모 종료 및 기존 tail 보존). 실제 P31 window40개 모두6000ms 이내임을 확인했다.
- Projector targeted5 tests PASS:10/16 summon,잘못된좌표/짧은창/금지child거절,Effectloop/본회전계약.
- 수정5패턴과 dependency42를 포함한 실제 source closure의 validate/project PASS.
- 왼손 actual CModel CPU676samples failed0; whitehead174samples vs installed socket 최대오차9.16e-6m. 긴24891ms 구간은 WModel1495samples 수치검증이며 같은 CModel long-window 실행성공으로 표기하지 않는다.
- JSON parse, PowerShell parser, git diff --check PASS. 기존 다른 파일의 line-ending 안내경고와 검증오류는 구분했다.

## 게시 및 실행 중 프로그램

1660 게시 시도는 사용자1664 동시 저장을 감지해 실패했고, domain owner가 이전 Product/Server data/receipt를 복원했다. 사용자에게 잠시 저장 중단을 요청하여 응답받은 뒤1664 기준으로 재게시했다. 1664 첫 게시도 P31 Collider 경계 오류로 전체 rollback됐고, 원본을 바꾸지 않고 projector의 부모 종료 연결을 고친 뒤 같은1664를 재게시했다.

최종 전체 게시가 exit 0으로 완료됐다. `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1664`의 `koukusaydon.product`, `map.kakulsaydon`, `world.gameplay`, `gameplay.balance` 네 domain 모두 PASS다. Composition revision1664 / World revision2094 기준으로 제품79 patterns,450 stages,8 bundles를 생성했다. 게시 로그는 `out/KoukuRepair20260918/root/publish.log`다.

실제 최종 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`에서 P25의 MAP summon10행과 각 child84 참조, P81의 COUNTER_WINDOW/추적 이동 행을 확인했다. 실제 `CGameplayCatalog::Load()`로 전체 게시본을 읽는 검사도 exit0, `1 Loaded gameplay bootstrap`으로 성공했다. 증거는 `out/KoukuPizzaSummon20260918/server-runtime/published-consumers.json`, `published-full-load.log`다. 최종 게시 및 소비자 검사에서 남은 실패는 없다.

최초 게시 완료 시점에는 Product EXE 링크와 Client 화면 검증을 하지 않았다. 실행 중 Client/Server를 종료하거나 UI를 조작하지 않았다. 파일 교체가 실행 중 draft/Effect cache/Server catalog 자동갱신을 뜻하지 않는다. 저장 중단을 해제하고 사용자에게 Client와 Server 재빌드 → Server 재시작 → Client 재접속 후 화면 확인을 안내했다.

## 사용자 재빌드 후 현재 EXE 검토

사용자의 현재 EXE 반영 검토 요청에 따라 읽기 전용으로 다시 확인했다. Client EXE는 2026-09-18 17:37:38, Server EXE는17:30:26 빌드다. Client PID63832와 Server PID63480은 모두17:37:40에 해당 저장소의 `Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`에서 시작했다. 최종 bootstrap17:31:14/게시 완료17:31:16보다 뒤에 실행됐다.

- Client8개 관련 TU의 source/OBJ/링크 순서 및 실제 링크 tlog 입력을 확인했다. 각 OBJ보다 나중에 바뀐 tracked local C++/header dependency는 없다. Product OBJ의 CodeView checksum27개(해당 source8개와 관련 header 기록)가 현재 원본과 모두 일치한다. 증거는 `out/KoukuRepair20260918/root/current-client-obj-checksums.json`이다. 새 Append/Bone rotation/loop UI·저장 문자열도 EXE에 존재한다.
- Client EXE/PDB의 RSDS GUID·age468이 일치한다. Client SHA256은 `ab93d1fc10996d5793c2daba48856107988763efcbbec0e82826dece399c42a4`다. 증거는 `out/KoukuRepair20260918/root/current-client-exe-audit.json`이다.
- Server Product OBJ3개(GameRoom_BossSimulation/GameplayCatalog/KoukuSaydonBrain)의 CodeView source SHA256과 현재 원본이 모두 일치한다. 관련 헤더4개도 일치하며 실제 링크 입력에 포함됐다. Server EXE/PDB GUID `59C7A9BD-2E3D-49F7-A565-EE1B1CD0ED84`, age402가 일치한다. DIA source checksum은 kind0이라 checksum 확인 근거로 사용하지 않았다.
- Composition source/product1664, World source/runtime2094가 유지된다. World는 전체 JSON semantic equal이며10 summon의 좌표/yaw/child84/MAP, P27/P81 mouth BONE+loop, 왼손24891ms, 공11832ms, 칼날8 emissions/11000ms loop, 원본 갈고리·즉사칼날 참조를 대조했다. 사용자 normal/red Effect source bytes도 보존됐다.
- 현재 Client 진단 로그에서 `connect.succeeded`와 `entry.accepted`를 확인했다. 빌드 로그에 C4819 인코딩 및 DirectXTK LNK4099 debug-symbol 경고는 있지만 Client 링크는 성공했다. 경고 없음으로 기록하지 않는다.

현재 EXE에 요청 코드가 반영되고 게시 후 새 프로세스로 실행된 상태다. 이 확인에서 앱 실행·종료·UI 조작·재게시·제품 재빌드는 수행하지 않았다. 실제 패턴 화면/회전/이펙트의 최종 시각 판정은 여전히 사용자 확인 범위다.

## 상세 근거

- [추적·소환 Server 결과](2026-09-18_KOUKU_TRACKING_SUMMON_REPAIR_RESULT.md)
- [World 칼날·공 복구](2026-09-18_KOUKU_WORLD_BLADE_REPAIR_RESULT.md)
- [왼손·입 본 실측](2026-09-18_KOUKU_TRAIL_MOUTH_REPAIR_RESULT.md)
- 설치/backup/field delta: out/KoukuRepair20260918/root,world,trail
- Source install before hash 2d851c161e12dc61e6c5321bac301f28b2b5fdcb638ec6582da26b3ff073c6d9, after hash a0d04631f06d5c9cde5bd4d8bcaacbadcb5acb404bcc41d5281433c5a66df62b

## G07. 쇼타임 Seek·Summon 배치·불뿜기·연출 정보 후속 결과 (20:14 KST)

이 절이 본 문서의 최신 상태다. 앞의17시 EXE/게시 확인은 당시 기록이며, 이번 변경으로 Client와 Server를20:12/20:10에 다시 빌드했다. 현재 두 프로세스는 실행 중이 아니다. 사용자 authored JSON은 이번 후속에서 수정하지 않았다.

### Profiler로 확인한 병목

입력은 `Client/Bin/ProfilerCaptures/profiler_20260918_200142_156_frame458_63832_7.json`이다. 마지막20프레임(439~458) 평균을 비교했다.

| 측정 범위 | 평균 CPU 시간 | 의미 |
|---|---:|---|
| 전체 frame | 1913.148ms | 약1.9초 지속 지연 |
| Effect.Playback.HistoryUpdate | 1739.255ms /18회 | 전체의90.91% |
| MainApp.Presentation.Prepare | 102.202ms | 위 비용과 별도 준비 범위 |
| ImGui.BuildAndSubmit | 51.001ms | 주 병목은 아님 |
| Render.World | 8.027ms | CPU 제출 측정; GPU 포화 단정 불가 |

첫 진입 frame426에는182007.325ms가 기록되었으나 scope8192 상한과 누적 droppedCpuScopes6881313 때문에 그 프레임의 세부 비용은 유실됐다. capture에 timeline ms 필드는 없어서44299/55637ms와의 대응은 사용자 관찰이다. 저장본 P35는392개의 presentation 행이 있으나 해당 두 시점의 활성 authored 행은 각각10/6개여서392개를 모두 그리는 문제라고 해석하지 않았다.

직접 원인은 `Effect_PresentationService::Commit_ExternalTransformHistorySample`의 provider가 `Sample_BundlePreviewPose`를 호출할 때마다0초부터 현재까지 모든30Hz 회전 사건과 root animation을 다시 계산하는 것이다. 정지 프레임의 Advance(0)도 FinalSample provider를 부른다. 긴 seek에서는60Hz particle history의 각 시점 안에 이 과거 재평가가 중첩됐다.

### 구현

`KoukuSaydonPresentationPlayer.h/.cpp`의 각 preview member가 완료된 event 시점의 yaw·animation별 yaw·누적 follow offset을 저장한다. 다음 요청은 가장 가까운 이전 checkpoint에서 시작한다. 같은 시각의 Stage→tracking 사건을 모두 처리한 뒤 저장하며, 최종 시점 pose도 최대16384개 재사용한다. 역방향 seek, 최초 관측 target 고정, 미관측 미래 실패 및 과거 샘플이 실제 actor pose를 바꾸지 않는 계약을 유지한다. member 교체와 명시 위치/yaw 변경 시 두 cache를 비운다.

Summon Box Detail의 `World Preview`와 pos/yaw 편집을 `Workbench → MainApp → Preview_SummonPlacement`로 연결했다. 현재 커서가 소환창 밖이면30Hz 최초 생성 tick으로 옮겨 정지 상태를 보여준다. 배치만 바꾸면 stable pattern/occurrence/spawn ID로 기존 배우를 찾아 대상 actor의 pose/이펙트 이력만 갱신한다. 열 명 전체의 모델 재생성을 반복하지 않는다.

회전 전용은 `Shared/Public/Gameplay/KoukuTargetTracking.h`의 `ROTATE_ONLY_RESPONSE_SCALE=10`을 Server와 Preview에서 함께 소비한다. 현재 저장본 영향은 P35의4개 SHOWTIME 회전행이며 공굴리기 이동추적180도/초는 유지한다. 새 header는 Shared vcxproj/filters에 등록했다.

| 쇼타임 occurrence | 창 시작/길이 | 실제 첫 tick 대기 | 90도 차이의 초기 속도: 이전→이번 |
|---|---|---:|---:|
| logic.11 | 5742/1513ms | 24.667ms | 60→600도/초 |
| logic.12 | 11066/1469ms | 0.667ms | 60→600도/초 |
| logic.14 | 25163/1848ms | 3.667ms | 48.214→482.143도/초 |
| logic.16 | 30791/1875ms | 9ms | 48.214→482.143도/초 |

이 값은 남은 각도에 비례하는 첫 tick 응답이며 고정 각속도가 아니다. overshoot를 막고 묶음 tick 처리와 개별 tick 처리를 같게 했다. 별도의 반응 대기 코드는 없었으며 저작 startMs는 이동하지 않았다. 화면에서 더 큰 시작 지연이 남으면 그 관찰을 기준으로 Logic 시작을 조정할 수 있다.

불뿜기와 세 연출은 [finite Preview·World clip 결과](2026-09-18_KOUKU_FINITE_PREVIEW_AND_WORLD_CLIP_INFO_RESULT.md)를 따른다. 단독 Effect Preview도 finite source 반복/원본 loop0 방출 연장을 구분하며 사용자 수명2340/2360/2361/8554ms를 보존한다. 화이트 P90 `presentation.7`의 정확한 B Effect 박스는 이미15333ms이므로 재기록하지 않았다. 개별 source particle 수명은 변경하지 않았다.

세 연출은 이미 설치 WModel의 World animation을 재생하고 있었다. Animation lane에 읽기전용 World clip 행을 추가했고 클릭하면 World Box Detail의 실제 clip/model/window/source-in/rate로 연결한다. catalog generation/revision 갱신 때 metadata를 읽으며 frame마다 파일 I/O를 추가하지 않았다.

### 실행한 검증

- 기존 native spatial probe를 out 전용으로 재사용: 현재 production 함수/struct와 실제 설치 MN_RPCT_05 CModel/rootMotion, **3931 checks /0 failures**. 순방향·역방향·같은 시각 Stage+tracking·follow offset·미관측 미래 거부·actor/관측 입력 불변·위치 수정 cache 무효화·cache 없는 계산과 pose 동등성을 검사했다.
- 정지44299ms 반복조회200회 평균: cache 없음20.7249ms → cache 있음0.001721ms.55637ms:85.336ms →0.0016825ms. 이는 root pose 함수 CPU 비용이며 전체 Client FPS 수치가 아니다.
- 관측 입력/checkpoint가 준비된 뒤 새로운60Hz source 시각3339개(0~55.637초)의 전체 root 조회448.236ms. 실제 particle 전체 seek/GPU render 시간으로 해석하지 않는다. 증거: `out/KoukuScrub20260918/probe/validation.receipt.json`, `run.log`.
- 실제 Shared 회전 header를 컴파일·실행한 **619 checks PASS**. zero/short window·±shortest arc·10배 첫 응답·grouped/per-tick 동등성과 overshoot,2^40/UINT64_MAX 경계를 검사했다. 기존 Server 전체 contract suite는 실행하지 않았다. `out/KoukuSummonPreview20260918/tracking_native_receipt.json`.
- finite Preview의 production 분기/provider 호출부 C++ probe **16/16 PASS**. 실제 입자 GPU 결과는 사용자 확인 대상이다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug -BuildLogDirectory out/KoukuScrub20260918/product-build`: **Product PASS**,120.3초. Engine/Shared 재컴파일0, Server OBJ67/EXE1, Client OBJ113/EXE1, CSO/PCH 재생성0. 기존 dirty source 의존성도 포함한 정상 증분 빌드다. C4819/C4828 인코딩 및 기존 C4244/C4805 경고가 있으므로 warning-free로 기록하지 않는다. 새 회전 helper의 tick 입력 폭은 uint64_t로 맞췄고 아래 최종 증분 빌드에서 해당 C4244 경고 제거를 확인했다.
- 전체 `git diff --check` exit0. Composition/World JSON parse와 원본 SHA 불변 확인, Shared project/filter XML parse PASS.
- 빌드 receipt: `out/BuildPipeline/runs/20260918T111218289Z-debug-product.json`. 실행 파일은 `Client/Bin/Debug/Client.exe`(20:12:16), `Server/Bin/Debug/Server.exe`(20:10:48). Client/UI 실행·조작·화면 캡처·domain publish는 수행하지 않았다.

### Collider·Sound 설계 결론과 남은 범위

기존 Duration/CombatObject를 확장하여 **공격1회 정의 → instance별 생성 시각·지면 → Effect/Sound 표현과 Server 판정창**을 연결하는 방향이 적합하다. 지금의 `_showtime_visual_template`은 EFFECT-only이며 showtime Server CombatObject의 Hits는 비어 있다. 거대 Effect 하나로 압축하는 것만으로 이 연결은 생기지 않는다. 개별 렌더 입자마다 Collider를 붙이지 않고 의미 있는 타격1회·잔불 영역별 hit window를 둔다.

구체 소유값, 기존 Server hit primitive 재사용, 고정/선택/추적/랜덤 위치, Sound one-shot 중복 방지와 seek 처리 순서는 [계획 G08](2026-09-18_KOUKU_PATTERN_RUNTIME_REPAIR_PLAN.md#g08-장판collidersound-설계-범위)에 기록했다. 이 후속 gameplay/audio 연결은 **설계 완료·미구현**이며 이번 최적화가 Collider 지원을 추가했다고 기록하지 않는다. 사용자 피해량·판정창·sound asset을 임의 생성하지 않았다. 노란 장판 전체 삭제/랜덤화도 하지 않았다.

### 사용자가 새 실행에서 확인할 경로

1. 이 저장소의 새 Debug Server와 Client를 실행한다. 이미 빌드한 EXE의 no-build 실행은 재설치가 아니며 Client 작업 디렉터리는 `Client/Default`다. 이번 코드 변경을 위한 데이터 publish는 필요 없다.
2. `Lobby → KoukuSaydon → F1 → Action Workbench`에서 훌라후프10명 소환 Box Detail → `World Preview` → Pos/Yaw를 바꿔 즉시 배치가 반영되는지 확인한다.
3. 쇼타임 커서를44299/55637ms로 옮기고 정지 상태의 응답과 Profiler를 다시 확인한다. 새 전체FPS는 아직 측정하지 않았다. 회전 Duration 진입의 반응도 확인한다.
4. 공굴리기/대형세이튼 불을 단독 Preview에서 재생해 저장 lifetime과 반복 경계를 확인한다. 세 연출의 Animation lane에서 `World: <clip>`을 선택해 World 상세 정보가 나오는지 확인한다.

소유권이 섞인 대규모 dirty worktree이므로 자동 stage/commit/push는 하지 않았다.

### G07 최종64-bit tick 보정 빌드

회전 helper가 Server의64-bit 경과/남은 tick을 그대로 받도록 보정했다.619 native 검사에는2^40/UINT64_MAX 경계8개가 포함된다. 최종 Product Debug Build는16.488초, Engine/Shared OBJ0, Server OBJ1, Client OBJ1, 각EXE1, CSO/PCH0으로 PASS다. 회전 호출부의 새 C4244 경고는 없다. 최종 receipt는 `out/BuildPipeline/runs/20260918T111713742Z-debug-product.json`, 로그는 `out/KoukuScrub20260918/product-build-final.log`다. 최종 Client EXE는20:17:12, Server EXE는20:17:02이며 이 출력이 앞20:12/20:10 출력을 대체한다. 데이터 게시 및 제품 프로세스 실행은 하지 않았다.

## G09. 룰렛·World cue·마리오 수신 범위 후속 (2026-09-19)

### 실제 소스 변경

- `GameRoom_KoukuAudition.cpp`: 지지면 교체 전후의 Server navigation 높이 차를 source root-motion 보스에 한 번 적용한다. 곡선의 지면 상대 높이를 유지하고 첫 origin capture에 음수 바닥 offset이 남는 결함을 막는다. spawn reset도 현재 활성 지지면을 확인한다.
- `KoukuSaydonLogicRuntime.cpp`: 실제 P24 휠윈드는 RootMotion이 아니라10m/3204ms charge를 사용한다. 목적지 높이만 확인하던 charge에 segment LOS,1mm 경계 bisect, collision 이후 재검사를 연결했다. 막힌 셀 너머 walkable 목적지로 건너뛰지 않는다.
- `Level_KakulSaydonArena.cpp/.h`와 Server `Stop_KoukuWorldOwner`: Mario parent와 P33이 공유하는 run/member 종료를 기존 `iPatternSequence`로 한정한다. reliable stop과 persistent terminal snapshot 양쪽에 적용했다. 전체 run 종료와 cue 파괴는 계속 전체/해당 cue를 정리하며 Shared wire layout은 변경하지 않았다.
- World group/NEXT motion의 V1을 함께 준비하고 cold 상태의 reliable PLAY를 pending으로 유지한다. 준비 후 원래 Server 시각으로 catch-up하며 만료·종료·실제 리소스 실패는 구분한다. P83은 이미6개 motion,5회 분열의 총63개 공으로 연결되어 있어 저장된 개수·배율을 변경하지 않았다.
- `KoukuSaydonPresentationPlayer.cpp`, `EffectV2_Runtime.cpp`: 첫 product snapshot이0초 뒤에 도착하면 최초 관측 pose를 새 occurrence의 초기 birth 구간에만 제공한다. 실제 과거 pose 복원이 아니라 시작/늦은 참가 표현 근사다. 이후 missing-history/teleport 거절과 preview의 엄격한 이력은 유지한다. 긴 시계의 float endpoint 오차는 정밀도 기반 허용치로 처리한다.
- 네 Mario intro camera는 로컬 snapshot의 해당 `iMarioStage`에만 적용한다. room World sequence 재생만으로 미입장자 카메라·입력을 빼앗지 않는다. 명시적 authoring preview와 일반 공용 컷씬은 유지한다.

### 비둘기 중앙이동 저작 반영

P82의 `kakulsaydon.g1.logic.79`는 이름과 TRIGGER 종류만 있고 실행 필드가 없었다. 저장본1753→1754에 `triggerKind=BOSS_TELEPORT_XZ`, `teleportPosition=[-0.07,1.32,737.53]` 두 필드만 추가했다. 대상은 Gameplay.world와 설치 worldbootstrap이 일치하는 `boss.kakulsaydon.g1.saydon` 중심이다. 기존4587ms 시각·pattern·Effect는 보존했고 XZ 이동/높이는 기존 Server teleport/root/nav가 소비한다.

Client/Server 프로세스가 없는 상태에서 최신 저장본 재독해, stable ID 필드 병합, 동일 필드 충돌 검사, 백업, 최종 hash 검사, 원자 교체를 수행했다. 근거는 `out/KoukuNavigation20260919/pigeon-center.installation.json`과 같은 폴더 backup이다. 전체 문서에는 수정 전부터 `presentation occurrence exceeds the Pattern lifetime`인 별도 미게시 draft가 있다. 이를 수정하지 않았으며 실제 P82 publication closure의 validate/publishable/project 검사를 통과했다.

`logic.78` 사라지기는 비어 있고 참조 occurrence도 없다. 원본4219941 HidePawn4개를 조사했지만 현재 Kouku visibility 소비자는 없으므로 중앙이동 수정이 원본 HidePawn 복원까지 구현한 것은 아니다. 정확한 원본 시간·hash는 `out/KoukuNavigation20260919/NAVIGATION_RESULT.md`에 있다.

### 실행한 집중 검사

| 검사 | 결과와 한계 |
|---|---|
| 실제 camera selector/input lambda 추출 |71개 통과. 수정 전 동일 검사49개 실패. stage4종, 미입장, 다른 stage, 복귀, 공용 camera와 입력 확인. 화면 검사는 아님. |
| 실제 World cue 소비자/terminal lambda 추출 |16개 통과. resource/render는 test double. parent→P33, 늦은 stop, 전체 종료, async 준비·중복·catch-up·만료·실패 확인. |
| 실제 pivot history/bootstrap + DirectXMath |82개 통과.69개 float endpoint 오류 재현. ±ULP/teleport endpoint, 실제 내부 단절·미래·NaN 거절, preview 엄격성 확인. |
| 대상 resource closure |19개 V1,98개 설치 경로, 누락0. P33/P82/P83/P84/P90 연결 및63개 공 확인. GPU/화면 PASS 아님. |
| Server 변경3개 TU 격리 컴파일 |통과. Product 링크와 runtime contract는 별도 기록. |

근거는 `out/KoukuRuntimeRepair20260919`, `out/KoukuWorldRepair20260919`, `out/KoukuNavigation20260919`다. 독립 읽기 전용 검토에서 support delta 중복, charge tunnelling, Mario audience의 추가 재현 결함은 발견되지 않았다. dirty WIP를 최종 PR 승인으로 표시하지 않는다.

### 게시와 쿠크 원본 조명 비교

`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1754`는 exit0, map.kakulsaydon/world.gameplay/gameplay.balance PASS다. 게시된 Encounter와 patternbindings의 sourceRevision은1754이며 P82의4587ms `BOSS_TELEPORT_XZ`를 확인했다. 실제 resource closure도1754 기준 재검사하여19개 V1/98개 경로 누락0이다. 실행 중 Server의 catalog나 Client의 메모리 draft를 Reload하지 않았다.

`RenderingBenchmark`의 기존 Rendering restoration에 Current authored lights / Imported source lights (2026-09-11) / Map lights off를 추가했다. `CLevel_KakulSaydonArena`의 세션 상태만 바꾸며 reference parse와 관문·팝업 provider stage가 성공한 뒤 commit한다. 실패 시 기존 provider를 보존한다. Return to entry/Workbench 닫기는 Current로 돌아가고, 캡처 중 전환은 막으며 비교 모드를 capture condition에 포함한다. Scene/Effect light는 이 선택에 포함하지 않는다.

비교 원본은 commit `359412c46d12fd5a0df3155045d6468a01acfd12`의115개 import stable ID다. 기존4개 저작 ID와 현재 추가7개를 이름 추정 없이 구분했다. 현재115개 원본의 위치·색·광도·범위·cone은 import와 float32 기준 동일하다. 과거 SOURCE_CHARACTER84개는 현재 수정된 UNBAKED로 유지하고 ALL31개도 유지한다. G1의-204.8m popup 변환, G3 mask, Composition popup의 별도 배치를 그대로 소비한다. 이 비교는 현재 renderer에서 추가 조명의 영향을 분리하며 원본 게임과 시각적으로 동일하다는 증거는 아니다.

`Data/Rendering/Reference/KoukuImportedSourceLights.maplights.json` SHA256은 `3ed3b3d79fb3db598228fe996c7b0863136e27c7e68d0689904f26434efba34e`다. 실제 CMapLightDocument parse115/UNBAKED84/ALL31, Serialize→Parse115, 실패 load 시 기존115개 유지 검사를 통과했다. Client 프로젝트와 filters에 `None`으로 등록했다. Level/Benchmark 최종 MSVC `/Zs`도 통과했다.

Kouku source profile의 UE3 tone scale.85/range8/toe1/desaturation.15와 neutral LUT는 기존 연결을 사용한다. 원본 LUT override가 꺼진 자료를 근거 없이 켜지 않았다. 추가 조명7개가 현재 화질 불만의 원인인지는 사용자 A/B 확인 전까지 미확정이다. 상세 원본·검증 근거는 `out/GhostMaterial20260919/rendering-notes.md`에 있다.

### 유령 발탄의 확인 범위

실제 설치 ghost body3slot,87bone, preScale.01, material program84와 설치 shader pass10을 사용한 headless WARP 검사에서 변경192pixel/alphaMax.947501/nonfinite0, 모든 bind/pass HRESULT0을 확인했다. 전용 donor와 current88개 pattern clip도 연결되어 있고 idle/groggy/portal CPU pose는 finite metre scale이다. 아레나 phase>=3의 ghost 교체·BLEND 제출·유한 hide window를 추적했지만 현재 조사만으로 실제 화면의 누락 원인은 재현하지 못했다. 이 결과를 유령 발탄 수정 완료나 실제 아레나 표시 PASS로 기록하지 않는다.

Effect Tool Full Restore는 현재 normal BOSS_VALTAN만 선택한다는 별도 제한을 확인했다. 이는 보통 일반 몸체가 나오는 제한이며, 사용자가 말한 완전한 비표시 원인이라고 확정하지 않았다. `out/KoukuNavigation20260919/GHOST_VALTAN_READONLY_RESULT.md`에 실제 carrier/phase/hidden 근거와 남은 재현 경계를 기록했다.

### Product 빌드 상태

정상 Product Debug를 시도했으나 Debug Client PID51696와 Server PID54260이 각 표준 EXE를 사용 중이라 Product output guard에서 exit1로 중단됐다. 컴파일·링크 성공으로 기록하지 않는다. 사용자의 저장·종료를 요청했고 자동 종료나 Reload를 하지 않았다. 로그는 `out/KoukuRuntimeRepair20260919/product-build.log`, receipt는 `out/BuildPipeline/runs/20260918T230006782Z-debug-product.json`이다. 현재 실행 EXE는 이번 최종 코드의 설치 결과가 아니다.

### 추가 최소 컴파일와 구조 검사

변경된 Client `EffectV2_Runtime.cpp`, `KoukuSaydonPresentationPlayer.cpp`도 정본 VS14.44 x64/Debug 설정의 격리 `/Zs`를 통과했다. 앞의 Level/Benchmark 검사와 합쳐 Client 변경4개 TU의 문법/타입 검사를 확인했다. `out/KoukuRuntimeRepair20260919/compile-world.log`가 근거이며 표준 Client EXE 링크를 대신하지 않는다.

변경/게시 JSON7개, Client vcxproj/filters2개 parse와 실제 reference 파일 연결,115개 light receiver84/31 계약을 통과했다. `out/KoukuRuntimeRepair20260919/final-structure-validation.json`에 경로와 revision을 기록했다. 전체 `git diff --check`도 통과했다(기존 autocrlf 알림만 존재).

### 사용자가 새 바이너리에서 확인할 범위

1. 저장 후 실행 중 Debug Client/Server를 닫고 정상 Product Debug Build를 완료해야 이번 최종 C++가 실행 파일에 반영된다. 이후 Server를 새로 시작해야1754 게시 데이터를 새 catalog로 소비한다. 자동 종료·메모리 Reload·Client 실행은 하지 않았다.
2. `Lobby → KoukuSaydon → F1 → Action Workbench`의 Server Play에서 룰렛 생성 직후 세이튼 발높이, 다음 root motion, 룰렛 제거 후 높이와 P24 휠윈드의 후퇴 경계를 확인한다.
3. P82 중앙 이동/비둘기, P83 투하의5회 분열63개 공, 공던지기·훌라후프, Mario 후속 P33 갈고리·일반/즉사 칼날을 cold 첫 재생과 반복 재생에서 확인한다. resource closure·cue 검사와 실제 GPU 표시는 별도다.
4. 두 참가자 중 한 명만 Mario에 들어가면 입장자의 intro camera만 재생되고 미입장자의 카메라·이동입력은 유지되는지 확인한다.
5. `F1 → Rendering Workbench → Rendering restoration`에서 Before/Restored source profile과 Map light comparison을 각각 비교한다. Current/Return to entry/도구 닫기는 사용자 저작 조명으로 돌아간다. 유령 발탄은 실제 아레나 본체/portal 분신/Full Restore 중 실패 경로를 아직 구분해야 하며 수정 완료로 보지 않는다.

### 발탄420633 재질 승인 실패의 별도 수정

사용자의 기존 `Client/Default/EffectFailure.user.log`07:22:57에는 `effect.valtan.pattern.420633.active`가 `Native Artist requires its recovered material variant, carrier and named inputs`로 거부된 기록이 있었다. 이는 유령 몸체가 아니라 입장/sequence/일반 휠윈드 Effect의 별도 결함이다. native2377/2378/2379를 쓰는 `valtan.420633.notify004.emitter5259/5260/5258`은 실제 trail+`animationTrailBakedEdgeV1`인데 비활성 SourceRecipe의 `rendererShape`만 sprite로 남아 있었다. 재질 계약도 이 carrier metadata를 검사하므로 전체 문서가 거부됐다.

`Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json`의 해당3개 필드만 `animationTrail`로 고쳤다(3+/3-). alpha·texture·shader·scale·history·다른 element는 변경하지 않았다. 최신 저장본 stable ID/기존 kind/carrier/program 검사, 백업, 최종 hash 재확인, 원자 교체 후 native 검증 후보와 byte-identical인 것을 확인했다. receipt는 `out/GhostMaterial20260919/whirlwind_shape_install.receipt.json`이다.

현재 실제 CEffectDocumentCodec은 수정 전 로그와 같은 실패를 재현했고 수정 후9elements/1history를 승인했다. 각3개 field를 sprite로 되돌리는 negative 검사도 모두 거부했다. `Capture_ProductLoadStageRequest → Stage_ProductLoadTarget`은 projection1로 성공했고 actual projection playback에서3trail/최대157edge pair가 생성됐다. 이는 GPU/나머지 particle5개나 유령 본체 화면 확인이 아니다. log는 `out/GhostMaterial20260919/whirlwind_shape_probe.log`다.

이 파일은 EffectCatalog가 직접 읽는 Authored 원본이라 별도 publisher 대상이 아니다. 설치된 문서는 검증 후보와 동일하지만 이미 캐시된 immutable target이나 열린 미저장 draft는 교체하지 않았다. 새 Client/사용자의 명시적 Reload에서 새 디스크 문서를 읽어야 한다. 최종 구조 검사 범위는 JSON8개/XML2개로 늘었다.

### 실제 Server 회귀 실행 결과

표준 출력이 실행 중이므로 모든 산출물을 `out/KoukuNavigation20260919/isolated-contract`에 둔 별도 실행 파일로 현재 Server 함수를 연결했다. 변경3개 TU는 현재 소스로 다시 컴파일했고, 나머지 Server/PCH90개·Shared8개 object의 소스/의존성 시각·hash를 감사했다. 이 검사는 표준 Product 교체가 아니다.

- 신규 룰렛 지지면6개/실제 P24 휠윈드4개, 총10개 회귀 모두 PASS.
- `--kouku-bundle-contract-test`:91 PASS/0 FAIL, exit0.
- `--kouku-support-surface-contract-test`:197 PASS/4 FAIL, exit1. 동일 의존성에 HEAD `8dc16688517f567c64976434f24bb7425f0e0830`의 변경 전3개 TU를 별도 컴파일·링크한 baseline은187 PASS/동일4 FAIL이었다. 이번 수정으로 추가된 실패는 없지만 이 suite 전체가 통과한 것은 아니다.
- 기존4개 실패는 finite card 종료 burst, swept card 접촉 explosion/retirement, 추적자 반속 이동, 동일 tick 재실행 항목이다. 별도 원인 수정은 이번 요청에 섞지 않았다.

정확한 검사 문구, 실행 exit/count, executable/Shared/bootstrap hash, 기존 소스와 object의 불변 확인은 `isolated-contract/verification.json`에 있다. Client와 실행 중 Server를 종료하거나 조작하지 않았다.

### 조명 비교 최종 검토 보완

독립 검토에서 발견한 두 경계를 수정했다. Return to entry는 scene profile 복구가 성공한 뒤에만 map light 비교를 초기화하므로 profile activation 실패 시 기존 조명도 보존한다. 캡처 조건에는 transient light 활성, 실제 map intensity multiplier, 선택 provider의 stable ID/위치/색/광도/receiver/관문·popup 상태까지 포함하여 캡처 중 다른 조명 설정이 바뀐 A/B를 동일 조건으로 취급하지 않는다. 이를 위해 `MapLightPresentationRuntime.h`에 현재 multiplier의 읽기 전용 getter만 추가했다. 최종 Level/Benchmark `/Zs`와 diff 검사를 다시 통과했다.

최종 상태는 소스·위3개 Effect field·1754 게시 데이터 반영, 집중 검사 완료이며 표준 Product 링크와 사용자 화면 검증은 미완료다. 유령 발탄 본체의 비표시 원인은 미확정이다. 실행 중 Debug Client/Server가 유지되어 빌드 차단 상태도 그대로다. 무관한 기존 transport/MainApp/배포 문서 변경을 보존하고 자동 stage/commit/push하지 않았다.

## G10. 레이드 진입 전 Effect dependency 준비 (2026-09-19)

### 원인과 실제 연결

Shader는 이미 Debug/Release 빌드에서 CSO를 굽고 `Shader.cpp`가 module-adjacent CSO를 읽어 `D3DX11CreateEffectFromMemory`로 생성한다. 패턴 중 보인 준비를 전부 런타임 HLSL 컴파일이라고 해석하지 않는다. 실제 누락은 Level_Loading의 쿠크 대상이 BossCatalog combat-object Effect 중심이어서 Product occurrence, Sequence composition, World effectTracks가 충분히 포함되지 않은 것이다. 첫 `Sample`의 V1 priority queue와 V2 `Load_ResourceSnapshot`, World `Prepare_ObjectResources`가 JSON·shader effect 객체·model·texture 준비를 첫 재생 때 수행할 수 있었다.

조사 당시 Product sourceRevision1770 / Sequence revision65 / World revision2129에서 Product85패턴·Effect occurrence698개를 확인했다. 직접 V1 ID84개 중 기존 BossCatalog 대상과 겹치는 것은1개여서83개가 이 초기 대상에서 빠졌다. Product·사용되는 Sequence resource·enabled World template 및 Server state로 선택되는 card/ball을 합한 closure는 V1 119개, V2 45개(28 groups, group 확장 후 leaf79개)였다. 참조 V1 및 V2 JSON 누락은0개였다. 이후 사용자 저장본이나 publish로 수가 바뀔 수 있으므로 이 수를 영구 고정 요구사항으로 쓰지 않는다.

`KoukuSaydonPresentationPlayer::Collect_ProductEffectTargets`가 기존 Product loader와 Sequence 경로·published World 문서를 읽어 이 closure를 수집한다. Sequence resourceId·World templateId 참조가 빠지면 진입 준비에서 실패한다. 새 manifest나 별도 이펙트 런타임을 만들지 않고 기존 V1 Loader worker/target gate와 V2 catalog/Prewarm_Group을 사용한다. selected class는 Server가 승인한 `Get_LocalCharacterClass()`를 사용해 실제 Loader class와 맞춘다.

`Level_Loading`은 해당 V1 및 선택 class 준비를 마치고 Loader producer가 끝난 뒤 V2를 준비한다. 필수 쿠크 Effect의 등록·준비 실패는 상태에 남기고 기존 load recovery로 되돌린다. 실패를 격리 성공으로 간주해 전투에 들어가지 않는다. `Level_KakulSaydonArena::Initialize`에서 prepared World document의 모든 enabled instance 자원과 기존 Joker clone prewarm도 입장 전에 준비하고 실패하면 E_FAIL로 닫는다.

V2 snapshot은 owner thread에서 준비한 immutable set을 runtime consumer가 재사용한다. 전역 Effect cache generation이 바뀌면 이 set도 비워 기존 tool save/reload invalidation 계약을 유지한다. 새 레이드 준비에서 기존 set을 교체하므로 무한히 누적되는 전역 catalog를 만들지 않는다. 사용자 draft나 runtime 도구 상태를 강제로 Reload하지 않았다.

### enabled World 범위 검증

현재 published World instances326개 모두 enabled이며 NEXT2건도 enabled target을 가리킨다. `WorldSequencePlayer.cpp`의 `Play`는 disabled를 거부하고 `Prepare_ObjectMotionChain`은 NEXT의 disabled target에서 실패한다. `Apply_ObjectMotion`과 `Resolve_ObjectMotion`도 disabled를 거부한다. `WorldSequencePlayer_Objects.cpp::Prepare_InstanceResources`는 같은 gate를 사용한다. `Level_KakulSaydonArena.cpp::CompositionWorldMotions`는 group의 enabled member만 반환하며, direct ID 역시 최종 Play/Prepare gate를 통과해야 한다. 따라서 enabled-only 수집이 실제 explicit PLAY·NEXT·Composition 재생 가능한 범위를 누락하지 않는다.

### 실행한 검증과 남은 비용

- 저장된 Product·Sequence·World 및 V1/V2 dependency를 별도 구조 검사로 대조했고 누락0을 확인했다. disabled/NEXT consumer는 실제 C++ 분기로 추적했다.
- 변경 Client5개 C++/header의 기존 인코딩·줄끝을 보존했다. `Level_KakulSaydonArena.cpp`의 BOM도 유지했다. 담당 파일 `git diff --check` PASS.
- 사용자 저장·종료 확인 뒤 root의 정상 Product Release/Debug 빌드가 모두 PASS했다. receipt는 `out/BuildPipeline/runs/20260919T084237404Z-release-product.json`, `20260919T084643387Z-debug-product.json`이다. 이 G10의 preload 소스도 표준 Client에 반영됐다. Client/UI를 실행하지 않았다.
- 처음 `Test-CompiledShaderClosure.ps1 -Configuration Release -Modules Product`는 `Shader_VtxEffectParticleArtist448.hlsl` family technique/pass count mismatch로 실패했고 정상 Product 재빌드 후에도 동일했다. 원인은 검사기가 모든 particle을5-pass로 가정한 것이며 현재 ARTIST family7은 원본 Modulate용6번째 pass를 실제 소비하고 있었다. `ProductEffectShaderWarpProbe.cpp`를 현재 ABI에 맞춰6번째 pass 이름·SourceModulate PS·Dst*Src blend·RT0 alpha 보존·독립 RT1/2 write 차단까지 검증하도록 수정했다. 제품 source/CSO를 바꾸거나 검사를 완화하지 않았다. 최종 Debug/Release 모두99 family/8 resource-root/140 FxCompile/120 Client consumer와 V1·V2 각1352 pixels 검사를 PASS했다. 로그는 `out/KoukuRenderingQuality20260919/shader-closure-release-after.log` 및 `shader-closure-debug-after.log`다.
- 이 변경은 패턴 시작 시 발생하던 immutable asset·V2 JSON 준비를 진입 시점으로 옮긴다. 새 instance clone, particle/trail mutable buffer allocation, provider/history sampling, draw work 자체는 재생 중에도 필요하다. 모든 frame allocation이나 끊김이0이라고 주장하지 않는다.
- Sequence runtime metadata parsing, runtime tool save 뒤의 명시 cache invalidation/reload, 실제 cold 첫 패턴의 frametime 및 GPU 표시 확인은 위 구조 검사와 별도다. Product 빌드는 완료됐으나 새 Client 실행·화면 검증은 사용자 경계로 남긴다.

G09 작성 시점의 표준 Product 잠금·미링크 상태는 사용자의 저장·종료 확인과 위 G10 Debug/Release 빌드로 해소됐다. G09의 별도 Server 기능 검사4개 기존 실패 및 유령 발탄 실제 화면 미확정 상태를 이번 선준비 검증의 PASS로 바꾸지는 않는다.
