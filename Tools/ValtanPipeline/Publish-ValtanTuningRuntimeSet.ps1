[CmdletBinding()]
param(
    [ValidateSet('Validate', 'ValidateDraft', 'SaveAuthoring', 'SourceManifest', 'PublishCandidate')]
    [string]$Mode = 'Validate',
    [string]$RepositoryRoot = '',
    [string]$CandidateRoot = 'Intermediate/ValtanTuningCandidates',
    [string]$AuthoringRoot = 'Intermediate/ValtanTuningAuthoring',
    [string]$AuthoringRevision = '',
    [string]$SourceManifestPath = '',
    [string]$DraftPatchPath = '',
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
