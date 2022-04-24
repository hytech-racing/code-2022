/*
   Teensy 3.2 Main Control Unit code
   Written by Ethan Weinstock with assistance from Shaan Dhawan

   Rev 10 - 6/16/2021
   FSAE Nevada
*/

#include <stdint.h>
#define _USE_MATH_DEFINES
#include <cmath>

#include "ADC_SPI.h"
#include "HyTech_FlexCAN.h"
#include "HyTech_CAN.h"
#include "kinetis_flexcan.h"
#include "Metro.h"

#include "drivers.h"

// constants to define for different operation

#define DRIVER DAVID
#define TORQUE_1 100
#define TORQUE_2 130
#define TORQUE_3 160

#define LAUNCH_CONTROL_START_MULTIPLIER 0.65
#define LAUNCH_CONTROL_START_RPM 0
#define LAUNCH_CONTROL_END_RPM 1000

// set to true or false for debugging
#define DEBUG true
#define BMS_DEBUG_ENABLE false

#define LINEAR 0

// EXP
// torque = ae^(bx) + x - a
// see here: https://www.desmos.com/calculator/daidnwee5b
// https://www.desmos.com/calculator/9xgerxnlyv
#define B 0.065

#define MAP_MODE LINEAR

#include "MCU_rev10_dfs.h"

#include "driver_constants.h"

// Outbound CAN messages
MCU_pedal_readings mcu_pedal_readings{};
MCU_status mcu_status{};
MCU_wheel_speed mcu_wheel_speed{};

// Inbound CAN messages
BMS_coulomb_counts bms_coulomb_counts{};
BMS_status bms_status{};
BMS_temperatures bms_temperatures{};
BMS_voltages bms_voltages{};
Dashboard_status dashboard_status{};
MC_current_information mc_current_information{};
MC_internal_states mc_internal_states{};
MC_motor_position_information mc_motor_position_information{};
MC_voltage_information mc_voltage_information{};

//Timers
#if DEBUG
Metro timer_bms_imd_print_fault = Metro(500);
Metro timer_debug = Metro(200);
Metro timer_debug_raw_torque = Metro(200);
Metro timer_debug_torque = Metro(200);
Metro timer_bms_print_fault = Metro(500);
Metro timer_imd_print_fault = Metro(500);
#endif
Metro timer_inverter_enable = Metro(2000); // Timeout failed inverter enable
Metro timer_motor_controller_send = Metro(50);
Metro timer_coloumb_count_send = Metro(1000);
Metro timer_ready_sound = Metro(2000); // Time to play RTD sound
Metro timer_can_update = Metro(100);
Metro timer_sensor_can_update = Metro(5);
Metro timer_restart_inverter = Metro(500, 1); // Allow the MCU to restart the inverter
Metro timer_status_send = Metro(100);
Metro timer_watchdog_timer = Metro(500);

// this abuses Metro timer functionality to stay faulting once a fault has occurred
// autoreset makes the timer update to the current time and not by the interval
// this allows me to set the interval as 0 once a fault has occurred, leading to continuous faulting
// until a CAN message comes in which resets the timer and the interval
Metro timer_bms_heartbeat = Metro(0, 1);
// add dashboard heartbeat
// Metro timer_dashboard_heartbeat = Metro(0, 1);
// Metro timer_software_enable_interval = Metro(TIMER_SOFTWARE_ENABLE, 1);

#if BMS_DEBUG_ENABLE
#define TOTAL_IC 8                      // Number of ICs in the system
#define CELLS_PER_IC 9                  // Number of cells per IC
#define THERMISTORS_PER_IC 3            // Number of cell thermistors per IC
#define PCB_THERM_PER_IC 2              // Number of PCB thermistors per IC

BMS_detailed_voltages bms_detailed_voltages[8][3];
BMS_detailed_temperatures bms_detailed_temperatures[8];
BMS_onboard_detailed_temperatures bms_onboard_detailed_temperatures[TOTAL_IC];
BMS_onboard_temperatures bms_onboard_temperatures;
BMS_balancing_status bms_balancing_status[(TOTAL_IC + 3) / 4]; // Round up TOTAL_IC / 4 since data from 4 ICs can fit in a single message
MCU_analog_readings mcu_analog_readings{};
Metro timer_bms_print(1000);

#endif
/*
   Variables to store filtered values from ADC channels
*/
float filtered_accel1_reading{};
float filtered_accel2_reading{};
float filtered_brake1_reading{};
float filtered_brake2_reading{};

bool imd_faulting = false;
bool inverter_restart = false; // True when restarting the inverter

// uint16_t MAX_TORQUE = 600; // Torque in Nm * 10

uint32_t total_charge_amount = 0;
uint32_t total_discharge_amount = 0;

ADC_SPI ADC(ADC_CS, ADC_SPI_SPEED);
FlexCAN CAN(500000);

/*
   Wheel speed stuff
*/
int total_ticks_front_left{};
int total_ticks_front_right{};
float rpm_front_left{};
float rpm_front_right{};

int total_ticks_back_left{};
int total_ticks_back_right{};
float rpm_back_left{};
float rpm_back_right{};

static CAN_message_t tx_msg;

// coloumb counts
uint32_t total_discharge;
unsigned long previous_data_time;

