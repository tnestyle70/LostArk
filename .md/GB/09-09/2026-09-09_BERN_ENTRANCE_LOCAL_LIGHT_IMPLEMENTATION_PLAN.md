# 2026-09-09 베른 입장 카메라 경로의 국소 조명 구현 계획

기준: `codex/dimensionmaster-tool-round3`, `591012dbebf7eeab0b660baec42852b9396e77d4`의 기존 dirty 상태를 보존한다. 사용자가 전체 맵보다 스폰부터 현재 카메라 연출 구간의 light를 먼저 복구하도록 범위를 확정했다.

## G00. 스폰과 카메라가 실제 소비하는 구간

`Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`의 활성 spawn 4개는 X136.236~139.377, Y42.250~42.508, Z-25.093~-20.246이다. `Data/Encounters/Bern/BernEntranceCamera.json`은 16초/16key이며 eye와 lookAt이 이 스폰에서 Z-175.937까지 이어진다. 이 JSON은 `CLevel_Bern::Ready_EntranceCinematic -> CValtanCinematicCameraController::Sample_Cue`가 실제 소비한다.

현재 설치본 `LV_BER_BERNCASTLE_T_*` 36package를 headless 직접 읽었다. Light 340개 중 PS/SL00~SL10 304개를 상시 맵 후보로 삼고 EVENT/SCENE 36개는 이번 조명에서 제외한다. 카메라 eye/lookAt 경로와 이 둘을 잇는 선분, spawn에서 source point sphere까지의 3D 거리가 12m 이내인 10개를 선택한다. 12m는 60도 시야의 가까운 주변 표면을 포함하기 위한 프로젝트 선택 여백이며 원작 활성 스트리밍 증거가 아니다. 해당 국소 후보에 Spot은 없다. 스폰 바로 옆에 원본 local light는 없어 임의 새 key/fill을 추가하지 않는다.

좌표는 기존 `build_maptool_scene.py::convert_position`과 동일한 `(X,Z,-Y)*0.01`이다. RGB는 원본 Color byte/255를 기록하고 광원 alpha는1이다. 원본 Color alpha0을 화면광 opacity로 해석하지 않는다. 원본이 직렬화하지 않은 radius1024cm, brightness1, falloff2, color white는 이번 프로젝트 대체값이며 원작 class-default를 확인했다고 주장하지 않는다. 따라서 기존 v2 schema의 `PROJECT_AUTHORED`를 사용한다.

| 원본 component | Client 좌표(m) | 반경(m) | 밝기 | falloff | 같은 GUID의 source baked component 수 | 프로젝트 대체 field |
|---|---|---:|---:|---:|---:|---|
| SL00:859 `pointlight_14_lc` | (133.542, 36.104, -163.748) | 12 | 0.2 | 2 | 113 | falloffExponent |
| SL00:861 `pointlight_18_lc` | (157.530, 34.764, -90.803) | 10.24 | 0.4 | 2 | 105 | rangeMeters, falloffExponent |
| SL00:862 `pointlight_19_lc` | (148.807, 34.764, -97.700) | 10.24 | 0.4 | 2 | 111 | rangeMeters, falloffExponent |
| SL00:863 `pointlight_1_lc` | (147.554, 44.715, -159.891) | 3 | 2 | 2 | 16 | falloffExponent, color |
| SL00:865 `pointlight_22_lc` | (125.282, 39.036, -154.189) | 12 | 0.3 | 2 | 112 | falloffExponent |
| SL00:866 `pointlight_27_lc` | (138.740, 36.104, -163.748) | 12 | 0.6 | 2 | 107 | falloffExponent |
| SL00:868 `pointlight_53_lc` | (127.368, 35.924, -161.650) | 12 | 0.3 | 2 | 125 | falloffExponent |
| SL00:869 `pointlight_55_lc` | (141.153, 55.916, -168.050) | 5 | 1 | 4 | 83 | brightness |
| SL00:870 `pointlight_64_lc` | (132.833, 55.916, -168.050) | 5 | 1 | 4 | 90 | brightness |
| SL00:872 `pointlight_9_lc` | (123.999, 30.458, -98.901) | 12 | 0.5 | 2 | 107 | falloffExponent |

현재 베른 runtime mapmaterials/placementLighting은0이고 WModel1,003개의 UV1도0이므로 이번 direct와 중복되는 제품 RNM 기여는 없다. 원본 후보10개는 baked GUID가 있으므로 앞으로 이 구간 RNM을 연결할 때 같은 GUID direct 대체를 제거하거나 직접광/구운 diffuse 수신을 분리한다. 원본340개를 모두 dynamic으로 올리지 않는다. 현재56개 프레임 공유 budget 안에서 이 구간10개만 제출한다.

## G01. 국소 maplight JSON과 Catalog

새 파일은 `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.maplights.json`이다. stable ID는 source level/export를 포함한다. `Data/Maps/MapCatalog.json`의 Bern 항목에 canonical `sourceLights/lights` pair를 추가한다. runtime 파일은 기존 `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE`가 생성한다.

