# F1 Sequence Viewer 결과

## 현재 반영 상태

2026-09-07, `codex/mario234-camera-intros`의 현재 작업 트리에 반영했다.
기존 사용자 변경은 되돌리지 않았고 자동 stage/commit/push는 하지 않았다.
코드·데이터 배포·아래 자동 검사는 완료했으며, Client 화면과 네 명 공동 재생 확인은 사용자 확인 대기다.

## G1. F1 목록과 한글 식별

- 모든 **Debug Level**의 전역 F1에 `Sequence Viewer / 시퀀스 뷰어`를 배치했다. 로비·Test·아레나에서 목록을 열 수 있다. Release에 새 F1을 추가한 것은 아니다.
- `KoukuSaydon / 쿠크세이튼`, `Valtan / 발탄` 탭을 분리했다.
- 트리거, 배치된 맵 시퀀스, 기존 제품 보스 패턴을 목록에 모았다. 한글 이름·구역·stable ID·연결 대상·동작·비활성 사유를 표시한다.
- 한글/영문 검색과 종류 필터, Refresh, Play, Replay, Stop, Open Editor, Go To, Enter Arena를 연결했다. 해당 항목에 없는 기능은 비활성화하거나 이유를 안내한다.
- `Data/Maps/SequenceViewer.labels.json`에 주요 트리거의 한글 이름/구역 16개를 등록했다. 표시 이름만 바꾸며 서버에 보내는 ID는 바꾸지 않는다. 나머지 항목은 기존 displayName/원본 ID를 보존한다.
- 기존 ImGui의 Korean glyph font 경로를 사용한다. 새 UI 번역 파일 때문에 별도 폰트나 Resources binary를 추가하지 않았다.
- 목록은 Refresh 시 읽는다. 트리거 또는 시퀀스 문서가 깨졌으면 해당 종류의 이전 목록을 보존하고 오류를 표시한다. 실행 시에는 실제 소비자가 다시 검증한다.

## G2. Test와 제품 아레나 실행

`MainApp_SequenceViewer.cpp`는 선택과 typed 명령을 전달하는 UI이며 두 번째 시퀀스 런타임이 아니다.

- **Lobby > Test**: 현재 MapTool의 `m_ArenaRisePlayer`와 카메라 트랙을 사용해 내 화면에 미리보기한다. 다른 Area 준비는 기존 editor preload 경로를 사용하고, 저장하지 않은 저작 내용이 있으면 Area 전환을 거절한다.
- **쿠크/발탄 아레나**: `IPlayerCommandSink -> NetworkPlayerCommandSink -> NetworkManager -> Shared -> ServerApp room queue -> GameRoom`으로 요청한다. 현재 world/session/player, 활성 ID와 중복 요청을 Server가 검증한다.
- 서버 트리거는 기존 `ServerTriggerSystem::Run_Action` 경로를 재사용한다. 이동·소환·인카운터를 Client local 실행으로 흉내 내지 않는다.
- 맵 시퀀스는 기존 `S2C_WORLD_SEQUENCE_PLAY` 방송을 사용한다. 보스 패턴은 각 보스 도구의 기존 Server Play/Restart 경로를 사용한다.
- 다른 Level에서도 두 목록은 볼 수 있다. 서버 실행은 해당 아레나에 입장해야 하며 `Enter Arena`는 기존 승인된 레벨 전환 경로를 사용한다. 맵 세부 조정은 Test에서 한다.
- `Go To`는 Test에서는 편집 카메라 이동, 아레나에서는 기존 서버 권위 자기 플레이어 debug teleport 요청이다. 이동 시 트리거가 자연 발동할 수 있다. Test에서 컷신 카메라가 재생/편집 중이면 Stop 후 이동하도록 안내한다.
- F1을 닫아도 대기 중인 Area 전환 후 재생과 서버 응답 처리는 MainApp Update에서 계속된다. 서버 응답은 5초, Area 준비는 60초 후 미확인/실패를 표시하며 자동 서버 재전송은 없다.

## G3. Replay / Stop 경계

