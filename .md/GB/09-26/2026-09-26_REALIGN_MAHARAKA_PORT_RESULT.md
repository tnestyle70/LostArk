# 마하라카 작업을 새 main 위로 이식 (2026-09-26, 임무 I)

## 0. 요약

- 기준: 브랜치 `codex/main-ship-maharaka-0926`, `origin/main` = `a84bbcd4`. 시작 시점 추적 변경 0개.
- 예전 상태는 브랜치 `backup/pre-realign-0926-worktree`(= `135772ae`, 저장 안 했던 추적 수정까지 담은 스냅샷 S)와 `backup/pre-realign-0926-head`(= `cd58d12b`)에 보존돼 있다.
- 이 작업이 한 것: 마하라카(`LV_OCN_EVENTIS_MHP`) 변경만 S에서 새 main 위로 다시 적용하고, 마하라카 Area만 게시했다. 쿠크 수정, 실패 수정, 항목 11·14 결과는 이식하지 않았다.
- 결과: 추적 파일 12개 변경. 마하라카 Area 게시 Validate 26초, Publish 6초, Check 6초 모두 성공. 게시본 5개는 S의 게시본과 내용이 같다.
- **빌드는 하지 않았다. 화면은 확인하지 못했다(사용자 판정).**

## 1. 이식한 파일

### 1.1 통째로 복원 (S에서 정확한 경로만 `git restore --source=135772ae --worktree`)

main이 이 파일들을 바꾸지 않았음을 `git diff ca02c873 a84bbcd4`로 확인한 뒤 복원했다(main은 `MapCatalog.json`과 마하라카 imported·authoring, `Level_Development.h`를 바꾸지 않았다).

- `Data/Maps/MapCatalog.json`: 마하라카 항목 hunk 1개뿐(placementCount 3839 → 4651, assetCount 398 → 406, `sourceWater/water`, `sourceLights/lights`, `sourceMaterials/materials` 쌍). 다른 항목 변경 없음(hunk 수 1로 확인).
- `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/`: `build.receipt.json`, `landscape-merge.receipt.json`, `mapassets`, `mapplacements`(LFS, 4651행), `renderprofiles.json`.
- `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements`(LFS).
- `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/`의 `maplights.json`, `mapwater.json`(main에는 없던 파일, 새 파일로 생김).
- `Client/Public/Level_Development.h`: 4줄(전부 마하라카 전용: `CMapLightPresentationRuntime` 전방 선언, 멤버 2개).
- `Tools/LevelPlacementExtractor/README.md`: 마하라카 재질 입력 절 40줄 추가.
- `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapmotions.json`(65행): **게시자(`Publish-MapAuthoring.ps1`)가 만들지 않는 파일**이라 S의 파일을 그대로 복원했다. 로더 `MapPlacementRuntime`의 `<AreaId>.mapmotions.json` 읽기는 main에 이미 있다. 저작 정본이 저장소에 없다(이전 fork 도구로 만든 산출물).
- `.md/GB/09-25/2026-09-25_MAHARAKA_WATERBOMB_RESTORATION_RESULT.md`(문서).

### 1.2 hunk 단위 (겹침 파일 `Client/Private/Level_Development.cpp`)

main이 `Update()`에 `m_Replication.Update_CombatHover(...)` 1줄을 추가했다(겹침). `git diff ca02c873 135772ae`로 내 변경만 추출해 `out/Realign20260926/maharaka/Level_Development.cpp.patch`를 만들고 `git apply --check`를 통과시킨 뒤 적용했다(`--3way`나 스테이징 없음). hunk 4개, +69줄 전부 마하라카 분기다.

- include 2줄: `EffectFailureDiagnostic.h`, `MapLightPresentationRuntime.h`.
- `Initialize()`: 마하라카 진입 시 진단 로그 `map.water.loaded`(area, assets, waterAssets, waterRows, materialAssets)와 로드 실패 시 `map.area.load-failed`, 자체 모션 문서 `Load_SelfMotions`.
- `Update()`: 마하라카일 때 `Update_SelfMotions`와 지도 조명 매 프레임 제출(`Submit_Frame`, 실패는 한 번만 보고).
- 지도 조명 런타임 로드: 조명 문서가 없거나 거부되면 로드 실패(`E_FAIL`)로 처리.
- 결과 확인: main의 `Update_CombatHover` 줄 보존(1개), CRLF 343줄/LF 단독 0/BOM 없음(원래와 같은 형식).

### 1.3 게시로 다시 생성 (S에서 복사하지 않음)

`Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP`의 Validate → Publish → Check(범위 Area)로 아래를 만들었다: `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.{mapassets, mapplacements, maplights.json, mapwater.json, mapmaterials.json}`.

