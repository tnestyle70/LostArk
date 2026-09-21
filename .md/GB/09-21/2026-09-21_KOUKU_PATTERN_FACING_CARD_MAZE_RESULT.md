# 쿠크 방향·판정·카드미로 통합 결과

## 현재 상태

저장본 병합과 revision 2027의 KoukuSaydon domain publish를 완료했다. `out/KoukuPatternFacingCardMaze20260921/integrate.py`로 134개 stable ID 필드 작업과 BossCatalog의 animationSetId 한 필드를 revision 2026으로 병합하고, P77 종료 위치 복귀를 추가해 revision 2027로 반영했다. 교체 직전 재확인·백업·원자 교체를 수행했다. 추가로 보고된 로딩 화면의 ImGui·도구 자막 잔류도 코드 수정했다. 사용자가 Debug 빌드와 화면 검증을 직접 진행하겠다고 지정했으므로 최종 제품 빌드는 실행하지 않았다.

## 반영 내용

- 큰 세이튼 P86/P87: 위치 복원과 함께 배치 yaw 226.5° 복원. 기존 매 타격 16m/242ms 강제 밀림 및 아레나 이탈 허용 유지. 모든 타격 위치에서 반드시 낙사하는지 추가 조사·확대는 사용자 요청으로 진행하지 않았다.
- P21 레이저: 준비 stage에서 살아 있는 플레이어 조준, 모델 +X와 Server 조준·collider 일치. 기존 여러 빔의 각도는 보존.
- P25 피자: 시작 yaw 306.5°와 0ms 중앙 이동. MAP V1 폭발 13개의 위치·시각·회전에서 피해 영역 생성. 실제 mesh의 local 피해 중심 yaw 149.4°를 사용하고 원래 이펙트 배치는 보존.
- P23 팡파레: 회전하는 네 Sector를 밀림 결과에 연결. 6m/242ms 강제 밀림과 이탈 허용. 겹치는 고정 BOX 판정 비활성화.
- P85 바주카: 실제 두 빔 판정에 강제 밀림 연결, 겹치는 첫 판정 제거. 다른 패턴의 공용 결과는 보존.
- P13 조커 찾기: `대형 세이튼 | 노란시선` 리소스로 원본 V1 spotlight를 재사용. 좌우 눈에 각 하나를 최초 탐색 구간 0~3664ms에 부착. BONE 회전과 기존 duration fit 사용. 원본 Effect 문서 수정 없음.
- P77 카드미로: 별도 WORLD 쿠크 occurrence 하나를 제거하고 기존 Kouku CModel에 같은 골격 donor clip을 추가해 재생. 원래 WORLD 이동과 native root를 기존 bossMotion의 optional keys로 합성. 최종 278개 키는 11949ms까지 원래 궤적을 유지하고 11950ms에 현재 spawn으로 복귀해 바닥 아래 잔류를 방지한다. `animationRootVerticalScale=0`으로 중복 이동 억제. 원본 Resources 수정 없음.
- 사용자 logic112: `CARD_MAZE_STAGE_PLAYERS`와 소멸 Effect21~24의 stable ID 연결. Server가 1~4인 roster를 해당 MAP 위치로 일괄 이동하고, hide 시각을 3245/3426/3695/3930ms로 동기화. 기존 CARD_MAZE_ENTER의 8097ms 입장은 유지.
- 로딩 화면: `MainApp::Render`에서 loading 자체 화면을 그린 뒤 `LEVEL::LOADING`이면 ImGui 프레임을 취소하고 UI 입력 프레임·렌더 프레임을 정상 종료한다. ImGui 및 뒤쪽 도구 자막 출력만 차단하며, loading 화면 자체의 제목·팁·오류 표시는 유지한다. 이번 패턴 변경이 기존 숨김 코드를 제거한 것은 아니며, 현재 코드에서 누락된 최종 제출 경계를 보완했다.

## 검증 증거

