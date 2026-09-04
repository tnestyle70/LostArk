[CmdletBinding()]
param(
    [ValidateSet('Validate', 'ValidateDraft', 'SaveAuthoring', 'CommitCanonicalDraft', 'SourceManifest', 'PublishCandidate')]
    [string]$Mode = 'Validate',
    [string]$RepositoryRoot = '',
    [string]$CandidateRoot = 'Intermediate/ValtanTuningCandidates',
    [string]$AuthoringRoot = 'Intermediate/ValtanTuningAuthoring',
    [string]$AuthoringRevision = '',
    [string]$SourceManifestPath = '',
    [string]$DraftPatchPath = '',
    [string]$PatternSoundBaselinePath = '',
    [string]$PatternSoundCandidatePath = '',
    [string]$EffectV2BaselinePath = '',
    [string]$EffectV2CandidatePath = '',
    [string]$EffectV2ReadSetPath = '',
    [ValidateRange(0.0, 300.0)]
    [double]$LockTimeoutSeconds = 30.0,
    [ValidateSet('', 'after_stage', 'after_validate', 'after_revision_manifest',
        'before_promote', 'after_promote', 'before_pointer', 'after_pointer')]
    [string]$FailAt = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
}
else {
    [IO.Path]::GetFullPath($RepositoryRoot)
}
$pipeline = Join-Path $PSScriptRoot 'valtan_tuning_pipeline.py'
if (-not [IO.File]::Exists($pipeline)) {
    throw "Missing Valtan tuning pipeline: $pipeline"
}

