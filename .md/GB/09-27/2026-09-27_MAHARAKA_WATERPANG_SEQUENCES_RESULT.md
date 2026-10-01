# 마하라카 워터팡 Camera·발판·효과음 등록 결과

## 2026-10-01 G07 — 이동한 점프 도착점과 붕괴 통합 검사

`jump1_1`은 [72.2330017, 23.1000004, -986.317017], `jump2_1`은
[77.5360031, 23.2280006, -981.762024]로 저장돼 있었지만 출발 trigger의 movePlayer는
옛 위치를 가리켰다. 사용자 마커·높이·기타 편집은 보존하고 두 이벤트의 XZ만 현재 위치에 맞췄다.
revision371→372. 작업 직전 데이터와 비교해 달라진 것은 revision과 두 이벤트 XZ의 네 값뿐이다.

두 지점은 중앙에서 각각3.456m/3.562m로 붕괴 뒤에도 남는 바닥이다. jump3_1은5.464m라
붕괴 이후에만 런타임 목표를 같은 방위의4.4m로 조정한다. 저작 jump3 좌표는 바꾸지 않는다.
MAHARAKA World Validate/Publish를 실행해 Server bootstrap과 Client viewer에 함께 반영했다.
NPC presentation은 내용 변화가 없다. 원본 WorldSequences Check도 통과했다.

최신 main 통합 전 검사:

- Product Debug `20260930T215910156Z-debug-product.json`, Release
  `20260930T215953822Z-release-product.json` 빌드 통과. 각 Server 검사2TU 변경, Client 재컴파일0.
- Debug `--maharaka-ai-contract-test` exit0. 세 쌍의 실제 게시 marker/action 일치, 각 도착점8방향
  보행, G 입력과 정확한 착지, 카운트다운1회, 붕괴 뒤 세 G 착지/생존 및 기존 경기 검사가 통과했다.
  로그: `out/WaterpangEffects20260930/collapse-jumps-debug.log`.
- 기존 Python EntryContract의 landing/nav와 published hold/camera 두 검사 통과.
  첫 호출의 모듈 탐색 경로 및 Windows CP949 오류는 Tools/MapPipeline 작업 디렉터리와
  `python -X utf8`로 바로잡았다. 테스트나 제품 JSON을 이에 맞춰 변경하지 않았다.

사용자 정정대로 #495는 이미 머지됐다. main `b83d646be` 위 새 브랜치
`codex/waterpang-collapse-landings-20261001`에 통합했다. 겹친 MaharakaAI 검사 파일은
main의 파티/종료/귀환/재입장 검사를 전부 유지하고 붕괴 검사를 별도 상태 보존 구간으로 추가했다.
main의 2m 낙사 높이 판정에 맞춰 검사도 실제 fixed tick 하강을 진행한다. main에 이미 있는
Maharaka 검사 CLI는 중복 추가를 제거했다. 별도 Lobby 120초 미커밋 변경은 recoverable stash에
보관한 뒤 main 변경 위에 그대로 복원했으며 기능 PR에는 제외한다.

통합된 Debug Server `--maharaka-ai-contract-test`: **286 PASS, 0 FAIL, exit0**.
`out/WaterpangEffects20260930/collapse-main-debug.log`에 세 점프·8방향 보행·붕괴 전후 착지,
실제 종료 tick·snapshot·전원 귀환 뒤 이동·G 재입장·다음 AI roster 검사 결과가 있다.
최신 publisher Validate와 기존 Python2검사도 재실행해 통과했다. 저작/viewer는 revision372,
42개 배치로 의미적으로 동일하다. 원본 붕괴는 서로 다른18개 binding/track·5000ms·효과음1개이며
도입 stage와 정확히 같은18개 target을 사용한다. Drive 효과음은 runtime 파일과 같다.

아래 G06의 미게시/미push 기록은 이전 단계 기록이다. Client 육안 검증은 미실행이다.
통합 Product Debug는 Client 공통 셰이더 컴파일 중이며 아직 전체 PASS로 기록하지 않는다.
통합 후 Release Product는 미실행이다. 위 통합 전 빌드 성공과 구분한다.
후속 PR: https://github.com/tnestyle70/LostArk/pull/498 (base main `b83d646be`).
생성 후 GitHub `mergeable=true`, `mergeable_state=clean` 확인. #495를 다시 수정하지 않았다.
진행 중 전체 Client 빌드를 코드·서버 계약 검증 성공이나 최종 실행 준비 완료와 혼동하지 않는다.

