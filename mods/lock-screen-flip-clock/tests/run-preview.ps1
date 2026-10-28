# Builds tests\preview.cpp with Windhawk's own clang (the mod is compiled inside
# it) and runs it. See the top of preview.cpp for the modes.
#
#   .\run-preview.ps1 check  $env:TEMP\lsfc-check
#   .\run-preview.ps1 art    $env:TEMP\lsfc-art
#   .\run-preview.ps1 frames $env:TEMP\lsfc-frames start=10:59:58 secs=4 anim=gravity
#   .\run-preview.ps1 live   secs=30
$ErrorActionPreference = 'Stop'
$clang = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$out = Join-Path $env:TEMP 'lsfc-preview-bin'
New-Item -ItemType Directory -Force $out | Out-Null
$exe = Join-Path $out 'preview.exe'

$flags = @('-std=c++23', '-target', 'x86_64-w64-mingw32', '-static', '-O1',
    '-DUNICODE', '-D_UNICODE', '-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00',
    '-DNTDDI_VERSION=0x0A000008', '-DWH_MOD', '-DWH_EDITING',
    '-include', 'windhawk_api.h', '-Wall', '-Wno-unused-parameter', '-Wno-unused-function',
    (Join-Path $PSScriptRoot 'preview.cpp'), '-o', $exe,
    '-ld2d1', '-ld3d11', '-ldxgi', '-lole32', '-loleaut32', '-lpowrprof', '-lruntimeobject',
    '-lxaudio2_9', '-lwindowscodecs', '-luser32', '-lgdi32')
& $clang @flags
if ($LASTEXITCODE) { throw "build failed ($LASTEXITCODE)" }
# No embedded manifest, so Windows reads this one from beside the exe.
Copy-Item (Join-Path $PSScriptRoot 'preview.exe.manifest') "$exe.manifest" -Force

& $exe @args
exit $LASTEXITCODE
