# User settings contract harness

저장소 루트에서 한 번에 컴파일·실행한다. Visual Studio C++ x64 도구와 Windows SDK가 필요하다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/UserSettingsContractHarness/Run-UserSettingsContractHarness.ps1
```

`vswhere`로 도구 체인을 찾고 실제 `Client/Private/UserSettingsDocument.cpp`와 `Client/Private/DataJson.cpp`를 직접 컴파일한다. 제품 소스를 복제하지 않으며 제품 프로젝트 항목은 추가하지 않는다. 실행 파일·object·로그·receipt·fixture는 `out/UserSettingsContractHarness` 아래에만 생성한다. `-OutputDirectory`로 바꾸는 경우도 저장소의 `out` 내부만 허용한다. 테스트 실행 파일도 fixture 경로가 repository `out` 내부인지 검사한다. 개별 실행의 PID/tick 하위 폴더가 격리 경로이므로 재실행해도 이전 fixture를 덮어쓰지 않는다.

테스트 프로세스의 `LOCALAPPDATA`만 fixture 경로로 바꾼다. 실제 사용자 JSON을 읽거나 쓰지 않으며 Client/UI를 실행하지 않는다. `stubs`는 Engine의 audio/type/render-quality 소비자만 대체한다. display callback은 성공·실패와 복원 요청을 기록하는 fake다. JSON parser, validation, serialization, Preview, Commit, Win32 파일 I/O는 실제 제품 구현을 실행한다.

검증 범위는 다음과 같다.

- 첫 저장/다시 로드, unknown numeric row와 escaped ID 보존, defaults seeding, 세 window mode round-trip, exact-byte backup.
- Preview 비저장 및 non-display만 적용, cancel snapshot 복원.
- 문법 오류·array root·schema 누락·분수 해상도·알 수 없는 window mode·문자열 row value 거부, 기존 메모리·파일 보존.
- callback 실패와 NaN 후보 거부, 디스크·메모리 보존과 임시 파일 정리.
- Apply 전 또는 callback 실행 도중 외부 저장 발생 시 freshness 검사 거부, display rollback, 외부 bytes 보존.
- 실제 파일의 delete-share 금지 잠금으로 atomic replace 실패를 만들고 이전 설정·파일·display callback 복원을 검증한 뒤 잠금 해제 후 재시도.

성공 결과는 `results.txt`와 대상 소스 SHA-256를 기록하는 `receipt.json`에 남는다. 컴파일 또는 테스트 exit code가 0이 아니면 runner도 실패한다. 실제 Win32 창 조작·DXGI/viewport·GPU 표시·입력·재실행 화면, 파일시스템의 부분 rename 실패 강제 주입은 이 harness의 검증 범위가 아니다.
