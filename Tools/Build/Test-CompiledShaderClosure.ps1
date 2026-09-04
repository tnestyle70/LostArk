[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration,

    [string]$RepositoryRoot,

    [ValidateSet("Product", "All")]
    [string]$Modules = "All"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = Join-Path $PSScriptRoot "..\.."
}

$repositoryPath = [System.IO.Path]::GetFullPath($RepositoryRoot)
$platform = "x64"
$script:failures = [System.Collections.Generic.List[string]]::new()

function Add-Failure {
    param([string]$Message)

    $script:failures.Add($Message)
}

function Test-MsBuildCondition {
    param([AllowEmptyString()][string]$Condition)

    if ([string]::IsNullOrWhiteSpace($Condition)) {
        return $true
    }

    $expanded = $Condition.Replace('$(Configuration)', $Configuration)
    $expanded = $expanded.Replace('$(Platform)', $platform).Trim()
    $match = [regex]::Match($expanded, "^'([^']*)'\s*(==|!=)\s*'([^']*)'$",
        [System.Text.RegularExpressions.RegexOptions]::CultureInvariant)
    if (-not $match.Success) {
        throw "Unsupported MSBuild condition in compiled shader closure: $Condition"
    }

    $equal = [string]::Equals(
        $match.Groups[1].Value,
        $match.Groups[3].Value,
        [System.StringComparison]::OrdinalIgnoreCase)
    if ($match.Groups[2].Value -eq "==") {
        return $equal
    }

    return -not $equal
}

function Get-ActiveFxCompileItems {
    param(
        [string]$ProjectPath,
        [string]$Owner,
        [string]$OutputDirectory
    )

    if (-not (Test-Path -LiteralPath $ProjectPath -PathType Leaf)) {
        Add-Failure "[$Owner] project is missing: $ProjectPath"
        return @()
    }

    [xml]$project = Get-Content -LiteralPath $ProjectPath -Raw
    $configurationName = "$Configuration|$platform"
    $configurationNodes = $project.SelectNodes(
        "//*[local-name()='ProjectConfiguration']")
    $hasConfiguration = $false
    foreach ($node in $configurationNodes) {
        if ([string]::Equals(
            $node.GetAttribute("Include"),
            $configurationName,
            [System.StringComparison]::OrdinalIgnoreCase)) {
            $hasConfiguration = $true
            break
        }
    }
    if (-not $hasConfiguration) {
        Add-Failure "[$Owner] project has no $configurationName configuration: $ProjectPath"
    }

    $items = [System.Collections.Generic.List[object]]::new()
    foreach ($node in $project.SelectNodes("//*[local-name()='FxCompile']")) {
        $condition = $node.GetAttribute("Condition")
        if (-not (Test-MsBuildCondition $condition)) {
            continue
        }

        $excluded = $false
        foreach ($child in $node.ChildNodes) {
            if ($child.LocalName -ne "ExcludedFromBuild") {
                continue
            }
            if ((Test-MsBuildCondition $child.GetAttribute("Condition")) -and
                $child.InnerText.Trim().Equals("true", [System.StringComparison]::OrdinalIgnoreCase)) {
                $excluded = $true
                break
            }
        }
        if ($excluded) {
            continue
        }

        $include = $node.GetAttribute("Include")
        if ([string]::IsNullOrWhiteSpace($include)) {
            continue
        }
        if ($include.Contains('$(')) {
            Add-Failure "[$Owner] FxCompile path contains an unsupported property: $include"
            continue
        }

        $sourcePath = [System.IO.Path]::GetFullPath(
            (Join-Path ([System.IO.Path]::GetDirectoryName($ProjectPath)) $include))
        $stem = [System.IO.Path]::GetFileNameWithoutExtension($sourcePath)
        if ([string]::IsNullOrWhiteSpace($stem)) {
            Add-Failure "[$Owner] FxCompile item has no output stem: $include"
            continue
        }

        $items.Add([pscustomobject]@{
            Owner = $Owner
            ProjectPath = $ProjectPath
            SourcePath = $sourcePath
            ShaderId = "$stem.hlsl"
            Stem = $stem
            OutputPath = Join-Path $OutputDirectory "$stem.cso"
        })
    }

    return $items
}

