# Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.24)
project(noctalia-windows VERSION 0.0.1 LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR "noctalia-windows only builds on Windows.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NOCTALIA_WIN_BUILD_PROBES "Build the small Windows shell probes" ON)

if(NOCTALIA_WIN_BUILD_PROBES)
  add_subdirectory(probes)
endif()

# Upstream Noctalia core will be added here (submodule) once the platform seams are mapped.
