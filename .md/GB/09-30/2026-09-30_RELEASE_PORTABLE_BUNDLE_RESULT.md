# 2026-09-30 Release 포터블 묶음 RESULT

## 결론

**Release 빌드와 포터블 묶음 생성을 끝냈다.** 사용자가 VS를 닫은 뒤(05:11) 진행했고 컴파일·링크 오류는 없었으며 소스를 고친 것은 없다.
결과 폴더: `C:\Users\USER\OneDrive\바탕 화면\LostArk-Release-20260930-Maharaka` (2,800개 파일, 1.80GB, 참고 폴더 `LostArk-Release-20260929 (2)`와 같은 구성).
게임 실행과 화면·음향 확인은 하지 않았다. 사용자가 한다.

## 소스 기준

- 브랜치 `codex/maharaka-sea-island-waterpang-0930`, HEAD `6faf5518`(origin/main `50dcd986` 병합), `NETWORK_PROTOCOL_VERSION = 125`.
- 참고 폴더(팀 묶음)는 protocol 124, gitHead `50dcd986`이라 이 묶음과 접속되지 않는다.

## 제작 방법 (실측)

- 도구: `Tools/ReleasePackaging/build_portable.py`. 런처 `LostArk.exe`는 같은 폴더의 `PortableLauncher.cs`를 .NET Framework C# 컴파일러로 컴파일한 산출물이다.
- 순서: Release Product 빌드 → `generate_numeric_source_bindings.py` → `test_package_tools.py` → `build_portable.py --plan-only` → `build_portable.py --stage ... --output-zip ... --build-receipt ...` → stage 폴더를 바탕화면으로 복사.
- 패키저 제약: `--output-zip`의 부모는 저장소의 부모 폴더여야 한다. ZIP은 `C:\Users\USER\source\졸업팀폴\LostArk-Release-20260930-Maharaka.zip`(167,333,132바이트, sha256 `1c8bcd85…`)에 생겼고 바탕화면 폴더와는 별개다. 불필요하면 지워도 된다.

## Release 빌드 (정본 러너 `Invoke-BuildAndRegression.ps1 -Configuration Release`)

| 단계 | 결과 | 시간 |
|---|---|---|
| Engine | PASS (OBJ 71, CSO 29, 바이너리 2) | 1,348,858 ms (22분 29초) |
| Shared | PASS (OBJ 5) | 1,180 ms |
| Server | PASS (OBJ 91) | 223,476 ms (3분 43초) |
| Client | PASS (OBJ 353, CSO 195, 바이너리 2) | 4,201,683 ms (70분 2초) |

- 시작 05:12, 종료 06:48, 종료 코드 0. Rebuild/Clean은 하지 않았다. toolchain은 MSBuild 17.14.23(vswhere 최신 C++ 설치).
- 영수증 `out/BuildPipeline/runs/20260929T214823390Z-release-product.json`: `result=PASS`, `skippedBuild=false`, `missingRuntimeInputs=[]`, `invalidRuntimeInputs=[]`. 아이템 카탈로그 등 runtime 데이터 검사 PASS.
- 셰이더 컴파일 경고(X4000, X3571, X4717)는 기존 셰이더에서 나오는 것이고 오류가 아니다.

## 패키저 준비와 생성

- `generate_numeric_source_bindings.py`: 576 fields, 6 files. 이 실행이 `Data/Balance/NumericSourceBindings.json`을 갱신했다(추적 파일 변경 1개, 커밋 대상 여부는 별도 판단).
- `test_package_tools.py`: 19건 통과.
- `--plan-only`: files 2,773 / directDataFiles 2,147 / compiledShaders 256 / resourcesIncluded 0 / protocol 125.
- `build_portable.py`: status PASS, 7분 소요.

## 검증 (실행한 것만)

