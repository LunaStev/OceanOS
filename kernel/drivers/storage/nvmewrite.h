#ifndef NVMEWRITE_H
#define NVMEWRITE_H

#include <stdint.h>
#include "nvme.h"  // 기존 NVMe 컨트롤러 정보와 기본 타입 사용

// NVMe Write Command function
int nvme_write(nvme_ctrl_t* ctrl, uint32_t nsid, uint64_t lba, uint16_t count, const void* buffer);

#endif // NVMEWRITE_H
