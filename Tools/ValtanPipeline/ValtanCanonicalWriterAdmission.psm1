Set-StrictMode -Version Latest

$script:MarkerPrefix = 'lostark.valtan-canonical-writer-owner-v1'
$script:LockRelativePath = 'out\ValtanPatternTransactions\create-pattern.lock'

function Test-ValtanCanonicalWriterAncestor {
    param([Parameter(Mandatory = $true)][int]$OwnerPid)

    if ($OwnerPid -le 0) { return $false }
    $current = [int]$PID
    $seen = [Collections.Generic.HashSet[int]]::new()
    while ($current -gt 0 -and $seen.Add($current)) {
        if ($current -eq $OwnerPid) { return $true }
        try {
            $row = Get-CimInstance -ClassName Win32_Process `
                -Filter "ProcessId = $current" -ErrorAction Stop
        }
        catch { return $false }
        if ($null -eq $row) { return $false }
        $current = [int]$row.ParentProcessId
    }
    return $false
}

function Get-ValtanCanonicalWriterMarker {
    param([Parameter(Mandatory = $true)][int]$OwnerPid,
          [Parameter(Mandatory = $true)][string]$OwnerNonce)

    return "${script:MarkerPrefix}:${OwnerPid}:${OwnerNonce}`n"
}

function Enter-ValtanCanonicalWriterAdmission {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)][string]$RepositoryRoot,
        [ValidateRange(0.0, 300.0)][double]$TimeoutSeconds = 30.0,
        [int]$ExternalOwnerPid = 0,
        [string]$ExternalOwnerNonce = ''
    )

    $root = [IO.Path]::GetFullPath($RepositoryRoot)
    if (-not [IO.Directory]::Exists($root)) {
        throw "Valtan canonical writer repository root is missing: $root"
    }
    if (($ExternalOwnerPid -gt 0) -ne
        (-not [string]::IsNullOrWhiteSpace($ExternalOwnerNonce))) {
        throw 'External canonical writer owner PID and nonce must be supplied together.'
    }
    if ($ExternalOwnerPid -gt 0 -and
        $ExternalOwnerNonce -cnotmatch '^[0-9a-f]{32}$') {
        throw 'External canonical writer owner nonce is invalid.'
    }

    $lockPath = [IO.Path]::GetFullPath((Join-Path $root $script:LockRelativePath))
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($lockPath)) |
        Out-Null
    $stream = [IO.FileStream]::new(
        $lockPath,
        [IO.FileMode]::OpenOrCreate,
        [IO.FileAccess]::ReadWrite,
        [IO.FileShare]::ReadWrite)

    if ($ExternalOwnerPid -gt 0) {
        try {
            if (-not (Test-ValtanCanonicalWriterAncestor $ExternalOwnerPid)) {
                throw 'External canonical writer owner is not an ancestor process.'
            }
            $expectedMarker = Get-ValtanCanonicalWriterMarker `
                $ExternalOwnerPid $ExternalOwnerNonce
            $stream.Position = 1L
            $reader = [IO.StreamReader]::new(
                $stream, [Text.Encoding]::ASCII, $false, 1024, $true)
            try { $actualMarker = $reader.ReadToEnd() }
            finally { $reader.Dispose() }
            if ($actualMarker -cne $expectedMarker) {
                throw 'External canonical writer marker does not match its owner.'
            }
            $unexpectedlyAcquired = $false
            try {
                $stream.Lock(0L, 1L)
                $unexpectedlyAcquired = $true
            }
            catch [IO.IOException] {
                # Expected: the ancestor still owns byte zero.
            }
            if ($unexpectedlyAcquired) {
                $stream.Unlock(0L, 1L)
                throw 'External canonical writer admission was not actually held.'
            }
        }
        catch {
            $stream.Dispose()
            throw
        }
        $stream.Dispose()
        return [pscustomobject]@{
            Stream = $null
            OwnsLock = $false
            OwnerPid = $ExternalOwnerPid
            OwnerNonce = $ExternalOwnerNonce
            LockPath = $lockPath
        }
    }

    $ownsLock = $false
    try {
        if ($stream.Length -lt 1L) {
            $stream.SetLength(1L)
            $stream.Position = 0L
            $stream.WriteByte(0)
            $stream.Flush($true)
        }
        $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
        while (-not $ownsLock) {
            try {
                $stream.Lock(0L, 1L)
                $ownsLock = $true
            }
            catch [IO.IOException] {
                if ([DateTime]::UtcNow -ge $deadline) {
                    throw 'Timed out waiting for Valtan canonical writer admission.'
                }
                Start-Sleep -Milliseconds 25
            }
        }
        $nonce = [Guid]::NewGuid().ToString('N')
        [byte[]]$markerBytes = [Text.Encoding]::ASCII.GetBytes(
            (Get-ValtanCanonicalWriterMarker $PID $nonce))
        $stream.SetLength(1L + $markerBytes.Length)
        $stream.Position = 0L
        $stream.WriteByte(0)
        $stream.Position = 1L
        $stream.Write($markerBytes, 0, $markerBytes.Length)
        $stream.Flush($true)
        return [pscustomobject]@{
            Stream = $stream
            OwnsLock = $true
            OwnerPid = [int]$PID
            OwnerNonce = $nonce
            LockPath = $lockPath
        }
    }
    catch {
        if ($ownsLock) {
            try { $stream.Unlock(0L, 1L) } catch {}
        }
        $stream.Dispose()
        throw
    }
}

function Exit-ValtanCanonicalWriterAdmission {
    [CmdletBinding()]
    param([AllowNull()][object]$Admission)

    if ($null -eq $Admission -or -not [bool]$Admission.OwnsLock) { return }
    $stream = $Admission.Stream
    if ($null -eq $stream) { return }
    try {
        $stream.SetLength(1L)
        $stream.Position = 0L
        $stream.WriteByte(0)
        $stream.Flush($true)
        $stream.Unlock(0L, 1L)
    }
    finally { $stream.Dispose() }
}

Export-ModuleMember -Function `
    Enter-ValtanCanonicalWriterAdmission, Exit-ValtanCanonicalWriterAdmission