function Get-DirectShaderConsumers {
    param([string[]]$SearchRoots)

    $result = [System.Collections.Generic.HashSet[string]]::new(
        [System.StringComparer]::OrdinalIgnoreCase)
    $createPattern = '(?s)CShader::Create\s*\(.{0,1200}?TEXT\(\s*"[^"]*(Shader_[^"/\\]+\.hlsl)"\s*\)'

    foreach ($root in $SearchRoots) {
        if (-not (Test-Path -LiteralPath $root -PathType Container)) {
            continue
        }

        foreach ($file in Get-ChildItem -LiteralPath $root -Recurse -File -Include *.cpp, *.h) {
            $source = [System.IO.File]::ReadAllText($file.FullName)
            foreach ($match in [regex]::Matches($source, $createPattern)) {
                [void]$result.Add($match.Groups[1].Value)
            }
        }
    }

    $effectV2Path = Join-Path $repositoryPath "Client\Private\EffectV2_Object.cpp"
    if (Test-Path -LiteralPath $effectV2Path -PathType Leaf) {
        $effectV2Source = [System.IO.File]::ReadAllText($effectV2Path)
        foreach ($match in [regex]::Matches(
            $effectV2Source,
            'TEXT\(\s*"[^"]*(Shader_[^"/\\]+\.hlsl)"\s*\)')) {
            [void]$result.Add($match.Groups[1].Value)
        }
    }

    return @($result | Sort-Object)
}

function Get-FileHashValue {
    param([string]$Path)

    # Do not depend on PowerShell module auto-loading here.  This validator is
    # also invoked through the nested full-pipeline runner, where a fresh
    # no-profile process can otherwise fail to discover Get-FileHash after the
    # native build steps have completed.
    $stream = [System.IO.File]::OpenRead($Path)
    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    try {
        return [System.BitConverter]::ToString(
            $sha256.ComputeHash($stream)).Replace('-', '')
    }
    finally {
        $sha256.Dispose()
        $stream.Dispose()
    }
}

