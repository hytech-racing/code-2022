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
   Button debouncing and input variables
*/
bool btn_mark_pressed = false;
bool btn_mode_pressed = false;
bool btn_mc_cycle_pressed = false;
bool btn_start_pressed = false;
bool btn_extra_pressed = false;

bool btn_mark_debouncing = false;
bool btn_mode_debouncing = false;
bool btn_mc_cycle_debouncing = false;
bool btn_start_debouncing = false;
bool btn_extra_debouncing = false;

Metro timer_btn_mark = Metro(100);
Metro timer_btn_mode = Metro(100);
Metro timer_btn_mc_cycle = Metro(100);
Metro timer_btn_start = Metro(100);
Metro timer_btn_extra = Metro(100);

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
  update();

  //Send CAN message
  if(timer_can_update.check()){ //Timer to ensure dashboard isn't flooding data bus

    //create message to send
    byte msg[8] = dashboard_status.write();
    byte sndStat = CAN0.sendMsgBuf(ID_DASHBOARD_STATUS, 0, 8, msg);

    //rest update timer
    timer_can_update.reset();
  }
  
}

void update(){

  //Update start LED
  if(led_start){
    digitalWrite(BTN_START, HIGH);
  }
  else{
  digitalWrite(BTN_START, LOW);
  }
  
  //Update LED values from MCU Status
  if(mcu_status.get_imd_okhs_high()){
    digitalWrite(LED_IMD, HIGH);
  }
  else{
    digitalWrite(LED_IMD, LOW);
  }
}

/*DELETE COMMENT AFTER 
 * #define BUZZER = 28
#define LED_AMS = 30

#define LED_MODE = 32
#define LED_MC_ERR = 1

still need to do:
buzzer
ams (is this bms?)
mode (can't find)
mc_err (can't find)

 */


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
      
    }
    
  }
}



void read_buttons(){
//Check if buttons have been pressed
  //mark button
  if(digitalRead(BTN_MARK) == HIGH && !btn_mark_debouncing){
    btn_mark_debouncing = true;
    timer_btn_mark.reset();
    dashboard_status.set_mark(!dashboard_status.get_mark());
  }
  //mode button
  if(digitalRead(BTN_MODE) == HIGH && !btn_mode_debouncing){
    btn_mode_debouncing = true;
    timer_btn_mode.reset();
    dashboard_status.set_mode(!dashboard_status.get_mode());
  }
  //mc_cycle button
  if(digitalRead(BTN_MC_CYCLE) == HIGH && !btn_mc_cycle_debouncing){
    btn_mc_cycle_debouncing = true;
    timer_btn_mc_cycle.reset();
    dashboard_status.set_mc_cycle(!dashboard_status.get_mc_cycle());
  }
  //start button
  if(digitalRead(BTN_START) == HIGH && !btn_start_debouncing){
    btn_start_debouncing = true;
    timer_btn_start.reset();
    dashboard_status.set_start(!dashboard_status.get_start());
  }
  //extra button
  if(digitalRead(BTN_EXTRA) == HIGH && !btn_extra_debouncing){
    btn_extra_debouncing = true;
    timer_btn_extra.reset();
    dashboard_status.set_extra(!dashboard_status.get_extra());
  }

//Reset debouncing timers
  if(timer_btn_mark.check()){        //mark button
    btn_mark_debouncing = false;
  }
  if(timer_btn_mode.check()){        //mode button
    btn_mode_debouncing = false;
  }
  if(timer_btn_mc_cycle.check()){    //mc cycle button
    btn_mc_cycle_debouncing = false;
  }
  if(timer_btn_start.check()){      //start button
    btn_start_debouncing = false;
  }
  if(timer_btn_extra.check()){      //extra button
    btn_extra_debouncing = false;
  }
}
