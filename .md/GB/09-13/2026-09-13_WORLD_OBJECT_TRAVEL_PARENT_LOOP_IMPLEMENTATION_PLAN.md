# World Object 칼날 이동·수명과 Parent 한 주기 반복 구현 계획

## G00. 목표와 현재 기준

Composition Resources에서 선택한 칼날 Object를 여러 WORLD 박스로 구성하고, 한 Parent의
전체 주기를 유한한 시간창 안에서 반복한다. Object Tool Box Detail은 이동 시작·끝점,
방향·속도, 도착 시각, 기본 방향, 개별 수명과 끝점 도착 후 유지/소멸을 편집한다.
자체 회전 속도는 기존 Self Rotation을 유지한다. 도착 후에도 자체 회전은 개별 수명까지
계속되며 이동 경로의 최종 기본 방향과 구분한다.

현재 `world.object.kouku.cutting_blade`의 기본 Motion은 11000ms LOOP,
velocity [0,0,2], angularVelocityDegrees [1440,0,0], 6개 emission을 사용한다.
P31의 WORLD 5개가 이를 참조한다. 사용자가 저장한 이 조합을 자동으로 단일 칼날로
교체하지 않는다. 기존 갈고리는 Transform keys와 emission의 위치·방향·지연으로
이동/정지/소멸을 구성한다. 이 저장·재생 경로를 칼날에도 사용한다.

사용자는 편집을 저장하고 EXE를 종료했다고 알렸으며 Client/Server/MSBuild 부재를
확인했다. 최종 변경 후 정상 Product 빌드를 수행한다. Client 실행과 화면 판정은 사용자 몫이다.

## G01. Object Box Detail의 Travel 저작

대상은 `Client/Public/WorldObjectTool.h`, `Client/Private/WorldObjectTool.cpp`다.
부모 Object에서 기본 Motion으로 진입하는 명령을 제공하고, 자식 Motion에서 Travel을 편집한다.
기존 LINEAR Transform keys를 저장 정본으로 사용하며 별도 편집용 JSON 좌표 복사본을 만들지 않는다.

- 단순 직선 이동과 유지/소멸 키를 읽어 Start/End, 속도, 도착 시각과 수명을 복원한다.
- 정적 키와 일정 velocity로 만든 Motion은 명시적인 Convert to Travel 명령에서
  기존 끝점·기본 자세·emission 배치를 보존하며 위치 키로 변환한다.
- 중간 곡선·가속·복수 트랙 등 지원하지 않는 변환은 이유를 표시하고 기존 키를 보존한다.
- 도착 뒤 위치 유지 또는 visible=false로 소멸한다. 여러 emission은 개별 수명 뒤에 숨고,
  마지막 emission이 끝나는 시각까지 전체 Motion의 시간창을 확보한다.
- 기준 위치·방향은 Motion local space이며 WORLD 배치와 emission yaw/offset이 기존 순서로 적용된다.
- 임시 template/instance를 검사한 뒤 한 번만 draft에 반영한다. 저장은 기존 World Object
  Save와 연결된 Collider/Logic 동기화 및 외부 변경 검사를 유지한다.

## G02. Parent 한 주기 반복

대상은 `KoukuSaydonActionWorkbench.h/.cpp`와 기존 Composition native contract tests다.
Parent의 공통 WORLD/Effect/Collider/Logic 행과 기존 자식 Pattern을 실제
Try_ExpandPatternDocument로 한 주기 문서로 만들고, 별도 일반 자식 Pattern에 보관한다.
원래 Parent는 이 자식을 repeat=true로 지정한 한 행을 통해 사용자가 정한 Loop Window만큼 재생한다.

기존 stable occurrence ID 재명명과 Logic 참조 연결을 보존한다. 모든 검증과 반복 확장 예산
검사를 통과한 뒤에만 draft를 교체한다. 실패하면 기존 Parent와 선택 상태를 유지한다.
사용자가 해당 명령을 누르기 전에는 저장된 Parent를 자동 변환하지 않는다.
무한 재귀나 별도 Server 반복 시계를 만들지 않고 현재 Parent expansion과 publisher를 소비한다.

## G03. 칼날의 원형 판정과 기존 Server 권위 연결

현재 gameplay projector는 물리 속도와 비-Y 회전을 거부한다. Travel은 이동을 기존 위치 키로
저장하여 이 경로를 재사용한다. 중심 CIRCLE 판정은 칼날의 시각적인 X축 자전과 분리하여
위치와 균일 배율을 따라가도록 제한적으로 지원한다. 중심에서 벗어난 판정이나 불명확한 배율을
조용히 원형으로 치환하지 않는다.

대상은 `project_kouku_saydon_composition.py`와 기존 관련 tests,
`KoukuSaydonPresentationPlayer.cpp`의 WORLD Circle 표시,
`Server/Private/KoukuSaydonLogicRuntime.cpp`의 실제 ENTER_AREA 소비자다.
기존 Shared collision primitive를 사용해 빠르게 이동하는 원형 판정의 tick 사이 누락을 검사한다.
Lifetime 이후와 Parent 반복 경계에서 이전 판정이 남지 않도록 검증한다.
피해와 즉사는 기존 ENTER_AREA 결과와 Server damage 경로를 사용한다.

## G04. 검증과 적용

- Travel 변환 전후 끝점·기본 자세·emission 보존, 저장/재로드 역산, 도착 후 유지와 수명 소멸을 검사한다.
- Parent 전체 주기 반복에서 WORLD/Effect/Collider/Logic의 시작·끝과 참조, 실패 시 원본 보존을 검사한다.
- 실제 칼날의 시각 회전이 centered CIRCLE 위치를 변경하지 않는지, 이동 구간 접촉과 수명 종료를 검사한다.
- 변경 C++의 최소 컴파일, 변경 Python의 집중 테스트, 변경 문서 parse와 git diff --check를 수행한다.
- 마지막에 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`의
  정상 Engine → Shared → Server → Client Product Build를 실행한다. Clean/Rebuild로 우회하지 않는다.
- 새 C++ 파일을 추가할 경우 해당 vcxproj와 filters 등록을 함께 확인한다. 현재 제안은 기존 파일 확장이다.
- 결과 문서는 실제 소스·데이터 적용, 실행한 검증, 제품 빌드와 사용자 화면 확인을 구분한다.

## G05. 기존 서버 검사 실패 설명과 현재 제품 검증

사용자는 실패 항목의 의미를 확인한 뒤, 치명적인 문제가 아니라면 현재 빌드로 직접 검증하고
후속 수정사항을 전달하기로 했다. 따라서 맵 끝·아이언메이든 접촉에 의한 개별 칼날 소멸은
이번 후속 작업에서 추가하지 않는다. 끝점·Lifetime 소멸과 동적 접촉 소멸을 구분해 보고한다.

`ServerGameplayContractTests_KoukuProduct.cpp`의 관문/배치 선택, 현재 저작 Gaze 좌표,
실제 audition tick 절차와 `ServerGameplayContractTests_WorldTriggers.cpp`의 bootstrap fixture를
현재 제품 호출자와 데이터 계약에 맞춘다. 실패 assertion을 삭제하거나 결과를 무조건 통과시키지 않는다.
제품 런타임 수정 없이 검사 입력·절차를 바로잡아 집중 검사로 원인을 검증하고, 정상 Product 빌드 후
새 Server.exe의 전체 `--contract-test` 결과를 확인한다. Client/UI는 실행하지 않는다.
