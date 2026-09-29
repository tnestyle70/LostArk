# 2026-09-29 베른 배 물살(웨이크) 이펙트 복원 RESULT

## 1. 원본 체인 (증명됨)

```text
배 vehicleId(8200~8208) -> LookInfo EFDLShip_<배>.loa (data4.lpk XmlData/LookInfo/Ship)
  -> CEFParticleData 블록(ParticleSystem + 본 B_EffectRoot + 1.0,1.0 + UE cm 오프셋)
  -> FX_CM_05.Par_S/L_WaterTrail_N_01 / _F_01, fx_cm_03.Water.Par_*_WaterTrail_side_01, FX_CM_05.SHIP.Magic.Par_N_Magic_WaterTrail_N_01/_F_02
  -> sprite 이미터(고정 spawn rate, balwaysinworldspace, 초기 속도 X -200 cm/s = 선미 방향)
```

배별 오프셋(UE cm, B_EffectRoot 기준): Estoc/Whitewind/Brahms (0,-25,20), Astray N/F (0,-15,20)+측면 (0,0,20),(0,50,20),
Barkstorm L_N (0,15,15)·D_side (0,-70,15)·L_F (0,15,15), Ghost (0,0,15), Tragon (0,-20,15), Pneuma N (0,-10,20)/F (0,-10,15), Luminous magic (0,-10,10).
LookInfo에는 속도 스위치가 없다. 영상에서 정지 시 물살 없음, 출항 후 뱃머리 물보라와 선미 흰 거품이 자라나는 것은 파티클이 월드 공간이라 배의 속도만큼 길어지는 결과다.

## 2. 복원 산출물

- 빌더: `Tools/EffectPipeline/build_ship_wake_source_effects.py` (기존 Kouku/Inanna/Vehicle 드라이버 재사용. LookInfo 블록을 실제 PlayParticleEffect 노티파이 헤더 뒤에 붙이는 합성 액션 방식)
- 문서 9개: `Data/Effects/Authored/effect.vehicle.ship.wake.<8200~8208>.run_battle_1.full.restore.effect.json` (요소 8/8/12/21/8/8/17/8/21 = 111개) + `Data/Effects/EffectCatalog.json` 9행
- native 셰이더 프로그램 4961~4968 (bucket 4928 파일 3개 + `Effect_ArtistMaterial_Tables.inl`만 변경, vcxproj/filters 무변경)
- 리소스: `Client/Bin/Resources/Effect/Vehicle/Ship/{Meshes,Textures}` 3개 파일, `CY_Resources`(바탕 화면)에 미러, 전달 스크립트 `Copy_ResourceDistribution_2026-09-29_ShipWake.ps1`
- 기존 Kouku/Warlord/Valtan 텍스처 17개를 재사용(모두 설치본에 존재 확인)

## 3. 연결(런타임)과 속도 반응

- `Data/Actors/VehicleCatalog.json` 8200~8208 각 행에 `ambientEffectCues`(MOUNT_END) 추가 (1287행부터). 기존 CEffectCatalog -> CEffectPresentationService -> CEffectObject 경로, `bVehicleModelAnchors`로 선체 본에 부착.
- `Client/Private/Character.cpp:1341~1344` ship 전용 게이트 호출, `:1347` `Update_ShipWakeGate`, `:3821` 탑승 시 상태 초기화. `Client/Public/Character.h:561,745` 멤버/선언.
- 움직임 판정은 기존 `m_isMoving`(0.15 s 지연 idle 포함). 정지하면 `Stop_VehicleOwner`로 물살을 즉시 제거하고 다음 출항에 다시 생성. 하차/레벨 이탈은 기존 MOUNT_END/`Stop_VehicleOwner` 경로. `isShip`만 대상이라 말·고대의 바다(9523)는 영향 없음.
- 속도 비례: 파티클이 월드 공간이므로 물살 길이가 배 속도에 비례한다. 별도 alpha 스케일 API는 없어 속도별 투명도 페이드는 넣지 않았다.
- 준비 규칙: 탑승 시 `Queue_VehicleSkillEffects`가 target을 미리 준비한다. 플레이 중 셰이더 컴파일·모델/DDS 로드 없음.

## 4. 요소 전수 검증

### 4.1 발견한 결함과 수정 (before / after)

측정: 9척 모두 `b_effectroot`는 모델 원점, 스케일 1, 회전 RotY(-90도) (행 (0,0,1),(0,1,0),(-1,0,0)). 선수는 모델 +X (돛대 좌표: mizen -89, main 4.5, fore +100).
LookInfo/이미터 숫자는 선체 프레임(X 선수, Y 위) 기준이라, 보정 없이 뼈 프레임에 얹으면 초기 속도 -X(선미)가 옆(모델 -Z) 방향이 된다. 월드 공간 파티클은 스폰 시점 루트 프레임으로 속도를 계산하므로 물살이 옆으로 튄다.

