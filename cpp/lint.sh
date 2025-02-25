#!/bin/bash
set -euo pipefail  # Exit on any error

cd "$(dirname "$0")"

# C++ linting with clang-tidy
echo "Running C++ linting..."
find \
    "native/main.cpp" "pico/main.cpp" \
    "include/" -type f -name "*.hpp" \
| while read -r file; do
    clang-tidy "$file" -- -std=c++23 -I.
done

echo "All lint checks passed! ✨"
