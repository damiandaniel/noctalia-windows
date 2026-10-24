<!-- Copyright (c) 2026 damiandaniel and contributors. MIT License. -->
# How it is built

Notes on how the port is put together. Anything marked **(not checked yet)** is a guess. We still
need to test it with the probes or read the upstream code.

## Three layers

1. **Upstream core.** Drawing, widgets, themes, TOML config, IPC and plugins. It comes from
   Noctalia v5. We change as little of it as we can.
2. **Windows layer (this repo).** It does the jobs the core now gets from Wayland and Linux
   services.
3. **Shell role.** The jobs Explorer normally does: the tray, the desktop, hotkeys, sign out and
   shut down.

Upstream works with many compositors. Its `WorkspaceBackend` class is a good place for a Windows
window and workspace backend. But the wider compositor layer leaks Wayland types, and the surface
classes matter more. The details are in [upstream-seams.md](upstream-seams.md).

## What maps to what

| Noctalia (Linux) | Windows | Notes |
|---|---|---|
| layer-shell bars and panels | AppBar API and borderless tool windows | Keeps room on the screen edge |
| Wayland + GLES | EGL on ANGLE (GLES on top of D3D11) | |
| See-through and blur | DirectComposition, plus DWM backdrop or acrylic for blur | Biggest risk for an identical look |
| Compositor IPC (windows, workspaces) | `EnumWindows`, DWM thumbnails, virtual desktops | Windows does not document virtual desktops, and they change between builds |
| PipeWire | WASAPI | |
| MPRIS | GSMTC (WinRT) | |
| BlueZ and NetworkManager | WinRT Bluetooth, WLAN API | |
| Notification daemon | Notification listener bridge | See below |
| StatusNotifierItem tray | `Shell_TrayWnd` host | Only works if we are the shell |
| App list for the launcher | Start Menu shortcuts and `shell:AppsFolder` | |
| Unix socket IPC | Named pipes | |
| Screenshots | Windows.Graphics.Capture | |
| Wallpaper and desktop widgets | `IDesktopWallpaper`, `Progman` and WorkerW | |

## The shell role

Noctalia is set as the Winlogon shell. Then it has to do what Explorer did:

- Make a window with the class name `Shell_TrayWnd`. Apps send their tray icon messages to it.
  Also broadcast `TaskbarCreated`, so running apps add their icons again. Windows does not
  document the message format, so `tray_probe` prints the raw bytes.
- Make the `Progman` and WorkerW desktop windows, for the wallpaper and desktop widgets.
- Handle the Win key and Alt+Tab.
- Handle lock, sleep, sign out and shut down.

Explorer has no way to hand its tray icons to another program. So it can't be our tray.
Noctalia has to be the tray.

### Explorer in the background (optional)

Explorer may still be able to run quietly, without being the shell. That might keep some things
working that expect it, like pop-up notifications and the Action Center. It is also handy for file
windows. We don't know yet how current Windows acts when a second `explorer.exe` starts while
another shell is set (not checked yet).

## Notifications

The idea: let Windows get the notification, hide its pop-up with Focus Assist, read it with
`UserNotificationListener`, and show it in Noctalia. Two questions (not checked yet):

- Does the listener still see notifications when the pop-ups are hidden?
- Can a plain Win32 app use it, or does it need a package identity? (A sparse package can give it one.)

## Hard parts

- See-through windows with GL drawing.
- Virtual desktops, because Windows does not document them.
- Taskbar features like progress bars and badges (`ITaskbarList3`). Explorer normally serves these.
- Typing in other languages (IME) and screen reader support, in a UI that draws everything itself.
