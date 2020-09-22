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
   Button values
*/
bool btn_mark_value = false;
bool btn_mode_value = false;
bool btn_mc_cycle_value = false;
bool btn_start_value = false;
bool btn_extra_value = false;

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

void setup() {
  pinMode(BTN_MARK, INPUT);
  pinMode(BTN_MODE, INPUT);
  pinMode(BTN_MC_CYCLE, INPUT);
  pinMode(BTN_START, INPUT);
  pinMode(BTN_EXTRA, INPUT);

  //Sets SPI data transmission rate
  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 250K
    {
        Serial.println("CAN BUS Module Failed to Initialized");
        Serial.println("Retrying....");
        delay(200);
        
    }
  Serial.println("CAN BUS Shield init ok!");

}

void loop() {
  read_buttons();

  //Update the dashboard status
  dashboard_status.update(btn_mark_value, btn_mode_value, btn_mc_cycle_value, btn_start_value, btn_extra_value); 

  //Send CAN message
  if(timer_can_update.check()){ //Timer to ensure dashboard isn't flooding data bus

    //create message to send
    byte msg[8] = {btn_mark_value, btn_mode_value, btn_mc_cycle_value, btn_start_value, btn_extra_value};
    byte sndStat = CAN0.sendMsgBuf(ID_DASHBOARD_STATUS, 0, 8, msg);

    //rest update timer
    timer_can_update.reset();
  }
  
}


void read_buttons(){
//Check if buttons have been pressed
  //mark button
  if(digitalRead(BTN_MARK) == HIGH && !btn_mark_debouncing){
    btn_mark_debouncing = true;
    timer_btn_mark.reset();
    btn_mark_value = !btn_mark_value;
  }
  //mode button
  if(digitalRead(BTN_MODE) == HIGH && !btn_mode_debouncing){
    btn_mode_debouncing = true;
    timer_btn_mode.reset();
    btn_mode_value = !btn_mode_value;
  }
  //mc_cycle button
  if(digitalRead(BTN_MC_CYCLE) == HIGH && !btn_mc_cycle_debouncing){
    btn_mc_cycle_debouncing = true;
    timer_btn_mc_cycle.reset();
    btn_mc_cycle_value = !btn_mc_cycle_value;
  }
  //start button
  if(digitalRead(BTN_START) == HIGH && !btn_start_debouncing){
    btn_start_debouncing = true;
    timer_btn_start.reset();
    btn_start_value = !btn_start_value;
  }
  //extra button
  if(digitalRead(BTN_EXTRA) == HIGH && !btn_extra_debouncing){
    btn_extra_debouncing = true;
    timer_btn_extra.reset();
    btn_extra_value = !btn_mark_value;
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
