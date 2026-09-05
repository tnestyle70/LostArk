[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Client', 'Server', 'KoukuSaydon')]
    [string]$Owner,
    [string]$ResourceRoot = '',
    [uint32]$ExpectedKoukuSaydonSourceRevision = 0,
    [ValidateRange(100, 3600000)]
    [int]$LockTimeoutMilliseconds = 300000
)

$ErrorActionPreference = 'Stop'
# Publishers pipe native tool output (git, python) that is UTF-8 and can
# carry this repository's non-ASCII path. Under msbuild the console is the
# ANSI code page, which corrupts that output before ConvertFrom-Json or
# GetFullPath ever sees it, so the whole owner process decodes as UTF-8.
[Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
$OutputEncoding = [Text.UTF8Encoding]::new($false)
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    $ResourceRoot = Join-Path $repositoryRoot 'Client\Bin\Resources'
}
else {
    $ResourceRoot = [IO.Path]::GetFullPath($ResourceRoot)
}
$receiptRoot = Join-Path $repositoryRoot 'out\BuildPipeline\receipts'
$modulePath = Join-Path $PSScriptRoot 'BuildDomainPipeline.psm1'
$manifestPath = Join-Path $PSScriptRoot 'BuildDomains.json'
Import-Module $modulePath -Force
$manifest = Read-BuildDomainManifest $manifestPath

function Assert-KoukuSaydonSourceRevision([string]$Phase) {
    if ($Owner -ne 'KoukuSaydon' -or
        $ExpectedKoukuSaydonSourceRevision -eq 0) {
        return
    }
    $sourcePath = Join-Path $repositoryRoot `
        'Data\KoukuSaydon\Gate1\KoukuSaydonComposition.json'
    try {
        $source = Get-Content -LiteralPath $sourcePath -Raw -Encoding UTF8 |
            ConvertFrom-Json
    }
    catch {
        throw "STALE_REVISION: KoukuSaydon source is unreadable during ${Phase}: $($_.Exception.Message)"
    }
    if ([string]$source.schema -cne 'lostark.kouku-saydon-composition' -or
        [uint32]$source.formatVersion -ne 1 -or
        [uint32]$source.revision -ne $ExpectedKoukuSaydonSourceRevision) {
        throw "STALE_REVISION: expected KoukuSaydon source revision $ExpectedKoukuSaydonSourceRevision during ${Phase}, found $($source.revision)."
    }
}
# Explicit runtime-data generation; compilation does not call this by default.
# Pure validation domains are selected by diagnostic profiles, not publishers.
$domainIds = if ($Owner -eq 'Client') {
    @(
        'koukusaydon.product',
        'map.kakulsaydon',
        'composition.presentation',
        'world.gameplay',
        'navigation'
    )
}
elseif ($Owner -eq 'KoukuSaydon') {
    @(
        'koukusaydon.product',
        'gameplay.balance'
    )
}
else {
    @(
        'koukusaydon.product',
        'world.gameplay',
        'navigation',
        'world.destruction',
        'gameplay.balance',
        'items.catalog',
        'valtan.rewards'
    )
}

foreach ($domainId in $domainIds) {
    Assert-KoukuSaydonSourceRevision "domain $domainId start"
    $domain = Get-BuildDomainById $manifest $domainId
    $result = Invoke-BuildDomain $repositoryRoot $domain $ResourceRoot `
        $receiptRoot -LockTimeoutMilliseconds $LockTimeoutMilliseconds
    $state = if ($result.reused) { 'REUSED' } else { 'PASS' }
    Write-Host "Build domain $domainId for ${Owner}: $state"
    Assert-KoukuSaydonSourceRevision "domain $domainId completion"
}
