#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "adxl345_driver.h"

const int spiSpeed = 2000000;  // SPI communication speed, bps

typedef struct {
	char bwRate;
	char dataFormat;
	char powerCtl;
	char fifoCtl;
} adxl;



int setupAdxl(int spiSpeed, adxl conf) {
    int h = openAdxl(spiSpeed); 
    setAdxlRegister(h, BW_RATE, conf.bwRate);
    char xArr[2];
    xArr[0] = DATA_FORMAT;
    xArr[1] = RANGE_PM_2g;
    writeAdxlBytes(h, xArr, 2);
    xArr[0] = POWER_CTL;
    xArr[1] = PCTL_MEASURE;
    writeAdxlBytes(h, xArr, 2);
    return h;
}

void getReading(int h, int16_t * x) {
    int dataLen = 10;
    char data[dataLen];
    data[0] = DATAX0;
    int bytes = readAdxlBytes(h, data, dataLen);
    if (bytes != dataLen) {
    printf("Error occurred!");
    }
    x[0] = ((int)data[2]<<8)|(int)data[1];
    x[1] = ((int)data[4]<<8)|(int)data[3];
    x[2] = ((int)data[6]<<8)|(int)data[5];
}


int main() {
    // SPI sensor setup
    adxl conf;
    conf.bwRate = RATE_800_HZ;
    int h = setupAdxl(spiSpeed, conf); 
    int dataLen = 10;
    char data[dataLen];
    int16_t x[3];
    int success = 1;
    int bytes;
    data[0] = DATAX0;
    bytes = readAdxlBytes(h, data, dataLen);
    if (bytes != dataLen) {
	success = 0;
    }
    getReading(h, x);


if (success == 0) {
    printf("Error occurred!");
    return 1;
}
   printf("X,Y,Z %d %d %d\n", x[0], x[1], x[2]);

   for(int loop = 0; loop < dataLen; loop++) { 
      printf("%d ", data[loop]);
   }
    printf("Done ;) .\n");
    return 0;
    teardownAdxl(); 
}
