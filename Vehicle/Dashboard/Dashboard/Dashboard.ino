#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <HyTech_CAN.h>
#include <Metro.h>
#include <SPI.h>
#include <DebouncedButton.h>
#include <VariableLED.h>

/*
   Button pin definition
*/
#define BTN_MARK 23
#define BTN_MODE 24
#define BTN_MC_CYCLE 25
#define BTN_START 26
#define BTN_EXTRA 27

/*
   LED and buzzer pin definition
*/
#define BUZZER 28
#define LED_AMS 30
#define LED_IMD 31
#define LED_MODE 32
#define LED_MC_ERR 1
#define LED_START 2

/*
   LED and buzzer values
*/

bool is_mc_err = false;


/*
 * LED Variables
 */

VariableLED variable_led_start(LED_START);

/*
   Button debouncing variables
*/

DebouncedButton debounced_btn_mark(100);
DebouncedButton debounced_btn_mode(100);
DebouncedButton debounced_btn_mc_cycle(100);
DebouncedButton debounced_btn_start(100);
DebouncedButton debounced_btn_extra(100);

/*
   CAN Variables
*/

Metro timer_can_update = Metro(100);
#define SPI_CS 14
MCP_CAN CAN(SPI_CS);



/*
   Global Variables
*/
Dashboard_status dashboard_status;
MCU_status mcu_status;

void setup() {
  pinMode(BTN_MARK, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_MC_CYCLE, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_EXTRA, INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_AMS, OUTPUT);
  pinMode(LED_IMD, OUTPUT);
  pinMode(LED_MODE, OUTPUT);
  pinMode(LED_MC_ERR, OUTPUT);
  pinMode(LED_START, OUTPUT);

  //Initiallizes CAN
  while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 250K
    {
        delay(200);
    }
}

void loop() { 
  read_can();
  led_update();
  btn_update();

  //Send CAN message
  if(timer_can_update.check()){ //Timer to ensure dashboard isn't flooding data bus

    //create message to send
    byte msg[8];
    dashboard_status.write(msg);
    byte sndStat = CAN.sendMsgBuf(ID_DASHBOARD_STATUS, 0, 8, msg);

    //rest update timer
    timer_can_update.reset();
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

  if(debounced_btn_mark.update(digitalRead(BTN_MARK))){
    dashboard_status.toggle_mark_btn();
  }
  if(debounced_btn_mode.update(digitalRead(BTN_MODE))){
    dashboard_status.toggle_mode_btn();
  }
  if(debounced_btn_mc_cycle.update(digitalRead(BTN_MC_CYCLE))){
    dashboard_status.toggle_mc_cycle_btn();
  }
  if(debounced_btn_start.update(digitalRead(BTN_START))){
    dashboard_status.toggle_start_btn();
  }
  if(debounced_btn_extra.update(digitalRead(BTN_EXTRA))){
    dashboard_status.toggle_extra_btn();
  }
  
}

inline void read_can(){

  //len is message length, buf is the actual data from the CAN message
  unsigned char len = 0;
  unsigned char buf[8];
  
  while(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&len, buf);
    unsigned long canID = CAN.getCanId();

    //Sees if the canID is something the dashboard needs to deal with
    switch(canID){
      case ID_MCU_STATUS:
        mcu_status.load(buf);
        break;
      case ID_MC_FAULT_CODES:
        is_mc_err = false;
        for(int i = 0; i < 8; i++){
          if(buf[i] != 0){
            is_mc_err = true;
          }
        }
        break;
        
    }
  }
}