function Invoke-ProductEffectShaderWarpProbe {
    $sourcePath = Join-Path $repositoryPath `
        'Tools\RenderingPipeline\ProductEffectShaderWarpProbe.cpp'
    if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
        Add-Failure "Product Effect shader WARP probe source is missing: $sourcePath"
        return
    }

    $programFilesX86 = ${env:ProgramFiles(x86)}
    $vswhere = Join-Path $programFilesX86 `
        'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
        Add-Failure "Product Effect shader WARP probe requires vswhere: $vswhere"
        return
    }
    $installationPath = [string](@(& $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath) | Select-Object -First 1)
    if ([string]::IsNullOrWhiteSpace($installationPath)) {
        Add-Failure 'Product Effect shader WARP probe could not locate Visual Studio C++ tools.'
        return
    }
    $vsDevCmd = Join-Path $installationPath 'Common7\Tools\VsDevCmd.bat'
    if (-not (Test-Path -LiteralPath $vsDevCmd -PathType Leaf)) {
        Add-Failure "Product Effect shader WARP probe requires VsDevCmd: $vsDevCmd"
        return
    }

    $generatedRoot = [IO.Path]::GetFullPath(
        (Join-Path $repositoryPath '.codex_tmp'))
    $probeRoot = Join-Path $generatedRoot `
        ('product-effect-shader-warp-' + [Guid]::NewGuid().ToString('N'))
    try {
        New-Item -ItemType Directory -Path $probeRoot -Force | Out-Null
        $probeExecutable = Join-Path $probeRoot 'ProductEffectShaderWarpProbe.exe'
        $probeObject = Join-Path $probeRoot 'ProductEffectShaderWarpProbe.obj'
        $compilerPdb = Join-Path $probeRoot 'ProductEffectShaderWarpProbe.compiler.pdb'
        $linkPdb = Join-Path $probeRoot 'ProductEffectShaderWarpProbe.pdb'
        $engineSdkIncludes = Join-Path $repositoryPath 'EngineSDK\Inc'
        $thirdPartyLibraries = Join-Path $repositoryPath 'Engine\ThirdPartyLib'
        $runtimeOptions = if ($Configuration -eq 'Debug') {
            '/MDd /D_DEBUG'
        }
        else {
            '/MD /DNDEBUG'
        }
        $effectsLibrary = if ($Configuration -eq 'Debug') {
            'Effects11d.lib'
        }
        else {
            'Effects11.lib'
        }
        $compileCommand =
            'call "{0}" -no_logo -arch=x64 -host_arch=x64 && ' +
            'cl.exe /nologo /std:c++20 /EHsc /W4 /utf-8 {1} ' +
            '/DUNICODE /D_UNICODE /I"{2}" "{3}" /Fo:"{4}" /Fd:"{5}" ' +
            '/Fe:"{6}" /link /INCREMENTAL:NO /PDB:"{7}" /LIBPATH:"{8}" ' +
            '{9} d3d11.lib dxgi.lib'
        $compileCommand = $compileCommand -f $vsDevCmd, $runtimeOptions,
            $engineSdkIncludes, $sourcePath, $probeObject, $compilerPdb,
            $probeExecutable, $linkPdb, $thirdPartyLibraries, $effectsLibrary
        $previousNativeErrorAction = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $global:LASTEXITCODE = -1
            $compileOutput = @(& $env:COMSPEC /d /s /c $compileCommand 2>&1)
            $compileExitCode = $global:LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $previousNativeErrorAction
        }
        if ($compileExitCode -ne 0 -or
            -not (Test-Path -LiteralPath $probeExecutable -PathType Leaf)) {
            Add-Failure (
                "Product Effect shader WARP probe compile failed with code " +
                "${compileExitCode}: $($compileOutput -join ' ')")
            return
        }

        $externalResourceRoot = Join-Path $probeRoot 'external-resources'
        New-Item -ItemType Directory -Path $externalResourceRoot -Force |
            Out-Null
        $defaultResourceRoot = Join-Path $repositoryPath `
            'Client\Bin\Resources'
        $missingResourceRoot = Join-Path $probeRoot 'missing-resources'
        $previousResourceRoot = [Environment]::GetEnvironmentVariable(
            'LOSTARK_RESOURCE_ROOT', 'Process')
        $previousSharedRoot = [Environment]::GetEnvironmentVariable(
            'LOSTARK_SHARED_ASSET_ROOT', 'Process')
        $setRootEnvironment = {
            param(
                [string]$Name,
                [AllowNull()][AllowEmptyString()][object]$Value
            )
            if ($null -eq $Value) {
                $Value = [System.Management.Automation.Language.NullString]::Value
            }
            [Environment]::SetEnvironmentVariable(
                $Name, $Value, 'Process')
        }
        $invokeRootCase = {
            param(
                [string]$Name,
                [AllowNull()][AllowEmptyString()][object]$Primary,
                [AllowNull()][AllowEmptyString()][object]$Shared,
                [AllowNull()][string]$Expected,
                [int]$ExpectedExit
            )
            & $setRootEnvironment 'LOSTARK_RESOURCE_ROOT' $Primary
            & $setRootEnvironment 'LOSTARK_SHARED_ASSET_ROOT' $Shared
            $previousRootCaseErrorAction = $ErrorActionPreference
            try {
                $ErrorActionPreference = 'Continue'
                $global:LASTEXITCODE = -1
                $rootOutput = @(& $probeExecutable $repositoryPath `
                    $Configuration '--validate-resource-root' 2>&1)
                $rootExitCode = $global:LASTEXITCODE
            }
            finally {
                $ErrorActionPreference = $previousRootCaseErrorAction
            }
            if ($rootExitCode -ne $ExpectedExit) {
                Add-Failure (
                    "Product Effect resource-root case '$Name' returned " +
                    "$rootExitCode, expected ${ExpectedExit}: " +
                    ($rootOutput -join ' '))
                return $false
            }
            if ($ExpectedExit -eq 0) {
                $rootJson = @($rootOutput | Where-Object {
                    $_.ToString().TrimStart().StartsWith('{')
                } | Select-Object -Last 1)
                if ($rootJson.Count -ne 1) {
                    Add-Failure (
                        "Product Effect resource-root case '$Name' emitted " +
                        "no result: " + ($rootOutput -join ' '))
                    return $false
                }
                try {
                    $rootResult = $rootJson[0].ToString() | ConvertFrom-Json
                }
                catch {
                    Add-Failure (
                        "Product Effect resource-root case '$Name' emitted " +
                        "invalid JSON: $($_.Exception.Message)")
                    return $false
                }
                $selectedRoot = [IO.Path]::GetFullPath(
                    [string]$rootResult.resourceRoot).TrimEnd('\', '/')
                $expectedRoot = [IO.Path]::GetFullPath(
                    $Expected).TrimEnd('\', '/')
                if (-not [StringComparer]::OrdinalIgnoreCase.Equals(
                        $selectedRoot, $expectedRoot) -or
                    $rootResult.assetPathBoundaryValidated -ne $true) {
                    Add-Failure (
                        "Product Effect resource-root case '$Name' selected " +
                        "'$selectedRoot', expected '$expectedRoot', or lost " +
                        'the canonical asset boundary.')
                    return $false
                }
            }
            elseif (($rootOutput -join ' ') -notmatch
                'Product Effect resource root is unavailable:') {
                Add-Failure (
                    "Product Effect resource-root case '$Name' lost the " +
                    'invalid configured-root failure reason.')
                return $false
            }
            return $true
        }
        $resourceRootCases = @(
            [pscustomobject]@{ Name = 'external primary';
                Primary = $externalResourceRoot; Shared = $null;
                Expected = $externalResourceRoot; Exit = 0 },
            [pscustomobject]@{ Name = 'primary takes precedence';
                Primary = $externalResourceRoot; Shared = $missingResourceRoot;
                Expected = $externalResourceRoot; Exit = 0 },
            [pscustomobject]@{ Name = 'shared root fallback';
                Primary = $null; Shared = $externalResourceRoot;
                Expected = $externalResourceRoot; Exit = 0 },
            [pscustomobject]@{ Name = 'repository default';
                Primary = $null; Shared = $null;
                Expected = $defaultResourceRoot; Exit = 0 },
            [pscustomobject]@{ Name = 'empty roots use repository default';
                Primary = ''; Shared = '';
                Expected = $defaultResourceRoot; Exit = 0 },
            [pscustomobject]@{ Name = 'empty primary permits shared fallback';
                Primary = ''; Shared = $externalResourceRoot;
                Expected = $externalResourceRoot; Exit = 0 },
            [pscustomobject]@{ Name = 'invalid primary cannot fall back';
                Primary = $missingResourceRoot; Shared = $externalResourceRoot;
                Expected = $null; Exit = 2 },
            [pscustomobject]@{ Name = 'invalid shared cannot fall back';
                Primary = $null; Shared = $missingResourceRoot;
                Expected = $null; Exit = 2 }
        )
        try {
            foreach ($rootCase in $resourceRootCases) {
                if (-not (& $invokeRootCase -Name $rootCase.Name `
                        -Primary $rootCase.Primary -Shared $rootCase.Shared `
                        -Expected $rootCase.Expected `
                        -ExpectedExit $rootCase.Exit)) {
                    return
                }
            }
        }
        finally {
            & $setRootEnvironment 'LOSTARK_RESOURCE_ROOT' $previousResourceRoot
            & $setRootEnvironment 'LOSTARK_SHARED_ASSET_ROOT' $previousSharedRoot
            $global:LASTEXITCODE = 0
        }
        Write-Host '  Product Effect resource-root cases : 8'

        try {
            $ErrorActionPreference = 'Continue'
            $global:LASTEXITCODE = -1
            $probeOutput = @(& $probeExecutable $repositoryPath $Configuration 2>&1)
            $probeExitCode = $global:LASTEXITCODE
        }
        finally {
            $ErrorActionPreference = $previousNativeErrorAction
        }
        if ($probeExitCode -ne 0) {
            Add-Failure (
                "Product Effect shader WARP probe failed with code " +
                "${probeExitCode}: $($probeOutput -join ' ')")
            return
        }
        $jsonLine = @($probeOutput | Where-Object {
            $_.ToString().TrimStart().StartsWith('{')
        } | Select-Object -Last 1)
        if ($jsonLine.Count -ne 1) {
            Add-Failure (
                "Product Effect shader WARP probe emitted no result: " +
                ($probeOutput -join ' '))
            return
        }
        try {
            $result = $jsonLine[0].ToString() | ConvertFrom-Json
        }
        catch {
            Add-Failure "Product Effect shader WARP result is invalid JSON: $($_.Exception.Message)"
            return
        }
        if ($result.driver -ne 'WARP' -or
            [int64]$result.v1LitPixels -le 0 -or
            [int64]$result.v2LitPixels -le 0 -or
            $result.assetPathBoundaryValidated -ne $true -or
            [string]::IsNullOrWhiteSpace([string]$result.resourceRoot)) {
            Add-Failure (
                "Product Effect shader WARP result is incomplete: " +
                ($jsonLine[0].ToString()))
        }
        else {
            Write-Host (
                "  Product Effect WARP pixels : V1=$($result.v1LitPixels), " +
                "V2=$($result.v2LitPixels)")
        }
    }
    finally {
        if (Test-Path -LiteralPath $probeRoot -PathType Container) {
            $resolvedProbeRoot = (Resolve-Path -LiteralPath $probeRoot).Path
            if (-not $resolvedProbeRoot.StartsWith(
                $generatedRoot.TrimEnd('\') + '\',
                [StringComparison]::OrdinalIgnoreCase)) {
                throw "Product Effect shader WARP cleanup escaped generated root: $resolvedProbeRoot"
            }
            Remove-Item -LiteralPath $resolvedProbeRoot -Recurse -Force
        }
    }
}

