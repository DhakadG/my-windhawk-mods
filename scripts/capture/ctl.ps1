# Mouse/window control for the demo captures.
#
# Physical pixels throughout. The process is made Per-Monitor-V2 DPI aware so
# SetCursorPos coordinates match the physical 3840x2160 capture surface.
#
# This script is safe to dot-source repeatedly: native types are only added if
# they do not already exist.

Set-StrictMode -Version Latest

$loggerPath = Join-Path $PSScriptRoot 'logger.ps1'
if ((Get-Command Write-Log -ErrorAction SilentlyContinue) -eq $null -and (Test-Path -LiteralPath $loggerPath)) {
    . $loggerPath
}

if (-not ('Ctl' -as [type])) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;

[StructLayout(LayoutKind.Sequential)]
public struct WPOINT { public int X; public int Y; }

public static class Ctl {
    [DllImport("user32.dll", SetLastError=true)] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll", SetLastError=true)] public static extern bool GetCursorPos(out WPOINT p);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
    [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll", SetLastError=true)] public static extern IntPtr SetProcessDpiAwarenessContext(IntPtr ctx);
    [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowTextW(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr p);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassNameW(IntPtr h, StringBuilder s, int n);
    public delegate bool EnumProc(IntPtr h, IntPtr p);
}
'@
}

try {
    [void][Ctl]::SetProcessDpiAwarenessContext([IntPtr](-4))
    Write-Log -Level DEBUG -Event 'DPI_AWARENESS' -Message 'Per-monitor V2 DPI awareness requested.'
}
catch {
    try {
        [void][Ctl]::SetProcessDPIAware()
        Write-Log -Level WARN -Event 'DPI_AWARENESS_FALLBACK' -Message 'Per-monitor V2 unavailable; system DPI awareness enabled.'
    }
    catch {
        Write-LogException -Event 'DPI_AWARENESS_FAILED' -Exception $_.Exception
    }
}

if (-not ('Mouse' -as [type])) {
    Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class Mouse {
    [DllImport("user32.dll")] public static extern void mouse_event(uint flags, int dx, int dy, uint data, UIntPtr extra);
}
"@
}

if (-not ('Win' -as [type])) {
    Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class Win {
    [DllImport("user32.dll", SetLastError=true)] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
    [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr h);
}
"@
}

$script:DemoScreenW = [Ctl]::GetSystemMetrics(0)
$script:DemoScreenH = [Ctl]::GetSystemMetrics(1)

function global:Get-ScreenSize {
    [pscustomobject]@{ W = $script:DemoScreenW; H = $script:DemoScreenH }
}

function global:Get-MousePos {
    $p = New-Object WPOINT
    if (-not [Ctl]::GetCursorPos([ref]$p)) {
        throw "GetCursorPos failed with Win32 error $([Runtime.InteropServices.Marshal]::GetLastWin32Error())."
    }
    return $p
}

function global:Move-MouseTo {
    param(
        [Parameter(Mandatory)][int]$X,
        [Parameter(Mandatory)][int]$Y,
        [int]$Ms = 500
    )

    if ($Ms -lt 0) { throw 'Move duration cannot be negative.' }
    if ($X -lt 0 -or $Y -lt 0 -or $X -ge $script:DemoScreenW -or $Y -ge $script:DemoScreenH) {
        throw "Mouse target ($X,$Y) is outside the detected primary screen ${script:DemoScreenW}x${script:DemoScreenH}."
    }

    $from = Get-MousePos

    Write-Log -Level INFO -Event 'MOUSE_MOVE' -Message "($($from.X),$($from.Y)) -> ($X,$Y)" -Data @{
        fromX = $from.X; fromY = $from.Y; toX = $X; toY = $Y; durationMs = $Ms
    }

    # Position is derived from the elapsed clock, not from a step counter. A
    # counted loop of "8ms" sleeps actually ran at ~21ms a step, so every move
    # took ~2.7x the requested time - a 200ms flick became 547ms. That silently
    # broke the knock demo, whose whole point is re-entering inside a 400ms
    # window. Timing the loop instead keeps the duration honest on any machine;
    # a coarse sleep now costs intermediate frames rather than wall-clock.
    $clock = [System.Diagnostics.Stopwatch]::StartNew()
    while ($true) {
        $t = $clock.Elapsed.TotalMilliseconds / [Math]::Max(1, $Ms)
        if ($t -ge 1) { break }
        $e = if ($t -lt 0.5) { 2 * $t * $t } else { 1 - [Math]::Pow(-2 * $t + 2, 2) / 2 }
        $ok = [Ctl]::SetCursorPos(
            [int]($from.X + ($X - $from.X) * $e),
            [int]($from.Y + ($Y - $from.Y) * $e))
        if (-not $ok) { throw "SetCursorPos failed while moving to ($X,$Y)." }
        [System.Threading.Thread]::Sleep(4)
    }
    $clock.Stop()

    if (-not [Ctl]::SetCursorPos($X, $Y)) {
        throw "SetCursorPos failed at final position ($X,$Y)."
    }

    $actual = Get-MousePos
    Write-Log -Level DEBUG -Event 'MOUSE_ARRIVED' -Message "Cursor now at ($($actual.X),$($actual.Y))." -Data @{ x = $actual.X; y = $actual.Y }
    return $actual
}

function global:Send-Key {
    param([Parameter(Mandatory)][int[]]$Vks)

    if ($Vks.Count -eq 0) { return }

    Write-Log -Level INFO -Event 'KEY_SEQUENCE' -Message (($Vks | ForEach-Object { "0x{0:X2}" -f $_ }) -join '+')

    $pressed = New-Object System.Collections.Generic.List[byte]
    try {
        foreach ($v in $Vks) {
            [Ctl]::keybd_event([byte]$v, 0, 0, [UIntPtr]::Zero)
            $pressed.Add([byte]$v)
        }
        Start-Sleep -Milliseconds 40
    }
    finally {
        for ($i = $pressed.Count - 1; $i -ge 0; $i--) {
            [Ctl]::keybd_event($pressed[$i], 0, 2, [UIntPtr]::Zero)
        }
    }
}

function global:Key-Down {
    param([Parameter(Mandatory)][int]$Vk)
    [Ctl]::keybd_event([byte]$Vk, 0, 0, [UIntPtr]::Zero)
    Write-Log -Level INFO -Event 'KEY_DOWN' -Message ("0x{0:X2}" -f $Vk)
}

function global:Key-Up {
    param([Parameter(Mandatory)][int]$Vk)
    [Ctl]::keybd_event([byte]$Vk, 0, 2, [UIntPtr]::Zero)
    Write-Log -Level INFO -Event 'KEY_UP' -Message ("0x{0:X2}" -f $Vk)
}

function global:Get-VisibleWindows {
    $list = New-Object System.Collections.ArrayList
    $cb = [Ctl+EnumProc]{
        param($h, $p)
        if ([Ctl]::IsWindowVisible($h)) {
            $sb = New-Object System.Text.StringBuilder 512
            [void][Ctl]::GetWindowTextW($h, $sb, 512)
            $title = $sb.ToString()
            if ($title.Length -gt 0) {
                $cb2 = New-Object System.Text.StringBuilder 256
                [void][Ctl]::GetClassNameW($h, $cb2, 256)
                [void]$list.Add([pscustomobject]@{ H = $h; Title = $title; Class = $cb2.ToString() })
            }
        }
        return $true
    }
    [void][Ctl]::EnumWindows($cb, [IntPtr]::Zero)
    return $list
}

function global:Invoke-Click {
    param([ValidateSet('left','right')][string]$Button = 'left')

    Write-Log -Level INFO -Event 'MOUSE_CLICK' -Message $Button
    if ($Button -eq 'right') {
        [Mouse]::mouse_event(0x08, 0, 0, 0, [UIntPtr]::Zero)
        Start-Sleep -Milliseconds 60
        [Mouse]::mouse_event(0x10, 0, 0, 0, [UIntPtr]::Zero)
    }
    else {
        [Mouse]::mouse_event(0x02, 0, 0, 0, [UIntPtr]::Zero)
        Start-Sleep -Milliseconds 60
        [Mouse]::mouse_event(0x04, 0, 0, 0, [UIntPtr]::Zero)
    }
}

function global:Set-Stage {
    # -Overlap stacks the two windows instead of placing them side by side.
    # "Switch to last window" only changes which window has focus, and with
    # non-overlapping windows that is invisible - the knock clip looked like a
    # dead zone when the zone was firing correctly. Overlapped, the switch
    # visibly raises the window behind.
    param([switch]$Overlap)

    $wins = Get-VisibleWindows
    $repo = $wins | Where-Object { $_.Title -match 'windhawk-mods' } | Select-Object -First 1
    $site = $wins | Where-Object { $_.Title -match '^Windhawk - ' } | Select-Object -First 1

    Write-Log -Level INFO -Event 'STAGE_WINDOWS_FOUND' -Data @{
        repoFound = [bool]$repo
        siteFound = [bool]$site
        repoTitle = if ($repo) { $repo.Title } else { $null }
        siteTitle = if ($site) { $site.Title } else { $null }
    }

    if (-not $repo) { Write-Log -Level WARN -Event 'STAGE_REPO_MISSING' -Message 'windhawk-mods window was not found.' }
    if (-not $site) { Write-Log -Level WARN -Event 'STAGE_SITE_MISSING' -Message 'Windhawk site window was not found.' }

    foreach ($w in @($repo, $site)) {
        if ($w) {
            if (-not [Win]::IsWindow($w.H)) {
                Write-Log -Level WARN -Event 'STAGE_WINDOW_INVALID' -Message $w.Title
                continue
            }
            [void][Win]::ShowWindow($w.H, 9) # SW_RESTORE
        }
    }

    Start-Sleep -Milliseconds 250

    $flags = 0x0054 # SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW

    if ($Overlap) {
        $repoRect = @(300,  200, 2400, 1700)
        $siteRect = @(1240, 460, 2400, 1620)
    } else {
        $repoRect = @(180,  140, 1700, 1560)
        $siteRect = @(1980, 140, 1700, 1560)
    }

    if ($repo) {
        $ok = [Win]::SetWindowPos($repo.H, [IntPtr]::Zero, $repoRect[0], $repoRect[1], $repoRect[2], $repoRect[3], $flags)
        if (-not $ok) { Write-Log -Level WARN -Event 'STAGE_REPO_POSITION_FAILED' }
    }
    if ($site) {
        $ok = [Win]::SetWindowPos($site.H, [IntPtr]::Zero, $siteRect[0], $siteRect[1], $siteRect[2], $siteRect[3], $flags)
        if (-not $ok) { Write-Log -Level WARN -Event 'STAGE_SITE_POSITION_FAILED' }
    }

    Start-Sleep -Milliseconds 250

    # Focus the repo window first, then the site window. "Switch to last window"
    # is Alt+Tab, which follows the system's most-recently-used order - and that
    # order contains every window on the machine, not just the staged ones. With
    # only the site window ever focused here, the knock clip alt-tabbed to
    # whatever the user had touched last (it picked a window on the other
    # monitor, so the clip appeared to show nothing happening at all). Touching
    # both in order pins MRU[0]=site and MRU[1]=repo, so the switch is both
    # visible and confined to windows this script actually placed.
    if ($repo) {
        [void][Win]::SetForegroundWindow($repo.H)
        Start-Sleep -Milliseconds 300
    }

    if ($site) {
        if (-not [Win]::SetForegroundWindow($site.H)) {
            Write-Log -Level WARN -Event 'STAGE_FOREGROUND_FAILED' -Message $site.Title
        }
    }

    Start-Sleep -Milliseconds 250
    Write-Log -Level SUCCESS -Event 'STAGE_READY'
}
