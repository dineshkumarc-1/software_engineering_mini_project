#requires -RunAsAdministrator
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ExecutablePath,
    [ValidateRange(10, 600)]
    [int]$DurationSeconds = 60,
    [string]$OutputPath = ".\issue-24-offline-evidence.json"
)

$ErrorActionPreference = "Stop"
$resolvedExecutable = (Resolve-Path -LiteralPath $ExecutablePath).Path
$ruleName = "SnakeXtreme-Issue24-Block-$([guid]::NewGuid().ToString('N'))"
$ruleCreated = $false
$observations = [System.Collections.Generic.List[object]]::new()

try {
    $ruleParams = @{
        PolicyStore  = "PersistentStore"
        DisplayName  = $ruleName
        Description  = "Temporary Issue #24 offline-evidence collection; removed by collector."
        Direction    = "Outbound"
        Program      = $resolvedExecutable
        Action       = "Block"
        Profile      = "Any"
        Enabled      = "True"
    }
    $null = New-NetFirewallRule @ruleParams
    $ruleCreated = $true

    $activeRules = @(Get-NetFirewallRule -PolicyStore ActiveStore -DisplayName $ruleName)
    if ($activeRules.Count -ne 1) {
        throw "The temporary outbound block rule was not found exactly once in ActiveStore."
    }

    $activeRule = $activeRules[0]
    $appFilters = @(Get-NetFirewallApplicationFilter -AssociatedNetFirewallRule $activeRule)
    if ($activeRule.Enabled -ne "True" -or
        $activeRule.Direction -ne "Outbound" -or
        $activeRule.Action -ne "Block" -or
        $appFilters.Count -ne 1 -or
        $appFilters[0].Program -ine $resolvedExecutable) {
        throw "The ActiveStore rule did not match the expected enabled outbound block for this executable."
    }

    $process = Start-Process -FilePath $resolvedExecutable -PassThru
    $captureStarted = Get-Date

    for ($second = 0; $second -lt $DurationSeconds; $second++) {
        $tcp = @(
            Get-NetTCPConnection -OwningProcess $process.Id -ErrorAction SilentlyContinue |
                Select-Object LocalAddress, LocalPort, RemoteAddress, RemotePort, State
        )
        $udp = @(
            Get-NetUDPEndpoint -OwningProcess $process.Id -ErrorAction SilentlyContinue |
                Select-Object LocalAddress, LocalPort
        )

        $isRunning = $false
        try {
            $null = Get-Process -Id $process.Id -ErrorAction Stop
            $isRunning = $true
        } catch {
            $isRunning = $false
        }

        $observations.Add([pscustomobject]@{
            Timestamp = (Get-Date).ToString("o")
            ProcessId = $process.Id
            ProcessRunning = $isRunning
            TcpConnections = $tcp
            UdpEndpoints = $udp
        })

        Start-Sleep -Seconds 1
    }

    $activeRulesAtEnd = @(Get-NetFirewallRule -PolicyStore ActiveStore -DisplayName $ruleName)
    $firewallStillActive = $activeRulesAtEnd.Count -eq 1 -and
        $activeRulesAtEnd[0].Enabled -eq "True" -and
        $activeRulesAtEnd[0].Direction -eq "Outbound" -and
        $activeRulesAtEnd[0].Action -eq "Block"

    $result = [pscustomobject]@{
        CapturedAt = (Get-Date).ToString("o")
        ComputerName = $env:COMPUTERNAME
        OperatingSystem = (Get-CimInstance Win32_OperatingSystem).Caption
        ExecutablePath = $resolvedExecutable
        ProcessId = $process.Id
        CaptureStartedAt = $captureStarted.ToString("o")
        DurationSecondsRequested = $DurationSeconds
        ObservationCount = $observations.Count
        ActiveStoreRuleVerifiedAtStart = $true
        ActiveStoreRuleVerifiedAtEnd = $firewallStillActive
        FirewallRule = [pscustomobject]@{
            DisplayName = $ruleName
            EnabledAtStart = $activeRule.Enabled
            Direction = $activeRule.Direction
            Action = $activeRule.Action
            Profile = [string]$activeRule.Profile
            Program = $appFilters[0].Program
        }
        Observations = $observations
        Limitations = @(
            "Interact with the game during the capture and record separately whether normal play/menu navigation remained functional.",
            "TCP connection and UDP endpoint snapshots are process-scoped observations, not a packet capture.",
            "A UDP endpoint does not identify every remote destination; do not describe this result as proof that no network attempt is possible."
        )
    }

    $fullOutputPath = [System.IO.Path]::GetFullPath($OutputPath)
    $parentDirectory = Split-Path -Parent $fullOutputPath
    if ($parentDirectory -and -not (Test-Path -LiteralPath $parentDirectory)) {
        New-Item -ItemType Directory -Path $parentDirectory -Force | Out-Null
    }
    $result | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $fullOutputPath -Encoding UTF8

    Write-Host "Evidence saved to: $fullOutputPath"
    Write-Host "PID: $($process.Id); samples: $($observations.Count); rule active at end: $firewallStillActive"
    Write-Host "Interact with the game throughout the capture; note whether it remained usable."
}
finally {
    if ($ruleCreated) {
        Remove-NetFirewallRule -PolicyStore PersistentStore -DisplayName $ruleName -ErrorAction SilentlyContinue
    }
}
