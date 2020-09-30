#include <stdio.h>
#include <pigpio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* /////////////////// ADXL345 DRIVER FUNDAMENTALS ////////////////// */
// SOURCE : https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/input/misc/adxl34x.c?id=HEAD

/* ADXL345/6 Register Map */
#define DEVID    0x00  /* R   Device ID */
#define THRESH_TAP  0x1D  /* R/W Tap threshold */
#define OFSX    0x1E  /* R/W X-axis offset */
#define OFSY    0x1F  /* R/W Y-axis offset */
#define OFSZ    0x20  /* R/W Z-axis offset */
#define DUR   0x21  /* R/W Tap duration */
#define LATENT    0x22  /* R/W Tap latency */
#define WINDOW    0x23  /* R/W Tap window */
#define THRESH_ACT  0x24  /* R/W Activity threshold */
#define THRESH_INACT  0x25  /* R/W Inactivity threshold */
#define TIME_INACT  0x26  /* R/W Inactivity time */
#define ACT_INACT_CTL 0x27  /* R/W Axis enable control for activity and */
#define THRESH_FF 0x28  /* R/W Free-fall threshold */
#define TIME_FF   0x29  /* R/W Free-fall time */
#define TAP_AXES  0x2A  /* R/W Axis control for tap/double tap */
#define ACT_TAP_STATUS  0x2B  /* R   Source of tap/double tap */
#define BW_RATE   0x2C  /* R/W Data rate and power mode control */
#define POWER_CTL 0x2D  /* R/W Power saving features control */
#define INT_ENABLE  0x2E  /* R/W Interrupt enable control */
#define INT_MAP   0x2F  /* R/W Interrupt mapping control */
#define INT_SOURCE  0x30  /* R   Source of interrupts */
#define DATA_FORMAT 0x31  /* R/W Data format control */
#define DATAX0    0x32  /* R   X-Axis Data 0 */
#define DATAX1    0x33  /* R   X-Axis Data 1 */
#define DATAY0    0x34  /* R   Y-Axis Data 0 */
#define DATAY1    0x35  /* R   Y-Axis Data 1 */
#define DATAZ0    0x36  /* R   Z-Axis Data 0 */
#define DATAZ1    0x37  /* R   Z-Axis Data 1 */
#define FIFO_CTL  0x38  /* R/W FIFO control */
#define FIFO_STATUS 0x39  /* R   FIFO status */
#define TAP_SIGN  0x3A  /* R   Sign and source for tap/double tap 

/* DEVIDs */
#define ID_ADXL345  0xE5

/* INT_ENABLE/INT_MAP/INT_SOURCE Bits */
#define DATA_READY  (1 << 7)
#define SINGLE_TAP  (1 << 6)
#define DOUBLE_TAP  (1 << 5)
#define ACTIVITY  (1 << 4)
#define INACTIVITY  (1 << 3)
#define FREE_FALL (1 << 2)
#define WATERMARK (1 << 1)
#define OVERRUN   (1 << 0)

/* ACT_INACT_CONTROL Bits */
#define ACT_ACDC  (1 << 7)
#define ACT_X_EN  (1 << 6)
#define ACT_Y_EN  (1 << 5)
#define ACT_Z_EN  (1 << 4)
#define INACT_ACDC  (1 << 3)
#define INACT_X_EN  (1 << 2)
#define INACT_Y_EN  (1 << 1)
#define INACT_Z_EN  (1 << 0)

/* TAP_AXES Bits */
#define SUPPRESS  (1 << 3)
#define TAP_X_EN  (1 << 2)
#define TAP_Y_EN  (1 << 1)
#define TAP_Z_EN  (1 << 0)

/* ACT_TAP_STATUS Bits */
#define ACT_X_SRC (1 << 6)
#define ACT_Y_SRC (1 << 5)
#define ACT_Z_SRC (1 << 4)
#define ASLEEP    (1 << 3)
#define TAP_X_SRC (1 << 2)
#define TAP_Y_SRC (1 << 1)
#define TAP_Z_SRC (1 << 0)

