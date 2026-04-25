# Project Release Script

$ErrorActionPreference = "Stop"

# Configuration
$sourceDir = $PSScriptRoot
$releaseDir = Join-Path $sourceDir "release"
$zipFileName = "3.0.0.zip"
$zipFilePath = Join-Path $releaseDir $zipFileName
$fileInfoPath = Join-Path $releaseDir "fileInfo.txt"

# Exclude patterns
$excludePatterns = @(
    ".vscode",
    ".git",
    ".gitignore",
    ".gitattributes",
    "release",
    "create_release.ps1"
)

Write-Host "========================================"
Write-Host "       Project Release Script"
Write-Host "========================================"

# Clean/create release directory
if (Test-Path $releaseDir) {
    Remove-Item $releaseDir -Recurse -Force
}
New-Item -ItemType Directory -Path $releaseDir | Out-Null

# Create temp directory
$tempDir = Join-Path $env:TEMP "release_temp_$(Get-Random)"
New-Item -ItemType Directory -Path $tempDir | Out-Null

# Create secondary directory for extraction
$secondaryDir = "3.0.0"
$secondaryDirPath = Join-Path $tempDir $secondaryDir
New-Item -ItemType Directory -Path $secondaryDirPath | Out-Null

Write-Host "Copying files to temp directory..."

# Copy all files except excluded ones
Get-ChildItem -Path $sourceDir | Where-Object {
    $name = $_.Name
    $excludePatterns -notcontains $name
} | ForEach-Object {
    $dest = Join-Path $secondaryDirPath $_.Name
    Copy-Item $_.FullName -Destination $dest -Recurse -Force
}

Write-Host "Creating zip archive..."
Compress-Archive -Path "$tempDir\*" -DestinationPath $zipFilePath -Force

# Cleanup
Remove-Item $tempDir -Recurse -Force

# Generate file info
$file = Get-Item $zipFilePath
$size = $file.Length
$sizeMB = [math]::Round($size / 1MB, 2)
$hash = (Get-FileHash $zipFilePath -Algorithm SHA256).Hash
$time = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

# Write fileInfo.txt
@"
===============================================
File Information
===============================================
Generated: $time
Package: $zipFileName
Size: $size bytes ($sizeMB MB)
SHA256: $hash
===============================================
Excluded:
$(foreach ($e in $excludePatterns) { "  - $e`n" })
===============================================
"@ | Set-Content $fileInfoPath -Encoding UTF8

Write-Host ""
Write-Host "========================================"
Write-Host "Release Complete!"
Write-Host "========================================"
Write-Host "Zip: $zipFilePath"
Write-Host "Size: $sizeMB MB"
Write-Host "SHA256: $hash"
Write-Host ""
Get-Content $fileInfoPath
