# Bern 카메라 팔로우 안정화 결과

## 완료 상태

- Bern 진입 직후 카메라 초기 위치를 Server 승인 로컬 스폰 위치에 맞추도록 수정했다.
- follow 카메라 갱신을 오브젝트 이동 이후인 `Late_Update`로 옮겨 한 프레임 늦게 추적하던 경로를 제거했다.
- follow 활성화와 위치 프레이밍 시 카메라 파이프라인과 frustum을 즉시 같은 상태로 갱신하도록 수정했다.
- Bern에서는 이미 보간된 플레이어 Transform을 고정 offset으로 따라가며 별도 카메라 지연 보간을 중복 적용하지 않는다.
- 카메라 동일 프레임 갱신 계약을 ProjectAudit에 추가했다.

## 변경 계약

### Server 승인 스폰 캐시

`CNetworkManager`는 현재 로컬 플레이어와 entity ID가 일치하고 pose 값이 유한한
`S2C_PLAYER_SPAWNED`를 캐시한다. 새 world 승인, 연결 종료, 로컬 despawn에서는 캐시를
초기화한다. Bern 카메라는 이 값을 사용할 수 있으면 초기 eye/at을 스폰 기준으로 만들고,
아직 도착하지 않았으면 기존 map bounds framing을 fallback으로 사용한다.

### 같은 프레임 follow

`CCamera_Free`는 `Priority_Update`에서 follow를 갱신하지 않는다. 모든 GameObject의
`Update`가 끝난 뒤 `Late_Update`에서 대상 Transform을 읽고 카메라 파이프라인을 commit한다.
follow 활성화 직후에도 한 번 즉시 commit해 이전 카메라 위치가 잠깐 노출되지 않게 했다.

`CCamera::Update_PipeLine`은 view/projection 설정 뒤 inverse view, 카메라 위치와 frustum까지
동일한 commit에서 갱신한다.

## 자동 검증

- Engine x64 Debug build: PASS
- `UpdateLib.bat Debug`: PASS
- Client x64 Debug build/link: PASS
- NetworkProtocolHarness x64 Debug: PASS (`failures : 0`)
- ClientFrontendHarness x64 Debug: PASS (`exit 0`)
- Server x64 Debug `--contract-test`: PASS (`exit 0`)
- ProjectAudit `camera.follow-same-frame-transform`: PASS
- 변경 파일 대상 `git diff --check`: PASS

전체 ProjectAudit에는 이번 변경과 무관한 기존/병렬 작업 상태인
`projects.data-source-visibility: expected=544 project=543 filters=543` 한 건이 남아 있다.

Release 검증은 병렬 세션과 겹친다는 사용자 요청에 따라 실행 중 중단했으며 PASS 또는 FAIL로
기록하지 않는다.

## 수동 검증

Bern 실제 플레이 진입 및 이동 smoke는 실행하지 않았다. 따라서 스폰 직후와 연속 이동 중
육안 카메라 안정성은 미검증 상태다.

## Git 경계

작업 브랜치는 `codex/bern-camera-follow-stability`다. worktree에 다른 세션의 Effect 관련
변경이 함께 존재하므로 이 작업에서는 stage, commit, push하지 않았다.
