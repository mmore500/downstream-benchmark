#!/bin/bash
set -euo pipefail  # Exit on any error

cd "$(dirname "$0")"

# Format C++ files
echo "Formatting C++ files..."
find \
    "native/main.cpp" "pico/main.cpp" \
    "include/" -type f -name "*.hpp" \
| while read -r file; do
    clang-format -style=file -i "$file"
done

# Check for trailing whitespace and missing final newlines
echo "Checking for whitespace issues..."
! find . -type f \( -name "*.hpp" -o -name "*.cpp" \) -exec grep -l "[[:blank:]]$" {} \;

# Ensure all files end with newline
echo "Checking for final newlines..."
! find . -type f \( -name "*.hpp" -o -name "*.cpp" \) -print0 | \
    while IFS= read -r -d '' file; do
        if [ -s "$file" ] && [ "$(tail -c1 "$file"; echo x)" != $'\nx' ]; then
            echo "No newline at end of $file"
            exit 1
        fi
    done

echo "All style checks passed! ✨"
