# Level 네비게이션 표시와 1관문 세이튼 생성 위치 결과

## G01. 현재 Level의 게시 네비게이션 표시

F1 `Arena Camera / Player` 바로 아래에 `Show Navigation` 체크박스와
`Reload Navigation`을 연결했다. `CLevelNavigationDebug`는 현재 LevelRegistry의 Area에
대해 Client에 게시된 base `.navgrid/.navpolicy`와 `.navregions`에 나열된 detail을 읽는다.
MapTool의 선택 Area나 미게시 region manifest를 제품 이동 데이터로 대신 사용하지 않는다.
Development의 Map Editor 모드에서는 이 표시를 비활성화하고 기존 MapTool 표시를 사용한다.

표시는 기본 꺼짐이며 F1 창을 닫아도 유지한다. Level이 바뀌면 이전 snapshot과 표시를
초기화한다. Reload는 전체 grid를 임시 상태로 검사한 뒤 교체하고, 실패하면 같은 Area의
이전 snapshot을 보존한다. 성공한 Reload는 region 순서 변경으로 수동 선택이 다른 grid를
가리키지 않도록 Auto로 돌아간다. C++ 두 새 파일은 Client 프로젝트와 기존 Map filter에
각각 한 번 등록했다. 일반 Release 빌드에는 표시 코드가 포함되지 않는다.

초록은 게시 이동 가능, 주황은 원본 바닥이 있으나 막힌 셀, 자홍은 NO_SURFACE,
빨강은 원본 정합 확인이 불가능한 게시 blocked 셀이다. source/paint의 descriptor와 모든
셀의 실제 저장 높이·walkable이 게시 grid와 일치할 때만 막힘 원인을 구분한다.
NO_SURFACE 선의 높이는 바닥 측정값이 아니라 기존 authoring 표시용 추정 높이다.
`Get_RuntimeCellHeight()`를 분리해 이 추정 높이를 게시 데이터 검증에 사용하지 않는다.

Auto는 base와 detail을 함께 표시하고 detail 사각형에 덮인 base 부분만 잘라낸다.
부분 중첩된 거친 셀의 바깥 영역을 함께 제거해 가짜 구멍을 만들지 않는다. 수동 Base는
전체 base를 볼 수 있다. 카메라 주변 반경과 화면 clipping을 적용하고, grid별 반경 최대
70셀·화면 최대 4,000셀로 제한한다. 생략 수와 실제 표시 반경을 UI에 표시한다.
표시는 X-ray이며 실행 중 Server의 동적 blocker·support 변화는 포함하지 않는다.

1관문 중앙은 detail 네 곳 밖이므로 4m base grid를 사용한다. 실제 게시 셀에서 동쪽
`(14,734)`는 높이 34.544m blocked, 남쪽 `(-2,726)`는 39.227m blocked이며 서쪽·북쪽에는
NO_SURFACE가 있다. Client와 Server의 게시 파일은 동일하다. Baker는 설정 Y 범위에서
최상단 교차 삼각형을 고르고 현재 범위는 `[-50,45]`다. 상부 지붕·천막 기하를 포착했을
가능성이 있지만 해당 mesh까지 추적하지 않아 확정 원인으로 기록하지 않는다.
기존 09-13 검증은 중앙 세 좌표에 한정돼 가장자리 정상의 근거가 아니다.
이번 작업에서 navgrid를 재베이크하거나 이동 판정을 변경하지 않았다.
수치 증거는 `out/LevelNavigationDebug20260917/gate1_navigation_inspection.receipt.json`이다.

## G02. 1관문 세이튼 기본 생성 위치와 연출 시작점 분리

G1 Saydon의 일반 생성 위치를 기존 중앙 `[-0.07, 1.32, 737.53]`으로 적용했다.
`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`의 stable ID
`boss.kakulsaydon.g1.saydon` position과 문서 revision `8795→8796`만 변경했다.
yaw `237`, disabled 상태와 다른 배치는 그대로다. 최신 byte hash를 두 번 확인하고
원본 백업 뒤 같은 디렉터리의 임시 파일을 원자 교체했다. 사용자에게 이미 받은 현재 저장본
기준 적용 승인을 사용했으며 실행 중 Client의 draft를 자동 Reload하거나 종료하지 않았다.

원인은 기존 연출 작업에서 일반 boss placement도 시작점 `[6.43, 1.3, 730]`으로 옮긴 뒤,
연출의 BossMotion을 별도로 연결한 상태가 남아 있었기 때문이다. F1 관문 버튼은 Client의
임의 좌표가 아니라 placement ID만 보낸다. Server의 `Handle_SpawnWorldEntity`가
`Find_Placement→Build_WorldEntity→Broadcast_WorldEntitySpawned`를 실행하므로
잘못된 공통 placement가 실제 기본 생성 위치와 spawn 기준 패턴에 함께 사용됐다.

중앙 좌표는 기존 Sequence Playback 계획 G20에서 확정한 원래 세이튼 spawn을 사용했다.
룰렛 데이터 중심 XZ `[-0.319, 737.531]`와는 약 0.249m 차이가 있으며 서로 같은 수치라고
기록하지 않는다. P36/P8 연출 이동은 이 spawn 보정과 별도로 다음 값을 유지한다.

| 항목 | 보존 값 |
|---|---|
| Gameplay P36 / Sequence P8 시작 | `[6.43, 1.3, 730]` |
| 연출 도착 | `[-0.07, 1.3, 737.53]` |
| 걷기 구간 | `0~4997ms` |
| 연출 yaw / spawn reset | `-130.801213684°` / `resetBossToSpawn=false` |
| 통합 Sequence P4 안의 걷기 | `23698~28695ms` |

