[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateSet('Product', 'Core', 'FullDiagnostic')]
    [string]$Profile = 'Product',
    [string]$MSBuildPath = '',
    [string]$BuildLogDirectory = '',
    [ValidateRange(0, 64)]
    [int]$MaxCompilerProcesses = 0,
    [switch]$SkipBuild,
    [string]$ResourceRoot = '',
    [switch]$AllowLocalEffectResources,
    [ValidatePattern('^(?:[0-9a-f]{64})?$')]
    [string]$ExpectedValtanSourceRevision = ''
)

$ErrorActionPreference = 'Stop'
if ($SkipBuild -and $Profile -ne 'Product') {
    throw '-SkipBuild is supported only for Product. Core/FullDiagnostic require freshly built harness executables and PDBs.'
}
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$includeCore = $Profile -in @('Core', 'FullDiagnostic')
$includeFullDiagnostic = $Profile -eq 'FullDiagnostic'
$clientExe = Join-Path $repoRoot "Client\Bin\$Configuration\Client.exe"
$serverExe = Join-Path $repoRoot "Server\Bin\$Configuration\Server.exe"
$protocolHarnessExe = Join-Path $repoRoot `
    "Tools\NetworkProtocolHarness\Bin\$Configuration\NetworkProtocolHarness.exe"
$characterSelectIsolationHarnessExe = Join-Path $repoRoot `
    "Tools\CharacterSelectIsolationHarness\Bin\$Configuration\CharacterSelectIsolationHarness.exe"
$valtanAuditionServiceHarnessExe = Join-Path $repoRoot `
    "Tools\ValtanPatternAuditionServiceHarness\Bin\$Configuration\ValtanPatternAuditionServiceHarness.exe"
$pointLightFalloffHarnessExe = Join-Path $repoRoot `
    "Tools\PointLightFalloffContractHarness\Bin\$Configuration\PointLightFalloffContractHarness.exe"
$physicsHarnessExe = Join-Path $repoRoot `
    "Tools\PhysicsContractHarness\Bin\$Configuration\PhysicsContractHarness.exe"
$wmodelHarnessExe = Join-Path $repoRoot `
    "Tools\WModelGeometryContractHarness\Bin\$Configuration\WModelGeometryContractHarness.exe"
$runtimeResourceRoot = if ([string]::IsNullOrWhiteSpace($ResourceRoot)) {
    Join-Path $repoRoot 'Client\Bin\Resources'
}
else {
    [IO.Path]::GetFullPath($ResourceRoot)
}
$domainManifestPath = Join-Path $PSScriptRoot 'BuildDomains.json'
$domainPipelinePath = Join-Path $PSScriptRoot 'BuildDomainPipeline.psm1'
$productOutputGuardPath = Join-Path $PSScriptRoot 'ProductOutputGuard.psm1'
$buildReceiptRoot = Join-Path $repoRoot 'out\BuildPipeline\receipts'
$buildEvidenceRoot = Join-Path $repoRoot 'out\BuildPipeline\runs'
$valtanPipeline = Join-Path $repoRoot `
    'Tools\ValtanPipeline\valtan_tuning_pipeline.py'
Import-Module $domainPipelinePath -Force
Import-Module $productOutputGuardPath -Force
Import-Module (Join-Path $PSScriptRoot 'BuildIncrementalDiagnostics.psm1') -Force
$buildDomainManifest = if ($includeCore) { Read-BuildDomainManifest $domainManifestPath } else { $null }
$script:buildStepRecords = [Collections.Generic.List[object]]::new()
$script:buildDomainResults = [Collections.Generic.List[object]]::new()
$script:buildStartedUtc = [DateTime]::UtcNow.ToString('o')
$script:buildRunTimer = [Diagnostics.Stopwatch]::StartNew()
$script:buildStartGitIdentity = $null
$script:buildStartProductSourceInputSha256 = ''
$script:buildToolchain = $null
$script:productCompileEvidencePath = ''
$script:buildFailure = ''
$script:runtimeDataChecks = @()

function Get-ValtanRepositorySourceRevision {
    $manifestText = (& python $valtanPipeline --repository-root $repoRoot `
        source-manifest --repository-only | Out-String).Trim()
    if ($global:LASTEXITCODE -ne 0 -or
        [string]::IsNullOrWhiteSpace($manifestText)) {
        throw 'STALE_REVISION: Valtan exact source revision could not be read.'
    }
    $manifestResult = $manifestText | ConvertFrom-Json
    [string]$revision = $manifestResult.payload.sourceManifestId
    if (-not [bool]$manifestResult.ok -or
        [string]$manifestResult.command -cne 'SOURCE_MANIFEST' -or
        $revision -cnotmatch '^[0-9a-f]{64}$') {
        throw 'STALE_REVISION: Valtan exact source revision result is invalid.'
    }
    return $revision
}

function Assert-ExpectedValtanSourceRevision {
    param([Parameter(Mandatory = $true)][string]$Phase)

    # Keep direct/legacy calls compatible. Run-FullPipeline always supplies the
    # pin and therefore takes the strict branch at every mutation boundary.
    if ([string]::IsNullOrWhiteSpace($ExpectedValtanSourceRevision)) { return }
    [string]$actual = Get-ValtanRepositorySourceRevision
    if ($actual -cne $ExpectedValtanSourceRevision) {
        throw "STALE_REVISION: Valtan source revision mismatch at ${Phase}: expected $ExpectedValtanSourceRevision, actual $actual."
    }
}

