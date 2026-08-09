# Bern 이동 시 카메라·프러스텀 프레임 순서 수정 구현 계획서

## 목표

Bern 제품 맵에서 정지 화면은 정상인데 로컬 플레이어가 이동하거나 최초 follow camera가 연결되는 프레임에 정적 맵 배치가 큰 덩어리로 사라지는 현상을 닫는다. 렌더링 대상 목록을 만드는 `Late_Update` 전에 Server snapshot과 Level gameplay update를 먼저 반영하여, follow camera와 CPU frustum culling이 같은 프레임의 최종 캐릭터 Transform을 소비하게 한다.

이번 변경은 이펙트 문서·이펙트 런타임·Deferred/HDR shader·맵 placement/data를 수정하지 않는다. 현재 다른 세션이 변경 중인 파일을 되돌리거나 재정렬하지 않고 `CGameInstance::Update_Engine`의 독립된 호출 순서와 새 전용 audit만 추가한다.

## 현재 코드 실측

현재 프레임 호출 순서는 다음과 같다.

```text
CMainApp::Update
→ CNetworkManager::Update
→ CGameInstance::Update_Engine
  → CObject_Manager::Priority_Update
  → Refresh_CameraState
  → CObject_Manager::Update
  → CPhysics_Manager::Update
  → CObject_Manager::Post_Physics_Update
  → CObject_Manager::Late_Update
     → CCamera_Free::Late_Update
        → Update_FollowCamera
        → CCamera::Update_PipeLine
        → CGameInstance::Refresh_CameraState
     → CMapStaticBatchObject::Late_Update
        → Upload_VisibleInstances
        → CFrustum::isIn_Frustum_InWorldSpace
     → CMapAssetObject::Late_Update
        → CFrustum::isIn_Frustum_InWorldSpace
  → CLevel_Manager::Update
     → CLevel_Bern::Update
        → CClientReplication::Update
        → Apply_WorldSnapshot
        → CCharacter::Apply_NetworkState
        → Bind_CameraToLocalCharacter
```

`CMapStaticBatchObject`와 `CMapAssetObject`는 `Late_Update`에서 이번 프레임 render queue와 visible instance buffer를 확정한다. 그러나 Bern의 authoritative snapshot 적용과 최초 camera target bind는 그 뒤의 `CLevel_Bern::Update`에서 실행된다. 따라서 snapshot 또는 follow bind가 발생하는 경계 프레임에는 컬링 결과가 이전 상태를 기준으로 고정된 뒤 캐릭터/카메라 상태가 바뀔 수 있다. 첫 번째 이미지의 전체-map fallback camera와 두 번째 이미지의 부분 배치만 남는 화면은 이 순서 불일치와 일치한다.

반면 다음 항목은 실측상 직접 원인이 아니다.

- `CModel` local bounds는 모든 static vertex에 `0.01` map pre-transform을 적용하여 집계한다.
- `Build_StaticInstance`는 local AABB의 대각 반경과 signed scale 최대 절댓값으로 보수적인 world sphere를 만든다.
- `CFrustum`의 sphere 판정은 양의 radius를 plane distance와 비교하며, 이번 현상처럼 이동 입력 경계에서만 상태가 바뀌는 원인을 만들지 않는다.
- 현재 dirty HDR/shadow 변경은 별도 render pass이며 카메라 frustum의 world plane을 갱신하지 않는다.

## 변경 파일

| 파일 | 변경 이유 |
|---|---|
| `Engine/Private/GameInstance.cpp` | Level gameplay가 snapshot/target Transform을 commit한 뒤 camera follow와 map culling을 수행하도록 frame phase 순서를 교정한다. |
| `Tools/ProjectAudit/Test-CameraFrustumFrameOrder.ps1` | replication → follow camera → map culling 순서가 다시 뒤집히지 않도록 독립 회귀 계약을 검사한다. |
| `.md/GB/08-09/2026-08-09_BERN_CAMERA_FRUSTUM_FRAME_ORDER_DETAIL_PLAN.md` | 적용 후 기존 CPP 전체 코드와 새 audit 전체 코드를 보존한다. |
| `.md/GB/08-09/2026-08-09_BERN_CAMERA_FRUSTUM_FRAME_ORDER_RESULT.md` | 실제 diff, 자동 검증, 수동 Bern smoke와 남은 경계를 분리해 기록한다. |

