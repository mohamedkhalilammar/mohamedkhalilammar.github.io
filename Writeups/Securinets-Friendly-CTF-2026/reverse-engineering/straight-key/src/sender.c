#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "payload.h"
#include "decoys.h"

#define VK_TARGET 0x4B

static unsigned int rng_state;

static unsigned char next_key(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return (unsigned char)((rng_state >> 16) & 0xFFu);
}

static unsigned int code_checksum(void)
{
    unsigned char *base = (unsigned char *)GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)base;
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
    unsigned int h = 2166136261u;
    unsigned int i, n;

    for (i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (memcmp(sec[i].Name, ".text", 5) != 0) {
            continue;
        }
        for (n = 0; n < sec[i].SizeOfRawData; n++) {
            h = (h ^ base[sec[i].VirtualAddress + n]) * 16777619u;
        }
        break;
    }
    return h;
}

static void greet(void)
{
    char text[MSG_LEN + 1];
    int i;

    rng_state = MSG_SEED;
    for (i = 0; i < MSG_LEN; i++) {
        text[i] = (char)(MSG_BLOB[i] ^ next_key());
    }
    text[MSG_LEN] = '\0';
    printf("%s\n", text);
    fflush(stdout);
    SecureZeroMemory(text, sizeof text);
}

static void expand_pattern(int *out)
{
    unsigned char buf[PAT_BYTES];
    int i;

    rng_state = code_checksum();
    for (i = 0; i < PAT_BYTES; i++) {
        buf[i] = (unsigned char)(PAT_STORE[PAT_MARKLEN + i] ^ next_key());
    }
    for (i = 0; i < PAT_BITS; i++) {
        out[i] = ((buf[i >> 3] >> (7 - (i & 7))) & 1) ? PAT_UNIT * 3 : PAT_UNIT;
    }
    SecureZeroMemory(buf, sizeof buf);
}

static void wait_ms(int ms)
{
    LARGE_INTEGER freq, start, now;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    if (ms > 16) {
        Sleep((DWORD)(ms - 16));
    }
    do {
        QueryPerformanceCounter(&now);
    } while ((double)(now.QuadPart - start.QuadPart) * 1000.0 /
             (double)freq.QuadPart < (double)ms);
}

int main(void)
{
    int pattern[PAT_BITS];
    int scratch[PAT_BITS];
    int i;

    greet();

    RunSelfTestSuite((unsigned int)GetTickCount(), scratch);

    expand_pattern(pattern);

    for (i = 0; i < PAT_BITS; i += 2) {
        keybd_event(VK_TARGET, 0, 0, 0);
        wait_ms(pattern[i]);
        keybd_event(VK_TARGET, 0, KEYEVENTF_KEYUP, 0);
        if (i + 1 < PAT_BITS) {
            wait_ms(pattern[i + 1]);
        }
    }

    SecureZeroMemory(pattern, sizeof pattern);
    return 0;
}
