# 쿠크 체력·패턴 게이지와 PR 통합 결과

## 구현 상태

기준선은 main의 #465·#467, #466의 8543589d4, #468의 5fc2f940a, #469의 5d0ca1f98을 포함한 482cfd75c다. 병합 중 SourceCharacter material program 충돌과 탈것 speed 계약을 보존한 과정은 `2026-09-27_MAIN_PR465_PR467_INTEGRATION_RESULT.md`에 있다. 기존 이펙트 위치·방향과 Data/Rendering 설정은 보존했다.

### 주사위 Complete Play 회귀

실제 두 Client의 실행 로그는 저작·게시 revision 2443과 sequence 182 일치를 보였다. 이전 게시본을 읽어서 생긴 문제가 아니었다. Server가 속박 플레이어의 카드 피격을 허용하도록 `isPatternBound=true`, `isCombatReady=true`를 전송했지만 Shared의 상태 검증은 이를 거절했다. 그 결과 속박 시작 tick 2498부터 방 전체 snapshot encode가 실패하고 두 Client는 tick 2497에서 멎었다. 자유 플레이어의 이동과 속박 효과가 함께 멈춘 원인이다.

속박 입력 차단과 피격 가능 상태를 분리했다. 실제 Complete Play READY→intro→P79→P78 전환, 두 session snapshot decode, 자유 플레이어의 4 tick navigation/collision 이동을 검증했다. 2명과 2명+GUIDE 모두 인간 1명만 속박되고 자유 1명만 실제 이동하며 양쪽 수신 tick은 현재 tick 2502와 일치했다. 인간 1~4명의 N-1 속박, 같은 문양 0 피해·즉시 해제, 다른 문양 최대 HP 90% 피해 검증도 통과했다. 고정 바닥 경계의 presentation에는 이동/충돌/nav 상태 변경 경로가 없었다.

속박 바닥 표시도 별도로 추적했다. ClientReplication이 수락한 각 player snapshot과 실제 Character 참조를 전달하고 `Update_DiceBindVisuals`가 bound=true 및 게시 `diceBindVisual=true`에서만 `effect.kouku.card.match.bind.floor`를 시작한다. 매 프레임 그 플레이어 Transform의 translation-only 발밑 좌표를 따라가며, 자유 플레이어에는 생성하지 않는다. bound=false는 바닥 효과 Stop 뒤 해제 효과를 재생하고 사망/despawn/누락/Reset도 정리한다. 원본 `fx_mn_rpct_05_x.par_x_rpct_spotlight_01_loc_int` 바닥 효과의 visible 11 emitters/15 Resources 참조와 release 9 emitters/15 참조 모두 설치돼 있다. revision 2444에서는 PATTERN_78만 해당 시각화가 활성이다. projector의 주사위 visual admission CPU 검사도 통과했다. 이 증거는 코드·게시 데이터·리소스 연결이며 GPU 화면 표시의 직접 확인은 아니다.

### 체력 UI와 수치

- 서버 snapshot의 살아 있는 몬스터·보스는 빨간 HP, 다른 플레이어는 원본 하늘색 HP, 보호막은 흰색으로 표시한다. 본인 HP와 사망/제거/화면 밖 대상은 숨긴다. UI는 `CCombatHUDViewModel`의 읽기 전용 상태와 약한 actor 참조를 사용한다.
- 상단 주황 바는 활성 occurrence의 typed mechanic gauge만 표시한다. 1관문과 Mario 2페이즈의 기존 STAGGER_WINDOW는 창술사 명목 기본 공격 23,000×5인 115,000을 사용한다. 실제 보스가 잃은 HP로 남은 양을 투영한다.
- 블랙홀은 BINGO_DETONATION 직전 13초 동안 실제 보스 현재/최대 HP를 그대로 표시한다. 별도 무력화 성공·패턴 취소 판정을 추가하지 않았다.
- 카드비 logic 129의 병정 HP는 69,000, 공격 피해는 13,200이다. 피해는 방어력 재감쇠 없이 기존 보호막·무적·받는 피해 효과를 거친다. 미로 병정 profile은 유지했다. 실제 Server collider 피격으로 확인했다.
- 카드비 새 필드는 Client parse/validate/save/workbench→Python/PowerShell publisher→Server catalog/spawn/MonsterBrain까지 연결했다. 0 또는 누락은 기존 profile이고 허용 범위는 0..2,000,000,000이다.
- Shared boss snapshot의 kind U8/current U32/max U32 추가로 protocol은 117이다. Server와 모든 Client를 같은 버전으로 다시 실행해야 한다.

