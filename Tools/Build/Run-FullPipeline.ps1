[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateSet('Product', 'Core', 'FullDiagnostic')]
    [string]$Profile = 'FullDiagnostic',
    # Data-only pass: run every authoring domain publisher (the same
    # receipt-based executor the build runner uses) without compiling.
    [switch]$DataOnly,
    [string]$ResourceRoot = '',
    [ValidatePattern('^(?:[0-9a-f]{64})?$')]
    [string]$ExpectedValtanSourceRevision = ''
)

# One entry point for "make everything current":
#   1. project the Valtan split authoring sources into their Product documents;
#   2. validate that projection through the canonical valtan.product domain;
#   3. in -DataOnly, execute every FullDiagnostic domain through the canonical
#      receipt executor; otherwise delegate all remaining domain/build/harness
#      work to Invoke-BuildAndRegression.ps1.
# The order matters: PublishV2 repairs the common split-source drift before the
# validation domain is allowed to fail closed on a still-inconsistent Product.

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$runtimeResourceRoot = if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    Join-Path $repoRoot 'Client\Bin\Resources'
}
else {
    [IO.Path]::GetFullPath($ResourceRoot)
}
$buildReceiptRoot = Join-Path $repoRoot 'out\BuildPipeline\receipts'
Import-Module (Join-Path $PSScriptRoot 'BuildDomainPipeline.psm1') -Force
Push-Location $repoRoot
$timer = [Diagnostics.Stopwatch]::StartNew()
$steps = [Collections.Generic.List[object]]::new()
$script:failureClass = 'UNCLASSIFIED'
$script:currentStepLog = [Collections.Generic.List[string]]::new()
$valtanPipeline = Join-Path $repoRoot `
    'Tools\ValtanPipeline\valtan_tuning_pipeline.py'

function Get-ValtanRepositorySourceRevision {
    $manifestText = (& python $valtanPipeline --repository-root $repoRoot `
        source-manifest --repository-only | Out-String).Trim()
    if ($global:LASTEXITCODE -ne 0 -or
        [string]::IsNullOrWhiteSpace($manifestText)) {
        throw 'STALE_REVISION: could not read the canonical Valtan source identity.'
    }
    $manifestResult = $manifestText | ConvertFrom-Json
    [string]$revision = $manifestResult.payload.sourceManifestId
    if (-not [bool]$manifestResult.ok -or
        [string]$manifestResult.command -cne 'SOURCE_MANIFEST' -or
        $revision -cnotmatch '^[0-9a-f]{64}$') {
        throw 'STALE_REVISION: canonical Valtan source identity result is invalid.'
    }
    return $revision
}

function Assert-ValtanSourceRevision {
    param(
        [Parameter(Mandatory = $true)][string]$Expected,
        [Parameter(Mandatory = $true)][string]$Phase
    )
    [string]$actual = Get-ValtanRepositorySourceRevision
    if ($actual -cne $Expected) {
        throw "STALE_REVISION: expected Valtan source $Expected at $Phase, actual $actual."
    }
}

# Failure classes distinguish the owner of a stop instead of one exit code:
#   DOMAIN_VALIDATION  PublishV2 or a BuildDomains action rejected authoring data
#   STALE_REVISION     split source and projected Product disagree after PublishV2
#   OUTPUT_LOCKED      a running Client/Server still holds an EXE/DLL/PDB (LNK1104, MSB3021, MSB3027)
#   COMPILE / LINK     compiler or linker error in the product build
#   REGRESSION         product built but a protocol/contract/harness check failed
function Get-FailureClass {
    param([string]$StepName, [string]$Log, [string]$Message)
    $text = "$Log`n$Message"
    if ($text -match 'LNK1104|MSB3021|MSB3027|Standard Product output is in use') {
        return 'OUTPUT_LOCKED'
    }
    if ($text -match 'error LNK\d+') { return 'LINK' }
    if ($text -match 'error C\d{4}|fatal error C\d{4}') { return 'COMPILE' }
    if ($text -match ('STALE_REVISION|split authoring Product drift|source closure changed|' +
            'sources changed|stale (?:revision|product)|revision mismatch|' +
            'identity changed')) {
        return 'STALE_REVISION'
    }
    if ($text -match 'Build-domain action failed') { return 'DOMAIN_VALIDATION' }
    if ($StepName -like 'Valtan split Product PublishV2*') {
        return 'DOMAIN_VALIDATION'
    }
    if ($StepName -like 'domain:*') { return 'DOMAIN_VALIDATION' }
    if ($StepName -like 'Valtan split source Validate*') { return 'STALE_REVISION' }
    if ($StepName -like 'Build + regression*') { return 'REGRESSION' }
    return 'UNCLASSIFIED'
}

