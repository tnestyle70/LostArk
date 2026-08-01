# 베른성 전체 맵 원본 복원 계획

작성일: 2026-08-01  
대상: `LV_BER_BERNCASTLE_T_*`, `CModel -> CMaterial`, MapTool  
상태: STATIC+LANDSCAPE+FOLIAGE RUNTIME COMPLETE / DECAL FIXTURE RECOVERED / OTHER SPECIAL RUNTIME PENDING

## C1. 목표

원본 UE3 레벨 패키지가 참조하는 베른성 정적 메시, 재질, 텍스처와 Transform을
추측 없이 복구한다. 이미 완료된 Landscape 42개와 결합해 MapTool에서 구역별로
불러올 수 있는 런타임 팩을 만든 뒤 Decal, Foliage, Water, 환경 Particle, 조명,
충돌과 Navigation 순으로 확장한다.

## C2. 현재 기준선

- 원본 Landscape 42개: 추출, `.wmodel`, catalog, placement 완료
- 베른성 정적 배치: 핵심 T 패키지 32,324개, FAV 선택 레이어 326개
- 고유 StaticMesh exact object path: 950개
- 기존 effect glTF corpus 감사 결과: exact 329개, 동명이지만 출처가 다른 후보 20개,
  exact source 미확보 601개
- 원본 UPK exact 재추출: 950/950개
- Bern static `.wmodel`: 950/950개
- 원본 정적 placement: 32,324개
- Landscape placement: 42개
- Foliage native instance placement: 17,651개
- MapTool 통합 placement: 50,017개
- MapTool 고유 runtime asset: 1,003개
- 현재 catalog 한 파일의 최대 asset 수: 512개

## C3. 완료 조건

1. 950개 exact package/object가 모두 원본 UPK export로 검증된다.
2. 각 에셋의 target glTF/bin, MaterialInstance 속성, 직접 texture dependency와
   UModel receipt가 보존된다.
3. 950개가 `ModelAssetConverter --pretransform --scale 100`으로 조리되고
   WINT/WMOD, material path, texture 존재 여부를 통과한다.
4. SL00~SL10별 catalog가 512 asset 제한을 지키고, 원본 placement의 asset join
   누락이 0이다.
5. placement는 `UE3 (X,Y,Z) cm -> Client (X,Z,-Y) m` 계약과 signed scale을
   보존하며 parse/validate/stage/commit에 성공한다.
6. Landscape 42개와 정적 맵을 같은 구역 검증 흐름에서 불러온다.
7. 비정적 lane은 각자 원본 수량과 runtime 구현 상태를 별도 receipt로 남긴다.

1~6은 완료했다. 7은 원본 inventory, Foliage 17,651개 runtime 연결, 첫 Decal
material graph 복구까지 완료했다. Water/Particle/Light/Decal projection과 게임
연동은 후속 단계다.

## C4. 문제 해결 순서

① 24개 placement JSON을 전수 읽어 950 exact path manifest를 만든다.  
② 기존 corpus 대조는 감사 정보로만 쓰고, 원본 UPK에서 object 단위 재추출해
329/20/601의 출처와 머티리얼을 동일 절차로 확정한다.  
③ MaterialInstance의 명시적 parameter만 diffuse, normal, specular, emissive,
opacity, ORM 슬롯으로 연결하고 알 수 없는 슬롯은 추측하지 않는다.  
④ 에셋별 WModel pack을 원자적으로 만들고 검증 후에만 runtime에 설치한다.  
⑤ 정적 맵 PASS 뒤 Decal -> Foliage -> Water -> 환경 Particle -> 조명 ->
collision/navigation 순으로 진행한다.

## C5. 자료구조와 안정 ID

- Asset key: `(root package casefold, object name casefold)`
- Asset ID: `MAP_<SHA1(fullPath) 12자리>_<정규화 object name>`
- Placement ID: 기존 `source package + export identity` 기반 uint64 유지
- Catalog: 생성 가능한 asset 정의
- Placement: source level, 원본 identity, position/quaternion/signed scale/visibility
- 저장 흐름: `parse -> validate -> stage -> commit`, 실패 시 신규 stage 전체 rollback

## C6. 구현 파일

