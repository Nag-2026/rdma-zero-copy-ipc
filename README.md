# rdma-zero-copy-ipc

> A High-Throughput User-Space RDMA Transport & Memory Management Engine utilizing `libibverbs` and Soft-RoCE (`rdma_rxe`) for low-latency scale-out fabrics.

---

##  Overview

`rdma-zero-copy-ipc` is a systems-level C/C++ library designed to demonstrate kernel-bypass inter-process communication (IPC) and direct memory access (DMA) primitives. Built as part of a high-performance scale-out fabric research initiative, this project implements the core building blocks required for modern high-speed interconnect frameworks (such as Cray Slingshot, InfiniBand, and RoCEv2).

By bypassing the Linux network stack and operating directly on hardware-mapped verbs contexts, this engine achieves sub-microsecond latency and eliminates CPU context-switching overhead during data movement.

---

## 📂 Repository Structure

```text
## 📂 Repository Structure
## 📂 Repository Structure

rdma-zero-copy-ipc/
├── CMakeLists.txt              # Modern CMake build configuration
├── README.md                   # Project documentation & architecture guide
├── include/
│   └── rdma_common.h           # Shared structures, constants, and API prototypes
├── src/
│   ├── rdma_context.c          # Day 1: Device discovery, context opening, & Protection Domains (PD)
│   ├── rdma_memory.c           # Day 2: POSIX mlock, cache alignment, & ibv_reg_mr registration
│   └── rdma_transport.c        # Day 3: Completion Queue (CQ), Queue Pair (QP), & state transitions
└── tests/
    ├── test_day1_init.c        # Standalone verification executable for device/PD initialization
    ├── test_day2_memory.c      # Standalone test harness for zero-copy memory pinning & registration
    └── test_day3_qp.c          # Standalone test harness for QP creation & state machine lifecycles
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
# Expected Output (Day 1)
```text
====================================================
 Day 1: Modular Test Harness (Context & PD Setup)   
====================================================

[+] Found 1 RDMA device(s):
    - Device 0: rxe0
[+] Successfully opened verbs context for device.
[+] Device Attributes Queried Successfully:
    - Max Queue Pairs (QPs)   : 262144
    - Max Memory Regions (MRs): 262144
    - Max CQ Entries          : 65535
    - Physical Port Count     : 1
[+] Port 1 Attributes:
    - Port State : 4 (Active = 4)
    - Link Layer : Ethernet (RoCEv2)
[+] Successfully allocated Protection Domain (PD address: 0x55dc1c2a12a0)

[+] Cleaning up resources...
    - Protection Domain deallocated.
    - Device context closed.

[SUCCESS] Day 1 modular execution complete!
```

## 🛠️ Technical Implementation Milestones

- **Day 1: Context Lifecycle & Protection Domains**
  - Implemented dynamic device discovery (`ibv_get_device_list`) and hardware context binding (`ibv_open_device`).
  - Queried physical HCA/emulator capabilities (`ibv_query_device`, `ibv_query_port`) to inspect maximum queue pairs, MTU sizes, and link layers (`IBV_LINK_LAYER_ETHERNET`).
  - Allocated isolated **Protection Domains (`ibv_pd`)** to enforce memory security boundaries.

- **Day 2: Zero-Copy Memory Pinning & Alignment**
  - Integrated `posix_memalign` for 64-byte cache-line buffer alignment to prevent cache-line bouncing.
  - Applied POSIX page locking (`mlock`) to pin virtual RAM pages and prevent OS paging/swapping overhead during DMA.
  - Registered memory regions with the HCA driver via `ibv_reg_mr` using `IBV_ACCESS_LOCAL_WRITE`, `IBV_ACCESS_REMOTE_WRITE`, and `IBV_ACCESS_REMOTE_READ` flags, extracting local (`lkey`) and remote (`rkey`) security tokens.

- **Day 3: Queue Management & State Machine Transitions**
  - Implemented Completion Queues (`ibv_cq`) for asynchronous work request notifications.
  - Configured Reliable Connection (`IBV_QPT_RC`) Queue Pairs (`ibv_qp`) associated with the Protection Domain and Completion Queue.
  - Programmatically drove the InfiniBand transport state machine through its complete lifecycle: **RESET ➔ INIT ➔ RTR (Ready-to-Receive) ➔ RTS (Ready-to-Send)**.
