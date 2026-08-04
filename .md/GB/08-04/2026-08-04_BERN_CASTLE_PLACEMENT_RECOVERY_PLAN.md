# 베른성 배치 오염 긴급 복구 계획

작성일: 2026-08-04
대상 Area: `LV_BER_BERNCASTLE`

상태: 구현 및 자동 검증 완료. 실행 화면 수동 smoke만 남음.
결과: `2026-08-04_BERN_CASTLE_PLACEMENT_RECOVERY_RESULT.md`

## 1. 목표와 종료 증거

이번 변경은 베른성 전체 시각 품질 개선이 아니라, 후속 지형·재질 작업을 방해하는 배치 정본 오염 두 가지를 먼저 제거한다.

1. `EFMotionStaticMeshActor`를 인식하지 못해 Actor Transform 대신 원점 기본값이 기록된 131개 placement를 원본 UPK에서 다시 복구한다.
2. 일반 베른성에 섞인 `LV_BER_BERNCASTLE_T_EVENT01`과 `LV_BER_BERNCASTLE_T_SCENE03E` placement를 삭제하지 않고 기본 비표시 상태로 생성한다.
3. unresolved owner/property parse가 한 건이라도 남으면 shard와 authoring을 교체하지 않는다.
4. 기존 authoring이 imported baseline과 완전히 같을 때만 수정된 baseline으로 authoring을 다시 만든다.

종료 증거는 다음과 같다.

```text
EFMotionStaticMeshActor recovered placement: 131
unresolved placement: 0
EVENT01 exact 1,127 + foliage 6: visible 0
SCENE03E exact 45: visible 0
total placement: 50,017 유지
stable placement ID set: 변경 없음
```

## 2. 데이터 소유권과 호출 흐름

```text
원본 UPK
-> extract_ue3_placements.py
-> *.placements.json
-> build_bern_castle_shards.py parse/validate/stage/commit
-> Data/Maps/Imported/LV_BER_BERNCASTLE shard set
-> Data/Maps/Authoring/LV_BER_BERNCASTLE authoring baseline
-> Publish-MapAuthoring.ps1
-> Client/Bin/DataFiles/Map runtime
```

생성된 `.mapplacements`를 손으로 고치지 않는다. `Client/Bin/DataFiles`는 publisher만 교체한다.

## 3. 수정 파일

### `Tools/LevelPlacementExtractor/extract_ue3_placements.py`

- placement Actor 클래스 정본에 `efmotionstaticmeshactor`를 추가한다.
- component owner를 지원 Actor로 해석하지 못하면 원점 fallback을 만들지 않는다.
- unresolved 항목은 `unresolvedPlacements`에 owner/class/archetype/asset 근거와 함께 격리한다.
- package와 전체 manifest에 unresolved count를 기록하고 하나라도 남으면 CLI가 실패한다.

### `Tools/LevelPlacementExtractor/build_bern_castle_shards.py`

- source document의 unresolved 항목과 actor source의 null actor를 commit 전 거부한다.
- 기본 비표시 level을 `EVENT01`, `SCENE03E`로 고정한다.
- exact placement는 기존 render-profile visibility override 경로를 사용한다.
- foliage overlay는 같은 source level이면 `visible=false`로 정규화한다.
- staged shard와 aggregate receipt에서 hidden count를 검증한다.

### 하네스

- `test_extract_ue3_placements.py`: Lost Ark 전용 Actor admission과 unresolved fallback 금지를 검증한다.
- `test_build_bern_castle_shards.py`: hidden exact/overlay, unresolved source rollback, aggregate hidden count를 검증한다.

## 4. 실패와 rollback

- raw 재추출은 별도 staging 폴더에 먼저 기록한다.
- 131개 ID의 Actor owner와 finite Transform을 검증한 뒤에만 source JSON을 사용한다.
- shard builder는 output directory 내부 temporary stage를 사용하고 전체 검증 뒤에만 파일 세트를 교체한다.
- authoring은 작업 시작 시 imported baseline과 50,017행 전체가 동일하다는 검증이 통과한 경우에만 새 shard 행을 합쳐 교체한다.
- publisher 실패 시 기존 `Client/Bin/DataFiles/Map` 세트를 유지한다.

## 5. 검증 순서

```text
Python unit tests
-> affected UPK 5개 재추출
-> recovered 131 / unresolved 0 검증
-> Bern shard builder
-> 50,017 placement / stable ID / hidden 1,178 검증
-> authoring 재생성
-> Publish-MapAuthoring
-> JSON 및 map document parse
-> ProjectAudit
-> git diff --check
-> Debug Client Bern/Map Editor 수동 smoke
```

Landscape 재질, Water, Decal, 제품 streaming scope, 충돌과 Navigation은 이번 긴급 복구에 섞지 않고 RESULT의 후속 작업으로 남긴다.
