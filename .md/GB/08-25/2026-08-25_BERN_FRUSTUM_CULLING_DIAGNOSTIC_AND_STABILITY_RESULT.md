# 베른 프러스텀 컬링 진단·안정화 결과

## 구현 상태

- 베른 전용 프러스텀 우회/진단과 원인 격리: 완료
- 제품 기본 프러스텀 컬링 재활성화: 완료
- stable placement ID와 경계구·여섯 평면 거리 기록: 완료
- binary static model 전체 decoded vertex 기준 경계 재계산: 완료
- 컬링과 렌더의 동일 View/Projection 스냅샷 사용: 완료
- Landscape 및 대형 floor/bridge geometry 보수적 margin: 완료
- 3-frame rejection hysteresis: 완료
- 우회 상태 사용자 화면 판정: PASS(깜빡임 소멸)
- 정상 컬링 재활성화 후 사용자 화면 판정: 대기

## 확인된 원인

사용자가 플레이어를 멈추고 F6 자유 카메라만 움직인 상태에서도 깜빡임을 재현했고, 베른 프러스텀 우회를 켜자 깜빡임이 사라졌다. 따라서 내비게이션 높이 또는 플레이어 위치 변경이 아니라 맵 배치의 프러스텀 false rejection이 직접 원인이다.

우회 실행에서 `VISIBLE_TO_REJECTED` 전환 157,729건이 기록됐고, 이 중 반경 4m 이상 또는 Landscape로 분류한 대형 geometry가 20,176건이었다. 잔디뿐 아니라 `BREEZE_BRIDGE`, `FLOOR`, 성벽과 기둥 asset도 측면 평면 경계에서 반복 탈락했다.

기존 경로에는 세 문제가 겹쳐 있었다.

1. fallback과 static batch가 `Late_Update`의 Engine frustum으로 먼저 탈락했지만 실제 shader View/Projection은 뒤의 Render에서 다시 읽었다. 특히 follow camera가 `Late_Update`에서 갱신되는 프레임에는 컬링과 실제 렌더 카메라의 시점이 달라질 수 있었다.
2. binary static model은 embedded bounds가 있으면 전체 decoded vertex를 순회하지 않아, 잘못되거나 축소된 metadata가 대형 바닥·다리 경계구를 작게 만들 수 있었다.
3. 대형 지형에도 일반 배치와 같은 날카로운 구-평면 판정을 사용해 화면 가장자리의 작은 행렬·경계 오차가 즉시 한 프레임 소멸로 이어졌다.

현재는 final Render에서 한 View/Projection 스냅샷으로 평면 판정과 shader 바인딩을 함께 수행하고, 전체 decoded vertex 경계, 대형 geometry margin, 3-frame rejection hysteresis를 적용한다.

## 런타임 동작

진단 실행에서는 베른에 들어갈 때 로그를 새로 시작하고, 프러스텀 계산상 탈락한 배치도 실제로는 계속 렌더한다. 계산 상태가 visible에서 rejected로 바뀌면 다음 파일에 한 줄을 추가한다.

`Client/Bin/Debug/Diagnostics/BernFrustumCulling.log`

각 행은 `cameraRevision`, `assetId`, `groupId`, `placementId`, `center`, `baseRadius`, `margin`, `effectiveRadius`, `largeGeometry`, `planeDistances[6]`를 포함한다.

원인 확인 뒤 제품 기본값은 `bypass=false`, `diagnostics=false`로 전환했다. 따라서 평상시에는 프러스텀 컬링이 다시 동작하고 대량 진단 로그를 쓰지 않는다. 진단 구현과 기존 확인 로그는 후속 재현에 사용할 수 있도록 유지한다.

## 자동 검증

- Engine x64 Debug: PASS
- Client 변경 파일 compile: PASS
- Client x64 Debug link: PASS
- 2026-08-26 정상 컬링 재활성화 Client x64 Debug rebuild: PASS
- 정적 계약 하네스: PASS
- 변경 파일 대상 `git diff --check`: PASS

`UpdateLib.bat Debug`는 Engine DLL/LIB와 헤더를 복사했으나 첫 실행에서 기존 PDB 쓰기 공유 오류를 반환했다. 이어진 Client 빌드 post-build가 Engine DLL/PDB를 정상 배포했고 Client 링크는 PASS했다.

## 수동 검증 경계

에이전트는 Client를 실행하거나 화면을 대신 판정하지 않는다. 우회 상태에서 깜빡임이 사라졌다는 사용자 판정은 기록했다. 이제 정상 컬링을 다시 켠 빌드에서 같은 재현 위치에 플레이어를 멈추고 F6 자유 카메라만 움직였을 때 깜빡임이 재발하지 않는지 사용자가 확인해야 한다.

## 2026-08-28 PR 병합 전 교정

PR #247의 최신 `main` 합성 병합 트리에서 Bern 전용 안정화 값이 모두 0으로 바뀐 것을
회귀로 확인했다. 우회 실행으로 false rejection을 확인한 위 증거와 맞지 않으므로 제품값을
`baseMargin=0.25`, `largeObjectRadiusThreshold=4`,
`largeObjectAbsoluteMargin=2`, `largeObjectRelativeMargin=0.12`,
`rejectHysteresisFrames=3`으로 복원했다. 정적 계약도 필드 존재만 보던 방식에서
`MakeBernMapScope`의 이 다섯 수치를 정확히 검사하도록 강화했다.

자동 계약은 다시 통과했지만 정상 컬링 상태의 최종 화면 판정은 여전히 사용자 확인 항목이다.
