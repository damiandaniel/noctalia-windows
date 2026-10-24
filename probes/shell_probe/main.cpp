// Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors
// SPDX-License-Identifier: MIT

// Reports which process currently holds the Windows shell role.
#include <windows.h>
#include <tlhelp32.h>

#include <cstdio>
#include <string>

static int count_processes(const wchar_t* name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE) return -1;
  PROCESSENTRY32W pe{};
  pe.dwSize = sizeof(pe);
  int n = 0;
  for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
    if (_wcsicmp(pe.szExeFile, name) == 0) ++n;
  }
  CloseHandle(snap);
  return n;
}

static std::wstring process_of(HWND hwnd) {
  if (!hwnd) return L"-";
  DWORD pid = 0;
  GetWindowThreadProcessId(hwnd, &pid);
  HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (!h) return L"pid " + std::to_wstring(pid);
  wchar_t path[MAX_PATH]{};
  DWORD len = MAX_PATH;
  std::wstring out = QueryFullProcessImageNameW(h, 0, path, &len) ? path : L"?";
  CloseHandle(h);
  return out + L" (pid " + std::to_wstring(pid) + L")";
}

static std::wstring winlogon_shell(HKEY root) {
  wchar_t buf[512]{};
  DWORD size = sizeof(buf);
  LSTATUS rc = RegGetValueW(root, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                            L"Shell", RRF_RT_REG_SZ, nullptr, buf, &size);
  return rc == ERROR_SUCCESS ? buf : L"(not set)";
}

int wmain() {
  HWND shell = GetShellWindow();
  HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
  HWND progman = FindWindowW(L"Progman", nullptr);

  wprintf(L"GetShellWindow : %p  %ls\n", static_cast<void*>(shell), process_of(shell).c_str());
  wprintf(L"Shell_TrayWnd  : %p  %ls\n", static_cast<void*>(tray), process_of(tray).c_str());
  wprintf(L"Progman        : %p  %ls\n", static_cast<void*>(progman), process_of(progman).c_str());
  wprintf(L"explorer.exe   : %d process(es)\n", count_processes(L"explorer.exe"));
  wprintf(L"HKCU Shell     : %ls\n", winlogon_shell(HKEY_CURRENT_USER).c_str());
  wprintf(L"HKLM Shell     : %ls\n", winlogon_shell(HKEY_LOCAL_MACHINE).c_str());
  return 0;
}
