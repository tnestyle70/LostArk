# 레이드 PR 통합과 Release 공통 표시

## G00. 기준과 병합

현재 기능 브랜치 `codex/kouku-gate3-bingo-flow-0923`의 PR #456을 갱신한다. 시작 HEAD는
`7cea970b6`, 작업 트리는 clean이다. main `5dc2adca0`의 플레이어 hit shape를 합치고
Gameplay publisher로 양쪽 source를 함께 게시한다. PR #453은 앵콜 공통 UI/시간과
NPC 복귀·Mario dead return을 가져오되 현재 5m 낙사면, 1/3관문 낙사 금지와
현재 관문 시작점 부활을 보존한다. 생성물 충돌은 publisher로 해결한다.

PR #454는 영향 범위를 조사한다. Retail Server 수치와 Client 표시, 공용 publisher의
선택 프로필 유지, World 게시 범위가 닫히기 전에는 이번 통합에 포함하지 않는다.

## G01. MainApp 공통 표시

`RenderFpsText`의 Release 경로는 사용자 FPS 숨김 설정과 cinematic/HUD suppression에
관계없이 기존 엔진 폰트로 FPS를 그린다. Debug 설정은 유지한다. F7 Profiler는
사용자 결정대로 유지하고 창을 열 때 자동 Capture를 시작하는 동작을 제거한다.
수집은 기존 Profiler 창의 명시적 Capture 조작이 소유한다.

일반 저작 창은 Debug에만 노출한다. Release의 기존 제품 입력/텍스트 서비스가 사용하는
ImGui context를 무작정 제거하지 않고, 필요 없는 docking/외부 viewport를 비활성화한다.
다른 담당자의 MainApp Debug Balance Test 메뉴 변경과 함수 단위로 병합한다.

## G02. 기능별 구현과 검증

공용 Balance Test와 Kill Boss, Release 전체 클래스 사전 로딩·이동 회귀,
앵콜 UI 수명과 실패 격리는 각각 대응 09-24 PLAN/RESULT에서 소유한다.
사용자가 추가한 카드미로 스폰 높이 navigation과 갈고리 상승 전 해제는 실제 source와
runtime 소비자를 조사한 뒤 별도 기능 단위로 수정한다.

변경된 정본의 domain만 publish하고 JSON/XML parse 및 diff check를 수행한다.
Engine/Shared/Server/Client는 정본 runner로 Debug/Release 증분 Build한다.
실제 새 Server EXE의 관련 계약 검사와 게시된 navigation 높이/이동을 확인한다.
Client 실행과 4인 화면·성능 판정은 사용자가 직접 하며 자동 검증과 구분한다.