void setup() {
  // no torque can be provided on startup
  mcu_status.set_max_torque(0);
  mcu_status.set_torque_mode(0);
  mcu_status.set_software_is_ok(true);

  pinMode(BRAKE_LIGHT_CTRL, OUTPUT);

  // change to input if comparator is PUSH PULL
  pinMode(FRONT_LEFT_WHEEL, INPUT_PULLUP);
  pinMode(FRONT_RIGHT_WHEEL, INPUT_PULLUP);
  pinMode(BACK_LEFT_WHEEL, INPUT_PULLUP);
  pinMode(BACK_RIGHT_WHEEL, INPUT_PULLUP);
  pinMode(INVERTER_CTRL, OUTPUT);

  // pinMode(FAN_1, OUTPUT);
  // pinMode(FAN_2, OUTPUT);

  pinMode(WATCHDOG_INPUT, OUTPUT);
  // the initial state of the watchdog is high
  // this is reflected in the static watchdog_state
  // starting high
  digitalWrite(WATCHDOG_INPUT, HIGH);
  pinMode(TEENSY_OK, OUTPUT);
  digitalWrite(TEENSY_OK, HIGH);

#if DEBUG
  Serial.begin(115200);
#endif
  CAN.begin();

  /* Configure CAN rx interrupt */
  interrupts();
  NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);
  attachInterruptVector(IRQ_CAN_MESSAGE, parse_can_message);
  FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;
  /* Configure CAN rx interrupt */

  delay(500);

#if DEBUG
  Serial.println("CAN system and serial communication initialized");
#endif


  analogWrite(FAN_1, FAN_1_DUTY_CYCLE);
  analogWrite(FAN_2, FAN_2_DUTY_CYCLE);
  // these are false by default
  mcu_status.set_bms_ok_high(false);
  mcu_status.set_imd_ok_high(false);

  digitalWrite(INVERTER_CTRL, HIGH);
  mcu_status.set_inverter_powered(true);
  inverter_heartbeat(0);


  // present action for 5s
  delay(5000);

  set_state(MCU_STATE::TRACTIVE_SYSTEM_NOT_ACTIVE);
  mcu_status.set_max_torque(TORQUE_1);
  mcu_status.set_torque_mode(1);

  /* Set up total discharge readings */
  setup_total_discharge();

}

void loop() {
  // Serial.println(analogRead(A4)*55 /12*3.3/1024);
  read_pedal_values();
  read_wheel_speed();
  read_status_values();

  /* Send state over CAN */
  if (timer_can_update.check()) {
    // Send Main Control Unit status message
    mcu_status.write(tx_msg.buf);
    tx_msg.id = ID_MCU_STATUS;
    tx_msg.len = sizeof(mcu_status);
    CAN.write(tx_msg);

    /*// Send coulomb counting information
      bms_coulomb_counts.set_total_charge(total_charge_amount);
      bms_coulomb_counts.set_total_discharge(total_discharge_amount);
      tx_msg.id = ID_BMS_COULOMB_COUNTS;
      tx_msg.len = sizeof(bms_coulomb_counts);
      CAN.write(tx_msg);*/
  }

  /* Send sensor data over CAN */
  if (timer_sensor_can_update.check()) {
    // Update the pedal readings to send over CAN
    mcu_pedal_readings.set_accelerator_pedal_1(filtered_accel1_reading);
    mcu_pedal_readings.set_accelerator_pedal_2(filtered_accel2_reading);
    mcu_pedal_readings.set_brake_transducer_1(filtered_brake1_reading);
    mcu_pedal_readings.set_brake_transducer_2(filtered_brake2_reading);

    // Send Main Control Unit pedal reading message
    mcu_pedal_readings.write(tx_msg.buf);
    tx_msg.id = ID_MCU_PEDAL_READINGS;
    tx_msg.len = sizeof(mcu_pedal_readings);
    CAN.write(tx_msg);

    //        // write the rpm data
    //        // scale factor for transmit
    //        mcu_wheel_speed.set_rpm_front_left(rpm_front_left*10);
    //        mcu_wheel_speed.set_rpm_front_right(rpm_front_left*10);
    //        mcu_wheel_speed.set_rpm_back_left(rpm_back_left*10);
    //        mcu_wheel_speed.set_rpm_back_right(rpm_back_right*10);
    //
    //        // Send Main Control Unit pedal reading message
    //        mcu_wheel_speed.write(tx_msg.buf);
    //        tx_msg.id = ID_MCU_WHEEL_SPEED;
    //        tx_msg.len = sizeof(mcu_wheel_speed);
    //        CAN.write(tx_msg);
  }

  if (timer_coloumb_count_send.check()) {
    bms_coulomb_counts.write(tx_msg.buf);
    tx_msg.id = ID_BMS_COULOMB_COUNTS;
    tx_msg.len = sizeof(bms_coulomb_counts);
    CAN.write(tx_msg);
  }

  /* Finish restarting the inverter when timer expires */
  if (timer_restart_inverter.check() && inverter_restart) {
    inverter_restart = false;
    digitalWrite(INVERTER_CTRL, HIGH);
    mcu_status.set_inverter_powered(true);
  }

  /* handle state functionality */
  state_machine();
  software_shutdown();

#if BMS_DEBUG_ENABLE
  static bool bms_print = false;
  if (Serial.available()) {
    String a = Serial.readString();
    if (a == "on" || a == "on\n") {
      Serial.println("BMS print turned on");
      bms_print = true;
    }
    else if (a == "off" || a == "off\n") {
      Serial.println("BMS print turned off");
      bms_print = false;
    }
  }
  if (bms_print && timer_bms_print.check()) print_bms();
#endif
}

inline void setup_total_discharge() {
  total_discharge = 0;
  previous_data_time = millis();
  bms_coulomb_counts.set_total_discharge(total_discharge);
}

