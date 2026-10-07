param(
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [ValidateSet('Debug','Release')][string]$Configuration='Debug',
    [string]$SourceHeader,
    [string]$SourceCpp,
    [switch]$RequireStrongCopy,
    [string]$Variant,
    [string]$RepoRoot,
    [string]$VcVarsPath,
    [ValidatePattern('^[0-9.]+$')][string]$ToolsetVersion='14.44',
    [ValidatePattern('^[0-9.]+$')][string]$WindowsSdkVersion='10.0.26100.0'
)

$ErrorActionPreference='Stop'
if(!$RepoRoot){$RepoRoot=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)}
$repo=(Resolve-Path -LiteralPath $RepoRoot).Path
if([bool]$SourceHeader -ne [bool]$SourceCpp){
    throw 'Specify both -SourceHeader and -SourceCpp, or omit both for current product sources.'
}
$customSources=[bool]$SourceHeader
if(!$customSources){
    $SourceHeader=Join-Path $repo 'Client/Public/DataJson.h'
    $SourceCpp=Join-Path $repo 'Client/Private/DataJson.cpp'
}
if(!$Variant){$Variant=if($customSources){'custom'}else{'current'}}
$output=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
$buildArguments=@{
    SourceHeader=$SourceHeader
    SourceCpp=$SourceCpp
    OutputDirectory=$output
    Configuration=$Configuration
    Variant=$Variant
    Probe='ValueContracts'
    RepoRoot=$repo
    ToolsetVersion=$ToolsetVersion
    WindowsSdkVersion=$WindowsSdkVersion
}
if($VcVarsPath){$buildArguments.VcVarsPath=$VcVarsPath}

# Reuse the benchmark's source snapshot, compiler selection, flags and receipt.
# The shared builder requires a fresh output directory and preserves old results.
& (Join-Path $PSScriptRoot 'Build-Benchmark.ps1') @buildArguments
$executable=Join-Path $output 'value-contracts.exe'
$nativeArguments=@()
if($RequireStrongCopy){$nativeArguments+='--require-strong-copy'}
$resultText=Join-Path $output 'results.txt'
$utcStart=[DateTime]::UtcNow.ToString('o')
$previousPreference=$ErrorActionPreference
$ErrorActionPreference='Continue'
try{
    & $executable @nativeArguments *> $resultText
    $nativeExit=$LASTEXITCODE
}finally{
    $ErrorActionPreference=$previousPreference
}
$utcEnd=[DateTime]::UtcNow.ToString('o')
$text=(Get-Content -LiteralPath $resultText -Raw).Trim()
$parsed=[regex]::Match($text,
    '^checks=(\d+) allocationFailures=(\d+) iteratorDebugLevel=(\d+) strongCopy=([01]) failures=(\d+)$')
$result=[ordered]@{
    variant=$Variant
    configuration=$Configuration
    utcStart=$utcStart
    utcEnd=$utcEnd
    executable=$executable
    executableSha256=(Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant()
    arguments=$nativeArguments
    exitCode=$nativeExit
    buildReceipt=(Join-Path $output 'build-receipt.json')
    textOutput=$resultText
    requireStrongCopy=[bool]$RequireStrongCopy
    parsed=$parsed.Success
    boundary='Isolated actual DataJson value contracts; no performance measurement, Client, UI or GPU execution.'
}
if($parsed.Success){
    $result.checks=[int]$parsed.Groups[1].Value
    $result.injectedAllocationFailures=[int]$parsed.Groups[2].Value
    $result.iteratorDebugLevel=[int]$parsed.Groups[3].Value
    $result.strongCopy=($parsed.Groups[4].Value -eq '1')
    $result.failures=[int]$parsed.Groups[5].Value
}
[IO.File]::WriteAllText((Join-Path $output 'results.json'),
    ($result|ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
Write-Output $text
if($nativeExit -or !$parsed.Success -or $result.failures -ne 0){
    throw "DataJson value contracts failed; inspect $resultText"
}
