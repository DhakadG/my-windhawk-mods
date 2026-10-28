# Run DarkionAvey's offscreen regression suite against explorer-justified-view.
#
#   git clone https://github.com/DarkionAvey/windhawk-gpu-accelerated-explorer-view <dir>
#   pwsh -File scripts/run-upstream-suite.ps1 -Upstream <dir> [-TestArguments --selection]
#
# His suite includes the mod source directly, so it runs on ours with a few edits,
# made in the clone (restored first, so reruns don't stack; a changed anchor fails
# instead of guessing):
#  - options-smoke.h pins the exact settings schema: taught the settings we add.
#  - main() starts with our layout features off (justified rows, tight photo grid,
#    preloading, Photos-style motion: his tests are timed to 180 ms ease-out and
#    expect his view-centre zoom anchor). His
#    suite then checks that his paths still behave as his; those features are
#    ours to test.
#  - icon-sizing-smoke.h expects plain icons magnified to fill any slot (his
#    0.9.257). This mod caps that at 2x the artwork by choice; the check keeps its
#    aspect and fill assertions up to that cap.
#
# TEMP must be the long path: with the 8.3 form (C:\Users\LOST_H~1\...) his
# PlainFilesystemProvider compare fails and the Office-lock test fails on his own
# release too.

param(
    [Parameter(Mandatory)][string]$Upstream,
    [string]$Source = (Join-Path (Split-Path $PSScriptRoot -Parent) 'mods\explorer-justified-view\explorer-justified-view.wh.cpp'),
    [string[]]$TestArguments = @()
)
$ErrorActionPreference = 'Stop'

$off = ' g_photoGridBelow96=0; g_justifiedGrid=false; g_preloadThumbnails=false; g_photoMotion=false;'
# file  ->  (text in his source  ->  what replaces it)
$patches = [ordered]@{
    'tests\options-smoke.h' = [ordered]@{
        '{L"Hover.hoverSelectedOpacity",100,0,100,&g_bothAlpha},' =
            '{L"Hover.hoverSelectedOpacity",100,0,100,&g_bothAlpha},{L"Appearance.photoGridBelow",64,0,512,&g_photoGridBelow96},'
        '{L"enableLogging",false,&g_perf},' =
            '{L"enableLogging",false,&g_perf},{L"Appearance.preloadThumbnails",true,&g_preloadThumbnails},'
        '{L"Columns.folderPlacement",L"foldersBottom"}' =
            '{L"Columns.folderPlacement",L"foldersBottom"},{L"Appearance.gridLayout",L"justified"}'
    }
    # Again after the options test: it reloads every setting from the schema
    # defaults and restores only numbers and booleans, not gridLayout.
    'tests\background-regression.cpp' = [ordered]@{
        'int wmain(int argc,wchar_t** argv) {' = 'int wmain(int argc,wchar_t** argv) {' + $off
        'TestUserOptions();' = 'TestUserOptions();' + $off
    }
    'tests\icon-sizing-smoke.h' = [ordered]@{
        'const float expectedHeight=side*record->pixels.height/record->pixels.width;' =
            'const float capped=std::min(side,2.f*record->pixels.width); const float expectedHeight=capped*record->pixels.height/record->pixels.width;'
        'Check(abs(float(right-left)-side)<=2 && abs(float(bottom-top)-expectedHeight)<=2,' =
            'Check(abs(float(right-left)-capped)<=2 && abs(float(bottom-top)-expectedHeight)<=2,'
    }
}
foreach ($file in $patches.Keys) {
    git -C $Upstream checkout -- ($file -replace '\\', '/')
    $path = Join-Path $Upstream $file
    $text = [IO.File]::ReadAllText($path)
    foreach ($from in $patches[$file].Keys) {
        if (-not $text.Contains($from)) { throw "$file changed shape; text not found: $from" }
        $text = $text.Replace($from, $patches[$file][$from])
    }
    [IO.File]::WriteAllText($path, $text)
}

$env:TEMP = $env:TMP = [IO.Path]::Combine($env:USERPROFILE, 'AppData\Local\Temp')
$arguments = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $Upstream 'tests\run-regressions.ps1'),
    '-SourceFile', (Resolve-Path $Source).Path)
if ($TestArguments) { $arguments += @('-TestArguments') + $TestArguments }
powershell @arguments
exit $LASTEXITCODE
