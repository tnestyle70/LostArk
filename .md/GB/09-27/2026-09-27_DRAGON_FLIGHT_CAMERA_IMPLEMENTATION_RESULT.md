# 베른·드래곤 비행 카메라 구현 결과

## G01. 반영한 소스·데이터

- 베른 자유 카메라 초기값을 map span과 무관하게 20 m/s로 설정했다. F1 Camera의 속도/20 reset은 Debug와 Release에서 노출되며 F6와 Shift 배수는 기존 계약을 따른다. Debug Valtan/Kouku의 기존 세션 속도 저장 경로는 보존했다.
- 드래곤 지상 탑승은 원래 follow 시점/FOV를 유지한다. 복제된 TAKEOFF/FLYING/LANDING에서만 비행 orbit를 적용하고 GROUNDED와 F6 free에서 해제한다. Debug Dragon은 거리 16 m, pitch 24도, look height 2.4 m 기본값 및 활성/heading-follow를 즉시 조절한다.
- Server FLYING 이동은 walkable XZ 제한을 제거하고 실제 고도 collision sweep을 사용한다. E 착륙에는 현재 XZ의 정확한 walkable 지면 및 충돌 없는 하강을 요구한다. 실패하면 비행을 유지한다. 착륙 중에도 매 tick 다시 검증하여 동적 바닥 붕괴·점유 시 현재 고도에서 FLYING으로 복귀하고, 낮은 deck 착지 취소 직후에도 고도가 순간 clamp되지 않는다. 강제 하차는 과거 안전 XYZ를 현재 nav/collision으로 재검증하고 같은 deck의 생존 지면 또는 유효 spawn으로 복구하며, 전부 없으면 기존 Server fall로 넘긴다. 일반 지상 이동과 다른 낙사 상태는 유지하고 유효한 비행 상태에만 walking void 검사를 건너뛴다.
- 고정 Y의 기존 body collision 경로는 유지했다. 수직 변위가 있는 body sweep은 Y slab와 XZ circle 시간 구간의 교집합으로 계산하여 같은 XZ의 수직 하강과 얇은 body 전체 통과도 차단한다. 기존 box sweep의 Y 범위 broadphase는 보수적 계약이며 exact 3D TOI 변경이 아니다.
- Part_Vehicle의 작은 snapshot 시각 차이는 frame마다 0.5~1.5배의 양수 시간 전진으로 흡수하여 뒤로 seek하지 않으며 반복 blend는 smoothstep으로 처리했다. part matrix에 이번 frame bob delta를 추가해 rider root와 한 frame 차이를 제거했다. flight loop 주기의 최대 0.16 m 목표 bob을 presentation root에 적용해 dragon, rider seat, bone effects가 함께 움직인다. Server 좌표에 bob을 더하지 않는다.
- 9523의 기본 속도는 지상 5→10 m/s, 수평 비행 8→16 m/s, 수직 4→8 m/s다. 원본 EFTable moveSpeed/source는 유지하고 `moveSpeedOverride`를 통해 지상 값만 프로젝트 튜닝한다. 공식 publisher로 Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap을 게시했다.
- Debug Dragon의 Save / Save + Publish는 기존 publisher의 Save/SaveAndPublish 모드다. 세 속도 field의 baseline 충돌, 최신 디스크 병합, 전체 검증, 입력 freshness, writer lock, atomic source/게시 교체, 실패 시 자기 source rollback을 연결했다. 서버 속도는 실시간 변경이 아니며 게시 후 Server 재시작이 필요하다.
- E 효과의 실제 날개·몸통 본 부착 plume 30개는 sourceScale count 0.8 / lifeTime 0.4 / alpha 0.6으로 조절했다. 원본 1~1.5초 plume을 0.4~0.6초로 줄여 여러 날갯짓 동안 구름이 누적되는 정도를 낮췄다. 원본 sourceRecipe, 크기/방향, 다른 12개 element와 ambient는 유지했다. GPU 시각 성공으로 기록하지 않는다.

## G02. 실행한 검증

- 팀 LAN sync: client,192.168.0.22:7777,not-listening. 로컬 설정 정상 완료.
- Vehicle publisher Validate/Publish: 7 vehicles /25 skills PASS.
- 격리 publisher fixture 10조건 PASS: Save, SaveAndPublish, 게시 값10/16/8, stale-field 거절, invalid ground/flight range, repository 탈출 거절, 다른 vehicle 의미 보존, 최신 unrelated edit 보존, 잠긴 bootstrap 실패 시 source rollback과 기존 게시 bytes 보존. `out/DragonCamera20260927/publisher-tests.json`.
- E 데이터 의미 비교: 변경은 정확히30개의 sourceScale 세 field뿐이며 sourceRecipe와 다른12개 element는 동일. `out/DragonCamera20260927/effect-structure.json`.
- Client project/filter XML parse 및 신규 TU 중복 없음, 변경 JSON parse, 작업 범위 git diff --check PASS.
- Server 기존 vehicle contract에 nav 범위 밖 이동, nav-gap 착륙 거절과 강제 해제 안전 XYZ 복귀 회귀조건을 추가했다. 초기 통합 Debug Product `20260926T204439420Z`와 초기 focused test exit0는 통합 담당이 확인했다. 이후 dynamic floor/same-deck fallback/수직 body/동일 frame bob 보강을 포함한 Debug Product `20260926T205710985Z-debug-product.json`와 Release Product `20260926T210031989Z-release-product.json`가 PASS다. 실제 Debug와 Release `Server.exe --vehicle-riding-contract-test`는 각각 56 assertions PASS, failures0이며 `out/BernValtan20260927/vehicle-final-debug.log`, `vehicle-final-release.log`를 직접 확인했다.

