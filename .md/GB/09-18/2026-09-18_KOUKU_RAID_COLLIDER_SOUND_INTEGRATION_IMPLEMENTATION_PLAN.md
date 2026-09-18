# 쿠크세이튼 전체 재생·피해 판정·사운드 연결 구현 계획

## 작업 기준

2026-09-18 사용자가 저장한 관문별 Pattern Flow, 1마리오 1~4페이즈 Parent,
카드 주사위 속박과 피자·마리오 위치 Trigger를 기존 Server 실행 경로에 연결한다.
편집 중 저장본은 계속 바뀌므로 revision 1748은 조사 기준일 뿐 교체 기준이 아니다.
설치 후보를 검증한 뒤 최신 저장본을 stable ID와 변경 필드로 병합한다.

## G00. 현재 동작과 변경 경계

피해는 Server fixed tick의 CombatObject/Logic 판정이 소유하고 Client 이펙트는
같은 객체의 pose와 lifetime을 표현한다. 사운드는 기존 SoundCueCatalog와 SOUND
occurrence를 사용한다. 개별 particle에 collider를 붙이거나 별도 재생기를 만들지 않는다.
카드미로의 망원경 담당자·사냥꾼 역할과 Mario lane 이동은 이미 있으므로 이를 확장한다.
현재 Complete Play의 시퀀스와 Flow 진행은 요청 Client에 묶여 있으므로 관문 진행과
시퀀스 시작 시각을 Server가 소유하도록 연결한다.

## G01. 스킬별 피해 템플릿

원본 Action/SkillEffect의 범위·시간·offset을 출처와 함께 추출한다. 정적 영역은 기존
COLLIDER와 ENTER_AREA, 투사체와 추적 과녁은 CombatObject 인스턴스에 붙는 피해
템플릿을 사용한다. 한 발당 한 판정 객체이며 visual의 파티클 수와 무관하다.
normal damage, 최대 HP 비율, 즉사를 구분하고 원본에서 입증되지 않은 피해량은
PROJECT_TUNED로 기록한다. 원본 geometry를 복원하지 못한 항목은 coverage에 남긴다.
빠른 투사체는 이전 pose부터 현재 pose까지 검사하고 충돌 판정 뒤 contact despawn한다.

## G02. 원본 사운드

Action clip notify와 시퀀스 InterpTrackAkEvent의 실제 원본 시각을 사용한다.
Wwise Random은 한 variant를 선택하고 Layer/동시 Play는 함께 재생한다. 필요한
layer·delay·gain은 원본 PCM으로 오프라인 합성하고 단일 무변조 WAV는 재사용한다.
soundEvent stable ID로 기존 catalog를 조회하며 occurrence seed를 고정해 seek 결과가
바뀌지 않게 한다. 미해석 switch/state와 control-only event는 누락 사유를 기록한다.

## G03. 카드 주사위와 위치 Trigger

속박 lifetime 동안 생존 참가자 중 한 명을 Server가 고르고 나머지만 속박한다.
1인은 속박하지 않는다. 카드 projectile은 같은 비속박 대상을 사용한다. 종료·취소 시
이 occurrence가 소유한 속박만 해제한다. 중복 Trigger와 Duration이 대상을 재추첨하지
않도록 동일 역할의 활성 구간을 합친다. 피자 시작 위치는 typed boss teleport로 연결한다.
카운터 unavailable은 저장본의 typed definition과 follow-up의 Gate/body 계약을 모두
검증하며, 잘못된 참조를 숨기기 위해 PRODUCT 검증을 완화하지 않는다.

## G04. 마리오 1~4페이즈

각 Parent의 실제 접촉 창에 설정한 MARIO_ENTER stage를 사용한다. 마지막 tick의
접촉은 패턴 완료 뒤에도 commit하고, 완료 정리가 먼저 큐를 버리지 않게 한다.
솔로는 마리오 복귀 후, 파티는 진입자와 별도로 나머지 인원이 2페이즈를 진행한다.
비진입 인원은 저작된 아이언메이든 위치 기준으로 배치하고 3~4인은 한 명만 속박한다.
복귀 지점은 진입자에 pin해 0키 복귀와 자연 출구가 같은 위치를 사용한다.
보스는 사용자가 지정한 (5.96, 1.30, 950.59)에서 중앙을 보고 무력화 성공까지 진행한다.
1페이즈 실패와 즉사 칼날은 기존 Server 즉사 RESULT 계약으로 연결한다.

## G05. 카드미로와 관문 진행

카드미로 포탈은 처치한 해당 문양의 위치에 생성한다. 망원경 시야 확대는 담당자에게만
적용하며 나머지 N-1명은 각자 포탈로 중앙에 돌아온 뒤 전원이 2관문으로 복귀한다.
Gate2 Flow는 이 완료 조건 뒤 피자로 이어진다. 단독 패턴의 반대 보스는 Idle,
Bundle은 포함된 보스들이 함께 재생한다.
Release에서는 1관문 진입 콜라이더가 동일한 Server raid 준비를 시작하고, Debug Complete Play도
같은 경로를 사용한다. 고정 참가자 1~4명 모두의 revision/리소스 준비 ACK가 끝난 뒤
공통 Server 시각으로 시퀀스·실제 참가자 스폰·현재 저장 Flow를 진행한다.
2026-09-18 추가 요청에 따라 보스 사망 후 자동 10초 이동은 제거한다. main의
클리어/MVP·관문 투표 UI에서 전원 진행 승인하면 다음 관문 시퀀스를 시작하고,
재시작 승인하면 현재 관문 시퀀스를 다시 시작한다. 마지막 3관문은 퇴장/재시작 UI를 유지한다.
컷씬 종료 전에는 전투를 시작하지 않으며 관전자·연출 중 UI 명령은 서버에서 거절한다.
Server orchestrator는 기존 audition 실행과 world sequence broadcast를 재사용한다.
새 C++ TU를 만들면 해당 .vcxproj와 .filters에 함께 등록한다.

## G06. 검증과 반영

Client/Server 최소 컴파일, 실제 catalog parse/project/publish 후보 검증, 1~4인
카드미로·마리오·속박·피해 판정·관문 상태 전이 계약을 실행한다. ServerPlayer layout
변경이 있으므로 과거 object 파일 재링크로 실행 성공을 판정하지 않는다.
설치 직전 hash/revision을 다시 확인하고 백업·원자 교체·자기 변경 rollback을 사용한다.
실행 중 도구의 Reload와 Server 재시작, 실제 Client 화면 확인은 별도로 기록한다.
사용자가 직접 하는 화면 검증을 headless 수치 검증으로 대신 완료 처리하지 않는다.

## G07. 기존 폴더 선택형 얇은 ZIP 배포

2026-09-19 최종 추가 요청은 기존 ResourceDelivery 방식의 얇은 실행 ZIP이다.
Release EXE/DLL/CSO와 양쪽 게시 DataFiles, Client가 직접 읽는 이번 변경의 필수
작은 Data JSON만 포함한다. 전체 Data 1.1GB와 Resources·사운드 미디어는 넣지 않는다.
다른 리소스는 이미 팀에서 공유받았다는 사용자 확인을 적용한다. 이번 신규 WAV 18개는
`NEW_SOUND_PATHS.txt`의 상대 경로만 별도로 전달한다.
기존 LostArk 폴더 선택 wrapper, 설치 전 검증·기존 파일 백업과 no-build Client 바로가기를
재사용하고 새 배포 체계를 확장하지 않는다. 최종 빌드·게시 파일의 hash manifest를
검증한 뒤 ZIP을 전달하며 Client를 에이전트가 자동 실행하지 않는다.
