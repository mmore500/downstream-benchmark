#!/bin/bash
set -euo pipefail  # Exit on any error

cd "$(dirname "$0")"

rm -rf CMakeFiles pico CMakeCache.txt Makefile buildcMakeDoxyfile.in CMakeDoxygenDefaults.cmake compile_commands.json main.{bin,elf,elf.map,hex,uf2} pico_flash_region.ld generated pico-sdk