## 2026-10-01 G06 — 경기 남은60초 원본 외곽 발판 붕괴

### 실제 적용

원인은 원본 `source.collapse`가 Camera 편집 목록에만 있고 경기 재생 호출자가 없던 것이다.
서버가 이미 복제하는 도입 시작 tick을 기준으로140초에 외곽18개/5초 원본 시퀀스를
재생한다. 도입20초를 제외하면 경기120초 경과, 남은60초다. 중앙 노란 원판, 옆 점프대,
캐논·큰 모코모코와 카메라는 변경하지 않았다. 도입 stage 소유자만 반환한 뒤 같은 stable ID의
붕괴를 획득하여 기존 WorldSequencePlayer로 구동한다. 마지막 자세는 경기 STOP까지 남으며
다음 경기/중단에는 기준 배치로 복구한다. 늦은 입장·맵툴 복귀는 현재 Server 시각으로 Seek한다.
누락/다른 바인딩/재생 실패는 `maharaka.waterpang.collapse` 진단을 남기고 기존 배우 연출을 보존한다.

프로젝트 판정은 붕괴2초 뒤 외곽 반경5~7.5m의 지지를 제거한다. 중앙5m와 낮은 탐험 바닥은
유지하고, 서 있거나 걷는 사람·AI 및 날아오는 사람 모두 기존 Waterpang fall/점프대 복귀를
사용한다. 이는 원본18개 삼각형의 연속 물리 시뮬레이션이나 navgrid 동적 재베이크가 아닌
기존 측정 반경을 쓰는 Server 경기 지지 판정이다. 붕괴 후 G 점프 착지와 AI 재입장은
중앙4.4m로 조정하며 원래 저장한 jump 좌표는 바꾸지 않는다. AI 이동 목표도 중앙으로 좁힌다.

### 검증

- Debug Product PASS: `out/BuildPipeline/runs/20260930T214811879Z-debug-product.json`.
- Release Product PASS: `out/BuildPipeline/runs/20260930T215040723Z-release-product.json`.
- 최초 구현 빌드에서 구성별 Server60TU·Client8TU 재컴파일, 이후 검증 진입점 Main1TU 증분 링크.
  Engine/Shared 컴파일·CSO 변경0. 기존 C4819 및 Release DirectXTK PDB 경고는 남는다.
- Debug `Server.exe --maharaka-ai-contract-test` exit0. 기존 world playback failures0 및 AI 검증,
  남은60초 경계·지지 전환 직전/직후·정지한 플레이어 낙하·점프대 생존 복귀·중앙/낮은 바닥 보존·
  G 점프 목적지·ballistic 착지 금지·경기 STOP/지지 복구 검사 모두 통과.
- 원본 WorldSequences Scope Publish/Check PASS. 초기 Check는 게시본의 CRLF만 달라 실패했고,
  publisher로 LF 정규화 후 통과했다. 정본과 게시본은 줄바꿈을 제외한 문자 내용이 정확히 동일하며
  Git 내용 변경은 없다. 전체 Area나 사용자 Gameplay.world.json은 게시하지 않았다.
- `git diff --check` 통과. Release 집중 실행 검사와 Client 화면 확인은 미실행.
- 첫 추가 Release 링크 시 집중 검사 Server 프로세스가 출력 잠금에 걸려 중단됐으며,
  검사 정상 종료 후 최종 Release 빌드는 통과했다. 사용자 프로세스를 종료하지 않았다.

### 배포와 사용자 확인

실행 파일은 `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/{Debug|Release}/Client.exe`와
`Server/Bin/{Debug|Release}/Server.exe`에 반영했다. Client/UI 자율 실행·화면 캡처는 하지 않았다.
Server와 Client를 같은 구성으로 다시 실행하고 마하라카 아레나에서 경기 타이머01:00을 확인한다.
5초간 원본 분리/하강 → 종료 전까지 붕괴 유지 → 경기 종료 후 발판 복귀가 수동 확인 항목이다.
맵툴의 `Camera → 워터팡 / 원본 바닥 붕괴 (맵 동작 미리보기)`는 같은 원본 트랙을 유지한다.

