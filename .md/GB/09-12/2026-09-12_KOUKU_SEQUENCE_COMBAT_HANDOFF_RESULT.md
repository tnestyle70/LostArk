# 쿠크 입장 시퀀스와 서버 전투 연결 결과

최신 설치·게시 상태는 G08을 따른다. G00~G07의 후보·빌드 진행·이전 작업 세션 기록은 당시 상태다.

## G00. 반영 범위와 현재 상태

입장 시퀀스 선택, 종료 후 Server Gate 승인, 열린 전투 아레나의 Pattern Flow 연결을 소스에 반영했다. 기존 실행 중인 Client/Server와 런타임 데이터는 교체하지 않았다. 새 시퀀스·SceneProfile·카메라 데이터는 `out/KoukuSourceSequenceRestore20260912/candidate/Data`에서 실제 reader로 검사했다. 전체 제품 링크·publisher 배포·사용자 화면 확인은 다음 설치 단계다.

## G01. 입장 연출 선택

`KOUKU_SAYDON_COMPOSITION_PATTERN::bEnterCombatOnFinish` / JSON `enterCombatOnFinish`는 optional bool이며 기본 false다. true는 GATE1/GATE2/GATE3에만 허용한다. 잘못된 타입은 기존 문서 격리 경로로 해당 Pattern의 JSON을 보존한다.

Complete Play는 선택한 관문에서 true인 유효 Pattern 정확히 하나를 요구한다. 해당 입장의 Play Sequence도 0ms부터 같은 admission과 transport를 사용한다. 기존 popup/finale와 클리어·카드미로가 자동으로 함께 재생되는 순회를 제거했다. 신규 후보는 p4 `1관문_통합_시퀀스`, p3 `2관문_진입`, p7 `3관문_진입`만 전투 진입 metadata를 갖는다.

## G02. 열린 아레나와 동일 보스

MainApp Prepare는 저장된 Pattern Flow·arena·world generation·revision을 검증한 뒤 기존 보스 despawn command를 제출하고 cinematic 입력/HUD 대기를 시작한다. Finish는 기존 `Debug_ActivateGate`를 사용하며 Level이 보스 생성 결과와 플레이어 이동 승인을 모두 확인할 때까지 기존 KakulFade_Screen 암전을 유지한다. 성공 후 follow camera·HUD를 복귀하고 실제 재로드된 Flow revision까지 확인한 뒤 서버 Pattern Flow를 제출한다.

F1 `Get_DebugGates`는 HEAD와 전체 일치한다. G1/G3의 플레이어 위치는 기존 (-2.45, 1.32, 945.17)이며 SL05 열린 전투 아레나와 기존 관문별 stable boss placement를 사용한다. 따라서 전투 HP·archetype·scale은 기존 F1 경로와 같은 정본을 소비한다. SL04의 책 연출 위치를 플레이어의 전투 시작점으로 직접 쓰지 않는다. 연출 중 모델은 presentation 프록시이며 공격 가능한 서버 보스라고 기록하지 않는다.

## G03. 실패·중단·늦은 응답

WORLD Play/Seek 실패를 정상 completion과 분리했고, 최초 sample 실패도 admission에서 거부한다. Workbench Stop, 공유 shell Stop, 연출 종료 후 서버 대기 중 Stop은 같은 Complete 취소 소비자로 연결한다. camera/fade/input/HUD 및 audition target 대기를 정리한다. 실패 상태를 completion으로 변환해 전투를 시작하지 않는다.

Level은 gate 대기 15초 timeout과 명시 취소 때 PlayerController의 pending 이동 sequence를 retire한다. 뒤늦게 받은 응답은 해당 취소를 되돌리지 않는다. 이미 서버에 제출된 이동·생성의 rollback을 의미하지 않는다.

기존 spawn 응답은 request ID가 없어 NetworkManager가 placement별 ordered request token을 관리한다. 취소한 A와 같은 placement의 재시도 B가 겹쳐도 A 응답은 A token으로 소비돼 B 승인으로 오인되지 않는다. world reset은 기존 Server의 old-room 처리/동기 Leave → 다음 Tick ENTER_ACCEPTED → reliable FIFO 경계에서 pending 목록을 비운다. 이 순서가 바뀌면 wire request token을 추가해야 한다. 상세 근거와 7개 CPU 사례는 `2026-09-12_GATE_REQUEST_RETIREMENT_REVIEW_RESULT.md`에 있다.

## G04. 실행한 검증