inline void process_total_discharge() {
  unsigned long current_time = millis();
  double new_current = mc_current_information.get_dc_bus_current() / 10;
  uint32_t added_Ah = new_current * ((current_time - previous_data_time) * 10000 / (1000 * 60 * 60 )); //scaled by 10000 for telemetry parsing
  previous_data_time = current_time;
  total_discharge += added_Ah;
  bms_coulomb_counts.set_total_discharge(total_discharge);
}

inline void state_machine() {
  switch (mcu_status.get_state()) {
    case MCU_STATE::STARTUP: break;
    case MCU_STATE::TRACTIVE_SYSTEM_NOT_ACTIVE:
      inverter_heartbeat(0);
#if DEBUG
      Serial.println("TS NOT ACTIVE");
#endif
      // if TS is above HV threshold, move to Tractive System Active
      if (mc_voltage_information.get_dc_bus_voltage() >= MIN_HV_VOLTAGE) {
#if DEBUG
        Serial.println("Setting state to TS Active from TS Not Active");
#endif
        set_state(MCU_STATE::TRACTIVE_SYSTEM_ACTIVE);
      }
      break;

    case MCU_STATE::TRACTIVE_SYSTEM_ACTIVE:
      check_TS_active();
      inverter_heartbeat(0);

      // if start button has been pressed and brake pedal is held down, transition to the next state
      if (dashboard_status.get_start_btn() && mcu_status.get_brake_pedal_active()) {
#if DEBUG
        Serial.println("Setting state to Enabling Inverter");
#endif
        set_state(MCU_STATE::ENABLING_INVERTER);
      }
      break;

    case MCU_STATE::ENABLING_INVERTER:
      check_TS_active();
      inverter_heartbeat(1);

      // inverter enabling timed out
      if (timer_inverter_enable.check()) {
#if DEBUG
        Serial.println("Setting state to TS Active from Enabling Inverter");
#endif
        set_state(MCU_STATE::TRACTIVE_SYSTEM_ACTIVE);
      }
      // motor controller indicates that inverter has enabled within timeout period
      if (mc_internal_states.get_inverter_enable_state()) {
#if DEBUG
        Serial.println("Setting state to Waiting Ready to Drive Sound");
#endif
        set_state(MCU_STATE::WAITING_READY_TO_DRIVE_SOUND);
      }
      break;

    case MCU_STATE::WAITING_READY_TO_DRIVE_SOUND:
      check_TS_active();
      check_inverter_disabled();
      inverter_heartbeat(1);

      // if the ready to drive sound has been playing for long enough, move to ready to drive mode
      if (timer_ready_sound.check()) {
#if DEBUG
        Serial.println("Setting state to Ready to Drive");
#endif
        set_state(MCU_STATE::READY_TO_DRIVE);
      }
      break;

    case MCU_STATE::READY_TO_DRIVE:
      check_TS_active();
      check_inverter_disabled();

      //update_coulomb_count();
      if (timer_motor_controller_send.check()) {
        MC_command_message mc_command_message(0, 0, 0, 1, 0, 0);

        // FSAE EV.5.5
        // FSAE T.4.2.10
        if (filtered_accel1_reading < MIN_ACCELERATOR_PEDAL_1 || filtered_accel1_reading > MAX_ACCELERATOR_PEDAL_1) {
          mcu_status.set_no_accel_implausability(false);
#if DEBUG
          Serial.println("T.4.2.10 1");
#endif
        }
        else if (filtered_accel2_reading > MAX_ACCELERATOR_PEDAL_2 || filtered_accel2_reading < MIN_ACCELERATOR_PEDAL_2) {
          mcu_status.set_no_accel_implausability(false);
#if DEBUG
          Serial.println("T.4.2.10 2");
#endif
        }
        // check that the pedals are reading within 10% of each other
        // T.4.2.4
        else if (fabs((filtered_accel1_reading - START_ACCELERATOR_PEDAL_1) / (END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1) -
                      (filtered_accel2_reading - START_ACCELERATOR_PEDAL_2) / (END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2)) > 0.1) {
#if DEBUG
          Serial.println("T.4.2.4");
          Serial.printf("pedal 1 - %f\n", (filtered_accel1_reading - START_ACCELERATOR_PEDAL_1) / (END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1));
          Serial.printf("pedal 2 - %f\n", (filtered_accel2_reading - START_ACCELERATOR_PEDAL_2) / (END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2));
#endif
          mcu_status.set_no_accel_implausability(false);
        }
        else {
          mcu_status.set_no_accel_implausability(true);
        }

        // BSE check
        // EV.5.6
        // FSAE T.4.3.4
        if (filtered_brake1_reading < 409 || filtered_brake1_reading > 3687) {
          mcu_status.set_no_brake_implausability(false);
        }
        else {
          mcu_status.set_no_brake_implausability(true);
        }

        // FSAE EV.5.7
        // APPS/Brake Pedal Plausability Check
        if  (
          (
            (filtered_accel1_reading > ((END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1) / 4 + START_ACCELERATOR_PEDAL_1))
            ||
            (filtered_accel2_reading > ((END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2) / 4 + START_ACCELERATOR_PEDAL_2))
          )
          && mcu_status.get_brake_pedal_active()
        )
        {
          mcu_status.set_no_accel_brake_implausability(false);
        }
        else if
        (
          (filtered_accel1_reading < ((END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1) / 20 + START_ACCELERATOR_PEDAL_1))
          &&
          (filtered_accel2_reading < ((END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2) / 20 + START_ACCELERATOR_PEDAL_2))
        )
        {
          mcu_status.set_no_accel_brake_implausability(true);
        }

        int calculated_torque = 0;

        if (
          mcu_status.get_no_brake_implausability() &&
          mcu_status.get_no_accel_implausability() &&
          mcu_status.get_no_accel_brake_implausability() &&
          //mcu_status.get_software_is_ok() && //why was software not ok?
          mcu_status.get_bms_ok_high() &&
          mcu_status.get_imd_ok_high()
        ) {
          calculated_torque = calculate_torque();
        } else {
          Serial.println("not calculating torque");
          Serial.printf("no brake implausibility: %d\n", mcu_status.get_no_brake_implausability());
          Serial.printf("no accel implausibility: %d\n", mcu_status.get_no_accel_implausability());
          Serial.printf("no accel brake implausibility: %d\n", mcu_status.get_no_accel_brake_implausability());
          Serial.printf("software is ok: %d\n", mcu_status.get_software_is_ok());
          Serial.printf("get bms ok high: %d\n", mcu_status.get_bms_ok_high());
          Serial.printf("get imd ok high: %d\n", mcu_status.get_imd_ok_high());

        }
        // Implausibility exists, command 0 torque

#if DEBUG
        if (timer_debug_torque.check()) {
          /* Serial.print("MCU REQUESTED TORQUE: ");
            Serial.println(calculated_torque);
            Serial.print("MCU NO IMPLAUS ACCEL: ");
            Serial.println(mcu_status.get_no_accel_implausability());
            Serial.print("MCU NO IMPLAUS BRAKE: ");
            Serial.println(mcu_status.get_no_brake_implausability());
            Serial.print("MCU NO IMPLAUS ACCEL BRAKE: ");
            Serial.println(mcu_status.get_no_accel_brake_implausability());*/
          /* Serial.printf("ssok: %d\n", mcu_status.get_software_is_ok());
            Serial.printf("bms: %d\n", mcu_status.get_bms_ok_high());
            Serial.printf("imd: %d\n", mcu_status.get_imd_ok_high());*/
        }
#endif

        // Serial.print("RPM: ");
        // Serial.println(mc_motor_position_information.get_motor_speed());
        // Serial.println(calculated_torque);

        mc_command_message.set_torque_command(calculated_torque);

        mc_command_message.write(tx_msg.buf);
        tx_msg.id = ID_MC_COMMAND_MESSAGE;
        tx_msg.len = 8;
        CAN.write(tx_msg);
      }
      break;
  }
}

