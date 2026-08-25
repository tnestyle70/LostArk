#requires -Version 7.0

[CmdletBinding()]
param(
    [string]$Root = (Join-Path $PSScriptRoot '..\..\Client\Bin\Resources\Sound'),
    [string]$FfprobePath = 'ffprobe',
    [string]$FfmpegPath = 'ffmpeg',
    [ValidateRange(1, 300)]
    [int]$ProbeTimeoutSeconds = 30,
    [ValidateRange(1, 32)]
    [int]$ThrottleLimit = 12,
    [switch]$FullDecode
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$failures = [System.Collections.Generic.List[string]]::new()

function Add-ValidationFailure {
    param([Parameter(Mandatory = $true)][string]$Message)

    $failures.Add($Message)
}

function Resolve-AudioExecutable {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (Test-Path -LiteralPath $Path -PathType Leaf) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    $command = Get-Command -Name $Path -CommandType Application -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($null -eq $command) {
        throw "ffprobe executable was not found: $Path"
    }
    return $command.Source
}

function Get-JsonStringLeaf {
    param(
        [AllowNull()][object]$Value,
        [Parameter(Mandatory = $true)][string]$Location,
        [string]$PropertyName = ''
    )

    if ($null -eq $Value) {
        return
    }
    if ($Value -is [string]) {
        [pscustomobject]@{
            Location = $Location
            PropertyName = $PropertyName
            Value = [string]$Value
        }
        return
    }
    if ($Value -is [System.Collections.IDictionary]) {
        foreach ($key in $Value.Keys) {
            Get-JsonStringLeaf -Value $Value[$key] -Location "$Location.$key" -PropertyName ([string]$key)
        }
        return
    }
    if ($Value -is [System.Collections.IEnumerable] -and $Value -isnot [pscustomobject]) {
        $index = 0
        foreach ($item in $Value) {
            Get-JsonStringLeaf -Value $item -Location "$Location[$index]" -PropertyName $PropertyName
            $index++
        }
        return
    }
    if ($Value.GetType().IsPrimitive -or $Value -is [decimal] -or $Value -is [datetime] -or
        $Value -is [datetimeoffset]) {
        return
    }

    foreach ($property in $Value.PSObject.Properties) {
        if ($property.MemberType -notin @('NoteProperty', 'Property')) {
            continue
        }
        Get-JsonStringLeaf -Value $property.Value -Location "$Location.$($property.Name)" `
            -PropertyName $property.Name
    }
}

function Test-JsonRelativePath {
    param(
        [Parameter(Mandatory = $true)][string]$Value,
        [Parameter(Mandatory = $true)][string]$Location,
        [Parameter(Mandatory = $true)][string]$DocumentName,
        [Parameter(Mandatory = $true)][string]$SoundRoot,
        [switch]$RequireWav
    )

    $displayLocation = "${DocumentName}:$Location"
    if ([string]::IsNullOrWhiteSpace($Value)) {
        Add-ValidationFailure "$displayLocation contains an empty path."
        return $null
    }

    $trimmed = $Value.Trim()
    $normalized = $trimmed.Replace('\', '/')
    if ($trimmed.IndexOf([char]0) -ge 0) {
        Add-ValidationFailure "$displayLocation contains a NUL byte."
        return $null
    }
    if ([System.IO.Path]::IsPathFullyQualified($trimmed) -or $normalized.StartsWith('/') -or
        $normalized -match '^[A-Za-z][A-Za-z0-9+.-]*:') {
        Add-ValidationFailure "$displayLocation must be relative, got '$Value'."
        return $null
    }

    $segments = @($normalized.Split('/', [System.StringSplitOptions]::RemoveEmptyEntries))
    if ($segments.Count -eq 0) {
        Add-ValidationFailure "$displayLocation does not identify a file."
        return $null
    }
    if ($segments -contains '..') {
        Add-ValidationFailure "$displayLocation contains a prohibited '..' segment: '$Value'."
        return $null
    }
    if ($RequireWav -and -not $normalized.EndsWith('.wav', [System.StringComparison]::OrdinalIgnoreCase)) {
        Add-ValidationFailure "$displayLocation must reference a WAV file: '$Value'."
        return $null
    }

    $soundRelative = $normalized
    if ($soundRelative.StartsWith('Sound/', [System.StringComparison]::OrdinalIgnoreCase)) {
        $soundRelative = $soundRelative.Substring('Sound/'.Length)
    }
    if ([string]::IsNullOrWhiteSpace($soundRelative)) {
        Add-ValidationFailure "$displayLocation does not identify a file below Sound: '$Value'."
        return $null
    }

    $nativeRelative = $soundRelative.Replace('/', [System.IO.Path]::DirectorySeparatorChar)
    $candidate = [System.IO.Path]::GetFullPath((Join-Path $SoundRoot $nativeRelative))
    $rootPrefix = $SoundRoot.TrimEnd([char[]]@('\', '/')) + [System.IO.Path]::DirectorySeparatorChar
    if (-not $candidate.StartsWith($rootPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        Add-ValidationFailure "$displayLocation escapes the Sound root: '$Value'."
        return $null
    }

    if ($RequireWav -and -not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        Add-ValidationFailure "$displayLocation references a missing WAV: '$Value'."
        return $null
    }
    return $candidate
}

function Test-JsonDocument {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$SoundRoot
    )

    $documentName = [System.IO.Path]::GetFileName($Path)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        Add-ValidationFailure "Required JSON document is missing: $documentName"
        return 0
    }

    try {
        $document = Get-Content -LiteralPath $Path -Raw -Encoding UTF8 | ConvertFrom-Json -Depth 100
    }
    catch {
        Add-ValidationFailure ("{0} is not valid JSON: {1}" -f $documentName, $_.Exception.Message)
        return 0
    }

    $wavReferenceCount = 0
    foreach ($leaf in Get-JsonStringLeaf -Value $document -Location '$') {
        $isWavReference = $leaf.Value.EndsWith('.wav', [System.StringComparison]::OrdinalIgnoreCase)
        $isPathProperty = $leaf.PropertyName -match '(?i)(path|file|asset|resource|manifest|catalog)'
        $looksLikePath = $leaf.Value.IndexOfAny([char[]]@('/', '\')) -ge 0
        $looksLikeFileName = $leaf.Value -match '(?i)\.[a-z0-9]{1,10}$'
        if (-not ($isWavReference -or $looksLikePath -or ($isPathProperty -and $looksLikeFileName))) {
            continue
        }

        $null = Test-JsonRelativePath -Value $leaf.Value -Location $leaf.Location `
            -DocumentName $documentName -SoundRoot $SoundRoot -RequireWav:$isWavReference
        if ($isWavReference) {
            $wavReferenceCount++
        }
    }
    return $wavReferenceCount
}

function Invoke-Ffprobe {
    param(
        [Parameter(Mandatory = $true)][string]$Executable,
        [Parameter(Mandatory = $true)][string]$WavPath,
        [Parameter(Mandatory = $true)][int]$TimeoutSeconds
    )

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $Executable
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    foreach ($argument in @(
        '-v', 'error',
        '-select_streams', 'a:0',
        '-show_entries', 'stream=codec_name,sample_rate,channels,duration:format=duration',
        '-of', 'json',
        $WavPath
    )) {
        $startInfo.ArgumentList.Add($argument)
    }

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    try {
        if (-not $process.Start()) {
            throw 'Process.Start returned false.'
        }
        $standardOutput = $process.StandardOutput.ReadToEndAsync()
        $standardError = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            try {
                $process.Kill($true)
            }
            catch {
                # The process may have exited between the timeout and Kill.
            }
            throw "ffprobe timed out after $TimeoutSeconds seconds."
        }
        $stdout = $standardOutput.GetAwaiter().GetResult()
        $stderr = $standardError.GetAwaiter().GetResult()
        if ($process.ExitCode -ne 0) {
            throw "ffprobe exited with code $($process.ExitCode): $($stderr.Trim())"
        }
        if ([string]::IsNullOrWhiteSpace($stdout)) {
            throw 'ffprobe returned no JSON.'
        }
        return $stdout | ConvertFrom-Json -Depth 20
    }
    finally {
        $process.Dispose()
    }
}