/* BW_RATE Bits */
#define LOW_POWER (1 << 4)
#define RATE_0_10_HZ 0x00
#define RATE_0_20_HZ 0x01
#define RATE_0_39_HZ 0x02
#define RATE_0_78_HZ 0x03
#define RATE_1_56_HZ 0x04
#define RATE_3_13_HZ 0x05
#define RATE_6_25HZ  0x06
#define RATE_12_5_HZ 0x07
#define RATE_25_HZ   0x08
#define RATE_50_HZ   0x09
#define RATE_100_HZ  0x0A # (default)
#define RATE_200_HZ  0x0B
#define RATE_400_HZ  0x0C
#define RATE_800_HZ  0x0D
#define RATE_1600_HZ 0x0E
#define RATE_3200_HZ 0x0F

/* POWER_CTL Bits */
#define PCTL_LINK (1 << 5)
#define PCTL_AUTO_SLEEP (1 << 4)
#define PCTL_MEASURE  (1 << 3)
#define PCTL_SLEEP  (1 << 2)
#define PCTL_WAKEUP(x)  ((x) & 0x3)

/* DATA_FORMAT Bits */
#define SELF_TEST (1 << 7)
#define SPI_   (1 << 6)
#define INT_INVERT  (1 << 5)
#define FULL_RES  (1 << 3)
#define JUSTIFY   (1 << 2)
#define RANGE_PM_2g 0
#define RANGE_PM_4g 1
#define RANGE_PM_8g 2
#define RANGE_PM_16g  3

/*
   Maximum value our axis may get in full res mode for the input device
   (signed 13 bits)
*/
#define ADXL_FULLRES_MAX_VAL 4096

/*
   Maximum value our axis may get in fixed res mode for the input device
   (signed 10 bits)
*/
#define ADXL_FIXEDRES_MAX_VAL 512

/* FIFO_CTL Bits */
#define FIFO_MODE(x)  (((x) & 0x3) << 6)
#define FIFO_BYPASS 0
#define FIFO_FIFO 1
#define FIFO_STREAM 2
#define FIFO_TRIGGER  3
#define TRIGGER   (1 << 5)
#define SAMPLES(x)  ((x) & 0x1F)

/* FIFO_STATUS Bits */
#define FIFO_TRIG (1 << 7)
#define ENTRIES(x)  ((x) & 0x3F)

#undef ADXL_DEBUG

#define ADXL_X_AXIS     0
#define ADXL_Y_AXIS     1
#define ADXL_Z_AXIS     2


/* /////////////////// ADXL345 DRIVER FUNDAMENTALS ////////////////// */

const char codeVersion[3] = "0.2";  // code version number
const int timeDefault = 5;  // default duration of data stream, seconds
const int freqDefault = 5;  // default sampling rate of data stream, Hz
const int freqMax = 3200;  // maximal allowed cmdline arg sampling rate of data stream, Hz
const int speedSPI = 2000000;  // SPI communication speed, bps
const int freqMaxSPI = 100000;  // maximal possible communication sampling rate through SPI, Hz (assumption)
const int coldStartSamples = 2;  // number of samples to be read before outputting data to console (cold start delays)
const double coldStartDelay = 0.1;  // time delay between cold start reads
const double accConversion = 2 * 16.0 / 8192.0;  // +/- 16g range, 13-bit resolution
const double tStatusReport = 1;  // time period of status report if data read to file, seconds

