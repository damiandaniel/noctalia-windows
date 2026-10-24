# Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors
# SPDX-License-Identifier: MIT

# Removes the per-user shell override so Explorer is the shell again.
$key = 'HKCU:\Software\Microsoft\Windows NT\CurrentVersion\Winlogon'
Remove-ItemProperty -Path $key -Name Shell -ErrorAction SilentlyContinue
Write-Host "Per-user shell override removed. Log off and on to apply."
