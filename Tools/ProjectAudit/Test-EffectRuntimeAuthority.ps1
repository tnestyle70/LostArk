[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [string]$HarnessPath = ''
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    $RepositoryRoot = [IO.Path]::GetFullPath($RepositoryRoot)
}

function Read-RequiredSource([string]$RelativePath) {
    $path = Join-Path $RepositoryRoot $RelativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required Effect runtime authority file is missing: $RelativePath"
    }
    return [IO.File]::ReadAllText($path)
}

$header = Read-RequiredSource 'Client\Public\Effect_RuntimeAuthority.h'
$source = Read-RequiredSource 'Client\Private\Effect_RuntimeAuthority.cpp'
$catalogHeader = Read-RequiredSource 'Client\Public\Effect_Catalog.h'
$catalogSource = Read-RequiredSource 'Client\Private\Effect_Catalog.cpp'
$jsonHeader = Read-RequiredSource 'Client\Public\DataJson.h'
$jsonSource = Read-RequiredSource 'Client\Private\DataJson.cpp'
$harness = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Private\ClientFrontendHarness.cpp'
$clientProject = Read-RequiredSource 'Client\Default\Client.vcxproj'
$clientFilters = Read-RequiredSource 'Client\Default\Client.vcxproj.filters'
$harnessProject = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj'
$harnessFilters = Read-RequiredSource `
    'Tools\ClientFrontendHarness\Default\ClientFrontendHarness.vcxproj.filters'

foreach ($type in @(
    'EFFECT_RUNTIME_DERIVED_IDENTITY',
    'EFFECT_RUNTIME_EXECUTION_CONTRACT',
    'EFFECT_RUNTIME_HANDLER_RECEIPT',
    'EFFECT_RUNTIME_AUTHORITY_IDENTITY',
    'EFFECT_COMPILED_RUNTIME_DOCUMENT',
    'CEffectRuntimeAuthorityCodec')) {
    if ($header -notmatch [regex]::Escape($type)) {
        throw "Effect runtime authority public type is missing: $type"
    }
}
foreach ($identity in @(
    'strSourceContractHash',
    'strSourceSemanticClosureHash',
    'strGeometryContractHash',
    'strMaterialContractHash',
    'strResourceBindingHash',
    'strCompilerInputHash',
    'strCompiledIrSha256',
    'strCompilerReceiptTokenSha256')) {
    if ($header -notmatch [regex]::Escape($identity)) {
        throw "Effect runtime authority identity is missing: $identity"
    }
}
foreach ($boundary in @(
    'IMMUTABLE_COMPILED_IR',
    'COMPILED_AUTHORITY_EXTERNAL_AUTHENTICATION_PENDING',
    'TYPED_RUNTIME_PROGRAM_ADAPTER_PENDING',
    'bTypedProgramMaterialized = false',
    'bExternalIdentityAuthenticated = false',
    'bRuntimeExecutionAdmission = false',
    'bProductAdmission = false',
    'Compiled Effect IR canonical SHA mismatch',
    'Compiled Effect handler execution contract mismatch',
    'Compiled Effect receipt identity mismatch')) {
    if ($source -notmatch [regex]::Escape($boundary)) {
        throw "Effect runtime authority fail-closed boundary is missing: $boundary"
    }
}
foreach ($catalogBoundary in @(
    'g_RuntimeAuthorities',
    'Find_RuntimeAuthority',
    'Contains_RuntimeAuthority',
    'Get_RuntimeAuthorityAssetIds',
    'legacy formatVersion 2 or typed formatVersion 3',
    'Parse_DerivedEntry')) {
    if ($catalogHeader -notmatch [regex]::Escape($catalogBoundary) -and
        $catalogSource -notmatch [regex]::Escape($catalogBoundary)) {
        throw "Effect Catalog format3 boundary is missing: $catalogBoundary"
    }
}
if ($catalogSource -match 'ResourceDocument.*RuntimeAuthorit' -or
    $catalogSource -match 'Compile_Assembly\(.*RuntimeAuthorit') {
    throw 'Compiled runtime authority must not become a raw drawable document.'
}
if ($jsonHeader -notmatch 'Was_FloatingPointToken' -or
    $jsonSource -notmatch 'bFloatingPointToken') {
    throw 'Canonical runtime JSON must preserve integer versus floating-point token domains.'
}
foreach ($registration in @(
    $clientProject, $clientFilters, $harnessProject, $harnessFilters)) {
    if ($registration -notmatch 'Effect_RuntimeAuthority') {
        throw 'Effect runtime authority project/filter registration is missing.'
    }
}
foreach ($test in @(
    'Runtime Authority Canonical JSON Preserves Integer And Float Token Domains',
    'Format3 Compiled Authority Parses As Immutable Non-Executable Runtime Input',
    'Format3 Product Promotion Rejects Before Replacing Parsed Authority',
    'Format3 Cross-Layer Compiled IR Hash Mutation Rejects Transactionally',
    'Format3 Catalog Commits Compiled Authority Without Raw Drawable Document',
    'Format3 Floating Point Version Rejects And Preserves Prior Catalog',
    'Format3 Catalog Failed Reload Preserves Prior Revision And Pointer')) {
    if ($harness -notmatch [regex]::Escape($test)) {
        throw "Effect runtime authority harness evidence is missing: $test"
    }
}

if (-not [string]::IsNullOrWhiteSpace($HarnessPath)) {
    $resolvedHarness = [IO.Path]::GetFullPath($HarnessPath)
    if (-not (Test-Path -LiteralPath $resolvedHarness -PathType Leaf)) {
        throw "Effect runtime authority harness executable is missing: $resolvedHarness"
    }
    $output = & $resolvedHarness '--effect-runtime-authority' 2>&1
    if ($LASTEXITCODE -ne 0 -or
        ($output -join "`n") -notmatch 'failures : 0') {
        throw "Effect runtime authority harness failed:`n$($output -join "`n")"
    }
}

Write-Output `
    'PASS: format3 immutable compiled authority is identity-bound, transactionally staged, and runtime/Product blocked'
