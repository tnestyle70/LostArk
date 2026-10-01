# Deploy 바닥 CPU 이동 피킹 수정 결과

## G00. 원인과 실제 수정

`9403f6d49`의 GPU→CPU 이동 피킹 전환에서 Level resolver가 일반 Map runtime만 조회했다.
발탄의 외곽 돌판 A/B 네 배치와 난간 두 배치는 별도 Deploy 소유여서 누락됐다.
실제 설치 A/B geometry의 CPU 교차는 정상이고 Client/Server navigation은 같은 hash이며
최근 피킹 변경 때 navigation은 바뀌지 않았다. 통신 장애를 재현한 결과는 아니다.

`CDeployPropObject`와 `CDeployPropRuntime`에 기존 CModel 피킹을 연결하고 Valtan/Kouku
resolver가 Map/Deploy 중 가까운 hit를 선택하도록 수정했다. 정적 객체는 현재 intact/fractured
모델과 실제 world transform, animated 종이 다리는 현재 골격 pose를 검사한다.
Render의 despawn/opacity/source suppression/camera suppression과 일치하며 flying debris는
검사하지 않는다. miss와 invalid 입력은 기존 출력·이동을 보존한다. GPU 대기는 추가하지 않았다.

게시된 Deploy 영역은 발탄12asset/145placement, 쿠크5asset/7placement 두 곳이다.
발탄 바닥·난간 ID는7000000000000000001/2/3/5/6/7, 쿠크 종이 다리는1/4다.
두 Level의 모든 현재 Deploy source object를 검사하므로 특정 ID만 예외로 처리하지 않는다.
다른 Level의 authoring용 Deploy는 현재 제품 Load가 없으며 추가 활성 통행 바닥 누락은
조사 범위에서 발견하지 않았다. 쿠크 빙고 바닥은 기존 Map placement 조회 대상이다.

## G01. 실행한 검증

- 설치 A/B WModel을 읽고 생산 Mesh/Model CPU 함수를 실행한 ray4개 모두 hit.
  실제 높이는23.019974/22.999283/23.005508/22.978371m이며 navigation 높이와 일치한다.
  `out/DeployPickingAudit20261001/history_geometry/{manifest.json,results.txt}`.
- 기존 CPU 이동 dispatch Release27검사 PASS.
  `out/DeployPickingAudit20261001/dispatch-release/result.json`.
- 신규 생산 Deploy/두 Level resolver 추출 Release64검사 PASS.
  geometry는 deterministic CModel 대역이며 실제 설치 geometry 검증과 구분한다.
- Release Product Build PASS, SkipBuild=false. Engine/Shared/Server/Client 정상 증분 빌드와
  runtime 입력 검사를 통과했다. 기존 인코딩/PDB 경고는 남았고 compile/link 오류는 없다.
  receipt: `out/BuildPipeline/runs/20261001T015758833Z-release-product.json`.
- 기존 C++ UTF-8/BOM/CRLF를 유지했다. 신규 제품 C++ 파일이 없어 project/filter 등록은 없다.
  데이터·shader·protocol·Server navigation·렌더링 옵션 변경은 없다.

## G02. 배포와 사용자 확인 경계

사용자 승인으로 이 저장소 Release Server PID31712를 종료한 뒤 빌드했다.
Client/UI를 실행하거나 화면을 캡처하지 않았다. Server도 자동 재시작하지 않았다.
새 portable ZIP 생성·검증 결과는 완료 후 아래에 기록한다. 기존 FINAL ZIP은 보존한다.

사용자 확인은 발탄 양쪽 외곽 돌판·난간 클릭, 파괴 뒤 사라진 표면 제외,
쿠크 첫째/둘째 종이 다리를 펼친 뒤 이동이다. 실제 Server 이동은 여전히 navigation과
collision이 결정한다. 현재 pose API의 양면 CPU 검사와 GPU alpha/culling 픽셀 일치는 별개이며,
자동 수치 검증은 실제 화면·조작 성공 판정을 대신하지 않는다.
