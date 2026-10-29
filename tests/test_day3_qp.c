#include "rdma_common.h"

int main(int argc, char *argv[]) {
    struct ibv_context *context = NULL;
    struct ibv_pd *pd = NULL;
    struct ibv_cq *cq = NULL;
    struct ibv_qp *qp = NULL;
    uint8_t port_num = 1;

    printf("====================================================\n");
    printf(" Day 3: Completion Queue & Queue Pair Test Harness  \n");
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

    /* 3. Create Completion Queue (Capacity: 100 entries) */
    cq = create_completion_queue(context, 100);
    if (!cq) {
        ibv_dealloc_pd(pd);
        ibv_close_device(context);
        return EXIT_FAILURE;
    }

    /* 4. Create Reliable Connection (RC) Queue Pair */
    qp = create_queue_pair(pd, cq, IBV_QPT_RC);
    if (!qp) {
        ibv_destroy_cq(cq);
        ibv_dealloc_pd(pd);
        ibv_close_device(context);
        return EXIT_FAILURE;
    }

    /* 5. Execute QP State Transitions (RESET ➔ INIT ➔ RTR ➔ RTS) */
    printf("\n[+] Executing QP State Machine Transitions:\n");
    
    if (modify_qp_to_init(qp, port_num) != 0) goto cleanup;
    
    /* For self-loopback test on Soft-RoCE, we transition using local QP attributes */
    if (modify_qp_to_rtr(qp, qp->qp_num, 1, port_num) != 0) goto cleanup;
    
    if (modify_qp_to_rts(qp) != 0) goto cleanup;

    printf("\n[SUCCESS] Day 3 QP setup and state transitions complete!\n");

cleanup:
    /* 6. Graceful Cleanup in Reverse Order */
    cleanup_transport_resources(qp, cq);
    if (pd) ibv_dealloc_pd(pd);
    if (context) ibv_close_device(context);

    return EXIT_SUCCESS;
}
