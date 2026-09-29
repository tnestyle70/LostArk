# 발탄·쿠크 패턴과 Movie 수정 통합 계획

## G00. 반영 범위

2026-09-29 사용자 요청에 따라 발탄·쿠크 패턴 수정, 보류했던 창술사 얼굴 트랙 복구,
현재 작업 폴더의 Movie·마리오·스킬·Release 준비 변경을 함께 검증하고 Debug/Release를
빌드한다. 기능 브랜치에서 PR을 만들고 검증한 변경을 main에 merge한다.
시작 branch는 `codex/kouku-release-sequence-ready`, HEAD는 `d6a9cc224`다.
다른 세션의 마리오 색상·목표 진행도 구현을 보존하며 같은 기능을 다시 구현하지 않는다.

## G01. 발탄과 쿠크의 실제 소비 경로

발탄의 stage 수명, arena center, 고정 공격 방향, 돌 반지름, 실패 전멸, 유령 전환은
기존 Data/Valtan 정본과 Server 소비자를 수정한다. Client는 실제 stage 종료에 맞춰
돌진 이펙트를 정리하고 유령 phase의 실제 사망에만 엔딩을 재생하며 버러지들 자막을 올린다.
상세 계획은 같은 날짜 `VALTAN_HEALTH_ROTATION_AND_ANCHOR`와
`VALTAN_EFFECT_RECEIVER_AND_PATTERN_SYNC` PLAN/RESULT에서 유지한다.

쿠크는 댄스타임의 HUD와 보스 HP 표시를 분리하고, 대형 세이튼 등장 중 캐릭터 표시와
Server 승인 이동 경로를 유지한다. 모든 일반 반복에서 저글링→나팔→슈퍼바주카 순서를
저작 정본에 반영한다. 마리오 충돌 공 피해는 Server에서 1320으로 조정한다.
기존 `KOUKU_STAGGER_ANCHOR_HOOK_LANDING` PLAN/RESULT에 해당 구현을 기록한다.

## G02. Movie 저장과 리소스

창술사는 기존 09-27 `WORLD_MOVIE_HAIR_GUARDIAN_EYES_IMPLEMENTATION` G11 후보의
두 트랙 materialName/family/parameters만 최신 디스크 저장본에 병합한다. Movie 제외 목록,
곡선과 사용자의 Effect Visible 저장은 유지한다. writer lock·hash·백업·원자 교체를 사용한다.
추가 리소스가 필요하면 Resources 상대 경로를 유지해 설치본과
`C:/Users/user/Desktop/GBResources`에 동일한 파일을 두고 hash를 대조한다.

## G03. 게시·통합·검증