| 항목 | 이전(보정 없음) | 이후 |
|---|---|---|
| 소켓 회전 | (0,0,0) | (0,90,0) 전 요소 111개 |
| 선미 방향 속도 성분 (hull yaw 0/37/-120) | 저작 X와 불일치(대부분 0, 옆으로 이동) | 저작값과 1e-9 이내 일치, 불일치 0 |

### 4.2 요소 목록 (시스템별)

| 배 | 시스템 | 요소 수 | native 프로그램 | 수명(s) | spawn/s | 공간 |
|---|---|---|---|---|---|---|
| 8200 | par_s_watertrail_n_01 | 2 | 4961 | 0.5~0.7 | 23~23 | world |
| 8200 | par_s_watertrail_f_01 | 6 | 4961/4962/4963 | 0.4~2 | 7~23 | world |
| 8201 | par_s_watertrail_n_01 | 2 | 4961 | 0.5~0.7 | 23~23 | world |
| 8201 | par_s_watertrail_f_01 | 6 | 4961/4962/4963 | 0.4~2 | 7~23 | world |
| 8202 | par_s_watertrail_n_01 | 2 | 4961 | 0.5~0.7 | 23~23 | world |
| 8202 | fx_cm_03.water.par_s_watertrail_side_01 | 4 | 4964 | 0.6~0.8 | 6~6 | world |
| 8202 | par_s_watertrail_f_01 | 6 | 4961/4962/4963 | 0.4~2 | 7~23 | world |
| 8203 | par_l_watertrail_n_01 | 5 | 4961 | 0.4~0.8 | 0~23 | world |
| 8203 | fx_cm_03.water.par_d_watertrail_side_01 | 4 | 4962/4964 | 0.6~0.8 | 2~4 | world |
| 8203 | par_l_watertrail_f_01 | 12 | 4961/4962/4963 | 0.4~2 | 0~23 | world |
| 8204 | par_s_watertrail_n_01 | 2 | 4961 | 0.5~0.7 | 23~23 | world |
| 8204 | par_s_watertrail_f_01 | 6 | 4961/4962/4963 | 0.4~2 | 7~23 | world |
| 8205 | par_s_watertrail_n_01 | 2 | 4961 | 0.5~0.7 | 23~23 | world |
| 8205 | par_s_watertrail_f_01 | 6 | 4961/4962/4963 | 0.4~2 | 7~23 | world |
| 8206 | par_l_watertrail_n_01 | 5 | 4961 | 0.4~0.8 | 0~23 | world |
| 8206 | par_l_watertrail_f_01 | 12 | 4961/4962/4963 | 0.4~2 | 0~23 | world |
| 8207 | par_s_watertrail_n_01 | 2 | 4961 | 0.5~0.7 | 23~23 | world |
| 8207 | par_s_watertrail_f_01 | 6 | 4961/4962/4963 | 0.4~2 | 7~23 | world |
| 8208 | ship.magic.par_n_magic_watertrail_n_01 | 2 | 4961 | 0.5~0.7 | 23~23 | world |
| 8208 | ship.magic.par_n_magic_watertrail_f_02 | 19 | 4961/4962/4963/4965/4966/4967/4968 | 0.4~1.6 | 0~40 | world+local(5) |

공통: 111개 전부 표시 true, 부착 follow `b_effectroot`(EXACT_SOURCE_BONE), authoringApproximate false, 소켓 yaw 90.
모듈 집계: velocity 111, acceleration 72, colorscale 111, sizemultiplylife 109, locationprimitivecylinder 60, sphere 2, orbit 1, velocityoverlifetime 3, spawnperunit 4, parameterdynamic 4, cameraoffset 5. 속도 곡선/spawnperunit/orbit/parameterdynamic/cameraoffset은 8208(Luminous 마법) 요소에만 있다.
월드 공간(`localSpace=false`) 106개, 로컬 공간 5개.

### 4.3 속도별 물살 길이 (선체 프레임, 수식 계산)

시나리오: 정지 0, 순항 = moveSpeed x 2, 부스트 = 순항 x 3 (`VehicleProfiles.json projectTuning`). 길이 = (배속도 + 선미 초기속도) x 최장 수명.

