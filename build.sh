#!/bin/bash
src="src/server.cpp"
out="build/server.exe"
src2="src/client.cpp"
out2="build/client.exe"

if [ "$src" -nt "$out" ]; then
    echo "Server Source changed, compiling..."
    start=$(date +%s%3N)
    g++ "$src" -o "$out" -lws2_32
    end=$(date +%s%3N)
    echo "Server Build done after $(( end - start ))ms"
else
    echo "No Server changes, skipping build"
fi

if [ "$src2" -nt "$out2" ]; then
    echo "Client Source changed, compiling..."
    start=$(date +%s%3N)
    g++ "$src2" -o "$out2" -lws2_32
    end=$(date +%s%3N)
    echo "Client Build done after $(( end - start ))ms"
else
    echo "No Client changes, skipping build"
fi

build/client.exe