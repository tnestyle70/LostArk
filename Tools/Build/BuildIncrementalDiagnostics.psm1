Set-StrictMode -Version Latest

function Get-BuildOutputSnapshot {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)][string]$ProjectPath,
        [Parameter(Mandatory = $true)][ValidateSet('Debug', 'Release')][string]$Configuration
    )

    $directory = [IO.Path]::GetDirectoryName([IO.Path]::GetFullPath($ProjectPath))
    $name = [IO.Path]::GetFileNameWithoutExtension($ProjectPath)
    # These are the repository's canonical native intermediate trees and the
    # historical standalone VC tree. Inspect metadata only; never alter tlogs.
    $roots = @(
        (Join-Path $directory "x64\$Configuration"),
        (Join-Path $directory "..\Intermediate\x64\$Configuration"),
        (Join-Path $directory "$name\x64\$Configuration"),
        (Join-Path $directory "..\Bin\$Configuration")
    ) | ForEach-Object { [IO.Path]::GetFullPath($_) } | Select-Object -Unique
    $outputs = [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::OrdinalIgnoreCase)
    $directories = [Collections.Generic.List[string]]::new()
    foreach ($root in $roots) {
        if (-not [IO.Directory]::Exists($root)) { continue }
        $directories.Add($root)
        foreach ($item in Get-ChildItem -LiteralPath $root -File -Recurse -ErrorAction Stop) {
            $extension = $item.Extension.ToLowerInvariant()
            if ($extension -notin @('.obj', '.pch', '.cso', '.exe', '.dll', '.lib')) { continue }
            $outputs[$item.FullName] = [pscustomobject]@{
                path = $item.FullName
                extension = $extension
                length = $item.Length
                lastWriteUtcTicks = $item.LastWriteTimeUtc.Ticks
            }
        }
    }
    return [pscustomobject]@{ directories = @($directories); files = $outputs }
}

function Compare-BuildOutputSnapshot {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)][object]$Before,
        [Parameter(Mandatory = $true)][object]$After
    )

    $changed = [Collections.Generic.List[object]]::new()
    foreach ($path in $After.files.Keys) {
        $next = $After.files[$path]
        $previous = if ($Before.files.ContainsKey($path)) { $Before.files[$path] } else { $null }
        if ($null -eq $previous -or $next.length -ne $previous.length -or
            $next.lastWriteUtcTicks -ne $previous.lastWriteUtcTicks) {
            $changed.Add($next)
        }
    }
    $removed = @($Before.files.Keys | Where-Object { -not $After.files.ContainsKey($_) } | Sort-Object)
    return [pscustomobject][ordered]@{
        measurement = 'Changed output size or modification time; successful output writes, not failed compiler attempts.'
        directories = @($After.directories)
        objectWrites = @($changed | Where-Object extension -eq '.obj').Count
        precompiledHeaderWrites = @($changed | Where-Object extension -eq '.pch').Count
        shaderWrites = @($changed | Where-Object extension -eq '.cso').Count
        binaryWrites = @($changed | Where-Object { $_.extension -in @('.exe', '.dll', '.lib') }).Count
        changedPaths = @($changed | Sort-Object path | ForEach-Object { $_.path })
        removedPaths = $removed
    }
}

function Get-MSBuildIncrementalReasons {
    [CmdletBinding()]
    param([Parameter(Mandatory = $true)][string]$LogPath)

    if (-not [IO.File]::Exists($LogPath)) { return }
    # Diagnostic wording is toolset/localization dependent. Preserve bounded
    # verbatim evidence; lack of a matching line never means no rebuild occurred.
    $pattern = '(?i)out.of.date|not up.to.date|newer than|older than|command.line.*chang|because.*(?:input|output)|\uBA85\uB839\uC904.*\uBCC0\uACBD|\uC785\uB825.*\uCD9C\uB825.*\uBCF4\uB2E4.*(?:\uCD5C\uC2E0|\uC0C8)'
    $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($line in [IO.File]::ReadLines($LogPath)) {
        if ($line -notmatch $pattern) { continue }
        $message = $line.Trim()
        if ($message.Length -gt 1000) { $message = $message.Substring(0, 1000) + ' [truncated]' }
        if ($seen.Add($message)) { $message }
        if ($seen.Count -ge 20) { break }
    }
}

Export-ModuleMember -Function @(
    'Get-BuildOutputSnapshot',
    'Compare-BuildOutputSnapshot',
    'Get-MSBuildIncrementalReasons'
)
