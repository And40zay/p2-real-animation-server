#!/usr/bin/env bash
# Create a new C file with a basic template
# Usage: create_c_file.sh name.c [dir (optional)]
set -euo pipefail
if [ "$#" -lt 1 ]; then
  echo "Usage: $0 name.c [dir]"
  exit 1
fi
NAME=$1
DIR=${2:-.}
mkdir -p "$DIR"
if [ -e "$DIR/$NAME" ]; then
  echo "File already exists: $DIR/$NAME"
  exit 1
fi
cat > "$DIR/$NAME" <<'C'
#include <stdio.h>

// Note: IntelliCode and autosuggestions are disabled for C files in this workspace.
// This is to help you practice coding without IDE assistance.
// To toggle suggestions, run: Terminal -> Run Task -> Toggle C Suggestions

int main(void) {
    printf("Hello from new C file!\n");
    return 0;
}
C

echo "Created $DIR/$NAME"
echo "To compile: gcc -Wall -Wextra -g -O0 $DIR/$NAME -o $DIR/${NAME%.c}"
echo "To run: $DIR/${NAME%.c}"
chmod +x "$DIR/$NAME"