### 열린 PR 보완

#465 초상은 요청마다 full G-buffer viewport와 원래 DSV를 복구하고 field forward hair/eyelash도 HDR에 그린다. alpha-over coverage와 straight RGB resolve를 추가했다. field 재질·광원·exposure/gamma/LUT/FXAA를 재사용하되 기존 초상 SSAO/bloom/screen post 제외 정책은 유지한다. 따라서 전체 필드 후처리와 완전히 같다고 설명하지 않는다.

#467 배 전환은 목적지 검증 성공 뒤 기존 mount action을 종료한다. 거절되면 비행·속도·action을 보존한다. #469 에스더는 쿠크 관문 초기화 commit에서 살아 있는 소환·대기 소환·보호 영역·guard를 함께 종료한다. preflight 실패는 기존 상태를 그대로 보존한다. class 변경도 이전 에스더 guard를 넘기지 않는다.

## 게시와 리소스 전달

공식 Kouku projector→GameplayBalance publisher 순서로 게시했다. revision은 2444이며 114 patterns/577 stages/9 bundles다. 변경은 revision, 두 STAGGER_WINDOW threshold, 카드비의 두 수치뿐이다. source freshness 및 모든 Data/Rendering JSON hash 검사를 통과했다. Client presentation의 구조 변경은 sourceRevision 2443→2444 하나뿐이다.

Gameplay.bootstrap SHA256은 `106f8f660500d1d2e68b27bdc5c82dc21766a57d8ae17069fdccffcbd1fda8b4`다. 게시 로그는 `out/kouku-health-publish.log`, 교체 전 백업은 `out/kouku-health-publish-backup-20260927-185748`이다. 생성 bootstrap을 수동 편집하지 않았다.

원본 하늘색은 설치된 EFUI_STATUS의 `headstatus_i6` atlas에서 (763,53,78,5)를 색·크기·alpha 변경 없이 추출했다. asset ID는 `UI/HeadStatus/HS_Fill_Player.png`이고 중립 tint를 사용한다. `Client/Bin/Resources`와 `C:/Users/user/Desktop/GBResources`의 같은 상대 경로에 설치했다. 출처·픽셀 영역·전달 파일 해시는 `2026-09-27_HEAD_HEALTH_BAR_RESOURCES_RECEIPT.json`에 있다. Resources PNG는 Git에 강제 추가하지 않으며 Drive 업로드는 별개다.

GBResources 전달에는 하늘색 fill, 기존 frame/빨강/흰 shield와 `UI/BossUI/boss_bar_fill_orange.png`까지 다섯 장을 포함했다. 설치본/전달본 SHA256은 모두 일치한다.

사용자가 받은 실제 `C:/Users/user/Downloads/CY_Resources.zip`도 읽기 전용으로 확인했다. 1,869개 파일 모두 현재 설치 Resources와 size/CRC32가 일치한다. 마하라카 map 고유 모델 408/408, map presentation JSON 리소스 527/527, NPC 2/2, Sound 12/12 참조가 충족됐다. 이 팩은 GBResources로 복사하지 않았으며 추출/설치도 에이전트가 수행하지 않았다. 별도의 베른 선박 9종과 항구 NPC 2종은 이 팩과 현재 설치 폴더에 없으므로 해당 별도 팩이 필요하다. 읽기 전용 조사 결과는 `out/KoukuHealthIntegration20260927/cy-resources-audit.json`에 있다.

