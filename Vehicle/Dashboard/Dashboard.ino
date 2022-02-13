#include "Dashboard.h"
#include "DebouncedButton.h"
#include "HyTech_CAN.h"
#include "mcp_can.h"
#include "Metro.h"
#include "VariableLED.h"

// only send if receiving mcu status messages

// LED Variables
VariableLED led_ams   (LED_AMS);
VariableLED led_imd   (LED_IMD);
VariableLED led_mc_err(LED_MC_ERR);
VariableLED led_start (LED_START);
VariableLED led_mode  (LED_MODE);

Metro timer_led_ams   (LED_MIN_FAULT);
Metro timer_led_imd   (LED_MIN_FAULT);
Metro timer_led_mc_err(LED_MIN_FAULT);

// bool init_ams = true, init_imd = true;

// Button debouncing variables
DebouncedButton btn_mark;
DebouncedButton btn_mode;
DebouncedButton btn_mc_cycle;
DebouncedButton btn_start;
DebouncedButton btn_lc;

// CAN Variables
Metro timer_can_update = Metro(100);
MCP_CAN CAN(SPI_CS);

// CAN Messages
Dashboard_status dashboard_status{};
MC_fault_codes mc_fault_codes{};
MCU_status mcu_status{};

Metro timer_mcu_heartbeat(0);

inline void led_update();
inline void read_can();
inline void btn_update();
inline void mcu_status_received();
inline void mc_fault_codes_received();

void setup() {
    btn_mark.begin(BTN_MARK, 100);
    btn_mode.begin(BTN_MODE, 100);
    btn_mc_cycle.begin(BTN_MC_CYCLE, 100);
    btn_start.begin(BTN_START, 100);
    btn_lc.begin(BTN_LC, 100);

    pinMode(BUZZER,     OUTPUT);
    pinMode(LED_AMS,    OUTPUT);
    pinMode(LED_IMD,    OUTPUT);
    pinMode(LED_MODE,   OUTPUT);
    pinMode(LED_MC_ERR, OUTPUT);
    pinMode(LED_START,  OUTPUT);

    //Initiallizes CAN
    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500K
    {
        delay(200);
    }

    mcu_status.set_imd_ok_high(true);
    mcu_status.set_bms_ok_high(true);

    delay(7000);
}

void loop() {
    read_can();
    led_update();
    btn_update();

    static bool should_send = false;

    if (timer_mcu_heartbeat.check()){
        timer_mcu_heartbeat.interval(0);
        should_send = false;
    }
    else {
        should_send = true;
    }

    static uint8_t prev_buttons{}, curr_buttons{}, temp_buttons{};
    prev_buttons = curr_buttons;
    curr_buttons = dashboard_status.get_button_flags();
    temp_buttons = curr_buttons & (curr_buttons ^ prev_buttons);
    static bool prev_start_state;


    //Send CAN message
    //Timer to ensure dashboard isn't flooding data bus, also fires after a button is pressed
    // How does the check for button press work
    // the xor against previous buttons removes the button flags that were sent previously
    // the and enforces that only buttons that are currently pressed are allowed to be sent
    if(should_send &&
        (timer_can_update.check() || (temp_buttons) || (prev_start_state != dashboard_status.get_start_btn()))
      ){
        //create message to send
        uint8_t msg[8] = {0};
        dashboard_status.set_button_flags(temp_buttons);
        dashboard_status.write(msg);
        CAN.sendMsgBuf(ID_DASHBOARD_STATUS, 0, sizeof(dashboard_status), msg);
        //rest update timer
        timer_can_update.reset();
    }
    // clear buttons so they can be retoggled on in the loop
    dashboard_status.set_button_flags(0);
    prev_start_state = dashboard_status.get_start_btn();
}

inline void led_update(){
    led_ams.update();
    led_imd.update();
    led_mc_err.update();
    led_start.update();
    led_mode.update();
}

inline void btn_update(){
    // this sets the button to be high: it is set low in send can
    if (btn_mark.isPressed())     { dashboard_status.toggle_mark_btn();     }
    if (btn_mode.isPressed())     { dashboard_status.toggle_mode_btn();     }
    if (btn_mc_cycle.isPressed()) { dashboard_status.toggle_mc_cycle_btn(); }
    if (btn_lc.isPressed())       { dashboard_status.toggle_launch_ctrl_btn();    }

    dashboard_status.set_start_btn(btn_start.isPressed());
}

inline void read_can(){
    //len is message length, buf is the actual data from the CAN message
    static unsigned char len = 0;
    static unsigned char buf[8] = {0};

    while(CAN_MSGAVAIL == CAN.checkReceive()){
        CAN.readMsgBuf(&len, buf);
        static unsigned long canID = {};
        canID = CAN.getCanId();

        switch(canID){
            case ID_MCU_STATUS:
                mcu_status.load(buf);
                timer_mcu_heartbeat.reset();
                timer_mcu_heartbeat.interval(MCU_HEARTBEAT_TIMEOUT);
                mcu_status_received();
                break;

            case ID_MC_FAULT_CODES:
                mc_fault_codes.load(buf);
                mc_fault_codes_received();
            default:
                break;
        }
    }
}

