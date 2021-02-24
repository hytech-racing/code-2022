#include <BNO055_IMU.h>

Data *data;

void setup()
{   
    Wire.begin();

    delay(4000);

    Serial.begin(9600);

    imu_init();
    if (!(data = (Data *)malloc(sizeof(Data)))) {
      return;
    }
}

void loop()
{
    if (!data) return;

    update_data(data);
  
    Serial.println("Accel data");
    Serial.print("x:\t");
    Serial.println(data->accelAxes[0]);
    Serial.print("y:\t");
    Serial.println(data->accelAxes[1]);
    Serial.print("z:\t");
    Serial.println(data->accelAxes[2]);
    Serial.println("---------------------");

    Serial.println("Gyro data");
    Serial.print("x:\t");
    Serial.println(data->gyroAxes[0]);
    Serial.print("y:\t");
    Serial.println(data->gyroAxes[1]);
    Serial.print("z:\t");
    Serial.println(data->gyroAxes[2]);
    Serial.println("---------------------");

    Serial.println("Euler data");
    Serial.print("yaw:\t");
    Serial.println(data->eulerAngles[0]);
    Serial.print("pitch:\t");
    Serial.println(data->eulerAngles[1]);
    Serial.print("roll:\t");
    Serial.println(data->eulerAngles[2]);
    Serial.println("---------------------");

    Serial.println("Gravity data");
    Serial.print("x:\t");
    Serial.println(data->gravAxes[0]);
    Serial.print("y:\t");
    Serial.println(data->gravAxes[1]);
    Serial.print("z:\t");
    Serial.println("---------------------");

    Serial.println("Quaternion data");
    Serial.print("yaw:\t");
    Serial.println(data->orientation[0]);
    Serial.print("pitch:\t");
    Serial.println(data->orientation[1]);
    Serial.print("roll:\t");
    Serial.println(data->orientation[2]);
    Serial.println();
    
    Serial.flush();
    delay(10);
}