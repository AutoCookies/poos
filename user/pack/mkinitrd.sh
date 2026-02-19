#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-user/pack/rootfs}"
OUT="${2:-build/initrd.tar}"
mkdir -p "$(dirname "$OUT")"

required=(
  "$ROOT/sbin/init"
  "$ROOT/bin/sh"
  "$ROOT/etc/passwd"
  "$ROOT/etc/shadow"
  "$ROOT/etc/group"
)
for path in "${required[@]}"; do
  [[ -f "$path" ]] || { echo "mkinitrd: missing required file: $path"; exit 1; }
done

tar --format=ustar --sort=name --owner=0 --group=0 --numeric-owner --mtime='UTC 1970-01-01' \
  -cf "$OUT" -C "$ROOT" .

echo "mkinitrd: wrote $OUT"
