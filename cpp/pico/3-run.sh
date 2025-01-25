#!/bin/bash
set -euo pipefail  # Exit on any error

cd "$(dirname "$0")"

outfile="a=picobench+date=$(date '+%Y-%m-%d')+ext=.csv"
echo "Output file: ${outfile}"

echo "LED active indicates benchmark is ready..."
echo "Press the BOOTSEL button on the pico board to begin benchmark!"
echo "LED will re-activate when complete."

minicom -D /dev/ttyACM0 -b 1152 -C "${outfile}"
