[CmdletBinding()]
param(
    [string]$PythonPath = '',
    [string]$DestinationRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Every Python helper in this script emits UTF-8. Windows PowerShell decodes
# native stdout with the console codepage, so pin both ends to UTF-8 before
# any capture. This process is the one the operator launches, so the change
# does not leak into an interactive session.
$env:PYTHONIOENCODING = 'utf-8'
$env:PYTHONUTF8 = '1'
try {
    [Console]::OutputEncoding = [Text.UTF8Encoding]::new($false)
    $OutputEncoding = [Text.UTF8Encoding]::new($false)
}
catch {
    Write-Warning 'Could not pin the console to UTF-8; non-ASCII paths may fail.'
}

function Resolve-RequiredFile {
    param([string]$Path, [string]$Label)

    $resolved = Resolve-Path -LiteralPath $Path -ErrorAction Stop
    if (-not (Test-Path -LiteralPath $resolved.Path -PathType Leaf)) {
        throw "$Label is not a file: $Path"
    }
    return $resolved.Path
}

function Resolve-Python {
    param([string]$Requested, [string]$RepositoryRoot)

    if ($Requested) {
        return Resolve-RequiredFile -Path $Requested -Label 'Python'
    }

    $command = Get-Command python.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $bundled = Join-Path $env:USERPROFILE '.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
    if (Test-Path -LiteralPath $bundled -PathType Leaf) {
        return (Resolve-Path -LiteralPath $bundled).Path
    }

    throw 'Python 3 was not found. Pass -PythonPath explicitly.'
}

function Invoke-NativeChecked {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$WorkingDirectory,
        [string]$Label
    )

    Push-Location -LiteralPath $WorkingDirectory
    try {
        $lines = @(& $FilePath @Arguments 2>&1 | ForEach-Object { "$_" })
        $exitCode = $LASTEXITCODE
    }
    finally {
        Pop-Location
    }
    if ($exitCode -ne 0) {
        throw "$Label failed with exit code $exitCode.`n$($lines -join [Environment]::NewLine)"
    }
    return @($lines)
}

