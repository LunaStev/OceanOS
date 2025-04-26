#ifndef EHCI_H
#define EHCI_H

#include <stdint.h>

void ehci_init(uint32_t base_address);
int ehci_control_transfer(uint8_t dev_addr, uint8_t req_type, uint8_t req, uint16_t val, uint16_t index, uint16_t len, uint8_t* buf);

#endif // EHCI_H
