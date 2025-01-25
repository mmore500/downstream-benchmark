#!/bin/bash
set -euo pipefail  # Exit on any error

cd "$(dirname "$0")"

cmake .
make