| 검사 | 결과 및 근거 |
|---|---|
| 실제 Debug 개별 컴파일 | CompositionDocument, Workbench, Level, Level WorldObjects, MainApp, PlayerController, NetworkWorldEntityCommandSink, NetworkManager 통과. `out/KoukuSequenceCombat20260912/compile.log` |
| 실제 Sequence reader와 Workbench 계약 | out에 빌드한 기존 ValtanPatternAuditionServiceHarness의 `--kouku-sequence-document-contract` 통과. 신규 후보 7개 Pattern admission, bool 왕복/잘못된 타입 격리, 입장 선택·0ms 시작·pause·실패·중단·중복 입장 거부, 원자 저장/CAS/Action 보존. `sequence-contract.log` |
| 실제 카메라 reader/cache | 1,311,326 bytes / 83 shots 읽기와 1,000회 Ensure의 추가 I/O 0회, malformed reload·dirty 원본 보존 통과. `out/SequencerOpen20260912/camera_full_probe/cache_probe.run.log` |
| Preview 실패·shared Stop·Flow revision | 개별 컴파일과 실제 함수 CPU 검사 통과. `2026-09-12_KOUKU_SEQUENCE_PREVIEW_FAILURE_RESULT.md` |
| 취소·timeout·동일 placement 재시도 | Shared 실제 codec을 포함한 7개 CPU 사례 통과. `out/GateRequestRetirement20260912/run.log` |

검사 중 신규 데이터의 잘못된 World/SceneProfile ID, G2 target placement, 원본 0도/180도 FOV의 런타임 특이점을 발견해 generator에서 교정했다. 제품 reader의 identity 검사를 완화하지 않았다. full product 링크나 새 화면 실행을 이 검사로 대신하지 않는다.

## G05. 설치 후 사용자 확인

새 Client와 호환하는 Sequence/World/Camera/Effect 후보, 고친 리소스, 해당 domain publisher 출력을 함께 설치해야 한다. 기존 Client는 새 metadata/ALT V material 계약을 알지 못하므로 현재 저작 후보도 설치를 보류했다. 설치 전 사용자의 추가 저작 변경과 baseline을 다시 비교한다.

사용자가 새 exe에서 F1의 기존 G1/G3 즉시 테스트, Sequencer의 `1관문_통합_시퀀스` 또는 Complete Play, 중간 Stop과 종료 후 열린 아레나의 보스 HUD·follow camera·패턴 시작을 직접 확인해야 한다. 현재 화면 PASS는 기존 exe의 Sequencer frame drop 해소와 기존 popup/finale 두 연출에 한해 사용자가 확인한 상태다.

## G06. 설치 진행과 이전 Client 작업 세션

12:47:47에 종료된 표준 Client/Server를 확인한 뒤 frozen source 후보 22개를 설치했다. 설치 후보 SHA 및 기존 Action/animation binding/runtime 보호 SHA가 모두 일치했다. `out/KoukuSourceSequenceRestore20260912/authoring_install_receipt.json`에 기록했다. 최종 source로 기존 out harness를 다시 빌드하고 실제 설치된 Data를 대상으로 Sequence 계약 검사를 통과했다(`harness-build-final.log`, `sequence-contract-final.log`).

정본 Debug Product 빌드는 `out/KoukuSequenceCombat20260912/product-build.log`에서 진행 중이다. Engine/Shared/Server 단계는 통과했고 Client가 남아 있다. 실행 파일 완료 상태는 이 중간 기록으로 판정하지 않는다. 빌드 성공 뒤 Map Area Publish/Check, 기존 모델 6개와 ALT V 문서 2개 설치가 남아 있다.

빌드 중 사용자의 이전 exe 재실행 요청에 따라 `out/PreviousAnimationSession20260912`에 이전 Client와 호환 Server, DataFiles, 물리 Data 복사본을 준비했다. Server CMD와 Client를 명시 요청 범위에서 실행했고 listener/프로세스 응답만 확인했다. UI 조작과 화면 캡처는 하지 않았다. 사용자 애니메이션 저장은 분리 복사본에 남으며 종료 뒤 파일별 CAS/필요한 세 방향 병합으로 반영해야 한다. 실행 설정과 원본 보존/병합 경계는 해당 `SESSION.md`, `data_snapshot_manifest.json`에 있다. 이 세션은 기존 Resources를 공유하므로 모델 6개 교체를 아직 진행하지 않았다.

클립 앞/뒤 trim, Source In/Out, 선택 구간 LOOP 및 인접 Blend 수정과 검증은 `2026-09-12_KOUKU_ANIMATION_CLIP_RANGE_RESULT.md`를 따른다. 원본 연출의 GlobalSlomo, 일부 미해결 재질, 실제 파티원 교체/착지 표현은 아직 미완료다.

## G07 사용자와 확정한 같은 서버 보스 구조 및 최신 완료 경계

사용자는 Complete Play에서 책 펼침과 함께 실제 서버 권위 세이튼을 사용하고, 같은 NetEntityId/HP/모델을 연출 종료 후 전투까지 유지하는 구조를 확정했다. 종료 때 서버가 플레이어를 열린 아레나 시작 위치에 배치하고 플레이어 카메라/HUD를 복구한 뒤 저장 Pattern Flow를 시작한다. F1 1관문/3관문은 기존 열린 전투 아레나의 즉시 보스/패턴 테스트를 유지한다.

현재 구현은 Prepare에서 기존 보스를 despawn하고 Finish에서 새 서버 보스와 플레이어 이동을 요청하므로 이 최신 목표와 다르다. 같은 개체 구조의 C++ 및 Product 데이터는 아직 반영하지 않았으며 PLAN을 이 목표와 실제 조사 근거로 갱신했다. 이전 구조의 Debug Product 빌드는 성공했고 out/BuildPipeline/runs/20260912T042510321Z-debug-product.json이 근거다. Client.exe 13:25:09/55,181,312bytes, Server.exe 12:54:25/13,252,608bytes를 확인했으나 이 빌드에 최신 같은 개체 구조가 포함된 것은 아니다.

