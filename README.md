# PoOS v0.6

PoOS v0.6 adds Unix-like process control + IPC on top of the v0.5 VFS/initrd base: `fork`, `waitpid`, `pipe`, `dup2`, interactive TTY input, basic signal defaults, and writable `/tmp` via memfs.

## Process model

- `fork()` clones the current process and its user address space (full page copy, no COW yet).
- Parent receives child PID, child resumes at the same user PC with return value `0`.
- Child inherits open file descriptors by refcounted `file` objects.
- `execve()` replaces current user image.
- States: `RUNNING`, `ZOMBIE`, `DEAD`.
- Parent/child links (`parent`, `ppid`) are tracked in `struct proc`.

## waitpid semantics

- `waitpid(pid, status, flags)` supports:
  - `pid > 0`: specific child
  - `pid <= 0`: any child
- If matching zombie exists, returns immediately and reaps.
- If caller has no matching children, returns `-ECHILD` equivalent (`-10`).
- Blocking waits sleep/yield in scheduler loops (no IRQ busy-spin).
- `WNOHANG`-style behavior is wired through `flags & 1`.

## IPC: pipes + descriptor control

- `pipe(fds)` creates read/write file descriptors backed by a ring buffer.
- Semantics:
  - read blocks while empty and writers remain
  - read returns `0` on EOF when last writer closes
  - write returns `-EPIPE` when no readers exist
- `dup2(oldfd, newfd)` implemented with correct close/rebind behavior.
- FD lifecycle uses file refcounts and close callbacks for pipe endpoint release.

## Signals (minimal defaults)

Implemented default-only model (no user-installed handlers yet):
- `SIGCHLD`: parent pending bit set when child exits.
- `SIGKILL`: pending kill terminates process.
- `SIGSEGV`: user page fault terminates faulting process.
- `kill(pid, sig)` supports basic delivery by PID.

Limitations:
- No custom signal handlers.
- No process groups/job control signals yet.

## TTY + console input

- Added `/dev/tty` line-buffered input path.
- Keyboard IRQ1 (`kbd.c`) translates basic set-1 scancodes to ASCII.
- `read(0, ...)` from tty returns on newline.
- Minimal line editing: backspace supported.
- Console output remains VGA text mode.

## Writable `/tmp` (memfs)

- Mounted memfs at `/tmp`.
- Flat namespace under `/tmp` (no nested directories).
- Supports open/create/truncate/read/write for small regular files.
- Max file size: 64 KiB per file.

## Shell v0.6

`/bin/sh` now runs interactively from TTY and uses fork/exec/wait.
Supported forms:
- `echo hi`
- `echo hi > /tmp/out`
- `cat /tmp/out`
- `cmd1 | cmd2`
- `hello &`
- external `/bin/<name>` execution

## Syscall ABI additions

- `fork()`
- `pipe(int fds[2])`
- `dup2(oldfd, newfd)`
- `kill(pid, sig)`

Legacy v0.5 syscalls remain available (`open/read/write/execve/waitpid/spawn/...`).

## Known limitations / future work

- No copy-on-write fork yet (full-copy clone only).
- No full POSIX wait status encoding.
- No termios, canonical/raw mode control.
- Pipe blocking currently scheduler-sleep based (simple, not full wait queues).
- No advanced shell parser (single pipeline segment, simple redirections).

## PoOS v0.7 Virtual Memory

PoOS v0.7 introduces an address-space centric VM layer:

- `struct addrspace` owns per-process `cr3` + sorted VMA list.
- `mmap/munmap` create and remove VMAs without eager allocation.
- Anonymous and file-backed mappings are demand-paged in page fault path.
- `fork()` uses Copy-on-Write by marking user PTEs readonly + `PTE_COW` (x86 available bit 9).
- COW write faults allocate private pages only when needed.
- File mappings use a minimal global page cache keyed by `(vnode*, file_page_index)`.

### Demand paging flow

1. CPU raises page fault.
2. Fault handler checks process VMA for fault address.
3. If unmapped or permission-violating: SIGSEGV.
4. If not-present + anon VMA: allocate zeroed page.
5. If not-present + file VMA: pull page from page cache (load from vnode on miss).
6. If present+write+COW: perform COW break (copy if shared, relabel writable if exclusive).
7. Install PTE, `invlpg`, resume execution.

### VM diagnostics

Kernel vmstat counters include:
- faults total/handled/sigsegv
- COW faults/copies
- anon pages allocated
- file pages loaded
- pagecache hit/miss/entries

### Userland tests

Run from shell:

