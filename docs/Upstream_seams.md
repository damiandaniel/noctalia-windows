<!-- Copyright (c) 2026 damiandaniel and contributors. MIT License. -->
# Upstream seams

Where the Linux-only code lives in Noctalia, and where Windows code can plug in.

## What I read

- Noctalia **v5.3.0**, commit `5f864d4` (2026-10-23), shallow clone.
- Read in full: `ARCHITECTURE.md`, `BUILDING.md`, parts of `CONTRIBUTING.md` and `PACKAGING.md`.
- Read the headers for the poll loop, `CompositorPlatform`, `WorkspaceBackend` and the keyboard and
  output backends.
- Everything else is **grep counts**, not a code read. Numbers are rough. Treat "not read" as a
  real gap.

Not read in detail: most of `shell/`, the `wayland/` implementation, `capture/`, `system/`, `theme/`.

## Short version

**Good news**

- Drawing is boxed in. Windows surfaces are created in one place (`gles_render_backend.cpp`).
  Text uses Pango and Cairo, which exist on Windows.
- Config, scene graph, themes, Luau plugins, calendar and launcher are mostly plain C++.
- `WorkspaceBackend` is a real interface with several backends already. A Windows one fits.

**Bad news**

- Wayland types spread much further than I guessed. `CompositorPlatform` is **not** a clean seam.
  Its public API passes raw `wl_output*`, `wl_surface*` and `zwlr_*` pointers.
- 88 of 574 files in `shell/` and `ui/` mention `wl_output`. 54 mention `wl_surface`.
- The event loop is built on POSIX `poll()` and `pollfd`, in 35 classes.
- Upstream builds with **Meson and C++23**, not CMake.

**Policy**

- `PACKAGING.md` says "Linux only". `CONTRIBUTING.md` lists "Direct Wayland + OpenGL ES only" as a
  design principle. Whether upstream wants Windows support at all is a team decision, not a
  technical one.

## Rough size

About 340,000 lines of C++ in `src/`. About 70,000 of them (a bit over a fifth) are platform code
we would replace. That counts about half of `system/`, which is a guess.

| Folder | Lines | Plan |
|---|---|---|
| `shell/` | 137k | Keep. About 89 real Wayland calls to fix. Drop the lock screen. |
| `ui/` | 31k | Keep. Uses Linux button codes (`BTN_LEFT` and friends). |
| `render/` | 21k | Keep. Change the surface and display setup. |
| `config/`, `theme/`, `scripting/` | 16k, 13k, 14k | Keep. Path code needs a Windows version. |
| `calendar/`, `launcher/` | 6k, 3k | Keep. App list code needs a Windows version. |
| `dbus/` | 18k | Replace with Windows services. |
| `wayland/` | 12k | Replace. |
| `compositors/` | 13k | Replace (window and workspace backends). |
| `capture/` | 10k | Replace the capture part. Annotation code may stay (not checked). |
| `pipewire/` | 5.5k | Replace with WASAPI. |
| `system/` | 15k | Split. Some of it (CPU, disk, battery) needs Windows versions. |
| `idle/`, `security/`, `auth/` | 1.4k, 1.9k, 0.9k | Replace. |
| `app/` | 6k | Change the main loop. |

## The seams

### 1. Window surface and GL context (easy)

- `render/backend/gles_render_backend.cpp` makes a `wl_egl_window`, then calls
  `eglCreateWindowSurface` (around lines 194 to 204).
- `render/gl_shared_context.cpp` calls `eglGetDisplay` with the `wl_display` (around line 180).
- Plan: ANGLE gives us EGL on Windows. Give it an `HWND` or a DirectComposition surface instead.
- `createDefaultRenderBackend()` and `createDefaultTextureManager()` already exist as factories.
- 10 references to dmabuf, GBM or EGL images need a look (not checked).

### 2. Surface classes (medium, the real seam)

- `wayland/surface`, `layer_surface`, `popup_surface`, `toplevel_surface`.
- Shell code uses `LayerSurface` in 42 files and `PopupSurface` in 25.
- Plan: write Windows versions with the same class names and methods. A `LayerSurface` becomes an
  AppBar window. A `PopupSurface` becomes a tool window.
- This is better than changing 40 callers.

### 3. Wayland handles used as IDs (medium, needs an upstream talk)

- `wl_output` is mentioned about 300 times and `wl_surface` about 180 times in `shell/` and `ui/`.
- Most uses just pass the pointer around to say "which monitor" or "which window".
- Plan A (no upstream change): a small shim header on Windows that declares `struct wl_output` and
  `struct wl_surface` as our own types (holding an `HMONITOR` or `HWND`). It's ugly but small.
- Plan B (cleaner): upstream adds neutral `OutputId` and `SurfaceId` types. That's a big change in
  their code, so they need to agree to it.
- I'd start with Plan A and propose Plan B in pieces.

### 4. Real Wayland calls in shell code (medium)

About 89 call sites, in a short list of files:

- `shell/lockscreen/` (ext-session-lock). We drop it.
- `shell/tray/tray_menu.cpp`, `shell/switcher/window_switcher.cpp`, `shell/panel/panel_manager.cpp`
  (frame callbacks and commits).
