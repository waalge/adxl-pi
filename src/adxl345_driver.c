#include "adxl345_driver.h"

#define READ_BIT 0x80
#define MULTI_BIT 0x40

void printArr(char* arr, int arrLen) {
  for (int loop = 0; loop < arrLen; loop++) {
    printf("%d\t", arr[loop]);
  }
}

int openAdxl(int spiSpeed) {
  gpioInitialise();
  if (gpioInitialise() < 0) {
    printf("Failed to initialize GPIO!");
    return 1;
  }
  return spiOpen(0, spiSpeed, 3);
}

int setupAdxl(int spiSpeed, adxlConfig conf) {
  int h = openAdxl(spiSpeed);
  setAdxlRegister(h, BW_RATE, conf.bwRate);
  setAdxlRegister(h, DATA_FORMAT, conf.dataFormat);
  setAdxlRegister(h, POWER_CTL, conf.powerCtl);
  setAdxlRegister(h, FIFO_CTL, conf.fifoCtl);
  return h;
}

int readAdxlBytes(int handle, char *data, int count) {
    data[0] |= READ_BIT;
    if (count > 1) data[0] |= MULTI_BIT;
    return spiXfer(handle, data, data, count);
}

void readAdxlTriple(int devId, int16_t *triple) {
  int dataLen = 7;
  char data[dataLen];
  data[0] = DATAX0;
  int bytes = readAdxlBytes(devId, data, 7);
  if (bytes != dataLen) {
    printf("Can't read triple!");
  }
  triple[0] = (int16_t)(data[2] << 8 | data[1]);
  triple[1] = (int16_t)(data[4] << 8 | data[3]);
  triple[2] = (int16_t)(data[6] << 8 | data[5]);
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

void teardownAdxl() { return gpioTerminate(); }
