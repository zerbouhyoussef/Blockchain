#!/usr/bin/env bash
set -e
make clean && make
echo "Running full AC_HASH + Blockchain tests..."
./workshop
