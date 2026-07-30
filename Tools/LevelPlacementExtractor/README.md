# Lost Ark UE3 Level Placement Extractor

Lost Ark 레벨 패키지에서 `StaticMeshActor / InterpActor /
StaticMeshCollectionActor -> StaticMeshComponent -> StaticMesh` 연결과 원본
Transform을 JSON으로 복구하는 읽기 전용 도구다.

## 왜 필요한가

UModel의 일반 메시 export는 메시 파일 자체만 내보낸다. 레벨이 어떤 메시를
몇 개 만들었고 어디에 배치했는지는 레벨 UPK의 ExportTable과 각 export의
UE3 tagged property 안에 별도로 저장된다. 이 도구는 다음 순서로 그 배치표를
복구한다.

1. Lost Ark 전용 UModel의 `-nameresolve`로 논리 패키지명을 실제 난독화 UPK에 연결한다.
2. UPK summary의 Lost Ark 20-byte chunk table을 읽는다.
3. 각 chunk의 앞 4096바이트를 AES-256 ECB로 해제하고 LZ4 block을 복원한다.
4. NameTable, ImportTable, ExportTable을 UE3 규칙으로 파싱한다.
5. Actor와 Component의 tagged property를 읽고 StaticMesh import를 연결한다.
6. MapTool이 소비할 수 있는 안정적인 `placementId`, asset path, UE3-native Transform을 기록한다.

## 실행

```powershell
python Tools\LevelPlacementExtractor\extract_ue3_placements.py `
  --umodel C:\path\to\umodel_lostark_v7.exe `
  --package-root C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages `
  --output C:\path\to\placements `
  LV_LUT_HEARTRB_ED_PS `
  LV_LUT_HEARTRB_ED_SL00 `
  LV_LUT_HEARTRB_ED_SL01 `
  LV_LUT_HEARTRB_ED_SL02 `
  LV_LUT_HEARTRB_ED_SL03 `
  LV_LUT_HEARTRB_ED_SL04 `
  LV_LUT_HEARTRB_ED_SL05
```

출력은 레벨별 `*.placements.json`과 전체 집계
`placement_manifest.json`이다. 입력 UPK를 수정하거나 중간 복호화 패키지를
디스크에 만들지 않는다.

## Transform 계약

- `actor`와 `component`에는 추출한 원시 속성을 모두 보존한다.
- `transform`은 바로 배치에 사용할 정규화 결과다.
- 일반 `StaticMeshActor / InterpActor`는 Actor의
  `Location / Rotation / DrawScale / DrawScale3D`가 정본이다.
- `StaticMeshCollectionActor`는 Actor가 identity이고 Component의
  `Translation / Rotation / Scale / Scale3D`가 인스턴스별 정본이다.
- 좌표는 아직 `UE3-native`다. Client 좌표계 전환은 MapTool importer의 한
  지점에서만 적용해야 하며 추출 JSON 자체를 파괴적으로 변환하지 않는다.
- Rotator 원시값과 degree 값을 함께 저장한다. 한 바퀴는 65536 units다.

## Lost Ark 전용 주의점

- compressed chunk descriptor는 표준 UE3의 16바이트가 아니라 마지막
  `encrypted_lz4` 필드가 붙은 20바이트다.
- 현재 KR level package의 compression flags는 `0x44`이며 실제 payload는
  AES 해제 뒤 LZ4로 푼다.
- `IntProperty`는 선언된 4바이트 payload 앞에 Lost Ark 전용 8바이트
  descriptor가 추가된다.
- Actor export에는 UnrealScript stack frame이 있을 수 있어 tagged property의
  시작 위치가 고정 4바이트가 아니다. 파서는 유효한 Property FName 쌍과
  `None` terminator를 함께 검증한다.
- 에셋 import 목록만으로는 배치 복구가 되지 않는다. 반드시 ExportTable의
  Actor와 Component 직렬화 데이터를 읽어야 한다.

## HeartRB 실증과 이중 검토

