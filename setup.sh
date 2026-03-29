#!/bin/sh

echo "=== Setting up game ==="

apk add build-base git

make

GAME_PATH="$(pwd)/dots_and_boxes"
sed -i "s|^tty1::.*|tty1::respawn:$GAME_PATH|" /etc/inittab

echo "=== Done! Run: reboot ==="
