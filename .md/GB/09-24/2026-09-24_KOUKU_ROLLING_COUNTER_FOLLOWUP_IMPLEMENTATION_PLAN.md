# 공굴리기 카운터 성공 후 무력화 연결 계획

## G00. 실제 결함과 기존 소비자

PR454 병합 d9361f98f의 Composition revision2236에서 P81(GATE1)과 P118(GATE3)의
logic.1은 COUNTER_WINDOW logic.85, 0~2837ms, endsPatternOnSuccess=true다.
두 onSuccessLogicIds가 비어 있어 게시 Encounter에도 성공 결과가 없다. 카운터는
현재 패턴만 끝내고 G1의 다음 P101 추적에 즉시 진입한다.

기존 LogicRuntime→Apply_KoukuLogicOutput는 검증된 바닥 착지, 공 WORLD 소유권 정리,
후속 패턴 한 번 삽입을 지원한다. 09-17 계획 G30의 P81 성공→P4 의도와 맞춘다.

## G01. Composition stable field 수정

P81.logic.1의 성공을 기존 공용 result7→P4에, P118.logic.1을 result57→P42에 연결한다.
두 무력화 패턴은 같은 관문·보스·MN_RPCT_05이며 시작1167/유지1667/복귀1333ms와
제품 presentation이 존재한다. 최신 저장본의 hash를 확인하고 두 필드와 revision만
원자 교체한다. 병렬 HP flow 변경은 보존한다. 공용 C++ 판정이나 빈 결과의 의미는 바꾸지 않는다.

## G02. 기존 native fixture 검증

ServerGameplayContractTests_KoukuSupportSurface.cpp의 counter fixture에 실제 게시
P81/P118의 counter window를 소비하는 경우를 추가한다. 성공 결과, 같은 관문의 P4/P42,
다음 추적 앞에 한 번 삽입되는 순서와 3개 stage의 실행을 검사한다. 기존 빈 결과가
후속을 만들어내지 않는 검사도 유지한다. 새 C++/프로젝트 등록은 없다.

projector validate와 실제 publisher는 통합 담당자가 실행하고, Debug/Release의 기존
--kouku-support-surface-contract-test로 소비자를 검증한다. Client/UI 실행은 하지 않는다.
