#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

echo "[test] clean build"
make clean

echo "[test] build iso"
make iso

echo "[test] verify iso"
tools/verify_iso.sh build/poos.iso build/iso-root build/initrd.tar build/kernel.bin

echo "[test] boot smoke check (headless)"
LOG="build/qemu_boot.log"
rm -f "$LOG"
set +e
qemu-system-i386 \
  -drive format=raw,file=build/poos.iso,if=ide,index=0 \
  -drive format=raw,file=build/poos_disk.img,if=ide,index=1 \
  -m 80M \
  -no-reboot \
  -no-shutdown \
  -display none \
  -debugcon file:"$LOG" \
  -global isa-debugcon.iobase=0xe9 \
  -serial none \
  -net none \
  -monitor none \
  -d guest_errors \
  -D build/qemu_guest_errors.log &
QEMU_PID=$!
for _ in $(seq 1 25); do
  if rg -q "\[init\] pid1 online" "$LOG" 2>/dev/null; then
    kill "$QEMU_PID" >/dev/null 2>&1 || true
    wait "$QEMU_PID" >/dev/null 2>&1 || true
    echo "[test] init started"
    exit 0
  fi
  sleep 1
done
kill "$QEMU_PID" >/dev/null 2>&1 || true
wait "$QEMU_PID" >/dev/null 2>&1 || true
set -e

echo "error: init startup log not detected in $LOG"
exit 1
