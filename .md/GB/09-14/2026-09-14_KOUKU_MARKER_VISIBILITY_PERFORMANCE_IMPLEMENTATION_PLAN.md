# 쿠크 이동 마커 가시성·재생 비용 개선 구현 계획

## G00. 사용자 요청과 실측

사용자는 pull 뒤 쿠크 진입맵의 60fps 저하 원인을 검토한 뒤 마커 전체 최적화를 승인했다.
기준 HEAD는 `904303a9`다. 같은 checkout의 별도 피날레 맵 복구와 미저장 Composition을 보존한다.
`fb9a8bb9`가 입구 5개에 Mario 이동 마커 18개를 더했으나, 기존 갱신 루프는 23개 모두를
거리·카메라와 무관하게 생성하고 매 프레임 sample한다.

사용자 저장 `profiler_20260914_194637_078_frame57_3948_0.json`의 10~53프레임은
interval 18.480ms, CPU 17.725ms다. 이펙트 Render 4.202ms와 Service Update 1.814ms는
중첩되지 않아 합계 6.015ms다. 전체 비용이며 이번 증분만의 수치는 아니다.
동일 진입 위치와 유사한 과거 기록의 occurrence 5회에서 23회로 증가했다.
새 마리오 18개는 시작점에서 약 1.5~2.6km 떨어져 있다.

## G01. Level의 시계와 최종 카메라 제출

`Client/Private/Level_KakulSaydonArena.cpp`와 대응 헤더의 기존 marker owner를 확장한다.
Level Update는 23개의 scalar 7초 시계만 진행한다. 실제 생성·sample·render 제출은
`Client/Private/MainApp.cpp`의 portrait 카메라 복귀 후, Render.World 직전에 수행한다.
따라서 follow/free, 제품 스킬과 Debug 연출 카메라가 결정된 현재 view/projection을 사용한다.
Server movePlayer/playSequence 트리거, 배치 ID와 retire/reload/Level 종료 계약은 유지한다.

`effect.world.move_destination`의 실제 source module과 설치 mesh에 대해 보수적 8m sphere를
확인한다. 활성 판정은 16m 여유를 더한 sphere, 이미 활성인 표시는 32m 여유를 더한 sphere를
사용한다. 작은 카메라 왕복은 같은 occurrence를 유지한다. invalid 카메라·수치는 기존 실행을
유지하며 임의의 player 거리 제한으로 자유 카메라의 원거리 표시를 제거하지 않는다.

처음 필요한 marker만 기존 prepared Spawn_LevelPlacement와 선택 handle commit을 사용한다.
화면 밖에서는 객체를 재생성하지 않고 보존하며 external sample과 render 제출을 생략한다.
재진입하면 보존한 7초 phase로 기존 transform-history sample을 재구축해 현재 입자 tail을 복원한다.
새 진입의 과거 전체 시간 대신 현재 0~7초만 재구축하며 첫 구간은 여유 영역에서 준비한다.
계산/준비 실패는 해당 marker를 숨기고 기존 stop·retire 경로로 격리한다.

## G02. 기존 Effect 제출 경로의 명시 호출

`Effect_Object.h/.cpp`에 기본 false인 수동 제출 상태와 `Submit_RenderGroups`를 추가한다.
기존 Late_Update의 rendergroup 등록 본문을 이 함수로 이동하고 일반 호출자는 그대로 사용한다.
수동 상태에서는 자동 Late_Update가 제출하지 않는다. 실제 렌더러와 재질·입자 코드는 공유한다.

`Effect_PresentationService.h/.cpp`의 `Submit_LevelPlacementSample(handle, visible)`는
Level-owned, externally-sampled active handle에만 연결한다. 비가시 호출은 visibility를 끄고
대기 sample을 취소한다. 가시 호출은 기존 history sample 성공 뒤 visibility와 같은 제출 함수를
연결한다. MainApp의 marker seam이 프레임당 한 번만 호출한다. 첫 생성도 그 seam에서 commit해
자동 Late_Update와 겹치거나 재진입 첫 프레임의 제출이 빠지지 않게 한다.

## G03. 검증과 완료 경계

새 제품 파일과 shader·데이터 변경은 없다. 따라서 vcxproj/filters 등록 추가와 publisher는 없다.
기존 C++ 인코딩·BOM·CRLF를 유지한다. 실제 codec/playback을 사용하는 out CPU 검사로 전체
7초 bounds, wrap, suspend/resume의 입자 행렬·색·tail 일치를 확인하고, 실제 가시성 함수의
경계·invalid 입력과 marker retire/reload/제출 수명은 작은 focused 검사와 독립 diff로 확인한다.

현재 Client PID 3948와 Server PID 46360은 사용 중이다. 먼저 out에 필요한 CPP를 컴파일하고,
실행 파일 교체가 가능한 시점에 정본 Product Debug 증분 Build로 링크·배포한다.
실제 실행 또는 UI 조작·캡처는 하지 않는다. 사용자 동일 조건 재캡처 전에는 80~100fps 회복을
완료로 기록하지 않는다. RESULT에는 실제 source diff, 수치 검사, compile/link와 미실행 화면 검증을 구분한다.
