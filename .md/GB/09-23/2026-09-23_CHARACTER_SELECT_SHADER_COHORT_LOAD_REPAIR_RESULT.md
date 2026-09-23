# Character Select 맵 셰이더 불일치 복구 결과

## G00. 확정한 최초 실패

사용자는 방금 빌드한 Debug에서 Character Select 진입 실패를 보고했다. 화면의 `Effect load job is already cancelled`는 최초 Shader 실패 뒤 Loader가 형제 Effect 작업을 취소한 후속 상태다. 이번 발탄 넉백 변경은 Server 및 발탄 navpolicy에 한정돼 있으며 이 Client Shader 경로를 변경하지 않았다.

Debug의 `Shader_VtxMeshBinary_SourceGroup084.cso`에 새 source PBR 간접광 변수 다섯 개가 없었다. base와 다른13개 source group에는 있다. `CShader::Stage_ProgramVariants`는 base의 공개 변수와 각 group의 FX 타입 일치를 요구하며084에서 실패한다. 파일은 존재하고 읽을 수 있지만 같은 shader family 안의 ABI가 다른 상태다.

실패 파일의 수정 시각은12:23:06, base는12:29:57이다. 공용 HLSLI의12:21:09보다 CSO가 늦다는 사실만으로 최신 소스 반영을 증명할 수 없다.12:49:53 Debug Product 결과의 변경 목록에는 base와 다른13개 group이 있고084는 없다. 이전 입력으로 진행하던 compiler의 늦은 출력이나 외부 빌드 재사용으로 이런 상태가 생길 수 있으나, 이 파일을 만든 정확한 프로세스까지 확정한 것은 아니다.

## G01. 실제 설치 입력으로 재현

Client/UI를 실행하지 않고 현재 Debug Engine DLL의 실제 `CShader::Create`를 호출하는 작은 WARP probe를 out에 구성했다. probe 자신에게만 debugger를 붙여 기존 OutputDebugString 진단을 수집했다. 입력 CSO와 DLL의 source path·SHA는 `out/CharacterSelectLoadFailure20260923/snapshot-manifest.json`에 있다.

- 설치 static base 생성: 실패, `stage=program-variants`, HRESULT `0x80004005`,16,125.156ms.
- SourceGroup001/009/017/025/080/084 각각의 FX 생성은 성공했지만084 직후 base와의 ABI join이 실패했다.
- Preview 생성: 성공,24.528ms.
- probe exit1. 증거는 `probe.log`, `probe-summary.log`다.
- 기존 `Test-CompiledShaderClosure.ps1 -Configuration Debug -Modules Product`는 PASS였다. 파일/생산자/Effect 소비자 검사 성공은 static base의 전체 variant ABI 검사 성공을 대신하지 않는다.

## G02. 지연 구간과 실패 원인의 구분

최근 PID57668의12:58 실행은 Effect 렌더러 준비 세 항목에서 각각7,985/8,016/8,219ms를 기록했다. 겹쳐 실행된 항목이므로 합계24초의 직렬 비용으로 계산하지 않는다. 이 로그의 모든 항목을 화면의 동일 Loading 인스턴스 실패와 직접 연결하는 근거는 없다.

첫 renderer 준비의 `Acquire_RendererCore`는 공용 build mutex 아래126개 Effect shader program과 공통 animated/screen/rect/trail/decal 자원을 준비한다. static/animated base 생성은각14개 source group을 미리 생성한다. 첫 실행의 FX 생성과 공유 코어 준비는 실제 지연 후보이며, compiler 부하가 겹친 이번 측정으로 평상시 GPU 성능이나 전체 로딩 개선율을 계산하지 않는다. runtime은 CSO를 읽으며 HLSL 컴파일은 하지 않는다.

이번 실패의 확정 원인은 느린 준비나 timeout이 아니라 static084의 변수 ABI 불일치다. 이를 해결하려고 취소 조건·ABI 검사를 없애거나 timeout을 늘리지 않는다.

## G03. 복구와 완료 상태

SourceGroup084를 정확한 Debug fx_5_0 /O1 설정으로 out에 재컴파일했다. FXC exit0, 547.156초. 관련 HLSL/include 50개의 전후 hash는 모두 같았고, 후보에 새 PBR 변수 다섯 개가 모두 있다. 기존 X4000 경고는 남아 있지만 컴파일 오류는 없다. `rebuilt/compile.receipt.json`에 입력과 컴파일 명령을 기록했다.

같은 Debug Engine과 base·나머지13개 group을 유지하고084 한 개만 후보로 바꾼 WARP probe는 exit0이었다. static base와 전체14개 variant가 `stage=complete`, HRESULT0으로 생성됐다. base 생성은34,344ms, Preview는6.957ms였다. 기존 실패 로그와 입력은 `baseline/`에 보존했고 후보 증거는 `candidate-probe-summary.log`, `candidate-change.json`이다. 성공 검사에서는 기존 실패 지점 이후의 나머지 group까지 모두 생성하므로16초와34초를 성능 회귀로 비교하지 않는다.

13:18:49 KST에 설치 직전 전체22개 baseline 입력과50개 HLSL 입력의 hash를 재확인하고, 진행 중인 Client·FXC·MSBuild가 없는 상태에서084만 원자적으로 교체했다. 기존 파일은 `backup/`에 보존했다. 설치 후 SHA는 후보와 같은 `1231BE63946D8AA7B3F2D3C01252A5232A9D4EDBC0BB8A1D2A19B49285FE187D`, 크기는1,007,842bytes다. `install.receipt.json`에 이전 SHA·백업 위치와 새 SHA를 기록했다.

설치 후 `Client/Bin/Debug`의 실제 Engine DLL·종속 DLL·static shader 전체·Preview를 별도 `installed-probe/`에 다시 복사해 한 번 검사했다. static base와14개 group 전부 HRESULT0, base 생성33,004.9ms, Preview5.803ms, probe exit0이었다.084는 검증한 후보와 동일하고 나머지21개 입력은 실패 재현 당시 baseline과 동일했다. 설치 검증 증거는 `installed-probe/installed-manifest.json`, `installed-probe-summary.log`, `installed-probe.receipt.json`이다. `git diff --check`도 통과했다. 이번 작업에서 새 모델·텍스처·이펙트 리소스는 추가하지 않았으며 probe·백업·로그는 out에만 두었다.

Release Product는 별도 빌드에서 Engine/Shared/Server/Client 모두 컴파일·링크 PASS였다(`out/BuildPipeline/runs/20260923T041302260Z-release-product.json`). 이번 복구에서 Debug 전체 C++ 빌드를 다시 실행한 것은 아니다. 실제 Client 화면 확인은 사용자가 수행한다. 첫 Shader 실패의 상세 경로/HRESULT를 로딩 화면에 직접 보존하는 진단 개선은 이번 단일 산출물 복구와 별도다.
