#include <BNO055_IMU.h>
#include <math.h>

typedef struct quaternion {
  float x;
  float y;
  float z;
  float w;
} Quaternion;

float yaw, pitch, roll;
Quaternion q;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  
  delay(4000);
  
  Serial.begin(9600);
  
  imu_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  AxisData *data;
  if (!(data = (AxisData *)malloc(sizeof(AxisData)))) {
    return;
  }

  read_data(data, EUL);

  Serial.println("Euler data");
  Serial.print("yaw:\t");
  Serial.println((double) data->eulerAngles[0] / 16);
  Serial.print("pitch:\t");
  Serial.println((double) data->eulerAngles[1] / 16);
  Serial.print("roll:\t");
  Serial.println((double) data->eulerAngles[2] / 16);
  Serial.println();

  read_data(data, QUAT);

  q.x = (float)(data->quaternions[0]) / 16384.;
  q.y = (float)(data->quaternions[1]) / 16384.;
  q.z = (float)(data->quaternions[2]) / 16384.;
  q.w = (float)(data->quaternions[3]) / 16384.;
  
  yaw = atan2(2.0f*(q.y*q.z + q.w*q.x), q.x*q.x + q.y*q.y - q.z*q.z - q.w*q.w);
  pitch = -asin(2.0f*(q.y*q.w - q.x*q.z));
  roll = atan2(2.0f*(q.x*q.y + q.z*q.w), q.x*q.x - q.y*q.y - q.z*q.z + q.w*q.w);

  yaw *= 180.0f / PI;
  pitch *= 180.0f / PI;
  roll *= 180.0f / PI;

  Serial.println("Quaternion data");
  Serial.print("yaw:\t");
  Serial.println(yaw);
  Serial.print("pitch:\t");
  Serial.println(pitch);
  Serial.print("roll:\t");
  Serial.println(roll);
  Serial.println();

  free(data);
  delay(1000);
}