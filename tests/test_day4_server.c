#include "rdma_common.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    struct ibv_context *context = NULL;
    struct ibv_pd *pd = NULL;
    struct ibv_cq *cq = NULL;
    struct ibv_qp *qp = NULL;
    struct ibv_mr *mr = NULL;
    void *buffer = NULL;
    size_t buffer_size = 1024 * 1024; // 1 MB
    uint8_t port_num = 1;
    int tcp_port = 12345;
    int tcp_fd = -1;

    printf("====================================================\n");
    printf(" Day 4: RDMA Server (Receiver) Handshake & Setup    \n");
    printf("====================================================\n\n");

    /* 1. Initialize RDMA context, PD, CQ, QP, and registered memory */
    context = init_rdma_context("rxe0");
    if (!context) return EXIT_FAILURE;

    pd = allocate_protection_domain(context);
    if (!pd) goto cleanup_context;

    cq = create_completion_queue(context, 100);
    if (!cq) goto cleanup_pd;

    qp = create_queue_pair(pd, cq, IBV_QPT_RC);
    if (!qp) goto cleanup_cq;

    mr = register_memory_region(pd, &buffer, buffer_size);
    if (!mr) goto cleanup_qp;

    /* 2. Transition QP to INIT */
    if (modify_qp_to_init(qp, port_num) != 0) goto cleanup_mr;

    /* 3. Start OOB TCP Server and accept client connection */
    tcp_fd = tcp_server_connect(tcp_port);
    if (tcp_fd < 0) goto cleanup_mr;

    /* 4. Query local port attributes to get LID */
    struct ibv_port_attr port_attr;
    memset(&port_attr, 0, sizeof(port_attr));
    if (ibv_query_port(context, port_num, &port_attr) != 0) {
        fprintf(stderr, "Error: Failed to query port attributes.\n");
        goto cleanup_tcp;
    }

    /* 5. Prepare local connection metadata to send to client */
    struct rdma_conn_data local_data, remote_data;
    local_data.qp_num = qp->qp_num;
    local_data.lid    = port_attr.lid;
    local_data.addr   = (uint64_t)buffer;
    local_data.rkey   = mr->rkey;

    /* 6. Exchange metadata over TCP socket */
    if (write(tcp_fd, &local_data, sizeof(local_data)) != sizeof(local_data)) {
        perror("write local_data");
        goto cleanup_tcp;
    }
    if (read(tcp_fd, &remote_data, sizeof(remote_data)) != sizeof(remote_data)) {
        perror("read remote_data");
        goto cleanup_tcp;
    }

    printf("[+] Received Remote Connection Metadata:\n");
    printf("    - Remote QPN  : 0x%x\n", remote_data.qp_num);
    printf("    - Remote LID  : %u\n", remote_data.lid);
    printf("    - Remote Addr : 0x%lx\n", remote_data.addr);
    printf("    - Remote RKey : 0x%x\n", remote_data.rkey);

    /* 7. Transition QP to RTR (Ready-to-Receive) using remote peer metadata */
    if (modify_qp_to_rtr(qp, remote_data.qp_num, remote_data.lid, port_num) != 0) goto cleanup_tcp;

    /* 8. Transition QP to RTS (Ready-to-Send) */
    if (modify_qp_to_rts(qp) != 0) goto cleanup_tcp;

    printf("\n[SUCCESS] Server QP is fully armed and in RTS state!\n");

cleanup_tcp:
    if (tcp_fd >= 0) close(tcp_fd);
cleanup_mr:
    deregister_memory_region(mr, buffer);
cleanup_qp:
    if (qp) ibv_destroy_qp(qp);
cleanup_cq:
    if (cq) ibv_destroy_cq(cq);
cleanup_pd:
    if (pd) ibv_dealloc_pd(pd);
cleanup_context:
    if (context) ibv_close_device(context);

    return EXIT_SUCCESS;
}
