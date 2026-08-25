# Post 2 — r/Windows11

| | |
| --- | --- |
| **Subreddit** | r/Windows11 (r/Windows works as a fallback) |
| **Permission needed** | **Yes, probably.** Modmail first — see below. |
| **Post as** | Image/video post with GIF, body in the text field |
| **When** | Day 3–5, only after a mod replies |
| **Why this angle** | Biggest audience, and the title is the highest-value search phrase we have. |

This is the SEO workhorse. The title contains **"Windows 11"** and **"hot
corners"** verbatim, because that is what people type.

## Send this modmail first

Recent Windhawk mod posts in r/Windows11 carry an auto-disclaimer saying
promotion was approved by the mods. I could not verify the rule directly —
Reddit blocks automated fetching — so **read the sub's rules yourself** and send
this before posting.

```
Subject: Permission to post a free open-source Windhawk mod I wrote

Hi — I wrote a free, open-source Windhawk mod that adds macOS-style hot corners
and screen edges to Windows 11, with per-monitor configuration. It was accepted
into the official Windhawk catalogue last week.

I saw that recent Windhawk mod posts here carry a "promotion approved by the
mods" note, so I wanted to ask first rather than assume. There is nothing paid
and nothing to sign up for — it is MIT licensed and installs through Windhawk.

Happy to post it in whatever format you prefer, or not at all if it is not a
fit. Thanks for your time.

Mod page: https://windhawk.net/mods/win-x-hotcorners
Source: https://github.com/DhakadG/win-x-hotcorners
```

## Media

Lead with Task View — it is the most universally understood payoff, and the
title promises hot corners so the first frame must show one working.

| Order | File | Shows | Alt text |
| --- | --- | --- | --- |
| 1 | `docs/media/task-view.gif` (2.2 MB) | top-left corner → Task View | Pointer moved into the top-left corner; Task View opens |
| 2 | `docs/media/dashboard.png` (35 KB) | Zones & settings window | The settings window, one tab per display, each zone's action shown in place |

Raw URLs:
- https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/task-view.gif
- https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/dashboard.png

If the sub only allows one image, drop the dashboard screenshot and put it in a
first comment instead — people ask "is it configurable?" within minutes, and
having the answer ready as an image is worth more than having it in the post.

## Title

```
Windows 11 still doesn't have hot corners, so I built them — free and open source
```

## Body

```
Every time I use a Mac I miss hot corners, and every time I come back to Windows
I re-discover that eleven versions in, it still cannot do it.

So: throw the pointer into the top-left corner, Task View opens.

[GIF HERE]

Any corner, any edge, and you pick what each one does — Task View, Start, snap
the window, virtual desktops, Quick Settings, lock, media keys, or just run a
program or a key combination you name yourself.

Two things I cared about that most hot-corner tools skip:

**It doesn't fire by accident.** A corner can wait for you to pause in it, or
need a double-knock, or only work with Ctrl held. Passing through on the way
somewhere else does nothing.

**Each monitor gets its own setup.** The same physical corner can do different
things on different screens, and it binds to the display by name — so
rearranging monitors doesn't shuffle your config.

It's free, MIT licensed, no account, nothing paid. It installs through Windhawk
(a Windows customisation platform) — search **Win-X Hot Corners** in its Explore
tab: https://windhawk.net/mods/win-x-hotcorners

I wrote it, so obviously I like it. What I'd actually like to know: which corner
would you bind first? I keep going back and forth on whether Task View or Show
Desktop deserves the top-left.
```

## Notes

- Say **"I wrote it"** plainly. Undisclosed self-promotion is what gets removed;
  disclosed self-promotion is usually fine and makes people generous.
- Explain what Windhawk is in one parenthetical. This audience is much broader
  than r/Windhawk and most of them have never heard of it.
- No feature matrix. The list of 38 actions belongs on the mod page.

## Likely replies, and what to say

**"Isn't this what PowerToys does?"**
> PowerToys has FancyZones, which is window *snapping* regions — you drag a
> window into them. This is the opposite direction: the pointer arriving
> somewhere triggers an action, and you never drag anything.

**"Windhawk needs admin / injects into Explorer, no thanks."**
> Fair concern, and worth being precise: this one is a *tool mod*, so it runs as
> its own small process and is not injected into Explorer or any other app.
> Source is MIT and on GitHub if you want to read what it does.

**"Does it work with multiple monitors of different scaling?"**
> Yes — that was one of the harder parts. It is per-monitor DPI aware, and zones
> bind to display names rather than screen coordinates so rearranging screens
> does not reshuffle anything.

**"Can it do the old Aero Peek?"**
> Partly, and there is a separate post about exactly that — a corner can run one
> action on arrival and another on leaving. The honest caveat is that Windows 11
> exposes no API for the real transparent peek, so it is Win+D both ways.
