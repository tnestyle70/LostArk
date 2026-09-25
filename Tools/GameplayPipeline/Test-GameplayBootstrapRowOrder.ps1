[CmdletBinding()]
param()

# Exercise the actual publisher sorter without publishing or starting the Server.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$publisher = Join-Path $PSScriptRoot '../KoukuSaydonPipeline/KoukuBootstrapRows.ps1'
$tokens = $null
$parseErrors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile(
    $publisher, [ref]$tokens, [ref]$parseErrors)
if ($parseErrors.Count) { throw ($parseErrors | Out-String) }
$functions = @($ast.FindAll({
    param($node)
    $node -is [Management.Automation.Language.FunctionDefinitionAst] -and
        $node.Name -ceq 'Get-BootstrapRowSortKey'
}, $true))
if ($functions.Count -ne 1) { throw 'Expected one actual publisher sort function.' }
Invoke-Expression $functions[0].Extent.Text

function Assert-Condition {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) { throw $Message }
}

function Get-RowOwner {
    param([string[]]$Fields)
    return $Fields[1..3] -join "`t"
}

function Assert-ShowtimeOrder {
    param([string[]]$Rows, [hashtable]$ExpectedCounts)
    $seen = @{}
    foreach ($row in $Rows) {
        $fields = $row.Split("`t")
        if ($fields[0] -cnotin @('PATTERNSHOWTIMETARGETS', 'PATTERNSHOWTIMERANDOM')) {
            continue
        }
        $owner = Get-RowOwner $fields
        if ($fields[0] -ceq 'PATTERNSHOWTIMETARGETS') {
            Assert-Condition (-not $seen.ContainsKey($owner)) "Repeated Showtime parent: $owner"
            $seen[$owner] = 0
        } else {
            Assert-Condition ($seen.ContainsKey($owner)) "Showtime child before its parent: $owner"
            Assert-Condition ([int]$fields[4] -eq $seen[$owner]) "Showtime ordinal is not dense: $owner"
            $seen[$owner]++
        }
    }
    Assert-Condition ($seen.Count -eq $ExpectedCounts.Count) 'Showtime owner count changed.'
    foreach ($owner in $ExpectedCounts.Keys) {
        Assert-Condition ($seen.ContainsKey($owner) -and $seen[$owner] -eq $ExpectedCounts[$owner]) `
            "Showtime child count changed: $owner"
    }
}

function Assert-ExistingDependencyOrder {
    param([string[]]$Rows)
    $worlds = @{}
    $outcomes = @{}
    $stages = @{}
    $worldChildren = 0
    $logicChildren = 0
    foreach ($row in $Rows) {
        $fields = $row.Split("`t")
        switch -CaseSensitive ($fields[0]) {
            'PATTERNWORLDSEQUENCE' {
                $owner = @($fields[1], $fields[2], $fields[14]) -join "`t"
                Assert-Condition (-not $worlds.ContainsKey($owner)) 'Repeated World occurrence.'
                $worlds[$owner] = $true
            }
            { $_ -cin @('PATTERNWORLDPLACEMENT', 'PATTERNWORLDSUPPORT') } {
                $owner = Get-RowOwner $fields
                Assert-Condition ($worlds.ContainsKey($owner)) "World child before its parent: $owner"
                $worldChildren++
            }
            'PATTERNLOGICOUTCOME' {
                $owner = $fields[1..4] -join "`t"
                if (-not $outcomes.ContainsKey($owner)) { $outcomes[$owner] = 0 }
                Assert-Condition ([int]$fields[5] -eq $outcomes[$owner]) 'Logic outcome ordinal is not dense.'
                $outcomes[$owner]++
            }
            { $_ -cin @('PATTERNLOGICCONTACTMOTION', 'PATTERNLOGICSIGNAL') } {
                $owner = $fields[1..4] -join "`t"
                Assert-Condition ($outcomes.ContainsKey($owner) -and
                    [int]$fields[5] -lt $outcomes[$owner]) 'Logic child before its outcome slot.'
                $logicChildren++
            }
            { $_ -cin @('PATTERNSTAGEACTION', 'PATTERNSTAGEVOLLEY') } {
                $owner = Get-RowOwner $fields
                if (-not $stages.ContainsKey($owner)) { $stages[$owner] = 0 }
                Assert-Condition ([int]$fields[4] -eq $stages[$owner]) 'Mixed Stage action ordinal is not dense.'
                $stages[$owner]++
            }
        }
    }
    Assert-Condition ($worlds.Count -eq 4 -and $worldChildren -eq 4) 'World fixture coverage changed.'
    Assert-Condition ($outcomes.Count -eq 4 -and $logicChildren -eq 32) 'Logic fixture coverage changed.'
    foreach ($count in $outcomes.Values) { Assert-Condition ($count -eq 4) 'Logic outcomes are incomplete.' }
    Assert-Condition ($stages.Count -eq 4) 'Stage fixture owner count changed.'
    foreach ($count in $stages.Values) { Assert-Condition ($count -eq 32) 'Stage actions are incomplete.' }
}

