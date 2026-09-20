[CmdletBinding()]
param([string]$OutputDirectory = '')

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
$taskSources = @(
    (Join-Path $PSScriptRoot 'UserSettingsContractHarness.cpp'),
    (Join-Path $taskRepo 'Client/Private/UserSettingsDocument.cpp'),
    (Join-Path $taskRepo 'Client/Private/DataJson.cpp')
)
# Compile the actual production sources; only unrelated Engine consumers are stubbed.
$taskArguments = @(
    '/nologo', '/EHsc', '/std:c++20', '/utf-8', '/permissive-', '/DNOMINMAX',
    '/D_UNICODE', '/DUNICODE', '/MD', '/W4',
    ('/I"{0}"' -f (Join-Path $PSScriptRoot 'stubs')),
    ('/I"{0}"' -f (Join-Path $taskRepo 'Client/Public')),
    ('/Fo"{0}/"' -f $taskOutput), ('/Fe"{0}"' -f $taskExecutable)
)
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
foreach ($taskPath in ($taskSources + @((Join-Path $taskRepo 'Client/Public/UserSettingsDocument.h'), (Join-Path $taskRepo 'Client/Public/DataJson.h')))) {
    $taskHashes[$taskPath] = (Get-FileHash -LiteralPath $taskPath -Algorithm SHA256).Hash
}
$taskReceipt = [ordered]@{
    result = 'PASS'
    utc = [DateTime]::UtcNow.ToString('o')
    compilerExitCode = $taskCompileExit
    testExitCode = $taskRunExit
    outputDirectory = $taskOutput
    sourceSha256 = $taskHashes
    boundary = 'Actual production JSON/persistence sources and Win32 file APIs; stubbed Engine consumers and display callback. No Client, UI, GPU, or display switching.'
}
[IO.File]::WriteAllText((Join-Path $taskOutput 'receipt.json'), ($taskReceipt | ConvertTo-Json -Depth 5) + "`r`n", $taskUtf8)
