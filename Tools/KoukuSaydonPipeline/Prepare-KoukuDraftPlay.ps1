[CmdletBinding(DefaultParameterSetName='Pattern')]
param(
    [string]$RepoRoot = '',
    [Parameter(Mandatory=$true)][string]$SourcePath,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [Parameter(Mandatory=$true,ParameterSetName='Pattern')][string]$PatternId,
    [Parameter(Mandatory=$true,ParameterSetName='Bundle')][string]$BundleId
)
$ErrorActionPreference = 'Stop'
$repo = if ($RepoRoot) { [IO.Path]::GetFullPath($RepoRoot) } else { [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..')) }
$output = [IO.Path]::GetFullPath($OutputDirectory)
$outRoot = [IO.Path]::GetFullPath((Join-Path $repo 'out')) + [IO.Path]::DirectorySeparatorChar
if (-not $output.StartsWith($outRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Draft artifacts require a request directory under repository/out.'
}
$admissionPath = Join-Path $output 'admission.json'
$arguments = @('-B', (Join-Path $PSScriptRoot 'prepare_kouku_draft_play.py'),
    '--repository-root', $repo, '--source-path', $SourcePath, '--output-directory', $output)
if ($PSCmdlet.ParameterSetName -eq 'Pattern') { $arguments += @('--pattern-id', $PatternId) }
else { $arguments += @('--bundle-id', $BundleId) }
$projectionReport = & python @arguments
if ($LASTEXITCODE -ne 0) { throw "Draft projection failed with exit code $LASTEXITCODE." }

. (Join-Path $PSScriptRoot 'KoukuBootstrapRows.ps1')
$encounter = [IO.File]::ReadAllText((Join-Path $output 'encounter.json'), [Text.Encoding]::UTF8) | ConvertFrom-Json
$presentation = [IO.File]::ReadAllText((Join-Path $output 'presentation.json'), [Text.Encoding]::UTF8) | ConvertFrom-Json
$bosses = [IO.File]::ReadAllText((Join-Path $repo 'Data/Balance/BossProfiles.json'), [Text.Encoding]::UTF8) | ConvertFrom-Json
$rows = [Collections.Generic.List[string]]::new()
Add-KoukuBootstrapRows -Encounter $encounter -Presentation $presentation -BossProfiles $bosses -Rows $rows
$sorted = @($rows | Sort-Object -Property @{ Expression = { Get-BootstrapRowSortKey -Row $_ } })
$bytes = [Text.UTF8Encoding]::new($false).GetBytes(($sorted -join "`n") + "`n")
if ($sorted.Count -eq 0 -or $bytes.Length -gt 16 * 1024 * 1024) {
    throw 'Draft gameplay rows require a nonempty payload of at most 16 MiB.'
}
function Write-DraftArtifact([string]$Name, [byte[]]$Bytes) {
    $destination = Join-Path $output $Name
    $temporary = $destination + '.tmp'
    [IO.File]::WriteAllBytes($temporary, $Bytes)
    if ([IO.File]::Exists($destination)) { [IO.File]::Replace($temporary, $destination, $null) }
    else { [IO.File]::Move($temporary, $destination) }
}
Write-DraftArtifact 'gameplay.rows' $bytes
$sha = [Security.Cryptography.SHA256]::Create()
try { $hash = ([BitConverter]::ToString($sha.ComputeHash($bytes))).Replace('-', '').ToLowerInvariant() }
finally { $sha.Dispose() }
$metadata = [IO.File]::ReadAllText((Join-Path $output 'projection.json'), [Text.Encoding]::UTF8) | ConvertFrom-Json
$metadata | Add-Member -NotePropertyName rowsSha256 -NotePropertyValue $hash
$metadata | Add-Member -NotePropertyName rowsBytes -NotePropertyValue $bytes.Length
$metadata | Add-Member -NotePropertyName rowsCount -NotePropertyValue $sorted.Count
$json = ($metadata | ConvertTo-Json -Depth 32) + "`n"
Write-DraftArtifact 'admission.json' ([Text.UTF8Encoding]::new($false).GetBytes($json))
Write-Output $json
