#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "adxl345_driver.h"

const int spiSpeed = 2000000; // SPI communication speed, bps

int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

adxlConfig makeConfig () {
  adxlConfig conf;
  conf.bwRate = RATE_800_HZ;
  conf.dataFormat = RANGE_PM_2g;
  conf.powerCtl = PCTL_MEASURE;
  conf.fifoCtl = FIFO_STREAM;
  return conf;
}

int main() {
  // SPI sensor setup
  adxlConfig conf = makeConfig();
  int devId = setupAdxl(spiSpeed, conf);
  int16_t triple[3];
  for (int loop = 0; loop < 10; loop++) {
    readAdxlTriple(devId, triple);
    msleep(10);
  }
  int success = 1;
  int bytes;
  teardownAdxl();
  printf("Done ;( .\n");
  return 0;
}