새 모델/텍스처는 없다. 이번 자동 경기 연출에 필요한 기존 원본 효과음
`Sound/Maharaka/WaterpangSource/scene_maharakap_fallout_foley.wav` (660258 bytes)를
`C:/Users/USER/OneDrive/바탕 화면/CY_Resource/Sound/Maharaka/WaterpangSource/`에 추가하고
live Resources와 동일한 복사본임을 확인했다. 기존 배포 파일을 삭제하거나 대체하지 않았다.

다른 작업의 `Client/Private/Level_Lobby.cpp`와 사용자의
`Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json` 변경은 보존했다. 이번 G06은
로컬 구현·빌드까지이며 commit/push/PR 수정은 하지 않았다.

## 2026-09-28 G05 — 점프 입장·10초 예약·컷신 종료 유지

구현/저장/게시 완료, 사용자 화면 검증 대기. 아래 이전 G01~G04 기록보다 이 절의 현재 상태가 우선한다.

- 큰 모코모코와 중앙 물대포 OBJECT_RESOURCE의 STOP을 HOLD로 바꿨다. 자연 종료는 마지막
  visible pose와 기존 NPC suppression을 유지한다. 명시적 Stop/MapTool 전환/맵 퇴장에서는 반환한다.
- 사용자 출발/도착 6개 위치와 회전, NPC 및 기타 배치를 보존했다. jump1~3 출발점만 enabled,
  requiresInteract, movePlayer로 연결한다. 도착 jump1_1~3_1은 비활성 marker이며 자동 왕복하지 않는다.
- waterpang.arena.start는 무대 위 높이에서 입장을 판정한다. Server가 최초 요청에 300tick(30Hz)
  예약을 만들고 Client는 10초부터 표시한다. 0이 되면 게시된 도입15 camera와 연결 배우/무대/음원을 재생한다.
  재진입/다른 플레이어/Debug Replay는 예약을 재시작하지 않는다. 빈 방에서 예약을 지운다.
- 늦은 입장도 원래 start tick을 받는다. 카메라가 이미 끝났으면 다시 보여주지 않고 World 최종 상태를 적용한다.
- 실제 21개 mesh placement에서 좁은 WaterpangEntry navigation을 굽고 Client/Server에 동일 게시했다.
  3개 도착 지점과 다음 걸음이 20.48m 평지가 아니라 22.4m 부근 무대에 남는 것을 서버 하네스로 확인했다.
  이 영역 밖의 지형, NPC, 재질, 원본 카메라는 이번 단계에서 변경하지 않았다.
- authoring Gameplay revision 274 / WorldSequences revision 6. 원본과 후보/비교 보고서는
  `out/MaharakaWaterpangEntry20260928/{before,candidate,report.json}`에 보존했다.

검증:

- Debug Product Engine/Shared/Server/Client 빌드 통과. 최종 증거는
  `out/BuildPipeline/runs/20260928T062344069Z-debug-product.json`. C4819/DirectXTK PDB 경고는 남아 있다.
- WorldSequence publisher Publish/Check, WorldGameplay MAHARAKA Validate/Publish,
  Navigation MAHARAKA Validate/Publish 및 Navigation ContractTest 통과.
- `test_maharaka_waterpang_entry.py` 3개 테스트 통과: 반복 설치 무변경, 기존 행 보존,
  HOLD/runtime 동등성, jump 목적지, 양쪽 navigation bytes/착지 높이.
- `--world-playback-contract-test`의 새 Waterpang 22검사 통과: 예약/중복/실패 예약 rollback/늦은 입장
  packet/빈 방/착지/진입 및 세 G 점프의 실제 Server motion 완료와 exact 목적지 검사.
  전체 하네스는 별도의 Valtan sequence-ID 초기화 및 legacy Kouku admission 두 검사에서 실패했다.
  해당 두 검사와 관련한 기존 데이터/동작을 이번 요청에서 덮어쓰지 않았으며 전체 PASS로 기록하지 않는다.
- WorldSequence 실제 publisher의 정상/잘못된 object/motion 입력 검증 테스트 통과.
  project/filter XML parse, `git diff --check` 통과. Release 빌드는 이번 검증에 포함하지 않았다.
