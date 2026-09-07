# 마리오 변신·고정 진행선 2D 조작·위 방향키 점프 결과

## G8. Shared 불완전한 형식·과거 Mario 선언 오류의 VS 분석 캐시 갱신

### 확인과 조치

사용자의 pasted-text.txt는 현재 디스크에 없는 float XZ 인자 선언을 인용했다.
현재 NetworkManager.h 208/209와 cpp 1389/1408은 모두 MARIO_DIRECTION을 사용한다.
PacketMessages.h 584의 enum, 594/606의 request와 eDirection, 1758의 revision result는
완전정의되어 있다. 실제 Debug CL include 기록도 이 저장소의 같은 헤더를 가리킨다.
Debug x64의 include/C++20/define/프로젝트 매핑에 별도 IntelliSense 불일치는 발견하지 못했다.
Shared 헤더의 UTF-8/제어문자 검사도 정상이었다. 과거 선언 인용은 현재 디스크와 맞지 않지만,
사용자 오류 목록의 출처 열을 직접 보지 않았으므로 캐시 원인 확정·화면 해결 PASS로 기록하지 않는다.

사용자가 Visual Studio를 저장 후 종료했다고 응답한 뒤, devenv/VCPkgsrv/MSBuild 부재와
캐시 파일의 exclusive-open 가능 여부를 확인했다. reparse point와 절대 경로를 확인한 다음
`.vs/Framework/v17/Browse.VC.db`와 `ipch`(18개 캐시 파일)를 아래로 이동했다.

`C:/Users/USER/source/졸업팀폴/LostArk/.vs/Framework/CodexCacheBackups/20260907-161321/`

파일을 삭제하지 않았으므로 복구 가능하다. 원래 캐시 경로 부재와 백업 존재를 확인했다.
`.suo`, DocumentLayout/backup JSON, Solution.VC.db, Client/Server.vcxproj.user의 SHA256은
이동 전후 동일했다. 소스·리소스·맵 데이터·프로젝트 설정을 수정하지 않았다.
오류 표시 비활성화, 공용 헤더 중복 정의, 타입 변경으로 진단을 숨기지 않았다.
VS 다음 로드에서 분석 데이터가 재생성되도록 한 로컬 환경 조치이며 Git에 캐시를 추가하지 않는다.

### 검증과 사용자 확인 경계

- Client project/filter, Shared/Server project XML parse PASS.
- git diff --check exit0. 기존 미커밋 변경을 보존했고 stage/commit/push하지 않았다.
- 최초 sandbox 빌드는 Windows SDK 경로 접근 거부 MSB4184로 시작 단계에서 실패했다.
  권한을 승인받아 동일 정본 Debug Product 명령으로 재시도했다. 이는 C++ 타입 오류와 별개다.
- 정본 Debug Product 빌드 exit0: Engine/Shared/Server/Client 전 단계 PASS.
  `out/BuildPipeline/runs/20260907T071726821Z-debug-product.json`의 skippedBuild=false,
  missingRuntimeInputs=[]를 확인했다. Server.exe 16:14:49, Client.exe 16:17:26 KST.
  첨부의 C++ 타입 오류는 실제 재컴파일에서 재현되지 않았다. 기존 인코딩/셰이더/외부 PDB
  경고는 남아 있다. runtime publisher와 광역 하네스는 실행하지 않았다.
- 빌드 뒤에도 보존 파일 해시가 동일하며 VS/Client/Server 실행 프로세스는 없다.
- VS/UI를 자율 실행·조작하지 않았다. 사용자가 Framework.sln을 Debug/x64로 다시 열어
  분석이 끝난 후 첨부의 불완전한 형식/과거 float 선언 오류가 없어지는지 확인해야 한다.
  캐시 재생성 전·후의 실제 IntelliSense 오류 목록 비교는 아직 사용자 확인 대기다.

