// Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors
// SPDX-License-Identifier: MIT

// Stub Shell_TrayWnd host: logs raw messages apps send via Shell_NotifyIcon.
// Only meaningful where Explorer's taskbar is NOT running (see docs/dev-environment.md).
#include <windows.h>

#include <cstdio>

static void dump(const COPYDATASTRUCT* cds) {
  wprintf(L"WM_COPYDATA dwData=%llu cbData=%lu\n",
          static_cast<unsigned long long>(cds->dwData), cds->cbData);
  const auto* p = static_cast<const unsigned char*>(cds->lpData);
  for (DWORD i = 0; p && i < cds->cbData && i < 64; ++i) {
    wprintf(L"%02x%ls", p[i], (i % 16 == 15) ? L"\n" : L" ");
  }
  wprintf(L"\n");
}

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
    case WM_COPYDATA:
      dump(reinterpret_cast<const COPYDATASTRUCT*>(lp));
      return TRUE;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProcW(hwnd, msg, wp, lp);
  }
}

int wmain() {
  if (HWND existing = FindWindowW(L"Shell_TrayWnd", nullptr)) {
    wprintf(L"Shell_TrayWnd already exists (%p); results will be unreliable.\n",
            static_cast<void*>(existing));
  }

  WNDCLASSW wc{};
  wc.lpfnWndProc = wnd_proc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"Shell_TrayWnd";
  if (!RegisterClassW(&wc)) {
    wprintf(L"RegisterClass failed: %lu\n", GetLastError());
    return 1;
  }

  HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0,
                              nullptr, nullptr, wc.hInstance, nullptr);
  if (!hwnd) {
    wprintf(L"CreateWindow failed: %lu\n", GetLastError());
    return 1;
  }

  // Tell running apps to re-add their tray icons.
  UINT taskbar_created = RegisterWindowMessageW(L"TaskbarCreated");
  SendNotifyMessageW(HWND_BROADCAST, taskbar_created, 0, 0);
  wprintf(L"Listening on Shell_TrayWnd. Close the console to exit.\n");

  MSG m;
  while (GetMessageW(&m, nullptr, 0, 0) > 0) {
    TranslateMessage(&m);
    DispatchMessageW(&m);
  }
  return 0;
}
