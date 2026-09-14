# Debug 탈것 최초 탑승 로딩 구현 계획

## G00. 현재 경로와 목표

`CLoader::Ready_For_CharacterSelect/Ready_For_Bern`은 차량 catalog 전체를 순회해
`CVehiclePresentationAssetService::Ensure_Prototypes`로 모델을 미리 등록한다.
서비스는 기존 `CModel -> CMaterial`로 모델·재질·텍스처를 준비하고 좌석 본과 idle/run을
검증한 뒤 level/vehicle ID별로 캐시한다. `CClientReplication`은 snapshot마다
`CCharacter::Apply_NetworkVehicle`에 Server가 확정한 탑승 ID를 전달한다.

Debug의 진입 로딩에서 차량 모델 준비를 제거하고 첫 실제 탑승 snapshot에서 해당 차량만
준비한다. 차량 catalog 조회와 가벼운 `Part_Vehicle` GameObject prototype은 유지한다.

## G01. 변경 파일과 실패 처리

- `Client/Private/Loader.cpp`: 두 level의 `Begin_LevelLoad`는 유지하고 전체 차량 모델 준비
  루프와 상태 문구를 `#ifndef _DEBUG` 안에 둔다. Release의 선로딩은 유지한다.
- `Client/Private/Character.cpp`: Debug에서 차량 prototype이 준비되지 않았으면 기존
  `Ensure_Prototypes(m_pDevice, m_pContext, level, vehicleId)`를 호출한다. catalog/rider
  검증 뒤 준비하고 성공한 후보 part만 기존 `Replace_PartObjectGroup`으로 교체한다.
  준비 실패는 이유를 보존하고 기존 presentation을 유지한다. 같은 실패 snapshot의 반복
  로드는 차단하고 Server가 하차/다른 차량 ID를 보내면 실패 latch를 해제해 다음 탑승을 허용한다.
- `CLAUDE.md`: Debug 첫 탑승 준비와 같은 level의 재사용 경계를 차량 사용법에 추가한다.

기존 동기 준비 경로를 재사용하므로 최초 탑승은 모델 I/O·GPU resource 생성 시간을 부담한다.
비동기 무정지 로딩을 구현했다고 표현하지 않는다. 캐릭터의 탑승자 animation set은 기존
캐릭터 bundle에 유지한다. Server 승인/속도/판정, 모델 좌표·재질·pass, Resources는 변경하지 않는다.
새 C++ 파일이나 public 인터페이스가 없으므로 project/filter 등록은 필요 없다.

## G02. 검증과 실행 경계

사용자는 현재 Client/Server 실행 유지를 요청했다. 공용 출력·PCH·EXE를 교체하지 않고
선택된 VS18 x64 toolset과 현재 compile 옵션을 사용해 변경 CPP의 OBJ/PDB를 `out` 아래에
격리해 컴파일한다. Debug/Release 전처리 분기와 UTF-8/CRLF, 실제 diff도 확인한다.
Product 링크·배포와 실제 첫 탑승/재탑승/레벨 재진입 화면은 프로그램 종료 후 검증할 항목이다.
동시에 요청된 F1 메뉴 위치·기본 접힘 변경은 09-07 F1 Sequence Viewer PLAN G7을 따른다.
