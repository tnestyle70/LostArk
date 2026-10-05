[CmdletBinding()]
param([string]$OutputDirectory = '', [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release')

$ErrorActionPreference = 'Stop'
$taskRepo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $taskRepo 'out/UserSettingsContractHarness'
}
$taskOutput = [IO.Path]::GetFullPath($OutputDirectory)
$taskAllowed = [IO.Path]::GetFullPath((Join-Path $taskRepo 'out')) + [IO.Path]::DirectorySeparatorChar
if (-not $taskOutput.StartsWith($taskAllowed, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Harness output must remain inside the repository out directory.'
}
New-Item -ItemType Directory -Force -Path $taskOutput | Out-Null
$taskVsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path -LiteralPath $taskVsWhere -PathType Leaf)) { throw 'vswhere.exe was not found.' }
$taskVs = [string](@(& $taskVsWhere -latest -prerelease -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath) | Select-Object -First 1)
if ([string]::IsNullOrWhiteSpace($taskVs)) { throw 'Visual C++ x64 toolchain was not found.' }
$taskDevCmd = Join-Path $taskVs 'Common7/Tools/VsDevCmd.bat'
$taskExecutable = Join-Path $taskOutput 'UserSettingsContractHarness.exe'
$taskResponse = Join-Path $taskOutput 'compile.rsp'
$taskCompileLog = Join-Path $taskOutput 'compile.log'
$taskRunLog = Join-Path $taskOutput 'results.txt'
$taskUtf8 = New-Object System.Text.UTF8Encoding($false)
# Compile the actual pure UI default method with a narrow class boundary; no UI is constructed.
$taskUiSource = Join-Path $taskRepo 'Client/Private/SystemOptionWindowView.cpp'
$taskUiText = [IO.File]::ReadAllText($taskUiSource)
$taskUiSignature = 'f32_t Client::CSystemOptionWindowView::Effective_Default(const SYSTEM_OPTION_ROW& Row)'
$taskUiStart = $taskUiText.IndexOf($taskUiSignature, [StringComparison]::Ordinal)
if ($taskUiStart -lt 0) { throw 'The actual UI default method was not found.' }
$taskUiEnd = $taskUiText.IndexOf('{', $taskUiStart) + 1
$taskUiDepth = 1
while ($taskUiDepth -gt 0 -and $taskUiEnd -lt $taskUiText.Length) {
    if ($taskUiText[$taskUiEnd] -eq '{') { ++$taskUiDepth }
    elseif ($taskUiText[$taskUiEnd] -eq '}') { --$taskUiDepth }
    ++$taskUiEnd
}
if ($taskUiDepth -ne 0) { throw 'The actual UI default method is incomplete.' }
$taskUiBody = $taskUiText.Substring($taskUiStart, $taskUiEnd - $taskUiStart)
$taskUiExtract = Join-Path $taskOutput 'SystemOptionDefaultExtract.cpp'
$taskUiPrefix = @'
#include "UserSettingsDocument.h"
#include "SystemOptionRowsDocument.h"
namespace Client { class CSystemOptionWindowView { public: static f32_t Effective_Default(const SYSTEM_OPTION_ROW& Row); }; }
'@
$taskUiSuffix = @'
float EffectiveDefaultForHarness(const std::string& id, float authoredDefault) {
    Client::SYSTEM_OPTION_ROW row; row.strId=id; row.fDefault=authoredDefault;
    row.eControl=Client::SYSTEM_OPTION_CONTROL::COMBOBOX;
    return Client::CSystemOptionWindowView::Effective_Default(row);
}
'@
[IO.File]::WriteAllText($taskUiExtract, $taskUiPrefix + "`r`n" + $taskUiBody + "`r`n" + $taskUiSuffix + "`r`n", $taskUtf8)
$taskSources = @(
    (Join-Path $PSScriptRoot 'UserSettingsContractHarness.cpp'),
    (Join-Path $taskRepo 'Client/Private/UserSettingsDocument.cpp'),
    (Join-Path $taskRepo 'Client/Private/DataJson.cpp'),
    $taskUiExtract
)
# Compile the actual production sources; only unrelated Engine consumers are stubbed.
$taskArguments = @(
    '/nologo', '/EHsc', '/std:c++20', '/utf-8', '/permissive-', '/DNOMINMAX',
    '/D_UNICODE', '/DUNICODE', '/W4',
    ('/I"{0}"' -f (Join-Path $PSScriptRoot 'stubs')),
    ('/I"{0}"' -f (Join-Path $taskRepo 'Client/Public')),
    ('/Fo"{0}/"' -f $taskOutput), ('/Fe"{0}"' -f $taskExecutable)
)
$taskArguments += $(if ($Configuration -eq 'Debug') { @('/D_DEBUG', '/MDd') } else { @('/DNDEBUG', '/MD') })
$taskArguments += $taskSources | ForEach-Object { '"{0}"' -f $_ }
$taskArguments += @('/link', '/INCREMENTAL:NO', 'user32.lib')
[IO.File]::WriteAllText($taskResponse, ($taskArguments -join "`r`n") + "`r`n", $taskUtf8)
$taskCommand = 'call "{0}" -no_logo -arch=x64 -host_arch=x64 && cl.exe @"{1}"' -f $taskDevCmd, $taskResponse
$taskPreviousErrorAction = $ErrorActionPreference
$taskCompileExit = -1
$taskRunExit = -1
try {
    # Native diagnostics are captured without PowerShell interpreting stderr as a failure.
    $ErrorActionPreference = 'Continue'
    $global:LASTEXITCODE = -1
    & $env:COMSPEC /d /s /c $taskCommand *> $taskCompileLog
    $taskCompileExit = $global:LASTEXITCODE
    if ($taskCompileExit -eq 0) {
        $global:LASTEXITCODE = -1
        & $taskExecutable $taskRepo (Join-Path $taskOutput 'fixtures') *> $taskRunLog
        $taskRunExit = $global:LASTEXITCODE
    }
}
finally {
    $ErrorActionPreference = $taskPreviousErrorAction
}
if ($taskCompileExit -ne 0) {
    Get-Content -LiteralPath $taskCompileLog -Tail 30
    throw "UserSettingsContractHarness compilation failed with exit code $taskCompileExit"
}
Get-Content -LiteralPath $taskRunLog
if ($taskRunExit -ne 0) { throw "UserSettingsContractHarness failed with exit code $taskRunExit" }
$taskHashes = [ordered]@{}
foreach ($taskPath in ($taskSources + @($taskUiSource, (Join-Path $taskRepo 'Client/Public/SystemOptionRowsDocument.h'), (Join-Path $taskRepo 'Client/Public/UserSettingsDocument.h'), (Join-Path $taskRepo 'Client/Public/DataJson.h')))) {
    $taskHashes[$taskPath] = (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash
}
$taskReceipt = [ordered]@{
    result = 'PASS'
    utc = [DateTime]::UtcNow.ToString('o')
    configuration = $Configuration
    compilerExitCode = $taskCompileExit
    testExitCode = $taskRunExit
    outputDirectory = $taskOutput
    sourceSha256 = $taskHashes
    boundary = 'Actual production JSON/persistence and pure UI Effective_Default body with Win32 file APIs; stubbed UI class, Engine consumers and display callback. No Client, UI, GPU, or display switching.'
}
[IO.File]::WriteAllText((Join-Path $taskOutput 'receipt.json'), ($taskReceipt | ConvertTo-Json -Depth 5) + "`r`n", $taskUtf8)
