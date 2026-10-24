# Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors
# SPDX-License-Identifier: MIT

add_executable(shell_probe shell_probe/main.cpp)
target_link_libraries(shell_probe PRIVATE advapi32 user32)

add_executable(tray_probe tray_probe/main.cpp)
target_link_libraries(tray_probe PRIVATE user32)

foreach(t shell_probe tray_probe)
  target_compile_definitions(${t} PRIVATE UNICODE _UNICODE)
  if(MSVC)
    target_compile_options(${t} PRIVATE /W4 /utf-8)
  endif()
endforeach()
