# Post 3 — the Show Desktop / Aero Peek angle

| | |
| --- | --- |
| **Subreddit** | r/Windows11, or r/Windows if 02 already ran there |
| **Permission needed** | Likely yes in r/Windows11 — reuse the modmail from post 02 |
| **Post as** | Image/video post with GIF |
| **When** | Day 10, at least a week after post 02 |
| **Why this angle** | Targets a real grievance with steady search volume. Highest-intent title of the set — people search this one while actively annoyed. |

## Media

| Order | File | Shows | Alt text |
| --- | --- | --- | --- |
| 1 | `docs/media/trigger-hold.gif` (1.0 MB) | enter action, then a different action on leaving | A corner held to peek at the desktop, restoring the windows on leaving |

Raw: https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/trigger-hold.gif

> **Check the clip before posting.** `trigger-hold.gif` demonstrates the
> hold-to-peek *mechanism*, which is the point, but confirm it reads as
> "desktop revealed, then restored" to someone who has not seen the mod. If it
> does not, this post is worth a fresh 5-second capture bound to Show desktop on
> the bottom-right corner — that is the exact gesture the title promises, and
> the whole post rests on the GIF matching it.

## Title

```
Windows 11 removed the Show Desktop peek — I got it back with a hot corner
```

## Body

```
Windows 7 through 10 had that sliver in the bottom-right corner: hover, all your
windows go transparent, look at the desktop, move away, everything comes back.
Windows 11 kept a click-to-toggle version and dropped the hover-peek.

[GIF HERE]

That's a corner set to "hold to peek" — one action when the pointer arrives,
another when it leaves. Bottom-right corner, hold, peek, move away, back to
normal.

Honest caveat, because it matters: Windows 11 exposes no API for the real
transparent peek, so this is Win+D on the way in and Win+D on the way out. It's
a toggle wearing a peek's clothing — visually a swap rather than a fade. But the
*gesture* is back, which was the part I actually missed.

Same mechanism does other things — bottom-left for Start, an edge for snapping
the current window, a corner for Quick Settings.

It's a free MIT-licensed Windhawk mod I wrote, installs from Windhawk's Explore
tab: https://windhawk.net/mods/win-x-hotcorners

Did anyone ever find a way to get the genuine transparent peek back on 11? I
could not find one that survives an update.
```

## Notes

- **Keep the caveat.** Overselling this as real Aero Peek is the fastest way to
  get the top comment be "this isn't Aero Peek". Naming the limitation first
  turns that comment into agreement instead of a correction.
- The closing question is genuine and invites the exact people who know the most
  about this to reply, which is good for the thread.

## Likely replies, and what to say

**"That's not Aero Peek, that's just Win+D."**
> Correct, and the post says so — Windows 11 has no API for the transparent
> version. What is restored is the corner gesture, not the fade.

**"Win+D already exists, why do I need a corner for it?"**
> You don't, if the keyboard suits you. The point is having it under the pointer
> when your hand is already on the mouse, which is the same argument the original
> taskbar sliver was making.

**"There's a registry tweak / DisablePreviewDesktop for this."**
> Worth checking — that setting controls the click-to-toggle button's behaviour
> rather than bringing hover-peek back, at least on the builds I tested. If you
> have one that genuinely restores the hover fade I would like to see it.
