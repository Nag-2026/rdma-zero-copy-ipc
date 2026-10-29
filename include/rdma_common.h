#ifndef RDMA_COMMON_H
#define RDMA_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <infiniband/verbs.h>

#include <sys/mman.h> // D2

/* Function prototypes for Day 1: Device Context & Protection Domain */
struct ibv_context* init_rdma_context(const char *device_name);
int query_rdma_device_and_port(struct ibv_context *context);
struct ibv_pd* allocate_protection_domain(struct ibv_context *context);
void cleanup_rdma_resources(struct ibv_context *context, struct ibv_pd *pd);

/* Day 2 Prototypes: Memory Registration & Pinning */
struct ibv_mr* register_memory_region(struct ibv_pd *pd, void **buffer, size_t size);
void deregister_memory_region(struct ibv_mr *mr, void *buffer);

/* Day 3 Prototypes: CQ and QP Management */
struct ibv_cq* create_completion_queue(struct ibv_context *context, int cqe_capacity);
struct ibv_qp* create_queue_pair(struct ibv_pd *pd, struct ibv_cq *cq, enum ibv_qp_type qp_type);
int modify_qp_to_init(struct ibv_qp *qp, uint8_t port_num);
int modify_qp_to_rtr(struct ibv_qp *qp, uint32_t dest_qp_num, uint16_t dest_lid, uint8_t port_num);
int modify_qp_to_rts(struct ibv_qp *qp);
void cleanup_transport_resources(struct ibv_qp *qp, struct ibv_cq *cq);

/* Day 4 Connection metadata exchanged out-of-band via TCP */
struct rdma_conn_data {
    uint32_t qp_num;          /* Queue Pair Number */
    uint16_t lid;             /* Local Identifier (LID) */
    uint64_t addr;            /* Buffer virtual address */
    uint32_t rkey;            /* Remote memory key */
};

/* Day 4 Prototypes: Out-of-Band TCP Handshake helpers */
int tcp_server_connect(int port);
int tcp_client_connect(const char *server_ip, int port);

#endif /* RDMA_COMMON_H */