$clientProjectPath = Join-Path $repositoryPath "Client\Default\Client.vcxproj"
$engineProjectPath = Join-Path $repositoryPath "Engine\Default\Engine.vcxproj"
$clientOutputDirectory = Join-Path $repositoryPath "Client\Bin\$Configuration"
$engineOutputDirectory = Join-Path $repositoryPath "Engine\Bin\$Configuration"

$producers = @()
$producers += Get-ActiveFxCompileItems `
    -ProjectPath $clientProjectPath `
    -Owner "Client" `
    -OutputDirectory $clientOutputDirectory
$producers += Get-ActiveFxCompileItems `
    -ProjectPath $engineProjectPath `
    -Owner "Engine" `
    -OutputDirectory $engineOutputDirectory

$producerByStem = @{}
foreach ($producer in $producers) {
    if (-not (Test-Path -LiteralPath $producer.SourcePath -PathType Leaf)) {
        Add-Failure "[$($producer.Owner)] FxCompile source is missing: $($producer.SourcePath)"
    }

    if ($producerByStem.ContainsKey($producer.Stem)) {
        $existing = $producerByStem[$producer.Stem]
        Add-Failure "Compiled shader basename collision '$($producer.Stem)': $($existing.SourcePath) <-> $($producer.SourcePath)"
        continue
    }
    $producerByStem[$producer.Stem] = $producer

    if (-not (Test-Path -LiteralPath $producer.OutputPath -PathType Leaf)) {
        Add-Failure "[$($producer.Owner)] compiled output is missing for $Configuration|${platform}: $($producer.OutputPath)"
        continue
    }
    if ((Get-Item -LiteralPath $producer.OutputPath).Length -le 0) {
        Add-Failure "[$($producer.Owner)] compiled output is empty: $($producer.OutputPath)"
    }
}

