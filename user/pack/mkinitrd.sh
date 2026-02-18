#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-user/pack/rootfs}"
OUT="${2:-build/initrd.tar}"
mkdir -p "$(dirname "$OUT")"
tar --format=ustar -cf "$OUT" -C "$ROOT" .
