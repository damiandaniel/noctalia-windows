<!-- Copyright (c) 2026 damiandaniel and contributors. MIT License. -->
# First build (stripped)

The goal: try to **compile** upstream Noctalia on Windows without its Linux-only folders. It will
not run, and it will not link. What you get is a list of what breaks. That list is our real
to-do list.

## 1. Set up MSYS2

Install [MSYS2](https://www.msys2.org) and open the **MSYS2 UCRT64** shell. Then:

```bash
pacman -Syu
pacman -S --needed git python mingw-w64-ucrt-x86_64-{gcc,meson,ninja,pkgconf}
pacman -S --needed mingw-w64-ucrt-x86_64-{cairo,pango,harfbuzz,freetype,fontconfig,glib2,librsvg}
pacman -S --needed mingw-w64-ucrt-x86_64-{curl,libwebp,libjxl,libsndfile,libxml2,md4c,tomlplusplus}
pacman -S --needed mingw-w64-ucrt-x86_64-{libical,nlohmann-json,libsodium,libxkbcommon,stb}
```

- Some package names may be different. Search with `pacman -Ss <name>`.
- I did not check if `libqalculate` and ANGLE (`pacman -Ss angle`) exist there. Skip them for now.
- Check `gcc --version`. Upstream uses C++23 `<print>`, which needs **GCC 14 or newer**. GCC 13
  failed on it in my Linux test.

## 2. Add upstream

From the root of this repo:

```bash
git submodule add https://github.com/noctalia-dev/noctalia third_party/noctalia
git -C third_party/noctalia checkout 5f864d4
```

`5f864d4` is the commit I read for `upstream-seams.md` (version 5.3.0, 2026-10-23). Upstream keeps
its vendored code in its own tree, so no extra submodules are needed.

## 3. Generate and build

```bash
python scripts/gen-stripped-meson.py
meson setup build-stripped
ninja -C build-stripped -k 0 2>&1 | tee build-stripped/log.txt
```

- The script reads upstream's source list and drops the Linux-only folders. In my test that was
  608 files, 120 dropped, 488 kept.
- `meson setup` prints `missing dependency: ...` for every library it can't find. Install those, or
  leave them. Missing ones just add to the error list.
- `-k 0` means "keep going after errors", so you see everything and not just the first failure.
- Don't edit the generated `meson.build`. Run the script again instead.

## 4. Read the result

```bash
# how many files failed
grep -c '^FAILED' build-stripped/log.txt

# which headers are missing, most common first
grep -oE "fatal error: [^:]+: No such file" build-stripped/log.txt | sort | uniq -c | sort -rn

# other errors
grep -E "error:" build-stripped/log.txt | grep -v "fatal error" | sed 's/^[^ ]* //' | sort | uniq -c | sort -rn | head -20
```

The compiler stops at the **first** missing header in each file. So fix the top ones, run again,
and the list will change. Expect a few rounds.

Sort what you find into four piles:

1. **Missing library.** Install it, or find a Windows replacement.
2. **Wayland type or header** in code we kept. These are the seams in `upstream-seams.md`.
3. **POSIX call or header** (`unistd.h`, `poll.h`, `sys/*`). These need a Windows version.
4. **Something else.** Windows macros like `min`, `max` and `ERROR` are common ones.

## What I saw on Linux

This is only a preview. It is not a Windows result. I ran the same setup on Linux with many
libraries missing and stopped it after 280 seconds:

- 309 of 489 steps done, 182 of them failed.
- Most failures were missing libraries (`toml++` 95 files, `curl` 21, `GLES2` 20).
- A few kept files still include Wayland protocol headers (`cursor-shape` 5 files,
  `wayland-client-protocol` 2, `xdg-shell` 1, `viewporter` 1). These are real leaks outside the
  folders we dropped.
- `<print>` was missing, which is the GCC 14 point above.

## What "done" looks like

- Every kept file either compiles, or has a reason on the list.
- The list is sorted into the four piles above.
- Nothing links and nothing runs. That comes later.

## Housekeeping

Add these to `.gitignore`, since the Meson file and its build folder are generated:

```
/meson.build
build-stripped/
```