```json
{
  "schema": "lostark.map-light-presentation",
  "formatVersion": 2,
  "areaId": "LV_BER_BERNCASTLE",
  "provenance": "PROJECT_AUTHORED",
  "nextLightOrdinal": 11,
  "lights": [
    {
      "lightId": "light.bern.entrance.sl00.859",
      "displayName": "SL00 export 859 pointlight_14_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        133.541835937,
        36.103962402,
        -163.7484375
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 12.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.788235294,
        0.91372549,
        1.0,
        1
      ],
      "brightness": 0.20000000298023224
    },
    {
      "lightId": "light.bern.entrance.sl00.861",
      "displayName": "SL00 export 861 pointlight_18_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        157.530068359,
        34.764008789,
        -90.802792969
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 10.24,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.788235294,
        0.91372549,
        1.0,
        1
      ],
      "brightness": 0.4000000059604645
    },
    {
      "lightId": "light.bern.entrance.sl00.862",
      "displayName": "SL00 export 862 pointlight_19_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        148.806611328,
        34.763981934,
        -97.700146484
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 10.24,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.788235294,
        0.91372549,
        1.0,
        1
      ],
      "brightness": 0.4000000059604645
    },
    {
      "lightId": "light.bern.entrance.sl00.863",
      "displayName": "SL00 export 863 pointlight_1_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        147.553662109,
        44.715380859,
        -159.890703125
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 3.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        1.0,
        1.0,
        1.0,
        1
      ],
      "brightness": 2.0
    },
    {
      "lightId": "light.bern.entrance.sl00.865",
      "displayName": "SL00 export 865 pointlight_22_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        125.281552734,
        39.035656738,
        -154.189326172
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 12.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.788235294,
        0.91372549,
        1.0,
        1
      ],
      "brightness": 0.30000001192092896
    },
    {
      "lightId": "light.bern.entrance.sl00.866",
      "displayName": "SL00 export 866 pointlight_27_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        138.740498047,
        36.103964844,
        -163.7484375
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 12.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.788235294,
        0.91372549,
        1.0,
        1
      ],
      "brightness": 0.6000000238418579
    },
    {
      "lightId": "light.bern.entrance.sl00.868",
      "displayName": "SL00 export 868 pointlight_53_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        127.368261719,
        35.923964844,
        -161.649560547
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 12.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.788235294,
        0.91372549,
        1.0,
        1
      ],
      "brightness": 0.30000001192092896
    },
    {
      "lightId": "light.bern.entrance.sl00.869",
      "displayName": "SL00 export 869 pointlight_55_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        141.153173828,
        55.915859375,
        -168.050488281
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 5.0,
      "falloffExponent": 4.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.784313725,
        1.0,
        0.980392157,
        1
      ],
      "brightness": 1.0
    },
    {
      "lightId": "light.bern.entrance.sl00.870",
      "displayName": "SL00 export 870 pointlight_64_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        132.833173828,
        55.915859375,
        -168.050488281
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 5.0,
      "falloffExponent": 4.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.784313725,
        1.0,
        0.980392157,
        1
      ],
      "brightness": 1.0
    },
    {
      "lightId": "light.bern.entrance.sl00.872",
      "displayName": "SL00 export 872 pointlight_9_lc",
      "kind": "POINT",
      "groupId": "bern.entrance.source-direct",
      "enabled": true,
      "position": [
        123.999462891,
        30.458337402,
        -98.900664062
      ],
      "rotationDegrees": [
        0,
        0,
        0
      ],
      "rangeMeters": 12.0,
      "falloffExponent": 2.0,
      "innerConeDegrees": 0,
      "outerConeDegrees": 0,
      "color": [
        0.788235294,
        0.91372549,
        1.0,
        1
      ],
      "brightness": 0.5
    }
  ]
}
```

## G02. CLevel_Bern의 수명과 실패 처리

- `Client/Public/Level_Bern.h`: `CMapLightPresentationRuntime` 전방 선언, `m_MapRuntime` 뒤에 강한 owner `m_pMapLightPresentation`과 단발 오류 보고 bool을 추가한다. 새 C++ 파일은 없다.
- `Client/Private/Level_Bern.cpp`: 해당 runtime header를 include한다. `Initialize`에서 map 로드 성공 뒤 local shared runtime으로 JSON을 parse/validate하고 실패하면 map을 clear하며 E_FAIL을 보존한다. 이후 초기화가 모두 성공한 시점에 멤버로 commit한다.
- `Update`: Server world transfer의 early return 뒤에 기존 `Submit_Frame`을 호출한다. 이후 `CPresentation_Manager -> CMapLightPresentationRuntime::Submit_Presentation -> transient point light` 경로가 그 프레임을 소비한다. 제출 실패는 단발 진단으로 보존한다.
- destructor: provider가 들고 있던 문서를 clear하고 강한 owner를 해제한다. 재입장마다 새 문서를 읽으므로 이전 Level의 light가 누적되지 않는다.

새 JSON은 Client `None`와 `.filters`의 `96.DataFiles\Map`에만 노출한다. 프로젝트 등록 두 파일은 root와 소유권을 조정한 뒤 필요한 항목만 추가한다. 지형42개 visible, material, scene 방향광/fog, spawn, 카메라 문서는 이번 변경에서 수정하지 않는다.

## G03. 검증과 인계

1. 원본10개 ID/좌표/반경과 JSON 대응, strict JSON/v2 light validator.
2. `powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Validate`.
3. `git diff --check`와 변경 XML parse. root가 담당하는 Client 최소 compile 및 필요한 publish.
4. 실행 중 Client/Server를 종료하지 않는다. 실행 중인 프로세스가 runtime 문서를 소비할 수 있으므로 이 하위 작업은 source 준비와 Validate까지만 수행하고 실제 publish/빌드 시점은 root가 조정한다.
5. 사용자: 새 실행 파일의 Lobby -> Bern, 기존16초 입장 카메라를 따라 SL00 주변 표면의 빛을 확인한다. ESC skip과 재입장도 직접 확인한다. 실제 Client 실행/프레임 제출/visual PASS는 사용자의 화면 관찰 전 완료로 기록하지 않는다.
