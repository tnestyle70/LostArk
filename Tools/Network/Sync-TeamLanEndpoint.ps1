[CmdletBinding()]
param(
    [switch]$SkipConnectionCheck,
    [switch]$AllowExpired,
    [ValidateSet('Auto', 'Server', 'Client')]
    [string]$Role = 'Auto'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Xml.Linq

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$endpointPath = Join-Path $PSScriptRoot 'TeamLanEndpoint.json'
$serverUserPath = Join-Path $repoRoot 'Server\Default\Server.vcxproj.user'
$clientUserPath = Join-Path $repoRoot 'Client\Default\Client.vcxproj.user'

function Set-ProjectUserProperty {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$PropertyName,
        [Parameter(Mandatory = $true)]
        [string]$Value
    )

    $namespaceUri = 'http://schemas.microsoft.com/developer/msbuild/2003'
    $xmlNamespace = [System.Xml.Linq.XNamespace]::Get($namespaceUri)
    if (Test-Path -LiteralPath $Path) {
        $document = [System.Xml.Linq.XDocument]::Load(
            $Path,
            [System.Xml.Linq.LoadOptions]::PreserveWhitespace)
        if ($null -eq $document.Root -or
            $document.Root.Name.LocalName -ne 'Project' -or
            $document.Root.Name.NamespaceName -ne $namespaceUri) {
            throw "Invalid Visual Studio user project XML: $Path"
        }
    }
    else {
        $document = [System.Xml.Linq.XDocument]::new(
            [System.Xml.Linq.XDeclaration]::new('1.0', 'utf-8', $null),
            [System.Xml.Linq.XElement]::new(
                $xmlNamespace + 'Project',
                [System.Xml.Linq.XAttribute]::new('ToolsVersion', 'Current'),
                [System.Xml.Linq.XElement]::new(
                    $xmlNamespace + 'PropertyGroup')))
    }

    $propertyGroups = @($document.Root.Elements(
        $xmlNamespace + 'PropertyGroup'))
    if (0 -eq $propertyGroups.Count) {
        $propertyGroup = [System.Xml.Linq.XElement]::new(
            $xmlNamespace + 'PropertyGroup')
        $document.Root.Add($propertyGroup)
    }
    else {
        $propertyGroup = $propertyGroups[0]
    }

    $properties = @($document.Descendants($xmlNamespace + $PropertyName))
    if (0 -eq $properties.Count) {
        $propertyGroup.Add([System.Xml.Linq.XElement]::new(
            $xmlNamespace + $PropertyName,
            $Value))
    }
    else {
        foreach ($property in $properties) {
            $property.Value = $Value
        }
    }

    $temporaryPath = "$Path.$PID.tmp"
    try {
        $writerSettings = [System.Xml.XmlWriterSettings]::new()
        $writerSettings.Encoding = [Text.UTF8Encoding]::new($false)
        $writerSettings.Indent = $true
        $writerSettings.NewLineChars = "`r`n"
        $writerSettings.NewLineHandling =
            [System.Xml.NewLineHandling]::Replace
        $writer = [System.Xml.XmlWriter]::Create(
            $temporaryPath,
            $writerSettings)
        try {
            $document.Save($writer)
        }
        finally {
            $writer.Dispose()
        }

        $stagedDocument = [System.Xml.Linq.XDocument]::Load($temporaryPath)
        $stagedValues = @($stagedDocument.Descendants(
            $xmlNamespace + $PropertyName) | ForEach-Object { $_.Value })
        if (0 -eq $stagedValues.Count -or
            @($stagedValues | Where-Object { $_ -ne $Value }).Count -ne 0) {
            throw "Failed to stage $PropertyName in $Path"
        }

        Move-Item -LiteralPath $temporaryPath -Destination $Path -Force
    }
    finally {
        if (Test-Path -LiteralPath $temporaryPath) {
            Remove-Item -LiteralPath $temporaryPath -Force
        }
    }
}

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Sync-HostFirewall {
    param(
        [Parameter(Mandatory = $true)]
        [uint16]$Port
    )

    $definitions = @(
        @{
            Name = "LostArk Server Debug TCP $Port (LAN)"
            Program = Join-Path $repoRoot 'Server\Bin\Debug\Server.exe'
        },
        @{
            Name = "LostArk Server Release TCP $Port (LAN)"
            Program = Join-Path $repoRoot 'Server\Bin\Release\Server.exe'
        })
    $missingRules = @($definitions | Where-Object {
        $null -eq (Get-NetFirewallRule -DisplayName $_.Name `
            -ErrorAction SilentlyContinue)
    })
    if (0 -eq $missingRules.Count) {
        return
    }

    if (-not (Test-IsAdministrator)) {
        Write-Warning (
            'This PC owns the team Server address, but the LostArk TCP ' +
            "$Port LAN firewall rule is missing. Run this script once from " +
            'an elevated PowerShell session.')
        return
    }

    foreach ($definition in $definitions) {
        $existing = Get-NetFirewallRule -DisplayName $definition.Name `
            -ErrorAction SilentlyContinue
        if ($null -ne $existing) {
            Remove-NetFirewallRule -DisplayName $definition.Name
        }
        New-NetFirewallRule `
            -DisplayName $definition.Name `
            -Direction Inbound `
            -Action Allow `
            -Enabled True `
            -Profile Any `
            -Protocol TCP `
            -LocalPort $Port `
            -Program $definition.Program `
            -RemoteAddress LocalSubnet | Out-Null
    }
}

if (-not (Test-Path -LiteralPath $endpointPath)) {
    throw "Team LAN endpoint document is missing: $endpointPath"
}

$endpoint = Get-Content -LiteralPath $endpointPath -Raw -Encoding UTF8 |
    ConvertFrom-Json
if ($endpoint.schema -ne 'lostark.team-lan-endpoint' -or
    [int]$endpoint.version -ne 1 -or
    [string]$endpoint.serverBindAddress -ne '0.0.0.0' -or
    [int]$endpoint.port -ne 7777) {
    throw 'Team LAN endpoint document has an unsupported contract.'
}

$serverAddress = $null
if (-not [Net.IPAddress]::TryParse(
    [string]$endpoint.serverHost,
    [ref]$serverAddress) -or
    $serverAddress.AddressFamily -ne
        [Net.Sockets.AddressFamily]::InterNetwork -or
    $serverAddress.Equals([Net.IPAddress]::Any) -or
    $serverAddress.Equals([Net.IPAddress]::Loopback)) {
    throw 'Team LAN serverHost must be one concrete non-loopback IPv4 address.'
}

$activeThrough = [DateTimeOffset]::MinValue
if (-not [DateTimeOffset]::TryParse(
    [string]$endpoint.activeThroughKst,
    [Globalization.CultureInfo]::InvariantCulture,
    [Globalization.DateTimeStyles]::None,
    [ref]$activeThrough)) {
    throw 'Team LAN activeThroughKst is invalid.'
}
if (-not $AllowExpired -and [DateTimeOffset]::Now -gt $activeThrough) {
    throw (
        "The temporary team LAN endpoint expired at $activeThrough. " +
        'Update TeamLanEndpoint.json and the shared runtime contract before use.')
}

$serverHost = $serverAddress.ToString()
$serverBindAddress = [string]$endpoint.serverBindAddress
$serverPort = [uint16]$endpoint.port
$localAddresses = @(Get-NetIPAddress -AddressFamily IPv4 `
    -ErrorAction SilentlyContinue | ForEach-Object { $_.IPAddress })
$ownsServerAddress = $localAddresses -contains $serverHost
$effectiveRole = switch ($Role) {
    'Server' {
        if (-not $ownsServerAddress) {
            throw (
                "This PC does not own configured Server address $serverHost. " +
                'Fix the endpoint document or use the Client role.')
        }
        'server-host'
    }
    'Client' { 'client' }
    default {
        if ($ownsServerAddress) { 'server-host' } else { 'client' }
    }
}

if ('server-host' -eq $effectiveRole) {
    Set-ProjectUserProperty `
        -Path $serverUserPath `
        -PropertyName 'LocalDebuggerCommandArguments' `
        -Value "--bind-address $serverBindAddress"
}
Set-ProjectUserProperty `
    -Path $clientUserPath `
    -PropertyName 'LocalDebuggerEnvironment' `
    -Value "LOSTARK_SERVER_HOST=$serverHost"

if ('server-host' -eq $effectiveRole) {
    Sync-HostFirewall -Port $serverPort
}

$connectionStatus = 'skipped'
if (-not $SkipConnectionCheck) {
    $tcpClient = [Net.Sockets.TcpClient]::new()
    try {
        $connectTask = $tcpClient.ConnectAsync($serverHost, $serverPort)
        $connectionStatus = if ($connectTask.Wait(750) -and
            $tcpClient.Connected) {
            'reachable'
        }
        else {
            'not-listening'
        }
    }
    catch {
        $connectionStatus = 'not-listening'
    }
    finally {
        $tcpClient.Dispose()
    }
}

Write-Output "Team LAN active through: $($activeThrough.ToString('o'))"
Write-Output "Machine role: $effectiveRole"
Write-Output "Server debugger bind: $serverBindAddress`:$serverPort"
Write-Output "Client debugger endpoint: $serverHost`:$serverPort"
Write-Output $(if ('server-host' -eq $effectiveRole) {
    'Visual Studio start target: Server + Client profile'
} else {
    'Visual Studio start target: Client project only'
})
Write-Output "Endpoint status now: $connectionStatus"
