# 2026-09-18 Character Select Map Tool 모델 identity 불일치 RESULT

## 증상

Character Select 레벨에서 F1 → Map Tool 을 열면 런타임 바인딩 문구
(`Editing the current level's runtime map.`)는 표시되지만 Area 콤보가 `<select Area>` 로 비어 있고
Navigation 탭이 `Navigation authoring is disabled for this Area.` 로 막힌다.

```
Workspace status: Runtime/source model identity differs: MAP_5A40868D13C3_BG_GDOGODS_PILLAR01C_SM_FR_407AA7EB7C29
```

## 1. 검사가 비교하는 대상

문구의 출처는 `MapTool_Area.cpp` 가 아니라 **`Client/Private/MapAssetCatalog.cpp:309-329`**
(`CMapAssetCatalog::Bind_RuntimePrototypes`) 다.

```cpp
for (const auto& asset : m_Entries)              // m_Entries = 저작 source 카탈로그
{
    const auto* live = runtimeCatalog.Find(asset.id);
    if (!live || live->prototypeTag.empty() ||
        live->resolvedModelPath.lexically_normal() != asset.resolvedModelPath.lexically_normal())
    {
        m_Status = "Runtime/source model identity differs: " + asset.id;
        return false;
    }
}
```

asset ID 별로 세 가지를 본다.

1. runtime 카탈로그에 같은 ID 가 **존재**하는가
2. runtime 쪽 `prototypeTag` 가 비어 있지 않은가
3. `resolvedModelPath` 가 같은가 (`MapAssetCatalog.cpp:2054`
   `entry.resolvedModelPath = ResolveRuntimePath(entry.modelRelativePath)`, 즉 `.mapassets`
   행의 modelPath 를 같은 asset root 로 해석한 값)

호출부는 `Client/Private/MapTool_Area.cpp:849` 이며, 이 검사를 통과해야
그 다음 placement ID 검사(`MapTool_Area.cpp:878`)와 Area attach 로 넘어간다.

- source = `descriptor.sourceCatalog` = `Data/Maps/Imported/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapassets`
- runtime = `runtimeTargets.pCatalog` = 레벨이 로드한 `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapassets`

## 2. 해당 ID 의 실제 값 차이

| 위치 | 결과 |
|---|---|
| source `.mapassets` | `MAP_5A40868D13C3_BG_GDOGODS_PILLAR01C_SM_FR_407AA7EB7C29` **행 있음** (modelPath `Map/CHARACTERSELECTMAP/MAP_5A40868D13C3_BG_GDOGODS_PILLAR01C_SM/MAP_5A40868D13C3_BG_GDOGODS_PILLAR01C_SM.wmodel`) |
| runtime `.mapassets` | **행 없음 (0건)** |

즉 `runtimeCatalog.Find(asset.id)` 가 nullptr 를 돌려주어 첫 조건에서 실패했다.
`resolvedModelPath` 가 다른 것이 아니라 **행 자체가 없었다.**

배치 문서에서 원인이 더 분명하게 드러난다. 같은 placementId·같은 transform 인데 assetId 만 다르다.

```
# source (Data/Maps/Authoring/.../LV_LOBBY_CLASSSELECT_SL00.mapplacements)
14665354827634359844 "…:export:1000" … "MAP_5A40868D13C3_BG_GDOGODS_PILLAR01C_SM_FR_407AA7EB7C29" -780.493594 …

# runtime (Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements)
14665354827634359844 "…:export:1000" … "MAP_5A40868D13C3_BG_GDOGODS_PILLAR01C_SM"                  -780.493594 …
```

source 는 재질 변형별 `_FR_<hash>` 접미사 ID 로 재추출된 신버전이고,
runtime 은 그 이전의 base ID 판이다.

## 3. 전수 불일치

### 카탈로그 (`.mapassets`)

| 항목 | 값 |
|---|---|
| source 행 수 | 209 (formatVersion 4) |
| runtime 행 수 | 181 (formatVersion 5) |
| source 에만 있는 ID | **28** |
| runtime 에만 있는 ID | 0 |
| 양쪽에 있으나 modelPath 다름 | 0 |

runtime 은 source 의 **진부분집합**이었다. 빠진 28 개는 전부 `_FR_` 변형이며 두 기본 모델에서 파생된다.

- `MAP_5A40868D13C3_BG_GDOGODS_PILLAR01C_SM_FR_*` 15 개
- `MAP_B00FEC87C7D8_BG_GDOGODS_PILLAR01B_SM_FR_*` 13 개

(상위 20 개 목록은 위 두 계열의 hash 접미사 변형이며 다른 계열은 없다.)

### 배치 (`.mapplacements`)

| 항목 | 값 |
|---|---|
| source 배치 수 | 805 |
| runtime 배치 수 | 804 |
| source 에만 있는 placementId | 1 (`2`, 에디터 저작 행 `editor:LV_LOBBY_CLASSSELECT_SL00:2`, asset `MAP_D3C4AD9CE187_BG_ELG_FILENYSUSM_STATUE01F_SM_DODO_FR_9DAD46F71CD6`) |
| runtime 에만 있는 placementId | 0 |
| 같은 placementId 인데 assetId 다름 | **350** |
| 서로 다른 (source, runtime) assetId 조합 | 28 |

