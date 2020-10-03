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

int main() {
    // SPI sensor setup
    int h = setupAdxl(spiSpeed); 
    char data[100];
    int16_t x, y, z;
    int success = 1;
    int bytes;
    data[0] = 0x01;
    bytes = readAdxlBytes(h, data, 100);
    if (bytes != 100) {
	success = 0;
    }
    teardownAdxl(); 
if (success == 0) {
    printf("Error occurred!");
    return 1;
}

   for(int loop = 0; loop < 100; loop++) { 
      printf("%d ", data[loop]);
   }
    printf("Done ;) \n");
    return 0;
}
