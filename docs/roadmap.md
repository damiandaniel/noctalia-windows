<!-- Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors. MIT License. -->
# Roadmap

## M0: Probes (small tests)

- [x] `shell_probe`: shows the shell window, the tray window, Explorer processes and the Winlogon shell values
- [x] `tray_probe`: a fake `Shell_TrayWnd` that prints the raw tray messages
- [ ] Run both on Windows 10 and 11 (a few builds) and write down what happens
- [ ] Test: what does a second `explorer.exe` do when another shell is set?
- [ ] Test: does the notification listener work with pop-ups hidden, and does it need a package identity?

## M1: Drawing

- [ ] Find where the Windows code plugs into upstream (windows, compositor backend, services)
- [ ] An ANGLE EGL context on a window
- [ ] A see-through AppBar window with DirectComposition
- [ ] The upstream bar drawn pixel for pixel like the Linux one
- [ ] Pick a way to do blur

## M2: Services

- [ ] Sound (WASAPI)
- [ ] Media (GSMTC)
- [ ] Power and battery
- [ ] Network and Bluetooth
- [ ] Window list and switcher (DWM thumbnails)
- [ ] Workspaces with virtual desktops
- [ ] Config paths and named-pipe IPC

## M3: Shell role

- [ ] Set as the Winlogon shell (one user first)
- [ ] `Shell_TrayWnd` tray
- [ ] Desktop windows, wallpaper and desktop widgets
- [ ] Win key, Alt+Tab and global hotkeys
- [ ] Lock, sleep, sign out and shut down
- [ ] Optional Explorer in the background

## M4: Notifications

- [ ] Read Windows notifications and show them in the Noctalia notification UI

## M5: Finish

- [ ] Installer and uninstaller that can safely go back to Explorer
- [ ] Many monitors and mixed DPI
- [ ] Typing in other languages (IME) and screen readers
- [ ] A simple way to follow upstream releases