동일 rig 클립 추가는 기존 CModel::Attach_AnimationSet으로 가능하고, 현재 Server Kouku Pattern은 기존 entity와 HP를 유지한다. 같은 clock으로 Npc/Camera/World/Effect가 표현되는 경로도 존재한다. 서버 연출 역할과 피해/입력 보호, 원본 position/yaw key track, 실제 COMPLETED 소비 및 플레이어 이동 승인 뒤 Flow 시작, prototype의 연출 AnimSet admission은 남아 있다. 조사 파일은 out/KoukuSourceSequenceRestore20260912/live_boss_animation_admission_review.json, out/KoukuActualBossSequenceReview20260912/source_boss_review.json이다.

사용자 분리 작업 사본의 세이튼_1관문연출(p35)에 native 10종/13개 구간/41,488ms 편집 DRAFT를 연결한 내용은 ANIMATION_CLIP_RANGE_RESULT G06을 따른다. 원본 A/B 혼합과 runtime handoff를 구현한 결과가 아니다. 사용자가 편집 중인 이전 Client/Server는 종료하거나 조작하지 않았고 기존 공유 Resources 6개 및 ALT V 2개 후보 설치는 보류 상태다.

기존 kouku-client heartbeat는 이전 구조의 완료를 잘못 알리지 않도록 잠시 PAUSED한 뒤 최신 목표/빌드 성공/사용자 DRAFT 및 후속 작업으로 갱신해 ACTIVE로 복구했다. 의미 있는 새 완료/실패/사용자 조치만 알리고 동일 상태는 조용히 유지한다. 새 구조의 소스/빌드/데이터와 사용자 최종 화면 판정은 각각 완료 여부를 구분한다.

## G08 실제 Play 거부 해소와 이전 저작 복구

사용자 종료 확인 뒤 기존 후보6모델과 ALT V2문서를 실제 설치했다. 인형 child rotation,
외곽불D/E/F 재질, CardEruption14초, 차원술사 중심 큐브 및 튜닝 문서가 대상이다.
`out/KoukuSequenceCombat20260912/resource_install_receipt.json`에 원본·후보 SHA와 설치 결과를
남겼다. Table의 잘못된80byte bounds tail도 별도 수정·설치했다. 실제 설치된 Resources로
7개 시퀀스의 CModel/Clone146개·clip29개·WORLD binding453개와 source/runtime 문서 검사를
통과했다. 상세 근거는 SOURCE_SEQUENCE_RESTORE_RESULT G06-3이다.

이전 EXE에서 저장한 P33/P34 마리오 애니메이션 편집은 공통 기준본과 비교해 본 Data로 합쳤다.
사본의 P35 입장 연출과 현재 P35 쇼타임은 별개 신규 항목이므로, 현재 쇼타임을 보존하고 입장
연출을 P36으로 넣었다. 재배정은 pattern/action/animation occurrence27개 ID만 변경했으며
clip·시간·Source In·loop와 나머지 원본 값은 보존했다. `out/PreviousAnimationSessionMerge20260912/`
의 merge_report/install_receipt에 근거가 있다.

WORLD/Table 외에 Complete Prepare가 source350/published345 불일치로도 거부되는 것을
확인했다. 기존 publisher의 unavailable 격리는 초안을 보존하면서 유효 Pattern을 게시하므로
사용자 타임라인을 임의로 줄이지 않았다. 저장 Flow가 없는 G3에는 정상 P18 외곽불·갈고리
시각테스트 한 행을 기본값으로 연결했고 G1/G2 Flow는 그대로 보존했다. 최종 source351을
명시한 KoukuSaydon owner 게시로 Product·Map·World·Balance를 함께 갱신한다. 게시 완료는
`out/KoukuSequenceAdmission20260912/owner-publish.log`의 네 domain 성공으로 판정한다.

Debug Product 빌드와 실행 폴더 배포는 성공했다. 최초403,367ms, 뒤따른 C++ 파일 하나의
동시 변경까지 반영한 추가 빌드 후 무변경 재빌드는3,367ms이며 EXE/DLL/CSO127개가
hash/mtime 모두 유지됐다. 셰이더 최적화 상세와 수치의 조건은09-09 성능 RESULT G09를 따른다.
현재 kouku-client 예약 작업은 사용자의 중지 요청에 따라 PAUSED 상태이며 새 예약을 만들지 않았다.

입장 중부터 같은 서버 NetEntityId/HP를 유지하는 G07 목표는 여전히 미구현이다. 현재 경로는
연출 presentation 뒤 서버 보스 전투를 시작한다. 이번 설치·컴파일·준비 검사를 실제 Client에서의
Complete Play, 카메라 전환, 전투 종료와 원본 전체 visual fidelity 성공으로 대신 기록하지 않는다.
