# 카드미로 플레이어 망치와 빙고 앵콜 시작 회전 편집

## G00. 현재 소비 경로와 변경 범위

`MainApp.cpp`의 `RenderArenaFollowCameraSettings`에는 이미 Card Maze Player Hammer의
Pos(cm), Rotation(deg), 축별 Size가 있다. 그러나 공용 `preview`는 카메라의
`Is_PresentationOverrideActive()`가 참이면 호출을 생략한다. 카드미로 카메라가 화면을
소유할 때 망치 저작값도 활성 Level profile에 전달되지 않는 것이 현재 코드의 결함이다.
`CCharacter::Update_PresentationRootMatrix`는 이미 매 Update에서 profile의 세 값을
`Part_95_MazeHammer`에 적용하므로 본 부착이나 별도 망치 런타임을 새로 만들지 않는다.

`Render_KoukuEncoreRotation`도 이미 존재한다. 실제 빙고 Saydon의 presentation yaw offset을
갱신하며, Save는 `Gameplay.world.json`의 stable placement
`boss.kakulsaydon.bingo.saydon`의 `yawDegrees`와 문서 revision만 갱신한다. 최신 저장본과
해당 yaw 충돌을 검사하고 atomic replacement/rollback을 사용한다. 이 패널이 현재 관문 버튼
위에 있으므로 사용자가 지정한 빙고 앵콜 버튼 바로 아래로 옮긴다.

시작 기준은 `codex/spider-pattern-fear-sound`, HEAD
`979601d0f62fc60265457678c192fbb37ccf68b7`이다. MainApp의 기존 preparation status 관련 dirty
hunk와 다른 세션의 Level/Server/Composition 변경은 보존한다.

## G01. MainApp.cpp의 망치 편집과 Save

변경 파일은 `Client/Private/MainApp.cpp` 하나다. Card Maze Player Hammer 내부에서 별도
`hammerEdited`를 모으고, 수정 및 Reset 직후 기존 `applyProfile`을 호출한다. 카메라 override
여부로 이 호출을 막지 않고 `Set_FollowEnabled`도 호출하지 않는다. 기존 Level setter는
profile validation 후 활성 map profile과 Character의 presentation snapshot을 갱신하며,
`CCamera_Free::Set_FollowPose`는 진행 중인 presentation camera의 화면 소유권을 유지한다.

TRS 입력 바로 아래에 `Save player hammer`를 둔다. 기존 `CArenaCameraProfile::Save`와
map별 source baseline 검사를 그대로 사용하여 맵 camera JSON 안의 망치 필드를 영속화한다.
저장 실패 시 오류와 draft를 보존한다. 현재 map이면 기존 apply 경로를 사용하고, 다른 map을
선택한 경우 다음 입장에서 적용되는 저장값임을 기존 상태 메시지로 구분한다.

## G02. 앵콜 회전 패널 위치

`RenderKoukuSaydonArenaControls`의 관문 반복문에서 `pAuditionPlacementId`가
`boss.kakulsaydon.bingo.saydon`인 행 바로 아래에 기존 `Render_KoukuEncoreRotation`을
호출한다. index를 하드코딩하지 않는다. 기존 live preview, Save, Reload, Reset과 freshness
계약을 유지한다. 저장한 시작 회전을 Server combat에 적용하는 경로는 기존 World Gameplay
publish와 Server world reload이며, Client의 presentation preview를 Server 회전 변경으로
설명하지 않는다.

## G03. 검증과 완료 경계

새 C++ 파일이나 schema는 없으므로 vcxproj/filter 등록 변경은 필요 없다. 현재 MainApp TU를
독립 out 경로로 실제 컴파일하고 기존 카드미로 profile 검사의 production parser/save를
격리 Data root에서 실행한다. 망치 TRS roundtrip, 범위 거절, stale write 보존을 확인한다.
변경 범위 `git diff --check`를 실행한다.

Client/UI를 실행하지 않는다. 실제 카드미로 카메라 중 TRS 변화, 근처 Save, 빙고 버튼 바로
아래 회전 편집과 화면 변화는 사용자 확인 대상이다. 소스 구현·독립 컴파일과 Product
통합 빌드/배포를 RESULT에서 구분하며 이 작업은 live authoring JSON을 자동 교체하거나
publish하지 않는다.
