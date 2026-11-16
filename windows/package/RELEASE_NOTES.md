**First public preview.** Win.apk runs Android apps on Windows 10/11 (x64) as ordinary Windows programs: each app gets its own window, Start menu entry and notifications. It is an unofficial port of [Mac.apk](https://github.com/kksimp/Mac.apk).

> Preview build, not code-signed yet: SmartScreen may warn. Click **More info**, then **Run anyway**.

## Download
- **`Win.apk-Setup.exe`**: the installer (per user, no administrator rights). Recommended.
- **`Win.apk-windows-x64.zip`**: portable; unzip it and run `Win.apk.exe`.
- **`Win.apk-Setup-arm64.exe`**, **`Win.apk-windows-arm64.zip`**: for Windows on Arm PCs (Snapdragon). Native: apps' ARM code runs as it is, nothing translated or emulated. Install apps' `arm64-v8a` builds there.

## Highlights
- Install `.apk`, `.apkm`, `.xapk` and `.apks` files by double-click or drag and drop, or from the built-in F-Droid Store.
- Apps that ship only 64-bit ARM code are translated for your PC in the background, at the lowest priority.
- Your PC's hardware for apps: webcam, microphone, location, Bluetooth, game controllers, and your Windows folders.
- **Experimental:** Google's own Play services and Play Store, with a guided setup (Settings > Google account); Google Play opens inside the Win.apk window.
- An app's window opens the moment you start it, showing its icon until the app has drawn, as on a phone; an install shows the app's own icon and name while it runs.
- English and Hebrew, light and dark.
- Apps start about a fifth sooner than in 0.8 and use about 30 MB less memory each: Win.apk ships its own, smaller Java runtime and keeps the Java classes every app loads ready to map, a file it makes in the background after the first app start.

## Known limitations
- Apps with only 32-bit ARM (`armeabi-v7a`) code do not run.
- On Windows on Arm, apps with only x86_64 code do not run, and a library that uses the x18 register is refused (two libGDX games in the 92-app test).
- No in-app purchases, Play Integrity or Vulkan; OpenSL ES sound is missing; sensors report a phone lying still.

[Getting started](https://github.com/israel2176/win.apk/blob/main/docs/getting-started.md) · [What works](https://github.com/israel2176/win.apk/blob/main/docs/APPS.md) · [User guide](https://github.com/israel2176/win.apk/blob/main/docs/USER_GUIDE.md) · [עברית](https://github.com/israel2176/win.apk/blob/main/README.he.md). Found a problem? Use **Report a problem** in Win.apk, or [open an issue](https://github.com/israel2176/win.apk/issues/new/choose) ([how](https://github.com/israel2176/win.apk/blob/main/docs/troubleshooting.md#reporting-a-problem)).
