# Copilot Instructions for MicroKernel Development

## Overview
This repository implements a microkernel-based operating system. The kernel is designed to provide minimal core functionality, delegating most services to user space. The architecture emphasizes modularity, process isolation, and efficient inter-process communication (IPC).

## Key Components
- **Scheduler**: Implements Completely Fair Scheduler (CFS) in `src/kernel/sched_fair.c`.
- **Memory Management**: Includes a buddy system for page allocation in `src/mm/buddy.c`.
- **System Calls**: Defined in `src/kernel/main.c` with a syscall table and basic implementations.
- **Interrupt Handling**: Handled in `src/kernel/main.c` with support for hardware interrupts and exceptions.
- **Build System**: Managed via `Makefile` with targets for building, testing, and debugging.

## Developer Workflows
### Building the Kernel
- Ensure dependencies are installed: GCC, NASM, Binutils, QEMU.
- Build the kernel:
  ```bash
  make all
  ```
- Clean build artifacts:
  ```bash
  make clean
  ```

### Running and Debugging
- Run the kernel in QEMU:
  ```bash
  make qemu
  ```
- Debug with QEMU:
  ```bash
  make debug
  ```
- Generate ISO for bootable media:
  ```bash
  make iso
  ```

### Testing
- Run kernel tests:
  ```bash
  make test
  ```
- Perform static analysis:
  ```bash
  make analyze
  ```

## Coding Conventions
- Follow GNU C99 standard.
- Use Linux kernel coding style.
- Document all functions with comments.
- Use meaningful variable names.

## Important Files and Directories
- `arch/`: Architecture-specific code (e.g., `arch/x86_64/boot.S`).
- `include/`: Header files for kernel subsystems.
  - `sched.h`: Scheduler definitions.
  - `mm.h`: Memory management definitions.
  - `list.h`: Linked list utilities.
- `src/kernel/`: Core kernel source code.
  - `main.c`: Kernel entry point and system call table.
  - `sched.c`: Scheduler implementation.
  - `sched_fair.c`: CFS implementation.
- `src/mm/`: Memory management code.
  - `buddy.c`: Buddy allocator implementation.
- `Makefile`: Build system configuration.

## Patterns and Practices
- **System Calls**: Add new syscalls to the table in `main.c` and implement them as `sys_<name>` functions.
- **Interrupts**: Handle hardware interrupts in `handle_interrupt` and exceptions in `handle_exception`.
- **Memory Allocation**: Use `kmalloc` and `kfree` for dynamic memory management.
- **Scheduler**: Modify `sched_fair.c` for changes to the CFS.

## External Dependencies
- QEMU: For emulating the kernel.
- GRUB: Optional for creating bootable ISO images.

## Notes
- The kernel is currently single-threaded and runs on x86_64 architecture.
- Future work includes adding support for multi-threading and additional system calls.
