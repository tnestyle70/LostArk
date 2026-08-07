# Bern Camera Follow Stability 구현 계획

## G0. 목표와 완료 경계

- Bern 활성화 첫 화면부터 Server가 승인한 local player spawn을 기준으로 카메라를 배치한다.
- follow camera가 character transform 갱신 뒤 같은 프레임의 View, inverse View, camera position, frustum을 commit하게 한다.
- Bern 제품 follow는 별도 camera lag를 더하지 않고 player의 이미 보간된 transform에 고정 offset으로 붙인다.
- F6 free/follow 전환, Character Select와 Development/Valtan의 기존 `CCamera_Free` 소비 경로는 유지한다.
- Client Debug/Release build, 관련 regression, ProjectAudit, `git diff --check`를 통과하면 자동 검증 완료로 처리한다. 실제 화면 체감 smoke는 별도로 남긴다.

## G1. 현재 원인과 소유자

1. `CLevel_Bern::Ready_Layer_Camera`는 전체 map placement bounds로 초기 View를 만들기 때문에 local spawn 카메라와 약 2,170 world unit 차이가 난다.
2. `CCamera_Free::Priority_Update`가 character `Update`보다 먼저 follow target을 읽고 View를 확정한다.
3. character는 Server snapshot을 보간한 뒤 렌더되지만 camera는 이전 transform을 추가 보간해 frame spike에서 화면 기준이 갈라진다.
4. `CNetworkManager`는 local player spawn packet을 replication queue에만 보관하므로 Bern 활성화 시점의 camera가 승인 spawn을 조회할 수 없다.

상태 소유자는 다음과 같다.

- Server spawn 정답: `S2C_PLAYER_SPAWNED`
- Client session의 최신 local spawn pose: `CNetworkManager`
- 렌더 transform: `CCharacter`
- follow pose와 View commit: `CCamera_Free` / Engine `CCamera`
- Bern 제품 offset과 lag 정책: `CLevel_Bern`

## G2. 변경 파일과 계약

### G2-1. NetworkManager

- `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp`
- local player/entity와 일치하는 `S2C_PLAYER_SPAWNED` 복사본을 session state로 저장한다.
- 새 Enter 승인, local despawn, connection close/reconnect에서는 캐시를 지운다.
- Bern camera가 복사본을 조회하는 read-only 함수만 공개한다. packet queue를 소비하거나 Server 정답을 새로 만들지 않는다.

### G2-2. Camera pipeline

- `Engine/Public/GameInstance.h`, `Engine/Private/GameInstance.cpp`, `Engine/Private/Camera.cpp`
- camera가 Priority 이외의 frame phase에서 View를 바꿔도 inverse transform, camera position, world frustum을 같은 commit으로 갱신하는 Engine 내부 함수를 둔다.
- 기존 Engine update도 같은 내부 함수를 사용한다. Client가 pipeline manager나 frustum을 직접 소유하지 않는다.

### G2-3. Follow camera와 Bern

- `Client/Public/Camera_Free.h`, `Client/Private/Camera_Free.cpp`, `Client/Private/Level_Bern.cpp`
- free camera 입력은 Priority에서 유지한다.
- follow target 계산은 object `Update` 뒤의 `Late_Update`에서 수행하고 camera pipeline을 즉시 commit한다.
- target을 처음 연결하거나 follow를 다시 활성화할 때는 첫 렌더 전에 승인 transform으로 초기화한다.
- offset 편집은 follow 초기화를 매번 파기하지 않고 다음 follow update가 이동을 반영하게 한다.
- Bern 초기 camera는 cached Server spawn이 있으면 그 position을 사용하며 follow response `0`으로 고정 offset을 유지한다. 캐시가 아직 없다면 기존 bounds framing은 안전 fallback으로 남기고 첫 local spawn bind에서 즉시 교체한다.

새 C++ 파일은 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다. Engine public header가 바뀌므로 Engine build 뒤 `UpdateLib.bat`과 Client build까지 검증한다.

## G3. 실패와 보존 경계

- malformed spawn packet은 기존 decode 실패 경로를 유지하며 camera cache를 commit하지 않는다.
- 다른 player spawn은 local spawn cache를 바꾸지 않는다.
- disconnect나 world transfer 승인 뒤에는 이전 world pose를 재사용하지 않는다.
- follow target이 만료되면 camera는 기존 free-camera 전환 계약을 유지한다.
- cached spawn이 없는 정상 지연 상황은 Bern 진입 실패로 승격하지 않는다.

## G4. 검증

1. source audit: follow update가 `Late_Update`에 있고 Priority follow update가 제거됐는지 확인한다.
2. source audit: local spawn cache가 accepted/despawn/close에서 reset되고 Bern 초기 eye/at에 소비되는지 확인한다.
3. Engine x64 Debug/Release build.
4. `UpdateLib.bat Debug/Release`.
5. Client x64 Debug/Release build와 Client frontend regression.
6. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`.
7. `git diff --check` 및 변경 파일 인코딩 확인.
8. 수동 smoke: Bern 첫 프레임에서 맵 전체 조망이 노출되지 않는지, 우클릭 이동 중 player의 screen-space 위치가 끊기지 않는지, F6 왕복이 유지되는지 확인한다.
