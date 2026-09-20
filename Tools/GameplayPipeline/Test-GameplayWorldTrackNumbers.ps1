[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$tokens = $null
$errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile(
    (Join-Path $PSScriptRoot 'Publish-GameplayBalance.ps1'), [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw ($errors | Out-String) }
foreach ($name in @('Assert-JsonNumber', 'Format-InvariantSignedFloat', 'Format-JsonSignedNumbers')) {
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
