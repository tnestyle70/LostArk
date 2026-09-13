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

## 다른 PC 준비

- Server PC는 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server`(또는 `Publish-VehicleProfiles.ps1 -Mode Publish`) 후 재시작한다.
- Resources(Drive 전달): `Character/Vehicle/{SilverBattleRaptor,SereneStarlightBlessing,RainbowMokoboard}/` 폴더 전체(wmodel, `textures`, `SourceMaterials`),
  `Character/{LanceMaster,Warlord,Artist,DimensionMaster}/AnimSets/<Class>_Ride{Raptor,Swing,Hoverboard}AnimSet.wmodel`.

## 남은 범위

- LookInfo 파티클·스폰 이펙트, 탈것 고유 스킬.
- 별빛의 가호 나머지 7색 등 색 변형, 제품 탈것 선택 UI, Gunslinger/Slayer 탑승자.
