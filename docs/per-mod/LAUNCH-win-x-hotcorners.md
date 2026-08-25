# Win-X Hot Corners — launch and reach plan

Goal is **reach and search visibility**, not revenue. The win condition is that
someone typing *"does Windows 11 have hot corners"* into Google lands on
something we wrote.

Status as of 2026-08-25: **v1.3.0, published.** Live at
<https://windhawk.net/mods/win-x-hotcorners>, accepted via
[ramensoftware/windhawk-mods#5001](https://github.com/ramensoftware/windhawk-mods/pull/5001),
merged 22 Aug 2026.

> The individual, copy-paste-ready post guides live in
> [`posts/`](posts/). This file is the strategy and the schedule; each guide is
> self-contained, with its own media list, title, body and prepared replies.
> Drafts are **not** duplicated here — one copy, so it cannot drift.

## The posts

| # | Guide | Where | Needs permission |
| --- | --- | --- | --- |
| 00 | [Answering threads that already rank](posts/00-answering-existing-threads.md) | anywhere, as comments | no |
| 01 | [Why 16 zones instead of 4](posts/01-windhawk-why-16-zones.md) | r/Windhawk | no |
| 02 | [Windows 11 still doesn't have hot corners](posts/02-windows11-hot-corners.md) | r/Windows11 | **probably yes** |
| 03 | [The Show Desktop peek, restored](posts/03-show-desktop-peek.md) | r/Windows11 or r/Windows | probably yes |
| 04 | [I ended up with a programmable perimeter](posts/04-sideproject-overbuilt.md) | r/SideProject | no |
| 05 | [Show HN: without a global mouse hook](posts/05-show-hn.md) | Hacker News | no |

## Why the titles look plain

Reddit threads and GitHub repos rank in Google for long-tail questions. Almost
nobody searches *"Win-X Hot Corners"* — they search the **problem**:

| What people actually type | Currently answered by |
| --- | --- |
| does windows 11 have hot corners | an old Reddit thread |
| mac hot corners for windows | an old Reddit thread |
| windows 11 show desktop button missing | an old Reddit thread |
| aero peek windows 11 | an old Reddit thread |
| hot corners windows 10 | an old Reddit thread |

So the **title is the SEO surface**, not the body. Every title in `posts/`
contains a phrase somebody would actually type. Resist clever titles — a clever
title ranks for nothing.

And note what that table says: the queries are already answered, mostly badly.
That is why [post 00](posts/00-answering-existing-threads.md) outranks the rest
in value despite being the least glamorous — it adds the answer to pages that
already hold the ranking, instead of building new pages that might earn one.

## Media

All in `docs/media/`, mirrored in the standalone repo. Every file below is
verified reachable over `raw.githubusercontent.com` on `main`.

| Asset | Size | Shows | Used by |
| --- | --- | --- | --- |
| `hot-corners.gif` | 445 KB | bottom-left → Start menu | 01, 04 |
| `task-view.gif` | 2.2 MB | top-left → Task View | 02 |
| `start-menu.gif` | 445 KB | corner → Start | spare hero |
| `snap-left.gif` / `snap-right.gif` | 1.4 / 1.5 MB | snap active window | spare |
| `quick-settings.gif` | 648 KB | corner → Quick Settings | spare |
| `switch-window.gif` | 669 KB | corner → last window | spare |
| `trigger-arrival.gif` | 2.0 MB | fires on arrival | spare |
| `trigger-dwell.gif` | 381 KB | fires after a pause | spare |
| `trigger-knock.gif` | 253 KB | leave and come back | spare |
| `trigger-hold.gif` | 1.0 MB | enter action + leave action | 03 |
| `trigger-modifier.gif` | 989 KB | inert unless Ctrl held | spare |
| `trigger-alternate.gif` | 1.2 MB | alternating actions | spare |
| `dashboard.png` | 35 KB | Zones & settings window | 02, 04 |

Base URL:
`https://raw.githubusercontent.com/DhakadG/my-windhawk-mods/main/docs/media/`

Upload GIFs **directly to Reddit** rather than linking. Native media gets
materially better reach, and a bare external link reads as promo.

### The one gap

**Multi-monitor — the same corner doing different things on two displays.** It
is the mod's strongest differentiator and nothing in the set shows it. No post
currently depends on it, so nothing is blocked, but it is the first thing to
capture whenever the high-resolution pass happens. Tracked as
[win-x-hotcorners#11](https://github.com/DhakadG/win-x-hotcorners/issues/11),
along with a clip of the monitor list in the log.

Capture rig is `scripts/capture/`. Note its File-Explorer preflight guard exists
for a reason and must not be removed.

## Before posting: the rules problem

I could not verify subreddit rules directly — Reddit blocks automated fetching —
so **check each sub's rules yourself before posting**. What is believed:

- **r/Windows11 appears to require moderator permission** for promoting your own
  software; recent Windhawk mod posts there carry an auto-disclaimer saying
  permission was obtained. Modmail draft is in
  [post 02](posts/02-windows11-hot-corners.md).
- **r/Windhawk** already has a bot-generated release post, so post 01
  deliberately does not re-announce.
- Most Windows subs enforce some version of the 90/10 rule. If the account has
  no history in a sub, build some before dropping a project post into it.

Disclose authorship in every post. "I made this" is not a weakness on Reddit —
it is what makes the post allowed and makes people generous with it.

## Sequencing

Do not fire these in one day; it reads as a campaign, and Reddit is good at
spotting campaigns.

| When | What | Needs |
| --- | --- | --- |
| Day 0 | post 01 — r/Windhawk | nothing |
| Day 0 onward | post 00 — replies, 1/day max | nothing |
| Day 2 | modmail r/Windows11 | — |
| Day 3–5 | post 02 — r/Windows11 | mod permission |
| Day 7 | post 04 — r/SideProject | nothing |
| Day 10 | post 03 — Show Desktop angle | maybe permission |
| Later | post 05 — Show HN | only if the above landed |

Product Hunt is deliberately absent. It rewards products with a signup funnel and
a landing page; a free Windows mod with neither will do badly there, and the
audience is not Windows power users. Revisit only if there is ever a website.

## What not to do

- No "please upvote / star / share". Ask for **opinions** instead — same call to
  action, allowed everywhere.
- No cross-posting the same text to five subs in one day.
- No posting the README. The README is for people who already decided; a post is
  for people who have not.
- Do not lead with "macOS-style" everywhere. It is a useful *hook* and a real
  search term, but the product is bigger than the comparison — nobody searches
  for a programmable perimeter, which is exactly why that phrase belongs in the
  body rather than the title.

## Housekeeping already done (2026-08-25)

- Merged PR branch `add-win-x-hotcorners` deleted; tip preserved as tag
  `pr-5001-merged` on the fork, since #5001 was squash-merged and those 23
  commits exist nowhere else. Fork `main` fast-forwarded to upstream, so the next
  mod update branches from current gallery code.
- Standalone repo synced 1.2.1 → 1.3.0, tagged `v1.3.0` with a release. Its
  README had still told people to wait for #5001 to merge, and its hero GIF was
  the stale one 1.3.0 replaced.
- Repo topics (12) and homepage set on `DhakadG/win-x-hotcorners`.
- `mods/win-x-hotcorners/README.md` was a 300-line duplicate still describing the
  4.1.x tray-only design ("this mod has no Settings page"). Replaced with a
  pointer to the embedded readme, which is the source of truth.
- Top-level `README.md` listed the mod at **4.1.4** with #5001 "open", and was
  missing two mods entirely. Corrected.
- Issue #11 checklist updated to what is genuinely captured; left open for the
  two clips that are not.

## Worth doing later

- **Capture the multi-monitor clip.** Biggest gap in the media set.
- The catalogue `@description` is good but says "Windows" rather than
  "Windows 11". Worth a word next time the mod is updated anyway — not worth a
  PR on its own.
- A written article on the no-mouse-hook decision would be the natural canonical
  piece if a blog ever happens; posts 04 and 05 are drafts of it already.
