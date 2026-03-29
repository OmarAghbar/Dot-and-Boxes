#!/bin/sh

echo "=== Setting up Dots and Boxes ==="

make

sed -i 's|^tty1::.*|tty1::respawn:/sbin/getty --autologin root 38400 tty1|' /etc/inittab

GAME_PATH="$(pwd)/dots_and_boxes"

grep -qxF "$GAME_PATH" /root/.profile || echo "$GAME_PATH" >> /root/.profile

echo "=== Done! Run: reboot ==="
