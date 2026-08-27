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

function Set-ProjectUserEnvironmentVariable {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$VariableName,
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

    $properties = @($document.Descendants(
        $xmlNamespace + 'LocalDebuggerEnvironment'))
    if (0 -eq $properties.Count) {
        $property = [System.Xml.Linq.XElement]::new(
            $xmlNamespace + 'LocalDebuggerEnvironment')
        $propertyGroup.Add($property)
        $properties = @($property)
    }

    $canonicalRow = "$VariableName=$Value"
    foreach ($property in $properties) {
        $rows = if ([string]::IsNullOrEmpty($property.Value)) {
            @()
        }
        else {
            @([Text.RegularExpressions.Regex]::Split(
                $property.Value, "`r`n|`n|`r"))
        }
        $canonicalRows = [Collections.Generic.List[string]]::new()
        $hasCanonicalRow = $false
        foreach ($row in $rows) {
            $separatorIndex = $row.IndexOf('=')
            $isTargetRow = $separatorIndex -ge 0 -and
                $row.Substring(0, $separatorIndex) -ieq $VariableName
            if ($isTargetRow) {
                if (-not $hasCanonicalRow) {
                    $canonicalRows.Add($canonicalRow)
                    $hasCanonicalRow = $true
                }
                continue
            }
            $canonicalRows.Add($row)
        }
        if (-not $hasCanonicalRow) {
            $canonicalRows.Add($canonicalRow)
        }
        $property.Value = $canonicalRows -join "`n"
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
        $stagedProperties = @($stagedDocument.Descendants(
            $xmlNamespace + 'LocalDebuggerEnvironment'))
        if ($stagedProperties.Count -ne $properties.Count) {
            throw "Failed to stage LocalDebuggerEnvironment in $Path"
        }
        foreach ($property in $stagedProperties) {
            $targetRows = @([Text.RegularExpressions.Regex]::Split(
                $property.Value, "`r`n|`n|`r") | Where-Object {
                    $separatorIndex = $_.IndexOf('=')
                    $separatorIndex -ge 0 -and
                        $_.Substring(0, $separatorIndex) -ieq $VariableName
                })
            if (1 -ne $targetRows.Count -or
                $targetRows[0] -cne $canonicalRow) {
                throw "Failed to canonicalize $VariableName in $Path"
            }
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

function Test-HostFirewallRule {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [Parameter(Mandatory = $true)]
        [string]$Program,
        [Parameter(Mandatory = $true)]
        [uint16]$Port
    )

    $rules = @(Get-NetFirewallRule -DisplayName $Name `
        -ErrorAction SilentlyContinue)
    if (1 -ne $rules.Count) {
        return $false
    }

    $rule = $rules[0]
    if ([string]$rule.Enabled -ne 'True' -or
        [string]$rule.Direction -ne 'Inbound' -or
        [string]$rule.Action -ne 'Allow' -or
        [string]$rule.Profile -ne 'Any') {
        return $false
    }

    $portFilters = @($rule | Get-NetFirewallPortFilter)
    $addressFilters = @($rule | Get-NetFirewallAddressFilter)
    $applicationFilters = @($rule | Get-NetFirewallApplicationFilter)
    if (1 -ne $portFilters.Count -or
        [string]$portFilters[0].Protocol -ne 'TCP' -or
        [string]$portFilters[0].LocalPort -ne [string]$Port -or
        1 -ne $addressFilters.Count -or
        @($addressFilters[0].RemoteAddress).Count -ne 1 -or
        [string]$addressFilters[0].RemoteAddress -ne 'LocalSubnet' -or
        1 -ne $applicationFilters.Count) {
        return $false
    }

    $expectedProgram = [IO.Path]::GetFullPath($Program)
    $actualProgram = [string]$applicationFilters[0].Program
    return $expectedProgram.Equals(
        $actualProgram,
        [StringComparison]::OrdinalIgnoreCase)
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
    $staleRules = @($definitions | Where-Object {
        -not (Test-HostFirewallRule `
            -Name $_.Name `
            -Program $_.Program `
            -Port $Port)
    })
    if (0 -eq $staleRules.Count) {
        Write-Output "Server firewall: TCP $Port LocalSubnet ready"
        return
    }

    if (-not (Test-IsAdministrator)) {
        throw (
            'This PC owns the team Server address, but its LostArk TCP ' +
            "$Port LocalSubnet firewall rule is missing or stale. Run this " +
            'script once from an elevated PowerShell session.')
    }

    foreach ($definition in $staleRules) {
        $existing = @(Get-NetFirewallRule -DisplayName $definition.Name `
            -ErrorAction SilentlyContinue)
        if (0 -ne $existing.Count) {
            $existing | Remove-NetFirewallRule
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

        if (-not (Test-HostFirewallRule `
            -Name $definition.Name `
            -Program $definition.Program `
            -Port $Port)) {
            throw "Failed to establish exact firewall rule: $($definition.Name)"
        }
    }

    Write-Output "Server firewall: TCP $Port LocalSubnet ready"
}

