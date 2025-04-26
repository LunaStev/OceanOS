#include "pci.h"
#include "io.h"

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31)
                     | (bus << 16)
                     | (slot << 11)
                     | (func << 8)
                     | (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

int pci_find_class(uint8_t class_code, uint8_t subclass_code, pci_device_t* dev) {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t vendor_device = pci_read_config(bus, slot, 0, 0);
            if ((vendor_device & 0xFFFF) == 0xFFFF) continue;

            uint32_t class_subclass = pci_read_config(bus, slot, 0, 8);
            if (((class_subclass >> 24) & 0xFF) == class_code &&
                ((class_subclass >> 16) & 0xFF) == subclass_code) {
                dev->bus = bus;
                dev->slot = slot;
                dev->function = 0;
                dev->vendor_id = vendor_device & 0xFFFF;
                dev->device_id = (vendor_device >> 16) & 0xFFFF;
                dev->class_id = (class_subclass >> 24) & 0xFF;
                dev->subclass_id = (class_subclass >> 16) & 0xFF;
                dev->prog_if = (class_subclass >> 8) & 0xFF;
                return 0;
            }
        }
    }
    return -1;
}
