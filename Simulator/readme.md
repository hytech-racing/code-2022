Hi and welcome to the HyTech Arduino Simulator

You may be wondering "Atul what the fuck have you created" and frankly you're not wrong to be asking that. Therefore I've created this "useful" (ignore the quotes) guide on how and why and what in the name of god's most unholy creations is going on here.

# Setup

**IMPORTANT**: Please run all code in a linux environment (either a Mac/Linux machine or using [Windows 10 WSL](https://www.windowscentral.com/install-windows-subsystem-linux-windows-10))

Other dependencies:
- CMake
- dos2unix (maybe)
- some form of C++ compiler

Note about dos2unix: Git encodes end-of-line characters using CRLF (`'\r\n'`). Many linux systems use LF (`'\n'`) as their line endings. As such, running a shell script in linux after pulling from git will often result in illegal characters. dox2unix is a command-line utility that converts CRLF to LF. May not be necessary on all systems, but definitely required for WSL.

# Compilation and Running Tests

If running `fresh_build` causes an illegal-character error, run `dos2unix fresh_build` to fix line endings.

To build the simulator, run

`./fresh_build <board>`

where `<board>` represents the specific microcontroller version for which test cases should be compiled. Just running `./fresh_build` will show the full list, but for reference, they are

	+-----------+-----------+-----------+-----+
	| teensy_32 | teensy_35 | teensy_40 | uno |
	+-----------+-----------+-----------+-----+

`fresh_build` parameters are case-insensitive. You should only need to `fresh_build` for the first compilation and any time that you switch board versions. After that, enter the `build` folder and run `make <target>` at any time to quickly compile the designated test case.

# Writing Test Cases

Test cases should be added to the [test](./test) subfolder. Tests are written using the [GoogleTest C++ Testing Framework](https://github.com/google/googletest/blob/master/googletest/docs/primer.md). This readme will not cover GTest in detail.

Due to the nature of Arduino, note that global variables do not reset to their default states between tests. As such, it is important to define a SetUp and TearDown method in a Testing Framework class to reset internal states.

```C++
class DashboardTesting : public ::testing::Test {
protected:
	DashboardTesting () = default;
	void SetUp() {
		extern bool is_mc_err;
		extern VariableLED variable_led_start;
		extern Metro timer_can_update;
		extern MCP_CAN CAN;
		extern Dashboard_status dashboard_status;
		extern MCU_status mcu_status;

		is_mc_err = false;
		variable_led_start = VariableLED(LED_START);
		timer_can_update = Metro(100);
		CAN = MCP_CAN(SPI_CS);
		dashboard_status = {};
		mcu_status = {};
		Simulator::begin();
	}

	void TearDown() {
		Simulator::teardown();
	}
};
```

To utilize the setup class, test headers must be declared using `TEST_F` instead of `TEST`

`TEST_F(DashboardTesting, <TEST NAME>) { ... }`

# Configuring Test Case Compilation

To create an isolated set of test cases, create a new subdirectory in the [test](./test) folder and add it to the bottom of the [CMakeList](./test/CMakeLists.txt). Within the new subdirectory, create a new `CMakeLists.txt` and populate it using the [Test case CMake Template](./test/CMakeTemplate.txt). After that, you should be able to start writing test cases.

To add a new test file to an existing set of tests, simply create a new test file in that folder and add it to the `TEST_SOURCES` list in the corresponding CMakeList.

# API Documentation

## Simulator Class

Each test case must call `Simulator::begin()`, which runs the `setup()` method in the associated Arduino code.

Each test case must terminate by calling `Simulator::teardown()'. It is recommended that the `begin` and `teardown` methods be handled in the GTest class as demonstrated above.

`Simulator::begin` accepts one optional argument `loop_period`, which automatically increases the system time by `loop_period` milliseconds before each loop.

To call the arduino `loop()` function once, call `Simulator::next()`

- `Simulator::analogWrite(pin, value)` - Sets the input value for an analog pin
- `Simulator::digitalWrite(pin, value)` - Sets the input value for a digiital pin
- `Simulator::analogRead(pin)` - Reads the output value for an analog pin
- `Simulator::digitalRead(pin)` - Reads the output value for a digital pin
- `Simulator::getPinMode(pin)` - Gets the pinMode for a given pin

## CAN Simulator
This simulator facilitates the CAN bus. The user interface is similar to that of FlexCan.

- `CAN_simulator::read(CAN_message_t &msg)` - gets CAN message sent by simulator
- `CAN_simulator::write(const CAN_message_t& msg)` - creates CAN message to be read by simulator

For cleanup purposes, Simulator::teardown calls an additional method `CAN_simulator::purge()`, which removes all messages still sitting in the simulator buffer and resets all CAN configuration flags

## Serial Buses
As a helpful debugging utility, Serial and Serial2 both support a new function `setOutputPath(String filepath)`, which, if called before Serial.begin(), will redirect all Serial logging output to the specified filepath. Output path resets to `stdout` after each test case