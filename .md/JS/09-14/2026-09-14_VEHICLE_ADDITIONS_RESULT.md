# 탈것 3종 추가 결과

작성일: 2026-09-14. 브랜치: `feature/terpeion-vehicle-riding`. 계획: [PLAN](2026-09-14_VEHICLE_ADDITIONS_PLAN.md).
로컬 커밋 기준 상태다.

## 구현 상태

| G | 내용 | 상태 |
|---|---|---|
| G01 | 추출기 normal 파라미터 29B, 생성기 clamp·환경 cube | 완료 |
| G02 | program 86·87·88 설치, `Model.cpp` 상한 88, 텍스처 맵 3개와 원본 TGA 24장 | 완료 |
| G03 | `SilverBattleRaptor`·`SereneStarlightBlessing`·`RainbowMokoboard` wmodel 쿠킹 | 완료 |
| G04 | 4직업 × Raptor/Swing/Hoverboard 탑승자 애니셋 12개, `CharacterCatalog.json` 등록 | 완료 |
| G05 | `VehicleProfiles.json` 4행, `VehicleCatalog.json` 3항목 | 완료 |
| G06 | 반투명 forward PS program 18/88 분기, pass 10 단면, `Part_Vehicle` pass 선택 | 완료 |
| G07 | F1 `Vehicle Riding (Debug)` 기본 탈것 선택, H 선택 로직 | 완료 |
| G08 | 문서, Product 빌드, 계약 테스트 | 완료 |

## 계획과 달라진 점

- **install 기준점:** 도구가 항상 `case 84u:` 뒤에 삽입해 순서가 뒤집히므로 `case {N-1}u:` 뒤에 넣도록 고쳤다.
- **재질 슬롯 이름:** `cook_npc.py`는 LookInfo 교체 MIC의 텍스처를 쓰되 슬롯 이름은 메시 기본값(`mn_isrx_02_mi`, `mn_isrx_00_mi`, `mn_pmsmk_00_mi`)을 유지한다.
  `rows` 명령에 `dump=family@슬롯이름`을 추가해 catalog `materialName`을 슬롯 이름으로 만들었다.
- **차원술사 호버보드 클립 이름:** WModel section 이름 40B 제한으로 `pc_sp_m_00_sk_ride_hoverboard_idle_norm` / `_run_norma`가 런타임 이름이다. catalog에 그대로 넣었다.
- **호버보드 탑승자 idle fps:** 원본 68f@29fps를 `build_npc_animset.py`가 30fps로 굽는다(루프 약 0.08s 짧음). 사용자 확인 항목.

## 자동 검증

| 검사 | 결과 |
|---|---|
| verify program 24 / 85 (생성기 보강 후) | base/light/configure EXACT |
| verify program 86 / 87 / 88 (설치 직후) | EXACT (base 828·706·495줄, light 456·701·393줄) |
| Engine·Client `Shader_SourceCharacter{Base,Light}Programs.hlsli` | SHA-256 동일 |
| 탈것 wmodel 3개 | validate OK, `b_cockpit`, `npc_idle_normal_1`·`npc_run_normal_1` 존재 (본 63/22/12) |
| 탑승자 애니셋 12개 `compare_attach.py` | 전부 OK, bone table·skeletonHash 일치(224/218/239/236) |
| rows | 랩터 45+40 파라미터·9+8 텍스처, 별빛 35+30·6+7, 모코보드 44·8 |
| VehicleCatalog | JSON parse, 항목 필드 10개, rows와 float32 동일, 모든 모델·텍스처 파일 존재, family 설치 확인 |
| `Publish-VehicleProfiles.ps1` Validate → Publish | `Vehicles.bootstrap` 4행 |
| `fxc /T fx_5_0 Shader_VtxAnimMeshBinary.hlsl` | 성공, 신규 경고 없음 |
| Product Debug 빌드 | Engine/Shared/Server/Client PASS. `out/BuildPipeline/runs/20260913T162354083Z-debug-product.json` |
| `Server.exe --vehicle-riding-contract-test` | `vehicle riding failures: 0` |
| `git diff --check` | 경고 0 |

## 실행 준비와 사용자 확인

로컬 Server(`--bind-address 127.0.0.1`)와 Client(`LOSTARK_SERVER_HOST=127.0.0.1`)를 새 빌드로 띄웠다. 화면 조작·캡처는 하지 않았다.

사용자 관찰: "일단 전체적으로 나오긴 다 나와 디테일 한건 내일 더 수정하기로". 아래 세부 항목은 다음 작업에서 조정한다.

1. F1 `Vehicle Riding (Debug)`에서 각 탈것 선택 → H: 탈것과 모드별 자세(랩터 앉기, 스윙, 모코보드 서기).
2. 이동 시 run 클립과 속도(5 m/s).
3. 랩터 재질, 별빛의 가호 파란 반투명 외피와 몸체, 모코보드 무지개 VFX.
4. 크기·좌석 높이, 호버보드 idle 루프 박자.
5. 직업 변경·발탄 입장 시 하차.

## 재질·좌석 보정 (09-14 오후, 브랜치 `feature/vehicle-material-restoration`)

사용자 관찰: 모코보드 무지개가 안 흐름, 발이 보드 아래로 내려감, 별빛 초승달이 안 보이다가 진한 파랑·투명으로 나옴.

