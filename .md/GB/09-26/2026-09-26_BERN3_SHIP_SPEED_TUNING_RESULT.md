# 2026-09-26 베른 배 항해 속도·카메라 튜닝 결과

## 요청

1차: 배 이동 속도를 2배로 올리고, 탑승 중 스페이스로 그 속도의 5배로 움직이게 한다.
2차(같은 날 수정): 5배는 말이 안 되게 빠르니 **3배**로 줄이고, 카메라가 너무 멀어 배가 작아
보이니 더 당긴다. 사용자가 **20 m**를 지정했다.

최종 저장값은 평속 2배, 스페이스 3배, 카메라 20 m다.

## 왜 느렸는지

원본 `EFTable_VoyageShip.MoveSpeed`가 배 9종(8200~8208)에 180~220 cm/s를 주고 있었다.
divisor 100을 적용하면 1.8~2.2 m/s다. 같은 서버의 플레이어 걷기 속도는 2.8 m/s이므로
(`Server/Private/ServerGameplayContractTests_VehicleRiding.cpp`의 하차 후 기대값)
배가 걷는 것보다 느렸다. 원작 쾌속(`BoostSpeed`)을 더해도 2.6~3.2 m/s로 걷기 수준이다.

## 적용 방식

원본 표 값을 고쳐 쓰지 않는다. `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1`이
`moveSpeed == source.value / source.divisor`와
`boost.moveSpeed == (boost.source.baseValue + boost.source.value) / boost.source.divisor`를
정확히 대조하고 있으므로, 숫자를 직접 바꾸면 원본 근거가 훼손되거나 publisher가 거부한다.
그래서 튜닝 배수를 담는 optional 블록을 따로 뒀다.

`Data/Vehicles/VehicleProfiles.json` — 배 9종에만 추가:

```json
"projectTuning": {
  "moveSpeedMultiplier": 2.0,
  "boostSpeedMultiplierOfMoveSpeed": 3.0
}
```

publisher 계산:

- 게시 평속 = `moveSpeed × moveSpeedMultiplier`
- 게시 부스트 = 게시 평속 × `boostSpeedMultiplierOfMoveSpeed`
- 두 값 모두 소수 4자리로 반올림해 `VEHICLE` 행 3번째·11번째 필드로 싣는다.

검증은 유지·추가했다. 원본 `source`/`boost` 대조는 그대로 두고, 그 위에
배수 범위 1~10, 8200~8208 밖 거부, `boost` 없는 배 거부, 게시 평속 0 초과·30 m/s 이하,
게시 부스트가 게시 평속보다 빠르고 30 m/s 이하를 검사한다.

## 게시 결과

`Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap` 헤더는 `LOSTARK_VEHICLE_BOOTSTRAP 4 50`으로
세대·필드 수가 그대로다. 따라서 Server reader(`Server/Private/VehicleCatalog.cpp:164`의
version>=4 → 11 필드)는 변경 없이 읽는다.

| 배 | 원본 평속 | 게시 평속 | 스페이스(3배) | 1차 시도(5배) |
|---|---|---|---|---|
| 8200 | 2.0 | 4.0 | 12.0 | 20.0 |
| 8201 | 1.9 | 3.8 | 11.4 | 19.0 |
| 8202 | 2.2 | 4.4 | 13.2 | 22.0 |
| 8203 | 1.8 | 3.6 | 10.8 | 18.0 |
| 8204 | 1.83 | 3.66 | 10.98 | 18.3 |
| 8205 | 1.85 | 3.7 | 11.1 | 18.5 |
| 8206 | 1.8 | 3.6 | 10.8 | 18.0 |
| 8207 | 2.1 | 4.2 | 12.6 | 21.0 |
| 8208 | 2.0 | 4.0 | 12.0 | 20.0 |

단위는 m/s다. 걷기 2.8 m/s 기준으로 평속은 1.29~1.57배, 스페이스는 3.86~4.71배다.
1차의 5배는 걷기의 6.4~7.9배여서 사용자가 과하다고 판정했다.

## 카메라

`Data/Camera/Bern.camera.json`의 `shipCamera.distanceMeters`만 바꿨다. publish가 없는 문서이고
Level이 입장할 때 저장 profile을 읽으므로 재빌드도 게시도 필요 없다. 재입장으로 확인한다.

| 값 | 출처 | 유령선 8204 화면 높이 | 작은 배들 |
|---|---|---|---|
| 16 m | 베른 도보 카메라 | — | — |
| 17 m | 원작 `EFTable_CameraSetting` 1001/1 `ZoomDist` 1700 cm | 108% (잘림) | 42~56% |
| 40 m | 1차 PROJECT_AUTHORED | 46% | 18~24% |
| 28 m | 2차 중간값(잠깐 저장) | 약 66% | 26~34% |
| **20 m** | **사용자 지정, 현재값** | 약 92% | 36~48% |

