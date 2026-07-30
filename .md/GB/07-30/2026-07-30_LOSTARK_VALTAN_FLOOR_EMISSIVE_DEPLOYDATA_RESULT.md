# 발탄 Floor 에미시브·DeployData 파괴 오브젝트 복구 결과

- 작성일: 2026-07-30
- 대상: `LV_LUT_HEARTRB_ED`, zone `37051`
- 범위: Floor01/A/B 완성 배치, crack 녹색 틈빛, 파괴 가능한 게임플레이 Prop 전수 조사
- 후속 구현: `2026-07-30_LOSTARK_VALTAN_DYNAMIC_ENVIRONMENT_DEPLOY_RUNTIME_RESULT.md`

## 1. 최종 판결

1. Floor01/A/B는 각각 원본 방향과 180도 회전 방향을 배치해 총 6개 reconstruction overlay로 완성했다.
2. crack의 exact diffuse와 exact normal을 유지하고, 원작 영상 근거의 약한 녹색 emissive mask를 별도 texture로 추가했다.
3. 파괴 가능한 구조물의 정본은 PS/SL 정적 placement가 아니다. `DeployData + EFTable_Prop + LookInfo + TriggerMapData + GameAction/SkillEffect`가 함께 소유한다.
4. 발탄 아레나 exact 시각 Prop 85개를 찾았으므로 임의 대체 오브젝트는 필요 없다.
5. 이 85개는 정적 `.mapplacements`에 합치지 않고 별도 `.deployassets/.deployplacements`와 Deploy layer로 구현했다.

## 2. Floor 완성 배치

overlay 정의:

```text
Floor01   yaw 0 / 180
Floor01A  yaw 0 / 180
Floor01B  yaw 0 / 180
```

공통 Client 위치는 `(156.279, 23.242, -121.977)m`이며 yaw 180 quaternion은
`(0, 1, 0, 0)`이다. 결과 scene은 다음 수량으로 다시 생성했다.

```text
catalog                 269 = exact 260 + overlay asset 9
placement            13,103 = exact 13,091 + overlay 12
negative-scale         5,042
reflection             4,521
```

정본 파일:

