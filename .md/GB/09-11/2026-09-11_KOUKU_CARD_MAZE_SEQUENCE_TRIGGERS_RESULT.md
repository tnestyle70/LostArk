# 쿠크 카드미로 Sequence Trigger 연결 결과

## G01. 실제 연결

- `KAKULSAYDON_G1_PATTERN_28.logic.2~5`의 기존 3744/4182/4529/4824ms를 보존했다. 공유 logic43은 `CARD_MAZE_HIDE_NEXT`이고 첫 cue의 살아 있는 전투 준비 참가자를 월드 X 내림차순, 동률 PlayerId 오름차순으로 고정한다. 다음 cue는 고정 순서의 다음 한 명을 숨긴다.
- 마지막 `logic.1`은 5136ms의 `CARD_MAZE_ENTER`다. 목적지 `(0.28, -0.01, 1351.65)`를 Data에 저장하고 서버 navigation, 미로 중앙 범위, collision과 참가자 상태를 검증한 뒤 전원 위치와 MAZE HUD를 같은 tick에 commit한다. 현재 class의 정상 몸체로 다시 표시하고 중앙 Q 망원경 이후 기존 문양·병사·행진·출구 경로를 사용한다.
- cue가 파일에서 진입부터 저장되어 있어도 runtime은 시작 시간으로 정렬한다. 늦게 도착한 tick에서 진입 뒤 hide가 실행되는 순서 역전을 막는다. 목적지나 참가자 상태가 실패하면 위치는 전원 유지하고 숨김을 해제하며 실패 이유를 보존한다. Stop/abort/discard에서도 해당 roster의 숨김을 해제한다.
- Shared protocol79의 `CARD_MAZE_PRESENTATION.flags` bit16이 표시 상태를 전달한다. wire 길이는 그대로지만 이전 peer는 이 flag를 거부하므로 Server/Client를 함께 재빌드한다. ClientReplication이 snapshot을 Character에 전달하고 Character의 composite body/equipment 렌더 큐만 생략한다. 장비별 visibility 값은 보존한다.
- 기존 Logic resource 편집에 두 typed trigger를 추가했다. HIDE는 별도 값 없이 사용하고 ENTER는 중앙 목적지 pos를 편집한다. bundle의 player-state 충돌 검사도 두 타입을 포함한다. 새 C++ 파일이 없어 project/filter 추가가 없다.

## G02. 기존 사용자 dissolve 저작 보존

pattern15 presentation7~15의 기존 dissolve 1/1 때문에 전체 문서 parser가 거부했다. 실제 EffectV2 `Dissolve_Amount`는 `start >= end` 분기로 이미 분모 0 없이 즉시 전환을 처리한다. parser/Workbench/projector와 EffectV2 group validator가 같은 값을 허용하도록 맞췄고 역전된 값은 계속 거부한다. runtime 계산식은 바꾸지 않았다. 해당 9개 occurrence의 fadeOutMs는 0이므로 실제 group에는 -1/-1(저작 envelope 유지)이 전달된다.

## G03. 실행한 자동 검증

- `Server/Default/Server.vcxproj`, Debug x64 Build 성공. Shared 라이브러리와 Server 실행 파일을 빌드했다. 로그: `out/cardmaze-server-build.log`.
- `Server/Bin/Debug/Server.exe --card-maze-contract-test`: failures0. 오른쪽 첫 hide, 이동 후 roster 순서 고정, 늦은 tick에서 나머지 hide→전원 입장, MAZE Q·normal body, navigation 실패 전원 위치 보존과 표시 복귀, Stop 표시 복귀 및 기존 카드미로 검사 통과. 로그: `out/cardmaze-contract.log`.
- NetworkProtocolHarness Debug 빌드와 `--mario-controls-only` exit0. 새 hidden flag serialize/deserialize, unknown flag 거부와 기존 snapshot 계약을 검사했다. 로그: `out/cardmaze-protocol-build.log`, `out/cardmaze-protocol-contract.log`.
- 기존 Python 테스트에 추가한 `test_card_maze_triggers_preserve_authored_times_and_typed_destination`, `test_presentation_equal_dissolve_lifetime_endpoints_remain_valid` 모두 통과. 실제 cardmaze publication candidate를 써서 다른 미완성 패턴과 독립 검증한다.
- 변경 범위 `git diff --check` 통과. 기존 인코딩을 유지했고 Resources payload는 추가하지 않았다.

## G04. 통합 검증 체크포인트

위 Server 테스트 후 실제 bootstrap collision layer를 전달하는 검사와 한 참가자의 진행 중 TriggerMove가 전원 입장을 막는 검사를 추가했다. 이 마지막 테스트 추가와 Client/equality 변경은 root의 최종 Product 빌드·재실행 대상이다.

Composition projector Publish는 한 번 성공했으나 다음 Gameplay publish가 Encounter stale로 실패했다. read-only `prepare_publication`의 최신 source revision327에서 pattern28은 `unavailableReason=""`이고 새 typed trigger 5개를 모두 projection했다. 당시 실제 Encounter의 같은 pattern은 mechanicTriggers가 빈 배열이어서 최신 projection과 차이가 있었다. 마지막 source 편집 후 root가 revision 갱신 및 Composition projector → Gameplay publisher를 직렬 실행하고 최종 실행 파일과 함께 검증한다. 이 문서의 체크포인트 자체는 runtime 배포 완료를 뜻하지 않는다.

전체 fixture 검증에는 별도로 기존 pattern15 presentation14 종료21714ms, presentation15 종료22001ms가 pattern lifetime21501ms를 넘는 오류가 남았다. 사용자 타이밍은 수정하지 않았고 해당 항목은 기존 publisher의 개별 admission/격리 계약을 따른다.

## G05. 사용자 화면 확인

Client/UI 실행·조작·캡처와 visual PASS는 수행하지 않았다. 이 PC는 LAN `server-host`, 방화벽 TCP7777 LocalSubnet 준비 완료이며 세션 시작 probe는 not-listening이었다. 최종 배포 뒤 사용자가 Visual Studio `Server + Client` profile을 Ctrl+F5로 시작한다.

Lobby → KoukuSaydon → 2관문 → Composition Patterns → `쿠크_카드미로연출` → Complete Play에서 앞 네 cue가 오른쪽부터 한 명씩 숨기는지, 5136ms 뒤 전원이 미로 중앙에 나타나는지 확인한다. 혼자이면 첫 cue만 자신의 캐릭터를 숨기고 남은 세 cue는 빈 순서를 소비한다. 중앙에서 Q를 누르면 기존 망원경/문양 진행이 시작된다. 숨김 도중 Stop은 즉시 표시 복귀를 확인한다. 카메라를 회전해도 서버 순서는 월드 X 기준으로 고정된다.
