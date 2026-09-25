[CmdletBinding()]
param([string]$SequencePath = 'Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json', [string]$LaneOutputPath = '')
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$tokens = $null; $errors = $null
$ast = [Management.Automation.Language.Parser]::ParseFile((Join-Path $PSScriptRoot 'Publish-WorldGameplay.ps1'), [ref]$tokens, [ref]$errors)
if (@($errors).Count) { throw ($errors -join "`n") }
$function = $ast.Find({param($node) $node -is [Management.Automation.Language.FunctionDefinitionAst] -and $node.Name -eq 'ConvertTo-KoukuCardMazeLane'}, $false)
if (!$function) { throw 'Missing product lane projector.' }
. ([scriptblock]::Create($function.Extent.Text))
$resolved = if ([IO.Path]::IsPathRooted($SequencePath)) { $SequencePath } else { Join-Path $repoRoot $SequencePath }
$document = [IO.File]::ReadAllText($resolved, [Text.Encoding]::UTF8) | ConvertFrom-Json
$lanes = @($document.instances | Where-Object { $_.instanceId -like 'cardmiro.march.instance.*' -and $_.enabled })
if ($lanes.Count -ne 36) { throw 'Fixture needs all 36 published lanes.' }
$lines = @()
foreach ($instance in $lanes) {
    $template = @($document.templates | Where-Object sequenceId -CEQ $instance.templateId)[0]
    $line = ConvertTo-KoukuCardMazeLane $instance $template
    $fields = $line -split "`t"
    $keys = @($template.tracks[0].keys)
    $lead = if ($keys.Count -eq 3) { [int]$keys[1].timeMs } else { 0 }
    if ([int]$fields[1] -ne $instance.startDelayMs + $lead -or [int]$fields[2] -ne $template.durationMs - $lead) { throw 'Contact and visible motion clocks disagree.' }
    if ($lead -gt 0) {
        if ($lead -ne 2000) { throw 'The current Seto warning contract requires 2000ms.' }
        foreach ($effect in $template.effectTracks) {
            if ($effect.startMs -ne 0 -or $effect.durationMs -ne $lead -or $effect.followObject -or $effect.inheritObjectRotation) { throw 'Warning must remain at its fixed starting lane throughout the lead-in.' }
        }
    }
    for ($axis = 0; $axis -lt 3; ++$axis) {
        if ([Math]::Abs([double]$fields[3+$axis] - ($instance.position[$axis] + $keys[0].positionOffset[$axis])) -gt .0001 -or
            [Math]::Abs([double]$fields[6+$axis] - ($instance.position[$axis] + $keys[-1].positionOffset[$axis])) -gt .0001) { throw 'Contact and visual lane endpoints disagree.' }
    }
    $lines += $line
}
$template = $document.templates | Where-Object sequenceId -CEQ $lanes[0].templateId
$legacy = ($template | ConvertTo-Json -Depth 40 | ConvertFrom-Json)
if ($legacy.tracks[0].keys.Count -eq 3) {
    $legacy.durationMs -= 2000
    $legacy.tracks[0].keys = @($legacy.tracks[0].keys[1],$legacy.tracks[0].keys[2])
    $legacy.tracks[0].keys[0].timeMs = 0; $legacy.tracks[0].keys[1].timeMs -= 2000
}
$legacyFields = (ConvertTo-KoukuCardMazeLane $lanes[0] $legacy) -split "`t"
if ([int]$legacyFields[1] -ne $lanes[0].startDelayMs -or [int]$legacyFields[2] -ne $legacy.durationMs) { throw 'Legacy two-key projection changed.' }
$fixture = $legacy | ConvertTo-Json -Depth 40 | ConvertFrom-Json
$start = $fixture.tracks[0].keys[0] | ConvertTo-Json -Depth 20 | ConvertFrom-Json
$start.visible = $false; $fixture.tracks[0].keys[0].timeMs = 2000; $fixture.tracks[0].keys[1].timeMs += 2000; $fixture.durationMs += 2000
$fixture.tracks[0].keys = @($start) + $fixture.tracks[0].keys
foreach ($fault in @('visible','moving-hold','zero-lead','end-before-lead','rotation-during-hold')) {
    $bad = $fixture | ConvertTo-Json -Depth 40 | ConvertFrom-Json
    switch ($fault) {
        'visible' { $bad.tracks[0].keys[0].visible = $true }
        'moving-hold' { $bad.tracks[0].keys[0].positionOffset[0] += 1 }
        'zero-lead' { $bad.tracks[0].keys[1].timeMs = 0 }
        'end-before-lead' { $bad.durationMs = 1000; $bad.tracks[0].keys[-1].timeMs = 1000 }
        'rotation-during-hold' { $bad.tracks[0].keys[0].rotationQuaternion[0] += .1 }
    }
    $rejected = $false
    try { $null = ConvertTo-KoukuCardMazeLane $lanes[0] $bad } catch { $rejected = $true }
    if (!$rejected) { throw "Unsafe lead-in was admitted: $fault" }
}
if ($LaneOutputPath) { [IO.File]::WriteAllLines([IO.Path]::GetFullPath($LaneOutputPath), [string[]]$lines, [Text.UTF8Encoding]::new($false)) }
Write-Output 'Card maze warning contract PASS: 36 visual/contact paths, legacy two-key preservation, five invalid lead-ins rejected.'
