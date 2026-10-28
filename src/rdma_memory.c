#include "rdma_common.h"

/* Step 1 & 2: Allocate cache-aligned memory and pin pages using mlock
 * Step 3: Register Memory Region (MR) with the Protection Domain
 */
struct ibv_mr* register_memory_region(struct ibv_pd *pd, void **buffer, size_t size) {
    int ret;
    struct ibv_mr *mr = NULL;

    /* 1. Allocate 64-byte cache-line aligned memory */
    ret = posix_memalign(buffer, 64, size);
    if (ret != 0) {
        fprintf(stderr, "Error: posix_memalign failed for size %zu\n", size);
        return NULL;
    }

    /* Clear allocated buffer content */
    memset(*buffer, 0, size);

    /* 2. Pin memory pages in physical RAM to prevent swapping (mlock) */
    if (mlock(*buffer, size) != 0) {
        fprintf(stderr, "Warning: mlock failed to lock memory pages. Continuing anyway...\n");
        /* Note: mlock can fail if RLIMIT_MEMLOCK is restricted on standard user VMs */
    } else {
        printf("[+] Successfully pinned %zu bytes in physical RAM using mlock.\n", size);
    }

    /* 3. Register Memory Region (MR) with the Protection Domain */
    mr = ibv_reg_mr(
        pd, 
        *buffer, 
        size, 
        IBV_ACCESS_LOCAL_WRITE | 
        IBV_ACCESS_REMOTE_WRITE | 
        IBV_ACCESS_REMOTE_READ
    );

    if (!mr) {
        fprintf(stderr, "Error: ibv_reg_mr failed to register memory region.\n");
        munlock(*buffer, size);
        free(*buffer);
        *buffer = NULL;
        return NULL;
    }

    printf("[+] Successfully registered Memory Region (MR):\n");
    printf("    - Buffer Address : %p\n", *buffer);
    printf("    - Buffer Size    : %zu bytes\n", size);
    printf("    - Local Key (lkey) : 0x%x\n", mr->lkey);
    printf("    - Remote Key (rkey): 0x%x\n", mr->rkey);

    return mr;
}

/* Graceful Cleanup for Memory Regions */
void deregister_memory_region(struct ibv_mr *mr, void *buffer) {
    printf("\n[+] Cleaning up memory resources...\n");
    if (mr) {
        if (ibv_dereg_mr(mr) != 0) {
            fprintf(stderr, "Warning: Failed to deregister memory region.\n");
        } else {
            printf("    - Memory Region deregistered via ibv_dereg_mr.\n");
        }
    }
    if (buffer) {
        munlock(buffer, 1024 * 1024); /* Unlock pages */
        free(buffer);                 /* Free aligned memory */
        printf("    - Pinned buffer freed and un-mlocked.\n");
    }
}

