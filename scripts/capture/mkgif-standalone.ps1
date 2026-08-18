# Turn the recorded clips into README-sized GIFs.
#
# Colour notes, measured rather than guessed:
#   * The palette recipe was never the problem. 128-colour bayer vs 256-colour
#     undithered moved mean saturation by 0.2 points (18.1% -> 17.9%).
#   * The desktop itself is near-greyscale - dark browser chrome over a muted
#     photo measured 1.9% saturation - so a faithful capture looks flat.
#   * A modest eq lift is what actually helps: 17.9% -> 26.1% at
#     saturation 1.20, 29.7% at 1.35. 1.25 sits where the UI reads richer
#     without the wallpaper turning lurid.
#
# dither=none beats a dithered palette here because screen content is large
# flat areas of colour: dithering only adds noise, and noise costs GIF bytes.

param(
    [string]$Dir = $PSScriptRoot,
    [string]$Out = "$PSScriptRoot\gif",
    [int]$Width = 1200,
    [int]$Fps = 24,
    [double]$Start = 0.95,
    [double]$Length = 5.2
)

New-Item -ItemType Directory -Force -Path $Out | Out-Null

$vf = "eq=saturation=1.25:contrast=1.05," +
      "fps=$Fps,scale=${Width}:-1:flags=lanczos,split[a][b];" +
      "[a]palettegen=max_colors=256:stats_mode=full[p];" +
      "[b][p]paletteuse=dither=none"

foreach ($m in Get-ChildItem "$Dir\*.mkv" | Sort-Object Name) {
    $gif = Join-Path $Out "$($m.BaseName -replace '^\d\d-','').gif"
    & ffmpeg -hide_banner -loglevel error -ss $Start -t $Length -i $m.FullName `
        -filter_complex $vf -loop 0 -y $gif
    if (Test-Path $gif) {
        "{0,-24} {1,7:N2} MB" -f (Split-Path $gif -Leaf), ((Get-Item $gif).Length / 1MB)
    } else {
        "{0,-24} FAILED" -f $m.BaseName
    }
}
