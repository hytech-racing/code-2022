# Arduino-BNO055

A library for the BNO055 IMU for Arduino. You can find the datasheet [here](https://cdn-shop.adafruit.com/datasheets/BST_BNO055_DS000_12.pdf).

## Features
There are currently 5 supported data modes, represented with the enum `DataMode`.
* GYRO    - angular velocity in degrees / sec
* GRAVITY - gravitational acceleration in meters / sec^2
* ACCEL   - linear acceleration in meters / sec^2
* EUL     - absolute angular orientation in degrees [0, 360)
* QUAT    - absolute orientation in quaternion form [-180, 180)

There is an `AxisData` union that is used to store data retrieved from the IMU.
* `accelAxes[3]`
* `gyroAxes[3]`
* `gravAxes[3]`
* `eulerAngles[3]`
* `quaternions[4]`

This is the raw data from the IMU. For data that has been calculated and is useful right from the box,
you can use the `Data` struct. There are 5 float arrays stored in the `Data` struct.
* `accelAxes[3]`
* `gyroAxes[3]`
* `gravAxes[3]`
* `eulerAngles[3]`
* `orientation[3]`
The orientation values are calculated using the quaternions.

## Usage
To use the library, simply add the header file as an include. Then, in your `setup()` function, begin the `Wire` to communicate with I2C. Then, initialize the IMU with `imu_init()`.
```ino
#include <BNO055_IMU.h>

Data *data;

void setup()
{
  Wire.begin();

  imu_init(); // initializes the IMU

  if (!(data = (Data *)malloc(sizeof(Data)))) {
    return;
  }
}
```

To read data from the IMU, simply call `update_data(*data)` with a `Data` destination called `data`. An implementation of this would be the following:
```ino
void loop()
{
  if (!data) return;

  update_data(data);

  // you can then use the data
}
```