- Client/UI를 실행하거나 캡처하지 않았다. 물대포 공격·승패·보상·무대 붕괴는 이번 구현 범위가 아니다.

사용자 확인: Server와 Client를 새 빌드로 다시 시작 → Maharaka → MapTool을 닫고 follow camera 상태에서
jump3의 G로 무대 진입 → 10초 문구 → 도입15 → 카메라 종료 후 큰 모코모코/타워 유지.
jump1/2는 사용자가 설치한 통 위 출발점에서 G를 누른다. 접근 경로를 임의로 추가하지 않았다.
MapTool 미리보기는 Camera의 도입15를 끝까지 재생한다. 명시적 Stop은 원위치 복원을 수행한다.

전체 코드와 등록 파일: [G05 코드 부록](2026-09-28_MAHARAKA_WATERPANG_ENTRY_CODE.md).

## 요청과 현재 경계

요청은 영상처럼 워터팡 전체를 구현하고 MapTool Camera에서 편집 가능하게 만드는 것이다.
이번 결과는 그중 원본 도입 카메라 2종, 기존 발판 18개의 동작 4종, 시작·붕괴 효과음까지다.
**전체 이벤트 구현 완료가 아니다.** 원본 배우·파티클·BGM·게임 진행은 아래 미완료 목록을 따른다.

참고 영상: https://www.youtube.com/watch?v=9dolMRcm_JE&t=135s
브라우저에서 영상의 경기 중 물줄기와 이후 외곽 발판 붕괴를 관찰했다. 이것은 구현된 Client의
육안 검증이 아니다. 영상만으로 정확한 시작 버전·모든 서버 이벤트 시간을 확정하지 않았다.

## 원본 연결과 적용

- SCENE03B 원본: ReleasePC/Packages/A89UVJO9YX8XOUPN9JHI29ONJXOX7SC.upk.
- SHA256: 804afbb33121d25b43c2d5f5d9690fb88ab923be873b6d24f9fc9037fb50b929.
- source Matinee variableLinks → InterpGroup → actor.staticmeshcomponent → sourcePlacementId
  → 기존 stable placementId로 18개를 정확히 결합했다. 이름·가까운 위치 조인은 사용하지 않았다.
- 원본 cm/축 변환 뒤 기존 배치의 역회전으로 local offset/quaternion을 만들었다.
  배치 transform·모델·재질·물·조명·NPC는 이번 변경에서 수정하지 않았다.
- source Hermite 곡선을 재샘플해 중간 오차 1mm/0.05도 이내로 검증했다.
  무대 복구 +180/-180 Euler 키는 임의 정규화하지 않았다.

| Camera 목록 | 원본 Matinee/Data (1-based) | 길이 | 실제 연결 |
|---|---|---:|---|
| 워터팡 / 원본 도입 카메라 15 (카메라·효과음) | 43/158 | 9507ms | 2 camera cuts, 기본 자세 18개, 효과음 172ms |
| 워터팡 / 원본 도입 카메라 20 (카메라·효과음) | 45/160 | 11735ms | 3 camera cuts, 기본 자세 18개, 효과음 403ms |
| 워터팡 / 원본 바닥 흔들림 (맵 동작 미리보기) | 41/156 | 3004ms | 18개 발판 |
| 워터팡 / 원본 바닥 붕괴 (맵 동작 미리보기) | 42/157 | 5000ms | 18개 발판, 효과음 0ms |
| 워터팡 / 원본 바닥 붕괴 상태 (맵 동작 미리보기) | 44/159 | 1000ms | 18개 발판의 무너진 자세 |
| 워터팡 / 원본 바닥 복구 (맵 동작 미리보기) | 46/161 | 2000ms | 18개 발판 |

붕괴 마지막 키는 약 3.644초지만 InterpLength 생략값은 설치본 Engine.u의
Default__InterpData(packageIndex17330).InterpLength=5.0이다. 5초까지 마지막 자세를 유지한다.
원본 ta_grounddestroy는 loop이며 이번 Camera 항목은 1초 상태 미리보기다. 서버 loop 복원이라고
주장하지 않는다. 네 발판 동작은 같은 배치를 소유하므로 서로 독립된 컷신으로 등록했다.
도입 2개 중 어떤 것이 사용자 영상의 시작인지 아직 사용자의 확인이 없다.

