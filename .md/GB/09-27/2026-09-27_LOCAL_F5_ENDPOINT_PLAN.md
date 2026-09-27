# 로컬 F5 endpoint 선택 유지

## G01. 개인 선택을 LAN 동기화에서 보존

9월23일 로컬 debugger 설정이 이후 무인 LAN sync에서192.168.0.22로 덮였다.
현재 팀 endpoint는 도달하지 않고, 로컬 Server listener도 없다. NPC 데이터 오류가 아니라
입장 전 TCP 연결 단계의 실패다. 공유 TeamLanEndpoint.json과 C++ fallback은 바꾸지 않는다.

기존 Sync-TeamLanEndpoint.ps1에 EndpointMode(Saved/Team/Local)를 추가한다.
Saved 기본값은 Client.vcxproj.user의 LostArkEndpointMode를 읽고, 선택이 없으면 종전 Team이다.
Local 명시 선택은127.0.0.1 client/bind를 사용한다. Team 명시 선택은 공유 JSON을 다시 사용한다.
개인 선택은 Git 제외 user XML에 저장한다. 유효하지 않거나 서로 다른 선택이 중복되면 거부한다.
기존 XML 환경변수 보존·원자적 개별 저장과 endpoint validation을 그대로 사용한다.

Server + Client 기존 solution launch profile을 사용하며 UI·suo를 자동 조작하지 않는다.
사용자가 IDE reload 후 Debug/x64, Server + Client를 고르고 F5로 시작한다.
C++ 및 asset 변경이 없으므로 새 컴파일·게시 없이 기존 Debug Server의 bounded headless
시작/127.0.0.1 TCP 도달을 확인하고 종료한다. 실제 Client 입장은 사용자가 확인한다.

## G01 반영 코드

기존 script의 정확한 교체 블록이다. C++/project/filter/런타임 데이터 변경은 없다.
대소문자 옵션을 정규화하여 `local` 선택 뒤 기본 sync가 실패하지 않게 한다.

```diff
diff --git a/Tools/Network/Sync-TeamLanEndpoint.ps1 b/Tools/Network/Sync-TeamLanEndpoint.ps1
index 252959a0..a83b2d82 100644
--- a/Tools/Network/Sync-TeamLanEndpoint.ps1
+++ b/Tools/Network/Sync-TeamLanEndpoint.ps1
@@ -3,7 +3,9 @@ param(
     [switch]$SkipConnectionCheck,
     [switch]$AllowExpired,
     [ValidateSet('Auto', 'Server', 'Client')]
-    [string]$Role = 'Auto'
+    [string]$Role = 'Auto',
+    [ValidateSet('Saved', 'Team', 'Local')]
+    [string]$EndpointMode = 'Saved'
 )

 Set-StrictMode -Version Latest
@@ -351,6 +353,35 @@ if (-not (Test-Path -LiteralPath $endpointPath)) {

 $endpoint = Get-Content -LiteralPath $endpointPath -Raw -Encoding UTF8 |
     ConvertFrom-Json
+# A deliberate per-PC local F5 choice must survive the next session's sync.
+# Do not infer this from a stale host value or automatically fall back on failure.
+$effectiveEndpointMode = $EndpointMode
+if ('Saved' -eq $effectiveEndpointMode) {
+    $savedModes = @()
+    if (Test-Path -LiteralPath $clientUserPath) {
+        $savedDocument = [System.Xml.Linq.XDocument]::Load($clientUserPath)
+        $savedNamespace = [System.Xml.Linq.XNamespace]::Get(
+            'http://schemas.microsoft.com/developer/msbuild/2003')
+        if ($null -eq $savedDocument.Root -or
+            $savedDocument.Root.Name -ne ($savedNamespace + 'Project')) {
+            throw "Invalid Visual Studio user project XML: $clientUserPath"
+        }
+        $savedModes = @($savedDocument.Descendants($savedNamespace + 'LostArkEndpointMode') |
+            ForEach-Object { $_.Value } | Select-Object -Unique)
+    }
+    if ($savedModes.Count -gt 1 -or
+        ($savedModes.Count -eq 1 -and $savedModes[0] -notin @('Team', 'Local'))) {
+        throw 'LostArkEndpointMode must have one consistent Team or Local value.'
+    }
+    $effectiveEndpointMode = if ($savedModes.Count -eq 1) { $savedModes[0] } else { 'Team' }
+}
+# ValidateSet accepts case variants; persist a canonical value for later syncs.
+$effectiveEndpointMode = if ('Local' -eq $effectiveEndpointMode) { 'Local' } else { 'Team' }
+if ('Local' -eq $effectiveEndpointMode) {
+    # This only changes the in-memory endpoint, never the shared team document.
+    $endpoint.serverHost = '127.0.0.1'
+    $endpoint.serverBindAddress = '127.0.0.1'
+}
 # Loopback remains a supported isolated-test endpoint, while a shared LAN
 # contract uses one concrete Client address and an all-adapter Server bind.
 if ($endpoint.schema -ne 'lostark.team-lan-endpoint' -or
@@ -436,6 +467,10 @@ Set-ProjectUserEnvironmentVariable `
     -Path $clientUserPath `
     -VariableName 'LOSTARK_SERVER_HOST' `
     -Value $serverHost
+Set-ProjectUserProperty `
+    -Path $clientUserPath `
+    -PropertyName 'LostArkEndpointMode' `
+    -Value $effectiveEndpointMode

 # Loopback never leaves the machine, so it needs no inbound rule - and asking
 # for one would demand an elevated shell for nothing.
@@ -465,6 +500,7 @@ if (-not $SkipConnectionCheck) {
 }

 Write-Output "Team LAN active through: $($activeThrough.ToString('o'))"
+Write-Output "Debugger endpoint mode: $effectiveEndpointMode (saved per PC)"
 Write-Output "Machine role: $effectiveRole"
 Write-Output "Server debugger bind: $serverBindAddress`:$serverPort"
 Write-Output "Client debugger endpoint: $serverHost`:$serverPort"
```
