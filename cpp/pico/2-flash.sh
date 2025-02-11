#!/bin/bash
set -euo pipefail  # Exit on any error

cd "$(dirname "$0")"

"${HOME}/.pico-sdk/picotool/2.1.0/picotool/picotool" \
    load ./main.elf -fx
