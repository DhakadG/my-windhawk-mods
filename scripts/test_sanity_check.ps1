# Regression test for cpp_sanity_check.ps1.
#
# Deliberately reintroduces each bug that has actually broken this mod and
# asserts the checker reports it. A checker nobody has tested is just a
# comforting noise generator.

$chk  = Join-Path $PSScriptRoot 'cpp_sanity_check.ps1'
$good = 'C:\Users\lost_husky\Downloads\Programs\VS Code Works\WindHawk Mods\Win-x-HotCorners.cpp'
$work = Join-Path ([IO.Path]::GetTempPath()) 'sanitycase.cpp'
$src  = [IO.File]::ReadAllText($good)
$fails = 0

function Case($name, $mutated, $expect) {
    [IO.File]::WriteAllText($work, $mutated, (New-Object Text.UTF8Encoding($false)))
    $out = (& $chk $work 2>&1) -join "`n"
    if ($out -match [regex]::Escape($expect)) {
        "  CAUGHT   $name"
    } else {
        $script:fails++
        "  MISSED   $name   (expected text: '$expect')"
    }
}

'--- does the checker catch each bug that actually shipped? ---'

# v3.9.0: String.Replace hit two anchors and duplicated a function
$dup = $src
$m = [regex]::Match($dup, '(?s)static const wchar_t \*ActionIdFromEnum\(CornerAction a\)\s*\{.*?\n\}')
if ($m.Success) { $dup = $dup.Insert($m.Index + $m.Length, "`n`n" + $m.Value) }
Case 'duplicate function definition' $dup 'defined more than once'

# v3.9.0: non-capturing lambda cannot convert to a __stdcall WNDENUMPROC
$lam = $src.Replace(
    'EnumChildWindows(hWnd, DashSetChildFont, (LPARAM)s->hFont);',
    'EnumChildWindows(hWnd, [](HWND c, LPARAM p) -> BOOL { return TRUE; }, (LPARAM)s->hFont);')
Case 'lambda passed to a stdcall callback' $lam 'lambda passed to EnumChildWindows'

# v3.6.0: windef.h #defines near and far as nothing
$nf = $src.Replace('LONG hi, LONG nearSide, LONG farSide', 'LONG hi, LONG near, LONG far')
Case 'identifier shadowed by an SDK macro' $nf 'the Windows SDK #defines it'

# v3.8.0: GDI calls with no -lgdi32
$lib = $src.Replace('-lcomctl32 -lgdi32', '-lcomctl32')
Case 'missing import library' $lib 'gdi32 but -lgdi32 is not in'

# v3.0.0: @architecture stopped the tool mod loading at all
$arch = $src.Replace('// @github          https://github.com/DhakadG',
                     "// @github          https://github.com/DhakadG`n// @architecture    x86-64")
Case '@architecture on a tool mod' $arch '@architecture declared'

# v3.9.0: OpenDashboard called before it was declared
$fwd = $src.Replace("// Defined further down with the dashboard; the tray menu needs it earlier.`nstatic void OpenDashboard();`n`n", '')
Case 'use before declaration' $fwd 'not declared until'

# a settings key read but never declared in the settings block
$set = $src.Replace('Wh_GetIntSetting(L"CooldownMs")', 'Wh_GetIntSetting(L"TotallyMadeUp")')
Case 'setting read but not declared' $set 'settings read but not declared'

# v4.0.2: a numbered dropdown made Windhawk reject the whole settings block
$opt = $src.Replace("- RequireModifier: none", "- RequireModifier: 0")
Case 'numbered $options dropdown' $opt 'must have a string value'

# unbalanced braces
$br = $src.Replace('static void ActionLock() { LockWorkStation(); }',
                   'static void ActionLock() { LockWorkStation();')
Case 'unbalanced braces' $br 'brace mismatch'

''
# and the real file must still be clean
[IO.File]::WriteAllText($work, $src, (New-Object Text.UTF8Encoding($false)))
$out = (& $chk $work 2>&1) -join "`n"
if ($out -match 'SANITY CHECKS PASSED') { '  CLEAN    the real file reports no problems' }
else { $script:fails++; '  ** the real file reports problems - false positive **'; $out }

Remove-Item -LiteralPath $work -Force -ErrorAction SilentlyContinue
''
if ($fails -eq 0) { 'CHECKER VERIFIED: catches all 9 known failure modes, no false positives' }
else { "$fails CHECKER GAP(S)" }
exit $(if ($fails -eq 0) { 0 } else { 1 })
