# 쿠크 카드미로 진입 배우 재사용과 플레이어 배치 결과

## G00. 완료 상태

source 구현과 revision 2025 기준 데이터 후보를 완료했다. 이 문서는 live authoring 설치나 domain publish 완료를 의미하지 않는다. 최종 최신 저장본 병합과 게시 결과는 root 통합 기록에서 구분한다. Client/UI 실행과 제품 링크는 하지 않았다.

## G01. 기존 쿠크 재사용

`Client/Private/KoukuSaydonPresentationAssetService.cpp`는 BossCatalog.animationSetId가 bodyModel과 다른 경우 기존 body에 CModel::Attach_AnimationSet을 적용한다. bone count/hash와 중복 clip 검사 후 prototype을 commit하며 기존 body/재질/91개 clip은 보존한다. G2 쿠크만 원본 카드미로 donor를 연결하는 후보를 준비했다. Engine 경로나 Resources는 추가하지 않았다.

P77의 WORLD1(world26)만 제거하고 같은 11.95초 `kouku.gate2.maze.kouku`를 실제 NPC AnimationOccurrence로 재생한다. 망치와 나팔 WORLD는 유지하고 기존 boss socket resolver가 같은 NPC를 따른다. 별도 Sequence P6는 변경하지 않았다.

## G02. 원본 이동과 root 중복 방지

원본 WORLD76키 중 7410→7590ms 구간은 상단 navigation 경계를 벗어나므로 일반 root 이동은 퇴장 동선을 클램프한다. 기존 absolute BossMotion을 optional Keys로 확장했다. Keys는 strict time order와 endpoint 일치, 2..512개, finite XYZ를 검사하며 이전 linear 문서는 고정 Y와 기존 보간을 유지한다. GameRoom admission은 Keys의 최초 지면을 검증하고 명시된 컷씬 퇴장 구간의 nav 검사를 생략한다.

P77 BossMotion277키에는 원래 WORLD 위치와 `(native b_root - rest) × ancestor basis × model preScale × actor yaw`를 합성했다. P77 animationRootVerticalScale=0과 기존 수평 root suppression으로 animation의 XYZ를 다시 더하지 않는다. native donor와 모델은 그대로 유지한다. source/world yaw는 -134.9999994236도다.

정수 ms 키 계약으로 원본 30Hz fractional root 급변점을 근사하므로 전체 곡선 최대 오차는 5.848mm다. source 월드 pose, 카메라, 이펙트 시간은 보존했으며 실제 화면 판정은 사용자 확인으로 남는다.

## G03. Server 권위 플레이어 이동과 소멸

새 CARD_MAZE_STAGE_PLAYERS는 같은 Pattern의 MAP Effect stable ID 1..4개를 참조한다. projector가 해당 XYZ를 Product playerEntryPositions로 내리고 PATTERNCARDMAZESTAGING row가 기존 GameplayCatalog를 통해 소비된다. GameplayCatalog/Brain은 owner, slot order, 1..4개 좌표와 finite 범위를 확인한다. Client codec/UI/projector/publisher 연결은 root/다른 agent의 같은 변경이다.

LogicRuntime은 현재 alive/combat-ready roster를 기존 순서(X 내림차순, 동일 X이면 PlayerId)로 한 번 고정한다. 모든 목적지 exact nav/지면 높이와 참여 상태 및 collision을 확인한 뒤 player 복사본을 모두 commit한다. 실패하면 위치를 보존하며 고정 roster를 비워 이후 hide/enter가 재캡처하지 않는다. 성공하면 기존 HIDE_NEXT와 CARD_MAZE_ENTRY_HIDDEN 복제·render 경로를 쓴다. Discard/입장 실패의 reveal은 유지한다.

사용자 logic112의 ID/이름과 occurrence7은 유지한다. P77 presentation21~24 MAP XYZ가 이동 목적지이며 HIDE occurrence2~5 시작을 3245/3426/3695/3930ms로 맞췄다. 실제 maze transfer는 기존8097ms를 유지한다. source/world authoring을 직접 교체하지 않았다.

## G04. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| 현재 ServerNavigation의 FX21~24 목적지 | 4/4 exact+sample 성공, authored Y와 ground Y 차이0 |
| 실제 GameplayCatalog/LogicRuntime/Brain/GameRoom_Helpers Debug TU scratch compile | 4/4 PASS |
| 실제 KoukuSaydonPresentationAssetService Debug TU scratch compile | PASS |
| fresh 14 TU Server native staging probe | 61/61 PASS: 1~4인, 순서, hide/entry, 13가지 transactional 거부, 재캡처 금지, Discard reveal, XYZ keys와 legacy linear |
| 실제 Engine CModel WARP probe | 기존91clip+donor1=92, 중복Attach거부 후92유지,103bone rig 일치 |
| 실제 CModel pose 비교 | 375시각 ×81 weighted bones, world matrix 최대 차이5.728mm; window0/draw0 |
| source pose/vertex 및 곡선 계산 | 30,106 vertices,12,325 curve samples, 전체 skinned curve 오차 상한5.848mm |
| Python 현재 Composition validator | candidate 구조 PASS |
| 기존7파일 인코딩/BOM/CRLF 및 before snapshot delta | 보존, git diff --check PASS |