- stage와 바탕화면 사본: 파일 2,800개, 1,932,481,247바이트로 동일. 한쪽에만 있는 파일 0, 크기 불일치 0.
- `bundle-manifest.json` 항목 2,799개의 sha256을 바탕화면 사본에서 재계산: 불일치 0, 누락 0.
- `release-ready.receipt.json`: status PASS, protocol 125, gitHead `6faf55189a72…`, dataRevisions source 2469 / sequence 183.
- binaryPins 3개(Client.exe 18,151,936 / Server.exe 5,262,848 / Engine.dll 2,539,520바이트)의 크기와 sha256이 묶음 안 실제 파일과 일치.
- 런처 사전 점검 `LostArk.exe --check <Client\Bin> preflight.json`: PASS, `clientStarted=false`, `serverStarted=false`. 게임 프로세스는 시작되지 않았고 결과 JSON은 묶음 밖(작업 tmp)에 저장했다.
- 참고 폴더와 구조 비교: 최상위 항목은 같고 참고 폴더에만 있는 것은 `out/`(서버 실행 중 생기는 `ValtanPatternTransactions` 런타임 폴더)뿐이다. 새 묶음에만 있는 최상위 항목은 없다. 파일 수는 참고 2,791개(1.91GB) 대 새 2,800개(1.93GB).

## 실행 방법과 주의

1. 서버 주소는 패키저 상수로 `192.168.0.22:7777` 고정이다. 런처가 자식 프로세스의 `LOSTARK_SERVER_HOST`를 이 주소로 덮어쓰므로(`PortableLauncher.cs` 170행) 이 PC의 잔재 환경변수 `10.16.127.103`은 묶음 실행에 영향이 없다.
2. 서버는 `192.168.0.22`를 가진 PC에서 `ServerHost.cmd`를 한 번 실행한다(내부적으로 `LostArk.exe --server`, 서버는 `--bind-address 0.0.0.0`).
3. 클라이언트는 각 PC에서 `LostArk.exe`를 실행하고 뜨는 폴더 선택창에서 최신 Resources가 있는 폴더를 고른다. 이 PC라면 `C:\Users\USER\source\졸업팀폴\LostArk\Client\Bin` 또는 `...\Client\Bin\Resources`를 선택하면 된다(런처는 `Client/Bin/Resources`, `Resources`, 선택 폴더 자체 순서로 필수 폴더 7개를 찾는다).
4. 이 PC(IP `192.168.0.43`, `192.168.137.1`) 단독으로 서버와 클라이언트를 함께 확인하는 것은 이 묶음만으로는 안 된다. 묶음 endpoint가 고정이고 `--check`가 endpoint 불일치를 거부하기 때문이다. `192.168.0.22`를 가진 PC에서 서버를 실행해야 한다. 이 PC에서 단독 확인은 Debug 빌드(로컬 endpoint 설정 `Sync-TeamLanEndpoint.ps1 -EndpointMode Local`)로 해야 한다.
5. EXE를 종료하면 만든 캐릭터와 플레이 상태는 사라진다(README 그대로).

## 남은 것 / 위험

- 실제 게임 실행, 화면·음향·LAN 품질 확인은 미실행이다. 링크와 셰이더는 컴파일 성공까지만 확인했다.
- 06:27에 누군가 `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements`에 `LV_BER_BERNCASTLE_FLOORFILL` 434행(바닥 채우기)을 추가했다. 이번 묶음이나 작업의 산출이 아니고 삭제·변경 없이 추가만이며, 게시본에 반영되지 않았으므로 묶음에는 영향이 없다. 출처를 확인하지 못했고 손대지 않았다. 커밋 전 사용자가 확인해야 한다.
- `Data/Balance/NumericSourceBindings.json` 갱신은 패키저 준비 단계의 정상 동작이다.
- 프로토콜 125는 참고 폴더(124)와 접속되지 않으므로 팀원 PC와 시험하려면 같은 빌드를 함께 받아야 한다.
- 커밋과 푸시는 하지 않았다.
