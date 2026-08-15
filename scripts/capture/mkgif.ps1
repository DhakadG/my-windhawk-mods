# Turn recorded MKV clips into README-sized GIFs.
#
# Each conversion is verified and logged. ffmpeg stderr is captured into a
# per-clip .ffmpeg.stderr.log file when conversion fails or emits warnings.

param(
    [string]$Dir = $PSScriptRoot,
    [string]$Out = (Join-Path $PSScriptRoot 'gif'),
    [ValidateRange(120,3840)][int]$Width = 1200,
    [ValidateRange(1,60)][int]$Fps = 24,
    [ValidateRange(0,600)][double]$Start = 0.95,
    [ValidateRange(0.1,600)][double]$Length = 5.2,
    [string]$Log = '',
    [string]$SessionId = ''
)
Set-StrictMode -Version Latest


. (Join-Path $PSScriptRoot 'logger.ps1')
if ($Log) { Initialize-DemoLog -Path $Log -SessionId $(if ($SessionId) { $SessionId } else { [guid]::NewGuid().ToString('N') }) -Append }
$global:DemoScriptName = 'mkgif'

function Require-Tool {
    param([Parameter(Mandatory)][string]$Name)
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "$Name was not found on PATH."
    }
}

Require-Tool 'ffmpeg'
Require-Tool 'ffprobe'

if (-not (Test-Path -LiteralPath $Dir -PathType Container)) { throw "Input directory does not exist: $Dir" }
New-Item -ItemType Directory -Force -Path $Out | Out-Null

$vf = "eq=saturation=1.25:contrast=1.05,fps=$Fps,scale=${Width}:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=256:stats_mode=full[p];[b][p]paletteuse=dither=none"

# Per-clip trim. A single window across all seven either opened mid-staging or
# ran past the end: the clips are 6.0-8.5s long and the moment that matters sits
# at a different offset in each. Values are the recorded timeline, so they
# start just after the desktop settles and stop once the result has been held
# long enough to read. -Start/-Length still apply to anything not listed.
$TRIMS = @{
    '01-arrival'    = @(1.10, 4.60)   # Task View opens ~3.4s
    '02-dwell'      = @(1.00, 6.60)   # brush fails ~2.2s, dwell fires ~5.0s
    '03-knock'      = @(2.00, 5.60)   # overlap staging ends ~2.0s, knock fires ~4.9s
    '04-hold'       = @(1.00, 6.60)   # peek ~1.8s, restored ~5.5s
    '05-modifier'   = @(1.00, 7.00)   # inert touch ~2.4s, Ctrl snap ~4.4s
    '06-start-menu' = @(1.10, 4.60)   # Start opens ~3.4s
    '07-alternate'  = @(1.00, 7.00)   # maximise ~3.5s, restore ~6.6s
}

$clips = @(Get-ChildItem -LiteralPath $Dir -Filter '*.mkv' -File | Sort-Object Name)
if ($clips.Count -eq 0) {
    Write-Log -Level WARN -Event 'GIF_NO_INPUTS' -Message $Dir
    return
}

$okCount = 0
$failCount = 0

foreach ($m in $clips) {
    $gifName = "$($m.BaseName -replace '^\d\d-','').gif"
    $gif = Join-Path $Out $gifName
    $stderr = Join-Path $Out "$($m.BaseName).ffmpeg.stderr.log"

    $clipStart = $Start
    $clipLength = $Length
    if ($TRIMS.ContainsKey($m.BaseName)) {
        $clipStart = $TRIMS[$m.BaseName][0]
        $clipLength = $TRIMS[$m.BaseName][1]
    }

    Write-Log -Level INFO -Event 'GIF_START' -Message $m.Name -Data @{
        output = $gif; start = $clipStart; length = $clipLength; width = $Width; fps = $Fps
    }

    try {
        Remove-Item -LiteralPath $gif, $stderr -Force -ErrorAction SilentlyContinue

        $psi = [System.Diagnostics.ProcessStartInfo]::new()
        $psi.FileName = (Get-Command ffmpeg).Source
        $psi.UseShellExecute = $false
        $psi.CreateNoWindow = $true
        $psi.RedirectStandardError = $true
        $psi.RedirectStandardOutput = $true

        $args = @(
            '-hide_banner', '-loglevel', 'warning',
            '-ss', $clipStart.ToString([Globalization.CultureInfo]::InvariantCulture),
            '-t', $clipLength.ToString([Globalization.CultureInfo]::InvariantCulture),
            '-i', $m.FullName,
            '-filter_complex', $vf,
            '-loop', '0',
            '-y', $gif
        )
        foreach ($arg in $args) { [void]$psi.ArgumentList.Add($arg) }

        $p = [System.Diagnostics.Process]::new()
        $p.StartInfo = $psi
        [void]$p.Start()
        $stdoutTask = $p.StandardOutput.ReadToEndAsync()
        $stderrTask = $p.StandardError.ReadToEndAsync()
        $p.WaitForExit()
        $stdoutText = $stdoutTask.Result
        $stderrText = $stderrTask.Result
        $exitCode = $p.ExitCode
        $p.Dispose()

        if ($stdoutText) { [System.IO.File]::WriteAllText((Join-Path $Out "$($m.BaseName).ffmpeg.stdout.log"), $stdoutText) }
        if ($stderrText) { [System.IO.File]::WriteAllText($stderr, $stderrText) }

        if ($stderrText) {
            Write-Log -Level WARN -Event 'GIF_FFMPEG_WARN' -Message $m.Name -Data @{ stderr = $stderrText.Trim() }
        }

        if ($exitCode -ne 0 -or -not (Test-Path -LiteralPath $gif)) {
            throw "ffmpeg failed with exit code $exitCode."
        }

        $sizeMb = (Get-Item -LiteralPath $gif).Length / 1MB
        $probe = & ffprobe -v error -select_streams v:0 -show_entries stream=width,height,r_frame_rate -of csv=p=0 -- $gif 2>&1
        if ($LASTEXITCODE -ne 0) { throw "ffprobe failed for GIF: $($probe -join ' ')" }

        $okCount++
        Write-Log -Level SUCCESS -Event 'GIF_COMPLETE' -Message $gifName -Data @{
            sizeMB = [math]::Round($sizeMb, 2); probe = ($probe -join ' ')
        }
        "{0,-28} {1,7:N2} MB" -f $gifName, $sizeMb
    }
    catch {
        $failCount++
        Write-LogException -Event 'GIF_FAILED' -Exception $_.Exception -Message $m.Name
        "{0,-28} FAILED" -f $m.BaseName
    }
}

Write-Log -Level $(if ($failCount -eq 0) { 'SUCCESS' } else { 'WARN' }) -Event 'GIF_SUMMARY' -Data @{
    inputs = $clips.Count; succeeded = $okCount; failed = $failCount; outputDir = $Out
}

if ($failCount -gt 0) { exit 1 }

