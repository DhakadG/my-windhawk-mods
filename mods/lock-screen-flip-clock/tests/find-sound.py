"""Finds what the clock played inside a loopback recording of the speakers.

    python find-sound.py <played.wav> <loopback.wav>

Each WAV has a sidecar .txt (<name>.txt or <name>.wav.txt) holding the
FILETIME of its first sample (preview.exe tee=..., loopback.exe). Music playing
at the same time is fine: every sound event in the played timeline (a flip, a
roll-in) is looked for on its own by normalised cross-correlation, around one
common output latency, and its score is compared with the same search run on a
time-reversed copy - a signal that was never played. An event counts as heard
when it beats its null clearly. Exit code 0 = most events were heard.
"""
import os
import sys
import wave

import numpy as np
from scipy.signal import correlate, resample_poly


def load(path):
    with wave.open(path) as w:
        rate = w.getframerate()
        x = np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).astype(np.float64) / 32768
    side = path + ".txt" if os.path.exists(path + ".txt") else path[:-4] + ".txt"
    with open(side) as f:
        start = int(f.read().split()[0]) / 1e7
    return rate, x, start


def score_at(template, signal):
    c = correlate(signal, template, mode="valid")
    e = np.sqrt(np.convolve(signal**2, np.ones(len(template)), mode="valid"))
    return c / (np.linalg.norm(template) * np.maximum(e, 1e-12))


def events(x, rate):
    """[start, end) sample ranges of the non-silent stretches, gaps > 150 ms split them."""
    loud = np.abs(x) > 1e-3
    idx = np.flatnonzero(loud)
    if not len(idx):
        return []
    out, s, last = [], idx[0], idx[0]
    for i in idx[1:]:
        if i - last > 0.15 * rate:
            out.append((s, last + 1))
            s = i
        last = i
    out.append((s, last + 1))
    return out


def main():
    pr, played, p0 = load(sys.argv[1])
    lr, loop, l0 = load(sys.argv[2])
    if lr != pr:
        loop = resample_poly(loop, pr, lr)
    offset = (p0 - l0) * pr  # where the played timeline starts inside the loopback, in samples
    evs = events(played, pr)
    if not evs:
        print("FAIL  nothing was played")
        return 2
    # One output latency for the whole run, from the strongest event in 0..600 ms.
    lags = []
    for a, b in evs:
        lo = int(offset + a)
        seg = loop[max(0, lo) : lo + (b - a) + int(0.6 * pr)]
        if len(seg) <= b - a:
            continue
        sc = score_at(played[a:b], seg)
        i = int(np.argmax(sc))
        lags.append((sc[i], max(0, lo) + i - lo))
    if not lags:
        print("FAIL  the loopback recording does not cover the played sound")
        return 2
    lag = max(lags)[1]
    print(f"output latency {lag / pr * 1000:.0f} ms, {len(evs)} events")
    heard = 0
    for a, b in evs:
        lo = int(offset + a + lag - 0.02 * pr)
        seg = loop[max(0, lo) : lo + (b - a) + int(0.04 * pr)]
        if len(seg) <= b - a:
            continue
        s = float(np.max(score_at(played[a:b], seg)))
        n = float(np.max(score_at(played[a:b][::-1], seg)))
        ok = s > 0.3 and s > 1.6 * n
        heard += ok
        print(f"  {a / pr:6.2f} s  {(b - a) / pr * 1000:5.0f} ms  match {s:.2f}  null {n:.2f}  {'heard' if ok else '-'}")
    ok = heard * 2 > len(evs)
    print(f"{'PASS' if ok else 'FAIL'}  {heard} of {len(evs)} sound events found in what the speakers played")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