참고: [Microsoft IntelliSense 설정의 데이터베이스 재생성](https://learn.microsoft.com/en-us/visualstudio/ide/configure-languages-c-cpp-intellisense?view=visualstudio),
[Microsoft의 iPCH 재생성 진단](https://devblogs.microsoft.com/cppblog/troubleshooting-tips-for-intellisense-slowness/).

## G7. 4마리오 마지막 카드 길의 네비 추가와 툴 높이 저작 수정

### 실제 위치·원인

151333/151956 첨부 화면의 카드를 MapTool placement 385~390 및 최종 구간과 대조했다.
`Mario4_Tigger_7` 착지 `(-1629.37,-11.97,-1430.88)`에서
`Mario4_Tigger_13` 종료 `(-1615.255,-12.8104362,-1450.40991)`까지의 카드 길이다.
WModel geometry에 Loader의 0.01 prescale과 실제 저장 quaternion/scale/position을 적용했다.
그 결과 실제 카드 상면의 셀 누락은 388번 3셀, 389번 50셀, 390번 49셀로 총 102셀이다.
385~387의 기존 통행 셀은 그대로 보존했다.

처음 멈추는 셀 `(175,63)`은 카드 385→386 사이 실제 공중 틈이었다.
첫 틈 약 1.142m, 끝 틈 약 1.110m는 메시 면이 없다. 사용자가 비동기 질문에
`빈 틈에도 보이지 않는 통행 셀을 넣어 걸어서 넘기기`를 명시 승인하여 두 틈의 6셀도 추가했다.
이 6셀은 원본 바닥 복원이나 새 메시가 아닌 프로젝트 통행 연결이다.

후반 카드 중앙의 일부 기존 source는 카드 높이 대신 `12.6133099m` 또는 `-0.190089703m`를 가졌다.
기존 Paint는 이미 resolved인 셀의 picked Y를 무시하므로 Force Walkable만으로 이를 카드 높이로
고칠 수 없었다. 실제 빈 공간은 GPU 렌더 surface 피킹도 실패할 수 있는데, 이전에는 이유를 표시하지 않았다.
Bake Preview가 남아 있으면 Walkability에서도 이전 preview가 live 셀을 가릴 수 있는 경로도 확인했다.
사용자 화면의 당시 Region/preview/input owner 상태 자체는 코드만으로 확정하지 않았다.

### 변경

- 재베이크하지 않았다. 기존 navsource/Bounds/geometry/카메라/Trigger/시퀀스를 수정하지 않았다.
- 새 `Data/Navigation/LV_LUT_MIDNIGHTC_ED.Mario4.navpaint`에 108개 WALKABLE+명시적 높이를 저장했다.
  102셀은 실제 상면 약 -12.857~-12.873m, 첫 틈 3셀은 -12.8104362m, 끝 틈 3셀은 -12.8675m다.
- 기존 `CNavGridPaintDocument::Paint`의 기본 높이 보존 동작을 유지하고
  `replaceResolvedHeight=false` opt-in을 추가했다. 명시 선택된 Force Walkable에서만 기존 높이도 교체한다.
  같은 explicit 높이를 다시 칠하면 no-op, 비유한 높이는 변경 전 거부, Reset은 원래 baked 상태로 복원한다.
- MapTool의 `Force Walkable → Use Picked Height`와 한국어 도움말, 피킹 실패 이유 표시를 추가했다.
  Walkability는 live 셀을 표시하고 Bake Preview 자체는 삭제하지 않고 Bake 모드에 보존한다.
- 새 navpaint는 Client 프로젝트의 기존 96.DataFiles/Navigation None 항목으로 등록했다.
  새 C++ 파일·새 런타임·새 하네스·프로토콜 변경·Resources payload는 없다.

### 검증

- 정식 Navigation publisher Publish 검증·배포 exit0. source 재베이크/생성 도구는 실행하지 않았다.
- Mario4 전체 56,000셀: 정확히 108셀 walkability/높이만 변경, 기존 WALKABLE 셀 변경0.
- 실제 고정 경로 DDA 68셀 모두 WALKABLE, 최대 인접 단차 약 0.047001m. 1m 정책은 유지했다.
- Navigation 50파일 전후 SHA256 비교: Client/Server Mario4 navgrid 두 파일만 내용이 달라졌다.
  양쪽 SHA256은 `2F331346B118D6DFC06918D606A10D2E486568D7C093CAE0DFC60DD9724DFF30`로 동일하다.
- 이전 Mario3 E 구간 4셀 보정과 나머지 영역 Navigation은 보존했다.
- 기존 Server focused 검사: 걷기만으로 160틱 동안 진행/복귀, T13 실제 접촉·movePlayer 실행,
  아레나 `(-3.901,1.31762564,733.617004)` 복귀, Mario stage0, 원래 NORMAL 외형 상태와 HP/class 보존.
  `[Mario4CardRoute] walkOnly=1 ticks=160 exitStarted=1 cleared=1 ... stage=0`, `failures : 0`.
- 기존 M3 E 걷기→하강 회귀도 PASS. Server Debug 컴파일·링크 exit0, Server.exe 15:52:02 KST.
- Client Debug 컴파일·링크·정상 배포 exit0. 한국어 도움말을 기존 컴파일 설정에 맞는 ASCII UTF-8
  바이트 문자열로 저장하고 원문 역변환 일치를 검사한 뒤 최종 재빌드했다. Client.exe는 15:56:34 KST.
- 프로젝트/필터 XML parse PASS, git diff --check exit0, PLAN의 전체 코드 31개와 디스크 불일치0.
  기존 shader/코드 페이지/외부 PDB 경고는 남아 있으며 이번 기능의 빌드 오류는 없다.
  툴의 실제 클릭·Save/Reload 화면 검증은 사용자 확인 대기다.

### 사용자가 확인할 경로와 남은 경계

최신 Client로 Lobby → Test → MapTool → KoukuSaydon → Navigation → Region `Mario4` →
Walkability → Force Walkable에서 `Use Picked Height`를 켜면 이미 다른 층 높이를 가진 셀도
클릭한 카드 표면 높이로 바꿀 수 있다. 겹친 층은 Brush 0으로 시작한다.
미저장된 기존 툴 상태를 새 파일 위에 저장하지 말고 먼저 최신 데이터를 다시 연다.
Save Navigation은 저작 저장이며 제품 반영은 Navigation publisher 후 Server 재시작이다.

아레나 확인은 Server/Client를 재시작하고 Lobby → KoukuSaydon → F1 → 4마리오에서 한다.
192.168.0.4 공유 Server라면 그 PC에도 새 Mario4 navpaint를 반영해 publisher 실행 후 재시작해야 한다.
현재 PC에만 배포했으며 원격 Server 반영과 Client/UI 자율 실행·조작·캡처는 하지 않았다.
카드 playSequence의 모든 회전 프레임에 맞춰 동적 nav를 닫고 여는 기능은 추가하지 않았다.
사용자 요청의 정적 걷기 통과 경로와 기존 복귀 트리거를 검증한 것이며 전체 4마리오 기믹/클리어 보상 구현은 아니다.

## G6. 3마리오 중간 발판 하강 트리거 접근 차단 수정

### 확인된 원인과 변경

사용자 첨부 화면의 중간 발판 문제를 `Mario3_Trigger_8` 착지 → `Mario3_Trigger_10` 진행 구간에서
실제 Server 이동기로 재현했다. 수정 전 정지 좌표는 `(-1916.44, -6.40876, -1686.85)`다.
다음 셀은 WALKABLE이고 collisionBlocked=false였지만 높이가 `-3.847718m`로 기록되어 있었다.
하층 바닥 `-6.408759m`와 약 2.561041m 차이여서 기존 1m 높이 정책이 이동을 거절했다.
초록색 WALKABLE 표시는 통행 플래그이며, 올바른 바닥 높이/다음 셀까지의 연결을 보장하지 않는다.

publish 누락은 아니었다. 수정 전 source+paint와 runtime 79,524셀이 모두 일치했다.
높이 없는 WALKABLE paint는 bake된 상층 높이를 유지했고, publisher의 일반 seam 보정은
이를 하층으로 내리지 않는다. 서버/클라이언트 네비 형식은 XZ당 단일 높이다.

`Data/Navigation/LV_LUT_MIDNIGHTC_ED.Mario3.navpaint`의 `(122,98)`, `(125,100)`,
`(126,101)`, `(127,102)`에만 명시적 높이 `-6.40875912`를 지정했다.
주변 하층 셀 및 기존 Trigger10의 높이를 근거로 사용했다. Trigger/카메라/외형/1m 정책/충돌은 변경하지 않았다.

### 검증 결과

- 정식 Navigation publisher Validate/Publish: exit0.
- 배포 전후 Navigation 50파일 비교: Client/Server의 Mario3 navgrid 두 파일만 내용 변경.
- 79,524셀 수치 비교: WALKABLE 변경0, 높이 변경4셀. 그 외 높이 전부 보존.
- Client/Server Mario3 navgrid SHA256 동일:
  `8FCCC57D92CCE01599C3BFAE4DD9289427E406518D595DF7F65ADA164DCB9B05`.
- E 전체 고정 진행선 정확 DDA 40셀: 모두 WALKABLE, 최대 인접 단차 약 0.00000143m.
- D 상층 진행선 32셀: 수정 전 walkability/높이 전부 동일. 상층 전체 통과를 뜻하지 않는다.
- 기존 Server Debug 최소 컴파일·링크: exit0. 최종 Server.exe는 2026-09-07 15:34:17 KST.
- 기존 `Server.exe --debug-teleport-contract-test`: exit0, failures0.
  E 걷기 62틱 후 T10 OBB 접촉, 실제 published T10 action 발동과 30Hz 하강,
  F층 `(-1907.51, -8.97, -1679.82)` 착지 및 T10 진행선 전환을 확인했다.
- PLAN의 기존 27개 전체 코드와 디스크 일치를 확인했다. 새 테스트 실행 파일/별도 하네스는 추가하지 않았다.
- git diff --check: exit0. 기존 LF/CRLF 변환 경고 외 공백 오류 없음.

이전 G5의 짧은 왕복 검사는 전체 구간의 Navigation 높이 품질까지 검증하지 못했다.
이번에 끝 지점까지 걷는 재현을 보강했다. 다른 M3 A/C/D/F 구간에도 걷기 전용 진단상 정지가 남는다.
A/D는 BLOCKED 셀, C/F는 1m 초과 높이 전이가 기록되었으나 의도된 점프와 별도 데이터 문제를
이번 범위에서 구분·수정하지 않았다. 해당 구간은 회귀 PASS 항목이 아닌 진단 출력으로만 남긴다.
이번 failures0은 E 수정과 기존 focused assertion의 통과이지 3마리오 전체 코스 통과가 아니다.

### 실행 반영과 사용자 확인 대기

현재 PC의 Client/Server Navigation 배포는 완료했다. 이미 실행 중인 프로세스의 메모리는 바뀌지 않으므로
서버와 클라이언트를 재시작한 뒤 Lobby → KoukuSaydon → F1 → 3마리오에서 같은 발판을 확인한다.
이 PC는 팀 LAN client 역할이다. 192.168.0.4 공유 서버를 사용하면 서버 PC에도 수정 정본을 반영하고
Navigation publisher를 실행한 뒤 그 Server를 재시작해야 한다. 원격 서버 반영은 수행하지 않았다.
Client/UI 자율 실행·조작·캡처를 하지 않았으며 사용자 화면 재확인은 대기 상태다.
G6에서는 게임 런타임 C++/JSON/XML/Resources를 바꾸지 않았으므로 Client 재컴파일은 필요하지 않다.
아래 G5 Product/프로토콜 검증은 이전 조작 변경 시점의 기록이다.

## 구현 상태

`codex/mario234-camera-intros` 실제 프레임워크에 반영했다. 이 문서는 이전 Shift 점프와
카메라 RIGHT 기반 v62 이동 결과를 대체한다. Client/UI 실행·조작·캡처는 하지 않았다.
기존 미커밋 카메라·월드·도구·시퀀스 작업을 보존했고 stage/commit/push는 하지 않았다.

- 마리오 1~4의 기존 Intro OBB 진입에서 Server가 iMarioStage를 정하고 기존 CLOWN 외형으로 전환한다.
- ←/→는 Server가 확정한 현재 구간의 고정 진행선을 따라 왕복한다. 카메라 회전/보정과 마우스가 축을 바꾸지 않는다.
- Shared protocol63의 MARIO_DIRECTION(STOP/LEFT/RIGHT)을 사용한다. 이동·Debug 점프 요청은 각 7바이트이며 자유 XZ 입력을 제거했다.
- 기존 WorldBootstrap의 17개 stable entry/exit trigger 연결을 조회한다. 승인된 실제 착지점부터 다음 출구까지의 축과
  고정 RIGHT 부호를 사용한다. 상세 연결표는 대응 PLAN G5가 소유한다. 새 좌표 데이터/카메라 런타임 의존성은 없다.
- 키 해제 시 STOP, 입력이 끊기면 서버 9틱(300ms) 만료로 정지한다. A* 우회와 Mario body 옆미끄러짐을 사용하지 않는다.
- 일반 이동·점프·knockback을 고정 원점/축에 투영해 깊이 방향 이탈을 막는다. 기존 navigation/충돌 판정은 유지한다.
- 기존 저작 movePlayer 완료 시 출처 placement ID로 다음 구간을 선택한다. 일반 Debug 점프마다 원점을 갱신하지 않는다.
  초기 진입 이동 중에는 외형을 바꾸되 실제 착지 후 축을 확정한다. M4 Tigger_2는 기존 축을 유지하고 착지 원점만 갱신한다.
- ↑는 Server가 제안한 기존 건너가기 상호작용을 우선 실행한다. 없으면 최근 좌우 방향, 같은 진행선에서 최대 4m의 Debug 점프다.
  기존 TRIGGER_MOVE의 0.6초/arcHeight 1.5m를 재사용하며 새 점프 스킬·스켈레탈 clip은 추가하지 않았다.
- ↓/Shift 점프/마우스 이동/일반 class 스킬은 Mario에서 차단한다. 일반 아레나 이동과 F6 자유 카메라 Shift 가속은 유지한다.
- 컷신·UI·자유 카메라·공중·사망·잡힘 중 입력은 제출하지 않는다. 같은 Server player 몸체 재바인딩은
  물리 입력 edge/capture gate를 유지해 계속 누른 키·마우스를 새 입력으로 재해석하지 않는다.
- 기존 퇴장/다른 F1 이동/책 컷신 강제 배치/사망·낙하/프로필 변경/월드 이동에서 전용 상태를 정리하고 입장 전 외형을 복원한다.
- F1 Mario Controls (Debug Jump)에 사용법과 점프 결과를 표시한다. 상호작용 안내는 Mario [ Up ], 일반 [ G ]다.

## 자동 검증

| 검사 | 결과 |
|---|---|
| Debug Product Engine/Shared/Server/Client 컴파일·링크·배포 | PASS |
| 기존 NetworkProtocolHarness Debug 빌드 | PASS |
| NetworkProtocolHarness --mario-controls-only | PASS, exit0 |
| NetworkProtocolHarness 전체 | PASS, failures0 |
| Server --debug-teleport-contract-test | PASS, failures0 |
| git diff --check | PASS |
| 기존 저작/런타임 camerashots JSON parse | PASS, 양쪽 revision68·동일 SHA256 |

Product 증거: `out/BuildPipeline/runs/20260907T055429143Z-debug-product.json`.
missingRuntimeInputs는 비어 있다. 최종 물리 파일은 Server.exe 2026-09-07 14:53:29,
Client.exe 14:54:28 KST에 생성되었고 두 파일 존재를 확인했다.
첫 빌드에서 기존 테스트의 SERVER_TRIGGER_MOVE aggregate가 새 source ID 필드와 맞지 않아 C2440이 발생했다.
해당 초기화 한 곳에 빈 source ID를 추가한 후 최종 Product 빌드가 통과했다. 기존 인코딩/외부 도구 경고는 남아 있다.

Server 집중 검사는 실제 17개 binding, Mario 1~4의 30회 왕복 깊이 오차, enum/world 거절,
같은 진행선 점프·knockback, 비행 중 진입 후 착지, 저작 이동 source 보존과 다음 구간 전환,
M4 T2 축 보존, Mario no-slide와 일반 아레나 slide 보존을 확인했다.
기존 Clown/입장·퇴장·스킬 차단/점프 착지·실패 검사를 함께 유지했다.
프로토콜 검사는 7바이트 intent, 유효 enum/STOP 허용 경계, 잘린 payload, 출력 보존, stage 범위를 확인했다.
전체 Server 광역 계약과 Release 빌드는 실행하지 않았다. Release Server는 코드상 Debug 점프를 거절한다.

이번 고정 진행선 변경에서 JSON/XML 형식·데이터·Resources를 수정하지 않았고 publisher 실행도 필요하지 않다.
사용자가 조정한 카메라 저작/런타임 revision68을 보존했다.
SHA256: `125F388862C4214D24F88F5463F9DE092CC178EB62CDD34DBF87384D9DD8C97A`.
수직 슬라이스 전체 코드 27개 파일은 기존 PLAN에 갱신했다. 새 C++ 파일/프로젝트 등록은 없다.

## 사용자 런타임 확인 대기

1. Server와 Client를 모두 protocol63 Debug로 맞춘다. 이전 실행 파일과 혼용할 수 없다.
2. 이 PC의 LAN 역할은 client, 대상은 192.168.0.4:7777이며 세션 시작 probe는 not-listening이었다.
   Server PC에서 같은 변경의 Debug Server를 먼저 실행하고, 이 PC는 Visual Studio Client 프로젝트를 Ctrl+F5로 실행한다.
3. Lobby → KoukuSaydon → F1 → 1/2/3/4마리오 → F1 닫기.
4. 컷신 후 자동 Clown, ←/→ 왕복·release 정지, ↑ 점프를 확인한다. 카메라가 돌아가도 깊이 방향으로 이탈하지 않아야 한다.
5. [ Up ] 안내 지점은 ↑로 기존 층간 이동을 실행한다. 착지 후 다음 층의 고정 진행선으로 움직이는지 확인한다.
6. ↓/Shift/우클릭이 Mario 이동을 만들지 않는지, 퇴장 시 이전 외형과 일반 조작으로 복귀하는지 확인한다.

실제 키 입력·사용자 저장 카메라와의 좌우 체감·전체 코스 통과·점프 애니메이션 느낌은 사용자 확인 대기다.
고정 진행선은 원본의 완전한 이동 로직 복원이라고 주장하지 않는다. 현재 보유 gameplay 연결을 이용한 프로젝트 구현이다.
기존 navigation 구멍/장애물/착지 높이를 강제로 보정하지 않았으며, 4m 내 적합한 착지가 없거나 높이 차이가 1m를 넘으면 거절한다.
