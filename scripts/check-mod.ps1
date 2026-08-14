# Type-check a Windhawk mod with Windhawk's own compiler.
#
# Windhawk bundles clang 20 + mingw-w64 at C:\Program Files\Windhawk\Compiler and drives it
# with the flags in compile_flags.txt - the same ones clangd uses for the squiggles in the
# Windhawk editor. Pointing that compiler at a mod source reproduces those diagnostics
# offline, so a mod can be verified without pasting it into the editor first.
#
# ponytail: -fsyntax-only, not a real build. Producing a loadable .dll would mean
# replicating Windhawk's link step and its per-mod @compilerOptions handling, and every
# error worth catching before a paste is a front-end error anyway. Add -c and a link line
# if a codegen bug ever actually bites.
#
#   .\scripts\check-mod.ps1                          # all mods under mods\
#   .\scripts\check-mod.ps1 mods\foo\foo.wh.cpp      # one file
#   .\scripts\check-mod.ps1 -Warnings                # include warnings, not just errors

[CmdletBinding()]
param(
    [string[]]$Path,
    [switch]$Warnings,
    [string]$CompilerRoot = 'C:\Program Files\Windhawk\Compiler'
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent

$clang = Join-Path $CompilerRoot 'bin\clang++.exe'
$flagsFile = Join-Path $CompilerRoot 'compile_flags.txt'
if (-not (Test-Path $clang))     { throw "Windhawk compiler not found: $clang" }
if (-not (Test-Path $flagsFile)) { throw "compile_flags.txt not found: $flagsFile" }

# compile_flags.txt is one argument per line. It carries the language, standard, target,
# the Windows version defines and the forced -include of windhawk_api.h.
#
# -DWH_EDITING is dropped, and that matters more than it looks. It is the *editor's*
# configuration, and windhawk_api.h swaps several macros under it - most importantly
# Wh_Log, which is a permissive variadic stub while editing and
#
#     InternalWh_Log_Wrapper(L"[%d:%S]: " message, ...)
#
# in a real build. That concatenates the format onto a literal, so anything that is not
# itself a string literal - a ternary, a variable - is a syntax error when Windhawk
# actually compiles the mod and compiles clean here. That exact case reached CI once.
# WH_MOD_ID / WH_MOD_VERSION are only defined under WH_EDITING, so supply them instead;
# the real values come from the metadata block at build time.
$baseFlags = Get-Content $flagsFile |
             Where-Object { $_.Trim() -ne '' -and $_.Trim() -ne '-DWH_EDITING' }
$baseFlags += '-I'
$baseFlags += (Join-Path $CompilerRoot 'include')
$baseFlags += '-DWH_MOD_ID=L"check-mod"'
$baseFlags += '-DWH_MOD_VERSION=L"0.0"'
$baseFlags += '-fsyntax-only'
$baseFlags += '-fno-caret-diagnostics'
# The editor's clangd surfaces -Wall diagnostics (that is where -Wsign-compare comes
# from), so ask for the same set or this would miss what the user sees on screen.
$baseFlags += '-Wall'

if (-not $Path) {
    $Path = Get-ChildItem (Join-Path $repo 'mods') -Recurse -Filter '*.wh.cpp' |
            Select-Object -ExpandProperty FullName
}

$failed = 0
foreach ($file in $Path) {
    if (-not [System.IO.Path]::IsPathRooted($file)) { $file = Join-Path $repo $file }
    $name = Split-Path $file -Leaf

    # A mod declares its own extra flags in the metadata block, e.g.
    #   // @compilerOptions -lole32 -loleaut32 -lruntimeobject
    # Library flags are inert under -fsyntax-only but -D/-I/-std ones are not, so pass
    # them all through and let clang ignore what it does not need.
    # -l flags are dropped: nothing is linked under -fsyntax-only, and leaving them in
    # buries the real diagnostics under one "linker input unused" warning per library.
    $modFlags = @()
    $optLine = Select-String -Path $file -Pattern '^//\s*@compilerOptions\s+(.+)$' |
               Select-Object -First 1
    if ($optLine) {
        # @() matters: a lone surviving flag would otherwise collapse to a bare string,
        # and splatting a string spreads it one character per argument.
        $modFlags = @($optLine.Matches.Groups[1].Value -split '\s+' |
                      Where-Object { $_ -and $_ -notmatch '^-l' })
    }

    Write-Host "== $name" -ForegroundColor Cyan

    # The gallery's CI builds every architecture the mod declares, so check the
    # same set rather than only the target compile_flags.txt happens to name -
    # a pointer-width or calling-convention problem shows up on exactly one.
    # No @architecture line means "all of them", which is Windhawk's own rule;
    # declaring x86-64 is how a 64-bit-only taskbar mod opts out of x86.
    $archMap = @{ 'x86' = 'i686-w64-mingw32'
                  'x86-64' = 'x86_64-w64-mingw32'
                  'amd64' = 'x86_64-w64-mingw32'
                  'arm64' = 'aarch64-w64-mingw32' }
    $declared = @(Select-String -Path $file -Pattern '^//\s*@architecture\s+(\S+)' |
                  ForEach-Object { $archMap[$_.Matches.Groups[1].Value] } |
                  Where-Object { $_ })
    if (-not $declared) {
        $declared = @('i686-w64-mingw32', 'x86_64-w64-mingw32',
                      'aarch64-w64-mingw32')
    }

    $out = ''
    $clangExit = 0
    foreach ($target in $declared) {
        $targetFlags = @($baseFlags | Where-Object { $_ -ne '-target' -and
                                                     $_ -notmatch '-w64-mingw32$' })
        $targetFlags += '-target'
        $targetFlags += $target
        $o = & $clang @targetFlags @modFlags $file 2>&1 | Out-String
        if ($LASTEXITCODE -ne 0) { $clangExit = $LASTEXITCODE }
        if ($o.Trim()) { $out += "[$target]`n$o" }
    }

    # Not $errors/$warnings: PowerShell matches variable names case-insensitively, so
    # those would collide with $Error and with the -Warnings switch parameter.
    # Count from the exit code, not the text: driver errors ("no such file or
    # directory") carry no line:col, so a regex over the output silently misses them.
    $errCount  = ([regex]::Matches($out, '(?m)error:')).Count
    $warnCount = ([regex]::Matches($out, '(?m)warning:')).Count
    if ($clangExit -ne 0 -and $errCount -eq 0) { $errCount = 1 }

    if ($out.Trim()) {
        $out -split "`n" | Where-Object {
            $_ -match 'error:' -or ($Warnings -and $_ -match 'warning:')
        } | ForEach-Object { "   $($_.Trim())" }
    }

    if ($errCount -gt 0) {
        Write-Host "   $errCount error(s), $warnCount warning(s)" -ForegroundColor Red
        $failed++
    } else {
        Write-Host "   clean ($warnCount warning(s))" -ForegroundColor Green
    }
}

if ($failed) { Write-Host "`n$failed mod(s) failed to compile." -ForegroundColor Red; exit 1 }
Write-Host "`nAll mods type-check." -ForegroundColor Green
