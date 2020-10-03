#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "adxl345_driver.h"

#define DATA_FORMAT_B 0x0B  // data format bytes: +/- 16g range, 13-bit resolution (p. 26 of ADXL345 datasheet)

const int timeDefault = 5;  // default duration of data stream, seconds
const int freqDefault = 5;  // default sampling rate of data stream, Hz
const int freqMax = 3200;  // maximal allowed cmdline arg sampling rate of data stream, Hz
const int spiSpeed = 2000000;  // SPI communication speed, bps
const int freqMaxSPI = 100000;  // maximal possible communication sampling rate through SPI, Hz (assumption)
const int coldStartSamples = 2;  // number of samples to be read before outputting data to console (cold start delays)
const double coldStartDelay = 0.1;  // time delay between cold start reads
const double accConversion = 2 * 16.0 / 8192.0;  // +/- 16g range, 13-bit resolution
const double tStatusReport = 1;  // time period of status report if data read to file, seconds

int main(int argc, char *argv[]) {
    // handling command-line arguments

    double vTime = timeDefault;
    double vFreq = freqDefault;
    // SPI sensor setup
    int samples = vFreq * vTime;
    int samplesMaxSPI = freqMaxSPI * vTime;
    int success = 1;
    int h, bytes;
    h = getSpi(spiSpeed); 


    char data[100];
    int16_t x, y, z;
    double tStart, tDuration, t;
    data[0] = BW_RATE;
    data[1] = 0x0F;
    writeBytes(h, data, 2);
    data[0] = DATA_FORMAT;
    data[1] = DATA_FORMAT_B;
    writeBytes(h, data, 2);
    data[0] = POWER_CTL;
    data[1] = 0x08;
    writeBytes(h, data, 2);

    double delay = 1.0 / vFreq;  // delay between reads in seconds

    // depending from the output mode (print to cmdline / save to file) data is read in different ways

    // for cmdline output, data is read directly to the screen with a sampling rate which is *approximately* equal...
    // but always less than the specified value, since reading takes some time

    data[0] = 0x01;
    bytes = readBytes(h, data, 100);
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
    printf("Done\n");
    return 0;
}
