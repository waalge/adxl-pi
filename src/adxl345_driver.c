#include "adxl345_driver.h"

#define READ_BIT      0x80
#define MULTI_BIT     0x40

int readBytes(int handle, char *data, int count) {
    data[0] |= READ_BIT;
    if (count > 1) data[0] |= MULTI_BIT;
    return spiXfer(handle, data, data, count);
}

int writeBytes(int handle, char *data, int count) {
    if (count > 1) data[0] |= MULTI_BIT;
    return spiWrite(handle, data, count);
}

int getSpi(int spiSpeed) {
    gpioInitialise();
    if (gpioInitialise() < 0) {
        printf("Failed to initialize GPIO!");
        return 1;
    }
    return spiOpen(0, spiSpeed, 3);
}

