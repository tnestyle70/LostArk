# LostArk Release ZIP 안내

2026-09-19 v4 기준. 쿠크 1~3관문을 Release 4인 환경에서 검증하기 위한 배포본이다.
빌드된 Client/Server와 게시 데이터를 기존 LostArk 폴더에 설치한다.

## 1. 배포 파일

| 항목 | 값 |
|---|---|
| ZIP | `LostArk-Release-20260919-Flow-v4-192.168.0.14.zip` |
| 위치 | `C:\Users\user\Desktop\LostArk\Release` |
| 크기 | 99,175,950 bytes, 약 99.2 MB |
| 구성 | Release x64 |
| 서버 접속 주소 | `192.168.0.14:7777` |
| 서버 bind | `0.0.0.0:7777` |
| 데이터 | Action revision **1753**, Sequence revision **65**, protocol **93** |
| UI 통합 | PR **#413** 포함 |

[ZIP 열기](C:/Users/user/Desktop/LostArk/Release/LostArk-Release-20260919-Flow-v4-192.168.0.14.zip)

SHA256:

```text
51cbc99f7ad4849c7eccd209268fb38a8e71e8ea590284744c54936f416cf1d0
```

파일 확인 명령:

```powershell
Get-FileHash -Algorithm SHA256 -LiteralPath "C:\Users\user\Desktop\LostArk\Release\LostArk-Release-20260919-Flow-v4-192.168.0.14.zip"
```

## 2. 설치 전 준비

- `Framework.sln`이 있는 기존 LostArk 폴더가 필요하다. 현재 PC는 `C:\Users\user\Desktop\LostArk`다.
- 팀 Drive의 최신 리소스를 `Client\Bin\Resources`에 준비한다. 모델·텍스처·사운드 전체는 이 ZIP에 포함되지 않는다.
- 이번 ZIP에 추가로 포함한 리소스는 `UI/Interact/Key_G.png` 한 장이다.
- 새 사운드 18개의 상대 경로는 ZIP의 `NEW_SOUND_PATHS.txt`에서 확인한다.
- 편집 내용을 저장하고, 설치할 폴더의 Client와 Server를 종료한 뒤 설치한다. 설치기는 해당 실행 파일과 Data/DataFiles를 배포본으로 교체한다.

설치 전에 각 PC의 코드 빌드를 요구하지 않는다. ZIP을 받은 것만으로 Visual Studio의 다음 빌드까지 생략되는 것은 아니다.

## 3. 설치와 실행

### 일반 실행

1. ZIP 전체를 압축 해제한다. 압축 파일 안에서 EXE만 직접 실행하지 않는다.
2. 압축을 푼 폴더의 `LostArk.exe`를 실행한다.
3. 설치 대상으로 `Framework.sln`이 있는 기존 LostArk 폴더를 선택한다.
4. 실행 파일과 데이터 검증·백업·설치를 마치면 Release Client가 시작된다. 설치 과정에서 빌드는 하지 않는다.
5. 서버 PC도 같은 배포본을 설치하고 새 Server를 실행한다. 다른 PC는 Client만 실행한다.

설치 전용 `Install-Runtime.cmd` 또는 아래 PowerShell 설치 명령을 사용하면
압축을 푼 폴더의 `Runtime` 안에 다음 바로가기를 만든다.
`LostArk.exe` 방식은 바로가기 생성을 생략하고 설치 후 Client를 직접 실행한다.

| 바로가기 | 용도 |
|---|---|
| `Client (no build).lnk` | 설치된 Release Client 실행. 추가 빌드와 재설치를 하지 않는다. |
| `Server (host PC only).lnk` | 호스트 PC의 Release Server 실행. 작업 폴더는 `Server/Default`, 인자는 `--bind-address 0.0.0.0`이다. |

반복 테스트에는 위 바로가기를 사용한다. `LostArk.exe`는 설치 절차도 수행하므로 이후 로컬에서 수정·빌드한 파일을 다시 배포본으로 교체할 수 있다.
Visual Studio의 F5/Ctrl+F5는 설정에 따라 빌드를 수행한다.

