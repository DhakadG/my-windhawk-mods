# Pre-compile sanity checks for a single-file Windhawk mod.
#
# There is no C++ toolchain on this machine, so these catch the classes of
# error that have actually bitten this file, generically rather than by
# hand-listing symbols:
#
#   1. duplicate function definitions      (a Replace() hitting 2 anchors)
#   2. use before declaration              (any static function)
#   3. lambdas passed to __stdcall callbacks
#   4. identifiers shadowed by SDK macros  (near/far/min/max/...)
#   5. Win32 APIs whose import library is not linked
#   6. brace / paren balance
#   7. @version present, settings keys read
#
# Usage: cpp_sanity_check.ps1 [path-to-mod.cpp]
# Defaults to the tracked mod file; pass a path to check the working copy
# before it has been copied over.

param([string]$Path = (Join-Path $PSScriptRoot '..\win-x-hotcorners.wh.cpp'))

$src   = [IO.File]::ReadAllText($Path)
$lines = [IO.File]::ReadAllLines($Path)
$fails = 0
function Fail($m){ $script:fails++; "  FAIL  $m" }
function Pass($m){ "  PASS  $m" }

# Strip comments and string literals so scans don't trip on prose or the
# embedded readme/settings blocks.
$code = [regex]::Replace($src, '/\*.*?\*/', { "`n" * ($args[0].Value -split "`n").Count }, 'Singleline')
$code = [regex]::Replace($code, '//[^\n]*', '')
$code = [regex]::Replace($code, 'L?"(\\.|[^"\\])*"', '""')

"=== 1. duplicate definitions ==="
# a definition is  <ret> Name(args)  followed by { at the end of the line
$defs = @{}
foreach ($m in [regex]::Matches($code, '(?m)^\s*static\s+[^;()\n=]*?(\w+)\s*\([^;]*?\)\s*(?:const\s*)?\{')) {
    $n = $m.Groups[1].Value
    if ($n -in @('if','for','while','switch','catch','return','sizeof','else','void','const','ARRAYSIZE','_countof','decltype','constexpr','inline')) { continue }
    $ln = ($code.Substring(0, $m.Index) -split "`n").Count
    if (-not $defs.ContainsKey($n)) { $defs[$n] = @() }
    $defs[$n] += $ln
}
$dupes = $defs.GetEnumerator() | Where-Object { $_.Value.Count -gt 1 }
if ($dupes) {
    foreach ($d in $dupes) {
        # overloads are legal; flag only when the parameter lists match
        $sigs = @()
        foreach ($ln in $d.Value) { $sigs += ($lines[$ln-1] -replace '\s+',' ').Trim() }
        if (($sigs | Select-Object -Unique).Count -lt $sigs.Count) {
            Fail "'$($d.Key)' defined more than once with the same signature at lines $($d.Value -join ', ')"
        }
    }
}
if ($script:fails -eq 0) { Pass "no duplicate definitions ($($defs.Count) functions)" }

"`n=== 2. use before declaration ==="
$declLine = @{}
foreach ($n in $defs.Keys) { $declLine[$n] = ($defs[$n] | Measure-Object -Minimum).Minimum }
# forward declarations count as a declaration
foreach ($m in [regex]::Matches($code, '(?m)^\s*static\s+[^;()\n=]*?(\w+)\s*\([^;{]*\)\s*;')) {
    $n = $m.Groups[1].Value
    if ($n -in @('if','for','while','switch','catch','return','sizeof','else','void','const','ARRAYSIZE','_countof','decltype','constexpr','inline')) { continue }
    $ln = ($code.Substring(0, $m.Index) -split "`n").Count
    if (-not $declLine.ContainsKey($n) -or $ln -lt $declLine[$n]) { $declLine[$n] = $ln }
}
$bad = 0
foreach ($n in $declLine.Keys) {
    if ($n.Length -lt 4) { continue }
    foreach ($m in [regex]::Matches($code, "(?<![\w:])$([regex]::Escape($n))\s*\(")) {
        $ln = ($code.Substring(0, $m.Index) -split "`n").Count
        if ($ln -lt $declLine[$n]) {
            Fail "'$n' used at line $ln but not declared until $($declLine[$n])"
            $bad++
            break
        }
    }
}
if ($bad -eq 0) { Pass "every function is declared before its first use" }

