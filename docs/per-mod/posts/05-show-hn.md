# Post 5 — Show HN

| | |
| --- | --- |
| **Where** | news.ycombinator.com, Show HN |
| **Permission needed** | No, but read the [Show HN rules](https://news.ycombinator.com/showhn.html) |
| **Post as** | URL post pointing at the **GitHub repo**, plus a first comment |
| **When** | Last. Only if posts 02–04 landed reasonably. |
| **Why this angle** | HN is a technical audience with no interest in the product pitch. The hook is the implementation decision, not the feature. |

## Media

**None in the post.** HN does not render images, and a Show HN is a URL plus
text. The GIFs do their work in the linked repo's README, which is why that
README's hero matters here more than any attachment.

Before posting, confirm the repo README renders correctly and its hero GIF plays
— that page *is* the media for this post.

## URL to submit

```
https://github.com/DhakadG/win-x-hotcorners
```

## Title

```
Show HN: Hot corners for Windows, without a global mouse hook
```

Rules for this title: no adjectives, no exclamation, no "I built". The
differentiator goes in the title because that is the only thing that earns a
click from this crowd.

## First comment (post immediately after submitting)

```
I wanted macOS hot corners on Windows and did not want the usual implementation,
which installs a WH_MOUSE_LL hook — that puts your callback in the synchronous
input path of every application on the machine, and a slow callback there
degrades input system-wide.

Instead, a dedicated thread samples the cursor every 16ms and does its own
hit-testing. The trade is that you cannot see clicks or swallow input, neither of
which hot corners need.

Other things that were harder than expected:

- Not firing is the whole problem. Crossing a corner en route to something else
  must not trigger it, so zones can require a dwell, a double entry, or a held
  modifier.
- Monitor identity. Binding zones to screen coordinates breaks the moment
  displays are rearranged, so zones bind to display names with wildcard
  inheritance.
- Per-monitor DPI, where window-frame maths differs per display.
- Injected modifiers. Sending a combo while the user physically holds Ctrl
  contaminates it, so held modifiers are masked out — including a Ctrl tap to
  stop a synthetic Win keyup opening the Start menu.

It ships as a Windhawk tool mod (its own process, nothing injected), ~6k lines of
C++, MIT.
```

## Notes

- **Do not ask anyone to upvote.** HN detects voting rings and penalises the
  submission and the account. Same for coordinated comments.
- Answer every technical comment, including hostile ones, in a technical
  register. HN forgives a flawed design and does not forgive defensiveness.
- If it does not get traction, leave it. Reposting a Show HN quickly reads as
  gaming and is against the guidelines.
- Best time is usually a weekday morning US Eastern, but do not over-optimise
  this — the title matters far more than the hour.

## Likely replies, and what to say

**"Polling every 16ms is wasteful versus an event-driven hook."**
> It is a real trade. A `GetCursorPos` call plus a hit test is cheap, and it
> stays off the input path entirely — the hook version is event-driven but every
> application on the machine pays for your callback's latency. Happy to be shown
> a third option.

**"Why not `SetWinEventHook` / raw input?"**
> Raw input gives deltas rather than a screen position and still needs a message
> loop and its own accumulation; for "is the cursor inside this rectangle" the
> position is the thing being asked for directly.

**"Why does a hot-corner tool need 6,000 lines?"**
> Honest answer: roughly a third is a read-only settings dashboard that arguably
> should not be in scope. The detection and action core is much smaller.

**"Windows already has this via [X]."**
> Ask what they mean specifically, then answer specifically. FancyZones is
> drag-target snapping; the edge-swipe gestures are touch; neither is
> pointer-arrival triggering. Do not get defensive about it.