function Assert-ParentChildOrder {
    param([string[]]$Rows)
    $ends = @{}
    $seen = @{}
    $lastOwner = ''
    foreach ($row in $Rows) {
        $fields = $row.Split("`t")
        if ($fields[0] -cne 'PATTERNPARENTCHILD') { continue }
        $owner = $fields[1..2] -join "`t"
        if (-not $ends.ContainsKey($owner)) {
            $ends[$owner] = [uint64]0
            $seen[$owner] = 0
        } elseif ($lastOwner -cne $owner) {
            throw "Parent child owners are interleaved: $owner"
        }
        Assert-Condition ([uint64]$fields[5] -ge $ends[$owner]) `
            "Parent child time order is invalid: $owner"
        $ends[$owner] = [uint64]$fields[5] + [uint64]$fields[6]
        $seen[$owner]++
        $lastOwner = $owner
    }
    Assert-Condition ($seen.Count -eq 3) 'Parent child owner coverage changed.'
    foreach ($count in $seen.Values) {
        Assert-Condition ($count -eq 3) 'Parent child count changed.'
    }
}

function Assert-RowBytesPreserved {
    param([string[]]$Before, [string[]]$After)
    # Base64 keys compare every UTF-8 byte, including tabs, numeric spellings and
    # repeated rows; the sort key must never replace the original emitted row.
    $counts = [Collections.Generic.Dictionary[string, int]]::new([StringComparer]::Ordinal)
    foreach ($row in $Before) {
        $key = [Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes($row))
        if (-not $counts.ContainsKey($key)) { $counts[$key] = 0 }
        $counts[$key]++
    }
    Assert-Condition ($Before.Count -eq $After.Count) 'The sorter changed the row count.'
    foreach ($row in $After) {
        $key = [Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes($row))
        Assert-Condition ($counts.ContainsKey($key) -and $counts[$key] -gt 0) 'The sorter changed row bytes.'
        $counts[$key]--
    }
    foreach ($count in $counts.Values) { Assert-Condition ($count -eq 0) 'The sorter dropped a row.' }
}

$rows = [Collections.Generic.List[string]]::new()
$showtimeCounts = @{}
# Reuse each lower-level identity under different higher-level owners, so a
# key that drops encounter, pattern or occurrence cannot pass accidentally.
$owners = @(
    @('encounter.2', 'pattern.2', 'trigger.2'),
    @('encounter.2', 'pattern.2', 'trigger.10'),
    @('encounter.2', 'pattern.10', 'trigger.2'),
    @('encounter.10', 'pattern.2', 'trigger.2')
)
foreach ($owner in $owners) {
    $ownerKey = $owner -join "`t"
    $showtimeCounts[$ownerKey] = 32
    $rows.Add((@('PATTERNSHOWTIMETARGETS') + $owner +
        @('41315', '17071', 'visual.fixed', 'visual.tracking', '4384', '2000', '0.5')) -join "`t")
    foreach ($ordinal in 0..31) {
        $rows.Add((@('PATTERNSHOWTIMERANDOM') + $owner +
            @($ordinal, ('visual.random.' + ($ordinal % 2)), '4853', '500', '16.000', '0.100')) -join "`t")
    }
}
foreach ($trigger in @('only.2', 'only.10')) {
    $owner = @('encounter.only', 'pattern.only', $trigger)
    $showtimeCounts[($owner -join "`t")] = 0
    $rows.Add((@('PATTERNSHOWTIMETARGETS') + $owner +
        @('0', '1000', 'visual.fixed', 'visual.tracking', '1000', '500', '1.0')) -join "`t")
}

# These are ordering fixtures, not complete independently loadable catalogs.
# Preserve the indexed/dependent row widths and the consumer-owned key fields.
$index = 0
foreach ($owner in $owners) {
    $world = 'world.' + $index
    $rows.Add((@('PATTERNWORLDSEQUENCE', $owner[0], $owner[1], '0', 'instance.1', '1',
        '0', '0', '0', 'NONE', '0', '0', '0', '1000', $world)) -join "`t")
    if ($index % 2 -eq 0) {
        $rows.Add((@('PATTERNWORLDPLACEMENT', $owner[0], $owner[1], $world,
            '0', '0', '0', '0', '0', '0', '1', '1', '1')) -join "`t")
    } else {
        $rows.Add((@('PATTERNWORLDSUPPORT', $owner[0], $owner[1], $world,
            '0', '30', '0', '0', '0', '1')) -join "`t")
    }
    foreach ($ordinal in 0..3) {
        $rows.Add((@('PATTERNLOGICOUTCOME') + $owner +
            @('SUCCESS', $ordinal, 'PLAY_CONTACT_WORLD_OBJECT_MOTION', '0', '0', '-')) -join "`t")
        $rows.Add((@('PATTERNLOGICCONTACTMOTION') + $owner +
            @('SUCCESS', $ordinal, 'target.1', 'motion.1')) -join "`t")
        $rows.Add((@('PATTERNLOGICSIGNAL') + $owner +
            @('SUCCESS', $ordinal, 'completion.1', 'signal.1')) -join "`t")
    }
    foreach ($ordinal in 0..31) {
        if ($ordinal % 2 -eq 0) {
            $rows.Add((@('PATTERNSTAGEACTION') + $owner +
                @($ordinal, 'ENTER', 'PLAY_ANIMATION', 'animation.1', '0', '1000')) -join "`t")
        } else {
            $rows.Add((@('PATTERNSTAGEVOLLEY') + $owner +
                @($ordinal, 'ENTER', 'object.1', 'ARENA_CENTER', '1', 'RING', '1',
                    '0', '0', '0', '4', '1', '0', '0', '0', '0', '0', '0')) -join "`t")
        }
    }
    $index++
}
# Stable IDs are identities, not chronology: a later edit can insert .44/.45
# before .30. Numeric times 9/10/100 also reject lexicographic time ordering.
foreach ($owner in @(
    @('encounter.2', 'pattern.2'),
    @('encounter.2', 'pattern.10'),
    @('encounter.10', 'pattern.2')
)) {
    $rows.Add((@('PATTERNPARENTCHILD') + $owner +
        @('child.30', 'target.2', '100', '1', '1')) -join "`t")
    $rows.Add((@('PATTERNPARENTCHILD') + $owner +
        @('child.45', 'target.10', '10', '90', '0')) -join "`t")
    $rows.Add((@('PATTERNPARENTCHILD') + $owner +
        @('child.44', 'target.2', '9', '1', '0')) -join "`t")
}
# Repeated uninterpreted rows also retain their original bytes and multiplicity.
$rows.Add("ORDER_FIXTURE`t0002`t-0.000`t1e-05")
$rows.Add("ORDER_FIXTURE`t0002`t-0.000`t1e-05")

$before = $rows.ToArray()
foreach ($seed in @(7, 19, 20260915)) {
    $shuffled = [string[]]$before.Clone()
    $random = [Random]::new($seed)
    for ($index = $shuffled.Length - 1; $index -gt 0; $index--) {
        $other = $random.Next($index + 1)
        $saved = $shuffled[$index]
        $shuffled[$index] = $shuffled[$other]
        $shuffled[$other] = $saved
    }
    $sorted = @($shuffled | Sort-Object -Property @{
        Expression = { Get-BootstrapRowSortKey -Row $_ }
    })
    Assert-ShowtimeOrder $sorted $showtimeCounts
    Assert-ExistingDependencyOrder $sorted
    Assert-ParentChildOrder $sorted
    Assert-RowBytesPreserved $before $sorted
}

# Confirm this suite detects the original RANDOM-before-TARGETS regression.
$caughtOriginalFailure = $false
try { Assert-ShowtimeOrder @($before | Sort-Object) $showtimeCounts }
catch {
    if ($_.Exception.Message -notlike 'Showtime child before its parent:*') { throw }
    $caughtOriginalFailure = $true
}
Assert-Condition $caughtOriginalFailure 'The original parent-order regression was not detected.'
$caughtParentTimeFailure = $false
try { Assert-ParentChildOrder @($before | Sort-Object) }
catch {
    if ($_.Exception.Message -notlike 'Parent child time order is invalid:*') { throw }
    $caughtParentTimeFailure = $true
}
Assert-Condition $caughtParentTimeFailure 'The ID-before-time regression was not detected.'
Write-Output (("PASS: actual publisher sort; {0} rows x 3 shuffles; 6 Showtime owners, " +
    "128 random volleys, targets-only, byte preservation, World/Logic/Stage dependencies, 3 Parent child owners; " +
    'original-order and Parent ID-before-time negative controls rejected.') -f $before.Count)
