# Debug 탈것 최초 탑승 로딩 결과

## G00. 소스 반영

`pattern3-rendering-restore`에서 Debug Character Select/Bern의 모든 차량 모델 선로딩을
제거했다. 두 Level의 `Begin_LevelLoad`와 가벼운 Part prototype은 유지한다.
Server snapshot의 첫 nonzero 차량 ID를 `CCharacter::Apply_NetworkVehicle`이 관측하면
기존 `CVehiclePresentationAssetService::Ensure_Prototypes`로 해당 모델만 동기 준비한다.
같은 Level에서 같은 차량의 재탑승/다른 플레이어 탑승은 기존 ready 캐시를 소비한다.

catalog/rider pose 검증 뒤 모델을 준비하고 성공한 part 후보만 교체한다. 실패 이유를
기록하고 기존 presentation을 유지하며, 같은 실패 snapshot에 I/O를 반복하지 않는다.
Server가 하차/다른 차량 상태를 보낸 뒤에는 같은 차량 탑승을 다시 시도할 수 있다.
Release의 진입 선로딩과 Server 탑승 승인·이동 속도는 변경하지 않았다.

첫 탑승의 동기 모델 로딩 비용은 남는다. 캐릭터의 rider animation set은 기존 character
bundle에서 준비한다. 다른 Level로 이동할 때 캐릭터/차량 리소스를 계속 보존하는
process cache는 이번 변경에 추가하지 않았다.

## G01. 실행한 검증

- 기존 Debug command tlog의 VS18 / MSVC 14.44.35207 / x64 / SDK 10.0.26100.0을 사용해
  Character, Loader, MainApp, MainApp_SequenceViewer 네 CPP를 각각 컴파일했다. 모두 종료 0.
  PCH를 읽거나 갱신하지 않도록 `/Y-`, 분리된 `/Fo`·`/Fd`와 `/Zi`를 사용했다.
  이는 격리된 소스 컴파일이며 Product 링크·배포 성공은 아니다.
- 실제 MSVC 전처리: Debug Loader의 차량 준비 호출 0개/level 초기화 2개,
  non-Debug Loader의 준비 2개/초기화 2개. Character는 각각 준비 호출 1개/0개.
  non-Debug 검사는 전처리 분기 검사이며 Release 제품 빌드가 아니다.
- CPP 네 파일의 UTF-8 BOM 없음/CRLF 유지, `git diff --check` 통과.
- 검증 전후 Client/Server EXE, Client PCH/기존 compiler PDB의 크기·수정 시각 변경 0.
  user Client/Server PID 54924/59264를 종료하거나 재실행하지 않았다.
- 근거는 `out/F1VehicleLazy20260914/compile-results.json`, `preprocess-results.json`,
  `product-preservation.json` 및 파일별 로그다. 전처리 도구의 PowerShell stderr 처리와
  `/MDd`의 자동 `_DEBUG` 정의를 교정한 뒤 위 최종 네 분기를 확인했다.

## G02. 사용자 실행 대기

사용자는 Client/Server 실행 유지를 요청했다. 새 소스는 실행 중 EXE에 반영되지 않았다.
사용자가 편집을 저장하고 종료한 뒤 정상 Debug Product Build로 링크·배포한다.
F1 Sequence Viewer는 탈것 바로 아래에서 기본 접힘으로 시작하며, 사용자가 펼칠 수 있다.
Bern/Character Select의 첫 H 탑승, 같은 차량 재탑승과 Level 재진입 화면은 사용자 확인 대상이다.
Client/UI 실행·조작·캡처와 실제 로딩 시간/화면 판정은 수행하지 않았다.
