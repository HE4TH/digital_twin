#!/bin/sh
# Foreground only: Ctrl+C/terminal disconnect stops the receiver. No boot service.
set -eu
cd -- "$(dirname -- "$0")"
mode="${1:---dry-run}"
case "$mode" in
  --dry-run|--motors) ;;
  *) echo 'Usage: sh run_rc_session.sh [--dry-run|--motors]' >&2; exit 2 ;;
esac
state_topic=codex/ue-bench/20261019/state
if [ "$mode" = --dry-run ]; then state_topic="$state_topic/dry-run"; fi
# Prevent duplicate sessions launched through this entry point.
exec flock -n -E 73 /tmp/underwater-rc-session.lock python3 -u ./rover_drive.py "$mode" \
  --max-speed 0.2 --command-topic codex/ue-bench/20261019/cmd \
  --state-topic "$state_topic"