void printUsage() {
    printf( "adxl345spi (version %s) \n"
            "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
            "\n"
            "Usage: adxl345spi [OPTION]... \n"
            "Read data from ADXL345 accelerometer through SPI interface on Raspberry Pi.\n"
            "Online help, docs & bug reports: <https://github.com/nagimov/adxl345spi/>\n"
            "\n"
            "Mandatory arguments to long options are mandatory for short options too.\n"
            "  -s, --save FILE     save data to specified FILE (data printed to command-line\n"
            "                      output, if not specified)\n"
            "  -t, --time TIME     set the duration of data stream to TIME seconds\n"
            "                      (default: %i seconds) [integer]\n"
            "  -f, --freq FREQ     set the sampling rate of data stream to FREQ samples per\n"
            "                      second, 1 <= FREQ <= %i (default: %i Hz) [integer]\n"
            "\n"
            "Data is streamed in comma separated format, e. g.:\n"
            "  time,     x,     y,     z\n"
            "   0.0,  10.0,   0.0, -10.0\n"
            "   1.0,   5.0,  -5.0,  10.0\n"
            "   ...,   ...,   ...,   ...\n"
            "  time shows seconds elapsed since the first reading;\n"
            "  x, y and z show acceleration along x, y and z axis in fractions of <g>.\n"
            "\n"
            "Exit status:\n"
            "  0  if OK\n"
            "  1  if error occurred during data reading or wrong cmdline arguments.\n"
            "", codeVersion, timeDefault, freqMax, freqDefault);
}

int readBytes(int handle, char *data, int count) {
    data[0] |= READ_BIT;
    if (count > 1) data[0] |= MULTI_BIT;
    return spiXfer(handle, data, data, count);
}

int writeBytes(int handle, char *data, int count) {
    if (count > 1) data[0] |= MULTI_BIT;
    return spiWrite(handle, data, count);
}

