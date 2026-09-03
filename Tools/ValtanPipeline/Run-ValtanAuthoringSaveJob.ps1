[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9a-f]{64}$')]
    [string]$ExpectedSourceRevision,
    [Parameter(Mandatory = $true)]
    [string]$ResultPath,
    [string]$DraftPatchPath = '',
    [string]$PatternSoundBaselinePath = '',
    [string]$PatternSoundCandidatePath = '',
    [string]$EffectV2BaselinePath = '',
    [string]$EffectV2CandidatePath = '',
    [switch]$CommitOnly,
    [ValidateRange(0.0, 300.0)]
    [double]$LockTimeoutSeconds = 30.0
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$pipeline = Join-Path $PSScriptRoot 'Publish-ValtanTuningRuntimeSet.ps1'
$resultFullPath = [IO.Path]::GetFullPath($ResultPath)
$resultRoot = [IO.Path]::GetDirectoryName($resultFullPath)
$manifestPath = Join-Path $resultRoot 'candidate-source-manifest.json'
$canonicalCommitted = $false
$committedSourceRevision = $ExpectedSourceRevision
$candidateRevision = $null
$applyClass = $null
$changedCount = 0
$failureClass = 'UNCLASSIFIED'

function Assert-Sha256([string]$Value, [string]$Label) {
    if ($Value -cnotmatch '^[0-9a-f]{64}$') {
        throw "STALE_REVISION: $Label is not one lowercase SHA-256 revision."
    }
}

function Invoke-StructuredPipeline([string[]]$Arguments, [string]$Phase) {
    $lines = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass `
        -File $pipeline @Arguments 2>&1)
    $exitCode = $LASTEXITCODE
    $text = ($lines | ForEach-Object { [string]$_ }) -join "`n"
    if ([string]::IsNullOrWhiteSpace($text)) {
        throw "$Phase returned no structured result (exit $exitCode)."
    }
    try {
        $document = $text.Trim() | ConvertFrom-Json
    }
    catch {
        throw "$Phase returned malformed structured output (exit $exitCode): $text"
    }
    if ($exitCode -ne 0 -or -not [bool]$document.ok) {
        $diagnostic = if ($document.errors.Count -gt 0) {
            [string]$document.errors[0].errorCode + ': ' +
                [string]$document.errors[0].message
        }
        else { $text }
        throw "$Phase failed (exit $exitCode): $diagnostic"
    }
    return $document
}

function Write-JobResult([bool]$Ok, [string]$Diagnostic) {
    $document = [ordered]@{
        schema = 'lostark.valtan-authoring-save-job-result'
        formatVersion = 1
        ok = $Ok
        expectedSourceRevision = $ExpectedSourceRevision
        canonicalCommitted = $canonicalCommitted
        sourceRevision = $committedSourceRevision
        candidateRevision = $candidateRevision
        applyClass = $applyClass
        changedCount = [uint32]$changedCount
        failureClass = if ($Ok) { $null } else { $failureClass }
        diagnostic = $Diagnostic
    }
    [IO.Directory]::CreateDirectory($resultRoot) | Out-Null
    $temporary = $resultFullPath + '.tmp.' + [string]$PID
    $utf8 = [Text.UTF8Encoding]::new($false)
    [IO.File]::WriteAllText(
        $temporary,
        ($document | ConvertTo-Json -Depth 8 -Compress) + "`n",
        $utf8)
    if ([IO.File]::Exists($resultFullPath)) {
        [IO.File]::Replace($temporary, $resultFullPath, $null, $true)
    }
    else {
        [IO.File]::Move($temporary, $resultFullPath)
    }
    Write-Output ($document | ConvertTo-Json -Depth 8 -Compress)
}

