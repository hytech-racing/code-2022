#include <BNO055_IMU.h>

void read_data(AxisData *data, DataMode mode)
{
    uint8_t raw_data[8];
    switch (mode) {
        case GYRO:
            {
                i2c_read(I2C_ADDRESS, GYR_DATA_X_LSB, 6, raw_data);
                data->gyroAxes[0] = UNPACK(raw_data[0], raw_data[1]);
                data->gyroAxes[1] = UNPACK(raw_data[2], raw_data[3]);
                data->gyroAxes[2] = UNPACK(raw_data[4], raw_data[5]);
            }
            break;
        case ACCEL:
            {
                i2c_read(I2C_ADDRESS, LIA_DATA_X_LSB, 6, raw_data);
                data->accelAxes[0] = UNPACK(raw_data[0], raw_data[1]);
                data->accelAxes[1] = UNPACK(raw_data[2], raw_data[3]);
                data->accelAxes[2] = UNPACK(raw_data[4], raw_data[5]);
            }
            break;
        case GRAVITY:
            {
                i2c_read(I2C_ADDRESS, GRV_DATA_X_LSB, 6, raw_data);
                data->gravAxes[0] = UNPACK(raw_data[0], raw_data[1]);
                data->gravAxes[1] = UNPACK(raw_data[2], raw_data[3]);
                data->gravAxes[2] = UNPACK(raw_data[4], raw_data[5]);
            }
            break;
        case EUL:
            {
                i2c_read(I2C_ADDRESS, EUL_HEADING_LSB, 6, raw_data);
                data->eulerAngles[0] = UNPACK(raw_data[0], raw_data[1]);
                data->eulerAngles[1] = UNPACK(raw_data[2], raw_data[3]);
                data->eulerAngles[2] = UNPACK(raw_data[4], raw_data[5]);
            }
            break;
        case QUAT:
            {
                i2c_read(I2C_ADDRESS, QUA_DATA_W_LSB, 8, raw_data);
                data->quaternions[0] = UNPACK(raw_data[0], raw_data[1]);
                data->quaternions[1] = UNPACK(raw_data[2], raw_data[3]);
                data->quaternions[2] = UNPACK(raw_data[4], raw_data[5]);
                data->quaternions[3] = UNPACK(raw_data[6], raw_data[7]);
            }
            break;
    }
}

void update_data(IMUData *my_data)
{
    AxisData *data;
    if (!(data = (AxisData *)malloc(sizeof(AxisData)))) {
        return;
    }

    read_data(data, ACCEL);
    for (size_t i = 0; i < 3; i++) {
        my_data->accelAxes[i] = (float) data->accelAxes[i] / 100.;
    }

    read_data(data, GYRO);
    for (size_t i = 0; i < 3; i++) {
        my_data->gyroAxes[i] = (float) data->gyroAxes[i] / 16.;
    }

    read_data(data, GRAVITY);
    for (size_t i = 0; i < 3; i++) {
        my_data->gravAxes[i] = (float) data->gravAxes[i] / 100.;
    }

    read_data(data, EUL);
    for (size_t i = 0; i < 3; i++) {
        my_data->eulerAngles[i] = (float) data->eulerAngles[i] / 16.;
    }

    Quaternion q;
    read_data(data, QUAT);
    q.x = (float) data->quaternions[0] / 16384.;
    q.y = (float) data->quaternions[1] / 16384.;
    q.z = (float) data->quaternions[2] / 16384.;
    q.w = (float) data->quaternions[3] / 16384.;

    float yaw = atan2(2.0f*(q.y*q.z + q.w*q.x), q.x*q.x + q.y*q.y - q.z*q.z - q.w*q.w);
    float pitch = -asin(2.0f*(q.y*q.w - q.x*q.z));
    float roll = atan2(2.0f*(q.x*q.y + q.z*q.w), q.x*q.x - q.y*q.y - q.z*q.z + q.w*q.w);
    
    yaw *= -180.0f / M_PI;
    pitch *= -180.0f / M_PI;
    roll *= -180.0f / M_PI;

    my_data->orientation[0] = yaw;
    my_data->orientation[1] = pitch;
    my_data->orientation[2] = roll;

    free(data);
}

void imu_init()
{
    i2c_write(I2C_ADDRESS, OPR_MODE, 0x00);
    
    i2c_write(I2C_ADDRESS, PAGE_ID, 0x01);
    i2c_write(I2C_ADDRESS, GYRO_CONFIG_0, 0x00);
    i2c_write(I2C_ADDRESS, GYRO_CONFIG_1, 0x00);
    i2c_write(I2C_ADDRESS, ACC_CONFIG, 0x00);
    
    i2c_write(I2C_ADDRESS, PAGE_ID, 0x00);

    i2c_write(I2C_ADDRESS, UNIT_SEL, 0x01);
    
    i2c_write(I2C_ADDRESS, OPR_MODE, 0x0C);
}

void i2c_write(uint8_t address, uint8_t sub_address, uint8_t data)
{
    Wire.beginTransmission(address);
    Wire.write(sub_address);
    Wire.write(data);
    Wire.endTransmission();
}

void i2c_read(uint8_t address, uint8_t sub_address, size_t bytes, uint8_t *dest)
{
    Wire.beginTransmission(address);
    Wire.write(sub_address);
    Wire.endTransmission(false);

    size_t i = 0;
    Wire.requestFrom(address, bytes);
    while (Wire.available())
        dest[i++] = Wire.read();
}