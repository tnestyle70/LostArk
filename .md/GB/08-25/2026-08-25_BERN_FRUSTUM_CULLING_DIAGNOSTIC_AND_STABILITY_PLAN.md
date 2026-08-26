# 베른 프러스텀 컬링 진단·안정화 계획

## 목표

F6 자유 시점에서 플레이어가 멈춰 있어도 카메라 방향만으로 베른 지형이 사라지는 문제를 내비게이션과 분리해 진단한다. 베른만 프러스텀 컬링을 우회하면서 원래 탈락했을 배치의 stable placement ID, 경계구, 카메라 리비전, 여섯 평면 거리를 남긴다. 이후 우회를 끄더라도 같은 카메라 행렬 스냅샷과 보수적인 대형 지형 경계로 팝핑이 재발하지 않게 한다.

## 구현 계약

1. `MAP_LOAD_SCOPE`가 Area별 `MAP_FRUSTUM_CULLING_POLICY`를 소유한다.
2. 진단 단계에서는 베른 descriptor만 `bypass + diagnostics`를 활성화한다. 사용자 확인과 로그 분석 뒤 제품 기본값은 둘 다 끄고 정상 컬링으로 복귀한다. 다른 Level과 MapTool 기본값은 건드리지 않는다.
3. 맵 렌더 직전에 View/Projection을 한 번 복사하고 그 복사본에서 프러스텀 여섯 평면을 만든다.
4. 같은 복사본을 실제 셰이더 View/Projection에도 바인딩한다.
5. 인스턴스 배치와 fallback 배치 모두 `Late_Update`에서는 authored-visible 객체를 큐에 넣고 최종 `Render`에서 판정한다.
6. visible에서 rejected로 바뀌는 순간에만 placement ID, asset/group ID, 중심, 원래 반경, margin, 유효 반경, 여섯 평면 거리를 파일에 기록한다.
7. `landscape` stable group과 반경 4m 이상 대형 geometry는 2m 또는 반경 12% 중 큰 margin을 사용한다. 베른 전체에 0.25m 기본 margin과 3-frame rejection hysteresis를 둔다.
8. binary static model의 local cull AABB는 embedded metadata만 신뢰하지 않고 모든 decoded vertex를 순회해 다시 만든다.

## 변경 파일

- `Client/Public/MapLoadScope.h`
- `Client/Private/LevelRegistry.cpp`
- `Client/Public/MapAssetRenderUtils.h`
- `Client/Private/MapAssetRenderUtils.cpp`
- `Client/Public/MapAssetObject.h`
- `Client/Private/MapAssetObject.cpp`
- `Client/Public/MapStaticBatchObject.h`
- `Client/Private/MapStaticBatchObject.cpp`
- `Client/Public/MapPlacementRuntime.h`
- `Client/Private/MapPlacementRuntime.cpp`
- `Engine/Private/Model.cpp`
- `Tools/ProjectAudit/Test-BernFrustumCullingContract.ps1`

새 C++ 파일은 추가하지 않으므로 `.vcxproj`와 `.filters` 등록 변경은 없다.

## 검증

1. 정적 계약 하네스 PASS.
2. Engine x64 Debug 빌드 PASS.
3. `UpdateLib.bat Debug`로 Engine DLL/LIB/runtime 배포.
4. Client x64 Debug 빌드 PASS.
5. `git diff --check` PASS.
6. 사용자가 Lobby → Character Select → Bern 진입 후 F6으로 동일 위치를 바라보며 깜빡임 소멸 여부를 판정한다.
7. 발생 시 `Client/Bin/Debug/Diagnostics/BernFrustumCulling.log`를 다음 수정 입력으로 사용한다.
8. 우회 상태에서 깜빡임이 사라지면 로그로 오판 경로를 확정하고, `bypass=false`, `diagnostics=false`에서 동일 위치를 다시 확인한다.