- 레이저 실제 CGameRoom 네 방향: 조준 dot=1, 첫 collider 모두 적중. 피자 첫 tick 최종 중앙 위치/yaw 확인. `out/KoukuGate2Aim20260921`.
- 피자·팡파레·바주카 실제 LogicRuntime: 208 checks / 0 failures. 실제 강제 밀림 설정·방향·시간·재피격 보호를 검사했으며 전체 물리 주행 및 화면 승인과 구분. `out/KoukuHitCollider20260921`.
- 카드미로 실제 LogicRuntime Build/Update: 61/61. roster1~4, 목적지 실패의 원자성, 숨김/입장/종료 복구, sampled 및 legacy motion 검사. 새 ABI 기준 14 TU 재컴파일. `out/KoukuCardMazeEntry20260921/validation`.
- CModel 원본 91 clips + donor 1 = 92 유지, 중복 부착 거부. 실제 375 pose·81 weighted bones와 30,106 vertices 대조. 정수 ms curve 최대 오차 5.85mm. 창·draw 없이 native 모델 소비자 검증. `out/KoukuCardMazeEntry20260921/{receipt,model_probe}.json`.
- Client staging 저장·복구 31 checks, staging+motion codec 51 checks PASS. `out/KoukuCardMazeAuthoring20260921`.
- Python 신규 staging/donor/motion 및 기존 blend 검사 17 PASS, 기존 bossMotion/parent/teleport 관련 검사 6 PASS.
- 실제 PowerShell publisher의 staging/motion 블록과 정렬 함수 검사 20 PASS. `out/KoukuPatternFacingCardMaze20260921/check_publisher.ps1`.
- 통합 candidate의 dependency closure 10개 Pattern에 대해 문서·publishable·encounter·presentation projection PASS.
- 종료 복귀 실제 Brain 검사 19/19 PASS: 마지막 active tick 원래 위치 유지, 완료 tick spawn 복귀, 같은 NetEntityId 유지, reset 없는 후속 P15도 정상 위치로 시작. `out/KoukuPatternFacingCardMaze20260921/terminal/probe.log`.
- 첫 publish의 World validator가 새 keys 필드를 거부해 해당 검증기를 보완했다. 실패 transaction의 27개 파일이 모두 백업과 동일함을 hash audit로 확인했다. 최종 재게시에서 product·map·world·gameplay domain 모두 PASS, sourceRevision 2027, 198701ms. `out/KoukuPatternFacingCardMaze20260921/publish-retry.log`.
- 최종 설치된 Gameplay.bootstrap를 새 ABI의 native catalog로 실제 로드하고 staging probe 61/61 재확인. `out/KoukuPatternFacingCardMaze20260921/published-catalog-probe.log`.
- 로딩 수정은 MainApp.cpp의 10줄 추가이며 기존 미커밋 변경·인코딩을 보존했다. 정적 경로 확인과 diff check만 수행했고 사용자 빌드/화면 확인은 아직 수행 전이다.

## 남은 단계

사용자가 빌드한 새 Client/Server로 로딩 화면 및 실제 아레나를 확인한다. 데이터 publish와 native 소비자 검증 및 아래 사용자 빌드 로그의 링크 성공은 확인했다. 실제 눈 이펙트·판정·카드미로 화면은 아직 확인하지 않았다. Client/Server 실행·도구 Reload는 자동 수행하지 않는다.

## 사용자 빌드 후 덮어쓰기 확인

2026-09-21 08:47~08:49 KST 읽기 전용 확인에서 Composition revision 2027과 BossCatalog의 SHA-256이 최종 반영 receipt와 일치했다. product/map/world/gameplay 네 domain의 게시 출력 24개도 최종 publish receipt와 모두 일치했다. 증거는 `out/KoukuPatternFacingCardMaze20260921/user-build-data-audit.json`이다.

로딩 guard는 `MainApp.cpp:3330`에 그대로 있다. 해당 소스 수정 시각 08:45:14 이후 사용자 빌드가 MainApp.obj를 08:46:12에 만들고 Client.log에 Client.exe 링크·런타임 배포 성공을 남겼다(Client.exe 08:46:14). Server.log에도 GameRoom_KoukuAudition.cpp 재컴파일과 Server.exe 링크가 기록됐다(Server.exe 08:46:05). 이전 내용으로 복구된 증거는 없으며, 에이전트는 이번 확인 중 재빌드·재게시·제품 실행을 하지 않았다.
