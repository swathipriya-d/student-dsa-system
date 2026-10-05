#!/usr/bin/env bash
# Exit on error
set -o errexit

echo ">> Compiling C DSA Engine on Linux..."
gcc -Wall -Wextra -O2 \
    c_src/linked_list.c \
    c_src/stack.c \
    c_src/queue.c \
    c_src/hash_table.c \
    c_src/bst.c \
    c_src/graph.c \
    c_src/sorting_searching.c \
    c_src/file_io.c \
    c_src/main.c \
    -o dsa_core

chmod +x dsa_core

echo ">> Installing Python dependencies..."
pip install -r requirements.txt

echo ">> Build complete! Ready to start web service."
