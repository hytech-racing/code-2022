#include <cstring>
#include <cstdint>

#pragma pack(push,1)

class Dashboard_status {
    public:
  
        Dashboard_status() = default;
        Dashboard_status(uint8_t buf[]){load(buf);}

        void load(uint8_t buf[8]){
            memcpy(this,buf,sizeof(*this));
        }
        
        void write(uint8_t buf[8]){
            memcpy(buf,this,sizeof(*this));
        }

        //Button Getters
        uint8_t get_button_flags(){
            return button_flags;
        }
        bool get_mark_btn(){
            return button_flags & 0x1;
        }
        bool get_mode_btn(){
            return (button_flags & 0x2);
        }
        bool get_mc_cycle_btn(){
            return (button_flags & 0x4);
        }
        bool get_start_btn(){
            return (button_flags & 0x8);
        }
        bool get_extra_btn(){
            return (button_flags & 0x10);
        }

        //Button Setters
        void set_button_flags(uint8_t flags){
            button_flags =  flags;
        }
        void set_mark_btn(bool mark_btn){
            button_flags = (button_flags & 0xFE) | (mark_btn & 0x1); 
        }
        void set_mode_btn(bool mode_btn){
            button_flags = (button_flags & 0xFD) | ((mode_btn & 0x1) << 1);
        }
        void set_mc_cycle_btn(bool mc_cycle_btn){
            button_flags = (button_flags & 0xFB) | ((mc_cycle_btn & 0x1) << 2);
        }
        void set_start_btn(bool start_btn){
            button_flags = (button_flags & 0xF7) | ((start_btn & 0x1) << 3);
        }
        void set_extra_btn(bool extra_btn){
            button_flags = (button_flags & 0xEF) | ((extra_btn & 0x1) << 4);
        }

        //Button Toggles
        void toggle_mark_btn(){
            button_flags ^= 0x1;
        }
        void toggle_mode_btn(){
            button_flags ^= 0x2;
        }
        void toggle_mc_cycle_btn(){
            button_flags ^= 0x4;
        }
        void toggle_start_btn(){
            button_flags ^= 0x8;
        }
        void toggle_extra_btn(){
            button_flags ^= 0x10;
        }

        //LED Getters
        uint8_t get_led_flags(){
            return led_flags;
        }
        bool get_ams_led(){
            return led_flags & 0x1;
        }
        bool get_imd_led(){
            return (led_flags & 0x2);
        }
        uint8_t get_mode_led(){
            return (led_flags & 0xC) >> 2; //2 bits required to store mode
        }
        bool get_mc_error_led(){
            return (led_flags & 0x10);
        }
        uint8_t get_start_led(){
            return (led_flags & 0x60) >> 5;//2 bits required to store start
        }

        //LED Setters
        void set_led_flags(uint8_t flags){
            led_flags = flags;
        }
        void set_ams_led(bool ams_led) {
            led_flags = (led_flags & 0xFE) | (ams_led & 0x1); 
        }
        void set_imd_led(bool imd_led) {
            led_flags = (led_flags & 0xFD) | ((imd_led & 0x1) << 1);
        }
        void set_mode_led(uint8_t mode_led) {
            led_flags = (led_flags & 0xF3) | ((mode_led & 0x3) << 2);
        }
        void set_mc_error_led(bool mc_error_led) {
            led_flags = (led_flags & 0xEF) | ((mc_error_led & 0x1) << 4);
        }
        void set_start_led(uint8_t start_led) {
            led_flags = (led_flags & 0x9F) | ((start_led & 0x3) << 5);
        }

    private:
        uint8_t button_flags;
        uint8_t led_flags;
};

#pragma pack(pop)
