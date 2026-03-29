#!/bin/sh

echo "=== Setting up game ==="

apk add build-base git

make

sed -i 's|^tty1::.*|tty1::respawn:/sbin/getty -n -l /bin/sh 38400 tty1|' /etc/inittab

GAME_PATH="$(pwd)/dots_and_boxes"
grep -qxF "$GAME_PATH" /root/.profile || echo "$GAME_PATH" >> /root/.profile

echo "=== Done! Run: reboot ==="