2026-07-30 `PS + SL00~SL05`를 원본 package에서 두 번 독립 실행했다.

```text
placement                         13,091
unique StaticMesh object path        260
duplicate placement ID                 0
asset manifest join missing             0
owner Actor missing                      0
property errors                          0
non-finite Transform                     0
zero scale                               0
negative scale                       5,042
```

기존 결과와 재실행 결과의 placement JSON 7개 및 manifest 1개는 SHA-256이 전부
일치했다. 전수 감사 receipt는 다음 파일이다.

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/
  LV_LUT_HEARTRB_ED/placements/placement_audit.json
```

`12,949`는 streaming level `SL00~SL05`만의 Component 수이고, `13,091`은
persistent level `PS`의 142개를 더한 값이다.

## DeployData 게임플레이 Prop 복구

`PS/SL`의 StaticMesh placement와 `DeployData`의 Prop placement는 서로 다른
source layer다. 파괴물은 다음 네 정본을 ID로 join한다.

1. `DeployData.loa`: deploy actor ID와 UE3 Transform
2. `EFTable_Prop.db`: LookInfo ID, HP, blocking, hit mesh, state action
3. `EFDLProp_*.loa`: intact/fractured/skeletal mesh와 particle/audio reference
4. `TriggerMapData.loa`: actor별 spawn/despawn/destroy/state/조건 연결

HeartRB zone `37051` 재현 명령은 다음과 같다.

```powershell
python Tools\LevelPlacementExtractor\extract_deploydata_props.py `
  --deploy-data C:\path\to\37051\DeployData.loa `
  --prop-db C:\path\to\EFTable_Prop.db `
  --game-action-db C:\path\to\EFTable_GameAction.db `
  --skill-effect-db C:\path\to\EFTable_SkillEffect.db `
  --lookinfo-dir C:\path\to\37051\LookInfo `
  --static-raw-root C:\path\to\extracted\ITR `
  --trigger-map C:\path\to\37051\TriggerMapData.loa `
  --output C:\path\to\LV_LUT_HEARTRB_ED.deployprops.json `
  --arena-output C:\path\to\LV_LUT_HEARTRB_ED.valtan_arena.deployprops.json `
  --expect-records 169 --expect-arena-records 112
```

실측 결과는 전체 169개, 발탄 아레나 112개다. 아레나 112개 중 시각 모델은
85개이며, 77개는 fractured static mesh가 해석됐고 8개는 skeletal mesh다.
111개는 TriggerMapData 바이너리에서 deploy actor ID의 little-endian 4바이트 패턴이
2~6회 발견됐다. 특정 Trigger node·필드의 직접 참조를 구조 파싱한 수치는 아니다.
이 레코드는 정적
`.mapplacements`에 합쳐 저장하지 않는다. `ChangePropState`, `DestroyHitProp`,
`SpawnProp`, 난이도/페이즈 조건을 해석한 별도 Deploy runtime layer가 소유한다.

`StateOffActionId`는 `GameAction -> SkillEffect`까지 연결한다. HeartRB의
`3705101`~`3705107`은 실제 DB row이며, `3705102`~`3705107`은 250~450 cm 범위의
`SkHit_Claw1` 낙하/피격 효과와 공통 10,000 cm 효과를 가진다. 따라서 상태 액션을
단순히 “mesh 숨김”으로 축약하면 원본 충돌과 연출을 잃는다.

## Floor crack 재구성 도구

원본 crack MaterialInstance에는 diffuse와 normal만 있고 authored emissive slot은
없다. 원작 영상에서 보이는 녹색 틈빛은 exact normal을 유지하면서 영상 근거
재구성 texture로 분리한다.

```powershell
python Tools\LevelPlacementExtractor\build_valtan_crack_emissive.py `
  --input C:\path\to\bg_rad_valtan_crack_floor01_d_lsj.png `
  --output C:\path\to\bg_rad_valtan_crack_floor01_em_reconstruction.png