| 배 | 순항 m/s | 부스트 m/s | 최장 요소 | 정지 | 순항 | 부스트 |
|---|---|---|---|---|---|---|
| 8200 | 4.00 | 12.00 | F_01.E11 (수명 2.0) | 0 | 10.0 | 26.0 |
| 8201 | 3.80 | 11.40 | 동일 | 0 | 9.6 | 24.8 |
| 8202 | 4.40 | 13.20 | 동일 | 0 | 10.8 | 28.4 |
| 8203 | 3.60 | 10.80 | L_F_01.E11 | 0 | 9.2 | 23.6 |
| 8204 | 3.66 | 10.98 | F_01.E11 | 0 | 9.3 | 24.0 |
| 8205 | 3.70 | 11.10 | 동일 | 0 | 9.4 | 24.2 |
| 8206 | 3.60 | 10.80 | L_F_01.E11 | 0 | 9.2 | 23.6 |
| 8207 | 4.20 | 12.60 | F_01.E11 | 0 | 10.4 | 27.2 |
| 8208 | 4.00 | 12.00 | magic F_02.E39 (수명 1.6) | 0 | 6.7 | 19.5 |

Estoc 선체 길이 약 4.9 m 기준 순항 물살은 약 2배, 영상(약 2.7배)보다 짧다. 원작 선박 속도가 더 빠르기 때문이며 프로젝트 배속 튜닝(x2)의 결과다. 스케일 억지 보정은 하지 않았다.

### 4.4 진단 로그

베른에서 로컬 플레이어가 배를 타면 `Client/Default/EffectFailure.user.log`에 채널 `ShipWake.Bern`으로 출발/정지 시점과 항해 중 약 2초마다 한 줄이 남는다(프로세스당 최대 240줄).
필드: `vehicle, state, speed_mps, pos, bow_yaw_deg, travel_yaw_deg, wake_aim_yaw_deg, aim_minus_travel_deg, hull_scale_x`.
선수 방향으로 항해 중이면 `aim_minus_travel_deg`가 약 +-180이어야 물살이 선미를 향한다. `hull_scale_x`는 0.01 근처여야 한다.

## 5. 미해결 항목과 정확한 이유

- 평면 메시 거품(`fm_e_planeup_001` + 재질 `fx_d_me_ocnfoam_01_01_ad`): 8200/8203/8208의 이미터 6개(배당 2개)는 네이티브 프로그램 4960이 "비unlit 엔진 CB prefix, world-position varying"으로 보류되어 문서에서 제외했다(pipeline deferred 목록). 입력 정확 복원이 별도 작업이다. 설치한 `fm_e_planeup_001.wmodel`은 아직 참조되지 않는다.
- 8203 Icebreaker 굴뚝 연기 `Par_L_Ship_Smoke_01/02`: LookInfo 블록에 본/오프셋 레이아웃이 없어 추측하지 않고 제외했다. 물살이 아니다.
- 원본 B_EffectRoot의 UE 뼈 프레임을 원본 psk로 직접 대조하지 않았다. 선체 프레임 해석은 이미터 속도 분포(측면 +-0.5~0.7, 위 0.35~0.5)와 영상의 선미 흐름으로 뒷받침되는 추론이다. 화면 확인 필요.
- 배 모델 `Get_BoneMatrix` 결합 스케일이 0.01이면 source bone 정규화 경로를 통과한다(루트 뼈에 PreTransform 0.01이 곱해지는 코드로 확인). 실제 생성 성공 여부는 실행 확인 필요.
- 정지 시 물살은 즉시 제거된다(점진 소멸 API 없음).
- 빌드/실행은 하지 않았다. 시각 판정(visual PASS)은 하지 않았다.

## 6. 검증 실행 내역

- 빌더 `ast.parse` OK, `--acquire`(9척), `--native`(8 프로그램, 소스 실패 0), `--install-native`, `--project --install` 성공
- 문서 9개에 대해 material color space / module overrides / attachment orientation / native sprite option 검증기 통과, 참조 리소스 누락 0
  (저장소 전체 `Validate-EffectSources.ps1`는 무관한 Kouku 문서 `blade-dance.circle.impact`에서 먼저 실패하므로 문서 단위로 실행)
- VehicleCatalog JSON parse OK (CRLF 유지), `git diff --check` 해당 파일 경고 없음, `.vcxproj/.filters` 무변경

## 7. 게임에서 확인할 순서

1. Server + Client 재빌드 후 베른 입장, 배 탑승(가장 쉬운 것: Estoc 8200).
2. 정지 상태: 물살 없음.
3. 클릭 이동 출항: 뱃머리 물보라, 선미 흰 거품 트레일이 생긴다. 트레일이 옆이 아니라 뒤로 흘러야 한다.
4. Space 쾌속 운항: 트레일이 약 2.5배로 길어진다.
5. 회전: 트레일이 곡선을 그린다. 다시 멈추면 사라진다.
6. 하차/섬 입항/레벨 이동 후 물살이 남지 않는다. 말/고대의 바다에는 물살이 없다.
7. `Client/Default/EffectFailure.user.log`에서 `ShipWake.Bern` 줄과 `aim_minus_travel_deg` 확인.
8. 다른 8201~8208 배도 같은 방식으로 확인.
