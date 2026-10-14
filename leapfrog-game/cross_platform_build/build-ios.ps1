# Leapfrog iOS build - run on Windows. Copies build-ios.sh to the Mac and runs it.
# Examples:
#   .\build-ios.ps1                      # pull + Debug build
#   .\build-ios.ps1 -Install             # ...and install + launch on the iPhone
#   .\build-ios.ps1 -Configuration Release -NoPull
param(
    [ValidateSet("Debug", "Release")][string]$Configuration = "Release",
    [switch]$Install,
    [switch]$NoPull,
    [string]$Device = "",
    [string]$Scheme = "Leapfrog_ios_debug"
)

$here = Split-Path -Parent $MyInvocation.MyCommand.Path

scp -q "$here\build-ios.sh" mac:build-ios.sh
if ($LASTEXITCODE -ne 0) { Write-Error "Could not copy build-ios.sh to the Mac"; exit 1 }

$pull = if ($NoPull) { "nopull" } else { "pull" }
$inst = if ($Install) { "install" } else { "noinstall" }

ssh -t mac "bash ~/build-ios.sh $Configuration $pull $inst '$Device' '$Scheme'"
exit $LASTEXITCODE