- `shell/bar/bar.cpp` and `shell/dock/dock.cpp` (display roundtrips).
- `shell/backdrop/`, `shell/panel/panel_click_shield.cpp` (shared-memory buffers).

Each one needs a look. None is large.

### 5. Compositor layer (medium to hard)

- `WorkspaceBackend` (180 lines) is a proper abstract class with `Workspace` and `WorkspaceWindow`
  structs. Windows backend: virtual desktops plus `EnumWindows`.
- `KeyboardLayoutBackend` and `OutputLifecycleObserver` are small. Windows versions are easy.
- Baggage: both include `<poll.h>` and take `wl_output*`.
- `CompositorPlatform` itself (dozens of methods) mixes Wayland state with window and output
  logic. Plan: our own implementation behind the same class name, same as seam 2.
- `ToplevelInfo` holds `zwlr_foreign_toplevel_handle_v1*`. The window list and switcher use it
  (4 shell files include `wayland_toplevels.h`).

### 6. Event loop (hard, needs an upstream talk)

- `PollSource` exposes `std::vector<pollfd>` and POSIX `poll()`. There are 35 subclasses.
- On Windows, `poll` (WSAPoll) only works on sockets. Other things (events, pipes, timers, window
  messages) are `HANDLE`s.
- Plan A: keep the interface. Each Windows source runs on its own thread and wakes the loop
  through a loopback socket. Works, but it's clumsy.
- Plan B: change the interface to a neutral "wait handle" and use
  `MsgWaitForMultipleObjectsEx`, which also pumps window messages. Cleaner. Touches all 35.
- Windows windows need a message pump on their thread either way.

### 7. System services (medium each)

| Linux (`dbus/`, `pipewire/`) | Lines | Windows |
|---|---|---|
| network (NetworkManager) | 5.5k | WLAN API, network list manager |
| mpris | 3.4k | GSMTC |
| tray (StatusNotifier) | 2.3k | `Shell_TrayWnd` host |
| bluetooth (BlueZ) | 1.8k | WinRT Bluetooth |
| upower, power | 1.6k | power status APIs |
| notification | 1k (+2.4k in `notification/`) | notification listener bridge |
| polkit, secret, accounts, logind | 1.5k | UAC, Credential Manager, session APIs |
| modem | 0.5k | skip at first |
| pipewire | 5.5k | WASAPI |

Upstream backends already have `isAvailable()` checks. A first Windows build can return
"not available" for most of these and still show a bar.

### 8. Input and text (medium)

- `BTN_LEFT`, `KEY_S` and similar come from `<linux/input-event-codes.h>`. A small shim header
  covers them.
- Keyboard code uses xkbcommon (about 17 files). Windows key events must become the same key
  codes. IME needs a separate piece later (`wayland/text_input_service`).

### 9. Paths, processes, desktop entries (easy, but spread out)

- XDG paths show up in close to 60 files. We need one path layer (`%APPDATA%`, `%LOCALAPPDATA%`).
- POSIX headers (`unistd.h`, `sys/*`) show up in about 70 files. `core/process` wraps some of the
  process code. I haven't checked how much is left outside it.
- `.desktop` files, icon themes and `freedesktop_key_file` (in `system/`) become Start Menu
  shortcuts and `shell:AppsFolder`.

### 10. Text and fonts (probably easy)

- Cairo text code calls Fontconfig and `pango_fc_font_map_*` directly.
- Pango and Fontconfig both have Windows builds. Not checked whether the Pango build we get on
  Windows keeps the Fontconfig path. This is a quick test.

## What will not port

- **Lock screen** (`shell/lockscreen/`). Non-goal. Windows owns this.
- `compositors/umbriel`, `dwl`, `niri`, `sway`, `hyprland`, `labwc`, `mango`, `kde`. Wayland only.
- Idle and screen-saver code for D-Bus. Windows has its own.
- Polkit and PAM auth.

## Build system

- Upstream: Meson, C++23, 680 `.cpp` files listed by hand in `meson.build`, and no Windows
  conditionals at all.
- Our skeleton uses **CMake**. That only fits the small probes. For the real port we should use
  Meson too, so we can follow upstream.
- Most libraries have Windows builds (through MSYS2 or vcpkg), but I haven't checked each one.
  `libqalculate`, `libjxl` and `librsvg` need a check. `sdbus-cpp`, PipeWire, WirePlumber, Wayland,
  polkit, PAM and libsecret are Linux-only, and we replace them.
- Compiler: GCC or Clang. MSVC with C++23 and `std::format` is not checked.

## Questions for upstream

1. Does the team want Windows support in the main repo at all, given "Linux only"?
2. Would they take small neutral refactors (Plan B in seams 3 and 6) before any Windows code?
3. Meson on Windows: OK to add platform folders and options to `meson.build`?
4. Should the Windows port live as a fork that tracks releases, or in-tree behind an option?

## Suggested order

1. Test the open questions (probes, Pango with Fontconfig on Windows).
2. Build upstream on Windows with all Linux parts stubbed out. Fails at first. The list of errors
   is our real to-do list.
3. Seam 1 and seam 2: one transparent bar window, drawn by the real upstream renderer.
4. Seams 5 and 6 (outputs, event loop). Then the first services.
5. The shell role (`Shell_TrayWnd`, desktop).
