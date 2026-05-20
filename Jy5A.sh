#!/usr/bin/env bash
# Create a small C project skeleton under a given directory name
# Usage: ./create_c_project.sh mycproject

set -euo pipefail
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 project_name"
    exit 1
fi

PROJ=$1
if [ -d "$PROJ" ]; then
    echo "Directory $PROJ already exists"
    exit 1
fi

mkdir -p "$PROJ"
cat > "$PROJ/main.c" <<'C'
#include <stdio.h>

int main(void) {
    printf("Hello from new C project!\n");
    return 0;
}
C

echo "C project created at $PROJ"

echo "To compile: gcc -Wall -Wextra -g -O0 $PROJ/main.c -o $PROJ/main"
echo "To run: ./$PROJ/main"
