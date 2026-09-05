# 2026-09-05 아레나 카메라와 피킹 위치 플레이어 배치 결과

## G00. 실제 구현 상태

작업 브랜치는 `codex/arena-camera-player-placement`다. PR #316의 merge commit `96f907b2`를
기준으로 정리한다. 원본 Desktop 작업 폴더의 브랜치·다른 작업 문서와 실행 중인 프로세스는 보존했다.

- 쿠크 카메라는 기존 맵 span 4661.12805에서 372.890244m/s로 계산되었다. 발탄의 기존 20m/s로 통일했다.
- `CCamera_Free`가 이동 속도의 유한 값과 0.1~400m/s 범위를 검증하고 이동량에만 적용한다.
  F1 `Arena Camera / Player`에서 현재 아레나 속도와 기본값 복원을 제공한다. Shift는 기존 30배다.
  두 Level이 아레나별 process-session 값을 보존한다. 디스크 저장은 추가하지 않았다.
- F6 자유 카메라에서 F1 `Move Player`를 누르면 mouse-look을 끄고 UI 밖의 새 지면 클릭을 한 번 받는다.
  버튼을 누른 클릭은 소비하지 않는다. Esc/우클릭/follow 복귀·capture·카메라 override는 미제출 선택을 취소한다.
- `CPlayerController -> IPlayerCommandSink -> Shared -> Server GameRoom`으로 피킹 좌표 의도를 전달한다.
  Server가 실제 session의 자기 player, world, sequence, 생존/capture, navigation, 높이와 blocker를 검증한다.
  피킹 표면과 Server 바닥의 높이 차이가 1m를 넘으면 거절한다. 실패 시 기존 player/action/path를 유지한다.
- 성공 시 기존 teleport reset을 재사용하고 Server 바닥에 배치한 뒤 일반 snapshot으로 표현한다.
  중복 요청은 이전 결과만 회신한다. Client는 Transform을 직접 수정하지 않으며 승인/거절을 F1에 표시한다.
  5초 응답 지연은 미확정 상태로 표시한다. Release Server는 명시적으로 거절한다.
- ImGui와 제품 UI의 같은 프레임 mouse claim을 모두 검사해 UI 클릭으로 지면을 선택하지 않게 했다.

Shared protocol은 57에서 58로 변경했다. 실제 request/result codec, room command, server consumer,
Client network queue와 typed sink를 연결했다. 새 C++ 파일, project/filter 등록, JSON/schema 변경은 없다.
Stage 자동 접기는 별도 [결과 문서](2026-09-05_COMPOSITION_RESOURCE_STAGE_ACCORDION_RESULT.md)에 기록한다.

## G01. 자동 검증

격리 worktree `C:/Users/user/.codex/worktrees/pr316-conflict-resolution/LostArk`에서 수행했다.

| 검증 | 결과 |
|---|---|
| Shared + Server Debug x64 컴파일·링크 | 성공, 종료 코드 0 |
| Shared + Server Release x64 컴파일·링크 | 성공, 종료 코드 0 |
| 기존 NetworkProtocolHarness `--debug-teleport-only` | 25개 통과 |
| 기존 Server `--debug-teleport-contract-test`, Debug | 쿠크·발탄 20개 통과, failures 0 |
| 같은 Server 검사, Release | 4개 통과, typed disabled 거절·상태 보존 |
| Camera_Free, Level_ValtanArena, Level_KakulSaydonArena, MainApp Debug 최소 컴파일 | 모두 오류 0 |
| 최종 PlayerController Debug 최소 컴파일 | UI mouse claim 수정 포함 오류 0 |
| NetworkManager, NetworkPlayerCommandSink Debug 최소 컴파일 | 모두 오류 0 |
| protocol 버전 기대값을 갱신한 기존 Python 검사 3개 | AST parse 성공; 전체 검사는 실행하지 않음 |
| 변경 JSON/XML parse | 대상 없음 |
| `git diff --check` | 통과 |

Server 검사는 원본 `Server/Bin/DataFiles`를 `LOSTARK_SERVER_DATA_ROOT`로 읽기만 했다.
현재 navigation을 로드하고 잘못된 층·범위·사망·capture·다른 world·stale 요청·NPC 겹침의 거절,
Server 높이 채택·자기 player만 reset·중복 적용 방지를 검사했다. 원본 데이터 publisher는 실행하지 않았다.
Client 컴파일에는 기존 C4819 인코딩 경고가 있었으며 기존 파일 인코딩을 유지했다.

Client 로그는 `out/ArenaCamera/compile/client-*.log`이고 최종 Controller 로그는
`out/ArenaCamera/compile/client-PlayerController-final.log`다. Server/codec/network 로그는 worktree의
상위 폴더 `C:/Users/user/.codex/worktrees/pr316-conflict-resolution/`의
`server-{debug,release}-teleport-{build,contract}.log`, `teleport-codec-build.log`,
`client-teleport-{networkmanager,sink}-build.log`다. 빌드 산출물과 로그는 커밋하지 않는다.

## G02. 수동 검증과 다음 단계

Client 전체 링크·실행과 F1/F6 실제 입력, snapshot 화면·카메라 체감은 아직 확인하지 않았다.
에이전트는 Client/UI를 실행·조작·캡처하지 않았으며 visual PASS를 기록하지 않는다.
현재 Desktop에서 실행 중인 Server/Client는 이번 변경 전 프로그램이다.

1. 새 기능 커밋을 받은 뒤 Server와 Client를 종료하고 같은 protocol 58 소스로 Product Debug 빌드한다.
   기본 명령은 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`다.
2. LAN 동기화 결과에 따라 server-host PC는 `Server + Client`, client PC는 Client를 `Ctrl+F5`로 실행한다.
3. Lobby에서 발탄 또는 쿠크에 진입하고 F6 → F1 `Arena Camera / Player`로 속도 조절을 확인한다.
4. `Move Player` → UI 밖 지면 클릭 → F1 Server 승인/거절과 실제 player 위치를 확인한다.
   UI 위 클릭, Cancel Pick/Esc/우클릭, F6 follow 복귀도 확인한다. Tab은 mouse-look을 다시 켠다.
5. 동일 프로세스에서 아레나 재진입 후 속도 유지와 두 아레나의 독립 설정을 확인한다.

새 Resources asset ID나 물리 폴더, Drive 전달물은 필요하지 않다. 런타임 EXE를 원본에 교체하거나
Resources를 배포하지 않았다. 자동 검증 완료와 사용자의 실제 화면 확인은 별개다.
