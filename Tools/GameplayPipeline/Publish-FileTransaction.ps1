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

# Small text catalogs share the same read-only freshness check and atomic writer.
# LF/CRLF checkouts are equivalent; BOMs, changed rows and changed schemas are not.
function Read-PublishJsonSnapshot([string]$Path, [hashtable]$Sources) {
	$bytes = [IO.File]::ReadAllBytes($Path)
	$Sources[$Path] = [Convert]::ToBase64String($bytes)
	return [Text.UTF8Encoding]::new($false, $true).GetString($bytes).TrimStart([char]0xFEFF) | ConvertFrom-Json
}

function Assert-PublishSourceSnapshots([hashtable]$Sources) {
	foreach ($path in $Sources.Keys) {
		if (-not [IO.File]::Exists($path) -or
			[Convert]::ToBase64String([IO.File]::ReadAllBytes($path)) -cne $Sources[$path]) {
			throw "Publish input changed during validation: $path. Retry with the latest saved source."
		}
	}
}

function Write-PublishTextCatalog(
	[ValidateSet('Publish', 'CheckPublished')][string]$Mode,
	[string]$Destination,
	[string[]]$Lines,
	[hashtable]$Sources,
	[string]$Context,
	[string]$RepairCommand) {
	$expectedText = [string]::Join("`n", $Lines) + "`n"
	$mutex = $null
	$staged = ''
	$rollback = ''
	try {
		if ($Mode -eq 'Publish') {
			$mutex = Enter-PublishDestinationMutex (Get-PublishDestinationMutexName $Destination)
		}
		$previousHash = $null
		$currentText = ''
		$actualHeader = '<missing>'
		if ([IO.File]::Exists($Destination)) {
			$previousHash = Get-PublishFileSha256 $Destination
			try {
				$currentText = [Text.UTF8Encoding]::new($false, $true).GetString(
					[IO.File]::ReadAllBytes($Destination)).Replace("`r`n", "`n")
				$actualHeader = ($currentText -split "`n", 2)[0]
			}
			catch [Text.DecoderFallbackException] {
				# Explicit Publish must be able to repair a corrupt output too.
				$actualHeader = '<invalid UTF-8>'
			}
		}
		Assert-PublishSourceSnapshots $Sources
		if ([string]::Equals($currentText, $expectedText, [StringComparison]::Ordinal)) {
			Write-Output "$Context $Mode succeeded: unchanged -> $Destination"
			return
		}
		if ($Mode -eq 'CheckPublished') {
			throw "$Context published data is stale or invalid: $Destination. Expected header '$($Lines[0])'; actual '$actualHeader'. Run: $RepairCommand"
		}
		[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Destination)) | Out-Null
		$transactionId = [Guid]::NewGuid().ToString('N')
		$staged = "$Destination.staging.$transactionId"
		$rollback = "$Destination.rollback.$transactionId"
		[IO.File]::WriteAllLines($staged, $Lines, [Text.UTF8Encoding]::new($false))
		Assert-PublishSourceSnapshots $Sources
		if ($null -ne $previousHash) {
			if (-not (Test-PublishFileHash $Destination $previousHash $Context)) {
				throw "$Context destination changed during publish: $Destination"
			}
			Invoke-PublishFileOperation {
				Assert-PublishSourceSnapshots $Sources
				if (-not (Test-PublishFileHash $Destination $previousHash $Context)) {
					throw "$Context destination changed during publish: $Destination"
				}
				[IO.File]::Replace($staged, $Destination, $rollback)
			} $Context
		}
		else {
			# Move refuses to overwrite a destination created by another writer.
			[IO.File]::Move($staged, $Destination)
		}
		Write-Output "$Context Publish succeeded: $($Lines.Length - 1) items -> $Destination"
	}
	finally {
		try {
			foreach ($temporary in @($staged, $rollback)) {
				if ($temporary -and [IO.File]::Exists($temporary)) {
					try { [IO.File]::Delete($temporary) }
					catch { Write-Warning "$Context temporary cleanup failed; retained $temporary`: $($_.Exception.Message)" }
				}
			}
		}
		finally {
			Close-PublishDestinationMutex $mutex $Context
		}
	}
}
