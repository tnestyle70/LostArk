param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path,
    [string]$SourcePath = ''
)
$ErrorActionPreference = 'Stop'
if (!$SourcePath) { $SourcePath = Join-Path $RepositoryRoot 'Client/Private/MapPlacementEditSession.cpp' }
$taskOutput = Join-Path $RepositoryRoot ('out/MapPlacementHistoryContracts/' + $Configuration)
[IO.Directory]::CreateDirectory($taskOutput) | Out-Null
$source = [IO.File]::ReadAllText($SourcePath)
$start = $source.IndexOf('void Client::CMapPlacementEditSession::Reset_History()')
$end = $source.IndexOf('std::filesystem::path Client::CMapPlacementEditSession::Rollback_CopyPath()', $start)
if ($start -lt 0 -or $end -le $start) { throw 'Production history method boundaries changed.' }
$methods = $source.Substring($start, $end - $start)
$fixture = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'MapPlacementHistoryContracts.cpp.in'))
if (($fixture -split '// PRODUCTION_HISTORY_METHODS', 0, 'SimpleMatch').Count -ne 2) { throw 'Contract insertion marker is invalid.' }
$utf8 = [Text.UTF8Encoding]::new($false)
$generated = Join-Path $taskOutput 'MapPlacementHistoryContracts.cpp'
[IO.File]::WriteAllText($generated, $fixture.Replace('// PRODUCTION_HISTORY_METHODS', $methods), $utf8)
$flags = @('/nologo','/EHsc','/std:c++20','/utf-8','/W4','/Od','/sdl','/permissive-')
if ($Configuration -eq 'Debug') { $flags += @('/MDd','/D_DEBUG') } else { $flags += @('/MD','/DNDEBUG') }
$response = Join-Path $taskOutput 'compile.rsp'
$args = $flags + @('/Fo"' + $taskOutput + '/"', '/Fd"' + $taskOutput + '/compiler.pdb"', '/Fe"' + $taskOutput + '/MapPlacementHistoryContracts.exe"', '"' + $generated + '"', '/link', '/INCREMENTAL:NO')
[IO.File]::WriteAllText($response, ($args -join "`r`n"), $utf8)
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual C++ Build Tools are required.' }
$vcvars = Join-Path $vs 'VC/Auxiliary/Build/vcvars64.bat'
$command = 'call "' + $vcvars + '" >nul && cl.exe @"' + $response + '"'
& $env:COMSPEC /d /s /c $command *> (Join-Path $taskOutput 'compile.log')
if ($LASTEXITCODE) { Get-Content (Join-Path $taskOutput 'compile.log'); throw 'Map history contract compilation failed.' }
& (Join-Path $taskOutput 'MapPlacementHistoryContracts.exe') | Tee-Object -FilePath (Join-Path $taskOutput 'run.log')
if ($LASTEXITCODE) { throw 'Map history contracts failed.' }
$receipt = @{configuration=$Configuration; source=$SourcePath; sourceSha256=(Get-FileHash $SourcePath).Hash; fixtureSha256=(Get-FileHash (Join-Path $PSScriptRoot 'MapPlacementHistoryContracts.cpp.in')).Hash}
[IO.File]::WriteAllText((Join-Path $taskOutput 'receipt.json'), ($receipt | ConvertTo-Json), $utf8)
