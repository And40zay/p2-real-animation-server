#!/usr/bin/env bash
# Create a new Java file with a basic template
# Usage: create_java_file.sh ClassName.java [dir (optional)]
set -euo pipefail
if [ "$#" -lt 1 ]; then
  echo "Usage: $0 ClassName.java [dir]"
  exit 1
fi
NAME=$1
DIR=${2:-src}
# Ensure the class name and filename match (strip .java)
BASE=${NAME%.java}
mkdir -p "$DIR"
if [ -e "$DIR/$NAME" ]; then
  echo "File already exists: $DIR/$NAME"
  exit 1
fi
cat > "$DIR/$NAME" <<JAVA
public class $BASE {
    public static void main(String[] args) {
        System.out.println("Hello from new Java class $BASE!");
    }
}
JAVA

echo "Created $DIR/$NAME"
echo "To compile: javac -d bin $DIR/$NAME"
echo "To run: java -cp bin $BASE"