- 트리거 Replay는 선택 트리거의 진입/G/one-shot 조건을 debug 실행에서만 우회한다. 살아 있는 플레이어, 강제 이동 상태, 실제 소환/이동/인카운터 실행 검증은 유지한다. 실패를 성공 latch로 기록하지 않는다.
- 이동 트리거는 요청자의 **현재 위치**에서 원래 목적지로 실행한다. 원래 출발점에서 보려면 Go To를 먼저 사용한다.
- Stop은 선택 시퀀스의 표현을 정리한다. 전투·피해·소환·다른 플레이어 이동까지 전체 되감기하는 기능이 아니다.
- 쿠크 책 컷신은 기존 묶음 재생 함수를 사용하고 원본 companion 시퀀스/책·보스 소품/아레나 가시성 정리도 함께 처리한다. 종이다리 재생은 기존 bridge/lever 표현 경로를 유지한다.
- 자연 종료되어 마지막 모습이 남은 Deploy 애니메이션의 owner/baseline을 `WorldSequencePlayer::m_Held`에 보존해 후속 Stop/Replay가 정리할 수 있게 했다. 완료 항목을 계속 ticking하거나 Is_Playing=true로 유지하지 않는다.
- 보스 패턴에는 범용 Stop을 제공하지 않는다. 해당 Boss Tool의 기존 실행 제약과 지원 동작을 따른다. 쿠크 보스 Replay도 기존 Play 경로이며 실행 중 패턴의 무조건적인 강제 초기화를 보장하지 않는다.
- Server 승인 표시는 명령 검증/실행 요청의 승인이다. Client 모델·카메라 화면 성공을 확인했다는 뜻은 아니다.
- 같은 아레나의 기존 방송 경로를 연결했지만, 네 PC의 준비 완료 barrier, frame-exact 동기 재생, 늦게 입장한 플레이어의 시퀀스 이력 복원은 새로 구현하지 않았다.

## G4. 데이터·프로젝트·공유 계약

- Shared protocol **66**. 신규 request/result packet을 기존 enum 뒤에 추가하고 world sequence event에 PLAY/REPLAY/STOP을 직렬화했다. Release Server는 이 debug 요청을 DISABLED로 회신한다.
- World publisher의 bootstrap **v9**는 활성 시퀀스 ID allowlist를 포함한다. Server는 기존 v8도 읽지만 allowlist가 없는 배포에서 단독 시퀀스 실행을 허용하지 않는다.
- 현재 쿠크 authoring/runtime 시퀀스는 revision 152, 활성 instance 93개이며 Server allowlist와 일치한다. 발탄은 독립 worldsequence 0개이고 기존 트리거/보스 패턴 목록을 사용한다.
- `Publish-WorldGameplay.ps1 -Mode Publish`로 bootstrap과 조회용 fallback 문서를 갱신했다. 기존 dirty Gameplay authoring을 임의로 고치지 않고 현재 저장된 내용을 소비했다.
- 생성된 조회 파일: `Client/Bin/DataFiles/World/LV_LUT_MIDNIGHTC_ED.viewer.world.json`, `LV_LUT_HEARTRB_ED.viewer.world.json`, `SequenceViewer.labels.json`. 이들은 publisher 생성물이며 직접 편집하거나 원본 대신 관리하지 않는다.
- 신규 `Client/Private/MainApp_SequenceViewer.cpp`를 vcxproj/filter의 `00.MainApp`에 등록했다. 표시 이름 원본은 `96.DataFiles/Maps`의 None 항목으로 등록했다. 신규 소스와 한글 안내를 추가한 MapTool 소스에 UTF-8 컴파일 옵션을 적용했다.
- `AREA_DATA_LAYER_GUIDE.md`, `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`에 public 계약과 사용 범위를 반영했다. 이번 기능에 추가 전달할 Resources asset ID/binary는 없다. 팀 Drive Resource pack 변경은 필요 없다.

## 실행한 자동 검증

