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

void xgetreading(int h, int16_t *x) {
  int dataLen = 7;
  char data[dataLen];
  data[0] = DATAX0;
  int bytes = readBytes(h, data, 7);
  if (bytes != dataLen) {
    printf("Error occurred!");
  }
  printf("\n0* " );
  printArr(data, dataLen);
  x[0] = (int16_t)(data[2] << 8 | data[1]);
  x[1] = (int16_t)(data[4] << 8 | data[3]);
  x[2] = (int16_t)(data[6] << 8 | data[5]);
  printf("\n0# %i\t%i\t%i\t", x[0], x[1], x[2]);
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
    xgetreading(h, x);
    msleep(10);
  }
  int success = 1;
  int bytes;
  teardownAdxl();
  printf("Done ;( .\n");
  return 0;
}