/* Shared state functinality */

// if TS is below HV threshold, return to Tractive System Not Active
inline void check_TS_active() {
  if (mc_voltage_information.get_dc_bus_voltage() < MIN_HV_VOLTAGE) {
#if DEBUG
    Serial.println("Setting state to TS Not Active, because TS is below HV threshold");
#endif
    set_state(MCU_STATE::TRACTIVE_SYSTEM_NOT_ACTIVE);
  }
}
// if the inverter becomes disabled, return to Tractive system active
inline void check_inverter_disabled() {
  if (!mc_internal_states.get_inverter_enable_state()) {
#if DEBUG
    Serial.println("Setting state to TS Active because inverter is disabled");
#endif
    set_state(MCU_STATE::TRACTIVE_SYSTEM_ACTIVE);
  }
}
// Send a message to the Motor Controller over CAN when vehicle is not ready to drive
inline void inverter_heartbeat(int enable) {
  if (timer_motor_controller_send.check()) {
    MC_command_message mc_command_message(0, 0, 0, enable, 0, 0);

    mc_command_message.write(tx_msg.buf);
    tx_msg.id = ID_MC_COMMAND_MESSAGE;
    tx_msg.len = sizeof(mc_command_message);
    CAN.write(tx_msg);
  }
}

/* Implementation of software shutdown */
inline void software_shutdown() {

  mcu_status.set_software_is_ok(true);

  // conditionally check depending on current state
  /*if (mcu_status.get_software_is_ok()){
      // check that software high ok and shutdown e are high when software is OK
      if (timer_software_enable_interval.check()){
          // once 100 ms passes, check this every loop
          timer_software_enable_interval.interval(0);
          // if software ok based signals are low 100 ms after software ok has been turned on, fault software ok
          if ((mcu_status.get_shutdown_inputs() & 0xC0) != 0xC0){
              mcu_status.set_software_is_ok(false);
          }
      }
    }
    else {
    /*
      // if the software ok based signals are high, software ok is false
      if ((mcu_status.get_shutdown_inputs() & 0xC0) != 0) { //need to do testing with this line
          mcu_status.set_software_is_ok(false);
      }
      // assume software is ok because any subsequent check will fail it
      // because all software ok based checks have been preforned
      else {
          mcu_status.set_software_is_ok(true);
     // }
    }*/


  // check inputs
  // BMS heartbeat has not arrived within time interval
  if (timer_bms_heartbeat.check()) {
    timer_bms_heartbeat.interval(0);
#if DEBUG
    Serial.println("no bms heartbeat");
#endif
    mcu_status.set_software_is_ok(false);
  }
  // if (timer_dashboard_heartbeat.check()){
  //     timer_dashboard_heartbeat.interval(0);
  //     #if DEBUG
  //        Serial.println("no dashboard");
  //     #endif
  //     mcu_status.set_software_is_ok(false);
  // }
  // check if any shutdown circuit inputs are low except software shutdown ones
  /*if ((mcu_status.get_shutdown_inputs() & 0x3F) != 0x3F){
     // Serial.println("not 3F");
      mcu_status.set_software_is_ok(false);
    }*/
  // add BMS software checks
  // software ok/not ok action
  if (mcu_status.get_software_is_ok()) {
    digitalWrite(TEENSY_OK, HIGH); //eventually make this HIGH only if software is ok
  } else {
    digitalWrite(TEENSY_OK, LOW);
  }
  /* Watchdog timer */
  if (timer_watchdog_timer.check()) {
    static bool watchdog_state = HIGH;
    watchdog_state = !watchdog_state;
    digitalWrite(WATCHDOG_INPUT, watchdog_state);
  }
  // }
  /*else {
      digitalWrite(TEENSY_OK, LOW);
      timer_software_enable_interval.interval(TIMER_SOFTWARE_ENABLE);
      timer_software_enable_interval.reset();
    }*/
  //Serial.println(mcu_status.get_software_is_ok());
}

