#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
usage:
  tools/verify_iso.sh [ISO_PATH ISO_ROOT INITRD_PATH KERNEL_PATH]

If no arguments are provided, defaults are used:
  build/poos.iso build/iso-root build/initrd.tar build/kernel.bin
USAGE
}

if [[ $# -eq 0 ]]; then
  ISO_PATH="build/poos.iso"
  ISO_ROOT="build/iso-root"
  INITRD_PATH="build/initrd.tar"
  KERNEL_PATH="build/kernel.bin"
elif [[ $# -eq 4 ]]; then
  ISO_PATH="$1"
  ISO_ROOT="$2"
  INITRD_PATH="$3"
  KERNEL_PATH="$4"
else
  usage
  exit 2
fi

for path in "$ISO_PATH" "$ISO_ROOT" "$INITRD_PATH" "$KERNEL_PATH"; do
  [[ -e "$path" ]] || { echo "error: missing required artifact: $path"; exit 1; }
done

[[ -s "$KERNEL_PATH" ]] || { echo "error: kernel is empty: $KERNEL_PATH"; exit 1; }
[[ -s "$INITRD_PATH" ]] || { echo "error: initrd is empty: $INITRD_PATH"; exit 1; }
[[ -s "$ISO_PATH" ]] || { echo "error: iso artifact is empty: $ISO_PATH"; exit 1; }

[[ -f "$ISO_ROOT/boot/kernel.bin" ]] || { echo "error: ISO root missing boot/kernel.bin"; exit 1; }
[[ -f "$ISO_ROOT/boot/initrd.tar" ]] || { echo "error: ISO root missing boot/initrd.tar"; exit 1; }
[[ -f "$ISO_ROOT/boot/grub/grub.cfg" ]] || { echo "error: ISO root missing grub.cfg"; exit 1; }

tar -tf "$INITRD_PATH" | rg -q '^\./sbin/init$' || { echo "error: initrd missing /sbin/init"; exit 1; }
tar -tf "$INITRD_PATH" | rg -q '^\./bin/sh$' || { echo "error: initrd missing /bin/sh"; exit 1; }
tar -tf "$INITRD_PATH" | rg -q '^\./etc/passwd$' || { echo "error: initrd missing /etc/passwd"; exit 1; }
tar -tf "$INITRD_PATH" | rg -q '^\./etc/shadow$' || { echo "error: initrd missing /etc/shadow"; exit 1; }
tar -tf "$INITRD_PATH" | rg -q '^\./etc/group$' || { echo "error: initrd missing /etc/group"; exit 1; }

echo "[verify-iso] verified artifacts and initrd runtime files"
