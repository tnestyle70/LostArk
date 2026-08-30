# Runtime 빌드 ZIP 전달 가이드

## 경계

팀 Git은 코드와 `Data` 저작 정본을 관리하고, 팀장 Drive는
`Client/Bin/Resources` 물리 리소스를 관리한다. Runtime ZIP은 이미 빌드·publish된 EXE, DLL,
CSO와 필요한 `Client/Server Bin/DataFiles`만 전달한다. Resource 전체를 ZIP, manifest 또는 Git
정본으로 승격하지 않는다.

## 보내는 PC

먼저 같은 commit에서 필요한 Product/Core 빌드를 통과시킨다. 그 뒤 다음처럼 ZIP을 만든다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ResourceDelivery/New-LostArkRuntimeDelivery.ps1 `
  -Configuration Debug `
  -RepositoryRoot C:\Users\user\Desktop\LostArk `
  -OutputZip C:\전달\LostArk-Debug-Runtime.zip `
  -IncludePdb
```

ZIP에는 `Client/Bin/Resources`가 0개여야 한다. manifest의 per-file hash는 전송 중 손상과
경로 변조를 설치 전에 거부하기 위한 것이며, Resource pack version/lock 계약이 아니다.

## 받는 PC

1. Git에서 보내는 PC와 같은 commit을 checkout한다.
2. 팀장 Drive의 `Client/Bin/Resources`를 원래 상대 경로에 둔다.
3. 아래 설치 스크립트에 ZIP과 실제 LostArk 물리 폴더를 전달한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ResourceDelivery/Install-LostArkRuntimeDelivery.ps1 `
  -PackagePath C:\받은파일\LostArk-Debug-Runtime.zip `
  -RepositoryRoot C:\Users\user\Desktop\LostArk
```

설치기는 허용된 `Client/Server Bin` 경로만 받고, 모든 크기/hash를 먼저 검증한 다음 적용한다.
실패하면 이번 실행에서 바꾼 파일을 복구한다. `Client/Bin/Resources`는 읽거나 덮어쓰지 않는다.

로컬 Drive 경계는 다음으로 확인한다.

```powershell
python Tools/ResourceDelivery/validate_resource_delivery_policy.py --require-local
```

## 실행 확인

Debug 저작 기능은 Server + Client profile에서 사용자가 직접 확인한다. Release에서는 F1/Workbench가
노출되지 않으므로 제품 Lobby/Level 진입만 확인한다. 화면과 음향 fidelity는 자동 설치 결과가 아니라
사용자의 수동 smoke 판정이다.