/* Parse incoming CAN messages */
void parse_can_message() {
  static CAN_message_t rx_msg;
  while (CAN.read(rx_msg)) {
    switch (rx_msg.id) {
      case ID_MC_VOLTAGE_INFORMATION:        mc_voltage_information.load(rx_msg.buf);        break;
      case ID_MC_INTERNAL_STATES:            mc_internal_states.load(rx_msg.buf);            break;
      case ID_MC_CURRENT_INFORMATION:
        mc_current_information.load(rx_msg.buf);
        process_total_discharge();
        break;
      case ID_MC_MOTOR_POSITION_INFORMATION: mc_motor_position_information.load(rx_msg.buf); break;
      case ID_BMS_TEMPERATURES:              bms_temperatures.load(rx_msg.buf);              break;
      case ID_BMS_VOLTAGES:                  bms_voltages.load(rx_msg.buf);                  break;
      case ID_BMS_COULOMB_COUNTS:            bms_coulomb_counts.load(rx_msg.buf);            break;
      case ID_BMS_STATUS:
        bms_status.load(rx_msg.buf);
        // BMS heartbeat timer
        timer_bms_heartbeat.reset();
        timer_bms_heartbeat.interval(BMS_HEARTBEAT_TIMEOUT);
        break;
      case ID_DASHBOARD_STATUS:
        dashboard_status.load(rx_msg.buf);

        // timer_dashboard_heartbeat.reset();
        // timer_dashboard_heartbeat.interval(DASH_HEARTBEAT_TIMEOUT);
        /* process dashboard buttons */
        if (dashboard_status.get_mode_btn()) {
          switch (mcu_status.get_torque_mode()) {
            case 1:
              mcu_status.set_max_torque(TORQUE_2);
              mcu_status.set_torque_mode(2); break;
            case 2:
              mcu_status.set_max_torque(TORQUE_3);
              mcu_status.set_torque_mode(3); break;
            case 3:
              mcu_status.set_max_torque(TORQUE_1);
              mcu_status.set_torque_mode(1); break;
          }
        }
        if (dashboard_status.get_launch_ctrl_btn()) {
          mcu_status.toggle_launch_ctrl_active();
        }
        if (dashboard_status.get_mc_cycle_btn()) {
          reset_inverter();
        }
        // eliminate all action buttons to not process twice
        dashboard_status.set_button_flags(0);
        break;
#if BMS_DEBUG_ENABLE
      case ID_BMS_DETAILED_TEMPERATURES: {
          BMS_detailed_temperatures temp_det_temp = BMS_detailed_temperatures(rx_msg.buf);
          bms_detailed_temperatures[temp_det_temp.get_ic_id()].load(rx_msg.buf);
          break;
        }
      case ID_BMS_DETAILED_VOLTAGES: {
          BMS_detailed_voltages temp_det_volt = BMS_detailed_voltages(rx_msg.buf);
          bms_detailed_voltages[temp_det_volt.get_ic_id()][temp_det_volt.get_group_id()].load(rx_msg.buf);
          break;
        }
      case ID_BMS_ONBOARD_TEMPERATURES:
        bms_onboard_temperatures.load(rx_msg.buf);
        break;
      case ID_BMS_ONBOARD_DETAILED_TEMPERATURES: {
          BMS_onboard_detailed_temperatures temp_onboard_det_temp = BMS_onboard_detailed_temperatures(rx_msg.buf);
          bms_onboard_detailed_temperatures[temp_onboard_det_temp.get_ic_id()].load(rx_msg.buf);
          break;
        } s
      case ID_BMS_BALANCING_STATUS: {
          BMS_balancing_status temp = BMS_balancing_status(rx_msg.buf);
          bms_balancing_status[temp.get_group_id()].load(rx_msg.buf);
          break;
        }
      case ID_MCU_ANALOG_READINGS:
        mcu_analog_readings.load(rx_msg.buf);
        break;
#endif
    }
  }
}

inline void reset_inverter() {
  inverter_restart = true;
  digitalWrite(INVERTER_CTRL, LOW);
  timer_restart_inverter.reset();
  mcu_status.set_inverter_powered(false);

#if DEBUG
  Serial.println("INVERTER RESET");
#endif
}


float calculate_launch_control_torque_multiplier() {
  //float average_front_wheel_rpm = (rpm_front_right + rpm_front_left)/2.0;
  int16_t mc_rpm = abs(mc_motor_position_information.get_motor_speed());
  float slope = (LAUNCH_CONTROL_END_RPM - LAUNCH_CONTROL_START_RPM) / (1 - LAUNCH_CONTROL_START_MULTIPLIER);
  if (mc_rpm < LAUNCH_CONTROL_START_RPM) {
    return LAUNCH_CONTROL_START_MULTIPLIER;
  } else if (mc_rpm > LAUNCH_CONTROL_END_RPM) {
    return 1;
  } else {
    return (LAUNCH_CONTROL_START_MULTIPLIER + (mc_rpm - LAUNCH_CONTROL_START_RPM) * slope);
  }
}


