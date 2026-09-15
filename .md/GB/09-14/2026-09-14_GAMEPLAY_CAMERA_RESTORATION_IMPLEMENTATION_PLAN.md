# 원작 플레이 카메라 조사·복원과 FOV 도구 구현 계획

## G00. 목표와 현재 실측

베른의 재질·조명 복원 전에 Character Select, Bern, Valtan, KoukuSaydon의 플레이 카메라 기준을 연결한다.
원본값과 프로젝트 튜닝값을 구분하고 사용자 참고 이미지 여섯 장을 개별 분석한다. Client/UI 실행과
화면 판정은 사용자가 수행한다. 이 작업은 기존 `pattern-3`의 다른 미커밋 변경을 보존하며 자동 stage/commit하지 않는다.

- 실제 참고 폴더는 `C:/Users/user/Desktop/로스트아크_베른성`이며 PNG 6개다.
- 현재 엔진은 `XMMatrixPerspectiveFovLH`에 수직 FOV를 전달한다.
- 현재 Character Select는 수직 70도, KoukuSaydon은 수직 60도다. 두 JSON만 F1 도구에 연결됐다.
- Bern/Valtan은 수직 60도·eye [0.4,7.5,4.5]·look [0,1.2,0]를 코드에서 사용한다.
  Bern의 응답은 0, Valtan은 18이다. Valtan 시네마틱 복귀에도 별도 FOV/주시점 상수가 남았다.
- 원본 설치 패키지의 `Default__EFIsometricCamera`에는 DefaultFOV/CurrentFOV 50,
  ZoomDist 1600cm, Pitch -45, Yaw 45가 존재한다. 클래스·맵·카메라 영역·연출의 override는
  각각 조사하고 이 클래스 기본값을 지역별 실행의 완전 일치 증거로 사용하지 않는다.

## G01. ArenaCameraProfile 데이터 정본

`Client/Public/ArenaCameraProfile.h`, `Client/Private/ArenaCameraProfile.cpp`가 기존 JSON
schema와 parse → validate → stage → commit을 계속 소유한다. enum에 BERN/VALTAN을 추가하고
`Data/Camera/Bern.camera.json`, `Valtan.camera.json`을 연결한다. 네 Level이 자기 profile을 한 번
읽고 이후 F1 적용·재진입·캐릭터 재바인딩·시네마틱 복귀가 같은 profile을 소비한다.

FOV의 저장 정본은 계속 `fovYDegrees`다. 원본 수평각은 16:9 기준으로 수직각으로 한 번 변환한다.
원본 카메라의 좌표 변환은 기존 map/cinematic importer의 UE cm → runtime m basis를 사용한다.
확인된 공통 원본 기준을 도구에서 선택 가능하게 하고, 미확정 지역별 override를 임의의 원작값으로 만들지 않는다.
Save는 편집을 시작한 파일 baseline과 현재 디스크 내용을 대조해 외부 변경이 있으면 draft와 파일을 보존한다.

사용자 후속 요청에 따라 발견한 원본값을 네 맵의 시작 JSON에도 적용한다. CS/Bern50도·16m,
Valtan55도·18m, Kouku50도·19m다. Kouku의 source volume은1관문이며2·3관문에는 시험 baseline으로
확장한다는 한계를 UI와 RESULT에 표시한다. native B_CameraTarget의 높이가 미확정이므로
현재 플레이어 origin에 source RelativeZ -0.1m를 적용하는 부분은 프로젝트 연결값으로 구분한다.
이전 카메라 설정은 `BeforeRestoration` preset으로 보존한다. 캐릭터·이펙트의 크기는 변경하지 않는다.
새/이전 projection으로 같은1m 수직선·지면 사각형을 비교해 단일 보정 배율의 한계를 기록한다.

## G02. Bern/Valtan 실제 소비자

`Client/Public/Level_Bern.h`, `Client/Private/Level_Bern.cpp`,
`Client/Public/Level_ValtanArena.h`, `Client/Private/Level_ValtanArena.cpp`가 자기 프로필과
로드 상태를 소유하고, 기존 Character Select/Kouku와 같은 읽기 getter·적용 함수를 제공한다.
`Ready_Layer_Camera`의 descriptor, Bern 초기 spawn framing, `Bind_CameraToLocalCharacter`와
Valtan `Update_CinematicCameraExitTransition`을 같은 offset/look/FOV로 연결한다.
Server의 위치·전투·world 계약은 변경하지 않는다. 잘못된 문서는 이유를 보존하며 기존 카메라 상태를 유지한다.

## G03. F1에서 FOV만 조절

`Client/Public/MainApp.h`, `Client/Private/MainApp.cpp`의 기존 Player Follow Camera를 네 맵으로
확장한다. 기본 편집은 FOV 하나이며 위치·피치·요·롤·응답은 접힌 고급 항목에 둔다.
수평 FOV(16:9 기준), 실제 수직 FOV, 현재 viewport aspect의 수평각을 구별해 표시한다.
원본 기본 기준 적용과 이전 저장값 재로드를 제공한다. 맵·캐릭터 크기는 FOV 조절에서 변경하지 않는다.
카메라 연출 중에는 follow preview를 적용하지 않고 기존 presentation priority를 유지한다.

`Engine/Private/Camera.cpp`는 projection 갱신 시 유효한 viewport aspect를 읽어 현재 창 비율과
표시된 FOV가 일치하게 한다. 임의의 21:9 letterbox나 확인되지 않은 원작 aspect 정책은 추가하지 않는다.

## G04. 등록·검증·인계

새 C++ 파일은 없다. 새 Camera JSON 두 개만 `Client/Default/Client.vcxproj`와 `.filters`의
기존 `96.DataFiles` None 항목에 등록한다. 다른 팀원의 프로젝트 변경은 보존한다.

검증은 변경 JSON/XML parse, 실제 profile parser의 저장/로드·실패 보존, 원본 FOV 변환 수치,
네 Level의 실제 소비자 연결과 정상 Debug Product 증분 build, `git diff --check`로 수행한다.
별도 제품 런타임이나 광역 검증 체계를 만들지 않는다. Client는 실행하지 않는다.
RESULT에 원본 출처·확정 수치·override 경계·여섯 이미지 관찰·자동 검사와 사용자 미확인을 분리한다.
사용자는 Lobby에서 각 맵 진입 후 F1 → Player Follow Camera에서 FOV 변경·Save·Reload·F6 복귀·
캐릭터 변경·연출 복귀·재진입을 확인한다.
