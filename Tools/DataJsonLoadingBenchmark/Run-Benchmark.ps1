param(
    [Parameter(Mandatory=$true)][string]$Executable,
    [Parameter(Mandatory=$true)][string]$Corpus,
    [Parameter(Mandatory=$true)][string]$Result,
    [ValidateSet(1,3)][int]$Workers=1,
    [switch]$Allocation,
    [string]$CorpusManifest,
    [string]$BuildReceipt
)
$ErrorActionPreference='Stop'
$exe=(Resolve-Path -LiteralPath $Executable).Path
$corpusPath=(Resolve-Path -LiteralPath $Corpus).Path
$resultPath=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Result)
if(Test-Path -LiteralPath $resultPath){throw "Result already exists: $resultPath"}
if(Test-Path -LiteralPath ($resultPath+'.meta.json')){throw 'Result metadata already exists.'}
if($Allocation -and $Workers -ne 1){throw 'Allocation instrumentation requires one worker.'}
if(!$CorpusManifest){$CorpusManifest=Join-Path (Split-Path -Parent $corpusPath) 'corpus-manifest.json'}
if(!$BuildReceipt){$BuildReceipt=Join-Path (Split-Path -Parent $exe) 'build-receipt.json'}
$receipt=$null
if(Test-Path -LiteralPath $BuildReceipt){$receipt=Get-Content -LiteralPath $BuildReceipt -Encoding UTF8 -Raw|ConvertFrom-Json}
if($Allocation -and (!$receipt -or $receipt.configuration -ne 'Debug')){throw 'Allocation instrumentation requires a Debug build receipt.'}
New-Item -ItemType Directory -Force (Split-Path -Parent $resultPath)|Out-Null
$nativeArgs=@($corpusPath,[string]$Workers,$resultPath)
if($Allocation){$nativeArgs+='alloc'}
$before=[DateTime]::UtcNow.ToString('o')
# Relative corpus paths, when supplied, are resolved relative to the manifest directory.
Push-Location -LiteralPath (Split-Path -Parent $corpusPath)
try{& $exe @nativeArgs;$nativeExit=$LASTEXITCODE}finally{Pop-Location}
if($nativeExit){throw "Native benchmark failed: $nativeExit"}
$value=Get-Content -LiteralPath $resultPath -Encoding UTF8 -Raw|ConvertFrom-Json
$meta=[ordered]@{runId=[IO.Path]::GetFileNameWithoutExtension($resultPath);variant=$receipt.variant;configuration=$receipt.configuration;utcStart=$before;utcEnd=[DateTime]::UtcNow.ToString('o');executable=$exe;executableSha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();arguments=$nativeArgs;buildReceipt=$BuildReceipt;corpusSha256=(Get-FileHash -LiteralPath $corpusPath).Hash.ToLowerInvariant();corpusManifestSha256=$(if(Test-Path -LiteralPath $CorpusManifest){(Get-FileHash -LiteralPath $CorpusManifest).Hash.ToLowerInvariant()}else{$null});processorCount=[Environment]::ProcessorCount;os=[Environment]::OSVersion.VersionString}
[IO.File]::WriteAllText(($resultPath+'.meta.json'),($meta|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
$value|Select-Object workers,debug,sizeofValue,contractCases,fileCount,inputBytes,failed,parseWallMs,digestWallMs,destructionWallMs,totalWallMs,processCpuMs,systemCpuBusyPercent,semanticDigestFnv1a64,parseAllocations|ConvertTo-Json