/* Handle changes in state */
void set_state(MCU_STATE new_state) {
  if (mcu_status.get_state() == new_state) {
    return;
  }

  // exit logic
  switch (mcu_status.get_state()) {
    case MCU_STATE::STARTUP: break;
    case MCU_STATE::TRACTIVE_SYSTEM_NOT_ACTIVE: break;
    case MCU_STATE::TRACTIVE_SYSTEM_ACTIVE: break;
    case MCU_STATE::ENABLING_INVERTER: break;
    case MCU_STATE::WAITING_READY_TO_DRIVE_SOUND:
      // make dashboard stop buzzer
      mcu_status.set_activate_buzzer(false);
      mcu_status.write(tx_msg.buf);
      tx_msg.id = ID_MCU_STATUS;
      tx_msg.len = sizeof(mcu_status);
      CAN.write(tx_msg);
      break;
    case MCU_STATE::READY_TO_DRIVE: break;
  }

  mcu_status.set_state(new_state);

  // entry logic
  switch (new_state) {
    case MCU_STATE::STARTUP: break;
    case MCU_STATE::TRACTIVE_SYSTEM_NOT_ACTIVE: break;
    case MCU_STATE::TRACTIVE_SYSTEM_ACTIVE: break;
    case MCU_STATE::ENABLING_INVERTER: {
        MC_command_message mc_command_message(0, 0, 0, 1, 0, 0);
        tx_msg.id = 0xC0;
        tx_msg.len = 8;
        mc_command_message.write(tx_msg.buf); // many enable commands

        for (int i = 0; i < 10; i++) {
          CAN.write(tx_msg);
        }
        // look this up in datasheet
        mc_command_message.set_inverter_enable(false);
        mc_command_message.write(tx_msg.buf); // disable command
        CAN.write(tx_msg);

        mc_command_message.set_inverter_enable(true);
        mc_command_message.write(tx_msg.buf); // many more enable commands
        for (int i = 0; i < 10; i++) {
          CAN.write(tx_msg);
        }

        Serial.println("MCU Sent enable command");
        timer_inverter_enable.reset();
        break;
      }
    case MCU_STATE::WAITING_READY_TO_DRIVE_SOUND:
      // make dashboard sound buzzer
      mcu_status.set_activate_buzzer(true);
      mcu_status.write(tx_msg.buf);
      tx_msg.id = ID_MCU_STATUS;
      tx_msg.len = sizeof(mcu_status);
      CAN.write(tx_msg);

      timer_ready_sound.reset();
      Serial.println("RTDS enabled");
      break;
    case MCU_STATE::READY_TO_DRIVE:
      Serial.println("Ready to drive");
      break;
  }
}

int calculate_torque() {
  int calculated_torque = 0;

  const int max_torque = mcu_status.get_max_torque() * 10;
  if (calculate_launch_control_torque_multiplier() + 0.01 >= 1.0) {
    mcu_status.set_launch_ctrl_active(false);
  }
  int torque1 = map(round(filtered_accel1_reading), START_ACCELERATOR_PEDAL_1, END_ACCELERATOR_PEDAL_1, 0,  max_torque );
  int torque2 = map(round(filtered_accel2_reading), START_ACCELERATOR_PEDAL_2, END_ACCELERATOR_PEDAL_2, 0,  max_torque );

  float launch_control_torque_limit = max_torque * calculate_launch_control_torque_multiplier()
  if (mcu_status.get_launch_ctrl_btn_active()) {
    if (torque1 > launch_control_torque_limit) {
      torque1 = (int)launch_control_torque_limit;
    }
    if (torque2 > launch_control_torque_limit) {
      torque2 = (int)launch_control_torque_limit;
    }
  }


#if DEBUG
  Serial.print("max torque: ");
  Serial.println(max_torque);
  Serial.print("torque1: ");
  Serial.println(torque1);
  Serial.print("torque2: ");
  Serial.println(torque2);
#endif

  // torque values are greater than the max possible value, set them to max
  if (torque1 > max_torque) {
    torque1 = max_torque;
  }
  if (torque2 > max_torque) {
    torque2 = max_torque;
  }
  // compare torques to check for accelerator implausibility
  calculated_torque = (torque1 + torque2) / 2;

  if (calculated_torque > max_torque) {
    calculated_torque = max_torque;
  }
  if (calculated_torque < 0) {
    calculated_torque = 0;
  }


#if DEBUG
  if (timer_debug_raw_torque.check()) {
    Serial.print("TORQUE REQUEST DELTA PERCENT: "); // Print the % difference between the 2 accelerator sensor requests
    Serial.println(abs(torque1 - torque2) / (double) max_torque * 100);
    Serial.print("MCU RAW TORQUE: ");
    Serial.println(calculated_torque);
    Serial.print("TORQUE 1: ");
    Serial.println(torque1);
    Serial.print("TORQUE 2: ");
    Serial.print(torque2);
  }
#endif

  return calculated_torque;
}

// inline void update_coulomb_count() {
//     int new_current = mc_current_informtarion.get_dc_bus_current() * 10; // get current in Amps * 100
//     if (new_current > 0) {
//         total_discharge_amount += new_current;
//     } else {
//         total_charge_amount -= new_current;
//     }
// }

