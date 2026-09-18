# LostArk Release ZIP 안내

2026-09-19 v5 배포 기준. **최종 Release 빌드와 v5 ZIP 생성·파일 검증을 완료했다.**
실제 4인 화면·음향·접속 안정성은 이 배포본으로 사용자 재검증한다.

v5는 압축을 푼 폴더의 EXE·DLL·컴파일된 shader와 Data/DataFiles를 사용한다.
선택한 기존 LostArk 폴더에서는 **Resources만 읽는다.** 기존 저장소에 설치하거나 파일을 덮어쓰지 않는다.

## 1. 배포 상태와 구성

| 항목 | 값 |
|---|---|
| ZIP | `LostArk-Release-20260919-v5-192.168.0.14.zip` |
| 위치 | 저장소의 `Release` 폴더 |
| 상태 | 최종 Release 제품 빌드·바이너리 확인·ZIP CRC/전체 파일 hash·사전검사 PASS |
| ZIP 크기 | **101,394,117 bytes** (약 101.4 MB) |
| ZIP SHA256 | `b6e956b7c106843e34313efcb9ee61780133b68f7439c44c332f9e5b24d04f36` |
| 구성 | Release x64 |
| 서버 접속 주소 | `192.168.0.14:7777` |
| 서버 bind | `0.0.0.0:7777` |
| 현재 게시 데이터 기준 | Action revision **1753**, Sequence revision **65**, protocol **93** |

[v5 ZIP 열기](C:/Users/user/Desktop/LostArk/Release/LostArk-Release-20260919-v5-192.168.0.14.zip)

배포 입력은 runtime 458개와 직접 소비 Data 1,727개(JSON 1,721개·애니메이션 이벤트 6개)이며,
압축 전 약 1.18 GB다. V2 JSON 324개와 제품 UI·카메라·조명·클래스 입력도 포함한다.
기존 v4 변경분 목록만으로 독립 실행 데이터를 대신하지 않는다.
최종 빌드 receipt와 Client/Server/Engine hash가 일치하고, Client/Server는 기존 v4 바이너리와 다른 것을 확인했다.

폴더 구조는 다음과 같다. 빈 `Client/Default`, `Server/Default`도 작업 폴더로 유지한다.

```text
LostArk.exe
ServerHost.cmd
Collect-RuntimeDiagnostics.ps1
README_실행방법.md
bundle-manifest.json
Client/Bin/Release/        Client.exe, Engine.dll, 의존 DLL, 컴파일된 .cso
Client/Bin/DataFiles/     게시된 Client 런타임 데이터
Client/Default/           Client 작업 폴더·로그
Server/Bin/Release/       Server.exe
Server/Bin/DataFiles/     게시된 Server 런타임 데이터
Server/Default/           Server 작업 폴더
Data/                    직접 소비 JSON·애니메이션 이벤트와 참조 문서
```

**모든 Resources와 PNG를 제외한다. `Key_G.png`도 포함하지 않는다.**
모델·텍스처·폰트·음원·UI 이미지는 팀 Drive의 Resources를 별도로 준비한다.
`ChangedData` 폴더, 중첩 Runtime ZIP, 기존 저장소를 변경하는 설치기는 v5에 넣지 않는다.
HLSL 원본 대신 런타임이 읽는 컴파일된 `.cso`를 실행 파일 옆에 둔다.

## 2. 실행 전 준비

- ZIP 전체를 새 폴더에 압축 해제한다. EXE 한 개만 복사하거나 압축 파일 안에서 실행하지 않는다.
- 최신 Resources가 있는 기존 LostArk 폴더 또는 Resources 폴더 자체를 준비한다.
- Resources에는 `Fonts`, `Character`, `Deploy`, `Effect`, `Map`, `Sound`, `UI`가 있어야 한다.
- v5 폴더에 `Framework.sln`, Git checkout, Visual Studio 빌드는 필요하지 않다.
- 공유 서버 PC도 같은 v5의 새 Server와 게시 데이터를 사용해야 한다.

실행기의 사전검사는 패키지 파일의 크기·SHA256과 Resources의 필수 폴더 존재를 확인한다.
외부 Resources의 모든 미디어 내용이 최신인지, 실제 화면·음향이 정상인지는 이 검사만으로 보장하지 않는다.

## 3. v5 실행 방법

1. 압축을 푼 v5 폴더의 `LostArk.exe`를 실행한다.
2. Resources가 있는 기존 LostArk 폴더를 선택한다. `Client/Bin/Resources`를 자동으로 찾으며, Resources 폴더 자체를 선택해도 된다.
3. 패키지 검증이 끝나면 **v5 폴더 안의 `Client/Bin/Release/Client.exe`**가 시작된다.
4. 서버 PC에서는 기존 Server를 종료한 뒤 v5 폴더의 `ServerHost.cmd`를 한 번 실행한다. 다른 PC에서는 Client만 실행한다.

