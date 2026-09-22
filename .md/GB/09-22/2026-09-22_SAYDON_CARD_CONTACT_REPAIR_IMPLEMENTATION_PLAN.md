# 세이튼 카드 Trigger·비둘기·접촉 넉백 수정 구현 계획

## G00. 현재 기준과 수정 경계

`GB/collider-pattern-bug-fix`의 기존 미커밋 변경과 사용자 저장 시간을 보존한다.
카드 4개 Trigger는 각 저장 시각에 한 장을 생성하고 접촉 전까지 추적한다.
생성 방향은 현재 보스 방향에 180도를 더한다. 원본 사운드 notify와 설치된
Sound resource를 대조하여 카드 생성 시각에 Sound Box를 연결한다.

## G01. 카드 생성 계약

Composition codec·Workbench·projector가 Trigger의 PURSUIT_PROJECTILES를 저장하고
기존 Server projectile 생성과 Client preview 경로로 전달하도록 연결한다.
기존 Duration 순차 생성은 호환하며 해당 패턴에서는 비활성화한다.
사용자가 만든 Trigger ID와 시작 시각을 재사용하고 자동 간격으로 덮어쓰지 않는다.

## G02. 실제 접촉과 서버 재생

`KoukuSaydonActionWorkbench.cpp`의 `Follows_ServerClock`은 접촉 피해·밀림이 있는
패턴의 Preview Play도 기존 Server audition을 사용하도록 확장한다.
Client가 별도 피해·낙사를 계산하지 않으며 Play Pattern과 같은 판정 입력을 사용한다.

조커찾기의 기존 생성 피해 원형은 실제 뿅망치에 붙은 BOX로 교체한다.
카드 뒤집기용 접촉과 사용자 World/MAP 배치는 보존한다. projector의 본 부착 경로가
OBJECT_CONTACT만 처리하던 조건을 피해 ENTER_AREA에도 연결하여, 표시되는
본과 서버에서 굽는 본 좌표가 같은 source clip·preScale·occurrence TRS를 소비하게 한다.
바람·바주카·두 번/세 번 내려치기는 각 공격의 현재 표시 위치와 판정 시각을 대조한다.

Server의 기존 ballistic 이동은 XZ navigation clamp 없이 중력을 적분한다.
기존 dirty 수정에 있는 FALLING 전환 시 flight flag 해제를 보존한다.
새 Client movement 또는 두 번째 낙사 runtime을 만들지 않는다.

## G03. 비둘기 재질

기존 카드 복구의 원본 native 재질 입력과 exposure 처리 이력을 대조한다.
설치된 비둘기 mesh leaf의 입력만 수정하고 정상 카드·다른 effect의 공용 셰이더나
전역 scene exposure에 보정을 전파하지 않는다. 원본 복원과 사용자 요청 밝기 조정은
RESULT에서 구분한다.

## G04. 병합·검증·게시

수정 후보의 구조와 소비자를 먼저 검증한다. 교체 시점에는 최신 저장본을 다시 읽고
stable ID/필드 단위로 병합하며 hash 재확인·백업·원자 교체와 자기 변경 rollback을 유지한다.
사용자 편집 저장 기준을 한 번 확인한 후 Composition과 Gameplay를 publish한다.
각 대상 패턴이 unavailable로 빠지지 않았는지 결과 inventory와 실제 bootstrap을 검사한다.

기존 파일을 수정하므로 신규 C++ project/filter 등록은 없다. C++ 변경은 정상 증분
Product 빌드로 검증한다. 실행 중 EXE 잠금은 데이터 publish와 분리하고 사용자가
종료한 뒤 링크한다. Client/UI 실행과 화면 판정은 사용자가 담당하며 자동 완료로 기록하지 않는다.
