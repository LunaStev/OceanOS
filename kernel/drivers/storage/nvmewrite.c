#include "nvmewrite.h"
#include "nvme.h"
#include "display.h"

extern int submit_admin_command(nvme_ctrl_t* ctrl, nvme_command_t* cmd); 
// (nvme.c 안에 구현돼있던 명령 전송 함수)

int nvme_write(nvme_ctrl_t* ctrl, uint32_t nsid, uint64_t lba, uint16_t count, const void* buffer) {
    nvme_command_t cmd;
    for (int i = 0; i < sizeof(cmd)/4; i++) ((uint32_t*)&cmd)[i] = 0; // zero fill

    cmd.cdw0 = (NVME_OPC_WRITE & 0xFF);
    cmd.nsid = nsid;
    cmd.prp1 = (uint64_t)buffer;
    cmd.cdw10 = (uint32_t)(lba & 0xFFFFFFFF);
    cmd.cdw11 = (uint32_t)((lba >> 32) & 0xFFFFFFFF);
    cmd.cdw12 = count - 1; // Number of blocks - 1

    submit_admin_command(ctrl, &cmd);

    print("NVMe Write done.\n");
    return 0;
}