"`n=== 3. lambdas passed to __stdcall callbacks ==="
# these all take a CALLBACK/WINAPI function pointer; a non-capturing lambda
# decays to cdecl and will not convert in a 32-bit build
$cbApis = 'EnumChildWindows|EnumWindows|EnumDisplayMonitors|EnumThreadWindows|EnumFontFamiliesEx|SetWindowsHookEx|SetWinEventHook|EnumResourceNames|EnumPropsEx|GrayString|SetTimer'
$lam = 0
foreach ($m in [regex]::Matches($code, "($cbApis)\s*\(", 'Singleline')) {
    # walk the call's own argument list, balanced, so the scan cannot spill
    # into the following statement
    $i = $code.IndexOf('(', $m.Index); $depth = 0; $end = $i
    for ($j = $i; $j -lt [Math]::Min($code.Length, $i + 4000); $j++) {
        if ($code[$j] -eq '(') { $depth++ }
        elseif ($code[$j] -eq ')') { $depth--; if ($depth -eq 0) { $end = $j; break } }
    }
    $tail = $code.Substring($i, $end - $i + 1)
    if ($tail -match '\[\s*\]\s*\(') {
        $ln = ($code.Substring(0, $m.Index) -split "`n").Count
        Fail "lambda passed to $($m.Groups[1].Value) at line $ln - needs a free function marked CALLBACK"
        $lam++
    }
}
if ($lam -eq 0) { Pass "no lambdas passed where a __stdcall callback is required" }

"`n=== 4. identifiers shadowed by SDK macros ==="
$reserved = 'near','far','min','max','small','IN','OUT','interface','CONST','pascal','cdecl','huge','VOID','TRUE','FALSE','NULL'
$sh = 0
foreach ($r in $reserved) {
    foreach ($m in [regex]::Matches($code, "(?<![\w:])(?:int|LONG|DWORD|bool|auto|UINT|WORD|HWND|RECT|HICON|HMENU|float|double|size_t|BOOL|LPARAM|WPARAM)\s+$r\b")) {
        $ln = ($code.Substring(0, $m.Index) -split "`n").Count
        Fail "'$r' used as an identifier at line $ln - the Windows SDK #defines it"
        $sh++
    }
}
if ($sh -eq 0) { Pass "no identifiers collide with Windows SDK macros" }

"`n=== 5. APIs vs linked import libraries ==="
$libLine = ([regex]::Match($src, '@compilerOptions\s+(.+)')).Groups[1].Value
$libs = ($libLine -split '\s+' | Where-Object { $_ -like '-l*' }) -replace '^-l',''
$apiLib = @{
  'gdi32'   = 'CreateDIBSection|CreateBitmap|CreateSolidBrush|CreateFontIndirectW?|DeleteObject|SelectObject|CreateCompatibleDC|DeleteDC|SetTextColor|SetBkColor|GetStockObject|CreatePen|Rectangle|BitBlt|GetObjectW?'
  'shell32' = 'Shell_NotifyIconW?|ShellExecuteExW?|SHQueryUserNotificationState|CommandLineToArgvW|Shell_NotifyIconGetRect|ExtractIconW?'
  'powrprof'= 'SetSuspendState|PowerSetActiveScheme'
  'comctl32'= 'SetWindowSubclass|RemoveWindowSubclass|DefSubclassProc|InitCommonControls'
  'ole32'   = 'CoInitialize|CoCreateInstance|CoTaskMemFree'
  'advapi32'= 'RegOpenKeyExW?|RegQueryValueExW?|RegSetValueExW?|OpenProcessToken'
  'dwmapi'  = 'DwmSetWindowAttribute|DwmExtendFrameIntoClientArea'
  'uxtheme' = 'SetWindowTheme|OpenThemeData'
}
foreach ($k in $apiLib.Keys) {
    $hits = [regex]::Matches($code, "(?<![\w:])($($apiLib[$k]))\s*\(")
    # ignore ones resolved dynamically through GetProcAddress
    $direct = 0
    foreach ($h in $hits) {
        $ln = ($code.Substring(0, $h.Index) -split "`n").Count
        $ctx = ($lines[[Math]::Max(0,$ln-6)..([Math]::Min($lines.Count-1,$ln))] -join ' ')
        if ($ctx -notmatch 'GetProcAddress') { $direct++ }
    }
    if ($direct -gt 0 -and $libs -notcontains $k) {
        Fail "$direct direct call(s) into $k but -l$k is not in @compilerOptions"
    } elseif ($direct -gt 0) {
        Pass "$k linked ($direct direct call$(if($direct -ne 1){'s'}))"
    }
}

