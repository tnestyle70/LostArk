[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$RepositoryRoot = '',
    [string]$HistoryRoot = '',
    [string]$VcVarsPath = '',
    [ValidatePattern('^[0-9.]+$')][string]$ToolsetVersion = '14.44',
    [ValidatePattern('^[0-9.]+$')][string]$WindowsSdkVersion = '10.0.26100.0'
)

# Explicit, isolated value-history contract regression. It never builds or links
# Product outputs, publishes Data, or starts the game.
$ErrorActionPreference = 'Stop'
if (!$RepositoryRoot) { $RepositoryRoot = Join-Path $PSScriptRoot '../..' }
$repo = (Resolve-Path -LiteralPath $RepositoryRoot).Path
if (!$HistoryRoot) { $HistoryRoot = $repo }
$historySource = (Resolve-Path -LiteralPath $HistoryRoot).Path
$header = Join-Path $historySource 'Client/Public/EditorUndoHistory.h'
$sources = @(
    (Join-Path $PSScriptRoot 'EditorUndoHistoryContracts.cpp')
)
foreach ($inputPath in @($header) + $sources) {
    if (!(Test-Path -LiteralPath $inputPath)) { throw "Missing contract input: $inputPath" }
}
if (!$VcVarsPath) {
    if ($env:VSINSTALLDIR) { $installation = $env:VSINSTALLDIR }
    else {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
        if (!(Test-Path -LiteralPath $vswhere)) { throw 'Specify -VcVarsPath or install the Visual Studio C++ toolchain.' }
        $installation = @(& $vswhere -latest -prerelease -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
        if ($LASTEXITCODE -or !$installation.Count) { throw 'No installed Visual Studio C++ toolchain was found.' }
        $installation = $installation[0]
    }
    $VcVarsPath = Join-Path $installation 'VC/Auxiliary/Build/vcvars64.bat'
}
$vcvars = (Resolve-Path -LiteralPath $VcVarsPath).Path
$stamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
$output = Join-Path $repo "out/EditorUndoHistory/$stamp-$Configuration-$PID"
New-Item -ItemType Directory -Path $output | Out-Null
foreach ($inputPath in @($repo, $historySource, $PSScriptRoot, $vcvars, $output)) {
    if ($inputPath.IndexOfAny([char[]]@('"', "`r", "`n", '%', '!')) -ge 0) {
        throw "Unsupported command path: $inputPath"
    }
}

$flags = @('/nologo','/EHsc','/std:c++20','/utf-8','/wd4828','/W4','/Od','/sdl','/permissive-',
    '/DNOMINMAX','/D_UNICODE','/DUNICODE','/D_WINDOWS')
if ($Configuration -eq 'Debug') { $flags += @('/MDd','/D_DEBUG','/D_ITERATOR_DEBUG_LEVEL=2') }
else { $flags += @('/MD','/DNDEBUG','/D_ITERATOR_DEBUG_LEVEL=0') }
$executable = Join-Path $output 'EditorUndoHistoryContracts.exe'
$arguments = $flags + @(
    '/I"' + (Join-Path $historySource 'Client/Public') + '"',
    '/I"' + (Join-Path $repo 'Client/Public') + '"',
    '/Fo"' + $output + '/"',
    '/Fd"' + (Join-Path $output 'compiler.pdb') + '"',
    '/Fe"' + $executable + '"'
)
$arguments += @($sources | ForEach-Object { '"' + $_ + '"' })
$arguments += @('/link', '/INCREMENTAL:NO')
$utf8 = [Text.UTF8Encoding]::new($false)
$response = Join-Path $output 'compile.rsp'
[IO.File]::WriteAllText($response, ($arguments -join "`r`n") + "`r`n", $utf8)
$command = 'call "' + $vcvars + '" ' + $WindowsSdkVersion + ' -vcvars_ver=' + $ToolsetVersion + ' >nul && cl.exe @"' + $response + '"'
$receipt = [ordered]@{
    configuration = $Configuration
    startedUtc = [DateTime]::UtcNow.ToString('o')
    compilerExitCode = $null
    testExitCode = $null
    command = $command
    sources = @(@($header) + $sources | ForEach-Object {
        [ordered]@{ path = $_; sha256 = (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash.ToLowerInvariant() }
    })
}
$oldPreference = $ErrorActionPreference
try {
    $ErrorActionPreference = 'Continue'
    & $env:COMSPEC /d /v:off /s /c $command *> (Join-Path $output 'compile.log')
    $receipt.compilerExitCode = $LASTEXITCODE
    $ErrorActionPreference = $oldPreference
    Get-Content -LiteralPath (Join-Path $output 'compile.log') -Tail 30
    if ($receipt.compilerExitCode) { throw "Editor history contract compilation failed: $($receipt.compilerExitCode)" }
    $ErrorActionPreference = 'Continue'
    & $executable $output *> (Join-Path $output 'run.log')
    $receipt.testExitCode = $LASTEXITCODE
    $ErrorActionPreference = $oldPreference
    Get-Content -LiteralPath (Join-Path $output 'run.log')
    if ($receipt.testExitCode) { throw "Editor history contracts failed: $($receipt.testExitCode)" }
    Write-Output "Editor history contract evidence: $output"
}
finally {
    $ErrorActionPreference = $oldPreference
    [IO.File]::WriteAllText((Join-Path $output 'receipt.json'), ($receipt | ConvertTo-Json -Depth 5), $utf8)
}
