# PoOS v0.5

PoOS v0.5 extends v0.4 with a scalable virtual filesystem substrate, initrd/tarfs root mount, path-based program execution, and a minimal userland init/shell toolchain.

## Architecture overview

### VFS core (`kernel/vfs/`)
- `vnode`: filesystem object abstraction (regular file, directory, device)
- `file`: open-file state (offset/flags/vnode ref)
- `fdtable`: per-process fixed descriptor table (`FD_MAX=64`)
- `mount`: minimal mount table with root mount plus extra mountpoints (`/dev`)
- `path`: absolute path traversal with component parsing and mount crossing

The design is intentionally small but structured so additional filesystems (FAT/ext2) can register vnode ops and mount roots without changing syscall logic.

### Filesystems (`kernel/fs/`)
- `tarfs`: read-only root filesystem backed by initrd ustar archive
- `devfs`: minimal device nodes:
  - `/dev/console` (write)
  - `/dev/null` (discard writes, EOF on read)
- `initrd`: boot-time initrd region registration from `BootInfo`

## Boot/initrd pipeline
1. Bootloader loads kernel and fixed-size initrd region from disk.
2. Bootloader passes initrd physical start/size in `BootInfo`.
3. Kernel initializes VFS and mounts tarfs at `/`.
4. Kernel mounts devfs at `/dev`.
5. Kernel spawns `/sbin/init` by path.

`user/pack/mkinitrd.sh` builds a ustar archive from `user/pack/rootfs`.

## Process/runtime model
- Kernel launches PID 1 from `/sbin/init`.
- `init` respawns `/bin/sh` and reaps children with `waitpid`.
- Shell supports basic builtins and external command launch by path/
  `/bin/<cmd>` resolution.
- Zombie processes transition to dead state and are cleaned by the reaper path.

## Syscall ABI (`int 0x80`)
Numbers:
1. `write(fd, buf, len)`
2. `exit(code)`
3. `yield()`
4. `sleep(ms)`
5. `getpid()`
6. `open(path, flags)`
7. `close(fd)`
8. `read(fd, buf, len)`
9. `lseek(fd, off, whence)`
10. `stat(path, st)`
11. `getdents(fd, dirent, len)`
12. `execve(path)`
13. `waitpid(pid, status)`
14. `spawn(path)` (minimal helper for v0.5 process launch)

All user pointers are copied via usercopy helpers.

## Rootfs contents
Packed into initrd tar:
- `/sbin/init`
- `/bin/sh`
- `/bin/ls`
- `/bin/cat`
- `/bin/hello`
- `/etc/motd`

## Add a new user program
1. Add `user/apps/<prog>.c`
2. Add `<prog>` to `USER_APPS` in `Makefile`
3. Copy resulting ELF into `user/pack/rootfs` in `$(INITRD_TAR)` rule
4. Rebuild (`make build`) and boot (`make run`)
