#include "rdma_common.h"

int main(int argc, char *argv[]) {
    struct ibv_context *context = NULL;
    struct ibv_pd *pd = NULL;

    printf("====================================================\n");
    printf(" Day 1: Modular Test Harness (Context & PD Setup)   \n");
    printf("====================================================\n\n");

    /* 1. Initialize Context for Soft-RoCE ('rxe0') */
    context = init_rdma_context("rxe0");
    if (!context) {
        return EXIT_FAILURE;
    }

    /* 2. Query Device & Port Attributes */
    if (query_rdma_device_and_port(context) != 0) {
        ibv_close_device(context);
        return EXIT_FAILURE;
    }

    /* 3. Allocate Protection Domain */
    pd = allocate_protection_domain(context);
    if (!pd) {
        ibv_close_device(context);
        return EXIT_FAILURE;
    }

    /* 4. Teardown & Cleanup */
    cleanup_rdma_resources(context, pd);

    printf("\n[SUCCESS] Day 1 modular execution complete!\n");
    return EXIT_SUCCESS;
}

