# Guru-600 1.0.3 USB playback

A personal build of [Guru-600](https://github.com/MarcGoudy/guru-600) at commit `3f6c7272108ab6d2a285f53f37c5dc7b008f3002`, plus USB audio playback.

The FM-1 appears as a USB speaker. A computer or iPad can play through the speaker and headphones, mixed with the synth. MASTER sets how loud that playback is. Recording back to the computer is 16-bit stereo at 44.1 kHz only. The USB name is Guru-600.

Download the firmware file:

https://github.com/chinotto1/guru-600-usb-playback/raw/main/playback/guru-600-1.0.3-usb.fwsc

An iPad can download that file. iPad browsers cannot install it. On a computer, download this folder, run `python3 -m http.server` inside `playback`, and open http://localhost:8000 in Chrome, Edge, Brave, or Opera. Then press Install.

This build has not been run on a real FM-1. If the unit no longer starts, recovery uses [FM-1-transporter](https://github.com/kurogedelic/FM-1-transporter).

The files in `src/` are the changed sources. The rest of the firmware is the Guru-600 commit above, GPL-3.0-only. The playback path is adapted from felucca-bsp `chip/usb.c` (MPL-2.0, Copyright 2026 Leo Kuroshita, Hügelton Instruments).