선택한 기존 폴더의 EXE·DLL·Data는 사용하거나 교체하지 않는다.
반복 검증도 v5의 `LostArk.exe`로 실행한다. v4의 설치 바로가기는 기존 저장소 바이너리를 실행하므로 v5 실행 방법과 구분한다.
서버 메모리의 데이터는 파일 게시만으로 갱신되지 않으므로 새 Server로 시작해야 한다.

| 대상 | 사용 경로·설정 |
|---|---|
| Client EXE / 작업 폴더 | v5의 `Client/Bin/Release/Client.exe` / `Client/Default` |
| 직접 소비 Data | `LOSTARK_PROJECT_DATA_ROOT` = v5의 `Data` |
| 외부 Resources | `LOSTARK_RESOURCE_ROOT` = 선택한 Resources |
| Client 서버 주소 | `LOSTARK_SERVER_HOST` = `192.168.0.14` |
| Server EXE / 작업 폴더 | v5의 `Server/Bin/Release/Server.exe` / `Server/Default` |
| Server 인자 | `--bind-address 0.0.0.0` |

같은 PC에서 4인 검증을 할 때는 서버 PC에서 `ServerHost.cmd`를 한 번,
`LostArk.exe`를 네 번 실행한다. 같은 PC도 `192.168.0.14`를 사용한다.
실행기는 시스템 환경 변수를 영구 변경하지 않고 자식 프로세스에 필요한 경로를 전달한다.

Client/Server를 시작하지 않고 경로와 파일만 확인하려면, v5를 푼 폴더에서 다음을 실행한다.
외부 LostArk 경로는 해당 PC의 실제 경로로 바꾼다.

```powershell
Start-Process -FilePath .\LostArk.exe -ArgumentList '--check "C:\Users\user\Desktop\LostArk" "preflight.json"' -WindowStyle Hidden -Wait -PassThru
Get-Content -LiteralPath .\preflight.json
```

`status=PASS`, `clientStarted=false`, `serverStarted=false`와 실제 EXE·Data·Resources 경로를 확인한다.
검사 receipt는 지정한 파일에만 기록한다.

## 4. 현재 게시된 Pattern Flow

`P`는 패턴, `B`는 여러 패턴을 묶은 번들이다. v5에 포함된 현재 게시 데이터 기준이다.

| 관문 | 순서 |
|---|---|
| 1관문 | P1 → P2 → P6 → P7 → P47 → P48 → P58 → P78 → P79 → P80 → P81 → P82 → P83 |
| 2관문 | B1 → B2 → B3 → B6 → B7 → P21 → B10 → B9 → P27 → P85 → P86 → P87 → B4 → P25 |
| 3관문 | P88 → P91 → P92 → P93 → P52 → P46 → P66 → P76 → P35 |

2관문 번들은 B1=P8+P9, B2=P10+P11, B3=P12+P13, B6=P15, B7=P17, B10=P23, B9=P24, B4=P28이다.

- **1관문:** P29 내려치기와 P30은 Flow에서 제외했다. 패턴 정의는 보존했다.
- **2관문:** B4/P28 미로 진입부터 실제 복귀 완료까지 대기하고, 그 뒤 P25 쿠크 피자를 시작한다. 망원경 조작 전 대기도 포함한다. 피자 후에는 Idle로 기다린다.
- **3관문:** 마리오 부모 P88/P91/P92/P93이 각각 stage 1/2/3/4를 실행한다. 각 부모의 1페이즈 뒤 P33 2페이즈 무력화, 성공 시 P42 후속까지 끝나고 다음 부모로 진행한다. 무력화 실패·시간 초과는 기존 전멸 조건이다.
- **마리오 4회 이후:** P52 분신소환 기분나빠 → P46 백스텝 후 감전빔 → P66 분신소환 3갈래 불뿜기 → P76 쇼타임 연출 → P35 쇼타임 → Idle.

4인 마리오는 입장 후 남은 파티의 2페이즈를 진행한다. 각 참가자의 복귀까지 기다리는 별도 Flow 조건은 없다.
일반 행 사이 대기는 1초이고, P76→P35와 관문 마지막 행은 0초다.
Flow 종료만으로 보스를 처치하지 않는다. 사용자가 보스를 처치한 뒤 기존 클리어·전원 투표로 다음 관문을 진행한다.

## 5. 오류 발생 시 로그 수집

