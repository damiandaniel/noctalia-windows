# Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors
# SPDX-License-Identifier: MIT

# Sets a per-user Winlogon shell. Run ONLY in a throwaway account or VM.
param(
  [Parameter(Mandatory = $true)][string]$ShellPath,
  [switch]$Force
)

$key = 'HKCU:\Software\Microsoft\Windows NT\CurrentVersion\Winlogon'
if (-not (Test-Path $ShellPath)) { throw "Not found: $ShellPath" }
if (-not $Force) {
  Write-Host "This replaces the shell for user '$env:USERNAME' at next logon."
  Write-Host "Recovery: Ctrl+Alt+Del > Task Manager > Run new task > explorer.exe"
  if ((Read-Host "Type YES to continue") -ne 'YES') { exit 1 }
}
if (-not (Test-Path $key)) { New-Item -Path $key -Force | Out-Null }
Set-ItemProperty -Path $key -Name Shell -Value (Resolve-Path $ShellPath).Path
Write-Host "Shell set. Log off and on to apply."