### 1.4 이미 작업 폴더에 있던 미추적 파일(이식 대상, 그대로 사용)

`Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapmaterials.json`(326행), `Tools/LevelPlacementExtractor/author_ocean_water_rows.py`, `build_source_map_material_inputs.py`, 마하라카 전달 목록(`Resource_Distribution_2026-09-*_Maharaka*.txt`, `Copy_ResourceDistribution_2026-09-*_Maharaka*.ps1`), 마하라카 결과 문서들.

## 2. 이식하지 않은 것과 이유

- 쿠크 수정 전부(카드미로, 앙코르 자막, 마리오, 빙고 컷신 재타이밍, 항목 11·14 결과물), 실패 수정(Compositions `shape` 검증기, Retail 삐에로 상자 제외, world-playback 테스트), Compositions 게시본: 임무 범위 밖(요청은 배 탑승과 마하라카만 살리는 것).
- 배 탑승 관련 파일: 다른 fork(H) 몫.
- 마하라카 환경 프로필 `scene.maharaka.source-rendering.v1`: 팀장 렌더링 정본이라 이식하지 않았다(main 그대로).
- `LevelRegistry.cpp`의 마하라카 항목: main 것 그대로(`git diff origin/main`에 없음).
- 마하라카 서버 월드(`WORLD_ID::MAHARAKA`), 네비(`LV_OCN_EVENTIS_MHP.nav*`), `Data/Worlds/LV_OCN_EVENTIS_MHP/Gameplay.world.json`: main에 이미 있어 그대로 뒀다.

## 3. 충돌 해결

충돌 없음. `Level_Development.cpp`는 main의 1줄과 내 hunk 4개가 서로 다른 위치라 `git apply --check`가 통과했다. 적용 뒤 `Update()`에서 내 마하라카 블록이 앞, main의 `Update_CombatHover`가 뒤로 공존함을 소스를 읽어 확인했다.

## 4. 게시 결과와 S 게시본 대비

- Validate 26초 성공, Publish 6초 성공, Check 6초 성공. 잠금은 실행 뒤 풀렸다. 로그: `out/Realign20260926/maharaka/publish_{Validate,Publish,Check}.log`.
- 게시 전 게시본 백업: `out/Realign20260926/maharaka/backup_published_before_publish/`.
- S 게시본과 비교(`git cat-file --filters 135772ae:<path>` 또는 미추적 백업 대비 SHA-256):
  - `mapassets`: 220,625 B, 바이트 동일. `mapplacements`: 1,175,993 B, 바이트 동일. `mapmaterials.json`: 610,873 B, 바이트 동일.
  - `maplights.json`: 게시본 16,961 B, S 17,894 B. `mapwater.json`: 게시본 17,538 B, S 18,160 B. **줄바꿈(LF 대 CRLF)만 다르고 줄바꿈을 무시하면 내용이 같다.** 게시자는 LF로 쓰고 git 체크아웃 필터가 CRLF로 바꾼다.
  - 그러므로 main의 게시자/스키마가 S 시점과 달라져서 생긴 차이는 없다.

## 5. 수치 검증

- JSON 11개 parse 성공. MapCatalog 마하라카 항목: placementCount 4651, assetCount 406, 여섯 경로(`sourceWater/water/sourceLights/lights/sourceMaterials/materials`) 전부 실제 파일 존재.
- 배치 4651(`mapplacements` 첫 줄), 물 행 10(`SOURCE_MATERIAL_EXACT` 3 + `PROJECT_AUTHORED` 7), 재질 행 326(그중 `water-41` 7행), 자체 모션 65행, 지도 조명 33개.
- `Level_Development.cpp` 구문 검사: `cl.exe /Zs`(실제 Debug|x64 옵션 `/std:c++20 /I../Public/ /I../../EngineSDK/Inc/ /I../../Shared/Public/`, `CL.command.1.tlog`에서 가져옴) EXITCODE=0, 오류 0, 경고는 기존 C4819(코드 페이지) 19개뿐. **구문 검사만이며 링크·전체 빌드가 아니다.** 로그 `out/Realign20260926/maharaka/cl_zs_level_development.log`.
- `git diff --check` 공백 오류 0. 렌더링 보호 파일(`Renderer.*`, `ShaderFiles`, `Data/Rendering`, `Client/Bin/DataFiles/Rendering`, `UI_Sprite.cpp`, `LevelRegistry.cpp`) `origin/main` 대비 diff 0.
- 리소스(Git 비추적)는 만지지 않았다: `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP` 3,254개 파일, `_FOLIAGE` 51개 그대로.

