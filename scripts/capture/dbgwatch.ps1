# Capture OutputDebugString system-wide using the DBWIN shared buffer.
#
# The raw debug capture is written to -Out, while lifecycle/error information
# is also recorded in the shared session log when -Log is provided.

param(
    [ValidateRange(1,3600)][int]$Seconds = 20,
    [Parameter(Mandatory)][string]$Out,
    [string]$Log = '',
    [string]$SessionId = ''
)
Set-StrictMode -Version Latest


. (Join-Path $PSScriptRoot 'logger.ps1')
if ($Log) { Initialize-DemoLog -Path $Log -SessionId $(if ($SessionId) { $SessionId } else { [guid]::NewGuid().ToString('N') }) -Append }
$global:DemoScriptName = 'dbgwatch'

$parent = Split-Path -Parent ([System.IO.Path]::GetFullPath($Out))
if ($parent) { New-Item -ItemType Directory -Force -Path $parent | Out-Null }

if (-not ('DbgMon' -as [type])) {
    Add-Type @'
using System;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Text;
using System.Threading;

public static class DbgMon {
    public static int Run(int seconds, string outPath, string sessionId) {
        int count = 0;
        using (var bufferReady = new EventWaitHandle(false, EventResetMode.AutoReset, "DBWIN_BUFFER_READY"))
        using (var dataReady   = new EventWaitHandle(false, EventResetMode.AutoReset, "DBWIN_DATA_READY"))
        using (var mmf = MemoryMappedFile.CreateOrOpen("DBWIN_BUFFER", 4096))
        using (var view = mmf.CreateViewStream())
        using (var writer = new StreamWriter(outPath, true, new UTF8Encoding(false)))
        {
            var reader = new BinaryReader(view);
            writer.WriteLine("# SessionId: " + sessionId);
            writer.WriteLine("# Started: " + DateTimeOffset.Now.ToString("o"));
            writer.Flush();

            var deadline = DateTime.UtcNow.AddSeconds(seconds);
            bufferReady.Set();

            while (DateTime.UtcNow < deadline) {
                if (!dataReady.WaitOne(500)) continue;

                view.Seek(0, SeekOrigin.Begin);
                int pid = reader.ReadInt32();
                var bytes = new byte[4092];
                int read = view.Read(bytes, 0, bytes.Length);
                int len = Array.IndexOf(bytes, (byte)0, 0, read);
                if (len < 0) len = read;

                string msg = Encoding.Default.GetString(bytes, 0, len).TrimEnd('\r', '\n');
                if (msg.Length > 0) {
                    writer.WriteLine("[" + DateTimeOffset.Now.ToString("o") + "] [PID " + pid + "] " + msg);
                    writer.Flush();
                    count++;
                }

                bufferReady.Set();
            }
        }
        return count;
    }
}
'@
}

Write-Log -Level INFO -Event 'DEBUG_WATCH_START' -Message $Out -Data @{ seconds = $Seconds }

try {
    $count = [DbgMon]::Run($Seconds, $Out, $(if ($global:DemoSessionId) { $global:DemoSessionId } else { $SessionId }))
    Write-Log -Level SUCCESS -Event 'DEBUG_WATCH_COMPLETE' -Message $Out -Data @{ messages = $count }
}
catch {
    Write-LogException -Event 'DEBUG_WATCH_FAILED' -Exception $_.Exception -Message $Out
    throw
}
