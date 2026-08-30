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
