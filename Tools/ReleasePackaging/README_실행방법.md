# LostArk Release 2026-09-29 실행

1. 모든 PC에서 같은 ZIP 전체를 새 폴더에 압축 해제합니다. EXE 하나만 복사하지 마세요.
2. Drive에서 받은 최신 Resources를 준비합니다. 이 ZIP은 Resources를 포함하지 않습니다.
3. 서버 PC `192.168.0.22`에서 이전 Server를 종료하고 이 폴더의 `ServerHost.cmd`를 한 번 실행합니다.
4. 각 PC에서 `LostArk.exe`를 실행하고 최신 Resources가 있는 기존 LostArk 폴더 또는 Resources 폴더를 선택합니다.

선택한 기존 폴더에서는 Resources만 읽습니다. 실행 파일·Data·DataFiles는 압축 해제한 이 폴더를 사용하며 기존 저장소에 설치하거나 덮어쓰지 않습니다. Visual Studio 없이 실행하도록 필요한 VC 런타임 DLL도 포함합니다. Windows 10/11 x64 및 .NET Framework 4.x가 필요합니다.

F1 → Balance Test에서 Server 수치를 편집하고 `Save + Apply`를 누르면 Server가 검증·저장·게시 후
연결된 모든 플레이어에게 재시작 없이 반영합니다. Skills는 직업·슬롯·이름으로 찾고 ALT_V를 따로
고를 수 있습니다. 연결된 대미지 profile, 패턴 최대 HP 비율 피해, 패턴별 무력화 threshold도 구분합니다.
다른 플레이어가 먼저 저장하면 현재 draft는 보존되고 Server 값을 다시 읽어야 합니다.
플레이 중 수치를 조정하고 `SAVED AND APPLIED`를 확인한 뒤, 처음부터 다시 시험하려면 게임 안의
관문/레이드 재시작을 사용합니다(필요한 파티 승인 포함). EXE나 Server 프로세스를 껐다 켤 필요는 없습니다.
Server가 저장하는 Data·bootstrap·numeric receipt는 같은 폴더에 함께 보존하세요. 실행기는 이 영수증과
파일 hash를 검사하므로 일부 저장 파일만 이전 ZIP 파일로 덮어쓰지 마세요.

네 명은 같은 서버 `192.168.0.22:7777`로 접속합니다. 서버 PC의 Client 1명과 다른 PC의 Client 3명으로 입장하거나 한 PC에서 `LostArk.exe`를 네 번 실행할 수 있습니다. 다른 PC는 Server를 실행하지 않습니다.

실행하지 않고 파일과 경로를 검사하려면:

```powershell
.\LostArk.exe --check "C:\Users\user\Desktop\LostArk" .\preflight.json
```

발탄은 4인 입장·컷씬·체력별 패턴·부활과 유령 처치·클리어, 쿠크는 입장·1~3관문·마리오·빙고·최종 연출과 MVP를 사용자 화면에서 확인합니다. 구조화된 서버 검사와 ZIP 검증은 네 Client의 실제 화면·음향·LAN 품질 판정을 대신하지 않습니다.

실패한 Client PC와 서버 PC에서 각각 다음 명령으로 진단 기록을 수집할 수 있습니다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Collect-RuntimeDiagnostics.ps1 -RuntimeRoot .
```
