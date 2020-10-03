#include <math.h>
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}
#include "adxl345_driver.h"

const int spiSpeed = 2000000; // SPI communication speed, bps

typedef struct {
  char bwRate;
  char dataFormat;
  char powerCtl;
  char fifoCtl;
} adxl;

int setupAdxl(int spiSpeed, adxl conf) {
  int h = openAdxl(spiSpeed);
  setAdxlRegister(h, BW_RATE, conf.bwRate);
  setAdxlRegister(h, DATA_FORMAT, conf.dataFormat);
  setAdxlRegister(h, POWER_CTL, conf.powerCtl);
  return h;
}

void getReading(int h, int16_t *x) {
  int dataLen = 7;
  char data[dataLen];
  data[0] = DATAX0;
  int bytes = readAdxlBytes(h, data, dataLen);
  printf("* " );
  printArr(data, dataLen);
  if (bytes != dataLen) {
    printf("Error occurred!");
  }
  x[0] = ((int)data[2] << 8) | (int)data[1];
  x[1] = ((int)data[4] << 8) | (int)data[3];
  x[2] = ((int)data[6] << 8) | (int)data[5];
}

int main() {
  // SPI sensor setup
  adxl conf;
  conf.bwRate = RATE_3200_HZ;
  conf.dataFormat = (RANGE_PM_2g & 3);
  conf.powerCtl = PCTL_MEASURE;
  int h = setupAdxl(spiSpeed, conf);

  int dataLen = 7;
  char data[dataLen];
  int16_t x[3];
  for (int loop = 0; loop < 10; loop++) {
    getReading(h, x);
    printf("X,Y,Z %d %d %d\n", x[0], x[1], x[2]);
    msleep(10);
  }
  getReading(h, x);
  int success = 1;
  int bytes;
  data[0] = DATAX0;
  bytes = readAdxlBytes(h, data, dataLen);
  if (bytes != dataLen) {
    success = 0;
  }

  if (success == 0) {
    printf("Error occurred!");
    return 1;
  }
  printf("X,Y,Z %d %d %d\n", x[0], x[1], x[2]);

  for (int loop = 0; loop < dataLen; loop++) {
    printf("%d ", data[loop]);
  }
  teardownAdxl();
  printf("Done ;( .\n");
  return 0;
}