$soundRoot = [System.IO.Path]::GetFullPath($Root)
if (-not (Test-Path -LiteralPath $soundRoot -PathType Container)) {
    throw "Sound root does not exist: $soundRoot"
}

$files = @(Get-ChildItem -LiteralPath $soundRoot -Recurse -File)
$wavFiles = @($files | Where-Object Extension -IEQ '.wav' | Sort-Object FullName)
if ($wavFiles.Count -eq 0) {
    Add-ValidationFailure "Sound root does not contain any WAV files: $soundRoot"
}

$forbiddenExtensions = [System.Collections.Generic.HashSet[string]]::new(
    [string[]]@('.wem', '.bnk', '.key', '.pem', '.pfx', '.p12'),
    [System.StringComparer]::OrdinalIgnoreCase
)
foreach ($file in $files) {
    $isNamedKeyFile = $file.Extension -notin @('.wav', '.json') -and
        $file.BaseName -match '(?i)(^|[._-])key($|[._-])'
    if ($forbiddenExtensions.Contains($file.Extension) -or $isNamedKeyFile) {
        $relative = [System.IO.Path]::GetRelativePath($soundRoot, $file.FullName)
        Add-ValidationFailure "Prohibited raw or key file is present: $relative"
    }
}

$catalogReferenceCount = Test-JsonDocument -Path (Join-Path $soundRoot 'SoundCatalog.json') `
    -SoundRoot $soundRoot
$manifestReferenceCount = Test-JsonDocument -Path (Join-Path $soundRoot 'ExtractionManifest.json') `
    -SoundRoot $soundRoot

