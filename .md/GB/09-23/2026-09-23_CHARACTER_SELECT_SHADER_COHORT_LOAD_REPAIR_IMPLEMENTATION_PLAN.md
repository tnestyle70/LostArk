# Character Select 맵 셰이더 묶음 불일치 복구 계획

## G00. 사용자 요청과 확인한 실패

사용자는 방금 빌드한 Debug의 Character Select 진입에서 `visual map / Effect load job is already cancelled`가 표시되며 이 문제를 가장 먼저 수정하도록 요청했다. 진행 중인 발탄 수정의 추가 구현은 중단하고 이 로딩 실패를 우선 처리한다. Client/UI는 자동 실행하지 않는다.

`Ready_For_CharacterSelect`의 visual map 단계 직후 `Ready_MapAuthoringCore`가 static/preview shader를 만든다. 여기서 실패하면 Loader가 Effect 작업을 취소하고, main의 후속 시작 요청이 already cancelled를 받는다. 화면 문구만으로 Effect codec이나 timeout을 원인으로 단정하지 않는다.

현재 Debug `Shader_VtxMeshBinary.cso`와14개 source group 중13개에는 새 source PBR 간접광 변수들이 있지만 `Shader_VtxMeshBinary_SourceGroup084.cso`에는 없다. 실제 `CShader::Stage_ProgramVariants`는 base의 공개 변수와 각 group의 타입 일치를 요구하므로 서로 다른 시점의 컴파일 결과가 섞이면 생성에 실패한다. 파일 존재·mtime와 일반 shader closure 성공은 이 ABI의 일치를 보증하지 않는다.

## G01. 실패 재현과 후보 준비

기존 Engine의 `CShader::Create`를 사용하는 작은 WARP probe를 out에 격리한다. 현재 Debug Engine DLL과 static shader14개 및 Preview의 복사본만 사용한다. 화면을 만들지 않고 생성 성공·시간·실제 Shader 진단을 수집한다.

문제가 확인된 static SourceGroup084만 현재 HLSL과 프로젝트의 fx_5_0 /O1 설정으로 out에 재컴파일한다. source/include hash를 전후 비교하며 설치 CSO는 후보 검사 전 교체하지 않는다. 새 C++ 제품 파일이나 프로젝트 항목은 추가하지 않는다.

## G02. 검증 후 반영

기존 CSO의 실패와 후보 CSO로 교체한 격리 입력의 성공을 동일한 product Shader 경로에서 대조한다. 실제 설치 CSO의 최신 hash를 다시 확인하고 백업 후 후보를 교체한다. 다른 source group과 진행 중인 Release 출력은 덮어쓰지 않는다. Debug의 설치 후 파일 hash와 product Shader 생성까지 확인한다.

Release Product 빌드는 이미 진행 중이며 별도 완료 상태를 기록한다. 이번 Debug 실패 복구와 Release 컴파일 성공, 실제 Client 화면 확인은 서로 구분한다. 이번 즉시 복구를 위해 Effect 취소 검사나 셰이더 ABI 검증을 제거하지 않는다.

## G03. 재발 방지 기록과 완료 조건

서로 다른 source snapshot으로 생성된 shader family가 mtime상 최신처럼 보일 수 있다는 원리와 실제 변수 ABI 검증 필요성을 gotchas와 렌더링 복원 노트에 남긴다. 동일 Debug 출력에 대한 중복 빌드가 관측됐다는 사실과 정확히 어느 프로세스가 오래된 CSO를 썼는지 확인되지 않았다는 한계를 구분한다.

RESULT에는 설치 전 실패·후보 성공·설치 hash와 실행한 검사만 기록한다. 첫 Shader 오류의 경로/HRESULT가 후속 취소 문구에 가려지는 진단 품질 문제는 별도 후속 개선으로 표시한다. 사용자 화면 성공을 대신 주장하지 않는다.
