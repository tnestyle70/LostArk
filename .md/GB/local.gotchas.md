# 개인 구현 Gotchas

이 파일은 현재 사용자의 학습·구현 습관을 위한 로컬 메모다. 팀 공용 규칙이 아니며
Git에 커밋하지 않는다.

## 이해하지 못한 구현은 부채다

- 빌드와 실행 성공만으로 완료라고 부르지 않는다.
- 새 클래스나 함수를 넣기 전에 `무엇을 담는가`, `누가 소유하는가`, `누가 호출하는가`,
  `무슨 상태을 바꾸는가`, `결과를 누가 받는가`를 설명할 수 있어야 한다.
- 설명할 수 없는 범용화·미래 대비 필드·두 번째 런타임 경로는 넣지 않는다.
- 기존 Engine/Client 소유권에 녹지 않는 코드는 복사하기 전에 구조부터 다시 확인한다.

## 물리 폴더가 소유권의 정본이다

- 범용 알고리즘과 이동 메커니즘은 `Engine/`이 소유한다.
- LostArk 보스·레벨·입력·애니메이션 정책은 `Client/`가 소유한다.
- Visual Studio `.vcxproj.filters`는 화면상의 가상 분류일 뿐이다. 잘못된 filter에 보인다고
  물리 소유권이 바뀌는 것도 아니고, 올바른 filter로 보인다고 잘못된 물리 위치가
  정당화되는 것도 아니다.
- 새 파일은 먼저 물리 위치와 owner를 확정한 다음 `.vcxproj`와 `.filters`를 등록한다.
- 다른 게임 오브젝트에서도 그대로 쓸 수 있고 Client 타입을 전혀 모르는 클래스라면
  Client/Boss에 두지 말고 Engine의 대응 서브시스템에 둔다.

## Navigation 경계

```text
오프라인 Python baker -> .navgrid 데이터
Engine CNavGrid       -> 셀/높이 데이터와 직선 통과 판정
Engine CPathFinder    -> A* 탐색
Engine CNavigation    -> Component 진입점과 world waypoint 변환
Engine CNavPathFollower -> waypoint 소비와 Transform 이동
Client CLevel_AssetTest -> F5/마우스 입력
Client CValtan/CBody_Valtan -> 보스 상태·애니메이션·모델 전방축 보정
```

- A*가 반환한 모든 셀 중심을 그대로 조향점으로 쓰면 셀 크기가 작아도 지그재그가 보인다.
  셀 크기를 먼저 바꾸지 말고 walkable 직선 검증으로 중간 waypoint를 제거한다.
- 경로 방향은 parent `CTransform::LOOK(+Z)`가 소유한다. 특정 모델의 forward axis 차이는
  navigation을 비틀지 말고 해당 body의 local yaw에서만 보정한다.
