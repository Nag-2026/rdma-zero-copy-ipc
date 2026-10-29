#include "rdma_common.h"

/* Step 1: Create Completion Queue (CQ) */
struct ibv_cq* create_completion_queue(struct ibv_context *context, int cqe_capacity) {
    /* Parameters: context, cqe (capacity), user_context, completion_channel, comp_vector */
    struct ibv_cq *cq = ibv_create_cq(context, cqe_capacity, NULL, NULL, 0);
    if (!cq) {
        fprintf(stderr, "Error: Failed to create Completion Queue (CQ).\n");
        return NULL;
    }
    printf("[+] Successfully created Completion Queue (Capacity: %d)\n", cqe_capacity);
    return cq;
}

/* Step 2: Create Queue Pair (QP) */
struct ibv_qp* create_queue_pair(struct ibv_pd *pd, struct ibv_cq *cq, enum ibv_qp_type qp_type) {
    struct ibv_qp_init_attr qp_init_attr;
    memset(&qp_init_attr, 0, sizeof(qp_init_attr));

    qp_init_attr.send_cq = cq;
    qp_init_attr.recv_cq = cq;
    qp_init_attr.qp_type = qp_type; // e.g., IBV_QPT_RC (Reliable Connection)
    
    /* Set maximum limits for Work Requests in Send and Receive queues */
    qp_init_attr.cap.max_send_wr  = 128;
    qp_init_attr.cap.max_recv_wr  = 128;
    qp_init_attr.cap.max_send_sge = 1;  /* Scatter/Gather elements per request */
    qp_init_attr.cap.max_recv_sge = 1;

    struct ibv_qp *qp = ibv_create_qp(pd, &qp_init_attr);
    if (!qp) {
        fprintf(stderr, "Error: Failed to create Queue Pair (QP).\n");
        return NULL;
    }
    printf("[+] Successfully created Queue Pair (QP Number: 0x%x)\n", qp->qp_num);
    return qp;
}

/* Step 3: Transition QP from RESET to INIT state */
int modify_qp_to_init(struct ibv_qp *qp, uint8_t port_num) {
    struct ibv_qp_attr attr;
    memset(&attr, 0, sizeof(attr));

    attr.qp_state   = IBV_QPS_INIT;
    attr.pkey_index = 0;
    attr.port_num   = port_num;
    attr.qp_access_flags = IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ;

    int flags = IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS;
    
    if (ibv_modify_qp(qp, &attr, flags) != 0) {
        fprintf(stderr, "Error: Failed to modify QP to INIT state.\n");
        return -1;
    }
    printf("[+] QP successfully transitioned to INIT state.\n");
    return 0;
}

/* Step 4: Transition QP from INIT to Ready-to-Receive (RTR) state */
int modify_qp_to_rtr(struct ibv_qp *qp, uint32_t dest_qp_num, uint16_t dest_lid, uint8_t port_num) {
    struct ibv_qp_attr attr;
    memset(&attr, 0, sizeof(attr));

    attr.qp_state              = IBV_QPS_RTR;
    attr.path_mtu              = IBV_MTU_1024;
    attr.dest_qp_num           = dest_qp_num;
    attr.rq_psn                = 0; // Packet Sequence Number
    attr.max_dest_rd_atomic    = 1;
    attr.min_rnr_timer         = 12;
    attr.ah_attr.is_global     = 0; // Set to 1 if using RoCEv2 Global Routing Header (GRH)
    attr.ah_attr.dlid          = dest_lid;
    attr.ah_attr.sl            = 0;
    attr.ah_attr.src_path_bits = 0;
    attr.ah_attr.port_num      = port_num;

    int flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
                IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;

    if (ibv_modify_qp(qp, &attr, flags) != 0) {
        fprintf(stderr, "Error: Failed to modify QP to RTR state.\n");
        return -1;
    }
    printf("[+] QP successfully transitioned to RTR state.\n");
    return 0;
}

/* Step 5: Transition QP from RTR to Ready-to-Send (RTS) state */
int modify_qp_to_rts(struct ibv_qp *qp) {
    struct ibv_qp_attr attr;
    memset(&attr, 0, sizeof(attr));

    attr.qp_state           = IBV_QPS_RTS;
    attr.timeout            = 14;
    attr.retry_cnt          = 7;
    attr.rnr_retry          = 7;
    attr.sq_psn             = 0;
    attr.max_rd_atomic      = 1;

    int flags = IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | 
                IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC;

    if (ibv_modify_qp(qp, &attr, flags) != 0) {
        fprintf(stderr, "Error: Failed to modify QP to RTS state.\n");
        return -1;
    }
    printf("[+] QP successfully transitioned to RTS (Ready-to-Send) state.\n");
    return 0;
}

/* Cleanup Transport Resources */
void cleanup_transport_resources(struct ibv_qp *qp, struct ibv_cq *cq) {
    printf("\n[+] Cleaning up transport resources...\n");
    if (qp) {
        ibv_destroy_qp(qp);
        printf("    - Queue Pair destroyed.\n");
    }
    if (cq) {
        ibv_destroy_cq(cq);
        printf("    - Completion Queue destroyed.\n");
    }
}
