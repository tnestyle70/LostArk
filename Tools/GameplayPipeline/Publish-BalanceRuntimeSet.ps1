[CmdletBinding()]
param(
    [ValidateSet('Validate', 'Publish')]
    [string]$Mode = 'Validate',
    [string]$OutputRoot = 'Server/Bin/DataFiles',
    [ValidateRange(0, 6)]
    [int]$FailureAfterPromote = 0
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$gameplayPublisher = Join-Path $PSScriptRoot 'Publish-GameplayBalance.ps1'
$worldPublisher = Join-Path $repoRoot 'Tools\WorldPipeline\Publish-WorldGameplay.ps1'
$itemPublisher = Join-Path $PSScriptRoot 'Publish-ItemCatalog.ps1'

& $gameplayPublisher -Mode Validate
& $worldPublisher -Mode Validate
& $itemPublisher -Mode Validate
if ($Mode -eq 'Validate') {
    Write-Output 'Balance runtime set Validate succeeded.'
    return
}

$transactionId = [Guid]::NewGuid().ToString('N')
if ([IO.Path]::IsPathRooted($OutputRoot)) {
    throw 'Balance runtime set OutputRoot must be repository-relative.'
}
$runtimeRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot $OutputRoot))
$repoPrefix = $repoRoot.TrimEnd('\') + '\'
if (-not $runtimeRoot.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Balance runtime set OutputRoot escaped the repository.'
}
$stagingRelative = Join-Path $OutputRoot ".balance-runtime-set.staging.$transactionId"
$stagingRoot = Join-Path $repoRoot $stagingRelative
$stagedGameplayRoot = Join-Path $stagingRoot 'Gameplay'
$stagedWorldRoot = Join-Path $stagingRoot 'World'
$stagedItemsRoot = Join-Path $stagingRoot 'Items'
$promotions = [Collections.Generic.List[object]]::new()

try {
    [IO.Directory]::CreateDirectory($stagedGameplayRoot) | Out-Null
    [IO.Directory]::CreateDirectory($stagedWorldRoot) | Out-Null
    [IO.Directory]::CreateDirectory($stagedItemsRoot) | Out-Null
    & $gameplayPublisher -Mode Publish -OutputRoot (Join-Path $stagingRelative 'Gameplay')
    & $worldPublisher -Mode Publish -OutputRoot (Join-Path $stagingRelative 'World')
    & $itemPublisher -Mode Publish -OutputRoot (Join-Path $stagingRelative 'Items')

    $targets = @(
        @{ Staged = Join-Path $stagedGameplayRoot 'Gameplay.bootstrap'; Destination = Join-Path $runtimeRoot 'Gameplay\Gameplay.bootstrap' },
        @{ Staged = Join-Path $stagedWorldRoot 'BERN.worldbootstrap'; Destination = Join-Path $runtimeRoot 'World\BERN.worldbootstrap' },
        @{ Staged = Join-Path $stagedWorldRoot 'VALTAN_ARENA.worldbootstrap'; Destination = Join-Path $runtimeRoot 'World\VALTAN_ARENA.worldbootstrap' },
        @{ Staged = Join-Path $stagedWorldRoot 'TRAINING_GROUND.worldbootstrap'; Destination = Join-Path $runtimeRoot 'World\TRAINING_GROUND.worldbootstrap' },
        @{ Staged = Join-Path $stagedWorldRoot 'CHARACTER_SELECT_ARENA.worldbootstrap'; Destination = Join-Path $runtimeRoot 'World\CHARACTER_SELECT_ARENA.worldbootstrap' },
        @{ Staged = Join-Path $stagedItemsRoot 'Items.bootstrap'; Destination = Join-Path $runtimeRoot 'Items\Items.bootstrap' }
    )
    foreach ($target in $targets) {
        if (-not [IO.File]::Exists($target.Staged)) {
            throw "Balance runtime staged output is missing: $($target.Staged)"
        }
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($target.Destination)) | Out-Null
        $promotions.Add([ordered]@{
            Staged = $target.Staged
            Destination = $target.Destination
            Rollback = "$($target.Destination).rollback.$transactionId"
            HadPrevious = $false
            Promoted = $false
        })
    }

    $promotedCount = 0
    foreach ($promotion in $promotions) {
        if ([IO.File]::Exists($promotion.Destination)) {
            [IO.File]::Move($promotion.Destination, $promotion.Rollback)
            $promotion.HadPrevious = $true
        }
        [IO.File]::Move($promotion.Staged, $promotion.Destination)
        $promotion.Promoted = $true
        $promotedCount++
        if ($FailureAfterPromote -eq $promotedCount) {
            throw "Injected balance runtime set failure after promotion $promotedCount."
        }
    }

    foreach ($promotion in $promotions) {
        if ([IO.File]::Exists($promotion.Rollback)) {
            [IO.File]::Delete($promotion.Rollback)
        }
    }
    Write-Output 'Balance runtime set Publish succeeded: gameplay + 4 worlds + items.'
}
catch {
    $publishFailure = $_
	$rollbackFailures = [Collections.Generic.List[string]]::new()
    for ($index = $promotions.Count - 1; $index -ge 0; --$index) {
        $promotion = $promotions[$index]
		try {
			if ($promotion.Promoted -and [IO.File]::Exists($promotion.Destination)) {
				[IO.File]::Delete($promotion.Destination)
			}
			if ($promotion.HadPrevious) {
				if (-not [IO.File]::Exists($promotion.Rollback)) {
					throw "Rollback backup is missing: $($promotion.Rollback)"
				}
				[IO.File]::Move($promotion.Rollback, $promotion.Destination)
			}
		}
		catch {
			$rollbackFailures.Add("$($promotion.Destination): $($_.Exception.Message)")
		}
    }
	if ($rollbackFailures.Count -ne 0) {
		$preserved = @($promotions | Where-Object { [IO.File]::Exists($_.Rollback) } |
			ForEach-Object Rollback)
		throw "Balance runtime publish failed: $($publishFailure.Exception.Message) Rollback recovery was incomplete. Preserved backups=[$($preserved -join ',')]. Failures=[$($rollbackFailures -join '; ')]"
	}
    throw $publishFailure
}
finally {
    if ([IO.Directory]::Exists($stagingRoot)) {
        Remove-Item -LiteralPath $stagingRoot -Recurse -Force
    }
}
