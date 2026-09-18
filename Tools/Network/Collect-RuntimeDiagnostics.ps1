# Standalone local capture; no upload, environment dump, resource scan or process changes.
# Example: powershell -NoProfile -ExecutionPolicy Bypass -File Collect-RuntimeDiagnostics.ps1 -RepositoryRoot C:\LostArk
#requires -Version 5.1
[CmdletBinding()]
param(
    [Alias('RuntimeRoot')][string]$RepositoryRoot = '',
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release'
)
$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) { $RepositoryRoot = $PSScriptRoot }
$repo = (Resolve-Path -LiteralPath $RepositoryRoot).Path.TrimEnd('\')
if (-not (Test-Path -LiteralPath (Join-Path $repo 'Framework.sln') -PathType Leaf)) {
    $bundlePath = Join-Path $repo 'bundle-manifest.json'
    $bundle = Get-Content -LiteralPath $bundlePath -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($bundle.schema -cne 'lostark.portable-runtime-bundle' -or $bundle.formatVersion -ne 1 -or
        $bundle.configuration -cne $Configuration -or
        (-not (Test-Path -LiteralPath (Join-Path $repo "Client/Bin/$Configuration/Client.exe") -PathType Leaf) -and
         -not (Test-Path -LiteralPath (Join-Path $repo "Server/Bin/$Configuration/Server.exe") -PathType Leaf))) {
        throw 'RuntimeRoot must contain Framework.sln or a matching portable manifest and Client/Server executable.'
    }
}
$capturedAt = [DateTime]::UtcNow.ToString('o')
$token = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [Guid]::NewGuid().ToString('N').Substring(0,8)
$folder = Join-Path $repo ('out/DiagnosticBundles/' + $token)
$zipPath = $folder + '.zip'
New-Item -ItemType Directory -Path $folder | Out-Null
$rows = [Collections.Generic.List[object]]::new()
$sources = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)