## 효과음과 배포 리소스

SOUND_SCENE_OCEAN1/3의 Event → 단일 Play → Sound → media를 원본에서 추적했다.
기존 WorldSequence soundTracks 경로이며 새 더미 모델이나 C++ 런타임을 만들지 않았다.

| Resources 상대 asset ID | 원본 event / media | 재생 길이 |
|---|---|---:|
| Sound/Maharaka/WaterpangSource/scene_maharakap_waterpangstart.wav | 129050398 / 714113137 | 9331ms |
| Sound/Maharaka/WaterpangSource/scene_maharakap_fallout_foley.wav | 360619994 / 427337176 | 7485ms |

물리 위치는 Client/Bin/Resources/Sound/Maharaka/WaterpangSource/이다. 팀 배포 때 두 WAV가
별도로 필요하다. 원본 WEM payload와 후보를 대조했고 WAV digest도 검증했다.
Wwise bus gain/전체 믹스 복원 완료는 아니다. 붕괴 효과음은 영상 길이 5초를 넘어 자연 tail로
재생되며 명시적 Stop에서는 종료된다. 저속 프레임의 MapTool 시간 보정(100ms cap)과 오디오
실시간 시계 차이 때문에 10FPS 미만 환경의 동기화는 별도 개선·검증이 필요하다.

## 저장·게시 경로

- Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/의 .worldsequences.json과 .camerashots.json이 정본이다.
- MapCatalog에 sourceSequences/sequences/sourceCameraShots/cameraShots를 연결했다.
- Client.vcxproj와 filters의 96.DataFiles None 등록을 추가했다.
- WorldSequence 6 templates / 6 instances, Camera 6 cutscenes / 5 shots, 현재 revision3.
- Publish-MapAuthoring.ps1로 Client/Bin/DataFiles/Map에 게시했다. 생성물 직접 편집 없음.
- Maharaka Level_Development에 자동 시퀀스 재생은 연결하지 않았다. enabled는 입장 시 자동
  실행 보장이 아니다. Camera에서 선택하여 Play하는 편집 프리뷰다.

## 실행한 검증

| 검사 | 결과 |
|---|---|
| test_maharaka_waterpang_sequences.py | 8 tests PASS |
| Publish-MapAuthoring Area Validate | PASS, 4671 placements / 7 files |
| Publish-MapAuthoring Area Publish | PASS |
| Publish-MapAuthoring Area Check | PASS |
| Foley installer dry-run | files=0, 현재 저장본과 후보 일치 |
| JSON/XML parse 및 diff whitespace | PASS, authoring/runtime JSON 2종 byte 일치; 신규 파일 whitespace도 검사 |
| C++ build | 미실행: 이번 변경은 Python·문서·데이터·WAV이며 C++ 변경 없음 |
| Client/UI 실행·화면 캡처 | 미실행, 사용자 전용 |
| 실제 화면·소리·영상 일치 | 사용자 확인 대기, visual PASS 아님 |

추가 광역 검사 test_world_sequence_authoring_contract.py는 42개 중 39개 통과·3개 실패했다.
실패는 MapTool 선언/정의 텍스트 검사, CardMiro prototype 등록의 이전 코드 literal 검사,
walkable reflected-object 기대값 검사다. 관련 기존 C++/해당 검사 파일은 이번 작업에서
수정하지 않았다. 따라서 광역 regression PASS라고 보고하지 않는다.

## 미완료 / 후속 작업

1. 도입 씬의 배우 애니메이션·물 튀김·blur·fade·material parameter 등 원본 비카메라 트랙.
   원본은 source-registration-report.json의 unconsumed 트랙 목록으로 남겼다.
2. 워터캐논·모코코의 action4225601 배우 애니메이션과 얼굴/물줄기/바닥/마무리 파티클.
   source notification 시점은 각각 0.591559 / 2.292587 / 2.390214 / 5.071869초다.
   소켓 FX_01과 root 효과를 구별해야 하며, 효과 경로 이름만으로 연결 완료로 처리하면 안 된다.
