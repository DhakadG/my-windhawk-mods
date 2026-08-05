# Checks for Win-x-HotCorners.cpp v3.1.0 — ResolveZone(), BuildZoneSet()
# geometry, and DetectTick(). No C++ toolchain here, so the branch-heavy logic
# is verified by simulation. Geometry is done in C# because PowerShell's
# implicit array coercion silently corrupts the arithmetic.
#
# Defaults to the tracked mod file so a fresh clone can run it; pass -Path to
# check the working copy before it has been copied over.

param([string]$Path = (Join-Path $PSScriptRoot '..\win-x-hotcorners.wh.cpp'))

$script:fails = 0
function Assert($cond, $msg) {
  if ($cond) { "  PASS  $msg" }
  else { $script:fails++; "  FAIL  $msg" }
}

# ---- ResolveZone: name match > legacy ordinal > wildcard, resolved per zone ----
function ResolveZone($configs, $mon, $zone) {
  foreach ($c in $configs) {
    if (-not $c.monitorId -or $c.monitorId -eq '*') { continue }
    if ($c.monitorId.ToLower() -ne $mon.id.ToLower()) { continue }
    if ($c.zones[$zone]) { return $c.zones[$zone] }
  }
  foreach ($c in $configs) {
    if ($c.monitorId -or $c.monitorIndex -ne $mon.index) { continue }
    if ($c.zones[$zone]) { return $c.zones[$zone] }
  }
  foreach ($c in $configs) {
    $isWild = ($c.monitorId -eq '*') -or ((-not $c.monitorId) -and $c.monitorIndex -eq 0)
    if (-not $isWild) { continue }
    if ($c.zones[$zone]) { return $c.zones[$zone] }
  }
  return $null
}
function Cfg($id, $idx, $zoneMap) {
  $z = @($null) * 12   # ZONE_COUNT
  foreach ($k in $zoneMap.Keys) { $z[$k] = $zoneMap[$k] }
  return @{ monitorId = $id; monitorIndex = $idx; zones = $z }
}

$TL = 0; $BR = 3
$monA = @{ id = 'Dell U2720Q'; index = 1 }
$monB = @{ id = 'BOE0998';     index = 2 }

'--- ResolveZone ---'
$c = @( (Cfg '*' 0 @{ $TL = 'TaskView'; $BR = 'ShowDesktop' }) )
Assert ((ResolveZone $c $monA $TL) -eq 'TaskView') 'wildcard covers monitor A'
Assert ((ResolveZone $c $monB $TL) -eq 'TaskView') 'wildcard covers monitor B'

$c = @( (Cfg 'BOE0998' 0 @{ $TL = 'Lock' }), (Cfg '*' 0 @{ $TL = 'TaskView'; $BR = 'ShowDesktop' }) )
Assert ((ResolveZone $c $monB $TL) -eq 'Lock')        'name match beats wildcard'
Assert ((ResolveZone $c $monB $BR) -eq 'ShowDesktop') 'wildcard still fills undefined zone (per-zone)'
Assert ((ResolveZone $c $monA $TL) -eq 'TaskView')    'other monitor unaffected by name match'

$c = @( (Cfg 'boe0998' 0 @{ $TL = 'Sleep' }) )
Assert ((ResolveZone $c $monB $TL) -eq 'Sleep') 'name match is case-insensitive'

$c = @( (Cfg '' 2 @{ $TL = 'Mute' }) )
Assert ((ResolveZone $c $monB $TL) -eq 'Mute') 'legacy ordinal used when name empty (v2.x config)'
Assert ((ResolveZone $c $monA $TL) -eq $null)  'legacy ordinal does not leak to other monitor'

$dupA = @{ id = 'Dell U2720Q';    index = 1 }
$dupB = @{ id = 'Dell U2720Q #2'; index = 2 }
$c = @( (Cfg 'Dell U2720Q' 0 @{ $TL = 'A' }), (Cfg 'Dell U2720Q #2' 0 @{ $TL = 'B' }) )
Assert ((ResolveZone $c $dupA $TL) -eq 'A') 'identical models: first stays distinct'
Assert ((ResolveZone $c $dupB $TL) -eq 'B') 'identical models: #2 suffix resolves separately'
Assert ((ResolveZone @() $monA $TL) -eq $null) 'no config -> no zone'