int main(int argc, char *argv[]) {
    int i;

    // handling command-line arguments

    int bSave = 0;
    char vSave[256] = "";
    double vTime = timeDefault;
    double vFreq = freqDefault;
    for (i = 1; i < argc; i++) {  // skip argv[0] (program name)
        if ((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--save") == 0)) {
            bSave = 1;
            if (i + 1 <= argc - 1) {  // make sure there are enough arguments in argv
                i++;
                strcpy(vSave, argv[i]);
            }
            else {
                printUsage();
                return 1;
            }
        }
        else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "--time") == 0)) {
            if (i + 1 <= argc - 1) {
                i++;
                vTime = atoi(argv[i]);
            }
            else {
                printUsage();
                return 1;
            }
        }
        else if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--freq") == 0)) {
            if (i + 1 <= argc - 1) {
                i++;
                vFreq = atoi(argv[i]);
                if ((vFreq < 1) || (vFreq > 3200)) {
                    printf("Wrong sampling rate specified!\n\n");
                    printUsage();
                    return 1;
                }
            }
            else {
                printUsage();
                return 1;
            }
        }
        else {
            printUsage();
            return 1;
        }
    }

    // reading sensor data

    // SPI sensor setup
    int samples = vFreq * vTime;
    int samplesMaxSPI = freqMaxSPI * vTime;
    int success = 1;
    int h, bytes;
    char data[7];
    int16_t x, y, z;
    double tStart, tDuration, t;
    if (gpioInitialise() < 0) {
        printf("Failed to initialize GPIO!");
        return 1;
    }
    h = spiOpen(0, speedSPI, 3);
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

    if (bSave == 0) {
        // fake reads to eliminate cold start timing issues (~0.01 s shift of sampling time after the first reading)
        for (i = 0; i < coldStartSamples; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes != 7) {
                success = 0;
            }
            time_sleep(coldStartDelay);
        }
        // real reads happen here
        tStart = time_time();
        for (i = 0; i < samples; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes == 7) {
                x = (data[2]<<8)|data[1];
                y = (data[4]<<8)|data[3];
                z = (data[6]<<8)|data[5];
                t = time_time() - tStart;
                printf("time = %.3f, x = %.3f, y = %.3f, z = %.3f\n",
                       t, x * accConversion, y * accConversion, z * accConversion);
                }
            else {
                success = 0;
            }
            time_sleep(delay);  // pigpio sleep is accurate enough for console output, not necessary to use nanosleep
        }
        gpioTerminate();
        tDuration = time_time() - tStart;  // need to update current time to give a closer estimate of sampling rate
        printf("%d samples read in %.2f seconds with sampling rate %.1f Hz\n", samples, tDuration, samples/tDuration);
        if (success == 0) {
            printf("Error occurred!");
            return 1;
        }
    }

    // for the file output, data is read with a maximal possible sampling rate (around 30,000 Hz)...
    // and then accurately rescaled to *exactly* match the specified sampling rate...
    // therefore, saved data can be easily analyzed (e. g. with fft)
    else {
        // reserve vectors for file-output arrays: time, x, y, z
        // arrays will not change their lengths, so separate track of the size is not needed
        double *at, *ax, *ay, *az;
        at = malloc(samples * sizeof(double));
        ax = malloc(samples * sizeof(double));
        ay = malloc(samples * sizeof(double));
        az = malloc(samples * sizeof(double));

        // reserve vectors for raw data: time, x, y, z
        // maximal achievable sampling rate depends from the hardware...
        // in my case, for Raspberry Pi 3 at 2 Mbps SPI bus speed sampling rate never exceeded ~30,000 Hz...
        // so to be sure that there is always enough memory allocated, freqMaxSPI is set to 60,000 Hz
        double *rt, *rx, *ry, *rz;
        rt = malloc(samplesMaxSPI * sizeof(double));
        rx = malloc(samplesMaxSPI * sizeof(double));
        ry = malloc(samplesMaxSPI * sizeof(double));
        rz = malloc(samplesMaxSPI * sizeof(double));

        printf("Reading %d samples in %.1f seconds with sampling rate %.1f Hz...\n", samples, vTime, vFreq);
        int statusReportedTimes = 0;
        double tCurrent, tClosest, tError, tErrorPrev, tLeft;
        int j, jClosest;

        tStart = time_time();
        int samplesRead;
        for (i = 0; i < samplesMaxSPI; i++) {
            data[0] = DATAX0;
            bytes = readBytes(h, data, 7);
            if (bytes == 7) {
                x = (data[2]<<8)|data[1];
                y = (data[4]<<8)|data[3];
                z = (data[6]<<8)|data[5];
                t = time_time();
                rx[i] = x * accConversion;
                ry[i] = y * accConversion;
                rz[i] = z * accConversion;
                rt[i] = t - tStart;
            }
            else {
                gpioTerminate();
                printf("Error occurred!");
                return 1;
            }
            tDuration = t - tStart;
            if (tDuration > tStatusReport * ((float)statusReportedTimes + 1.0)) {
                statusReportedTimes++;
                tLeft = vTime - tDuration;
                if (tLeft < 0) {
                    tLeft = 0.0;
                }
                printf("%.2f seconds left...\n", tLeft);
            }
            if (tDuration > vTime) {  // enough data read
                samplesRead = i;
                break;
            }
        }
        gpioTerminate();
        printf("Writing to the output file...\n");
        for (i = 0; i < samples; i++) {
            if (i == 0) {  // always get the first reading from position 0
                tCurrent = 0.0;
                jClosest = 0;
                tClosest = rt[jClosest];
            }
            else {
                tCurrent = (float)i * delay;
                tError = fabs(tClosest - tCurrent);
                tErrorPrev = tError;
                for (j = jClosest; j < samplesRead; j++) {  // lookup starting from previous j value
                    if (fabs(rt[j] - tCurrent) < tError) {  // in order to save some iterations
                        jClosest = j;
                        tClosest = rt[jClosest];
                        tErrorPrev = tError;
                        tError = fabs(tClosest - tCurrent);
                    }
                    else {
	                    if (tError > tErrorPrev) {  // if the error starts growing
    	                	break;                  // break the loop since the minimal error point passed
	                    }
                    }
                }  // when this loop is ended, jClosest and tClosest keep coordinates of the closest (j, t) point
            }
            ax[i] = rx[jClosest];
            ay[i] = ry[jClosest];
            az[i] = rz[jClosest];
            at[i] = tCurrent;
        }
        FILE *f;
        f = fopen(vSave, "w");
        fprintf(f, "time, x, y, z\n");
        for (i = 0; i < samples; i++) {
            fprintf(f, "%.5f, %.5f, %.5f, %.5f \n", at[i], ax[i], ay[i], az[i]);
        }
        fclose(f);
    }

    printf("Done\n");
    return 0;
}
