[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateScript({ Test-Path -LiteralPath $_ -PathType Leaf })]
    [string]$ExecutablePath,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9a-fA-F]{40}$')]
    [string]$SourceCommitSha,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9a-fA-F]{40}$')]
    [string]$SourceTreeSha,

    [ValidateRange(10, 600)]
    [int]$DurationSeconds = 60,

    [ValidateRange(250, 10000)]
    [int]$SampleIntervalMilliseconds = 1000,

    [string]$OutputPath = ".\issue-24-exact-build-raw.json",

    [switch]$EnableScopedOutboundBlock
)

$ErrorActionPreference = "Stop"

function Add-CollectionError {
    param(
        [System.Collections.Generic.List[object]]$Target,
        [string]$Stage,
        [string]$Message,
        [string]$ErrorType
    )

    $Target.Add([pscustomobject]@{
        TimestampUtc = [DateTimeOffset]::UtcNow.ToString("o")
        Stage = $Stage
        ErrorType = $ErrorType
        Message = $Message
    })
}

$errors = [System.Collections.Generic.List[object]]::new()
$observations = [System.Collections.Generic.List[object]]::new()
$firewall = [ordered]@{
    Requested = [bool]$EnableScopedOutboundBlock
    RuleCreated = $false
    RuleName = $null
    VerifiedAtStart = $null
    VerifiedAtEndBeforeCleanup = $null
    CleanupAttempted = $false
    CleanupSucceeded = $null
    VerifiedAbsentAfterCleanup = $null
    CleanupError = $null
}
$process = $null
$stopwatch = [System.Diagnostics.Stopwatch]::new()
$captureStartedAt = $null
$captureEndedAt = $null
$fatalError = $null
$gameplayObservation = "Not recorded"
$resolvedExecutable = $null
$executableInfo = $null
$executableHash = $null
$osCaption = $null
$osVersion = $null
$osBuild = $null