function Invoke-PythonSource {
    param(
        [string]$Python,
        [string]$Source,
        [string[]]$Arguments,
        [string]$WorkingDirectory,
        [string]$Label
    )

    # Windows PowerShell 5.1 cannot hand a multi-line here-string to
    # `python -c` as one native argument, so the source is staged as a
    # temporary module. sys.argv indexing is identical either way.
    $file = Join-Path ([IO.Path]::GetTempPath()) `
        ('lostark-kakul-' + [Guid]::NewGuid().ToString('N') + '.py')
    [IO.File]::WriteAllText($file, $Source, (New-Object Text.UTF8Encoding($false)))
    try {
        return Invoke-NativeChecked -FilePath $Python `
            -Arguments (@($file) + $Arguments) `
            -WorkingDirectory $WorkingDirectory -Label $Label
    }
    finally {
        Remove-Item -LiteralPath $file -Force -ErrorAction SilentlyContinue
    }
}

function Get-RelativeResourcePath {
    param([string]$Root, [string]$Target)

    # [IO.Path]::GetRelativePath is .NET Core only; Windows PowerShell runs on
    # .NET Framework, so the relative path is derived through Uri instead.
    $rootFull = [IO.Path]::GetFullPath($Root)
    if (-not $rootFull.EndsWith([IO.Path]::DirectorySeparatorChar)) {
        $rootFull += [IO.Path]::DirectorySeparatorChar
    }
    $targetFull = [IO.Path]::GetFullPath($Target)
    if (-not $targetFull.StartsWith($rootFull, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Path is not below the resource root: $targetFull"
    }
    return $targetFull.Substring($rootFull.Length).Replace('\', '/')
}

function Get-Sha256 {
    param([string]$Path)
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Convert-GltfWeightsToFloat {
    param(
        [string]$Python,
        [string]$Gltf,
        [string]$WorkingDirectory
    )

    # The repository converter's Assimp build does not safely ingest UModel's
    # legal normalized UBYTE/USHORT WEIGHTS_0 accessors.  Promote only that
    # vertex channel in disposable staging input so WModel blend data is finite
    # and deterministic; the extracted source files remain byte-for-byte intact.
    $code = @'
import json
import math
from pathlib import Path
import struct
import sys

gltf = Path(sys.argv[1]).resolve(strict=True)
document = json.loads(gltf.read_text(encoding="utf-8"))
buffers = document.get("buffers")
if not isinstance(buffers, list) or len(buffers) != 1:
    raise ValueError("staged glTF must contain exactly one buffer")
uri = buffers[0].get("uri")
if not isinstance(uri, str) or not uri or ":" in uri or "/" in uri or "\\" in uri:
    raise ValueError("staged glTF buffer URI must be a local file name")
buffer_path = (gltf.parent / uri).resolve(strict=True)
if buffer_path.parent != gltf.parent:
    raise ValueError("staged glTF buffer escapes its directory")
payload = bytearray(buffer_path.read_bytes())
component = {
    5121: ("B", 1, 255.0),
    5123: ("H", 2, 65535.0),
}
promoted = 0
for mesh in document.get("meshes", []):
    for primitive in mesh.get("primitives", []):
        attributes = primitive.get("attributes", {})
        accessor_index = attributes.get("WEIGHTS_0")
        if not isinstance(accessor_index, int):
            raise ValueError("skinned primitive has no WEIGHTS_0 accessor")
        accessor = document["accessors"][accessor_index]
        if accessor.get("type") != "VEC4":
            raise ValueError("WEIGHTS_0 must be VEC4")
        component_type = accessor.get("componentType")
        if component_type == 5126:
            continue
        if component_type not in component or accessor.get("normalized") is not True:
            raise ValueError("integer WEIGHTS_0 must be normalized UBYTE or USHORT")
        view = document["bufferViews"][accessor["bufferView"]]
        if view.get("buffer") != 0:
            raise ValueError("WEIGHTS_0 references an unsupported buffer")
        fmt, width, denominator = component[component_type]
        element_size = width * 4
        stride = view.get("byteStride", element_size)
        if not isinstance(stride, int) or stride < element_size:
            raise ValueError("WEIGHTS_0 stride is invalid")
        start = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
        count = accessor.get("count")
        if not isinstance(count, int) or count <= 0 or start < 0 or start + (count - 1) * stride + element_size > len(payload):
            raise ValueError("WEIGHTS_0 span is out of range")
        rows = []
        for index in range(count):
            raw = struct.unpack_from("<" + fmt * 4, payload, start + index * stride)
            value = tuple(component / denominator for component in raw)
            total = sum(value)
            if not all(math.isfinite(component) and component >= 0.0 for component in value) or total <= 1e-6:
                raise ValueError("WEIGHTS_0 contains an invalid vertex")
            rows.append(tuple(component / total for component in value))
        while len(payload) % 4:
            payload.append(0)
        byte_offset = len(payload)
        for row in rows:
            payload.extend(struct.pack("<4f", *row))
        view_index = len(document["bufferViews"])
        document["bufferViews"].append({
            "buffer": 0,
            "byteOffset": byte_offset,
            "byteLength": len(rows) * 16,
        })
        new_accessor = len(document["accessors"])
        document["accessors"].append({
            "bufferView": view_index,
            "componentType": 5126,
            "count": len(rows),
            "type": "VEC4",
        })
        attributes["WEIGHTS_0"] = new_accessor
        promoted += 1
if promoted <= 0:
    raise ValueError("no normalized integer WEIGHTS_0 accessor was promoted")
while len(payload) % 4:
    payload.append(0)
document["buffers"][0]["byteLength"] = len(payload)
buffer_path.write_bytes(payload)
gltf.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print(f"GLTF_WEIGHT_PROMOTION_OK primitives={promoted}")
'@

    Invoke-PythonSource -Python $Python -Source $code -Arguments @($Gltf) `
        -WorkingDirectory $WorkingDirectory -Label 'glTF weight promotion' | Out-Null
}

function Assert-NearlyEqual {
    param([double]$Actual, [double]$Expected, [double]$Tolerance, [string]$Label)
    if ([double]::IsNaN($Actual) -or [double]::IsInfinity($Actual) -or
        [math]::Abs($Actual - $Expected) -gt $Tolerance) {
        throw "$Label differs. expected=$Expected actual=$Actual"
    }
}

