# Plan: rebuild Win.apk as a soft fork of Mac.apk, for Windows on Arm only

Status: proposal (2026-11-16). Nothing here is done yet.

## Goal

1. **A soft fork of [kksimp/Mac.apk](https://github.com/kksimp/Mac.apk).** The new
   repository shares Mac.apk's git history, so `git merge upstream/main` brings in
   each Mac.apk release. We change Mac.apk's own files as little as possible.
2. **Windows on Arm (ARM64) only.** Apps' arm64 code runs natively. The x86_64
   guest path (the ELF `%fs` patch, the System V thunks, x86_64 `.so` files) and
   the arm64-on-x64 translation layer (interpreter, JIT, AOT translator,
   background translation, the bundled clang) are not carried over.
3. **[israel2176/win.apk](https://github.com/israel2176/win.apk) as a source of
   picks, not an upstream.** We never pull or merge it. Its commits come over one
   at a time with `git cherry-pick -x`, gathered into one PR per topic, and each
   is recorded as taken, adapted or skipped.

## Where things stand (measured 2026-11-16)

| | Fact |
|---|---|
| Mac.apk | 131 commits. The public repo receives squashed release syncs from a private repo ("Sync from Mac.apk-private v1.0.NNNN"). Newest: **v1.0.2379** (`1500431`, Nov 5). AGPL-3.0, with a CLA (`cla.yml`) and a commercial license (`COMMERCIAL.md`). `TRADEMARKS.md` covers the name. |
| israel2176/win.apk | A GitHub fork of Mac.apk. It shares history up to `35c9b02` (Mac.apk v1.0.1702, Oct 3). GitHub shows it **580 commits ahead (457 without merges) and 92 behind**. "Behind" is mostly on paper: since Oct 3 each Mac.apk release was copied in file by file, never merged (`windows/README.md`, "Keeping up with upstream"), so git cannot see the syncs. Its `runtime/` matches Mac.apk v1.0.2379 plus its own changes. |
| This repo (airisu-dev/Win.apk) | A copy of israel2176's history up to `bc51747` (Nov 12; our clone is shallow), plus **19 commits** of our own: the ARM64 build, `WINAPK_REPO`, BGRA readback. israel2176 has **35 commits** (31 without merges) since that point that we lack (list under Phase 3). |
| Our `runtime/` vs Mac.apk v1.0.2379 | **245 files differ** (about +21.6k / −11.9k lines), and **54 Java files** are ours alone. About 76 of the changed files mention Windows (hooks). About 148 are generic framework fixes that would apply equally on the Mac. `windows/README.md` lists 154 changes to the upstream tree. Mac.apk's `runtime/CMakeLists.txt`, its plist and 6 headers in `runtime/include/macapk/` were deleted here. `runtime/translator/` (Mac-only) has 6 files changed. |
| x64-only code | `src/arm64/arm64_cpu.cpp`, `arm64_simd.cpp`, `arm64_guest.cpp`, `arm64_jit.cpp`, `arm64_aot.cpp` (about 4.6k lines; CMake already builds them only off ARM64), `aot_files.cpp`, `abi_thunk.cpp`, `fs_tls_patch.cpp`, `host/translator_main.cpp`, `host/aot_guard.*`, `tools/arm64_aot_main.cpp`, `stage_clang.sh`, the dynarmic option, the AOT tests, the `AotBackground` setting, `aot-cache\`, `docs/research/aot*`, `background-translation.md`, 13 files with `WINAPK_HOST_X64` branches, and the x64 CI, release and installer jobs. |

## Verdict: feasible

- **Git history.** Mac.apk's history is ordinary merges on `main`, so a fork that
  shares it can merge each release. israel2176 lost that ability by copying files.
  We keep it by never editing Mac.apk's files except through registered hooks
  (rules below).
- **Native code.** `windows/` is entirely ours; Mac.apk has no `windows/` and no
  `docs/`. Only a handful of root files collide (see rule R2).
- **The real cost is the 245-file Java delta.** It has to be sorted once: hook,
  generic fix, or Windows-only class. Each kind then has a home (R3). This is
  most of the work in Phase 1.
- **ARM64 only removes work.** The ARM64 build already compiles none of the
  translation layer. Dropping it shrinks the package (no clang), CI (one
  architecture) and the docs.
- **What users lose:** x64 PCs (israel2176's Win.apk still serves them), apps that
  ship only x86_64 libraries, and apps that ship only `armeabi-v7a`: Snapdragon X
  has no AArch32, and neither build ever ran 32-bit code. Apps from Google Play
  almost always include `arm64-v8a`.

## Soft-fork rules (these keep merges cheap)

- **R1: Mac.apk's files are never deleted, moved or reformatted.** That covers the
  Swift `launcher/`, `debug/`, `signatures/`, `scripts/*.sh`, the Mac-only
  `runtime/src/*.mm`, `runtime/translator/`, `runtime/CMakeLists.txt`, the plists,
  `COMPATIBILITY.md`, `BUILDING.md` and `VERSION`. We just don't build them. A
  file we delete conflicts every time upstream edits it.
- **R2: our files live where upstream has none.** That means `windows/`, `docs/`,
  `CLAUDE.md`, `.github/workflows/win-*.yml` and `.github/ISSUE_TEMPLATE/`. The
  front page goes in **`.github/README.md`**: GitHub shows that file in place of
  the root `README.md`, so upstream's README stays untouched. Root files that
  collide (`README.md`, `CONTRIBUTING.md`, `SECURITY.md`,
  `THIRD_PARTY_NOTICES.txt`, `.gitignore`) stay upstream's. Our versions go in
  `windows/` (`windows/package/` already ships `THIRD_PARTY_NOTICES.txt`).
  `.gitignore` takes appended lines only.
- **R3: a change to `runtime/` takes one of three forms:**
  1. **Windows-only class:** goes in `windows/java/` (already a source root of
     `build_java.sh`), not `runtime/java/`. Examples: `WinProcesses`,
     `WindowsBroker`, `MacApkConscryptTls` if it is Windows-only.
  2. **Hook:** the smallest possible edit to an upstream file, so that Windows
     code in `windows/java/` can take over. It is marked `// WINAPK-HOOK: <why>`.
     Target: under 40 files.
  3. **Generic fix** (a bug that exists on the Mac too): carried as a marked edit
     (`// WINAPK-FIX: <why>`) and offered upstream as a PR to kksimp/Mac.apk (this
     needs the CLA). Once a Mac.apk release contains it, the next sync drops ours.
- **R4: one register.** `windows/UPSTREAM_DELTA.md` lists every file under
  `runtime/` that differs from the last merged Mac.apk release: its kind (hook or
  fix), why, and its upstream PR if any. It replaces the 154-item list in
  `windows/README.md`. A CI step (`windows/scripts/check_upstream_delta.sh`) diffs
  `runtime/` against `upstream/main`'s merge base and fails on any changed file
  the register does not list.
- **R5: provenance.** A commit taken from israel2176 is cherry-picked with `-x` and
  gets a trailer `Picked-from: israel2176/win.apk@<sha>` (or `Adapted-from:` when
  its `runtime/` part had to be rewritten to fit R3). `windows/PICKS.md` records
  every israel2176 commit we have reviewed (sha, group, verdict, PR) and the last
  sha reviewed.

## Phases

### Phase 0: decisions and setup (small)

- [ ] **The repository.** GitHub allows one fork of a network per account. If
      airisu-dev/Win.apk is already in Mac.apk's network (through israel2176),
      the "Fork" button will refuse. Options:
      (a) make the new repo under another org, using the Fork button;
      (b) create an ordinary repo and push Mac.apk's history into it. Merging still
      works; only GitHub's ahead/behind view is missing;
      (c) ask GitHub support to detach airisu-dev/Win.apk from the network first.
      **Recommended: (b) or (a).** The soft fork comes from shared git history,
      not from GitHub's fork link.
- [ ] **The name.** "Win.apk" is israel2176's product name, and Mac.apk's
      `TRADEMARKS.md` covers "Mac.apk". Pick a distinct name (for example one that
      says Arm) before the docs and installer are rewritten, since `AppId`, the
      registry key, `%LOCALAPPDATA%` and the AppUserModelIDs all carry it.
- [ ] **License.** The license stays AGPL-3.0. Keep upstream's `LICENSE` and
      `TRADEMARKS.md` as they are. Credit Mac.apk and israel2176 in
      `.github/README.md` and `windows/package/README.txt`.
- [ ] **Remotes and branches:** `upstream` = kksimp/Mac.apk (merged),
      `israel` = israel2176/win.apk (fetched, cherry-picked from, never merged).
      `main` is protected and takes PRs only.

```bash
git clone https://github.com/kksimp/Mac.apk NEW && cd NEW
git remote rename origin upstream
git remote add origin https://github.com/airisu-dev/NEW.git
git remote add israel https://github.com/israel2176/win.apk.git
git remote add old https://github.com/airisu-dev/Win.apk.git
git push -u origin main
```

### Phase 1: bootstrap, docs and the WoA app list (large; your "first phase")

Each step below is one PR to the new repo's `main`, in this order.

1. **Import `windows/` and our own trees.** Copy `windows/`, `docs/`,
   `CLAUDE.md`, `.github/ISSUE_TEMPLATE/`, `.github/probe/` and the workflows
   (renamed `win-ci.yml`, `win-release.yml`, `win-app-probe.yml`) from
   airisu-dev/Win.apk `a2c7b72`. It is one squashed commit, because replaying
   ~450 commits whose `runtime/` halves no longer apply would only add noise. The
   trailer names the source commits: `Imported-from:
   airisu-dev/Win.apk@a2c7b72 (israel2176/win.apk@bc51747 + 19)`. Root files
   follow R2.
2. **Sort the `runtime/` delta** (the bulk of Phase 1). For each of the 245 files
   and 54 new ones, decide its R3 kind and land it in a few PRs, one per area:
   - Windows-only new classes, moved to `windows/java/`;
   - hooks (`android/os/MacApkBinderTransport`, `com/macapk/loader/*`,
     `SystemServer`/`SystemClient`/`AppUids`, the paths, ...), each cut down to
     the minimum;
   - generic fixes, one PR per package (`android/net`, `android/view`,
     `android/app`, `android/media`, `android/bluetooth`, `com/macapk/dex`, ...),
     each listed in the register and queued as a candidate PR to Mac.apk.

   Mac.apk's deleted files come back untouched (`runtime/CMakeLists.txt`, the
   plist, the 6 headers, `runtime/translator/`). `build_java.sh` and
   `windows/CMakeLists.txt` already ignore them. Add the register
   (`UPSTREAM_DELTA.md`) and `check_upstream_delta.sh` to CI in the same series.
3. **CI and release: ARM64 only.** Keep the `windows-11-arm` jobs: build, unit
   tests, smoke test and package. Drop the x64 build, the x64 sweep shards, the
   x64 installer and the `-dev` x64 asset. Release ships only
   `<name>-Setup-arm64.exe` (and a portable zip). The launcher's
   `setup_asset_rank` picks only the arm64 asset.
4. **Docs for an ARM64-only product.** Every page in `docs/` and `docs/he/`, the
   front page (`.github/README.md`), `windows/README.md`, `windows/PORTING.md`,
   `RELEASE_NOTES.md` and `CLAUDE.md`:
   - requirements: Windows 11 on Arm (Snapdragon X and other arm64 PCs); x64 PCs
     are pointed to israel2176's Win.apk;
   - `arm-apps.md` is rewritten as "Which app build to install": `arm64-v8a`
     yes; `x86_64`-only and `armeabi-v7a`-only no, with the message the user
     sees;
   - remove "Speed up ARM apps", the translation, `aot-cache\`, the clang folder
     and the AOT log lines from `USER_GUIDE.md`, `troubleshooting.md`, `faq.md`
     and `how-it-works.md`; move the x18 scan and its log lines up;
   - `ARCHITECTURE.md` keeps "The ARM64 host" as the main path and cuts the
     x86_64 loader, the `%fs` patch and the translator to a note;
   - "Keeping up with upstream" describes `git merge upstream/main` plus the
     register (R4), and a new section describes picks from israel2176 (R5);
   - the screenshots in `docs/images/` are retaken on a WoA PC. Wine on x64
     cannot run the arm64 build.
5. **The tested apps, on WoA.** These are all checked on `windows-11-arm` and,
   for the hand-tested list, on a real Snapdragon PC.
   - `docs/APPS.md`: the ✅/⚠️/❌ table is rechecked against the ARM64 code
     (rows for the translation go away; rows for native speed and the x18 refusal
     come in). "Apps people have tried" starts again with an architecture column;
     rows inherited from x64 testing are kept with an "x64, not rechecked" note
     until retested.
   - `windows/test/compat/apps.txt`: keep only apps whose F-Droid entry ships
     `arm64-v8a` or no native code, and fetch the arm64 build. Note which apps
     drop out and why. `smoke.txt`: ten apps that all draw on `windows-11-arm`,
     including at least one Flutter, one Unity/native game and one WebView app.
   - Record a full sweep on ARM64 in `APPS.md` (run link, n of N drew).
   - `app-probe.yml` runs on `windows-11-arm`.

   **Exit:** CI is green on `windows-11-arm`, 10 of 10 smoke apps draw, the arm64
   installer installs and launches an app, `check_upstream_delta.sh` passes, and
   no doc mentions what was removed.

### Phase 2: remove the x64 code (medium)

Phase 1 stops building and shipping x64. This phase deletes the code.

1. **Translation layer:** `arm64_cpu`, `arm64_simd`, `arm64_guest`, `arm64_jit`,
   `arm64_aot`, `aot_files`, `translator_main`, `aot_guard`, `arm64_aot_main`,
   `include/winapk/arm64_aot.h`, `aot_files.h`, `aot_stamp.h` (with the
   `.aot.stamp` lines the loader and launcher read), `stage_clang.sh`,
   `--aot-translate`, `WINAPK_AOT`, the AOT tests, the dynarmic option and
   `docs/research/aot*`.
2. **x86_64 guest path:** `fs_tls_patch`, `abi_thunk.cpp` (System V), the x86_64
   relocations and `e_machine` branches in `elf_loader`/`linker`, `sysv_abi` in
   the shim, and every `#if WINAPK_HOST_X64` branch (13 files). `arch.h` keeps
   one architecture. Decide whether `WINAPK_HOST_ARM64` stays as a constant or
   goes.
3. **Launcher:** remove the `AotBackground` row and setting, the
   `aot-guard.txt` handling and the x64 asset ranking. Bump `kShortcutVersion`
   only if the AppUserModelIDs change with the new name.
4. **CMake:** a hard error when the target is not ARM64.

Each part is its own PR, and CI must stay green after each one.

### Phase 3: picks from israel2176, one PR per group (medium, ongoing)

These are the 35 israel2176 commits after `bc51747`. The cut point is fixed:
everything before it came in with Phase 1's import. Each group is one PR of
`cherry-pick -x` commits. A commit's `runtime/` part is re-expressed per R3,
because it was written against israel2176's tree, not Mac.apk's.

| # | Group | Commits | Verdict to expect |
|---|---|---|---|
| G1 | Bionic shim: about 50 libc calls, `gethostbyname`, real-time signals, pthread keys, POSIX rename | `940e05f` (shim part), `14c52b1`, `cd3750c` (pthread part), `b78a040` (rename part) | Take. Check `arm64_abi.cpp` for every struct or flag they touch. |
| G2 | NDK: `AMediaFormat`, a sensor manager with no sensors, no-codec answers | `b78a040` (rest) | Take. |
| G3 | DEX converter: a split method's eager new, cloned handlers | `940e05f` (converter part), `1073cf3` (splitter part) | Generic fix (R3.3). Offer upstream. |
| G4 | Framework members popular apps call | `8f313e3`, `3c34703`, `5c1adbe`, `1073cf3` (rest), `cd3750c` (JNI_OnLoad, pills) | Mostly generic fixes. Offer upstream. |
| G5 | Activities: an app with no launcher activity; the phone turns back when a finished activity reveals another | `ea025eb`, `0c4432a` | Take. |
| G6 | Google account added from an app | `8e9e342` | Take (hook or fix). |
| G7 | Camera window for `ACTION_IMAGE_CAPTURE` | `48c5ddf`, `c587fb0`, `c02ea9f` | Take. Test with an arm64 webcam driver. |
| G8 | Diagnostics: Threads window, Ctrl+Shift+F12 snapshot, refused-TLS log | `9b8e450`, `e2135e2`, `55d8ffb` | Take. |
| G9 | Linker with mixed ABIs | `5f072f7`, `4da2494`, `1a88957` | `1a88957` skipped (translation). `5f072f7` adapted: prefer the `arm64-v8a` dependency, refuse x86_64. |
| G10 | GL async readback (opt-in double PBO) | `b3164ab`, `f764a82`, `75f5a34`, `8052025` | Measure on Snapdragon against our BGRA readback. Take only if it wins; it conflicts with `200847a`. |
| G11 | Wine 9 boot test | `dcd0639` | Skip: Wine on x64 cannot run the arm64 build. Revisit if Wine on aarch64 is adopted. |
| G12 | Project site (GitHub Pages) | `71817a3`, `903a722`, `42d166d` | Optional. Adapt to our name. |

After that, keep a **watermark**: the last israel2176 sha reviewed, in
`windows/PICKS.md`. A regular pass (`git log <watermark>..israel/main`) sorts new
commits into groups the same way.

### Phase 4: cut over (small)

- [ ] The new repo's `main` becomes the project. Releases come from its
      `win-release.yml`. The update check (`WINAPK_REPO`) points at it, and
      0.9.0 (or 1.0.0) is the first ARM64-only release.
- [ ] airisu-dev/Win.apk: its README points to the new repo; then it is archived.
      Nothing is force-pushed over the old `main`, so its history stays readable.
- [ ] Installs of the old ARM64 build: if the name, `AppId` or data folder
      change, the installer moves `%LOCALAPPDATA%\Win.apk` and `HKCU\Software\Win.apk`
      on the first run, or the release notes say plainly that they don't.

### Ongoing: each Mac.apk release

1. `git fetch upstream` and open a PR named `sync/macapk-v1.0.NNNN` that runs
   `git merge upstream/main`.
2. Conflicts can only fall on files in the register. Resolve each one, and drop
   any fix the release now contains (from our PR or the same fix found
   independently).
3. Give every native a new class gains an answer in `windows/host/`, as today.
4. Run CI with the smoke test and a sweep, then merge with a merge commit, never a
   rebase or squash, so that the next `git merge upstream/main` finds its base.

## Risks

- **Sorting the 245 files takes time and judgment.** Some of israel2176's
  changes will be unclear (hook or fix?). When unsure, register it as a fix with
  no upstream PR. That costs nothing until the next conflict.
- **Mac.apk's syncs are squashed and large.** A sync that rewrites a hooked file
  conflicts in full. The register keeps those to a short, known list.
- **The CLA.** Offering fixes upstream means signing kksimp's CLA, which grants
  relicensing (Mac.apk sells a commercial license). Decide this once in Phase 0.
  If not, the fixes stay registered here indefinitely.
- **`windows-11-arm` runners** are fewer and slower. The sweep may need more
  shards or a nightly schedule instead of per PR.
- **Fewer apps.** x86_64-only and 32-bit-only apps stop working. `APPS.md` says
  so, and the launcher's install message names the reason.
- **israel2176's later work** may build on code we removed (the translation, x64
  paths). Those commits are skipped or adapted, and the reason is recorded in
  `PICKS.md`.
