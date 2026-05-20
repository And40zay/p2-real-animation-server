#!/usr/bin/env bash
# Run a single source file. Detects extension (.c or .java) and compiles+runs.
# Usage: scripts/run.sh path/to/file.c or scripts/run.sh src/Main.java
set -euo pipefail
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 path/to/file.c|src/Main.java"
    exit 1
fi
FILE=$1
EXT=${FILE##*.}
case "$EXT" in
  c)
    BIN_NAME=$(basename "${FILE%.c}")
    mkdir -p build/Debug
    gcc -Wall -Wextra -g -O0 "$FILE" -o "build/Debug/${BIN_NAME}"
    ./build/Debug/${BIN_NAME}
    ;;
  java)
    # Extract class name
    NAME=$(basename "${FILE%.java}")
    mkdir -p bin
    javac -d bin "$FILE"
    java -cp bin "$NAME"
    ;;
  *)
    echo "Unsupported extension: $EXT"
    exit 2
    ;;
esac
