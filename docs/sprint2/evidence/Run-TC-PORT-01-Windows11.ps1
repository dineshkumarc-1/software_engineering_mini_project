[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9a-fA-F]{40}$')]
    [string]$CandidateSha,

    [string]$EvidenceRoot = ".\TC-PORT-01-Windows11-Evidence"
)

$ErrorActionPreference = "Stop"
$EvidenceRoot = [System.IO.Path]::GetFullPath($EvidenceRoot)

if (Test-Path -LiteralPath $EvidenceRoot) {
    throw "EvidenceRoot already exists. Choose a new empty path so earlier attempts are never overwritten: $EvidenceRoot"
}

$null = New-Item -ItemType Directory -Path $EvidenceRoot
$checkoutDirectory = Join-Path $EvidenceRoot "source"
$buildDirectory = Join-Path $EvidenceRoot "build"
$summaryPath = Join-Path $EvidenceRoot "run-summary.txt"
$transcriptPath = Join-Path $EvidenceRoot "operator-transcript.txt"
$configureLog = Join-Path $EvidenceRoot "configure.log"
$buildLog = Join-Path $EvidenceRoot "build-release.log"
$inventoryLog = Join-Path $EvidenceRoot "ctest-inventory.log"
$ctestLog = Join-Path $EvidenceRoot "ctest-release.log"
$toolchainLog = Join-Path $EvidenceRoot "toolchain.log"
$identityLog = Join-Path $EvidenceRoot "source-and-machine-identity.txt"

$repoUrl = "https://github.com/dineshkumarc-1/software_engineering_mini_project.git"
$expectedWindowsCTestCount = 20
$configureExitCode = $null
$buildExitCode = $null
$inventoryExitCode = $null
$ctestExitCode = $null
$registeredTestCount = $null
$exeSizeBytes = $null
$exeSha256 = $null
$sourceTreeSha = $null
$workingTreeStatus = "Not checked"
$overallError = $null
$manualChecks = @(
    "Menu window opens and menu text is visible (font loaded).",
    "Classic: eat food and verify the score increases by +10.",
    "Timed: countdown starts at 1:00 and runs down.",
    "Obstacle: collide with a brown block and observe You hit an obstacle.",
    "Collect a power-up; HUD shows its name and remaining seconds.",
    "Esc pauses/resumes; M returns to menu; Esc on menu exits.",
    "Window X closes the game cleanly.",
    "Leaderboard shows the score after a game; a changed setting survives restart."
)

Start-Transcript -Path $transcriptPath -NoClobber | Out-Null

function Invoke-LoggedNative {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string[]]$Arguments,

        [Parameter(Mandatory = $true)]
        [string]$LogPath
    )

    $commandLine = "$Path " + ($Arguments -join " ")
    "COMMAND: $commandLine" | Tee-Object -FilePath $LogPath -Append | Out-Host
    & $Path @Arguments 2>&1 | Tee-Object -FilePath $LogPath -Append | Out-Host
    $nativeExitCode = $LASTEXITCODE
    "EXIT CODE: $nativeExitCode" | Tee-Object -FilePath $LogPath -Append | Out-Host
    return $nativeExitCode
}

