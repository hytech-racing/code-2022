*This file is in no way complete or exclusive*

# Formula Hybrid

### EV2.11 Accumulator - Accumulator Management System
* The AMS must monitor all critical voltages and temperatures in the accumulator as well the integrity of all its voltage and temperature inputs. If an out-of-range or a malfunction is detected, it must shut down the electrical systems, open the AIRs and shut down the I.C. drive system within 60 seconds.7 (Some GLV systems may remain energized – See Figure 34)
* The tractive system must remain disabled until manually reset by a person other than the driver. It must not be possible for the driver to re-activate the tractive system from within the car in case of an AMS fault.
* (Basically, if BMS signals an error, we have to *shut everything down* and *not allow re-activation until someone resets it*.)

### EV3.5 Motor Controllers
* All analog acceleration control signals (between accelerator pedal and motor controller) must have error checking which can detect open circuit, short to ground and short to sensor power and will shut down the torque production in less than one (1) second if a fault is detected.
  * Might be built into MC, in which case that's sufficient

### EV7.1 Shutdown Circuit
* The shutdown circuit is the primary safety system within a Formula Hybrid vehicle. It consists of a current loop that holds the Accumulator Isolation Relays (AIRs) closed. If the flow of current through this loop is interrupted, the AIRs will open, disconnecting the vehicle’s high voltage systems from the source of that voltage within the accumulator container.
* Shutdown may be initiated by several devices having different priorities as shown in the following table. (Look at Figure 34 in FH rules, page 109)
* In the event of an AMS, IMD or Brake over-travel fault, it must not be possible for the driver to re- activate the tractive system from within the cockpit. This includes “cycling power” through the use of the cockpit shutdown button.
* After enabling the shutdown circuit, at least one action, such as pressing a “start” button must be performed by the driver before the vehicle will respond to any accelerator input.
* The “start” action must be configured such that it cannot inadvertently be left in the “on” position after system shutdown.
* The cockpit shutdown button must be driver resettable. I.e. if the driver disables the system by pressing the cockpit-mounted shutdown button, the driver must then be able to restore system operation by pulling the button back out. Note: There must still be one additional action by the driver after pulling the button back out to reactivate the motor controller per EV7.7.2. (start action)

### EV7.8 Shutdown System Sequencing
* Look at Figure 38, FH rules pg 114. Very useful diagram of vehicle states

### EV7.9 Insulation Monitoring Device
* The status of the IMD must be displayed to the driver by a red indicator light in the cockpit

### EV7.9.3 Tractive System Disabling
* The tractive system must remain disabled until manually reset by a person other than the driver. It must not be possible for the driver to re-activate the tractive system from within the car in case of an IMD-related fault.

### EV9.2 Ready-to-Drive Sound
* The car must make a characteristic sound, for a minimum of 1 second and a maximum of 3 seconds, when it is ready to drive.

# Formula SAE Electric
### EV2.3 Accelerator Pedal Position Sensor
* At least two entirely separate sensors have to be used as APPSs. The sensors must have different transfer functions, each having a positive slope sense with either different gradients and/or offsets to the other(s).
* If an implausibility occurs between the values of the APPSs and persists for more than 100msec, the power to the motor(s) must be immediately shut down completely. It is not necessary to completely deactivate the tractive system, the motor controller(s) shutting down the power to the motor(s) is sufficient.
* Any failure of the APPS or APPS wiring must be detectable by the controller and must be treated like an implausibility, see EV2.3.5.
* When any kind of digital data transmission is used to transmit the APPS signal, the FMEA study must contain a detailed description of all the potential failure modes that can occur, the strategy that is used to detect these failures and the tests that have been conducted to prove that the detection strategy works. The failures to be considered must include but are not limited to the failure of the APPS, APPS signals being out of range, corruption of the message and loss of messages and the associated time outs.
* Any algorithm or electronic control unit that can manipulate the APPS signal, for example for vehicle dynamic functions such as traction control, may only lower the total driver requested torque and must never increase it. Thus the drive torque which is requested by the driver may never be exceeded.

### EV2.4 Brake System Encoder
* Any failure of the BSE or BSE wiring that persists more than 100 msec must be detectable by the controller and treated like an implausibility such that power to the motor(s) is immediately and completely shut down.
* Same failure mode requirement as acceleration pedal

### EV2.5 APPS/Brake Pedal Plausibility Check
* The power to the motors must be immediately shut down completely, if the mechanical brakes are actuated and the APPS signals more than 25% pedal travel at the same time. This must be demonstrated when the motor controllers are under load.
* The motor power shut down must remain active until the APPS signals less than 5% pedal travel, no matter whether the brakes are still actuated or not.

### EV3.6 Accumulator Management System (BMS for our purposes)
* The AMS must shutdown the tractive system by opening the AIRs, if critical voltage or temperature values according to the cell manufacturer’s datasheet and taking into account the accuracy of the measurement system are detected. If the AMS does perform a shutdown, then a red LED marked AMS must light up in the cockpit to confirm this.

### EV4.10 Activating the Tractive System
* The driver must be able to (re-)activate or reset the tractive system from within the cockpit without the assistance of any other person except for situations in which the AMS, IMD or BSPD have shut down the tractive system, see EV5.1.4 and EV5.1.5.
* Additional actions are required by the driver to set the car to ready-to-drive-mode e.g. pressing a dedicated start button, after the tractive system has been activated. One of these actions must include the brake pedal being pressed as ready-to-drive-mode is entered.

### EV5.1 Shutdown Circuit
* If the shutdown circuit is opened by the AMS, the IMD or the BSPD the tractive system must remain disabled until being manually reset by a person directly at the car which is not the driver. Remote reset, for example via WLAN or use of the three shutdown buttons or the TS master switch to reset the AMS, IMD or BSPD is not permitted.
