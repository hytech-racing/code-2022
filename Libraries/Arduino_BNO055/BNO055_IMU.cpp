#include <BNO055_IMU.h>

void read_data(AxisData *data, DataMode mode)
{
    switch (mode) {
        case GYRO:
            {
                uint8_t raw_data[6];
                i2c_read(I2C_ADDRESS, GYR_DATA_X_LSB, 6, raw_data);
                data->gyroAxes[0] = ((int16_t)raw_data[1] << 8) | raw_data[0];
                data->gyroAxes[1] = ((int16_t)raw_data[3] << 8) | raw_data[2];
                data->gyroAxes[2] = ((int16_t)raw_data[5] << 8) | raw_data[4];
            }
            break;
        case ACCEL:
            {
                uint8_t raw_data[6];
                i2c_read(I2C_ADDRESS, LIA_DATA_X_LSB, 6, raw_data);
                data->accelAxes[0] = ((int16_t)raw_data[1] << 8) | raw_data[0];
                data->accelAxes[1] = ((int16_t)raw_data[3] << 8) | raw_data[2];
                data->accelAxes[2] = ((int16_t)raw_data[5] << 8) | raw_data[4];
            }
            break;
        case GRAVITY:
            {
                uint8_t raw_data[6];
                i2c_read(I2C_ADDRESS, GRV_DATA_X_LSB, 6, raw_data);
                data->gravAxes[0] = ((int16_t)raw_data[1] << 8) | raw_data[0];
                data->gravAxes[1] = ((int16_t)raw_data[3] << 8) | raw_data[2];
                data->gravAxes[2] = ((int16_t)raw_data[5] << 8) | raw_data[4];
            }
            break;
        case EUL:
            {
                uint8_t raw_data[6];
                i2c_read(I2C_ADDRESS, EUL_HEADING_LSB, 6, raw_data);
                data->eulerAngles[0] = ((int16_t)raw_data[1] << 8) | raw_data[0];
                data->eulerAngles[1] = ((int16_t)raw_data[3] << 8) | raw_data[2];
                data->eulerAngles[2] = ((int16_t)raw_data[5] << 8) | raw_data[4];
            }
            break;
        case QUAT:
            {
                uint8_t raw_data[8];
                i2c_read(I2C_ADDRESS, QUA_DATA_W_LSB, 8, raw_data);
                data->quaternions[0] = ((int16_t)raw_data[1] << 8) | raw_data[0];
                data->quaternions[1] = ((int16_t)raw_data[3] << 8) | raw_data[2];
                data->quaternions[2] = ((int16_t)raw_data[5] << 8) | raw_data[4];
                data->quaternions[3] = ((int16_t)raw_data[7] << 8) | raw_data[6];
            }
            break;
    }
}

void update_data(Data *my_data)
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