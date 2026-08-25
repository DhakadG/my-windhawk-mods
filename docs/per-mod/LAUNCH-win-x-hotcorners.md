# Win-X Hot Corners — launch and reach kit

Goal is **reach and search visibility**, not revenue. The win condition is that
someone typing *"does Windows 11 have hot corners"* into Google lands on
something we wrote.

Status as of 2026-08-25: mod is **live in the catalogue** at
<https://windhawk.net/mods/win-x-hotcorners>, PR
[#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001) merged 22 Aug.

---

## The one thing that matters most

Reddit threads and GitHub repos rank in Google for long-tail questions. Almost
nobody searches *"Win-X Hot Corners"* — they search the **problem**:

| What people actually type | Currently ranked by |
|---|---|
| does windows 11 have hot corners | an old Reddit thread |
| mac hot corners for windows | an old Reddit thread |
| windows 11 show desktop button missing | an old Reddit thread |
| aero peek windows 11 | an old Reddit thread |
| hot corners windows 10 | an old Reddit thread |

So the **title is the SEO surface**, not the body. Every title below is written
to contain a phrase somebody would actually type. Resist clever titles — a
clever title ranks for nothing.

### The highest-ROI action is not posting

It is **answering the threads that already rank**. Search
`site:reddit.com windows 11 hot corners` and similar, find the older threads
where someone asked and the top answer is "you can't" or a dead AutoHotkey
script, and leave one genuinely useful reply. Those threads already have the
Google ranking a new post has to earn from zero.

Rules for those replies, or it reads as spam and gets removed:

- Answer the question first, in full, including the part the mod does not solve.
- Mention the mod once, plainly, and disclose that you wrote it.
- Never reply to an old thread with nothing but a link.
- One per day at most, spread across days.

---

## Where the assets are

All in `docs/media/`, mirrored in the standalone repo. Reuse as-is; a
higher-resolution pass can come later.

| Asset | Shows | Best used for |
|---|---|---|
| `hot-corners.gif` | bottom-left → Start menu | hero, any post |
| `task-view.gif` | top-left → Task View | the r/Windows11 post |
| `start-menu.gif` | corner → Start | alternates with hero |
| `snap-left.gif` / `snap-right.gif` | snap active window | "actions" posts |
| `quick-settings.gif` | corner → Quick Settings | "actions" posts |
| `switch-window.gif` | corner → last window | "actions" posts |
| `trigger-arrival.gif` | fires on arrival | trigger-model post |
| `trigger-dwell.gif` | fires after a pause | accidental-trigger post |
| `trigger-knock.gif` | leave and come back | accidental-trigger post |
| `trigger-hold.gif` | enter action + leave action | Aero Peek post |
| `trigger-modifier.gif` | inert unless Ctrl held | accidental-trigger post |
| `dashboard.png` | Zones & settings window | any post needing proof of config |

**Not yet captured**, tracked in
[win-x-hotcorners#11](https://github.com/DhakadG/win-x-hotcorners/issues/11):
multi-monitor (same corner, two displays, different actions) and the monitor
list in the log. The multi-monitor clip is the single most valuable missing
asset — it is the mod's strongest differentiator and nothing currently shows it.

Upload GIFs **directly to Reddit** rather than linking imgur. Native media gets
materially better reach, and a bare external link reads as promo.

---

## Before posting anywhere: the rules problem

I could not verify subreddit rules directly — Reddit blocks automated fetching —
so **check each sub's rules yourself before posting**. What is believed:

- **r/Windows11 appears to require moderator permission** for promoting your own
  software; recent Windhawk mod posts there carry an auto-disclaimer saying
  permission was obtained. **Modmail first.** Draft below.
- **r/Windhawk** is home turf and already has a bot-generated release post. A
  second announcement adds nothing — post what the bot cannot.
- Most Windows subs enforce some version of the 90/10 rule. If the account has
  no history in a sub, build some before dropping a project post into it.

Disclose authorship in every post. "I made this" is not a weakness on Reddit —
it is what makes the post allowed and makes people generous with it.

### Modmail to r/Windows11 (send before posting)

> **Subject:** Permission to post a free open-source Windhawk mod I wrote
>
> Hi — I wrote a free, open-source Windhawk mod that adds macOS-style hot
> corners and screen edges to Windows 11, with per-monitor configuration. It was
> accepted into the official Windhawk catalogue last week.
>
> I saw that recent Windhawk mod posts here carry a "promotion approved by the
> mods" note, so I wanted to ask first rather than assume. There is nothing paid
> and nothing to sign up for — it is MIT licensed and installs through Windhawk.
>
> Happy to post it in whatever format you prefer, or not at all if it is not a
> fit. Thanks for your time.
>
> Mod page: https://windhawk.net/mods/win-x-hotcorners
> Source: https://github.com/DhakadG/win-x-hotcorners

---

## Post 1 — r/Windhawk (post first, it is the safest room)

The release bot already announced 1.3.0 here, so do **not** re-announce. Post
the thing a bot cannot: the reasoning.

**Title:** `Why Win-X Hot Corners ended up with 16 zones per display instead of 4`

> The bot posted the 1.3.0 release here a couple of days ago, so this is the
> other half — why it looks the way it does, since a few of the decisions were
> not obvious to me when I started.
>
> I wanted Mac-style hot corners. Four corners, four actions, done. That lasted
> about a week of actually using it.
>
> **Corners are a tiny target; edges are a huge one.** Once you have thrown the
> pointer at a corner a few hundred times you notice you are aiming, and aiming
> is the thing hot corners were supposed to remove. So edges got included — but
> a whole edge is too coarse, since the top edge of a maximised window is where
> the title bar lives. Splitting each edge into three segments was the
> compromise that survived: 4 corners + 4 edges × 3 = 16 zones. Give two
> neighbouring segments the same action and they merge back into one wide zone,
> so you are not forced to care about the split.
>
> **The real problem is not triggering, it is *not* triggering.** A corner that
> fires whenever the pointer passes through is unusable — you cross corners
> constantly on the way to somewhere else. So a zone can require a dwell, or a
> double-knock (leave and come straight back), or a held modifier, or any
> combination. That is most of the complexity in the mod and all of the reason
> it is usable.
>
> **No global mouse hook.** It samples the cursor on its own thread instead. A
> low-level hook sits in the input path of every application on the machine,
> games included, and I did not want that trade for a convenience feature.
>
> ![hot-corners.gif]
>
> It runs as a tool mod, so it is its own small process rather than injected
> into Explorer.
>
> Mod page: https://windhawk.net/mods/win-x-hotcorners
> Source: https://github.com/DhakadG/win-x-hotcorners
>
> The thing I am least sure about is the zone count — 16 is either exactly right
> or twice what anyone needs, and I cannot tell from my own use. If you have
> installed it, how many did you actually bind?

---

## Post 2 — r/Windows11 (only after mod permission)

**Title:** `Windows 11 still doesn't have hot corners, so I built them — free and open source`

Contains "Windows 11" and "hot corners" verbatim. That is the point.

> Every time I use a Mac I miss hot corners, and every time I come back to
> Windows I re-discover that eleven versions in, it still cannot do it.
>
> So: throw the pointer into the top-left corner, Task View opens.
>
> ![task-view.gif]
>
> Any corner, any edge, and you pick what each one does — Task View, Start, snap
> the window, virtual desktops, Quick Settings, lock, media keys, or just run a
> program or a key combination you name yourself.
>
> Two things I cared about that most hot-corner tools skip:
>
> **It doesn't fire by accident.** A corner can wait for you to pause in it, or
> need a double-knock, or only work with Ctrl held. Passing through on the way
> somewhere else does nothing.
>
> **Each monitor gets its own setup.** The same physical corner can do different
> things on different screens, and it binds to the display by name — so
> rearranging monitors doesn't shuffle your config.
>
> It's free, MIT licensed, no account, nothing paid. It installs through
> [Windhawk](https://windhawk.net) (a Windows customisation platform) — search
> **Win-X Hot Corners** in its Explore tab.
>
> I wrote it, so obviously I like it. What I'd actually like to know: which
> corner would you bind first? I keep going back and forth on whether Task View
> or Show Desktop deserves the top-left.

---

## Post 3 — the Aero Peek angle (r/Windows11 or r/Windows, ~1 week later)

Targets a real, heavily-searched grievance the mod genuinely addresses.
Strongest SEO title of the set.

**Title:** `Windows 11 removed the Show Desktop peek — I got it back with a hot corner`

> Windows 7 through 10 had that sliver in the bottom-right corner: hover, all
> your windows go transparent, look at the desktop, move away, everything comes
> back. Windows 11 kept a click-to-toggle version and dropped the hover-peek.
>
> ![trigger-hold.gif]
>
> That's a corner set to "hold to peek" — one action when the pointer arrives,
> another when it leaves. Bottom-right corner, hold, peek, move away, back to
> normal.
>
> Honest caveat: Windows 11 exposes no API for the real transparent peek, so
> this is Win+D on the way in and Win+D on the way out. It's a toggle wearing a
> peek's clothing — visually a swap rather than a fade. But the *gesture* is
> back, which was the part I actually missed.
>
> Same mechanism does other things — bottom-left for Start, an edge for snapping
> the current window, a corner for Quick Settings.
>
> Free and open source, installs through Windhawk:
> https://windhawk.net/mods/win-x-hotcorners
>
> Did anyone ever find a way to get the genuine transparent peek back on 11? I
> could not find one that survives an update.

---

## Post 4 — r/SideProject (any time, no permission needed)

Different audience: they want the making-of, not the feature list.

**Title:** `I wanted Mac hot corners on Windows. I ended up with a programmable perimeter around every monitor.`

> Started as a weekend thing. Four corners, four actions.
>
> Then: corners are a small target, so add edges. Then: a whole edge is too
> coarse, so split each into three. Then: it fires when you cross a corner by
> accident, so add a dwell timer. Then a double-knock. Then a modifier key.
> Then: I have two monitors and they should not do the same thing. Then: unplug
> a monitor and the config reshuffles, so bind zones to display *names*.
>
> That is 16 zones per display, 38 actions, five ways to trigger, and about
> 6,000 lines of C++ for something I originally described as "like the Mac
> thing".
>
> ![hot-corners.gif]
>
> What I would tell past me:
>
> - The interesting problem was never *detecting* the corner. It was **not**
>   firing — a hot corner that triggers when you pass through is worse than no
>   hot corner at all.
> - Multi-monitor is where these tools quietly fall over. Screen coordinates
>   move when you rearrange displays; display names do not.
> - Per-monitor DPI will find every place you assumed a pixel is a pixel.
> - I skipped the global mouse hook and sampled the cursor on a thread instead.
>   Slightly more code, and it stays out of everyone else's input path.
>
> It got accepted into the Windhawk catalogue last week, which was the goal.
> Free, MIT: https://github.com/DhakadG/win-x-hotcorners

---

## Post 5 — Show HN (last, and only if 2–4 landed well)

HN wants how and why, and punishes marketing voice. Strip every adjective.

**Title:** `Show HN: Hot corners for Windows, without a global mouse hook`

> I wanted macOS hot corners on Windows and did not want the usual
> implementation, which installs a WH_MOUSE_LL hook — that puts your callback in
> the synchronous input path of every application on the machine, and a slow
> callback there degrades input system-wide.
>
> Instead, a dedicated thread samples the cursor every 16ms and does its own
> hit-testing. The trade is that you cannot see clicks or swallow input, neither
> of which hot corners need.
>
> Other things that were harder than expected:
>
> - **Not firing** is the whole problem. Crossing a corner en route to something
>   else must not trigger it, so zones can require a dwell, a double entry, or a
>   held modifier.
> - **Monitor identity.** Binding zones to screen coordinates breaks the moment
>   displays are rearranged, so zones bind to display names with wildcard
>   inheritance.
> - **Per-monitor DPI**, where window-frame maths differs per display.
> - **Injected modifiers.** Sending a combo while the user physically holds Ctrl
>   contaminates it, so held modifiers are masked out — including a Ctrl tap to
>   stop a synthetic Win keyup opening the Start menu.
>
> It ships as a Windhawk tool mod (its own process, nothing injected), ~6k lines
> of C++, MIT.
>
> https://github.com/DhakadG/win-x-hotcorners

---

## Sequencing

Do not fire all of these in one day; it reads as a campaign, and Reddit is good
at spotting campaigns.

| When | Where | Needs |
|---|---|---|
| Day 0 | r/Windhawk — post 1 | nothing |
| Day 0 onward | reply to old ranking threads | ongoing, 1/day max |
| Day 2 | modmail r/Windows11 | — |
| Day 3–5 | r/Windows11 — post 2 | mod permission |
| Day 7 | r/SideProject — post 4 | nothing |
| Day 10 | r/Windows or r/Windows11 — post 3 | maybe permission |
| Later | Show HN — post 5 | only if the above landed |

Product Hunt is deliberately absent. It rewards products with a signup funnel
and a landing page; a free Windows mod with neither will do badly there, and the
audience is not Windows power users. Revisit only if there is ever a website.

## What not to do

- No "please upvote / star / share". Ask for **opinions** instead — same call to
  action, allowed everywhere.
- No cross-posting the same text to five subs in one day.
- No posting the README. The README is for people who already decided; a post is
  for people who have not.
- Do not lead with "macOS-style" everywhere. It is a useful *hook* and a real
  search term, but the product is bigger than the comparison — nobody searches
  for a programmable perimeter, and that is exactly why the phrase is worth
  keeping for the body rather than the title.

## Housekeeping already done (2026-08-25)

- Merged PR branch `add-win-x-hotcorners` deleted; tip preserved as tag
  `pr-5001-merged` on the fork, since #5001 was squash-merged and those 23
  commits exist nowhere else. Fork `main` fast-forwarded to upstream, so the
  next mod update branches from current gallery code.
- Standalone repo synced 1.2.1 → 1.3.0. Its README still told people to wait for
  PR #5001 to merge, and its hero GIF was the stale one that 1.3.0 replaced.
- `v1.3.0` tagged and released on the standalone repo.
- Repo topics and homepage set on `DhakadG/win-x-hotcorners` for GitHub search.
- Issue #11 checklist updated to what is genuinely captured; left open for the
  two clips that are not.

## Worth doing later

- **Capture the multi-monitor clip.** Biggest gap in the media set.
- The catalogue `@description` is good but says "Windows" rather than
  "Windows 11". Worth a word next time the mod is updated anyway — not worth a
  PR on its own.
- A written post on the no-mouse-hook decision would be the natural canonical
  article if a blog ever happens; posts 4 and 5 are drafts of it already.
