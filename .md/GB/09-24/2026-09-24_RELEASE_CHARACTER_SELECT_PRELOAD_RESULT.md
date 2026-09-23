# Release Character Select 전 class 준비 및 Bern hotpath 점검 결과

## G00. 반영 상태

Release Character Select 입장에서 현재 선택 화면의 7 class를 미리 준비한다. 기존 선택 class 준비를 전체 roster로 확장한 것이며, 선택 class/identity의 Server 승인과 class 변경 transaction은 그대로 사용한다. Debug와 Bern·raid 입장은 기존 선택 class 준비 범위다. 실제 입장 시간, 메모리 증가, class 변경 체감과 4인 이동/FPS는 Client를 실행하지 않아 아직 확인하지 않았다.

현재 통합 브랜치의 다른 담당 변경과 함께 작업했다. 이 RESULT의 구현·검증 범위는 아래 9 C++ 파일과 Loading 사용서 문단이다. 전체 Product 빌드, EngineSDK 갱신, publish, commit은 통합 담당이 수행한다.

## G01. Release preload 구현

| 파일 | 실제 변경 |
|---|---|
| `Client/Public/CharacterCatalog.h` | 기존 선택 화면 순서의 `CHARACTER_SELECT_CLASSES`를 공용 constexpr 목록으로 이동했다. Lance Master, Gunslinger, Slayer, Artist, Dimension Master, Warlord, Guardian Knight 7 class다. |
| `Client/Public/Level_CharacterSelect.h` | `SUPPORTED_CLASSES`가 같은 roster를 사용한다. |
| `Client/Public/Loader.h` | immutable `AUTHORING_INPUT`을 class ID별 map으로 보유한다. |
| `Client/Private/Loader.cpp` | owner thread가 Release Character Select 전체 roster 입력을 캡처하고 기존 worker의 `Ensure_Prototypes`에 전달한다. 기존 body/part/animation/shader 준비와 rollback/cancel을 사용한다. |
| `Client/Private/Level_Loading.cpp` | 같은 roster의 skillbinding Effect cue를 기존 우선 준비 큐에 넣고 dedup한 정확한 target의 settled 상태로 activation을 판단한다. optional class 문서 실패는 해당 표현만 격리하고 다른 class 준비를 계속한다. |