function Get-FailureExitCode {
    param([string]$FailureClass)
    switch ($FailureClass) {
        'DOMAIN_VALIDATION' { return 2 }
        'STALE_REVISION' { return 3 }
        'OUTPUT_LOCKED' { return 4 }
        'COMPILE' { return 5 }
        'LINK' { return 6 }
        'REGRESSION' { return 7 }
        default { return 1 }
    }
}

function Write-StepOutput {
    param([Parameter(ValueFromPipeline = $true)]$Line)
    process {
        if ($null -ne $Line) {
            $script:currentStepLog.Add([string]$Line) | Out-Null
            Write-Host $Line
        }
    }
}

function Invoke-PipelineStep {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Body
    )
    $stepTimer = [Diagnostics.Stopwatch]::StartNew()
    Write-Host "==> $Name"
    $global:LASTEXITCODE = 0
    $result = 'PASS'
    $script:currentStepLog = [Collections.Generic.List[string]]::new()
    try {
        # PowerShell variable names are case-insensitive.  Reusing `$body`
        # here would overwrite the typed `$Body` scriptblock parameter with
        # the step's string result (for example `PASS`) and fail conversion.
        $bodyResult = & $Body
        if ($global:LASTEXITCODE -ne 0) {
            throw "$Name failed (exit $global:LASTEXITCODE)."
        }
        if ($null -ne $bodyResult -and $bodyResult -is [string]) {
            $result = $bodyResult
        }
    }
    catch {
        $stepTimer.Stop()
        $script:failureClass = Get-FailureClass $Name ($script:currentStepLog -join "`n") $_.Exception.Message
        $steps.Add([pscustomobject]@{ step = $Name; result = "FAIL ($script:failureClass)"; ms = $stepTimer.ElapsedMilliseconds }) | Out-Null
        throw
    }
    $stepTimer.Stop()
    $steps.Add([pscustomobject]@{ step = $Name; result = $result; ms = $stepTimer.ElapsedMilliseconds }) | Out-Null
}