## 6. 이 fork가 바꾼 파일 (시작 시 추적 변경 0 대비)

추적 파일 12개(전부 마하라카 범위):

- `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapassets`(게시, LFS 포인터), `...mapplacements`(게시, LFS 포인터)
- `Client/Private/Level_Development.cpp`(+69), `Client/Public/Level_Development.h`(+4)
- `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapplacements`(LFS 포인터)
- `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/`: `build.receipt.json`, `landscape-merge.receipt.json`, `mapassets`, `mapplacements`, `renderprofiles.json`(+29 -1)
- `Data/Maps/MapCatalog.json`(+9 -3, 마하라카 항목 hunk 1개)
- `Tools/LevelPlacementExtractor/README.md`(+40)

미추적 파일(main에는 추적되지 않는 새 파일이라 커밋할 때 `git add` 필요): `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.{maplights.json, mapwater.json, mapmaterials.json, mapmotions.json}`, `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.{maplights.json, mapwater.json, mapmaterials.json}`, `Tools/LevelPlacementExtractor/{author_ocean_water_rows.py, build_source_map_material_inputs.py}`, 마하라카 문서·전달 목록.

작업 폴더에는 H(배) fork가 동시에 바꾼 파일도 있다(`Level_Bern.cpp`, `Character.cpp` 등). 그것들은 이 목록에 포함하지 않았다.

## 7. 빌드 뒤 사용자 확인 순서

1. VS에서 Debug 정상 Build(Rebuild·Clean 아님). 이 fork의 C++ 변경은 `Level_Development.cpp` 1개다.
2. Server와 Client를 재시작하고 Lobby에서 Maharaka로 들어간다.
3. `Client/Default/EffectFailure.user.log`에서 `map.water.loaded`로 시작하는 줄을 찾는다. 예상 값: `area=LV_OCN_EVENTIS_MHP assets=406 waterAssets=10 waterRows=10 materialAssets=248`. 다르거나 없으면 그 줄(과 `map.area.load-failed`가 있으면 그 줄)을 알려 달라.
4. 화면에서 물·재질·조명이 어떻게 보이는지는 사용자 판정이다. 어젯밤 확인한 노랑·초록 바닥 얼룩과 물 색 불균일은 **이 이식으로 고치지 않았다**(F fork의 후속 작업, 아직 미적용).

## 8. 되돌리는 방법

- 파일별 원래(main) 상태: `out/Realign20260926/maharaka/backup/`(12개, 경로 유지). 되돌릴 때는 그 복사본을 같은 경로로 덮어쓴다.
- 게시본 원래 상태: `out/Realign20260926/maharaka/backup_published_before_publish/`.
- 예전 상태 전체: 브랜치 `backup/pre-realign-0926-worktree`(= S).

## 9. 사용자 결정이 필요한 것과 확정하지 못한 것

- **`renderprofiles.json`(imported) 이식 여부.** 변경은 신규 에셋 7개에 `emissiveIntensity: 0.0`을 추가한 것뿐이고 기존 항목·최상위 키는 그대로다. 이 파일을 읽는 곳은 지도 추출 파이프라인(`build_bern_castle_shards.py`)이라 팀장님 렌더링 옵션(품질·장면·영역 값)이 아니라 에셋 메타데이터로 판단해 이식했다. 팀장님 정책상 렌더링 값으로 봐야 한다면 `Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.renderprofiles.json`만 백업본으로 되돌리고 마하라카 Area를 다시 게시하면 된다.
- `mapmotions.json`은 게시자 없이 복원한 파일이다. 나중에 정본과 게시자가 생기면 교체해야 한다.
- 실제 게임에서 이 재질 문서와 물 문서를 C++ 파서가 읽어 성공하는지는 확인하지 못했다(진단 로그로 사용자가 확인).
- 화면 결과는 확인하지 못했다.

## 10. 최종 재확인

- 확인한 것: 시작 시 추적 변경 0, 백업 12개 생성, 전용 파일 13경로 복원(경로 명시), 겹침 hunk 검사 후 적용, 구문 검사 EXITCODE=0, 게시 Validate/Publish/Check 성공, 게시본 5개의 S 대비 내용 일치(3개 바이트 동일, 2개 줄바꿈만 상이), JSON 11개 parse, 수치(4651/406/10/326/65/33), 렌더링 보호 diff 0, `git diff --check` 0, Resources 파일 수 불변.
- 확인하지 못한 것: 빌드와 링크, C++ 파서의 실제 로드, 화면, 어젯밤 얼룩·물 색 문제의 해결 여부.
- 커밋·push·브랜치 변경·stash·reset은 하지 않았다.

MAHARAKA_PORT_DONE
