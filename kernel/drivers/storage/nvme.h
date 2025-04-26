#ifndef NVME_H
#define NVME_H

#include <stdint.h>

// NVMe Controller Registers (offset 기준)
#define NVME_REG_CAP   0x0000  // Controller Capabilities
#define NVME_REG_VS    0x0008  // Version
#define NVME_REG_CC    0x0014  // Controller Configuration
#define NVME_REG_CSTS  0x001C  // Controller Status
#define NVME_REG_AQA   0x0024  // Admin Queue Attributes
#define NVME_REG_ASQ   0x0028  // Admin Submission Queue Base Address
#define NVME_REG_ACQ   0x0030  // Admin Completion Queue Base Address

// NVMe Command Opcodes
#define NVME_OPC_IDENTIFY 0x06
#define NVME_OPC_READ     0x02
#define NVME_OPC_WRITE    0x01

// NVMe Status Codes
#define NVME_CSTS_RDY 0x00000001

// Queue depth
#define NVME_QUEUE_DEPTH 32

// NVMe command structure
typedef struct {
    uint32_t cdw0;    // opcode and fuse
    uint32_t nsid;
    uint64_t reserved;
    uint64_t mptr;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
} nvme_command_t;

// NVMe completion structure
typedef struct {
    uint32_t result;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t command_id;
    uint16_t status;
} nvme_completion_t;

// NVMe Controller Info
typedef struct {
    volatile uint32_t* bar; // Base address (MMIO region)
    nvme_command_t* admin_sq;
    nvme_completion_t* admin_cq;
    uint16_t sq_tail;
    uint16_t cq_head;
    uint16_t phase;
} nvme_ctrl_t;

// Functions
int nvme_init(nvme_ctrl_t* ctrl, volatile uint32_t* mmio_base);
int nvme_identify(nvme_ctrl_t* ctrl, void* buffer);
int nvme_read(nvme_ctrl_t* ctrl, uint32_t nsid, uint64_t lba, uint16_t count, void* buffer);

#endif // NVME_H
