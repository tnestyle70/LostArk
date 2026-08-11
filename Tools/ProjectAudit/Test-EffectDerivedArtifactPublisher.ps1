[CmdletBinding()]
param(
    [string]$RepositoryRoot = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

$builder = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\build_effect_derived_artifact.py'
$tests = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\test_build_effect_derived_artifact.py'
$publisher = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\Publish-Effects.ps1'
$schema = Join-Path $RepositoryRoot `
    'Tools\EffectPipeline\Schemas\lostark.effect-derived-artifact-contract.schema.json'

foreach ($path in @($builder, $tests, $publisher, $schema)) {
    if (-not [IO.File]::Exists($path)) {
        throw "Missing derived Effect publisher contract file: $path"
    }
}

$schemaRoot = [IO.File]::ReadAllText($schema, [Text.Encoding]::UTF8) |
    ConvertFrom-Json
if ($schemaRoot.'$schema' -cne 'https://json-schema.org/draft/2020-12/schema' -or
    $null -eq $schemaRoot.'$defs'.buildRequest -or
    $null -eq $schemaRoot.'$defs'.executionContract -or
    $null -eq $schemaRoot.'$defs'.sourceContractInput -or
    $null -eq $schemaRoot.'$defs'.typedUpstreamInput -or
    $null -eq $schemaRoot.'$defs'.compilerReceiptInput -or
    $null -eq $schemaRoot.'$defs'.authoringCarrier -or
    $null -eq $schemaRoot.'$defs'.assemblyCarrier -or
    $null -eq $schemaRoot.'$defs'.compiledArtifact -or
    $null -eq $schemaRoot.'$defs'.compiledReceipt -or
    $null -eq $schemaRoot.'$defs'.runtimeCatalog) {
    throw 'Derived Effect artifact JSON schema is incomplete.'
}

$tokens = $null
$errors = $null
[void][System.Management.Automation.Language.Parser]::ParseFile(
    $publisher, [ref]$tokens, [ref]$errors)
if (@($errors).Count -ne 0) {
    throw "Publish-Effects.ps1 parse failed: $($errors[0].Message)"
}

$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
$output = (& python -B $tests 2>&1 | Out-String).Trim()
$testExitCode = $LASTEXITCODE
$ErrorActionPreference = $previousErrorActionPreference
if ($testExitCode -ne 0 -or $output -notmatch 'Ran 28 tests' -or
    $output -notmatch '(?m)^OK$') {
    throw "Derived Effect artifact publisher tests failed: $output"
}

$commonGitDirectory = (& git -C $RepositoryRoot rev-parse --git-common-dir).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($commonGitDirectory)) {
    throw 'Cannot resolve common Git directory for Effect resource validation.'
}
if (-not [IO.Path]::IsPathRooted($commonGitDirectory)) {
    $commonGitDirectory = Join-Path $RepositoryRoot $commonGitDirectory
}
$canonicalResourceRoot = Join-Path `
    (Split-Path -Parent ([IO.Path]::GetFullPath($commonGitDirectory))) `
    'Client\Bin\Resources'
if (-not [IO.Directory]::Exists($canonicalResourceRoot)) {
    throw "Missing canonical Effect resource root: $canonicalResourceRoot"
}

$validateOutput = (& powershell -NoProfile -ExecutionPolicy Bypass -File `
    $publisher -Mode Validate -ResourceRoot $canonicalResourceRoot `
    2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or
    $validateOutput -notmatch 'PASS: validated 102 Effect catalog entries\.') {
    throw "Actual Effect publisher Validate failed: $validateOutput"
}

$runtimePath = Join-Path $RepositoryRoot `
    'Client\Bin\DataFiles\Effect\EffectCatalog.runtime.json'
$sourcePath = Join-Path $RepositoryRoot 'Data\Effects\EffectCatalog.json'
if (-not [IO.File]::Exists($runtimePath) -or
    -not [IO.File]::Exists($sourcePath)) {
    throw 'Actual Effect source/runtime catalog is missing.'
}
& python -B $builder validate-runtime-catalog --catalog $runtimePath
if ($LASTEXITCODE -ne 0) {
    throw 'Actual Effect runtime catalog failed strict Python validation.'
}

$runtime = Get-Content -LiteralPath $runtimePath -Raw -Encoding UTF8 |
    ConvertFrom-Json
$source = Get-Content -LiteralPath $sourcePath -Raw -Encoding UTF8 |
    ConvertFrom-Json
$sourceEntry = @($source.effects | Where-Object {
    $_.effectAssetId -ceq 'effect.artist.skill.31470'
})
$runtimeEntry = @($runtime.effects | Where-Object {
    $_.effectAssetId -ceq 'effect.artist.skill.31470'
})
if ($sourceEntry.Count -ne 1 -or $runtimeEntry.Count -ne 1) {
    throw 'Artist 31470 source/runtime denominator changed.'
}
$sourceEntry = $sourceEntry[0]
$runtimeEntry = $runtimeEntry[0]
$sourceKeyOrder = @($sourceEntry.PSObject.Properties.Name) -join ','
$outerKeyOrder = @($runtimeEntry.PSObject.Properties.Name) -join ','
$resourceLink = $runtimeEntry.reconstructedRenderResourceAuthority
$resourceReceipt = $runtimeEntry.renderResourcePublishReceipt
$resourceLinkKeyOrder = @($resourceLink.PSObject.Properties.Name) -join ','
$resourceReceiptKeyOrder = @($resourceReceipt.PSObject.Properties.Name) -join ','
if ([int]$source.formatVersion -ne 1 -or @($source.effects).Count -ne 102 -or
    $sourceKeyOrder -cne
        'effectAssetId,payloadKind,reconstructedRuntimeProgramPath,reconstructedRenderResourceAuthorityPath' -or
    $sourceEntry.payloadKind -cne 'IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM' -or
    $sourceEntry.reconstructedRuntimeProgramPath -cne
        'Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json' -or
    $sourceEntry.reconstructedRenderResourceAuthorityPath -cne
        'Effects/Imported/Artist/Materials/skill.31470.reconstructed-render-resource-authority.receipt.json' -or
    $runtime.schema -cne 'lostark.effect-runtime-catalog' -or
    [int]$runtime.formatVersion -ne 3 -or
    @($runtime.effects).Count -ne 102 -or @($runtime.components).Count -ne 555 -or
    $outerKeyOrder -cne
        'payloadKind,effectAssetId,artifactRevision,compilerRevision,sourceExact,runtimeExecutionAdmission,productAdmission,publishReceiptSha256,publishReceipt,reconstructedRuntimeProgram,renderResourcePublishReceiptSha256,renderResourcePublishReceipt,reconstructedRenderResourceAuthority' -or
    $resourceLinkKeyOrder -cne
        'schema,formatVersion,encoding,effectAssetId,programId,programVersion,programSha256,sidecarSchema,sidecarFormatVersion,sidecarAuthorityId,sidecarDecisionProjectionSha256,sidecarReceiptSha256,sidecarRawSha256,sidecarByteCount,sourceExact,runtimeExecutionAdmission,executeAdmission,submitAdmission,renderAdmission,productAdmission,sidecarUtf8Json' -or
    $resourceReceiptKeyOrder -cne
        'schema,formatVersion,receiptRole,payloadKind,effectAssetId,artifactRevision,compilerRevision,sourceExact,runtimeExecutionAdmission,executeAdmission,submitAdmission,renderAdmission,productAdmission,programId,programVersion,programSha256,baseRuntimeEntryProjectionSha256,reconstructedRuntimeProgramSha256,basePublishReceiptSha256,renderResourceAuthorityLinkSha256,sidecarRawSha256,sidecarReceiptSha256,sidecarDecisionProjectionSha256,toolDependencies,receiptSha256Domain,receiptSha256' -or
    @($resourceReceipt.toolDependencies).Count -ne 3 -or
    [bool]$runtimeEntry.sourceExact -or
    [bool]$runtimeEntry.runtimeExecutionAdmission -or
    [bool]$runtimeEntry.productAdmission -or
    [bool]$resourceLink.sourceExact -or
    [bool]$resourceLink.runtimeExecutionAdmission -or
    [bool]$resourceLink.executeAdmission -or
    [bool]$resourceLink.submitAdmission -or
    [bool]$resourceLink.renderAdmission -or
    [bool]$resourceLink.productAdmission -or
    $resourceReceipt.baseRuntimeEntryProjectionSha256 -cne
        'e9694f000a50a426386afd6ff8f65b4a2a5fcafe9883860efff9103e1fff82d2' -or
    $resourceReceipt.reconstructedRuntimeProgramSha256 -cne
        '74175fe1e41b22ae593a9d1ff92027606bc0b31d62d17927ef6ac5673dd4a7a2' -or
    $resourceReceipt.basePublishReceiptSha256 -cne
        '92c883f78d88018a50d8dec09eb6fb155974bec4b3756a796b3499fc2f839d94' -or
    $resourceLink.sidecarRawSha256 -cne
        'bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff' -or
    $resourceLink.sidecarReceiptSha256 -cne
        'bd05c7dca6bdef205b27c208644be19bb94bdbef2e05712bfc49b9b946d8f28a' -or
    $resourceLink.sidecarDecisionProjectionSha256 -cne
        '4efa9ea724df336a5f3af719e24211b7206fe21dfd97becc630f88c5dbd9b412' -or
    [int]$resourceLink.sidecarByteCount -ne 746788 -or
    $null -ne $runtimeEntry.PSObject.Properties['reconstructedRuntimeProgramPath'] -or
    $null -ne $runtimeEntry.PSObject.Properties['reconstructedRenderResourceAuthorityPath']) {
    throw 'Actual reconstructed render-resource bridge contract changed.'
}

$sidecarBytes = [Text.UTF8Encoding]::new($false).GetBytes(
    [string]$resourceLink.sidecarUtf8Json)
$sha256 = [Security.Cryptography.SHA256]::Create()
try {
    $embeddedSidecarSha = ([BitConverter]::ToString(
        $sha256.ComputeHash($sidecarBytes))).Replace('-', '').ToLowerInvariant()
}
finally {
    $sha256.Dispose()
}
if ($sidecarBytes.Length -ne 746788 -or
    $embeddedSidecarSha -cne
        'bc5cd1accbbe3c628993a47093dc829eec6f050ab8467fca82f6b7bcf2dfe0ff') {
    throw 'Embedded render-resource sidecar bytes changed.'
}

$runtimeRaw = (Get-FileHash -LiteralPath $runtimePath -Algorithm SHA256).Hash.ToLowerInvariant()
Write-Output (
    'PASS: derived Effect artifact publisher schema tests=28 ' +
    'reserved-reconstructed-id=true current-tools=3 duplicate-json-keys=true ' +
    'duplicate-json-walk=true clean-checkout-lf=true reconstructed-source-id=true ' +
    'authenticated-blocker-union=true bridge=13/21/26/tool3 ' +
    'catalog=102/555 sourceExact=false runtime=false execute=false submit=false ' +
    'render=false product=false rollback=true ' +
    "link=$($resourceReceipt.renderResourceAuthorityLinkSha256) " +
    "receipt=$($resourceReceipt.receiptSha256) " +
    "outer=$($runtimeEntry.renderResourcePublishReceiptSha256) runtimeRaw=$runtimeRaw"
)