새 C++ 파일과 public header는 추가하지 않으므로 `.vcxproj`와 `.vcxproj.filters` 등록은 없다. Engine public header 변경도 없으므로 이 수정 자체 때문에 `UpdateLib.bat`의 header 동기화가 필요한 것은 아니지만, 정본 빌드 순서 검증에서는 기존대로 Engine → UpdateLib → Client를 실행한다.

## G00. 프레임 순서 원인 고정

`CGameInstance::Update_Engine`에서 `CLevel_Manager::Update`를 `Post_Physics_Update` 뒤, `CObject_Manager::Late_Update` 앞으로 이동한다.

변경 후 호출 흐름은 다음 하나로 고정한다.

```text
network queue pump
→ object Update
→ physics
→ post-physics pose commit
→ current Level gameplay update
   → replication snapshot commit
   → player Transform/target bind
→ object Late_Update
   → follow camera commit + pipeline/frustum refresh
   → map visible-list build + render queue submit
→ render
```

Level 전환은 여전히 `CMainApp::Apply_LevelRequest`만 현재 Level update가 끝난 뒤 수행한다. `CGameInstance` 안에서 `Change_Level`을 호출하지 않는다. 물리 시뮬레이션과 post-physics pose commit 순서도 유지한다.

종료 증거:

- 전용 audit가 `Post_Physics_Update < Level_Manager::Update < Object_Manager::Late_Update`를 확인한다.
- 기존 camera audit의 `Late_Update → Update_FollowCamera → Update_PipeLine → Refresh_CameraState` 계약을 유지한다.

## G01. camera/frustum 전용 회귀 audit

새 PowerShell audit는 다음을 함께 검사한다.

1. Engine frame phase에서 Level update가 Late update보다 먼저 실행된다.
2. Bern Level update가 `m_Replication.Update()`와 `Bind_CameraToLocalCharacter()`를 소유한다.
3. follow camera가 Late update에서 pipeline/frustum을 갱신한다.
4. static batch와 fallback map object가 Late update에서 world-space frustum을 소비한다.
5. `CObject_Manager`의 ordered layer traversal에서 `Layer_Camera`가 `Layer_MapStaticBatch`와 `Layer_MapAsset_...`보다 먼저 온다는 현재 tag 계약을 확인한다.

실패하면 어떤 단계가 역전됐는지 예외 메시지로 남기며 partial PASS를 출력하지 않는다.

## G02. 빌드·런타임 검증

자동 검증은 다음 순서로 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-CameraFrustumFrameOrder.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

다른 세션의 실행 중 `Client.exe`/`Engine.dll`을 종료하지 않는다. Debug 출력이 점유된 동안에는 먼저 Release 정본을 검증하고, 해당 세션이 종료된 뒤 Debug 정본을 실행한다. 점유가 계속되면 RESULT에 Debug link 미실행을 PASS로 쓰지 않는다.

수동 Bern smoke는 `Client/Default` 작업 디렉터리와 팀 LAN endpoint를 사용한다.

```text
Lobby → Bern Server 승인 진입
→ 최초 local character spawn/follow bind 화면 확인
→ 정지 상태 screenshot 기준 정상 맵 확인
→ 우클릭 이동을 여러 방향으로 연속 제출
→ 이동 시작/정지 및 snapshot 경계에서 중앙·화면 가장자리 정적 구조물 pop/대규모 절단 없음 확인
→ F6 free/follow 왕복 뒤 같은 확인
```

성공 기준은 세 번째 이미지처럼 플레이어 주변 도로·벽·식생이 이동 프레임에도 연속적으로 유지되고, 첫 번째/두 번째 이미지처럼 전체-map fallback 또는 다른 camera 상태의 visible list가 섞이지 않는 것이다.

## 변경하지 않는 경계

- CPU frustum 알고리즘과 world sphere 크기를 임의로 완화하거나 culling을 끄지 않는다.
- Bern placement/catalog/runtime Resources를 재생성하지 않는다.
- Effect Tool, effect document, effect shader, animation cue를 수정하지 않는다.
- HDR, SSAO, bloom, shadow 품질 설정을 되돌리거나 재정렬하지 않는다.
- 실행 중인 다른 세션의 Client/Server process를 종료하지 않는다.