function Read-WModelMetadata {
    param(
        [string]$Python,
        [string]$ReaderScript,
        [string]$MeshReaderScript,
        [string]$WModel,
        [string]$WorkingDirectory
    )

    $code = @'
import json
from pathlib import Path
import runpy
import struct
import sys

reader = runpy.run_path(sys.argv[1], run_name="kakul_wmodel_reader")
path = Path(sys.argv[2])
data = path.read_bytes()
animations = reader["read_wmodel_animation_sections"](data)
mesh_reader = runpy.run_path(sys.argv[3], run_name="kakul_wmodel_mesh_reader")
model = mesh_reader["read_wmodel"](path)
maximum_weight_error = 0.0
for vertex_index, vertex in enumerate(model.vertices):
    if any(index < 0 or index >= len(model.skeleton_bones) for index in vertex.indices):
        raise ValueError(f"WModel vertex {vertex_index} has an invalid blend index")
    if any(not __import__("math").isfinite(weight) or weight < 0.0 for weight in vertex.weights):
        raise ValueError(f"WModel vertex {vertex_index} has an invalid blend weight")
    total = sum(vertex.weights)
    if total <= 1e-6:
        raise ValueError(f"WModel vertex {vertex_index} has no positive blend weight")
    maximum_weight_error = max(maximum_weight_error, abs(total - 1.0))
if maximum_weight_error > 1e-4:
    raise ValueError(f"WModel blend weight sum differs by {maximum_weight_error}")
file_header = reader["FILE_HEADER"]
model_header = reader["MODEL_HEADER"]
section_desc = reader["SECTION_DESC"]
_, section_count, _, _, *_ = model_header.unpack_from(data, file_header.size)
table = file_header.size + model_header.size
materials = []
for row in range(section_count):
    type_id, section_index, relative, size, _ = section_desc.unpack_from(
        data, table + row * section_desc.size
    )
    if type_id != 2:
        continue
    section = file_header.size + relative
    magic, major, _, flags, content_size = file_header.unpack_from(data, section)
    if magic != b"WINT" or major != 1 or flags != 0 or content_size != size - file_header.size:
        raise ValueError("WModel material WINT header is invalid")
    cursor = section + file_header.size
    if data[cursor:cursor + 4] != b"WMA2":
        raise ValueError("WModel material payload is not WMA2")
    count = struct.unpack_from("<I", data, cursor + 4)[0]
    cursor += 8
    if content_size != 8 + count * 4756:
        raise ValueError("WModel material payload size is invalid")
    slot_names = (
        "base", "normal", "specular", "emissive", "opacity", "orm",
        "metallic", "roughness", "ambientOcclusion",
    )
    for _ in range(count):
        index = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        cursor += 8
        raw_name = data[cursor:cursor + 64]
        cursor += 64
        name = raw_name.split(b"\0", 1)[0].decode("ascii", "strict")
        value = {"index": index, "name": name}
        for slot in slot_names:
            raw_path = data[cursor:cursor + 520]
            cursor += 520
            value[slot] = raw_path.decode("utf-16-le", "strict").split("\0", 1)[0]
        materials.append(value)

print(json.dumps({
    "animations": animations,
    "materials": materials,
    "skinning": {
        "vertexCount": len(model.vertices),
        "boneCount": len(model.skeleton_bones),
        "maximumWeightSumError": maximum_weight_error,
    },
}, separators=(",", ":")))
'@

    $output = @(Invoke-PythonSource -Python $Python -Source $code `
        -Arguments @($ReaderScript, $WModel, $MeshReaderScript) `
        -WorkingDirectory $WorkingDirectory -Label 'WModel metadata validation')
    if ($output.Count -ne 1) {
        throw "WModel metadata validator returned unexpected output: $($output -join ' ')"
    }
    return ($output[0] | ConvertFrom-Json)
}

