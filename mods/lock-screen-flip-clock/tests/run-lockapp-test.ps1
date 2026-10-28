# Builds the lock screen test DLL (tests\lockapp-test.cpp) and the injector,
# and copies the DLL into the target app's AC\Temp folder, which its sandbox
# can read. Prints the folder; see lockapp-test.cpp for the files it reads and
# writes there.
#
#   .\run-lockapp-test.ps1                     # for LockApp
#   .\run-lockapp-test.ps1 -Package Microsoft.WindowsCalculator_8wekyb3d8bbwe -Process CalculatorApp.exe -Load
param(
    [string]$Package = 'Microsoft.LockApp_cw5n1h2txyewy',
    [string]$Process = 'LockApp.exe',
    [switch]$Load
)
$ErrorActionPreference = 'Stop'
$clang = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$bin = Join-Path $env:TEMP 'lsfc-preview-bin'
New-Item -ItemType Directory -Force $bin | Out-Null
$dir = Join-Path $env:LOCALAPPDATA "Packages\$Package\AC\Temp"
if (-not (Test-Path $dir)) { throw "no such folder: $dir" }

$common = @('-std=c++23', '-target', 'x86_64-w64-mingw32', '-O1', '-DUNICODE', '-D_UNICODE',
    '-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00', '-DNTDDI_VERSION=0x0A000008')
& $clang @common '-static' (Join-Path $PSScriptRoot 'inject.cpp') -o (Join-Path $bin 'inject.exe') '-municode'
if ($LASTEXITCODE) { throw 'injector build failed' }

# A new name for every build: an older copy may still be loaded.
$name = "lsfc-test-$(Get-Date -Format HHmmss).dll"
& $clang @common '-shared' '-static' '-DWH_MOD' '-DWH_EDITING' '-include' 'windhawk_api.h' `
    (Join-Path $PSScriptRoot 'lockapp-test.cpp') -o (Join-Path $bin $name) `
    -ld2d1 -ld3d11 -ldxgi -lole32 -loleaut32 -lpowrprof -lruntimeobject -lxaudio2_9 -lwindowscodecs
if ($LASTEXITCODE) { throw 'test DLL build failed' }
Copy-Item (Join-Path $bin $name) (Join-Path $dir $name)
Write-Host "dll:    $(Join-Path $dir $name)"
Write-Host "folder: $dir"
if ($Load) { & (Join-Path $bin 'inject.exe') $Process (Join-Path $dir $name) }