### 같은 PC에서 Server 1개 + Client 4개 검증

앱 자동 실행 없이 먼저 설치만 하려면 다음 순서를 사용한다.
아래 예시는 ZIP을 `C:\Users\user\Desktop\LostArk\Release\Flow-v4`에 풀었다는 기준이다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File "C:\Users\user\Desktop\LostArk\Release\Flow-v4\Runtime\Install-Runtime.ps1" -RepositoryRoot "C:\Users\user\Desktop\LostArk"
```

1. 설치 확인창에서 대상 폴더와 변경 내용을 확인한다.
2. `Runtime\Server (host PC only).lnk`를 한 번 실행한다.
3. `Runtime\Client (no build).lnk`를 네 번 실행한다.
4. 네 Client로 같은 파티를 구성해 관문을 검증한다.

같은 PC에서도 이번 패키지의 접속 주소 `192.168.0.14`를 그대로 사용한다.
서버 메모리의 데이터는 파일 교체만으로 갱신되지 않으므로 설치 후 새 Server로 시작해야 한다.

설치 변경 파일의 백업은 기존 LostArk의 `out\RuntimeDeliveryBackups`에 저장된다.
설치 중 파일 변경이나 실패를 감지하면 작업을 중단하고, 이번에 교체한 파일을 복구한다. 동시 편집된 파일은 보존한다.

## 4. 적용된 Pattern Flow

`P`는 패턴, `B`는 여러 패턴을 묶은 번들이다. 전체 정의·게시 목록은 ZIP의 `PATTERN_FLOW_전체목록.md`를 확인한다.

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

## 5. 이번 ZIP에 반영한 수정

- Release에서 누락됐던 쿠크 lifecycle 알림 소비를 연결해 메시지 큐가 계속 쌓이는 결함을 수정했다.
- 미로 입장 직후 대기 상태를 놓쳐 피자가 먼저 소모되던 서버 Flow 판정을 수정했다.
- 팝업북 Sequence P1/P4의 무대가 카메라보다 2.852초 먼저 사라지는 12개 duration 필드를 수정·게시했다.
- 컷씬 중 제품 UI/HUD와 입력을 숨기고 종료·취소·실패 시 기존 창 표시 상태를 복원한다.
- 쿠크의 정상 4인 스킬 중첩·회전 카드·진입 조명을 수용하도록 이펙트 예산을 수정했다.
- 베른 폭포 이펙트에서 비균일 배치 크기와 회전의 조합을 잘못된 행렬로 처리하던 오류를 수정했다.
- 창 위쪽의 흰색 `파일 / 도움말` 메뉴바를 제거했다.
- UI PR #413의 이름표·팝업·ESC·파티 광기 게이지·쿠크 에스더·G 키캡 변경을 통합했다.
- Release 연결 실패, Lobby 복귀, 이펙트 실패, 서버 처리 지연의 진단 기록을 보강했다.

이펙트 예산은 무제한으로 풀지 않았다. 쿠크 아레나의 particles/trail/light hard 한도는 각각 **49,152 / 32,768 / 160**이다.
4인 × 동일 스킬 2회 잔상 중첩, 카드 42개, 도입 조명을 포함한 조합으로 검사했으며 다른 Level의 기존 한도는 유지했다.
전체 축과 현재 검증 근거는 `EFFECT_BUDGET_현재검증.md`, 이전 한도 기록은 `EFFECT_BUDGET_변경전전체조사.md`에 있다.

## 6. 오류 발생 시 로그 수집

문제가 발생한 시각, 관문·패턴, 문제가 난 Client를 기록하고 아래 수집기를 실행한다.
실행 중인 Client/Server를 자동 종료하거나 로그를 업로드하지 않고, 해당 PC의 로그 사본만 ZIP으로 만든다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File "C:\Users\user\Desktop\LostArk\Release\Flow-v4\Collect-Diagnostics.ps1" -RepositoryRoot "C:\Users\user\Desktop\LostArk"
```