try {
    Assert-Sha256 $ExpectedSourceRevision 'expected source revision'
    if (-not [IO.File]::Exists($pipeline)) {
        throw "Missing Valtan tuning pipeline: $pipeline"
    }
    if (-not [string]::IsNullOrWhiteSpace($DraftPatchPath)) {
        $failureClass = 'COMMIT_FAILED'
        $commitArguments = @(
            '-Mode', 'CommitCanonicalDraft',
            '-DraftPatchPath', [IO.Path]::GetFullPath($DraftPatchPath),
            '-LockTimeoutSeconds',
            $LockTimeoutSeconds.ToString(
                [Globalization.CultureInfo]::InvariantCulture)
        )
        $pairs = @(
            @('-PatternSoundBaselinePath', $PatternSoundBaselinePath),
            @('-PatternSoundCandidatePath', $PatternSoundCandidatePath),
            @('-EffectV2BaselinePath', $EffectV2BaselinePath),
            @('-EffectV2CandidatePath', $EffectV2CandidatePath)
        )
        foreach ($pair in $pairs) {
            if (-not [string]::IsNullOrWhiteSpace([string]$pair[1])) {
                $commitArguments += @([string]$pair[0],
                    [IO.Path]::GetFullPath([string]$pair[1]))
            }
        }
        $commit = Invoke-StructuredPipeline $commitArguments 'canonical commit'
        if ([string]$commit.command -cne 'COMMIT_CANONICAL_DRAFT' -or
            $null -ne $commit.candidateRevision -or
            [string]$commit.payload.previousSourceRevision -cne
                $ExpectedSourceRevision -or
            [string]$commit.payload.runtimeActivation -cne 'NOT_ACTIVATED') {
            throw 'STALE_REVISION: canonical commit receipt does not match the immutable request.'
        }
        Assert-Sha256 ([string]$commit.sourceRevision) `
            'committed source revision'
        $committedSourceRevision = [string]$commit.sourceRevision
        $changedCount = [uint32]$commit.payload.changedCount
        $canonicalCommitted = $true
        # Persist the irreversible boundary before any later read/candidate
        # stage. If this wrapper is interrupted, the Client can still classify
        # the exact commit and must not repeat the source write.
        $failureClass = 'CANDIDATE_PENDING'
        Write-JobResult $false 'Canonical commit succeeded; candidate publication is pending.'
    }

    $failureClass = 'STALE_REVISION'
    $manifest = Invoke-StructuredPipeline @(
        '-Mode', 'SourceManifest',
        '-SourceManifestPath', $manifestPath
    ) 'candidate source manifest'
    if ([string]$manifest.command -cne 'SOURCE_MANIFEST' -or
        [string]$manifest.sourceRevision -cne $committedSourceRevision -or
        [string]$manifest.payload.sourceManifestId -cne
            $committedSourceRevision -or
        $null -ne $manifest.payload.authoringRevision -or
        -not [bool]$manifest.payload.splitJoinValidated) {
        throw 'STALE_REVISION: committed source did not reopen as one validated repository join.'
    }

    if (-not $CommitOnly) {
        $failureClass = 'CANDIDATE_FAILED'
        $candidate = Invoke-StructuredPipeline @(
            '-Mode', 'PublishCandidate',
            '-SourceManifestPath', $manifestPath,
            '-LockTimeoutSeconds',
            $LockTimeoutSeconds.ToString(
                [Globalization.CultureInfo]::InvariantCulture)
        ) 'candidate publish'
        if ([string]$candidate.command -cne 'PUBLISH_CANDIDATE' -or
            [string]$candidate.sourceRevision -cne $committedSourceRevision) {
            throw 'STALE_REVISION: candidate receipt does not match the committed source.'
        }
        Assert-Sha256 ([string]$candidate.candidateRevision) 'candidate revision'
        $candidateRevision = [string]$candidate.candidateRevision
        $applyClass = [string]$candidate.payload.applyClass
        if ($applyClass -notin @('HOT_RELOAD', 'ENCOUNTER_RESET', 'SERVER_RESTART')) {
            throw 'Candidate receipt contains an unsupported apply class.'
        }
    }
    Write-JobResult $true ''
    exit 0
}
catch {
    if ($_.Exception.Message -match '^STALE_REVISION:') {
        $failureClass = 'STALE_REVISION'
    }
    Write-JobResult $false $_.Exception.Message
    exit 1
}
