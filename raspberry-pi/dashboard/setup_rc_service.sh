#!/bin/sh
# Run as pi after deploying this dashboard directory. Does not start the receiver.
set -eu
[ "$(id -un)" = pi ] || { echo "Run as pi" >&2; exit 1; }
[ -f /home/pi/rover_drive.py ] || { echo "Receiver missing" >&2; exit 1; }
export XDG_RUNTIME_DIR="/run/user/$(id -u)"
mkdir -p "$HOME/.config/systemd/user"
install -m 644 "$(dirname "$0")/dmix-rc-session.service" "$HOME/.config/systemd/user/dmix-rc-session.service"
sudo -n loginctl enable-linger pi
systemctl --user daemon-reload
# Keep the existing dashboard command and flags; add the service opt-in via env.
sudo -n mkdir -p /etc/systemd/system/dmix-dashboard.service.d
printf '[Service]\nEnvironment=DMIX_RC_SERVICE=1\n' | sudo -n tee /etc/systemd/system/dmix-dashboard.service.d/rc-control.conf >/dev/null
sudo -n systemctl daemon-reload
sudo -n systemctl restart dmix-dashboard
printf 'Dashboard updated. Receiver was not started or restarted.\n'