재질 재번호와 팩의 호환성도 확인했다. NPC 두 WModel의 WMA2에는 재질 이름과 texture path만 있고 숫자 shader program은 저장하지 않는다. NpcCatalog override 6행의 materialName이 실제 모델과 일치하며 stable family `source.character.maharaka-ismp-1.v1`가 현재 CPU registry의 1528로 resolve된다. Guardian 1526과 충돌하지 않는다. NPC texture 참조 14개도 모두 설치돼 있다(팩 10개, 기존 공용 4개). 근거는 `out/KoukuHealthIntegration20260927/cy-material-binding-audit.json`이다.

## 실행한 검증

- Python 카드 수치 계약 10개 통과.
- NetworkProtocolHarness Debug x64 build 및 전체 실행 1,314 PASS/실패 0. protocol 117과 새 gauge 직렬화 왕복 포함.
- ValtanPatternAuditionServiceHarness Debug x64 build 및 `--kouku-fixed-damage-contract` exit 0. 69,000/13,200 save/reopen, 범위 초과·다른 trigger 필드 거절과 기존 draft 보존 포함.
- Server Debug `--kouku-dice-hit-contract-test`, `--card-maze-contract-test`, `--vehicle-riding-contract-test`, `--skill-stages-contract-test`, `--bingo-contract-test`, `--kouku-support-surface-contract-test` 통과. SupportSurface의 supplemental parser는 `LOSTARK_SERVER_DATA_ROOT`를 현재 게시 경로로 명시해 실행했다.
- `--kouku-product-contract-test` 재실행도 failures 0/exit 0으로 통과했다. 실제 게시 generation, Mario 공 완료 후 귀환·착지·후속 패턴, 관문 reset과 FIFO lifecycle을 포함한다.
- Shader_Deferred FXC fx_5_0 /O1 및 실제 compiled PORTRAIT_RESOLVE D3D11 WARP 통과. alpha 0/1/0.25/0.625와 straight RGB 검증이며 실제 게임 화면 검증은 아니다.
- 공식 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -MaxCompilerProcesses 4` 전체 PASS/exit 0. Engine→Shared→Server→Client 컴파일·링크와 SDK/shader/runtime 배포를 완료했다. 필수 runtime 입력 누락/invalid 목록 모두 비어 있다. 최종 증거는 `out/BuildPipeline/runs/20260927T102446888Z-debug-product.json`이다.
- 변경 JSON 6개 및 프로젝트/filter XML 2개 parse, 두 UI layout의 참조 이미지 존재, 전달 UI 이미지 5개 일치, `git diff --check` 통과. Data/Rendering의 기능 변경 diff는 없다.

검증 로그는 `out/KoukuHealthIntegration20260927`, `out/kouku-health-*-contract.log`, `out/protocol-pr-integration-debug-contract-final.log`에 있다. 빌드·진단 산출물은 Git 제외다.

초기 통합 검사에서 현재 계약과 달라 실패하던 fixture도 교정했다. protocol 116/옛 packet 길이, 새 threshold보다 작은 Mario fixture HP의 unsigned subtraction, geometry 없는 synthetic Bingo board, Inanna 보호 전에 이미 회복된 HP의 잘못된 기댓값, 공 3개 완료 없이 Mario 귀환 요청, 관문 초기화 후 Esther를 보존하던 옛 기댓값이다. 해당 변경은 실제 게시 geometry와 필요한 입력 상태를 준비하며 정상 경로와 실패 보존 검사를 유지한다. 제품 검사를 통과시키려고 판정을 완화하지 않았다.

## 수동 확인과 실행 경계

Client/UI를 에이전트가 실행·조작하거나 화면을 캡처하지 않았다. 실제 속박 바닥 이펙트, 머리 위 HP/보호막 위치, 여러 초상의 머리카락·속눈썹 최종 화면은 사용자 확인 대상이다. 설치 데이터와 실행 중 Server/Client 메모리는 구분한다. 새 EXE와 게시 데이터가 있어도 이미 실행 중인 이전 process가 자동 갱신되지는 않는다.
