# Lobby Bern Canonical Endpoint PLAN

작성일: 2026-08-14

상태: 구현 완료. 자동 검증 결과는 대응 RESULT를 따른다.

## 1. 문제와 실측 근거

사용자 화면에서 Lobby는 다음 두 주소를 동시에 표시했다.

```text
Server: 192.168.200.103:7777
Test (Map Editor): 127.0.0.1:7777
Server connection failed for 192.168.200.103:7777 (WSA 10060)
```

`CLevel_Lobby::Begin_NetworkEntry`는 Test Map Editor에만
`CNetworkManager::Resolve_MapEditorServerHost()`를 사용하고 Bern, Valtan,
Character Select에는 `Resolve_ServerHost()`를 사용한다. 따라서 Test 성공은 Bern 서버
입장 경로의 성공 증거가 아니었다. 서로 다른 서버 주소로 접속하고 있었기 때문이다.

팀 고정 계약은 Lobby의 네 명령이 모두 하나의 `LOSTARK_SERVER_HOST` endpoint를 통해
Server 승인을 받은 뒤 진입하는 것이다. Test만 별도 loopback endpoint를 선택하는 경로를
유지하지 않는다.

## 2. 변경 계약

- `Resolve_ServerHost()`가 Test, Character Select, Valtan, Bern의 단일 endpoint 정본이다.
- 기존 호출자 호환을 위해 `Resolve_MapEditorServerHost()` 선언은 유지하지만 결과는 반드시
  `Resolve_ServerHost()`와 같아야 한다.
- 과거 프로세스 환경에 `LOSTARK_MAPEDITOR_SERVER_HOST=127.0.0.1`이 남아 있어도 무시한다.
- Test의 `MAP_EDITOR_WORKSPACE` 목적, 승인 후 Development editor shell 진입 동작은 그대로
  유지한다. 바뀌는 것은 TCP 서버 주소 선택뿐이다.
- 실제 Server가 꺼져 있으면 Test와 Bern 모두 실패하는 것이 정상이다. Client가 로컬
  fallback으로 성공한 것처럼 처리하지 않는다.

## 3. 구현 코드

`Client/Private/NetworkManager.cpp`의 기존
`CNetworkManager::Resolve_MapEditorServerHost` 함수 전체를 다음 코드로 교체한다.

```cpp
std::string CNetworkManager::Resolve_MapEditorServerHost()
{
	// Test(Map Editor) and product worlds must enter through the same
	// authoritative Server endpoint. Keep this compatibility entry point so
	// existing lobby code cannot reintroduce a private loopback route.
	return Resolve_ServerHost();
}
```

`Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`에는 옛 Test 전용 환경변수를
의도적으로 loopback으로 설정해도 두 resolver가 같은 값을 반환하는 테스트를 추가한다.

```cpp
void Test_CanonicalLobbyServerEndpoint(TEST_RUNNER& runner)
{
	SCOPED_ENVIRONMENT_VARIABLE serverHostEnvironment(
		L"LOSTARK_SERVER_HOST");
	SCOPED_ENVIRONMENT_VARIABLE legacyMapEditorHostEnvironment(
		L"LOSTARK_MAPEDITOR_SERVER_HOST");
	const bool_t environmentConfigured =
		serverHostEnvironment.Set(L"192.168.200.103") &&
		legacyMapEditorHostEnvironment.Set(L"127.0.0.1");

	runner.Require(
		environmentConfigured &&
		CNetworkManager::Resolve_ServerHost() == "192.168.200.103" &&
		CNetworkManager::Resolve_MapEditorServerHost() ==
			CNetworkManager::Resolve_ServerHost(),
		"Lobby Test And Product Worlds Share Canonical Server Endpoint");
}
```

빠른 회귀 모드는 다음과 같다.

```cpp
if (Mode == "--lobby-endpoint-fast")
{
	Test_CanonicalLobbyServerEndpoint(runner);
	std::cout << "failures : " << runner.iFailureCount << '\n';
	return 0 == runner.iFailureCount ? 0 : 1;
}
```

전체 ClientFrontendHarness에도 같은 테스트를 등록한다.

```cpp
Test_NormalHandoff(runner);
Test_CanonicalLobbyServerEndpoint(runner);
Test_CharacterSelectAuthorizedSelection(runner);
```

## 4. 검증 순서

```powershell
msbuild Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe --lobby-endpoint-fast
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

수동 화면 검증은 사용자가 팀 Server 실행을 확인한 뒤 수행한다.

1. Client를 재시작한다.
2. Lobby에 `Server: <하나의 주소>:7777`만 표시되는지 확인한다.
3. Test가 Server 승인을 받고 Development Map Editor로 진입하는지 확인한다.
4. Lobby로 돌아온 뒤 Bern이 같은 Server 승인을 받고 Bern Level로 진입하는지 확인한다.
5. Server가 꺼져 있으면 두 명령 모두 명시적인 접속 실패를 보여야 한다.