각 담당자가 정본 변경과 집중 검증을 완료하면 root가 최신 입력을 다시 읽고 해당 domain의
공식 projector/publisher로 runtime 문서를 생성한다. 변경 JSON/XML parse와 diff를 확인하고,
실제 Server 계약 및 Network protocol 검사를 필요한 범위에서 실행한다. 기존 rendering
option과 사용자 numeric tuning을 임의 복원하지 않는다.

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`와 `Release`의
Product 빌드로 Engine/Shared/Server/Client를 순서대로 컴파일·배포한다. UI는 실행하지 않고
사용자 화면 판정과 자동 검사 결과를 구분한다. 원격 main과 차이를 검토해 통합하고,
빌드 산출물·out·개인 규칙·임시 백업은 제외한 source와 게시 데이터를 커밋한다.
PR 본문에 최종 동작과 실제 검증을 기록하고 PR을 연결한 뒤 merge 상태를 확인한다.

## G04. 완료 증거

RESULT에 각 요청의 반영 파일, 집중 검사 결과, publisher 결과, Debug/Release 증거,
리소스 대조, PR URL과 merge commit을 기록한다. 실패한 검사를 성공으로 기록하지 않으며
Client의 최종 화면과 다인 플레이는 사용자 확인 경계로 남긴다.

## G05. Release 4인 전 패턴 재감사와 마리오 커튼

2026-09-29 추가 요청으로 현재 저장된 HIGH_JUMP 경고 XZ1.4와 맞춰 LAND의
기본/개별 contact 반경을12.25m로 맞춘다. 상세는 같은 날짜 발탄 health/rotation
계획 G22에 둔다. 사자후 STEP10의100m 광역과 돌 cover·폭발은 실제 Server 소비와
대조하고 유효한 판정을 중복 추가하지 않는다. 기존 쿠크 Dance HUD/보스 HP 분리와
대형 등장 player visibility/typed picking 이동도 실제 소비자까지 감사한다.

마리오 참가자 커튼 차단은 기존 조건이 source LogicOccurrences의
MARIO_PHASE2_PLAYERS를 찾지만 Product reader가 해당 logic을 읽지 않는 누락이다.
project_kouku_saydon_composition.py에서 해당 semantic을 가진 정확한 curtain
occurrence에 optional suppressLocalMario=true를 투영하고 Product reader가
타입·resource를 검증해 읽는다. local snapshot의 iMarioStage1~4인 Client만
기존 Sample의 suppression/Stop_Group 경계로 처리한다. 댄스의 같은 leaf와
바깥 파티원, Preview는 보존한다. 새 C++ 파일은 없고 기존 header 소비자를
Debug/Release Product Build로 확인한다. 사용자 HUD 편집과 무관한 데이터는 보존한다.

## G06. 마리오 2·3·4 비행 공의 소비와 생성 간격

Shared KoukuMarioBombContract의 생성 간격4000→8000ms를 Server 판정과 Client
동일 시계 표현에 함께 적용한다. 생성기는 Mario2 세 개, Mario3 세 개, Mario4 한 개이며
각 marker seed 기반 phase offset과 이동속도3m/s, 높이·반경·피해1320은 보존한다.
Server 피격 소비는 stable world sequence instance와 정확한 birth generation의
기존 STOP 메시지를 전달하고 Client는 해당 공만 종료·이전 피격 Effect를 한 번 재생한다.
늦은 STOP이 새 generation을 지우거나 같은 snapshot에서 공을 다시 만들지 않도록
세대별 소비를 보존한다. 다른 공·다음 generation 및 공격용 색깔 목표 공은 변경하지 않는다.
기존 Kouku overlap 계약과 Client 소비 집중 검사, Debug/Release Product Build로 확인한다.

최종 빌드 직전 사용자가 Debug에서 검토 중이므로 Release만 먼저 검증하도록 지시했다.
추가 수정의 최종 컴파일·링크·배포는 Release로 완료하고, 실행 중 Debug Client/Server와
미저장 편집은 유지한다. 두 구성의 데이터 경로 일치와 현재 프로세스 메모리 갱신을 구분한다.

## G06. 무비·배틀 아이템을 포함한 Debug/Release 통합 검토 (2026-09-29)

사용자는 이번 Movie 사운드·잔디·카메라 변경과 배틀 아이템4종을 함께 반영하고 두 구성의
빌드·게시 및 Release4인 완료 경계를 검토하도록 요청했다. 현재 checkout의 무관한 편집을
보존하며 최신 저장본에 필요한 필드만 병합한다. 기존 Debug 유지 요청은 이번 두 구성 빌드
요청으로 갱신됐고 실제 제품 프로세스 종료를 확인한 뒤 정상 Product 빌드를 수행한다.

원본/후보 검증을 마친 Movie·Map 리소스는 프로젝트 및 Desktop/GBResources에 같은 상대
경로로 전달한다. 기존 다른 파일은 지우지 않는다. 해당 domain publisher의 Publish/Check와
Item·Gameplay·Composition 게시 일치를 확인한 뒤 Debug/Release Engine·Shared·Server·Client를
정상 순서로 빌드한다. 생성물 직접 수정과 임의 렌더링 옵션 변경은 하지 않는다.

Release Server의 실제4인 room lifecycle·쿠크 raid·배틀 아이템과 protocol, 필요 Party4 live
network scenario를 실행한다. 이전 광역 실패는 현재 데이터/코드로 다시 분류하고 제품 결함과
오래된 fixture를 근거로 구분한다. 검사 통과를 위해 제품 데이터를 이전 값으로 돌리지 않는다.
Client/UI 자동 실행 없이 가능한 정량·네트워크 검증을 끝내고 실제4클라 화면·입력·음향과
전투 완료의 사용자 확인 경계를 RESULT에 분리한다.


## G07. Server 프로젝트 다시 로드와 Ctrl+F5 복구 (2026-09-29)

사용자 오류창의 KoukuSupportSurface.cpp 중복 ClCompile을 실제 Server.vcxproj와 대조했다.
동일 metadata의 KoukuSupportSurface.cpp, Navigation.cpp와 PlayerSkillFixtures.h가 각각 두 번
등록돼 있다. MSBuild의 성공과 Visual Studio 프로젝트 로드 성공은 다르므로, 뒤쪽 중복 항목만
제거하고 원래 등록과 bigobj 설정, 새 BattleItems 등록은 보존한다. 소스 파일은 삭제하지 않는다.
Server + Client의 Server Start 항목은 복구된 최신 상태를 유지한다.

기존 프로젝트 XML만 수정하며 새 소스 및 project/filter 추가는 없다. 네 project/filter 문서의
XML parse와 항목 중복, Debug x64 Product 빌드·배포와 실행 파일 의존성, 서버의 짧은 격리 실행을
검사한다. Client/VS UI의 다시 로드·Ctrl+F5 실행은 사용자 조작으로 확인하며 미확인 상태를 구분한다.