- 신규: `Tools/BernCastlePipeline/build_bern_castle_assets.py`
- 신규: `Tools/BernCastlePipeline/build_bern_castle_nonstatic_manifest.py`
- 신규: `Tools/BernCastlePipeline/build_bern_castle_foliage_supplement.py`
- 신규: `Tools/BernCastlePipeline/build_bern_castle_foliage_overlay.py`
- 신규: `Tools/BernCastlePipeline/build_bern_castle_decal_probe.py`
- 신규: `Tools/BernCastlePipeline/README.md`
- 신규: `Tools/BernCastlePipeline/NONSTATIC_README.md`
- 신규: `Tools/LevelPlacementExtractor/build_bern_castle_shards.py`
- 재사용: `Tools/LevelPlacementExtractor/extract_ue3_placements.py`
- 재사용: `Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe`
- 변경: `Client/MapAssetCatalog`, `MapTool`의 strict `.mapset` 로드
- 신규: `Client/MapNavigationContract`
- 생성: `Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_*.mapassets`
- 생성: `Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_*.mapplacements`
- 생성: `Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE.mapset`
- 정적 런타임 팩: `Client/Bin/Resources/LostArk/Map/LV_BER_BERNCASTLE/`
- 지형 런타임 팩: `Client/Bin/Resources/LostArk/Map/LV_BER_BERNCASTLE_T/Landscape/`

950개가 catalog당 512개 제한을 넘는 것이 실측되어 `.mapset`과 shard 계약을
추가했다. 기존 `CMapAssetCatalog`, `CLoader`, `CMapTool`을 확장했고 기존 CP949와
CRLF를 유지했다.

## C7. 단계별 검증

### Phase A - Exact static source

- 950/950 object export
- duplicate asset ID 0
- unresolved package/object 0
- target glTF/bin 누락 0
- UModel이 만든 child process만 정리

### Phase B - Material/Cook

- 950/950 WModel
- 각 파일 offset 0 `WINT`, offset 16 `WMOD`
- converter `info` 성공
- material texture 상대경로와 실제 pack 파일 일치
- raw glTF bounds ×100과 cooked bounds 계약 검증

### Phase C - Scene

- 각 shard asset 수 512 이하
- source placement 수와 출력 placement 수 일치
- catalog에 없는 asset ID 0
- duplicate placement ID 0
- non-finite/zero scale 0
- signed/reflected scale 보존

### Phase D - Runtime

- Client Debug/Release
- F1에서 13개 shard, 1,003개 고유 asset, 50,017개 placement 로드
- shard set은 원본 대량 배치 보호를 위해 read-only
- 실패 입력에서 기존 Scene 보존

### Phase E - Non-static inventory

- 원본 core level package 22개
- Decal 86, Foliage component 1,697, Water placement 108
- ParticleSystemComponent 1,373, Light 326, Fog 1, Wind 1
- 총 3,592개 source item
- base 950에 없던 Foliage exact mesh 11종: source/Cook 11/11 별도 완료
- Foliage native suffix를 해독해 1,697 component / 17,651 instance Transform 복구
- 기존 base mesh 4종 938 instance 재사용, supplemental 11종 16,713 instance 연결
- 첫 Decal fixture의 MIC -> parent -> diffuse/opacity branch와 DDS exact 복구
- inventory 완료를 runtime 렌더링 완료로 표시하지 않는다.

## C8. 범위와 주의사항

- 원본 설치 UPK는 읽기 전용이다.
- package 전체 무제한 export 대신 `-obj=<exact object>`만 사용한다.
- UModel glTF는 meter이므로 cook 때만 `--scale 100`을 적용한다.
- `_s`, `_p` 같은 접미사만 보고 ORM을 추측하지 않는다.
- 회색 fallback은 형상 확인용이며 원본 재질 완료로 계산하지 않는다.
- Landscape 표시용 bake는 원본 UE3 다층 셰이더 실행 결과가 아니다.
- `Client/Bin/Resources`는 Git 미추적 공유 Drive 팩이며 DataFiles만 Git/LFS
  정책을 따른다.
- 기존 `ACTIVE.maparea`와 다른 팀원의 작업 파일은 자동으로 덮어쓰지 않는다.
- 베른성 전체 팩은 Foliage 연결 후 약 3.82 GB private memory를 사용하므로 texture content-address
  deduplication과 구역별 지연 로딩 없이는 게임 기본 맵으로 상시 올리지 않는다.
- collision/navigation은 파일명과 prototype 계약만 분리했다. 베른성용 실제
  walkable 분류, `.navgrid`, 정적 blocker 충돌은 아직 완료가 아니다.