function Copy-Snapshot([string]$Source, [string]$Relative) {
    if (-not (Test-Path -LiteralPath $Source -PathType Leaf) -or -not $sources.Add($Source)) { return }
    $target = Join-Path $folder $Relative
    $inputStream = $null; $outputStream = $null
    try {
        if ((Get-Item -LiteralPath $Source).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Redirected file skipped.' }
        New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
        $inputStream = [IO.File]::Open($Source,[IO.FileMode]::Open,[IO.FileAccess]::Read,([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
        $outputStream = [IO.File]::Open($target,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write,[IO.FileShare]::None)
        $remaining = $inputStream.Length; $initialLength = $remaining
        $buffer = New-Object byte[] 65536
        # A live log may keep growing: capture only the length observed at open.
        while ($remaining -gt 0) {
            $count = $inputStream.Read($buffer,0,[int][Math]::Min($buffer.Length,$remaining))
            if ($count -eq 0) { break }
            $outputStream.Write($buffer,0,$count); $remaining -= $count
        }
        $outputStream.Dispose(); $outputStream = $null
        $inputStream.Dispose(); $inputStream = $null
        $rows.Add([pscustomobject]@{path=$Relative; source=$Source; initialBytes=$initialLength; capturedBytes=(Get-Item -LiteralPath $target).Length; sha256=(Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant()})
    } catch { $rows.Add([pscustomobject]@{path=$Relative; source=$Source; error=$_.Exception.Message}) }
    finally {
        if ($null -ne $outputStream) { $outputStream.Dispose() }
        if ($null -ne $inputStream) { $inputStream.Dispose() }
    }
}

foreach ($pair in @(
    @{ Folder="Client/Bin/$Configuration/Diagnostics"; Filter='client-session-*.jsonl' },
    @{ Folder="Server/Bin/$Configuration/Diagnostics"; Filter='server-session-*.jsonl' },
    @{ Folder="Server/Bin/$Configuration/Diagnostics"; Filter='server-send-progress-*.jsonl' },
    @{ Folder="Server/Bin/$Configuration/Diagnostics"; Filter='server-room-perf-*.log' }
)) {
    $directory = Join-Path $repo $pair.Folder
    if (-not (Test-Path -LiteralPath $directory -PathType Container)) { continue }
    foreach ($file in @(Get-ChildItem -LiteralPath $directory -Filter $pair.Filter -File | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 4)) {
        $relative = $pair.Folder + '/' + $file.Name
        Copy-Snapshot $file.FullName $relative
        Copy-Snapshot ($file.FullName + '.previous') ($relative + '.previous')
    }
}
foreach ($relative in @('Client/Default/ClientStartup.user.log','Client/Default/ClientExit.user.log',
    'Client/Default/EffectFailure.user.log','Client/Default/RendererExit.user.log')) {
    Copy-Snapshot (Join-Path $repo $relative) $relative
    Copy-Snapshot (Join-Path $repo ($relative + '.previous')) ($relative + '.previous')
}
Copy-Snapshot (Join-Path $repo 'Tools/Network/TeamLanEndpoint.json') 'Tools/Network/TeamLanEndpoint.json'

# Retained installation receipts, if present. Package manifests are evidence of
# an available package, not proof that it was installed or loaded by a process.
$backups = Join-Path $repo 'out/RuntimeDeliveryBackups'
if (Test-Path -LiteralPath $backups -PathType Container) {
    foreach ($name in @('runtime-restore.json','data-restore.json')) {
        $receipts = @(Get-ChildItem -LiteralPath $backups -Directory | ForEach-Object {
            Get-Item -LiteralPath (Join-Path $_.FullName $name) -ErrorAction SilentlyContinue
        } | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 4)
        foreach ($file in $receipts) {
            Copy-Snapshot $file.FullName ('installation/receipts/' + $file.Directory.Name + '/' + $file.Name)
        }
    }
}
$manifestDirectories = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
[void]$manifestDirectories.Add($repo); [void]$manifestDirectories.Add($PSScriptRoot)
$release = Join-Path $repo 'Release'
if (Test-Path -LiteralPath $release -PathType Container) {
    foreach ($directory in @(Get-ChildItem -LiteralPath $release -Directory | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 4)) {
        [void]$manifestDirectories.Add($directory.FullName)
    }
}
Add-Type -AssemblyName System.IO.Compression.FileSystem
$manifestIndex = 0
foreach ($directory in $manifestDirectories) {
    $prefix = 'installation/packages/' + $manifestIndex++ + '/'
    foreach ($name in @('bundle-manifest.json','changed-data.json','runtime-delivery.json','Runtime/runtime-delivery.json')) {
        Copy-Snapshot (Join-Path $directory $name) ($prefix + $name)
    }
    $runtimeZip = Join-Path $directory "Runtime/LostArk-$Configuration-Runtime.zip"
    if (-not (Test-Path -LiteralPath $runtimeZip -PathType Leaf)) { continue }
    $archive = $null; $reader = $null
    try {
        $archive = [IO.Compression.ZipFile]::OpenRead($runtimeZip)
        $entry = $archive.GetEntry('runtime-delivery.json')
        if ($null -eq $entry -or $entry.Length -gt 4MB) { throw 'Runtime manifest missing or larger than 4 MiB.' }
        $reader = [IO.StreamReader]::new($entry.Open())
        $text = $reader.ReadToEnd(); $null = $text | ConvertFrom-Json
        $relative = $prefix + 'archive-runtime-delivery.json'; $target = Join-Path $folder $relative
        New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
        [IO.File]::WriteAllText($target,$text,[Text.UTF8Encoding]::new($false))
        $rows.Add([pscustomobject]@{path=$relative; source=$runtimeZip + '!runtime-delivery.json'; capturedBytes=(Get-Item -LiteralPath $target).Length; sha256=(Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant()})
    } catch { $rows.Add([pscustomobject]@{source=$runtimeZip; error=$_.Exception.Message}) }
    finally { if ($null -ne $reader) { $reader.Dispose() }; if ($null -ne $archive) { $archive.Dispose() } }
}

$binaries = @(
    foreach ($relative in @("Client/Bin/$Configuration/Client.exe","Client/Bin/$Configuration/Engine.dll","Server/Bin/$Configuration/Server.exe")) {
        $path = Join-Path $repo $relative
        try {
            $before = Get-Item -LiteralPath $path
            $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
            $after = Get-Item -LiteralPath $path
            [pscustomobject]@{path=$relative; absolutePath=$path; bytes=$before.Length; lastWriteUtc=$before.LastWriteTimeUtc.ToString('o'); sha256=$hash; stableDuringHash=($before.Length -eq $after.Length -and $before.LastWriteTimeUtc -eq $after.LastWriteTimeUtc)}
        } catch { [pscustomobject]@{path=$relative; error=$_.Exception.Message} }
    }
)
$processes = @(); $processError = $null
try {
    $processes = @(Get-CimInstance Win32_Process -Filter "Name='Client.exe' OR Name='Server.exe'" | ForEach-Object {
        [pscustomobject]@{processId=$_.ProcessId; parentProcessId=$_.ParentProcessId; name=$_.Name; executablePath=$_.ExecutablePath; startedAtUtc=$(if ($_.CreationDate) {$_.CreationDate.ToUniversalTime().ToString('o')} else {$null})}
    })
} catch { $processError = $_.Exception.Message }
[pscustomobject]@{schema='lostark.runtime-diagnostic-identity'; formatVersion=1; capturedAtUtc=$capturedAt; configuration=$Configuration; repositoryRoot=$repo; machineName=[Environment]::MachineName; binaries=$binaries; processes=$processes; processQueryError=$processError; identityScope='Files on disk; process metadata is not an in-memory binary hash.'} |
    ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $folder 'identity.json') -Encoding UTF8
[pscustomobject]@{schema='lostark.local-diagnostic-bundle'; formatVersion=1; capturedAtUtc=$capturedAt; completedAtUtc=[DateTime]::UtcNow.ToString('o'); configuration=$Configuration; files=@($rows); networkUpload=$false; processesChanged=$false} |
    ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $folder 'collection.json') -Encoding UTF8
[IO.Compression.ZipFile]::CreateFromDirectory($folder,$zipPath,[IO.Compression.CompressionLevel]::Optimal,$false)
Write-Output "Local diagnostic ZIP created: $zipPath"
Write-Output 'Only log/manifest copies and binary identities were captured. No upload or process changes occurred.'
