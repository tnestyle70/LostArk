[CmdletBinding()]
param(
    [string]$RepositoryRoot = '',
    [string]$HarnessPath = '',
    [string]$ReconstructedProgramPath = ''
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
    'EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM_IDENTITY',
    'EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM',
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
	'Compiled Effect receipt identity mismatch',
	'Get_FrozenArtist31470FProgramIdentity',
	'Parse_ReconstructedRuntimeProgram',
	'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS',
	'Validate_ReconstructedRuntimeProgramForHarness',
	'MAXIMUM_PROGRAM_BYTES',
    'MAXIMUM_PROGRAM_DEPTH',
    'MAXIMUM_PROGRAM_VALUES',
    'blockerOwnership',
    'a85b8b41afb2f2a51bceafa55d06bf0937b1a245',
	'72e417747dee14dd0a3be5ffd64f69f904bd696ef1acc049037fc81f38779849',
	'618d5684c94fffa2c21ec0ee911e564fd0f6a1d35fc92843d8efcaeeadd55b4b',
	'94b0bc3704e4c9aaca5216cfafbfcb397c924390395597442c6831d096f2bff5',
	'sourceDeploymentRowId',
    'sourceDeploymentRowSha256',
    'InOutProgram = std::move(Staged)')) {
    if ($source -notmatch [regex]::Escape($boundary)) {
        throw "Effect runtime authority fail-closed boundary is missing: $boundary"
	}
}
if ($header -match 'SemanticTest' -or
	$header -match 'Validate_ReconstructedRuntimeProgramForHarness' -or
	$header -match 'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS') {
	throw 'The semantic-test validator must not be part of the production public header.'
}
if ($clientProject -match 'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS') {
	throw 'The production Client project must not compile the semantic-test validator.'
}
if ($harnessProject -notmatch 'LOSTARK_EFFECT_RUNTIME_AUTHORITY_SEMANTIC_TESTS') {
	throw 'The ClientFrontendHarness project must explicitly own the semantic-test validator.'
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
    'Format3 Catalog Failed Reload Preserves Prior Revision And Pointer',
    'Artist 31470 Reconstructed Runtime Program Parses To Immutable Typed Rows',
	'Artist 31470 Reconstructed Program Rejects Malformed And Raw Identity Attacks Transactionally',
	'Artist 31470 Reconstructed Program Reaches Semantic Validation And Rejects Canonically Resealed Nested Ownership Identity And Admission Attacks Transactionally',
    'Artist 31470 Reconstructed Program Rejects A-B Builder Identity Swap Transactionally')) {
    if ($harness -notmatch [regex]::Escape($test)) {
        throw "Effect runtime authority harness evidence is missing: $test"
	}
}
foreach ($semanticAttack in @(
	'schedule-id',
	'schedule-order',
	'schedule-source-cue',
	'schedule-source-occurrence',
	'schedule-source-system',
	'schedule-source-event-index',
	'schedule-global-time',
	'schedule-duration',
	'schedule-source-cue-row-sha',
	'schedule-event-index-float-token',
	'schedule-global-time-int-token',
	'random-seed-authority',
	'renderer-config-color',
	'distribution-sample-input-digest',
	'material-d3d-descriptor',
	'geometry-carrier-byte-size',
	'Expected semantic rejection stage')) {
	if ($harness -notmatch [regex]::Escape($semanticAttack)) {
		throw "Effect runtime authority resealed semantic attack is missing: $semanticAttack"
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

    if (-not [string]::IsNullOrWhiteSpace($ReconstructedProgramPath)) {
        $resolvedProgram = [IO.Path]::GetFullPath($ReconstructedProgramPath)
        if (-not (Test-Path -LiteralPath $resolvedProgram -PathType Leaf)) {
            throw "Reconstructed runtime program candidate is missing: $resolvedProgram"
        }
        $programOutput = & $resolvedHarness `
            '--effect-reconstructed-runtime-program' $resolvedProgram 2>&1
        if ($LASTEXITCODE -ne 0 -or
            ($programOutput -join "`n") -notmatch 'failures : 0') {
            throw "Reconstructed runtime program harness failed:`n$($programOutput -join "`n")"
        }
    }
}

Write-Output `
    'PASS: format3 immutable compiled authority is identity-bound, transactionally staged, and runtime/Product blocked'