증거는 `out/KoukuCardMazeEntry20260921/`의 `receipt.json`, `model_probe.json`, `navigation.json`, `boss-navigation.json`, `validation/staging_run.log`, compile logs에 있다. 전체 문서 후보는 `candidate/Data/`이며 root가 `field-patch.json`을 stable ID/field 단위로 최신 저장본에 병합한다. `input-guards.json`은 WORLD template/instance/resource, 모델 SHA, BossCatalog geometry/scale와 FX 위치/시각의 derivation 입력을 고정한다.

## G05. 남은 경계

live source 설치, owner publish, 새 코드가 포함된 Client/Server 제품 빌드와 사용자 화면 확인은 별개다. 기존 실행 프로세스의 prototype이나 미저장 draft를 이 작업에서 Reload/폐기하지 않았다. 데이터 후보 전체를 그대로 덮어쓰지 않는다.

## G06. World publisher 호환 수정

최종 게시에서 World publisher의 기존 bossMotion exact-fields 검사가 keys를 거부한 원인을 수정했다. Tools/WorldPipeline/Publish-WorldGameplay.ps1은 Gameplay publisher와 같은 optional keys 2..512개, finite·범위·시간순서·양 끝점 일치를 검증한다. keys 없는 기존 문서는 고정 Y 제한을 유지한다. 이 publisher는 mechanicTriggers 내부 종류/필드 검증을 Gameplay publisher에 위임하므로 CARD_MAZE_STAGE_PLAYERS용 별도 중복 계약은 추가하지 않았다.

PowerShell Parser 오류 0, 실제 -Mode Validate -WorldId KAKULSAYDON_ARENA 성공(114 placements, 7 spawn groups), git diff --check를 확인했다. 파일의 기존 BOM/CRLF를 보존했고 변경 전 백업과 validate.log는 out/KoukuCardMazeEntry20260921/world-publisher에 있다. 이 Validate 실행은 runtime output을 쓰지 않으며 최종 재게시·rollback 상태는 root 통합 결과에서 기록한다.

Server GameRoom_KoukuAudition.cpp의 bundle bOwnsPlayerMode 판정에도 CARD_MAZE_STAGE_PLAYERS를 추가해 publisher와 같은 단일 player-mode owner 방어를 유지했다. 기존 파일 바이트/BOM/CRLF와 무관한 diff를 보존했고 git diff --check를 통과했다. 별도 컴파일은 실행하지 않았으며 root의 전체 Product 빌드가 확인한다.

## G07. 컷씬 종료 뒤 기존 쿠크 위치 복귀

최종 통합에서 P77 종료 뒤 기존 NPC가 원본 컷씬의 퇴장 높이 Y6.4077에 남는 후조건을 확인했다. Brain Finish_Pattern과 audition Clear는 좌표를 복원하지 않으며 후속 P15의 resetBossToSpawn=false도 그 위치를 이어받는다.

root는 source revision2026에 대한 stable-field patch, 현재 boss placement 위치 guard, SHA 재확인, 백업과 원자적 교체를 거쳐 revision2027을 반영했다. P77 bossMotion.keys는277→278개가 됐으며11949ms까지 기존 곡선은24,138시각 비교에서 차이0이었다. 카메라가 끝나는11950ms에 현재 spawn `[6.36000013,10.5600004,321.290009]`으로 복귀하고 endPosition을 동기화한다. stage, camera, animation, logic 및 P15는 바꾸지 않았다.

새 probe main과 기존 fresh14 TU 객체로 실제 Catalog/Brain 경로를 실행해19/19 PASS했다. baseline 침하를 재현하고, 마지막 active tick11933.333ms의 기존 pose 보존,11966.667ms의 Apply_BossMotion→Finish_Pattern에서 spawn 복귀, 동일 NetEntityId, 정상 완료 receipt, P15 resetfalse 상태의 후속 시작 위치까지 확인했다. 증거는 `out/KoukuPatternFacingCardMaze20260921/terminal/{probe.cpp,probe.log}`, `terminal.field-patch.json`, `terminal.guard.json`, `terminal.receipt.json`이다.

이 항목 작성 시 root의 owner publish는 진행 중이다. 최종 게시와 제품 빌드 완료 여부는 root 통합 결과를 따른다. 이 보정은 자연 종료 후 복귀이며 컷씬 도중 수동 중단의 좌표 정책을 새로 추가하지 않는다.
