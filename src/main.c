#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "adxl345_driver.h"

const int spiSpeed = 2000000;  // SPI communication speed, bps

int setupAdxl(int spiSpeed) {
    int h = openAdxl(spiSpeed); 
    char xArr[2];
    xArr[0] = BW_RATE;
    xArr[1] = RATE_800_HZ;
    writeAdxlBytes(h, xArr, 2);
    xArr[0] = DATA_FORMAT;
    xArr[1] = RANGE_PM_2g;
    writeAdxlBytes(h, xArr, 2);
    xArr[0] = POWER_CTL;
    xArr[1] = PCTL_MEASURE;
    writeAdxlBytes(h, xArr, 2);
    return h;
}

void getReading(int h, int16_t x, int16_t y, int16_t z) {
    int dataLen = 10;
    char data[dataLen];
    data[0] = DATAX0;
    int bytes = readAdxlBytes(h, data, dataLen);
    if (bytes != dataLen) {
    printf("Error occurred!");
    }
    x = (data[2]<<8)|data[1];
    y = (data[4]<<8)|data[3];
    z = (data[6]<<8)|data[5];
}


int main() {
    // SPI sensor setup
    int h = setupAdxl(spiSpeed); 
    int dataLen = 10;
    char data[dataLen];
    int16_t x, y, z;
    int success = 1;
    int bytes;
    data[0] = DATAX0;
    bytes = readAdxlBytes(h, data, dataLen);
    if (bytes != dataLen) {
	success = 0;
    }
    getReading(h, x, y, z);


if (success == 0) {
    printf("Error occurred!");
    return 1;
}
   printf("X,Y,Z %d %d %d\n", x,y,z);

   for(int loop = 0; loop < dataLen; loop++) { 
      printf("%d ", data[loop]);
   }
    printf("Done ;) .\n");
    return 0;
    teardownAdxl(); 
}
