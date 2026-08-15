# Record one scripted demo clip with ffmpeg ddagrab.
#
# The recorder is deliberately defensive: it captures ffmpeg stderr, verifies
# the output dimensions, cleans up ffmpeg on action failure, and records every
# important transition in the shared session log.

param(
    [Parameter(Mandatory)][string]$Name,
    [Parameter(Mandatory)][ValidateRange(0.5, 600)][double]$Seconds,
    [Parameter(Mandatory)][scriptblock]$Action,
    [ValidateRange(1,240)][int]$Fps = 60,
    [string]$Dir = $PSScriptRoot,
    [ValidateRange(0,16)][int]$Output = 0,
    [string]$Log = '',
    [string]$SessionId = '',
    [ValidateRange(1,600)][int]$TimeoutSeconds = 120
)
Set-StrictMode -Version Latest


. (Join-Path $PSScriptRoot 'logger.ps1')
if ($Log) { Initialize-DemoLog -Path $Log -SessionId $(if ($SessionId) { $SessionId } else { [guid]::NewGuid().ToString('N') }) -Append }
$global:DemoScriptName = 'rec'
. (Join-Path $PSScriptRoot 'ctl.ps1')

function Test-CommandAvailable {
    param([Parameter(Mandatory)][string]$Command)
    $null -ne (Get-Command $Command -ErrorAction SilentlyContinue)
}

if (-not (Test-CommandAvailable 'ffmpeg')) { throw 'ffmpeg was not found on PATH.' }
if (-not (Test-CommandAvailable 'ffprobe')) { throw 'ffprobe was not found on PATH.' }

New-Item -ItemType Directory -Force -Path $Dir | Out-Null

$raw = Join-Path $Dir "$Name.mkv"
$stderr = Join-Path $Dir "$Name.ffmpeg.stderr.log"
$stdout = Join-Path $Dir "$Name.ffmpeg.stdout.log"

Remove-Item -LiteralPath $raw, $stderr, $stdout -Force -ErrorAction SilentlyContinue

$psi = [System.Diagnostics.ProcessStartInfo]::new()
$psi.FileName = (Get-Command ffmpeg).Source
$psi.UseShellExecute = $false
$psi.CreateNoWindow = $true
$psi.RedirectStandardError = $true
$psi.RedirectStandardOutput = $true

$args = @(
    '-hide_banner', '-loglevel', 'warning',
    '-f', 'lavfi',
    '-i', "ddagrab=output_idx=$Output`:framerate=$Fps`:draw_mouse=1,hwdownload,format=bgra",
    '-t', $Seconds.ToString([Globalization.CultureInfo]::InvariantCulture),
    '-c:v', 'libx264rgb', '-preset', 'ultrafast', '-qp', '0',
    '-pix_fmt', 'rgb24',
    '-y', $raw
)
foreach ($arg in $args) { [void]$psi.ArgumentList.Add($arg) }

$ff = [System.Diagnostics.Process]::new()
$ff.StartInfo = $psi

Write-Log -Level INFO -Event 'RECORD_START' -Message $Name -Data @{
    seconds = $Seconds; fps = $Fps; output = $Output; raw = $raw
    ffmpegArgs = $args
}

$startedAt = Get-Date
$actionError = $null

try {
    if (-not $ff.Start()) { throw 'ffmpeg process failed to start.' }

    Write-Log -Level INFO -Event 'FFMPEG_STARTED' -Message $Name -Data @{ pid = $ff.Id }

    $stdoutTask = $ff.StandardOutput.ReadToEndAsync()
    $stderrTask = $ff.StandardError.ReadToEndAsync()

    # Give Desktop Duplication a moment to deliver the first stable frame.
    Start-Sleep -Milliseconds 700
    Write-Log -Level DEBUG -Event 'CAPTURE_WARMUP_COMPLETE' -Message $Name

    try {
        Write-Log -Level INFO -Event 'ACTION_EXECUTE' -Message $Name
        & $Action
        Write-Log -Level SUCCESS -Event 'ACTION_COMPLETE' -Message $Name
    }
    catch {
        $actionError = $_.Exception
        Write-LogException -Event 'ACTION_FAILED' -Exception $actionError -Message $Name
        throw
    }
}
finally {
    if (-not $ff.HasExited) {
        $waitMs = [int]([Math]::Min($TimeoutSeconds * 1000, [Math]::Max(5000, ($Seconds * 1000) + 15000)))
        Write-Log -Level INFO -Event 'FFMPEG_WAIT' -Message $Name -Data @{ timeoutMs = $waitMs }
        if (-not $ff.WaitForExit($waitMs)) {
            Write-Log -Level ERROR -Event 'FFMPEG_TIMEOUT' -Message $Name -Data @{ pid = $ff.Id }
            try { $ff.Kill($true) } catch {}
        }
    }

    try {
        $stdoutText = if ($stdoutTask) { $stdoutTask.Result } else { '' }
        if ($stdoutText) { [System.IO.File]::WriteAllText($stdout, $stdoutText) }
    } catch {}

    try {
        $stderrText = if ($stderrTask) { $stderrTask.Result } else { '' }
        if ($stderrText) { [System.IO.File]::WriteAllText($stderr, $stderrText) }
        if ($stderrText) {
            Write-Log -Level WARN -Event 'FFMPEG_STDERR' -Message $Name -Data @{ stderr = $stderrText.Trim() }
        }
    } catch {}

    $exitCode = if ($ff.HasExited) { $ff.ExitCode } else { -1 }
    Write-Log -Level INFO -Event 'FFMPEG_EXIT' -Message $Name -Data @{ exitCode = $exitCode }
    $ff.Dispose()
}

if ($actionError) { throw $actionError }

if (-not (Test-Path -LiteralPath $raw)) {
    Write-Log -Level ERROR -Event 'RECORD_OUTPUT_MISSING' -Message $Name
    throw "Recording '$Name' did not produce $raw"
}

$probe = & ffprobe -v error `
    -select_streams v:0 `
    -show_entries stream=width,height,r_frame_rate,codec_name:format=duration,size `
    -of json -- $raw 2>&1

if ($LASTEXITCODE -ne 0) {
    Write-Log -Level ERROR -Event 'FFPROBE_FAILED' -Message $Name -Data @{ output = ($probe -join "`n") }
    throw "ffprobe failed for '$raw'."
}

$info = $probe -join "`n"
$expectedW = 3840
$expectedH = 2160

if ($info -notmatch '"width"\s*:\s*3840' -or $info -notmatch '"height"\s*:\s*2160') {
    Write-Log -Level ERROR -Event 'RECORD_DIMENSION_MISMATCH' -Message $Name -Data @{ probe = $info }
    throw "$Name did not record the expected 3840x2160 panel. Probe: $info"
}

$sizeMb = (Get-Item -LiteralPath $raw).Length / 1MB
$elapsed = ((Get-Date) - $startedAt).TotalSeconds

Write-Log -Level SUCCESS -Event 'RECORD_VERIFIED' -Message $Name -Data @{
    sizeMB = [math]::Round($sizeMb, 2)
    elapsedSec = [math]::Round($elapsed, 2)
    probe = $info
}

"recorded $Name ({0:N2} MB)" -f $sizeMb