유령선 화면 높이는 40 m에서 실측한 46%를 거리에 반비례로 환산한 값이다. 17 m에서 잘리는
기준이 108%였으므로 20 m의 92%는 프레임 안에 들어오지만 유령선만 빡빡하다.
원작보다 3 m 먼 값이고, 도보 카메라 16 m보다 4 m 멀다.
`provenance` 문자열에 원작 17 m와 20 m를 고른 이유를 함께 적어 출처가 흐려지지 않게 했다.

## 변경 파일

- `Data/Vehicles/VehicleProfiles.json` — 배 9종에 `projectTuning` 추가, boost 배수 3.0
- `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1` — optional 블록 파싱·검증·적용 (+2074 bytes)
- `Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap` — 재게시 2회, 세대 4 유지
- `Server/Private/ServerGameplayContractTests_VehicleRiding.cpp` — 탑승 기대값 2.0→4.0(8200),
  2.1→4.2(8207), `SHIP_EXPECTATION` 표에 `fBoostSpeed` 열과 게시 부스트 대조 한 줄 추가.
  ASCII·CRLF 유지
- `Data/Camera/Bern.camera.json` — `shipCamera.distanceMeters` 40 → 20, provenance 갱신
- `CLAUDE.md` — 탈것 문단의 `projectTuning` 계약, `shipFog` 문단의 카메라 거리 20m
- 같은 날 앞선 카메라·안개 RESULT 두 건에 현재값을 가리키는 추가 조정 주석

부스트 지속 시간과 쿨타임은 건드리지 않았다. 각 배의 SPACE `VEHICLESKILL` 행이 그대로
소유하며 현재 5000ms / 5000ms다. 원작 쾌속 게이지는 여전히 미구현이므로 제한은 쿨타임뿐이다.

## 실행한 검증

- `Publish-VehicleProfiles.ps1 -Mode Validate` → `16 vehicles, 34 skills` PASS
- `Publish-VehicleProfiles.ps1 -Mode Publish` 2회 → 성공, 위 표의 행 생성 확인
- JSON 재파싱 OK, CRLF·BOM 없음 유지 (`VehicleProfiles.json`, `Bern.camera.json`)
- Server 프로젝트만 MSBuild `/t:Build` Debug x64 3회 → 전부 exit 0,
  매번 `ServerGameplayContractTests_VehicleRiding.cpp` 1개만 재컴파일 후 링크 성공
- `Server.exe --vehicle-riding-contract-test` → **86 PASS / 실패 0**, exit 0.
  배 9종의 게시 평속 대조와 게시 부스트 대조가 각각 9번 들어 있다.
- 기대값 표를 고치기 전 실행에서는 원본 대조 검사가 9번 `[FAILURE]`로 잡혔고, 갱신 후 0이 됐다.
- 전체 `--contract-test` 스위트는 이 변경과 무관한 다른 도메인까지 오래 돌아서 중단했다.
  중단 시점까지 96 PASS, 실패 0이었다.

## 빌드 중 확인한 충돌

정본 Product 러너를 처음 돌렸다가 중단했다. 17:28:03에 이 저장소의 다른 에이전트(Codex)가
`Shader_SourceCharacterBaseGroup1472.hlsli`, `Shader_SourceCharacterLightGroup1472.hlsli`와
두 `*Programs.hlsli`를 새로 써 넣었고, 공용 include가 바뀌어 내 빌드가 그 미완성 셰이더를
함께 컴파일하기 시작했다. 내가 띄운 프로세스 트리(powershell 19676 하위의 MSBuild·Tracker·fxc)만
PID로 종료했고 다른 프로세스는 건드리지 않았다. 0바이트 CSO는 없었고, 중단 시점까지 완전히
기록된 CSO는 `Engine/Bin/Debug/Shader_Deferred.cso`(606839 bytes, 17:29:58) 하나다.
이 파일은 Codex의 1472 그룹 변경 결과이며 Client 쪽으로는 배포되지 않았으므로
`Engine/Bin/Debug`와 `Client/Bin/Debug`의 `Shader_Deferred.cso`가 다를 수 있다.
Codex 작업이 끝난 뒤 정상 Product 빌드를 한 번 돌리면 맞춰진다. 이 상태를 내가 되돌리지 않았다.

내 속도 변경은 Server 프로젝트 하나뿐이고 카메라 변경은 데이터뿐이므로 Server만 빌드해 확인했다.

## 사용자 확인이 남은 것

Server를 재시작한 뒤 베른에서 배에 타서 평속 체감, 스페이스 쾌속, 20 m 카메라 화면 크기를
직접 확인해야 한다. 게시본은 디스크에 있고 실행 중인 Server 메모리에는 반영되지 않는다.
카메라는 Client 재입장으로 반영된다.

조정 방법: 속도 배수는 `Data/Vehicles/VehicleProfiles.json`의 두 숫자를 고치고
`Publish-VehicleProfiles.ps1 -Mode Publish` 재실행. 단 계약 검사 기대값 표도 같이 맞추려면
Server 재빌드가 필요하다. 카메라는 `Data/Camera/Bern.camera.json`의 `distanceMeters` 한 숫자만
고치면 되고 빌드·게시가 없다.
