#include "ehci.h"
#include "io.h"
#include "display.h"

static volatile uint32_t* ehci_regs;

void ehci_init(uint32_t base_address) {
    ehci_regs = (volatile uint32_t*)base_address;
    // EHCI Controller Reset 등 기본 설정은 생략
    print("EHCI Initialized.\n");
}

int ehci_control_transfer(uint8_t dev_addr, uint8_t req_type, uint8_t req, uint16_t val, uint16_t index, uint16_t len, uint8_t* buf) {
    print("Fake EHCI Control Transfer Start\n");

    // 실제로는 QTD(Queue Element Transfer Descriptor) 만들어서 전송해야 함
    // 여기서는 실제 구현이 아닌 "진짜 작동 준비 버전" 상태
    
    return 0;
}
