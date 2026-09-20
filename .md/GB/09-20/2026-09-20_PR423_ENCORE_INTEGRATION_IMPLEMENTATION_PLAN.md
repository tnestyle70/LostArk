# PR #423와 앵콜 컷신 병합 구현 계획

## G00. 기준과 범위

PR #423의 `b1a84dc45`와 PR #422가 병합된 main `6a0d7264a`를 별도 worktree에서 합친다. 원래 작업 폴더의 편집 데이터와 실행 파일은 교체하지 않는다. 사용자가 요청한 충돌 해결, 새 PR 생성과 main 병합까지 수행한다. 이 대화에서 조사한 공격 콜라이더와 Title 분류 기능은 이번 병합에 새로 구현하지 않는다.

## G01. 저장 ID와 원본 병합

우리 P96 빙고 반복 전투를 유지하고, 같은 ID를 사용한 incoming 앵콜 컷신은 사용하지 않은 P97로 옮긴다. 앵콜 내부 stage/action/occurrence 참조도 함께 변경한다. Sequence 앵콜 P10과 충돌하지 않는 World ID는 유지한다. 기존 P75/P9 엔딩과 사망 애니메이션 연결을 원본과 비교한다. 두 Composition은 기존 항목을 보존하며 revision을 각각 1918/120으로 올린다.

WorldSequence는 기존 사운드·자막·배우 변경과 앵콜 resource/template/instance를 stable ID로 합친다. Independent Effect 목록에는 망치 화살표와 앵콜 fade를 함께 보존한다. 문서 충돌은 양쪽 유효한 항목을 유지한다.

## G02. 소비자와 생성물

자동 병합된 C++의 베른 이동, 컷신 주변 숨김, 자막·사운드 수명, 빙고 전환과 Shared packet 계약을 확인한다. Encounter/patternbindings와 Composition 게시물은 저작 원본에서 기존 publisher로 재생성한다. 이전 generated JSON 중 한쪽을 고르는 방식으로 완료하지 않는다. 새 C++ 파일은 없으므로 project/filter 추가는 필요하지 않다.

실행 검사에서 발견한 Mario4 T2 착지 기대값은 main의 `rightSign=-1` 및 카메라 기준 오른쪽과 일치하게 교정한다. 초기 lane 표는 이미 -1인데 뒤의 착지 단언만 +1을 기대하므로 해당 단언과 설명만 바꾸고 runtime/data는 유지한다. Server 재빌드 후 동일 관문 전환 검사를 다시 실행한다.

## G03. 검증과 PR

변경 JSON/XML parse, stable ID 중복·참조, 원래 양쪽 패턴의 보존, publisher 및 관련 focused Python 검사를 수행한다. 격리 worktree에서 Debug Product Build와 필요한 Server 계약 검사를 실행한다. Client/UI는 실행하지 않는다. 실제 실행 리소스의 존재와 PR #422의 별도 배포 리소스를 확인하고 화면·청취 미확인 경계를 RESULT에 기록한다.

검증된 변경을 commit/push하고 main 기준 새 PR을 만든 뒤 사용자 승인 범위대로 병합한다. 새 PR에서 기존 #423을 대체했음을 명시하고 중복 PR을 정리한다.