350 건 모두 `<base>_FR_<hash>`(source) ↔ `<base>`(runtime) 형태다.

## 4. 어느 쪽이 낡았는가 — 증거

**runtime 이 낡았다.** 근거 셋.

1. **publisher 자체 판정.** `Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00 -Mode Check`
   가 스스로 실패했다.
   ```
   Map runtime output differs from authoring: …\Client\Bin\DataFiles\Map\LV_LOBBY_CLASSSELECT_SL00.mapassets
   (Publish-MapAuthoring.ps1:3473)
   ```
2. **저장소 계약.** `AGENTS.md` — "MapTool 은 `Data/Maps/Authoring` 에 저장한다. 검증/publish 도구만
   `Client/Bin/DataFiles/Map` 런타임 문서를 교체할 수 있다." runtime 은 파생 산출물이지 정본이 아니다.
3. **포함 관계.** runtime 이 source 의 진부분집합(runtime-only 0 건)이라는 것은
   source 가 더 나중에 항목이 추가된 상태임을 뜻한다. 반대 방향의 증거는 없다.
4. **MapCatalog 메타데이터가 이미 source 수치였다.** `Data/Maps/MapCatalog.json` 의
   Character Select 항목은 게시 전부터 `placementCount: 805`, `assetCount: 209` 로
   선언돼 있었다. 낡은 runtime 의 804/181 이 아니라 source 의 805/209 다.
   카탈로그 선언과 runtime 산출물이 어긋나 있었다는 직접 증거다.

git 이력은 원인을 특정하지 못한다. 네 파일(source/runtime × catalog/placements) 모두
마지막 변경 커밋이 `560741ac character select, showtime, pattern3, valtan restore` 로 같고
mtime 도 전부 `Sep 16 11:00`(체크아웃 시각)이다. 즉 **커밋된 상태 자체가 이미 불일치**였고,
그 커밋에서 source 재추출 후 runtime 재게시를 빠뜨린 것으로 보인다(— 이 인과는 추론이다).

`_FR_` 특수 처리가 publisher 에 있어서 의도적으로 collapse 한 것은 아니다.
`Publish-MapAuthoring.ps1` 전체에 `_FR_` / variant 관련 분기는 **0 건**이다(grep 확인).

## 5. Bern 도 같은 문제인가 — 아니다

`LV_BER_BERNCASTLE` 는 shard-set 이라 23 개 shard 를 각각 대조했다.

| 항목 | 값 |
|---|---|
| 카탈로그 합계 | source 19152 = runtime 19152, 불일치 shard **0** |
| runtime 에 없는 shard | 0 |
| 배치 | 저작본 50017 vs runtime shard 합계 50017 |
| source 에만 / runtime 에만 / assetId 다름 | 0 / 0 / **0** |

두 검사(`MapAssetCatalog.cpp:321`, `MapTool_Area.cpp:878`) 모두 Bern 에서는 통과할 자료 상태다.
(코드 판정이며 실행 확인은 사용자 몫이다.)

## 6. 쿠크·발탄과의 차이

| Area | source 카탈로그 | runtime 카탈로그 | 차이 | `_FR_` 보유 |
|---|---|---|---|---|
| LV_LOBBY_CLASSSELECT_SL00 | 209 | 181 | **28** | S 145 / R 117 |
| LV_LUT_MIDNIGHTC_ED (쿠크) | 1303 | 1303 | 0 | 0 / 0 |
| LV_BER_BERNCASTLE | 19152 | 19152 | 0 | 0 / 0 |
| LV_LUT_HEARTRB_ED (발탄) | 8 (mapset) | 8 | 0 | 0 / 0 |

쿠크·발탄·베른은 `_FR_` 재질 변형 ID 를 아예 쓰지 않아 재추출로 ID 집합이 흔들린 적이 없고,
마지막 저작 변경 때마다 publish 가 함께 돌아 runtime 이 source 와 같은 세대로 유지됐다.
Character Select 만 `_FR_` 변형 재추출이 들어갔고 그 뒤 runtime 재게시가 빠졌다.

## 7. 수행한 조치

원인이 "runtime 산출물이 낡은 것"으로 확정됐으므로 **데이터 쪽에서 재게시**로 해결했다.
C++ 수정은 하지 않았다(검사 로직은 정상 동작했고, 오히려 낡은 runtime 을 정확히 잡아냈다).

```
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 `
  -AreaId LV_LOBBY_CLASSSELECT_SL00 -Mode Publish