| 항목 | 결과 |
|---|---|
| Debug Engine / Shared / Server / Client 컴파일·링크 | 통과. 최초 Product 빌드 후 Client include 순서 문제 수정. 마지막 Client는 ClCompile/Link/PostBuildEvent 최소 타깃 exit 0 |
| NetworkProtocolHarness | `failures : 0`; 새 명령/결과/event roundtrip, 잘린 입력과 잘못된 ID, 기존 protocol 계약 포함 |
| Server `--world-playback-contract-test` | `World playback contract failures: 0`; bootstrap ID, trigger 재생·one-shot·Replay·실패 latch·invalid/dead player 검사 |
| World publisher | Publish 통과; 템플릿/인스턴스 ID·참조·활성값·중복·개수 검사 및 staging/promotion |
| 변경 JSON/XML | labels/생성 조회 문서 JSON, Client project/filter XML parse 통과 |
| 시퀀스 배포 일치 | 쿠크 enabled ID 93개가 Server bootstrap과 정확히 일치, authoring/runtime revision 152 일치 |
| 독립 비평 | 실제 코드로 확인한 완료 소품 owner, 책 묶음 재생, 재재생 소품 가시성, 실패 목록 보존 지적을 반영 |

외부 DirectXTK PDB 누락 LNK4099 등 기존 경고는 남아 있으므로 warning-free 빌드로 보고하지 않는다.

## 사용자 실행 순서와 미검증

1. 공유 서버 PC에서 새 protocol 66 Server와 갱신한 DataFiles/World를 적용하고 재시작한다. 참가자 Client도 같은 변경으로 빌드한다. 이전 Server/Client와 혼용하지 않는다.
2. 이 PC는 LAN 설정상 **client**다. Visual Studio에서 Client 프로젝트를 `Ctrl+F5`로 시작한다. 세션 시작 시 공유 `192.168.0.4:7777` probe는 not-listening이었다. 에이전트는 Client나 지속 실행 Server를 시작하지 않았다.
3. 어느 Debug Level에서든 `F1 > Sequence Viewer > 쿠크/발탄 탭`을 연다. 한글 검색, 종류 필터, 이름·위치·연결 ID를 확인한다.
4. `Lobby > Test`에서는 책/종이다리/마리오 등 등록된 시퀀스를 Play → F1 닫기 → Stop → Replay 순서로 확인한다. Open Editor로 현재 MapTool 선택이 이어지는지 확인한다.
5. 실제 아레나에서는 같은 방 참가자와 재생한다. 한 번 사용된 트리거의 Play 거절/Replay, 잘못된 대상 거절, 이동·소환·보스 패턴의 기존 제약을 확인한다.
6. 화면의 한글 렌더링, 연출·카메라 일치, 소품 원상 복구, 다른 참가자의 수신/재생은 **미검증**이다. 사용자의 실제 관찰 없이는 visual PASS 또는 완전 완료로 기록하지 않는다.

## 종료 확인

- `git diff --check` 통과. 신규 뷰어 CPP/labels JSON/RESULT는 `git diff --no-index --check`에서도 공백 오류 출력 없음(새 파일 차이 자체의 exit 1은 오류 출력과 구분).
- 최종 JSON/XML 재검사와 쿠크 Server allowlist 93개 대조 통과.
- 최종 Client 최소 빌드 `ClCompile;Link;PostBuildEvent` exit 0. 로그: `out/BuildPipeline/f1-sequence-client-minimum-final.log`.
- 마지막 전체 Client 재빌드 도중 사용자가 실행한 별도 Visual Studio 빌드와 FXC tlog가 겹쳐 FTK1011이 발생했다. 사용자 빌드는 건드리지 않고 명령줄로 확인한 에이전트 빌드 tree만 종료했다. 외부 컴파일 종료를 확인한 후 셰이더 재컴파일 없는 최소 C++ 타깃으로 재검증했다. 이를 C++ 코드 오류나 성공한 전체 재빌드로 바꾸어 기록하지 않는다.
- 종료 프로세스 확인에서 Client/Server/CL/Link/FXC 실행 프로세스 없음. 테스트 전용 Server는 결과를 반환하고 종료했으며 공유 서버 listener를 시작하지 않았다.
- 실제 Test/아레나 입력과 한글·카메라 화면, 네 명 공동 관찰은 사용자 확인 대기다.