function Add-BuildStepRecord {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Result,
        [Parameter(Mandatory = $true)][long]$ElapsedMilliseconds,
        [object]$Details = $null
    )

    $record = [ordered]@{
        name = $Name
        result = $Result
        elapsedMs = $ElapsedMilliseconds
    }
    if ($null -ne $Details) { $record.details = $Details }
    $script:buildStepRecords.Add([pscustomobject]$record) | Out-Null
}

function Write-ProductCompileEvidence {
    param(
        [Parameter(Mandatory = $true)][ValidateSet('PASS', 'FAIL')][string]$Result,
        [string[]]$MissingRuntimeInputs = @(),
        [string[]]$InvalidRuntimeInputs = @()
    )

    $compileResult = [ordered]@{
        schema = 'lostark.compile-result'
        formatVersion = 1
        configuration = $Configuration
        profile = $Profile
        result = $Result
        failure = $script:buildFailure
        skippedBuild = [bool]$SkipBuild
        startedUtc = $script:buildStartedUtc
        elapsedMs = $script:buildRunTimer.ElapsedMilliseconds
        toolchain = $script:buildToolchain
        steps = @($script:buildStepRecords)
        missingRuntimeInputs = @($MissingRuntimeInputs)
        invalidRuntimeInputs = @($InvalidRuntimeInputs)
        runtimeDataChecks = @($script:runtimeDataChecks)
    }
    $stamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
    $path = Join-Path $buildEvidenceRoot "$stamp-$($Configuration.ToLowerInvariant())-product.json"
    Write-BuildAtomicJson $path $compileResult
    $script:productCompileEvidencePath = $path
    Write-Host "Compile result: $path"
}

function Get-InstallationMSBuild {
    param([Parameter(Mandatory = $true)][string]$InstallationPath)

    $candidate = Join-Path $InstallationPath 'MSBuild\Current\Bin\amd64\MSBuild.exe'
    if (Test-Path -LiteralPath $candidate -PathType Leaf) { return $candidate }
    throw "The selected Visual Studio installation has no x64 MSBuild: $InstallationPath"
}

function Resolve-MSBuild {
    $selection = ''
    $candidate = ''
    if (-not [string]::IsNullOrWhiteSpace($MSBuildPath)) {
        $candidate = $MSBuildPath
        $selection = 'MSBuildPath parameter'
    }
    elseif (-not [string]::IsNullOrWhiteSpace($env:MSBUILD_EXE_PATH)) {
        $candidate = $env:MSBUILD_EXE_PATH
        $selection = 'MSBUILD_EXE_PATH environment'
    }
    elseif (-not [string]::IsNullOrWhiteSpace($env:VSINSTALLDIR)) {
        $candidate = Get-InstallationMSBuild $env:VSINSTALLDIR
        $selection = 'VSINSTALLDIR environment'
    }
    else {
        # Preserve a deliberately configured developer PATH. A discovered
        # installation uses its x64 host instead of switching to a fixed VS2022.
        $command = Get-Command msbuild.exe -CommandType Application -ErrorAction SilentlyContinue
        if ($null -ne $command) {
            $candidate = $command.Source
            $selection = 'PATH'
        }
        else {
            $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
            if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
                throw 'MSBuild was not found. Install Visual Studio C++ build tools or supply -MSBuildPath.'
            }
            $installationsText = (& $vswhere -all -products '*' -prerelease `
                -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json -utf8 | Out-String)
            if ($global:LASTEXITCODE -ne 0) { throw 'Visual Studio discovery failed (vswhere).' }
            $installations = $installationsText | ConvertFrom-Json
            $installation = $installations | Where-Object { $_.isComplete -and $_.isLaunchable } |
                Sort-Object @{ Expression = { [version]$_.installationVersion }; Descending = $true }, `
                    @{ Expression = { [string]$_.installationPath }; Descending = $false } |
                Select-Object -First 1
            if ($null -eq $installation) {
                throw 'No complete Visual Studio C++ installation was found. Supply -MSBuildPath to select an installed toolchain.'
            }
            $candidate = Get-InstallationMSBuild $installation.installationPath
            $selection = 'vswhere latest complete C++ installation (including Preview)'
        }
    }

    $candidate = [IO.Path]::GetFullPath([Environment]::ExpandEnvironmentVariables($candidate))
    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        throw "Selected MSBuild does not exist ($selection): $candidate"
    }
    # Explicit executable selections remain exact. The compiler and SDK host
    # architecture are pinned separately for every project invocation below.
    $version = [Diagnostics.FileVersionInfo]::GetVersionInfo($candidate)
    $installationRoot = ''
    $marker = $candidate.IndexOf('\MSBuild\', [StringComparison]::OrdinalIgnoreCase)
    if ($marker -ge 0) { $installationRoot = $candidate.Substring(0, $marker) }
    $script:buildToolchain = [pscustomobject][ordered]@{
        selection = $selection
        msbuildPath = $candidate
        msbuildFileVersion = $version.FileVersion
        msbuildProductVersion = $version.ProductVersion
        visualStudioInstallation = $installationRoot
        preferredToolArchitecture = 'x64'
        windowsSdkToolArchitecture = 'Native64Bit'
        targetPlatform = 'x64'
    }
    Add-BuildStepRecord 'build:toolchain' 'PASS' 0 $script:buildToolchain
    Write-Host "MSBuild: $candidate ($($version.FileVersion)); selected by $selection; compiler/SDK host x64."
    return $candidate
}