# ---- Zone geometry: all 8 zones pairwise disjoint and non-empty ----
$geoSrc = @'
using System;
public class Geo {
  public static string Check(int L,int T,int R,int B,int csCfg,int esCfg){
    int span = Math.Min(R-L, B-T);
    int cs = Math.Min(csCfg, span/2); if (cs < 1) cs = 1;
    int es = Math.Min(esCfg, cs);
    string[] names = {"TL","TR","BL","BR","ET","EB","EL","ER"};
    int[][] z = new int[8][];
    z[0]=new[]{L,      T,      L+cs,  T+cs};
    z[1]=new[]{R-cs,   T,      R,     T+cs};
    z[2]=new[]{L,      B-cs,   L+cs,  B};
    z[3]=new[]{R-cs,   B-cs,   R,     B};
    z[4]=(L+cs < R-cs)? new[]{L+cs, T,    R-cs, T+es} : null;
    z[5]=(L+cs < R-cs)? new[]{L+cs, B-es, R-cs, B}    : null;
    z[6]=(T+cs < B-cs)? new[]{L,    T+cs, L+es, B-cs} : null;
    z[7]=(T+cs < B-cs)? new[]{R-es, T+cs, R,    B-cs} : null;
    string bad=""; int built=0;
    for(int i=0;i<8;i++){
      if(z[i]==null) continue; built++;
      if(z[i][2]<=z[i][0] || z[i][3]<=z[i][1]) bad += " EMPTY:"+names[i];
      for(int j=i+1;j<8;j++){
        if(z[j]==null) continue;
        if(z[i][0]<z[j][2] && z[j][0]<z[i][2] && z[i][1]<z[j][3] && z[j][1]<z[i][3])
          bad += " OVERLAP:"+names[i]+"/"+names[j];
      }
    }
    return (bad==""? "OK" : "BAD"+bad) + "  (built="+built+" cs="+cs+" es="+es+")";
  }
}
'@
if (-not ('Geo' -as [type])) { Add-Type -TypeDefinition $geoSrc }
''
'--- Zone geometry (pairwise disjoint) ---'
$geoCases = @(
  @{n='4K real cfg';        a=@(0,0,3840,2160,4,2)}
  @{n='4K tiny';            a=@(0,0,3840,2160,2,1)}
  @{n='4K equal sizes';     a=@(0,0,3840,2160,6,6)}
  @{n='edge >> corner';     a=@(0,0,3840,2160,2,50)}   # the latent overlap bug
  @{n='corner > screen';    a=@(0,0,3840,2160,5000,20)}
  @{n='1080p';              a=@(0,0,1920,1080,20,20)}
  @{n='2nd monitor offset'; a=@(3840,0,5760,1080,10,3)}
  @{n='tiny screen';        a=@(0,0,800,600,300,300)}
  @{n='negative coords';    a=@(-1920,0,0,1080,6,6)}
)
foreach ($g in $geoCases) {
  $r = [Geo]::Check($g.a[0],$g.a[1],$g.a[2],$g.a[3],$g.a[4],$g.a[5])
  # "no overlaps" is trivially true when nothing was built, so the count has
  # to be part of the assertion or a case that builds no zones passes
  $built = if ($r -match 'built=(\d+)') { [int]$matches[1] } else { -1 }
  Assert (($r -like 'OK*') -and $built -gt 0) ("{0,-20} {1}" -f $g.n, $r)
}

