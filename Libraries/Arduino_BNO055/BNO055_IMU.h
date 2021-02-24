#include <Wire.h>
#include <stdlib.h>
#include <math.h>

#ifndef BNO055_IMU_H
#define BNO055_IMU_H

#define I2C_ADDRESS         0x28

// REGISTERS
// as per https://cdn-shop.adafruit.com/datasheets/BST_DS000_12.pdf

// page 0
#define CHIP_ID             0x00
#define ACC_ID              0x01
#define MAG_ID              0x02
#define GYR_ID              0x03
#define SW_REV_ID_LSB       0x04                                                                          
#define SW_REV_ID_MSB       0x05
#define BL_REV_ID           0x06
#define PAGE_ID             0x07
#define ACC_DATA_X_LSB      0x08
#define ACC_DATA_X_MSB      0x09
#define ACC_DATA_Y_LSB      0x0A
#define ACC_DATA_Y_MSB      0x0B
#define ACC_DATA_Z_LSB      0x0C
#define ACC_DATA_Z_MSB      0x0D
#define MAG_DATA_X_LSB      0x0E
#define MAG_DATA_X_MSB      0x0F
#define MAG_DATA_Y_LSB      0x10
#define MAG_DATA_Y_MSB      0x11
#define MAG_DATA_Z_LSB      0x12
#define MAG_DATA_Z_MSB      0x13
#define GYR_DATA_X_LSB      0x14
#define GYR_DATA_X_MSB      0x15
#define GYR_DATA_Y_LSB      0x16
#define GYR_DATA_Y_MSB      0x17
#define GYR_DATA_Z_LSB      0x18
#define GYR_DATA_Z_MSB      0x19
#define EUL_HEADING_LSB     0x1A
#define EUL_HEADING_MSB     0x1B
#define EUL_ROLL_LSB        0x1C
#define EUL_ROLL_MSB        0x1D
#define EUL_PITCH_LSB       0x1E
#define EUL_PITCH_MSB       0x1F
#define QUA_DATA_W_LSB      0x20
#define QUA_DATA_W_MSB      0x21
#define QUA_DATA_X_LSB      0x22
#define QUA_DATA_X_MSB      0x23
#define QUA_DATA_Y_LSB      0x24
#define QUA_DATA_Y_MSB      0x25
#define QUA_DATA_Z_LSB      0x26
#define QUA_DATA_Z_MSB      0x27
#define LIA_DATA_X_LSB      0x28
#define LIA_DATA_X_MSB      0x29
#define LIA_DATA_Y_LSB      0x2A
#define LIA_DATA_Y_MSB      0x2B
#define LIA_DATA_Z_LSB      0x2C
#define LIA_DATA_Z_MSB      0x2D
#define GRV_DATA_X_LSB      0x2E
#define GRV_DATA_X_MSB      0x2F
#define GRV_DATA_Y_LSB      0x30
#define GRV_DATA_Y_MSB      0x31
#define GRV_DATA_Z_LSB      0x32
#define GRV_DATA_Z_MSB      0x33
#define TEMP                0x34
#define CALIB_STAT          0x35
#define ST_RESULT           0x36
#define INT_STATUS          0x37
#define SYS_CLK_STATUS      0x38
#define SYS_STATUS          0x39
#define SYS_ERR             0x3A
#define UNIT_SEL            0x3B
#define OPR_MODE            0x3D
#define PWR_MODE            0x3E
#define SYS_TRIGGER         0x3F
#define TEMP_SOURCE         0x40
#define AXIS_MAP_CONFIG     0x41
#define AXIS_MAP_SIGN       0x42
#define ACC_OFFSET_X_LSB    0x55
#define ACC_OFFSET_X_MSB    0x56
#define ACC_OFFSET_Y_LSB    0x57
#define ACC_OFFSET_Y_MSB    0x58
#define ACC_OFFSET_Z_LSB    0x59
#define ACC_OFFSET_Z_MSB    0x5A
#define MAG_OFFSET_X_LSB    0x5B
#define MAG_OFFSET_X_MSB    0x5C
#define MAG_OFFSET_Y_LSB    0x5D
#define MAG_OFFSET_Y_MSB    0x5E
#define MAG_OFFSET_Z_LSB    0x5F
#define MAG_OFFSET_Z_MSB    0x60
#define GYR_OFFSET_X_LSB    0x61
#define GYR_OFFSET_X_MSB    0x62
#define GYR_OFFSET_Y_LSB    0x63
#define GYR_OFFSET_Y_MSB    0x64
#define GYR_OFFSET_Z_LSB    0x65
#define GYR_OFFSET_Z_MSB    0x66
#define ACC_RADIUS_LSB      0x67
#define ACC_RADIUS_MSB      0x68
#define MAG_RADIUS_LSB      0x69
#define MAG_RADIUS_MSB      0x6A

// page 1
#define PAGE_ID             0x07
#define ACC_CONFIG          0x08
#define MAG_CONFIG          0x09
#define GYRO_CONFIG_0       0x0A
#define GYRO_CONFIG_1       0x0B
#define ACC_SLEEP_CONFIG    0x0C
#define GYR_SLEEP_CONFIG    0x0D
#define INT_MSK             0x0F
#define INT_EN              0x10
#define ACC_AM_THRES        0x11
#define ACC_INT_SETTINGS    0x12
#define ACC_HG_DURATION     0x13
#define ACC_HG_THRESH       0x14
#define ACC_NM_THRESH       0x15
#define ACC_NM_SET          0x16
#define GYR_INT_SETTINGS    0x17
#define GYR_HR_X_SET        0x18
#define GYR_DUR_X           0x19
#define GYR_HR_Y_SET        0x1A
#define GYR_DUR_Y           0x1B
#define GYR_HR_Z_SET        0x1C
#define GYR_DUR_Z           0x1D
#define GYR_AM_THRESH       0x1E
#define GYR_AM_SET          0x1F

typedef enum {
    GYRO, GRAVITY, ACCEL, EUL, QUAT
} DataMode;

typedef union axes {
    int16_t accelAxes[3];   // accelerometer sensor output
    int16_t gyroAxes[3];    // gyro sensor output
    int16_t gravAxes[3];    // gravity vector output
    int16_t eulerAngles[3]; // euler angles
    int16_t quaternions[4]; // qauaternion
} AxisData;

// a struct of useful data
typedef struct data {
    float accelAxes[3];
    float gyroAxes[3];
    float gravAxes[3];
    float eulerAngles[3];
    float orientation[3];
} Data;

// quaternion
typedef struct quaternion {
  float x;
  float y;
  float z;
  float w;
} Quaternion;

/* Function definitions */

// read data from registers
void read_data(AxisData *data, DataMode mode);

void update_data(Data *my_data);

// initialize the IMU
void imu_init();

// I2C write/read functions
void i2c_write(uint8_t address, uint8_t sub_address, uint8_t data);
void i2c_read(uint8_t address, uint8_t sub_address, size_t bytes, uint8_t *dest);

#endif