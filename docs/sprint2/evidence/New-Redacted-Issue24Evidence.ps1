[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateScript({ Test-Path -LiteralPath $_ -PathType Leaf })]
    [string]$RawEvidencePath,

    [string]$RedactedOutputPath = ".\issue-24-exact-build-redacted.json"
)

$ErrorActionPreference = "Stop"

$rawPath = (Resolve-Path -LiteralPath $RawEvidencePath).Path
$outputPath = [System.IO.Path]::GetFullPath($RedactedOutputPath)
if ([string]::Equals($rawPath, $outputPath, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "The redacted output path must differ from the private raw evidence path."
}
if (Test-Path -LiteralPath $outputPath) {
    throw "Redacted output already exists. Choose a new path; do not overwrite previous evidence."
}

$rawText = Get-Content -LiteralPath $rawPath -Raw -ErrorAction Stop
$evidence = $rawText | ConvertFrom-Json -ErrorAction Stop

if (-not $evidence.Source.CommitSha -or -not $evidence.Executable.Sha256) {
    throw "Input does not contain the expected exact-build source and executable identity fields."
}

$privateValues = [System.Collections.Generic.List[string]]::new()
if ($evidence.ComputerName) {
    $privateValues.Add([string]$evidence.ComputerName)
    $evidence.ComputerName = "<REDACTED_COMPUTER>"
}
if ($evidence.Executable.FullPath) {
    $privateValues.Add([string]$evidence.Executable.FullPath)
    $evidence.Executable.FullPath = "<REDACTED_EXECUTABLE_PATH>"
}
if ($evidence.Firewall -and $evidence.Firewall.RuleName) {
    $privateValues.Add([string]$evidence.Firewall.RuleName)
    $evidence.Firewall.RuleName = "<REDACTED_TEMPORARY_RULE>"
}

$evidence | Add-Member -NotePropertyName RedactionNotice -NotePropertyValue @(
    "This is a redacted derivative of a private raw evidence file.",
    "Computer name, full executable path, and temporary firewall rule name were redacted.",
    "Review this file manually before publication; collection error messages may contain additional local details."
) -Force

$redactedText = $evidence | ConvertTo-Json -Depth 16
foreach ($privateValue in $privateValues) {
    if (-not [string]::IsNullOrEmpty($privateValue)) {
        $redactedText = $redactedText.Replace($privateValue, "<REDACTED_LOCAL_VALUE>")
    }
}

$parentDirectory = Split-Path -Parent $outputPath
if ($parentDirectory -and -not (Test-Path -LiteralPath $parentDirectory)) {
    New-Item -ItemType Directory -Path $parentDirectory -Force | Out-Null
}

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
[System.IO.File]::WriteAllText($outputPath, $redactedText + [Environment]::NewLine, $utf8NoBom)

$hash = (Get-FileHash -LiteralPath $outputPath -Algorithm SHA256).Hash
Write-Host "Redacted evidence written to: $outputPath"
Write-Host "Redacted JSON SHA-256: $hash"
Write-Warning "Manually inspect the complete redacted JSON for local paths, usernames, hostnames, or details in error messages before publishing. Keep the raw input private."
