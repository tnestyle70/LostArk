[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug',
    [string]$OutputDirectory = ''
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
$taskCommand = 'call "{0}" -no_logo -arch=x64 -host_arch=x64 && cl.exe /nologo /std:c++20 /EHsc /W4 {1} /DUNICODE /D_UNICODE /I"{2}" "{3}" /Fo:"{4}" /Fe:"{5}" /link /INCREMENTAL:NO /LIBPATH:"{6}" Engine.lib d3d11.lib dxgi.lib' -f `
    $taskDevCmd, $taskRuntime, (Join-Path $taskRepo 'Engine/Public'), $taskSource, (Join-Path $taskOutput 'probe.obj'), $taskExecutable, $taskEngine
& $env:COMSPEC /d /s /c $taskCommand *> (Join-Path $taskOutput 'compile.log')
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
& $taskExecutable $taskOutput *> (Join-Path $taskOutput 'probe.log')
$taskCode = $LASTEXITCODE
Get-Content (Join-Path $taskOutput 'probe.log')
if ($taskCode -ne 0) { throw "SourceCharacter probe failed with exit $taskCode." }