function Get-MSBuildProjectState {
    param([Parameter(Mandatory = $true)][string]$Project)

    $projectPath = [IO.Path]::GetFullPath((Join-Path $repoRoot $Project))
    $projectDirectory = [IO.Path]::GetDirectoryName($projectPath)
    $projectName = [IO.Path]::GetFileNameWithoutExtension($projectPath)
    # Record the Engine/Client and Shared/Server canonical trees plus VC's
    # older standalone default, without creating paths or inventing a cache hit.
    foreach ($relative in @(
            "x64\$Configuration\$projectName.tlog\$projectName.lastbuildstate",
            "..\Intermediate\x64\$Configuration\$projectName.tlog\$projectName.lastbuildstate",
            "$projectName\x64\$Configuration\$projectName.tlog\$projectName.lastbuildstate")) {
        $path = [IO.Path]::GetFullPath((Join-Path $projectDirectory $relative))
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            $item = Get-Item -LiteralPath $path
            [pscustomobject][ordered]@{
                path = $path
                lastWriteUtc = $item.LastWriteTimeUtc.ToString('o')
                state = [IO.File]::ReadAllLines($path)
            }
        }
    }
}

function Invoke-MSBuildProject {
    param(
        [string]$MSBuild,
        [string]$Project
    )

    $loggingArguments = @()
    $buildLogs = $null
    if (-not [string]::IsNullOrWhiteSpace($BuildLogDirectory)) {
        $logDirectory = [IO.Path]::GetFullPath($BuildLogDirectory)
        [IO.Directory]::CreateDirectory($logDirectory) | Out-Null
        $projectLabel = [IO.Path]::GetFileNameWithoutExtension($Project)
        $stamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
        $logStem = Join-Path $logDirectory "$stamp-$projectLabel-$Configuration"
        $buildLogs = [pscustomobject][ordered]@{
            binaryLog = "$logStem.binlog"
            diagnosticLog = "$logStem.log"
        }
        $loggingArguments = @(
            "/bl:$($buildLogs.binaryLog)",
            "/flp:LogFile=$($buildLogs.diagnosticLog);Verbosity=diagnostic;Encoding=UTF-8")
    }
    $before = @(Get-MSBuildProjectState $Project)
    $projectPath = [IO.Path]::GetFullPath((Join-Path $repoRoot $Project))
    $beforeOutputs = $null
    try { $beforeOutputs = Get-BuildOutputSnapshot $projectPath $Configuration }
    catch { Write-Warning "Output metadata could not be read for ${Project}: $($_.Exception.Message)" }
    $compilerArguments = @()
    if ($MaxCompilerProcesses -gt 0) {
        $compilerArguments = @("/p:CL_MPCount=$MaxCompilerProcesses")
    }
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $startedUtc = [DateTime]::UtcNow.ToString('o')
    $result = 'FAIL'
    try {
        # Projects are already ordered here with references disabled. Keep one
        # project node and let the shared /MP budget parallelize source files.
        & $MSBuild $Project /m:1 /nodeReuse:false /t:Build `
            "/p:Configuration=$Configuration" /p:Platform=x64 `
            /p:PreferredToolArchitecture=x64 /p:WindowsSDKToolArchitecture=Native64Bit `
            /p:BuildProjectReferences=false /v:minimal @compilerArguments @loggingArguments
        if ($global:LASTEXITCODE -ne 0) {
            throw "Build failed: $Project"
        }
        $result = 'PASS'
    }
    finally {
        $timer.Stop()
        $after = @(Get-MSBuildProjectState $Project)
        $outputChanges = $null
        $rebuildReasons = @()
        $diagnosticError = ''
        try {
            if ($null -ne $beforeOutputs) {
                $afterOutputs = Get-BuildOutputSnapshot $projectPath $Configuration
                $outputChanges = Compare-BuildOutputSnapshot $beforeOutputs $afterOutputs
            }
            if ($null -ne $buildLogs) {
                $rebuildReasons = @(Get-MSBuildIncrementalReasons $buildLogs.diagnosticLog)
            }
        }
        catch { $diagnosticError = $_.Exception.Message }
        $beforeIdentity = @($before | ForEach-Object { $_.path; $_.state }) -join "`n"
        $afterIdentity = @($after | ForEach-Object { $_.path; $_.state }) -join "`n"
        $trackingStateChanged = $beforeIdentity -cne $afterIdentity
        Add-BuildStepRecord "msbuild:$Project" $result $timer.ElapsedMilliseconds `
            ([pscustomobject][ordered]@{
                startedUtc = $startedUtc
                completedUtc = [DateTime]::UtcNow.ToString('o')
                toolchain = $script:buildToolchain
                logs = $buildLogs
                maxProjectNodes = 1
                compilerProcessOverride = $MaxCompilerProcesses
                projectStateBefore = $before
                projectStateAfter = $after
                trackingStateChanged = $trackingStateChanged
                outputChanges = $outputChanges
                rebuildReasonSamples = $rebuildReasons
                diagnosticError = $diagnosticError
            })
        Write-Host "Build phase ${Project}: $result in $($timer.ElapsedMilliseconds) ms."
        if ($null -ne $outputChanges) {
            Write-Host ("Outputs written: OBJ={0}, PCH={1}, CSO={2}, binaries={3}; tracking identity changed={4}." -f `
                $outputChanges.objectWrites, $outputChanges.precompiledHeaderWrites, `
                $outputChanges.shaderWrites, $outputChanges.binaryWrites, $trackingStateChanged)
        }
        if ($trackingStateChanged -and $before.Count -gt 0) {
            Write-Host 'MSBuild tracking identity changed. Compare projectStateBefore/After for toolset, SDK and configuration changes.'
        }
        if ($diagnosticError) { Write-Warning "Build diagnostics unavailable: $diagnosticError" }
    }
}

function Assert-RuntimeLayout {
    $required = @(
        $clientExe,
        $serverExe,
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_Deferred.hlsl'),
        (Join-Path $repoRoot 'Client\Bin\ShaderFiles\Shader_VtxTex.hlsl')
    )
    if ($includeCore) {
        $required += @(
            $protocolHarnessExe,
            $characterSelectIsolationHarnessExe
        )
    }
    if ($includeFullDiagnostic) {
        $required += @(
            $valtanAuditionServiceHarnessExe,
            $pointLightFalloffHarnessExe,
            $physicsHarnessExe,
            $wmodelHarnessExe
        )
    }
    $missing = @($required | Where-Object {
        -not (Test-Path -LiteralPath $_)
    })
    if ($missing.Count -ne 0) {
        throw "Runtime layout is incomplete: $($missing -join ', ')"
    }
}

function Test-ProductNavigationInputs {
    param($NavigationDomain)

    # Inspect published dependencies only. Baking and world admission stay with
    # the navigation publisher and Server; this must not mutate either root.
    $grids = [ordered]@{}
    foreach ($relative in $NavigationDomain.requiredOutputPatterns) {
        if ($relative -match '^(Server|Client)/Bin/DataFiles/Navigation/[^/]+\.navgrid$') {
            $grids[[string]$relative] = $null
        }
    }
    foreach ($side in @('Server', 'Client')) {
        $relativeRoot = "$side/Bin/DataFiles/Navigation"
        foreach ($manifest in @(Get-ChildItem -LiteralPath (Join-Path $repoRoot $relativeRoot) `
            -Filter '*.navregions' -File -ErrorAction SilentlyContinue)) {
            $timer = [Diagnostics.Stopwatch]::StartNew()
            $relative = "$relativeRoot/$($manifest.Name)"
            try {
                $lines = @([IO.File]::ReadAllLines($manifest.FullName) | Where-Object { $_.Trim() })
                $area = $manifest.BaseName
                if ($area -notmatch '^[A-Za-z0-9_-]+$' -or $lines.Count -eq 0 -or
                    $lines[0] -cnotmatch ('^LOSTARK_NAVGRID_REGIONS 1 "' + [regex]::Escape($area) + '" ([0-9]+)$')) {
                    throw 'Invalid navigation region manifest header'
                }
                $count = [uint64]$Matches[1]
                if ($count -gt 64 -or $lines.Count -ne $count + 1) { throw 'Truncated or invalid navigation region manifest' }
                $grids["$relativeRoot/$area.navgrid"] = $null
                $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                foreach ($line in @($lines | Select-Object -Skip 1)) {
                    if ($line -cnotmatch '^REGION "([A-Za-z0-9_-]{1,32})" (\S+)$') { throw 'Invalid navigation region row' }
                    $region = $Matches[1]
                    $step = [single]::Parse($Matches[2], [Globalization.CultureInfo]::InvariantCulture)
                    if (-not $seen.Add($region) -or [double]::IsNaN($step) -or [double]::IsInfinity($step) -or $step -lt 0) {
                        throw 'Duplicate region or invalid navigation step policy'
                    }
                    $grids["$relativeRoot/$area.$region.navgrid"] = $step
                }
            }
            catch {
                [pscustomobject]@{ path=$relative; publisher='Tools/NavigationPipeline/Publish-ServerNavigation.ps1';
                    result='FAIL'; elapsedMs=$timer.ElapsedMilliseconds; details=$_.Exception.Message }
            }
            finally { $timer.Stop() }
        }
    }
    foreach ($entry in $grids.GetEnumerator()) {
        $timer = [Diagnostics.Stopwatch]::StartNew()
        $relative = [string]$entry.Key
        $result = 'FAIL'
        $details = ''
        try {
            $path = Join-Path $repoRoot $relative
            $stream = [IO.File]::OpenRead($path)
            $reader = [IO.BinaryReader]::new($stream)
            try {
                if ($stream.Length -lt 20) { throw 'Truncated navigation grid header (expected 20 bytes)' }
                $width = $reader.ReadUInt32(); $height = $reader.ReadUInt32()
                $cellSize = $reader.ReadSingle(); $originX = $reader.ReadSingle(); $originZ = $reader.ReadSingle()
                $cells = [uint64]$width * [uint64]$height
                if ($width -eq 0 -or $height -eq 0 -or $cells -gt 1000000 -or $cellSize -le 0 -or
                    [single]::IsNaN($cellSize) -or [single]::IsInfinity($cellSize) -or
                    [single]::IsNaN($originX) -or [single]::IsInfinity($originX) -or
                    [single]::IsNaN($originZ) -or [single]::IsInfinity($originZ)) { throw 'Invalid navigation grid header' }
                if ($stream.Length -ne 20 + 5 * $cells) { throw 'Truncated navigation grid payload or unexpected trailing bytes' }
            }
            finally { $reader.Dispose() }
            $stem = [IO.Path]::GetFileNameWithoutExtension($path)
            foreach ($extension in @('navpolicy', 'navblockers')) {
                $relative = [IO.Path]::ChangeExtension([string]$entry.Key, $extension).Replace('\', '/')
                $sidecar = Join-Path $repoRoot $relative
                if (-not (Test-Path -LiteralPath $sidecar -PathType Leaf) -or (Get-Item -LiteralPath $sidecar).Length -eq 0) {
                    throw 'Missing or empty navigation sidecar'
                }
                $header = [IO.File]::ReadLines($sidecar) | Select-Object -First 1
                $tokens = [regex]::Matches($header, '"[^"]*"|\S+') | ForEach-Object { $_.Value.Trim('"') }
                if ($tokens.Count -lt 3 -or $tokens[1] -cne '1' -or $tokens[2] -cne $stem) { throw 'Invalid navigation sidecar identity' }
                if ($extension -eq 'navpolicy') {
                    if ($tokens.Count -ne 4 -or $tokens[0] -cne 'LOSTARK_NAVIGATION_POLICY') { throw 'Invalid navigation policy header' }
                    $step = [single]::Parse($tokens[3], [Globalization.CultureInfo]::InvariantCulture)
                    if ([double]::IsNaN($step) -or [double]::IsInfinity($step) -or $step -lt 0 -or
                        ($null -ne $entry.Value -and [Math]::Abs($step - $entry.Value) -gt 0.000001)) { throw 'Navigation policy differs from region manifest' }
                }
                else {
                    if ($tokens.Count -ne 9 -or $tokens[0] -cne 'LOSTARK_NAVGRID_BLOCKERS' -or
                        [uint32]$tokens[3] -ne $width -or [uint32]$tokens[4] -ne $height -or [uint32]$tokens[8] -gt 256) {
                        throw 'Invalid navigation blocker header'
                    }
                    for ($index = 0; $index -lt 3; ++$index) {
                        $value = [single]::Parse($tokens[5 + $index], [Globalization.CultureInfo]::InvariantCulture)
                        if ([double]::IsNaN($value) -or [double]::IsInfinity($value) -or
                            [Math]::Abs($value - @($cellSize, $originX, $originZ)[$index]) -gt 0.000001) { throw 'Navigation blocker grid differs from payload' }
                    }
                    if ($null -ne $entry.Value -and [uint32]$tokens[8] -ne 0) { throw 'Detail navigation region declares runtime blockers' }
                }
            }
            $relative = [string]$entry.Key
            $result = 'PASS'
            $details = 'Published grid byte length and required sidecar headers are complete'
        }
        catch { $details = $_.Exception.Message }
        finally { $timer.Stop() }
        [pscustomobject]@{ path=$relative; publisher='Tools/NavigationPipeline/Publish-ServerNavigation.ps1';
            result=$result; elapsedMs=$timer.ElapsedMilliseconds; details=$details }
    }
}

