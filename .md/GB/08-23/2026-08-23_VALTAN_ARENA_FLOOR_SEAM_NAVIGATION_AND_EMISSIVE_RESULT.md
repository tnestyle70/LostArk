# 2026-08-23 발탄 아레나 바닥 균열 — 낙하 보정과 발광 강도 결과

담당: 맵/발탄. 최종 화면 판정자: 사용자.

## 1. 증상과 원인

사용자 증상은 두 가지였고 원인이 하나로 이어진다.

- 아레나 바닥 균열의 청록 발광이 보이지 않는다.
- 그 균열 사이로 플레이어가 계속 빠진다.

`BG_RAD_VALTAN_FLOOR01A/B_SM`의 submesh 1은 바닥 균열이며 실제 지오메트리 틈이다.
Navigation bake의 레이가 그 틈으로 통과해 두 종류의 자국을 남겼다.

| 자국 | navsource 상태 | 결과 |
|---|---|---|
| 핀홀 | `surface=0 walkable=0 height=0` | 그 셀이 walkable에서 빠짐 |
| 이음매 | `surface=1 walkable=1` + 이웃보다 크게 낮은 height | 그 셀 위에서 Y가 급락 |

낙하의 실제 경로는 `Update_PlayerFall`이 아니다. 그 함수는 void region에서만 낙하를 만들고
발탄의 void region은 84줄/30줄 붕괴 섹터 6개뿐이라 평상시에는 발동하지 않는다.
실제 원인은 이동 경로의 Y다. `GameRoom.cpp`의 이동 단계는 `MovePath` 웨이포인트의
`pathPoint.y`를 그대로 목표 Y로 쓰고, 웨이포인트 높이는 `Cell_ToPoint`가 반환하는
`m_Heights[index]`다. 따라서 이음매 셀을 지날 때마다 플레이어 Y가 이웃 바닥보다
최대 10.62 m 아래로 내려갔다가 돌아온다.

핀홀은 별도의 부작용도 만들었다. `Sample_Position`은 walkable이 아니면 실패하고
`Has_LineOfSight`는 0.125 m 간격으로 그 함수를 호출한다. 0.5 m 핀홀 하나가 그 위를 지나는
모든 직선 시야를 끊어 `Smooth_MovePath`의 string-pull이 무너지고 경로가 지그재그가 됐다.

`Data/Navigation/LV_LUT_HEARTRB_ED.navpaint`에는 이미 이음매 셀 43개가 `WALKABLE`로
칠해져 있었다. 이전 시도가 걷힘만 복구하고 높이는 baked 값을 그대로 뒀기 때문에 화면상
변화가 없었다.

## 2. 구현한 계약

### 2.1 `WALKABLE` 페인트가 이음매를 평탄화한다

`Tools/NavigationPipeline/Publish-ServerNavigation.ps1`을 수정했다.

- 페인트 루프 이전에 baked `resolved`/`heights` 스냅샷을 떠서, 치유 판정이 페인트 행 순서에
  의존하지 않게 했다.
- `WALKABLE` 행이 내부 셀을 가리키면 그 셀의 8-이웃 중 baked surface를 가진 것들의 높이를
  모아 중앙값을 낸다. 그런 이웃이 5개 이상이고 그중 4분의 3 이상이 중앙값에서 `0.5 m`
  안이면 그 중앙값을 이음매 높이로 삼는다.
- 대상 셀이 unresolved면 resolved로 승격하고 그 높이를 넣는다.
- 대상 셀이 resolved이지만 이음매 높이보다 `0.5 m` 넘게 낮으면 이음매 높이로 교체한다.
- 조건을 못 채우면 치유하지 않는다. unresolved 셀에 대한 `WALKABLE`이 아닌 페인트와 grid
  가장자리 셀은 계속 fail-closed한다.
