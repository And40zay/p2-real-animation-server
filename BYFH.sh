#!/usr/bin/env bash
set -e
mkdir -p bin
javac -d bin "$1"
classname=$(basename "$1" .java)
java -cp bin "$classname" "${@:2}"