if (-not (Test-Path -LiteralPath $endpointPath)) {
    throw "Team LAN endpoint document is missing: $endpointPath"
}

$endpoint = Get-Content -LiteralPath $endpointPath -Raw -Encoding UTF8 |
    ConvertFrom-Json
# Loopback remains a supported isolated-test endpoint, while a shared LAN
# contract uses one concrete Client address and an all-adapter Server bind.
if ($endpoint.schema -ne 'lostark.team-lan-endpoint' -or
    [int]$endpoint.version -ne 1 -or
    [string]$endpoint.serverBindAddress -notin @('0.0.0.0', '127.0.0.1') -or
    [int]$endpoint.port -ne 7777) {
    throw 'Team LAN endpoint document has an unsupported contract.'
}

$serverAddress = $null
if (-not [Net.IPAddress]::TryParse(
    [string]$endpoint.serverHost,
    [ref]$serverAddress) -or
    $serverAddress.AddressFamily -ne
        [Net.Sockets.AddressFamily]::InterNetwork -or
    $serverAddress.Equals([Net.IPAddress]::Any)) {
    throw 'serverHost must be one concrete IPv4 address, and never 0.0.0.0.'
}
$isLoopbackEndpoint = $serverAddress.Equals([Net.IPAddress]::Loopback)
$serverBindAddress = [string]$endpoint.serverBindAddress
if (($isLoopbackEndpoint -and '127.0.0.1' -ne $serverBindAddress) -or
    (-not $isLoopbackEndpoint -and '0.0.0.0' -ne $serverBindAddress)) {
    throw 'Loopback must bind 127.0.0.1; a shared LAN endpoint must bind 0.0.0.0.'
}

$activeThrough = [DateTimeOffset]::MinValue
if (-not [DateTimeOffset]::TryParse(
    [string]$endpoint.activeThroughKst,
    [Globalization.CultureInfo]::InvariantCulture,
    [Globalization.DateTimeStyles]::None,
    [ref]$activeThrough)) {
    throw 'Team LAN activeThroughKst is invalid.'
}
# The expiry exists so a borrowed teammate's IP cannot silently outlive the
# arrangement. A loopback endpoint borrows nothing, so it does not expire.
if (-not $isLoopbackEndpoint -and -not $AllowExpired -and
    [DateTimeOffset]::Now -gt $activeThrough) {
    throw (
        "The temporary team LAN endpoint expired at $activeThrough. " +
        'Update TeamLanEndpoint.json and the shared runtime contract before use.')
}

$serverHost = $serverAddress.ToString()
$serverPort = [uint16]$endpoint.port
$connectedInterfaceIndexes = @(Get-NetIPInterface -AddressFamily IPv4 `
    -ErrorAction SilentlyContinue | Where-Object {
        $_.ConnectionState -eq 'Connected'
    } | ForEach-Object { $_.InterfaceIndex })
$localAddresses = @(Get-NetIPAddress -AddressFamily IPv4 `
    -ErrorAction SilentlyContinue | Where-Object {
        $_.AddressState -eq 'Preferred' -and
        $connectedInterfaceIndexes -contains $_.InterfaceIndex
    } | ForEach-Object { $_.IPAddress })
$ownsServerAddress = $isLoopbackEndpoint -or
    $localAddresses -contains $serverHost
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
    $canonicalServerArguments = "--bind-address $serverBindAddress"
    Set-ProjectUserProperty `
        -Path $serverUserPath `
        -PropertyName 'LocalDebuggerCommandArguments' `
        -Value $canonicalServerArguments
    Set-ProjectUserProperty `
        -Path $serverUserPath `
        -PropertyName 'LocalDebuggerCommandArgumentsHistory' `
        -Value "$canonicalServerArguments|"
}
Set-ProjectUserEnvironmentVariable `
    -Path $clientUserPath `
    -VariableName 'LOSTARK_SERVER_HOST' `
    -Value $serverHost

# Loopback never leaves the machine, so it needs no inbound rule - and asking
# for one would demand an elevated shell for nothing.
if ('server-host' -eq $effectiveRole -and -not $isLoopbackEndpoint) {
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
