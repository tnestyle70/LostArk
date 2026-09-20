[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$tokens = $null
$errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile(
    (Join-Path $PSScriptRoot 'Publish-GameplayBalance.ps1'), [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw ($errors | Out-String) }
foreach ($name in @('Assert-JsonNumber', 'Format-InvariantFloat', 'Format-InvariantSignedFloat', 'Format-JsonSignedNumbers')) {
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