function Test-ProductPublishedCatalogs {
    # These small publishers own their expected serialized format. Compare their
    # current source projection without publishing, hashing the full tree, or
    # duplicating bootstrap versions in this build runner.
    $checks = @(
        @{ publisher = 'Tools\GameplayPipeline\Publish-ItemCatalog.ps1';
           output = 'Server/Bin/DataFiles/Items/Items.bootstrap' },
        @{ publisher = 'Tools\ValtanPipeline\Publish-ValtanClearRewards.ps1';
           output = 'Server/Bin/DataFiles/Valtan/ClearRewards.bootstrap' }
    )
    foreach ($check in $checks) {
        if (-not (Test-Path -LiteralPath (Join-Path $repoRoot $check.output) -PathType Leaf)) {
            continue # Reported by the existing required-file check.
        }
        $timer = [Diagnostics.Stopwatch]::StartNew()
        $checkResult = 'FAIL'
        $checkOutput = ''
        try {
            # Both PowerShell publishers report failure by throwing. Invoke in
            # script scope to avoid two fresh PowerShell hosts on every Build.
            $checkOutput = (& (Join-Path $repoRoot $check.publisher) `
                -Mode CheckPublished | Out-String).Trim()
            $checkResult = 'PASS'
        }
        catch {
            $checkOutput = $_.Exception.Message
        }
        finally {
            $timer.Stop()
        }
        [pscustomobject][ordered]@{
            path = $check.output
            publisher = $check.publisher
            result = $checkResult
            elapsedMs = $timer.ElapsedMilliseconds
            details = $checkOutput
        }
    }
}

function Invoke-PythonGate {
    param(
        [string]$Description,
        [string[]]$Arguments
    )

    $python = (Get-Command python -ErrorAction Stop).Source
    $pythonExitCode = -1
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $previousErrorActionPreference = $ErrorActionPreference
    try {
        # unittest writes successful progress to stderr.  Windows PowerShell
        # surfaces that stream as NativeCommandError when the script-wide
        # preference is Stop, so preserve the process exit code explicitly.
        $ErrorActionPreference = 'Continue'
        # Native commands update the global automatic variable. Assigning the
        # unqualified name here creates a function-local shadow in Windows
        # PowerShell, which made every failed Python gate look successful.
        $global:LASTEXITCODE = 0
        & $python -B @Arguments
        $pythonExitCode = $global:LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
        $timer.Stop()
        $result = if ($pythonExitCode -eq 0) { 'PASS' } else { 'FAIL' }
        Add-BuildStepRecord "python:$Description" $result $timer.ElapsedMilliseconds
    }
    if ($pythonExitCode -ne 0) {
        throw "$Description failed."
    }
}

function Assert-ProductOutputsUnlocked {
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $result = 'FAIL'
    try {
        Assert-StandardProductOutputsNotRunning -RepositoryRoot $repoRoot
        $result = 'PASS'
    }
    finally {
        $timer.Stop()
        Add-BuildStepRecord 'product:output-lock-preflight' $result `
            $timer.ElapsedMilliseconds
    }
}

function Invoke-SelectedBuildDomains {
    foreach ($domain in @(Get-BuildDomainsForProfile $buildDomainManifest $Profile)) {
        $timer = [Diagnostics.Stopwatch]::StartNew()
        $result = 'FAIL'
        try {
            Assert-ExpectedValtanSourceRevision "domain $($domain.id) start"
            $domainResult = Invoke-BuildDomain $repoRoot $domain `
                $runtimeResourceRoot $buildReceiptRoot
            Assert-ExpectedValtanSourceRevision "domain $($domain.id) completion"
            $script:buildDomainResults.Add($domainResult) | Out-Null
            $result = if ($domainResult.reused) { 'REUSED' } else { 'PASS' }
            Write-Host "Build domain $($domainResult.domainId): $result"
        }
        finally {
            $timer.Stop()
            Add-BuildStepRecord "domain:$($domain.id)" $result `
                $timer.ElapsedMilliseconds
        }
    }
}

function Write-CurrentProductReceipt {
    $timer = [Diagnostics.Stopwatch]::StartNew()
    $result = 'FAIL'
    try {
        Assert-ExpectedValtanSourceRevision 'product receipt start'
        $path = Write-BuildProductReceipt $repoRoot $buildDomainManifest `
            $Configuration $buildReceiptRoot
        Assert-ExpectedValtanSourceRevision 'product receipt completion'
        $result = 'PASS'
        Write-Host "Product build receipt: $path"
    }
    finally {
        $timer.Stop()
        Add-BuildStepRecord 'product:receipt' $result $timer.ElapsedMilliseconds
    }
}

function Capture-BuildStartIdentity {
    $script:buildStartGitIdentity = Get-BuildGitIdentity $repoRoot
    $source = Get-BuildProductSourceFingerprint $repoRoot `
        $buildDomainManifest $Configuration
    $script:buildStartProductSourceInputSha256 =
        [string]$source.sourceInputSha256
}

function Write-CurrentBuildEvidence {
    Assert-ExpectedValtanSourceRevision 'final evidence start'
    $stability = Assert-BuildRunStability $repoRoot $buildDomainManifest `
        $Configuration $runtimeResourceRoot $buildReceiptRoot `
        $script:buildStartGitIdentity `
        $script:buildStartProductSourceInputSha256 `
        @($script:buildDomainResults)
    $script:buildRunTimer.Stop()
    $path = Write-BuildRunEvidence $repoRoot $Configuration $Profile `
        ([bool]$SkipBuild) @($script:buildStepRecords) `
        @($script:buildDomainResults) $buildEvidenceRoot $stability `
        $script:buildStartedUtc $script:buildRunTimer.ElapsedMilliseconds
    Assert-ExpectedValtanSourceRevision 'final evidence completion'
    Write-Host "Build evidence: $path"
}

$productRunLockPath = Join-Path (Join-Path $buildReceiptRoot 'locks') `
    "product.$($Configuration.ToLowerInvariant()).lock"
$productRunLock = Enter-BuildExclusiveLock $productRunLockPath 300000 `
    "$Configuration product build/run"
Push-Location $repoRoot
$previousResourceRoot = [Environment]::GetEnvironmentVariable(
    'LOSTARK_RESOURCE_ROOT', 'Process')
try {
    [Environment]::SetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', $runtimeResourceRoot, 'Process')

    Assert-ExpectedValtanSourceRevision 'build runner admission'

    if (-not $SkipBuild) { Assert-ProductOutputsUnlocked }

    if (-not $includeCore) {
        # Daily authoring needs compilation and normal MSBuild deployment only.
        # Publishers and diagnostics are separate, explicit operations.
        if (-not $SkipBuild) {
            $msbuild = Resolve-MSBuild
            Invoke-MSBuildProject $msbuild 'Engine\Default\Engine.vcxproj'
            Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
            Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
            Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
        }
        Assert-RuntimeLayout
        # Read the publishers' required-file contract without running them.
        # Server startup admits every world, regardless of the selected raid.
        $runtimeInputManifest = Read-BuildDomainManifest $domainManifestPath
        $missingRuntimeInputs = @(
            foreach ($runtimeDomainId in @('gameplay.balance', 'items.catalog',
                'vehicles.profiles', 'honortitles.catalog', 'valtan.rewards',
                'world.gameplay', 'navigation')) {
                $runtimeDomain = Get-BuildDomainById $runtimeInputManifest $runtimeDomainId
                $runtimeDomain.requiredOutputPatterns | Where-Object {
                    ([string]$_).StartsWith('Server/Bin/DataFiles/', [StringComparison]::Ordinal)
                }
            }
            'Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.mapassets'
        ) | Where-Object {
            $runtimeInputPath = Join-Path $repoRoot $_
            if ([Management.Automation.WildcardPattern]::ContainsWildcardCharacters([IO.Path]::GetFileName($_))) {
                @(Get-ChildItem -LiteralPath (Split-Path -Parent $runtimeInputPath) `
                    -Filter (Split-Path -Leaf $runtimeInputPath) -File -ErrorAction SilentlyContinue).Count -eq 0
            }
            else { -not (Test-Path -LiteralPath $runtimeInputPath -PathType Leaf) }
        }
        if ($missingRuntimeInputs) {
            Write-Warning ('Runtime data is not prepared: ' + ($missingRuntimeInputs -join ', '))
            Write-Host 'Before launching, run these explicit data-generation commands:'
            Write-Host 'powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server'
            Write-Host 'powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Client'
        }
        $script:runtimeDataChecks = @(
            Test-ProductPublishedCatalogs
            Test-ProductNavigationInputs (Get-BuildDomainById $runtimeInputManifest 'navigation')
        )
        $invalidRuntimeInputs = @($script:runtimeDataChecks | Where-Object result -ne 'PASS' |
            ForEach-Object { $_.path })
        foreach ($check in $script:runtimeDataChecks | Where-Object result -ne 'PASS') {
            Write-Warning ("Published runtime data is not current: $($check.path)`n$($check.details)")
            Write-Host "Repair this catalog explicitly: powershell -NoProfile -ExecutionPolicy Bypass -File $($check.publisher) -Mode Publish"
        }
        $script:buildRunTimer.Stop()
        Write-ProductCompileEvidence 'PASS' @($missingRuntimeInputs) @($invalidRuntimeInputs)
        Write-Host "Product compile/deploy completed: $Configuration (SkipBuild=$([bool]$SkipBuild))"
        Write-Host 'Runtime file presence, Navigation references and Item/Valtan reward catalog content were checked; no data was published. Other runtime domains and visual/audio review remain separate.'
        return
    }

    if ($includeCore) {
        Invoke-PythonGate `
            'Action Presentation Workbench joined-domain gate' `
            @('Tools/ValtanPipeline/test_action_presentation_workbench_contract.py')
        Invoke-PythonGate `
            'Bern entrance camera authored runtime gate' `
            @('Tools/ValtanPipeline/test_bern_entrance_camera_contract.py')
        Invoke-PythonGate `
            'KoukuSaydon world and protected resource-alias admission gate' `
            @('Tools/KoukuSaydonPipeline/test_kouku_saydon_world_admission.py')
        Invoke-PythonGate `
            'KoukuSaydon Client product level contract gate' `
            @('Tools/KoukuSaydonPipeline/test_kouku_saydon_client_product_level_contract.py')
        Invoke-PythonGate `
            'Valtan eight-player raid capacity gate' `
            @('Tools/Network/test_valtan_raid_capacity_contract.py')
        Invoke-PythonGate `
            'Effect Tool V2 authored document gate' `
            @('Tools/EffectToolV2/test_validate_effect_v2.py')
        Invoke-PythonGate `
            'Effect Tool V2 binding schema, migration, and read-set gate' `
            @('Tools/EffectToolV2/test_effect_v2_binding_pipeline.py')
        Invoke-PythonGate `
            'Effect Tool V2 Product transaction gate' `
            @('Tools/EffectToolV2/test_effect_v2_product_contract.py')
        Invoke-PythonGate `
            'Effect Tool V2 catalog schema contract gate' `
            @('Tools/EffectToolV2/test_effect_v2_catalog_contract.py')
        Invoke-PythonGate `
            'Effect Tool V2 occurrence runtime contract gate' `
            @('Tools/EffectToolV2/test_effect_v2_occurrence_runtime_contract.py')
        Invoke-PythonGate `
            'Team LAN endpoint contract gate' `
            @('Tools/Network/test_team_lan_endpoint_contract.py')
        Invoke-PythonGate `
            'Valtan status and response data-contract gate' `
            @('-m', 'unittest',
              'Tools.ValtanPipeline.test_valtan_status_pattern_contract')

        $global:LASTEXITCODE = 0
        & '.\Tools\WorldPipeline\Publish-ValtanWorldDestruction.ps1' `
            -Mode ContractTest
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Valtan world destruction contract tests failed.'
        }

        $global:LASTEXITCODE = 0
        & '.\Tools\EffectPipeline\Validate-EffectSources.ps1' `
            -RepositoryRoot $repoRoot `
            -ResourceRoot $runtimeResourceRoot `
            -AllowLocalResources:$AllowLocalEffectResources
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Effect source validation failed before compilation.'
        }
    }

    Invoke-SelectedBuildDomains
    Capture-BuildStartIdentity

    if (-not $SkipBuild) {
        $msbuild = Resolve-MSBuild
        Invoke-MSBuildProject $msbuild 'Engine\Default\Engine.vcxproj'
        Invoke-MSBuildProject $msbuild 'Shared\Default\Shared.vcxproj'
        if ($includeCore) {
            Invoke-MSBuildProject $msbuild `
                'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\CharacterSelectIsolationHarness\Default\CharacterSelectIsolationHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\ValtanPatternAuditionServiceHarness\Default\ValtanPatternAuditionServiceHarness.vcxproj'
        }
        Invoke-MSBuildProject $msbuild 'Server\Default\Server.vcxproj'
        Invoke-MSBuildProject $msbuild 'Client\Default\Client.vcxproj'
        if ($includeFullDiagnostic) {
            Invoke-MSBuildProject $msbuild `
                'Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\PhysicsContractHarness\Default\PhysicsContractHarness.vcxproj'
            Invoke-MSBuildProject $msbuild `
                'Tools\WModelGeometryContractHarness\Default\WModelGeometryContractHarness.vcxproj'
        }
    }

    $shaderTimer = [Diagnostics.Stopwatch]::StartNew()
    $shaderResult = 'FAIL'
    try {
        $global:LASTEXITCODE = 0
        & '.\Tools\Build\Test-CompiledShaderClosure.ps1' `
            -Configuration $Configuration `
            -RepositoryRoot $repoRoot `
            -Modules Product
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Compiled shader closure validation failed.'
        }
        $shaderResult = 'PASS'
    }
    finally {
        $shaderTimer.Stop()
        Add-BuildStepRecord 'product:compiled-shader-closure' $shaderResult `
            $shaderTimer.ElapsedMilliseconds
    }

    Assert-RuntimeLayout
    if (-not $SkipBuild) {
        Write-CurrentProductReceipt
    }

    if ($includeFullDiagnostic) {
        $global:LASTEXITCODE = 0
        & '.\Tools\MapPipeline\Test-MapWaterRenderContract.ps1'
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Map water render contract validation failed.'
        }
        Invoke-PythonGate `
            'Map surface geometry and depth diagnostic unit gate' `
            @('Tools/MapPipeline/test_map_surface_depth_contract.py')
    }

	if ($includeFullDiagnostic) {
		$global:LASTEXITCODE = 0
		& '.\Tools\ValtanPipeline\Test-ValtanPatternMaster.ps1'
		if ($global:LASTEXITCODE -ne 0) {
			throw 'Valtan split pattern master focused harness failed.'
		}
		Invoke-PythonGate `
			'Valtan Animation Tool master timeline gate' `
			@('Tools/ValtanPipeline/test_animation_tool_valtan_pattern_master.py')
		Invoke-PythonGate `
			'Valtan pattern sound cue exact-join gate' `
			@('-m', 'unittest',
			  'Tools.ValtanPipeline.test_valtan_pattern_sound_cue_contract')
		Invoke-PythonGate `
			'Valtan Effect Tool master tree gate' `
			@('Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py')
		Invoke-PythonGate `
			'Valtan CROSS Product cue and fixed-step rock wave gate' `
			@('Tools/EffectPipeline/test_valtan_cross_rock_wave_effect.py')
		Invoke-PythonGate `
			'Valtan combat-object hit Effect presentation gate' `
			@('Tools/ValtanPipeline/test_valtan_combat_object_hit_effect_presentation_contract.py')
	}

    $global:LASTEXITCODE = 0
    & '.\Tools\WorldPipeline\Split-ValtanIndependentWallGroups.ps1' `
        -Mode CheckNavigation
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Valtan wall navigation footprint validation failed.'
    }
    if ($includeFullDiagnostic) {
        Invoke-PythonGate `
            'Valtan floor crack emissive runtime contract gate' `
            @('Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py')
        Invoke-PythonGate `
            'Valtan body collision and model composition gate' `
            @('Tools/EffectPipeline/test_valtan_model_view_composition.py')
        Invoke-PythonGate `
            'Ground-target preview prototype scope gate' `
            @('Tools/GameplayPipeline/test_ground_target_preview_prototype_scope.py')
    }

    $global:LASTEXITCODE = 0
    & '.\Tools\RenderingPipeline\Publish-RenderingProfiles.ps1' `
        -Mode Validate
    if ($global:LASTEXITCODE -ne 0) {
        throw 'Rendering profile validation failed.'
    }
    if ($includeFullDiagnostic) {
        Invoke-PythonGate `
            'Rendering authored/runtime identity and float32 boundary gate' `
            @('Tools/RenderingPipeline/test_publish_rendering_profiles.py')
    }

    & $protocolHarnessExe
    if ($global:LASTEXITCODE -ne 0) {
        throw 'NetworkProtocolHarness failed.'
    }

    & $valtanAuditionServiceHarnessExe
    if ($global:LASTEXITCODE -ne 0) {
        throw 'ValtanPatternAuditionServiceHarness failed.'
    }

    if ($includeFullDiagnostic) {
        & $serverExe --contract-test
        if ($global:LASTEXITCODE -ne 0) {
            throw 'Server gameplay contract tests failed.'
        }
    }

	& (Join-Path $repoRoot `
		'Tools\Network\Run-CharacterSelectIsolationHarness.ps1') `
		-Configuration $Configuration `
		-Scenario Core

    if ($includeFullDiagnostic) {
		foreach ($partyScenario in @('Party2', 'Party4')) {
			& (Join-Path $repoRoot `
				'Tools\Network\Run-CharacterSelectIsolationHarness.ps1') `
				-Configuration $Configuration `
				-Scenario $partyScenario
		}

        & (Join-Path $repoRoot `
            'Tools\PointLightFalloffContractHarness\Run-PointLightFalloffContractHarness.ps1') `
            -Configuration $Configuration

        & (Join-Path $repoRoot `
            'Tools\PhysicsContractHarness\Run-PhysicsContractHarness.ps1') `
            -Configuration $Configuration

        & (Join-Path $repoRoot `
            'Tools\WModelGeometryContractHarness\Run-WModelGeometryContractHarness.ps1') `
            -Configuration $Configuration
    }

    Write-CurrentBuildEvidence
	Write-Host "Build and regression completed: $Configuration / $Profile"
    Write-Host 'Runtime level validation uses Framework.slnLaunch (Server + Client).'
}
catch {
    $script:buildFailure = $_.Exception.Message
    throw
}
finally {
    [Environment]::SetEnvironmentVariable(
        'LOSTARK_RESOURCE_ROOT', $previousResourceRoot, 'Process')
    Pop-Location
    $script:buildRunTimer.Stop()
    if (-not $includeCore -and -not $script:productCompileEvidencePath) {
        try { Write-ProductCompileEvidence 'FAIL' }
        catch { Write-Warning "Could not write failed build evidence: $($_.Exception.Message)" }
    }
    $productRunLock.Dispose()
}
