# Shared logging for the Windhawk demo scripts.
#
# The logger writes one human-readable, machine-parseable line per event:
#   ISO_TIMESTAMP [LEVEL] [SCRIPT] EVENT :: message | {json data}
#
# It is intentionally dependency-free and safe to dot-source more than once.

Set-StrictMode -Version Latest

# Every script here runs under StrictMode, which makes reading a variable that
# was never assigned a terminating error - and the whole point of these globals
# is that they may not be set yet. Declaring them as $null up front is what lets
# the "if ($global:X)" defaults below work at all; without this, dot-sourcing
# logger.ps1 fails on its own last line and nothing in the folder runs.
# The loop variable is deliberately long-winded. This file is dot-sourced into
# the caller's scope, so a plain $name here would land in that scope and silently
# overwrite a caller's variable of any casing - which is exactly what happened
# with rec.ps1's -Name parameter, sending all seven clips to one filename.
foreach ($demoGlobalVarName in 'DemoLogPath', 'DemoSessionId', 'DemoScriptName',
                               'DemoLogConsole', 'DemoLogInitialized') {
    if (-not (Get-Variable -Name $demoGlobalVarName -Scope Global -ErrorAction SilentlyContinue)) {
        Set-Variable -Name $demoGlobalVarName -Scope Global -Value $null
    }
}
Remove-Variable demoGlobalVarName -ErrorAction SilentlyContinue

function global:Initialize-DemoLog {
    param(
        [Parameter(Mandatory)]
        [string]$Path,

        [string]$SessionId = $(if ($global:DemoSessionId) { $global:DemoSessionId } else { [guid]::NewGuid().ToString('N') }),

        [switch]$Append
    )

    $full = [System.IO.Path]::GetFullPath($Path)
    $parent = Split-Path -Parent $full

    if ($parent) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    $global:DemoLogPath = $full
    $global:DemoSessionId = $SessionId

    if (-not $Append) {
        $header = @(
            "# Windhawk demo session",
            "# SessionId: $SessionId",
            "# Started: $((Get-Date).ToString('o'))",
            "# Host: $($env:COMPUTERNAME)",
            "# User: $env:USERNAME",
            "# PowerShell: $($PSVersionTable.PSVersion)",
            "# ------------------------------------------------------------"
        ) -join [Environment]::NewLine

        [System.IO.File]::WriteAllText($full, $header + [Environment]::NewLine, [System.Text.UTF8Encoding]::new($false))
    }

    return $full
}

function global:Get-DemoLogPath {
    if ($global:DemoLogPath) {
        return $global:DemoLogPath
    }

    $stamp = Get-Date -Format 'yyyyMMdd-HHmmssfff'
    $default = Join-Path $PSScriptRoot "logs\demo-$stamp.log"
    Initialize-DemoLog -Path $default | Out-Null
    return $global:DemoLogPath
}

function global:Write-Log {
    param(
        [ValidateSet('TRACE','DEBUG','INFO','WARN','ERROR','SUCCESS')]
        [string]$Level = 'INFO',

        [Parameter(Mandatory)]
        [string]$Event,

        [string]$Message = '',

        [hashtable]$Data
    )

    $path = Get-DemoLogPath
    $scriptName = if ($global:DemoScriptName) { $global:DemoScriptName } else { Split-Path -Leaf $PSCommandPath }
    $timestamp = (Get-Date).ToString('o')
    $session = if ($global:DemoSessionId) { $global:DemoSessionId } else { 'unknown' }

    $line = "$timestamp [$Level] [$scriptName] [$session] $Event"
    if ($Message) {
        $line += " :: $Message"
    }

    if ($Data) {
        $json = $Data | ConvertTo-Json -Compress -Depth 8
        $line += " | $json"
    }

    Add-Content -LiteralPath $path -Value $line -Encoding utf8

    if ($global:DemoLogConsole -ne $false) {
        $prefix = "[{0}] {1}" -f $Level, $Event
        if ($Message) { $prefix += " - $Message" }
        switch ($Level) {
            'ERROR'   { Write-Host $prefix -ForegroundColor Red }
            'WARN'    { Write-Host $prefix -ForegroundColor Yellow }
            'SUCCESS' { Write-Host $prefix -ForegroundColor Green }
            default   { Write-Host $prefix }
        }
    }
}

function global:Write-LogException {
    param(
        [Parameter(Mandatory)]
        [string]$Event,

        [Parameter(Mandatory)]
        [System.Exception]$Exception,

        [string]$Message = ''
    )

    $details = @{
        type       = $Exception.GetType().FullName
        message    = $Exception.Message
        stackTrace = $Exception.StackTrace
    }

    Write-Log -Level ERROR -Event $Event -Message $Message -Data $details
}

function global:Invoke-Logged {
    param(
        [Parameter(Mandatory)]
        [string]$Name,

        [Parameter(Mandatory)]
        [scriptblock]$Action,

        [hashtable]$Data
    )

    $started = Get-Date
    Write-Log -Level INFO -Event 'ACTION_START' -Message $Name -Data $Data

    try {
        $result = & $Action
        $elapsed = ((Get-Date) - $started).TotalMilliseconds
        Write-Log -Level SUCCESS -Event 'ACTION_END' -Message $Name -Data @{ durationMs = [math]::Round($elapsed, 0) }
        return $result
    }
    catch {
        $elapsed = ((Get-Date) - $started).TotalMilliseconds
        Write-LogException -Event 'ACTION_FAILED' -Exception $_.Exception -Message $Name
        Write-Log -Level DEBUG -Event 'ACTION_DURATION' -Message $Name -Data @{ durationMs = [math]::Round($elapsed, 0) }
        throw
    }
}

if (-not $global:DemoLogInitialized) {
    $global:DemoLogInitialized = $true
}