# ---- ParseKeyCombo: semicolons separate combos, they are NOT merged ----
# Regression for the flattening bug CodeRabbit caught: "Ctrl+C;Alt+Tab" used to
# become one Ctrl+Alt+C+Tab chord instead of two sequential keystrokes.
$mods = @{ 'CTRL'=0xA2; 'ALT'=0xA4; 'SHIFT'=0xA0; 'WIN'=0x5B }
$vks  = @{ 'A'=0x41; 'C'=0x43; 'L'=0x4C; 'TAB'=0x09; 'ESC'=0x1B; 'F4'=0x73; 'HOME'=0x24 }
function ParseKeyCombo([string]$spec) {
  $out = @()
  foreach ($combo in ($spec -split ';')) {
    $combo = $combo.Trim(); if (-not $combo) { continue }
    $m = @(); $main = 0
    foreach ($tok in ($combo -split '\+')) {
      $t = $tok.Trim().ToUpper(); if (-not $t) { continue }
      if ($mods.ContainsKey($t)) { $m += $mods[$t] }
      elseif ($vks.ContainsKey($t)) { $main = $vks[$t] }
    }
    $keys = @($m); if ($main) { $keys += $main }
    if ($keys.Count) { $out += ,$keys }
  }
  return ,$out
}
''
'--- ParseKeyCombo ---'
$r = ParseKeyCombo 'Ctrl+Shift+Esc'
Assert ($r.Count -eq 1 -and $r[0].Count -eq 3) "single combo -> 1 chord of 3 keys (got $($r.Count)/$($r[0].Count))"

$r = ParseKeyCombo 'Ctrl+C;Alt+Tab'
Assert ($r.Count -eq 2) "two combos stay separate, NOT merged (got $($r.Count))"
Assert ($r[0].Count -eq 2 -and $r[1].Count -eq 2) "each combo keeps its own keys ($($r[0].Count),$($r[1].Count))"
Assert ($r[0][0] -eq 0xA2 -and $r[0][1] -eq 0x43) 'first combo is Ctrl+C'
Assert ($r[1][0] -eq 0xA4 -and $r[1][1] -eq 0x09) 'second combo is Alt+Tab'

$r = ParseKeyCombo 'Win+L'
Assert ($r.Count -eq 1 -and $r[0][0] -eq 0x5B -and $r[0][1] -eq 0x4C) 'Win+L parses to LWIN then L'

$r = ParseKeyCombo '  Alt+F4  ;  ; Win+Home '
Assert ($r.Count -eq 2) "whitespace and empty segments ignored (got $($r.Count))"

$r = ParseKeyCombo 'Nonsense+Garbage'
Assert ($r.Count -eq 0) 'unrecognised tokens produce no combo (executor is refused)'

$r = ParseKeyCombo ''
Assert ($r.Count -eq 0) 'empty string produces no combo'

