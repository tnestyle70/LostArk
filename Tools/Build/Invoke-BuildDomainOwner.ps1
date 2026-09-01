[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('Client', 'Server')]
    [string]$Owner,
    [string]$ResourceRoot = '',
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
$domainIds = if ($Owner -eq 'Client') {
    @(
        'map.kakul',
        'world.gameplay',
        'navigation',
        'valtan.product'
    )
}
else {
    @(
        'valtan.product',
        'world.gameplay',
        'navigation',
        'world.destruction',
        'gameplay.balance',
        'items.catalog',
        'valtan.rewards'
    )
}

foreach ($domainId in $domainIds) {
    $domain = Get-BuildDomainById $manifest $domainId
    $result = Invoke-BuildDomain $repositoryRoot $domain $ResourceRoot `
        $receiptRoot -LockTimeoutMilliseconds $LockTimeoutMilliseconds
    $state = if ($result.reused) { 'REUSED' } else { 'PASS' }
    Write-Host "Build domain $domainId for ${Owner}: $state"
}
