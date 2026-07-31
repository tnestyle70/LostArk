[CmdletBinding()]
param(
    [string]$SourceCatalogCsv =
        "Client/Bin/Resources/LostArk/Effect/Effect_Tool/SourceCatalog/particle_systems.csv",

    [string]$OutputJson =
        "Client/Bin/Resources/LostArk/Effect/Effect_Tool/SourceCatalog/glaivier_skill_phase_1_3.json",

    [string]$OutputCsv =
        "Client/Bin/Resources/LostArk/Effect/Effect_Tool/SourceCatalog/glaivier_skill_phase_1_3.csv"
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $SourceCatalogCsv)) {
    throw "Source catalog csv does not exist: $SourceCatalogCsv"
}

$Rows = Import-Csv -Path $SourceCatalogCsv
$GlaivierRows = @($Rows | Where-Object {
    $_.group -eq 'Glaivier' -and $_.role -eq 'class_particle_source'
})

if ($GlaivierRows.Count -eq 0) {
    throw "No Glaivier class_particle_source rows found in $SourceCatalogCsv"
}

function Get-PhaseTag {
    param([string]$Tags)

    if ([string]::IsNullOrWhiteSpace($Tags)) {
        return 1
    }

    if ($Tags -like '*pointer_or_target_candidate*') {
        return 3
    }

    if ($Tags -like '*locomotion_candidate*') {
        return 2
    }

    return 1
}

function Get-SkillFamily {
    param([string]$ObjectName)

    if ([string]::IsNullOrWhiteSpace($ObjectName) -or $ObjectName.Length -lt 6) {
        return $ObjectName
    }

    # par_k_flm_counterattack_atk_01 -> par_k_flm, counterattack_atk
    if ($ObjectName -notmatch '^par_[^_]+_[^_]+_(.+)$') {
        return $ObjectName
    }

    $body = $Matches[1]
    if ($body -match '^([^_]+(?:_[^_]+)?)_') {
        return $Matches[1]
    }

    return $body
}

function Get-AssetId {
    param(
        [string]$LogicalPackage,
        [string]$ObjectName
    )

    return ($LogicalPackage + "." + $ObjectName).ToLowerInvariant()
}

$ExportRows = @(
    foreach ($Row in $GlaivierRows) {
        $phase = Get-PhaseTag -Tags $Row.candidate_tags
        $family = Get-SkillFamily -ObjectName $Row.object_name

        [PSCustomObject]@{
            phase            = $phase
            family           = $family
            asset_id         = Get-AssetId -LogicalPackage $Row.logical_package -ObjectName $Row.object_name
            group            = $Row.group
            role             = $Row.role
            logical_package  = $Row.logical_package
            physical_package = $Row.physical_package
            export_index     = [int]$Row.export_index
            export_offset_hex = $Row.export_offset_hex
            export_size_hex   = $Row.export_size_hex
            object_name      = $Row.object_name
            candidate_tags   = $Row.candidate_tags
        }
    }
)

$Sorted = $ExportRows | Sort-Object phase, family, logical_package, object_name

$grouped = $Sorted | Group-Object phase | Sort-Object Name | ForEach-Object {
    [PSCustomObject]@{
        phase = [int]$_.Name
        count = $_.Count
        examples = @(
            @($_.Group | Select-Object -ExpandProperty asset_id -First 5)
        )
    }
}

$manifest = [PSCustomObject]@{
    schema_version = 1
    generated_at = (Get-Date).ToString('o')
    source_csv = (Resolve-Path -LiteralPath $SourceCatalogCsv).ProviderPath
    source_group = 'Glaivier'
    source_role  = 'class_particle_source'
    total_rows = $Sorted.Count
    phase_summary = @($grouped)
    rows = @($Sorted)
}

$OutputDir = Split-Path -Path $OutputJson -Parent
if (-not (Test-Path -LiteralPath $OutputDir)) {
	throw "Output directory does not exist: $OutputDir"
}

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText(
    $OutputJson,
    ($manifest | ConvertTo-Json -Depth 64) + [Environment]::NewLine,
    $utf8NoBom
)

$Sorted | Export-Csv -Path $OutputCsv -NoTypeInformation -Encoding UTF8

Write-Host "출력됨: Glaivier Row Count = $($Sorted.Count)"
Write-Host "JSON: $((Resolve-Path -LiteralPath $OutputJson).ProviderPath)"
Write-Host "CSV : $((Resolve-Path -LiteralPath $OutputCsv).ProviderPath)"
