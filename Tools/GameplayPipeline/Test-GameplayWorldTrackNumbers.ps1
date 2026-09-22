[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$tokens = $null
$errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile(
    (Join-Path $PSScriptRoot 'Publish-GameplayBalance.ps1'), [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw ($errors | Out-String) }
foreach ($name in @('Assert-JsonNumber', 'Assert-JsonInteger', 'Assert-StableId', 'Format-InvariantFloat', 'Format-InvariantSignedFloat', 'Format-JsonSignedNumbers')) {
    $functions = @($ast.FindAll({ param($node)
        $node -is [Management.Automation.Language.FunctionDefinitionAst] -and $node.Name -ceq $name
    }, $true))
    if ($functions.Count -ne 1) { throw "Missing actual publisher function: $name" }
    Invoke-Expression $functions[0].Extent.Text
}

# Match the previous scalar admission/formatting, including JSON type rejection.
$cases = @(@(0, 1, -1, 0.1, -0.0, -100000.0, 100000.0),
    @([long]42, [uint32]42, [uint64]42, [decimal]0.125),
    @('1'), @($null), @($true), @([double]::NaN),
    @([double]::PositiveInfinity), @(100000.01), @(-100000.01))
foreach ($values in $cases) {
    $expected = $null
    $oldFailed = $false
    try {
        $expected = @($values | ForEach-Object {
            Assert-JsonNumber $_ 'fixture'
            Format-InvariantSignedFloat $_ 'fixture'
        })
    } catch { $oldFailed = $true }
    $actual = $null
    $newFailed = $false
    try { $actual = Format-JsonSignedNumbers $values 'fixture' }
    catch { $newFailed = $true }
    if ($oldFailed -ne $newFailed -or
        (-not $oldFailed -and (($expected -join "`t") -cne ($actual -join "`t")))) {
        throw 'Batched WORLD track number admission or formatting changed.'
    }
}
Write-Host "Gameplay WORLD track number parity: $($cases.Count) cases passed."

# Execute the publisher's real charge admission and PATTERNLOGICCHARGE writer.
# Extract sibling AST statements so this also catches choosing an unsigned formatter.
$chargeInitializers = @($ast.FindAll({ param($node)
    $node -is [Management.Automation.Language.AssignmentStatementAst] -and
    $node.Left -is [Management.Automation.Language.VariableExpressionAst] -and
    $node.Left.VariablePath.UserPath -ceq 'chargeYawOffset' -and $node.Right.Extent.Text -ceq '0.0'
}, $true))
if ($chargeInitializers.Count -ne 1) { throw 'Missing unique publisher charge admission block.' }
$siblings = @($chargeInitializers[0].Parent.Statements)
$start = [Array]::IndexOf($siblings, $chargeInitializers[0])
if ($start -lt 0 -or $start + 2 -ge $siblings.Count -or
    $siblings[$start + 1] -isnot [Management.Automation.Language.IfStatementAst] -or
    $siblings[$start + 2] -isnot [Management.Automation.Language.IfStatementAst] -or
    -not $siblings[$start + 2].Extent.Text.Contains("'PATTERNLOGICCHARGE'")) {
    throw 'Publisher charge admission/writer structure changed; update the focused fixture.'
}
$chargeBlock = [scriptblock]::Create(($siblings[$start..($start + 2)] |
    ForEach-Object { $_.Extent.Text }) -join "`n")
$chargeCases = @(
    @{ Yaw = -90; Text = '-90' }, @{ Yaw = 90; Text = '90' },
    @{ Yaw = -360; Text = '-360' }, @{ Yaw = 360; Text = '360' },
    @{ Yaw = 0; Text = '0' }, @{ Yaw = -90.25; Text = '-90.25' },
    @{ OmitYaw = $true; Text = '0' },
    @{ Yaw = -360.01 }, @{ Yaw = 360.01 }, @{ Yaw = '-90' },
    @{ Yaw = $null }, @{ Yaw = $true }, @{ Yaw = [double]::NaN },
    @{ Yaw = [double]::PositiveInfinity }, @{ Yaw = [double]::NegativeInfinity },
    @{ Yaw = -90; OmitDistance = $true })
$previousCulture = [Threading.Thread]::CurrentThread.CurrentCulture
try {
    [Threading.Thread]::CurrentThread.CurrentCulture = [Globalization.CultureInfo]::GetCultureInfo('fr-FR')
    foreach ($case in $chargeCases) {
        $window = [pscustomobject]@{ windowId = 'fixture.charge'; bossChargeDistanceM = 10.5 }
        if (-not $case.OmitYaw) { $window | Add-Member NoteProperty chargeYawOffsetDegrees $case.Yaw }
        if ($case.OmitDistance) { $window.PSObject.Properties.Remove('bossChargeDistanceM') }
        $windowKind = 'ENTER_AREA'
        $koukuEncounterDocument = [pscustomobject]@{ encounterId = 'fixture.encounter' }
        $koukuPattern = [pscustomobject]@{ patternId = 'fixture.pattern' }
        $patternRows = [Collections.Generic.List[string]]::new()
        $failure = $null
        try { & $chargeBlock } catch { $failure = $_ }
        if ($case.ContainsKey('Text')) {
            $expected = "PATTERNLOGICCHARGE`tfixture.encounter`tfixture.pattern`tfixture.charge`t10.5`t$($case.Text)"
            if ($failure -or $patternRows.Count -ne 1 -or $patternRows[0] -cne $expected) {
                throw "Publisher charge row failed for $($case.Text): $failure"
            }
        } elseif (-not $failure -or $patternRows.Count -ne 0) {
            throw 'Invalid charge yaw was accepted or wrote a partial charge row.'
        }
    }
} finally { [Threading.Thread]::CurrentThread.CurrentCulture = $previousCulture }
Write-Host "Gameplay charge yaw admission/serialization: $($chargeCases.Count) cases passed."

# Run the actual skill admission statements and existing five-column writer.
# No Product projection, source document write or runtime publish is needed.
$skillLoops = @($ast.FindAll({ param($node)
    $node -is [Management.Automation.Language.ForEachStatementAst] -and
    $node.Variable.VariablePath.UserPath -ceq 'skill' -and
    $node.Body.Extent.Text.Contains("'SKILLCOMBATTRAITS'")
}, $true))
if ($skillLoops.Count -ne 1) { throw 'Missing unique publisher player skill loop.' }
$skillStatements = @($skillLoops[0].Body.Statements)
$integerChecks = @($skillStatements | Where-Object {
    $_ -is [Management.Automation.Language.ForEachStatementAst] -and
    $_.Variable.VariablePath.UserPath -ceq 'integerField'
})
$traitStarts = @($skillStatements | Where-Object {
    $_ -is [Management.Automation.Language.AssignmentStatementAst] -and
    $_.Left.Extent.Text -ceq '$damageProfileId'
})
$traitGuards = @($skillStatements | Where-Object {
    $_ -is [Management.Automation.Language.IfStatementAst] -and
    $_.Extent.Text.Contains('Player skill boss combat traits violate the deterministic landed-hit policy:')
})
$traitWriters = @($skillStatements | Where-Object {
    $_.Extent.Text.Contains("'SKILLCOMBATTRAITS'")
})
if ($integerChecks.Count -ne 1 -or $traitStarts.Count -ne 1 -or
    $traitGuards.Count -ne 1 -or $traitWriters.Count -ne 1) {
    throw 'Publisher skill admission/writer structure changed; update the focused fixture.'
}
$traitStart = [Array]::IndexOf($skillStatements, $traitStarts[0])
$traitEnd = [Array]::IndexOf($skillStatements, $traitGuards[0])
if ($traitEnd -le $traitStart) { throw 'Publisher skill trait guard precedes its input.' }
$traitBlock = [scriptblock]::Create((@($integerChecks[0].Extent.Text) +
    @($skillStatements[$traitStart..$traitEnd] | ForEach-Object { $_.Extent.Text }) +
    @($traitWriters[0].Extent.Text)) -join "`n")
$stableIdPattern = '^[A-Za-z0-9_.-]{1,128}$'
$traitCases = @(
    @{ Name = 'existing defaults'; Accept = $true; Change = @{} },
    @{ Name = 'authored 27/350'; Accept = $true; Change = @{ staggerDamage = 27; partDamage = 350 } },
    @{ Name = 'zero powers'; Accept = $true; Change = @{ staggerDamage = 0; partDamage = 0 } },
    @{ Name = 'reader upper bounds'; Accept = $true; Change = @{ staggerDamage = 1000000; partDamage = 1000000 } },
    @{ Name = 'negative stagger'; Change = @{ staggerDamage = -1 } },
    @{ Name = 'negative part'; Change = @{ partDamage = -1 } },
    @{ Name = 'large stagger'; Change = @{ staggerDamage = 1000001 } },
    @{ Name = 'large part'; Change = @{ partDamage = 1000001 } },
    @{ Name = 'uint overflow'; Change = @{ staggerDamage = [uint32]::MaxValue } },
    @{ Name = 'fractional stagger'; Change = @{ staggerDamage = 27.5 } },
    @{ Name = 'floating integer part'; Change = @{ partDamage = 350.0 } },
    @{ Name = 'string stagger'; Change = @{ staggerDamage = '27' } },
    @{ Name = 'null part'; Change = @{ partDamage = $null } },
    @{ Name = 'boolean stagger'; Change = @{ staggerDamage = $true } },
    @{ Name = 'NaN part'; Change = @{ partDamage = [double]::NaN } },
    @{ Name = 'non-damaging zero'; Accept = $true; Change = @{ serverDamageProfileId = ''; staggerDamage = 0; partDamage = 0 } },
    @{ Name = 'non-damaging stagger'; Change = @{ serverDamageProfileId = ''; staggerDamage = 27; partDamage = 0 } },
    @{ Name = 'non-damaging part'; Change = @{ serverDamageProfileId = ''; staggerDamage = 0; partDamage = 350 } },
    @{ Name = 'Q counter'; Accept = $true; Change = @{ inputSlot = 'Q'; counterPower = 1 } },
    @{ Name = 'W counter'; Accept = $true; Change = @{ inputSlot = 'W'; counterPower = 1 } },
    @{ Name = 'E counter'; Accept = $true; Change = @{ inputSlot = 'E'; counterPower = 1 } },
    @{ Name = 'R counter'; Accept = $true; Change = @{ inputSlot = 'R'; counterPower = 1 } },
    @{ Name = 'missing Q capability'; Change = @{ inputSlot = 'Q'; counterPower = 0 } },
    @{ Name = 'scaled counter'; Change = @{ inputSlot = 'Q'; counterPower = 2 } },
    @{ Name = 'unclaimed A capability'; Change = @{ inputSlot = 'A'; counterPower = 1 } },
    @{ Name = 'string counter'; Change = @{ inputSlot = 'Q'; counterPower = '1' } },
    @{ Name = 'Lance Master guard'; Accept = $true; Change = @{ skillId = 34580; skillKind = 'COUNTER'; counterPower = 1 } },
    @{ Name = 'guard without capability'; Change = @{ skillId = 34580; skillKind = 'COUNTER'; counterPower = 0 } },
    @{ Name = 'guard with wrong kind'; Change = @{ skillId = 34580; counterPower = 1 } })
foreach ($case in $traitCases) {
    $skill = [pscustomobject]@{
        skillId = 100; serverDamageProfileId = 'fixture.damage'; inputSlot = 'A'; skillKind = 'ACTIVE'
        staggerDamage = 10; partDamage = 100; counterPower = 0
        cooldownMs = 0; actionDurationMs = 0; hitTimeMs = 0; resourceCost = 0; identityCost = 0
    }
    foreach ($field in $case.Change.Keys) { $skill.$field = $case.Change[$field] }
    $id = [uint32]$skill.skillId
    $skillRows = [Collections.Generic.List[string]]::new()
    $failure = $null
    try { & $traitBlock } catch { $failure = $_ }
    if ($case.Accept) {
        $expected = "SKILLCOMBATTRAITS`t$id`t$($skill.staggerDamage)`t$($skill.partDamage)`t$($skill.counterPower)"
        if ($failure -or $skillRows.Count -ne 1 -or $skillRows[0] -cne $expected) {
            throw "Publisher skill trait case failed ($($case.Name)): $failure"
        }
    } elseif (-not $failure -or $skillRows.Count -ne 0) {
        throw "Invalid skill trait was accepted or emitted a row: $($case.Name)"
    }
}
Write-Host "Gameplay skill trait admission/serialization: $($traitCases.Count) cases passed."
