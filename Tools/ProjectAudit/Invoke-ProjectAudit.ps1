[CmdletBinding()]
param(
    [switch]$DeepAssetHash,
    [string]$ReportPath = '.codex_tmp/ProjectAudit.json'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Get-Location).Path)
$checks = [Collections.Generic.List[object]]::new()
$failures = [Collections.Generic.List[string]]::new()

function Add-Check {
    param(
        [string]$Name,
        [bool]$Passed,
        [string]$Detail
    )

    $checks.Add([ordered]@{
        name = $Name
        passed = $Passed
        detail = $Detail
    })
    if (-not $Passed) {
        $failures.Add("$Name`: $Detail")
    }
}

function Read-Json {
    param([string]$Path)
	return Get-Content -LiteralPath $Path -Raw -Encoding UTF8 | ConvertFrom-Json
}

function Get-ProjectItems {
    param([string]$ProjectPath)

    [xml]$project = Get-Content -LiteralPath $ProjectPath
    $manager = [Xml.XmlNamespaceManager]::new($project.NameTable)
    $manager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')
    return @($project.SelectNodes('//m:ClInclude|//m:ClCompile|//m:FxCompile|//m:None', $manager))
}

try {
    $resourceRoot = Join-Path $repoRoot 'Client\Bin\Resources'
    $expectedDomains = @('Character', 'Deploy', 'Effect', 'Fonts', 'Map', 'UI')
    $actualDomains = @(Get-ChildItem -LiteralPath $resourceRoot -Force |
        ForEach-Object Name | Sort-Object)
    Add-Check 'resource.top-level' (($actualDomains -join "`n") -eq (($expectedDomains | Sort-Object) -join "`n")) "actual=$($actualDomains -join ',')"

    $forbiddenResourceNames = @('LostArk', 'Models', 'Textures', 'SourceData', 'Sound')
    $presentForbidden = @($forbiddenResourceNames | Where-Object {
        Test-Path -LiteralPath (Join-Path $resourceRoot $_) })
    Add-Check 'resource.forbidden-roots' ($presentForbidden.Count -eq 0) "present=$($presentForbidden -join ',')"

    $forbiddenRuntimeFiles = @(Get-ChildItem -LiteralPath $resourceRoot -Recurse -File -Force |
        Where-Object { $_.Extension.ToLowerInvariant() -in @('.cfg', '.exe', '.dll', '.pdb') })
    Add-Check 'resource.forbidden-files' ($forbiddenRuntimeFiles.Count -eq 0) "count=$($forbiddenRuntimeFiles.Count)"

    $lock = Read-Json 'Data\AssetPacks.lock.json'
    $pack = @($lock.packs | Where-Object packId -eq 'lostark-resources')
    Add-Check 'asset-lock.identity' ($lock.schema -eq 'lostark.asset-pack-lock' -and $lock.formatVersion -eq 1 -and $pack.Count -eq 1) "packs=$($pack.Count)"
    if ($pack.Count -eq 1) {
        $manifest = Read-Json $pack[0].manifest
        $actualFileCount = @(Get-ChildItem -LiteralPath $resourceRoot -Recurse -File -Force).Count
        $actualBytes = [long]((Get-ChildItem -LiteralPath $resourceRoot -Recurse -File -Force |
            Measure-Object Length -Sum).Sum)
        Add-Check 'asset-lock.inventory' ($actualFileCount -eq $manifest.fileCount -and $actualBytes -eq $manifest.totalBytes) "files=$actualFileCount bytes=$actualBytes"
        $manifestHash = (Get-FileHash -LiteralPath $pack[0].manifest -Algorithm SHA256).Hash.ToLowerInvariant()
        Add-Check 'asset-lock.manifest-hash' ($manifestHash -eq $pack[0].manifestSha256) $manifestHash
    }

    if ($DeepAssetHash) {
        $verifyOutput = & .\Tools\AssetPipeline\Manage-ResourcePack.ps1 -Mode Verify 2>&1
        Add-Check 'asset-lock.deep-verify' $true ($verifyOutput -join ' ')
    }

    $snapshotRollbackPassed = $false
    $snapshotRollbackDetail = ''
    $snapshotFixtureRoot = Join-Path $repoRoot ".codex_tmp\ResourceSnapshotFixture-$PID"
    $resourcePackScript = Join-Path $repoRoot 'Tools\AssetPipeline\Manage-ResourcePack.ps1'
    try {
        $snapshotResourceRoot = Join-Path $snapshotFixtureRoot 'Client\Bin\Resources'
        foreach ($domain in @('Character', 'Deploy', 'Effect', 'Fonts', 'Map', 'UI')) {
            [IO.Directory]::CreateDirectory((Join-Path $snapshotResourceRoot $domain)) | Out-Null
        }
        [IO.File]::WriteAllText(
            (Join-Path $snapshotResourceRoot 'Fonts\fixture.asset'),
            'fixture',
            [Text.UTF8Encoding]::new($false))
        $snapshotLockPath = Join-Path $snapshotFixtureRoot 'Data\AssetPacks.lock.json'
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($snapshotLockPath)) | Out-Null
        [IO.File]::WriteAllText(
            $snapshotLockPath,
            "{`"schema`":`"lostark.asset-pack-lock`",`"formatVersion`":1,`"packs`":[]}",
            [Text.UTF8Encoding]::new($false))

        Push-Location $snapshotFixtureRoot
        try {
            $injectedFailureRejected = $false
            try {
                & $resourcePackScript `
                    -Mode Snapshot `
                    -PackId fixture-resources `
                    -Version 1.0.0 `
                    -FailureAfterManifest | Out-Null
            }
            catch {
                $injectedFailureRejected = $true
            }
            $orphanAfterFailure = @(Get-ChildItem -LiteralPath 'Data\AssetManifests' -File -ErrorAction SilentlyContinue)
            $temporaryAfterFailure = @(Get-ChildItem -LiteralPath 'Data' -Recurse -Force -ErrorAction SilentlyContinue |
                Where-Object Name -Match '\.tmp$|\.replace-backup$')
            $lockAfterFailure = Get-Content -LiteralPath 'Data\AssetPacks.lock.json' -Raw | ConvertFrom-Json

            & $resourcePackScript `
                -Mode Snapshot `
                -PackId fixture-resources `
                -Version 1.0.0 | Out-Null
            & $resourcePackScript `
                -Mode Verify `
                -PackId fixture-resources | Out-Null
            $manifestAfterSuccess = @(Get-ChildItem -LiteralPath 'Data\AssetManifests' -File)
            $lockAfterSuccess = Get-Content -LiteralPath 'Data\AssetPacks.lock.json' -Raw | ConvertFrom-Json
            $snapshotRollbackPassed =
                $injectedFailureRejected -and
                $orphanAfterFailure.Count -eq 0 -and
                $temporaryAfterFailure.Count -eq 0 -and
                @($lockAfterFailure.packs).Count -eq 0 -and
                $manifestAfterSuccess.Count -eq 1 -and
                @($lockAfterSuccess.packs).Count -eq 1
            $snapshotRollbackDetail =
                "rejected=$injectedFailureRejected orphan=$($orphanAfterFailure.Count) temp=$($temporaryAfterFailure.Count)"
        }
        finally {
            Pop-Location
        }
    }
    catch {
        $snapshotRollbackDetail = $_.Exception.Message
    }
    finally {
        if ([IO.Directory]::Exists($snapshotFixtureRoot)) {
            Remove-Item -LiteralPath $snapshotFixtureRoot -Recurse -Force
        }
    }
    Add-Check 'asset-lock.snapshot-rollback' $snapshotRollbackPassed $snapshotRollbackDetail

    $uiDocuments = @(
        'Data\UI\HUD\HUD_Layout.json',
        'Data\UI\ScreenUI\ScreenUI.json')
    $totalSlots = 0
    $uiValid = $true
    foreach ($uiPath in $uiDocuments) {
        $ui = Read-Json $uiPath
        $slotIds = @($ui.slots | ForEach-Object id)
        $duplicates = @($slotIds | Group-Object | Where-Object Count -gt 1)
        $badPaths = @($ui.slots | ForEach-Object {
            $_.layers.path
            $_.layers.hoverPath
            $_.shine.texture
            $_.animation.frames
        } | Where-Object { $_ -and $_ -notlike 'UI/*' })
        $uiValid = $uiValid -and $ui.schema -eq 'lostark.ui-layout' -and
            $ui.formatVersion -eq 1 -and $duplicates.Count -eq 0 -and
            $badPaths.Count -eq 0 -and @($ui.slots).Count -gt 0
        $totalSlots += @($ui.slots).Count
    }
    Add-Check 'ui.json-contract' $uiValid "documents=$($uiDocuments.Count) slots=$totalSlots"

    $levelCatalog = Read-Json 'Data\Levels\LevelCatalog.json'
    $requiredScenarios = @(
        'front.lobby', 'world.bern', 'raid.valtan.arena',
        'dev.training.ground', 'dev.map.active', 'asset.character.lance-master',
        'render.hdr-readback', 'effect.preview', 'ui.hud.layout')
    $scenarioIds = @($levelCatalog.levels | ForEach-Object id)
    $missingScenarios = @($requiredScenarios | Where-Object { $_ -notin $scenarioIds })
    $productActive = @($levelCatalog.levels | Where-Object {
        $_.kind -eq 'product' -and $_.mapAreaId -eq '@ACTIVE' })
    Add-Check 'levels.catalog' ($levelCatalog.schema -eq 'lostark.level-catalog' -and $missingScenarios.Count -eq 0 -and $productActive.Count -eq 0) "missing=$($missingScenarios -join ',') productActive=$($productActive.Count)"
	$expectedLevelMetadata = @{
		'front.lobby' = @{ domains = ''; tools = '' }
		'world.bern' = @{ domains = 'Map,Character'; tools = '' }
		'raid.valtan.arena' = @{ domains = 'Map,Character,Deploy,Effect,UI'; tools = '' }
		'dev.training.ground' = @{ domains = 'Map,Character,UI'; tools = '' }
		'dev.map.active' = @{ domains = 'Map'; tools = 'MapTool' }
		'asset.character.lance-master' = @{ domains = 'Character'; tools = 'AnimationTool' }
		'render.hdr-readback' = @{ domains = 'Effect'; tools = 'EffectTool' }
		'effect.preview' = @{ domains = 'Effect'; tools = 'EffectTool' }
		'ui.hud.layout' = @{ domains = 'UI'; tools = 'HUDLayoutTool' }
	}
	$metadataMismatches = @($levelCatalog.levels | Where-Object {
		$expected = $expectedLevelMetadata[$_.id]
		$null -eq $expected -or
		(@($_.assetDomains) -join ',') -ne $expected.domains -or
		(@($_.tools) -join ',') -ne $expected.tools
	})
	Add-Check 'levels.execution-metadata-parity' (
		$metadataMismatches.Count -eq 0) "mismatches=$($metadataMismatches.id -join ',')"

    $scopedProductLevels = @($levelCatalog.levels | Where-Object {
        $_.kind -eq 'product' -and $_.mapAreaId -and $null -ne $_.mapLoadBounds })
    $fullDevelopmentMaps = @($levelCatalog.levels | Where-Object {
        $_.kind -eq 'development' -and $_.mapAreaId -and $null -eq $_.mapLoadBounds })
    $scopedProductIds = (@($scopedProductLevels.id) | Sort-Object) -join ','
    Add-Check 'levels.product-load-scope' (
        $scopedProductLevels.Count -eq 2 -and
        $scopedProductIds -eq 'raid.valtan.arena,world.bern' -and
        $fullDevelopmentMaps.Count -eq 1 -and
        $fullDevelopmentMaps[0].id -eq 'dev.map.active') "product=$($scopedProductLevels.id -join ',') fullDev=$($fullDevelopmentMaps.id -join ',')"
	$trainingScenario = @($levelCatalog.levels | Where-Object id -eq 'dev.training.ground')
	Add-Check 'levels.training-scope' (
		$trainingScenario.Count -eq 1 -and
		$trainingScenario[0].level -eq 'DEVELOPMENT' -and
		$trainingScenario[0].mapAreaId -eq 'LV_DEV_TRAINING_GROUND' -and
		$null -ne $trainingScenario[0].mapLoadBounds) "count=$($trainingScenario.Count)"

    $mapCatalog = Read-Json 'Data\Maps\MapCatalog.json'
    $mapRoot = 'Client\Bin\DataFiles\Map'
    $missingMapFiles = [Collections.Generic.List[string]]::new()
    foreach ($area in $mapCatalog.areas) {
        foreach ($property in @('catalog', 'placements', 'deployCatalog', 'deployPlacements', 'gameplayDocument')) {
            if ($area.PSObject.Properties.Name -contains $property -and
                -not (Test-Path -LiteralPath $area.$property)) {
                $missingMapFiles.Add($area.$property)
            }
        }
    }
    $mapDataNames = @(Get-ChildItem -LiteralPath $mapRoot -File | ForEach-Object Name)
    $legacyMapFiles = @($mapDataNames | Where-Object { $_ -like 'BG_RAD_VALTAN_A*' })
    Add-Check 'maps.catalog' ($mapCatalog.schema -eq 'lostark.map-catalog' -and $missingMapFiles.Count -eq 0 -and $legacyMapFiles.Count -eq 0) "missing=$($missingMapFiles.Count) legacy=$($legacyMapFiles.Count)"
	$trainingArea = @($mapCatalog.areas | Where-Object id -eq 'LV_DEV_TRAINING_GROUND')
	$trainingAssetRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapassets') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapassets' | Select-Object -Skip 1).Count
	} else { 0 }
	$trainingPlacementRows = if (Test-Path -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapplacements') {
		@(Get-Content -LiteralPath 'Client\Bin\DataFiles\Map\LV_DEV_TRAINING_GROUND.mapplacements' | Select-Object -Skip 1).Count
	} else { 0 }
	$trainingWorld = Read-Json 'Data\Worlds\LV_DEV_TRAINING_GROUND\Gameplay.world.json'
	$invalidTrainingSpawns = @($trainingWorld.placements | Where-Object {
		$_.kind -ne 'playerSpawn' -or $null -ne $_.archetypeId })
	Add-Check 'maps.training-area-contract' (
		$trainingArea.Count -eq 1 -and
		$trainingArea[0].assetCount -eq 10 -and
		$trainingArea[0].placementCount -eq 18 -and
		$trainingAssetRows -eq 10 -and
		$trainingPlacementRows -eq 18 -and
		@($trainingWorld.placements).Count -eq 4 -and
		$invalidTrainingSpawns.Count -eq 0 -and
		(Test-Path -LiteralPath 'Client\Bin\DataFiles\Navigation\LV_DEV_TRAINING_GROUND.navgrid')) "assets=$trainingAssetRows placements=$trainingPlacementRows spawns=$(@($trainingWorld.placements).Count)"
	Add-Check 'resource.no-lol-annie' (
		-not (Test-Path -LiteralPath 'Client\Bin\Resources\Map\LoL\Annie')) 'legacy Annie resources are quarantined outside the repository'

	$mapPublishPassed = $false
	$mapPublishDetail = ''
	$mapFixtureRoot = Join-Path $repoRoot ".codex_tmp\MapPublishFixture-$PID"
	try {
		$mapFixtureAuthoring = Join-Path $mapFixtureRoot 'Data\Maps\Authoring\FIXTURE'
		$mapFixtureRuntime = Join-Path $mapFixtureRoot 'Client\Bin\DataFiles\Map'
		[IO.Directory]::CreateDirectory($mapFixtureAuthoring) | Out-Null
		[IO.Directory]::CreateDirectory($mapFixtureRuntime) | Out-Null
		$fixtureUtf8 = [Text.UTF8Encoding]::new($false)
		$rowA = '1 "source:a" "A" "editor" "ASSET_A" 0 0 0 0 0 0 1 1 1 1 1'
		$rowB = '2 "source:b" "B" "editor" "ASSET_B" 0 0 0 0 0 0 1 1 1 1 1'
		$rowNew = '3 "source:new" "A" "editor" "ASSET_A" 1 0 0 0 0 0 1 1 1 1 1'
		[IO.File]::WriteAllLines((Join-Path $mapFixtureRuntime 'FIXTURE.mapset'), @(
			'LOSTARK_MAP_SHARD_SET 1 "FIXTURE" 2',
			'"A" "A.mapassets" "A.mapplacements" 1 1',
			'"B" "B.mapassets" "B.mapplacements" 1 1'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureRuntime 'A.mapassets'), @(
			'LOSTARK_MAP_ASSET_CATALOG 4 "FIXTURE" 1', '"ASSET_A" placeholder'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureRuntime 'B.mapassets'), @(
			'LOSTARK_MAP_ASSET_CATALOG 4 "FIXTURE" 1', '"ASSET_B" placeholder'), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureRuntime 'A.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1', $rowA), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureRuntime 'B.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1', $rowB), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowNew), $fixtureUtf8)

		& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 `
			-AreaId FIXTURE `
			-ProjectRoot $mapFixtureRoot | Out-Null
		$publishedMapSet = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'FIXTURE.mapset'))
		$publishedShardA = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'A.mapplacements'))
		$publishedShardB = [IO.File]::ReadAllText((Join-Path $mapFixtureRuntime 'B.mapplacements'))
		$staleMapPublish = @(Get-ChildItem -LiteralPath $mapFixtureRuntime -Force |
			Where-Object Name -Match 'staging|rollback')
		$baselineRuntimeHashes = @{}
		foreach ($runtimeName in @('A.mapplacements', 'B.mapplacements', 'FIXTURE.mapset')) {
			$baselineRuntimeHashes[$runtimeName] =
				(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime $runtimeName)).Hash
		}
		$rowChanged = '3 "source:new" "A" "editor" "ASSET_A" 2 0 0 0 0 0 1 1 1 1 1'
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowChanged), $fixtureUtf8)
		$rollbackRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 `
				-AreaId FIXTURE `
				-ProjectRoot $mapFixtureRoot `
				-FailureAfterPromote 1 | Out-Null
		}
		catch { $rollbackRejected = $true }
		$rollbackPreserved = $true
		foreach ($runtimeName in $baselineRuntimeHashes.Keys) {
			if ($baselineRuntimeHashes[$runtimeName] -ne
				(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime $runtimeName)).Hash) {
				$rollbackPreserved = $false
			}
		}
		$rollbackStale = @(Get-ChildItem -LiteralPath $mapFixtureRuntime -Force |
			Where-Object Name -Match 'staging|rollback')
		$baselineRuntimeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime 'A.mapplacements')).Hash
		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, ($rowNew + ' EXTRA')), $fixtureUtf8)
		$malformedRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 -AreaId FIXTURE -ProjectRoot $mapFixtureRoot | Out-Null
		}
		catch { $malformedRejected = $true }
		$malformedPreservedRuntime = $baselineRuntimeHash -eq
			(Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $mapFixtureRuntime 'A.mapplacements')).Hash

		[IO.File]::WriteAllLines((Join-Path $mapFixtureAuthoring 'FIXTURE.mapplacements'), @(
			'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 3', $rowA, $rowB, $rowNew), $fixtureUtf8)
		[IO.File]::WriteAllLines((Join-Path $mapFixtureRuntime 'FIXTURE.mapset'), @(
			'LOSTARK_MAP_SHARD_SET 1 "FIXTURE" 2',
			'"A" "..\outside.mapassets" "A.mapplacements" 1 2',
			'"B" "B.mapassets" "B.mapplacements" 1 1'), $fixtureUtf8)
		$escapeRejected = $false
		try {
			& .\Tools\MapPipeline\Publish-MapAuthoring.ps1 -AreaId FIXTURE -ProjectRoot $mapFixtureRoot | Out-Null
		}
		catch { $escapeRejected = $true }
		$mapPublishPassed =
			$publishedMapSet -match '"A" "A\.mapassets" "A\.mapplacements" 1 2' -and
			$publishedMapSet -match '"B" "B\.mapassets" "B\.mapplacements" 1 1' -and
			$publishedShardA -match 'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 2' -and
			$publishedShardB -match 'LOSTARK_MAP_PLACEMENTS 2 "FIXTURE" 1' -and
			$staleMapPublish.Count -eq 0 -and
			$rollbackRejected -and $rollbackPreserved -and
			$rollbackStale.Count -eq 0 -and
			$malformedRejected -and $malformedPreservedRuntime -and
			$escapeRejected
		$mapPublishDetail = "stale=$($staleMapPublish.Count) rollbackRejected=$rollbackRejected rollbackPreserved=$rollbackPreserved malformedRejected=$malformedRejected escapeRejected=$escapeRejected"
	}
	catch {
		$mapPublishDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($mapFixtureRoot)) {
			Remove-Item -LiteralPath $mapFixtureRoot -Recurse -Force
		}
	}
	Add-Check 'maps.sharded-authoring-publish' $mapPublishPassed $mapPublishDetail

	$agentsGuide = Get-Content -LiteralPath 'AGENTS.md' -Raw -Encoding utf8
	$claudeGuide = Get-Content -LiteralPath 'CLAUDE.md' -Raw -Encoding utf8
	$teamReadme = Get-Content -LiteralPath '.md\TEAM\README.md' -Raw -Encoding utf8
	$teamGameplayHandbook = Get-Content -LiteralPath '.md\TEAM\TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md' -Raw -Encoding utf8
	$verticalSliceContractMarker =
		'team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions'
	Add-Check 'team.vertical-slice-ownership' (
		$agentsGuide.Contains($verticalSliceContractMarker) -and
		$claudeGuide.Contains($verticalSliceContractMarker) -and
		$teamReadme.Contains($verticalSliceContractMarker) -and
		$teamGameplayHandbook.Contains($verticalSliceContractMarker)) 'team roles define authority interfaces while feature owners may implement required Data/Shared/Server/Client/harness slices'

    $clientItems = Get-ProjectItems 'Client\Default\Client.vcxproj'
    $engineItems = Get-ProjectItems 'Engine\Default\Engine.vcxproj'
    $serverItems = Get-ProjectItems 'Server\Default\Server.vcxproj'
    $missingProjectItems = [Collections.Generic.List[string]]::new()
    foreach ($pair in @(
        @('Client\Default', $clientItems),
        @('Engine\Default', $engineItems),
        @('Server\Default', $serverItems))) {
        $projectDirectory = [IO.Path]::GetFullPath($pair[0])
        foreach ($item in $pair[1]) {
            $fullName = [IO.Path]::GetFullPath((Join-Path $projectDirectory $item.Include))
            if (-not (Test-Path -LiteralPath $fullName)) {
                $missingProjectItems.Add($fullName)
            }
        }
    }
    Add-Check 'projects.registered-files-exist' ($missingProjectItems.Count -eq 0) "missing=$($missingProjectItems.Count)"

    [xml]$clientFilters = Get-Content -LiteralPath 'Client\Default\Client.vcxproj.filters'
    [xml]$engineFilters = Get-Content -LiteralPath 'Engine\Default\Engine.vcxproj.filters'
    [xml]$serverFilters = Get-Content -LiteralPath 'Server\Default\Server.vcxproj.filters'
    Add-Check 'projects.filters-xml' ($null -ne $clientFilters.Project -and $null -ne $engineFilters.Project -and $null -ne $serverFilters.Project) 'XML parsed'

	$expectedDataIncludes = @(& git ls-files --cached --others --exclude-standard -- Data |
		ForEach-Object { '..\..\' + $_.Replace('/', '\') } | Sort-Object)
	$clientDataItems = @($clientItems | Where-Object Include -Like '..\..\Data\*')
	$actualDataIncludes = @($clientDataItems | ForEach-Object Include | Sort-Object)
	$filterManager = [Xml.XmlNamespaceManager]::new($clientFilters.NameTable)
	$filterManager.AddNamespace('m', 'http://schemas.microsoft.com/developer/msbuild/2003')
	$clientDataFilterItems = @($clientFilters.SelectNodes('//m:None', $filterManager) |
		Where-Object Include -Like '..\..\Data\*')
	$invalidDataFilters = @($clientDataFilterItems | Where-Object {
		$null -eq $_.Filter -or -not $_.Filter.StartsWith('96.DataFiles') })
	Add-Check 'projects.data-source-visibility' (
		$clientDataItems.Count -eq $expectedDataIncludes.Count -and
		($actualDataIncludes -join "`n") -eq ($expectedDataIncludes -join "`n") -and
		@($clientDataItems | Where-Object LocalName -ne 'None').Count -eq 0 -and
		$clientDataFilterItems.Count -eq $expectedDataIncludes.Count -and
		$invalidDataFilters.Count -eq 0) "expected=$($expectedDataIncludes.Count) project=$($clientDataItems.Count) filters=$($clientDataFilterItems.Count)"

    $activeSourceRoots = @('Client\Private', 'Client\Public', 'Engine\Private', 'Engine\Public')
    $activeFiles = @(Get-ChildItem -Path $activeSourceRoots -Recurse -File |
        Where-Object Extension -in @('.cpp', '.h'))
    $clientSourceFiles = @(Get-ChildItem -Path @('Client\Private', 'Client\Public') -Recurse -File |
        Where-Object Extension -in @('.cpp', '.h'))
    $forbiddenSymbols = 'CCookedModel|CBinaryAssetObject|\bCMonster\b|CVIBuffer_(Cube|Terrain|Instance_Point|Instance_Rect)|LEVEL_(ASSET_TEST|TEST_LEVEL2|BAREN|GAMEPLAY|MAINMENU|LOGO|CHAOS)'
    $legacyHits = @($activeFiles | Select-String -Pattern $forbiddenSymbols -CaseSensitive)
    Add-Check 'source.legacy-symbols' ($legacyHits.Count -eq 0) "hits=$($legacyHits.Count)"
	Add-Check 'source.no-course-sample-root' (
		-not (Test-Path -LiteralPath 'astar') -and
		-not (Test-Path -LiteralPath '99.수업') -and
		-not (Test-Path -LiteralPath '_work\resource-layout-backup\legacy-map-packs\dev-lol-annie') -and
		-not (Test-Path -LiteralPath 'Data\Effects\Editor\IntakeTest')) 'course and legacy smoke samples are quarantined outside the repository'

    $wrapperHits = @($activeFiles | Select-String -Pattern 'Resources[\\/]LostArk')
    Add-Check 'source.resource-wrapper' ($wrapperHits.Count -eq 0) "hits=$($wrapperHits.Count)"

    $effectCookSource = Get-Content -LiteralPath 'Tools\EffectResourceIntake\Cook-SelectedEffectAsset.ps1' -Raw
    $legacyEffectCookHits = @($effectCookSource | Select-String -Pattern 'WintersAssetConverter|\.wmesh|\.wmat|CCookedModel' -AllMatches)
    Add-Check 'effect.no-legacy-mesh-cook' ($legacyEffectCookHits.Count -eq 0) "hits=$($legacyEffectCookHits.Count)"

    $legacyLaunchHits = @($clientSourceFiles | Select-String -Pattern '--network-lobby|--map-level|--hdr-readback|world\.bern\.smoke|world\.valtan\.(smoke|tool)')
    Add-Check 'source.canonical-launch-options' ($legacyLaunchHits.Count -eq 0) "legacyAliases=$($legacyLaunchHits.Count)"

    $readyGaraHits = @($activeFiles | Select-String -Pattern 'Ready_Gara')
    Add-Check 'source.runtime-authoring' ($readyGaraHits.Count -eq 0) "Ready_Gara hits=$($readyGaraHits.Count)"

    $activeCfgFiles = @(Get-ChildItem -Path @('Data', 'Client\Private', 'Client\Public') -Recurse -File -Force |
        Where-Object { $_.Extension.ToLowerInvariant() -eq '.cfg' })
    $cfgReaderHits = @($activeFiles | Select-String -Pattern '\.cfg\b')
    Add-Check 'data.json-only' ($activeCfgFiles.Count -eq 0 -and $cfgReaderHits.Count -eq 0) "cfgFiles=$($activeCfgFiles.Count) readers=$($cfgReaderHits.Count)"

    $animationFiles = @(Get-ChildItem -LiteralPath 'Data\Animation' -Recurse -File -Force)
    $animationLegacyFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\DataFiles\Anim' -File -Force -ErrorAction SilentlyContinue)
    $animationLegacyHits = @($activeFiles | Select-String -Pattern 'DataFiles[\\/]Anim')
    Add-Check 'animation.data-boundary' ($animationFiles.Count -gt 0 -and $animationLegacyFiles.Count -eq 0 -and $animationLegacyHits.Count -eq 0) "data=$($animationFiles.Count) legacyFiles=$($animationLegacyFiles.Count) legacyRefs=$($animationLegacyHits.Count)"

    $functionKeyHits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F([2-5]|[7-9]|1[0-2])\b')
    $f1Hits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F1\b')
    $f6Hits = @($clientSourceFiles | Select-String -Pattern '(VK|DIK)_F6\b')
    Add-Check 'input.official-function-keys' (
		$functionKeyHits.Count -eq 0 -and
		$f1Hits.Count -eq 1 -and
		$f6Hits.Count -eq 1) "forbidden=$($functionKeyHits.Count) f1=$($f1Hits.Count) f6=$($f6Hits.Count)"

	$lanceLogicSource = Get-Content -LiteralPath 'Client\Private\Logic_LanceMaster.cpp' -Raw
	$characterSource = Get-Content -LiteralPath 'Client\Private\Character.cpp' -Raw
	$modelSource = Get-Content -LiteralPath 'Engine\Private\Model.cpp' -Raw
	$boneSource = Get-Content -LiteralPath 'Engine\Private\Bone.cpp' -Raw
	$obsoletePrRuntimeFiles = @(
		'Client\Private\Level_Test2.cpp',
		'Client\Private\SkillData.cpp',
		'Client\Public\SkillData.h')
	$presentObsoletePrRuntimeFiles = @($obsoletePrRuntimeFiles |
		Where-Object { Test-Path -LiteralPath $_ })
	Add-Check 'integration.pr34-pr35-reconciliation' (
		$presentObsoletePrRuntimeFiles.Count -eq 0 -and
		$lanceLogicSource -notmatch 'Get_DIKeyState|DIK_|Play_Skill' -and
		$characterSource -match 'CLIP_BLEND_SECONDS' -and
		$modelSource -match 'Begin_AnimBlend' -and
		$boneSource -match 'Blend_TransformationMatrix') "obsolete=$($presentObsoletePrRuntimeFiles -join ',') localInput=$($lanceLogicSource -match 'Get_DIKeyState|DIK_|Play_Skill') crossfade=$($characterSource -match 'CLIP_BLEND_SECONDS')"

    $changeLevelHits = @($clientSourceFiles | Select-String -Pattern 'Change_Level\(')
    $unexpectedChangeLevelHits = @($changeLevelHits | Where-Object {
        $_.Path -notlike '*Client\Private\MainApp.cpp' -and
        $_.Path -notlike '*Client\Private\Level_Loading.cpp' })
    Add-Check 'levels.transition-boundary' ($changeLevelHits.Count -eq 3 -and $unexpectedChangeLevelHits.Count -eq 0) "calls=$($changeLevelHits.Count) unexpected=$($unexpectedChangeLevelHits.Count)"

    $mainAppSource = Get-Content -LiteralPath 'Client\Private\MainApp.cpp' -Raw
    Add-Check 'levels.lobby-start' ($mainAppSource -match 'Start_Level\(LEVEL::LOBBY\)') 'MainApp starts Lobby'
	$lobbySource = Get-Content -LiteralPath 'Client\Private\Level_Lobby.cpp' -Raw
	$clientEntrySource = Get-Content -LiteralPath 'Client\Default\Client.cpp' -Raw
	Add-Check 'levels.lobby-release-entry-ui' (
		$mainAppSource -match 'HRESULT CMainApp::ReadyImGuiRuntime\(\)' -and
		$mainAppSource -match 'if \(FAILED\(ReadyImGuiRuntime\(\)\)\)' -and
		$lobbySource -match 'Render_CharacterSelectPanel\(\);' -and
		$lobbySource -notmatch '#ifdef _DEBUG\s*\r?\n\s*void CLevel_Lobby::Render_CharacterSelectPanel' -and
		$clientEntrySource -match 'CImGuiLayer::HandleWindowMessage' -and
		$clientEntrySource -notmatch '#ifdef _DEBUG\s*\r?\n\s*if \(CImGuiLayer::HandleWindowMessage') 'Lobby entry UI and ImGui input runtime are compiled in Release'
	$launchOptionsHeader = Get-Content -LiteralPath 'Client\Public\ClientLaunchOptions.h' -Raw
	$loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$replicationHeader = Get-Content -LiteralPath 'Client\Public\ClientReplication.h' -Raw
	$replicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
	$offlinePreviewHeader = Get-Content -LiteralPath 'Client\Public\OfflinePlayerPreview.h' -Raw
	$offlinePreviewSource = Get-Content -LiteralPath 'Client\Private\OfflinePlayerPreview.cpp' -Raw
	$bernLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Bern.cpp' -Raw
	$valtanLevelSource = Get-Content -LiteralPath 'Client\Private\Level_ValtanArena.cpp' -Raw
	$developmentLevelSource = Get-Content -LiteralPath 'Client\Private\Level_Development.cpp' -Raw
	$clientProjectText = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
	$clientFilterText = Get-Content -LiteralPath 'Client\Default\Client.vcxproj.filters' -Raw
	$networkSinkHeader = Get-Content -LiteralPath 'Client\Public\NetworkPlayerCommandSink.h' -Raw
	$offlineSmokeSource = Get-Content -LiteralPath 'Tools\Build\Invoke-OfflineClientSmoke.ps1' -Raw
	Add-Check 'levels.offline-stage-entry' (
		$lobbySource -match 'lobby\.local-preview' -and
		$lobbySource -match 'Local Preview' -and
		$lobbySource -match 'Multiplayer' -and
		$lobbySource -match 'Connect_To_Server\(' -and
		$lobbySource -notmatch 'Set_OfflinePreview' -and
		$lobbySource -match 'SetNextWindowViewport\(mainViewport->ID\)' -and
		$lobbySource -match 'ImGuiWindowFlags_NoSavedSettings' -and
		$launchOptionsHeader -match 'SelectedCharacterClass' -and
		$launchOptionsHeader -match 'CLIENT_ENTRY_MODE' -and
		$launchOptionsHeader -match 'eEntryMode = CLIENT_ENTRY_MODE::LOCAL_PREVIEW' -and
		$launchOptionsHeader -match 'strServerHost' -and
		$launchOptionsHeader -match 'isOfflinePreview' -and
		$loaderSource -match 'isOfflinePreview' -and
		$loaderSource -match 'SelectedCharacterClass' -and
		$replicationHeader -match 'LOCAL_PREVIEW_PLAYER_DESC' -and
		$replicationHeader -match 'm_LocalPreviewCharacter' -and
		$replicationSource -match 'Create_Character' -and
		$replicationSource -match 'Spawn_LocalPreview' -and
		$offlinePreviewSource -match 'CProjectDataRoot::Resolve' -and
		$offlinePreviewSource -match 'CWorldGameplayDocument' -and
		$offlinePreviewSource -match 'WORLD_PLACEMENT_KIND::PLAYER_SPAWN' -and
		$offlinePreviewSource -match 'placementId <' -and
		$offlinePreviewSource -notmatch 'S2C_PLAYER_SPAWNED|PLAYER_ID|NET_ENTITY_ID|CNetObjectRegistry' -and
		$bernLevelSource -match 'COfflinePlayerPreview::Spawn' -and
		$valtanLevelSource -match 'COfflinePlayerPreview::Spawn' -and
		$developmentLevelSource -match 'COfflinePlayerPreview::Spawn' -and
		$clientProjectText -match 'OfflinePlayerPreview\.cpp' -and
		$clientProjectText -match 'OfflinePlayerPreview\.h' -and
		$clientFilterText -match 'OfflinePlayerPreview\.cpp[\s\S]{0,80}<Filter>01\. Levels</Filter>' -and
		$networkSinkHeader -match 'Get_LiveInstanceCount' -and
		$mainAppSource -match 'Get_LiveInstanceCount\(\)' -and
		$mainAppSource -match 'networkCommandSinkLiveCount' -and
		$offlineSmokeSource -match 'networkCommandSinkLiveCount -ne 0' -and
		$mainAppSource -match 'offline-stage-local-player-and-camera-ready') 'explicit local preview creates a non-network Character from canonical world playerSpawn and validates camera binding and command-sink absence'
	$networkManagerSource = Get-Content -LiteralPath 'Client\Private\NetworkManager.cpp' -Raw
	$serverMainSource = Get-Content -LiteralPath 'Server\Private\Main.cpp' -Raw
	$tcpListenerSource = Get-Content -LiteralPath 'Server\Private\TcpListener.cpp' -Raw
	$networkEndpointSmokeSource = Get-Content -LiteralPath 'Tools\Build\Invoke-NetworkEndpointSmoke.ps1' -Raw
	Add-Check 'network.explicit-mode-and-bind' (
		$networkManagerSource -match 'Connect_To_Server\(' -and
		$networkManagerSource -match 'InetPtonA' -and
		$networkManagerSource -match 'WSAETIMEDOUT' -and
		$networkManagerSource -match 'Receive_Loop\(const SOCKET serverSocket\)' -and
		$networkManagerSource -match '::recv\([\s\S]{0,80}serverSocket' -and
		$networkManagerSource -notmatch '::recv\([\s\S]{0,80}m_hServerSocket' -and
		$networkManagerSource -match '::closesocket\(socketToClose\)[\s\S]*m_ReceiveThread\.join\(\)' -and
		$serverMainSource -match '--bind-address' -and
		$tcpListenerSource -match 'INADDR_ANY' -and
		$tcpListenerSource -match 'INADDR_LOOPBACK' -and
		$replicationHeader -match 'Has_PendingConnectionLoss' -and
		$replicationHeader -match 'Acknowledge_ConnectionLoss' -and
		$bernLevelSource -match 'network\.connection-lost' -and
		$valtanLevelSource -match 'network\.connection-lost' -and
		$developmentLevelSource -match 'network\.connection-lost' -and
		$mainAppSource -match 'network-disconnect-returned-to-lobby' -and
		$lobbySource -match 'm_isAwaitingEnterAcceptance' -and
		$lobbySource -match 'steady_clock::now' -and
		$lobbySource -match 'seconds\(5\)' -and
		$mainAppSource -match 'network-enter-approval-timeout-returned-to-lobby' -and
		$mainAppSource -match 'requestedScenarioId' -and
		$networkEndpointSmokeSource -match "LocalAddress -eq '0\.0\.0\.0'" -and
		$networkEndpointSmokeSource -match 'OwningProcess -eq \$Process\.Id' -and
		$networkEndpointSmokeSource -match 'expect-disconnect-recovery' -and
		$networkEndpointSmokeSource -match 'expect-enter-approval-timeout' -and
		$networkEndpointSmokeSource -match 'TcpListener') 'Client selects an IPv4 endpoint explicitly; Server opt-in LAN binding, approval timeout, and disconnect-to-Lobby recovery have executable smoke contracts'

    $playerControllerSource = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
    Add-Check 'player.command-sink-boundary' ($playerControllerSource -notmatch 'NetworkManager' -and $playerControllerSource -match 'IPlayerCommandSink') 'PlayerController depends on command sink'

    $characterLogicFiles = @(Get-ChildItem -LiteralPath 'Client\Private' -Filter 'Logic_*.cpp' -File)
    $characterLogicBoundaryHits = @($characterLogicFiles | Select-String -Pattern 'Get_DIKey|Get_DIMouse|NetworkManager|Play_Skill\(')
    $characterSpecSource = Get-Content -LiteralPath 'Client\Public\CharacterSpec.h' -Raw
    Add-Check 'character.presentation-boundary' (
        $characterLogicBoundaryHits.Count -eq 0 -and
        $characterSpecSource -match 'Update_Presentation') "forbiddenCalls=$($characterLogicBoundaryHits.Count)"

    $effectRuntimeSource = Get-Content -LiteralPath 'Client\Private\Effect_Runtime.cpp' -Raw
    $effectToolSource = Get-Content -LiteralPath 'Client\Private\Effect_Tool.cpp' -Raw
    Add-Check 'effect.resource-confinement' ($effectRuntimeSource -notmatch 'RequestedPath\.is_absolute\(\)\s*&&' -and $effectToolSource -notmatch 'Candidates\[\][\s\S]{0,120}RequestedPath') 'absolute path candidates rejected'

    $materialReaderSource = Get-Content -LiteralPath 'Engine\Private\BinaryAsset\Winters\WMaterialReader.cpp' -Raw
    $modelSource = Get-Content -LiteralPath 'Engine\Private\Model.cpp' -Raw
    Add-Check 'model.resource-confinement' (
        $materialReaderSource -match 'IsBelowRoot' -and
        $materialReaderSource -notmatch 'Resources/LostArk' -and
        $materialReaderSource -notmatch 'filesystem::exists\(normalizedPath\)' -and
        $modelSource -notmatch 'L"LostArk"') 'material textures remain below the flat Resources root'
	$loaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$loaderFactoryFiles = @(
		'Engine\Private\Shader.cpp',
		'Engine\Private\Model.cpp',
		'Engine\Private\Navigation.cpp',
		'Client\Private\Camera_Free.cpp',
		'Client\Private\Character.cpp',
		'Client\Private\Part_Body.cpp',
		'Client\Private\Part_Equipment.cpp',
		'Client\Private\Valtan.cpp',
		'Client\Private\Body_Valtan.cpp')
	$loaderFactoryModalHits = @($loaderFactoryFiles |
		ForEach-Object { Select-String -LiteralPath $_ -Pattern 'MSG_BOX|MessageBox' })
    Add-Check 'loader.no-worker-modal' (
		$loaderFactoryModalHits.Count -eq 0) "factoryModalHits=$($loaderFactoryModalHits.Count)"
	Add-Check 'loader.no-thread-termination' (
		$loaderSource -notmatch 'TerminateThread' -and
		$loaderSource -match 'TerminateProcess\(GetCurrentProcess\(\), ERROR_TIMEOUT\)') 'cooperative cancellation escalates to bounded process fail-fast, never thread termination'

    $regressionHarnessSource = Get-Content -LiteralPath 'Tools\Build\Invoke-BuildAndRegression.ps1' -Raw
    Add-Check 'harness.runtime-working-directory' (
        $regressionHarnessSource -match '\$clientWorkingDirectory = Join-Path \$repoRoot ''Client\\Default''' -and
        $regressionHarnessSource -match 'Assert-ClientRuntimeLayout' -and
        $regressionHarnessSource -match 'finally') 'regression harness pins runtime roots and process cleanup'
	Add-Check 'harness.server-gameplay-contract' (
		$regressionHarnessSource -match '--contract-test' -and
		(Test-Path -LiteralPath 'Server\Private\ServerGameplayContractTests.cpp')) 'regression runs server skill, damage, boss, and navigation contracts'
	Add-Check 'harness.playable-class-runtime-matrix' (
		$regressionHarnessSource -match "'lance-master', 'gunslinger', 'slayer', 'artist'" -and
		$regressionHarnessSource -match '--character-class=\$CharacterClass') 'training smoke decodes and enters with each supported playable class'

    $missingHarnessScenarios = @($requiredScenarios | Where-Object {
        $regressionHarnessSource -notmatch ([regex]::Escape("'$_'")) })
    Add-Check 'harness.scenario-coverage' ($missingHarnessScenarios.Count -eq 0) "missing=$($missingHarnessScenarios -join ',')"

    $characterCatalog = Read-Json 'Data\Actors\CharacterCatalog.json'
    $bossCatalog = Read-Json 'Data\Actors\BossCatalog.json'
    $missingActorAssets = [Collections.Generic.List[string]]::new()
    foreach ($character in @($characterCatalog.characters | Where-Object runtimeStatus -eq 'supported')) {
		foreach ($assetPath in @($character.bodyModel, $character.weaponModel) + @($character.equipmentModels)) {
			if ($assetPath -and -not (Test-Path -LiteralPath (Join-Path $resourceRoot $assetPath))) {
				$missingActorAssets.Add($assetPath)
			}
        }
    }
    foreach ($boss in @($bossCatalog.bosses)) {
        foreach ($assetPath in @($boss.bodyModel, $boss.weaponModel)) {
            if ($assetPath -and -not (Test-Path -LiteralPath (Join-Path $resourceRoot $assetPath))) {
                $missingActorAssets.Add($assetPath)
            }
        }
    }
    Add-Check 'actors.catalog-assets' ($missingActorAssets.Count -eq 0) "missing=$($missingActorAssets -join ',')"
	$lobbySource = Get-Content -LiteralPath 'Client\Private\Level_Lobby.cpp' -Raw
	$playableRoster = @($characterCatalog.characters | ForEach-Object networkClassId) -join ','
	$rosterStatus = @($characterCatalog.characters | ForEach-Object runtimeStatus) -join ','
	Add-Check 'actors.playable-roster' (
		$playableRoster -eq 'LANCE_MASTER,GUNSLINGER,SLAYER,ARTIST' -and
		$rosterStatus -eq 'supported,supported,supported,supported' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::LANCE_MASTER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::GUNSLINGER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::SLAYER' -and
		$lobbySource -match 'CHARACTER_CLASS_ID::ARTIST' -and
		$lobbySource -notmatch 'CHARACTER_CLASS_ID::DESTROYER' -and
		$lobbySource -match 'Is_Supported_Playable_Character_Class') "roster=$playableRoster status=$rosterStatus"
	$hudViewModelHeader = Get-Content -LiteralPath 'Client\Public\CombatHUDViewModel.h' -Raw
	$hudViewModelSource = Get-Content -LiteralPath 'Client\Private\CombatHUDViewModel.cpp' -Raw
	Add-Check 'hud.selected-class-boundary' (
		$hudViewModelHeader -match 'HUD_PLAYER_STATE[\s\S]{0,300}eCharacterClass' -and
		$hudViewModelSource -match 'definition\.eCharacterClass != characterClass' -and
		$mainAppSource -match 'RenderCombatHUD\(\);') 'runtime HUD carries local class and filters skill definitions by class'
	$actorCatalogSource = Get-Content -LiteralPath 'Client\Private\ActorCatalog.cpp' -Raw
	$actorLoaderSource = Get-Content -LiteralPath 'Client\Private\Loader.cpp' -Raw
	$playableAssetServiceSource = Get-Content -LiteralPath 'Client\Private\PlayableCharacterAssetService.cpp' -Raw
	$replicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
	$hardcodedActorModelHits = @(($actorLoaderSource + $playableAssetServiceSource) |
		Select-String -AllMatches -Pattern 'Character/[A-Za-z0-9_./-]+\.wmodel')
	Add-Check 'actors.runtime-catalog-boundary' (
		$actorCatalogSource -match 'Actors/CharacterCatalog\.json' -and
		$actorCatalogSource -match 'Actors/BossCatalog\.json' -and
		$playableAssetServiceSource -match 'CActorCatalog::Find_Character' -and
		$actorLoaderSource -match 'CActorCatalog::Find_Boss' -and
		$replicationSource -match 'CActorCatalog::Find_Boss\(spawned\.strArchetypeId\)' -and
		$hardcodedActorModelHits.Count -eq 0) "hardcodedModelPaths=$($hardcodedActorModelHits.Count)"
	Add-Check 'actors.selected-first-on-demand-load' (
		$actorLoaderSource -match 'Get_LocalCharacterClass\(\)' -and
		$actorLoaderSource -match 'CPlayableCharacterAssetService::Begin_LevelLoad' -and
		$actorLoaderSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$replicationSource -match 'CPlayableCharacterAssetService::Ensure_Prototypes' -and
		$playableAssetServiceSource -match 'g_ReadyClassesByLevel') 'loader admits selected class; replication admits a remote class once'

    $worldValidationPassed = $true
    $worldValidationDetail = ''
    try {
        $worldValidationDetail = (& .\Tools\WorldPipeline\Publish-WorldGameplay.ps1 -Mode Validate 2>&1) -join ' '
    }
    catch {
        $worldValidationPassed = $false
        $worldValidationDetail = $_.Exception.Message
    }
    Add-Check 'world.publish-contract' $worldValidationPassed $worldValidationDetail

	$gameplayValidationPassed = $true
	$gameplayValidationDetail = ''
	try {
		$gameplayValidationDetail = (& .\Tools\GameplayPipeline\Publish-GameplayBalance.ps1 -Mode Validate 2>&1) -join ' '
	}
	catch {
		$gameplayValidationPassed = $false
		$gameplayValidationDetail = $_.Exception.Message
	}
	Add-Check 'gameplay.balance-publish-contract' $gameplayValidationPassed $gameplayValidationDetail

	$navigationValidationPassed = $true
	$navigationValidationDetail = ''
	try {
		$navigationValidationDetail = (& .\Tools\NavigationPipeline\Publish-ServerNavigation.ps1 -Mode Validate 2>&1) -join ' '
	}
	catch {
		$navigationValidationPassed = $false
		$navigationValidationDetail = $_.Exception.Message
	}
	Add-Check 'navigation.server-publish-contract' $navigationValidationPassed $navigationValidationDetail

	$worldRollbackPassed = $false
	$worldRollbackDetail = ''
	$worldFixtureRelative = ".codex_tmp/ProjectAuditWorldPublish-$PID"
	$worldFixturePath = Join-Path $repoRoot $worldFixtureRelative
	try {
		[IO.Directory]::CreateDirectory($worldFixturePath) | Out-Null
		$bernFixture = Join-Path $worldFixturePath 'BERN.worldbootstrap'
		$valtanFixture = Join-Path $worldFixturePath 'VALTAN_ARENA.worldbootstrap'
		$trainingFixture = Join-Path $worldFixturePath 'TRAINING_GROUND.worldbootstrap'
		[IO.File]::WriteAllText($bernFixture, 'original-bern')
		[IO.File]::WriteAllText($valtanFixture, 'original-valtan')
		[IO.File]::WriteAllText($trainingFixture, 'original-training')
		try {
			& .\Tools\WorldPipeline\Publish-WorldGameplay.ps1 `
				-Mode Publish `
				-OutputRoot $worldFixtureRelative `
				-FailureAfterPromote 1 | Out-Null
			throw 'Failure injection unexpectedly succeeded.'
		}
		catch {
			if ($_.Exception.Message -eq 'Failure injection unexpectedly succeeded.') {
				throw
			}
		}
		$staleTransactionFiles = @(Get-ChildItem -LiteralPath $worldFixturePath -Force |
			Where-Object Name -Match '^\.staging\.|\.rollback\.')
		$worldRollbackPassed =
			[IO.File]::ReadAllText($bernFixture) -eq 'original-bern' -and
			[IO.File]::ReadAllText($valtanFixture) -eq 'original-valtan' -and
			[IO.File]::ReadAllText($trainingFixture) -eq 'original-training' -and
			$staleTransactionFiles.Count -eq 0
		$worldRollbackDetail = "stale=$($staleTransactionFiles.Count)"
	}
	catch {
		$worldRollbackDetail = $_.Exception.Message
	}
	finally {
		if ([IO.Directory]::Exists($worldFixturePath)) {
			Remove-Item -LiteralPath $worldFixturePath -Recurse -Force
		}
	}
	Add-Check 'world.publish-generation-rollback' $worldRollbackPassed $worldRollbackDetail

    $worldContractFiles = @(
        'Client\Public\WorldGameplayDocument.h',
        'Client\Private\WorldGameplayDocument.cpp',
        'Server\Public\WorldBootstrap.h',
        'Server\Private\WorldBootstrap.cpp',
        'Shared\Public\Network\PacketMessages.h',
        'Tools\WorldPipeline\Publish-WorldGameplay.ps1')
    $monsterContractHits = @($worldContractFiles |
        ForEach-Object { Select-String -LiteralPath $_ -Pattern '\bMONSTER\b|"monster"' })
    $staleWorldPublishFiles = @(Get-ChildItem -LiteralPath 'Server\Bin\DataFiles\World' -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '\.(tmp|rollback)\.' })
    Add-Check 'world.no-legacy-monster-contract' (
        -not (Test-Path -LiteralPath 'Data\Actors\MonsterCatalog.json') -and
        $monsterContractHits.Count -eq 0) "contractHits=$($monsterContractHits.Count) catalogExists=$(Test-Path -LiteralPath 'Data\Actors\MonsterCatalog.json')"
    Add-Check 'world.publish-cleanup' ($staleWorldPublishFiles.Count -eq 0) "stale=$($staleWorldPublishFiles.Name -join ',')"

    $serverRoomSource = Get-Content -LiteralPath 'Server\Private\GameRoom.cpp' -Raw
    $serverProjectSource = Get-Content -LiteralPath 'Server\Default\Server.vcxproj' -Raw
    Add-Check 'server.world-bootstrap-boundary' (
        $serverRoomSource -match 'Find_AvailablePlayerSpawn' -and
        $serverRoomSource -match 'Update_WorldEntities' -and
        $serverRoomSource -match 'iPatternTelegraphMs' -and
        $serverRoomSource -notmatch 'fActionElapsedSeconds >= 0\.8f' -and
        $serverRoomSource -notmatch 'm_Players\.size\(\)\) \* 2\.f' -and
        $serverProjectSource -match 'PublishWorldGameplay') 'MapTool world documents publish before Server compile'

    $packetMessagesSource = Get-Content -LiteralPath 'Shared\Public\Network\PacketMessages.h' -Raw
    $clientReplicationSource = Get-Content -LiteralPath 'Client\Private\ClientReplication.cpp' -Raw
    Add-Check 'server.world-entity-replication' (
        $packetMessagesSource -match 'S2C_WORLD_ENTITY_SPAWNED' -and
        $packetMessagesSource -match 'WORLD_ENTITY_ACTION' -and
		$clientReplicationSource -match 'CActorCatalog::Find_Boss\(spawned\.strArchetypeId\)' -and
        $serverRoomSource -match 'Broadcast_WorldSnapshot' -and
        $serverRoomSource -match 'Update_WorldEntities') 'server owns world entity state; client consumes presentation snapshots'

	$playerControllerSource = Get-Content -LiteralPath 'Client\Private\PlayerController.cpp' -Raw
	$hudViewModelSource = Get-Content -LiteralPath 'Client\Private\CombatHUDViewModel.cpp' -Raw
	Add-Check 'gameplay.command-server-truth' (
		$packetMessagesSource -match 'C2S_USE_SKILL' -and
		$packetMessagesSource -match 'PLAYER_ACTION_STATE' -and
		$serverRoomSource -match 'Handle_UseSkill' -and
		$playerControllerSource -match 'Request_UseSkill' -and
		$playerControllerSource -notmatch 'Play_Skill') 'input emits intent and presentation consumes approved snapshot action'
	Add-Check 'ui.combat-viewmodel-boundary' (
		$hudViewModelSource -match 'Balance/PlayerSkills\.json' -and
		$hudViewModelSource -match 'Balance/DamageProfiles\.json' -and
		$clientReplicationSource -match 'Apply_LocalPlayer' -and
		$clientReplicationSource -match 'Apply_Boss') 'HUD consumes server snapshot plus validated balance definitions without packets'

    $report = [ordered]@{
        schema = 'lostark.project-audit-report'
        formatVersion = 1
        generatedUtc = [DateTime]::UtcNow.ToString('o')
        passed = $failures.Count -eq 0
        checks = $checks
        failures = $failures
    }
    $resolvedReport = [IO.Path]::GetFullPath($ReportPath)
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($resolvedReport)) | Out-Null
    [IO.File]::WriteAllText(
        $resolvedReport,
        ($report | ConvertTo-Json -Depth 8) + [Environment]::NewLine,
        [Text.UTF8Encoding]::new($false))

    if ($failures.Count -gt 0) {
        throw "Project audit failed ($($failures.Count)): $($failures -join '; ')"
    }
    Write-Output "Project audit passed: $($checks.Count) checks. Report: $resolvedReport"
}
catch {
    Write-Error $_
    exit 1
}