새 model/Effect runtime, packet, 데이터 정본과 프로젝트 항목은 추가하지 않았다. `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 Loading 설명을 Debug 선택 class / Release 전체 roster 준비 및 입장 시간·메모리 교환 관계로 갱신했다.

## G02. Bern에서 확정된 불필요 계산과 최소 수정

09-22의 `829385641`에서 추가한 `VIEW_LOD_ENVELOPE`가 visibility cache miss마다 camera linear scale bound를 계산하고, visible instance마다 view 공간 bounds를 누적한다. 그러나 `CMesh::Render_Instanced`는 실제 생성된 `m_StaticLod`가 없거나 morph mesh이면 이 값을 LOD 선택에 사용하지 않는다. 작은 mesh와 LOD 생성이 성립하지 않는 mesh도 같은 추가 계산을 하고 있었다.

`Engine/Public/Model.h`, `Engine/Private/Model.cpp`에 실제 준비된 static LOD의 read-only 조회 `Has_StaticMeshLod`를 추가했다. `Client/Public/MapStaticBatchObject.h`, `Client/Private/MapStaticBatchObject.cpp`는 geometry가 고정되는 component clone 직후 모델에 해당 LOD가 하나라도 있는지 한 번 캐시한다. 없는 batch만 tight view envelope 계산을 건너뛴다.

기존 world envelope와 frustum culling, instance payload, material/texture admission, LOD 품질 선택, profiler의 source/submitted indices 및 LOD0 분모는 유지했다. profiler나 LOD 기능 전체를 rollback하지 않았다. 이 변경은 불필요 CPU 계산의 제거이며, 보고된 Bern 전체 FPS 회귀의 원인을 모두 확정하거나 해결한 증거는 아니다.

현재 `CProfiler`는 enabled/collecting 기본값이 false이고 상세 per-draw 수집은 opt-in이며 `Add_Counter`는 비활성 상태에서 일찍 반환한다. 따라서 F7 창이 닫혀 있다는 이유만으로 전체 capture가 항상 실행되는 구조로 판단하지 않았다. 09-22 기존 RESULT에도 제품 Bern의 같은 카메라·같은 구성 전후 FPS 확인은 남아 있었다. 보관된 큰 Bern capture는 이전 날짜 자료이고 현재 변경과 같은 조건의 비교가 아니다.

## G03. 수행한 검증

MSVC 14.44.35207, Windows SDK 10.0.26100.0의 x64 compiler로 독립 out 경로에 `/std:c++20 /Y-` 컴파일했다. Debug는 `/MDd /D_DEBUG /Od /RTC1`, Release는 `/MD /DNDEBUG /O2`다.

| 검증 | 결과 | 로컬 증거 |
|---|---|---|
| Loader, Level_Loading, Level_CharacterSelect, Debug/Release | 6 TU 모두 exit 0 | `out/ReleaseCharacterSelect20260924/compile-results.json` |
| Model, MapStaticBatchObject, Debug/Release | 4 TU 모두 exit 0 | 같은 폴더 `lod-compile-results.json` |
| 기존 ClientPresentationPrimitiveContractTests + 실제 ActionPresentationTimeline, Debug/Release native 실행 | 둘 다 PASS / exit 0 | `cpu-probe-results.json`, `movement_probe-Debug.log`, `movement_probe-Release.log` |
| 실제 `VIEW_LOD_ENVELOPE` helper를 추출한 CPU probe, Debug/Release | 각 구성 500,000 enclosure, 1,000 LOD envelope parity, 1,000 no-LOD guard case PASS | `lod_envelope_probe-Debug.log`, `lod_envelope_probe-Release.log` |
| 기존 9 C++ 파일 encoding/line ending | HEAD와 같은 UTF-8 BOM 없음, bare LF 없음, CRLF 유지 | `encoding-audit.json` |
| 소유 변경의 `git diff --check` | PASS | 통합 전 실행 |

CPU probe의 모든 batch에 LOD가 없는 합성 fixture는 16,421 batch × 100 frame, ABBA 순서 5회로 실행했다. Release 중앙값은 기존 1.294 ms / guard 0.040 ms, Debug는 4.938 ms / 0.284 ms였다. **이 값은 helper CPU fixture이며 실제 Bern FPS, 실제 no-LOD batch 비율, GPU frame time이 아니다.** GPU와 Client UI는 실행하지 않았다.

기존 이동 tests는 빠른 두 번째 클릭, 이전 클릭 ACK, stale tick/regressing sequence, sequence wrap, 반대 방향 재클릭, ground height, snapshot/ACK timeout과 discontinuous teleport를 포함한다. 최초 검증에서는 원본 테스트 코드를 변경하지 않았다. 후속 지연 snapshot 검증은 G06에 추가했다. 최초 runner 링크는 `CActionPresentationTimeline::Is_ForwardTick` 미해결로 실패했고 실제 `ActionPresentationTimeline.cpp`를 링크한 뒤 두 구성 모두 통과했다.

## G04. 기존 Release 로그의 시점·world 구분

`Client/Bin/Release/Diagnostics/client-session-*.jsonl`, Server room-perf, Client EffectFailure 로그를 읽었다. 확인한 최근 자료는 09-23이며 local endpoint도 공유 Server와 같은 192.168.0.22다. 이를 사용자 최신 4-PC 테스트의 재현 자료로 단정하지 않는다.

Character Select(`worldId=4`) 구간만 분리한 최근 11 session 중 PID 38940은 main-pump gap 최대 656 ms, 기록된 stall 0회, receive age 최대 46 ms였다. PID 59276은 gap 최대 1,765 ms/1회, PID 46332는 1,890 ms/2회였다. 나머지 최신 session은 최대 562~938 ms 범위였다. 이 누적 max는 Loading 진입 시점을 포함할 수 있어 일정 프레임 이동의 네트워크 지연과 동일하지 않다. 요약은 `existing-release-character-select-logs.json`에 있다.

8.1초 main-pump stall은 Character Select가 아니라 **Bern(`worldId=1`)**이었다. PID 38940의 entry acceptance와 동일 PID EffectFailure를 시간으로 결합했다.

| stall 종료 Unix ms | gap | 직전 Bern 승인 후 | 재개 시 receive age | raw queue |
|---|---:|---:|---:|---:|
| 1790142276175 | 8,187 ms | 32.367 s | 9 ms | 32 |
| 1790142351781 | 1,984 ms | 107.973 s | 15 ms | 30 |
| 1790143314629 | 8,141 ms | 29.972 s | 26 ms | 30 |

이 세 stall 구간에 같은 PID의 EffectFailure 기록은 없었다. 8초 구간은 Bern 승인 약 30초 뒤에 끝난 반복 패턴이므로 Loading 후 synchronous activation/map placement는 조사 후보지만, 당시 scope capture가 없어 특정 함수가 8초를 사용했다고 확정하지 못한다. receive age가 작다는 것은 재개 직전 socket 수신이 살아 있었다는 근거이며 stall 전체 동안 네트워크가 정상이었다는 증명은 아니다. 결합 근거는 `existing-stall-join.json`에 있다.

Server room-perf에 tick maximum 약 10.6 ms, loop lateness maximum 약 21.5 ms, loop reset 0인 기록이 있고 wire send maximum 1.8초가 있는 기록도 있다. 누적 최고값을 Client 한 stall의 원인이나 같은 시점의 4인 simulation 부하로 결합할 수 없어 Server tick을 임의 변경하지 않았다.

## G05. 남은 범위

일반 클릭은 typed movement sequence와 prediction을 사용하며 직접 teleport 경로는 Debug 명령에만 있다. `LocalMovePrediction`은 09-22 profiler 변경에서 바뀌지 않았다. 이동 tests가 통과해도 실제 더블클릭 순간이동이 없다고 결론 내리지 않았다. 재현된 입력·snapshot·frame 시점 자료 없이 correction threshold, timeout, socket dispatch를 임의로 변경하지 않았다.

Release 전체 roster preload가 제거하는 것은 class 첫 model/Effect 준비 대기다. 초기 Loading 시간과 상주 메모리는 늘어날 수 있다. 실제 4인 테스트의 일반 이동 밀림과 더블클릭, Bern의 정상 플레이 중 FPS 저하, 반복된 activation stall의 정확한 scope는 미확인으로 남긴다. 통합 Product 빌드 뒤 사용자가 같은 Release 환경에서 확인할 대상이다.

## G06. 지연 snapshot 시 강제 snap 경로 수정

추가 읽기 조사에서 일반 이동의 두 번째 우클릭이 별도 teleport handler를 호출하는 경로는 없었다. gameplay는 DirectInput press/hold와 typed `C2S_MOVE` 한 경로이며, `WM_RBUTTONDBLCLK` 처리는 외부 ImGui backend의 일반 down 변환뿐이다. Debug placement update는 Release에서 컴파일되지 않는다. class presentation 교체는 `Rebind_LocalCharacter`로 입력 sequence를 보존한다. 교체 대기 동안 Controller 호출이 생략되므로 대기 중 raw edge 관찰이 중단되는 점은 확인했지만, 그 자체로 transform 순간이동을 만들지는 않아 이 작업에서 변경하지 않았다.

확정한 별도 결함은 `CLocalMovePrediction::IsDiscontinuous`의 경과 시간 계산이다. Server tick 간격을 client prediction freshness인 0.35초로 잘랐기 때문에, 긴 stall/coalescing 뒤 정상 속도 이동도 teleport RESET이 될 수 있었다. speed 2.95m/s에서 30 tick 동안 정상 이동한 2.95m는 계산된 허용 1.7825m를 넘었고 `Character::Apply_LocalMoveSnapshot`이 즉시 authoritative position을 적용했다.

실제 수정 전 header의 격리 native Debug/Release probe에서 빠른 두 클릭 후 30/53/57 tick 정상 이동을 재현했다. current는 각각 visual 0.295m에서 2.95/5.21167/5.605m로 한 번에 바뀌었다. 실제 tick 경과 시간 후보는 모두 첫 재개 frame의 0.295m를 보존하고 RECONCILE로 들어갔다. 1 tick 동안 20m 이동한 실제 teleport는 양쪽 모두 RESET이었다. 증거는 `out/ReleaseCharacterSelect20260924/move_gap_probe-{Debug,Release}.jsonl`, `move-gap-probe-results.json`이다.

`Client/Public/LocalMovePrediction.h`는 Server 허용 이동 거리만 actual forward tick 차이로 계산한다. client freshness 0.35초, extrapolation 0.15초, 큰 visual/server 10m disagreement, 첫 snapshot, 제어 불가 상태, class/world reset은 유지한다. `ApplySnapshot`의 forward counter admission이 먼저 duplicate/backward/half-range tick을 거부하므로 unsigned wrap은 정확한 작은 양수 간격이 된다. uint32 차이를 double로 변환해 계산하며 정수 overflow를 추가하지 않았다.

기존 `Tools/CharacterSelectIsolationHarness/Private/ClientPresentationPrimitiveContractTests.cpp`에 다음 검증을 추가하고 전체 primitive contract를 Debug/Release native 실행하여 둘 다 PASS/exit 0을 확인했다.

- 빠른 두 클릭 후 30/53/57 tick 정상 Server 이동은 RECONCILE이며 첫 재개 pose 유지.
- 같은 검증을 uint32 Server tick wrap을 가로질러 반복.
- half-range/backward tick 및 non-finite position은 IGNORED.
- 1 tick/20m 실제 teleport 및 긴 tick 간격에서도 10m를 넘는 visual disagreement는 RESET.
- 기존 initial snapshot, forced state, class/world reset, stale ACK, move sequence wrap과 timeout tests도 함께 통과.

후속 증거는 `movement-delayed-snapshot-results.json`, `movement_probe-Debug.log`, `movement_probe-Release.log`다. 변경 C++와 test는 기존 UTF-8 BOM 없음/CRLF를 유지했고 scoped `git diff --check`를 통과했다. Product 빌드 반영은 통합 담당의 후속 결과를 따른다.

09-23 Character Select 로그의 1.765/1.890초 main-pump gap과 snapshot coalescing은 이 조건이 가능한 실제 자료지만 입력·pose·ACK 시점이 기록되지 않아 **사용자 더블클릭 현상과 동일 원인이라고 확정하지 않는다.** 현재 조치는 ‘지연 snapshot 시 강제 snap 경로 수정’이다. 사용자는 동일 class로 일반 두 클릭, class 변경 직후 두 클릭, 긴 클릭 유지의 세 경우를 같은 위치에서 비교하면 남은 증상을 구분할 수 있다. 재발 시 시각과 class, 상대 화면에서도 같은 순간이동인지가 필요한 최소 정보다.
