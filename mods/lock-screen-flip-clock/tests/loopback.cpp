// Records what the default speakers play (WASAPI loopback) for N seconds and
// reports when it was loud. Proves a sound really left the process - on the
// desktop and on the lock screen alike.
//
//   loopback <seconds> <out.wav>
//
// Prints one line per 50 ms window that is louder than -50 dBFS.

#include <audioclient.h>
#include <mmdeviceapi.h>
#include <windows.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    double secs = argc > 1 ? atof(argv[1]) : 5;
    const char* out = argc > 2 ? argv[2] : "loopback.wav";
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IMMDeviceEnumerator* en = nullptr;
    IMMDevice* dev = nullptr;
    IAudioClient* ac = nullptr;
    IAudioCaptureClient* cap = nullptr;
    WAVEFORMATEX* fmt = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&en)) ||
        FAILED(en->GetDefaultAudioEndpoint(eRender, eConsole, &dev)) ||
        FAILED(dev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&ac)) || FAILED(ac->GetMixFormat(&fmt)) ||
        FAILED(ac->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, fmt, nullptr)) ||
        FAILED(ac->GetService(__uuidof(IAudioCaptureClient), (void**)&cap)) || FAILED(ac->Start())) {
        printf("FAIL  cannot open loopback capture\n");
        return 1;
    }
    LPWSTR id = nullptr;
    dev->GetId(&id);
    printf("device %ls\n", id ? id : L"?");
    // The mix format is 32-bit float on every shared-mode endpoint Windows ships.
    const int ch = fmt->nChannels, rate = fmt->nSamplesPerSec;
    std::vector<float> mono;
    unsigned long long first = 0;  // FILETIME of the first sample captured
    DWORD start = GetTickCount();
    while (GetTickCount() - start < secs * 1000) {
        Sleep(10);
        UINT32 n;
        while (SUCCEEDED(cap->GetNextPacketSize(&n)) && n) {
            BYTE* data;
            DWORD flags;
            if (FAILED(cap->GetBuffer(&data, &n, &flags, nullptr, nullptr))) break;
            if (!first) {
                FILETIME ft;
                GetSystemTimePreciseAsFileTime(&ft);
                first = (((unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime) - (unsigned long long)n * 10000000ull / rate;
            }
            const float* f = (const float*)data;
            for (UINT32 i = 0; i < n; i++) {
                float s = 0;
                if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                    for (int c = 0; c < ch; c++) s += f[i * ch + c];
                }
                mono.push_back(s / ch);
            }
            cap->ReleaseBuffer(n);
        }
    }
    ac->Stop();
    size_t win = rate / 20;
    double peakAll = 0;
    int loud = 0;
    for (size_t w = 0; w * win < mono.size(); w++) {
        double pk = 0;
        for (size_t i = w * win; i < std::min(mono.size(), (w + 1) * win); i++) pk = std::max(pk, (double)std::fabs(mono[i]));
        peakAll = std::max(peakAll, pk);
        if (pk > 0.00316) {  // -50 dBFS
            printf("  %6.2f s  peak %5.1f dBFS\n", w * win / (double)rate, 20 * std::log10(pk));
            loud++;
        }
    }
    printf("%zu samples at %d Hz, %d loud windows, peak %.1f dBFS\n", mono.size(), rate, loud,
           peakAll > 0 ? 20 * std::log10(peakAll) : -999.0);
    FILE* f = fopen(out, "wb");
    if (f) {
        std::vector<short> pcm(mono.size());
        for (size_t i = 0; i < mono.size(); i++) pcm[i] = (short)std::lround(std::fmax(-1, std::fmin(1, mono[i])) * 32767);
        unsigned data = (unsigned)pcm.size() * 2, riff = 36 + data, fl = 16, r = rate, br = rate * 2;
        unsigned short tag = 1, c1 = 1, al = 2, bits = 16;
        fwrite("RIFF", 1, 4, f), fwrite(&riff, 4, 1, f), fwrite("WAVEfmt ", 1, 8, f), fwrite(&fl, 4, 1, f);
        fwrite(&tag, 2, 1, f), fwrite(&c1, 2, 1, f), fwrite(&r, 4, 1, f), fwrite(&br, 4, 1, f), fwrite(&al, 2, 1, f);
        fwrite(&bits, 2, 1, f), fwrite("data", 1, 4, f), fwrite(&data, 4, 1, f), fwrite(pcm.data(), 2, pcm.size(), f);
        fclose(f);
    }
    std::string txt = std::string(out) + ".txt";
    if (FILE* t = fopen(txt.c_str(), "w")) {
        fprintf(t, "%llu\n", first);
        fclose(t);
    }
    return loud ? 0 : 3;
}