3. 회전·공격 AI, 소환/위치 전환, 경기 시작·생존·종료 순서. TriggerMapData57011과
   원본 AI action 연결을 Server 권위 이벤트와 presentation 경계로 구현해야 한다.
4. 붕괴에 따른 서버 support/collision/navigation/낙사. 현재 동작은 시각적 프리뷰뿐이다.
5. BGM start/skipend의 MusicSwitch(type13) 및 Pause/Stop/Resume 상태 전환.
   기존 Sound resolver가 0 media를 반환하는 것은 원본 삭제 근거가 아니다.
6. MapTool은 MAP_PLACEMENT/DEPLOY_PLACEMENT/OBJECT_RESOURCE를 대상으로 한다.
   현재 서버 NPC를 로컬로 복제·이동시켜 제품 이벤트인 것처럼 만들지 않는다.
   NPC 애니메이션 편집 프리뷰와 서버 실행의 소유 경계를 확인한 뒤 기존 typed 경로를 확장한다.

## 사용자가 직접 확인할 경로

마하라카 입장 → F1 → Open Map Tool → Camera → 컷신 목록 → 위 워터팡 항목 → Play.
한 항목씩 확인하고 Stop한 뒤 다음 항목을 선택한다. 끝에 도달하면 편집기는 마지막 자세를
유지하며, Stop이 기존 배치 자세와 카메라를 복구한다. 동작만 있는 항목에는 카메라가 없다.
시간 이동·재생·Stop·Save·재로드를 확인하고, 도입15/20 중 영상에 맞는 버전을 결정해야 한다.
현재 확인 시 Client/Server 프로세스는 꺼져 있었다. 에이전트가 시작하지 않았다.

## 재현 도구

- Tools/MapPipeline/build_maharaka_waterpang_sequences.py: 원본 곡선·정확 배치 결합.
- Tools/SoundPipeline/audit_maharaka_waterpang_audio.py: 원본 Event/media 근거.
- Tools/MapPipeline/install_maharaka_waterpang_foley.py: G01 후보 소유권 검사 후 G02 원자적 반영.
- Tools/MapPipeline/test_maharaka_waterpang_sequences.py: 정상/ID 오류/키 제한/편집 보존/rollback.

G01 최초 후보와 before 백업은 out/MaharakaWaterpang20260927/에 있다. installer는 이 고정
후보가 있어야 실행된다. 사용자 편집 후 무조건 재생성하는 도구가 아니며 충돌 시 거부한다.
source-registration-report.json은 초기 G01의 판독 기록이므로 붕괴 길이는 본 결과의
Engine CDO 기반 5000ms와 최종 authoring을 우선한다. commit/push는 하지 않았다.

## G03 — 2026-09-28 사용자 동영상 대조: 저장·게시·Debug 빌드 완료

> 아래 G03 카메라 retarget은 이후 G04에서 철회했다. 현재 상태는 문서 끝 G04를 따른다.

사용자가 제공한 원본 12.633초/379프레임과 현재 11.033초/331프레임 영상을 읽어 비교했다.
원본의 큰 모코모코 얼굴 close-up 구간에서 현재 영상은 바닥·야자수 쪽을 향한다.
동영상 녹화 시작 시점이 다르므로 파일 경과 시각을 동일한 scene 시각이라고 간주하지 않았다.

### 확인한 원인과 수정

- 설치본 Matinee43 / InterpData158의 원본 키 자체가 현재 큰 모코모코를 향하지 않는다.
  외부 게임 상태의 rebase 여부는 미확정이다. 좌표 변환 버그가 확정됐다고 보고하지 않는다.
  원본 importer와 before JSON을 보존하고, 도입15만 영상 기준 카메라로 조정했다.
  실제 NPC actor100 위치 기준 얼굴 오프셋 [0,1.5,1]m, heading -45도, close-up 위치 보정은
  PROJECT_VIDEO_RETARGET이다. source-exact camera나 사용자 visual PASS가 아니다.
- 원본 smile group205 / track286 / import -70 mn_ismp_00.mat.mn_ismp_00-2_mi가 빠져 있었다.
  opacity_intensity를 6501ms=0 / 6602ms=1 / 9502ms=0으로 materialTracks에 연결했다.
  skeletal 웃음 clip을 찾았다는 뜻이 아니라 실제 원본 얼굴 재질 전환을 연결한 것이다.
