#ifndef USB_H
#define USB_H

#include <stdint.h>

typedef struct {
    uint8_t address;
    uint16_t vid;
    uint16_t pid;
} usb_device_t;

void usb_init();
int usb_enumerate(usb_device_t* device);

#endif // USB_H
