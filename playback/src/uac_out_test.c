/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * Playback checks: the host plays through the FM-1 (FELUCCA_UAC_OUT). */
/* Host test of the USB audio output added for playback from a computer or iPad:
 * the configuration enumerates a speaker terminal and an adaptive isochronous OUT
 * endpoint at 44.1 kHz, and uac_play mixes a primed ring into a 24-bit block at MASTER. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define RING_PUBLISH() __asm__ volatile("" ::: "memory")
#define HALF_FRAMES 128
#define FELUCCA_OTA 0
#define FELUCCA_CDC 0
#define FELUCCA_UAC 1
#define FELUCCA_UAC_OUT 1
#define FELUCCA_UAC_48K 0
#define FELUCCA_UAC_AS_FIRST 1
static void fm1_delay_ms(uint32_t ms) { (void)ms; }
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#include "../firmware/src/usb.c"

static int fails;
static void check(const char *what, int ok)
{
    printf("%-72s %s\n", what, ok ? "ok" : "FAIL");
    if (!ok)
        fails++;
}

static uint32_t le16(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8; }

static void test_descriptors(void)
{
    const uint8_t *c = CFG_DESC;
    uint32_t n = sizeof CFG_DESC, off, total = le16(c + 2);
    int speaker = 0, ep_out = 0, rate = 0, capture = 0, lens = 1;
    check("configuration length matches wTotalLength", c[1] == 2 && total == n && c[4] == 4);
    check("bcdDevice carries the playback bit (3.58: 44.1 capture, playback, AS-first)",
          DEV_DESC[12] == 0x58 && DEV_DESC[13] == 0x03);
    for (off = 0; off + 1 < n && lens;) {
        const uint8_t *d = c + off;
        if (d[0] < 2 || off + d[0] > n) {
            lens = 0;
            break;
        }
        if (d[1] == 0x24 && d[2] == 3 && d[0] >= 6 && le16(d + 4) == 0x0301)
            speaker = 1;
        if (d[1] == 0x24 && d[2] == 2 && d[0] == 11 && d[7] == 1 && le16(d + 8) == 44100)
            rate = 1;
        if (d[1] == 5 && d[2] == 0x04 && d[3] == 0x09 && le16(d + 4) == 196)
            ep_out = 1;
        if (d[1] == 5 && d[2] == 0x84 && d[3] == 0x05)
            capture = 1;
        off += d[0];
    }
    check("descriptors are well formed", lens && off == n);
    check("speaker terminal (0x0301) is present", speaker);
    check("playback format is 16-bit stereo 44100", rate);
    check("EP4 OUT is adaptive isochronous, 196 bytes", ep_out);
    check("EP4 IN capture endpoint is still present", capture);
    check("SET_INTERFACE numbers: capture IF 2, playback IF 3", UAC_AS_IF == 2 && UAC_ASO_IF == 3);
}

static void test_play(void)
{
    enum { N = 64 };
    int32_t buf[2 * N];
    uint32_t i;
    int32_t peak = 0;
    memset(buf, 0, sizeof buf);
    usb.config = 1;
    uo.alt = 0;
    uo.go = 0;
    uac_play(buf, N, 2048);
    check("no playback until the host selects the stream", buf[0] == 0 && buf[1] == 0);

    for (i = 0; i < 300u; i++)
        uo_ring[i] = (uint32_t)(uint16_t)1000 | ((uint32_t)(uint16_t)(int16_t)-1000 << 16);
    uo_r = 0;
    uo_w = 300;
    uo.alt = 1;
    uo.go = 0;
    uac_play(buf, N, 2048);                 /* gain 2048: 16-bit sample * 128, the full-MASTER level */
    for (i = 0; i < 2u * N; i++)
        if (buf[i] > peak)
            peak = buf[i];
        else if (-buf[i] > peak)
            peak = -buf[i];
    check("a primed 1000-sample tone reaches the DAC at gain 2048", peak > 100000 && peak < 150000);
    check("left and right keep their signs", buf[0] > 0 && buf[1] < 0);
    check("the ring advanced", uo_r > 40 && uo_r < 200);

    memset(buf, 0, sizeof buf);
    uo.alt = 0;
    uac_play(buf, N, 2048);
    check("stopping the stream adds silence", buf[0] == 0 && buf[1] == 0);
}

int main(void)
{
    test_descriptors();
    test_play();
    if (fails)
        printf("%d failed\n", fails);
    return fails ? 1 : 0;
}
