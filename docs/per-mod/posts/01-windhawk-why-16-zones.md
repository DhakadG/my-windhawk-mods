# Post 1 — r/Windhawk

| | |
| --- | --- |
| **Subreddit** | r/Windhawk |
| **Permission needed** | No — home turf |
| **Post as** | Text post with one GIF |
| **When** | Day 0. Post this first; it is the safest room. |
| **Why this angle** | The release bot already announced 1.3.0 here. Re-announcing adds nothing. This posts what a bot cannot: the reasoning. |

## Media

One GIF. Do not add more — this post's value is the text, and a gallery buries it.

| Order | File | Shows | Alt text |
| --- | --- | --- | --- |
| 1 | `docs/media/hot-corners.gif` (445 KB) | bottom-left corner → Start menu | Pointer thrown into the bottom-left corner; the Start menu opens |

Local: `docs/media/hot-corners.gif`
Raw: https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/hot-corners.gif

Upload it to Reddit directly rather than linking. Place it after the "No global
mouse hook" paragraph, where the marker sits in the body.

## Title

```
Why Win-X Hot Corners ended up with 16 zones per display instead of 4
```

## Body

```
The bot posted the 1.3.0 release here a couple of days ago, so this is the other
half — why it looks the way it does, since a few of the decisions were not
obvious to me when I started.

I wanted Mac-style hot corners. Four corners, four actions, done. That lasted
about a week of actually using it.

**Corners are a tiny target; edges are a huge one.** Once you have thrown the
pointer at a corner a few hundred times you notice you are aiming, and aiming is
the thing hot corners were supposed to remove. So edges got included — but a
whole edge is too coarse, since the top edge of a maximised window is where the
title bar lives. Splitting each edge into three segments was the compromise that
survived: 4 corners + 4 edges × 3 = 16 zones. Give two neighbouring segments the
same action and they merge back into one wide zone, so you are not forced to
care about the split.

**The real problem is not triggering, it is *not* triggering.** A corner that
fires whenever the pointer passes through is unusable — you cross corners
constantly on the way to somewhere else. So a zone can require a dwell, or a
double-knock (leave and come straight back), or a held modifier, or any
combination. That is most of the complexity in the mod and all of the reason it
is usable.

**No global mouse hook.** It samples the cursor on its own thread instead. A
low-level hook sits in the input path of every application on the machine, games
included, and I did not want that trade for a convenience feature.

[GIF HERE]

It runs as a tool mod, so it is its own small process rather than injected into
Explorer.

Mod page: https://windhawk.net/mods/win-x-hotcorners
Source: https://github.com/DhakadG/win-x-hotcorners

The thing I am least sure about is the zone count — 16 is either exactly right or
twice what anyone needs, and I cannot tell from my own use. If you have installed
it, how many did you actually bind?
```

## Notes

- Ends on a real question. That is the engagement ask; do **not** add "please
  star the repo".
- This crowd knows what a tool mod is, so the last line about Explorer is a
  detail, not an explainer. Leave it short.

## Likely replies, and what to say

**"Why not just use AutoHotkey?"**
> You can, and for one corner it is less trouble. The parts that got tedious in
> AHK for me were multi-monitor (screen coordinates move when you rearrange
> displays) and per-monitor DPI. Those are most of what this handles.

**"Does it work on Windows 10?"**
> Yes. Windhawk supports 10 and 11, and the mod does not depend on anything
> 11-only.

**"Does it interfere with games?"**
> That was the reason for skipping the low-level mouse hook — it stays out of
> other applications' input path. There is also a fullscreen guard, so zones go
> inert while something is fullscreen.