function Assert-CookedTexture {
    param(
        [object[]]$Materials,
        [string]$MaterialName,
        [string]$Slot,
        [string]$ExpectedFileName,
        [string]$ExpectedSource,
        [string]$PackageRoot
    )

    $matches = @($Materials | Where-Object { $_.name -ceq $MaterialName })
    if ($matches.Count -ne 1) {
        throw "Material '$MaterialName' did not resolve exactly once in the WModel."
    }
    $stored = [string]$matches[0].$Slot
    $expectedStored = "textures/$ExpectedFileName"
    if ($stored -cne $expectedStored) {
        throw "Material texture differs. material=$MaterialName slot=$Slot expected=$expectedStored actual=$stored"
    }
    if ([IO.Path]::IsPathRooted($stored) -or $stored.Contains('..') -or $stored.Contains('\')) {
        throw "WModel stores an unsafe texture path: $stored"
    }
    $physical = Join-Path $PackageRoot ($stored.Replace('/', [IO.Path]::DirectorySeparatorChar))
    if (-not (Test-Path -LiteralPath $physical -PathType Leaf)) {
        throw "Cooked texture is missing: $physical"
    }
    if ((Get-Sha256 $physical) -cne (Get-Sha256 $ExpectedSource)) {
        throw "Cooked texture bytes differ from the source: $physical"
    }
    return [PSCustomObject]@{
        material = $MaterialName
        slot = $Slot
        path = $stored
        sha256 = Get-Sha256 $physical
    }
}

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$resourceRoot = Join-Path $repoRoot 'Client\Bin\Resources'
$mapRoot = Join-Path $resourceRoot 'Map\LV_LUT_MIDNIGHTC_ED'
$builder = Resolve-RequiredFile `
    -Path (Join-Path $PSScriptRoot 'build_umodel_gltf_psa.py') `
    -Label 'UModel glTF/PSA staging tool'
$converter = Resolve-RequiredFile `
    -Path (Join-Path $repoRoot 'Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe') `
    -Label 'ModelAssetConverter'
$reader = Resolve-RequiredFile `
    -Path (Join-Path $repoRoot 'Tools\ModelAssetConverter\retime_wmodel_from_psa.py') `
    -Label 'WModel animation reader'
$meshReader = Resolve-RequiredFile `
    -Path (Join-Path $repoRoot 'Tools\ModelAssetConverter\verify_dimensionmaster_summon_bind_pose.py') `
    -Label 'WModel mesh reader'
$retimer = Resolve-RequiredFile `
    -Path (Join-Path $repoRoot 'Tools\ActorXAssetCooker\retime_wmodel_ticks.py') `
    -Label 'WModel tick retimer'
# CAnimation ignores the rate stored in a cooked clip and always advances at
# COOKED_TICK_RATE. Clips are re-expressed on that rate so this Area plays at
# the authored speed without changing shared Engine timing for other assets.
$runtimeTicksPerSecond = '30'
$converterTicksPerSecond = '1000'
$python = Resolve-Python -Requested $PythonPath -RepositoryRoot $repoRoot

if (-not $DestinationRoot) {
    $DestinationRoot = Join-Path $mapRoot 'AnimatedProps'
}
$destination = [IO.Path]::GetFullPath($DestinationRoot)
$expectedParent = [IO.Path]::GetFullPath($mapRoot) + [IO.Path]::DirectorySeparatorChar
if (-not $destination.StartsWith($expectedParent, [StringComparison]::OrdinalIgnoreCase)) {
    throw "DestinationRoot must be below the Kakul map resource directory: $destination"
}
if (Test-Path -LiteralPath $destination) {
    throw "DestinationRoot already exists; refusing to overwrite runtime resources: $destination"
}

$leverRaw = Join-Path $mapRoot '_레버_ITR_02283'
$paperRaw = Join-Path $mapRoot '_종이펼침_paperstage'
$assets = @(
    [PSCustomObject]@{
        AssetName = 'DEPLOY_ITR_02283'
        RawRoot = $leverRaw
        Gltf = 'mesh\ITR_02283\SkeletalMesh3\itr_02283_sk.gltf'
        Psa = 'anim\ITR_02283\AnimSet\itr_02283_ani.psa'
        Clips = [ordered]@{
            go_off = 60.0 / 30.0
            go_on = 40.0 / 30.0
            off = 1.0 / 30.0
            on = 1.0 / 30.0
        }
        Base = @(
            [PSCustomObject]@{ Material = 'itr_02283_01_mi'; Texture = 'mesh\BG_RAD_KOUKUSATON_F\Texture2D\bg_rad_koukusaton_deco22_d_khb.dds' },
            [PSCustomObject]@{ Material = 'itr_02283_02_mi'; Texture = 'mesh\BG_RAD_KOUKUSATON_F\Texture2D\bg_rad_koukusaton_deco22a_d_khb.dds' }
        )
        Normal = @(
            [PSCustomObject]@{ Material = 'itr_02283_02_mi'; Texture = 'mesh\BG_RAD_KOUKUSATON_F\Texture2D\bg_rad_koukusaton_deco22a_n_khb.dds' }
        )
    },
    [PSCustomObject]@{
        AssetName = 'DEPLOY_BG_RAD_KOUKUSATON_PAPERSTAGE'
        RawRoot = $paperRaw
        Gltf = 'mesh\CINE_PROB_09_S2_2012\SkeletalMesh3\bg_rad_koukusaton_paperstage.gltf'
        Psa = 'anim_펼침\bg_rad_koukusaton_paperstage_evt2_ani.psa'
        Clips = [ordered]@{
            evt2_paperstage_open01 = 92.0 / 30.0
        }
        Base = @(
            [PSCustomObject]@{ Material = 'bg_rad_koukusaton_book01a_mi_khg'; Texture = 'mesh\BG_RAD_KOUKUSATON_D\Texture2D\bg_rad_koukusaton_book01a_da_khg_loc_int.dds' }
        )
        Normal = @()
    }
)

# Preflight every source before creating an output directory.
foreach ($asset in $assets) {
    Resolve-RequiredFile -Path (Join-Path $asset.RawRoot $asset.Gltf) -Label "$($asset.AssetName) glTF" | Out-Null
    Resolve-RequiredFile -Path (Join-Path $asset.RawRoot $asset.Psa) -Label "$($asset.AssetName) PSA" | Out-Null
    foreach ($mapping in @($asset.Base) + @($asset.Normal)) {
        Resolve-RequiredFile -Path (Join-Path $asset.RawRoot $mapping.Texture) -Label "$($asset.AssetName) texture" | Out-Null
    }
}

$tempBase = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$workRoot = Join-Path $tempBase ('lostark-kakul-animated-props-' + [Guid]::NewGuid().ToString('N'))
$candidateRoot = Join-Path $workRoot 'AnimatedProps'
New-Item -ItemType Directory -Path $candidateRoot | Out-Null

try {
    $summary = New-Object 'System.Collections.Generic.List[object]'
    foreach ($asset in $assets) {
        $assetWork = Join-Path $workRoot ('work-' + $asset.AssetName)
        $package = Join-Path $candidateRoot $asset.AssetName
        $sourceTextures = Join-Path $assetWork 'source-textures'
        New-Item -ItemType Directory -Path $assetWork, $package, $sourceTextures | Out-Null

        $sourceGltf = Resolve-RequiredFile -Path (Join-Path $asset.RawRoot $asset.Gltf) -Label 'glTF'
        $sourcePsa = Resolve-RequiredFile -Path (Join-Path $asset.RawRoot $asset.Psa) -Label 'PSA'
        $stagedGltf = Join-Path $assetWork 'staged.gltf'
        $stagedBin = Join-Path $assetWork 'staged.bin'
        $stageReport = Join-Path $assetWork 'stage.json'
        Invoke-NativeChecked -FilePath $python `
            -Arguments @($builder, '--gltf', $sourceGltf, '--psa', $sourcePsa,
                '--output-gltf', $stagedGltf, '--output-bin', $stagedBin, '--report', $stageReport,
                '--scale', '100') `
            -WorkingDirectory $repoRoot -Label "$($asset.AssetName) glTF/PSA staging" | Out-Null
        Convert-GltfWeightsToFloat -Python $python -Gltf $stagedGltf -WorkingDirectory $repoRoot

        $converterArguments = @('staged.gltf', '-o', "$($asset.AssetName).wmodel")
        $expectedTextureSources = @{}
        foreach ($mapping in $asset.Base) {
            $source = Resolve-RequiredFile -Path (Join-Path $asset.RawRoot $mapping.Texture) -Label 'base texture'
            $name = [IO.Path]::GetFileName($source)
            $copy = Join-Path $sourceTextures $name
            Copy-Item -LiteralPath $source -Destination $copy
            $converterArguments += @('--material-remap', "$($mapping.Material)=$copy")
            $expectedTextureSources["$($mapping.Material)|base"] = $source
        }
        foreach ($mapping in $asset.Normal) {
            $source = Resolve-RequiredFile -Path (Join-Path $asset.RawRoot $mapping.Texture) -Label 'normal texture'
            $name = [IO.Path]::GetFileName($source)
            $copy = Join-Path $sourceTextures $name
            Copy-Item -LiteralPath $source -Destination $copy
            $converterArguments += @('--normal-remap', "$($mapping.Material)=$copy")
            $expectedTextureSources["$($mapping.Material)|normal"] = $source
        }
        $converterArguments += '--no-auto-textures'
        Invoke-NativeChecked -FilePath $converter -Arguments $converterArguments `
            -WorkingDirectory $assetWork -Label "$($asset.AssetName) WModel cook" | Out-Null

        $wmodel = Resolve-RequiredFile -Path (Join-Path $assetWork "$($asset.AssetName).wmodel") -Label 'cooked WModel'
        Invoke-NativeChecked -FilePath $python `
            -Arguments @($retimer, '--wmodel', $wmodel,
                '--ticks-per-second', $runtimeTicksPerSecond,
                '--expect-ticks-per-second', $converterTicksPerSecond) `
            -WorkingDirectory $repoRoot `
            -Label "$($asset.AssetName) WModel tick retime" | Out-Null
        $metadata = Read-WModelMetadata -Python $python -ReaderScript $reader `
            -MeshReaderScript $meshReader -WModel $wmodel -WorkingDirectory $repoRoot
        $animations = @($metadata.animations)
        if ($animations.Count -ne $asset.Clips.Count) {
            throw "$($asset.AssetName) animation count differs. expected=$($asset.Clips.Count) actual=$($animations.Count)"
        }
        foreach ($entry in $asset.Clips.GetEnumerator()) {
            $matches = @($animations | Where-Object { $_.name -ceq $entry.Key })
            if ($matches.Count -ne 1) {
                throw "$($asset.AssetName) clip '$($entry.Key)' did not resolve exactly once."
            }
            Assert-NearlyEqual -Actual ([double]$matches[0].ticksPerSecond) `
                -Expected ([double]$runtimeTicksPerSecond) `
                -Tolerance 0.001 -Label "$($asset.AssetName).$($entry.Key) ticksPerSecond"
            $seconds = [double]$matches[0].durationTicks / [double]$matches[0].ticksPerSecond
            Assert-NearlyEqual -Actual $seconds -Expected ([double]$entry.Value) `
                -Tolerance 0.00001 -Label "$($asset.AssetName).$($entry.Key) durationSeconds"
        }

        $textureRows = New-Object 'System.Collections.Generic.List[object]'
        foreach ($mapping in $asset.Base) {
            $source = $expectedTextureSources["$($mapping.Material)|base"]
            $textureRows.Add((Assert-CookedTexture -Materials @($metadata.materials) `
                -MaterialName $mapping.Material -Slot 'base' -ExpectedFileName ([IO.Path]::GetFileName($source)) `
                -ExpectedSource $source -PackageRoot $assetWork))
        }
        foreach ($mapping in $asset.Normal) {
            $source = $expectedTextureSources["$($mapping.Material)|normal"]
            $textureRows.Add((Assert-CookedTexture -Materials @($metadata.materials) `
                -MaterialName $mapping.Material -Slot 'normal' -ExpectedFileName ([IO.Path]::GetFileName($source)) `
                -ExpectedSource $source -PackageRoot $assetWork))
        }

        $info = Invoke-NativeChecked -FilePath $converter -Arguments @('info', $wmodel) `
            -WorkingDirectory $assetWork -Label "$($asset.AssetName) WModel info"
        $infoPath = Join-Path $assetWork "$($asset.AssetName).wmodel.info.txt"
        [IO.File]::WriteAllText($infoPath, ($info -join [Environment]::NewLine) + [Environment]::NewLine,
            [Text.UTF8Encoding]::new($false))

        $stage = Get-Content -LiteralPath $stageReport -Raw -Encoding UTF8 |
            ConvertFrom-Json
        $resourceRelativeWModel = 'Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/' +
            $asset.AssetName + '/' + $asset.AssetName + '.wmodel'
        $animationRows = @()
        foreach ($animation in $animations) {
            $animationRows += [PSCustomObject][ordered]@{
                name = [string]$animation.name
                durationTicks = [double]$animation.durationTicks
                ticksPerSecond = [double]$animation.ticksPerSecond
                durationSeconds = [double]$animation.durationTicks / [double]$animation.ticksPerSecond
            }
        }
        $receipt = [ordered]@{
            schema = 'lostark.kakul-interaction-prop-cook'
            formatVersion = 1
            assetId = $asset.AssetName
            runtimeAssetId = $resourceRelativeWModel
            source = [ordered]@{
                gltf = ('Map/LV_LUT_MIDNIGHTC_ED/' +
                    (Get-RelativeResourcePath -Root $mapRoot -Target $sourceGltf))
                gltfSha256 = Get-Sha256 $sourceGltf
                bufferSha256 = [string]$stage.source.bufferSha256
                psa = ('Map/LV_LUT_MIDNIGHTC_ED/' +
                    (Get-RelativeResourcePath -Root $mapRoot -Target $sourcePsa))
                psaSha256 = Get-Sha256 $sourcePsa
            }
            animations = $animationRows
            skinning = [ordered]@{
                vertexCount = [int]$metadata.skinning.vertexCount
                boneCount = [int]$metadata.skinning.boneCount
                maximumWeightSumError = [double]$metadata.skinning.maximumWeightSumError
            }
            textures = $textureRows.ToArray()
            output = [ordered]@{
                wmodel = ($asset.AssetName + '.wmodel')
                wmodelSha256 = Get-Sha256 $wmodel
                info = ($asset.AssetName + '.wmodel.info.txt')
            }
        }
        $receiptPath = Join-Path $assetWork "$($asset.AssetName).cook.json"
        [IO.File]::WriteAllText($receiptPath,
            ($receipt | ConvertTo-Json -Depth 12) + [Environment]::NewLine,
            [Text.UTF8Encoding]::new($false))

        Move-Item -LiteralPath $wmodel -Destination (Join-Path $package ([IO.Path]::GetFileName($wmodel)))
        Move-Item -LiteralPath $infoPath -Destination (Join-Path $package ([IO.Path]::GetFileName($infoPath)))
        Move-Item -LiteralPath $receiptPath -Destination (Join-Path $package ([IO.Path]::GetFileName($receiptPath)))
        Move-Item -LiteralPath (Join-Path $assetWork 'textures') -Destination (Join-Path $package 'textures')

        $summary.Add([PSCustomObject]@{
            AssetId = $asset.AssetName
            RuntimeAssetId = $resourceRelativeWModel
            WModelSha256 = $receipt.output.wmodelSha256
            Animations = $animationRows
            Textures = $textureRows.ToArray()
        })
    }

    New-Item -ItemType Directory -Path ([IO.Path]::GetDirectoryName($destination)) -Force | Out-Null
    Move-Item -LiteralPath $candidateRoot -Destination $destination
    Write-Host "KAKUL_INTERACTION_PROPS_COOK_OK destination=$destination assets=$($summary.Count)"
    $summary.ToArray() | ConvertTo-Json -Depth 10
}
finally {
    $resolvedWork = [IO.Path]::GetFullPath($workRoot)
    if ($resolvedWork.StartsWith($tempBase, [StringComparison]::OrdinalIgnoreCase) -and
        [IO.Path]::GetFileName($resolvedWork).StartsWith('lostark-kakul-animated-props-', [StringComparison]::Ordinal)) {
        if (Test-Path -LiteralPath $resolvedWork) {
            Remove-Item -LiteralPath $resolvedWork -Recurse -Force
        }
    }
}
