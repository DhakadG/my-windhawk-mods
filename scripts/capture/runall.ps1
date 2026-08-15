# Record one clip per trigger style with a shared session log.
#
# Gated triggers deliberately show the unsuccessful attempt before the
# successful attempt so the resulting clip teaches the gate rather than merely
# showing a hot corner firing.
#
# Every clip starts from the same staged desktop. Per-clip cleanup is performed
# even when ffmpeg or the scripted action fails.

param(
    [string]$LogDir = (Join-Path $PSScriptRoot 'logs'),
    [switch]$NoDebugWatch,
    [ValidateRange(60,1200)][int]$DebugWatchSeconds = 300,
    [switch]$StopOnError,
    [switch]$AllowWindows
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

. (Join-Path $PSScriptRoot 'logger.ps1')

$global:DemoScriptName = 'runall'
$sessionId = [guid]::NewGuid().ToString('N')
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$logPath = Join-Path $LogDir ("run-{0}-{1}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'), $sessionId.Substring(0,8))
Initialize-DemoLog -Path $logPath -SessionId $sessionId | Out-Null

$debugLog = Join-Path $LogDir ("debug-{0}-{1}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'), $sessionId.Substring(0,8))
$script:debugProc = $null
$failedClips = New-Object System.Collections.Generic.List[string]
$completedClips = New-Object System.Collections.Generic.List[string]

$ctlScript = Join-Path $PSScriptRoot 'ctl.ps1'
$recScript = Join-Path $PSScriptRoot 'rec.ps1'
$dbgScript = Join-Path $PSScriptRoot 'dbgwatch.ps1'

foreach ($required in @($ctlScript, $recScript)) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Missing required script: $required" }
}

. $ctlScript

$ESC  = 0x1B
$CTRL = 0x11

$SCREEN_WIDTH  = 3840
$SCREEN_HEIGHT = 2160

$TL = @{ X = 3;    Y = 3 }
$TR = @{ X = 3837; Y = 3 }
$BL = @{ X = 3;    Y = 2157 }
$BR = @{ X = 3837; Y = 2157 }
$LEFT_CENTER  = @{ X = 4;    Y = 1080 }
$RIGHT_CENTER = @{ X = 3836; Y = 1080 }
$TOP_CENTER   = @{ X = 1920; Y = 3 }
$STAGE_CURSOR = @{ X = 1700; Y = 900 }

$detectedScreen = Get-ScreenSize
if ($detectedScreen.W -ne $SCREEN_WIDTH -or $detectedScreen.H -ne $SCREEN_HEIGHT) {
    Write-Log -Level ERROR -Event 'SCREEN_SIZE_MISMATCH' -Message "Expected ${SCREEN_WIDTH}x${SCREEN_HEIGHT}, detected $($detectedScreen.W)x$($detectedScreen.H)."
    throw "Expected ${SCREEN_WIDTH}x${SCREEN_HEIGHT}, detected $($detectedScreen.W)x$($detectedScreen.H)."
}

# Snap Assist previews every switchable window, not just the staged ones, so a
# window nobody thought about can appear in a clip that was never pointed at it.
# That is how a folder of personal documents reached a recording once. The run
# refuses to start unless every visible window is one it expects; -AllowWindows
# overrides for a deliberate exception.
# Matched on window class, not title. A title allowlist is useless here because
# the windows that are legitimately open - a browser, a music player - have
# titles that change constantly, so the list would either be permanently
# overridden or permanently wrong. These two classes are File Explorer, which is
# the window that actually puts personal documents on screen.
$RISKY_WINDOW_CLASSES = @('CabinetWClass', 'ExploreWClass')

function Assert-CleanDesktop {
    $all = @(Get-VisibleWindows)
    $risky = @($all | Where-Object { $RISKY_WINDOW_CLASSES -contains $_.Class })

    Write-Log -Level INFO -Event 'PREFLIGHT_WINDOWS' -Data @{
        visible = @($all | ForEach-Object { "$($_.Title) [$($_.Class)]" })
        risky   = @($risky | ForEach-Object { $_.Title })
    }

    if ($risky.Count -gt 0) {
        $names = ($risky | ForEach-Object { $_.Title }) -join '; '
        Write-Log -Level ERROR -Event 'PREFLIGHT_EXPLORER_OPEN' -Message $names
        if (-not $AllowWindows) {
            throw "File Explorer is open and Snap Assist will preview its contents: $names. Close it, or re-run with -AllowWindows."
        }
        Write-Log -Level WARN -Event 'PREFLIGHT_OVERRIDDEN' -Message $names
    }
    else {
        Write-Log -Level SUCCESS -Event 'PREFLIGHT_CLEAN' -Message "$($all.Count) windows open, no File Explorer."
    }
}

function Wait-Demo {
    param([int]$Ms, [string]$Reason)
    Write-Log -Level DEBUG -Event 'WAIT' -Message $Reason -Data @{ milliseconds = $Ms }
    Start-Sleep -Milliseconds $Ms
}

function Move-To {
    param([int]$X, [int]$Y, [int]$Ms, [string]$Reason = '')
    Write-Log -Level INFO -Event 'STEP_MOVE' -Message $(if ($Reason) { $Reason } else { "to ($X,$Y)" }) -Data @{ x = $X; y = $Y; durationMs = $Ms }
    # Move-MouseTo returns the arrived-at point; discard it, or every step of
    # every clip prints a WPOINT into the recorder's output.
    [void](Move-MouseTo -X $X -Y $Y -Ms $Ms)
}

function Move-ToPoint {
    param([hashtable]$Point, [int]$Ms, [string]$Reason = '')
    Move-To -X $Point.X -Y $Point.Y -Ms $Ms -Reason $Reason
}

function Reset-Stage {
    Write-Log -Level INFO -Event 'STAGE_RESET_START'
    Send-Key -Vks @($ESC)
    Wait-Demo 400 'dismiss transient UI'
    Set-Stage
    [void][Ctl]::SetCursorPos($STAGE_CURSOR.X, $STAGE_CURSOR.Y)
    Wait-Demo 700 'stage settle'
    Write-Log -Level SUCCESS -Event 'STAGE_RESET_COMPLETE'
}

function Start-DebugWatch {
    if ($NoDebugWatch) {
        Write-Log -Level INFO -Event 'DEBUG_WATCH_DISABLED'
        return
    }

    if (-not (Test-Path -LiteralPath $dbgScript)) {
        Write-Log -Level WARN -Event 'DEBUG_WATCH_SCRIPT_MISSING' -Message $dbgScript
        return
    }

    $pwsh = Join-Path $PSHOME 'pwsh.exe'
    if (-not (Test-Path -LiteralPath $pwsh)) {
        Write-Log -Level WARN -Event 'DEBUG_WATCH_PWSH_MISSING' -Message $pwsh
        return
    }

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $pwsh
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true

    $args = @(
        '-NoLogo', '-NoProfile', '-File', $dbgScript,
        '-Seconds', $DebugWatchSeconds.ToString(),
        '-Out', $debugLog,
        '-Log', $global:DemoLogPath,
        '-SessionId', $global:DemoSessionId
    )
    foreach ($arg in $args) { [void]$psi.ArgumentList.Add($arg) }

    $script:debugProc = [System.Diagnostics.Process]::new()
    $script:debugProc.StartInfo = $psi
    if (-not $script:debugProc.Start()) {
        $script:debugProc.Dispose()
        $script:debugProc = $null
        throw 'Unable to start dbgwatch.ps1.'
    }

    Write-Log -Level SUCCESS -Event 'DEBUG_WATCH_STARTED' -Data @{ pid = $script:debugProc.Id; output = $debugLog; seconds = $DebugWatchSeconds }
}

function Stop-DebugWatch {
    if (-not $script:debugProc) { return }

    try {
        if (-not $script:debugProc.HasExited) {
            $script:debugProc.Kill()
            $script:debugProc.WaitForExit(3000)
        }
        Write-Log -Level INFO -Event 'DEBUG_WATCH_STOPPED' -Data @{ pid = $script:debugProc.Id }
    }
    catch {
        Write-LogException -Event 'DEBUG_WATCH_STOP_FAILED' -Exception $_.Exception
    }
    finally {
        $script:debugProc.Dispose()
        $script:debugProc = $null
    }
}

function Rec {
    param(
        [Parameter(Mandatory)][string]$Name,
        [Parameter(Mandatory)][double]$Secs,
        [Parameter(Mandatory)][string]$Description,
        [Parameter(Mandatory)][scriptblock]$Body
    )

    $started = Get-Date
    Write-Log -Level INFO -Event 'CLIP_START' -Message $Name -Data @{ seconds = $Secs; description = $Description }

    try {
        Reset-Stage
        & $recScript -Name $Name -Seconds $Secs -Action $Body -Log $global:DemoLogPath -SessionId $global:DemoSessionId
        $completedClips.Add($Name)
        $elapsed = ((Get-Date) - $started).TotalSeconds
        Write-Log -Level SUCCESS -Event 'CLIP_COMPLETE' -Message $Name -Data @{ elapsedSec = [math]::Round($elapsed, 2) }
    }
    catch {
        $failedClips.Add($Name)
        Write-LogException -Event 'CLIP_FAILED' -Exception $_.Exception -Message $Name

        try {
            Reset-Stage
        }
        catch {
            Write-LogException -Event 'POST_FAILURE_STAGE_RESET_FAILED' -Exception $_.Exception -Message $Name
        }

        if ($StopOnError) { throw }
    }
    finally {
        try {
            Send-Key -Vks @($ESC)
        }
        catch {
            Write-LogException -Event 'FINAL_ESCAPE_FAILED' -Exception $_.Exception -Message $Name
        }
    }
}

Write-Log -Level INFO -Event 'RUN_START' -Message 'Windhawk trigger demo recording started.' -Data @{
    scriptDir = $PSScriptRoot; screenWidth = $SCREEN_WIDTH; screenHeight = $SCREEN_HEIGHT
    log = $logPath; debugLog = $debugLog
}

try {
    Assert-CleanDesktop
    Start-DebugWatch

    Rec '01-arrival' 6.0 'Arrival trigger: top-left corner opens Task View.' {
        Wait-Demo 500 'pre-arrival pause'
        Move-ToPoint $TL 620 'enter top-left arrival corner'
        Wait-Demo 2700 'allow Task View animation'
        Send-Key -Vks @($ESC)
        Wait-Demo 900 'dismiss Task View'
    }

    Rec '02-dwell' 8.5 'Dwell gate: fast brush fails, sustained presence succeeds.' {
        Wait-Demo 400 'pre-dwell pause'
        Move-To -X $TOP_CENTER.X -Y $TOP_CENTER.Y -Ms 500 -Reason 'enter top-center edge for failed dwell'
        Move-To -X 2600 -Y 3 -Ms 260 -Reason 'leave too quickly; should not fire'
        Wait-Demo 900 'show failed attempt'
        Move-ToPoint $TOP_CENTER 500 'return to top-center edge'
        Wait-Demo 2900 'satisfy dwell threshold and show result'
        Send-Key -Vks @($ESC)
        Wait-Demo 900 'dismiss resulting UI'
    }

    Rec '03-knock' 8.0 'Knock gate: first visit fails, quick re-entry succeeds.' {
        # Overlapped so the window switch is actually visible on screen.
        Set-Stage -Overlap
        Wait-Demo 400 'pre-knock pause'
        Move-ToPoint $BR 620 'first bottom-right visit; should not fire'
        Wait-Demo 1300 'show failed first visit'
        # The knock has to complete within the zone's 400ms window, measured
        # from leaving to re-entering. Two 200ms moves cost ~470ms end to end
        # once per-call overhead is counted, which missed it every time and made
        # a working zone look dead. 90ms each lands at ~250ms with room to
        # spare, and a fast flick is what a knock looks like anyway.
        Move-To -X 3550 -Y 1900 -Ms 90 -Reason 'leave knock corner'
        Move-ToPoint $BR 90 're-enter inside the knock window'
        Wait-Demo 2800 'allow switch-last action to appear'
    }

    Rec '04-hold' 8.5 'Hold trigger: sustained corner presence shows desktop peek.' {
        Wait-Demo 400 'pre-hold pause'
        Move-ToPoint $TR 620 'enter top-right hold corner'
        Wait-Demo 3100 'hold long enough to show desktop peek'
        Move-To -X 3000 -Y 700 -Ms 700 -Reason 'leave corner so windows return'
        Wait-Demo 2300 'show restored windows'
    }

    Rec '05-modifier' 8.5 'Modifier gate: no Ctrl fails, Ctrl held before entry succeeds.' {
        Wait-Demo 400 'pre-modifier pause'
        Move-ToPoint $LEFT_CENTER 620 'touch left edge without Ctrl; should not fire'
        Wait-Demo 1500 'show failed modifier attempt'
        Move-To -X 700 -Y 1080 -Ms 300 -Reason 'leave left trigger region'

        Key-Down -Vk $CTRL
        try {
            Move-ToPoint $LEFT_CENTER 400 're-enter while Ctrl is already held'
            Wait-Demo 400 'let the snap land'
        }
        finally {
            Key-Up -Vk $CTRL
        }
        # Snap Assist offers to fill the other half by previewing every other
        # window, which puts windows on screen that this clip never staged.
        # Dismiss it immediately - and only after Ctrl is up, because Ctrl+Esc
        # is Start menu.
        Send-Key -Vks @($ESC)
        Wait-Demo 2400 'show the snapped window on its own'
    }

    Rec '06-start-menu' 6.0 'Arrival trigger: bottom-left corner opens Start.' {
        Wait-Demo 500 'pre-start-menu pause'
        Move-ToPoint $BL 620 'enter bottom-left Start corner'
        Wait-Demo 2700 'allow Start menu animation'
        Send-Key -Vks @($ESC)
        Wait-Demo 900 'dismiss Start menu'
    }

    Rec '07-alternate' 8.5 'Alternate trigger: first visit shows one half, second visit the other.' {
        Wait-Demo 400 'pre-alternate pause'
        Move-ToPoint $RIGHT_CENTER 560 'first right-edge visit'
        Wait-Demo 2100 'show first alternate action'
        Move-To -X 3200 -Y 900 -Ms 350 -Reason 'leave right trigger region'
        Move-ToPoint $RIGHT_CENTER 450 'second right-edge visit'
        Wait-Demo 2500 'show second alternate action'
    }
}
catch {
    Write-LogException -Event 'RUN_FATAL' -Exception $_.Exception
    throw
}
finally {
    try { Stop-DebugWatch } catch {}
    try { Send-Key -Vks @($ESC) } catch {}
    try { Reset-Stage } catch { Write-LogException -Event 'FINAL_STAGE_RESET_FAILED' -Exception $_.Exception }

    $level = if ($failedClips.Count -eq 0) { 'SUCCESS' } else { 'WARN' }
    Write-Log -Level $level -Event 'RUN_SUMMARY' -Data @{
        completed = @($completedClips)
        failed = @($failedClips)
        log = $logPath
        debugLog = $debugLog
    }

    Write-Host ''
    Write-Host "Session log : $logPath"
    Write-Host "Debug log   : $debugLog"
    Write-Host "Completed   : $($completedClips.Count)"
    Write-Host "Failed      : $($failedClips.Count)"

    if ($failedClips.Count -gt 0) {
        Write-Host ("Failed clips: " + ($failedClips -join ', ')) -ForegroundColor Yellow
    }
}

if ($failedClips.Count -gt 0) { exit 1 }