실제 기존 Python projector의 `expand_pattern_document(P4)`를 실행해 child BossMotion의
시각만 부모 배치 시작만큼 이동하고 절대 좌표는 그대로임을 확인했다. t=0/23697/23698ms는
연출 시작점, t=28695/30375/61662ms는 연출 도착점이다. Gameplay Composition, Sequence
Composition과 Encounter의 전체 SHA가 설치 전후 동일하다. Client preview는 BossMotion이
있으면 placement보다 StartPosition을 우선하고, Server Brain도 Begin_Pattern에서
Apply_BossMotion을 호출하므로 일반 spawn 변경을 연출 경로 변경으로 전파하지 않는다.

게시 navgrid의 중앙 셀은 `[489,629]`, walkable=1, 지면 Y=`1.317625641822815`다.
저장 Y=1.32와의 차이는 약 2.374mm이며, 새 위치는 네 detail region 바깥이므로 base grid를
사용한다. 현재 Server Build_WorldEntity는 walkable XZ를 보존하고 Y만 이 지면 높이로
맞춘다. 원래 시작점은 `[491,627]`, 지면 Y=`1.2990055084228516`였다. 이 값은 게시 binary와
현재 C++ 수식을 대조한 수치이며 native Server 실행 검증으로 기록하지 않는다.

공식 `Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_LUT_MIDNIGHTC_ED`는 exit 0으로
새 boss placement와 base+4개 region 검증을 통과했다. JSON parse와 해당 World 파일의
`git diff --check`도 통과했다. 설치·백업·SHA·수치 정본은
`out/LevelNavigationSaydonSpawn20260917/spawn-repair.json`,
`intro-order-verification.json`, `verification.json`, `navigation-validate.log`에 둔다.

담당 root의 World publisher가 exit 0으로 완료한 뒤 실제 게시 파일을 다시 읽었다.
`Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.worldbootstrap` header revision은 8796이며
G1 Saydon 행의 위치 `[-0.07,1.32,737.53]`, yaw 237, disabled 값이 정본과 일치한다.
Client의 `LV_LUT_MIDNIGHTC_ED.viewer.world.json`도 같은 revision·위치다. 게시 로그는
`out/LevelNavigationDebug20260917/world-publish.log`이고 위 verification의 publication에
두 출력 SHA와 행 번호를 보존했다. 생성물을 직접 편집하지 않았다.

`CGameRoom`은 생성 시 worldbootstrap을 로드한다. 게시된 디스크 파일 검증만으로 이미
실행 중인 Server room의 메모리가 갱신됐다고 판정하지 않으며 이번 기록에서 Server 재시작과
새 생성 위치의 실제 화면은 미확인이다. Client/UI 실행·자동 조작·캡처와 visual PASS는 없다.

## G03. 컴파일·반영 경계

현재 헤더로 `LevelNavigationDebug.cpp`와 `MainApp.cpp`의 Debug 개별 컴파일을 실행해
둘 다 exit 0을 확인했다. 기존 Engine 헤더를 UTF-8로 해석할 때 발생하는 C4828 경고가
있으며 파일 인코딩을 일괄 변환하지 않았다. 새 helper에서 ImGui의 placement new와
Engine Debug `new` 매크로가 충돌하지 않게 ImGui를 먼저 include했다.

변경 World JSON과 Client 프로젝트/XML filter parse, 새 항목 각각 한 번 등록,
MainApp 기존 CRLF·BOM 정책 보존, 새 C++ UTF-8 BOM 없음, 변경 파일 `git diff --check`를
통과했다. 세션 시작 백업과 비교한 MainApp·프로젝트 변경은 이 기능의 추가 줄뿐이다.
컴파일·구조 증거는 `out/LevelNavigationDebug20260917/structural-verification.json`과
각 `*.compile.log`, `*.session.diff`에 있다.

실제 helper, CNavGrid, source/paint parser와 경로 계약을 사용하는 CPU 전용 probe를
검토 후 숨김으로 한 번 실행했다. 45초 제한 안에서 4.014초, exit 0이며 1,844,416개
셀·계약 검사가 통과했다. base+4 detail의 분류, 잘못된 runtime/manifest/policy Reload의
기존 snapshot 보존, source 부재 시 원인 미상 처리, Level 초기화, 근평면 clipping과
부분 중첩 base 보존을 확인했다. 손상 입력은 out 복사본에서만 만들고 원복 SHA도 확인했다.
LevelRegistry mapping만 probe의 명시 stub이며 제품 매핑과 MainApp 호출은 소스 검토 및
개별 컴파일로 확인했다. GPU draw와 UI 호출은 0회다.

`probe-result.json`·`probe-stdout.log`가 CPU 검증 근거다. 종료 stderr에는 연결된 Debug
모듈의 CRT 잔여 할당 보고가 남아 있어 메모리 누수 0건을 검증한 것으로 기록하지 않는다.
이번 probe의 성공 범위는 위 네비게이션 기능 검사이며 전체 제품 실행·메모리 회귀 검증이
아니다. 실제 화면은 여전히 사용자 확인 전이다.

실행 중 Client EXE의 쓰기 handle 검사에서 공유 위반 32를 확인했다. 사용자가
“Client는 계속 사용할게, 코드 반영까지만 해줘”라고 선택했으므로 제품 EXE 링크·교체는
진행하지 않았다. 새 토글은 다음 Client 빌드·실행부터 제공된다. 데이터 교체·World 게시는
이 점유와 무관하게 완료했다. Server 재시작, 실제 관문 생성 및 최종 화면 판정은 사용자가
수행한다. Client와 Server를 자동 종료하거나 UI를 조작하지 않았다.
