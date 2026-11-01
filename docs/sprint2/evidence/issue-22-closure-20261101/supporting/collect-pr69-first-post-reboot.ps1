$ErrorActionPreference = 'Stop'

$packageDir = Split-Path -Parent $PSScriptRoot
$repoDir = (Resolve-Path (Join-Path $packageDir '..\..\..\..')).Path
$workspace = (Resolve-Path (Join-Path $repoDir '..')).Path
$evidenceDir = Join-Path $workspace 'evidence\startup-pr69-post-reboot-20261101-retry01'
$launchDir = Join-Path $evidenceDir 'launches'
$exe = Join-Path $workspace 'source-pr69\build-pr69-app\Release\SnakeXtreme.exe'
$sourceCommit = '841ee1dccac37236a4bdcf92ec3dbf17e7a18c80'
$taskName = 'Issue22-PR69-PostReboot-Once'
$started = Get-Date
New-Item -ItemType Directory -Force -Path $evidenceDir | Out-Null

try {
    Start-Sleep -Seconds 120
    Remove-Item Env:ALSOFT_DRIVERS -ErrorAction SilentlyContinue

    $os = Get-CimInstance Win32_OperatingSystem
    $cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
    $gpus = @(Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion)
    $computer = Get-CimInstance Win32_ComputerSystem
    $battery = @(Get-CimInstance Win32_Battery -ErrorAction SilentlyContinue | Select-Object BatteryStatus,EstimatedChargeRemaining)
    Add-Type -AssemblyName System.Windows.Forms
    $powerLine = [System.Windows.Forms.SystemInformation]::PowerStatus.PowerLineStatus.ToString()
    $audio = @(Get-CimInstance Win32_SoundDevice | Select-Object Name,Status)
    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    $stream = [System.IO.File]::OpenRead($exe)
    try { $exeHash = [System.BitConverter]::ToString($sha256.ComputeHash($stream)).Replace('-', '').ToLowerInvariant() }
    finally { $stream.Dispose(); $sha256.Dispose() }

    New-Item -ItemType Directory -Force -Path $evidenceDir | Out-Null
    $manifest = [ordered]@{
        protocol = 'Reboot; sign in; settle for 120 seconds; normal audio; first launch followed by five warm launches.'
        antivirus_disabled = $false
        audio_null_output = $false
        boot_time_local = $os.LastBootUpTime.ToString('o')
        collector_start_local = $started.ToString('o')
        wait_after_task_start_seconds = 120
        collection_start_local = (Get-Date).ToString('o')
        os = "$($os.Caption), build $($os.BuildNumber)"
        model = $computer.Model
        cpu = $cpu.Name
        gpus = $gpus
        power_line_status = $powerLine
        battery = $battery
        audio_devices = $audio
        source_commit = $sourceCommit
        executable_size_bytes = (Get-Item $exe).Length
        executable_sha256 = $exeHash
        method = 'collect_launch.py external process creation request to stdout marker after first display; raw phase report also retained'
        acceptance = 'not_assessed_pending_review'
    }
    $manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidenceDir 'protocol-metadata.json') -Encoding utf8

    $env:ALSOFT_DRIVERS = $null
    & python (Join-Path $repoDir 'tools\collect_launch.py') $exe $launchDir --runs 6 --source-snapshot $sourceCommit 2>&1 |
        Set-Content -LiteralPath (Join-Path $evidenceDir 'collector-console.txt') -Encoding utf8
    $exitCode = $LASTEXITCODE
    Set-Content -LiteralPath (Join-Path $evidenceDir 'collector-exit-code.txt') -Value $exitCode -Encoding ascii
}
catch {
    $_ | Out-String | Set-Content -LiteralPath (Join-Path $evidenceDir 'collector-error.txt') -Encoding utf8
}
finally {
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
}
