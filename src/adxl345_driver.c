#include "adxl345_driver.h"

#define READ_BIT 0x80
#define MULTI_BIT 0x40

void printArr(char* arr, int arrLen) {
  for (int loop = 0; loop < arrLen; loop++) {
    printf("%d ", arr[loop]);
  }
    printf("\n");
}

int readAdxlBytes(int handle, int address, int count, char *data) {
  address |= READ_BIT;
  if (count > 1)
    address |= MULTI_BIT;
  return spiXfer(handle, (char *)(&address), data, count);
}

int writeAdxlBytes(int handle, char *data, int count) {
  if (count > 1)
    data[0] |= MULTI_BIT;
  return spiWrite(handle, data, count);
}

int setAdxlRegister(int handle, char address, char dat) {
  char data[] = {address, dat};
  return spiWrite(handle, data, 2);
}

int openAdxl(int spiSpeed) {
  gpioInitialise();
  if (gpioInitialise() < 0) {
    printf("Failed to initialize GPIO!");
    return 1;
  }
  return spiOpen(0, spiSpeed, 3);
}

void teardownAdxl() { return gpioTerminate(); }