"`n=== 6. balance ==="
$ob=([regex]::Matches($code,'\{')).Count; $cb=([regex]::Matches($code,'\}')).Count
if ($ob -eq $cb) { Pass "braces balanced ($ob)" } else { Fail "brace mismatch: $ob open, $cb close" }
$op=([regex]::Matches($code,'\(')).Count; $cp=([regex]::Matches($code,'\)')).Count
if ($op -eq $cp) { Pass "parens balanced ($op)" } else { Fail "paren mismatch: $op open, $cp close" }

"`n=== 7. mod metadata ==="
if ($src -match '@version\s+\S+') { Pass "@version present: $(([regex]::Match($src,'@version\s+(\S+)')).Groups[1].Value)" } else { Fail "no @version" }
$meta = ([regex]::Match($src, '(?s)==WindhawkMod==(.*?)==/WindhawkMod==')).Groups[1].Value
if ($meta -match '(?m)^//\s*@architecture') { Fail "@architecture declared in the metadata block - a windhawk.exe tool mod must not restrict architecture" } else { Pass "no @architecture restriction in metadata" }
# every setting read must exist in the settings block, and vice versa
$declared = [regex]::Matches($src, '(?m)^-\s*(\w+):') | ForEach-Object { $_.Groups[1].Value }
$readNames = @(
    [regex]::Matches($src, 'Wh_Get(?:Int|String)Setting\(L"(\w+)"')
    [regex]::Matches($src, 'StringSetting::make\(L"(\w+)"')
) | ForEach-Object { $_.Groups[1].Value } | Select-Object -Unique
$missing = $readNames | Where-Object { $_ -notin $declared }
if ($missing) { Fail "settings read but not declared: $($missing -join ', ')" } else { Pass "all $($readNames.Count) settings read are declared" }

"`n=== 8. settings dropdowns ==="
# Windhawk refuses to parse the entire settings block if a value carrying
# $options is not a string, and the mod then never loads at all. Cost a CI
# round trip in v4.0.2, where RequireModifier was a number with numbered
# options.
$sLines = ([regex]::Match($src, '(?s)==WindhawkModSettings==.*?/\*(.*?)\*/')).Groups[1].Value -split "`r?`n"
$badOpts = @()
for ($i = 0; $i -lt $sLines.Count; $i++) {
    if ($sLines[$i] -notmatch '^\s*\$options:') { continue }
    # the setting this list belongs to is the nearest key line above it
    for ($j = $i - 1; $j -ge 0; $j--) {
        if ($sLines[$j] -match '^[\s-]*(\w+):\s*(\S.*?)?\s*$') {
            if ($matches[2] -match '^(-?\d+(\.\d+)?|true|false)$') {
                $badOpts += "$($matches[1]) = $($matches[2])"
            }
            break
        }
    }
}
if ($badOpts) { Fail "settings using `$options must have a string value: $($badOpts -join ', ')" }
else { Pass "every `$options list belongs to a string setting" }

"`n$(if($fails -eq 0){'SANITY CHECKS PASSED'}else{"$fails PROBLEM(S) FOUND"})"
exit $(if($fails -eq 0){0}else{1})
