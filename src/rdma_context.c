#include "rdma_common.h"

/* Step 1 & 2: Discover and open the requested RDMA device context */
struct ibv_context* init_rdma_context(const char *target_name) {
    int num_devices = 0;
    struct ibv_device **dev_list = NULL;
    struct ibv_device *target_dev = NULL;
    struct ibv_context *context = NULL;
    int i = 0;

    dev_list = ibv_get_device_list(&num_devices);
    if (!dev_list || num_devices == 0) {
        fprintf(stderr, "Error: No RDMA devices found. Verify 'rdma_rxe' is loaded.\n");
        return NULL;
    }

    printf("[+] Found %d RDMA device(s):\n", num_devices);
    for (i = 0; i < num_devices; i++) {
        printf("    - Device %d: %s\n", i, ibv_get_device_name(dev_list[i]));
        if (target_name && strcmp(ibv_get_device_name(dev_list[i]), target_name) == 0) {
            target_dev = dev_list[i];
        }
    }

    if (!target_dev) {
        printf("[!] Target device not found. Selecting default device: %s\n", 
               ibv_get_device_name(dev_list[0]));
        target_dev = dev_list[0];
    }

    context = ibv_open_device(target_dev);
    ibv_free_device_list(dev_list); // Free list pointer; doesn't close context

    if (!context) {
        fprintf(stderr, "Error: Failed to open device context.\n");
        return NULL;
    }

    printf("[+] Successfully opened verbs context for device.\n");
    return context;
}

/* Step 3: Query Device and Port Attributes (Exercise 1.2 extension) */
int query_rdma_device_and_port(struct ibv_context *context) {
    struct ibv_device_attr device_attr;
    struct ibv_port_attr port_attr;

    memset(&device_attr, 0, sizeof(device_attr));
    if (ibv_query_device(context, &device_attr) != 0) {
        fprintf(stderr, "Error: Failed to query device attributes.\n");
        return -1;
    }

    printf("[+] Device Attributes Queried Successfully:\n");
    printf("    - Max Queue Pairs (QPs)   : %d\n", device_attr.max_qp);
    printf("    - Max Memory Regions (MRs): %d\n", device_attr.max_mr);
    printf("    - Max CQ Entries          : %d\n", device_attr.max_cqe);
    printf("    - Physical Port Count     : %d\n", device_attr.phys_port_cnt);

    memset(&port_attr, 0, sizeof(port_attr));
    if (ibv_query_port(context, 1, &port_attr) != 0) {
        fprintf(stderr, "Error: Failed to query port 1 attributes.\n");
        return -1;
    }

    printf("[+] Port 1 Attributes:\n");
    printf("    - Port State : %d (Active = %d)\n", port_attr.state, IBV_PORT_ACTIVE);
    printf("    - Link Layer : %s\n", (port_attr.link_layer == IBV_LINK_LAYER_ETHERNET) ? "Ethernet (RoCEv2)" : "InfiniBand");

    return 0;
}

/* Step 4: Allocate Protection Domain */
struct ibv_pd* allocate_protection_domain(struct ibv_context *context) {
    struct ibv_pd *pd = ibv_alloc_pd(context);
    if (!pd) {
        fprintf(stderr, "Error: Failed to allocate Protection Domain (PD).\n");
        return NULL;
    }
    printf("[+] Successfully allocated Protection Domain (PD address: %p)\n", (void *)pd);
    return pd;
}

/* Graceful Cleanup */
void cleanup_rdma_resources(struct ibv_context *context, struct ibv_pd *pd) {
    printf("\n[+] Cleaning up resources...\n");
    if (pd) {
        if (ibv_dealloc_pd(pd) != 0) {
            fprintf(stderr, "Warning: Failed to deallocate Protection Domain.\n");
        } else {
            printf("    - Protection Domain deallocated.\n");
        }
    }
    if (context) {
        if (ibv_close_device(context) != 0) {
            fprintf(stderr, "Warning: Failed to close device context.\n");
        } else {
            printf("    - Device context closed.\n");
        }
    }
}

