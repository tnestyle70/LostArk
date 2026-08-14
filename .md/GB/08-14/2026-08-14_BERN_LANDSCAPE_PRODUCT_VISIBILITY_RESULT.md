# Bern Landscape 제품 가시성 RESULT

작성일: 2026-08-14

상태: 원인 확정과 코드 반영 완료. 사용자 화면 확인 대기.

연결 문서: [2026-08-04 Landscape 시각 복구](../08-04/2026-08-04_BERN_CASTLE_LANDSCAPE_VISUAL_RECOVERY_RESULT.md)

## 1. 증상

Bern 맵의 일부 구역은 지형이 보이고 일부 구역은 지면이 전혀 없이 하늘만 보인다.
탑과 나무는 떠 있고 구조물의 밑면이 노출된다.
(사용자 관찰, `Screenshots/6.png`, `7.png`, `스크린샷 2026-08-14 034222.png`, 2026-08-14 03:42)

## 2. 원인 — 제품 Level이 landscape 그룹을 제외하고 있었다

`Client/Private/LevelRegistry.cpp`의 Bern descriptor만 asset group 제외를 선언했다.

```cpp
LEVEL::BERN, ... "scene.bern.neutral-day.v1",
MakeFullMapScope("landscape"),      // Valtan은 MakeFullMapScope()
```

`CMapPlacementRuntime::Apply_LoadScope`가 이 값으로 placement를 삭제한다.

```cpp
if (nullptr != pAsset && !loadScope.excludedAssetGroupId.empty() &&
    pAsset->groupId == loadScope.excludedAssetGroupId)
{
    return true;                    // 42개 landscape placement 제거
}
```

Bern shard set은 13개이며 `LANDSCAPE` shard가 asset 42개와 placement 42개를 갖는다.
이 42개가 걷는 구역 전체의 지면이다.

```text
landscape placement 범위   X  -0.6 .. 237.4   Z -197.8 .. 40.3
navigation 통로 범위       X 124.0 .. 149.0   Z -175.9 ..  -9.4
```

통로가 landscape 범위 안에 완전히 들어간다. 따라서 포장된 static mesh가 덮은 광장·계단에서는
바닥이 보이고, 그 밖의 정원·다리 구간에서는 지면이 통째로 사라진다. 스크린샷의 차이가 이것이다.

## 3. 이 제외가 들어온 경위

2026-08-04 Landscape 재추출에서 render/collision 높이 166,698개가 정확히 일치했으나,
2026-08-05 사용자 수동 smoke에서 절벽면의 초록색 수직 늘어짐이 재현됐다. 그래서 잘못된 표현이
마을 제작을 막지 않도록 가역적인 표시 격리를 넣기로 했다.

그 PLAN과 RESULT가 정한 격리 범위는 **Map Editor 전용**이었다.

> 대신 `CMapTool`의 Bern editor view에서만 catalog `groupId == "landscape"`인 placement를
> 기본 비표시한다. ... Bern 제품 Level과 `CMapPlacementRuntime` 공용 경로는 변경하지 않음

MapTool 격리는 지금도 독립적으로 존재한다(`MapTool.cpp:744` `Show Bern Landscape`,
`MapTool.cpp:4537` 필터). 그런데 커밋 `3a16667`이 `LevelRegistry.cpp`의 Bern 제품 descriptor에도
같은 제외를 추가했다. 이것은 위 문서가 명시적으로 제외한 범위였고 별도로 기록되지 않았다.

즉 이번 증상은 우연한 버그가 아니라, 문서화된 authoring 격리가 제품 Level까지 확장된 결과다.

## 4. 반영 내용

- `Client/Private/LevelRegistry.cpp` — Bern descriptor를 `MakeFullMapScope()`로 되돌려 제품
  Level이 published 전체 맵을 로드한다. 이유를 주석으로 남겼다. `MakeFullMapScope`의 optional
  파라미터와 `MAP_LOAD_SCOPE::excludedAssetGroupId` 기구는 그대로 둔다. 격리를 다시 걸어야 하면
  문자열 하나로 되돌릴 수 있어야 하기 때문이다.
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` — `maps.product-editor-visual-scope`가 기존에
  `MakeFullMapScope("landscape")` 존재를 **요구**하고 있었다. 이를 제품 두 Level이 모두 전체 맵을
  로드하는지 검사하도록 교체하고, MapTool의 가역 토글 검사는 유지했다.

Imported/Authoring placement 50,017개, `.mapset`, runtime shard, Resources payload는 건드리지 않았다.

## 5. 자동 검증

```text
Client x64 Debug 빌드/링크: 오류 0
Engine, Shared 동반 빌드: 오류 0
ProjectAudit: maps.product-editor-visual-scope 통과
  (변경 전 실패 목록에 없었고 변경 후에도 maps.* 실패 없음)
```

ProjectAudit 전체는 작업트리 기준 `effect.artist-31470-*`, `levels.character-select-contract`,
`world.authoring-format-v4` 등 진행 중 WIP로 계속 실패한다. 이번 변경 전후 동일하며 전체를
PASS로 기록하지 않는다.

리소스 존재도 확인했다.

```text
Client/Bin/Resources/Map/LV_BER_BERNCASTLE_T/Landscape  구성요소 42개
각 구성요소  <id>.wmodel + textures/baked_diffuse.png + textures/baked_normal.png
runtime catalog LV_BER_BERNCASTLE_LANDSCAPE.mapassets  groupId="landscape" 42행
mapset LANDSCAPE shard  assets 42 / placements 42
```

## 6. 사용자 확인 사항

Client만 다시 시작하면 된다. Server 변경은 없다.

지면이 채워졌는지와 함께, 2026-08-05에 격리 사유였던 **절벽면 초록색 수직 늘어짐**이 지금도
보이는지 함께 봐 달라. 그 결함은 이번 변경으로 교정되지 않았으며 상태가 확인되지 않았다.

- 늘어짐이 보이지 않으면 격리 사유가 해소된 것이므로 이대로 유지한다.
- 여전히 보이면 두 가지 중 하나를 선택한다. 지면 없는 상태로 되돌리거나(문자열 하나 복원),
  `Tools/LandscapeExtractor`의 cliff projection 교정을 별도 작업으로 진행한다.

에이전트는 화면 판정을 대신하지 않는다. 위 관찰은 사용자 확인 후에 기록한다.
