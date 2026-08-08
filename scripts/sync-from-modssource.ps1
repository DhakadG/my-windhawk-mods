# Pull the live Windhawk sources into the repo so `git diff` shows what actually changed.
# Windhawk compiles from C:\ProgramData\Windhawk\ModsSource (admin-only for writes), so that
# directory is the source of truth and this repo is the tracked mirror of it.
#
# ponytail: a plain copy loop, not a two-way sync. Edits flow ModsSource -> repo only;
# to go the other way, paste into the Windhawk editor and compile there.

param([string]$ModsSource = 'C:\ProgramData\Windhawk\ModsSource')

$repo = Split-Path $PSScriptRoot -Parent

# repo mod folder  ->  file name in ModsSource
$mods = @{
    'win-x-hotcorners'                = 'local@win-x-hotcorners.wh.cpp'
    'taskbar-ai-quota-fork'           = 'local@taskbar-ai-quota-fork.wh.cpp'
    'taskbar-clock-customization-v3'  = 'local@taskbar-clock-customization-v3.wh.cpp'
    'mac-magnifying-cursor'           = 'local@mac-magnifying-cursor.wh.cpp'
}

foreach ($name in $mods.Keys | Sort-Object) {
    $from = Join-Path $ModsSource $mods[$name]
    $to   = Join-Path $repo "mods\$name\$name.wh.cpp"

    if (-not (Test-Path $from)) { Write-Warning "not installed, skipped: $($mods[$name])"; continue }

    $version = (Select-String -Path $from -Pattern '^// @version\s+(.+)$' |
                Select-Object -First 1).Matches.Groups[1].Value

    if ((Test-Path $to) -and (Get-FileHash $from).Hash -eq (Get-FileHash $to).Hash) {
        Write-Host "  unchanged  $name (v$version)"
        continue
    }
    Copy-Item $from $to -Force
    Write-Host "  updated    $name (v$version)" -ForegroundColor Green
}

Write-Host "`nReview with: git -C `"$repo`" diff --stat"
