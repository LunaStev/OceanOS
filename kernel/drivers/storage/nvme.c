// OceanOS
// Kernel Drivers Storage
// nvme.h
#include "nvme.h"
#include "io.h"
#include "display.h"

#define NVME_ADMIN_QUEUE_SIZE (NVME_QUEUE_DEPTH * sizeof(nvme_command_t))
#define NVME_COMPLETION_QUEUE_SIZE (NVME_QUEUE_DEPTH * sizeof(nvme_completion_t))

static inline void mmio_write32(volatile uint32_t* base, uint32_t reg, uint32_t val) {
    base[reg / 4] = val;
}

static inline uint32_t mmio_read32(volatile uint32_t* base, uint32_t reg) {
    return base[reg / 4];
}

static void wait_until_ready(volatile uint32_t* bar) {
    while (!(mmio_read32(bar, NVME_REG_CSTS) & NVME_CSTS_RDY)) {
        // wait
    }
}

int nvme_init(nvme_ctrl_t* ctrl, volatile uint32_t* mmio_base) {
    ctrl->bar = mmio_base;
    ctrl->sq_tail = 0;
    ctrl->cq_head = 0;
    ctrl->phase = 1;

    // Queue memory 할당 (여기선 고정 주소 가정, 필요시 kmalloc)
    ctrl->admin_sq = (nvme_command_t*) 0x100000; // 예: 1MB 위치
    ctrl->admin_cq = (nvme_completion_t*) 0x101000; // 바로 뒤에

    // Admin Queue 설정
    mmio_write32(ctrl->bar, NVME_REG_AQA, (NVME_QUEUE_DEPTH-1) | ((NVME_QUEUE_DEPTH-1) << 16));
    mmio_write32(ctrl->bar, NVME_REG_ASQ, (uint32_t)(uint64_t)(ctrl->admin_sq));
    mmio_write32(ctrl->bar, NVME_REG_ACQ, (uint32_t)(uint64_t)(ctrl->admin_cq));

    // 컨트롤러 Enable
    uint32_t cc = 0;
    cc |= (0x6 << 20); // IOSQES=6 (64B)
    cc |= (0x4 << 16); // IOCQES=4 (16B)
    cc |= (1 << 0);    // Enable

    mmio_write32(ctrl->bar, NVME_REG_CC, cc);

    wait_until_ready(ctrl->bar);

    print("NVMe initialized.\n");
    return 0;
}

static int submit_admin_command(nvme_ctrl_t* ctrl, nvme_command_t* cmd) {
    // 큐에 넣기
    ctrl->admin_sq[ctrl->sq_tail] = *cmd;

    // SQ Tail 증가
    ctrl->sq_tail = (ctrl->sq_tail + 1) % NVME_QUEUE_DEPTH;

    // Doorbell (여기선 레지스터는 0x1000 base에 추가될 수도 있음, HW설계마다 다름 → 간단화)
    mmio_write32(ctrl->bar, 0x1000, ctrl->sq_tail);

    // CQ polling (간단화)
    while (1) {
        nvme_completion_t cpl = ctrl->admin_cq[ctrl->cq_head];
        if ((cpl.status & 1) == ctrl->phase) {
            ctrl->cq_head = (ctrl->cq_head + 1) % NVME_QUEUE_DEPTH;
            if (ctrl->cq_head == 0)
                ctrl->phase ^= 1;
            break;
        }
    }
    return 0;
}

int nvme_identify(nvme_ctrl_t* ctrl, void* buffer) {
    nvme_command_t cmd;
    for (int i = 0; i < sizeof(cmd)/4; i++) ((uint32_t*)&cmd)[i] = 0; // zero

    cmd.cdw0 = (NVME_OPC_IDENTIFY & 0xFF);
    cmd.prp1 = (uint64_t)buffer;
    cmd.cdw10 = 1; // CNS=1 (Identify Controller)

    submit_admin_command(ctrl, &cmd);

    print("NVMe Identify done.\n");
    return 0;
}

int nvme_read(nvme_ctrl_t* ctrl, uint32_t nsid, uint64_t lba, uint16_t count, void* buffer) {
    nvme_command_t cmd;
    for (int i = 0; i < sizeof(cmd)/4; i++) ((uint32_t*)&cmd)[i] = 0; // zero

    cmd.cdw0 = (NVME_OPC_READ & 0xFF);
    cmd.nsid = nsid;
    cmd.prp1 = (uint64_t)buffer;
    cmd.cdw10 = (uint32_t)(lba & 0xFFFFFFFF);
    cmd.cdw11 = (uint32_t)((lba >> 32) & 0xFFFFFFFF);
    cmd.cdw12 = count - 1; // Number of blocks -1

    submit_admin_command(ctrl, &cmd);

    print("NVMe Read done.\n");
    return 0;
}
