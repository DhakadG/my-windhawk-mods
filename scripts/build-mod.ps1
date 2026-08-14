# Really build a Windhawk mod: compile AND link, for every architecture.
#
# check-mod.ps1 is -fsyntax-only, so it cannot see a link error. That is not
# hypothetical - `CLSID_Shell` needed -luuid, compiled clean locally, and broke
# the gallery's compatibility check on all three targets.
#
# This mirrors what the gallery's scripts/compile_mod.py does, using the local
# Windhawk installation's compiler and engine import libraries:
#
#   clang++ -O2 -shared -DWH_MOD -DWH_MOD_ID=... <engine>\windhawk.lib
#           -include windhawk_api.h -target <arch> -Wl,--export-all-symbols
#           -o out.dll mod.wh.cpp <@compilerOptions>
#
# Nothing is installed and the running Windhawk is untouched; the DLLs go to a
# temp directory and are deleted.
#
#   .\scripts\build-mod.ps1                        # every mod under mods\
#   .\scripts\build-mod.ps1 mods\foo\foo.wh.cpp    # one file

[CmdletBinding()]
param(
    [string[]]$Path,
    [string]$WindhawkRoot = 'C:\Program Files\Windhawk',
    # Older Windhawk releases ship an older clang. Drop to c++20 for those.
    [string]$Std = 'c++23'
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent

$clang = Join-Path $WindhawkRoot 'Compiler\bin\clang++.exe'
if (-not (Test-Path $clang)) { throw "Windhawk compiler not found: $clang" }

# The engine directory is versioned (Engine\2.0_1\...), so find it rather than
# hardcoding a version that will move.
$engineRoot = Get-ChildItem (Join-Path $WindhawkRoot 'Engine') -Directory |
              Sort-Object Name -Descending | Select-Object -First 1
if (-not $engineRoot) { throw 'No Windhawk engine directory found' }

$targets = @(
    @{ Arch = 'x86';    Dir = '32';    Target = 'i686-w64-mingw32' }
    @{ Arch = 'x86-64'; Dir = '64';    Target = 'x86_64-w64-mingw32' }
    @{ Arch = 'arm64';  Dir = 'arm64'; Target = 'aarch64-w64-mingw32' }
)

if (-not $Path) {
    $Path = Get-ChildItem (Join-Path $repo 'mods') -Recurse -Filter '*.wh.cpp' |
            Select-Object -ExpandProperty FullName
}

$outDir = Join-Path $env:TEMP "wh-build-$PID"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$failed = 0
try {
    foreach ($file in $Path) {
        if (-not [System.IO.Path]::IsPathRooted($file)) { $file = Join-Path $repo $file }
        $name = Split-Path $file -Leaf
        Write-Host "== $name" -ForegroundColor Cyan

        $meta = @{}
        foreach ($key in 'id', 'version', 'compilerOptions', 'architecture') {
            $meta[$key] = @(Select-String -Path $file -Pattern "^//\s*@$key\s+(.+)$" |
                            ForEach-Object { $_.Matches.Groups[1].Value.Trim() })
        }

        # No @architecture means every one, which is Windhawk's own rule.
        $wanted = $targets
        if ($meta['architecture'].Count) {
            $wanted = $targets | Where-Object { $meta['architecture'] -contains $_.Arch }
        }

        # @() so a single surviving flag is not splatted one character per arg.
        $modFlags = @()
        if ($meta['compilerOptions'].Count) {
            $modFlags = @($meta['compilerOptions'][0] -split '\s+' | Where-Object { $_ })
        }

        $modId = if ($meta['id'].Count) { $meta['id'][0] } else { 'mod' }
        $modVer = if ($meta['version'].Count) { $meta['version'][0] } else { '0.0' }

        $modFailed = $false
        foreach ($t in $wanted) {
            $lib = Join-Path $engineRoot.FullName "$($t.Dir)\windhawk.lib"
            if (-not (Test-Path $lib)) {
                Write-Host "   $($t.Arch): engine lib missing, skipped" -ForegroundColor Yellow
                continue
            }

            $args = @(
                "-std=$Std", '-O2', '-shared',
                '-DUNICODE', '-D_UNICODE',
                '-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00',
                '-D_WIN32_IE=0x0A00', '-DNTDDI_VERSION=0x0A000008',
                '-D__USE_MINGW_ANSI_STDIO=0', '-DWH_MOD',
                "-DWH_MOD_ID=L`"$modId`"", "-DWH_MOD_VERSION=L`"$modVer`"",
                $lib,
                '-I', (Join-Path $WindhawkRoot 'Compiler\include'),
                '-x', 'c++', $file,
                '-include', 'windhawk_api.h',
                '-target', $t.Target,
                '-Wl,--export-all-symbols',
                '-o', (Join-Path $outDir "$modId-$($t.Arch).dll")
            ) + $modFlags

            $out = & $clang @args 2>&1 | Out-String
            if ($LASTEXITCODE -ne 0) {
                $modFailed = $true
                Write-Host "   $($t.Arch): FAILED" -ForegroundColor Red
                $out -split "`n" | Where-Object { $_ -match 'error' } |
                    Select-Object -First 8 | ForEach-Object { "      $($_.Trim())" }
            } else {
                Write-Host "   $($t.Arch): linked" -ForegroundColor Green
            }
        }
        if ($modFailed) { $failed++ }
    }
} finally {
    Remove-Item $outDir -Recurse -Force -ErrorAction SilentlyContinue
}

if ($failed) { Write-Host "`n$failed mod(s) failed to build." -ForegroundColor Red; exit 1 }
Write-Host "`nAll mods build and link." -ForegroundColor Green
