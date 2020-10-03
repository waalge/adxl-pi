#include <pigpio.h>
#include <stdio.h>
/* /////////////////// ADXL345 DRIVER FUNDAMENTALS ////////////////// */
// SOURCE :
// https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/input/misc/adxl34x.c?id=HEAD

/* ADXL345/6 Register Map */
#define DEVID 0x00          /* R   Device ID */
#define THRESH_TAP 0x1D     /* R/W Tap threshold */
#define OFSX 0x1E           /* R/W X-axis offset */
#define OFSY 0x1F           /* R/W Y-axis offset */
#define OFSZ 0x20           /* R/W Z-axis offset */
#define DUR 0x21            /* R/W Tap duration */
#define LATENT 0x22         /* R/W Tap latency */
#define WINDOW 0x23         /* R/W Tap window */
#define THRESH_ACT 0x24     /* R/W Activity threshold */
#define THRESH_INACT 0x25   /* R/W Inactivity threshold */
#define TIME_INACT 0x26     /* R/W Inactivity time */
#define ACT_INACT_CTL 0x27  /* R/W Axis enable control for activity and */
#define THRESH_FF 0x28      /* R/W Free-fall threshold */
#define TIME_FF 0x29        /* R/W Free-fall time */
#define TAP_AXES 0x2A       /* R/W Axis control for tap/double tap */
#define ACT_TAP_STATUS 0x2B /* R   Source of tap/double tap */
#define BW_RATE 0x2C        /* R/W Data rate and power mode control */
#define POWER_CTL 0x2D      /* R/W Power saving features control */
#define INT_ENABLE 0x2E     /* R/W Interrupt enable control */
#define INT_MAP 0x2F        /* R/W Interrupt mapping control */
#define INT_SOURCE 0x30     /* R   Source of interrupts */
#define DATA_FORMAT 0x31    /* R/W Data format control */
#define DATAX0 0x32         /* R   X-Axis Data 0 */
#define DATAX1 0x33         /* R   X-Axis Data 1 */
#define DATAY0 0x34         /* R   Y-Axis Data 0 */
#define DATAY1 0x35         /* R   Y-Axis Data 1 */
#define DATAZ0 0x36         /* R   Z-Axis Data 0 */
#define DATAZ1 0x37         /* R   Z-Axis Data 1 */
#define FIFO_CTL 0x38       /* R/W FIFO control */
#define FIFO_STATUS 0x39    /* R   FIFO status */
#define TAP_SIGN                                                               \
  0x3A /* R   Sign and source for tap/double tap                               \
                                                                               \
/* DEVIDs */
#define ID_ADXL345 0xE5

/* INT_ENABLE/INT_MAP/INT_SOURCE Bits */
#define DATA_READY (1 << 7)
#define SINGLE_TAP (1 << 6)
#define DOUBLE_TAP (1 << 5)
#define ACTIVITY (1 << 4)
#define INACTIVITY (1 << 3)
#define FREE_FALL (1 << 2)
#define WATERMARK (1 << 1)
#define OVERRUN (1 << 0)

/* ACT_INACT_CONTROL Bits */
#define ACT_ACDC (1 << 7)
#define ACT_X_EN (1 << 6)
#define ACT_Y_EN (1 << 5)
#define ACT_Z_EN (1 << 4)
#define INACT_ACDC (1 << 3)
#define INACT_X_EN (1 << 2)
#define INACT_Y_EN (1 << 1)
#define INACT_Z_EN (1 << 0)

/* TAP_AXES Bits */
#define SUPPRESS (1 << 3)
#define TAP_X_EN (1 << 2)
#define TAP_Y_EN (1 << 1)
#define TAP_Z_EN (1 << 0)

/* ACT_TAP_STATUS Bits */
#define ACT_X_SRC (1 << 6)
#define ACT_Y_SRC (1 << 5)
#define ACT_Z_SRC (1 << 4)
#define ASLEEP (1 << 3)
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
#define RATE_6_25HZ 0x06
#define RATE_12_5_HZ 0x07
#define RATE_25_HZ 0x08
#define RATE_50_HZ 0x09
#define RATE_100_HZ 0x0A #(default)
#define RATE_200_HZ 0x0B
#define RATE_400_HZ 0x0C
#define RATE_800_HZ 0x0D
#define RATE_1600_HZ 0x0E
#define RATE_3200_HZ 0x0F

/* POWER_CTL Bits */
#define PCTL_LINK (1 << 5)
#define PCTL_AUTO_SLEEP (1 << 4)
#define PCTL_MEASURE (1 << 3)
#define PCTL_SLEEP (1 << 2)
#define PCTL_WAKEUP(x) ((x)&0x3)

/* DATA_FORMAT Bits */
#define SELF_TEST (1 << 7)
#define SPI_ (1 << 6)
#define INT_INVERT (1 << 5)
#define FULL_RES (1 << 3)
#define JUSTIFY (1 << 2)
#define RANGE_PM_2g 0
#define RANGE_PM_4g 1
#define RANGE_PM_8g 2
#define RANGE_PM_16g 3

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
#define FIFO_MODE(x) (((x)&0x3) << 6)
#define FIFO_BYPASS 0
#define FIFO_FIFO 1
#define FIFO_STREAM 2
#define FIFO_TRIGGER 3
#define TRIGGER (1 << 5)
#define SAMPLES(x) ((x)&0x1F)

/* FIFO_STATUS Bits */
#define FIFO_TRIG (1 << 7)
#define ENTRIES(x) ((x)&0x3F)

#undef ADXL_DEBUG

#define ADXL_X_AXIS 0
#define ADXL_Y_AXIS 1
#define ADXL_Z_AXIS 2

/* /////////////////// ADXL345 DRIVER FUNDAMENTALS ////////////////// */

int readAdxlBytes(int handle, unsigned char *data, int count);

int writeAdxlBytes(int handle, char *data, int count);

int setAdxlRegister(int handle, char address, char dat);

int openAdxl(int spiSpeed);

void teardownAdxl();

void printArr(char* arr, int arrLen);