결과는 `C:\Users\user\Desktop\LostArk\out\DiagnosticBundles`에 생성된다.
여러 PC라면 문제가 난 Client PC와 Server PC에서 각각 수집한다.
수집기는 Client session, Server session, Server room-perf 종류별 최신 4개 파일과 회전본을 포함한다.

아래 경로는 설치한 LostArk 폴더 기준이다.

| 로그 | 확인 내용 |
|---|---|
| `Client/Bin/Release/Diagnostics/client-session-<PID>.jsonl` | 연결·입장·최초 terminal 원인, 큐 깊이, 서버 tick, 메인 처리 지연, 메모리, 컷씬 상태 |
| 같은 파일의 `.previous` | 8MiB 순환 전 기록. 현재 파일과 함께 확인 |
| `Client/Default/EffectFailure.user.log` | 이펙트 ID·발생 위치, 준비/생성/렌더 실패, 요청량과 예산 |
| `Client/Default/RendererExit.user.log` | 렌더러 실패 |
| `Client/Default/ClientStartup.user.log`, `ClientExit.user.log` | 초기화·종료 실패 |
| `Server/Bin/Release/Diagnostics/server-session-<PID>.jsonl` | 연결 종료 사유, 소켓 오류, 송신 큐·부분 송신 정보 |
| `Server/Bin/Release/Diagnostics/server-room-perf-<PID>.log` 및 `.previous` | 서버 tick 지연, 관문·Flow·패턴, 객체 수, 송신 상태 |

`Server entry failed.`는 공통 복귀 문구다. `lobby.recovery.presented`와 `recovery.reported`를 찾고,
`terminalReason/terminalDetail`, `source`, `HRESULT`, `wsaError`, 시각·PID를 함께 확인한다.
뒤에 발생한 정리 오류가 최초 terminal 원인을 덮어쓰지 않도록 기록한다.
`connection.heartbeat`는 연결 중 60초마다 기록하고, `main-pump.stall`은 1초 이상의 처리 공백을 기록한다.
컷씬 배경 문제는 `kouku.cinematic.transition`과 `kouku.cinematic.failed`를 같은 시각으로 대조한다.

실행기 자체가 실패하면 `%LOCALAPPDATA%\LostArk\LauncherLogs`도 별도로 확인한다. 이 폴더는 위 수집기의 수집 범위에 포함되지 않는다.

## 7. 검증 상태와 남은 확인

| 확인 항목 | 상태 |
|---|---|
| UI PR #413 통합 후 최종 Release 제품 빌드 | PASS, 컴파일·링크 오류 없음 |
| Sequence 65 게시와 Server bootstrap 3관문 revision 일치 | PASS |
| 실제 서버 함수의 미로 대기·복귀 후 P25 시작 | 27/27 PASS |
| 실제 이펙트 예산 admission·경계값 | 125개 PASS |
| Billboard 행렬 및 기존 TRS 결과 보존 | 230개 PASS |
| Release JSON 진단·최초 원인 보존·파일 순환 | PASS |
| ZIP CRC/SHA256, 최종 Client/Server EXE hash, 설치 사전검증 | PASS |
| 실제 4인 화면·음향·이펙트와 장시간 접속 안정성 | 사용자 재검증 필요 |

이번 v4 수정 이후의 Debug 전체 빌드는 수행하지 않았다. 이 문서와 ZIP은 Release 기준이다.
확인한 배경 무대 공백을 수정했지만, 모든 검은 화면이나 모든 연결 종료를 해결했다고 판정한 것은 아니다.
이전 로컬 4Client 로그에서는 이펙트 예산 거부가 확인됐고 같은 실행의 서버 송신 실패는 0이었다.
와이파이나 클라우드 위치가 원인이라고 단정하지 않고 새 진단 로그로 재현 시점을 구분한다.

배포 구성은 runtime 458개, 직접 소비 Data JSON 1,193개, G 키캡 이미지 1개다.
EffectCatalog 참조 JSON 1,167개는 직접 소비 Data 수에 포함된다.
이 문서는 ZIP 옆의 안내 파일이며 ZIP 자체의 내용과 SHA256은 변경하지 않았다.
