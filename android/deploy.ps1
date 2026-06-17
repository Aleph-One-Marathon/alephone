# Quest VR build: build + install + sync a scenario (verbatim) + launch. Repeatable & idempotent.
#
# Usage:
#   pwsh android\deploy.ps1                          # build, install, sync "Marathon" (M1), launch
#   pwsh android\deploy.ps1 -Scenario "Marathon 2"
#   pwsh android\deploy.ps1 -Scenario "Marathon Infinity"
#   pwsh android\deploy.ps1 -SkipBuild               # don't rebuild; just install + sync + launch
#   pwsh android\deploy.ps1 -NoData                  # code-only redeploy (don't touch device scenario data)
#   pwsh android\deploy.ps1 -Base                    # ALSO push base AO data (do this once on a fresh device)
#
# SCENARIO RULE (learned the hard way, 2026-06-16):
#   Copy the scenario folder from ./data/Scenarios/<name> EXACTLY AS-IS -- original filenames
#   (Map.scen / Map.sceA / Shapes.shps / Shapes.shpA / Images.imgA / Marathon.appl / Plugins / Scripts /
#   Music / Demos / Physics Models / ...). DO NOT rename to plain "Map"/"Shapes" and DO NOT make aliases.
#   The engine resolves the real data filenames from the scenario's own Scripts/MML, so it looks for
#   e.g. "Map.scen" (M1) or "Map.sceA" (M2), NOT a plain "Map". When switching scenarios, wipe the
#   previous scenario's files first so nothing bleeds through; keep base AO data + user saves.

param(
    [string]$Scenario = "Marathon",
    [switch]$SkipBuild,
    [switch]$NoData,
    [switch]$Base
)
$ErrorActionPreference = "Stop"

$repo  = Split-Path -Parent $PSScriptRoot          # android/ is directly under the repo root
$adb   = "$env:LOCALAPPDATA\Android\Sdk\platform-tools\adb.exe"
$apk   = Join-Path $repo "android\app\build\outputs\apk\debug\app-debug.apk"
$files = "/sdcard/Android/data/org.alephone/files"
$scenRoot = Join-Path $repo "data\Scenarios"

function Adb { & $adb @args }

if (-not $SkipBuild) {
    $env:JAVA_HOME = "C:\Program Files\Android\Android Studio\jbr"
    Push-Location (Join-Path $repo "android")
    try { .\gradlew.bat :app:assembleDebug } finally { Pop-Location }
}

Write-Host "Installing $apk"
Adb install -r $apk | Out-Null

# Optionally (re)push base AO data (fonts, MML, themes, icons, mime) -- needed once on a fresh device.
if ($Base) {
    $baseDir = Join-Path $repo "data"
    Write-Host "Pushing base AO data"
    Get-ChildItem -LiteralPath $baseDir -Force |
        Where-Object { $_.Name -ne "Scenarios" -and $_.Name -notmatch '\.(txt|svg|am)$' -and $_.Name -ne "README" } |
        ForEach-Object { Adb push $_.FullName "$files/" | Out-Null; Write-Host "  base + $($_.Name)" }
}

if (-not $NoData) {
    $sdir = Join-Path $scenRoot $Scenario
    if (-not (Test-Path -LiteralPath $sdir)) { throw "Scenario not found: $sdir (expected a subdir of data/Scenarios)" }

    # 1. Wipe every possible scenario item (union of all scenarios' top-level names + the plain-name
    #    aliases from earlier mistakes) so no previous scenario's data bleeds through. Base AO data and
    #    user data (Saved Games/Quick Saves/Recordings/Screenshots/Image Cache/logs) are NOT in this set.
    $wipe = @('Map','Shapes','Sounds','Images','Physics Model')   # legacy plain-name aliases (cleanup)
    Get-ChildItem -LiteralPath $scenRoot -Directory | ForEach-Object {
        Get-ChildItem -LiteralPath $_.FullName -Force |
            Where-Object { $_.Name -notin @('.git','.gitignore','Readme.md','README.txt') } |
            ForEach-Object { $wipe += $_.Name }
    }
    $wipe = $wipe | Sort-Object -Unique
    $quoted = ($wipe | ForEach-Object { "'$_'" }) -join ' '
    Write-Host "Wiping previous scenario items"
    Adb shell "cd '$files' && rm -rf $quoted"

    # 2. Copy the target scenario folder AS-IS (original filenames, all subdirs; skip git/readme).
    Write-Host "Copying scenario '$Scenario' as-is"
    Get-ChildItem -LiteralPath $sdir -Force |
        Where-Object { $_.Name -notin @('.git','.gitignore','Readme.md','README.txt') } |
        ForEach-Object { Adb push $_.FullName "$files/" | Out-Null; Write-Host "  + $($_.Name)" }
}

# 3. Relaunch clean (sync so large files are flushed; force-stop for a fresh process).
Adb shell sync
Adb shell am force-stop org.alephone
Adb logcat -c
Adb shell monkey -p org.alephone -c android.intent.category.LAUNCHER 1 | Out-Null
Write-Host "Deployed '$Scenario'. Put the headset on to run (immersive app needs focus to start SDL_main)."