$command = @($pipeline, '--repository-root', $repoRoot)
$resolvedAuthoringRoot = if ([IO.Path]::IsPathRooted($AuthoringRoot)) {
    [IO.Path]::GetFullPath($AuthoringRoot)
}
else {
    [IO.Path]::GetFullPath((Join-Path $repoRoot $AuthoringRoot))
}
$resolvedDraftPatch = ''
if (-not [string]::IsNullOrWhiteSpace($DraftPatchPath)) {
    $resolvedDraftPatch = if ([IO.Path]::IsPathRooted($DraftPatchPath)) {
        [IO.Path]::GetFullPath($DraftPatchPath)
    }
    else {
        [IO.Path]::GetFullPath((Join-Path $repoRoot $DraftPatchPath))
    }
}
function Resolve-OptionalPath([string]$Value) {
    if ([string]::IsNullOrWhiteSpace($Value)) {
        return ''
    }
    if ([IO.Path]::IsPathRooted($Value)) {
        return [IO.Path]::GetFullPath($Value)
    }
    return [IO.Path]::GetFullPath((Join-Path $repoRoot $Value))
}
$resolvedPatternSoundBaseline = Resolve-OptionalPath $PatternSoundBaselinePath
$resolvedPatternSoundCandidate = Resolve-OptionalPath $PatternSoundCandidatePath
$resolvedEffectV2Baseline = Resolve-OptionalPath $EffectV2BaselinePath
$resolvedEffectV2Candidate = Resolve-OptionalPath $EffectV2CandidatePath
$resolvedEffectV2ReadSet = Resolve-OptionalPath $EffectV2ReadSetPath
switch ($Mode) {
    'Validate' {
        $command += 'validate'
    }
    'ValidateDraft' {
        $command += @('validate-draft', '--authoring-root', $resolvedAuthoringRoot)
        if (-not [string]::IsNullOrWhiteSpace($resolvedDraftPatch)) {
            $command += @('--draft-patch', $resolvedDraftPatch)
        }
    }
    'SaveAuthoring' {
        $command += @('save-authoring', '--authoring-root', $resolvedAuthoringRoot)
        if (-not [string]::IsNullOrWhiteSpace($resolvedDraftPatch)) {
            $command += @('--draft-patch', $resolvedDraftPatch)
        }
        if (-not [string]::IsNullOrWhiteSpace($FailAt)) {
            $command += @('--fail-at', $FailAt)
        }
    }
    'CommitCanonicalDraft' {
        if ([string]::IsNullOrWhiteSpace($resolvedDraftPatch)) {
            throw 'CommitCanonicalDraft requires DraftPatchPath.'
        }
        if (([string]::IsNullOrWhiteSpace($resolvedPatternSoundBaseline)) -ne
            ([string]::IsNullOrWhiteSpace($resolvedPatternSoundCandidate))) {
            throw 'Pattern Sound baseline/candidate paths must be paired.'
        }
        if (([string]::IsNullOrWhiteSpace($resolvedEffectV2Baseline)) -ne
            ([string]::IsNullOrWhiteSpace($resolvedEffectV2Candidate))) {
            throw 'Effect V2 baseline/candidate paths must be paired.'
        }
        if (-not [string]::IsNullOrWhiteSpace($resolvedEffectV2ReadSet) -and
            [string]::IsNullOrWhiteSpace($resolvedEffectV2Baseline)) {
            throw 'Effect V2 read-set path requires a paired baseline/candidate owner.'
        }
        $command += @(
            'commit-canonical-draft',
            '--authoring-root', $resolvedAuthoringRoot,
            '--draft-patch', $resolvedDraftPatch,
            '--lock-timeout-seconds',
            $LockTimeoutSeconds.ToString(
                [Globalization.CultureInfo]::InvariantCulture)
        )
        if (-not [string]::IsNullOrWhiteSpace($resolvedPatternSoundBaseline)) {
            $command += @(
                '--pattern-sound-baseline', $resolvedPatternSoundBaseline,
                '--pattern-sound-candidate', $resolvedPatternSoundCandidate
            )
        }
        if (-not [string]::IsNullOrWhiteSpace($resolvedEffectV2Baseline)) {
            $command += @(
                '--effect-v2-baseline', $resolvedEffectV2Baseline,
                '--effect-v2-candidate', $resolvedEffectV2Candidate
            )
            if (-not [string]::IsNullOrWhiteSpace($resolvedEffectV2ReadSet)) {
                $command += @('--effect-v2-read-set', $resolvedEffectV2ReadSet)
            }
        }
    }
    'SourceManifest' {
        $command += @('source-manifest', '--authoring-root', $resolvedAuthoringRoot)
        if (-not [string]::IsNullOrWhiteSpace($SourceManifestPath)) {
            $manifestPath = if ([IO.Path]::IsPathRooted($SourceManifestPath)) {
                [IO.Path]::GetFullPath($SourceManifestPath)
            }
            else {
                [IO.Path]::GetFullPath((Join-Path $repoRoot $SourceManifestPath))
            }
            $command += @('--output', $manifestPath)
        }
    }
    'PublishCandidate' {
        $candidatePath = if ([IO.Path]::IsPathRooted($CandidateRoot)) {
            [IO.Path]::GetFullPath($CandidateRoot)
        }
        else {
            [IO.Path]::GetFullPath((Join-Path $repoRoot $CandidateRoot))
        }
        $command += @('publish-candidate', '--candidate-root', $candidatePath)
        if (-not [string]::IsNullOrWhiteSpace($AuthoringRevision) -or
            -not [string]::IsNullOrWhiteSpace($resolvedDraftPatch)) {
            $command += @('--authoring-root', $resolvedAuthoringRoot)
        }
        if (-not [string]::IsNullOrWhiteSpace($AuthoringRevision)) {
            $command += @('--authoring-revision', $AuthoringRevision)
        }
        if (-not [string]::IsNullOrWhiteSpace($SourceManifestPath)) {
            $manifestPath = if ([IO.Path]::IsPathRooted($SourceManifestPath)) {
                [IO.Path]::GetFullPath($SourceManifestPath)
            }
            else {
                [IO.Path]::GetFullPath((Join-Path $repoRoot $SourceManifestPath))
            }
            $command += @('--expected-source-manifest', $manifestPath)
        }
        if (-not [string]::IsNullOrWhiteSpace($resolvedDraftPatch)) {
            $command += @('--draft-patch', $resolvedDraftPatch)
        }
        if (-not [string]::IsNullOrWhiteSpace($FailAt)) {
            $command += @('--fail-at', $FailAt)
        }
    }
}

& python @command
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
if ($Mode -eq 'Validate') {
    foreach ($validatorName in @(
        'validate_valtan_clip_template_parity.py',
        'validate_valtan_hit_presentation_alignment.py')) {
        $validatorPath = Join-Path $PSScriptRoot $validatorName
        if (-not [IO.File]::Exists($validatorPath)) {
            throw "Missing Valtan product validator: $validatorPath"
        }
        & python $validatorPath --repository-root $repoRoot --check
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }
}
