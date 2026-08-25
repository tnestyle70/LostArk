# 베른 프러스텀 컬링 진단·안정화 결과

## 구현 상태

- 베른 전용 프러스텀 우회/진단 정책: 완료
- stable placement ID와 경계구·여섯 평면 거리 기록: 완료
- binary static model 전체 decoded vertex 기준 경계 재계산: 완료
- 컬링과 렌더의 동일 View/Projection 스냅샷 사용: 완료
- Landscape 및 대형 floor/bridge geometry 보수적 margin: 완료
- 3-frame rejection hysteresis: 완료
- 사용자 최종 화면 판정: 대기

## 런타임 동작

베른에 들어가면 진단 로그를 새로 시작한다. 현재 bypass가 켜져 있으므로 프러스텀 계산상 탈락한 배치도 실제로는 계속 렌더한다. 계산 상태가 visible에서 rejected로 바뀌면 다음 파일에 한 줄을 추가한다.

`Client/Bin/Debug/Diagnostics/BernFrustumCulling.log`

각 행은 `cameraRevision`, `assetId`, `groupId`, `placementId`, `center`, `baseRadius`, `margin`, `effectiveRadius`, `largeGeometry`, `planeDistances[6]`를 포함한다.

## 자동 검증

- Engine x64 Debug: PASS
- Client 변경 파일 compile: PASS
- Client x64 Debug link: PASS
- 정적 계약 하네스: PASS
- 변경 파일 대상 `git diff --check`: PASS

`UpdateLib.bat Debug`는 Engine DLL/LIB와 헤더를 복사했으나 첫 실행에서 기존 PDB 쓰기 공유 오류를 반환했다. 이어진 Client 빌드 post-build가 Engine DLL/PDB를 정상 배포했고 Client 링크는 PASS했다.

## 수동 검증 경계

에이전트는 Client를 실행하거나 화면을 대신 판정하지 않는다. 사용자가 기존 재현 위치에서 플레이어를 멈추고 F6 자유 카메라만 움직였을 때 지형 깜빡임이 사라졌는지 확인해야 한다. 사라지지 않으면 생성된 로그와 새 영상/스크린샷을 근거로 프러스텀 이외의 렌더 상태를 이어서 조사한다.
