# Leapfrog Android build - runs locally on Windows (no Mac needed).
# Builds proj.android with Gradle, optionally installs and launches on a connected phone.
# Examples:
#   .\build-android.ps1                          # Debug build
#   .\build-android.ps1 -Install                 # ...and install + launch on the phone
#   .\build-android.ps1 -Configuration Release -Install
#   .\build-android.ps1 -Install -NoBuild        # reinstall the last built APK
#   .\build-android.ps1 -Clean                   # clean build
param(
    [ValidateSet("Debug", "Release")][string]$Configuration = "Debug",
    [switch]$Install,
    [switch]$NoBuild,
    [switch]$Clean,
    [string]$Device = ""        # adb serial, only needed if several devices are connected
)

$ErrorActionPreference = "Stop"
$here    = Split-Path -Parent $MyInvocation.MyCommand.Path
$projDir = Resolve-Path (Join-Path $here "..\proj.android")
$package = "org.oxygine.Leapfrog"
$activity = "org.oxygine.Leapfrog.MainActivity"

# --- Java: Gradle needs a JDK 17+. Fall back to the one bundled with Android Studio.
if (-not $env:JAVA_HOME -and -not (Get-Command java -ErrorAction SilentlyContinue)) {
    $jbr = "C:\Program Files\Android\Android Studio\jbr"
    if (Test-Path $jbr) { $env:JAVA_HOME = $jbr }
    else { throw "No Java found. Set JAVA_HOME to a JDK 17+ (e.g. Android Studio's 'jbr' folder)." }
}

# --- Android SDK: read sdk.dir from local.properties to find adb.
$sdkDir = $env:ANDROID_HOME
$localProps = Join-Path $projDir "local.properties"
if (-not $sdkDir -and (Test-Path $localProps)) {
    $line = Select-String -Path $localProps -Pattern '^sdk\.dir=' | Select-Object -First 1
    if ($line) { $sdkDir = ($line.Line -replace '^sdk\.dir=', '') -replace '\\\\', '\' -replace '\\:', ':' }
}
$adb = if ($sdkDir -and (Test-Path "$sdkDir\platform-tools\adb.exe")) { "$sdkDir\platform-tools\adb.exe" } else { "adb" }

# --- Build
if (-not $NoBuild) {
    $tasks = @()
    if ($Clean) { $tasks += "clean" }
    $tasks += "assemble$Configuration"

    Write-Host "== Building $Configuration ($($tasks -join ', '))"
    Push-Location $projDir
    try {
        & .\gradlew.bat @tasks --console=plain
        if ($LASTEXITCODE -ne 0) { Write-Host "== BUILD FAILED" -ForegroundColor Red; exit $LASTEXITCODE }
    } finally { Pop-Location }
    Write-Host "== BUILD SUCCEEDED" -ForegroundColor Green
}

$apkDir = Join-Path $projDir "build\outputs\apk\$($Configuration.ToLower())"
$apk = Get-ChildItem $apkDir -Filter *.apk -ErrorAction SilentlyContinue |
       Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $apk) { throw "No APK found in $apkDir" }
Write-Host "APK: $($apk.FullName)"

# --- Install and launch
if ($Install) {
    $devices = & $adb devices | Select-String "\tdevice$"
    if (-not $devices) {
        throw "No Android device found. Connect the phone with USB debugging enabled (or pair it with 'adb pair' for wireless debugging)."
    }
    $adbArgs = @()
    if ($Device) { $adbArgs = @("-s", $Device) }
    elseif (@($devices).Count -gt 1) { throw "Several devices connected - pick one with -Device <serial> (see 'adb devices')." }

    Write-Host "== Installing on device"
    & $adb @adbArgs install -r $apk.FullName
    if ($LASTEXITCODE -ne 0) { throw "adb install failed" }

    Write-Host "== Launching $package"
    & $adb @adbArgs shell am start -n "$package/$activity"
}