- 중앙값을 쓰는 이유는 이음매가 선으로 이어져 자기 셀끼리 맞닿기 때문이다. 이웃 전원 일치를
  요구하면 맞닿은 두 자국이 서로를 영원히 붙잡는다. 실제로 `(323,73)`과 `(324,73)`이 그
  교착이었다. 반대로 램프나 단차처럼 진짜로 높이가 갈리는 자리는 4분의 3 조건을 못 넘겨
  계속 거부된다. `(293,98)`이 그 예이며 baked 값 그대로 published된다.
- 출력 `.navgrid` 포맷은 바뀌지 않는다. Server와 Client 코드는 수정하지 않았다.

### 2.2 페인트 행 221개 추가

`LV_LUT_HEARTRB_ED.navpaint`에 `WALKABLE` 221행을 추가했다(7037 → 7258). 대상 선정은
위 publisher 규칙을 그대로 적용한 뒤, 8-이웃 중 5개 이상이 플레이어가 실제로 도달하는
walkable 성분(스폰섬과 아레나)에 속하는 셀만 남겼다. 도달할 수 없는 잡석 구역은 규칙을
통과해도 제외했다.

### 2.3 발광 강도

`Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployassets`의
`VALTAN_FLOOR_BRICK_A/B` `emissiveIntensity`를 `0.35`에서 `1.5`로 올렸다. `VALTAN_FLOOR_RAIL`은
`1 0` 그대로다.

측정 근거는 다음과 같다.

| 항목 | 값 |
|---|---|
| 재구성 마스크 밝은 텍셀(64+, 텍셀의 5.09%) 평균 | sRGB `(7,109,64)`, 색상 153도 |
| 마스크 최대 텍셀 | sRGB `(12,180,105)`, 선형 G `0.451` |
| 강도 `0.35` 적용 시 밝은 텍셀의 선형 G | `0.054` |
| 원본 영상 발광 상위 5% | sRGB `(135,212,196)`, 색상 168도, 선형 G `0.666` |
| 원본 영상 주변 바닥 | sRGB `(42,49,51)` |

강도 `0.35`는 원본 대비 약 12배 어두웠다. `1.5`에서 마스크 최대 텍셀의 선형 G가
`0.451 x 1.5 = 0.677`이 되어 원본 상위 5%의 `0.666`과 맞는다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| `Publish-ServerNavigation.ps1` PowerShell parse | PASS |
| `Publish-ServerNavigation.ps1 -Mode Validate` | PASS, walkable `21381 -> 21524` |
| `Publish-ServerNavigation.ps1 -Mode Publish` 2회 연속 sha256 | 동일, 결정적 PASS |
| `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED` | PASS, placement 13186, sha256 `8945e9e7...` 유지 |
| deployassets source/runtime parity | PASS, A/B `1.5 1`, RAIL `1 0` |
| `Publish-WorldGameplay.ps1 -Mode Validate` | PASS |
| `Server.exe --contract-test` | 705 PASS / failures 0 |
| `git diff --check` | 오류 없음 |

### 수치 확인

| 항목 | 수정 전 | 수정 후 |
|---|---|---|
| 아레나 바닥 평면(22.5~23.5 m) 낙차 0.5 m 초과 셀 | 다수, 최악 10.62 m | **0** |
| 아레나 무작위 200쌍 직선 시야 | — | 199/200 |
| walkable 셀 | 21381 | 21524 |
| 연결 성분 | 449 | 448 |

baked에 표면이 없던 셀 143개가 새로 resolved되고, 표면은 있으나 높이가 꺼져 있던 셀 202개의
높이를 교정했다. 그중 105개가 아레나 22 m 안이다. 최대 교정은 `(268,137)`의 `8.01 -> 23.05`,
즉 15.04 m다. 스폰섬과 아레나는 그대로 분리돼 있고 `player_Move.1` movePlayer 트리거가 계속
유일한 연결이다. `LV_BER_BERNCASTLE`, `LV_DEV_TRAINING_GROUND`, `LV_LOBBY_CLASSSELECT_SL00`
navgrid는 수치가 바뀌지 않았다.

