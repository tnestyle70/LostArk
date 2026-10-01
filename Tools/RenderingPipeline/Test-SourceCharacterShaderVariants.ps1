[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug',
    [string]$OutputDirectory = '',
    [ValidateSet('All', 'MaterialLightRows')][string]$Focus = 'All',
    [string]$MaterialSourcePath = '',
    [ValidatePattern('^$|^\d+\.\d+(\.\d+)?$')][string]$VCToolsVersion = ''
)
$ErrorActionPreference = 'Stop'
$taskRepo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $taskRepo 'out/SourceCharacterShaderVariants'
}
$taskOutput = [IO.Path]::GetFullPath($OutputDirectory)
$taskExpectedRoot = [IO.Path]::GetFullPath((Join-Path $taskRepo 'out')) + [IO.Path]::DirectorySeparatorChar
if (-not $taskOutput.StartsWith($taskExpectedRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'The probe output must remain inside the repository out directory.'
}
New-Item -ItemType Directory -Force -Path $taskOutput | Out-Null
$taskVsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$taskVs = [string](@(& $taskVsWhere -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath) | Select-Object -First 1)
if ([string]::IsNullOrWhiteSpace($taskVs)) { throw 'Visual C++ toolchain was not found.' }
$taskDevCmd = Join-Path $taskVs 'Common7/Tools/VsDevCmd.bat'
$taskEngine = Join-Path $taskRepo "Engine/Bin/$Configuration"
$taskClient = Join-Path $taskRepo "Client/Bin/$Configuration"
$taskRuntime = if ($Configuration -eq 'Debug') { '/MDd /D_DEBUG' } else { '/MD /DNDEBUG' }
$taskSource = Join-Path $PSScriptRoot 'SourceCharacterShaderVariantProbe.cpp'
$taskExecutable = Join-Path $taskOutput 'SourceCharacterShaderVariantProbe.exe'
$taskExtraCompile = ''; $taskExtraLibraries = ''; $taskArguments = @($taskOutput)
$taskToolchainArgument = if ($VCToolsVersion) { '-vcvars_ver=' + $VCToolsVersion } else { '' }
if ($Focus -eq 'MaterialLightRows') {
    if ([string]::IsNullOrWhiteSpace($MaterialSourcePath)) { $MaterialSourcePath = Join-Path $taskRepo 'Engine/Private/Material.cpp' }
    $MaterialSourcePath = (Resolve-Path -LiteralPath $MaterialSourcePath).Path
    # CMaterial is internal to Engine, so compile its actual translation unit as
    # the existing light harness does. CShader still comes from the product DLL.
    $taskExtraCompile = '/DSOURCE_MATERIAL_LIGHT_ROW_PROBE /I"{0}" /I"{1}" "{2}"' -f `
        (Join-Path $taskRepo 'Engine/ThirdPartyLib/FMOD/Inc'), (Join-Path $taskRepo 'Engine/ThirdPartyLib/PhysX/Inc'), $MaterialSourcePath
    $taskNativeLibraries = if ($Configuration -eq 'Debug') { 'DirectXTKd.lib assimp-vc143-mtd.lib' } else { 'DirectXTK.lib assimp-vc143-mt.lib' }
    $taskExtraLibraries = '/LIBPATH:"{0}" {1} windowscodecs.lib ole32.lib' -f (Join-Path $taskRepo 'Engine/ThirdPartyLib'), $taskNativeLibraries
    $taskArguments += '--material-light-rows-only'
}
$taskCommand = 'call "{0}" -no_logo -arch=x64 -host_arch=x64 {9} && cl.exe /nologo /std:c++20 /EHsc /W4 {1} /DUNICODE /D_UNICODE /I"{2}" "{3}" {7} /Fo:"{4}\\" /Fe:"{5}" /link /INCREMENTAL:NO /LIBPATH:"{6}" Engine.lib d3d11.lib dxgi.lib {8}' -f `
    $taskDevCmd, $taskRuntime, (Join-Path $taskRepo 'Engine/Public'), $taskSource, $taskOutput, $taskExecutable, $taskEngine, $taskExtraCompile, $taskExtraLibraries, $taskToolchainArgument
$taskCommand += ' > "{0}" 2>&1' -f (Join-Path $taskOutput 'compile.log')
& $env:COMSPEC /d /s /c $taskCommand
if ($LASTEXITCODE -ne 0) { Get-Content (Join-Path $taskOutput 'compile.log') -Tail 25; throw 'SourceCharacter probe compilation failed.' }

# The executable resolves shaders beside itself. Missing/corrupt-input fixtures
# therefore operate only on these isolated copies, never installed CSOs.
foreach ($taskDirectory in @($taskClient, $taskEngine)) {
    Get-ChildItem -LiteralPath $taskDirectory -Filter '*.dll' | Copy-Item -Destination $taskOutput -Force
}
foreach ($taskStem in @('Shader_VtxAnimMeshBinary', 'Shader_VtxMeshBinary', 'Shader_Deferred')) {
    $taskDirectory = if ($taskStem -eq 'Shader_Deferred') { $taskEngine } else { $taskClient }
    Copy-Item -LiteralPath (Join-Path $taskDirectory ($taskStem + '.cso')) -Destination $taskOutput -Force
    Get-ChildItem -LiteralPath $taskDirectory -Filter ($taskStem + '_SourceGroup*.cso') |
        Copy-Item -Destination $taskOutput -Force
}
$taskPreviousErrorAction = $ErrorActionPreference
$taskCode = -1
try {
    $ErrorActionPreference = 'Continue' # Windows PowerShell wraps expected native stderr failures.
    $LASTEXITCODE = -1 # A launch failure must not inherit the successful compiler's exit code.
    & $taskExecutable @taskArguments *> (Join-Path $taskOutput 'probe.log')
    $taskCode = $LASTEXITCODE
}
finally { $ErrorActionPreference = $taskPreviousErrorAction }
Get-Content (Join-Path $taskOutput 'probe.log')
if ($Focus -eq 'MaterialLightRows') {
    $taskNumbers = @(Get-Content (Join-Path $taskOutput 'probe.log') | Where-Object { $_.StartsWith('{"materialLightRows":') })
    $taskSummary = [ordered]@{
        configuration = $Configuration; focus = $Focus; exitCode = $taskCode
        materialSourcePath = $MaterialSourcePath
        materialSourceSha256 = (Get-FileHash -LiteralPath $MaterialSourcePath -Algorithm SHA256).Hash
        probeSourceSha256 = (Get-FileHash -LiteralPath $taskSource -Algorithm SHA256).Hash
        engineDllSha256 = (Get-FileHash -LiteralPath (Join-Path $taskOutput 'Engine.dll') -Algorithm SHA256).Hash
        numericResults = if ($taskNumbers.Count) { $taskNumbers[-1] | ConvertFrom-Json } else { $null }
        limits = 'Actual Material.cpp and product CShader on headless WARP. Synthetic program81 GBuffer/mip textures; no Client execution, installed scene capture, FPS or hardware GPU timing.'
    }
    $taskSummary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $taskOutput 'material-light-rows-result.json') -Encoding UTF8
}
if ($taskCode -ne 0) { throw "SourceCharacter probe failed with exit $taskCode." }