$ffprobeExecutable = $null
try {
    $ffprobeExecutable = Resolve-AudioExecutable -Path $FfprobePath
}
catch {
    Add-ValidationFailure $_.Exception.Message
}

$monoCount = 0
$stereoCount = 0
$totalDurationSeconds = 0.0
if ($null -ne $ffprobeExecutable) {
    $wavNumber = 0
    $wavFiles |
        ForEach-Object -Parallel {
            $wavPath = $_.FullName
            $executable = $using:ffprobeExecutable
            $timeoutSeconds = $using:ProbeTimeoutSeconds
            try {
                $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
                $startInfo.FileName = $executable
                $startInfo.UseShellExecute = $false
                $startInfo.CreateNoWindow = $true
                $startInfo.RedirectStandardOutput = $true
                $startInfo.RedirectStandardError = $true
                foreach ($argument in @(
                    '-v', 'error',
                    '-select_streams', 'a:0',
                    '-show_entries', 'stream=codec_name,sample_rate,channels,duration:format=duration',
                    '-of', 'json',
                    $wavPath
                )) {
                    $startInfo.ArgumentList.Add($argument)
                }

                $process = [System.Diagnostics.Process]::new()
                $process.StartInfo = $startInfo
                try {
                    if (-not $process.Start()) {
                        throw 'Process.Start returned false.'
                    }
                    $standardOutput = $process.StandardOutput.ReadToEndAsync()
                    $standardError = $process.StandardError.ReadToEndAsync()
                    if (-not $process.WaitForExit($timeoutSeconds * 1000)) {
                        try { $process.Kill($true) } catch { }
                        throw "ffprobe timed out after $timeoutSeconds seconds."
                    }
                    $stdout = $standardOutput.GetAwaiter().GetResult()
                    $stderr = $standardError.GetAwaiter().GetResult()
                    if ($process.ExitCode -ne 0) {
                        throw "ffprobe exited with code $($process.ExitCode): $($stderr.Trim())"
                    }
                }
                finally {
                    $process.Dispose()
                }

                $probe = $stdout | ConvertFrom-Json -Depth 20
                $streams = @($probe.streams)
                if ($streams.Count -ne 1) {
                    throw "expected exactly one audio stream, got $($streams.Count)."
                }
                $stream = $streams[0]
                if ([string]$stream.codec_name -cne 'pcm_s16le') {
                    throw "codec is '$($stream.codec_name)', expected pcm_s16le."
                }

                $sampleRate = 0
                if (-not [int]::TryParse([string]$stream.sample_rate,
                        [Globalization.NumberStyles]::Integer,
                        [Globalization.CultureInfo]::InvariantCulture,
                        [ref]$sampleRate) -or $sampleRate -ne 48000) {
                    throw "sample rate is '$($stream.sample_rate)', expected 48000 Hz."
                }
                $channels = 0
                if (-not [int]::TryParse([string]$stream.channels,
                        [Globalization.NumberStyles]::Integer,
                        [Globalization.CultureInfo]::InvariantCulture,
                        [ref]$channels) -or $channels -notin @(1, 2)) {
                    throw "channel count is '$($stream.channels)', expected 1 or 2."
                }

                $durationText = if ($null -ne $probe.format -and $null -ne $probe.format.duration) {
                    [string]$probe.format.duration
                }
                else {
                    [string]$stream.duration
                }
                $duration = 0.0
                if (-not [double]::TryParse($durationText,
                        [Globalization.NumberStyles]::Float,
                        [Globalization.CultureInfo]::InvariantCulture,
                        [ref]$duration) -or -not [double]::IsFinite($duration) -or $duration -le 0.0) {
                    throw "duration is '$durationText', expected a positive finite value."
                }
                [pscustomobject]@{
                    Path = $wavPath
                    Success = $true
                    Channels = $channels
                    DurationSeconds = $duration
                    Error = $null
                }
            }
            catch {
                [pscustomobject]@{
                    Path = $wavPath
                    Success = $false
                    Channels = 0
                    DurationSeconds = 0.0
                    Error = $_.Exception.Message
                }
            }
        } -ThrottleLimit $ThrottleLimit |
        ForEach-Object {
            $wavNumber++
            if ($_.Success) {
                if ($_.Channels -eq 1) {
                    $monoCount++
                }
                else {
                    $stereoCount++
                }
                $totalDurationSeconds += $_.DurationSeconds
            }
            else {
                $relative = [System.IO.Path]::GetRelativePath($soundRoot, $_.Path)
                Add-ValidationFailure ("Invalid WAV '{0}': {1}" -f $relative, $_.Error)
            }
            if ($wavNumber % 100 -eq 0 -or $wavNumber -eq $wavFiles.Count) {
                Write-Host ("Validated WAV {0}/{1}" -f $wavNumber, $wavFiles.Count)
            }
        }
}

