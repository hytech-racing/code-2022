*This file is in no way complete or exclusive*

## EV2.11 Accumulator - Accumulator Management System
* The AMS must monitor all critical voltages and temperatures in the accumulator as well the integrity of all its voltage and temperature inputs. If an out-of-range or a malfunction is detected, it must shut down the electrical systems, open the AIRs and shut down the I.C. drive system within 60 seconds.7 (Some GLV systems may remain energized – See Figure 34)
* The tractive system must remain disabled until manually reset by a person other than the driver. It must not be possible for the driver to re-activate the tractive system from within the car in case of an AMS fault.
* (Basically, if BMS signals an error, we have to *shut everything down* and *not allow re-activation until someone resets it*.)

## EV3.5 Motor Controllers
* All analog acceleration control signals (between accelerator pedal and motor controller) must have error checking which can detect open circuit, short to ground and short to sensor power and will shut down the torque production in less than one (1) second if a fault is detected.
  * Might be built into MC, in which case that's sufficient

##EV7.1 Shutdown Circuit
* The shutdown circuit is the primary safety system within a Formula Hybrid vehicle. It consists of a current loop that holds the Accumulator Isolation Relays (AIRs) closed. If the flow of current through this loop is interrupted, the AIRs will open, disconnecting the vehicle’s high voltage systems from the source of that voltage within the accumulator container.
* Shutdown may be initiated by several devices having different priorities as shown in the following table. (Look at Figure 34 in FH rules, page 109)
* In the event of an AMS, IMD or Brake over-travel fault, it must not be possible for the driver to re- activate the tractive system from within the cockpit. This includes “cycling power” through the use of the cockpit shutdown button.
* After enabling the shutdown circuit, at least one action, such as pressing a “start” button must be performed by the driver before the vehicle will respond to any accelerator input.
* The “start” action must be configured such that it cannot inadvertently be left in the “on” position after system shutdown.

##EV7.8 Shutdown System Sequencing
* Look at Figure 38, FH rules pg 114. Very useful diagram of vehicle states

##EV7.9 Insulation Monitoring Device
* The status of the IMD must be displayed to the driver by a red indicator light in the cockpit

##EV9.2 Ready-to-Drive Sound
* The car must make a characteristic sound, for a minimum of 1 second and a maximum of 3 seconds, when it is ready to drive.