python Tools\LevelPlacementExtractor\sync_wmodel_pack_manifest.py `
  --pack C:\path\to\BG_RAD_VALTAN_FLOOR01A_SM `
  --material-index 1 `
  --normal bg_rad_valtan_crack_floor01_n_lsj.png `
  --emissive bg_rad_valtan_crack_floor01_em_reconstruction.png `
  --emissive-proof "VIDEO_MATCH reconstruction; no authored emissive slot"
```

에미시브는 diffuse를 밝게 덮는 텍스처가 아니라 주변보다 어두운 국소 골만 골라
만든다. `sync_wmodel_pack_manifest.py`는 v2 WMat 변경 뒤 stale hash가 남지 않게
`asset.manifest.json`과 `.complete.json`을 실제 산출물에서 원자적으로 갱신한다.

## HeartRB 환경·Deploy runtime 생성

정적 환경, phase texture proxy, gameplay Deploy layer는 서로 다른 provenance와 파일을
유지한다.

```powershell
python Tools\LevelPlacementExtractor\build_valtan_environment_runtime.py
python Tools\LevelPlacementExtractor\build_valtan_phase_layers.py
python Tools\LevelPlacementExtractor\build_deployprop_runtime.py
python Tools\LevelPlacementExtractor\build_maptool_scene.py
```

현재 생성 결과는 다음과 같다.

```text
Map Catalog v3                  269 = exact 260 + overlay 9
Map placement                13,103 = exact 13,091 + overlay 12
render profile                  12
Deploy catalog                   9
Deploy visual placement         85 = static 77 + skeletal 8
```

- 쇠사슬 A/B 13개, CloudPlane 2개, sky mirror 1개는 exact level placement다.
- crack Floor01A/B와 중앙 보강은 reconstruction overlay다.
- spacehole/hugechaosgate 6개 plane은 exact particle texture를 사용하지만 topology와
  debug 전환 timing은 영상 기반 reconstruction이다. 기본 저장 상태는 hidden이다.
- Deploy static 77개는 LookInfo의 FracturedStaticMesh 직접 참조와 같은 package의 intact
  sibling을 쌍으로 조리한다.
- `ITR_02326` 8개는 exact skinned WModel이지만 source glTF에 animation clip이 없어
  현재 bind pose다. `ITR_02326_Ani` AnimSet을 찾기 전에는 animation 완료로 표기하지 않는다.
- MapTool은 환경 phase(Baseline/SpaceHole/ChaosGate)와 Deploy state
  (Intact/Fractured/Despawned)를 수동 검증할 수 있다. 이 radio는 원작 Trigger timing이 아니다.

Catalog v3의 emissive intensity는 asset별 render profile이 소유한다. crack profile의
`0.35`를 모든 맵 자산에 전역 적용하지 않는다. 같은 G-buffer MRT를 쓰는 모든 일반
writer는 emissive target에 0을 기록해 뒤쪽 발광이 전경을 뚫는 현상을 막는다.

## MapTool commit 이후에도 남는 검증 경계

정적 Map과 Deploy visual layer의 parse/validate/stage/commit은 구현됐다. 다만 아래
항목까지 자동으로 복구됐다는 뜻은 아니다.

- 원본 placement 5,042개에는 음수 scale이 있다. 현재 MapTool의 양수-only validator로
  버리지 말고 reflection/winding/culling을 검증한다.
- property 오류 0은 tagged property를 읽었다는 뜻이다. hidden, collision-only,
  per-actor material override, Terrain, Particle, DeployData, navigation까지 끝났다는 뜻이 아니다.
- TriggerMap의 node/field 구조, Matinee의 정확한 프레임과 fade timing은 아직 별도 파서가
  필요하다. 지금 phase/state radio는 자동 raid 진행 대체물이 아니라 검증 도구다.

전체 공통 순서와 모든 함정은 다음 두 정본에 기록한다.

```text
C:/Users/user/Desktop/LostArk/.md/GB/07-30/맵추출파이프라인.md
C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/gotchas.md
```