$previousResourceRoot = [Environment]::GetEnvironmentVariable('LOSTARK_RESOURCE_ROOT', 'Process')
try {
    [Environment]::SetEnvironmentVariable('LOSTARK_RESOURCE_ROOT', $runtimeResourceRoot, 'Process')

    $manifest = Read-BuildDomainManifest (Join-Path $PSScriptRoot 'BuildDomains.json')
    [string]$pinnedValtanSourceRevision = Get-ValtanRepositorySourceRevision
    if (-not [string]::IsNullOrWhiteSpace($ExpectedValtanSourceRevision) -and
        $pinnedValtanSourceRevision -cne $ExpectedValtanSourceRevision) {
        throw "STALE_REVISION: requested Valtan source $ExpectedValtanSourceRevision, current $pinnedValtanSourceRevision."
    }

    Invoke-PipelineStep 'Valtan split Product PublishV2' {
        Assert-ValtanSourceRevision $pinnedValtanSourceRevision 'PublishV2 start'
        & powershell -NoProfile -ExecutionPolicy Bypass -File `
            'Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1' `
            -Mode PublishV2 -RepositoryRoot $repoRoot 2>&1 | Write-StepOutput
        Assert-ValtanSourceRevision $pinnedValtanSourceRevision 'PublishV2 completion'
    }

    $valtanDomain = Get-BuildDomainById $manifest 'valtan.product'
    Invoke-PipelineStep 'Valtan split source Validate (post-condition)' {
        Assert-ValtanSourceRevision $pinnedValtanSourceRevision `
            'valtan.product validation start'
        $domainResult = Invoke-BuildDomain $repoRoot $valtanDomain `
            $runtimeResourceRoot $buildReceiptRoot
        Assert-ValtanSourceRevision $pinnedValtanSourceRevision `
            'valtan.product validation completion'
        if ($domainResult.reused) { 'REUSED' } else { 'PASS' }
    }

    if ($DataOnly) {
        foreach ($domain in @(Get-BuildDomainsForProfile $manifest 'FullDiagnostic')) {
            if ([string]$domain.id -ceq 'valtan.product') { continue }
            Invoke-PipelineStep "domain: $($domain.id)" {
                Assert-ValtanSourceRevision $pinnedValtanSourceRevision `
                    "domain $($domain.id) start"
                $domainResult = Invoke-BuildDomain $repoRoot $domain `
                    $runtimeResourceRoot $buildReceiptRoot
                Assert-ValtanSourceRevision $pinnedValtanSourceRevision `
                    "domain $($domain.id) completion"
                if ($domainResult.reused) { 'REUSED' } else { 'PASS' }
            }
        }
    }
    else {
        Invoke-PipelineStep "Build + regression ($Configuration / $Profile)" {
            Assert-ValtanSourceRevision $pinnedValtanSourceRevision `
                'build and regression start'
            # unittest writes progress to stderr even on success.  Under the
            # runner's Stop preference PowerShell wraps those lines as error
            # records, so capture both streams with Continue and decide only
            # from the child process exit code.
            $previousBuildPreference = $ErrorActionPreference
            $buildExitCode = -1
            try {
                $ErrorActionPreference = 'Continue'
                & powershell -NoProfile -ExecutionPolicy Bypass -File `
                    'Tools/Build/Invoke-BuildAndRegression.ps1' `
                    -Configuration $Configuration -Profile $Profile `
                    -ResourceRoot $runtimeResourceRoot `
                    -ExpectedValtanSourceRevision $pinnedValtanSourceRevision `
                    2>&1 | Write-StepOutput
                $buildExitCode = $global:LASTEXITCODE
            }
            finally {
                $ErrorActionPreference = $previousBuildPreference
            }
            if ($buildExitCode -ne 0) {
                throw "Build + regression child failed (exit $buildExitCode)."
            }
            $global:LASTEXITCODE = 0
            Assert-ValtanSourceRevision $pinnedValtanSourceRevision `
                'build and regression completion'
        }
    }
    Assert-ValtanSourceRevision $pinnedValtanSourceRevision `
        'full pipeline receipt'
    $timer.Stop()
    Write-Host ''
    Write-Host ("Full pipeline completed in {0:n1} s" -f ($timer.ElapsedMilliseconds / 1000.0))
    $steps | Format-Table -AutoSize | Out-String | Write-Host
    Write-Host "FULL_PIPELINE_SOURCE_REVISION`t$pinnedValtanSourceRevision"
    Write-Host 'Restart Server.exe to load the published bootstrap. Client visual/audio checks remain user-operated.'
}
catch {
    $timer.Stop()
    if ($script:failureClass -ceq 'UNCLASSIFIED') {
        $script:failureClass = Get-FailureClass `
            'Full pipeline admission' '' $_.Exception.Message
    }
    Write-Host ''
    Write-Host "Full pipeline stopped [$script:failureClass]: $($_.Exception.Message)"
    Write-Host "FULL_PIPELINE_FAILURE_CLASS=$script:failureClass"
    $steps | Format-Table -AutoSize | Out-String | Write-Host
    exit (Get-FailureExitCode $script:failureClass)
}
finally {
    [Environment]::SetEnvironmentVariable('LOSTARK_RESOURCE_ROOT', $previousResourceRoot, 'Process')
    Pop-Location
}