$fullDecodeCount = 0
if ($FullDecode) {
    $ffmpegExecutable = $null
    try {
        $ffmpegExecutable = Resolve-AudioExecutable -Path $FfmpegPath
    }
    catch {
        Add-ValidationFailure $_.Exception.Message
    }
    if ($null -ne $ffmpegExecutable) {
        $wavFiles |
            ForEach-Object -Parallel {
                $wavPath = $_.FullName
                $executable = $using:ffmpegExecutable
                $timeoutSeconds = $using:ProbeTimeoutSeconds
                try {
                    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
                    $startInfo.FileName = $executable
                    $startInfo.UseShellExecute = $false
                    $startInfo.CreateNoWindow = $true
                    $startInfo.RedirectStandardOutput = $true
                    $startInfo.RedirectStandardError = $true
                    foreach ($argument in @(
                        '-v', 'error', '-nostdin',
                        '-i', $wavPath,
                        '-map', '0:a:0',
                        '-f', 'null', '-'
                    )) {
                        $startInfo.ArgumentList.Add($argument)
                    }

                    $process = [System.Diagnostics.Process]::new()
                    $process.StartInfo = $startInfo
                    try {
                        if (-not $process.Start()) {
                            throw 'Process.Start returned false.'
                        }
                        $standardOutput = $process.StandardOutput.ReadToEndAsync()
                        $standardError = $process.StandardError.ReadToEndAsync()
                        if (-not $process.WaitForExit($timeoutSeconds * 1000)) {
                            try { $process.Kill($true) } catch { }
                            throw "ffmpeg timed out after $timeoutSeconds seconds."
                        }
                        $null = $standardOutput.GetAwaiter().GetResult()
                        $stderr = $standardError.GetAwaiter().GetResult()
                        if ($process.ExitCode -ne 0) {
                            throw "ffmpeg exited with code $($process.ExitCode): $($stderr.Trim())"
                        }
                    }
                    finally {
                        $process.Dispose()
                    }
                    [pscustomobject]@{ Path = $wavPath; Success = $true; Error = $null }
                }
                catch {
                    [pscustomobject]@{ Path = $wavPath; Success = $false; Error = $_.Exception.Message }
                }
            } -ThrottleLimit $ThrottleLimit |
            ForEach-Object {
                $fullDecodeCount++
                if (-not $_.Success) {
                    $relative = [System.IO.Path]::GetRelativePath($soundRoot, $_.Path)
                    Add-ValidationFailure ("Full decode failed for '{0}': {1}" -f $relative, $_.Error)
                }
                if ($fullDecodeCount % 100 -eq 0 -or $fullDecodeCount -eq $wavFiles.Count) {
                    Write-Host ("Fully decoded WAV {0}/{1}" -f $fullDecodeCount, $wavFiles.Count)
                }
            }
    }
}

Write-Host 'Lost Ark sound validation summary'
Write-Host "  Root:                 $soundRoot"
Write-Host "  WAV files:            $($wavFiles.Count)"
Write-Host "  Mono / stereo:        $monoCount / $stereoCount"
Write-Host ("  Total duration:       {0:N3} seconds" -f $totalDurationSeconds)
Write-Host "  Catalog WAV refs:     $catalogReferenceCount"
Write-Host "  Manifest WAV refs:    $manifestReferenceCount"
Write-Host "  Full decoded WAVs:    $fullDecodeCount"
Write-Host "  Validation failures:  $($failures.Count)"

if ($failures.Count -gt 0) {
    foreach ($failure in $failures) {
        [Console]::Error.WriteLine("ERROR: $failure")
    }
    throw "Lost Ark sound validation failed with $($failures.Count) error(s)."
}

Write-Host 'Lost Ark sound validation PASS.'
