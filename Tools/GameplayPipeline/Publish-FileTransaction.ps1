function Get-PublishFileSha256([string]$Path) {
	$stream = [IO.File]::Open(
		$Path,
		[IO.FileMode]::Open,
		[IO.FileAccess]::Read,
		([IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete))
	$sha256 = [Security.Cryptography.SHA256]::Create()
	try {
		return [BitConverter]::ToString(
			$sha256.ComputeHash($stream)).Replace('-', '')
	}
	finally {
		$sha256.Dispose()
		$stream.Dispose()
	}
}

function Get-PublishDestinationMutexName([string]$Destination) {
	$identity = [IO.Path]::GetFullPath($Destination).ToUpperInvariant()
	$sha256 = [Security.Cryptography.SHA256]::Create()
	try {
		$hash = [BitConverter]::ToString(
			$sha256.ComputeHash([Text.Encoding]::UTF8.GetBytes($identity))).Replace('-', '')
		return "Local\LostArk.PublishDestination.$hash"
	}
	finally {
		$sha256.Dispose()
	}
}

function Enter-PublishDestinationMutex(
	[string]$Name,
	[int]$TimeoutMilliseconds = 30000) {
	$mutex = [Threading.Mutex]::new($false, $Name)
	$acquired = $false
	try {
		try {
			$acquired = $mutex.WaitOne($TimeoutMilliseconds)
		}
		catch [Threading.AbandonedMutexException] {
			# Windows transfers an abandoned mutex to this process before raising.
			$acquired = $true
		}
		if (-not $acquired) {
			throw "Timed out waiting for publish destination lock: $Name"
		}
		return $mutex
	}
	catch {
		if (-not $acquired) { $mutex.Dispose() }
		throw
	}
}

function Invoke-PublishFileOperation(
	[scriptblock]$Operation,
	[string]$Context,
	[int]$TimeoutMilliseconds = 5000) {
	$sharingViolation = -2147024864 # HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION)
	$lockViolation = -2147024863 # HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION)
	$timer = [Diagnostics.Stopwatch]::StartNew()
	while ($true) {
		try {
			return & $Operation
		}
		catch [IO.IOException] {
			if ($_.Exception.HResult -notin @($sharingViolation, $lockViolation) -or
				$timer.ElapsedMilliseconds -ge $TimeoutMilliseconds) {
				throw "$Context failed: $($_.Exception.Message)"
			}
			Start-Sleep -Milliseconds 100
		}
	}
}

function Test-PublishFileHash(
	[string]$Path,
	[string]$ExpectedHash,
	[string]$Context) {
	if (-not [IO.File]::Exists($Path)) { return $false }
	$actualHash = Invoke-PublishFileOperation {
		Get-PublishFileSha256 $Path
	} $Context
	return $actualHash -ceq $ExpectedHash
}

function Close-PublishDestinationMutex(
	[Threading.Mutex]$Mutex,
	[string]$Context) {
	if ($null -eq $Mutex) { return }
	try {
		$Mutex.ReleaseMutex()
	}
	catch {
		Write-Warning "$Context release failed: $($_.Exception.Message)"
	}
	finally {
		try { $Mutex.Dispose() }
		catch {
			Write-Warning "$Context dispose failed: $($_.Exception.Message)"
		}
	}
}
