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

    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
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

$effectHarnessConsumers = $clientConsumers

$modulesToValidate = [System.Collections.Generic.List[object]]::new()
$modulesToValidate.Add([pscustomobject]@{
    Name = "Client"
    ProjectPath = $clientProjectPath
    Directory = $clientOutputDirectory
    Consumers = $clientConsumers
})
if ($Modules -eq "All") {
    $modulesToValidate.Add([pscustomobject]@{
        Name = "EffectRenderContractHarness"
        ProjectPath = Join-Path $repositoryPath "Tools\EffectRenderContractHarness\Default\EffectRenderContractHarness.vcxproj"
        Directory = Join-Path $repositoryPath "Tools\EffectRenderContractHarness\Bin\$Configuration"
        Consumers = $effectHarnessConsumers
    })
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
