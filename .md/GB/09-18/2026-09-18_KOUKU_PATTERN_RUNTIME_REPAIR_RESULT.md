# 쿠크 패턴 실제 재생 연결·복구 결과

최신 후속 상태는 아래 **G07. 쇼타임 Seek·Summon 배치·불뿜기·연출 정보** 절에 기록한다.

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
