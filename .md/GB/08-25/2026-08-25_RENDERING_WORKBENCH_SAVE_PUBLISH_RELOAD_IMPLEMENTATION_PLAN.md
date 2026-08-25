# Rendering Workbench Save/Publish/Reload 복구 구현 계획

## 1. 요청과 정본

- 요청: Rendering Workbench에서 저장한 Bloom Scatter가 실제 EXE runtime에 반영되지 않는 문제를 고친다.
- authoring 정본: `Data/Rendering/Authored/RenderingProfiles.json`
- runtime 생성물: `Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`
- 범위 밖: 오늘 수정 중인 Character/Effect authored·runtime data와 Effect renderer 코드는 변경하지 않는다.

## 2. 실측된 실패

- authored는 revision 10, `bloomScatter = 1`로 정상 저장됐다.
- runtime은 revision 9, `bloomScatter = 2.75999999`로 남아 있다.
- publisher Validate는 authored의 `fxaaEdgeThreshold = 0.0311999992`를 decimal double 최소값
  `0.0312`보다 작다고 판정해 실패한다.
- C++ serializer/parser/renderer는 같은 값을 float32 최소값 `0.0312f`로 정상 수용한다.
- 따라서 Scatter binding 문제가 아니라, unrelated FXAA float32 직렬화 값이 전체 runtime publish를
  차단한 것이 직접 원인이다.

## 3. 구현 단위

### G01. Publisher와 C++ float32 계약 통일

- revision/version의 정수·double 검증은 유지한다.
- 실제 renderer가 소유하는 모든 f32 field는 C++ float literal과 같은 float32 경계로 검증한다.
- effective exposure/bloom 곱도 float32 입력과 float32 결과로 검증한다.
- nominal decimal 아래/위로 보이는 정상 float32 round-trip 값을 허용하되 실제 float32 경계 밖 값은
  계속 거부한다.

### G02. stale authored 덮어쓰기 차단

- `Save_Authored` 직전에 기존 authored를 parse한다.
- disk authored revision과 현재 service catalog revision이 다르면 저장을 거부하고 기존 authored를
  보존한다.
- 고정 `.tmp` 대신 process/clock 기반 unique staged path를 사용하고 모든 실패 경로에서 정리한다.

### G03. 명확한 적용 동선과 실패 상태

- 기존 개별 Save/Publish/Reload 버튼은 recovery와 진단을 위해 유지한다.
- `Save + Publish + Reload` 버튼을 추가하고 단계 성공 때만 다음 단계로 진행한다.
- Save 성공/Publish 실패와 Publish 성공/Reload 실패를 서로 다른 partial state로 보고한다.
- publisher stdout/stderr와 Win32 wait/launch 오류를 Workbench status에 보존한다.

### G04. focused domain regression

- 퇴역한 전역 `ProjectAudit`은 복구하지 않는다.
- `Tools/RenderingPipeline/Test-RenderingProfiles.ps1`에서 canonical authored Validate, temp publish,
  float32 min/max acceptance, 실제 경계 밖 rejection, 실패 publish의 기존 runtime 보존을 검증한다.

## 4. 검증

1. focused rendering pipeline test
2. canonical authored `-Mode Validate`
3. Client 종료 확인 뒤 canonical `-Mode Publish`
4. authored/runtime revision과 Scatter semantic parity 확인
5. Client Release build, 가능하면 Client 종료 뒤 Debug build
6. JSON parse와 `git diff --check`
7. 실제 화면 반영은 사용자가 새 Debug Client에서 Workbench 버튼과 Scatter 변화로 최종 판정

## 5. 완료 경계

- 자동 검증은 저장·publish·reload 코드와 파일 parity까지 증명한다.
- 현재 실행 중인 구버전 Client를 에이전트가 종료하거나 UI를 조작하지 않는다.
- 최종 first-pixel/visual fidelity는 사용자 관찰 전까지 PASS로 기록하지 않는다.
