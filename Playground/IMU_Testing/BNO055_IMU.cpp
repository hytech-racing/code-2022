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
                i2c_read(I2C_ADDRESS, ACC_DATA_X_LSB, 6, raw_data);
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

// TODO: if needed, customize the init data bytes
void imu_init()
{
    i2c_write(I2C_ADDRESS, OPR_MODE, 0x00);
    
    i2c_write(I2C_ADDRESS, PAGE_ID, 0x01);
    i2c_write(I2C_ADDRESS, GYRO_CONFIG_0, 0x00);
    i2c_write(I2C_ADDRESS, GYRO_CONFIG_1, 0x00);
    i2c_write(I2C_ADDRESS, ACC_CONFIG, 0x00);
    
    i2c_write(I2C_ADDRESS, PAGE_ID, 0x00);
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