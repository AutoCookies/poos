# PoOS v0.6

PoOS v0.6 adds Unix-like process control + IPC on top of the v0.5 VFS/initrd base: `fork`, `waitpid`, `pipe`, `dup2`, interactive TTY input, basic signal defaults, and writable `/tmp` via memfs.

## Build ISO (quick start)

```sh
make clean
make iso
```

This produces:
- `build/poos.iso`
- `build/kernel.bin`
- `build/initrd.tar`
- `build/iso-root/`

Important:
- `build/poos.iso` is a **raw boot disk image** (renamed `.iso` for artifact convenience), not an ISO9660 filesystem image.
- Boot it with `-drive format=raw,file=...`, **not** `-cdrom`.

Boot in QEMU (recommended):

```sh
make run
# or headless debug output:
make run-headless
```

`run-headless` is expected to keep the terminal attached and print debug output; it does **not** open a QEMU window.

Equivalent manual command:

```sh
qemu-system-i386   -m 80M -smp 1   -drive format=raw,file=build/poos.iso,if=ide,index=0   -drive format=raw,file=build/poos_disk.img,if=ide,index=1   -netdev user,id=n1,hostfwd=udp::5555-:5555   -device rtl8139,netdev=n1
```

VirtualBox note:
- Error `VERR_VMX_IN_VMX_ROOT_MODE` means host nested virtualization/VT-x is unavailable or blocked, so the VM cannot start. This is a host hypervisor setting issue, not a PoOS image format issue.

If QEMU remains at `Booting from Hard Disk...`:
- run a fresh rebuild so boot sector metadata matches current artifact sizes:

```sh
make clean
make iso
```

(bootloader sector counts are generated from current `kernel.bin` and `initrd.tar` at build time.)

Validate artifacts:

```sh
make verify-iso
# or directly:
./tools/verify_iso.sh
```

Run the full build + verify + boot smoke test:

```sh
./tools/test_full_build.sh
```

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

## PoOS v1.1 SMP foundations

PoOS v1.1 introduces a dedicated SMP subsystem with explicit per-CPU data paths and split modules for APIC/IPI/scheduler locks.

### Boot flow (BSP + AP startup)

- BSP performs normal early init (paging/heap/interrupts), then calls `smp_init()`.
- `smp_init()` initializes CPU descriptors, LAPIC/IOAPIC entry points, and CPU discovery (ACPI MADT first, MP table fallback).
- `smp_boot_aps()` performs INIT/SIPI sequencing via APIC helpers and marks secondary CPUs online.
- AP trampoline entry is provided at `kernel/arch/x86/smp/start_ap.asm` and reserved for low-memory SIPI vector handoff.

### Per-CPU layout

Per-CPU metadata lives in `kernel/arch/x86/smp/cpu.c` and tracks:
- cpu/apic id
- online state
- tick counters
- current task pointer
- interrupt nesting depth
- preemption disable depth

The per-CPU API is exposed by:
- `cpu_id()`
- `this_cpu_ptr(var)`
- `per_cpu(var, cpu)`

### Scheduler model

- SMP scheduler code is split under `kernel/sched/smp/`.
- Local enqueue/dequeue fast path is isolated in `sched_smp_*` APIs.
- Preemption counters are per CPU (`preempt_disable()/preempt_enable()`).
- Load-balance hook is kept in `load_balance.c` for deterministic steal policy growth.

### TLB shootdown + IPIs

- IPI surface includes reschedule and TLB shootdown messages (`ipi_send_resched`, `ipi_send_tlb_shootdown`).
- Shootdown handler hooks are split into `kernel/arch/x86/smp/ipi.c` and can be wired to address-space CPU masks in VM code.

### Locking primitives and rules

PoOS v1.1 adds central lock primitives under `kernel/locks/`:
- spinlock (`spin_lock`, `spin_lock_irqsave`)
- mutex (sleep/yield based)
- rwlock (reader/writer serialization)

Debug/telemetry stubs for contention are exposed in `lock_debug.c`.

