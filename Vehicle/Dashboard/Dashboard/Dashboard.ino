#include <mcp_can_dfs.h>
#include <mcp_can.h>
#include <HyTech_CAN.h>
#include <Metro.h>
#include <SPI.h>

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
#define BUZZER = 28
#define LED_AMS = 30
#define LED_IMD = 31
#define LED_MODE = 32
#define LED_MC_ERR = 1
#define LED_START = 2

/*
   LED and buzzer values
*/
bool buzzer = false;
bool led_ams = false;
bool led_imd = false;
bool led_mode =  false;
bool led_mc_err = false;
bool led_start = false;

/*
 * LED Variables
 */

_VariableLED variable_led_ams;
_VariableLED variable_led_imd;
_VariableLED variable_led_mode;
_VariableLED variable_led_mc_err;
_VariableLED variable_led_start;

/*
   Button debouncing variables
*/

_DebouncedButton debounced_btn_mark(100);
_DebouncedButton debounced_btn_mode(100);
_DebouncedButton debounced_btn_mc_cycle(100);
_DebouncedButton debounced_btn_start(100);
_DebouncedButton debounced_btn_extra(100);

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
MCU_status mcu_status

void setup() {
  pinMode(BTN_MARK, INPUT);
  pinMode(BTN_MODE, INPUT);
  pinMode(BTN_MC_CYCLE, INPUT);
  pinMode(BTN_START, INPUT);
  pinMode(BTN_EXTRA, INPUT);

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
  Serial.println("CAN BUS Shield init ok!");

}

void loop() {
  read_buttons();
  read_can()
  btn_update();

  //Send CAN message
  if(timer_can_update.check()){ //Timer to ensure dashboard isn't flooding data bus

    //create message to send
    byte msg[8] = dashboard_status.write();
    byte sndStat = CAN0.sendMsgBuf(ID_DASHBOARD_STATUS, 0, 8, msg);

    //rest update timer
    timer_can_update.reset();
  }
  
}

void led_update(){
  //BMS/AMS LED (bms and ams are the same thing)
  if(!mcu_status.get_bms_ok_high()){ //get_bms_ok_high outputs 1 if things are good.  We want light on when things are bad.
    variable_led_ams.setMode(1);
  }
  else{
    variable_led_ams.setMode(0);
  }

  //IMD LED
  if(!mcu_status.get_imd_okhs_high()){ //get_imd_okhs_high outputs 1 if things are good.  We want light on when things are bad.
    variable_led_imd.setMode(1);
  }
  else{
    variable_led_imd.setMode(0);
  }

  //Start LED
  if(!mcu_status.get_imd_okhs_high()){ //get_imd_okhs_high outputs 1 if things are good.  We want light on when things are bad.
    variable_led_imd.setMode(1);
  }
  else{
    variable_led_imd.setMode(0);
  }

  switch(mcu_status.getState()){
    case MCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE:
    variable_led_start(0);
    break;
    case MCU_STATE_TRACTIVE_SYSTEM_ACTIVE:
    variable_led_start(2);
    break;
    case MCU_STATE_ENABLING_INVERTER:
    variable_led_start(1);
    break;
  }
  
  
}

void btn_update(){

  if(debounced_btn_mark.update(digitalRead(BTN_MARK))){
    dashboard_status.set_mark(!dashboard_status.get_mark());
  }
  if(debounced_btn_mode.update(digitalRead(BTN_MODE))){
    dashboard_status.set_mode(!dashboard_status.get_mode());
  }
  if(debounced_btn_mc_cycle.update(digitalRead(BTN_MC_CYCLE))){
    dashboard_status.set_mc_cycle(!dashboard_status.get_mc_cycle());
  }
  if(debounced_btn_start.update(digitalRead(BTN_START))){
    dashboard_status.set_start(!dashboard_status.get_start());
  }
  if(debounced_btn_extra.update(digitalRead(BTN_EXTRA))){
    dashboard_status.set_extra(!dashboard_status.get_extra());
  }
  
}

void read_can(){

  //len is message length, buf is the actual data from the CAN message
  unsigned char len = 0;
  unsigned char buf[8];
  
  if(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&len, buf);
    unsigned long canID = CAN.getCanId();

    //Sees if the canID is something the dashboard needs to deal with
    switch(canID){
      case ID_MCU_STATUS:
        mcu_status.load(buf);
        break;
    }
  }
}
