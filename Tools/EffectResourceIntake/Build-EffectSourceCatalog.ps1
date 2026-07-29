[CmdletBinding()]
param(
    [string]$PackageRoot = '',
    [string]$UModelPath = '',
    [string]$DestinationRoot = '',
    [switch]$SkipRawCopy
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

function New-Directory {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Get-DefaultUModelPath {
    $candidates = @(
        (Join-Path $env:USERPROFILE 'OneDrive\Desktop\UModel\umodel_win32\umodel_lostark_v7.exe'),
        (Join-Path $env:USERPROFILE 'Desktop\UModel\umodel_win32\umodel_lostark_v7.exe')
    )

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    foreach ($searchRoot in @(
        (Join-Path $env:USERPROFILE 'OneDrive'),
        (Join-Path $env:USERPROFILE 'Desktop')
    )) {
        if (-not (Test-Path -LiteralPath $searchRoot)) {
            continue
        }

        $match = Get-ChildItem `
            -LiteralPath $searchRoot `
            -Filter 'umodel_lostark_v7.exe' `
            -File `
            -Recurse `
            -ErrorAction SilentlyContinue |
            Select-Object -First 1

        if ($null -ne $match) {
            return $match.FullName
        }
    }

    return ''
}

function Invoke-UModel {
    param(
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [switch]$AllowFailure
    )

    $lines = @(& $script:UModelPath @Arguments 2>&1 | ForEach-Object { "$_" })
    $exitCode = $LASTEXITCODE

    if ($exitCode -ne 0 -and -not $AllowFailure) {
        throw "UModel failed with exit code $exitCode.`n$($lines -join [Environment]::NewLine)"
    }

    return [pscustomobject]@{
        ExitCode = $exitCode
        Lines = $lines
    }
}

function Resolve-PhysicalPackage {
    param([Parameter(Mandatory = $true)][string]$LogicalPackage)

    $arguments = @(
        '-pkginfo',
        '-game=lostark',
        '-kr',
        '-nameresolve',
        '-noanim',
        "-path=$script:ReleaseRoot",
        $LogicalPackage
    )

    $result = Invoke-UModel -Arguments $arguments -AllowFailure
    foreach ($line in $result.Lines) {
        if ($line -match 'Loading package:\s+(?:Packages[/\\])?(?<Physical>[^ ]+\.upk)\s') {
            return $Matches.Physical
        }
    }

    return $null
}

function Get-ParticleRows {
    param(
        [Parameter(Mandatory = $true)]$PackageDefinition,
        [Parameter(Mandatory = $true)][string]$PhysicalPackage,
        [Parameter(Mandatory = $true)][string]$LogRoot
    )

    $arguments = @(
        '-list',
        '-game=lostark',
        '-kr',
        '-nameresolve',
        '-noanim',
        "-path=$script:ReleaseRoot",
        $PackageDefinition.Logical
    )

    $result = Invoke-UModel -Arguments $arguments
    $logPath = Join-Path $LogRoot ($PackageDefinition.Logical + '.list.txt')
    $result.Lines | Set-Content -LiteralPath $logPath -Encoding utf8

    $rows = foreach ($line in $result.Lines) {
        if ($line -notmatch '^\s*(?<Index>\d+)\s+(?<Offset>[0-9A-F]+)\s+(?<Size>[0-9A-F]+)\s+particlesystem\s+(?<Name>\S+)\s*$') {
            continue
        }

        $exportIndex = [int]$Matches.Index
        $exportOffset = $Matches.Offset
        $exportSize = $Matches.Size
        $name = $Matches.Name
        $tags = [System.Collections.Generic.List[string]]::new()

        if ($PackageDefinition.Group -eq 'Bard' -or $PackageDefinition.Group -eq 'Glaivier') {
            $tags.Add('class_skill_candidate')
        }
        if ($name -match '(?i)foot|step|walk|run|moving|move|dash|exmove|glide') {
            $tags.Add('locomotion_candidate')
        }
        if ($name -match '(?i)cursor|mouse|click|pointer|select|movetrack|target|marker') {
            $tags.Add('pointer_or_target_candidate')
        }

        [pscustomobject]@{
            group = $PackageDefinition.Group
            role = $PackageDefinition.Role
            logical_package = $PackageDefinition.Logical
            physical_package = $PhysicalPackage
            export_index = $exportIndex
            export_offset_hex = $exportOffset
            export_size_hex = $exportSize
            object_name = $name
            candidate_tags = ($tags -join ';')
        }
    }

    return @($rows)
}

function Export-CursorTexture {
    param(
        [Parameter(Mandatory = $true)][string]$Format,
        [Parameter(Mandatory = $true)][string]$OutputRoot
    )

    New-Directory -Path $OutputRoot

    $arguments = @(
        '-export',
        '-game=lostark',
        '-kr',
        '-nameresolve',
        '-noanim',
        "-$Format",
        '-groups',
        "-path=$script:ReleaseRoot",
        "-out=$OutputRoot",
        '-obj=cursoreffect_i5',
        'EFUI_CURSOREFFECT'
    )

    [void](Invoke-UModel -Arguments $arguments)
}

if ([string]::IsNullOrWhiteSpace($PackageRoot)) {
    $PackageRoot = Join-Path $env:ProgramData 'Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages'
}
if ([string]::IsNullOrWhiteSpace($UModelPath)) {
    $UModelPath = Get-DefaultUModelPath
}
if ([string]::IsNullOrWhiteSpace($DestinationRoot)) {
    $DestinationRoot = Join-Path $PSScriptRoot '..\..\Client\Bin\Resources\LostArk\Effect'
}

$PackageRoot = [System.IO.Path]::GetFullPath($PackageRoot)
$DestinationRoot = [System.IO.Path]::GetFullPath($DestinationRoot)
if ([string]::IsNullOrWhiteSpace($UModelPath)) {
    throw 'Lost Ark UModel was not found. Pass -UModelPath explicitly.'
}
$script:UModelPath = [System.IO.Path]::GetFullPath($UModelPath)
$script:ReleaseRoot = Split-Path -Parent $PackageRoot

if (-not (Test-Path -LiteralPath $PackageRoot)) {
    throw "Lost Ark package directory was not found: $PackageRoot"
}
if ([string]::IsNullOrWhiteSpace($UModelPath) -or -not (Test-Path -LiteralPath $script:UModelPath)) {
    throw 'Lost Ark UModel was not found. Pass -UModelPath explicitly.'
}

$sourceRawRoot = Join-Path $DestinationRoot 'SourceRaw'
$sourceCatalogRoot = Join-Path $DestinationRoot 'SourceCatalog'
$sourceExtractedRoot = Join-Path $DestinationRoot 'SourceExtracted'
$logRoot = Join-Path $sourceCatalogRoot 'UModelLists'

foreach ($directory in @($sourceRawRoot, $sourceCatalogRoot, $sourceExtractedRoot, $logRoot)) {
    New-Directory -Path $directory
}

$packageDefinitions = [System.Collections.Generic.List[object]]::new()

0..8 | ForEach-Object {
    $packageDefinitions.Add([pscustomobject]@{
        Group = 'Bard'
        Role = 'class_particle_source'
        Logical = ('FX_PC_MBD_{0:D2}' -f $_)
        CopyRaw = $true
        ScanParticles = $true
    })
}

0..10 | ForEach-Object {
    $packageDefinitions.Add([pscustomobject]@{
        Group = 'Glaivier'
        Role = 'class_particle_source'
        Logical = ('FX_PC_FLM_{0:D2}' -f $_)
        CopyRaw = $true
        ScanParticles = $true
    })
}

foreach ($definition in @(
    @{ Group = 'Common'; Role = 'selection_and_move_track'; Logical = 'FX_BS_03'; CopyRaw = $true; ScanParticles = $true },
    @{ Group = 'Common'; Role = 'dash_and_extra_move'; Logical = 'FX_BS_04'; CopyRaw = $true; ScanParticles = $true },
    @{ Group = 'Common'; Role = 'terrain_footstep'; Logical = 'FX_BS_06'; CopyRaw = $true; ScanParticles = $true },
    @{ Group = 'MousePointer'; Role = 'cursor_data'; Logical = 'EFUI_CURSOR'; CopyRaw = $true; ScanParticles = $false },
    @{ Group = 'MousePointer'; Role = 'cursor_click_effect'; Logical = 'EFUI_CURSOREFFECT'; CopyRaw = $true; ScanParticles = $false },
    @{ Group = 'Glaivier'; Role = 'weapon_trail_source'; Logical = 'LANCEMASTER_ANIMNOTIFY_TRAILS'; CopyRaw = $true; ScanParticles = $false },
    @{ Group = 'Bard'; Role = 'class_animation_reference'; Logical = 'PC_MBD_00'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Bard'; Role = 'shared_mage_animation_reference'; Logical = 'PC_MG_00'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Bard'; Role = 'class_sound_reference'; Logical = 'PC_BARD_F'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Bard'; Role = 'harp_weapon_reference'; Logical = 'WP_MBD_HR_00'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Bard'; Role = 'skill_camera_reference'; Logical = 'STANDARD_SKILLCAM_BARD'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Glaivier'; Role = 'class_animation_reference'; Logical = 'PC_FLM_00'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Glaivier'; Role = 'shared_female_fighter_animation_reference'; Logical = 'PC_FT_00'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Glaivier'; Role = 'starter_weapon_reference'; Logical = 'WP_WFLM_00'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Glaivier'; Role = 'class_sound_reference'; Logical = 'PC_LANCEMASTER_F'; CopyRaw = $false; ScanParticles = $false },
    @{ Group = 'Glaivier'; Role = 'skill_camera_reference'; Logical = 'STANDARD_SKILLCAM_LANCEMASTER'; CopyRaw = $false; ScanParticles = $false }
)) {
    $packageDefinitions.Add([pscustomobject]$definition)
}

$manifestRows = [System.Collections.Generic.List[object]]::new()
$particleRows = [System.Collections.Generic.List[object]]::new()

foreach ($definition in $packageDefinitions) {
    Write-Host "Resolving $($definition.Logical) ..."
    $physicalPackage = Resolve-PhysicalPackage -LogicalPackage $definition.Logical

    if ([string]::IsNullOrWhiteSpace($physicalPackage)) {
        $manifestRows.Add([pscustomobject]@{
            group = $definition.Group
            role = $definition.Role
            logical_package = $definition.Logical
            physical_package = $null
            status = 'not_found'
            source_size_bytes = 0
            sha256 = $null
            copied_raw = $false
            particle_system_count = 0
        })
        continue
    }

    $sourcePath = Join-Path $PackageRoot $physicalPackage
    if (-not (Test-Path -LiteralPath $sourcePath)) {
        throw "Resolved package is missing from disk: $sourcePath"
    }

    $file = Get-Item -LiteralPath $sourcePath
    $hash = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash
    $copiedRaw = $false

    if ($definition.CopyRaw -and -not $SkipRawCopy) {
        $groupRoot = Join-Path $sourceRawRoot $definition.Group
        New-Directory -Path $groupRoot
        $destinationName = '{0}__{1}' -f $definition.Logical, $physicalPackage
        Copy-Item -LiteralPath $sourcePath -Destination (Join-Path $groupRoot $destinationName) -Force
        $copiedRaw = $true
    }

    $packageParticleRows = @()
    if ($definition.ScanParticles) {
        $packageParticleRows = Get-ParticleRows `
            -PackageDefinition $definition `
            -PhysicalPackage $physicalPackage `
            -LogRoot $logRoot

        foreach ($row in $packageParticleRows) {
            $particleRows.Add($row)
        }
    }

    $manifestRows.Add([pscustomobject]@{
        group = $definition.Group
        role = $definition.Role
        logical_package = $definition.Logical
        physical_package = $physicalPackage
        status = 'resolved'
        source_size_bytes = [long]$file.Length
        sha256 = $hash
        copied_raw = $copiedRaw
        particle_system_count = $packageParticleRows.Count
    })
}

$particleCsvPath = Join-Path $sourceCatalogRoot 'particle_systems.csv'
$locomotionCsvPath = Join-Path $sourceCatalogRoot 'locomotion_candidates.csv'
$pointerCsvPath = Join-Path $sourceCatalogRoot 'pointer_and_target_candidates.csv'

$particleRows |
    Sort-Object group, logical_package, export_index |
    Export-Csv -LiteralPath $particleCsvPath -NoTypeInformation -Encoding utf8

$particleRows |
    Where-Object { $_.candidate_tags -match 'locomotion_candidate' } |
    Sort-Object group, logical_package, object_name |
    Export-Csv -LiteralPath $locomotionCsvPath -NoTypeInformation -Encoding utf8

$particleRows |
    Where-Object { $_.candidate_tags -match 'pointer_or_target_candidate' } |
    Sort-Object group, logical_package, object_name |
    Export-Csv -LiteralPath $pointerCsvPath -NoTypeInformation -Encoding utf8

Export-CursorTexture `
    -Format 'png' `
    -OutputRoot (Join-Path $sourceExtractedRoot 'MousePointer\PNG')
Export-CursorTexture `
    -Format 'dds' `
    -OutputRoot (Join-Path $sourceExtractedRoot 'MousePointer\DDS')

$bardParticleCount = @($particleRows | Where-Object { $_.group -eq 'Bard' }).Count
$glaivierParticleCount = @($particleRows | Where-Object { $_.group -eq 'Glaivier' }).Count
$commonParticleCount = @($particleRows | Where-Object { $_.group -eq 'Common' }).Count

$manifest = [ordered]@{
    schema_version = 1
    generated_at = (Get-Date).ToString('o')
    source_package_root = $PackageRoot
    umodel_path = $script:UModelPath
    destination_root = $DestinationRoot
    particle_summary = [ordered]@{
        bard = $bardParticleCount
        glaivier = $glaivierParticleCount
        common_scanned = $commonParticleCount
        total = $particleRows.Count
    }
    starter_candidates = [ordered]@{
        bard_dash = @(
            'FX_PC_MBD_05.par_s_mbd_exmove_01',
            'FX_PC_MBD_05.par_s_mbd_exmove_02'
        )
        glaivier_dash = @(
            'FX_PC_FLM_02.par_d_flm_dash_01',
            'FX_PC_FLM_03.par_n_flm_dash_002',
            'FX_PC_FLM_03.par_n_flm_dash_01',
            'FX_PC_FLM_03.par_n_flm_dash_02',
            'FX_PC_FLM_03.par_n_flm_dash_03',
            'FX_PC_FLM_03.par_n_flm_dashup_01',
            'FX_PC_FLM_03.par_n_flm_shockdash_01',
            'FX_PC_FLM_03.par_n_flm_shockdash_02',
            'FX_PC_FLM_03.par_n_flm_shockdash_04',
            'FX_PC_FLM_05.par_d_flm_moving_01'
        )
        footstep = @(
            'FX_BS_06.par_d_footstepdefault_001',
            'FX_BS_06.par_g_footstep_light_01',
            'FX_BS_06.par_g_footstep_light_grass_01',
            'FX_BS_06.par_g_footstep_light_mud_001',
            'FX_BS_06.par_g_footstep_light_snow_001',
            'FX_BS_06.par_s_footstepwater_001'
        )
        mouse_ground_feedback = @(
            'FX_BS_03.par_b_select_01',
            'FX_BS_03.par_b_select_02_loc_int',
            'FX_BS_03.par_g_movetrack_01',
            'FX_BS_03.par_g_movetrack_02',
            'FX_BS_03.par_i_movetrack_01',
            'FX_BS_03.par_i_movetrack_02',
            'FX_BS_03.par_l_movetrack_01'
        )
        mouse_cursor_visual = @(
            'EFUI_CURSOR (56 EFCursorData objects; raw package only)',
            'EFUI_CURSOREFFECT.cursoreffect_i5 (PNG and DDS exported)'
        )
    }
    limitations = @(
        'ParticleSystem names are discovery evidence, not a recovered runtime invocation graph.',
        'The current Lost Ark UModel build does not deserialize the custom Cascade module graph.',
        'Per-particle mesh, material, texture, curve, timing, and shader dependencies remain unresolved.',
        'Raw UPK files and extracted resources stay under Client/Bin/Resources and are intentionally ignored by Git.'
    )
    packages = @($manifestRows)
}

$manifestPath = Join-Path $sourceCatalogRoot 'source_manifest.json'
$manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $manifestPath -Encoding utf8

Write-Host ''
Write-Host "Bard ParticleSystem count: $bardParticleCount"
Write-Host "Glaivier ParticleSystem count: $glaivierParticleCount"
Write-Host "Common ParticleSystem count: $commonParticleCount"
Write-Host "Manifest: $manifestPath"