## 4. 확인했지만 결함이 아니었던 것

발광 경로는 데이터와 코드 전 구간이 정상이다. 강도 외에 고칠 곳이 없었다.

- `.wmat` material 1이 emissive PNG를 4번째 텍스처 필드에 참조한다.
- `ResolveBelowAssetRoot`가 `Resource/LostArk/` 접두사를 벗겨 실제 경로로 해석한다.
- `CMaterial`이 `aiTextureType_EMISSIVE` 슬롯을 만든다.
- `.wmesh` submesh 1의 `materialIndex`가 1이다.
- `RENDERGROUP::DEFERRED_OVERLAY`가 존재하고 `Render_NonBlend`가 모든 NONBLEND 뒤,
  `MRT_GameObject`를 닫기 전에 처리한다.
- `Render_DeferredOverlay`는 `CGameObject`에서 virtual이고 `CDeployPropObject`가 override한다.
- 셰이더 pass 15 `DeferredEmissiveOverlayPass`가 존재하며 `SV_TARGET4`만 기록하고
  `DSS_ReadOnly`의 `DepthFunc`가 `less_equal`이라 동일 깊이 재기록이 통과한다.
- MRT slot 4가 `Target_Emissive`이고 `Render_Combined`가 이를 `g_EmissiveTexture`로 바인딩해
  `vLitColor + vEmissive`로 더한다.
- Deploy prop 초기 상태는 `INTACT`이고 억제는 파괴 경로에서만 켜진다.
- emissive PNG는 1024x1024 8-bit RGB 비인터레이스로 WIC가 읽을 수 있는 형식이다.

발탄 스폰섬이 아레나와 분리된 것은 결함이 아니다. `player_Move.1`과
`player.spawn.editor`의 movePlayer가 두 성분을 잇는 설계다.

## 5. 남은 경계

- 재구성 마스크의 색상이 153도로 원본 168도보다 초록에 가깝다. 강도와 무관한 별도 항목이며
  마스크 PNG는 팀장이 관리하는 `Client/Bin/Resources` 자산이라 이번에 교체하지 않았다.
  마스크가 돌 하나하나의 테두리를 전부 그리고 있어 원본의 성긴 균열 몇 줄과 성격이 다르다.
- 아레나 바닥 평면에 뚫린 셀 10개가 남아 있다. `(296,102)` 하나만 unresolved이고 나머지
  9개는 `surface=1 walkable=0`, 즉 실제 표면이 있는데 베이크가 못 걷는다고 판정한 장애물이다.
  플레이어가 올라서지 않으므로 낙차를 만들지 않는다. 근거 없이 바닥으로 바꾸지 않았다.
- `(293,98)` 주변은 15 m 경사와 25 m 단이 섞인 실제 지형이라 규칙이 계속 거부한다. 낙차
  측정에서 9.60 m로 잡히지만 자국이 아니다.
- 플레이어가 도달할 수 없는 성분의 자국은 손대지 않았다.
- 화면 판정은 사용자 전용이다. 아래 절차 전에는 visual PASS로 기록하지 않는다.

## 6. 사용자 수동 확인

Server와 Client를 모두 재시작해야 한다. 두 변경 모두 데이터 파일이므로 재빌드는 필요 없다.

1. Server를 재시작한다. `Server/Bin/<Configuration>/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid`를 새로 읽는다.
2. Client를 재시작하고 Lobby에서 발탄으로 진입한다.
3. 아레나를 가로질러 우클릭 이동하면서 바닥 균열 위에서 캐릭터가 꺼지거나 빠지지 않는지 본다.
4. 같은 이동에서 경로가 직선에 가깝게 나오는지, 지그재그가 줄었는지 본다.
5. 돌 틈에 청록 발광선이 보이는지, 비스듬한 카메라에서 깜빡임이나 바닥 밖 발광이 없는지 본다.
6. 84줄/30줄 붕괴 후 사라진 sector의 균열 발광도 함께 사라지는지 본다.
