#include "Dashboard.h"
#include "DebouncedButton.h"
#include "HyTech_CAN.h"
#include "mcp_can.h"
#include "Metro.h"
#include "VariableLED.h"

// LED and buzzer values
bool is_mc_err = false;

// LED Variables
VariableLED variable_led_start(LED_START);

// Button debouncing variables
DebouncedButton debounced_btn_mark;
DebouncedButton debounced_btn_mode;
DebouncedButton debounced_btn_mc_cycle;
DebouncedButton debounced_btn_start;
DebouncedButton debounced_btn_extra;

// CAN Variables
Metro timer_can_update = Metro(100);
MCP_CAN CAN(SPI_CS);

// CAN Messages
Dashboard_status dashboard_status{};
MCU_status mcu_status{};

inline void led_update();
inline void read_can();
inline void btn_update();

void setup() {
  debounced_btn_mark.begin(BTN_MARK, 100);
  debounced_btn_mode.begin(BTN_MODE, 100);
  debounced_btn_mc_cycle.begin(BTN_MC_CYCLE, 100);
  debounced_btn_start.begin(BTN_START, 100);
  debounced_btn_extra.begin(BTN_EXTRA, 100);

  pinMode(BUZZER,     OUTPUT);
  pinMode(LED_AMS,    OUTPUT);
  pinMode(LED_IMD,    OUTPUT);
  pinMode(LED_MODE,   OUTPUT);
  pinMode(LED_MC_ERR, OUTPUT);
  pinMode(LED_START,  OUTPUT);

  //Initiallizes CAN
  while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 250K
  {
    delay(200);
  }
}

void loop() { 
  read_can();
  led_update();
  btn_update();
  static uint8_t prev_buttons{}, curr_buttons{};
  prev_buttons = curr_buttons;
  curr_buttons = dashboard_status.get_button_flags();
  //Send CAN message
  //Timer to ensure dashboard isn't flooding data bus, also fires after a button is pressed
  // How does the check for button press work
    // the xor against previous buttons removes the button flags that were sent previously
    // the and enforces that only buttons that are currently pressed are allowed to be sent
    if(timer_can_update.check() || (curr_buttons & (curr_buttons ^ prev_buttons))){
    //create message to send
    static byte msg[8] = {0};
    dashboard_status.write(msg);
    CAN.sendMsgBuf(ID_DASHBOARD_STATUS, 0, sizeof(dashboard_status), msg);
    //rest update timer
    timer_can_update.reset();
    // clear buttons so they can be retoggled on in the loop
    dashboard_status.set_button_flags(0);
    }
}

inline void led_update(){
  //BMS/AMS LED (bms and ams are the same thing)
  digitalWrite(LED_AMS, !mcu_status.get_bms_ok_high()); //get_bms_ok_high outputs 1 if things are good.  We want light on when things are bad so negate 
  dashboard_status.set_ams_led(!mcu_status.get_bms_ok_high());
  
  //IMD LED
  digitalWrite(LED_IMD, !mcu_status.get_imd_okhs_high());//get_imd_okhs_high outputs 1 if things are good.  We want light on when things are bad so negate
  dashboard_status.set_imd_led(!mcu_status.get_imd_okhs_high());

  //MC Error LED
  digitalWrite(LED_MC_ERR, is_mc_err);
  dashboard_status.set_mc_error_led(is_mc_err);

  //Start LED
  switch(mcu_status.get_state()){
    case MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
      variable_led_start.setMode(BLINK_MODES::OFF);
      dashboard_status.set_start_led(0);
      break;
    case MCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
      variable_led_start.setMode(BLINK_MODES::FAST);
      dashboard_status.set_start_led(2);
      break;
    case MCU_STATE_ENABLING_INVERTER:
      variable_led_start.setMode(BLINK_MODES::ON);
      dashboard_status.set_start_led(1);
      break;
  }
  variable_led_start.update();
}

inline void btn_update(){
  // this sets the button to be high: it is set low in send can
  if(debounced_btn_mark.check())     {dashboard_status.toggle_mark_btn();}
  if(debounced_btn_mode.check())     {dashboard_status.toggle_mode_btn();}
  if(debounced_btn_mc_cycle.check()) {dashboard_status.toggle_mc_cycle_btn();}
  if(debounced_btn_start.check())    {dashboard_status.toggle_start_btn();}
  if(debounced_btn_extra.check())    {dashboard_status.toggle_extra_btn();}

}

inline void read_can(){
  //len is message length, buf is the actual data from the CAN message
  static unsigned char len = 0;
  static unsigned char buf[8] = {0};
  
  while(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&len, buf);
    static unsigned long canID = {};
    canID = CAN.getCanId();

    //Sees if the canID is something the dashboard needs to deal with
    switch(canID){
      case ID_MCU_STATUS:
        mcu_status.load(buf);
        break;
      case ID_MC_FAULT_CODES:
        is_mc_err = false;
        static int i;
        for(i = 0; i < 8; i++){
          if(buf[i] != 0){
            is_mc_err = true;
            break;
          }
        }
        break;
    }
  }
}
