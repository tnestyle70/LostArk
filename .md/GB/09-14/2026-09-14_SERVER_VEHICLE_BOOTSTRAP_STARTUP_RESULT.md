# Server 차량 bootstrap 누락 복구 결과

## G00. 원인과 반영

`pattern3-rendering-restore`의 기존 Debug Server가 World=1 초기화 중
`Missing vehicle bootstrap`으로 종료됐다. 확인 시
`Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap`이 존재하지 않았다.

차량 정의 정본 `Data/Vehicles/VehicleProfiles.json`과 정식 게시기는 이미 있으며,
`vehicles.profiles`의 BuildDomains 및 Server owner 등록도 존재한다. 일반 Product Build는
runtime publisher를 기본 실행하지 않으므로 누락 파일을 명시적으로 생성했다.
이 PC에서 파일이 처음부터 생성되지 않았는지, 이전에 삭제됐는지는 확인하지 않았다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-VehicleProfiles.ps1 -Mode Publish
```

게시기는 schema, ID, 이동 속도와 원본 근거를 검증하고 차량 4종
`6705, 7104, 9370, 7209`를 모두 5 m/s로 게시했다. 생성 bootstrap은 Git 제외 runtime
파일이며 직접 편집하지 않았다. C++/HLSL/프로젝트/authoring JSON 변경 및 재빌드는 없다.

## G01. 실제 검증

- 기존 `Server/Bin/Debug/Server.exe --vehicle-riding-contract-test`: 16개 PASS,
  `vehicle riding failures: 0`, 종료 코드 0. Bern/Valtan 준비 및 탑승 계약 확인.
- 같은 EXE의 `--headless --bind-address 127.0.0.1 --port 17777 --smoke-timeout-ms 1000`:
  Bern, Valtan, Training, Kouku와 Character Select 초기화 뒤 listening 출력, 종료 코드 0.
  검증용 loopback/별도 포트이며 공유 LAN 설정은 변경하지 않았다.
- 실행 로그는 `out/VehicleBootstrapFix20260914/vehicle-contract.log`와
  `server-startup.log`에 보존했다.

## G02. 실행 인계

검증 Server는 자동 종료했으며 Client/UI는 실행하지 않았다. 사용자는 기존
`Server + Client` Visual Studio 시작 profile로 다시 실행하면 된다.
이번 복구에 C++/셰이더 재빌드는 필요하지 않다. F5/Ctrl+F5 자체는 Visual Studio 설정에
따라 Build를 수행할 수 있다. 다른 PC에 같은 누락이 있으면 위 차량 전용 게시 명령을
실행하거나 최초 데이터 준비용 `Invoke-BuildDomainOwner.ps1 -Owner Server`를 사용한다.