- 큰 모코모코와 중앙 작은 모코모코를 기존 WorldSequence OBJECT_RESOURCE로 preview한다.
  source NPC의 기존 모델·전체 native 재질을 사용하고 smile 슬롯만 instance 상수로 바꾼다.
  작은 모코모코는 설치된 att_battle_2_01(1초)과 att_battle_2_02를 8101/9101ms에 배치했다.
  이 두 clip의 타이밍은 영상 기반 편집이며 원본 Matinee 배우 binding이라고 주장하지 않는다.
- binding의 previewNpcPlacementId가 정확한 기존 NPC를 조회하고 clone이 보일 때만 render를
  억제한다. hide/Stop/seek/실패 정리/풀 반환에서 해제한다. Server 위치·AI·네트워크는 불변이다.
  callback 없는 제품 실행 경로는 실패한다. 마하라카 MapTool 편집 프리뷰 지원이다.

### 자동 검증

- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: Engine/Shared/Server/Client PASS.
  최초 Client OBJ194 재빌드 뒤 최종 STOP 제약 추가분 OBJ1 재빌드도 PASS.
  최종 receipt: out/BuildPipeline/runs/20260928T042025631Z-debug-product.json.
  Shader 재컴파일 0, 기존 C4819 경고 있음. Client/UI를 실행하지 않았다.
- 후보에 정본 publisher의 실제 Read-WorldSequenceDocument / Read-CameraShotDocument와
  Assert-WorldSequencePlacementTargets를 적용해 PASS. 원본 NPC placement/model identity 포함.
- 동일 validator에 없는 placement, 다른 NPC 모델 연결, HOLD motion 입력을 주어 세 사례 모두
  예상 오류로 거부됨을 확인했다. 검사는 out 후보만 사용했으며 저작·게시 파일을 바꾸지 않았다.
- close-up 완전 가중치 구간 21개 샘플에서 실제 얼굴 기준점에 대한 시선 오차 < 1e-8m.
  이는 시선 수치 검사이며 피사체 가림·프레이밍의 육안 검증이 아니다.
- 기존 WorldSequence 행 전부 보존, 도입15 외 cutscene 보존, 무관한 Save 변경 병합,
  revision 증가, 같은 카메라 key 동시 편집 거부 검사 PASS.
- git diff --check PASS. 독립 코드 리뷰에서 모델 import 크기·회전, clip 선택,
  material sampler, NPC suppression 수명을 실제 소비자와 대조했다.

### 반영 상태와 남은 단계

사용자가 맵툴 확인을 위해 저장·게시를 명시 승인하여 authoring/runtime JSON을 반영했다.
후보·원본 백업·검사 기록은 out/MaharakaCameraVideo20260928/에 있다.
retarget_maharaka_waterpang_intro.py --apply-reviewed로 최신 저장본의 stable ID/필드 병합,
적용 직전 백업, CAS 원자적 교체를 완료했다. CameraShots/WorldSequences 각각 정본 publisher의
Publish(구조 검증 포함)와 Check가 PASS했다. 두 문서 revision은 3에서 4로 증가했다.

마지막 실제 Camera Play 호출 경로 재점검에서 Build_CutsceneTargets가 별도 TARGET_SET을
만들면서 새 previewNpc callback을 전달하지 않는 누락을 발견했다. Runtime_AuthoringTargets의
NPC 조회만 이 경로로 전달하도록 수정했다. draft map/deploy 소유권은 유지한다.
이 누락 때문에 이전 단계의 빌드·독립 리뷰만으로 실제 Play 연결 완료를 주장할 수 없었다.
수정 후 Product Debug 빌드 PASS: Client MapTool_Cutscenes.cpp OBJ1·EXE1, CSO0.
최종 receipt는 out/BuildPipeline/runs/20260928T044746410Z-debug-product.json이다.
기존 include의 C4828 인코딩 경고는 남아 있다. Client/Server는 에이전트가 실행하지 않았다.

