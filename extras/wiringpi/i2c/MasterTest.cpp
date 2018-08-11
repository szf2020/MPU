/* 07/6/2017 Copyright Tlera Corporation
 *  
 * Created by Kris Winer
 *
 * Adapted for WiringPi by Simon D. Levy April 2018
 *  
 * Demonstrate basic MPU-9250 functionality in master mode including
 * parameterizing the register addresses, initializing the sensor, getting
 * properly scaled accelerometer, gyroscope, and magnetometer data out. 
 *
 * SDA and SCL have 4K7 pull-up resistors (to 3.3V).
 *
 * Library may be used freely and without limit with attribution.
 */

#include <stdio.h>
#include <MPU9250.h>
#include <wiringPi.h>

/*
   MPU9250 Configuration

   Specify sensor full scale

   Choices are:

Gscale: GFS_250 = 250 dps, GFS_500 = 500 dps, GFS_1000 = 1000 dps, GFS_2000DPS = 2000 degrees per second gyro full scale
Ascale: AFS_2G = 2 g, AFS_4G = 4 g, AFS_8G = 8 g, and AFS_16G = 16 g accelerometer full scale
Mscale: MFS_14BITS = 0.6 mG per LSB and MFS_16BITS = 0.15 mG per LSB
Mmode:  Mmode = M_8Hz for 8 Hz data rate or Mmode = M_100Hz for 100 Hz data rate
sampleRate: (1 + sampleRate) is a simple divisor of the fundamental 1000 kHz rate of the gyro and accel, so 
sampleRate = 0x00 means 1 kHz sample rate for both accel and gyro, 0x04 means 200 Hz, etc.
 */

static const Gscale_t GSCALE    = GFS_250DPS;
static const Ascale_t ASCALE    = AFS_2G;
static const Mscale_t MSCALE    = MFS_16BITS;
static const Mmode_t  MMODE     = M_100Hz;
static const uint8_t SAMPLE_RATE_DIVISOR = 0x04;         

// Pin definitions
static const uint8_t intPin = 0;   //  MPU9250 interrupt

// Interrupt support 
static bool gotNewData;
static void myinthandler()
{
    gotNewData = true;
}

// Instantiate MPU9250 class in master mode
static MPU9250_Master imu(ASCALE, GSCALE, MSCALE, MMODE, SAMPLE_RATE_DIVISOR);

void setup()
{
    void error(const char * errmsg);

    // Setup WirinPi
    wiringPiSetup();

    // Start the MPU9250
    switch (imu.begin()) {

        case MPU_ERROR_IMU_ID:
            error("Bad IMU device ID");
        case MPU_ERROR_MAG_ID:
            error("Bad magnetometer device ID");
        case MPU_ERROR_SELFTEST:
            error("Failed self-test");
        default:
            printf("MPU6050 online!\n");
    }

    wiringPiISR(intPin, INT_EDGE_RISING, &myinthandler);// define interrupt for intPin output of MPU9250
}

void loop()
{  
    static float ax, ay, az, gx, gy, gz, mx, my, mz, temperature;

    // If INTERRUPT_PIN goes high, either all data registers have new data
    // or the accel wake on motion threshold has been crossed
    if(true) { //gotNewData) {   // On interrupt, read data

        gotNewData = false;     // reset gotNewData flag

        if (imu.checkNewData())  { // data ready interrupt is detected

            imu.readAccelerometer(ax, ay, az);
            imu.readGyrometer(gx, gy, gz);
            imu.readMagnetometer(mx, my, mz);
            temperature = imu.readTemperature();
        }
    }

    // Report at 4 Hz
    static uint32_t msec_prev;
    uint32_t msec_curr = millis();

    if (msec_curr-msec_prev > 250) {

        msec_prev = msec_curr;

        printf("ax = %d  ay = %d  az = %d mg\n", (int)(1000*ax), (int)(1000*ay), (int)(1000*az));
        printf("gx = %+2.2f  gy = %+2.2f  gz = %+2.2f deg/s\n", gx, gy, gz);
        printf("mx = %d  my = %d  mz = %d mG\n", (int)mx, (int)my, (int)mz);

        float temperature = ((float) MPU9250Data[3]) / 333.87f + 21.0f; // Gyro chip temperature in degrees Centigrade

        // Print temperature in degrees Centigrade      
        printf("Gyro temperature is %+1.1f degrees C\n", temperature);  
    }

    } // if got new data
}