$engineRuntimeSource = Join-Path $repositoryPath "Engine\Private"
$runtimeCompilerCall = Get-ChildItem -LiteralPath $engineRuntimeSource -Recurse -File -Include *.cpp, *.h |
    Select-String -SimpleMatch "D3DX11CompileEffectFromFile"
foreach ($match in $runtimeCompilerCall) {
    Add-Failure "Runtime Effect source compiler call remains: $($match.Path):$($match.LineNumber)"
}

$shaderImplementation = Join-Path $repositoryPath "Engine\Private\Shader.cpp"
if (-not (Select-String -LiteralPath $shaderImplementation -SimpleMatch "D3DX11CreateEffectFromMemory" -Quiet)) {
    Add-Failure "CShader does not use D3DX11CreateEffectFromMemory: $shaderImplementation"
}

$clientConsumers = Get-DirectShaderConsumers @(
    (Join-Path $repositoryPath "Engine\Private"),
    (Join-Path $repositoryPath "Client\Private")
)

$modulesToValidate = [System.Collections.Generic.List[object]]::new()
$modulesToValidate.Add([pscustomobject]@{
    Name = "Client"
    ProjectPath = $clientProjectPath
    Directory = $clientOutputDirectory
    Consumers = $clientConsumers
})
if ($Modules -eq "All") {
    $modulesToValidate.Add([pscustomobject]@{
        Name = "PointLightFalloffContractHarness"
        ProjectPath = Join-Path $repositoryPath "Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj"
        Directory = Join-Path $repositoryPath "Tools\PointLightFalloffContractHarness\Bin\$Configuration"
        Consumers = @("Shader_Deferred.hlsl")
    })
}