### Time and IRQ SMP split

- `kernel/time/time_smp.c` tracks per-CPU ticks.
- `kernel/time/clocksource.c` provides a monotonic ns view from ticks.
- Generic IRQ affinity plumbing lives in `kernel/irq/irq*.c`.

### Running with multiple CPUs

Run QEMU with SMP enabled, for example:

```sh
qemu-system-i386 -smp 4 \
  -drive format=raw,file=build/poos.img,if=ide,index=0 \
  -drive format=raw,file=build/poos_disk.img,if=ide,index=1
```

## PoOS v1.2 security model

PoOS v1.2 adds a credential-based security layer:

- Per-process credentials (`uid/euid/suid`, `gid/egid/sgid`, supplementary groups, umask).
- Capability bitsets (`permitted/effective/inheritable`) with checks for net raw/admin, chown, DAC override, sysadmin, kill.
- VFS permission enforcement centralized in `kernel/vfs/vfs_perm.c`.
- File ownership + mode bits on vnodes (`uid/gid/mode`, including setuid/setgid bits).
- Secure exec hooks for setuid/setgid transitions and capability trimming.
- Process-root isolation (`chroot`) via per-process root vnode + chroot-aware path resolve.
- Audit logging via non-blocking console audit events (`[AUDIT] ...`).

### Credential semantics

- Real/effective/saved IDs are tracked in `struct cred`.
- `fork()` clones credentials from parent.
- `exec()` may transition effective IDs when setuid/setgid bits are present on executable and binary is not group/world writable.

### Users/groups/password DB

- `/etc/passwd` and `/etc/group` are parsed by kernel auth layer.
- `/etc/shadow` is present for password storage format (minimal v1 placeholder hash format).
- `login` user app uses `sys_auth()` and then switches uid before starting `/bin/sh`.

### Admin/user tools

- `login`, `su`, `id`, `chmod`, `chown`, `umask`, `passwd` included in `/bin`.

### Isolation and audit

- `chroot` syscall requires CAP_SYS_ADMIN.
- `..` traversal does not escape process root.
- Security-relevant events are audited: login/chmod/chown/capset/chroot/setuid-exec/permission denies.

### Adding users (v1)

Edit `/etc/passwd`, `/etc/group`, and `/etc/shadow` inside initrd rootfs and rebuild image with `make build`.

## PoOS v1.3 TLS groundwork (in-progress)

- Added kernel crypto module layout under `kernel/crypto/` with hardened primitives currently used by RNG (`memwipe`, constant-time compare, SHA-256, HMAC, HKDF).
- Added kernel CSPRNG plumbing and `getrandom(2)` syscall (`SYS_GETRANDOM`).
- Added wall-clock epoch syscalls (`time(2)` and `settime(2)`) for certificate-validity checks.
- Added TLS/X.509 module scaffolding under `kernel/net/tls/` and `kernel/crypto/x509/` for phased integration.
- Added userland commands:
  - `/bin/httpsget`
  - `/bin/tlsprobe`

### Trust store and TLS notes

- Planned trust store path: `/etc/ssl/certs/ca-bundle.der` (or PEM bundle in same directory).
- TLS hostname verification and chain validation are planned to run in strict mode by default.
- Time must be set correctly before TLS validation; use `settime` syscall from privileged tooling.

### Current limitations

- TLS 1.2 handshake and HTTPS data path are not fully enabled in this snapshot.
- TLS 1.3, OCSP, and CRL checks are not yet implemented.

## PoOS v1.4 containers-lite sandboxing

PoOS v1.4 introduces a minimal container substrate with namespace, cgroup, and seccomp-like isolation plumbing.

### Namespaces

