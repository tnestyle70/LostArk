param(
    [Parameter(Mandatory=$true)][string]$SourceHeader,
    [Parameter(Mandatory=$true)][string]$SourceCpp,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [ValidateSet('Debug','Release')][string]$Configuration='Debug',
    [ValidateSet('Benchmark','ValueContracts')][string]$Probe='Benchmark',
    [string]$Variant='snapshot',
    [string]$RepoRoot,
    [string]$VcVarsPath,
    [ValidatePattern('^[0-9.]+$')][string]$ToolsetVersion='14.44',
    [ValidatePattern('^[0-9.]+$')][string]$WindowsSdkVersion='10.0.26100.0'
)
$ErrorActionPreference='Stop'
if(!$RepoRoot){$RepoRoot=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)}
$repo=(Resolve-Path -LiteralPath $RepoRoot).Path
$header=(Resolve-Path -LiteralPath $SourceHeader).Path
$cpp=(Resolve-Path -LiteralPath $SourceCpp).Path
$output=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
if(Test-Path -LiteralPath $output){throw "Output directory already exists: $output"}
if(!$VcVarsPath){
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if(!(Test-Path -LiteralPath $vswhere)){throw 'vswhere unavailable; specify -VcVarsPath.'}
    $installation=@(& $vswhere -latest -prerelease -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
    if($LASTEXITCODE -or !$installation.Count){throw 'Visual C++ installation unavailable; specify -VcVarsPath.'}
    $VcVarsPath=Join-Path $installation[0] 'VC/Auxiliary/Build/vcvars64.bat'
}
$vcvars=(Resolve-Path -LiteralPath $VcVarsPath).Path
$nativeName=if($Probe -eq 'ValueContracts'){'ValueContracts.cpp'}else{'DataJsonLoadingBenchmark.cpp'}
$executableName=if($Probe -eq 'ValueContracts'){'value-contracts.exe'}else{'benchmark.exe'}
$native=Join-Path $PSScriptRoot $nativeName
$defines=Join-Path $repo 'Client/Public/Client_Defines.h'
$engine=Join-Path $repo 'EngineSDK/Inc'
if(!(Test-Path -LiteralPath $defines) -or !(Test-Path -LiteralPath $engine)){throw 'Actual Client/EngineSDK headers are required.'}
foreach($path in @($repo,$header,$cpp,$output,$vcvars,$native)){
    if($path.IndexOfAny([char[]]@('"',"`r","`n",'%')) -ge 0){throw "Unsupported command path: $path"}
}
$sourceDirectory=Join-Path $output 'sources'
New-Item -ItemType Directory -Path $sourceDirectory -Force|Out-Null
$inputs=@(@{original=$header;name='DataJson.h'},@{original=$cpp;name='DataJson.cpp'},@{original=$defines;name='Client_Defines.h'})
$sourceRecords=@(foreach($inputFile in $inputs){
    $before=(Get-FileHash -LiteralPath $inputFile.original -Algorithm SHA256).Hash.ToLowerInvariant()
    $snapshot=Join-Path $sourceDirectory $inputFile.name
    Copy-Item -LiteralPath $inputFile.original -Destination $snapshot
    if($before -ne (Get-FileHash -LiteralPath $inputFile.original).Hash.ToLowerInvariant() -or $before -ne (Get-FileHash -LiteralPath $snapshot).Hash.ToLowerInvariant()){throw 'Source changed while snapshotting.'}
    [ordered]@{path=$inputFile.original;snapshot=$snapshot;sha256=$before}
})
$sourceRecords+=@([ordered]@{path=$native;sha256=(Get-FileHash -LiteralPath $native).Hash.ToLowerInvariant()})
$engineRecords=@(Get-ChildItem -LiteralPath $engine -Filter '*.h'|ForEach-Object{[ordered]@{path=$_.FullName;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash.ToLowerInvariant()}})
$flags=@('/nologo','/EHsc','/std:c++20','/utf-8','/wd4828','/W3','/O2','/sdl','/permissive-','/DNOMINMAX','/D_UNICODE','/DUNICODE','/D_WINDOWS')
if($Configuration -eq 'Debug'){$flags+=@('/MDd','/D_DEBUG','/D_ITERATOR_DEBUG_LEVEL=2')}else{$flags+=@('/MD','/DNDEBUG','/D_ITERATOR_DEBUG_LEVEL=0')}
$compileArgs=$flags+@('/I"'+$sourceDirectory+'"','/I"'+(Join-Path $repo 'Client/Public')+'"','/I"'+$engine+'"','/Fo"'+$output+'/"','/Fe"'+(Join-Path $output $executableName)+'"','"'+$native+'"','"'+(Join-Path $sourceDirectory 'DataJson.cpp')+'"','/link','/INCREMENTAL:NO','Psapi.lib')
$utf8=[Text.UTF8Encoding]::new($false)
$rsp=Join-Path $output 'compile.rsp'
[IO.File]::WriteAllText($rsp,($compileArgs -join "`r`n")+"`r`n",$utf8)
$command='call "'+$vcvars+'" '+$WindowsSdkVersion+' -vcvars_ver='+$ToolsetVersion+' >nul && cl.exe @"'+$rsp+'"'
$previousPreference=$ErrorActionPreference;$ErrorActionPreference='Continue'
& $env:COMSPEC /d /s /c $command *> (Join-Path $output 'compile.log')
$compileExit=$LASTEXITCODE;$ErrorActionPreference=$previousPreference
$receipt=[ordered]@{variant=$Variant;configuration=$Configuration;probe=$Probe;gitRef=(& git -C $repo rev-parse HEAD);utc=[DateTime]::UtcNow.ToString('o');command=$command;toolsetVersion=$ToolsetVersion;windowsSdkVersion=$WindowsSdkVersion;flags=$flags;compilerExitCode=$compileExit;sources=$sourceRecords;engineHeaders=$engineRecords}
[IO.File]::WriteAllText((Join-Path $output 'build-receipt.json'),($receipt|ConvertTo-Json -Depth 7),$utf8)
Get-Content -LiteralPath (Join-Path $output 'compile.log') -Tail 18
if($compileExit){throw "Native compilation failed: $compileExit"}
Write-Output (Join-Path $output $executableName)