사용자 확인 경로: 새 Debug Client → 마하라카 → F1 → Open Map Tool → Camera →
`워터팡 / 도입 15 · 영상 맞춤 카메라·표정` → Play. 6.6~8.1초 얼굴, 8.101초 중앙 상승,
Stop 후 NPC 복귀, 뒤로 seek, Save/Reload 보존을 사용자가 확인한다.
intro20, 기존 발판 흔들림/붕괴/복구, Foley, NPC gameplay 좌표와 rendering 옵션은 바꾸지 않았다.
새 Drive 리소스는 없다. 웃음 음성 추가·물 분사 action4225601·경기 AI를 완료한 변경이 아니다.
commit/push는 수행하지 않았다.

## G04 — 원본 카메라 복귀 / 경기 도입에서만 타워·모코모코 이동

사용자가 평상시 위치와 경기 중 위치를 구분하도록 정정했다. 57011 DeployData의 actor22,
NPC570941, model MN_ISMP_00 연결을 확인했다. runtime 좌표계의 원본 XZ는
[81.8360400390625, -991.514375], source yaw는 225.87890625도다.
57009 평상시 NPC actor100을 기준으로 카메라를 돌린 G03 판단은 철회했다.

### 저장·게시한 변경

- 도입15 두 cameraTrack을 G03 이전 원본 키와 정확히 같게 복구했다.
- 도입15 시작부터 큰 모코모코 preview를 경기 XZ/방향으로 옮긴다.
- 현재 SCENE04A export612 / MAP_65096D72C5C9_ITR_02453_SK 타워를 같은 rigid group으로 옮긴다.
  새 MAP_PLACEMENT instance는 baseline-relative offset/rotation을 사용한다.
- 기존 큰 모코모코 표정, 작은 모코모코 상승, 효과음과 도입20/발판 시퀀스는 보존했다.
- 저작 CameraShots/WorldSequences revision4→5를 CAS로 저장하고 정본 publisher로 각각 Publish했다.
  Gameplay.world.json과 mapplacements는 수정하지 않았다. 경기 외 평상시 배치는 그대로다.

### 원본과 보정의 구분

NPC 원본 rootY는 20.5628662109375m이나 현재 head/support 피벗·접촉과 호환된다고 확인되지 않았다.
원본 카메라는 고정하고 설치된 smile 면의 중심을 close-up 수직 중심에 맞추는
PROJECT_FRAME_CONTACT_ADAPTER를 적용했다. 결과 headY=24.3577745710577m,
standY=19.063083920198324m이며 기존 머리/받침 높이차 5.294690650859376m를 유지했다.
따라서 전체 transform이 원본과 동일하다고 주장하지 않는다.
인접 Prop570987의 실제 배치도 읽었지만 모델 테이블 연결은 미확정이다.
이 Prop의 원본 모델을 찾았다는 뜻이 아니라 사용자 요청대로 기존 확인된 타워를 함께 이동했다.

### 확인과 사용 범위

후보 publisher 파싱·placement/model identity 검사와 기존 negative3사례가 통과했다.
다른 행 보존, 원본 cameraTrack 일치, baseline-relative 타워 좌표 역산, 원본 파일 freshness를 검사했다.
Stop_Instance의 baseline 복원 및 Release_Objects의 NPC render suppression 해제 경로를 확인했다.
새 C++ 수정은 없으며 G03 최종 Debug EXE를 그대로 사용한다. 새 Drive 리소스도 없다.

MapTool → Camera(통합 컷신 편집 OFF) → `워터팡 / 도입 15 · 원본 카메라·경기 배치` → Play.
Stop은 프리뷰 타워/NPC를 평상시로 복귀시킨다. 끝 프레임은 Stop 전까지 편집기에서 유지한다.
실제 화면·가림·재생/Stop의 최종 확인은 사용자에게 남아 있다. Client/Server는 실행하지 않았다.
전체 서버 경기 시작/종료 상태머신을 구현한 변경이 아니며 현재 범위는 경기 도입 MapTool 프리뷰다.
백업·후보·원본/보정 근거는 out/MaharakaMatchLayout20260928/에 보존했다. commit/push 없음.

최종 CameraShots/WorldSequences Check 모두 PASS. 저작/게시 파일은 각각 byte 동일, revision5다.
git diff --check PASS. 7504ms 원본 카메라 기준 smile 중심의 (right,up,forward)는
(1.0552674, 약0, 12.7772485)m다. 이는 수치 진단이며 화면 가시성 PASS가 아니다.