# ---- ActionStartProcess: unquoted paths containing spaces ----
$pathSrc = @'
using System;using System.Runtime.InteropServices;using System.IO;using System.Text;
public class PathSplit {
 [DllImport("shell32.dll",SetLastError=true,CharSet=CharSet.Unicode)]
 static extern IntPtr CommandLineToArgvW(string cmd,out int argc);
 [DllImport("kernel32.dll")] static extern IntPtr LocalFree(IntPtr p);
 static bool Exists(string p){ try { return File.Exists(p) || Directory.Exists(p); } catch { return false; } }
 public static string Exe(string cmd){ return Go(cmd,true); }
 public static string Args(string cmd){ return Go(cmd,false); }
 static string Go(string cmd,bool wantExe){
   int argc; IntPtr pv = CommandLineToArgvW(cmd, out argc);
   if(pv==IntPtr.Zero||argc<1) return wantExe?cmd:"";
   string[] argv=new string[argc];
   for(int i=0;i<argc;i++) argv[i]=Marshal.PtrToStringUni(Marshal.ReadIntPtr(pv,i*IntPtr.Size));
   LocalFree(pv);
   string exe=argv[0]; int firstArg=1;
   if(argc>1 && !Exists(exe)){
     string joined=exe;
     for(int i=1;i<argc;i++){
       joined += " "+argv[i];
       if(Exists(joined)){ exe=joined; firstArg=i+1; break; }
     }
   }
   if(wantExe) return exe;
   var sb=new StringBuilder();
   for(int i=firstArg;i<argc;i++){ if(sb.Length>0) sb.Append(' ');
     if(argv[i].Contains(" ")) sb.Append('"').Append(argv[i]).Append('"'); else sb.Append(argv[i]); }
   return sb.ToString();
 }
}
'@
if (-not ('PathSplit' -as [type])) { Add-Type -TypeDefinition $pathSrc }
''
'--- ActionStartProcess path splitting ---'
# The token-gluing heuristic only matters for a path containing spaces, and it
# decides by asking the filesystem whether the glued path exists. Pointing that
# at an installed Windhawk made the whole group SKIP on any machine without one
# - a silent hole exactly where the logic is hardest. Build the file instead.
$spaceDir = Join-Path ([IO.Path]::GetTempPath()) 'hot corners check'
$wh = Join-Path $spaceDir 'windhawk.exe'
try {
  New-Item -ItemType Directory -Force -Path $spaceDir | Out-Null
  if (-not (Test-Path -LiteralPath $wh)) { New-Item -ItemType File -Path $wh | Out-Null }
  Assert ([PathSplit]::Exe($wh) -eq $wh) 'unquoted path with spaces, no args'
  Assert ([PathSplit]::Exe("$wh -tray-only") -eq $wh) 'unquoted path with spaces + arg -> exe'
  Assert ([PathSplit]::Args("$wh -tray-only") -eq '-tray-only') 'unquoted path with spaces + arg -> params'
  Assert ([PathSplit]::Exe("`"$wh`" -tray-only") -eq $wh) 'quoted path still works'
} finally {
  Remove-Item -LiteralPath $spaceDir -Recurse -Force -ErrorAction SilentlyContinue
}
Assert ([PathSplit]::Exe('C:\Windows\System32\cmd.exe /c echo hi') -eq 'C:\Windows\System32\cmd.exe') 'path without spaces unaffected'
Assert ([PathSplit]::Args('C:\Windows\System32\cmd.exe /c echo hi') -eq '/c echo hi') 'args preserved verbatim'
Assert ([PathSplit]::Exe('notepad.exe') -eq 'notepad.exe') 'bare command left for the shell to resolve via PATH'
Assert ([PathSplit]::Exe('https://example.com') -eq 'https://example.com') 'URL passes through untouched'

# ---- DetectTick state machine ----
function RunTicks($steps, $delay, $cooldown, $dragGuard, $settle = 0, $globalFloor = 0, $knockMs = 0, $reqMod = $false) {
  $active = -1; $enter = 0; $fired = $false
  $lastFire = @{}; $fires = @(); $lastAny = 0
  $lastExit = @{}; $knockOk = $true
  $t = 0
  foreach ($s in $steps) {
    $t += 16
    $idx = $s.idx
    if ($idx -ne $active) {
      if ($active -ge 0) { $lastExit[$active] = $t }
      $active = $idx; $enter = $t; $fired = $false
      $knockOk = ($knockMs -le 0) -or ($idx -lt 0) -or
                 ($lastExit.ContainsKey($idx) -and ($t - $lastExit[$idx]) -le $knockMs)
    }
    if ($idx -lt 0 -or $fired) { continue }
    if (-not $knockOk) { continue }
    if ($reqMod -and -not $s.mod) { continue }
    if ($dragGuard -and $s.drag) { $fired = $true; continue }
    $dwell = [Math]::Max($delay, $settle)
    if ($dwell -gt 0 -and ($t - $enter) -lt $dwell) { continue }
    if ($cooldown -gt 0 -and $lastFire.ContainsKey($idx)) {
      if (($t - $lastFire[$idx]) -lt $cooldown) { $fired = $true; continue }
    }
    if ($globalFloor -gt 0 -and $lastAny -ne 0 -and ($t - $lastAny) -lt $globalFloor) {
      $fired = $true; continue
    }
    $fired = $true; $lastAny = $t; $lastFire[$idx] = $t; $fires += $t
  }
  return $fires
}
function Steps($spec) { $spec | ForEach-Object { @{ idx = $_[0]; drag = [bool]$_[1]; mod = [bool]$_[2] } } }
function Dwell($idx, $n) { 1..$n | ForEach-Object { @{ idx = $idx; drag = $false; mod = $false } } }
function DwellMod($idx, $n) { 1..$n | ForEach-Object { @{ idx = $idx; drag = $false; mod = $true } } }

''
'--- DetectTick ---'
$f = RunTicks (Dwell 0 10) 0 0 $true
Assert ($f.Count -eq 1) "dwelling in a zone fires exactly once (got $($f.Count))"

$f = RunTicks (Steps @(@(0,0),@(0,0),@(-1,0),@(-1,0),@(0,0),@(0,0))) 0 0 $true
Assert ($f.Count -eq 2) "leaving and re-entering re-arms (got $($f.Count))"

$f = RunTicks (Steps @(@(0,0),@(-1,0),@(0,0),@(-1,0),@(0,0))) 0 300 $true
Assert ($f.Count -eq 1) "cooldown blocks rapid re-entry (got $($f.Count))"

$f = RunTicks (Dwell 0 20) 200 0 $true
Assert ($f.Count -eq 1 -and $f[0] -ge 200) "dwell delay: fires once, not before 200ms (at $($f -join ','))"

$f = RunTicks (Steps @(@(0,0),@(0,0),@(0,0))) 200 0 $true
Assert ($f.Count -eq 0) 'dwell delay: leaving before the delay never fires'

$f = RunTicks (Steps @(@(0,1),@(0,1),@(0,0),@(0,0))) 0 0 $true
Assert ($f.Count -eq 0) 'drag suppresses the whole visit, incl. after button release'

$f = RunTicks (Steps @(@(0,1),@(-1,0),@(0,0))) 0 0 $true
Assert ($f.Count -eq 1) 'after a suppressed drag visit, a fresh entry still fires'

$f = RunTicks (Steps @(@(0,1),@(0,1))) 0 0 $false
Assert ($f.Count -eq 1) 'drag guard disabled -> fires while dragging'

# Regression: reaching a corner means crossing the edge strip beside it.
# With settle=0 the transit fires BOTH and a toggle action cancels itself out
# — the v3.0.1 bug that made the mod look completely dead.
$transit = Steps (@(@(1,0),@(1,0)) + (1..12 | ForEach-Object { ,@(0,0) }))
$f = RunTicks $transit 0 0 $true 0
Assert ($f.Count -eq 2) "settle=0: fast transit fires edge AND corner (the bug, got $($f.Count))"

$f = RunTicks $transit 0 0 $true 80
Assert ($f.Count -eq 1) "settle=80: transit fires only the zone rested in (got $($f.Count))"

$deliberate = Steps ((1..10 | ForEach-Object { ,@(1,0) }) + (1..10 | ForEach-Object { ,@(0,0) }))
$f = RunTicks $deliberate 0 0 $true 80
Assert ($f.Count -eq 2) "settle=80: resting in each of two zones still fires both (got $($f.Count))"

$f = RunTicks (Dwell 0 10) 0 0 $true 80
Assert ($f.Count -eq 1) "settle=80: normal dwell still fires exactly once (got $($f.Count))"

# Retrigger: leaving and returning must work once cooldown elapsed.
$retrigger = Steps ((1..8 | ForEach-Object { ,@(0,0) }) + (1..25 | ForEach-Object { ,@(-1,0) }) + (1..8 | ForEach-Object { ,@(0,0) }))
$f = RunTicks $retrigger 0 300 $true 80
Assert ($f.Count -eq 2) "retrigger after leaving + cooldown works (got $($f.Count))"

# Global rate floor (kMinFireIntervalMs). Sweeping across many DIFFERENT zones
# defeats the per-zone cooldown entirely — that is how four Win+Tab injections
# landed in 40ms and made the shell stagger.
$sweepSpec = @()
foreach ($zz in 0..9) { foreach ($kk in 1..6) { $sweepSpec += ,@($zz,0) } }
$sweep = Steps $sweepSpec
$f = RunTicks $sweep 0 300 $true 80 0
$noFloor = $f.Count
$f = RunTicks $sweep 0 300 $true 80 250
Assert ($noFloor -gt $f.Count) "global floor throttles a multi-zone sweep ($noFloor -> $($f.Count) fires)"
$minGap = [int]::MaxValue
for ($i=1; $i -lt $f.Count; $i++) { $g = $f[$i] - $f[$i-1]; if ($g -lt $minGap) { $minGap = $g } }
Assert ($f.Count -lt 2 -or $minGap -ge 250) "no two actions closer than 250ms (min gap $minGap)"

$f = RunTicks (Dwell 0 10) 0 300 $true 80 250
Assert ($f.Count -eq 1) "global floor does not block a normal single trigger (got $($f.Count))"

''
'--- Knock to activate (#1) ---'
# leave the zone, come back quickly = knock. Two entries inside the window.
$knock = Steps (@(@(0,0,0),@(0,0,0)) + @(@(-1,0,0)) + @(@(0,0,0),@(0,0,0),@(0,0,0),@(0,0,0),@(0,0,0),@(0,0,0)))
$f = RunTicks $knock 0 0 $true 80 0 400
Assert ($f.Count -eq 1) "knock: leave and quickly return fires once (got $($f.Count))"

# a single uninterrupted visit must never fire in knock mode
$f = RunTicks (Dwell 0 30) 0 0 $true 80 0 400
Assert ($f.Count -eq 0) "knock: a single entry never fires (got $($f.Count))"

# returning too late is not a knock
$slow = Steps (@(@(0,0,0)) + (1..40 | ForEach-Object { ,@(-1,0,0) }) + (1..8 | ForEach-Object { ,@(0,0,0) }))
$f = RunTicks $slow 0 0 $true 80 0 400
Assert ($f.Count -eq 0) "knock: returning after the window does not fire (got $($f.Count))"

# knock disabled (0) must behave exactly as before
$f = RunTicks (Dwell 0 10) 0 0 $true 80 0 0
Assert ($f.Count -eq 1) "knock off: unchanged single-entry behaviour (got $($f.Count))"

''
'--- Require a modifier (#2) ---'
$f = RunTicks (Dwell 0 10) 0 0 $true 80 0 0 $true
Assert ($f.Count -eq 0) "modifier required but not held -> never fires (got $($f.Count))"

$f = RunTicks (DwellMod 0 10) 0 0 $true 80 0 0 $true
Assert ($f.Count -eq 1) "modifier required and held -> fires once (got $($f.Count))"

# modifier pressed after the cursor is already parked must still arm the zone
$late = Steps ((1..6 | ForEach-Object { ,@(0,0,0) }) + (1..6 | ForEach-Object { ,@(0,0,1) }))
$f = RunTicks $late 0 0 $true 80 0 0 $true
Assert ($f.Count -eq 1) "modifier pressed while already in the zone still fires (got $($f.Count))"

$f = RunTicks (Dwell 0 10) 0 0 $true 80 0 0 $false
Assert ($f.Count -eq 1) "modifier not required -> unchanged behaviour (got $($f.Count))"
''
'--- Dashboard layout geometry (no overlaps) ---'
# Parsed straight out of the source so the test cannot drift from the code.
$modSrc = [IO.File]::ReadAllText($Path)
# A miss returns '', and [int]'' is 0 in PowerShell - so a renamed constant
# would quietly zero every geometry assertion below instead of failing.
function K($n){
  $m = [regex]::Match($modSrc, "constexpr (?:int|DWORD|LONG) $n = (\d+);")
  if (-not $m.Success) { throw "constant '$n' not found in the source" }
  [int]$m.Groups[1].Value
}
$Pad=K 'Pad'; $Gap=K 'Gap'; $RowH=K 'RowH'; $CheckH=K 'CheckH'; $TabH=K 'TabH'
$CtlH=K 'CtlH'; $BtnH=K 'BtnH'; $LblW=K 'LblW'; $CmbW=K 'CmbW'; $ArgW=K 'ArgW'
$OptLblW=K 'OptLblW'; $OptCtlW=K 'OptCtlW'; $DiagW=K 'DiagW'; $DiagH=K 'DiagH'
$ZC = 12
Assert ($Pad -gt 0 -and $RowH -gt 0 -and $DiagW -gt 0) "layout constants parsed from source (pad=$Pad row=$RowH)"

$leftW   = $Pad+$LblW+$Gap+$CmbW+$Gap+$ArgW
$clientW = $leftW+$Gap+$DiagW+$Pad
$HdrH=K 'HdrH'; $TzRowH=K 'TzRowH'
$tzPanel = 14+20+20+6*$TzRowH
$zonesLeft  = $Pad+$TabH+$Gap+$CtlH+$Gap+$HdrH+$ZC*$RowH
$zonesRight = $Pad+$TabH+$Gap+$CtlH+$Gap+$DiagH+$tzPanel
$zonesH  = [Math]::Max($zonesLeft,$zonesRight)
# Counted out of kOpts rather than hardcoded: an option added without resizing
# the window puts the last row under the button bar, which is exactly what this
# assertion exists to catch.
$SecH = K 'SecH'
$optDefs = [regex]::Matches($modSrc, '(?m)^\s*\{IDC_\w+, L"[^"]*", (true|false),')
if ($optDefs.Count -eq 0) { throw "kOpts entries not found in the source" }
$nCheck = @($optDefs | Where-Object { $_.Groups[1].Value -eq 'true' }).Count
$nEdit  = $optDefs.Count - $nCheck
$nSec   = ([regex]::Matches($modSrc, '(?m)^\s*\{IDC_\w+, L"[^"]*", (?:true|false), L"[^"]*", L"')).Count
# 0 would silently shrink $optH and let the height assertions pass without ever
# accounting for the group headings.
if ($nSec -eq 0) { throw "no kOpts section headings matched - the regex has drifted" }
$optH    = $Pad+$TabH+$Gap+$nEdit*$RowH+$nCheck*$CheckH+$nSec*$SecH
$content = [Math]::Max($zonesH,$optH)
$clientH = $content+$Gap*2+$BtnH+$Pad
$btnTop  = $clientH-$Pad-$BtnH

Assert ($zonesLeft -le $btnTop)  "zone rows end ($zonesLeft) above the button bar ($btnTop)"
Assert ($zonesRight -le $btnTop) "right column ends ($zonesRight) above the button bar ($btnTop)"
Assert ($optH   -le $btnTop)  "options page ends ($optH) above the button bar ($btnTop)"
$diagBottom = $Pad+$TabH+$Gap+$CtlH+$Gap+$DiagH+$tzPanel
Assert ($diagBottom -le $btnTop) "preview + per-zone panel ($diagBottom) clears the button bar"
Assert (($leftW+$Gap+$DiagW+$Pad) -le $clientW) "preview fits to the right of the rows"
Assert (($Pad+130+$Gap+90+$Gap+210+$Pad) -le $clientW) "three buttons fit across the window"
Assert (($Pad+$OptLblW+$Gap+$OptCtlW+$DiagW+$Pad) -le $clientW) "widest options field stays inside the window"
Assert ($ArgW -ge 120) "args field wide enough to be usable ($ArgW px)"

''
'--- The source still has the shape these simulations assume ---'
# The three groups below model algorithms rather than run the C++, so on their
# own they would keep passing after the C++ stopped doing what they model.
# These assertions are the tether: each one fails if the property under test is
# edited out of the source.
function Src($pattern, $what) {
  Assert ([regex]::IsMatch($modSrc, $pattern)) $what
}
Src 'fsMon == kAllMonitors \|\| fsMon == job\.monitor' `
    'the fullscreen guard compares the zone monitor, with an all-monitors sentinel'
Src 'HMONITOR monitor = nullptr;' `
    'HitZone still carries the monitor the guard compares against'
# Both halves must not merely have an executor - they must be handed the SAME
# one. Checking only that the cache is filled would still pass if the
# assignment were changed back to a fresh MakeExecutor per half.
Src '(?s)std::function<void\(\)> altExec\[ZONE_COUNT\];.*?if \(!altExec\[z\]\)\s*\n\s*altExec\[z\] = MakeExecutor[^;]*;\s*\n\s*hz\.exec = altExec\[z\];' `
    'a split edge hands both halves one executor, so they share its A/B state'

''
'--- Poll interval (#4) ---'
# There is deliberately no adaptive backoff. Any rule for easing off decides
# from a sample taken BEFORE the user starts moving, so a flick can cross a
# zone between two samples - and on a corner shared with another monitor there
# is no screen edge to stop the pointer, so that is a lost trigger rather than
# a late one. Two versions of that idea have already been written and removed;
# these assertions exist to stop a third.
$kTick = K 'kTickMs'; $kIdle = K 'kIdleTickMs'
Assert ($kTick -lt $kIdle) 'the idle rate is genuinely slower than the full rate'

$dt = [regex]::Match($modSrc, '(?s)static DWORD DetectTick\(\)\s*\{.*?\n\}').Value
Assert ($dt.Length -gt 0) 'DetectTick body located in the source'

# Every idle return must sit in the head of the function, before the hit test -
# those are the paths where nothing can fire at all.
$hitTest = $dt.IndexOf('int idx = -1;')
$idleAt = @([regex]::Matches($dt, 'return kIdleTickMs;') | ForEach-Object { $_.Index })
Assert ($hitTest -gt 0) 'the hit test marks where "nothing can fire" ends'
Assert ($idleAt.Count -eq 3) "the three no-work paths return the idle rate (found $($idleAt.Count))"
Assert (($idleAt | Where-Object { $_ -gt $hitTest }).Count -eq 0) `
       'no idle return survives past the hit test'

# ...and what it returns once a zone could fire is the flat rate, nothing else.
Assert ([regex]::IsMatch($dt, 'const DWORD next = kTickMs;')) `
       'the interval after the hit test is the flat full rate'
Assert (-not [regex]::IsMatch($dt, 'nearest|kNearPx|lastPt')) `
       'no distance or movement heuristic has crept back into the interval'
# Those three returns are the only place the idle rate may appear at all, so a
# backoff smuggled in as `next = cond ? kIdleTickMs : kTickMs` cannot slip past.
$idleUses = ([regex]::Matches($dt, 'kIdleTickMs')).Count
Assert ($idleUses -eq $idleAt.Count) `
       "the idle rate appears only in those returns (used $idleUses times, $($idleAt.Count) returns)"

''
'--- Per-monitor fullscreen guard (#5) ---'
# 0 = nothing fullscreen (nullptr), -1 = kAllMonitors, 1/2 = a real HMONITOR.
function FsSkip($fsMon, $zoneMon) {
  return ($fsMon -ne 0) -and (($fsMon -eq -1) -or ($fsMon -eq $zoneMon))
}
Assert (-not (FsSkip 0 1)) 'nothing fullscreen -> nothing is suppressed'
Assert (FsSkip 1 1)        'fullscreen app suppresses zones on its own display'
Assert (-not (FsSkip 1 2)) 'a game on display 1 leaves display 2 working'
Assert (FsSkip -1 1)       'unknown display -> suppress everywhere (the old behaviour)'
Assert (FsSkip -1 2)       'unknown display -> suppress everywhere, both displays'

''
'--- Split-edge alternation (#6) ---'
# An edge with a centre zone becomes two rectangles, but it is still one edge.
# Left half, right half, left half must give A, B, A - which only happens if
# both halves share one flip flag.
function EdgeFireSeq([bool]$shared) {
  $cache = @{}
  $halves = @()
  foreach ($h in 1, 2) {
    if ($shared) {
      if (-not $cache.ContainsKey('EDGE_TOP')) { $cache['EDGE_TOP'] = @{ n = 0 } }
      $halves += $cache['EDGE_TOP']
    } else {
      $halves += @{ n = 0 }
    }
  }
  $seq = ''
  foreach ($i in 0, 1, 0) {
    $st = $halves[$i]
    $seq += @('A', 'B')[$st.n % 2]
    $st.n++
  }
  return $seq
}
Assert ((EdgeFireSeq $true) -eq 'ABA') "one flag per edge alternates correctly (got $(EdgeFireSeq $true))"
Assert ((EdgeFireSeq $false) -eq 'AAB') "a flag per half does not (got $(EdgeFireSeq $false)) - this is the bug that was fixed"

''
if ($script:fails -eq 0) { 'ALL CHECKS PASSED'; exit 0 }
else { "$($script:fails) CHECK(S) FAILED"; exit 1 }
