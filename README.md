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
