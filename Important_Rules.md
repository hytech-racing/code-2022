## EV2.11 Accumulator - Accumulator Management System
* The AMS must monitor all critical voltages and temperatures in the accumulator as well the integrity of all its voltage and temperature inputs. If an out-of-range or a malfunction is detected, it must shut down the electrical systems, open the AIRs and shut down the I.C. drive system within 60 seconds.7 (Some GLV systems may remain energized – See Figure 34)
* The tractive system must remain disabled until manually reset by a person other than the driver. It must not be possible for the driver to re-activate the tractive system from within the car in case of an AMS fault.
* (Basically, if BMS signals an error, we have to *shut everything down* and *not allow re-activation until someone resets it*.)

## EV3.5 Motor Controllers
* All analog acceleration control signals (between accelerator pedal and motor controller) must have error checking which can detect open circuit, short to ground and short to sensor power and will shut down the torque production in less than one (1) second if a fault is detected.
** Might be built into MC, in which case that's sufficient

