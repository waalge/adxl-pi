#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "adxl345_driver.h"

const int timeDefault = 5;  // default duration of data stream, seconds
const int freqDefault = 5;  // default sampling rate of data stream, Hz
const int spiSpeed = 2000000;  // SPI communication speed, bps
const int spiMaxFreq = 100000;  // maximal possible communication sampling rate through SPI, Hz (assumption)

int setupAdxl(spiSpeed) {
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

int main(int argc, char *argv[]) {
    // handling command-line arguments

    double vTime = timeDefault;
    double vFreq = freqDefault;
    // SPI sensor setup
    int samples = vFreq * vTime;
    int samplesMaxSPI = spiMaxFreq * vTime;
    int success = 1;
    int h, bytes;
    h = setupAdxl(spiSpeed); 


    char data[100];
    int16_t x, y, z;
    double tStart, tDuration, t;
    data[0] = BW_RATE;
    data[1] = RATE_800_HZ;
    writeAdxlBytes(h, data, 2);
    data[0] = DATA_FORMAT;
    data[1] = RANGE_PM_2g;
    writeAdxlBytes(h, data, 2);
    data[0] = POWER_CTL;
    data[1] = PCTL_MEASURE;
    writeAdxlBytes(h, data, 2);

    double delay = 1.0 / vFreq;  // delay between reads in seconds

    // depending from the output mode (print to cmdline / save to file) data is read in different ways

    // for cmdline output, data is read directly to the screen with a sampling rate which is *approximately* equal...
    // but always less than the specified value, since reading takes some time

    data[0] = 0x01;
    bytes = readAdxlBytes(h, data, 100);
    if (bytes != 100) {
	success = 0;
    }
    gpioTerminate();
    tDuration = time_time() - tStart;  // need to update current time to give a closer estimate of sampling rate
    printf("%d samples read in %.2f seconds with sampling rate %.1f Hz\n", samples, tDuration, samples/tDuration);
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
