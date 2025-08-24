# Development Setup Guide

## Required Tools
- gcc/x86_64-elf-gcc (cross compiler)
- nasm (assembler)
- qemu-system-x86_64 (emulator)
- make (build system)

## Build Environment Setup
1. Install dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential nasm qemu-system-x86

# For cross-compiler (if not already installed)
# Follow cross-compiler build instructions in tools/build-toolchain.sh
```

2. Build the project:
```bash
make          # Build everything
make kernel   # Build kernel only
make user     # Build user services
make qemu     # Run in QEMU
```

## Development Workflow
1. Create a new branch for your feature
2. Make changes
3. Run tests: `make test`
4. Submit PR

## Directory Structure
Refer to README.md for detailed directory structure