foreach ($module in $modulesToValidate) {
    $projectText = if (Test-Path -LiteralPath $module.ProjectPath -PathType Leaf) {
        Get-Content -LiteralPath $module.ProjectPath -Raw
    }
    else {
        Add-Failure "[$($module.Name)] project is missing: $($module.ProjectPath)"
        ""
    }

    foreach ($shaderId in @($module.Consumers | Sort-Object -Unique)) {
        $stem = [System.IO.Path]::GetFileNameWithoutExtension($shaderId)
        if (-not $producerByStem.ContainsKey($stem)) {
            Add-Failure "[$($module.Name)] consumer has no active $Configuration|$platform FxCompile producer: $shaderId"
            continue
        }

        $producer = $producerByStem[$stem]
        $declaresExactDeployment =
            $projectText.IndexOf("$stem.cso", [System.StringComparison]::OrdinalIgnoreCase) -ge 0
        $declaresShaderWildcardDeployment =
            $projectText.IndexOf("Shader_*.cso", [System.StringComparison]::OrdinalIgnoreCase) -ge 0
        if ($producer.Owner -ne $module.Name -and
            -not $declaresExactDeployment -and
            -not $declaresShaderWildcardDeployment) {
            Add-Failure "[$($module.Name)] project does not declare deployment for $stem.cso"
        }

        $deploymentPath = Join-Path $module.Directory "$stem.cso"
        if (-not (Test-Path -LiteralPath $deploymentPath -PathType Leaf)) {
            Add-Failure "[$($module.Name)] module-adjacent compiled shader is missing: $deploymentPath"
            continue
        }
        if ((Get-Item -LiteralPath $deploymentPath).Length -le 0) {
            Add-Failure "[$($module.Name)] module-adjacent compiled shader is empty: $deploymentPath"
            continue
        }
        if (-not (Test-Path -LiteralPath $producer.OutputPath -PathType Leaf)) {
            continue
        }

        $producerHash = Get-FileHashValue $producer.OutputPath
        $deploymentHash = Get-FileHashValue $deploymentPath
        if (-not [string]::Equals(
            $producerHash,
            $deploymentHash,
            [System.StringComparison]::OrdinalIgnoreCase)) {
            Add-Failure "[$($module.Name)] deployed shader hash differs from $($producer.Owner) output: $deploymentPath"
        }

    }
}

Invoke-ProductEffectShaderWarpProbe

if ($script:failures.Count -gt 0) {
    Write-Host "Compiled shader closure FAILED for $Configuration|$platform" -ForegroundColor Red
    foreach ($failure in $script:failures) {
        Write-Host "  - $failure" -ForegroundColor Red
    }
    throw "Compiled shader closure failed with $($script:failures.Count) error(s)."
}

Write-Host "Compiled shader closure PASS for $Configuration|$platform" -ForegroundColor Green
Write-Host "  active FxCompile producers : $($producers.Count)"
foreach ($module in $modulesToValidate) {
    Write-Host "  $($module.Name) consumers : $(@($module.Consumers | Sort-Object -Unique).Count)"
}
