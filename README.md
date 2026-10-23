# rdma-zero-copy-ipc

> A High-Throughput User-Space RDMA Transport & Memory Management Engine utilizing `libibverbs` and Soft-RoCE (`rdma_rxe`) for low-latency scale-out fabrics.

---

##  Overview

`rdma-zero-copy-ipc` is a systems-level C/C++ library designed to demonstrate kernel-bypass inter-process communication (IPC) and direct memory access (DMA) primitives. Built as part of a high-performance scale-out fabric research initiative, this project implements the core building blocks required for modern high-speed interconnect frameworks (such as Cray Slingshot, InfiniBand, and RoCEv2).

By bypassing the Linux network stack and operating directly on hardware-mapped verbs contexts, this engine achieves sub-microsecond latency and eliminates CPU context-switching overhead during data movement.

---

## 📂 Repository Structure

```text
rdma-zero-copy-ipc/
├── CMakeLists.txt              # Modern CMake build configuration
├── README.md                   # Project documentation & architecture guide
├── include/
│   └── rdma_common.h           # Shared structures, constants, and API prototypes
├── src/
│   ├── rdma_context.c          # Day 1: Device discovery, context opening, & Protection Domains (PD)
│   └── rdma_memory.c           # Day 2: POSIX mlock, cache alignment, & ibv_reg_mr (In Progress)
└── tests/
    └── test_day1_init.c        # Standalone verification executable for device/PD initialization
```

# Prerequisites & Environment Setup
```text
This project requires a Linux environment configured with libibverbs and a Soft-RoCE (rdma_rxe) software emulator interface.

1. Install Dependencies (Ubuntu / Debian)
Bash
sudo apt update && sudo apt install -y build-essential cmake git libibverbs-dev rdmacm-utils rdma-core iproute2
2. Configure Soft-RoCE Interface (rxe0)
Bash
sudo modprobe rdma_rxe
Replace enp0s3 with your active network interface name (run 'ip link show' to verify)
sudo rdma link add rxe0 type rxe netdev enp0s3
ibv_devinfo -v
```

## ⚙️ Build & Execution Instructions
```text
This project uses Modern CMake for clean, modular compilation.
1. Create Build Directory
     Bash
     mkdir build
     cd build
2. Run CMake Configuration
     Bash
     cmake ..
3. Compile the Binaries
     Bash
     cmake --build .
4. Run the Day 1 Test Harness
     Bash
     ./test_day1
```