/* Read pedal sensor values */
inline void read_pedal_values() {
  /* Filter ADC readings */
  filtered_accel1_reading = ALPHA * filtered_accel1_reading + (1 - ALPHA) * ADC.read_adc(ADC_ACCEL_1_CHANNEL);
  filtered_accel2_reading = ALPHA * filtered_accel2_reading + (1 - ALPHA) * ADC.read_adc(ADC_ACCEL_2_CHANNEL);
  filtered_brake1_reading = ALPHA * filtered_brake1_reading + (1 - ALPHA) * ADC.read_adc(ADC_BRAKE_1_CHANNEL);
  filtered_brake2_reading = ALPHA * filtered_brake2_reading + (1 - ALPHA) * ADC.read_adc(ADC_BRAKE_2_CHANNEL);

#if DEBUG
  // Serial.print("ACCEL 1: "); Serial.println(filtered_accel1_reading);
  // Serial.print("ACCEL 2: "); Serial.println(filtered_accel2_reading);
  //  Serial.print("BRAKE 1: "); Serial.println(filtered_brake1_reading);
  //  Serial.print("BRAKE 2: "); Serial.println(filtered_brake2_reading);
#endif

  // only uses front brake pedal
  mcu_status.set_brake_pedal_active(filtered_brake1_reading >= BRAKE_ACTIVE);
  digitalWrite(BRAKE_LIGHT_CTRL, mcu_status.get_brake_pedal_active());

  /* Print values for debugging */
  /*#if DEBUG
    if (timer_debug.check()) {
      Serial.print("MCU PEDAL ACCEL 1: ");
      Serial.println(mcu_pedal_readings.get_accelerator_pedal_1());
      Serial.print("MCU PEDAL ACCEL 2: ");
      Serial.println(mcu_pedal_readings.get_accelerator_pedal_2());
      Serial.print("MCU PEDAL BRAKE: ");
      Serial.println(mcu_pedal_readings.get_brake_transducer_1());
      Serial.print("MCU BRAKE ACT: ");
      Serial.println(mcu_status.get_brake_pedal_active());
      Serial.print("MCU STATE: ");
      Serial.println(mcu_status.get_state());
    }
    #endif*/
}

/* Read shutdown system values */
inline void read_status_values() {
  /* Measure shutdown circuits' input */
  mcu_status.set_bms_ok_high(analogRead(BMS_OK_READ) > SHUTDOWN_HIGH);
  mcu_status.set_imd_ok_high(analogRead(IMD_OK_READ) > SHUTDOWN_HIGH);
  mcu_status.set_bspd_ok_high(analogRead(BSPD_OK_READ) > SHUTDOWN_HIGH);
  mcu_status.set_software_ok_high(analogRead(SOFTWARE_OK_READ) > SHUTDOWN_HIGH);

  /* Measure shutdown circuits' voltages */
  mcu_status.set_shutdown_b_above_threshold(analogRead(SHUTDOWN_B_READ) > SHUTDOWN_HIGH);
  mcu_status.set_shutdown_c_above_threshold(analogRead(SHUTDOWN_C_READ) > SHUTDOWN_HIGH);
  mcu_status.set_shutdown_d_above_threshold(analogRead(SHUTDOWN_D_READ) > SHUTDOWN_HIGH);
  mcu_status.set_shutdown_e_above_threshold(analogRead(SHUTDOWN_E_READ) > SHUTDOWN_HIGH);
}

/* Read wheel speed values */
// may need to move to interrupt based approach
inline void read_wheel_speed() {
  // front left
  {
    static uint8_t cur_state_front_left{};
    static uint8_t prev_state_front_left{};
    static int cur_time_front_left{};
    static int prev_time_front_left{};

    cur_state_front_left = digitalRead(FRONT_LEFT_WHEEL);

    if (cur_state_front_left == 0 && prev_state_front_left == 1) {
      cur_time_front_left = micros();
      int micros_elapsed = cur_time_front_left - prev_time_front_left;
      if (micros_elapsed > 500) {
        rpm_front_left = (60000000.0 / NUM_TEETH) / micros_elapsed;
        prev_time_front_left = cur_time_front_left;
        total_ticks_front_left += 1;
      }
    }

    if (micros() - prev_time_front_left > TIME_OUT && rpm_front_left) {
      rpm_front_left = 0;
    }

    prev_state_front_left = cur_state_front_left;
    update_distance_traveled();
  }
  // front right
  {
    static uint8_t cur_state_front_right{};
    static uint8_t prev_state_front_right{};
    static int cur_time_front_right{};
    static int prev_time_front_right{};

    cur_state_front_right = digitalRead(FRONT_RIGHT_WHEEL);

    if (cur_state_front_right == 0 && prev_state_front_right == 1) {
      cur_time_front_right = micros();
      int micros_elapsed = cur_time_front_right - prev_time_front_right;
      if (micros_elapsed > 500) {
        rpm_front_right = (60000000.0 / NUM_TEETH) / micros_elapsed;
        prev_time_front_right = cur_time_front_right;
        total_ticks_front_right += 1;
      }
    }

    if (micros() - prev_time_front_right > TIME_OUT && rpm_front_right) {
      rpm_front_right = 0;
    }

    prev_state_front_right = cur_state_front_right;
    update_distance_traveled();
  }
  // back left
  {
    static uint8_t cur_state_back_left{};
    static uint8_t prev_state_back_left{};
    static int cur_time_back_left{};
    static int prev_time_back_left{};

    cur_state_back_left = digitalRead(BACK_LEFT_WHEEL);

    if (cur_state_back_left == 0 && prev_state_back_left == 1) {
      cur_time_back_left = micros();
      int micros_elapsed = cur_time_back_left - prev_time_back_left;
      if (micros_elapsed > 500) {
        rpm_back_left = (60000000.0 / NUM_TEETH) / micros_elapsed;
        prev_time_back_left = cur_time_back_left;
        total_ticks_back_left += 1;
      }
    }

    if (micros() - prev_time_back_left > TIME_OUT && rpm_back_left) {
      rpm_back_left = 0;
    }

    prev_state_back_left = cur_state_back_left;
  }
  // back right
  {
    static uint8_t cur_state_back_right{};
    static uint8_t prev_state_back_right{};
    static int cur_time_back_right{};
    static int prev_time_back_right{};

    cur_state_back_right = digitalRead(BACK_RIGHT_WHEEL);

    if (cur_state_back_right == 0 && prev_state_back_right == 1) {
      cur_time_back_right = micros();
      int micros_elapsed = cur_time_back_right - prev_time_back_right;
      if (micros_elapsed > 500) {
        rpm_back_right = (60000000.0 / NUM_TEETH) / micros_elapsed;
        prev_time_back_right = cur_time_back_right;
        total_ticks_back_right += 1;
      }
    }

    if (micros() - prev_time_back_right > TIME_OUT && rpm_back_right) {
      rpm_back_right = 0;
    }

    prev_state_back_right = cur_state_back_right;
  }
}

