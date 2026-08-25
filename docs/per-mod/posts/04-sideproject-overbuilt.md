# Post 4 — r/SideProject

| | |
| --- | --- |
| **Subreddit** | r/SideProject (r/programming and r/coolgithubprojects are variants) |
| **Permission needed** | No |
| **Post as** | Text post with one GIF |
| **When** | Day 7 |
| **Why this angle** | Different audience entirely. They want the making-of and the lessons, not the feature list. Scope creep as a story is the hook. |

## Media

| Order | File | Shows | Alt text |
| --- | --- | --- | --- |
| 1 | `docs/media/hot-corners.gif` (445 KB) | bottom-left corner → Start menu | A corner triggering the Start menu |

Raw: https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/hot-corners.gif

Optional second, only if the sub renders galleries well:
`docs/media/dashboard.png` — proof it is genuinely configurable rather than a demo.

## Title

```
I wanted Mac hot corners on Windows. I ended up with a programmable perimeter around every monitor.
```

## Body

```
Started as a weekend thing. Four corners, four actions.

Then: corners are a small target, so add edges. Then: a whole edge is too
coarse, so split each into three. Then: it fires when you cross a corner by
accident, so add a dwell timer. Then a double-knock. Then a modifier key. Then:
I have two monitors and they should not do the same thing. Then: unplug a
monitor and the config reshuffles, so bind zones to display *names*.

That is 16 zones per display, 38 actions, five ways to trigger, and about 6,000
lines of C++ for something I originally described as "like the Mac thing".

[GIF HERE]

What I would tell past me:

- The interesting problem was never *detecting* the corner. It was **not**
  firing — a hot corner that triggers when you pass through is worse than no hot
  corner at all.
- Multi-monitor is where these tools quietly fall over. Screen coordinates move
  when you rearrange displays; display names do not.
- Per-monitor DPI will find every place you assumed a pixel is a pixel.
- I skipped the global mouse hook and sampled the cursor on a thread instead.
  Slightly more code, and it stays out of everyone else's input path.

It got accepted into the Windhawk catalogue last week, which was the goal. Free,
MIT: https://github.com/DhakadG/win-x-hotcorners
```

## Notes

- The scope-creep list is the hook. Do not tidy it into prose — the repetition of
  "Then:" is doing the work.
- Admitting 6,000 lines for "the Mac thing" is self-deprecating in the way this
  sub rewards. Leave it in.
- No install instructions. This audience clicks the repo, not the mod page.

## Likely replies, and what to say

**"Why C++ and not AutoHotkey / C#?"**
> Windhawk mods are C++ — that is the platform. It also meant no runtime to ship
> and no separate installer, since Windhawk handles distribution and updates.

**"6,000 lines seems like a lot."**
> It is, and roughly a third of it is the read-only settings dashboard. Whether
> that should exist at all is a fair question and one a reviewer raised too.

**"How do you test something that is all mouse position and timing?"**
> Mostly by building small standalone probes that exercise one Windows API and
> print what it actually returns, rather than trusting the documentation. Several
> assumptions did not survive that.

**"Did the AI write it?"**
> Answer this however you want, but answer it consistently — the question comes
> up on every project post now and a non-answer reads worse than either answer.