- New namespace subsystem under `kernel/ns/` with `nsproxy` wiring for per-process mount/pid/net/uts/user namespace references.
- `unshare`/`clone` control flags support creating new namespace views (`CLONE_NEWNS`, `CLONE_NEWPID`, `CLONE_NEWNET`, `CLONE_NEWUTS`, `CLONE_NEWUSER`).
- Mount namespace root is used by path resolution through `proc_current_root()` for per-sandbox rootfs view.
- PID namespace has independent virtual PID allocation (`pid_ns` allocator); `getpid()` returns namespace pid.
- UTS namespace carries per-namespace hostname.
- User namespace includes a minimal uid mapping structure suitable for root-inside to non-root-outside mappings.

### Cgroups

- Added minimal cgroup core under `kernel/cgroup/`.
- Per-cgroup limits: pids, memory budget field, and cpu percentage field.
- `fork/clone` path enforces pids controller limit (`-EAGAIN`-like failure in limit reached case).
- Counters and lifecycle hooks include pids current count, throttle/oom counters, and audit event on cgroup creation.

### Seccomp-like filtering

- Added per-process seccomp filter object under `kernel/seccomp/`.
- Supports modes:
  - `SECCOMP_MODE_DISABLED`
  - `SECCOMP_MODE_STRICT` (allowlist baseline)
- Syscall dispatcher checks filter before executing handlers; denied calls are audited and blocked.

### poosrun sandbox runtime

- Added `/bin/poosrun` user app.
- Supports:
  - `--root <path>`
  - `--net=none|host`
  - `--mem <MiB>`
  - `--pids <n>`
  - `--cpu <percent>`
  - `--seccomp=off`
- Runtime flow:
  1. `unshare` namespace flags
  2. `chroot` to sandbox root
  3. set cgroup limits
  4. apply seccomp mode
  5. `execve` target command

### Limitations

- cgroup memory/cpu quota fields are plumbed and observable, with pids controller being the hard-enforced controller in this snapshot.
- `setns()` and argument-level seccomp filters are reserved for follow-up.

## PoOS v1.5 container networking

PoOS v1.5 extends the v1.4 namespace substrate with an integrated container networking control plane.

- `CLONE_NEWNET` namespaces now bootstrap a virtual topology model: `veth` host/peer names, per-namespace software bridge identity, container-side leased IPv4, and default route metadata (`kernel/ns/netns.*`).
- Per-netns state now tracks isolated route/ARP tables and L2/L3 counters, plus NAT and port-forward rule tables.
- `sys_netctl` now exposes namespace diagnostics and control commands:
  - `10`: fetch per-netns bridge/veth/NAT diagnostics and counters
  - `11`: add host->container TCP/UDP port-forward mapping (CAP_NET_ADMIN)
  - `12`: enable/disable per-netns NAT masquerade mode (CAP_NET_ADMIN)
- Added userland `/bin/netnsctl` for namespace networking operations (`show`, `nat on/off`, `pfwd <host-port> <container-port>`).

### v1.5 notes

- Packet data path remains single-NIC in this snapshot; veth/bridge/NAT are represented in per-namespace control-plane state and hardened syscall plumbing.
- NAT/port-forward counters are maintained for observability and future packet-path wiring.

## PoOS Edge80 profile (Ultra-Lean server mode)

PoOS now ships build profiles to support memory-bounded edge deployments.

### Build profiles

- `CONFIG_EDGE_80MB`: enabled by default via `PROFILE=edge80`; tuned for bounded memory use.
- `CONFIG_LEAN_SERVER`: enabled for `PROFILE=edge80` and `PROFILE=lean`.
- `CONFIG_FULL`: enabled for `PROFILE=full`.

Build commands:

- `make edge80`
- `make PROFILE=lean build`
- `make PROFILE=full build`

Recommended QEMU invocation for edge mode:

- `qemu-system-i386 -m 80M -smp 1 -drive format=raw,file=build/poos.img,if=ide,index=0 -drive format=raw,file=build/poos_disk.img,if=ide,index=1 -netdev user,id=n1,hostfwd=tcp::8443-:8443 -device rtl8139,netdev=n1`

### Memory budget table (single source of truth)

Budget definitions are centralized in `kernel/mm/budget.h` + `kernel/mm/budget.c`.

