# 2026-09-14 탈것 스킬 RESULT

대응 PLAN: `2026-09-14_VEHICLE_SKILLS_PLAN.md`

## 구현 상태

| 항목 | 상태 |
|---|---|
| 원본 스킬 사실 `Tools/VehiclePipeline/VehicleSkills.spec.json` (랩터 제외 후 6종 21스킬) | 완료 |
| 루트 이동 bake·catalog/profile v2 기록 `build_vehicle_skills.py` | 완료 |
| 탑승자 애니셋 28개 스킬 클립 재쿠킹 | 완료 (Resources, Drive 전달 필요) |
| bootstrap v2 `VEHICLESKILL` publish | 완료 (랩터 제외 후 6 vehicles, 21 skills) |
| Shared protocol 85 `PLAYER_ACTION_STATE::VEHICLE_SKILL` (main의 Mario 84와 병합하며 85로 올림) | 완료 |
| Server 쿨타임·이동 잠금·루트 이동(navigation clamp·collision)·하차 시 종료 | 완료 |
| Client Space/Q/W/E 요청, R 하차 | 완료 |
| Client 탑승자·탈것 클립 체인 seek, 종료 시 locomotion 복귀 | 완료 |
| 파티클·사운드·랩터 버프·HUD | 범위 밖 (후속) |

## 스킬 표

| 탈것 | SPACE | Q | W | E |
|---|---|---|---|---|
| 6705 황금 테르페이온 | 96030 대시 9.89m | 96000 | 96010 | 96020 |
| 9370 고요한 별빛의 가호 | 98380 대시 10.0m | 98381 | 98382 | 98383 |
| 7209 레인보우 모코보드 | 95722 대시 5.0m | - | - | - |
| 8302 아우프슈텐-R | 97300 대시 9.55m | 97310 | 97320 | 97330 |
| 8906 바다 유니콘 튜브 | 97730 10.0m | 97731 | 97732 | 97733 |
| 9524 고대의 신화 | 98520 대시 10.0m | 98521 | 98522 | 98523 |

쿨타임은 `EFTable_Skill.Cooltime`, 행동 길이는 원본 Action `LookInfoAnim` 클립 합이다.

## 자동 검증

- Debug Product 빌드 PASS (`out/BuildPipeline/runs/20260914T122553095Z-debug-product.json`).
- `Server.exe --vehicle-riding-contract-test` → `vehicle riding failures: 0` (스킬 시작·쿨타임·거부·길이 종료·대시 거리·하차 종료·재탑승 포함).
- `Publish-VehicleProfiles.ps1` Validate/Publish PASS.
- `git diff --check` 통과.
- `NetworkProtocolHarness`는 main에서 이미 protocol 82를 pin하고 있어(당시 main 83) 이번 84 변경으로 갱신하지 않았다. 기존 불일치다.

## 첫 실행 결함과 수정

- 증상: Space는 멈췄다가 순간이동, Q/W/E는 정지 자세.
- 원인: `Is_Valid_PlayerSnapshot`의 탑승 조건이 `eAction == NONE`만 허용해 스킬 중 snapshot을 Client가 전부 거부했다. Server 계약 테스트는 직렬화 검증을 거치지 않아 잡지 못했다.
- 수정: 탑승 조건에 `VEHICLE_SKILL` 허용, `ClientReplication`의 local move handoff에서 `VEHICLE_SKILL`을 스킬로 취급.

## 사용자 확인

사용자 확인: "잘 작동한다" (2026-09-14). 확인 경로는 Bern 또는 Character Select에서 F1 `Vehicle Riding (Debug)`로 탈것을 고르고 H로 탑승한 뒤:

- Space: 전진 거리·벽/비보행 지역 정지, 쿨타임 동안 재입력 무시
- Q/W/E: 탑승자·탈것 클립 체인, 재생 중 우클릭 이동 무시, 종료 후 idle/run 복귀
- R: 하차 (스킬 중 하차 시 스킬 종료)

## 은색 전투 랩터 제외

사용자 결정: 원작 Q는 누르는 동안 마우스 방향 가속으로 추정되며 외형도 마음에 들지 않아 탈것 자체를 제외한다. `VehicleCatalog.json`, `VehicleProfiles.json`, `VehicleSkills.spec.json`의 7104 항목, `CharacterCatalog.json`의 `*_RideRaptorAnimSet.wmodel` 4행, `Tools/VehiclePipeline/SilverBattleRaptor.texture-map.json`을 제거했다. 재publish 결과 6 vehicles, 21 skills이며 계약 테스트 실패 0. `Character/Vehicle/SilverBattleRaptor/`와 RideRaptor 애니셋 리소스 파일은 로컬 Resources에 남아 있으나 더 이상 참조되지 않는다.

## 남은 경계

- 쿠크 생성물 7개(`Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.*`, KoukuSaydon patternbindings/encounter)는 이 작업과 무관하며 커밋에서 제외한다.
- `Character/<Class>/AnimSets/*_Ride*AnimSet.wmodel` 28개는 재쿠킹됐으므로 Drive 전달이 필요하다.