- 추가 focused 회귀조건: 낮은 벽 차단/같은 벽 위 통과, 같은 XZ 수직 body 하강 차단, endpoint가 body를 완전히 지난 큰 수직 sweep 차단, 실제 Valtan floor mutation에 의한 착륙 취소, 낮은 deck 취소 다음 tick 고도 보존, 붕괴된 last-safe의 같은 deck 재투영.
- 30/60/120/240fps 12초 동안 30Hz snapshot에 -70~+55ms jitter를 적용한 수치 clock 확인은 모두 단조 전진하며 frame 전진 0.5~1.5배 이내 PASS. 이는 화면 검증이 아닌 수식 수치 검증이다. `out/DragonCamera20260927/flight-clock-numerical.json`.

## G03. 남은 사용자 확인과 전달

Client/UI는 실행·조작하지 않았다. 새 Server/Client 빌드로 Bern 진입 후 F6 자유 카메라20 확인, 드래곤 탑승 상태의 기존 시점, E 이후 flight orbit/WASD/Space/Ctrl, nav 밖 이동과 착륙 거절/안전 위치 복귀, 날개·상하 움직임·plume 시각을 확인해야 한다. F1 Debug Dragon의 카메라 값은 즉시 반영되고 세 속도는 Save+Publish 후 Server 재시작으로 확인한다.

신규 Resources가 없어 GBResources에 추가한 파일은 없다. 효과 저작 JSON과 게시 bootstrap은 소스 전달에 포함된다. MainApp_Dragon.cpp는 기존 MainApp filter에 등록했다. 본 작업 외 기존 미커밋 RESULT와 다른 담당 파일을 되돌리지 않았다. 공용 AGENTS/CLAUDE/gotchas/restoration/team 계약 갱신은 통합 담당이 소유한다.

## G04. 최종 회귀 fixture 교정

발탄 collapse cell은 초기 intact wall navigation region과 겹쳐 기존 exact-walkable 사전조건이 실패했다. 실제 non-floor wall mutation의 navigation 조건을 먼저 적용하여 벽이 제거된 encounter 단계에서 착륙을 시작하도록 fixture를 교정했다. 착륙 runtime 검증을 완화하지 않았다. 초기 보강 실행의 wall/vertical-body 조건은 PASS였으며 fixture 교정 뒤 전체 focused 56조건과 dynamic floor/낮은 deck 다음 tick/생존 지면 복구가 PASS다.

## G05. 추가 넓은 회귀의 별도 경계

통합 담당이 추가 실행한 `--debug-teleport-contract-test`는 18 FAIL(Mario nearby attack exactly-once 14, 다른 F1 도구의 Mario mode 해제 4)을 보고했다. 이번 변경이 사용한 일반 body 차단/접선 이동과 드래곤 focused 56조건은 PASS다. 넓은 회귀의 실패 경로는 별도 Mario monster/도구 상태 경로이며 기준 브랜치 비교는 하지 않았으므로 기존 결함으로 확정하지 않는다. 로그는 `out/BernValtan20260927/collision-regression-debug.log`다.

## G06. 자유 카메라의 Debug 탑승 명령 제한

최종 읽기 검토에서 새 F1 Dragon Mount/Dismount가 직접 `Request_VehicleRiding -> Send_VehicleRidingRequest -> IPlayerCommandSink::Request_SetVehicleRiding`으로 전달되어 Level의 일반 gameplay camera gate를 우회함을 확인했다. `MainApp_Dragon.cpp`에서 실제 follow 활성, presentation override 없음, 유효한 비-preview player/controller를 공통 조건으로 사용해 버튼 disabled와 submit 두 곳을 제한했다. F6 free에서는 명령이 나가지 않으며 기존 Move Player 예외를 확대하지 않았다. 카메라·속도 저작 편집은 그대로 허용한다. typed sink와 Server 검증은 바꾸지 않았다.

최소 검증은 해당 TU를 제품 산출물과 분리한 out 폴더에서 Debug/Release 각각 컴파일하여 exit0를 확인했다. `out/DragonCamera20260927/free-camera-gate/compile-report.json`에 두 명령과 최종 소스 SHA256을 기록했다. shader 및 다른 제품 C++은 이 보강에서 수정하지 않았다.

이 F6 gate를 포함한 최종 통합 Debug Product `out/BuildPipeline/runs/20260926T215804868Z-debug-product.json`도 PASS이며, 전체 공식 DLL/CSO 배포 완료 후 확인했다.