- `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements`
- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/placements/maptool_scene_receipt_G5_dynamic_environment.json`

## 3. Crack 녹색 틈빛

원본 `bg_rad_valtan_crack_floor01_mi_lsj`는 diffuse와 normal을 참조하지만 authored
emissive texture slot은 없다. 따라서 증거 등급을 다음처럼 분리했다.

```text
diffuse     MATERIAL_EXACT
normal      MATERIAL_EXACT
emissive    VIDEO_MATCH_RECONSTRUCTION
```

재구성 mask는 diffuse의 국소 평균보다 어두운 골만 추출하며 색은 `(12,180,105)`다.
crack asset별 Catalog render profile intensity는 `0.35`이고 다른 MapAsset에는 전파하지
않는다. 전체 돌 표면을 밝히지 않는다. Engine에는
`Target_Emissive`를 추가하고 Map binary mesh shader의 `SV_TARGET4`를 deferred
combine에서 더한다. 같은 MRT를 쓰는 나머지 G-buffer writer는 Target4에 0을 써서
전경 occlusion leak을 막는다. 현재 구현은 self-lit 틈빛이며 넓은 halo/bloom은 별도
post-process 작업이다.

Floor01A/B WMat은 material-v2 2개 재질로 다시 조리했고, material 1에 exact normal과
reconstruction emissive가 들어간다. 각 source pack의 manifest와 completion receipt도
6개 실제 deliverable 기준으로 다시 동기화했다.

## 4. DeployData 전수 조사

| 항목 | 수량 |
|---|---:|
| 전체 `CEFDeployActor_Prop` | 169 |
| 고유 deploy actor ID | 169 |
| 고유 Prop definition | 27 |
| 상태·파괴 레코드 | 121 |
| 발탄 아레나 반경 2,000 cm | 112 |
| 아레나 시각 모델 | 85 |
| 아레나 destructible/stateful | 86 |
| exact extracted fractured static | 77 |
| exact skeletal | 8 |
| TriggerMapData binary ID occurrence | 111 |
| StateOffAction DB chain 해석 | 30 |

아레나 exact asset군:

```text
ITR_02306 5   ITR_02307 4   ITR_02308 6   ITR_02309 2
ITR_02310 3   ITR_02311 2   ITR_02315 28  ITR_02316 27
ITR_02326 8 skeletal
```

`ITR_02306~02316`의 LookInfo 직접 mesh reference 77개는 전부
`FracturedStaticMesh`다. intact mesh는 같은 package의 sibling export 77개에서
별도로 찾았으므로 provenance를 구분한다. LookInfo는 파편 particle과 파괴 audio도
참조한다. `ITR_02326`은 SkeletalMesh, AnimSet, PhysicsAsset, on/off/spawn
particle과 audio를 참조한다. 이는 “Floor A/B 통짜를 런타임에 임의 submesh로
자른다”는 가설보다 별도 Deploy gameplay prop가 파괴 상태를 재생한다는 결론을
직접 지지한다.

## 5. 상태 액션 근거

`EFTable_Prop.StateOffActionId 3705101~3705107`은 실제 `GameAction` row로 확인됐다.
각 action은 두 단계이며 `SkillEffect`까지 연결된다.

| StateOffAction | 첫 효과 | 범위 | 둘째 효과 |
|---:|---:|---:|---:|
| 3705101 | 42060011 | self | 42060040 / 10,000 cm |
| 3705102 | 42060091 | 250 cm | 42060040 / 10,000 cm |
| 3705103 | 42060092 | 250 cm | 42060040 / 10,000 cm |
| 3705104 | 42060093 | 250 cm | 42060040 / 10,000 cm |
| 3705105 | 42060094 | 300 cm | 42060040 / 10,000 cm |
| 3705106 | 42060095 | 400 cm | 42060040 / 10,000 cm |
| 3705107 | 42060096 | 450 cm | 42060040 / 10,000 cm |

`42060091~96`은 `SkHit_Claw1`, fallDown, hit strength를 포함한다. 파괴 처리는
메시 visibility만 바꾸는 기능이 아니라 hit/범위/상태/연출 계약이다.

TriggerMapData raw 바이너리에는 `ChangePropProperty 77`, `Condition_HitProp 21`,
`SpawnProp 11`, `DestroyHitProp 10`, `ChangePropState 5`, `DespawnProp 2`와
`Destruct/RestoreStart/RestoreFinish` 문자열이 존재한다. deploy ID 111개의
little-endian 4바이트 패턴도 2~6회 발견되지만, 아직 특정 node·field의 직접 참조를
구조 파싱한 결과는 아니다.

## 6. 런타임 구현 경계

후속 구현은 다음 구조로 완료했다.

1. Deploy catalog에 8 static family와 `ITR_02326` skeletal family, 총 9개를 등록했다.
2. Deploy placement 문서에 시각 레코드 85개의 deploy actor ID와 Transform을 저장했다.
3. MapTool은 문서 전체를 parse/validate하고 85개를 임시 stage한 뒤 한 번에 commit한다.
4. static 77개는 intact/fractured/despawned 상태를 전환한다.
5. skeletal `ITR_02326` 8개는 exact skinned WModel의 bind pose로 렌더한다. source glTF
   animation이 0이므로 exact AnimSet을 찾기 전에는 애니메이션 완료로 표기하지 않는다.
6. model 없는 actor는 추출 JSON의 gameplay record로 보존하고 시각 proxy를 만들지 않는다.
7. 어느 단계든 실패하면 새 Deploy layer만 rollback하고 기존 scene을 유지한다.

TriggerMap node/field 구조 파서와 원작 자동 phase timing은 아직 후속 경계다. 현재
Intact/Fractured/Despawned 선택기는 명시적인 수동 검증 도구다.

임시 시각 검증이 꼭 필요할 경우 exact ITR fractured sibling을 debug proxy로 쓸 수
있지만 최종 scene에 저장하지 않는다. exact 에셋 자체를 못 찾았을 때의 fallback은
같은 LookInfo의 intact sibling → 같은 ITR family 순이며, unrelated 성벽/돌 메시를
원작 복원본으로 표시하지 않는다.

## 7. 생성물과 검증

- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_valtan_crack_emissive.py`
- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/sync_wmodel_pack_manifest.py`
- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/extract_deploydata_props.py`
- `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_deployprop_runtime.py`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/deploydata/LV_LUT_HEARTRB_ED.deployprops.json`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/deploydata/LV_LUT_HEARTRB_ED.valtan_arena.deployprops.json`

검증 결과:

```text
Python py_compile                 PASS
Deploy record expectation 169    PASS
Arena record expectation 112     PASS
LookInfo missing 0               PASS
HLSL FXC compile                 PASS
Engine x64 Debug/Release         PASS
UpdateLib Debug/Release          PASS
Client x64 Debug/Release         PASS
Debug Enter -> F2 -> AssetTest   PASS
13,103 placement + Deploy 85 AssetTest smoke PASS
```

최신 실행 로그는
`C:/Users/user/Desktop/LostArk/.codex_tmp/validation/valtan_dynamic_runtime/18_Runtime_AssetTest_Smoke.log`다.
창 제목은 `Valtan WModel Asset Test`까지 전환됐으며 startup/loading/G-buffer/resource
load crash는 없었다. 다만 이 smoke만으로 crack emissive의 최종 체감 밝기,
CloudPlane 흐름, phase sky 화면 구성을 합격 처리하지 않는다. 대상 카메라에서 원작
영상과 asset별 render profile을 육안 비교하는 절차가 마지막 시각 QA다.
