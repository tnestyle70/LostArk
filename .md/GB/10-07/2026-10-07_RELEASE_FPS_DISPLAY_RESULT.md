# Release 노란 FPS 표시 결과

## G00. 반영한 코드와 계약

`CMainApp::RenderFpsText`의 전체 Debug 제한을 표시 모드 조건에만 적용했다.
Release는 기존 평활화된 `FPS N`을 좌측 상단에 노란색으로 표시한다.
F1을 열거나 개인 환경설정을 바꿀 필요가 없고, 기존 cinematic HUD 숨김은 유지한다.
Debug의 항상/최근 피격/숨김 조건과 Release F7·Profiler 비활성 경계는 그대로다.

기존 Font_YG760, 색·위치·크기, 매 프레임 FPS 갱신과 렌더 호출을 재사용했다.
MainApp.cpp는 UTF-8 BOM 없음/CRLF를 유지했고 새 H/CPP·프로젝트 등록·JSON 변경은 없다.
AGENTS, CLAUDE와 팀 사용서의 현재 Release FPS 계약만 갱신했다.
렌더링 품질, 프레임 제한, UserSettings 저장 파일과 Resources는 변경하지 않았다.

## G01. 실행한 검증

정본 명령 `powershell -NoProfile -ExecutionPolicy Bypass -File
Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product`가 exit 0으로
완료됐다. Engine→Shared→Server→Client 증분 컴파일·링크·배포가 PASS이며
Client MainApp.cpp의 OBJ 1개와 Client.exe 1개가 갱신됐다. Client 단계 47,195ms,
PCH 0개·CSO 0개 쓰기다. 기존 코드 페이지 C4819와 DirectXTK PDB LNK4099 경고는 남았다.

- 제품 receipt: `out/BuildPipeline/runs/20261007T021111286Z-release-product.json`
- 빌드 로그: `out/ReleaseFps20261007/product-release.log`
- 새 실행 파일: `Client/Bin/Release/Client.exe`
- `git diff --check` 통과. 소스 UTF-8 BOM 없음/CRLF 유지 확인.
- JSON/XML·프로젝트/filter 변경은 없어 신규 parse 검증 대상이 없다.

## G02. 사용자 확인과 남은 범위

Client와 Server를 자율 실행하거나 UI를 조작·캡처하지 않았다.
Release Client를 새로 실행하면 일반 화면 좌측 상단의 노란 FPS를 확인할 수 있다.
직접 실행의 작업 디렉터리는 `Client/Default`이며 Visual Studio에서는 Release|x64를 선택한다.
컷신에서 HUD가 숨겨질 때 FPS도 숨겨진다.

실제 화면 표시와 프레임 드랍 수치·원인은 미측정이다. FPS 표시 복구를 성능 개선이나
언리얼 마이그레이션에 의한 회귀 원인 확인으로 기록하지 않는다. Debug 제품 빌드와 광역
하네스는 실행하지 않았으며 Debug 표시 분기의 기존 동작은 코드 diff로 확인했다.
