# Post 0 — answering the threads that already rank

| | |
| --- | --- |
| **Where** | Existing Reddit/forum threads, anywhere |
| **Permission needed** | No |
| **Post as** | Comments, not posts |
| **When** | Ongoing from day 0. **One per day, maximum.** |
| **Why this angle** | Highest return of anything in this folder, and the least like marketing. |

A new post starts at zero and has to earn a Google ranking. A 2019 thread titled
*"Is there a hot corners equivalent for Windows?"* **already has one**. Adding
the answer to a page that already ranks is faster than building a new page that
might.

This is the single most effective thing for the stated goal — someone searching
the problem in a year lands on a useful answer.

## Finding the threads

Run these in Google, not in Reddit's own search, which is bad at this:

```
site:reddit.com windows hot corners
site:reddit.com "hot corners" windows 11
site:reddit.com mac hot corners windows equivalent
site:reddit.com windows 11 show desktop peek missing
site:reddit.com aero peek windows 11 gone
site:reddit.com winxcorners alternative
site:superuser.com windows hot corners
```

Worth it beyond Reddit too — Super User and Microsoft Answers threads rank
persistently and almost never get a good answer.

**Prioritise by:** thread ranks on page 1 → the question is unanswered or the
top answer is "you can't" or a dead AutoHotkey script → the sub allows links.

## The rules, and they are not optional

Breaking these makes it spam, gets it removed, and can get the account
shadowbanned — which would undo everything else in this folder.

1. **Answer the question first**, completely, including the parts the mod does
   not solve. If the mod is not actually the right answer for that person, say
   so and do not link it.
2. **Disclose that you wrote it**, every time, in the same comment.
3. **One mention, plainly.** No pitch, no feature list, no bold text.
4. **Never** reply with only a link.
5. **One per day at most**, spread across different subs. A burst of ten in an
   evening is the pattern spam filters are built to catch.
6. **Do not necro-bump rudely.** On a thread older than about two years, open by
   acknowledging it: "This is an old thread, but it still comes up in search, so
   for anyone landing here —". That framing is honest and it is what makes the
   comment useful to the searcher rather than to the original poster.

## Template — thread asking if Windows has hot corners

```
This is an old thread but it still comes up in search, so for anyone landing
here: Windows has no built-in hot corners, and as of Windows 11 that is still
true. The options are AutoHotkey if you want to script it yourself, or a mod.

I wrote one — Win-X Hot Corners, a free MIT-licensed Windhawk mod. Corners and
screen edges, per-monitor, and zones can require a pause or a held modifier so
they do not fire when you cross a corner by accident.

https://windhawk.net/mods/win-x-hotcorners

If you only want one specific behaviour there are smaller mods that do just that
one thing, and AutoHotkey is genuinely fine for a single corner on a single
monitor.
```

That last paragraph is not modesty — it is what stops the comment reading as an
advert, and it is true.

## Template — thread about the missing Show Desktop peek

```
Windows 11 dropped the hover-peek and kept the click-to-toggle. As far as I can
tell there is no setting that brings the transparent hover version back, because
the API behind it is not exposed any more.

The closest I got was binding a corner to Win+D on entry and Win+D again on
leaving, so the gesture works even though it is a toggle rather than a fade. I
did that with a Windhawk mod I wrote (Win-X Hot Corners,
https://windhawk.net/mods/win-x-hotcorners) but the same thing is scriptable in
AutoHotkey if you would rather not install anything.

If anyone has found a way to get the real transparent peek back, I would like to
know — I looked fairly hard.
```

## Template — thread asking for a WinXCorners alternative

```
WinXCorners still works for a lot of people but it has not been updated in
years, and the usual complaints are multi-monitor and HiDPI.

I rebuilt the idea as a Windhawk mod — Win-X Hot Corners
(https://windhawk.net/mods/win-x-hotcorners). Disclosure: I wrote it. It handles
per-monitor configuration and per-monitor DPI, which were the two things that
sent me looking in the first place, and adds screen edges as well as corners.

I went through the WinXCorners issue tracker while building it, so if there is a
specific bug you hit there, ask and I can tell you whether this one has the same
problem.
```

That last offer is genuine — the backlog review is in
`docs/per-mod/FEATURE-BACKLOG-win-x-hotcorners.md`.

## Tracking

Keep a note of where you have commented. Two comments on the same thread months
apart looks like astroturfing even when it is forgetfulness.

| Date | Thread | Sub | Outcome |
| --- | --- | --- | --- |
| | | | |
