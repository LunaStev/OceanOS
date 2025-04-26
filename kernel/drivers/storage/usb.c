// OceanOS
// Kernel Drivers Storage
// usb.c
#include "usb.h"
#include "ehci.h"
#include "pci.h"
#include "display.h"

static uint8_t next_device_addr = 1;

void usb_init() {
    pci_device_t dev;
    if (pci_find_class(0x0C, 0x03, &dev) == 0) { // USB Controller
        uint32_t bar = pci_read_config(dev.bus, dev.slot, 0, 0x10) & ~0xF;
        ehci_init(bar);
    } else {
        print("No USB controller found.\n");
    }
}

int usb_enumerate(usb_device_t* device) {
    uint8_t device_descriptor[18];

    if (ehci_control_transfer(0, 0x80, 0x06, 0x0100, 0x00, 18, device_descriptor) != 0) {
        print("Control transfer failed.\n");
        return -1;
    }

    device->vid = device_descriptor[8] | (device_descriptor[9] << 8);
    device->pid = device_descriptor[10] | (device_descriptor[11] << 8);

    device->address = next_device_addr++;
    ehci_control_transfer(0, 0x00, 0x05, device->address, 0, 0, 0); // SET_ADDRESS

    print("USB Device Found\n");
    return 0;
}
