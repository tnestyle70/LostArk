[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$DraftPath)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
. (Join-Path $PSScriptRoot 'Publish-FileTransaction.ps1')
Import-Module (Join-Path $repoRoot 'Tools/ValtanPipeline/ValtanCanonicalWriterAdmission.psm1') -Force

# Stable identity and an explicit scalar allowlist preserve every unedited field,
# including newer schema fields that this small panel does not understand.
$domains = @{
    'Data/Balance/PlayerProfiles.json' = @('players', 'characterClass', @(
        'maximumHp','maximumResource','resourceRegenPerSecond','attackPower','defense',
        'moveSpeed','defenseStanceMoveSpeedScale','maximumIdentity','identityRegenPerSecond',
        'identityDrainPerSecond','identityStanceSwitchCost'))
    'Data/Balance/PlayerSkills.json' = @('skills', 'skillId', @(
        'cooldownMs','resourceCost','identityCost','staggerDamage','partDamage',
        'actionDurationMs','hitTimeMs','movementDistance','maximumRange'))
    'Data/Balance/DamageProfiles.json' = @('profiles', 'damageProfileId', @('damageRatePercent'))
    'Data/Balance/BossProfiles.json' = @('bosses', 'archetypeId', @(
        'maximumHp','maximumHealthBars','attackPower','collisionRadius','engageDistance','moveSpeed'))
}
$profilePath = 'Data/Balance/Profiles/Retail.balanceprofile.json'
$profileDomains = @{
    'players' = @('players', 'characterClass', @('maximumHp','maximumResource','resourceRegenPerSecond','attackPower','defense','criticalChancePercent','criticalDamagePercent'))
    'skills' = @('skills', 'skillId', @('cooldownMs','resourceCost','staggerDamage','partDamage'))
    'damageProfiles' = @('damageProfiles', 'damageProfileId', @('attackCoefficientBp','damageAddend','damageSpreadPercent'))
    'bosses' = @('bosses', 'archetypeId', @('maximumHp','maximumHealthBars','attackPower'))
}
$draft = Get-Content -LiteralPath $DraftPath -Raw -Encoding UTF8 | ConvertFrom-Json
if ($draft.schema -cne 'lostark.balance-test-draft' -or $draft.formatVersion -ne 1 -or
    @($draft.changes).Count -eq 0 -or @($draft.changes).Count -gt 4096) {
    throw 'Invalid or empty Balance Test draft.'
}
$id = [Guid]::NewGuid().ToString('N')
$stage = Join-Path $repoRoot "Intermediate/BalanceTest/$id"
[IO.Directory]::CreateDirectory($stage) | Out-Null
$sources = @{}
$documents = @{}
$changed = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$fields = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
$promoted = [Collections.Generic.List[object]]::new()
$locks = [Collections.Generic.List[Threading.Mutex]]::new()
$admission = $null
try {
    $admission = Enter-ValtanCanonicalWriterAdmission -RepositoryRoot $repoRoot -TimeoutSeconds 30.0
    # Keep the complete balance input set stable while the publisher validates.
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repoRoot 'Data/Balance') -File -Recurse -Filter '*.json') {
        $null = Read-PublishJsonSnapshot $file.FullName $sources
    }
    foreach ($change in @($draft.changes)) {
        $path = [string]$change.document
        if ($path -ceq $profilePath) {
            if (-not $profileDomains.ContainsKey([string]$change.domain)) { throw 'Unsupported Retail profile domain.' }
            $domain = $profileDomains[[string]$change.domain]
        }
        else {
            if (-not $domains.ContainsKey($path)) { throw "Unsupported balance document: $path" }
            $domain = $domains[$path]
        }
        if ([string]$change.field -cnotin $domain[2] -or
            $null -eq $change.before -or $null -eq $change.value -or
            $change.before -is [string] -or $change.value -is [string] -or
            $change.before -is [bool] -or $change.value -is [bool] -or
            [double]::IsNaN([double]$change.value) -or [double]::IsInfinity([double]$change.value)) {
            throw "Unsupported numeric field: $path/$($change.field)"
        }
        if ([string]$change.field -cnotin @('moveSpeed','defenseStanceMoveSpeedScale','movementDistance','maximumRange','collisionRadius','engageDistance') -and
            [math]::Floor([double]$change.value) -ne [double]$change.value) { throw 'A whole number is required.' }
        if ([double]$change.value -lt 0) { throw 'Negative balance values are not supported.' }
        if (-not $fields.Add("$path#$($domain[0])/$($change.id).$($change.field)")) { throw 'Duplicate draft field.' }
        if (-not $documents.ContainsKey($path)) {
            $documents[$path] = Read-PublishJsonSnapshot (Join-Path $repoRoot $path) $sources
        }
        $document = $documents[$path]
        # A stale base draft must not silently save a field now owned by Retail.
        if ($path -cne $profilePath -and (Test-Path -LiteralPath (Join-Path $repoRoot $profilePath))) {
            if (-not $documents.ContainsKey($profilePath)) {
                $documents[$profilePath] = Read-PublishJsonSnapshot (Join-Path $repoRoot $profilePath) $sources
            }
            $profileArray = if ($domain[0] -ceq 'profiles') { 'damageProfiles' } else { $domain[0] }
            if ($profileDomains.ContainsKey($profileArray) -and [string]$change.field -cin $profileDomains[$profileArray][2]) {
                $owners = @($documents[$profilePath].($profileArray) | Where-Object {
                    [string]$_.($domain[1]) -ceq [string]$change.id -and $_.PSObject.Properties.Name -ccontains [string]$change.field })
                if ($owners.Count -gt 0) { throw "CONFLICT: Retail now owns $path/$($change.id).$($change.field). Reload the effective field." }
            }
        }
        $rows = @($document.($domain[0]) | Where-Object { [string]$_.($domain[1]) -ceq [string]$change.id })
        if ($rows.Count -ne 1 -or $rows[0].PSObject.Properties.Name -cnotcontains [string]$change.field) {
            throw "Missing or ambiguous stable row: $path/$($change.id)"
        }
        if ([double]$rows[0].([string]$change.field) -ne [double]$change.before) {
            throw "CONFLICT: $path/$($change.id).$($change.field) changed on disk. Reload or reconcile the saved value."
        }
        $rows[0].([string]$change.field) = $change.value
        [void]$changed.Add($path)
    }
    foreach ($path in $changed) {
        $candidate = Join-Path $stage $path
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($candidate)) | Out-Null
        [IO.File]::WriteAllText($candidate, ($documents[$path] | ConvertTo-Json -Depth 64) + "`n", [Text.UTF8Encoding]::new($false))
    }
    & (Join-Path $PSScriptRoot 'Update-BalanceProvenanceReceipt.ps1') -InputOverlayRoot $stage
    & (Join-Path $PSScriptRoot 'Publish-GameplayBalance.ps1') -Mode Validate -InputOverlayRoot $stage -BalanceProfile Retail
    $receipt = 'Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json'
    [void]$changed.Add($receipt)
    foreach ($path in @($changed | Sort-Object)) {
        $locks.Add((Enter-PublishDestinationMutex (Get-PublishDestinationMutexName (Join-Path $repoRoot $path))))
    }
    Assert-PublishSourceSnapshots $sources
    foreach ($path in @($changed | Sort-Object)) {
        $destination = Join-Path $repoRoot $path
        $candidate = Join-Path $stage $path
        $backup = Join-Path $stage ("Backup/" + $path)
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($backup)) | Out-Null
        $hash = Get-PublishFileSha256 $candidate
        $candidateBytes = [Convert]::ToBase64String([IO.File]::ReadAllBytes($candidate))
        # Recheck remaining inputs before every promotion. Our earlier writes
        # replace only their own snapshot identity for the next check.
        Assert-PublishSourceSnapshots $sources
        [IO.File]::Replace($candidate, $destination, $backup)
        $promoted.Add([pscustomobject]@{ path = $destination; backup = $backup; hash = $hash })
        # Pin exactly our staged bytes. Reading the destination here could
        # accidentally adopt an intervening external save as our own write.
        $sources[$destination] = $candidateBytes
    }
    Assert-PublishSourceSnapshots $sources
    Write-Output "SAVED: $($draft.changes.Count) numeric fields and provenance. Publish Server Data, then restart Server and Client to use the saved balance."
}
catch {
    $failure = $_
    for ($index = $promoted.Count - 1; $index -ge 0; --$index) {
        $entry = $promoted[$index]
        try {
            if (Test-PublishFileHash $entry.path $entry.hash 'Balance Test rollback') {
                [IO.File]::Replace($entry.backup, $entry.path, ($entry.backup + '.discarded'))
            }
            else { Write-Warning "Concurrent edit preserved; recover prior bytes from $($entry.backup)" }
        }
        catch { Write-Warning "Rollback incomplete; retained $($entry.backup): $($_.Exception.Message)" }
    }
    throw $failure
}
finally {
    foreach ($mutex in $locks) { Close-PublishDestinationMutex $mutex 'Balance Test' }
    if ($null -ne $admission) { Exit-ValtanCanonicalWriterAdmission -Admission $admission }
}