```

```
AreaId         : LV_LOBBY_CLASSSELECT_SL00
Mode           : Publish
Scope          : Area
CatalogType    : single
PlacementCount : 805
FileCount      : 4
Sha256         : b798152c0c6106cb9db19aa510494ce172bb3f7789cd27a3264e8c38992969ae
```

### 변경 파일

| 파일 | LFS | 변경 |
|---|---|---|
| `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapassets` | lfs | 181 → 209 행 |
| `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements` | lfs | 804 → 805 배치, 350 행 assetId 교정 |
| `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json` | 일반 | 219 → 247 항목, `diffuseBrightness` 0.0 → 1.0 6 건 |
| `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.maplights.json` | 일반 | **내용 무변경**(publisher 가 다시 썼으나 diff 0) |

`Data/` 아래 저작 원본은 **한 파일도 수정하지 않았다.**
`Data/Maps/MapCatalog.json` 이 `git status` 에 modified 로 보이지만 그 diff 는 전부
다른 작업의 발탄 `sourceCameraShots`/`sequences` 선언이며 이번 게시가 건드린 부분은 없다.

## 8. 재검증 (불일치 0)

```
Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00 -Mode Check
→ 실패 없이 완료 (PlacementCount 805, FileCount 4)

catalog   S=209 R=209  S-only=0  path다름=0
          runtime hdr: LOSTARK_MAP_ASSET_CATALOG 5 "LV_LOBBY_CLASSSELECT_SL00" 209 "…mapmaterials.json"
placement S=805 R=805  S-only=0 R-only=0 assetId다름=0
materials 저작본 247 = 현재 runtime 247 (diffuseBrightness 분포 완전 일치)
```

`git diff --check` 종료코드 0, 오류 없음.
(`maplights.json`/`mapmaterials.json` 에 대한 `LF will be replaced by CRLF` 경고는 git 의
`core.autocrlf` 일반 안내이며, 두 파일의 CR 줄 수는 HEAD 와 동일해 줄끝이 바뀌지 않았음을 확인했다.)

## 9. 사용자가 할 일

**빌드 불필요.** C++ 은 한 줄도 바뀌지 않았다. 데이터 재게시만 했으므로 Client 를 다시 실행하면 된다.

1. 실행 중인 Client 종료 후 재실행 (런타임 맵 문서를 진입 시 읽는다)
2. Lobby → Character Select 진입 → F1 → Map Tool
3. 정상이면 `Runtime/source model identity differs: …` 가 사라지고 Area 콤보에
   `Character Select [LV_LOBBY_CLASSSELECT_SL00]` 가 잡히며 `Workspace status` 에 활성 Area 와
   배치 수(805)가 표시된다
4. Navigation 탭 활성 → Bake → **Save** → `Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource/.navpaint` 갱신
5. Server 반영: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` Validate → Publish → Server 재시작

⚠️ **World Gameplay 탭의 Save 는 누르지 말 것.** `Gameplay.world.json` 을 전부 재작성한다(float32 오염 이력).

### 화면에서 함께 확인할 것

이번 재게시로 Character Select 의 **외형이 달라진다.** 낡은 runtime 에 있던
`diffuseBrightness 0.0` 재질 6 개가 저작본 값 `1.0` 으로 바뀌고, 기둥 350 개가 base 모델 대신
재질 변형 모델을 쓰며, 배치 1 개(석상)가 추가된다. 이는 저작본이 원래 의도한 상태이지만
**화면 판정은 사용자 몫**이다. 이상하면 `git checkout -- Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.*`
로 되돌릴 수 있다(단 그러면 Map Tool 바인딩도 다시 막힌다).

## 10. 이번에 한 실수

1. 문구 출처를 지시받은 대로 `MapTool_Area.cpp:822~835` 로 가정하고 찾기 시작했다.
   실제 출처는 `MapAssetCatalog.cpp:323` 이었다. grep 으로 먼저 확인해 바로잡았다.
2. 배치 문서 파서에서 따옴표 토큰 순서를 잘못 잡아(첫 토큰을 assetId 로 간주)
   "source 배치에만 쓰인 assetId 1 개" 라는 무의미한 결과를 한 번 냈다.
   실제 행을 눈으로 확인한 뒤 assetId 가 네 번째 따옴표 토큰임을 알고 다시 계산했다.
3. Bern/쿠크/발탄 Check 를 한 번에 돌려 600 초 타임아웃으로 백그라운드로 밀렸다.
   Bern 은 50017 배치라 오래 걸린다는 점을 먼저 고려했어야 했다.
   (다만 Map Tool 이 실제로 보는 두 비교는 파일 직접 대조로 이미 0 불일치를 확인했다.)

## 확인 사실 / 미확인 구분

- **확인**: 검사 위치와 비교 대상, 28 개 카탈로그 누락, 350 건 assetId 차이, publisher Check 의
  실패·성공, 재게시 후 불일치 0, 변경 파일 목록과 LFS 여부, 줄끝 보존, `Data/` 저작본 무변경,
  Bern/쿠크/발탄의 자료 일치.
- **미확인**: Character Select 에서 Map Tool 이 실제로 Area 를 잡고 Navigation 이 활성화되는지
  (Client 실행 필요, 사용자 몫). 재게시로 바뀐 외형이 의도대로인지. `560741ac` 커밋에서
  재게시가 누락된 경위(인과는 추론).