try {
    $gitCommand = Get-Command git.exe -ErrorAction Stop
    $cmakeCommand = Get-Command cmake.exe -ErrorAction Stop
    $ctestCommand = Get-Command ctest.exe -ErrorAction Stop
    $clCommand = Get-Command cl.exe -ErrorAction Stop

    $os = Get-CimInstance -ClassName Win32_OperatingSystem -ErrorAction Stop
    if ($os.Caption -notmatch "Windows 11") {
        throw "This helper is for a Windows 11 PC. Detected OS: $($os.Caption) $($os.Version) build $($os.BuildNumber)."
    }
    $machineInfo = @(
        "CapturedAtUtc: $([DateTimeOffset]::UtcNow.ToString('o'))"
        "ComputerName: $env:COMPUTERNAME"
        "OSCaption: $($os.Caption)"
        "OSVersion: $($os.Version)"
        "OSBuildNumber: $($os.BuildNumber)"
        "OSArchitecture: $($os.OSArchitecture)"
        "PowerShellVersion: $($PSVersionTable.PSVersion)"
        "GitPath: $($gitCommand.Source)"
        "CMakePath: $($cmakeCommand.Source)"
        "CTestPath: $($ctestCommand.Source)"
        "CLPath: $($clCommand.Source)"
        "CandidateShaRequested: $($CandidateSha.ToLowerInvariant())"
        "BuildConfiguration: Release"
        "CMakeOptions: -DSNAKE_FETCH_SFML=ON -DBUILD_SHARED_LIBS=OFF"
        "ExpectedWindowsCTestCount: $expectedWindowsCTestCount"
    )
    $machineInfo | Set-Content -LiteralPath $identityLog -Encoding UTF8
    $machineInfo | Tee-Object -FilePath $summaryPath

    "=== Toolchain versions ===" | Tee-Object -FilePath $toolchainLog
    Invoke-LoggedNative -Path $cmakeCommand.Source -Arguments @("--version") -LogPath $toolchainLog | Out-Null
    Invoke-LoggedNative -Path $gitCommand.Source -Arguments @("--version") -LogPath $toolchainLog | Out-Null
    Invoke-LoggedNative -Path $clCommand.Source -Arguments @("/Bv") -LogPath $toolchainLog | Out-Null

    $programFilesX86 = [Environment]::GetEnvironmentVariable("ProgramFiles(x86)")
    $vswherePath = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswherePath -PathType Leaf) {
        Invoke-LoggedNative -Path $vswherePath -Arguments @(
            "-latest", "-products", "*",
            "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "-property", "installationVersion"
        ) -LogPath $toolchainLog | Out-Null
    }
    else {
        "vswhere.exe not found at expected Visual Studio Installer path." | Tee-Object -FilePath $toolchainLog -Append
    }

    $cloneCode = Invoke-LoggedNative -Path $gitCommand.Source -Arguments @(
        "clone", "--no-checkout", $repoUrl, $checkoutDirectory
    ) -LogPath (Join-Path $EvidenceRoot "git-clone.log")
    if ($cloneCode -ne 0) {
        throw "git clone failed with exit code $cloneCode. The original log is preserved."
    }

    $checkoutCode = Invoke-LoggedNative -Path $gitCommand.Source -Arguments @(
        "-C", $checkoutDirectory, "checkout", "--detach", $CandidateSha
    ) -LogPath (Join-Path $EvidenceRoot "git-checkout.log")
    if ($checkoutCode -ne 0) {
        throw "Checking out the requested candidate failed with exit code $checkoutCode."
    }

    $actualSha = (& $gitCommand.Source -C $checkoutDirectory rev-parse HEAD).Trim()
    $sourceTreeSha = (& $gitCommand.Source -C $checkoutDirectory rev-parse "HEAD^{tree}").Trim()
    $statusLines = @(& $gitCommand.Source -C $checkoutDirectory status --porcelain)
    $workingTreeStatus = if ($statusLines.Count -eq 0) { "Clean" } else { "Not clean" }

    @(
        "RequestedCommitSha: $($CandidateSha.ToLowerInvariant())"
        "ActualCommitSha: $actualSha"
        "SourceTreeSha: $sourceTreeSha"
        "WorkingTreeStatusBeforeBuild: $workingTreeStatus"
        "GitStatusPorcelain:"
        if ($statusLines.Count -eq 0) { "(empty)" } else { $statusLines }
    ) | Set-Content -LiteralPath $identityLog -Encoding UTF8 -Append

    if ($actualSha -ine $CandidateSha) {
        throw "Checked-out SHA $actualSha does not match requested SHA $CandidateSha."
    }
    if ($statusLines.Count -ne 0) {
        throw "The source checkout is not clean. Preserve this evidence and do not proceed as a clean build."
    }

    $configureExitCode = Invoke-LoggedNative -Path $cmakeCommand.Source -Arguments @(
        "-S", $checkoutDirectory,
        "-B", $buildDirectory,
        "-DSNAKE_FETCH_SFML=ON",
        "-DBUILD_SHARED_LIBS=OFF"
    ) -LogPath $configureLog

    if ($configureExitCode -eq 0) {
        $buildExitCode = Invoke-LoggedNative -Path $cmakeCommand.Source -Arguments @(
            "--build", $buildDirectory, "--config", "Release", "--parallel"
        ) -LogPath $buildLog
    }
    else {
        "BUILD NOT RUN: configure returned exit code $configureExitCode." | Set-Content -LiteralPath $buildLog -Encoding UTF8
    }

    if ($buildExitCode -eq 0) {
        "=== ctest -N -C Release ===" | Set-Content -LiteralPath $inventoryLog -Encoding UTF8
        $inventoryOutput = @(
            & $ctestCommand.Source --test-dir $buildDirectory -C Release -N 2>&1 |
                Tee-Object -FilePath $inventoryLog -Append
        )
        $inventoryExitCode = $LASTEXITCODE
        $inventoryText = $inventoryOutput -join [Environment]::NewLine
        $registeredTestCount = [regex]::Matches($inventoryText, "(?m)^\s*Test\s+#\d+:").Count
        "RegisteredTestsParsed: $registeredTestCount" | Tee-Object -FilePath $inventoryLog -Append
        "InventoryExitCode: $inventoryExitCode" | Tee-Object -FilePath $inventoryLog -Append

        $ctestExitCode = Invoke-LoggedNative -Path $ctestCommand.Source -Arguments @(
            "--test-dir", $buildDirectory,
            "-C", "Release",
            "--output-on-failure"
        ) -LogPath $ctestLog
    }
    else {
        "CTest NOT RUN: build did not complete successfully." | Set-Content -LiteralPath $inventoryLog -Encoding UTF8
        "CTest NOT RUN: build did not complete successfully." | Set-Content -LiteralPath $ctestLog -Encoding UTF8
    }

    $exePath = Join-Path $buildDirectory "Release\SnakeXtreme.exe"
    if (Test-Path -LiteralPath $exePath -PathType Leaf) {
        $exeInfo = Get-Item -LiteralPath $exePath
        $exeHash = Get-FileHash -LiteralPath $exePath -Algorithm SHA256
        $exeSizeBytes = $exeInfo.Length
        $exeSha256 = $exeHash.Hash
        @(
            "ExecutablePath: $exePath"
            "ExecutableSizeBytes: $exeSizeBytes"
            "ExecutableSHA256: $exeSha256"
        ) | Set-Content -LiteralPath (Join-Path $EvidenceRoot "executable-identity.txt") -Encoding UTF8

        "Launch the locally built Release EXE for the visible-client checklist: $exePath" | Tee-Object -FilePath $summaryPath -Append
        Start-Process -FilePath $exePath
    }
    else {
        "Release executable was not found at expected path: $exePath" | Tee-Object -FilePath $summaryPath -Append
    }

    "Open winver, capture the version/build dialog, and save the actual screenshot into the evidence folder." | Tee-Object -FilePath $summaryPath -Append
    Start-Process -FilePath "winver.exe"
    $winverScreenshot = Read-Host "Enter the winver screenshot path (or NOT RECORDED)"
    if ([string]::IsNullOrWhiteSpace($winverScreenshot)) {
        $winverScreenshot = "NOT RECORDED"
    }

    "When ready, perform each visible-client check on the locally built EXE. Record Pass, Fail, or Not tested honestly; add the screenshot filename and concise observation." | Tee-Object -FilePath $summaryPath -Append
    $manualResultLines = [System.Collections.Generic.List[string]]::new()
    $manualResultLines.Add("TC-PORT-01 Windows 11 manual observations")
    $manualResultLines.Add("Winver screenshot: $winverScreenshot")
    $manualResultLines.Add("Capture timestamp UTC: $([DateTimeOffset]::UtcNow.ToString('o'))")

    $manualIndex = 0
    foreach ($check in $manualChecks) {
        $manualIndex++
        $manualResult = Read-Host "Check $manualIndex/$($manualChecks.Count): $check [Passed/Failed/Not tested]"
        if ($manualResult -notin @("Passed", "Failed", "Not tested")) {
            $manualResult = "Not tested"
            Write-Warning "Unrecognized result. Recorded as Not tested."
        }
        $observation = Read-Host "Observed result and screenshot filename for check $manualIndex"
        if ([string]::IsNullOrWhiteSpace($observation)) {
            $observation = "No observation or screenshot supplied."
        }
        $manualResultLines.Add("")
        $manualResultLines.Add("[$manualResult] $check")
        $manualResultLines.Add("Observation / screenshot: $observation")
    }

    $manualEvidencePath = Join-Path $EvidenceRoot "manual-gameplay-results.txt"
    $manualResultLines | Set-Content -LiteralPath $manualEvidencePath -Encoding UTF8
    "Manual result record: $manualEvidencePath" | Tee-Object -FilePath $summaryPath -Append
}
catch {
    $overallError = $_.Exception.Message
    "ERROR: $overallError" | Tee-Object -FilePath $summaryPath -Append
}
finally {
    $resultLines = @(
        ""
        "=== Machine-build result summary ==="
        "Candidate SHA: $CandidateSha"
        "Source tree SHA: $sourceTreeSha"
        "Working tree before build: $workingTreeStatus"
        "Configure exit code: $configureExitCode"
        "Build exit code: $buildExitCode"
        "Registered CTest tests: $registeredTestCount (Windows expectation: $expectedWindowsCTestCount)"
        "CTest inventory exit code: $inventoryExitCode"
        "CTest exit code: $ctestExitCode"
        "EXE size bytes: $exeSizeBytes"
        "EXE SHA-256: $exeSha256"
        "Manual gameplay checklist: Not recorded by this script"
        "Overall error: $overallError"
        "Complete logs are preserved in: $EvidenceRoot"
    )
    $resultLines | Tee-Object -FilePath $summaryPath -Append
    Stop-Transcript | Out-Null
}

$testInventoryMatches = ($registeredTestCount -eq $expectedWindowsCTestCount)
$automatedChecksPassed = (
    -not $overallError -and
    $workingTreeStatus -eq "Clean" -and
    $configureExitCode -eq 0 -and
    $buildExitCode -eq 0 -and
    $inventoryExitCode -eq 0 -and
    $testInventoryMatches -and
    $ctestExitCode -eq 0 -and
    $exeSha256
)

if ($automatedChecksPassed) {
    Write-Host "Automated TC-PORT-01 build and CTest steps completed. Manual gameplay and screenshot evidence are still required."
    Write-Host "Evidence folder: $EvidenceRoot"
    exit 0
}

Write-Error "TC-PORT-01 automated steps did not all pass. Preserve the failed-attempt logs in: $EvidenceRoot"
exit 1
