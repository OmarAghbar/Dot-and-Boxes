#!/bin/sh

echo "=== Setting up game ==="

make

sed -i 's|tty1::respawn:.*|tty1::respawn:/sbin/getty --autologin root 38400 tty1|' /etc/inittab

GAME_PATH="$(pwd)/dots_and_boxes"

grep -qxF "$GAME_PATH" /root/.profile || echo "$GAME_PATH" >> /root/.profile

echo "=== Done! Reboot to launch game automatically ==="