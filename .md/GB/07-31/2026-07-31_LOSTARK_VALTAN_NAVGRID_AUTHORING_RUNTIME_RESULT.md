# LostArk Valtan NavGrid Authoring 및 Runtime Blocker 구현 결과

## 1. 결론

Valtan NavGrid를 다음 세 계층으로 분리했다.

1. Python이 지형에서 높이 후보만 굽는 `.navsource`
2. MapTool 피킹으로 작성하는 정적 이동 불가 셀 `.navpaint`
3. flag/condition으로 켜고 끄는 동적 이동 불가 region `.navblockers`

F5는 계속 runtime Navigation Debug 표시만 담당한다. 베이크와 편집은 F5와 분리되어
있으며, 최종 `.navgrid`는 검증된 source와 paint를 MapTool에서 명시적으로 export할 때만
교체된다.

## 2. 정적 Navigation Authoring

`Tools/LevelPlacementExtractor/build_valtan_navgrid.py`는 glTF와 placement를 읽어
`Client/Bin/DataFiles/Navigation/ValtanArena.navsource`를 만든다. 완만한 면의 높이를
우선하고, 완만한 면이 없는 셀은 가파른 면의 높이를 fallback으로 보존한다. 이 단계에서는
walkable 여부를 판정하지 않는다.

MapTool F1의 Navigation 모드에서 다음 작업을 수행한다.

- `Static Walkability` + `Blocked`: 선택한 셀을 이동 불가로 칠한다.
- `Static Walkability` + `Walkable`: 선택한 셀의 이동 불가 표시를 지운다.
- `Save Paint`: stable cell 좌표를 `.navpaint`에 원자적으로 저장한다.
- `Reload`: source, paint, blocker 문서를 parse → validate → stage → commit 순서로 다시 읽는다.
- `Export Runtime`: 모든 walkable 셀의 높이가 해결된 경우에만 기존 binary `.navgrid`를 쓴다.

색상은 초록이 유효 walkable, 노랑이 수동 blocked, 빨강이 unresolved height다. 따라서
현재 unresolved 셀은 모두 실제 지형 의미를 보고 blocked로 확정하거나 source 범위를
수정하기 전까지 export할 수 없다.

## 3. 계단과 높이 차

walkability와 height를 분리했으므로 계단이 추가되어도 저장 계약을 바꿀 필요가 없다.
각 XZ 셀에 굽힌 높이를 유지하고, A*의 이웃 연결은 기존 `maxStepHeight`를 기준으로
판정한다. 낮은 계단은 연속된 셀 사이의 높이 차가 허용 범위 안이면 연결되고, 절벽은
차이가 허용 범위를 넘으면 끊어진다.

현재 계약은 XZ 하나당 높이 하나인 heightfield다. 같은 XZ에 위층과 아래층이 동시에
존재하는 다층 구조가 필요해질 때만 NavMesh나 layer별 grid로 확장한다.

## 4. 파괴 지형과 Runtime Blocker

MapTool의 `Runtime Region` 모드에서 다음을 저장한다.

- stable blocker ID
- condition ID
- condition이 true/false일 때 blocker가 활성화되는지 나타내는 극성
- 피킹한 cell 집합

Engine은 정적 walkability를 수정하지 않고 runtime blocker reference count를 합성한다.

```text
effectiveWalkable = baseWalkable && runtimeBlockCount == 0
```

여러 blocker가 같은 셀을 점유해도 한 blocker가 해제되었다는 이유로 다른 blocker까지
풀리지 않는다. 상태가 실제로 바뀔 때만 grid revision을 올리며, 이전 revision에서 만든
`CNavPathFollower` 경로는 취소되어 다음 요청에서 다시 탐색된다.

AssetTest의 `Set_DeployPhase()`는 검증용으로 `VALTAN_ARENA_DESTROYED` condition을
연결한다.

- `INTACT`: condition false
- `FRACTURED`, `DESPAWNED`: condition true

실제 레이드에서는 보스 패턴 controller가 정확한 파괴 프레임에 같은 condition을 갱신하면
된다. 지형이 사라진 뒤 막아야 하는 바닥은 `activateWhenTrue`, 벽이 파괴되기 전까지만
막아야 하는 영역은 `activateWhenFalse`로 작성할 수 있다.

## 5. 생성 데이터 검증

| 항목 | 결과 |
|---|---:|
| area ID | `LV_LUT_HEARTRB_ED` |
| grid | `62 x 63` |
| cell size | `0.5` |
| 총 셀 | `3,906` |
| resolved height | `2,897` |
| unresolved height | `1,009` |
| 기존 runtime navgrid 크기 | `19,550 bytes` |
| navsource SHA-256 | `2E7EC2686D327C417D3CDC5863ACC997AD7A07D4DBB6E4D9624F1F3F005CA30A` |

동일 명령으로 source를 다시 생성했을 때 SHA-256이 같았고,
`python -m py_compile Tools/LevelPlacementExtractor/build_valtan_navgrid.py`도 통과했다.
`ValtanArena.navblockers`는 실제 파괴 범위를 추측하지 않기 위해 region 0개로 생성했다.

## 6. 빌드 및 실행 검증

다음 순서로 모두 성공했다.

1. Engine x64 Debug
2. UpdateLib.bat Debug
3. Client x64 Debug
4. Engine x64 Release
5. UpdateLib.bat Release
6. Client x64 Release

최종 산출물은 `Engine/Bin/Debug/Engine.dll`,
`Engine/Bin/Release/Engine.dll`, `Client/Bin/Debug/Client.exe`,
`Client/Bin/Release/Client.exe`다. 기존 CP949 관련 C4819와 DirectXTK PDB 관련
LNK4099 경고가 있었지만 오류는 없었다.

Debug Client는 `Client/Default`를 working directory로 실행했다. 첫 Loading Complete,
Logo, Binary Asset Test Loading Complete를 거쳐 최종 창 제목
`Valtan WModel Asset Test`에 도달했고 프로세스가 응답 중임을 확인한 뒤 검증용 프로세스만
종료했다.

## 7. 남은 콘텐츠 작성

코드와 저장 경로, condition 연동은 완료됐다. 다음 실제 콘텐츠 작업은 MapTool에서
아티스트 또는 레벨 디자이너가 수행한다.

1. 빨간 unresolved 셀을 실제 이동 불가 영역으로 노랗게 확정한다.
2. 계단과 이동 경계의 높이 연결을 F5와 경로 요청으로 확인한다.
3. 파괴 전후 영역을 stable blocker ID와 condition ID로 피킹해 저장한다.
4. `Export Runtime` 후 AssetTest에 재진입해 Character와 Valtan의 우회 및 재탐색을 확인한다.

현재 runtime `.navgrid`는 의도적으로 교체하지 않았다. 미완성 paint를 강제로 export해
기존 이동을 망가뜨리지 않기 위한 선택이다.