| 증상 | 원인(실측) | 수정 |
|---|---|---|
| 모코보드 무지개 | 셰이더 time 경로 정상. `-3_vfx`(DXT5) RGB가 원본부터 흰색이라 흐름 무늬 없음. 최신 빌드 후 사용자 "잘 바뀌네" | 변경 없음 |
| 모코보드 발 위치 | `build_npc.py`가 메시를 master `MN_PMSHB_00`에 rebind → inverse bind `b_body_00` 19.41cm, 메시 ref 50cm. 메시만 약 30cm 떠 보임. 4직업 hoverboard 클립 발끝은 root 0~4cm | `master.selfRigged=true`(메시 PSK)로 FBX·WModel 재쿠킹, 트랙 25개 경고 0, bind 50/57.97cm, validate OK |
| 별빛 초승달 안 보임 | 88 opacity = diffuse α × cb0[0].w(엔진 행, 0) | Base88·Light88 `source[0].w = 1` |
| 진한 파랑 발광 | WModel 1.0에 UV1 없음, 88 panning이 `v4.zw` 샘플 | PSK EXTRAUVS0를 삼각형 join(`cook_psk_extra_uv1.py`, 13,581정점·14,510삼각형 모호 0)해 1.3 |
| 시선 따라 파랑·보라 그라데이션 | Base88이 v5=fog, v6=view, v7=up인데 기본 배치(v5=view) 적용 | `MakeSourceCharacterInput` program 18 배치 목록에 88 추가 |
| 초승달 속이 비침 | cb0[21].x==0 분기에서 α<0.9 픽셀이 0으로 깎임(외피 링 면적 29%) | `source[21].x = 1`(원본 값 미확인, 원작 화면 기준 추론) |
| 바깥벽만 진함 | sky light 행 cb0[18..20]이 0 | 반투명 PS가 scene ambient를 `lightColor`로 넘기고 Base88이 sky 행에 사용(근사) |

- 은색 전투 랩터 master rig 차이는 1cm 미만이라 재쿠킹하지 않았다.
- 88 verify는 이제 생성물과 byte 불일치다(엔진 행 3줄 추가).
- fxc 컴파일 성공, Product Debug 빌드 PASS(`out/BuildPipeline/runs/20260914T*-debug-product.json`), `cook_psk_extra_uv1.py`는 1.0 백업에서 설치본과 SHA-256 동일하게 재현.
- 사용자 관찰: 모코보드 "발판위에 잘 올라갔어", 별빛 최종 "잘 나와".
- 백업: `out/VehicleAdditions20260913/SereneStarlightBlessing.v1_0.wmodel`, `RainbowMokoboard.pmshb-rig.wmodel`.

## 탈것 2종 추가 (09-14 오후)

| 항목 | 아우프슈텐-R | 바다 유니콘 튜브 |
|---|---|---|
| EFTable_Vehicle | 8302 `MN_PMSHE_00-2`, RidingMode 13 `HEAVYWALKER_BM9` | 8906 `MN_PMSUT_00-6`, RidingMode 17 `TUBE` |
| 재질 | `mn_pmshe_00-2_mi` → 기존 program 85(Base/Light PS ID 일치) | `mn_pmsut_00-6_mi` → 기존 program 24 |
| 모델 | self-rigged, 본 63, 클립 9, validate OK | self-rigged, 본 12, 클립 14, validate OK |
| 탑승자 | `ride_heavywalker_bm9_idle/run_normal_1` (차원술사 `…_bm9_idle` / `…_bm9_run_`로 잘림) | `ride_tube_idle/run_normal_1` |

- RidingMode 번호→이름은 `NU1V7NCQ4YAE9ZPJVNOQS.u`의 `ActionConditionRidingModeOutput` enum을 직접 읽어 확정했다(기존 1/3/5/46과 일치).
- program 24/85는 `v4.zw`를 쓰지 않아 UV1 보강 불필요.
- 탑승자 애니셋 8개 `compare_attach.py` 전부 OK. 작업 스크립트 `out/VehicleAdditions20260914b/cook_riders.ps1`.
- `VehicleCatalog.json` 6종, `VehicleProfiles.json` 6행, `Publish-VehicleProfiles.ps1 -Mode Publish` 성공. C++·셰이더 변경 없음.
- 창술사 튜브 idle은 원본 약 28.7fps를 30fps로 구웠다.
- 사용자 관찰: "다 잘 나와".
- Drive 전달: `Character/Vehicle/{Aufstehen,SeaUnicornTube}/`, 4직업 `AnimSets/<Class>_Ride{HeavywalkerBm9,Tube}AnimSet.wmodel`.

## 다른 PC 준비

- Server PC는 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server`(또는 `Publish-VehicleProfiles.ps1 -Mode Publish`) 후 재시작한다.
- Resources(Drive 전달, 09-14 오후 보정본): `Character/Vehicle/{SilverBattleRaptor,SereneStarlightBlessing,RainbowMokoboard}/` 폴더 전체(wmodel, `textures`, `SourceMaterials`, 별빛 1.3·모코보드 self-rigged wmodel이 최신),
  `Character/{LanceMaster,Warlord,Artist,DimensionMaster}/AnimSets/<Class>_Ride{Raptor,Swing,Hoverboard}AnimSet.wmodel`.

## 남은 범위

- LookInfo 파티클·스폰 이펙트, 탈것 고유 스킬.
- 별빛의 가호 나머지 7색 등 색 변형, 제품 탈것 선택 UI, Gunslinger/Slayer 탑승자.
