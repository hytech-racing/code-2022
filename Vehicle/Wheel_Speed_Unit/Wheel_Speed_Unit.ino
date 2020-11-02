#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <Metro.h>

bool is_front = false; //Set based on which board you are uploading to

Metro timer_can_update_fast = Metro(10);

/**
 * CAN Variables
 */
FlexCAN CAN(500000);
const CAN_filter_t can_filter_ccu_status = {0, 0, ID_CCU_STATUS}; // Note: If this is passed into CAN.begin() it will be treated as a mask. Instead, pass it into CAN.setFilter(), making sure to fill all slots 0-7 with duplicate filters as necessary
static CAN_message_t tx_msg;

TCU_wheel_rpm tcu_wheel_rpm;
TCU_distance_traveled tcu_distance_traveled;

volatile byte cur_state_left = 0;
volatile byte cur_state_right = 0;
volatile byte prev_state_left = 0;
volatile byte prev_state_right = 0;
int cur_time_left = 0;
int cur_time_right = 0;
int prev_time_left = 0;
int prev_time_right = 0;
int total_ticks_left = 0;
int total_ticks_right = 0;
float rpm_left = 0;
float rpm_right = 0;
float total_revs = 0;

int num_teeth = 24;//CHANGE THIS FOR #OF TEETH PER REVOLUTION
float wheel_circumference = 1.300619; //CIRCUMFERENCE OF WHEEL IN METERS

void setup()
{
  pinMode(15, INPUT);
  pinMode(10, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(9600);
  Serial.println("Starting up");
  CAN.begin();
}

void set_states() {
  cur_state_left = digitalRead(15);
  cur_state_right = digitalRead(10);
}

void set_rpm_left() {
  cur_time_left = micros();
  int micros_elapsed = cur_time_left - prev_time_left;

  if (micros_elapsed > 500) {
    rpm_left = (60.0 * 1000.0 * 1000.0) / (micros_elapsed * 24.0);
    prev_time_left = cur_time_left;
    total_ticks_left += 1;
    print_rpms();
  }
}

void set_rpm_right() {
  cur_time_right = micros();
  int micros_elapsed = cur_time_right - prev_time_right;

  if (micros_elapsed > 500) {
    rpm_right = (60.0 * 1000.0 * 1000.0) / (micros_elapsed * 24.0);
    prev_time_right = cur_time_right;
    total_ticks_right += 1;
    print_rpms();
  }
}

void update_wheel_speeds() {
  set_states();
  
  if (cur_state_left == 0 && prev_state_left == 1) {
    set_rpm_left();
    if(is_front) update_distance_traveled(); 
  }

  if (cur_state_right == 0 && prev_state_right == 1) {
    set_rpm_right();
    if(is_front) update_distance_traveled();
  }

  if (micros() - prev_time_left > 500000) {
    if (rpm_left != 0) {
      rpm_left = 0;
      print_rpms();
    }
  }

  if (micros() - prev_time_right > 500000) {
    if (rpm_right != 0) {
      rpm_right = 0;
      print_rpms();
    }
  }

  prev_state_left = cur_state_left;
  prev_state_right = cur_state_right;
}

void update_distance_traveled() {
  total_revs = ((total_ticks_left + total_ticks_right) / (1.0 * num_teeth)); //Should be devided by 2 * num_teeth, but currently only one wheel is sensed
}

void print_rpms() {
    Serial.print("RPM Left: ");
    Serial.print(rpm_left);
    Serial.print("    RPM Right: ");
    Serial.print(rpm_right);
    if(is_front) {
      Serial.print("    Total Revs: ");
      Serial.print(total_revs);
    }
    Serial.println();
}

void loop()
{
  update_wheel_speeds();
  
  if (timer_can_update_fast.check()) {
        tx_msg.timeout = 10; // Use blocking mode, wait up to ?ms to send each message instead of immediately failing (keep in mind this is slower)

        tcu_wheel_rpm.set_wheel_rpm_left(rpm_left * 100);
        tcu_wheel_rpm.set_wheel_rpm_right(rpm_right * 100);
        tcu_wheel_rpm.write(tx_msg.buf);

        if (is_front) { tx_msg.id = ID_TCU_WHEEL_RPM_FRONT; }
        else { tx_msg.id = ID_TCU_WHEEL_RPM_REAR; }
        
        tx_msg.len = sizeof(TCU_wheel_rpm);
        CAN.write(tx_msg);
        tx_msg.timeout = 0;

        if (is_front) {
          tx_msg.timeout = 10;
          tcu_distance_traveled.set_distance_traveled(total_revs * wheel_circumference * 100);
          tcu_distance_traveled.write(tx_msg.buf);
          tx_msg.id = ID_TCU_DISTANCE_TRAVELED;
          tx_msg.len = sizeof(TCU_distance_traveled);
          CAN.write(tx_msg);
          tx_msg.timeout = 0;
        }
    }
}
