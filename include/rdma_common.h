#ifndef RDMA_COMMON_H
#define RDMA_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <infiniband/verbs.h>

/* Function prototypes for Day 1: Device Context & Protection Domain */
struct ibv_context* init_rdma_context(const char *device_name);
int query_rdma_device_and_port(struct ibv_context *context);
struct ibv_pd* allocate_protection_domain(struct ibv_context *context);
void cleanup_rdma_resources(struct ibv_context *context, struct ibv_pd *pd);

#endif /* RDMA_COMMON_H */

