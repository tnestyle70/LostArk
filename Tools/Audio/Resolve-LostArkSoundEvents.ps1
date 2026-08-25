#requires -Version 7.0

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$StageRoot,

    [Parameter(Mandatory = $true)]
    [string]$WwiserPath,

    [Parameter(Mandatory = $true)]
    [string]$NamesPath,

    [Parameter()]
    [string]$PythonPath = 'python',

    [Parameter()]
    [string]$OutputRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-AbsolutePath {
    param([Parameter(Mandatory = $true)][string]$Path)

    if ([System.IO.Path]::IsPathFullyQualified($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Resolve-Executable {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (Test-Path -LiteralPath $Path -PathType Leaf) {
        return Resolve-AbsolutePath $Path
    }
    $command = Get-Command -Name $Path -CommandType Application -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($null -eq $command) {
        throw "Executable was not found: $Path"
    }
    return $command.Source
}

function Assert-EmptyOrMissingDirectory {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        throw "Expected a directory path: $Path"
    }
    if (Get-ChildItem -LiteralPath $Path -Force | Select-Object -First 1) {
        throw "Event output directory must be empty or missing: $Path"
    }
}

function Get-PackageBanks {
    param(
        [Parameter(Mandatory = $true)][string]$BankRoot,
        [Parameter(Mandatory = $true)][string[]]$Packages
    )

    $banks = [System.Collections.Generic.List[string]]::new()
    foreach ($package in $Packages) {
        $packageRoot = Join-Path $BankRoot $package
        if (-not (Test-Path -LiteralPath $packageRoot -PathType Container)) {
            throw "Selected bank package directory is missing: $packageRoot"
        }
        foreach ($bank in Get-ChildItem -LiteralPath $packageRoot -File -Filter '*.bnk' | Sort-Object Name) {
            $banks.Add($bank.FullName)
        }
    }
    if ($banks.Count -eq 0) {
        throw "None of the selected packages contains a bank: $($Packages -join ', ')"
    }
    return @($banks)
}

function Get-ExactBanks {
    param(
        [Parameter(Mandatory = $true)][string]$BankRoot,
        [Parameter(Mandatory = $true)][uint64[]]$BankIds
    )

    $allBanks = @(Get-ChildItem -LiteralPath $BankRoot -Recurse -File -Filter '*.bnk')
    $result = [System.Collections.Generic.List[string]]::new()
    foreach ($bankId in $BankIds) {
        $matches = @($allBanks | Where-Object BaseName -CEQ ([string]$bankId))
        if ($matches.Count -ne 1) {
            throw "Expected exactly one staged bank $bankId, found $($matches.Count)."
        }
        $result.Add($matches[0].FullName)
    }
    return @($result)
}

$StageRoot = Resolve-AbsolutePath $StageRoot
$WwiserPath = Resolve-AbsolutePath $WwiserPath
$NamesPath = Resolve-AbsolutePath $NamesPath
$PythonPath = Resolve-Executable $PythonPath
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $StageRoot 'txtp_full'
}
$OutputRoot = Resolve-AbsolutePath $OutputRoot

foreach ($requiredFile in @($WwiserPath, $NamesPath, (Join-Path $StageRoot 'StageManifest.json'))) {
    if (-not (Test-Path -LiteralPath $requiredFile -PathType Leaf)) {
        throw "Required file does not exist: $requiredFile"
    }
}
$bankRoot = Join-Path $StageRoot 'banks'
$mediaRoot = Join-Path $StageRoot 'media'
foreach ($requiredDirectory in @($bankRoot, $mediaRoot)) {
    if (-not (Test-Path -LiteralPath $requiredDirectory -PathType Container)) {
        throw "Required stage directory does not exist: $requiredDirectory"
    }
}
Assert-EmptyOrMissingDirectory $OutputRoot
$null = [System.IO.Directory]::CreateDirectory($OutputRoot)

$groups = [System.Collections.Generic.List[object]]::new()
foreach ($definition in @(
    [pscustomobject]@{ Name = 'BgmLobbyCharacter'; Packages = @('SOUND_BGM_ALL') },
    [pscustomobject]@{ Name = 'BgmTrision'; Packages = @('SOUND_BGM_TRISION') },
    [pscustomobject]@{ Name = 'BgmValtan'; Packages = @('SOUND_BGM_COMMANDERRAID') },
    [pscustomobject]@{ Name = 'BgmBern'; Packages = @('SOUND_BGM_BERN') },
    [pscustomobject]@{ Name = 'LanceMaster'; Packages = @('SOUND_PC_LANCEMASTER') },
    [pscustomobject]@{ Name = 'Artist'; Packages = @('SOUND_PC_YINYANGSHI') },
    [pscustomobject]@{ Name = 'Warlord'; Packages = @('SOUND_PC_GUNLANCER') },
    [pscustomobject]@{ Name = 'DimensionMaster'; Packages = @('SOUND_PC_DIMENSIONMASTER') },
    [pscustomobject]@{
        Name = 'Common'
        Packages = @(
            'SOUND_PC_COMMON',
            'SOUND_PC_COMMON_NONSTREAM',
            'SOUND_PC_COMMON_HITTED',
            'SOUND_PC_COMMON_GLOBAL',
            'SOUND_PC_COMMON_GLOBAL_NONSTREAM'
        )
    }
)) {
    $groups.Add([pscustomobject]@{
        Name = $definition.Name
        Banks = @(Get-PackageBanks -BankRoot $bankRoot -Packages $definition.Packages)
    })
}

$groups.Add([pscustomobject]@{
    Name = 'Valtan'
    Banks = @(Get-ExactBanks -BankRoot $bankRoot -BankIds @(
        [uint64]1990761776, [uint64]1990761777, [uint64]1990761778
    ))
})
$uiGroups = [ordered]@{
    UiSystems = [uint64]2450321888
    UiInterface = [uint64]2573187115
    UiMinigame = [uint64]1891807024
    UiSystems1 = [uint64]2505345628
    UiCore = [uint64]2802418788
    UiItem = [uint64]4238991158
}
foreach ($pair in $uiGroups.GetEnumerator()) {
    $groups.Add([pscustomobject]@{
        Name = $pair.Key
        Banks = @(Get-ExactBanks -BankRoot $bankRoot -BankIds @([uint64]$pair.Value))
    })
}

$summary = [System.Collections.Generic.List[object]]::new()
$groupNumber = 0
foreach ($group in $groups) {
    $groupNumber++
    $groupRoot = Join-Path $OutputRoot $group.Name
    $null = [System.IO.Directory]::CreateDirectory($groupRoot)
    Write-Host ("[{0}/{1}] Resolving {2} from {3} bank(s)" -f
        $groupNumber, $groups.Count, $group.Name, $group.Banks.Count)

    $arguments = @(
        $WwiserPath,
        '-d', 'none',
        '-nl', $NamesPath,
        '-g', '-gd', '-gra',
        '-go', $groupRoot,
        '-gw', $mediaRoot,
        '-te', '-gxnl'
    ) + @($group.Banks)
    $wwiserOutput = & $PythonPath @arguments 2>&1
    $exitCode = $LASTEXITCODE
    [System.IO.File]::WriteAllLines(
        (Join-Path $groupRoot 'wwiser.log'),
        [string[]]@($wwiserOutput),
        [System.Text.UTF8Encoding]::new($false)
    )
    if ($exitCode -ne 0) {
        throw "wwiser failed for group '$($group.Name)' with exit code $exitCode."
    }

    $tagPath = Join-Path $groupRoot '!tags.m3u'
    if (-not (Test-Path -LiteralPath $tagPath -PathType Leaf)) {
        throw "wwiser did not produce !tags.m3u for group '$($group.Name)'."
    }
    $txtpFiles = @(Get-ChildItem -LiteralPath $groupRoot -File -Filter '*.txtp')
    if ($txtpFiles.Count -eq 0) {
        throw "wwiser produced no TXTP files for group '$($group.Name)'."
    }
    $unresolvedReferences = 0
    foreach ($txtp in $txtpFiles) {
        $unresolvedReferences += ([regex]::Matches(
            [System.IO.File]::ReadAllText($txtp.FullName),
            '(?i)\?\d+\.wem'
        )).Count
    }
    $summary.Add([ordered]@{
        group = $group.Name
        bankCount = $group.Banks.Count
        txtpCount = $txtpFiles.Count
        unresolvedMediaReferencesToRewriteAtPublish = $unresolvedReferences
    })
}

$summaryPath = Join-Path $OutputRoot 'EventResolutionSummary.json'
[System.IO.File]::WriteAllText(
    $summaryPath,
    (($summary | ConvertTo-Json -Depth 6) + [Environment]::NewLine),
    [System.Text.UTF8Encoding]::new($false)
)
Write-Host 'Lost Ark Wwise event resolution complete.'
Write-Host "  Output:  $OutputRoot"
Write-Host "  Groups:  $($summary.Count)"
$totalTxtp = 0
foreach ($item in $summary) {
    $totalTxtp += [int]$item['txtpCount']
}
Write-Host "  TXTP:    $totalTxtp"
Write-Host "  Summary: $summaryPath"
