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


#endif /* RDMA_COMMON_H */

