#!/bin/bash
set -euo pipefail  # Exit on any error

cd "$(dirname "$0")"

rm -rf CMakeFiles pico CMakeCache.txt Makefile build
