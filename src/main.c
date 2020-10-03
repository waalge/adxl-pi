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
  int dataLen = 6;
  unsigned char data[dataLen];
  int bytes = readAdxlBytes(h, DATAX0, dataLen, data);
  if (bytes != dataLen) {
    printf("Error occurred!");
  }
  printf("* " );
  printArr(data, dataLen);
  //
  x[0] = (int16_t)(data[1] << 8 | data[0]);
  x[1] = (int16_t)(data[3] << 8 | data[2]);
  x[2] = (int16_t)(data[5] << 8 | data[4]);
  printf("# %i    %i    %i\n", x[0], x[1], x[2]);
}

int main() {
  // SPI sensor setup
  adxl conf;
  conf.bwRate = RATE_800_HZ;
  conf.dataFormat = RANGE_PM_2g;
  conf.powerCtl = PCTL_MEASURE;
  int h = setupAdxl(spiSpeed, conf);
  int16_t x[3];
  for (int loop = 0; loop < 10; loop++) {
    getReading(h, x);
    msleep(10);
  }
  int success = 1;
  int bytes;
  int dataLen = 6;
  unsigned char data[dataLen];
  bytes = readAdxlBytes(h, DATAX0, dataLen, data);
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