inline void mcu_status_received(){
    // control buzzer
    digitalWrite(BUZZER, mcu_status.get_activate_buzzer());

    //BMS/AMS LED (bms and ams are the same thing)
    if (!mcu_status.get_bms_ok_high()){
        led_ams.setMode(BLINK_MODES::ON);
        dashboard_status.set_ams_led(static_cast<uint8_t>(BLINK_MODES::ON));
        timer_led_ams.reset();
    }
    // else if (init_ams){
    //     led_ams.setMode(BLINK_MODES::OFF);
    //     dashboard_status.set_ams_led(static_cast<uint8_t>(BLINK_MODES::OFF));
    //     init_ams = false;
    // }
    else if (led_ams.getMode() != BLINK_MODES::OFF && timer_led_ams.check()){
        led_ams.setMode(BLINK_MODES::SLOW);
        dashboard_status.set_ams_led(static_cast<uint8_t>(BLINK_MODES::SLOW));
    }

    //IMD LED
    if (!mcu_status.get_imd_ok_high()){
        led_imd.setMode(BLINK_MODES::ON);
        dashboard_status.set_imd_led(static_cast<uint8_t>(BLINK_MODES::ON));
        timer_led_imd.reset();
    }
    // else if (init_imd){
    //     led_imd.setMode(BLINK_MODES::OFF);
    //     dashboard_status.set_imd_led(static_cast<uint8_t>(BLINK_MODES::OFF));
    //     init_imd = false;
    // }
    else if (led_imd.getMode() != BLINK_MODES::OFF && timer_led_imd.check()){
        led_imd.setMode(BLINK_MODES::SLOW);
        dashboard_status.set_imd_led(static_cast<uint8_t>(BLINK_MODES::SLOW));
    }

    //Start LED
    switch(mcu_status.get_state()){
        case MCU_STATE::STARTUP:
            led_start.setMode(BLINK_MODES::OFF);
            dashboard_status.set_start_led(static_cast<uint8_t>(BLINK_MODES::OFF));
            break;
        case MCU_STATE::TRACTIVE_SYSTEM_NOT_ACTIVE:
            led_start.setMode(BLINK_MODES::SLOW);
            dashboard_status.set_start_led(static_cast<uint8_t>(BLINK_MODES::SLOW));
            break;
        case MCU_STATE::TRACTIVE_SYSTEM_ACTIVE:
            led_start.setMode(BLINK_MODES::FAST);
            dashboard_status.set_start_led(static_cast<uint8_t>(BLINK_MODES::FAST));
            break;
        case MCU_STATE::ENABLING_INVERTER:
        case MCU_STATE::WAITING_READY_TO_DRIVE_SOUND:
        case MCU_STATE::READY_TO_DRIVE:
            led_start.setMode(BLINK_MODES::ON);
            dashboard_status.set_start_led(static_cast<uint8_t>(BLINK_MODES::ON));
            break;
        default:
            led_start.setMode(BLINK_MODES::OFF);
            dashboard_status.set_start_led(static_cast<uint8_t>(BLINK_MODES::OFF));
            break;
    }

    // Mode LED
    switch(mcu_status.get_torque_mode()){
        case 1:
            led_mode.setMode(BLINK_MODES::OFF);
            dashboard_status.set_mode_led(static_cast<uint8_t>(BLINK_MODES::OFF));
            break;
        case 2:
            led_mode.setMode(BLINK_MODES::FAST);
            dashboard_status.set_mode_led(static_cast<uint8_t>(BLINK_MODES::FAST));
            break;
        case 3:
            led_mode.setMode(BLINK_MODES::ON);
            dashboard_status.set_mode_led(static_cast<uint8_t>(BLINK_MODES::ON));
            break;
        default:
            //led_mode.setMode(BLINK_MODES::OFF);
            //dashboard_status.set_mode_led(static_cast<uint8_t>(BLINK_MODES::OFF));
            break;
    }
}

inline void mc_fault_codes_received(){
    bool is_mc_err = false;

    if (mc_fault_codes.get_post_fault_hi() ||
        mc_fault_codes.get_post_fault_lo() ||
        mc_fault_codes.get_run_fault_hi() ||
        mc_fault_codes.get_run_fault_lo())
    {
        is_mc_err = true;
    }
    //MC Error LED

    if (is_mc_err){
        led_mc_err.setMode(BLINK_MODES::ON);
        dashboard_status.set_mc_error_led(static_cast<uint8_t>(BLINK_MODES::ON));
        timer_led_mc_err.reset();
    // display fault for 1 second and then it clears
    } else if (led_mc_err.getMode() != BLINK_MODES::OFF && timer_led_mc_err.check()){
        led_mc_err.setMode(BLINK_MODES::OFF);
        dashboard_status.set_mc_error_led(static_cast<uint8_t>(BLINK_MODES::OFF));
    }

    /*if (is_mc_err){
        led_mc_err.setMode(BLINK_MODES::ON);
        dashboard_status.set_mc_error_led(static_cast<uint8_t>(BLINK_MODES::ON));
        timer_led_mc_err.reset();
    }
    else if (led_mc_err.getMode() != BLINK_MODES::OFF && timer_led_mc_err.check()){
        led_mc_err.setMode(BLINK_MODES::SLOW);
        dashboard_status.set_mc_error_led(static_cast<uint8_t>(BLINK_MODES::SLOW));
    }*/
}
