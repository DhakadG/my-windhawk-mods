# Open the mod's Zones & settings window and screenshot just that window.
#
# The tray icon cannot be right-clicked synthetically (UIPI blocks injected
# input into the elevated Windhawk process), so the dashboard is opened by
# posting the tray window's own menu command instead. FindWindowW does not match
# the dashboard's class, so it is located by title afterwards.

$global:DemoLogConsole = $false
. (Join-Path $PSScriptRoot 'ctl.ps1')

Add-Type @'
using System;
using System.Runtime.InteropServices;

public static class Dash {
    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern IntPtr FindWindowW(string cls, string name);
    [DllImport("user32.dll")]
    public static extern IntPtr PostMessageW(IntPtr h, uint msg, IntPtr w, IntPtr l);
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr h, out RC r);
    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr h);
    [StructLayout(LayoutKind.Sequential)]
    public struct RC { public int L, T, R, B; }
}
'@

$IDM_SETTINGS = 100
$WM_COMMAND   = 0x0111

$tray = [Dash]::FindWindowW('WindhawkHotCornersTray', $null)
"tray hwnd : $tray"
if ($tray -eq [IntPtr]::Zero) { throw 'Tray window not found - is the mod loaded?' }

[void][Dash]::PostMessageW($tray, $WM_COMMAND, [IntPtr]$IDM_SETTINGS, [IntPtr]::Zero)
Start-Sleep -Milliseconds 2200

$dash = Get-VisibleWindows | Where-Object { $_.Title -match 'Zones|Hot Corners' } | Select-Object -First 1
if (-not $dash) {
    'dashboard NOT found; visible windows:'
    Get-VisibleWindows | ForEach-Object { '   ' + $_.Title }
    throw 'dashboard did not open'
}

[void][Dash]::SetForegroundWindow($dash.H)
Start-Sleep -Milliseconds 700

$r = New-Object Dash+RC
[void][Dash]::GetWindowRect($dash.H, [ref]$r)
$w = $r.R - $r.L
$h = $r.B - $r.T
"dashboard : $($dash.Title)  $($r.L),$($r.T)  ${w}x${h}"

# Park the pointer over the top-right zone so the detail panel is populated
# rather than showing its empty prompt - the panel is half the point of the
# window. Top-right is the hold zone, so the shot also documents the release
# action, which nothing else in the readme can show as a still.
[void][Ctl]::SetCursorPos($r.L + [int]($w * 0.888), $r.T + [int]($h * 0.180))
Start-Sleep -Milliseconds 1100

$out = Join-Path $PSScriptRoot 'dashboard.png'
& ffmpeg -hide_banner -loglevel error `
    -f lavfi -i "ddagrab=output_idx=0:framerate=10:draw_mouse=1,hwdownload,format=bgra" `
    -frames:v 3 -y (Join-Path $PSScriptRoot 'dash-full.png') 2>&1 | Out-Null

& ffmpeg -hide_banner -loglevel error -i (Join-Path $PSScriptRoot 'dash-full.png') `
    -vf "crop=${w}:${h}:$($r.L):$($r.T)" -y $out 2>&1 | Out-Null

if (Test-Path $out) { "written    : $out ({0:N0} KB)" -f ((Get-Item $out).Length / 1KB) }
else { 'screenshot FAILED' }
