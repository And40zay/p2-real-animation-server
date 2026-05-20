#!/usr/bin/env bash
# Create a new Java project skeleton under a given directory name
# Usage: ./create_java_project.sh mynewproject

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

mkdir -p "$PROJ/src"
mkdir -p "$PROJ/bin"
cat > "$PROJ/src/Main.java" <<'JAVA'
public class Main {
    public static void main(String[] args) {
        System.out.println("Hello from new Java project!");
    }
}
JAVA

echo "Java project created at $PROJ"

echo "To compile: javac -d $PROJ/bin $PROJ/src/*.java"
echo "To run: java -cp $PROJ/bin Main"
