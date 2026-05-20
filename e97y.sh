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
# If the user only provides a filename without any directory and the default dir doesn't exist,
# prefer to use projects/java/src if it exists in the repo. This helps beginners put files in
# the example project area where other Java source files are stored.
if [ "$DIR" = "src" ] && [ -d "projects/java/src" ]; then
  DIR="projects/java/src"
fi
# Ensure the class name and filename match (strip .java)
BASE=${NAME%.java}
mkdir -p "$DIR"
if [ -e "$DIR/$NAME" ]; then
  echo "File already exists: $DIR/$NAME"
  exit 1
fi

# Compute package based on the selected source root. Prefer projects/java/src, then projects/java, then src.
PACKAGE=""
ROOT=""
if [[ "$DIR" == projects/java/src* ]]; then
  ROOT="projects/java/src"
elif [[ "$DIR" == projects/java* ]]; then
  ROOT="projects/java"
elif [[ "$DIR" == src* ]]; then
  ROOT="src"
fi
if [ -n "$ROOT" ]; then
  # base is the root expressed as package (path segments -> dot)
  if [ "$ROOT" = "src" ]; then
    BASE_PKG=""
  else
    BASE_PKG=$(echo "$ROOT" | sed 's#/#.#g')
  fi
  # suffix is the part of the path after the root
  SUFFIX="${DIR#$ROOT}"
  SUFFIX=${SUFFIX#/}
  if [ -n "$SUFFIX" ]; then
    SUFFIX_PKG=$(echo "$SUFFIX" | sed 's#/#.#g')
    if [ -n "$BASE_PKG" ]; then
      PACKAGE="$BASE_PKG.$SUFFIX_PKG"
    else
      PACKAGE="$SUFFIX_PKG"
    fi
  else
    PACKAGE="$BASE_PKG"
  fi
fi
cat > "$DIR/$NAME" <<JAVA
${PACKAGE:+package $PACKAGE;}

public class $BASE {
    public static void main(String[] args) {
    System.out.println("Hello from new Java class $BASE!");
    }
}
JAVA

echo "Created $DIR/$NAME"
echo "To compile: javac -d bin $DIR/$NAME"
echo "To run: java -cp bin $BASE"