Default `CONFIG_EDGE_80MB` caps:

- Kernel core + stacks: 10 MiB
- Slab allocator total: 10 MiB
- Page cache: 8 MiB
- Buffer cache: 8 MiB
- Networking buffers: 6 MiB
- TCP memory: 8 MiB
- TLS memory: 6 MiB
- User RSS aggregate: 20 MiB

Total target: 76 MiB + 4 MiB safety margin.

### Enforced bounded behavior in this snapshot

- pbuf allocations are served from fixed pools (`256B` and `1536B` classes) with drop-on-exhaust behavior.
- Buffer cache memory is hard-accounted into the global budget at init.
- Allocation refusals and packet drops are tracked via fixed counters.
- `memstat` user tool fetches kernel budget snapshots via `SYS_MEMSTAT`.

### Observability

Run:

- `/bin/memstat` for budget caps/used/peak/refused and pressure counters.
- `/bin/iostat` for storage counters.
- `/bin/ifconfig` for network state.

### Current edge-mode limitations

- TLS transport stack remains scaffolded in this tree and is not production-enabled yet.
- Throughput is intentionally constrained by fixed buffer pools and conservative memory caps.

## PoOS Edge80-Prox (`proxyd`)

`proxyd` is a memory-bounded reverse-proxy fetcher/cache agent designed for Edge80 limits. In this build it supports strict bounded caching and upstream HTTP fetch, but PoOS still lacks a user-visible TCP listen/accept syscall, so daemon socket-accept mode is intentionally degraded.

### Command

```sh
proxyd --listen 443 --upstream 10.42.0.2:8080 --cache-mem 8M --cache-disk 32M --path /index.html
```

### Memory bounds

- Memory cache hard cap: configurable (default `8M`, upper bound compiled at 8 MiB arena).
- Disk cache hard cap: configurable (default `32M`).
- Memory index cap: 2048 entries.
- Disk index cap: 4096 entries.
- Object caps:
  - memory tier: `<= 64KiB`
  - disk tier: `<= 1MiB`

### Cache policy

- Cacheable responses: `HTTP/1.1 200` + `Cache-Control: max-age=N`.
- Other responses are streamed/passthrough only (no cache insert).
- TTL expiry causes eviction-on-access.

### Operational notes

- Use `--stats` to print bounded cache counters.
- Current transport path is upstream client mode (`GET`) with fixed memory buffers and no dynamic allocation.
- `--listen` is accepted for forward-compatibility but currently informational due to kernel socket API limits (no listen/accept syscall yet).

## PoOS Phase 17 Edge80-Prox readiness

- Boot now prefers `/bin/edge` from `init` and falls back to `login` if unavailable.
- `edge` runs a tiny crash-only supervisor for `proxyd` with restart backoff (50ms→1s), restart storm detection, and status persistence in `/tmp/edge.status`.
- Supervisor controls:
  - `edge svc status`
  - `edge svc list`
  - `edge svc restart proxyd`
- Proxy config file: `/etc/edge/proxyd.conf` (line-based `key = value`).
- Reload workflow:
  - `proxyd --reload` writes a bounded reload request.
  - Daemon applies parse+validate+swap atomically and retains old config on failure.
- Hardening knobs include bounded request/header limits, per-IP token-bucket rate limiting, timeout accounting, and shed-load behavior on low memory.
- Observability commands:
  - `proxystat` (reads bounded counter snapshot)
  - `memstat`
  - `healthcheck` (`OK`, `DEGRADED`, `FAIL`)
- Benchmark + report:
  - `edge run-golden`
  - Runs health + bench workflow and writes `/var/log/edge-report.json` (fallback `/tmp/edge-report.json`).
- Default 80MB tuning:
  - `max_conns=64`, `max_tls_conns=32`
  - `cache_mem=8M`, `cache_disk=32M`
  - `timeout_header_ms=3000`, `timeout_idle_ms=15000`, `timeout_upstream_ms=1500`