문제가 발생한 시각, 관문·패턴, 문제가 난 Client를 기록한다.
문제가 난 Client PC와 Server PC 각각에서 **실제로 실행한 v5 폴더**를 대상으로 수집한다.
아래 명령은 v5를 푼 폴더에서 실행한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Collect-RuntimeDiagnostics.ps1 -RuntimeRoot .
```

결과는 해당 v5 폴더의 `out/DiagnosticBundles/<시각>-<ID>.zip`과 같은 이름의 폴더에 생성된다.
`-RuntimeRoot`를 생략하면 수집기 파일이 있는 폴더를 사용한다. 기본 구성은 Release다.
수집기만 다른 PC에 복사해도 실행할 수 있으며, 기존 저장소를 대상으로 할 때는 다음처럼 지정한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Collect-RuntimeDiagnostics.ps1 -RepositoryRoot "C:\Users\user\Desktop\LostArk" -Configuration Release
```

수집기는 Client/Server를 조작하거나 파일을 업로드하지 않고 다음 기록의 로컬 사본을 만든다.

| 항목 | 수집 내용 |
|---|---|
| Client session | `Client/Bin/Release/Diagnostics/client-session-*.jsonl` 최신 4개와 `.previous` |
| Server session | `Server/Bin/Release/Diagnostics/server-session-*.jsonl` 최신 4개와 `.previous` |
| Server send progress | `Server/Bin/Release/Diagnostics/server-send-progress-*.jsonl` 최신 4개와 `.previous` |
| Server room perf | `Server/Bin/Release/Diagnostics/server-room-perf-*.log` 최신 4개와 `.previous` |
| Client 일반 로그 | `Client/Default`의 Startup·Exit·EffectFailure·RendererExit 로그와 존재하는 `.previous` |
| 실행 식별 | Client.exe·Engine.dll·Server.exe의 경로·크기·SHA256, Client/Server PID·실행 경로·시작 시각 |
| 배포 식별 | 존재하는 `bundle-manifest.json`, TeamLanEndpoint, 기존 설치 receipt·manifest |

`identity.json`의 hash는 디스크 파일 기준이며 실행 중 메모리 이미지의 hash가 아니다.
`collection.json`에는 수집 파일 hash와 수집 실패가 기록된다. Resources·자격 증명·환경 변수 전체는 수집하지 않는다.

`Server entry failed.`는 공통 복귀 문구다. `lobby.recovery.presented`와 `recovery.reported`,
`terminalReason/terminalDetail`, `source`, `HRESULT`, `wsaError`, 시각·PID를 함께 확인한다.
Client와 Server 로그를 같은 시각으로 비교해야 원격 PC의 연결 종료 원인을 구분할 수 있다.
컷씬 배경은 `kouku.cinematic.transition`과 `kouku.cinematic.failed`를 함께 확인한다.
v5 실행기 자체의 실패는 오류창 또는 `--check` receipt에 표시하며 별도 LauncherLogs 파일은 만들지 않는다.

## 6. 검증 상태와 이전 배포본

최종 Release Product 빌드와 Client/Server/Engine hash 일치, ZIP CRC·전체 manifest 파일 hash,
Resources/PNG/ChangedData 0개, V2 JSON 324개와 초기 필수 UI JSON 포함 검증을 통과했다.
완성된 v5 폴더의 `--check`도 실제 외부 Resources 경로와 bundle EXE·Data·작업 폴더를 확인했다.
Client/Server는 이 검증 중 실행하지 않았다.

별도 사전검사에서는 한글·공백 경로, `Framework.sln` 없는 폴더, 독립 진단 수집기와 실제
Client 경로 해석 코드를 확인했다. 실제 화면·음향·4인 재현과 장시간 접속 안정성은 사용자 검증이 남아 있다.

| 근거 | 파일 |
|---|---|
| 최종 Release 빌드 | `out/BuildPipeline/runs/20260918T205656146Z-release-product.json` |
| 최종 EXE/DLL pin | `out/BernDisconnect20260919/release-ready.receipt.json` |
| ZIP 생성 결과 | `out/BernDisconnect20260919/portable-delivery.receipt.json` |
| 독립 ZIP 재검증 | `out/BernDisconnect20260919/portable-final-verification.receipt.json` |
| 실제 bundle 사전검사 | `out/BernDisconnect20260919/v5-portable.preflight.json` |

기존 v4는 저장소에 설치하는 별도 배포본으로 보존한다. v4에는 G 키캡 PNG 1개가 포함됐으나 v5는 모든 PNG를 제외한다.
v4에서 완료한 Release 빌드·Sequence 65 게시·미로 대기·이펙트 예산·Billboard·진단 검증을 v5 최종 검증으로 대신 기록하지 않는다.

| 기존 파일 | 값 |
|---|---|
| v4 ZIP | `LostArk-Release-20260919-Flow-v4-192.168.0.14.zip` |
| 크기 | 99,175,950 bytes |
| SHA256 | `51cbc99f7ad4849c7eccd209268fb38a8e71e8ea590284744c54936f416cf1d0` |

이 문서 갱신은 기존 v4 ZIP 내용과 SHA256을 변경하지 않는다.