try {
    $resolvedExecutable = (Resolve-Path -LiteralPath $ExecutablePath).Path
    $executableInfo = Get-Item -LiteralPath $resolvedExecutable
    $executableHash = (Get-FileHash -LiteralPath $resolvedExecutable -Algorithm SHA256).Hash

    try {
        $os = Get-CimInstance -ClassName Win32_OperatingSystem -ErrorAction Stop
        $osCaption = $os.Caption
        $osVersion = $os.Version
        $osBuild = $os.BuildNumber
    }
    catch {
        Add-CollectionError -Target $errors -Stage "OperatingSystemIdentity" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
    }

    if ($EnableScopedOutboundBlock) {
        $identity = [System.Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = [System.Security.Principal.WindowsPrincipal]::new($identity)
        $adminRole = [System.Security.Principal.WindowsBuiltInRole]::Administrator

        if (-not $principal.IsInRole($adminRole)) {
            throw "The firewall mode requires an elevated PowerShell session. No firewall rule was created."
        }

        $approval = Read-Host "Owner approval is required. Type APPROVED to create one temporary outbound block rule scoped only to this EXE"
        if ($approval -cne "APPROVED") {
            throw "The approval phrase was not supplied. No firewall rule was created."
        }

        $ruleDisplayName = "SnakeXtreme-Issue24-$([guid]::NewGuid().ToString('N'))"
        $ruleParams = @{
            PolicyStore = "PersistentStore"
            DisplayName = $ruleDisplayName
            Description = "Temporary Issue #24 exact-build test rule; removed by the collector."
            Direction = "Outbound"
            Program = $resolvedExecutable
            Action = "Block"
            Profile = "Any"
            Enabled = "True"
        }
        $newRule = New-NetFirewallRule @ruleParams -ErrorAction Stop

        $firewall.RuleCreated = $true
        $firewall.RuleName = $newRule.Name

        $activeRule = Get-NetFirewallRule -PolicyStore ActiveStore -Name $newRule.Name -ErrorAction Stop
        $appFilters = @(Get-NetFirewallApplicationFilter -AssociatedNetFirewallRule $activeRule -ErrorAction Stop)
        $ruleValid = (
            $activeRule.Enabled -eq "True" -and
            $activeRule.Direction -eq "Outbound" -and
            $activeRule.Action -eq "Block" -and
            $appFilters.Count -eq 1 -and
            $appFilters[0].Program -ieq $resolvedExecutable
        )

        $firewall.VerifiedAtStart = [bool]$ruleValid
        if (-not $ruleValid) {
            throw "The ActiveStore rule did not match the exact enabled outbound block for this executable."
        }
    }

    $process = Start-Process -FilePath $resolvedExecutable -PassThru -ErrorAction Stop
    $captureStartedAt = [DateTimeOffset]::UtcNow
    $stopwatch.Start()

    $nextSampleAtMilliseconds = 0.0
    $lastSampleAtMilliseconds = $null

    while ($stopwatch.Elapsed.TotalSeconds -lt $DurationSeconds) {
        while ($stopwatch.Elapsed.TotalMilliseconds -lt $nextSampleAtMilliseconds) {
            $remaining = $nextSampleAtMilliseconds - $stopwatch.Elapsed.TotalMilliseconds
            $sleepMilliseconds = [int][Math]::Max(1, [Math]::Min(100, [Math]::Ceiling($remaining)))
            Start-Sleep -Milliseconds $sleepMilliseconds
        }

        if ($stopwatch.Elapsed.TotalSeconds -ge $DurationSeconds) {
            break
        }

        $sampleElapsedMilliseconds = $stopwatch.Elapsed.TotalMilliseconds
        $sampleErrors = [System.Collections.Generic.List[object]]::new()
        $tcp = @()
        $udp = @()
        $tcpSucceeded = $true
        $udpSucceeded = $true
        $processRunning = $null
        $processLivenessSucceeded = $true
        $processState = "Unknown"

        try {
            $tcp = @(
                Get-NetTCPConnection -OwningProcess $process.Id -ErrorAction Stop |
                    Select-Object LocalAddress, LocalPort, RemoteAddress, RemotePort, State
            )
        }
        catch {
            $tcpSucceeded = $false
            Add-CollectionError -Target $sampleErrors -Stage "Get-NetTCPConnection" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
            Add-CollectionError -Target $errors -Stage "Get-NetTCPConnection" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
        }

        try {
            $udp = @(
                Get-NetUDPEndpoint -OwningProcess $process.Id -ErrorAction Stop |
                    Select-Object LocalAddress, LocalPort
            )
        }
        catch {
            $udpSucceeded = $false
            Add-CollectionError -Target $sampleErrors -Stage "Get-NetUDPEndpoint" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
            Add-CollectionError -Target $errors -Stage "Get-NetUDPEndpoint" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
        }

        try {
            $process.Refresh()
            $processRunning = -not $process.HasExited
            if ($processRunning) {
                $processState = "Running"
            }
            else {
                $processState = "Exited"
            }
        }
        catch {
            $processLivenessSucceeded = $false
            $processState = "QueryError"
            Add-CollectionError -Target $sampleErrors -Stage "ProcessLiveness" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
            Add-CollectionError -Target $errors -Stage "ProcessLiveness" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
        }

        $intervalMilliseconds = $null
        if ($null -ne $lastSampleAtMilliseconds) {
            $intervalMilliseconds = [Math]::Round($sampleElapsedMilliseconds - $lastSampleAtMilliseconds, 3)
        }

        $observations.Add([pscustomobject]@{
            TimestampUtc = [DateTimeOffset]::UtcNow.ToString("o")
            ElapsedMilliseconds = [Math]::Round($sampleElapsedMilliseconds, 3)
            IntervalSincePreviousMilliseconds = $intervalMilliseconds
            ProcessId = $process.Id
            ProcessRunning = $processRunning
            ProcessState = $processState
            TcpQuerySucceeded = $tcpSucceeded
            TcpConnections = $tcp
            UdpQuerySucceeded = $udpSucceeded
            UdpEndpoints = $udp
            CollectionErrors = @($sampleErrors)
        })

        $lastSampleAtMilliseconds = $sampleElapsedMilliseconds
        $nextSampleAtMilliseconds = $sampleElapsedMilliseconds + $SampleIntervalMilliseconds
    }

    $captureEndedAt = [DateTimeOffset]::UtcNow

    if ($EnableScopedOutboundBlock -and $firewall.RuleCreated) {
        try {
            $endRules = @(Get-NetFirewallRule -PolicyStore ActiveStore -Name $firewall.RuleName -ErrorAction Stop)
            if ($endRules.Count -eq 1) {
                $endFilters = @(Get-NetFirewallApplicationFilter -AssociatedNetFirewallRule $endRules[0] -ErrorAction Stop)
                $firewall.VerifiedAtEndBeforeCleanup = [bool](
                    $endRules[0].Enabled -eq "True" -and
                    $endRules[0].Direction -eq "Outbound" -and
                    $endRules[0].Action -eq "Block" -and
                    $endFilters.Count -eq 1 -and
                    $endFilters[0].Program -ieq $resolvedExecutable
                )
            }
            else {
                $firewall.VerifiedAtEndBeforeCleanup = $false
                Add-CollectionError -Target $errors -Stage "FirewallEndVerification" -Message "The exact temporary rule was not present exactly once in ActiveStore at the end of sampling." -ErrorType "VerificationFailure"
            }
        }
        catch {
            $firewall.VerifiedAtEndBeforeCleanup = $false
            Add-CollectionError -Target $errors -Stage "FirewallEndVerification" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
        }
    }

    if ($EnableScopedOutboundBlock) {
        $gameplayObservation = Read-Host "Record what you observed while the rule was active (for example: menu and gameplay remained usable)"
    }
}
catch {
    $fatalError = $_.Exception.Message
    Add-CollectionError -Target $errors -Stage "Capture" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
}
finally {
    if ($firewall.RuleCreated -and $firewall.RuleName) {
        $firewall.CleanupAttempted = $true
        try {
            Remove-NetFirewallRule -PolicyStore PersistentStore -Name $firewall.RuleName -ErrorAction Stop
            $firewall.CleanupSucceeded = $true
        }
        catch {
            $firewall.CleanupSucceeded = $false
            $firewall.CleanupError = $_.Exception.Message
            Add-CollectionError -Target $errors -Stage "FirewallCleanup" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
        }

        try {
            $lookupErrors = @()
            $remainingRules = @(Get-NetFirewallRule -PolicyStore ActiveStore -Name $firewall.RuleName -ErrorAction SilentlyContinue -ErrorVariable lookupErrors)
            if ($lookupErrors.Count -eq 0) {
                $firewall.VerifiedAbsentAfterCleanup = ($remainingRules.Count -eq 0)
            }
            else {
                $firewall.VerifiedAbsentAfterCleanup = $false
                Add-CollectionError -Target $errors -Stage "FirewallCleanupVerification" -Message (($lookupErrors | ForEach-Object { $_.Exception.Message }) -join "; ") -ErrorType "QueryError"
            }
        }
        catch {
            $firewall.VerifiedAbsentAfterCleanup = $false
            Add-CollectionError -Target $errors -Stage "FirewallCleanupVerification" -Message $_.Exception.Message -ErrorType $_.Exception.GetType().FullName
        }
    }
}

if ($stopwatch.IsRunning) {
    $stopwatch.Stop()
}
if (-not $captureEndedAt) {
    $captureEndedAt = [DateTimeOffset]::UtcNow
}
if (-not $captureStartedAt) {
    $captureStartedAt = $captureEndedAt
}

$intervals = @(
    $observations |
        Where-Object { $null -ne $_.IntervalSincePreviousMilliseconds } |
        ForEach-Object { [double]$_.IntervalSincePreviousMilliseconds }
)
$intervalSummary = $null
if ($intervals.Count -gt 0) {
    $intervalSummary = [ordered]@{
        Count = $intervals.Count
        MinimumMilliseconds = [Math]::Round(($intervals | Measure-Object -Minimum).Minimum, 3)
        AverageMilliseconds = [Math]::Round(($intervals | Measure-Object -Average).Average, 3)
        MaximumMilliseconds = [Math]::Round(($intervals | Measure-Object -Maximum).Maximum, 3)
    }
}

$result = [ordered]@{
    SchemaVersion = 2
    CapturedAtUtc = [DateTimeOffset]::UtcNow.ToString("o")
    ComputerName = $env:COMPUTERNAME
    OperatingSystem = [ordered]@{
        Caption = $osCaption
        Version = $osVersion
        BuildNumber = $osBuild
    }
    PowerShellVersion = $PSVersionTable.PSVersion.ToString()
    Source = [ordered]@{
        CommitSha = $SourceCommitSha.ToLowerInvariant()
        TreeSha = $SourceTreeSha.ToLowerInvariant()
    }
    Executable = [ordered]@{
        FullPath = $resolvedExecutable
        SizeBytes = if ($executableInfo) { $executableInfo.Length } else { $null }
        Sha256 = if ($executableHash) { $executableHash.ToLowerInvariant() } else { $null }
    }
    Process = [ordered]@{
        Id = if ($process) { $process.Id } else { $null }
        StateAtEnd = if ($process) {
            try {
                $process.Refresh()
                if ($process.HasExited) { "Exited" } else { "Running" }
            }
            catch {
                "QueryError"
            }
        } else {
            "NotStarted"
        }
    }
    Capture = [ordered]@{
        StartedAtUtc = $captureStartedAt.ToString("o")
        EndedAtUtc = $captureEndedAt.ToString("o")
        RequestedDurationSeconds = $DurationSeconds
        ActualDurationMilliseconds = [Math]::Round($stopwatch.Elapsed.TotalMilliseconds, 3)
        SampleIntervalRequestedMilliseconds = $SampleIntervalMilliseconds
        ObservationCount = $observations.Count
        IntervalSummary = $intervalSummary
    }
    Firewall = $firewall
    GameplayObservation = $gameplayObservation
    FatalError = $fatalError
    CollectionErrors = $errors
    Observations = $observations
    Limitations = @(
        "TCP connections and UDP endpoints are process-owned point-in-time snapshots, not packet capture.",
        "No observed endpoint in a sample does not rule out brief activity between samples.",
        "UDP endpoint records show local endpoints and do not identify every remote destination.",
        "This evidence does not prove that no network attempt is possible.",
        "A passive run does not establish that Windows Firewall blocked outbound traffic.",
        "Keep this raw file private: it includes the computer name and full executable path."
    )
}

$fullOutputPath = [System.IO.Path]::GetFullPath($OutputPath)
$parentDirectory = Split-Path -Parent $fullOutputPath
if ($parentDirectory -and -not (Test-Path -LiteralPath $parentDirectory)) {
    New-Item -ItemType Directory -Path $parentDirectory -Force | Out-Null
}
$result | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $fullOutputPath -Encoding UTF8

Write-Host "Raw evidence saved to: $fullOutputPath"
Write-Host "Source: $($SourceCommitSha.ToLowerInvariant()); EXE size: $($executableInfo.Length) bytes; EXE SHA-256: $($executableHash.ToLowerInvariant())"
Write-Host "Samples: $($observations.Count); actual elapsed: $([Math]::Round($stopwatch.Elapsed.TotalMilliseconds, 3)) ms; collection errors: $($errors.Count)"

if ($EnableScopedOutboundBlock) {
    Write-Host "Firewall start verified: $($firewall.VerifiedAtStart); end verified: $($firewall.VerifiedAtEndBeforeCleanup); removed: $($firewall.CleanupSucceeded); absent after cleanup: $($firewall.VerifiedAbsentAfterCleanup)"
}

if ($fatalError) {
    throw "Capture ended with an error. The partial evidence was preserved at $fullOutputPath. Error: $fatalError"
}
if ($EnableScopedOutboundBlock -and ($firewall.CleanupSucceeded -ne $true -or $firewall.VerifiedAbsentAfterCleanup -ne $true)) {
    throw "Firewall cleanup could not be fully verified. Preserve the evidence and inspect Windows Firewall before further testing."
}
