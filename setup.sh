#!/bin/sh

echo "=== Setting up game ==="

make

GAME_PATH="$(pwd)/dots_and_boxes"

sudo mkdir -p /etc/systemd/system/getty@tty1.service.d/
sudo bash -c 'cat > /etc/systemd/system/getty@tty1.service.d/override.conf << EOF
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin root --noclear %I \$TERM
EOF'

grep -qxF "$GAME_PATH" /root/.profile || echo "$GAME_PATH" | sudo tee -a /root/.profile

echo "=== Done! Run: sudo reboot ==="