- `/bin/cowtest` (fork + COW isolation)
- `/bin/mmaptest` (anon mmap + lazy touch)
- `/bin/filemaptest` (file-backed mmap + cache reuse)

Current limitations:
- MAP_SHARED is minimally recognized; writeback is not implemented.
- Eviction policy is simple and skips referenced cache entries.
- ELF demand-loading is left for follow-up (Phase 7.5).

## PoOS v0.8 Disk-backed storage (experimental)

- Added block device core (`kernel/blk`) and ATA PIO IDE primary driver (`kernel/drivers/ata_pio.c`).
- Added MBR partition scanning that registers `hd0p1` slices.
- Added 4KiB buffer cache with LRU-ish eviction and writeback (`kernel/bcache`).
- Added FAT16 filesystem implementation (`kernel/fs/fat`) mounted at `/home` when `hd0p1` exists.
- VFS gained writable operations (`create`, `mkdir`, `unlink`, `truncate`) and `sync` syscall path.
- New user tools: `mkdir`, `rm`, `mv`, `cp`, `sync`.
- Build now generates a second IDE disk image (`build/poos_disk.img`) with MBR + FAT16 using `tools/mkfatdisk.py`.

### Run

```sh
make run
```

QEMU launches with:
- `build/poos.img` as boot disk
- `build/poos_disk.img` as secondary IDE disk for `/home`

### Current FAT limitations

- 8.3 filenames only.
- Directory semantics are minimal.
- Rename is currently stubbed.
- No journal; use `sync` before shutdown.

## PoOS v0.9 Networking

PoOS v0.9 adds a modular IPv4 stack with RTL8139 PCI NIC support for QEMU.

### QEMU run arguments
`make run` now starts with:
- `-netdev user,id=n1,hostfwd=udp::5555-:5555`
- `-device rtl8139,netdev=n1`

### Stack modules
- PCI scan/enabling: `kernel/pci/*`
- NIC driver: `kernel/drivers/rtl8139.c`
- Core + net thread + queue: `kernel/net/net.c`
- Packet buffer: `kernel/net/pbuf.c`
- L2/L3/L4: Ethernet, ARP, IPv4, ICMP, UDP in `kernel/net/*`
- DHCP lease initialization, DNS lookup, route, and socket layer in `kernel/net/*`

### Protocol coverage
- Ethernet II
- ARP request/reply + table
- IPv4 (no fragmentation reassembly)
- ICMP echo (ping)
- UDP datagrams
- DHCP lease provisioning (QEMU user-net defaults)
- DNS A query over UDP

### Syscalls
Supported minimal socket APIs:
- `socket(AF_INET, SOCK_DGRAM, 0)`
- `bind(fd, sockaddr_in)`
- `sendto(...)`
- `recvfrom(...)`
- `close(fd)` via socket close syscall

### User tools
- `/bin/ifconfig`
- `/bin/ping`
- `/bin/udpsend`
- `/bin/udprecv`
- `/bin/dnslookup`

### Host UDP test
1. Run `udprecv` inside PoOS.
2. On host: `echo -n hi | nc -u 127.0.0.1 5555`
3. Use `udpsend` in PoOS to send to gateway on port 5555.

## PoOS v1.0 TCP stack

PoOS now includes a modular TCP stack under `kernel/net/tcp/` split by state machine, input, output, timers, retransmit, window control, connection table, and socket integration.

### TCP state machine

Implemented states: `CLOSED`, `LISTEN`, `SYN_SENT`, `SYN_RECEIVED`, `ESTABLISHED`, `FIN_WAIT_1`, `FIN_WAIT_2`, `CLOSE_WAIT`, `LAST_ACK`, `TIME_WAIT`.

### Connection lifecycle

- Active opens allocate a `tcp_conn` keyed by 4-tuple.
- 3-way handshake is validated (`SYN`, `SYN+ACK`, `ACK`) before entering `ESTABLISHED`.
- Ordered receive buffering tracks `rcv_nxt` and only delivers contiguous payload.
- FIN close transitions through active/passive close states with `TIME_WAIT` timer handling.

### Socket semantics

- Added `SOCK_STREAM` with `socket/connect/send/recv/close` support.
- `connect()` waits for handshake completion.
- `recv()` sleeps until bytes available or EOF.
- `send()` respects remote advertised window and segments by MSS.

### Current limitations

- No congestion control yet (window-based flow control only).
- No TLS.
- Server-side `LISTEN` path is reserved for future release.

### Performance notes

- Retransmit queue supports exponential RTO backoff.
- Net timers run from network-thread context (no IRQ blocking).
- TCP and UDP/ICMP share IPv4 demux safely.
