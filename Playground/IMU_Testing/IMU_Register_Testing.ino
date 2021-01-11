#include <BNO055_IMU.h>

void setup()
{   
    Wire.begin();

    delay(4000);

    Serial.begin(9600);

    imu_init();
}

void loop()
{
    AxisData *data;
    if (!(data = (AxisData *)malloc(sizeof(AxisData)))) {
        return;
    }

    read_data(data, ACCEL);

    Serial.println("Accel data");
    Serial.print("x:\t");
    Serial.println((double) data->accelAxes[0]);
    Serial.print("y:\t");
    Serial.println((double) data->accelAxes[1]);
    Serial.print("z:\t");
    Serial.println((double) data->accelAxes[2]);
    Serial.println("---------------------");

    read_data(data, GYRO);

    Serial.println("Gyro data");
    Serial.print("x:\t");
    Serial.println((double) data->gyroAxes[0] / 16);
    Serial.print("y:\t");
    Serial.println((double) data->gyroAxes[1] / 16);
    Serial.print("z:\t");
    Serial.println((double) data->gyroAxes[2] / 16);
    Serial.println("---------------------");

    read_data(data, GRAVITY);

    Serial.println("Gravity data");
    Serial.print("x:\t");
    Serial.println((double) data->gravAxes[0]);
    Serial.print("y:\t");
    Serial.println((double) data->gravAxes[1]);
    Serial.print("z:\t");
    Serial.println((double) data->gravAxes[2]);
    Serial.println("---------------------");

    read_data(data, EUL);

    Serial.println("Euler data");
    Serial.print("yaw:\t");
    Serial.println((double) data->eulerAngles[0] / 16);
    Serial.print("pitch:\t");
    Serial.println((double) data->eulerAngles[1] / 16);
    Serial.print("roll:\t");
    Serial.println((double) data->eulerAngles[2] / 16);
    Serial.println();
    

    free(data);
    Serial.flush();
    delay(500);
}