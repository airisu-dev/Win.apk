**Win.apk 0.8.1: native on Windows on Arm.** Win.apk runs Android apps on Windows 10/11 as ordinary Windows programs: each app gets its own window, Start menu entry and notifications. It is an unofficial port of [Mac.apk](https://github.com/kksimp/Mac.apk).

> Preview build, not code-signed yet: SmartScreen may warn. Click **More info**, then **Run anyway**.

## Download
- **Intel or AMD PC:** **`Win.apk-Setup.exe`**, the installer (per user, no administrator rights), or **`Win.apk-windows-x64.zip`**, portable: unzip it and run `Win.apk.exe`.
- **Windows on Arm PC (Snapdragon):** **`Win.apk-Setup-arm64.exe`** or **`Win.apk-windows-arm64.zip`**. A copy of the x64 build on an Arm PC moves to this one with its next update.

## New in 0.8.1
- **Native Windows on Arm build.** Apps' ARM code runs as it is: nothing is translated, and Win.apk itself is no longer emulated. Install apps' `arm64-v8a` builds there; the Store picks them for you.
- **A maximized window is the phone turned on its side**, at a density that draws it pixel for pixel.
- **Text and layout closer to Android:** right-to-left paragraphs, line limits and baselines, `TextView` sizing, password and phone fields, themes' colours and sizes, tinted and inset drawables, image matrices.

## Known limitations
- Apps with only 32-bit ARM (`armeabi-v7a`) code do not run.
- On Windows on Arm, apps with only x86_64 code do not run, and a library that uses the x18 register is refused (two libGDX games of 92 tested apps).
- No in-app purchases, Play Integrity or Vulkan; OpenSL ES sound is missing; sensors report a phone lying still.

[Getting started](https://github.com/israel2176/win.apk/blob/main/docs/getting-started.md) · [What works](https://github.com/israel2176/win.apk/blob/main/docs/APPS.md) · [User guide](https://github.com/israel2176/win.apk/blob/main/docs/USER_GUIDE.md) · [עברית](https://github.com/israel2176/win.apk/blob/main/README.he.md). Found a problem? Use **Report a problem** in Win.apk, or [open an issue](https://github.com/israel2176/win.apk/issues/new/choose) ([how](https://github.com/israel2176/win.apk/blob/main/docs/troubleshooting.md#reporting-a-problem)).