/* Track how far the car has driven */
inline void update_distance_traveled() {
  // total_revs = ((total_ticks_front_left + total_ticks_front_right) / (2.0 * NUM_TEETH));
  // distance travelled = total_revs * wheel_circumference
  // scaled by 100 for digitizing
  mcu_status.set_distance_travelled(((total_ticks_front_left + total_ticks_front_right) / (2.0 * NUM_TEETH)) * WHEEL_CIRCUMFERENCE * 100);
}

#if BMS_DEBUG_ENABLE
inline void print_bms() {
  print_cells();
  print_temps();
  Serial.print("BMS state: ");
  Serial.println(bms_status.get_state());
}

void print_cells() {
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.println("\t\t\t\tRaw Cell Voltages\t\t\t\t\t\t\tCell Status (Ignoring or Balancing)");
  Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8\t\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
  for (int ic = 0; ic < TOTAL_IC; ic++) {
    Serial.print("IC"); Serial.print(ic); Serial.print("\t");
    for (int cell = 0; cell < CELLS_PER_IC; cell++) {
      double voltage = bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3) * 0.0001;
      Serial.print(voltage, 4); Serial.print("V\t");
    }
    Serial.print("\t");
    for (int cell = 0; cell < CELLS_PER_IC; cell++) {
      int balancing = bms_balancing_status[ic / 4].get_cell_balancing(ic % 4, cell);
      if (balancing) {
        Serial.print("BAL");
      }
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();
  Serial.println("\t\t\t\tDelta from Min Cell");
  Serial.println("\tC0\tC1\tC2\tC3\tC4\tC5\tC6\tC7\tC8");
  for (int ic = 0; ic < TOTAL_IC; ic++) {
    Serial.print("IC"); Serial.print(ic); Serial.print("\t");
    for (int cell = 0; cell < CELLS_PER_IC; cell++) {
      double voltage = (bms_detailed_voltages[ic][cell / 3].get_voltage(cell % 3) - bms_voltages.get_low()) * 0.0001;
      Serial.print(voltage, 4);
      Serial.print("V");
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println();
  Serial.print("Cell voltage statistics\t\tTotal: "); Serial.print(bms_voltages.get_total() / (double) 1e2, 4); Serial.print("V\t\t");
  Serial.print("Average: "); Serial.print(bms_voltages.get_average() / (double) 1e4, 4); Serial.print("V\t\t");
  Serial.print("Min: "); Serial.print(bms_voltages.get_low() / (double) 1e4, 4); Serial.print("V\t\t");
  Serial.print("Max: "); Serial.print(bms_voltages.get_high() / (double) 1e4, 4); Serial.println("V");
}

void print_temps() {
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.println("\t\tCell Temperatures\t\t\t\t\t\t\t\t\t   PCB Temperatures");
  Serial.println("\tTHERM 0\t\tTHERM 1\t\tTHERM 2\t\t\t\t\t\t\t\tTHERM 0\t\tTHERM 1");
  for (int ic = 0; ic < TOTAL_IC; ic++) {
    Serial.print("IC"); Serial.print(ic); Serial.print("\t");
    for (int therm = 0; therm < THERMISTORS_PER_IC; therm++) {
      double temp = ((double) bms_detailed_temperatures[ic].get_temperature(therm)) / 100;
      Serial.print(temp, 2);
      Serial.print(" ºC");
      Serial.print("\t");
    }
    Serial.print("\t\t\t\t\t\t");
    for (int therm = 0; therm < PCB_THERM_PER_IC; therm++) {
      double temp = ((double) bms_onboard_detailed_temperatures[ic].get_temperature(therm)) / 100;
      Serial.print(temp, 2);
      Serial.print(" ºC");
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.print("\nCell temperature statistics\t\t Average: ");
  Serial.print(bms_temperatures.get_average_temperature() / (double) 100, 2);
  Serial.print(" ºC\t\t");
  Serial.print("Min: ");
  Serial.print(bms_temperatures.get_low_temperature() / (double) 100, 2);
  Serial.print(" ºC\t\t");
  Serial.print("Max: ");
  Serial.print(bms_temperatures.get_high_temperature() / (double) 100, 2);
  Serial.println(" ºC\n");
  Serial.print("PCB temperature statistics\t\t Average: ");
  Serial.print(bms_onboard_temperatures.get_average_temperature() / (double) 100, 2);
  Serial.print(" ºC\t\t");
  Serial.print("Min: ");
  Serial.print(bms_onboard_temperatures.get_low_temperature() / (double) 100, 2);
  Serial.print(" ºC\t\t");
  Serial.print("Max: ");
  Serial.print(bms_onboard_temperatures.get_high_temperature() / (double) 100, 2);
  Serial.println(" ºC\n");
  Serial.println();
  Serial.print("GLV Battery voltage: ");
  Serial.println(mcu_analog_readings.get_glv_battery_voltage());
}


#endif
