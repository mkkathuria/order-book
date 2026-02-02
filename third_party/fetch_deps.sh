#!/bin/bash
# Run this once before building to pull in the nlohmann/json single header.
# No other external dependencies are needed.
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TARGET="$SCRIPT_DIR/nlohmann"
mkdir -p "$TARGET"
if [ ! -f "$TARGET/json.hpp" ]; then
    echo "Downloading nlohmann/json..."
    curl -sL "https://raw.githubusercontent.com/nlohmann/json/master/single_include/nlohmann/json.hpp" \
         -o "$TARGET/json.hpp"
    echo "Done. nlohmann/json.hpp is ready."
else
    echo "nlohmann/json.hpp already present."
fi
