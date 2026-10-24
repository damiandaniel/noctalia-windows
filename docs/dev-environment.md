<!-- Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors. MIT License. -->
# Dev setup

## Don't test on your main account

If the shell breaks, you sign in to an empty screen. Use a spare local user or a VM. A VM is best,
because you can use snapshots.

## Per-user shell setting

Windows reads the shell from `Winlogon\Shell`. A value under `HKCU` should only change it for one
user (check this on your build):

```
HKCU\Software\Microsoft\Windows NT\CurrentVersion\Winlogon
    Shell (REG_SZ) = C:\path\to\noctalia.exe
```

Two scripts help:

```powershell
.\scripts\set-test-shell.ps1 -ShellPath C:\path\to\noctalia.exe
.\scripts\restore-explorer-shell.ps1
```

Sign out and back in for either one to take effect.

## If something goes wrong

If you see an empty screen:

1. Press `Ctrl+Alt+Del` and open **Task Manager**.
2. Choose **Run new task** and type `explorer.exe`.
3. Run `scripts\restore-explorer-shell.ps1`.

If Task Manager won't open, sign in as another user and remove the `Shell` value from the broken
user's registry settings. Or put back the VM snapshot.

## Everyday work

While you work on the drawing code, run your builds in a normal Explorer session. That is only for
development. It is not a mode we support.

## Running the probes

```powershell
.\build\probes\Release\shell_probe.exe     # safe anywhere
.\build\probes\Release\tray_probe.exe      # only where Explorer's taskbar is not running
```

`tray_probe` makes a second `Shell_TrayWnd`. In a normal Explorer session you can't trust what it
shows, because apps use the first one they find.
