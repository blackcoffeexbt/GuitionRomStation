#!/bin/sh
# User-authorised, on-demand MacBook camera capture; no background recording.
set -eu
cd "$(dirname "$0")/.."
mkdir -p device-review
exec imagesnap -d 'FaceTime HD Camera' -w 1 "${1:-device-review/latest.jpg}"
