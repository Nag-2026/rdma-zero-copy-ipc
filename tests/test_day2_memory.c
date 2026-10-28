#include "rdma_common.h"

int main(int argc, char *argv[]) {
    struct ibv_context *context = NULL;
    struct ibv_pd *pd = NULL;
    struct ibv_mr *mr = NULL;
    void *buffer = NULL;
    size_t buffer_size = 1024 * 1024; // 1 MB buffer

    printf("====================================================\n");
    printf(" Day 2: Memory Pinning & Registration Test Harness  \n");
    printf("====================================================\n\n");

    /* 1. Initialize Context */
    context = init_rdma_context("rxe0");
    if (!context) return EXIT_FAILURE;

    /* 2. Allocate Protection Domain */
    pd = allocate_protection_domain(context);
    if (!pd) {
        ibv_close_device(context);
        return EXIT_FAILURE;
    }

    /* 3. Allocate, Pin, and Register Memory Region (Day 2 Core) */
    mr = register_memory_region(pd, &buffer, buffer_size);
    if (!mr) {
        ibv_dealloc_pd(pd);
        ibv_close_device(context);
        return EXIT_FAILURE;
    }

    /* 4. Teardown & Graceful Cleanup in Reverse Order */
    deregister_memory_region(mr, buffer);
    if (pd) ibv_dealloc_pd(pd);
    if (context) ibv_close_device(context);

    printf("\n[SUCCESS] Day 2 memory registration complete!\n");
    return EXIT_SUCCESS;
}
